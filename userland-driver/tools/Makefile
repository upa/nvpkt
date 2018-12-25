CC = gcc
INCLUDE := 
LDFLAGS := 
LDLIBS	:= -lunvme
CFLAGS	:= -g -Wall $(INCLUDE)

PROGNAME = load-and-xmit store-pkt dump-nvme

all: $(PROGNAME)

.c.o:
	$(CC) $< -o $@

clean:
	rm -rf *.o
	rm -rf $(PROGNAME)
