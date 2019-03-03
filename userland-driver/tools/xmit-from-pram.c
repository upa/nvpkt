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


#define NETMAP_WITH_LIBS
#include <net/netmap_user.h>


struct body {
	char	*nm_name;	/* netmap port name, e.g, netmap:eth0, vale0:x */
	char 	*nvme_pci;
	int	nblocks;
	__u64	slba;
	int	count;
	int	usleep;
	int	batch_size;
};

struct body body;

static uintptr_t phy_addr(void* virt) {
	int fd;
	long pagesize;
	off_t ret;
	ssize_t rc;
	uintptr_t entry = 0;

	fd = open("/proc/self/pagemap", O_RDONLY);
	if (fd < 0) {
		fprintf(stderr, "open /proc/self/pagemap: %s",
			strerror(errno));
		exit(1);
	}

	pagesize = sysconf(_SC_PAGESIZE);

	ret = lseek(fd, (uintptr_t)virt / pagesize * sizeof(uintptr_t), SEEK_SET);
	if (ret < 0) {
		fprintf(stderr,
			"lseek for /proc/self/pagemap: %s\n", strerror(errno));
		exit(1);
	}


	rc = read(fd, &entry, sizeof(entry));
	if (rc < 1 || entry == 0) {
		fprintf(stderr,
			"read for /proc/self/pagemap: %s\n", strerror(errno));
		exit(1);
	}

	close(fd);

	return (entry & 0x7fffffffffffffULL) * pagesize +
		((uintptr_t)virt) % pagesize;
}

void usage() {
	printf("usage:\n"
	       "    -i NMNAME	 netmap port name to xmit packets\n"
	       "    -c count     number of loop\n"
	       "    -n no write  no overrite on pram\n"
	       "    -u usleep    usleep between tx packets\n"
	       "    -l pktlen    packet length\n"
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
	int ch, n, ret, fd, pktlen = 512;
	int no_write = 0;
	char *buf;
	uintptr_t paddr;
	struct nm_desc *nm_desc;


	body.batch_size = 1;

	while ((ch = getopt(argc, argv, "i:c:u:l:n")) != -1) {
		switch (ch) {
		case 'i':
			body.nm_name = optarg;
			break;
		case 'c':
			body.count = atoi(optarg);
			break;
		case 'u':
			body.usleep = atoi(optarg);
			break;
		case 'n':
			no_write = 1;
			break;
		case 'l':
			pktlen = atoi(optarg);
			break;
		default:
			usage();
			return -1;
		}
	}

	printf("netmap port: %s\n", body.nm_name);
	printf("count:       %d\n", body.count);
	printf("pktlen:      %d\n", pktlen);

	/* open netmap */
	nm_desc = nm_open(body.nm_name, NULL, 0, NULL);
	if (!nm_desc) {
		perror("failed to open netmap");
		return -1;
	}
	
	/* open pram and build packet */
	fd = open("/dev/p2pmmap", O_RDWR);
	if (fd < 0) {
		perror("open /dev/p2pmmap failed");
		ret = -1;
		goto out;
	}

	buf = mmap(0, 4096, PROT_READ | PROT_WRITE,
		   MAP_LOCKED | MAP_SHARED, fd, 0);
	if (buf == MAP_FAILED) {
		perror("mmap failed");
		ret = -1;
		goto out;
	}
	paddr = phy_addr(buf);

	printf("buf:         %p\n", buf);
	printf("paddr:       %lx\n", paddr);

	if (!no_write)
		build_pkt(buf, pktlen);

	/* xmit pakcet from nvme to nic */
	for (n = 0; n < body.count; n++) {
		unsigned int cur;
		struct netmap_ring *txring;

		txring = NETMAP_TXRING(nm_desc->nifp, 0);
		if (nm_ring_empty(txring)) {
			fprintf(stderr, "no empty slot on txring %d\n", 0);
			goto out;
		}

		cur = txring->cur;
		txring->slot[cur].len = pktlen;
		txring->slot[cur].ptr = paddr;
		txring->slot[cur].flags |= NS_INDIRECT;

		cur = nm_ring_next(txring, cur);
		txring->head = txring->cur = cur;

		ret = ioctl(nm_desc->fd, NIOCTXSYNC, NULL);
		if (ret < 0) {
			fprintf(stderr, "failed to NIOCTXSYNC\n");
			goto out;
		}

		usleep(body.usleep);
	}

out:
	nm_close(nm_desc);

	return ret;
}


