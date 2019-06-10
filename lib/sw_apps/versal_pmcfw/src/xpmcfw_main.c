/******************************************************************************
*
* Copyright (C) 2017-2018 Xilinx, Inc.  All rights reserved.
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
* XILINX  BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
* WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
* OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*
* Except as contained in this notice, the name of the Xilinx shall not be used
* in advertising or otherwise to promote the sale, use or other dealings in
* this Software without prior written authorization from Xilinx.
*
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xpmcfw_main.c
*
* This is the main file which contains code for the PMCFW.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ====- ==== ======== ======================================================-
* 1.00  kc   2/21/2017 Initial release
*
* </pre>
*
* @note
*
******************************************************************************/

/***************************** Include Files *********************************/
#include "xpmcfw_main.h"
#include "xil_exception.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/
static void XPmcFw_UpdateMultiBoot(u32 MultiBootValue);
static void XPmcFw_FallBack(void);
static void XPmcFw_InitializeVar(void);
static XStatus XPmcFw_LoadSubsystem(void);
static XStatus XPmcFw_StartSubsystem(void);
#if (defined(XPMCFW_SSIT) && defined(XPMCFW_SBI))
static XStatus XPmcFw_LoadSlaveSlr(u64 SlrBaseAddr);
#endif

/************************** Variable Definitions *****************************/
XPmcFw PmcFwInstance;
u32 Platform;
/*****************************************************************************/
/** This is the PMCFW main function and is implemented stage wise.
 *
 * @param	None
 *
 * @return	None
 *
 *****************************************************************************/
int main(void )
{
	XStatus Status;
	u64 StartT1;

	Status = XPMCFW_SUCCESS;

	/**
	 * Enable exceptions and timers
	 */
	XPmcFw_ExceptionInit();
	XPmcFw_InitIOModule();
	StartT1 = XPmcFw_GetTimerValue();

#ifdef DEBUG_UART_MDM
	/** If MDM UART, banner can be printed before any initialization */
	XPmcFw_InitMdmUart();
	XPmcFw_PrintPmcFwBanner();
#endif
	/** Initialize PMC FW instance*/
	XPmcFw_InitializeVar();

	/* Initialization */
	XPMCFW_DBG_WRITE(0x1U);
	XPmcFw_Printf(DEBUG_GENERAL,
		 "\n\r==========Initialization========\n\r");
	Status = XPmcFw_Initialize(&PmcFwInstance);
	if (XPMCFW_SUCCESS != Status)
	{
		goto END;
	}

	/*Subsystems are read from PDI only if it not JTAG boot mode */
	if (PmcFwInstance.PrimaryBootDevice != XPMCFW_JTAG_BOOT_MODE)
	{
		/* Load subsystems */
		Status = XPmcFw_LoadSubsystem();
		if (XPMCFW_SUCCESS != Status)
		{
			goto END;
		}

		/* Start subsystems */
		Status = XPmcFw_StartSubsystem();
		if (XPMCFW_SUCCESS != Status)
		{
			goto END;
		}
#if (defined(XPMCFW_SSIT) && defined(XPMCFW_SBI))

		if (PmcFwInstance.SlrType == SSIT_MASTER_SLR) {
			/*
			 * Load Slave SLR1
			 */
			Status = XPmcFw_LoadSlaveSlr(PMC_ALIAS1_GLOBAL_BASEADDR);
			if (XPMCFW_SUCCESS != Status) {
				goto END;
			}

			/*
			 * Wait for SLR1 programming completion to load Slave SLR2
			 */
			while ((Xil_In32(PMC_GLOBAL_PMC_ERR2_STATUS) &
					PMC_GLOBAL_PMC_ERR2_STATUS_SSIT_ERR0_MASK) !=
					PMC_GLOBAL_PMC_ERR2_STATUS_SSIT_ERR0_MASK);

			/* Clear SSIT interrupt */
			Xil_Out32(PMC_GLOBAL_PMC_ERR2_STATUS,
					PMC_GLOBAL_PMC_ERR2_STATUS_SSIT_ERR0_MASK);

			Xil_Out32(PPU1_IOMODULE_IRQ_ACK,
					PPU1_IOMODULE_IRQ_ACK_SSIT_IRQ0_MASK);

			XPmcFw_Printf(DEBUG_GENERAL, "NoC config done on Slave0 \n\r");
			Status = XPmcFw_LoadSlaveSlr(PMC_ALIAS2_GLOBAL_BASEADDR);
			if (XPMCFW_SUCCESS != Status) {
				goto END;
			}
		}
#endif
	}

	XPmcFw_MeasurePerfTime(StartT1);
	XPmcFw_Printf(DEBUG_GENERAL, "Boot time \n\r");

	/* Life Cycle Management */
	XPmcFw_Printf(DEBUG_GENERAL,
	      "\n\r============Life Cycle Management===========\n\r");

	/*Configure interrupt handlers and enable interrupts */
	XPmcFw_SetUpInterruptSystem();
	/* Be in sleep mode waiting for interrupts */
	/* TODO to indicate PMC FW has completed booting */
	Xil_Out32(PMC_GLOBAL_PMC_FW_ERR, XPMCFW_POST_BOOT);
	while(1){
		mb_sleep();
	}

END:
	/* Error Lockdown PMCFW */
	XPmcFw_ErrorLockDown(Status);
	while(1);

	return 0;
}

/*****************************************************************************/
/**
 * This function loads the subsystems present in PDI in order
 *
 * @param none
 *
 * @return	returns the error codes described in xpmcfw_error.h on any error
 *		returns XPMCFW_SUCCESS on success
 *
 *****************************************************************************/
static XStatus XPmcFw_LoadSubsystem(void)
{
	XStatus Status;
	u32 PrtnNum;
	u64 PrtnStartT1;

	XPmcFw_Printf(DEBUG_GENERAL,
		      "\n\r==========Partition Load========\n\r");

	for (PrtnNum = 1U;
	     PrtnNum < PmcFwInstance.MetaHdr.ImgHdrTable.NoOfPrtns;
	     PrtnNum++)
	{
		PrtnStartT1 = XPmcFw_GetTimerValue();
		Status = XPmcFw_PrtnLoad(&PmcFwInstance, PrtnNum);
		if (XPMCFW_SUCCESS != Status) {
			goto END;
		}
		XPmcFw_MeasurePerfTime(PrtnStartT1);
		XPmcFw_Printf(DEBUG_GENERAL,
			      "P%d Load Time \n\r", PrtnNum);
	}
	XPMCFW_DBG_WRITE(0x5U);
	Status = XPMCFW_SUCCESS;

END:
	return Status;
}

/*****************************************************************************/
/**
 * This function starts the subsystems present in PDI in order
 *
 * @param none
 *
 * @return	returns the error codes described in xpmcfw_error.h on any error
 *		returns XPMCFW_SUCCESS on success
 *
 *****************************************************************************/
static XStatus XPmcFw_StartSubsystem(void)
{
	XStatus Status;
	XPmcFw_Printf(DEBUG_GENERAL,
	      "\n\r==========Handoff===============\n\r");
	Status = XPmcFw_Handoff(&PmcFwInstance);
	if (XPMCFW_SUCCESS != Status) {
		goto END;
	}
	XPMCFW_DBG_WRITE(0x6U);
END:
	return Status;
}

#if (defined(XPMCFW_SSIT) && defined(XPMCFW_SBI))
/*****************************************************************************/
/**
 * This function starts the subsystems present in PDI in order
 *
 * @param none
 *
 * @return	returns the error codes described in xpmcfw_error.h on any error
 *		returns XPMCFW_SUCCESS on success
 *
 *****************************************************************************/
static XStatus XPmcFw_LoadSlaveSlr(u64 SlrBaseAddr)
{
	u32 PdiRemLength = XPMCFW_SLAVE_PDI_LENGTH;
	u32 TransBytes;
	u32 MultiBootOffset;
	u32 PdiSrcAddr;
	u32 Status = XST_SUCCESS;
	u64 SlrSbiBufAddr = (SlrBaseAddr +
					((u64)(PMC_SBI_STREAM_BUF_BASEADDR - PMC_LOCAL_BASEADDR)));

	MultiBootOffset = Xil_In32(PMC_GLOBAL_PMC_MULTI_BOOT) &
						XPMCFW_MULTIBOOT_OFFSET_MASK;
	PdiSrcAddr = XPMCFW_SLAVE_PDI_QSPI_OFFSET +
				(MultiBootOffset * XPMCFW_IMAGE_SEARCH_OFST);

    if (PmcFwInstance.SlrType == SSIT_MASTER_SLR) { // Master SLR

	XPmcFw_Printf(DEBUG_GENERAL,
		 "\n\r==========Loading Slave SLR PDI========\n\r");

		XPmcFw_Printf(DEBUG_INFO, "Load the image to Slave SLR via AXI SBI interface\r\n");

		while (PdiRemLength != 0) {
			if (PdiRemLength > 0x10000U) {
				TransBytes = 0x10000U;
			} else {
				TransBytes = PdiRemLength;
			}
			XPmcFw_Printf(DEBUG_INFO, "Loading of image to Slave SLR is in progress...\r\n");

			Status = PmcFwInstance.DeviceOps.Copy(PdiSrcAddr,
							SlrSbiBufAddr, TransBytes, 0x0U);
			if (XPMCFW_SUCCESS != Status)
			{
				goto END;
			}
			PdiSrcAddr += TransBytes;
			PdiRemLength -= TransBytes;
		}
		XPmcFw_Printf(DEBUG_INFO, "Loading of image to Slave SLR is completed\r\n");
	}

END:
	return Status;

}
#endif

/*****************************************************************************/
/**
 * This function prints PMC FW banner
 *
 * @param none
 *
 * @return	none
 *
 *****************************************************************************/
void XPmcFw_PrintPmcFwBanner(void )
{
	u32 Version;
	u32 PlatformVersion;
	u32 RtlVersion;
	u32 PsVersion;
	u32 PmcVersion;

	/* Print the PMCFW Banner */
	XPmcFw_Printf(DEBUG_PRINT_ALWAYS,
                 "\n\r****************************************\n\r");
	XPmcFw_Printf(DEBUG_PRINT_ALWAYS,
                 "Xilinx versal PMC Firmware \n\r");
	XPmcFw_Printf(DEBUG_PRINT_ALWAYS,
                 "Release  v%s %s.%s   %s  -  %s\n\r",
                 PMCFW_RELEASE_VERSION, SDK_RELEASE_YEAR, SDK_RELEASE_QUARTER,
		__DATE__, __TIME__);

	/* Read the Version */
	Version = Xil_In32(PMC_TAP_VERSION);
	PlatformVersion = ((Version & PMC_TAP_VERSION_PLATFORM_VERSION_MASK) >>
			PMC_TAP_VERSION_PLATFORM_VERSION_SHIFT);
	Platform = ((Version & PMC_TAP_VERSION_PLATFORM_MASK) >>
			PMC_TAP_VERSION_PLATFORM_SHIFT);
	RtlVersion = ((Version & PMC_TAP_VERSION_RTL_VERSION_MASK) >>
			PMC_TAP_VERSION_RTL_VERSION_SHIFT);
	PsVersion = ((Version & PMC_TAP_VERSION_PS_VERSION_MASK) >>
			PMC_TAP_VERSION_PS_VERSION_SHIFT);
	PmcVersion = ((Version & PMC_TAP_VERSION_PMC_VERSION_MASK) >>
			PMC_TAP_VERSION_PMC_VERSION_SHIFT);
	switch(Platform)
	{
		case PMC_TAP_VERSION_SILICON:
			XPmcFw_Printf(DEBUG_PRINT_ALWAYS, "Silicon: "); break;
		case PMC_TAP_VERSION_SPP:
			XPmcFw_Printf(DEBUG_PRINT_ALWAYS, "SPP: "); break;
		case PMC_TAP_VERSION_EMU:
			XPmcFw_Printf(DEBUG_PRINT_ALWAYS, "EMU: "); break;
		case PMC_TAP_VERSION_QEMU:
			XPmcFw_Printf(DEBUG_PRINT_ALWAYS, "QEMU: "); break;
		default:break;
	}

	XPmcFw_Printf(DEBUG_PRINT_ALWAYS, "v%d, ", PlatformVersion);
	XPmcFw_Printf(DEBUG_PRINT_ALWAYS, "RTL: ITR%d, ", (RtlVersion/16));
	XPmcFw_Printf(DEBUG_PRINT_ALWAYS, "PMC: v%d.%d, ",
				(PmcVersion/16), PmcVersion%16);
	XPmcFw_Printf(DEBUG_PRINT_ALWAYS, "PS: v%d.%d",
				(PsVersion/16), PsVersion%16);
	XPmcFw_Printf(DEBUG_PRINT_ALWAYS, "\n\r");
#ifdef DEBUG_UART_MDM
	XPmcFw_Printf(DEBUG_INFO, "STDOUT: MDM UART \n\r");
#else
	XPmcFw_Printf(DEBUG_INFO, "STDOUT: PS UART \n\r");
#endif
	XPmcFw_Printf(DEBUG_PRINT_ALWAYS,
                 "****************************************\n\r");
}

/*****************************************************************************/
/**
 * It initializes the PmcFwInstace variables
 *
 * @param none
 *
 * @return none
 *
 *****************************************************************************/
static void XPmcFw_InitializeVar(void )
{

	memset(&PmcFwInstance, 0x0, sizeof(PmcFwInstance));
	PmcFwInstance.Version=0U;
	PmcFwInstance.NoOfHandoffCpus=0U;
	PmcFwInstance.Status=XPMCFW_FAILURE;
	PmcFwInstance.PlCleaningDone=FALSE;
	PmcFwInstance.PartialPdi=FALSE;
}

/*****************************************************************************/
/**
 * This function is called in PMCFW error cases. Error status
 * register is updated and fallback is applied
 *
 * @param ErrorStatus is the error code which is written to the
 *		  error status register
 *
 * @return none
 *
 * @note Fallback is applied only for fallback supported bootmodes
 *****************************************************************************/
void XPmcFw_ErrorLockDown(u32 ErrorStatus)
{

	u32 BootMode;

	/* Print the PMCFW error */
	XPmcFw_Printf(DEBUG_GENERAL, "PmcFw Error Status: 0x%08lx\n\r",
		ErrorStatus);

	XPmcFw_DumpRegisters();

	/* Update the error status register and PmcFw instance structure */
	PmcFwInstance.Status = ErrorStatus;
	Xil_Out32(PMC_GLOBAL_PMC_FW_ERR, ErrorStatus);

	if ((PmcFwInstance.SlrType == SSIT_MASTER_SLR) ||
		(PmcFwInstance.SlrType == SSIT_MONOLITIC)) {
		/* Read Boot Mode register */
		BootMode = XPmcFw_In32(CRP_BOOT_MODE_USER) &
				CRP_BOOT_MODE_USER_BOOT_MODE_MASK;

		/* Fallback if bootmode supports */
		if ( (BootMode == XPMCFW_QSPI24_BOOT_MODE) ||
		     (BootMode == XPMCFW_QSPI32_BOOT_MODE) ||
		     (BootMode == XPMCFW_SD0_BOOT_MODE)    ||
		     (BootMode == XPMCFW_EMMC_BOOT_MODE)   ||
		     (BootMode == XPMCFW_SD1_BOOT_MODE)    ||
		     (BootMode == XPMCFW_SD1_LS_BOOT_MODE) )
		{
			XPmcFw_FallBack();
		} else {
			/* Be in while loop if fallback is not supported */
			XPmcFw_Printf(DEBUG_GENERAL,"Fallback not supported \n\r");

			/* TODO Exit to handoffexit code or postboot */
			while(1);
		}
	} else {
		Xil_Out32(PMC_GLOBAL_PERS_GLOB_GEN_STORAGE4, 0xFU);
		XPmcFw_Printf(DEBUG_GENERAL,"Fallback occurred. Generating an error to Master PMC \n\r");
		Xil_Out32(PMC_GLOBAL_SSIT_ERR, SSIT_ERR_INTR_MASK);

		while(1);
	}

	/* we should never be here */
}

/*****************************************************************************/
/**
 * In Fallback,  soft reset is applied to the system after incrementing
 * the multiboot register. A hook is provided to before the fallback so
 * that users can write their own code before soft reset
 *
 * @param none
 *
 * @return none
 *
 * @note We will not return from this function as it does soft reset
 *****************************************************************************/
static void XPmcFw_FallBack(void)
{
	u32 RegValue;

#ifdef XPMCFW_WDT_PRESENT
	/* Stop WDT as we are restarting */
	XPmcFw_StopWdt();
#endif

	/* Hook before PMCFW Fallback */
	(void)XPmcFw_HookBeforeFallback();

	/* Read the Multiboot register */
	/* TODO update multiboot register */
	RegValue = Xil_In32(PMC_GLOBAL_PMC_MULTI_BOOT);

	XPmcFw_Printf(DEBUG_GENERAL, "Performing PMCFW FallBack\n\r");

	XPmcFw_UpdateMultiBoot(RegValue+1U);
}


/*****************************************************************************/
/**
 * This is the function which actually updates the multiboot register and
 * does the soft reset. This function is called in fallback case and
 * in the cases where user would like to jump to a different image,
 * corresponding to the multiboot value being passed to this function.
 * The latter case is a generic one and need arise because of error scenario.
 *
 * @param MultiBootValue is the new value for the multiboot register
 *
 * @return none
 *
 * @note We will not return from this function as it does soft reset
 *****************************************************************************/
static void XPmcFw_UpdateMultiBoot(u32 MultiBootValue)
{

	/* TODO Update Multiboot Register */
	XPmcFw_UtilRMW(PMC_GLOBAL_PMC_MULTI_BOOT, XPMCFW_MULTIBOOT_OFFSET_MASK,
						MultiBootValue);

	/* make sure every thing completes */
	/* TODO check for dsb/isb similar terms for MB */
	mbar(1); //Data synchronization
	mbar(0); //Instruction synchronization

	/* Soft reset the system */
	XPmcFw_Printf(DEBUG_INFO, "Performing System Soft Reset\n\r");
	/* TODO soft reset the system when supported */
	XPmcFw_UtilRMW(CRP_RST_PS, CRP_RST_PS_PMC_SRST_MASK,
					CRP_RST_PS_PMC_SRST_MASK);

	/* wait here until reset happens */
	while(1) {
	;
	}

	/* we should never be here */
}

/*****************************************************************************/
/**
 * This function enables the exceptions and interrupts
 * Enable interrupts from the hardware
 *
 * @param
 *
 * @return none
 *
 * @note none
 *****************************************************************************/
void XPmcFw_ExceptionInit(void )
{
	u32 Index;
	Xil_ExceptionInit();

	/* Register exception handlers */
	for (Index = XIL_EXCEPTION_ID_FIRST;
	     Index <= XIL_EXCEPTION_ID_LAST; Index++)
	{
		Xil_ExceptionRegisterHandler(Index,
			     (Xil_ExceptionHandler)XPmcFw_ExceptionHandler,
			     (void *)XPMCFW_ERR_EXCEPTION);
	}

	microblaze_enable_exceptions();
}

/*****************************************************************************/
/**
 * This is a function handler for exceptions
 *
 * @param
 *
 * @return none
 *
 * @note none
 *****************************************************************************/
void XPmcFw_ExceptionHandler(u32 Status)
{
	XPmcFw_Printf(DEBUG_GENERAL, "Received Exception \n\r"
		      "MSR: 0x%08x, EAR: 0x%08x, EDR: 0x%08x, ESR: 0x%08x, \n\r"
		      "R14: 0x%08x, R15: 0x%08x, R16: 0x%08x, R17: 0%08x \n\r",
		      mfmsr(), mfear(), mfedr(), mfesr(),
		      mfgpr(r14), mfgpr(r15), mfgpr(r16), mfgpr(r17));

	XPmcFw_ErrorLockDown(Status);

	/*Just in Case if it returns */
	while(1);
}

/*****************************************************************************/
/**
 * This function dumps the registers which can help debugging
 *
 * @param
 *
 * @return none
 *
 * @note none
 *****************************************************************************/
void XPmcFw_DumpRegisters()
{

	XPmcFw_Printf(DEBUG_GENERAL, "====Register Dump============\n\r");

	XPmcFw_Printf(DEBUG_GENERAL, "IDCODE: 0x%08x\n\r",
		      Xil_In32(PMC_TAP_IDCODE));
	XPmcFw_Printf(DEBUG_GENERAL, "Version: 0x%08x\n\r",
		      Xil_In32(PMC_TAP_VERSION));
	XPmcFw_Printf(DEBUG_GENERAL, "Bootmode User: 0x%08x\n\r",
		      Xil_In32(CRP_BOOT_MODE_USER));
	XPmcFw_Printf(DEBUG_GENERAL, "Bootmode POR: 0x%08x\n\r",
		      Xil_In32(CRP_BOOT_MODE_POR));
	XPmcFw_Printf(DEBUG_GENERAL, "Reset Reason: 0x%08x\n\r",
		      Xil_In32(CRP_RESET_REASON));
	XPmcFw_Printf(DEBUG_GENERAL, "Multiboot: 0x%08x\n\r",
		      Xil_In32(PMC_GLOBAL_PMC_MULTI_BOOT));
	XPmcFw_Printf(DEBUG_GENERAL, "PMC PWR Status: 0x%08x\n\r",
		      Xil_In32(PMC_GLOBAL_PWR_STATUS));
	XPmcFw_Printf(DEBUG_GENERAL, "PMC GSW Err: 0x%08x\n\r",
		      Xil_In32(PMC_GLOBAL_PMC_GSW_ERR));
	XPmcFw_Printf(DEBUG_GENERAL, "PMC FW Error: 0x%08x\n\r",
		      Xil_In32(PMC_GLOBAL_PMC_FW_ERR));
	XPmcFw_Printf(DEBUG_GENERAL, "PMC ERR OUT1 Status: 0x%08x\n\r",
		      Xil_In32(PMC_GLOBAL_PMC_ERR1_STATUS));
	XPmcFw_Printf(DEBUG_GENERAL, "PMC ERR OUT2 Status: 0x%08x\n\r",
		      Xil_In32(PMC_GLOBAL_PMC_ERR2_STATUS));
	XPmcFw_Printf(DEBUG_GENERAL, "GICP0 IRQ Status: 0x%08x\n\r",
		      Xil_In32(PMC_GLOBAL_GICP0_IRQ_STATUS));
	XPmcFw_Printf(DEBUG_GENERAL, "GICP1 IRQ Status: 0x%08x\n\r",
		      Xil_In32(PMC_GLOBAL_GICP1_IRQ_STATUS));
	XPmcFw_Printf(DEBUG_GENERAL, "GICP2 IRQ Status: 0x%08x\n\r",
		      Xil_In32(PMC_GLOBAL_GICP2_IRQ_STATUS));
	XPmcFw_Printf(DEBUG_GENERAL, "GICP3 IRQ Status: 0x%08x\n\r",
		      Xil_In32(PMC_GLOBAL_GICP3_IRQ_STATUS));
	XPmcFw_Printf(DEBUG_GENERAL, "GICP4 IRQ Status: 0x%08x\n\r",
		      Xil_In32(PMC_GLOBAL_GICP4_IRQ_STATUS));
	XPmcFw_Printf(DEBUG_GENERAL, "GICP5 IRQ Status: 0x%08x\n\r",
		      Xil_In32(PMC_GLOBAL_GICP5_IRQ_STATUS));
	XPmcFw_Printf(DEBUG_GENERAL, "GICP6 IRQ Status: 0x%08x\n\r",
		      Xil_In32(PMC_GLOBAL_GICP6_IRQ_STATUS));
	XPmcFw_Printf(DEBUG_GENERAL, "GICP7 IRQ Status: 0x%08x\n\r",
		      Xil_In32(PMC_GLOBAL_GICP7_IRQ_STATUS));
	XPmcFw_Printf(DEBUG_GENERAL, "GICPPMC IRQ Status: 0x%08x\n\r",
		      Xil_In32(PMC_GLOBAL_GICP_PMC_IRQ_STATUS));

	XPmcFw_Printf(DEBUG_GENERAL, "====Register Dump============\n\r");
}
