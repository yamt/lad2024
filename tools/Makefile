CFLAGS = -std=c99 -Wall -Werror
CFLAGS += -MMD -MP
CFLAGS += -fshort-enums
#CFLAGS += -flto=full -O3 -DNDEBUG -Wno-unused-variable
CFLAGS += -flto=full -O3 -Wno-unused-variable
#CFLAGS += -Og -g -DLIST_DISABLE_HEAVY_CHECK
# CFLAGS += -Dmap_height=16 -Dmap_width=16
# CFLAGS += -Dmap_height=10 -Dmap_width=10
#CFLAGS += -Dmap_height=20 -Dmap_width=6
#CFLAGS += -Dmap_height=6 -Dmap_width=20
#CFLAGS += -Dmap_height=12 -Dmap_width=12
# CFLAGS += -DMAX_ITERATIONS=1000000000
# CFLAGS += -DMAX_ITERATIONS=4294967295
# CFLAGS += -DMEMLIMIT_GB=10
# CFLAGS += -DMEMLIMIT_GB=11
#CFLAGS += -DMEMLIMIT_GB=2
#CFLAGS += -DMEMLIMIT_GB=1
#CFLAGS += -DMEMLIMIT_GB=10
CFLAGS += -DMEMLIMIT_GB=8
CFLAGS += -DSMALL_NODE
CFLAGS += -DNODE_KEEP_HASH
CFLAGS += -DHUFF_NSYMS=512

# XXX a bit difficult to tune manually.
# maybe we should somehow make it automatic.
CFLAGS += -DUSE_BLOOM_FILTER
#CFLAGS += -DBLOOM_FILTER_SIZE=400000000
CFLAGS += -DBLOOM_FILTER_SIZE=1600000000
CFLAGS += -DBLOOM_FILTER_K=4

CLINKFLAGS = -flto=full
#CLINKFLAGS += -g

CFLAGS += -I../src
VPATH = ../src

SOLVER_OBJS = main.o

GENERATOR_OBJS = generate.o

REFINER_OBJS = refine_main.o

DUMPER_OBJS = dump_main.o

RNG_TEST_OBJS = rng_test.o

HUFF_TEST_OBJS = huff_test.o

CHUFF_TEST_OBJS = chuff_test.o

LZ_TEST_OBJS = lz_test.o

LZHUFF_TEST_OBJS = lzhuff_test.o

HASH_TEST_OBJS = hash_test.o

ANALYZE_TEST_OBJS = analyze_test.o

STAGE_UNIQ_OBJS = stage_uniq.o

PACK_STAGES_OBJS = pack_stages.o

OBJS = hash.o
OBJS += rule.o
OBJS += loader.o
OBJS += stages.o
OBJS += list.o
OBJS += slist.o
OBJS += maputil.o
OBJS += bb.o
OBJS += bloom_filter.o
OBJS += dump.o
OBJS += draw.o
OBJS += bitbuf.o
OBJS += chuff.o
OBJS += bitin.o
OBJS += lz.o
OBJS += lzhuff.o
OBJS += huff.o
OBJS += huff_debug.o
OBJS += lz_decode.o
OBJS += lzhuff_decode.o
OBJS += huff_decode.o
OBJS += chuff_decode.o
OBJS += node.o
OBJS += pool.o
OBJS += info.o
OBJS += item_cache.o
OBJS += rng.o
OBJS += sha256.o
OBJS += solver.o
OBJS += analyze.o
OBJS += simplify.o
OBJS += evaluater.o
OBJS += refine.o
OBJS += validate.o

ALL_OBJS += ${OBJS}
ALL_OBJS += ${SOLVER_OBJS}
ALL_OBJS += ${GENERATOR_OBJS}
ALL_OBJS += ${REFINER_OBJS}
ALL_OBJS += ${DUMPER_OBJS}
ALL_OBJS += ${RNG_TEST_OBJS}
ALL_OBJS += ${HUFF_TEST_OBJS}
ALL_OBJS += ${CHUFF_TEST_OBJS}
ALL_OBJS += ${LZ_TEST_OBJS}
ALL_OBJS += ${LZHUFF_TEST_OBJS}
ALL_OBJS += ${HASH_TEST_OBJS}
ALL_OBJS += ${ANALYZE_TEST_OBJS}
ALL_OBJS += ${STAGE_UNIQ_OBJS}
ALL_OBJS += ${PACK_STAGES_OBJS}

BINS = solver generator refiner dumper rng_test huff_test chuff_test lz_test lzhuff_test hash_test analyze_test stage_uniq pack_stages

.PHONY: all
all: $(BINS)

solver: $(SOLVER_OBJS) $(OBJS)
	$(CC) \
	$(CLINKFLAGS) \
	-o $@ $(SOLVER_OBJS) $(OBJS)

generator: $(GENERATOR_OBJS) $(OBJS)
	$(CC) \
	$(CLINKFLAGS) \
	-o $@ $(GENERATOR_OBJS) $(OBJS)

refiner: $(REFINER_OBJS) $(OBJS)
	$(CC) \
	$(CLINKFLAGS) \
	-o $@ $(REFINER_OBJS) $(OBJS)

dumper: $(DUMPER_OBJS) $(OBJS)
	$(CC) \
	$(CLINKFLAGS) \
	-o $@ $(DUMPER_OBJS) $(OBJS)

rng_test: $(RNG_TEST_OBJS) $(OBJS)
	$(CC) \
	$(CLINKFLAGS) \
	-o $@ $(RNG_TEST_OBJS) $(OBJS)

huff_test: $(HUFF_TEST_OBJS) $(OBJS)
	$(CC) \
	$(CLINKFLAGS) \
	-o $@ $(HUFF_TEST_OBJS) $(OBJS)

chuff_test: $(CHUFF_TEST_OBJS) $(OBJS)
	$(CC) \
	$(CLINKFLAGS) \
	-o $@ $(CHUFF_TEST_OBJS) $(OBJS)

lz_test: $(LZ_TEST_OBJS) $(OBJS)
	$(CC) \
	$(CLINKFLAGS) \
	-o $@ $(LZ_TEST_OBJS) $(OBJS)

lzhuff_test: $(LZHUFF_TEST_OBJS) $(OBJS)
	$(CC) \
	$(CLINKFLAGS) \
	-o $@ $(LZHUFF_TEST_OBJS) $(OBJS)

hash_test: $(HASH_TEST_OBJS) $(OBJS)
	$(CC) \
	$(CLINKFLAGS) \
	-o $@ $(HASH_TEST_OBJS) $(OBJS)

analyze_test: $(ANALYZE_TEST_OBJS) $(OBJS)
	$(CC) \
	$(CLINKFLAGS) \
	-o $@ $(ANALYZE_TEST_OBJS) $(OBJS)

stage_uniq: $(STAGE_UNIQ_OBJS) $(OBJS)
	$(CC) \
	$(CLINKFLAGS) \
	-o $@ $(STAGE_UNIQ_OBJS) $(OBJS)

pack_stages: $(PACK_STAGES_OBJS) $(OBJS)
	$(CC) \
	$(CLINKFLAGS) \
	-o $@ $(PACK_STAGES_OBJS) $(OBJS)

.c.o:
	$(CC) -c $(CFLAGS) -o $@ $<

.PHONY: regen
regen: pack_stages
	./pack_stages > ../src/hstages.c

.PHONY: clean
clean:
	rm -f $(BINS)
	rm -f $(ALL_OBJS)

DEPS = $(ALL_OBJS:.o=.d)

-include $(DEPS)
