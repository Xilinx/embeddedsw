/******************************************************************************
* Copyright (c) 2014 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2022 - 2024 Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
* @file xcsudma.h
* @addtogroup csuma_api CSUDMA APIs
* @{
* @details
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who     Date     Changes
* ----- ------  -------- -----------------------------------------------------
* 1.0   vnsld   22/10/14 First release
* 1.1   adk     10/05/16 Fixed CR#951040 race condition in the recv path when
*                        source and destination points to the same buffer.
*       ms      03/17/17 Added readme.txt file in examples folder for doxygen
*                        generation.
*       ms      04/10/17 Modified filename tag in xcsudma_selftest_example.c to
*                        include the file in doxygen examples.
* 1.2   adk     11/22/17 Added peripheral test app support for CSUDMA driver.
* 1.4   adk     04/12/17 Added support for PMC DMA.
*       adk     09/03/18 Added new API XCsuDma_64BitTransfer() useful for 64-bit
*                        dma transfers through PMU processor(CR#996201).
*       adk     25/06/18 Move CRP and PMC Global address defines to
*			 xparameters_ps.h file(CR#1002035).
*	adk	08/08/18 Added new API XCsuDma_WaitForDoneTimeout() useful for
*			 polling dma transfer done.
*	adk     28/08/18 Fixed misra-c required standard violations..
*       Rama	02/26/19 Fixed IAR issue by changing
*						 "XCsuDma_WaitForDoneTimeout" to function
*       arc     03/26/19 Fixed MISRA-C violations.
* 1.7	hk	08/03/20 Reorganize transfer function to accommodate all
*			 processors and cache functionality.
* 1.7	sk	08/26/20 Fix MISRA-C violations.
* 1.8   nsk     12/14/20 Updated the tcl to not to use the instance names.
* 1.9	sk	02/11/21 Add description for the dmatype macros.
* 1.9	sk	02/11/21 Remove the prototype of undefined functions.
* 1.11	sk	03/03/22 Move addtogroup to starting of the file and replace
* 			 driver version with Overview.
* 1.11	sk	03/03/22 Update overview section based on review comments.
* 1.11	adk	03/15/22 Fixed syntax errors in csudma_tapp.tcl file, when stdout
* 			 is configured as none.
* 1.14	ab	01/16/23 Added Xil_PlmStubHandler() to XCsuDma_WaitForDone.
* 1.14	ab	01/18/23 Added byte-aligned transfer API for VERSAL_NET devices.
* 1.14  adk     04/14/23 Added support for system device-tree flow.
* </pre>
*
******************************************************************************/

#ifndef XCSUDMA_H_
#define XCSUDMA_H_	/**< Prevent circular inclusions
			  *  by using protection macros */

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/

#include "xcsudma_hw.h"
#include "xil_types.h"
#include "xil_assert.h"
#include "xstatus.h"
#include "xil_cache.h"
#include "sleep.h"
#include "xil_util.h"

/************************** Constant Definitions *****************************/

/**
 * This typedef contains CSU_DMA Channel Types.
 */
typedef enum {
	XCSUDMA_SRC_CHANNEL = 0U,	/**< Source Channel of CSU_DMA */
	XCSUDMA_DST_CHANNEL		/**< Destination Channel of CSU_DMA */
}XCsuDma_Channel;
/*@}*/

/**
 * This typedef contains CSU_DMA Pause Types.
 */
typedef enum {
	XCSUDMA_PAUSE_MEMORY,		/**< Pauses memory data transfer
					  *  to/from CSU_DMA */
	XCSUDMA_PAUSE_STREAM,		/**< Pauses stream data transfer
					  *  to/from CSU_DMA */
}XCsuDma_PauseType;

/*@}*/


/** @name Ranges of Size
 * @{
 */
#if defined(VERSAL_NET) || defined(VERSAL_AIEPG2)
#define XCSUDMA_SIZE_MAX 0x1FFFFFFFU	/**< Maximum allowed no of bytes */
#else
#define XCSUDMA_SIZE_MAX 0x07FFFFFFU	/**< Maximum allowed no of words */
#endif

#define XCSUDMA_DMATYPEIS_CSUDMA 	0U	/**< DMA is CSUDMA  */
#define XCSUDMA_DMATYPEIS_PMCDMA0	1U	/**< DMA is PMCDMA0 */
#define XCSUDMA_DMATYPEIS_PMCDMA1	2U	/**< DMA is PMCDMA1 */
#define XCSUDMA_DMATYPEIS_ASUDMA0	3U	/**< DMA is ASUDMA0 */
#define XCSUDMA_DMATYPEIS_ASUDMA1	4U	/**< DMA is ASUDMA1 */

/*@}*/

/***************** Macros (Inline Functions) Definitions *********************/

/*****************************************************************************/
/**
*
* This function resets the CSU_DMA core.
*
* @return	None.
*
* @note		None.
*		C-style signature:
*		void XCsuDma_Reset()
*
******************************************************************************/
#define XCsuDma_Reset()  \
	Xil_Out32(((u32)(XCSU_BASEADDRESS) + (u32)(XCSU_DMA_RESET_OFFSET)), \
				(u32)(XCSUDMA_RESET_SET_MASK)); \
	Xil_Out32(((u32)(XCSU_BASEADDRESS) + (u32)(XCSU_DMA_RESET_OFFSET)), \
					(u32)(XCSUDMA_RESET_UNSET_MASK));

#if defined (versal)
/*****************************************************************************/
/**
*
* This function resets the PMC_DMA core.
*
* @param	DmaType Type of DMA (PMCDMA0 or PMCDMA1).
*
* @return	None.
*
* @note	 C-style signature:
*		 void XCsuDma_PmcReset(u8 DmaType)
*
******************************************************************************/
#define XCsuDma_PmcReset(DmaType)  \
	Xil_Out32(((u32)(XPS_CRP_BASEADDRESS) + (u32)(XCRP_PMCDMA_RESET_OFFSET)), \
			((u32)XCSUDMA_RESET_SET_MASK << ((u32)DmaType - 1U))); \
	Xil_Out32(((u32)(XPS_CRP_BASEADDRESS) + (u32)(XCRP_PMCDMA_RESET_OFFSET)), \
			((u32)XCSUDMA_RESET_UNSET_MASK << ((u32)DmaType - 1U)));
#endif

/*****************************************************************************/
/**
* This function will be in busy while loop until the data transfer is
* completed.
*
* @param	InstancePtr Pointer to XCsuDma instance to be worked on.
* @param	Channel Represents the type of channel either it is Source or
*		Destination.
*		Source channel      - XCSUDMA_SRC_CHANNEL
*		Destination Channel - XCSUDMA_DST_CHANNEL
*
* @return	None.
*
* @note		This function should be called after XCsuDma_Transfer in polled
*		mode  to wait until the data gets transferred completely.
*		C-style signature:
*		void XCsuDma_WaitForDone(XCsuDma *InstancePtr,
*						XCsuDma_Channel Channel)
*
******************************************************************************/
#ifdef VERSAL_PLM
#define XCsuDma_WaitForDone(InstancePtr,Channel) \
		while((XCsuDma_ReadReg(((InstancePtr)->Config.BaseAddress), \
			((u32)(XCSUDMA_I_STS_OFFSET) + \
			((u32)(Channel) * (u32)(XCSUDMA_OFFSET_DIFF)))) & \
			(u32)(XCSUDMA_IXR_DONE_MASK)) != (XCSUDMA_IXR_DONE_MASK)) { \
			Xil_PlmStubHandler(); \
		}
#else
#define XCsuDma_WaitForDone(InstancePtr,Channel) \
		while((XCsuDma_ReadReg(((InstancePtr)->Config.BaseAddress), \
			((u32)(XCSUDMA_I_STS_OFFSET) + \
			((u32)(Channel) * (u32)(XCSUDMA_OFFSET_DIFF)))) & \
			(u32)(XCSUDMA_IXR_DONE_MASK)) != (XCSUDMA_IXR_DONE_MASK))
#endif

/*****************************************************************************/
/**
*
* This function returns the number of completed SRC/DST DMA transfers that
* have not been acknowledged by software based on the channel selection.
*
* @param	InstancePtr Pointer to XCsuDma instance to be worked on.
* @param	Channel Type of channel.
*		Source channel      - XCSUDMA_SRC_CHANNEL
*		Destination Channel - XCSUDMA_DST_CHANNEL
*
* @return	Count is number of completed DMA transfers but not acknowledged
*		(Range is 0 to 7).
*		- 000 - All finished transfers have been acknowledged.
*		- Count - Count number of finished transfers are still
*		outstanding.
*
* @note	 C-style signature:
*		 u8 XCsuDma_GetDoneCount(XCsuDma *InstancePtr,
*						XCsuDma_Channel Channel)
*
******************************************************************************/
#define XCsuDma_GetDoneCount(InstancePtr, Channel)  \
		((XCsuDma_ReadReg(((InstancePtr)->Config.BaseAddress), \
			((u32)(XCSUDMA_STS_OFFSET) + \
			((u32)(Channel) * (u32)(XCSUDMA_OFFSET_DIFF)))) & \
			(u32)(XCSUDMA_STS_DONE_CNT_MASK)) >> \
				(u32)(XCSUDMA_STS_DONE_CNT_SHIFT))

/*****************************************************************************/
/**
*
* This function returns the current SRC/DST FIFO level in 32 bit words of the
* selected channel
*
* @param	InstancePtr Pointer to XCsuDma instance to be worked on.
* @param	Channel Type of channel
*		Source channel      - XCSUDMA_SRC_CHANNEL
*		Destination Channel - XCSUDMA_DST_CHANNEL
*
* @return	FIFO level. (Range is 0 to 128)
*		- 0 Indicates empty
*		- Any number 1 to 128 indicates the number of entries in FIFO.
*
* @note	 C-style signature:
*		 u8 XCsuDma_GetFIFOLevel(XCsuDma *InstancePtr,
*					XCsuDma_Channel Channel)
*
******************************************************************************/
#define XCsuDma_GetFIFOLevel(InstancePtr, Channel)  \
		((XCsuDma_ReadReg(((InstancePtr)->Config.BaseAddress), \
			((u32)(XCSUDMA_STS_OFFSET) + \
			((u32)(Channel) * (u32)(XCSUDMA_OFFSET_DIFF)))) & \
			(u32)(XCSUDMA_STS_FIFO_LEVEL_MASK)) >> \
					(u32)(XCSUDMA_STS_FIFO_LEVEL_SHIFT))

/*****************************************************************************/
/**
*
* This function returns the current number of read(src)/write(dst) outstanding
* commands based on the type of channel selected.
*
* @param	InstancePtr Pointer to XCsuDma instance to be worked on.
* @param	Channel Type of channel
*		Source channel      - XCSUDMA_SRC_CHANNEL
*		Destination Channel - XCSUDMA_DST_CHANNEL
*
* @return	Count of outstanding commands. (Range is 0 to 9).
*
* @note	 C-style signature:
*		 u8 XCsuDma_GetWROutstandCount(XCsuDma *InstancePtr,
*						XCsuDma_Channel Channel)
*
******************************************************************************/
#define XCsuDma_GetWROutstandCount(InstancePtr, Channel)  \
		((XCsuDma_ReadReg(((InstancePtr)->Config.BaseAddress), \
				((u32)(XCSUDMA_STS_OFFSET) + \
			((u32)(Channel) * (u32)(XCSUDMA_OFFSET_DIFF)))) & \
			(u32)(XCUSDMA_STS_OUTSTDG_MASK)) >> \
				(u32)(XCUSDMA_STS_OUTSTDG_SHIFT))

/*****************************************************************************/
/**
*
* This function returns the status of Channel either it is busy or not.
*
* @param	InstancePtr Pointer to XCsuDma instance to be worked on.
* @param	Channel Type of channel
*		Source channel      - XCSUDMA_SRC_CHANNEL
*		Destination Channel - XCSUDMA_DST_CHANNEL
*
* @return	Returns the current status of the core.
*		- TRUE represents core is currently busy.
*		- FALSE represents core is not involved in any transfers.
*
* @note  C-style signature:
*		 s32 XCsuDma_IsBusy(XCsuDma *InstancePtr, XCsuDma_Channel Channel)
*
******************************************************************************/

#define XCsuDma_IsBusy(InstancePtr, Channel) \
		(((XCsuDma_ReadReg(((InstancePtr)->Config.BaseAddress), \
					((u32)(XCSUDMA_STS_OFFSET) + \
			((u32)(Channel) * (u32)(XCSUDMA_OFFSET_DIFF)))) & \
		(u32)(XCSUDMA_STS_BUSY_MASK)) == (XCSUDMA_STS_BUSY_MASK)) ? \
				TRUE : FALSE)


/**************************** Type Definitions *******************************/

/**
* This typedef contains configuration information for a CSU_DMA core.
* Each CSU_DMA core should have an associated configuration structure.
*/
typedef struct {
#ifndef SDT
	u16 DeviceId;		/**< DeviceId is the unique ID of the
				  *  device */
#else
	char *Name;		/**< Unique name of the device */
#endif
	UINTPTR BaseAddress;	/**< BaseAddress is the physical base address
				  *  of the device's registers */
	u8 DmaType;		/**< DMA type
				 * 0 -- CSUDMA
				 * 1 -- PMC DMA 0
				 * 2 -- PMC DMA 1 */
#ifdef SDT
	u32 IntrId;		/** Bits[11:0] Interrupt-id Bits[15:12]
				 * trigger type and level flags */
	UINTPTR IntrParent; 	/** Bit[0] Interrupt parent type Bit[64/32:1]
				 * Parent base address */
#endif
} XCsuDma_Config;


/******************************************************************************/
/**
*
* The XCsuDma driver instance data structure. A pointer to an instance data
* structure is passed around by functions to refer to a specific driver
* instance.
*/
typedef struct {
	XCsuDma_Config Config;		/**< Hardware configuration */
	u32 IsReady;			/**< Device and the driver instance
					  *  are initialized */
}XCsuDma;


/******************************************************************************/
/**
* This typedef contains all the configuration feilds which must be set
* before the start of the data transfer. All these feilds of CSU_DMA can be
* configured by using XCsuDma_SetConfig API.
*/
typedef struct {
	u8 SssFifoThesh;	/**< SSS FIFO threshold value */
	u8 ApbErr;		/**< ABP invalid access error */
	u8 EndianType;		/**< Type of endianness */
	u8 AxiBurstType;	/**< Type of AXI bus */
	u32 TimeoutValue;	/**< Time out value */
	u8 FifoThresh;		/**< FIFO threshold value */
	u8 Acache;		/**< AXI CACHE selection */
	u8 RouteBit;		/**< Selection of Route */
	u8 TimeoutEn;		/**< Enable of time out counters */
	u16 TimeoutPre;		/**< Pre scaler value */
	u8 MaxOutCmds;		/**< Maximum number of outstanding
				  *  commands */
}XCsuDma_Configure;

/*****************************************************************************/

/************************** Variable Definitions *****************************/

#ifndef SDT
extern XCsuDma_Config XCsuDma_ConfigTable[XPAR_XCSUDMA_NUM_INSTANCES];
#else
extern XCsuDma_Config XCsuDma_ConfigTable[];
#endif


/************************** Function Prototypes ******************************/

#ifndef SDT
XCsuDma_Config *XCsuDma_LookupConfig(u16 DeviceId);
#else
XCsuDma_Config *XCsuDma_LookupConfig(UINTPTR BaseAddress);
#endif

s32 XCsuDma_CfgInitialize(XCsuDma *InstancePtr, XCsuDma_Config *CfgPtr,
			UINTPTR EffectiveAddr);
void XCsuDma_Transfer(XCsuDma *InstancePtr, XCsuDma_Channel Channel,
					u64 Addr, u32 Size, u8 EnDataLast);
void XCsuDma_64BitTransfer(XCsuDma *InstancePtr, XCsuDma_Channel Channel,
			   u32 AddrLow, u32 AddrHigh, u32 Size, u8 EnDataLast);
#if defined(VERSAL_NET) || defined(VERSAL_AIEPG2)
void XCsuDma_ByteAlignedTransfer(XCsuDma *InstancePtr, XCsuDma_Channel Channel,
					u64 Addr, u32 Size, u8 EnDataLast);
#endif
u64 XCsuDma_GetAddr(XCsuDma *InstancePtr, XCsuDma_Channel Channel);
u32 XCsuDma_GetSize(XCsuDma *InstancePtr, XCsuDma_Channel Channel);

void XCsuDma_Pause(XCsuDma *InstancePtr, XCsuDma_Channel Channel,
						XCsuDma_PauseType Type);
s32 XCsuDma_IsPaused(XCsuDma *InstancePtr, XCsuDma_Channel Channel,
						XCsuDma_PauseType Type);
void XCsuDma_Resume(XCsuDma *InstancePtr, XCsuDma_Channel Channel,
						XCsuDma_PauseType Type);

u32 XCsuDma_GetCheckSum(XCsuDma *InstancePtr);
void XCsuDma_ClearCheckSum(XCsuDma *InstancePtr);

void XCsuDma_SetConfig(XCsuDma *InstancePtr, XCsuDma_Channel Channel,
					XCsuDma_Configure *ConfigurValues);
void XCsuDma_GetConfig(XCsuDma *InstancePtr, XCsuDma_Channel Channel,
					XCsuDma_Configure *ConfigurValues);

u32 XCsuDma_WaitForDoneTimeout(XCsuDma *InstancePtr, XCsuDma_Channel Channel);

/* Interrupt related APIs */
u32 XCsuDma_IntrGetStatus(XCsuDma *InstancePtr, XCsuDma_Channel Channel);
void XCsuDma_IntrClear(XCsuDma *InstancePtr, XCsuDma_Channel Channel,
								u32 Mask);
void XCsuDma_EnableIntr(XCsuDma *InstancePtr, XCsuDma_Channel Channel,
								u32 Mask);
void XCsuDma_DisableIntr(XCsuDma *InstancePtr, XCsuDma_Channel Channel,
								u32 Mask);
u32 XCsuDma_GetIntrMask(XCsuDma *InstancePtr, XCsuDma_Channel Channel);

s32 XCsuDma_SelfTest(XCsuDma *InstancePtr);

/******************************************************************************/

#ifdef __cplusplus
}

#endif

#endif /* End of protection macro */
/** @} */
