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
*
* This is main header file of the Xilinx Video Processing Subsystem driver
*
* <b>Video Processing Subsystem Overview</b>
*
* Video Subsystem is a collection of IP cores bonded together by software
* to provide an abstract view of the processing pipe. It hides all the
* complexities of programming the underlying cores from end user.
*
* <b>Subsystem Driver Features</b>
*
* Video Subsystem supports following features
* 	- AXI Stream Input/Output interface
* 	- 1, 2 or 4 pixel-wide video interface
* 	- 8/10/12/16 bits per component
* 	- RGB & YCbCr color space
* 	- Memory based/streaming mode scaling in either direction (Up/Down)
* 	- Up to 4k2k 60Hz resolution at both Input and Output interface
* 	- Interlaced input support (1080i 50Hz/60Hz)
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
* Six types of configurations, each with options, are supported via GUI in IPI
* 	- Full Configuration: provides all the features mentioned above
* 	- Five streaming mode configurations have specific limited functionalities:
*     - Scaler-only mode allows only for changing the picture size
*     - Deinterlace-only mode allows for converting interlaced to progressive
*     - Csc-only mode allows only for changing the color space, e.g. YUV to RGB
*     - Vertical Chroma Resamp-only mode allows only for 420<->422 conversion
*     - Horizontal Chroma Resamp-only mode allows only for 422<->444 conversion
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
* 	- Deinterlacer
* 	- Chroma Resamplers (one horizontal and two vertical)
* 	- Color Space Converter
* 	- VDMA for buffer management
* 	- Letterbox
* 	- AXIS Switch
*
* Streaming mode configurations include the following sub-cores in HW
* 	- Scaler-only: Scalers (horizontal/vertical)
* 	- Deinterlace-only: Deinterlacer
* 	- Csc-only: Color Space Converter
* 	- Vertical Chroma Resamp-only: One Vertical Chroma Resampler
* 	- Horizontal Chroma Resamp-only: Horizontal Chroma Resampler
*
* The subsystem driver itself always includes the full software stack
* irrespective of the configuration selected. Generic API's are provided to
* interact with the subsystem and/or with the included sub-cores.
* At run-time the subsystem will query the static configuration and configure
* itself for supported use cases
*
* <b>Subsystem Driver Description</b>
*
* Subsystem driver is built upon layer 1&2 device drivers of included sub-cores
* Layer 1 provides API's to peek/poke registers at HW level.
* Layer 2 provides API's that abstract sub-core functionality, providing an
* easy to use feature interface
*
* <b>Pre-Requisite's</b>
*
*   - For memory based design (Full Fledged Topology with VDMA, and/or
*     Deinterlace with MADi) the application must program the base address of
*     the video buffers in memory. Refer to Memory Requirement section below.
*   - For microblaze based designs it is recommended to include a timer
*     peripheral in the design and application should register a delay handling
*     routine with the subsystem using the provided API.
*
* <b>Subsystem Driver Usage</b>
*
* The subsystem driver in itself is a dormant driver that needs application SW to
* make use of provided API's to configure it at boot-up. Thereafter application
* SW is responsible to monitor the system for external impetus and call the
* subsystem API's to communicate the change and trigger the reconfiguration of
* internal data processing pipe (refer to API XVprocSs_ConfigureSubsystem())
* AXI Stream configuration for input/output interface is derived from the
* Xilinx video common driver and only the resolutions listed therein are
* supported at this time
*
* <b>Memory Requirement</b>
*
* For full configuration mode DDR memory is used to store video frame buffers
* Subsystem uses 5 frame buffers for Progressive input and 3 field buffers for
* interlaced input. The amount of memory required by the subsystem can be
* calculated by below equation
*
*  - 5 * MAX_WIDTHp * MAX_HEIGHTp * NUM_VIDEO_COMPONENTS * BytesPerComp
*                        +
*    3 * MAX_WIDTHi * MAX_HEIGHTi * NUM_VIDEO_COMPONENTS * BytesPerComp
*
* BytesPerComp
*   - 1 Byte for 8 bit data pipe
*   - 2 Byte for 10/12/16 bit data pipe
*
* The location of these buffers in the memory is system dependent and as such
* must be determined by the system designer and the application code is
* responsible to program the base address of the buffer memory prior to
* initializing the subsystem. API to use is defined below
*   - XVprocSs_SetFrameBufBaseaddr
*
* <b>Interrupt Service</b>
*
* Currently no interrupts are available from the subsystem. User application is
* responsible for triggering processing pipe update when any change in subsystem
* configuration is performed at application level
*
* <b>Log Capability</b>
* Subsystem driver implements a logging feature that captures the interaction
* between included sub-core(s) as the subsystem is being configured and started.
* This is a potenital debugging aid should the system not behave as expected.
* If code size becomes a concern this logging capability can be removed from
* the driver by defining XV_CONFIG_LOG_VPRCOSS_DISABLE preprocessor macro in
* driver/BSP makefile. For maximun code savings logging capaibility can be
* disabled, globally, for all included video drivers in BSP by defining the
* preprocessor macro XV_CONFIG_LOG_DISABLE_ALL in the BSP makefile.
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
* 1.00  rco   08/28/15  Initial Release
* 2.00  rco   11/05/15  Update to adapt to sub-core layer 2 changes
*       dmc   12/02/15  Added four new topologies. There are now six topologies
*       dmc   12/17/15  Accommodate Full topology with no VDMA
*                       Rename and modify H,VCresample constants and routines
*                       Add macros to query for the new topologies
*       dmc   01/11/16  Add new data struct, enums, constants and prototypes
*                       to support a new Event Logging system for xvprocss.
* 2.10  rco   07/20/16  Add lbox background color storage to context data
*                       Used UINTPTR instead of u32 for Baseaddress, Frameaddr
*                       Changed the prototype of XVprocSs_CfgInitialize and
*                       XVprocSs_SetFrameBufBaseaddr API
* 2.30  rco  11/15/16   Make debug log optional (can be disabled via makefile)
* 			 12/15/16   Added HasMADI configuration option
*
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
#include "xgpio.h"
#include "xaxis_switch.h"
#include "xvidc.h"
#include "xaxivdma.h"
#include "xvprocss_log.h"

/**
 *  Subsystem sub-core layer 2 header files
 *  Layer 2 includes Layer-1
 */
#include "xv_csc_l2.h"
#include "xv_deinterlacer_l2.h"
#include "xv_hcresampler_l2.h"
#include "xv_vcresampler_l2.h"
#include "xv_hscaler_l2.h"
#include "xv_vscaler_l2.h"
#include "xv_letterbox_l2.h"

/****************************** Type Definitions ******************************/
/**
 *  This typedef enumerates the AXIS Switch Port for Sub-Core connection
 */
typedef enum
{
  XVPROCSS_SUBCORE_SCALER_V = 1,
  XVPROCSS_SUBCORE_SCALER_H,
  XVPROCSS_SUBCORE_VDMA,
  XVPROCSS_SUBCORE_LBOX,
  XVPROCSS_SUBCORE_CR_H,
  XVPROCSS_SUBCORE_CR_V_IN,
  XVPROCSS_SUBCORE_CR_V_OUT,
  XVPROCSS_SUBCORE_CSC,
  XVPROCSS_SUBCORE_DEINT,
  XVPROCSS_SUBCORE_MAX
}XVPROCSS_SUBCORE_ID;

/**
 * This typedef enumerates supported subsystem configuration topology
 */
typedef enum
{
  XVPROCSS_TOPOLOGY_SCALER_ONLY = 0,
  XVPROCSS_TOPOLOGY_FULL_FLEDGED,
  XVPROCSS_TOPOLOGY_DEINTERLACE_ONLY,
  XVPROCSS_TOPOLOGY_CSC_ONLY,
  XVPROCSS_TOPOLOGY_VCRESAMPLE_ONLY,
  XVPROCSS_TOPOLOGY_HCRESAMPLE_ONLY,
  XVPROCSS_TOPOLOGY_NUM_SUPPORTED
}XVPROCSS_CONFIG_TOPOLOGY;

/**
 * This typedef enumerates types of Windows (Sub-frames) available in the
 * Subsystem
 */
typedef enum
{
  XVPROCSS_ZOOM_WIN = 0,
  XVPROCSS_PIP_WIN,
  XVPROCSS_PIXEL_WIN,
  XVPROCSS_WIN_NUM_SUPPORTED
}XVprocSs_Win;

/**
 * This typedef enumerates supported scaling modes
 */
typedef enum
{
  XVPROCSS_SCALE_1_1 = 0,
  XVPROCSS_SCALE_UP,
  XVPROCSS_SCALE_DN,
  XVPROCSS_SCALE_NOT_SUPPORTED
}XVprocSs_ScaleMode;

/** This typedef enumerates supported Color Channels
 *
 */
typedef enum
{
  XVPROCSS_COLOR_CH_Y_RED = 0,
  XVPROCSS_COLOR_CH_CB_GREEN,
  XVPROCSS_COLOR_CH_CR_BLUE,
  XVPROCSS_COLOR_CH_NUM_SUPPORTED
}XVprocSs_ColorChannel;

/**
 * Video Processing Subsystem context scratch pad memory.
 * This contains internal flags, state variables, routing table
 * and other meta-data required by the subsystem. Each instance
 * of the subsystem will have its own context data memory
 */
typedef struct
{
  XVidC_VideoWindow RdWindow; /**< window for Zoom/Pip feature support */
  XVidC_VideoWindow WrWindow; /**< window for Zoom/Pip feature support */

  UINTPTR DeintBufAddr;       /**< Deinterlacer field buffer Addr. in DDR */
  u8 PixelWidthInBits;        /**< Number of bits required to store 1 pixel */

  u8 RtngTable[XVPROCSS_SUBCORE_MAX]; /**< Storage for computed routing map */
  u8 StartCore[XVPROCSS_SUBCORE_MAX]; /**< Enable flag to start sub-core */
  u8 RtrNumCores;             /**< Number of sub-cores in routing map */
  u8 ScaleMode;               /**< Stored computed scaling mode - UP/DN/1:1 */
  u8 ZoomEn;                  /**< Flag to store Zoom feature state */
  u8 PipEn;                   /**< Flag to store PIP feature state */
  u16 VidInWidth;             /**< Input H Active */
  u16 VidInHeight;            /**< Input V Active */
  u16 PixelHStepSize;         /**< Increment step size for Pip/Zoom window */
  XVidC_ColorFormat StrmCformat; /**< processing pipe color format */
  XVidC_ColorFormat CscIn;    /**< CSC core input color format */
  XVidC_ColorFormat CscOut;   /**< CSC core output color format */
  XVidC_ColorFormat HcrIn;    /**< horiz. cresmplr core input color format */
  XVidC_ColorFormat HcrOut;   /**< horiz. cresmplr core output color format */
  XLboxColorId LboxBkgndColor; /**< Lbox background color */
}XVprocSs_ContextData;

/**
 * Sub-Core Configuration Table
 */
typedef struct
{
  u16 IsPresent;  /**< Flag to indicate if sub-core is present in the design*/
  u16 DeviceId;   /**< Device ID of the sub-core */
  u32 AddrOffset; /**< sub-core offset from subsystem base address */
}XSubCore;

/**
 * Video Processing Subsystem configuration structure.
 * Each subsystem device should have a configuration structure associated
 * that defines the MAX supported sub-cores within subsystem
 */

typedef struct
{
  u16 DeviceId;	         /**< DeviceId is the unique ID  of the device */
  UINTPTR BaseAddress;   /**< BaseAddress is the physical base address of the
                              subsystem address range */
  UINTPTR HighAddress;   /**< HighAddress is the physical MAX address of the
                              subsystem address range */
  u8 Topology;           /**< Subsystem configuration mode */
  u8 PixPerClock;        /**< Number of Pixels Per Clock processed by Subsystem */
  u16 ColorDepth;        /**< Processing precision of the data pipe */
  u16 NumVidComponents;  /**< Number of Video Components */
  u16 MaxWidth;          /**< Maximum cols supported by subsystem instance */
  u16 MaxHeight;         /**< Maximum rows supported by subsystem instance */
  u16 HasMADI;           /**< Motion Adaptive Deinterlacer available flag */
  XSubCore RstAximm;     /**< Axi MM reset network instance configuration */
  XSubCore RstAxis;      /**< Axi stream reset network instance configuration */
  XSubCore Vdma;         /**< Sub-core instance configuration */
  XSubCore Router;       /**< Sub-core instance configuration */
  XSubCore Csc;          /**< Sub-core instance configuration */
  XSubCore Deint;        /**< Sub-core instance configuration */
  XSubCore HCrsmplr;     /**< Sub-core instance configuration */
  XSubCore Hscale;       /**< Sub-core instance configuration */
  XSubCore Lbox;         /**< Sub-core instance configuration */
  XSubCore VCrsmplrIn;   /**< Sub-core instance configuration */
  XSubCore VCrsmplrOut;  /**< Sub-core instance configuration */
  XSubCore Vscale;       /**< Sub-core instance configuration */
} XVprocSs_Config;

/**
 * The XVprocSs driver instance data. The user is required to allocate a variable
 * of this type for every XVprocSs device in the system. A pointer to a variable
 * of this type is then passed to the driver API functions.
 */
typedef struct
{
  XVprocSs_Config Config;	         /**< Hardware configuration */
  u32 IsReady;		                 /**< Device and the driver instance are
                                       initialized */

  XAxis_Switch *RouterPtr;           /**< handle to sub-core driver instance */
  XGpio *RstAxisPtr;                 /**< handle to sub-core driver instance */
  XGpio *RstAximmPtr;                /**< handle to sub-core driver instance */

  XV_Hcresampler_l2 *HcrsmplrPtr;    /**< handle to sub-core driver instance */
  XV_Vcresampler_l2 *VcrsmplrInPtr;  /**< handle to sub-core driver instance */
  XV_Vcresampler_l2 *VcrsmplrOutPtr; /**< handle to sub-core driver instance */
  XV_Vscaler_l2 *VscalerPtr;         /**< handle to sub-core driver instance */
  XV_Hscaler_l2 *HscalerPtr;         /**< handle to sub-core driver instance */
  XAxiVdma *VdmaPtr;                 /**< handle to sub-core driver instance */
  XV_Lbox_l2 *LboxPtr;               /**< handle to sub-core driver instance */
  XV_Csc_l2 *CscPtr;                 /**< handle to sub-core driver instance */
  XV_Deint_l2 *DeintPtr;             /**< handle to sub-core driver instance */

  //I/O Streams
  XVidC_VideoStream VidIn;           /**< Input  AXIS configuration */
  XVidC_VideoStream VidOut;          /**< Output AXIS configuration */

  XVprocSs_ContextData CtxtData;     /**< Internal Scratch pad memory for subsystem
                                         instance */
  UINTPTR FrameBufBaseaddr;          /**< Base address for frame buffer storage */

  XVidC_DelayHandler UsrDelayUs;     /**< custom user function for delay/sleep */
  void *UsrTmrPtr;                   /**< handle to timer instance used by user
                                         delay function */

#ifdef XV_VPROCSS_LOG_ENABLE
  XVprocSs_Log Log;                  /**< A log of events. */
#endif
} XVprocSs;

/************************** Macros Definitions *******************************/
/*****************************************************************************/
/**
 * This macro returns the subsystem topology
 *
 * @param  XVprocSsPtr is a pointer to the Video Processing subsystem instance
 *
 * @return XVPROCSS_TOPOLOGY_FULL_FLEDGED or XVPROCSS_TOPOLOGY_SCALER_ONLY
 *
 *****************************************************************************/
#define XVprocSs_GetSubsystemTopology(XVprocSsPtr) \
   ((XVprocSsPtr)->Config.Topology)

/*****************************************************************************/
/**
 * This macro returns the subsystem Color Depth
 *
 * @param  XVprocSsPtr is a pointer to the Video Processing subsystem instance
 *
 * @return Color Depth
 *
 *****************************************************************************/
#define XVprocSs_GetColorDepth(XVprocSsPtr) ((XVprocSsPtr)->Config.ColorDepth)

/*****************************************************************************/
/**
 * This macro checks if subsystem is in Maximum (Full_Fledged) configuration
 *
 * @param  XVprocSsPtr is a pointer to the Video Processing subsystem instance
 *
 * @return Return 1 if condition is TRUE or 0 if FALSE
 *
 *****************************************************************************/
#define XVprocSs_IsConfigModeMax(XVprocSsPtr) \
   ((XVprocSsPtr)->Config.Topology == XVPROCSS_TOPOLOGY_FULL_FLEDGED)

/*****************************************************************************/
/**
 * This macro checks if subsystem configuration is in Scaler Only Mode
 *
 * @param  XVprocSsPtr is pointer to the Video Processing subsystem instance
 *
 * @return Returns 1 if condition is TRUE or 0 if FALSE
 *
 *****************************************************************************/
#define XVprocSs_IsConfigModeSscalerOnly(XVprocSsPtr)  \
   ((XVprocSsPtr)->Config.Topology == XVPROCSS_TOPOLOGY_SCALER_ONLY)

/*****************************************************************************/
/**
 * This macro checks if subsystem configuration is in Deinterlace Only Mode
 *
 * @param  XVprocSsPtr is pointer to the Video Processing subsystem instance
 *
 * @return Returns 1 if condition is TRUE or 0 if FALSE
 *
 *****************************************************************************/
#define XVprocSs_IsConfigModeDeinterlaceOnly(XVprocSsPtr)  \
   ((XVprocSsPtr)->Config.Topology == XVPROCSS_TOPOLOGY_DEINTERLACE_ONLY)

/*****************************************************************************/
/**
 * This macro checks if subsystem configuration is in CSC Only Mode
 *
 * @param  XVprocSsPtr is pointer to the Video Processing subsystem instance
 *
 * @return Returns 1 if condition is TRUE or 0 if FALSE
 *
 *****************************************************************************/
#define XVprocSs_IsConfigModeCscOnly(XVprocSsPtr)  \
   ((XVprocSsPtr)->Config.Topology == XVPROCSS_TOPOLOGY_CSC_ONLY)

/*****************************************************************************/
/**
 * This macro checks if subsystem configuration is in V Chroma ResampleMode
 *
 * @param  XVprocSsPtr is pointer to the Video Processing subsystem instance
 *
 * @return Returns 1 if condition is TRUE or 0 if FALSE
 *
 *****************************************************************************/
#define XVprocSs_IsConfigModeVCResampleOnly(XVprocSsPtr)  \
   ((XVprocSsPtr)->Config.Topology == XVPROCSS_TOPOLOGY_VCRESAMPLE_ONLY)

/*****************************************************************************/
/**
 * This macro checks if subsystem configuration is in H Chroma Resample Mode
 *
 * @param  XVprocSsPtr is pointer to the Video Processing subsystem instance
 *
 * @return Returns 1 if condition is TRUE or 0 if FALSE
 *
 *****************************************************************************/
#define XVprocSs_IsConfigModeHCResampleOnly(XVprocSsPtr)  \
   ((XVprocSsPtr)->Config.Topology == XVPROCSS_TOPOLOGY_HCRESAMPLE_ONLY)

/*****************************************************************************/
/**
 * This macro returns the current state of PIP Mode stored in subsystem internal
 * scratch pad memory
 *
 * @param  XVprocSsPtr is a pointer to the Video Processing subsystem instance
 *
 * @return Returns 1 if PIP mode is ON or 0 if OFF
 *
 *****************************************************************************/
#define XVprocSs_IsPipModeOn(XVprocSsPtr)       ((XVprocSsPtr)->CtxtData.PipEn)

/*****************************************************************************/
/**
 * This macro returns the current state of Zoom Mode stored in subsystem internal
 * scratch pad memory
 *
 * @param  XVprocSsPtr is a pointer to the Video Processing subsystem instance
 *
 * @return Returns 1 if ZOOM mode is ON or 0 if OFF
 *
 *****************************************************************************/
#define XVprocSs_IsZoomModeOn(XVprocSsPtr)     ((XVprocSsPtr)->CtxtData.ZoomEn)

/*****************************************************************************/
/**
 * This macro returns the Pip/Zoom window horizontal increment size
 *
 * @param  XVprocSsPtr is a pointer to the Video Processing subsystem instance
 *
 * @return Pixel H Step size
 *
 *****************************************************************************/
#define XVprocSs_GetPipZoomWinHStepSize(XVprocSsPtr) \
	                                   ((XVprocSsPtr)->CtxtData.PixelHStepSize)


/*****************************************************************************/
/**
 * This macro clears the PIP mode flag stored in subsystem internal scratch
 * pad memory. This call has no side-effect
 *
 * @param  XVprocSsPtr is a pointer to the Video Processing subsystem instance
 *
 * @return None
 *
 *****************************************************************************/
#define XVprocSs_ResetPipModeFlag(XVprocSsPtr) \
                                        ((XVprocSsPtr)->CtxtData.PipEn = FALSE)

/*****************************************************************************/
/**
 * This macro clears the ZOOM mode flag stored in subsystem internal scratch
 * pad memory. This call has no side-effect
 *
 * @param  XVprocSsPtr is pointer to the Video Processing subsystem instance
 *
 * @return None
 *
 *****************************************************************************/
#define XVprocSs_ResetZoomModeFlag(XVprocSsPtr)  \
                                       ((XVprocSsPtr)->CtxtData.ZoomEn = FALSE)

/*****************************************************************************/
/**
 * This macro sets the specified stream's color format. It can be used to
 * update input or output stream. This call has no side-effect in isolation.
 * For change to take effect user must trigger processing path reconfiguration
 * by calling XVprocSs_ConfigureSubsystem()
 *
 * @param  Stream is a pointer to the Subsystem Input or Output Stream
 * @param  ColorFormat is the requested color format
 *
 * @return None
 *
 *****************************************************************************/
#define XVprocSs_SetStreamColorFormat(Stream, ColorFormat) \
                                        ((Stream)->ColorFormatId = ColorFormat)

/*****************************************************************************/
/**
 * This macro sets the specified stream's color depth. It can be used to update
 * input or output stream. This call has no side-effect in isolation
 * For change to take effect user must trigger processing path reconfiguration
 * by calling XVprocSs_ConfigureSubsystem()
 *
 * @param  Stream is a pointer to the Subsystem Input or Output Stream
 * @param  ColorDepth is the requested color depth
 *
 * @return None
 *
 *****************************************************************************/
#define XVprocSs_SetStreamColorDepth(Stream, ColorDepth) \
                                            ((Stream)->ColorDepth = ColorDepth)


/************************** Function Prototypes ******************************/
/* Subsystem configuration and management functions */
int XVprocSs_CfgInitialize(XVprocSs *InstancePtr,
                           XVprocSs_Config *CfgPtr,
						   UINTPTR EffectiveAddr);
int XVprocSs_SetSubsystemConfig(XVprocSs *InstancePtr);
XVprocSs_Config* XVprocSs_LookupConfig(u32 DeviceId);

void XVprocSs_Start(XVprocSs *InstancePtr);
void XVprocSs_Stop(XVprocSs *InstancePtr);
void XVprocSs_Reset(XVprocSs *InstancePtr);

int XVprocSs_SetVidStreamIn(XVprocSs *InstancePtr,
                            const XVidC_VideoStream *StrmIn);
int XVprocSs_SetVidStreamOut(XVprocSs *InstancePtr,
                             const XVidC_VideoStream *StrmOut);
int XVprocSs_SetStreamResolution(XVidC_VideoStream *StreamPtr,
                                 const XVidC_VideoMode VmId,
                                 XVidC_VideoTiming const *Timing);
void XVprocSs_SetFrameBufBaseaddr(XVprocSs *InstancePtr, UINTPTR addr);

void XVprocSs_SetUserTimerHandler(XVprocSs *InstancePtr,
                                  XVidC_DelayHandler CallbackFunc,
                                  void *CallbackRef);

/* Zoom and PIP Control functions */
void XVprocSs_SetZoomMode(XVprocSs *InstancePtr, u8 OnOff);
void XVprocSs_SetPipMode(XVprocSs *InstancePtr, u8 OnOff);
void XVprocSs_SetZoomPipWindow(XVprocSs *InstancePtr,
                               XVprocSs_Win mode,
                               XVidC_VideoWindow *win);
void XVprocSs_GetZoomPipWindow(XVprocSs *InstancePtr,
                               XVprocSs_Win mode,
                               XVidC_VideoWindow *win);
void XVprocSs_UpdateZoomPipWindow(XVprocSs *InstancePtr);

/* Picture Control functions */
s32 XVprocSs_GetPictureBrightness(XVprocSs *InstancePtr);
void XVprocSs_SetPictureBrightness(XVprocSs *InstancePtr, s32 NewValue);
s32 XVprocSs_GetPictureContrast(XVprocSs *InstancePtr);
void XVprocSs_SetPictureContrast(XVprocSs *InstancePtr, s32 NewValue);
s32 XVprocSs_GetPictureSaturation(XVprocSs *InstancePtr);
void XVprocSs_SetPictureSaturation(XVprocSs *InstancePtr, s32 NewValue);
s32 XVprocSs_GetPictureGain(XVprocSs *InstancePtr,
                            XVprocSs_ColorChannel ChId);
void XVprocSs_SetPictureGain(XVprocSs *InstancePtr,
		                     XVprocSs_ColorChannel ChId,
		                     s32 NewValue);
XVidC_ColorStd XVprocSs_GetPictureColorStdIn(XVprocSs *InstancePtr);
void XVprocSs_SetPictureColorStdIn(XVprocSs *InstancePtr,
	                               XVidC_ColorStd NewVal);
XVidC_ColorStd XVprocSs_GetPictureColorStdOut(XVprocSs *InstancePtr);
void XVprocSs_SetPictureColorStdOut(XVprocSs *InstancePtr,
	                                XVidC_ColorStd NewVal);
XVidC_ColorRange XVprocSs_GetPictureColorRange(XVprocSs *InstancePtr);
void XVprocSs_SetPictureColorRange(XVprocSs *InstancePtr,
	                               XVidC_ColorRange NewVal);
int XVprocSs_SetPictureDemoWindow(XVprocSs *InstancePtr,
	                              XVidC_VideoWindow *Win);
void XVprocSs_SetPIPBackgroundColor(XVprocSs *InstancePtr,
		                            XLboxColorId ColorId);

/* External Filter Load functions */
void XVprocSs_LoadScalerCoeff(XVprocSs *InstancePtr,
		                      u32 CoreId,
                              u16 num_phases,
                              u16 num_taps,
                              const short *Coeff);

void XVprocSs_LoadChromaResamplerCoeff(XVprocSs *InstancePtr,
		                               u32 CoreId,
                                       u16 num_taps,
                                       const short *Coeff);

/* Debug functions */
void XVprocSs_ReportSubsystemConfig(XVprocSs *InstancePtr);
void XVprocSs_ReportSubsystemCoreInfo(XVprocSs *InstancePtr);
void XVprocSs_ReportSubcoreStatus(XVprocSs *InstancePtr,
		                          u32 SubcoreId);

/* Event Logging functions. */
void XVprocSs_LogReset(XVprocSs *InstancePtr);
u16  XVprocSs_LogRead(XVprocSs *InstancePtr);
void XVprocSs_LogDisplay(XVprocSs *InstancePtr);
#ifdef XV_VPROCSS_LOG_ENABLE
void XVprocSs_LogWrite(XVprocSs *InstancePtr, XVprocSs_LogEvent Evt, u8 Data);
#else
#define XVprocSs_LogWrite(...)
#endif

#ifdef __cplusplus
}
#endif

#endif /* end of protection macro */
/** @} */
