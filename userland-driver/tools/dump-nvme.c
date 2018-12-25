/* load-pkt.c */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

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

void usage(void)
{
	printf("usage:\n"
	       "    -p PCIbus\n"
	       "    -s SLBA\n"
	       "    -n Nblocks\n");
}

int main(int argc, char **argv)
{
	int ch;
	char *pci;
	u64 slba;
	u64 nblocks;

	char *buf = NULL;
	const unvme_ns_t *unvme = NULL;

	while ((ch = getopt(argc, argv, "p:s:n:")) != -1) {
		switch(ch) {
		case 'p':
			pci = optarg;
			break;
		case 's':
			slba = strtol(optarg, NULL, 0);
			break;

		case 'n':
			nblocks = strtol(optarg, NULL, 0);
			break;
		default:
			usage();
			return -1;
		}
	}
	      


	unvme = unvme_open(pci);
	if (!unvme) {
		printf("failed to open %s\n", pci);
		return -1;
	}

	buf = unvme_alloc(unvme, nblocks * 512);
	if (!buf) {
		printf("failed to alloc buf\n");
		goto out;
	}

	unvme_read(unvme, 0, buf, slba, nblocks);
	hexdump(buf, nblocks * 512);
	unvme_free(unvme, buf);

out:
	unvme_close(unvme);
	return 0;
}




