#ifndef _HIL_H_
#define _HIL_H_

/*
 * Copyright (c) 2014, Mentor Graphics Corporation
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. Neither the name of the <ORGANIZATION> nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

/**************************************************************************
 * FILE NAME
 *
 *       hil.h
 *
 * DESCRIPTION
 *
 *       This file defines interface layer to access hardware features. This
 *       interface is used by both RPMSG and remoteproc components.
 *
 ***************************************************************************/

#include "openamp/virtio.h"
#include "openamp/firmware.h"
#include "metal/list.h"
#include "metal/io.h"
#include "metal/device.h"

/* Configurable parameters */
#define HIL_MAX_CORES                   2
#define HIL_MAX_NUM_VRINGS              2
#define HIL_MAX_NUM_CHANNELS            1
/* Reserved CPU id */
#define HIL_RSVD_CPU_ID                 0xffffffff

/**
 * struct proc_info_hdr
 *
 * This structure is maintained by hardware interface layer
 * for user to pass hardware information to remote processor.
 */
struct proc_info_hdr {
	/* CPU ID as defined by the platform */
	unsigned long cpu_id;
	/* HIL platform ops table */
	struct hil_platform_ops *ops;
};

/**
 * struct proc_shm
 *
 * This structure is maintained by hardware interface layer for
 * shared memory information. The shared memory provides buffers
 * for use by the vring to exchange messages between the cores.
 *
 */
struct proc_shm {
	/* Start address of shared memory used for buffers. */
	void *start_addr;
	/* sharmed memory I/O region */
	struct metal_io_region *io;
	/* Size of shared memory. */
	unsigned long size;
	/* Attributes for shared memory - cached or uncached. */
	unsigned long flags;
};

/**
* struct proc_intr
*
* This structure is maintained by hardware interface layer for
* notification(interrupts) mechanism. The most common notification mechanism
* is Inter-Processor Interrupt(IPI). There can be other mechanism depending
* on SoC architecture.
*
*/
struct proc_intr {
	/* Interrupt number for vring - use for IPI */
	unsigned int vect_id;
	/* Interrupt priority */
	unsigned int priority;
	/* Interrupt trigger type */
	unsigned int trigger_type;
	/* IPI metal device */
	struct metal_device *dev;
	/* IPI device I/O */
	struct metal_io_region *io;
	/* Private data */
	void *data;
};

/**
* struct proc_vring
*
* This structure is maintained by hardware interface layer to keep
* vring physical memory and notification info.
*
*/
struct proc_vring {
	/* Pointer to virtqueue encapsulating the vring */
	struct virtqueue *vq;
	/* Vring logical address */
	void *vaddr;
	/* Vring metal device */
	struct metal_device *dev;
	/* Vring I/O region */
	struct metal_io_region *io;
	/* Number of vring descriptors */
	unsigned short num_descs;
	/* Vring alignment */
	unsigned long align;
	/* Vring interrupt control block */
	struct proc_intr intr_info;
};

/**
 * struct proc_vdev
 *
 * This structure represents a virtio HW device for remote processor.
 * Currently only one virtio device per processor is supported.
 *
 */
struct proc_vdev {
	/* Number of vrings */
	unsigned int num_vrings;
	/* Virtio device features */
	unsigned int dfeatures;
	/* Virtio gen features */
	unsigned int gfeatures;
	/* Vring info control blocks */
	struct proc_vring vring_info[HIL_MAX_NUM_VRINGS];
};

/**
 * struct proc_chnl
 *
 * This structure represents channel IDs that would be used by
 * the remote in the name service message. This will be extended
 * further to support static channel creation.
 *
 */
struct proc_chnl {
	/* Channel ID */
	char name[32];
};

/**
* struct hil_proc
*
* This structure represents a remote processor and encapsulates shared
* memory and notification info required for IPC.
*
*/
struct hil_proc {
	/* CPU ID as defined by the platform */
	unsigned long cpu_id;
	/* HIL platform ops table */
	struct hil_platform_ops *ops;
	/* Shared memory info */
	struct proc_shm sh_buff;
	/* Virtio device hardware info */
	struct proc_vdev vdev;
	/* Number of RPMSG channels */
	unsigned long num_chnls;
	/* RPMsg channels array */
	struct proc_chnl chnls[HIL_MAX_NUM_CHANNELS];
	/* Attrbites to represent processor role, master or remote . This field is for
	 * future use. */
	unsigned long attr;
	/*
	 * CPU bitmask - shared variable updated by each core
	 * after it has been initialized. This field is for future use.
	 */
	unsigned long cpu_bitmask;
	/* Spin lock - This field is for future use. */
	volatile unsigned int *slock;
	/* List node */
	struct metal_list node;
};

/**
 * hil_create_proc
 *
 * This function creates a HIL proc instance for given CPU id and populates
 * it with platform info.
 *
 * @param pdata  - platform data for remote processor
 * @param cpu_id - cpu id
 *
 * @return - pointer to proc instance
 *
 */
struct hil_proc *hil_create_proc(void *pdata, int cpu_id);

/**
 * hil_delete_proc
 *
 * This function deletes the given proc instance and frees the
 * associated resources.
 *
 * @param proc - pointer to HIL proc instance
 *
 */
void hil_delete_proc(struct hil_proc *proc);

/**
 * hil_get_proc
 *
 * This function finds the proc instance based on the given ID
 * from the proc list and returns it to user.
 *
 * @param cpu_id - cpu id
 *
 * @return - pointer to proc instance
 *
 */
struct hil_proc *hil_get_proc(int cpu_id);

/**
 * hil_isr()
 *
 * This function is called when interrupt is received for the vring.
 * This function gets the corresponding virtqueue and generates
 * call back for it.
 *
 * @param vring_hw   - pointer to vring control block
 *
 */
void hil_isr(struct proc_vring *vring_hw);

/**
 * hil_get_vdev_info
 *
 * This function return virtio device for remote core.
 *
 * @param proc - pointer to remote proc
 *
 * @return - pointer to virtio HW device.
 *
 */
struct proc_vdev *hil_get_vdev_info(struct hil_proc *proc);

/**
 * hil_get_chnl_info
 *
 * This function returns channels info for given proc.
 *
 * @param proc - pointer to proc info struct
 * @param num_chnls - pointer to integer variable to hold
 *                    number of available channels
 *
 * @return - pointer to channel info control block
 *
 */
struct proc_chnl *hil_get_chnl_info(struct hil_proc *proc, int *num_chnls);

/**
 * hil_get_vring_info
 *
 * This function returns vring_info_table. The caller will use
 * this table to get the vring HW info which will be subsequently
 * used to create virtqueues.
 *
 * @param vdev - pointer to virtio HW device
 * @param num_vrings - pointer to hold number of vrings
 *
 * @return - pointer to vring hardware info table
 */
struct proc_vring *hil_get_vring_info(struct proc_vdev *vdev, int *num_vrings);

/**
 * hil_get_shm_info
 *
 * This function returns shared memory info control block. The caller
 * will use this information to create and manage memory buffers for
 * vring descriptor table.
 *
 * @param proc - pointer to proc instance
 *
 * @return - pointer to shared memory region used for buffers
 *
 */
struct proc_shm *hil_get_shm_info(struct hil_proc *proc);

/**
 * hil_enable_vring_notifications()
 *
 * This function is called after successful creation of virtqueues.
 * This function saves queue handle in the vring_info_table which
 * will be used during interrupt handling .This function setups
 * interrupt handlers.
 *
 * @param vring_index - index to vring HW table
 * @param vq          - pointer to virtqueue to save in vring HW table
 *
 * @return            - execution status
 */
int hil_enable_vring_notifications(int vring_index, struct virtqueue *vq);

/**
 * hil_vring_notify()
 *
 * This function generates IPI to let the other side know that there is
 * job available for it. The required information to achieve this, like interrupt
 * vector, CPU id etc is be obtained from the proc_vring table.
 *
 * @param vq - pointer to virtqueue
 *
 */
void hil_vring_notify(struct virtqueue *vq);

/**
 * hil_get_status
 *
 * This function is used to check if the given core is up and running.
 * This call will return after it is confirmed that remote core has
 * started.
 *
 * @param proc - pointer to proc instance
 *
 * @return - execution status
 */
int hil_get_status(struct hil_proc *proc);

/**
 * hil_set_status
 *
 * This function is used to update the status
 * of the given core i.e it is ready for IPC.
 *
 * @param proc - pointer to remote proc
 *
 * @return - execution status
 */

int hil_set_status(struct hil_proc *proc);

/**
 * hil_boot_cpu
 *
 * This function starts remote processor at given address.
 *
 * @param proc      - pointer to remote proc
 * @param load_addr - load address of remote firmware
 *
 * @return - execution status
 */
int hil_boot_cpu(struct hil_proc *proc, unsigned int load_addr);

/**
 * hil_shutdown_cpu
 *
 *  This function shutdowns the remote processor
 *
 * @param proc - pointer to remote proc
 *
 */
void hil_shutdown_cpu(struct hil_proc *proc);

/**
 * hil_get_firmware
 *
 * This function returns address and size of given firmware name passed as
 * parameter.
 *
 * @param fw_name    - name of the firmware
 * @param start_addr - pointer t hold start address of firmware
 * @param size       - pointer to hold size of firmware
 *
 * returns -  status of function execution
 *
 */
int hil_get_firmware(char *fw_name, uintptr_t *start_addr,
		     unsigned int *size);

/**
 * hil_poll
 *
 * This function polls the remote processor.
 * If it is blocking mode, it will not return until the remoteproc
 * is signaled. If it is non-blocking mode, it will return 0
 * if the remoteproc has pending signals, it will return non 0
 * otherwise.
 *
 * @param proc     - hil_proc to poll
 * @param nonblock - 0 for blocking, non-0 for non-blocking.
 *
 * @return - 0 for no errors, non-0 for errors.
 */
int hil_poll (struct hil_proc *proc, int nonblock);
/**
 *
 * This structure is an interface between HIL and platform porting
 * component. It is required for the user to provide definitions of
 * these functions when framework is ported to new hardware platform.
 *
 */
struct hil_platform_ops {
    /**
     * enable_interrupt()
     *
     * This function enables interrupt(IPI) for given vring.
     *
     * @param vring_hw - pointer to vring control block
     *
     * @return  - execution status
     */
	int (*enable_interrupt) (struct proc_vring * vring_hw);

    /**
     * notify()
     *
     * This function generates IPI to let the other side know that there is
     * job available for it.
     *
     * @param cpu_id - ID of CPU which is to be notified
     * @param intr_info - pointer to interrupt info control block
     */
	void (*notify) (int cpu_id, struct proc_intr * intr_info);

    /**
     * get_status
     *
     * This function is used to check if the given core is
     * up and running. This call will return after it is confirmed
     * that remote core is initialized.
     *
     * @param cpu_id - ID of CPU for which status is requested.
     *
     * @return - execution status
     */
	int (*get_status) (int cpu_id);

    /**
     * set_status
     *
     * This function is used to update the status
     * of the given core i.e it is ready for IPC.
     *
     * @param cpu_id - ID of CPU for which status is to be set
     *
     * @return - execution status
     */

	int (*set_status) (int cpu_id);

    /**
     * boot_cpu
     *
     * This function boots the remote processor.
     *
     * @param cpu_id     - ID of CPU to boot
     * @param start_addr - start address of remote cpu
     *
     * @return - execution status
     */
	int (*boot_cpu) (int cpu_id, unsigned int start_addr);

    /**
     * shutdown_cpu
     *
     *  This function shutdowns the remote processor.
     *
     * @param cpu_id    - ID of CPU to shutdown
     *
     */
	void (*shutdown_cpu) (int cpu_id);

    /**
     * poll
     *
     * This function polls the remote processor.
     *
     * @param proc     - hil_proc to poll
     * @param nonblock - 0 for blocking, non-0 for non-blocking.
     *
     * @return - 0 for no errors, non-0 for errors.
     */
	int (*poll) (struct hil_proc *proc, int nonblock);

    /**
     * initialize
     *
     *  This function initialize remote processor with platform data.
     *
     * @param[in] pdata - platform data
     * @param[in] cpu_id - CPU id
     *
     * @return NULL on failure, hil_proc pointer otherwise
     *
     */
	struct hil_proc *(*initialize) (void *pdata, int cpu_id);

    /**
     * release
     *
     *  This function is to release remote processor resource
     *
     * @param[in] proc - pointer to the remote processor
     *
     */
	void (*release) (struct hil_proc *proc);
};

/* Utility macros for register read/write */
#define         HIL_MEM_READ8(addr)         *(volatile unsigned char *)(addr)
#define         HIL_MEM_READ16(addr)        *(volatile unsigned short *)(addr)
#define         HIL_MEM_READ32(addr)        *(volatile unsigned long *)(addr)
#define         HIL_MEM_WRITE8(addr,data)   *(volatile unsigned char *)(addr) = (unsigned char)(data)
#define         HIL_MEM_WRITE16(addr,data)  *(volatile unsigned short *)(addr) = (unsigned short)(data)
#define         HIL_MEM_WRITE32(addr,data)  *(volatile unsigned long *)(addr) = (unsigned long)(data)

#endif				/* _HIL_H_ */
