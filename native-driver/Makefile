KERNELSRCDIR := /lib/modules/$(shell uname -r)/build
BUILD_DIR := $(shell pwd)
VERBOSE = 0

obj-m := nvpkt.o

ccflags-y :=

all:
	make -C $(KERNELSRCDIR) M=$(BUILD_DIR) V=$(VERBOSE) modules

clean:
	make -C $(KERNELSRCDIR) M=$(BUILD_DIR) clean
