/******************************************************************************
* Copyright (C) 2008 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2022-2026, Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/**
 * @file xwarp_input_configs.c
 * @addtogroup v_warp_init Overview
 */

/***************************** Include Files *********************************/
#include <math.h>
#include <string.h>
#include "xwarp_input_configs.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/
enum remap_err_type {
	REMAP_NO_ERR,
	REMAP_MEM_ERR,
	REMAP_SIZ_ERR,
	REMAP_USP_ERR,
	REMAP_EST_ERR,
	REMAP_UDE_ERR
};

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/
static enum remap_err_type get_affine2d(struct Geometric_info_fl *geometric_info, float *h);
static void matrix_multiply(float *m1, int h1, int w1,
	float *m2, int h2, int w2, float *m3);
static void apply_affine(float *h, short *keystone_pts, float *proj_pts);
static enum remap_err_type estimate_projective_transform(short *src_pts,
	short *dst_pts, float *transform);
static void gaussianElimination(float *a, int n);
static void matrix_inv_3x3(float *src, float *dst);
static float my_fabs(float abs);
static float my_cos(float angle);
static float my_sin(float angle);
static int est_geometric_transform(struct Geometric_info_fl *geometric_info);
static void config_warp_convert_float_fixed(struct Geometric_info_fl *geometric_info_fl,
		warp_driver_Configs *drvconfigs);

/************************** Variable Definitions *****************************/

/*****************************************************************************/
/**
 * @brief Estimate geometric transformation matrix
 *
 * This function computes the complete geometric transformation by combining
 * affine and projective transforms based on keystone parameters.
 *
 * @param  geometric_info is a pointer to geometric information structure
 *
 * @return 0 on success
 *
 * @note None
 *
 *******************************************************************************/
static int est_geometric_transform(struct Geometric_info_fl *geometric_info)
{
	enum remap_err_type ret = REMAP_NO_ERR;
	float h[9], proj_pts[8], inv_h[9];
	short src_pts[8], tproj_pts[8];
	short diff_flag, i, keystone_pts[8];

	h[0] = 1.0; h[1] = 0.0; h[2] = 0.0;
	h[3] = 0.0; h[4] = 1.0; h[5] = 0.0;
	h[6] = 0.0; h[7] = 0.0; h[8] = 1.0;

	get_affine2d(geometric_info, &h[0]);

	keystone_pts[0] = geometric_info->keystone_params.left_top_x;
	keystone_pts[1] = geometric_info->keystone_params.left_top_y;
	keystone_pts[2] = geometric_info->keystone_params.right_top_x;
	keystone_pts[3] = geometric_info->keystone_params.right_top_y;
	keystone_pts[4] = geometric_info->keystone_params.right_bottom_x;
	keystone_pts[5] = geometric_info->keystone_params.right_bottom_y;
	keystone_pts[6] = geometric_info->keystone_params.left_bottom_x;
	keystone_pts[7] = geometric_info->keystone_params.left_bottom_y;

	src_pts[0] = 0;
	src_pts[1] = 0;
	src_pts[2] = geometric_info->fr_width;
	src_pts[3] = 0;
	src_pts[4] = geometric_info->fr_width;
	src_pts[5] = geometric_info->fr_height;
	src_pts[6] = 0;
	src_pts[7] = geometric_info->fr_height;

	diff_flag = 0;
	for (i = 0; i < 8; i++) {
		if (src_pts[i] != keystone_pts[i]) {
			diff_flag = 1;
			break;
		}
	}

	if (diff_flag) {
		//Applying affline transform on keystone vector
		apply_affine(h, &keystone_pts[0], proj_pts);

		for (i = 0; i < 8; i++)
			tproj_pts[i] = (short)(proj_pts[i] + 0.5);

		ret = estimate_projective_transform(&src_pts[0], &tproj_pts[0], &h[0]);
	}

	matrix_inv_3x3(h, &geometric_info->proj_trans[0]);

	return 0;
}

/*****************************************************************************/
/**
 * @brief Convert float configuration to fixed-point
 *
 * This function converts floating-point geometric parameters to fixed-point
 * representation for hardware configuration.
 *
 * @param  geometric_info_fl is a pointer to floating-point geometric info
 * @param  drvconfigs is a pointer to driver configuration structure
 *
 * @return None
 *
 * @note None
 *
 *******************************************************************************/
static void config_warp_convert_float_fixed(struct Geometric_info_fl *geometric_info_fl,
		warp_driver_Configs *drvconfigs)
{
	drvconfigs->initVectConfigs.width = geometric_info_fl->fr_width;
	drvconfigs->initVectConfigs.height = geometric_info_fl->fr_height;
	drvconfigs->initVectConfigs.warp_type = geometric_info_fl->warp_type;

	if (!drvconfigs->initVectConfigs.warp_type)
	{
		drvconfigs->initVectConfigs.k_pre = (int)(geometric_info_fl->lens_params.k_pre * (1 << LENS_DIST_SHIFT));
		drvconfigs->initVectConfigs.k_post = (int)(geometric_info_fl->lens_params.k_post * (1 << LENS_DIST_SHIFT));

		drvconfigs->initVectConfigs.h[0] = (int)(geometric_info_fl->proj_trans[0] * (1 << 24));
		drvconfigs->initVectConfigs.h[1] = (int)(geometric_info_fl->proj_trans[1] * (1 << 24));
		drvconfigs->initVectConfigs.h[2] = (int)(geometric_info_fl->proj_trans[2] * (1 << 12));
		drvconfigs->initVectConfigs.h[3] = (int)(geometric_info_fl->proj_trans[3] * (1 << 24));
		drvconfigs->initVectConfigs.h[4] = (int)(geometric_info_fl->proj_trans[4] * (1 << 24));
		drvconfigs->initVectConfigs.h[5] = (int)(geometric_info_fl->proj_trans[5] * (1 << 12));
		drvconfigs->initVectConfigs.h[6] = (int)(geometric_info_fl->proj_trans[6] * (1 << 24));
		drvconfigs->initVectConfigs.h[7] = (int)(geometric_info_fl->proj_trans[7] * (1 << 24));
		drvconfigs->initVectConfigs.h[8] = (int)(geometric_info_fl->proj_trans[8] * (1 << 16));
	}
}

/*****************************************************************************/
/**
 * @brief Compute 3x3 matrix inverse
 *
 * This function computes the inverse of a 3x3 matrix using the adjoint
 * method and determinant calculation.
 *
 * @param  src is a pointer to source 3x3 matrix
 * @param  dst is a pointer to destination inverse matrix
 *
 * @return None
 *
 * @note None
 *
 *******************************************************************************/
static void matrix_inv_3x3(float *src, float *dst) {
	float det;

	/* Compute adjoint: */

	dst[0] = +src[4] * src[8] - src[5] * src[7];
	dst[1] = -src[1] * src[8] + src[2] * src[7];
	dst[2] = +src[1] * src[5] - src[2] * src[4];
	dst[3] = -src[3] * src[8] + src[5] * src[6];
	dst[4] = +src[0] * src[8] - src[2] * src[6];
	dst[5] = -src[0] * src[5] + src[2] * src[3];
	dst[6] = +src[3] * src[7] - src[4] * src[6];
	dst[7] = -src[0] * src[7] + src[1] * src[6];
	dst[8] = +src[0] * src[4] - src[1] * src[3];

	/* Compute determinant: */

	det = src[0] * dst[0] + src[1] * dst[3] + src[2] * dst[6];

	/* Multiply adjoint with reciprocal of determinant: */

	det = 1.0f / det;
	if (!det) {
		dst[0] = 0;
		dst[1] = 0;
		dst[2] = 1;
		dst[3] = 0;
		dst[4] = 0;
		dst[5] = 1;
		dst[6] = 0;
		dst[7] = 0;
		dst[8] = 1;
	}

	dst[0] *= det;
	dst[1] *= det;
	dst[2] *= det;
	dst[3] *= det;
	dst[4] *= det;
	dst[5] *= det;
	dst[6] *= det;
	dst[7] *= det;
	dst[8] *= det;
}

/*****************************************************************************/
/**
 * @brief Perform Gaussian elimination on matrix
 *
 * This function performs Gaussian elimination with partial pivoting to solve
 * a system of linear equations.
 *
 * @param  a is a pointer to the augmented matrix
 * @param  n is the number of columns in the matrix
 *
 * @return None
 *
 * @note None
 *
 *******************************************************************************/
static void gaussianElimination(float *a, int n) {
	int       i = 0;
	int       j = 0;
	const int m = n - 1;

	while (i < m && j < n) {
		int maxi = i;
		for (int k = i + 1; k < m; ++k) {
			if (fabs(a[k * n + j]) > fabs(a[maxi * n + j])) {
				maxi = k;
			}
		}

		if (a[maxi * n + j] != 0) {
			if (i != maxi)
				for (int k = 0; k < n; k++) {
					const float aux = a[i * n + k];
					a[i * n + k] = a[maxi * n + k];
					a[maxi * n + k] = aux;
				}

			const float a_ij = a[i * n + j];
			for (int k = 0; k < n; k++) {
				a[i * n + k] /= a_ij;
			}

			for (int u = i + 1; u < m; u++) {
				const float a_uj = a[u * n + j];
				for (int k = 0; k < n; k++) {
					a[u * n + k] -= a_uj * a[i * n + k];
				}
			}

			++i;
		}
		++j;
	}

	for (i = m - 2; i >= 0; --i) {
		for (j = i + 1; j < n - 1; j++) {
			a[i * n + m] -= a[i * n + j] * a[j * n + m];
		}
	}
}

/*****************************************************************************/
/**
 * @brief Estimate projective transformation matrix
 *
 * This function estimates a 3x3 projective transformation matrix from
 * four point correspondences using Gaussian elimination.
 *
 * @param  src_pts is a pointer to source points array (8 values)
 * @param  dst_pts is a pointer to destination points array (8 values)
 * @param  transform is a pointer to output transformation matrix (9 values)
 *
 * @return REMAP_NO_ERR on success
 *
 * @note None
 *
 *******************************************************************************/
static enum remap_err_type estimate_projective_transform(short *src_pts,
	short *dst_pts, float *transform) {
	enum remap_err_type ret = REMAP_NO_ERR;

	float p[8][9] = {
		{ -src_pts[0], -src_pts[1], -1, 0, 0, 0, src_pts[0] * dst_pts[0], src_pts[1] * dst_pts[0], -dst_pts[0] },
		{ 0, 0, 0, -src_pts[0], -src_pts[1], -1, src_pts[0] * dst_pts[1], src_pts[1] * dst_pts[1], -dst_pts[1] },
		{ -src_pts[2], -src_pts[3], -1, 0, 0, 0, src_pts[2] * dst_pts[2], src_pts[3] * dst_pts[2], -dst_pts[2] },
		{ 0, 0, 0, -src_pts[2], -src_pts[3], -1, src_pts[2] * dst_pts[3], src_pts[3] * dst_pts[3], -dst_pts[3] },
		{ -src_pts[4], -src_pts[5], -1, 0, 0, 0, src_pts[4] * dst_pts[4], src_pts[5] * dst_pts[4], -dst_pts[4] },
		{ 0, 0, 0, -src_pts[4], -src_pts[5], -1, src_pts[4] * dst_pts[5], src_pts[5] * dst_pts[5], -dst_pts[5] },
		{ -src_pts[6], -src_pts[7], -1, 0, 0, 0, src_pts[6] * dst_pts[6], src_pts[7] * dst_pts[6], -dst_pts[6] },
		{ 0, 0, 0, -src_pts[6], -src_pts[7], -1, src_pts[6] * dst_pts[7], src_pts[7] * dst_pts[7], -dst_pts[7] },
	};

	gaussianElimination(&p[0][0], 9);

	transform[0] = p[0][8]; transform[1] = p[1][8]; transform[2] = p[2][8];
	transform[3] = p[3][8]; transform[4] = p[4][8]; transform[5] = p[5][8];
	transform[6] = p[6][8]; transform[7] = p[7][8]; transform[8] = 1;

	return ret;
}

/*****************************************************************************/
/**
 * @brief Apply affine transformation to points
 *
 * This function applies a 2D affine transformation matrix to an array of
 * keystone points.
 *
 * @param  h is a pointer to the affine transformation matrix
 * @param  keystone_pts is a pointer to input keystone points
 * @param  proj_pts is a pointer to output projected points
 *
 * @return None
 *
 * @note None
 *
 *******************************************************************************/
static void apply_affine(float *h, short *keystone_pts, float *proj_pts) {
	int i;

	for (i = 0; i < 8; i+=2) {
		proj_pts[i] = keystone_pts[i] * h[0] + keystone_pts[i+1] * h[1] + h[2];
		proj_pts[i+1] = keystone_pts[i] * h[3] +keystone_pts[i+1] * h[4] + h[5];
	}
}

/*****************************************************************************/
/**
 * @brief Multiply two matrices
 *
 * This function performs matrix multiplication: m3 = m1 * m2.
 *
 * @param  m1 is a pointer to first matrix
 * @param  h1 is the height (rows) of first matrix
 * @param  w1 is the width (columns) of first matrix
 * @param  m2 is a pointer to second matrix
 * @param  h2 is the height (rows) of second matrix
 * @param  w2 is the width (columns) of second matrix
 * @param  m3 is a pointer to result matrix
 *
 * @return None
 *
 * @note None
 *
 *******************************************************************************/
static void matrix_multiply(float *m1, int h1, int w1,
	float *m2, int h2, int w2, float *m3) {
	int i, j, k;
	memset(m3, 0, sizeof(float) * 9);
	for (i = 0; i < h1; ++i)
		for (j = 0; j < w2; ++j)
			for (k = 0; k < w1; ++k)
			{
				m3[i*w2+j] += m1[i*w1+k] * m2[k*w2+j];
			}
}

/*****************************************************************************/
/**
 * @brief Calculate sine of angle
 *
 * This is a placeholder function for sine calculation.
 *
 * @param  angle is the angle value
 *
 * @return Sine value (currently returns 0)
 *
 * @note This is a stub implementation
 *
 *******************************************************************************/
static float my_sin(float angle) {
	return 0;
}

/*****************************************************************************/
/**
 * @brief Calculate cosine of angle
 *
 * This is a placeholder function for cosine calculation.
 *
 * @param  angle is the angle value
 *
 * @return Cosine value (currently returns 0)
 *
 * @note This is a stub implementation
 *
 *******************************************************************************/
static float my_cos(float angle) {
	return 0;
}

/*****************************************************************************/
/**
 * @brief Calculate absolute value of float
 *
 * This function returns the absolute value of a floating-point number.
 *
 * @param  abs is the input value
 *
 * @return Absolute value of input
 *
 * @note None
 *
 *******************************************************************************/
static float my_fabs(float abs) {
	if (abs < 0)
		return (-1 * abs);
	else
		return abs;
}

/*****************************************************************************/
/**
 * @brief Compute 2D affine transformation matrix
 *
 * This function computes a 2D affine transformation matrix based on
 * scale, rotation, translation, and zoom parameters.
 *
 * @param  geometric_info is a pointer to geometric information structure
 * @param  h is a pointer to output transformation matrix
 *
 * @return REMAP_NO_ERR on success
 *
 * @note None
 *
 *******************************************************************************/
static enum remap_err_type get_affine2d(struct Geometric_info_fl *geometric_info, float *h) {
	enum remap_err_type ret = REMAP_NO_ERR;
	float angle = (geometric_info->affine_param.rot_angle * WI_PI) / 180.0;
	float sin_val = sin(angle);
	float cos_val = cos(angle);
	float h_temp1[9], h_temp2[9], h_temp3[9];
	float cen_x = geometric_info->fr_width / 2.0;
	float cen_y = geometric_info->fr_height / 2.0;

	//Adding transition effect
	h_temp1[0] = 1.0; h_temp1[1] = 0.0; h_temp1[2] = cen_x + geometric_info->affine_param.trans_x;
	h_temp1[3] = 0.0; h_temp1[4] = 1.0; h_temp1[5] = cen_y + geometric_info->affine_param.trans_y;
	h_temp1[6] = 0.0; h_temp1[7] = 0.0; h_temp1[8] = 1.0;

	//Adding rotation effect
	h_temp2[0] = cos_val; h_temp2[1] = -sin_val; h_temp2[2] = 0;
	h_temp2[3] = sin_val; h_temp2[4] = cos_val; h_temp2[5] = 0;
	h_temp2[6] = 0.0; h_temp2[7] = 0.0; h_temp2[8] = 1.0;

	//multiplying rotation and translation
	matrix_multiply(h_temp1, 3, 3, h_temp2, 3, 3, h_temp3);

	//Adding zooming effect
	h_temp1[0] = geometric_info->affine_param.zoom * geometric_info->affine_param.scale_x; h_temp1[1] = 0.0; h_temp1[2] = 0;
	h_temp1[3] = 0.0; h_temp1[4] = geometric_info->affine_param.zoom * geometric_info->affine_param.scale_y; h_temp1[5] = 0;
	h_temp1[6] = 0.0; h_temp1[7] = 0.0; h_temp1[8] = 1.0;

	//multiplying with scale and zoom
	matrix_multiply(h_temp3, 3, 3, h_temp1, 3, 3, h_temp2);

	//Normalizing the shift
	h_temp1[0] = 1.0; h_temp1[1] = 0.0; h_temp1[2] = -cen_x;
	h_temp1[3] = 0.0; h_temp1[4] = 1.0; h_temp1[5] = -cen_y;
	h_temp1[6] = 0.0; h_temp1[7] = 0.0; h_temp1[8] = 1.0;

	//multiplying with normalization
	matrix_multiply(h_temp2, 3, 3, h_temp1, 3, 3, h);

	return ret;
}

/*****************************************************************************/
/**
 * @brief Get initialization vector input configurations
 *
 * This function converts user-provided warp configuration parameters into
 * driver-specific configuration format for both warp initialization and
 * warp filter operations.
 *
 * @param  drvconfigs is a pointer to driver configuration structure
 * @param  InputConfigs is a pointer to input warp configuration
 *
 * @return None
 *
 * @note None
 *
 *******************************************************************************/
void get_init_vect_input_configs(warp_driver_Configs *drvconfigs, WARP_CFG *InputConfigs)
{
	struct Geometric_info_fl geo_param_fl;

	/*Get input configs into geo_param_fl*/
	geo_param_fl.fr_height = InputConfigs->frame_height;
	geo_param_fl.fr_width = InputConfigs->frame_width;

	geo_param_fl.lens_params.k_pre = InputConfigs->pre_fisheye;
	geo_param_fl.lens_params.k_post = InputConfigs->post_fisheye;

	geo_param_fl.keystone_params.left_top_x = InputConfigs->left_top_x;
	geo_param_fl.keystone_params.left_top_y = InputConfigs->left_top_y;
	geo_param_fl.keystone_params.left_bottom_x = InputConfigs->left_bottom_x;
	geo_param_fl.keystone_params.left_bottom_y = InputConfigs->left_bottom_y;
	geo_param_fl.keystone_params.right_top_x = InputConfigs->right_top_x;
	geo_param_fl.keystone_params.right_top_y = InputConfigs->right_top_y;
	geo_param_fl.keystone_params.right_bottom_x = InputConfigs->right_bottom_x;
	geo_param_fl.keystone_params.right_bottom_y = InputConfigs->right_bottom_y;

	geo_param_fl.affine_param.scale_x = InputConfigs->scale_x;
	geo_param_fl.affine_param.scale_y = InputConfigs->scale_y;
	geo_param_fl.affine_param.trans_x = InputConfigs->trans_x;
	geo_param_fl.affine_param.trans_y = InputConfigs->trans_y;
	geo_param_fl.affine_param.rot_angle = InputConfigs->rotation;
	geo_param_fl.affine_param.zoom = InputConfigs->zoom;

	geo_param_fl.warp_type = InputConfigs->warp_type;

	est_geometric_transform(&geo_param_fl);

	config_warp_convert_float_fixed(&geo_param_fl, drvconfigs);

	drvconfigs->initVectConfigs.ctr_pts = InputConfigs->ctrl_pts;
	drvconfigs->initVectConfigs.bytes_per_pixel = 3; /*supports only 444 for now*/
	drvconfigs->initVectConfigs.num_ctrl_pts = InputConfigs->num_ctrl_pts;
	drvconfigs->initVectConfigs.filter_table_addr_0 = REMAP_VECT_0_ADDR_0/4;
	drvconfigs->initVectConfigs.filter_table_addr_1 = REMAP_VECT_0_ADDR_1/4;

	drvconfigs->filterConfigs.height = InputConfigs->frame_height;
	drvconfigs->filterConfigs.width = InputConfigs->frame_width;
	drvconfigs->filterConfigs.seg_table_addr = REMAP_VECT_0_ADDR_1;
	drvconfigs->filterConfigs.stride = InputConfigs->frame_width * 3;
	drvconfigs->filterConfigs.format = 20;
	drvconfigs->golden_crc = 0xff; /* changes for different outputs for different warp configurations */
}
