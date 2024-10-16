CFLAGS = -std=c99 -Wall
CFLAGS += -flto=full -O3 -DNDEBUG -Wno-unused-variable
#CFLAGS += -Og -g
# CFLAGS += -Dmap_height=16 -Dmap_width=16
# CFLAGS += -Dmap_height=10 -Dmap_width=10
CLINKFLAGS = -flto=full

CFLAGS += -I../src
VPATH = ../src

SOLVER_OBJS = main.o

GENERATOR_OBJS = generate.o

REFINER_OBJS = refine_main.o

RNG_TEST_OBJS = rng_test.o

STAGE_UNIQ_OBJS = stage_uniq.o

OBJS = hash.o
OBJS += rule.o
OBJS += loader.o
OBJS += stages.o
OBJS += list.o
OBJS += maputil.o
OBJS += bb.o
OBJS += dump.o
OBJS += draw.o
OBJS += node.o
OBJS += pool.o
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
ALL_OBJS += ${RNG_TEST_OBJS}
ALL_OBJS += ${STAGE_UNIQ_OBJS}

BINS = solver generator refiner rng_test stage_uniq

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

stage_uniq: $(STAGE_UNIQ_OBJS) $(OBJS)
	$(CC) \
	$(CLINKFLAGS) \
	-o $@ $(STAGE_UNIQ_OBJS) $(OBJS)

.c.o:
	$(CC) -c $(CFLAGS) -o $@ $<

.PHONY: clean
clean:
	rm -f $(BINS)
	rm -f $(ALL_OBJS)
