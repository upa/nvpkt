/* load-pkt.c */

#include <stdio.h>
#include <string.h>
#include <net/ethernet.h>
#include <netinet/ip.h>
#include <netinet/udp.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/nvme_ioctl.h>

#include <unvme.h>

void hexdump(void *buf, int len)
{
        int n;
        unsigned char *p = buf;

        printf("Hex dump\n");

        for (n = 0; n < len; n++) {
                printf("%02x", p[n]);

                if ((n + 1) % 2 == 0)
                        printf(" ");
                if ((n + 1) % 32 == 0)
                        printf("\n");
        }
        printf("\n");

}

int main(int argc, char **argv)
{
	int ret = 0;
	char *dst;

	const unvme_ns_t *unvme;

	if (argc < 2) {
		fprintf(stderr, "%s [NVMe PCI BUS]\n", argv[0]);
		return -1;
	}

        unvme = unvme_open(argv[1]);
        if (!unvme) {
                fprintf(stderr, "failed to open nvme device\n");
                return -1;
        }
        

	dst = unvme_alloc_pram(unvme, 4096);
	//dst = unvme_alloc(unvme, 4096);
	if (!dst) {
		fprintf(stderr, "failed to allocate memory from unvme\n");
		unvme_close(unvme);
		return -1;
	}
	printf("!!! dst is %p\n", dst);

	//build_pkt(dst);
	/* veriy the data */
	printf("data to be written\n");
	hexdump(dst, 128);

	return ret;
}




