/******************************************************************************
* Copyright (C) 2024 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/
/*****************************************************************************/
/**
* @file xilnvm_bbram_config_limiter_client_example.c
* @addtogroup xnvm_apis XilNvm APIs
* @{
* This file illustrates how to program parameters of configuration limiter into BBRAM.
*
* To build this application, xilmailbox library must be included in BSP and
* xilnvm library must be in client mode
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who     Date     Changes
* ----- ------  -------- ------------------------------------------------------
* 1.0   har     08/22/24 First release
*
* </pre>
*
* User configurable parameters
* ------------------------------------------------------------------------------
* #define CONFIGURATION_LIMITER_FEATURE_ENABLE				(FALSE)
*
* User must configure this macro to enable the Configuration limiter feature.
* FALSE means the feature is disabled and TRUE means the feature is enabled.
* By default the feature is disabled
*
* #define CONFIGURATION_LIMITER_MODE		(XNVM_BBRAM_CONFIG_LIMITER_FAIL_CONFIGS_COUNT)
*
* User must configure this macro to enable the Configuration limiter mode.
* The configuration limiter can maintain the count of either the failed configurations or total
* configurations.
* XNVM_BBRAM_CONFIG_LIMITER_FAIL_CONFIGS_COUNT means the counter of failed configurations is maintained
* XNVM_BBRAM_CONFIG_LIMITER_TOTAL_CONFIGS_COUNT means the counter of total configurations is maintained
* By default the counter holds the value of max number of failed configurations.
*
* #define CONFIGURATION_LIMITER_MAX_COUNT_VAL				(0x0)
* User must configure this macro to a non-zero value which should be less than 0x0FFFFFFFU.
*
* This example is supported for Versal Gen 2 devices only.
*
******************************************************************************/

/***************************** Include Files *********************************/
#include "xil_cache.h"
#include "xil_util.h"
#include "xnvm_bbramclient.h"
#include "xnvm_defs.h"

/***************** Macros (Inline Functions) Definitions *********************/
#define CONFIGURATION_LIMITER_FEATURE_ENABLE				(FALSE)
#define CONFIGURATION_LIMITER_MODE		(XNVM_BBRAM_CONFIG_LIMITER_FAIL_CONFIGS_COUNT)
#define CONFIGURATION_LIMITER_MAX_COUNT_VAL				(0x0)

/**************************** Type Definitions *******************************/


/************************** Variable Definitions ****************************/

/************************** Function Prototypes ******************************/
static int BbramWriteCLConfigParams(XNvm_ClientInstance *InstancePtr);

/*****************************************************************************/
/**
*
* Main function to call the function to provision Configuration limiter parameters
*
* @return
*		- XST_FAILURE if the BBRAM programming failed.
*
* @note		By default PLM does not include the NVM client code, it is
* 		disabled by a macro PLM_NVM_EXCLUDE. So, to run this
* 		client application successfully we need to enable NVM code in
* 		PLM before executing this application.
*
******************************************************************************/
int main(void)
{
	int Status = XST_FAILURE;
	XMailbox MailboxInstance;
	XNvm_ClientInstance NvmClientInstance;

#ifndef SDT
	Status = XMailbox_Initialize(&MailboxInstance, 0U);
#else
	Status = XMailbox_Initialize(&MailboxInstance, XPAR_XIPIPSU_0_BASEADDR);
#endif
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = XNvm_ClientInit(&NvmClientInstance, &MailboxInstance);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	NvmClientInstance.SlrIndex = XNVM_SLR_INDEX_0;

	Status = BbramWriteCLConfigParams(&NvmClientInstance);
	if (XST_SUCCESS == Status) {
		xil_printf("Successfully ran Versal Gen 2 example to provision configuration limiter parameters in BBRAM \n\r");
	}
	else {
		xil_printf("Provisioning configuration limiter parameters in BBRAM failed with error code = %08x\n\r", Status);
	}

END:
	return Status;
}



/*****************************************************************************/
/**
* BbramWriteUsrData function writes user data defined by macro
* XNVM_BBRAM_USER_DATA in BBRAM.
*
* @return
*		- XST_SUCCESS if the user data write successful
*		- XST_FAILURE if the user data write failed
*
******************************************************************************/
static int BbramWriteCLConfigParams(XNvm_ClientInstance *InstancePtr)
{
	int Status = XST_FAILURE;
	u32 ClEnFlag = 0;
	u32 ClMode = 0;
	u32 MaxNumOfConfigs = 0;

	if (CONFIGURATION_LIMITER_FEATURE_ENABLE == TRUE) {
		ClEnFlag = XNVM_BBRAM_CONFIG_LIMITER_ENABLED;
	}
	ClMode = CONFIGURATION_LIMITER_MODE;

	MaxNumOfConfigs = CONFIGURATION_LIMITER_MAX_COUNT_VAL;

	Status = XNvm_BbramWriteConfigLimiterParams(InstancePtr, ClEnFlag, ClMode, MaxNumOfConfigs);

	return Status;
}


/** //! [xilnvm_bbram_config_limiter_client_example] */
/** @} */
