/******************************************************************************
 *
 * Copyright (C) 2015 Xilinx, Inc.  All rights reserved.
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
* @file xvprocss.h
* @addtogroup vprocss_v1_0
* @{
* @details
*
* This is main header file of the Xilinx Video Processing Subsystem driver
*
* <b>Video Processing Subsystem Overview</b>
*
* Video Subsystem is a collection of IP cores bounded together by software
* to provide an abstract view of the processing pipe. It hides all the
* complexities of programming the underlying cores from end user.
*
* <b>Subsystem Driver Features</b>
*
* Video Subsystem supports following features
* 	- AXI Stream Input/Output interface
* 	- 2 or 4 pixel-wide video interface
* 	- up to 16 bits per component
* 	- RGB & YCbCb color space
* 	- Memory based/streaming mode scaling in either direction (Up/Down)
* 	- Up to 4k2k 60Hz resolution at both Input and Output interface
* 	- Interlaced input support
* 	- Frame rate conversion
* 		- Drop frames if input rate > output rate
* 		- Repeat frames if input rate < output rate
* 	- Auto configuration of processing pipe based on detected use case
* 		- Scale Up/Down
* 		- Zoom mode wherein a window in input is scaled to panel resolution
* 		- Picture-In-Picture mode wherein the input stream is scaled down to
* 		  a defined window size and background is painted to user define color
* 		- Color Space and color format Conversion
* 		- Interlaced to Progressive conversion
*
* <b>Subsystem Configurations</b>
*
* Two types of configurations are supported via GUI in IPI
* 	- Full Configuration: provides all the features mentioned above
* 	- Streaming Mode Configuration (aka Scaler-Only) with limited functionality
*
* Number of processing cores that get included in the design will depend upon
* the configuration selected. Static configuration parameters are stored in
* vprocss_g.c file that gets generated when compiling the board support package
* (BSP). A table is defined where each entry contains configuration information
* for the instances of the subsystem in the design. This information includes
* the elected configuration, sub-cores used and their device ID, base addresses
* of memory mapped devices, user specified DDR address for buffer management
* and address range available for subsystem frame/field buffers.
*
* Full configuration mode includes following sub-cores in HW
* 	- Scalers (horizontal/vertical)
* 	- Chroma Resampler (horizontal/vertical)
* 	- Color Space Converter
* 	- VDMA for buffer management
* 	- De-Interlacer
* 	- Letter Box
* 	- AXIS Switch
*
* Stream mode configuration mode includes following sub-cores in HW
* 	- Scalers (horizontal/vertical)
*
* The subsystem driver itself always includes the full software stack
* irrespective of the configuration selected. Generic API's are provided to
* interact with the subsystem and/or with the included sub-cores.
* At run-time the subsystem will query the static configuration and configures
* itself for supported use cases
*
* <b>Subsystem Driver Description</b>
*
* Subsystem driver is built upon layer 1&2 device drivers of included sub-cores
* Layer 1 provides API's to peek/poke registers at HW level.
* Layer 2 provides API's that abstract sub-core functionality, providing an easy to
* use feature interface
*
* <b>Pre-Requisite's</b>
*
* Subsystem driver requires 2 support peripherals, Timer and an Interrupt
* Controller, to be present in the design and the application must register a
* handle to these with the subsystem using the provided API's.
*
* <b>Subsystem Driver Usage</b>
*
* The subsystem driver in itself is a dormant driver that needs application SW to
* make use of provided API's to configure it at boot-up. Thereafter application
* SW is responsible to monitor the system for external impetus and call the
* subsystem API's to communicate the change and trigger the reconfiguration of
* internal data processing pipe (refer API XVprocss_ConfigureSubsystem())
* AXI Stream configuration for input/output interface is derived from the
* Xilinx video common driver and only the resolutions listed therein are
* supported at this time
*
* <b>Interrupt Service</b>
*
* Currently no interrupts are available from the subsystem. User application is
* responsible for triggering processing pipe update when any change in subsystem
* configuration is performed at application level
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

* <b>Asserts</b>
*
* Asserts are used within all Xilinx drivers to enforce constraints on argument
* values. Asserts can be turned off on a system-wide basis by defining, at
* compile time, the NDEBUG identifier.  By default, asserts are turned on and
* it is recommended that application developers leave asserts on during
* development.
*
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who    Date     Changes
* ----- ---- -------- -------------------------------------------------------
* 1.00  rc   05/01/15   Initial Release

* </pre>
*
******************************************************************************/

#ifndef XVPROCSS_H /**< prevent circular inclusions by using protection macros*/
#define XVPROCSS_H

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/
#include "xstatus.h"
#include "xintc.h"
#include "xgpio.h"
#include "xaxis_switch.h"
#include "xvidc.h"
/**
 *  Sub-core Layer 2 driver includes
 *  Layer 2 includes Layer-1
 */
#include "xaxivdma.h"
#include "xv_csc_l2.h"
#include "xv_deinterlacer_l2.h"
#include "xv_hcresampler_l2.h"
#include "xv_vcresampler_l2.h"
#include "xv_hscaler_l2.h"
#include "xv_vscaler_l2.h"
#include "xv_letterbox_l2.h"

/****************************** Type Definitions ******************************/
/**
 *  AXIS Switch Port enumeration for Sub-Core connection
 */
typedef enum
{
  XVPROCSS_RTR_VIDOUT = 0, //M0
  XVPROCSS_RTR_SCALER_V,   //M1
  XVPROCSS_RTR_SCALER_H,   //M2
  XVPROCSS_RTR_VDMA,       //M3
  XVPROCSS_RTR_LBOX,       //M4
  XVPROCSS_RTR_CR_H,       //M5
  XVPROCSS_RTR_CR_V_IN,    //M6
  XVPROCSS_RTR_CR_V_OUT,   //M7
  XVPROCSS_RTR_CSC,        //M8
  XVPROCSS_RTR_DEINT,      //M9
  XVPROCSS_RTR_MAX
}XVPROCSS_RTR_MIx_ID;

/**
 * Subsystem Configuration Mode Select
 */
typedef enum
{
  XVPROCSS_MODE_MAX = 0,
  XVPROCSS_MODE_STREAM
}XVPROCSS_CONFIG_MODE;

/**
 * Types of Windows (Sub-frames) available in the Subsystem
 */
typedef enum
{
  XVPROCSS_ZOOM_WIN = 0,
  XVPROCSS_PIP_WIN,
  XVPROCSS_PIXEL_WIN
}XVprocss_Win;

/**
 * Scaling Modes supported
 */
typedef enum
{
  XVPROCSS_SCALE_1_1 = 0,
  XVPROCSS_SCALE_UP,
  XVPROCSS_SCALE_DN,
  XVPROCSS_SCALE_NOT_SUPPORTED
}XVprocss_ScaleMode;

/**
 * Video Processing Subsystem internal scratch pad memory.
 * This contains internal flags, state variables, routing table
 * and other meta-data required by the subsystem. Each instance
 * of the subsystem will have its own scratch pad memory
 */
typedef struct
{
  XVidC_VideoWindow rdWindow; /**< window for Zoom/Pip feature support */
  XVidC_VideoWindow wrWindow; /**< window for Zoom/Pip feature support */

  u32 deintBufAddr;           /**< Deinterlacer field buffer Addr. in DDR */
  u8 vdmaBytesPerPixel;       /**< Number of bytes required to store 1 pixel */

  u8 RtngTable[XVPROCSS_RTR_MAX]; /**< Storage for computed routing map */
  u8 startCore[XVPROCSS_RTR_MAX]; /**< Enable flag to start sub-core */
  u8 RtrNumCores;      /**< Number of sub-cores in routing map */
  u8 ScaleMode;        /**< Stored computed scaling mode - UP/DN/1:1 */
  u8 memEn;            /**< Flag to indicate if stream routes through memory */
  u8 ZoomEn;           /**< Flag to store Zoom feature state */
  u8 PipEn;            /**< Flag to store PIP feature state */
  u16 vidInWidth;      /**< Input H Active */
  u16 vidInHeight;     /**< Input V Active */
  XVidC_ColorFormat strmCformat; /**< processing pipe color format */
  XVidC_ColorFormat cscIn;  /**< CSC core input color format */
  XVidC_ColorFormat cscOut; /**< CSC core output color format */
  XVidC_ColorFormat hcrIn;  /**< horiz. cresmplr core input color format */
  XVidC_ColorFormat hcrOut; /**< horiz. cresmplr core output color format */
}XVprocss_IData;

/**
 * Sub-Core Configuration Table
 */
typedef struct
{
  u16 isPresent; /**< Flag to indicate if sub-core is present in the design*/
  u16 DeviceId;  /**< Device ID of the sub-core */
}XSubCore;

/**
 * Video Processing Subsystem configuration structure.
 * Each subsystem device should have a configuration structure associated
 * that defines the MAX supported sub-cores within subsystem
 */

typedef struct
{
  u16 DeviceId;	         /**< DeviceId is the unique ID  of the device */
  u32 BaseAddress;       /**< BaseAddress is the physical base address of
                              the device's registers */
  u8 Mode;               /**< Subsystem configuration mode */
  u8 PixPerClock;        /**< Number of Pixels Per Clock processed by Subsystem */
  u16 PixPrecision;      /**< Processing precision of the data pipe */
  XSubCore HCrsmplr;     /**< Sub-core instance configuration */
  XSubCore VCrsmplrIn;   /**< Sub-core instance configuration */
  XSubCore VCrsmplrOut;  /**< Sub-core instance configuration */
  XSubCore Vscale;       /**< Sub-core instance configuration */
  XSubCore Hscale;       /**< Sub-core instance configuration */
  XSubCore Vdma;         /**< Sub-core instance configuration */
  XSubCore Lbox;         /**< Sub-core instance configuration */
  XSubCore Csc;          /**< Sub-core instance configuration */
  XSubCore Deint;        /**< Sub-core instance configuration */
  XSubCore Router;       /**< Sub-core instance configuration */
  XSubCore RstAxis;      /**< Axi stream reset network instance configuration */
  XSubCore RstAximm;     /**< Axi MM reset network instance configuration */
  u32 UsrExtMemBaseAddr; /**< DDR base address for buffer management */
  u32 UsrExtMemAddr_Range; /**< Range of addresses available for buffers */
} XVprocss_Config;

/**
 * The XVprocss driver instance data. The user is required to allocate a variable
 * of this type for every XVprocss device in the system. A pointer to a variable
 * of this type is then passed to the driver API functions.
 */
typedef struct
{
  XVprocss_Config Config;	/**< Hardware configuration */
  u32 IsReady;		        /**< Device and the driver instance are initialized */

  XAxis_Switch *router;         /**< handle to sub-core driver instance */
  XGpio *rstAxis;               /**< handle to sub-core driver instance */
  XGpio *rstAximm;              /**< handle to sub-core driver instance */
                                /**< handle to sub-core driver instance */
  XV_hcresampler *hcrsmplr;     /**< handle to sub-core driver instance */
  XV_vcresampler *vcrsmplrIn;   /**< handle to sub-core driver instance */
  XV_vcresampler *vcrsmplrOut;  /**< handle to sub-core driver instance */
  XV_vscaler *vscaler;          /**< handle to sub-core driver instance */
  XV_hscaler *hscaler;          /**< handle to sub-core driver instance */
  XAxiVdma *vdma;               /**< handle to sub-core driver instance */
  XV_letterbox *lbox;           /**< handle to sub-core driver instance */
  XV_csc *csc;                  /**< handle to sub-core driver instance */
  XV_deinterlacer *deint;       /**< handle to sub-core driver instance */

  /**
   * Layer2 SW Register (Every Subsystem instance will have it's own copy
     of Layer 2 register bank for applicable sub-cores)
   */
  XV_csc_L2Reg cscL2Reg;      /**< Layer 2 register bank for csc sub-core */
  XV_vscaler_l2 vscL2Reg;     /**< Layer 2 register bank for vsc sub-core */
  XV_hscaler_l2 hscL2Reg;     /**< Layer 2 register bank for hsc sub-core */

  //I/O Streams
  XVidC_VideoStream VidIn;     /**< Input  AXIS configuration */
  XVidC_VideoStream VidOut;    /**< Output AXIS configuration */

  XVprocss_IData idata;        /**< Internal Scratch pad memory for subsystem instance */

  XIntc *pXintc;           /**< handle to system interrupt controller */
  XVidC_DelayHandler UsrDelaymsec;  /**< custom user function for delay/sleep */
  void *pUsrTmr;           /**< handle to timer instance used by user delay function */
} XVprocss;

/************************** Macros Definitions *******************************/
/*****************************************************************************/
/**
 * This macro checks if subsystem is in Maximum configuration mode
 *
 * @param  pVprocss is a pointer to the Video Processing subsystem instance
 * @return Return 1 if condition is TRUE or 0 if FALSE
 *
 *****************************************************************************/
#define XVprocss_IsConfigModeMax(pVprocss) \
                   ((pVprocss)->Config.Mode == XVPROCSS_MODE_MAX)

/*****************************************************************************/
/**
 * This macro checks if subsystem configuration is in Stream Mode (Scaler Only)
 *
 * @param  pVprocss is pointer to the Video Processing subsystem instance
 *
 * @return Returns 1 if condition is TRUE or 0 if FALSE
 *
 *****************************************************************************/
#define XVprocss_IsConfigModeStream(pVprocss)  \
                 ((pVprocss)->Config.Mode == XVPROCSS_MODE_STREAM)


/*****************************************************************************/
/**
 * This macro returns the current state of PIP Mode stored in subsystem internal
 * scratch pad memory
 *
 * @param  pVprocss is a pointer to the Video Processing subsystem instance
 *
 * @return Returns 1 if PIP mode is ON or 0 if OFF
 *
 *****************************************************************************/
#define XVprocss_IsPipModeOn(pVprocss)        ((pVprocss)->idata.PipEn)

/*****************************************************************************/
/**
 * This macro returns the current state of Zoom Mode stored in subsystem internal
 * scratch pad memory
 *
 * @param  pVprocss is a pointer to the Video Processing subsystem instance
 *
 * @return Returns 1 if ZOOM mode is ON or 0 if OFF
 *
 *****************************************************************************/
#define XVprocss_IsZoomModeOn(pVprocss)       ((pVprocss)->idata.ZoomEn)

/*****************************************************************************/
/**
 * This macro clears the PIP mode flag stored in subsystem internal scratch
 * pad memory. This call has no side-effect
 *
 * @param  pVprocss is a pointer to the Video Processing subsystem instance
 *
 * @return None
 *
 *****************************************************************************/
#define XVprocss_ResetPipModeFlag(pVprocss)    ((pVprocss)->idata.PipEn = FALSE)

/*****************************************************************************/
/**
 * This macro clears the ZOOM mode flag stored in subsystem internal scratch
 * pad memory. This call has no side-effect
 *
 * @param  pVprocss is pointer to the Video Processing subsystem instance
 *
 * @return None
 *
 *****************************************************************************/
#define XVprocss_ResetZoomModeFlag(pVprocss)    ((pVprocss)->idata.ZoomEn = FALSE)

/*****************************************************************************/
/**
 * This macro sets the specified stream's color format. It can be used to
 * update input or output stream. This call has no side-effect in isolation.
 * For change to take effect user must trigger processing path reconfiguration
 * by calling XVprocss_ConfigureSubsystem()
 *
 * @param  Stream is a pointer to the Subsystem Input or Output Stream
 * @param  ColorFormat is the requested color format
 *
 * @return None
 *
 *****************************************************************************/
#define XVprocss_SetStreamColorFormat(Stream, ColorFormat) \
                 ((Stream)->ColorFormatId = ColorFormat)

/*****************************************************************************/
/**
 * This macro sets the specified stream's color depth. It can be used to update
 * input or output stream. This call has no side-effect in isolation
 * For change to take effect user must trigger processing path reconfiguration
 * by calling XVprocss_ConfigureSubsystem()
 *
 * @param  Stream is a pointer to the Subsystem Input or Output Stream
 * @param  ColorDepth is the requested color depth
 *
 * @return None
 *
 *****************************************************************************/
#define XVprocss_SetStreamColorDepth(Stream, ColorDepth) \
                 ((Stream)->ColorDepth = ColorDepth)

/************************** Function Prototypes ******************************/
XVprocss_Config* XVprocss_LookupConfig(u32 DeviceId);
int  XVprocss_CfgInitialize(XVprocss *InstancePtr,
                            XVprocss_Config *CfgPtr,
                            u32 EffectiveAddr);
int  XVprocss_PowerOnInit(XVprocss *InstancePtr, u32 DeviceId);
void XVprocss_Start(XVprocss *InstancePtr);
void XVprocss_Stop(XVprocss *InstancePtr);
void XVprocss_Reset(XVprocss *InstancePtr);
int XVprocss_SetVidStreamIn(XVprocss *InstancePtr,
                            const XVidC_VideoStream *StrmIn);
int XVprocss_SetVidStreamOut(XVprocss *InstancePtr,
                             const XVidC_VideoStream *StrmOut);
int XVprocss_SetStreamResolution(XVidC_VideoStream *StreamPtr,
                                 const XVidC_VideoMode VmId);
void XVprocss_ReportCoreInfo(XVprocss *InstancePtr);
int XVprocss_ConfigureSubsystem(XVprocss *InstancePtr);
void XVprocss_SetZoomMode(XVprocss *InstancePtr, u8 OnOff);
void XVprocss_SetPipMode(XVprocss *InstancePtr, u8 OnOff);

void XVprocss_SetZoomPipWindow(XVprocss       *InstancePtr,
                               XVprocss_Win   mode,
                               XVidC_VideoWindow *win);
void XVprocss_GetZoomPipWindow(XVprocss       *InstancePtr,
                               XVprocss_Win   mode,
                               XVidC_VideoWindow *win);

void XVprocss_UpdateZoomPipWindow(XVprocss *InstancePtr);

void XVprocss_RegisterSysIntc(XVprocss *InstancePtr, XIntc *sysIntc);
void XVprocss_RegisterDelayHandler(XVprocss *InstancePtr,
                                   XVidC_DelayHandler waitmsec,
                                   void *pTimer);


#ifdef __cplusplus
}
#endif

#endif /* end of protection macro */
/** @} */
