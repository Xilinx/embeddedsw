/******************************************************************************
* Copyright (C) 2022 Xilinx, Inc.  All rights reserved.
* Copyright 2022-2023 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

#ifndef XV_WARP_INIT_L2_H     /* prevent circular inclusions */
#define XV_WARP_INIT_L2_H     /* by using protection macros */

/***************************** Include Files *********************************/
#include "xv_warp_init.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/
typedef struct {
	 s32 s_x;
	 s32 s_y;
	 s32 d_x;
	 s32 d_y;
} XVWarpInit_ArbParam_MeshInfo;

/*
 * This structure is used for all internal calculations
 * of warp init driver
 */
typedef struct {
	u16 width;
	u16 height;
	u16 bytes_per_pixel;
	u8 reserved_0[2];
	u64 filter_table_addr_0;
	u64 filter_table_addr_1;
	u32 width_Q4;
	u32 height_Q4;
	s16 k_pre;
	s16 k_post;
	u16 k0_pre;
	u16 k1_pre;
	u16 k0_post;
	u16 k1_post;
	u32 Q_fact_pre;
	u32 Q_fact_post;
	s8 k0_pre_Q_bits;
	s8 k0_post_Q_bits;
	s8 k1_pre_Q_bits;
	s8 k1_post_Q_bits;
	s16 h[6];
	s32 h_trans[3];
	u8 h_Qbits[6];
	u8 reserved_1[2];
	u32 normfactor;
	u16 cenX;
	u16 cenY;
	u32 num_ctrl_pts;
	u64 src_ctrl_x_pts;
	u64 src_ctrl_y_pts;
	u64 src_tangents_x;
	u64 src_tangents_y;
	u64 interm_x;
	u64 interm_y;
	u8 warp_type;
	u8 reserved_2[3];
	u32 valid_segs;
	u32 lblk_count;
	u32 line_num_seg;
} XVWarpInitVector_Hw;

/*
 * This structure is used for Descriptions of warp init IP
 */
typedef volatile struct {
	u32	width;
	u32	height;
	s32	k_pre;
	s32	k_post;
	u32 k0_pre;
	u32 k1_pre;
	u32 k0_post;
	u32 k1_post;
	u32	Q_fact_pre;
	u32	Q_fact_post;
	s32 k0_pre_Q_bits;
	s32 k0_post_Q_bits;
	s32 k1_pre_Q_bits;
	s32 k1_post_Q_bits;
	u32	normfactor;
	u32	cenX;
	u32	cenY;
	u32 width_Q4;
	u32 height_Q4;
	s32	h[6];
	u32 h_Qbits[6];
	s32 h_trans[3];
	u64	filter_table_addr_0;
	u64	filter_table_addr_1;
	u64 src_ctrl_x_pts;
	u64 src_ctrl_y_pts;
	u64 src_tangents_x;
	u64 src_tangents_y;
	u64 interm_x;
	u64 interm_y;
	u32 num_ctrl_pts;
	s32 bytes_per_pixel;
	u32 warp_type;
    u32 valid_seg;
    u32 lblock_count;
    u32 line_num;
    u32 ip_status;
	u32 driver_checksum;
	u32 ip_checksum;
	u64 remap_nextaddr;
} XVWarpInitVector_Hw_Aligned;

typedef struct {
	u32 grid_size;
	u32 num_ctrl_pts;
	u16 *dst_ctrl_x_pts;
	u16 *dst_ctrl_y_pts;
	u16 *src_ctrl_x_pts;
	u16 *src_ctrl_y_pts;
	s32 *src_tangents_x;
	s32 *src_tangents_y;
	s16 *knots_x;
	s16 *knots_y;
	s32 *temp_row;
	s32 *interm_x;
	s32 *interm_y;
} XVWarpInit_ArbParam;

typedef struct {
	u16	width;
	u16	height;
	u16 bytes_per_pixel;
	u8	warp_type;
	u32 num_ctrl_pts;
	s16	k_pre;
	s16	k_post;
	s32 h[9];
	u64	filter_table_addr_0;
	u64	filter_table_addr_1;
	XVWarpInit_ArbParam_MeshInfo *ctr_pts;
} XVWarpInit_InputConfigs;

/**************************** Function Prototypes *****************************/
void XVWarpInit_EnableInterrupts(XV_warp_init *InstancePtr,
		u32 Mask);
void XVWarpInit_SetCallback(XV_warp_init *InstancePtr,
		void *CallbackFunc, void *CallbackRef);
void *XVWarpInit_IntrHandler(void *InstancePtr);
int XVWarpInit_SetNumOfDescriptors(XV_warp_init *InstancePtr,
		u32 num_descriptors);
void XVWarpInit_ClearNumOfDescriptors(XV_warp_init *InstancePtr);
int XVWarpInit_ProgramDescriptor(XV_warp_init *InstancePtr,
		u32 Descnum, XVWarpInit_InputConfigs *ConfigPtr);
int XVWarpInit_start_with_desc(XV_warp_init *InstancePtr,
		u32 descnum);
void XVWarpInit_Stop(XV_warp_init *InstancePtr);
#endif
