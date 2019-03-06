/*
 * Copyright (c) 2019, Xilinx Inc. and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*
 * @file	scatterlist.h
 * @brief	Scatter list primitives for libmetal.
 */

#ifndef __METAL_SCATTERLIST__H__
#define __METAL_SCATTERLIST__H__

#include <metal/io.h>

#ifdef __cplusplus
extern "C" {
#endif

/** \defgroup scatterlist Scatter List Interfaces
 *  @{ */

/** Metal scatter list structure. */
struct metal_scatter_list {
	struct metal_io_region	*ios;    /**< I/O region array */
	int nents; /**< number of I/O region entries */
};

/**
 * @brief Get number of entries of a scatter list
 *
 * @param[in]	sg_list	Scatter list handle
 * @return	number of entries of the scatter list
 */
static inline
int metal_scatterlist_get_nents(struct metal_scatter_list *sg_list)
{
	return sg_list->nents;
}

/**
 * @brief Get I/O entries array of a scatter list
 *
 * @param[in]	sg_list	Scatter list handle
 * @param[out]	ios handle of I/O regions entries array
 * @return	of entries of the scatter list
 */
static inline
int metal_scatterlist_get_ios(struct metal_scatter_list *sg_list,
			      struct metal_io_region **ios)
{
	*ios = sg_list->ios;
	return sg_list->nents;
}

/** @} */

#ifdef __cplusplus
}
#endif

#endif /* __METAL_SCATTERLIST__H__ */
