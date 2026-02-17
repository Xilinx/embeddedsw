/******************************************************************************
* Copyright (C) 2008 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2022-2026, Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/**
 * @file xwarp_input_configs.h
 * @addtogroup v_warp_init Overview
 */

#ifndef XWARP_INPUT_CONFIGS_H_
#define XWARP_INPUT_CONFIGS_H_

/***************************** Include Files *********************************/
#include "xv_warp_init_l2.h"
#include "xv_warp_filter_l2.h"
#include "xil_types.h"

/************************** Constant Definitions *****************************/

/**
 * Enable JB-specific code changes
 */
#define JB_CHANGES		1

/**
 * Base address for remap vector descriptor 0
 */
#define REMAP_VECT_0_ADDR_0				0x10000000

/**
 * Secondary address for remap vector descriptor 0
 */
#define REMAP_VECT_0_ADDR_1				0x1219e400

/**
 * Start address for source buffer in memory
 */
#define SRC_BUF_START_ADDR 				0x30000000

/**
 * Start address for destination buffer in memory
 */
#define DST_BUF_START_ADDR 				0x40000000

/**
 * Maximum size allocated for 4K frame in memory
 */
#define MAX_SIZE_OF_4K_FRAME_IN_MEM		0x04000000

/**
 * Frame size in bytes for 1920x1080 resolution
 */
#define _1920_1080_FRAME_SIZE			6220800

/**
 * Frame size in bytes for 4K resolution
 */
#define _4K_FRAME_SIZE					24883200

/**
 * Size of buffer queue
 */
#define BUFF_QUEUE_SIZE					5

/**
 * Value of Pi constant for warp initialization
 */
#define WI_PI			3.14159265358979

/**
 * Shift value for lens distortion fixed-point conversion
 */
#define LENS_DIST_SHIFT	12

/**************************** Type Definitions *******************************/

/**
 * @brief Lens distortion parameters in floating-point format
 *
 * This structure contains the fisheye lens distortion coefficients
 * used for pre and post distortion correction.
 */
struct Lens_params_fl {
	float k_pre;   /**< Pre-distortion coefficient for fisheye lens */
	float k_post;  /**< Post-distortion coefficient for fisheye lens */
};

/**
 * @brief Keystone correction parameters in floating-point format
 *
 * This structure contains the four corner points for keystone/trapezoidal
 * correction defining the quadrilateral transformation.
 */
struct Keystone_params_fl {
	short left_top_x;      /**< X coordinate of left top corner */
	short left_top_y;      /**< Y coordinate of left top corner */
	short right_top_x;     /**< X coordinate of right top corner */
	short right_top_y;     /**< Y coordinate of right top corner */
	short left_bottom_x;   /**< X coordinate of left bottom corner */
	short left_bottom_y;   /**< Y coordinate of left bottom corner */
	short right_bottom_x;  /**< X coordinate of right bottom corner */
	short right_bottom_y;  /**< Y coordinate of right bottom corner */
};

/**
 * @brief Affine transformation parameters in floating-point format
 *
 * This structure contains parameters for 2D affine transformations including
 * scale, rotation, translation, and zoom operations.
 */
struct affine_param_fl {
	float scale_x;     /**< Scale factor in X direction */
	float scale_y;     /**< Scale factor in Y direction */
	float rot_angle;   /**< Rotation angle in degrees */
	float trans_x;     /**< Translation offset in X direction */
	float trans_y;     /**< Translation offset in Y direction */
	float zoom;        /**< Zoom factor */
};

/**
 * @brief Arbitrary warp parameters in floating-point format
 *
 * This structure contains parameters for arbitrary mesh-based warping
 * including control points, tangents, and intermediate computation buffers.
 */
struct Arbitrary_param_fl {
	int grid_size;            /**< Size of the grid for mesh warping */
	int num_ctrl_pts;         /**< Number of control points */
	short *dst_ctrl_x_pts;    /**< Destination control points X coordinates */
	short *dst_ctrl_y_pts;    /**< Destination control points Y coordinates */
	short *src_ctrl_x_pts;    /**< Source control points X coordinates */
	short *src_ctrl_y_pts;    /**< Source control points Y coordinates */
	short *update_x;          /**< Update values for X coordinates */
	short *update_y;          /**< Update values for Y coordinates */
	short *update_blk_x;      /**< Block update values for X coordinates */
	short *update_blk_y;      /**< Block update values for Y coordinates */
	float *src_tangents_x;    /**< Source tangent vectors in X direction */
	float *src_tangents_y;    /**< Source tangent vectors in Y direction */
	float *knots_x;           /**< Knot values for X spline interpolation */
	float *knots_y;           /**< Knot values for Y spline interpolation */
	float *temp_row;          /**< Temporary row buffer for computation */
	float *interm_x;          /**< Intermediate X coordinate values */
	float *interm_y;          /**< Intermediate Y coordinate values */
	float *remap_blk;         /**< Remap block data */
};

/**
 * @brief Complete geometric transformation information in floating-point format
 *
 * This structure aggregates all geometric transformation parameters including
 * lens distortion, keystone, affine, and arbitrary transformations along with
 * frame dimensions and computed transformation matrices.
 */
struct Geometric_info_fl {
	struct Lens_params_fl lens_params;          /**< Lens distortion parameters */
	struct Keystone_params_fl keystone_params;  /**< Keystone correction parameters */
	struct affine_param_fl affine_param;        /**< Affine transformation parameters */
	struct Arbitrary_param_fl arbitrary_param;  /**< Arbitrary warp parameters */
	short warp_type;                            /**< Warp type selector (0=L+H, 1=Arbitrary) */
	float proj_trans[9];                        /**< 3x3 projective transformation matrix */
	unsigned short fr_width;                    /**< Frame width in pixels */
	unsigned short fr_height;                   /**< Frame height in pixels */
	float *remap_xy;                            /**< Remap coordinate array */
};

/**
 * @brief Geometric transformation parameters in fixed-point format
 *
 * This structure contains lens distortion coefficients and homography matrix
 * in fixed-point representation for hardware processing.
 */
struct Geo_trans_param {
	int k_pre;   /**< Pre-distortion coefficient in fixed-point */
	int k_post;  /**< Post-distortion coefficient in fixed-point */
	int h[9];    /**< 3x3 homography matrix in fixed-point */
};

/**
 * @brief Complete geometric transformation information in fixed-point format
 *
 * This structure contains all geometric transformation parameters in fixed-point
 * representation suitable for hardware implementation.
 */
struct Geometric_info {
	struct Geo_trans_param geo_trans_params;  /**< Geometric transformation parameters */
	XVWarpInit_ArbParam arbitrary_param;      /**< Arbitrary warp parameters */
	unsigned short fr_width;                  /**< Frame width in pixels */
	unsigned short fr_height;                 /**< Frame height in pixels */
	unsigned char *remap_xy;                  /**< Remap coordinate array */
	unsigned char *remap_xy_sort;             /**< Sorted remap coordinate array */
	short warp_type;                          /**< Warp type selector */
};

/**
 * @brief Warp configuration structure
 *
 * This structure contains user-specified warp configuration parameters including
 * frame dimensions, lens distortion, keystone correction, affine transformations,
 * and arbitrary mesh control points.
 */
typedef struct {
	int frame_width;                          /**< Frame width in pixels */
	int frame_height;                         /**< Frame height in pixels */
	int warp_type;                            /**< Warp type (0=L+H, 1=Arbitrary) */
	float pre_fisheye;                        /**< Pre-fisheye distortion coefficient */
	float post_fisheye;                       /**< Post-fisheye distortion coefficient */
	int left_top_x;                           /**< Left top corner X coordinate */
	int left_top_y;                           /**< Left top corner Y coordinate */
	int right_top_x;                          /**< Right top corner X coordinate */
	int right_top_y;                          /**< Right top corner Y coordinate */
	int left_bottom_x;                        /**< Left bottom corner X coordinate */
	int left_bottom_y;                        /**< Left bottom corner Y coordinate */
	int right_bottom_x;                       /**< Right bottom corner X coordinate */
	int right_bottom_y;                       /**< Right bottom corner Y coordinate */
	float scale_x;                            /**< Scale factor in X direction */
	float scale_y;                            /**< Scale factor in Y direction */
	float rotation;                           /**< Rotation angle in degrees */
	float trans_x;                            /**< Translation offset in X direction */
	float trans_y;                            /**< Translation offset in Y direction */
	float zoom;                               /**< Zoom factor */
	int num_ctrl_pts;                         /**< Number of control points for arbitrary warp */
	XVWarpInit_ArbParam_MeshInfo *ctrl_pts;   /**< Pointer to control point mesh information */
} WARP_CFG;

/**
 * @brief Warp driver configuration structure
 *
 * This structure contains configuration parameters for both warp initialization
 * and warp filter operations along with expected CRC for validation.
 */
typedef struct {
	XVWarpInit_InputConfigs		initVectConfigs;   /**< Warp initialization input configurations */
	XVWarpFilter_InputConfigs	filterConfigs;     /**< Warp filter input configurations */
	int golden_crc;                               /**< Expected CRC value for output validation */
}warp_driver_Configs;


/************************** Function Prototypes ******************************/
void get_init_vect_input_configs(warp_driver_Configs *drvconfigs, WARP_CFG *InputConfigs);

#endif
