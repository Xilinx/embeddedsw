/*
 * Copyright (c) 2014, Mentor Graphics Corporation
 * All rights reserved.
 * Copyright (c) 2016 Freescale Semiconductor, Inc. All rights reserved.
 * Copyright (c) 2018 Linaro, Inc. All rights reserved.
 * Copyright (c) 2021 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <metal/alloc.h>
#include <metal/sleep.h>
#include <metal/utilities.h>
#include <openamp/rpmsg_virtio.h>
#include <openamp/virtqueue.h>

#include "rpmsg_internal.h"

#define RPMSG_NUM_VRINGS                        2

/* Total tick count for 15secs - 1usec tick. */
#define RPMSG_TICK_COUNT                        15000000

/* Time to wait - In multiple of 1 msecs. */
#define RPMSG_TICKS_PER_INTERVAL                1000

/**
 * struct vbuff_reclaimer_t - vring buffer recycler
 *
 * This structure is used by the rpmsg virtio to store unused virtio buffer, as the
 * virtqueue structure has been already updated and memory allocated.
 *
 * @node: node in reclaimer list.
 * @idx:  virtio descriptor index containing the buffer information.
 */
struct vbuff_reclaimer_t {
	struct metal_list node;
	uint16_t idx;
};

/* Default configuration */
#ifndef VIRTIO_DEVICE_ONLY
#define RPMSG_VIRTIO_DEFAULT_CONFIG                \
	(&(const struct rpmsg_virtio_config) {     \
		.h2r_buf_size = RPMSG_BUFFER_SIZE, \
		.r2h_buf_size = RPMSG_BUFFER_SIZE, \
		.split_shpool = false,             \
	})
#else
#define RPMSG_VIRTIO_DEFAULT_CONFIG          NULL
#endif

#ifndef VIRTIO_DEVICE_ONLY
metal_weak void *
rpmsg_virtio_shm_pool_get_buffer(struct rpmsg_virtio_shm_pool *shpool,
				 size_t size)
{
	void *buffer;

	if (!shpool || size == 0 || shpool->avail < size)
		return NULL;
	buffer = (char *)shpool->base + shpool->size - shpool->avail;
	shpool->avail -= size;

	return buffer;
}
#endif /*!VIRTIO_DEVICE_ONLY*/

void rpmsg_virtio_init_shm_pool(struct rpmsg_virtio_shm_pool *shpool,
				void *shb, size_t size)
{
	if (!shpool || !shb || size == 0)
		return;
	shpool->base = shb;
	shpool->size = size;
	shpool->avail = size;
}

/**
 * @internal
 *
 * @brief Places the used buffer back on the virtqueue.
 *
 * @param rvdev		Pointer to remote core
 * @param buffer	Buffer pointer
 * @param len		Buffer length
 * @param idx		Buffer index
 */
static void rpmsg_virtio_return_buffer(struct rpmsg_virtio_device *rvdev,
				       void *buffer, uint32_t len,
				       uint16_t idx)
{
	unsigned int role = rpmsg_virtio_get_role(rvdev);

	BUFFER_INVALIDATE(buffer, len);

#ifndef VIRTIO_DEVICE_ONLY
	if (role == RPMSG_HOST) {
		struct virtqueue_buf vqbuf;

		(void)idx;
		/* Initialize buffer node */
		vqbuf.buf = buffer;
		vqbuf.len = len;
		virtqueue_add_buffer(rvdev->rvq, &vqbuf, 0, 1, buffer);
	}
#endif /*VIRTIO_DEVICE_ONLY*/

#ifndef VIRTIO_DRIVER_ONLY
	if (role == RPMSG_REMOTE) {
		(void)buffer;
		virtqueue_add_consumed_buffer(rvdev->rvq, idx, len);
	}
#endif /*VIRTIO_DRIVER_ONLY*/
}

/**
 * @internal
 *
 * @brief Places buffer on the virtqueue for consumption by the other side.
 *
 * @param rvdev		Pointer to rpmsg virtio
 * @param buffer	Buffer pointer
 * @param len		Buffer length
 * @param idx		Buffer index
 *
 * @return Status of function execution
 */
static int rpmsg_virtio_enqueue_buffer(struct rpmsg_virtio_device *rvdev,
				       void *buffer, uint32_t len,
				       uint16_t idx)
{
	unsigned int role = rpmsg_virtio_get_role(rvdev);

	BUFFER_FLUSH(buffer, len);

#ifndef VIRTIO_DEVICE_ONLY
	if (role == RPMSG_HOST) {
		struct virtqueue_buf vqbuf;
		(void)idx;

		/* Initialize buffer node */
		vqbuf.buf = buffer;
		vqbuf.len = len;
		return virtqueue_add_buffer(rvdev->svq, &vqbuf, 1, 0, buffer);
	}
#endif /*!VIRTIO_DEVICE_ONLY*/

#ifndef VIRTIO_DRIVER_ONLY
	if (role == RPMSG_REMOTE) {
		(void)buffer;
		return virtqueue_add_consumed_buffer(rvdev->svq, idx, len);
	}
#endif /*!VIRTIO_DRIVER_ONLY*/
	return 0;
}

/**
 * @internal
 *
 * @brief Provides buffer to transmit messages.
 *
 * @param rvdev	Pointer to rpmsg device
 * @param len	Length of returned buffer
 * @param idx	Buffer index
 *
 * @return Pointer to buffer.
 */
static void *rpmsg_virtio_get_tx_buffer(struct rpmsg_virtio_device *rvdev,
					uint32_t *len, uint16_t *idx)
{
	unsigned int role = rpmsg_virtio_get_role(rvdev);
	struct metal_list *node;
	struct vbuff_reclaimer_t *r_desc;
	void *data = NULL;

	/* Try first to recycle a buffer that has been freed without been used */
	node = metal_list_first(&rvdev->reclaimer);
	if (node) {
		r_desc = metal_container_of(node, struct vbuff_reclaimer_t, node);
		metal_list_del(node);
		data = r_desc;

#ifndef VIRTIO_DEVICE_ONLY
		if (role == RPMSG_HOST)
			*len = rvdev->config.h2r_buf_size;
#endif /*!VIRTIO_DEVICE_ONLY*/
#ifndef VIRTIO_DRIVER_ONLY
		if (role == RPMSG_REMOTE) {
			*idx = r_desc->idx;
			*len = virtqueue_get_buffer_length(rvdev->svq, *idx);
		}
#endif /*!VIRTIO_DRIVER_ONLY*/
#ifndef VIRTIO_DEVICE_ONLY
	} else if (role == RPMSG_HOST) {
		data = virtqueue_get_buffer(rvdev->svq, len, idx);
		if (!data && rvdev->svq->vq_free_cnt) {
			data = rpmsg_virtio_shm_pool_get_buffer(rvdev->shpool,
					rvdev->config.h2r_buf_size);
			*len = rvdev->config.h2r_buf_size;
			*idx = 0;
		}
#endif /*!VIRTIO_DEVICE_ONLY*/
#ifndef VIRTIO_DRIVER_ONLY
	} else if (role == RPMSG_REMOTE) {
		data = virtqueue_get_available_buffer(rvdev->svq, idx, len);
#endif /*!VIRTIO_DRIVER_ONLY*/
	}

	return data;
}

/**
 * @internal
 *
 * @brief Retrieves the received buffer from the virtqueue.
 *
 * @param rvdev	Pointer to rpmsg device
 * @param len	Size of received buffer
 * @param idx	Index of buffer
 *
 * @return Pointer to received buffer
 */
static void *rpmsg_virtio_get_rx_buffer(struct rpmsg_virtio_device *rvdev,
					uint32_t *len, uint16_t *idx)
{
	unsigned int role = rpmsg_virtio_get_role(rvdev);
	void *data = NULL;

#ifndef VIRTIO_DEVICE_ONLY
	if (role == RPMSG_HOST) {
		data = virtqueue_get_buffer(rvdev->rvq, len, idx);
	}
#endif /*!VIRTIO_DEVICE_ONLY*/

#ifndef VIRTIO_DRIVER_ONLY
	if (role == RPMSG_REMOTE) {
		data =
		    virtqueue_get_available_buffer(rvdev->rvq, idx, len);
	}
#endif /*!VIRTIO_DRIVER_ONLY*/

	/* Invalidate the buffer before returning it */
	if (data)
		BUFFER_INVALIDATE(data, *len);

	return data;
}

#ifndef VIRTIO_DRIVER_ONLY
/*
 * check if the remote is ready to start RPMsg communication
 */
static int rpmsg_virtio_wait_remote_ready(struct rpmsg_virtio_device *rvdev)
{
	uint8_t status;

	while (1) {
		status = rpmsg_virtio_get_status(rvdev);
		/* Busy wait until the remote is ready */
		if (status & VIRTIO_CONFIG_STATUS_NEEDS_RESET) {
			rpmsg_virtio_set_status(rvdev, 0);
			/* TODO notify remote processor */
		} else if (status & VIRTIO_CONFIG_STATUS_DRIVER_OK) {
			return true;
		}
		/* TODO: clarify metal_cpu_yield usage*/
		metal_cpu_yield();
	}
}
#endif /*!VIRTIO_DRIVER_ONLY*/

/**
 * @internal
 *
 * @brief Returns buffer size available for sending messages.
 *
 * @param rvdev	Pointer to rpmsg device
 *
 * @return Buffer size
 */
static int _rpmsg_virtio_get_buffer_size(struct rpmsg_virtio_device *rvdev)
{
	unsigned int role = rpmsg_virtio_get_role(rvdev);
	int length = 0;

#ifndef VIRTIO_DEVICE_ONLY
	if (role == RPMSG_HOST) {
		/*
		 * If device role is host then buffers are provided by us,
		 * so just provide the macro.
		 */
		length = rvdev->config.h2r_buf_size - sizeof(struct rpmsg_hdr);
	}
#endif /*!VIRTIO_DEVICE_ONLY*/

#ifndef VIRTIO_DRIVER_ONLY
	if (role == RPMSG_REMOTE) {
		/*
		 * If other core is host then buffers are provided by it,
		 * so get the buffer size from the virtqueue.
		 */
		length =
		    (int)virtqueue_get_desc_size(rvdev->svq) -
		    sizeof(struct rpmsg_hdr);
	}
#endif /*!VIRTIO_DRIVER_ONLY*/

	if (length <= 0) {
		length = RPMSG_ERR_NO_BUFF;
	}

	return length;
}

static void rpmsg_virtio_hold_rx_buffer(struct rpmsg_device *rdev, void *rxbuf)
{
	struct rpmsg_hdr *rp_hdr;

	(void)rdev;

	rp_hdr = RPMSG_LOCATE_HDR(rxbuf);

	/* Set held status to keep buffer */
	rp_hdr->reserved |= RPMSG_BUF_HELD;
}

static void rpmsg_virtio_release_rx_buffer(struct rpmsg_device *rdev,
					   void *rxbuf)
{
	struct rpmsg_virtio_device *rvdev;
	struct rpmsg_hdr *rp_hdr;
	uint16_t idx;
	uint32_t len;

	rvdev = metal_container_of(rdev, struct rpmsg_virtio_device, rdev);
	rp_hdr = RPMSG_LOCATE_HDR(rxbuf);
	/* The reserved field contains buffer index */
	idx = (uint16_t)(rp_hdr->reserved & ~RPMSG_BUF_HELD);

	metal_mutex_acquire(&rdev->lock);
	/* Return buffer on virtqueue. */
	len = virtqueue_get_buffer_length(rvdev->rvq, idx);
	rpmsg_virtio_return_buffer(rvdev, rp_hdr, len, idx);
	/* Tell peer we return some rx buffers */
	virtqueue_kick(rvdev->rvq);
	metal_mutex_release(&rdev->lock);
}

static void *rpmsg_virtio_get_tx_payload_buffer(struct rpmsg_device *rdev,
						uint32_t *len, int wait)
{
	struct rpmsg_virtio_device *rvdev;
	struct rpmsg_hdr *rp_hdr;
	uint16_t idx;
	int tick_count;
	int status;

	/* Get the associated remote device for channel. */
	rvdev = metal_container_of(rdev, struct rpmsg_virtio_device, rdev);

	/* Validate device state */
	status = rpmsg_virtio_get_status(rvdev);
	if (!(status & VIRTIO_CONFIG_STATUS_DRIVER_OK))
		return NULL;

	if (wait)
		tick_count = RPMSG_TICK_COUNT / RPMSG_TICKS_PER_INTERVAL;
	else
		tick_count = 0;

	while (1) {
		/* Lock the device to enable exclusive access to virtqueues */
		metal_mutex_acquire(&rdev->lock);
		rp_hdr = rpmsg_virtio_get_tx_buffer(rvdev, len, &idx);
		metal_mutex_release(&rdev->lock);
		if (rp_hdr || !tick_count)
			break;
		metal_sleep_usec(RPMSG_TICKS_PER_INTERVAL);
		tick_count--;
	}

	if (!rp_hdr)
		return NULL;

	/* Store the index into the reserved field to be used when sending */
	rp_hdr->reserved = idx;

	/* Actual data buffer size is vring buffer size minus header length */
	*len -= sizeof(struct rpmsg_hdr);
	return RPMSG_LOCATE_DATA(rp_hdr);
}

static int rpmsg_virtio_send_offchannel_nocopy(struct rpmsg_device *rdev,
					       uint32_t src, uint32_t dst,
					       const void *data, int len)
{
	struct rpmsg_virtio_device *rvdev;
	struct metal_io_region *io;
	struct rpmsg_hdr rp_hdr;
	struct rpmsg_hdr *hdr;
	uint32_t buff_len;
	uint16_t idx;
	int status;

	/* Get the associated remote device for channel. */
	rvdev = metal_container_of(rdev, struct rpmsg_virtio_device, rdev);

	hdr = RPMSG_LOCATE_HDR(data);
	/* The reserved field contains buffer index */
	idx = hdr->reserved;

	/* Initialize RPMSG header. */
	rp_hdr.dst = dst;
	rp_hdr.src = src;
	rp_hdr.len = len;
	rp_hdr.reserved = 0;
	rp_hdr.flags = 0;

	/* Copy data to rpmsg buffer. */
	io = rvdev->shbuf_io;
	status = metal_io_block_write(io, metal_io_virt_to_offset(io, hdr),
				      &rp_hdr, sizeof(rp_hdr));
	RPMSG_ASSERT(status == sizeof(rp_hdr), "failed to write header\r\n");

	metal_mutex_acquire(&rdev->lock);

#ifndef VIRTIO_DEVICE_ONLY
	if (rpmsg_virtio_get_role(rvdev) == RPMSG_HOST)
		buff_len = rvdev->config.h2r_buf_size;
	else
#endif /*!VIRTIO_DEVICE_ONLY*/
		buff_len = virtqueue_get_buffer_length(rvdev->svq, idx);

	/* Enqueue buffer on virtqueue. */
	status = rpmsg_virtio_enqueue_buffer(rvdev, hdr, buff_len, idx);
	RPMSG_ASSERT(status == VQUEUE_SUCCESS, "failed to enqueue buffer\r\n");
	/* Let the other side know that there is a job to process. */
	virtqueue_kick(rvdev->svq);

	metal_mutex_release(&rdev->lock);

	return len;
}

static int rpmsg_virtio_release_tx_buffer(struct rpmsg_device *rdev, void *txbuf)
{
	struct rpmsg_virtio_device *rvdev;
	struct rpmsg_hdr *rp_hdr = RPMSG_LOCATE_HDR(txbuf);
	void *vbuff = rp_hdr;  /* only used to avoid warning on the cast of a packed structure */
	struct vbuff_reclaimer_t *r_desc = (struct vbuff_reclaimer_t *)vbuff;
	uint16_t idx;

	/*
	 * Reuse the RPMsg buffer to temporary store the vbuff_reclaimer_t structure.
	 * Stores the index locally before overwriting the RPMsg header.
	 */
	idx = rp_hdr->reserved;

	rvdev = metal_container_of(rdev, struct rpmsg_virtio_device, rdev);

	metal_mutex_acquire(&rdev->lock);

	r_desc->idx = idx;
	metal_list_add_tail(&rvdev->reclaimer, &r_desc->node);

	metal_mutex_release(&rdev->lock);

	return RPMSG_SUCCESS;
}

/**
 * @internal
 *
 * @brief This function sends rpmsg "message" to remote device.
 *
 * @param rdev	Pointer to rpmsg device
 * @param src	Source address of channel
 * @param dst	Destination address of channel
 * @param data	Data to transmit
 * @param len	Size of data
 * @param wait	Boolean, wait or not for buffer to become
 *		available
 *
 * @return Size of data sent or negative value for failure.
 */
static int rpmsg_virtio_send_offchannel_raw(struct rpmsg_device *rdev,
					    uint32_t src, uint32_t dst,
					    const void *data,
					    int len, int wait)
{
	struct rpmsg_virtio_device *rvdev;
	struct metal_io_region *io;
	uint32_t buff_len;
	void *buffer;
	int status;

	/* Get the associated remote device for channel. */
	rvdev = metal_container_of(rdev, struct rpmsg_virtio_device, rdev);

	/* Get the payload buffer. */
	buffer = rpmsg_virtio_get_tx_payload_buffer(rdev, &buff_len, wait);
	if (!buffer)
		return RPMSG_ERR_NO_BUFF;

	/* Copy data to rpmsg buffer. */
	if (len > (int)buff_len)
		len = buff_len;
	io = rvdev->shbuf_io;
	status = metal_io_block_write(io, metal_io_virt_to_offset(io, buffer),
				      data, len);
	RPMSG_ASSERT(status == len, "failed to write buffer\r\n");

	return rpmsg_virtio_send_offchannel_nocopy(rdev, src, dst, buffer, len);
}

/**
 * @internal
 *
 * @brief Tx callback function.
 *
 * @param vq	Pointer to virtqueue on which Tx is has been
 *		completed.
 */
static void rpmsg_virtio_tx_callback(struct virtqueue *vq)
{
	(void)vq;
}

/**
 * @internal
 *
 * @brief Rx callback function.
 *
 * @param vq	Pointer to virtqueue on which messages is received
 */
static void rpmsg_virtio_rx_callback(struct virtqueue *vq)
{
	struct virtio_device *vdev = vq->vq_dev;
	struct rpmsg_virtio_device *rvdev = vdev->priv;
	struct rpmsg_device *rdev = &rvdev->rdev;
	struct rpmsg_endpoint *ept;
	struct rpmsg_hdr *rp_hdr;
	uint32_t len;
	uint16_t idx;
	int status;

	metal_mutex_acquire(&rdev->lock);

	/* Process the received data from remote node */
	rp_hdr = rpmsg_virtio_get_rx_buffer(rvdev, &len, &idx);

	metal_mutex_release(&rdev->lock);

	while (rp_hdr) {
		rp_hdr->reserved = idx;

		/* Get the channel node from the remote device channels list. */
		metal_mutex_acquire(&rdev->lock);
		ept = rpmsg_get_ept_from_addr(rdev, rp_hdr->dst);
		metal_mutex_release(&rdev->lock);

		if (ept) {
			if (ept->dest_addr == RPMSG_ADDR_ANY) {
				/*
				 * First message received from the remote side,
				 * update channel destination address
				 */
				ept->dest_addr = rp_hdr->src;
			}
			status = ept->cb(ept, RPMSG_LOCATE_DATA(rp_hdr),
					 rp_hdr->len, rp_hdr->src, ept->priv);

			RPMSG_ASSERT(status >= 0,
				     "unexpected callback status\r\n");
		}

		metal_mutex_acquire(&rdev->lock);

		/* Check whether callback wants to hold buffer */
		if (!(rp_hdr->reserved & RPMSG_BUF_HELD)) {
			/* No, return used buffers. */
			rpmsg_virtio_return_buffer(rvdev, rp_hdr, len, idx);
		}

		rp_hdr = rpmsg_virtio_get_rx_buffer(rvdev, &len, &idx);
		if (!rp_hdr) {
			/* tell peer we return some rx buffer */
			virtqueue_kick(rvdev->rvq);
		}
		metal_mutex_release(&rdev->lock);
	}
}

/**
 * @internal
 *
 * @brief This callback handles name service announcement from the remote
 * device and creates/deletes rpmsg channels.
 *
 * @param ept	Pointer to server channel control block.
 * @param data	Pointer to received messages
 * @param len	Length of received data
 * @param priv	Any private data
 * @param src	Source address
 *
 * @return Rpmsg endpoint callback handled
 */
static int rpmsg_virtio_ns_callback(struct rpmsg_endpoint *ept, void *data,
				    size_t len, uint32_t src, void *priv)
{
	struct rpmsg_device *rdev = ept->rdev;
	struct rpmsg_virtio_device *rvdev = (struct rpmsg_virtio_device *)rdev;
	struct metal_io_region *io = rvdev->shbuf_io;
	struct rpmsg_endpoint *_ept;
	struct rpmsg_ns_msg *ns_msg;
	uint32_t dest;
	char name[RPMSG_NAME_SIZE];

	(void)priv;
	(void)src;

	ns_msg = data;
	if (len != sizeof(*ns_msg))
		/* Returns as the message is corrupted */
		return RPMSG_SUCCESS;
	metal_io_block_read(io,
			    metal_io_virt_to_offset(io, ns_msg->name),
			    &name, sizeof(name));
	dest = ns_msg->addr;

	/* check if a Ept has been locally registered */
	metal_mutex_acquire(&rdev->lock);
	_ept = rpmsg_get_endpoint(rdev, name, RPMSG_ADDR_ANY, dest);

	if (ns_msg->flags & RPMSG_NS_DESTROY) {
		if (_ept)
			_ept->dest_addr = RPMSG_ADDR_ANY;
		metal_mutex_release(&rdev->lock);
		if (_ept && _ept->ns_unbind_cb)
			_ept->ns_unbind_cb(_ept);
		if (rdev->ns_unbind_cb)
			rdev->ns_unbind_cb(rdev, name, dest);
	} else {
		if (!_ept) {
			/*
			 * send callback to application, that can
			 * - create the associated endpoints.
			 * - store information for future use.
			 * - just ignore the request as service not supported.
			 */
			metal_mutex_release(&rdev->lock);
			if (rdev->ns_bind_cb)
				rdev->ns_bind_cb(rdev, name, dest);
		} else {
			_ept->dest_addr = dest;
			metal_mutex_release(&rdev->lock);
		}
	}

	return RPMSG_SUCCESS;
}

int rpmsg_virtio_get_buffer_size(struct rpmsg_device *rdev)
{
	int size;
	struct rpmsg_virtio_device *rvdev;

	if (!rdev)
		return RPMSG_ERR_PARAM;
	metal_mutex_acquire(&rdev->lock);
	rvdev = (struct rpmsg_virtio_device *)rdev;
	size = _rpmsg_virtio_get_buffer_size(rvdev);
	metal_mutex_release(&rdev->lock);
	return size;
}

int rpmsg_init_vdev(struct rpmsg_virtio_device *rvdev,
		    struct virtio_device *vdev,
		    rpmsg_ns_bind_cb ns_bind_cb,
		    struct metal_io_region *shm_io,
		    struct rpmsg_virtio_shm_pool *shpool)
{
	return rpmsg_init_vdev_with_config(rvdev, vdev, ns_bind_cb, shm_io,
			   shpool, RPMSG_VIRTIO_DEFAULT_CONFIG);
}

int rpmsg_init_vdev_with_config(struct rpmsg_virtio_device *rvdev,
				struct virtio_device *vdev,
				rpmsg_ns_bind_cb ns_bind_cb,
				struct metal_io_region *shm_io,
				struct rpmsg_virtio_shm_pool *shpool,
				const struct rpmsg_virtio_config *config)
{
	struct rpmsg_device *rdev;
	const char *vq_names[RPMSG_NUM_VRINGS];
	vq_callback callback[RPMSG_NUM_VRINGS];
	int status;
	unsigned int i, role;

	if (!rvdev || !vdev || !shm_io)
		return RPMSG_ERR_PARAM;

	rdev = &rvdev->rdev;
	memset(rdev, 0, sizeof(*rdev));
	metal_mutex_init(&rdev->lock);
	rvdev->vdev = vdev;
	rdev->ns_bind_cb = ns_bind_cb;
	vdev->priv = rvdev;
	rdev->ops.send_offchannel_raw = rpmsg_virtio_send_offchannel_raw;
	rdev->ops.hold_rx_buffer = rpmsg_virtio_hold_rx_buffer;
	rdev->ops.release_rx_buffer = rpmsg_virtio_release_rx_buffer;
	rdev->ops.get_tx_payload_buffer = rpmsg_virtio_get_tx_payload_buffer;
	rdev->ops.send_offchannel_nocopy = rpmsg_virtio_send_offchannel_nocopy;
	rdev->ops.release_tx_buffer = rpmsg_virtio_release_tx_buffer;
	role = rpmsg_virtio_get_role(rvdev);

#ifndef VIRTIO_DEVICE_ONLY
	if (role == RPMSG_HOST) {
		/*
		 * The virtio configuration contains only options applicable to
		 * a virtio driver, implying rpmsg host role.
		 */
		if (config == NULL) {
			return RPMSG_ERR_PARAM;
		}
		rvdev->config = *config;
	}
#else /*!VIRTIO_DEVICE_ONLY*/
	/* Ignore passed config in the virtio-device-only configuration. */
	(void)config;
#endif /*!VIRTIO_DEVICE_ONLY*/


#ifndef VIRTIO_DRIVER_ONLY
	if (role == RPMSG_REMOTE) {
		/* wait synchro with the host */
		rpmsg_virtio_wait_remote_ready(rvdev);
	}
#endif /*!VIRTIO_DRIVER_ONLY*/
	vdev->features = rpmsg_virtio_get_features(rvdev);
	rdev->support_ns = !!(vdev->features & (1 << VIRTIO_RPMSG_F_NS));

#ifndef VIRTIO_DEVICE_ONLY
	if (role == RPMSG_HOST) {
		/*
		 * Since device is RPMSG Remote so we need to manage the
		 * shared buffers. Create shared memory pool to handle buffers.
		 */
		rvdev->shpool = config->split_shpool ? shpool + 1 : shpool;
		if (!shpool)
			return RPMSG_ERR_PARAM;
		if (!shpool->size || !rvdev->shpool->size)
			return RPMSG_ERR_NO_BUFF;

		vq_names[0] = "rx_vq";
		vq_names[1] = "tx_vq";
		callback[0] = rpmsg_virtio_rx_callback;
		callback[1] = rpmsg_virtio_tx_callback;
		rvdev->rvq  = vdev->vrings_info[0].vq;
		rvdev->svq  = vdev->vrings_info[1].vq;
	}
#endif /*!VIRTIO_DEVICE_ONLY*/

#ifndef VIRTIO_DRIVER_ONLY
	(void)shpool;
	if (role == RPMSG_REMOTE) {
		vq_names[0] = "tx_vq";
		vq_names[1] = "rx_vq";
		callback[0] = rpmsg_virtio_tx_callback;
		callback[1] = rpmsg_virtio_rx_callback;
		rvdev->rvq  = vdev->vrings_info[1].vq;
		rvdev->svq  = vdev->vrings_info[0].vq;
	}
#endif /*!VIRTIO_DRIVER_ONLY*/
	rvdev->shbuf_io = shm_io;
	metal_list_init(&rvdev->reclaimer);

	/* Create virtqueues for remote device */
	status = rpmsg_virtio_create_virtqueues(rvdev, 0, RPMSG_NUM_VRINGS,
						vq_names, callback);
	if (status != RPMSG_SUCCESS)
		return status;

	/*
	 * Suppress "tx-complete" interrupts
	 * since send method use busy loop when buffer pool exhaust
	 */
	virtqueue_disable_cb(rvdev->svq);

	/* TODO: can have a virtio function to set the shared memory I/O */
	for (i = 0; i < RPMSG_NUM_VRINGS; i++) {
		struct virtqueue *vq;

		vq = vdev->vrings_info[i].vq;
		vq->shm_io = shm_io;
	}

#ifndef VIRTIO_DEVICE_ONLY
	if (role == RPMSG_HOST) {
		struct virtqueue_buf vqbuf;
		unsigned int idx;
		void *buffer;

		vqbuf.len = rvdev->config.r2h_buf_size;
		for (idx = 0; idx < rvdev->rvq->vq_nentries; idx++) {
			/* Initialize TX virtqueue buffers for remote device */
			buffer = rpmsg_virtio_shm_pool_get_buffer(shpool,
					rvdev->config.r2h_buf_size);

			if (!buffer) {
				return RPMSG_ERR_NO_BUFF;
			}

			vqbuf.buf = buffer;

			metal_io_block_set(shm_io,
					   metal_io_virt_to_offset(shm_io,
								   buffer),
					   0x00, rvdev->config.r2h_buf_size);
			status =
				virtqueue_add_buffer(rvdev->rvq, &vqbuf, 0, 1,
						     buffer);

			if (status != RPMSG_SUCCESS) {
				return status;
			}
		}
	}
#endif /*!VIRTIO_DEVICE_ONLY*/

	/* Initialize channels and endpoints list */
	metal_list_init(&rdev->endpoints);

	/*
	 * Create name service announcement endpoint if device supports name
	 * service announcement feature.
	 */
	if (rdev->support_ns) {
		rpmsg_register_endpoint(rdev, &rdev->ns_ept, "NS",
				     RPMSG_NS_EPT_ADDR, RPMSG_NS_EPT_ADDR,
				     rpmsg_virtio_ns_callback, NULL);
	}

#ifndef VIRTIO_DEVICE_ONLY
	if (role == RPMSG_HOST)
		rpmsg_virtio_set_status(rvdev, VIRTIO_CONFIG_STATUS_DRIVER_OK);
#endif /*!VIRTIO_DEVICE_ONLY*/

	return status;
}

void rpmsg_deinit_vdev(struct rpmsg_virtio_device *rvdev)
{
	struct metal_list *node;
	struct rpmsg_device *rdev;
	struct rpmsg_endpoint *ept;

	if (rvdev) {
		rdev = &rvdev->rdev;
		while (!metal_list_is_empty(&rdev->endpoints)) {
			node = rdev->endpoints.next;
			ept = metal_container_of(node, struct rpmsg_endpoint, node);
			rpmsg_destroy_ept(ept);
		}

		rvdev->rvq = 0;
		rvdev->svq = 0;

		metal_mutex_deinit(&rdev->lock);
	}
}
