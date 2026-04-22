/**
 * @file syscalls.c
 * @author Swan BaiLei27 (https://github.com/BaiLei27)
 * @brief weak syscalls implement
 * @version 0.1
 * @date 2026/04/13/21:04
 *
 * @copyright Copyright (c) 2026 Swan BaiLei27 <SwanBaiLei2Seven@foxmail.com>
 *
 */

// NOLINTBEGIN
/* 与 picolibc 的 -Derrno-function=rvos_errno 配套：全库经 (*rvos_errno()) 访问 errno，
 * 避免 C 侧 TLS/__emutls 与 libstdc++ 的 errno 模型在链接时冲突。 */
#include <picolibc.h>

#include <stddef.h>
#include <stdio.h>
#include <errno.h>
#include <sys/stat.h>

static int rvos_errno_storage;

int *rvos_errno(void)
{
    return &rvos_errno_storage;
}
#include <sys/time.h>
#include <sys/times.h>

extern int __io_putchar(int ch) __attribute__((weak));
extern int __io_getchar(void) __attribute__((weak));

/*
 * libstdc++ 会引用 _impure_ptr 并读 _stderr（rv64 _REENT_SMALL 下偏移 24）。
 * 未开 posix-console 时 picolibc 无 stdin/stdout/stderr 符号，用 FDEV 提供最小 stderr。
 */
typedef struct __file __FILE;

struct _rvos_impure_layout {
    int _errno;
    char _pad[4];
    __FILE *_stdin;
    __FILE *_stdout;
    __FILE *_stderr;
};
_Static_assert(offsetof(struct _rvos_impure_layout, _stderr) == 24,
               "libstdc++ expects _stderr at offset 24");

static int rvos_stdio_putc(char c, FILE *stream)
{
    (void)c;
    (void)stream;
    return 0;
}

static int rvos_stdio_getc_null(FILE *stream)
{
    (void)stream;
    return -1; /* EOF */
}

static struct __file __rvos_stdin_file =
    FDEV_SETUP_STREAM(NULL, rvos_stdio_getc_null, NULL, __SRD);
static struct __file __rvos_stdout_file =
    FDEV_SETUP_STREAM(rvos_stdio_putc, NULL, NULL, __SWR);
static struct __file __rvos_stderr_file =
    FDEV_SETUP_STREAM(rvos_stdio_putc, NULL, NULL, __SWR);

/* picolibc 未启用 posix-console 时不提供这三个符号；libstdc++ 会直接引用 stderr。 */
FILE *const stdin = &__rvos_stdin_file;
FILE *const stdout = &__rvos_stdout_file;
FILE *const stderr = &__rvos_stderr_file;

static struct {
    int _errno;
    char _pad[4];
    __FILE *_stdin;
    __FILE *_stdout;
    __FILE *_stderr;
} _impure_data = {
    0,
    {0},
    &__rvos_stdin_file,
    &__rvos_stdout_file,
    &__rvos_stderr_file,
};

void *_impure_ptr = &_impure_data;

#ifdef __cplusplus
extern "C" {
#endif

    /* 2. 实现 sbrk (动态内存分配) */
    extern char __heap_start; /* 来自链接脚本 */
    extern char __heap_end;

    static char *heap_end= &__heap_start;

    void *sbrk(ptrdiff_t incr)
    {
        char *prev_heap_end;

        if(heap_end + incr > &__heap_end) {
            errno= ENOMEM;
            return (void *)-1;
        }

        prev_heap_end= heap_end;
        heap_end+= incr;
        return prev_heap_end;
    }

    void initialise_monitor_handles() { }

    int _getpid(void) { return 1; }

    int _kill(int pid, int sig)
    {
        (void)pid;
        (void)sig;
        errno= EINVAL;
        return -1;
    }

    void _exit(int status)
    {
        _kill(status, -1);
        while(1) {
        } /* Make sure we hang here */
    }

    __attribute__((weak)) int _read(int file, char *ptr, int len)
    {
        (void)file;

        for(int dateIdx= 0; dateIdx < len; ++dateIdx) {
            *ptr++= __io_getchar();
        }

        return len;
    }

    __attribute__((weak)) int _write(int file, char *ptr, int len)
    {
        (void)file;

        for(int dateIdx= 0; dateIdx < len; ++dateIdx) {
            __io_putchar(*ptr++);
        }
        return len;
    }

    int _close(int file)
    {
        (void)file;
        return -1;
    }

    int _fstat(int file, struct stat *st)
    {
        (void)file;
        st->st_mode= S_IFCHR;
        return 0;
    }

    int _isatty(int file)
    {
        (void)file;
        return 1;
    }

    int _lseek(int file, int ptr, int dir)
    {
        (void)file;
        (void)ptr;
        (void)dir;
        return 0;
    }

    int _open(char *path, int flags, ...)
    {
        (void)path;
        (void)flags;
        /* Pretend like we always fail */
        return -1;
    }

    int _wait(int *status)
    {
        (void)status;
        errno= ECHILD;
        return -1;
    }

    int _unlink(char *name)
    {
        (void)name;
        errno= ENOENT;
        return -1;
    }

    int _times(struct tms *buf)
    {
        (void)buf;
        return -1;
    }

    int _stat(char *file, struct stat *st)
    {
        (void)file;
        st->st_mode= S_IFCHR;
        return 0;
    }

    int _link(char *oldPath, char *newPath)
    {
        (void)oldPath;
        (void)newPath;
        errno= EMLINK;
        return -1;
    }

    int _fork(void)
    {
        errno= EAGAIN;
        return -1;
    }

    int _execve(char *name, char **argv, char **env)
    {
        (void)name;
        (void)argv;
        (void)env;
        errno= ENOMEM;
        return -1;
    }

#ifdef __cplusplus
}
#endif
// NOLINTEND
