/******************************************************************************
* Copyright (c) 2019 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


#include "xpm_pmbus.h"
#include "xpm_common.h"
#include "sleep.h"

#ifdef XPAR_XIICPS_1_DEVICE_ID

/*************************** Variable Definitions ****************************/
#define PMBUS_BUFFER_SIZE	3	/**< I2C Buffer size */

static u8 SendBuffer[PMBUS_BUFFER_SIZE];	/**< Data Transmitting Buffer */

/*****************************************************************************/
/**
 * This function waits for the I2C Bus to be idle and times out after 1 sec
 *
 * @param Iic	I2C instance
 *
 * @return		XST_SUCCESS if successful, otherwise XST_FAILURE
 *****************************************************************************/
static XStatus XPmBus_IdleBusWait(XIicPs *Iic)
{
	XStatus Status = XST_FAILURE;
	u32 Timeout;

	Timeout = 100000;
	while (0 != XIicPs_BusIsBusy(Iic)) {
		usleep(10);
		Timeout--;

		if (0U == Timeout) {
			PmErr("ERROR: I2C bus idle wait timeout\r\n");
			goto done;
		}
	}

	Status = XST_SUCCESS;

done:
	return Status;
}

/*****************************************************************************/
/**
 * This function is used to call the Master send from IICPS driver
 *
 * @param Iic		I2C instance
 * @param SlaveAddr	Address of slave device to write to
 * @param Buffer	Data buffer for commands
 * @param ByteCount	Number of bytes in the buffer
 *
 * @return      XST_SUCCESS if successful, otherwise XST_FAILURE
 *****************************************************************************/
static XStatus XPmBus_Write(XIicPs *Iic, u16 SlaveAddr,
				     u8 *Buffer, s32 ByteCount)
{
	XStatus Status = XST_FAILURE;

	/* Continuously try to send in case of arbitration */
	do {
		if (0 != Iic->IsRepeatedStart) {
			Status = XPmBus_IdleBusWait(Iic);
			if (XST_SUCCESS != Status) {
				goto done;
			}
		}

		Status = XIicPs_MasterSendPolled(Iic, Buffer, ByteCount, SlaveAddr);

	} while (XST_IIC_ARB_LOST == Status);

	if (XST_SUCCESS != Status) {
		PmErr("I2C write failure\r\n");
	}

done:
	return Status;
}

/*****************************************************************************/
/**
 * This function is used to call the Master receive from IICPS driver
 *
 * @param Iic       I2C instance
 * @param SlaveAddr Address of slave device to read from
 * @param Buffer    Data buffer for commands
 * @param ByteCount Number of bytes in the buffer
 *
 * @return      XST_SUCCESS if successful, otherwise XST_FAILURE
 *****************************************************************************/
static XStatus XPmBus_Read(XIicPs *Iic, u16 SlaveAddr,
				     u8 *Buffer, s32 ByteCount)
{
	XStatus Status = XST_FAILURE;

	/* Continuously try to read in case of arbitration */
	do {
		Status = XPmBus_IdleBusWait(Iic);
		if (XST_SUCCESS != Status) {
			goto done;
		}

		Status = XIicPs_MasterRecvPolled(Iic, Buffer, ByteCount, SlaveAddr);

	} while (XST_IIC_ARB_LOST == Status);

	if (XST_SUCCESS != Status) {
		PmErr("I2C read failure\r\n");
	}

done:
	return Status;
}

/*****************************************************************************/
/**
 * This function is used to write a single byte to a slave device
 *
 * @param Iic       I2C instance
 * @param SlaveAddr Address of slave device to write to
 * @param Command   PmBus command to send to slave
 * @param Byte		Single byte of data to write
 *
 * @return      XST_SUCCESS if successful, otherwise XST_FAILURE
 *****************************************************************************/
XStatus XPmBus_WriteByte(XIicPs *Iic, u16 SlaveAddr, u8 Command, u8 Byte)
{
	XStatus Status = XST_FAILURE;

	SendBuffer[0] = Command;
	SendBuffer[1] = Byte;

	Status = XPmBus_Write(Iic, SlaveAddr, SendBuffer, 2);
	if (XST_SUCCESS != Status) {
		PmErr("I2C write byte failure\r\n");
	}

	return Status;
}

/*****************************************************************************/
/**
 * This function is used to write a word to a slave device
 *
 * @param Iic       I2C instance
 * @param SlaveAddr Address of slave device to write to
 * @param Command   PmBus command to send to slave
 * @param Word      Word of data to write
 *
 * @return      XST_SUCCESS if successful, otherwise XST_FAILURE
 *****************************************************************************/
XStatus XPmBus_WriteWord(XIicPs *Iic, u16 SlaveAddr, u8 Command, u16 Word)
{
	XStatus Status = XST_FAILURE;

	SendBuffer[0] = Command;
	SendBuffer[1] = (u8) (Word >> 8);
	SendBuffer[2] = (u8) (Word);

	Status = XPmBus_Write(Iic, SlaveAddr, SendBuffer, 3);
	if (XST_SUCCESS != Status) {
		PmErr("I2C write word failure\r\n");
	}

	return Status;
}

/*****************************************************************************/
/**
 * This function is used to read from a slave device
 *
 * @param Iic       I2C instance
 * @param Buffer	Buffer to store data read from device
 * @param SlaveAddr Address of slave device to read from
 * @param Command   PmBus command to send to slave
 * @param ByteCount Number of bytes to receive from slave device
 *
 * @return      XST_SUCCESS if successful, otherwise XST_FAILURE
 *****************************************************************************/
XStatus XPmBus_ReadData(XIicPs *Iic, u8 *Buffer, u16 SlaveAddr,
				     u8 Command, s32 ByteCount)
{
	XStatus Status = XST_FAILURE;

	SendBuffer[0] = Command;

	Status = XPmBus_Write(Iic, SlaveAddr, SendBuffer, 1);
	if (XST_SUCCESS != Status) {
		PmErr("Iic write command for read failure\r\n");
		goto done;
	}

	/* Clear repeated start condition to reset hold bit */
	if (0 != Iic->IsRepeatedStart) {
		Status = XIicPs_ClearOptions(Iic, XIICPS_REP_START_OPTION);
		if (XST_SUCCESS != Status) {
			goto done;
		}
	}

	Status = XPmBus_Read(Iic, SlaveAddr, Buffer, ByteCount);
	if (XST_SUCCESS != Status) {
		PmErr("I2C read data failure\r\n");
	}

done:
	return Status;
}

#endif /* XPAR_XIICPS_1_DEVICE_ID */
