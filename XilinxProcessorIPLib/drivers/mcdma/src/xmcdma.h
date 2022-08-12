/******************************************************************************
* Copyright (C) 2017 - 2022 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xmcdma.h
* @addtogroup mcdma_v1_7
* @{
* @details
*
* AXI MultiChannel Direct Memory Access (AXI MDMA) IP provides high-bandwidth
* Direct memory access between the AXI4 memory mapped and AXI4-Stream
* IP interfaces. Its scatter gather capabilities also offload data
* Movement tasks from the Central Processing Unit (CPU) in processor-based
* Systems.
*
* In AXI MCDMA Primary high-speed DMA data movement between system memory and
* Stream target is through the AXI4 Read Master to AXI4 memory-mapped to
* Stream (MM2S) Master, and AXI stream to memory-mapped (S2MM) Slave to AXI4
* Write Master. AXI DMA enables up to 16 multiple channels of data
* movement on both MM2S and S2MM paths.
*
* <b>Initialization & Configuration</b>
*
* The device driver enables higher layer software (e.g., an application) to
* communicate to the AXI MCDMA IP core.
*
* XMcDma_CfgInitialize() API is used to initialize the MCDMA core.
* The user needs to first call the XMcDma_LookupConfig() API which returns
* the Configuration structure pointer which is passed as a parameter to the
* XMcDma_CfgInitialize() API.
*
* <b> Interrupts </b>
* In single interrupt multiple channels case driver provides interrupt
* handler XMcdma_TxIntrHandler and XMcdma_IntrHandler for mm2s and s2mm
* side respectively for handling the interrupt from the MCDMA core.
* The users of this driver have to register this handler with the interrupt
* system and provide the callback functions by using XMcdma_SetCallBack  API.
*
* <b>Buffer Descriptors(BD) management </b>
*
* BD is shared by the software and the hardware. To use BD for SG DMA
* transfers, the application needs to use the driver API to do the following:
*
* - Setup the BD's for the Channel:
*      - XMcDma_ChanBdCreate(...)
*
* - Submit a DMA transfer for the required length.
*      - XMcDma_ChanSubmit(...)
*
* - Submit all prepared BDs to the hardware:
*      - XMcDma_ChantoHw(...)
*
* - Upon transfer completion, the application can request completed BDs from
*   the hardware:
*      - XMcdma_BdChainFromHW(...)
*
* - After the application has finished using the BDs, it should free the
*   BDs back to the free pool:
*      - XMcdma_BdChainFree(...)
*
* The driver also provides API functions to get the status of a completed
* BD, along with get functions for other fields in the BD.
*
* The following diagram shows the correct flow of BDs:
*
* The diagram shows a complete cycle for BDs, starting from
* requesting the BDs to freeing the BDs.
* <pre>
*
*         XMcDma_ChanSubmit()                   XMcDma_ChanToHw()
* Free ------------------------> Pre-process ----------------------> Hardware
*                                                                    |
*  /|\                                                               |
*   |    XMcdma_BdChainFree()                 XMcdma_BdChainFromHW() |
*   +--------------------------- Post-process <----------------------+
*
* </pre>
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
* <b> Asserts </b>
*
* Asserts are used within all Xilinx drivers to enforce constraints on argument
* values. Asserts can be turned off on a system-wide basis by defining, at
* compile time, the NDEBUG identifier. By default, asserts are turned on and it
* is recommended that users leave asserts on during development.
*
* <b> Building the driver </b>
*
* The AXI MCDMA driver is composed of several source files. This allows the user
* to build and link only those parts of the driver that are necessary.
*
* This header file contains identifiers and register-level driver functions (or
* macros), range macros, structure typedefs that can be used to access the
* Xilinx AXI MCDMA core instance.
*
* MODIFICATION HISTORY:
*
* Ver   Who     Date     Changes
* ----- ------  -------- ------------------------------------------------------
* 1.0   adk 	18/07/17 Initial version.
* 1.0   adk     09/02/18 Fixed CR#994435 Changes are made in the
*			 driver tcl file.
* 1.2   mj      05/03/18 Exported APIs XMcdma_BdChainFree() and
*                        XMcDma_BdSetAppWord().
* 1.2   mus     11/05/18 Support 64 bit DMA addresses for Microblaze-X platform.
* 1.3   rsp     02/12/19 Add HasRxLength field in config and channel structure.
* 1.3   rsp     02/11/19 Add top level submit XMcDma_Chan_Sideband_Submit() API
*                        to program BD control and sideband information.
* 1.5	sk	07/13/20 Add XMcDma_BdGetAppWord() function declaration to fix
* 			 the gcc warning in mcdma integration test suite.
* 1.7   sa      08/12/22 Updated the examples to use latest MIG cannoical define
* 		         i.e XPAR_MIG_0_C0_DDR4_MEMORY_MAP_BASEADDR.
******************************************************************************/
#ifndef XMCDMA_H_
#define XMCDMA_H_

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/

#include "xmcdma_hw.h"
#include "xmcdma_bd.h"
#include "xil_assert.h"
#include "xstatus.h"
#include "xil_cache.h"

/************************** Constant Definitions *****************************/

#define XMCDMA_LOOP_COUNT		1000000
#define XMCDMA_NUM_APP_WORDS		5
#define XMCDMA_MAX_CHAN_PER_DEVICE	32
#define XMCDMA_CHAN_IDLE		0
#define XMCDMA_CHAN_PAUSE		1
#define XMCDMA_CHAN_BUSY		2
#define XMCDMA_BD_MINIMUM_ALIGNMENT	0x40
#define XMCDMA_AXCACHE			0xB

/* Direction flags */
#define XMCDMA_DEV_TO_MEM		0
#define XMCDMA_MEM_TO_DEV		1

/**************************** Type Definitions *******************************/

typedef enum {
	XMCDMA_TX_HANDLER_DONE,     /**< For Done Handler */
	XMCDMA_TX_HANDLER_ERROR,    /**< For Error Handler */
	XMCDMA_HANDLER_DONE,     /**< For Done Handler */
	XMCDMA_HANDLER_ERROR,    /**< For Error Handler */
	XMCDMA_HANDLER_PKTDROP,    /**< For Error Handler */
} XMcdma_Handler;

typedef enum {
        XMCDMA_CHAN_HANDLER_DONE,     /**< For Done Handler */
        XMCDMA_CHAN_HANDLER_ERROR,    /**< For Error Handler */
	XMCDMA_CHAN_HANDLER_PKTDROP,    /**< For Error Handler */
} XMcdma_ChanHandler;

typedef void (*XMcdma_DoneHandler) (void *CallBackRef, u32 Chan_Id);
typedef void (*XMcdma_ErrorHandler) (void *CallBackRef, u32 Chan_id, u32 ErrorMask);
typedef void (*XMcdma_PktDropHandler) (void *CallBackRef, u32 Chan_Id);
typedef void (*XMcdma_TxDoneHandler) (void *CallBackRef, u32 Chan_Id);
typedef void (*XMcdma_TxErrorHandler) (void *CallBackRef, u32 Chan_id, u32 ErrorMask);


typedef void (*XMcdma_ChanDoneHandler) (void *CallBackRef);
typedef void (*XMcdma_ChanErrorHandler) (void *CallBackRef, u32 ErrorMask);
typedef void (*XMcdma_ChanPktDropHandler) (void *CallBackRef);

typedef enum {
	XMCDMA_FIXED_PRIORITY,
	XMCDMA_WRR,
	XMCDMA_WRR_PRIORITY,
} XMcdma_QScheduler;

typedef struct {
	UINTPTR ChanBase;
	u32 Chan_id;		/* Channel Number */
	u32 MaxTransferLen;	/* Maximum transfer length */
	u32 len;		/* Total size of bd's length in channel */
	u32 IsRxChan;
	u32 ext_addr;
	u32 Has_Txdre;
	u32 Has_Rxdre;
	u32 TxDataWidth;
	u32 RxDataWidth;
	u32 HasStsCntrlStrm;
	int HasRxLength;
	volatile int ChanState;

	UINTPTR FirstBdAddr;
	UINTPTR LastBdAddr;
	UINTPTR Separation;

	XMcdma_Bd *BdHead;
	XMcdma_Bd *BdTail;
	XMcdma_Bd *BdRestart;

	u32 BdCnt;
	u32 BdPendingCnt;
	u32 BdSubmitCnt;
	u32 BdDoneCnt;
	u32 Length;

	XMcdma_QScheduler Schedulertype;

	XMcdma_ChanDoneHandler DoneHandler;  /**< Call back for transfer
	                                          *  done interrupt */
	void *DoneRef;                  /**< To be passed to the done
	                                     * interrupt callback */

	XMcdma_ChanErrorHandler ErrorHandler;/**< Call back for error
	                                          *  interrupt */
	void *ErrorRef;                 /**< To be passed to the error
	                                     * interrupt callback */
	XMcdma_ChanPktDropHandler PktdropHandler;
	void *PktDropRef;
} XMcdma_ChanCtrl;

typedef struct {
	u32 DeviceId;
	UINTPTR BaseAddress;
	int AddrWidth;
	int Has_SingleIntr;
	int HasMM2S;
	int HasMM2SDRE;
	int TxNumChannels;
	int HasS2MM;
	int HasS2MMDRE;
	int RxNumChannels;
	int MM2SDataWidth;
	int S2MMDataWidth;
	u32 MaxTransferlen;
	int HasStsCntrlStrm;
	int HasRxLength;
	u8 IsTxCacheCoherent; /**< Describes whether Cache Coherent or not */
	u8 IsRxCacheCoherent; /**< Describes whether Cache Coherent or not */
} XMcdma_Config;

typedef struct {
	u32 IsReady;
	XMcdma_Config   Config;
	XMcdma_ChanCtrl Tx_Chan[XMCDMA_MAX_CHAN_PER_DEVICE];
	XMcdma_ChanCtrl Rx_Chan[XMCDMA_MAX_CHAN_PER_DEVICE];
	XMcdma_TxDoneHandler TxDoneHandler;  /**< Call back for transfer
	                                          *  done interrupt */
	void *TxDoneRef;                  /**< To be passed to the done
	                                     * interrupt callback */

	XMcdma_TxErrorHandler TxErrorHandler;/**< Call back for error
	                                          *  interrupt */
	void *TxErrorRef;                 /**< To be passed to the error
	                                     * interrupt callback */
	XMcdma_DoneHandler DoneHandler;  /**< Call back for transfer
	                                          *  done interrupt */
	void *DoneRef;                  /**< To be passed to the done
	                                     * interrupt callback */

	XMcdma_ErrorHandler ErrorHandler;/**< Call back for error
	                                          *  interrupt */
	void *ErrorRef;                 /**< To be passed to the error
	                                     * interrupt callback */
	XMcdma_PktDropHandler PktDropHandler;/**< Call back for error
	                                          *  interrupt */
	void *PktDropRef;                 /**< To be passed to the error
	                                     * interrupt callback */

} XMcdma;
/***************** Macros (Inline Functions) Definitions *********************/

/*****************************************************************************/
/**
* Disable Particular Channel in the MCDMA Core.
*
* @param	Chan is the MCDMA Channel to be worked on.
* @param	ChanId is the channel to disable.
*
* @note		C-style signature:
* 		XMcdma_DisableCh(XMcdma_ChanCtrl *Chan, u32 ChanId)
*****************************************************************************/
#define XMcdma_DisableCh(Chan, ChanId) \
	(XMcdma_WriteReg(Chan->ChanBase,(XMCDMA_CHEN_OFFSET), \
		XMcdma_ReadReg((Chan)->ChanBase, XMCDMA_CHEN_OFFSET) & \
			~(ChanId & XMCDMA_CHEN_MASK)))

/*****************************************************************************/
/**
* Get Channel number that last Serviced.
*
* @param	Chan is the MCDMA Channel to be worked on.
*
* @return	Channel Number that last serviced.
*
* @note		C-style signature:
* 	 	XMcdma_GetChan(XMcdma_ChanCtrl *Chan)
*****************************************************************************/
#define XMcdma_GetChan(Chan) \
	XMcdma_ReadReg(Chan->ChanBase, XMCDMA_CHSER_OFFSET)

/*****************************************************************************/
/**
* This function checks whether DMA is Idle or not
*
* @param	Chan is the MCDMA Channel to be worked on.
*
* @return
*		- TRUE if MCDMA is idle
*		- FALSE if MCDMA is not idle
*
* @note		None
*
*****************************************************************************/
#define XMcdma_isidle(Chan) \
	(XMcdma_ReadReg(Chan->ChanBase, XMCDMA_SR_OFFSET) & \
			XMCDMA_SR_IDLE_MASK)

/*****************************************************************************/
/**
 * This function enables interrupts specified by the Mask in specified
 * direction, Interrupts that are not in the mask are not affected.
 *
 * @param	Chan is the MCDMA Channel to be worked on.
 * @param	Mask is the mask for the interrupts to be enabled
 *
 * @return	None
 *
 * @note	None
 *
 *****************************************************************************/
#define XMcdma_IntrEnable(Chan, Mask) 	\
     (XMcdma_WriteReg((Chan)->ChanBase, \
			 (Chan->Chan_id - 1) * XMCDMA_NXTCHAN_OFFSET + XMCDMA_CR_OFFSET, \
		 XMcdma_ReadReg((Chan)->ChanBase, (Chan->Chan_id - 1) *    \
				XMCDMA_NXTCHAN_OFFSET + XMCDMA_CR_OFFSET) | \
			        ((Mask) & XMCDMA_IRQ_ALL_MASK)))

/*****************************************************************************/
/**
 * This function disables interrupts specified by the Mask. Interrupts that
 * are not in the mask are not affected.
 *
 * @param	Chan is the MCDMA Channel to be worked on.
 * @param	Mask is the mask for the interrupts to be disabled
 *
 * @return	None
 *
 * @note	None
 *
 *****************************************************************************/
#define XMcdma_IntrDisable(Chan, Mask) 	\
	(XMcdma_WriteReg((Chan)->ChanBase,  \
			(Chan->Chan_id - 1) * XMCDMA_NXTCHAN_OFFSET + XMCDMA_CR_OFFSET, \
			XMcdma_ReadReg((Chan)->ChanBase, (Chan->Chan_id - 1) * \
				       XMCDMA_NXTCHAN_OFFSET + XMCDMA_CR_OFFSET) & \
				       ~((Mask) & XMCDMA_IRQ_ALL_MASK)))

/*****************************************************************************/
/**
* Gets global Packet drop Count
*
* @param	Chan is the MCDMA Channel to be worked on.
*
* @return	Packet Drop Count.
*
* @note		C-style signature:
* 		XMcdma *XMcdma_GetPktDrp_cnt(XMcdma_ChanCtrl *Chan)
*****************************************************************************/
#define XMcdma_GetPktDrp_cnt(Chan) \
	XMcdma_ReadReg(Chan->ChanBase, XMCDMA_CPKTDROP_OFFSET)

/*****************************************************************************/
/**
* Gets the TX Channel Pointer
*
* @param	InstancePtr is a pointer to the DMA engine instance to be
*		worked on.
* @param	ChanId is the channel to operate on
*
* @return	Pointer to the Channel
*
* @note		C-style signature:
* 	        XMcdma_ChanCtrl *XMcdma_McdmaTxChan(XMcdma * InstancePtr,
* 	        				  u32 ChanId)
*
*****************************************************************************/
#define XMcdma_GetMcdmaTxChan(InstancePtr, ChanId) \
		 (&((InstancePtr)->Tx_Chan[ChanId]))

/*****************************************************************************/
/**
* Gets the RX Channel Pointer
*
* @param	InstancePtr is a pointer to the DMA engine instance to be
*		worked on.
* @param	ChanId is the channel to operate on
*
* @return	Pointer to the Channel
*
* @note		C-style signature:
* 	        XMcdma_ChanCtrl *XMcdma_McdmaRxChan(XMcdma * InstancePtr,
* 	        				  u32 ChanId)
*
*****************************************************************************/
#define XMcdma_GetMcdmaRxChan(InstancePtr, ChanId) \
		 (&((InstancePtr)->Rx_Chan[ChanId]))

/*****************************************************************************/
/**
* Gets the Channel BD Chain Current BD
*
* @param        Chan is the MCDMA Channel to Operate on.
*
* @return       Channel BD Chain current Buffer descriptor.
*
* @note         C-style signature:
*               XMdma_ChanCtrl *XMcdma_GetChanCurBd(XMcdma_ChanCtrl * Chan)
*****************************************************************************/
#define XMcdma_GetChanCurBd(Chan)  ((Chan)->BdRestart)

/*****************************************************************************/
/**
* This functions gives the Buffer descriptor count processed by the h/w.
*
* @param        Chan is the MCDMA Channel to Operate on.
*
* @return       Buffer descriptor count processed by the h/w.
*
* @note         C-style signature:
*               XMdma_ChanCtrl *XMcdma_GetChanBdDoneCnt(XMcdma_ChanCtrl * Chan)
*****************************************************************************/
#define XMcdma_GetChanBdDoneCnt(Chan)  ((Chan)->BdDoneCnt)

/*****************************************************************************/
/**
 * This function gets the interrupts that are asserted.
 *
 * @param       Chan is the MCDMA Channel to Operate on.
 *
 * @return	The bit mask for the interrupts asserted.
 *
 * @note	None
 *
 *****************************************************************************/
#define XMcdma_ChanGetIrq(Chan)				\
	(XMcdma_ReadReg(Chan->ChanBase, ((Chan->Chan_id - 1) * XMCDMA_NXTCHAN_OFFSET + \
		       XMCDMA_SR_OFFSET)) & XMCDMA_IRQ_ALL_MASK)
/*****************************************************************************/
/**
 * This function acknowledges the interrupts that are specified in Mask
 *
 * @param       Chan is the MCDMA Channel to Operate on.
 * @param	Mask is the mask for the interrupts to be acknowledge
 *
 * @return	None
 *
 * @note	None.
 *
 *****************************************************************************/
#define XMcdma_ChanAckIrq(Chan, Mask)		\
	XMcdma_WriteReg((Chan)->ChanBase,	\
			((Chan->Chan_id - 1) * XMCDMA_NXTCHAN_OFFSET + XMCDMA_SR_OFFSET) ,\
			(Mask) & XMCDMA_IRQ_ALL_MASK)

/*****************************************************************************/
/**
* Get Packet drop Count for the particular Channel
*
* @param	Chan is the MCDMA Channel to be worked on.
* @param	ChanId is the Channel number to be worked on.
*
* @return	Packet Drop Count.
*
* @note		C-style signature:
* 		XMcdma *XMcdma_GetChanPktDrp_Cnt(XMcdma_ChanCtrl *Chan,
* 						 u32 ChanId)
*****************************************************************************/
#define XMcdma_GetChanPktDrp_Cnt(Chan, ChanId) \
	XMcdma_ReadReg(Chan->ChanBase, ((ChanId - 1) * XMCDMA_NXTCHAN_OFFSET + \
		       XMCDMA_PKTDROP_OFFSET))

/*****************************************************************************/
/**
 * This function sets the ARCACHE field with the user specified value
 *
 * @param	InstancePtr is the driver instance we are working on
 * @param	Value is the ARCACHE value to be written.
 *
 * @return	None
 *
 * @note	None.
 *
 *****************************************************************************/
#define XMcdma_SetARCache(InstancePtr, Value)			     \
	XMcdma_WriteReg(InstancePtr->Config.BaseAddress,	     \
			XMCDMA_TXAXCACHE_OFFSET,  		     \
			(Value) & XMCDMA_AXCACHE_MASK)

/*****************************************************************************/
/**
 * This function sets the AWCACHE field with the user specified value
 *
 * @param	InstancePtr is the driver instance we are working on
 * @param	Value is the AWCACHE value to be written.
 *
 * @return	None
 *
 * @note	None.
 *
 *****************************************************************************/
#define XMcdma_SetAWCache(InstancePtr, Value)			     \
	XMcdma_WriteReg(InstancePtr->Config.BaseAddress,	     \
			XMCDMA_RXAXCACHE_OFFSET + XMCDMA_RX_OFFSET,  \
			(Value) & XMCDMA_AXCACHE_MASK)

/*****************************************************************************/
/**
 * Get Egress Channel Observer contents
 *
 * @param	InstancePtr is the driver instance we are working on.
 * @param	ObsId is the Egress Channel observer number to be worked on.
 *
 * @return	Channel Observer register contents.
 *
 * @note	C-style signature:
 * 		XMcdma *XMcdma_GetEgressObserver(XMcdma *InstancePtr,
 * 						 u32 ObsId)
*****************************************************************************/
#define XMcdma_GetEgressObserver(InstancePtr, ObsId)	\
	XMcdma_ReadReg(InstancePtr->Config.BaseAddress, (((ObsId - 1) * XMCDMA_NXTOBS_OFFSET) + \
			   XMCDMA_CHOBS1_OFFSET))

/*****************************************************************************/
/**
 * Get Ingress Channel Observer contents
 *
 * @param	InstancePtr is the driver instance we are working on.
 * @param	ObsId is the Ingress Channel observer number to be worked on.
 *
 * @return	Channel Observer register contents.
 *
 * @note	C-style signature:
 * 		XMcdma *XMcdma_GetIngressObserver(XMcdma *InstancePtr,
 * 						 u32 ObsId)
*****************************************************************************/
#define XMcdma_GetIngressObserver(InstancePtr, ObsId)	\
	XMcdma_ReadReg(InstancePtr->Config.BaseAddress, (((ObsId - 1) * XMCDMA_NXTOBS_OFFSET) + \
		       XMCDMA_RX_OFFSET + XMCDMA_CHOBS1_OFFSET))

#if defined (__aarch64__) || defined (__arch64__)
#define MAX_TRANSFER_LEN(h) \
	(((~0UL) << (0)) & (~0UL >> (64 - 1 - (h))))
#else
#define MAX_TRANSFER_LEN(h) \
	(((~0UL) << (0)) & (~0UL >> (32 - 1 - (h))))
#endif

#if defined (__aarch64__) || defined (__arch64__)
#define WRR_MASK(h, l) \
         (((~0UL) << (l)) & (~0UL >> (64 - 1 - (h))))
#else
#define WRR_MASK(h, l) \
         (((~0UL) << (l)) & (~0UL >> (32 - 1 - (h))))
#endif


/*@}*/

/************************ Prototypes of functions **************************/
XMcdma_Config *XMcdma_LookupConfig(u16 DeviceId);
XMcdma_Config *XMcdma_LookupConfigBaseAddr(UINTPTR Baseaddr);
s32 XMcDma_CfgInitialize(XMcdma *InstancePtr, XMcdma_Config *CfgPtr);
s32 XMcDma_Initialize(XMcdma *InstancePtr, XMcdma_Config *CfgPtr);
void XMcDma_Reset(XMcdma *InstancePtr);
s32 XMcdma_SelfTest(XMcdma *InstancePtr);
u32 XMcdma_ResetIsDone(XMcdma *InstancePtr);
void XMcdma_DumpChanRegs(XMcdma_ChanCtrl *Chan);
u32 XMcdma_Start(XMcdma_ChanCtrl *Chan);
u32 XMcdma_ChanHwStop(XMcdma_ChanCtrl *Chan);
void XMcdma_EnableCh(XMcdma_ChanCtrl *Chan);
u16 XMcdma_GetChanServiced(XMcdma *InstancePtr);
u16 XMcdma_GetTxChanServiced(XMcdma *InstancePtr);
u32 XMcdma_SetChanCoalesceDelay(XMcdma_ChanCtrl *Chan, u32 IrqCoalesce,
				u32 IrqDelay);
u32 XMCdma_SetChan_Weight(XMcdma_ChanCtrl *Chan, u8 Weight);
u32 XMCdma_GetChan_Weight(XMcdma_ChanCtrl *Chan);
u32 XMCdma_GetChan_PktDoneCnt(XMcdma_ChanCtrl *Chan);
void XMcdma_SetSGAWCache(XMcdma *InstancePtr, u8 Value);
void XMcdma_SetSGARCache(XMcdma *InstancePtr, u8 Value);

int XMcdma_UpdateChanCDesc(XMcdma_ChanCtrl *Chan);
int XMcdma_UpdateChanTDesc(XMcdma_ChanCtrl *Chan);
u32 XMcDma_ChanBdCreate(XMcdma_ChanCtrl *Chan, UINTPTR Addr, u32 Count);
u32 XMcDma_ChanSubmit(XMcdma_ChanCtrl *Chan, UINTPTR BufAddr, u32 len);
u32 XMcDma_Chan_Sideband_Submit(XMcdma_ChanCtrl *ChanPtr, UINTPTR BufAddr,
				u32 Len, u32 *AppPtr, u16 Tuser, u16 Tid);
u32 XMcDma_ChanToHw(XMcdma_ChanCtrl *Chan);
int XMcdma_BdChainFromHW(XMcdma_ChanCtrl *Chan, u32 BdLimit,
			 XMcdma_Bd **BdSetPtr);
int XMcdma_BdChainFree(XMcdma_ChanCtrl *Chan, int BdCount, XMcdma_Bd *BdSetPtr);
u32 XMcdma_BdSetBufAddr(XMcdma_Bd *BdPtr, UINTPTR Addr);
void XMcDma_BdSetCtrl(XMcdma_Bd *BdPtr, u32 Data);
void XMcDma_DumpBd(XMcdma_Bd* BdPtr);

int XMcDma_BdSetAppWord(XMcdma_Bd* BdPtr, int Offset, u32 Word);
u32 XMcDma_BdGetAppWord(XMcdma_Bd* BdPtr, int Offset, int *Valid);

/* Global OR'ed Single interrupt */
void XMcdma_IntrHandler(void *Instance);
void XMcdma_TxIntrHandler(void *Instance);
s32 XMcdma_SetCallBack(XMcdma *InstancePtr, XMcdma_Handler HandlerType,
		       void *CallBackFunc, void *CallBackRef);
/* Per Channel interrupt */
void XMcdma_ChanIntrHandler(void *Instance);
s32 XMcdma_ChanSetCallBack(XMcdma_ChanCtrl *Chan, XMcdma_ChanHandler HandlerType,
			      void *CallBackFunc, void *CallBackRef);
#ifdef __cplusplus
}

#endif
#endif /* XMCDMA_H_ */
