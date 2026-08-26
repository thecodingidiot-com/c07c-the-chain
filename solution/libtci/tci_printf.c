#include "libtci.h"
#include <stdarg.h>   /* va_list, va_start, va_arg, va_end */
#include <unistd.h>   /* write() */
#include <stdint.h>   /* uintptr_t — pointer-to-integer cast for %p */

/*
 * t_fmt — parsed flags, width, and precision for one conversion specifier.
 *
 * Flags are set to 1 when present in the format string, 0 otherwise.
 * width      — minimum field width; output is padded to this many characters.
 * precision  — for %s: maximum characters printed; for %d/%i/%u/%x/%X:
 *              minimum digits (zero-padded on the left).
 * has_precision — distinguishes "%.0d" (precision 0) from "%d" (no precision).
 */
typedef struct s_fmt
{
    int  minus;          /* '-' : left-align within the field           */
    int  zero;           /* '0' : pad with zeros instead of spaces      */
    int  plus;           /* '+' : always print a sign for signed values */
    int  space;          /* ' ' : prefix positive values with a space   */
    int  hash;           /* '#' : alternate form (0x/0X prefix for hex) */
    int  width;          /* minimum total field width                   */
    int  precision;      /* precision value, if has_precision is set    */
    int  has_precision;  /* 1 if a '.' was present in the specifier     */
}   t_fmt;

/* forward declarations — all helpers are file-scoped */
static int  tci_putchar_fd(char c, int fd);
static int  tci_putstr_fd(char *s, int fd);
static int  tci_putnbr_base(unsigned long n, const char *base, int fd);
static int  pad_chars(char c, int n, int fd);
static int  tci_print_char(int c, t_fmt *f);
static int  tci_print_str(char *s, t_fmt *f);
static int  tci_print_ptr(void *ptr);
static int  tci_print_signed(int n, t_fmt *f);
static int  tci_print_unsigned(unsigned int n, const char *base, t_fmt *f);
static int  parse_fmt(const char *fmt, int i, t_fmt *f, va_list *args);
static int  dispatch_fmt(char spec, va_list *args, t_fmt *f);

/*
 * tci_putchar_fd — write one character to fd.
 * Returns 1 (bytes written) so callers can accumulate the total count.
 */
static int  tci_putchar_fd(char c, int fd)
{
    write(fd, &c, 1);
    return (1);
}

/*
 * tci_putstr_fd — write a null-terminated string to fd.
 * NULL is treated as the string "(null)", matching libc printf behaviour.
 * Returns the number of bytes written.
 */
static int  tci_putstr_fd(char *s, int fd)
{
    int  len;

    if (!s)
        return (tci_putstr_fd("(null)", fd));
    len = (int)tci_strlen(s);
    write(fd, s, len);
    return (len);
}

/*
 * tci_putnbr_base — write an unsigned long in the given base to fd.
 *
 * base is a string of digit characters: "0123456789" for decimal,
 * "0123456789abcdef" for lowercase hex, etc.  The length of the base string
 * determines the radix.
 *
 * The digits are computed in reverse order (least-significant first), stored
 * in a local buffer, then reversed before writing.  Returns digit count.
 */
static int  tci_putnbr_base(unsigned long n, const char *base, int fd)
{
    char  buf[64];
    int   blen;
    int   len;
    int   i;
    char  tmp;

    blen = (int)tci_strlen(base);
    len = 0;
    if (n == 0) {          /* zero must produce at least one digit */
        buf[len++] = base[0];
    }
    while (n > 0) {
        buf[len++] = base[n % blen];   /* least-significant digit first */
        n /= blen;
    }
    /* reverse the digit string in-place */
    i = 0;
    while (i < len / 2) {
        tmp = buf[i];
        buf[i] = buf[len - 1 - i];
        buf[len - 1 - i] = tmp;
        i++;
    }
    write(fd, buf, len);
    return (len);
}

/*
 * pad_chars — write n copies of character c to fd.
 * Used for both space-padding (width alignment) and zero-padding.
 * Returns n if n > 0, otherwise 0 (never a negative count).
 */
static int  pad_chars(char c, int n, int fd)
{
    int  i;

    i = 0;
    while (i < n) {
        write(fd, &c, 1);
        i++;
    }
    return (n > 0 ? n : 0);
}

/*
 * tci_print_char — handle the %c specifier.
 *
 * A single character occupies one byte of content; the field width applies
 * around it.  The character value arrives as int because va_arg promotes
 * char to int — it is cast back to unsigned char before writing.
 *
 *   %5c   'X'  →  "    X"    (right-aligned, 4 spaces then the char)
 *   %-5c  'X'  →  "X    "    (left-aligned, char then 4 spaces)
 */
static int  tci_print_char(int c, t_fmt *f)
{
    int  pad;
    int  count;

    pad = f->width > 1 ? f->width - 1 : 0;
    count = 0;
    if (!f->minus)
        count += pad_chars(' ', pad, 1);    /* leading spaces for right-align */
    count += tci_putchar_fd((unsigned char)c, 1);
    if (f->minus)
        count += pad_chars(' ', pad, 1);    /* trailing spaces for left-align */
    return (count);
}

/*
 * tci_print_str — handle the %s specifier.
 *
 * Precision truncates the string to at most f->precision characters (bytes,
 * not Unicode code points).  Width then pads the (possibly truncated) result.
 *
 *   %10s   "hello"  →  "     hello"    (right-aligned)
 *   %-10s  "hello"  →  "hello     "    (left-aligned)
 *   %.3s   "hello"  →  "hel"           (truncated)
 *   %10.3s "hello"  →  "       hel"    (truncated then right-aligned)
 */
static int  tci_print_str(char *s, t_fmt *f)
{
    int  slen;
    int  pad;
    int  count;

    if (!s)
        s = "(null)";
    slen = (int)tci_strlen(s);
    if (f->has_precision && f->precision < slen)
        slen = f->precision;         /* clamp to precision */
    pad = f->width > slen ? f->width - slen : 0;
    count = 0;
    if (!f->minus)
        count += pad_chars(' ', pad, 1);
    count += (int)write(1, s, slen); /* write exactly slen bytes */
    if (f->minus)
        count += pad_chars(' ', pad, 1);
    return (count);
}

/*
 * tci_print_ptr — handle the %p specifier.
 *
 * Pointers are always printed as "0x" followed by a lowercase hex address.
 * NULL is the special case "(nil)", matching glibc printf behaviour.
 * No width or precision flags are applied — %p ignores them in our subset.
 *
 * uintptr_t is used for the cast because it is guaranteed wide enough to hold
 * any pointer value without loss, unlike unsigned int or unsigned long.
 */
static int  tci_print_ptr(void *ptr)
{
    int  count;

    if (!ptr)
        return (tci_putstr_fd("(nil)", 1));
    count = (int)write(1, "0x", 2);
    count += tci_putnbr_base((uintptr_t)ptr, "0123456789abcdef", 1);
    return (count);
}

/*
 * tci_print_signed — handle the %d and %i specifiers.
 *
 * The value is converted to long before negating so that INT_MIN (-2147483648)
 * does not overflow: -INT_MIN is undefined as int because INT_MAX is 2147483647.
 *
 * Output layout (left to right):
 *   [spaces]  [sign]  [zeros from precision]  [digits]  [spaces if left-align]
 *
 * The 'zero' flag pads with '0' between the sign and the digits.  It is
 * overridden by '-' (left-align) and ignored when precision is specified —
 * the same interaction rules as libc printf.
 *
 *   %10d    7   →  "         7"
 *   %010d   7   →  "0000000007"
 *   %-10d   7   →  "7         "
 *   %+d     7   →  "+7"
 *   % d     7   →  " 7"
 *   %.5d    7   →  "00007"
 */
static int  tci_print_signed(int n, t_fmt *f)
{
    char  buf[20];    /* enough for any 32-bit decimal (max 10 digits + sign) */
    int   len;
    long  val;        /* long to survive -INT_MIN without overflow */
    char  sign;
    int   prec_pad;   /* extra '0' digits from precision */
    int   content;    /* total printable characters: sign + prec_pad + digits */
    int   pad;        /* spaces or zeros added by width */
    int   count;
    int   i;
    char  tmp;

    val = n;
    sign = 0;
    if (val < 0) {
        sign = '-';
        val = -val;    /* safe because val is long */
    } else if (f->plus) {
        sign = '+';
    } else if (f->space) {
        sign = ' ';
    }
    /* build digits in reverse order, then flip */
    len = 0;
    if (val == 0)
        buf[len++] = '0';
    while (val > 0) {
        buf[len++] = "0123456789"[val % 10];
        val /= 10;
    }
    i = 0;
    while (i < len / 2) {
        tmp = buf[i];
        buf[i] = buf[len - 1 - i];
        buf[len - 1 - i] = tmp;
        i++;
    }
    prec_pad = 0;
    if (f->has_precision && f->precision > len)
        prec_pad = f->precision - len;
    content = (sign ? 1 : 0) + prec_pad + len;
    pad = f->width > content ? f->width - content : 0;
    count = 0;
    if (!f->minus && !(f->zero && !f->has_precision))
        count += pad_chars(' ', pad, 1);    /* space-pad before sign */
    if (sign)
        count += tci_putchar_fd(sign, 1);
    if (!f->minus && f->zero && !f->has_precision)
        count += pad_chars('0', pad, 1);    /* zero-pad after sign */
    count += pad_chars('0', prec_pad, 1);   /* precision zero-fill */
    count += (int)write(1, buf, len);
    if (f->minus)
        count += pad_chars(' ', pad, 1);    /* trailing spaces for left-align */
    return (count);
}

/*
 * tci_print_unsigned — handle %u, %x, and %X.
 *
 * The base string selects the output format:
 *   "0123456789"        → decimal  (%u)
 *   "0123456789abcdef"  → lowercase hex  (%x)
 *   "0123456789ABCDEF"  → uppercase hex  (%X)
 *
 * The '#' flag prepends "0x" (lowercase) or "0X" (uppercase) for non-zero hex
 * values.  The prefix is detected by checking whether the base length is 16
 * and whether the 'a'/'A' character in the base string is lowercase.
 *
 * The prefix is written before zero-padding, so "%#010x" with 255 produces
 * "0x000000ff" (prefix then zeros, not zeros then prefix).
 *
 * Layout:
 *   [spaces]  [prefix]  [zeros from 0-flag or precision]  [digits]  [spaces]
 */
static int  tci_print_unsigned(unsigned int n, const char *base, t_fmt *f)
{
    char          buf[20];
    int           len;
    unsigned long val;
    int           blen;
    const char   *prefix;
    int           prefix_len;
    int           prec_pad;
    int           content;
    int           pad;
    int           count;
    int           i;
    char          tmp;

    val = (unsigned long)n;
    blen = (int)tci_strlen(base);
    prefix = "";
    prefix_len = 0;
    if (f->hash && n != 0) {
        /* base[10] == 'a' distinguishes lowercase from uppercase hex */
        if (blen == 16 && base[10] == 'a')
            prefix = "0x";
        else if (blen == 16)
            prefix = "0X";
        prefix_len = (int)tci_strlen(prefix);
    }
    len = 0;
    if (val == 0)
        buf[len++] = base[0];
    while (val > 0) {
        buf[len++] = base[val % blen];
        val /= blen;
    }
    i = 0;
    while (i < len / 2) {
        tmp = buf[i];
        buf[i] = buf[len - 1 - i];
        buf[len - 1 - i] = tmp;
        i++;
    }
    prec_pad = 0;
    if (f->has_precision && f->precision > len)
        prec_pad = f->precision - len;
    content = prefix_len + prec_pad + len;
    pad = f->width > content ? f->width - content : 0;
    count = 0;
    if (!f->minus && !(f->zero && !f->has_precision))
        count += pad_chars(' ', pad, 1);
    if (prefix_len)
        count += (int)write(1, prefix, prefix_len);  /* "0x"/"0X" after spaces */
    if (!f->minus && f->zero && !f->has_precision)
        count += pad_chars('0', pad, 1);    /* zero-pad after prefix */
    count += pad_chars('0', prec_pad, 1);
    count += (int)write(1, buf, len);
    if (f->minus)
        count += pad_chars(' ', pad, 1);
    return (count);
}

/*
 * parse_fmt — parse the flags/width/precision that follow a '%'.
 *
 * Called with i pointing to the first character after '%'.  Advances i
 * past all flag characters, the width, the '.', and the precision, stopping
 * at the conversion specifier character.  Returns the updated index.
 *
 * Width and precision may be given as '*', in which case the next int
 * argument is consumed from the va_list.  A negative '*' width sets the
 * '-' flag and uses the absolute value (matching POSIX behaviour).
 * A negative '*' precision is treated as if no precision was specified.
 *
 * Interaction rules applied at the end:
 *   '-' overrides '0'   (left-align makes zero-padding meaningless)
 *   '+' overrides ' '   (explicit sign supersedes the space prefix)
 */
static int  parse_fmt(const char *fmt, int i, t_fmt *f, va_list *args)
{
    tci_memset(f, 0, sizeof(*f));
    f->precision = -1;
    /* consume flag characters in any order */
    while (fmt[i] == '-' || fmt[i] == '0' || fmt[i] == '+'
            || fmt[i] == ' ' || fmt[i] == '#') {
        if (fmt[i] == '-') f->minus = 1;
        if (fmt[i] == '0') f->zero  = 1;
        if (fmt[i] == '+') f->plus  = 1;
        if (fmt[i] == ' ') f->space = 1;
        if (fmt[i] == '#') f->hash  = 1;
        i++;
    }
    /* width: either '*' (consume next int arg) or literal digits */
    if (fmt[i] == '*') {
        f->width = va_arg(*args, int);
        if (f->width < 0) { f->minus = 1; f->width = -f->width; }
        i++;
    } else {
        while (fmt[i] >= '0' && fmt[i] <= '9')
            f->width = f->width * 10 + (fmt[i++] - '0');
    }
    /* precision: introduced by '.', then '*' or literal digits */
    if (fmt[i] == '.') {
        f->has_precision = 1;
        f->precision = 0;
        i++;
        if (fmt[i] == '*') {
            f->precision = va_arg(*args, int);
            if (f->precision < 0) { f->has_precision = 0; f->precision = -1; }
            i++;
        } else {
            while (fmt[i] >= '0' && fmt[i] <= '9')
                f->precision = f->precision * 10 + (fmt[i++] - '0');
        }
    }
    if (f->minus)
        f->zero = 0;   /* '-' overrides '0' */
    if (f->plus)
        f->space = 0;  /* '+' overrides ' ' */
    return (i);
}

/*
 * dispatch_fmt — call the appropriate print function for a conversion specifier.
 *
 * spec is the character immediately after the parsed flags/width/precision.
 * Each branch pops exactly the argument(s) its specifier expects from args.
 *
 * Unknown specifiers are printed as-is ("%q" → "%q"), matching the behaviour
 * documented for the bonus section.  '%%' writes a single '%' with no va_arg
 * call — advancing the argument cursor for '%%' would corrupt every subsequent
 * specifier silently.
 */
static int  dispatch_fmt(char spec, va_list *args, t_fmt *f)
{
    if (spec == 'c')
        return (tci_print_char(va_arg(*args, int), f));
    if (spec == 's')
        return (tci_print_str(va_arg(*args, char *), f));
    if (spec == 'p')
        return (tci_print_ptr(va_arg(*args, void *)));
    if (spec == 'd' || spec == 'i')
        return (tci_print_signed(va_arg(*args, int), f));
    if (spec == 'u')
        return (tci_print_unsigned(va_arg(*args, unsigned int),
                "0123456789", f));
    if (spec == 'x')
        return (tci_print_unsigned(va_arg(*args, unsigned int),
                "0123456789abcdef", f));
    if (spec == 'X')
        return (tci_print_unsigned(va_arg(*args, unsigned int),
                "0123456789ABCDEF", f));
    if (spec == '%')
        return (tci_putchar_fd('%', 1));
    return (tci_putchar_fd('%', 1) + tci_putchar_fd(spec, 1));
}

/*
 * tci_printf — variadic output function matching libc printf.
 *
 * Walks the format string one character at a time:
 *   - Literal characters are written directly to stdout (fd 1).
 *   - '%' followed by a non-NUL character triggers format parsing and
 *     dispatch.  parse_fmt advances past flags/width/precision; dispatch_fmt
 *     consumes the specifier character and the corresponding va_list argument.
 *   - A '%' at the very end of the string (no following character) is written
 *     as a literal '%' — the loop's else branch handles it.
 *
 * Returns the total number of characters written, matching libc printf.
 */
int     tci_printf(const char *fmt, ...)
{
    va_list  args;
    int      count;
    int      i;
    t_fmt    f;

    va_start(args, fmt);
    count = 0;
    i = 0;
    while (fmt[i]) {
        if (fmt[i] == '%' && fmt[i + 1]) {
            i++;                                        /* skip '%'            */
            i = parse_fmt(fmt, i, &f, &args);          /* parse modifiers     */
            count += dispatch_fmt(fmt[i], &args, &f);  /* write converted arg */
        } else {
            count += tci_putchar_fd(fmt[i], 1);         /* literal character   */
        }
        i++;
    }
    va_end(args);
    return (count);
}
