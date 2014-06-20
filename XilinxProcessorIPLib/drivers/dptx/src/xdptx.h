/*******************************************************************************
 *
 * Copyright (C) 2014 Xilinx, Inc.  All rights reserved.
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
*******************************************************************************/
/******************************************************************************/
/**
 *
 * @file xdptx.h
 *
 * The Xilinx DisplayPort transmitter (TX) driver.
 *
 * The driver currently supports single-stream transport (SST) functionality.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- -----------------------------------------------
 * 1.00a als  05/17/14 Initial release.
 * </pre>
 *
*******************************************************************************/

#ifndef XDPTX_H_                        /* Prevent circular inclusions */
#define XDPTX_H_                        /* by using protection macros. */

/******************************* Include Files ********************************/

#include "xdptx_hw.h"
#include "xil_assert.h"
#include "xil_types.h"

/******************* Macros (Inline Functions) Definitions ********************/

/******************************************************************************/
/**
 * This macro checks if there is a connected sink.
 *
 * @param       InstancePtr is a pointer to the XDptx instance.
 *
 * @return
 *              - TRUE if there is a connection.
 *              - FALSE if there is no connection.
 *
 * @note        C-style signature:
 *              void XDptx_IsConnected(XDptx *InstancePtr)
 *
*******************************************************************************/
#define XDptx_IsConnected(InstancePtr) \
        (XDptx_ReadReg(InstancePtr->TxConfig.BaseAddr, \
        XDPTX_INTERRUPT_SIG_STATE) & XDPTX_INTERRUPT_SIG_STATE_HPD_STATE_MASK)

/****************************** Type Definitions ******************************/

/**
 * This typedef enumerates the list of available standard display monitor
 * timings as specified in the mode_table.c file. The naming format is:
 *
 * XDPTX_VM_<RESOLUTION>_<REFRESH RATE (HZ)>_<P|RB>
 *
 * Where RB stands for reduced blanking.
 */
typedef enum {
        XDPTX_VM_640x480_60_P,
        XDPTX_VM_800x600_60_P,
        XDPTX_VM_848x480_60_P,
        XDPTX_VM_1024x768_60_P,
        XDPTX_VM_1280x768_60_P_RB,
        XDPTX_VM_1280x768_60_P,
        XDPTX_VM_1280x800_60_P_RB,
        XDPTX_VM_1280x800_60_P,
        XDPTX_VM_1280x960_60_P,
        XDPTX_VM_1280x1024_60_P,
        XDPTX_VM_1360x768_60_P,
        XDPTX_VM_1400x1050_60_P_RB,
        XDPTX_VM_1400x1050_60_P,
        XDPTX_VM_1440x900_60_P_RB,
        XDPTX_VM_1440x900_60_P,
        XDPTX_VM_1600x1200_60_P,
        XDPTX_VM_1680x1050_60_P_RB,
        XDPTX_VM_1680x1050_60_P,
        XDPTX_VM_1792x1344_60_P,
        XDPTX_VM_1856x1392_60_P,
        XDPTX_VM_1920x1200_60_P_RB,
        XDPTX_VM_1920x1200_60_P,
        XDPTX_VM_1920x1440_60_P,
        XDPTX_VM_2560x1600_60_P_RB,
        XDPTX_VM_2560x1600_60_P,
        XDPTX_VM_800x600_56_P,
        XDPTX_VM_1600x1200_65_P,
        XDPTX_VM_1600x1200_70_P,
        XDPTX_VM_1024x768_70_P,
        XDPTX_VM_640x480_72_P,
        XDPTX_VM_800x600_72_P,
        XDPTX_VM_640x480_75_P,
        XDPTX_VM_800x600_75_P,
        XDPTX_VM_1024x768_75_P,
        XDPTX_VM_1152x864_75_P,
        XDPTX_VM_1280x768_75_P,
        XDPTX_VM_1280x800_75_P,
        XDPTX_VM_1280x1024_75_P,
        XDPTX_VM_1400x1050_75_P,
        XDPTX_VM_1440x900_75_P,
        XDPTX_VM_1600x1200_75_P,
        XDPTX_VM_1680x1050_75_P,
        XDPTX_VM_1792x1344_75_P,
        XDPTX_VM_1856x1392_75_P,
        XDPTX_VM_1920x1200_75_P,
        XDPTX_VM_1920x1440_75_P,
        XDPTX_VM_2560x1600_75_P,
        XDPTX_VM_640x350_85_P,
        XDPTX_VM_640x400_85_P,
        XDPTX_VM_720x400_85_P,
        XDPTX_VM_640x480_85_P,
        XDPTX_VM_800x600_85_P,
        XDPTX_VM_1024x768_85_P,
        XDPTX_VM_1280x768_85_P,
        XDPTX_VM_1280x800_85_P,
        XDPTX_VM_1280x960_85_P,
        XDPTX_VM_1280x1024_85_P,
        XDPTX_VM_1400x1050_85_P,
        XDPTX_VM_1440x900_85_P,
        XDPTX_VM_1600x1200_85_P,
        XDPTX_VM_1680x1050_85_P,
        XDPTX_VM_1920x1200_85_P,
        XDPTX_VM_2560x1600_85_P,
        XDPTX_VM_800x600_120_P_RB,
        XDPTX_VM_1024x768_120_P_RB,
        XDPTX_VM_1280x768_120_P_RB,
        XDPTX_VM_1280x800_120_P_RB,
        XDPTX_VM_1280x960_120_P_RB,
        XDPTX_VM_1280x1024_120_P_RB,
        XDPTX_VM_1360x768_120_P_RB,
        XDPTX_VM_1400x1050_120_P_RB,
        XDPTX_VM_1440x900_120_P_RB,
        XDPTX_VM_1600x1200_120_P_RB,
        XDPTX_VM_1680x1050_120_P_RB,
        XDPTX_VM_1792x1344_120_P_RB,
        XDPTX_VM_1856x1392_120_P_RB,
        XDPTX_VM_1920x1200_120_P_RB,
        XDPTX_VM_1920x1440_120_P_RB,
        XDPTX_VM_2560x1600_120_P_RB,
        XDPTX_VM_1366x768_60_P,
        XDPTX_VM_1920x1080_60_P,
        XDPTX_VM_UHD_30_P,
        XDPTX_VM_720_60_P,
        XDPTX_VM_480_60_P,
        XDPTX_VM_UHD2_60_P,
        XDPTX_VM_UHD_60,
        XDPTX_VM_USE_EDID_PREFERRED,
        XDPTX_VM_LAST = XDPTX_VM_USE_EDID_PREFERRED
} XDptx_VideoMode;

/**
 * This typedef contains the display monitor timing attributes for a video mode.
 */
typedef struct {
        XDptx_VideoMode	VideoMode;      /**< Enumerated key. */
        u8 DmtId;                       /**< Standard DMT ID number. */
        u16 HResolution;                /**< Horizontal resolution. */
        u16 VResolution;                /**< Vertical resolution. */
        u32 PixelClkKhz;                /**< Pixel frequency (in KHz). */
        u8 Scan;                        /**< Interlaced/non-interlaced. */
        u8 HSyncPolarity;               /**< Horizontal polarity. */
        u8 VSyncPolarity;               /**< Vertical polarity. */
        u32 HFrontPorch;                /**< Horizontal front porch. */
        u32 HSyncPulseWidth;            /**< Horizontal synchronization pulse
                                                width. */
        u32 HBackPorch;                 /**< Horizontal back porch. */
        u32 VFrontPorch;                /**< Vertical front porch. */
        u32 VSyncPulseWidth;            /**< Vertical synchronization pulse
                                                width.*/
        u32 VBackPorch;                 /**< Vertical back porch. */
} XDptx_DmtMode;

extern XDptx_DmtMode XDptx_DmtModes[];

/**
 * This typedef contains configuration information for the DisplayPort TX core.
 */
typedef struct {
        u16 DeviceId;           /**< Device instance ID. */
        u32 BaseAddr;           /**< The base address of the core. */
        u32 SAxiClkHz;          /**< The clock frequency of the core's
                                        S_AXI_ACLK port. */
        u8 MaxLaneCount;	/**< The maximum lane count supported by this
                                        core's instance. */
        u8 MaxLinkRate;		/**< The maximum link rate supported by this
                                        core's instance. */
        u8 MaxBitsPerColor;	/**< The maximum bits/color supported by this
                                        core's instance*/
        u8 QuadPixelEn;		/**< Quad pixel support by this core's
                                        instance. */
        u8 DualPixelEn;		/**< Dual pixel support by this core's
                                        instance. */
        u8 YOnlyEn;             /**< YOnly format support by this core's
                                        instance. */
        u8 YCrCbEn;             /**< YCrCb format support by this core's
                                        instance. */
} XDptx_Config;

/**
 * This typedef contains configuration information about the sink.
 */
typedef struct {
        u8 DpcdRxCapsField[256];        /**< The raw capabilities field
                                                of the sink's DPCD. */
        u8 Edid[128];                   /**< The sink's raw EDID. */
        u8 LaneStatusAdjReqs[6];        /**< This is a raw read of the receiver
                                                DPCD's status registers. The
                                                first 4 bytes correspond to the
                                                lane status from the receiver's
                                                DPCD associated with clock
                                                recovery, channel equalization,
                                                symbol lock, and interlane
                                                alignment. The 2 remaining bytes
                                                represent the adjustments
                                                requested by the DPCD. */
} XDptx_SinkConfig;

/**
 * This typedef contains configuration information about the main link settings.
 */
typedef struct {
        u8 LaneCount;                   /**< The current lane count of the main
                                                link. */
        u8 LinkRate;                    /**< The current link rate of the main
                                                link. */
        u8 ScramblerEn;                 /**< Symbol scrambling is currently in
                                                use over the main link. */
        u8 EnhancedFramingMode;         /**< Enhanced frame mode is currently in
                                                use over the main link. */
        u8 DownspreadControl;           /**< Downspread control is currently in
                                                use over the main link. */
        u8 MaxLaneCount;                /**< The maximum lane count of the
                                                source-sink main link. */
        u8 MaxLinkRate;                 /**< The maximum link rate of the
                                                source-sink main link. */
        u8 SupportEnhancedFramingMode;  /**< Enhanced frame mode is supported by
                                                the receiver. */
        u8 SupportDownspreadControl;    /**< Downspread control is supported by
                                                the receiver. */
        u8 VsLevel;                     /**< The current voltage swing level for
                                                each lane. */
        u8 PeLevel;                     /**< The current pre-emphasis/cursor
                                                level for each lane. */
        u8 ComponentFormat;             /**< The component format currently in
                                                use over the main link. */
        u8 DynamicRange;                /**< The dynamic range currently in use
                                                over the main link. */
        u8 YCbCrColorimetry;            /**< The YCbCr colorimetry currently in
                                                use over the main link. */
        u8 SynchronousClockMode;        /**< Synchronous clock mode is currently
                                                in use over the main link. */
        u8 Pattern;                     /**< The current pattern currently in
                                                use over the main link. */
} XDptx_LinkConfig;                     

/**
 * This typedef contains the main stream attributes which determine how the
 * video will be displayed.
 */
typedef struct {
        u32 HClkTotal;
        u32 VClkTotal;
        u32 HSyncPulseWidth;
        u32 VSyncPulseWidth;
        u32 HResolution;
        u32 VResolution;
        u32 HSyncPolarity;
        u32 VSyncPolarity;
        u32 HStart;
        u32 VStart;
        u32 VBackPorch;
        u32 VFrontPorch;
        u32 HBackPorch;
        u32 HFrontPorch;
        u32 Misc0;
        u32 Misc1;
        u32 MVid;
        u32 NVid;
        u32 TransferUnitSize;
        u32 UserPixelWidth;
        u32 DataPerLane;
        u32 AvgBytesPerTU;
        u32 InitWait;
        u32 Interlaced;
        u32 BitsPerColor;
} XDptx_MainStreamAttributes;

/******************************************************************************/
/**
 * Callback type which represents a custom timer wait handler. This is only
 * used for Microblaze since it doesn't have a native sleep function. To avoid
 * dependency on a hardware timer, the default wait functionality is implemented
 * using loop iterations; this isn't too accurate. If a custom timer handler is
 * used, the user may implement their own wait implementation using a hardware
 * timer (see example/) for better accuracy.
 * 
 * @param       InstancePtr is a pointer to the XDptx instance.
 * @param       MicroSeconds is the number of microseconds to be passed to the
 *              timer function.
 *
*******************************************************************************/
typedef void (*XDptx_TimerHandler)(void *InstancePtr, u32 MicroSeconds);

/******************************************************************************/ 
/**
 * Callback type which represents the handler for a hot-plug-detect event
 * interrupt.
 *
 * @param       InstancePtr is a pointer to the XDptx instance.
 *
*******************************************************************************/
typedef void (*XDptx_HpdEventHandler)(void *InstancePtr);

/******************************************************************************/
/**
 * Callback type which represents the handler for a hot-plug-detect pulse
 * interrupt.
 *
 * @param       InstancePtr is a pointer to the XDptx instance.
 *
*******************************************************************************/
typedef void (*XDptx_HpdPulseHandler)(void *InstancePtr);

/**
 * The XDptx driver instance data. The user is required to allocate a variable
 * of this type for every XDptx device in the system. A pointer to a variable of
 * this type is then passed to the driver API functions.
 */
typedef struct {
        u32 IsReady;                            /**< Device is initialized and
                                                        ready. */
        u8 TrainAdaptive;                       /**< Downshift lane count and
                                                        link rate if necessary
                                                        during training. */
        u8 HasRedriverInPath;                   /**< Redriver in path requires
                                                        different voltage swing
                                                        and pre-emphasis. */
        XDptx_Config TxConfig;                  /**< Configuration structure for
                                                        the core. */
        XDptx_SinkConfig RxConfig;              /**< Configuration structure for
                                                        the sink. */
        XDptx_LinkConfig LinkConfig;            /**< Configuration structure for
                                                        the main link. */
        XDptx_MainStreamAttributes MsaConfig;   /**< Configuration structure for
                                                        the main stream
                                                        attributes. */
        XDptx_TimerHandler UserTimerWaitUs;     /**< Custom user function for
                                                        delay/sleep. */
        void *UserTimerPtr;                     /**< Pointer to a timer instance
                                                        used by the custom user
                                                        delay/sleep function. */
        XDptx_HpdEventHandler HpdEventHandler;  /**< Callback function for hot-
                                                        plug-detect event
                                                        interrupts. */
        void *HpdEventCallbackRef;              /**< A pointer to the user data
                                                        passed to the HPD event
                                                        callback function.*/
        XDptx_HpdPulseHandler HpdPulseHandler;  /**< Callback function for hot-
                                                        plug-detect pulse
                                                        interrupts. */
        void *HpdPulseCallbackRef;              /**< A pointer to the user data
                                                        passed to the HPD pulse
                                                        callback function.*/
} XDptx;

/**************************** Function Prototypes *****************************/

/* xdptx.c: Setup and initialization functions. */
u32 XDptx_InitializeTx(XDptx *InstancePtr);
void XDptx_CfgInitialize(XDptx *InstancePtr, XDptx_Config *ConfigPtr,
                                                        u32 EffectiveAddr);
u32 XDptx_GetSinkCapabilities(XDptx *InstancePtr);
u32 XDptx_GetEdid(XDptx *InstancePtr);

/* xdptx.c: Link policy maker functions. */
u32 XDptx_CfgMainLinkMax(XDptx *InstancePtr);
u32 XDptx_EstablishLink(XDptx *InstancePtr);
u32 XDptx_CheckLinkStatus(XDptx *InstancePtr, u8 LaneCount);
void XDptx_EnableTrainAdaptive(XDptx *InstancePtr, u8 Enable);
void XDptx_SetHasRedriverInPath(XDptx *InstancePtr, u8 Set);

/* xdptx.c: AUX transaction functions. */
u32 XDptx_AuxRead(XDptx *InstancePtr, u32 Address, u32 NumBytes, void *Data);
u32 XDptx_AuxWrite(XDptx *InstancePtr, u32 Address, u32 NumBytes, void *Data);
u32 XDptx_IicWrite(XDptx *InstancePtr, u8 IicAddress, u8 RegStartAddress,
                                                u8 NumBytes, u8 *DataBuffer);
u32 XDptx_IicRead(XDptx *InstancePtr, u8 IicAddress, u8 RegStartAddress,
                                                u8 NumBytes, u8 *DataBuffer);

/* xdptx.c: Functions for controlling the link configuration. */
u32 XDptx_SetDownspread(XDptx *InstancePtr, u8 Enable);
u32 XDptx_SetEnhancedFrameMode(XDptx *InstancePtr, u8 Enable);
u32 XDptx_SetLaneCount(XDptx *InstancePtr, u8 LaneCount);
u32 XDptx_SetLinkRate(XDptx *InstancePtr, u8 LinkRate);
u32 XDptx_SetScrambler(XDptx *InstancePtr, u8 Enable);

/* xdptx.c: General usage functions. */
void XDptx_EnableMainLink(XDptx *InstancePtr);
void XDptx_DisableMainLink(XDptx *InstancePtr);
void XDptx_ResetPhy(XDptx *InstancePtr, u32 Reset);
void XDptx_WaitUs(XDptx *InstancePtr, u32 MicroSeconds);
void XDptx_SetUserTimerHandler(XDptx *InstancePtr,
                        XDptx_TimerHandler CallbackFunc, void *CallbackRef);

/* xdptx_spm.c: Stream policy maker functions. */
void XDptx_CfgMsaRecalculate(XDptx *InstancePtr);
u32 XDptx_CfgMsaUseStandardVideoMode(XDptx *InstancePtr,
                                                XDptx_VideoMode VideoMode);
void XDptx_CfgMsaUseEdidPreferredTiming(XDptx *InstancePtr);
void XDptx_CfgMsaUseCustom(XDptx *InstancePtr,
                XDptx_MainStreamAttributes *MsaConfigCustom, u8 Recalculate);
u32 XDptx_CfgMsaSetBpc(XDptx *InstancePtr, u8 BitsPerColor);
void XDptx_SetVideoMode(XDptx *InstancePtr);

/* xdptx_intr.c: Interrupt handling functions. */
void XDptx_SetHpdEventHandler(XDptx *InstancePtr,
                        XDptx_HpdEventHandler CallbackFunc, void *CallbackRef);
void XDptx_SetHpdPulseHandler(XDptx *InstancePtr,
                        XDptx_HpdPulseHandler CallbackFunc, void *CallbackRef);
void XDptx_HpdInterruptHandler(XDptx *InstancePtr);

/* xdptx_selftest.c: Self test function. */
u32 XDptx_SelfTest(XDptx *InstancePtr);

/* xdptx_sinit.c: Configuration extraction function.*/
XDptx_Config *XDptx_LookupConfig(u16 DeviceId);

#endif /* XDPTX_H_ */
