#include <stdbool.h>

/*
 * initialization
 */
void siginfo_setup_handler(void);

/*
 * set/clear the message for SIGINFO
 */
void siginfo_set_message(const char *msg, ...);
void siginfo_clear_message(void);

/*
 * an alternative method for performance sensitive places,
 * where frequent siginfo_set_message is not desirable.
 */
bool siginfo_latch_pending(void);
