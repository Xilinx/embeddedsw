/*******************************************************************************
 *
 * Copyright (C) 2017 Xilinx, Inc.  All rights reserved.
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
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 *
 *
*******************************************************************************/
/******************************************************************************/
/**
 *
 * @file xdppsu.h
 *
 * The Xilinx DisplayPort transmitter (DPTX_PS) driver. This driver supports the
 * Xilinx DisplayPort PS core in TX mode. This driver follows the
 * DisplayPort 1.2a specification.
 *
 * The Xilinx DisplayPort soft IP supports the following features:
 *	- 1, 2 lanes.
 *	- A link rate of 1.62, 2.70, or 5.40Gbps per lane.
 *	- 1, 2, or 4 pixel-wide video interfaces.
 *	- RGB and YCbCr color space.
 *	- Up to 12 bits per component.
 *	- Up to 4Kx2K monitor resolution.
 *	- Auto lane rate and width negotiation.
 *	- I2C over a 1Mb/s AUX channel.
 *	- Secondary channel audio support (2 channels).
 *
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- -----------------------------------------------
 * 1.0   aad  05/17/14 Initial release.
 * </pre>
 *
*******************************************************************************/

#ifndef XDPPSU_H_
/* Prevent circular inclusions by using protection macros. */
#define XDPPSU_H_

/******************************* Include Files ********************************/

#include "xdppsu_hw.h"
#include "xil_types.h"
#include "xvidc.h"
/****************************** Type Definitions ******************************/
/**
 * This typedef contains configuration information for the DisplayPort TX core.
 */

typedef struct {

	u16 DeviceId;		/**< Device instance ID. */
	u32 BaseAddr;		/**< The base address of the core instance. */
} XDpPsu_Config;
/**
 * This typedef contains configuration information about the RX device.
 */
typedef struct {
	u8 DpcdRxCapsField[16];	/**< The first 16 bytes of the raw capabilities
					field of the RX device's DisplayPort
					Configuration Data (DPCD). */
	u8 LaneStatusAdjReqs[6];/**< This is a raw read of the RX device's
					status registers. The first 4 bytes
					correspond to the lane status associated
					with clock recovery, channel
					equalization, symbol lock, and interlane
					alignment. The remaining 2 bytes
					represent the pre-emphasis and voltage
					swing level adjustments requested by the
					RX device. */
	u8 SinkCount;		/**< The total number of downstream sink devices
					inside of the RX and downstream all
					devices. */
	u8 DevServiceIrqVec;	/**< Holds the value of the device service IRQ
					vector register. */
} XDpPsu_SinkConfig;

/**
 * This typedef contains configuration information about the main link settings.
 */
typedef struct {
	u8 LaneCount;			/**< The current lane count of the main
						link. */
	u8 LinkRate;			/**< The current link rate of the main
						link. */
	u8 ScramblerEn;			/**< Symbol scrambling is currently in
						use over the main link. */
	u8 EnhancedFramingMode;		/**< Enhanced frame mode is currently in
						use over the main link. */
	u8 DownspreadControl;		/**< Downspread control is currently in
						use over the main link. */
	u8 MaxLaneCount;		/**< The maximum lane count of the main
						link. */
	u8 MaxLinkRate;			/**< The maximum link rate of the main
						link. */
	u8 SupportEnhancedFramingMode;	/**< Enhanced frame mode is supported by
						the RX device. */
	u8 SupportDownspreadControl;	/**< Downspread control is supported by
						the RX device. */
	u8 VsLevel;			/**< The current voltage swing level for
						each lane. */
	u8 PeLevel;			/**< The current pre-emphasis/cursor
						level for each lane. */
	u8 Pattern;			/**< The current pattern currently in
						use over the main link. */
} XDpPsu_LinkConfig;

/**
 * This typedef contains the color encoding schemes that are supported
 */
typedef enum {
	XDPPSU_CENC_RGB = 0,		/**< RGB Color Encoding. */
	XDPPSU_CENC_XVYCC_422_BT601,	/**< XVYCC 422 Color Encoding BT601
						standard */
	XDPPSU_CENC_XVYCC_422_BT709,	/**< XVYCC 422 Color Encoding BT709
						standard */
	XDPPSU_CENC_XVYCC_444_BT601,	/**< XVYCC 444 Color Encoding BT601
						standard */
	XDPPSU_CENC_XVYCC_444_BT709,	/**< XVYCC 444 Color Encoding BT709
						standard */
	XDPPSU_CENC_YCBCR_422_BT601,	/**< YCbCr 422 Color Encoding BT601
						standard */
	XDPPSU_CENC_YCBCR_422_BT709,	/**< YCbCr 422 Color Encoding BT709
						standard */
	XDPPSU_CENC_YCBCR_444_BT601,	/**< XVYCC 444 Color Encoding BT601
						standard */
	XDPPSU_CENC_YCBCR_444_BT709,	/**< XVYCC 444 Color Encoding BT709
						standard */
	XDPPSU_CENC_YONLY,		/**< XVYCC 422 Color Encoding BT709
						standard */

} XDpPsu_ColorEncoding;

/**
 * This typedef contains the main stream attributes which determine how the
 * video will be displayed.
 */
typedef struct {
	XVidC_VideoTimingMode Vtm;	/**< The video timing. */
	u32 PixelClockHz;		/**< The pixel clock of the stream (in
						Hz). */
	u32 HStart;			/**< Horizontal blank start (in
						pixels). */
	u32 VStart;			/**< Vertical blank start (in lines). */
	u32 Misc0;			/**< Miscellaneous stream attributes 0
						as specified by the DisplayPort
						1.2 specification. */
	u32 Misc1;			/**< Miscellaneous stream attributes 1
						as specified by the DisplayPort
						1.2 specification. */
	u32 NVid;			/**< N value for the video stream. */
	u32 UserPixelWidth;		/**< The width of the user data input
						port. */
	u32 DataPerLane;		/**< Used to translate the number of
						pixels per line to the native
						internal 16-bit datapath. */
	u32 AvgBytesPerTU;		/**< Average number of bytes per
						transfer unit, scaled up by a
						factor of 1000. */
	u32 TransferUnitSize;		/**< Size of the transfer unit in the
						framing logic. In MST mode, this
						is also the number of time slots
						that are alloted in the payload
						ID table. */
	u32 InitWait;			/**< Number of initial wait cycles at
						the start of a new line by
						the framing logic. */
	u32 BitsPerColor;		/**< Number of bits per color
						component. */
	u8 ComponentFormat;		/**< The component format currently in
						use by the video stream. */
	u8 DynamicRange;		/**< The dynamic range currently in use
						by the video stream. */
	u8 YCbCrColorimetry;		/**< The YCbCr colorimetry currently in
						use by the video stream. */
	u8 SynchronousClockMode;	/**< Synchronous clock mode is currently
						in use by the video stream. */
} XDpPsu_MainStreamAttributes;

/**
 * This typedef describes some board characteristics information that affects
 * link training.
 */
typedef struct {
	u8 TxVsLevels[4];	/**< The voltage swing levels to be used by the
					DisplayPort TX. */
	u8 TxPeLevels[4];	/**< The pre-emphasis/cursor level to be used by
					the DisplayPort TX. */
} XDpPsu_BoardChar;

/******************************************************************************/
/**
 * Callback type which represents the handler for a Hot-Plug-Detect (HPD) event
 * interrupt.
 *
 * @param	InstancePtr is a pointer to the XDpPsu instance.
 *
 * @note	None.
 *
*******************************************************************************/
typedef void (*XDpPsu_HpdEventHandler)(void *InstancePtr);

/******************************************************************************/
/**
 * Callback type which represents the handler for a Hot-Plug-Detect (HPD) pulse
 * interrupt.
 *
 * @param	InstancePtr is a pointer to the XDpPsu instance.
 *
 * @note	None.
 *
*******************************************************************************/
typedef void (*XDpPsu_HpdPulseHandler)(void *InstancePtr);

/**
 * The XDpPsu driver instance data. The user is required to allocate a variable
 * of this type for every XDpPsu device in the system. A pointer to a variable of
 * this type is then passed to the driver API functions.
 */
typedef struct {
	u32 IsReady;				/**< Device is initialized and
							ready. */
	XDpPsu_Config Config;			/**< Configuration structure for
							the DisplayPort TX
							core. */
	XDpPsu_SinkConfig RxConfig;		/**< Configuration structure for
							the RX device. */
	XDpPsu_LinkConfig LinkConfig;		/**< Configuration structure for
							the main link. */
	XDpPsu_BoardChar BoardChar;		/**< Some board characteristics
							information that affects
							link training. */
	XDpPsu_MainStreamAttributes MsaConfig; /**< Configuration structure
							for the main stream
							attributes (MSA). */
	u32 AuxDelayUs;				/**< Amount of latency in micro-
							seconds to use between
							AUX transactions. */
	u32 SAxiClkHz;				/**< The clock frequency of the
							core instance's
							S_AXI_ACLK port. */
	void *UserTimerPtr;			/**< Pointer to a timer instance
							used by the custom user
							delay/sleep function. */
	XDpPsu_HpdEventHandler HpdEventHandler;	/**< Callback function for Hot-
							Plug-Detect (HPD) event
							interrupts. */
	void *HpdEventCallbackRef;		/**< A pointer to the user data
							passed to the HPD event
							callback function. */
	XDpPsu_HpdPulseHandler HpdPulseHandler;	/**< Callback function for Hot-
							Plug-Detect (HPD) pulse
							interrupts. */
	void *HpdPulseCallbackRef;		/**< A pointer to the user data
							passed to the HPD pulse
							callback function. */

} XDpPsu;

extern const XVidC_VideoTimingMode XVidC_VideoTimingModes[XVIDC_VM_NUM_SUPPORTED];
/**************************** Function Prototypes *****************************/

/* xdppsu.c: Setup and initialization functions. */
u32 XDpPsu_InitializeTx(XDpPsu *InstancePtr);
void XDpPsu_CfgInitialize(XDpPsu *InstancePtr, XDpPsu_Config *ConfigPtr,
							u32 EffectiveAddr);
u32 XDpPsu_GetRxCapabilities(XDpPsu *InstancePtr);

/* xdppsu.c: Link policy maker functions. */
u32 XDpPsu_CfgMainLinkMax(XDpPsu *InstancePtr);
u32 XDpPsu_EstablishLink(XDpPsu *InstancePtr);
u32 XDpPsu_CheckLinkStatus(XDpPsu *InstancePtr, u8 LaneCount);

/* xdppsu.c: AUX transaction functions. */
u32 XDpPsu_AuxRead(XDpPsu *InstancePtr, u32 DpcdAddress, u32 BytesToRead,
								void *ReadData);
u32 XDpPsu_AuxWrite(XDpPsu *InstancePtr, u32 DpcdAddress, u32 BytesToWrite,
							void *WriteData);
u32 XDpPsu_IicRead(XDpPsu *InstancePtr, u8 IicAddress, u16 Offset,
					u16 BytesToRead, void *ReadData);
u32 XDpPsu_IicWrite(XDpPsu *InstancePtr, u8 IicAddress, u8 BytesToWrite,
							void *WriteData);

/* xdppsu.c: Functions for controlling the link configuration. */
u32 XDpPsu_SetDownspread(XDpPsu *InstancePtr, u8 Enable);
u32 XDpPsu_SetEnhancedFrameMode(XDpPsu *InstancePtr, u8 Enable);
u32 XDpPsu_SetLaneCount(XDpPsu *InstancePtr, u8 LaneCount);
u32 XDpPsu_SetLinkRate(XDpPsu *InstancePtr, u8 LinkRate);
u32 XDpPsu_SetScrambler(XDpPsu *InstancePtr, u8 Enable);

/* xdppsu.c: General usage functions. */
u32 XDpPsu_IsConnected(XDpPsu *InstancePtr);
void XDpPsu_EnableMainLink(XDpPsu *InstancePtr, u8 Enable);
void XDpPsu_ResetPhy(XDpPsu *InstancePtr, u32 Reset);

/* xdppsu_spm.c: Stream policy maker functions. */
void XDpPsu_CfgMsaRecalculate(XDpPsu *InstancePtr);
void XDpPsu_CfgMsaUseStandardVideoMode(XDpPsu *InstancePtr,
						XVidC_VideoMode VideoMode);
void XDpPsu_CfgMsaUseEdidPreferredTiming(XDpPsu *InstancePtr, u8 *Edid);
void XDpPsu_CfgMsaUseCustom(XDpPsu *InstancePtr,
		XDpPsu_MainStreamAttributes *MsaConfigCustom, u8 Recalculate);
void XDpPsu_CfgMsaSetBpc(XDpPsu *InstancePtr, u8 BitsPerColor);
void XDpPsu_CfgMsaEnSynchClkMode(XDpPsu *InstancePtr, u8 Enable);
void XDpPsu_SetVideoMode(XDpPsu *InstancePtr);
void XDpPsu_ClearMsaValues(XDpPsu *InstancePtr);
void XDpPsu_SetMsaValues(XDpPsu *InstancePtr);

/* xdppsu_intr.c: Interrupt handling functions. */
void XDpPsu_SetHpdEventHandler(XDpPsu *InstancePtr,
			XDpPsu_HpdEventHandler CallbackFunc, void *CallbackRef);
void XDpPsu_SetHpdPulseHandler(XDpPsu *InstancePtr,
			XDpPsu_HpdPulseHandler CallbackFunc, void *CallbackRef);
void XDpPsu_HpdInterruptHandler(XDpPsu *InstancePtr);

/* xdppsu_selftest.c: Self test function. */
u32 XDpPsu_SelfTest(XDpPsu *InstancePtr);

/* xdppsu_sinit.c: Configuration extraction function.*/
XDpPsu_Config *XDpPsu_LookupConfig(u16 DeviceId);

/* xdppsu_edid.c: EDID utility functions. */
u32 XDpPsu_GetEdid(XDpPsu *InstancePtr, u8 *Edid);
u32 XDpPsu_GetEdidBlock(XDpPsu *InstancePtr, u8 *Data, u8 BlockNum);
u32 XDpPsu_GetDispIdDataBlock(u8 *DisplayIdRaw, u8 SectionTag,
							u8 **DataBlockPtr);

void XDpPsu_SetColorEncode(XDpPsu *InstancePtr, XDpPsu_ColorEncoding ColorEncode);

#endif /* XDPPSU_H_ */
