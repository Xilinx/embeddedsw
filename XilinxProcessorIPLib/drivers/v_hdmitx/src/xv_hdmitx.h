/******************************************************************************
*
* Copyright (C) 2016 Xilinx, Inc. All rights reserved.
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
* @file xv_hdmitx.h
*
* This is the main header file for Xilinx HDMI TX core. HDMI TX core is used
* for transmitting the incoming video and audio streams. It consists of
* - Transmitter core
* - AXI4-Stream to Video Bridge
* - Video Timing Controller and
* - High-bandwidth Digital Content Protection (HDCP) (Optional).
*
* The HDMI TX uses three AXI interfaces for Video, Audio and Processor:
* - AXI4-Stream interface for Video, can be single, dual or quad pixels per
* clock and supports 8 and 10 bits per component.
* - AXI4-Stream interface for Audio, accepts multiple channels uncompressed
* and compressed audio data.
* - AXI4-Lite interface for processor, controls the transmitter.
* Please do refer AXI Reference Guide (UG761) for more information on AXI
* interfaces.
*
* Transmitter core performs following operations:
* - Converts video data from the video clock domain into the link clock domain.
* - TMDS (Transition Minimized Differential Signaling) encoding.
* - Merges encoded video data and packet data into a single HDMI stream.
* - Optional HDMI stream is encrypted by an external HDCP module.
* - Over samples HDMI stream if stream bandwidth is too low for the transceiver
* to handle.
* - Scrambles encrypted/HDMI stream if data rate is above 3.4 Gbps otherwise
* bypasses the Scrambler.
*
* AXI Video Bridge converts the incoming video AXI-stream to native video.
*
* Video Timing Controller (VTC) generates the native video timing.
*
* <b>Core Features </b>
*
* For a full description of HDMI TX features, please see the hardware
* specification.
*
* <b>Software Initialization & Configuration</b>
*
* The application needs to do following steps in order for preparing the
* HDMI TX core to be ready.
*
* - Call XV_HdmiTx_LookupConfig using a device ID to find the core
*   configuration.
* - Call XV_HdmiTx_CfgInitialize to initialize the device and the driver
*   instance associated with it.
*
* <b>Interrupts </b>
*
* This driver provides interrupt handlers
* - XV_HdmiTx_IntrHandler, for handling the interrupts from the HDMI TX core
* PIO and DDC peripheral respectively.
*
* Application developer needs to register interrupt handler with the processor,
* within their examples. Whenever processor calls registered application's
* interrupt handler associated with interrupt id, application's interrupt
* handler needs to call appropriate peripheral interrupt handler reading
* peripheral's Status register.

* This driver provides XV_HdmiTx_SetCallback API to register functions with HDMI
* TX core instance.
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
* values. Asserts can be turned off on a system-wide basis by defining at
* compile time, the NDEBUG identifier. By default, asserts are turned on and it
* is recommended that users leave asserts on during development.
*
* <b> Building the driver </b>
*
* The HDMI TX driver is composed of several source files. This allows the user
* to build and link only those parts of the driver that are necessary.
*
* <pre>
* MODIFICATION HISTORY:
*s
* Ver   Who    Date     Changes
* ----- ------ -------- --------------------------------------------------
* 1.00         10/07/15 Initial release.
* 1.1   yh     15/01/16 Add 3D Support
* 1.2   MG     09/03/16 Added XV_HdmiTx_SetHdmiMode and XV_HdmiTx_SetDviMode.
*                       Removed support for reduced blanking
* 1.3   YH     25/07/16 Used UINTPTR instead of u32 for BaseAddress
*                       XV_HdmiTx_Config
*                       XV_HdmiTx_CfgInitialize
* 1.4   YH     17/08/16 Added XV_HdmiTx_SetAxiClkFreq
* 1.5   YH     14/11/16 Added XV_HdmiTx_Bridge_yuv420 & XV_HdmiTx_Bridge_pixel
*                       mode macros
* 1.6   MG     28/03/17 Added XV_HdmiTx_Mask macros
* 1.7   YH     19/07/17 Added XV_HdmiTx_IsMasked macro
*              22/08/17 Added XV_HdmiTx_Audio_LPCM macro
*                       Added XV_HdmiTx_Audio_HBR macro
* 1.8   YH     06/10/17 Replaced XV_HdmiTx_Audio_LPCM and XV_HdmiTx_Audio_HBR
*                           macro with API XV_HdmiTx_SetAudioFormat
*                       Added XV_HdmiTx_GetAudioFormat
* 1.9   EB     24/10/17 Added enum XV_HdmiTx_AudioFormatType
* 1.10  MMO    19/12/17 Added XV_HdmiTx_SetTmdsClk API
* 2.0   EB     16/01/18 Updated function XV_HdmiTx_SetTmdsClk and renamed to
*                           XV_HdmiTx_ConfigTmdsClk
*                       Moved Vendor Specific InfoFrame related functions to
*                           HDMI Common library
*                       Deprecating XV_HdmiTx_VSIF_GeneratePacket,
*                           XV_HdmiTx_VSIF_DisplayInfo,
*                           XV_HdmiTx_VSIF_3DStructToString,
*                           XV_HdmiTx_VSIF_3DSampMethodToString and
*                           XV_HdmiTx_VSIF_3DSampPosToString APIs
*                       Moved VicTable, XV_HdmiTx_Aux to Hdmi Common library
*       EB     24/01/18 Added OverrideHdmi14Scrambler to XV_HdmiTx_Stream
* </pre>
*
******************************************************************************/
#ifndef XV_HDMITX_H_
#define XV_HDMITX_H_        /**< Prevent circular inclusions
                  *  by using protection macros */

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/

#include "xv_hdmitx_hw.h"
#include "xil_assert.h"
#include "xstatus.h"
#include "xdebug.h"
#include "xvidc.h"
#include "xv_hdmitx_vsif.h"
#include "xv_hdmic.h"

/************************** Constant Definitions *****************************/


/**************************** Type Definitions *******************************/

/** @name Handler Types
* @{
*/
/**
* These constants specify different types of handler and used to differentiate
* interrupt requests from peripheral.
*/
typedef enum {
    XV_HDMITX_HANDLER_CONNECT = 1,  // Handler for connect
    XV_HDMITX_HANDLER_TOGGLE,       // Handler for toggle
	XV_HDMITX_HANDLER_BRDGUNLOCK,   // Handler for bridge unlocked
    XV_HDMITX_HANDLER_VS,           // Handler for vsync
    XV_HDMITX_HANDLER_STREAM_DOWN,  // Handler for stream down
    XV_HDMITX_HANDLER_STREAM_UP     // Handler for stream up
} XV_HdmiTx_HandlerType;
/*@}*/

/** @name HDMI TX stream status
* @{
*/
typedef enum {
    XV_HDMITX_STATE_STREAM_DOWN,    // Stream down
    XV_HDMITX_STATE_STREAM_UP       // Stream up
} XV_HdmiTx_State;

/** @name HDMI TX audio format
* @{
*/
typedef enum {
    XV_HDMITX_AUDFMT_LPCM = 0,    // L-PCM
    XV_HDMITX_AUDFMT_HBR          // HBR
} XV_HdmiTx_AudioFormatType;

/**
* This typedef contains configuration information for the HDMI TX core.
* Each HDMI TX device should have a configuration structure associated.
*/
typedef struct {
    u16 DeviceId;       /**< DeviceId is the unique ID of the HDMI TX core */
    UINTPTR BaseAddress;    /**< BaseAddress is the physical
                        * base address of the core's registers */
	u32 AxiLiteClkFreq;
} XV_HdmiTx_Config;

/**
* This typedef contains audio stream specific data structure
*/
typedef struct {
    u8 Channels;            //< Video Identification code */
} XV_HdmiTx_AudioStream;

/**
* This typedef contains HDMI TX stream specific data structure.
*/
typedef struct {
    XVidC_VideoStream       Video;              /**< Video stream for HDMI TX */
    XV_HdmiTx_AudioStream   Audio;              /**< Audio stream for HDMI TX */
    u8                      Vic;                /**< Video Identification code
                                                flag  */
    u8                      IsHdmi;             /**< HDMI flag. 1 - HDMI Stream,
                                                0 - DVI Stream  */
    u8                      IsHdmi20;           /**< HDMI 2.0 flag  */
    u8                      IsScrambled;        /**< Scrambler flag
                                1 - scrambled data , 0 - non scrambled data */
    u8						OverrideScrambler; /**< Override scramble
													 flag */
    u32                     TMDSClock;          /**< TMDS clock */
    u8                      TMDSClockRatio;     /**< TMDS clock ration
                                0 - 1/10, 1 - 1/40 */
    u32                     PixelClk;           /**< Pixel Clock  */
    XV_HdmiTx_State         State;              /**< State */
    u8                      IsConnected;        /**< Connected flag.
                            This flag is set when the cable is connected  */
    u8                      SampleRate;         /**< Sample rate */
} XV_HdmiTx_Stream;

/**
* Callback type for Vsync event interrupt.
*
* @param    CallbackRef is a callback reference passed in by the upper
*       layer when setting the callback functions, and passed back to
*       the upper layer when the callback is invoked.
*
* @return   None.
*
* @note     None.
*
*/
typedef void (*XV_HdmiTx_Callback)(void *CallbackRef);

/**
* The XV_HdmiTx driver instance data. An instance must be allocated for each
* HDMI TX core in use.
*/
typedef struct {
    XV_HdmiTx_Config Config;    /**< Hardware Configuration */
    u32 IsReady;        /**< Core and the driver instance are initialized */

    /* Callbacks */
    XV_HdmiTx_Callback ConnectCallback;     /**< Callback for connect event
                                            interrupt */
    void *ConnectRef;                       /**< To be passed to the connect
                                            interrupt callback */
    u32 IsConnectCallbackSet;               /**< Set flag. This flag is set
                                to true when the callback has been registered */

    XV_HdmiTx_Callback ToggleCallback;     /**< Callback for toggle event
                                            interrupt */
    void *ToggleRef;                       /**< To be passed to the toggle
                                            interrupt callback */
    u32 IsToggleCallbackSet;               /**< Set flag. This flag is set
                                to true when the callback has been registered */

    XV_HdmiTx_Callback VsCallback;          /**< Callback for Vsync event
                                            interrupt */
    void *VsRef;                            /**< To be passed to the Vsync
                                        interrupt callback */
    u32 IsVsCallbackSet;                    /**< Set flag. This flag is set to
                                true when the callback has been registered */

    XV_HdmiTx_Callback BrdgUnlockedCallback;   /**< Callback for Bridge UnLocked
                                                  event interrupt */
    void *BrdgUnlockedRef;                  /**< To be passed to the Bridge
                                              Unlocked interrupt callback */
    u32 IsBrdgUnlockedCallbackSet;       /**< Set flag. This flag is set to
                                true when the callback has been registered */

    XV_HdmiTx_Callback StreamDownCallback;  /**< Callback for stream down
                                            callback */
    void *StreamDownRef;                    /**< To be passed to the stream
                                            down callback */
    u32 IsStreamDownCallbackSet;            /**< Set flag. This flag is set to
                                true when the callback has been registered */

    XV_HdmiTx_Callback StreamUpCallback;    /**< Callback for stream up
    callback */
    void *StreamUpRef;                      /**< To be passed to the stream up
    callback */
    u32 IsStreamUpCallbackSet;              /**< Set flag. This flag is set to
    true when the callback has been registered */

    /* Aux peripheral specific */
    XHdmiC_Aux Aux;                         /**< AUX peripheral information */

    /* HDMI TX stream */
    XV_HdmiTx_Stream Stream;                /**< HDMI TX stream information */
    u32 CpuClkFreq;                         /* CPU Clock frequency */

} XV_HdmiTx;

/***************** Macros (Inline Functions) Definitions *********************/

/*****************************************************************************/
/**
*
* This macro reads the TX version
*
* @param  InstancePtr is a pointer to the XV_HdmiTX core instance.
*
* @return None.
*
*
******************************************************************************/
#define XV_HdmiTx_GetVersion(InstancePtr) \
  XV_HdmiTx_ReadReg((InstancePtr)->Config.BaseAddress, \
  (XV_HDMITX_VER_VERSION_OFFSET))

/*****************************************************************************/
/**
*
* This macro asserts or releases the HDMI TX reset.
*
* @param    InstancePtr is a pointer to the XV_HdmiTx core instance.
* @param    Reset specifies TRUE/FALSE value to either assert or
*       release HDMI TX reset.
*
* @return   None.
*
* @note     The reset output of the PIO is inverted. When the system is
*       in reset, the PIO output is cleared and this will reset the
*       HDMI RX. Therefore, clearing the PIO reset output will assert
*       the HDMI link and video reset.
*       C-style signature:
*       void XV_HdmiTx_Reset(XV_HdmiTx *InstancePtr, u8 Reset)
*
******************************************************************************/
#define XV_HdmiTx_Reset(InstancePtr, Reset) \
{ \
    if (Reset) { \
        XV_HdmiTx_WriteReg((InstancePtr)->Config.BaseAddress, \
        (XV_HDMITX_PIO_OUT_CLR_OFFSET), (XV_HDMITX_PIO_OUT_RST_MASK)); \
    } \
    else { \
        XV_HdmiTx_WriteReg((InstancePtr)->Config.BaseAddress, \
        (XV_HDMITX_PIO_OUT_SET_OFFSET), (XV_HDMITX_PIO_OUT_RST_MASK)); \
    } \
}

/*****************************************************************************/
/**
*
* This macro controls the HDMI TX Scrambler.
*
* @param    InstancePtr is a pointer to the XV_HdmiTx core instance.
* @param    SetClr specifies TRUE/FALSE value to either set ON or clear
*       Scrambler.
*
* @return   None.
*
* @note     C-style signature:
*       void XV_HdmiTx_SetScrambler(XV_HdmiTx *InstancePtr, u8 SetClr)
*
******************************************************************************/
#define XV_HdmiTx_SetScrambler(InstancePtr, SetClr) \
{ \
    if (SetClr) { \
        XV_HdmiTx_WriteReg((InstancePtr)->Config.BaseAddress, \
        (XV_HDMITX_PIO_OUT_SET_OFFSET), (XV_HDMITX_PIO_OUT_SCRM_MASK)); \
        (InstancePtr)->Stream.IsScrambled = (TRUE); \
    } \
    else { \
        XV_HdmiTx_WriteReg((InstancePtr)->Config.BaseAddress, \
        (XV_HDMITX_PIO_OUT_CLR_OFFSET), (XV_HDMITX_PIO_OUT_SCRM_MASK)); \
        (InstancePtr)->Stream.IsScrambled = (FALSE); \
    } \
}

/*****************************************************************************/
/**
*
* This macro controls the YUV420 mode for video bridge.
*
* @param	InstancePtr is a pointer to the XHdmi_Tx core instance.
* @param	SetClr specifies TRUE/FALSE value to either enable or disable the
*		YUV 420 Support.
*
* @return	None.
*
* @note		C-style signature:
*		void XV_HdmiTx_Bridge_yuv420(XV_HdmiTx *InstancePtr, u8 SetClr)
*
******************************************************************************/
#define XV_HdmiTx_Bridge_yuv420(InstancePtr, SetClr) \
{ \
	if (SetClr) { \
		XV_HdmiTx_WriteReg((InstancePtr)->Config.BaseAddress, \
		                   (XV_HDMITX_PIO_OUT_SET_OFFSET), \
						   (XV_HDMITX_PIO_OUT_BRIDGE_YUV420_MASK)); \
	} \
	else { \
		XV_HdmiTx_WriteReg((InstancePtr)->Config.BaseAddress, \
		                   (XV_HDMITX_PIO_OUT_CLR_OFFSET), \
						   (XV_HDMITX_PIO_OUT_BRIDGE_YUV420_MASK)); \
	} \
}

/*****************************************************************************/
/**
*
* This macro controls the Pixel Repeat mode for video bridge.
*
* @param	InstancePtr is a pointer to the XHdmi_Tx core instance.
* @param	SetClr specifies TRUE/FALSE value to either enable or disable the
*		Pixel Repitition Support.
*
* @return	None.
*
* @note		C-style signature:
*		void XV_HdmiTx_Bridge_pixel(XV_HdmiTx *InstancePtr, u8 SetClr)
*
******************************************************************************/
#define XV_HdmiTx_Bridge_pixel(InstancePtr, SetClr) \
{ \
	if (SetClr) { \
		XV_HdmiTx_WriteReg((InstancePtr)->Config.BaseAddress, \
		                   (XV_HDMITX_PIO_OUT_SET_OFFSET), \
						   (XV_HDMITX_PIO_OUT_BRIDGE_PIXEL_MASK)); \
	} \
	else { \
		XV_HdmiTx_WriteReg((InstancePtr)->Config.BaseAddress, \
		                   (XV_HDMITX_PIO_OUT_CLR_OFFSET), \
						   (XV_HDMITX_PIO_OUT_BRIDGE_PIXEL_MASK)); \
	} \
}

/*****************************************************************************/
/**
*
* This macro enables the HDMI TX PIO peripheral.
*
* @param    InstancePtr is a pointer to the XV_HdmiTx core instance.
*
* @return   None.
*
* @note     C-style signature:
*       void XV_HdmiTx_PioEnable(XV_HdmiTx *InstancePtr)
*
******************************************************************************/
#define XV_HdmiTx_PioEnable(InstancePtr) \
    XV_HdmiTx_WriteReg((InstancePtr)->Config.BaseAddress, \
    (XV_HDMITX_PIO_CTRL_SET_OFFSET), (XV_HDMITX_PIO_CTRL_RUN_MASK))

/*****************************************************************************/
/**
*
* This macro disables the HDMI TX PIO peripheral.
*
* @param    InstancePtr is a pointer to the XV_HdmiTx core instance.
*
* @return   None.
*
* @note     C-style signature:
*       void XV_HdmiTx_PioDisable(XV_HdmiTx *InstancePtr)
*
******************************************************************************/
#define XV_HdmiTx_PioDisable(InstancePtr) \
    XV_HdmiTx_WriteReg((InstancePtr)->Config.BaseAddress, \
    (XV_HDMITX_PIO_CTRL_CLR_OFFSET), (XV_HDMITX_PIO_CTRL_RUN_MASK))

/*****************************************************************************/
/**
*
* This macro enables interrupt in the HDMI TX PIO peripheral.
*
* @param    InstancePtr is a pointer to the XV_HdmiTx core instance.
*
* @return   None.
*
* @note     C-style signature:
*       void XV_HdmiTx_PioIntrEnable(XV_HdmiTx *InstancePtr)
*
******************************************************************************/
#define XV_HdmiTx_PioIntrEnable(InstancePtr) \
    XV_HdmiTx_WriteReg((InstancePtr)->Config.BaseAddress, \
    (XV_HDMITX_PIO_CTRL_SET_OFFSET), (XV_HDMITX_PIO_CTRL_IE_MASK))

/*****************************************************************************/
/**
*
* This macro disables interrupt in the HDMI TX PIO peripheral.
*
* @param    InstancePtr is a pointer to the XV_HdmiTx core instance.
*
* @return   None.
*
* @note     C-style signature:
*       void XV_HdmiTx_PioIntrDisable(XV_HdmiTx *InstancePtr)
*
******************************************************************************/
#define XV_HdmiTx_PioIntrDisable(InstancePtr) \
    XV_HdmiTx_WriteReg((InstancePtr)->Config.BaseAddress, \
    (XV_HDMITX_PIO_CTRL_CLR_OFFSET), (XV_HDMITX_PIO_CTRL_IE_MASK))

/*****************************************************************************/
/**
*
* This macro clears HDMI TX PIO interrupt.
*
* @param    InstancePtr is a pointer to the XV_HdmiTx core instance.
*
* @return   None.
*
* @note     C-style signature:
*       void XV_HdmiTx_PioIntrClear(XV_HdmiTx *InstancePtr)
*
******************************************************************************/
#define XV_HdmiTx_PioIntrClear(InstancePtr) \
    XV_HdmiTx_WriteReg((InstancePtr)->Config.BaseAddress, \
    (XV_HDMITX_PIO_STA_OFFSET), (XV_HDMITX_PIO_STA_IRQ_MASK))

/*****************************************************************************/
/**
*
* This macro enables the HDMI TX Display Data Channel (DDC) peripheral.
*
* @param    InstancePtr is a pointer to the XV_HdmiTx core instance.
*
* @return   None.
*
* @note     C-style signature:
*       void XV_HdmiTx_DdcEnable(XV_HdmiTx *InstancePtr)
*
******************************************************************************/
#define XV_HdmiTx_DdcEnable(InstancePtr) \
    XV_HdmiTx_WriteReg((InstancePtr)->Config.BaseAddress, \
    (XV_HDMITX_DDC_CTRL_SET_OFFSET), (XV_HDMITX_DDC_CTRL_RUN_MASK))

/*****************************************************************************/
/**
*
* This macro disables the HDMI TX Display Data Channel (DDC) peripheral.
*
* @param    InstancePtr is a pointer to the XV_HdmiTx core instance.
*
* @return   None.
*
* @note     C-style signature:
*       void XV_HdmiTx_DdcDisable(XV_HdmiTx *InstancePtr)
*
******************************************************************************/
#define XV_HdmiTx_DdcDisable(InstancePtr) \
    XV_HdmiTx_WriteReg((InstancePtr)->Config.BaseAddress, \
    (XV_HDMITX_DDC_CTRL_CLR_OFFSET), (XV_HDMITX_DDC_CTRL_RUN_MASK))

/*****************************************************************************/
/**
*
* This macro enables interrupt in the HDMI TX DDC peripheral.
*
* @param    InstancePtr is a pointer to the XV_HdmiTx core instance.
*
* @return   None.
*
* @note     C-style signature:
*       void XV_HdmiTx_DdcIntrEnable(XV_HdmiTx *InstancePtr)
*
******************************************************************************/
#define XV_HdmiTx_DdcIntrEnable(InstancePtr) \
    XV_HdmiTx_WriteReg((InstancePtr)->Config.BaseAddress, \
    (XV_HDMITX_DDC_CTRL_SET_OFFSET), (XV_HDMITX_DDC_CTRL_IE_MASK))

/*****************************************************************************/
/**
*
* This macro disables interrupt in the HDMI TX DDC peripheral.
*
* @param    InstancePtr is a pointer to the XV_HdmiTx core instance.
*
* @return   None.
*
* @note     C-style signature:
*       void XV_HdmiTx_DdcIntrDisable(XV_HdmiTx *InstancePtr)
*
******************************************************************************/
#define XV_HdmiTx_DdcIntrDisable(InstancePtr) \
    XV_HdmiTx_WriteReg((InstancePtr)->Config.BaseAddress, \
    (XV_HDMITX_DDC_CTRL_CLR_OFFSET), (XV_HDMITX_DDC_CTRL_IE_MASK))

/*****************************************************************************/
/**
*
* This macro clears HDMI TX DDC interrupt.
*
* @param    InstancePtr is a pointer to the XV_HdmiTx core instance.
*
* @return   None.
*
* @note     C-style signature:
*       void XV_HdmiTx_DdcIntrClear(XV_HdmiTx *InstancePtr)
*
******************************************************************************/
#define XV_HdmiTx_DdcIntrClear(InstancePtr) \
    XV_HdmiTx_WriteReg((InstancePtr)->Config.BaseAddress, \
    (XV_HDMITX_DDC_STA_OFFSET), (XV_HDMITX_DDC_STA_IRQ_MASK))

/*****************************************************************************/
/**
*
* This macro enables the HDMI TX Auxiliary (AUX) peripheral.
*
* @param    InstancePtr is a pointer to the XV_HdmiTx core instance.
*
* @return   None.
*
* @note     C-style signature:
*       void XV_HdmiTx_AuxEnable(XV_HdmiTx *InstancePtr)
*
******************************************************************************/
#define XV_HdmiTx_AuxEnable(InstancePtr) \
{ \
    if ((InstancePtr)->Stream.IsHdmi) { \
        XV_HdmiTx_WriteReg((InstancePtr)->Config.BaseAddress, \
        (XV_HDMITX_AUX_CTRL_SET_OFFSET), (XV_HDMITX_AUX_CTRL_RUN_MASK)); \
    } \
}

/*****************************************************************************/
/**
*
* This macro disables the HDMI TX Auxiliary (AUX) peripheral.
*
* @param    InstancePtr is a pointer to the XV_HdmiTx core instance.
*
* @return   None.
*
* @note     C-style signature:
*       void XV_HdmiTx_AuxDisable(XV_HdmiTx *InstancePtr)
*
******************************************************************************/
#define XV_HdmiTx_AuxDisable(InstancePtr) \
    XV_HdmiTx_WriteReg((InstancePtr)->Config.BaseAddress, \
    (XV_HDMITX_AUX_CTRL_CLR_OFFSET), (XV_HDMITX_AUX_CTRL_RUN_MASK))

/*****************************************************************************/
/**
*
* This macro enables interrupt in the HDMI TX AUX peripheral.
*
* @param    InstancePtr is a pointer to the XV_HdmiTx core instance.
*
* @return   None.
*
* @note     C-style signature:
*       void XV_HdmiTx_AuxIntrEnable(XV_HdmiTx *InstancePtr)
*
******************************************************************************/
#define XV_HdmiTx_AuxIntrEnable(InstancePtr) \
    XV_HdmiTx_WriteReg((InstancePtr)->Config.BaseAddress, \
    (XV_HDMITX_AUX_CTRL_SET_OFFSET), (XV_HDMITX_AUX_CTRL_IE_MASK))

/*****************************************************************************/
/**
*
* This macro disables interrupt in the HDMI TX AUX peripheral.
*
* @param    InstancePtr is a pointer to the XV_HdmiTx core instance.
*
* @return   None.
*
* @note     C-style signature:
*       void XV_HdmiTx_AuxIntrDisable(XV_HdmiTx *InstancePtr)
*
******************************************************************************/
#define XV_HdmiTx_AuxIntrDisable(InstancePtr) \
    XV_HdmiTx_WriteReg((InstancePtr)->Config.BaseAddress, \
    (XV_HDMITX_AUX_CTRL_CLR_OFFSET), (XV_HDMITX_AUX_CTRL_IE_MASK))

/*****************************************************************************/
/**
*
* This macro enables audio in HDMI TX core.
*
* @param    InstancePtr is a pointer to the XV_HdmiTx core instance.
*
* @return   None.
*
* @note     C-style signature:
*       void XV_HdmiTx_AudioEnable(XV_HdmiTx *InstancePtr)
*
******************************************************************************/
#define XV_HdmiTx_AudioEnable(InstancePtr) \
{ \
    if ((InstancePtr)->Stream.IsHdmi) { \
        XV_HdmiTx_WriteReg((InstancePtr)->Config.BaseAddress, \
        (XV_HDMITX_AUD_CTRL_SET_OFFSET), (XV_HDMITX_AUD_CTRL_RUN_MASK)); \
    } \
}

/*****************************************************************************/
/**
*
* This macro disables audio in HDMI TX core.
*
* @param    InstancePtr is a pointer to the XV_HdmiTx core instance.
*
* @return   None.
*
* @note     C-style signature:
*       void XV_HdmiTx_AudioDisable(XV_HdmiTx *InstancePtr)
*
******************************************************************************/
#define XV_HdmiTx_AudioDisable(InstancePtr) \
    XV_HdmiTx_WriteReg((InstancePtr)->Config.BaseAddress, \
    (XV_HDMITX_AUD_CTRL_CLR_OFFSET), (XV_HDMITX_AUD_CTRL_RUN_MASK))

/*****************************************************************************/
/**
*
* This macro unmutes audio in HDMI TX core.
*
* @param    InstancePtr is a pointer to the XV_HdmiTx core instance.
*
* @return   None.
*
* @note     C-style signature:
*       void XV_HdmiTx_AudioEnable(XV_HdmiTx *InstancePtr)
*
******************************************************************************/
#define XV_HdmiTx_AudioUnmute(InstancePtr) \
    XV_HdmiTx_WriteReg((InstancePtr)->Config.BaseAddress, \
    (XV_HDMITX_AUD_CTRL_SET_OFFSET), (XV_HDMITX_AUD_CTRL_RUN_MASK))

/*****************************************************************************/
/**
*
* This macro mutes audio in HDMI TX core.
*
* @param    InstancePtr is a pointer to the XV_HdmiTx core instance.
*
* @return   None.
*
* @note     C-style signature:
*       void XV_HdmiTx_AudioDisable(XV_HdmiTx *InstancePtr)
*
******************************************************************************/
#define XV_HdmiTx_AudioMute(InstancePtr) \
    XV_HdmiTx_WriteReg((InstancePtr)->Config.BaseAddress, \
    (XV_HDMITX_AUD_CTRL_CLR_OFFSET), (XV_HDMITX_AUD_CTRL_RUN_MASK))

/*****************************************************************************/
/**
*
* This macro sets the mode bit.
*
* @param    InstancePtr is a pointer to the XV_HdmiTx core instance.
*
* @return   None.
*
* @note     C-style signature:
*       void XV_HdmiTx_SetMode(XV_HdmiTx *InstancePtr)
*
******************************************************************************/
#define XV_HdmiTx_SetMode(InstancePtr) \
    XV_HdmiTx_WriteReg((InstancePtr)->Config.BaseAddress, \
    (XV_HDMITX_PIO_OUT_SET_OFFSET), (XV_HDMITX_PIO_OUT_MODE_MASK))

/*****************************************************************************/
/**
*
* This macro clears the mode bit.
*
* @param    InstancePtr is a pointer to the XV_HdmiTx core instance.
*
* @return   None.
*
* @note     C-style signature:
*       void XV_HdmiTx_ClearMode(XV_HdmiTx *InstancePtr)
*
******************************************************************************/
#define XV_HdmiTx_ClearMode(InstancePtr) \
    XV_HdmiTx_WriteReg((InstancePtr)->Config.BaseAddress, \
    (XV_HDMITX_PIO_OUT_CLR_OFFSET), (XV_HDMITX_PIO_OUT_MODE_MASK))

/*****************************************************************************/
/**
*
* This macro provides the current mode.
*
* @param    InstancePtr is a pointer to the XV_HdmiTx core instance.
*
* @return   Current mode.
*       0 = DVI
*       1 = HDMI
*
* @note     C-style signature:
*       u8 XV_HdmiTx_GetMode(XV_HdmiTx *InstancePtr)
*
******************************************************************************/
#define XV_HdmiTx_GetMode(InstancePtr) \
    (XV_HdmiTx_ReadReg((InstancePtr)->Config.BaseAddress, \
    (XV_HDMITX_PIO_OUT_OFFSET)) & (XV_HDMITX_PIO_OUT_MODE_MASK))

/*****************************************************************************/
/**
*
* This macro provides the current sample rate.
*
* @param    InstancePtr is a pointer to the XV_HdmiTx core instance.
*
* @return   Sample rate
*
* @note     C-style signature:
*       u8 XV_HdmiTx_GetMode(XV_HdmiTx *InstancePtr)
*
******************************************************************************/
#define XV_HdmiTx_GetSampleRate(InstancePtr) \
    (InstancePtr)->Stream.SampleRate

/*****************************************************************************/
/**
*
* This macro provides the active audio channels.
*
* @param    InstancePtr is a pointer to the XV_HdmiTx core instance.
*
* @return   Audio channels
*
*
******************************************************************************/
#define XV_HdmiTx_GetAudioChannels(InstancePtr) \
    (InstancePtr)->Stream.Audio.Channels

/*****************************************************************************/
/**
*
* This macro provides the current pixel packing phase.
*
* @param    InstancePtr is a pointer to the XV_HdmiTx core instance.
*
* @return   Pixel packing phase.
*
*
******************************************************************************/
#define XV_HdmiTx_GetPixelPackingPhase(InstancePtr) \
    ( ( (XV_HdmiTx_ReadReg( (InstancePtr)->Config.BaseAddress, \
    (XV_HDMITX_PIO_IN_OFFSET) ) ) >> (XV_HDMITX_PIO_IN_PPP_SHIFT)) \
    & (XV_HDMITX_PIO_IN_PPP_MASK))

/*****************************************************************************/
/**
*
* This macro disables video mask in HDMI TX core.
*
* @param    InstancePtr is a pointer to the XV_HdmiTx core instance.
*
* @return   None.
*
* @note     C-style signature:
*       void XV_HdmiTx_MaskDisable(XV_HdmiTx *InstancePtr)
*
******************************************************************************/
#define XV_HdmiTx_MaskDisable(InstancePtr) \
{ \
        XV_HdmiTx_WriteReg((InstancePtr)->Config.BaseAddress, \
        (XV_HDMITX_MASK_CTRL_CLR_OFFSET), (XV_HDMITX_MASK_CTRL_RUN_MASK)); \
}

/*****************************************************************************/
/**
*
* This macro enables video mask in HDMI TX core.
*
* @param    InstancePtr is a pointer to the XV_HdmiTx core instance.
*
* @return   None.
*
* @note     C-style signature:
*       void XV_HdmiTx_MaskEnable(XV_HdmiTx *InstancePtr)
*
******************************************************************************/
#define XV_HdmiTx_MaskEnable(InstancePtr) \
{ \
        XV_HdmiTx_WriteReg((InstancePtr)->Config.BaseAddress, \
        (XV_HDMITX_MASK_CTRL_SET_OFFSET), (XV_HDMITX_MASK_CTRL_RUN_MASK)); \
}

/*****************************************************************************/
/**
*
* This macro enables or disables the noise in the video mask.
*
* @param	InstancePtr is a pointer to the XHdmi_Tx core instance.
* @param	SetClr specifies TRUE/FALSE value to either enable or disable the
*		Noise.
*
* @return	None.
*
* @note		C-style signature:
*		void XV_HdmiTx_MaskNoise(XV_HdmiTx *InstancePtr, u8 SetClr)
*
******************************************************************************/
#define XV_HdmiTx_MaskNoise(InstancePtr, SetClr) \
{ \
	if (SetClr) { \
		XV_HdmiTx_WriteReg((InstancePtr)->Config.BaseAddress, \
		                   (XV_HDMITX_MASK_CTRL_SET_OFFSET), \
						   (XV_HDMITX_MASK_CTRL_NOISE_MASK)); \
	} \
	else { \
		XV_HdmiTx_WriteReg((InstancePtr)->Config.BaseAddress, \
		                   (XV_HDMITX_MASK_CTRL_CLR_OFFSET), \
						   (XV_HDMITX_MASK_CTRL_NOISE_MASK)); \
	} \
}

/*****************************************************************************/
/**
*
* This macro sets the red component value in the video mask.
*
* @param	InstancePtr is a pointer to the XHdmi_Tx core instance.
* @param	Value
*
* @return	None.
*
* @note		C-style signature:
*		void XV_HdmiTx_MaskSetRed(XV_HdmiTx *InstancePtr, u16 Value)
*
******************************************************************************/
#define XV_HdmiTx_MaskSetRed(InstancePtr, Value) \
{ \
		XV_HdmiTx_WriteReg((InstancePtr)->Config.BaseAddress, \
		                   (XV_HDMITX_MASK_RED_OFFSET), \
						   (Value)); \
}

/*****************************************************************************/
/**
*
* This macro sets the green component value in the video mask.
*
* @param	InstancePtr is a pointer to the XHdmi_Tx core instance.
* @param	Value
*
* @return	None.
*
* @note		C-style signature:
*		void XV_HdmiTx_MaskSetGreen(XV_HdmiTx *InstancePtr, u16 Value)
*
******************************************************************************/
#define XV_HdmiTx_MaskSetGreen(InstancePtr, Value) \
{ \
		XV_HdmiTx_WriteReg((InstancePtr)->Config.BaseAddress, \
		                   (XV_HDMITX_MASK_GREEN_OFFSET), \
						   (Value)); \
}

/*****************************************************************************/
/**
*
* This macro sets the blue component value in the video mask.
*
* @param	InstancePtr is a pointer to the XHdmi_Tx core instance.
* @param	Value
*
* @return	None.
*
* @note		C-style signature:
*		void XV_HdmiTx_MaskSetBlue(XV_HdmiTx *InstancePtr, u16 Value)
*
******************************************************************************/
#define XV_HdmiTx_MaskSetBlue(InstancePtr, Value) \
{ \
		XV_HdmiTx_WriteReg((InstancePtr)->Config.BaseAddress, \
		                   (XV_HDMITX_MASK_BLUE_OFFSET), \
						   (Value)); \
}

/*****************************************************************************/
/**
*
* This macro provides the current video mask mode.
*
* @param    InstancePtr is a pointer to the XV_HdmiTx core instance.
*
* @return   Current mode.
*       0 = Video masking is disabled
*       1 = Video masking is enabled
*
* @note     C-style signature:
*       u8 XV_HdmiTx_IsMasked(XV_HdmiTx *InstancePtr)
*
******************************************************************************/
#define XV_HdmiTx_IsMasked(InstancePtr) \
    XV_HdmiTx_ReadReg((InstancePtr)->Config.BaseAddress, \
    (XV_HDMITX_MASK_CTRL_OFFSET)) & (XV_HDMITX_MASK_CTRL_RUN_MASK)

/************************** Function Prototypes ******************************/

/* Initialization function in xv_hdmitx_sinit.c */
XV_HdmiTx_Config *XV_HdmiTx_LookupConfig(u16 DeviceId);

/* Initialization and control functions in xv_hdmitx.c */
int XV_HdmiTx_CfgInitialize(XV_HdmiTx *InstancePtr,
    XV_HdmiTx_Config *CfgPtr,
    UINTPTR EffectiveAddr);
void XV_HdmiTx_SetHdmiMode(XV_HdmiTx *InstancePtr);
void XV_HdmiTx_SetDviMode(XV_HdmiTx *InstancePtr);
void XV_HdmiTx_Clear(XV_HdmiTx *InstancePtr);
u8 XV_HdmiTx_GetVic(XVidC_VideoMode VideoMode);
XVidC_VideoMode XV_HdmiTx_GetVideoModeFromVic(u8 Vic);
u32 XV_HdmiTx_SetStream(XV_HdmiTx *InstancePtr,
    XVidC_VideoMode VideoMode,
    XVidC_ColorFormat ColorFormat,
    XVidC_ColorDepth Bpc,
    XVidC_PixelsPerClock Ppc,
    XVidC_3DInfo *Info3D);

u32 XV_HdmiTx_GetTmdsClk (XV_HdmiTx *InstancePtr,
    XVidC_VideoMode VideoMode,
    XVidC_ColorFormat ColorFormat,
    XVidC_ColorDepth Bpc);

void XV_HdmiTx_INT_VRST(XV_HdmiTx *InstancePtr, u8 Reset);
void XV_HdmiTx_INT_LRST(XV_HdmiTx *InstancePtr, u8 Reset);
void XV_HdmiTx_EXT_VRST(XV_HdmiTx *InstancePtr, u8 Reset);
void XV_HdmiTx_EXT_SYSRST(XV_HdmiTx *InstancePtr, u8 Reset);
void XV_HdmiTx_SetGcpAvmuteBit(XV_HdmiTx *InstancePtr);
void XV_HdmiTx_ClearGcpAvmuteBit(XV_HdmiTx *InstancePtr);
void XV_HdmiTx_SetGcpClearAvmuteBit(XV_HdmiTx *InstancePtr);
void XV_HdmiTx_ClearGcpClearAvmuteBit(XV_HdmiTx *InstancePtr);
void XV_HdmiTx_SetPixelRate(XV_HdmiTx *InstancePtr);
void XV_HdmiTx_SetSampleRate(XV_HdmiTx *InstancePtr, u8 SampleRate);
void XV_HdmiTx_SetColorFormat(XV_HdmiTx *InstancePtr);
void XV_HdmiTx_SetColorDepth(XV_HdmiTx *InstancePtr);
int XV_HdmiTx_IsStreamScrambled(XV_HdmiTx *InstancePtr);
int XV_HdmiTx_IsStreamConnected(XV_HdmiTx *InstancePtr);
void XV_HdmiTx_SetAxiClkFreq(XV_HdmiTx *InstancePtr, u32 ClkFreq);
void XV_HdmiTx_DdcInit(XV_HdmiTx *InstancePtr, u32 Frequency);
int XV_HdmiTx_DdcWrite(XV_HdmiTx *InstancePtr, u8 Slave, u16 Length,
    u8 *Buffer, u8 Stop);
int XV_HdmiTx_DdcRead(XV_HdmiTx *InstancePtr, u8 Slave, u16 Length,
    u8 *Buffer, u8 Stop);
u32 XV_HdmiTx_AuxSend(XV_HdmiTx *InstancePtr);
int XV_HdmiTx_Scrambler(XV_HdmiTx *InstancePtr);
int XV_HdmiTx_ClockRatio(XV_HdmiTx *InstancePtr);
int XV_HdmiTx_DetectHdmi20(XV_HdmiTx *InstancePtr);
void XV_HdmiTx_ShowSCDC(XV_HdmiTx *InstancePtr);
void XV_HdmiTx_DebugInfo(XV_HdmiTx *InstancePtr);
int XV_HdmiTx_SetAudioChannels(XV_HdmiTx *InstancePtr, u8 Value);
int XV_HdmiTx_SetAudioFormat(XV_HdmiTx *InstancePtr, XV_HdmiTx_AudioFormatType Value);
XV_HdmiTx_AudioFormatType XV_HdmiTx_GetAudioFormat(XV_HdmiTx *InstancePtr);
/* Self test function in xv_hdmitx_selftest.c */
int XV_HdmiTx_SelfTest(XV_HdmiTx *InstancePtr);

/* Interrupt related functions in xv_hdmitx_intr.c */
void XV_HdmiTx_IntrHandler(void *InstancePtr);
int XV_HdmiTx_SetCallback(XV_HdmiTx *InstancePtr, u32 HandlerType,
    void *CallbackFunc, void *CallbackRef);


/* Vendor Specific Infomation related functions in xv_hdmitx_vsif.c */
int XV_HdmiTx_VSIF_GeneratePacket(XV_HdmiTx_VSIF  *VSIFPtr,
		XHdmiC_Aux *AuxPtr);
void XV_HdmiTx_VSIF_DisplayInfo(XV_HdmiTx_VSIF  *VSIFPtr);
char* XV_HdmiTx_VSIF_3DStructToString(XV_HdmiTx_3D_Struct_Field Item);
char* XV_HdmiTx_VSIF_3DSampMethodToString(XV_HdmiTx_3D_Sampling_Method Item);
char* XV_HdmiTx_VSIF_3DSampPosToString(XV_HdmiTx_3D_Sampling_Position Item);

/************************** Variable Declarations ****************************/

/************************** Variable Declarations ****************************/

#ifdef __cplusplus
}
#endif

#endif /* End of protection macro */
