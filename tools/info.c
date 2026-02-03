#define _GNU_SOURCE
#include <errno.h>
#include <signal.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "info.h"

static char g_msg_buf[128] = "none\n";
static const char *g_msg;
static sig_atomic_t g_pending;

void
siginfo_set_message(const char *fmt, ...)
{
        int ret;
        sigset_t set;
        sigset_t oset;
        va_list ap;
        va_start(ap, fmt);
        sigemptyset(&set);
#if defined(SIGINFO)
        sigaddset(&set, SIGINFO);
#endif
        ret = sigprocmask(SIG_BLOCK, &set, &oset);
        if (ret == -1) {
                fprintf(stderr, "sigprocmask failed with %d (%s)\n", errno,
                        strerror(errno));
                exit(1);
        }
        ret = vsnprintf(g_msg_buf, sizeof(g_msg_buf), fmt, ap);
        if (ret == -1) {
                fprintf(stderr, "vsnprintf failed with %d (%s)\n", errno,
                        strerror(errno));
                exit(1);
        }
        g_msg = g_msg_buf;
        ret = sigprocmask(SIG_SETMASK, &oset, NULL);
        if (ret == -1) {
                fprintf(stderr, "sigprocmask failed with %d (%s)\n", errno,
                        strerror(errno));
                exit(1);
        }
        va_end(ap);
        g_pending = false;
}

void
siginfo_clear_message(void)
{
        g_msg = NULL;
}

#if defined(SIGINFO)
static void
handler(int sig)
{
        if (g_msg != NULL) {
                write(STDERR_FILENO, g_msg, strlen(g_msg));
        } else {
                g_pending = true;
        }
}
#endif

bool
siginfo_latch_pending(void)
{
        bool p = g_pending;
        g_pending = false;
        return p;
}

void
siginfo_setup_handler(void)
{
#if defined(SIGINFO)
        signal(SIGINFO, handler);
#endif
}
