/******************************************************************************
*
* Copyright (C) 2017 Xilinx, Inc. All rights reserved.
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
*
* Use of the Software is limited solely to applications:
* (a) running on a Xilinx device, or
* (b) that interact with a Xilinx device through a bus or interconnect.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
* XILINX BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
* WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
* OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*
* Except as contained in this notice, the name of the Xilinx shall not be used
* in advertising or otherwise to promote the sale, use or other dealings in
* this Software without prior written authorization from Xilinx.
*
******************************************************************************/
/*****************************************************************************/
/**
*
* @file xv_sdirx.h
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who    Date     Changes
* ----- ------ -------- --------------------------------------------------
* 1.0   jsr    07/17/17 Initial release.
* </pre>
*
******************************************************************************/
#ifndef XV_SDIRX_H_
#define XV_SDIRX_H_		/**< Prevent circular inclusions
				 *  by using protection macros */

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/

#include "xv_sdirx_hw.h"
#include "xil_assert.h"
#include "xstatus.h"
#include "xdebug.h"
#include "xv_sdivid.h"
#include "xvidc.h"

/************************** Constant Definitions *****************************/

#define XV_SDIRX_MAX_DATASTREAM 8
#define XV_SDIRX_VID_LCK_WINDOW	0x3000	/* The period of clock cycles Video
										   lock interrupt must be asserted
										   before interrupt is generated */

/** @name Handler Types
* @{
*/
/**
* These constants specify different types of handler and used to differentiate
* interrupt requests from peripheral.
*/
typedef enum {
	XV_SDIRX_HANDLER_STREAM_DOWN = 1,
	XV_SDIRX_HANDLER_STREAM_UP,
	XV_SDIRX_HANDLER_OVERFLOW,
	XV_SDIRX_HANDLER_UNDERFLOW
} XV_SdiRx_HandlerType;
/*@}*/

/** @name SDI Transport Mode
* @{
*/
typedef enum {
	XV_SDIRX_MODE_HD = 0,
	XV_SDIRX_MODE_SD,
	XV_SDIRX_MODE_3G,
	XV_SDIRX_MODE_UNKNOWN,
	XV_SDIRX_MODE_6G,
	XV_SDIRX_MODE_12G,
	XV_SDIRX_MODE_NUM_SUPPORTED
} XV_SdiRx_Modes;

/** @name SDI Debug Settings
* @{
*/
typedef enum {
	XV_SDIRX_DBGSELID_STRMINFO = 0,
	XV_SDIRX_DBGSELID_TIMINGINFO,
	XV_SDIRX_DBGSELID_SDIINFO,
	XV_SDIRX_DBGSELID_SDIDBGINFO,
	XV_SDIRX_DBGSELID_REGDUMP
} XV_SdiRx_DebugSelId;

/** @name SDI Transport Family Encoding
* @{
*/
typedef enum {
	XV_SDIRX_SMPTE_ST_274    = 0,
	XV_SDIRX_SMPTE_ST_296    = 1,
	XV_SDIRX_SMPTE_ST_2048_2 = 2,
	XV_SDIRX_SMPTE_ST_295    = 3,
	XV_SDIRX_NTSC            = 8,
	XV_SDIRX_PAL             = 9,
} XV_SdiRx_Family_Encoding;

/**
* SDI Search Modes
*/
typedef enum {
	XV_SDIRX_SINGLESEARCHMODE_HD = 0,	/**< Sdi HD */
	XV_SDIRX_SINGLESEARCHMODE_SD = 1,	/**< Sdi SD */
	XV_SDIRX_SINGLESEARCHMODE_3G = 2,	/**< Sdi 3G */
	XV_SDIRX_SINGLESEARCHMODE_6G = 4,	/**< Sdi 6G */
	XV_SDIRX_SINGLESEARCHMODE_12GI = 5,	/**< Sdi 12G Integer Framerate*/
	XV_SDIRX_SINGLESEARCHMODE_12GF = 6,	/**< Sdi 12G Fractional Framerate*/
	XV_SDIRX_MULTISEARCHMODE = 10
} XV_SdiRx_SearchMode;

/**
* SDI Supported Modes
* @{
*/
typedef enum {
	XV_SDIRX_SUPPORT_HD = 1,		/**< Sdi HD */
	XV_SDIRX_SUPPORT_SD = 2,		/**< Sdi SD */
	XV_SDIRX_SUPPORT_3G = 4,		/**< Sdi 3G */
	XV_SDIRX_SUPPORT_6G = 8,		/**< Sdi 6G */
	XV_SDIRX_SUPPORT_12GI = 16,		/**< Sdi 12G Integer Framerate*/
	XV_SDIRX_SUPPORT_12GF = 32,		/**< Sdi 12G Fractional Framerate*/
	XV_SDIRX_SUPPORT_ALL = 63
} XV_SdiRx_SupportedModes;

/** @name SDI Transport Rate
* @{
*/
typedef enum {
	XV_SDIRX_FR_NONE    = 0,
	XV_SDIRX_FR_UNKNOWN,
	XV_SDIRX_FR_23_98HZ,
	XV_SDIRX_FR_24HZ,
	XV_SDIRX_FR_47_95HZ,
	XV_SDIRX_FR_25HZ,
	XV_SDIRX_FR_29_97HZ,
	XV_SDIRX_FR_30HZ,
	XV_SDIRX_FR_48HZ,
	XV_SDIRX_FR_50HZ,
	XV_SDIRX_FR_59_94HZ,
	XV_SDIRX_FR_60HZ,
	XV_SDIRX_FR_NUM_SUPPORTED
} XV_SdiRx_FrameRate;

/**************************** Type Definitions *******************************/

/**
* This typedef contains configuration information for the SDI RX core.
* Each SDI RX device should have a configuration structure associated.
*/
typedef struct {
	u16 DeviceId;		/**< DeviceId is the unique ID of the SDI RX core */
	UINTPTR BaseAddress;	/**< BaseAddress is the physical base address
							*    of the core's registers */
	u8 IsEdhIncluded;
	u8 MaxRateSupported;
} XV_SdiRx_Config;

/**
* This typedef contains SDI RX stream specific data structure.
*/
typedef struct {
	XVidC_VideoStream	Video;	/**< Video stream for SDI RX */

	XSdiVid_Standard	Standard;
	XSdiVid_ChannelAssignment	CAssignment;	/**< Channel assignment */
	u32 PayloadId;
} XV_SdiRx_Stream;

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
typedef void (*XV_SdiRx_Callback)(void *CallbackRef);

/**
* The XSdiRx driver instance data. An instance must be allocated for each
* SDI RX core in use.
*/
typedef struct {
	XV_SdiRx_Config Config;			/**< Hardware Configuration */
	u32				IsReady;		/**< Core and the driver instance are initialized */

	XV_SdiRx_Callback StreamDownCallback;	/**< Callback for stream down callback */
	void	*StreamDownRef;			/**< To be passed to the stream down callback */
	u32		IsStreamDownCallbackSet;	/**< Set flag. This flag is set to true when the
									*    callback has been registered */

	XV_SdiRx_Callback	StreamUpCallback;		/**< Callback for stream up callback */
	void	*StreamUpRef;				/**< To be passed to the stream up callback */
	u32		IsStreamUpCallbackSet;		/**< Set flag. This flag is set to true when
										*    the callback has been registered */

	XV_SdiRx_Callback	OverFlowCallback;		/**< Callback for Overflow callback */
	void	*OverFlowRef;				/**< To be passed to the Overflow callback */
	u32		IsOverFlowCallbackSet;		/**< Set flag. This flag is set to true when
										*    the callback has been registered */

	XV_SdiRx_Callback	UnderFlowCallback;		/**< Callback for Underflow callback */
	void	*UnderFlowRef;				/**< To be passed to the Underflow callback */
	u32		IsUnderFlowCallbackSet;		/**< Set flag. This flag is set to true when
										*    the callback has been registered */

	/* SDI RX stream */
	XV_SdiRx_Stream		Stream[XV_SDIRX_MAX_DATASTREAM];	/**< SDI RX stream information */
	XSdiVid_Transport	Transport;
	u8					SupportedModes;
	u8					VideoStreamNum;
} XV_SdiRx;

/***************** Macros (Inline Functions) Definitions *********************/

/*****************************************************************************/
/**
*
* This macro reads the RX version
*
* @param  InstancePtr is a pointer to the XSdi_RX core instance.
*
* @return RX version.
*
* *note	C-style signature:
*		u32 XV_SdiRx_GetVersion(XV_SdiRx *InstancePtr)
*
******************************************************************************/
#define XV_SdiRx_GetVersion(InstancePtr) \
	XV_SdiRx_ReadReg((InstancePtr)->Config.BaseAddress, (XV_SDIRX_VER_OFFSET))

/* Clear VideoLock Interrupt */
#define XV_SdiRx_VidLckIntrClr(InstancePtr) \
	XV_SdiRx_WriteReg((InstancePtr)->Config.BaseAddress, (XV_SDIRX_INT_CLR_OFFSET), \
		(XV_SDIRX_INT_STS_VID_LOCK_MASK))

/* Clear VideoUnlock Interrupt */
#define XV_SdiRx_VidUnlckIntrClr(InstancePtr) \
	XV_SdiRx_WriteReg((InstancePtr)->Config.BaseAddress, (XV_SDIRX_INT_CLR_OFFSET), \
		(XV_SDIRX_INT_STS_VID_UNLOCK_MASK))

/************************** Function Prototypes ******************************/

/* Initialization function in xv_sdirx_sinit.c */
XV_SdiRx_Config *XV_SdiRx_LookupConfig(u16 DeviceId);

/* Initialization and control functions in xv_sdirx.c */
int XV_SdiRx_CfgInitialize(XV_SdiRx *InstancePtr,
	XV_SdiRx_Config *CfgPtr,
	UINTPTR EffectiveAddr);
void XV_SdiRx_ResetStream(XV_SdiRx *InstancePtr);
u32 XV_SdiRx_GetPayloadId(XV_SdiRx *InstancePtr, u8 DataStream);
u32 XV_SdiRx_GetSdiMode(XV_SdiRx *InstancePtr);
void XV_SdiRx_FramerEnable(XV_SdiRx *InstancePtr);
void XV_SdiRx_FramerDisable(XV_SdiRx *InstancePtr);
void XV_SdiRx_SetEdhErrCntTrigger(XV_SdiRx *InstancePtr, u32 Enable);
void XV_SdiRx_EnableMode(XV_SdiRx *InstancePtr,
		XV_SdiRx_SupportedModes SupportModes);
void XV_SdiRx_DisableMode(XV_SdiRx *InstancePtr,
		XV_SdiRx_SupportedModes RemoveModes);
void XV_SdiRx_Start(XV_SdiRx *InstancePtr, XV_SdiRx_SearchMode Mode);
int XV_SdiRx_Stop(XV_SdiRx *InstancePtr);
u32 XV_SdiRx_ReportDetectedError(XV_SdiRx *InstancePtr);
void XV_SdiRx_SetVidLckWindow(XV_SdiRx *InstancePtr, u32 Data);

/* Bridge and reset specific functions */
void XV_SdiRx_VidBridgeEnable(XV_SdiRx *InstancePtr);
void XV_SdiRx_VidBridgeDisable(XV_SdiRx *InstancePtr);
void XV_SdiRx_Axi4sBridgeEnable(XV_SdiRx *InstancePtr);
void XV_SdiRx_Axi4sBridgeDisable(XV_SdiRx *InstancePtr);

/* Log specific functions */
void XV_SdiRx_DebugInfo(XV_SdiRx *InstancePtr, XV_SdiRx_DebugSelId SelId);


/* Self test function in xv_sdirx_selftest.c */
u32 XV_SdiRx_SelfTest(XV_SdiRx *InstancePtr);

/* Interrupt related function in xv_sdirx_intr.c */
u32 XV_SdiRx_GetIntrEnable(XV_SdiRx *InstancePtr);
u32 XV_SdiRx_GetIntrStatus(XV_SdiRx *InstancePtr);
void XV_SdiRx_InterruptClear(XV_SdiRx *InstancePtr, u32 Mask);
void XV_SdiRx_IntrHandler(void *InstancePtr);

int XV_SdiRx_SetCallback(XV_SdiRx *InstancePtr,
	u32 HandlerType,
	void *CallbackFunc,
	void *CallbackRef);
void XV_SdiRx_IntrDisable(XV_SdiRx *InstancePtr, u32 Mask);
void XV_SdiRx_IntrEnable(XV_SdiRx *InstancePtr, u32 Mask);

/************************** Variable Declarations ****************************/
/************************** Variable Declarations ****************************/


#ifdef __cplusplus
}
#endif

#endif /* End of protection macro */
