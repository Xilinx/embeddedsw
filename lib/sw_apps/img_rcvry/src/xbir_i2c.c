/******************************************************************************
* Copyright (c) 2020 - 2021 Xilinx, Inc. All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
* @file xbir_i2c.c
*
* This file contains I2C related code.
*
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who    Date      Changes
* ----- ---- ---------- -------------------------------------------------------
* 1.00  bsv   07/02/20   First release
*
* </pre>
*
******************************************************************************/


/***************************** Include Files *********************************/
#include "xbir_i2c.h"
#include "xstatus.h"
#include "xbir_config.h"

#if defined(XPAR_XIICPS_NUM_INSTANCES)
#include "xbir_err.h"
#include "sleep.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/
#if (XPAR_XIICPS_NUM_INSTANCES == 2U)
#define XBIR_I2C_EEPROM_INDEX	(1U)
#else
#define XBIR_I2C_EEPROM_INDEX	(0U)
#endif

#define XBIR_IIC_SCLK_RATE		(100000U)
#define XBIR_MAX_DELAY			(10000000U)
#define XBIR_I2C_GPIO_EXPANDER		(0x11U)
#define XBIR_GEM1_RESET_MASK		(0x40U)

/************************** Function Prototypes ******************************/


/************************** Variable Definitions *****************************/
static XIicPs IicInstance = {0U};	/* The instance of the IIC device. */

/************************** Function Definitions *****************************/
/*****************************************************************************/
/**
 * @brief
 * This function reads data from the IIC serial EEPROM into a specified buffer.
 *
 * @return	XST_SUCCESS on successful read of EEPROM
 *		Error code on failure
 *
 ******************************************************************************/
int Xbir_I2cExpanderReset(void)
{
	int Status = XST_FAILURE;
	/* Eeprom Page size is 32 bytes and hence 2 bytes for array */
	u8 Buffer[2U] = {0xDB, 0U};
	u32 TimeOutCount = XBIR_MAX_DELAY;

	Status = XIicPs_MasterSendPolled(&IicInstance, &Buffer[0U],
		sizeof(Buffer), XBIR_I2C_GPIO_EXPANDER);
	if (Status != XST_SUCCESS) {
		Status = XBIR_ERROR_IIC_MASTER_SEND;
		goto END;
	}
	/*
	 * Wait until bus is idle to start another transfer.
	 */
	TimeOutCount = XBIR_MAX_DELAY;
	while ((XIicPs_BusIsBusy(&IicInstance)) && (TimeOutCount > 0U)) {
		TimeOutCount--;
	}
	if (TimeOutCount == 0U) {
		Status = XBIR_ERROR_I2C_READ_TIMEOUT;
		goto END;
	}
		usleep(XBIR_LATCH_TIME_FOR_PHY_RESET_IN_US);

	Buffer[1U] = XBIR_GEM1_RESET_MASK;
	Status = XIicPs_MasterSendPolled(&IicInstance, &Buffer[0U],
		sizeof(Buffer), XBIR_I2C_GPIO_EXPANDER);
	if (Status != XST_SUCCESS) {
		Status = XBIR_ERROR_IIC_MASTER_SEND;
		goto END;
	}
	TimeOutCount = XBIR_MAX_DELAY;
	while ((XIicPs_BusIsBusy(&IicInstance)) && (TimeOutCount > 0U)) {
		TimeOutCount--;
	}
	if (TimeOutCount == 0U) {
		Status = XBIR_ERROR_I2C_READ_TIMEOUT;
	}

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief
 * This function reads data from the IIC serial EEPROM into a specified buffer.
 *
 * @param	BufferPtr 	Pointer to the data buffer to be filled
 * @param	ByteCount 	Number of bytes in the buffer to be read
 * @param	EepromAddr 	IIC address of the EEPROM
 *
 * @return	XST_SUCCESS on successful read of EEPROM
 *		Error code on failure
 *
 ******************************************************************************/
int Xbir_IicEepromReadData(u8 *BufferPtr, u16 ByteCount, u8 EepromAddr)
{
	int Status = XST_FAILURE;
	/* Eeprom Page size is 32 bytes and hence 2 bytes for array */
	u8 WriteBuffer[2U] = {0U};
	u32 TimeOutCount = XBIR_MAX_DELAY;

	Status = XIicPs_MasterSendPolled(&IicInstance, WriteBuffer,
		sizeof(WriteBuffer), EepromAddr);
	if (Status != XST_SUCCESS) {
		Status = XBIR_ERROR_IIC_MASTER_SEND;
		goto END;
	}

	/*
	 * Wait until bus is idle to start another transfer.
	 */
	while ((XIicPs_BusIsBusy(&IicInstance)) && (TimeOutCount > 0U)) {
		TimeOutCount--;
	}
	if (TimeOutCount == 0U) {
		Status = XBIR_ERROR_I2C_WRITE_TIMEOUT;
		goto END;
	}

	/* Receive the Data */
	Status = XIicPs_MasterRecvPolled(&IicInstance, BufferPtr,
		ByteCount, EepromAddr);
	if (Status != XST_SUCCESS) {
		Status = XBIR_ERROR_IIC_MASTER_RECV;
		goto END;
	}

	/*
	 * Wait until bus is idle to start another transfer.
	 */
	TimeOutCount = XBIR_MAX_DELAY;
	while ((XIicPs_BusIsBusy(&IicInstance)) && (TimeOutCount > 0U)) {
		TimeOutCount--;
	}
	if (TimeOutCount == 0U) {
		Status = XBIR_ERROR_I2C_READ_TIMEOUT;
		goto END;
	}

END:
	return Status;
}
#endif

/*****************************************************************************/
/**
 * This function perform the initial configuration for the IIC Device.
 *
 * @param	None.
 *
 * @return	XST_SUCCESS on successful intialization
 * 		Error code on failure
 *
 ****************************************************************************/
int Xbir_IicInit (void)
{
	int Status = XST_FAILURE;

#ifdef XPAR_XIICPS_NUM_INSTANCES
	XIicPs_Config *ConfigPtr;	/* Pointer to configuration data */
	u32 TimeOutCount = XBIR_MAX_DELAY;

	/* Initialize the IIC driver so that it is ready to use */
	ConfigPtr = XIicPs_LookupConfig(XBIR_I2C_EEPROM_INDEX);
	if (ConfigPtr == NULL) {
		Status = XBIR_ERROR_IIC_LKP_CONFIG;
		goto END;
	}

	Status = XIicPs_CfgInitialize(&IicInstance, ConfigPtr,
			ConfigPtr->BaseAddress);
	if (Status != XST_SUCCESS) {
		Status = XBIR_ERROR_IIC_CONFIG_INIT;
		goto END;
	}

	/* Set the IIC serial clock rate */
	Status = XIicPs_SetSClk(&IicInstance, XBIR_IIC_SCLK_RATE);
	if (Status != XST_SUCCESS) {
		Status = XBIR_ERROR_IIC_SET_SCLK;
		goto END;
	}

	while ((XIicPs_BusIsBusy(&IicInstance)) && (TimeOutCount > 0U)) {
		TimeOutCount--;
	}
	if (TimeOutCount == 0U) {
		Status = XBIR_ERROR_IIC_SET_SCLK_TIMEOUT;
		goto END;
	}

END:
#else
	Status = XST_SUCCESS;
#endif
	return Status;
}
