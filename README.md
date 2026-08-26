# c07c — The Chain

Part 3 of the c07 arc ("The Console"), continuing the shell built in
[c07a](https://github.com/thecodingidiot-com/c07a-the-command) and
[c07b](https://github.com/thecodingidiot-com/c07b-the-grammar). This
part wires pipes and all four redirection forms into real execution.

Chapter: https://thecodingidiot.com — c07c-the-chain

## What's here

- `solution/` — the reference solution: `pipe()`+`dup2()`-based
  pipeline execution (recursive, any number of stages), redirections
  (`<`, `>`, `>>`, `<<` heredoc) resolved in the parent before
  `fork()`, and the parser's structural unit tests carried forward
  from c07b.
- `test.sh` — a bash-comparison tester. Every case is piped, as one
  script, to both the built shell and to real `bash`; stdout, stderr,
  and exit status are diffed. Bash is the oracle.

## Building

```bash
cd solution
make re
```

## Testing

```bash
cp test.sh solution/
cd solution
bash test.sh
```

15 cases, all diffed clean against real bash output.

## License

MIT — see `LICENSE`.
