/******************************************************************************
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
 * Use of the Software is limited solely to applications:
 * (a) running on a Xilinx device, or
 * (b) that interact with a Xilinx device through a bus or interconnect.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * XILINX  BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
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
* @file xv_frmbufrd_l2.h
* @addtogroup v_frmbuf_rd
* @{
* @details
*
* This header file contains layer 2 API's of the frame buffer read core driver.
* The functions contained herein provides a high level implementation of
* features provided by the IP, abstracting away the register level details from
* the user
*
* <b>Frame Buffer Read IP Features </b>
*
* The Frame Buffer Read IP supports following features
*   - AXI4-Stream Output
*   - 1, 2, or 4 pixel-wide video interface
*   - 8/10 bits per component
*   - Up to 18 different memory color formats (user configurable)
*
* <b>Dependency</b>
*
* This driver makes use of the video enumerations and data types defined in the
* Xilinx Video Common Driver (video_common_vX.x) and as such the common driver
* must be included as dependency to compile this driver

* <b>Initialization & Configuration</b>
*
* The device driver enables higher layer software (e.g., an application) to
* communicate with the frame buffer read core.
*
* Driver is built with layered architecture
*   - Layer 1 provides API's to peek/poke registers at HW level.
*   - Layer 2 provides API's that abstract sub-core functionality, providing an
*     easy to use feature interface
*
* Before using the layer-2 API's user must initialize the core by calling
* API XVFrmbufRd_Initialize(). This function will look for a configuration
* structure for the device and initialize it to defined HW settings. It is
* recommended user always make use of Layer-2 API to interact with this core.
* Advanced users always have the capability to directly interact with the IP
* core using Layer-1 API's that perform low level register peek/poke.
*
* <b>Pre-Requisite's</b>
*
*   - Application must set the memory address using provided API
*     Address must be aligned to memory width. This can be computed with
*     following equation
*       Align = 2 * PPC * 4 Bytes
*       (where PPC is the Pixels/Clock selected in IP configuration)
*
*   - The Stride must be provided in Bytes and must be aligned to
*     memory width. This can be computed with following equation
*       StrideInBytes = 2 * PPC * 4 Bytes
*       (where PPC is the Pixels/Clock selected in IP configuration)
*
* <b> Interrupts </b>
*
* Driver is configured to operate both in polling as well as interrupt mode.
*   - To use interrupt based processing, application must set up the system's
*     interrupt controller and connect the XVFrmbufRd_InterruptHandler function
*     to service interrupts. Next interrupts must be enabled using the provided
*     API. When an interrupt occurs, ISR will confirm if frame processing is
*     is done. If call back is registered such function will be called and
*     application can apply new setting updates here. Subsequently next frame
*     processing will be triggered with new settings.
*   - To use polling method disable interrupts using the provided API. Doing so
*     will configure the IP to keep processing frames without sw intervention.
*   - Polling mode is the default configuration set during driver initialization
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
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who    Date     Changes
* ----- ---- -------- -------------------------------------------------------
* 1.00  vyc   04/05/17   Initial Release
* 2.00  vyc   10/04/17   Add second buffer pointer for semi-planar formats
*                        Add memory formats RGBA8, YUVA8, BGRA8, BGRX8, UYVY8
* </pre>
*
******************************************************************************/
#ifndef XV_FRMBUFRD_L2_H     /* prevent circular inclusions */
#define XV_FRMBUFRD_L2_H     /* by using protection macros */

#ifdef __cplusplus
extern "C" {
#endif

#include "xvidc.h"
#include "xv_frmbufrd.h"

/************************** Constant Definitions *****************************/
#define XVFRMBUFRD_IRQ_DONE_MASK              (0x01)
#define XVFRMBUFRD_IRQ_READY_MASK             (0x02)

/**************************** Type Definitions *******************************/

/****************** Frame Buffer Read status 4096 - 4100  ********************/
typedef enum {
  XVFRMBUFRD_ERR_FRAME_SIZE_INVALID       = 0x1000L,
  XVFRMBUFRD_ERR_STRIDE_MISALIGNED        = 0x1001L,
  XVFRMBUFRD_ERR_MEM_ADDR_MISALIGNED      = 0x1002L,
  XVFRMBUFRD_ERR_VIDEO_FORMAT_MISMATCH    = 0x1003L,
  XVFRMBUFRD_ERR_DISABLED_IN_HW           = 0x1004L,
  XVFRMBUFRD_ERR_LAST
}XVFrmbufRd_ErrorCodes;

/**
* Callback type for interrupt.
*
* @param    CallbackRef is a callback reference passed in by the upper
*           layer when setting the callback functions, and passed back to
*           the upper layer when the callback is invoked.
*
* @return   None.
*
* @note     None.
*
*/
typedef void (*XVFrmbufRd_Callback)(void *CallbackRef);

/**
 * Frame Buffer Read driver Layer 2 data. The user is required to allocate a
 * variable of this type for every frame buffer read device in the system. A
 * pointer to a variable of this type is then passed to the driver API functions.
 */
typedef struct {
    XV_frmbufrd FrmbufRd;            /**< Frame Buffer Read instance data */

    /*Callbacks */
    XVFrmbufRd_Callback FrameDoneCallback; /**< Callback for
                                                frame processing done */
    void *CallbackRef;     /**< To be passed to the connect interrupt
                                callback */

    XVidC_VideoStream Stream;    /**< Output AXIS */
}XV_FrmbufRd_l2;

/************************** Macros Definitions *******************************/
/*****************************************************************************/
/**
*
* This macro returns if Video Format RGBX8 is available
*
* @param    InstancePtr is a pointer to the core instance.
*
* @return   Enabled(1)/Disabled(0)
*
* @note     None.
*
******************************************************************************/
#define XVFrmbufRd_IsRGBX8Enabled(InstancePtr) \
                                 ((InstancePtr)->FrmbufRd.Config.RGBX8En)

/*****************************************************************************/
/**
*
* This macro returns if Video Format YUVX8 is available
*
* @param    InstancePtr is a pointer to the core instance.
*
* @return   Enabled(1)/Disabled(0)
*
* @note     None.
*
******************************************************************************/
#define XVFrmbufRd_IsYUVX8Enabled(InstancePtr) \
                                 ((InstancePtr)->FrmbufRd.Config.YUVX8En)

/*****************************************************************************/
/**
*
* This macro returns if Video Format YUYV8 is available
*
* @param    InstancePtr is a pointer to the core instance.
*
* @return   Enabled(1)/Disabled(0)
*
* @note     None.
*
******************************************************************************/
#define XVFrmbufRd_IsYUYV8Enabled(InstancePtr) \
                                 ((InstancePtr)->FrmbufRd.Config.YUYV8En)

/*****************************************************************************/
/**
*
* This macro returns if Video Format RGBA8 is available
*
* @param    InstancePtr is a pointer to the core instance.
*
* @return   Enabled(1)/Disabled(0)
*
* @note     None.
*
******************************************************************************/
#define XVFrmbufRd_IsRGBA8Enabled(InstancePtr) \
                                 ((InstancePtr)->FrmbufRd.Config.RGBA8En)

/*****************************************************************************/
/**
*
* This macro returns if Video Format YUVA8 is available
*
* @param    InstancePtr is a pointer to the core instance.
*
* @return   Enabled(1)/Disabled(0)
*
* @note     None.
*
******************************************************************************/
#define XVFrmbufRd_IsYUVA8Enabled(InstancePtr) \
                                 ((InstancePtr)->FrmbufRd.Config.YUVA8En)

/*****************************************************************************/
/**
*
* This macro returns if Video Format RGBX10 is available
*
* @param    InstancePtr is a pointer to the core instance.
*
* @return   Enabled(1)/Disabled(0)
*
* @note     None.
*
******************************************************************************/
#define XVFrmbufRd_IsRGBX10Enabled(InstancePtr) \
                                  ((InstancePtr)->FrmbufRd.Config.RGBX10En)

/*****************************************************************************/
/**
*
* This macro returns if Video Format YUVX10 is available
*
* @param    InstancePtr is a pointer to the core instance.
*
* @return   Enabled(1)/Disabled(0)
*
* @note     None.
*
******************************************************************************/
#define XVFrmbufRd_IsYUVX10Enabled(InstancePtr) \
                                  ((InstancePtr)->FrmbufRd.Config.YUVX10En)

/*****************************************************************************/
/**
*
* This macro returns if Video Format Y_UV8 is available
*
* @param    InstancePtr is a pointer to the core instance.
*
* @return   Enabled(1)/Disabled(0)
*
* @note     None.
*
******************************************************************************/
#define XVFrmbufRd_IsY_UV8Enabled(InstancePtr) \
                                 ((InstancePtr)->FrmbufRd.Config.Y_UV8En)

/*****************************************************************************/
/**
*
* This macro returns if Video Format Y_UV8_420 is available
*
* @param    InstancePtr is a pointer to the core instance.
*
* @return   Enabled(1)/Disabled(0)
*
* @note     None.
*
******************************************************************************/
#define XVFrmbufRd_IsY_UV8_420Enabled(InstancePtr) \
                                    ((InstancePtr)->FrmbufRd.Config.Y_UV8_420En)

/*****************************************************************************/
/**
*
* This macro returns if Video Format RGB8 is available
*
* @param    InstancePtr is a pointer to the core instance.
*
* @return   Enabled(1)/Disabled(0)
*
* @note     None.
*
******************************************************************************/
#define XVFrmbufRd_IsRGB8Enabled(InstancePtr) \
                                ((InstancePtr)->FrmbufRd.Config.RGB8En)

/*****************************************************************************/
/**
*
* This macro returns if Video Format YUV8 is available
*
* @param    InstancePtr is a pointer to the core instance.
*
* @return   Enabled(1)/Disabled(0)
*
* @note     None.
*
******************************************************************************/
#define XVFrmbufRd_IsYUV8Enabled(InstancePtr) \
                                ((InstancePtr)->FrmbufRd.Config.YUV8En)

/*****************************************************************************/
/**
*
* This macro returns if Video Format Y_UV10 is available
*
* @param    InstancePtr is a pointer to the core instance.
*
* @return   Enabled(1)/Disabled(0)
*
* @note     None.
*
******************************************************************************/
#define XVFrmbufRd_IsY_UV10Enabled(InstancePtr) \
                                  ((InstancePtr)->FrmbufRd.Config.Y_UV10En)

/*****************************************************************************/
/**
*
* This macro returns if Video Format Y_UV10_420 is available
*
* @param    InstancePtr is a pointer to the core instance.
*
* @return   Enabled(1)/Disabled(0)
*
* @note     None.
*
******************************************************************************/
#define XVFrmbufRd_IsY_UV10_420Enabled(InstancePtr) \
                                   ((InstancePtr)->FrmbufRd.Config.Y_UV10_420En)

/*****************************************************************************/
/**
*
* This macro returns if Video Format Y8 is available
*
* @param    InstancePtr is a pointer to the core instance.
*
* @return   Enabled(1)/Disabled(0)
*
* @note     None.
*
******************************************************************************/
#define XVFrmbufRd_IsY8Enabled(InstancePtr) \
                              ((InstancePtr)->FrmbufRd.Config.Y8En)

/*****************************************************************************/
/**
*
* This macro returns if Video Format Y10 is available
*
* @param    InstancePtr is a pointer to the core instance.
*
* @return   Enabled(1)/Disabled(0)
*
* @note     None.
*
******************************************************************************/
#define XVFrmbufRd_IsY10Enabled(InstancePtr) \
                               ((InstancePtr)->FrmbufRd.Config.Y10En)

/*****************************************************************************/
/**
*
* This macro returns if Video Format BGRA8 is available
*
* @param    InstancePtr is a pointer to the core instance.
*
* @return   Enabled(1)/Disabled(0)
*
* @note     None.
*
******************************************************************************/
#define XVFrmbufRd_IsBGRA8Enabled(InstancePtr) \
                                 ((InstancePtr)->FrmbufRd.Config.BGRA8En)

/*****************************************************************************/
/**
*
* This macro returns if Video Format BGRX8 is available
*
* @param    InstancePtr is a pointer to the core instance.
*
* @return   Enabled(1)/Disabled(0)
*
* @note     None.
*
******************************************************************************/
#define XVFrmbufRd_IsBGRX8Enabled(InstancePtr) \
                                 ((InstancePtr)->FrmbufRd.Config.BGRX8En)

/*****************************************************************************/
/**
*
* This macro returns if Video Format UYVY8 is available
*
* @param    InstancePtr is a pointer to the core instance.
*
* @return   Enabled(1)/Disabled(0)
*
* @note     None.
*
******************************************************************************/
#define XVFrmbufRd_IsUYVY8Enabled(InstancePtr) \
                                 ((InstancePtr)->FrmbufRd.Config.UYVY8En)

/**************************** Function Prototypes *****************************/
int XVFrmbufRd_Initialize(XV_FrmbufRd_l2 *InstancePtr, u16 DeviceId);
void XVFrmbufRd_Start(XV_FrmbufRd_l2 *InstancePtr);
int XVFrmbufRd_Stop(XV_FrmbufRd_l2 *InstancePtr);
int XVFrmbufRd_SetMemFormat(XV_FrmbufRd_l2 *InstancePtr,
                            u32 StrideInBytes,
                            XVidC_ColorFormat MemFmt,
                            const XVidC_VideoStream *StrmOut);
XVidC_VideoStream *XVFrmbufRd_GetVideoStream(XV_FrmbufRd_l2 *InstancePtr);
int XVFrmbufRd_SetBufferAddr(XV_FrmbufRd_l2 *InstancePtr,
                             UINTPTR Addr);
UINTPTR XVFrmbufRd_GetBufferAddr(XV_FrmbufRd_l2 *InstancePtr);
int XVFrmbufRd_SetChromaBufferAddr(XV_FrmbufRd_l2 *InstancePtr,
                              UINTPTR Addr);
UINTPTR XVFrmbufRd_GetChromaBufferAddr(XV_FrmbufRd_l2 *InstancePtr);
void XVFrmbufRd_DbgReportStatus(XV_FrmbufRd_l2 *InstancePtr);

/* Interrupt related function */
void XVFrmbufRd_InterruptHandler(void *InstancePtr);
int XVFrmbufRd_SetCallback(XV_FrmbufRd_l2 *InstancePtr,
                           void *CallbackFunc,
                           void *CallbackRef);
void XVFrmbufRd_InterruptEnable(XV_FrmbufRd_l2 *InstancePtr);
void XVFrmbufRd_InterruptDisable(XV_FrmbufRd_l2 *InstancePtr);

#ifdef __cplusplus
}
#endif
#endif
/** @} */
