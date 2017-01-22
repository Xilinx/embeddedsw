/*
 * Copyright (C) 2014 - 2016 Xilinx, Inc.  All rights reserved.
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
 */

#include "pm_config.h"
#include "xil_types.h"
#include "xstatus.h"
#include "pm_common.h"
#include "pm_defs.h"

typedef int (*const PmConfigSectionHandler)(u32* const addr);

static int PmConfigSlaveSectionHandler(u32* const addr);
static int PmConfigMasterSectionHandler(u32* const addr);
static int PmConfigPreallocSectionHandler(u32* const addr);
static int PmConfigPowerSectionHandler(u32* const addr);
static int PmConfigResetSectionHandler(u32* const addr);
static int PmConfigShutdownSectionHandler(u32* const addr);
static int PmConfigSetConfigSectionHandler(u32* const addr);

/*********************************************************************
 * Macros
 ********************************************************************/

/* Section IDs (must match to what PCW generates) */
#define PM_CONFIG_MASTER_SECTION_ID	0x101U
#define PM_CONFIG_SLAVE_SECTION_ID	0x102U
#define PM_CONFIG_PREALLOC_SECTION_ID	0x103U
#define PM_CONFIG_POWER_SECTION_ID	0x104U
#define PM_CONFIG_RESET_SECTION_ID	0x105U
#define PM_CONFIG_SHUTDOWN_SECTION_ID	0x106U
#define PM_CONFIG_SET_CONFIG_SECTION_ID	0x107U

/* Size in bytes of one data field in configuration object */
#define PM_CONFIG_WORD_SIZE	4U

#define PM_CONFIG_OBJECT_LOADED	0x1U

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
	},
};

/**
 * PmConfig - Configuration object data
 * @configPerms ORed masks of masters which are allowed to load the object
 * @flags       Flags: bit(0) - set if the configuration object is loaded
 */
typedef struct {
	u32 configPerms;
	u8 flags;
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
static int PmConfigMasterSectionHandler(u32* const addr)
{
	int status = XST_SUCCESS;

	return status;
}

/**
 * PmConfigSlaveSectionHandler() - Read and process slaves section
 * @addr        Start address of the section in configuration object
 *
 * @return      XST_SUCCESS if section is loaded successfully, XST_FAILURE
 *              otherwise
 */
static int PmConfigSlaveSectionHandler(u32* const addr)
{
	int status = XST_SUCCESS;

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
static int PmConfigPreallocSectionHandler(u32* const addr)
{
	int status = XST_SUCCESS;

	return status;
}

/**
 * PmConfigPowerSectionHandler() - Read and process power section
 * @addr        Start address of the section in configuration object
 *
 * @return      XST_SUCCESS if section is loaded successfully, XST_FAILURE
 *              otherwise
 */
static int PmConfigPowerSectionHandler(u32* const addr)
{
	int status = XST_SUCCESS;

	return status;
}

/**
 * PmConfigResetSectionHandler() - Read and process reset section
 * @addr        Start address of the section in configuration object
 *
 * @return      XST_SUCCESS if section is loaded successfully, XST_FAILURE
 *              otherwise
 */
static int PmConfigResetSectionHandler(u32* const addr)
{
	int status = XST_SUCCESS;

	return status;
}

/**
 * PmConfigShutdownSectionHandler() - Read and process shutdown section
 * @addr        Start address of the section in configuration object
 *
 * @return      XST_SUCCESS if section is loaded successfully, XST_FAILURE
 *              otherwise
 */
static int PmConfigShutdownSectionHandler(u32* const addr)
{
	int status = XST_SUCCESS;

	return status;
}

/**
 * PmConfigSetConfigSectionHandler() - Read and process set configuration
 *              section
 * @addr        Start address of the section in configuration object
 *
 * @return      XST_SUCCESS if section is loaded successfully, XST_FAILURE
 *              otherwise
 */
static int PmConfigSetConfigSectionHandler(u32* const addr)
{
	int status = XST_SUCCESS;

	return status;
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
 * PmConfigLoadObject() - Load information provided in configuration object
 * @address     Start address of the configuration object
 * @callerIpi   IPI mask of the master who called set configuration API
 *
 * @return      Status of loading information from configuration object
 */
int PmConfigLoadObject(const u32 address, const u32 callerIpi)
{
	int status = XST_SUCCESS;
	u32 currAddr = address;
	u32 i, remWords;

	/* Check for permissions to load the configuration object */
	if (0U == (callerIpi & pmConfig.configPerms)) {
		status = XST_PM_NO_ACCESS;
		goto ret;
	}

	/* Read number of remaining words in header */
	remWords = PmConfigReadNext(&currAddr);

	/* Skip the remaining words in header */
	PmConfigSkipWords(&currAddr, remWords);

	/* Read and process each section from the object */
	for (i = 0U; i < ARRAY_SIZE(pmConfigSections); i++) {
		PmConfigSection* section;
		u32 sectionId;

		sectionId = PmConfigReadNext(&currAddr);
		section = PmConfigGetSectionById(sectionId);

		if ((NULL == section) || (NULL == section->handler)) {
			status = XST_FAILURE;
			goto done;
		}

		status = section->handler(&currAddr);
		if (XST_SUCCESS != status) {
			goto done;
		}
	}

done:
	if (XST_SUCCESS == status) {
		pmConfig.flags |= PM_CONFIG_OBJECT_LOADED;
	} else {
		pmConfig.flags &= ~PM_CONFIG_OBJECT_LOADED;
	}
ret:
	return status;
}
