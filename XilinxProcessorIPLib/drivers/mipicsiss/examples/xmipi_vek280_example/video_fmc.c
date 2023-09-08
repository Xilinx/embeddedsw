/******************************************************************************
* Copyright (C) 2018 – 2021 Xilinx, Inc.  All rights reserved.
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

#include "video_fmc.h"

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

/* BASE BOARD I2C ADDRESSES */
#define VFMC_I2C_IDT8N49_ADDR   0x6C /**< I2C IDT 8N49N241 Address */
#define VFMC_I2C_IOEXP_1_ADDR 	0x65 /**< I2C IO Expander 1 address */
#define VFMC_I2C_IOEXP_0_ADDR 	0x64 /**< I2C IO Expander 0 address */
#define VFMC_I2C_LMK03318_ADDR  0x51 /**< I2C TI LMK03318 Address */
#define VFMC_I2C_SI5344_ADDR    0x68 /**< I2C SI5344 Address */
#define RC21008A_ADDR   		0x09 /**<PS I2C RC21008A Address */

/* MEZZANINE CARD I2C ADDRESSES */
#define VFMC_MEZZ_I2C_NB7NQ621M_TX_ADDR   0x5B  /**< I2C Address NB7NQ621M*/
#define VFMC_MEZZ_I2C_NB7NQ621M_RX_ADDR   0x5C  /**< I2C Address NB7NQ621M*/

#define VFMC_MEZZ_I2C_TMDS1204_TX_ADDR   0x5E  /**< I2C Address NB7NQ621M*/
#define VFMC_MEZZ_I2C_TMDS1204_RX_ADDR   0x5B  /**< I2C Address NB7NQ621M*/


static int  Vfmc_ModifyRegister(void *IicPtr, u8 SlaveAddr, u8 Value, u8 Mask);

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
#if ! (defined (XPS_BOARD_ZCU102) || \
	defined (XPS_BOARD_ZCU104) || \
	defined (XPS_BOARD_ZCU106) || \
    defined (XPS_BOARD_VCK190) || \
	defined (XPS_BOARD_VEK280))
	XIicPs *Iic_Ptr = IicPtr;
	u32 Status;

	XIicPs_SetOptions(Iic_Ptr, XIICPS_7_BIT_ADDR_OPTION);
	XIicPs_ClearOptions(Iic_Ptr, XIICPS_10_BIT_ADDR_OPTION);
	if (Option == I2C_REPEATED_START) {
		XIicPs_SetOptions(Iic_Ptr, XIICPS_REP_START_OPTION);
	} else {
		XIicPs_ClearOptions(Iic_Ptr, XIICPS_REP_START_OPTION);
	}

	Status = XIicPs_MasterSendPolled(Iic_Ptr, MsgPtr, ByteCount,
			SlaveAddr);

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

#if (defined XPS_BOARD_VEK280)

unsigned Vfmc_I2cSend_RC(void *IicPtr, u16 SlaveAddr, u8 *MsgPtr,
		unsigned ByteCount, u8 Option)
{
#if defined (XPS_BOARD_ZCU102) || \
	defined (XPS_BOARD_ZCU104) || \
	defined (XPS_BOARD_ZCU106) || \
    defined (XPS_BOARD_VCK190) || \
	defined (XPS_BOARD_VEK280)
	XIicPs *Iic_Ptr = IicPtr;
	u32 Status;

	XIicPs_SetOptions(Iic_Ptr, XIICPS_7_BIT_ADDR_OPTION);
	XIicPs_ClearOptions(Iic_Ptr, XIICPS_10_BIT_ADDR_OPTION);
	if (Option == I2C_REPEATED_START) {
		XIicPs_SetOptions(Iic_Ptr, XIICPS_REP_START_OPTION);
	} else {
		XIicPs_ClearOptions(Iic_Ptr, XIICPS_REP_START_OPTION);
	}

	Status = XIicPs_MasterSendPolled(Iic_Ptr, MsgPtr, ByteCount,
			SlaveAddr);

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
#endif
}
#endif
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
#if ! (defined (XPS_BOARD_ZCU102) || \
	defined (XPS_BOARD_ZCU104) || \
	defined (XPS_BOARD_ZCU106) || \
    defined (XPS_BOARD_VCK190) || \
	defined (XPS_BOARD_VEK280))
	XIicPs *Iic_Ptr = IicPtr;
	u32 Status;

	XIicPs_SetOptions(Iic_Ptr, XIICPS_7_BIT_ADDR_OPTION);
	XIicPs_ClearOptions(Iic_Ptr, XIICPS_10_BIT_ADDR_OPTION);
	if (Option == I2C_REPEATED_START) {
		XIicPs_SetOptions(Iic_Ptr, XIICPS_REP_START_OPTION);
	} else {
		XIicPs_ClearOptions(Iic_Ptr, XIICPS_REP_START_OPTION);
	}

	Status = XIicPs_MasterRecvPolled(Iic_Ptr, BufPtr, ByteCount,
			SlaveAddr);

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
static int Vfmc_ModifyRegister(void *IicPtr, u8 SlaveAddr, u8 Value, u8 Mask)
{
	u8 Data;
	int ByteCount;

	/* Read data */
	ByteCount = Vfmc_I2cRecv(IicPtr, SlaveAddr, (u8 *)&Data, 1, I2C_STOP);

	/* Clear masked bits */
	Data &= ~Mask;

	/* Update */
	Data |= (Value & Mask);

	/* Write data */
	ByteCount +=
		Vfmc_I2cSend(IicPtr, SlaveAddr, (u8 *)&Data, 1, (I2C_STOP));

	if (ByteCount == 2)
	  return XST_SUCCESS;
	else
	  return XST_FAILURE;
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
int Vfmc_I2cMuxSelect(XVfmc *VfmcPtr)
{
	u8 Buffer;
	int Status;
	void *IicPtr = VfmcPtr->IicPtr;
	XVfmc_Location Loc = VfmcPtr->Loc;

#if (defined (XPS_BOARD_VCU118)) && (!defined (XPS_BOARD_ZCU106))
	Loc = Loc;

	/* Reset I2C controller before issuing new transaction. This is
	 * required to recover the IIC controller in case a previous
	 * transaction is pending.
	 */
	/*XIic_WriteReg(XPAR_IIC_0_BASEADDR, XIIC_RESETR_OFFSET,
				  XIIC_RESET_MASK);*/

	/* Set TCA9548 MUX1 to select port 1 (FMCP HSCP IIC port)*/
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

	/* Set TCA9548 U135 to select port 0 or 1 (HPC0/1)*/
	if (Loc == VFMC_HPC0) {
		Buffer = 0x01;
	} else {
		Buffer = 0x02;
	}

	Status = Vfmc_I2cSend(IicPtr, 0x75,
					   (u8 *)&Buffer, 1, (I2C_STOP));

#elif defined (XPS_BOARD_ZCU106)
	/* Set TCA9548 U34 to select port 7 (No connection)*/
	Buffer = 0x80;
	Status = Vfmc_I2cSend(IicPtr, 0x74,
					   (u8 *)&Buffer, 1, (I2C_STOP));

	/* Set TCA9548 U135 to select port 0 or 1 (HPC0/1)*/
	if (Loc == VFMC_HPC0) {
		Buffer = 0x01;
	} else {
		Buffer = 0x02;
	}

	Status = Vfmc_I2cSend(IicPtr, 0x75,
					   (u8 *)&Buffer, 1, (I2C_STOP));
#elif defined (XPS_BOARD_VCK190)
	/* Set TCA9548 U135 to select port 0 or 1 (HPC0/1)*/
	if (Loc == VFMC_HPC0) {
		Buffer = 0x02;
	} else {
		Buffer = 0x04;
	}
	Status = Vfmc_I2cSend(IicPtr, 0x74,
					   (u8 *)&Buffer, 1, (I2C_STOP));

#elif defined (XPS_BOARD_VEK280)
	/* Set TCA9548 U135 to select port 0 or 1 (HPC0/1)*/
	if (Loc == VFMC_HPC0) {
		Buffer = 0x02;
	} else {
		Buffer = 0x04;
	}
	Status = Vfmc_I2cSend(IicPtr, 0x74,
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
u32 Vfmc_HdmiInit(XVfmc *VfmcPtr, u16 GpioDeviceId, void *IicPtr,
		XVfmc_Location Loc)
{
	int Status;
	u8 Buffer[2];
	int ByteCount;
	u8 RevisionNumber;
	XGpio_Config *Gpio_Vfmc_ConfigPtr;

//	VfmcPtr->IicPtr = IicPtr;

	/* Check if VFMC was already Initialized */
	if (VfmcPtr->IsReady == XIL_COMPONENT_IS_READY) {
		xil_printf("VFMC has already been initialized. "
				"Exiting Vfmc_HdmiInit\r\n");
		return (XST_FAILURE);
	} else {
		VfmcPtr->IicPtr = IicPtr;
		VfmcPtr->Loc = Loc;
		VfmcPtr->TxMezzType = VFMC_MEZZ_INVALID;
		VfmcPtr->RxMezzType = VFMC_MEZZ_INVALID;
		VfmcPtr->IsReady = 0;
	}


#if ! (defined (XPS_BOARD_ZCU102) || \
	defined (XPS_BOARD_ZCU104) || \
	defined (XPS_BOARD_ZCU106) || \
    defined (XPS_BOARD_VCK190) || \
    defined (XPS_BOARD_VEK280))
	XIicPs *Iic_Ptr = IicPtr;
#else
	XIic *Iic_Ptr = IicPtr;
#endif

	/* Initialize GPIO for VFMC */
	Gpio_Vfmc_ConfigPtr =
		XGpio_LookupConfig(GpioDeviceId);

	if(Gpio_Vfmc_ConfigPtr == NULL) {
		VfmcPtr->Gpio.IsReady = 0;
		xil_printf("ERR:: GPIO for VFMC not found\r\n");
		return (XST_FAILURE);
	}

	Status = XGpio_CfgInitialize(&VfmcPtr->Gpio,
			Gpio_Vfmc_ConfigPtr,
			Gpio_Vfmc_ConfigPtr->BaseAddress);
	if(Status != XST_SUCCESS) {
		xil_printf("ERR:: GPIO for VFMC ");
		xil_printf("Initialization failed %d\r\n", Status);
		return(XST_FAILURE);
	}

//	Vfmc_I2cMuxSelect(VfmcPtr);

	/* Configure VFMC IO Expander 0:
	 * Enabled SI5344, To Disable SI5344 set Buffer[0] = 0x52
	 * Set primary clock source for LMK03318 to IOCLKp(0)
	 * Set secondary clock source for LMK03318 to IOCLKp(1)
	 * Disable LMK61E2*/
#if (defined XPS_BOARD_ZCU102)
		Buffer[0] = 0x41;
#elif (defined XPS_BOARD_ZCU106)
		Buffer[0] = 0x41;
#elif (defined XPS_BOARD_VCU118)
		Buffer[0] = 0x41;
#else /* Place Holder for other board */
		Buffer[0] = 0x41;
#endif

//	ByteCount = Vfmc_I2cSend(IicPtr, VFMC_I2C_IOEXP_0_ADDR,
//			(u8*)Buffer, 1, I2C_STOP);
//	if (ByteCount != 1) {
//		xil_printf("Failed to set the I2C IO Expander.\r\n");
//		return XST_FAILURE;
//	}

	/* Configure VFMC IO Expander 1:
	 * Enable LMK03318 -> In a power-down state the I2C bus becomes
	 * unusable.
	 * Select LMK03318 clock as source for FMC_GT_CLKp(0)
	 * Select IDT8T49N241 clock as source for FMC_GT_CLKp(1)
	 * Enable IDT8T49N241 */
//	Buffer[0] = 0x1A;
//	ByteCount = Vfmc_I2cSend(IicPtr, VFMC_I2C_IOEXP_1_ADDR,
//			(u8*)Buffer, 1, I2C_STOP);
//	if (ByteCount != 1) {
//		xil_printf("Failed to set the I2C IO Expander.\r\n");
//		return XST_FAILURE;
//	}

	/* Configure HDMI2.0 Mezz in Slot 1 IO Expander 1:
	 * Enable DP159 Output
	 * Select GT3 as TMDS Clk source */
	/* Note : for HDMI4K FMC,
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

#if defined (XPS_BOARD_VEK280)

#else

	Status = TI_LMK03318_Init(Iic_Ptr, VFMC_I2C_LMK03318_ADDR);
	if (Status != XST_SUCCESS) {
		xil_printf("Failed to initialize TI LMK03318.\r\n");
		return XST_FAILURE;
	}

	/*SI5344 Initialization */
	Status = SI5344_Init(Iic_Ptr, VFMC_I2C_SI5344_ADDR);
	if (Status != XST_SUCCESS) {
		xil_printf("Failed to initialize SI5344.\r\n");
		return XST_FAILURE;
	}
#endif

#ifdef XPAR_XV_HDMITXSS1_NUM_INSTANCES
	/* Check if mezzanine card is with an active device */
	if (ONSEMI_NB7NQ621M_CheckDeviceID(Iic_Ptr,
			VFMC_MEZZ_I2C_NB7NQ621M_TX_ADDR) == XST_SUCCESS) {
		RevisionNumber = ONSEMI_NB7NQ621M_CheckDeviceVersion(Iic_Ptr,
				VFMC_MEZZ_I2C_NB7NQ621M_TX_ADDR);
		if (RevisionNumber == 0x00) {
			/* Revision 0 Silicon */
			VfmcPtr->TxMezzType = VFMC_MEZZ_HDMI_ONSEMI_R0;
		} else if (RevisionNumber == 0x01) {
			/* Revision 1 Silicon */
			VfmcPtr->TxMezzType = VFMC_MEZZ_HDMI_ONSEMI_R1;
		} else if (RevisionNumber == 0x24) {
			/* Revision Pass3 Silicon */
			VfmcPtr->TxMezzType = VFMC_MEZZ_HDMI_ONSEMI_R2;
		} else if (RevisionNumber == 0x02) {
			/* Revision Pass4 Silicon */
			VfmcPtr->TxMezzType = VFMC_MEZZ_HDMI_ONSEMI_R3;
		} else {
			VfmcPtr->TxMezzType = VFMC_MEZZ_INVALID;
			xil_printf("VFMC TX Mezz Not Supported!\r\n");
		}
		ONSEMI_NB7NQ621M_Init(Iic_Ptr, VFMC_MEZZ_I2C_NB7NQ621M_TX_ADDR,
				VfmcPtr->TxMezzType - VFMC_MEZZ_HDMI_ONSEMI_R0, 1);
		Vfmc_Gpio_Mezz_HdmiTxDriver_Enable(VfmcPtr, TRUE);
		xil_printf("VFMC Active HDMI TX Mezz (R%d) Detected\r\n",
				RevisionNumber);
		VfmcPtr->isTxTi = 0;
	} else if (TI_TMDS1204_CheckDeviceID(Iic_Ptr,
			VFMC_MEZZ_I2C_TMDS1204_TX_ADDR) == XST_SUCCESS) {
		RevisionNumber = TI_TMDS1204_CheckDeviceVersion(Iic_Ptr,
				VFMC_MEZZ_I2C_TMDS1204_TX_ADDR);
		if (RevisionNumber == 0x01) {
			VfmcPtr->TxMezzType = VFMC_MEZZ_HDMI_TI_R1;
		} else if (RevisionNumber == 0x03) {
			/* Revision Pass4 Silicon */
			VfmcPtr->TxMezzType = VFMC_MEZZ_HDMI_TI_R3;
		} else {
			VfmcPtr->TxMezzType = VFMC_MEZZ_INVALID;
			xil_printf("VFMC TX Mezz Not Supported!\r\n");
		}
		TI_TMDS1204_Init(Iic_Ptr, VFMC_MEZZ_I2C_TMDS1204_TX_ADDR,
				VfmcPtr->TxMezzType - VFMC_MEZZ_HDMI_TI_R0, 1);
		Vfmc_Gpio_Mezz_HdmiTxDriver_Enable(VfmcPtr, TRUE);
		xil_printf("VFMC Active HDMI TX (TI) Mezz (R%d) Detected\r\n",
				RevisionNumber);
		VfmcPtr->isTxTi = 1;

	} else {
		VfmcPtr->TxMezzType = VFMC_MEZZ_HDMI_PASSIVE;
		VfmcPtr->isTxTi = 0;
		xil_printf("VFMC Passive HDMI TX Mezz Detected\r\n");
	}
	/* TX Mezzanine Init Done */
	Vfmc_Gpio_Led_On(VfmcPtr, VFMC_GPIO_TX_LED0, TRUE);
#endif

	VfmcPtr->IsReady = XIL_COMPONENT_IS_READY;

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
int Vfmc_PowerDownTiLMK03318(XVfmc *VfmcPtr, u8 Powerdown)
{
	u8 Buffer;
	int ByteCount;
	void *IicPtr = VfmcPtr->IicPtr;

	/* Read IO Expander ouput register */
	ByteCount = Vfmc_I2cRecv(IicPtr, VFMC_I2C_IOEXP_1_ADDR,
			(u8 *)&Buffer, 1, I2C_STOP);

	/* Mask out the TI LMK03318 Power Down pin */
	Buffer &= ~0x02;

	/* Assign Power down pin value */
	Buffer |= ((Powerdown==0) ? 0 : 1<<2);

	/* Write updated values */
	ByteCount += Vfmc_I2cSend(IicPtr, VFMC_I2C_IOEXP_1_ADDR,
			(u8*)&Buffer, 1, I2C_STOP);

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
void Vfmc_Gpio_Led_On(XVfmc *VfmcPtr, XVfmc_Gpio_Led Led, u8 On)
{
	u32 Data;

	Data = XGpio_DiscreteRead(&VfmcPtr->Gpio, 1);

	if (On == TRUE) {
		Data |= Led;
	} else {
		Data &= ~Led;
	}

	XGpio_DiscreteWrite(&VfmcPtr->Gpio, 1, Data);
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
* @note   - LED1 - ON Ch4 as Data ;  OFF- Clock
* 		  - Ignore configuration of TxMezz is Active type
*
******************************************************************************/
void Vfmc_Gpio_Ch4_DataClock_Sel(XVfmc *VfmcPtr,
		XVfmc_Gpio_Ch4_DataClkSel DataClkSel)
{
	u32 Data;

	/* Skip if Tx Mezzanine is Active type since it is using the 4th GT
	 * channel as the TMDS Clock and Data source
	 */
	if (((DataClkSel == VFMC_GPIO_TX_CH4_As_DataAndClock) ||
		 (DataClkSel == VFMC_GPIO_TX_CH4_As_ClockOut)) &&
		(VfmcPtr->TxMezzType != VFMC_MEZZ_HDMI_PASSIVE)) {
		return;
	}

	Data = XGpio_DiscreteRead(&VfmcPtr->Gpio, 1);

	if (DataClkSel == VFMC_GPIO_TX_CH4_As_DataAndClock) {
		Data |=   0x00000004 | VFMC_GPIO_TX_LED1;
	} else if (DataClkSel == VFMC_GPIO_TX_CH4_As_ClockOut) {
		Data &= ~(0x00000004 | VFMC_GPIO_TX_LED1);
	} else if (DataClkSel == VFMC_GPIO_RX_CH4_As_Data) {
		Data |=   0x00040000 | VFMC_GPIO_RX_LED1;
	} else if (DataClkSel == VFMC_GPIO_RX_CH4_As_Clock) {
		Data &= ~(0x00040000 | VFMC_GPIO_RX_LED1);
	}

	XGpio_DiscreteWrite(&VfmcPtr->Gpio, 1, Data);

}


/*****************************************************************************/
/**
*
* This function Enables or Disables the ONSEMI NB7NQ621M on the TX MEZZ slot
*
* @param  Enable - TRUE / FALSE
*
* @return None.

*
******************************************************************************/
void Vfmc_Gpio_Mezz_HdmiTxDriver_Enable(XVfmc *VfmcPtr, u8 Enable)
{
	u32 Data;
	/* Read GPIO Register */
	Data = XGpio_DiscreteRead(&VfmcPtr->Gpio, 1);

	if (Enable == TRUE) {
		Data &= ~(0x00000004);
	} else {
		Data |= 0x00000004;
	}

	/* Write new register value */
	XGpio_DiscreteWrite(&VfmcPtr->Gpio, 1, Data);
}

/*****************************************************************************/
/**
*
* This function Enables or Disables the ONSEMI NB7NQ621M on the RX MEZZ slot
*
* @param  Enable - TRUE / FALSE
*
* @return None.

*
******************************************************************************/
void Vfmc_Gpio_Mezz_HdmiRxEqualizer_Enable(XVfmc *VfmcPtr, u8 Enable)
{
	u32 Data;
	/* Read GPIO Register */
	Data = XGpio_DiscreteRead(&VfmcPtr->Gpio, 1);

	if (Enable == TRUE) {
		Data &= ~(0x00080000);
	} else {
		Data |= 0x00080000;
	}

	/* Write new register value */
	XGpio_DiscreteWrite(&VfmcPtr->Gpio, 1, Data);
}

/*****************************************************************************/
/**
*
* This function Enables or Disables the ONSEMI NB7NQ621M on the TX MEZZ slot
*
* @param  Enable - TRUE / FALSE
*
* @return None.

*
******************************************************************************/
void Vfmc_Gpio_Mezz_HdmiTxDriver_Reconfig(XVfmc *VfmcPtr, u8 IsFRL,
		u64 LineRate, u8 Lanes)
{
	if (VfmcPtr->TxMezzType != VFMC_MEZZ_HDMI_PASSIVE) {
		if (VfmcPtr->isTxTi) {
			TI_TMDS1204_LineRateReconfig(VfmcPtr->IicPtr,
				VFMC_MEZZ_I2C_TMDS1204_TX_ADDR,
				(VfmcPtr->TxMezzType -
						VFMC_MEZZ_HDMI_TI_R0),
				IsFRL, LineRate, Lanes, 1);
		} else {
			ONSEMI_NB7NQ621M_LineRateReconfig(VfmcPtr->IicPtr,
				VFMC_MEZZ_I2C_NB7NQ621M_TX_ADDR,
				(VfmcPtr->TxMezzType -
						VFMC_MEZZ_HDMI_ONSEMI_R0),
				IsFRL, LineRate, 1);
		}
	}
}

/*****************************************************************************/
/**
*
* This function Enables or Disables the ONSEMI NB7NQ621M on the RX MEZZ slot
*
* @param  Enable - TRUE / FALSE
*
* @return None.

*
******************************************************************************/
void Vfmc_Gpio_Mezz_HdmiRxDriver_Reconfig(XVfmc *VfmcPtr, u8 IsFRL,
		u64 LineRate, u8 Lanes)
{
	if (VfmcPtr->isRxTi) {
		xil_printf ("programming rx ti\r\n");
		if (VfmcPtr->RxMezzType == VFMC_MEZZ_HDMI_TI_R3) {
				TI_TMDS1204_LineRateReconfig(VfmcPtr->IicPtr,
				VFMC_MEZZ_I2C_TMDS1204_RX_ADDR,
				(VfmcPtr->RxMezzType -
						VFMC_MEZZ_HDMI_TI_R0),
				IsFRL, LineRate, Lanes, 0);
		}
	} else {
		if (VfmcPtr->RxMezzType >= VFMC_MEZZ_HDMI_ONSEMI_R1) {
				ONSEMI_NB7NQ621M_LineRateReconfig(VfmcPtr->IicPtr,
				VFMC_MEZZ_I2C_NB7NQ621M_RX_ADDR,
				(VfmcPtr->RxMezzType -
						VFMC_MEZZ_HDMI_ONSEMI_R0),
				IsFRL, LineRate, 0);
		}
	}
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
u32 Vfmc_Mezz_HdmiRxRefClock_Sel(XVfmc *VfmcPtr, XVfmc_Mezz_RxRefClkSel Sel)
{
	u32 Status;
	void *IicPtr = VfmcPtr->IicPtr;
	Vfmc_I2cMuxSelect(VfmcPtr);

	if (Sel == VFMC_MEZZ_RxRefclk_From_Si5344) {
		/* Set RX Refclk to Si5344 */
		Status = Vfmc_ModifyRegister(IicPtr, VFMC_I2C_IOEXP_0_ADDR,
						0x41, 0x18);
	} else if (Sel == VFMC_MEZZ_RxRefclk_From_Cable) {
		/* Set RX Refclk to IOCLK(0) */
		Status = Vfmc_ModifyRegister(IicPtr, VFMC_I2C_IOEXP_0_ADDR,
						0x51, 0x18);
	} else {
		xil_printf("Invalid RX Ref clock selected.\r\n");
		return XST_FAILURE;
	}
	return Status;
}

/*****************************************************************************/
/**
*
* This function selects the TX ref clock on GTYE4 devices
*
* @param  Source of ref clock
*
* @return XST_SUCCESS if the ref clock source is successfuly set.
*         XST_FAILURE otherwise.
*
* @note   None.
*
******************************************************************************/
u32 Vfmc_Mezz_HdmiTxRefClock_Sel(XVfmc *VfmcPtr, XVfmc_Mezz_TxRefClkSel Sel)
{
	u32 Status = XST_SUCCESS;
	void *IicPtr = VfmcPtr->IicPtr;
	Vfmc_I2cMuxSelect(VfmcPtr);

	if (Sel == VFMC_MEZZ_TxRefclk_From_IDT) {
		Status = Vfmc_ModifyRegister(IicPtr, VFMC_I2C_IDT8N49_ADDR,
						0x1A, 0x08);
		Status |= Vfmc_ModifyRegister(IicPtr, VFMC_I2C_IDT8N49_ADDR,
						0x41, 0x60);

	} else if (Sel == VFMC_MEZZ_TxRefclk_From_Si5344) {
		Status = Vfmc_ModifyRegister(IicPtr, VFMC_I2C_IOEXP_1_ADDR,
						0x12, 0x08);
		Status |= Vfmc_ModifyRegister(IicPtr, VFMC_I2C_IOEXP_0_ADDR,
						0x01, 0x60);
	} else{
		xil_printf("Invalid TX Ref clock selected.\r\n");
		return XST_FAILURE;
	}

	if (Status == XST_FAILURE) {
		xil_printf("Failed to select TX Ref clock.\r\n");
	}
	return Status;
}
