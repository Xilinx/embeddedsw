/******************************************************************************
*
* Copyright (C) 2018 – 2019 Xilinx, Inc.  All rights reserved.
* 
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
* 
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
* 
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
* THE AUTHORS OR COPYRIGHT HOLDERS  BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
* FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
* IN THE SOFTWARE.
*
******************************************************************************/
/*****************************************************************************/
/**
*
* @file ONSEMI_NB7NQ621M.c
* @addtogroup ONSEMI_NB7NQ621M
* @{
*
* <pre>
*
* MODIFICATION HISTORY:
*
* Ver   Who    Date     Changes
* ----- ------ -------- --------------------------------------------------
* X.XX  XX     YY/MM/DD
* 1.00  GM     19/05/14 Initial release.
* </pre>
*
******************************************************************************/

#include "onsemi_nb7nq621m.h"
#include "sleep.h"
#if defined (XPS_BOARD_ZCU102) || \
	defined (XPS_BOARD_ZCU104) || \
	defined (XPS_BOARD_ZCU106)
#include "xiicps.h"
#else
#include "xiic.h"
#endif

#if defined (XPS_BOARD_ZCU102) || \
	defined (XPS_BOARD_ZCU104) || \
	defined (XPS_BOARD_ZCU106)
#define I2C_REPEATED_START 0x01
#define I2C_STOP 0x00
#else
#define I2C_REPEATED_START XIIC_REPEATED_START
#define I2C_STOP XIIC_STOP
#endif

#if 0
static void ONSEMI_NB7NQ621M_I2cReset(void *IicPtr);
#endif
static unsigned ONSEMI_NB7NQ621M_I2cSend(void *IicPtr, u16 SlaveAddr, u8 *MsgPtr,
							unsigned ByteCount, u8 Option);
static unsigned ONSEMI_NB7NQ621M_I2cRecv(void *IicPtr, u16 SlaveAddr, u8 *BufPtr,
							unsigned ByteCount, u8 Option);
static u8 ONSEMI_NB7NQ621M_GetRegister(void *IicPtr, u8 I2CSlaveAddress,
			u8 RegisterAddress);
static int ONSEMI_NB7NQ621M_SetRegister(void *IicPtr, u8 I2CSlaveAddress,
				u8 RegisterAddress, u8 Value);
#if 0
static int ONSEMI_NB7NQ621M_ModifyRegister(void *IicPtr, u8 I2CSlaveAddress,
							u16 RegisterAddress, u8 Value, u8 Mask);
#endif

#if 0
/*****************************************************************************/
/**
*
* This function resets the IIC instance for ONSEMI_NB7NQ621M
*
* @param  IicPtr IIC instance pointer.

*
* @return None.
*
* @note   None.
*
******************************************************************************/
static void ONSEMI_NB7NQ621M_I2cReset(void *IicPtr)
{
#if defined (XPS_BOARD_ZCU102) || \
	defined (XPS_BOARD_ZCU104) || \
	defined (XPS_BOARD_ZCU106)
	/* Do nothing
	XIicPs *Iic_Ptr = IicPtr;
	XIicPs_Reset(Iic_Ptr);*/
#else
	XIic *Iic_Ptr = IicPtr;
	XIic_WriteReg(Iic_Ptr->BaseAddress, XIIC_RESETR_OFFSET,
				  XIIC_RESET_MASK);
#endif
}
#endif

/*****************************************************************************/
/**
*
* This function send the IIC data to ONSEMI_NB7NQ621M
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
static unsigned ONSEMI_NB7NQ621M_I2cSend(void *IicPtr, u16 SlaveAddr, u8 *MsgPtr,
							unsigned ByteCount, u8 Option)
{
#if defined (XPS_BOARD_ZCU102) || \
	defined (XPS_BOARD_ZCU104) || \
	defined (XPS_BOARD_ZCU106)
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

	if (Status == XST_SUCCESS) {
		return ByteCount;
	} else {
		return 0;
	}
#else
	XIic *Iic_Ptr = IicPtr;
	/* This delay prevents IIC access from hanging */
	usleep(1000);
	return XIic_Send(Iic_Ptr->BaseAddress, SlaveAddr, MsgPtr,
					ByteCount, Option);
#endif
}

/*****************************************************************************/
/**
*
* This function send the IIC data to ONSEMI_NB7NQ621M
*
* @param  IicPtr IIC instance pointer.
* @param  SlaveAddr contains the 7 bit IIC address of the device to send the
*		   specified data to.
* @param BufPtr points to the memory to write the data.
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
static unsigned ONSEMI_NB7NQ621M_I2cRecv(void *IicPtr, u16 SlaveAddr, u8 *BufPtr,
							unsigned ByteCount, u8 Option)
{
#if defined (XPS_BOARD_ZCU102) || \
	defined (XPS_BOARD_ZCU104) || \
	defined (XPS_BOARD_ZCU106)
	XIicPs *Iic_Ptr = IicPtr;
	u32 Status;

	XIicPs_SetOptions(Iic_Ptr, XIICPS_7_BIT_ADDR_OPTION);
	XIicPs_ClearOptions(Iic_Ptr, XIICPS_10_BIT_ADDR_OPTION);
	if (Option == I2C_REPEATED_START) {
		XIicPs_SetOptions(Iic_Ptr, XIICPS_REP_START_OPTION);
	} else {
		XIicPs_ClearOptions(Iic_Ptr, XIICPS_REP_START_OPTION);
	}

	Status = XIicPs_MasterRecvPolled(Iic_Ptr, BufPtr, ByteCount, SlaveAddr);

	/*
	 * Wait until bus is idle to start another transfer.
	 */
	if (!(Iic_Ptr->IsRepeatedStart)) {
		while (XIicPs_BusIsBusy(Iic_Ptr));
	}

	if (Status == XST_SUCCESS) {
		return ByteCount;
	} else {
		return 0;
	}
#else
	XIic *Iic_Ptr = IicPtr;
	return XIic_Recv(Iic_Ptr->BaseAddress, SlaveAddr, BufPtr,
					ByteCount, Option);
#endif
}

/*****************************************************************************/
/**
*
* This function send a single byte to the ONSEMI NB7NQ621M
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
static int ONSEMI_NB7NQ621M_SetRegister(void *IicPtr, u8 I2CSlaveAddress,
				u8 RegisterAddress, u8 Value)
{
	u32 ByteCount = 0;
	u8 Buffer[2];

	Buffer[0] = RegisterAddress;
	Buffer[1] = Value;
	ByteCount = ONSEMI_NB7NQ621M_I2cSend(IicPtr, I2CSlaveAddress, (u8*)Buffer,
								2, I2C_STOP);
	if (ByteCount != 2) {
		return XST_FAILURE;
	}

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* This function send a single byte to the ONSEMI NB7NQ621M
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
static u8 ONSEMI_NB7NQ621M_GetRegister(void *IicPtr, u8 I2CSlaveAddress,
			u8 RegisterAddress)
{
	u8 Buffer[2];

	Buffer[0] = RegisterAddress;
	ONSEMI_NB7NQ621M_I2cSend(IicPtr, I2CSlaveAddress, (u8*)Buffer,
								1, I2C_REPEATED_START);
	ONSEMI_NB7NQ621M_I2cRecv(IicPtr, I2CSlaveAddress,
					(u8*)Buffer, 1, I2C_STOP);
	return Buffer[0];
}

#if 0
/*****************************************************************************/
/**
*
* This function modifies a single byte to the ONSEMI NB7NQ621M
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
static int ONSEMI_NB7NQ621M_ModifyRegister(void *IicPtr, u8 I2CSlaveAddress,
				u16 RegisterAddress, u8 Value, u8 Mask)
{
	u8 Data;
	int Result;

	/* Read data */
	Data = ONSEMI_NB7NQ621M_GetRegister(IicPtr, I2CSlaveAddress,
				       RegisterAddress);

	/* Clear masked bits */
	Data &= ~Mask;

	/* Update */
	Data |= (Value & Mask);

	/* Write data */
	Result = ONSEMI_NB7NQ621M_SetRegister(IicPtr, I2CSlaveAddress,
					 RegisterAddress, Data);

	return Result;
}
#endif

/*****************************************************************************/
/**
*
* This function initializes the ONSEMI NB7NQ621M with default values
* for use with the Video FMC.
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
int ONSEMI_NB7NQ621M_Init(void *IicPtr, u8 I2CSlaveAddress, u8 IsTx)
{
	int Result = XST_SUCCESS;
	u8 Data;

	/* Register 04 */
	/* Set Functional Control 0 to Global */

	/* Step 1 */
	Result = ONSEMI_NB7NQ621M_SetRegister(IicPtr, I2CSlaveAddress,
					 0x04, 0x00);

	/* Lane swap - Enabled */
	Data = 	(REG04_BIT7_LANE_CTRL_GLOBAL << 7) |
			(REG04_BIT6_LANE_SWAP_DISABLE << 6 ) |
			(IsTx ? (REG04_BIT53_MODE_TMDS_HIZ << 3) :
					(REG04_BIT53_MODE_FRL_AC  << 3));

	Result = ONSEMI_NB7NQ621M_SetRegister(IicPtr, I2CSlaveAddress,
					 0x04, Data);

	/* Register 05 */
	/* Disable AUX Monitor - Due to ONSEMI BUG */
	/* TX - Disable Sig Detect*/
	/* RX - Disable HPD Auto Power Down */
	Result = ONSEMI_NB7NQ621M_SetRegister(IicPtr, I2CSlaveAddress,
					 0x05, (IsTx ? 0x0B : 0x0D));

	/* Register 06 */
	/* Termination Controls */
	Result = ONSEMI_NB7NQ621M_SetRegister(IicPtr, I2CSlaveAddress,
					 0x06, 0x00);

	/* Register 07 */
	/* CLK Control 0 */
	Result = ONSEMI_NB7NQ621M_SetRegister(IicPtr, I2CSlaveAddress,
					 0x07, (IsTx ? 0x00 : 0x32));

	/* Register 08 */
	/* CLK Control 1 */
	Result = ONSEMI_NB7NQ621M_SetRegister(IicPtr, I2CSlaveAddress,
					 0x08, (IsTx ? 0x03 : 0x0B));

	/* Register 09 */
	/* Data Control 0 */
	Result = ONSEMI_NB7NQ621M_SetRegister(IicPtr, I2CSlaveAddress,
#if defined (XPS_BOARD_ZCU106)
					 0x09, (IsTx ? 0x00 : 0x32));
#elif defined (XPS_BOARD_VCU118)
					 0x09, (IsTx ? 0x00 : 0x30));
#else
/* Place holder for future board support, Below Value just a random value */
					 0x09, (IsTx ? 0x00 : 0x32));
#endif

	/* Register 0A */
	/* Data Control 1 */
	Result = ONSEMI_NB7NQ621M_SetRegister(IicPtr, I2CSlaveAddress,
					 0x0A, (IsTx ? 0x03 : 0x0B));

	/* Register 0B */
	/* Channel Enable/Disable */
	Result = ONSEMI_NB7NQ621M_SetRegister(IicPtr, I2CSlaveAddress,
					 0x0B, 0x0F);

	if (Result != XST_SUCCESS) {
		return XST_FAILURE;
	}

	return Result;
}

/*****************************************************************************/
/**
*
* This function checks the ONSEMI NB7NQ621M device ID
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
int ONSEMI_NB7NQ621M_CheckDeviceID(void *IicPtr, u8 I2CSlaveAddress)
{
	u16 DeviceId;
	u8 Data;

	Data = ONSEMI_NB7NQ621M_GetRegister(IicPtr, I2CSlaveAddress, 0x0001);

	/* Copy */
	DeviceId = Data;

	/* Shift */
	DeviceId <<= 8;

	Data = ONSEMI_NB7NQ621M_GetRegister(IicPtr, I2CSlaveAddress, 0x0000);

	/* Copy */
	DeviceId |= Data;

	/* Check */
	if (DeviceId == 0x4E4F)
		return XST_SUCCESS;
	else
		return XST_FAILURE;
}

/*****************************************************************************/
/**
*
* This function reconfigures the ONSEMI NB7NQ621M cable redriver based on the
* HDMI mode and line rate.
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
int ONSEMI_NB7NQ621M_LineRateReconfig(void *IicPtr, u8 I2CSlaveAddress,
		u8 IsFRL, u64 LineRate)
{
	int Result = XST_SUCCESS;
	u32 LineRateMbps;

	LineRateMbps = (u32)((u64) LineRate / 1000000);

	if (IsFRL) { /* FRL */
		/* Mode Control: 100 Ohm */
		Result |= ONSEMI_NB7NQ621M_SetRegister(IicPtr, I2CSlaveAddress,
						 0x04, 0x18);
		/* Data Controls 0: Flat Gain = 1.5 dB & Compression=1000mV */
		Result |= ONSEMI_NB7NQ621M_SetRegister(IicPtr, I2CSlaveAddress,
						 0x09, 0x20);
		/* Data Controls 1: Slew Rate = 30 ps & Equalization = 5 */
		Result |= ONSEMI_NB7NQ621M_SetRegister(IicPtr, I2CSlaveAddress,
						 0x0A, 0x05);
		/* Ch A Controls 0: Flat Gain = 0 dB & Compression=1000mV */
		Result |= ONSEMI_NB7NQ621M_SetRegister(IicPtr, I2CSlaveAddress,
						 0x0D, 0x00);
		/* Ch A Controls 1: Slew Rate = 30 ps & Equalization = 3 */
		Result |= ONSEMI_NB7NQ621M_SetRegister(IicPtr, I2CSlaveAddress,
						 0x0E, 0x03);
		/* Ch B Controls 0: Flat Gain = 0 dB & Compression=1000mV */
		Result |= ONSEMI_NB7NQ621M_SetRegister(IicPtr, I2CSlaveAddress,
						 0x10, 0x00);
		/* Ch B Controls 1: Slew Rate = 30 ps & Equalization = 3 */
		Result |= ONSEMI_NB7NQ621M_SetRegister(IicPtr, I2CSlaveAddress,
						 0x11, 0x03);
		/* Ch C Controls 0: Flat Gain = 0 dB & Compression=1000mV */
		Result |= ONSEMI_NB7NQ621M_SetRegister(IicPtr, I2CSlaveAddress,
						 0x13, 0x00);
		/* Ch C Controls 1: Slew Rate = 30 ps & Equalization = 3 */
		Result |= ONSEMI_NB7NQ621M_SetRegister(IicPtr, I2CSlaveAddress,
						 0x14, 0x03);
		/* Ch D Controls 0: Flat Gain = 0 dB & Compression=1000mV */
		Result |= ONSEMI_NB7NQ621M_SetRegister(IicPtr, I2CSlaveAddress,
						 0x16, 0x00);
		/* Ch D Controls 1: Slew Rate = 240 ps & Equalization = 3 */
		Result |= ONSEMI_NB7NQ621M_SetRegister(IicPtr, I2CSlaveAddress,
						 0x17, 0x03);
		/* Channel D Termination to 100 Ohms */
		Result |= ONSEMI_NB7NQ621M_SetRegister(IicPtr, I2CSlaveAddress,
						 0x18, 0x00);
	} else { /* TMDS / DVI */

		/* HDMI 2.0 */
		if ((LineRateMbps >= 3400) && (LineRateMbps < 6000)) {

				/*FnCtrlData |= REG04_BIT53_MODE_TMDS_100 << 3;*/
				/*Functional Controls 0: Lane Control = Individual */
				/*                       Mode Control = FRL DC coupled */
				Result |= ONSEMI_NB7NQ621M_SetRegister(IicPtr, I2CSlaveAddress,
								 0x04, 0xA0);
				/* Data Controls 0: Flat Gain = 0 dB & Compression=1000mV */
				Result |= ONSEMI_NB7NQ621M_SetRegister(IicPtr, I2CSlaveAddress,
								 0x09, 0x00);
				/* Data Controls 1: Slew Rate = 30 ps & Equalization = 3 */
				Result |= ONSEMI_NB7NQ621M_SetRegister(IicPtr, I2CSlaveAddress,
								 0x0A, 0x03);
				/* Ch A Controls 0: Flat Gain = 4.5 dB & Compression=1200mV */
				Result |= ONSEMI_NB7NQ621M_SetRegister(IicPtr, I2CSlaveAddress,
								 0x0D, 0x31);
				/* Ch A Controls 1: Slew Rate = 30 ps & Equalization = 15 */
				Result |= ONSEMI_NB7NQ621M_SetRegister(IicPtr, I2CSlaveAddress,
								 0x0E, 0x0F);
				/* Ch B Controls 0: Flat Gain = 4.5 dB & Compression=1200mV */
				Result |= ONSEMI_NB7NQ621M_SetRegister(IicPtr, I2CSlaveAddress,
								 0x10, 0x31);
				/* Ch B Controls 1: Slew Rate = 30 ps & Equalization = 15 */
				Result |= ONSEMI_NB7NQ621M_SetRegister(IicPtr, I2CSlaveAddress,
								 0x11, 0x0F);
				/* Ch C Controls 0: Flat Gain = 4.5 dB & Compression=1200mV */
				Result |= ONSEMI_NB7NQ621M_SetRegister(IicPtr, I2CSlaveAddress,
								 0x13, 0x31);
				/* Ch C Controls 1: Slew Rate = 30 ps & Equalization = 15 */
				Result |= ONSEMI_NB7NQ621M_SetRegister(IicPtr, I2CSlaveAddress,
								 0x14, 0x0F);
				/* Ch D Controls 0: Flat Gain = 0 dB & Compression=800mV */
				Result |= ONSEMI_NB7NQ621M_SetRegister(IicPtr, I2CSlaveAddress,
								 0x16, 0x02);
				/* Ch D Controls 1: Slew Rate = 240 ps & Equalization = 3 */
				Result |= ONSEMI_NB7NQ621M_SetRegister(IicPtr, I2CSlaveAddress,
								 0x17, 0x63);
				/* Channel D Termination to 100 Ohms */
				Result |= ONSEMI_NB7NQ621M_SetRegister(IicPtr, I2CSlaveAddress,
								 0x18, 0x0B);
		}
		/* HDMI 1.4 1.65-3.4 Gbps */
		else if ((LineRateMbps >= 1650) && (LineRateMbps < 3400)) {

			/*FnCtrlData |= REG04_BIT53_MODE_TMDS_200 << 3;*/
			/*Functional Controls 0: Lane Control = Individual */
			/*                       Mode Control = FRL AC coupled */
			Result |= ONSEMI_NB7NQ621M_SetRegister(IicPtr, I2CSlaveAddress,
							 0x04, 0xA0);
			/* Data Controls 0: Flat Gain = 0 dB & Compression=1000mV */
			Result |= ONSEMI_NB7NQ621M_SetRegister(IicPtr, I2CSlaveAddress,
							 0x09, 0x00);
			/* Data Controls 1: Slew Rate = 30 ps & Equalization = 3 */
			Result |= ONSEMI_NB7NQ621M_SetRegister(IicPtr, I2CSlaveAddress,
							 0x0A, 0x03);
			/* Ch A Controls 0: Flat Gain = 0 dB & Compression=800mV */
			Result |= ONSEMI_NB7NQ621M_SetRegister(IicPtr, I2CSlaveAddress,
							 0x0D, 0x30);
			/* Ch A Controls 1: Slew Rate = 30 ps & Equalization = 15 */
			Result |= ONSEMI_NB7NQ621M_SetRegister(IicPtr, I2CSlaveAddress,
							 0x0E, 0x0F);
			/* Ch B Controls 0: Flat Gain = 0 dB & Compression=800mV */
			Result |= ONSEMI_NB7NQ621M_SetRegister(IicPtr, I2CSlaveAddress,
							 0x10, 0x30);
			/* Ch B Controls 1: Slew Rate = 30 ps & Equalization = 15 */
			Result |= ONSEMI_NB7NQ621M_SetRegister(IicPtr, I2CSlaveAddress,
							 0x11, 0x0F);
			/* Ch C Controls 0: Flat Gain = 0 dB & Compression=800mV */
			Result |= ONSEMI_NB7NQ621M_SetRegister(IicPtr, I2CSlaveAddress,
							 0x13, 0x30);
			/* Ch C Controls 1: Slew Rate = 30 ps & Equalization = 15 */
			Result |= ONSEMI_NB7NQ621M_SetRegister(IicPtr, I2CSlaveAddress,
							 0x14, 0x0F);
			/* Ch D Controls 0: Flat Gain = 0 dB & Compression=800mV */
			Result |= ONSEMI_NB7NQ621M_SetRegister(IicPtr, I2CSlaveAddress,
							 0x16, 0x02);
			/* Ch D Controls 1: Slew Rate = 240 ps & Equalization = 3 */
			Result |= ONSEMI_NB7NQ621M_SetRegister(IicPtr, I2CSlaveAddress,
							 0x17, 0x63);
			/* Channel D Termination to 100 Ohms */
			Result |= ONSEMI_NB7NQ621M_SetRegister(IicPtr, I2CSlaveAddress,
							 0x18, 0x0B);
		}
		/* HDMI 1.4 0.25-1.65 Gbps */
		else {

			/*FnCtrlData |= REG04_BIT53_MODE_TMDS_HIZ << 3;*/
			/*Functional Controls 0: Lane Control = Individual */
			/*                       Mode Control = FRL AC coupled */
			Result |= ONSEMI_NB7NQ621M_SetRegister(IicPtr, I2CSlaveAddress,
							 0x04, 0xB0);
			/* Data Controls 0: Flat Gain = 0 dB & Compression=1000mV */
			Result |= ONSEMI_NB7NQ621M_SetRegister(IicPtr, I2CSlaveAddress,
							 0x09, 0x00);
			/* Data Controls 1: Slew Rate = 30 ps & Equalization = 3 */
			Result |= ONSEMI_NB7NQ621M_SetRegister(IicPtr, I2CSlaveAddress,
							 0x0A, 0x03);
			/* Ch A Controls 0: Flat Gain = 0 dB & Compression=800mV */
			Result |= ONSEMI_NB7NQ621M_SetRegister(IicPtr, I2CSlaveAddress,
							 0x0D, 0x02);
			/* Ch A Controls 1: Slew Rate = 30 ps & Equalization = 15 */
			Result |= ONSEMI_NB7NQ621M_SetRegister(IicPtr, I2CSlaveAddress,
							 0x0E, 0x0F);
			/* Ch B Controls 0: Flat Gain = 0 dB & Compression=800mV */
			Result |= ONSEMI_NB7NQ621M_SetRegister(IicPtr, I2CSlaveAddress,
							 0x10, 0x02);
			/* Ch B Controls 1: Slew Rate = 30 ps & Equalization = 15 */
			Result |= ONSEMI_NB7NQ621M_SetRegister(IicPtr, I2CSlaveAddress,
							 0x11, 0x0F);
			/* Ch C Controls 0: Flat Gain = 0 dB & Compression=800mV */
			Result |= ONSEMI_NB7NQ621M_SetRegister(IicPtr, I2CSlaveAddress,
							 0x13, 0x02);
			/* Ch C Controls 1: Slew Rate = 30 ps & Equalization = 15 */
			Result |= ONSEMI_NB7NQ621M_SetRegister(IicPtr, I2CSlaveAddress,
							 0x14, 0x0F);
			/* Ch D Controls 0: Flat Gain = 0 dB & Compression=800mV */
			Result |= ONSEMI_NB7NQ621M_SetRegister(IicPtr, I2CSlaveAddress,
							 0x16, 0x02);
			/* Ch D Controls 1: Slew Rate = 240 ps & Equalization = 3 */
			Result |= ONSEMI_NB7NQ621M_SetRegister(IicPtr, I2CSlaveAddress,
							 0x17, 0x63);
			/* Channel D Termination to 100 Ohms */
			Result |= ONSEMI_NB7NQ621M_SetRegister(IicPtr, I2CSlaveAddress,
							 0x18, 0x0B);
		}

	}


	if (Result != XST_SUCCESS) {
		return XST_FAILURE;
	}

	return Result;
}

/*****************************************************************************/
/**
*
* This function displays a registerdump of the ONSEMI NB7NQ621M device.
*
* @param I2CBaseAddress is the baseaddress of the I2C core.
* @param I2CSlaveAddress is the 7-bit I2C slave address.
*
* @return None
*
* @note None.
*
******************************************************************************/
void ONSEMI_NB7NQ621M_RegisterDump(void *IicPtr, u8 I2CSlaveAddress)
{
	u8 Data;
	u32 i;
	int Result;

	xil_printf("-----------------------------\r\n");
	xil_printf("- ONSEMI_NB7NQ621M I2C dump:\r\n");
	xil_printf("-----------------------------\r\n");

	Result = ONSEMI_NB7NQ621M_CheckDeviceID(IicPtr, I2CSlaveAddress);

	if (Result == XST_SUCCESS) {
		xil_printf("     ");
		for (i=0; i<8; i++)
			xil_printf("+%01x ", i);

		xil_printf("\r\n     ");
		for (i=0; i<8; i++)
			xil_printf("---");

		for (i=0; i<25; i++) {
			if ((i % 8) == 0) {
				xil_printf("\r\n%02x : ", i);
			}
			Data = ONSEMI_NB7NQ621M_GetRegister(IicPtr,
							I2CSlaveAddress, i);
			xil_printf("%02x ", Data);
		}

		xil_printf("\r\n");
	}

	else {
		xil_printf("ONSEMI NB7NQ621M not found!\r\n");
	}
}
