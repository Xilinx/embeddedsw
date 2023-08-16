/******************************************************************************
* Copyright (C) 2018 – 2022 Xilinx, Inc.  All rights reserved.
* Copyright 2022-2023 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
*
* @file si5324drv.c
*
* This file contains low-level driver functions for controlling the
* SiliconLabs Si5324 clock generator as mounted on the KC705 demo board.
* The user should refer to the hardware device specification for more details
* of the device operation.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who Date         Changes
* ----- --- ----------   -----------------------------------------------
*           dd/mm/yyyy
* ----- --- ----------   -----------------------------------------------
* 1.00  gm  12/05/2018   Initial release
*
* </pre>
*
****************************************************************************/

#include "xil_types.h"
#include "xgpio.h"
#include "video_fmc.h"
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
#if defined (XPS_BOARD_VEK280)
#else
#define XPS_BOARD_VCU118
#endif
#endif

#define VFMC_I2C_IDT8N49_ADDR   0x7C /**< I2C IDT 8N49N241 Address */
#define VFMC_I2C_IOEXP_1_ADDR 	0x65 /**< I2C IO Expander 1 address */
#define VFMC_I2C_IOEXP_0_ADDR 	0x64 /**< I2C IO Expander 0 address */
#define VFMC_I2C_LMK03318_ADDR  0x51 /**< I2C TI LMK03318 Address */
#define VFMC_I2C_DP159_ES_ADDR 	0x5D /**< I2C DP159 Address */

XGpio              Gpio_Vfmc;
XGpio_Config       *Gpio_Vfmc_ConfigPtr;


/*****************************************************************************/
/**
*
* This function send the IIC data to Vfmc
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
static unsigned Vfmc_I2cSend(void *IicPtr, u16 SlaveAddr, u8 *MsgPtr,
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
	usleep(1000);
	return XIic_Send(Iic_Ptr->BaseAddress, SlaveAddr, MsgPtr,
					ByteCount, Option);
#endif
}

/*****************************************************************************/
/**
*
* This function send the IIC data to Vfmc
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
static unsigned Vfmc_I2cRecv(void *IicPtr, u16 SlaveAddr, u8 *BufPtr,
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
* This function setup the IIC MUX to select the VFMC on the HPC header
*
* @param  None.
*
* @return The number of bytes sent.
*
* @note   None.
*
******************************************************************************/
int Vfmc_I2cMuxSelect(void *IicPtr)
{
	u8 Buffer;
	int Status;

#if defined (XPS_BOARD_VCU118)
	Buffer = 0x02;
	Status = Vfmc_I2cSend(IicPtr, 0x75,
					   (u8 *)&Buffer, 1, (I2C_STOP));

	/* Set TCA9548 MUX1 to select port 1 (No connection)*/
	Buffer = 0x02;
	Status = Vfmc_I2cSend(IicPtr, 0x74,
					   (u8 *)&Buffer, 1, (I2C_STOP));


#elif defined (XPS_BOARD_ZCU102)
	/* Set TCA9548 U34 to select port 7 (No connection)*/
	Buffer = 0x80;
	Status = Vfmc_I2cSend(IicPtr, 0x74,
					   (u8 *)&Buffer, 1, (I2C_STOP));

	/* Set TCA9548 U135 to select port 0 (HPC0)*/
	Buffer = 0x01;
	Status = Vfmc_I2cSend(IicPtr, 0x75,
					   (u8 *)&Buffer, 1, (I2C_STOP));

#elif defined (XPS_BOARD_ZCU106)
	/* Set TCA9548 U34 to select port 7 (No connection)*/
	Buffer = 0x80;
	Status = Vfmc_I2cSend(IicPtr, 0x74,
					   (u8 *)&Buffer, 1, (I2C_STOP));

	/* Set TCA9548 U135 to select port 0 (HPC0)*/
	Buffer = 0x01;
	Status = Vfmc_I2cSend(IicPtr, 0x75,
					   (u8 *)&Buffer, 1, (I2C_STOP));

#endif

	/* When a device is found, it returns one byte */
	if (Status)
	  return XST_SUCCESS;
	else
	  return XST_FAILURE;
}

/*****************************************************************************/
/**
*
* This function initializes the Video FMC clock generator and clock MUXs
* for HDMI operation
*
* @param  None.
*
* @return None.
*
* @note   None.
*
******************************************************************************/
int Vfmc_HdmiInit(u16 GpioDeviceId, void *IicPtr)
{
	int Status;
	u8 Buffer[2];
	int ByteCount;
#if defined (XPS_BOARD_ZCU102) || \
	defined (XPS_BOARD_ZCU104) || \
	defined (XPS_BOARD_ZCU106)
	XIicPs *Iic_Ptr = IicPtr;
#else
	XIic *Iic_Ptr = IicPtr;
#endif

	/* Initialize GPIO for VFMC */
	Gpio_Vfmc_ConfigPtr =
		XGpio_LookupConfig(GpioDeviceId);

	if(Gpio_Vfmc_ConfigPtr == NULL) {
		Gpio_Vfmc.IsReady = 0;
		return (XST_DEVICE_NOT_FOUND);
	}

	Status = XGpio_CfgInitialize(&Gpio_Vfmc,
								 Gpio_Vfmc_ConfigPtr,
								 Gpio_Vfmc_ConfigPtr->BaseAddress);
	if(Status != XST_SUCCESS) {
		xil_printf("ERR:: GPIO for VFMC ");
		xil_printf("Initialization failed %d\r\n", Status);
		return(XST_FAILURE);
	}

	Vfmc_I2cMuxSelect(IicPtr);

	/* Configure VFMC IO Expander 0:
	 * Disable Si5344
	 * Set primary clock source for LMK03318 to IOCLKp(0)
	 * Set secondary clock source for LMK03318 to IOCLKp(1)
	 * Disable LMK61E2*/
	Buffer[0] = 0x52;
	ByteCount = Vfmc_I2cSend(IicPtr, VFMC_I2C_IOEXP_0_ADDR,
			(u8*)Buffer, 1, I2C_STOP);
	if (ByteCount != 1) {
		xil_printf("Failed to set the I2C IO Expander.\r\n");
		return XST_FAILURE;
	}

	/* Configure VFMC IO Expander 1:
	 * Enable LMK03318 -> In a power-down state the I2C bus becomes unusable.
	 * Select LMK03318 clock as source for FMC_GT_CLKp(0)
	 * Select IDT8T49N241 clock as source for FMC_GT_CLKp(1)
	 * Enable IDT8T49N241 */
	Buffer[0] = 0x1A;
	ByteCount = Vfmc_I2cSend(IicPtr, VFMC_I2C_IOEXP_1_ADDR,
			(u8*)Buffer, 1, I2C_STOP);
	if (ByteCount != 1) {
		xil_printf("Failed to set the I2C IO Expander.\r\n");
		return XST_FAILURE;
	}

	/* Configure HDMI2.0 Mezz in Slot 1 IO Expander 1:
	 * Enable DP159 Output
	 * Select GT3 as TMDS Clk source */
	/* Note : for HDMI4K FMC:
	 * Buffer[0] = 0x05;
	 * ByteCount = XIic_Send(BaseAddr, 0x22,
	 *		(u8*)Buffer, 1, XIIC_STOP);
	 * if (ByteCount != 1) {
	 *	xil_printf("Failed to set the I2C IO Expander.\r\n");
	 *	return XST_FAILURE;
	 * }
	 */

	Status |= IDT_8T49N24x_Init(Iic_Ptr, VFMC_I2C_IDT8N49_ADDR);
	Status |= IDT_8T49N24x_GpioLolEnable(Iic_Ptr,
					VFMC_I2C_IDT8N49_ADDR);
	if (Status != XST_SUCCESS) {
		xil_printf("Failed to initialize IDT 8T49N241.\r\n");
		return XST_FAILURE;
	}

	Status = TI_LMK03318_Init(Iic_Ptr, VFMC_I2C_LMK03318_ADDR);
	if (Status != XST_SUCCESS) {
		xil_printf("Failed to initialize TI LMK03318.\r\n");
		return XST_FAILURE;
	}

	/* Used for the RX GT ref clock */
	Status = TI_LMK03318_EnableBypass(Iic_Ptr,
					VFMC_I2C_LMK03318_ADDR, 0, 4);
	if (Status != XST_SUCCESS) {
		xil_printf("Failed to enable bypass for port 4.\r\n");
		return XST_FAILURE;
	}

	/* Used for the TX GT ref clock */
	Status = TI_LMK03318_EnableBypass(Iic_Ptr,
					VFMC_I2C_LMK03318_ADDR, 0, 6);
	if (Status != XST_SUCCESS) {
		xil_printf("Failed to enable bypass for port 6.\r\n");
		return XST_FAILURE;
	}

	/* TX Mezzanine Init Done */
	Vfmc_Gpio_Led_On(VFMC_GPIO_TX_LED0, TRUE);

	/* RX Mezzanine Init Done */
	Vfmc_Gpio_Led_On(VFMC_GPIO_RX_LED0, TRUE);

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* This function en/disables the power up pin of TI LMK03318 clock generator.
*
* @param  PowerDown true =power down / false= power up.
*
* @return The number of bytes sent.
*
* @note   None.
*
******************************************************************************/
int Vfmc_PowerDownTiLMK03318(void *IicPtr, u8 Powerdown)
{
	u8 Buffer;
	int ByteCount;

	/* Read IO Expander ouput register */
	ByteCount = Vfmc_I2cRecv(IicPtr, VFMC_I2C_IOEXP_1_ADDR,
			(u8 *)&Buffer, 1, I2C_STOP);

	/* Mask out the TI LMK03318 Power Down pin */
	Buffer &= ~0x02;

	/* Assign Power down pin value */
	Buffer |= ((Powerdown==0) ? 0 : 1<<2);

	/* Write updated values */
	ByteCount += Vfmc_I2cSend(IicPtr, VFMC_I2C_IOEXP_1_ADDR,
			(u8*)Buffer, 1, I2C_STOP);

	if (ByteCount == 2)
	  return XST_SUCCESS;
	else
	  return XST_FAILURE;
}

/*****************************************************************************/
/**
*
* This function Turns on or off the VFMC LED.
*
* @param  Led - LED position based on XVfmc_Gpio_Led typdef.
* @param  On  - TRUE=On ; FALSE=Off.
*
* @return None.
*
* @note   LED0 - Init Done
*         LED1 - Ch4 as Data
*
******************************************************************************/
void Vfmc_Gpio_Led_On(XVfmc_Gpio_Led Led, u8 On)
{
	u32 Data;

	Data = XGpio_DiscreteRead(&Gpio_Vfmc, 1);

	if (On == TRUE) {
		Data |= Led;
	} else {
		Data &= ~Led;
	}

	XGpio_DiscreteWrite(&Gpio_Vfmc, 1, Data);
}

/*****************************************************************************/
/**
*
* This function Sets the clock or data selection for channel 4 of the
* TX or RX mezzanine cards.
*
* @param  DataClkSel - Ch4 Selection based on  XVfmc_Gpio_Ch4_DataClkSel
*                      typdef.
*                      TX
*                      --> VFMC_GPIO_TX_CH4_As_DataAndClock
*                      --> VFMC_GPIO_TX_CH4_As_ClockOut
*                      RX
*                      --> VFMC_GPIO_RX_CH4_As_Data
*                      --> VFMC_GPIO_RX_CH4_As_Clock
*
* @return None.
*
* @note   LED1 - ON Ch4 as Data ;  OFF- Clock
*
******************************************************************************/
void Vfmc_Gpio_Ch4_DataClock_Sel(XVfmc_Gpio_Ch4_DataClkSel DataClkSel)
{
	u32 Data;

	Data = XGpio_DiscreteRead(&Gpio_Vfmc, 1);

	if (DataClkSel == VFMC_GPIO_TX_CH4_As_DataAndClock) {
		Data |=   0x00000004 | VFMC_GPIO_TX_LED1;
	} else if (DataClkSel == VFMC_GPIO_TX_CH4_As_ClockOut) {
		Data &= ~(0x00000004 | VFMC_GPIO_TX_LED1);
	} else if (DataClkSel == VFMC_GPIO_RX_CH4_As_Data) {
		Data |=   0x00040000 | VFMC_GPIO_RX_LED1;
	} else if (DataClkSel == VFMC_GPIO_RX_CH4_As_Clock) {
		Data &= ~(0x00040000 | VFMC_GPIO_RX_LED1);
	}

	XGpio_DiscreteWrite(&Gpio_Vfmc, 1, Data);
}

