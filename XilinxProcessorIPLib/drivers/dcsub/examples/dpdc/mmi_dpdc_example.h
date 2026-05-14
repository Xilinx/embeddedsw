
/******************************************************************************
* Copyright (c) 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file mmi_dpdc_example.h
*
* This header file contains type definitions, constants, and external
* declarations for the DisplayPort Display Controller example application.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver  Who     Date      Changes
* ---- ---     --------  --------------------------------------------------
* 1.00 arm     04/02/26  Initial version
*
* </pre>
*
******************************************************************************/
#ifndef __MMI_DPDC_EXAMPLE_H__
#define __MMI_DPDC_EXAMPLE_H__	/**< Prevent circular inclusions
				  *  by using protection macros */

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/

#include "xdc.h"
#include "xdcdma.h"
#include "xdcsub.h"
#include "xmmidp.h"
#include "xparameters.h"

/************************** Constant Definitions *****************************/

#define APP_DEFAULT_WIDTH       1920
#define APP_DEFAULT_HEIGHT      1080
#define APP_DEFAULT_FPS         60
#define APP_DEFAULT_NL_PIXFMT   RGBA8888
#define APP_DEFAULT_LIVE_PIXFMT RGB_8BPC
#define APP_DEFAULT_DC_OUT_FMT  RGB_8BPC

/**************************** Type Definitions *******************************/

/**
* This typedef contains the frame buffer information.
*/
typedef struct {
        u64 Address;		/**< Frame buffer base address */
        u32 Size;		/**< Frame buffer size in bytes */
        u32 Stride;		/**< Stride in bytes */
        u32 LineSize;		/**< Line size in bytes */
        u32 Width;		/**< Width in pixels */
        u32 Height;		/**< Height in pixels */
        u32 Bpc;		/**< Bits per component */
        XDc_VideoFormat VideoFormat;	/**< Video format */
} FrameInfo;

/**
* This typedef contains the AV Pattern Generator run configuration.
*/
typedef struct {
        u8 bpc;			/**< Bits per component: 6, 8, 10, 12 */
        u8 pattern;		/**< Test pattern selection */
        u8 pix_fmt;		/**< Pixel format: 0=RGB, 1=YUV422 */
        u8 colorimetry;		/**< Colorimetry: 0=BT.601, 1=BT.709 */
        u8 ppc;			/**< Pixels per clock: 0=dual, 1=quad */
} AvpgRunConfig;

/**
* This typedef contains the run-time configuration for the DisplayPort
* Display Controller example application.
*/
typedef struct {

        XDcSub *DcSubPtr;		/**< Pointer to DcSub instance */
        XMmiDp *DpPsuPtr;		/**< Pointer to DP instance */

        AvpgRunConfig avpg[4];		/**< AV pattern generator configs */

        XDcDma_Descriptor *Desc1;	/**< DMA descriptor 1 */
        XDcDma_Descriptor *Desc2;	/**< DMA descriptor 2 */
        XDcDma_Descriptor *Desc3;	/**< DMA descriptor 3 */
        XDcDma_Descriptor *Desc4;	/**< DMA descriptor 4 */
        XDcDma_Descriptor *Desc5;	/**< DMA descriptor 5 */
        XDcDma_Descriptor *Desc6;	/**< DMA descriptor 6 */
        XDcDma_Descriptor *Desc7;	/**< DMA descriptor 7 */

        FrameInfo *V1_FbInfo;		/**< Video stream 1 frame info */
        FrameInfo *V2_FbInfo;		/**< Video stream 2 frame info */
        FrameInfo *V3_FbInfo;		/**< Video stream 3 frame info */
        FrameInfo *V4_FbInfo;		/**< Video stream 4 frame info */
        FrameInfo *V5_FbInfo;		/**< Video stream 5 frame info */
        FrameInfo *V6_FbInfo;		/**< Video stream 6 frame info */
        FrameInfo *Cursor_FbInfo;	/**< Cursor frame info */
        FrameInfo *Sdp_FbInfo;		/**< SDP frame info */

        u32 Width;			/**< Active width in pixels */
        u32 Height;			/**< Active height in pixels */
        XVidC_VideoMode VideoMode;	/**< Video mode */
        u64 PixelClkHz;			/**< Pixel clock in Hz */

        u8 PPC;				/**< Pixels per clock */

        XDc_VideoFormat Stream1Format;	/**< Stream 1 video format */
        XDc_VideoFormat Stream2Format;	/**< Stream 2 video format */
        XDc_VideoFormat OutStreamFormat;	/**< Output stream format */
        XDc_CursorBlend CursorEnable;	/**< Cursor enable */
        XDc_AudEn AudioEnable;		/**< Audio enable */
        u8 AudioChannels;		/**< Number of audio channels */
        u8 SdpEnable;			/**< SDP enable */
        XDc_PartialBlendEn Stream1PbEnable; /**< Stream 1 partial blend */
        XDc_PartialBlendEn Stream2PbEnable; /**< Stream 2 partial blend */

        u32 CursorCoordX;		/**< Cursor X coordinate */
        u32 CursorCoordY;		/**< Cursor Y coordinate */
        u32 CursorSizeX;		/**< Cursor width */
        u32 CursorSizeY;		/**< Cursor height */

        u32 PpbCoordX;			/**< Partial blend X coordinate */
        u32 PpbCoordY;			/**< Partial blend Y coordinate */
        u32 PpbSizeX;			/**< Partial blend width */
        u32 PpbSizeY;			/**< Partial blend height */
        u32 PpbOffsetX;			/**< Partial blend X offset */
        u32 PpbOffsetY;			/**< Partial blend Y offset */

        u8 operatingmode;		/**< 0=DC_Functional, 1=DC_Bypass */
        u8 presentationmode;		/**< 0=Non-live, 1=Live, 2=Mixed */
        u8 livevidselect;		/**< 0=V01, 1=V02, 2=Both */
        u8 livevideo01mode;		/**< 0=Audio & Video, 1=Video only */
        u8 livevideo02mode;		/**< 1=Video only */
        u8 livevideoalphaen;		/**< 0=Disabled, 1=Enabled */
        u8 livevideosdpen;		/**< 0=Disabled, 1=Enabled */
        u8 byp_streams;			/**< Number of bypass streams */
        u8 byp_stream_pix_mode[4];	/**< Bypass stream PPC: 1, 2, 4 */
        u8 byp_stream_sdp_en[4];	/**< Bypass stream SDP enable */
        u8 byp_stream_vid_mode[4];	/**< Bypass stream video mode */

        u8 MaxLaneCount;		/**< 0=auto, 1, 2, or 4 */
        u8 MaxLinkRate;			/**< 0=auto, 6=RBR, 10=HBR,
					  *  20=HBR2, 30=HBR3 */
} RunConfig;

/***************** Macros (Inline Functions) Definitions *********************/


/************************** Variable Definitions *****************************/

extern RunConfig RunCfg;
extern XDcSub DcSub;
extern XDcDma DcDma;
extern XDc Dc;
extern XMmiDp DpPsuPtr;

/* SDTV CSC Coefficients */
extern u32 CSCCoeff_RGB[];
extern u32 CSCOffset_RGB[];

/* YUV-to-RGB input conversion (SDTV BT.601) */
extern u32 In_CSCCoeff_YUV[];
extern u32 In_CSCOffset_YUV[];

/* RGB-to-YUV output conversion (SDTV BT.601) */
extern u32 Out_CSCCoeff_YUV[];
extern u32 Out_CSCOffset_YUV[];

/************************** Function Prototypes ******************************/


#ifdef __cplusplus
}
#endif

#endif /* __MMI_DPDC_EXAMPLE_H__ */