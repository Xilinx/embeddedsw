
/******************************************************************************
* Copyright (C) 2024 - 2025 Advanced Micro Devices, Inc. All Rights Reserved
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xi3c.c
* @addtogroup Overview
* @{
*
* Handles init functions.
*
* <pre> MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---  -------- ---------------------------------------------
* 1.00 	gm   02/09/24 First release
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xi3c.h"
#include "xi3c_hw.h"

/************************** Function Prototypes ******************************/

void XI3c_ConfigIbi(XI3c *InstancePtr, u8 DevCount);

/************************** Variable Definitions *****************************/
/*
 * 108 dynamic addresses are available for I3C
 */
u8 XI3C_DynaAddrList[] = { 0x8, 0x9, 0xa, 0xb, 0xc, 0xd, 0xe, 0xf, 0x10, 0x11,
			   0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1a, 0x1b,
			   0x1c, 0x1d, 0x1e, 0x1f, 0x20, 0x21, 0x22, 0x23, 0x24, 0x25,
			   0x26, 0x27, 0x28, 0x29, 0x2a, 0x2b, 0x2c, 0x2d, 0x2e, 0x2f,
			   0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39,
			   0x3a, 0x3b, 0x3c, 0x3d, 0x3f, 0x40, 0x41, 0x42, 0x43, 0x44,
			   0x45, 0x46, 0x47, 0x48, 0x49, 0x4a, 0x4b, 0x4c, 0x4d, 0x4e,
			   0x4f, 0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58,
			   0x59, 0x5a, 0x5b, 0x5c, 0x5d, 0x5f, 0x60, 0x61, 0x62, 0x63,
			   0x64, 0x65, 0x66, 0x67, 0x68, 0x69, 0x6a, 0x6b, 0x6c, 0x6d,
			   0x6f, 0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x77 };

/*****************************************************************************/
/**
*
* @brief
* Initializes the XI3c slaves devices by disable/enable events and reset
* dynamic addresses.
*
* @param	InstancePtr is a pointer to the XI3c instance.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/

void XI3C_BusInit(XI3c *InstancePtr)
{
	XI3c_Cmd Cmd;
	s32 Status;

	/* Assert the arguments */
	Xil_AssertVoid(InstancePtr != NULL);

	Cmd.SlaveAddr = XI3C_BROADCAST_ADDRESS;
	Cmd.NoRepeatedStart = 1;
	Cmd.Tid = 0;
	Cmd.Pec = 0;
	Cmd.CmdType = 1;                /**< SDR mode */

	/* Disable Target Events */
	Status = XI3c_SendTransferCmd(InstancePtr, &Cmd,
				      (u8)XI3C_CCC_BRDCAST_DISEC);
	if (Status != XST_SUCCESS) {
		return;
	}

	Cmd.SlaveAddr = XI3C_BROADCAST_ADDRESS;
	Cmd.NoRepeatedStart = 1;
	Cmd.Tid = 0;
	Cmd.Pec = 0;
	Cmd.CmdType = 1;                /**< SDR mode */
	/* Enable Target Events */
	Status = XI3c_SendTransferCmd(InstancePtr, &Cmd,
				      (u8)XI3C_CCC_BRDCAST_ENEC);
	if (Status != XST_SUCCESS) {
		return;
	}

	Cmd.SlaveAddr = XI3C_BROADCAST_ADDRESS;
	Cmd.NoRepeatedStart = 1;
	Cmd.Tid = 0;
	Cmd.Pec = 0;
	Cmd.CmdType = 1;                /**< SDR mode */
	/* Reset Dynamic Address assigned to all the I3C Targets */
	Status = XI3c_SendTransferCmd(InstancePtr, &Cmd,
				      (u8)XI3C_CCC_BRDCAST_RSTDAA);
	if (Status != XST_SUCCESS) {
		return;
	}
}

/*****************************************************************************/
/**
*
* @brief
* Initializes a specific XI3c instance such that the driver is ready to use.
*
* @param	InstancePtr is a pointer to the XI3c instance.
* @param	ConfigPtr is a reference to a structure containing information
*		about a specific I3C device. This function initializes an
*		InstancePtr object for a specific device specified by the
*		contents of Config.
* @param	EffectiveAddr is the device base address in the virtual memory
*		address space. The caller is responsible for keeping the address
*		mapping from EffectiveAddr to the device physical base address
*		unchanged once this function is invoked. Unexpected errors may
*		occur if the address mapping changes after this function is
*		called. If address translation is not used, use
*		ConfigPtr->BaseAddress for this parameter, passing the physical
*		address instead.
*
* @return	The return value is XST_SUCCESS if successful.
*
* @note		None.
*
******************************************************************************/

s32 XI3c_CfgInitialize(XI3c *InstancePtr, XI3c_Config *ConfigPtr,
		       u32 EffectiveAddr)
{
	/* Assert the arguments */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(ConfigPtr != NULL);

	if (InstancePtr->IsReady == XIL_COMPONENT_IS_READY) {
		return XST_DEVICE_IS_STARTED;
	}

	/*
	 * Set the values read from the device config and the base address.
	 */
#ifndef SDT
	InstancePtr->Config.DeviceId = ConfigPtr->DeviceId;
#endif
	InstancePtr->Config.BaseAddress = EffectiveAddr;
	InstancePtr->Config.InputClockHz = ConfigPtr->InputClockHz;
	InstancePtr->Config.RwFifoDepth = ConfigPtr->RwFifoDepth;
	/*
	 * WrThreshold value in terms of words from design,
	 * so convert to bytes.
	 */
	InstancePtr->Config.WrThreshold = ConfigPtr->WrThreshold * WORD_TO_BYTE;
	InstancePtr->Config.DeviceCount = ConfigPtr->DeviceCount;
	InstancePtr->Config.IbiCapable = ConfigPtr->IbiCapable;
	InstancePtr->Config.HjCapable = ConfigPtr->HjCapable;
	InstancePtr->Config.DeviceRole = ConfigPtr->DeviceRole;

	InstancePtr->CurDeviceCount = 0;
	/*
	 * Indicate the instance is now ready to use, initialized without error
	 */
	InstancePtr->IsReady = XIL_COMPONENT_IS_READY;

	/*
	 * Reset the I3C controller to get it into its initial state. It is expected
	 * that device configuration will take place after this initialization
	 * is done, but before the device is started.
	 */

	XI3c_Reset(InstancePtr);
	XI3c_ResetFifos(InstancePtr);

	if (InstancePtr->Config.IbiCapable)
		XI3c_EnableIbi(InstancePtr);

	if (InstancePtr->Config.HjCapable)
		XI3c_EnableHotjoin(InstancePtr);

	/*
	 *  Enable I3C controller
	 */
	XI3c_Enable(InstancePtr, 1);

	if (!InstancePtr->Config.DeviceRole) {	/**< Master mode */
		XI3C_BusInit(InstancePtr);

		if (InstancePtr->Config.DeviceCount) {
			XI3c_DynaAddrAssign(InstancePtr, XI3C_DynaAddrList, InstancePtr->Config.DeviceCount);

			if (InstancePtr->Config.IbiCapable)
				XI3c_ConfigIbi(InstancePtr, InstancePtr->Config.DeviceCount);
		}

		/*
		 * Enable Hot join raising edge interrupt.
		 */
		if (InstancePtr->Config.HjCapable)
			XI3c_EnableREInterrupts(InstancePtr->Config.BaseAddress,
						XI3C_INTR_HJ_MASK);
	} else {	/**< Slave mode */
		/*
		 * Eanble response fifo not empty interrupt
		 */
		XI3c_EnableREInterrupts(InstancePtr->Config.BaseAddress,
					XI3C_INTR_RESP_NOT_EMPTY_MASK);
	}

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* @brief
* Fill I3C Command fifo
*
* @param	InstancePtr is a pointer to the XI3c instance.
* @param	TransferCmd is a pointer to the XI3c_Cmd instance.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void XI3c_FillCmdFifo(XI3c *InstancePtr, XI3c_Cmd *Cmd)
{
	u32 TransferCmd = 0;
	u8 DevAddr = 0;

	DevAddr = ((Cmd->SlaveAddr & XI3C_7BITS_MASK) << 1) | (Cmd->Rw & XI3C_1BIT_MASK);

	TransferCmd = Cmd->CmdType & XI3C_4BITS_MASK;				/**< Command Type: 0 to 3 bits */
	TransferCmd |= (u32)(Cmd->NoRepeatedStart & XI3C_1BIT_MASK) << 4;	/**< Repeated start or Termination on Completion: 4th bit */
	TransferCmd |= (u32)(Cmd->Pec & XI3C_1BIT_MASK) << 5;			/**< Parity Error Check: 5th bit */
	TransferCmd |= (u32)(DevAddr) << 8;					/**< RW: 8th bit + Device address: 9 to 15 bits */
	TransferCmd |= (u32)(Cmd->ByteCount & XI3C_12BITS_MASK) << 16;		/**< No.of bytes R/W to/from R/W FIFOs */
	TransferCmd |= (u32)(Cmd->Tid & XI3C_4BITS_MASK) << 28;			/**< Transaction ID */

	XI3c_WriteReg(InstancePtr->Config.BaseAddress, XI3C_CMD_FIFO_OFFSET, TransferCmd);
}

/***************************************************************************/
/**
* @brief
* Fill I3C Write Tx FIFO.
*
* @param	BaseAddress contains the base address of the device.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void XI3c_WriteTxFifo(XI3c *InstancePtr)
{
	u32 Data = 0;
	u16 Index;

	if (InstancePtr->SendByteCount > 3) {
		Data = (u32)((*(InstancePtr->SendBufferPtr + 0) << 24) |
			     (*(InstancePtr->SendBufferPtr + 1) << 16) |
			     (*(InstancePtr->SendBufferPtr + 2) << 8)  |
			     (*(InstancePtr->SendBufferPtr + 3) << 0));

		InstancePtr->SendByteCount = InstancePtr->SendByteCount - 4;
		InstancePtr->SendBufferPtr = InstancePtr->SendBufferPtr + 4;
	} else {
		for (Index = 0; Index < InstancePtr->SendByteCount; Index++) {
			Data |= (u32)(*(InstancePtr->SendBufferPtr + Index) << (24-(8*Index)));
		}
		InstancePtr->SendByteCount = 0;
	}

	XI3c_WriteReg(InstancePtr->Config.BaseAddress, XI3C_WR_FIFO_OFFSET, Data);
}

/***************************************************************************/
/**
* @brief
* Read RX I3C FIFO.
*
* @param	BaseAddress contains the base address of the device.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void XI3c_ReadRxFifo(XI3c *InstancePtr)
{
	u32 Data = 0;
	u16 Index;

	Data = XI3c_ReadReg(InstancePtr->Config.BaseAddress, XI3C_RD_FIFO_OFFSET);

	if (InstancePtr->RecvByteCount > 3) {
		*(InstancePtr->RecvBufferPtr + 0) = (u8)((Data >> 24) & 0xFF);
		*(InstancePtr->RecvBufferPtr + 1) = (u8)((Data >> 16) & 0xFF);
		*(InstancePtr->RecvBufferPtr + 2) = (u8)((Data >> 8) & 0xFF);
		*(InstancePtr->RecvBufferPtr + 3) = (u8)((Data >> 0) & 0xFF);

		InstancePtr->RecvByteCount = InstancePtr->RecvByteCount - 4;
		InstancePtr->RecvBufferPtr = InstancePtr->RecvBufferPtr + 4;
	} else {
		for (Index = 0; Index < InstancePtr->RecvByteCount; Index++) {
			*(InstancePtr->RecvBufferPtr + Index) = (u8)((Data >> (24-(8*Index))) & 0xFF);
		}
		InstancePtr->RecvByteCount = 0;
	}
}

/*****************************************************************************/
/**
* @brief
* This function sends dynamic Address Assignment for available devices.
*
* @param	InstancePtr is a pointer to the XI3c instance.
* @param	DynaAddr is an array of dynamic addresses.
* @param	DevCount is the number of slave devices present.
*
* @return
*               - XST_SUCCESS if everything went well.
*               - XST_FAILURE if any error.
*
* @note         None.
*
******************************************************************************/
s32 XI3c_DynaAddrAssign(XI3c *InstancePtr, u8 DynaAddr[], u8 DevCount)
{
	u8 RecvBuffer[8];
	XI3c_Cmd Cmd;
	u16 Index;
	u8 Addr;
	s32 Status;

	/* Assert the arguments */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == (u32)XIL_COMPONENT_IS_READY);

	Cmd.NoRepeatedStart = 0;		/**< Enable repeated start */
	Cmd.SlaveAddr = XI3C_BROADCAST_ADDRESS;	/**< Broadcast address */
	Cmd.Tid = 0;
	Cmd.Pec = 0;
	Cmd.CmdType = 1;

	Status = XI3c_SendTransferCmd(InstancePtr, &Cmd, (u8)XI3C_CCC_BRDCAST_ENTDAA);
	if (Status != XST_SUCCESS) {
		return Status;
	}

	for(Index = 0; (Index < DevCount) && (Index < XI3C_MAXDAACOUNT); Index++) {
		Addr = (DynaAddr[Index] << 1) | (XI3c_GetOddParity(DynaAddr[Index]));

		InstancePtr->SendBufferPtr = &Addr;
		InstancePtr->SendByteCount = 1;

		XI3c_WriteTxFifo(InstancePtr);

		if (Index + 1 == DevCount)
			Cmd.NoRepeatedStart = 1;
		else
			Cmd.NoRepeatedStart = 0;

		Cmd.SlaveAddr = XI3C_BROADCAST_ADDRESS;
		Cmd.Tid = 0;
		Cmd.Pec = 0;
		Cmd.CmdType = 1;

		Status = XI3c_MasterRecvPolled(InstancePtr, &Cmd, RecvBuffer,
					       XI3C_SLAVEINFO_READ_BYTECOUNT);
		if (Status != XST_SUCCESS) {
			return Status;
		}

		/**< ID - RecvBuffer[0] to RecvBuffer[5] */
		InstancePtr->XI3c_SlaveInfoTable[InstancePtr->CurDeviceCount].Id = (((u64)RecvBuffer[0] << 40)|
										    ((u64)RecvBuffer[1] << 32)|
										    ((u64)RecvBuffer[2] << 24)|
										    ((u64)RecvBuffer[3] << 16)|
										    ((u64)RecvBuffer[4] << 8) |
										    ((u64)RecvBuffer[5]));
		/**< BCR - RecvBuffer[6] */
		InstancePtr->XI3c_SlaveInfoTable[InstancePtr->CurDeviceCount].Bcr = RecvBuffer[6];
		/**< DCR - RecvBuffer[7] */
		InstancePtr->XI3c_SlaveInfoTable[InstancePtr->CurDeviceCount].Dcr = RecvBuffer[7];
		/**< Dynamic address */
		InstancePtr->XI3c_SlaveInfoTable[InstancePtr->CurDeviceCount].DynaAddr = DynaAddr[Index];

		InstancePtr->CurDeviceCount++;
	}
	return XST_SUCCESS;
}

/*****************************************************************************/
/**
* @brief
* This configure target address and BCR register values of available devices
* to the controller RAM.
*
* @param	InstancePtr is a pointer to the XI3c instance.
* @param	DevCount is the number of slave devices present.
*
* @return	None.
*
* @note         None.
*
******************************************************************************/
void XI3c_ConfigIbi(XI3c *InstancePtr, u8 DevCount)
{
	u16 Index;

	/* Assert the arguments */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == (u32)XIL_COMPONENT_IS_READY);

	for(Index = 0; (Index < DevCount) && (Index < XI3C_MAXDAACOUNT); Index++)
		XI3c_UpdateAddrBcr(InstancePtr, Index);
}
