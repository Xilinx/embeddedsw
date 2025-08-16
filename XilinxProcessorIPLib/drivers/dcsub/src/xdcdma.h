/*******************************************************************************
* Copyright (C) 2025 Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/

/******************************************************************************/
/**
 *
 * @file xdcdma.h
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- -----------------------------------------------
 * 1.0   ck   03/14/25 Initial Release.
 * </pre>
 *
*******************************************************************************/

#ifndef __XDCDMA_H__
#define __XDCDMA_H__

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files **********************************/
#include "xdcdma_hw.h"
#include "xdc.h"

/************************** Constant Definitions ******************************/

#define XDCDMA_DESC_ALIGN       (256U)

#define XDCDMA_DESC_PREAMBLE    (0xA5)

#define XDCDMA_VIDEO_CHANNEL0       0U
#define XDCDMA_VIDEO_CHANNEL1       1U
#define XDCDMA_VIDEO_CHANNEL2       2U
#define XDCDMA_GRAPHICS_CHANNEL3    0U
#define XDCDMA_GRAPHICS_CHANNEL4    1U
#define XDCDMA_GRAPHICS_CHANNEL5    2U
#define XDCDMA_AUDIO_CHANNEL        6U
#define XDCDMA_SDP_CHANNEL          7U

#define XDCDMA_TRIGGER_DONE         0U
#define XDCDMA_RETRIGGER_DONE       0U
#define XDCDMA_TRIGGER_EN           1U
#define XDCDMA_RETRIGGER_EN         2U

/* descriptor definition */
typedef struct {
	u32 Id : 16; 		/**< Identifier */
	u32 Control_Lo : 16; 	/* [15:0] control bits */

	u32 Control_Hi : 16; 	/* [23:16] control bits */
	u32 Data_Size_Lo : 16;

	u32 Data_Size_Hi : 16;
	u32 Src_Addr_Lo : 16;

	u32 Src_Addr_Hi;

	u32 Next_Desc_Addr_Lo;

	u32 Next_Desc_Addr_Hi : 16;
	u32 TLB_prefetch_en : 1;
	u32 TLB_prefetch_blk_size : 14;
	u32 TLB_prefetch_blk_off_lo : 1;
	u32 TL_prefetch_blk_off_hi : 13;
	u32 Line_Tile : 1; 		/**< 0 - Line, 1 - Tile */
	u32 Line_Size : 18; 	/**< Bytes per line */

	u32 Line_Stride : 14; 	/**< Stride in 16 byte resolution */
	u32 Tile_Type : 1; 	/**< 0 - 32x4 1 - 64x4 */
	u32 Tile_Pitch : 14; 	/**< Addr between 2 rows of tiles in unit of 32 bytes. N/A for raster */
	u32 Target_Addr : 1; 	/**< 0 for DP / SDP, 1 - Cursor RAM */
	u32 Intr_Enable : 1;
	u32 Reserved_1 : 1;

	u32 Presentation_TS_Lo;

	u32 Presentation_TS_Hi;

	u32 Reserved_2;

	u32 Checksum;
} XDcDma_Descriptor __attribute__ ((aligned(XDCDMA_DESC_ALIGN))) __attribute__((packed));

typedef struct {
	u32 BaseAddr;
	u32 IntId;
} XDcDma_Config;

typedef void (*XDcDma_VSyncIntHandler)(void *InstancePtr);
typedef void (*XDcDma_DscrDoneIntHandler)(void *InstancePtr);
typedef void (*XDcDma_DscrErrIntHandler)(void *InstancePtr);
typedef void (*XDcDma_AxiErrIntHandler)(void *InstancePtr);
typedef void (*XDcDma_NoOStandErrIntHandler)(void *InstancePtr);
typedef void (*XDcDma_AxiRd4kErrIntHandler)(void *InstancePtr);
typedef void (*XDcDma_AddrDecodeErrIntHandler)(void *InstancePtr);
typedef void (*XDcDma_DscrRdWrErrIntHandler)(void *InstancePtr);
typedef void (*XDcDma_CrcErrIntHandler)(void *InstancePtr);
typedef void (*XDcDma_PreambleErrIntHandler)(void *InstancePtr);
typedef void (*XDcDma_ChanOvrFlowErrIntHandler)(void *InstancePtr);

typedef enum {
	/* 0 - 2 are Video */
	XDCDMA_CH0 = 0,
	XDCDMA_CH1 = 1,
	XDCDMA_CH2 = 2,
	/* 3 - 5 are Graphics */
	XDCDMA_CH3 = 3,
	XDCDMA_CH4 = 4,
	XDCDMA_CH5 = 5,
	/* 6 is audio */
	XDCDMA_CH6 = 6,
	/* 7 is SDP / Cursor RAM */
	XDCDMA_CH7 = 7,
} XDcDma_ChannelId;

typedef enum {
	XDCDMA_HANDLER_VSYNC = 0,
	XDCDMA_HANDLER_DSCR_ERR = 1,
	XDCDMA_HANDLER_AXI_ERR = 2,
	XDCDMA_HANDLER_NO_OSTAND_ERR = 3,
	XDCDMA_HANDLER_DSCR_DONE = 4,
	XDCDMA_HANDLER_AXI_RD_4K_ERR = 5,
	XDCDMA_HANDLER_ADDR_DECODE_ERR = 6,
	XDCDMA_HANDLER_DSCR_RD_WR_ERR = 7,
	XDCDMA_HANDLER_CRC_ERR = 8,
	XDCDMA_HANDLER_PREAMBLE_ERR = 9,
	XDCDMA_HANDLER_CHAN_OVR_FLOW_ERR = 10,
} XDcDma_IntHandlerType;

typedef enum {
	XDCDMA_STATE_IDLE,
	XDCDMA_STATE_ENABLE,
	XDCDMA_STATE_DISABLE,
	XDCDMA_STATE_PAUSE
} XDcDma_State;

/**
 * This data structure describes video planes.
 */
typedef enum {
	INTERLEAVED = 1,
	SEMIPLANAR = 2,
	PLANAR = 3,
} XDcDma_VideoModes;

/**
 * This typedef is the information needed to transfer video info.
 */
typedef struct {
	u64 Address;
	u32 Size;
	u32 Stride  __attribute__ ((aligned (256)));
	u32 LineSize;
} XDcDma_FrameBuffer;

/**
 * This typedef is the information needed to transfer Audio info.
 */
typedef struct {
	u64 Address;
	u32 Size;
} XDcDma_AudioBuffer;

/**
 * This typedef is the information needed to transfer SDP info.
 */
typedef struct {
	u64 Address;
	u32 Size;
} XDcDma_SDPBuffer;

/**
 * This typedef defines the Video/Graphics Channel attributes.
 */
typedef struct {
	XDcDma_Descriptor *Current;
} XDcDma_Channel_Dscr;

/**
 * This typedef defines the Video/Graphics Channel attributes.
 */
typedef struct {
	XDcDma_Channel_Dscr Channel[3];
	XDc_VideoAttribute *VideoInfo;
	XDcDma_FrameBuffer *FrameBuffer[3];
	u8 Video_TriggerStatus;
	u8 Graphics_TriggerStatus;
	XDcDma_State State;
} XDcDma_Channel;

/**
 * This typedef defines the Audio Channel attributes.
 */
typedef struct {
	XDcDma_Channel_Dscr Channel;
	XDcDma_AudioBuffer *Buffer;
	u8 Audio_TriggerStatus;
	XDcDma_State State;
	u8 DscrErrDis;
	u8 VidActvFetchEn;
} XDcDma_AudioChannel;

/**
 * This typedef defines the SDP Channel attributes.
 */
typedef struct {
	XDcDma_Channel_Dscr Channel;
	XDcDma_SDPBuffer *Buffer;
	u8 SDP_TriggerStatus;
	XDcDma_State State;
} XDcDma_SDPChannel;

/**
 * This typedef defines the Channel types.
 */
typedef enum {
	VideoChan = 0x0,
	GraphicsChan = 0x3,
	AudioChan = 0x6,
	CursorSDPChan = 0x7,
} XDcDma_ChannelType;

/**
 * This typedef defines the DCDMA configration.
 */
typedef struct {
	XDcDma_Config Config;
	u32 LastDescId; 		/**< Incremented on every get. Used to set ID in descriptors */
	u32 ChanTrigger;
	XDcDma_Channel Video;
	XDcDma_Channel Gfx;
	XDcDma_AudioChannel Audio;
	XDcDma_SDPChannel SDP;
	XDcDma_VSyncIntHandler  XDcDmaVsyncHandler;
	void *XDcDmaVsyncRef;
	XDcDma_DscrErrIntHandler    XDcDmaDscrErrHandler;
	void *XDcDmaDscrErrRef;
	XDcDma_AxiErrIntHandler    XDcDmaAxiErrHandler;
	void *XDcDmaAxiErrRef;
	XDcDma_NoOStandErrIntHandler    XDcDmaNoOStandErrHandler;
	void *XDcDmaNoOStandErrRef;
	XDcDma_DscrDoneIntHandler   XDcDmaDscrDoneHandler;
	void *XDcDmaDscrDoneRef;
	XDcDma_AxiRd4kErrIntHandler    XDcDmaAxiRd4kErrHandler;
	void *XDcDmaAxiRd4kErrRef;
	XDcDma_AddrDecodeErrIntHandler    XDcDmaAddrDecodeErrHandler;
	void *XDcDmaAddrDecodeErrRef;
	XDcDma_DscrRdWrErrIntHandler    XDcDmaDscrRdWrErrHandler;
	void *XDcDmaDscrRdWrErrRef;
	XDcDma_CrcErrIntHandler    XDcDmaCrcErrHandler;
	void *XDcDmaCrcErrRef;
	XDcDma_PreambleErrIntHandler    XDcDmaPreambleErrHandler;
	void *XDcDmaPreambleErrRef;
	XDcDma_ChanOvrFlowErrIntHandler    XDcDmaChanOvrFlowErrHandler;
	void *XDcDmaChanOvrFlowErrRef;
} XDcDma;

/**************************** Function Prototypes *****************************/
XDcDma_Config *XDcDma_LookupConfig(u32 DeviceId);
void XDcDma_CfgInitialize(XDcDma *InstancePtr, u32 EffectiveAddr);
void XDcDma_WriteProtEnable(XDcDma *InstancePtr);
void XDcDma_WriteProtDisable(XDcDma *InstancePtr);
u32 XDCDma_GetWriteProt(XDcDma *InstancePtr);
u32 XDcDma_SetCallBack(XDcDma *InstancePtr, XDcDma_IntHandlerType HandlerType,
		       void *CallbackFunc, void *CallbackRef);
u32 XDcDma_ConfigChannelState(XDcDma *InstancePtr, XDcDma_ChannelId Id, XDcDma_State Enable);
void XDcDma_SetupChannel(XDcDma *InstancePtr, XDcDma_ChannelType Channel);
u32 XDcDma_Trigger(XDcDma *InstancePtr, XDcDma_ChannelType Channel);
u32 XDcDma_ReTrigger(XDcDma *InstancePtr, XDcDma_ChannelType Channel);
void XDcDma_SetDataQoS(XDcDma *InstancePtr, XDcDma_ChannelType Channel, u8 Val);
void XDcDma_SetDescQoS(XDcDma *InstancePtr, XDcDma_ChannelType Channel, u8 Val);
void XDcDma_SetAxCache(XDcDma *InstancePtr, XDcDma_ChannelId Id, u8 Val);
void XDcDma_SetAxProt(XDcDma *InstancePtr, XDcDma_ChannelId Id, u8 Val);
void XDcDma_SetDescDelay(XDcDma *InstancePtr, XDcDma_ChannelId Id, u16 Val);
u32 XDCDma_GetOutstandingTxn(XDcDma *InstancePtr, XDcDma_ChannelId Id);
u32 XDcDma_WaitPendingTransaction(XDcDma *InstancePtr, XDcDma_ChannelId Id);
u32 XDcDma_GenerateChecksum(XDcDma_Descriptor *DescPtr);
void XDcDma_InterruptEnable(XDcDma *InstancePtr, u32 Mask);
void XDcDma_InterruptDisable(XDcDma *InstancePtr, u32 Mask);
void XDcDma_InterruptHandler(XDcDma *InstancePtr);
void XDcDma_VSyncHandler(XDcDma *InstancePtr);
void XDcDma_DescInit(XDcDma_Descriptor *XDesc);
u32 XDcDma_GetNewDescId(XDcDma *InstancePtr);
void XDcDma_SetAudioChCtrl(XDcDma *InstancePtr);

#ifdef __cplusplus
}
#endif

#endif /* __XDCDMA_H__ */
