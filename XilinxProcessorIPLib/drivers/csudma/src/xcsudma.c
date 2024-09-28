/******************************************************************************
* Copyright (C) 2014 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2022 - 2024 Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
*
* @file xcsudma.c
* @addtogroup csuma_api CSUDMA APIs
* @{
*
* This section contains the functions of the CSU_DMA driver.
* Refer to the header file xcsudma.h for more detailed information.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who     Date     Changes
* ----- ------  -------- ---------------------------------------------------
* 1.0   vnsld   22/10/14 First release
* 1.1   adk     10/05/16 Fixed CR#951040 race condition in the recv path when
*                        source and destination points to the same buffer.
* 1.4	Nava	1/10/19	 Added PSU_PMU processor check to skip the Flushing
*			 cache memory and Invalidating cache memory API's for
*			 PMU Microblaze platform.
*		Rama	02/26/19 Fixed IAR issue by changing
*						 "XCsuDma_WaitForDoneTimeout" to function
*       arc     03/26/19 Fixed MISRA-C violations.
* 1.5   aru     07/05/19 Fixed coverity warning.
* 1.6   aru     08/29/19 Added assert check in XCsuDma_WaitForDoneTimeout().
* 1.6   rm      11/05/19 Modified usleep waitloop and timeout value in
*				XCsuDma_WaitForDoneTimeout().
* 1.7	hk	08/03/20 Reorganize transfer function to accommodate all
*			 processors and cache functionality.
* 1.7	sk	08/26/20 Fix MISRA-C violations.
* 1.7	sk	08/26/20 Remove busy check in SetConfig.
* 1.11  bm      02/21/22 Add byte-aligned support for VERSAL_NET devices
* 1.11	sk	03/03/22 Replace driver version in addtogroup with Overview.
* 1.11	sk	03/03/22 Update Overview section based on review comments.
* 1.11	sk	03/03/22 Update XCsuDma_GetSize return type description.
* 1.14	ab	01/16/23 Added Xil_WaitForEvent() to XcsuDma_WaitForDoneTimeout.
* 1.14	ab	01/18/23 Added byte-aligned transfer API for VERSAL_NET devices.
* 1.14	bm	05/01/23 Fixed Assert condition in XCsuDma_Transfer for VERSAL_NET.
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xcsudma.h"
#include <stdbool.h>

/************************** Constant Definitions *****************************/
#define XCSUDMA_WORD_SIZE	(4U)	/**< Transfer size conversion to
					 * bytes for Versal Net */
/************************** Function Prototypes ******************************/


/************************** Function Definitions *****************************/

/*****************************************************************************/
/**
*
* This function initializes CSU_DMA core. This function must be called
* prior using a CSU_DMA core. Initialization of an CSU_DMA includes setting
* up the instance data and ensuring the hardware is in a quiescent state.
*
* @param	InstancePtr Pointer to the XCsuDma instance.
* @param	CfgPtr Reference to a structure containing information
*		about a specific XCsuDma instance.
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
* @note		None.
*
******************************************************************************/
s32 XCsuDma_CfgInitialize(XCsuDma *InstancePtr, XCsuDma_Config *CfgPtr,
			  UINTPTR EffectiveAddr)
{

	/* Verify arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(CfgPtr != NULL);
	Xil_AssertNonvoid(EffectiveAddr != ((u32)0x0));

	/* Setup the instance */
	(void)memcpy((void *) & (InstancePtr->Config), (const void *)CfgPtr,
		     sizeof(XCsuDma_Config));
	InstancePtr->Config.BaseAddress = EffectiveAddr;

	/* Verify the DMA type  */
	if (InstancePtr->Config.DmaType == (u8)XCSUDMA_DMATYPEIS_CSUDMA) {
		/* Reset CSUDMA */
		XCsuDma_Reset();
	}

	InstancePtr->IsReady = (u32)(XIL_COMPONENT_IS_READY);

	/* Return status after Initializing DMA configuration */
	return (s32)(XST_SUCCESS);

}

/*****************************************************************************/
/**
*
* This function sets the starting address and size of the data to be
* transferred from/to the memory through the AXI interface.
*
* @param	InstancePtr Pointer to XCsuDma instance to be worked on.
* @param	Channel Type of channel
*		Source channel      - XCSUDMA_SRC_CHANNEL
*		Destination Channel - XCSUDMA_DST_CHANNEL
* @param	Addr 64 bit variable which holds the starting address of
* 		data which needs to write into the memory(DST) (or read	from
* 		the memory(SRC)).
* @param	Size 32 bit variable which represents the number of 4 byte
* 		words needs to be transferred from starting address.
* @param	EnDataLast Triggers an end of the message. Enables or
* 		disables data_inp_last signal to stream interface when current
* 		command is completed. It is applicable only to source channel;
* 		ignored for destination channel.
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
void XCsuDma_Transfer(XCsuDma *InstancePtr, XCsuDma_Channel Channel,
		      u64 Addr, u32 Size, u8 EnDataLast)
{
	u32 DataSize = 0U;
	/* Verify arguments */
	Xil_AssertVoid(InstancePtr != NULL);
#if !defined(VERSAL_NET) && !defined(VERSAL_AIEPG2)
	Xil_AssertVoid(((Addr) & (u64)(XCSUDMA_ADDR_LSB_MASK)) == (u64)0x00);
#endif
	Xil_AssertVoid((Channel == (XCSUDMA_SRC_CHANNEL)) ||
		       (Channel == (XCSUDMA_DST_CHANNEL)));
	Xil_AssertVoid(Size <= (u32)(XCSUDMA_SIZE_MAX));
	Xil_AssertVoid(InstancePtr->IsReady == (u32)(XIL_COMPONENT_IS_READY));

#if defined(VERSAL_NET) || defined(VERSAL_AIEPG2)
	DataSize = Size * XCSUDMA_WORD_SIZE;
#else
	DataSize = Size;
#endif

	/* Cache flush/invalidate */
#if defined(ARMR52)
	if (((Addr >> XCSUDMA_MSB_ADDR_SHIFT) == 0U) && (Channel == (XCSUDMA_DST_CHANNEL))) {
		Xil_DCacheInvalidateRange((INTPTR)Addr, DataSize << XCSUDMA_SIZE_SHIFT);
	}
#elif defined(ARMR5)
	/* No action if 64 bit address is used when this code is running on R5.
	 * Flush if 32 bit addressing is used.
	 */
	if ((Addr >> XCSUDMA_MSB_ADDR_SHIFT) == 0U) {
		Xil_DCacheFlushRange((INTPTR)Addr, DataSize << XCSUDMA_SIZE_SHIFT);
	}
#endif
	/* No action required for PSU_PMU.
	 * Perform cache operations on ARM64 (either 32 bit and 64 bit address)
	 */
#if defined(__aarch64__)
	if (Channel == (XCSUDMA_SRC_CHANNEL)) {
		Xil_DCacheFlushRange((INTPTR)Addr,
				     (INTPTR)(DataSize << XCSUDMA_SIZE_SHIFT));
	} else {
		Xil_DCacheInvalidateRange((INTPTR)Addr,
					  (INTPTR)(DataSize << XCSUDMA_SIZE_SHIFT));
	}
#endif
	/* Set the starting address of the data to be tansferred from/to memory */
	XCsuDma_WriteReg(InstancePtr->Config.BaseAddress,
			 ((u32)(XCSUDMA_ADDR_OFFSET) +
			  ((u32)Channel * (u32)(XCSUDMA_OFFSET_DIFF))),
			 ((u32)(Addr) & (u32)(XCSUDMA_ADDR_MASK)));

	XCsuDma_WriteReg(InstancePtr->Config.BaseAddress,
			 (u32)(XCSUDMA_ADDR_MSB_OFFSET +
			       ((u32)Channel * XCSUDMA_OFFSET_DIFF)),
			 ((u32)((Addr & ULONG64_HI_MASK) >> XCSUDMA_MSB_ADDR_SHIFT) &
			  (u32)(XCSUDMA_MSB_ADDR_MASK)));

	/* Check and inform DMA if this is the last word(end of data) */
	if (EnDataLast == (u8)1U) {
		XCsuDma_WriteReg(InstancePtr->Config.BaseAddress,
				 ((u32)(XCSUDMA_SIZE_OFFSET) +
				  ((u32)Channel * (u32)(XCSUDMA_OFFSET_DIFF))),
				 ((DataSize << (u32)(XCSUDMA_SIZE_SHIFT)) |
				  (u32)(XCSUDMA_LAST_WORD_MASK)));
	} else {
		XCsuDma_WriteReg(InstancePtr->Config.BaseAddress,
				 ((u32)(XCSUDMA_SIZE_OFFSET) +
				  ((u32)Channel * (u32)(XCSUDMA_OFFSET_DIFF))),
				 (DataSize << (u32)(XCSUDMA_SIZE_SHIFT)));
	}
}

/*****************************************************************************/
/**
*
* This function sets the starting address and size of the data to be
* transferred from/to the memory through the AXI interface.
* This function is useful for pmu processor to execute
* a 64-bit DMA transfer.
*
* @param	InstancePtr Pointer to XCsuDma instance to be worked on.
* @param	Channel Type of channel
*		Source channel      - XCSUDMA_SRC_CHANNEL
*		Destination Channel - XCSUDMA_DST_CHANNEL
* @param	AddrLow Bit variable which holds the starting lower address of
* 		data which needs to write into the memory(DST) (or read	from
* 		the memory(SRC)).
* @param    	AddrHigh 32 bit variable which holds the higher address of data
* 		which needs to write into the memory(DST) (or read from
* 		the memoroy(SRC)).
* @param	Size 32 bit variable which represents the number of 4 byte
* 		words to be transferred from starting address.
* @param	EnDataLast Trigger an end of message. Enables or
* 		disable data_inp_last signal to stream interface when current
* 		command is completed. Only applicable for the source channel;
* 		ignored for destination channel.
* 		-	1 - Asserts data_inp_last signal.
* 		-	0 - data_inp_last will not be asserted.
*
* @return	None.
*
* @note		Data_inp_last signal is asserted simultaneously with the
* 		data_inp_valid signal associated with the final 32-bit word
*		transfer.
*		This API does not invalidate the DMA buffer.
*		It is recommended to call this API only through PMU processor.
*
******************************************************************************/
void XCsuDma_64BitTransfer(XCsuDma *InstancePtr, XCsuDma_Channel Channel,
			   u32 AddrLow, u32 AddrHigh, u32 Size, u8 EnDataLast)
{
	u32 DataSize = 0U;
	/* Verify arguments */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid((Channel == (XCSUDMA_SRC_CHANNEL)) ||
		       (Channel == (XCSUDMA_DST_CHANNEL)));
	Xil_AssertVoid(Size <= (u32)(XCSUDMA_SIZE_MAX));
	Xil_AssertVoid(InstancePtr->IsReady == (u32)(XIL_COMPONENT_IS_READY));

#if defined(VERSAL_NET) || defined(VERSAL_AIEPG2)
	DataSize = Size * XCSUDMA_WORD_SIZE;
#else
	DataSize = Size;
#endif
	/* Set the starting address of the data to be transferred from/to the memory */
	XCsuDma_WriteReg(InstancePtr->Config.BaseAddress,
			 ((u32)(XCSUDMA_ADDR_OFFSET) +
			  ((u32)Channel * (u32)(XCSUDMA_OFFSET_DIFF))),
			 (AddrLow & XCSUDMA_ADDR_MASK));

	XCsuDma_WriteReg(InstancePtr->Config.BaseAddress,
			 ((u32)(XCSUDMA_ADDR_MSB_OFFSET) +
			  ((u32)Channel * (u32)(XCSUDMA_OFFSET_DIFF))),
			 (AddrHigh & XCSUDMA_MSB_ADDR_MASK));

	/* Check and inform DMA if this is the last word(end of data) */
	if (EnDataLast == (u8)1U) {
		XCsuDma_WriteReg(InstancePtr->Config.BaseAddress,
				 ((u32)(XCSUDMA_SIZE_OFFSET) +
				  ((u32)Channel * (u32)(XCSUDMA_OFFSET_DIFF))),
				 ((DataSize << (u32)(XCSUDMA_SIZE_SHIFT)) |
				  (u32)(XCSUDMA_LAST_WORD_MASK)));
	} else {
		XCsuDma_WriteReg(InstancePtr->Config.BaseAddress,
				 ((u32)(XCSUDMA_SIZE_OFFSET) +
				  ((u32)Channel * (u32)(XCSUDMA_OFFSET_DIFF))),
				 (DataSize << (u32)(XCSUDMA_SIZE_SHIFT)));
	}
}

/*****************************************************************************/
/*****************************************************************************/
/**
*
* This function returns the current address location of the memory, from where
* it has to read the data(SRC) or the location where it has to write the data
* (DST) based on the channel selection.
*
* @param	InstancePtr Pointer to XCsuDma instance to be worked on.
* @param	Channel Type of channel
*		Source channel      - XCSUDMA_SRC_CHANNEL
*		Destination Channel - XCSUDMA_DST_CHANNEL
*
* @return	Address is a 64 bit variable which holds the current address.
*		- From this location data has to be read(SRC)
*		- At this location data has to be written(DST)
*
* @note		None.
*
******************************************************************************/
u64 XCsuDma_GetAddr(XCsuDma *InstancePtr, XCsuDma_Channel Channel)
{
	u64 FullAddr;

	/* Verify arguments */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid((Channel == (XCSUDMA_SRC_CHANNEL)) ||
			  (Channel == (XCSUDMA_DST_CHANNEL)));

	FullAddr = (u64)XCsuDma_ReadReg(InstancePtr->Config.BaseAddress,
					((u32)(XCSUDMA_ADDR_OFFSET) +
					 ((u32)Channel * (u32)(XCSUDMA_OFFSET_DIFF))));

	FullAddr |= (u64)((u64)XCsuDma_ReadReg(InstancePtr->Config.BaseAddress,
					       ((u32)(XCSUDMA_ADDR_MSB_OFFSET) +
						((u32)Channel * (u32)(XCSUDMA_OFFSET_DIFF)))) <<
			  (u64)(XCSUDMA_MSB_ADDR_SHIFT));

	/* Return current address where data has to be read/written  */
	return FullAddr;
}

/*****************************************************************************/
/**
*
* This function returns the size of the data yet to be transferred from memory
* to CSU_DMA or CSU_DMA to memory based on the channel selection.
*
* @param	InstancePtr Pointer to XCsuDma instance to be worked on.
* @param	Channel Type of channel
*		Source channel      - XCSUDMA_SRC_CHANNEL
*		Destination Channel - XCSUDMA_DST_CHANNEL
*
* @return	Returns number of bytes of data yet to be transferred.
*
* @note		None.
*
******************************************************************************/
u32 XCsuDma_GetSize(XCsuDma *InstancePtr, XCsuDma_Channel Channel)
{
	u32 Size;

	/* Verify arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid((Channel == (XCSUDMA_SRC_CHANNEL)) ||
			  (Channel == (XCSUDMA_DST_CHANNEL)));

	Size = XCsuDma_ReadReg(InstancePtr->Config.BaseAddress,
			       ((u32)(XCSUDMA_SIZE_OFFSET) +
				((u32)Channel * (u32)(XCSUDMA_OFFSET_DIFF)))) >>
	       (u32)(XCSUDMA_SIZE_SHIFT);

	/* Return size in bytes to be transferred */
	return Size;
}

/*****************************************************************************/
/**
*
* This function pauses the Channel data transfer to/from memory or to/from stream
* based on pause type.
*
* @param	InstancePtr Pointer to XCsuDma instance to be worked on.
* @param	Channel Type of channel
*		Source channel      - XCSUDMA_SRC_CHANNEL
*		Destination Channel - XCSUDMA_DST_CHANNEL
* @param	Type Type of pause to be enabled.
*		- XCSUDMA_PAUSE_MEMORY(0) - Pause memory
*			- SRC stops issuing new read commands to memory.
*			- DST stops issuing new write commands to memory.
*		- XCSUDMA_PAUSE_STREAM(1) - Pause stream
*			- SRC stops the transfer of data from FIFO to Stream.
*			- DST stops the transfer of data from stream to FIFO.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void XCsuDma_Pause(XCsuDma *InstancePtr, XCsuDma_Channel Channel,
		   XCsuDma_PauseType Type)
{
	u32 CsuDmaChannel = (u32)Channel;

	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid((Type == (XCSUDMA_PAUSE_MEMORY)) ||
		       (Type == (XCSUDMA_PAUSE_STREAM)));
	Xil_AssertVoid((Channel == (XCSUDMA_SRC_CHANNEL)) ||
		       (Channel == (XCSUDMA_DST_CHANNEL)));
	Xil_AssertVoid(InstancePtr->IsReady == (u32)(XIL_COMPONENT_IS_READY));

	/* Pause Memory Read/Write operations */
	if (Type == (XCSUDMA_PAUSE_MEMORY)) {
		XCsuDma_WriteReg(InstancePtr->Config.BaseAddress,
				 ((XCSUDMA_CTRL_OFFSET) +
				  (u32)(CsuDmaChannel * XCSUDMA_OFFSET_DIFF)),
				 (XCsuDma_ReadReg(InstancePtr->Config.BaseAddress,
						  ((XCSUDMA_CTRL_OFFSET) +
						   (u32)(CsuDmaChannel * XCSUDMA_OFFSET_DIFF))) |
				  (u32)(XCSUDMA_CTRL_PAUSE_MEM_MASK)));
	}
	/* Pause Stream operation */
	if (Type == (XCSUDMA_PAUSE_STREAM)) {
		XCsuDma_WriteReg(InstancePtr->Config.BaseAddress,
				 (XCSUDMA_CTRL_OFFSET +
				  (u32)(CsuDmaChannel * XCSUDMA_OFFSET_DIFF)),
				 (XCsuDma_ReadReg(InstancePtr->Config.BaseAddress,
						  (XCSUDMA_CTRL_OFFSET +
						   (u32)(CsuDmaChannel * XCSUDMA_OFFSET_DIFF))) |
				  (u32)(XCSUDMA_CTRL_PAUSE_STRM_MASK)));
	}
}

/*****************************************************************************/
/**
*
* This function checks whether Channel memory/stream is paused
* based on the given pause type.
*
* @param	InstancePtr Pointer to XCsuDma instance to be worked on.
* @param	Channel Type of channel
*		Source channel      - XCSUDMA_SRC_CHANNEL
*		Destination Channel - XCSUDMA_DST_CHANNEL
* @param	Type Type of the pause which needs to be checked.
*		- XCSUDMA_PAUSE_MEMORY(0) - Pause memory
*			- SRC stops issuing new read commands to memory.
*			- DST stops issuing new write commands to memory.
*		- XCSUDMA_PAUSE_STREAM(1) - Pause stream
*			- SRC stops the transfer of data from FIFO to Stream.
*			- DST stops the transfer of data from stream to FIFO.
*
* @return	Returns the pause status.
*		- TRUE if it is in paused state.
*		- FALSE if it is not in pause state.
*
* @note		None.
*
******************************************************************************/
s32 XCsuDma_IsPaused(XCsuDma *InstancePtr, XCsuDma_Channel Channel,
		     XCsuDma_PauseType Type)
{

	u32 Data;
	bool PauseState;

	/* Verify arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid((Channel == (XCSUDMA_SRC_CHANNEL)) ||
			  (Channel == (XCSUDMA_DST_CHANNEL)));
	Xil_AssertNonvoid((Type == (XCSUDMA_PAUSE_MEMORY)) ||
			  (Type == (XCSUDMA_PAUSE_STREAM)));

	Data = XCsuDma_ReadReg(InstancePtr->Config.BaseAddress,
			       ((u32)(XCSUDMA_CTRL_OFFSET) +
				((u32)Channel * (u32)(XCSUDMA_OFFSET_DIFF))));

	/* To know Pause condition of Memory Read/Write/Stream operations */
	if (Type == (XCSUDMA_PAUSE_MEMORY)) {
		if ((Data & (u32)(XCSUDMA_CTRL_PAUSE_MEM_MASK)) ==
		    (u32)0x00) {
			PauseState = FALSE;
		} else {
			PauseState = TRUE;
		}
	} else {
		if ((Data & (u32)(XCSUDMA_CTRL_PAUSE_STRM_MASK)) ==
		    (u32)0x00) {
			PauseState = FALSE;
		} else {
			PauseState = TRUE;
		}
	}

	/* Return status on whether DMA is paused or not */
	return (s32)PauseState;

}

/*****************************************************************************/
/**
*
* This function resumes the channel if it is in paused state and continues
* where it has left. Based on the type of pause, there is no effect if it is not in paused state.
*
*
* @param	InstancePtr Pointer to XCsuDma instance to be worked on.
* @param	Channel Type of channel
*		Source channel      - XCSUDMA_SRC_CHANNEL
*		Destination Channel - XCSUDMA_DST_CHANNEL
* @param	Type Type of pause to be resumed if it is paused.
*		- XCSUDMA_PAUSE_MEMORY(0) - Pause memory
*			- SRC stops issuing new read commands to memory.
*			- DST stops issuing new write commands to memory.
*		- XCSUDMA_PAUSE_STREAM(1) - Pause stream
*			- SRC stops the transfer of data from FIFO to Stream.
*			- DST stops the transfer of data from stream to FIFO.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void XCsuDma_Resume(XCsuDma *InstancePtr, XCsuDma_Channel Channel,
		    XCsuDma_PauseType Type)
{
	u32 Data;
	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid((Type == (XCSUDMA_PAUSE_MEMORY)) ||
		       (Type == (XCSUDMA_PAUSE_STREAM)));
	Xil_AssertVoid((Channel == (XCSUDMA_SRC_CHANNEL)) ||
		       (Channel == (XCSUDMA_DST_CHANNEL)));
	Xil_AssertVoid(InstancePtr->IsReady == (u32)(XIL_COMPONENT_IS_READY));

	Data = XCsuDma_ReadReg(InstancePtr->Config.BaseAddress,
			       ((u32)(XCSUDMA_CTRL_OFFSET) +
				((u32)Channel * (u32)(XCSUDMA_OFFSET_DIFF))));

	/* To Resume the channel if it is in paused state */
	if (Type == (XCSUDMA_PAUSE_MEMORY)) {
		XCsuDma_WriteReg(InstancePtr->Config.BaseAddress,
				 ((u32)(XCSUDMA_CTRL_OFFSET) +
				  ((u32)Channel * (u32)(XCSUDMA_OFFSET_DIFF))),
				 (Data &
				  (~(XCSUDMA_CTRL_PAUSE_MEM_MASK))));
	} else {
		XCsuDma_WriteReg(InstancePtr->Config.BaseAddress,
				 ((u32)(XCSUDMA_CTRL_OFFSET) +
				  (((u32)Channel) * (u32)(XCSUDMA_OFFSET_DIFF))),
				 ( Data &
				   (~(XCSUDMA_CTRL_PAUSE_STRM_MASK))));
	}
}

/*****************************************************************************/
/**
*
* This function returns the sum of all the data read from AXI memory. It is
* valid only when using the CSU_DMA source channel.
*
* @param	InstancePtr Pointer to XCsuDma instance to be worked on.
*
* @return	The sum of all the data read from memory.
*
* @note		Before the transfer starts, clear this register to get the
*		correct sum. Otherwise, it adds to previous value which results
*		in an incorrect output.
*		Valid only for source channel.
*
******************************************************************************/
u32 XCsuDma_GetCheckSum(XCsuDma *InstancePtr)
{
	u32 ChkSum;

	/* Verify arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady ==
			  (u32)(XIL_COMPONENT_IS_READY));

	ChkSum = XCsuDma_ReadReg(InstancePtr->Config.BaseAddress,
				 (u32)(XCSUDMA_CRC_OFFSET));

	/* Return checksum */
	return ChkSum;

}
/*****************************************************************************/
/**
*
* This function clears the check sum of the data read from AXI memory. It is
* valid only for CSU_DMA source channel.
*
* @param	InstancePtr Pointer to XCsuDma instance to be worked on.
*
* @return	The sum of all the data read from memory.
*
* @note		Before the transfer starts, clear this register to get
*		correct sum. Otherwise, it adds to previous value which results
*		in an incorrect output.
*
******************************************************************************/
void XCsuDma_ClearCheckSum(XCsuDma *InstancePtr)
{

	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);

	XCsuDma_WriteReg(InstancePtr->Config.BaseAddress,
			 (u32)(XCSUDMA_CRC_OFFSET), (u32)(XCSUDMA_CRC_RESET_MASK));
}

/*****************************************************************************/
/**
* This function polls for completion of data transfer periodically until the
* DMA done bit set or until the timeout occurs.
*
* @param	InstancePtr Pointer to XCsuDma instance to be worked on.
* @param	Channel Type of channel
*		Source channel      - XCSUDMA_SRC_CHANNEL
*		Destination Channel - XCSUDMA_DST_CHANNEL
*
* @return	XST_SUCCESS - In case of Success
*		    XST_FAILURE - In case of Timeout.
*
* @note		None.
*
******************************************************************************/
u32 XCsuDma_WaitForDoneTimeout(XCsuDma *InstancePtr, XCsuDma_Channel Channel)
{
	UINTPTR Addr;
	u32 TimeoutFlag = (u32)XST_FAILURE;

	/* Verify arguments */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid((Channel == (XCSUDMA_SRC_CHANNEL)) ||
			  (Channel == (XCSUDMA_DST_CHANNEL)));

	Addr = InstancePtr->Config.BaseAddress +
	       (u32)XCSUDMA_I_STS_OFFSET +
	       ((u32)Channel * (u32)XCSUDMA_OFFSET_DIFF);

	/* Verify completion of data transfer or timeout */
	TimeoutFlag = Xil_WaitForEvent(Addr, XCSUDMA_IXR_DONE_MASK,
				       XCSUDMA_IXR_DONE_MASK, XCSUDMA_DONE_TIMEOUT_VAL);

	return TimeoutFlag;
}
/*****************************************************************************/
/**
* This function configures all the values of CSU_DMA Channels with the values
* of updated XCsuDma_Configure structure.
*
* @param	InstancePtr Pointer to XCsuDma instance to be worked on.
* @param	Channel Type of channel
*		Source channel      - XCSUDMA_SRC_CHANNEL
*		Destination Channel - XCSUDMA_DST_CHANNEL
* @param	ConfigurValues Pointer to the structure XCsuDma_Configure
*		whose values are used to configure CSU_DMA core.
*		- SssFifoThesh:   When the DST FIFO level >= this value,
*		  the SSS interface signal, "data_out_fifo_level_hit" is
*		  asserted. This mechanism can be used by the SSS to flow
*		  control data that is being looped back from the SRC DMA.
*			- Range is (0x10 to 0x7A) threshold is 17 to 123
*			entries.
*			- It is valid only for DST CSU_DMA IP.
*		- ApbErr:          When accessed to invalid APB the resulting
*		  pslerr will be
*			- 0 - 1'b0
*			- 1 - 1'b1
*		- EndianType:      Type of endianness
*			- 0 doesn't change order
*			- 1 will flip the order.
*		- AxiBurstType:   Type of the burst
*			- 0 will issue INCR type burst
*			- 1 will issue FIXED type burst
*		- TimeoutValue:    Time out value for timers
*			- 0x000 to 0xFFE are valid inputs
*			- 0xFFF clears both timers
*		- FifoThresh:  Programmed watermark value
*			- Range is 0x00 to 0x80 (0 to 128 entries).
*		- Acache:         Sets the AXI CACHE bits on the AXI Write/Read
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
*		- RouteBit:        To select route
*			- 0 : Command will be routed normally
*			- 1 : Command will be routed to APU's cache controller
*		- TimeoutEn:       To enable or disable time out counters
*			- 0 : The 2 Timeout counters are disabled
*			- 1 : The 2 Timeout counters are enabled
*		- TimeoutPre:      Set the prescaler value for the timeout in
*		clk (~2.5ns) cycles
*			- Range is 0x000(Prescaler enables timer every cycles)
*			  to 0xFFF(Prescaler enables timer every 4096 cycles)
*		- MaxOutCmds:      Controls the maximumum number of outstanding
*		AXI read commands issued.
*			- Range is 0x0(Up to 1 Outstanding Read command
*			  allowed) to 0x8 (Up to 9 Outstanding Read
*			  command allowed)
*
* @return	None.
*
* @note		To use timers timeout value Timeout enable field should be
*		enabled.
*		Users should check for the status of existing transfers before
*		making configuration changes.
*
******************************************************************************/
void XCsuDma_SetConfig(XCsuDma *InstancePtr, XCsuDma_Channel Channel,
		       XCsuDma_Configure *ConfigurValues)
{
	u32 Data;

	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == (u32)(XIL_COMPONENT_IS_READY));
	Xil_AssertVoid(ConfigurValues != NULL);
	Xil_AssertVoid((Channel == (XCSUDMA_SRC_CHANNEL)) ||
		       (Channel == (XCSUDMA_DST_CHANNEL)));

	Data = ((((u32)(ConfigurValues->EndianType) <<
		  (u32)(XCSUDMA_CTRL_ENDIAN_SHIFT)) &
		 (u32)(XCSUDMA_CTRL_ENDIAN_MASK)) |
		(((u32)(ConfigurValues->ApbErr) <<
		  (u32)(XCSUDMA_CTRL_APB_ERR_SHIFT)) &
		 (u32)(XCSUDMA_CTRL_APB_ERR_MASK)) |
		(((u32)(ConfigurValues->AxiBurstType) <<
		  (u32)(XCSUDMA_CTRL_BURST_SHIFT)) &
		 (u32)(XCSUDMA_CTRL_BURST_MASK)) |
		((ConfigurValues->TimeoutValue <<
		  (u32)(XCSUDMA_CTRL_TIMEOUT_SHIFT)) &
		 (u32)(XCSUDMA_CTRL_TIMEOUT_MASK)) |
		(((u32)(ConfigurValues->FifoThresh) <<
		  (u32)(XCSUDMA_CTRL_FIFO_THRESH_SHIFT)) &
		 (u32)(XCSUDMA_CTRL_FIFO_THRESH_MASK)));

	/* Set Config for Destination channel */
	if (Channel == XCSUDMA_DST_CHANNEL) {
		Data = Data | (u32)(((u32)(ConfigurValues->SssFifoThesh) <<
				     (u32)(XCSUDMA_CTRL_SSS_FIFOTHRESH_SHIFT)) &
				    (u32)(XCSUDMA_CTRL_SSS_FIFOTHRESH_MASK));
	}

	XCsuDma_WriteReg(InstancePtr->Config.BaseAddress,
			 ((u32)(XCSUDMA_CTRL_OFFSET) +
			  ((u32)Channel * (u32)(XCSUDMA_OFFSET_DIFF))), Data);

	Data = (XCsuDma_ReadReg(InstancePtr->Config.BaseAddress,
				((u32)(XCSUDMA_CTRL2_OFFSET) +
				 ((u32)Channel * (u32)(XCSUDMA_OFFSET_DIFF)))) &
		(u32)(XCSUDMA_CTRL2_RESERVED_MASK));
	Data |= ((((u32)(ConfigurValues->Acache) <<
		   (u32)(XCSUDMA_CTRL2_ACACHE_SHIFT)) &
		  (u32)(XCSUDMA_CTRL2_ACACHE_MASK)) |
		 (((u32)(ConfigurValues->RouteBit) <<
		   (u32)(XCSUDMA_CTRL2_ROUTE_SHIFT)) &
		  (u32)(XCSUDMA_CTRL2_ROUTE_MASK)) |
		 (((u32)(ConfigurValues->TimeoutEn) <<
		   (u32)(XCSUDMA_CTRL2_TIMEOUT_EN_SHIFT)) &
		  (u32)(XCSUDMA_CTRL2_TIMEOUT_EN_MASK)) |
		 (((u32)(ConfigurValues->TimeoutPre) <<
		   (u32)(XCSUDMA_CTRL2_TIMEOUT_PRE_SHIFT)) &
		  (u32)(XCSUDMA_CTRL2_TIMEOUT_PRE_MASK)) |
		 ((ConfigurValues->MaxOutCmds) &
		  (u32)(XCSUDMA_CTRL2_MAXCMDS_MASK)));

	XCsuDma_WriteReg(InstancePtr->Config.BaseAddress,
			 ((u32)(XCSUDMA_CTRL2_OFFSET) +
			  ((u32)Channel * (u32)(XCSUDMA_OFFSET_DIFF))), Data);
}

/*****************************************************************************/
/**
*
* This function updates XCsuDma_Configure structure members with the configured
* values of CSU_DMA's Channel.
*
* @param	InstancePtr Pointer to XCsuDma instance to be worked on.
* @param	Channel Type of channel
*		Source channel      - XCSUDMA_SRC_CHANNEL
*		Destination Channel - XCSUDMA_DST_CHANNEL
* @param	ConfigurValues Pointer to the structure XCsuDma_Configure
*		whose members are updated with configurations of CSU_DMA core.
*		- SssFifoThesh:   When the DST FIFO level >= this value,
*		  the SSS interface signal, "data_out_fifo_level_hit" will be
*		  asserted. This mechanism can be used by the SSS to flow
*		  control data that is being looped back from the SRC DMA.
*			- Range is (0x10 to 0x7A) threshold is 17 to 123
*			entries.
*			- It is valid only for DST CSU_DMA IP.
*		- ApbErr:          When accessed to invalid APB the resulting
*		  pslerr will be
*			- 0 - 1'b0
*			- 1 - 1'b1
*		- EndianType:      Type of endianness
*			- 0 doesn't change order
*			- 1 will flip the order.
*		- AxiBurstType:  Type of the burst
*			- 0 will issue INCR type burst
*			- 1 will issue FIXED type burst
*		- TimeoutValue:    Time out value for timers
*			- 0x000 to 0xFFE are valid inputs
*			- 0xFFF clears both timers
*		- FifoThresh:  Programmed watermark value
*			- Range is 0x00 to 0x80 (0 to 128 entries).
*		- Acache:         Sets the AXI CACHE bits on the AXI Write/Read
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
*		- RouteBit:        To select route
*			- 0 : Command will be routed based normally
*			- 1 : Command will be routed to APU's cache controller
*		- TimeoutEn:       To enable or disable time out counters
*			- 0 : The 2 Timeout counters are disabled
*			- 1 : The 2 Timeout counters are enabled
*		- TimeoutPre:      Set the prescaler value for the timeout in
*		clk (~2.5ns) cycles
*			- Range is 0x000(Prescaler enables timer every cycles)
*			 to 0xFFF(Prescaler enables timer every 4096 cycles)
*		- MaxOutCmds:      Controls the maximumum number of outstanding
*		AXI read commands issued.
*			- Range is 0x0(Up to 1 Outstanding Read command
*			allowed) to 0x8 (Up to 9 Outstanding Read command
*			allowed)
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void XCsuDma_GetConfig(XCsuDma *InstancePtr, XCsuDma_Channel Channel,
		       XCsuDma_Configure *ConfigurValues)
{
	u32 Data;

	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(ConfigurValues != NULL);
	Xil_AssertVoid((Channel == (XCSUDMA_SRC_CHANNEL)) ||
		       (Channel == (XCSUDMA_DST_CHANNEL)));

	Data = XCsuDma_ReadReg(InstancePtr->Config.BaseAddress,
			       ((u32)(XCSUDMA_CTRL_OFFSET) +
				((u32)Channel * (u32)(XCSUDMA_OFFSET_DIFF))));

	/* Get Config for Destination channel */
	if (Channel == (XCSUDMA_DST_CHANNEL)) {
		ConfigurValues->SssFifoThesh =
			(u8)((Data &
			      (u32)(XCSUDMA_CTRL_SSS_FIFOTHRESH_MASK)) >>
			     (u32)(XCSUDMA_CTRL_SSS_FIFOTHRESH_SHIFT));
	}

	/* Update config structure members with configured values of CSUDMA channel*/
	ConfigurValues->ApbErr =
		(u8)((Data & (u32)(XCSUDMA_CTRL_APB_ERR_MASK)) >>
		     (u32)(XCSUDMA_CTRL_APB_ERR_SHIFT));
	ConfigurValues->EndianType =
		(u8)((Data & (u32)(XCSUDMA_CTRL_ENDIAN_MASK)) >>
		     (u32)(XCSUDMA_CTRL_ENDIAN_SHIFT));
	ConfigurValues->AxiBurstType =
		(u8)((Data & (u32)(XCSUDMA_CTRL_BURST_MASK)) >>
		     (u32)(XCSUDMA_CTRL_BURST_SHIFT));
	ConfigurValues->TimeoutValue =
		((Data & (u32)(XCSUDMA_CTRL_TIMEOUT_MASK)) >>
		 (u32)(XCSUDMA_CTRL_TIMEOUT_SHIFT));
	ConfigurValues->FifoThresh =
		(u8)((Data & (u32)(XCSUDMA_CTRL_FIFO_THRESH_MASK)) >>
		     (u32)(XCSUDMA_CTRL_FIFO_THRESH_SHIFT));

	Data = XCsuDma_ReadReg(InstancePtr->Config.BaseAddress,
			       ((u32)(XCSUDMA_CTRL2_OFFSET) +
				((u32)Channel * (u32)(XCSUDMA_OFFSET_DIFF))));

	ConfigurValues->Acache =
		(u8)((Data & (u32)(XCSUDMA_CTRL2_ACACHE_MASK)) >>
		     (u32)(XCSUDMA_CTRL2_ACACHE_SHIFT));
	ConfigurValues->RouteBit =
		(u8)((Data & (u32)(XCSUDMA_CTRL2_ROUTE_MASK)) >>
		     (u32)(XCSUDMA_CTRL2_ROUTE_SHIFT));
	ConfigurValues->TimeoutEn =
		(u8)((Data & (u32)(XCSUDMA_CTRL2_TIMEOUT_EN_MASK)) >>
		     (u32)(XCSUDMA_CTRL2_TIMEOUT_EN_SHIFT));
	ConfigurValues->TimeoutPre =
		(u16)((Data & (u32)(XCSUDMA_CTRL2_TIMEOUT_PRE_MASK)) >>
		      (u32)(XCSUDMA_CTRL2_TIMEOUT_PRE_SHIFT));
	ConfigurValues->MaxOutCmds =
		(u8)((Data & (u32)(XCSUDMA_CTRL2_MAXCMDS_MASK)));

}

#if defined(VERSAL_NET) || defined(VERSAL_AIEPG2)
/*****************************************************************************/
/**
*
* This function sets the starting address and amount(size) of the data to be
* transferred from/to the memory through the AXI interface in VERSAL NET.
* ByteAlignedTransfer is defeatured in Versal Net device. However there is a
* workaround to allow user software to use this feature while handling
* endianness. Hence this function will continue to be supported with the
* assumption that the workaround is implemented by calling software.
*
* @param	InstancePtr Pointer to XCsuDma instance to be worked on.
* @param	Channel Type of channel
*		Source channel      - XCSUDMA_SRC_CHANNEL
*		Destination Channel - XCSUDMA_DST_CHANNEL
* @param	Addr 64 bit variable which holds the starting address of
* 		data which needs to write into the memory(DST) (or read	from
* 		the memory(SRC)).
* @param	Size 32 bit variable which represents the number of bytes
* 		needs to be transferred from starting address.
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
void XCsuDma_ByteAlignedTransfer(XCsuDma *InstancePtr, XCsuDma_Channel Channel,
				 u64 Addr, u32 Size, u8 EnDataLast)
{
	/* Verify arguments */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid((Channel == (XCSUDMA_SRC_CHANNEL)) ||
		       (Channel == (XCSUDMA_DST_CHANNEL)));
	Xil_AssertVoid(Size <= (u32)(XCSUDMA_SIZE_MAX));
	Xil_AssertVoid(InstancePtr->IsReady == (u32)(XIL_COMPONENT_IS_READY));

#if defined(ARMR52)
	/* No action if 64 bit address is used when this code is running on R5.
	 * Flush if 32 bit addressing is used.
	 */
	if (((Addr >> XCSUDMA_MSB_ADDR_SHIFT) == 0U) && (Channel == (XCSUDMA_DST_CHANNEL))) {
		Xil_DCacheInvalidateRange((INTPTR)Addr, Size << XCSUDMA_SIZE_SHIFT);
	}
#else
	/* No action required for PSU_PMU.
	 * Perform cache operations on ARM64 (either 32 bit and 64 bit address)
	 */
#if defined(__aarch64__)
	if (Channel == (XCSUDMA_SRC_CHANNEL)) {
		Xil_DCacheFlushRange((INTPTR)Addr,
				     (INTPTR)(Size << XCSUDMA_SIZE_SHIFT));
	} else {
		Xil_DCacheInvalidateRange((INTPTR)Addr,
					  (INTPTR)(Size << XCSUDMA_SIZE_SHIFT));
	}
#endif
#endif

	XCsuDma_WriteReg(InstancePtr->Config.BaseAddress,
			 ((u32)(XCSUDMA_ADDR_OFFSET) +
			  ((u32)Channel * (u32)(XCSUDMA_OFFSET_DIFF))),
			 ((u32)(Addr) & (u32)(XCSUDMA_ADDR_MASK)));

	XCsuDma_WriteReg(InstancePtr->Config.BaseAddress,
			 (u32)(XCSUDMA_ADDR_MSB_OFFSET +
			       ((u32)Channel * XCSUDMA_OFFSET_DIFF)),
			 ((u32)((Addr & ULONG64_HI_MASK) >> XCSUDMA_MSB_ADDR_SHIFT) &
			  (u32)(XCSUDMA_MSB_ADDR_MASK)));

	if (EnDataLast == (u8)1U) {
		XCsuDma_WriteReg(InstancePtr->Config.BaseAddress,
				 ((u32)(XCSUDMA_SIZE_OFFSET) +
				  ((u32)Channel * (u32)(XCSUDMA_OFFSET_DIFF))),
				 ((Size << (u32)(XCSUDMA_SIZE_SHIFT)) |
				  (u32)(XCSUDMA_LAST_WORD_MASK)));
	} else {
		XCsuDma_WriteReg(InstancePtr->Config.BaseAddress,
				 ((u32)(XCSUDMA_SIZE_OFFSET) +
				  ((u32)Channel * (u32)(XCSUDMA_OFFSET_DIFF))),
				 (Size << (u32)(XCSUDMA_SIZE_SHIFT)));
	}
}
#endif
/** @} */
