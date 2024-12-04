/******************************************************************************
 * * Copyright (C) 2024 Advanced Micro Devices, Inc.  All rights reserved.
 * * SPDX-License-Identifier: MIT
 * *******************************************************************************/

#ifndef XVISP_SS_H         /* prevent circular inclusions */
#define XVISP_SS_H         /* by using protection macros */

#ifdef __cplusplus
extern "C" {
#endif

/**************************** Type Definitions ******************************/
#ifdef __linux__
typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
#else

/**
* This typedef contains configuration information for the frame buffer write core
* Each core instance should have a configuration structure associated.
*/
typedef struct {
#ifndef SDT
	u16 DeviceId;             /**< Unique ID of device */
#else
	char *Name;		    /**< Unique Name of device */
#endif
	UINTPTR BaseAddress;      /**< The base address of the core instance. */
	u16 x;            /**< Io Type variable> */
	u16 y;
	u16 z;
	/* it is variable, must and should properies what we need in intlization */
} xvisp_ss_Config;
#endif
