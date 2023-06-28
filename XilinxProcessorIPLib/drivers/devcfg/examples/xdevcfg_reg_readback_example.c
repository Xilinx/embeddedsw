/******************************************************************************
* Copyright (C) 2014 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2023 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
* @file  xdevcfg_reg_readback_example.c
*
* This file contains a design example using the DevCfg driver and hardware
* device.
*
* This example prints out the values of all the configuration registers in the
* FPGA.
*
* This example assumes that there is a UART Device or STDIO Device in the
* hardware system.
*
* @note		None.
*
* MODIFICATION HISTORY:
*
*<pre>
* Ver   Who  Date     Changes
* ----- ---- -------- ---------------------------------------------
* 3.1   sb  08/25/14  First Release
* 3.8  Nava 06/21/23  Added support for system device-tree flow.
*</pre>
******************************************************************************/

/***************************** Include Files *********************************/

#include "xparameters.h"
#include "xdevcfg.h"
#include "xil_cache.h"
/************************** Constant Definitions *****************************/
/*
 * The following constants map to the XPAR parameters created in the
 * xparameters.h file. They are only defined here such that a user can easily
 * change all the needed parameters in one place.
 */
#ifndef SDT
#define DCFG_DEVICE_ID		XPAR_XDCFG_0_DEVICE_ID
#else
#define DCFG_BASEADDR           XPAR_XDEVCFG_0_BASEADDR
#endif

/**
 * @name Configuration Type1 packet headers masks
 * @{
 */
#define XDC_TYPE_SHIFT			29
#define XDC_REGISTER_SHIFT		13
#define XDC_OP_SHIFT			27
#define XDC_TYPE_1			1
#define OPCODE_READ			1
/* @} */

/*
 * Addresses of the Configuration Registers
 */
#define CRC		0	/* Status Register */
#define FAR		1	/* Frame Address Register */
#define FDRI		2	/* FDRI Register */
#define FDRO		3	/* FDRO Register */
#define CMD		4	/* Command Register */
#define CTL0		5	/* Control Register 0 */
#define MASK		6	/* MASK Register */
#define STAT		7	/* Status Register */
#define LOUT		8	/* LOUT Register */
#define COR0		9	/* Configuration Options Register 0 */
#define MFWR		10	/* MFWR Register */
#define CBC		11	/* CBC Register */
#define IDCODE		12	/* IDCODE Register */
#define AXSS		13	/* AXSS Register */
#define COR1		14	/* Configuration Options Register 1 */
#define WBSTAR		15	/* Warm Boot Start Address Register */
#define TIMER		16	/* Watchdog Timer Register */
#define BOOTSTS		17	/* Boot History Status Register */
#define CTL1		18	/* Control Register 1 */

/*
 * Mask For IDCODE
 */
#define IDCODE_MASK   0x0FFFFFFF

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/
#ifndef SDT
int XDcfgRegReadExample(u16 DeviceId);
#else
int XDcfgRegReadExample(UINTPTR BaseAddress);
#endif
int XDcfg_GetConfigReg(XDcfg *InstancePtr, u32 ConfigReg, u32 *RegData);
u32 XDcfg_RegAddr(u8 Register, u8 OpCode, u8 Size);
/************************** Variable Definitions *****************************/

XDcfg DcfgInstance;		/* Device Configuration Interface Instance */

/*****************************************************************************/
/**
*
* Main function to call the DevCfg Reg Read example.
*
* @param	None.
*
* @return
*		- XST_SUCCESS if successful
*		- XST_FAILURE if unsuccessful
*
* @note		None.
*
******************************************************************************/
int main(void)
{
	int Status;
	xil_printf("Dev Cfg Register Read back example\r\n");

	Xil_DCacheDisable();
	Xil_ICacheDisable();

	/*
	 * Call the example , specify the device ID that is generated in
	 * xparameters.h.
	 */
#ifndef SDT
	Status = XDcfgRegReadExample(DCFG_DEVICE_ID);
#else
	Status = XDcfgRegReadExample(DCFG_BASEADDR);
#endif
	if (Status != XST_SUCCESS) {
		xil_printf("Dev Cfg Register Read back example Failed\r\n");
		return XST_FAILURE;
	}

	xil_printf("Successfully ran Dev Cfg Register Read back example\r\n");

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* This function reads the configuration registers inside the FPGA.
*
* @param	DeviceId is the unique device id of the device.
*
* @return
*		- XST_SUCCESS if successful
*		- XST_FAILURE if unsuccessful
*
* @note		None.
*
******************************************************************************/
#ifndef SDT
int XDcfgRegReadExample(u16 DeviceId)
#else
int XDcfgRegReadExample(UINTPTR BaseAddress)
#endif
{
	int Status;
	unsigned int ValueBack;

	XDcfg_Config *ConfigPtr;

	/*
	 * Initialize the Device Configuration Interface driver.
	 */
#ifndef SDT
	ConfigPtr = XDcfg_LookupConfig(DeviceId);
#else
	ConfigPtr = XDcfg_LookupConfig(BaseAddress);
#endif
	/*
	 * This is where the virtual address would be used, this example
	 * uses physical address.
	 */
	Status = XDcfg_CfgInitialize(&DcfgInstance, ConfigPtr,
				     ConfigPtr->BaseAddr);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Run the Self test.
	 */
	Status = XDcfg_SelfTest(&DcfgInstance);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	xil_printf("Value of the Configuration Registers. \r\n\r\n");

	if (XDcfg_GetConfigReg(&DcfgInstance, CRC, (u32 *)&ValueBack) !=
	    XST_SUCCESS) {
		return XST_FAILURE;
	}
	xil_printf(" CRC -> \t %x \t\r\n", ValueBack);


	if (XDcfg_GetConfigReg(&DcfgInstance, FAR, (u32 *)&ValueBack) !=
	    XST_SUCCESS) {
		return XST_FAILURE;
	}
	xil_printf(" FAR -> \t %x \t\r\n", ValueBack);

	if (XDcfg_GetConfigReg(&DcfgInstance, FDRI, (u32 *)&ValueBack) !=
	    XST_SUCCESS) {
		return XST_FAILURE;
	}
	xil_printf(" FDRI -> \t %x \t\r\n", ValueBack);

	if (XDcfg_GetConfigReg(&DcfgInstance, FDRO, (u32 *)&ValueBack) !=
	    XST_SUCCESS) {
		return XST_FAILURE;
	}
	xil_printf(" FDRO -> \t %x \t\r\n", ValueBack);

	if (XDcfg_GetConfigReg(&DcfgInstance, CMD, (u32 *)&ValueBack) !=
	    XST_SUCCESS) {
		return XST_FAILURE;
	}
	xil_printf(" CMD -> \t %x \t\r\n", ValueBack);

	if (XDcfg_GetConfigReg(&DcfgInstance, CTL0, (u32 *)&ValueBack) !=
	    XST_SUCCESS) {
		return XST_FAILURE;
	}
	xil_printf(" CTL0 -> \t %x \t\r\n", ValueBack);

	if (XDcfg_GetConfigReg(&DcfgInstance, MASK, (u32 *)&ValueBack) !=
	    XST_SUCCESS) {
		return XST_FAILURE;
	}
	xil_printf(" MASK -> \t %x \t\r\n", ValueBack);

	if (XDcfg_GetConfigReg(&DcfgInstance, STAT, (u32 *)&ValueBack) !=
	    XST_SUCCESS) {
		return XST_FAILURE;
	}
	xil_printf(" STAT -> \t %x \t\r\n", ValueBack);

	if (XDcfg_GetConfigReg(&DcfgInstance, LOUT, (u32 *)&ValueBack) !=
	    XST_SUCCESS) {
		return XST_FAILURE;
	}
	xil_printf(" LOUT -> \t %x \t\r\n", ValueBack);

	if (XDcfg_GetConfigReg(&DcfgInstance, COR0, (u32 *)&ValueBack) !=
	    XST_SUCCESS) {
		return XST_FAILURE;
	}
	xil_printf(" COR0 -> \t %x \t\r\n", ValueBack);

	if (XDcfg_GetConfigReg(&DcfgInstance, MFWR, (u32 *)&ValueBack) !=
	    XST_SUCCESS) {
		return XST_FAILURE;
	}
	xil_printf(" MFWR -> \t %x \t\r\n", ValueBack);

	if (XDcfg_GetConfigReg(&DcfgInstance, CBC, (u32 *)&ValueBack) !=
	    XST_SUCCESS) {
		return XST_FAILURE;
	}
	xil_printf(" CBC -> \t %x \t\r\n", ValueBack);

	if (XDcfg_GetConfigReg(&DcfgInstance, IDCODE, (u32 *)&ValueBack) !=
	    XST_SUCCESS) {
		return XST_FAILURE;
	}
	xil_printf(" IDCODE -> \t %x \t\r\n", ValueBack & IDCODE_MASK);

	if (XDcfg_GetConfigReg(&DcfgInstance, AXSS, (u32 *)&ValueBack) !=
	    XST_SUCCESS) {
		return XST_FAILURE;
	}
	xil_printf(" AXSS -> \t %x \t\r\n", ValueBack);

	if (XDcfg_GetConfigReg(&DcfgInstance, COR1, (u32 *)&ValueBack) !=
	    XST_SUCCESS) {
		return XST_FAILURE;
	}
	xil_printf(" COR1 -> \t %x \t\r\n", ValueBack);

	if (XDcfg_GetConfigReg(&DcfgInstance, WBSTAR, (u32 *)&ValueBack) !=
	    XST_SUCCESS) {
		return XST_FAILURE;
	}
	xil_printf(" WBSTAR -> \t %x \t\r\n", ValueBack);

	if (XDcfg_GetConfigReg(&DcfgInstance, TIMER, (u32 *)&ValueBack) !=
	    XST_SUCCESS) {
		return XST_FAILURE;
	}
	xil_printf(" TIMER -> \t %x \t\r\n", ValueBack);

	if (XDcfg_GetConfigReg(&DcfgInstance, BOOTSTS, (u32 *)&ValueBack) !=
	    XST_SUCCESS) {
		return XST_FAILURE;
	}
	xil_printf(" BOOTSTS -> \t %x \t\r\n", ValueBack);

	if (XDcfg_GetConfigReg(&DcfgInstance, CTL1, (u32 *)&ValueBack) !=
	    XST_SUCCESS) {
		return XST_FAILURE;
	}
	xil_printf(" CTL1 -> \t %x \t\r\n", ValueBack);

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* This function returns the value of the specified configuration register.
*
* @param	InstancePtr is a pointer to the XHwIcap instance.
* @param	ConfigReg  is a constant which represents the configuration
*			register value to be returned.
* @param	RegData is the value of the specified configuration
*			register.
*
* @return
*		- XST_SUCCESS if successful
*		- XST_FAILURE if unsuccessful
*
* @note	None.
*
****************************************************************************/
int XDcfg_GetConfigReg(XDcfg *DcfgInstancePtr, u32 ConfigReg, u32 *RegData)
{
	u32 IntrStsReg;
	u32 StatusReg;
	unsigned int CmdIndex;
	unsigned int CmdBuf[18];

	/*
	 * Clear the interrupt status bits
	 */
	XDcfg_IntrClear(DcfgInstancePtr, (XDCFG_IXR_PCFG_DONE_MASK |
					  XDCFG_IXR_D_P_DONE_MASK | XDCFG_IXR_DMA_DONE_MASK));

	/* Check if DMA command queue is full */
	StatusReg = XDcfg_ReadReg(DcfgInstancePtr->Config.BaseAddr,
				  XDCFG_STATUS_OFFSET);
	if ((StatusReg & XDCFG_STATUS_DMA_CMD_Q_F_MASK) ==
	    XDCFG_STATUS_DMA_CMD_Q_F_MASK) {
		return XST_FAILURE;
	}

	/*
	 * Register Readback in non secure mode
	 * Create the data to be written to read back the
	 * Configuration Registers from PL Region.
	 */
	CmdIndex = 0;
	CmdBuf[CmdIndex++] = 0xFFFFFFFF; 	/* Dummy Word */
	CmdBuf[CmdIndex++] = 0xFFFFFFFF; 	/* Dummy Word */
	CmdBuf[CmdIndex++] = 0xFFFFFFFF; 	/* Dummy Word */
	CmdBuf[CmdIndex++] = 0xFFFFFFFF; 	/* Dummy Word */
	CmdBuf[CmdIndex++] = 0xFFFFFFFF; 	/* Dummy Word */
	CmdBuf[CmdIndex++] = 0xFFFFFFFF; 	/* Dummy Word */
	CmdBuf[CmdIndex++] = 0xFFFFFFFF; 	/* Dummy Word */
	CmdBuf[CmdIndex++] = 0xFFFFFFFF; 	/* Dummy Word */
	CmdBuf[CmdIndex++] = 0x000000BB; 	/* Bus Width Sync Word */
	CmdBuf[CmdIndex++] = 0x11220044; 	/* Bus Width Detect */
	CmdBuf[CmdIndex++] = 0xFFFFFFFF; 	/* Dummy Word */
	CmdBuf[CmdIndex++] = 0xAA995566; 	/* Sync Word */
	CmdBuf[CmdIndex++] = 0x20000000; 	/* Type 1 NOOP Word 0 */
	CmdBuf[CmdIndex++] = XDcfg_RegAddr(ConfigReg, OPCODE_READ, 0x1);
	CmdBuf[CmdIndex++] = 0x20000000; 	/* Type 1 NOOP Word 0 */
	CmdBuf[CmdIndex++] = 0x20000000; 	/* Type 1 NOOP Word 0 */

	XDcfg_Transfer(&DcfgInstance, (&CmdBuf[0]),
		       CmdIndex, RegData, 1, XDCFG_PCAP_READBACK);

	/* Poll IXR_DMA_DONE */
	IntrStsReg = XDcfg_IntrGetStatus(DcfgInstancePtr);
	while ((IntrStsReg & XDCFG_IXR_DMA_DONE_MASK) !=
	       XDCFG_IXR_DMA_DONE_MASK) {
		IntrStsReg = XDcfg_IntrGetStatus(DcfgInstancePtr);
	}

	/* Poll IXR_D_P_DONE */
	while ((IntrStsReg & XDCFG_IXR_D_P_DONE_MASK) !=
	       XDCFG_IXR_D_P_DONE_MASK) {
		IntrStsReg = XDcfg_IntrGetStatus(DcfgInstancePtr);
	}

	CmdIndex = 0;
	CmdBuf[CmdIndex++] = 0x30008001;	/* Dummy Word */
	CmdBuf[CmdIndex++] = 0x0000000D;	/* Bus Width Sync Word */
	CmdBuf[CmdIndex++] = 0x20000000;	/* Bus Width Detect */
	CmdBuf[CmdIndex++] = 0x20000000;	/* Dummy Word */
	CmdBuf[CmdIndex++] = 0x20000000;	/* Bus Width Detect */
	CmdBuf[CmdIndex++] = 0x20000000;	/* Dummy Word */

	XDcfg_InitiateDma(DcfgInstancePtr, (u32)(&CmdBuf[0]),
			  XDCFG_DMA_INVALID_ADDRESS, CmdIndex, 0);

	/* Poll IXR_DMA_DONE */
	IntrStsReg = XDcfg_IntrGetStatus(DcfgInstancePtr);
	while ((IntrStsReg & XDCFG_IXR_DMA_DONE_MASK) !=
	       XDCFG_IXR_DMA_DONE_MASK) {
		IntrStsReg = XDcfg_IntrGetStatus(DcfgInstancePtr);
	}

	/* Poll IXR_D_P_DONE */
	while ((IntrStsReg & XDCFG_IXR_D_P_DONE_MASK) !=
	       XDCFG_IXR_D_P_DONE_MASK) {
		IntrStsReg = XDcfg_IntrGetStatus(DcfgInstancePtr);
	}

	return XST_SUCCESS;
}

/****************************************************************************/
/**
*
* Generates a Type 1 packet header that reads back the requested Configuration
* register.
*
* @param	Register is the address of the register to be read back.
* @param	OpCode is the read/write operation code.
* @param	Size is the size of the word to be read.
*
* @return	Type 1 packet header to read the specified register
*
* @note		None.
*
*****************************************************************************/
u32 XDcfg_RegAddr(u8 Register, u8 OpCode, u8 Size)
{
	/*
	 * Type 1 Packet Header Format
	 * The header section is always a 32-bit word.
	 *
	 * HeaderType | Opcode | Register Address | Reserved | Word Count
	 * [31:29]	[28:27]		[26:13]	     [12:11]     [10:0]
	 * --------------------------------------------------------------
	 *   001 	  xx 	  RRRRRRRRRxxxxx	RR	xxxxxxxxxxx
	 *
	 * �R� means the bit is not used and reserved for future use.
	 * The reserved bits should be written as 0s.
	 *
	 * Generating the Type 1 packet header which involves sifting of Type 1
	 * Header Mask, Register value and the OpCode which is 01 in this case
	 * as only read operation is to be carried out and then performing OR
	 * operation with the Word Length.
	 */
	return ( ((XDC_TYPE_1 << XDC_TYPE_SHIFT) |
		  (Register << XDC_REGISTER_SHIFT) |
		  (OpCode << XDC_OP_SHIFT)) | Size);
}
