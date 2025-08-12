#include <signal.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "info.h"

static char g_msg_buf[128] = "none\n";
static const char *g_msg;
static sig_atomic_t g_pending;

void
siginfo_set_message(const char *fmt, ...)
{
        sigset_t set;
        sigset_t oset;
        va_list ap;
        va_start(ap, fmt);
        sigemptyset(&set);
        sigaddset(&set, SIGINFO);
        sigprocmask(SIG_BLOCK, &set, &oset);
        vsnprintf(g_msg_buf, sizeof(g_msg_buf), fmt, ap);
        g_msg = g_msg_buf;
        sigprocmask(SIG_SETMASK, &oset, NULL);
        va_end(ap);
        g_pending = false;
}

void
siginfo_clear_message(void)
{
        g_msg = NULL;
}

static void
handler(int sig)
{
        if (g_msg != NULL) {
                write(STDERR_FILENO, g_msg, strlen(g_msg));
        } else {
                g_pending = true;
        }
}

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
        signal(SIGINFO, handler);
}
