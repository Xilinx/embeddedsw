/******************************************************************************
* Copyright (C) 2018 – 2022 Xilinx, Inc.  All rights reserved.
* Copyright 2022-2023 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file rc21008adrv.c
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
* 1.0   ssh  25-01-2023 Initial version
*
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/
#include "rc21008adrv.h"

#if defined (XPS_BOARD_VEK280)
/************************** Constant Definitions *****************************/
#define IDT_8T49N24X_ADV_FUNC_EN 0 /* Enable unused APIs */
#if defined (XPS_BOARD_ZCU102) || \
	defined (XPS_BOARD_ZCU104) || \
	defined (XPS_BOARD_ZCU106) || \
    defined (XPS_BOARD_VCK190) || \
	defined (XPS_BOARD_VEK280)
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
static unsigned RC21008A_I2cSend(void *IicPtr, u16 SlaveAddr, u8 *MsgPtr,
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
static unsigned RC21008A_I2cSend(void *IicPtr, u16 SlaveAddr, u8 *MsgPtr,
							   unsigned ByteCount, u8 Option)
{
#if defined (XPS_BOARD_ZCU102) || \
	defined (XPS_BOARD_ZCU104) || \
	defined (XPS_BOARD_ZCU106) || \
    defined (XPS_BOARD_VCK190) || \
	defined (XPS_BOARD_VEK280)
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
int RC21008A_Init(void *IicPtr, u8 I2CSlaveAddress)
{
	u32 Status = XST_SUCCESS;
	u8 ByteCount = 0;
	u8 ByteCount_RC = 0;
	u8 Buffer[17];
	u8 Retry = 0;
	for (int i = 0; i < RC21008A_REVD_CONFIG_NUM_REGS; i++)
	{
		/* Write data */
		ByteCount_RC = rc21008a_revd_registers[i].bytecount; // ByteCount
		Buffer[0] = rc21008a_revd_registers[i].address; // Register
		Buffer[1] = rc21008a_revd_registers[i].value; // Value
		Buffer[2] = rc21008a_revd_registers[i].value1; // Value
		Buffer[3] = rc21008a_revd_registers[i].value2; // Value
		Buffer[4] = rc21008a_revd_registers[i].value3; // Value
		Buffer[5] = rc21008a_revd_registers[i].value4; // Value
		Buffer[6] = rc21008a_revd_registers[i].value5; // Value
		Buffer[7] = rc21008a_revd_registers[i].value6; // Value
		Buffer[8] = rc21008a_revd_registers[i].value7; // Value
		Buffer[9] = rc21008a_revd_registers[i].value8; // Value
		Buffer[10] = rc21008a_revd_registers[i].value9; // Value
		Buffer[11] = rc21008a_revd_registers[i].value10; // Value
		Buffer[12] = rc21008a_revd_registers[i].value11; // Value
		Buffer[13] = rc21008a_revd_registers[i].value12; // Value
		Buffer[14] = rc21008a_revd_registers[i].value13; // Value
		Buffer[15] = rc21008a_revd_registers[i].value14; // Value
		Buffer[16] = rc21008a_revd_registers[i].value15; // Value

		do {
			ByteCount = RC21008A_I2cSend(IicPtr, I2CSlaveAddress, (u8*)Buffer,
							(ByteCount_RC + 1), I2C_STOP);
			if (ByteCount != (ByteCount_RC + 1)) {
				Retry++;

				/* Maximum retries */
				if (Retry == 255) {
					xil_printf("RC21008A_Init Error!\r\n");
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

#endif
/** @} */
