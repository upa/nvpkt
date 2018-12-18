/* packet.c */

#define pr_fmt(fmt) KBUILD_MODNAME ": " fmt

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/netdevice.h>
#include <net/net_namespace.h>

#include "nvme.h"

#define NVME_PKT_VERSION	"0.0.0"

static char *nvme_device = "";
module_param(nvme_device, charp, 0644);
MODULE_PARM_DESC(nvme_device, "target nvme block device name");

static char *net_device = "";
module_param(net_device, charp, 0644);
MODULE_PARM_DESC(net_device, "target network device name");

/*
 * Represents nvme packet generator hiring a pair of NVMe and NIC.
 */
struct nvme_pkt {
	struct gendisk		*disk;		/* nvme ns block device */
	struct hd_struct	*part;		/* partion of the nvme */

	struct net_device	*netdev;	/* nic */
};
static struct nvme_pkt nvmep;


static int __init nvme_pkt_init_module(void)
{
	int ret = 0, partno;
	dev_t devt;
	struct gendisk *disk;
	struct hd_struct *part;
	struct net_device *dev;

	pr_info("%s (%s) loading...\n", KBUILD_MODNAME, NVME_PKT_VERSION);
	pr_info("target nvme_device is '%s', target net_device is '%s'\n",
		nvme_device, net_device);

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
	nvmep.disk = disk;
	nvmep.part = part;


	/* find and acquire the nic */
	dev = dev_get_by_name(&init_net, net_device);
	if (!dev) {
		pr_err("net_device '%s' not found\n", net_device);
		ret = -ENODEV;
		goto err_netdev;
	}
	
	pr_info("ok, we found '%s' as network device\n", dev->name);
	nvmep.netdev = dev;

	return ret;
	
err_netdev:
	disk_put_part(nvmep.part);
out:
	return ret;
}

static void __exit nvme_pkt_exit_module(void)
{
	pr_info("%s (%s) unloading...\n", KBUILD_MODNAME, NVME_PKT_VERSION);

	/* release the nvme device */
	disk_put_part(nvmep.part);

	/* release the net device */
	dev_put(nvmep.netdev);
}



module_init(nvme_pkt_init_module);
module_exit(nvme_pkt_exit_module);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("upa@haeena.net");
MODULE_VERSION(NVME_PKT_VERSION);
