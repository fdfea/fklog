CC=gcc
CFLAGS=-Wall -O3
EXE=fklog

SRCS=fklog.c
OBJS=$(subst .c,.o,$(SRCS))

all: fklog

fklog: $(OBJS)
	$(CC) $(CFLAGS) -o $(EXE) $(OBJS)

clean:
	rm -f fklog.o fklog

fklog.o: fklog.c

.PHONY: all clean
