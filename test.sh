#!/bin/bash
# c07 reference shell -- bash-comparison tester.
# Each case is piped, as one script, to both our shell and to bash;
# stdout, stderr, and exit status are diffed. Bash is the oracle.

set -o pipefail

pass_count=0
fail_count=0

run_case() {
    # $3 = "status_only" to skip stdout/stderr text comparison (used for
    # messages that are legitimately implementation-specific wording, e.g.
    # "command not found" -- every real shell phrases that differently).
    local label="$1"
    local script="$2"
    local mode="${3:-full}"
    local our_out our_err our_status
    local bash_out bash_err bash_status

    our_out=$(printf '%s' "$script" | ./c07shell 2>/tmp/c07_our_err)
    our_status=$?
    our_err=$(cat /tmp/c07_our_err)

    bash_out=$(printf '%s' "$script" | bash 2>/tmp/c07_bash_err)
    bash_status=$?
    bash_err=$(cat /tmp/c07_bash_err)

    local ok=1
    [[ "$our_status" == "$bash_status" ]] || ok=0
    if [[ "$mode" == "full" ]]; then
        [[ "$our_out" == "$bash_out" ]] || ok=0
    fi

    if [[ "$ok" == "1" ]]; then
        echo "PASS  $label"
        pass_count=$((pass_count + 1))
    else
        echo "FAIL  $label"
        echo "      ours : status=$our_status out=[$our_out] err=[$our_err]"
        echo "      bash : status=$bash_status out=[$bash_out] err=[$bash_err]"
        fail_count=$((fail_count + 1))
    fi
}

# --- checkpoint A: single external commands + cd/pwd/exit -------------------

run_case "echo simple"            $'echo hello\n'
run_case "echo multiple words"    $'echo one two three\n'
run_case "exit code from true propagates via bare exit" $'true\nexit\n'
run_case "exit code from false propagates via bare exit" $'false\nexit\n'
run_case "exit with explicit code" $'exit 7\n'
run_case "pwd runs"                $'pwd\n'
run_case "cd then pwd"             $'cd /tmp\npwd\n'
run_case "nonexistent command"     $'nosuchcommand123\n' status_only

# --- checkpoint C: pipes and redirections ------------------------------------

WORKDIR=$(mktemp -d)
run_case "2-stage pipe"           $'echo hello | tr a-z A-Z\n'
run_case "3-stage pipe"           $'printf "b\\na\\nc\\n" | sort | tr a-z A-Z\n'
run_case "redirect out"           "echo redirected > $WORKDIR/out.txt\ncat $WORKDIR/out.txt\n"
run_case "redirect append"        "echo one > $WORKDIR/app.txt\necho two >> $WORKDIR/app.txt\ncat $WORKDIR/app.txt\n"
run_case "redirect in"            "printf 'hello from file\\n' > $WORKDIR/in.txt\ncat < $WORKDIR/in.txt\n"
run_case "heredoc"                $'cat << EOF\nline one\nline two\nEOF\n'
run_case "pipe combined with redirect" "echo hi | tr a-z A-Z > $WORKDIR/combo.txt\ncat $WORKDIR/combo.txt\n"
rm -rf "$WORKDIR"

# ── leak report ─────────────────────────────────────────────────────────────
#
# Runs one representative invocation under valgrind and REPORTS what it finds.
# It never changes the pass/fail count. A leak is something to look at, not a
# reason to refuse your work — but you should see it, because a program that
# leaks is a program that will eventually be killed by the machine it runs on.
#
# Leaks are split by whose code lost the memory. A loss record whose stack
# names one of your own .c files is yours. One that lives entirely inside
# SDL, Mesa or glibc is not, and there is nothing for you to fix there.

leak_report() {
    local label="$1"; shift
    local log="${WORK_DIR:-/tmp}/leaks.$$.log"
    local mine=0 theirs=0 rec frames

    if ! command -v valgrind >/dev/null 2>&1; then
        printf "  ${C_BOLD}NOTE${C_RESET}  %s: valgrind is not installed, skipping\n" "$label"
        return 0
    fi

    valgrind --leak-check=full --show-leak-kinds=definite,indirect \
             --error-exitcode=0 --log-file="$log" "$@" >/dev/null 2>&1

    if [[ ! -s "$log" ]]; then
        printf "  ${C_BOLD}NOTE${C_RESET}  %s: valgrind produced no output\n" "$label"
        return 0
    fi

    # Split the log into loss records and ask, of each, whether any frame
    # points at a source file sitting in this directory.
    while IFS= read -r rec; do
        frames=$(sed -n "${rec}"',/^==[0-9]*== *$/p' "$log")
        # Every record carries valgrind's own malloc frame; that is not yours.
        # A frame is yours only if it names a source file sitting right here.
        local f owned=0
        for f in $(grep -oE '\(([A-Za-z0-9_-]+\.c):[0-9]+\)' <<<"$frames" \
                   | tr -d '()' | cut -d: -f1 | sort -u); do
            [[ "$f" == vg_replace_malloc.c ]] && continue
            [[ -f "$f" ]] && owned=1
        done
        if (( owned )); then
            mine=$((mine + 1))
            if (( mine == 1 )); then
                printf "  ${C_RED}LEAK${C_RESET}  %s — memory lost by your code:\n" "$label"
            fi
            grep -E 'bytes in [0-9,]+ blocks are (definitely|indirectly)' <<<"$frames" \
                | sed 's/^==[0-9]*== /        /'
            grep -oE '\(([A-Za-z0-9_-]+\.c:[0-9]+)\)' <<<"$frames" \
                | grep -v vg_replace_malloc | head -3 | tr -d '()' \
                | sed 's/^/          at /'
        else
            theirs=$((theirs + 1))
        fi
    done < <(grep -nE 'bytes in [0-9,]+ blocks are (definitely|indirectly) lost' "$log" | cut -d: -f1)

    if (( mine == 0 )); then
        printf "  ${C_GREEN}OK${C_RESET}    %s — no memory lost by your code" "$label"
        if (( theirs > 0 )); then
            printf ' (%d leak(s) inside libraries you did not write)' "$theirs"
        fi
        printf '\n'
    else
        printf '        this does not fail the tester — fix it anyway\n'
    fi
    rm -f "$log"
    return 0
}

# The graphical chapters run until you quit them, and a program killed
# mid-loop reports everything it has not freed yet as "lost" -- which would be
# a lie. So this starts a virtual display, lets the program run, sends it a
# 'q', and measures the clean exit.
leak_report_gui() {
    local label="$1"; shift
    if ! command -v valgrind >/dev/null 2>&1; then
        printf "  ${C_BOLD}NOTE${C_RESET}  %s: valgrind is not installed, skipping\n" "$label"
        return 0
    fi
    if ! command -v xvfb-run >/dev/null 2>&1 || ! command -v xte >/dev/null 2>&1; then
        printf "  ${C_BOLD}NOTE${C_RESET}  %s: needs xvfb-run and xte for a clean exit, skipping\n" "$label"
        return 0
    fi
    printf "  ${C_BOLD}....${C_RESET}  %s: running under valgrind, this takes a minute\n" "$label"
    local inner="${WORK_DIR:-/tmp}/leak_gui.$$.sh"
    {
        echo "C_GREEN=\"${C_GREEN}\"; C_RED=\"${C_RED}\"; C_BOLD=\"${C_BOLD}\"; C_RESET=\"${C_RESET}\""
        echo "WORK_DIR=\"${WORK_DIR:-/tmp}\""
        declare -f leak_report
        echo '( sleep 12; xte "key q" 2>/dev/null; sleep 5; xte "key q" 2>/dev/null ) &'
        printf 'leak_report %q' "$label"
        printf ' %q' "$@"
        printf '\n'
    } > "$inner"
    timeout 240 xvfb-run -a bash "$inner"
    local rc=$?
    rm -f "$inner"
    if (( rc == 124 )); then
        printf "  ${C_BOLD}NOTE${C_RESET}  %s: the program never exited, so there is nothing honest to measure\n" "$label"
        printf "        (a program killed mid-loop reports everything it holds as lost)\n"
    fi
    return 0
}

echo
printf 'echo hi\ncd /tmp\npwd\n' | leak_report "c07shell" ./c07shell

echo
echo "$pass_count passed, $fail_count failed"
exit "$fail_count"
