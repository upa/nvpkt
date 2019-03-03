CC = gcc
INCLUDE := 
LDFLAGS := 
LDLIBS	:= -lunvme -pthread
CFLAGS	:= -g -Wall $(INCLUDE)

PROGNAME = load-and-xmit store-pkt dump-nvme bench-nvme xmit-from-pram	\
	   read-pkt

all: $(PROGNAME)

.c.o:
	$(CC) $< -o $@

clean:
	rm -rf *.o
	rm -rf $(PROGNAME)
