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
* @file xv_mix_l2.h
*
* This header file contains layer 2 API's of the mixer core driver.
* The functions contained herein provides a high level implementation of
* features provided by the IP, abstracting away the register level details from
* the user
*
* <b>Mixer IP Features </b>
*
* The Mixer IP supports following features
* 	- AXI4-S Input and Output
* 	- Upto 7 Memory layers
* 	- 1 Logo Layer (optional)
*   - Alpha Level (8 bit) per layer (optional)
*   - Upsample (1x, 2x, 4x) capability  per layer (optional)
*
* <b>Dependency</b>
*
* This driver makes use of the video enumerations and data types defined in the
* Xilinx Video Common Driver (video_common_vX.x) and as such the common driver
* must be included as dependency to compile this driver

* <b>Initialization & Configuration</b>
*
* The device driver enables higher layer software (e.g., an application) to
* communicate to the mixer core.
*
* Before using the layer-2 API's user must initialize the core by calling
* API XVMix_Initialize(). This function will look for a configuration structure
* for the device and initialize it to defined HW settings. It is recommended
* user always make use of Layer-2 API to interact with this core.
* Advanced users always have the capability to directly interact with the IP
* core using Layer-1 API's that perform low level register peek/poke.
*
* <b> Interrupts </b>
*
* For the driver to process interrupts, the application must set up the
* system's interrupt controller and connect the XVMix_InterruptHandler function
* to service interrupts. When an interrupt occurs, XVMix_InterruptHandler will
* check if ISR source is the Frame Done signal and will trigger next frame
* processing
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
* 1.00  rco   10/29/15   Initial Release
*             12/14/15   Added interrupt handler
* </pre>
*
******************************************************************************/
#ifndef XV_MIX_L2_H     /* prevent circular inclusions */
#define XV_MIX_L2_H     /* by using protection macros */

#ifdef __cplusplus
extern "C" {
#endif

#include "xvidc.h"
#include "xv_mix.h"

/************************** Constant Definitions *****************************/
#define XVMIX_MAX_SUPPORTED_LAYERS       (9)
#define XVMIX_ALPHA_MIN                  (0)
#define XVMIX_ALPHA_MAX                  (255)

#define XVMIX_IRQ_DONE_MASK              (0x01)
#define XVMIX_IRQ_READY_MASK             (0x02)

/**************************** Type Definitions *******************************/
/**
 * This typedef enumerates supported background colors
 */
typedef enum {
    XVMIX_BKGND_BLACK = 0,
    XVMIX_BKGND_WHITE,
    XVMIX_BKGND_RED,
    XVMIX_BKGND_GREEN,
    XVMIX_BKGND_BLUE,
    XVMIX_BKGND_LAST
}XVMix_BackgroundId;

/**
 * This typedef enumerates scale factors supported by mixer core
 */
typedef enum
{
    XVMIX_SCALE_FACTOR_1X = 0,
    XVMIX_SCALE_FACTOR_2X,
    XVMIX_SCALE_FACTOR_4X,
}XVMix_Scalefactor;

/**
 * This typedef enumerates layer index
 */
typedef enum {
    XVMIX_LAYER_STREAM = 0,
    XVMIX_LAYER_1,
    XVMIX_LAYER_2,
    XVMIX_LAYER_3,
    XVMIX_LAYER_4,
    XVMIX_LAYER_5,
    XVMIX_LAYER_6,
    XVMIX_LAYER_7,
    XVMIX_LAYER_LOGO,
    XVMIX_LAYER_ALL,
    XVMIX_LAYER_LAST
}XVMix_LayerId;

/**
 * This typedef contains configuration information for a given layer
 */
typedef struct {
    XVidC_VideoWindow Win;
    u8 IsEnabled;
    int ColorFormat;
    union {
        struct {
            u8 *RBuffer;
            u8 *GBuffer;
            u8 *BBuffer;
        };
        u32 BufAddr;
    };
}XVMix_Layer;

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
typedef void (*XVMix_Callback)(void *CallbackRef);

/**
 * Mixer driver Layer 2 data. The user is required to allocate a variable
 * of this type for every mixer device in the system. A pointer to a
 * variable of this type is then passed to the driver API functions.
 */
typedef struct {
    XV_mix Mix;            /**< Mixer Layer 1 instance data */

    /*Callbacks */
    XVMix_Callback FrameDoneCallback; /**< Callback for frame processing done */
    void *CallbackRef;	   /**< To be passed to the connect interrupt
                                callback */

    XVMix_Layer Layer[XVMIX_MAX_SUPPORTED_LAYERS];  /**< Layer configuration
                                                         structure */
    XVMix_BackgroundId BkgndColor;

    //I/O Streams
    XVidC_VideoStream StrmIn;      /**< Input  AXIS */
    XVidC_VideoStream StrmOut;     /**< Output AXIS */

}XV_Mix_l2;

/************************** Macros Definitions *******************************/
/*****************************************************************************/
/**
*
* This macro returns the current set background color id
*
* @param    InstancePtr is a pointer to the core instance.
*
* @return   Background color id
*
* @note     None.
*
******************************************************************************/
#define XVMix_GetBackgroundId(InstancePtr)         ((InstancePtr)->BkgndColor)

/*****************************************************************************/
/**
*
* This macro returns if alpha feature of specified layer is available
*
* @param    InstancePtr is a pointer to the core instance.
* @param    LayerId is the layer index for which information is requested
*
* @return   Enabled(1)/Disabled(0)
*
* @note     None.
*
******************************************************************************/
#define XVMix_IsAlphaEnabled(InstancePtr, LayerId) \
                ((InstancePtr)->Mix.Config.AlphaEn[LayerId-1])

/*****************************************************************************/
/**
*
* This macro returns if scaling feature of specified layer is available
*
* @param    InstancePtr is a pointer to the core instance.
* @param    LayerId is the layer index for which information is requested
*
* @return   Enabled(1)/Disabled(0)
*
* @note     None.
*
******************************************************************************/
#define XVMix_IsScalingEnabled(InstancePtr, LayerId) \
                ((InstancePtr)->Mix.Config.UpSampleEn[LayerId-1])


/*****************************************************************************/
/**
*
* This macro returns state of the specified layer [enabled or disabled]
*
* @param    InstancePtr is a pointer to the core instance.
* @param    LayerId is the layer index for which information is requested
*
* @return   Enabled(1)/Disabled(0)
*
******************************************************************************/
#define XVMix_IsLayerEnabled(InstancePtr, LayerId) \
                ((InstancePtr)->Layer[LayerId].IsEnabled)


/**************************** Function Prototypes *****************************/
int XVMix_Initialize(XV_Mix_l2 *InstancePtr, u16 DeviceId);
void XVMix_Start(XV_Mix_l2 *InstancePtr);
void XVMix_Stop(XV_Mix_l2 *InstancePtr);
void XVMix_SetVidStreamIn(XV_Mix_l2 *InstancePtr,
                          const XVidC_VideoStream *StrmIn);
void XVMix_SetVidStreamOut(XV_Mix_l2 *InstancePtr,
                           const XVidC_VideoStream *StrmOut);
int XVMix_LayerEnable(XV_Mix_l2 *InstancePtr, XVMix_LayerId LayerId);
int XVMix_LayerDisable(XV_Mix_l2 *InstancePtr, XVMix_LayerId LayerId);
void XVMix_SetResolution(XV_Mix_l2 *InstancePtr, u32 Width, u32 Height);
void XVMix_SetBackgndColor(XV_Mix_l2 *InstancePtr,
                           XVMix_BackgroundId ColorId,
                           XVidC_ColorDepth  bpc);
int XVMix_SetLayerColorFormat(XV_Mix_l2 *InstancePtr,
                              XVMix_LayerId LayerId,
                              XVidC_ColorFormat Cfmt);
int XVMix_GetLayerColorFormat(XV_Mix_l2 *InstancePtr,
                              XVMix_LayerId LayerId,
                              XVidC_ColorFormat *Cfmt);
int XVMix_SetLayerWindow(XV_Mix_l2 *InstancePtr,
                         XVMix_LayerId LayerId,
                         XVidC_VideoWindow *Win);
int XVMix_GetLayerWindow(XV_Mix_l2 *InstancePtr,
                         XVMix_LayerId LayerId,
                         XVidC_VideoWindow *Win);

int XVMix_SetLayerPosition(XV_Mix_l2 *InstancePtr,
                           XVMix_LayerId LayerId,
                           u16 StartX,
                           u16 StartY);
int XVMix_SetLayerScaleFactor(XV_Mix_l2 *InstancePtr,
                              XVMix_LayerId LayerId,
                              XVMix_Scalefactor Scale);
int XVMix_GetLayerScaleFactor(XV_Mix_l2 *InstancePtr, XVMix_LayerId LayerId);

int XVMix_SetLayerAlpha(XV_Mix_l2 *InstancePtr,
                        XVMix_LayerId LayerId,
                        u8 Alpha);
int XVMix_GetLayerAlpha(XV_Mix_l2 *InstancePtr, XVMix_LayerId LayerId);

int XVMix_SetLayerBufferAddr(XV_Mix_l2 *InstancePtr,
                             XVMix_LayerId LayerId,
                             u32 Addr);
u32 XVMix_GetLayerBufferAddr(XV_Mix_l2 *InstancePtr, XVMix_LayerId LayerId);

int XVMix_LoadLogo(XV_Mix_l2 *InstancePtr,
                   XVidC_VideoWindow *Win,
                   u8 *RBuffer,
                   u8 *GBuffer,
                   u8 *BBuffer);


void XVMix_DbgReportStatus(XV_Mix_l2 *InstancePtr);
void XVMix_DbgLayerInfo(XV_Mix_l2 *InstancePtr, XVMix_LayerId LayerId);

/* Interrupt related function */
void XVMix_InterruptHandler(void *InstancePtr);
int XVMix_SetCallback(XV_Mix_l2 *InstancePtr, void *CallbackFunc, void *CallbackRef);

#ifdef __cplusplus
}
#endif
#endif
/** @} */
