/******************************************************************************
* Copyright (C) 2014 - 2020 Xilinx, Inc.  All rights reserved.
* Copyright 2022-2023 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* <b>Initialization & Configuration</b>
*
* The device driver enables higher layer software (e.g., an application) to
* communicate to the Audio Formatter core.
*
* XAudioFormatter_CfgInitialize() API is used to initialize the Audio Formatter
* core. The user needs to first call the XAudioFormatter_LookupConfig() API
* which returns the Configuration structure pointer which is passed as a
* parameter to the XAudioFormatter_CfgInitialize() API.
*
* <b> Interrupts </b>
* The driver does the interrupt handling, and dispatch to the user application
* through callback functions that user has registered.
*
* <b> Virtual Memory </b>
*
* This driver supports Virtual Memory. The RTOS is responsible for calculating
* the correct device base address in Virtual Memory space.
*
* <b> Threads </b>
*
* This driver is not thread safe. Any needs for threads or thread mutual
* exclusion must be satisfied by the layer above this driver.
*
* <b> Asserts </b>
*
* Asserts are used within all Xilinx drivers to enforce constraints on argument
* values. Asserts can be turned off on a system-wide basis by defining, at
* compile time, the NDEBUG identifier. By default, asserts are turned on and it
* is recommended that users leave asserts on during development.
*
* @file xaudioformatter.h
* @addtogroup audio_formatter Overview
* @{
* @details
*
* This header file contains identifiers and register-level driver functions (or
* macros), range macros, structure typedefs that can be used to access the
* Xilinx Audio Formatter core instance.
*
*
******************************************************************************/

#ifndef XAUDFMT_H_
#define XAUDFMT_H_	/**< Prevent circular inclusions
			  *  by using protection macros */

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/

#include "xaudioformatter_hw.h"
#include "xil_types.h"
#include "xil_assert.h"
#include "xstatus.h"
#include "xil_cache.h"

/************************** Constant Definitions *****************************/

enum XAudioFormatter_BitDepth {
        BIT_DEPTH_8,
        BIT_DEPTH_16,
        BIT_DEPTH_20,
        BIT_DEPTH_24,
        BIT_DEPTH_32,
};

typedef enum {
        XAudioFormatter_S2MM,
        XAudioFormatter_MM2S
} XAudioFormatter_ChannelId;

typedef enum {
        XAudioFormatter_IOC_Handler,
        XAudioFormatter_TIMEOUT_Handler,
        XAudioFormatter_ERROR_Handler,
} XAudioFormatter_HandlerType;

/***************** Macros (Inline Functions) Definitions *********************/


/**************************** Type Definitions *******************************/
typedef void (*XAudioFormatter_Callback)(void *CallbackRef);
/**
* This typedef contains configuration information for a audio formatter core.
* Each audio formatter core should have a configuration structure associated.
*/
typedef struct {
#ifndef SDT
	u16 DeviceId;		/**< DeviceId is the unique ID of the
				  *  device */
#else
	char *Name;
#endif
	u32 BaseAddress;	/**< BaseAddress is the physical base address
				  *  of the device's registers */
	u8 IncludeMM2S; 	/**< IncludeMM2S is MM2S included or not */
	u8 IncludeS2MM; 	/**< IncludeS2MM is S2MM included or not */
	u8 MaxChannelsMM2S;	/**< MaxChannelsMM2S is the max number of
				  *  channels supported in MM2S */
	u8 MaxChannelsS2MM;	/**< MaxChannelsS2MM is the max number of
				  *  channels supported in S2MM */
	u8 MM2SAddrWidth;	/**< MM2SAddrWidth is the MM2S Address Width */
	u8 MM2SDataFormat;	/**< MM2SDataFormat is the MM2S Data format */
	u8 PackagingModeMM2S;	/**< PackagingModeMM2S is the MM2S packaging
				  *  mode */
	u8 PackagingModeS2MM;	/**< PackagingModeS2MM is the S2MM packaging
				  *  mode */
	u8 S2MMAddrWidth;	/**< S2MMAddrWidth is the S2MM Address Width */
	u8 S2MMDataFormat;	/**< S2MMDataFormat is the S2MM Data format */
#ifdef SDT
	u16 IntrId;		/**< Interrupt ID */
	UINTPTR IntrParent;	/**< Bit[0] Interrupt parent type Bit[64/32:1]
				  *  Parent base address */
#endif
} XAudioFormatter_Config;

extern XAudioFormatter_Config XAudioFormatter_ConfigTable[];
/******************************************************************************/
/**
*
* The audio formatter driver instance data structure. A pointer to an instance
* data structure is passed around by functions to refer to a specific driver
* instance.
*/
typedef struct {
	XAudioFormatter_Config Config;	/**< Hardware configuration */
	u32 IsReady;			/**< Device and the driver instance
					  *  are initialized */
	u32 BaseAddress;
	_Bool mm2s_presence;
	_Bool s2mm_presence;
	XAudioFormatter_ChannelId ChannelId;
	XAudioFormatter_Callback S2MMIOCCallback;
	void *S2MMIOCCallbackRef;
	XAudioFormatter_Callback MM2SIOCCallback;
	void *MM2SIOCCallbackRef;
	XAudioFormatter_Callback S2MMTOCallback;
	void *S2MMTOCallbackRef;
	XAudioFormatter_Callback MM2STOCallback;
	void *MM2STOCallbackRef;
	XAudioFormatter_Callback S2MMERRCallback;
	void *S2MMERRCallbackRef;
	XAudioFormatter_Callback MM2SERRCallback;
	void *MM2SERRCallbackRef;
} XAudioFormatter;

/**
* This typedef contains hw params information for a audio formatter core.
*/
typedef struct {
	u64 buf_addr;
	u32 active_ch;
	u32 bits_per_sample;
	u32 periods;
	u32 bytes_per_period;
} XAudioFormatterHwParams;

/*****************************************************************************/


/************************** Function Prototypes ******************************/
#ifndef SDT
XAudioFormatter_Config *XAudioFormatter_LookupConfig(u16 DeviceId);
u32 XAudioFormatter_Initialize(XAudioFormatter *InstancePtr, u16 DeviceId);
#else
XAudioFormatter_Config *XAudioFormatter_LookupConfig(UINTPTR BaseAddress);
u32 XAudioFormatter_Initialize(XAudioFormatter *InstancePtr, UINTPTR BaseAddress);
#endif

u32 XAudioFormatter_CfgInitialize(XAudioFormatter *InstancePtr,
	XAudioFormatter_Config *CfgPtr);
void XAudioFormatter_InterruptEnable(XAudioFormatter *InstancePtr, u32 Mask);
void XAudioFormatter_InterruptDisable(XAudioFormatter *InstancePtr, u32 Mask);
void XAudioFormatterDMAStart(XAudioFormatter *InstancePtr);
void XAudioFormatterDMAStop(XAudioFormatter *InstancePtr);
void XAudioFormatterSetHwParams(XAudioFormatter *InstancePtr,
	XAudioFormatterHwParams *hw_params);
void XAudioFormatter_SetS2MMCallback(XAudioFormatter *InstancePtr,
	XAudioFormatter_HandlerType handler_type, void *CallbackFunc,
	void *CallbackRef);
void XAudioFormatter_SetMM2SCallback(XAudioFormatter *InstancePtr,
	XAudioFormatter_HandlerType handler_type, void *CallbackFunc,
	void *CallbackRef);
void *XAudioFormatterS2MMIntrHandler(void *InstancePtr);
void *XAudioFormatterMM2SIntrHandler(void *InstancePtr);
void XAudioFormatter_InterruptClear(XAudioFormatter *InstancePtr, u32 mask);
void XAudioFormatterDMAReset(XAudioFormatter *InstancePtr);
void XAudioFormatterSetFsMultiplier(XAudioFormatter *InstancePtr, u32 Mclk,
	u32 Fs);
u32 XAudioFormatterGetDMATransferCount(XAudioFormatter *InstancePtr);
void XSdiAud_GetChStat(XAudioFormatter *InstancePtr, u8 *ChStatBuf);
void XAudioFormatterSetS2MMTimeOut(XAudioFormatter *InstancePtr, u32 TimeOut);
/******************************************************************************/

#ifdef __cplusplus
}

#endif

#endif /* End of protection macro */
/** @} */
