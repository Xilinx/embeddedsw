/*
 * Copyright (c) 2015, Xilinx Inc. and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*
 * @file	shmem.h
 * @brief	Shared memory primitives for libmetal.
 */

#ifndef __METAL_SHMEM__H__
#define __METAL_SHMEM__H__

#include <metal/device.h>
#include <metal/io.h>
#include <metal/mutex.h>
#include <metal/scatterlist.h>
#include <metal/shmem-provider.h>

#ifdef __cplusplus
extern "C" {
#endif

/** \defgroup shmem Shared Memory Interfaces
 *  @{ */

#define METAL_SHM_DIR_DEV_R  1U /**< shmem direction, device read */
#define METAL_SHM_DIR_DEV_W  2U /**< shmem direction, device write */
#define METAL_SHM_DIR_DEV_RW 3U /**< shmem direction, device read/write */

#define METAL_SHM_NOTCACHED  1U /**< shmem not cached */

struct metal_generic_shmem;
struct metal_shm_ops {
	/**
	 * @brief shmem sync for device function
	 * @param[in]	shm		shmem handle
	 * @param[in]	dev		device handle
	 * @param[out]	direction	device write or read
	 * @return 0 for sucess, or negative value for failure
	 */
	int (*sync_for_device)(struct metal_generic_shmem *shm,
			       struct metal_device *dev,
			       unsigned int direction);
	/**
	 * @brief shmem sync for cpu function
	 * @param[in]	shm		shmem handle
	 * @param[out]	direction	device write or read
	 * @return 0 for sucess, or negative value for failure
	 */
	int (*sync_for_cpu)(struct metal_generic_shmem *shm,
			    unsigned int direction);
	/**
	 * @brief shmem mmap function
	 * @param[in]	shm		shmem handle
	 * @param[in]	size		mmap shmem size
	 * @param[out]	sg		scatter list of the mmap result
	 */
	int (*mmap)(struct metal_generic_shmem *shm,
		    size_t size,
		    struct metal_scatter_list *sg);
	/**
	 * @brief shmem munmap function
	 * @param[in]	shm		shmem handle
	 * @param[in]	sg		scatter list to unmap
	 */
	void (*munmap)(struct metal_generic_shmem *shm,
		       struct metal_scatter_list *sg);
};

/** Metal shmem reference structure */
struct metal_shm_ref {
	struct metal_scatter_list sg; /**< shmem scatter list */
	struct metal_device *dev; /**< device this link belongs to */
	unsigned flags; /**< reserved reference flags */
	struct metal_list node; /**< node */
};

/** Generic shared memory data structure. */
struct metal_generic_shmem {
	char			name[64]; /**< shared memory name */
	struct metal_scatter_list
				sg; /**< scatter list */
	size_t			size; /**< shared memory size */
	unsigned int		flags; /**< shared memory flag, cacheable,
					    or noncacheable */
	struct metal_shm_ops	*ops;  /**< shared memory operations */
	int			id;    /**< shared memory id */
	int			refcount; /**< reference count */
	struct metal_shm_provider
				*provider; /**< provider where this shared
						memory is from */
	struct metal_list	refs; /**< references list */
	metal_mutex_t		lock;  /**< lock */
	struct metal_list	node;  /**< memory node */
};

/**
 * @brief	Attach a shared memory to a device
 *
 * This operation will attach the shared memory to a device.
 * The device driver can notify the device to prepare
 * access to this shared memory.
 *
 * @param[in]	shmem Shared memory to assign to the device
 * @param[in]	dev Device the shared memory is attached to
 * @param[in]	direction shared memory direction
 *
 * @return attached shmem scatter list for success, NULL for failure.
 */
struct metal_scatter_list *
metal_shm_attach(struct metal_generic_shmem *shmem,
		 struct metal_device *dev,
		 unsigned int direction);

/**
 * @brief	Detach a shared memory from a device
 *
 * This operation will detach the shared memory from a device.
 * The device driver can notify the device not to use this
 * shared memory any more.
 *
 * @param[in]	shmem Shared memory to assign to the device
 * @param[in]	dev Device the shared memory is detached to.
 */
void metal_shm_detach(struct metal_generic_shmem *shmem,
		     struct metal_device *dev);

/**
 * @brief	sync shared memory for device access
 *
 * This operation will make sure the device can write/read data
 * to/from the shared memory. The host is not expected to access the
 * shared memory until metal_shm_sync_for_cpu() is called.
 *
 * @param[in]	shmem Shared memory to attach to a device
 * @param[in]	dev device the shared memory will attach to
 * @param[in]	direction indicate if it is to write to the device
 *			  or read from the device.
 *
 * @return 0 for success, negative value for failure.
 */
int metal_shm_sync_for_device(struct metal_generic_shmem *shmem,
			      struct metal_device *dev,
			      unsigned int direction);

/**
 * @brief	sync shared memory for cpu access
 *
 * This operation will make sure the host can write/read data
 * to/from the shared memory. The device is not expected to
 * access the shared memory after this operation until
 * metal_shm_sync_for_device() is called.
 *
 * @param[in]	shmem Shared memory to attach to a device
 * @param[in]	direction indicate if sync is to write to
 *			  the device or read from it.
 * @return 0 for success, negative value for failure.
 */
int metal_shm_sync_for_cpu(struct metal_generic_shmem *shmem,
			    unsigned int direction);

/**
 * @brief	Open a libmetal shared memory segment.
 *
 * Open a shared memory segment. If the shared memory is not opened,
 * it will open the shared memory from the speicified provider.
 *
 * @param[in]		name	Name of segment to open.
 *				The name format is expected to be
 *				"/shmem_provider/shmem"
 * @param[in]		size	Size of segment.
 * @param[in]		flags	Shmem flags to indicate if it wants
 *				cacheable or not.
 * @param[out]		result	shared memory handle, if successful.
 * @return	0 on success, or -errno on failure.
 */
extern int metal_shmem_open(const char *name,
			    size_t size,
			    unsigned int flags,
			    struct metal_generic_shmem **result);

/**
 * @brief	Close a libmetal shared memory segment.
 *
 * Close a shared memory segment.
 *
 * @param[in]		shmem	Shared memory.
 * @return	0 on success, or -errno on failure.
 */
extern int metal_shmem_close(struct metal_generic_shmem *shmem);

/**
 * @brief	mmap the shared memory for CPU to access
 *
 * memory map the shared memory for CPU to access.
 *
 * @param[in]		shmem	Shared memory.
 * @param[in]		size	Size of segment.
 * @return	I/O regions scatter list for success, NULL for failure.
 */
extern struct metal_scatter_list *
metal_shmem_mmap(struct metal_generic_shmem *shmem, size_t size);

/**
 * @brief	munmap the shared memory for CPU to access
 *
 * Unmap the shared memory for CPU to access.
 *
 * @param[in]		shmem	Shared memory.
 * @param[in]		sg	Scatter list
 */
extern void
metal_shmem_munmap(struct metal_generic_shmem *shmem,
		   struct metal_scatter_list *sg);

/**
 * @brief	Statically register a generic shared memory region.
 *
 * Shared memory regions may be statically registered at application
 * initialization, or may be dynamically opened.  This interface is used for
 * static registration of regions.  Subsequent calls to metal_shmem_open() look
 * up in this list of pre-registered regions.
 *
 * @param[in]	shmem	Generic shmem structure.
 * @return 0 on success, or -errno on failure.
 */
extern int metal_shmem_register_generic(struct metal_generic_shmem *shmem);

#ifdef METAL_INTERNAL

/**
 * @brief	Open a statically registered shmem segment.
 *
 * This interface is meant for internal libmetal use within system specific
 * shmem implementations.
 *
 * @param[in]		name	Name of segment to open.
 * @param[in]		size	Size of segment.
 * @param[out]		result	Shared memory handle, if successful.
 * @return	0 on success, or -errno on failure.
 */
int metal_shmem_open_generic(const char *name, size_t size,
			     struct metal_generic_shmem **result);

/**
 * @brief Get provider name from full shared memory name
 *
 * This function is to get the shared memory provider name from a shared
 * memory name.
 *
 * @param[in]	name	Name of the shared memory
 * @param[out]	result	holder of the shared memory provider name
 * @return	length of provider name on success, or -errno on failure.
 */
int metal_shmem_get_provider_name(const char *name, char *result);

/**
 * @brief Get shared memory subname from full name
 *
 * This function is to get the shared memory subname from the full shared
 * memory name which includes the provider name.
 *
 * @param[in]	name	Name of the shared memory
 * @param[out]	result	holder of the shared memory subname
 * @return	length of shared memory subname on success,
 *		or -errno on failure.
 */
int metal_shmem_get_subname(const char *name, char *result);

#endif

/** @} */

#ifdef __cplusplus
}
#endif

#endif /* __METAL_SHMEM__H__ */
