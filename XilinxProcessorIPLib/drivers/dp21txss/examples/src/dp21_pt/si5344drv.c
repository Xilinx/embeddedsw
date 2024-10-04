/******************************************************************************
* Copyright (C) 2018 – 2022 Xilinx, Inc.  All rights reserved.
* Copyright 2023-2024 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file si5344drv.c
*
* This file contains the Xilinx Menu implementation as used
* in the HDMI example design. Please see xhdmi_menu.h for more details.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date       Changes
* ----- ---- ---------- --------------------------------------------------
* X.X   ..   DD-MM-YYYY ..
* 1.0        23-07-2019 Initial version
*
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/
#include "si5344drv.h"

/************************** Constant Definitions *****************************/
#define IDT_8T49N24X_ADV_FUNC_EN 0 /* Enable unused APIs */
#if defined (XPS_BOARD_ZCU102) || \
	defined (XPS_BOARD_ZCU104) || \
	defined (XPS_BOARD_ZCU106) || \
    defined (XPS_BOARD_VCK190)
#define I2C_REPEATED_START 0x01
#define I2C_STOP 0x00
#else
#define I2C_REPEATED_START XIIC_REPEATED_START
#define I2C_STOP XIIC_STOP
#endif
/***************** Macros (Inline Functions) Definitions *********************/

/**************************** Type Definitions *******************************/

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/

/************************** Function Definitions *****************************/
static unsigned SI5344_I2cSend(void *IicPtr, u16 SlaveAddr, u8 *MsgPtr,
							unsigned ByteCount, u8 Option);

/*****************************************************************************/
/**
*
* This function send the IIC data to SI 5344
*
* @param  IicPtr IIC instance pointer.
* @param  SlaveAddr contains the 7 bit IIC address of the device to send the
*		   specified data to.
* @param MsgPtr points to the data to be sent.
* @param ByteCount is the number of bytes to be sent.
* @param Option indicates whether to hold or free the bus after
* 		  transmitting the data.
*
* @return	The number of bytes sent.
*
* @return None.
*
* @note   None.
*
******************************************************************************/
static unsigned SI5344_I2cSend(void *IicPtr, u16 SlaveAddr, u8 *MsgPtr,
							   unsigned ByteCount, u8 Option)
{
#if defined (XPS_BOARD_ZCU102) || \
	defined (XPS_BOARD_ZCU104) || \
	defined (XPS_BOARD_ZCU106) || \
    defined (XPS_BOARD_VCK190)
	XIicPs *Iic_Ptr = IicPtr;
	u32 Status;

	/* Set operation to 7-bit mode */
	XIicPs_SetOptions(Iic_Ptr, XIICPS_7_BIT_ADDR_OPTION);
	XIicPs_ClearOptions(Iic_Ptr, XIICPS_10_BIT_ADDR_OPTION);

	/* Set Repeated Start option */
	if (Option == I2C_REPEATED_START) {
		XIicPs_SetOptions(Iic_Ptr, XIICPS_REP_START_OPTION);
	} else {
		XIicPs_ClearOptions(Iic_Ptr, XIICPS_REP_START_OPTION);
	}

	Status = XIicPs_MasterSendPolled(Iic_Ptr, MsgPtr, ByteCount, SlaveAddr);

	/*
	 * Wait until bus is idle to start another transfer.
	 */
	if (!(Iic_Ptr->IsRepeatedStart)) {
		while (XIicPs_BusIsBusy(Iic_Ptr));
	}

	/* This delay prevents IIC access from hanging */
	usleep(500);

	if (Status == XST_SUCCESS) {
		return ByteCount;
	} else {
		return 0;
	}
#else
	XIic *Iic_Ptr = IicPtr;
	/* This delay prevents IIC access from hanging */
	usleep(350);
	return XIic_Send(Iic_Ptr->BaseAddress, SlaveAddr, MsgPtr,
					ByteCount, Option);



#endif
}

/*****************************************************************************/
/**
*
* This function initializes the SI 5344 device
*
* @param I2CBaseAddress is the baseaddress of the I2C core.
* @param I2CSlaveAddress is the 7-bit I2C slave address.
*
* @return
*    - XST_SUCCESS Initialization was successful.
*    - XST_FAILURE I2C write error.
*
* @note None.
*
******************************************************************************/
int SI5344_Init(void *IicPtr, u8 I2CSlaveAddress)
{
	u32 Status = XST_SUCCESS;
	u8 ByteCount = 0;
	u8 Page[2];
	u8 Buffer[2];
	u8 Retry = 0;
	for (int i = 0; i < SI5344_REVD_CONFIG_NUM_REGS; i++)
	{
		/* Write data */
		Page[0] = 0x01; // Page register
		Page[1] = si5344_revd_registers[i].address >> 8; // Set Page
		Buffer[0] = si5344_revd_registers[i].address & 0xff; // Register
		Buffer[1] = si5344_revd_registers[i].value; // Value
		if (i == 3) {
			usleep(300000);
		}
		do {
			ByteCount = SI5344_I2cSend(IicPtr, I2CSlaveAddress, (u8*)Page,
								2, I2C_STOP);
			if (ByteCount != 2) {
				Retry++;

				/* Maximum retries */
				if (Retry == 255) {
					xil_printf("SI5344_Init Error!\r\n");
					return XST_FAILURE;
				}
			}
			else {
				break;
			}

		} while (Retry < 255);

		do {
			ByteCount = SI5344_I2cSend(IicPtr, I2CSlaveAddress, (u8*)Buffer,
								2, I2C_STOP);
			if (ByteCount != 2) {
				Retry++;

				/* Maximum retries */
				if (Retry == 255) {
					xil_printf("SI5344_Init Error!\r\n");
					return XST_FAILURE;
				}
			}
			else {
				break;
			}

		} while (Retry < 255);
	}

	return Status;
}

/** @} */
