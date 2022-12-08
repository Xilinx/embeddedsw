/******************************************************************************
* Copyright (C) 2022 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2022-2023, Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/***************************** Include Files *********************************/
#include "xil_types.h"
#include "xv_warp_filter_l2.h"
#include "sleep.h"
#include <stdlib.h>

/************************** Constant Definitions *****************************/
#define PTR_OFFSET_SZ sizeof(u16)
typedef u16 offset_t;
#ifndef align_up
#define align_up(num, align) \
    (((num) + ((align) - 1)) & ~((align) - 1))
#endif

#define XV_WAIT_FOR_FLUSH_DONE		         (25)
#define XV_WAIT_FOR_FLUSH_DONE_TIMEOUT		 (2000)
#define WARP_FILTER_ADDR_WIDTH				 128

/**************************Static Function Prototypes ************************/
static void *XVWarpFilter_AlignedMalloc(size_t align, size_t size);
static void XVWarpFilter_AlignedFree(void * ptr);

/************************** Function Definitions *****************************/
/*****************************************************************************/
/**
* This function creates the descriptors.
*
* @param  InstancePtr is a pointer to core instance to be worked upon
* @param  num_desc is the number of descriptors to be created
*
* @return XST_SUCCESS if Descriptors created successfully
*         XST_FAILURE if Descriptors creation failed
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
* This function clears and de-allocates all the descriptors.
*
* @param  InstancePtr is a pointer to core instance to be worked upon
*
* @return none
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
* This function programs a descriptor with given configurations.
*
* @param  InstancePtr is a pointer to core instance to be worked upon
* @param  Descnum is the descriptor number which to be configured
* @param  ConfigPtr is the input configuration pointer which to be configured
* 					into the descriptor
* @param  valid_seg is the number of valid segs in the remap vector
* @param  lblock_count is the number of output blocks decoded by the first warp
* 					   filter core.
* @param  line_num is the line number from which the second warp filter core
* 				   starts reading the source image
*
* @return XST_SUCCESS if programming descriptor is successful
*         XST_FAILURE if Descriptor is not valid.
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

/*****************************************************************************/
/**
* This function sets the src frame buffer address.
*
* @param  InstancePtr is a pointer to core instance to be worked upon
* @param  Descnum is the descriptor number which to be configured
* @param  src_buf_addr is the address of the source frame buffer
*
* @return XST_SUCCESS if programming descriptor is successful
*         XST_FAILURE if Descriptor is not valid.
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
* This function sets the destination frame buffer address.
*
* @param  InstancePtr is a pointer to core instance to be worked upon
* @param  Descnum is the descriptor number which to be configured
* @param  dst_buf_addr is the address of the destination frame buffer
*
* @return XST_SUCCESS if programming descriptor is successful
*         XST_FAILURE if Descriptor is not valid.
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
* This function Enables the Interrupts for the core instance
*
* @param  InstancePtr is a pointer to core instance to be worked upon
* @param  Mask is the Interrupt enable mask
*
* @return none
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
* This function disables interrupts in the core
*
* @param  InstancePtr is a pointer to core instance to be worked upon
*
* @return none
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
* This function starts the core instance
*
* @param  InstancePtr is a pointer to core instance to be worked upon
*
* @return none
*
******************************************************************************/
void XVWarpFilter_Start(XV_warp_filter *InstancePtr)
{
  Xil_AssertVoid(InstancePtr);

  XV_warp_filter_Start(InstancePtr);
}

/*****************************************************************************/
/**
* This function stops the core instance
*
* @param  InstancePtr is a pointer to core instance to be worked upon
*
* @return XST_SUCCESS if the core is stop state
*         XST_FAILURE if the core is not in stop state
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
/*****************************************************************************/
/**
* This function creates alligned void memory pointer.
*
* @param	align is the alignment size.
* @param	size, it is the size of pointer to be created.
*
* @return	ptr is memory pointer to be returned.
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

/*****************************************************************************/
/**
* This function frees the allocated aligned memory.
*
* @param	ptr is the memory pointer to be free.
*
* @return	None
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
