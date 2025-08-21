/*
 * Copyright (c) 2014 - 2022 Xilinx, Inc.  All rights reserved.
 * Copyright (c) 2022 - 2025 Advanced Micro Devices, Inc. All Rights Reserved.
 * SPDX-License-Identifier: MIT
 */

#include "xpfw_config.h"
#ifdef ENABLE_PM

/*********************************************************************
 * Implementations of the functions to be used for integrating power
 * management (PM) within PMU firmware.
 *********************************************************************/

#ifdef ENABLE_CSU_MULTIBOOT
#include "csu.h"
#endif
#include "pm_binding.h"
#include "pm_defs.h"
#include "pm_common.h"
#include "pm_proc.h"
#include "pm_core.h"
#include "pm_notifier.h"
#include "pm_power.h"
#include "pm_gic_proxy.h"
#include "pm_requirement.h"
#include "pm_extern.h"
#include "pm_usb.h"
#include "pm_hooks.h"

/* All GIC wakes in GPI1 */
#define PMU_IOMODULE_GPI1_GIC_WAKES_ALL_MASK \
		(PMU_IOMODULE_GPI1_ACPU_0_WAKE_MASK | \
		PMU_IOMODULE_GPI1_ACPU_1_WAKE_MASK | \
		PMU_IOMODULE_GPI1_ACPU_2_WAKE_MASK | \
		PMU_IOMODULE_GPI1_ACPU_3_WAKE_MASK | \
		PMU_IOMODULE_GPI1_R5_0_WAKE_MASK | \
		PMU_IOMODULE_GPI1_R5_1_WAKE_MASK)

#define PMU_IOMODULE_GPI1_MIO_WAKE_ALL_MASK \
		(PMU_IOMODULE_GPI1_MIO_WAKE_0_MASK | \
		PMU_IOMODULE_GPI1_MIO_WAKE_1_MASK | \
		PMU_IOMODULE_GPI1_MIO_WAKE_2_MASK | \
		PMU_IOMODULE_GPI1_MIO_WAKE_3_MASK | \
		PMU_IOMODULE_GPI1_MIO_WAKE_4_MASK | \
		PMU_IOMODULE_GPI1_MIO_WAKE_5_MASK)

#ifdef ENABLE_CSU_MULTIBOOT
static u32 CsuMultibootRegVal;

/**
 * GetCsuMultibootVal() - Get CSU multiboot register value
 */
u32 GetCsuMultibootVal(void)
{
	return CsuMultibootRegVal;
}
#endif

#ifdef ENABLE_SECURE_FLAG
/* Number of bits allocated to represent permissions for each API. */
#define BITS_PER_API		2U
/* Number of bits in one word. */
#define WORD_SIZE_BITS		32U
/* Total number of permission bits required for all APIs. */
#define PERMISSION_BITS		(PM_API_MAX * BITS_PER_API)
/*
 * Number of words required to store all permission bits. This uses
 * ceiling division to ensure any remaining bits fit in the last word.
 */
#define PERMISSION_WORDS	((PERMISSION_BITS + WORD_SIZE_BITS - 1) / WORD_SIZE_BITS)

/* PMU API access permission types */
#define PMU_API_NO_ACCESS		0U
#define PMU_API_SECURE_ACCESS		1U
#define PMU_API_NON_SECURE_ACCESS	2U
#define PMU_API_FULL_ACCESS		3U

/* APERPERM register configuration */
#define APERPERM_48_REG_BASE_ADDR		0xFF9810C0U
#define APERPERM_48_REG_OFFSET			0x4U
#define APERPERM_48_REG_TRUSTZONE_BIT_INDEX	27U

#define SECURE_FLAG_BIT_SHIFT		24U
#define SECURE_FLAG_BIT_MASK		(1UL << SECURE_FLAG_BIT_SHIFT)

/* Bitmap storing access permissions for all APIs. */
static uint32_t apiPermissionBitmap[PERMISSION_WORDS];

/**
 * PmApiPermission - Represents access permissions for a specific API.
 * This structure defines an API by its identifier and the
 * corresponding permission flags that determine which masters
 * can access it.
 *
 * @apiId		Unique identifier of the API.
 * @permissionFlags	Permission flags for the API.
 */
typedef struct {
	uint32_t apiId;
	uint32_t permissionFlags;
} PmApiPermission;

/* Default permission table for all APIs. */
static const PmApiPermission defaultApiPermissions[] = {
	{PM_API(PM_GET_API_VERSION), PMU_API_FULL_ACCESS},
	{PM_API(PM_SET_CONFIGURATION), PMU_API_FULL_ACCESS},
	{PM_API(PM_GET_NODE_STATUS), PMU_API_FULL_ACCESS},
	{PM_API(PM_GET_OP_CHARACTERISTIC), PMU_API_FULL_ACCESS},
	{PM_API(PM_REGISTER_NOTIFIER), PMU_API_FULL_ACCESS},
	{PM_API(PM_REQUEST_SUSPEND), PMU_API_FULL_ACCESS},
	{PM_API(PM_SELF_SUSPEND), PMU_API_FULL_ACCESS},
	{PM_API(PM_FORCE_POWERDOWN), PMU_API_FULL_ACCESS},
	{PM_API(PM_ABORT_SUSPEND), PMU_API_FULL_ACCESS},
	{PM_API(PM_REQUEST_WAKEUP), PMU_API_FULL_ACCESS},
	{PM_API(PM_SET_WAKEUP_SOURCE), PMU_API_FULL_ACCESS},
	{PM_API(PM_SYSTEM_SHUTDOWN), PMU_API_FULL_ACCESS},
	{PM_API(PM_REQUEST_NODE), PMU_API_FULL_ACCESS},
	{PM_API(PM_RELEASE_NODE), PMU_API_FULL_ACCESS},
	{PM_API(PM_SET_REQUIREMENT), PMU_API_FULL_ACCESS},
	{PM_API(PM_SET_MAX_LATENCY), PMU_API_FULL_ACCESS},
	{PM_API(PM_RESET_ASSERT), PMU_API_FULL_ACCESS},
	{PM_API(PM_RESET_GET_STATUS), PMU_API_FULL_ACCESS},
	{PM_API(PM_MMIO_WRITE), PMU_API_FULL_ACCESS},
	{PM_API(PM_MMIO_READ), PMU_API_FULL_ACCESS},
	{PM_API(PM_INIT_FINALIZE), PMU_API_FULL_ACCESS},
	{PM_API(PM_GET_CHIPID), PMU_API_FULL_ACCESS},
	{PM_API(PM_PINCTRL_REQUEST), PMU_API_FULL_ACCESS},
	{PM_API(PM_PINCTRL_RELEASE), PMU_API_FULL_ACCESS},
	{PM_API(PM_PINCTRL_GET_FUNCTION), PMU_API_FULL_ACCESS},
	{PM_API(PM_PINCTRL_SET_FUNCTION), PMU_API_FULL_ACCESS},
	{PM_API(PM_PINCTRL_CONFIG_PARAM_GET), PMU_API_FULL_ACCESS},
	{PM_API(PM_PINCTRL_CONFIG_PARAM_SET), PMU_API_FULL_ACCESS},
#ifdef ENABLE_IOCTL
	{PM_API(PM_IOCTL), PMU_API_FULL_ACCESS},
#endif
	{PM_API(PM_CLOCK_ENABLE), PMU_API_FULL_ACCESS},
	{PM_API(PM_CLOCK_DISABLE), PMU_API_FULL_ACCESS},
	{PM_API(PM_CLOCK_GETSTATE), PMU_API_FULL_ACCESS},
	{PM_API(PM_CLOCK_SETDIVIDER), PMU_API_FULL_ACCESS},
	{PM_API(PM_CLOCK_GETDIVIDER), PMU_API_FULL_ACCESS},
	{PM_API(PM_CLOCK_SETRATE), PMU_API_NO_ACCESS},
	{PM_API(PM_CLOCK_SETPARENT), PMU_API_FULL_ACCESS},
	{PM_API(PM_CLOCK_GETPARENT), PMU_API_FULL_ACCESS},
#ifdef ENABLE_SECURE
	{PM_API(PM_SECURE_SHA), PMU_API_FULL_ACCESS},
	{PM_API(PM_SECURE_RSA), PMU_API_FULL_ACCESS},
	{PM_API(PM_SECURE_IMAGE), PMU_API_FULL_ACCESS},
	{PM_API(PM_SECURE_AES), PMU_API_FULL_ACCESS},
#endif
	{PM_API(PM_PLL_SET_PARAMETER), PMU_API_FULL_ACCESS},
	{PM_API(PM_PLL_GET_PARAMETER), PMU_API_FULL_ACCESS},
	{PM_API(PM_PLL_SET_MODE), PMU_API_FULL_ACCESS},
	{PM_API(PM_PLL_GET_MODE), PMU_API_FULL_ACCESS},
	{PM_API(PM_REGISTER_ACCESS), PMU_API_FULL_ACCESS},
#ifdef EFUSE_ACCESS
	{PM_API(PM_EFUSE_ACCESS), PMU_API_FULL_ACCESS},
#endif
	{PM_API(PM_FEATURE_CHECK), PMU_API_FULL_ACCESS},
#ifdef ENABLE_FPGA_LOAD
	{PM_API(PM_FPGA_LOAD), PMU_API_FULL_ACCESS},
	{PM_API(PM_FPGA_GET_STATUS), PMU_API_FULL_ACCESS},
#if defined(ENABLE_FPGA_READ_CONFIG_DATA) || defined(ENABLE_FPGA_READ_CONFIG_REG)
	{PM_API(PM_FPGA_READ), PMU_API_FULL_ACCESS},
#endif
#if defined(XFPGA_GET_VERSION_INFO)
	{PM_API(PM_FPGA_GET_VERSION), PMU_API_FULL_ACCESS},
#endif
#if defined(XFPGA_GET_FEATURE_LIST)
	{PM_API(PM_FPGA_GET_FEATURE_LIST), PMU_API_FULL_ACCESS},
#endif
#endif
	/* Add more entries as needed */
};

#define NUM_DEFAULT_APIS (sizeof(defaultApiPermissions) / sizeof(defaultApiPermissions[0]))

/**
 * readTrustzoneBit - Reads the TrustZone bit for a given IPI channel.
 *
 * This function reads the TrustZone configuration bit from the
 * APERPERM register corresponding to the specified IPI channel.
 *
 * @ipiMask	 IPI channel number to read from.
 * @trustzoneBit Pointer to store the retrieved TrustZone bit value
 *		 (0 = secure, 1 = non-ecure).
 *
 * @return
 *  - 0 on success or error code on failure.
 */
s32 readTrustzoneBit(uint32_t ipiMask, uint32_t *trustzoneBit)
{
	s32 status = XST_FAILURE;
	uint32_t regAddr;
	uint32_t regVal;
	uint32_t channel;

	/* Mapping IPI channel with aperture permission registers */
	switch (ipiMask) {
		case IPI_PMU_0_IER_APU_MASK:
			channel = 0U;
			break;
		case IPI_PMU_0_IER_RPU_0_MASK:
			channel = 1U;
			break;
		case IPI_PMU_0_IER_RPU_1_MASK:
			channel = 2U;
			break;
		case IPI_PMU_0_IER_PMU_0_MASK:
			channel = 3U;
			break;
		default:
			status = XST_INVALID_PARAM;
			goto done;
	}

	regAddr = APERPERM_48_REG_BASE_ADDR + (channel * APERPERM_48_REG_OFFSET);
	regVal = XPfw_Read32(regAddr);

	*trustzoneBit = (regVal >> APERPERM_48_REG_TRUSTZONE_BIT_INDEX) & 0x1U;
	status = XST_SUCCESS;

done:
	return status;
}

/**
 * setApiPermission - Sets the access permission for a specific API.
 *
 * Updates the permission bitmap entry corresponding to the given
 * API ID with the specified permission value.
 *
 * @apiId	Unique identifier of the API.
 * @permission	Permission value to set.
 */
static void setApiPermission(const uint32_t apiId, const uint32_t permission)
{
	uint32_t index;
	uint32_t wordIndex;
	uint32_t bitOffset;

	index = (apiId - 1U) * BITS_PER_API;
	wordIndex = index / WORD_SIZE_BITS;
	bitOffset = index % WORD_SIZE_BITS;

	if ((permission & PMU_API_SECURE_ACCESS) != 0U) {
		apiPermissionBitmap[wordIndex] |= (1UL << (bitOffset + 0U));
	}

	if ((permission & PMU_API_NON_SECURE_ACCESS) != 0U) {
		apiPermissionBitmap[wordIndex] |= (1UL << (bitOffset + 1U));
	}
}

/**
 * getApiPermission - Retrieves the access permission for a specific API.
 *
 * Looks up the permission bitmap entry for the given API ID and
 * returns its current permission value.
 *
 * @apiId  Unique identifier of the API.
 *
 * @return Permission value for the specified API.
 */
static uint32_t getApiPermission(const uint32_t apiId)
{
	uint32_t index;
	uint32_t wordIndex;
	uint32_t bitOffset;

	index = (apiId - 1U) * BITS_PER_API;
	wordIndex = index / WORD_SIZE_BITS;
	bitOffset = index % WORD_SIZE_BITS;

	return ((apiPermissionBitmap[wordIndex] >> bitOffset) & PMU_API_FULL_ACCESS);
}

/**
 * isApiAllowed - Checks if an API is allowed for the caller based on security state.
 *
 * Determines whether the given API ID can be accessed by the caller,
 * depending on whether the caller is operating in secure or non-secure mode.
 *
 * @apiId	   Unique identifier of the API.
 * @isCallerSecure true if the caller is secure, false if non-secure.
 *
 * @return
 *  - true if the API is allowed for the caller.
 *  - false if the API is not allowed.
 */
uint32_t isApiAllowed(const uint32_t apiId, const uint32_t isCallerSecure)
{
	uint32_t permission;
	uint32_t isAllowed;

	permission = getApiPermission(apiId);
	if (isCallerSecure != 0U) {
		isAllowed = (uint32_t)((permission & PMU_API_SECURE_ACCESS) != 0U);
	}
	else {
		isAllowed = (uint32_t)((permission & PMU_API_NON_SECURE_ACCESS) != 0U);
	}

	return isAllowed;
}

/**
 * initApiPermissions - Initializes the API permission table with default values.
 *
 * Populates the permission bitmap by setting each API's permission
 * according to the entries in defaultApiPermissions.
 */
static void initApiPermissions(void)
{
	uint32_t i;

	for (i=0; i < NUM_DEFAULT_APIS; i++) {
		setApiPermission(defaultApiPermissions[i].apiId,
				 defaultApiPermissions[i].permissionFlags);
	}
}

/**
 * extractSecureFlagTfa - Extracts the secure flag from a given API ID.
 *
 * @apiId API identifier containing the secure flag in bit 24.
 *
 * @return
 * - 0: Secure flag.
 * - 1: Non secure flag.
 */
uint32_t extractSecureFlagTfa(const uint32_t apiId)
{
	uint32_t secureFlag;

	secureFlag = (apiId & SECURE_FLAG_BIT_MASK) >> SECURE_FLAG_BIT_SHIFT;

	return secureFlag;
}
#endif

/**
 * XPfw_PmInit() - initializes PM firmware
 *
 * @note	Call on startup to initialize PM firmware.
 */
void XPfw_PmInit(void)
{
#ifdef ENABLE_POS
	u32 bootType = PmHookGetBootType();

	/* Call user hook for Power Off Suspend initialization */
	PmHookInitPowerOffSuspend();
#else
	u32 bootType = PM_COLD_BOOT;
#endif

	PmInfo("Power Management Init\r\n");

	if (bootType == PM_COLD_BOOT) {
		PmMasterDefaultConfig();
		PmNodeConstruct();
	}

#ifdef ENABLE_CSU_MULTIBOOT
	/* Store Multiboot register value */
	CsuMultibootRegVal = XPfw_Read32(CSU_MULTI_BOOT);
#endif

#ifdef ENABLE_SECURE_FLAG
	initApiPermissions();
#endif
}

/**
 * XPfw_PmIpiHandler() - Call from IPI interrupt handler to process PM API call
 * @IsrMask IPI's ISR register value. Needed to determine the source master
 *
 * @Payload  Pointer to IPI Payload
 *
 * @Len Size of the payload in words
 *
 * @return  Status of the processing IPI
 *          - XST_INVALID_PARAM if input parameters have invalid value
 *          - XST_SUCCESS otherwise
 *          - Note that if request is processed, firmware is not receiving any
 *            status of processing information. Processing status is returned to
 *            the master which initiated communication through IPI.
 *
 */
s32 XPfw_PmIpiHandler(const u32 IsrMask, const u32* Payload, u8 Len)
{
	s32 status = XST_SUCCESS;
	PmMaster* master = PmGetMasterByIpiMask(IsrMask);

	if ((NULL == Payload) || (NULL == master) || (Len < PAYLOAD_ELEM_CNT)) {
		/* Never happens if IPI irq handler is implemented correctly */
		PmErr("Unknown IPI %lu\r\n", IsrMask);
		status = XST_INVALID_PARAM;
		goto done;
	}

	PmProcessRequest(master, Payload);

done:
	return status;
}

/**
 * XPfw_PmWfiHandler() - Call from GPI2 interrupt handler to process sleep req
 * @srcMask Value read from GPI2 register which determines master requestor
 *
 * @return  Status of triggering sleep for a processor (XST_INVALID_PARAM if
 *          processor cannot be determined by srcMask, status of performing
 *          sleep operation otherwise)
 *
 * @note    Call from GPI2 interrupt routine to process sleep request. Must not
 *          clear GPI2 interrupt before this function returns.
 */
s32 XPfw_PmWfiHandler(const u32 srcMask)
{
	s32 status;
	PmProc *proc = PmGetProcByWfiStatus(srcMask);

	if (NULL == proc) {
		PmErr("Unknown processor 0x%lx\r\n", srcMask);
		status = XST_INVALID_PARAM;
		goto done;
	}

	status = PmProcFsm(proc, PM_PROC_EVENT_SLEEP);

done:
	return status;
}

/**
 * XPfw_PmWakeHandler() - Call from GPI1 interrupt to process wake request
 * @srcMask     Value read from GPI1 register which determines interrupt source
 *
 * @return      Status of performing wake-up (XST_INVALID_PARAM if wake is a
 *              processor wake event but processor is not found, status of
 *              performing wake otherwise)
 *
 * @note    Call from GPI1 interrupt routine to process wake request. Must not
 *          clear GPI1 interrupt before this function returns.
 *          If the wake source is one of GIC wakes, source of the interrupt
 *          (peripheral that actually generated interrupt to GIC) cannot be
 *          determined, and target should be immediately woken-up (target is
 *          processor whose GIC wake bit is set in srcMask). If the wake is the
 *          FPD GIC Proxy interrupt, the APU needs to be woken up.
 */
s32 XPfw_PmWakeHandler(const u32 srcMask)
{
	s32 status = XST_INVALID_PARAM;

#if defined(PMU_MIO_INPUT_PIN) && (PMU_MIO_INPUT_PIN >= 0U) \
				&& (PMU_MIO_INPUT_PIN <= 5U)
	if ((PMU_IOMODULE_GPI1_MIO_WAKE_0_MASK << PMU_MIO_INPUT_PIN) == srcMask) {
		PmShutdownInterruptHandler();
		return XST_SUCCESS;
	}
#endif
	if (0U != (PMU_IOMODULE_GPI1_GIC_WAKES_ALL_MASK & srcMask))  {
		/* Processor GIC wake */
		PmProc* proc = PmProcGetByWakeMask(srcMask);
		if ((NULL != proc) && (NULL != proc->master)) {
			status = PmMasterWakeProc(proc);
		} else {
			status = XST_INVALID_PARAM;
		}
	} else if (0U != (PMU_IOMODULE_GPI1_FPD_WAKE_GIC_PROXY_MASK & srcMask)) {
		status = PmMasterWake(&pmMasterApu_g);
	} else if (0U != (PMU_IOMODULE_GPI1_MIO_WAKE_ALL_MASK & srcMask)) {
		status = PmExternWakeMasters();
	} else if (0U != (PMU_IOMODULE_GPI1_USB_0_WAKE_MASK & srcMask)) {
		status = PmWakeMasterBySlave(&pmSlaveUsb0_g.slv);
	} else if (0U != (PMU_IOMODULE_GPI1_USB_1_WAKE_MASK & srcMask)) {
		status = PmWakeMasterBySlave(&pmSlaveUsb1_g.slv);
	} else {
		/* For MISRA compliance */
	}

	return status;
}

/**
 * XPfw_PmCheckIpiRequest() - Check whether the IPI interrupt is a PM call
 * @isrVal  IPI's ISR register value
 * @apiId   Pointer to a variable holding the api id (first word of message)
 *
 * @return  Check result
 *
 * @note    Call from IPI interrupt routine to check is interrupt a PM call.
 *          Function reads first argument of payload in IPI buffer of
 *          requestor master to determine whether first argument is within
 *          PM API regular ids.
 */
XPfw_PmIpiStatus XPfw_PmCheckIpiRequest(const u32 isrVal,
					const u32* apiId)
{
	XPfw_PmIpiStatus status;
	const PmMaster *master = PmGetMasterByIpiMask(isrVal);

	if (NULL == master) {
		/* IPI is not generated by one of the PM supported PUs */
		status = XPFW_PM_IPI_SRC_UNKNOWN;
		goto done;
	}

	/* Api id is first argument in payload */
	if (((*apiId & 0xFFU) > PM_API(PM_API_MIN)) && ((*apiId & 0xFFU) < PM_API(PM_API_MAX))) {
		/* Api id is within valid range */
		status = XPFW_PM_IPI_IS_PM_CALL;
	} else {
		/* This IPI was not a PM call */
		status = XPFW_PM_IPI_NOT_PM_CALL;
	}

done:
	return status;
}

/**
 * XPfw_DapFpdWakeEvent() - Inform PM about the FPD DAP wake event
 */
void XPfw_DapFpdWakeEvent(void)
{
	if (0U != (XPfw_Read32(PMU_GLOBAL_PWR_STATE) &
		  PMU_GLOBAL_PWR_STATE_FP_MASK)) {
		pmPowerDomainFpd_g.power.node.currState = PM_PWR_STATE_ON;
	}
}

/**
 * XPfw_DapRpuWakeEvent() - Inform PM about the RPU DAP wake event
 */
void XPfw_DapRpuWakeEvent(void)
{
	if (0U != (XPfw_Read32(PMU_GLOBAL_PWR_STATE) &
		  PMU_GLOBAL_PWR_STATE_R5_0_MASK)) {
		pmPowerIslandRpu_g.power.node.currState = PM_PWR_STATE_ON;
	}
}

#endif
