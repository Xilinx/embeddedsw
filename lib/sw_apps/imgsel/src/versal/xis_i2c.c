/******************************************************************************
* Copyright (c) 2022 Xilinx, Inc. All rights reserved.
* Copyright (c) 2022-2024 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
* @file xis_i2c.c
*
* This file used to Read the Board Name from IIC EEPROM. Based on the Board
* name, multiboot offset value will get updated.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- ---------------------------------------------------------
* 1.00  skd  01/13/23 Initial release
* 2.00  sd   05/17/24 Add SDT support
*
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/
#include "xis_config.h"
#include "xis_i2c.h"
#include "xis_error.h"
#include "xplmi_debug.h"


#ifdef XIS_GET_BOARD_PARAMS
/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/
#ifndef SDT
#define XIS_I2C_EEPROM_DEVICE (0U)
#else
#define XIS_I2C_EEPROM_DEVICE (XPAR_XIICPS_0_BASEADDR)
#endif

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/
u8 WriteBuffer[sizeof(AddressType) + XIS_MAX_SIZE];
XIicPs IicInstance;

/************************** Function Definitions *****************************/
static int XIs_EepromWriteData(XIicPs *IicInstance, u16 ByteCount);
static int XIs_MuxInitChannel(u16 MuxIicAddr, u8 WriteBuffer);
static int XIs_IicPsConfig(void);

/*****************************************************************************/
/**
* This API gives a XIs_Delay in microseconds
*
* @param	useconds requested
*
* @return	None
*
*
****************************************************************************/
static void XIs_Delay(unsigned int useconds)
{
	int i,j;

	for(j = 0U; j < useconds; j++) {
		for(i = 0U; i < 15U; i++) {
			asm("nop");
		}
	}
}

/*****************************************************************************/
/**
 * This function writes a buffer of data to the IIC serial EEPROM.
 *
 * @param	IicInstance Pointer to IIC instance
 * @param	ByteCount contains the number of bytes in the buffer to be
 *			written.
 *
 * @return	XST_SUCCESS if successful else XST_FAILURE.
 *
 * @note	The Byte count should not exceed the page size of the EEPROM as
 *		noted by the constant PAGE_SIZE.
 *
 ******************************************************************************/
static int XIs_EepromWriteData(XIicPs *IicInstance, u16 ByteCount)
{
	int Status = XST_FAILURE;
	u32 Timeout = XIICPS_POLL_DEFAULT_TIMEOUT_VAL;

	/*
	 * Send the Data.
	 */
	Status = XIicPs_MasterSendPolled(IicInstance, WriteBuffer,
			ByteCount, XIS_EEPROM_ADDRESS);
	if (Status != XST_SUCCESS) {
		Status = XIS_IICPS_MASTER_SEND_POLLED_ERROR;
		goto END;
	}

	/*
	 * Wait until bus is idle to start another transfer.
	 */
	while (Timeout != 0U) {
		if(XIicPs_BusIsBusy(IicInstance) == FALSE ) {
			break;
		}
		XIs_Delay(XIS_DELAY);
		Timeout--;
		if(!Timeout) {
			Status = XIS_IICPS_TIMEOUT;
			goto END;
		}
	}

END:
	return Status;
}

/*****************************************************************************/
/**
 * This function reads data from the IIC serial EEPROM into a specified buffer.
 *
 * @param	BufferPtr contains the address of the data buffer to be filled.
 * @param	ByteCount contains the number of bytes in the buffer to be read.
 *
 * @return	XST_SUCCESS if successful else XST_FAILURE.
 *
 ******************************************************************************/
int XIs_EepromReadData(u8 *BufferPtr, u16 ReadAddress, u16 ByteCount,
									u32 WrBfrOffset)
{
	int Status = XST_FAILURE;
	AddressType Address = ReadAddress;
	u32 Timeout = XIICPS_POLL_DEFAULT_TIMEOUT_VAL;

	/*
	 * Position the Pointer in EEPROM.
	 */
	WriteBuffer[0U] = (u8)(Address);

	Status = XIs_EepromWriteData(&IicInstance, WrBfrOffset);
	if (Status != XST_SUCCESS) {
		Status = XIS_EEPROM_WRITE_ERROR;
		goto END;
	}
	/*
	 * Receive the Data.
	 */
	Status = XIicPs_MasterRecvPolled(&IicInstance, BufferPtr,
			ByteCount, XIS_EEPROM_ADDRESS);
	if (Status != XST_SUCCESS) {
		Status = XIS_IICPS_MASTER_RECV_POLLED_ERROR;
		goto END;
	}

	/*
	 * Wait until bus is idle to start another transfer.
	 */
	while (Timeout != 0U) {
		if(XIicPs_BusIsBusy(&IicInstance) == FALSE ) {
			break;
		}
		XIs_Delay(XIS_DELAY);
		Timeout--;
		if(!Timeout) {
			Status = XIS_IICPS_TIMEOUT;
			goto END;
		}
	}

END:
	return Status;
}

/*****************************************************************************/
/**
 * This function initializes the IIC MUX to select the required channel.
 *
 * @param	MuxAddress Contains the address of IIC MUX
 * @param 	Channel Contains the channel number.
 *
 * @return	XST_SUCCESS if pass, otherwise XST_FAILURE.
 *
 * @note		None.
 *
 ****************************************************************************/
static int XIs_MuxInitChannel(u16 MuxIicAddr, u8 Channel)
{
	int Status = XST_FAILURE;
	u8 Buffer = 0U;
	u32 Timeout = XIICPS_POLL_DEFAULT_TIMEOUT_VAL;

	/*
	 * Wait until bus is idle to start another transfer.
	 */
	while (Timeout != 0U) {
		if(XIicPs_BusIsBusy(&IicInstance) == FALSE ) {
			break;
		}
		XIs_Delay(XIS_DELAY);
		Timeout--;
		if(!Timeout) {
			Status = XIS_IICPS_TIMEOUT;
			goto END;
		}
	}

	/*
	 * Send the Data.
	 */
	Status = XIicPs_MasterSendPolled(&IicInstance, &Channel, 1U,
			MuxIicAddr);
	if (Status != XST_SUCCESS) {
		Status = XIS_IICPS_MASTER_SEND_POLLED_ERROR;
		goto END;
	}

	Timeout = XIICPS_POLL_DEFAULT_TIMEOUT_VAL;

	/*
	 * Wait until bus is idle to start another transfer.
	 */
	while (Timeout != 0U) {
		if(XIicPs_BusIsBusy(&IicInstance) == FALSE ) {
			break;
		}
		XIs_Delay(XIS_DELAY);
		Timeout--;
		if(!Timeout) {
			Status = XIS_IICPS_TIMEOUT;
			goto END;
		}
	}

	/*
	 * Receive the Data.
	 */
	Status = XIicPs_MasterRecvPolled(&IicInstance, &Buffer, 1U, MuxIicAddr);
	if (Status != XST_SUCCESS) {
		Status = XIS_IICPS_MASTER_RECV_POLLED_ERROR;
		goto END;
	}

	Timeout = XIICPS_POLL_DEFAULT_TIMEOUT_VAL;

	/*
	 * Wait until bus is idle to start another transfer.
	 */
	while (Timeout != 0U) {
		if(XIicPs_BusIsBusy(&IicInstance) == FALSE ) {
			break;
		}
		XIs_Delay(XIS_DELAY);
		Timeout--;
		if(!Timeout) {
			Status = XIS_IICPS_TIMEOUT;
			goto END;
		}
	}

END:
	return Status;
}

/*****************************************************************************/
/**
 * This function perform the initial configuration for the IICPS Device.
 *
 * @param	DeviceId instance.
 *
 * @return	XST_SUCCESS if pass, otherwise XST_FAILURE.
 *
 ****************************************************************************/
static int XIs_IicPsConfig(void)
{
	int Status = XST_FAILURE;
	XIicPs_Config *ConfigPtr;	/* Pointer to configuration data */

	/*
	 * Initialize the IIC driver so that it is ready to use.
	 */
	ConfigPtr = XIicPs_LookupConfig(XIS_I2C_EEPROM_DEVICE);
	if (ConfigPtr == NULL) {
		Status = XIS_IICPS_LKP_CONFIG_ERROR;
		goto END;
	}

	Status = XIicPs_CfgInitialize(&IicInstance, ConfigPtr,
			ConfigPtr->BaseAddress);
	if (Status != XST_SUCCESS) {
		Status = XIS_IICPS_CONFIG_INIT_ERROR;
		goto END;
	}
	/*
	 * Set the IIC serial clock rate.
	 */
	Status = XIicPs_SetSClk(&IicInstance, XIS_IIC_SCLK_RATE);
	if (Status != XST_SUCCESS) {
		Status = XIS_IICPS_SET_SCLK_ERROR;
		goto END;
	}

END:
	return Status;
}

/*****************************************************************************/
/**
 * This function is use to configure the iic device and Init the mux channel
 *
 * @param	None
 *
 * @return	XST_SUCCESS if successful and also update the epprom slave
 * device address in addr variable else XST_FAILURE.
 *
 * @note		None.
 *
 ******************************************************************************/
int XIs_IicPsMuxInit(void)
{
	int Status = XST_FAILURE;

	Status = XIs_IicPsConfig();
	if (Status != XST_SUCCESS) {
		Status = XIS_IICPS_CONFIG_ERROR;
		goto END;
	}

	Status = XIs_MuxInitChannel(XIS_MUX_ADDR, XIS_I2C_MUX_INDEX);
	if (Status != XST_SUCCESS) {
		XIs_Printf(XIS_DEBUG_PRINT_ALWAYS,"Ignore this error only for VEK280\r\n");
		Status = XST_SUCCESS;
	}

END:
	return Status;
}
#endif
