/**************************************************************************************************
* Copyright (c) 2024 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/

/*************************************************************************************************/
/**
 *
 * @file xasufw_status.h
 * @addtogroup Overview
 * @{
 *
 * This file contains ASU client error codes
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- ----------------------------------------------------------------------------
 * 1.0   vns  08/06/24 Initial release
 *
 * </pre>
 *
 *************************************************************************************************/

#ifndef XASU_STATUS_H
#define XASU_STATUS_H

#ifdef __cplusplus
extern "C" {
#endif

/*************************************** Include Files *******************************************/
#include "xil_types.h"

/************************************ Constant Definitions ***************************************/

/************************************** Type Definitions *****************************************/

/*
 * This contains the client error codes
 */
enum XAsu_Status {
	XASU_INVALID_ARGUMENT = 0x10,
	XASU_QUEUE_FULL,
};
/*************************** Macros (Inline Functions) Definitions *******************************/

/************************************ Function Prototypes ****************************************/

/************************************ Variable Definitions ***************************************/

#ifdef __cplusplus
}
#endif

#endif  /* XASU_STATUS_H */
/** @} */