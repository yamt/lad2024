CFLAGS = -std=c99 -Wall
CFLAGS = -flto=full -O3 -DNDEBUG
#CFLAGS = -Og -g
# CFLAGS += -Dmap_height=16 -Dmap_width=16
CLINKFLAGS = -flto=full

CFLAGS += -I../src
VPATH = ../src

SOLVER_OBJS = main.o

OBJS = hash.o
OBJS += rule.o
OBJS += loader.o
OBJS += stages.o
OBJS += list.o
OBJS += dump.o
OBJS += node.o
OBJS += solver.o
OBJS += evaluater.o

BINS = solver

.PHONY: all
all: $(BINS)

solver: $(SOLVER_OBJS) $(OBJS)
	$(CC) \
	$(CLINKFLAGS) \
	-o $@ $(SOLVER_OBJS) $(OBJS)

.c.o:
	$(CC) -c $(CFLAGS) -o $@ $<

.PHONY: clean
clean:
	rm -f $(BINS)
	rm -f $(OBJS)
	rm -f $(SOLVER_OBJS)
