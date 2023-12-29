/******************************************************************************
* Copyright (C) 2017 - 2020 Xilinx, Inc. All rights reserved.
* Copyright 2022-2023 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xv_sditx.h

* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who    Date     Changes
* ----- ------ -------- --------------------------------------------------
* 1.00  jsr    07/17/17 Initial release.
* 	jsr    02/23/2018 YUV420 color format support.
# 2.0   vve    10/03/18 Add support for ST352 in C Stream
* </pre>
*
******************************************************************************/
#ifndef XV_SDITX_H_
#define XV_SDITX_H_        /**< Prevent circular inclusions
			     *  by using protection macros */

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/

#include "xv_sditx_hw.h"
#include "xil_assert.h"
#include "xstatus.h"
#include "xdebug.h"
#include "xvidc.h"
#include "xv_sdivid.h"

/************************** Constant Definitions *****************************/

#define XV_SDITX_MAX_DATASTREAM 8
#define XV_SDITX_COLORFORMAT	(0x0 << 16)
#define XV_SDITX_COLORDEPTH_10	(0x1 << 24)
#define XV_SDITX_COLORDEPTH_12	(0x2 << 24)
/**************************** Type Definitions *******************************/

/** @name Handler Types
* @{
*/
/**
* These constants specify different types of handler and used to differentiate
* interrupt requests from peripheral.
*/
typedef enum {
	XV_SDITX_HANDLER_GTRESET_DONE = 1,
	XV_SDITX_HANDLER_OVERFLOW,
	XV_SDITX_HANDLER_UNDERFLOW,
	XV_SDITX_HANDLER_CEALIGN,
	XV_SDITX_HANDLER_AXI4SVIDLOCK

/*	XV_SDITX_HANDLER_STREAM_UP */
} XV_SdiTx_HandlerType;
/*@}*/

/** @name SDI TX stream status
* @{
*/
typedef enum {
	XV_SDITX_STATE_GTRESETDONE_WORKAROUND,	/* GTResetDone workaround */
	XV_SDITX_STATE_GTRESETDONE_NORMAL	/* Stream up */
} XV_SdiTx_State;

/** @name SDI Stream Configurable Settings
* @{
*/
typedef enum {
	XV_SDITX_STREAMSELID_VMID,
	XV_SDITX_STREAMSELID_COLORFORMAT,
	XV_SDITX_STREAMSELID_BPC,
	XV_SDITX_STREAMSELID_PPC,
	XV_SDITX_STREAMSELID_ASPECTRATIO,
	XV_SDITX_STREAMSELID_STANDARD,
	XV_SDITX_STREAMSELID_STREAMINTERLACE,
	XV_SDITX_STREAMSELID_CHANNEL,
} XV_SdiTx_StreamSelId;

/** @name SDI Debug Settings
* @{
*/
typedef enum {
	XV_SDITX_DBGSELID_STRMINFO = 0,
	XV_SDITX_DBGSELID_TIMINGINFO,
	XV_SDITX_DBGSELID_SDIINFO,
	XV_SDITX_DBGSELID_SDIDBGINFO,
	XV_SDITX_DBGSELID_REGDUMP
} XV_SdiTx_DebugSelId;

/** @name SDI Core Configurable Settings
* @{
*/
typedef enum {
	XV_SDITX_CORESELID_INSERTCRC,
	XV_SDITX_CORESELID_INSERTST352,
	XV_SDITX_CORESELID_ST352OVERWRITE,
	XV_SDITX_CORESELID_INSERTSYNCBIT,
	XV_SDITX_CORESELID_SDBITREPBYPASS,
	XV_SDITX_CORESELID_USEANCIN,
	XV_SDITX_CORESELID_INSERTLN,
	XV_SDITX_CORESELID_INSERTEDH,
} XV_SdiTx_CoreSelId;

/** @name SDI Mux Pattern
* @{
*/
typedef enum {
	XV_SDITX_MUX_SD_HD_3GA = 0,
	XV_SDITX_MUX_3GB = 1,
	XV_SDITX_MUX_8STREAM_6G_12G = 2,
	XV_SDITX_MUX_4STREAM_6G = 3,
	XV_SDITX_MUX_16STREAM_12G = 4
} XV_SdiTx_MuxPattern;

/** @name Default Payload Id Line 1 Number
* @{
*/
typedef enum {
	XV_SDITX_PAYLOADLN1_HD_3G_6G_12G = 10,
	XV_SDITX_PAYLOADLN1_SDPAL = 9,
	XV_SDITX_PAYLOADLN1_SDNTSC = 13
} XV_SdiTx_PayloadLineNum1;

/** @name Default Payload Id Line 2 Number
* @{
*/
typedef enum {
	XV_SDITX_PAYLOADLN2_HD_3G_6G_12G = 572,
	XV_SDITX_PAYLOADLN2_SDPAL = 322,
	XV_SDITX_PAYLOADLN2_SDNTSC = 276
} XV_SdiTx_PayloadLineNum2;

/**
* This typedef contains configuration information for the SDI TX core.
* Each SDI TX device should have a configuration structure associated.
*/
typedef struct {
#ifndef SDT
    u16 DeviceId;       /**< DeviceId is the unique ID of the SDI TX core */
#else
    char *Name;
#endif
    UINTPTR BaseAddress;    /**< BaseAddress is the physical
			     * base address of the core's registers */
    u8 IsEdhIncluded;
    u8 MaxRateSupported;
    u8 InsertCSTRST352;         /**< Insert ST352 in C stream */
} XV_SdiTx_Config;

/**
* This typedef contains SDI TX stream specific data structure.
*/
typedef struct {
	XVidC_VideoStream	Video;	/**< Video stream for SDI TX */
	XSdiVid_Standard	Standard;
	XVidC_VideoFormat	IsStreamInterlaced;
	XSdiVid_ChannelAssignment	CAssignment;	/**< Channel assignment */
	u32 PayloadId;
	u8 IsPsF;
} XV_SdiTx_Stream;

/**
* Callback type for interrupt.
*
* @param	CallbackRef is a callback reference passed in by the upper
*		layer when setting the callback functions, and passed back to
*		the upper layer when the callback is invoked.
*
* @return	None.
*
* @note		None.
*
*/
typedef void (*XV_SdiTx_Callback)(void *CallbackRef);

/**
* The XV_SdiTx driver instance data. An instance must be allocated for each
* SDI TX core in use.
*/
typedef struct {
  XV_SdiTx_Config	Config;	/**< Hardware Configuration */
  u32			IsReady;	/**< Core and the driver instance are initialized */
  XV_SdiTx_Callback	GtRstDoneCallback;	/**< Callback for stream down callback */
  void			*GtRstDoneRef;		/**< To be passed to the stream down callback */

  XV_SdiTx_Callback	OverFlowCallback;	/**< Callback for over flow callback */
  void			*OverFlowRef;		/**< To be passed to the over flow callback */

  XV_SdiTx_Callback	UnderFlowCallback;	/**< Callback for under flow callback */
  void			*UnderFlowRef;		/**< To be passed to the under flow callback */

  XV_SdiTx_Callback	CeAlignErrCallback;	/**< Callback for CE align error callback */
  void			*CeAlignErrRef;		/**< To be passed to the CE align error callback */

  XV_SdiTx_Callback	Axi4sVidLockCallback;	/**< Callback for Axi4s video lock callback */
  void			*Axi4sVidLockRef;	/**< To be passed to the Axi4s Video lock callback */

  XV_SdiTx_Stream	Stream[XV_SDITX_MAX_DATASTREAM];/**< SDI TX stream information */
  XSdiVid_Transport	Transport;	/**< SDI TX Transport information */
  XV_SdiTx_State	State;		/**< State */
  u8			IsStreamUp;
  XVidC_ColorDepth	bitdepth;	/**< bit depth */
} XV_SdiTx;

/***************** Macros (Inline Functions) Definitions *********************/

/*****************************************************************************/
/**
*
* This macro reads the TX version
*
* @param  InstancePtr is a pointer to the XV_SdiTX core instance.
*
* @return None.
*
*
******************************************************************************/
#define XV_SdiTx_GetVersion(InstancePtr) \
  XV_SdiTx_ReadReg((InstancePtr)->Config.BaseAddress, \
  (XV_SDITX_VER_OFFSET))

/* Clear VideoLock Interrupt */
#define XV_SdiTx_VidLckIntrClr(InstancePtr) \
	XV_SdiTx_WriteReg((InstancePtr)->Config.BaseAddress, (XV_SDITX_INT_CLR_OFFSET), \
		(XV_SDITX_INT_STS_VID_LOCK_MASK))

/* Clear VideoUnlock Interrupt */
#define XV_SdiTx_VidUnlckIntrClr(InstancePtr) \
	XV_SdiTx_WriteReg((InstancePtr)->Config.BaseAddress, (XV_SDITX_INT_CLR_OFFSET), \
		(XV_SDITX_INT_STS_VID_UNLOCK_MASK))

/************************** Function Prototypes ******************************/

/* Initialization function in xv_sditx_sinit.c */
#ifndef SDT
XV_SdiTx_Config *XV_SdiTx_LookupConfig(u16 DeviceId);
#else
XV_SdiTx_Config *XV_SdiTx_LookupConfig(UINTPTR BaseAddress);
#endif

/* Initialization and control functions in xv_sditx.c */
int XV_SdiTx_CfgInitialize(XV_SdiTx *InstancePtr, XV_SdiTx_Config *CfgPtr,
			    UINTPTR EffectiveAddr);
void XV_SdiTx_Reset(XV_SdiTx *InstancePtr);
void XV_SdiTx_ClearPayloadId(XV_SdiTx *InstancePtr);
u32 XV_SdiTx_SetStream(XV_SdiTx *InstancePtr, XV_SdiTx_StreamSelId SelId,
			u32 Data, u8 StreamId);
void XV_SdiTx_StreamStart(XV_SdiTx *InstancePtr);
void XV_SdiTx_StartSdi(XV_SdiTx *InstancePtr,
	XSdiVid_TransMode SdiMode,
	XSdiVid_BitRate IsFractional,
	XV_SdiTx_MuxPattern MuxPattern);
int XV_SdiTx_StopSdi(XV_SdiTx *InstancePtr);
int XV_SdiTx_SetVidFormat(XV_SdiTx *InstancePtr, XVidC_ColorFormat ColorFormat);
void XV_SdiTx_ReportDetectedError(XV_SdiTx *InstancePtr);
void XV_SdiTx_ClearDetectedError(XV_SdiTx *InstancePtr);
u32 XV_SdiTx_GetPayloadEotf(XV_SdiTx *InstancePtr, XVidC_Eotf Eotf, XVidC_ColorStd Colorimetry);
u32 XV_SdiTx_GetPayload(XV_SdiTx *InstancePtr, XVidC_VideoMode VideoMode, XSdiVid_TransMode SdiMode, u8 DataStream);
void XV_SdiTx_SetPayloadId(XV_SdiTx *InstancePtr, u8 DataStream, u32 Payload);
void XV_SdiTx_SetPayloadLineNum(XV_SdiTx *InstancePtr,
				XV_SdiTx_PayloadLineNum1 Field1LineNum,
				XV_SdiTx_PayloadLineNum2 Field2LineNum,
				u8 Field2En);
void XV_SdiTx_SetCoreSettings(XV_SdiTx *InstancePtr,
				XV_SdiTx_CoreSelId SelId, u8 Data);

u8 XV_SdiTx_GetPayloadFrameRate(XVidC_FrameRate FrameRateValid, XSdiVid_BitRate BitRate);
u8 XV_SdiTx_GetPayloadIsInterlaced(XVidC_VideoFormat VideoFormat);
u8 XV_SdiTx_GetPayloadAspectRatio(XVidC_AspectRatio AspectRatio);
u32 XV_SdiTx_GetPayloadByte1(u16 VActiveValid, XSdiVid_TransMode SdiMode, u8 *Data);
u8 XV_SdiTx_GetPayloadColorFormat(XSdiVid_TransMode SdiMode, XVidC_ColorFormat ColorFormatId);
int XV_SdiTx_SetColorFormat(XV_SdiTx *InstancePtr, XVidC_ColorFormat ColorFormat);
void XV_SdiTx_ST352CStreamEnable(XV_SdiTx *InstancePtr);
void XV_SdiTx_ST352CSwitch3GA(XV_SdiTx *InstancePtr);

/* Bridge and reset specific functions */
void XV_SdiTx_VidBridgeEnable(XV_SdiTx *InstancePtr);
void XV_SdiTx_VidBridgeDisable(XV_SdiTx *InstancePtr);
void XV_SdiTx_Axi4sBridgeVtcEnable(XV_SdiTx *InstancePtr);
void XV_SdiTx_Axi4sBridgeVtcDisable(XV_SdiTx *InstancePtr);
void XV_SdiTx_SetVidBridgeMode(XV_SdiTx *InstancePtr, XSdiVid_TransMode Mode);

/* Log specific functions */
void XV_SdiTx_DebugInfo(XV_SdiTx *InstancePtr, XV_SdiTx_DebugSelId SelId);

/* Self test function in xv_sditx_selftest.c */
u32 XV_SdiTx_SelfTest(XV_SdiTx *InstancePtr);

/* Interrupt related function in xv_sditx_intr.c */
u32 XV_SdiTx_GetIntrEnable(XV_SdiTx *InstancePtr);
u32 XV_SdiTx_GetIntrStatus(XV_SdiTx *InstancePtr);
void XV_SdiTx_InterruptClear(XV_SdiTx *InstancePtr, u32 Mask);
void XV_SdiTx_IntrHandler(void *InstancePtr);
int XV_SdiTx_SetCallback(XV_SdiTx *InstancePtr,	u32 HandlerType,
				void *CallbackFunc, void *CallbackRef);
void XV_SdiTx_IntrDisable(XV_SdiTx *InstancePtr, u32 Mask);
void XV_SdiTx_IntrEnable(XV_SdiTx *InstancePtr, u32 Mask);
void XV_SdiTx_SetYCbCr444_RGB_10bit(XV_SdiTx *InstancePtr);
void XV_SdiTx_ClearYCbCr444_RGB_10bit(XV_SdiTx *InstancePtr);
void XV_SdiTx_Set_Bpc(XV_SdiTx *InstancePtr,
		XVidC_ColorDepth bitdepth);

/************************** Variable Declarations ****************************/

/************************** Variable Declarations ****************************/

#ifdef __cplusplus
}
#endif

#endif /* End of protection macro */
