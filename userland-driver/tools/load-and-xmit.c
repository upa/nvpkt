/* load-and-xmit.c */

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <poll.h>
#include <net/ethernet.h>
#include <netinet/ip.h>
#include <netinet/udp.h>
#include <arpa/inet.h>


#include <unvme.h>
#include <unvme_vfio.h>

#define NETMAP_WITH_LIBS
#include <net/netmap_user.h>


struct body {
	char	*nm_name;	/* netmap port name, e.g, netmap:eth0, vale0:x */
	char 	*nvme_pci;
	int	nblocks;
	__u64	slba;
	int	count;
	int	usleep;
};

struct body body;

void usage() {
	printf("usage:\n"
	       "    -i NMNAME	netmap port name to xmit packets\n"
	       "    -s SLBA	start logical address block\n"
	       "    -n nblocks	number of logical blocks read\n"
	       "    -p PCIbus   pci bus number of nvme in which packet storead"
	       "    -c count    number of loop\n"
	       "    -u usleep   usleep between tx packets\n"
	       "\n");
}

void build_pkt(char *buf, int len) {

        struct ether_header *eth;
        struct ip *ip;
        struct udphdr *udp;

        eth = (struct ether_header *)buf;
        eth->ether_shost[0] = 0x01;
        eth->ether_shost[1] = 0x02;
        eth->ether_shost[2] = 0x03;             
        eth->ether_shost[3] = 0x04;     
        eth->ether_shost[4] = 0x05;
        eth->ether_shost[5] = 0x06;

        eth->ether_dhost[0] = 0xff;
        eth->ether_dhost[1] = 0xff;
        eth->ether_dhost[2] = 0xff;             
        eth->ether_dhost[3] = 0xff;     
        eth->ether_dhost[4] = 0xff;
        eth->ether_dhost[5] = 0xff;

        eth->ether_type = htons(ETHERTYPE_IP);

        ip = (struct ip*)(eth + 1);
        ip->ip_v        = IPVERSION;
        ip->ip_hl       = 5;
        ip->ip_id       = 0;
        ip->ip_tos      = IPTOS_LOWDELAY;
        ip->ip_len      = htons(len - sizeof(*eth));
        ip->ip_off      = 0;
        ip->ip_ttl      = 16;
        ip->ip_p        = IPPROTO_UDP;
        ip->ip_sum      = 0;
        ip->ip_src.s_addr = inet_addr("10.0.0.2");
        ip->ip_dst.s_addr = inet_addr("10.0.0.1");


        udp = (struct udphdr*)(ip + 1);
        udp->uh_ulen    = htons(len - sizeof(*eth) - sizeof(*ip));
        udp->uh_dport   = htons(60000);
        udp->uh_sport   = htons(60000);
        udp->uh_sum     = 0;
}

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
	int ch, n, i, ret = 0, use_build_pkt = 0;
	char *buf;

	struct nm_desc *nm_desc;

	const unvme_ns_t *unvme;


	while ((ch = getopt(argc, argv, "i:s:n:p:c:u:b")) != -1) {
		switch (ch) {
		case 'i':
			body.nm_name = optarg;
			break;

		case 's':
			body.slba = strtol(optarg, NULL, 0);
			break;

		case 'n':
			body.nblocks = atoi(optarg);
			if (body.nblocks < 0) {
				fprintf(stderr, "invalid nblocks %d\n",
					body.nblocks);
				return -1;
			}
			break;

		case 'p':
			body.nvme_pci = optarg;
			break;

		case 'c':
			body.count = atoi(optarg);
			break;
		case 'b':
			use_build_pkt = 1;
			break;

		case 'u':
			body.usleep = atoi(optarg);
			break;

		default:
			usage();
			return -1;
		}
	}

	printf("netmap port: %s\n", body.nm_name);
	printf("SLBA:        %llu\n", body.slba);
	printf("PCI:         %s\n", body.nvme_pci);
	printf("nBlocks:     %d\n", body.nblocks);
	

	/* open netmap */
	nm_desc = nm_open(body.nm_name, NULL, 0, NULL);
	if (!nm_desc) {
		perror("failed to open netmap");
		return -1;
	}
	
	/* open nvme */
	printf("%s\n", body.nvme_pci);
	unvme = unvme_open(body.nvme_pci);
	if (!unvme) {
		fprintf(stderr, "failed to open nvme device %s\n",
			body.nvme_pci);
		ret = -1;
		goto nvme_out;
	}

	buf = unvme_alloc(unvme, 4096);
	if (!buf) {
		fprintf(stderr, "failed to allocate memory\n");
		ret = -1;
		goto out;
	}

	/* xmit pakcet from nvme to nic */
	for (i = 0; i < body.count; i++) {
		for (n = 0; n < body.nblocks; n++) {
			ret = unvme_read(unvme, 0, buf, body.slba + n, 1);
			if (ret < 0) {
				fprintf(stderr, "failed to unvme_read: %d\n",
					ret);
				goto free_out;
			}
			printf("unvme_read returns %d\n", ret);


			unsigned int cur;
			struct netmap_ring *txring;

			txring = NETMAP_TXRING(nm_desc->nifp, 0);

			if (nm_ring_empty(txring)) {
				fprintf(stderr, "no emtpy slot on txring\n");
				goto free_out;
			}

			cur = txring->cur;
			txring->slot[cur].len = 1024;

			if (!use_build_pkt) {
				txring->slot[cur].ptr = (uintptr_t)buf;
				txring->slot[cur].flags = NS_INDIRECT;
				hexdump(buf, 128);
			} else {
				char *nm_buf;
				nm_buf = NETMAP_BUF(txring, txring->slot[cur].buf_idx);
				build_pkt(nm_buf, 1024);
				hexdump(nm_buf, 128);
			}

			cur = nm_ring_next(txring, cur);
			txring->head = txring->cur = cur;

			ret = ioctl(nm_desc->fd, NIOCTXSYNC, NULL);
			if (ret < 0) {
				fprintf(stderr, "failed to NIOCTXSYNC\n");
				goto free_out;
			}
			printf("txsync %d, txring->head=%u\n",
			       i, txring->head);

			usleep(body.usleep);
		}
	}



free_out:
	unvme_free(unvme, buf);
out:
	unvme_close(unvme);
nvme_out:
	nm_close(nm_desc);

	return ret;
}


