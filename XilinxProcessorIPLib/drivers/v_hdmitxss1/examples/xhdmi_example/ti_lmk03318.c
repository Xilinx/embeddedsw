/******************************************************************************
* Copyright (C) 2018 – 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file ti_lmk03318.c
* @addtogroup TI_LMK03318
* @{
*
* <pre>
*
* MODIFICATION HISTORY:
*
* Ver   Who    Date     Changes
* ----- ------ -------- --------------------------------------------------
* X.XX  XX     YY/MM/DD
* 1.00  RHe    16/06/20 Initial release.
* 1.01  EB     19/10/23 Updated TI_LMK03318_Init API
* </pre>
*
******************************************************************************/

#include "ti_lmk03318.h"

#if defined (XPS_BOARD_ZCU102) || \
	defined (XPS_BOARD_ZCU104) || \
	defined (XPS_BOARD_ZCU106)
#define I2C_REPEATED_START 0x01
#define I2C_STOP 0x00
#else
#define I2C_REPEATED_START XIIC_REPEATED_START
#define I2C_STOP XIIC_STOP
#endif

static void TI_LMK03318_I2cReset(void *IicPtr);
static unsigned TI_LMK03318_I2cSend(void *IicPtr, u16 SlaveAddr, u8 *MsgPtr,
				unsigned ByteCount, u8 Option);
static unsigned TI_LMK03318_I2cRecv(void *IicPtr, u16 SlaveAddr, u8 *BufPtr,
				unsigned ByteCount, u8 Option);

/*****************************************************************************/
/**
*
* This function resets the IIC instance for TI_LMK03318
*
* @param  IicPtr IIC instance pointer.

*
* @return None.
*
* @note   None.
*
******************************************************************************/
static void TI_LMK03318_I2cReset(void *IicPtr)
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

/*****************************************************************************/
/**
*
* This function send the IIC data to TI_LMK03318
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
static unsigned TI_LMK03318_I2cSend(void *IicPtr, u16 SlaveAddr, u8 *MsgPtr,
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
	usleep(350);
	return XIic_Send(Iic_Ptr->BaseAddress, SlaveAddr, MsgPtr,
					ByteCount, Option);
#endif
}

/*****************************************************************************/
/**
*
* This function send the IIC data to TI_LMK03318
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
static unsigned TI_LMK03318_I2cRecv(void *IicPtr, u16 SlaveAddr, u8 *BufPtr,
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
* This function send a single byte to the TI LMK03318
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
static int TI_LMK03318_SetRegister(void *IicPtr, u8 I2CSlaveAddress,
				u8 RegisterAddress, u8 Value)
{
	u32 ByteCount = 0;
	u8 Buffer[2];

	Buffer[0] = RegisterAddress;
	Buffer[1] = Value;
	ByteCount = TI_LMK03318_I2cSend(IicPtr, I2CSlaveAddress, (u8*)Buffer,
					2, I2C_STOP);
	if (ByteCount != 2) {
		return XST_FAILURE;
	}

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* This function send a single byte to the TI LMK03318
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
static u8 TI_LMK03318_GetRegister(void *IicPtr, u8 I2CSlaveAddress,
				u8 RegisterAddress)
{
	u8 Buffer[2];

	Buffer[0] = RegisterAddress;
	TI_LMK03318_I2cSend(IicPtr, I2CSlaveAddress, (u8*)Buffer,
				1, I2C_REPEATED_START);
	TI_LMK03318_I2cRecv(IicPtr, I2CSlaveAddress,
			(u8*)Buffer, 1, I2C_STOP);
	return Buffer[0];
}

/*****************************************************************************/
/**
*
* This function modifies a single byte to the TI LMK03318
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
static int TI_LMK03318_ModifyRegister(void *IicPtr, u8 I2CSlaveAddress,
				u16 RegisterAddress, u8 Value, u8 Mask)
{
	u8 Data;
	int Result;

	/* Read data */
	Data = TI_LMK03318_GetRegister(IicPtr, I2CSlaveAddress,
				       RegisterAddress);

	/* Clear masked bits */
	Data &= ~Mask;

	/* Update */
	Data |= (Value & Mask);

	/* Write data */
	Result = TI_LMK03318_SetRegister(IicPtr, I2CSlaveAddress,
					 RegisterAddress, Data);

	return Result;
}

int TI_LMK03318_SetClock(void *IicPtr, u8 I2CSlaveAddress,
				int FIn, int FOut, u8 FreeRun)
{
	return XST_FAILURE;
}

/*****************************************************************************/
/**
*
* This function configures the TI LMK03318 to route the specified input port
* to the specified output port bypassing the PLL.
*
* @param I2CBaseAddress is the baseaddress of the I2C core.
* @param I2CSlaveAddress is the 7-bit I2C slave address.
* @param InPortID specifies the input port. Valid values are 0 to 1.
* @param OutPortID specifies the output port. Valid values are 4 to 7.
*
* @return
*    - XST_SUCCESS Initialization was successful.
*    - XST_FAILURE I2C write error or incorrect parameters detected.
*
* @note
*
******************************************************************************/
int TI_LMK03318_EnableBypass(void *IicPtr, u8 I2CSlaveAddress,
			u8 InPortID, u8 OutPortID)
{
	if (InPortID > 1) {
		xil_printf("Invalid input port ID\r\n");
		return XST_FAILURE;
	}

	if (OutPortID < 4 || OutPortID > 7) {
		xil_printf("Invalid output port ID\r\n");
		return XST_FAILURE;
	}

	u32 ByteCount = 0;
	u8 Data = 0;
	u8 Buffer[32];
	u8 RegisterAddress = 37 + (OutPortID - 4) * 2; // OUTCTL_x register address

	Buffer[0] = RegisterAddress;
	ByteCount = TI_LMK03318_I2cSend(IicPtr, I2CSlaveAddress, (u8*)Buffer,
					1, I2C_REPEATED_START);
	if (ByteCount != 1) {
		xil_printf("I2C write error: %d\r\n", ByteCount);
		return XST_FAILURE;
	}
	ByteCount = TI_LMK03318_I2cRecv(IicPtr, I2CSlaveAddress,
					(u8*)Buffer, 1, I2C_STOP);
	if (ByteCount != 1) {
		xil_printf("I2C read error: %d\r\n", ByteCount);
		return XST_FAILURE;
	}
	Data = Buffer[0];
	/* Clear the Clock Source Mux Control field */
	Data &= ~(0x3<<6);

	/* Set the Clock Source Mux Control field */
	Data |= ((InPortID+2)<<6);

	Buffer[0] = RegisterAddress;
	Buffer[1] = Data;
	ByteCount = TI_LMK03318_I2cSend(IicPtr, I2CSlaveAddress, (u8*)Buffer,
					2, I2C_STOP);
	if (ByteCount != 2) {
		xil_printf("I2C write error: %d\r\n", ByteCount);
		return XST_FAILURE;
	}

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* This function initializes the TI LMK03318 with default values
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
int TI_LMK03318_Init(void *IicPtr, u8 I2CSlaveAddress)
{
//	u32 ByteCount = 0;
//	u8 Data = 0;
//	u8 Buffer[32];
	int Result = XST_SUCCESS;

	/* Reset I2C controller before issuing new transaction. This is
	 * required to recover the IIC controller in case a previous transaction
	 * is pending.
	 */
	TI_LMK03318_I2cReset(IicPtr);

	/* Register 29 */
	/* Set input reference clock features */
	Result = TI_LMK03318_SetRegister(IicPtr, I2CSlaveAddress,
					 29, 0x8F);

	/* Register 50 */
	/* Set input buffer */
	Result |= TI_LMK03318_SetRegister(IicPtr, I2CSlaveAddress,
					 50, 0x50);

	/* Register 56 */
	/* PLL_CTRL0 - PLL_PDN = 1 */
	Result |= TI_LMK03318_SetRegister(IicPtr, I2CSlaveAddress,
					 56, 0x01);

	/* Register 30 */
	/* Power down register - Power down channel 0, 1, 2, 3, 7 outputs */
	Result |= TI_LMK03318_SetRegister(IicPtr, I2CSlaveAddress,
					 30, 0x23);

	/* Register 31 START*/
	/* Disables Q0 output */
	Result |= TI_LMK03318_SetRegister(IicPtr, I2CSlaveAddress,
					 31, 0x00);

	/* Register 32 */
	/* Disables Q1 output */
	Result |= TI_LMK03318_SetRegister(IicPtr, I2CSlaveAddress,
					 32, 0x00);

	/* Register 34 */
	/* Disables Q2 output */
	Result |= TI_LMK03318_SetRegister(IicPtr, I2CSlaveAddress,
					 34, 0x00);

	/* Register 35 */
	/* Disables Q3 output */
	Result |= TI_LMK03318_SetRegister(IicPtr, I2CSlaveAddress,
					 35, 0x00);

	/* Register 37 */
	/* Set Q4 output */
	Result |= TI_LMK03318_SetRegister(IicPtr, I2CSlaveAddress,
					 37, 0x92);

	/* Register 39 */
	/* Set Q5 output */
	Result |= TI_LMK03318_SetRegister(IicPtr, I2CSlaveAddress,
					 39, 0xD2);

	/* Register 41 */
	/* Set Q6 output */
	Result |= TI_LMK03318_SetRegister(IicPtr, I2CSlaveAddress,
					 41, 0x92);

	/* Register 43 */
	/* Disables Q7 output */
	Result |= TI_LMK03318_SetRegister(IicPtr, I2CSlaveAddress,
					 43, 0x00);

/*
	// Issue a Software Reset.
	Buffer[0] = 12; // DEV_CTL register address
	Buffer[1] = 0x59; // R12 -> RESETN_SW = 0, the rest is default.
	ByteCount = XIic_Send(I2CBaseAddress, I2CSlaveAddress,
			      (u8*)Buffer, 2, XIIC_STOP);
	if (ByteCount != 2) {
		return XST_FAILURE;
	}

	// Release Software Reset.
	Buffer[0] = 12; // DEV_CTL register address
	Buffer[1] = 0xD9; // R12 -> RESETN_SW = 1, the rest is default.
	ByteCount = XIic_Send(I2CBaseAddress, I2CSlaveAddress,
			      (u8*)Buffer, 2, XIIC_STOP);
	if (ByteCount != 2) {
		return XST_FAILURE;
	}

	// Set the default output modes
	// CK_OUT0/1 are not connected on the VFMC, so disable these outputs.
	// CK_OUT2/3 are configured as AC LVDS.
	// CK_OUT4/5/6/7 are configured as AC LVPECL.
	Buffer[0] = 31; // OUTCTL_0 register address
	Buffer[1] = 0x00; // R31 -> CK_OUT0 disabled
	Buffer[2] = 0x00; // R32 -> CK_OUT1 disabled
	Buffer[3] = 0x01; // R33 -> CK_OUT0/1 DIV Default
	Buffer[4] = 0x20; // R34 -> CK_OUT2 AC-LVDS 4mA
	Buffer[5] = 0x20; // R35 -> CK_OUT3 AC-LVDS 4mA
	Buffer[6] = 0x03; // R36 -> CK_OUT2/3 DIV Default
	Buffer[7] = 0x18; // R37 -> CK_OUT4 AC-LVPECL 8mA, source is PLL
	Buffer[8] = 0x02; // R38 -> CK_OUT4 DIV Default
	Buffer[9] = 0x18; // R39 -> CK_OUT5 AC-LVPECL 8mA, source is PLL
	Buffer[10] = 0x02; // R40 -> CK_OUT5 DIV Default
	Buffer[11] = 0x18; // R41 -> CK_OUT6 AC-LVPECL 8mA, source is PLL
	Buffer[12] = 0x05; // R42 -> CK_OUT6 DIV Default
	Buffer[13] = 0x18; // R43 -> CK_OUT7 AC-LVPECL 8mA, source is PLL
	Buffer[14] = 0x05; // R44 -> CK_OUT7 DIV Default
	ByteCount = XIic_Send(I2CBaseAddress, I2CSlaveAddress,
			      (u8*)Buffer, 15, XIIC_STOP);
	if (ByteCount != 15) {
		return XST_FAILURE;
	}

	// Set the default input modes
	Buffer[0] = 29; // OSCCTL1 register address
	Buffer[1] = 0x0f; // R29 -> PRI/SEC internal 100-Ohm termination, PRI/SEC external AC coupling
	ByteCount = XIic_Send(I2CBaseAddress, I2CSlaveAddress,
			      (u8*)Buffer, 2, XIIC_STOP);
	if (ByteCount != 2) {
		return XST_FAILURE;
	}

	// PRIREF and SECREF are configured as differential input.
	Buffer[0] = 50; // IPCLKSEL register address
	Buffer[1] = 0x50; // R50 -> PRIREF/SECREF Dif input, input mode -> automatic
	ByteCount = XIic_Send(I2CBaseAddress, I2CSlaveAddress,
			      (u8*)Buffer, 2, XIIC_STOP);
	if (ByteCount != 2) {
		return XST_FAILURE;
	}

	// Disable the Secondary Reference Doubler
	Buffer[0] = 72; // SEC_CTRL register address
	Buffer[1] = 0x08; // R72 -> Secondary Reference Doubler disabled.
	ByteCount = XIic_Send(I2CBaseAddress, I2CSlaveAddress,
			      (u8*)Buffer, 2, XIIC_STOP);
	if (ByteCount != 2) {
		return XST_FAILURE;
	}
*/
	if (Result != XST_SUCCESS) {
		return XST_FAILURE;
	}

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* This function puts the TI LMK03318 into sleep
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
int TI_LMK03318_PowerDown(void *IicPtr, u8 I2CSlaveAddress)
{

	/* Register 29 */
	TI_LMK03318_SetRegister(IicPtr, I2CSlaveAddress, 29, 0x03);

	/* Register 30 */
	TI_LMK03318_SetRegister(IicPtr, I2CSlaveAddress, 30, 0x3f);

	/* Register 31 */
	TI_LMK03318_SetRegister(IicPtr, I2CSlaveAddress, 31, 0x00);

	/* Register 32 */
	TI_LMK03318_SetRegister(IicPtr, I2CSlaveAddress, 32, 0x00);

	/* Register 34 */
	TI_LMK03318_SetRegister(IicPtr, I2CSlaveAddress, 34, 0x00);

	/* Register 35 */
	TI_LMK03318_SetRegister(IicPtr, I2CSlaveAddress, 35, 0x00);

	/* Register 37 */
	TI_LMK03318_SetRegister(IicPtr, I2CSlaveAddress, 37, 0x00);

	/* Register 39 */
	TI_LMK03318_SetRegister(IicPtr, I2CSlaveAddress, 39, 0x00);

	/* Register 41 */
	TI_LMK03318_SetRegister(IicPtr, I2CSlaveAddress, 41, 0x00);

	/* Register 43 */
	TI_LMK03318_SetRegister(IicPtr, I2CSlaveAddress, 43, 0x00);

	/* Register 50 */
	TI_LMK03318_SetRegister(IicPtr, I2CSlaveAddress, 50, 0xf6);

	/* Register 56 */
	TI_LMK03318_SetRegister(IicPtr, I2CSlaveAddress, 56, 0x01);

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* This function displays a registerdump of the TI LMK03318 device.
*
* @param I2CBaseAddress is the baseaddress of the I2C core.
* @param I2CSlaveAddress is the 7-bit I2C slave address.
*
* @return None
*
* @note None.
*
******************************************************************************/
void TI_LMK03318_RegisterDump(void *IicPtr, u8 I2CSlaveAddress)
{
	u32 ByteCount = 0;
	u8 Buffer[256];
	int i;

	xil_printf("\r\n");
	xil_printf("-----------------------\r\n");
	xil_printf("- TI LMK03381 I2C dump:\r\n");
	xil_printf("-----------------------\r\n");
	Buffer[0] = 0;
	ByteCount = TI_LMK03318_I2cSend(IicPtr, I2CSlaveAddress, (u8*)Buffer,
					1, I2C_REPEATED_START);
	if (ByteCount != 1) {
		xil_printf("I2C write error: %d\r\n", ByteCount);
	}
	ByteCount = TI_LMK03318_I2cRecv(IicPtr, I2CSlaveAddress,
					(u8*)Buffer, 145, I2C_STOP);
	if (ByteCount != 145) {
		xil_printf("I2C read error: %d\r\n", ByteCount);
	}

	xil_printf("      ");
	for (i = 0 ; i < 10 ; i++)
		xil_printf("+%01d ", i);

	xil_printf("\r\n      ");
	for (i = 0 ; i < 10 ; i++)
		xil_printf("---");

	for (i = 0 ; i < ByteCount ; i++) {
		if ((i % 10) == 0) {
			xil_printf("\r\n%02d : ", i);
		}
		xil_printf("%02x ", Buffer[i]);
	}

	xil_printf("\r\n");
}
