/*
 * Copyright (c) 2016, Xilinx Inc. and Contributors. All rights reserved.
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
 * @file	dma.h
 * @brief	DMA primitives for libmetal.
 */

#ifndef __METAL_DMA__H__
#define __METAL_DMA__H__

#ifdef __cplusplus
extern "C" {
#endif

/** \defgroup dma DMA Interfaces
 *  @{ */

#include <stdint.h>
#include <metal/sys.h>

#define METAL_DMA_DEV_R  1 /**< DMA direction, device read */
#define METAL_DMA_DEV_W  2 /**< DMA direction, device write */
#define METAL_DMA_DEV_WR 3 /**< DMA direction, device read/write */

/**
 * @brief scatter/gather list element structure
 */
struct metal_sg {
	void *virt; /**< CPU virtual address */
	struct metal_io_region *io; /**< IO region */
	int len; /**< length */
};

struct metal_device;

/**
 * @brief      Map memory for DMA transaction.
 *             After the memory is DMA mapped, the memory should be
 *             accessed by the DMA device but not the CPU.
 *
 * @param[in]  dev       DMA device
 * @param[in]  dir       DMA direction
 * @param[in]  sg_in     sg list of memory to map
 * @param[in]  nents_in  number of sg list entries of memory to map
 * @param[out] sg_out    sg list of mapped memory
 * @return     number of mapped sg entries, -error on failure.
 */
int metal_dma_map(struct metal_device *dev,
		  uint32_t dir,
		  struct metal_sg *sg_in,
		  int nents_in,
		  struct metal_sg *sg_out);

/**
 * @brief      Unmap DMA memory
 *             After the memory is DMA unmapped, the memory should
 *             be accessed by the CPU but not the DMA device.
 *
 * @param[in]  dev       DMA device
 * @param[in]  dir       DMA direction
 * @param[in]  sg        sg list of mapped DMA memory
 * @param[in]  nents     number of sg list entries of DMA memory
 */
void metal_dma_unmap(struct metal_device *dev,
		  uint32_t dir,
		  struct metal_sg *sg,
		  int nents);

/** @} */

#ifdef __cplusplus
}
#endif

#endif /* __METAL_DMA__H__ */
