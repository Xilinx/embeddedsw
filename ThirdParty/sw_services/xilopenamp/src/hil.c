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
 * 3. Neither the name of Mentor Graphics Corporation nor the names of its
 *    contributors may be used to endorse or promote products derived from this
 *    software without specific prior written permission.
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
 *       hil.c
 *
 * COMPONENT
 *
 *         OpenAMP  Stack.
 *
 * DESCRIPTION
 *
 *       This file is implementation of generic part of HIL.
 *
 *
 *
 **************************************************************************/

#include "hil.h"

/*--------------------------- Globals ---------------------------------- */
struct hil_proc_list procs;

#if defined (OPENAMP_BENCHMARK_ENABLE)

unsigned long long boot_time_stamp;
unsigned long long shutdown_time_stamp;

#endif

extern int platform_get_processor_info(struct hil_proc *proc, int cpu_id);
extern int platform_get_processor_for_fw(char *fw_name);

/**
 * hil_create_proc
 *
 * This function creates a HIL proc instance for given CPU id and populates
 * it with platform info.
 *
 * @param cpu_id - cpu id
 *
 * @return - pointer to proc instance
 *
 */
struct hil_proc *hil_create_proc(int cpu_id) {
    struct hil_proc *proc = NULL;
    struct llist *node = NULL;
    struct llist *proc_hd = procs.proc_list;
    int status;

    /* If proc already exists then return it */
    while (proc_hd != NULL) {
        proc = (struct hil_proc *) proc_hd->data;
        if (proc->cpu_id == cpu_id) {
            return proc;
        }
        proc_hd = proc_hd->next;
    }

    /* Allocate memory for proc instance */
    proc = env_allocate_memory(sizeof(struct hil_proc));
    if (!proc) {
        return NULL;
    }

    /* Get HW specfic info */
    status = platform_get_processor_info(proc, cpu_id);
    if (status) {
        env_free_memory(proc);
        return NULL;
    }

    /* Enable mapping for the shared memory region */
    env_map_memory((unsigned int) proc->sh_buff.start_addr,
                    (unsigned int) proc->sh_buff.start_addr, proc->sh_buff.size,
                    (SHARED_MEM | UNCACHED));

    /* Put the new proc in the procs list */
    node = env_allocate_memory(sizeof(struct llist));

    if (!node) {
        env_free_memory(proc);
        return NULL;
    }

    node->data = proc;
    add_to_list(&procs.proc_list, node);

    return proc;
}

/**
 * hil_get_cpuforfw
 *
 * This function provides the CPU ID for the given firmware.
 *
 * @param fw_name - name of firmware
 *
 * @return - cpu id
 *
 */
int hil_get_cpuforfw(char *fw_name) {
    return (platform_get_processor_for_fw(fw_name));
}

/**
 * hil_delete_proc
 *
 * This function deletes the given proc instance and frees the
 * associated resources.
 *
 * @param proc - pointer to hil remote_proc instance
 *
 */
void hil_delete_proc(struct hil_proc *proc) {
    struct llist *proc_hd = NULL;

    if (!proc)
        return;

    proc_hd = procs.proc_list;

    while (proc_hd != NULL) {
        if (proc_hd->data == proc) {
            remove_from_list(&procs.proc_list, proc_hd);
            env_free_memory(proc_hd);
            break;
        }
        proc_hd = proc_hd->next;
    }

    env_free_memory(proc);
}


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
void hil_isr(struct proc_vring *vring_hw){
	virtqueue_notification(vring_hw->vq);
}

/**
 * hil_get_proc
 *
 * This function finds the proc instance based on the given ID
 * from the proc list and returns it to user.
 *
 * @param cpu_id - cpu id
 *
 * @return - pointer to hil proc instance
 *
 */
struct hil_proc *hil_get_proc(int cpu_id) {
    struct llist *proc_hd = procs.proc_list;

    if (!proc_hd)
        return NULL;

    while (proc_hd != NULL) {
        struct hil_proc *proc = (struct hil_proc *) proc_hd->data;
        if (proc->cpu_id == cpu_id) {
            return proc;
        }
        proc_hd = proc_hd->next;
    }

    return NULL;
}

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
struct proc_chnl *hil_get_chnl_info(struct hil_proc *proc, int *num_chnls) {
    *num_chnls = proc->num_chnls;
    return (proc->chnls);
}

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

struct proc_vdev *hil_get_vdev_info(struct hil_proc *proc) {
    return (&proc->vdev);

}

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
struct proc_vring *hil_get_vring_info(struct proc_vdev *vdev, int *num_vrings) {

    *num_vrings = vdev->num_vrings;
    return (vdev->vring_info);

}

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
struct proc_shm *hil_get_shm_info(struct hil_proc *proc) {
    return (&proc->sh_buff);
}

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
int hil_enable_vring_notifications(int vring_index, struct virtqueue *vq) {
    struct hil_proc *proc_hw = (struct hil_proc *) vq->vq_dev->device;
    struct proc_vring *vring_hw = &proc_hw->vdev.vring_info[vring_index];
    /* Save virtqueue pointer for later reference */
    vring_hw->vq = vq;

    if (proc_hw->ops->enable_interrupt) {
        proc_hw->ops->enable_interrupt(vring_hw);
    }

    return 0;
}

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
void hil_vring_notify(struct virtqueue *vq) {
    struct hil_proc *proc_hw = (struct hil_proc *) vq->vq_dev->device;
    struct proc_vring *vring_hw = &proc_hw->vdev.vring_info[vq->vq_queue_index];

    if (proc_hw->ops->notify) {
        proc_hw->ops->notify(proc_hw->cpu_id, &vring_hw->intr_info);
    }
}

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
int hil_get_status(struct hil_proc *proc) {
    /* For future use only.*/
    return 0;
}

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
int hil_set_status(struct hil_proc *proc) {
    /* For future use only.*/
    return 0;
}

/**
 * hil_boot_cpu
 *
 * This function boots the remote processor.
 *
 * @param proc       - pointer to remote proc
 * @param start_addr - start address of remote cpu
 *
 * @return - execution status
 */
int hil_boot_cpu(struct hil_proc *proc, unsigned int start_addr) {

    if (proc->ops->boot_cpu) {
        proc->ops->boot_cpu(proc->cpu_id, start_addr);
    }

#if defined (OPENAMP_BENCHMARK_ENABLE)
    boot_time_stamp = env_get_timestamp();
#endif

    return 0;
}

/**
 * hil_shutdown_cpu
 *
 *  This function shutdowns the remote processor
 *
 * @param proc - pointer to remote proc
 *
 */
void hil_shutdown_cpu(struct hil_proc *proc) {
    if (proc->ops->shutdown_cpu) {
        proc->ops->shutdown_cpu(proc->cpu_id);
    }

#if defined (OPENAMP_BENCHMARK_ENABLE)
    shutdown_time_stamp = env_get_timestamp();
#endif
}

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
int hil_get_firmware(char *fw_name, unsigned int *start_addr, unsigned int *size){
    return (config_get_firmware(fw_name , start_addr, size));
}
