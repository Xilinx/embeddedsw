/******************************************************************************
* Copyright (C) 2022 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright 2022-2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/
/**
 * @file xv_warp_filter_l2.c
 * @addtogroup v_warp_filter Overview
 */

/***************************** Include Files *********************************/
#include "xil_types.h"
#include "xv_warp_filter_l2.h"
#include "sleep.h"
#include <stdlib.h>

/************************** Constant Definitions *****************************/
/** Pointer offset size for aligned memory allocation */
#define PTR_OFFSET_SZ sizeof(u16)
/** Wait time for flush done operation (in iterations) */
#define XV_WAIT_FOR_FLUSH_DONE		         (25)
/** Timeout value for flush done operation (in microseconds) */
#define XV_WAIT_FOR_FLUSH_DONE_TIMEOUT		 (2000)
/** Warp filter address width in bits */
#define WARP_FILTER_ADDR_WIDTH				 128

/**************************** Type Definitions *******************************/
/** Offset type for aligned memory management */
typedef u16 offset_t;

/***************** Macros (Inline Functions) Definitions *********************/
/** Macro to align a number up to the specified alignment boundary */
#ifndef align_up
#define align_up(num, align) \
    (((num) + ((align) - 1)) & ~((align) - 1))
#endif

/************************** Function Prototypes ******************************/
static void *XVWarpFilter_AlignedMalloc(size_t align, size_t size);
static void XVWarpFilter_AlignedFree(void * ptr);

/************************** Function Definitions *****************************/
/**
 * @brief Creates the descriptors for the warp filter.
 *
 * This function allocates and initializes a chain of descriptors for the
 * warp filter operation. Each descriptor is aligned to the address width
 * requirement and linked together in a chain.
 *
 * @param  InstancePtr is a pointer to the XV_warp_filter instance.
 * @param  num_descriptors is the number of descriptors to be created.
 *
 * @return XST_SUCCESS if descriptors created successfully.
 *         XST_FAILURE if descriptor creation failed.
 *
 * @note   Descriptors are allocated with aligned memory and linked in a chain.
 *
 ******************************************************************************/
s32 XVWarpFilter_SetNumOfDescriptors(XV_warp_filter *InstancePtr,
		u32 num_descriptors)
{
	XVWarpFilter_Desc *descptr = NULL, *currptr, *prevptr;
	u32 descnum;

	Xil_AssertNonvoid(InstancePtr);

	for (descnum = 0; descnum < num_descriptors; descnum++)
	{
		currptr = (XVWarpFilter_Desc *)XVWarpFilter_AlignedMalloc(WARP_FILTER_ADDR_WIDTH/8,
				sizeof(XVWarpFilter_Desc));
		memset((u32 *)currptr, 0, sizeof(XVWarpFilter_Desc));

		if (descnum == 0)
			descptr = currptr;
		if (descnum > 0)
			prevptr->Warp_NextDescAddr = (u64)currptr;
		if (descnum == (num_descriptors - 1))
			currptr->Warp_NextDescAddr = (u64)0;

		prevptr = currptr;
	}

	InstancePtr->WarpFilterDesc_BaseAddr = (u64)descptr;
	InstancePtr->NumDescriptors = descnum;

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
 * @brief Clears and de-allocates all descriptors.
 *
 * This function traverses the descriptor chain and frees all allocated
 * memory for each descriptor, then resets the base address and count.
 *
 * @param  InstancePtr is a pointer to the XV_warp_filter instance.
 *
 * @return None.
 *
 * @note   This function must be called before the instance is destroyed to
 *         prevent memory leaks.
 *
 ******************************************************************************/
void XVWarpFilter_ClearNumOfDescriptors(XV_warp_filter *InstancePtr)
{
	XVWarpFilter_Desc *head, *tmpptr;

	Xil_AssertVoid(InstancePtr);

	head = (XVWarpFilter_Desc *)InstancePtr->WarpFilterDesc_BaseAddr;

	while(head->Warp_NextDescAddr) {
		tmpptr = (XVWarpFilter_Desc *)head->Warp_NextDescAddr;
		XVWarpFilter_AlignedFree(head);
		head = tmpptr;
	}

	XVWarpFilter_AlignedFree(head);
	InstancePtr->WarpFilterDesc_BaseAddr = 0;
	InstancePtr->NumDescriptors = 0;
}

/*****************************************************************************/
/**
 * @brief Programs a descriptor with given configurations.
 *
 * This function locates the specified descriptor and configures it with
 * the provided input parameters including dimensions, addresses, and
 * processing parameters.
 *
 * @param  InstancePtr is a pointer to the XV_warp_filter instance.
 * @param  DescNum is the descriptor number to be configured.
 * @param  configPtr is the input configuration pointer containing the
 *         configuration parameters.
 * @param  valid_seg is the number of valid segments in the remap vector.
 * @param  lblock_count is the number of output blocks decoded by the first
 *         warp filter core.
 * @param  line_num is the line number from which the second warp filter core
 *         starts reading the source image.
 *
 * @return XST_SUCCESS if programming descriptor is successful.
 *         XST_FAILURE if descriptor is not valid.
 *
 * @note   Descriptor number must be less than the total number of descriptors.
 *
 ******************************************************************************/
s32 XVWarpFilter_ProgramDescriptor(XV_warp_filter *InstancePtr, u32 DescNum,
		XVWarpFilter_InputConfigs *configPtr, u32 valid_seg,
		u32 lblock_count, u32 line_num)
{
	XVWarpFilter_Desc *descptr;
	u32 i;

	Xil_AssertNonvoid(InstancePtr);

	if (DescNum >= InstancePtr->NumDescriptors) {
		xil_printf("Wrong descriptor\n\r");
		return XST_FAILURE;
	}

	descptr = (XVWarpFilter_Desc *)InstancePtr->WarpFilterDesc_BaseAddr;
	for (i = 0; i < DescNum; i++) {
		descptr = (XVWarpFilter_Desc *)descptr->Warp_NextDescAddr;
	}

	descptr->height = configPtr->height;
	descptr->width = configPtr->width;
	descptr->stride = configPtr->stride;
	descptr->format = configPtr->format;
	descptr->valid_seg = valid_seg;
	descptr->lblock_count = lblock_count;
	descptr->line_num = line_num;
	descptr->reserved = 0;
	descptr->src_buf_addr = configPtr->src_buf_addr;
	descptr->seg_table_addr = configPtr->seg_table_addr;
	descptr->dest_buf_addr = configPtr->dest_buf_addr;

	XV_warp_filter_Set_desc_addr(InstancePtr, (u64)descptr);

	return XST_SUCCESS;
}

/**
 * @brief Sets the source frame buffer address in a descriptor.
 *
 * This function locates the specified descriptor and updates its source
 * frame buffer address with the provided value.
 *
 * @param  InstancePtr is a pointer to the XV_warp_filter instance.
 * @param  Descnum is the descriptor number to be configured.
 * @param  src_buf_addr is the address of the source frame buffer.
 *
 * @return XST_SUCCESS if programming descriptor is successful.
 *         XST_FAILURE if descriptor is not valid.
 *
 * @note   Descriptor number must be less than the total number of descriptors.
 *
 ******************************************************************************/
s32 XVWarpFilter_update_src_frame_addr(XV_warp_filter *InstancePtr,
		u32 Descnum, u64 src_buf_addr)
{
	XVWarpFilter_Desc *descptr;
	u32 i;

	Xil_AssertNonvoid(InstancePtr);

	if (Descnum >= InstancePtr->NumDescriptors) {
		xil_printf("Wrong descriptor\n\r");
		return XST_FAILURE;
	}

	descptr = (XVWarpFilter_Desc *)InstancePtr->WarpFilterDesc_BaseAddr;
	for (i = 0; i < Descnum; i++) {
		descptr = (XVWarpFilter_Desc *)descptr->Warp_NextDescAddr;
	}

	descptr->src_buf_addr = src_buf_addr;

	XV_warp_filter_Set_desc_addr(InstancePtr, (u64)descptr);

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
 * @brief Sets the destination frame buffer address in a descriptor.
 *
 * This function locates the specified descriptor and updates its destination
 * frame buffer address with the provided value.
 *
 * @param  InstancePtr is a pointer to the XV_warp_filter instance.
 * @param  Descnum is the descriptor number to be configured.
 * @param  dest_buf_addr is the address of the destination frame buffer.
 *
 * @return XST_SUCCESS if programming descriptor is successful.
 *         XST_FAILURE if descriptor is not valid.
 *
 * @note   Descriptor number must be less than the total number of descriptors.
 *
 ******************************************************************************/
s32 XVWarpFilter_update_dst_frame_addr(XV_warp_filter *InstancePtr,
		u32 Descnum, u64 dest_buf_addr)
{
	XVWarpFilter_Desc *descptr;
	u32 i;

	Xil_AssertNonvoid(InstancePtr);

	descptr = (XVWarpFilter_Desc *)InstancePtr->WarpFilterDesc_BaseAddr;
	for (i = 0; i < Descnum; i++) {
		descptr = (XVWarpFilter_Desc *)descptr->Warp_NextDescAddr;
	}

	descptr->dest_buf_addr = dest_buf_addr;

	XV_warp_filter_Set_desc_addr(InstancePtr, (u64)descptr);

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
 * @brief Enables interrupts for the core instance.
 *
 * This function enables the specified interrupts and the global interrupt
 * enable for the warp filter core.
 *
 * @param  InstancePtr is a pointer to the XV_warp_filter instance.
 * @param  Mask is the interrupt enable mask.
 *
 * @return None.
 *
 * @note   Both the specific interrupt and global interrupt enable are set.
 *
 ******************************************************************************/
void XVWarpFilter_InterruptEnable(XV_warp_filter *InstancePtr,
		u32 Mask)
{
	Xil_AssertVoid(InstancePtr);

	XV_warp_filter_InterruptEnable(InstancePtr, Mask);
	XV_warp_filter_InterruptGlobalEnable(InstancePtr);
}

/*****************************************************************************/
/**
 * @brief Disables interrupts in the core.
 *
 * This function disables the specified interrupts and the global interrupt
 * enable for the warp filter core.
 *
 * @param  InstancePtr is a pointer to the XV_warp_filter instance.
 * @param  IrqMask is the interrupt disable mask.
 *
 * @return None.
 *
 * @note   Both the specific interrupt and global interrupt enable are cleared.
 *
 ******************************************************************************/
void XVWarpFilter_InterruptDisable(XV_warp_filter *InstancePtr, u32 IrqMask)
{
  Xil_AssertVoid(InstancePtr);

  /* Disable Interrupts */
  XV_warp_filter_InterruptDisable(InstancePtr, IrqMask);
  XV_warp_filter_InterruptGlobalDisable(InstancePtr);

}

/*****************************************************************************/
/**
 * @brief Starts the core instance.
 *
 * This function initiates the warp filter core processing.
 *
 * @param  InstancePtr is a pointer to the XV_warp_filter instance.
 *
 * @return None.
 *
 * @note   None.
 *
 ******************************************************************************/
void XVWarpFilter_Start(XV_warp_filter *InstancePtr)
{
  Xil_AssertVoid(InstancePtr);

  XV_warp_filter_Start(InstancePtr);
}

/*****************************************************************************/
/**
 * @brief Stops the core instance.
 *
 * This function stops the warp filter core by clearing the autostart bit,
 * setting the flush bit, and waiting for the flush operation to complete.
 *
 * @param  InstancePtr is a pointer to the XV_warp_filter instance.
 *
 * @return XST_SUCCESS if the core entered stop state successfully.
 *         XST_FAILURE if the core failed to stop (flush timeout).
 *
 * @note   The function waits for a flush done signal with timeout.
 *
 ******************************************************************************/
s32 XVWarpFilter_Stop(XV_warp_filter *InstancePtr)
{
  int Status = XST_SUCCESS;
  u32 cnt = 0;
  u32 Data = 0;

  Xil_AssertNonvoid(InstancePtr);

  /* Clear autostart bit */
  XV_warp_filter_DisableAutoRestart(InstancePtr);

  /* Flush the core bit */
  XV_warp_filter_SetFlushbit(InstancePtr);

  do {
    Data = XV_warp_filter_Get_FlushDone(InstancePtr);
    usleep(XV_WAIT_FOR_FLUSH_DONE_TIMEOUT);
    cnt++;
  } while((Data == 0) && (cnt < XV_WAIT_FOR_FLUSH_DONE));

  if (Data == 0)
        Status = XST_FAILURE;

  return(Status);
}

/*****************************************************************************/
/**
 * @brief Allocates aligned memory.
 *
 * This function allocates memory aligned to the specified boundary. It stores
 * an offset value before the returned pointer to enable proper deallocation.
 * The alignment must be a power of two.
 *
 * @param  align is the alignment size (must be power of two).
 * @param  size is the size of memory to allocate in bytes.
 *
 * @return Pointer to the aligned memory block, or NULL if allocation fails
 *         or invalid arguments are provided.
 *
 * @note   The allocated memory must be freed using XVWarpFilter_AlignedFree().
 *
 ******************************************************************************/
static void * XVWarpFilter_AlignedMalloc(size_t align, size_t size)
{
    void * ptr = NULL;

    /*
     * We want it to be a power of two since
     * align_up operates on powers of two
     */

    if(align && size)
    {
        /*
         * We know we have to fit an offset value
         * We also allocate extra bytes to ensure we
         * can meet the alignment
         */
        uint32_t hdr_size = PTR_OFFSET_SZ + (align - 1);
        void * p = malloc(size + hdr_size);
        memset(p, 0, (size + hdr_size));

        if(p)
        {
            /*
             * Add the offset size to malloc's pointer
             * (we will always store that)
             * Then align the resulting value to the
             * target alignment
             */
            ptr = (void *) align_up(((uintptr_t)p + PTR_OFFSET_SZ), align);

            /*
             * Calculate the offset and store it
             * behind our aligned pointer
             */
            *((u16 *)ptr - 1) =
                (u16)((uintptr_t)ptr - (uintptr_t)p);

        } 	/* else NULL, could not malloc */
    } /* else NULL, invalid arguments */

    return ptr;
}

/**
 * @brief Frees aligned memory.
 *
 * This function frees memory that was allocated using XVWarpFilter_AlignedMalloc().
 * It uses the stored offset to locate the original pointer returned by malloc().
 *
 * @param  ptr is the aligned memory pointer to be freed.
 *
 * @return None.
 *
 * @note   Only use this function to free memory allocated by
 *         XVWarpFilter_AlignedMalloc().
 *
 ******************************************************************************/
static void XVWarpFilter_AlignedFree(void * ptr)
{
    /*
    * Walk backwards from the passed-in pointer
    * to get the pointer offset. We convert to an offset_t
    * pointer and rely on pointer math to get the data
    */
    offset_t offset = *((offset_t *)ptr - 1);

    /*
    * Once we have the offset, we can get our
    * original pointer and call free
    */
    void * p = (void *)((uint8_t *)ptr - offset);
    free(p);
}
