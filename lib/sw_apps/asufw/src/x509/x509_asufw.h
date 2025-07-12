/**************************************************************************************************
* Copyright (c) 2025 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/

/*************************************************************************************************/
/**
 *
 * @file x509_asufw.h
 *
 * This file contains declarations for x509_asufw.c file.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- ----------------------------------------------------------------------------
 * 1.0   rmv  05/19/25 Initial release
 *
 * </pre>
 *
 *************************************************************************************************/
/**
* @addtogroup x509_apis X.509 APIs
* @{
*/
#ifndef X509_ASUFW_H_
#define X509_ASUFW_H_

#ifdef __cplusplus
extern "C" {
#endif

/*************************************** Include Files *******************************************/
#include "xil_types.h"
#include "xsha.h"
#include "xasufw_dma.h"

/************************************ Constant Definitions ***************************************/

/************************************** Type Definitions *****************************************/
/**
 * This typedef contains the platform specific data.
 */
typedef struct {
	XSha *ShaPtr;			/**< SHA instance pointer */
	XAsufw_Dma *DmaPtr;		/**< DMA instance pointer */
} X509_PlatData;

/*************************** Macros (Inline Functions) Definitions *******************************/

/************************************ Function Prototypes ****************************************/
s32 X509_CfgInitialize(void);

/************************************ Variable Definitions ***************************************/

#ifdef __cplusplus
}
#endif

#endif  /* X509_ASUFW_H_ */
/** @} */
