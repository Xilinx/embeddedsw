/******************************************************************************
 * Copyright (c) 2024 - 2025 Advanced Micro Devices, Inc. All Rights Reserved.
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

/*****************************************************************************/
/**
 *
 * @file xplm_post_boot.c
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- -------------------------------------------------------
 * 1.00  ng   05/31/24 Initial release
 * 1.01  ng   11/05/24 Add boot time measurements
 *       ng   12/04/24 Fix secondary boot control
 *       sk   02/04/25 Make redundancy variable as volatile
 *       ng   02/11/25 Add Secure lockdown and tamper response support
 * </pre>
 *
 ******************************************************************************/

/**
 * @addtogroup spartanup_plm_apis SpartanUP PLM APIs
 * @{
 */

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
#include "xplm_init.h"
#include "xplm_util.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/
/** @cond spartanup_plm_internal */
static void XPlm_SetRunTimeEvent(const u32 Event);
static void XPlm_ClearRunTimeEvent(const u32 Event);
/** @endcond */

static void XPlm_EnableIntrSbiDataRdy(void);

/************************** Variable Definitions *****************************/
static volatile u32 RunTimeEvents = 0x0U; /**< To store the current Run Time event state. */

/******************************************************************************/

/*****************************************************************************/
/**
 * @brief	This function serves as the interrupt handler for the 'SBI Data Ready' event.
 *
 * @param	Data unused.
 *
 * @note	SBI interface setting for JTAG/SMAP should be set before this handler.
 *
 *****************************************************************************/
void XPlm_SbiLoadPdi(void *Data)
{
	(void)Data;

	/** - Disable the SBI data RDY interrupt. */
	XPlm_UtilRMW(SLAVE_BOOT_SBI_IRQ_DISABLE, SLAVE_BOOT_SBI_IRQ_DISABLE_DATA_RDY_MASK,
		     SLAVE_BOOT_SBI_IRQ_DISABLE_DATA_RDY_MASK);

	/** - Set run time event to trigger the partial PDI laoding. */
	XPlm_SetRunTimeEvent(XPLM_RUN_TIME_PARTIAL_PDI_EVENT);
}

/*****************************************************************************/
/**
 * @brief	This function clears any pending SBI Data Ready interrupts and then enables the
 * interrupt for future data ready events.
 *
 *****************************************************************************/
static void XPlm_EnableIntrSbiDataRdy(void)
{
	/** - Clear any existing SBI Data Ready interrupt. */
	XPlm_UtilRMW(SLAVE_BOOT_SBI_IRQ_STATUS, SLAVE_BOOT_SBI_IRQ_STATUS_DATA_RDY_MASK,
		     SLAVE_BOOT_SBI_IRQ_STATUS_DATA_RDY_MASK);

	/** - Enable the SBI Data Ready interrupt for subsequent events. */
	XPlm_UtilRMW(SLAVE_BOOT_SBI_IRQ_ENABLE, SLAVE_BOOT_SBI_IRQ_ENABLE_DATA_RDY_MASK,
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
	volatile u32 EfusePufhdInvalidBits;
	volatile u32 EfusePufhdInvalidBitsTmp;
	u32 SecBootRtcaCfg;
	u32 SecBootInterf;
	u32 SecBootEn;

	XPlm_LogPlmStage(XPLM_POST_BOOT_STAGE);

	XPlm_Printf(DEBUG_DETAILED, "Post Boot Configuration\n\r");

	/** - Clear All Run-Time Events. */
	XPlm_ClearRunTimeEvent(XPLM_ALLFS);

	XSECURE_REDUNDANT_IMPL(XPlm_CaptureCriticalInfo);

	/** - Disable PUF if PUFHD invalid efuse is blown. */
	EfusePufhdInvalidBits = Xil_In32(EFUSE_XILINX_CTRL) & EFUSE_XILINX_CTRL_PUFHD_INVLD_MASK;
	EfusePufhdInvalidBitsTmp = Xil_In32(EFUSE_XILINX_CTRL) & EFUSE_XILINX_CTRL_PUFHD_INVLD_MASK;
	if ((EfusePufhdInvalidBits != XPLM_ZERO) || (EfusePufhdInvalidBitsTmp != XPLM_ZERO)) {
		XSECURE_REDUNDANT_IMPL(Xil_Out32, PMC_GLOBAL_PUF_DISABLE, PMC_GLOBAL_PUF_DISABLE_PUF_MASK);
	}

	/**
	 * - Enable Secondary boot if enabled in RTCA register, set the boot interface and toggle
	 *   SBI reset.
	 */
	SecBootRtcaCfg = Xil_In32(XPLM_RTCFG_SEC_BOOT_CTRL);
	SecBootEn = SecBootRtcaCfg & XPLM_RTCFG_SEC_BOOT_CTRL_ENABLE_MASK;
	if (SecBootEn  == XPLM_RTCFG_SEC_BOOT_CTRL_ENABLE_MASK) {
		SecBootInterf = SecBootRtcaCfg & XPLM_RTCFG_SEC_BOOT_CTRL_BOOT_IF_MASK;
		SecBootInterf >>= XPLM_RTCFG_SEC_BOOT_CTRL_BOOT_IF_SHIFT;
		if ((SecBootInterf != XPLM_SBI_IF_JTAG) && (SecBootInterf != XPLM_SBI_IF_AXI_SLAVE)
		    && (SecBootInterf != XPLM_SBI_IF_MCAP)) {
			Status = (u32)XPLM_ERR_INVALID_SECONDARY_BOOT_INTF;
			goto END;
		}

		/* Take SBI out of reset */
		XPlm_UtilRMW(PMC_GLOBAL_RST_SBI, PMC_GLOBAL_RST_SBI_FULLMASK, XPLM_ZERO);

		/* Set SBI interface as per the user config */
		XPlm_UtilRMW(SLAVE_BOOT_SBI_CTRL,
			     SLAVE_BOOT_SBI_CTRL_INTERFACE_MASK | SLAVE_BOOT_SBI_CTRL_ENABLE_MASK,
			     ((SecBootInterf << XPLM_RTCFG_SEC_BOOT_CTRL_BOOT_IF_SHIFT) | SecBootEn));
	}

	XPlm_EnableIntrSbiDataRdy();

	Status = (u32)XST_SUCCESS;
END:
	return Status;
}

/** @cond spartanup_plm_internal */
static void XPlm_SetRunTimeEvent(const u32 Event)
{
	RunTimeEvents |= Event;
}

static void XPlm_ClearRunTimeEvent(const u32 Event)
{
	RunTimeEvents &= ~Event;
}
/** @endcond */

/*****************************************************************************/
/**
 * @brief	Run time event handler to process partial PDI on SBI interrupt.
 * This handler runs infinitely and does not return.
 *
 *****************************************************************************/
void XPlm_EventLoop(void)
{
	u32 Status = (u32)XST_FAILURE;

	/* Update PLM stage before entering infinite loop. */
	XPlm_LogPlmStage(XPLM_RUN_TIME_EVENT_PROCESS_STAGE);

	while (TRUE) {
		/**
		 * - On partial PDI load event
		 *	- Load partial PDI,
		 *	- reset SBI interrupt to receive SBI interrupts for loading partial PDIs
		 *	again.
		 * - If no event, update PLM stage and goto sleep.
		 */
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

/** @} end of spartanup_plm_apis group*/
