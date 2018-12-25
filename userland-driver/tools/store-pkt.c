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
	int ret;
	char buf[4096];
	char *dst;
	struct ether_header *eth;
	struct ip *ip;
	struct udphdr *udp;

	int len = 1024;
	__u64 slba = 1;
	__u16 nblocks = 1;

	const unvme_ns_t *unvme;


	eth = (struct ether_header *)buf;
	eth->ether_shost[0] = 0x10;
	eth->ether_shost[1] = 0x20;
	eth->ether_shost[2] = 0x30;		
	eth->ether_shost[3] = 0x40;	
	eth->ether_shost[4] = 0x50;
	eth->ether_shost[5] = 0x60;

	eth->ether_dhost[0] = 0xff;
	eth->ether_dhost[1] = 0xff;
	eth->ether_dhost[2] = 0xff;		
	eth->ether_dhost[3] = 0xff;	
	eth->ether_dhost[4] = 0xff;
	eth->ether_dhost[5] = 0xff;

	eth->ether_type = htons(ETHERTYPE_IP);

	ip = (struct ip*)(eth + 1);
	ip->ip_v 	= IPVERSION;
	ip->ip_hl	= 5;
	ip->ip_id	= 0;
	ip->ip_tos	= IPTOS_LOWDELAY;
	ip->ip_len	= htons(len - sizeof(*eth));
	ip->ip_off	= 0;
	ip->ip_ttl	= 16;
	ip->ip_p	= IPPROTO_UDP;
	ip->ip_sum	= 0;
	ip->ip_src.s_addr = inet_addr("45.0.0.2");
	ip->ip_dst.s_addr = inet_addr("45.0.0.1");


	udp = (struct udphdr*)(ip + 1);
	udp->uh_ulen	= htons(len - sizeof(*eth) - sizeof(*ip));
	udp->uh_dport	= htons(60000);
	udp->uh_sport	= htons(60000);
	udp->uh_sum	= 0;

	if (argc < 2) {
		fprintf(stderr, "%s [NVMe PCI BUS]\n", argv[0]);
		return -1;
	}
	unvme = unvme_open(argv[1]);
	if (!unvme) {
		fprintf(stderr, "failed to open nvme device\n");
		return -1;
	}
	
	dst = unvme_alloc(unvme, sizeof(buf));
	if (!dst) {
		fprintf(stderr, "failed to allocate memory from unvme\n");
		unvme_close(unvme);
		return -1;
	}

	memcpy(dst, buf, len);
	/* veriy the data */
	printf("data to be written\n");
	hexdump(dst, 128);

	ret = unvme_write(unvme, 0, dst, slba, nblocks);
	if (ret < 0) {
		fprintf(stderr, "failed to store packet\n");
	}

	unvme_free(unvme, dst);

	char *r = unvme_alloc(unvme, 1024);
	unvme_read(unvme, 0, r, slba, nblocks);
	printf("verify: data writte\n");
	hexdump(r, 128);

	unvme_free(unvme, r);
	unvme_close(unvme);

	return ret;
}




