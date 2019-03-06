/******************************************************************************
*
* Copyright (C) 2018 Xilinx, Inc.  All rights reserved.
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
#include "xillibpm_defs.h"
#include "xpm_domain_iso.h"
#include "xpm_common.h"
#include "xpm_node.h"
#include "xpm_powerdomain.h"
#include "xpm_reset.h"

XStatus XPmPowerDomain_Init(XPm_PowerDomain *PowerDomain, u32 Id,
			    u32 BaseAddress, XPm_Power *Parent,
			    struct XPm_PowerDomainOps *Ops)
{
	XPmPower_Init(&PowerDomain->Power, Id, BaseAddress, Parent);

	PowerDomain->Children = NULL;
	PowerDomain->DomainOps = Ops;

	return XST_SUCCESS;
}

XStatus XPm_PowerUpLPD(XPm_Node *Node)
{
	XStatus Status = XST_SUCCESS;

	if (XPM_POWER_STATE_ON == Node->State) {
		goto done;
	} else {
		/* TODO: Right now it is expected that CDO will trigger init node
		 * commands to power up LPD so here LPD status shoul always be ON.
		 * Later support can be added to power up and house clean LPD
		 * here to make it optional for CDO
		 */
		Status = XST_FAILURE;
	}

done:
	return Status;
}

XStatus XPm_PowerDwnLPD()
{
	XStatus Status = XST_SUCCESS;

	/* Isolate PS_PL */
	Status = XPm_DomainIsoEnable(XPM_DOMAIN_ISO_LPD_PL_TEST);
	if (Status != XST_SUCCESS)
		goto done;

	Status = XPm_DomainIsoEnable(XPM_DOMAIN_ISO_LPD_PL);
	if (Status != XST_SUCCESS)
		goto done;

	/* Isolate PS_CPM domains */
	Status = XPm_DomainIsoEnable(XPM_DOMAIN_ISO_LPD_CPM_DFX);
	if (Status != XST_SUCCESS)
		goto done;

	Status = XPm_DomainIsoEnable(XPM_DOMAIN_ISO_LPD_CPM);
	if (Status != XST_SUCCESS)
		goto done;

	/* Isolate FP-SOC */
	Status = XPm_DomainIsoEnable(XPM_DOMAIN_ISO_FPD_SOC);
	if (Status != XST_SUCCESS)
		goto done;

	/* Isolate LP-SoC */
	Status = XPm_DomainIsoEnable(XPM_DOMAIN_ISO_LPD_SOC);
	if (Status != XST_SUCCESS)
		goto done;

	/* Isolate PS_PMC domains */
	Status = XPm_DomainIsoEnable(XPM_DOMAIN_ISO_PMC_LPD_DFX);
	if (Status != XST_SUCCESS)
		goto done;

	Status = XPm_DomainIsoEnable(XPM_DOMAIN_ISO_PMC_LPD);
	if (Status != XST_SUCCESS)
		goto done;

	/*
	 * Assert POR for PS-LPD
	 */
	Status = XPmReset_AssertbyId(POR_RSTID(XPM_NODEIDX_RST_PS_POR),
				     PM_RESET_ACTION_ASSERT);

	/*TODO: Send PMC_I2C command to turn off PS-LPD power rail */

done:
	return Status;
}

XStatus XPm_PowerUpPLD()
{
	XStatus Status = XST_SUCCESS;

	/* TODO: Proceed only if vccint, vccaux, vccint_ram is 1 */

	/*TODO: Check NoC power state before disabling Isolation */

	/* Remove PL-NoC isolation */
	Status = XPm_DomainIsoDisable(XPM_DOMAIN_ISO_PL_SOC);
	if (Status != XST_SUCCESS)
		goto done;

	/*TODO: Check FPD and LPD  power state before disabling Isolation */

	/* Remove FPD-PL isolation */
	Status = XPm_DomainIsoDisable(XPM_DOMAIN_ISO_FPD_PL);
	if (Status != XST_SUCCESS)
		goto done;

	Status = XPm_DomainIsoDisable(XPM_DOMAIN_ISO_FPD_PL_TEST);
	if (Status != XST_SUCCESS)
		goto done;

	/* Remove LPD-PL isolation */
	Status = XPm_DomainIsoDisable(XPM_DOMAIN_ISO_LPD_PL);
	if (Status != XST_SUCCESS)
		goto done;

	Status = XPm_DomainIsoDisable(XPM_DOMAIN_ISO_LPD_PL_TEST);
	if (Status != XST_SUCCESS)
		goto done;

	/* Remove PL-PMC isolation */
	Status = XPm_DomainIsoDisable(XPM_DOMAIN_ISO_PMC_PL);
	if (Status != XST_SUCCESS)
		goto done;

	Status = XPm_DomainIsoDisable(XPM_DOMAIN_ISO_PMC_PL_TEST);
	if (Status != XST_SUCCESS)
		goto done;

	Status = XPm_DomainIsoDisable(XPM_DOMAIN_ISO_PMC_PL_CFRAME);
	if (Status != XST_SUCCESS)
		goto done;

	/* Remove isolation from VCCINT to VCCINT_RAM*/
	Status = XPm_DomainIsoDisable(XPM_DOMAIN_ISO_VCCRAM_SOC);
	if (Status != XST_SUCCESS)
		goto done;

	Status = XPm_DomainIsoDisable(XPM_DOMAIN_ISO_VCCAUX_SOC);
	if (Status != XST_SUCCESS)
		goto done;

	Status = XPm_DomainIsoDisable(XPM_DOMAIN_ISO_VCCAUX_VCCRAM);
	if (Status != XST_SUCCESS)
		goto done;

	/* Remove POR for PL */
	Status = XPmReset_AssertbyId(POR_RSTID(XPM_NODEIDX_RST_PL_POR),
				     PM_RESET_ACTION_RELEASE);
done:
	return Status;
}

XStatus XPm_PowerDwnPLD()
{
	XStatus Status = XST_SUCCESS;

	/* Isolate PL-NoC */
	Status = XPm_DomainIsoEnable(XPM_DOMAIN_ISO_PL_SOC);
	if (Status != XST_SUCCESS)
		goto done;

	/* Isolate FPD-PL */
	Status = XPm_DomainIsoEnable(XPM_DOMAIN_ISO_FPD_PL);
	if (Status != XST_SUCCESS)
		goto done;

	Status = XPm_DomainIsoEnable(XPM_DOMAIN_ISO_FPD_PL_TEST);
	if (Status != XST_SUCCESS)
		goto done;

	/* Isolate LPD-PL */
	Status = XPm_DomainIsoEnable(XPM_DOMAIN_ISO_LPD_PL);
	if (Status != XST_SUCCESS)
		goto done;

	Status = XPm_DomainIsoEnable(XPM_DOMAIN_ISO_LPD_PL_TEST);
	if (Status != XST_SUCCESS)
		goto done;

	/* Isolate PL-PMC */
	Status = XPm_DomainIsoEnable(XPM_DOMAIN_ISO_PMC_PL);
	if (Status != XST_SUCCESS)
		goto done;

	Status = XPm_DomainIsoEnable(XPM_DOMAIN_ISO_PMC_PL_TEST);
	if (Status != XST_SUCCESS)
		goto done;

	Status = XPm_DomainIsoEnable(XPM_DOMAIN_ISO_PMC_PL_CFRAME);
	if (Status != XST_SUCCESS)
		goto done;

	/* Isolate VCCINT_RAM from VCCINT */
	Status = XPm_DomainIsoEnable(XPM_DOMAIN_ISO_VCCRAM_SOC);
	if (Status != XST_SUCCESS)
		goto done;

	Status = XPm_DomainIsoEnable(XPM_DOMAIN_ISO_VCCAUX_SOC);
	if (Status != XST_SUCCESS)
		goto done;

	Status = XPm_DomainIsoEnable(XPM_DOMAIN_ISO_VCCAUX_VCCRAM);
	if (Status != XST_SUCCESS)
		goto done;

	/* Assert POR PL */
	Status = XPmReset_AssertbyId(POR_RSTID(XPM_NODEIDX_RST_PL_POR),
				     PM_RESET_ACTION_ASSERT);

	/* TODO: Send PMC_I2C command to turn of PLD power rail */
done:
	return Status;
}

XStatus XPm_PowerUpME()
{
	XStatus Status = XST_SUCCESS;

	/* TODO: Remove ME POR */

	return Status;
}

XStatus XPm_PowerDwnME()
{
	XStatus Status = XST_SUCCESS;

	/* TODO: Isolate ME */

	/* TODO: Assert POR ME */

	/* TODO: Send PMC_I2C command to turn of ME power rail */

	return Status;
}

XStatus XPm_PowerUpCPM()
{
	XStatus Status = XST_SUCCESS;

	/* TODO: Wait till PL and LPD on */

	/* Remove LPD-CPM isolation */
	Status = XPm_DomainIsoDisable(XPM_DOMAIN_ISO_LPD_CPM);
	if (Status != XST_SUCCESS)
		goto done;

	Status = XPm_DomainIsoDisable(XPM_DOMAIN_ISO_LPD_CPM_DFX);
	if (Status != XST_SUCCESS)
		goto done;

	/* Remove POR for CPM */
	Status = XPmReset_AssertbyId(POR_RSTID(XPM_NODEIDX_RST_CPM_POR),
				     PM_RESET_ACTION_RELEASE);

done:
	return Status;
}

XStatus XPm_PowerDwnCPM()
{
	XStatus Status = XST_SUCCESS;

	/* Isolate LPD-CPM */
	Status = XPm_DomainIsoEnable(XPM_DOMAIN_ISO_LPD_CPM);
	if (Status != XST_SUCCESS)
		goto done;

	Status = XPm_DomainIsoEnable(XPM_DOMAIN_ISO_LPD_CPM_DFX);
	if (Status != XST_SUCCESS)
		goto done;

	/* Assert POR for CPM */
	Status = XPmReset_AssertbyId(POR_RSTID(XPM_NODEIDX_RST_CPM_POR),
				     PM_RESET_ACTION_ASSERT);

	/* TODO: Send PMC_I2C command to turn off CPM power rail */

done:
	return Status;
}

XStatus XPm_PowerUpDDR()
{
	XStatus Status = XST_SUCCESS;

	/* TODO: Implement sequence for DDR power on */

	return Status;
}

XStatus XPm_PowerDwnDDR()
{
	XStatus Status = XST_SUCCESS;

	/* TODO: Implement DDR-SR function */

	/* TODO: Isolate DDR, NoC in order */

	/* TODO: Assert POR for DDR, NoC */

	/* TODO: Send PMC_I2C command to turn off DDR/NPI/NOC power rail */

	return Status;
}

XStatus XPm_PowerUpNoC()
{
	XStatus Status = XST_SUCCESS;

	/*TODO: wait till vccint_soc is 1 */

	/* Isolate FPD-NoC domain */
	Status = XPm_DomainIsoDisable(XPM_DOMAIN_ISO_FPD_SOC);
	if (Status != XST_SUCCESS)
		goto done;

	/* Isolate LPD-NoC domain */
	Status = XPm_DomainIsoDisable(XPM_DOMAIN_ISO_LPD_SOC);
	if (Status != XST_SUCCESS)
		goto done;

	/* Isolate PL-NoC domain */
	Status = XPm_DomainIsoDisable(XPM_DOMAIN_ISO_PL_SOC);
	if (Status != XST_SUCCESS)
		goto done;

	/* Isolate VCCAUX-NoC domain */
	Status = XPm_DomainIsoDisable(XPM_DOMAIN_ISO_VCCAUX_SOC);
	if (Status != XST_SUCCESS)
		goto done;

	/* Isolate VCCRAM-NoC domain */
	Status = XPm_DomainIsoDisable(XPM_DOMAIN_ISO_VCCRAM_SOC);
	if (Status != XST_SUCCESS)
		goto done;

	/* Isolate PMC-NoC domain */
	Status = XPm_DomainIsoDisable(XPM_DOMAIN_ISO_PMC_SOC);
	if (Status != XST_SUCCESS)
		goto done;

	/* Isolate PMC-NoC NPI domain */
	Status = XPm_DomainIsoDisable(XPM_DOMAIN_ISO_PMC_SOC_NPI);
	if (Status != XST_SUCCESS)
		goto done;

	/* Remove POR for NoC */
	Status = XPmReset_AssertbyId(POR_RSTID(XPM_NODEIDX_RST_NOC_POR),
				     PM_RESET_ACTION_RELEASE);
	if (Status != XST_SUCCESS)
		goto done;

	/* TODO: Initiate scan clear/MBIST/BISR for NoC as applicable */

done:
	return Status;
}

XStatus XPm_PowerDwnNoC()
{
	XStatus Status = XST_SUCCESS;

	/* Isolate FPD-NoC domain */
	Status = XPm_DomainIsoEnable(XPM_DOMAIN_ISO_FPD_SOC);
	if (Status != XST_SUCCESS)
		goto done;

	/* Isolate LPD-NoC domain */
	Status = XPm_DomainIsoEnable(XPM_DOMAIN_ISO_LPD_SOC);
	if (Status != XST_SUCCESS)
		goto done;

	/* Isolate PL-NoC domain */
	Status = XPm_DomainIsoEnable(XPM_DOMAIN_ISO_PL_SOC);
	if (Status != XST_SUCCESS)
		goto done;

	/* Isolate VCCAUX-NoC domain */
	Status = XPm_DomainIsoEnable(XPM_DOMAIN_ISO_VCCAUX_SOC);
	if (Status != XST_SUCCESS)
		goto done;

	/* Isolate VCCRAM-NoC domain */
	Status = XPm_DomainIsoEnable(XPM_DOMAIN_ISO_VCCRAM_SOC);
	if (Status != XST_SUCCESS)
		goto done;

	/* Isolate PMC-NoC domain */
	Status = XPm_DomainIsoEnable(XPM_DOMAIN_ISO_PMC_SOC);
	if (Status != XST_SUCCESS)
		goto done;

	/* Isolate PMC-NoC NPI domain */
	Status = XPm_DomainIsoEnable(XPM_DOMAIN_ISO_PMC_SOC_NPI);
	if (Status != XST_SUCCESS)
		goto done;

	/* Assert POR for NoC */
	Status = XPmReset_AssertbyId(POR_RSTID(XPM_NODEIDX_RST_NOC_POR),
				     PM_RESET_ACTION_ASSERT);

	/* TODO: Send PMC_I2C command to turn off NoC power rail */

done:
	return Status;
}


XStatus XPmPowerDomain_InitDomain(XPm_PowerDomain *PwrDomain, u32 Function)
{
	XStatus Status = XST_SUCCESS;
	struct XPm_PowerDomainOps *Ops = PwrDomain->DomainOps;

	if (XPM_POWER_STATE_ON == PwrDomain->Power.Node.State) {
		goto done;
	}

	switch (Function) {
	case FUNC_INIT_START:
		if (XPM_POWER_STATE_OFF != PwrDomain->Power.Node.State) {
			Status = XST_FAILURE;
			goto done;
		}
		if (Ops && Ops->PreHouseClean) {
			Status = Ops->PreHouseClean();
			if (XST_SUCCESS != Status) {
				goto done;
			}
		}
		PwrDomain->Power.Node.State = XPM_POWER_STATE_INITIALIZING;
		break;
	case FUNC_INIT_FINISH:
		if (XPM_POWER_STATE_INITIALIZING != PwrDomain->Power.Node.State) {
			Status = XST_FAILURE;
			goto done;
		}
		if (Ops && Ops->PostHouseClean) {
			Status = Ops->PostHouseClean();
			if (XST_SUCCESS != Status) {
				goto done;
			}
		}
		PwrDomain->Power.Node.State = XPM_POWER_STATE_ON;
		break;
	case FUNC_SCAN_CLEAR:
		if (XPM_POWER_STATE_INITIALIZING != PwrDomain->Power.Node.State) {
			Status = XST_FAILURE;
			goto done;
		}
		if (Ops && Ops->ScanClear) {
			Status = Ops->ScanClear();
			if (XST_SUCCESS != Status) {
				goto done;
			}
		}
		break;
	case FUNC_BISR:
		if (XPM_POWER_STATE_INITIALIZING != PwrDomain->Power.Node.State) {
			Status = XST_FAILURE;
			goto done;
		}
		if (Ops && Ops->Bisr) {
			Status = Ops->Bisr();
			if (XST_SUCCESS != Status) {
				goto done;
			}
		}
		break;
	case FUNC_LBIST:
		if (XPM_POWER_STATE_INITIALIZING != PwrDomain->Power.Node.State) {
			Status = XST_FAILURE;
			goto done;
		}
		if (Ops && Ops->Lbist) {
			Status = Ops->Lbist();
			if (XST_SUCCESS != Status) {
				goto done;
			}
		}
		break;
	case FUNC_MBIST_CLEAR:
		if (XPM_POWER_STATE_INITIALIZING != PwrDomain->Power.Node.State) {
			Status = XST_FAILURE;
			goto done;
		}
		if (Ops && Ops->Mbist) {
			Status = Ops->Mbist();
			if (XST_SUCCESS != Status) {
				goto done;
			}
		}
		break;
	default:
		Status = XST_INVALID_PARAM;
		break;
	}

done:
	return Status;
}
