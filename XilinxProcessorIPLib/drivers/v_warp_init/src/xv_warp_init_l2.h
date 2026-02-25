/******************************************************************************
* Copyright (C) 2022 Xilinx, Inc.  All rights reserved.
* Copyright 2022-2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/**
 * @file xv_warp_init_l2.h
 * @addtogroup v_warp_init Overview
 */

#ifndef XV_WARP_INIT_L2_H     /* prevent circular inclusions by using protection macros */
/* Define header guard macro to prevent multiple inclusion */
#define XV_WARP_INIT_L2_H

/***************************** Include Files *********************************/
#include "xv_warp_init.h"

/**************************** Type Definitions *******************************/
/**
 * Mesh information structure for arbitrary warp control points
 */
typedef struct {
	 s32 s_x;  /**< Source X coordinate */
	 s32 s_y;  /**< Source Y coordinate */
	 s32 d_x;  /**< Destination X coordinate */
	 s32 d_y;  /**< Destination Y coordinate */
} XVWarpInit_ArbParam_MeshInfo;

/**
 * Hardware initialization vector structure for internal calculations
 */
typedef struct {
	u16 width;                  /**< Frame width */
	u16 height;                 /**< Frame height */
	u16 bytes_per_pixel;        /**< Bytes per pixel */
	u8 reserved_0[2];           /**< Reserved bytes for alignment */
	u64 filter_table_addr_0;    /**< Filter table address 0 */
	u64 filter_table_addr_1;    /**< Filter table address 1 */
	u32 width_Q4;               /**< Width in Q4 fixed-point format */
	u32 height_Q4;              /**< Height in Q4 fixed-point format */
	s16 k_pre;                  /**< Pre-distortion coefficient */
	s16 k_post;                 /**< Post-distortion coefficient */
	u16 k0_pre;                 /**< Pre-distortion k0 coefficient */
	u16 k1_pre;                 /**< Pre-distortion k1 coefficient */
	u16 k0_post;                /**< Post-distortion k0 coefficient */
	u16 k1_post;                /**< Post-distortion k1 coefficient */
	u32 Q_fact_pre;             /**< Pre-distortion Q factor */
	u32 Q_fact_post;            /**< Post-distortion Q factor */
	s8 k0_pre_Q_bits;           /**< Pre-distortion k0 Q bits */
	s8 k0_post_Q_bits;          /**< Post-distortion k0 Q bits */
	s8 k1_pre_Q_bits;           /**< Pre-distortion k1 Q bits */
	s8 k1_post_Q_bits;          /**< Post-distortion k1 Q bits */
	s16 h[6];                   /**< Homography matrix coefficients (first 2 columns) */
	s32 h_trans[3];             /**< Homography matrix translation (last column) */
	u8 h_Qbits[6];              /**< Homography Q bits for each coefficient */
	u8 reserved_1[2];           /**< Reserved bytes for alignment */
	u32 normfactor;             /**< Normalization factor */
	u16 cenX;                   /**< Center X coordinate */
	u16 cenY;                   /**< Center Y coordinate */
	u32 num_ctrl_pts;           /**< Number of control points */
	u64 src_ctrl_x_pts;         /**< Source control X points address */
	u64 src_ctrl_y_pts;         /**< Source control Y points address */
	u64 src_tangents_x;         /**< Source tangents X address */
	u64 src_tangents_y;         /**< Source tangents Y address */
	u64 interm_x;               /**< Intermediate X data address */
	u64 interm_y;               /**< Intermediate Y data address */
	u8 warp_type;               /**< Warp type selector */
	u8 reserved_2[3];           /**< Reserved bytes for alignment */
	u32 valid_segs;             /**< Valid segments count */
	u32 lblk_count;             /**< Line block count */
	u32 line_num_seg;           /**< Line number per segment */
} XVWarpInitVector_Hw;

/**
 * Hardware-aligned descriptor structure for warp init IP
 */
typedef volatile struct {
	u32	width;                  /**< Frame width */
	u32	height;                 /**< Frame height */
	s32	k_pre;                  /**< Pre-distortion coefficient */
	s32	k_post;                 /**< Post-distortion coefficient */
	u32 k0_pre;                 /**< Pre-distortion k0 coefficient */
	u32 k1_pre;                 /**< Pre-distortion k1 coefficient */
	u32 k0_post;                /**< Post-distortion k0 coefficient */
	u32 k1_post;                /**< Post-distortion k1 coefficient */
	u32	Q_fact_pre;             /**< Pre-distortion Q factor */
	u32	Q_fact_post;            /**< Post-distortion Q factor */
	s32 k0_pre_Q_bits;          /**< Pre-distortion k0 Q bits */
	s32 k0_post_Q_bits;         /**< Post-distortion k0 Q bits */
	s32 k1_pre_Q_bits;          /**< Pre-distortion k1 Q bits */
	s32 k1_post_Q_bits;         /**< Post-distortion k1 Q bits */
	u32	normfactor;             /**< Normalization factor */
	u32	cenX;                   /**< Center X coordinate */
	u32	cenY;                   /**< Center Y coordinate */
	u32 width_Q4;               /**< Width in Q4 fixed-point format */
	u32 height_Q4;              /**< Height in Q4 fixed-point format */
	s32	h[6];                   /**< Homography matrix coefficients */
	u32 h_Qbits[6];             /**< Homography Q bits array */
	s32 h_trans[3];             /**< Homography translation vector */
	u64	filter_table_addr_0;    /**< Filter table address 0 */
	u64	filter_table_addr_1;    /**< Filter table address 1 */
	u64 src_ctrl_x_pts;         /**< Source control X points address */
	u64 src_ctrl_y_pts;         /**< Source control Y points address */
	u64 src_tangents_x;         /**< Source tangents X address */
	u64 src_tangents_y;         /**< Source tangents Y address */
	u64 interm_x;               /**< Intermediate X data address */
	u64 interm_y;               /**< Intermediate Y data address */
	u32 num_ctrl_pts;           /**< Number of control points */
	s32 bytes_per_pixel;        /**< Bytes per pixel */
	u32 warp_type;              /**< Warp type selector */
    u32 valid_seg;              /**< Valid segment indicator */
    u32 lblock_count;           /**< Line block count */
    u32 line_num;               /**< Line number */
    u32 ip_status;              /**< IP status register */
	u32 driver_checksum;        /**< Driver checksum */
	u32 ip_checksum;            /**< IP checksum */
	u64 remap_nextaddr;         /**< Next descriptor address */
} XVWarpInitVector_Hw_Aligned;

/**
 * Arbitrary warp parameter structure for geometric transformations
 */
typedef struct {
	u32 grid_size;              /**< Grid size for control points */
	u32 num_ctrl_pts;           /**< Number of control points */
	u16 *dst_ctrl_x_pts;        /**< Destination control X points array */
	u16 *dst_ctrl_y_pts;        /**< Destination control Y points array */
	u16 *src_ctrl_x_pts;        /**< Source control X points array */
	u16 *src_ctrl_y_pts;        /**< Source control Y points array */
	s32 *src_tangents_x;        /**< Source tangents X array */
	s32 *src_tangents_y;        /**< Source tangents Y array */
	s16 *knots_x;               /**< Knots X array for spline interpolation */
	s16 *knots_y;               /**< Knots Y array for spline interpolation */
	s32 *temp_row;              /**< Temporary row buffer */
	s32 *interm_x;              /**< Intermediate X calculation buffer */
	s32 *interm_y;              /**< Intermediate Y calculation buffer */
} XVWarpInit_ArbParam;

/**
 * Input configuration structure for warp initialization
 */
typedef struct {
	u16	width;                  /**< Frame width */
	u16	height;                 /**< Frame height */
	u16 bytes_per_pixel;        /**< Bytes per pixel */
	u8	warp_type;              /**< Warp type selector */
	u32 num_ctrl_pts;           /**< Number of control points */
	s16	k_pre;                  /**< Pre-distortion coefficient */
	s16	k_post;                 /**< Post-distortion coefficient */
	s32 h[9];                   /**< 3x3 homography matrix */
	u64	filter_table_addr_0;    /**< Filter table address 0 */
	u64	filter_table_addr_1;    /**< Filter table address 1 */
	XVWarpInit_ArbParam_MeshInfo *ctr_pts;  /**< Pointer to mesh control points */
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
