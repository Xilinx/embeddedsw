/******************************************************************************
* Copyright (c) 2024 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

#include "xpm_gic_proxy.h"
#include "xpm_pmc.h"
#include "xpm_device.h"
#include "xpm_common.h"
#include "xpm_regs.h"
#include "xpm_update.h"
#include "xil_sutil.h"

#define XPM_GIC_PROXY_IS_ENABLED		0x1U

/**
 * @brief  Invoke an address helper twice and compare computed addresses.
 *
 * @param  Status        Status from the first helper call.
 * @param  StatusTmp     Status from the second helper call.
 * @param  RegAddress    Address from the first helper call.
 * @param  RegAddressTmp Address from the second helper call.
 * @param  Function      Address helper function to invoke redundantly.
 * @param  ...           Arguments forwarded to Function before output pointer.
 */
#define XPM_GIC_PROXY_REDUNDANT_ADDR(Status, StatusTmp, RegAddress,	\
				     RegAddressTmp, Function, ...)	\
	do {								\
		(Status) = Function(__VA_ARGS__, &(RegAddress));	\
		(StatusTmp) = Function(__VA_ARGS__, &(RegAddressTmp));	\
		if ((XST_SUCCESS != (Status)) ||			\
		    (XST_SUCCESS != (StatusTmp)) ||			\
		    ((RegAddress) != (RegAddressTmp))) {		\
			(Status) = XST_GLITCH_ERROR;			\
			goto done;					\
		}							\
	} while (0)

/**
 * @brief  Compute a GIC Proxy group register address.
 *
 * @param  BaseAddress  PMC Global base address.
 * @param  Group        GIC Proxy group index.
 * @param  RegOffset    Per-group register offset.
 * @param  RegAddress   Pointer to store computed register address.
 *
 * @return XST_SUCCESS.
 */
static XStatus XPmGicProxy_GetGroupRegAddr(u32 BaseAddress, u32 Group,
					   u32 RegOffset,
					   volatile u32 *RegAddress)
					   __attribute__((noinline));

/**
 * @brief  Compute a top-level GIC Proxy register address.
 *
 * @param  BaseAddress  PMC Global base address.
 * @param  RegOffset    Top-level GIC Proxy register offset.
 * @param  RegAddress   Pointer to store computed register address.
 *
 * @return XST_SUCCESS.
 */
static XStatus XPmGicProxy_GetTopRegAddr(u32 BaseAddress, u32 RegOffset,
					 volatile u32 *RegAddress)
					 __attribute__((noinline));

/**
 * @brief  Define the noinline GIC Proxy group register address helper.
 *
 * @param  BaseAddress  PMC Global base address.
 * @param  Group        GIC Proxy group index.
 * @param  RegOffset    Per-group register offset.
 * @param  RegAddress   Pointer to store computed register address.
 *
 * @return XST_SUCCESS.
 */
static XStatus XPmGicProxy_GetGroupRegAddr(u32 BaseAddress, u32 Group,
					   u32 RegOffset,
					   volatile u32 *RegAddress)
{
	*RegAddress = BaseAddress + PMC_GLOBAL_GIC_PROXY_BASE_OFFSET +
		      GIC_PROXY_GROUP_OFFSET(Group) + RegOffset;

	return XST_SUCCESS;
}

/**
 * @brief  Define the noinline top-level GIC Proxy register address helper.
 *
 * @param  BaseAddress  PMC Global base address.
 * @param  RegOffset    Top-level GIC Proxy register offset.
 * @param  RegAddress   Pointer to store computed register address.
 *
 * @return XST_SUCCESS.
 */
static XStatus XPmGicProxy_GetTopRegAddr(u32 BaseAddress, u32 RegOffset,
					 volatile u32 *RegAddress)
{
	*RegAddress = BaseAddress + RegOffset;

	return XST_SUCCESS;
}

/**
 * @brief  Enable or disable a wake event in the GIC Proxy for a peripheral.
 *
 * When enabling, clears any pending IRQ for the peripheral in the GIC Proxy
 * status register and remembers the wake-source bit so a later call to
 * XPm_GicProxy.Enable() can program the IRQ-enable register. When disabling,
 * the wake-source bit is cleared in the cached SetMask only.
 *
 * @param  Periph  Pointer to the peripheral whose wake event is configured.
 * @param  Enable  Non-zero to remember the wake source and clear pending IRQ;
 *                 zero to clear the cached wake-source bit.
 *
 * @return XST_SUCCESS on success, error code on failure.
 */
XStatus XPmGicProxy_WakeEventSet(const XPm_Periph *Periph, u8 Enable)
{
	volatile XStatus Status = XST_FAILURE;
	volatile XStatus StatusTmp = XST_FAILURE;
	volatile u32 RegAddress = 0U;
	volatile u32 RegAddressTmp = 0U;
	u32 GicProxyMask = Periph->GicProxyMask;
	u32 GicProxyGroup = Periph->GicProxyGroup;
	u32 BaseAddress;
	const XPm_Pmc *Pmc = (XPm_Pmc *)XPmDevice_GetById(PM_DEV_PMC_PROC);

	if (GicProxyGroup >= XPm_GicProxy.GroupsCnt) {
		Status = XST_INVALID_PARAM;
		goto done;
	}

	if (0U == Enable) {
		XPm_GicProxy.Groups[GicProxyGroup].SetMask &= ~GicProxyMask;
		Status = XST_SUCCESS;
		goto done;
	}

	/* PMC Global base address */
	BaseAddress = Pmc->PmcGlobalBaseAddr;

	XPM_GIC_PROXY_REDUNDANT_ADDR(Status, StatusTmp, RegAddress,
				     RegAddressTmp, XPmGicProxy_GetGroupRegAddr,
				     BaseAddress, GicProxyGroup,
				     GIC_PROXY_IRQ_STATUS_OFFSET);

	/* Write 1 into status register to Clear interrupt */
	XPm_Out32(RegAddress, GicProxyMask);

	/* Remember which interrupt in the group needs to be Enabled */
	XPm_GicProxy.Groups[GicProxyGroup].SetMask |= GicProxyMask;

	Status = XST_SUCCESS;
done:
	return Status;
}

/**
 * @brief  Enable all interrupts requested as GIC Proxy wake sources.
 *
 * @return XST_SUCCESS on success, error code on failure.
 */
static XStatus XPmGicProxy_Enable(void)
{
	volatile XStatus Status = XST_FAILURE;
	volatile XStatus StatusTmp = XST_FAILURE;
	volatile u32 RegAddress = 0U;
	volatile u32 RegAddressTmp = 0U;
	u32 g;
	const XPm_Pmc *Pmc = (XPm_Pmc *)XPmDevice_GetById(PM_DEV_PMC_PROC);

	for (g = 0U; g < XPm_GicProxy.GroupsCnt; g++) {
		/* PMC Global base address */
		u32 BaseAddress = Pmc->PmcGlobalBaseAddr;

		XPM_GIC_PROXY_REDUNDANT_ADDR(Status, StatusTmp, RegAddress,
					     RegAddressTmp,
					     XPmGicProxy_GetGroupRegAddr,
					     BaseAddress, g,
					     GIC_PROXY_IRQ_ENABLE_OFFSET);

		/* Enable interrupts in the group that are set as wake */
		XPm_Out32(RegAddress, XPm_GicProxy.Groups[g].SetMask);

		if (0U != XPm_GicProxy.Groups[g].SetMask) {
			XPM_GIC_PROXY_REDUNDANT_ADDR(Status, StatusTmp,
						     RegAddress, RegAddressTmp,
						     XPmGicProxy_GetTopRegAddr,
						     BaseAddress,
						     PMC_GLOBAL_GICP_IRQ_ENABLE_OFFSET);

			XPm_Out32(RegAddress, BIT32(g));
		}
	}

	XPm_GicProxy.Flags |= XPM_GIC_PROXY_IS_ENABLED;
	Status = XST_SUCCESS;
done:
	return Status;
}

/**
 * @brief  Disable all interrupts in the GIC Proxy.
 *
 * @return XST_SUCCESS on success, error code on failure.
 */
static XStatus XPm_GicProxyDisable(void)
{
	volatile XStatus Status = XST_FAILURE;
	volatile XStatus StatusTmp = XST_FAILURE;
	volatile u32 RegAddress = 0U;
	volatile u32 RegAddressTmp = 0U;
	volatile u32 MaskAddr = 0U;
	volatile u32 MaskAddrTmp = 0U;
	u32 g;
	const XPm_Pmc *Pmc = (XPm_Pmc *)XPmDevice_GetById(PM_DEV_PMC_PROC);

	for (g = 0U; g < XPm_GicProxy.GroupsCnt; g++) {
		/* PMC Global base address */
		u32 BaseAddress = Pmc->PmcGlobalBaseAddr;

		XPM_GIC_PROXY_REDUNDANT_ADDR(Status, StatusTmp, MaskAddr,
					     MaskAddrTmp,
					     XPmGicProxy_GetGroupRegAddr,
					     BaseAddress, g,
					     GIC_PROXY_IRQ_MASK_OFFSET);

		/* Clear interrupts in the GIC Proxy group that are set as wake */
		XPM_GIC_PROXY_REDUNDANT_ADDR(Status, StatusTmp, RegAddress,
					     RegAddressTmp,
					     XPmGicProxy_GetGroupRegAddr,
					     BaseAddress, g,
					     GIC_PROXY_IRQ_STATUS_OFFSET);

		XPm_Out32(RegAddress, XPm_GicProxy.Groups[g].SetMask);

		/* Disable interrupts in the GIC Proxy group that are set as wake */
		XPM_GIC_PROXY_REDUNDANT_ADDR(Status, StatusTmp, RegAddress,
					     RegAddressTmp,
					     XPmGicProxy_GetGroupRegAddr,
					     BaseAddress, g,
					     GIC_PROXY_IRQ_DISABLE_OFFSET);

		XPm_Out32(RegAddress, XPm_GicProxy.Groups[g].SetMask);

		if (GIC_PROXY_ALL_MASK == XPm_In32(MaskAddr)) {
			XPM_GIC_PROXY_REDUNDANT_ADDR(Status, StatusTmp,
						     RegAddress, RegAddressTmp,
						     XPmGicProxy_GetTopRegAddr,
						     BaseAddress,
						     PMC_GLOBAL_GICP_IRQ_DISABLE_OFFSET);

			XPm_Out32(RegAddress, BIT32(g));
		}
	}

	XPm_GicProxy.Flags &= (u8)(~XPM_GIC_PROXY_IS_ENABLED);
	Status = XST_SUCCESS;
done:
	return Status;
}

/**
 * XPmGicProxy_Clear() - Clear wake-up sources
 */
static void XPmGicProxy_Clear(void)
{
	u32 g;

	if (0U != (XPm_GicProxy.Flags & XPM_GIC_PROXY_IS_ENABLED)) {
		XStatus Status = XPm_GicProxyDisable();
		if (XST_SUCCESS != Status) {
			PmErr("0x%x\n\r", Status);
		}
	}

	for (g = 0U; g < XPm_GicProxy.GroupsCnt; g++) {
		XPm_GicProxy.Groups[g].SetMask = 0U;
	}
}

/* FPD GIC Proxy has interrupts organized in 5 Groups */
static XPm_GicProxyGroup XPm_GicProxyGroups[5] XPM_INIT_DATA(XPm_GicProxyGroups);

XPm_GicProxy_t XPm_GicProxy = {
	.Groups = XPm_GicProxyGroups,
	.GroupsCnt = ARRAY_SIZE(XPm_GicProxyGroups),
	.Clear = XPmGicProxy_Clear,
	.Enable = XPmGicProxy_Enable,
	.Flags = 0U,
};
