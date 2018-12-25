CC = gcc
INCLUDE := -I../include/
LDFLAGS :=
CFLAGS := -g -Wall $(INCLUDE)

PROGNAME = load-pkt xmit-pkt

all: $(PROGNAME)

.c.o:
	$(CC) $< -o $@

clean:
	rm -rf *.o
	rm -rf $(PROGNAME)
