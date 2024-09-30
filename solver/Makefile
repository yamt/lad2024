CFLAGS = -std=c99 -Wall
CFLAGS = -flto=full -O3 -DNDEBUG
#CFLAGS = -Og -g
# CFLAGS += -Dmap_height=16 -Dmap_width=16
# CFLAGS += -Dmap_height=10 -Dmap_width=10
CLINKFLAGS = -flto=full

CFLAGS += -I../src
VPATH = ../src

SOLVER_OBJS = main.o

GENERATOR_OBJS = generate.o

OBJS = hash.o
OBJS += rule.o
OBJS += loader.o
OBJS += stages.o
OBJS += list.o
OBJS += maputil.o
OBJS += bb.o
OBJS += dump.o
OBJS += node.o
OBJS += rng.o
OBJS += solver.o
OBJS += simplify.o
OBJS += evaluater.o

BINS = solver generator

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

.c.o:
	$(CC) -c $(CFLAGS) -o $@ $<

.PHONY: clean
clean:
	rm -f $(BINS)
	rm -f $(OBJS)
	rm -f $(SOLVER_OBJS)
	rm -f $(GENERATOR_OBJS)
