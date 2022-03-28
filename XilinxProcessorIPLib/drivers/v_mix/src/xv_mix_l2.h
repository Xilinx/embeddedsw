/******************************************************************************
* Copyright (C) 1986 - 2022 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xv_mix_l2.h
* @addtogroup v_mix_v6_3
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
* 6.00  pg    01/10/20   Add Colorimetry Feature
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

#define XVMIX_CSC_COEFF_FRACTIONAL_BITS	(12)
#define XVMIX_CSC_COEFF_DIVISOR	(10000)
#define XVMIX_CSC_MAX_ROWS		(3)
#define XVMIX_CSC_MAX_COLS		(3)
#define XVMIX_CSC_MATRIX_SIZE	(XVMIX_CSC_MAX_ROWS * XVMIX_CSC_MAX_COLS)
#define XVMIX_CSC_COEFF_SIZE		(12)

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


/*
* These are CSC coefficient tables for Colorimetry with magic numbers
* and range are derived from the following references:
* [1] Rec. ITU-R BT.601-6
* [2] Rec. ITU-R BT.709-5
* [3] Rec. ITU-R BT.2020
* [4] http://en.wikipedia.org/wiki/YCbCr
* Coefficient table supports BT601 / BT709 / BT2020 encoding schemes
* and 16-235 / 16-240 / 0-255 range.
* These are defined in signed interger format and divided with 10000
* to get fractional values.
*/

static const s32
xv_mix_yuv2rgb_coeffs[XVMIX_CSC_MAX_ROWS][XVMIX_CSC_MAX_COLS][XVMIX_CSC_COEFF_SIZE] = {

	{
	 { 10000, 0, 14426, 10000, -1609, -5589, 10000, 18406, 0, -185, 92, -236 },
	 { 10233, 0, 14754, 10233, -1646, -5716, 10233, 18824, 0, -189, 94, -241 },
	 { 11644, 0, 16787, 11644, -1873, -6504, 11644, 21418, 0, -234, 89, -293 } },

	{
	 { 10000, 0, 15406, 10000, -1832, -4579, 10000, 18153, 0, -197, 82, -232 },
	 { 10233, 0, 15756, 10233, -1873, -4683, 10233, 18566, 0, -202, 84, -238 },
	 { 11644, 0, 17927, 11644, -2132, -5329, 11644, 21124, 0, -248, 77, -289 } },

	{
	 { 10000, 0, 13669, 10000, -3367, -6986, 10000, 17335, 0, -175, 132, -222 },
	 { 10479, 0, 13979, 10479, -3443, -7145, 10479, 17729, 0, -179, 136, -227 },
	 { 11644, 0, 15906, 11644, -3918, -8130, 11644, 20172, 0, -223, 136, -277 } }
};


static const s32
xv_mix_rgb2yuv_coeffs[XVMIX_CSC_MAX_ROWS][XVMIX_CSC_MAX_COLS][XVMIX_CSC_COEFF_SIZE] = {

	{
	 { 2625, 6775, 592, -1427, -3684, 5110, 5110, -4699, -410,  0, 128, 128 },
	 { 2566, 6625, 579, -1396, -3602, 4997, 4997, -4595, -401,  0, 128, 128 },
	 { 2256, 5823, 509, -1227, -3166, 4392, 4392, -4039, -353, 16, 128, 128 } },

	{
	 { 2120, 7150, 720, -1170, -3940, 5110, 5110, -4640, -470,  0, 128, 128 },
	 { 2077, 6988, 705, -1144, -3582, 4997, 4997, -4538, -458,  0, 128, 128 },
	 { 1826, 6142, 620, -1006, -3386, 4392, 4392, -3989, -403, 16, 128, 128 } },

	{
	 { 2990, 5870, 1440, -1720, -3390, 5110, 5110, -4280, -830,  0, 128, 128 },
	 { 2921, 5735, 1113, -1686, -3310, 4393, 4393, -4184, -812,  0, 128, 128 },
	 { 2568, 5041,  979, -1482, -2910, 4393, 4393, -3678, -714, 16, 128, 128 } }
};

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
 * This macro returns if CSC Coefficient Registers is enabled
 *
 * @param    InstancePtr is a pointer to the core instance.
 *
 * @return   Enabled(1)/Disabled(0)
 *
 * @note     None.
 *
 ******************************************************************************/
#define XVMix_IsCscCoeffsRegsEnabled(InstancePtr) \
	((InstancePtr)->Mix.Config.CscCoeffsRegsEn)


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
static void XVMix_SetCoeffForYuvToRgb(XV_Mix_l2 *InstancePtr,
		XVidC_ColorStd colorStandard,
		XVidC_ColorRange colorRange,
		u8 colorDepth);
static void XVMix_SetCoeffForRgbToYuv(XV_Mix_l2 *InstancePtr,
		XVidC_ColorStd ColorStd,
		XVidC_ColorRange colorRange,
		u8 colorDepth);
u32 XVMix_SetCscCoeffs(XV_Mix_l2 *InstancePtr,
		XVidC_ColorStd ColorStd,
		XVidC_ColorRange colorRange,
		u8 colorDepth);

#ifdef __cplusplus
}
#endif
#endif
/** @} */
