#include <stddef.h>
#include <stdio-bufio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

extern void uart_init();
extern void uart_putc(const char ch);
extern void uart_puts(const char *s);

void syscall_putc(const char ch)
{
    uart_putc(ch);
}

void syscall_puts(const char *s)
{
    uart_puts(s);
}

/*
 * picolibc tinystdio + POSIX_IO：必须提供带 bufio 的 stdin/stdout/stderr 强符号，
 * 勿用魔数 FILE*。printf/puts 经 fd 1/2 走到下方 write() → UART。
 */

void _exit(int code) {
    (void)code;
    while (1) {
    }
}

ssize_t write(int fd, const void *buf, size_t count)
{
    const unsigned char *p = (const unsigned char *)buf;

    if ((fd != STDOUT_FILENO && fd != STDERR_FILENO) || buf == NULL) {
        return -1;
    }
    for (size_t i = 0; i < count; ++i) {
        uart_putc((char)p[i]);
    }
    return (ssize_t)count;
}

ssize_t read(int fd, void *buf, size_t count)
{
    (void)fd;
    (void)buf;
    (void)count;
    return 0;
}

off_t lseek(int fd, off_t offset, int whence)
{
    (void)fd;
    (void)offset;
    (void)whence;
    return 0;
}

int close(int fd)
{
    (void)fd;
    return 0;
}

#define STDIN_BUFSZ  32
#define STDOUT_BUFSZ 128
#define STDERR_BUFSZ 1

static char stdin_buf[STDIN_BUFSZ];
static char stdout_buf[STDOUT_BUFSZ];
static char stderr_buf[STDERR_BUFSZ];

static struct __file_bufio __picolibc_stdin =
    FDEV_SETUP_BUFIO(STDIN_FILENO, stdin_buf, STDIN_BUFSZ,
                     read, write, lseek, close, __SRD, 0);

static struct __file_bufio __picolibc_stdout =
    FDEV_SETUP_BUFIO(STDOUT_FILENO, stdout_buf, STDOUT_BUFSZ,
                     read, write, lseek, close, __SWR, __BLBF);

static struct __file_bufio __picolibc_stderr =
    FDEV_SETUP_BUFIO(STDERR_FILENO, stderr_buf, STDERR_BUFSZ,
                     read, write, lseek, close, __SWR, 0);

FILE *const stdin = &__picolibc_stdin.xfile.cfile.file;
FILE *const stdout = &__picolibc_stdout.xfile.cfile.file;
FILE *const stderr = &__picolibc_stderr.xfile.cfile.file;

int getentropy(void *buf, size_t count) {
    (void)buf; (void)count;
    return -1;
}

int fstat(int fd, struct stat *st) { (void)fd; (void)st; return -1; }
int isatty(int fd) { (void)fd; return 1; }