/*
 * rANS version of crans.h
 */

#include "rans_encode.h"
#include "rans_probs.h"

#define CRANS_NTABLES RANS_NSYMS
struct crans {
        uint8_t context;
        size_t counts[CRANS_NTABLES][RANS_NSYMS];
        struct rans_probs ps[CRANS_NTABLES];
        unsigned int ntables;
};

struct bitbuf;

void crans_init(struct crans *c);
void crans_update(struct crans *c, const uint8_t *p, size_t len);
void crans_build(struct crans *c);
void crans_encode(struct crans *c, const uint8_t *p, size_t len,
                  struct rans_encode_state *enc, bool need_rans_init,
                  struct bitbuf *bo);

#define CRANS_TABLE_MAX_NELEMS (CRANS_NTABLES * RANS_TABLE_MAX_NELEMS)
void crans_table(const struct crans *c, rans_prob_t *out,
                 rans_prob_t *outsp[CRANS_NTABLES],
                 size_t lensp[CRANS_NTABLES]);
void crans_table_with_trans(const struct crans *c, rans_prob_t *out,
                            rans_sym_t trans[CRANS_NTABLES],
                            rans_prob_t *outsp[CRANS_NTABLES],
                            size_t lensp[CRANS_NTABLES]);
void crans_table_with_shared_trans(const struct crans *ch, rans_prob_t *out,
                                   rans_prob_t *outsp[CRANS_NTABLES],
                                   size_t lensp[CRANS_NTABLES],
                                   rans_sym_t *trans, size_t *ntransp);
