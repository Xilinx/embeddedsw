/******************************************************************************
* Copyright (c) 2018 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2022 - 2025 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

#include "xplmi.h"
#include "xplmi_error_node.h"
#include "xplmi_hw.h"
#include "xplmi_ipi.h"
#include "xplmi_scheduler.h"
#include "xplmi_sysmon.h"
#include "xplmi_util.h"
#include "xpm_api_plat.h"
#include "xpm_bisr.h"
#include "xpm_device.h"
#include "xpm_common.h"
#include "xpm_defs.h"
#include "xpm_err.h"
#include "xpm_nodeid.h"
#include "xpm_psm.h"
#include "xpm_regs.h"
#include "xpm_subsystem.h"
#include "xsysmonpsv.h"
#include "xpm_ioctl.h"
#include "xpm_ipi.h"
#include "xpm_psm_api.h"
#include "xpm_powerdomain.h"
#include "xpm_pmcdomain.h"
#include "xpm_pslpdomain.h"
#include "xpm_psfpdomain.h"
#include "xpm_cpmdomain.h"
#include "xpm_hnicxdomain.h"
#include "xpm_pldomain.h"
#include "xpm_npdomain.h"
#include "xpm_notifier.h"
#include "xpm_pll.h"
#include "xpm_reset.h"
#include "xpm_domain_iso.h"
#include "xpm_pmc.h"
#include "xpm_apucore.h"
#include "xpm_rpucore.h"
#include "xpm_psm.h"
#include "xpm_periph.h"
#include "xpm_requirement.h"
#include "xpm_mem.h"
#include "xpm_debug.h"
#include "xpm_pldevice.h"
#include "xpm_powerdomain.h"
#define XPm_RegisterWakeUpHandler(GicId, SrcId, NodeId)	\
	{ \
		Status = XPlmi_GicRegisterHandler(GicId, SrcId, \
				XPm_DispatchWakeHandler, (void *)(NodeId)); \
		if (Status != XST_SUCCESS) {\
			goto END;\
		}\
	}

/* Macro to typecast PM API ID */
#define PM_API(ApiId)			((u32)ApiId)

#define PM_IOCTL_FEATURE_BITMASK ( \
	(1ULL << (u64)IOCTL_GET_APU_OPER_MODE) | \
	(1ULL << (u64)IOCTL_SET_APU_OPER_MODE) | \
	(1ULL << (u64)IOCTL_GET_RPU_OPER_MODE) | \
	(1ULL << (u64)IOCTL_SET_RPU_OPER_MODE) | \
	(1ULL << (u64)IOCTL_RPU_BOOT_ADDR_CONFIG) | \
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
	(1ULL << (u64)IOCTL_READ_REG) | \
	(1ULL << (u64)IOCTL_MASK_WRITE_REG))

static XStatus GetMemRegnForSubsystem(u32 SubsystemId, XPm_AddrRegion *AddrRegnArray,
	u32 AddrRegnArrayLen, u32 *NumOfRegions)
{
	XStatus Status = XST_FAILURE;
	const XPm_Subsystem *Subsystem;
	const XPm_Requirement *Reqm;
	const XPm_MemRegnDevice *MemRegnDevice;
	u64 Address, Size;
	u32 DeviceId;

	*NumOfRegions = 0U;

	Subsystem = XPmSubsystem_GetById(SubsystemId);
	if (NULL == Subsystem) {
		Status = XPM_INVALID_SUBSYSID;
		goto done;
	}

	Reqm = Subsystem->Requirements;

	/* Iterate over all devices for particular subsystem */
	while (NULL != Reqm) {
		DeviceId = Reqm->Device->Node.Id;

		if ((u32)XPM_NODESUBCL_DEV_MEM_REGN != NODESUBCLASS(DeviceId)) {
			Reqm = Reqm->NextDevice;
			continue;
		}

		if (AddrRegnArrayLen <= *NumOfRegions) {
			Status = XST_BUFFER_TOO_SMALL;
			goto done;
		}
		MemRegnDevice  = (XPm_MemRegnDevice *)Reqm->Device;
		Address = MemRegnDevice->AddrRegion.Address;
		Size = MemRegnDevice->AddrRegion.Size;

		if (IS_PL_MEM_REGN(Size)) {
			/* Zero-ing out upper flag bits[31:28] from 64bit size for PL */
			Size &= ~PL_MEM_REGN_FLAGS_MASK_64;
		}
		AddrRegnArray[*NumOfRegions].Address = Address;
		AddrRegnArray[*NumOfRegions].Size = Size;
		(*NumOfRegions)++;

		Reqm = Reqm->NextDevice;
	}
	Status = XST_SUCCESS;

done:
	return Status;
}


XStatus XPm_PlatCmnFlush(const u32 SubsystemId)
{
	XStatus Status = XST_FAILURE;
	u32 Idx;
	const u32 MemAddr[4] = {0xF8100000U, 0xF8900000U, 0xF9100000U, 0xF9900000U};
	XPm_AddrRegion AddrRegionarray[XPM_NODEIDX_DEV_MEM_REGN_MAX];
	u32 NumberOfRegions = 0U;
	/**
	 * Flush DDR and OCM addresses using ABF (Address based flush) for NID8, NID72,
	 * NID136 and NID200.
	 */
	Status = GetMemRegnForSubsystem(SubsystemId, AddrRegionarray,
					     ARRAY_SIZE(AddrRegionarray), &NumberOfRegions);
	if (XST_SUCCESS != Status) {
			goto done;
	}

	for (u32 i = 0; i < NumberOfRegions; i++) {
		for (Idx = 0U; Idx < 4U; Idx++) {
			XPm_Out32(MemAddr[Idx] + 0xF50U,
				  (u32)AddrRegionarray[i].Address);
			XPm_Out32(MemAddr[Idx] + 0xF54U,
				  (u32)(AddrRegionarray[i].Address >> 32));
			XPm_Out32(MemAddr[Idx] + 0xF58U,
				  (u32)(AddrRegionarray[i].Address
				   + AddrRegionarray[i].Size));
			XPm_Out32(MemAddr[Idx] + 0xF5CU,
				  (u32)((AddrRegionarray[i].Address
				   + AddrRegionarray[i].Size) >> 32));
			XPm_Out32(MemAddr[Idx] + 0xF60U, 0x1U);
			Status = XPm_PollForMask(MemAddr[Idx] + 0xF68U, 0x1U,
						 XPM_POLL_TIMEOUT);
			if (XST_SUCCESS != Status) {
				goto done;
			}
		}
	}

done:
	return Status;
}

/****************************************************************************/
/**
 * @brief  This function is used for HNICX NPI indirect write for a given
 *	   address.
 *
 * @param  Address	32 bit address
 * @param  Value	32 bit value to be written
 *
 * @return XST_SUCCESS if successful else XST_FAILURE
 *
 * @note   None
 *
 ****************************************************************************/
static XStatus XPm_HnicxNpiDataXfer(u32 Address, u32 Value)
{
	XStatus Status = XST_FAILURE;

	/* Fake read of status register */
	(void)XPm_In32(HNICX_NPI_0_BASEADDR +
		       HNICX_NPI_0_NPI_CSR_WR_STATUS_OFFSET);

	/* Write NPI data in NPI_CSR_WDATA register */
	XPm_Out32(HNICX_NPI_0_BASEADDR + HNICX_NPI_0_NPI_CSR_WDATA_OFFSET,
		  Value);

	/*
	 * Write address + set command for data write in NPI_CSR_INST
	 * register
	 */
	XPm_Out32(HNICX_NPI_0_BASEADDR + HNICX_NPI_0_NPI_CSR_INST_OFFSET,
		  ((Address & HNICX_NPI_0_NPI_CSR_INST_NPI_CSR_ADDR_MASK) |
		  HNICX_NPI_0_NPI_CSR_INST_NPI_CSR_CMD_WRITE));

	/* Wait for a valid write response for the successful transaction */
	Status = XPlmi_UtilPoll(HNICX_NPI_0_BASEADDR +
				HNICX_NPI_0_NPI_CSR_WR_STATUS_OFFSET,
				HNICX_NPI_0_NPI_CSR_WR_STATUS_MASK,
				HNICX_NPI_0_NPI_CSR_WR_STATUS_VALID_RESP,
				XPM_NPI_CSR_POLL_TIMEOUT, NULL);

	return Status;
}

int XPm_PlatProcessCmd(XPlmi_Cmd *Cmd)
{
	XStatus Status = XST_FAILURE;
	u32 CmdId = Cmd->CmdId & 0xFFU;
	const u32 *Pload = Cmd->Payload;
	u32 Len = Cmd->Len;

	switch (CmdId) {
	case PM_API(PM_BISR):
		if (XST_SUCCESS == XPmBisr_TagSupportCheck2(Pload[0])){
			Status =  XPmBisr_Repair2(Pload[0]);
		}else{
			Status =  XPmBisr_Repair(Pload[0]);
		}
		break;
	case PM_API(PM_INIT_NODE):
		Status = XPm_InitNode(Pload[0], Pload[1], &Pload[2], Len-2U);
		break;
	case PM_API(PM_APPLY_TRIM):
		Status = XPm_PldApplyTrim(Pload[0]);
		break;
	case PM_API(PM_HNICX_NPI_DATA_XFER):
		Status = XPm_HnicxNpiDataXfer(Pload[0], Pload[1]);
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
	XPm_RegisterWakeUpHandler((u32)XPLMI_PMC_GIC_IRQ_GICP0, (u32)XPLMI_GICP0_SRC23, XPM_NODEIDX_DEV_SPI_0);
	XPm_RegisterWakeUpHandler((u32)XPLMI_PMC_GIC_IRQ_GICP0, (u32)XPLMI_GICP0_SRC24, XPM_NODEIDX_DEV_SPI_1);
	XPm_RegisterWakeUpHandler((u32)XPLMI_PMC_GIC_IRQ_GICP0, (u32)XPLMI_GICP0_SRC25, XPM_NODEIDX_DEV_UART_0);
	XPm_RegisterWakeUpHandler((u32)XPLMI_PMC_GIC_IRQ_GICP0, (u32)XPLMI_GICP0_SRC26, XPM_NODEIDX_DEV_UART_1);
	XPm_RegisterWakeUpHandler((u32)XPLMI_PMC_GIC_IRQ_GICP0, (u32)XPLMI_GICP0_SRC29, XPM_NODEIDX_DEV_USB_0);
	XPm_RegisterWakeUpHandler((u32)XPLMI_PMC_GIC_IRQ_GICP0, (u32)XPLMI_GICP0_SRC30, XPM_NODEIDX_DEV_USB_0);
	XPm_RegisterWakeUpHandler((u32)XPLMI_PMC_GIC_IRQ_GICP0, (u32)XPLMI_GICP0_SRC31, XPM_NODEIDX_DEV_USB_0);
	XPm_RegisterWakeUpHandler((u32)XPLMI_PMC_GIC_IRQ_GICP1, (u32)XPLMI_GICP1_SRC0, XPM_NODEIDX_DEV_USB_0);
	XPm_RegisterWakeUpHandler((u32)XPLMI_PMC_GIC_IRQ_GICP1, (u32)XPLMI_GICP1_SRC1, XPM_NODEIDX_DEV_USB_0);
	XPm_RegisterWakeUpHandler((u32)XPLMI_PMC_GIC_IRQ_GICP1, (u32)XPLMI_GICP1_SRC2, XPM_NODEIDX_DEV_USB_1);
	XPm_RegisterWakeUpHandler((u32)XPLMI_PMC_GIC_IRQ_GICP1, (u32)XPLMI_GICP1_SRC3, XPM_NODEIDX_DEV_USB_1);
	XPm_RegisterWakeUpHandler((u32)XPLMI_PMC_GIC_IRQ_GICP1, (u32)XPLMI_GICP1_SRC4, XPM_NODEIDX_DEV_USB_1);
	XPm_RegisterWakeUpHandler((u32)XPLMI_PMC_GIC_IRQ_GICP1, (u32)XPLMI_GICP1_SRC5, XPM_NODEIDX_DEV_USB_1);
	XPm_RegisterWakeUpHandler((u32)XPLMI_PMC_GIC_IRQ_GICP1, (u32)XPLMI_GICP1_SRC6, XPM_NODEIDX_DEV_USB_1);
	XPm_RegisterWakeUpHandler((u32)XPLMI_PMC_GIC_IRQ_GICP5, (u32)XPLMI_GICP5_SRC24, XPM_NODEIDX_DEV_SDIO_0);
	XPm_RegisterWakeUpHandler((u32)XPLMI_PMC_GIC_IRQ_GICP5, (u32)XPLMI_GICP5_SRC25, XPM_NODEIDX_DEV_SDIO_0);

END:
	return Status;
}

XStatus XPm_PlatInit(void)
{
	u32 DefaultDomainIsoMask = PMC_GLOBAL_DOMAIN_ISO_CNTRL_VCCAUX_VCCRAM_MASK \
		| PMC_GLOBAL_DOMAIN_ISO_CNTRL_VCCRAM_SOC_MASK \
		| PMC_GLOBAL_DOMAIN_ISO_CNTRL_VCCAUX_SOC_MASK \
		| PMC_GLOBAL_DOMAIN_ISO_CNTRL_PL_SOC_MASK \
		| PMC_GLOBAL_DOMAIN_ISO_CNTRL_PMC_SOC_MASK \
		| PMC_GLOBAL_DOMAIN_ISO_CNTRL_PMC_SOC_NPI_MASK \
		| PMC_GLOBAL_DOMAIN_ISO_CNTRL_PMC_PL_MASK \
		| PMC_GLOBAL_DOMAIN_ISO_CNTRL_PMC_PL_TEST_MASK \
		| PMC_GLOBAL_DOMAIN_ISO_CNTRL_PMC_PL_CFRAME_MASK \
		| PMC_GLOBAL_DOMAIN_ISO_CNTRL_PMC_LPD_MASK \
		| PMC_GLOBAL_DOMAIN_ISO_CNTRL_PMC_LPD_DFX_MASK \
		| PMC_GLOBAL_DOMAIN_ISO_CNTRL_LPD_SOC_MASK \
		| PMC_GLOBAL_DOMAIN_ISO_CNTRL_LPD_PL_MASK \
		| PMC_GLOBAL_DOMAIN_ISO_CNTRL_LPD_PL_TEST_MASK \
		| PMC_GLOBAL_DOMAIN_ISO_CNTRL_LPD_CPM_MASK \
		| PMC_GLOBAL_DOMAIN_ISO_CNTRL_LPD_CPM_DFX_MASK \
		| PMC_GLOBAL_DOMAIN_ISO_CNTRL_FPD_SOC_MASK \
		| PMC_GLOBAL_DOMAIN_ISO_CNTRL_FPD_PL_MASK \
		| PMC_GLOBAL_DOMAIN_ISO_CNTRL_FPD_PL_TEST_MASK;
	/* Assert PS_PL_SRST */
	/* NOTE: other resets are already asserted in versal_common XPm_Init*/
	XPm_RMW32(CRP_RST_PS, CRP_RST_PS_PL_SRST_MASK, CRP_RST_PS_PL_SRST_MASK);
	/* Reenable all domain isolation in PMC_GLOBAL*/
	XPm_Write32(PMC_GLOBAL_DOMAIN_ISO_CNTRL, DefaultDomainIsoMask);
	return XST_SUCCESS;
}

void XPm_PlatChangeCoreState(XPm_Core *Core, const u32 State)
{
	if (PM_SUSPEND_STATE_CPU_OFF == State) {
		Core->Device.Node.State = (u8)XPM_DEVSTATE_PENDING_PWR_DWN;
	} else if (PM_SUSPEND_STATE_SUSPEND_TO_RAM == State) {
		Core->Device.Node.State = (u8)XPM_DEVSTATE_SUSPENDING;
	} else {
		/* Required by MISRA */
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
	u32 i = 0, j = 0UL, Prealloc = 0, Capability = 0;
	const XPm_Requirement *Req = NULL;
	const u32 DefaultPreallocDevList[][2] = {
		{PM_DEV_PSM_PROC, (u32)PM_CAP_ACCESS},
		{PM_DEV_UART_0, (u32)XPM_MAX_CAPABILITY},
		{PM_DEV_UART_1, (u32)XPM_MAX_CAPABILITY},
		{PM_DEV_ACPU_0_0, (u32)PM_CAP_ACCESS},
		{PM_DEV_ACPU_0_1, (u32)PM_CAP_ACCESS},
		{PM_DEV_ACPU_0_2, (u32)PM_CAP_ACCESS},
		{PM_DEV_ACPU_0_3, (u32)PM_CAP_ACCESS},
		{PM_DEV_ACPU_1_0, (u32)PM_CAP_ACCESS},
		{PM_DEV_ACPU_1_1, (u32)PM_CAP_ACCESS},
		{PM_DEV_ACPU_1_2, (u32)PM_CAP_ACCESS},
		{PM_DEV_ACPU_1_3, (u32)PM_CAP_ACCESS},
		{PM_DEV_ACPU_2_0, (u32)PM_CAP_ACCESS},
		{PM_DEV_ACPU_2_1, (u32)PM_CAP_ACCESS},
		{PM_DEV_ACPU_2_2, (u32)PM_CAP_ACCESS},
		{PM_DEV_ACPU_2_3, (u32)PM_CAP_ACCESS},
		{PM_DEV_ACPU_3_0, (u32)PM_CAP_ACCESS},
		{PM_DEV_ACPU_3_1, (u32)PM_CAP_ACCESS},
		{PM_DEV_ACPU_3_2, (u32)PM_CAP_ACCESS},
		{PM_DEV_ACPU_3_3, (u32)PM_CAP_ACCESS},
		{PM_DEV_SDIO_0, (u32)PM_CAP_ACCESS | (u32)PM_CAP_SECURE},
		{PM_DEV_QSPI, (u32)PM_CAP_ACCESS | (u32)PM_CAP_SECURE},
		{PM_DEV_OSPI, (u32)PM_CAP_ACCESS | (u32)PM_CAP_SECURE},
		{PM_DEV_RPU_A_0, (u32)PM_CAP_ACCESS | (u32)PM_CAP_SECURE},
		{PM_DEV_RPU_B_0, (u32)PM_CAP_ACCESS | (u32)PM_CAP_SECURE},
		{PM_DEV_IPI_0, (u32)PM_CAP_ACCESS},
		{PM_DEV_IPI_1, (u32)PM_CAP_ACCESS},
		{PM_DEV_IPI_2, (u32)PM_CAP_ACCESS},
		{PM_DEV_IPI_3, (u32)PM_CAP_ACCESS},
		{PM_DEV_IPI_4, (u32)PM_CAP_ACCESS},
		{PM_DEV_IPI_5, (u32)PM_CAP_ACCESS},
		{PM_DEV_IPI_6, (u32)PM_CAP_ACCESS},
		{PM_DEV_SDIO_1, (u32)PM_CAP_ACCESS | (u32)PM_CAP_SECURE},
		{PM_DEV_I2C_0, (u32)PM_CAP_ACCESS},
		{PM_DEV_I2C_1, (u32)PM_CAP_ACCESS},
		{PM_DEV_GEM_0, (u32)XPM_MAX_CAPABILITY | (u32)PM_CAP_SECURE},
		{PM_DEV_GEM_1, (u32)XPM_MAX_CAPABILITY | (u32)PM_CAP_SECURE},
		{PM_DEV_OCM_0_0, (u32)PM_CAP_ACCESS | (u32)PM_CAP_CONTEXT},
		{PM_DEV_OCM_0_1, (u32)PM_CAP_ACCESS | (u32)PM_CAP_CONTEXT},
		{PM_DEV_OCM_0_2, (u32)PM_CAP_ACCESS | (u32)PM_CAP_CONTEXT},
		{PM_DEV_OCM_0_3, (u32)PM_CAP_ACCESS | (u32)PM_CAP_CONTEXT},
		{PM_DEV_OCM_1_0, (u32)PM_CAP_ACCESS | (u32)PM_CAP_CONTEXT},
		{PM_DEV_OCM_1_1, (u32)PM_CAP_ACCESS | (u32)PM_CAP_CONTEXT},
		{PM_DEV_OCM_1_2, (u32)PM_CAP_ACCESS | (u32)PM_CAP_CONTEXT},
		{PM_DEV_OCM_1_3, (u32)PM_CAP_ACCESS | (u32)PM_CAP_CONTEXT},
		{PM_DEV_DDR_0, (u32)PM_CAP_ACCESS | (u32)PM_CAP_CONTEXT},
		{PM_DEV_TTC_0, (u32)PM_CAP_ACCESS},
		{PM_DEV_TTC_1, (u32)PM_CAP_ACCESS},
		{PM_DEV_TTC_2, (u32)PM_CAP_ACCESS},
		{PM_DEV_TTC_3, (u32)PM_CAP_ACCESS},
		{PM_DEV_GPIO_PMC, (u32)PM_CAP_ACCESS},
		{PM_DEV_GPIO, (u32)PM_CAP_ACCESS},
		{PM_DEV_TCM_A_0A, (u32)PM_CAP_ACCESS | (u32)PM_CAP_CONTEXT},
		{PM_DEV_TCM_A_0B, (u32)PM_CAP_ACCESS | (u32)PM_CAP_CONTEXT},
		{PM_DEV_TCM_A_0C, (u32)PM_CAP_ACCESS | (u32)PM_CAP_CONTEXT},
		{PM_DEV_TCM_A_1A, (u32)PM_CAP_ACCESS | (u32)PM_CAP_CONTEXT},
		{PM_DEV_TCM_A_1B, (u32)PM_CAP_ACCESS | (u32)PM_CAP_CONTEXT},
		{PM_DEV_TCM_A_1C, (u32)PM_CAP_ACCESS | (u32)PM_CAP_CONTEXT},
		{PM_DEV_TCM_B_0A, (u32)PM_CAP_ACCESS | (u32)PM_CAP_CONTEXT},
		{PM_DEV_TCM_B_0B, (u32)PM_CAP_ACCESS | (u32)PM_CAP_CONTEXT},
		{PM_DEV_TCM_B_0C, (u32)PM_CAP_ACCESS | (u32)PM_CAP_CONTEXT},
		{PM_DEV_TCM_B_1A, (u32)PM_CAP_ACCESS | (u32)PM_CAP_CONTEXT},
		{PM_DEV_TCM_B_1B, (u32)PM_CAP_ACCESS | (u32)PM_CAP_CONTEXT},
		{PM_DEV_TCM_B_1C, (u32)PM_CAP_ACCESS | (u32)PM_CAP_CONTEXT},
	};

	/*
	 * Only fill out default subsystem requirements:
	 *   - if no proc/mem/periph requirements are present
	 *   - if only 1 subsystem has been added
	 */
	Req = Subsystem->Requirements;
	while (NULL != Req) {
		u32 SubClass = NODESUBCLASS(Req->Device->Node.Id);
		/**
		 * Requirements can be present for non-plm managed nodes in PMC CDO
		 * (e.g. regnodes, ddrmcs etc.), thus only check for proc/mem/periph
		 * requirements which are usually present in subsystem definition;
		 * and stop as soon as first such requirement is found.
		 */
		if (((u32)XPM_NODESUBCL_DEV_CORE == SubClass) ||
		    ((u32)XPM_NODESUBCL_DEV_PERIPH == SubClass) ||
		    ((u32)XPM_NODESUBCL_DEV_MEM == SubClass)) {
			Status = XST_SUCCESS;
			break;
		}
		Req = Req->NextDevice;
	}
	if (XST_SUCCESS ==  Status) {
		XPm_SetOverlayCdoFlag(0U);
		goto done;
	}

	XPm_SetOverlayCdoFlag(1U);

	for (i = 0; i < (u32)XPM_NODEIDX_DEV_MAX; i++) {
		/*
		 * Note: XPmDevice_GetByIndex() assumes that the caller
		 * is responsible for validating the Node ID attributes
		 * other than node index.
		 */
		XPm_Device *Device = XPmDevice_GetByIndex(i);
		if ((NULL != Device) && (1U == XPmDevice_IsRequestable(Device->Node.Id))) {
			Prealloc = 0;
			Capability = 0;

			for (j = 0; j < ARRAY_SIZE(DefaultPreallocDevList); j++) {
				if (Device->Node.Id == DefaultPreallocDevList[j][0]) {
					Prealloc = 1;
					Capability = DefaultPreallocDevList[j][1];
					break;
				}
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
	}

	/* Add reset permissions */
	Status = XPmReset_AddPermForGlobalResets(Subsystem);
	if (XST_SUCCESS != Status) {
		goto done;
	}

	/* Add xGGS Permissions */
	j |= 1UL << IOCTL_PERM_READ_SHIFT_NS;
	j |= 1UL << IOCTL_PERM_WRITE_SHIFT_NS;
	j |= 1UL << IOCTL_PERM_READ_SHIFT_S;
	j |= 1UL << IOCTL_PERM_WRITE_SHIFT_S;
	for (i = PM_DEV_GGS_0; i <= PM_DEV_GGS_3; i++) {
		Status = XPm_AddDevRequirement(Subsystem, i, j, NULL, 0);
		if (XST_SUCCESS != Status) {
			goto done;
		}
	}

	for (i = PM_DEV_PGGS_0; i <= PM_DEV_PGGS_1; i++) {
		Status = XPm_AddDevRequirement(Subsystem, i, j, NULL, 0);
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
	XPm_Subsystem *Subsystem;

	/* TODO: Remove this when custom ADMA reset is added from topology */
	/* Make ADMA reset to custom reset */
	XPmReset_MakeAdmaResetCustom();

	/* If default subsystem is present, attempt to add requirements if needed. */
	Subsystem = XPmSubsystem_GetById(PM_SUBSYS_DEFAULT);
	if (((u32)1U == XPmSubsystem_GetMaxSubsysIdx()) && (NULL != Subsystem) &&
	    ((u8)ONLINE == Subsystem->State)) {
		if ((u8)TRUE == XPlmi_IsPlmUpdateDone()) {
			/** Skip adding Default Subsystem Requirements */
			Status = XST_SUCCESS;
			goto done;
		}
		Status = XPm_AddReqsDefaultSubsystem(Subsystem);
		if (XST_SUCCESS != Status) {
			goto done;
		}
	}

	Status = XST_SUCCESS;

done:
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


static XStatus PwrDomainInitNode(u32 NodeId, u32 Function, const u32 *Args, u32 NumArgs)
{
	XStatus Status = XST_FAILURE;
	XPm_PowerDomain *PwrDomainNode;

	PwrDomainNode = (XPm_PowerDomain *)XPmPower_GetById(NodeId);
	if (NULL == PwrDomainNode) {
		Status = XPM_PM_INVALID_NODE;
		goto done;
	}

	switch (NODEINDEX(NodeId)) {
	case (u32)XPM_NODEIDX_POWER_LPD:
	case (u32)XPM_NODEIDX_POWER_FPD:
	case (u32)XPM_NODEIDX_POWER_NOC:
	case (u32)XPM_NODEIDX_POWER_PLD:
	case (u32)XPM_NODEIDX_POWER_HNICX:
	case (u32)XPM_NODEIDX_POWER_CPM5N:
		Status = XPmPowerDomain_InitDomain(PwrDomainNode, Function,
						   Args, NumArgs);
		break;
	default:
		Status = XPM_INVALID_PWRDOMAIN;
		break;
	}

	/*
	 * Call LPD init to initialize required components
	 */
	if (((u32)XPM_NODEIDX_POWER_LPD == NODEINDEX(NodeId)) &&
	    ((u32)FUNC_INIT_FINISH == Function)
	    /* TODO: Comment out below condition once INIT_START call added for LPD */
	    /* && (XST_SUCCESS == Status) */) {
		/*
		 * Mark domain init status bit in DomainInitStatusReg
		 */
		XPm_RMW32(XPM_DOMAIN_INIT_STATUS_REG,0x2,0x2);
		/**
		 * PLM needs to request UART0 and UART1, otherwise XilPM
		 * will turn it off when it is not used by other processor.
		 * During such scenario when PLM/other application tries to
		 * print debug message, system may not work properly.
		 */
#if (XPLMI_UART_NUM_INSTANCES > 0U)
		Status = XPm_RequestDevice(PM_SUBSYS_PMC, PM_DEV_UART_0,
					   (u32)PM_CAP_ACCESS, XPM_MAX_QOS, 0U,
					   XPLMI_CMD_SECURE);
		if (XST_SUCCESS != Status) {
			PmErr("Error %d in request UART_0\r\n", Status);
			goto done;
		}
#endif

#if (XPLMI_UART_NUM_INSTANCES > 1U)
		Status = XPm_RequestDevice(PM_SUBSYS_PMC, PM_DEV_UART_1,
					   (u32)PM_CAP_ACCESS, XPM_MAX_QOS, 0U,
					   XPLMI_CMD_SECURE);
		if (XST_SUCCESS != Status) {
			PmErr("Error %d in request UART_1\r\n", Status);
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
		Status = XPlmi_IpiInit(XPmSubsystem_GetSubSysIdByIpiMask, XPm_ProcessPsmCmd);
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
/****************************************************************************/
/**
 * @brief  This function allows to initialize the node.
 *
 * @param  NodeId	Supported power domain nodes, PLD
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
	} else {
		Status = XPM_PM_INVALID_NODE;
		DbgErr = XPM_INT_ERR_INITNODE;
	}

	XPm_PrintDbgErr(Status, DbgErr);
	return Status;
}

/****************************************************************************/
/**
 * @brief  This function add isolation node to isolation topology database
 *
 * @param Args		pointer to isolation node arguments or payload
 * @param NumArgs	number of arguments or words in payload
 *
 * Format of payload/args (word aligned):
 *
 * +--------------------------------------------------------------------+
 * |Isolation Node ID(Node id include  class , subclass, type and Index)|
 * +--------------------------------------------+-----------------------+
 * |		   rsvd[31:8]			|      Format[7:0]	|
 * +--------------------------------------------+-----------------------+
 * |		  Format specific payload (can be multiple words)	|
 * |				   ...					|
 * +--------------------------------------------+-----------------------+
 * |		   rsvd[31:8]			|      Format[7:0]	|
 * +--------------------------------------------+-----------------------+
 * |		  Format specific payload (can be multiple words)	|
 * |				   ...					|
 * +--------------------------------------------------------------------+
 * |				   .					|
 * |				   .					|
 * |				   .					|
 * +--------------------------------------------------------------------+
 * Format entry for single word isolation control:
 * +--------------------------------------------+-----------------------+
 * |		   rsvd[31:8]			|      Format[7:0]	|
 * +--------------------------------------------+-----------------------+
 * |			       BaseAddress				|
 * +--------------------------------------------------------------------+
 * |			       Mask					|
 * +--------------------------------------------------------------------+
 *
 * Format entry for power domain dependencies:
 * +----------------+----------------------------+----------------------+
 * | rsvd[31:16]    | Dependencies Count[15:8]	 |	Format[7:0]	|
 * +-------------- -+ ---------------------------+----------------------+
 * |		       NodeID of Dependency0				|
 * +--------------------------------------------------------------------+
 * |			       ...					|
 * +--------------------------------------------------------------------+
 *
 * Format entry for AFI interface control:
 * +--------------------------------------------+-----------------------+
 * |		   rsvd[31:8]			|      Format[7:0]	|
 * +--------------------------------------------+-----------------------+
 * |			       BaseAddress				|
 * +--------------------------------------------------------------------+
 *
 * @return XST_SUCCESS if successful else XST_FAILURE or an error code
 * or a reason code
 *
 * @note   None
 *
 ****************************************************************************/
static XStatus XPm_AddNodeIsolation(const u32 *Args, u32 NumArgs)
{
	XStatus Status = XST_FAILURE;
	if (3U > NumArgs) {
		Status = XST_INVALID_PARAM;
		goto done;
	}
	/* Start at beginning of the payload*/
	u32 Index = 0U;
	/* NodeID is always at the first word in payload*/
	u32 NodeId = Args[Index++];
	u32 IsoFormat = 0U;

	u32 Dependencies[PM_ISO_MAX_NUM_DEPENDENCIES] = {0U};
	u32 BaseAddr = 0U, Mask = 0U, NumDependencies = 0U;

	XPm_IsoCdoArgsFormat Format = SINGLE_WORD_ACTIVE_LOW;
	while(Index < NumArgs) {
		/* Extract format and number of dependencies*/
		Format = (XPm_IsoCdoArgsFormat)CDO_ISO_ARG_FORMAT(Args[Index]);
		NumDependencies = CDO_ISO_DEP_COUNT(Args[Index++]);

		switch (Format){
		case SINGLE_WORD_ACTIVE_LOW:
		case SINGLE_WORD_ACTIVE_HIGH:
		case PSM_SINGLE_WORD_ACTIVE_LOW:
		case PSM_SINGLE_WORD_ACTIVE_HIGH:
			IsoFormat = Format;
			/* Extract BaseAddress and Mask*/
			BaseAddr = Args[Index++];
			Mask = Args[Index++];
			break;
		case PM_ISO_FORMAT_AFI_FM:
			IsoFormat = Format;
			BaseAddr = Args[Index++];
			Mask = AFI_FM_PORT_EN_MASK;
			break;
		case PM_ISO_FORMAT_AFI_FS:
			IsoFormat = Format;
			/* Extract BaseAddress */
			BaseAddr = Args[Index++];
			Mask = AFI_FS_PORT_EN_MASK;
			break;
		case PM_ISO_FORMAT_CHI_FPD:
			IsoFormat = Format;
			/* Extract BaseAddress */
			BaseAddr = Args[Index++];
			Mask = CHI_FPD_PORT_EN_MASK;
			break;
		case PM_ISO_FORMAT_ACP_APU:
			IsoFormat = Format;
			/* Extract BaseAddress */
			BaseAddr = Args[Index++];
			Mask = ACP_APU_GET_MASK(NodeId);
			break;
		case PM_ISO_FORMAT_PS_DTI:
			IsoFormat = Format;
			/* Extract BaseAddress */
			BaseAddr = Args[Index++];
			Mask = PS_DTI_GET_MASK(NodeId);
			break;
		case POWER_DOMAIN_DEPENDENCY:
			/* Format power domain dependencies*/
			/* To save space in PMC RAM we statically allocate Dependencies list*/
			if (NumDependencies > PM_ISO_MAX_NUM_DEPENDENCIES){
				Status = XPM_INT_ERR_ISO_MAX_DEPENDENCIES;
				goto done;
			}else {
				for (u32 i = 0U ; i < NumDependencies; i++){
					Dependencies[i] = Args[Index++];
				}
			}
			break;
		default:
			Status = XPM_INT_ERR_ISO_INVALID_FORMAT;
			goto done;
		}
	}

	Status = XPmDomainIso_NodeInit(NodeId, BaseAddr, Mask, IsoFormat, Dependencies,
					NumDependencies);
done:
	return Status;
}

XStatus XPm_PlatAddNodePower(const u32 *Args, u32 NumArgs)
{
	XStatus Status = XST_FAILURE;
	u32 PowerId;
	u32 PowerType;
	u8 Width;
	u8 Shift;
	u32 BitMask;
	u32 ParentId;
	XPm_Power *PowerParent = NULL;
	XPm_HnicxDomain *HnicxDomain;

	/* Warning Fix */
	(void)NumArgs;

	PowerId = Args[0];
	PowerType = NODETYPE(PowerId);
	Width = (u8)(Args[1] >> 8) & 0xFFU;
	Shift = (u8)(Args[1] & 0xFFU);
	ParentId = Args[2];

	BitMask = BITNMASK(Shift, Width);

	if ((ParentId != (u32)XPM_NODEIDX_POWER_MIN)) {
		PowerParent = XPmPower_GetById(ParentId);
		if (NULL == PowerParent) {
			Status = XST_DEVICE_NOT_FOUND;
			goto done;
		}
	}

	switch (PowerType) {
	case (u32)XPM_NODETYPE_POWER_DOMAIN_HNICX:
		HnicxDomain = (XPm_HnicxDomain *)XPm_AllocBytes(sizeof(XPm_HnicxDomain));
		if (NULL == HnicxDomain) {
			Status = XST_BUFFER_TOO_SMALL;
			goto done;
		}
		Status = XPmHnicxDomain_Init(HnicxDomain, PowerId, BitMask, PowerParent);
		break;
	default:
		Status = XST_INVALID_PARAM;
		break;
	}

done:
	return Status;
}

XStatus XPm_PlatAddNode(const u32 *Args, u32 NumArgs)
{
	XStatus Status = XST_FAILURE;
	u32 Id = Args[0];

	switch (NODECLASS(Id)) {
	case (u32)XPM_NODECLASS_ISOLATION:
		Status = XPm_AddNodeIsolation(Args, NumArgs);
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
 * @note   None
 *
 ****************************************************************************/
XStatus XPm_PlatFeatureCheck(const u32 ApiId, u32 *const Version)
{
	XStatus Status = XST_FAILURE;

	switch (ApiId) {
	case PM_API(PM_BISR):
	case PM_API(PM_APPLY_TRIM):
		*Version = XST_API_BASE_VERSION;
		Status = XST_SUCCESS;
		break;
	case PM_API(PM_IOCTL):
		Version[0] = XST_API_PM_IOCTL_VERSION;
		Version[1] = (u32)(PM_IOCTL_FEATURE_BITMASK);
		Version[2] = (u32)(PM_IOCTL_FEATURE_BITMASK >> 32);
		Status = XST_SUCCESS;
		break;
	default:
		*Version = 0U;
		Status = XPM_NO_FEATURE;
		break;
	}

	return Status;
}
