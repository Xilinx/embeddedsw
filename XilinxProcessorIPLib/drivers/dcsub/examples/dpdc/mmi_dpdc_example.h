
#ifndef __MMI_DPDC_EXAMPLE_H__
#define __MMI_DPDC_EXAMPLE_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "xdc.h"
#include "xdcdma.h"
#include "xdcsub.h"
#include "xmmidp.h"

#define APP_DEFAULT_WIDTH       1920
#define APP_DEFAULT_HEIGHT      1080
#define APP_DEFAULT_FPS         60
#define APP_DEFAULT_NL_PIXFMT   RGBA8888
#define APP_DEFAULT_LIVE_PIXFMT RGB_8BPC
#define APP_DEFAULT_DC_OUT_FMT  RGB_8BPC

typedef struct {
        u64 Address;
        u32 Size;
        u32 Stride;
        u32 LineSize;
        u32 Width;
        u32 Height;
        u32 Bpc;
        XDc_VideoFormat VideoFormat;
} FrameInfo;

typedef struct {
        u8 bpc;         /* 6,8,10,12 */
        u8 pattern;
        u8 pix_fmt;     /* 0 - rgb, 1 - yuv422 */
        u8 colorimetry; /* 0 - 601 1 - 709 */
        u8 ppc;         /* 0 - dual pixel, 1 - quad pixel */
} AvpgRunConfig;

typedef struct {

        XDcSub *DcSubPtr;
        XMmiDp *DpPsuPtr;

        AvpgRunConfig avpg[4];

        XDcDma_Descriptor *Desc1;
        XDcDma_Descriptor *Desc2;
        XDcDma_Descriptor *Desc3;
        XDcDma_Descriptor *Desc4;
        XDcDma_Descriptor *Desc5;
        XDcDma_Descriptor *Desc6;
        XDcDma_Descriptor *Desc7;

        FrameInfo *V1_FbInfo;
        FrameInfo *V2_FbInfo;
        FrameInfo *V3_FbInfo;
        FrameInfo *V4_FbInfo;
        FrameInfo *V5_FbInfo;
        FrameInfo *V6_FbInfo;
        FrameInfo *Cursor_FbInfo;
        FrameInfo *Sdp_FbInfo;

        u32 Width;
        u32 Height;
        XVidC_VideoMode VideoMode;
        u64 PixelClkHz;

        u8 PPC;

        XDc_VideoFormat Stream1Format;
        XDc_VideoFormat Stream2Format;
        XDc_VideoFormat OutStreamFormat;
        XDc_CursorBlend CursorEnable;
        XDc_AudEn AudioEnable;
        u8 AudioChannels;
        u8 SdpEnable;
        XDc_PartialBlendEn Stream1PbEnable;
        XDc_PartialBlendEn Stream2PbEnable;

        u32 CursorCoordX;
        u32 CursorCoordY;
        u32 CursorSizeX;
        u32 CursorSizeY;

        /* Partial Plane Blend parameters */
        u32 PpbCoordX;
        u32 PpbCoordY;
        u32 PpbSizeX;
        u32 PpbSizeY;
        u32 PpbOffsetX;
        u32 PpbOffsetY;

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

        /* 0=auto, 1, 2, or 4 */
        u8 MaxLaneCount;
        /* 0=auto, 6=RBR, 10=HBR, 20=HBR2, 30=HBR3 (DPCD BW values) */
        u8 MaxLinkRate;
} RunConfig;

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

#ifdef __cplusplus
}
#endif

#endif /* __MMI_DPDC_EXAMPLE_H__ */