/******************************************************************************
* Copyright (C) 2021 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

#ifndef XV_WARP_FILTER_L2_H     /* prevent circular inclusions */
#define XV_WARP_FILTER_L2_H     /* by using protection macros */

/***************************** Include Files *********************************/
#include "xv_warp_filter.h"

/************************** Constant Definitions *****************************/
/**************************** Type Definitions *******************************/
typedef struct {
	u32 height;
	u32 width;
	u32 stride;
	u32 format;
	u32 valid_seg;
	u32 lblock_count;
	u32 line_num;
	u32 reserved;
	u64 src_buf_addr;
#if ((MAX_NR_PLANES == 2) || (MAX_NR_PLANES == 3) )
	u64 src_buf_addr1;
#endif
#if (MAX_NR_PLANES == 3)
	u64 src_buf_addr2;
#endif
	u64 seg_table_addr;
	u64 dest_buf_addr;
#if ((MAX_NR_PLANES == 2) || (MAX_NR_PLANES == 3) )
	u64 dest_buf_addr1;
#endif
#if (MAX_NR_PLANES == 3)
	u64 dest_buf_addr2;
#endif
#if (WRITE_INVALID == 1)
	AXIMM_WRITE dest_buf_addr_i;
	AXIMM_READ seg_table_addr_i;
#endif
	u64 Warp_NextDescAddr;
} XVWarpFilter_Desc;

typedef struct {
	u32 height;
	u32 width;
	u32 stride;
	u32 format;
	u64 src_buf_addr;
	u64 seg_table_addr;
	u64 dest_buf_addr;
} XVWarpFilter_InputConfigs;

/**************************** Function Prototypes *****************************/
void XVWarpFilter_InterruptEnable(XV_warp_filter *InstancePtr, u32 Mask);
void XVWarpFilter_InterruptDisable(XV_warp_filter *InstancePtr, u32 IrqMask);
void XVWarpFilter_SetCallback(XV_warp_filter *InstancePtr,
		void *CallbackFunc, void *CallbackRef);
void *XVWarpFilter_IntrHandler(void *InstancePtr);
s32 XVWarpFilter_ProgramDescriptor(XV_warp_filter *InstancePtr, u32 DescNum,
		XVWarpFilter_InputConfigs *configPtr, u32 valid_seg,
		u32 lblock_count, u32 line_num);
void XVWarpFilter_ClearNumOfDescriptors(XV_warp_filter *InstancePtr);
s32 XVWarpFilter_SetNumOfDescriptors(XV_warp_filter *InstancePtr, u32 num_descriptors);
s32 XVWarpFilter_update_src_frame_addr(XV_warp_filter *InstancePtr,
		u32 Descnum, u64 src_buf_addr);
s32 XVWarpFilter_update_dst_frame_addr(XV_warp_filter *InstancePtr,
		u32 Descnum, u64 dest_buf_addr);
void XVWarpFilter_Start(XV_warp_filter *InstancePtr);
s32 XVWarpFilter_Stop(XV_warp_filter *InstancePtr);
#endif
