/* xmit-pkt.c */

#include <stdio.h>
#include <fcntl.h>
#include <sys/ioctl.h>

#include "../nvpkt.h"

int main(int argc, char **argv)
{
	int ret;
	int fd;
	struct nvpkt_xmit npr;

	fd = open("/dev/nvpkt", O_RDWR);
	if (fd < 0) {
		perror("open");
		return fd;
	}

	npr.len = 1024;
	npr.slba = 1;

	ret = ioctl(fd, NVPKT_IOCTL_TXPKT, &npr);
	if (ret) {
		perror("ioctl");
	}

	return ret;
}
