/******************************************************************************
*
* (c) Copyright 2013 Xilinx, Inc. All rights reserved.
*
* This file contains confidential and proprietary information of Xilinx, Inc.
* and is protected under U.S. and international copyright and other
* intellectual property laws.
*
* DISCLAIMER
* This disclaimer is not a license and does not grant any rights to the
* materials distributed herewith. Except as otherwise provided in a valid
* license issued to you by Xilinx, and to the maximum extent permitted by
* applicable law: (1) THESE MATERIALS ARE MADE AVAILABLE "AS IS" AND WITH ALL
* FAULTS, AND XILINX HEREBY DISCLAIMS ALL WARRANTIES AND CONDITIONS, EXPRESS,
* IMPLIED, OR STATUTORY, INCLUDING BUT NOT LIMITED TO WARRANTIES OF
* MERCHANTABILITY, NON-INFRINGEMENT, OR FITNESS FOR ANY PARTICULAR PURPOSE;
* and (2) Xilinx shall not be liable (whether in contract or tort, including
* negligence, or under any other theory of liability) for any loss or damage
* of any kind or nature related to, arising under or in connection with these
* materials, including for any direct, or any indirect, special, incidental,
* or consequential loss or damage (including loss of data, profits, goodwill,
* or any type of loss or damage suffered as a result of any action brought by
* a third party) even if such damage or loss was reasonably foreseeable or
* Xilinx had been advised of the possibility of the same.
*
* CRITICAL APPLICATIONS
* Xilinx products are not designed or intended to be fail-safe, or for use in
* any application requiring fail-safe performance, such as life-support or
* safety devices or systems, Class III medical devices, nuclear facilities,
* applications related to the deployment of airbags, or any other applications
* that could lead to death, personal injury, or severe property or
* environmental damage (individually and collectively, "Critical
* Applications"). Customer assumes the sole risk and liability of any use of
* Xilinx products in Critical Applications, subject only to applicable laws
* and regulations governing limitations on product liability.
*
* THIS COPYRIGHT NOTICE AND DISCLAIMER MUST BE RETAINED AS PART OF THIS FILE
* AT ALL TIMES.
*
******************************************************************************/
/*****************************************************************************/
/**
* @file sfp.c
*
* This file programs sfp phy chip.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date	 Changes
* ----- ---- -------- ---------------------------------------------------------
* 1.0   srt  10/19/13 Initial Version
*
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/
#include "xparameters.h"
#if XPAR_GIGE_PCS_PMA_CORE_PRESENT == 1
#include "xil_printf.h"
#include "xiicps.h"
#include "sleep.h"
#include "xscugic.h"

/************************** Constant Definitions *****************************/
#define IIC_SLAVE_ADDR		0x56
#define IIC_MUX_ADDRESS		0x74
#define IIC_CHANNEL_ADDRESS	0x01

#define XIIC	XIicPs
#define INTC	XScuGic
/**************************** Type Definitions *******************************/
typedef struct {
	XIIC I2cInstance;
	INTC IntcInstance;
	volatile u8 TransmitComplete;   /* Flag to check completion of Transmission */
	volatile u8 ReceiveComplete;    /* Flag to check completion of Reception */
	volatile u32 TotalErrorCount;
} XIIC_LIB;
/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/
int I2cWriteData(XIIC_LIB *I2cLibPtr, u8 *WrBuffer, u16 ByteCount, u16 SlaveAddr);
int I2cReadData(XIIC_LIB *I2cLibPtr, u8 *RdBuffer, u16 ByteCount, u16 SlaveAddr);
int I2cPhyWrite(XIIC_LIB *I2cLibPtr, u8 PhyAddr, u8 Reg, u16 Data, u16 SlaveAddr);
int I2cPhyRead(XIIC_LIB *I2cLibPtr, u8 PhyAddr, u8 Reg, u16 *Data, u16 SlaveAddr);
int I2cSetupHardware(XIIC_LIB *I2cLibPtr);
/************************** Function Definitions *****************************/

/*****************************************************************************/
/**
 * This function initializes ZC706 MUX.
 *
 * @param	I2cLibPtr contains a pointer to the instance of the IIC library
 *
 * @return	XST_SUCCESS if successful else XST_FAILURE.
 *
 ******************************************************************************/
int ZC706MuxInit(XIIC_LIB *I2cLibInstancePtr)
{
	u8 WrBuffer;
	int Status;

	WrBuffer = IIC_CHANNEL_ADDRESS;

	Status = I2cWriteData(I2cLibInstancePtr, &WrBuffer, 1, IIC_MUX_ADDRESS);
	if (Status != XST_SUCCESS) {
		xil_printf("SFP_PHY: Writing failed\n\r");
		return XST_FAILURE;
 	}
	return XST_SUCCESS;
}

/*****************************************************************************/
/**
 * This function program SFP PHY.
 *
 * @return	XST_SUCCESS if successful else XST_FAILURE.
 *
 ******************************************************************************/
int ProgramSfpPhy(void)
{
	XIIC_LIB I2cLibInstance;
	int Status;
	u8 WrBuffer[2];
	u16 phy_read_val;

	Status = I2cSetupHardware(&I2cLibInstance);
	if (Status != XST_SUCCESS) {
		xil_printf("Fail!!!\n\r");
		xil_printf("SFP_PHY: Configuring HW failed\n\r");
		return XST_FAILURE;
	}
	Status = ZC706MuxInit(&I2cLibInstance);
	if (Status != XST_SUCCESS) {
		xil_printf("SFP_PHY: Mux Init failed\n\r");
		return XST_FAILURE;
	}

	WrBuffer[0] = 0;
	Status = I2cWriteData(&I2cLibInstance, WrBuffer, 1, IIC_SLAVE_ADDR);
	if (Status != XST_SUCCESS) {
		xil_printf("SFP_PHY: Writing failed\n\r");
		return XST_FAILURE;
	}

	/* Enabling SGMII */
	I2cPhyWrite(&I2cLibInstance, IIC_SLAVE_ADDR, 0x1B, 0x9084, IIC_SLAVE_ADDR);

	/* Apply Soft Reset */
	I2cPhyWrite(&I2cLibInstance, IIC_SLAVE_ADDR, 0x00, 0x9140, IIC_SLAVE_ADDR);

	/* Enable 1000BaseT Full Duplex capabilities */
	I2cPhyWrite(&I2cLibInstance, IIC_SLAVE_ADDR, 0x09, 0x0E00, IIC_SLAVE_ADDR);

	/* Apply Soft Reset */
	I2cPhyWrite(&I2cLibInstance, IIC_SLAVE_ADDR, 0x00, 0x9140, IIC_SLAVE_ADDR);

	/* Advertise 10/100 Capabilities else change the capabilities */
	I2cPhyWrite(&I2cLibInstance, IIC_SLAVE_ADDR, 0x04, 0x0141, IIC_SLAVE_ADDR);

	/* Apply Soft Reset */
	I2cPhyWrite(&I2cLibInstance, IIC_SLAVE_ADDR, 0x00, 0x9140, IIC_SLAVE_ADDR);

	/* Apply Soft Reset */
	I2cPhyWrite(&I2cLibInstance, IIC_SLAVE_ADDR, 0x00, 0x9140, IIC_SLAVE_ADDR);

	I2cPhyWrite(&I2cLibInstance, IIC_SLAVE_ADDR, 0x10, 0xF079, IIC_SLAVE_ADDR);
	/* Apply Soft Reset */
	I2cPhyWrite(&I2cLibInstance, IIC_SLAVE_ADDR, 0x00, 0x9140, IIC_SLAVE_ADDR);

	I2cPhyWrite(&I2cLibInstance, IIC_SLAVE_ADDR, 0x16, 0x0001, IIC_SLAVE_ADDR);
	I2cPhyWrite(&I2cLibInstance, IIC_SLAVE_ADDR, 0x00, 0x9140, IIC_SLAVE_ADDR);
	I2cPhyWrite(&I2cLibInstance, IIC_SLAVE_ADDR, 0x00, 0x9340, IIC_SLAVE_ADDR);
	usleep(1);

	I2cPhyWrite(&I2cLibInstance, IIC_SLAVE_ADDR, 0x16, 0x0, IIC_SLAVE_ADDR);
	phy_read_val = 0x0;

	while((phy_read_val & 0x0C00) != 0x0C00) {
		I2cPhyRead(&I2cLibInstance, IIC_SLAVE_ADDR, 0x11, &phy_read_val, IIC_SLAVE_ADDR);
	}
	usleep(1);

	I2cPhyWrite(&I2cLibInstance, IIC_SLAVE_ADDR, 0x16, 0x0, IIC_SLAVE_ADDR);
	I2cPhyRead(&I2cLibInstance, IIC_SLAVE_ADDR, 0x11, &phy_read_val, IIC_SLAVE_ADDR);
	I2cPhyWrite(&I2cLibInstance, IIC_SLAVE_ADDR, 0x16, 0x0001, IIC_SLAVE_ADDR);

	/* configure speed */
	I2cPhyWrite(&I2cLibInstance, IIC_SLAVE_ADDR, 0x14, 0x0c61, IIC_SLAVE_ADDR);
	I2cPhyWrite(&I2cLibInstance, IIC_SLAVE_ADDR, 0x00, 0x9340, IIC_SLAVE_ADDR);
	I2cPhyWrite(&I2cLibInstance, IIC_SLAVE_ADDR, 0x16, 0x0, IIC_SLAVE_ADDR);

	return XST_SUCCESS;
}
#endif