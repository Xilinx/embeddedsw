/*
 * Copyright (C) 2017 - 2018 Xilinx, Inc.  All rights reserved.
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
#include "xpfw_config.h"
#ifdef ENABLE_PM

#include "pm_hooks.h"
#include "pm_periph.h"
#include "pm_requirement.h"
#include "pm_qspi.h"
#include "pm_system.h"
#include "pmu_global.h"

#ifdef ENABLE_POS
#define IOU_SLCR_BASE		0XFF180000U
#define IOU_SCLR_MIO_PIN_26	( ( IOU_SLCR_BASE )  + 0X00000068U )
#define IOU_SCLR_MIO_PIN_34	( ( IOU_SLCR_BASE )  + 0X00000088U )
#define IOU_SCLR_MIO_PIN_37	( ( IOU_SLCR_BASE )  + 0X00000094U )
#define IOU_SCLR_MIO_MST_TRI0	( ( IOU_SLCR_BASE )  + 0X00000204U )
#define IOU_SCLR_MIO_MST_TRI1	( ( IOU_SLCR_BASE )  + 0X00000208U )
#define GPIO_BASE		0XFF0A0000U
#define GPIO_DATA_1_RO		( ( GPIO_BASE ) + 0X00000064U)

/**
 * These requirements are needed for the system in order to save DDR context
 * during Power Off Suspend. On ZCU102 board QSPI Flash memory device is used
 * for storing DDR context.
 */
PmPosRequirement pmPosDdrReqs_g[POS_DDR_REQS_SIZE] = {
	{
		.slave = &pmSlaveQSpi_g,
		.caps = PM_CAP_ACCESS,
	},
};

extern u8 __srdata_start;
extern u8 __srdata_end;

/**
 * PmHookPosSaveDdrContext() - User hook for saving context required for taking
 * 			       DDR out of self refresh after resume from Power
 * 			       Off Suspend
 *
 * @return	XST_SUCCESS if context is saved, failure code otherwise
 */
int PmHookPosSaveDdrContext(void)
{
	int status;
	u32 srDataStart = (u32)&__srdata_start;
	u32 srDataEnd = (u32)&__srdata_end;

	/* Initialize hardware required for QSPI operation */
	status = PmQspiHWInit();
	if (XST_SUCCESS != status) {
		goto done;
	}

	/* Initialize QSPI driver */
	status = PmQspiInit();
	if (XST_SUCCESS != status) {
		goto done;
	}

	/* Save data to QSPI */
	status = PmQspiWrite((u8*)srDataStart, srDataEnd - srDataStart);
	if (XST_SUCCESS != status) {
		goto done;
	}

done:
	return status;
}

/**
 * PmHookFinalizePowerOffSuspend() - User hook for finishing entry to Power Off
 * 				     Suspend state
 */
void PmHookFinalizePowerOffSuspend(void)
{
	u32 reg, suspendType;

	/* Drive MIO37 high if entering Power Off Suspend state */
	suspendType = PmSystemSuspendType();
	reg = XPfw_Read32(PMU_LOCAL_GPO1_READ);
	if (PM_SUSPEND_TYPE_POWER_OFF == suspendType) {
		reg |= PMU_IOMODULE_GPO1_MIO_5_MASK;
	} else {
		reg &= ~PMU_IOMODULE_GPO1_MIO_5_MASK;
	}
	XPfw_Write32(PMU_IOMODULE_GPO1, reg);

	/* Configure MIO37 to be controlled by the PMU */
	XPfw_RMW32(IOU_SCLR_MIO_PIN_37, 0x000000FEU ,0x00000008U);

	/* Configure MIO37 tri-state enables */
	XPfw_RMW32(IOU_SCLR_MIO_MST_TRI1, 0x00000020U ,0x00000000U);
}
#endif

/**
 * PmHookPowerDownLpd() - User hook for powering down LPD
 */
void PmHookPowerDownLpd(void)
{
	u32 reg;

	/* Drive MIO34 (LPD power down request pin) low */
	reg = XPfw_Read32(PMU_LOCAL_GPO1_READ);
	reg &= ~PMU_IOMODULE_GPO1_MIO_2_MASK;
	XPfw_Write32(PMU_IOMODULE_GPO1, reg);
}

#ifdef ENABLE_POS
/**
 * PmHookInitPowerOffSuspend() - User hook for Power Off Suspend state
 * 				 initialization
 */
void PmHookInitPowerOffSuspend(void)
{
	u32 reg;

	/* Drive MIO34 (LPD power down request pin) high */
	reg = XPfw_Read32(PMU_LOCAL_GPO1_READ);
	reg |= PMU_IOMODULE_GPO1_MIO_2_MASK;
	XPfw_Write32(PMU_IOMODULE_GPO1, reg);
	/* Configure MIO34 to be controlled by the PMU */
	XPfw_RMW32(IOU_SCLR_MIO_PIN_34, 0x000000FEU ,0x00000008U);
	/* Configure MIO34 tri-state enable */
	XPfw_RMW32(IOU_SCLR_MIO_MST_TRI1, 0x00000004U ,0x00000000U);
}

/**
 * PmHookGetBootType() - User hook for detecting boot type
 */
u32 PmHookGetBootType(void)
{
	u32 bootType;

	/* Release GPIO reset */
	XPfw_RMW32(CRL_APB_RST_LPD_IOU2, CRL_APB_RST_LPD_IOU2_GPIO_RESET_MASK,
			~CRL_APB_RST_LPD_IOU2_GPIO_RESET_MASK);

	/* Read state of MIO26 pin which is used to detect boot type */
	bootType = XPfw_Read32(GPIO_DATA_1_RO) & 1U;
	bootType += 1;

	/* Assert GPIO reset */
	XPfw_RMW32(CRL_APB_RST_LPD_IOU2, CRL_APB_RST_LPD_IOU2_GPIO_RESET_MASK,
			~CRL_APB_RST_LPD_IOU2_GPIO_RESET_MASK);

	/* Signal boot type to FSBL */
	XPfw_Write32(PMU_GLOBAL_GLOBAL_GEN_STORAGE1, bootType);

	/* Update last suspend type based on detected boot type */
	PmSystemSetSuspendType(bootType - 1U);

	return bootType;
}

/**
 * PmHookRestoreDdrContext() - User hook for restoring context required for
 * 			       taking DDR out of self refresh after resume from
 * 			       Power Off Suspend
 *
 * @return	XST_SUCCESS if context is restored, failure code otherwise
 */
int PmHookRestoreDdrContext(void)
{
	int status;
	u32 srDataStart = (u32)&__srdata_start;
	u32 srDataEnd = (u32)&__srdata_end;

	/* Initialize hardware required for QSPI operation */
	status = PmQspiHWInit();
	if (status != XST_SUCCESS) {
		goto done;
	}

	/* Initialize QSPI driver */
	status = PmQspiInit();
	if (XST_SUCCESS != status) {
		goto done;
	}

	/* Restore data from QSPI */
	status = PmQspiRead(srDataEnd - srDataStart, (u8*)srDataStart);
	if (XST_SUCCESS != status) {
		goto done;
	}

done:
	return status;
}


/**
 * PmHookPowerOffSuspendDdrReady() - User hook for signaling that PMUFW is ready
 * 				     to take DDR out of self refresh
 */
void PmHookPowerOffSuspendDdrReady(void)
{
	u32 reg;

	/* Configure MIO 26 to be controlled by the PMU */
	XPfw_RMW32(IOU_SCLR_MIO_PIN_26, 0x000000FEU ,0x00000008U);

	/* Configure MIO26 tri-state enables */
	XPfw_RMW32(IOU_SCLR_MIO_MST_TRI0, 0x04000000U ,0x00000000U);

	/* Get MIO26 state */
	reg = XPfw_Read32(PMU_IOMODULE_GPI1);
	reg &= PMU_IOMODULE_GPI1_MIO_WAKE_0_MASK;

	if (reg > 0U) {
		/* Drive MIO37 low to signal that PMU is ready to restore DDR */
		reg = XPfw_Read32(PMU_LOCAL_GPO1_READ);
		reg &= ~PMU_IOMODULE_GPO1_MIO_5_MASK;
		XPfw_Write32(PMU_IOMODULE_GPO1, reg);

		/* Configure MIO 37 to be controlled by the PMU */
		XPfw_RMW32(IOU_SCLR_MIO_PIN_37, 0x000000FEU ,0x00000008U);

		/* Configure MIO37 tri-state enables */
		XPfw_RMW32(IOU_SCLR_MIO_MST_TRI1, 0x00000020U ,0x00000000U);

		/* Wait for MIO26 to go down before continuing with resume */
		do {
			reg = XPfw_Read32(PMU_IOMODULE_GPI1);
			reg &= PMU_IOMODULE_GPI1_MIO_WAKE_0_MASK;
		} while (reg != 0U);
	}
}
#endif

#endif
