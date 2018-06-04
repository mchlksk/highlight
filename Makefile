SHELL=/bin/sh
CC=gcc

PREFIX=/usr/local
BINDIR=${PREFIX}/bin

highlight: highlight.o
	$(CC) $(CFLAGS) -o $@ $^

highlight.o: highlight.c ansicolor.h

clean:
	rm -f highlight.o highlight

install: highlight
	install -d $(BINDIR)
	install -s highlight $(BINDIR)/highlight
