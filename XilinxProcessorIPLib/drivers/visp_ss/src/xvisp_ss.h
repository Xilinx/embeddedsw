/******************************************************************************
* Copyright (C) 2025 Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

#ifndef XVISP_SS_H         /* prevent circular inclusions */
#define XVISP_SS_H         /* by using protection macros */

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/

#include "xil_types.h"
#include "xil_assert.h"
#include "xstatus.h"

/**************************** Type Definitions ******************************/
#ifdef __linux__
typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
#else

/**
* This typedef contains configuration information for the VISP SS core
* Each core instance should have a configuration structure associated.
*/
typedef struct {
#ifndef SDT
	u16 DeviceId;             /**< Unique ID of device */
#else
	char *Name;		          /**< Unique Name of device (compatible string) */
#endif
	UINTPTR BaseAddress;      /**< The base address of the core instance. */
	u16 NumStreams;           /**< Number of streams */
	u16 Rpu;                  /**< Rpu ID */
	u16 IspId;                /**< ISP ID */
	char *IoMode;             /**< IO mode info */
	/* IBA0 Configuration */
	u16 Iba0FrameRate;        /**< Iba0 framerate support */
	u16 Iba0DataFormat;       /**< Iba0 dataformat support */
	u16 Iba0Ppc;              /**< Iba0 pixel per clock */
	u16 Iba0MaxWidth;         /**< Iba0 Maximum columns support */
	u16 Iba0MaxHeight;        /**< Iba0 Maximum rows support */
	u16 Iba0Vcid;             /**< Iba0 vcid support */
	/* OBA0 Main Path Configuration */
	u16 Oba0MpBpp;            /**< Oba0 main path of bits per pixel */
	u16 Oba0MpPpc;            /**< Oba0 main path of pixel per clock */
	char *Oba0MpDataFormat;   /**< Oba0 main path of dataformat support */
	/* OBA0 Self Path Configuration */
	u16 Oba0SpBpp;            /**< Oba0 self path of bits per pixel */
	u16 Oba0SpPpc;            /**< Oba0 self path of pixel per clock */
	char *Oba0SpDataFormat;   /**< Oba0 self path of dataformat support */
	/* IBA1 Configuration */
	u16 Iba1FrameRate;        /**< Iba1 framerate support */
	u16 Iba1DataFormat;       /**< Iba1 dataformat support */
	u16 Iba1Ppc;              /**< Iba1 pixel per clock */
	u16 Iba1MaxWidth;         /**< Iba1 Maximum columns support */
	u16 Iba1MaxHeight;        /**< Iba1 Maximum rows support */
	u16 Iba1Vcid;             /**< Iba1 vcid support */
	/* IBA2 Configuration */
	u16 Iba2FrameRate;        /**< Iba2 framerate support */
	u16 Iba2DataFormat;       /**< Iba2 dataformat support */
	u16 Iba2Ppc;              /**< Iba2 pixel per clock */
	u16 Iba2MaxWidth;         /**< Iba2 Maximum columns support */
	u16 Iba2MaxHeight;        /**< Iba2 Maximum rows support */
	u16 Iba2Vcid;             /**< Iba2 vcid support */
	/* IBA3 Configuration */
	u16 Iba3FrameRate;        /**< Iba3 framerate support */
	u16 Iba3DataFormat;       /**< Iba3 dataformat support */
	u16 Iba3Ppc;              /**< Iba3 pixel per clock */
	u16 Iba3MaxWidth;         /**< Iba3 Maximum columns support */
	u16 Iba3MaxHeight;        /**< Iba3 Maximum rows support */
	u16 Iba3Vcid;             /**< Iba3 vcid support */
	/* IBA4 Configuration */
	u16 Iba4FrameRate;        /**< Iba4 framerate support */
	u16 Iba4DataFormat;       /**< Iba4 dataformat support */
	u16 Iba4Ppc;              /**< Iba4 pixel per clock */
	u16 Iba4MaxWidth;         /**< Iba4 Maximum columns support */
	u16 Iba4MaxHeight;        /**< Iba4 Maximum rows support */
	u16 Iba4Vcid;             /**< Iba4 vcid support */
	/* OBA1 Configuration */
	u16 Oba1MpBpp;            /**< Oba1 main path of bits per pixel */
	u16 Oba1MpPpc;            /**< Oba1 main path of pixel per clock */
	char *Oba1MpDataFormat;   /**< Oba1 main path of dataformat support */
	u16 Oba1SpBpp;            /**< Oba1 self path of bits per pixel */
	u16 Oba1SpPpc;            /**< Oba1 self path of pixel per clock */
	char *Oba1SpDataFormat;   /**< Oba1 self path of dataformat support */
	/* Interrupt Configuration */
	u16 Interrupts[4];        /**< Interrupts array */
	u32 InterruptParent;      /**< Interrupt parent ID */
} xvisp_ss_Config;

#endif

/**
 * Driver instance data. An instance must be allocated for each VISP SS core in use.
 */
typedef struct {
	xvisp_ss_Config Config;   /**< Hardware Configuration */
	u32 IsReady;              /**< Device and the driver instance are initialized */
} XVisp_Ss;

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

/*
 * Static Initialization API's in xvisp_ss_sinit.c
 */
#ifndef SDT
int XVisp_Ss_Initialize(XVisp_Ss *InstancePtr, u16 DeviceId);
xvisp_ss_Config *XVisp_Ss_LookupConfig(u16 DeviceId);
#else
int XVisp_Ss_Initialize(XVisp_Ss *InstancePtr, UINTPTR BaseAddress);
xvisp_ss_Config *XVisp_Ss_LookupConfig(UINTPTR BaseAddress);
#endif

/*
 * Initialization and control functions implemented in xvisp_ss.c
 */
int XVisp_Ss_CfgInitialize(XVisp_Ss *InstancePtr, xvisp_ss_Config *Config,
			   UINTPTR EffectiveAddr);
void XVisp_Ss_Enable(XVisp_Ss *InstancePtr);
void XVisp_Ss_Disable(XVisp_Ss *InstancePtr);
void XVisp_Ss_Reset(XVisp_Ss *InstancePtr);

/*
 * Configuration printing function
 */
void XVisp_Ss_PrintConfig(XVisp_Ss *InstancePtr);

void reset_isp2rpu_mapping();
void init_isp2rpu_mapping(u32 rpu_id, u32 isp_id);
void selectDestinationCore(u32 ispid);

#ifdef __cplusplus
}
#endif

#endif /* end of protection macro */
