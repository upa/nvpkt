/* packet.c */

#define pr_fmt(fmt) KBUILD_MODNAME ": " fmt

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/netdevice.h>
#include <net/net_namespace.h>
#include <linux/miscdevice.h>

#include "nvme.h"

#define NVPKT_VERSION	"0.0.0"

static char *nvme_device = "";
module_param(nvme_device, charp, 0644);
MODULE_PARM_DESC(nvme_device, "target nvme block device name");

static char *net_device = "";
module_param(net_device, charp, 0644);
MODULE_PARM_DESC(net_device, "target network device name");

static char *mdev_path = "nvpkt";
module_param(mdev_path, charp, 0644);
MODULE_PARM_DESC(mdev_path, "nvpkt char dev path");


/*
 * Represents nvme packet generator hiring a pair of NVMe and NIC.
 */
struct nvpkt {
	struct gendisk		*disk;		/* nvme ns block device */
	struct hd_struct	*part;		/* partion of the nvme */

	struct net_device	*netdev;	/* nic */

	struct miscdevice	mdev;		/* char dev of nvpkt */
};
static struct nvpkt nvpkt;


static int nvpkt_open(struct inode *inode, struct file *filp) {
	return 0;
}

static int nvpkt_release(struct inode *inode, struct file *filp) {
	return 0;
}

static ssize_t nvpkt_read(struct file *filp, char __user *buf, size_t count,
			  loff_t *ppos)
{
	return 0;
}

static ssize_t nvpkt_write(struct file *filp, const char __user *buf,
			   size_t count, loff_t *ppos)
{
	return 0;
}

static long nvpkt_ioctl(struct file *filp, unsigned intcmd,  unsigned long arg)
{
	return 0;
}

static struct file_operations nvpkt_fops = {
	.owner		= THIS_MODULE,
	.open		= nvpkt_open,
	.release	= nvpkt_release,
	.read		= nvpkt_read,
	.write		= nvpkt_write,
	.unlocked_ioctl	= nvpkt_ioctl,
	.compat_ioctl	= nvpkt_ioctl,
};



static int __init nvpkt_init_module(void)
{
	int ret, partno;
	dev_t devt;
	struct gendisk *disk;
	struct hd_struct *part;
	struct net_device *dev;

	pr_info("%s (%s) loading...\n", KBUILD_MODNAME, NVPKT_VERSION);
	pr_info("target nvme_device is '%s', target net_device is '%s'\n",
		nvme_device, net_device);

	ret = 0;
	memset(&nvpkt, 0, sizeof(nvpkt));

	/* find the nvme device */
	devt = blk_lookup_devt(nvme_device, 0);
	if (!devt) {
		pr_err("nvme_device '%s' not found\n", nvme_device);
		ret = -ENODEV;
		goto out;
	}

	disk = get_gendisk(devt, &partno);
	if (!disk) {
		pr_err("gendisk for '%s' not found\n", nvme_device);
		ret = -ENODEV;
		goto out;
	}
	
	/* acquire the nvme device */
	part = disk_get_part(disk, partno);
	if (!part) {
		pr_err("failed to acquire partion %d from %s\n",
		       partno, nvme_device);
		ret = -EBUSY;
		goto out;
	}

	pr_info("ok, we found '%s' as device number %d:%d partno %d\n",
		nvme_device, MAJOR(devt), MINOR(devt), partno);
	nvpkt.disk = disk;
	nvpkt.part = part;


	/* find and acquire the nic */
	dev = dev_get_by_name(&init_net, net_device);
	if (!dev) {
		pr_err("net_device '%s' not found\n", net_device);
		ret = -ENODEV;
		goto err_netdev;
	}
	
	pr_info("ok, we found '%s' as network device\n", dev->name);
	nvpkt.netdev = dev;

	/* register misc device for controlling nvpkt */
	nvpkt.mdev.minor = MISC_DYNAMIC_MINOR;
	nvpkt.mdev.fops = &nvpkt_fops;
	nvpkt.mdev.name = mdev_path;
	ret = misc_register(&nvpkt.mdev);
	if (ret < 0) {
		pr_err("failed to register misc device %s\n", mdev_path);
		goto err_mdev;
	}

	return ret;
	
err_mdev:
	dev_put(nvpkt.netdev);

err_netdev:
	disk_put_part(nvpkt.part);
out:
	return ret;
}

static void __exit nvpkt_exit_module(void)
{
	pr_info("%s (%s) unloading...\n", KBUILD_MODNAME, NVPKT_VERSION);

	/* release the nvme device */
	disk_put_part(nvpkt.part);

	/* release the net device */
	dev_put(nvpkt.netdev);

	/* release the misc device */
	misc_deregister(&nvpkt.mdev);
}



module_init(nvpkt_init_module);
module_exit(nvpkt_exit_module);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("upa@haeena.net");
MODULE_VERSION(NVPKT_VERSION);
