/******************************************************************************
* Copyright (C) 2018-2019 Xilinx, Inc. All rights reserved.
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
* Use of the Software is limited solely to applications:
* (a) running on a Xilinx device, or
* (b) that interact with a Xilinx device through a bus or interconnect.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
* XILINX  BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
* WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
* OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*
* Except as contained in this notice, the name of the Xilinx shall not be used
* in advertising or otherwise to promote the sale, use or other dealings in
* this Software without prior written authorization from Xilinx.
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xplm_main.c
*
* This is the main file which contains code for the PLM.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ====  ==== ======== ======================================================-
* 1.00  kc   07/12/2018 Initial release
*
* </pre>
*
* @note
*
******************************************************************************/

/***************************** Include Files *********************************/
#include "xplm_main.h"
#include "xplmi_debug.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/

/*****************************************************************************/
/**
 * @brief This is PLM main function
 *
 * @param	None
 *
 * @return	Ideally should not return, incase if it reaches end,
 *          error is returned
 *
 *****************************************************************************/
int main(void )
{
	int Status;

#ifdef DEBUG_UART_MDM
	/** If MDM UART, banner can be printed before any initialization */
	XPlm_InitUart();
#endif

	/** Initialize the processor, tasks lists */
	Status = XPlm_Init();
	if (Status != XST_SUCCESS)
	{
		XPlmi_ErrMgr(Status);
	}

	/** Initialize the start up events */
	Status = XPlm_AddStartUpTasks();
	if (Status != XST_SUCCESS)
	{
		XPlmi_ErrMgr(Status);
	}

	/** Run the handlers in task loop based on the priority */
	XPlmi_TaskDispatchLoop();

	/** should never reach here */
	while(1);
	return XPLM_FAILURE;
}

/*****************************************************************************/
/**
 * @brief This function processor and task structures
 *
 * @param	None
 *
 * @return	XST_SUCCESS
 *
 *****************************************************************************/
int XPlm_Init()
{
	int Status;

	/**
	 * Reset the wakeup signal set by ROM
	 * Otherwise MB will always wakeup, irrespective of the sleep state
	 */
	XPlmi_PpuWakeUpDis();

	/* Initialize the processor, enable exceptions */
	Status = XPlm_InitProc();
	if (Status != XST_SUCCESS)
	{
		goto END;
	}

	/* Initialize the tasks lists */
	XPlmi_TaskInit();
END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief This function initializes the PS/MDM uart and prints
 * PLM banner.
 *
 * @param	None
 *
 * @return	Status of the UART initialization
 *
 *****************************************************************************/
int XPlm_InitUart()
{
	int Status;

	/**
	 * TODO If UART is defined, can we initialize UART with default
	 * HW values so that we can print from the start
	 */
	/* Initialize UART */
	Status = XPlmi_InitUart();
	if (Status != XST_SUCCESS)
	{
		goto END;
	}

	/** Print PLM banner  */
	XPlm_PrintPlmBanner();

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief This function prints PMC FW banner
 *
 * @param none
 *
 * @return	none
 *
 *****************************************************************************/
void XPlm_PrintPlmBanner(void )
{
	u32 Version;
	u32 PlatformVersion;
	u32 Platform;
	u32 RtlVersion;
	u32 PsVersion;
	u32 PmcVersion;

	/* Print the PLM Banner */
	XPlmi_Printf(DEBUG_PRINT_ALWAYS,
                 "\n\r****************************************\n\r");
	XPlmi_Printf(DEBUG_PRINT_ALWAYS,
                 "Xilinx versal Platform Loader and Manager \n\r");
	XPlmi_Printf(DEBUG_PRINT_ALWAYS,
                 "Release %s.%s   %s  -  %s\n\r",
                 SDK_RELEASE_YEAR, SDK_RELEASE_QUARTER, __DATE__, __TIME__);

	/* Read the Version */
	Version = XPlmi_In32(PMC_TAP_VERSION);
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
			XPlmi_Printf(DEBUG_PRINT_ALWAYS, "Silicon: "); break;
		case PMC_TAP_VERSION_SPP:
			XPlmi_Printf(DEBUG_PRINT_ALWAYS, "SPP: "); break;
		case PMC_TAP_VERSION_EMU:
			XPlmi_Printf(DEBUG_PRINT_ALWAYS, "EMU: "); break;
		case PMC_TAP_VERSION_QEMU:
			XPlmi_Printf(DEBUG_PRINT_ALWAYS, "QEMU: "); break;
		default:break;
	}

	XPlmi_Printf(DEBUG_PRINT_ALWAYS, "v%d, ", PlatformVersion);
	XPlmi_Printf(DEBUG_PRINT_ALWAYS, "RTL: ITR%d, ", (RtlVersion/16));
	XPlmi_Printf(DEBUG_PRINT_ALWAYS, "PMC: v%d.%d, ",
				(PmcVersion/16), PmcVersion%16);
	XPlmi_Printf(DEBUG_PRINT_ALWAYS, "PS: v%d.%d",
				(PsVersion/16), PsVersion%16);
	XPlmi_Printf(DEBUG_PRINT_ALWAYS, "\n\r");
#ifdef DEBUG_UART_MDM
	XPlmi_Printf(DEBUG_INFO, "STDOUT: MDM UART \n\r");
#else
	XPlmi_Printf(DEBUG_INFO, "STDOUT: PS UART \n\r");
#endif
	XPlmi_Printf(DEBUG_PRINT_ALWAYS,
                 "****************************************\n\r");
}

