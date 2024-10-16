/******************************************************************************
* Copyright (c) 2023 - 2024 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

#include "xpm_aie.h"
#include "xpm_debug.h"

static XStatus Aie2InitStart(XPm_PowerDomain *PwrDomain, const u32 *Args,
		u32 NumOfArgs)
{
	(void)PwrDomain;
	(void)Args;
	(void)NumOfArgs;
	/* TODO Adding a place holder.
	 * Need to Implement while adding AIE support to xilpm */
	return XST_SUCCESS;
}

static XStatus Aie2InitFinish(const XPm_PowerDomain *PwrDomain, const u32 *Args,
		u32 NumOfArgs)
{
	(void)PwrDomain;
	(void)Args;
	(void)NumOfArgs;
	/* TODO Adding a place holder.
	 * Need to Implement while adding AIE support to xilpm */
	return XST_SUCCESS;
}

static struct XPm_PowerDomainOps AieOps = {
	.InitStart = Aie2InitStart,
	.InitFinish = Aie2InitFinish
};

XStatus XPmAieDomain_Init(XPm_AieDomain *AieDomain, u32 Id, u32 BaseAddress,
			  XPm_Power *Parent, const u32 *Args, u32 NumArgs)
{
	XStatus Status = XST_FAILURE;
	u32 Platform = XPm_GetPlatform();
	const struct XPm_PowerDomainOps *Ops = NULL;
	u16 DbgErr = XPM_INT_ERR_UNDEFINED;
	u16 ArrayInfoPresent = 0U;

	(void)Args;
	(void)NumArgs;
	XPm_AieArray *Array = &AieDomain->Array;
	Ops = &AieOps;

	if (PM_POWER_ME2 == Id) {
		if (PLATFORM_VERSION_SPP == Platform) {
			/* Non-Silicon defaults for SPP/EMU for AIE2PS */
			Array->NumCols = 7U;
			Array->NumRows = 5U;
			Array->StartCol = 6U;
			Array->StartRow = 1U;
			Array->NumShimRows = 1U;
			Array->NumAieRows = Array->NumRows - Array->NumMemRows;
			Array->GenVersion = AIE_GENV2;
			Array->LColOffset = 0U;		/**< left col offset = 0 */
			Array->RColOffset = 0U;		/**< right col offset = 0 */
			Array->TRowOffset = 0U;		/**< top row offset = 0 */
			/* Skip this info from topology in this case */
			ArrayInfoPresent = 1U;
			Ops = &AieOps;
		}
	} else {
		DbgErr = XPM_INT_ERR_INVALID_PWR_DOMAIN;
		Status = XPM_INVALID_PWRDOMAIN;
		goto done;
	}

	Array->NocAddress = (u64)VIVADO_ME_BASEADDR;

	/* Read AIE array geometry info from topology if available */
	if ((3U <= NumArgs) && (1U != ArrayInfoPresent)) {
	        Array->GenVersion = ARR_GENV(Args[0]);
		Array->NumRows = ARR_ROWS(Args[1]);
		Array->NumCols = ARR_COLS(Args[1]);
		Array->NumAieRows = ARR_AIEROWS(Args[2]);
		Array->NumMemRows = ARR_MEMROWS(Args[2]);
		Array->NumShimRows = ARR_SHMROWS(Args[2]);
		Array->StartCol = 0U;                   /**< always start from first column */
		Array->StartRow = Array->NumShimRows;   /**< always start after shim row */

		if (3U < NumArgs) {
		        Array->LColOffset = ARR_LCOLOFF(Args[3]);
		        Array->RColOffset = ARR_RCOLOFF(Args[3]);
		        Array->TRowOffset = ARR_TROWOFF(Args[3]);
		} else {
		        Array->LColOffset = 0U;         /**< left col offset = 0 */
		        Array->RColOffset = 0U;         /**< right col offset = 0 */
		        Array->TRowOffset = 0U;         /**< top row offset = 0 */
		}
	}

	/* Derive row and col ranges after offset adjustments */
	Array->StartCol += Array->LColOffset;
	Array->NumColsAdjusted = Array->NumCols - (u16)(Array->LColOffset + Array->RColOffset);
	Array->NumRowsAdjusted = Array->NumRows - Array->TRowOffset;

	Status = XPmPowerDomain_Init(&AieDomain->Domain, Id, BaseAddress,
			Parent, Ops);

	if (XST_SUCCESS != Status) {
		DbgErr = XPM_INT_ERR_POWER_DOMAIN_INIT;
	}

done:
	XPm_PrintDbgErr(Status, DbgErr);
	return Status;
}
