/******************************************************************************
* Copyright (c) 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022-2023, Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file	xilocp_client_pcr_example.c
* @addtogroup xocp_example_apis XilOcp API Example Usage
* @{
* This example illustrates the extension and get PCR by requesting ROM services.
* To build this application, xilmailbox library must be included in BSP and
* xilocp must be in client mode.
*
* @note
* Procedure to link and compile the example for the default ddr less designs
* ------------------------------------------------------------------------------------------------------------
* The default linker settings places a software stack, heap and data in DDR memory. For this example to work,
* any data shared between client running on A78/R52/PL and server running on PMC, should be placed in area
* which is accessible to both client and server.
*
* Following is the procedure to compile the example on OCM or any memory region which can be accessed by server
*
*		1. Open example linker script(lscript.ld) in Vitis project and section to memory mapping should
*			be updated to point all the required sections to shared memory(OCM or TCM)
*			using a memory region drop down selection
*
*						OR
*
*		1. In linker script(lscript.ld) user can add new memory section in source tab as shown below
*			.sharedmemory : {
*   			. = ALIGN(4);
*   			__sharedmemory_start = .;
*   			*(.sharedmemory)
*   			*(.sharedmemory.*)
*   			*(.gnu.linkonce.d.*)
*   			__sharedmemory_end = .;
* 			} > psu_ddr_0_MEM_0
*
* 		2. In this example ".data" section elements that are passed by reference to the server-side should
* 		   be stored in the above shared memory section. To make it happen in below example,
*		   replace ".data" in attribute section with ".sharedmemory". For example,
* 		   static u8 PcrBuf[XOCP_PCR_SIZE_BYTES] __attribute__ ((section (".data.PcrBuf")));
* 					should be changed to
* 		   static u8 PcrBuf[XOCP_PCR_SIZE_BYTES] __attribute__ ((section (".sharedmemory.PcrBuf")));
* 		   static u8 ExtendHash[XOCP_PCR_SIZE_BYTES] __attribute__ ((section (".data.ExtendHash")));
* 					should be changed to
* 		   static u8 ExtendHash[XOCP_PCR_SIZE_BYTES] __attribute__ ((section (".sharedmemory.ExtendHash")));
*
* To keep things simple, by default the cache is disabled for this example
*
* User configurable parameters for OCP
* ExtendHash[XOCP_EXTENDED_HASH_SIZE_IN_BYTES] can be configured with a 48 byte hash
* XOCP_SELECT_PCR_NUM	(XOCP_PCR_2)
* XOCP_SELECT_PCR_NUM can be configured as one of the seven provided PCR
* number from XOcp_HwPcr enum in xocp_common.h file.
*
* XOCP_READ_PCR_MASK	(0x00000004)
* The lower 8 bits of XOCP_READ_PCR_MASK indicates 8 PCRs, user can set
* the corresponding bit to read the specific PCR
* Example: XOCP_READ_PCR_MASK (0x00000004) , 2nd bit of the Mask is set means
* user want to read the PCR 2
* XOCP_READ_PCR_MASK (0x0000000B), 2nd and 3rd bitsof the Mask are set, which
* means user wants to read PCR 2 and PCR 3.
* Default value is (0x00000004)
*
* XOCP_READ_NUM_OF_LOG_ENTRIES Number of PcrLog entries to read into buffer.
* Default value is 1 and maximum value is upto 32.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who    Date     Changes
* ----- ------ -------- -------------------------------------------------
* 1.1   am     12/21/22 Initial release
*       am     01/10/23 Added configurable cache disable support
*       kal    02/01/23 Moved configurable parameters from input.h file to
*                       this file.
* 1.2   kal    05/28/23 Adder SW PCR extend and logging functions
*
* </pre>
* @note
*
******************************************************************************/

/***************************** Include Files *********************************/
#include "xocp_client.h"
#include "xil_util.h"
#include "xil_cache.h"
#include "xparameters.h"

/************************** Constant Definitions *****************************/
#define XOCP_WORD_LEN		(4U)

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/
#define XOCP_SELECT_PCR_NUM	(XOCP_PCR_2)

#define XOCP_READ_PCR_MASK	(0x00000004U)

#define XOCP_READ_NUM_OF_LOG_ENTRIES	(1U)

#define XOCP_NUM_SW_PCR_LOG_EVENTS	(4U)

/************************** Function Prototypes ******************************/
static void XOcp_PrintData(const u8 *Data, u32 size);
static int XOcp_HwPcrExample(XOcp_ClientInstance *ClientInstancePtr);
static int XOcp_SwPcrExample(XOcp_ClientInstance *ClientInstancePtr);

/************************** Variable Definitions *****************************/
static u8 PcrBuf[XOCP_PCR_SIZE_BYTES] __attribute__ ((section (".data.PcrBuf")));
static XOcp_HwPcrEvent   HwPcrEvents[XOCP_READ_NUM_OF_LOG_ENTRIES] __attribute__ ((section (".data.HwPcrEvents")));
static XOcp_HwPcrLogInfo HwPcrLogInfo __attribute__ ((section (".data.HwPcrLogInfo")));
static XOcp_PcrMeasurement SwPcrMeasurement[XOCP_NUM_SW_PCR_LOG_EVENTS] __attribute__ ((section (".data.SwPcrMeasurement")));
static u8 ExtendHash[XOCP_EXTENDED_HASH_SIZE_IN_BYTES]__attribute__ ((section (".data.ExtendHash")))=
						{0x70,0x69,0x77,0x35,0x0b,0x93,
						0x92,0xa0,0x48,0x2c,0xd8,0x23,
						0x38,0x47,0xd2,0xd9,0x2d,0x1a,
						0x95,0x0c,0xad,0xa8,0x60,0xc0,
						0x9b,0x70,0xc6,0xad,0x6e,0xf1,
						0x5d,0x49,0x68,0xa3,0x50,0x75,
						0x06,0xbb,0x0b,0x9b,0x03,0x7d,
						0xd5,0x93,0x76,0x50,0xdb,0xd4};
static XOcp_SwPcrExtendParams ExtendParams __attribute__ ((section (".data.ExtendParams")));
static XOcp_SwPcrLogReadData LogParams __attribute__ ((section (".data.LogParams")));
static XOcp_SwPcrReadData DataParams __attribute__ ((section (".data.DataParams")));

/*****************************************************************************/
/**
* @brief   Main function to call the OCP functions to extend and get PCR for
*          requesting ROM services.
*
* @return
*          - XST_SUCCESS - On success
*          - Errorcode - On failure
*
******************************************************************************/
int main(void)
{
	int Status = XST_FAILURE;
	XMailbox MailboxInstance;
	XOcp_ClientInstance OcpClientInstance;

#ifdef XOCP_CACHE_DISABLE
	Xil_DCacheDisable();
#endif

	Status = XMailbox_Initialize(&MailboxInstance, 0U);
	if (Status != XST_SUCCESS) {
		xil_printf("Mailbox initialize failed:%08x \r\n", Status);
		goto END;
	}

	Status = XOcp_ClientInit(&OcpClientInstance, &MailboxInstance);
	if (Status != XST_SUCCESS) {
		xil_printf("Client initialize failed:%08x \r\n", Status);
		goto END;
	}

#ifndef XOCP_CACHE_DISABLE
	Xil_DCacheInvalidateRange((UINTPTR)ExtendHash, XOCP_EXTENDED_HASH_SIZE_IN_BYTES);
	Xil_DCacheInvalidateRange((UINTPTR)PcrBuf, XOCP_PCR_SIZE_BYTES);
	Xil_DCacheInvalidateRange((UINTPTR)HwPcrEvents, sizeof(HwPcrEvents));
	Xil_DCacheInvalidateRange((UINTPTR)HwPcrLogInfo, sizeof(HwPcrLogInfo));
	Xil_DCacheInvalidateRange((UINTPTR)SwPcrMeasurement, sizeof(SwPcrMeasurement));
#endif

	Status = XOcp_HwPcrExample(&OcpClientInstance);
	if (Status != XST_SUCCESS) {
		xil_printf("HW PCR example failed with Status:%08x \r\n", Status);
		goto END;
	}

	Status = XOcp_SwPcrExample(&OcpClientInstance);
	if (Status != XST_SUCCESS) {
		xil_printf("SW PCR example failed with Status:%08x \r\n", Status);
		goto END;
	}
	xil_printf("\r\n Successfully ran OCP Client Example");
	Status = XST_SUCCESS;

END:
	return Status;
}

/****************************************************************************/
/**
* @brief   This function explains how to use HW PCR client APIs
*
* @param   ClientInstancePtr	Pointer to the OCP client instance
*
* @return  XST_SUCCESS	Upon Success
* 	   ErrorCode 	Upon Failure
*
****************************************************************************/
static int XOcp_HwPcrExample(XOcp_ClientInstance *ClientInstancePtr)
{
	int Status = XST_FAILURE;
	u32 PcrNum = (u32)XOCP_SELECT_PCR_NUM;
	u32 PcrMask = (u32)XOCP_READ_PCR_MASK;
	u8 Index;

	Status = XOcp_ExtendHwPcr(ClientInstancePtr, PcrNum,
		(u64)(UINTPTR)ExtendHash, sizeof(ExtendHash));
	if (Status != XST_SUCCESS) {
		xil_printf("Extend HW PCR failed Status: 0x%02x\n\r", Status);
		goto END;
	}

	Status = XOcp_GetHwPcr(ClientInstancePtr, PcrMask,
		(u64)(UINTPTR)PcrBuf, sizeof(PcrBuf));
	if (Status != XST_SUCCESS) {
		xil_printf("Get HW PCR failed Status: 0x%02x\n\r", Status);
		goto END;
	}

	xil_printf("Requested HW PCR contents:\n\r");
	XOcp_PrintData((const u8*)PcrBuf, XOCP_PCR_SIZE_BYTES);

	Status = XOcp_GetHwPcrLog(ClientInstancePtr, (u64)(UINTPTR)HwPcrEvents,
				(u64)(UINTPTR)&HwPcrLogInfo, XOCP_READ_NUM_OF_LOG_ENTRIES);
	if (Status != XST_SUCCESS) {
		xil_printf("Get HW PCR Log failed Status: 0x%02x\n\r", Status);
		goto END;
    }

	if (HwPcrLogInfo.HwPcrEventsRead != 0U) {
		xil_printf("\n\rHW PCR Log contents:\n\r");
		for (Index = 0U; Index < HwPcrLogInfo.HwPcrEventsRead; Index++) {
			xil_printf("PCR Number: %x\r\n", HwPcrEvents[Index].PcrNo);
			xil_printf("Hash to be extended:\n\r");
			XOcp_PrintData((const u8*)HwPcrEvents[Index].Hash, XOCP_PCR_SIZE_BYTES);
			xil_printf("PCR Extended Value:\n\r");
			XOcp_PrintData((const u8*)HwPcrEvents[Index].PcrValue, XOCP_PCR_SIZE_BYTES);
			xil_printf("\n\r");
		}
	}
	xil_printf("\n\rHW PCR Log status:\n\r");
	xil_printf("No of requested HWPCR log events:%d \n\r",XOCP_READ_NUM_OF_LOG_ENTRIES);
	xil_printf("No of read HWPCR log events occured:%d \n\r",HwPcrLogInfo.HwPcrEventsRead);
	xil_printf("No of pending HWPCR read log events:%d \n\r",HwPcrLogInfo.RemainingHwPcrEvents);
	xil_printf("Total No of update HWPCR log events:%d \n\r",HwPcrLogInfo.TotalHwPcrLogEvents);
	xil_printf("HWPCR log overflow count since last read:%d \n\r",HwPcrLogInfo.OverflowCntSinceLastRd);

	Status = XST_SUCCESS;
END:
	return Status;
}

/****************************************************************************/
/**
* @brief   This function explains how to use SW PCR client APIs
*
* @param   ClientInstancePtr	Pointer to the OCP client instance
*
* @return  XST_SUCCESS	Upon Success
* 	   ErrorCode 	Upon Failure
*
****************************************************************************/
static int XOcp_SwPcrExample(XOcp_ClientInstance *ClientInstancePtr)
{
	int Status = XST_FAILURE;
	u32 PcrNum = (u32)XOCP_SELECT_PCR_NUM;
	u32 PcrMask = (u32)XOCP_READ_PCR_MASK;
	u32 MeasurementIdx = 0U;
	u8 Index;

	ExtendParams.PcrNum = PcrNum;
	ExtendParams.MeasurementIdx = MeasurementIdx;
	ExtendParams.DataSize = sizeof(ExtendHash);
	ExtendParams.OverWrite = FALSE;
	ExtendParams.DataAddr = (u64)(UINTPTR)ExtendHash;
	Status = XOcp_ExtendSwPcr(ClientInstancePtr, &ExtendParams);
	if (Status != XST_SUCCESS) {
		xil_printf("Extend SW PCR failed Status: 0x%02x\n\r", Status);
		goto END;
	}

	Status = XOcp_GetSwPcr(ClientInstancePtr, PcrMask,
		&PcrBuf[0U], sizeof(PcrBuf));
	if (Status != XST_SUCCESS) {
		xil_printf("Get SW PCR failed Status: 0x%02x\n\r", Status);
		goto END;
	}

	xil_printf("\r\nRequested SW PCR contents for PCR %x:\n\r", PcrNum);
	XOcp_PrintData((const u8*)PcrBuf, XOCP_PCR_SIZE_BYTES);

	DataParams.PcrNum = PcrNum;
	DataParams.MeasurementIdx = MeasurementIdx;
	DataParams.BufSize = sizeof(PcrBuf);
	DataParams.DataStartIdx = 0U;
	DataParams.BufAddr = (u64)(UINTPTR)PcrBuf;
	Status = XOcp_GetSwPcrData(ClientInstancePtr, &DataParams);
	if (Status != XST_SUCCESS) {
		xil_printf("Get SwPcrData failed Status: 0x%02x\n\r", Status);
		goto END;
	}

	xil_printf("Requested SW PCR Data:\n\r");
        XOcp_PrintData((const u8*)(UINTPTR)DataParams.BufAddr, DataParams.ReturnedBytes);

	LogParams.PcrNum = PcrNum;
	LogParams.LogSize = sizeof(SwPcrMeasurement);
	LogParams.PcrLogAddr = (u64)(UINTPTR)&SwPcrMeasurement;
	Status = XOcp_GetSwPcrLog(ClientInstancePtr, &LogParams);
	if (Status != XST_SUCCESS) {
                xil_printf("Get SW PCR Log failed Status: 0x%02x\n\r", Status);
                goto END;
        }

	xil_printf("\n\rSW PCR Log contents for PCR: %x\n\r", PcrNum);
	for (Index = 0U; Index < LogParams.DigestCount; Index++) {
		xil_printf("EventId: ");
		XOcp_PrintData((const u8*)&SwPcrMeasurement[Index].EventId, XOCP_EVENT_ID_NUM_OF_BYTES);
		xil_printf("Version: ");
		XOcp_PrintData((const u8*)&SwPcrMeasurement[Index].Version, XOCP_VERSION_NUM_OF_BYTES);
		xil_printf("DataLength: %x\r\n", SwPcrMeasurement[Index].DataLength);
		xil_printf("Hash Of Data:\n\r");
		XOcp_PrintData((const u8*)SwPcrMeasurement[Index].HashOfData,
				XOCP_PCR_SIZE_BYTES);
		xil_printf("Measurement:\n\r");
		XOcp_PrintData((const u8*)SwPcrMeasurement[Index].MeasuredData,
				 XOCP_PCR_SIZE_BYTES);

		xil_printf("\n\r");
	}

	Status = XST_SUCCESS;
END:
	return Status;
}
/****************************************************************************/
/**
* @brief   This function prints the given data on the console
*
* @param   Data - Pointer to any given data buffer
*
* @param   Size - Size of the given buffer
*
****************************************************************************/
static void XOcp_PrintData(const u8 *Data, u32 Size)
{
	u32 Index;

	for (Index = 0U; Index < Size; Index++) {
		xil_printf("%02x", Data[Index]);
	}
	xil_printf("\r\n");
 }
