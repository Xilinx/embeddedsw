/*****************************************************************************/
/**
*
* @file xmulti_scaler_example.c
*
* This is main file for the MultiScaler example design.
*
* The MultiScaler HW configuration is detected and several use cases are
* selected to test it.
*
* On start-up the program reads the HW config and initializes its internal
* data structures. The interrupt handler is registered.
* The set of use cases mentioned in the array are selected.
* Testing a use case is done by:
*	1) Read the destination buffer before scaling.
*	2) Program the parameters mentioned in the use case to the HW registers.
*	3) Start the HW, and when the scaling completes interrupt handler is
*	invoked. In the interrupt handler again read the contents of the
*	destination buffer to verify that the scaled data is available in the
*	destination buffer.
*	4) Go back and set up the next use case, repeating steps 1,2,3.
*
******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "xil_cache.h"
#include "xparameters.h"
#include "xv_multi_scaler_l2.h"
#include "xscugic.h"

/************************** Local Constants *********************************/
#define XMULTISCALER_SW_VER "v1.00"
#define XGPIOPS_MASK_DATA_3_LSW_OFFSET	0x00000018
#define XGPIOPS_DIRM_3_OFFSET	0x000002C4
#define XGPIOPS_OEN_3_OFFSET	0x000002C8
#define XGPIOPS_DATA_3_OFFSET	0x0000004C
#define USECASE_COUNT 1
#define XNUM_OUTPUTS 3
#define SRC_BUF_START_ADDR 0x10000000
#define DST_BUF_START_ADDR 0x30000000

XScuGic Intc;
XV_multi_scaler MultiScalerInst;
XV_multi_scaler_Video_Config *thisCase;
u32 interrupt_flag;

/* list of use cases */
XV_multi_scaler_Video_Config useCase[USECASE_COUNT][XNUM_OUTPUTS] = {
	{
		{
			0, SRC_BUF_START_ADDR, SRC_BUF_START_ADDR + 8 *
			XV_MAX_BUF_SIZE, 720, 540, 1920, 1920,
			XV_MULTI_SCALER_RGB8, XV_MULTI_SCALER_RGB8, 0, 0,
			DST_BUF_START_ADDR, DST_BUF_START_ADDR + 7 *
			XV_MAX_BUF_SIZE
		},
		{
			1, SRC_BUF_START_ADDR + XV_MAX_BUF_SIZE,
			SRC_BUF_START_ADDR + 9 * XV_MAX_BUF_SIZE, 1280, 1920,
			720, 720, XV_MULTI_SCALER_RGB8,	XV_MULTI_SCALER_RGB8, 0,
			0, DST_BUF_START_ADDR + XV_MAX_BUF_SIZE,
			DST_BUF_START_ADDR + 8 * XV_MAX_BUF_SIZE
		},
		{
			2, SRC_BUF_START_ADDR + 2 * XV_MAX_BUF_SIZE,
			SRC_BUF_START_ADDR + 10 * XV_MAX_BUF_SIZE, 1080, 1920,
			1920, 1920, XV_MULTI_SCALER_RGB8, XV_MULTI_SCALER_RGB8,
			0, 0, DST_BUF_START_ADDR + 2 * XV_MAX_BUF_SIZE,
			DST_BUF_START_ADDR + 9 * XV_MAX_BUF_SIZE
		}
	}
};

/*****************************************************************************/
/**
 *
 * This function setups the interrupt system so interrupts can occur for the
 * multiscaler core.
 *
 * @return
 *	- XST_SUCCESS if interrupt setup was successful.
 *	- A specific error code defined in "xstatus.h" if an error
 *	occurs.
 *
 * @note	This function assumes a Microblaze or ARM system and no
 *	operating system is used.
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
	if (!IntcCfgPtr) {
		xil_printf("ERR:: Interrupt Controller not found");
		return XST_DEVICE_NOT_FOUND;
	}
	Status = XScuGic_CfgInitialize(IntcInstPtr, IntcCfgPtr,
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
		(Xil_ExceptionHandler) XScuGic_InterruptHandler,
		(XScuGic *)IntcInstPtr);

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
 *
 * This function drives the reset pin of core high to start multiscaler core.
 *
 * @return
 *
 * @note	This function assumes a Microblaze or ARM system and no
 *	operating system is used.
 *
 ******************************************************************************/
void XV_Reset_MultiScaler(void)
{
	XV_multi_scaler_WriteReg(XPAR_PSU_GPIO_0_BASEADDR,
	XGPIOPS_MASK_DATA_3_LSW_OFFSET, 0xFFFF0000);
	XV_multi_scaler_WriteReg(XPAR_PSU_GPIO_0_BASEADDR,
	XGPIOPS_DIRM_3_OFFSET, 0xFFFFFFFF);
	XV_multi_scaler_WriteReg(XPAR_PSU_GPIO_0_BASEADDR,
	XGPIOPS_OEN_3_OFFSET, 0xFFFFFFFF);
	XV_multi_scaler_WriteReg(XPAR_PSU_GPIO_0_BASEADDR,
	XGPIOPS_DATA_3_OFFSET, 0x00000001);
	XV_multi_scaler_WriteReg(XPAR_PSU_GPIO_0_BASEADDR,
	XGPIOPS_DATA_3_OFFSET, 0x00000000);
	XV_multi_scaler_WriteReg(XPAR_PSU_GPIO_0_BASEADDR,
	XGPIOPS_DATA_3_OFFSET, 0x00000001);
}

/*****************************************************************************/
/**
 *
 * This function is called from the interrupt handler of multiscaler core.
 * After the first interrupt is received the interrupt_flag is set here and
 * it stops the multi scaler core.
 *
 * @return
 *
 * @note	This function assumes a Microblaze or ARM system and no
 *	operating system is used.
 *
 ******************************************************************************/
void *XVMultiScalerCallback(void *data)
{
	xil_printf("\nMultiScaler interrupt received.\r\n");

	/* clear interrupt flag */
	interrupt_flag = 0;

	XV_multi_scaler_InterruptGlobalDisable(&MultiScalerInst);
	XV_multi_scaler_InterruptDisable(&MultiScalerInst, 0xF);
	XV_MultiScalerStop(&MultiScalerInst);
}

/*****************************************************************************/
/**
 * This function calculates the stride
 *
 * @returns stride in bytes
 *
 *****************************************************************************/
static u32 CalcStride(u16 Cfmt, u16 AXIMMDataWidth, u32 width)
{
	u32 stride;
	u16 MMWidthBytes = AXIMMDataWidth / 8;

	if (Cfmt == XV_MULTI_SCALER_Y_UV10 || Cfmt == XV_MULTI_SCALER_Y_UV10_420
		|| Cfmt == XV_MULTI_SCALER_Y10)
		/* 4 bytes per 3 pixels (Y_UV10, Y_UV10_420, Y10) */
		stride = ((((width * 4) / 3) + MMWidthBytes - 1) /
			MMWidthBytes) * MMWidthBytes;
	else if (Cfmt == XV_MULTI_SCALER_Y_UV8 ||
		Cfmt == XV_MULTI_SCALER_Y_UV8_420 || Cfmt == XV_MULTI_SCALER_Y8)
		/* 1 byte per pixel (Y_UV8, Y_UV8_420, Y8) */
		stride = ((width + MMWidthBytes - 1) / MMWidthBytes) *
			MMWidthBytes;
	else if (Cfmt == XV_MULTI_SCALER_RGB8 || Cfmt == XV_MULTI_SCALER_YUV8 ||
		Cfmt == XV_MULTI_SCALER_BGR8)
		/* 3 bytes per pixel (RGB8, YUV8, BGR8) */
		stride = (((width * 3) + MMWidthBytes - 1) /
			MMWidthBytes) * MMWidthBytes;
	else
		/* 4 bytes per pixel */
		stride = (((width * 4) + MMWidthBytes - 1) /
		MMWidthBytes) * MMWidthBytes;

	return stride;
}

int main(void)
{
	XV_multi_scaler *MultiScalerPtr;

	u32 status;
	u32 k;
	u32 i;
	u32 j;
	u32 cnt = 0;
	u32 width_in_words = 0;
	u32 width_out_words = 0;
	_Bool flag = 1;
	u32 num_outs;

	/* Bind instance pointer with definition */
	MultiScalerPtr	= &MultiScalerInst;

	/* Initialize ICache */
	Xil_ICacheInvalidate();
	Xil_ICacheDisable();

	/* Initialize DCache */
	Xil_DCacheInvalidate();
	Xil_DCacheDisable();

	Xil_ExceptionDisable();

	xil_printf("\r\n-----------------------------------------------\r\n");
	xil_printf(" Xilinx Multi Scaler Example Design %s\r\n",
		XMULTISCALER_SW_VER);
	xil_printf("	(c) 2018 by Xilinx Inc.\r\n");

	/* Initialize IRQ */
	status = SetupInterruptSystem();
	if (status == XST_FAILURE) {
		xil_printf("IRQ init failed.\n\r\r");
		return XST_FAILURE;
	}

	status = XV_multi_scaler_Initialize(MultiScalerPtr,
		XPAR_V_MULTI_SCALER_0_DEVICE_ID);
	if (status != XST_SUCCESS) {
		xil_printf("CRITICAL ERROR:: System Init Failed.\n\r");
		return XST_FAILURE;
	}

	XVMultiScaler_SetCallback(MultiScalerPtr, XVMultiScalerCallback,
		(void *)MultiScalerPtr);
	status = XScuGic_Connect(&Intc,
		XPAR_FABRIC_V_MULTI_SCALER_0_INTERRUPT_INTR,
		(XInterruptHandler)XV_MultiScalerIntrHandler,
		(void *)MultiScalerPtr);
	if (status == XST_SUCCESS) {
		XScuGic_Enable(&Intc,
			XPAR_FABRIC_V_MULTI_SCALER_0_INTERRUPT_INTR);
	} else {
		xil_printf("ERR:: Unable to register interrupt handler");
		return XST_FAILURE;
	}

	Xil_ExceptionEnable();
	while (cnt < USECASE_COUNT) {
		xil_printf("\nUse case %d:\n", cnt);

		XV_Reset_MultiScaler();
		XV_MultiScalerSetNumOutputs(MultiScalerPtr, XNUM_OUTPUTS);
		num_outs = XV_MultiScalerGetNumOutputs(MultiScalerPtr); 
		if (num_outs != XNUM_OUTPUTS) {
			xil_printf("\nERR:: Incorrect number of outputs\n");
			return XST_FAILURE;
		}
		/* fill the src buffer with a fixed pattern */
		for (k = 0; k < num_outs; k++)	{
			thisCase = &useCase[cnt][k];
			thisCase->InStride = CalcStride(
				thisCase->ColorFormatIn,
				MultiScalerPtr->MaxDataWidth,
				thisCase->WidthIn);
			thisCase->OutStride = CalcStride(
				thisCase->ColorFormatOut,
				MultiScalerPtr->MaxDataWidth,
				thisCase->WidthOut);
			memset((u32 *)thisCase->SrcImgBuf0, 0,
				thisCase->HeightIn * thisCase->InStride);
			memset((u32 *)thisCase->SrcImgBuf1, 0,
				thisCase->HeightIn * thisCase->InStride);
			memset((u32 *)thisCase->DstImgBuf0, 0xAA,
				thisCase->HeightOut * thisCase->OutStride +
				10);
			memset((u32 *)thisCase->DstImgBuf1, 0xAA,
				thisCase->HeightOut * thisCase->OutStride +
				10);
			XV_MultiScalerSetChannelConfig(MultiScalerPtr,
				thisCase);
		}	

		xil_printf("\nStart MultiScaler.\r\n");
		XV_multi_scaler_InterruptGlobalEnable(MultiScalerPtr);
		XV_multi_scaler_InterruptEnable(MultiScalerPtr, 0xF);

		interrupt_flag = 1;
		XV_MultiScalerStart(MultiScalerPtr);
		/* wait for interrupt */
		while (interrupt_flag == 1)
			;
		/* Check if the data in the destination buffer is proper*/
		for (k = 0; k < XNUM_OUTPUTS; k++) {
			thisCase = &useCase[cnt][k];
			width_in_words = thisCase->InStride / 4;
			width_out_words = thisCase->OutStride / 4;

			for (i = 0; i < thisCase->HeightOut; i++) {
				for (j = 0; j < width_out_words; j++) {
					if (*((u32 *)thisCase->DstImgBuf0 +
						width_out_words * i + j) !=
							0x0) {
						xil_printf("\ndata mismatch");
						xil_printf("addr=%x data=%x\n",
						(u32 *)thisCase->DstImgBuf0 +
						width_out_words*i+j,
						*((u32 *)thisCase->DstImgBuf0 +
						width_out_words*i+j));
						flag = 0;
						break;
					}
				}
				if (flag == 0) {
					break;
				}
			}
			if (*((u32 *)thisCase->DstImgBuf0 +
				thisCase->HeightOut * width_out_words+1)
				!= 0xAAAAAAAA){
				flag = 0;
				xil_printf("\nDestination buffer overflown\n");
			}
		}
		if (flag == 0) {
			xil_printf("Test case %d failed\n", cnt);
			break;
		}

		xil_printf("\nEnd testing this use case. \r\n");
		cnt++;
	}
	if (cnt == USECASE_COUNT)
		xil_printf("\nMultiScaler test successful. \r\n");
	else
		xil_printf("MultiScaler test failed. \r\n");

	return 0;
}
