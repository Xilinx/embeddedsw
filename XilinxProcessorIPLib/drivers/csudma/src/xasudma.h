/******************************************************************************
* Copyright (C) 2024 Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/
/*****************************************************************************/
/**
*
* The file is a wrapper that calls APIs declared in xscudma.h This wrapper is
* added for Versal_2VE_2VM devices as there is no CSU DMA in Versal_2VE_2VM
* devices and hence the ASUFW should have no reference to CSU DMA. This is a
* security requirement.
*
* The ASU_DMA is present inside ASU (Application Security Unit) module.
* ASU_DMA allows the ASU to move data efficiently between the memory (128 bit
* AXI interface) and the ASU stream peripherals (SHA, AES and PLI) via Secure
* Stream Switch (SSS).
*
* The ASU_DMA is a 2 channel simple DMA, allowing separate control of the SRC
* (read) channel and DST (write) channel. The DMA is effectively able to
* transfer data:
*	- From ASU to the SSS-side (SRC DMA only)
*	- From SSS-side to the ASU (DST DMA only)
*	- Simultaneous ASU to SSS_side and SSS-side to ASU
*
* Initialization & Configuration
*
* The device driver enables higher layer software (e.g., an application) to
* communicate to the ASU_DMA core. The device driver internally calls CSU DMA
* APIs.
*
* XAsuDma_CfgInitialize() API is used to initialize the ASU_DMA core.
* The user needs to first call the XAsuDma_LookupConfig() API which returns
* the Configuration structure pointer which is passed as a parameter to the
* XAsuDma_CfgInitialize() API.
*
* Reset
* This driver will not support handling of ASU_GLOBAL DMA Reset in case of
* ASU_DMA inorder to support multiple level of handoff's. User needs to call
* the XAsuDma_Reset() API before performing any driver operation to make
* sure ASU_DMA is in proper state.
*
* Interrupts
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
* The XAsuDma driver is composed of several source files. This allows the user
* to build and link only those parts of the driver that are necessary.
*
* This header file contains identifiers and register-level driver functions (or
* macros), range macros, structure typedefs that can be used to access the
* Xilinx ASU_DMA core instance.
*
* The functionality wise ZU+ CSU_DMA and Versal_2VE_2VM ASU_DMA are similar, so
* all ZU+ code is reused by wrapping in this file
*
* MODIFICATION HISTORY:
*
* Ver   Who     Date     Changes
* ----- ------  -------- -----------------------------------------------------
* 1.0   mmd     03/23/24 Initial release
*
* </pre>
*
******************************************************************************/

#ifndef XASUDMA_H_
#define XASUDMA_H_

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/
#include "xcsudma.h"
#if defined (VERSAL_2VE_2VM)

/************************** Constant Definitions *****************************/
/** Ranges of Size */

#ifndef SDT
#define ASUDMA_0_DEVICE_ID      XPAR_XCSUDMA_0_DEVICE_ID /* ASUDMA device Id */
#define ASUDMA_1_DEVICE_ID      XPAR_XCSUDMA_1_DEVICE_ID /* ASUDMA device Id */
#else
#define ASUDMA_0_DEVICE_ID      XPAR_XCSUDMA_0_BASEADDR /* ASUDMA device Id */
#define ASUDMA_1_DEVICE_ID      XPAR_XCSUDMA_1_BASEADDR /* ASUDMA device Id */
#endif
#define ASUDMA_LOOPBACK_CFG     (0x0000000FU)   /* LOOP BACK configuration */

#define XASUDMA_SIZE_MAX 		XCSUDMA_SIZE_MAX /* Maximum allowed no of words */
#define XASUDMA_ADDR_LSB_MASK	XCSUDMA_ADDR_LSB_MASK

#define XASUDMA_DMATYPEIS_INVALID	XCSUDMA_DMATYPEIS_CSUDMA
#define XASUDMA_DMATYPEIS_DMA0	XCSUDMA_DMATYPEIS_ASUDMA0
#define XASUDMA_DMATYPEIS_DMA1	XCSUDMA_DMATYPEIS_ASUDMA1

#define XASUDMA_SRC_CHANNEL	XCSUDMA_SRC_CHANNEL
#define XASUDMA_DST_CHANNEL	XCSUDMA_DST_CHANNEL

#define XASUDMA_PAUSE_MEMORY	XCSUDMA_PAUSE_MEMORY
#define XASUDMA_PAUSE_STREAM	XCSUDMA_PAUSE_STREAM

/** Interrupt Enable/Disable/Mask/Status registers bit masks */
#define XASUDMA_IXR_FIFO_OVERFLOW_MASK		XCSUDMA_IXR_FIFO_OVERFLOW_MASK
/**< FIFO overflow mask, it is valid only to Destination Channel */
#define XASUDMA_IXR_INVALID_APB_MASK 		XCSUDMA_IXR_INVALID_APB_MASK
/**< Invalid APB access mask */
#define XASUDMA_IXR_FIFO_THRESHHIT_MASK		XCSUDMA_IXR_FIFO_THRESHHIT_MASK
/**< FIFO threshold hit indicator mask */
#define XASUDMA_IXR_TIMEOUT_MEM_MASK		XCSUDMA_IXR_TIMEOUT_MEM_MASK
/**< Time out counter expired to access memory mask */
#define XASUDMA_IXR_TIMEOUT_STRM_MASK		XCSUDMA_IXR_TIMEOUT_STRM_MASK
/**< Time out counter expired to access stream mask */
#define XASUDMA_IXR_AXI_WRERR_MASK		XCSUDMA_IXR_AXI_WRERR_MASK
/**< AXI Read/Write error mask */
#define XASUDMA_IXR_DONE_MASK			XCSUDMA_IXR_DONE_MASK
/**< Done mask */
#define XASUDMA_IXR_MEM_DONE_MASK		XCSUDMA_IXR_MEM_DONE_MASK
/**< Memory done mask, it is valid only for source channel*/
#define XASUDMA_IXR_SRC_MASK			XCSUDMA_IXR_SRC_MASK
/**< All interrupt mask for source */
#define XASUDMA_IXR_DST_MASK			XCSUDMA_IXR_DST_MASK
/**< All interrupt mask for destination */

/**************************** Type Definitions *******************************/
/**
 * This typedef contains ASU_DMA Channel Types.
 * Note that the enum values are mapped with #define above as
 * 	XASUDMA_SRC_CHANNEL = XCSUDMA_SRC_CHANNEL
 *	XASUDMA_DST_CHANNEL = XCSUDMA_DST_CHANNEL
 */
typedef XCsuDma_Channel		XAsuDma_Channel;

/**
 * This typedef contains ASU_DMA Pause Types.
 * Note that the enum values are mapped with #define above as
 *	XASUDMA_PAUSE_MEMORY = XCSUDMA_PAUSE_MEMORY
 *	XASUDMA_PAUSE_STREAM = XCSUDMA_PAUSE_STREAM
 */
typedef XCsuDma_PauseType	XAsuDma_PauseType;
/**
* This typedef contains configuration information for a ASU_DMA core.
* Each ASU_DMA core should have a configuration structure associated.
*/
typedef XCsuDma_Config 	XAsuDma_Config;

/******************************************************************************/
/**
*
* The XAsuDma driver instance data structure. A pointer to an instance data
* structure is passed around by functions to refer to a specific driver
* instance.
*/
typedef XCsuDma XAsuDma;

/******************************************************************************/
/**
* This typedef contains all the configuration fields which needs to be set
* before the start of the data transfer. All these feilds of ASU_DMA can be
* configured by using XAsuDma_SetConfig API.
*/
typedef XCsuDma_Configure XAsuDma_Configure;

/***************** Macros (Inline Functions) Definitions *********************/
/** ASU Global DMA soft reset address */
#define XASU_GLOBAL_DMA_SOFT_RST_ADDR		0xEBF90050U
/** ASU Global DMA1 soft reset address */
#define XASU_GLOBAL_DMA1_SOFT_RST_ADDR		0xEBF90058U

/*****************************************************************************/
/**
*
* This function resets the ASU_DMA core.
*
* @param	Addr is the address of ASU DMA reset register (ASUDMA0 or ASUDMA1).
*
* @return	None.
*
* @note		None.
*		C-style signature:
*		void XAsuDma_AsuReset(u32 Addr)
*
******************************************************************************/
#define XAsuDma_AsuReset(Addr)  \
	Xil_Out32((u32)(Addr), (u32)(XCSUDMA_RESET_SET_MASK)); \
	Xil_Out32((u32)(Addr), (u32)(XCSUDMA_RESET_UNSET_MASK));

/*****************************************************************************/
/**
*
* This function resets the ASU_DMA core.
*
* @param	DmaType is the type of DMA
*		XASUDMA_DMATYPEIS_DMA0 or XASUDMA_DMATYPEIS_DMA1).
*
* @return	None.
*
******************************************************************************/
static INLINE void XAsuDma_Reset(u32 DmaType)
{
	if (DmaType == XASUDMA_DMATYPEIS_DMA0) {
		XAsuDma_AsuReset(XASU_GLOBAL_DMA_SOFT_RST_ADDR);
	} else {
		XAsuDma_AsuReset(XASU_GLOBAL_DMA1_SOFT_RST_ADDR);
	}
}

/*****************************************************************************/
/**
* This function will wait for DMA operation to complete
*
* @param	InstancePtr is a pointer to XAsuDma instance to be worked on.
* @param	Channel represents the type of channel either it is Source or
*		Destination.
*		Source channel      - XASUDMA_SRC_CHANNEL
*		Destination Channel - XASUDMA_DST_CHANNEL
*
* @return	None.
*
* @note		This function should be called after XAsuDma_Transfer in polled
*		mode to wait until the data gets transferred completely.
*
******************************************************************************/
static INLINE int XAsuDma_WaitForDone(XAsuDma *InstancePtr, XAsuDma_Channel Channel)
{
	XCsuDma_WaitForDone(InstancePtr, Channel);
	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* This function returns the number of completed SRC/DST DMA transfers that
* have not been acknowledged by software based on the channel selection.
*
* @param	InstancePtr is a pointer to XAsuDma instance to be worked on.
* @param	Channel represents the type of channel either it is Source or
* 		Destination.
*		Source channel      - XASUDMA_SRC_CHANNEL
*		Destination Channel - XASUDMA_DST_CHANNEL
*
* @return	Count is number of completed DMA transfers but not acknowledged
*		(Range is 0 to 7).
*		- 000 - All finished transfers have been acknowledged.
*		- Count - Count number of finished transfers are still
*		outstanding.
*
******************************************************************************/
static INLINE u32 XAsuDma_GetDoneCount(XAsuDma *InstancePtr, XAsuDma_Channel Channel)
{
	return XCsuDma_GetDoneCount(InstancePtr, Channel);
}

/*****************************************************************************/
/**
*
* This function returns the current SRC/DST FIFO level in 32 bit words of the
* selected channel
* @param	InstancePtr is a pointer to XAsuDma instance to be worked on.
* @param	Channel represents the type of channel either it is Source or
* 		Destination.
*		Source channel      - XASUDMA_SRC_CHANNEL
*		Destination Channel - XASUDMA_DST_CHANNEL
*
* @return	FIFO level. (Range is 0 to 128)
*		- 0 Indicates empty
*		- Any number 1 to 128 indicates the number of entries in FIFO.
*
******************************************************************************/
static INLINE u32 XAsuDma_GetFIFOLevel(XAsuDma *InstancePtr, XAsuDma_Channel Channel)
{
	return XCsuDma_GetFIFOLevel(InstancePtr, Channel);
}

/*****************************************************************************/
/**
*
* This function returns the current number of read(src)/write(dst) outstanding
* commands based on the type of channel selected.
*
* @param	InstancePtr is a pointer to XAsuDma instance to be worked on.
* @param	Channel represents the type of channel either it is Source or
* 		Destination.
*		Source channel      - XASUDMA_SRC_CHANNEL
*		Destination Channel - XASUDMA_DST_CHANNEL
*
* @return	Count of outstanding commands. (Range is 0 to 9).
*
******************************************************************************/
static INLINE u32 XAsuDma_GetWROutstandCount(XAsuDma *InstancePtr,
	XAsuDma_Channel Channel)
{
	return XCsuDma_GetWROutstandCount(InstancePtr, Channel);
}

/*****************************************************************************/
/**
*
* This function returns the status of Channel either it is busy or not.
*
* @param	InstancePtr is a pointer to XAsuDma instance to be worked on.
* @param	Channel represents the type of channel either it is Source or
* 		Destination.
*		Source channel      - XASUDMA_SRC_CHANNEL
*		Destination Channel - XASUDMA_DST_CHANNEL
*
* @return	Returns the current status of the core.
*		- TRUE represents core is currently busy.
*		- FALSE represents core is not involved in any transfers.
*
******************************************************************************/
static INLINE u32 XAsuDma_IsBusy(XAsuDma *InstancePtr, XAsuDma_Channel Channel)
{
	return XCsuDma_IsBusy(InstancePtr, Channel);
}

/*****************************************************************************/
/**
*
* XAsuDma_LookupConfig returns a reference to an XAsuDma_Config structure
* based on the unique device id, <i>DeviceId</i>. The return value will refer
* to an entry in the device configuration table defined in the xcsudma_g.c
* file.
*
* @param	DeviceId is the unique device ID of the device for the lookup
*		operation.
*
* @return	CfgPtr is a reference to a config record in the configuration
*		table (in xcsudma_g.c) corresponding to <i>DeviceId</i>, or
*		NULL if no match is found.
*
******************************************************************************/
#ifndef SDT
static INLINE XAsuDma_Config *XAsuDma_LookupConfig(u16 DeviceId)
{
	return XCsuDma_LookupConfig(DeviceId);
}
#else
static INLINE XAsuDma_Config *XAsuDma_LookupConfig(UINTPTR BaseAddress)
{
	return XCsuDma_LookupConfig(BaseAddress);
}
#endif

/*****************************************************************************/
/**
*
* This function initializes an ASU_DMA core. This function must be called
* prior to using an ASU_DMA core. Initialization of an ASU_DMA includes setting
* up the instance data and ensuring the hardware is in a quiescent state.
*
* @param	InstancePtr is a pointer to the XAsuDma instance.
* @param	CfgPtr is a reference to a structure containing information
*		about a specific XAsuDma instance.
* @param	EffectiveAddr is the device base address in the virtual memory
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
static INLINE s32 XAsuDma_CfgInitialize(XAsuDma *InstancePtr, XAsuDma_Config *CfgPtr,
					u32 EffectiveAddr)
{
	return XCsuDma_CfgInitialize(InstancePtr, CfgPtr, EffectiveAddr);
}

/*****************************************************************************/
/**
*
* This function sets the starting address and amount(size) of the data to be
* transferred from/to the memory through the AXI interface.
*
* @param	InstancePtr is a pointer to XAsuDma instance to be worked on.
* @param	Channel represents the type of channel either it is Source or
* 		Destination.
*		Source channel      - XASUDMA_SRC_CHANNEL
*		Destination Channel - XASUDMA_DST_CHANNEL
* @param	Addr is a 64 bit variable which holds the starting address of
* 		data which needs to write into the memory(DST) (or read	from
* 		the memory(SRC)).
* @param	Size is a 32 bit variable which represents the number of 4 byte
* 		words needs to be transferred from starting address.
* @param	EnDataLast is to trigger an end of message. It will enable or
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
static INLINE void XAsuDma_Transfer(XAsuDma *InstancePtr, XAsuDma_Channel Channel,
				    u64 Addr, u32 Size, u8 EnDataLast)
{
	XCsuDma_Transfer(InstancePtr, Channel, Addr, Size, EnDataLast);
}

/*****************************************************************************/
/**
*
* This function sets the starting address and amount(size) of the data to be
* transferred from/to the memory through the AXI interface.
* This function is useful for ASU processor when it wishes to do
* a 64-bit DMA transfer.
*
* @param	InstancePtr is a pointer to XAsuDma instance to be worked on.
* @param	Channel represents the type of channel either it is Source or
* 		Destination.
*		Source channel      - XASUDMA_SRC_CHANNEL
*		Destination Channel - XASUDMA_DST_CHANNEL
* @param	AddrLow is a 32 bit variable which holds the starting lower
* 		address of data which needs to write into the memory(DST)
* 		(or read from the memory(SRC)).
* @param	AddrHigh is a 32 bit variable which holds the higher address of data
* 		which needs to write into the memory(DST) (or read from
* 		the memory(SRC)).
* @param	Size is a 32 bit variable which represents the number of 4 byte
* 		words needs to be transferred from starting address.
* @param	EnDataLast is to trigger an end of message. It will enable or
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
*		It is recommended to call this API only through ASU processor.
*
******************************************************************************/
static INLINE void XAsuDma_64BitTransfer(XAsuDma *InstancePtr, XAsuDma_Channel Channel,
	u32 AddrLow, u32 AddrHigh, u32 Size, u8 EnDataLast)
{
	XCsuDma_64BitTransfer(InstancePtr, Channel, AddrLow, AddrHigh,
			      Size, EnDataLast);
}

/*****************************************************************************/
/**
*
* This function returns the current address location of the memory, from where
* it has to read the data(SRC) or the location where it has to write the data
* (DST) based on the channel selection.
*
* @param	InstancePtr is a pointer to XAsuDma instance to be worked on.
* @param	Channel represents the type of channel either it is Source or
* 		Destination.
*		Source channel      - XASUDMA_SRC_CHANNEL
*		Destination Channel - XASUDMA_DST_CHANNEL
*
* @return	Address is a 64 bit variable which holds the current address.
*		- From this location data has to be read(SRC)
*		- At this location data has to be written(DST)
*
******************************************************************************/
static INLINE u64 XAsuDma_GetAddr(XAsuDma *InstancePtr, XAsuDma_Channel Channel)
{
	return XCsuDma_GetAddr(InstancePtr, Channel);
}

/*****************************************************************************/
/**
*
* This function returns the size of the data yet to be transferred from memory
* to ASU_DMA or ASU_DMA to memory based on the channel selection.
*
* @param	InstancePtr is a pointer to XAsuDma instance to be worked on.
* @param	Channel represents the type of channel either it is Source or
* 		Destination.
*		Source channel      - XASUDMA_SRC_CHANNEL
*		Destination Channel - XASUDMA_DST_CHANNEL
*
* @return	Size is amount of data yet to be transferred.
*
******************************************************************************/
static INLINE u32 XAsuDma_GetSize(XAsuDma *InstancePtr, XAsuDma_Channel Channel)
{
	return XCsuDma_GetSize(InstancePtr, Channel);
}

/*****************************************************************************/
/**
*
* This function pause the Channel data transfer to/from memory or to/from stream
* based on pause type.
*
* @param	InstancePtr is a pointer to XAsuDma instance to be worked on.
* @param	Channel represents the type of channel either it is Source or
* 		Destination.
*		Source channel      - XASUDMA_SRC_CHANNEL
*		Destination Channel - XASUDMA_DST_CHANNEL
* @param	Type is type of the pause to be enabled.
*		- XASUDMA_PAUSE_MEMORY(0) - Pause memory
*			- SRC Stops issuing of new read commands to memory.
*			- DST Stops issuing of new write commands to memory.
*		- XASUDMA_PAUSE_STREAM(1) - Pause stream
*			- SRC Stops transfer of data from FIFO to Stream.
*			- DST Stops transfer of data from stream to FIFO.
*
* @return	None.
*
******************************************************************************/
static INLINE void XAsuDma_Pause(XAsuDma *InstancePtr, XAsuDma_Channel Channel,
				 XAsuDma_PauseType Type)
{
	XCsuDma_Pause(InstancePtr, Channel, Type);
}

/*****************************************************************************/
/**
*
* This functions checks whether Channel's memory or stream is paused or not
* based on the given pause type.
*
* @param	InstancePtr is a pointer to XAsuDma instance to be worked on.
* @param	Channel represents the type of channel either it is Source or
* 		Destination.
*		Source channel      - XASUDMA_SRC_CHANNEL
*		Destination Channel - XASUDMA_DST_CHANNEL
* @param	Type is type of the pause which needs to be checked.
*		- XASUDMA_PAUSE_MEMORY(0) - Pause memory
*			- SRC Stops issuing of new read commands to memory.
*			- DST Stops issuing of new write commands to memory.
*		- XASUDMA_PAUSE_STREAM(1) - Pause stream
*			- SRC Stops transfer of data from FIFO to Stream.
*			- DST Stops transfer of data from stream to FIFO.
*
* @return	Returns the pause status.
*		- TRUE if it is in paused state.
*		- FALSE if it is not in pause state.
*
******************************************************************************/
static INLINE s32 XAsuDma_IsPaused(XAsuDma *InstancePtr, XAsuDma_Channel Channel,
				   XAsuDma_PauseType Type)
{
	return XCsuDma_IsPaused(InstancePtr, Channel, Type);
}

/*****************************************************************************/
/**
*
* This function resumes the channel if it is in paused state and continues
* where it has left or no effect if it is not in paused state, based on the
* type of pause.
*
* @param	InstancePtr is a pointer to XAsuDma instance to be worked on.
* @param	Channel represents the type of channel either it is Source or
* 		Destination.
*		Source channel      - XASUDMA_SRC_CHANNEL
*		Destination Channel - XASUDMA_DST_CHANNEL
* @param	Type is type of the pause to be Resume if it is in pause
*		state.
*		- XASUDMA_PAUSE_MEMORY(0) - Resume memory
*			- SRC Resumes issuing of new read commands to memory.
*			- DST Resumes issuing of new write commands to memory.
*		- XASUDMA_PAUSE_STREAM(1) - Resules stream
*			- SRC Resumes transfer of data from FIFO to Stream.
*			- DST Resumes transfer of data from stream to FIFO.
*
* @return	None.
*
******************************************************************************/
static INLINE void XAsuDma_Resume(XAsuDma *InstancePtr, XAsuDma_Channel Channel,
				  XAsuDma_PauseType Type)
{
	XCsuDma_Resume(InstancePtr, Channel, Type);
}

/*****************************************************************************/
/**
*
* This function returns the sum of all the data read from AXI memory. It is
* valid only one we use ASU_DMA source channel.
*
* @param	InstancePtr is a pointer to XAsuDma instance to be worked on.
*
* @return	Returns the sum of all the data read from memory.
*
* @note		Before start of the transfer need to clear this register to get
*		correct sum otherwise it adds to previous value which results
*		to wrong output.
*		Valid only for source channel
*
******************************************************************************/
static INLINE u32 XAsuDma_GetCheckSum(XAsuDma *InstancePtr)
{
	return XCsuDma_GetCheckSum(InstancePtr);
}

/*****************************************************************************/
/**
*
* This function clears the check sum of the data read from AXI memory. It is
* valid only for ASU_DMA source channel.
*
* @param	InstancePtr is a pointer to XAsuDma instance to be worked on.
*
* @return	Returns the sum of all the data read from memory.
*
* @note		Before start of the transfer need to clear this register to get
*		correct sum otherwise it adds to previous value which results
*		to wrong output.
*
******************************************************************************/
static INLINE void XAsuDma_ClearCheckSum(XAsuDma *InstancePtr)
{
	XCsuDma_ClearCheckSum(InstancePtr);
}

/*****************************************************************************/
/**
* This function configures all the values of ASU_DMA's Channels with the values
* of updated XAsuDma_Configure structure.
*
* @param	InstancePtr is a pointer to XAsuDma instance to be worked on.
* @param	Channel represents the type of channel either it is Source or
* 		Destination.
*		Source channel      - XASUDMA_SRC_CHANNEL
*		Destination Channel - XASUDMA_DST_CHANNEL
* @param	ConfigurValues is a pointer to the structure XAsuDma_Configure
*		whose values are used to configure ASU_DMA core.
*		- SssFifoThesh   When the DST FIFO level >= this value,
*		  the SSS interface signal, "data_out_fifo_level_hit" will be
*		  asserted. This mechanism can be used by the SSS to flow
*		  control data that is being looped back from the SRC DMA.
*			- Range is (0x10 to 0x7A) threshold is 17 to 123
*			entries.
*			- It is valid only for DST ASU_DMA IP.
*		- ApbErr          When accessed to invalid APB the resulting
*		  pslerr will be
*			- 0 - 1'b0
*			- 1 - 1'b1
*		- EndianType      Type of endianness
*			- 0 doesn't change order
*			- 1 will flip the order.
*		- AxiBurstType....Type of the burst
*			- 0 will issue INCR type burst
*			- 1 will issue FIXED type burst
*		- TimeoutValue    Time out value for timers
*			- 0x000 to 0xFFE are valid inputs
*			- 0xFFF clears both timers
*		- FifoThresh......Programmed watermark value
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
*			- 0 : Command will be routed normally
*			- 1 : Command will be routed to APU's cache controller
*		- TimeoutEn       To enable or disable time out counters
*			- 0 : The 2 Timeout counters are disabled
*			- 1 : The 2 Timeout counters are enabled
*		- TimeoutPre      Set the prescaler value for the timeout in
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
static INLINE void XAsuDma_SetConfig(XAsuDma *InstancePtr, XAsuDma_Channel Channel,
				     XAsuDma_Configure *ConfigurValues)
{
	XCsuDma_SetConfig(InstancePtr, Channel, ConfigurValues);
}

/*****************************************************************************/
/**
*
* This function updates XAsuDma_Configure structure members with the configured
* values of ASU_DMA's Channel.
*
* @param	InstancePtr is a pointer to XAsuDma instance to be worked on.
* @param	Channel represents the type of channel either it is Source or
* 		Destination.
*		Source channel      - XASUDMA_SRC_CHANNEL
*		Destination Channel - XASUDMA_DST_CHANNEL
* @param	ConfigurValues is a pointer to the structure XAsuDma_Configure
*		whose members are updated with configurations of ASU_DMA core.
*		- SssFifoThesh   When the DST FIFO level >= this value,
*		  the SSS interface signal, "data_out_fifo_level_hit" will be
*		  asserted. This mechanism can be used by the SSS to flow
*		  control data that is being looped back from the SRC DMA.
*			- Range is (0x10 to 0x7A) threshold is 17 to 123
*			entries.
*			- It is valid only for DST ASU_DMA IP.
*		- ApbErr          When accessed to invalid APB the resulting
*		  pslerr will be
*			- 0 - 1'b0
*			- 1 - 1'b1
*		- EndianType      Type of endianness
*			- 0 doesn't change order
*			- 1 will flip the order.
*		- AxiBurstType....Type of the burst
*			- 0 will issue INCR type burst
*			- 1 will issue FIXED type burst
*		- TimeoutValue    Time out value for timers
*			- 0x000 to 0xFFE are valid inputs
*			- 0xFFF clears both timers
*		- FifoThresh......Programmed watermark value
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
static INLINE void XAsuDma_GetConfig(XAsuDma *InstancePtr, XAsuDma_Channel Channel,
				     XAsuDma_Configure *ConfigurValues)
{
	XCsuDma_GetConfig(InstancePtr, Channel, ConfigurValues);
}

/*****************************************************************************/
/**
* @brief	This function will poll for completion of data transfer until
* 			 DMA done bit set or till the timeout occurs
*
* @param	InstancePtr - Is a pointer to XAsuDma instance to be worked on
* @param	Channel     - Represents the type of channel either it is Source or
*						  Destination
*							Source channel      - XASUDMA_SRC_CHANNEL
*							Destination Channel - XASUDMA_DST_CHANNEL
*
* @return	XST_SUCCESS - In case of Success
*			XST_FAILURE - In case of Timeout
*
******************************************************************************/
static INLINE int XAsuDma_WaitForDoneTimeout(XAsuDma *InstancePtr,
	XAsuDma_Channel Channel)
{
	return (int)XCsuDma_WaitForDoneTimeout(InstancePtr, Channel);
}

/* Interrupt related APIs */

/*****************************************************************************/
/**
*
* This function returns interrupt status read from Interrupt Status Register.
* Use the XASUDMA_IXR_*_MASK constants defined in xasudma.h to interpret the
* returned value.
*
* @param	InstancePtr is a pointer to XAsuDma instance to be worked on.
* @param	Channel represents the type of channel either it is Source or
*		Destination.
*		Source channel      - XASUDMA_SRC_CHANNEL
*		Destination Channel - XASUDMA_DST_CHANNEL
*
* @return	The pending interrupts of the ASU_DMA. Use th following masks
*		to interpret the returned value.
*		XASUDMA_IXR_SRC_MASK   - For Source channel
*		XASUDMA_IXR_DST_MASK   - For Destination channel
*
******************************************************************************/
static INLINE u32 XAsuDma_IntrGetStatus(XAsuDma *InstancePtr, XAsuDma_Channel Channel)
{
	return XCsuDma_IntrGetStatus(InstancePtr, Channel);
}

/*****************************************************************************/
/**
*
* This function returns interrupt status read from Interrupt Status Register.
* Use the XASUDMA_IXR_*_MASK constants defined in xasudma.h to interpret the
* returned value.
*
* @param	InstancePtr is a pointer to XAsuDma instance to be worked on.
* @param	Channel represents the type of channel either it is Source or
*		Destination.
*		Source channel      - XASUDMA_SRC_CHANNEL
*		Destination Channel - XASUDMA_DST_CHANNEL
*
* @return	The pending interrupts of the ASU_DMA. Use th following masks
*		to interpret the returned value.
*		XASUDMA_IXR_SRC_MASK   - For Source channel
*		XASUDMA_IXR_DST_MASK   - For Destination channel
*
******************************************************************************/
static INLINE void XAsuDma_IntrClear(XAsuDma *InstancePtr, XAsuDma_Channel Channel,
				     u32 Mask)
{
	XCsuDma_IntrClear(InstancePtr, Channel, Mask);
}

/*****************************************************************************/
/**
*
* This function enables the interrupt(s). Use the XASUDMA_IXR_*_MASK constants
* defined in xasudma.h to create the bit-mask to enable interrupts.
*
* @param	InstancePtr is a pointer to XAsuDma instance to be worked on.
* @param	Channel represents the type of channel either it is Source or
*		Destination.
*		Source channel      - XASUDMA_SRC_CHANNEL
*		Destination Channel - XASUDMA_DST_CHANNEL
* @param	Mask contains interrupts to be enabled.
*		- Bit positions of 1 will be enabled.
*		This mask is formed by OR'ing XASUDMA_IXR_*_MASK bits defined
*		in xasudma.h.
*
* @return	None.
*
******************************************************************************/
static INLINE void XAsuDma_EnableIntr(XAsuDma *InstancePtr, XAsuDma_Channel Channel,
				      u32 Mask)
{
	XCsuDma_EnableIntr(InstancePtr, Channel, Mask);
}

/*****************************************************************************/
/**
*
* This function disables the interrupt(s). Use the XASUDMA_IXR_*_MASK constants
* defined in xasudma.h to create the bit-mask to disable interrupts.
*
* @param	InstancePtr is a pointer to XAsuDma instance to be worked on.
* @param	Channel represents the type of channel either it is Source or
*		Destination.
*		Source channel      - XASUDMA_SRC_CHANNEL
*		Destination Channel - XASUDMA_DST_CHANNEL
* @param	Mask contains interrupts to be disabled.
*		- Bit positions of 1 will be disabled.
*		This mask is formed by OR'ing XASUDMA_IXR_*_MASK bits defined
*		in xasudma.h.
*
* @return	None.
*
******************************************************************************/
static INLINE void XAsuDma_DisableIntr(XAsuDma *InstancePtr, XAsuDma_Channel Channel,
				       u32 Mask)
{
	XCsuDma_DisableIntr(InstancePtr, Channel, Mask);
}

/*****************************************************************************/
/**
*
* This function returns the interrupt mask to know which interrupts are
* enabled and which of them were disaled.
*
* @param	InstancePtr is a pointer to XAsuDma instance to be worked on.
* @param	Channel represents the type of channel either it is Source or
*		Destination.
*		Source channel      - XASUDMA_SRC_CHANNEL
*		Destination Channel - XASUDMA_DST_CHANNEL
*
* @return	The current interrupt mask. The mask indicates which interrupts
*		are enabled/disabled.
*		0 bit represents .....corresponding interrupt is enabled.
*		1 bit represents .....Corresponding interrupt is disabled.
*		To interpret returned mask use
*		XASUDMA_IXR_SRC_MASK........For source channel
*		XASUDMA_IXR_DST_MASK........For destination channel
*
******************************************************************************/
static INLINE u32 XAsuDma_GetIntrMask(XAsuDma *InstancePtr, XAsuDma_Channel Channel)
{
	return XCsuDma_GetIntrMask(InstancePtr, Channel);
}

/*****************************************************************************/
/**
*
* This function runs a self-test on the driver and hardware device. Performs
* reset of both source and destination channels and checks if reset is working
* properly or not.
*
* @param	InstancePtr is a pointer to the XAsuDma instance.
*
* @return
*		- XST_SUCCESS if the self-test passed.
* 		- XST_FAILURE otherwise.
*
******************************************************************************/
static INLINE s32 XAsuDma_SelfTest(XAsuDma *InstancePtr)
{
	return XCsuDma_SelfTest(InstancePtr);
}

/*****************************************************************************/
/**
*
* This function sets the starting address and amount(size) of the data to be
* transferred from/to the memory through the AXI interface in VERSAL NET.
*
* @param	InstancePtr is a pointer to XAsuDma instance to be worked on.
* @param	Channel represents the type of channel either it is Source or
* 		Destination.
*		Source channel      - XASUDMA_SRC_CHANNEL
*		Destination Channel - XASUDMA_DST_CHANNEL
* @param	Addr is a 64 bit variable which holds the starting address of
* 		data which needs to write into the memory(DST) (or read	from
* 		the memory(SRC)).
* @param	Size is a 32 bit variable which represents the number of bytes
* 		needs to be transferred from starting address.
* @param	EnDataLast is to trigger an end of message. It will enable or
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
static INLINE void XAsuDma_ByteAlignedTransfer(XAsuDma *InstancePtr, XAsuDma_Channel Channel,
	u64 Addr, u32 Size, u8 EnDataLast)
{
	XCsuDma_ByteAlignedTransfer(InstancePtr, Channel, Addr, Size, EnDataLast);
}

/******************************************************************************/

#endif /* VERSAL_2VE_2VM */

#ifdef __cplusplus
}
#endif

#endif /* XASUDMA_H */
