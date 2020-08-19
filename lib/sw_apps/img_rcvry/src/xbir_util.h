/******************************************************************************
* Copyright (c) 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xbir_util.h
*
* This file contains list of APIs provided by utility module (xbir_util.c file)
*
******************************************************************************/
#ifndef XBIR_UTIL_H
#define XBIR_UTIL_H

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/************************** Function Prototypes ******************************/
const char* Xbir_UtilGetFileExt (const char *FileName);
u8 Xbir_UtilIsNumber (const char *Str);

#ifdef __cplusplus
}
#endif

#endif