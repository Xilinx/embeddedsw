/******************************************************************************
*
* Copyright (C) 2017-2019 Xilinx, Inc.  All rights reserved.
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
* FITNESS FOR A PRTNICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
* THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
* THE SOFTWARE.
*
* 
*
 ******************************************************************************/

/*****************************************************************************/
/**
*
* @file xpmcfw_initilization.c
*
* This is the file which contains initialization code for the PMCFW.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- -------- -------------------------------------------------------
* 1.00  kc   02/21/2017 Initial release
*
* </pre>
*
* @note
*
******************************************************************************/

/***************************** Include Files *********************************/
#include "xpmcfw_default.h"
#include "xpmcfw_main.h"
#include "xpmcfw_misc.h"
#include "xpmcfw_fabric.h"
/************************** Constant Definitions *****************************/
#define PRTN_NAME_LEN_MAX		20U

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/
static XStatus XPmcFw_PrimaryBootInit(XPmcFw * PmcFwInstancePtr);
static XStatus XPmcFw_SecondaryBootInit(XPmcFw * PmcFwInstancePtr);
static XStatus XPmcFw_SystemInit(const XPmcFw * PmcFwInstancePtr);
XStatus XPmcFw_BootDeviceInit(XPmcFw * PmcFwInstancePtr);

/************************** Variable Definitions *****************************/

/*****************************************************************************/
/**
 * This function is initializes the processor and system.
 *
 * @param	PmcFwInstancePtr is pointer to the XPmcFw Instance
 *
 * @return
 *          - returns the error codes described in xpmcfw_error.h on any error
 *		- returns XPMCFW_SUCCESS on success
 *
 *****************************************************************************/
XStatus XPmcFw_Initialize(XPmcFw * PmcFwInstancePtr)
{
	XStatus Status;


	/* Read SLR Type register and update */
	PmcFwInstancePtr->SlrType = XPmcFw_In32(PMC_TAP_SLR_TYPE) &
			PMC_TAP_SLR_TYPE_VAL_MASK;

	/* Do system initialization - Release sub-system PORs */
	/* Configure the PMC, PS as in PSU */
	Status = XPmcFw_SystemInit(PmcFwInstancePtr);
	if (XPMCFW_SUCCESS != Status) {
		goto END;
	}

#if defined(DEBUG_UART_PS)
	/* Print the PMCFW banner */
	(void )XPmcFw_InitPsUart();
	XPmcFw_PrintPmcFwBanner();
#endif

	 /* Enable Vgg Clamp in VGG Ctrl Register */
     XPmcFw_UtilRMW(PMC_ANALOG_VGG_CTRL,
                       PMC_ANALOG_VGG_CTRL_EN_VGG_CLAMP_MASK,
                       PMC_ANALOG_VGG_CTRL_EN_VGG_CLAMP_MASK);

     /* Check for PL PowerUp */
     Status = XPmcFw_UtilPollForMask(PMC_GLOBAL_PL_STATUS,
                     PMC_GLOBAL_PL_STATUS_POR_PL_B_MASK, 0x1U);
     if(Status == XPMCFW_SUCCESS)
     {
		Status = XPmcFw_PreCfgPL(PmcFwInstancePtr);
	if (XPMCFW_SUCCESS != Status) {
			goto END;
        }
	 }
	 else
	 {
		/* Ignore if PL is not powered up yet */
	 }

	/* Initialize the Drivers that are required
	 * TODO Need to check if DMA can be initialized before
	 * so that it can be used in PMC data file
	 */
	Status = XPmcFw_DmaInit();
	if (XPMCFW_SUCCESS != Status) {
		goto END;
	}

	Status = XPmcFw_BoardInit();
	if (XPMCFW_SUCCESS != Status) {
		goto END;
	}

	/* Do Boot device initialization */
	Status = XPmcFw_BootDeviceInit(PmcFwInstancePtr);
	if (XPMCFW_SUCCESS != Status) {
		goto END;
	}

#ifndef XPMCFW_STATIC_NPI_BYPASS
	/* Static NPI Initialization */
	if ((XPMCFW_PLATFORM == PMC_TAP_VERSION_EMU) ||
			(XPMCFW_PLATFORM == PMC_TAP_VERSION_SPP))
	{
		XPmcFw_Printf(DEBUG_INFO, "Initializing NPI...");
		Status = npi_init();
		if (Status == 0) {
			XPmcFw_Printf(DEBUG_INFO, "Done \n\r");
		} else {
			XPmcFw_Printf(DEBUG_INFO, "Failed \n\r");
		}
#if (!defined(XPMCFW_ME) && !defined(XPMCFW_SSIT))
		/* check if DDR is properly initialized */
		Xil_Out32(XPMCFW_TEST_DDR_ADDR, XPMCFW_TEST_DDR_VAL);
		if (Xil_In32(XPMCFW_TEST_DDR_ADDR) == XPMCFW_TEST_DDR_VAL)
		{
			XPmcFw_Printf(DEBUG_INFO, "DDR Test Pass \n\r");
		} else {
			XPmcFw_Printf(DEBUG_GENERAL, "XPMCFW_ERR_DDR_TEST \n\r");
			Status = XPMCFW_ERR_DDR_TEST;
		}
#endif
	}
#endif
	XPMCFW_DBG_WRITE(0x4U);
END:
	return Status;
}


/*****************************************************************************/
/**
 * This function initializes the primary and secondary boot devices
 * and validates the image header
 *
 * @param	PmcFwInstancePtr is pointer to the XPmcFw Instance
 *
 * @return	returns the error codes described in xpmcfw_error.h on any error
 *		returns XPMCFW_SUCCESS on success
 ******************************************************************************/
XStatus XPmcFw_BootDeviceInit(XPmcFw * PmcFwInstancePtr)
{
	XStatus Status;

	/* Configure the primary boot device */
	Status = XPmcFw_PrimaryBootInit(PmcFwInstancePtr);
	if (XPMCFW_SUCCESS != Status) {
		goto END;
	}

	/* For JTAG boot no need to any initialization */
	if (PmcFwInstancePtr->PrimaryBootDevice == XPMCFW_JTAG_BOOT_MODE)
	{
		goto END;
	}

	/* Read and Validate the header */
	Status = XPmcFw_PdiInit(PmcFwInstancePtr);
	if (XPMCFW_SUCCESS != Status) {
		goto END;
	}

	/* Update the secondary boot device */

	/* Configure the secondary boot device if required */
	if (PmcFwInstancePtr->SecondaryBootDevice !=
			PmcFwInstancePtr->PrimaryBootDevice) {
		Status = XPmcFw_SecondaryBootInit(PmcFwInstancePtr);
		if (XPMCFW_SUCCESS != Status) {
			goto END;
		}
	}
	XPMCFW_DBG_WRITE(0x3U);

END:
	return Status;
}

/*****************************************************************************/
/**
 * This function initializes the system using the psu_init()
 *
 * @param	PmcFwInstancePtr is pointer to the XPmcFw Instance
 *
 * @return	returns the error codes described in xpmcfw_error.h on any error
 *		returns XPMCFW_SUCCESS on success
 *
 ******************************************************************************/
static XStatus XPmcFw_SystemInit(const XPmcFw * PmcFwInstancePtr)
{
	XStatus Status;

	/* pmc initialization */
	Status = XPmcFw_HookPmcPsuInit();
	if (XPMCFW_SUCCESS != Status) {
		goto END;
	}

#ifndef XPMCFW_SC_BYPASS
	/* LPD Scan Clear for silicon platforms */
	if (XPMCFW_PLATFORM == PMC_TAP_VERSION_SILICON) {
		Status = XPmcFw_RunScanClearLPD();
		if (XPMCFW_SUCCESS != Status) {
			XPmcFw_Printf(DEBUG_GENERAL,
				"XPMCFW_ERR_SCAN_CLEAR_LPD\n\r");
			Status = XPMCFW_ERR_SCAN_CLEAR_LPD;
			goto END;
		}
	}
#endif

END:
	return Status;
}

/*****************************************************************************/
/**
 * This function initializes the primary boot device
 *
 * @param	PmcFwInstancePtr is pointer to the XPmcFw Instance
 *
 * @return	returns the error codes described in xpmcfw_error.h on any error
 *		returns XPMCFW_SUCCESS on success
 *
 ******************************************************************************/
static XStatus XPmcFw_PrimaryBootInit(XPmcFw * PmcFwInstancePtr)
{
	XStatus Status;
	u32 BootMode;

	/* Read Boot Mode register and update the value */
	if ((PmcFwInstancePtr->SlrType == SSIT_MASTER_SLR) ||
		(PmcFwInstancePtr->SlrType == SSIT_MONOLITIC)) {
		BootMode = XPmcFw_In32(CRP_BOOT_MODE_USER) &
				CRP_BOOT_MODE_USER_BOOT_MODE_MASK;
	} else {
		BootMode = XPMCFW_SMAP_BOOT_MODE;
	}
	PmcFwInstancePtr->PrimaryBootDevice = BootMode;

	/**
	 * Enable drivers only if they are device boot modes
	 * Not required for JTAG modes
	 */
	if ( (BootMode == XPMCFW_QSPI24_BOOT_MODE) ||
	     (BootMode == XPMCFW_QSPI32_BOOT_MODE) ||
	     (BootMode == XPMCFW_SD0_BOOT_MODE) ||
	     (BootMode == XPMCFW_EMMC_BOOT_MODE) ||
	     (BootMode == XPMCFW_SD1_BOOT_MODE) ||
	     (BootMode == XPMCFW_SD1_LS_BOOT_MODE)) {

		/* Initialize the WDT drivers */
#ifdef XPMCFW_WDT_PRESENT
		Status = XPmcFw_InitWdt();
		if (XPMCFW_SUCCESS != Status) {
			XPmcFw_Printf(DEBUG_INFO,
				      "WDT initialization failed \n\r");
			goto END;
		}
#endif
	}

	switch(BootMode)
	{
		/* For JTAG boot mode, it will be in while loop */
		case XPMCFW_JTAG_BOOT_MODE:
		{	/**
			  * Check if it is SBI JTAG boot mode
			  */
			if ((XPmcFw_In32(SLAVE_BOOT_SBI_MODE) &
			    SLAVE_BOOT_SBI_MODE_JTAG_MASK) ==
			    SLAVE_BOOT_SBI_MODE_JTAG_MASK)
			{
				PmcFwInstancePtr->PrimaryBootDevice =
					XPMCFW_SBI_JTAG_BOOT_MODE;
				XPmcFw_Printf(DEBUG_GENERAL,
					      "In SBI JTAG Boot Mode \n\r");
			} else {
				XPmcFw_Printf(DEBUG_GENERAL,
					      "In JTAG Boot Mode \n\r");
				Status = XPMCFW_SUCCESS;
				goto END;
			}
#ifdef XPMCFW_SBI
			/* Update the deviceops structure with necessary values */
			PmcFwInstancePtr->DeviceOps.Init = XPmcFw_SbiInit;
			PmcFwInstancePtr->DeviceOps.Copy = XPmcFw_SbiCopy;
			PmcFwInstancePtr->DeviceOps.Release = XPmcFw_SbiRelease;
			Status = XPMCFW_SUCCESS;
#else
			/* This bootmode is not supported in this release */
            XPmcFw_Printf(DEBUG_GENERAL,
			"XPMCFW_ERR_UNSUPPORTED_BOOT_MODE\n\r");
            Status = XPMCFW_ERR_UNSUPPORTED_BOOT_MODE;
#endif

		}
		break;

		case XPMCFW_QSPI24_BOOT_MODE:
		{
			XPmcFw_Printf(DEBUG_GENERAL, "QSPI 24bit Boot Mode \n\r");
#ifdef XPMCFW_QSPI
			/* Update the deviceops structure with necessary values */
			PmcFwInstancePtr->DeviceOps.Init = XPmcFw_Qspi24Init;
			PmcFwInstancePtr->DeviceOps.Copy = XPmcFw_Qspi24Copy;
			PmcFwInstancePtr->DeviceOps.Release = XPmcFw_Qspi24Release;
			Status = XPMCFW_SUCCESS;
#else
			/* This bootmode is not supported in this release */
			XPmcFw_Printf(DEBUG_GENERAL,
				"XPMCFW_ERR_UNSUPPORTED_BOOT_MODE\n\r");
			Status = XPMCFW_ERR_UNSUPPORTED_BOOT_MODE;
#endif
		}
		break;

		case XPMCFW_QSPI32_BOOT_MODE:
		{
			XPmcFw_Printf(DEBUG_GENERAL, "QSPI 32 bit Boot Mode \n\r");
#ifdef XPMCFW_QSPI
			/* Update the deviceops structure with necessary values */
			PmcFwInstancePtr->DeviceOps.Init = XPmcFw_Qspi32Init;
			PmcFwInstancePtr->DeviceOps.Copy = XPmcFw_Qspi32Copy;
			PmcFwInstancePtr->DeviceOps.Release = XPmcFw_Qspi32Release;
			Status = XPMCFW_SUCCESS;
#else
			/* This bootmode is not supported in this release */
			XPmcFw_Printf(DEBUG_GENERAL,
					"XPMCFW_ERR_UNSUPPORTED_BOOT_MODE\n\r");
			Status = XPMCFW_ERR_UNSUPPORTED_BOOT_MODE;
#endif
		}
		break;

		case XPMCFW_SMAP_BOOT_MODE:
		{
			XPmcFw_Printf(DEBUG_GENERAL, "SMAP Boot Mode \n\r");
#ifdef XPMCFW_SBI
			/* Update the deviceops structure with necessary values */
			PmcFwInstancePtr->DeviceOps.Init = XPmcFw_SbiInit;
			PmcFwInstancePtr->DeviceOps.Copy = XPmcFw_SbiCopy;
			PmcFwInstancePtr->DeviceOps.Release = XPmcFw_SbiRelease;
			Status = XPMCFW_SUCCESS;
#else
			/* This bootmode is not supported in this release */
			XPmcFw_Printf(DEBUG_GENERAL,
					"XPMCFW_ERR_UNSUPPORTED_BOOT_MODE\n\r");
			Status = XPMCFW_ERR_UNSUPPORTED_BOOT_MODE;
#endif
		}
		break;

		case XPMCFW_SD0_BOOT_MODE:
		{
			XPmcFw_Printf(DEBUG_GENERAL, "SD0 Boot Mode \n\r");
#ifdef XPMCFW_SD_0
			/* Update the deviceops structure with necessary values */
			PmcFwInstancePtr->DeviceOps.Init = XPmcFw_SdInit;
			PmcFwInstancePtr->DeviceOps.Copy = XPmcFw_SdCopy;
			PmcFwInstancePtr->DeviceOps.Release = XPmcFw_SdRelease;
			Status = XPMCFW_SUCCESS;
#else
			/* This bootmode is not supported in this release */
			XPmcFw_Printf(DEBUG_GENERAL,
				"XPMCFW_ERR_UNSUPPORTED_BOOT_MODE\n\r");
			Status = XPMCFW_ERR_UNSUPPORTED_BOOT_MODE;
#endif
		} break;

		case XPMCFW_EMMC_BOOT_MODE:
		case XPMCFW_SD1_BOOT_MODE:
		case XPMCFW_SD1_LS_BOOT_MODE:
		{
			if (BootMode == XPMCFW_SD1_BOOT_MODE) {
				XPmcFw_Printf(DEBUG_GENERAL, "SD1 Boot Mode \n\r");
			} else if (BootMode == XPMCFW_EMMC_BOOT_MODE) {
				XPmcFw_Printf(DEBUG_GENERAL, "EMMC Boot Mode \n\r");
			}
			else {
				XPmcFw_Printf(DEBUG_GENERAL,
					"SD1 with level shifter Boot Mode \n\r");
			}
#ifdef XPMCFW_SD_1
			/* Update the deviceops structure with necessary values */
			PmcFwInstancePtr->DeviceOps.Init = XPmcFw_SdInit;
			PmcFwInstancePtr->DeviceOps.Copy = XPmcFw_SdCopy;
			PmcFwInstancePtr->DeviceOps.Release = XPmcFw_SdRelease;
			Status = XPMCFW_SUCCESS;
#else
			/* This bootmode is not supported in this release */
			XPmcFw_Printf(DEBUG_GENERAL,
				"XPMCFW_ERR_UNSUPPORTED_BOOT_MODE\n\r");
			Status = XPMCFW_ERR_UNSUPPORTED_BOOT_MODE;
#endif
		} break;

		case XPMCFW_OSPI_BOOT_MODE:
		{
#ifdef XPMCFW_OSPI
			PmcFwInstancePtr->DeviceOps.Init = XPmcFw_OspiInit;
                        PmcFwInstancePtr->DeviceOps.Copy = XPmcFw_OspiCopy;
                        PmcFwInstancePtr->DeviceOps.Release = XPmcFw_OspiRelease;
                        Status = XPMCFW_SUCCESS;
#else
			/* This bootmode is not supported in this release */
                        XPmcFw_Printf(DEBUG_GENERAL,
                                "XPMCFW_ERR_UNSUPPORTED_BOOT_MODE\n\r");
                        Status = XPMCFW_ERR_UNSUPPORTED_BOOT_MODE;
#endif
		} break;

		default:
		{
			XPmcFw_Printf(DEBUG_GENERAL,
				"XPMCFW_ERR_UNSUPPORTED_BOOT_MODE\n\r");
			Status = XPMCFW_ERR_UNSUPPORTED_BOOT_MODE;
		} break;

	}

	/**
	 * In case of error, goto end
	 */
	if (XPMCFW_SUCCESS != Status) {
		goto END;
	}

	/**
	 * Initialize the Device Driver.
	 * Skip initialization for Slave SLRs as master sets the SBI interface to
	 * AXI slave data transfer before sending the boot image.
	 */
	if ((PmcFwInstancePtr->SlrType == SSIT_MASTER_SLR) ||
		(PmcFwInstancePtr->SlrType == SSIT_MONOLITIC)) {
		Status = PmcFwInstancePtr->DeviceOps.Init(
				PmcFwInstancePtr->PrimaryBootDevice);
		if (XPMCFW_SUCCESS != Status) {
			goto END;
		}
	}

END:
	return Status;
}

/*****************************************************************************/
/**
 * This function initializes secondary boot device
 *
 * @param	PmcFwInstancePtr is pointer to the XPmcFw Instance
 *
 * @return	returns the error codes described in xpmcfw_error.h on any error
 *		returns XPMCFW_SUCCESS on success
 *
 ******************************************************************************/
static XStatus XPmcFw_SecondaryBootInit(XPmcFw * PmcFwInstancePtr)
{
	XStatus Status = XPMCFW_SUCCESS;

	/**
	 * Update the deviceops structure
	 */


	/**
	 * Initialize the Secondary Boot Device Driver
	 */

	return Status;
}

/****************************************************************************/
/**
 * This function is used to perform Pre Configuration operations on PL. It
 * initializes fabric drivers and does housecleaning of PL.
 *
 * @param       PmcFwInstancePtr is pointer to the PmcFw Instance
 *
 * @param       PrtnNum is the partition number in the image to be loaded
 *
 * @return
 *              - XPMCFW_SUCCESS on Success
 *              - ErrorCode as specified in xpmcfw_err.h
 *
 * @note
 *
 *****************************************************************************/
XStatus XPmcFw_PreCfgPL(XPmcFw* PmcFwInstancePtr)
{
	XStatus Status;

        /* Do the Fabric driver Initialization */
        Status = XPmcFw_FabricInit();
        if (XPMCFW_SUCCESS != Status)
        {
                goto END;
        }
        XPmcFw_FabricEnable();
#ifndef XPMCFW_HOUSECLEAN_BYPASS
        /* Check for PL clearing */
        if(PmcFwInstancePtr->PlCleaningDone==FALSE)
        {
                Status = XPmcFw_FabricClean();
                if (Status != XPMCFW_SUCCESS) {
                        goto END;
                }
                PmcFwInstancePtr->PlCleaningDone = TRUE;
        }
#else
	PmcFwInstancePtr->PlCleaningDone = TRUE;
#endif
END:
	return Status;
}
