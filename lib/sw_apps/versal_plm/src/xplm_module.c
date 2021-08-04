/******************************************************************************
* Copyright (c) 2018 - 2021 Xilinx, Inc. All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xplm_module.c
*
* This file contains the module initialization code for PLM.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ====  ==== ======== ======================================================-
* 1.00  kc   08/28/2018 Initial release
* 1.01  sn   07/04/2019 Added code to enable SysMon's over-temperature
*                       interrupt
*       kc   08/01/2019 Added error management module in PLM
*       ma   08/01/2019 Removed LPD module init related code from PLM app
* 1.02  kc   03/23/2020 Minor code cleanup
* 1.03  rama 07/29/2020 Added module support for STL
* 1.04  ma   01/12/2021 Directly call module init function instead of calling
*                       in for-loop
*                       Remove unused Status variable in XPlm_ErrInit
*       rb   01/28/2021 Added Sem PreInit API call
*       bm   02/17/2021 Reinitialize Status in XPlm_ModuleInit to add redundancy
*       rb   03/09/2021 Updated Sem Init API call
*       rama 03/22/2021 Fixed compilation warning on STL inclusion
* 1.05  td   07/08/2021 Fix doxygen warnings
*       kal  07/13/2021 Added module support for XilNvm
*       bm   08/03/2021 Added temporal redundancy check for XPlm_SecureInit
*
* </pre>
*
* @note
*
******************************************************************************/

/***************************** Include Files *********************************/
#include "xplm_module.h"
#include "xplm_default.h"
#include "xplmi_sysmon.h"
#include "xpm_api.h"
#include "xsecure_init.h"
#ifndef PLM_NVM_EXCLUDE
#include "xnvm_init.h"
#endif
#include "xplmi_err.h"
#include "xplm_loader.h"
#include "xplm_pm.h"
#ifdef XPLM_SEM
#include "xplm_sem_init.h"
#endif
#ifdef PLM_ENABLE_STL
#include "xplm_stl.h"
#endif
/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/
static int XPlm_PlmiInit(void);
static int XPlm_ErrInit(void);
static int XPlm_SecureInit(void);
#ifndef PLM_NVM_EXCLUDE
static void XPlm_NvmInit(void);
#endif

/************************** Variable Definitions *****************************/

/*****************************************************************************/

/*****************************************************************************/
/**
 * @brief This function initializes the XilSecure module and registers the
 * interrupt handlers and other requests.
 *
 * @return	Status as defined in xplmi_status.h
 *
 *****************************************************************************/
static int XPlm_SecureInit(void)
{
	int Status = XST_FAILURE;
	Status = XSecure_Init();

	return Status;
}

/*****************************************************************************/
/**
 * @brief This function initializes the Error module and registers the
 * error commands of the CDO.
 *
 * @return	Status as defined in xplmi_status.h
 *
 *****************************************************************************/
static int XPlm_ErrInit(void)
{
	XPlmi_EmInit(XPm_SystemShutdown);

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
 * @brief This function initializes the PLMI module and registers the
 * general commands of the CDO.
 *
 * @return	Status as defined in xplmi_status.h
 *
 *****************************************************************************/
static int XPlm_PlmiInit(void)
{
	int Status = XST_FAILURE;
	Status = XPlmi_Init();

	return Status;
}

#ifndef PLM_NVM_EXCLUDE
/*****************************************************************************/
/**
 * @brief This function initializes the XilNvm module and registers the
 * interrupt handlers and other requests.
 *
 * @return	Status as defined in xplmi_status.h
 *
 *****************************************************************************/
static void XPlm_NvmInit(void)
{
	XNvm_Init();
}
#endif

/*****************************************************************************/
/**
 * @brief This function call all the init functions of all the different
 * modules. As a part of init functions, modules can register the
 * command handlers, interrupt handlers with the interface layer.
 *
 * @param	Arg is not used
 *
 * @return	Status as defined in xplmi_status.h
 *
 *****************************************************************************/
int XPlm_ModuleInit(void *Arg)
{
	volatile int Status = XST_FAILURE;

	(void) Arg;

	/*
	 * Call initialization function for the below modules:
	 * 	-PLMI
	 * 	-Error manager
	 * 	-PM
	 * 	-Loader
	 * 	-Secure
	 * 	-STL
	 */

	Status = XPlm_PlmiInit();
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = XPlm_ErrInit();
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = XPlm_PmInit();
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = XPlm_LoaderInit();
	if (Status != XST_SUCCESS) {
		goto END;
	}

	XSECURE_TEMPORAL_CHECK(END, Status, XPlm_SecureInit);

#ifndef PLM_NVM_EXCLUDE
	XPlm_NvmInit();
#endif
#ifdef PLM_ENABLE_STL
	Status = XPlm_StlInit();
	if (Status != XST_SUCCESS) {
		goto END;
	}
#endif
#ifdef XPLM_SEM
	Status = XPlm_SemInit();
	if (Status != XST_SUCCESS) {
		goto END;
	}
#endif

END:
	return Status;
}
