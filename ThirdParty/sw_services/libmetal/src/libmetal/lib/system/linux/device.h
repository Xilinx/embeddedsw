#ifndef __METAL_LINUX_DEVICE__H__
#define __METAL_LINUX_DEVICE__H__

#include <metal/device.h>
#include <metal/sys.h>
#include <metal/utilities.h>
#include <metal/irq.h>
#include <metal/shmem.h>
#include <sys/ioctl.h>
#include <stdbool.h>
#include <libudev.h>

#ifdef __cplusplus
extern "C" {
#endif

#define MAX_DRIVERS	64

struct linux_bus;
struct linux_device;

struct linux_driver {
	const char		*drv_name;
	const char		*mod_name;
	const char		*cls_name;
	bool			is_drv_ready;
	int			(*dev_open)(struct linux_bus *lbus,
					    struct linux_device *ldev);
	void			(*dev_close)(struct linux_bus *lbus,
					     struct linux_device *ldev);
	void			(*dev_irq_ack)(struct linux_bus *lbus,
					     struct linux_device *ldev,
					     int irq);
	int			(*dev_dma_map)(struct linux_bus *lbus,
						struct linux_device *ldev,
						uint32_t dir,
						struct metal_sg *sg_in,
						int nents_in,
						struct metal_sg *sg_out);
	void			(*dev_dma_unmap)(struct linux_bus *lbus,
						struct linux_device *ldev,
						uint32_t dir,
						struct metal_sg *sg,
						int nents);
};

struct linux_bus {
	struct metal_bus	bus;
	const char		*bus_name;
	struct linux_driver	drivers[MAX_DRIVERS];
};

struct linux_device_shm_io_region {
	struct metal_io_region io;
	metal_phys_addr_t phys;
};

struct linux_device {
	struct metal_device		device;
	char				dev_name[PATH_MAX];
	char				dev_path[PATH_MAX];
	char				cls_path[PATH_MAX];
	char				sys_path[PATH_MAX];
	metal_phys_addr_t		region_phys[METAL_MAX_DEVICE_REGIONS];
	struct linux_driver		*ldrv;
	struct udev			*udev;
	struct udev_device		*udev_device;
	int				fd;
	/* Store the dma addressing capability of device */
	int				dma_cap;
	void				*priv_data;
};

/* API for initializing the iova allocator */
extern void metal_iova_init();
/* Set the device's private data */
extern void metal_device_set_pdata(struct linux_device *device, void *pdata);
/* Set the device's DMA addressing capability limit */
extern void metal_device_set_dmacap(struct metal_device *device, int val);
/* Get the device's DMA addressing capability limit */
extern int metal_device_get_dmacap(struct metal_device *device);
/* API for writing a string cmd_str to a file cmd_path (in sysfs) */
extern int metal_linux_exec_cmd(const char *cmd_str, char *cmd_path);
/* Find and return device name based on the given device address */
extern int metal_devname_from_addr(unsigned long addr, char *dev_name);
/* API for checking if the file mentioned in path available or not */
extern int metal_check_file_available(char *path);

#ifdef __cplusplus
}
#endif

#endif /* __METAL_LINUX_DEVICE__H__ */
