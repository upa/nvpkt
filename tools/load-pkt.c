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

#include "../nvpkt.h"



int main(int argc, char **argv)
{
	int ret;
	char buf[4096];
	struct ether_header *eth;
	struct ip *ip;
	struct udphdr *udp;

	int fd;
	int len = 1024;
	__u64 slba = 1;
	__u16 nblocks = 1;


	eth = (struct ether_header *)buf;
	eth->ether_dhost[0] = 0x01;
	eth->ether_dhost[1] = 0x02;
	eth->ether_dhost[2] = 0x03;		
	eth->ether_dhost[3] = 0x04;	
	eth->ether_dhost[4] = 0x05;
	eth->ether_dhost[5] = 0x06;

	eth->ether_shost[0] = 0x0a;
	eth->ether_shost[1] = 0x0b;
	eth->ether_shost[2] = 0x0c;		
	eth->ether_shost[3] = 0x0d;	
	eth->ether_shost[4] = 0x0e;
	eth->ether_shost[5] = 0x0f;

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
	ip->ip_dst.s_addr = inet_addr("10.0.0.1");
	ip->ip_src.s_addr = inet_addr("10.0.0.2");


	udp = (struct udphdr*)(ip + 1);
	udp->uh_ulen	= htons(len - sizeof(*eth) - sizeof(*ip));
	udp->uh_dport	= 60000;
	udp->uh_sport	= 60000;
	udp->uh_sum	= 0;


	/* prepare nvme_user_io */
	struct nvme_user_io io;
	memset(&io, 0, sizeof(io));
	io.opcode	= 0x01;	/* write */
	io.nblocks	= nblocks;
	io.addr		= (__u64)(uintptr_t)buf;
	io.slba		= slba;


	fd = open("/dev/nvpkt", O_RDWR);
	if (fd < 0) {
		perror("open");
		return fd;
	}

	ret = ioctl(fd, NVPKT_IOCTL_SUBMIT_IO, &io);
	if (ret) {
		perror("ioctl");
	}
	

	return ret;
}




