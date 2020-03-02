/******************************************************************************
*
* Copyright (C) 2020 Xilinx, Inc.  All rights reserved.
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
* THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMANGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
* THE SOFTWARE.
*
*
*
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

#ifndef __AIEBAREMTL__
/************************** Constant Definitions *****************************/
#define XAIE_NUM_ROWS		8
#define XAIE_NUM_COLS		50
#define XAIE_ADDR_ARRAY_OFF	0x800

#define EVENTS_NUM_ROWS	4
#define EVENTS_NUM_COLS	4

/************************** Variable Definitions *****************************/
static XAieGbl_Config *AieConfigPtr;	/**< AIE configuration pointer */
static XAieGbl AieInst;			/**< AIE global instance */
static XAieGbl_HwCfg AieConfig;          /**< AIE HW configuration instance */

static XAieGbl_Tile TileInst[XAIE_NUM_COLS][XAIE_NUM_ROWS+1];

static u8 ParirtyErrorsCount[XAIE_NUM_COLS][XAIE_NUM_ROWS][8];

static u8 MemDmaS2MM0Errors[XAIE_NUM_COLS][XAIE_NUM_ROWS];
static u8 CoreDMUnavailErrors[XAIE_NUM_COLS][XAIE_NUM_ROWS];
static u8 ShimCntrPktErrors[XAIE_NUM_COLS];

/************************** Function Prototypes  *****************************/

/************************** Function Definitions *****************************/
static XAieGbl_ErrorHandleStatus
_XAie_ParityErrorCallback(struct XAieGbl *AieInst, XAie_LocType Loc,
			  u8 Module, u8 Error, void *Arg)
{
	u32 RegVal;
	XAieGbl_Tile *TilePtr;

	(void)Arg;
	printf("%s, (%u,%u), module=%u, Error=%u\n", __func__,
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
	printf("%s, (%u,%u), module=%u, Event=%u\n", __func__,
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
	printf("%s, (%u,%u), module=%u, Error=%u\n", __func__,
		Loc.Col, Loc.Row, Module, Error);
	TilePtr = AieInst->Tiles;
	TilePtr += Loc.Col * (AieInst->Config->NumRows + 1) + Loc.Row;
	RegVal = XAieGbl_Read32(TilePtr->TileAddr + 0x32120);
	if ((RegVal & (1 << 15)) == 0) {
		printf("No ECC error has occurred .\n");
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
	printf("%s, (%u,%u), module=%u, Error=%u\n", __func__,
		Loc.Col, Loc.Row, Module, Error);
	return XAIETILE_ERROR_HANDLED;
}


static XAieGbl_ErrorHandleStatus
_XAie_DecodeErrorCallback(struct XAieGbl *AieInst, XAie_LocType Loc,
			 u8 Module, u8 Error, void *Arg)
{
	(void)AieInst;
	(void)Arg;
	printf("%s, (%u,%u), module=%u, Error=%u\n", __func__,
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
	u32 RegVal;
	XAie_LocType Loc[2];
	const char *LogFile = "xaie.log";

	printf("*************************************\n"
	       " XAIE Events Testing.\n"
	       "*************************************\n");
        /* Initialize AIE Instance */
	XAIEGBL_HWCFG_SET_CONFIG((&AieConfig), XAIE_NUM_ROWS, XAIE_NUM_COLS, XAIE_ADDR_ARRAY_OFF);
        XAieGbl_HwInit(&AieConfig);

	AieConfigPtr = XAieGbl_LookupConfig(XPAR_AIE_DEVICE_ID);
	(void)XAieGbl_CfgInitialize(&AieInst, &TileInst[0][0], AieConfigPtr);

	for(int c = 0; c < XAIE_NUM_COLS; c++) {
		XAieTile_ShimColumnReset(&(TileInst[c][0]), XAIE_RESETENABLE);
		XAieTile_ShimColumnReset(&(TileInst[c][0]), XAIE_RESETDISABLE);
	}
	RegVal = XAieGbl_NPIRead32(XAIE_NPI_ISR);
	XAieGbl_NPIWrite32(XAIE_NPI_ISR, RegVal);

	/* Initialize the boardcasting events */
	/* Enable AXI Default NPI interrupt routing */
	printf("Set logging file.\n");
	ret = (int)XAieLib_OpenLogFile(LogFile);
	if (ret != XAIE_SUCCESS) {
		fprintf(stderr, "Failed to ope log file.\n");
	}

	printf("Initialize events routing\n");
	ret = XAieTile_EventsHandlingInitialize(&AieInst);
	if (ret != XAIE_SUCCESS) {
		fprintf(stderr, "ERROR: failed to initialize Events Handling.\n");
		XAieLib_CloseLogFile();
		return -1;
	}
	/* Register events */
	printf("Register events\n");
	Loc[0].Col = 6;
	Loc[0].Row = 1;
	Loc[1].Col = 7;
	Loc[1].Row = 2;
	XAieTile_EventRegisterNotification(&AieInst, Loc, 2, XAIEGBL_MODULE_CORE,
					   XAIETILE_EVENT_CORE_PERF_CNT0,
					   _XAie_SRSEventCallback, NULL);
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
	XAieTile_ErrorRegisterNotification(&AieInst, XAIEGBL_MODULE_CORE,
					   XAIETILE_EVENT_CORE_INSTR_ERROR,
					   _XAie_InstrErrorCallback, NULL);

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
	XAieTile_EventsWaitForPending(&AieInst);
	XAieTileMem_EventGenerate(&TileInst[4][3], XAIETILE_EVENT_MEM_DM_PARITY_ERROR_BANK_3);
	XAieTileMem_EventGenerate(&TileInst[5][2], XAIETILE_EVENT_MEM_DM_ECC_ERROR_2BIT);
	XAieTileCore_EventGenerate(&TileInst[7][3], XAIETILE_EVENT_CORE_INSTR_ERROR);
	XAieTileCore_EventGenerate(&TileInst[8][3], XAIETILE_EVENT_CORE_INSTR_ERROR);
	XAieTileCore_EventGenerate(&TileInst[8][8], XAIETILE_EVENT_CORE_INSTR_ERROR);
	XAieTileCore_EventGenerate(&TileInst[48][8], XAIETILE_EVENT_CORE_INSTR_ERROR);
	XAieTileCore_EventGenerate(&TileInst[49][8], XAIETILE_EVENT_CORE_INSTR_ERROR);
	/* Wait untile all the errors have been handled */
	XAieTile_EventsWaitForPending(&AieInst);

	/* Unregister handlers */
	XAieTile_EventUnregisterNotification(&AieInst, Loc, 2, XAIEGBL_MODULE_CORE,
					     XAIETILE_EVENTS_ALL);
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
	XAieTile_ErrorUnregisterNotification(&AieInst, XAIEGBL_MODULE_CORE,
					     XAIETILE_EVENT_CORE_DM_ACCESS_TO_UNAVAILABLE, XAIE_ENABLE);
	XAieTile_ErrorUnregisterNotification(&AieInst, XAIEGBL_MODULE_MEM,
					     XAIETILE_EVENT_MEM_DMA_S2MM_0_ERROR, XAIE_ENABLE);
	XAieTile_ErrorUnregisterNotification(&AieInst, XAIEGBL_MODULE_PL,
					     XAIETILE_EVENT_SHIM_CONTROL_PKT_ERROR, XAIE_ENABLE);

	printf("Test All Cores Errors.\n");
	XAieTile_ErrorRegisterNotification(&AieInst, XAIEGBL_MODULE_ALL,
					   XAIETILE_ERROR_ALL,
					   _XAie_ErrorCallback, NULL);
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
	/* Wait untile all the errors have been handled */
	XAieTile_EventsWaitForPending(&AieInst);
	XAieTile_EventsDisableInterrupt(&AieInst);
	/* Unregister events */

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
	printf("Test Kill app.\n");
	/* Unregister the error handler first before registering for another handler */
	XAieTile_ErrorUnregisterNotification(&AieInst, XAIEGBL_MODULE_PL,
					     XAIETILE_EVENT_SHIM_AXI_MM_DECODE_NSU_ERROR_NOC, XAIE_ENABLE);
	XAieTile_ErrorRegisterNotification(&AieInst, XAIEGBL_MODULE_PL,
					   XAIETILE_EVENT_SHIM_AXI_MM_DECODE_NSU_ERROR_NOC,
					   _XAie_DecodeErrorCallback, NULL);
	XAieTile_EventsEnableInterrupt(&AieInst);
	XAieTilePl_EventGenerate(&TileInst[47][0], XAIETILE_EVENT_SHIM_AXI_MM_DECODE_NSU_ERROR_NOC);
	XAieTile_EventsWaitForPending(&AieInst);
	return 0;
}
#endif /* not __AIEBAREMTL__ */

/** @} */
