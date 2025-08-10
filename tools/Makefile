CFLAGS = -std=c99 -Wall
CFLAGS += -MMD -MP
CFLAGS += -fshort-enums
CFLAGS += -flto=full -O3 -DNDEBUG -Wno-unused-variable
#CFLAGS += -Og -g -DLIST_DISABLE_HEAVY_CHECK
# CFLAGS += -Dmap_height=16 -Dmap_width=16
# CFLAGS += -Dmap_height=10 -Dmap_width=10
# CFLAGS += -DMAX_ITERATIONS=1000000000
# CFLAGS += -DMAX_ITERATIONS=4294967295
# CFLAGS += -DMEMLIMIT_GB=10
# CFLAGS += -DMEMLIMIT_GB=11
#CFLAGS += -DMEMLIMIT_GB=2
CFLAGS += -DMEMLIMIT_GB=1
#CFLAGS += -DMEMLIMIT_GB=10
CFLAGS += -DSMALL_NODE
CFLAGS += -DNODE_KEEP_HASH
CLINKFLAGS = -flto=full

CFLAGS += -I../src
VPATH = ../src

SOLVER_OBJS = main.o

GENERATOR_OBJS = generate.o

REFINER_OBJS = refine_main.o

RNG_TEST_OBJS = rng_test.o

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
OBJS += dump.o
OBJS += draw.o
OBJS += huff.o
OBJS += huff_decode.o
OBJS += node.o
OBJS += pool.o
OBJS += info.o
OBJS += item_cache.o
OBJS += rng.o
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
ALL_OBJS += ${RNG_TEST_OBJS}
ALL_OBJS += ${HASH_TEST_OBJS}
ALL_OBJS += ${ANALYZE_TEST_OBJS}
ALL_OBJS += ${STAGE_UNIQ_OBJS}
ALL_OBJS += ${PACK_STAGES_OBJS}

BINS = solver generator refiner rng_test hash_test analyze_test stage_uniq pack_stages

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

rng_test: $(RNG_TEST_OBJS) $(OBJS)
	$(CC) \
	$(CLINKFLAGS) \
	-o $@ $(RNG_TEST_OBJS) $(OBJS)

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
