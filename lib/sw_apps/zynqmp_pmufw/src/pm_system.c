/*
* Copyright (c) 2014 - 2021 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
 */

#include "xpfw_config.h"
#ifdef ENABLE_PM

/*********************************************************************
 * Contains system-level PM functions and state
 ********************************************************************/

#include "pm_system.h"
#include "pm_common.h"
#include "crl_apb.h"
#include "pm_callbacks.h"
#include "xpfw_resets.h"
#include "pm_requirement.h"
#include "pm_sram.h"
#include "pm_ddr.h"
#include "pm_periph.h"
#include "pm_hooks.h"
#include "pm_extern.h"
#include "pm_qspi.h"
#include "pm_clock.h"
#include "pm_pll.h"
#include "pm_power.h"
#include "rpu.h"
#include "xpfw_default.h"

#define OCM_START_ADDR		0xFFFC0000U
#define OCM_END_ADDR		0xFFFFFFFFU
#define DDR_STORE_BASE		0x07000000U
#define TCM_0A_START_ADDR	0xFFE00000U
#define TCM_0A_END_ADDR		0xFFE0FFFFU
#define TCM_0B_START_ADDR	0xFFE20000U
#define TCM_0B_END_ADDR		0xFFE2FFFFU
#define TCM_1A_START_ADDR	0xFFE90000U
#define TCM_1A_END_ADDR		0xFFE9FFFFU
#define TCM_1B_START_ADDR	0xFFEB0000U
#define TCM_1B_END_ADDR		0xFFEBFFFFU
#define TCM_SKIP_MEMORY		0x0U
#define TCM_SAVE_MEMORY		0x1U
#define TCM_BANK_OFFSET		0x80000U


/*********************************************************************
 * Structure definitions
 ********************************************************************/
/**
 * PmSystem - System level information
 * @psRestartPerms	ORed IPI masks of masters allowed to restart PS
 * @systemRestartPerms	ORed IPI masks of masters allowed to restart system
 * @suspendType		Type of system suspend: Regular/Power Off Suspend
 */
typedef struct {
	u32 psRestartPerms;
	u32 systemRestartPerms;
	u32 suspendType;
} PmSystem;

/**
 * PmSystemRequirement - System level requirements (not assigned to any master)
 * @slave	Slave for which the requirements are set
 * @caps	Capabilities of the slave that are required by the system
 * @posCaps	Capabilities of the slave that are required for entering Power
 * 		Off Suspend state
 */
typedef struct PmSystemRequirement {
	PmSlave* const slave;
	u8 caps;
	u8 posCaps;
} PmSystemRequirement;

#ifdef ENABLE_POS
/**
 * PmTcmMemorySection - TCM memory that may be saved/restored
 * @section             Memory region start and end address
 * @save                Flag to mark that region needs to be saved/restored
 */
typedef struct PmTcmMemorySection {
	PmMemorySection section;
	u32 save;
} PmTcmMemorySection;
#endif

/*********************************************************************
 * Data initialization
 ********************************************************************/

static PmSystem pmSystem = {
	.psRestartPerms = 0U,
	.systemRestartPerms = 0U,
	.suspendType = PM_SUSPEND_TYPE_REGULAR,
};

/*
 * These requirements are needed for the system to operate:
 * - OCM bank(s) store FSBL which is needed to restart APU, and should never be
 *   powered down unless the whole system goes down.
 */
static PmSystemRequirement pmSystemReqs[] = {
	{
		.slave = &pmSlaveOcm0_g.slv,
		.caps = PM_CAP_CONTEXT,
		.posCaps = PM_CAP_ACCESS,
	}, {
		.slave = &pmSlaveOcm1_g.slv,
		.caps = PM_CAP_CONTEXT,
		.posCaps = PM_CAP_ACCESS,
	}, {
		.slave = &pmSlaveOcm2_g.slv,
		.caps = PM_CAP_CONTEXT,
		.posCaps = PM_CAP_ACCESS,
	},{
		.slave = &pmSlaveOcm3_g.slv,
		.caps = PM_CAP_CONTEXT,
		.posCaps = PM_CAP_ACCESS,
	},{
		.slave = &pmSlaveDdr_g,
		.caps = PM_CAP_CONTEXT,
		.posCaps = PM_CAP_ACCESS,
	},{
		.slave = &pmSlavePl_g,
		.caps = 0U,
		.posCaps = 0U,
	},
#ifdef DEBUG_MODE
#if (STDOUT_BASEADDRESS == XPMU_UART_0_BASEADDR)
	{
		.slave = &pmSlaveUart0_g,
		.caps = PM_CAP_ACCESS,
		.posCaps = PM_CAP_ACCESS,
	},
#endif
#if (STDOUT_BASEADDRESS == XPMU_UART_1_BASEADDR)
	{
		.slave = &pmSlaveUart1_g,
		.caps = PM_CAP_ACCESS,
		.posCaps = PM_CAP_ACCESS,
	},
#endif
#endif
#ifdef ENABLE_POS
	/* TCM requirements used when TCM context should be saved/restored */
	{
		.slave = &pmSlaveTcm0A_g.sram.slv,
		.caps = 0U,
		.posCaps = 0U,
	}, {
		.slave = &pmSlaveTcm0B_g.sram.slv,
		.caps = 0U,
		.posCaps = 0U,
	},{
		.slave = &pmSlaveTcm1A_g.sram.slv,
		.caps = 0U,
		.posCaps = 0U,
	},{
		.slave = &pmSlaveTcm1B_g.sram.slv,
		.caps = 0U,
		.posCaps = 0U,
	},
#endif
};

#ifdef ENABLE_POS
extern u8 __data_start;
extern u8 __data_end;
extern u8 __bss_start;
extern u8 __bss_end;

/* Memory sections that needs to be saved/restored during Power Off Suspend */
PmMemorySection pmSystemMemory[] = {
	/* PMU data and bss section */
	{
		.startAddr = (u32)&__data_start,
		.endAddr = (u32)&__data_end - 1U,
	}, {
		.startAddr = (u32)&__bss_start,
		.endAddr = (u32)&__bss_end - 1U,
	},
	/* OCM data */
	{
		.startAddr = OCM_START_ADDR,
		.endAddr = OCM_END_ADDR,
	},
};

/* TCM memory sections that may be saved/restored during Power Off Suspend */
PmTcmMemorySection pmSystemTcmMemory[] = {
	{
		{
			.startAddr = TCM_0A_START_ADDR,
			.endAddr = TCM_0A_END_ADDR,
		},
		.save = TCM_SKIP_MEMORY,
	}, {
		{
			.startAddr = TCM_0B_START_ADDR,
			.endAddr = TCM_0B_END_ADDR,
		},
		.save = TCM_SKIP_MEMORY,
	}, {
		{
			.startAddr = TCM_1A_START_ADDR,
			.endAddr = TCM_1A_END_ADDR,
		},
		.save = TCM_SKIP_MEMORY,
	}, {
		{
			.startAddr = TCM_1B_START_ADDR,
			.endAddr = TCM_1B_END_ADDR,
		},
		.save = TCM_SKIP_MEMORY,
	},
};

/* TCM slaves that may need context save/restore during Power Off Suspend */
PmSlaveTcm* pmSystemTcmSlaves [] = {
	&pmSlaveTcm0A_g,
	&pmSlaveTcm0B_g,
	&pmSlaveTcm1A_g,
	&pmSlaveTcm1B_g,
};

/* System registers that needs to be saved/restored during Power Off Suspend */
static PmRegisterContext pmSystemRegs[] = {
	/* IOU_SLCR */
	{ .addr = 0XFF180000U, },
	{ .addr = 0XFF180004U, },
	{ .addr = 0XFF180008U, },
	{ .addr = 0XFF18000CU, },
	{ .addr = 0XFF180010U, },
	{ .addr = 0XFF180014U, },
	{ .addr = 0XFF180018U, },
	{ .addr = 0XFF18001CU, },
	{ .addr = 0XFF180020U, },
	{ .addr = 0XFF180024U, },
	{ .addr = 0XFF180028U, },
	{ .addr = 0XFF18002CU, },
	{ .addr = 0XFF180030U, },
	{ .addr = 0XFF180034U, },
	{ .addr = 0XFF180038U, },
	{ .addr = 0XFF18003CU, },
	{ .addr = 0XFF180040U, },
	{ .addr = 0XFF180044U, },
	{ .addr = 0XFF180048U, },
	{ .addr = 0XFF18004CU, },
	{ .addr = 0XFF180050U, },
	{ .addr = 0XFF180054U, },
	{ .addr = 0XFF180058U, },
	{ .addr = 0XFF18005CU, },
	{ .addr = 0XFF180060U, },
	{ .addr = 0XFF180064U, },
	{ .addr = 0XFF180068U, },
	{ .addr = 0XFF18006CU, },
	{ .addr = 0XFF180070U, },
	{ .addr = 0XFF180074U, },
	{ .addr = 0XFF180078U, },
	{ .addr = 0XFF18007CU, },
	{ .addr = 0XFF180080U, },
	{ .addr = 0XFF180084U, },
	{ .addr = 0XFF180088U, },
	{ .addr = 0XFF18008CU, },
	{ .addr = 0XFF180090U, },
	{ .addr = 0XFF180094U, },
	{ .addr = 0XFF180098U, },
	{ .addr = 0XFF18009CU, },
	{ .addr = 0XFF1800A0U, },
	{ .addr = 0XFF1800A4U, },
	{ .addr = 0XFF1800A8U, },
	{ .addr = 0XFF1800ACU, },
	{ .addr = 0XFF1800B0U, },
	{ .addr = 0XFF1800B4U, },
	{ .addr = 0XFF1800B8U, },
	{ .addr = 0XFF1800BCU, },
	{ .addr = 0XFF1800C0U, },
	{ .addr = 0XFF1800C4U, },
	{ .addr = 0XFF1800C8U, },
	{ .addr = 0XFF1800CCU, },
	{ .addr = 0XFF1800D0U, },
	{ .addr = 0XFF1800D4U, },
	{ .addr = 0XFF1800D8U, },
	{ .addr = 0XFF1800DCU, },
	{ .addr = 0XFF1800E0U, },
	{ .addr = 0XFF1800E4U, },
	{ .addr = 0XFF1800E8U, },
	{ .addr = 0XFF1800ECU, },
	{ .addr = 0XFF1800F0U, },
	{ .addr = 0XFF1800F4U, },
	{ .addr = 0XFF1800F8U, },
	{ .addr = 0XFF1800FCU, },
	{ .addr = 0XFF180100U, },
	{ .addr = 0XFF180104U, },
	{ .addr = 0XFF180108U, },
	{ .addr = 0XFF18010CU, },
	{ .addr = 0XFF180110U, },
	{ .addr = 0XFF180114U, },
	{ .addr = 0XFF180118U, },
	{ .addr = 0XFF18011CU, },
	{ .addr = 0XFF180120U, },
	{ .addr = 0XFF180124U, },
	{ .addr = 0XFF180128U, },
	{ .addr = 0XFF18012CU, },
	{ .addr = 0XFF180130U, },
	{ .addr = 0XFF180134U, },
	{ .addr = 0XFF180138U, },
	{ .addr = 0XFF18013CU, },
	{ .addr = 0XFF180140U, },
	{ .addr = 0XFF180144U, },
	{ .addr = 0XFF180148U, },
	{ .addr = 0XFF18014CU, },
	{ .addr = 0XFF180154U, },
	{ .addr = 0XFF180158U, },
	{ .addr = 0XFF18015CU, },
	{ .addr = 0XFF180160U, },
	{ .addr = 0XFF180164U, },
	{ .addr = 0XFF180168U, },
	{ .addr = 0XFF180170U, },
	{ .addr = 0XFF180174U, },
	{ .addr = 0XFF180178U, },
	{ .addr = 0XFF18017CU, },
	{ .addr = 0XFF180180U, },
	{ .addr = 0XFF180184U, },
	{ .addr = 0XFF180200U, },
	{ .addr = 0XFF180204U, },
	{ .addr = 0XFF180208U, },
	{ .addr = 0XFF18020CU, },
	{ .addr = 0XFF180300U, },
	{ .addr = 0XFF180304U, },
	{ .addr = 0XFF180308U, },
	{ .addr = 0XFF18030CU, },
	{ .addr = 0XFF180310U, },
	{ .addr = 0XFF180314U, },
	{ .addr = 0XFF180318U, },
	{ .addr = 0XFF18031CU, },
	{ .addr = 0XFF180320U, },
	{ .addr = 0XFF180324U, },
	{ .addr = 0XFF180328U, },
	{ .addr = 0XFF18032CU, },
	{ .addr = 0XFF180330U, },
	{ .addr = 0XFF180334U, },
	{ .addr = 0XFF180338U, },
	{ .addr = 0XFF18033CU, },
	{ .addr = 0XFF180340U, },
	{ .addr = 0XFF180344U, },
	{ .addr = 0XFF18034CU, },
	{ .addr = 0XFF180350U, },
	{ .addr = 0XFF180354U, },
	{ .addr = 0XFF180358U, },
	{ .addr = 0XFF18035CU, },
	{ .addr = 0XFF180360U, },
	{ .addr = 0XFF180380U, },
	{ .addr = 0XFF180390U, },
	{ .addr = 0XFF180400U, },
	{ .addr = 0XFF180404U, },
	{ .addr = 0XFF180408U, },
	{ .addr = 0XFF180500U, },
	{ .addr = 0XFF180504U, },
	{ .addr = 0XFF180508U, },
	{ .addr = 0XFF18050CU, },
	{ .addr = 0XFF180510U, },
	{ .addr = 0XFF180514U, },
	{ .addr = 0XFF180518U, },
	{ .addr = 0XFF18051CU, },
	{ .addr = 0XFF180520U, },
	{ .addr = 0XFF180524U, },
	/* LPD_SLCR_SECURE */
	{ .addr = 0xFF4B0024U, },
	{ .addr = 0xFF4B0034U, },
	/* CRL_APB */
	{ .addr = 0XFF5E0000U, },
	{ .addr = 0XFF5E001CU, },
	{ .addr = 0XFF5E004CU, },
	{ .addr = 0XFF5E0050U, },
	{ .addr = 0XFF5E0054U, },
	{ .addr = 0XFF5E0058U, },
	{ .addr = 0XFF5E005CU, },
	{ .addr = 0XFF5E0060U, },
	{ .addr = 0XFF5E0064U, },
	{ .addr = 0XFF5E0068U, },
	{ .addr = 0XFF5E006CU, },
	{ .addr = 0XFF5E0070U, },
	{ .addr = 0XFF5E0074U, },
	{ .addr = 0XFF5E0078U, },
	{ .addr = 0XFF5E007CU, },
	{ .addr = 0XFF5E0080U, },
	{ .addr = 0XFF5E0084U, },
	{ .addr = 0XFF5E0088U, },
	{ .addr = 0XFF5E0090U, },
	{ .addr = 0XFF5E009CU, },
	{ .addr = 0XFF5E00A0U, },
	{ .addr = 0XFF5E00A4U, },
	{ .addr = 0XFF5E00A8U, },
	{ .addr = 0XFF5E00ACU, },
	{ .addr = 0XFF5E00B0U, },
	{ .addr = 0XFF5E00B4U, },
	{ .addr = 0XFF5E00B8U, },
	{ .addr = 0XFF5E00C0U, },
	{ .addr = 0XFF5E00C4U, },
	{ .addr = 0XFF5E00C8U, },
	{ .addr = 0XFF5E00CCU, },
	{ .addr = 0XFF5E00D0U, },
	{ .addr = 0XFF5E00D4U, },
	{ .addr = 0XFF5E00D8U, },
	{ .addr = 0XFF5E00DCU, },
	{ .addr = 0XFF5E00E0U, },
	{ .addr = 0XFF5E00E4U, },
	{ .addr = 0XFF5E00E8U, },
	{ .addr = 0XFF5E00FCU, },
	{ .addr = 0XFF5E0100U, },
	{ .addr = 0XFF5E0104U, },
	{ .addr = 0XFF5E0108U, },
	{ .addr = 0XFF5E0120U, },
	{ .addr = 0XFF5E0124U, },
	{ .addr = 0XFF5E0128U, },
	{ .addr = 0XFF5E0230U, },
	{ .addr = 0XFF5E0234U, },
	{ .addr = 0XFF5E0238U, },
	{ .addr = 0XFF5E023CU, },
	{ .addr = 0XFF5E0240U, },
	/* IOU_SCNTRS */
	{ .addr = 0XFF260020U, },
	{ .addr = 0XFF260008U, },
	{ .addr = 0XFF26000CU, },
	{ .addr = 0XFF260000U, },
	/* RPU */
	{ .addr = 0XFF9A0000U, },
	{ .addr = 0XFF9A0108U, },
	{ .addr = 0XFF9A0208U, },
#ifdef DEBUG_MODE
#if (STDOUT_BASEADDRESS == XPMU_UART_0_BASEADDR)
	{ .addr = 0XFF000034U, },
	{ .addr = 0XFF000018U, },
	{ .addr = 0XFF000000U, },
	{ .addr = 0XFF000004U, }
#endif
#if (STDOUT_BASEADDRESS == XPMU_UART_1_BASEADDR)
	{ .addr = 0XFF010034U, },
	{ .addr = 0XFF010018U, },
	{ .addr = 0XFF010000U, },
	{ .addr = 0XFF010004U, }
#endif
#endif
};
#endif

/*********************************************************************
 * Function definitions
 ********************************************************************/

#ifdef ENABLE_POS
/**
 * PmSystemPosDdrRequirementAdd() - Add DDR context saving requirements
 * @return	XST_SUCCESS if requirements are added, XST_FAILURE otherwise
 */
static s32 PmSystemPosDdrRequirementAdd(void)
{
	s32 status = XST_SUCCESS;
	u32 i;

	for (i = 0U; i < ARRAY_SIZE(pmPosDdrReqs_g); i++) {
		PmRequirement* req;

		req = PmRequirementGetNoMaster(pmPosDdrReqs_g[i].slave);
		if (req == NULL) {
			req = PmRequirementAdd(NULL, pmPosDdrReqs_g[i].slave);
			if (NULL == req) {
				status = XST_FAILURE;
				goto done;
			}

			status = PmCheckCapabilities(req->slave,
						pmPosDdrReqs_g[i].caps);
			if (XST_SUCCESS != status) {
				status = XST_FAILURE;
				goto done;
			}
			req->info |= PM_SYSTEM_USING_SLAVE_MASK;
			req->preReq = 0U;
			req->currReq = 0U;
			req->nextReq = 0U;
			req->defaultReq = 0U;
			req->latencyReq = 0U;
		}
	}

done:
	return status;
}
#endif

/**
 * PmSystemRequirementAdd() - Add requirements of the system
 * @return	XST_SUCCESS if requirements are added, XST_FAILURE otherwise
 */
s32 PmSystemRequirementAdd(void)
{
	s32 status = XST_FAILURE;
	u32 i;

	for (i = 0U; i < ARRAY_SIZE(pmSystemReqs); i++) {
		PmRequirement* req;

		req = PmRequirementAdd(NULL, pmSystemReqs[i].slave);
		if (NULL == req) {
			status = XST_FAILURE;
			goto done;
		}

		status = PmCheckCapabilities(req->slave, pmSystemReqs[i].caps);
		if (XST_SUCCESS != status) {
			status = XST_FAILURE;
			goto done;
		}
		/**
		 * Do not assign system using slave flag for PL as it required
		 * only during subsystem restart to skip power down during
		 * subsystem restart
		 */
		if ((&pmSlavePl_g != pmSystemReqs[i].slave) ||
		    (0U != pmSystemReqs[i].caps)) {
			req->info |= PM_SYSTEM_USING_SLAVE_MASK;
		}
		req->preReq = (u8)pmSystemReqs[i].caps;
		req->currReq = (u8)pmSystemReqs[i].caps;
		req->nextReq = (u8)pmSystemReqs[i].caps;
		req->defaultReq = (u8)pmSystemReqs[i].caps;
		req->latencyReq = MAX_LATENCY;
	}

#ifdef ENABLE_POS
	/* Add DDR context saving requirements */
	status = PmSystemPosDdrRequirementAdd();
#endif

done:
	return status;
}

/**
 * PmSystemGetRequirement() - Get system-level requirement for given slave
 * @slave	Slave in question
 *
 * @return	Capabilities of the slave the are required for the system
 */
static u32 PmSystemGetRequirement(const PmSlave* const slave)
{
	u32 i;
	u32 caps = 0U;

	for (i = 0U; i < ARRAY_SIZE(pmSystemReqs); i++) {
		if (slave == pmSystemReqs[i].slave) {
			caps = pmSystemReqs[i].caps;
			break;
		}
	}

	return caps;
}

/**
 * PmSystemPrepareForRestart() - Prepare for restarting the master
 * @master	Master to be restarted
 */
void PmSystemPrepareForRestart(const PmMaster* const master)
{
	PmRequirement* req;

	/* Change system requirement for DDR to hold it ON while restarting */
	req = PmRequirementGetNoMaster(&pmSlaveDdr_g);
	if ((NULL != req) && (0U == (PM_CAP_ACCESS & req->currReq))) {
		req->currReq |= PM_CAP_ACCESS;
	}

	/* Change system requirement for PL to hold it ON while restarting */
	req = PmRequirementGetNoMaster(&pmSlavePl_g);
	if ((NULL != req) && (0U == (PM_CAP_ACCESS & req->currReq))) {
		req->info |= PM_SYSTEM_USING_SLAVE_MASK;
		req->currReq |= PM_CAP_ACCESS;
	}

	return;
}

/**
 * PmSystemRestartDone() - Done restarting the master
 * @master	Master which is restarted
 */
void PmSystemRestartDone(const PmMaster* const master)
{
	PmRequirement* req;
	u32 caps;

	/* Change system requirement for DDR to hold it ON while restarting */
	req = PmRequirementGetNoMaster(&pmSlaveDdr_g);
	caps = PmSystemGetRequirement(&pmSlaveDdr_g);
	if ((NULL != req) && (0U == (PM_CAP_ACCESS & caps))) {
		req->currReq &= ~(u8)PM_CAP_ACCESS;
	}

	/* Clear system requirement for PL once restart is done*/
	req = PmRequirementGetNoMaster(&pmSlavePl_g);
	caps = PmSystemGetRequirement(&pmSlavePl_g);
	if ((NULL != req) && (0U == (PM_CAP_ACCESS & caps))) {
		req->info &= ~(u8)PM_SYSTEM_USING_SLAVE_MASK;
		req->currReq &= ~(u8)PM_CAP_ACCESS;
	}

	return;
}

#ifdef ENABLE_POS
/**
 * PmSystemSaveContext() - Save system context before entering Power Off Suspend
 */
static void PmSystemSaveContext(void)
{
	u32 i, start, size;
	u32 address = DDR_STORE_BASE;

	/* Save FPD context */
	PmFpdSaveContext();

	/* Save system registers */
	for (i = 0U; i < ARRAY_SIZE(pmSystemRegs); i++) {
		pmSystemRegs[i].value = XPfw_Read32(pmSystemRegs[i].addr);
	}

	/* Save system data regions */
	for (i = 0U; i < ARRAY_SIZE(pmSystemMemory); i++) {
		start = pmSystemMemory[i].startAddr;
		size = pmSystemMemory[i].endAddr - start + 1U;
		(void)memcpy((void*)address, (void*)start, size);
		address += size;
		if ((address % 4U) != 0U) {
			address += 4U - (address % 4U);
		}
	}

	/* Save TCM data regions */
	for (i = 0U; i < ARRAY_SIZE(pmSystemTcmMemory); i++) {
		if (TCM_SAVE_MEMORY == pmSystemTcmMemory[i].save) {
			u32 reg;
			start = pmSystemTcmMemory[i].section.startAddr;
			size = pmSystemTcmMemory[i].section.endAddr - start + 1U;
			/* Check TCM configuration */
			reg = XPfw_Read32(RPU_RPU_GLBL_CNTL);
			reg &= RPU_RPU_GLBL_CNTL_TCM_COMB_MASK;
			if ((reg != 0U) && (start >= TCM_1A_START_ADDR)) {
				start -= TCM_BANK_OFFSET;
			}
			(void)memcpy((void*)address, (void*)start, size);
			address += size;
		}
	}
}

/**
 * PmSystemPosHaltRpu() - Halt RPU0 and RPU1 cores in order to access TCMs
 */
static void PmSystemPosHaltRpu(void)
{
	/* Halt RPU0 */
	XPfw_RMW32(CRL_APB_RST_LPD_TOP,CRL_APB_RST_LPD_TOP_RPU_R50_RESET_MASK,
		   CRL_APB_RST_LPD_TOP_RPU_R50_RESET_MASK);
	XPfw_RMW32(RPU_RPU_0_CFG, RPU_RPU_0_CFG_NCPUHALT_MASK,
		  ~RPU_RPU_0_CFG_NCPUHALT_MASK);
	XPfw_RMW32(CRL_APB_RST_LPD_TOP, CRL_APB_RST_LPD_TOP_RPU_R50_RESET_MASK,
		  ~CRL_APB_RST_LPD_TOP_RPU_R50_RESET_MASK);

	/* Halt RPU1 */
	XPfw_RMW32(CRL_APB_RST_LPD_TOP, CRL_APB_RST_LPD_TOP_RPU_R51_RESET_MASK,
		   CRL_APB_RST_LPD_TOP_RPU_R51_RESET_MASK);
	XPfw_RMW32(RPU_RPU_1_CFG, RPU_RPU_1_CFG_NCPUHALT_MASK,
		  ~RPU_RPU_1_CFG_NCPUHALT_MASK);
	XPfw_RMW32(CRL_APB_RST_LPD_TOP, CRL_APB_RST_LPD_TOP_RPU_R51_RESET_MASK,
		  ~CRL_APB_RST_LPD_TOP_RPU_R51_RESET_MASK);
}

/**
 * PmSystemRestoreContext() - Restore system context after resume from Power Off
 * 			      Suspend
 */
static void PmSystemRestoreContext(void)
{
	u32 i, start, size;
	u32 address = DDR_STORE_BASE;
	bool haltRpu = false;

	/* Restore system data regions */
	for (i = 0U; i < ARRAY_SIZE(pmSystemMemory); i++) {
		start = pmSystemMemory[i].startAddr;
		size = pmSystemMemory[i].endAddr - start + 1U;
		(void)memcpy((void*)start, (void*)address, size);
		address += size;
		if ((address % 4U) != 0U) {
			address += 4U - (address % 4U);
		}
	}

	/* Restore system registers */
	for (i = 0U; i < ARRAY_SIZE(pmSystemRegs); i++) {
		XPfw_Write32(pmSystemRegs[i].addr, pmSystemRegs[i].value);
	}

	/* Restore FPD context */
	PmFpdRestoreContext();

	/* Restore TCM data regions */
	for (i = 0U; i < ARRAY_SIZE(pmSystemTcmMemory); i++) {
		if (TCM_SAVE_MEMORY == pmSystemTcmMemory[i].save) {
			u32 reg;

			/* Halt RPU cores if TCM restoring is required */
			if (!haltRpu) {
				PmSystemPosHaltRpu();
				haltRpu = true;
			}
			start = pmSystemTcmMemory[i].section.startAddr;
			size = pmSystemTcmMemory[i].section.endAddr - start + 1U;
			/* Check TCM configuration */
			reg = XPfw_Read32(RPU_RPU_GLBL_CNTL);
			reg &= RPU_RPU_GLBL_CNTL_TCM_COMB_MASK;
			if (reg != 0U && start >= TCM_1A_START_ADDR) {
				start -= TCM_BANK_OFFSET;
			}
			memcpy((void*)start, (void*)address, size);
			address += size;
		}
	}
}

/**
 * PmSystemSetPosRequirement() - Set Power Off Susped slave requirement
 * @slave	Slave for which requirement is set
 * @caps	Capability to be set
 *
 * @return	XST_SUCCESS if slave capability is set, XST_FAILURE otherwise
 */
static s32 PmSystemSetPosRequirement(const PmSlave* const slave, u32 caps)
{
	u32 i;
	s32 status = XST_FAILURE;

	for (i = 0U; i < ARRAY_SIZE(pmSystemReqs); i++) {
		if (slave == pmSystemReqs[i].slave) {
			pmSystemReqs[i].posCaps = caps;
			status = XST_SUCCESS;
			break;
		}
	}

	return status;
}

/**
 * PmSystemTcmSetSave() - Set save flag for TCM memory region
 * @baseAddress		Base address of TCM memory region
 * @save		Save flag
 */
static void PmSystemTcmSetSave(u32 baseAddress, u32 save)
{
	u32 i;

	for (i = 0U; i < ARRAY_SIZE(pmSystemTcmMemory); i++) {
		if (baseAddress == pmSystemTcmMemory[i].section.startAddr) {
			pmSystemTcmMemory[i].save = save;
			break;
		}
	}
}

/**
 * PmSystemCheckTcm() - Mark TCM memory regions that needs to be saved/restored
 * 			during Power Off Suspend
 */
static void PmSystemCheckTcm(void)
{
	u32 i;

	for (i = 0U; i < ARRAY_SIZE(pmSystemTcmSlaves); i++) {
		PmSlave* slave = &pmSystemTcmSlaves[i]->sram.slv;
		PmRequirement* req = slave->reqs;
		bool save = false;

		/* Check if TCM slave has requirement different from 0 */
		while (NULL != req) {
			PmMaster* master = req->master;
			if (NULL != master) {
				if (PmMasterIsSuspended(master)) {
					if (0U != req->currReq) {
						save = true;
						break;
					}
				} else if (PmMasterIsSuspending(master)) {
					if (0U != req->nextReq) {
						save = true;
						break;
					}
				} else {
				}
			}
			req = req->nextMaster;
		}

		if (true == save) {
			PmSystemSetPosRequirement(slave, PM_CAP_ACCESS);
			PmSystemTcmSetSave(pmSystemTcmSlaves[i]->base,
					   TCM_SAVE_MEMORY);
		} else {
			PmSystemSetPosRequirement(slave, 0U);
			PmSystemTcmSetSave(pmSystemTcmSlaves[i]->base,
					   TCM_SKIP_MEMORY);
		}
	}
}

/**
 * PmSystemDetectPowerOffSuspend() - Detecet whether Power Off Suspend state may
 * 				     be entered
 * @master	Suspending master
 *
 * @return	True if master is last suspending in the system and Extern
 * 		device is unique wakeup source in the system, false otherwise
 */
bool PmSystemDetectPowerOffSuspend(const PmMaster* const master)
{
	bool cond = false;

	cond = PmMasterIsLastSuspending(master);
	if (false == cond) {
		goto done;
	}

	cond = PmMasterIsUniqueWakeup(&pmSlaveExternDevice_g);

done:
	return cond;
}

/**
 * PmSystemPosPrepareDdrCaps() - Set slave capabilities required for DDR
 * 				 context saving
 *
 * @return	XST_SUCCESS if capabilities are set, failure code otherwise
 */
static s32 PmSystemPosPrepareDdrCaps(void)
{
	s32 status = XST_SUCCESS;
	u32 i;

	/* Set capabilities required for saving DDR context */
	for (i = 0U; i < ARRAY_SIZE(pmPosDdrReqs_g); i++) {
		PmRequirement* req;
		u32 caps = pmPosDdrReqs_g[i].caps;

		req = PmRequirementGetNoMaster(pmPosDdrReqs_g[i].slave);
		if (NULL != req) {
			if (0U == (caps & req->currReq)) {
				status = PmRequirementUpdate(req,
						caps | req->currReq);
				if (XST_SUCCESS != status) {
					goto done;
				}
			}
		} else {
			status = XST_FAILURE;
			goto done;
		}
	}

done:
	return status;
}

/**
 * PmSystemPreparePowerOffSuspend() - Prepare system for entering Power Off
 * 				      Suspend state
 *
 * @return	XST_SUCCESS if all Power Off Suspend system slave capabilities
 * 		are set, failure code otherwise
 */
s32 PmSystemPreparePowerOffSuspend(void)
{
	s32 status = XST_SUCCESS;
	u32 i;

	pmSystem.suspendType = PM_SUSPEND_TYPE_POWER_OFF;

	/* Mark used TCM memory regions */
	PmSystemCheckTcm();

	/* Set Power Off Suspend capability for every system slave */
	for (i = 0U; i < ARRAY_SIZE(pmSystemReqs); i++) {
		PmRequirement* req;
		u32 caps = pmSystemReqs[i].posCaps;

		req = PmRequirementGetNoMaster(pmSystemReqs[i].slave);
		if (NULL != req) {
			if (caps != req->currReq) {
				status = PmRequirementUpdate(req, caps);
				if (XST_SUCCESS != status) {
					goto done;
				}
			}
		} else {
			status = XST_FAILURE;
			goto done;
		}
	}

	/* Set DDR context saving slave capabilities */
	status = PmSystemPosPrepareDdrCaps();

done:
	return status;
}

/**
 * PmSystemPosFinalizeDdrCaps() - Release slave capabilities required for DDR
 * 				  context saving
 *
 * @return	XST_SUCCESS if capabilities are released, failure code otherwise
 */
static s32 PmSystemPosFinalizeDdrCaps(void)
{
	s32 status = XST_SUCCESS;
	u32 i;

	/* Set capabilities required for DDR context saving to 0 */
	for (i = 0U; i < ARRAY_SIZE(pmPosDdrReqs_g); i++) {
		PmRequirement* req;

		req = PmRequirementGetNoMaster(pmPosDdrReqs_g[i].slave);
		if (NULL != req) {
			if (0U != req->currReq) {
				status = PmRequirementUpdate(req, 0U);
				if (XST_SUCCESS != status) {
					goto done;
				}
			}
		} else {
			status = XST_FAILURE;
			goto done;
		}
	}

done:
	return status;
}

/**
 * PmSystemFinalizePowerOffSuspend() - Finalize entering Power Off Suspend state
 *
 * @return	This function will not return in case of entering Power Off
 * 		Suspend, failure code otherwise
 */
s32 PmSystemFinalizePowerOffSuspend(void)
{
	u32 i;
	s32 status;
	PmRequirement* req;

	/* Save system context */
	PmSystemSaveContext();

	/* Release all requirements for all masters in system */
	status = PmMasterReleaseAll();
	if (XST_SUCCESS != status) {
		goto done;
	}

	/* Prepare system requirements for entering Power Off Suspend */
	for (i = 0U; i < ARRAY_SIZE(pmSystemReqs); i++) {
		u32 caps = 0U;

		/* Put DDR in retention state during Power Off Suspend */
		if (pmSystemReqs[i].slave == &pmSlaveDdr_g) {
			caps |= PM_CAP_CONTEXT;
			/* Save DDR clocks settings */
			PmClockSave(&pmSlaveDdr_g.node);
		}

		req = PmRequirementGetNoMaster(pmSystemReqs[i].slave);
		if (NULL != req) {
			if (caps != req->currReq) {
				status = PmRequirementUpdate(req, caps);
				if (XST_SUCCESS != status) {
					goto done;
				}
			}
		} else {
			status = XST_FAILURE;
			goto done;
		}
	}

	/* Save DDR context */
	status = PmHookPosSaveDdrContext();
	if (XST_SUCCESS != status) {
		goto done;
	}

	/* Release DDR context saving slave capabilities */
	status = PmSystemPosFinalizeDdrCaps();

done:
	return status;
}

/**
 * PmSystemResumePowerOffSuspend() - Resume from Power Off Suspend state
 *
 * @return	XST_SUCCESS if system is resumed properly, error code otherwise
 */
s32 PmSystemResumePowerOffSuspend(void)
{
	s32 status;
	u32 i;
	PmRequirement* req;

	/* Wait for FSBL to start and get boot type */
	do {
		i = XPfw_Read32(PMU_GLOBAL_GLOBAL_GEN_STORAGE2);
	} while (0U == i);

	/* Clear Gen Storage register so it can be used later in system */
	XPfw_Write32(PMU_GLOBAL_GLOBAL_GEN_STORAGE2, 0U);

	/* DDR context restore */
	status = PmHookRestoreDdrContext();
	if (status != XST_SUCCESS) {
		goto done;
	}

	/* Resume DDR operation */
	status = PmDdrPowerOffSuspendResume();
	if (status != XST_SUCCESS) {
		goto done;
	}

	/* System context restore */
	PmSystemRestoreContext();

	pmSystem.suspendType = PM_SUSPEND_TYPE_REGULAR;

	/* Update state of slave nodes */
	for (i = 0U; i < pmNodeClassSlave_g.bucketSize; i++) {
		PmSlave* slave = pmNodeClassSlave_g.bucket[i]->derived;
		slave->node.currState = slave->slvFsm->statesCnt - 1U;
		status = PmUpdateSlave(slave);
		if (status != XST_SUCCESS) {
			goto done;
		}
	}

	/* Update state of proc nodes */
	for (i = 0U; i < pmNodeClassProc_g.bucketSize; i++) {
		PmProc* proc = pmNodeClassProc_g.bucket[i]->derived;
		status = PmProcSleep(proc);
		if (status != XST_SUCCESS) {
			goto done;
		}
	}

	/* Update state of power nodes */
	for (i = 0U; i < pmNodeClassPower_g.bucketSize; i++) {
		PmPower* power = pmNodeClassPower_g.bucket[i]->derived;
		power->node.currState = PM_PWR_STATE_ON;
		if (power->useCount == 0U) {
			status = PmPowerDown(power);
			if (status != XST_SUCCESS) {
				goto done;
			}
		}
	}

	/* Wake masters that have external node as wake source */
	status = PmExternWakeMasters();
	if (status != XST_SUCCESS) {
		goto done;
	}

	/* Set default value for system requirements */
	for (i = 0U; i < ARRAY_SIZE(pmSystemReqs); i++) {
		u32 caps = pmSystemReqs[i].caps;
		req = PmRequirementGetNoMaster(pmSystemReqs[i].slave);
		if (NULL != req) {
			if (req->currReq != caps) {
				status = PmRequirementUpdate(req, caps);
				if (XST_SUCCESS != status) {
					goto done;
				}
			}
		} else {
			status = XST_FAILURE;
			goto done;
		}
	}

	/* Release DDR context saving slave capabilities */
	status = PmSystemPosFinalizeDdrCaps();

done:
	return status;
}

#endif

u32 PmSystemSuspendType(void)
{
	return pmSystem.suspendType;
}

void PmSystemSetSuspendType(u32 type)
{
	pmSystem.suspendType = type;
}

/**
 * PmMasterCanSysShutdown() - Check if master has permissions for system
 *			      shutdown/restart.
 * @master      Master which wants to shutdown/restart the system
 *
 * @return      True if master has permission to restart the system,
 *              false otherwise
 */
bool PmMasterCanSysShutdown(const PmMaster* const master)
{
        return 0U != (pmSystem.systemRestartPerms & master->ipiMask);
}

/**
 * PmSystemSetConfig() - Set configuration for system control
 * @permissions Permissions to set (ORed IPI masks of permissible masters)
 *
 * @return	XST_SUCCESS, permissions are set
 */
s32 PmSystemSetConfig(const u32 perms)
{
	/* Set permission for system restart/shutdown */
	pmSystem.systemRestartPerms = perms;

	return XST_SUCCESS;
}
#endif
