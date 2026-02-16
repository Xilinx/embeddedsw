/******************************************************************************
* Copyright (C) 2022 Xilinx, Inc.  All rights reserved.
* Copyright 2022-2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/**
 * @file xv_warp_filter_l2.h
 * @addtogroup v_warp_filter Overview
 */

#ifndef XV_WARP_FILTER_L2_H     /* prevent circular inclusions */
#define XV_WARP_FILTER_L2_H     /* by using protection macros */

/***************************** Include Files *********************************/
#include "xv_warp_filter.h"

/**************************** Type Definitions *******************************/
/**
 * @brief Warp filter descriptor structure.
 *
 * This structure defines the descriptor format for warp filter operations,
 * containing frame dimensions, buffer addresses, and control information.
 */
typedef struct {
	u32 height;             /**< Frame height in pixels */
	u32 width;              /**< Frame width in pixels */
	u32 stride;             /**< Line stride in bytes */
	u32 format;             /**< Pixel format */
	u32 valid_seg;          /**< Number of valid segments in remap vector */
	u32 lblock_count;       /**< Number of output blocks from first core */
	u32 line_num;           /**< Line number for second core to start reading */
	u32 reserved;           /**< Reserved field */
	u64 src_buf_addr;       /**< Source buffer address (plane 0) */
#if ((MAX_NR_PLANES == 2) || (MAX_NR_PLANES == 3) )
	u64 src_buf_addr1;      /**< Source buffer address (plane 1) */
#endif
#if (MAX_NR_PLANES == 3)
	u64 src_buf_addr2;      /**< Source buffer address (plane 2) */
#endif
	u64 seg_table_addr;     /**< Segment table address */
	u64 dest_buf_addr;      /**< Destination buffer address (plane 0) */
#if ((MAX_NR_PLANES == 2) || (MAX_NR_PLANES == 3) )
	u64 dest_buf_addr1;     /**< Destination buffer address (plane 1) */
#endif
#if (MAX_NR_PLANES == 3)
	u64 dest_buf_addr2;     /**< Destination buffer address (plane 2) */
#endif
#if (WRITE_INVALID == 1)
	AXIMM_WRITE dest_buf_addr_i;    /**< Invalid destination buffer address */
	AXIMM_READ seg_table_addr_i;    /**< Invalid segment table address */
#endif
	u64 Warp_NextDescAddr;  /**< Address of next descriptor in chain */
} XVWarpFilter_Desc;

/**
 * @brief Input configuration structure for warp filter.
 *
 * This structure contains the basic configuration parameters needed to
 * program a warp filter descriptor.
 */
typedef struct {
	u32 height;             /**< Frame height in pixels */
	u32 width;              /**< Frame width in pixels */
	u32 stride;             /**< Line stride in bytes */
	u32 format;             /**< Pixel format */
	u64 src_buf_addr;       /**< Source buffer address */
	u64 seg_table_addr;     /**< Segment table address */
	u64 dest_buf_addr;      /**< Destination buffer address */
} XVWarpFilter_InputConfigs;

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes *****************************/
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
