/*
 * Remote processor messaging
 *
 * Copyright (C) 2011 Texas Instruments, Inc.
 * Copyright (C) 2011 Google, Inc.
 * All rights reserved.
 * Copyright (c) 2016 Freescale Semiconductor, Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * * Redistributions of source code must retain the above copyright
 *   notice, this list of conditions and the following disclaimer.
 * * Redistributions in binary form must reproduce the above copyright
 *   notice, this list of conditions and the following disclaimer in
 *   the documentation and/or other materials provided with the
 *   distribution.
 * * Neither the name Texas Instruments nor the names of its
 *   contributors may be used to endorse or promote products derived
 *   from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef _RPMSG_H_
#define _RPMSG_H_

#include "openamp/rpmsg_core.h"

/* The feature bitmap for virtio rpmsg */
#define VIRTIO_RPMSG_F_NS	0	/* RP supports name service notifications */
#define RPMSG_NAME_SIZE     32

#define RPMSG_LOCATE_DATA(p)  ((unsigned char *) p + sizeof (struct rpmsg_hdr))

/**
 * struct rpmsg_hdr - common header for all rpmsg messages
 * @src: source address
 * @dst: destination address
 * @reserved: reserved for future use
 * @len: length of payload (in bytes)
 * @flags: message flags
 *
 * Every message sent(/received) on the rpmsg bus begins with this header.
 */
OPENAMP_PACKED_BEGIN
struct rpmsg_hdr {
	uint32_t src;
	uint32_t dst;
	uint32_t reserved;
	uint16_t len;
	uint16_t flags;
} OPENAMP_PACKED_END;

/**
 * struct rpmsg_ns_msg - dynamic name service announcement message
 * @name: name of remote service that is published
 * @addr: address of remote service that is published
 * @flags: indicates whether service is created or destroyed
 *
 * This message is sent across to publish a new service, or announce
 * about its removal. When we receive these messages, an appropriate
 * rpmsg channel (i.e device) is created/destroyed. In turn, the ->probe()
 * or ->remove() handler of the appropriate rpmsg driver will be invoked
 * (if/as-soon-as one is registered).
 */
OPENAMP_PACKED_BEGIN
struct rpmsg_ns_msg {
	char name[RPMSG_NAME_SIZE];
	uint32_t addr;
	uint32_t flags;
} OPENAMP_PACKED_END;

/**
 * enum rpmsg_ns_flags - dynamic name service announcement flags
 *
 * @RPMSG_NS_CREATE: a new remote service was just created
 * @RPMSG_NS_DESTROY: a known remote service was just destroyed
 */
enum rpmsg_ns_flags {
	RPMSG_NS_CREATE = 0,
	RPMSG_NS_DESTROY = 1,
};

#define RPMSG_ADDR_ANY		0xFFFFFFFF

/**
 * rpmsg_channel - devices that belong to the rpmsg bus are called channels
 * @name: channel name
 * @src: local address
 * @dst: destination address
 * rdev: rpmsg remote device
 * @ept: the rpmsg endpoint of this channel
 * @state: channel state
 */
struct rpmsg_channel {
	char name[RPMSG_NAME_SIZE];
	uint32_t src;
	uint32_t dst;
	struct remote_device *rdev;
	struct rpmsg_endpoint *rp_ept;
	unsigned int state;
	struct metal_list node;
};

/**
 * channel_info - channel info
 * @name: channel name
 * @src: local address
 * @dst: destination address
 */

struct channel_info {
	char name[RPMSG_NAME_SIZE];
	uint32_t src;
	uint32_t dest;
};

/**
 * struct rpmsg_endpoint - binds a local rpmsg address to its user
 * @rp_chnl: rpmsg channel device
 * @cb: rx callback handler
 * @addr: local rpmsg address
 * @priv: private data for the driver's use
 *
 * In essence, an rpmsg endpoint represents a listener on the rpmsg bus, as
 * it binds an rpmsg address with an rx callback handler.
 *
 * Simple rpmsg drivers shouldn't use this struct directly, because
 * things just work: every rpmsg driver provides an rx callback upon
 * registering to the bus, and that callback is then bound to its rpmsg
 * address when the driver is probed. When relevant inbound messages arrive
 * (i.e. messages which their dst address equals to the src address of
 * the rpmsg channel), the driver's handler is invoked to process it.
 *
 * More complicated drivers though, that do need to allocate additional rpmsg
 * addresses, and bind them to different rx callbacks, must explicitly
 * create additional endpoints by themselves (see rpmsg_create_ept()).
 */
struct rpmsg_endpoint {
	struct rpmsg_channel *rp_chnl;
	rpmsg_rx_cb_t cb;
	uint32_t addr;
	void *priv;
	struct metal_list node;
};

struct rpmsg_endpoint *rpmsg_create_ept(struct rpmsg_channel *rp_chnl,
					rpmsg_rx_cb_t cb, void *priv,
					uint32_t addr);

void rpmsg_destroy_ept(struct rpmsg_endpoint *rp_ept);

int
rpmsg_send_offchannel_raw(struct rpmsg_channel *, uint32_t, uint32_t,
			  char *, int, int);

/**
 * rpmsg_send() - send a message across to the remote processor
 * @rpdev: the rpmsg channel
 * @data: payload of message
 * @len: length of payload
 *
 * This function sends @data of length @len on the @rpdev channel.
 * The message will be sent to the remote processor which the @rpdev
 * channel belongs to, using @rpdev's source and destination addresses.
 * In case there are no TX buffers available, the function will block until
 * one becomes available, or a timeout of 15 seconds elapses. When the latter
 * happens, -ERESTARTSYS is returned.
 *
 * Can only be called from process context (for now).
 *
 * Returns 0 on success and an appropriate error value on failure.
 */
static inline int rpmsg_send(struct rpmsg_channel *rpdev, void *data, int len)
{
	return rpmsg_send_offchannel_raw(rpdev, rpdev->src, rpdev->dst,
					 (char *)data, len, RPMSG_TRUE);
}

/**
 * rpmsg_sendto() - send a message across to the remote processor, specify dst
 * @rpdev: the rpmsg channel
 * @data: payload of message
 * @len: length of payload
 * @dst: destination address
 *
 * This function sends @data of length @len to the remote @dst address.
 * The message will be sent to the remote processor which the @rpdev
 * channel belongs to, using @rpdev's source address.
 * In case there are no TX buffers available, the function will block until
 * one becomes available, or a timeout of 15 seconds elapses. When the latter
 * happens, -ERESTARTSYS is returned.
 *
 * Can only be called from process context (for now).
 *
 * Returns 0 on success and an appropriate error value on failure.
 */
static inline int rpmsg_sendto(struct rpmsg_channel *rpdev, void *data,
			       int len, uint32_t dst)
{
	return rpmsg_send_offchannel_raw(rpdev, rpdev->src, dst, (char *)data,
					 len, RPMSG_TRUE);
}

/**
 * rpmsg_send_offchannel() - send a message using explicit src/dst addresses
 * @rpdev: the rpmsg channel
 * @src: source address
 * @dst: destination address
 * @data: payload of message
 * @len: length of payload
 *
 * This function sends @data of length @len to the remote @dst address,
 * and uses @src as the source address.
 * The message will be sent to the remote processor which the @rpdev
 * channel belongs to.
 * In case there are no TX buffers available, the function will block until
 * one becomes available, or a timeout of 15 seconds elapses. When the latter
 * happens, -ERESTARTSYS is returned.
 *
 * Can only be called from process context (for now).
 *
 * Returns 0 on success and an appropriate error value on failure.
 */
static inline int rpmsg_send_offchannel(struct rpmsg_channel *rpdev,
					uint32_t src, uint32_t dst,
					void *data, int len)
{
	return rpmsg_send_offchannel_raw(rpdev, src, dst, (char *)data, len,
					 RPMSG_TRUE);
}

/**
 * rpmsg_trysend() - send a message across to the remote processor
 * @rpdev: the rpmsg channel
 * @data: payload of message
 * @len: length of payload
 *
 * This function sends @data of length @len on the @rpdev channel.
 * The message will be sent to the remote processor which the @rpdev
 * channel belongs to, using @rpdev's source and destination addresses.
 * In case there are no TX buffers available, the function will immediately
 * return -ENOMEM without waiting until one becomes available.
 *
 * Can only be called from process context (for now).
 *
 * Returns 0 on success and an appropriate error value on failure.
 */
static inline int rpmsg_trysend(struct rpmsg_channel *rpdev, void *data,
				int len)
{
	return rpmsg_send_offchannel_raw(rpdev, rpdev->src, rpdev->dst,
					 (char *)data, len, RPMSG_FALSE);
}

/**
 * rpmsg_trysendto() - send a message across to the remote processor, specify dst
 * @rpdev: the rpmsg channel
 * @data: payload of message
 * @len: length of payload
 * @dst: destination address
 *
 * This function sends @data of length @len to the remote @dst address.
 * The message will be sent to the remote processor which the @rpdev
 * channel belongs to, using @rpdev's source address.
 * In case there are no TX buffers available, the function will immediately
 * return -ENOMEM without waiting until one becomes available.
 *
 * Can only be called from process context (for now).
 *
 * Returns 0 on success and an appropriate error value on failure.
 */
static inline int rpmsg_trysendto(struct rpmsg_channel *rpdev, void *data,
				  int len, uint32_t dst)
{
	return rpmsg_send_offchannel_raw(rpdev, rpdev->src, dst, (char *)data, len,
					 RPMSG_FALSE);
}

/**
 * rpmsg_trysend_offchannel() - send a message using explicit src/dst addresses
 * @rpdev: the rpmsg channel
 * @src: source address
 * @dst: destination address
 * @data: payload of message
 * @len: length of payload
 *
 * This function sends @data of length @len to the remote @dst address,
 * and uses @src as the source address.
 * The message will be sent to the remote processor which the @rpdev
 * channel belongs to.
 * In case there are no TX buffers available, the function will immediately
 * return -ENOMEM without waiting until one becomes available.
 *
 * Can only be called from process context (for now).
 *
 * Returns 0 on success and an appropriate error value on failure.
 */
static inline int rpmsg_trysend_offchannel(struct rpmsg_channel *rpdev,
					   uint32_t src, uint32_t dst,
					   void *data, int len)
{
	return rpmsg_send_offchannel_raw(rpdev, src, dst, (char *)data, len,
					 RPMSG_FALSE);
}

/**
 * rpmsg_init
 *
 * Thus function allocates and initializes the rpmsg driver resources for given
 * device id (cpu id).The successful return from this function leaves
 * fully enabled IPC link.
 *
 * @param pdata             - platform data for remote processor
 * @param dev_id            - rpmsg remote device for which driver is to
 *                            be initialized
 * @param rdev              - pointer to newly created remote device
 * @param channel_created   - callback function for channel creation
 * @param channel_destroyed - callback function for channel deletion
 * @default_cb              - default callback for channel
 * @param role              - role of the other device, Master or Remote
 * @return - status of function execution
 *
 */

int rpmsg_init(void *pdata, int dev_id, struct remote_device **rdev,
	       rpmsg_chnl_cb_t channel_created,
	       rpmsg_chnl_cb_t channel_destroyed,
	       rpmsg_rx_cb_t default_cb, int role);

/**
 * rpmsg_deinit
 *
 * Thus function releases the rpmsg driver resources for given remote
 * instance.
 *
 * @param rdev  -  pointer to device de-init
 *
 * @return - none
 *
 */
void rpmsg_deinit(struct remote_device *rdev);

/**
 * rpmsg_get_buffer_size
 *
 * Returns buffer size available for sending messages.
 *
 * @param channel - pointer to rpmsg channel/device
 *
 * @return - buffer size
 *
 */
int rpmsg_get_buffer_size(struct rpmsg_channel *rp_chnl);

/**
 * rpmsg_create_channel
 *
 * Creates RPMSG channel with the given name for remote device.
 *
 * @param rdev - pointer to rpmsg remote device
 * @param name - channel name
 *
 * @return - pointer to new rpmsg channel
 *
 */
struct rpmsg_channel *rpmsg_create_channel(struct remote_device *rdev,
					   char *name);

/**
 * rpmsg_delete_channel
 *
 * Deletes the given RPMSG channel. The channel must first be created with the
 * rpmsg_create_channel API.
 *
 * @param rp_chnl - pointer to rpmsg channel to delete
 *
 */
void rpmsg_delete_channel(struct rpmsg_channel *rp_chnl);

#endif				/* _RPMSG_H_ */
