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
 *       rpmsg.c
 *
 * COMPONENT
 *
 *       OpenAMP stack.
 *
 * DESCRIPTION
 *
 * Main file for the RPMSG driver. This file implements APIs as defined by
 * RPMSG documentation(Linux docs) and also provides some utility functions.
 *
 * RPMSG driver represents each processor/core to which it communicates with
 * remote_device control block.
 * Each remote device(processor) defines its role in the communication i.e
 * whether it is RPMSG Master or Remote. If the device(processor) to which
 * driver is talking is RPMSG master then RPMSG driver implicitly behaves as
 * Remote and vice versa.
 * RPMSG Master is responsible for initiating communications with the Remote
 * and shared buffers management. Terms remote device/core/proc are used
 * interchangeably for the processor to which RPMSG driver is communicating
 * irrespective of the fact whether it is RPMSG Remote or Master.
 *
 **************************************************************************/
#include "rpmsg.h"

/**
 * rpmsg_init
 *
 * Thus function allocates and initializes the rpmsg driver resources for
 * given device ID(cpu id). The successful return from this function leaves
 * fully enabled IPC link.
 *
 * @param dev_id            - remote device for which driver is to
 *                            be initialized
 * @param rdev              - pointer to newly created remote device
 * @param channel_created   - callback function for channel creation
 * @param channel_destroyed - callback function for channel deletion
 * @param default_cb        - default callback for channel I/O
 * @param role              - role of the other device, Master or Remote
 *
 * @return - status of function execution
 *
 */

int rpmsg_init(int dev_id, struct remote_device **rdev,
                rpmsg_chnl_cb_t channel_created,
                rpmsg_chnl_cb_t channel_destroyed,
                rpmsg_rx_cb_t default_cb, int role) {
    int status;

    /* Initialize IPC environment */
    status = env_init();
    if (status == RPMSG_SUCCESS) {
        /* Initialize the remote device for given cpu id */
        status = rpmsg_rdev_init(rdev, dev_id, role, channel_created,
                        channel_destroyed, default_cb);
        if (status == RPMSG_SUCCESS) {
            /* Kick off IPC with the remote device */
            status = rpmsg_start_ipc(*rdev);
        }
    }

    /* Deinit system in case of error */
    if (status != RPMSG_SUCCESS) {
        rpmsg_deinit(*rdev);
    }

    return status;
}

/**
 * rpmsg_deinit
 *
 * Thus function frees rpmsg driver resources for given remote device.
 *
 * @param rdev  -  pointer to device to de-init
 *
 */

void rpmsg_deinit(struct remote_device *rdev) {
    if (rdev) {
        rpmsg_rdev_deinit(rdev);
        env_deinit();
    }
}

/**
 * This function sends rpmsg "message" to remote device.
 *
 * @param rp_chnl - pointer to rpmsg channel
 * @param src     - source address of channel
 * @param dst     - destination address of channel
 * @param data    - data to transmit
 * @param size    - size of data
 * @param wait    - boolean, wait or not for buffer to become
 *                  available
 *
 * @return - status of function execution
 *
 */

int rpmsg_send_offchannel_raw(struct rpmsg_channel *rp_chnl, unsigned long src,
                unsigned long dst, char *data, int size, int wait) {
    struct remote_device *rdev;
    struct rpmsg_hdr *rp_hdr;
    void *buffer;
    int status = RPMSG_SUCCESS;
    unsigned short idx;
    int tick_count = 0;
    int buff_len;

    if (!rp_chnl) {
        return RPMSG_ERR_PARAM;
    }

    /* Get the associated remote device for channel. */
    rdev = rp_chnl->rdev;

    /* Validate device state */
    if (rp_chnl->state != RPMSG_CHNL_STATE_ACTIVE
                    || rdev->state != RPMSG_DEV_STATE_ACTIVE) {
        return RPMSG_ERR_DEV_STATE;
    }

    /* Lock the device to enable exclusive access to virtqueues */
    env_lock_mutex(rdev->lock);
    /* Get rpmsg buffer for sending message. */
    buffer = rpmsg_get_tx_buffer(rdev, &buff_len, &idx);
    if (!buffer && !wait) {
        status = RPMSG_ERR_NO_MEM;
    }
    env_unlock_mutex(rdev->lock);

    if (status == RPMSG_SUCCESS) {

        while (!buffer) {
            /*
             * Wait parameter is true - pool the buffer for
             * 15 secs as defined by the APIs.
             */
            env_sleep_msec(RPMSG_TICKS_PER_INTERVAL);
            env_lock_mutex(rdev->lock);
            buffer = rpmsg_get_tx_buffer(rdev, &buff_len, &idx);
            env_unlock_mutex(rdev->lock);
            tick_count += RPMSG_TICKS_PER_INTERVAL;
            if (tick_count >= (RPMSG_TICK_COUNT / RPMSG_TICKS_PER_INTERVAL)) {
                status = RPMSG_ERR_NO_BUFF;
                break;
            }
        }

        if (status == RPMSG_SUCCESS) {
            //FIXME : may be just copy the data size equal to buffer length and Tx it.
            if (size > (buff_len - sizeof(struct rpmsg_hdr)))
                status = RPMSG_ERR_BUFF_SIZE;

            if (status == RPMSG_SUCCESS) {
                rp_hdr = (struct rpmsg_hdr *) buffer;

                /* Initialize RPMSG header. */
                rp_hdr->dst = dst;
                rp_hdr->src = src;
                rp_hdr->len = size;

                /* Copy data to rpmsg buffer. */
                env_memcpy(rp_hdr->data, data, size);

                env_lock_mutex(rdev->lock);
                /* Enqueue buffer on virtqueue. */
                status = rpmsg_enqueue_buffer(rdev, buffer, buff_len, idx);
                if (status == RPMSG_SUCCESS) {
                    /* Let the other side know that there is a job to process. */
                    virtqueue_kick(rdev->tvq);
                }
                env_unlock_mutex(rdev->lock);
            }

        }
    }

    /* Do cleanup in case of error.*/
    if (status != RPMSG_SUCCESS) {
        rpmsg_free_buffer(rdev, buffer);
    }

    return status;
}

/**
 * rpmsg_get_buffer_size
 *
 * Returns buffer size available for sending messages.
 *
 * @param channel - pointer to rpmsg channel
 *
 * @return - buffer size
 *
 */
int rpmsg_get_buffer_size(struct rpmsg_channel *rp_chnl) {
    struct remote_device *rdev;
    int length;

    if (!rp_chnl) {
        return RPMSG_ERR_PARAM;
    }

    /* Get associated remote device for channel. */
    rdev = rp_chnl->rdev;

    /* Validate device state */
    if (rp_chnl->state != RPMSG_CHNL_STATE_ACTIVE
                    || rdev->state != RPMSG_DEV_STATE_ACTIVE) {
        return RPMSG_ERR_DEV_STATE;
    }

    env_lock_mutex(rdev->lock);

    if (rdev->role == RPMSG_REMOTE) {
        /*
         * If device role is Remote then buffers are provided by us
         * (RPMSG Master), so just provide the macro.
         */
        length = RPMSG_BUFFER_SIZE - sizeof(struct rpmsg_hdr);
    } else {
        /*
         * If other core is Master then buffers are provided by it,
         * so get the buffer size from the virtqueue.
         */
        length = (int) virtqueue_get_desc_size(rdev->tvq) - sizeof(struct rpmsg_hdr);
    }

    env_unlock_mutex(rdev->lock);

    return length;
}

/**
 * rpmsg_create_ept
 *
 * This function creates rpmsg endpoint for the rpmsg channel.
 *
 * @param channel - pointer to rpmsg channel
 * @param cb      - Rx completion call back
 * @param priv    - private data
 * @param addr    - endpoint src address
 *
 * @return - pointer to endpoint control block
 *
 */
struct rpmsg_endpoint *rpmsg_create_ept(struct rpmsg_channel *rp_chnl,
                rpmsg_rx_cb_t cb, void *priv, unsigned long addr) {

    struct remote_device *rdev = RPMSG_NULL;
    struct rpmsg_endpoint *rp_ept = RPMSG_NULL;

    if (!rp_chnl || !cb) {
        return RPMSG_NULL ;
    }

    rdev = rp_chnl->rdev;

    rp_ept = _create_endpoint(rdev, cb, priv, addr);

    if (rp_ept) {
        rp_ept->rp_chnl = rp_chnl;
    }

    return rp_ept;
}

/**
 * rpmsg_destroy_ept
 *
 * This function deletes rpmsg endpoint and performs cleanup.
 *
 * @param rp_ept - pointer to endpoint to destroy
 *
 */
void rpmsg_destroy_ept(struct rpmsg_endpoint *rp_ept) {

    struct remote_device *rdev;
    struct rpmsg_channel *rp_chnl;

    if (!rp_ept)
        return;

    rp_chnl = rp_ept->rp_chnl;
    rdev = rp_chnl->rdev;

    _destroy_endpoint(rdev, rp_ept);
}

/**
 * rpmsg_create_channel
 *
 * This function provides facility to create channel dynamically. It sends
 * Name Service announcement to remote device to let it know about the channel
 * creation. There must be an active communication among the cores (or atleast
 * one rpmsg channel must already exist) before using this API to create new
 * channels.
 *
 * @param rdev - pointer to remote device
 * @param name - channel name
 *
 * @return - pointer to new rpmsg channel
 *
 */
struct rpmsg_channel *rpmsg_create_channel(struct remote_device *rdev,
                char *name) {

    struct rpmsg_channel *rp_chnl;
    struct rpmsg_endpoint *rp_ept;

    if (!rdev || !name) {
        return RPMSG_NULL ;
    }

    /* Create channel instance */
    rp_chnl = _rpmsg_create_channel(rdev, name, RPMSG_NS_EPT_ADDR,
                    RPMSG_NS_EPT_ADDR);
    if (!rp_chnl) {
        return RPMSG_NULL ;
    }

    /* Create default endpoint for the channel */
    rp_ept = rpmsg_create_ept(rp_chnl , rdev->default_cb, rdev,
                    RPMSG_ADDR_ANY);

    if (!rp_ept) {
        _rpmsg_delete_channel(rp_chnl);
        return RPMSG_NULL;
    }

    rp_chnl->rp_ept = rp_ept;
    rp_chnl->src = rp_ept->addr;
    rp_chnl->state = RPMSG_CHNL_STATE_NS;

    /* Notify the application of channel creation event */
    if (rdev->channel_created) {
        rdev->channel_created(rp_chnl);
    }

    /* Send NS announcement to remote processor */
    rpmsg_send_ns_message(rdev, rp_chnl, RPMSG_NS_CREATE);

    return rp_chnl;
}

/**
 * rpmsg_delete_channel
 *
 * Deletes the given RPMSG channel. The channel must first be created with the
 * rpmsg_create_channel API.
 *
 * @param rp_chnl - pointer to rpmsg channel to delete
 *
 */
void rpmsg_delete_channel(struct rpmsg_channel *rp_chnl) {

    struct remote_device *rdev;

    if (!rp_chnl) {
        return;
    }

    rdev = rp_chnl->rdev;

    if (rp_chnl->state > RPMSG_CHNL_STATE_IDLE) {
        /* Notify the other processor that channel no longer exists */
        rpmsg_send_ns_message(rdev, rp_chnl, RPMSG_NS_DESTROY);
    }

    /* Notify channel deletion to application */
    if (rdev->channel_destroyed) {
        rdev->channel_destroyed(rp_chnl);
    }

    rpmsg_destroy_ept(rp_chnl->rp_ept);
    _rpmsg_delete_channel(rp_chnl);

    return;
}
