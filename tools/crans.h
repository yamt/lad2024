/*
 * rANS version of crans.h
 */

#include "rans_probs.h"
#include "rans_encode.h"

#define CRANS_NTABLES NSYMS
struct crans {
        uint8_t context;
        struct rans_encode_state enc;
        struct rans_probs ps[CRANS_NTABLES];
};

struct byteout;

void crans_init(struct crans *c);
void crans_update(struct crans *c, const uint8_t *p, size_t len);
void crans_build(struct crans *c);
void crans_encode(struct crans *c, const uint8_t *p, size_t len,
                  struct byteout *bo);

#define CRANS_TABLE_MAX_NELEMS (CRANS_NTABLES * RANS_TABLE_MAX_NELEMS)
void crans_table(const struct crans *c, prob_t *out,
                 prob_t *outsp[CRANS_NTABLES], size_t lensp[CRANS_NTABLES]);
