/* xmit-pkt.c */

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/ioctl.h>

#include "../nvpkt.h"

int main(int argc, char **argv)
{
	int ret;
	int fd;
	unsigned long count = 0;
	struct nvpkt_xmit npr;

	if (argc > 1)
		count = atoi(argv[1]);

	fd = open("/dev/nvpkt", O_RDWR);
	if (fd < 0) {
		perror("open");
		return fd;
	}

	npr.len = 1024;
	npr.slba = 1;
	npr.count = count;

	if (!count)
		ret = ioctl(fd, NVPKT_IOCTL_TXPKT, &npr);
	else
		ret = ioctl(fd, NVPKT_IOCTL_TXPKT_COUNT, &npr);

	if (ret) {
		perror("ioctl");
	}

	return ret;
}
