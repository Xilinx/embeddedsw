/******************************************************************************
* Copyright (c) 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file mmi_dpdc_menus.h
*
* This header file contains declarations for menu functions
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

#ifndef MMI_DPDC_MENUS_H_
#define MMI_DPDC_MENUS_H_	/**< Prevent circular inclusions
				  *  by using protection macros */

/***************************** Include Files *********************************/

#include "xil_types.h"
#include "xstatus.h"
#include "xmmidp.h"
#include "mmi_dpdc_example.h"

/************************** Constant Definitions *****************************/


/**************************** Type Definitions *******************************/

/**
* This typedef contains the initial user configuration collected from the
* interactive menu before starting the DisplayPort test.
*/
/* InitRunConfig structure */
typedef struct {

    AvpgRunConfig avpg[4];		/**< AV pattern generator configs */

    u32 width;				/**< Horizontal resolution */
    u32 height;				/**< Vertical resolution */
    u32 frame_rate;			/**< Frame rate */
    u32 stream1_format;			/**< Stream 1 pixel format */
    u32 stream2_format;			/**< Stream 2 pixel format */
    u32 output_format;			/**< Output pixel format */
    u8 cursor_enable;			/**< Cursor enable */
    u8 audio_enable;			/**< Audio enable */
    u8 audio_channels;			/**< Number of audio channels */
    u8 sdp_enable;			/**< SDP enable */
    u8 partial_plane_blend_enable;	/**< Partial plane blend enable */
    u8 ppb_stream_select;		/**< PPB stream: 1=Stream1, 2=Stream2 */
    u32 cursor_coord_x;			/**< Cursor X coordinate */
    u32 cursor_coord_y;			/**< Cursor Y coordinate */
    u32 cursor_size_x;			/**< Cursor width */
    u32 cursor_size_y;			/**< Cursor height */
    u32 ppb_coord_x;			/**< Partial blend X coordinate */
    u32 ppb_coord_y;			/**< Partial blend Y coordinate */
    u32 ppb_size_x;			/**< Partial blend width */
    u32 ppb_size_y;			/**< Partial blend height */
    u32 ppb_offset_x;			/**< Partial blend X offset */
    u32 ppb_offset_y;			/**< Partial blend Y offset */
    u8 lane_count;			/**< 0=auto (sink max), 1, 2, or 4 */
    u8 link_rate;			/**< 0=auto (sink max), 6=RBR,
					  *  10=HBR, 20=HBR2, 30=HBR3 */
    u8 power_cycle_on_start;		/**< Power cycle on start */

    u8 operatingmode;			/**< 0=DC_Functional, 1=DC_Bypass */
    u8 presentationmode;		/**< 0=Non-live, 1=Live, 2=Mixed */
    u8 livevidselect;			/**< 0=V01, 1=V02, 2=Both 3=None */
    u8 livevideo01mode;			/**< 0=Audio & Video, 1=Video only, 2=None */
    u8 livevideo02mode;			/**< 1=Video only 2=None */
    u8 livevideoalphaen;		/**< 0=Disabled, 1=Enabled */
    u8 livevideosdpen;			/**< 0=Disabled, 1=Enabled */
    u8 byp_streams;			/**< Number of bypass streams */
    u8 byp_stream_pix_mode[4];		/**< Bypass stream PPC: 1, 2, 4 */
    u8 byp_stream_sdp_en[4];		/**< Bypass stream SDP enable */
    u8 byp_stream_vid_mode[4];		/**< Bypass stream video mode */
} InitRunConfig;

/************************** Function Prototypes ******************************/

/* Function prototypes */
void XDpDc_ResolutionHelpMenu(void);
void XDpDc_FormatHelpMenu(void);
void XDpDc_MainHelpMenu(InitRunConfig *config);
void XDpDc_ListNonliveFormats(void);
void XDpDc_ListLiveFormats(void);
void XDpDc_DisplayConfig(InitRunConfig *config);
u32 XDpDc_GetWidth(void);
u32 XDpDc_GetHeight(void);
u32 XDpDc_ConfigureResolution(InitRunConfig *config);
u32 XDpDc_ConfigureFormat(u32 *format, const char *format_name);
u32 XDpDc_ConfigureOutputFormat(u32 *format, const char *format_name);
u32 XDpDc_ConfigureCursor(InitRunConfig *config);
u32 XDpDc_ConfigurePartialPlane(InitRunConfig *config);
void XDpDc_MenuLoop(InitRunConfig *config);
void XDpDc_InitConfigDefaults(InitRunConfig *config);
void XDpDc_AuxMenu(XMmiDp *DpPtr);

#endif /* MMI_DPDC_MENUS_H_ */
