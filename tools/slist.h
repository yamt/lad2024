#if !defined(_TOYWASM_SLIST_H)
#define _TOYWASM_SLIST_H

/*
 * This is a singly-linked queue implementation
 *
 * - the structure is similar to BSD queue.h STAILQ.
 *   re-invented just for NIH.
 */

#include <stdint.h>

#include "platform.h"

struct slist_elem;

struct slist_entry {
        struct slist_elem *next;
};

struct slist_head {
        struct slist_elem *first;
        struct slist_elem **tailnextp;
};

__BEGIN_EXTERN_C

void slist_head_init(struct slist_head *h);
void slist_remove(struct slist_head *h, struct slist_entry *prev,
                  struct slist_entry *e);
void slist_insert_tail(struct slist_head *h, void *elem,
                       struct slist_entry *e);
void slist_insert_head(struct slist_head *h, void *elem,
                       struct slist_entry *e);

__END_EXTERN_C

#if defined(toywasm_typeof)
#define CHECK_TYPE(a, b)                                                      \
        do {                                                                  \
                __unused toywasm_typeof(a) *__dummy = &b;                     \
        } while (0)
#else
#define CHECK_TYPE(a, b)
#endif

#define SLIST_ENTRY(TYPE)                                                     \
        struct {                                                              \
                TYPE *next;                                                   \
        }

#define SLIST_HEAD(TYPE)                                                      \
        struct {                                                              \
                TYPE *first;                                                  \
                TYPE **tailnextp;                                             \
        }

#define SLIST_HEAD_NAMED(TYPE, NAME)                                          \
        struct NAME {                                                         \
                TYPE *first;                                                  \
                TYPE **tailnextp;                                             \
        }

#define SLIST_FOREACH(VAR, HEAD, NAME)                                        \
        for (VAR = SLIST_FIRST(HEAD); VAR != NULL; VAR = SLIST_NEXT(VAR, NAME))

#define _SLIST_NEXT_PTR_TO_ELEM(ELEM, TYPE, NAME)                             \
        (TYPE *)((uintptr_t)(ELEM) - toywasm_offsetof(TYPE, NAME.next))

#define SLIST_FIRST(HEAD) (HEAD)->first
#define SLIST_LAST(HEAD, TYPE, NAME)                                          \
        (LIST_EMPTY(HEAD)                                                     \
                 ? NULL                                                       \
                 : _LIST_NEXT_PTR_TO_ELEM((HEAD)->tailnextp, TYPE, NAME))
#define SLIST_NEXT(VAR, NAME) (VAR)->NAME.next

#define SLIST_EMPTY(HEAD) (SLIST_FIRST(HEAD) == NULL)

#define SLIST_REMOVE(HEAD, PREV, ELEM, NAME)                                  \
        CHECK_TYPE(&(HEAD)->first, (HEAD)->tailnextp);                        \
        CHECK_TYPE((HEAD)->first, (ELEM)->NAME.next);                         \
        if (PREV != NULL) {                                                   \
                CHECK_TYPE((HEAD)->first, (PREV)->NAME.next);                 \
        }                                                                     \
        ctassert(sizeof(*(HEAD)) == sizeof(struct slist_head));               \
        ctassert(sizeof((ELEM)->NAME) == sizeof(struct slist_entry));         \
        slist_remove((struct slist_head *)(HEAD),                             \
                     (struct slist_entry *)&(PREV)->NAME,                     \
                     (struct slist_entry *)&(ELEM)->NAME)

#define SLIST_INSERT_TAIL(HEAD, ELEM, NAME)                                   \
        CHECK_TYPE(&(HEAD)->first, (HEAD)->tailnextp);                        \
        CHECK_TYPE((HEAD)->first, (ELEM)->NAME.next);                         \
        ctassert(sizeof(*(HEAD)) == sizeof(struct slist_head));               \
        ctassert(sizeof((ELEM)->NAME) == sizeof(struct slist_entry));         \
        slist_insert_tail((struct slist_head *)(HEAD), (ELEM),                \
                          (struct slist_entry *)&(ELEM)->NAME)

#define SLIST_INSERT_HEAD(HEAD, ELEM, NAME)                                   \
        CHECK_TYPE(&(HEAD)->first, (HEAD)->tailnextp);                        \
        CHECK_TYPE((HEAD)->first, (ELEM)->NAME.next);                         \
        ctassert(sizeof(*(HEAD)) == sizeof(struct slist_head));               \
        ctassert(sizeof((ELEM)->NAME) == sizeof(struct slist_entry));         \
        slist_insert_head((struct slist_head *)(HEAD), (ELEM),                \
                          (struct slist_entry *)&(ELEM)->NAME)

#define SLIST_HEAD_INIT(HEAD)                                                 \
        ctassert(sizeof(*(HEAD)) == sizeof(struct slist_head));               \
        slist_head_init((struct slist_head *)(HEAD));                         \
        CHECK_TYPE(&(HEAD)->first, (HEAD)->tailnextp)

#endif /* !defined(_TOYWASM_SLIST_H) */