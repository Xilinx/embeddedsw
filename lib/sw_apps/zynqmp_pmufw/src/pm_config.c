/*
* Copyright (c) 2014 - 2021 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
 */

#include "xpfw_config.h"
#ifdef ENABLE_PM

#include "pm_config.h"
#include "xil_types.h"
#include "xstatus.h"
#include "pm_common.h"
#include "pm_defs.h"
#include "pm_master.h"
#include "pm_slave.h"
#include "pm_reset.h"
#include "pm_requirement.h"
#include "pm_node.h"
#include "pm_system.h"
#include "pm_pll.h"

typedef s32 (*const PmConfigSectionHandler)(u32* const addr);

static s32 PmConfigSlaveSectionHandler(u32* const addr);
static s32 PmConfigMasterSectionHandler(u32* const addr);
static s32 PmConfigPreallocSectionHandler(u32* const addr);
static s32 PmConfigPowerSectionHandler(u32* const addr);
static s32 PmConfigResetSectionHandler(u32* const addr);
static s32 PmConfigShutdownSectionHandler(u32* const addr);
static s32 PmConfigSetConfigSectionHandler(u32* const addr);
static s32 PmConfigGpoSectionHandler(u32* const addr);

/*********************************************************************
 * Macros
 ********************************************************************/

#define PM_CONFIG_GPO_2_ENABLE_MASK	BIT(10U)
#define PM_CONFIG_GPO_3_ENABLE_MASK	BIT(11U)
#define PM_CONFIG_GPO_4_ENABLE_MASK	BIT(12U)
#define PM_CONFIG_GPO_5_ENABLE_MASK	BIT(13U)

/* Section IDs (must match to what PCW generates) */
#define PM_CONFIG_MASTER_SECTION_ID	0x101U
#define PM_CONFIG_SLAVE_SECTION_ID	0x102U
#define PM_CONFIG_PREALLOC_SECTION_ID	0x103U
#define PM_CONFIG_POWER_SECTION_ID	0x104U
#define PM_CONFIG_RESET_SECTION_ID	0x105U
#define PM_CONFIG_SHUTDOWN_SECTION_ID	0x106U
#define PM_CONFIG_SET_CONFIG_SECTION_ID	0x107U
#define PM_CONFIG_GPO_SECTION_ID	0x108U

/* Size in bytes of one data field in configuration object */
#define PM_CONFIG_WORD_SIZE	4U

#define PM_CONFIG_OBJECT_LOADED	0x1U

#define PM_CONFIG_GPO_MASK	(BIT(5U) | BIT(4U) | BIT(3U) | BIT(2U))

/* Default number of sections in configuration object */
#define DEFAULT_SECTIONS_NUM	7U

/* Type of Config Obejcts */
#define PM_CONFIG_OBJECT_TYPE_BASE	(1U)
#define PM_CONFIG_OBJECT_TYPE_OVERLAY	(2U)

/*********************************************************************
 * Structure definitions
 ********************************************************************/
/**
 * PmConfigSection - Information about the section
 * @id          Section identifier
 * @handler     Handler for processing section data from the object
 */
typedef struct {
	const u32 id;
	PmConfigSectionHandler handler;
} PmConfigSection;

static PmConfigSection pmConfigSections[] = {
	{
		.id = PM_CONFIG_MASTER_SECTION_ID,
		.handler = PmConfigMasterSectionHandler,
	}, {
		.id = PM_CONFIG_SLAVE_SECTION_ID,
		.handler = PmConfigSlaveSectionHandler,
	}, {
		.id = PM_CONFIG_PREALLOC_SECTION_ID,
		.handler = PmConfigPreallocSectionHandler,
	}, {
		.id = PM_CONFIG_POWER_SECTION_ID,
		.handler = PmConfigPowerSectionHandler,
	}, {
		.id = PM_CONFIG_RESET_SECTION_ID,
		.handler = PmConfigResetSectionHandler,
	}, {
		.id = PM_CONFIG_SHUTDOWN_SECTION_ID,
		.handler = PmConfigShutdownSectionHandler,
	}, {
		.id = PM_CONFIG_SET_CONFIG_SECTION_ID,
		.handler = PmConfigSetConfigSectionHandler,
	}, {
		.id = PM_CONFIG_GPO_SECTION_ID,
		.handler = PmConfigGpoSectionHandler,
	},
};

/**
 * PmConfig - Configuration object data
 * @configPerms ORed masks of masters which are allowed to load the object
 * @flags       Flags: bit(0) - set if the configuration object is loaded
 * @secNumber   Number of sections in configuration object
 */
typedef struct {
	u32 configPerms;
	u8 flags;
	u32 secNumber;
	u32 headerLength;
	u32 confObjType;
	u32 overlayConfigPerms;
} PmConfig;

/*
 * The first configuration object can be loaded only by APU or RPU_0. Later,
 * the loading depends on the permissions specified in the set config section
 * in the provided object. If zero permissions are given, the loading of
 * configuration is locked-down.
 */
static PmConfig pmConfig = {
	.configPerms = IPI_PMU_0_IER_APU_MASK | IPI_PMU_0_IER_RPU_0_MASK,
	.flags = 0U,
	.headerLength = 0U,
	.secNumber = DEFAULT_SECTIONS_NUM,
	.confObjType = PM_CONFIG_OBJECT_TYPE_BASE,
	.overlayConfigPerms = 0U,
};

/**
 * PmConfigObjectIsLoaded() - Check if the configuration object is loaded
 * @return	True if the object is loaded, false otherwise
 */
inline bool PmConfigObjectIsLoaded(void)
{
	return 0U != (pmConfig.flags & PM_CONFIG_OBJECT_LOADED);
}

/**
 * PmConfigReadNext() - Read the next word from configuration object
 *
 * @return      The word read from object
 *
 * @note        As the side effect, the nextAddr value is incremented
 */
static u32 PmConfigReadNext(u32* const addr)
{
	u32 val;

	val = XPfw_Read32(*addr);
	*addr += PM_CONFIG_WORD_SIZE;

	return val;
}

/**
 * PmConfigSkipWords() - Skip reading a number of words
 * @addr	Current address to increase by the number of skipped words
 * @words	Number of words to skip
 */
static void PmConfigSkipWords(u32* const addr, const u32 words)
{
	*addr += words * PM_CONFIG_WORD_SIZE;
}

/**
 * PmConfigMasterSectionHandler() - Read and process masters section
 * @addr        Start address of the section in configuration object
 *
 * @return      XST_SUCCESS if section is loaded successfully, XST_FAILURE
 *              otherwise
 */
static s32 PmConfigMasterSectionHandler(u32* const addr)
{
	s32 status = XST_SUCCESS;
	u32 i, mastersCnt;

	mastersCnt = PmConfigReadNext(addr);

	for (i = 0U; i < mastersCnt; i++) {
		u32 nodeId;
		PmMaster* master;
		PmMasterConfig config;

		nodeId = PmConfigReadNext(addr);
		master = PmMasterGetPlaceholder((PmNodeId)nodeId);
		if (NULL == master) {
			PmErr("Unknown master #%lu\r\n", nodeId);
			status = XST_FAILURE;
			goto done;
		}

		config.ipiMask = PmConfigReadNext(addr);
		config.suspendTimeout = PmConfigReadNext(addr);
		config.suspendPerms = PmConfigReadNext(addr);
		config.wakePerms = PmConfigReadNext(addr);
		PmMasterSetConfig(master, &config);
	}

done:
	return status;
}

/**
 * PmConfigSlaveSectionHandler() - Read and process slaves section
 * @addr        Start address of the section in configuration object
 *
 * @return      XST_SUCCESS if section is loaded successfully, XST_FAILURE
 *              otherwise
 */
static s32 PmConfigSlaveSectionHandler(u32* const addr)
{
	s32 status = XST_SUCCESS;
	u32 i, slavesCnt;

	slavesCnt = PmConfigReadNext(addr);

	for (i = 0U; i < slavesCnt; i++) {
		u32 nodeId, usagePolicy, usagePerms;
		PmSlave* slave;

		nodeId = PmConfigReadNext(addr);
		slave = (PmSlave*)PmNodeGetSlave(nodeId);
		if (NULL == slave) {
			PmErr("Unknown slave #%lu\r\n", nodeId);
			status = XST_FAILURE;
			goto done;
		}

		usagePolicy = PmConfigReadNext(addr);
		usagePerms = PmConfigReadNext(addr);
		status = PmSlaveSetConfig(slave, usagePolicy, usagePerms);
		if (XST_SUCCESS != status) {
			PmErr("%s config failed\r\n", slave->node.name);
			goto done;
		}
	}

done:
	return status;
}

/**
 * PmConfigPreallocForMaster() - Process preallocation for the master
 * @master      Master whose preallocated slaves are to be processed
 * @addr        Start address of the prealloc subsection in the configuration
 *              object
 * @return      XST_SUCCESS if preallocated slaves are processed correctly,
 *              XST_FAILURE otherwise
 */
static s32 PmConfigPreallocForMaster(const PmMaster* const master,
				     u32* const addr)
{
	s32 status = XST_SUCCESS;
	u32 i, preallocCnt;

	preallocCnt = PmConfigReadNext(addr);
	for (i = 0U; i < preallocCnt; i++) {
		u32 nodeId, flags, currReq, defReq;
		PmSlave* slave;
		PmRequirement* req;

		/* Get slave by node ID */
		nodeId = PmConfigReadNext(addr);
		slave = (PmSlave*)PmNodeGetSlave(nodeId);
		if (NULL == slave) {
			PmErr("Unknown slave #%lu\r\n", nodeId);
			status = XST_FAILURE;
			goto done;
		}

		/* Get the requirement structure for master/slave pair */
		req = PmRequirementGet(master, slave);
		if (NULL == req) {
			PmErr("Invalid (%s, %s) pair\r\n", master->name,
			      slave->node.name);
			status = XST_FAILURE;
			goto done;
		}

		flags = PmConfigReadNext(addr);
		currReq = PmConfigReadNext(addr);
		defReq = PmConfigReadNext(addr);
		status = PmRequirementSetConfig(req, flags, currReq, defReq);
		if (XST_SUCCESS != status) {
			goto done;
		}
	}

done:
	return status;
}

/**
 * PmConfigPreallocSectionHandler() - Read and process section containing
 *              information about the preallocated slaves
 * @addr        Start address of the section in configuration object
 *
 * @return      XST_SUCCESS if section is loaded successfully, XST_FAILURE
 *              otherwise
 */
static s32 PmConfigPreallocSectionHandler(u32* const addr)
{
	s32 status = XST_SUCCESS;

	u32 i, mastersCnt;

	mastersCnt = PmConfigReadNext(addr);

	for (i = 0U; i < mastersCnt; i++) {
		u32 ipiMask;
		PmMaster* master;

		/* Get the master by specified IPI mask */
		ipiMask = PmConfigReadNext(addr);
		master = PmGetMasterByIpiMask(ipiMask);
		if (NULL == master) {
			PmErr("Unknown master (ipi=0x%lx)\r\n", ipiMask);
			status = XST_FAILURE;
			goto done;
		}

		status = PmConfigPreallocForMaster(master, addr);
		if (XST_SUCCESS != status) {
			goto done;
		}
	}

done:
	return status;
}

/**
 * PmConfigPowerSectionHandler() - Read and process power section
 * @addr        Start address of the section in configuration object
 *
 * @return      XST_SUCCESS if section is loaded successfully, XST_FAILURE
 *              otherwise
 */
static s32 PmConfigPowerSectionHandler(u32* const addr)
{
	s32 status = XST_SUCCESS;
	u32 i, powersCnt;

	powersCnt = PmConfigReadNext(addr);

	for (i = 0U; i < powersCnt; i++) {
		u32 powerId;
		PmPower* power;

		powerId = PmConfigReadNext(addr);

		power = (PmPower*)PmNodeGetPower(powerId);
		if (NULL == power) {
			PmErr("Unknown power #%lu\r\n", powerId);
			status = XST_FAILURE;
			goto done;
		}
		power->forcePerms = PmConfigReadNext(addr);
	}

done:
	return status;
}

/**
 * PmConfigResetSectionHandler() - Read and process reset section
 * @addr        Start address of the section in configuration object
 *
 * @return      XST_SUCCESS if section is loaded successfully, XST_FAILURE
 *              otherwise
 */
static s32 PmConfigResetSectionHandler(u32* const addr)
{
	s32 status = XST_SUCCESS;
	u32 i, resetsCnt;

	resetsCnt = PmConfigReadNext(addr);

	for (i = 0U; i < resetsCnt; i++) {
		u32 resetId, permissions;

		resetId = PmConfigReadNext(addr);
		permissions = PmConfigReadNext(addr);

		status = PmResetSetConfig(resetId, permissions);
		if (XST_SUCCESS != status) {
			PmErr("Unknown reset #%lu\r\n", resetId);
			goto done;
		}
	}

done:
	return status;
}

/**
 * PmConfigShutdownSectionHandler() - Read and process shutdown section
 * @addr        Start address of the section in configuration object
 *
 * @return      XST_SUCCESS if section is loaded successfully, XST_FAILURE
 *              otherwise
 */
static s32 PmConfigShutdownSectionHandler(u32* const addr)
{
	s32 status = XST_SUCCESS;
	u32 permission;

	/* Read permission from config object */
	permission = PmConfigReadNext(addr);

	/* Set the permission in system config object */
	status = PmSystemSetConfig(permission);
	if (XST_SUCCESS != status) {
		PmErr("Failed to set system config\r\n");
	}

	return status;
}

/**
 * PmConfigSetConfigSectionHandler() - Read and process set configuration
 *              section
 * @addr        Start address of the section in configuration object
 *
 * @return      XST_SUCCESS always
 */
static s32 PmConfigSetConfigSectionHandler(u32* const addr)
{
	pmConfig.configPerms = PmConfigReadNext(addr);

	if ((pmConfig.headerLength > 1U) &&
	    (PM_CONFIG_OBJECT_TYPE_BASE == pmConfig.confObjType)) {
		pmConfig.overlayConfigPerms = PmConfigReadNext(addr);
	}

	return XST_SUCCESS;
}

/**
 * PmConfigHeaderHandler() - Read and process header of config object
 * @addr        Start address of the header in configuration object
 */
static void PmConfigHeaderHandler(u32* const addr)
{
	u32 remWords;

	/* Read number of remaining words in header */
	remWords = PmConfigReadNext(addr);
	pmConfig.headerLength = remWords;

	/* If there is words in header, get number of sections in object */
	if (remWords > 0U) {
		pmConfig.secNumber = PmConfigReadNext(addr);
		remWords--;
	}

	/* If there is words in header, get type of config object */
	if (remWords > 0U) {
		pmConfig.confObjType = PmConfigReadNext(addr);
		remWords--;
	}

	/* Skip the remaining words in header */
	PmConfigSkipWords(addr, remWords);
}

/**
 * PmConfigGpoSectionHandler() - Read and process GPO section
 * @addr        Start address of the section in configuration object
 *
 * @return      XST_SUCCESS always
 */
static s32 PmConfigGpoSectionHandler(u32* const addr)
{
	u32 gpoState, reg;

	gpoState = PmConfigReadNext(addr);
	reg = XPfw_Read32(PMU_LOCAL_GPO1_READ);
	reg &= ~PM_CONFIG_GPO_MASK;
	reg |= (gpoState & PM_CONFIG_GPO_MASK);
	XPfw_Write32(PMU_IOMODULE_GPO1, reg);

#ifdef CONNECT_PMU_GPO_2
	if ((gpoState & PM_CONFIG_GPO_2_ENABLE_MASK) != 0U) {
		XPfw_RMW32((IOU_SLCR_BASE + IOU_SLCR_MIO_PIN_34_OFFSET),
				0x000000FEU, 0x00000008U);
	}
#endif

#ifdef CONNECT_PMU_GPO_3
	if ((gpoState & PM_CONFIG_GPO_3_ENABLE_MASK) != 0U) {
		XPfw_RMW32((IOU_SLCR_BASE + IOU_SLCR_MIO_PIN_35_OFFSET),
				0x000000FEU, 0x00000008U);
	}
#endif

#ifdef CONNECT_PMU_GPO_4
	if ((gpoState & PM_CONFIG_GPO_4_ENABLE_MASK) != 0U) {
		XPfw_RMW32((IOU_SLCR_BASE + IOU_SLCR_MIO_PIN_36_OFFSET),
				0x000000FEU, 0x00000008U);
	}
#endif

#ifdef CONNECT_PMU_GPO_5
	if ((gpoState & PM_CONFIG_GPO_5_ENABLE_MASK) != 0U) {
		XPfw_RMW32((IOU_SLCR_BASE + IOU_SLCR_MIO_PIN_37_OFFSET),
				0x000000FEU, 0x00000008U);
	}
#endif

	return XST_SUCCESS;
}

/**
 * PmConfigClear() - Clear previous PFW configuration
 */
static void PmConfigClear(void)
{
	PmMasterClearConfig();
	PmResetClearConfig();
	PmNodeClearConfig();
	pmConfig.configPerms = 0U;
	pmConfig.secNumber = DEFAULT_SECTIONS_NUM;
}

/**
 * PmConfigGetSectionById() - Get section struct based on section ID
 * @sid         Section ID to look for
 *
 * @return      Pointer to the section structure or NULL if not found
 */
static PmConfigSection* PmConfigGetSectionById(const u32 sid)
{
	PmConfigSection* sec = NULL;
	u32 i;

	for (i = 0U; i < ARRAY_SIZE(pmConfigSections); i++) {
		if (sid == pmConfigSections[i].id) {
			sec = &pmConfigSections[i];
			break;
		}
	}

	return sec;
}

/**
 * PmConfigPllPermsWorkaround() - Workaround for configuring PLL permissions
 *
 * @note	This is a workaround for PMU-FW not knowing who should be given
 * permission to control a PLL, and also which PLLs. Currently, the DP driver in
 * linux requires direct control to VPLL (for video) and RPLL (for audio). This
 * information should be provided in configuration object. Since that's not
 * possible, the information has to be hard-coded here. Hopefully this
 * workaround will be removed in future. Thereby, it is required here to assume
 * that if APU has permission to use the DP it should be automatically given
 * permissions to directly control VPLL and RPLL.
 */
static void PmConfigPllPermsWorkaround(void)
{
	PmMaster* apu = PmMasterGetPlaceholder(NODE_APU);
	PmSlave* dp = (PmSlave*)PmNodeGetSlave(NODE_DP);
	PmRequirement* req;

	if ((NULL == apu) || (NULL == dp)) {
		goto done;
	}

	req = PmRequirementGet(apu, dp);
	if (NULL == req) {
		goto done;
	}
	PmPllOpenAccess(&pmVpll_g, apu->ipiMask);
	PmPllOpenAccess(&pmRpll_g, apu->ipiMask);

done:
	return;
}

/**
 * PmConfigLoadObject() - Load information provided in configuration object
 * @address     Start address of the configuration object
 * @callerIpi   IPI mask of the master who called set configuration API
 *
 * @return      Status of loading information from configuration object
 */
s32 PmConfigLoadObject(const u32 address, const u32 callerIpi)
{
	s32 status = XST_SUCCESS;
	u32 currAddr = address;
	u32 i;
	u32 confObjType;

	/* Set config object type as base before reading header to support
	 * backward compatibility.
	 */
	pmConfig.confObjType = PM_CONFIG_OBJECT_TYPE_BASE;

	/* Read and process header from the object */
	PmConfigHeaderHandler(&currAddr);
	confObjType = pmConfig.confObjType;

	/* Check for the config object type */
	if ((PM_CONFIG_OBJECT_TYPE_BASE != confObjType) &&
	    (PM_CONFIG_OBJECT_TYPE_OVERLAY != confObjType)) {
		PmWarn("Invalid config object type\r\n");
		status = XST_PM_INVALID_NODE;
		goto ret;
	}

	/* Check for permissions to load the configuration object */
	if (((PM_CONFIG_OBJECT_TYPE_BASE == confObjType) &&
	     (0U == (callerIpi & pmConfig.configPerms))) ||
	    ((PM_CONFIG_OBJECT_TYPE_OVERLAY == confObjType) &&
	     (0U == (callerIpi & pmConfig.overlayConfigPerms)))) {
			PmWarn("No permission to set config\r\n");
			status = XST_PM_NO_ACCESS;
			goto ret;
	}

	if (PM_CONFIG_OBJECT_TYPE_BASE == confObjType) {
		PmConfigClear();
		status = PmSystemRequirementAdd();
		if (XST_SUCCESS != status) {
			goto done;
		}
	}

	/* Read and process each section from the object */
	for (i = 0U; i < pmConfig.secNumber; i++) {
		PmConfigSection* section;
		u32 sectionId;

		sectionId = PmConfigReadNext(&currAddr);
		section = PmConfigGetSectionById(sectionId);

		if ((NULL == section) || (NULL == section->handler)) {
			PmAlert("Unknown section #%lu\r\n", sectionId);
			status = XST_FAILURE;
			goto done;
		}

		status = section->handler(&currAddr);
		if (XST_SUCCESS != status) {
			goto done;
		}
	}

done:
	if (PM_CONFIG_OBJECT_TYPE_BASE == confObjType) {
		if (XST_SUCCESS == status) {
			pmConfig.flags |= PM_CONFIG_OBJECT_LOADED;
			PmConfigPllPermsWorkaround();
			status = PmNodeInit();
			PmNodeForceDownUnusable();
		} else {
			pmConfig.flags &= ~(u8)PM_CONFIG_OBJECT_LOADED;
		}
	}
ret:
	return status;
}

#endif
