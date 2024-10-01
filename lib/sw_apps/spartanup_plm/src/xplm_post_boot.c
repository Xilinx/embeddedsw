/******************************************************************************
 * Copyright (c) 2024 Advanced Micro Devices, Inc. All Rights Reserved.
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

/*****************************************************************************/
/**
 *
 * @file xplm_intr.c
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- -------------------------------------------------------
 * 1.00  ng   05/31/24 Initial release
 * </pre>
 *
 ******************************************************************************/

/***************************** Include Files *********************************/
#include "xiomodule.h"
#include "xparameters.h"
#include "xil_exception.h"
#include "riscv_interface.h"
#include "xplm_debug.h"
#include "xplm_error.h"
#include "xplm_post_boot.h"
#include "xplm_hooks.h"
#include "xplm_hw.h"
#include "xplm_status.h"
#include "xplm_dma.h"
#include "xplm_load.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/
static void XPlm_SbiLoadPdi(void *Data);
static void XPlm_SetRunTimeEvent(const u32 Event);
static void XPlm_ClearRunTimeEvent(const u32 Event);
static void XPlm_EnableIntrSbiDataRdy(void);
static u32 XPlm_IntrInit(void);
/************************** Variable Definitions *****************************/
static volatile u32 RunTimeEvents = 0x0U;

/******************************************************************************/

/*****************************************************************************/
/**
 * @brief	Initializes interrupt for loading partial PDI through slave boot
 *
 * @return
 * 		- XST_SUCCESS on success and error code on failure.
 *
 *****************************************************************************/
static u32 XPlm_IntrInit(void)
{
	u32 Status = (u32)XST_FAILURE;
	static XIOModule IOModule;

	/** - Initialize the IO Module. */
	Status = XIOModule_Initialize(&IOModule, XPAR_IOMODULE_0_DEVICE_ID);
	if (Status != (u32)XST_SUCCESS) {
		Status = (u32)XPLM_ERR_IO_MOD_INIT;
		goto END;
	}

	riscv_disable_interrupts();
	/**
	 * - Register the SBI RDY interrupt to enable the PDI loading from
	 * SBI interface.
	 */
	(void)XIOModule_Connect(&IOModule, 17U, XPlm_SbiLoadPdi, (void *)0U);
	/*
	 * Register the IO module interrupt handler with the exception table.
	 */
	Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_INT,
				     (Xil_ExceptionHandler)XIOModule_DeviceInterruptHandler,
				     (void *)0);
	/** - Enable SBI data RDY interrupt. */
	XPlm_EnableIntrSbiDataRdy();
	/** - Enable the interrupt to receive interrupts for SBI RDY. */
	XIOModule_Enable(&IOModule, 17U);
	riscv_enable_interrupts();

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function is the interrupt handler for SBI data ready.
 * 		In this handler, PDI is loadeed through SBI interface.
 *
 * @param	Data Not used
 *
 * @note	SBI interface setting for JTAG/SMAP should be set before this handler.
 *
 *****************************************************************************/
static void XPlm_SbiLoadPdi(void *Data)
{
	(void)Data;

	/** - Disable the SBI data RDY interrupt. */
	XPlm_UtilRMW(SLAVE_BOOT_SBI_IRQ_DISABLE, SLAVE_BOOT_SBI_IRQ_DISABLE_DATA_RDY_MASK,
		     SLAVE_BOOT_SBI_IRQ_DISABLE_DATA_RDY_MASK);

	/** - Set run time event to trigger loading partial PDI. */
	XPlm_SetRunTimeEvent(XPLM_RUN_TIME_PARTIAL_PDI_EVENT);
}

/*****************************************************************************/
/**
 * @brief	This function enables IRQ for next interrupt.
 *
 *****************************************************************************/
static void XPlm_EnableIntrSbiDataRdy(void)
{
	/** - Clear SBI interrupt. */
	XPlm_UtilRMW(SLAVE_BOOT_SBI_IRQ_STATUS,
		      SLAVE_BOOT_SBI_IRQ_STATUS_DATA_RDY_MASK,
		      SLAVE_BOOT_SBI_IRQ_STATUS_DATA_RDY_MASK);

	/** - Enable SBI interrupt. */
	XPlm_UtilRMW(SLAVE_BOOT_SBI_IRQ_ENABLE,
		      SLAVE_BOOT_SBI_IRQ_ENABLE_DATA_RDY_MASK,
		      SLAVE_BOOT_SBI_IRQ_ENABLE_DATA_RDY_MASK);
}

/*****************************************************************************/
/**
 * @brief	Perform Post boot initialization tasks.
 *
 * @return
 *		- XST_SUCCESS on success.
 *		- XPLM_ERR_INVALID_SECONDARY_BOOT_INTF if the selected secondary
 *		boot interface is not valid.
 *
 *****************************************************************************/
u32 XPlm_PostBoot(void)
{
	u32 Status = (u32)XST_FAILURE;
	u32 EfusePufhdInvalidBits;
	u32 EfusePufhdInvalidBitsTmp;
	u32 SecBootConfig;
	u32 SecBootConfigTmp;
	u32 SecBootInterf;

	XPlm_LogPlmStage(XPLM_POST_BOOT_STAGE);

	XPlm_Printf(DEBUG_DETAILED, "Post Boot Configuration\n\r");

	/** Clear All Run-Time Events */
	XPlm_ClearRunTimeEvent(XPLM_ALLFS);

	XSECURE_REDUNDANT_IMPL(XPlm_CaptureCriticalInfo);

	/** - Disable PUF if PUFHD invalid efuse is blown. */
	EfusePufhdInvalidBits = Xil_In32(EFUSE_XILINX_CTRL) & EFUSE_XILINX_CTRL_PUFHD_INVLD_MASK;
	EfusePufhdInvalidBitsTmp = Xil_In32(EFUSE_XILINX_CTRL) & EFUSE_XILINX_CTRL_PUFHD_INVLD_MASK;
	if ((EfusePufhdInvalidBits != XPLM_ZERO) || (EfusePufhdInvalidBitsTmp != XPLM_ZERO)) {
		XSECURE_REDUNDANT_IMPL(Xil_Out32, PMC_GLOBAL_PUF_DISABLE, PMC_GLOBAL_PUF_DISABLE_PUF_MASK);
	}

	/** - Enable Secondary boot if enabled in RTCA register and set the boot interface. */
	SecBootConfig = Xil_In32(XPLM_RTCFG_SEC_BOOT_CTRL) & XPLM_RTCFG_SEC_BOOT_CTRL_ENABLE_MASK;
	SecBootConfigTmp = Xil_In32(XPLM_RTCFG_SEC_BOOT_CTRL) & XPLM_RTCFG_SEC_BOOT_CTRL_ENABLE_MASK;
	if ((SecBootConfig == XPLM_RTCFG_SEC_BOOT_CTRL_ENABLE_MASK)
	    && (SecBootConfigTmp == XPLM_RTCFG_SEC_BOOT_CTRL_ENABLE_MASK)) {
		SecBootInterf = (SecBootConfig & XPLM_RTCFG_SEC_BOOT_CTRL_BOOT_IF_MASK) >>
				XPLM_RTCFG_SEC_BOOT_CTRL_BOOT_IF_SHIFT;
		if ((SecBootInterf != XPLM_SBI_IF_JTAG) && (SecBootInterf != XPLM_SBI_IF_AXI_SLAVE)
		    && (SecBootInterf != XPLM_SBI_IF_MCAP)) {
			Status = (u32)XPLM_ERR_INVALID_SECONDARY_BOOT_INTF;
			goto END;
		}
		XPlm_UtilRMW(PMC_GLOBAL_RST_SBI, PMC_GLOBAL_RST_SBI_FULLMASK, XPLM_ZERO);
		XPlm_UtilRMW(SLAVE_BOOT_SBI_CTRL, SLAVE_BOOT_SBI_CTRL_INTERFACE_MASK |
				SLAVE_BOOT_SBI_CTRL_ENABLE_MASK, SecBootInterf);
	}

	Status = XPlm_IntrInit();
END:
	return Status;
}

static void XPlm_SetRunTimeEvent(const u32 Event)
{
	RunTimeEvents |= Event;
}

static void XPlm_ClearRunTimeEvent(const u32 Event)
{
	RunTimeEvents &= ~Event;
}

void XPlm_EventLoop(void) {
	u32 Status = (u32)XST_FAILURE;

	/* Update PLM stage before entering infinite loop. */
	XPlm_LogPlmStage(XPLM_RUN_TIME_EVENT_PROCESS_STAGE);

	while (TRUE) {
		if ((RunTimeEvents & XPLM_RUN_TIME_PARTIAL_PDI_EVENT) ==
				XPLM_RUN_TIME_PARTIAL_PDI_EVENT) {
			/* Load Partial PDI */
			Status = XPlm_LoadPartialPdi();
			if (Status != (u32)XST_SUCCESS) {
				XPlm_LogPlmErr(Status);
			}
			/* Clear the Partial PDI Event. */
			XPlm_ClearRunTimeEvent(XPLM_RUN_TIME_PARTIAL_PDI_EVENT);
			/* Enable the SBI interrupt. */
			XPlm_EnableIntrSbiDataRdy();
		}
		XPlm_LogPlmStage(XPLM_RUN_TIME_EVENT_PROCESS_STAGE);
		mb_sleep();
	}
}
