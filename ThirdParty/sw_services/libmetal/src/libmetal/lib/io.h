/*
 * Copyright (c) 2015, Xilinx Inc. and Contributors. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * 3. Neither the name of Xilinx nor the names of its contributors may be used
 *    to endorse or promote products derived from this software without
 *    specific prior written permission.
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

/*
 * @file	io.h
 * @brief	I/O access primitives for libmetal.
 */

#ifndef __METAL_IO__H__
#define __METAL_IO__H__

#include <assert.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "metal/atomic.h"
#include "metal/sys.h"

#ifdef __cplusplus
extern "C" {
#endif

/** \defgroup io IO Interfaces
 *  @{ */

/** I/O memory flags macros for caching scheme */

/** Cache bits */
#define METAL_CACHE_UNKNOWN  0x0 /** Use cache, unknown cache type */
#define METAL_UNCACHED       0x1 /** No cache */
#define METAL_CACHE_WB       0x2 /** Write back */
#define METAL_CACHE_WT       0x4 /** Write through */

/** Memory types */
#define METAL_MEM_MAPPED      0x10 /** Memory mapped */
#define METAL_IO_MAPPED       0x20 /** I/O mapped */
#define METAL_SHARED_MEM      0x40 /** Shared memory */

/** DMA cache bits */
#define METAL_DMA_NO_CACHE_OPS 0x0  /** No cache ops in DMA transaction */
#define METAL_DMA_CACHE_OPS    0x1  /** Require cache ops in DMA transaction */

struct metal_io_region;

/** Generic I/O operations. */
struct metal_io_ops {
	uint64_t	(*read)(struct metal_io_region *io,
				unsigned long offset,
				memory_order order,
				int width);
	void		(*write)(struct metal_io_region *io,
				 unsigned long offset,
				 uint64_t value,
				 memory_order order,
				 int width);
	void		(*close)(struct metal_io_region *io);
};

/** Libmetal I/O region structure. */
struct metal_io_region {
	void			*virt;
	const metal_phys_addr_t	*physmap;
	size_t			size;
	unsigned long		page_shift;
	metal_phys_addr_t	page_mask;
	unsigned int		mem_flags;
	struct metal_io_ops	ops;
};

/**
 * @brief	Open a libmetal I/O region.
 *
 * @param[in, out]	io		I/O region handle.
 * @param[in]		virt		Virtual address of region.
 * @param[in]		physmap		Array of physical addresses per page.
 * @param[in]		size		Size of region.
 * @param[in]		page_shift	Log2 of page size (-1 for single page).
 * @param[in]		mem_flags	Memory flags
 * @param[in]		ops			ops
 */
static inline void
metal_io_init(struct metal_io_region *io, void *virt,
	      const metal_phys_addr_t *physmap, size_t size,
	      unsigned page_shift, unsigned int mem_flags,
	      const struct metal_io_ops *ops)
{
	const struct metal_io_ops nops = {NULL, NULL, NULL};
	io->virt = virt;
	io->physmap = physmap;
	io->size = size;
	io->page_shift = page_shift;
	io->page_mask = (1UL << page_shift) - 1UL;
	io->mem_flags = mem_flags;
	io->ops = ops ? *ops : nops;
}

/**
 * @brief	Close a libmetal shared memory segment.
 * @param[in]	io	I/O region handle.
 */
static inline void metal_io_finish(struct metal_io_region *io)
{
	if (io->ops.close)
		(*io->ops.close)(io);
	memset(io, 0, sizeof(*io));
}

/**
 * @brief	Get size of I/O region.
 *
 * @param[in]	io	I/O region handle.
 * @return	Size of I/O region.
 */
static inline size_t metal_io_region_size(struct metal_io_region *io)
{
	return io->size;
}

/**
 * @brief	Get virtual address for a given offset into the I/O region.
 * @param[in]	io	I/O region handle.
 * @param[in]	offset	Offset into shared memory segment.
 * @return	NULL if offset is out of range, or pointer to offset.
 */
static inline void *
metal_io_virt(struct metal_io_region *io, unsigned long offset)
{
#ifdef METAL_INVALID_IO_VADDR
	return (io->virt != METAL_INVALID_IO_VADDR && offset <= io->size
#else
	return (offset <= io->size
#endif
		? (uint8_t *)io->virt + offset
		: NULL);
}

/**
 * @brief	Convert a virtual address to offset within I/O region.
 * @param[in]	io	I/O region handle.
 * @param[in]	virt	Virtual address within segment.
 * @return	METAL_BAD_OFFSET if out of range, or offset.
 */
static inline unsigned long
metal_io_virt_to_offset(struct metal_io_region *io, void *virt)
{
	size_t offset = (uint8_t *)virt - (uint8_t *)io->virt;
	return (offset < io->size ? offset : METAL_BAD_OFFSET);
}

/**
 * @brief	Get physical address for a given offset into the I/O region.
 * @param[in]	io	I/O region handle.
 * @param[in]	offset	Offset into shared memory segment.
 * @return	METAL_BAD_PHYS if offset is out of range, or physical address
 *		of offset.
 */
static inline metal_phys_addr_t
metal_io_phys(struct metal_io_region *io, unsigned long offset)
{
	unsigned long page = offset >> io->page_shift;
	return (io->physmap != NULL && offset <= io->size
		&& io->physmap[page] != METAL_BAD_PHYS
		? io->physmap[page] + (offset & io->page_mask)
		: METAL_BAD_PHYS);
}

/**
 * @brief	Convert a physical address to offset within I/O region.
 * @param[in]	io	I/O region handle.
 * @param[in]	phys	Physical address within segment.
 * @return	METAL_BAD_OFFSET if out of range, or offset.
 */
static inline unsigned long
metal_io_phys_to_offset(struct metal_io_region *io, metal_phys_addr_t phys)
{
	unsigned long offset =
		(io->page_mask == (metal_phys_addr_t)(-1) ?
		phys - io->physmap[0] :  phys & io->page_mask);
	do {
		if (metal_io_phys(io, offset) == phys)
			return offset;
		offset += io->page_mask + 1;
	} while (offset < io->size);
	return METAL_BAD_OFFSET;
}

/**
 * @brief	Convert a physical address to virtual address.
 * @param[in]	io	Shared memory segment handle.
 * @param[in]	phys	Physical address within segment.
 * @return	NULL if out of range, or corresponding virtual address.
 */
static inline void *
metal_io_phys_to_virt(struct metal_io_region *io, metal_phys_addr_t phys)
{
	return metal_io_virt(io, metal_io_phys_to_offset(io, phys));
}

/**
 * @brief	Convert a virtual address to physical address.
 * @param[in]	io	Shared memory segment handle.
 * @param[in]	virt	Virtual address within segment.
 * @return	METAL_BAD_PHYS if out of range, or corresponding
 *		physical address.
 */
static inline metal_phys_addr_t
metal_io_virt_to_phys(struct metal_io_region *io, void *virt)
{
	return metal_io_phys(io, metal_io_virt_to_offset(io, virt));
}

/**
 * @brief	Read a value from an I/O region.
 * @param[in]	io	I/O region handle.
 * @param[in]	offset	Offset into I/O region.
 * @param[in]	order	Memory ordering.
 * @param[in]	width	Width in bytes of datatype to read.  This must be 1, 2,
 *			4, or 8, and a compile time constant for this function
 *			to inline cleanly.
 * @return	Value.
 */
static inline uint64_t
metal_io_read(struct metal_io_region *io, unsigned long offset,
	      memory_order order, int width)
{
	void *ptr = metal_io_virt(io, offset);
	if (io->ops.read)
		return (*io->ops.read)(io, offset, order, width);
	else if (ptr && sizeof(atomic_uchar) == width)
		return atomic_load_explicit((atomic_uchar *)ptr, order);
	else if (ptr && sizeof(atomic_ushort) == width)
		return atomic_load_explicit((atomic_ushort *)ptr, order);
	else if (ptr && sizeof(atomic_uint) == width)
		return atomic_load_explicit((atomic_uint *)ptr, order);
	else if (ptr && sizeof(atomic_ulong) == width)
		return atomic_load_explicit((atomic_ulong *)ptr, order);
	else if (ptr && sizeof(atomic_ullong) == width)
		return atomic_load_explicit((atomic_ullong *)ptr, order);
	else
		assert(0);
}

/**
 * @brief	Write a value into an I/O region.
 * @param[in]	io	I/O region handle.
 * @param[in]	offset	Offset into I/O region.
 * @param[in]	value	Value to write.
 * @param[in]	order	Memory ordering.
 * @param[in]	width	Width in bytes of datatype to read.  This must be 1, 2,
 *			4, or 8, and a compile time constant for this function
 *			to inline cleanly.
 */
static inline void
metal_io_write(struct metal_io_region *io, unsigned long offset,
	       uint64_t value, memory_order order, int width)
{
	void *ptr = metal_io_virt(io, offset);
	if (io->ops.write)
		(*io->ops.write)(io, offset, value, order, width);
	else if (ptr && sizeof(atomic_uchar) == width)
		atomic_store_explicit((atomic_uchar *)ptr, value, order);
	else if (ptr && sizeof(atomic_ushort) == width)
		atomic_store_explicit((atomic_ushort *)ptr, value, order);
	else if (ptr && sizeof(atomic_uint) == width)
		atomic_store_explicit((atomic_uint *)ptr, value, order);
	else if (ptr && sizeof(atomic_ulong) == width)
		atomic_store_explicit((atomic_ulong *)ptr, value, order);
	else if (ptr && sizeof(atomic_ullong) == width)
		atomic_store_explicit((atomic_ullong *)ptr, value, order);
	else
		assert (0);
}

#define metal_io_read8_explicit(_io, _ofs, _order)			\
	metal_io_read((_io), (_ofs), (_order), 1)
#define metal_io_read8(_io, _ofs)					\
	metal_io_read((_io), (_ofs), memory_order_seq_cst, 1)
#define metal_io_write8_explicit(_io, _ofs, _val, _order)		\
	metal_io_write((_io), (_ofs), (_val), (_order), 1)
#define metal_io_write8(_io, _ofs, _val)				\
	metal_io_write((_io), (_ofs), (_val), memory_order_seq_cst, 1)

#define metal_io_read16_explicit(_io, _ofs, _order)			\
	metal_io_read((_io), (_ofs), (_order), 2)
#define metal_io_read16(_io, _ofs)					\
	metal_io_read((_io), (_ofs), memory_order_seq_cst, 2)
#define metal_io_write16_explicit(_io, _ofs, _val, _order)		\
	metal_io_write((_io), (_ofs), (_val), (_order), 2)
#define metal_io_write16(_io, _ofs, _val)				\
	metal_io_write((_io), (_ofs), (_val), memory_order_seq_cst, 2)

#define metal_io_read32_explicit(_io, _ofs, _order)			\
	metal_io_read((_io), (_ofs), (_order), 4)
#define metal_io_read32(_io, _ofs)					\
	metal_io_read((_io), (_ofs), memory_order_seq_cst, 4)
#define metal_io_write32_explicit(_io, _ofs, _val, _order)		\
	metal_io_write((_io), (_ofs), (_val), (_order), 4)
#define metal_io_write32(_io, _ofs, _val)				\
	metal_io_write((_io), (_ofs), (_val), memory_order_seq_cst, 4)

#define metal_io_read64_explicit(_io, _ofs, _order)			\
	metal_io_read((_io), (_ofs), (_order), 8)
#define metal_io_read64(_io, _ofs)					\
	metal_io_read((_io), (_ofs), memory_order_seq_cst, 8)
#define metal_io_write64_explicit(_io, _ofs, _val, _order)		\
	metal_io_write((_io), (_ofs), (_val), (_order), 8)
#define metal_io_write64(_io, _ofs, _val)				\
	metal_io_write((_io), (_ofs), (_val), memory_order_seq_cst, 8)

/**
 * @brief	libmetal memory map
 *
 * This function is to enable memory mapping to the specified I/O memory.
 *
 * @param[in]	pa     physical memory start address
 * @param[in]   io     memory region
 * @param[in]   size   size of the memory range
 * @return	logical address if suceeded, or 0 if failed.
 */
void *metal_io_mem_map(metal_phys_addr_t pa,
		     struct metal_io_region *io,
		     size_t size);

/**
 * @brief	libmetal set device memory
 *
 * This function is to fill the device memory with the specified value.
 *
 * @param[in]   dst  target memory
 * @param[in]   c    val to fill
 * @param[in]   size size of memory to fill.
 * @return	pointer to the target memory
 */
void *metal_memset_io(void *dst, int c, size_t size);

/**
 * @brief	libmetal copy to target memory
 *
 * This function is to copy specified memory area.
 * The source memory or the destination memory can be device memory.
 *
 * @param[in]   dst    target memory
 * @param[in]   src    source memory
 * @param[in]   size   size of memory to copy.
 * @return	pointer to the target memory
 */
void *metal_memcpy_io(void *dst, const void *src, size_t size);

#ifdef __cplusplus
}
#endif

#endif /* __METAL_IO__H__ */
