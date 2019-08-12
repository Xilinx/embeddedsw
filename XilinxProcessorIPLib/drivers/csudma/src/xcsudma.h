/******************************************************************************
*
* Copyright (C) 2014-2019 Xilinx, Inc.  All rights reserved.
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
* The CSU_DMA is present inside CSU (Configuration Security Unit) module which
* is located within the Low-Power Subsystem (LPS) internal to the PS.
* CSU_DMA allows the CSU to move data efficiently between the memory (32 bit
* AXI interface) and the CSU stream peripherals (SHA, AES and PCAP) via Secure
* Stream Switch (SSS).
*
* The CSU_DMA is a 2 channel simple DMA, allowing separate control of the SRC
* (read) channel and DST (write) channel. The DMA is effectively able to
* transfer data:
*	- From PS-side to the SSS-side (SRC DMA only)
*	- From SSS-side to the PS-side (DST DMA only)
*	- Simultaneous PS-side to SSS_side and SSS-side to the PS-side
*
* <b>Initialization & Configuration</b>
*
* The device driver enables higher layer software (e.g., an application) to
* communicate to the CSU_DMA core.
*
* XCsuDma_CfgInitialize() API is used to initialize the CSU_DMA core.
* The user needs to first call the XCsuDma_LookupConfig() API which returns
* the Configuration structure pointer which is passed as a parameter to the
* XCsuDma_CfgInitialize() API.
*
* <b> Reset </b>
* This driver will not support handling of CRP PDMA Reset in case of PMCDMA
* inorder to support multiple level of handoff's. User needs to call
* the XCsuDma_PmcReset() API before performing any driver operation to make
* sure PMCDMA is in proper state.
*
* <b> Interrupts </b>
* This driver will not support handling of interrupts user should write handler
* to handle the interrupts.
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
* The XCsuDma driver is composed of several source files. This allows the user
* to build and link only those parts of the driver that are necessary.
*
* @file xcsudma.h
* @addtogroup csudma_v1_5
* @{
* @details
*
* This header file contains identifiers and register-level driver functions (or
* macros), range macros, structure typedefs that can be used to access the
* Xilinx CSU_DMA core instance.
*
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
#define XCSUDMA_SIZE_MAX 0x07FFFFFFU	/**< Maximum allowed no of words */

#define XCSUDMA_DMATYPEIS_CSUDMA 	0U
#define XCSUDMA_DMATYPEIS_PMCDMA0	1U
#define XCSUDMA_DMATYPEIS_PMCDMA1	2U

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
* @param	DmaType is the type of DMA (PMCDMA0 or PMCDMA1).
*
* @return	None.
*
* @note		None.
*		C-style signature:
*		void XCsuDma_PmcReset(u8 DmaType)
*
******************************************************************************/
#define XCsuDma_PmcReset(DmaType)  \
	Xil_Out32(((u32)(XPS_CRP_BASEADDRESS) + (u32)(XCRP_PMCDMA_RESET_OFFSET)), \
			(u32)(XCSUDMA_RESET_SET_MASK << (DmaType - 1))); \
	Xil_Out32(((u32)(XPS_CRP_BASEADDRESS) + (u32)(XCRP_PMCDMA_RESET_OFFSET)), \
			(u32)(XCSUDMA_RESET_UNSET_MASK << (DmaType - 1)));
#endif

/*****************************************************************************/
/**
* This function will be in busy while loop until the data transfer is
* completed.
*
* @param	InstancePtr is a pointer to XCsuDma instance to be worked on.
* @param	Channel represents the type of channel either it is Source or
*		Destination.
*		Source channel      - XCSUDMA_SRC_CHANNEL
*		Destination Channel - XCSUDMA_DST_CHANNEL
*
* @return	None.
*
* @note		This function should be called after XCsuDma_Transfer in polled
*		mode  to wait until the data gets transfered completely.
*		C-style signature:
*		void XCsuDma_WaitForDone(XCsuDma *InstancePtr,
*						XCsuDma_Channel Channel)
*
******************************************************************************/
#define XCsuDma_WaitForDone(InstancePtr,Channel) \
		while((XCsuDma_ReadReg(((InstancePtr)->Config.BaseAddress), \
			((u32)(XCSUDMA_I_STS_OFFSET) + \
			((u32)(Channel) * (u32)(XCSUDMA_OFFSET_DIFF)))) & \
		(u32)(XCSUDMA_IXR_DONE_MASK)) != (XCSUDMA_IXR_DONE_MASK))


/*****************************************************************************/
/**
*
* This function returns the number of completed SRC/DST DMA transfers that
* have not been acknowledged by software based on the channel selection.
*
* @param	InstancePtr is a pointer to XCsuDma instance to be worked on.
* @param	Channel represents the type of channel either it is Source or
* 		Destination.
*		Source channel      - XCSUDMA_SRC_CHANNEL
*		Destination Channel - XCSUDMA_DST_CHANNEL
*
* @return	Count is number of completed DMA transfers but not acknowledged
*		(Range is 0 to 7).
*		- 000 - All finished transfers have been acknowledged.
*		- Count - Count number of finished transfers are still
*		outstanding.
*
* @note		None.
*		C-style signature:
*		u8 XCsuDma_GetDoneCount(XCsuDma *InstancePtr,
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
* @param	InstancePtr is a pointer to XCsuDma instance to be worked on.
* @param	Channel represents the type of channel either it is Source or
* 		Destination.
*		Source channel      - XCSUDMA_SRC_CHANNEL
*		Destination Channel - XCSUDMA_DST_CHANNEL
*
* @return	FIFO level. (Range is 0 to 128)
*		- 0 Indicates empty
*		- Any number 1 to 128 indicates the number of entries in FIFO.
*
* @note		None.
*		C-style signature:
*		u8 XCsuDma_GetFIFOLevel(XCsuDma *InstancePtr,
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
* @param	InstancePtr is a pointer to XCsuDma instance to be worked on.
* @param	Channel represents the type of channel either it is Source or
* 		Destination.
*		Source channel      - XCSUDMA_SRC_CHANNEL
*		Destination Channel - XCSUDMA_DST_CHANNEL
*
* @return	Count of outstanding commands. (Range is 0 to 9).
*
* @note		None.
*		C-style signature:
*		u8 XCsuDma_GetWROutstandCount(XCsuDma *InstancePtr,
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
* @param	InstancePtr is a pointer to XCsuDma instance to be worked on.
* @param	Channel represents the type of channel either it is Source or
* 		Destination.
*		Source channel      - XCSUDMA_SRC_CHANNEL
*		Destination Channel - XCSUDMA_DST_CHANNEL
*
* @return	Returns the current status of the core.
*		- TRUE represents core is currently busy.
*		- FALSE represents core is not involved in any transfers.
*
* @note		None.
*		C-style signature:
*		s32 XCsuDma_IsBusy(XCsuDma *InstancePtr, XCsuDma_Channel Channel)
*
******************************************************************************/

#define XCsuDma_IsBusy(InstancePtr, Channel) \
		((XCsuDma_ReadReg(((InstancePtr)->Config.BaseAddress), \
					((u32)(XCSUDMA_STS_OFFSET) + \
			((u32)(Channel) * (u32)(XCSUDMA_OFFSET_DIFF)))) & \
		(u32)(XCSUDMA_STS_BUSY_MASK)) == (XCSUDMA_STS_BUSY_MASK)) ? \
				(TRUE) : (FALSE)


/**************************** Type Definitions *******************************/

/**
* This typedef contains configuration information for a CSU_DMA core.
* Each CSU_DMA core should have a configuration structure associated.
*/
typedef struct {
	u16 DeviceId;		/**< DeviceId is the unique ID of the
				  *  device */
	u32 BaseAddress;	/**< BaseAddress is the physical base address
				  *  of the device's registers */
	u8 DmaType;		/**< DMA type
				 * 0 -- CSUDMA
				 * 1 -- PMC DMA 0
				 * 2 -- PMC DMA 1 */
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
* This typedef contains all the configuration feilds which needs to be set
* before the start of the data transfer. All these feilds of CSU_DMA can be
* configured by using XCsuDma_SetConfig API.
*/
typedef struct {
	u8 SssFifoThesh;	/**< SSS FIFO threshold value */
	u8 ApbErr;		/**< ABP invalid access error */
	u8 EndianType;		/**< Type of endianess */
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


/************************** Function Prototypes ******************************/

XCsuDma_Config *XCsuDma_LookupConfig(u16 DeviceId);

s32 XCsuDma_CfgInitialize(XCsuDma *InstancePtr, XCsuDma_Config *CfgPtr,
			u32 EffectiveAddr);
void XCsuDma_Transfer(XCsuDma *InstancePtr, XCsuDma_Channel Channel,
					UINTPTR Addr, u32 Size, u8 EnDataLast);
void XCsuDma_64BitTransfer(XCsuDma *InstancePtr, XCsuDma_Channel Channel,
			   u32 AddrLow, u32 AddrHigh, u32 Size, u8 EnDataLast);
void XCsuDma_LoopBackTransfer(XCsuDma *InstancePtr, u64 SrcAddr, u64 DstAddr,
						u32 Size);
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
void XCsuDma_ClearDoneCount(XCsuDma *InstancePtr, XCsuDma_Channel Channel);

void XCsuDma_SetSafetyCheck(XCsuDma *InstancePtr, u32 Value);
u32 XCsuDma_GetSafetyCheck(XCsuDma *InstancePtr);
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
