/******************************************************************************
 *
 * Copyright (C) 2017-2018 Xilinx, Inc.  All rights reserved.
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
* @file xv_frmbufwr_l2.h
* @addtogroup v_frmbuf_wr_v2_0
* @{
* @details
*
* This header file contains layer 2 API's of the frame buffer write core driver.
* The functions contained herein provides a high level implementation of
* features provided by the IP, abstracting away the register level details from
* the user
*
* <b>Frame Buffer Write IP Features </b>
*
* The Frame Buffer Write IP supports following features
*   - AXI4-Stream Input
*   - 1, 2, or 4 pixel-wide video interface
*   - 8/10 bits per component
*   - Up to 16 different memory color formats (user configurable)
*
* <b>Dependency</b>
*
* This driver makes use of the video enumerations and data types defined in the
* Xilinx Video Common Driver (video_common_vX.x) and as such the common driver
* must be included as dependency to compile this driver

* <b>Initialization & Configuration</b>
*
* The device driver enables higher layer software (e.g., an application) to
* communicate with the frame buffer write core.
*
* Driver is built with layered architecture
*   - Layer 1 provides API's to peek/poke registers at HW level.
*   - Layer 2 provides API's that abstract sub-core functionality, providing an
*     easy to use feature interface
*
* Before using the layer-2 API's user must initialize the core by calling
* API XVFrmbufWr_Initialize(). This function will look for a configuration
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
*     interrupt controller and connect the XVFrmbufWr_InterruptHandler function
*     to service interrupts. Next interrupts must be enabled using the provided
*     API. When an interrupt occurs, ISR will confirm if frame processing is
*     is done/ready. If call back is registered such function will be called and
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
*                        Add new memory formats BGRX8 and UYVY8
* 3.00  vyc   04/04/18   Add interlaced support
*                        Add new memory format BGR8
*                        Add interrupt handler for ap_ready
* </pre>
*
******************************************************************************/
#ifndef XV_FRMBUFWR_L2_H     /* prevent circular inclusions */
#define XV_FRMBUFWR_L2_H     /* by using protection macros */

#ifdef __cplusplus
extern "C" {
#endif

#include "xvidc.h"
#include "xv_frmbufwr.h"

/************************** Constant Definitions *****************************/
#define XVFRMBUFWR_IRQ_DONE_MASK            (0x01)
#define XVFRMBUFWR_IRQ_READY_MASK           (0x02)

/**************************** Type Definitions *******************************/

/****************** Frame Buffer Write status 4096 - 4100  *******************/
typedef enum {
  XVFRMBUFWR_ERR_FRAME_SIZE_INVALID       = 0x1000L,
  XVFRMBUFWR_ERR_STRIDE_MISALIGNED        = 0x1001L,
  XVFRMBUFWR_ERR_MEM_ADDR_MISALIGNED      = 0x1002L,
  XVFRMBUFWR_ERR_VIDEO_FORMAT_MISMATCH    = 0x1003L,
  XVFRMBUFWR_ERR_DISABLED_IN_HW           = 0x1004L,
  XVFRMBUFWR_ERR_LAST
}XVFrmbufWr_ErrorCodes;

/**
* These constants specify different types of handler and used to differentiate
* interrupt requests from peripheral.
*/
typedef enum {
  XVFRMBUFWR_HANDLER_DONE = 1,  /**< Handler for ap_done */
  XVFRMBUFWR_HANDLER_READY      /**< Handler for ap_ready */
} XVFrmbufWr_HandlerType;
/*@}*/

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
typedef void (*XVFrmbufWr_Callback)(void *CallbackRef);

/**
 * Frame Buffer Write driver Layer 2 data. The user is required to allocate a
 * variable of this type for every frame buffer write device in the system. A
 * pointer to a variable of this type is then passed to the driver API functions.
 */
typedef struct {
    XV_frmbufwr FrmbufWr;            /**< Frame Buffer Write instance data */

    /*Callbacks */
    XVFrmbufWr_Callback FrameDoneCallback; /**< Callback for
                                                frame processing done */
    void *CallbackDoneRef;     /**< To be passed to the connect interrupt
                                callback */
    XVFrmbufWr_Callback FrameReadyCallback; /**< Callback for
                                                frame processing ready */
    void *CallbackReadyRef;     /**< To be passed to the connect interrupt
                                callback */

    XVidC_VideoStream Stream;    /**< Input AXIS */
}XV_FrmbufWr_l2;

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
#define XVFrmbufWr_IsRGBX8Enabled(InstancePtr) \
                                 ((InstancePtr)->FrmbufWr.Config.RGBX8En)

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
#define XVFrmbufWr_IsYUVX8Enabled(InstancePtr) \
                                 ((InstancePtr)->FrmbufWr.Config.YUVX8En)

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
#define XVFrmbufWr_IsYUYV8Enabled(InstancePtr) \
                                 ((InstancePtr)->FrmbufWr.Config.YUYV8En)

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
#define XVFrmbufWr_IsRGBX10Enabled(InstancePtr) \
                                  ((InstancePtr)->FrmbufWr.Config.RGBX10En)

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
#define XVFrmbufWr_IsYUVX10Enabled(InstancePtr) \
                                  ((InstancePtr)->FrmbufWr.Config.YUVX10En)

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
#define XVFrmbufWr_IsY_UV8Enabled(InstancePtr) \
                                 ((InstancePtr)->FrmbufWr.Config.Y_UV8En)

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
#define XVFrmbufWr_IsY_UV8_420Enabled(InstancePtr) \
                                    ((InstancePtr)->FrmbufWr.Config.Y_UV8_420En)

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
#define XVFrmbufWr_IsRGB8Enabled(InstancePtr) \
                                ((InstancePtr)->FrmbufWr.Config.RGB8En)

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
#define XVFrmbufWr_IsYUV8Enabled(InstancePtr) \
                                ((InstancePtr)->FrmbufWr.Config.YUV8En)

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
#define XVFrmbufWr_IsY_UV10Enabled(InstancePtr) \
                                  ((InstancePtr)->FrmbufWr.Config.Y_UV10En)

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
#define XVFrmbufWr_IsY_UV10_420Enabled(InstancePtr) \
                                   ((InstancePtr)->FrmbufWr.Config.Y_UV10_420En)

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
#define XVFrmbufWr_IsY8Enabled(InstancePtr) \
                              ((InstancePtr)->FrmbufWr.Config.Y8En)

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
#define XVFrmbufWr_IsY10Enabled(InstancePtr) \
                               ((InstancePtr)->FrmbufWr.Config.Y10En)

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
#define XVFrmbufWr_IsBGRX8Enabled(InstancePtr) \
                                 ((InstancePtr)->FrmbufWr.Config.BGRX8En)

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
#define XVFrmbufWr_IsUYVY8Enabled(InstancePtr) \
                                 ((InstancePtr)->FrmbufWr.Config.UYVY8En)

/*****************************************************************************/
/**
*
* This macro returns if Video Format BGR8 is available
*
* @param    InstancePtr is a pointer to the core instance.
*
* @return   Enabled(1)/Disabled(0)
*
* @note     None.
*
******************************************************************************/
#define XVFrmbufWr_IsBGR8Enabled(InstancePtr) \
                                 ((InstancePtr)->FrmbufWr.Config.BGR8En)

/*****************************************************************************/
/**
*
* This macro returns if interlaced support is available
*
* @param    InstancePtr is a pointer to the core instance.
*
* @return   Enabled(1)/Disabled(0)
*
* @note     None.
*
******************************************************************************/
#define XVFrmbufWr_InterlacedEnabled(InstancePtr) \
                                     ((InstancePtr)->FrmbufWr.Config.Interlaced)

/**************************** Function Prototypes *****************************/
int XVFrmbufWr_Initialize(XV_FrmbufWr_l2 *InstancePtr, u16 DeviceId);
void XVFrmbufWr_Start(XV_FrmbufWr_l2 *InstancePtr);
int XVFrmbufWr_Stop(XV_FrmbufWr_l2 *InstancePtr);
int XVFrmbufWr_WaitForIdle(XV_FrmbufWr_l2 *InstancePtr);
int XVFrmbufWr_SetMemFormat(XV_FrmbufWr_l2 *InstancePtr,
                            u32 StrideInBytes,
                            XVidC_ColorFormat MemFmt,
                            const XVidC_VideoStream *StrmIn);
XVidC_VideoStream *XVFrmbufWr_GetVideoStream(XV_FrmbufWr_l2 *InstancePtr);
int XVFrmbufWr_SetBufferAddr(XV_FrmbufWr_l2 *InstancePtr,
                             UINTPTR Addr);
UINTPTR XVFrmbufWr_GetBufferAddr(XV_FrmbufWr_l2 *InstancePtr);
int XVFrmbufWr_SetChromaBufferAddr(XV_FrmbufWr_l2 *InstancePtr,
                              UINTPTR Addr);
UINTPTR XVFrmbufWr_GetChromaBufferAddr(XV_FrmbufWr_l2 *InstancePtr);
u32 XVFrmbufWr_GetFieldID(XV_FrmbufWr_l2 *InstancePtr);
void XVFrmbufWr_DbgReportStatus(XV_FrmbufWr_l2 *InstancePtr);

/* Interrupt related function */
void XVFrmbufWr_InterruptHandler(void *InstancePtr);
int XVFrmbufWr_SetCallback(XV_FrmbufWr_l2 *InstancePtr,
                           u32 HandlerType,
                           void *CallbackFunc,
                           void *CallbackRef);
void XVFrmbufWr_InterruptEnable(XV_FrmbufWr_l2 *InstancePtr, u32 IrqMask);
void XVFrmbufWr_InterruptDisable(XV_FrmbufWr_l2 *InstancePtr, u32 IrqMask);

#ifdef __cplusplus
}
#endif
#endif
/** @} */
