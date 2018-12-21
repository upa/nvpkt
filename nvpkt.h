/* nvpkt.h */

#ifndef _NVPKT_H_
#define _NVPKT_H_


#ifdef __KERNEL__
#include <uapi/linux/nvme_ioctl.h>
#else
#include <asm-generic/ioctl.h>
#include <linux/nvme_ioctl.h>
#endif


/*
 * nvpkt_ioctl 
 */


/* xmit a packet from slba */
struct nvpkt_xmit {
	size_t	len;
	__u64	slba;
	__u64	count;
};

#define NVPKT_IOCTL_SUBMIT_IO	_IOW('N', 0x42, struct nvme_user_io)
#define NVPKT_IOCTL_TXPKT	_IOW('N', 0x43, struct nvpkt_xmit)
#define NVPKT_IOCTL_TXPKT_COUNT	_IOW('N', 0x44, struct nvpkt_xmit)


#endif	/* _NVPKT_H_ */
