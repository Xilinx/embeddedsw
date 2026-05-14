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
******************************************************************************/

#ifndef MMI_DPDC_MENUS_H_
#define MMI_DPDC_MENUS_H_

#include "xil_types.h"
#include "xstatus.h"
#include "xmmidp.h"
#include "mmi_dpdc_example.h"


/* InitRunConfig structure */
typedef struct {

    AvpgRunConfig avpg[4];

    u32 width;
    u32 height;
    u32 frame_rate;
    u32 stream1_format;
    u32 stream2_format;
    u32 output_format;
    u8 cursor_enable;
    u8 audio_enable;
    u8 audio_channels;
    u8 sdp_enable;
    u8 partial_plane_blend_enable;
    /* 1=Stream1, 2=Stream2 */
    u8 ppb_stream_select;
    u32 cursor_coord_x;
    u32 cursor_coord_y;
    u32 cursor_size_x;
    u32 cursor_size_y;
    /* Partial Plane Blend parameters */
    u32 ppb_coord_x;
    u32 ppb_coord_y;
    u32 ppb_size_x;
    u32 ppb_size_y;
    u32 ppb_offset_x;
    u32 ppb_offset_y;
    /* 0=auto (sink max), 1, 2, or 4 */
    u8 lane_count;
    /* 0=auto (sink max), 6=RBR, 10=HBR, 20=HBR2, 30=HBR3 */
    u8 link_rate;
    u8 power_cycle_on_start;

    /* 0 - DC_Functional, 1 - DC_Bypass */
    u8 operatingmode;
    /* 0 - Non-live, 1 - Live, 2 - Mixed */
    u8 presentationmode;
    /* 0 - V01, 1 - V02, 2 - Both */
    u8 livevidselect;
    /* 0 - Audio & Video, 1 - Video only */
    u8 livevideo01mode;
    /* 1 - Video only */
    u8 livevideo02mode;
    /* 0 - Disabled , 1 - Enabled */
    u8 livevideoalphaen;
    /* 0 - Disabled , 1 - Enabled */
    u8 livevideosdpen;
    /* no of streams input to DC in bypass mode */
    u8 byp_streams;
    /* 1, 2, 4 ppc */
    u8 byp_stream_pix_mode[4];
    /* 0 - Disabled , 1 - Enabled */
    u8 byp_stream_sdp_en[4];
     /* 0 - Audio & Video, 1 - Video only */
    u8 byp_stream_vid_mode[4];
} InitRunConfig;

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
