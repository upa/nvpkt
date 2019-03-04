/* recv-to-pram.c */

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
#define DEBUG_NETMAP_USER
#include <net/netmap_user.h>



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
	printf("\n\n");
}


void usage(char *progname) {
	printf("usage: %s\n"
	       "    -i netmap port\n"
	       "    -n no p2pmem mode\n"
	       "\n", progname);
}

void *nmslot_to_buf(void *base, int ri, int idx, int slot_num) {

	return (base + (slot_num * 2048 * ri) + (2048 * idx));
}

void init_nm_rx_ring_for_phy(void *base, struct nm_desc *nm_desc) {

	int ri;
	unsigned int idx;
	void *buf;
	struct netmap_ring *ring;
	struct netmap_slot *slot;

	for (ri = 0; ri <= nm_desc->last_rx_ring; ri++) {
		ring = NETMAP_RXRING(nm_desc->nifp, ri);
		for (idx = 0; idx < ring->num_slots; idx++) {
			buf = nmslot_to_buf(base, ri, idx,
					    ring->num_slots);
			slot = &ring->slot[idx];
			slot->flags |= NS_PHY_INDIRECT;
			slot->ptr = phy_addr(buf);
			printf("ring %2d, slot %4u, buf %p, ptr %lx\n",
			       ri, idx, buf, slot->ptr);
		}
	}
}

int main(int argc, char **argv) {

	int ch, n, ri, fd;
	int ret = 0;
	int no_p2pmem = 0;
	char *nm_name, *buf;
	uintptr_t paddr = 0;
	struct nm_desc *nm_desc;

	while((ch = getopt(argc, argv, "i:n")) != -1) {
		switch (ch) {
		case 'i':
			nm_name = optarg;
			break;
		case 'n':
			no_p2pmem = 1;
			break;
		default:
			usage(argv[0]);
			return 1;
		}
	}

	/* open netmap */
	nm_desc = nm_open(nm_name, NULL, 0, NULL);
	if (!nm_desc) {
		perror("failed to open netmap");
		return -1;
	}

	/* open p2pmem */
	printf("open netmap port\n");
	fd = open("/dev/p2pmmap", O_RDWR);
	if (fd < 0) {
		perror("failed to open /dev/p2pmmap");
		nm_close(nm_desc);
		return fd;
	}

	/* allocate 64M byte for all slots of all rx rings */
	if (!no_p2pmem) {
		printf("mmap p2pmem\n");
		buf = mmap(0, 67108864, PROT_READ | PROT_WRITE,
			   MAP_LOCKED | MAP_SHARED, fd, 0);
		if (buf == MAP_FAILED) {
			perror("mmap failed");
			nm_close(nm_desc);
			return 1;
		}
		printf("initialize p2pmem\n");
		memset(buf, 0, 67108864);
		paddr = phy_addr(buf);

		/* fill the physical addresses on p2pmem to netmap slot */
		init_nm_rx_ring_for_phy(buf, nm_desc);
		*((char *)nmslot_to_buf(buf, 5, 0, 512)) = 0xaa; // test

		/* XXX
		 *
		 * Flush the rx rings. netmap native drivers allocate
		 * netmap krings and fill descriptor rings with the
		 * packet buffers in the netmap krings. However, the
		 * buffers exist on DRAM. Here makes cur and head go
		 * around the ring. As a result, the physical
		 * addresses in the ptr fields are going to be filled
		 * in the descriptor rings.
		 */
		for (ri = nm_desc->first_rx_ring; ri < nm_desc->last_rx_ring; ri++) {
			struct netmap_ring *ring = NETMAP_RXRING(nm_desc->nifp, ri);
			ring->cur = ring->num_slots - 1;
			ring->head = ring->num_slots - 1;
			ioctl(nm_desc->fd, NIOCRXSYNC, NULL);
		}

	}

	printf("netmap port: %s\n", nm_name);
	printf("vaddr:       %p\n", buf);
	printf("paddr:       %lx\n", paddr);

	while (1) {

		void *pkt;
		unsigned int cur;
		struct netmap_ring *ring;
		struct netmap_slot *slot;
		struct pollfd x = { .fd = nm_desc->fd, .events = POLLIN };

		ret = ioctl(nm_desc->fd, NIOCRXSYNC, NULL);
		if (ret < 0) {
			fprintf(stderr, "failed to NIOCRXSYNC\n");
			goto out;
		}

		ret = poll(&x, 1, -1);
		if (ret < -1) {
			perror("poll");
			goto out;
		}

		for (ri = nm_desc->first_rx_ring; ri <= nm_desc->last_rx_ring; ri++) {

			ring = NETMAP_RXRING(nm_desc->nifp, ri);

			while (!nm_ring_empty(ring)) {

				cur = ring->head;
				slot = &ring->slot[cur];
				pkt = nmslot_to_buf(buf, ri, cur, ring->num_slots);

				if (!no_p2pmem) {
					printf("pkt=%4d, ring=%d, idx=%u len=%u "
					       "pkt=%p paddr=%lx\n",
					       n, ri, cur, slot->len, pkt, phy_addr(pkt));
					hexdump(pkt, slot->len > 128 ? 128 : slot->len);
				} else {

					void *p = NETMAP_BUF(ring, slot->buf_idx);
					printf("netmap buf is %p\n", p);
					printf("head %02x\n", *((char *)p));
					hexdump(p, slot->len > 128 ? 128 : slot->len);
					printf("\n");
				}

				cur = nm_ring_next(ring, cur);
				ring->head = ring->cur = cur;
				n++;
			}
		}
	}
out:
	nm_close(nm_desc);
	return ret;
}
