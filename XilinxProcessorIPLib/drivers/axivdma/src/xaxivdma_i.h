/******************************************************************************
* Copyright (C) 2012 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2022 - 2026 Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

#ifndef XAXIVDMA_I_H_    /* prevent circular inclusions */
/** @brief Include guard for the AXI VDMA internal header. */
#define XAXIVDMA_I_H_

/*****************************************************************************/
/**
 *  @file xaxivdma_i.h
* @addtogroup axivdma Overview
* @{
 *
 * @brief Declares internal API definitions shared by driver files.
 *
 * Internal API definitions shared by driver files.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- -------------------------------------------------------
 * 1.00a jz   08/18/10 First release
 * 2.00a jz   12/10/10 Added support for direct register access mode, v3 core
 * 2.01a jz   01/19/11 Added ability to re-assign BD addresses
 *		       Replaced include xenv.h with string.h in xaxivdma_i.h
 * 		       file.
 *  	 rkv  03/28/11 Added support for frame store register.
 * 3.00a srt  08/26/11 Added support for Flush on Frame Sync and dynamic
 *		       programming of Line Buffer Thresholds.
 * 4.00a srt  11/21/11 Added support for 32 Frame Stores and modified bit
 *		       mask of Park Offset Register.
 *		       Added support for GenLock & Fsync Source Selection.
 * 4.03a srt  01/18/13 -  Added StreamWidth parameter to XAxiVdma_Channel
 *		       structure (CR 691866).
 * 4.04a srt  03/03/13 Support for the GenlockRepeat Control bit (Bit 15)
 *                     added in the new version of IP v5.04 (CR: 691391)
 *		       Support for *_ENABLE_DEBUG_INFO_* debug configuration
 *		       parameters (CR: 703738)
 * 6.6  rsp  07/02/18  Add vertical flip states in config structures
 *
 * </pre>
 *
 *****************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

#include <string.h>        /* memset */
#include "xil_types.h"
#include "xdebug.h"

/************************** Constant Definitions *****************************/
/**************************** Type Definitions *******************************/
/* Buffer Descriptor (BD) is only visible in this file
 */
typedef u32 XAxiVdma_Bd[XAXIVDMA_BD_MINIMUM_ALIGNMENT_WD];

/* The DMA channel is only visible to driver files
 */
typedef struct {
	UINTPTR ChanBase;       /**< Base address for this channel. */
	UINTPTR InstanceBase;   /**< Base address for the whole device. */
	UINTPTR StartAddrBase;  /**< Start address register array base. */

	int IsValid;        /**< Whether the channel has been initialized. */
	int FlushonFsync;	/**< Flushes VDMA transactions and resets channel state on frame sync. */
	int HasSG;          /**< Whether hardware has SG engine. */
	int IsRead;         /**< Read or write channel. */
	int HasDRE;         /**< Whether unaligned transfer is supported. */
	int LineBufDepth;	/**< Depth of channel line buffer FIFO. */
	int LineBufThreshold;	/**< Threshold where channel line almost-empty flag asserts. */
	int WordLength;     /**< Word length. */
	int NumFrames;	/**< Number of frames to work on. */

	UINTPTR HeadBdPhysAddr; /**< Physical address of the first BD. */
	UINTPTR HeadBdAddr;     /**< Virtual address of the first BD. */
	UINTPTR TailBdPhysAddr; /**< Physical address of the last BD. */
	UINTPTR TailBdAddr;     /**< Virtual address of the last BD. */
	int Hsize;          /**< Horizontal size. */
	int Vsize;          /**< Vertical size saved for non-SG mode hardware start. */

	int AllCnt;         /**< Total number of BDs. */

	int GenLock;	/**< MM2S genlock mode. */
	int S2MmSOF;	/**< S2MM start-of-frame flag. */
	int StreamWidth;     /**< Stream width. */
	XAxiVdma_Bd BDs[XAXIVDMA_MAX_FRAMESTORE] __attribute__((__aligned__(32)));
	/**< Statically allocated buffer descriptors. */
	u32 DbgFeatureFlags; /**< Debug parameter flags. */
	int AddrWidth; /**< Address width. */
	int direction;	/**< Determines whether read or write channel. */
	u8 HasVFlip;  /**< Whether hardware has vertical flip enabled. */
} XAxiVdma_Channel;

/* Duplicate layout of XAxiVdma_DmaSetup
 *
 * So to remove the dependency on xaxivdma.h
 */
typedef struct {
	int VertSizeInput;      /**< Vertical size input */
	int HoriSizeInput;      /**< Horizontal size input */
	int Stride;             /**< Stride */
	int FrameDelay;         /**< Frame Delay */

	int EnableCircularBuf;  /**< Circular Buffer Mode? */
	int EnableSync;         /**< Gen-Lock Mode? */
	int PointNum;           /**< Master we synchronize with */
	int EnableFrameCounter; /**< Frame Counter Enable */
	UINTPTR FrameStoreStartAddr[XAXIVDMA_MAX_FRAMESTORE];
	/**< Start Addresses of Frame Store Buffers. */
	int FixedFrameStoreAddr;/**< Fixed Frame Store Address index */
	int GenLockRepeat;      /**< Gen-Lock Repeat? */
	u8 EnableVFlip;	    /**< Vertical Flip state */
} XAxiVdma_ChannelSetup;

/************************** Function Prototypes ******************************/
/* Channel API
 */
void XAxiVdma_ChannelInit(XAxiVdma_Channel *Channel);
void XAxiVdma_ChannelReset(XAxiVdma_Channel *Channel);
int XAxiVdma_ChannelResetNotDone(XAxiVdma_Channel *Channel);
int XAxiVdma_ChannelIsRunning(XAxiVdma_Channel *Channel);
int XAxiVdma_ChannelIsBusy(XAxiVdma_Channel *Channel);
u32 XAxiVdma_ChannelGetStatus(XAxiVdma_Channel *Channel);
void XAxiVdma_ChannelRegisterDump(XAxiVdma_Channel *Channel);

int XAxiVdma_ChannelStartParking(XAxiVdma_Channel *Channel);
void XAxiVdma_ChannelStopParking(XAxiVdma_Channel *Channel);
void XAxiVdma_ChannelStartFrmCntEnable(XAxiVdma_Channel *Channel);

void XAxiVdma_ChannelEnableIntr(XAxiVdma_Channel *Channel, u32 IntrType);
void XAxiVdma_ChannelDisableIntr(XAxiVdma_Channel *Channel, u32 IntrType);
u32 XAxiVdma_ChannelGetPendingIntr(XAxiVdma_Channel *Channel);
u32 XAxiVdma_ChannelGetEnabledIntr(XAxiVdma_Channel *Channel);
void XAxiVdma_ChannelIntrClear(XAxiVdma_Channel *Channel, u32 IntrType);
int XAxiVdma_ChannelStartTransfer(XAxiVdma_Channel *Channel,
				  XAxiVdma_ChannelSetup *ChannelCfgPtr);
int XAxiVdma_ChannelSetBdAddrs(XAxiVdma_Channel *Channel, UINTPTR BdAddrPhys,
			       UINTPTR BdAddrVirt);
int XAxiVdma_ChannelConfig(XAxiVdma_Channel *Channel,
			   XAxiVdma_ChannelSetup *ChannelCfgPtr);
int XAxiVdma_ChannelSetBufferAddr(XAxiVdma_Channel *Channel, UINTPTR *AddrSet,
				  int NumFrames);
int XAxiVdma_ChannelStart(XAxiVdma_Channel *Channel);
void XAxiVdma_ChannelStop(XAxiVdma_Channel *Channel);
int XAxiVdma_ChannelSetFrmCnt(XAxiVdma_Channel *Channel, u8 FrmCnt,
			      u8 DlyCnt);
void XAxiVdma_ChannelGetFrmCnt(XAxiVdma_Channel *Channel, u8 *FrmCnt,
			       u8 *DlyCnt);
u32 XAxiVdma_ChannelErrors(XAxiVdma_Channel *Channel);
void XAxiVdma_ClearChannelErrors(XAxiVdma_Channel *Channel, u32 ErrorMask);
#ifdef __cplusplus
}
#endif

/** @} */
#endif
