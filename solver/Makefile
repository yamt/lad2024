CFLAGS = -std=c99 -flto=full -O3 -Wall
CLINKFLAGS = -flto=full

CFLAGS += -I../src
VPATH = ../src

OBJS = main.o
OBJS += hash.o
OBJS += rule.o
OBJS += loader.o
OBJS += stages.o
OBJS += list.o

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
