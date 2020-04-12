/******************************************************************************
* Copyright (C) 2018 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xsdi_example.c
*
* This file demonstrates how to use Xilinx SDI Rx Subsystem for standalone.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who    Date     Changes
* ----- ------ -------- --------------------------------------------------
* 1.00  PG     08/11/17 Initial release.
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/
#include <stdio.h>
#include "platform.h"
#include "xparameters.h"
#define ARMR5 1
#define __aarch64__ 1
#if defined (ARMR5) || (__aarch64__)
#include "xiicps.h"
#endif
#include "xiic.h"
#include "si5324drv.h"
#include "xil_printf.h"
#include "xil_types.h"
#include "xil_exception.h"
#include "xv_sdivid.h"
#include "xvidc.h"
#include "sleep.h"
#include "xsdi_menu.h"
#include "xv_sdirxss.h"
#include "xscugic.h"

/************************** Constant Definitions *****************************/
#define GPIO_RX_RST XPAR_GPIO_0_BASEADDR

typedef u8 AddressType;
#define PAGE_SIZE   16

#define I2C_MUX_ADDR	0x74  /**< I2C Mux Address */
#define I2C_CLK_ADDR	0x69  /**< I2C Clk Address */
#define I2C_CLK_ADDR_570	0x5D  /**< I2C Clk Address for Si570*/

#define SDI_RX_SS_INTR_ID XPAR_FABRIC_V_SMPTE_UHDSDI_RX_SS_SDI_RX_IRQ_INTR

/***************** Macros (Inline Functions) Definitions *********************/


/**************************** Type Definitions *******************************/


/************************** Function Prototypes ******************************/
void ClearScreen(void);
static int SetupInterruptSystem(void);
void RxStreamUpCallback(void *CallbackRef);
void RxStreamUp(void);
void RxStreamDownCallback(void *CallbackRef);
void Xil_AssertCallbackRoutine(u8 *File, s32 Line);
static int I2cMux(void);
int Si570_SetClock(u32 IICBaseAddress, u8 IICAddress1);
void Info(void);
void DebugInfo(void);

/*
 * Write buffer for writing a page.
 */
u8 WriteBuffer[sizeof(AddressType) + PAGE_SIZE];

u8 ReadBuffer[PAGE_SIZE];	/* Read buffer for reading a page. */

volatile u8 TransmitComplete;	/* Flag to check completion of Transmission */
volatile u8 ReceiveComplete;	/* Flag to check completion of Reception */

/************************** Variable Definitions *****************************/

static XScuGic Intc;

#if defined (ARMR5) || (__aarch64__)
XIicPs Iic;
#endif

XV_SdiRxSs SdiRxSs;       /* SDI RX SS structure */
XV_SdiRxSs_Config *XV_SdiRxSs_ConfigPtr;

#define UART_BASEADDR XPAR_XUARTPS_0_BASEADDR

u8 StartTxAfterRxFlag;
XSdi_Menu SdiMenu;			/**< Menu structure */

/************************** Function Definitions *****************************/

/*****************************************************************************/
/**
 *
 * This function Clears Uart terminal screen.
 *
 * @return	None
 *
 * @note	None.
 *
 ******************************************************************************/
void ClearScreen(void)
{
	xil_printf("%c\[2J", 27);	/**< Clear Sreen */
	xil_printf("%c\033[0;0H", 27);	/**< Bring Cursor to 0,0 */
}

/*****************************************************************************/
/**
 *
 * This function setups the interrupt system so interrupts can occur for the
 * SDI cores. The function is application-specific since the actual system
 * may or may not have an interrupt controller. The SDI cores could be
 * directly connected to a processor without an interrupt controller.
 * The user should modify this function to fit the application.
 *
 * @return
 *		- XST_SUCCESS if interrupt setup was successful.
 *		- A specific error code defined in "xstatus.h" if an error
 *		occurs.
 *
 * @note	This function assumes a Microblaze or ARM system and no operating
 *		system is used.
 *
 ******************************************************************************/
static int SetupInterruptSystem(void)
{
	int Status;
	XScuGic *IntcInstPtr = &Intc;

	/*
	 * Initialize the interrupt controller driver so that it's ready to
	 * use, specify the device ID that was generated in xparameters.h
	 */
	XScuGic_Config *IntcCfgPtr;
	IntcCfgPtr = XScuGic_LookupConfig(XPAR_PSU_ACPU_GIC_DEVICE_ID);
	if (IntcCfgPtr == NULL) {
		xil_printf("ERR:: Interrupt Controller not found");
		return XST_DEVICE_NOT_FOUND;
	}
	Status = XScuGic_CfgInitialize(IntcInstPtr,
					IntcCfgPtr,
					IntcCfgPtr->CpuBaseAddress);

	if (Status != XST_SUCCESS) {
		xil_printf("Intc initialization failed!\r\n");
		return XST_FAILURE;
	}

	Xil_ExceptionInit();

	/*
	 * Register the interrupt controller handler with the exception table.
	 */
	Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_INT,
				(Xil_ExceptionHandler)XScuGic_InterruptHandler,
				(XScuGic *)IntcInstPtr);

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
 *
 * This function setup SI5324 clock generator over IIC.
 *
 * @return	The number of bytes sent.
 *
 * @note	None.
 *
 ******************************************************************************/
static int I2cMux(void)
{
	u8 Buffer;
	int Status;

	xil_printf("Set i2c mux... ");

	Buffer = 0x18;
	Status = XIic_Send((XPAR_IIC_0_BASEADDR),
				(I2C_MUX_ADDR),
				(u8 *)&Buffer,
				1,
				(XIIC_STOP));
	xil_printf("done\n\r");

	return Status;
}

/*****************************************************************************/
/**
 *
 * This function is called when the RX Lock occurs, it calculates the frequency
 * wrt RxTransMode, programs SI5324 with the calculated frequency and then
 * enables Rx stream flow IPs.
 *
 * @param	CallbackRef is a callback function reference.
 *
 * @return	None.
 *
 * @note	None.
 *
 ******************************************************************************/
void RxStreamUpCallback(void *CallbackRef)
{
	xil_printf("INFO>> SDI Rx: Input Locked\r\n");

	XV_SdiRxSs_ReportInfo(&SdiRxSs);

	XV_SdiRxSs_StreamFlowEnable(&SdiRxSs);
}

/*****************************************************************************/
/**
 *
 * This function is called when the RX looses lock, and it stops SDI Tx
 * Subsystem.
 *
 * @param	CallbackRef is a callback function reference.
 *
 * @return	None.
 *
 * @note	None.
 *
 ******************************************************************************/
void RxStreamDownCallback(void *CallbackRef)
{
	xil_printf("INFO>> SDI Rx: Lock Lost\r\n");
}

/*****************************************************************************/
/**
 *
 * This function is called when the RX Over flow happens.
 *
 * @param	CallbackRef is a callback function reference.
 *
 * @return None.
 *
 * @note   None.
 *
 ******************************************************************************/
void RxOverFlowCallback(void *CallbackRef)
{
	xil_printf("INFO>> SDI Rx: Over FLow\r\n");
}

/*****************************************************************************/
/**
 *
 * This function is called when the RX Under flow happens.
 *
 * @param	CallbackRef is a callback function reference.
 *
 * @return None.
 *
 * @note   None.
 *
 ******************************************************************************/
void RxUnderFlowCallback(void *CallbackRef)
{
	xil_printf("INFO>> SDI Rx: Under FLow\r\n");
}

/*****************************************************************************/
/**
 *
 * This function is called when assertion hits in the file.
 *
 * @param	File - File name where assertion occured.
 * 		Line - Line number in this file.
 *
 * @return None.
 *
 * @note   None.
 *
 ******************************************************************************/
void Xil_AssertCallbackRoutine(u8 *File, s32 Line)
{
	xil_printf("Assertion in File %s, on line %0d\n\r", File, Line);
}

/*****************************************************************************/
/**
*
* This function outputs the video information on TX and RX.
*
* @return None.
*
* @note   None.
*
******************************************************************************/
void Info(void)
{
	xil_printf("\n\r-----\n\r");
	xil_printf("Info\n\r");
	xil_printf("-----\n\r\n\r");

	XV_SdiRxSs_ReportInfo(&SdiRxSs);
}

/*****************************************************************************/
/**
 *
 * This function outputs the debug information.
 *
 * @return	None.
 *
 * @note	None.
 *
 ******************************************************************************/
void DebugInfo(void)
{
	xil_printf("\n\r-----\n\r");
	xil_printf("Info\n\r");
	xil_printf("-----\n\r\n\r");

	XV_SdiRxSs_ReportDebugInfo(&SdiRxSs);
}
/*****************************************************************************/
/**
 *
 * Main function to call example with SDI TX and SDI RX drivers.
 *
 * @return
 *		- XST_SUCCESS if SDI example was successfully.
 *		- XST_FAILURE if SDI example failed.
 *
 * @note	None.
 *
 ******************************************************************************/
int main(void)
{
	u32 Status;

	/* Setting path for Si570 chip */
	I2cMux();

	/* si570 configuration of 148.5MHz */
	Si570_SetClock(XPAR_IIC_0_BASEADDR, I2C_CLK_ADDR_570);

	/* Rx Reset Sequence */
	Xil_Out32((UINTPTR)(GPIO_RX_RST), (u32)(0x00000002));
	Xil_Out32((UINTPTR)(GPIO_RX_RST), (u32)(0x00000001));

	StartTxAfterRxFlag = (FALSE);

	Xil_ExceptionDisable();
	init_platform();

	ClearScreen();

	xil_printf("\n\r");
	xil_printf("----------------------------------------\r\n");
	xil_printf("---      SDI Rx Standalone v1.0      ---\r\n");
	xil_printf("---     (c) 2017 by Xilinx, Inc.     ---\r\n");
	xil_printf("----------------------------------------\r\n");
	xil_printf("      Build %s - %s      \r\n", __DATE__, __TIME__);
	xil_printf("----------------------------------------\r\n");

	/* Initialize IRQ */
	Status = SetupInterruptSystem();
	if (Status == XST_FAILURE) {
		xil_printf("IRQ init failed.\n\r\r");
		return XST_FAILURE;
	}

    /* Initialize SDI RX Subsystem */
	XV_SdiRxSs_ConfigPtr = XV_SdiRxSs_LookupConfig(XPAR_XV_SDIRX_0_DEVICE_ID);

	XV_SdiRxSs_ConfigPtr->BaseAddress = XPAR_V_SMPTE_UHDSDI_RX_SS_BASEADDR;

	if (XV_SdiRxSs_ConfigPtr == NULL) {
		xil_printf(" Error::: XV_SdiRxSs_ConfigPtr Null pointer\n\r");
		SdiRxSs.IsReady = 0;
		return XST_DEVICE_NOT_FOUND;
	}

	/* Initialize top level and all included sub-cores */
	Status = XV_SdiRxSs_CfgInitialize(&SdiRxSs,
					XV_SdiRxSs_ConfigPtr,
					XV_SdiRxSs_ConfigPtr->BaseAddress);

	if (Status != XST_SUCCESS) {
		xil_printf("ERR:: SDI RX Initialization failed %d\r\n", Status);
		return XST_FAILURE;
	}

	/* Disable SDI Rx all Interrupts */
	XV_SdiRxSs_IntrDisable(&SdiRxSs, XV_SDIRXSS_IER_ALLINTR_MASK);

	XV_SdiRxSs_SetCallback(&SdiRxSs,
				XV_SDIRXSS_HANDLER_STREAM_UP,
				RxStreamUpCallback,
				(void *)&SdiRxSs);

	XV_SdiRxSs_SetCallback(&SdiRxSs,
				XV_SDIRXSS_HANDLER_STREAM_DOWN,
				RxStreamDownCallback,
				(void *)&SdiRxSs);

    XV_SdiRxSs_SetCallback(&SdiRxSs,
							XV_SDIRXSS_HANDLER_OVERFLOW,
							RxOverFlowCallback,
							(void *)&SdiRxSs);

    XV_SdiRxSs_SetCallback(&SdiRxSs,
							XV_SDIRXSS_HANDLER_UNDERFLOW,
							RxUnderFlowCallback,
							(void *)&SdiRxSs);

	/* Enable SDI Rx video lock and unlock Interrupts */
	XV_SdiRxSs_IntrEnable(&SdiRxSs, XV_SDIRXSS_IER_VIDEO_LOCK_MASK | XV_SDIRXSS_IER_VIDEO_UNLOCK_MASK);

	/* Register SDI RX SS Interrupt Handler with Interrupt Controller */
	Status = 0;
	Status |= XScuGic_Connect(&Intc,
				SDI_RX_SS_INTR_ID,
				(XInterruptHandler)XV_SdiRxSS_SdiRxIntrHandler,
				(void *)&SdiRxSs);
	if (Status == XST_SUCCESS) {
		XScuGic_Enable(&Intc, SDI_RX_SS_INTR_ID);
	} else {
		xil_printf("ERR:: Unable to register SDI RX interrupt handler");
		xil_printf("SDI RX SS initialization error\n\r");
		return XST_FAILURE;
	}

	memset((u32 *)0x10000000, 0xff, 2000);

	/* Initialize menu */
	XSdi_MenuInitialize(&SdiMenu, UART_BASEADDR);

	/* Enable exceptions. */
	Xil_AssertSetCallback((Xil_AssertCallback) Xil_AssertCallbackRoutine);
	Xil_ExceptionEnable();

	if(StartTxAfterRxFlag) {
		XSdi_MenuProcess(&SdiMenu);
	}
	return XST_SUCCESS;
}
