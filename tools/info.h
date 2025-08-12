#include <stdbool.h>

void siginfo_set_message(const char *msg, ...);
void siginfo_clear_message(void);
void siginfo_setup_handler(void);
bool siginfo_latch_pending(void);
