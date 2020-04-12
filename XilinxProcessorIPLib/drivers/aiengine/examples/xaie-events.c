/******************************************************************************
* Copyright (C) 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
* @file xaie-events.c
* @{
*
* This file contains the test application for AIE events
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who     Date     Changes
* ----- ------  -------- -----------------------------------------------------
* 1.0  Wendy   05/28/2019  Initial creation
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/
#include <stddef.h>
#include <stdio.h>
#include <xaiengine.h>
#include <unistd.h>

#ifdef __AIEBAREMTL__
#include <xil_printf.h>
#include <xil_exception.h>
#include <xstatus.h>
#include <xscugic.h>
#endif

/************************** Constant Definitions *****************************/
#define XAIE_NUM_ROWS		8
#define XAIE_NUM_COLS		50
#define XAIE_ADDR_ARRAY_OFF	0x800

#define EVENTS_NUM_ROWS	4
#define EVENTS_NUM_COLS	4

#ifdef __AIEBAREMTL__
#define INTC_DEVICE_ID	XPAR_SCUGIC_0_DEVICE_ID
#define IRQ1_VECT_ID	180U /* AI engine NPI interrupt 1 vector ID */
#endif

/************************** Variable Definitions *****************************/
static XAieGbl_Config *AieConfigPtr;	/**< AIE configuration pointer */
static XAieGbl AieInst;			/**< AIE global instance */
static XAieGbl_HwCfg AieConfig;          /**< AIE HW configuration instance */

static XAieGbl_Tile TileInst[XAIE_NUM_COLS][XAIE_NUM_ROWS+1];

static u8 ParirtyErrorsCount[XAIE_NUM_COLS][XAIE_NUM_ROWS][8];

static u8 MemDmaS2MM0Errors[XAIE_NUM_COLS][XAIE_NUM_ROWS];
static u8 CoreDMUnavailErrors[XAIE_NUM_COLS][XAIE_NUM_ROWS];
static u8 ShimCntrPktErrors[XAIE_NUM_COLS];

#ifdef __AIEBAREMTL__
static XScuGic xInterruptController;
#endif
/************************** Function Prototypes  *****************************/

/************************** Function Definitions *****************************/
#ifdef __AIEBAREMTL__
/*****************************************************************************/
/**
*
* This is to intialize interrupt controller
* It will register the AI engine driver interrupt handler to the interrupt
* controller.
*
* @param	None.
*
* @return	0 for success, and negative value for failure.

* @note		This is required to enable AI engine interrupt. It will
*		connect the AI engine driver interrupt handler to the interrupt
*		controller driver.
*		This is required to by done by the baremetal application, as
*		there is no interrupt abstraction in baremetal. The interrupt
*		controller instance is maintained by application. There is no
*		generic function for driver to call to register interrupt
*		handller for baremetal.
*
*******************************************************************************/
static int init_irq()
{
	int ret = 0;
	XScuGic_Config *IntcConfig;	/* The configuration parameters of
					   the interrupt controller */
	Xil_ExceptionDisable();

	/* Initialize the interrupt controller driver */
	IntcConfig = XScuGic_LookupConfig(INTC_DEVICE_ID);
	if (NULL == IntcConfig) {
		return (int)XST_FAILURE;
	}

	ret = XScuGic_CfgInitialize(&xInterruptController, IntcConfig,
			       IntcConfig->CpuBaseAddress);
	if (ret != XST_SUCCESS) {
		return (int)XST_FAILURE;
	}

	/*
	 * Register the interrupt handler to the hardware interrupt handling
	 * logic in the ARM processor.
	 */
	Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_IRQ_INT,
				(Xil_ExceptionHandler)XScuGic_InterruptHandler,
				&xInterruptController);

	Xil_ExceptionEnable();
	/* Connect Interrupt */
	XScuGic_Connect(&xInterruptController, IRQ1_VECT_ID,
		   (Xil_InterruptHandler)XAieTile_EventsIsr,
		   (void *)(&AieInst));

	XScuGic_Enable(&xInterruptController, IRQ1_VECT_ID);

	printf("%s\n", __func__);

	return 0;
}
#endif

static XAieGbl_ErrorHandleStatus
_XAie_ParityErrorCallback(struct XAieGbl *AieInst, XAie_LocType Loc,
			  u8 Module, u8 Error, void *Arg)
{
	u32 RegVal;
	XAieGbl_Tile *TilePtr;

	(void)Arg;
	printf("%s, (%u,%u), module=%u, Error=%u\r\n", __func__,
		Loc.Col, Loc.Row, Module, Error);
	TilePtr = AieInst->Tiles;
	TilePtr += Loc.Col * (AieInst->Config->NumRows + 1) + Loc.Row;
	RegVal = XAieGbl_Read32(TilePtr->TileAddr + 0x12124);
	if ((RegVal & (1 << 15)) == 0) {
		printf("No parity error has occurred .\n");
	} else {
		u8 Bank;
		u8 ErrorAddr;

		ErrorAddr = RegVal & 0x7FFF;
		Bank = ErrorAddr / 0x1000;
		ParirtyErrorsCount[Loc.Col][Loc.Row - 1][Bank]++;
		XAieGbl_Write32(TilePtr->TileAddr + 0x12124, (1 << 15));
		if (ParirtyErrorsCount[Loc.Col][Loc.Row - 1][Bank] > 16) {
			/* This will kill the application */
			return XAIETILE_ERROR_NOTHANDLED;
		}
	}
	return XAIETILE_ERROR_HANDLED;
}

static void
_XAie_SRSEventCallback(struct XAieGbl *AieInst, XAie_LocType Loc, u8 Module,
		       u8 Event, void *Arg)
{
	XAieGbl_Tile *TilePtr;

	(void)Arg;
	printf("%s, (%u,%u), module=%u, Event=%u\r\n", __func__,
		Loc.Col, Loc.Row, Module, Event);
	/* Update coefficient */
	TilePtr = AieInst->Tiles;
	TilePtr += Loc.Col * (AieInst->Config->NumRows + 1) + Loc.Row;
	XAieTile_DmWriteWord(TilePtr, 0, 0xdeadbeef);
}


static XAieGbl_ErrorHandleStatus
_XAie_ECC2BitErrorCallback(struct XAieGbl *AieInst, XAie_LocType Loc,
			   u8 Module, u8 Error, void *Arg)
{
	u32 RegVal;
	XAieGbl_Tile *TilePtr;

	(void)Arg;
	printf("%s, (%u,%u), module=%u, Error=%u\r\n", __func__,
		Loc.Col, Loc.Row, Module, Error);
	TilePtr = AieInst->Tiles;
	TilePtr += Loc.Col * (AieInst->Config->NumRows + 1) + Loc.Row;
	RegVal = XAieGbl_Read32(TilePtr->TileAddr + 0x32120);
	if ((RegVal & (1 << 15)) == 0) {
		printf("No ECC error has occurred.\r\n");
	} else {
		u32 ErrorAddr;

		ErrorAddr = RegVal & 0x3FFF;
		if (ErrorAddr >= 0x2000 && ErrorAddr < 0x4000) {
			XAieTile_DmWriteWord(TilePtr, ErrorAddr, 0xdeadbeef);
		}
	}
	return XAIETILE_ERROR_HANDLED;
}

static XAieGbl_ErrorHandleStatus
_XAie_InstrErrorCallback(struct XAieGbl *AieInst, XAie_LocType Loc,
			 u8 Module, u8 Error, void *Arg)
{
	(void)AieInst;
	(void)Arg;
	printf("%s, (%u,%u), module=%u, Error=%u\r\n", __func__,
		Loc.Col, Loc.Row, Module, Error);
	return XAIETILE_ERROR_HANDLED;
}


static XAieGbl_ErrorHandleStatus
_XAie_DecodeErrorCallback(struct XAieGbl *AieInst, XAie_LocType Loc,
			 u8 Module, u8 Error, void *Arg)
{
	(void)AieInst;
	(void)Arg;
	printf("%s, (%u,%u), module=%u, Error=%u\r\n", __func__,
		Loc.Col, Loc.Row, Module, Error);
	return XAIETILE_ERROR_NOTHANDLED;
}

static XAieGbl_ErrorHandleStatus
_XAie_ErrorCallback(struct XAieGbl *AieInst, XAie_LocType Loc,
		    u8 Module, u8 Error, void *Arg)
{
	(void)AieInst;
	(void)Arg;
	if (Module == XAIEGBL_MODULE_CORE) {
		if (Error == XAIETILE_EVENT_CORE_DM_ACCESS_TO_UNAVAILABLE) {
			CoreDMUnavailErrors[Loc.Col][Loc.Row - 1] += 1;
		}
	} else if (Module == XAIEGBL_MODULE_MEM) {
		if (Error == XAIETILE_EVENT_MEM_DMA_S2MM_0_ERROR) {
			MemDmaS2MM0Errors[Loc.Col][Loc.Row - 1] += 1;
		}
	} else {
		if (Error == XAIETILE_EVENT_SHIM_CONTROL_PKT_ERROR) {
			ShimCntrPktErrors[Loc.Col] += 1;
		}
	}
	return XAIETILE_ERROR_HANDLED;
}

/*****************************************************************************/
/**
*
* This is the test error to kill the application.
*
* @param	None.
*
* @return	0 for success, and negative value for failure.

* @note		None.
*
*******************************************************************************/
static int test_kill_error(void)
{
	printf("***********************\r\n");
	printf("* Test error to kill application\r\n");
	printf("***********************\r\n");
	/* Enable AXI Default NPI interrupt routing */
	printf("Close logging file.\n");
	XAieLib_CloseLogFile();
	/* Wait for pending errors to finish */
	XAieTile_EventsWaitForPending(&AieInst);
	/* Unregister the error handler first before registering for another handler */
	XAieTile_ErrorUnregisterNotification(&AieInst, XAIEGBL_MODULE_PL,
					     XAIETILE_EVENT_SHIM_AXI_MM_DECODE_NSU_ERROR_NOC, XAIE_ENABLE);
	XAieTile_ErrorRegisterNotification(&AieInst, XAIEGBL_MODULE_PL,
					   XAIETILE_EVENT_SHIM_AXI_MM_DECODE_NSU_ERROR_NOC,
					   _XAie_DecodeErrorCallback, NULL);
	XAieTile_EventsEnableInterrupt(&AieInst);
	XAieTilePl_EventGenerate(&TileInst[47][0], XAIETILE_EVENT_SHIM_AXI_MM_DECODE_NSU_ERROR_NOC);
	usleep(500);
	return 0;
}

static int test_all_error(void)
{
	const char *LogFile = "xaie.log";
	int ret;

	printf("***********************\r\n");
	printf("* Test errors for all cores\r\n");
	printf("***********************\r\n");
	/* Wait for pending errors to finish */
	XAieTile_EventsWaitForPending(&AieInst);
	XAieTile_EventsDisableInterrupt(&AieInst);
	/* Set log file */
	printf("Set logging file.\n");
	ret = (int)XAieLib_OpenLogFile(LogFile);
	if (ret != XAIE_SUCCESS) {
		fprintf(stderr, "Failed to open log file.\n");
	}

	XAieTile_ErrorRegisterNotification(&AieInst, XAIEGBL_MODULE_ALL,
					   XAIETILE_ERROR_ALL,
					   _XAie_ErrorCallback, NULL);
	XAieTile_EventsEnableInterrupt(&AieInst);
	for (u32 c = 0; c < XAIE_NUM_COLS; c++) {
		for (u32 r = 1; r <= XAIE_NUM_ROWS; r++) {
			XAieTileMem_EventGenerate(&TileInst[c][r],
						  XAIETILE_EVENT_MEM_DMA_S2MM_0_ERROR);
			XAieTileCore_EventGenerate(&TileInst[c][r],
						   XAIETILE_EVENT_CORE_DM_ACCESS_TO_UNAVAILABLE);
		}
	}
	for (u32 c = 0; c < XAIE_NUM_COLS; c++) {
		XAieTilePl_EventGenerate(&TileInst[c][0], XAIETILE_EVENT_SHIM_CONTROL_PKT_ERROR);
	}
	/* Wait until all the errors have been handled */
	for (u32 i = 0; i < 10; i++) {
		for (u32 c = 0; c < XAIE_NUM_COLS; c++) {
			for (u32 r = 1; r <= XAIE_NUM_ROWS; r++) {
				if (MemDmaS2MM0Errors[c][r-1] != 1) {
					sleep(1);
					continue;
				}
				if (CoreDMUnavailErrors[c][r-1] != 1) {
					sleep(1);
					continue;
				}
			}
		}
		for (u32 c = 0; c < XAIE_NUM_COLS; c++) {
			if (ShimCntrPktErrors[c] != 1) {
				sleep(1);
				continue;
			}
		}
	}
	for (u32 c = 0; c < XAIE_NUM_COLS; c++) {
		for (u32 r = 1; r <= XAIE_NUM_ROWS; r++) {
			if (MemDmaS2MM0Errors[c][r-1] != 1) {
				fprintf(stderr,
					"Mem (%u,%u) error is not captured.\n",
					c, r);
			}
			if (CoreDMUnavailErrors[c][r-1] != 1) {
				fprintf(stderr,
					"Core (%u,%u) error is not captured.\n",
					c, r);
			}
		}
	}
	for (u32 c = 0; c < XAIE_NUM_COLS; c++) {
		if (ShimCntrPktErrors[c] != 1) {
			fprintf(stderr,
				"Shim (%u,0) error is not captured.\n", c);
		}
	}
	return 0;
}

/*****************************************************************************/
/**
*
* This is the test to register for event handlers.
*
* @param	None.
*
* @return	0 for success, and negative value for failure.

* @note		None.
*
*******************************************************************************/
static int test_custom_event_handlers(void)
{
	XAie_LocType Loc[2];

	printf("***********************\r\n");
	printf("* Test customised events handling\r\n");
	printf("***********************\r\n");
	/* Register events */
	printf("Register events\n");
	Loc[0].Col = 6;
	Loc[0].Row = 1;
	Loc[1].Col = 7;
	Loc[1].Row = 2;
	/* Wait for pending errors to finish */
	XAieTile_EventsWaitForPending(&AieInst);
	XAieTile_EventsDisableInterrupt(&AieInst);
	XAieTile_EventRegisterNotification(&AieInst, Loc, 2, XAIEGBL_MODULE_CORE,
					   XAIETILE_EVENT_CORE_PERF_CNT0,
					   _XAie_SRSEventCallback, NULL);
	XAieTile_EventsEnableInterrupt(&AieInst);
	/* Enable AIE interrupts */
	printf("Enaling interrupts.\n");
	XAieTile_EventsEnableInterrupt(&AieInst);
	/* Generate events */
	printf("Generate events\n");
	/* Configure counter to count the SRS errors */
	XAieTileCore_PerfCounterSet(&TileInst[6][1], 0, 0);
	XAieTileCore_PerfCounterControl(&TileInst[6][1], 0, XAIETILE_EVENT_CORE_SRS_SATURATE,
					XAIETILE_EVENT_CORE_SRS_SATURATE, 0);
	XAieTileCore_PerfCounterEventValue(&TileInst[6][1], 0, 8);
	XAieTileCore_PerfCounterSet(&TileInst[7][2], 0, 0);
	XAieTileCore_PerfCounterControl(&TileInst[7][2], 0, XAIETILE_EVENT_CORE_SRS_SATURATE,
					XAIETILE_EVENT_CORE_SRS_SATURATE, 0);
	XAieTileCore_PerfCounterEventValue(&TileInst[7][2], 0, 8);

	for (u32 i = 0; i < 10; i++) {
		XAieTileCore_EventGenerate(&TileInst[6][1], XAIETILE_EVENT_CORE_SRS_SATURATE);
		XAieTileCore_EventGenerate(&TileInst[7][2], XAIETILE_EVENT_CORE_SRS_SATURATE);
	}
	usleep(500);
	/* Unregister handlers */
	XAieTile_EventUnregisterNotification(&AieInst, Loc, 2, XAIEGBL_MODULE_CORE,
					     XAIETILE_EVENTS_ALL);
	return 0;
}

/*****************************************************************************/
/**
*
* This is the test to register for customised error handlers.
*
* @param	None.
*
* @return	0 for success, and negative value for failure.

* @note		None.
*
*******************************************************************************/
static int test_custom_error_handlers(void)
{
	int ret;

	printf("***********************\r\n");
	printf("* Test customised errors handling\r\n");
	printf("***********************\r\n");
	/* Register for error handlers */
	XAieTile_ErrorRegisterNotification(&AieInst, XAIEGBL_MODULE_MEM,
					   XAIETILE_EVENT_MEM_DM_PARITY_ERROR_BANK_2,
					   _XAie_ParityErrorCallback, NULL);
	XAieTile_ErrorRegisterNotification(&AieInst, XAIEGBL_MODULE_MEM,
					   XAIETILE_EVENT_MEM_DM_PARITY_ERROR_BANK_3,
					   _XAie_ParityErrorCallback, NULL);
	XAieTile_ErrorRegisterNotification(&AieInst, XAIEGBL_MODULE_MEM,
					   XAIETILE_EVENT_MEM_DM_PARITY_ERROR_BANK_4,
					   _XAie_ParityErrorCallback, NULL);
	XAieTile_ErrorRegisterNotification(&AieInst, XAIEGBL_MODULE_MEM,
					   XAIETILE_EVENT_MEM_DM_PARITY_ERROR_BANK_5,
					   _XAie_ParityErrorCallback, NULL);
	XAieTile_ErrorRegisterNotification(&AieInst, XAIEGBL_MODULE_MEM,
					   XAIETILE_EVENT_MEM_DM_PARITY_ERROR_BANK_6,
					   _XAie_ParityErrorCallback, NULL);
	XAieTile_ErrorRegisterNotification(&AieInst, XAIEGBL_MODULE_MEM,
					   XAIETILE_EVENT_MEM_DM_PARITY_ERROR_BANK_7,
					   _XAie_ParityErrorCallback, NULL);
	XAieTile_ErrorRegisterNotification(&AieInst, XAIEGBL_MODULE_MEM,
					   XAIETILE_EVENT_MEM_DM_ECC_ERROR_2BIT,
					   _XAie_ECC2BitErrorCallback, NULL);
	/* Setup routing */
	printf("Initialize events routing\r\n");
	ret = XAieTile_EventsHandlingInitialize(&AieInst);
	if (ret != XAIE_SUCCESS) {
		fprintf(stderr, "ERROR: failed to initialize Events Handling.\n");
		XAieLib_CloseLogFile();
		return -1;
	}

	/* Generate errors */
	printf("Generating errors\r\n");
	XAieTileMem_EventGenerate(&TileInst[4][3], XAIETILE_EVENT_MEM_DM_PARITY_ERROR_BANK_3);
	XAieTileMem_EventGenerate(&TileInst[5][2], XAIETILE_EVENT_MEM_DM_ECC_ERROR_2BIT);
	usleep(500);

	/* Register notification for errors which are not generate interrupt
	 * by default. This will need to be done after the events handling
	 * is initialized.
	 */
	XAieTile_EventsDisableInterrupt(&AieInst);
	XAieTile_ErrorRegisterNotification(&AieInst, XAIEGBL_MODULE_CORE,
					   XAIETILE_EVENT_CORE_INSTR_ERROR,
					   _XAie_InstrErrorCallback, NULL);
	XAieTile_EventsEnableInterrupt(&AieInst);
	XAieTileCore_EventGenerate(&TileInst[7][3], XAIETILE_EVENT_CORE_INSTR_ERROR);
	XAieTileCore_EventGenerate(&TileInst[8][3], XAIETILE_EVENT_CORE_INSTR_ERROR);
	XAieTileCore_EventGenerate(&TileInst[8][8], XAIETILE_EVENT_CORE_INSTR_ERROR);
	XAieTileCore_EventGenerate(&TileInst[48][8], XAIETILE_EVENT_CORE_INSTR_ERROR);
	XAieTileCore_EventGenerate(&TileInst[49][8], XAIETILE_EVENT_CORE_INSTR_ERROR);
	usleep(500);

	XAieTile_ErrorUnregisterNotification(&AieInst, XAIEGBL_MODULE_MEM,
					     XAIETILE_EVENT_MEM_DM_PARITY_ERROR_BANK_2, XAIE_ENABLE);
	XAieTile_ErrorUnregisterNotification(&AieInst, XAIEGBL_MODULE_MEM,
					     XAIETILE_EVENT_MEM_DM_PARITY_ERROR_BANK_3, XAIE_ENABLE);
	XAieTile_ErrorUnregisterNotification(&AieInst, XAIEGBL_MODULE_MEM,
					     XAIETILE_EVENT_MEM_DM_PARITY_ERROR_BANK_4, XAIE_ENABLE);
	XAieTile_ErrorUnregisterNotification(&AieInst, XAIEGBL_MODULE_MEM,
					     XAIETILE_EVENT_MEM_DM_PARITY_ERROR_BANK_5, XAIE_ENABLE);
	XAieTile_ErrorUnregisterNotification(&AieInst, XAIEGBL_MODULE_MEM,
					     XAIETILE_EVENT_MEM_DM_PARITY_ERROR_BANK_6, XAIE_ENABLE);
	XAieTile_ErrorUnregisterNotification(&AieInst, XAIEGBL_MODULE_MEM,
					     XAIETILE_EVENT_MEM_DM_PARITY_ERROR_BANK_7, XAIE_ENABLE);
	XAieTile_ErrorUnregisterNotification(&AieInst, XAIEGBL_MODULE_MEM,
					     XAIETILE_EVENT_MEM_DM_ECC_ERROR_2BIT, XAIE_ENABLE);
	XAieTile_ErrorUnregisterNotification(&AieInst, XAIEGBL_MODULE_CORE,
					     XAIETILE_EVENT_CORE_INSTR_ERROR, XAIE_ENABLE);
	return 0;
}

/*****************************************************************************/
/**
*
* This is the main entry point for the AIE RTS driver test.
*
* @param	None.
*
* @return	None.

* @note		None.
*
*******************************************************************************/
int main(void)
{
	int ret;

#ifdef __AIEBAREMTL__
	init_irq();
#endif

	printf("*************************************\n"
	       " XAIE Events Testing.\n"
	       "*************************************\n");
        /* Initialize AIE Instance */
	XAIEGBL_HWCFG_SET_CONFIG((&AieConfig), XAIE_NUM_ROWS, XAIE_NUM_COLS, XAIE_ADDR_ARRAY_OFF);
        XAieGbl_HwInit(&AieConfig);

	AieConfigPtr = XAieGbl_LookupConfig(XPAR_AIE_DEVICE_ID);
	(void)XAieGbl_CfgInitialize(&AieInst, &TileInst[0][0], AieConfigPtr);

	XAieLib_NpiAieArrayReset(XAIE_RESETENABLE);
	usleep(500);
	XAieLib_NpiAieArrayReset(XAIE_RESETDISABLE);
	for(int c = 0; c < XAIE_NUM_COLS; c++) {
		XAieTile_ShimColumnReset(&(TileInst[c][0]), XAIE_RESETENABLE);
		XAieTile_ShimColumnReset(&(TileInst[c][0]), XAIE_RESETDISABLE);
	}

	ret = test_custom_error_handlers();
	if (ret < 0) {
		return ret;
	}
	ret = test_custom_event_handlers();
	if (ret < 0) {
		return ret;
	}
	ret = test_all_error();
	if (ret < 0) {
		return ret;
	}
	ret = test_kill_error();
	if (ret < 0) {
		return ret;
	}
	return 0;
}

/** @} */
