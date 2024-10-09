/**************************************************************************************************
* Copyright (C) 2024 Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/

/*************************************************************************************************/
/**
 *
 * @file xplmi_configure_secure_comm_example.c
 *
 * This file illustrates the configuration of secure communication between SLRs
 * This command contains the SLR number and address of buffer which contains the IVs and key.
 *
 * To build this application, xilmailbox library must be included in BSP
 *
 * This example is supported for Versal devices.
 *
 * Procedure to run the example.
 * ------------------------------------------------------------------------------------------------
 * Load the Pdi.
 * Select the target.
 * Download the example elf into the target.
 *
 * Procedure to link and compile the example for the default ddr less designs
 * -------------------------------------------------------------------------------------------------
 * The default linker settings places a software stack, heap and data in DDR memory. For this
 * example to work, any data shared between client running on A72/R5/PL and server running on PMC,
 * should be placed in area which is accessible to both client and server.
 *
 * Following is the procedure to compile the example on OCM or any memory region which can be
 * accessed by server
 *
 * 1. Open example linker script(lscript.ld) in Vitis project and section to memory mapping should
 *	be updated to point all the required sections to shared memory(OCM or TCM)
 *	using a memory region drop down selection
 *
 * 						OR
 *
 * 1. In linker script(lscript.ld) user can add new memory section in source tab as shown below
 *			.sharedmemory : {
 *   			. = ALIGN(4);
 *   			__sharedmemory_start = .;
 *   			*(.sharedmemory)
 *   			*(.sharedmemory.*)
 *   			*(.gnu.linkonce.d.*)
 *   			__sharedmemory_end = .;
 * 			} > versal_cips_0_pspmc_0_psv_ocm_ram_0_psv_ocm_ram_0
 *
 * 2. In this example ".data" section elements that are passed by reference to the server-side
 *   should be stored in the above shared memory section. To make it happen in below example,
 *		   replace ".data" in attribute section with ".sharedmemory". For example,
 * 	static u8 Data __attribute__ ((aligned (64U)) __attribute__
 *   ((section (".data.SsitSecCommData"))); should be changed to
 * 	static u8 Data __attribute__ ((aligned (64U)) __attribute__
 *   ((section (".sharedmemory.SsitSecCommData")));
 *
 * To keep things simple, by default the cache is disabled for this example
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- ----------------------------------------------------------------------------
 * 1.00  pre  07/10/24 Initial release
 *       pre  09/30/24 Added comment which says only 12bytes of IV are being used
 *
 * </pre>
 *
 *************************************************************************************************/

/**
 * @addtogroup xplmi_client_example_apis XilPlmi Client Example APIs
 * @{
 */

/*************************************** Include Files *******************************************/

#ifdef SDT
#include "xplmi_bsp_config.h"
#endif
#include "xplmi_client.h"
#include "xil_util.h"
#include "xil_cache.h"
#include "xplmi_defs.h"
#include "xtrngpsv.h"
#include "xil_types.h"

/************************************ Constant Definitions ***************************************/
#define EXAMPLE_SEEDLIFE	        (3U) /**< Seed life value */
#define SLR_INDEX                   XPLMI_SLR_INDEX_1 /*< SLR number */

/************************************** Type Definitions *****************************************/

/*************************** Macros (Inline Functions) Definitions *******************************/

/************************************ Function Prototypes ****************************************/

/************************************ Variable Definitions ***************************************/
/* shared memory allocation must be done in OCM or TCM */
static XPlmi_SsitSecComm SsitSecCommData __attribute__((aligned(64U)))
						__attribute__((section (".data.SsitSecCommData")));

const u8 PersStr[XTRNGPSV_PERS_STR_LEN_BYTES] = {
		0xB2U, 0x80U, 0x7EU, 0x4CU, 0xD0U, 0xE4U, 0xE2U, 0xA9U,
		0x2FU, 0x1FU, 0x5DU, 0xC1U, 0xA2U, 0x1FU, 0x40U, 0xFCU,
		0x1FU, 0x24U, 0x5DU, 0x42U, 0x61U, 0x80U, 0xE6U, 0xE9U,
		0x71U, 0x05U, 0x17U, 0x5BU, 0xAFU, 0x70U, 0x30U, 0x18U,
		0xBCU, 0x23U, 0x18U, 0x15U, 0xCBU, 0xB8U, 0xA6U, 0x3EU,
		0x83U, 0xB8U, 0x4AU, 0xFEU, 0x38U, 0xFCU, 0x25U, 0x87U
};
XTrngpsv Trngpsv;/* Instance of TRNGPSV */

/*************************************************************************************************/
/**
 * @brief	Main function to call the config secure communication function.
 *
 * @return
 *			 - XST_SUCCESS on success.
 *			 - XST_FAILURE on failure.
 *
 *************************************************************************************************/
int main(void)
{
	int Status = XST_FAILURE;
	XMailbox MailboxInstance;
	XPlmi_ClientInstance PlmiClientInstance;
	XTrngpsv_Config *Config;
	XTrngpsv_UsrCfg UsrCfg = {
			.Mode = XTRNGPSV_HRNG,
			.SeedLife = EXAMPLE_SEEDLIFE,
			.PredResistanceEn = XTRNGPSV_FALSE,
			.DFDisable = XTRNGPSV_TRUE,
			.DFLenMul = 0U,
			.InitSeedPresent =  XTRNGPSV_FALSE,
			.PersStrPresent = XTRNGPSV_TRUE
	};

	xil_printf("\r\nConfigure secure communication client example");

#ifndef SDT
	Status = XMailbox_Initialize(&MailboxInstance, 0U);
#else
	Status = XMailbox_Initialize(&MailboxInstance, XPAR_XIPIPSU_0_BASEADDR);
#endif
	if (Status != XST_SUCCESS) {
		xil_printf("\r\n Mailbox initialization failed");
		goto END;
	}

	if (UsrCfg.PersStrPresent == XTRNGPSV_TRUE) {
		Status = Xil_SMemCpy(&UsrCfg.PersString, sizeof(UsrCfg.PersString), PersStr,
				sizeof(PersStr), sizeof(PersStr));
		if (Status != XST_SUCCESS) {
			goto END;
		}
	}

	/*
	 * Initialize the TRNGPSV driver so that it's ready to use look up
	 * configuration in the config table, then initialize it.
	 */
#ifndef SDT
	Config = XTrngpsv_LookupConfig(XPAR_XTRNGPSV_0_DEVICE_ID);
#else
	Config = XTrngpsv_LookupConfig(XPAR_XTRNGPSV_0_BASEADDR);
#endif
	if (NULL == Config) {
		xil_printf("LookupConfig Failed \n\r");
		goto END;
	}

	/* Initialize the TRNGPSV driver so that it is ready to use. */
	Status = XTrngpsv_CfgInitialize(&Trngpsv, Config, Config->BaseAddress);
	if (Status != XST_SUCCESS) {
		xil_printf("CfgInitialize Failed, Status: 0x%08x\n\r", Status);
		goto END;
	}

	/* Instantiate to complete initialization and for (initial) reseeding */
	Status = XTrngpsv_Instantiate(&Trngpsv, &UsrCfg);
	if (Status != XST_SUCCESS) {
		xil_printf("Instantiate failed, Status: 0x%08x\n\r", Status);
		goto END;
	}

	/**
	 * Invoke Generate for IV, SharedMem contains random data from this call.
	 * Only 12 bytes will be used for each IV
	 */
	Status = XTrngpsv_Generate(&Trngpsv, (u8 *)SsitSecCommData.IVsandKey.IVs, XPLMI_KEY_SIZE_BYTES,
	                          XTRNGPSV_FALSE);
	if (Status != XST_SUCCESS) {
		xil_printf("IVs Generation Failed, Status: 0x%08x\n\r", Status);
		goto END;
	}

	/* Invoke Generate for Key and print, SharedMem contains random data from this call */
	Status = XTrngpsv_Generate(&Trngpsv, (u8 *)SsitSecCommData.IVsandKey.Key, XPLMI_KEY_SIZE_BYTES,
	                           XTRNGPSV_FALSE);
	if (Status != XST_SUCCESS) {
		xil_printf("Key Generation Failed, Status: 0x%08x\n\r", Status);
		goto END;
	}

	SsitSecCommData.SlrIndex = SLR_INDEX;

	Xil_DCacheFlushRange((UINTPTR)&SsitSecCommData, sizeof(XPlmi_SsitSecComm));
	Status = XPlmi_ClientInit(&PlmiClientInstance, &MailboxInstance);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = XPlmi_ConfigSecureComm(&PlmiClientInstance, &SsitSecCommData);
	if (Status != XST_SUCCESS) {
		goto END;
    }

END:
	if (Status == XST_SUCCESS) {
		xil_printf("\r\nSuccessfully enabled secure communication between master and slave "
		"SLR%02x",SLR_INDEX);
	}
	else {
		xil_printf("\r\nConfiguration of secure communication failed with error code = %08x", Status);
	}

	return Status;
}
