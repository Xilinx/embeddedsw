/*
 * rpmsg based on virtio
 *
 * Copyright (C) 2018 Linaro, Inc.
 *
 * All rights reserved.
 * Copyright (c) 2016 Freescale Semiconductor, Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _RPMSG_VIRTIO_H_
#define _RPMSG_VIRTIO_H_

#include <metal/io.h>
#include <metal/mutex.h>
#include <metal/cache.h>
#include <openamp/rpmsg.h>
#include <openamp/virtio.h>

#if defined __cplusplus
extern "C" {
#endif

/* Configurable parameters */
#ifndef RPMSG_BUFFER_SIZE
#define RPMSG_BUFFER_SIZE	(512)
#endif

/* The feature bitmap for virtio rpmsg */
#define VIRTIO_RPMSG_F_NS	0 /* RP supports name service notifications */

#ifdef VIRTIO_CACHED_BUFFERS
#warning "VIRTIO_CACHED_BUFFERS is deprecated, please use VIRTIO_USE_DCACHE"
#endif
#if defined(VIRTIO_CACHED_BUFFERS) || defined(VIRTIO_USE_DCACHE)
#define BUFFER_FLUSH(x, s)		CACHE_FLUSH(x, s)
#define BUFFER_INVALIDATE(x, s)		CACHE_INVALIDATE(x, s)
#else
#define BUFFER_FLUSH(x, s)		do { } while (0)
#define BUFFER_INVALIDATE(x, s)		do { } while (0)
#endif /* VIRTIO_CACHED_BUFFERS || VIRTIO_USE_DCACHE */

/** @brief Shared memory pool used for RPMsg buffers */
struct rpmsg_virtio_shm_pool {
	/** Base address of the memory pool */
	void *base;

	/** Available memory size */
	size_t avail;

	/** Total pool size */
	size_t size;
};

/**
 * @brief Configuration of RPMsg device based on virtio
 *
 * This structure is used by the RPMsg virtio host to configure the virtiio
 * layer.
 */
struct rpmsg_virtio_config {
	/** The size of the buffer used to send data from host to remote */
	uint32_t h2r_buf_size;

	/** The size of the buffer used to send data from remote to host */
	uint32_t r2h_buf_size;

	/** The flag for splitting shared memory pool to TX and RX */
	bool split_shpool;
};

/** @brief Representation of a RPMsg device based on virtio */
struct rpmsg_virtio_device {
	/** RPMsg device */
	struct rpmsg_device rdev;

	/** Structure containing virtio configuration */
	struct rpmsg_virtio_config config;

	/** Pointer to the virtio device */
	struct virtio_device *vdev;

	/** Pointer to receive virtqueue */
	struct virtqueue *rvq;

	/** Pointer to send virtqueue */
	struct virtqueue *svq;

	/** Pointer to the shared buffer I/O region */
	struct metal_io_region *shbuf_io;

	/** Pointer to the shared buffers pool */
	struct rpmsg_virtio_shm_pool *shpool;

	/**
	 * RPMsg buffer reclaimer that contains buffers released by the
	 * \ref rpmsg_virtio_release_tx_buffer function
	 */
	struct metal_list reclaimer;
};

#define RPMSG_REMOTE	VIRTIO_DEV_DEVICE
#define RPMSG_HOST	VIRTIO_DEV_DRIVER

#define RPMSG_SLAVE        deprecated_rpmsg_slave()
#define RPMSG_MASTER       deprecated_rpmsg_master()

__deprecated static inline int deprecated_rpmsg_master(void)
{
	/* "RPMSG_MASTER is deprecated, please use RPMSG_HOST" */
	return RPMSG_HOST;
}

__deprecated static inline int deprecated_rpmsg_slave(void)
{
	/* "RPMSG_SLAVE is deprecated, please use RPMSG_REMOTE" */
	return RPMSG_REMOTE;
}

static inline unsigned int
rpmsg_virtio_get_role(struct rpmsg_virtio_device *rvdev)
{
	return rvdev->vdev->role;
}

static inline void rpmsg_virtio_set_status(struct rpmsg_virtio_device *rvdev,
					   uint8_t status)
{
	rvdev->vdev->func->set_status(rvdev->vdev, status);
}

static inline uint8_t rpmsg_virtio_get_status(struct rpmsg_virtio_device *rvdev)
{
	return rvdev->vdev->func->get_status(rvdev->vdev);
}

static inline uint32_t
rpmsg_virtio_get_features(struct rpmsg_virtio_device *rvdev)
{
	return rvdev->vdev->func->get_features(rvdev->vdev);
}

static inline void
rpmsg_virtio_read_config(struct rpmsg_virtio_device *rvdev,
			 uint32_t offset, void *dst, int length)
{
	rvdev->vdev->func->read_config(rvdev->vdev, offset, dst, length);
}

static inline void
rpmsg_virtio_write_config(struct rpmsg_virtio_device *rvdev,
			 uint32_t offset, void *dst, int length)
{
	rvdev->vdev->func->write_config(rvdev->vdev, offset, dst, length);
}

static inline int
rpmsg_virtio_create_virtqueues(struct rpmsg_virtio_device *rvdev,
			       int flags, unsigned int nvqs,
			       const char *names[],
			       vq_callback *callbacks)
{
	return virtio_create_virtqueues(rvdev->vdev, flags, nvqs, names,
					callbacks, NULL);
}

/**
 * @brief Get rpmsg virtio buffer size
 *
 * @param rdev	Pointer to the rpmsg device
 *
 * @return Next available buffer size for text, negative value for failure
 */
int rpmsg_virtio_get_buffer_size(struct rpmsg_device *rdev);

/**
 * @brief Initialize rpmsg virtio device
 *
 * Host side:
 * Initialize RPMsg virtio queues and shared buffers, the address of shm can be
 * ANY. In this case, function will get shared memory from system shared memory
 * pools. If the vdev has the RPMsg name service feature, this API will create
 * a name service endpoint.
 *
 * Remote side:
 * This API will not return until the driver ready is set by the host side.
 *
 * @param rvdev		Pointer to the rpmsg virtio device
 * @param vdev		Pointer to the virtio device
 * @param ns_bind_cb	Callback handler for name service announcement without
 *                      local endpoints waiting to bind.
 * @param shm_io	Pointer to the share memory I/O region.
 * @param shpool	Pointer to shared memory pool.
 *			rpmsg_virtio_init_shm_pool has to be called first to
 *			fill this structure.
 *
 * @return Status of function execution
 */
int rpmsg_init_vdev(struct rpmsg_virtio_device *rvdev,
		    struct virtio_device *vdev,
		    rpmsg_ns_bind_cb ns_bind_cb,
		    struct metal_io_region *shm_io,
		    struct rpmsg_virtio_shm_pool *shpool);

/**
 * @brief Initialize rpmsg virtio device with config
 *
 * Host side:
 * Initialize RPMsg virtio queues and shared buffers, the address of shm can be
 * ANY. In this case, function will get shared memory from system shared memory
 * pools. If the vdev has the RPMsg name service feature, this API will create
 * a name service endpoint.
 * Sizes of virtio data buffers used by the initialized RPMsg instance are set
 * to values read from the passed configuration structure.
 *
 * Remote side:
 * This API will not return until the driver ready is set by the host side.
 * Sizes of virtio data buffers are set by the host side. Values passed in the
 * configuration structure have no effect.
 *
 * @param rvdev		Pointer to the rpmsg virtio device
 * @param vdev		Pointer to the virtio device
 * @param ns_bind_cb	Callback handler for name service announcement without
 *                      local endpoints waiting to bind.
 * @param shm_io	Pointer to the share memory I/O region.
 * @param shpool	Pointer to shared memory pool array.
 *			If the config->split_shpool is turn on, the array will
 *			contain two elements, the shpool of txshpool and
 *			rxshpool, Otherwise, the array has only one element,
 *			and txshpool rxshpool shares a shpool.
 *			And rpmsg_virtio_init_shm_pool has to be called first
 *			to fill each shpool in this array.
 * @param config	Pointer to configuration structure
 *
 * @return Status of function execution
 */
int rpmsg_init_vdev_with_config(struct rpmsg_virtio_device *rvdev,
				struct virtio_device *vdev,
				rpmsg_ns_bind_cb ns_bind_cb,
				struct metal_io_region *shm_io,
				struct rpmsg_virtio_shm_pool *shpool,
				const struct rpmsg_virtio_config *config);

/**
 * @brief Deinitialize rpmsg virtio device
 *
 * @param rvdev	Pointer to the rpmsg virtio device
 */
void rpmsg_deinit_vdev(struct rpmsg_virtio_device *rvdev);

/**
 * @brief Initialize default shared buffers pool
 *
 * RPMsg virtio has default shared buffers pool implementation.
 * The memory assigned to this pool will be dedicated to the RPMsg
 * virtio. This function has to be called before calling rpmsg_init_vdev,
 * to initialize the rpmsg_virtio_shm_pool structure.
 *
 * @param shpool	Pointer to the shared buffers pool structure
 * @param shbuf		Pointer to the beginning of shared buffers
 * @param size		Shared buffers total size
 */
void rpmsg_virtio_init_shm_pool(struct rpmsg_virtio_shm_pool *shpool,
				void *shbuf, size_t size);

/**
 * @brief Get RPMsg device from RPMsg virtio device
 *
 * @param rvdev	Pointer to RPMsg virtio device
 *
 * @return RPMsg device pointed by RPMsg virtio device
 */
static inline struct rpmsg_device *
rpmsg_virtio_get_rpmsg_device(struct rpmsg_virtio_device *rvdev)
{
	if (!rvdev)
		return NULL;

	return &rvdev->rdev;
}

/**
 * @brief Get buffer in the shared memory pool
 *
 * RPMsg virtio has default shared buffers pool implementation.
 * The memory assigned to this pool will be dedicated to the RPMsg
 * virtio. If you prefer to have other shared buffers allocation,
 * you can implement your rpmsg_virtio_shm_pool_get_buffer function.
 *
 * @param shpool	Pointer to the shared buffers pool
 * @param size		Shared buffers total size
 *
 * @return Buffer pointer if free buffer is available, NULL otherwise.
 */
metal_weak void *
rpmsg_virtio_shm_pool_get_buffer(struct rpmsg_virtio_shm_pool *shpool,
				 size_t size);

#if defined __cplusplus
}
#endif

#endif	/* _RPMSG_VIRTIO_H_ */
