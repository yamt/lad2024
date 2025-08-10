#include <signal.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "info.h"

static char g_msg[128] = "none\n";

void
setprocinfo(const char *fmt, ...)
{
        sigset_t set;
        sigset_t oset;
        va_list ap;
        va_start(ap, fmt);
        sigemptyset(&set);
        sigaddset(&set, SIGINFO);
        sigprocmask(SIG_BLOCK, &set, &oset);
        vsnprintf(g_msg, sizeof(g_msg), fmt, ap);
        sigprocmask(SIG_SETMASK, &oset, NULL);
        va_end(ap);
}

static void
handler(int sig)
{
        write(STDERR_FILENO, g_msg, strlen(g_msg));
}

void
setup_handler(void)
{
        signal(SIGINFO, handler);
}
