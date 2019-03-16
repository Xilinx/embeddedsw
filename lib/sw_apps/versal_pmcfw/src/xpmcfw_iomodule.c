/******************************************************************************
*
* Copyright (C) 2018 - 2019 Xilinx, Inc.  All rights reserved.
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
* @file xpmcfw_iomodule.c
*
* This is the file which contains iomodule interface functions.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- -------- -------------------------------------------------------
* 1.00  kc   03/27/2018 Initial release
*
* </pre>
*
* @note
*
******************************************************************************/

/***************************** Include Files *********************************/
#include "xpmcfw_iomodule.h"

/************************** Constant Definitions *****************************/
#define XPMCFW_MB_MSR_BIP_MASK		(0x8U)
/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/
static XIOModule IOModule; /* Instance of the IO Module */
extern XPmcFw PmcFwInstance; /* Instance of PMCFW instance */
/*****************************************************************************/
/**
* It initializes the Programmable Interval Timer
*
* @param	TimerNo PIT Timer to be initialized
* @param	ResetValue is the reset value of timer when started
* @return	None
*
*****************************************************************************/
void XPmcFw_InitPitTimer(u8 Timer, u32 ResetValue)
{
	/*
	 * When used in PIT1 prescalar to PIT2, PIT2 has least 32bits
	 * So, PIT2 is reloaded to get 64bit timer value.
	 */
	if (XPMCFW_PIT2 == Timer) {
		XIOModule_Timer_SetOptions(&IOModule, Timer,
				   XTC_AUTO_RELOAD_OPTION);
	}

	/*
	 * Set a reset value for the Programmable Interval Timers such that
	 * they will expire earlier than letting them roll over from 0, the
	 * reset value is loaded into the Programmable Interval Timers when
	 * they are started.
	 */
	XIOModule_SetResetValue(&IOModule, Timer, ResetValue);

	/*
	 * Start the Programmable Interval Timers and they are
	 * decrementing by default
	 */
	XIOModule_Timer_Start(&IOModule, Timer);
}

/*****************************************************************************/
/**
 *
 * This function is used to read the 64 bit timer value.
 * It reads from PIT1 and PIT2 and makes it 64bit
 *
 * @param       None
 *
 * @return      Returns 64 bit timer value
 *
 ******************************************************************************/
u64 XPmcFw_GetTimerValue(void )
{
	u64 TimerValue;
	u32 TPit1, TPit2;

	TPit1 = XIOModule_GetValue(&IOModule, (u8)XPMCFW_PIT1);
	TPit2 = XIOModule_GetValue(&IOModule, (u8)XPMCFW_PIT2);
	/* XPmcFw_Printf(DEBUG_INFO, "pit1 %08x pit2 %08x\r\n", TPit1, TPit2); */

	/**
	* Pit1 starts at 0 and preload the full value
	* after pit2 expires. So, recasting TPit1 0 value
	* to highest so that u64 comparison works fo
	* Tpit1 0 and TPit1 0xfffffffe
	*/
	if (TPit1 == 0U)
	{
		TPit1 = 0xfffffffeU;
	}

	TimerValue = (((u64)TPit1) << 32) | (u64)TPit2;
	return TimerValue;
}

/*****************************************************************************/
/**
 * This function measures the total time taken between two points for
 * performance measurement.
 *
 * @param Start time
 *
 * @return none
 *****************************************************************************/
void XPmcFw_MeasurePerfTime(u64 tCur)
{
	u64 tEnd = 0;
	u64 tDiff = 0;
	u64 tPerfNs;
	u64 tPerfMs = 0;
	u64 tPerfMsFrac = 0;

	tEnd = XPmcFw_GetTimerValue();
	tDiff = tCur - tEnd;

	/* Convert tPerf into nanoseconds */
	tPerfNs = ((double)tDiff / (double)XPAR_CPU_CORE_CLOCK_FREQ_HZ) * 1e9;

	tPerfMs = tPerfNs / 1e6;
	tPerfMsFrac = tPerfNs % (u64)1e6;

	/* Print the whole (in ms.) and fractional part */
	XPmcFw_Printf(DEBUG_PRINT_ALWAYS, "%d.%06d ms.",
			(u32)tPerfMs, (u32)tPerfMsFrac);
}

/*****************************************************************************/
/**
 *
 * This function is called from an interrupt handler when GIC interrupt comes
 *
 * @param       None
 *
 * @return      None
 *
 ******************************************************************************/
static void XPmcFw_GicIrqHandler(void)
{
	u32 RegVal;

	RegVal = Xil_In32(PMC_GLOBAL_GICP_PMC_IRQ_STATUS);
	if ((RegVal & PMC_GLOBAL_GICP_PMC_IRQ_STATUS_SRC0_MASK) ==
				PMC_GLOBAL_GICP_PMC_IRQ_STATUS_SRC0_MASK) {

		/* Handle the GICP0 interrupt */
		RegVal = Xil_In32(PMC_GLOBAL_GICP0_IRQ_STATUS);
		if ((RegVal & PMC_GLOBAL_GICP0_IRQ_STATUS_SRC13_MASK) != FALSE) {
			Xil_Out32(PMC_GLOBAL_GICP0_IRQ_STATUS,
					RegVal&(0x1U<<PMC_GLOBAL_GICP0_IRQ_STATUS_SRC13_SHIFT));
			Xil_Out32(PMC_GLOBAL_GICP_PMC_IRQ_STATUS,
					(0x1U<<PMC_GLOBAL_GICP_PMC_IRQ_STATUS_SRC0_SHIFT));
			PmcFwInstance.MetaHdr.DeviceCopy = XPmcFw_MemCopy;
			PmcFwInstance.DeviceOps.Copy = XPmcFw_MemCopy;
			(void)XPmcFw_PdiLoad(&PmcFwInstance);
		}

	} else if ((RegVal & PMC_GLOBAL_GICP_PMC_IRQ_STATUS_SRC1_MASK) ==
						PMC_GLOBAL_GICP_PMC_IRQ_STATUS_SRC1_MASK) {
		/* Handle the GICP1 interrupt */

	} else if ((RegVal & PMC_GLOBAL_GICP_PMC_IRQ_STATUS_SRC2_MASK) ==
						PMC_GLOBAL_GICP_PMC_IRQ_STATUS_SRC2_MASK) {

		/* Handle the GICP2 interrupt */
		RegVal = Xil_In32(PMC_GLOBAL_GICP2_IRQ_STATUS);
		if ((RegVal & PMC_GLOBAL_GICP2_IRQ_STATUS_SRC8_MASK) != FALSE) {

			XPmcFw_Printf(DEBUG_GENERAL,"Received CPM misc interrupt\r\n");
#ifdef XPMCFW_SBI
			PmcFwInstance.MetaHdr.DeviceCopy = XPmcFw_SbiCopy;
			PmcFwInstance.DeviceOps.Copy = XPmcFw_SbiCopy;

			XPmcFw_SbiConfig(XPMCFW_SBI_CTRL_INTERFACE_AXI_SLAVE);

			Xil_Out32(XPMCFW_XDMA_SCRATCH_PAD_REG1,
					XPMCFW_SBI_DATA_RECV_READY);
			(void)XPmcFw_PdiLoad(&PmcFwInstance);
#endif
			Xil_Out32(CPM_SLCR_PS_MISC_IR_STATUS,
					CPM_SLCR_PS_MISC_IR_STATUS_PCIE_LOCAL_EVENT_MASK);
			Xil_Out32(PMC_GLOBAL_GICP2_IRQ_STATUS,
					PMC_GLOBAL_GICP2_IRQ_STATUS_SRC8_MASK);
			Xil_Out32(PMC_GLOBAL_GICP_PMC_IRQ_STATUS,
					PMC_GLOBAL_GICP_PMC_IRQ_STATUS_SRC2_MASK);
		}
	} else if ((RegVal & PMC_GLOBAL_GICP_PMC_IRQ_STATUS_SRC3_MASK) ==
						PMC_GLOBAL_GICP_PMC_IRQ_STATUS_SRC3_MASK) {
		/* Handle the GICP3 interrupt */

	} else if ((RegVal & PMC_GLOBAL_GICP_PMC_IRQ_STATUS_SRC4_MASK) ==
						PMC_GLOBAL_GICP_PMC_IRQ_STATUS_SRC4_MASK) {

		/* Handle the GICP4 interrupt */
		RegVal = Xil_In32(PMC_GLOBAL_GICP4_IRQ_STATUS);
		if ((RegVal & PMC_GLOBAL_GICP4_IRQ_STATUS_SRC8_MASK) != FALSE) {
			Xil_Out32(PMC_GLOBAL_GICP4_IRQ_STATUS,
					RegVal&(0x1U<<PMC_GLOBAL_GICP4_IRQ_STATUS_SRC8_SHIFT));
			Xil_Out32(PMC_GLOBAL_GICP_PMC_IRQ_STATUS,
					(0x1U<<PMC_GLOBAL_GICP_PMC_IRQ_STATUS_SRC0_SHIFT));
			XPmcFw_RMW32(SLAVE_BOOT_SBI_IRQ_STATUS,
                                SLAVE_BOOT_SBI_IRQ_ENABLE_DATA_RDY_MASK,
                                SLAVE_BOOT_SBI_IRQ_ENABLE_DATA_RDY_MASK);
#ifdef XPMCFW_SBI
			PmcFwInstance.MetaHdr.DeviceCopy = XPmcFw_SbiCopy;
			PmcFwInstance.DeviceOps.Copy = XPmcFw_SbiCopy;
			(void)XPmcFw_PdiLoad(&PmcFwInstance);
#endif
		}
	} else if ((RegVal & PMC_GLOBAL_GICP_PMC_IRQ_STATUS_SRC5_MASK) ==
						PMC_GLOBAL_GICP_PMC_IRQ_STATUS_SRC5_MASK) {

		/* Handle the GICP5 interrupt */

	} else if ((RegVal & PMC_GLOBAL_GICP_PMC_IRQ_STATUS_SRC6_MASK) ==
						PMC_GLOBAL_GICP_PMC_IRQ_STATUS_SRC6_MASK) {

		/* Handle the GICP6 interrupt */

	} else if ((RegVal & PMC_GLOBAL_GICP_PMC_IRQ_STATUS_SRC7_MASK) ==
						PMC_GLOBAL_GICP_PMC_IRQ_STATUS_SRC7_MASK) {

		/* Handle the GICP7 interrupt */

	}
}

/*****************************************************************************/
/**
 *
 * This function is called from an interrupt handler when GPI interrupt occurs
 *
 * @param       None
 *
 * @return      None
 *
 ******************************************************************************/
static void XPmcFw_GpiIrqHandler(void)
{
	u32 RegVal;
	RegVal = Xil_In32(PMC_GLOBAL_PMC_PPU1_GPI);
	if((RegVal & PMC_GLOBAL_PMC_PPU1_GPI_GPI_0_MASK) != FALSE)
	{
		Xil_Out32(PMC_GLOBAL_PMC_PPU1_GPI,
		RegVal&(0x1U<<PMC_GLOBAL_PMC_PPU1_GPI_GPI_0_SHIFT));
#ifdef XPMCFW_SBI
		PmcFwInstance.MetaHdr.DeviceCopy = XPmcFw_SbiCopy;
		PmcFwInstance.DeviceOps.Copy = XPmcFw_SbiCopy;
		(void)XPmcFw_PdiLoad(&PmcFwInstance);
#endif
	}

	if((RegVal & PMC_GLOBAL_PMC_PPU1_GPI_GPI_1_MASK) != FALSE)
	{
		Xil_Out32(PMC_GLOBAL_PMC_PPU1_GPI,
		RegVal&(0x1U<<PMC_GLOBAL_PMC_PPU1_GPI_GPI_1_SHIFT));
		PmcFwInstance.MetaHdr.DeviceCopy = XPmcFw_MemCopy;
		PmcFwInstance.DeviceOps.Copy = XPmcFw_MemCopy;
		(void)XPmcFw_PdiLoad(&PmcFwInstance);
	}
}

/*****************************************************************************/
/**
 *
 * This function is called from an interrupt handler when Error interrupt comes
 *
 * @param       None
 *
 * @return      None
 *
 ******************************************************************************/
static void XPmcFw_ErrIrqHandler(void)
{
	u32 RegVal;

	RegVal = Xil_In32(PMC_GLOBAL_PMC_ERR1_STATUS);
	if(((RegVal & PMC_GLOBAL_PMC_ERR1_STATUS_SSIT_ERR3_MASK) ||
		(RegVal & PMC_GLOBAL_PMC_ERR1_STATUS_SSIT_ERR4_MASK) ||
		(RegVal & PMC_GLOBAL_PMC_ERR1_STATUS_SSIT_ERR5_MASK)) != FALSE) {

		XPmcFw_Printf(DEBUG_GENERAL,"Received SSIT error interrupt\r\n");
		/* Clear the interrupt */
		Xil_Out32(PMC_GLOBAL_PMC_ERR1_STATUS,
				(RegVal & (PMC_GLOBAL_PMC_ERR1_STATUS_SSIT_ERR3_MASK |
							PMC_GLOBAL_PMC_ERR1_STATUS_SSIT_ERR4_MASK |
							PMC_GLOBAL_PMC_ERR1_STATUS_SSIT_ERR5_MASK)));
		/* Update error register and perform fallback */
		Xil_Out32(PMC_GLOBAL_PMC_FW_ERR, XPMCFW_ERR_SSIT_BOOT);
		XPmcFw_ErrorLockDown(XPMCFW_ERR_SSIT_BOOT);
	}
}

/* Structure for Top level interrupt table */
static struct HandlerTable g_TopLevelInterruptTable[] = {
	{PPU1_IOMODULE_IRQ_PENDING_PMC_GIC_IRQ_MASK, XPmcFw_GicIrqHandler},
	{PPU1_IOMODULE_IRQ_PENDING_ERR_IRQ_MASK, XPmcFw_ErrIrqHandler},
	{PPU1_IOMODULE_IRQ_PENDING_PMC_GPI_MASK, XPmcFw_GpiIrqHandler}
};

/*****************************************************************************/
/**
* It initializes the IO module strutures and PIT timers
* @return	XST_SUCCESS if the initialization is successful
*
*****************************************************************************/
XStatus XPmcFw_InitIOModule()
{
	XStatus Status;

	/*
	 * Initialize the IO Module so that it's ready to use,
	 * specify the device ID that is generated in xparameters.h
	 */
	Status = XIOModule_Initialize(&IOModule, IOMODULE_DEVICE_ID);
	if (Status != XST_SUCCESS) {
		Status = XPMCFW_UPDATE_ERR(XPMCFW_ERR_IOMOD_INIT, Status);
		goto END;
	}

	/*
	 * Perform a self-test to ensure that the hardware was built correctly.
	 */
	//Status = XIOModule_SelfTest(&IOModule);
	Status = XIOModule_Start(&IOModule);
	if (Status != XST_SUCCESS) {
		Status = XPMCFW_UPDATE_ERR(XPMCFW_ERR_IOMOD_START, Status);
		goto END;
	}

	/** Initialize and start the timer
	 *  Use PIT1 and PIT2 in prescalor mode
	 */
	/* Setting for Prescaler mode */
	Xil_Out32(IOModule.BaseAddress + XGO_OUT_OFFSET,
			   MB_IOMODULE_GPO1_PIT1_PRESCALE_SRC_MASK);
	XPmcFw_InitPitTimer((u8)XPMCFW_PIT2,
			    XPMCFW_PIT2_RESET_VALUE);
	XPmcFw_InitPitTimer((u8)XPMCFW_PIT1,
			    XPMCFW_PIT1_RESET_VALUE);
END:
	return Status;
}

/******************************************************************************/
/**
*
* This function connects the interrupt handler of the IO Module to the
* processor.
* @param    None.
*
* @return	XST_SUCCESS if handlers are registered properly
****************************************************************************/
XStatus XPmcFw_SetUpInterruptSystem()
{
	XStatus Status;
	u32 IntrNum;

	/*
	 * Connect a device driver handler that will be called when an interrupt
	 * for the device occurs, the device driver handler performs the specific
	 * interrupt processing for the device
	 */
	for (IntrNum = 0U; IntrNum < XPAR_IOMODULE_INTC_MAX_INTR_SIZE; IntrNum++)
	{
		Status = XIOModule_Connect(&IOModule, IntrNum,
				   (XInterruptHandler) XPmcFw_IntrHandler,
				   (void *)IntrNum);
		if (Status != XST_SUCCESS)
		{
			Status = XPMCFW_UPDATE_ERR(
					XPMCFW_ERR_IOMOD_CONNECT, Status);
			goto END;
		}
	}

	/*
	 * TODO: Enable SSIT Error interrupts in the respective module
	 */
	Xil_Out32(PMC_GLOBAL_PMC_IRQ1_EN, PMC_GLOBAL_PMC_IRQ1_EN_SSIT_ERR3_MASK |
				PMC_GLOBAL_PMC_IRQ1_EN_SSIT_ERR4_MASK |
				PMC_GLOBAL_PMC_IRQ1_EN_SSIT_ERR5_MASK);
	/*
         * TODO: Enable GICP0 interrupt in the respective module
         */
        XPmcFw_RMW32(PMC_GLOBAL_GICP_PMC_IRQ_ENABLE,
                                PMC_GLOBAL_GICP_PMC_IRQ_ENABLE_SRC0_MASK,
                                PMC_GLOBAL_GICP_PMC_IRQ_ENABLE_SRC0_MASK);
	/*
	 * TODO: Enable GICP2 interrupt in the respective module
	 */
	XPmcFw_RMW32(PMC_GLOBAL_GICP_PMC_IRQ_ENABLE,
				PMC_GLOBAL_GICP_PMC_IRQ_ENABLE_SRC2_MASK,
				PMC_GLOBAL_GICP_PMC_IRQ_ENABLE_SRC2_MASK);
	/*
     * TODO: Enable GICP4 interrupt in the respective module
     */
	XPmcFw_RMW32(PMC_GLOBAL_GICP_PMC_IRQ_ENABLE,
			PMC_GLOBAL_GICP_PMC_IRQ_ENABLE_SRC4_MASK,
				PMC_GLOBAL_GICP_PMC_IRQ_ENABLE_SRC4_MASK);
	/*
         * TODO: Enable Readback mask in the respective module
         */
	XPmcFw_RMW32(PMC_GLOBAL_GICP0_IRQ_ENABLE,
                                PMC_GLOBAL_GICP0_IRQ_ENABLE_SRC13_MASK,
                                PMC_GLOBAL_GICP0_IRQ_ENABLE_SRC13_MASK);
	XPmcFw_RMW32(PMC_GLOBAL_GICP4_IRQ_ENABLE,
                                PMC_GLOBAL_GICP0_IRQ_ENABLE_SRC8_MASK,
                                PMC_GLOBAL_GICP0_IRQ_ENABLE_SRC8_MASK);
	XPmcFw_RMW32(SLAVE_BOOT_SBI_IRQ_STATUS,
                                SLAVE_BOOT_SBI_IRQ_ENABLE_DATA_RDY_MASK,
                                SLAVE_BOOT_SBI_IRQ_ENABLE_DATA_RDY_MASK);
	XPmcFw_RMW32(SLAVE_BOOT_SBI_IRQ_ENABLE,
                                SLAVE_BOOT_SBI_IRQ_ENABLE_DATA_RDY_MASK,
                                SLAVE_BOOT_SBI_IRQ_ENABLE_DATA_RDY_MASK);

	/*
         * TODO: Enable PR mask in the respective module
         */
        XPmcFw_RMW32(PMC_GLOBAL_PMC_PPU1_GPI_EN,
				PMC_GLOBAL_PMC_PPU1_GPI_EN_GPI_0_MASK |
				PMC_GLOBAL_PMC_PPU1_GPI_EN_GPI_1_MASK,
				PMC_GLOBAL_PMC_PPU1_GPI_EN_GPI_0_MASK |
				PMC_GLOBAL_PMC_PPU1_GPI_EN_GPI_1_MASK);
	/*
	 * TODO: Enable CPM misc events interrupt in the respective module
	 */
	XPmcFw_RMW32(PMC_GLOBAL_GICP2_IRQ_ENABLE,
				PMC_GLOBAL_GICP2_IRQ_ENABLE_SRC8_MASK,
				PMC_GLOBAL_GICP2_IRQ_ENABLE_SRC8_MASK);
#ifdef XPMCFW_CPM
	Xil_Out32(CPM_SLCR_PS_MISC_IR_ENABLE,
				CPM_SLCR_PS_MISC_IR_ENABLE_PCIE_LOCAL_EVENT_MASK);
	XPmcFw_RMW32(XPMCFW_PCIE_INTR_EN_REG,
				XPMCFW_PCIE_DOORBELL_INTR_MASK,
				XPMCFW_PCIE_DOORBELL_INTR_MASK);
#endif

	/*
	 * Enable interrupts for the device and then cause interrupts so the
	 * handlers will be called.
	 */
	for (IntrNum = XIN_IOMODULE_EXTERNAL_INTERRUPT_INTR; 
		IntrNum< XPAR_IOMODULE_INTC_MAX_INTR_SIZE; IntrNum++)
	{
		XIOModule_Enable(&IOModule, IntrNum);
	}

	/*
	 * Register the IO module interrupt handler with the exception table.
	 */
	Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_INT,
	     (Xil_ExceptionHandler)XIOModule_DeviceInterruptHandler,
		     (void*) IOMODULE_DEVICE_ID);

	/*
	 * Enable interrupts
	 */
	microblaze_enable_interrupts();

	/*
	 * Clear Break in progress to get interrupts
	 */
	mtmsr(mfmsr() & (~XPMCFW_MB_MSR_BIP_MASK));
END:
	return Status;
}

/******************************************************************************/
/**
* This function is an interrupt handler for the device.
* @param    CallbackRef is presently the interrupt number that is received
* @return   None.
****************************************************************************/
void XPmcFw_IntrHandler(void *CallbackRef)
{
	u32 l_IrqReg;
	u32 l_index;

	XPmcFw_Printf(DEBUG_GENERAL,
	      "Received Interrupt: 0x%0x\n\r", (u32) CallbackRef);

	l_IrqReg = Xil_In32(PPU1_IOMODULE_IRQ_PENDING);

	for(l_index = 0U; l_index < ARRAYSIZE(g_TopLevelInterruptTable);
		l_index++) {
			if ((l_IrqReg & g_TopLevelInterruptTable[l_index].Mask)
					== g_TopLevelInterruptTable[l_index].Mask) {
				/* Call interrupt handler */
				g_TopLevelInterruptTable[l_index].Handler();

				/* ACK the interrupt */
				Xil_Out32(PPU1_IOMODULE_IRQ_ACK,
				        g_TopLevelInterruptTable[l_index].Mask);
			}
	}

}
