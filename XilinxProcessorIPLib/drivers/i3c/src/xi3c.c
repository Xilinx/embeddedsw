
/******************************************************************************
* Copyright (C) 2024 Advanced Micro Devices, Inc. All Rights Reserved
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

/************************** Variable Prototypes ******************************/

/**
 * This table contains slave devices information for each I3C slave
 * in the system.
 */

XI3c_SlaveInfo XI3c_SlaveInfoTable[XI3C_MAXDAACOUNT]; /**< Slave info table */

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
		return Status;
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
		return Status;
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
		return Status;
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
	InstancePtr->Config.WrThreshold = XI3C_THRESHOLD_BYTECOUNT;	/**< Currently Write threshold value is static, expected to get parameter from design */

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

	/*
	 *  Enable I3C controller
	 */
	XI3c_Enable(InstancePtr, 1);

	XI3C_BusInit(InstancePtr);

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
		Addr = DynaAddr[Index] << 1;

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
		XI3c_SlaveInfoTable[Index].Id = (((u64)RecvBuffer[0] << 40)|
						 ((u64)RecvBuffer[1] << 32)|
						 ((u64)RecvBuffer[2] << 24)|
						 ((u64)RecvBuffer[3] << 16)|
						 ((u64)RecvBuffer[4] << 8) |
						 ((u64)RecvBuffer[5] << 0));
		/**< BCR - RecvBuffer[6] */
		XI3c_SlaveInfoTable[Index].Bcr = RecvBuffer[6];
		/**< DCR - RecvBuffer[7] */
		XI3c_SlaveInfoTable[Index].Dcr = RecvBuffer[7];
	}
	return XST_SUCCESS;
}
