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
	u16 NumStreams;           /**< Number of streams> */
	u16 Rpu;                  /**< Rpu ID> */
	u16 IoMode;               /**<Iomode info> */
	u16 Iba0FrameRate;        /**<Iba0 framerate supprot> */
	u16 Iba0DataFormat;       /**<Iba0 dataformat supprot> */
	u16 Iba0Ppc;              /**<Iba0 pixel per clock> */
	u16 Iba0MaxWidth;         /**<Iba0 Maximum columns support> */
	u16 Iba0MaxHeight;        /**<Iba0 Maximum rows support> */
	u16 Iba0Vcid;             /**<Iba0 vcid support> */
	u16 Oba0MpBpp;            /**<Oba0 main path of bits per pixel> */
	u16 Oba0MpPpc;            /**<Oba0 main path of pixel per clock > */
	u16 Oba0MpDataFormat;     /**<Oba0 main path of dataformat support> */
	u16 Oba0SpBpp;            /**<Oba0 self path of bits per pixel> */
	u16 Oba0SpPpc;            /**<Oba0 self path of pixel per clock > */
	u16 Oba0SpDataFormat;     /**<Oba0 self path of dataformat support> */
	u16 Iba1_frame_rate;      /**<Iba1 framerate supprot> */
	u16 Iba1_data_format;     /**<Iba1 dataformat supprot> */
	u16 Iba1_ppc;             /**<Iba1 pixel per clock> */
	u16 Iba1_max-width;       /**<Iba1 Maximum columns support> */
	u16 Iba1_max-height;      /**<Iba1 Maximum rows support> */
	u16 Iba1_vcid;            /**<Iba1 vcid support> */
	u16 Iba2FrameRate;        /**<Iba2 framerate supprot> */
	u16 Iba2DataFormat;       /**<Iba2 dataformat supprot> */
	u16 Iba2Ppc;              /**<Iba2 pixel per clock> */
	u16 Iba2MaxWidth;         /**<Iba2 Maximum columns support> */
	u16 Iba2MaxHeight;        /**<Iba2 Maximum rows support> */
	u16 Iba2Vcid;             /**<Iba2 vcid support> */
	u16 Iba3FrameRate;        /**<Iba3 framerate supprot> */
	u16 Iba3DataFormat;       /**<Iba3 dataformat supprot> */
	u16 Iba3Ppc;              /**<Iba3 pixel per clock> */
	u16 Iba3MaxWidth;         /**<Iba3 Maximum columns support> */
	u16 Iba3MaxHeight;        /**<Iba3 Maximum rows support> */
	u16 Iba3Vcid;             /**<Iba3 vcid support> */
	u16 Iba4FrameRate;        /**<Iba4 framerate supprot> */
	u16 Iba4DataFormat;       /**<Iba4 dataformat supprot> */
	u16 Iba4Ppc;              /**<Iba4 pixel per clock> */
	u16 Iba4MaxWidth;         /**<Iba4 Maximum columns support> */
	u16 Iba4MaxHeight;        /**<Iba4 Maximum rows support> */
	u16 Iba4Vcid;             /**<Iba4 vcid support> */
	u16 Oba1MpBpp;            /**<Oba1 main path of bits per pixel> */
	u16 Oba1MpPpc;            /**<Oba1 main path of pixel per clock > */
	u16 Oba1MpDataFormat;     /**<Oba1 main path of dataformat support> */
	u16 Oba1SpBpp;            /**<Oba1 self path of bits per pixel> */
	u16 Oba1SpPpc;            /**<Oba1 self path of pixel per clock > */
	u16 Oba1SpDataFormat;     /**<Oba1 self path of dataformat support> */
#ifdef SDT
	u16 IntrId; 		    /**< Interrupt ID */
	UINTPTR IntrParent; 	    /**< Bit[0] Interrupt parent type Bit[64/32:1] Parent base address */
#endif
} xvisp_ss_Config;
#endif
