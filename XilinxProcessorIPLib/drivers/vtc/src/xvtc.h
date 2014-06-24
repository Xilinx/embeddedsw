/******************************************************************************
*
* Copyright (C) 2008 - 2014 Xilinx, Inc.  All rights reserved.
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
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* XILINX CONSORTIUM BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
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
* @file xvtc.h
*
* This is the main header file of Xilinx MVI Video Timing Controller (VTC)
* device driver. The VTC device detects video signals, independently
* overrides any one of them, re-generates video signals with +/- delay and
* with polarity inversion, and generates up to 16 one cycle Frame Sync outputs.
*
* The device has the following main features:
* - Detect video signals:
*	- horizontal sync
*	- horizontal blank
*	- vertical sync
*	- vertical blank
*	- active video
*	- field id
* - Independently override any one signal.
* - Re-generate video signals with +/- delay and with polarity inversion.
* - Generate up to 16 one cycle Frame Sync outputs.
*
* For a full description of VTC features, please see the hardware
* specification.
*
* <b>Interrupt Service </b>
*
* The interrupt types supported are:
* - Frame Sync Interrupts 0 - 15
* - Generator interrupt
*	- Generator Active Video Interrupt
*	- Generator VBLANK Interrupt
* - Detector interrupt:
*	- Detector Active Video Interrupt
*	- Detector VBLANK Interrupt
* - Signal Lock interrupt
*	- Active Chroma signal lock
*	- Active Video Signal Lock
*	- Field ID Signal Lock
*	- Vertical Blank Signal Lock
*	- Vertical Sync Signal Lock
*	- Horizontal Blank Signal Lock
*	- Horizontal Sync Signal Lock
*
* <b>Software Initialization </b>
*
* The application needs to do following steps in order for preparing the
* VTC to be ready to process video signal handling.
*
* - Call XVtc_LookupConfig() using a device ID to find the device
*   configuration.
* - Call XVtc_CfgInitialize() to initialize the device and the driver
*   instance associated with it.
* - Call XVtc_SetGenerator() to set up the video signals to generate,
*   if desired.
* - Call XVtc_SetPolarity() to set up the video signal polarity.
* - Call XVtc_SetSource() for source selection
* - Call XVtc_SetGeneratorHoriOffset() to set up the Generator
*   VBlank/VSync horizontal offsets, if values other than the default are
*   needed
* - Call XVtc_EnableSync(), if generator needs to be synced to the detector
* - Call XVtc_Enable() to enable/start the VTC device.
*
* <b> Examples </b>
*
* An example is provided with this driver to demonstrate the driver usage.
*
* <b>Cache Coherency</b>
*
* <b>Alignment</b>
*
* <b>Limitations</b>
*
* <b>BUS Interface</b>
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date       Changes
* ----- ---- --------    -----------------------------------------------
* 1.00a xd   08/05/08    First release
* 1.01a xd   07/23/10    Added GIER; Added more h/w generic info into
*                        xparameters.h; Feed callbacks with pending
*                        interrupt info. Added Doxygen & Version support
* 2.00a xd   05/13/11    1. Renamed to "Video Timing Controller"
*                        2. Added Generator/Detector VBlank/VSync
*                           Horizontal offset setup/fetch support;
*                        3. Renamed the IP to support to be axi_vtc;
*                        4. Supported sync between generator and detector
*                           with addition of new XVtc_EnableSync() and
*                           XVtc_DisableSync() functions;
*                        5. Renamed XVtc_SetSync() to XVtc_SetFSync();
*                        6. Renamed XVtc_GetSync() to XVtc_GetFSync();
*                        7. Removed unnecessary register write in
*                           XVtc_Reset();
*                        8. Corrected driver name in .mdd file
*                        9. Updated register bit definition(a few fields grow
*                           from 12 to 13 bit wide)
* 2.00a cm   05/25/11    1. Renamed XVtc_SetSkip() to XVtc_SetSkipLine();
*                        2. Renamed XVtc_GetSkip() to XVtc_GetSkipLine();
*                        3. Added XVtc_SetSkipPixel();
*                        4. Added XVtc_GetSkipPixel();
* 2.00a cm   06/16/12    1. Added missing xil_assert.h include
* 2.00a cm   07/25/12    1. Removed unused XVtc_IntrSetLockPolarity() function
* 3.00a cm   08/02/12    1. Added the XVtc_Sync_Reset() frame sync'ed SW 
*                           reset function.
* 3.00a cjm  08/02/12 Converted from xio.h to xil_io.h, translating
*                     basic types, MB cache functions, exceptions and
*                     assertions to xil_io format. 
*                     Replaced the following 
*                     "XExc_Init" -> "Xil_ExceptionInit"
*                     "XExc_RegisterHandler" -> "Xil_ExceptionRegisterHandler"
*                     "XEXC_ID_NON_CRITICAL_INT" -> "XIL_EXCEPTION_ID_INT"
*                     "XExceptionHandler" -> "Xil_ExceptionHandler"
*                     "XExc_mEnableExceptions" -> "Xil_ExceptionEnable"
*                     "XEXC_NON_CRITICAL" -> "XIL_EXCEPTION_NON_CRITICAL"
*                     "XExc_DisableExceptions" -> "Xil_ExceptionDisable"
*                     "XExc_RemoveHandler" -> "Xil_ExceptionRemoveHandler"
*                     "microblaze_enable_interrupts" -> "Xil_ExceptionEnable"
*                     "microblaze_disable_interrupts" -> "Xil_ExceptionDisable"
*                     
*                     "XCOMPONENT_IS_STARTED" -> "XIL_COMPONENT_IS_STARTED"
*                     "XCOMPONENT_IS_READY" -> "XIL_COMPONENT_IS_READY"
*                     
*                     "XASSERT_NONVOID" -> "Xil_AssertNonvoid"
*                     "XASSERT_VOID_ALWAYS" -> "Xil_AssertVoidAlways"
*                     "XASSERT_VOID" -> "Xil_AssertVoid"
*                     "Xil_AssertVoid_ALWAYS" -> "Xil_AssertVoidAlways" 
*                     "XAssertStatus" -> "Xil_AssertStatus"
*                     "XAssertSetCallback" -> "Xil_AssertCallback"
*                     
*                     "XASSERT_OCCURRED" -> "XIL_ASSERT_OCCURRED"
*                     "XASSERT_NONE" -> "XIL_ASSERT_NONE"
*                     
*                     "microblaze_disable_dcache" -> "Xil_DCacheDisable"
*                     "microblaze_enable_dcache" -> "Xil_DCacheEnable"
*                     "microblaze_enable_icache" -> "Xil_ICacheEnable"
*                     "microblaze_disable_icache" -> "Xil_ICacheDisable"
*                     "microblaze_init_dcache_range" -> "Xil_DCacheInvalidateRange"
*                     
*                     "XCache_DisableDCache" -> "Xil_DCacheDisable"
*                     "XCache_DisableICache" -> "Xil_ICacheDisable"
*                     "XCache_EnableDCache" -> "Xil_DCacheEnableRegion"
*                     "XCache_EnableICache" -> "Xil_ICacheEnableRegion"
*                     "XCache_InvalidateDCacheLine" -> "Xil_DCacheInvalidateRange"
*                     
*                     "XUtil_MemoryTest32" -> "Xil_TestMem32"
*                     "XUtil_MemoryTest16" -> "Xil_TestMem16"
*                     "XUtil_MemoryTest8" -> "Xil_TestMem8"
*                     
*                     "xutil.h" -> "xil_testmem.h"
*                     
*                     "xbasic_types.h" -> "xil_types.h"
*                     "xio.h" -> "xil_io.h"
*                     
*                     "XIo_In32" -> "Xil_In32"
*                     "XIo_Out32" -> "Xil_Out32"
*                     
*                     "XTRUE" -> "TRUE"
*                     "XFALSE" -> "FALSE"
*                     "XNULL" -> "NULL"
*                     
*                     "Xuint8" -> "u8"
*                     "Xuint16" -> "u16"
*                     "Xuint32" -> "u32"
*                     "Xint8" -> "char"
*                     "Xint16" -> "short"
*                     "Xint32" -> "long"
*                     "Xfloat32" -> "float"
*                     "Xfloat64" -> "double"
*                     "Xboolean" -> "int"
*                     "XTEST_FAILED" -> "XST_FAILURE"
*                     "XTEST_PASSED" -> "XST_SUCCESS"
* 4.00a cjm  02/07/13 Removed Unused Functions
*                     XVtc_IntrEnableGlobal()
*                     XVtc_IntrDisableGlobal()
* 5.00a cjm  08/07/13 Replaced XVTC_RESET with XVTC_CTL
*                     Replaced XVTC_RESET_RESET_MASK with XVTC_CTL_RESET_MASK
*                     Replaced XVTC_SYNC_RESET_MASK with XVTC_CTL_SRST_MASK
* 5.00a cjm  10/30/13 Replaced XVtc_RegUpdate() with XVtc_RegUpdateEnable()
*                     Added XVtc_RegUpdateDisable()
*                     Removed type parameter from XVtc_Enable()   
*                     Added XVtc_EnableGenerator() to enable only the Generator  
*                     Added XVtc_EnableDetector() to enable only the Detector
* 5.00a cjm  11/01/13 Added Timing, VideoMode and Signal Conversion Functions:
*                       XVtc_ConvVideoMode2Timing()
*                       XVtc_ConvTiming2Signal()
*                       XVtc_ConvSignal2Timing()
*                       XVtc_ConvTiming2VideoMode()
*                     Added Timing and Video Mode Set/Get Functions:
*                       XVtc_SetGeneratorTiming()
*                       XVtc_SetGeneratorVideoMode()
*                       XVtc_GetGeneratorTiming()
*                       XVtc_GetGeneratorVideoMode()
*                       XVtc_GetDetectorTiming()
*                       XVtc_GetDetectorVideoMode()
*              
* 6.0   adk  19/12/13 Updated as per the New Tcl API's   
* </pre>
*
******************************************************************************/

#ifndef XVTC_H		  /* prevent circular inclusions */
#define XVTC_H		  /* by using protection macros */

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/

#include "xvtc_hw.h"
#include "xil_assert.h"
#include "xstatus.h"

/************************** Constant Definitions *****************************/

/** @name Interrupt Types for setting up Callbacks
 *  @{
 */
#define XVTC_HANDLER_FRAMESYNC	1 /**< A frame sync event interrupt type */
#define XVTC_HANDLER_LOCK	2 /**< A signal lock event interrupt type */
#define XVTC_HANDLER_DETECTOR	3 /**< A detector event interrupt type */
#define XVTC_HANDLER_GENERATOR	4 /**< A generator event interrupt type */
#define XVTC_HANDLER_ERROR	5 /**< An error condition interrupt type */
/*@}*/

/** @name Options for enabling VTC modules
 *  @{
 */
#define XVTC_EN_GENERATOR	1	/**< To enable generator */
#define XVTC_EN_DETECTOR	2	/**< To enable detector */
/*@}*/

/** @name Address gap between two register next to each other
 *  @{
 */
#define XVTC_REG_ADDRGAP	4


#define XVTC_VMODE_720P       1
#define XVTC_VMODE_1080P      2
#define XVTC_VMODE_480P       3
#define XVTC_VMODE_576P       4
#define XVTC_VMODE_VGA        5
#define XVTC_VMODE_SVGA       6
#define XVTC_VMODE_XGA        7
#define XVTC_VMODE_SXGA       8
#define XVTC_VMODE_WXGAPLUS   9
#define XVTC_VMODE_WSXGAPLUS  10
#define XVTC_VMODE_1080I      100
#define XVTC_VMODE_NTSC       101
#define XVTC_VMODE_PAL        102
/*@}*/

/**************************** Type Definitions *******************************/

/**
 * This typedef contains configuration information for a VTC device.
 * Each VTC device should have a configuration structure associated
 */
typedef struct {
	u16 DeviceId;	   /**< DeviceId is the unique ID  of the device */
	u32 BaseAddress;   /**< BaseAddress is the physical base address of the
			     *	device's registers */
} XVtc_Config;

/**
 * This typedef contains Polarity configuration information for a VTC
 * device.
 */
typedef struct {
	u8 ActiveChromaPol;	/**< Active Chroma Output Polarity */
	u8 ActiveVideoPol;	/**< Active Video Output Polarity */
	u8 FieldIdPol;		/**< Field ID Output Polarity */
	u8 VBlankPol;		/**< Vertical Blank Output Polarity */
	u8 VSyncPol;		/**< Vertical Sync Output Polarity */
	u8 HBlankPol;		/**< Horizontal Blank Output Polarity */
	u8 HSyncPol;		/**< Horizontal Sync Output Polarity */
} XVtc_Polarity;




/**
 * This typedef contains Source Selection configuration information for a
 * VTC device.
 */
typedef struct {

	u8 FieldIdPolSrc;		/**< Field ID Output Polarity Source */
	u8 ActiveChromaPolSrc;	/**< Active Chroma Output Polarity Source */
	u8 ActiveVideoPolSrc;	/**< Active Video Output Polarity Source */
	u8 HSyncPolSrc;		/**< Horizontal Sync Output Polarity Source */
	u8 VSyncPolSrc;		/**< Vertical Sync Output Polarity Source */
	u8 HBlankPolSrc;		/**< Horizontal Blank Output Polarity Source */
	u8 VBlankPolSrc;		/**< Vertical Blank Output Polarity Source */


	u8 VChromaSrc;		/**< Start of Active Chroma Register Source
				  *  Select */
	u8 VActiveSrc;		/**< Vertical Active Video Start Register
				  *  Source Select */
	u8 VBackPorchSrc;	/**< Vertical Back Porch Start Register Source
				  *  Select */
	u8 VSyncSrc;		/**< Vertical Sync Start Register Source Select
				  */
	u8 VFrontPorchSrc;	/**< Vertical Front Porch Start Register Source
				  *  Select */
	u8 VTotalSrc;		/**< Vertical Total Register Source Select */

	u8 HActiveSrc;		/**< Horizontal Active Video Start Register
				  *  Source Select */
	u8 HBackPorchSrc;	/**< Horizontal Back Porch Start Register
				  *  Source Select */
	u8 HSyncSrc;		/**< Horizontal Sync Start Register Source
				  *  Select */
	u8 HFrontPorchSrc;	/**< Horizontal Front Porch Start Register
				  *  Source Select */
	u8 HTotalSrc;		/**< Horizontal Total Register Source Select */

} XVtc_SourceSelect;

/**
 * This typedef contains the VTC signal configuration used by the
 * Generator/Detector modules in a VTC device.
 */
typedef struct {
  u16 OriginMode; 

	u16 HTotal;		/**< Horizontal total clock cycles per Line */
	u16 HFrontPorchStart;	/**< Horizontal Front Porch Start Cycle Count*/
	u16 HSyncStart;		/**< Horizontal Sync Start Cycle Count */
	u16 HBackPorchStart;	/**< Horizontal Back Porch Start Cycle Count */
	u16 HActiveStart;	/**< Horizontal Active Video Start Cycle Count */

	u16 V0Total;		/**< Total lines per Frame (Field 0) */
	u16 V0FrontPorchStart;	/**< Vertical Front Porch Start Line Count *  (Field 0) */
	u16 V0SyncStart;	/**< Vertical Sync Start Line Count (Field 0)*/
	u16 V0BackPorchStart;	/**< Vertical Back Porch Start Line Count *  (Field 0) */
	u16 V0ActiveStart;	/**< Vertical Active Video Start Line Count *  (Field 0) */
	u16 V0ChromaStart;	/**< Active Chroma Start Line Count (Field 0)*/

	u16 V1Total;		/**< Total lines per Frame (Field 1) */
	u16 V1FrontPorchStart;	/**< Vertical Front Porch Start Line Count *  (Field 1) */
	u16 V1SyncStart;	/**< Vertical Sync Start Line Count (Field 1)*/
	u16 V1BackPorchStart;	/**< Vertical Back Porch Start Line Count *  (Field 1)  */
	u16 V1ActiveStart;	/**< Vertical Active Video Start Line Count * (Field 1) */
	u16 V1ChromaStart;	/**< Active Chroma Start Line Count (Field 1)*/

} XVtc_Signal;


/**
 * This typedef contains Detector/Generator VBlank/VSync Horizontal Offset
 * configuration information for a VTC device.
 */
typedef struct {
	u16 V0BlankHoriStart;/**< Vertical Blank Hori Offset Start (field 0) */
	u16 V0BlankHoriEnd;  /**< Vertical Blank Hori Offset End   (field 0) */
	u16 V0SyncHoriStart; /**< Vertical Sync  Hori Offset Start (field 0) */
	u16 V0SyncHoriEnd;   /**< Vertical Sync  Hori Offset End   (field 0) */
	u16 V1BlankHoriStart;/**< Vertical Blank Hori Offset Start (field 1) */
	u16 V1BlankHoriEnd;  /**< Vertical Blank Hori Offset End   (field 1) */
	u16 V1SyncHoriStart; /**< Vertical Sync  Hori Offset Start (field 1) */
	u16 V1SyncHoriEnd;   /**< Vertical Sync  Hori Offset End   (field 1) */
} XVtc_HoriOffsets;


/**
 * This typedef contains Timing (typically in Display Timing) format
 * configuration information for a VTC device.
 */
typedef struct {
  // Horizontal Timing
  u16 HActiveVideo;   /**< Horizontal Active Video Size */
  u16 HFrontPorch;    /**< Horizontal Front Porch Size */
  u16 HSyncWidth;
  u16 HBackPorch;
  u16 HSyncPolarity;

  // Vertical Timing   
  u16 VActiveVideo;
  u16 V0FrontPorch;
  u16 V0SyncWidth;
  u16 V0BackPorch;

  u16 V1FrontPorch;
  u16 V1SyncWidth;
  u16 V1BackPorch;

  u16 VSyncPolarity;

  u8  Interlaced; 
} XVtc_Timing;


/**
 * Callback type for all interrupts except error interrupt.
 *
 * @param CallBackRef is a callback reference passed in by the upper layer
 *	  when setting the callback functions, and passed back to the
 *	  upper layer when the callback is invoked.
 * @param Mask is a bit mask indicating the cause of the event. For
 *	  current device version, this parameter is "OR" of 0 or more
 *	  XVTC_IXR_* constants defined in xvtc_hw.h
 */
typedef void (*XVtc_CallBack) (void *CallBackRef, u32 Mask);

/**
 * Callback type for Error interrupt.
 *
 * @param CallBackRef is a callback reference passed in by the upper layer
 *	  when setting the callback functions, and passed back to the
 *	  upper layer when the callback is invoked.
 * @param ErrorMask is a bit mask indicating the cause of the error. For
 *	  current device version, this parameter always have value 0 and
 *	  could be ignored.
 */
typedef void (*XVtc_ErrorCallBack) (void *CallBackRef, u32 ErrorMask);

/**
 * The XVtc driver instance data. An instance must be allocated for each
 * VTC device in use.
 */
typedef struct {
	XVtc_Config Config;		 /**< hardware configuration */
	u32 IsReady;			 /**< Device and the driver instance
					   *  are initialized */

	XVtc_CallBack FrameSyncCallBack; /**< Call back for Frame Sync
					   *  interrupt */
	void *FrameSyncRef;		 /**< To be passed to the Frame
					   *  Sync interrupt callback */

	XVtc_CallBack LockCallBack;      /**< Call back for Signal Lock
					   *  interrupt */
	void *LockRef;			 /**< To be passed to the Signal
					   *  Lock interrupt callback */

	XVtc_CallBack DetectorCallBack;  /**< Call back for Detector
					   *  interrupt */
	void *DetectorRef;		 /**< To be passed to the Detector
					   *  interrupt callback */

	XVtc_CallBack GeneratorCallBack; /**< Call back for Generator
					   *  interrupt */
	void *GeneratorRef;		 /**< To be passed to the
					   *  Generator interrupt
					   *  callback */

	XVtc_ErrorCallBack ErrCallBack;  /**< Call back for Error
					   *  interrupt */
	void *ErrRef;			 /**< To be passed to the Error
					   *  interrupt callback */

} XVtc;

/***************** Macros (Inline Functions) Definitions *********************/

/*****************************************************************************/
/**
*
* This macro resets a VTC device.
*
* @param  InstancePtr is a pointer to the VTC device instance to be worked
*	  on.
*
* @return None
*
* @note
* C-style signature:
*	 void XVtc_Reset(XVtc *InstancePtr)
*
******************************************************************************/
#define XVtc_Reset(InstancePtr) \
{ \
	XVtc_WriteReg((InstancePtr)->Config.BaseAddress, XVTC_CTL, \
			   XVTC_CTL_RESET_MASK); \
}

/*****************************************************************************/
/**
*
* This macro resets a VTC device after the next input frame is complete.
*
* @param  InstancePtr is a pointer to the VTC device instance to be worked
*	  on.
*
* @return None
*
* @note
* C-style signature:
*	 void XVtc_Sync_Reset(XVtc *InstancePtr)
*
******************************************************************************/
#define XVtc_Sync_Reset(InstancePtr) \
{ \
	XVtc_WriteReg((InstancePtr)->Config.BaseAddress, XVTC_CTL, \
			   XVTC_CTL_SRST_MASK); \
}


/*****************************************************************************/
/**
*
* This macro enables synchronization of the Generator with the Detector on
* the given VTC device.
*
* @param  InstancePtr is a pointer to the VTC device instance to be worked
*	  on.
*
* @return None
*
* @note
* C-style signature:
*	 void XVtc_EnableSync(XVtc *InstancePtr)
*
******************************************************************************/
#define XVtc_EnableSync(InstancePtr) \
{ \
	XVtc_WriteReg((InstancePtr)->Config.BaseAddress, XVTC_CTL, \
		XVtc_ReadReg((InstancePtr)->Config.BaseAddress, XVTC_CTL) | \
			XVTC_CTL_SE_MASK); \
}
/*****************************************************************************/
/**
*
* This macro enables updating timing registers at the end of each Generator
* frame. (DEPRECATED - replaced with XVtc_RegUpdateEnable)                   
*
* @param  InstancePtr is a pointer to the VTC device instance to be worked
*	  on.
*
* @return None
*
* @note
* C-style signature:
*	 void XVtc_RegUpdate(XVtc *InstancePtr)
*
******************************************************************************/
#define XVtc_RegUpdate(InstancePtr) \
{ \
	XVtc_WriteReg((InstancePtr)->Config.BaseAddress, XVTC_CTL, \
			XVtc_ReadReg((InstancePtr)->Config.BaseAddress, XVTC_CTL) | \
			XVTC_CTL_RU_MASK); \
}

/*****************************************************************************/
/**
*
* This macro enables updating timing registers at the end of each Generator
* frame. 
*
* @param  InstancePtr is a pointer to the VTC device instance to be worked
*	  on.
*
* @return None
*
* @note
* C-style signature:
*	 void XVtc_RegUpdateEnable(XVtc *InstancePtr)
*
******************************************************************************/
#define XVtc_RegUpdateEnable(InstancePtr) \
{ \
	XVtc_WriteReg((InstancePtr)->Config.BaseAddress, XVTC_CTL, \
			XVtc_ReadReg((InstancePtr)->Config.BaseAddress, XVTC_CTL) | \
			XVTC_CTL_RU_MASK); \
}

/*****************************************************************************/
/**
*
* This macro disables updating timing registers at the end of each Generator
* frame. 
*
* @param  InstancePtr is a pointer to the VTC device instance to be worked
*	  on.
*
* @return None
*
* @note
* C-style signature:
*	 void XVtc_RegUpdateDisable(XVtc *InstancePtr)
*
******************************************************************************/
#define XVtc_RegUpdateDisable(InstancePtr) \
{ \
	XVtc_WriteReg((InstancePtr)->Config.BaseAddress, XVTC_CTL, \
			XVtc_ReadReg((InstancePtr)->Config.BaseAddress, XVTC_CTL) & \
			(~XVTC_CTL_RU_MASK)); \
}

/*****************************************************************************/
/**
*
* This macro disables synchronization of the Generator with the Detector on
* the given VTC device.
*
* @param  InstancePtr is a pointer to the VTC device instance to be worked
*	  on.
*
* @return None
*
* @note
* C-style signature:
*	 void XVtc_DisableSync(XVtc *InstancePtr)
*
******************************************************************************/
#define XVtc_DisableSync(InstancePtr) \
{ \
	XVtc_WriteReg((InstancePtr)->Config.BaseAddress, XVTC_CTL, \
		XVtc_ReadReg((InstancePtr)->Config.BaseAddress, XVTC_CTL) & \
			~XVTC_CTL_SE_MASK); \
}

/*****************************************************************************/
/**
* This function gets the status of the Detector in a VTC device.
*
* @param  InstancePtr is a pointer to the VTC device instance to be
*	  worked on.
*
* @return The Detector Status. Use XVTC_DS_* in xvtc_hw.h to interpret
*	  the returned value.
*
* @note
* C-style signature:
*	  u32 XVtc_GetDetectionStatus(XVtc *InstancePtr)
*
******************************************************************************/
#define XVtc_GetDetectionStatus(InstancePtr) \
	XVtc_ReadReg((InstancePtr)->Config.BaseAddress, XVTC_DTSTAT)



/*****************************************************************************/
/**
*
* This macro enables individual interrupts of a VTC device.
*
* @param  InstancePtr is a pointer to the VTC device instance to be worked
*	  on.
*
* @param  IntrType is the type of the interrupts to enable. Use OR'ing of
*	  XVTC_IXR_* constants defined in xvtc_hw.h to create this
*	  parameter value.
*
* @return None
*
* @note
*
* The existing enabled interrupt(s) will remain enabled.
*
* C-style signature:
*	 void XVtc_IntrEnable(XVtc *InstancePtr, u32 IntrType)
*
******************************************************************************/
#define XVtc_IntrEnable(InstancePtr, IntrType) \
	XVtc_WriteReg((InstancePtr)->Config.BaseAddress, XVTC_IER, \
		((IntrType) & XVTC_IXR_ALLINTR_MASK) | \
		 XVtc_ReadReg((InstancePtr)->Config.BaseAddress, XVTC_IER))

/*****************************************************************************/
/**
*
* This macro disables individual interrupts of a VTC device.
*
* @param  InstancePtr is a pointer to the VTC device instance to be worked
*	  on.
*
* @param  IntrType is the type of the interrupts to disable. Use OR'ing of
*	  XVTC_IXR_* constants defined in xvtc_hw.h to create this
*	  parameter value.
*
* @return None
*
* @note
*
* Any other interrupt not covered by parameter IntrType, if enabled before
* this macro is called, will remain enabled.
*
* C-style signature:
*	 void XVtc_IntrDisable(XVtc *InstancePtr, u32 IntrType)
*
******************************************************************************/
#define XVtc_IntrDisable(InstancePtr, IntrType) \
	XVtc_WriteReg((InstancePtr)->Config.BaseAddress, XVTC_IER, \
		XVtc_ReadReg((InstancePtr)->Config.BaseAddress, XVTC_IER) \
		& ((~(IntrType)) & XVTC_IXR_ALLINTR_MASK))


/*****************************************************************************/
/**
*
* This macro returns the pending status of a VTC device.
*
* @param  InstancePtr is a pointer to the VTC device instance to be worked
*	  on.
*
* @return The pending interrupts of the VTC. Use XVTC_IXR_* constants
*	  defined in xvtc_hw.h to interpret this value.
*
* @note
*
* C-style signature:
*	 u32 XVtc_StatusGePending(XVtc *InstancePtr)
*
******************************************************************************/
#define XVtc_StatusGetPending(InstancePtr) \
	(XVtc_ReadReg((InstancePtr)->Config.BaseAddress, XVTC_ISR) & \
	 XVTC_IXR_ALLINTR_MASK)



/*****************************************************************************/
/**
*
* This macro returns the pending interrupts of a VTC device.
*
* @param  InstancePtr is a pointer to the VTC device instance to be worked
*	  on.
*
* @return The pending interrupts of the VTC. Use XVTC_IXR_* constants
*	  defined in xvtc_hw.h to interpret this value.
*
* @note
*
* C-style signature:
*	 u32 XVtc_IntrGetPending(XVtc *InstancePtr)
*
******************************************************************************/
#define XVtc_IntrGetPending(InstancePtr) \
	(XVtc_ReadReg((InstancePtr)->Config.BaseAddress, XVTC_IER) & \
	 XVtc_ReadReg((InstancePtr)->Config.BaseAddress, XVTC_ISR) & \
	 XVTC_IXR_ALLINTR_MASK)

/*****************************************************************************/
/**
*
* This macro clears/acknowledges pending interrupts of a VTC device.
*
* @param  InstancePtr is a pointer to the VTC device instance to be worked
*	  on.
*
* @param  IntrType is the pending interrupts to clear/acknowledge. Use OR'ing
*	  of XVTC_IXR_* constants defined in xvtc_hw.h to create this
*	  parameter value.
*
* @return None
*
* @note
*
* C-style signature:
*	 void XVtc_IntrClear(XVtc *InstancePtr, u32 IntrType)
*
******************************************************************************/
#define XVtc_IntrClear(InstancePtr, IntrType) \
	XVtc_WriteReg((InstancePtr)->Config.BaseAddress, XVTC_ISR, \
		(IntrType) & XVTC_IXR_ALLINTR_MASK)

/************************** Function Prototypes ******************************/

/*
 * Initialization and control functions in xvtc.c
 */

/* Initialization */
int XVtc_CfgInitialize(XVtc *InstancePtr, XVtc_Config *CfgPtr,
			u32 EffectiveAddr);

/* Enabling and Disabling */
void XVtc_EnableGenerator(XVtc *InstancePtr);
void XVtc_EnableDetector(XVtc *InstancePtr);
void XVtc_Enable(XVtc *InstancePtr);
void XVtc_DisableGenerator(XVtc *InstancePtr);
void XVtc_DisableDetector(XVtc *InstancePtr);
void XVtc_Disable(XVtc *InstancePtr);


/* Video Mode, Timing and Signal/HoriOffsets/Polarity Conversions*/
void XVtc_ConvVideoMode2Timing(XVtc *InstancePtr, u16 Mode, XVtc_Timing *TimingPtr);
void XVtc_ConvTiming2Signal(XVtc *InstancePtr, XVtc_Timing *TimingPtr, 
    XVtc_Signal *SignalCfgPtr, XVtc_HoriOffsets *HOffPtr, 
    XVtc_Polarity *PolarityPtr);
void XVtc_ConvSignal2Timing(XVtc *InstancePtr, XVtc_Signal *SignalCfgPtr, 
    XVtc_HoriOffsets *HOffPtr, XVtc_Polarity *PolarityPtr,
    XVtc_Timing *TimingPtr);
u16 XVtc_ConvTiming2VideoMode(XVtc *InstancePtr, XVtc_Timing *TimingPtr);

/* Timing/Video Mode Setting/Fetching */
void XVtc_SetGeneratorTiming(XVtc *InstancePtr, XVtc_Timing * TimingPtr);
void XVtc_SetGeneratorVideoMode(XVtc *InstancePtr, u16 Mode);
void XVtc_GetGeneratorTiming(XVtc *InstancePtr, XVtc_Timing *TimingPtr);
u16  XVtc_GetGeneratorVideoMode(XVtc *InstancePtr);
void XVtc_GetDetectorTiming(XVtc *InstancePtr, XVtc_Timing *TimingPtr);
u16  XVtc_GetDetectorVideoMode(XVtc *InstancePtr);

/* Polarity setting */
void XVtc_SetPolarity(XVtc *InstancePtr, XVtc_Polarity *PolarityPtr);
void XVtc_GetPolarity(XVtc *InstancePtr, XVtc_Polarity *PolarityPtr);
void XVtc_GetDetectorPolarity(XVtc *InstancePtr, XVtc_Polarity *PolarityPtr);

/* Source selection */
void XVtc_SetSource(XVtc *InstancePtr, XVtc_SourceSelect *SourcePtr);
void XVtc_GetSource(XVtc *InstancePtr, XVtc_SourceSelect *SourcePtr);


/* Skipping setting */
void XVtc_SetSkipLine(XVtc *InstancePtr, int GeneratorChromaSkip);
void XVtc_GetSkipLine(XVtc *InstancePtr, int *GeneratorChromaSkipPtr);
void XVtc_SetSkipPixel(XVtc *InstancePtr, int GeneratorChromaSkip);
void XVtc_GetSkipPixel(XVtc *InstancePtr, int *GeneratorChromaSkipPtr);

/* VTC generator/detector setting/fetching */
void XVtc_SetGenerator(XVtc *InstancePtr, XVtc_Signal *SignalCfgPtr);
void XVtc_GetGenerator(XVtc *InstancePtr, XVtc_Signal *SignalCfgPtr);
void XVtc_GetDetector(XVtc *InstancePtr, XVtc_Signal *SignalCfgPtr);

/* Delay setting */
void XVtc_SetDelay(XVtc *InstancePtr, int VertDelay, int HoriDelay);
void XVtc_GetDelay(XVtc *InstancePtr, int *VertDelayPtr, int *HoriDelayPtr);

/* Frame Sync setting */
void XVtc_SetFSync(XVtc *InstancePtr, u16 FrameSyncIndex,
			   u16 VertStart, u16 HoriStart);
void XVtc_GetFSync(XVtc *InstancePtr, u16 FrameSyncIndex,
			   u16 *VertStartPtr, u16 *HoriStartPtr);

/* Horizontal Offset Setting */
void XVtc_SetGeneratorHoriOffset(XVtc *InstancePtr,
			   XVtc_HoriOffsets *HoriOffset);
void XVtc_GetGeneratorHoriOffset(XVtc *InstancePtr,
			   XVtc_HoriOffsets *HoriOffset);
void XVtc_GetDetectorHoriOffset(XVtc *InstancePtr,
			   XVtc_HoriOffsets *HoriOffset);


/* Version functions */
void XVtc_GetVersion(XVtc *InstancePtr, u16 *Major, u16 *Minor, u16 *Revision);

/*
 * Initialization function(s) in xvtc_sinit.c
 */
XVtc_Config *XVtc_LookupConfig(u16 DeviceId);

/*
 * Interrupt related function(s) in xvtc_intr.c
 */
void XVtc_IntrHandler(void *InstancePtr);
int XVtc_SetCallBack(XVtc *InstancePtr, u32 IntrType,
			void *CallBackFunc, void *CallBackRef);


#ifdef __cplusplus
}
#endif

#endif /* end of protection macro */
