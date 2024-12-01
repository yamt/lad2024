#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "slist.h"

void
slist_remove(struct slist_head *h, struct slist_entry *prev,
             struct slist_entry *e)
{
        assert(h->first != NULL);
        if (prev == NULL) {
                /* removing the first entry */
                h->first = e->next;
                if (e->next == NULL) {
                        /* removing the only entry */
                        h->tailnextp = &h->first;
                }
        } else {
                prev->next = e->next;
                if (h->tailnextp == &e->next) {
                        /* removing the last entry */
                        assert(e->next == NULL);
                        h->tailnextp = &prev->next;
                }
        }
}

void
slist_insert_tail(struct slist_head *h, void *elem, struct slist_entry *e)
{
        *h->tailnextp = elem;
        h->tailnextp = &e->next;
        e->next = NULL;
        assert(h->first != NULL);
}

void
slist_insert_head(struct slist_head *h, void *elem, struct slist_entry *e)
{
        if (h->first == NULL) {
                h->tailnextp = &e->next;
        }
        e->next = h->first;
        h->first = elem;
        assert(h->first != NULL);
}

void
slist_head_init(struct slist_head *h)
{
        h->first = NULL;
        h->tailnextp = &h->first;
}
