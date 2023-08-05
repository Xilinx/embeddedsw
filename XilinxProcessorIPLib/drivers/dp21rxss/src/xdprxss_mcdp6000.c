/******************************************************************************
* Copyright (C) 2017 - 2020 Xilinx, Inc. All rights reserved.
* Copyright 2022-2023 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file mcdp6000.c
* @addtogroup dprxss Overview
* @{
*
* This file contains a set of functions to configure the MCDP6000.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver  Who Date     Changes
* ---- --- -------- ----------------------------------------------------------
* 1.00 Kei 01/23/18 Initial release.
* 6.0  rg  11/19/19 Modified MCDP6000 APIs to support both PS I2C and
* 		    PL I2C .
* </pre>
*
******************************************************************************/


/***************************** Include Files *********************************/
#include "xdprxss_mcdp6000.h"
#include "xdprxss.h"
#ifdef XPAR_XIIC_NUM_INSTANCES
#include "xiic.h"
#endif
#ifdef XPAR_XIICPS_NUM_INSTANCES
#include "xiicps.h"
#endif
#include "xil_types.h"
#include "xil_assert.h"
#include "xstatus.h"

/************************** Constant Definitions *****************************/

u32 MCDP6000_IC_Rev;

/***************** Macros (Inline Functions) Definitions *********************/

/**************************** Type Definitions *******************************/

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/

/************************** Function Definitions *****************************/

/*****************************************************************************/
/**
*
* This function reads a single 32b word from the MCDP6000 device
*
* @param DpRxSsPtr is a pointer to the XDpRxSs core instance.
* @param I2CSlaveAddress is the 7-bit I2C slave address.
* @param RegisterAddress is the 16-bit register address.
*
* @return
*    - The read 32b data.
*
* @note None.
*
******************************************************************************/
u32 XDpRxSs_MCDP6000_GetRegister(XDpRxSs *DpRxSsPtr, u8 I2CSlaveAddress,
		u16 RegisterAddress)
{
	u8 Buffer[4];
	u32 Data = 0;
	int i = 0;
	int Status;
	u32 I2CBaseAddress;
	u32 ByteCount = 0;
	u32 Retry = 0;

	/* Verify argument.*/
	Xil_AssertNonvoid(DpRxSsPtr != NULL);

	/* Set Address -> Little Endian */
	Buffer[0] = RegisterAddress & 0xff;
	Buffer[1] = (RegisterAddress >> 8) & 0xff;

#ifdef XPAR_XIIC_NUM_INSTANCES
	if (DpRxSsPtr->Config.IncludeAxiIic) {
		I2CBaseAddress = DpRxSsPtr->IicPtr->BaseAddress;
		while (1) {

			/* Maximum retries */
			if (Retry == 255) {
				break;
			}

			/* Set Address -> Little Endian */
			Buffer[0] = RegisterAddress & 0xff;
			Buffer[1] = (RegisterAddress >> 8) & 0xff;
			ByteCount = XIic_Send(I2CBaseAddress, I2CSlaveAddress,
					(u8*)Buffer, 2, XIIC_REPEATED_START);

			if (ByteCount != 2) {
				Retry++;
				continue;
			}

			/* Read data */
			ByteCount = XIic_Recv(I2CBaseAddress, I2CSlaveAddress,
					(u8*)Buffer, 4, XIIC_STOP);
			if (ByteCount != 4) {
				Retry++;
				continue;
			}

			for (i = 0; i < 4; i++) {
				Data |= (Buffer[i] << (i*8));
			}
			break;
		}
	}
	else
#endif
	{
#ifdef XPAR_XIICPS_NUM_INSTANCES
		Status = XIicPs_MasterSendPolled(DpRxSsPtr->IicPsPtr, Buffer,
							2, I2CSlaveAddress);
		if (Status != XST_SUCCESS) {
			return XST_FAILURE;
		}
		/*
		* Wait until bus is idle to start another transfer.
		*/
		while (XIicPs_BusIsBusy(DpRxSsPtr->IicPsPtr)) {
		/* NOP */
		}
		/* Read data */
		Status = XIicPs_MasterRecvPolled(DpRxSsPtr->IicPsPtr, Buffer,
							4, I2CSlaveAddress);
		if (Status != XST_SUCCESS) {
			return XST_FAILURE;
		}
		while (XIicPs_BusIsBusy(DpRxSsPtr->IicPsPtr)) {
		/* NOP */
		}
		for (i = 0; i < 4; i++) {
			Data |= (Buffer[i] << (i*8));
		}
#endif
	}
	return Data;
}

/*****************************************************************************/
/**
*
* This function writes a single 32b word to the MCDP6000 device
*
* @param DpRxSsPtr is a pointer to the XDpRxSs core instance.
* @param I2CSlaveAddress is the 7-bit I2C slave address.
* @param RegisterAddress is the 16-bit register address.
* @param Value is the 32b word to write
*
* @return
*    - XST_SUCCESS Register write was successful.
*    - XST_FAILURE I2C write error.
*
* @note None.
*
******************************************************************************/
int XDpRxSs_MCDP6000_SetRegister(XDpRxSs *DpRxSsPtr, u8 I2CSlaveAddress,
			 u16 RegisterAddress, u32 Value)
{
	u8 Buffer[6];
	int Status;
	u32 I2CBaseAddress;
	u32 ByteCount = 0;
	u32 Retry = 0;

	/* Verify argument.*/
	Xil_AssertNonvoid(DpRxSsPtr != NULL);

	/* Set Address -> Little Endian */
	Buffer[0] = RegisterAddress & 0xff;
	Buffer[1] = (RegisterAddress >> 8) & 0xff;

	/* Write data -> Little Endian */
	Buffer[2] = Value & 0xff;
	Buffer[3] = (Value >>  8) & 0xff;
	Buffer[4] = (Value >> 16) & 0xff;
	Buffer[5] = (Value >> 24) & 0xff;

#ifdef XPAR_XIIC_NUM_INSTANCES
	if (DpRxSsPtr->Config.IncludeAxiIic) {
		I2CBaseAddress = DpRxSsPtr->IicPtr->BaseAddress;
		while (1) {
			ByteCount = XIic_Send(I2CBaseAddress, I2CSlaveAddress,
					      (u8*)Buffer, 6, XIIC_STOP);
			if (ByteCount != 6) {
				Retry++;

				/* Maximum retries */
				if (Retry == 255) {
					return XST_FAILURE;
				}
			}
			else {
				return XST_SUCCESS;
			}
		}
	}
	else
#endif
	{
#ifdef XPAR_XIICPS_NUM_INSTANCES
		Status = XIicPs_MasterSendPolled(DpRxSsPtr->IicPsPtr, Buffer,
						6, I2CSlaveAddress);
		if (Status != XST_SUCCESS) {
			return XST_FAILURE;
		}
		/*
		 * Wait until bus is idle to start another transfer.
		 */
		while (XIicPs_BusIsBusy(DpRxSsPtr->IicPsPtr)) {
			/* NOP */
		}
#endif
	}
	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* This function modifies a single 32b word from the MCDP6000 device
*
* @param DpRxSsPtr is a pointer to the XDpRxSs core instance.
* @param I2CSlaveAddress is the 7-bit I2C slave address.
* @param RegisterAddress is the 16-bit register address.
* @param Value is the 32b word to write
* @param Mask is the 32b mask
*
* @return
*    - XST_SUCCESS Modification was successful.
*    - XST_FAILURE I2C write error.
*
* @note None.
*
******************************************************************************/
int XDpRxSs_MCDP6000_ModifyRegister(XDpRxSs *DpRxSsPtr, u8 I2CSlaveAddress,
			    u16 RegisterAddress, u32 Value, u32 Mask)
{
	u32 Data;
	int Result;

	/* Verify argument.*/
	Xil_AssertNonvoid(DpRxSsPtr != NULL);

	/* Read data */
	Data = XDpRxSs_MCDP6000_GetRegister(DpRxSsPtr, I2CSlaveAddress,
				    RegisterAddress);

	/* Clear masked bits */
	Data &= ~Mask;

	/* Update */
	Data |= (Value & Mask);

	/* Write data */
	Result = XDpRxSs_MCDP6000_SetRegister(DpRxSsPtr, I2CSlaveAddress,
				      RegisterAddress, Data);

	return Result;
}

/*****************************************************************************/
/**
*
* This function initializes the MCDP6000 device with default values
* for DP use with the Video FMC.
*
* @param DpRxSsPtr is a pointer to the XDpRxSs core instance.
* @param I2CSlaveAddress is the 7-bit I2C slave address.
*
* @return
*    - XST_SUCCESS Initialization was successful.
*    - XST_FAILURE I2C write error.
*
* @note None.
*
******************************************************************************/
int XDpRxSs_MCDP6000_DpInit(XDpRxSs *DpRxSsPtr, u8 I2CSlaveAddress)
{
	int Result;
	u32 MCDP6000_BS;

	/* Verify argument.*/
	Xil_AssertNonvoid(DpRxSsPtr != NULL);

	MCDP6000_IC_Rev = XDpRxSs_MCDP6000_GetRegister(DpRxSsPtr, I2CSlaveAddress, 0x0510) & 0xFF00;
	MCDP6000_BS = XDpRxSs_MCDP6000_GetRegister(DpRxSsPtr, I2CSlaveAddress, 0x0510) & 0x1C;

	/* According to AppNote DP Retimer Use Case rev 1.0.0, June 6th, 2017
	 * Table 1, pg7; Exit from disabled state to DP 4 lane with normal
	 * plug orientation. The data lanes need to be swapped so use the
	 * inverted plug orientation instead.
	 */
	/*AUX Setting to add latency for data forwarding*/

	if (MCDP6000_IC_Rev == 0x2100) {
		Result = XDpRxSs_MCDP6000_SetRegister(DpRxSsPtr, I2CSlaveAddress,
					      0x0350, 0x0000001F);
		if (Result != XST_SUCCESS) {
			return XST_FAILURE;
		}


		Result = XDpRxSs_MCDP6000_SetRegister(DpRxSsPtr, I2CSlaveAddress,
					      0x0504, 0x0000705E);
		if (Result != XST_SUCCESS) {
			return XST_FAILURE;
		}

		Result = XDpRxSs_MCDP6000_SetRegister(DpRxSsPtr, I2CSlaveAddress,
					      0x278C, 0x00000190);
		if (Result != XST_SUCCESS) {
			return XST_FAILURE;
		}


		Result = XDpRxSs_MCDP6000_SetRegister(DpRxSsPtr, I2CSlaveAddress,
					      0x010C, 0x0F0F2D24);
		if (Result != XST_SUCCESS) {
			return XST_FAILURE;
		}

		Result = XDpRxSs_MCDP6000_SetRegister(DpRxSsPtr, I2CSlaveAddress,
					      0x0504, 0x0000715E);
		if (Result != XST_SUCCESS) {
			return XST_FAILURE;
		}

		Result = XDpRxSs_MCDP6000_SetRegister(DpRxSsPtr, I2CSlaveAddress,
					      0x0504, 0x0000705E);
		if (Result != XST_SUCCESS) {
			return XST_FAILURE;
		}

		Result = XDpRxSs_MCDP6000_SetRegister(DpRxSsPtr, I2CSlaveAddress,
					      0x2614, 0x1A070F0F);
		if (Result != XST_SUCCESS) {
			return XST_FAILURE;
		}

		Result = XDpRxSs_MCDP6000_SetRegister(DpRxSsPtr, I2CSlaveAddress,
					      0x01A0, 0xCC884444);
		if (Result != XST_SUCCESS) {
			return XST_FAILURE;
		}

		Result = XDpRxSs_MCDP6000_SetRegister(DpRxSsPtr, I2CSlaveAddress,
					      0x01C0, 0x2C00A81E);
		if (Result != XST_SUCCESS) {
			return XST_FAILURE;
		}

		Result = XDpRxSs_MCDP6000_SetRegister(DpRxSsPtr, I2CSlaveAddress,
					      0x01D0, 0x0000C360);
		if (Result != XST_SUCCESS) {
			return XST_FAILURE;
		}

		Result = XDpRxSs_MCDP6000_SetRegister(DpRxSsPtr, I2CSlaveAddress,
					      0x0178, 0x13471480);
		if (Result != XST_SUCCESS) {
			return XST_FAILURE;
		}

		/* Setting MC to be fully transparent mode */
		/* Need to set bit10 for Xilinx fixed length mode. */
		Result = XDpRxSs_MCDP6000_SetRegister(DpRxSsPtr, I2CSlaveAddress,
					      0x0908, 0x0C00);
		if (Result != XST_SUCCESS) {
			return XST_FAILURE;
		}

		Result = XDpRxSs_MCDP6000_SetRegister(DpRxSsPtr, I2CSlaveAddress,
					      0x0B00, 0x0000);
		if (Result != XST_SUCCESS) {
			return XST_FAILURE;
		}

		Result = XDpRxSs_MCDP6000_SetRegister(DpRxSsPtr, I2CSlaveAddress,
					      0x0B04, 0x0000);
		if (Result != XST_SUCCESS) {
			return XST_FAILURE;
		}

		Result = XDpRxSs_MCDP6000_SetRegister(DpRxSsPtr, I2CSlaveAddress,
					      0x090C, 0x02020000);
		if (Result != XST_SUCCESS) {
			return XST_FAILURE;
		}

		Result = XDpRxSs_MCDP6000_SetRegister(DpRxSsPtr, I2CSlaveAddress,
					      0x0A00, 0x55801E10);
		if (Result != XST_SUCCESS) {
			return XST_FAILURE;
		}

		/*AUX Swing Adjustment - For lower DP1.4 swing guidance of 400 mV*/
		Result = XDpRxSs_MCDP6000_SetRegister(DpRxSsPtr, I2CSlaveAddress,
					      0x1608, 0x00748404);
		if (Result != XST_SUCCESS) {
			return XST_FAILURE;
		}
		Result = XDpRxSs_MCDP6000_SetRegister(DpRxSsPtr, I2CSlaveAddress,
					      0x2608, 0x00748404);
		if (Result != XST_SUCCESS) {
			return XST_FAILURE;
		}
	} else if (MCDP6000_IC_Rev==0x3100) {
		Result = XDpRxSs_MCDP6000_SetRegister(DpRxSsPtr, I2CSlaveAddress,
					      0x0350, 0x0000001F);
		if (Result != XST_SUCCESS) {
			return XST_FAILURE;
		}

		Result = XDpRxSs_MCDP6000_SetRegister(DpRxSsPtr, I2CSlaveAddress,
					      0x0504, 0x0001705E);
		if (Result != XST_SUCCESS) {
			return XST_FAILURE;
		}

		Result = XDpRxSs_MCDP6000_SetRegister(DpRxSsPtr, I2CSlaveAddress,
					      0x01C0, 0x2C002C9E);
		if (Result != XST_SUCCESS) {
			return XST_FAILURE;
		}

		Result = XDpRxSs_MCDP6000_SetRegister(DpRxSsPtr, I2CSlaveAddress,
					      0x092C, 0x5555A5A5);
		if (Result != XST_SUCCESS) {
			return XST_FAILURE;
		}

		Result = XDpRxSs_MCDP6000_SetRegister(DpRxSsPtr, I2CSlaveAddress,
					      0x0900, 0x04010506);
		if (Result != XST_SUCCESS) {
			return XST_FAILURE;
		}

		Result = XDpRxSs_MCDP6000_SetRegister(DpRxSsPtr, I2CSlaveAddress,
					      0x0178, 0x13471480);
		if (Result != XST_SUCCESS) {
			return XST_FAILURE;
		}

		Result = XDpRxSs_MCDP6000_SetRegister(DpRxSsPtr, I2CSlaveAddress,
					      0x01A0, 0xCC884444);
		if (Result != XST_SUCCESS) {
			return XST_FAILURE;
		}

		Result = XDpRxSs_MCDP6000_SetRegister(DpRxSsPtr, I2CSlaveAddress,
					      0x2614, 0x19890F0F);
		if (Result != XST_SUCCESS) {
			return XST_FAILURE;
		}

		if (MCDP6000_BS == 0x18) {
			Result = XDpRxSs_MCDP6000_SetRegister(DpRxSsPtr, I2CSlaveAddress,
						      0x2340, 0x00000500);
			if (Result != XST_SUCCESS) {
				return XST_FAILURE;
			}

			Result = XDpRxSs_MCDP6000_SetRegister(DpRxSsPtr, I2CSlaveAddress,
						      0x2540, 0x00000500);
			if (Result != XST_SUCCESS) {
				return XST_FAILURE;
			}
		} else if (MCDP6000_BS == 0x8) {
			Result = XDpRxSs_MCDP6000_SetRegister(DpRxSsPtr, I2CSlaveAddress,
						      0x2240, 0x00000500);
			if (Result != XST_SUCCESS) {
				return XST_FAILURE;
			}

			Result = XDpRxSs_MCDP6000_SetRegister(DpRxSsPtr, I2CSlaveAddress,
						      0x2440, 0x00000500);
			if (Result != XST_SUCCESS) {
				return XST_FAILURE;
			}
		}
		/*AUX Swing Adjustment - For lower DP1.4 swing guidance of 400 mV*/
		Result = XDpRxSs_MCDP6000_SetRegister(DpRxSsPtr, I2CSlaveAddress,
					      0x1608, 0x00748404);
		if (Result != XST_SUCCESS) {
			return XST_FAILURE;
		}
		Result = XDpRxSs_MCDP6000_SetRegister(DpRxSsPtr, I2CSlaveAddress,
					      0x2608, 0x00748404);
		if (Result != XST_SUCCESS) {
			return XST_FAILURE;
		}
	} else if (MCDP6000_IC_Rev==0x3200) {
		Result = XDpRxSs_MCDP6000_SetRegister(DpRxSsPtr, I2CSlaveAddress,
				0x024C, 0x22221A50);
		if (Result != XST_SUCCESS) {
			return XST_FAILURE;
		}

		Result = XDpRxSs_MCDP6000_SetRegister(DpRxSsPtr, I2CSlaveAddress,
					      0x0350, 0x0000001F);
		if (Result != XST_SUCCESS) {
			return XST_FAILURE;
		}

		Result = XDpRxSs_MCDP6000_SetRegister(DpRxSsPtr, I2CSlaveAddress,
					      0x0504, 0x0001705E);
		if (Result != XST_SUCCESS) {
			return XST_FAILURE;
		}

		Result = XDpRxSs_MCDP6000_SetRegister(DpRxSsPtr, I2CSlaveAddress,
					      0x2614, 0x19890F0F);
		if (Result != XST_SUCCESS) {
			return XST_FAILURE;
		}

		Result = XDpRxSs_MCDP6000_SetRegister(DpRxSsPtr, I2CSlaveAddress,
					      0x01D8, 0x00000601);
		if (Result != XST_SUCCESS) {
			return XST_FAILURE;
		}

		Result = XDpRxSs_MCDP6000_SetRegister(DpRxSsPtr, I2CSlaveAddress,
					      0x0660, 0x00005011);
		if (Result != XST_SUCCESS) {
			return XST_FAILURE;
		}

		Result = XDpRxSs_MCDP6000_SetRegister(DpRxSsPtr, I2CSlaveAddress,
					      0x067C, 0x00000001);
		if (Result != XST_SUCCESS) {
			return XST_FAILURE;
		}

		Result = XDpRxSs_MCDP6000_SetRegister(DpRxSsPtr, I2CSlaveAddress,
					      0x0908, 0x00000866);
		if (Result != XST_SUCCESS) {
			return XST_FAILURE;
		}

		Result = XDpRxSs_MCDP6000_SetRegister(DpRxSsPtr, I2CSlaveAddress,
					      0x090C, 0x04020000);
		if (Result != XST_SUCCESS) {
			return XST_FAILURE;
		}

		if (MCDP6000_BS == 0x18) {
			Result = XDpRxSs_MCDP6000_SetRegister(DpRxSsPtr, I2CSlaveAddress,
						      0x2340, 0x00000500);
			if (Result != XST_SUCCESS) {
				return XST_FAILURE;
			}

			Result = XDpRxSs_MCDP6000_SetRegister(DpRxSsPtr, I2CSlaveAddress,
						      0x2540, 0x00000500);
			if (Result != XST_SUCCESS) {
				return XST_FAILURE;
			}
		} else if (MCDP6000_BS == 0x8) {
			Result = XDpRxSs_MCDP6000_SetRegister(DpRxSsPtr, I2CSlaveAddress,
						      0x2240, 0x00000500);
			if (Result != XST_SUCCESS) {
				return XST_FAILURE;
			}

			Result = XDpRxSs_MCDP6000_SetRegister(DpRxSsPtr, I2CSlaveAddress,
						      0x2440, 0x00000500);
			if (Result != XST_SUCCESS) {
				return XST_FAILURE;
			}
		}
	} else {
		Result = XST_SUCCESS;
	}

	return Result;
}

/*****************************************************************************/
/**
*
* This function initializes the MCDP6000 device with default values
* for IBERT use with the Video FMC.
*
* @param DpRxSsPtr is a pointer to the XDpRxSs core instance.
* @param I2CSlaveAddress is the 7-bit I2C slave address.
*
* @return
*    - XST_SUCCESS Initialization was successful.
*    - XST_FAILURE I2C write error.
*
* @note None.
*
******************************************************************************/
int XDpRxSs_MCDP6000_IbertInit(XDpRxSs *DpRxSsPtr, u8 I2CSlaveAddress)
{

	unsigned int Result = XST_FAILURE;

	/* Verify argument.*/
	Xil_AssertNonvoid(DpRxSsPtr != NULL);

	if (MCDP6000_IC_Rev == 0x2100) {
		Result = XDpRxSs_MCDP6000_SetRegister(DpRxSsPtr, I2CSlaveAddress,
					      0x0504, 0x0000704E);
		if (Result != XST_SUCCESS) {
			return XST_FAILURE;
		}

		Result = XDpRxSs_MCDP6000_SetRegister(DpRxSsPtr, I2CSlaveAddress,
					      0x278C, 0x00000190);
		if (Result != XST_SUCCESS) {
			return XST_FAILURE;
		}

		Result = XDpRxSs_MCDP6000_SetRegister(DpRxSsPtr, I2CSlaveAddress,
					      0x010C, 0x0F0F2D24);
		if (Result != XST_SUCCESS) {
			return XST_FAILURE;
		}

		Result = XDpRxSs_MCDP6000_SetRegister(DpRxSsPtr, I2CSlaveAddress,
					      0x01A0, 0xCC884444);
		if (Result != XST_SUCCESS) {
			return XST_FAILURE;
		}

		Result = XDpRxSs_MCDP6000_SetRegister(DpRxSsPtr, I2CSlaveAddress,
					      0x01D0, 0x0000B360);
		if (Result != XST_SUCCESS) {
			return XST_FAILURE;
		}

		Result = XDpRxSs_MCDP6000_SetRegister(DpRxSsPtr, I2CSlaveAddress,
					      0x0504, 0x0000714E);
		if (Result != XST_SUCCESS) {
			return XST_FAILURE;
		}

		Result = XDpRxSs_MCDP6000_SetRegister(DpRxSsPtr, I2CSlaveAddress,
					      0x0504, 0x0000704E);
		if (Result != XST_SUCCESS) {
			return XST_FAILURE;
		}

		Result = XDpRxSs_MCDP6000_SetRegister(DpRxSsPtr, I2CSlaveAddress,
					      0x01C0, 0x2C00A81E);
		if (Result != XST_SUCCESS) {
			return XST_FAILURE;
		}

		Result = XDpRxSs_MCDP6000_SetRegister(DpRxSsPtr, I2CSlaveAddress,
					      0x0178, 0x13471480);
		if (Result != XST_SUCCESS) {
			return XST_FAILURE;
		}

		Result = XDpRxSs_MCDP6000_SetRegister(DpRxSsPtr, I2CSlaveAddress,
					      0x0604, 0x0000F004);
		if (Result != XST_SUCCESS) {
			return XST_FAILURE;
		}

		Result = XDpRxSs_MCDP6000_SetRegister(DpRxSsPtr, I2CSlaveAddress,
					      0x067C, 0x00000002);
		if (Result != XST_SUCCESS) {
			return XST_FAILURE;
		}

		Result = XDpRxSs_MCDP6000_SetRegister(DpRxSsPtr, I2CSlaveAddress,
					      0x0630, 0x0000041E);
		if (Result != XST_SUCCESS) {
			return XST_FAILURE;
		}

		Result = XDpRxSs_MCDP6000_SetRegister(DpRxSsPtr, I2CSlaveAddress,
					      0x0350, 0x0000001F);
		if (Result != XST_SUCCESS) {
			return XST_FAILURE;
		}

		Result = XDpRxSs_MCDP6000_SetRegister(DpRxSsPtr, I2CSlaveAddress,
					      0x0150, 0x00180000);
	}
	return Result;
}

/*****************************************************************************/
/**
*
* This function requests a reset of the CR path of the MCDP6000 device
*
* @param DpRxSsPtr is a pointer to the XDpRxSs core instance.
* @param I2CSlaveAddress is the 7-bit I2C slave address.
*
* @return
*    - XST_SUCCESS Reset request was successful.
*    - XST_FAILURE I2C error.
*
* @note None.
*
******************************************************************************/
int XDpRxSs_MCDP6000_ResetCrPath(XDpRxSs *DpRxSsPtr, u8 I2CSlaveAddress)
{
	int Result;

	/* Verify argument.*/
	Xil_AssertNonvoid(DpRxSsPtr != NULL);

	/* Set Reset bits : This is actually getting out from DP mode.
	 * Eventually reset the chip. */
	Result = XDpRxSs_MCDP6000_ModifyRegister(DpRxSsPtr, I2CSlaveAddress,
				      0x150, 0x00008000, 0x00008000);
	if (Result != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/* Clear Reset bits  */
	Result = XDpRxSs_MCDP6000_ModifyRegister(DpRxSsPtr, I2CSlaveAddress,
				      0x150, 0x00000000, 0x00008000);
	if (Result != XST_SUCCESS) {
		return XST_FAILURE;
	}

	return Result;
}


/*****************************************************************************/
/**
*
* This function requests a reset of the DP path of the MCDP6000 device
*
* @param DpRxSsPtr is a pointer to the XDpRxSs core instance.
* @param I2CSlaveAddress is the 7-bit I2C slave address.
*
* @return
*    - XST_SUCCESS Reset request was successful.
*    - XST_FAILURE I2C error.
*
* @note None.
*
******************************************************************************/
int XDpRxSs_MCDP6000_ResetDpPath(XDpRxSs *DpRxSsPtr, u8 I2CSlaveAddress)
{
	int Result;

	/* Verify argument.*/
	Xil_AssertNonvoid(DpRxSsPtr != NULL);

	/* Set Reset bits : This is actually getting out from DP mode.
	 * Eventually reset the chip. */
	Result = XDpRxSs_MCDP6000_SetRegister(DpRxSsPtr, I2CSlaveAddress,
				      0x0504, 0x1715E);
	if (Result != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/* Clear Reset bits  */
	Result = XDpRxSs_MCDP6000_SetRegister(DpRxSsPtr, I2CSlaveAddress,
				      0x0504, 0x1705E);
	if (Result != XST_SUCCESS) {
		return XST_FAILURE;
	}

	return Result;
}

/*****************************************************************************/
/**
*
* This function enables the PRBS7 output of the MCDP6000 device
*
* @param DpRxSsPtr is a pointer to the XDpRxSs core instance.
* @param I2CSlaveAddress is the 7-bit I2C slave address.
*
* @return
*    - XST_SUCCESS Enable request was successful.
*    - XST_FAILURE I2C error.
*
* @note None.
*
******************************************************************************/
int XDpRxSs_MCDP6000_EnablePrbs7_Tx(XDpRxSs *DpRxSsPtr, u8 I2CSlaveAddress)
{
	unsigned int Result = XST_FAILURE;

	/* Verify argument.*/
	Xil_AssertNonvoid(DpRxSsPtr != NULL);

	if (MCDP6000_IC_Rev == 0x2100) {
		Result = XDpRxSs_MCDP6000_SetRegister(DpRxSsPtr, I2CSlaveAddress,
					      0x024C, 0xAAAA9A05);
		if (Result != XST_SUCCESS) {
			return XST_FAILURE;
		}

		Result = XDpRxSs_MCDP6000_SetRegister(DpRxSsPtr, I2CSlaveAddress,
					      0x0150, 0x00100000);
		if (Result != XST_SUCCESS) {
			return XST_FAILURE;
		}

		Result = XDpRxSs_MCDP6000_SetRegister(DpRxSsPtr, I2CSlaveAddress,
					      0x0604, 0x0000F004);
		if (Result != XST_SUCCESS) {
			return XST_FAILURE;
		}

		Result = XDpRxSs_MCDP6000_SetRegister(DpRxSsPtr, I2CSlaveAddress,
					      0x0630, 0x0000041E);
		if (Result != XST_SUCCESS) {
			return XST_FAILURE;
		}

		Result = XDpRxSs_MCDP6000_SetRegister(DpRxSsPtr, I2CSlaveAddress,
					      0x0668, 0x00000001);
		if (Result != XST_SUCCESS) {
			return XST_FAILURE;
		}

		Result = XDpRxSs_MCDP6000_SetRegister(DpRxSsPtr, I2CSlaveAddress,
					      0x0674, 0x00000001);
		if (Result != XST_SUCCESS) {
			return XST_FAILURE;
		}

		Result = XDpRxSs_MCDP6000_SetRegister(DpRxSsPtr, I2CSlaveAddress,
					      0x0680, 0x00070000);
	}

	return Result;
}

/*****************************************************************************/
/**
*
* This function enables the PRBS7 counter mode in MC Rx path
* Used in DP PHY compliance mode
*
* @param DpRxSsPtr is a pointer to the XDpRxSs core instance.
* @param I2CSlaveAddress is the 7-bit I2C slave address.
*
* @return
*    - XST_SUCCESS Enable request was successful.
*    - XST_FAILURE I2C error.
*
* @note None.
*
******************************************************************************/
int XDpRxSs_MCDP6000_EnablePrbs7_Rx(XDpRxSs *DpRxSsPtr, u8 I2CSlaveAddress)
{
	int Result = XST_FAILURE;
	unsigned int ReadVal;

	/* Verify argument.*/
	Xil_AssertNonvoid(DpRxSsPtr != NULL);

	if (MCDP6000_IC_Rev == 0x2100) {
		/* Enable PRBS Mode */
		ReadVal = XDpRxSs_MCDP6000_GetRegister(DpRxSsPtr, I2CSlaveAddress,
					       0x0614);
		Result = XDpRxSs_MCDP6000_SetRegister(DpRxSsPtr, I2CSlaveAddress,
					      0x0614, (ReadVal | 0x800));
		if (Result != XST_SUCCESS) {
			return XST_FAILURE;
		}
	}

	return Result;
}

/*****************************************************************************/
/**
*
* This function Disables the PRBS7 counter mode in MC Rx path
* Used in DP PHY compliance mode
*
* @param DpRxSsPtr is a pointer to the XDpRxSs core instance.
* @param I2CSlaveAddress is the 7-bit I2C slave address.
*
* @return
*    - XST_SUCCESS Enable request was successful.
*    - XST_FAILURE I2C error.
*
* @note None.
*
******************************************************************************/
int XDpRxSs_MCDP6000_DisablePrbs7_Rx(XDpRxSs *DpRxSsPtr, u8 I2CSlaveAddress)
{
	unsigned int ReadVal;
	unsigned int Result;
	/* Verify argument.*/
	Xil_AssertNonvoid(DpRxSsPtr != NULL);

	/* Disable PRBS Mode */
	if (MCDP6000_IC_Rev == 0x2100) {
		ReadVal = XDpRxSs_MCDP6000_GetRegister(DpRxSsPtr, I2CSlaveAddress,
					       0x0614);
		Result = XDpRxSs_MCDP6000_SetRegister(DpRxSsPtr, I2CSlaveAddress,
					      0x0614, (ReadVal & ~0xFFFFF7FF));
		if (Result != XST_SUCCESS) {
			return XST_FAILURE;
		}
	}

	return Result;
}

/*****************************************************************************/
/**
*
* This function enables symbol counter
*
* @param DpRxSsPtr is a pointer to the XDpRxSs core instance.
* @param I2CSlaveAddress is the 7-bit I2C slave address.
*
* @return
*    - XST_SUCCESS Enable request was successful.
*    - XST_FAILURE I2C error.
*
* @note None.
*
******************************************************************************/
int XDpRxSs_MCDP6000_EnableCounter(XDpRxSs *DpRxSsPtr, u8 I2CSlaveAddress)
{
	int Result;
	unsigned int ReadVal;

	/* Verify argument.*/
	Xil_AssertNonvoid(DpRxSsPtr != NULL);

	/* Enable Symbol Counter Always*/
	ReadVal = XDpRxSs_MCDP6000_GetRegister(DpRxSsPtr, I2CSlaveAddress,
				       0x061c);
	Result = XDpRxSs_MCDP6000_SetRegister(DpRxSsPtr, I2CSlaveAddress,
				      0x061c, (ReadVal | 0x1));
	return Result;
}

/*****************************************************************************/
/**
*
* This function clears symbol counter
*
* @param DpRxSsPtr is a pointer to the XDpRxSs core instance.
* @param I2CSlaveAddress is the 7-bit I2C slave address.
*
* @return
*    - XST_SUCCESS Enable request was successful.
*    - XST_FAILURE I2C error.
*
* @note None.
*
******************************************************************************/
int XDpRxSs_MCDP6000_ClearCounter(XDpRxSs *DpRxSsPtr, u8 I2CSlaveAddress)
{		int Result;
	unsigned int ReadVal;

	/* Verify argument.*/
	Xil_AssertNonvoid(DpRxSsPtr != NULL);

	/* Enable Symbol Counter Always*/
	ReadVal = XDpRxSs_MCDP6000_GetRegister(DpRxSsPtr, I2CSlaveAddress,
				       0x061c);
	Result = XDpRxSs_MCDP6000_SetRegister(DpRxSsPtr, I2CSlaveAddress,
				      0x061c, (ReadVal & 0xFFFFFFFE));
	return Result;
}

/*****************************************************************************/
/**
*
* This function reads error counters for all lanes
*
* @param DpRxSsPtr is a pointer to the XDpRxSs core instance.
* @param I2CSlaveAddress is the 7-bit I2C slave address.
*
* @return
*    - XST_SUCCESS Enable request was successful.
*    - XST_FAILURE I2C error.
*
* @note None.
*
******************************************************************************/
int XDpRxSs_MCDP6000_Read_ErrorCounters(XDpRxSs *DpRxSsPtr, u8 I2CSlaveAddress)
{
	int Result;
	unsigned int ReadVal;

	/* Verify argument.*/
	Xil_AssertNonvoid(DpRxSsPtr != NULL);

	/* Set lane count to 0 in [2:1] */
	ReadVal = XDpRxSs_MCDP6000_GetRegister(DpRxSsPtr, I2CSlaveAddress,
				       0x061C);
	Result = XDpRxSs_MCDP6000_SetRegister(DpRxSsPtr, I2CSlaveAddress,
				      0x061C, (ReadVal & 0xFFFFFFF9));
	if (Result != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/* Read error counter */
	ReadVal = XDpRxSs_MCDP6000_GetRegister(DpRxSsPtr, I2CSlaveAddress,
				       0x0620);
	xil_printf("MCDP Error Counter (Lane0): %0x%x \n\r",ReadVal);

	/* Set lane count to 1 in [2:1] */
	ReadVal = XDpRxSs_MCDP6000_GetRegister(DpRxSsPtr, I2CSlaveAddress,
				       0x061C);
	Result = XDpRxSs_MCDP6000_SetRegister(DpRxSsPtr, I2CSlaveAddress,
				      0x061C, (ReadVal | 0x2));
	if (Result != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/* Read error counter */
	ReadVal = XDpRxSs_MCDP6000_GetRegister(DpRxSsPtr, I2CSlaveAddress,
				       0x0620);
	xil_printf("MCDP Error Counter (Lane1): %0x%x \n\r",ReadVal);

	/* Set lane count to 2 in [2:1] */
	ReadVal = XDpRxSs_MCDP6000_GetRegister(DpRxSsPtr, I2CSlaveAddress,
				       0x061C);
	Result = XDpRxSs_MCDP6000_SetRegister(DpRxSsPtr, I2CSlaveAddress,
				      0x061C, ReadVal | 0x4);
	if (Result != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/* Read error counter */
	ReadVal = XDpRxSs_MCDP6000_GetRegister(DpRxSsPtr, I2CSlaveAddress,
				       0x0620);
	xil_printf("MCDP Error Counter (Lane2): %0x%x \n\r",ReadVal);

	/* Set lane count to 3 in [2:1] */
	ReadVal = XDpRxSs_MCDP6000_GetRegister(DpRxSsPtr, I2CSlaveAddress,
				        0x061C);
	Result = XDpRxSs_MCDP6000_SetRegister(DpRxSsPtr, I2CSlaveAddress,
				      0x061C, ReadVal | 0x6);
	if (Result != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/* Read error counter */
	ReadVal = XDpRxSs_MCDP6000_GetRegister(DpRxSsPtr, I2CSlaveAddress,
				       0x0620);
	xil_printf("MCDP Error Counter (Lane3): %0x%x \n\r",ReadVal);

	return Result;
}

/*****************************************************************************/
/**
*
* This function displays a registerdump of the MCDP6000 device.
*
* @param DpRxSsPtr is a pointer to the XDpRxSs core instance.
* @param I2CSlaveAddress is the 7-bit I2C slave address.
*
* @return None
*
* @note None.
*
******************************************************************************/
void XDpRxSs_MCDP6000_RegisterDump(XDpRxSs *DpRxSsPtr, u8 I2CSlaveAddress)
{
	u32 Data;
	u32 i;

	/* Verify argument.*/
	Xil_AssertVoid(DpRxSsPtr != NULL);

	xil_printf("\n\r");
	xil_printf("---------------------\n\r");
	xil_printf("- MCDP6000 I2C dump:\n\r");
	xil_printf("---------------------\n\r");

	for (i = 0x200; i < 0xC00; i+=4) {
		if ((i % 4) == 0) {
			xil_printf("\n\r%04x : ", i);
		}
		Data = XDpRxSs_MCDP6000_GetRegister(DpRxSsPtr, I2CSlaveAddress, i);
		xil_printf("%08x ", Data);
	}

	for (i = 0x1200; i < 0x1600; i+=4) {
		if ((i % 4) == 0) {
			xil_printf("\n\r%04x : ", i);
		}
		Data = XDpRxSs_MCDP6000_GetRegister(DpRxSsPtr, I2CSlaveAddress, i);
		xil_printf("%08x ", Data);
	}

	for (i = 0x2200; i < 0x2600; i+=4) {
		if ((i % 4) == 0) {
			xil_printf("\n\r%04x : ", i);
		}
		Data = XDpRxSs_MCDP6000_GetRegister(DpRxSsPtr, I2CSlaveAddress, i);
		xil_printf("%08x ", Data);
	}

	for (i = 0x2700; i < 0x2800; i+=4) {
		if ((i % 4) == 0) {
			xil_printf("\n\r%04x : ", i);
		}
		Data = XDpRxSs_MCDP6000_GetRegister(DpRxSsPtr, I2CSlaveAddress, i);
		xil_printf("%08x ", Data);
	}
	xil_printf("\n\r");
}

/*****************************************************************************/
/**
*
* This function sets the transparent mode of the MCDP6000 device.
*
* @param DpRxSsPtr is a pointer to the XDpRxSs core instance.
* @param I2CSlaveAddress is the 7-bit I2C slave address.
*
* @return
*    - XST_SUCCESS transparent mode was set.
*    - XST_FAILURE setting transparent mode failed.
*
* @note None.
*
******************************************************************************/
int XDpRxSs_MCDP6000_TransparentMode(XDpRxSs *DpRxSsPtr, u8 I2CSlaveAddress){
	int Result;

	/* Verify argument.*/
	Xil_AssertNonvoid(DpRxSsPtr != NULL);

	Result = XDpRxSs_MCDP6000_SetRegister(DpRxSsPtr, I2CSlaveAddress,
				       0x0908, 0x00000800);
	if (Result != XST_SUCCESS) {
		return XST_FAILURE;
	}

	Result = XDpRxSs_MCDP6000_SetRegister(DpRxSsPtr, I2CSlaveAddress,
				       0x090C, 0x00020000);
	if (Result != XST_SUCCESS) {
		return XST_FAILURE;
	}

	return Result;
}

/*****************************************************************************/
/**
*
* This function changes the bandwidth of the MCDP6000 device.
*
* @param DpRxSsPtr is a pointer to the XDpRxSs core instance.
* @param I2CSlaveAddress is the 7-bit I2C slave address.
*
* @return
*    - XST_SUCCESS BW change successful.
*    - XST_FAILURE BW change failed.
*
* @note None.
*
******************************************************************************/
int XDpRxSs_MCDP6000_BWchange(XDpRxSs *DpRxSsPtr, u8 I2CSlaveAddress)
{
	int Result;

	/* Verify argument.*/
	Xil_AssertNonvoid(DpRxSsPtr != NULL);

	/* Set Reset bits (bits 10) */
	Result = XDpRxSs_MCDP6000_ModifyRegister(DpRxSsPtr, I2CSlaveAddress,
					 0x0504, 0x400, 0x400);
	if (Result != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/* Clear Reset bits (bits 10) */
	Result = XDpRxSs_MCDP6000_ModifyRegister(DpRxSsPtr, I2CSlaveAddress,
					 0x0504, 0x000, 0x400);
	if (Result != XST_SUCCESS) {
		return XST_FAILURE;
	}

	return Result;
}

/*****************************************************************************/
/**
*
* This function sets the access lane register of the MCDP6000 device.
*
* @param DpRxSsPtr is a pointer to the XDpRxSs core instance.
* @param I2CSlaveAddress is the 7-bit I2C slave address.
*
* @return
*    - XST_SUCCESS access lane set successful.
*    - XST_FAILURE setting access lane failed.
*
* @note None.
*
******************************************************************************/
int XDpRxSs_MCDP6000_AccessLaneSet(XDpRxSs *DpRxSsPtr, u8 I2CSlaveAddress)
{
	int Result;

	/* Verify argument.*/
	Xil_AssertNonvoid(DpRxSsPtr != NULL);

	/* Set Reset bits (bits 0) */
	Result = XDpRxSs_MCDP6000_SetRegister(DpRxSsPtr, I2CSlaveAddress,
				      0x0150, 0x01);
	if (Result != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/* Clear Reset bits (bits 0) */
	Result = XDpRxSs_MCDP6000_SetRegister(DpRxSsPtr, I2CSlaveAddress,
				      0x0150, 0x00);
	if (Result != XST_SUCCESS) {
		return XST_FAILURE;
	}

	return Result;
}
/** @} */
