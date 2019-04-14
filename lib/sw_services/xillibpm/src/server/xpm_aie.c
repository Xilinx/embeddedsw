/******************************************************************************
*
* Copyright (C) 2019 Xilinx, Inc.  All rights reserved.
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
*
******************************************************************************/

#include "xpm_common.h"
#include "xpm_aie.h"
#include "xpm_regs.h"
#include "xpm_bisr.h"

#define ME_PCSR_KEY 0xF9E8D7C6U
#define AIE_POLL_TIMEOUT 0X1000000U

#define ME_NPI_BASEADDR      0xF70A0000U
#define ME_NPI_REG_PCSR_MASK    ( ( ME_NPI_BASEADDR ) + 0x00000000U )
#define ME_NPI_REG_PCSR_CONTROL    ( ( ME_NPI_BASEADDR ) + 0x00000004U )
#define ME_NPI_REG_PCSR_STATUS    ( ( ME_NPI_BASEADDR ) + 0x00000008U )
#define ME_NPI_REG_PCSR_LOCK    ( ( ME_NPI_BASEADDR ) + 0x0000000CU )

#define ME_NPI_REG_PCSR_STATUS_ME_PWR_SUPPLY_MASK    0x00008000U
#define ME_NPI_REG_PCSR_STATUS_SCAN_CLEAR_DONE_MASK    0x00000002U
#define ME_NPI_REG_PCSR_STATUS_SCAN_CLEAR_PASS_MASK    0x00000004U

#define ME_NPI_REG_PCSR_MASK_ME_ARRAY_RESET_MASK    0x04000000U
#define ME_NPI_REG_PCSR_MASK_INITSTATE_MASK    0x00000040U

#define ME_NPI_REG_PCSR_MASK_ME_IPOR_MASK    0x01000000U
#define ME_NPI_REG_PCSR_MASK_SCAN_CLEAR_TRIGGER_MASK    0x00000800U
#define ME_NPI_REG_PCSR_MASK_GATEREG_MASK    0x00000002U
#define ME_NPI_REG_PCSR_MASK_PCOMPLETE_MASK    0x00000001U

#define ME_NPI_REG_PCSR_MASK_ODISABLE_SHIFT   2U
#define ME_NPI_REG_PCSR_MASK_ODISABLE_0_MASK  (1U << (ME_NPI_REG_PCSR_MASK_ODISABLE_SHIFT + 0))
#define ME_NPI_REG_PCSR_MASK_ODISABLE_1_MASK  (1U << (ME_NPI_REG_PCSR_MASK_ODISABLE_SHIFT + 1))


static void AiePcsrWrite(u32 Mask, u32 Value)
{
	PmOut32(ME_NPI_REG_PCSR_MASK, Mask);
	PmOut32(ME_NPI_REG_PCSR_CONTROL, Value);
}


static XStatus AiePreHouseclean(u32 *Args, u32 NumOfArgs)
{
	XStatus Status = XST_SUCCESS;

	/* This function does not use the args */
	(void)Args;
	(void)NumOfArgs;

	/* Check for ME Power Status */
	if( (XPm_In32(ME_NPI_REG_PCSR_STATUS) &
			 ME_NPI_REG_PCSR_STATUS_ME_PWR_SUPPLY_MASK) !=
			 ME_NPI_REG_PCSR_STATUS_ME_PWR_SUPPLY_MASK) {
		Status = XST_FAILURE;
		goto done;
	}

	/* Unlock ME PCSR */
	PmOut32(ME_NPI_REG_PCSR_LOCK, ME_PCSR_KEY);

	/* Relelase IPOR */
	AiePcsrWrite(ME_NPI_REG_PCSR_MASK_ME_IPOR_MASK, 0U);

	/* TODO: Configure TOP_ROW and ROW_OFFSET by reading from EFUSE */

done:
	return Status;
}

static XStatus AiePostHouseclean(u32 *Args, u32 NumOfArgs)
{
	XStatus Status = XST_SUCCESS;

	/* This function does not use the args */
	(void)Args;
	(void)NumOfArgs;

	/* Set PCOMPLETE bit */
	AiePcsrWrite(ME_NPI_REG_PCSR_MASK_PCOMPLETE_MASK,
		 ME_NPI_REG_PCSR_MASK_PCOMPLETE_MASK);
	/* TODO: Check if we can lock PCSR registers here */

	return Status;
}

static XStatus AieScanClear(u32 *Args, u32 NumOfArgs)
{
	XStatus Status = XST_SUCCESS;

	/* This function does not use the args */
	(void)Args;
	(void)NumOfArgs;

	/* De-assert ODISABLE[1] */
	AiePcsrWrite(ME_NPI_REG_PCSR_MASK_ODISABLE_1_MASK, 0U);

	if (PLATFORM_VERSION_SILICON == Platform) {
		/* Trigger Scan Clear */
		AiePcsrWrite(ME_NPI_REG_PCSR_MASK_SCAN_CLEAR_TRIGGER_MASK,
			     ME_NPI_REG_PCSR_MASK_SCAN_CLEAR_TRIGGER_MASK);

		/* Wait for Scan Clear DONE */
		Status = XPm_PollForMask(ME_NPI_REG_PCSR_STATUS,
					 ME_NPI_REG_PCSR_STATUS_SCAN_CLEAR_DONE_MASK,
					 AIE_POLL_TIMEOUT);
		if (XST_SUCCESS != Status) {
			goto done;
		}

		/* Check Scan Clear PASS */
		if( (XPm_In32(ME_NPI_REG_PCSR_STATUS) &
		     ME_NPI_REG_PCSR_STATUS_SCAN_CLEAR_PASS_MASK) !=
		    ME_NPI_REG_PCSR_STATUS_SCAN_CLEAR_PASS_MASK) {
			Status = XST_FAILURE;
			goto done;
		}
	}

	/* De-assert ODISABLE[0] */
	AiePcsrWrite(ME_NPI_REG_PCSR_MASK_ODISABLE_0_MASK, 0U);

	/* De-assert GATEREG */
	AiePcsrWrite(ME_NPI_REG_PCSR_MASK_GATEREG_MASK, 0U);

done:
	return Status;
}

static XStatus AieBisr(u32 *Args, u32 NumOfArgs)
{
	XStatus Status = XST_SUCCESS;

	/* This function does not use the args */
	(void)Args;
	(void)NumOfArgs;

	Status = XPmBisr_Repair(MEA_TAG_ID);
	if (XST_SUCCESS != Status) {
                goto done;
        }
	Status = XPmBisr_Repair(MEB_TAG_ID);
	if (XST_SUCCESS != Status) {
                goto done;
        }
	Status = XPmBisr_Repair(MEC_TAG_ID);
	if (XST_SUCCESS != Status) {
                goto done;
        }
done:
	return Status;
}

static XStatus AieMbistClear(u32 *Args, u32 NumOfArgs)
{
	XStatus Status = XST_SUCCESS;

	/* TODO: Implement MBIST  */

	/* This function does not use the args */
	(void)Args;
	(void)NumOfArgs;

        return Status;
}

static XStatus AieMemInit(u32 *Args, u32 NumOfArgs)
{
	XStatus Status = XST_SUCCESS;

        /* This function does not use the args */
	(void)Args;
	(void)NumOfArgs;

	/* TODO: Implement memory zeroization */
        return Status;
}

struct XPm_PowerDomainOps AieOps = {
	.PreHouseClean = AiePreHouseclean,
	.PostHouseClean = AiePostHouseclean,
	.ScanClear = AieScanClear,
	.Bisr = AieBisr,
	.Mbist = AieMbistClear,
	.MemInit = AieMemInit,
};

XStatus XPmAieDomain_Init(XPm_AieDomain *AieDomain, u32 Id, u32 BaseAddress,
			   XPm_Power *Parent)
{
	XPmPowerDomain_Init(&AieDomain->Domain, Id, BaseAddress, Parent, &AieOps);
	return XST_SUCCESS;
}
