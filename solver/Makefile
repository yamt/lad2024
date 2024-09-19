CFLAGS = -std=c99 -Wall
CFLAGS = -flto=full -O3 -DNDEBUG
#CFLAGS = -Og -g
# CFLAGS += -Dheight=16 -Dwidth=16
CLINKFLAGS = -flto=full

CFLAGS += -I../src
VPATH = ../src

OBJS = main.o
OBJS += hash.o
OBJS += rule.o
OBJS += loader.o
OBJS += stages.o
OBJS += list.o
OBJS += dump.o

BIN = solver

.PHONY: all
all: $(BIN)

$(BIN): $(OBJS)
	$(CC) \
	$(CLINKFLAGS) \
	-o $@ $(OBJS)

.c.o:
	$(CC) -c $(CFLAGS) -o $@ $<

.PHONY: clean
clean:
	rm -f $(BIN)
	rm -f $(OBJS)
