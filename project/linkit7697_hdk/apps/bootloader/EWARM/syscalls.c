/*
#include <sys/stat.h>

#ifdef BL_FOTA_DEBUG
#include "hw_uart.h"

int printf (__const char *__restrict __format, ...)
{
    hw_uart_printf((char *)__format);
     return 0;
}
#endif
*/
extern int __io_putchar(int ch) __attribute__((weak));
extern int __io_getchar(void) __attribute__((weak));

int __errno;

int _close(int file)
{
    return 0;
}

int _fstat(int file, struct stat *st)
{
    return 0;
}

int _isatty(int file)
{
    return 1;
}

int _lseek(int file, int ptr, int dir)
{
    return 0;
}

int _open(const char *name, int flags, int mode)
{
    return -1;
}

int _read(int file, char *ptr, int len)
{
    return 0;
    int DataIdx;

    for (DataIdx = 0; DataIdx < len; DataIdx++) {
        __io_putchar('+');
        *ptr++ = __io_getchar();
        __io_putchar('-');
    }

    return len;
}

caddr_t _sbrk_r(struct _reent *r, size_t nbytes)
{
    extern char end;       /* Defined by linker */
    static char *heap_end; /* Previous end of heap or 0 if none */
    char        *prev_heap_end;
    char        *stack;

    __asm volatile("MRS %0, msp\n" : "=r"(stack));

    if (0 == heap_end) {
        heap_end = &end; /* Initialize first time round */
    }

    prev_heap_end = heap_end;

    if (stack < (prev_heap_end + nbytes)) {
     /* heap would overlap the current stack depth.
        * Future:  use sbrk() system call to make simulator grow memory beyond
        * the stack and allocate that
        */
        //errno = ENOMEM;
        return (char *) - 1;
    }
    heap_end += nbytes;

    return (caddr_t) prev_heap_end;
}

int _write(int file, char *ptr, int len)
{
    int DataIdx;

    for (DataIdx = 0; DataIdx < len; DataIdx++) {
        __io_putchar(*ptr++);
    }
    return len;
}

pid_t _getpid()
{
    return 0;
}

void _exit(int status)
{
    (void)status;
    while (1);
}

int _kill(int pid, int sig)
{
    (void)pid;
    (void)sig;
    return -1;
}

