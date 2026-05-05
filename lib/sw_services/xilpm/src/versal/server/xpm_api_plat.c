/******************************************************************************
* Copyright (c) 2018 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2026, Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

#include "xil_util.h"
#include "xplmi_ipi.h"
#include "xplmi_util.h"
#include "xplmi_ssit.h"
#include "xpm_api.h"
#include "xpm_api_plat.h"
#include "xpm_defs.h"
#include "xpm_psm_api.h"
#include "xpm_pldomain.h"
#include "xpm_pll.h"
#include "xpm_powerdomain.h"
#include "xpm_pmcdomain.h"
#include "xpm_pslpdomain.h"
#include "xpm_psfpdomain.h"
#include "xpm_npdomain.h"
#include "xpm_cpmdomain.h"
#include "xpm_psm.h"
#include "xpm_pmc.h"
#include "xpm_periph.h"
#include "xpm_mem.h"
#include "xpm_apucore.h"
#include "xpm_rpucore.h"
#include "xpm_power.h"
#include "xplmi.h"
#include "xplmi_modules.h"
#include "xpm_aie.h"
#include "xpm_requirement.h"
#include "xpm_regs.h"
#include "xpm_ioctl.h"
#include "xpm_ipi.h"
#include "xsysmonpsv.h"
#include "xpm_notifier.h"
#include "xplmi_error_node.h"
#include "xpm_rail.h"
#include "xpm_pldevice.h"
#include "xpm_aiedevice.h"
#include "xpm_debug.h"
#include "xpm_device.h"
#include "xpm_regulator.h"
#include "xplmi_scheduler.h"
#include "xplmi_sysmon.h"
#include "xpm_access.h"
#include "xpm_noc_config.h"
#include "xpm_ams_trim.h"
#include "xplmi_gic_interrupts.h"
#include "xplmi_generic.h"
#include "xpm_pin_plat.h"
#include "xpm_pin.h"
#define XPm_RegisterWakeUpHandler(GicId, SrcId, NodeId)	\
	{ \
		Status = XPlmi_GicRegisterHandler(GicId, SrcId, \
				&XPm_DispatchWakeHandler, (void *)(NodeId)); \
		if (Status != XST_SUCCESS) {\
			goto END;\
		}\
	}

/* Macro to typecast PM API ID */
#define PM_API(ApiId)			((u32)ApiId)

/*
 * Macro for exporting xilpm command details. Use in the first line of commands
 * used in CDOs.
 */
#define XPM_EXPORT_CMD(CmdIdVal, MinArgCntVal, MaxArgCntVal) \
    XPLMI_EXPORT_CMD(CmdIdVal, XPLMI_MODULE_XILPM_ID, MinArgCntVal, MaxArgCntVal)

#define PM_IOCTL_FEATURE_BITMASK ( \
	(1ULL << (u64)IOCTL_GET_RPU_OPER_MODE) | \
	(1ULL << (u64)IOCTL_SET_RPU_OPER_MODE) | \
	(1ULL << (u64)IOCTL_RPU_BOOT_ADDR_CONFIG) | \
	(1ULL << (u64)IOCTL_TCM_COMB_CONFIG) | \
	(1ULL << (u64)IOCTL_SET_TAPDELAY_BYPASS) | \
	(1ULL << (u64)IOCTL_SD_DLL_RESET) | \
	(1ULL << (u64)IOCTL_SET_SD_TAPDELAY) | \
	(1ULL << (u64)IOCTL_SET_PLL_FRAC_MODE) | \
	(1ULL << (u64)IOCTL_GET_PLL_FRAC_MODE) | \
	(1ULL << (u64)IOCTL_SET_PLL_FRAC_DATA) | \
	(1ULL << (u64)IOCTL_GET_PLL_FRAC_DATA) | \
	(1ULL << (u64)IOCTL_WRITE_GGS) | \
	(1ULL << (u64)IOCTL_READ_GGS) | \
	(1ULL << (u64)IOCTL_WRITE_PGGS) | \
	(1ULL << (u64)IOCTL_READ_PGGS) | \
	(1ULL << (u64)IOCTL_SET_BOOT_HEALTH_STATUS) | \
	(1ULL << (u64)IOCTL_OSPI_MUX_SELECT) | \
	(1ULL << (u64)IOCTL_USB_SET_STATE) | \
	(1ULL << (u64)IOCTL_GET_LAST_RESET_REASON) | \
	(1ULL << (u64)IOCTL_AIE_ISR_CLEAR) | \
	(1ULL << (u64)IOCTL_READ_REG) | \
	(1ULL << (u64)IOCTL_MASK_WRITE_REG) | \
	(1ULL << (u64)IOCTL_AIE_OPS) | \
	(1ULL << (u64)IOCTL_GET_QOS) | \
	(1ULL << (u64)IOCTL_SET_AIE_CLK_DIV) | \
	(1ULL << (u64)IOCTL_PREPARE_DDR_SHUTDOWN)) | \
	(1ULL << (u64)IOCTL_GET_SSIT_TEMP)

#ifdef ENABLE_GPIO_PROC_HANDLER
static XPm_GpioProcCfg GpioProcCfg[MAX_GPIO_PROC_PINS];
static u32 GpioProcCfgCount = 0U;
#endif /* ENABLE_GPIO_PROC_HANDLER */

#ifdef PLM_ENABLE_STL
static void (*StlCbInitFinishHandler)(void);
static inline void XPmPlDevice_TriggerStlHandler(void)
{
	if (NULL != StlCbInitFinishHandler) {
		StlCbInitFinishHandler();
	}
}
/****************************************************************************/
/**
 * @brief  Set XilStl callback handler to execute STLs at the end of PLD init
 * finish command processing
 *
 * @param  Handler: Pointer to XilStl callback handler
 *
 * @return None
 *
 * @note None
 *
 ****************************************************************************/
void XPmPlDevice_SetStlInitFinishCb(void (*Handler)(void))
{
	if (NULL == StlCbInitFinishHandler) {
		StlCbInitFinishHandler = Handler;
	}
}
#else
static inline void XPmPlDevice_TriggerStlHandler(void)
{
	return;
}
void XPmPlDevice_SetStlInitFinishCb(void (*Handler)(void))
{
	(void)Handler;
}
#endif /* PLM_ENABLE_STL */

XStatus IsOnSecondarySLR(u32 SubsystemId)
{
	XStatus Status = XST_FAILURE;
	(void)SubsystemId;

#ifdef PLM_ENABLE_PLM_TO_PLM_COMM
	/*
	 * Ideally, we'd like to pass the source info "securely" from master
	 * to slave SLRs, however until such means are available,
	 * use SubsystemId as 0U - which will let slave SLRs know that this
	 * is likely a forwarded command from master. Each command handler on
	 * slave SLR will check validity of SubsystemId so unauthorized
	 * commands will not get executed.
	 */
	if ((0U == SubsystemId) && (XPLMI_SSIT_MASTER_SLR_INDEX != XPlmi_GetSlrIndex())) {
		Status = XST_SUCCESS;
	}
#endif /* PLM_ENABLE_PLM_TO_PLM_COMM */

	return Status;
}

int XPm_PlatProcessCmd(XPlmi_Cmd * Cmd)
{
	int Status = XST_FAILURE;
	const u32 *Pload = Cmd->Payload;
	u32 CmdId = Cmd->CmdId & 0xFFU;
	u32 Len = Cmd->Len;

	switch (CmdId) {
	case PM_API(PM_INIT_NODE):
		Status = XPm_InitNode(Pload[0], Pload[1], &Pload[2], Len-2U);
		break;
	case PM_API(PM_NOC_CLOCK_ENABLE):
		Status = XPm_NocClockEnable(Pload[0], &Pload[1], Len-1U);
		break;
	case PM_API(PM_IF_NOC_CLOCK_ENABLE):
		Status = XPm_IfNocClockEnable(Cmd, &Pload[0], Len);
		break;
	case PM_API(PM_FORCE_HOUSECLEAN):
		Status = XPm_ForceHouseClean(Pload[0]);
		break;
	default:
		PmErr("CMD: INVALID PARAM\r\n");
		Status = XST_INVALID_PARAM;
		break;
	}

	return Status;
}

/*****************************************************************************/
/**
 * @brief This is the handler for wake up interrupts
 *
 * @param  DeviceIdx	Index of peripheral device
 *
 * @return Status	XST_SUCCESS if processor wake successfully
 *			XST_FAILURE or error code in case of failure
 *
 *****************************************************************************/
static int XPm_DispatchWakeHandler(void *DeviceIdx)
{
	XStatus Status;

	Status = XPm_GicProxyWakeUp((u32)DeviceIdx);
	return Status;
}

/****************************************************************************/
/**
 * @brief Register wakeup handlers with XilPlmi
 * @return XST_SUCCESS on success and error code on failure
 ****************************************************************************/
int XPm_RegisterWakeUpHandlers(void)
{
	int Status = XST_FAILURE;
	/**
	 * Register the events for PM
	 */
	XPm_RegisterWakeUpHandler((u32)XPLMI_PMC_GIC_IRQ_GICP0, (u32)XPLMI_GICP0_SRC14, XPM_NODEIDX_DEV_I2C_0);
	XPm_RegisterWakeUpHandler((u32)XPLMI_PMC_GIC_IRQ_GICP0, (u32)XPLMI_GICP0_SRC15, XPM_NODEIDX_DEV_I2C_1);
	XPm_RegisterWakeUpHandler((u32)XPLMI_PMC_GIC_IRQ_GICP0, (u32)XPLMI_GICP0_SRC16, XPM_NODEIDX_DEV_SPI_0);
	XPm_RegisterWakeUpHandler((u32)XPLMI_PMC_GIC_IRQ_GICP0, (u32)XPLMI_GICP0_SRC17, XPM_NODEIDX_DEV_SPI_1);
	XPm_RegisterWakeUpHandler((u32)XPLMI_PMC_GIC_IRQ_GICP0, (u32)XPLMI_GICP0_SRC18, XPM_NODEIDX_DEV_UART_0);
	XPm_RegisterWakeUpHandler((u32)XPLMI_PMC_GIC_IRQ_GICP0, (u32)XPLMI_GICP0_SRC19, XPM_NODEIDX_DEV_UART_1);
	XPm_RegisterWakeUpHandler((u32)XPLMI_PMC_GIC_IRQ_GICP0, (u32)XPLMI_GICP0_SRC20, XPM_NODEIDX_DEV_CAN_FD_0);
	XPm_RegisterWakeUpHandler((u32)XPLMI_PMC_GIC_IRQ_GICP0, (u32)XPLMI_GICP0_SRC21, XPM_NODEIDX_DEV_CAN_FD_1);
	XPm_RegisterWakeUpHandler((u32)XPLMI_PMC_GIC_IRQ_GICP0, (u32)XPLMI_GICP0_SRC22, XPM_NODEIDX_DEV_USB_0);
	XPm_RegisterWakeUpHandler((u32)XPLMI_PMC_GIC_IRQ_GICP0, (u32)XPLMI_GICP0_SRC23, XPM_NODEIDX_DEV_USB_0);
	XPm_RegisterWakeUpHandler((u32)XPLMI_PMC_GIC_IRQ_GICP0, (u32)XPLMI_GICP0_SRC24, XPM_NODEIDX_DEV_USB_0);
	XPm_RegisterWakeUpHandler((u32)XPLMI_PMC_GIC_IRQ_GICP0, (u32)XPLMI_GICP0_SRC25, XPM_NODEIDX_DEV_USB_0);
	XPm_RegisterWakeUpHandler((u32)XPLMI_PMC_GIC_IRQ_GICP0, (u32)XPLMI_GICP0_SRC26, XPM_NODEIDX_DEV_USB_0);
	XPm_RegisterWakeUpHandler((u32)XPLMI_PMC_GIC_IRQ_GICP1, (u32)XPLMI_GICP1_SRC5, XPM_NODEIDX_DEV_TTC_0);
	XPm_RegisterWakeUpHandler((u32)XPLMI_PMC_GIC_IRQ_GICP1, (u32)XPLMI_GICP1_SRC6, XPM_NODEIDX_DEV_TTC_0);
	XPm_RegisterWakeUpHandler((u32)XPLMI_PMC_GIC_IRQ_GICP1, (u32)XPLMI_GICP1_SRC7, XPM_NODEIDX_DEV_TTC_0);
	XPm_RegisterWakeUpHandler((u32)XPLMI_PMC_GIC_IRQ_GICP1, (u32)XPLMI_GICP1_SRC8, XPM_NODEIDX_DEV_TTC_1);
	XPm_RegisterWakeUpHandler((u32)XPLMI_PMC_GIC_IRQ_GICP1, (u32)XPLMI_GICP1_SRC9, XPM_NODEIDX_DEV_TTC_1);
	XPm_RegisterWakeUpHandler((u32)XPLMI_PMC_GIC_IRQ_GICP1, (u32)XPLMI_GICP1_SRC10, XPM_NODEIDX_DEV_TTC_1);
	XPm_RegisterWakeUpHandler((u32)XPLMI_PMC_GIC_IRQ_GICP1, (u32)XPLMI_GICP1_SRC11, XPM_NODEIDX_DEV_TTC_2);
	XPm_RegisterWakeUpHandler((u32)XPLMI_PMC_GIC_IRQ_GICP1, (u32)XPLMI_GICP1_SRC12, XPM_NODEIDX_DEV_TTC_2);
	XPm_RegisterWakeUpHandler((u32)XPLMI_PMC_GIC_IRQ_GICP1, (u32)XPLMI_GICP1_SRC13, XPM_NODEIDX_DEV_TTC_2);
	XPm_RegisterWakeUpHandler((u32)XPLMI_PMC_GIC_IRQ_GICP1, (u32)XPLMI_GICP1_SRC14, XPM_NODEIDX_DEV_TTC_3);
	XPm_RegisterWakeUpHandler((u32)XPLMI_PMC_GIC_IRQ_GICP1, (u32)XPLMI_GICP1_SRC15, XPM_NODEIDX_DEV_TTC_3);
	XPm_RegisterWakeUpHandler((u32)XPLMI_PMC_GIC_IRQ_GICP1, (u32)XPLMI_GICP1_SRC16, XPM_NODEIDX_DEV_TTC_3);
	XPm_RegisterWakeUpHandler((u32)XPLMI_PMC_GIC_IRQ_GICP1, (u32)XPLMI_GICP1_SRC24, XPM_NODEIDX_DEV_GEM_0);
	XPm_RegisterWakeUpHandler((u32)XPLMI_PMC_GIC_IRQ_GICP1, (u32)XPLMI_GICP1_SRC25, XPM_NODEIDX_DEV_GEM_0);
	XPm_RegisterWakeUpHandler((u32)XPLMI_PMC_GIC_IRQ_GICP1, (u32)XPLMI_GICP1_SRC26, XPM_NODEIDX_DEV_GEM_1);
	XPm_RegisterWakeUpHandler((u32)XPLMI_PMC_GIC_IRQ_GICP1, (u32)XPLMI_GICP1_SRC27, XPM_NODEIDX_DEV_GEM_1);
	XPm_RegisterWakeUpHandler((u32)XPLMI_PMC_GIC_IRQ_GICP1, (u32)XPLMI_GICP1_SRC28, XPM_NODEIDX_DEV_ADMA_0);
	XPm_RegisterWakeUpHandler((u32)XPLMI_PMC_GIC_IRQ_GICP1, (u32)XPLMI_GICP1_SRC29, XPM_NODEIDX_DEV_ADMA_1);
	XPm_RegisterWakeUpHandler((u32)XPLMI_PMC_GIC_IRQ_GICP1, (u32)XPLMI_GICP1_SRC30, XPM_NODEIDX_DEV_ADMA_2);
	XPm_RegisterWakeUpHandler((u32)XPLMI_PMC_GIC_IRQ_GICP1, (u32)XPLMI_GICP1_SRC31, XPM_NODEIDX_DEV_ADMA_3);
	XPm_RegisterWakeUpHandler((u32)XPLMI_PMC_GIC_IRQ_GICP2, (u32)XPLMI_GICP2_SRC0, XPM_NODEIDX_DEV_ADMA_4);
	XPm_RegisterWakeUpHandler((u32)XPLMI_PMC_GIC_IRQ_GICP2, (u32)XPLMI_GICP2_SRC1, XPM_NODEIDX_DEV_ADMA_5);
	XPm_RegisterWakeUpHandler((u32)XPLMI_PMC_GIC_IRQ_GICP2, (u32)XPLMI_GICP2_SRC2, XPM_NODEIDX_DEV_ADMA_6);
	XPm_RegisterWakeUpHandler((u32)XPLMI_PMC_GIC_IRQ_GICP2, (u32)XPLMI_GICP2_SRC3, XPM_NODEIDX_DEV_ADMA_7);
	XPm_RegisterWakeUpHandler((u32)XPLMI_PMC_GIC_IRQ_GICP2, (u32)XPLMI_GICP2_SRC10, XPM_NODEIDX_DEV_USB_0);
	XPm_RegisterWakeUpHandler((u32)XPLMI_PMC_GIC_IRQ_GICP3, (u32)XPLMI_GICP3_SRC30, XPM_NODEIDX_DEV_SDIO_0);
	XPm_RegisterWakeUpHandler((u32)XPLMI_PMC_GIC_IRQ_GICP3, (u32)XPLMI_GICP3_SRC31, XPM_NODEIDX_DEV_SDIO_0);
	XPm_RegisterWakeUpHandler((u32)XPLMI_PMC_GIC_IRQ_GICP4, (u32)XPLMI_GICP4_SRC0, XPM_NODEIDX_DEV_SDIO_1);
	XPm_RegisterWakeUpHandler((u32)XPLMI_PMC_GIC_IRQ_GICP4, (u32)XPLMI_GICP4_SRC1, XPM_NODEIDX_DEV_SDIO_1);
	XPm_RegisterWakeUpHandler((u32)XPLMI_PMC_GIC_IRQ_GICP4, (u32)XPLMI_GICP4_SRC14, XPM_NODEIDX_DEV_RTC);

END:
	return Status;
}

#ifdef ENABLE_GPIO_PROC_HANDLER
/*****************************************************************************/
/**
 * @brief  Shared GPIO interrupt handler for configured MIO pins.
 *
 * Called by the GIC dispatcher when a PS-GPIO or PMC-GPIO interrupt
 * fires (GICP0 SRC13 for LPD GPIO, or GICP3 SRC26 for PMC GPIO).
 *
 * The handler iterates every entry in GpioProcCfg[] and, for each pin
 * whose INT_STAT bit is set, checks the DATA_RO level.  A rising edge
 * (INT_STAT pending + DATA_RO high) triggers XPlmi_ExecuteProc() with
 * that pin's ProcId.  This allows multiple MIO pins on the same GPIO
 * controller to share one GIC source while each executing a different
 * CDO proc.
 *
 * After processing all pins the handler clears INT_STAT, re-enables
 * the GPIO INT_EN / INT_ANY bits, and re-enables the GIC source so
 * subsequent toggles are delivered.
 *
 * @param  Data  Unused (handler iterates the global GpioProcCfg array).
 *
 * @return XST_SUCCESS on success, error code on failure.
 *
 *****************************************************************************/
int XPm_GpioProcHandler(void *Data)
{
	u32 Idx;
	u32 IntStatOff, DataRoOff, IntEnOff, IntAnyOff, GpioBaseAddr;
	u32 IntStat, DataRo, DevId;
	u32 MioNum, BitPos, BitMask;
	XPm_GpioProcCfg *Cfg;
	u8 IsProcExecuted = FALSE;
	XStatus Status;

	(void)Data;

	for (Idx = 0U; Idx < GpioProcCfgCount; Idx++) {
		Cfg = &GpioProcCfg[Idx];

		/* Resolve GPIO base address from pin node type */
		if (NODETYPE(Cfg->PinId) == XPM_NODETYPE_LPD_MIO) {
			DevId = PM_DEV_GPIO;
			/*
			 * PinId was built as PM_STMIC_LMIO_BASE + MioIdx during
			 * registration, so NODEINDEX(PinId) equals MioIdx (1-based).
			 * Subtract NODEINDEX(PM_STMIC_LMIO_0) (which is 1) to get
			 * a 0-based MIO pin number for the GPIO register bit math.
			 * e.g. LMIO_0 → NODEINDEX 1 − 1 = MioNum 0
			 *      LMIO_5 → NODEINDEX 6 − 1 = MioNum 5
			 */
			 MioNum  = (NODEINDEX(Cfg->PinId) - NODEINDEX(PM_STMIC_LMIO_0));
		} else if (NODETYPE(Cfg->PinId) == XPM_NODETYPE_PMC_MIO) {
			DevId = PM_DEV_GPIO_PMC;
						/*
			 * PinId was built as PM_STMIC_PMIO_BASE + MioIdx during
			 * registration, so NODEINDEX(PinId) equals MioIdx (1-based).
			 * Same conversion for PMC MIO pins.
			 * Subtract NODEINDEX(PM_STMIC_PMIO_0) (which is 27) to
			 * get a 0-based MIO pin number within the PMC GPIO block.
			 * e.g. PMIO_0 → NODEINDEX 27 − 27 = MioNum 0
			 *      PMIO_5 → NODEINDEX 32 − 27 = MioNum 5
			 */
			 MioNum  = (NODEINDEX(Cfg->PinId) - NODEINDEX(PM_STMIC_PMIO_0));
		} else {
			continue;
		}

		Status = XPm_GetDeviceBaseAddr(DevId, &GpioBaseAddr);
		if (Status != XST_SUCCESS) {
			PmErr("Failed to get GPIO base address for dev 0x%x\r\n", DevId);
			goto done;
		}

		/* Bit position within the bank: each bank covers GPIO_PINS_PER_BANK pins */
		BitPos  = MioNum % GPIO_PINS_PER_BANK;
		/* Single-bit mask for this pin's position in the bank registers */
		BitMask = (1U << BitPos);

		/* Select bank register offsets */
		if (Cfg->Bank == GPIO_BANK_0) {
			IntStatOff = PMC_GPIO_INT_STAT_0_OFFSET;
			DataRoOff  = PMC_GPIO_DATA_0_RO_OFFSET;
			IntEnOff   = PMC_GPIO_INT_EN_0_OFFSET;
			IntAnyOff  = PMC_GPIO_INT_ANY_0_OFFSET;
		} else if (Cfg->Bank == GPIO_BANK_1) {
			IntStatOff = PMC_GPIO_INT_STAT_1_OFFSET;
			DataRoOff  = PMC_GPIO_DATA_1_RO_OFFSET;
			IntEnOff   = PMC_GPIO_INT_EN_1_OFFSET;
			IntAnyOff  = PMC_GPIO_INT_ANY_1_OFFSET;
		} else if (Cfg->Bank == GPIO_BANK_2) {
			IntStatOff = PMC_GPIO_INT_STAT_2_OFFSET;
			DataRoOff  = PMC_GPIO_DATA_2_RO_OFFSET;
			IntEnOff   = PMC_GPIO_INT_EN_2_OFFSET;
			IntAnyOff  = PMC_GPIO_INT_ANY_2_OFFSET;
		} else {
			PmErr("Invalid bank %u\r\n", Cfg->Bank);
			goto done;
		}

		/* Read interrupt status and current pin level from the resolved bank */
		IntStat = XPm_In32(GpioBaseAddr + IntStatOff);
		DataRo  = XPm_In32(GpioBaseAddr + DataRoOff);

		/*
		 * Execute this pin's proc on a rising edge: INT_STAT pending
		 * and DATA_RO high indicates reset has been released.
		 */
		if ((IntStat & BitMask) != 0U) {
			if ((DataRo & BitMask) != 0U) {
				Status = XPlmi_ExecuteProc(Cfg->ProcId);
				if (XST_SUCCESS != Status) {
					PmErr("Proc 0x%x "
					      "failed\r\n", Cfg->ProcId);
				}
				IsProcExecuted = TRUE;
			}
			/* Write-to-clear this pin's interrupt */
			XPm_Out32(GpioBaseAddr + IntStatOff, BitMask);
		}
		/* Clear GIC pending and re-enable for this pin's source */
		XPlmi_GicIntrClearStatus(Cfg->GicId, Cfg->GicSrc);
		XPlmi_GicIntrEnable(Cfg->GicId, Cfg->GicSrc);
		/* Re-enable GPIO interrupt and INT_ANY for this pin */
		XPm_Out32(GpioBaseAddr + IntEnOff, BitMask);
		XPm_RMW32(GpioBaseAddr + IntAnyOff, BitMask, BitMask);
	}

	/*
	 * If no CDO proc was executed for any registered pin, log an error.
	 * This can happen if the interrupt fired but DATA_RO showed no
	 * rising edge, or no registered pin matched the pending INT_STAT.
	 */
	if (!IsProcExecuted) {
		PmErr("GPIO Interrupt is triggered but no proc executed\r\n");
		Status = XST_FAILURE;
	}

done:
	return Status;
}

/*****************************************************************************/
/**
 * @brief  Register a single GPIO interrupt handler from a 16-bit config half.
 *
 * Stores the pin configuration in GpioProcCfg[], enables the GPIO
 * INT_EN and INT_ANY registers for the pin, and registers the shared
 * XPm_GpioProcHandler with the GIC if no handler has been registered
 * yet for this (GicId, GicSrc) pair.
 *
 * GIC routing:
 *   LPD GPIO (type = 0) -> GICP0 SRC13
 *   PMC GPIO (type = 1) -> GICP3 SRC26
 *
 * @param  CfgHalf  16-bit configuration value for one GPIO entry.
 *
 * @return XST_SUCCESS if the handler was registered successfully.
 *         Error code from XPlmi_GicRegisterHandler on registration failure.
 *
 *****************************************************************************/
static int XPm_GpioRegisterPin(u32 CfgHalf)
{
	int Status = XST_FAILURE;
	u32 PinId, GicId, GicSrc;
	u8 SlrId;
	u32 GpioController, Bank, MioIdx, ProcId;
	u32 GpioBaseAddr, IntEnOff, IntAnyOff, BitMask;
	u32 Idx, DevId, MioNum;
	u8 GicRegistered = (u8)FALSE;
	XPm_GpioProcCfg *Cfg = NULL;

	if (GpioProcCfgCount >= MAX_GPIO_PROC_PINS) {
		PmErr("Max registrations (%u) reached\r\n",
		      MAX_GPIO_PROC_PINS);
		Status = XST_FAILURE;
		goto done;
	}

	SlrId    = CfgHalf & GPIO_CFG_SLRID_MASK;
	GpioController = (CfgHalf & GPIO_CFG_TYPE_MASK) >> GPIO_CFG_TYPE_SHIFT;
	Bank     = (CfgHalf & GPIO_CFG_BANK_MASK) >> GPIO_CFG_BANK_SHIFT;
	MioIdx   = (CfgHalf & GPIO_CFG_MIO_MASK) >> GPIO_CFG_MIO_SHIFT;
	ProcId   = (CfgHalf & GPIO_CFG_PROCID_MASK) >> GPIO_CFG_PROCID_SHIFT;

	/* MioIdx is 1-based in the RTCA config (0 is invalid) */
	if ((MioIdx == 0U) || (MioIdx > XPM_NODEIDX_STMIC_MAX)) {
		PmErr("Invalid MIO index %u\r\n", MioIdx);
		Status = XST_INVALID_PARAM;
		goto done;
	}

	if (Bank > GPIO_BANK_2) {
		PmErr("Invalid bank %u\r\n", Bank);
		Status = XST_INVALID_PARAM;
		goto done;
	}
	/*
	 * Map the GPIO Controller to the corresponding pin node ID, GIC interrupt
	 * source, and base address.
	 *   GpioController 0 - (LPD Controller) -> LPD_GPIO via GICP0/SRC13
	 *   GpioController 1 - (PMC Controller) -> PMC_GPIO via GICP3/SRC26
	 */

	if (GpioController == (u32)LPD_GPIO_CONTROLLER) {
		PinId        = PM_STMIC_LMIO_BASE + MioIdx;
		GicId        = (u32)XPLMI_PMC_GIC_IRQ_GICP0;
		GicSrc       = (u32)XPLMI_GICP0_SRC13;
		DevId = PM_DEV_GPIO;
	} else if (GpioController == (u32)PMC_GPIO_CONTROLLER) {
		PinId        = PM_STMIC_PMIO_BASE + MioIdx;
		GicId        = (u32)XPLMI_PMC_GIC_IRQ_GICP3;
		GicSrc       = (u32)XPLMI_GICP3_SRC26;
		DevId = PM_DEV_GPIO_PMC;
	} else {
		PmErr("Invalid GPIO type 0x%x\r\n", GpioController);
		Status = XST_INVALID_PARAM;
		goto done;
	}
	Status = XPm_GetDeviceBaseAddr(DevId, &GpioBaseAddr);
	if (Status != XST_SUCCESS) {
		PmErr("Failed to get GPIO base address for dev 0x%x\r\n", DevId);
		goto done;
	}

	/* Reject duplicate MIO pin or Proc ID across all previously registered entries */
	for (Idx = 0U; Idx < GpioProcCfgCount; Idx++) {
		if (GpioProcCfg[Idx].PinId == PinId) {
			PmErr("MIO pin 0x%x already registered\r\n", PinId);
			Status = XST_INVALID_PARAM;
			goto done;
		}
	}

	/* Convert 1-based RTCA MioIdx to 0-based MIO pin number for bit math */
	MioNum  = (MioIdx - GPIO_CFG_MIO_IDX_BASE);
	BitMask = (1U << (MioNum % GPIO_PINS_PER_BANK));

	if (Bank == GPIO_BANK_0) {
		IntEnOff  = PMC_GPIO_INT_EN_0_OFFSET;
		IntAnyOff = PMC_GPIO_INT_ANY_0_OFFSET;
	} else if (Bank == GPIO_BANK_1) {
		IntEnOff  = PMC_GPIO_INT_EN_1_OFFSET;
		IntAnyOff = PMC_GPIO_INT_ANY_1_OFFSET;
	} else if (Bank == GPIO_BANK_2) {
		IntEnOff  = PMC_GPIO_INT_EN_2_OFFSET;
		IntAnyOff = PMC_GPIO_INT_ANY_2_OFFSET;
	} else {
		PmErr("Invalid bank %u\r\n", Bank);
		Status = XST_INVALID_PARAM;
		goto done;
	}

	/* Store pin configuration */
	Cfg = &GpioProcCfg[GpioProcCfgCount];
	Cfg->PinId  = PinId;
	Cfg->Bank   = Bank;
	Cfg->ProcId = ProcId;
	Cfg->SlrId  = SlrId;
	Cfg->GicId  = GicId;
	Cfg->GicSrc = GicSrc;

	/*
	 * Check if a GIC handler is already registered for this source.
	 * Only one handler per (GicId, GicSrc) is dispatched by the GIC
	 * framework, so skip registration if a previous pin already
	 * registered on the same source.
	 */
	for (Idx = 0U; Idx < GpioProcCfgCount; Idx++) {
		if ((GpioProcCfg[Idx].GicId == GicId) &&
		    (GpioProcCfg[Idx].GicSrc == GicSrc)) {
			GicRegistered = (u8)TRUE;
			break;
		}
	}

	if (GicRegistered == (u8)FALSE) {
		Status = XPlmi_GicRegisterHandler(GicId, GicSrc,
						  &XPm_GpioProcHandler, NULL);
		if (Status != XST_SUCCESS) {
			PmErr("GPIO handler registration failed 0x%x\r\n",
			      Status);
			goto done;
		}
	}

	GpioProcCfgCount++;

	/* Enable GIC interrupt for this source */
	XPlmi_GicIntrEnable(GicId, GicSrc);
	/* Enable GPIO interrupt for this pin */
	XPm_Out32(GpioBaseAddr + IntEnOff, BitMask);
	/* Enable interrupt on any edge for this pin */
	XPm_RMW32(GpioBaseAddr + IntAnyOff, BitMask, BitMask);

	Status = XST_SUCCESS;

done:
	return Status;
}

/*****************************************************************************/
/**
 * @brief  Initialize GPIO interrupt handlers.
 *
 * Reads the GPIO configuration register (RTCA_GPIO_PROC_CFG_REG at
 * 0xF2014370).  The 32-bit register carries two independent 16-bit
 * pin configurations (bits [15:0] and [31:16]).  Each non-zero half
 * is parsed and a GIC interrupt handler is registered for the
 * corresponding MIO pin.
 *
 * 16-bit configuration field layout:
 *   [2:0]   / [18:16] : SLR ID
 *   [4:3]   / [20:19] : GPIO Controller — 0 = LPD GPIO Controller, 1 = PMC GPIO Controller
 *   [6:5]   / [22:21] : GPIO bank number
 *   [12:7]  / [28:23] : MIO pin index
 *   [15:13] / [31:29] : CDO Proc ID to execute on reset event
 *
 * If both halves are populated, the MIO index and Proc ID must differ
 * between the two entries; otherwise XST_INVALID_PARAM is returned.
 * Each valid half is decoded and registered via XPm_GpioRegisterPin().
 *
 * @return XST_SUCCESS if all non-zero entries were registered successfully.
 *         Error code from XPm_GpioRegisterPin on first failure.
 *
 *****************************************************************************/
int XPm_GpioProcHandlerInit(void)
{
	int Status = XST_FAILURE;
	u32 CfgVal, HalfCfg, Idx;

	/* Read the RTCA configuration register to get the GPIO configurations */
	CfgVal = XPm_In32(RTCA_GPIO_PROC_CFG_REG);
	if (0U == CfgVal) {
		Status = XST_SUCCESS;
		goto done;
	}

	/* Process each half of the configuration register */
	for (Idx = 0U; Idx < 2U; Idx++) {
		/* Extract the half of the configuration register */
		HalfCfg = (CfgVal >> (Idx * GPIO_CFG_UPPER_SHIFT))
			  & GPIO_CFG_HALF_MASK;
		if (HalfCfg != 0U) {
			Status = XPm_GpioRegisterPin(HalfCfg);
			if (Status != XST_SUCCESS) {
				PmErr("Half %u failed 0x%x\r\n",
				      Idx, Status);
				goto done;
			}
		}
	}

done:
	XPm_Out32(RTCA_GPIO_PROC_CFG_REG, 0U);
	return Status;
}
#endif /* ENABLE_GPIO_PROC_HANDLER */

/****************************************************************************/
/**
 * @brief  Initialize XilPM library
 *
 * @return XST_SUCCESS if successful else XST_FAILURE or an error code
 * or a reason code
 *
 * @note   None
 *
 ****************************************************************************/
XStatus XPm_PlatInit(void)
{
	XStatus Status = XST_FAILURE;
	u32 i;
	u32 Platform = 0;
	u32 PmcVersion;
	u32 IdCode = XPm_GetIdCode();

	const u32 IsolationIdx[] = {
		(u32)XPM_NODEIDX_ISO_VCCAUX_VCCRAM,
		(u32)XPM_NODEIDX_ISO_VCCRAM_SOC,
		(u32)XPM_NODEIDX_ISO_VCCAUX_SOC,
		(u32)XPM_NODEIDX_ISO_PL_SOC,
		(u32)XPM_NODEIDX_ISO_PMC_SOC,
		(u32)XPM_NODEIDX_ISO_PMC_SOC_NPI,
		(u32)XPM_NODEIDX_ISO_PMC_PL,
		(u32)XPM_NODEIDX_ISO_PMC_LPD,
		(u32)XPM_NODEIDX_ISO_LPD_SOC,
		(u32)XPM_NODEIDX_ISO_LPD_PL,
		(u32)XPM_NODEIDX_ISO_LPD_CPM,
		(u32)XPM_NODEIDX_ISO_FPD_SOC,
		(u32)XPM_NODEIDX_ISO_FPD_PL,
	};

	PmcVersion = XPm_GetPmcVersion();
	Platform = XPm_GetPlatform();

	/* Enable domain isolations after system reset */
	for (i = 0; i < ARRAY_SIZE(IsolationIdx); i++) {
		Status = XPmDomainIso_Control(IsolationIdx[i], TRUE_VALUE);
		if (Status != XST_SUCCESS) {
			goto done;
		}
	}

	/* Add repair NoC which is reset by NoC POR above*/
	if ((PLATFORM_VERSION_SILICON == XPm_GetPlatform()) &&
	    (PMC_TAP_IDCODE_DEV_SBFMLY_VP1902 == (IdCode & PMC_TAP_IDCODE_DEV_SBFMLY_MASK))) {
#ifdef XCVP1902
		/* VP1902 device */
		Status = XPm_NocConfig_vp1902();
#endif
	} else {
		Status = XPm_NoCConfig();
	}

	/* For some boards, vccaux workaround is implemented using gpio to control vccram supply.
	During system reset, when gpio goes low, delay is required for system controller to process
	vccram rail off, before pdi load is started */
	if ((PLATFORM_VERSION_SILICON == Platform) && (PMC_VERSION_SILICON_ES1 == PmcVersion)) {
		usleep(300000);
	}

done:
	return Status;
}

static void PostTopologyHook(void)
{
	/* TODO: Remove this when PL topology handling is added */
	/* Set all PL clock as read only so that Linux won't disable those */
	XPmClock_SetPlClockAsReadOnly();

	/* TODO: Remove this when custom CPM POR reset is added from topology */
	/* Make CPM POR reset to custom reset */
	XPmReset_MakeCustom(PM_RST_CPM_POR);

	/* TODO: Remove this when custom ADMA reset is added from topology */
	/* Make ADMA reset to custom reset */
	XPmReset_MakeCustom(PM_RST_ADMA);
}

XStatus XPm_EnableDdrSr(const u32 SubsystemId)
{
	const XPm_Node *DDR_Node = (XPm_Node *)XPmDevice_GetById((u32)PM_DEV_DDR_0);
	const XPm_Requirement *Reqm;
	XStatus Status = XST_FAILURE;

	Reqm = XPmDevice_FindRequirement(PM_DEV_DDR_0, SubsystemId);
	if ((XST_SUCCESS == XPmRequirement_IsExclusive(Reqm)) &&
	    (XST_SUCCESS == XPmNpDomain_IsNpdIdle(DDR_Node))) {
		Status = XPmDevice_SetRequirement(SubsystemId, PM_DEV_DDR_0,
						  (u32)PM_CAP_CONTEXT, 0);
		if (XST_SUCCESS != Status) {
			goto done;
		}
	}

	Status = XST_SUCCESS;

done:
	return Status;
}

XStatus XPm_DisableDdrSr(const u32 SubsystemId)
{
	const XPm_Requirement *Reqm;
	XStatus Status = XST_FAILURE;

	Reqm = XPmDevice_FindRequirement(PM_DEV_DDR_0, SubsystemId);
	if (XST_SUCCESS == XPmRequirement_IsExclusive(Reqm)) {
		Status = XPmDevice_SetRequirement(SubsystemId, PM_DEV_DDR_0,
						  (u32)PM_CAP_ACCESS, 0);
		if (XST_SUCCESS != Status) {
			goto done;
		}
	}

	Status = XST_SUCCESS;

done:
	return Status;
}

void XPm_PlatChangeCoreState(XPm_Core *Core, const u32 State)
{
	if (PM_SUSPEND_STATE_CPU_OFF == State) {
		Core->Device.Node.State = (u8)XPM_DEVSTATE_PENDING_PWR_DWN;
	} else {
		Core->Device.Node.State = (u8)XPM_DEVSTATE_SUSPENDING;
	}
}

/*****************************************************************************/
/**
 * @brief	This function is to link devices to the default subsystem.
 *
 * @return	XST_SUCCESS on success and error code on failure
 *
 *****************************************************************************/
static XStatus XPm_AddReqsDefaultSubsystem(XPm_Subsystem *Subsystem)
{
	XStatus Status = XST_FAILURE;
	u32 i = 0U, Prealloc = 0U, Capability = 0U;
	const XPm_Requirement *Req = NULL;

	/**
	 * We do not support any customizations in Default Subsystem
	 */
	Req = Subsystem->Requirements;
	if (NULL != Req) {
		/* We return error if any requirements exists in Default Subsystem */
		goto done;
	}

	for (i = 0; i < (u32)XPM_NODEIDX_DEV_MAX; i++) {
		/* Always prealloc all requestable devices */
		Prealloc = 1U;
		/* Always use full access by default on all prealloc devices */
		Capability = (u32)PM_CAP_ACCESS;

		/*
		* Note: XPmDevice_GetByIndex() assumes that the caller
		* is responsible for validating the Node ID attributes
		* other than node index.
		*/
		XPm_Device *Device = XPmDevice_GetByIndex(i);
		if ((NULL == Device) || (1U != XPmDevice_IsRequestable(Device->Node.Id))) {
			continue;
		}

		/* Skip MEM_CTRLR prealloc: PMC subsystem requests DDRMCs when
		 * PL device is added from RNPI, moving those to RUNNING state. */
		if ((u32)XPM_NODESUBCL_DEV_MEM_CTRLR == NODESUBCLASS(Device->Node.Id)) {
			continue;
		}

		/* All MEM devices need CAP_CONTEXT */
		if ((u32)XPM_NODESUBCL_DEV_MEM == NODESUBCLASS(Device->Node.Id)) {
			Capability |= (u32)PM_CAP_CONTEXT;
		}

		/* Set of devices which need secure access for bus masters */
		switch (NODEINDEX(Device->Node.Id))
		{
			case (u32)XPM_NODEIDX_DEV_RPU0_0:
			case (u32)XPM_NODEIDX_DEV_GEM_0:
			case (u32)XPM_NODEIDX_DEV_GEM_1:
			case (u32)XPM_NODEIDX_DEV_OSPI:
			case (u32)XPM_NODEIDX_DEV_QSPI:
			case (u32)XPM_NODEIDX_DEV_SDIO_0:
			case (u32)XPM_NODEIDX_DEV_SDIO_1:
				/* add secure capability */
				Capability |= (u32)PM_CAP_SECURE;
				break;
			default:
				/* no change in capability */
				break;
		}

		/**
		 * Since default subsystem is hard-coded for now, add security policy
		 * for all peripherals as REQ_ACCESS_SECURE. This allows any device
		 * with a _master_ port to be requested in secure mode if the topology
		 * supports it.
		 */
		Status = XPmRequirement_Add(Subsystem, Device,
				REQUIREMENT_FLAGS(Prealloc,
					(u32)REQ_ACCESS_SECURE,
					(u32)REQ_NO_RESTRICTION),
				Capability, XPM_DEF_QOS);
		if (XST_SUCCESS != Status) {
			goto done;
		}
	}

	for (i = 0; i < (u32)XPM_NODEIDX_DEV_PLD_MAX; i++) {
		XPm_Device *Device = XPmDevice_GetPlDeviceByIndex(i);
		if (NULL != Device) {
			Status = XPmRequirement_Add(Subsystem, Device,
					(u32)REQUIREMENT_FLAGS(0U,
						(u32)REQ_ACCESS_SECURE_NONSECURE,
						(u32)REQ_NO_RESTRICTION),
					0U, XPM_DEF_QOS);
			if (XST_SUCCESS != Status) {
				goto done;
			}
		}
	}

	/* Add reset permissions */
	Status = XPmReset_AddPermForGlobalResets(Subsystem);
	if (XST_SUCCESS != Status) {
		goto done;
	}

	for (i = PM_DEV_GGS_0; i <= PM_DEV_GGS_3; i++) {
		Status = XPm_AddDevRequirement(Subsystem, i, IOCTL_PERM_READ_WRITE_MASK, NULL, 0);
		if (XST_SUCCESS != Status) {
			goto done;
		}
	}

	/* Add PGGS registers to Default Subsystem as requirements */
	for (i = PM_DEV_PGGS_0; i <= PM_DEV_PGGS_1; i++) {
		Status = XPm_AddDevRequirement(Subsystem, i, IOCTL_PERM_READ_WRITE_MASK, NULL, 0);
		if (XST_SUCCESS != Status) {
			goto done;
		}
	}

	Status = XST_SUCCESS;

done:
	return Status;
}


XStatus XPm_HookAfterPlmCdo(void)
{
	XStatus Status = XST_FAILURE;
	u32 PlatformVersion;
	u32 SysmonAddr;
	XPm_Subsystem *Subsystem;

	/*
	 * TODO: Re-introduce Early Housecleaning
	 * Rationale:
	 * There is a silicon problem where on 2-4% of Versal ES1 XCVC1902 devices
	 * you can get 12A of VCCINT_PL current before CFI housecleaning is run.
	 * The problem is eliminated when PL Vgg frame housecleaning is run
	 * so we need to do that ASAP after PLM is loaded.
	 * Otherwise also, PL housecleaning needs to be triggered asap to reduce
	 * boot time.
	 */

	/* On Boot, Update PMC SAT0 & SAT1 sysmon trim */
	SysmonAddr = XPm_GetSysmonByIndex((u32)XPM_NODEIDX_MONITOR_SYSMON_PMC_0);
	if (0U == SysmonAddr) {
		Status = XST_DEVICE_NOT_FOUND;
		goto done;
	}

	(void)XPm_ApplyAmsTrim(SysmonAddr, PM_POWER_PMC, 0);

	SysmonAddr = XPm_GetSysmonByIndex((u32)XPM_NODEIDX_MONITOR_SYSMON_PMC_1);
	if (0U == SysmonAddr) {
		Status = XST_DEVICE_NOT_FOUND;
		goto done;
	}

	(void)XPm_ApplyAmsTrim(SysmonAddr, PM_POWER_PMC, 1);

	PostTopologyHook();

	/**
	 * VCK190/VMK180 boards have VCC_AUX workaround where MIO-37 (PMC GPIO)
	 * is used to enable the VCC_RAM which used to power the PL.
	 * As a result, if PMC GPIO is disabled, VCC_RAM goes off.
	 * To prevent this from happening, request PMC GPIO device on behalf PMC subsystem.
	 * This will take care of the use cases where PMC is up.
	 * GPIO will get reset only when PMC goes down.
	 *
	 * The VCC_AUX workaround will be removed from MIO-37 in future.
	 */
	PlatformVersion = XPm_GetPlatformVersion();

	if (((PLATFORM_VERSION_SILICON == XPm_GetPlatform()) &&
	    (((u32)PLATFORM_VERSION_SILICON_ES1 == PlatformVersion) ||
	     ((u32)PLATFORM_VERSION_SILICON_ES2 == PlatformVersion)))) {
		Status = XPmDevice_Request(PM_SUBSYS_PMC, PM_DEV_GPIO_PMC,
					   XPM_MAX_CAPABILITY, XPM_MAX_QOS,
					   XPLMI_CMD_SECURE);
		if (XST_SUCCESS != Status) {
			goto done;
		}
	}

	/* If default subsystem is present, attempt to add requirements if needed. */
	Subsystem = XPmSubsystem_GetById(PM_SUBSYS_DEFAULT);
	if (((u32)1U == XPmSubsystem_GetMaxSubsysIdx()) &&
	    (NULL != Subsystem) &&
	    ((u8)ONLINE == Subsystem->State)) {
		Status = XPm_AddReqsDefaultSubsystem(Subsystem);
		if (XST_SUCCESS != Status) {
			goto done;
		}
	}

	Status = XST_SUCCESS;

done:
	return Status;
}


/*****************************************************************************/
/**
 * @brief	This function is a workaround for COSIM emulation platform.
 *
 * @return	XST_SUCCESS on success and error code on failure
 *
 *****************************************************************************/
static XStatus XPm_CosimInit(void)
{
	XStatus Status = XST_FAILURE;
	u16 DbgErr = XPM_INT_ERR_UNDEFINED;
	const XPm_PlDevice *PlDevice;
	const u32 PldPwrNodeDependency[1U] = {PM_POWER_PLD};

	Status = AddAieDeviceNode();
	if (XST_SUCCESS != Status) {
		goto done;
	}

	PlDevice = (XPm_PlDevice *)XPmDevice_GetById(PM_DEV_PLD_0);
	if (NULL == PlDevice) {
		DbgErr = XST_DEVICE_NOT_FOUND;
		Status = XST_FAILURE;
		goto done;
	}

	if ((u8)XPM_DEVSTATE_UNUSED == PlDevice->Device.Node.State) {
		Status = XPm_InitNode(PM_DEV_PLD_0, (u32)FUNC_INIT_START,
				PldPwrNodeDependency, 1U);
		if (XST_SUCCESS != Status) {
			DbgErr = XPM_INT_ERR_PLDEVICE_INITNODE;
			goto done;
		}
		Status = XPm_InitNode(PM_DEV_PLD_0, (u32)FUNC_INIT_FINISH,
				PldPwrNodeDependency, 1U);
		if (XST_SUCCESS != Status) {
			DbgErr = XPM_INT_ERR_PLDEVICE_INITNODE;
			goto done;
		}
	}

done:
	XPm_PrintDbgErr(Status, DbgErr);
	return Status;
}

XStatus XPm_HookAfterBootPdi(void)
{
	volatile XStatus Status = XST_FAILURE;
	u32 Platform = XPm_GetPlatform();
	u32 DeviceType = PMC_TAP_SLR_TYPE_MASK & XPm_In32(PMC_TAP_BASEADDR + PMC_TAP_SLR_TYPE_OFFSET);

	/* If platform is COSIM, run setup for PL and AIE devices */
	if (PLATFORM_VERSION_COSIM == Platform) {
		XSECURE_TEMPORAL_CHECK(done, Status, XPm_CosimInit);
	}

	/* Start HBM temperature monitoring task, if applicable */
	XSECURE_TEMPORAL_CHECK(done, Status, XPmMem_HBMTempMonInitTask);

	/* If SSIT device on primary SLR, start SSIT temperature propagation task */
	if (SLR_TYPE_SSIT_DEV_MASTER_SLR == DeviceType) {
		XSECURE_TEMPORAL_CHECK(done, Status, XPmPeriph_SsitTempPropInitTask);
	}

#ifdef ENABLE_GPIO_PROC_HANDLER
	/* register gpio interrupt handler when MIO pin is selected */
	Status = XPm_GpioProcHandlerInit();
	if (XST_SUCCESS != Status) {
		PmErr("Failed to initialize GPIO handler, Status: 0x%x\r\n", Status);
	}
#endif /* ENABLE_GPIO_PROC_HANDLER */

done:
	return Status;
}

static XStatus PwrDomainInitNode(u32 NodeId, u32 Function, const u32 *Args, u32 NumArgs)
{
	XStatus Status = XST_FAILURE;
	XPm_PowerDomain *PwrDomainNode;

	PwrDomainNode = (XPm_PowerDomain *)XPmPower_GetById(NodeId);
	if (NULL == PwrDomainNode) {
		PmErr("Unable to find Power Domain for given Node Id\n\r");
		Status = XPM_PM_INVALID_NODE;
		goto done;
	}

	switch (NODEINDEX(NodeId)) {
	case (u32)XPM_NODEIDX_POWER_PMC:
	case (u32)XPM_NODEIDX_POWER_LPD:
	case (u32)XPM_NODEIDX_POWER_FPD:
	case (u32)XPM_NODEIDX_POWER_NOC:
	case (u32)XPM_NODEIDX_POWER_PLD:
	case (u32)XPM_NODEIDX_POWER_ME:
	case (u32)XPM_NODEIDX_POWER_ME2:
	case (u32)XPM_NODEIDX_POWER_CPM:
	case (u32)XPM_NODEIDX_POWER_CPM5:
#if defined(XC2VP3202) || defined(XC2VP3602)
	case (u32)XPM_NODEIDX_POWER_CPM6:
#endif
		Status = XPmPowerDomain_InitDomain(PwrDomainNode, Function,
						   Args, NumArgs);
		break;
	default:
		Status = XPM_INVALID_PWRDOMAIN;
		PmErr("Unrecognized Power Domain: 0x%x\n\r", NODEINDEX(NodeId));
		break;
	}

	/*
	 * Call LPD init to initialize required components
	 */
	if ((NODEINDEX(NodeId) == (u32)XPM_NODEIDX_POWER_LPD) &&
		(Function == (u32)FUNC_INIT_FINISH) &&
		(XST_SUCCESS == Status)) {
#ifdef DEBUG_UART_PS
		/**
		 * PLM needs to request UART if debug is enabled, else XilPM
		 * will turn it off when it is not used by other processor.
		 * During such scenario when PLM tries to print debug message,
		 * system may not work properly.
		 */
		Status = XPm_RequestDevice(PM_SUBSYS_PMC, NODE_UART,
					   (u32)PM_CAP_ACCESS, XPM_MAX_QOS, 0,
					   XPLMI_CMD_SECURE);
		if (XST_SUCCESS != Status) {
			goto done;
		}
#endif
		/**
		 * PLM needs to request PMC IPI, else XilPM will reset IPI
		 * when it is not used by other processor. Because of that PLM
		 * hangs when it tires to communicate through IPI.
		 */
		Status = XPm_RequestDevice(PM_SUBSYS_PMC, PM_DEV_IPI_PMC,
					   (u32)PM_CAP_ACCESS, XPM_MAX_QOS, 0,
					   XPLMI_CMD_SECURE);
		if (XST_SUCCESS != Status) {
			PmErr("Error %d in request IPI PMC\r\n", Status);
		}
		XPlmi_LpdInit();
#ifdef XPLMI_IPI_DEVICE_ID
		Status = XPlmi_IpiInit(&XPmSubsystem_GetSubSysIdByIpiMask, &XPm_ProcessPsmCmd);
		if (XST_SUCCESS != Status) {
			PmErr("Error %u in IPI initialization\r\n", Status);
		}
#else
		PmWarn("IPI is not enabled in design\r\n");
#endif /* XPLMI_IPI_DEVICE_ID */
	}

done:
	if (XST_SUCCESS != Status) {
		PmErr("0x%x in InitNode for NodeId: 0x%x Function: 0x%x\r\n",
		       Status, NodeId, Function);
	}
	return Status;
}

static XStatus PldInitNode(u32 NodeId, u32 Function, const u32 *Args, u32 NumArgs)
{
	XStatus Status = XST_FAILURE;
	u16 DbgErr = XPM_INT_ERR_UNDEFINED;
	XPm_PlDevice *PlDevice = NULL;

	PlDevice = (XPm_PlDevice *)XPmDevice_GetById(NodeId);
	if (NULL == PlDevice) {
		DbgErr = XPM_INT_ERR_INVALID_NODE;
		goto done;
	}

	if (NULL == PlDevice->Ops) {
		DbgErr = XPM_INT_ERR_NO_FEATURE;
		goto done;
	}

	switch (Function) {
	case (u32)FUNC_INIT_START:
		if (NULL == PlDevice->Ops->InitStart) {
			DbgErr = XPM_INT_ERR_NO_FEATURE;
			goto done;
		}
		Status = PlDevice->Ops->InitStart(PlDevice, Args, NumArgs);
		break;
	case (u32)FUNC_INIT_FINISH:
		if (NULL == PlDevice->Ops->InitFinish) {
			DbgErr = XPM_INT_ERR_NO_FEATURE;
			goto done;
		}
		Status = PlDevice->Ops->InitFinish(PlDevice, Args, NumArgs);
		if (XST_SUCCESS != Status) {
			goto done;
		}
		Status = XPmDomainIso_ProcessPending();
		if (XST_SUCCESS != Status) {
			DbgErr = XPM_INT_ERR_DOMAIN_ISO;
			goto done;
		}
		if ((u8)XPM_DEVSTATE_RUNNING == PlDevice->Device.Node.State) {
			XPmPlDevice_TriggerStlHandler();
		}
		break;
	case (u32)FUNC_MEM_CTRLR_MAP:
		if (NULL == PlDevice->Ops->MemCtrlrMap) {
			DbgErr = XPM_INT_ERR_NO_FEATURE;
			goto done;
		}
		Status = PlDevice->Ops->MemCtrlrMap(PlDevice, Args, NumArgs);
		break;
	default:
		DbgErr = XPM_INT_ERR_INVALID_FUNC;
		break;
	}

done:
	XPm_PrintDbgErr(Status, DbgErr);
	return Status;
}

XStatus XPm_PlatAddNodePeriph(const u32 *Args, u32 PowerId)
{
	XStatus Status = XST_FAILURE;
	u32 BaseAddr;
	u32 DeviceId;
	u32 Index;
	XPm_Power *Power;
	XPm_AieDomain *AieDomain;

	DeviceId = Args[0];
	BaseAddr = Args[2];
	Index = NODEINDEX(DeviceId);

	Power = XPmPower_GetById(PowerId);
	if (NULL == Power) {
		Status = XST_DEVICE_NOT_FOUND;
		goto done;
	}

	switch (Index) {
	case (u32)XPM_NODEIDX_DEV_AIE:
		/*
		 * The node PM_DEV_AIE has been deprecated but it is still used
		 * in topology to get the AIE baseaddress. Save the base address
		 * to AIE power domain so it can be used for housecleaning & AIE OPS
		 */
		AieDomain = (XPm_AieDomain *)Power;
		AieDomain->AieNpiAddress = BaseAddr;
		Status = XST_SUCCESS;
		break;
	default:
		Status = XST_INVALID_PARAM;
		break;
	}

done:
	return Status;
}

static XStatus AieInitNode(u32 NodeId, u32 Function, const u32 *Args, u32 NumArgs)
{
	XStatus Status = XST_FAILURE;
	u16 DbgErr = XPM_INT_ERR_UNDEFINED;
	XPm_AieDevice *AieDevice;

	AieDevice = (XPm_AieDevice *)XPmDevice_GetById(NodeId);
	if (NULL == AieDevice) {
		DbgErr = XPM_INT_ERR_INVALID_NODE;
		goto done;
	}

	if (NULL == AieDevice->Ops) {
		DbgErr = XPM_INT_ERR_AIE_UNDEF_INIT_NODE;
		goto done;
	}

	switch (Function) {
	case (u32)FUNC_INIT_START:
		if (NULL == AieDevice->Ops->InitStart) {
			DbgErr = XPM_INT_ERR_AIE_UNDEF_INIT_NODE_START;
			goto done;
		}
		Status = AieDevice->Ops->InitStart(AieDevice, Args, NumArgs);
		break;
	case (u32)FUNC_INIT_FINISH:
		if (NULL == AieDevice->Ops->InitFinish) {
			DbgErr = XPM_INT_ERR_AIE_UNDEF_INIT_NODE_FINISH;
			goto done;
		}
		Status = AieDevice->Ops->InitFinish(AieDevice, Args, NumArgs);
		break;
	default:
		DbgErr = XPM_INT_ERR_INVALID_FUNC;
		break;
	}

done:
	XPm_PrintDbgErr(Status, DbgErr);
	return Status;
}

/****************************************************************************/
/**
 * @brief  This function allows to initialize the node.
 *
 * @param  NodeId	Supported power domain nodes, PLD, AIE & Protection
 * nodes
 * @param  Function	Function id
 * @param  Args		Arguments speicifc to function
 * @param  NumArgs  Number of arguments
 *
 * @return XST_SUCCESS if successful else XST_FAILURE or an error code
 * or a reason code
 *
 * @note   none
 *
 ****************************************************************************/
XStatus XPm_InitNode(u32 NodeId, u32 Function, const u32 *Args, u32 NumArgs)
{
	XPM_EXPORT_CMD(PM_INIT_NODE, XPLMI_CMD_ARG_CNT_TWO, XPLMI_CMD_ARG_CNT_TWELVE);
	XStatus Status = XST_FAILURE;
	u16 DbgErr = XPM_INT_ERR_UNDEFINED;

	if (((u32)XPM_NODECLASS_POWER == NODECLASS(NodeId)) &&
	    ((u32)XPM_NODESUBCL_POWER_DOMAIN == NODESUBCLASS(NodeId)) &&
	    ((u32)XPM_NODEIDX_POWER_MAX > NODEINDEX(NodeId))) {
		Status = PwrDomainInitNode(NodeId, Function, Args, NumArgs);
	} else if (((u32)XPM_NODECLASS_DEVICE == NODECLASS(NodeId)) &&
		  ((u32)XPM_NODESUBCL_DEV_PL == NODESUBCLASS(NodeId)) &&
		  ((u32)XPM_NODEIDX_DEV_PLD_MAX > NODEINDEX(NodeId))) {
		Status = PldInitNode(NodeId, Function, Args, NumArgs);
	} else if (((u32)XPM_NODECLASS_DEVICE == NODECLASS(NodeId)) &&
		  ((u32)XPM_NODESUBCL_DEV_AIE == NODESUBCLASS(NodeId)) &&
		  ((u32)XPM_NODEIDX_DEV_AIE_MAX > NODEINDEX(NodeId))) {
		Status = AieInitNode(NodeId, Function, Args, NumArgs);
	} else {
		Status = XPM_PM_INVALID_NODE;
		DbgErr = XPM_INT_ERR_INITNODE;
	}

	XPm_PrintDbgErr(Status, DbgErr);
	return Status;
}

static XStatus AddPhyDevice(const u32 *Args, u32 PowerId)
{
	XStatus Status = XST_FAILURE;
	u32 DeviceId;
	u32 Type;
	XPm_Device *Device;
	XPm_Power *Power;
	u32 BaseAddr;

	DeviceId = Args[0];
	BaseAddr = Args[2];

	Power = XPmPower_GetById(PowerId);
	if (NULL == Power) {
		Status = XST_DEVICE_NOT_FOUND;
		goto done;
	}

	Type = NODETYPE(DeviceId);

	if (NULL != XPmDevice_GetById(DeviceId)) {
		Status = XST_DEVICE_BUSY;
		goto done;
	}

	switch (Type) {
	case (u32)XPM_NODETYPE_DEV_GT:
	case (u32)XPM_NODETYPE_DEV_VDU:
	case (u32)XPM_NODETYPE_DEV_BFRB:
	case (u32)XPM_NODETYPE_DEV_ADC:
	case (u32)XPM_NODETYPE_DEV_DAC:
		Device = (XPm_Device *)XPm_AllocBytes(sizeof(XPm_Device));
		if (NULL == Device) {
			Status = XST_BUFFER_TOO_SMALL;
			goto done;
		}
		Status = XPmDevice_Init(Device, DeviceId, BaseAddr,
					Power, NULL, NULL);
		break;
	default:
		Status = XST_INVALID_PARAM;
		break;
	}

done:
	return Status;
}

static XStatus AddAieDevice(const u32 *Args)
{
	XStatus Status = XST_FAILURE;
	u32 DeviceId = Args[0];
	u32 Index = NODEINDEX(DeviceId);
	u32 BaseAddr = Args[2];
	XPm_AieDevice *AieDevice;

	if ((u32)XPM_NODEIDX_DEV_AIE_MAX <= Index) {
		Status = XST_INVALID_PARAM;
		goto done;
	}

	/*
	 * Note: This function is executed as part of pm_add_node cmd triggered
	 * through CDO. Since there's a possibility of the same RM (hence CDO)
	 * being executed multiple times, we should not error out on addition
	 * of same node multiple times. Memory is allocated only if node is not
	 * present in database.
	 */
	AieDevice = (XPm_AieDevice *)XPmDevice_GetById(DeviceId);
	if (NULL == AieDevice) {
		AieDevice = (XPm_AieDevice *)XPm_AllocBytes(sizeof(XPm_AieDevice));
		if (NULL == AieDevice) {
			Status = XST_BUFFER_TOO_SMALL;
			goto done;
		}
	} else {
		PmInfo("0x%x Device is already added\r\n", DeviceId);
	}

	Status = XPmAieDevice_Init(AieDevice, DeviceId, BaseAddr, NULL, NULL, NULL);

done:
	return Status;
}

/****************************************************************************/
/**
 * @brief  This function adds device node to device topology database
 *
 * @param  Args		device specific arguments
 * @param NumArgs	number of arguments
 *
 * @return XST_SUCCESS if successful else XST_FAILURE or an error code
 * or a reason code
 *
 * @note   None
 *
 ****************************************************************************/
XStatus XPm_PlatAddDevice(const u32 *Args, u32 NumArgs)
{
	XStatus Status = XST_FAILURE;
	u32 DeviceId;
	u32 SubClass;
	u32 PowerId = 0;

	if (NumArgs < 1U) {
		Status = XST_INVALID_PARAM;
		goto done;
	}

	DeviceId = Args[0];
	SubClass = NODESUBCLASS(DeviceId);

	if (NumArgs > 1U) {
		/*
		 * Check for Num Args < 3U as device specific (except PLDevice)
		 * AddNode functions currently don't implement any NumArgs checks
		 */
		if (NumArgs < 3U) {
			Status = XST_INVALID_PARAM;
			goto done;
		}
		PowerId = Args[1];
		if (NULL == XPmPower_GetById(PowerId)) {
			Status = XST_DEVICE_NOT_FOUND;
			goto done;
		}
	}

	switch (SubClass) {
	case (u32)XPM_NODESUBCL_DEV_PHY:
		Status = AddPhyDevice(Args, PowerId);
		break;
	case (u32)XPM_NODESUBCL_DEV_AIE:
	/* PowerId is not passed by topology */
		Status = AddAieDevice(Args);
		break;
	default:
		Status = XST_INVALID_PARAM;
		break;
	}

done:
	return Status;
}

/****************************************************************************/
/**
 * @brief  This function add memic node to the topology database
 *
 * @param Args		MEMIC arguments
 * @param NumArgs	number of arguments
 *
 * @return XST_SUCCESS if successful else XST_FAILURE or an error code
 * or a reason code
 *
 * @note   None
 *
 ****************************************************************************/
static XStatus XPm_AddNodeMemIc(const u32 *Args, u32 NumArgs)
{
	XStatus Status = XST_FAILURE;
	u32 MemIcId;
	u32 BaseAddress;

	if (NumArgs < 3U) {
		Status = XST_INVALID_PARAM;
		goto done;
	}

	MemIcId = Args[0];
	BaseAddress = Args[2];


	if ((u32)XPM_NODESUBCL_MEMIC_NOC != NODESUBCLASS(MemIcId)) {
		Status = XST_INVALID_PARAM;
		goto done;
	}

	Status = XPmNpDomain_MemIcInit(MemIcId, BaseAddress);

done:
	return Status;
}


/****************************************************************************/
/**
 * @brief  This function add xmpu/xppu node to the topology database
 *
 * @param Args		Node arguments
 * @param NumArgs	number of arguments
 *
 * @return XST_SUCCESS if successful else XST_FAILURE or an error code
 * or a reason code
 *
 * @note   None
 *
 ****************************************************************************/
static XStatus XPm_AddNodeProt(const u32 *Args, u32 NumArgs)
{
	(void)Args;
	(void)NumArgs;

	/**
	 * NOTE:
	 * Protection nodes are now deprecated, but still passed from topology.
	 * Therefore, they must be silently ignored since firmware does not
	 * manage runtime protections for subsystems.
	 */
	return XST_SUCCESS;
}

XStatus XPm_PlatAddNodePower(const u32 *Args, u32 NumArgs)
{
	XPM_EXPORT_CMD(PM_ADD_NODE, XPLMI_CMD_ARG_CNT_ONE, XPLMI_UNLIMITED_ARG_CNT);
	XStatus Status = XST_FAILURE;
	u32 PowerId;
	u32 PowerType;
	u8 Width;
	u8 Shift;
	u32 BitMask;
	u32 ParentId;
	XPm_Power *PowerParent = NULL;
	XPm_AieDomain *AieDomain;
#ifdef VERSAL_ENABLE_DOMAIN_CONTROL_GPIO
	XPm_DomainCtrl *DomainCtrl;
#endif

	PowerId = Args[0];
	PowerType = NODETYPE(PowerId);
	Width = (u8)(Args[1] >> 8) & 0xFFU;
	Shift = (u8)(Args[1] & 0xFFU);
	ParentId = Args[2];

	BitMask = BITNMASK(Shift, Width);

	if ((ParentId != (u32)XPM_NODEIDX_POWER_MIN) &&
	    ((u32)XPM_NODETYPE_POWER_RAIL != PowerType) &&
#ifdef VERSAL_ENABLE_DOMAIN_CONTROL_GPIO
		((u32)XPM_NODETYPE_POWER_DOMAIN_CTRL != PowerType) &&
#endif
	    ((u32)XPM_NODETYPE_POWER_REGULATOR != PowerType)) {
		PowerParent = XPmPower_GetById(ParentId);
		if (NULL == PowerParent) {
			Status = XST_DEVICE_NOT_FOUND;
			goto done;
		}
	}

	switch (PowerType) {
	case (u32)XPM_NODETYPE_POWER_DOMAIN_ME:
		AieDomain = (XPm_AieDomain *)XPm_AllocBytes(sizeof(XPm_AieDomain));
		if (NULL == AieDomain) {
			Status = XST_BUFFER_TOO_SMALL;
			goto done;
		}
		Status = XPmAieDomain_Init(AieDomain, PowerId, BitMask, PowerParent,
				&Args[3], (NumArgs - 3U));
		break;
#ifdef VERSAL_ENABLE_DOMAIN_CONTROL_GPIO
	case (u32)XPM_NODETYPE_POWER_DOMAIN_CTRL:
		DomainCtrl = (XPm_DomainCtrl *)XPmPower_GetById(PowerId);
		if (NULL == DomainCtrl) {
			DomainCtrl = (XPm_DomainCtrl *)XPm_AllocBytes(sizeof(XPm_DomainCtrl));
			if (NULL == DomainCtrl) {
				Status = XST_BUFFER_TOO_SMALL;
				goto done;
			}
		}
		Status = XPmDomainCtrl_Init(DomainCtrl, PowerId, Args, NumArgs);
		break;
#endif
	default:
		Status = XST_INVALID_PARAM;
		break;
	}

done:
	return Status;
}

/****************************************************************************/
/**
 * @brief  This function allows adding node to clock, power, reset, mio
 *			or device topology
 *
 * @param  Args		Node specific arguments
 * @param NumArgs	number of arguments
 *
 * @return XST_SUCCESS if successful else XST_FAILURE or an error code
 * or a reason code
 *
 * @note   None
 *
 ****************************************************************************/
XStatus XPm_PlatAddNode(const u32 *Args, u32 NumArgs)
{
	XStatus Status = XST_FAILURE;
	u32 Id = Args[0];

	switch (NODECLASS(Id)) {
	case (u32)XPM_NODECLASS_MEMIC:
		Status = XPm_AddNodeMemIc(Args, NumArgs);
		break;
	case (u32)XPM_NODECLASS_PROTECTION:
		Status = XPm_AddNodeProt(Args, NumArgs);
		break;
	default:
		Status = XST_INVALID_PARAM;
		break;
	}

	return Status;
}

/****************************************************************************/
/**
 * @brief  This function returns supported version of the given API.
 *
 * @param  ApiId	API ID to check
 * @param  Version	pointer to array of 4 words
 *  - version[0] - EEMI API version number
 *  - version[1] - lower 32-bit bitmask of IOCTL or QUERY ID
 *  - version[2] - upper 32-bit bitmask of IOCTL or Query ID
 *  - Only PM_FEATURE_CHECK version 2 supports 64-bit bitmask
 *  - i.e. version[1] and version[2]
 * @return XST_SUCCESS if successful else XST_NO_FEATURE.
 *
 * @note   Remove CDO-only commands from versioning as it is for internal
 * use only, so no need to consider for versioning.
 *
 ****************************************************************************/
XStatus XPm_PlatFeatureCheck(const u32 ApiId, u32 *const Version)
{
	XStatus Status = XST_FAILURE;

	switch (ApiId) {
	case PM_API(PM_IOCTL):
		Version[0] = XST_API_PM_IOCTL_VERSION;
		Version[1] = (u32)(PM_IOCTL_FEATURE_BITMASK);
		Version[2] = (u32)((PM_IOCTL_FEATURE_BITMASK) >> 32);
		Status = XST_SUCCESS;
		break;
	default:
		*Version = 0U;
		Status = XPM_NO_FEATURE;
		break;
	}

	return Status;
}

/****************************************************************************/
/**
 * @brief  This function updates information for NoC clock enablement
 *
 * @param  NodeId	PLD Node Id
 * @param  Args	Arguments buffer specific to function
 * @param  NumArgs	Number of arguments passed
 *
 * @return XST_SUCCESS if successful else XST_FAILURE or an error code
 * or a reason code
 *
 * @note   None
 *
 ****************************************************************************/
XStatus XPm_NocClockEnable(u32 NodeId, const u32 *Args, u32 NumArgs)
{
	XPM_EXPORT_CMD(PM_NOC_CLOCK_ENABLE, XPLMI_CMD_ARG_CNT_TWO,
		XPLMI_CMD_ARG_CNT_NINE);
	XStatus Status = XST_FAILURE;
	XPm_PlDevice *PlDevice;

	PlDevice = (XPm_PlDevice *)XPmDevice_GetById(NodeId);
	if (NULL == PlDevice) {
		Status = XST_DEVICE_NOT_FOUND;
		goto done;
	}

	Status = XPmPlDevice_NocClkEnable(PlDevice, Args, NumArgs);

done:
	return Status;
}

/****************************************************************************/
/*
 * @brief  This function checks current and previous NoC clock enablement
 *
 * @param  Cmd		Pointer to CDO command
 * @param  Args	Arguments specific to function
 * @param  NumArgs	Number of arguments passed
 *
 * @return XST_SUCCESS if successful else XST_FAILURE or an error code
 * or a reason code
 *
 * @note   None
 *
 ****************************************************************************/
XStatus XPm_IfNocClockEnable(XPlmi_Cmd *Cmd, const u32 *Args, u32 NumArgs)
{
	XPM_EXPORT_CMD(PM_IF_NOC_CLOCK_ENABLE, XPLMI_CMD_ARG_CNT_TWO,
		XPLMI_CMD_ARG_CNT_THREE);
	XStatus Status = XST_FAILURE;
	u32 BitArrayIdx;
	u16 State, Mask;
	u32 Level = 1U;

	if (NumArgs < 2U) {
		Status = XST_INVALID_PARAM;
		goto done;
	}

	BitArrayIdx = Args[0];
	State = (u16)(Args[1] & 0xFFFFU);
	Mask = (u16)((Args[1] >> 16U) & 0xFFFFU);

	/*
	 * A Mask value of 0 indicates there is no requirement to ignore any bits
	 * in the State argument, so the Mask is set to 0xFFFF
	 */
	if (0U == Mask) {
		Mask = 0xFFFFU;
	}

	/* Level value defaults to 1 unless optional value is passed as argument */
	if (3U == NumArgs) {
		Level = Args[2];
	}

	Status = XPmPlDevice_IfNocClkEnable(Cmd, BitArrayIdx, State, Mask, Level);

done:
	return Status;
}

/****************************************************************************/
/*
 * @brief  This function performs house cleaning for a power domain
 *
 * @param NodeId	Node ID of a power domain to be forced house clean
 * @return		XST_SUCCESS if successful else error code.
 *
 ****************************************************************************/
XStatus XPm_ForceHouseClean(u32 NodeId)
{
	XStatus Status = XST_FAILURE;
	XStatus Status_st= XST_FAILURE;
	XStatus Status_sc = XST_FAILURE;
	XStatus Status_mc = XST_FAILURE;
	XStatus Status_sr = XST_FAILURE;
	const u32 Args[1]={1};
	XPm_PowerDomain* PwrDomainNode = (XPm_PowerDomain *)XPmPower_GetById(NodeId);
	if (NULL == PwrDomainNode) {
		PmErr("Unable to find Power Domain for given Node Id\n\r");
		Status = XPM_PM_INVALID_NODE;
		goto done;
	}

	/* If domain initialization not done, skip secure lock down */
	if ((NULL == PwrDomainNode->DomainOps) ||
		(PwrDomainNode->InitFlag != PwrDomainNode->DomainOps->InitMask)) {
		PmErr("Power domain is not initialized for the given Node Id\n\r");
		Status = XST_SUCCESS;
		goto done;
	}

	XPM_HOUSECLEAN(PwrDomainNode, InitStart, Args, 1, Status_st);
	XPM_HOUSECLEAN(PwrDomainNode, ScanClear, Args, 1, Status_sc);
	XPM_HOUSECLEAN(PwrDomainNode, Bisr, Args, 1, Status_sr);
	XPM_HOUSECLEAN(PwrDomainNode, Mbist, Args, 1, Status_mc);
	Status = Status_st + (Status_sc << 1U) + (Status_sr << 2U) \
		+ (Status_mc << 3U);
done:
	return Status;
}
