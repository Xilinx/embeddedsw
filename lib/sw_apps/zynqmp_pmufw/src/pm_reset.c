/*
* Copyright (c) 2014 - 2021 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
 */

#include "xpfw_config.h"
#ifdef ENABLE_PM

/*********************************************************************
 * Reset lines definitions: information needed to configure reset
 * (configuration register address and mask, pointer to a function that performs
 * a pulse on the reset line. If the function pointer points to NULL,
 * PmResetPulse() function is called.
 * Every reset line must have its entry in pmAllResets table.
 *********************************************************************/

#include "xpfw_util.h"
#include "xpfw_default.h"
#include "xpfw_rom_interface.h"
#include "xpfw_resets.h"
#include "pm_reset.h"
#include "pm_defs.h"
#include "pm_common.h"
#include "pm_master.h"
#include "crl_apb.h"
#include "crf_apb.h"
#include "pmu_iomodule.h"
#include "sleep.h"

/**
 * PmResetOps - Reset operations
 * @resetAssert	Assert or release reset line
 * @getStatus	Get current status of reset line
 * @pulse	Function performing reset pulse operation
 */
typedef struct PmResetOps {
	void (*const resetAssert)(const PmReset* const rst, const u32 action);
	u32 (*const getStatus)(const PmReset* const s);
	u32 (*const pulse)(const PmReset* const rst);
} PmResetOps;

/**
 * PmReset - Base structure containing information common to all reset lines
 * @ops		Pointer to the reset operations structure
 * @access	Access control bitmask (1 bit per master, see 'pmAllMasters')
 * @derived	Pointer to derived reset structure
 */
struct PmReset {
	const PmResetOps* const ops;
	void* const derived;
	u32 access;
};

/**
 * PmResetGeneric - Generic reset structure
 * @rst		Base reset structure
 * @ctrlAddr	Address of the reset control register
 * @mask	Bitmask of the control register
 */
typedef struct PmResetGeneric {
	PmReset rst;
	const u32 ctrlAddr;
	const u32 mask;
} PmResetGeneric;

/**
 * PmResetGpo - Reset structure used for PMU's GPO3 going to PL
 * @rst		Base reset structure
 * @ctrlAddr	Address of the reset control register
 * @mask	Bitmask of the control and status register
 * @statusAddr	Address of the reset status register
 */
typedef struct PmResetGpo {
	PmReset rst;
	const u32 ctrlAddr;
	const u32 mask;
	const u32 statusAddr;
} PmResetGpo;

/**
 * PmResetRom - Reset which has dedicated PMU ROM function performing pulse
 * @rst		Base reset structure
 * @ctrlAddr	Address of the reset control register
 * @mask	Bitmask of the control register
 * @pulseRom	PMU ROM function performing pulse reset
 */
typedef struct PmResetRom {
	PmReset rst;
	u32 (*const pulseRom)(void);
	const u32 ctrlAddr;
	const u32 mask;
} PmResetRom;

/**
 * PmResetGpioBankIOs - Reset structure used for GPIO Bank Input/Outputs
 * @rst		Base reset structure
 * @rstLine	GPIO Bank reset line number
 */
typedef struct PmResetGpioBankIOs {
	PmReset rst;
	const u32 rstMaskDataReg;
	const u32 rstDirectionReg;
	const u32 rstReadDataReg;
	const bool isMaskDataLsw;
	const u8 rstLine;
} PmResetGpioBankIOs;

bool PmResetMasterHasAccess(const PmMaster* const m, const PmReset* const r)
{
	return ((0U != (r->access & m->ipiMask)) ? true : false);
}

/**
 * PmResetAssertCommon() - Common assert handler
 * @ctrlAddr	Address of the reset control register
 * @mask		Bitmask of the control register
 * @action	States whether to assert or release reset line
 */
static void PmResetAssertCommon(const u32 ctrlAddr, const u32 mask,
				 const u32 action)
{
	if (PM_RESET_ACTION_RELEASE == action) {
		XPfw_RMW32(ctrlAddr, mask, 0U);
	} else if (PM_RESET_ACTION_ASSERT == action) {
		XPfw_RMW32(ctrlAddr, mask, mask);
	} else {
		/* For MISRA compliance */
	}
}

/**
 * PmResetGetStatusCommon() - Common function for getting reset status
 * @addr	Address of the reset status register
 * @mask	Bitmask of the status register
 */
static u32 PmResetGetStatusCommon(const u32 addr, const u32 mask)
{
	u32 resetStatus = 0U;

	if ((Xil_In32(addr) & mask) == mask) {
		resetStatus = 1U;
	}

	return resetStatus;
}

/**
 * PmResetPulseCommon() - Common function for performing reset pulse
 * @ctrlAddr	Address of the control register
 * @mask	Bitmask of the control register
 */
static u32 PmResetPulseCommon(const u32 ctrlAddr, const u32 mask)
{
	XPfw_RMW32(ctrlAddr, mask, mask);
	XPfw_RMW32(ctrlAddr, mask, 0U);

	return XST_SUCCESS;
}

/**
 * PmResetAssertGen() - Assert handler for PmResetGeneric reset class
 * @rstPtr	Pointer to the reset that needs to be asserted or released
 * @action	States whether to assert or release reset line
 */
static void PmResetAssertGen(const PmReset *const rstPtr, const u32 action)
{
	const PmResetGeneric *rstGenPtr = (PmResetGeneric*)rstPtr->derived;

	PmResetAssertCommon(rstGenPtr->ctrlAddr, rstGenPtr->mask, action);
}

/**
 * PmResetGetStatusGen() - Get reset status handler of PmResetGeneric class
 * @rstPtr	Reset whose status should be returned
 *
 * @return	Current reset status (0 - released, 1 - asserted)
 */
static u32 PmResetGetStatusGen(const PmReset *const rstPtr)
{
	const PmResetGeneric *rstGenPtr = (PmResetGeneric*)rstPtr->derived;

	return PmResetGetStatusCommon(rstGenPtr->ctrlAddr, rstGenPtr->mask);
}

/**
 * PmResetPulseGen() - Pulse handler for PmResetGeneric class
 * @rstPtr	Pointer to the reset that needs to be toggled
 *
 * @return	Operation success
 */
static u32 PmResetPulseGen(const PmReset *const rstPtr)
{
	const PmResetGeneric *rstGenPtr = (PmResetGeneric*)rstPtr->derived;

	return PmResetPulseCommon(rstGenPtr->ctrlAddr, rstGenPtr->mask);
}

/**
 * PmResetAssertGpo() - Assert handler for PmResetGPO reset class
 * @rstPtr	Pointer to the reset line that needs to be asserted or released
 * @action	States whether to assert or release reset line
 */
static void PmResetAssertGpo(const PmReset *const rstPtr, const u32 action)
{
	const PmResetGpo *rstGpoPtr = (PmResetGpo*)rstPtr->derived;

	PmResetAssertCommon(rstGpoPtr->ctrlAddr, rstGpoPtr->mask, action);
}

/**
 * PmResetGetStatusGpo() - Get reset status handler of PmResetGpo class
 * @rstPtr	Reset whose status should be returned
 *
 * @return	Current reset status (0 - released, 1 - asserted)
 */
static u32 PmResetGetStatusGpo(const PmReset *const rstPtr)
{
	const PmResetGpo *rstGpoPtr = (PmResetGpo*)rstPtr->derived;

	return PmResetGetStatusCommon(rstGpoPtr->statusAddr, rstGpoPtr->mask);
}

/**
 * PmResetPulseGpo() - Pulse handler for PmResetGpo class
 * @rstPtr	Pointer to the reset that needs to be toggled
 *
 * @return	Operation success
 */
static u32 PmResetPulseGpo(const PmReset *const rstPtr)
{
	const PmResetGpo *rstGpoPtr = (PmResetGpo*)rstPtr->derived;

	return PmResetPulseCommon(rstGpoPtr->ctrlAddr, rstGpoPtr->mask);
}

/**
 * PmResetAssertRom() - Assert handler for PmResetRom reset class
 * @rstPtr	Pointer to the reset line that needs to be asserted or released
 * @action	States whether to assert or release reset line
 */
static void PmResetAssertRom(const PmReset *const rstPtr, const u32 action)
{
	const PmResetRom *rstRomPtr = (PmResetRom*)rstPtr->derived;

	PmResetAssertCommon(rstRomPtr->ctrlAddr, rstRomPtr->mask, action);
}

/**
 * PmResetGetStatusRom() - Get reset status handler of PmResetRom class
 * @rstPtr	Reset whose status should be returned
 *
 * @return	Current reset status (0 - released, 1 - asserted)
 */
static u32 PmResetGetStatusRom(const PmReset *const rstPtr)
{
	const PmResetRom *rstRomPtr = (PmResetRom*)rstPtr->derived;

	return PmResetGetStatusCommon(rstRomPtr->ctrlAddr, rstRomPtr->mask);
}

/**
 * PmResetPulseRom() - Pulse handler for PmResetRom class
 * @rstPtr	Pointer to the reset that needs to be toggled
 *
 * @return	Operation success
 */
static u32 PmResetPulseRom(const PmReset *const rstPtr)
{
	const PmResetRom *rstRomPtr = (PmResetRom*)rstPtr->derived;

	return rstRomPtr->pulseRom();
}

/**
 * PmResetGetStatusGpioBankIOs() - Get reset status handler for GPIO bank
 * @rstPtr	Pointer to the reset whose status should be returned
 *
 * @return	Current reset status (0 - released, 1 - asserted)
 */
static u32 PmResetGetStatusGpioBankIOs(const PmReset* const rstPtr)
{
	const PmResetGpioBankIOs *rstGpioPtr = (PmResetGpioBankIOs*)rstPtr->derived;
	u32 RegShift = ((u32)MAX_REG_BITS/2U) + ((u32)rstGpioPtr->rstLine);
	u32 RegVal = 0;

	/* Read the MIO/EMIO data register */
	RegVal = Xil_In32(rstGpioPtr->rstReadDataReg);

	return ((RegVal >> RegShift) & 1U);
}

/**
 * PmResetPulseGpioBankIOs() - Pulse handler for PmResetGpioBankIOs class
 * @rstPtr	Pointer to the reset that needs to be toggled
 *
 * @return	Operation success
 */
static u32 PmResetPulseGpioBankIOs(const PmReset* const rstPtr)
{
	const PmResetGpioBankIOs *rstGpioPtr = (PmResetGpioBankIOs*)rstPtr->derived;
	u32 GpioRstLine = rstGpioPtr->rstLine;
	u32 GpioMaskDataReg = rstGpioPtr->rstMaskDataReg;
	u32 GpioDirReg = rstGpioPtr->rstDirectionReg;
	u32 RegVal = 0;
	u32 MaskVal = 0;
	u32 DirmShift = 0;

	/* Set MIO/EMIO Direction */
	if( rstGpioPtr->isMaskDataLsw == false ) {
		DirmShift = MAX_REG_BITS/2;
	}
	RegVal = Xil_In32(GpioDirReg) | ((u32)1 << (DirmShift + GpioRstLine));
	Xil_Out32(GpioDirReg, RegVal);

	/* Assert the MIO/EMIO with the required Mask */
	MaskVal = ((u32)1 << GpioRstLine) | GPIO_PIN_MASK_BITS;
	RegVal = MaskVal & (~((u32)1 << (((u32)MAX_REG_BITS/2U) + GpioRstLine)));
	Xil_Out32(GpioMaskDataReg, RegVal);
	usleep(1000);

	/* De-assert the MIO/EMIO with the required Mask */
	RegVal = (~((u32)1 << (((u32)MAX_REG_BITS/2U) + GpioRstLine))) & GPIO_PIN_MASK_BITS;
	Xil_Out32(GpioMaskDataReg, RegVal);
	usleep(1000);

	/* Assert the MIO/EMIO with the required Mask */
	MaskVal = ((u32)1 << GpioRstLine) | GPIO_PIN_MASK_BITS;
	RegVal = MaskVal & (~((u32)1 << (((u32)MAX_REG_BITS/2U) + GpioRstLine)));
	Xil_Out32(GpioMaskDataReg, RegVal);
	usleep(1000);

	return XST_SUCCESS;
}

/**
 * PmUserHookResetAssertPl() - Assert reset handler for PL
 * @rstPtr	Pointer to the PL reset structure
 * @action	States whether to assert or release reset
 */
#pragma weak PmUserHookResetAssertPl
static void PmUserHookResetAssertPl(const PmReset *const rstPtr, const u32 action)
{
}

/**
 * PmResetAssertPl() - Assert reset handler for PL
 * @rstPtr	Pointer to the PL reset structure
 * @action	States whether to assert or release reset
 */
static void PmResetAssertPl(const PmReset *const rstPtr, const u32 action)
{
	PmUserHookResetAssertPl(rstPtr, action);
}

/**
 * PmUserHookResetGetStatusPl() - Get PL reset status handler
 * @rstPtr	Pointer to PL reset structure
 *
 * @return	Current reset status (0 - released, 1 - asserted)
 */
#pragma weak PmUserHookResetGetStatusPl
static u32 PmUserHookResetGetStatusPl(const PmReset *const rstPtr)
{
	return 0;
}

/**
 * PmResetGetStatusPl() - Get PL reset status handler
 * @rstPtr	Pointer to PL reset structure
 *
 * @return	Current reset status (0 - released, 1 - asserted)
 */
static u32 PmResetGetStatusPl(const PmReset *const rstPtr)
{
	return PmUserHookResetGetStatusPl(rstPtr);
}

/**
 * PmUserHookResetPulsePl() - PL reset pulse handler
 * @rstPtr	Pointer to PL reset structure
 *
 * @return	Operation status
 */
#pragma weak PmUserHookResetPulsePl
static u32 PmUserHookResetPulsePl(const PmReset* const rst)
{
	return XST_SUCCESS;
}

/**
 * PmResetPulsePl() - PL reset pulse handler
 * @rstPtr	Pointer to PL reset structure
 *
 * @return	Operation status
 */
static u32 PmResetPulsePl(const PmReset* const rst)
{
	return PmUserHookResetPulsePl(rst);
}

static const PmResetOps pmResetOpsGeneric = {
	.resetAssert = PmResetAssertGen,
	.getStatus = PmResetGetStatusGen,
	.pulse = PmResetPulseGen,
};

static const PmResetOps pmResetOpsGpo = {
	.resetAssert = PmResetAssertGpo,
	.getStatus = PmResetGetStatusGpo,
	.pulse = PmResetPulseGpo,
};

static const PmResetOps pmResetOpsRom = {
	.resetAssert = PmResetAssertRom,
	.getStatus = PmResetGetStatusRom,
	.pulse = PmResetPulseRom,
};

static const PmResetOps pmResetOpsNoAssert = {
	.resetAssert = NULL,
	.getStatus = PmResetGetStatusRom,
	.pulse = PmResetPulseRom,
};

static const PmResetOps pmResetOpsPl = {
	.resetAssert = PmResetAssertPl,
	.getStatus = PmResetGetStatusPl,
	.pulse = PmResetPulsePl,
};

static const PmResetOps pmResetOpsGpioBankIO = {
	.resetAssert = NULL,
	.getStatus = PmResetGetStatusGpioBankIOs,
	.pulse = PmResetPulseGpioBankIOs,
};

static PmResetGeneric pmResetPcieCfg = {
	.rst = {
		.ops = &pmResetOpsGeneric,
		.access = 0U,
		.derived = &pmResetPcieCfg,
	},
	.ctrlAddr = CRF_APB_RST_FPD_TOP,
	.mask = CRF_APB_RST_FPD_TOP_PCIE_CFG_RESET_MASK,
};

static PmResetGeneric pmResetPcieBridge = {
	.rst = {
		.ops = &pmResetOpsGeneric,
		.access = 0U,
		.derived = &pmResetPcieBridge,
	},
	.ctrlAddr = CRF_APB_RST_FPD_TOP,
	.mask = CRF_APB_RST_FPD_TOP_PCIE_BRIDGE_RESET_MASK,
};

static PmResetRom pmResetPcieCtrl = {
	.rst = {
		.ops = &pmResetOpsRom,
		.access = 0U,
		.derived = &pmResetPcieCtrl,
	},
	.ctrlAddr = CRF_APB_RST_FPD_TOP,
	.mask = CRF_APB_RST_FPD_TOP_PCIE_CTRL_RESET_MASK,
	.pulseRom = XpbrRstPCIeHandler,
};

static PmResetRom pmResetDp = {
	.rst = {
		.ops = &pmResetOpsRom,
		.access = 0U,
		.derived = &pmResetDp,
	},
	.ctrlAddr = CRF_APB_RST_FPD_TOP,
	.mask = CRF_APB_RST_FPD_TOP_DP_RESET_MASK,
	.pulseRom = XpbrRstDisplayPortHandler,
};

static PmResetGeneric pmResetSwdtCrf = {
	.rst = {
		.ops = &pmResetOpsGeneric,
		.access = 0U,
		.derived = &pmResetSwdtCrf,
	},
	.ctrlAddr = CRF_APB_RST_FPD_TOP,
	.mask = CRF_APB_RST_FPD_TOP_SWDT_RESET_MASK,
};

static PmResetGeneric pmResetAfiFm5 = {
	.rst = {
		.ops = &pmResetOpsGeneric,
		.access = 0U,
		.derived = &pmResetAfiFm5,
	},
	.ctrlAddr = CRF_APB_RST_FPD_TOP,
	.mask = CRF_APB_RST_FPD_TOP_AFI_FM5_RESET_MASK,
};

static PmResetGeneric pmResetAfiFm4 = {
	.rst = {
		.ops = &pmResetOpsGeneric,
		.access = 0U,
		.derived = &pmResetAfiFm4,
	},
	.ctrlAddr = CRF_APB_RST_FPD_TOP,
	.mask = CRF_APB_RST_FPD_TOP_AFI_FM4_RESET_MASK,
};

static PmResetGeneric pmResetAfiFm3 = {
	.rst = {
		.ops = &pmResetOpsGeneric,
		.access = 0U,
		.derived = &pmResetAfiFm3,
	},
	.ctrlAddr = CRF_APB_RST_FPD_TOP,
	.mask = CRF_APB_RST_FPD_TOP_AFI_FM3_RESET_MASK,
};

static PmResetGeneric pmResetAfiFm2 = {
	.rst = {
		.ops = &pmResetOpsGeneric,
		.access = 0U,
		.derived = &pmResetAfiFm2,
	},
	.ctrlAddr = CRF_APB_RST_FPD_TOP,
	.mask = CRF_APB_RST_FPD_TOP_AFI_FM2_RESET_MASK,
};

static PmResetGeneric pmResetAfiFm1 = {
	.rst = {
		.ops = &pmResetOpsGeneric,
		.access = 0U,
		.derived = &pmResetAfiFm1,
	},
	.ctrlAddr = CRF_APB_RST_FPD_TOP,
	.mask = CRF_APB_RST_FPD_TOP_AFI_FM1_RESET_MASK,
};

static PmResetGeneric pmResetAfiFm0 = {
	.rst = {
		.ops = &pmResetOpsGeneric,
		.access = 0U,
		.derived = &pmResetAfiFm0,
	},
	.ctrlAddr = CRF_APB_RST_FPD_TOP,
	.mask = CRF_APB_RST_FPD_TOP_AFI_FM0_RESET_MASK,
};

static PmResetGeneric pmResetGdma = {
	.rst = {
		.ops = &pmResetOpsGeneric,
		.access = 0U,
		.derived = &pmResetGdma,
	},
	.ctrlAddr = CRF_APB_RST_FPD_TOP,
	.mask = CRF_APB_RST_FPD_TOP_GDMA_RESET_MASK,
};

static PmResetRom pmResetGpuPp1 = {
	.rst = {
		.ops = &pmResetOpsRom,
		.access = 0U,
		.derived = &pmResetGpuPp1,
	},
	.ctrlAddr = CRF_APB_RST_FPD_TOP,
	.mask = CRF_APB_RST_FPD_TOP_GPU_PP1_RESET_MASK,
	.pulseRom = XpbrRstPp1Handler,
};

static PmResetRom pmResetGpuPp0 = {
	.rst = {
		.ops = &pmResetOpsRom,
		.access = 0U,
		.derived = &pmResetGpuPp0,
	},
	.ctrlAddr = CRF_APB_RST_FPD_TOP,
	.mask = CRF_APB_RST_FPD_TOP_GPU_PP0_RESET_MASK,
	.pulseRom = XpbrRstPp0Handler,
};

static PmResetRom pmResetGpu = {
	.rst = {
		.ops = &pmResetOpsRom,
		.access = 0U,
		.derived = &pmResetGpu,
	},
	.ctrlAddr = CRF_APB_RST_FPD_TOP,
	.mask = CRF_APB_RST_FPD_TOP_GPU_RESET_MASK,
	.pulseRom = XpbrRstGpuHandler,
};

static PmResetGeneric pmResetGt = {
	.rst = {
		.ops = &pmResetOpsGeneric,
		.access = 0U,
		.derived = &pmResetGt,
	},
	.ctrlAddr = CRF_APB_RST_FPD_TOP,
	.mask = CRF_APB_RST_FPD_TOP_GT_RESET_MASK,
};

static PmResetRom pmResetSata = {
	.rst = {
		.ops = &pmResetOpsRom,
		.access = 0U,
		.derived = &pmResetSata,
	},
	.ctrlAddr = CRF_APB_RST_FPD_TOP,
	.mask = CRF_APB_RST_FPD_TOP_SATA_RESET_MASK,
	.pulseRom = XpbrRstSataHandler,
};

static PmResetGeneric pmResetAcpu3Pwron = {
	.rst = {
		.ops = &pmResetOpsGeneric,
		.access = 0U,
		.derived = &pmResetAcpu3Pwron,
	},
	.ctrlAddr = CRF_APB_RST_FPD_APU,
	.mask = CRF_APB_RST_FPD_APU_ACPU3_PWRON_RESET_MASK,
};

static PmResetGeneric pmResetAcpu2Pwron = {
	.rst = {
		.ops = &pmResetOpsGeneric,
		.access = 0U,
		.derived = &pmResetAcpu2Pwron,
	},
	.ctrlAddr = CRF_APB_RST_FPD_APU,
	.mask = CRF_APB_RST_FPD_APU_ACPU2_PWRON_RESET_MASK,
};

static PmResetGeneric pmResetAcpu1Pwron = {
	.rst = {
		.ops = &pmResetOpsGeneric,
		.access = 0U,
		.derived = &pmResetAcpu1Pwron,
	},
	.ctrlAddr = CRF_APB_RST_FPD_APU,
	.mask = CRF_APB_RST_FPD_APU_ACPU1_PWRON_RESET_MASK,
};

static PmResetGeneric pmResetAcpu0Pwron = {
	.rst = {
		.ops = &pmResetOpsGeneric,
		.access = 0U,
		.derived = &pmResetAcpu0Pwron,
	},
	.ctrlAddr = CRF_APB_RST_FPD_APU,
	.mask = CRF_APB_RST_FPD_APU_ACPU0_PWRON_RESET_MASK,
};

static PmResetGeneric pmResetApuL2 = {
	.rst = {
		.ops = &pmResetOpsGeneric,
		.access = 0U,
		.derived = &pmResetApuL2,
	},
	.ctrlAddr = CRF_APB_RST_FPD_APU,
	.mask = CRF_APB_RST_FPD_APU_APU_L2_RESET_MASK,
};

static PmResetRom pmResetAcpu3 = {
	.rst = {
		.ops = &pmResetOpsRom,
		.access = 0U,
		.derived = &pmResetAcpu3,
	},
	.ctrlAddr = CRF_APB_RST_FPD_APU,
	.mask = CRF_APB_RST_FPD_APU_ACPU3_RESET_MASK,
	.pulseRom = XpbrRstACPU3CPHandler,
};

static PmResetRom pmResetAcpu2 = {
	.rst = {
		.ops = &pmResetOpsRom,
		.access = 0U,
		.derived = &pmResetAcpu2,
	},
	.ctrlAddr = CRF_APB_RST_FPD_APU,
	.mask = CRF_APB_RST_FPD_APU_ACPU2_RESET_MASK,
	.pulseRom = XpbrRstACPU2CPHandler,
};

static PmResetRom pmResetAcpu1 = {
	.rst = {
		.ops = &pmResetOpsRom,
		.access = 0U,
		.derived = &pmResetAcpu1,
	},
	.ctrlAddr = CRF_APB_RST_FPD_APU,
	.mask = CRF_APB_RST_FPD_APU_ACPU1_RESET_MASK,
	.pulseRom = XpbrRstACPU1CPHandler,
};

static PmResetRom pmResetAcpu0 = {
	.rst = {
		.ops = &pmResetOpsRom,
		.access = 0U,
		.derived = &pmResetAcpu0,
	},
	.ctrlAddr = CRF_APB_RST_FPD_APU,
	.mask = CRF_APB_RST_FPD_APU_ACPU0_RESET_MASK,
	.pulseRom = XpbrRstACPU0CPHandler,
};

static PmResetGeneric pmResetDDR = {
	.rst = {
		.ops = &pmResetOpsGeneric,
		.access = 0U,
		.derived = &pmResetDDR,
	},
	.ctrlAddr = CRF_APB_RST_DDR_SS,
	.mask = CRF_APB_RST_DDR_SS_DDR_RESET_MASK,
};

static PmResetGeneric pmResetApmFpd = {
	.rst = {
		.ops = &pmResetOpsGeneric,
		.access = 0U,
		.derived = &pmResetApmFpd,
	},
	.ctrlAddr = CRF_APB_RST_DDR_SS,
	.mask = CRF_APB_RST_DDR_SS_APM_RESET_MASK,
};

static PmResetGeneric pmResetSoft = {
	.rst = {
		.ops = &pmResetOpsGeneric,
		.access = 0U,
		.derived = &pmResetSoft,
	},
	.ctrlAddr = CRL_APB_RESET_CTRL,
	.mask = CRL_APB_RESET_CTRL_SOFT_RESET_MASK,
};

static PmResetRom pmResetGem0 = {
	.rst = {
		.ops = &pmResetOpsRom,
		.access = 0U,
		.derived = &pmResetGem0,
	},
	.ctrlAddr = CRL_APB_RST_LPD_IOU0,
	.mask = CRL_APB_RST_LPD_IOU0_GEM0_RESET_MASK,
	.pulseRom = XpbrRstGem0Handler,
};

static PmResetRom pmResetGem1 = {
	.rst = {
		.ops = &pmResetOpsRom,
		.access = 0U,
		.derived = &pmResetGem1,
	},
	.ctrlAddr = CRL_APB_RST_LPD_IOU0,
	.mask = CRL_APB_RST_LPD_IOU0_GEM1_RESET_MASK,
	.pulseRom = XpbrRstGem1Handler,
};

static PmResetRom pmResetGem2 = {
	.rst = {
		.ops = &pmResetOpsRom,
		.access = 0U,
		.derived = &pmResetGem2,
	},
	.ctrlAddr = CRL_APB_RST_LPD_IOU0,
	.mask = CRL_APB_RST_LPD_IOU0_GEM2_RESET_MASK,
	.pulseRom = XpbrRstGem2Handler,
};

static PmResetRom pmResetGem3 = {
	.rst = {
		.ops = &pmResetOpsRom,
		.access = 0U,
		.derived = &pmResetGem3,
	},
	.ctrlAddr = CRL_APB_RST_LPD_IOU0,
	.mask = CRL_APB_RST_LPD_IOU0_GEM3_RESET_MASK,
	.pulseRom = XpbrRstGem3Handler,
};

static PmResetGeneric pmResetQspi = {
	.rst = {
		.ops = &pmResetOpsGeneric,
		.access = 0U,
		.derived = &pmResetQspi,
	},
	.ctrlAddr = CRL_APB_RST_LPD_IOU2,
	.mask = CRL_APB_RST_LPD_IOU2_QSPI_RESET_MASK,
};

static PmResetGeneric pmResetUart0 = {
	.rst = {
		.ops = &pmResetOpsGeneric,
		.access = 0U,
		.derived = &pmResetUart0,
	},
	.ctrlAddr = CRL_APB_RST_LPD_IOU2,
	.mask = CRL_APB_RST_LPD_IOU2_UART0_RESET_MASK,
};

static PmResetGeneric pmResetUart1 = {
	.rst = {
		.ops = &pmResetOpsGeneric,
		.access = 0U,
		.derived = &pmResetUart1,
	},
	.ctrlAddr = CRL_APB_RST_LPD_IOU2,
	.mask = CRL_APB_RST_LPD_IOU2_UART1_RESET_MASK,
};

static PmResetGeneric pmResetSpi0 = {
	.rst = {
		.ops = &pmResetOpsGeneric,
		.access = 0U,
		.derived = &pmResetSpi0,
	},
	.ctrlAddr = CRL_APB_RST_LPD_IOU2,
	.mask = CRL_APB_RST_LPD_IOU2_SPI0_RESET_MASK,
};

static PmResetGeneric pmResetSpi1 = {
	.rst = {
		.ops = &pmResetOpsGeneric,
		.access = 0U,
		.derived = &pmResetSpi1,
	},
	.ctrlAddr = CRL_APB_RST_LPD_IOU2,
	.mask = CRL_APB_RST_LPD_IOU2_SPI1_RESET_MASK,
};

static PmResetGeneric pmResetSdio0 = {
	.rst = {
		.ops = &pmResetOpsGeneric,
		.access = 0U,
		.derived = &pmResetSdio0,
	},
	.ctrlAddr = CRL_APB_RST_LPD_IOU2,
	.mask = CRL_APB_RST_LPD_IOU2_SDIO0_RESET_MASK,
};

static PmResetGeneric pmResetSdio1 = {
	.rst = {
		.ops = &pmResetOpsGeneric,
		.access = 0U,
		.derived = &pmResetSdio1,
	},
	.ctrlAddr = CRL_APB_RST_LPD_IOU2,
	.mask = CRL_APB_RST_LPD_IOU2_SDIO1_RESET_MASK,
};

static PmResetGeneric pmResetCan0 = {
	.rst = {
		.ops = &pmResetOpsGeneric,
		.access = 0U,
		.derived = &pmResetCan0,
	},
	.ctrlAddr = CRL_APB_RST_LPD_IOU2,
	.mask = CRL_APB_RST_LPD_IOU2_CAN0_RESET_MASK,
};

static PmResetGeneric pmResetCan1 = {
	.rst = {
		.ops = &pmResetOpsGeneric,
		.access = 0U,
		.derived = &pmResetCan1,
	},
	.ctrlAddr = CRL_APB_RST_LPD_IOU2,
	.mask = CRL_APB_RST_LPD_IOU2_CAN1_RESET_MASK,
};

static PmResetGeneric pmResetI2C0 = {
	.rst = {
		.ops = &pmResetOpsGeneric,
		.access = 0U,
		.derived = &pmResetI2C0,
	},
	.ctrlAddr = CRL_APB_RST_LPD_IOU2,
	.mask = CRL_APB_RST_LPD_IOU2_I2C0_RESET_MASK,
};

static PmResetGeneric pmResetI2C1 = {
	.rst = {
		.ops = &pmResetOpsGeneric,
		.access = 0U,
		.derived = &pmResetI2C1,
	},
	.ctrlAddr = CRL_APB_RST_LPD_IOU2,
	.mask = CRL_APB_RST_LPD_IOU2_I2C1_RESET_MASK,
};

static PmResetGeneric pmResetTtc0 = {
	.rst = {
		.ops = &pmResetOpsGeneric,
		.access = 0U,
		.derived = &pmResetTtc0,
	},
	.ctrlAddr = CRL_APB_RST_LPD_IOU2,
	.mask = CRL_APB_RST_LPD_IOU2_TTC0_RESET_MASK,
};

static PmResetGeneric pmResetTtc1 = {
	.rst = {
		.ops = &pmResetOpsGeneric,
		.access = 0U,
		.derived = &pmResetTtc1,
	},
	.ctrlAddr = CRL_APB_RST_LPD_IOU2,
	.mask = CRL_APB_RST_LPD_IOU2_TTC1_RESET_MASK,
};

static PmResetGeneric pmResetTtc2 = {
	.rst = {
		.ops = &pmResetOpsGeneric,
		.access = 0U,
		.derived = &pmResetTtc2,
	},
	.ctrlAddr = CRL_APB_RST_LPD_IOU2,
	.mask = CRL_APB_RST_LPD_IOU2_TTC2_RESET_MASK,
};

static PmResetGeneric pmResetTtc3 = {
	.rst = {
		.ops = &pmResetOpsGeneric,
		.access = 0U,
		.derived = &pmResetTtc3,
	},
	.ctrlAddr = CRL_APB_RST_LPD_IOU2,
	.mask = CRL_APB_RST_LPD_IOU2_TTC3_RESET_MASK,
};

static PmResetGeneric pmResetSwdtCrl = {
	.rst = {
		.ops = &pmResetOpsGeneric,
		.access = 0U,
		.derived = &pmResetSwdtCrl,
	},
	.ctrlAddr = CRL_APB_RST_LPD_IOU2,
	.mask = CRL_APB_RST_LPD_IOU2_SWDT_RESET_MASK,
};

static PmResetGeneric pmResetNand = {
	.rst = {
		.ops = &pmResetOpsGeneric,
		.access = 0U,
		.derived = &pmResetNand,
	},
	.ctrlAddr = CRL_APB_RST_LPD_IOU2,
	.mask = CRL_APB_RST_LPD_IOU2_NAND_RESET_MASK,
};

static PmResetGeneric pmResetAdma = {
	.rst = {
		.ops = &pmResetOpsGeneric,
		.access = 0U,
		.derived = &pmResetAdma,
	},
	.ctrlAddr = CRL_APB_RST_LPD_IOU2,
	.mask = CRL_APB_RST_LPD_IOU2_ADMA_RESET_MASK,
};

static PmResetGeneric pmResetGpio = {
	.rst = {
		.ops = &pmResetOpsGeneric,
		.access = 0U,
		.derived = &pmResetGpio,
	},
	.ctrlAddr = CRL_APB_RST_LPD_IOU2,
	.mask = CRL_APB_RST_LPD_IOU2_GPIO_RESET_MASK,
};

static PmResetGeneric pmResetIouCc = {
	.rst = {
		.ops = &pmResetOpsGeneric,
		.access = 0U,
		.derived = &pmResetIouCc,
	},
	.ctrlAddr = CRL_APB_RST_LPD_IOU2,
	.mask = CRL_APB_RST_LPD_IOU2_IOU_CC_RESET_MASK,
};

static PmResetGeneric pmResetTimestamp = {
	.rst = {
		.ops = &pmResetOpsGeneric,
		.access = 0U,
		.derived = &pmResetTimestamp,
	},
	.ctrlAddr = CRL_APB_RST_LPD_IOU2,
	.mask = CRL_APB_RST_LPD_IOU2_TIMESTAMP_RESET_MASK,
};

static PmResetRom pmResetRpuR50 = {
	.rst = {
		.ops = &pmResetOpsRom,
		.access = 0U,
		.derived = &pmResetRpuR50,
	},
	.ctrlAddr = CRL_APB_RST_LPD_TOP,
	.mask = CRL_APB_RST_LPD_TOP_RPU_R50_RESET_MASK,
	.pulseRom = XpbrRstR50Handler,
};

static PmResetRom pmResetRpuR51 = {
	.rst = {
		.ops = &pmResetOpsRom,
		.access = 0U,
		.derived = &pmResetRpuR51,
	},
	.ctrlAddr = CRL_APB_RST_LPD_TOP,
	.mask = CRL_APB_RST_LPD_TOP_RPU_R51_RESET_MASK,
	.pulseRom = XpbrRstR51Handler,
};

static PmResetGeneric pmResetRpuAmba = {
	.rst = {
		.ops = &pmResetOpsGeneric,
		.access = 0U,
		.derived = &pmResetRpuAmba,
	},
	.ctrlAddr = CRL_APB_RST_LPD_TOP,
	.mask = CRL_APB_RST_LPD_TOP_RPU_AMBA_RESET_MASK,
};

static PmResetGeneric pmResetOcm = {
	.rst = {
		.ops = &pmResetOpsGeneric,
		.access = 0U,
		.derived = &pmResetOcm,
	},
	.ctrlAddr = CRL_APB_RST_LPD_TOP,
	.mask = CRL_APB_RST_LPD_TOP_OCM_RESET_MASK,
};

static PmResetGeneric pmResetRpuPge = {
	.rst = {
		.ops = &pmResetOpsGeneric,
		.access = 0U,
		.derived = &pmResetRpuPge,
	},
	.ctrlAddr = CRL_APB_RST_LPD_TOP,
	.mask = CRL_APB_RST_LPD_TOP_RPU_PGE_RESET_MASK,
};

static PmResetRom pmResetUsb0Core = {
	.rst = {
		.ops = &pmResetOpsRom,
		.access = 0U,
		.derived = &pmResetUsb0Core,
	},
	.ctrlAddr = CRL_APB_RST_LPD_TOP,
	.mask = CRL_APB_RST_LPD_TOP_USB0_CORERESET_MASK,
	.pulseRom = XpbrRstUsb0Handler,
};

static PmResetRom pmResetUsb1Core = {
	.rst = {
		.ops = &pmResetOpsRom,
		.access = 0U,
		.derived = &pmResetUsb1Core,
	},
	.ctrlAddr = CRL_APB_RST_LPD_TOP,
	.mask = CRL_APB_RST_LPD_TOP_USB1_CORERESET_MASK,
	.pulseRom = XpbrRstUsb1Handler,
};

static PmResetGeneric pmResetUsb0Hiber = {
	.rst = {
		.ops = &pmResetOpsGeneric,
		.access = 0U,
		.derived = &pmResetUsb0Hiber,
	},
	.ctrlAddr = CRL_APB_RST_LPD_TOP,
	.mask = CRL_APB_RST_LPD_TOP_USB0_HIBERRESET_MASK,
};

static PmResetGeneric pmResetUsb1Hiber = {
	.rst = {
		.ops = &pmResetOpsGeneric,
		.access = 0U,
		.derived = &pmResetUsb1Hiber,
	},
	.ctrlAddr = CRL_APB_RST_LPD_TOP,
	.mask = CRL_APB_RST_LPD_TOP_USB1_HIBERRESET_MASK,
};

static PmResetGeneric pmResetUsb0Apb = {
	.rst = {
		.ops = &pmResetOpsGeneric,
		.access = 0U,
		.derived = &pmResetUsb0Apb,
	},
	.ctrlAddr = CRL_APB_RST_LPD_TOP,
	.mask = CRL_APB_RST_LPD_TOP_USB0_APB_RESET_MASK,
};

static PmResetGeneric pmResetUsb1Apb = {
	.rst = {
		.ops = &pmResetOpsGeneric,
		.access = 0U,
		.derived = &pmResetUsb1Apb,
	},
	.ctrlAddr = CRL_APB_RST_LPD_TOP,
	.mask = CRL_APB_RST_LPD_TOP_USB1_APB_RESET_MASK,
};

static PmResetGeneric pmResetIpi = {
	.rst = {
		.ops = &pmResetOpsGeneric,
		.access = 0U,
		.derived = &pmResetIpi,
	},
	.ctrlAddr = CRL_APB_RST_LPD_TOP,
	.mask = CRL_APB_RST_LPD_TOP_IPI_RESET_MASK,
};

static PmResetGeneric pmResetApmLpd = {
	.rst = {
		.ops = &pmResetOpsGeneric,
		.access = 0U,
		.derived = &pmResetApmLpd,
	},
	.ctrlAddr = CRL_APB_RST_LPD_TOP,
	.mask = CRL_APB_RST_LPD_TOP_APM_RESET_MASK,
};

static PmResetGeneric pmResetRtc = {
	.rst = {
		.ops = &pmResetOpsGeneric,
		.access = 0U,
		.derived = &pmResetRtc,
	},
	.ctrlAddr = CRL_APB_RST_LPD_TOP,
	.mask = CRL_APB_RST_LPD_TOP_RTC_RESET_MASK,
};

static PmResetGeneric pmResetSysmon = {
	.rst = {
		.ops = &pmResetOpsGeneric,
		.access = 0U,
		.derived = &pmResetSysmon,
	},
	.ctrlAddr = CRL_APB_RST_LPD_TOP,
	.mask = CRL_APB_RST_LPD_TOP_SYSMON_RESET_MASK,
};

static PmResetGeneric pmResetAfiFm6 = {
	.rst = {
		.ops = &pmResetOpsGeneric,
		.access = 0U,
		.derived = &pmResetAfiFm6,
	},
	.ctrlAddr = CRL_APB_RST_LPD_TOP,
	.mask = CRL_APB_RST_LPD_TOP_AFI_FM6_RESET_MASK,
};

static PmResetGeneric pmResetLpdSwdt = {
	.rst = {
		.ops = &pmResetOpsGeneric,
		.access = 0U,
		.derived = &pmResetLpdSwdt,
	},
	.ctrlAddr = CRL_APB_RST_LPD_TOP,
	.mask = CRL_APB_RST_LPD_TOP_LPD_SWDT_RESET_MASK,
};

/**
 * PmResetPulseFpd() - Gracefully cycle FPD reset
 *
 * @return	Operation status
 */
static u32 PmResetPulseFpd(void)
{
	return (u32)XPfw_ResetFpd();
}

static PmResetRom pmResetFpd = {
	.rst = {
		.ops = &pmResetOpsRom,
		.access = 0U,
		.derived = &pmResetFpd,
	},
	.ctrlAddr = CRL_APB_RST_LPD_TOP,
	.mask = CRL_APB_RST_LPD_TOP_FPD_RESET_MASK,
	.pulseRom = PmResetPulseFpd,
};

static PmResetGeneric pmResetRpuDbg1 = {
	.rst = {
		.ops = &pmResetOpsGeneric,
		.access = 0U,
		.derived = &pmResetRpuDbg1,
	},
	.ctrlAddr = CRL_APB_RST_LPD_DBG,
	.mask = CRL_APB_RST_LPD_DBG_RPU_DBG1_RESET_MASK,
};

static PmResetGeneric pmResetRpuDbg0 = {
	.rst = {
		.ops = &pmResetOpsGeneric,
		.access = 0U,
		.derived = &pmResetRpuDbg0,
	},
	.ctrlAddr = CRL_APB_RST_LPD_DBG,
	.mask = CRL_APB_RST_LPD_DBG_RPU_DBG0_RESET_MASK,
};

static PmResetGeneric pmResetDbgLpd = {
	.rst = {
		.ops = &pmResetOpsGeneric,
		.access = 0U,
		.derived = &pmResetDbgLpd,
	},
	.ctrlAddr = CRL_APB_RST_LPD_DBG,
	.mask = CRL_APB_RST_LPD_DBG_DBG_LPD_RESET_MASK,
};

static PmResetGeneric pmResetDbgFpd = {
	.rst = {
		.ops = &pmResetOpsGeneric,
		.access = 0U,
		.derived = &pmResetDbgFpd,
	},
	.ctrlAddr = CRL_APB_RST_LPD_DBG,
	.mask = CRL_APB_RST_LPD_DBG_DBG_FPD_RESET_MASK,
};

static PmResetGeneric pmResetApll = {
	.rst = {
		.ops = &pmResetOpsGeneric,
		.access = 0U,
		.derived = &pmResetApll,
	},
	.ctrlAddr = CRF_APB_APLL_CTRL,
	.mask = CRF_APB_APLL_CTRL_RESET_MASK,
};

static PmResetGeneric pmResetDpll = {
	.rst = {
		.ops = &pmResetOpsGeneric,
		.access = 0U,
		.derived = &pmResetDpll,
	},
	.ctrlAddr = CRF_APB_DPLL_CTRL,
	.mask = CRF_APB_DPLL_CTRL_RESET_MASK,
};

static PmResetGeneric pmResetVpll = {
	.rst = {
		.ops = &pmResetOpsGeneric,
		.access = 0U,
		.derived = &pmResetVpll,
	},
	.ctrlAddr = CRF_APB_VPLL_CTRL,
	.mask = CRF_APB_VPLL_CTRL_RESET_MASK,
};

static PmResetGeneric pmResetIopll = {
	.rst = {
		.ops = &pmResetOpsGeneric,
		.access = 0U,
		.derived = &pmResetIopll,
	},
	.ctrlAddr = CRL_APB_IOPLL_CTRL,
	.mask = CRL_APB_IOPLL_CTRL_RESET_MASK,
};

static PmResetGeneric pmResetRpll = {
	.rst = {
		.ops = &pmResetOpsGeneric,
		.access = 0U,
		.derived = &pmResetRpll,
	},
	.ctrlAddr = CRL_APB_RPLL_CTRL,
	.mask = CRL_APB_RPLL_CTRL_RESET_MASK,
};

static PmResetGpo pmResetGpo3Pl0 = {
	.rst = {
		.ops = &pmResetOpsGpo,
		.access = 0U,
		.derived = &pmResetGpo3Pl0,
	},
	.ctrlAddr = PMU_IOMODULE_GPO3,
	.mask = PMU_IOMODULE_GPO3_PL_GPO_0_MASK,
	.statusAddr = PMU_LOCAL_GPO3_READ,
};

static PmResetGpo pmResetGpo3Pl1 = {
	.rst = {
		.ops = &pmResetOpsGpo,
		.access = 0U,
		.derived = &pmResetGpo3Pl1,
	},
	.ctrlAddr = PMU_IOMODULE_GPO3,
	.mask = PMU_IOMODULE_GPO3_PL_GPO_1_MASK,
	.statusAddr = PMU_LOCAL_GPO3_READ,
};

static PmResetGpo pmResetGpo3Pl2 = {
	.rst = {
		.ops = &pmResetOpsGpo,
		.access = 0U,
		.derived = &pmResetGpo3Pl2,
	},
	.ctrlAddr = PMU_IOMODULE_GPO3,
	.mask = PMU_IOMODULE_GPO3_PL_GPO_2_MASK,
	.statusAddr = PMU_LOCAL_GPO3_READ,
};

static PmResetGpo pmResetGpo3Pl3 = {
	.rst = {
		.ops = &pmResetOpsGpo,
		.access = 0U,
		.derived = &pmResetGpo3Pl3,
	},
	.ctrlAddr = PMU_IOMODULE_GPO3,
	.mask = PMU_IOMODULE_GPO3_PL_GPO_3_MASK,
	.statusAddr = PMU_LOCAL_GPO3_READ,
};

static PmResetGpo pmResetGpo3Pl4 = {
	.rst = {
		.ops = &pmResetOpsGpo,
		.access = 0U,
		.derived = &pmResetGpo3Pl4,
	},
	.ctrlAddr = PMU_IOMODULE_GPO3,
	.mask = PMU_IOMODULE_GPO3_PL_GPO_4_MASK,
	.statusAddr = PMU_LOCAL_GPO3_READ,
};

static PmResetGpo pmResetGpo3Pl5 = {
	.rst = {
		.ops = &pmResetOpsGpo,
		.access = 0U,
		.derived = &pmResetGpo3Pl5,
	},
	.ctrlAddr = PMU_IOMODULE_GPO3,
	.mask = PMU_IOMODULE_GPO3_PL_GPO_5_MASK,
	.statusAddr = PMU_LOCAL_GPO3_READ,
};

static PmResetGpo pmResetGpo3Pl6 = {
	.rst = {
		.ops = &pmResetOpsGpo,
		.access = 0U,
		.derived = &pmResetGpo3Pl6,
	},
	.ctrlAddr = PMU_IOMODULE_GPO3,
	.mask = PMU_IOMODULE_GPO3_PL_GPO_6_MASK,
	.statusAddr = PMU_LOCAL_GPO3_READ,
};

static PmResetGpo pmResetGpo3Pl7 = {
	.rst = {
		.ops = &pmResetOpsGpo,
		.access = 0U,
		.derived = &pmResetGpo3Pl7,
	},
	.ctrlAddr = PMU_IOMODULE_GPO3,
	.mask = PMU_IOMODULE_GPO3_PL_GPO_7_MASK,
	.statusAddr = PMU_LOCAL_GPO3_READ,
};

static PmResetGpo pmResetGpo3Pl8 = {
	.rst = {
		.ops = &pmResetOpsGpo,
		.access = 0U,
		.derived = &pmResetGpo3Pl8,
	},
	.ctrlAddr = PMU_IOMODULE_GPO3,
	.mask = PMU_IOMODULE_GPO3_PL_GPO_8_MASK,
	.statusAddr = PMU_LOCAL_GPO3_READ,
};

static PmResetGpo pmResetGpo3Pl9 = {
	.rst = {
		.ops = &pmResetOpsGpo,
		.access = 0U,
		.derived = &pmResetGpo3Pl9,
	},
	.ctrlAddr = PMU_IOMODULE_GPO3,
	.mask = PMU_IOMODULE_GPO3_PL_GPO_9_MASK,
	.statusAddr = PMU_LOCAL_GPO3_READ,
};

static PmResetGpo pmResetGpo3Pl10 = {
	.rst = {
		.ops = &pmResetOpsGpo,
		.access = 0U,
		.derived = &pmResetGpo3Pl10,
	},
	.ctrlAddr = PMU_IOMODULE_GPO3,
	.mask = PMU_IOMODULE_GPO3_PL_GPO_10_MASK,
	.statusAddr = PMU_LOCAL_GPO3_READ,
};

static PmResetGpo pmResetGpo3Pl11 = {
	.rst = {
		.ops = &pmResetOpsGpo,
		.access = 0U,
		.derived = &pmResetGpo3Pl11,
	},
	.ctrlAddr = PMU_IOMODULE_GPO3,
	.mask = PMU_IOMODULE_GPO3_PL_GPO_11_MASK,
	.statusAddr = PMU_LOCAL_GPO3_READ,
};

static PmResetGpo pmResetGpo3Pl12 = {
	.rst = {
		.ops = &pmResetOpsGpo,
		.access = 0U,
		.derived = &pmResetGpo3Pl12,
	},
	.ctrlAddr = PMU_IOMODULE_GPO3,
	.mask = PMU_IOMODULE_GPO3_PL_GPO_12_MASK,
	.statusAddr = PMU_LOCAL_GPO3_READ,
};

static PmResetGpo pmResetGpo3Pl13 = {
	.rst = {
		.ops = &pmResetOpsGpo,
		.access = 0U,
		.derived = &pmResetGpo3Pl13,
	},
	.ctrlAddr = PMU_IOMODULE_GPO3,
	.mask = PMU_IOMODULE_GPO3_PL_GPO_13_MASK,
	.statusAddr = PMU_LOCAL_GPO3_READ,
};

static PmResetGpo pmResetGpo3Pl14 = {
	.rst = {
		.ops = &pmResetOpsGpo,
		.access = 0U,
		.derived = &pmResetGpo3Pl14,
	},
	.ctrlAddr = PMU_IOMODULE_GPO3,
	.mask = PMU_IOMODULE_GPO3_PL_GPO_14_MASK,
	.statusAddr = PMU_LOCAL_GPO3_READ,
};

static PmResetGpo pmResetGpo3Pl15 = {
	.rst = {
		.ops = &pmResetOpsGpo,
		.access = 0U,
		.derived = &pmResetGpo3Pl15,
	},
	.ctrlAddr = PMU_IOMODULE_GPO3,
	.mask = PMU_IOMODULE_GPO3_PL_GPO_15_MASK,
	.statusAddr = PMU_LOCAL_GPO3_READ,
};

static PmResetGpo pmResetGpo3Pl16 = {
	.rst = {
		.ops = &pmResetOpsGpo,
		.access = 0U,
		.derived = &pmResetGpo3Pl16,
	},
	.ctrlAddr = PMU_IOMODULE_GPO3,
	.mask = PMU_IOMODULE_GPO3_PL_GPO_16_MASK,
	.statusAddr = PMU_LOCAL_GPO3_READ,
};

static PmResetGpo pmResetGpo3Pl17 = {
	.rst = {
		.ops = &pmResetOpsGpo,
		.access = 0U,
		.derived = &pmResetGpo3Pl17,
	},
	.ctrlAddr = PMU_IOMODULE_GPO3,
	.mask = PMU_IOMODULE_GPO3_PL_GPO_17_MASK,
	.statusAddr = PMU_LOCAL_GPO3_READ,
};

static PmResetGpo pmResetGpo3Pl18 = {
	.rst = {
		.ops = &pmResetOpsGpo,
		.access = 0U,
		.derived = &pmResetGpo3Pl18,
	},
	.ctrlAddr = PMU_IOMODULE_GPO3,
	.mask = PMU_IOMODULE_GPO3_PL_GPO_18_MASK,
	.statusAddr = PMU_LOCAL_GPO3_READ,
};

static PmResetGpo pmResetGpo3Pl19 = {
	.rst = {
		.ops = &pmResetOpsGpo,
		.access = 0U,
		.derived = &pmResetGpo3Pl19,
	},
	.ctrlAddr = PMU_IOMODULE_GPO3,
	.mask = PMU_IOMODULE_GPO3_PL_GPO_19_MASK,
	.statusAddr = PMU_LOCAL_GPO3_READ,
};

static PmResetGpo pmResetGpo3Pl20 = {
	.rst = {
		.ops = &pmResetOpsGpo,
		.access = 0U,
		.derived = &pmResetGpo3Pl20,
	},
	.ctrlAddr = PMU_IOMODULE_GPO3,
	.mask = PMU_IOMODULE_GPO3_PL_GPO_20_MASK,
	.statusAddr = PMU_LOCAL_GPO3_READ,
};

static PmResetGpo pmResetGpo3Pl21 = {
	.rst = {
		.ops = &pmResetOpsGpo,
		.access = 0U,
		.derived = &pmResetGpo3Pl21,
	},
	.ctrlAddr = PMU_IOMODULE_GPO3,
	.mask = PMU_IOMODULE_GPO3_PL_GPO_21_MASK,
	.statusAddr = PMU_LOCAL_GPO3_READ,
};

static PmResetGpo pmResetGpo3Pl22 = {
	.rst = {
		.ops = &pmResetOpsGpo,
		.access = 0U,
		.derived = &pmResetGpo3Pl22,
	},
	.ctrlAddr = PMU_IOMODULE_GPO3,
	.mask = PMU_IOMODULE_GPO3_PL_GPO_22_MASK,
	.statusAddr = PMU_LOCAL_GPO3_READ,
};

static PmResetGpo pmResetGpo3Pl23 = {
	.rst = {
		.ops = &pmResetOpsGpo,
		.access = 0U,
		.derived = &pmResetGpo3Pl23,
	},
	.ctrlAddr = PMU_IOMODULE_GPO3,
	.mask = PMU_IOMODULE_GPO3_PL_GPO_23_MASK,
	.statusAddr = PMU_LOCAL_GPO3_READ,
};

static PmResetGpo pmResetGpo3Pl24 = {
	.rst = {
		.ops = &pmResetOpsGpo,
		.access = 0U,
		.derived = &pmResetGpo3Pl24,
	},
	.ctrlAddr = PMU_IOMODULE_GPO3,
	.mask = PMU_IOMODULE_GPO3_PL_GPO_24_MASK,
	.statusAddr = PMU_LOCAL_GPO3_READ,
};

static PmResetGpo pmResetGpo3Pl25 = {
	.rst = {
		.ops = &pmResetOpsGpo,
		.access = 0U,
		.derived = &pmResetGpo3Pl25,
	},
	.ctrlAddr = PMU_IOMODULE_GPO3,
	.mask = PMU_IOMODULE_GPO3_PL_GPO_25_MASK,
	.statusAddr = PMU_LOCAL_GPO3_READ,
};

static PmResetGpo pmResetGpo3Pl26 = {
	.rst = {
		.ops = &pmResetOpsGpo,
		.access = 0U,
		.derived = &pmResetGpo3Pl26,
	},
	.ctrlAddr = PMU_IOMODULE_GPO3,
	.mask = PMU_IOMODULE_GPO3_PL_GPO_26_MASK,
	.statusAddr = PMU_LOCAL_GPO3_READ,
};

static PmResetGpo pmResetGpo3Pl27 = {
	.rst = {
		.ops = &pmResetOpsGpo,
		.access = 0U,
		.derived = &pmResetGpo3Pl27,
	},
	.ctrlAddr = PMU_IOMODULE_GPO3,
	.mask = PMU_IOMODULE_GPO3_PL_GPO_27_MASK,
	.statusAddr = PMU_LOCAL_GPO3_READ,
};

static PmResetGpo pmResetGpo3Pl28 = {
	.rst = {
		.ops = &pmResetOpsGpo,
		.access = 0U,
		.derived = &pmResetGpo3Pl28,
	},
	.ctrlAddr = PMU_IOMODULE_GPO3,
	.mask = PMU_IOMODULE_GPO3_PL_GPO_28_MASK,
	.statusAddr = PMU_LOCAL_GPO3_READ,
};

static PmResetGpo pmResetGpo3Pl29 = {
	.rst = {
		.ops = &pmResetOpsGpo,
		.access = 0U,
		.derived = &pmResetGpo3Pl29,
	},
	.ctrlAddr = PMU_IOMODULE_GPO3,
	.mask = PMU_IOMODULE_GPO3_PL_GPO_29_MASK,
	.statusAddr = PMU_LOCAL_GPO3_READ,
};

static PmResetGpo pmResetGpo3Pl30 = {
	.rst = {
		.ops = &pmResetOpsGpo,
		.access = 0U,
		.derived = &pmResetGpo3Pl30,
	},
	.ctrlAddr = PMU_IOMODULE_GPO3,
	.mask = PMU_IOMODULE_GPO3_PL_GPO_30_MASK,
	.statusAddr = PMU_LOCAL_GPO3_READ,
};

static PmResetGpo pmResetGpo3Pl31 = {
	.rst = {
		.ops = &pmResetOpsGpo,
		.access = 0U,
		.derived = &pmResetGpo3Pl31,
	},
	.ctrlAddr = PMU_IOMODULE_GPO3,
	.mask = PMU_IOMODULE_GPO3_PL_GPO_31_MASK,
	.statusAddr = PMU_LOCAL_GPO3_READ,
};

/*
 * GPIO5 EMIO[95:92] are the reset lines going to PL
 */
static PmResetGpioBankIOs pmResetGpio5EMIO92 = {
	.rst = {
		.ops = &pmResetOpsGpioBankIO,
		.access = 0U,
		.derived = &pmResetGpio5EMIO92,
	},
	.rstMaskDataReg = GPIO_MASK_DATA_5_MSW_REG,
	.rstDirectionReg = GPIO_DIRM_5,
	.rstReadDataReg = GPIO_DATA_5_RO_REG,
	.isMaskDataLsw = false,
	.rstLine = GPIO5_EMIO92_MSW_DATA_BIT,
};

static PmResetGpioBankIOs pmResetGpio5EMIO93 = {
	.rst = {
		.ops = &pmResetOpsGpioBankIO,
		.access = 0U,
		.derived = &pmResetGpio5EMIO93,
	},
	.rstMaskDataReg = GPIO_MASK_DATA_5_MSW_REG,
	.rstDirectionReg = GPIO_DIRM_5,
	.rstReadDataReg = GPIO_DATA_5_RO_REG,
	.isMaskDataLsw = false,
	.rstLine = GPIO5_EMIO93_MSW_DATA_BIT,
};

static PmResetGpioBankIOs pmResetGpio5EMIO94 = {
	.rst = {
		.ops = &pmResetOpsGpioBankIO,
		.access = 0U,
		.derived = &pmResetGpio5EMIO94,
	},
	.rstMaskDataReg = GPIO_MASK_DATA_5_MSW_REG,
	.rstDirectionReg = GPIO_DIRM_5,
	.rstReadDataReg = GPIO_DATA_5_RO_REG,
	.isMaskDataLsw = false,
	.rstLine = GPIO5_EMIO94_MSW_DATA_BIT,
};

static PmResetGpioBankIOs pmResetGpio5EMIO95 = {
	.rst = {
		.ops = &pmResetOpsGpioBankIO,
		.access = 0U,
		.derived = &pmResetGpio5EMIO95,
	},
	.rstMaskDataReg = GPIO_MASK_DATA_5_MSW_REG,
	.rstDirectionReg = GPIO_DIRM_5,
	.rstReadDataReg = GPIO_DATA_5_RO_REG,
	.isMaskDataLsw = false,
	.rstLine = GPIO5_EMIO95_MSW_DATA_BIT,
};

/**
 * PmResetPulsePsOnly() - Gracefully reset PS while PL remains active
 *
 * @return	Operation status
 */
static u32 PmResetPulsePsOnly(void)
{
	XPfw_ResetPsOnly();
	return XST_SUCCESS;
}

static PmResetRom pmResetPsOnly = {
	.rst = {
		.ops = &pmResetOpsNoAssert,
		.access = 0U,
		.derived = &pmResetPsOnly,
	},
	.ctrlAddr = PMU_GLOBAL_GLOBAL_RESET,
	.mask = PMU_GLOBAL_GLOBAL_RESET_PS_ONLY_RST_MASK,
	.pulseRom = &PmResetPulsePsOnly,
};

/**
 * PmResetPulseRpuLs() - Gracefully cycle the lock-step RPU reset
 *
 * @return	Operation status
 */
static u32 PmResetPulseRpuLs(void)
{
	return (u32)XPfw_ResetRpu();
}

static PmResetRom pmResetRpuLs = {
	.rst = {
		.ops = &pmResetOpsNoAssert,
		.access = 0U,
		.derived = &pmResetRpuLs,
	},
	.ctrlAddr = PMU_GLOBAL_GLOBAL_RESET,
	.mask = PMU_GLOBAL_GLOBAL_RESET_RPU_LS_RST_MASK,
	.pulseRom = PmResetPulseRpuLs,
};

static PmReset pmResetPl = {
	.ops = &pmResetOpsPl,
	.access = 0U,
	.derived = &pmResetPl,
};

static PmReset* const pmAllResets[PM_RESET_MAX_LINE] = {
	[PM_RESET_PCIE_CFG - PM_RESET_BASE] = &pmResetPcieCfg.rst,
	[PM_RESET_PCIE_BRIDGE - PM_RESET_BASE] = &pmResetPcieBridge.rst,
	[PM_RESET_PCIE_CTRL - PM_RESET_BASE] = &pmResetPcieCtrl.rst,
	[PM_RESET_DP - PM_RESET_BASE] = &pmResetDp.rst,
	[PM_RESET_SWDT_CRF - PM_RESET_BASE] = &pmResetSwdtCrf.rst,
	[PM_RESET_AFI_FM5 - PM_RESET_BASE] = &pmResetAfiFm5.rst,
	[PM_RESET_AFI_FM4 - PM_RESET_BASE] = &pmResetAfiFm4.rst,
	[PM_RESET_AFI_FM3 - PM_RESET_BASE] = &pmResetAfiFm3.rst,
	[PM_RESET_AFI_FM2 - PM_RESET_BASE] = &pmResetAfiFm2.rst,
	[PM_RESET_AFI_FM1 - PM_RESET_BASE] = &pmResetAfiFm1.rst,
	[PM_RESET_AFI_FM0 - PM_RESET_BASE] = &pmResetAfiFm0.rst,
	[PM_RESET_GDMA - PM_RESET_BASE] = &pmResetGdma.rst,
	[PM_RESET_GPU_PP1 - PM_RESET_BASE] = &pmResetGpuPp1.rst,
	[PM_RESET_GPU_PP0 - PM_RESET_BASE] = &pmResetGpuPp0.rst,
	[PM_RESET_GPU - PM_RESET_BASE] = &pmResetGpu.rst,
	[PM_RESET_GT - PM_RESET_BASE] = &pmResetGt.rst,
	[PM_RESET_SATA - PM_RESET_BASE] = &pmResetSata.rst,
	[PM_RESET_ACPU3_PWRON - PM_RESET_BASE] = &pmResetAcpu3Pwron.rst,
	[PM_RESET_ACPU2_PWRON - PM_RESET_BASE] = &pmResetAcpu2Pwron.rst,
	[PM_RESET_ACPU1_PWRON - PM_RESET_BASE] = &pmResetAcpu1Pwron.rst,
	[PM_RESET_ACPU0_PWRON - PM_RESET_BASE] = &pmResetAcpu0Pwron.rst,
	[PM_RESET_APU_L2 - PM_RESET_BASE] = &pmResetApuL2.rst,
	[PM_RESET_ACPU3 - PM_RESET_BASE] = &pmResetAcpu3.rst,
	[PM_RESET_ACPU2 - PM_RESET_BASE] = &pmResetAcpu2.rst,
	[PM_RESET_ACPU1 - PM_RESET_BASE] = &pmResetAcpu1.rst,
	[PM_RESET_ACPU0 - PM_RESET_BASE] = &pmResetAcpu0.rst,
	[PM_RESET_DDR - PM_RESET_BASE] = &pmResetDDR.rst,
	[PM_RESET_APM_FPD - PM_RESET_BASE] = &pmResetApmFpd.rst,
	[PM_RESET_SOFT - PM_RESET_BASE] = &pmResetSoft.rst,
	[PM_RESET_GEM0 - PM_RESET_BASE] = &pmResetGem0.rst,
	[PM_RESET_GEM1 - PM_RESET_BASE] = &pmResetGem1.rst,
	[PM_RESET_GEM2 - PM_RESET_BASE] = &pmResetGem2.rst,
	[PM_RESET_GEM3 - PM_RESET_BASE] = &pmResetGem3.rst,
	[PM_RESET_QSPI - PM_RESET_BASE] = &pmResetQspi.rst,
	[PM_RESET_UART0 - PM_RESET_BASE] = &pmResetUart0.rst,
	[PM_RESET_UART1 - PM_RESET_BASE] = &pmResetUart1.rst,
	[PM_RESET_SPI0 - PM_RESET_BASE] = &pmResetSpi0.rst,
	[PM_RESET_SPI1 - PM_RESET_BASE] = &pmResetSpi1.rst,
	[PM_RESET_SDIO0 - PM_RESET_BASE] = &pmResetSdio0.rst,
	[PM_RESET_SDIO1 - PM_RESET_BASE] = &pmResetSdio1.rst,
	[PM_RESET_CAN0 - PM_RESET_BASE] = &pmResetCan0.rst,
	[PM_RESET_CAN1 - PM_RESET_BASE] = &pmResetCan1.rst,
	[PM_RESET_I2C0 - PM_RESET_BASE] = &pmResetI2C0.rst,
	[PM_RESET_I2C1 - PM_RESET_BASE] = &pmResetI2C1.rst,
	[PM_RESET_TTC0 - PM_RESET_BASE] = &pmResetTtc0.rst,
	[PM_RESET_TTC1 - PM_RESET_BASE] = &pmResetTtc1.rst,
	[PM_RESET_TTC2 - PM_RESET_BASE] = &pmResetTtc2.rst,
	[PM_RESET_TTC3 - PM_RESET_BASE] = &pmResetTtc3.rst,
	[PM_RESET_SWDT_CRL - PM_RESET_BASE] = &pmResetSwdtCrl.rst,
	[PM_RESET_NAND - PM_RESET_BASE] = &pmResetNand.rst,
	[PM_RESET_ADMA - PM_RESET_BASE] = &pmResetAdma.rst,
	[PM_RESET_GPIO - PM_RESET_BASE] = &pmResetGpio.rst,
	[PM_RESET_IOU_CC - PM_RESET_BASE] = &pmResetIouCc.rst,
	[PM_RESET_TIMESTAMP - PM_RESET_BASE] = &pmResetTimestamp.rst,
	[PM_RESET_RPU_R50 - PM_RESET_BASE] = &pmResetRpuR50.rst,
	[PM_RESET_RPU_R51 - PM_RESET_BASE] = &pmResetRpuR51.rst,
	[PM_RESET_RPU_AMBA - PM_RESET_BASE] = &pmResetRpuAmba.rst,
	[PM_RESET_OCM - PM_RESET_BASE] = &pmResetOcm.rst,
	[PM_RESET_RPU_PGE - PM_RESET_BASE] = &pmResetRpuPge.rst,
	[PM_RESET_USB0_CORERESET - PM_RESET_BASE] = &pmResetUsb0Core.rst,
	[PM_RESET_USB1_CORERESET - PM_RESET_BASE] = &pmResetUsb1Core.rst,
	[PM_RESET_USB0_HIBERRESET - PM_RESET_BASE] = &pmResetUsb0Hiber.rst,
	[PM_RESET_USB1_HIBERRESET - PM_RESET_BASE] = &pmResetUsb1Hiber.rst,
	[PM_RESET_USB0_APB - PM_RESET_BASE] = &pmResetUsb0Apb.rst,
	[PM_RESET_USB1_APB - PM_RESET_BASE] = &pmResetUsb1Apb.rst,
	[PM_RESET_IPI - PM_RESET_BASE] = &pmResetIpi.rst,
	[PM_RESET_APM_LPD - PM_RESET_BASE] = &pmResetApmLpd.rst,
	[PM_RESET_RTC - PM_RESET_BASE] = &pmResetRtc.rst,
	[PM_RESET_SYSMON - PM_RESET_BASE] = &pmResetSysmon.rst,
	[PM_RESET_AFI_FM6 - PM_RESET_BASE] = &pmResetAfiFm6.rst,
	[PM_RESET_LPD_SWDT - PM_RESET_BASE] = &pmResetLpdSwdt.rst,
	[PM_RESET_FPD - PM_RESET_BASE] = &pmResetFpd.rst,
	[PM_RESET_RPU_DBG1 - PM_RESET_BASE] = &pmResetRpuDbg1.rst,
	[PM_RESET_RPU_DBG0 - PM_RESET_BASE] = &pmResetRpuDbg0.rst,
	[PM_RESET_DBG_LPD - PM_RESET_BASE] = &pmResetDbgLpd.rst,
	[PM_RESET_DBG_FPD - PM_RESET_BASE] = &pmResetDbgFpd.rst,
	[PM_RESET_APLL - PM_RESET_BASE] = &pmResetApll.rst,
	[PM_RESET_DPLL - PM_RESET_BASE] = &pmResetDpll.rst,
	[PM_RESET_VPLL - PM_RESET_BASE] = &pmResetVpll.rst,
	[PM_RESET_IOPLL - PM_RESET_BASE] = &pmResetIopll.rst,
	[PM_RESET_RPLL - PM_RESET_BASE] = &pmResetRpll.rst,
	[PM_RESET_GPO3_PL_0 - PM_RESET_BASE] = &pmResetGpo3Pl0.rst,
	[PM_RESET_GPO3_PL_1 - PM_RESET_BASE] = &pmResetGpo3Pl1.rst,
	[PM_RESET_GPO3_PL_2 - PM_RESET_BASE] = &pmResetGpo3Pl2.rst,
	[PM_RESET_GPO3_PL_3 - PM_RESET_BASE] = &pmResetGpo3Pl3.rst,
	[PM_RESET_GPO3_PL_4 - PM_RESET_BASE] = &pmResetGpo3Pl4.rst,
	[PM_RESET_GPO3_PL_5 - PM_RESET_BASE] = &pmResetGpo3Pl5.rst,
	[PM_RESET_GPO3_PL_6 - PM_RESET_BASE] = &pmResetGpo3Pl6.rst,
	[PM_RESET_GPO3_PL_7 - PM_RESET_BASE] = &pmResetGpo3Pl7.rst,
	[PM_RESET_GPO3_PL_8 - PM_RESET_BASE] = &pmResetGpo3Pl8.rst,
	[PM_RESET_GPO3_PL_9 - PM_RESET_BASE] = &pmResetGpo3Pl9.rst,
	[PM_RESET_GPO3_PL_10 - PM_RESET_BASE] = &pmResetGpo3Pl10.rst,
	[PM_RESET_GPO3_PL_11 - PM_RESET_BASE] = &pmResetGpo3Pl11.rst,
	[PM_RESET_GPO3_PL_12 - PM_RESET_BASE] = &pmResetGpo3Pl12.rst,
	[PM_RESET_GPO3_PL_13 - PM_RESET_BASE] = &pmResetGpo3Pl13.rst,
	[PM_RESET_GPO3_PL_14 - PM_RESET_BASE] = &pmResetGpo3Pl14.rst,
	[PM_RESET_GPO3_PL_15 - PM_RESET_BASE] = &pmResetGpo3Pl15.rst,
	[PM_RESET_GPO3_PL_16 - PM_RESET_BASE] = &pmResetGpo3Pl16.rst,
	[PM_RESET_GPO3_PL_17 - PM_RESET_BASE] = &pmResetGpo3Pl17.rst,
	[PM_RESET_GPO3_PL_18 - PM_RESET_BASE] = &pmResetGpo3Pl18.rst,
	[PM_RESET_GPO3_PL_19 - PM_RESET_BASE] = &pmResetGpo3Pl19.rst,
	[PM_RESET_GPO3_PL_20 - PM_RESET_BASE] = &pmResetGpo3Pl20.rst,
	[PM_RESET_GPO3_PL_21 - PM_RESET_BASE] = &pmResetGpo3Pl21.rst,
	[PM_RESET_GPO3_PL_22 - PM_RESET_BASE] = &pmResetGpo3Pl22.rst,
	[PM_RESET_GPO3_PL_23 - PM_RESET_BASE] = &pmResetGpo3Pl23.rst,
	[PM_RESET_GPO3_PL_24 - PM_RESET_BASE] = &pmResetGpo3Pl24.rst,
	[PM_RESET_GPO3_PL_25 - PM_RESET_BASE] = &pmResetGpo3Pl25.rst,
	[PM_RESET_GPO3_PL_26 - PM_RESET_BASE] = &pmResetGpo3Pl26.rst,
	[PM_RESET_GPO3_PL_27 - PM_RESET_BASE] = &pmResetGpo3Pl27.rst,
	[PM_RESET_GPO3_PL_28 - PM_RESET_BASE] = &pmResetGpo3Pl28.rst,
	[PM_RESET_GPO3_PL_29 - PM_RESET_BASE] = &pmResetGpo3Pl29.rst,
	[PM_RESET_GPO3_PL_30 - PM_RESET_BASE] = &pmResetGpo3Pl30.rst,
	[PM_RESET_GPO3_PL_31 - PM_RESET_BASE] = &pmResetGpo3Pl31.rst,
	[PM_RESET_RPU_LS - PM_RESET_BASE] = &pmResetRpuLs.rst,
	[PM_RESET_PS_ONLY - PM_RESET_BASE] = &pmResetPsOnly.rst,
	[PM_RESET_PL - PM_RESET_BASE] = &pmResetPl,
	[PM_RESET_GPIO5_EMIO_92 - PM_RESET_BASE] = &pmResetGpio5EMIO92.rst,
	[PM_RESET_GPIO5_EMIO_93 - PM_RESET_BASE] = &pmResetGpio5EMIO93.rst,
	[PM_RESET_GPIO5_EMIO_94 - PM_RESET_BASE] = &pmResetGpio5EMIO94.rst,
	[PM_RESET_GPIO5_EMIO_95 - PM_RESET_BASE] = &pmResetGpio5EMIO95.rst,
};

/**
 * PmGetResetById() - Find reset that matches a given reset ID
 * @resetId      ID of the reset to find
 *
 * @return       Pointer to PmReset structure (or NULL if not found)
 */
PmReset* PmGetResetById(const u32 resetId)
{
	PmReset* resetPtr = NULL;

	if (resetId >= (ARRAY_SIZE(pmAllResets) + PM_RESET_BASE)) {
		/* Reset id is higher than maximum */
		goto done;
	}

	if (resetId < PM_RESET_BASE) {
		/* Reset id is smaller than minimum */
		goto done;
	}

	resetPtr = pmAllResets[resetId - PM_RESET_BASE];

done:
	return resetPtr;
}

s32 PmResetDoAssert(const PmReset *reset, u32 action)
{
	s32 status = XST_SUCCESS;

	switch (action) {
	case PM_RESET_ACTION_RELEASE:
	case PM_RESET_ACTION_ASSERT:
		if (NULL != reset->ops->resetAssert) {
			reset->ops->resetAssert(reset, action);
		} else {
			status = XST_INVALID_PARAM;
		}
		break;
	case PM_RESET_ACTION_PULSE:
		status = (s32)reset->ops->pulse(reset);
		break;
	default:
		PmWarn("invalid assert %lu\r\n", action);
		status = XST_INVALID_PARAM;
		break;
	};

	return status;
}

/**
 * PmResetAssertInt() - Configure reset line
 * @reset       ID of reset to be configured
 * @action      Specifies the action (assert, release, pulse)
 */
s32 PmResetAssertInt(u32 reset, u32 action)
{
	s32 status;
	const PmReset *resetPtr = PmGetResetById(reset);

	if (NULL == resetPtr) {
		PmWarn("Invalid reset %lu\r\n", reset);
		status = XST_INVALID_PARAM;
		goto err;
	}

	status = PmResetDoAssert(resetPtr, action);

err:
	return status;
}

inline u32 PmResetGetStatusInt(const PmReset* const resetPtr, u32 *status)
{
	s32 ret = XST_NO_FEATURE;

	if (NULL != resetPtr->ops->getStatus) {
		*status = resetPtr->ops->getStatus(resetPtr);
		ret = XST_SUCCESS;
	}

	return (u32)ret;
}

/**
 * PmResetSetConfig() - Set configuration for reset control
 * @resetId     ID of the reset whose permissions should be set
 * @permissions Permissions to set (ORed IPI masks of permissible masters)
 *
 * @return      XST_INVALID_PARAM if reset with given ID is not found,
 *              XST_SUCCESS if permissions are set
 */
s32 PmResetSetConfig(const u32 resetId, const u32 permissions)
{
	s32 status = XST_SUCCESS;
	PmReset* rst = PmGetResetById(resetId);

	if (NULL == rst) {
		status = XST_INVALID_PARAM;
		goto done;
	}

	rst->access = permissions;

done:
	return status;
}

/**
 * PmResetClearConfig() - Clear configuration for all resets
 */
void PmResetClearConfig(void)
{
	u32 i;

	for (i = 0U; i < ARRAY_SIZE(pmAllResets); i++) {
		pmAllResets[i]->access = 0U;
	}
}

#endif
