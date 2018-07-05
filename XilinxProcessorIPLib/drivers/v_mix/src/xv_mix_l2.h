/******************************************************************************
 *
 * Copyright (C) 1986-2018 Xilinx, Inc.  All rights reserved.
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
******************************************************************************/
/*****************************************************************************/
/**
*
* @file xv_mix_l2.h
* @addtogroup v_mix_v3_0
* @{
* @details
*
* This header file contains layer 2 API's of the mixer core driver.
* The functions contained herein provides a high level implementation of
* features provided by the IP, abstracting away the register level details from
* the user
*
* <b>Mixer IP Features </b>
*
* The Mixer IP supports following features
*   - AXI4-S Master Layer
*   - Up to 8 optional layers (user configurable)
*   - Each layer can be configured as Streaming or Memory (build time)
*      - Color format for each layer is set at build time
*   - 1 Logo Layer (optional)
*   - Logo Layer Color Key feature (optional)
*   - Alpha Level (8 bit) per layer (optional)
*   - Scale (1x, 2x, 4x) capability per layer (optional)
*
* <b>Dependency</b>
*
* This driver makes use of the video enumerations and data types defined in the
* Xilinx Video Common Driver (video_common_vX.x) and as such the common driver
* must be included as dependency to compile this driver

* <b>Initialization & Configuration</b>
*
* The device driver enables higher layer software (e.g., an application) to
* communicate with the mixer core.
*
* Driver is built with layered architecture
*   - Layer 1 provides API's to peek/poke registers at HW level.
*   - Layer 2 provides API's that abstract sub-core functionality, providing an
*     easy to use feature interface
*
* Before using the layer-2 API's user must initialize the core by calling
* API XVMix_Initialize(). This function will look for a configuration structure
* for the device and initialize it to defined HW settings. It is recommended
* user always make use of Layer-2 API to interact with this core.
* Advanced users always have the capability to directly interact with the IP
* core using Layer-1 API's that perform low level register peek/poke.
*
* <b>Pre-Requisite's</b>
*
* If optional layers are included in the IP then
*   - Application must set the memory address for each layer using provided API
*     Address must be aligned to memory width. This can be computed with
*     following equation
*       Align = 2 * PPC * 4 Bytes
*       (where PPC is the Pixels/Clock selected in IP configuration)
*
*   - When setting up layer window the Stride must be provided in Bytes and
*     must be aligned to respective color space of the layer. This can be
*     computed with following equation
*       StrideInBytes = (Window_Width * (YUV422 ? 2 : 4))
*
* <b> Interrupts </b>
*
* Driver is configured to operate both in polling as well as interrupt mode.
*   - To use interrupt based processing, application must set up the system's
*     interrupt controller and connect the XVMix_InterruptHandler function to
*     service interrupts. Next interrupts must be enabled using the provided
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
* 1.00  rco   10/29/15   Initial Release
*             12/14/15   Added interrupt handler
*             02/12/16   Added Stride and memory Alignement requirements
*             02/25/16   Replace GetColorFromat function with a macro
*             03/08/16   Replace GetColorFromat macro with function and added
*                        master layer video format
* 2.00  rco   07/21/16   Used UINTPTR instead of u32 for Baseaddress
*             08/03/16   Add Logo Pixel Alpha support
* 3.00  vyc   10/04/17   Add second buffer pointer for semi-planar formats
* 4.00  vyc   04/04/18   Add 8th overlayer
*                        Move logo layer enable from bit 8 to bit 15
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
#define XVMIX_MAX_SUPPORTED_LAYERS       (16)
#define XVMIX_ALPHA_MIN                  (0)
#define XVMIX_ALPHA_MAX                  (256)

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
    XVMIX_SCALE_FACTOR_NUM_SUPPORTED
}XVMix_Scalefactor;

/**
 * This typedef enumerates layer index
 */
typedef enum {
    XVMIX_LAYER_MASTER = 0,
    XVMIX_LAYER_1,
    XVMIX_LAYER_2,
    XVMIX_LAYER_3,
    XVMIX_LAYER_4,
    XVMIX_LAYER_5,
    XVMIX_LAYER_6,
    XVMIX_LAYER_7,
    XVMIX_LAYER_8,
    XVMIX_LAYER_9,
    XVMIX_LAYER_10,
    XVMIX_LAYER_11,
    XVMIX_LAYER_12,
    XVMIX_LAYER_13,
    XVMIX_LAYER_14,
    XVMIX_LAYER_15,
    XVMIX_LAYER_16,
    XVMIX_LAYER_LOGO = 23,
    XVMIX_LAYER_ALL,
    XVMIX_LAYER_LAST
}XVMix_LayerId;

/**
 * This typedef enumerates layer interface type
 */
typedef enum {
  XVMIX_LAYER_TYPE_MEMORY = 0,
  XVMIX_LAYER_TYPE_STREAM
}XVMix_LayerType;

/****************** Mixer status 4096 - 4100  *****************************/
typedef enum {
  XVMIX_ERR_LAYER_WINDOW_INVALID     = 0x1000L,
  XVMIX_ERR_WIN_STRIDE_MISALIGNED    = 0x1001L,
  XVMIX_ERR_MEM_ADDR_MISALIGNED      = 0x1002L,
  XVMIX_ERR_LAYER_INTF_TYPE_MISMATCH = 0x1003L,
  XVMIX_ERR_DISABLED_IN_HW           = 0x1004L,
  XVMIX_ERR_LAST
}XVMix_ErrorCodes;

/**
 * This typedef contains configuration information for Logo Color Key
 */
typedef struct {
  u8 RGB_Min[3];
  u8 RGB_Max[3];
}XVMix_LogoColorKey;

/**
 * This typedef contains configuration information for a given layer
 */
typedef struct {
    XVidC_VideoWindow Win;
    XVidC_ColorFormat ColorFormat;
    union {
        struct {
            u8 *RBuffer;
            u8 *GBuffer;
            u8 *BBuffer;
        };
        UINTPTR BufAddr;
        UINTPTR ChromaBufAddr;
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
    void *CallbackRef;     /**< To be passed to the connect interrupt
                                callback */

    XVMix_Layer Layer[XVMIX_MAX_SUPPORTED_LAYERS];  /**< Layer configuration
                                                         structure */
    XVMix_BackgroundId BkgndColor;

    XVidC_VideoStream Stream;    /**< Input AXIS */
}XV_Mix_l2;

/************************** Macros Definitions *******************************/
/*****************************************************************************/
/**
*
* This macro returns the available layers in IP
*
* @param    InstancePtr is a pointer to the core instance.
*
* @return   Number of layers enabled in HW
*
* @note     None.
*
******************************************************************************/
#define XVMix_GetNumLayers(InstancePtr)  ((InstancePtr)->Mix.Config.NumLayers)

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
#define XVMix_GetBackgndColor(InstancePtr)        ((InstancePtr)->BkgndColor)

/*****************************************************************************/
/**
*
* This macro returns the interface type for the specified layer
*
* @param    InstancePtr is a pointer to the core instance.
* @param    LayerId is the layer index for which information is requested
*
* @return   Layer Interface Type
*
* @note     None.
*
******************************************************************************/
#define XVMix_GetLayerInterfaceType(InstancePtr, LayerId)  \
                      ((InstancePtr)->Mix.Config.LayerIntrfType[LayerId-1])

/*****************************************************************************/
/**
*
* This macro returns if Logo layer is enabled
*
* @param    InstancePtr is a pointer to the core instance.
*
* @return   Enabled(1)/Disabled(0)
*
* @note     None.
*
******************************************************************************/
#define XVMix_IsLogoEnabled(InstancePtr)   ((InstancePtr)->Mix.Config.LogoEn)


/*****************************************************************************/
/**
*
* This macro returns if Logo layer color key feature is enabled
*
* @param    InstancePtr is a pointer to the core instance.
*
* @return   Enabled(1)/Disabled(0)
*
* @note     None.
*
******************************************************************************/
#define XVMix_IsLogoColorKeyEnabled(InstancePtr) \
                              ((InstancePtr)->Mix.Config.LogoColorKeyEn)


/*****************************************************************************/
/**
*
* This macro returns if Logo layer alpha feature is enabled
*
* @param    InstancePtr is a pointer to the core instance.
*
* @return   Enabled(1)/Disabled(0)
*
* @note     None.
*
******************************************************************************/
#define XVMix_IsLogoPixAlphaEnabled(InstancePtr) \
                              ((InstancePtr)->Mix.Config.LogoPixAlphaEn)


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
                ((InstancePtr)->Mix.Config.ScalingEn[LayerId-1])


/*****************************************************************************/
/**
*
* This macro check if specified layer interface type is STREAM
*
* @param    InstancePtr is a pointer to the core instance.
* @param    LayerId is the layer index for which information is requested
*
* @return   TRUE(1)/FALSE(0)
*
******************************************************************************/
#define XVMix_IsLayerInterfaceStream(InstancePtr, LayerId) \
 ((InstancePtr)->Mix.Config.LayerIntrfType[LayerId-1] == XVMIX_LAYER_TYPE_STREAM)

/**************************** Function Prototypes *****************************/
int XVMix_Initialize(XV_Mix_l2 *InstancePtr, u16 DeviceId);
void XVMix_Start(XV_Mix_l2 *InstancePtr);
void XV_mix_SetFlushbit(XV_mix *InstancePtr);
u32 XV_mix_Get_FlushDone(XV_mix *InstancePtr);
void XVMix_Stop(XV_Mix_l2 *InstancePtr);
void XVMix_SetVidStream(XV_Mix_l2 *InstancePtr,
                        const XVidC_VideoStream *StrmIn);
int XVMix_LayerEnable(XV_Mix_l2 *InstancePtr, XVMix_LayerId LayerId);
int XVMix_LayerDisable(XV_Mix_l2 *InstancePtr, XVMix_LayerId LayerId);
int XVMix_IsLayerEnabled(XV_Mix_l2 *InstancePtr, XVMix_LayerId LayerId);
void XVMix_SetBackgndColor(XV_Mix_l2 *InstancePtr,
                           XVMix_BackgroundId ColorId,
                           XVidC_ColorDepth  bpc);
int XVMix_SetLayerWindow(XV_Mix_l2 *InstancePtr,
                         XVMix_LayerId LayerId,
                         XVidC_VideoWindow *Win,
                         u32 StrideInBytes);
int XVMix_GetLayerWindow(XV_Mix_l2 *InstancePtr,
                         XVMix_LayerId LayerId,
                         XVidC_VideoWindow *Win);

int XVMix_MoveLayerWindow(XV_Mix_l2 *InstancePtr,
                          XVMix_LayerId LayerId,
                          u16 StartX,
                          u16 StartY);
int XVMix_SetLayerScaleFactor(XV_Mix_l2 *InstancePtr,
                              XVMix_LayerId LayerId,
                              XVMix_Scalefactor Scale);
int XVMix_GetLayerScaleFactor(XV_Mix_l2 *InstancePtr, XVMix_LayerId LayerId);

int XVMix_SetLayerAlpha(XV_Mix_l2 *InstancePtr,
                        XVMix_LayerId LayerId,
                        u16 Alpha);
int XVMix_GetLayerAlpha(XV_Mix_l2 *InstancePtr, XVMix_LayerId LayerId);

int XVMix_SetLayerBufferAddr(XV_Mix_l2 *InstancePtr,
                             XVMix_LayerId LayerId,
                             UINTPTR Addr);
UINTPTR XVMix_GetLayerBufferAddr(XV_Mix_l2 *InstancePtr, XVMix_LayerId LayerId);

int XVMix_SetLayerChromaBufferAddr(XV_Mix_l2 *InstancePtr,
                                   XVMix_LayerId LayerId,
                                   UINTPTR Addr);
UINTPTR XVMix_GetLayerChromaBufferAddr(XV_Mix_l2 *InstancePtr,
                                       XVMix_LayerId LayerId);

int XVMix_SetLogoColorKey(XV_Mix_l2 *InstancePtr,
                          XVMix_LogoColorKey ColorKeyData);
int XVMix_GetLogoColorKey(XV_Mix_l2 *InstancePtr,
                          XVMix_LogoColorKey *ColorKeyData);
int XVMix_GetLayerColorFormat(XV_Mix_l2 *InstancePtr,
                              XVMix_LayerId LayerId,
                              XVidC_ColorFormat *Cfmt);

int XVMix_LoadLogo(XV_Mix_l2 *InstancePtr,
                   XVidC_VideoWindow *Win,
                   u8 *RBuffer,
                   u8 *GBuffer,
                   u8 *BBuffer);
int XVMix_LoadLogoPixelAlpha(XV_Mix_l2 *InstancePtr,
                             XVidC_VideoWindow *Win,
                             u8 *ABuffer);

void XVMix_DbgReportStatus(XV_Mix_l2 *InstancePtr);
void XVMix_DbgLayerInfo(XV_Mix_l2 *InstancePtr, XVMix_LayerId LayerId);

/* Interrupt related function */
void XVMix_InterruptHandler(void *InstancePtr);
int XVMix_SetCallback(XV_Mix_l2 *InstancePtr, void *CallbackFunc, void *CallbackRef);
void XVMix_InterruptEnable(XV_Mix_l2 *InstancePtr);
void XVMix_InterruptDisable(XV_Mix_l2 *InstancePtr);

#ifdef __cplusplus
}
#endif
#endif
/** @} */
