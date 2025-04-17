/******************************************************************************
* Copyright (C) 2020 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2022 - 2024 Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/
/*****************************************************************************/
/**
*
*
* The file is a wrapper that calls APIs declared in xscudma.h This wrapper is
* added for Versal as there is no CSU DMA in Versal and hence the Versal
* libraries should have no reference to CSU DMA. This is a security requirement.
*
* The PMC_DMA is present inside PMC (Platform Management Controller) module.
* PMC_DMA allows the PMC to move data efficiently between the memory (128 bit
* AXI interface) and the PMC stream peripherals (SHA, AES and SBI) via Secure
* Stream Switch (SSS).
*
* The PMC_DMA is a 2 channel simple DMA, allowing separate control of the SRC
* (read) channel and DST (write) channel. The DMA is effectively able to
* transfer data:
*	- From PMC to the SSS-side (SRC DMA only)
*	- From SSS-side to the PMC (DST DMA only)
*	- Simultaneous PMC to SSS_side and SSS-side to PMC
*
* Initialization & Configuration
*
* The device driver enables higher layer software (e.g., an application) to
* communicate to the PMC_DMA core. The device driver internally calls CSU DMA
* APIs.
*
* XPmcDma_CfgInitialize() API is used to initialize the PMC_DMA core.
* The user needs to first call the XPmcDma_LookupConfig() API which returns
* the Configuration structure pointer which is passed as a parameter to the
* XPmcDma_CfgInitialize() API.
*
* Reset
*
* This driver will not support handling of CRP PDMA Reset in case of PMC_DMA
* inorder to support multiple level of handoff's. User needs to call
* the XPmcDma_Reset() API before performing any driver operation to make
* sure PMC_DMA is in proper state.
*
* Interrupts
*
* This driver will not support handling of interrupts user should write handler
* to handle the interrupts.
*
* Virtual Memory
*
* This driver supports Virtual Memory. The RTOS is responsible for calculating
* the correct device base address in Virtual Memory space.
*
* Threads
*
* This driver is not thread safe. Any needs for threads or thread mutual
* exclusion must be satisfied by the layer above this driver.
*
* Asserts
*
* Asserts are used within all Xilinx drivers to enforce constraints on argument
* values. Asserts can be turned off on a system-wide basis by defining, at
* compile time, the NDEBUG identifier. By default, asserts are turned on and it
* is recommended that users leave asserts on during development.
*
* Building the driver
*
* The XPmcDma driver is composed of several source files. This allows the user
* to build and link only those parts of the driver that are necessary.
*
* This header file contains identifiers and register-level driver functions (or
* macros), range macros, structure typedefs that can be used to access the
* Xilinx PMC_DMA core instance.
*
* The functionality wise ZU+ CSU_DMA and Versal PMC_DMA are similar, so all ZU+
* code is reused by wrapping in this file
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who     Date     Changes
* ----- ------  -------- -----------------------------------------------------
* 1.0   mmd     01/04/20 First release
* 1.7	am 		09/24/20 Changed return type of XPmcDma_WaitForDoneTimeout
*						 function from u32 to int
* 1.9   bm      01/13/21 Update PmcDmaTransfer argument to u64
* 1.14	ab	03/13/22 Add byte-wise transfer API for Versal-Net
* 1.14  ng      07/13/23 Added macro to detect if dma type is invalid.
* 1.16  ng      08/20/24 Added spartanup device support
*
* </pre>
*
******************************************************************************/

#ifndef XPMCDMA_H_
#define XPMCDMA_H_

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/
#include "xcsudma.h"
#if defined (versal) || defined(SPARTANUP)

/************************** Constant Definitions *****************************/
/** Ranges of Size */
#if defined(SPARTANUP)
	#ifndef SDT
	#define PMCDMA_0_DEVICE_ID      XPAR_XCSUDMA_0_DEVICE_ID /* PMCDMA device Id */
	#else
	#define PMCDMA_0_DEVICE_ID      XPAR_XCSUDMA_0_BASEADDR /* PMCDMA device Id */
	#endif
#else
	#ifndef SDT
	#define PMCDMA_0_DEVICE_ID      XPAR_XCSUDMA_0_DEVICE_ID /* PMCDMA device Id */
	#define PMCDMA_1_DEVICE_ID      XPAR_XCSUDMA_1_DEVICE_ID /* PMCDMA device Id */
	#else
	#define PMCDMA_0_DEVICE_ID      XPAR_XCSUDMA_0_BASEADDR /* PMCDMA device Id */
	#define PMCDMA_1_DEVICE_ID      XPAR_XCSUDMA_1_BASEADDR /* PMCDMA device Id */
	#endif
#endif
#define PMCDMA_LOOPBACK_CFG     (0x0000000FU)   /* LOOP BACK configuration */

#define XPMCDMA_SIZE_MAX 	XCSUDMA_SIZE_MAX /* Maximum allowed no of words */
#define XPMCDMA_ADDR_LSB_MASK	XCSUDMA_ADDR_LSB_MASK

#define XPMCDMA_DMATYPEIS_INVALID	XCSUDMA_DMATYPEIS_CSUDMA
#define XPMCDMA_DMATYPEIS_DMA0	XCSUDMA_DMATYPEIS_PMCDMA0
#define XPMCDMA_DMATYPEIS_DMA1	XCSUDMA_DMATYPEIS_PMCDMA1

#define XPMCDMA_SRC_CHANNEL	XCSUDMA_SRC_CHANNEL
#define XPMCDMA_DST_CHANNEL	XCSUDMA_DST_CHANNEL

#define XPMCDMA_PAUSE_MEMORY	XCSUDMA_PAUSE_MEMORY
#define XPMCDMA_PAUSE_STREAM	XCSUDMA_PAUSE_STREAM

/** Interrupt Enable/Disable/Mask/Status registers bit masks */
#define XPMCDMA_IXR_FIFO_OVERFLOW_MASK		XCSUDMA_IXR_FIFO_OVERFLOW_MASK
	/**< FIFO overflow mask, it is valid only to Destination Channel */
#define XPMCDMA_IXR_INVALID_APB_MASK 		XCSUDMA_IXR_INVALID_APB_MASK
	/**< Invalid APB access mask */
#define XPMCDMA_IXR_FIFO_THRESHHIT_MASK		XCSUDMA_IXR_FIFO_THRESHHIT_MASK
	/**< FIFO threshold hit indicator mask */
#define XPMCDMA_IXR_TIMEOUT_MEM_MASK		XCSUDMA_IXR_TIMEOUT_MEM_MASK
	/**< Time out counter expired to access memory mask */
#define XPMCDMA_IXR_TIMEOUT_STRM_MASK		XCSUDMA_IXR_TIMEOUT_STRM_MASK
	/**< Time out counter expired to access stream mask */
#define XPMCDMA_IXR_AXI_WRERR_MASK		XCSUDMA_IXR_AXI_WRERR_MASK
	/**< AXI Read/Write error mask */
#define XPMCDMA_IXR_DONE_MASK			XCSUDMA_IXR_DONE_MASK
	/**< Done mask */
#define XPMCDMA_IXR_MEM_DONE_MASK		XCSUDMA_IXR_MEM_DONE_MASK
	/**< Memory done mask, it is valid only for source channel*/
#define XPMCDMA_IXR_SRC_MASK			XCSUDMA_IXR_SRC_MASK
	/**< All interrupt mask for source */
#define XPMCDMA_IXR_DST_MASK			XCSUDMA_IXR_DST_MASK
	/**< All interrupt mask for destination */

/**************************** Type Definitions *******************************/
/**
 * This typedef contains PMC_DMA Channel Types.
 *
 * @note The enum values are mapped with #define above as
 * 	- XPMCDMA_SRC_CHANNEL = XCSUDMA_SRC_CHANNEL
 *	- XPMCDMA_DST_CHANNEL = XCSUDMA_DST_CHANNEL
 */
typedef XCsuDma_Channel		XPmcDma_Channel;

/**
 * This typedef contains PMC_DMA Pause Types.
 *
 * @note The enum values are mapped with #define above as
 *	- XPMCDMA_PAUSE_MEMORY = XCSUDMA_PAUSE_MEMORY
 *	- XPMCDMA_PAUSE_STREAM = XCSUDMA_PAUSE_STREAM
 */
typedef XCsuDma_PauseType	XPmcDma_PauseType;
/**
* This typedef contains configuration information for a PMC_DMA core.
* Each PMC_DMA core should have a configuration structure associated.
*/
typedef XCsuDma_Config 	XPmcDma_Config;

/******************************************************************************/
/**
*
* The XPmcDma driver instance data structure. A pointer to an instance data
* structure is passed around by functions to refer to a specific driver
* instance.
*/
typedef XCsuDma XPmcDma;

/******************************************************************************/
/**
* This typedef contains all the configuration fields which needs to be set
* before the start of the data transfer. All these feilds of PMC_DMA can be
* configured by using XPmcDma_SetConfig API.
*/
typedef XCsuDma_Configure XPmcDma_Configure;

/***************** Macros (Inline Functions) Definitions *********************/
#ifdef SPARTANUP
/*****************************************************************************/
/**
*
* This function resets the PMC_DMA core.
*
* @return	None.
*
******************************************************************************/
static INLINE void XPmcDma_Reset(void)
{
	Xil_Out32(((u32)PMC_GLOBAL_RST_DMA), (u32)(PMC_GLOBAL_RST_DMA_RESET_SET_MASK));
	Xil_Out32(((u32)PMC_GLOBAL_RST_DMA), (u32)(PMC_GLOBAL_RST_DMA_RESET_UNSET_MASK));
}
#else
/*****************************************************************************/
/**
*
* This function resets the PMC_DMA core.
*
* @param	DmaType is the type of DMA
*		XPMCDMA_DMATYPEIS_DMA0 or XPMCDMA_DMATYPEIS_DMA1).
*
* @return	None.
*
******************************************************************************/
static INLINE void XPmcDma_Reset(u32 DmaType)
{
	XCsuDma_PmcReset(DmaType);
}
#endif

/*****************************************************************************/
/**
* Waits for DMA operation to complete
*
* @param	InstancePtr Pointer to XPmcDma instance to be worked on.
* @param	Channel The type of channel.
*		Source channel      - XPMCDMA_SRC_CHANNEL
*		Destination Channel - XPMCDMA_DST_CHANNEL
*
* @return	None.
*
* @note		This function should be called after XPmcDma_Transfer in polled
*		mode to wait until the data gets transferred completely.
*
******************************************************************************/
static INLINE int XPmcDma_WaitForDone(XPmcDma *InstancePtr, XPmcDma_Channel Channel)
{
	XCsuDma_WaitForDone(InstancePtr, Channel);
	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* Returns the number of completed SRC/DST DMA transfers that
* have not been acknowledged by software based on the channel selection.
*
* @param	InstancePtr Pointer to XPmcDma instance to be worked on.
* @param	Channel The type of channel
*
*		Source channel      - XPMCDMA_SRC_CHANNEL
*		Destination Channel - XPMCDMA_DST_CHANNEL
*
* @return	Count Number of completed DMA transfers but not acknowledged
*		(Range is 0 to 7).
*		- 000 - All finished transfers have been acknowledged.
*		- Count - Count number of finished transfers are still
*		outstanding.
*
******************************************************************************/
static INLINE u32 XPmcDma_GetDoneCount(XPmcDma *InstancePtr, XPmcDma_Channel Channel)
{
	return XCsuDma_GetDoneCount(InstancePtr, Channel);
}

/*****************************************************************************/
/**
*
* Returns the current SRC/DST FIFO level in 32 bit words of the
* selected channel
* @param	InstancePtr Pointer to XPmcDma instance to be worked on.
* @param	Channel Represents the type of channel
*
*		Source channel      - XPMCDMA_SRC_CHANNEL
*		Destination Channel - XPMCDMA_DST_CHANNEL
*
* @return	FIFO level. (Range is 0 to 128)
*		- 0 Indicates empty
*		- Any number 1 to 128 indicates the number of entries in FIFO.
*
******************************************************************************/
static INLINE u32 XPmcDma_GetFIFOLevel(XPmcDma *InstancePtr, XPmcDma_Channel Channel)
{
	return XCsuDma_GetFIFOLevel(InstancePtr, Channel);
}

/*****************************************************************************/
/**
*
* Returns the current number of read(src)/write(dst) outstanding
* commands based on the type of channel selected.
*
* @param	InstancePtr Pointer to XPmcDma instance to be worked on.
* @param	Channel Represents the type of channel
*
*		Source channel      - XPMCDMA_SRC_CHANNEL
*		Destination Channel - XPMCDMA_DST_CHANNEL
*
* @return	Count of outstanding commands. (Range is 0 to 9).
*
******************************************************************************/
static INLINE u32 XPmcDma_GetWROutstandCount(XPmcDma *InstancePtr,
	XPmcDma_Channel Channel)
{
	return XCsuDma_GetWROutstandCount(InstancePtr, Channel);
}

/*****************************************************************************/
/**
*
* Returns the status of Channel either it is busy or not.
*
* @param	InstancePtr Pointer to XPmcDma instance to be worked on.
* @param	Channel The type of channel
*
*		Source channel      - XPMCDMA_SRC_CHANNEL
*		Destination Channel - XPMCDMA_DST_CHANNEL
*
* @return	Returns the current status of the core.
*		- TRUE represents core is currently busy.
*		- FALSE represents core is not involved in any transfers.
*
******************************************************************************/
static INLINE u32 XPmcDma_IsBusy(XPmcDma *InstancePtr, XPmcDma_Channel Channel)
{
	return XCsuDma_IsBusy(InstancePtr, Channel);
}

/*****************************************************************************/
/**
*
* XPmcDma_LookupConfig returns a reference to an XPmcDma_Config structure
* based on the unique device id, <i>DeviceId</i>. The return value will refer
* to an entry in the device configuration table defined in the xcsudma_g.c
* file.
*
* @param	DeviceId Unique device ID of the device for the lookup
*		operation.
*
* @return	CfgPtr Reference to a config record in the configuration
*		table (in xcsudma_g.c) corresponding to <i>DeviceId</i>, or
*		NULL if no match is found.
*
******************************************************************************/
#ifndef SDT
static INLINE XPmcDma_Config * XPmcDma_LookupConfig(u16 DeviceId)
{
	return XCsuDma_LookupConfig(DeviceId);
}
#else
static INLINE XPmcDma_Config * XPmcDma_LookupConfig(UINTPTR BaseAddress)
{
	return XCsuDma_LookupConfig(BaseAddress);
}
#endif

/*****************************************************************************/
/**
*
* Initializes an PMC_DMA core. This function must be called
* prior to using an PMC_DMA core. Initialization of an PMC_DMA includes setting
* up the instance data and ensuring the hardware is in a quiescent state.
*
* @param	InstancePtr Pointer to the XPmcDma instance.
* @param	CfgPtr Reference to a structure containing information
*		about a specific XPmcDma instance.
* @param	EffectiveAddr Device base address in the virtual memory
*		address space. The caller is responsible for keeping the
*		address mapping from EffectiveAddr to the device physical
*		base address unchanged once this function is invoked.
*		Unexpected errors may occur if the address mapping changes
*		after this function is called. If address translation is not
*		used, pass in the physical address instead.
*
* @return
*		- XST_SUCCESS if initialization was successful.
*
******************************************************************************/
static INLINE s32 XPmcDma_CfgInitialize(XPmcDma *InstancePtr, XPmcDma_Config *CfgPtr,
	u32 EffectiveAddr)
{
	return XCsuDma_CfgInitialize(InstancePtr, CfgPtr, EffectiveAddr);
}

/*****************************************************************************/
/**
*
* Sets the starting address and amount(size) of the data to be
* transferred from/to the memory through the AXI interface.
*
* @param	InstancePtr Pointer to XPmcDma instance to be worked on.
* @param	Channel Type of channel
*
*		Source channel      - XPMCDMA_SRC_CHANNEL
*		Destination Channel - XPMCDMA_DST_CHANNEL
* @param	Addr 64 bit variable which holds the starting address of
* 		data which needs to write into the memory(DST) (or read	from
* 		the memory(SRC)).
* @param	Size 32 bit variable which represents the number of 4 byte
* 		words needs to be transferred from starting address.
* @param	EnDataLast Triggers an end of message. It will enable or
* 		disable data_inp_last signal to stream interface when current
* 		command is completed. It is applicable only to source channel
* 		and neglected for destination channel.
* 		-	1 - Asserts data_inp_last signal.
* 		-	0 - data_inp_last will not be asserted.
*
* @return	None.
*
* @note		Data_inp_last signal is asserted simultaneously with the
* 		data_inp_valid signal associated with the final 32-bit word
*		transfer.
*
******************************************************************************/
static INLINE void XPmcDma_Transfer(XPmcDma *InstancePtr, XPmcDma_Channel Channel,
	u64 Addr, u32 Size, u8 EnDataLast)
{
	XCsuDma_Transfer(InstancePtr, Channel, Addr, Size, EnDataLast);
}

/*****************************************************************************/
/**
*
* Sets the starting address and amount(size) of the data to be
* transferred from/to the memory through the AXI interface.
* This function is useful for pmu processor when it wishes to do
* a 64-bit DMA transfer.
*
* @param	InstancePtr Pointer to XPmcDma instance to be worked on.
* @param	Channel Type of channel.
*
*		Source channel      - XPMCDMA_SRC_CHANNEL
*		Destination Channel - XPMCDMA_DST_CHANNEL
* @param	AddrLow 32 bit variable which holds the starting lower
* 		address of data which needs to write into the memory(DST)
* 		(or read from the memory(SRC)).
* @param	AddrHigh 32 bit variable which holds the higher address of data
* 		which needs to write into the memory(DST) (or read from
* 		the memory(SRC)).
* @param	Size 32 bit variable which represents the number of 4 byte
* 		words needs to be transferred from starting address.
* @param	EnDataLast Triggers an end of message. It will enable or
* 		disable data_inp_last signal to stream interface when current
* 		command is completed. It is applicable only to source channel
* 		and neglected for destination channel.
* 		-	1 - Asserts data_inp_last signal.
* 		-	0 - data_inp_last will not be asserted.
*
* @return	None.
*
* @note		Data_inp_last signal is asserted simultaneously with the
* 		data_inp_valid signal associated with the final 32-bit word
*		transfer
*		This API won't do flush/invalidation for the DMA buffer.
*		It is recommended to call this API only through PMU processor.
*
******************************************************************************/
static INLINE void XPmcDma_64BitTransfer(XPmcDma *InstancePtr, XPmcDma_Channel Channel,
	u32 AddrLow, u32 AddrHigh, u32 Size, u8 EnDataLast)
{
	XCsuDma_64BitTransfer(InstancePtr, Channel, AddrLow, AddrHigh,
		Size, EnDataLast);
}

/*****************************************************************************/
/**
*
* Returns the current address location of the memory, from where
* it has to read the data(SRC) or the location where it has to write the data
* (DST) based on the channel selection.
*
* @param	InstancePtr Pointer to XPmcDma instance to be worked on.
* @param	Channel Represents the type of channel
*
*		Source channel      - XPMCDMA_SRC_CHANNEL
*		Destination Channel - XPMCDMA_DST_CHANNEL
*
* @return	Address is a 64 bit variable which holds the current address.
*		- From this location data has to be read(SRC)
*		- At this location data has to be written(DST)
*
******************************************************************************/
static INLINE u64 XPmcDma_GetAddr(XPmcDma *InstancePtr, XPmcDma_Channel Channel)
{
	return XCsuDma_GetAddr(InstancePtr, Channel);
}

/*****************************************************************************/
/**
*
* Returns the size of the data yet to be transferred from memory
* to PMC_DMA or PMC_DMA to memory based on the channel selection.
*
* @param	InstancePtr Pointer to XPmcDma instance to be worked on.
* @param	Channel Represents the type of channel.
*
*		Source channel      - XPMCDMA_SRC_CHANNEL
*		Destination Channel - XPMCDMA_DST_CHANNEL
*
* @return	Size is amount of data yet to be transferred.
*
******************************************************************************/
static INLINE u32 XPmcDma_GetSize(XPmcDma *InstancePtr, XPmcDma_Channel Channel)
{
	return XCsuDma_GetSize(InstancePtr, Channel);
}

/*****************************************************************************/
/**
*
* Pauses the Channel data transfer to/from memory or to/from stream
* based on pause type.
*
* @param	InstancePtr Pointer to XPmcDma instance to be worked on.
* @param	Channel Represents the type of channel.
*
*		Source channel      - XPMCDMA_SRC_CHANNEL
*		Destination Channel - XPMCDMA_DST_CHANNEL
* @param	Type is type of the pause to be enabled.
*		- XPMCDMA_PAUSE_MEMORY(0) - Pause memory
*			- SRC Stops issuing of new read commands to memory.
*			- DST Stops issuing of new write commands to memory.
*		- XPMCDMA_PAUSE_STREAM(1) - Pause stream
*			- SRC Stops transfer of data from FIFO to Stream.
*			- DST Stops transfer of data from stream to FIFO.
*
* @return	None.
*
******************************************************************************/
static INLINE void XPmcDma_Pause(XPmcDma *InstancePtr, XPmcDma_Channel Channel,
	XPmcDma_PauseType Type)
{
	XCsuDma_Pause(InstancePtr, Channel,Type);
}

/*****************************************************************************/
/**
*
* Checks whether Channel's memory or stream is paused or not
* based on the given pause type.
*
* @param	InstancePtr Pointer to XPmcDma instance to be worked on.
* @param	Channel Represents the type of channel.
*
*		Source channel      - XPMCDMA_SRC_CHANNEL
*		Destination Channel - XPMCDMA_DST_CHANNEL
* @param	Type is type of the pause which needs to be checked.
*		- XPMCDMA_PAUSE_MEMORY(0) - Pause memory
*			- SRC Stops issuing of new read commands to memory.
*			- DST Stops issuing of new write commands to memory.
*		- XPMCDMA_PAUSE_STREAM(1) - Pause stream
*			- SRC Stops transfer of data from FIFO to Stream.
*			- DST Stops transfer of data from stream to FIFO.
*
* @return	Returns the pause status.
*		- TRUE if it is in paused state.
*		- FALSE if it is not in pause state.
*
******************************************************************************/
static INLINE s32 XPmcDma_IsPaused(XPmcDma *InstancePtr, XPmcDma_Channel Channel,
	XPmcDma_PauseType Type)
{
	return XCsuDma_IsPaused(InstancePtr, Channel, Type);
}

/*****************************************************************************/
/**
*
* Resumes the channel if it is in paused state and continues
* where it has left or no effect if it is not in paused state, based on the
* type of pause.
*
* @param	InstancePtr Pointer to XPmcDma instance to be worked on.
* @param	Channel Represents the type of channel.
*
*		Source channel      - XPMCDMA_SRC_CHANNEL
*		Destination Channel - XPMCDMA_DST_CHANNEL
* @param	Type is type of the pause to be Resume if it is in pause
*		state.
*		- XPMCDMA_PAUSE_MEMORY(0) - Resume memory
*			- SRC Resumes issuing of new read commands to memory.
*			- DST Resumes issuing of new write commands to memory.
*		- XPMCDMA_PAUSE_STREAM(1) - Resules stream
*			- SRC Resumes transfer of data from FIFO to Stream.
*			- DST Resumes transfer of data from stream to FIFO.
*
* @return	None.
*
******************************************************************************/
static INLINE void XPmcDma_Resume(XPmcDma *InstancePtr, XPmcDma_Channel Channel,
	XPmcDma_PauseType Type)
{
	XCsuDma_Resume(InstancePtr, Channel, Type);
}

/*****************************************************************************/
/**
*
* Returns the sum of all the data read from AXI memory. It is
* valid only one we use PMC_DMA source channel.
*
* @param	InstancePtr Pointer to XPmcDma instance to be worked on.
*
* @return	Returns the sum of all the data read from memory.
*
* @note		Before start of the transfer need to clear this register to get
*		correct sum otherwise it adds to previous value which results
*		to wrong output.
*		Valid only for source channel
*
******************************************************************************/
static INLINE u32 XPmcDma_GetCheckSum(XPmcDma *InstancePtr)
{
	return XCsuDma_GetCheckSum(InstancePtr);
}

/*****************************************************************************/
/**
*
* Clears the check sum of the data read from AXI memory. It is
* valid only for PMC_DMA source channel.
*
* @param	InstancePtr Pointer to XPmcDma instance to be worked on.
*
* @return	Returns the sum of all the data read from memory.
*
* @note		Before start of the transfer need to clear this register to get
*		correct sum otherwise it adds to previous value which results
*		to wrong output.
*
******************************************************************************/
static INLINE void XPmcDma_ClearCheckSum(XPmcDma *InstancePtr)
{
	XCsuDma_ClearCheckSum(InstancePtr);
}

/*****************************************************************************/
/**
* Configures all the values of PMC_DMA's Channels with the values
* of updated XPmcDma_Configure structure.
*
* @param	InstancePtr Pointer to XPmcDma instance to be worked on.
* @param	Channel Represents the type of channel.
*
*		Source channel      - XPMCDMA_SRC_CHANNEL
*		Destination Channel - XPMCDMA_DST_CHANNEL
* @param	ConfigurValues is a pointer to the structure XPmcDma_Configure
*		whose values are used to configure PMC_DMA core.
*		- SssFifoThesh   When the DST FIFO level >= this value,
*		  the SSS interface signal, "data_out_fifo_level_hit" will be
*		  asserted. This mechanism can be used by the SSS to flow
*		  control data that is being looped back from the SRC DMA.
*			- Range is (0x10 to 0x7A) threshold is 17 to 123
*			entries.
*			- It is valid only for DST PMC_DMA IP.
*		- ApbErr     When accessed to invalid APB the resulting
*		  pslerr will be
*			- 0 - 1'b0
*			- 1 - 1'b1
*		- EndianType      Type of endianness
*			- 0 doesn't change order
*			- 1 will flip the order.
*		- AxiBurstType     Type of the burst
*			- 0 will issue INCR type burst
*			- 1 will issue FIXED type burst
*		- TimeoutValue    Time out value for timers
*			- 0x000 to 0xFFE are valid inputs
*			- 0xFFF clears both timers
*		- FifoThresh     Programmed watermark value
*			- Range is 0x00 to 0x80 (0 to 128 entries).
*		- Acache         Sets the AXI CACHE bits on the AXI Write/Read
*		channel.
*			- Cacheable ARCACHE[1] for SRC Channel and AWCACHE[1]
*			  for DST channel are always 1, we need to configure
*			  remaining 3 signal support
*			  (Bufferable, Read allocate and Write allocate).
*			Valid inputs are:
*			- 0x000 - Cacheable, but do not allocate
*			- 0x001 - Cacheable and bufferable, but do not allocate
*			- 0x010 - Cacheable write-through, allocate on reads
*				  only
*			- 0x011 - Cacheable write-back, allocate on reads only
*			- 0x100 - Cacheable write-through, allocate on writes
*				  only
*			- 0x101 - Cacheable write-back, allocate on writes only
*			- 0x110 - Cacheable write-through, allocate on both
*				  reads and writes
*			- 0x111 - Cacheable write-back, allocate on both reads
*				  and writes
*		- RouteBit      To select route
*			- 0 : Command will be routed normally
*			- 1 : Command will be routed to APU's cache controller
*		- TimeoutEn      To enable or disable time out counters
*			- 0 : The 2 Timeout counters are disabled
*			- 1 : The 2 Timeout counters are enabled
*		- TimeoutPre     Set the prescaler value for the timeout in
*		clk (~1.6 ns) cycles
*			- Range is 0x000(Prescaler enables timer every cycles)
*			  to 0xFFF(Prescaler enables timer every 4096 cycles)
*		- MaxOutCmds      Controls the maximumum number of outstanding
*		AXI read commands issued.
*			- Range is 0x0(Up to 1 Outstanding Read command
*			  allowed) to 0x8 (Up to 9 Outstanding Read
*			  command allowed)
*
* @return	None.
*
* @note		To use timers timeout value Timeout enable field should be
*		enabled.
*
******************************************************************************/
static INLINE void XPmcDma_SetConfig(XPmcDma *InstancePtr, XPmcDma_Channel Channel,
	XPmcDma_Configure *ConfigurValues)
{
	XCsuDma_SetConfig(InstancePtr, Channel, ConfigurValues);
}

/*****************************************************************************/
/**
*
* Updates XPmcDma_Configure structure members with the configured
* values of PMC_DMA's Channel.
*
* @param	InstancePtr Pointer to XPmcDma instance to be worked on.
* @param	Channel Represents the type of channel.
*
*		Source channel      - XPMCDMA_SRC_CHANNEL
*		Destination Channel - XPMCDMA_DST_CHANNEL
* @param	ConfigurValues is a pointer to the structure XPmcDma_Configure
*		whose members are updated with configurations of PMC_DMA core.
*		- SssFifoThesh   When the DST FIFO level >= this value,
*		  the SSS interface signal, "data_out_fifo_level_hit" will be
*		  asserted. This mechanism can be used by the SSS to flow
*		  control data that is being looped back from the SRC DMA.
*			- Range is (0x10 to 0x7A) threshold is 17 to 123
*			entries.
*			- It is valid only for DST PMC_DMA IP.
*		- ApbErr          When accessed to invalid APB the resulting
*		  pslerr will be
*			- 0 - 1'b0
*			- 1 - 1'b1
*		- EndianType      Type of endianness
*			- 0 doesn't change order
*			- 1 will flip the order.
*		- AxiBurstType     Type of the burst
*			- 0 will issue INCR type burst
*			- 1 will issue FIXED type burst
*		- TimeoutValue    Time out value for timers
*			- 0x000 to 0xFFE are valid inputs
*			- 0xFFF clears both timers
*		- FifoThresh     Programmed watermark value
*			- Range is 0x00 to 0x80 (0 to 128 entries).
*		- Acache         Sets the AXI CACHE bits on the AXI Write/Read
*		channel.
*			- Cacheable ARCACHE[1] for SRC Channel and AWCACHE[1]
*			  for DST channel are always 1, we need to configure
*			  remaining 3 signal support
*			  (Bufferable, Read allocate and Write allocate).
*			Valid inputs are:
*			- 0x000 - Cacheable, but do not allocate
*			- 0x001 - Cacheable and bufferable, but do not allocate
*			- 0x010 - Cacheable write-through, allocate on reads
*				  only
*			- 0x011 - Cacheable write-back, allocate on reads only
*			- 0x100 - Cacheable write-through, allocate on writes
*				  only
*			- 0x101 - Cacheable write-back, allocate on writes only
*			- 0x110 - Cacheable write-through, allocate on both
*				  reads and writes
*			- 0x111 - Cacheable write-back, allocate on both reads
*				  and writes
*		- RouteBit        To select route
*			- 0 : Command will be routed based normally
*			- 1 : Command will be routed to APU's cache controller
*		- TimeoutEn       To enable or disable time out counters
*			- 0 : The 2 Timeout counters are disabled
*			- 1 : The 2 Timeout counters are enabled
*		- TimeoutPre      Set the prescaler value for the timeout in
*		clk (~1.6 ns) cycles
*			- Range is 0x000(Prescaler enables timer every cycles)
*			 to 0xFFF(Prescaler enables timer every 4096 cycles)
*		- MaxOutCmds      Controls the maximumum number of outstanding
*		AXI read commands issued.
*			- Range is 0x0(Up to 1 Outstanding Read command
*			allowed) to 0x8 (Up to 9 Outstanding Read command
*			allowed)
*
* @return	None.
*
******************************************************************************/
static INLINE void XPmcDma_GetConfig(XPmcDma *InstancePtr, XPmcDma_Channel Channel,
	XPmcDma_Configure *ConfigurValues)
{
	XCsuDma_GetConfig(InstancePtr, Channel,ConfigurValues);
}

/*****************************************************************************/
/**
* @brief	This function polls for completion of data transfer until
* 			 DMA done bit set or till the timeout occurs
*
* @param	InstancePtr - Pointer to XPmcDma instance to be worked on
* @param	Channel     - Represents the type of channel.
*
*							Source channel      - XPMCDMA_SRC_CHANNEL
*							Destination Channel - XPMCDMA_DST_CHANNEL
*
* @return	XST_SUCCESS - In case of Success
*			XST_FAILURE - In case of Timeout
*
******************************************************************************/
static INLINE int XPmcDma_WaitForDoneTimeout(XPmcDma *InstancePtr,
	XPmcDma_Channel Channel)
{
	return (int)XCsuDma_WaitForDoneTimeout(InstancePtr, Channel);
}

/* Interrupt related APIs */

/*****************************************************************************/
/**
*
* Returns interrupt status read from Interrupt Status Register.
* Use the XPMCDMA_IXR_*_MASK constants defined in xpmcdma.h to interpret the
* returned value.
*
* @param	InstancePtr Pointer to XPmcDma instance to be worked on.
* @param	Channel Represents the type of channel
*
*		Source channel      - XPMCDMA_SRC_CHANNEL
*		Destination Channel - XPMCDMA_DST_CHANNEL
*
* @return	The pending interrupts of the PMC_DMA. Use th following masks
*		to interpret the returned value.
*		XPMCDMA_IXR_SRC_MASK   - For Source channel
*		XPMCDMA_IXR_DST_MASK   - For Destination channel
*
******************************************************************************/
static INLINE u32 XPmcDma_IntrGetStatus(XPmcDma *InstancePtr, XPmcDma_Channel Channel)
{
	return XCsuDma_IntrGetStatus(InstancePtr, Channel);
}

/*****************************************************************************/
/**
*
* Returns interrupt status read from Interrupt Status Register.
* Use the XPMCDMA_IXR_*_MASK constants defined in xpmcdma.h to interpret the
* returned value.
*
* @param	InstancePtr Pointer to XPmcDma instance to be worked on.
* @param	Channel Represents the type of channel.
*
*		Source channel      - XPMCDMA_SRC_CHANNEL
*		Destination Channel - XPMCDMA_DST_CHANNEL
*
* @return	The pending interrupts of the PMC_DMA. Use th following masks
*		to interpret the returned value.
*		XPMCDMA_IXR_SRC_MASK   - For Source channel
*		XPMCDMA_IXR_DST_MASK   - For Destination channel
*
******************************************************************************/
static INLINE void XPmcDma_IntrClear(XPmcDma *InstancePtr, XPmcDma_Channel Channel,
	u32 Mask)
{
	XCsuDma_IntrClear(InstancePtr, Channel, Mask);
}

/*****************************************************************************/
/**
*
* This function enables the interrupt(s). Use the XPMCDMA_IXR_*_MASK constants
* defined in xpmcdma.h to create the bit-mask to enable interrupts.
*
* @param	InstancePtr Pointer to XPmcDma instance to be worked on.
* @param	Channel Represents the type of channel.
*
*		Source channel      - XPMCDMA_SRC_CHANNEL
*		Destination Channel - XPMCDMA_DST_CHANNEL
* @param	Mask contains interrupts to be enabled.
*		- Bit positions of 1 will be enabled.
*		This mask is formed by OR'ing XPMCDMA_IXR_*_MASK bits defined
*		in xpmcdma.h.
*
* @return	None.
*
******************************************************************************/
static INLINE void XPmcDma_EnableIntr(XPmcDma *InstancePtr, XPmcDma_Channel Channel,
	u32 Mask)
{
	XCsuDma_EnableIntr(InstancePtr, Channel, Mask);
}

/*****************************************************************************/
/**
*
* Disables the interrupt(s). Use the XPMCDMA_IXR_*_MASK constants
* defined in xpmcdma.h to create the bit-mask to disable interrupts.
*
* @param	InstancePtr Pointer to XPmcDma instance to be worked on.
* @param	Channel Represents the type of channel.
*
*		Source channel      - XPMCDMA_SRC_CHANNEL
*		Destination Channel - XPMCDMA_DST_CHANNEL
* @param	Mask contains interrupts to be disabled.
*		- Bit positions of 1 will be disabled.
*		This mask is formed by OR'ing XPMCDMA_IXR_*_MASK bits defined
*		in xpmcdma.h.
*
* @return	None.
*
******************************************************************************/
static INLINE void XPmcDma_DisableIntr(XPmcDma *InstancePtr, XPmcDma_Channel Channel,
	u32 Mask)
{
	XCsuDma_DisableIntr(InstancePtr, Channel, Mask);
}

/*****************************************************************************/
/**
*
* Returns the interrupt mask to know which interrupts are
* enabled and which of them were disaled.
*
* @param	InstancePtr Pointer to XPmcDma instance to be worked on.
* @param	Channel Represents the type of channel.
*
*		Source channel      - XPMCDMA_SRC_CHANNEL
*		Destination Channel - XPMCDMA_DST_CHANNEL
*
* @return	The current interrupt mask. The mask indicates which interrupts
*		are enabled/disabled.
*		0 bit represents .....corresponding interrupt is enabled.
*		1 bit represents .....Corresponding interrupt is disabled.
*		To interpret returned mask use
*		XPMCDMA_IXR_SRC_MASK........For source channel
*		XPMCDMA_IXR_DST_MASK........For destination channel
*
******************************************************************************/
static INLINE u32 XPmcDma_GetIntrMask(XPmcDma *InstancePtr, XPmcDma_Channel Channel)
{
	return XCsuDma_GetIntrMask(InstancePtr, Channel);
}

/*****************************************************************************/
/**
*
* Runs a self-test on the driver and hardware device. Performs
* reset of both source and destination channels and checks if reset is working
* properly or not.
*
* @param	InstancePtr Pointer to the XPmcDma instance.
*
* @return
*		- XST_SUCCESS if the self-test passed.
* 		- XST_FAILURE otherwise.
*
******************************************************************************/
static INLINE s32 XPmcDma_SelfTest(XPmcDma *InstancePtr)
{
	return XCsuDma_SelfTest(InstancePtr);
}

#if defined(VERSAL_NET) || defined(VERSAL_2VE_2VM)
/*****************************************************************************/
/**
*
* Sets the starting address and amount(size) of the data to be
* transferred from/to the memory through the AXI interface in VERSAL NET.
*
* @param	InstancePtr Pointer to XCsuDma instance to be worked on.
* @param	Channel Represents the type of channel.
*
*		Source channel      - XCSUDMA_SRC_CHANNEL
*		Destination Channel - XCSUDMA_DST_CHANNEL
* @param	Addr 64 bit variable which holds the starting address of
* 		data which needs to write into the memory(DST) (or read	from
* 		the memory(SRC)).
* @param	Size 32 bit variable which represents the number of bytes
* 		needs to be transferred from starting address.
* @param	EnDataLast Trigger an end of message. It will enable or
* 		disable data_inp_last signal to stream interface when current
* 		command is completed. It is applicable only to source channel
* 		and neglected for destination channel.
* 		-	1 - Asserts data_inp_last signal.
* 		-	0 - data_inp_last will not be asserted.
*
* @return	None.
*
* @note		Data_inp_last signal is asserted simultaneously with the
* 		data_inp_valid signal associated with the final 32-bit word
*		transfer.
*
******************************************************************************/
static INLINE void XPmcDma_ByteAlignedTransfer(XCsuDma *InstancePtr, XCsuDma_Channel Channel,
					u64 Addr, u32 Size, u8 EnDataLast)
{
	XCsuDma_ByteAlignedTransfer(InstancePtr, Channel, Addr, Size, EnDataLast);
}
#endif

/******************************************************************************/

#endif

#ifdef __cplusplus
}
#endif

#endif
