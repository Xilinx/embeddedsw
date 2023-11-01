/******************************************************************************
* Copyright (C) 2008 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2022-2023, Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


#ifndef XWARP_INPUT_CONFIGS_H_
#define XWARP_INPUT_CONFIGS_H_

#include "xv_warp_init_l2.h"
#include "xv_warp_filter_l2.h"
#include "xil_types.h"

/**************************************************************************/
#define JB_CHANGES		1

#define REMAP_VECT_0_ADDR_0				0x10000000
#define REMAP_VECT_0_ADDR_1				0x1219e400
#define SRC_BUF_START_ADDR 				0x30000000
#define DST_BUF_START_ADDR 				0x40000000
#define MAX_SIZE_OF_4K_FRAME_IN_MEM		0x04000000
#define _1920_1080_FRAME_SIZE			6220800
#define _4K_FRAME_SIZE					24883200
#define BUFF_QUEUE_SIZE					5
/**************************************************************************/
#define WI_PI			3.14159265358979
#define LENS_DIST_SHIFT	12

struct Lens_params_fl {
	float k_pre;
	float k_post;
};

struct Keystone_params_fl {
	short left_top_x;
	short left_top_y;
	short right_top_x;
	short right_top_y;
	short left_bottom_x;
	short left_bottom_y;
	short right_bottom_x;
	short right_bottom_y;
};

struct affine_param_fl {
	float scale_x;
	float scale_y;
	float rot_angle;
	float trans_x;
	float trans_y;
	float zoom;
};

struct Arbitrary_param_fl {
	int grid_size;
	int num_ctrl_pts;
	short *dst_ctrl_x_pts;
	short *dst_ctrl_y_pts;
	short *src_ctrl_x_pts;
	short *src_ctrl_y_pts;
	short *update_x;
	short *update_y;
	short *update_blk_x;
	short *update_blk_y;
	float *src_tangents_x;
	float *src_tangents_y;
	float *knots_x;
	float *knots_y;
	float *temp_row;
	float *interm_x;
	float *interm_y;
	float *remap_blk;
};

struct Geometric_info_fl {
	struct Lens_params_fl lens_params;
	struct Keystone_params_fl keystone_params;
	struct affine_param_fl affine_param;
	struct Arbitrary_param_fl arbitrary_param;
	short warp_type;
	float proj_trans[9];
	unsigned short fr_width;
	unsigned short fr_height;
	float *remap_xy;
};

struct Geo_trans_param {
	int k_pre;
	int k_post;
	int h[9];
};

struct Geometric_info {
	struct Geo_trans_param geo_trans_params;
	XVWarpInit_ArbParam arbitrary_param;
	unsigned short fr_width;
	unsigned short fr_height;
	unsigned char *remap_xy;
	unsigned char *remap_xy_sort;
	short warp_type;
};

typedef struct {
	int frame_width;
	int frame_height;
	int warp_type;
	float pre_fisheye;
	float post_fisheye;
	int left_top_x;
	int left_top_y;
	int right_top_x;
	int right_top_y;
	int left_bottom_x;
	int left_bottom_y;
	int right_bottom_x;
	int right_bottom_y;
	float scale_x;
	float scale_y;
	float rotation;
	float trans_x;
	float trans_y;
	float zoom;
	int num_ctrl_pts;
	XVWarpInit_ArbParam_MeshInfo *ctrl_pts;
} WARP_CFG;

typedef struct {
	XVWarpInit_InputConfigs		initVectConfigs;
	XVWarpFilter_InputConfigs		filterConfigs;
	int golden_crc;
}warp_driver_Configs;

/*****************************************************************************/
void get_init_vect_input_configs(warp_driver_Configs *drvconfigs, WARP_CFG *InputConfigs);

#endif
