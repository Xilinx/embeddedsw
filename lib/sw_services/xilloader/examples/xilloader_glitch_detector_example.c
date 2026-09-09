/**************************************************************************************************
* Copyright (C) 2026 Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/

/*************************************************************************************************/
/**
 *
 * @file xilloader_glitch_detector_example.c
 *
 * This file illustrates Versal Gen2 (2VE-2VM) glitch detector configuration via the
 * Loader IPI command XLOADER_CMD_ID_CONFIG_GLITCH_DETECTOR, and optionally generates
 * an on-chip test glitch via direct MMIO.
 *
 * To build this application, xilmailbox and xilloader libraries must be included in BSP and
 * xilloader library must be in client mode.
 *
 * Select which detectors to configure with the CONFIG_* macros below.
 * Each enabled group is packed into one IPI. Defaults: PMC + ASU + LPD
 * (CONFIG_{PMC,ASU,LPD}_GD = 1).
 *
 * Optional test-glitch stimulus (TEST_GLITCH_*):
 * - There is no Loader IPI for test-glitch generate. When TEST_GLITCH_ENABLE is 1,
 *   this example programs test mode / test glitch sel / test glitch gen with Xil_Out32
 *   after a successful configure IPI. Per-block TEST_GLITCH_{PMC,ASU,LPD}_GD
 *   macros further restrict which detectors are pulsed (and only if that group was
 *   also selected for configure).
 * - Direct MMIO only works if the running processor's isolation/XPPU-XMPU
 *   protection configuration grants it access to pmc_analog, asu_global, and
 *   lpd_slcr; the Loader IPI configure path has no such restriction since it runs
 *   on the PLM. If those blocks are isolated from this processor in your design,
 *   the writes below will fault - leave TEST_GLITCH_ENABLE at 0 (the default) and
 *   rely on the IPI configure only, or drive the glitch from a supply that can
 *   reach the block instead.
 * - When TEST_GLITCH_ENABLE is 1, GenerateSelectedTestGlitches() reads EAM status
 *   immediately after each pulse (PMC ERR3 and LPD SLCR ERR2).
 *
 * Procedure to run the example.
 * ------------------------------------------------------------------------------------------------
 * Load the Pdi.
 * Select the target.
 * Download the example elf into the target.
 *
 * @note
 * - The ASU glitch detector is held in reset out of POR (psx_crl.RST_ASU_GD
 *   resets to 0x3) and the boot flow does not release it. Configuration via
 *   XLoader_ConfigureGlitchDetector() programs the calibration while the block
 *   is held in reset and releases it on the way out. Calibration registers read
 *   back correctly even while the block is in reset, so readback alone does not
 *   prove the detector is alive.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date       Changes
 * ----- ---- ---------- --------------------------------------------------------------------------
 * 1.00  ng   08/17/2026 Initial release
 * 1.01  ng   09/09/2026 Unlock writes to LPD GD registers via wprot0
 *
 * </pre>
 *
 *************************************************************************************************/

/**
 * @addtogroup xloader_client_example_apis XilLoader Client Example APIs
 * @{
 */

/*************************************** Include Files *******************************************/
#include "xil_printf.h"
#include "xstatus.h"
#include "xil_cache.h"
#include "xil_io.h"
#include "xloader_client.h"

/************************************ Constant Definitions ***************************************/
/**
 * Which glitch detectors to configure via XLoader_ConfigureGlitchDetector.
 * Use 0 to disable a group and 1 to enable it. Enabled groups are packed
 * into one IPI.
 */
#ifndef CONFIG_PMC_GD
#define CONFIG_PMC_GD		(1U) /**< 0: disable, 1: enable PMC GD0 + PMC GD1 */
#endif
#ifndef CONFIG_ASU_GD
#define CONFIG_ASU_GD		(1U) /**< 0: disable, 1: enable ASU GD */
#endif
#ifndef CONFIG_LPD_GD
#define CONFIG_LPD_GD		(1U) /**< 0: disable, 1: enable LPD GD0 + LPD GD1 */
#endif

#if ((CONFIG_PMC_GD == 0U) && (CONFIG_ASU_GD == 0U) && (CONFIG_LPD_GD == 0U))
#error "No glitch detector selected: set one of CONFIG_PMC_GD/CONFIG_ASU_GD/CONFIG_LPD_GD"
#endif

/**
 * Test-glitch stimulus after configure. Master switch plus optional per-block
 * enables (0: disable, 1: enable). Stimulus uses direct Xil_Out32 (no Loader
 * IPI for test-glitch).
 *
 * Defaults to disabled (0): unlike the IPI configure path (serviced by the PLM),
 * this direct MMIO only works if isolation/protection settings grant this
 * processor access to pmc_analog/asu_global/lpd_slcr. Set to 1 only after
 * confirming that for your design.
 */
#ifndef TEST_GLITCH_ENABLE
#define TEST_GLITCH_ENABLE	(0U) /**< 0: disable, 1: enable test glitch via MMIO */
#endif
#ifndef TEST_GLITCH_PMC_GD
#define TEST_GLITCH_PMC_GD	(1U) /**< 0: disable, 1: pulse PMC GD0 + GD1 (if configured) */
#endif
#ifndef TEST_GLITCH_ASU_GD
#define TEST_GLITCH_ASU_GD	(1U) /**< 0: disable, 1: enable pulse ASU GD (if configured) */
#endif
#ifndef TEST_GLITCH_LPD_GD
#define TEST_GLITCH_LPD_GD	(1U) /**< 0: disable, 1: pulse LPD GD0 + GD1 (if configured) */
#endif


/**
 * Shared calibration for selected detectors, expressed the same way as the PMC gd_fuse_ctrl
 * word 0xC4211: Depth = 12, Width = 2, RefVoltage = 12. The ASU / LPD detection windows are
 * not guaranteed to match PMC, so if no glitch is detected after enable sweep these values.
 * Refer to XAPP1389 for more information related to the values to be written to
 * depth/width/ref voltage.
 */
#define GD_DEPTH_VAL		(12U) /**< GD_DEPTH_SEL, 5 bits */
#define GD_WIDTH_VAL		(2U)  /**< GD_WIDTH_SEL, 4 bits */
#define GD_REFVOL_VAL		(12U) /**< GD_LDO_REF_SEL, 4 bits */

#if (TEST_GLITCH_ENABLE != 0U)
/* PMC analog GD_CTRL (packed GD0 in [15:0], GD1 in [31:16]); see xplmi_glitch_detector_example */
#define PMC_ANALOG_BASE_ADDR		(0xF1160000U)
#define PMC_GD_CTRL_OFFSET		(0x00000U)
#define PMC_GD0_TEST_GLITCH_GENVAL	(0x00F800FEU) /**< GD0 test mode + gen + sel */
#define PMC_GD1_TEST_GLITCH_GENVAL	(0x00FE00F8U) /**< GD1 test mode + gen + sel */
#define PMC_GD_TEST_GLITCH_STOPVAL	(0x00F800F8U) /**< Clear gen/mode, keep sel */
#define PMC_GD_TEST_GLITCH_CLEARVAL	(0x00000000U) /**< Clear gen/mode and sel taps */

/* ASU GD (asu_global @ 0xEBF80000) */
#define ASU_GLOBAL_BASE_ADDR		(0xEBF80000U)
#define ASU_GD_EN_GLITCH_DET_B_OFFSET	(0x1002CU)
#define ASU_GD_GL_DET_TEST_MODE_OFFSET	(0x1003CU)
#define ASU_GD_TEST_GLITCH_GEN_OFFSET	(0x10040U)
#define ASU_GD_TEST_GLITCH_SEL_OFFSET	(0x10044U)
#define ASU_GD_FABRIC_GL_EN_OFFSET	(0x10048U)

/* LPD GD (lpd_slcr @ 0xEB430000); GD1 stride 0x28 */
#define LPD_SLCR_WPROT0_ADDR		(0xEB410000U) /**< 0: writes enabled, 1: writes disabled */
#define LPD_SLCR_WRITES_ENABLED		(0U)
#define LPD_GD_BASE_ADDR		(0xEB430000U)
#define LPD_GD_INSTANCE_STRIDE		(0x28U)
#define LPD_GD_EN_GLITCH_DET_B_OFFSET	(0x4U)
#define LPD_GD_GL_DET_TEST_MODE_OFFSET	(0x14U)
#define LPD_GD_TEST_GLITCH_GEN_OFFSET	(0x18U)
#define LPD_GD_TEST_GLITCH_SEL_OFFSET	(0x1CU)
#define LPD_GD_FABRIC_GL_EN_OFFSET	(0x20U)

#define GD_ENABLE_VAL			(0U)  /**< EN_GLITCH_DET_B active-low enable */
#define GD_FABRIC_DISABLE		(0U)  /**< PS-controlled test path */
#define GD_TEST_MODE_ON			(1U)
#define GD_TEST_MODE_OFF		(0U)
#define GD_TEST_GLITCH_ON		(1U)
#define GD_TEST_GLITCH_OFF		(0U)
#define GD_TEST_GLITCH_SEL		(0x1FU) /**< Drive all five test taps */
#endif

/* EAM status registers; offsets match xplmi_hw.h / xil_error_node.h */
#define PMC_GLOBAL_BASE_ADDR		(0xF1110000U)
#define PMC_GLOBAL_PMC_ERR1_STATUS	(PMC_GLOBAL_BASE_ADDR + 0x20000U)
#define PMC_GLOBAL_REG_ERR_OFFSET	(4U)
#define PMC_EAM_ERR3_INDEX		(2U)
#define PMC_EAM_ERR3_STATUS_ADDR	(PMC_GLOBAL_PMC_ERR1_STATUS + \
					 (PMC_EAM_ERR3_INDEX * PMC_GLOBAL_REG_ERR_OFFSET))
#define PMC_EAM_GD_MASK			(0x00080000U) /**< PMC_EAM_GD, ERR3 bit 19 */
#define ASU_EAM_GD_MASK			(0x00040000U) /**< ASU_EAM_GD, ERR3 bit 18 */
#define LPD_SLCR_EAM_ERR0_STATUS	(0xEB450000U)
#define LPD_SLCR_REG_ERR_OFFSET		(4U)
#define LPD_EAM_ERR2_INDEX		(2U)
#define LPD_EAM_ERR2_STATUS_ADDR	(LPD_SLCR_EAM_ERR0_STATUS + \
					 (LPD_EAM_ERR2_INDEX * LPD_SLCR_REG_ERR_OFFSET))
#define LPD_GD0_EAM_MASK		(0x40000000U) /**< LPX_GLITCH_DET0, ERR2 bit 30 */
#define LPD_GD1_EAM_MASK		(0x80000000U) /**< LPX_GLITCH_DET1, ERR2 bit 31 */

/************************************** Type Definitions *****************************************/

/*************************** Macros (Inline Functions) Definitions *******************************/
/** True if CONFIG_* selected this group. */
#define CFG_PMC_SELECTED()	((CONFIG_PMC_GD) != 0U)
#define CFG_ASU_SELECTED()	((CONFIG_ASU_GD) != 0U)
#define CFG_LPD_SELECTED()	((CONFIG_LPD_GD) != 0U)

/************************************ Function Prototypes ****************************************/
#if (TEST_GLITCH_ENABLE != 0U)
static void GeneratePmcTestGlitch(void);
static void GenerateAsuTestGlitch(void);
static void GenerateLpdTestGlitch(u8 GdNum);
static int GenerateSelectedTestGlitches(void);
#endif

/************************************ Variable Definitions ***************************************/
static XMailbox MailboxInstance; /* Instance of the mailbox used for IPI communication */
static XLoader_ClientInstance LoaderClientInstance; /* xilloader client for GD configure IPI */

#if (TEST_GLITCH_ENABLE != 0U)
/*************************************************************************************************/
/**
 * @brief	Pulse a PMC test glitch on GD0 then GD1 via pmc_analog.GD_CTRL (direct MMIO).
 *
 *************************************************************************************************/
static void GeneratePmcTestGlitch(void)
{
	u32 Addr = PMC_ANALOG_BASE_ADDR + PMC_GD_CTRL_OFFSET;

	xil_printf("PMC test glitch: GD0 then GD1 via GD_CTRL @ 0x%08x\n\r", Addr);
	Xil_Out32(Addr, PMC_GD0_TEST_GLITCH_GENVAL);
	Xil_Out32(Addr, PMC_GD_TEST_GLITCH_STOPVAL);
	Xil_Out32(Addr, PMC_GD1_TEST_GLITCH_GENVAL);
	Xil_Out32(Addr, PMC_GD_TEST_GLITCH_STOPVAL);
	Xil_Out32(Addr, PMC_GD_TEST_GLITCH_CLEARVAL);
}

/*************************************************************************************************/
/**
 * @brief	Enable the ASU detector and pulse a test glitch via asu_global (direct MMIO).
 *
 *************************************************************************************************/
static void GenerateAsuTestGlitch(void)
{
	u32 Base = ASU_GLOBAL_BASE_ADDR;

	xil_printf("ASU test glitch: test_mode/sel/gen via asu_global\n\r");
	Xil_Out32(Base + ASU_GD_FABRIC_GL_EN_OFFSET, GD_FABRIC_DISABLE);
	Xil_Out32(Base + ASU_GD_EN_GLITCH_DET_B_OFFSET, GD_ENABLE_VAL);
	Xil_Out32(Base + ASU_GD_GL_DET_TEST_MODE_OFFSET, GD_TEST_MODE_ON);
	Xil_Out32(Base + ASU_GD_TEST_GLITCH_SEL_OFFSET, GD_TEST_GLITCH_SEL);
	Xil_Out32(Base + ASU_GD_TEST_GLITCH_GEN_OFFSET, GD_TEST_GLITCH_ON);
	Xil_Out32(Base + ASU_GD_TEST_GLITCH_GEN_OFFSET, GD_TEST_GLITCH_OFF);
	Xil_Out32(Base + ASU_GD_GL_DET_TEST_MODE_OFFSET, GD_TEST_MODE_OFF);
	Xil_Out32(Base + ASU_GD_TEST_GLITCH_SEL_OFFSET, 0U);
}

/*************************************************************************************************/
/**
 * @brief	Enable one LPD detector and pulse a test glitch via lpd_slcr (direct MMIO).
 *
 * @param	GdNum is 0 for LPD GD0 or 1 for LPD GD1 (instance stride 0x28).
 *
 * @note	The GD registers sit in the lpd_slcr aperture, which is guarded by
 *		lpd_slcr.wprot0. While wprot0 is set the writes below are dropped
 *		silently, so it is cleared for the duration and restored afterwards.
 *
 *************************************************************************************************/
static void GenerateLpdTestGlitch(u8 GdNum)
{
	u32 Base = LPD_GD_BASE_ADDR +
		   (LPD_GD_INSTANCE_STRIDE * (u32)GdNum);
	u32 SavedWprot;

	xil_printf("LPD GD%u test glitch via lpd_slcr @ 0x%08x\n\r", GdNum, Base);

	/* Every lpd_slcr write below is gated by wprot0, so unlock it first */
	SavedWprot = Xil_In32(LPD_SLCR_WPROT0_ADDR);
	Xil_Out32(LPD_SLCR_WPROT0_ADDR, LPD_SLCR_WRITES_ENABLED);

	Xil_Out32(Base + LPD_GD_FABRIC_GL_EN_OFFSET, GD_FABRIC_DISABLE);
	Xil_Out32(Base + LPD_GD_EN_GLITCH_DET_B_OFFSET, GD_ENABLE_VAL);
	Xil_Out32(Base + LPD_GD_GL_DET_TEST_MODE_OFFSET, GD_TEST_MODE_ON);
	Xil_Out32(Base + LPD_GD_TEST_GLITCH_SEL_OFFSET, GD_TEST_GLITCH_SEL);
	Xil_Out32(Base + LPD_GD_TEST_GLITCH_GEN_OFFSET, GD_TEST_GLITCH_ON);
	Xil_Out32(Base + LPD_GD_TEST_GLITCH_GEN_OFFSET, GD_TEST_GLITCH_OFF);
	Xil_Out32(Base + LPD_GD_GL_DET_TEST_MODE_OFFSET, GD_TEST_MODE_OFF);
	Xil_Out32(Base + LPD_GD_TEST_GLITCH_SEL_OFFSET, 0U);

	/* Put the write protection back the way the platform had it */
	Xil_Out32(LPD_SLCR_WPROT0_ADDR, SavedWprot);
}

/*************************************************************************************************/
/**
 * @brief	Generate test glitches for blocks selected by TEST_GLITCH_* and
 *		CONFIG_*, then read EAM status immediately after each pulse.
 *
 * @return
 *		- XST_SUCCESS if every exercised detector latches in EAM status.
 *		- XST_FAILURE if any expected EAM bit is clear.
 *
 * @note	Direct MMIO to pmc_analog/asu_global/lpd_slcr only succeeds if
 *		isolation/protection settings grant this processor access to those
 *		blocks; otherwise these writes will fault.
 *
 *************************************************************************************************/
static int GenerateSelectedTestGlitches(void)
{
	int Status = XST_SUCCESS;
	u32 PmcEamErr3;
	u32 LpdEamErr2;

	if ((TEST_GLITCH_PMC_GD != 0U) && CFG_PMC_SELECTED()) {
		GeneratePmcTestGlitch();
		PmcEamErr3 = Xil_In32(PMC_EAM_ERR3_STATUS_ADDR);
		xil_printf("EAM: PMC ERR3 = 0x%08x after PMC test glitch\n\r", PmcEamErr3);
		if ((PmcEamErr3 & PMC_EAM_GD_MASK) != 0U) {
			xil_printf("EAM: PMC glitch detected (PMC_EAM_GD)\n\r");
		}
		else {
			xil_printf("EAM: PMC glitch not seen in PMC_EAM_GD\n\r");
			Status = XST_FAILURE;
		}
	}
	if ((TEST_GLITCH_ASU_GD != 0U) && CFG_ASU_SELECTED()) {
		GenerateAsuTestGlitch();
		PmcEamErr3 = Xil_In32(PMC_EAM_ERR3_STATUS_ADDR);
		xil_printf("EAM: PMC ERR3 = 0x%08x after ASU test glitch\n\r",
			   PmcEamErr3);
		if ((PmcEamErr3 & ASU_EAM_GD_MASK) != 0U) {
			xil_printf("EAM: ASU glitch detected (ASU_EAM_GD)\n\r");
		}
		else {
			xil_printf("EAM: ASU glitch not seen in ASU_EAM_GD\n\r");
			Status = XST_FAILURE;
		}
	}
	if ((TEST_GLITCH_LPD_GD != 0U) && CFG_LPD_SELECTED()) {
		GenerateLpdTestGlitch(0U);
		LpdEamErr2 = Xil_In32(LPD_EAM_ERR2_STATUS_ADDR);
		xil_printf("EAM: LPD SLCR ERR2 = 0x%08x after LPD GD0 test glitch\n\r",
			   LpdEamErr2);
		if ((LpdEamErr2 & LPD_GD0_EAM_MASK) != 0U) {
			xil_printf("EAM: LPD GD0 glitch detected (LPX_GLITCH_DET0)\n\r");
		}
		else {
			xil_printf("EAM: LPD GD0 glitch not seen in LPX_GLITCH_DET0\n\r");
			Status = XST_FAILURE;
		}
		GenerateLpdTestGlitch(1U);
		LpdEamErr2 = Xil_In32(LPD_EAM_ERR2_STATUS_ADDR);
		xil_printf("EAM: LPD SLCR ERR2 = 0x%08x after LPD GD1 test glitch\n\r",
			   LpdEamErr2);
		if ((LpdEamErr2 & LPD_GD1_EAM_MASK) != 0U) {
			xil_printf("EAM: LPD GD1 glitch detected (LPX_GLITCH_DET1)\n\r");
		}
		else {
			xil_printf("EAM: LPD GD1 glitch not seen in LPX_GLITCH_DET1\n\r");
			Status = XST_FAILURE;
		}
	}

	return Status;
}
#endif /* TEST_GLITCH_ENABLE */

/*************************************************************************************************/
/**
 * @brief	Main function to configure selected glitch detectors via Loader IPI, then
 *		optionally generate a test glitch via direct MMIO.
 *
 * @return
 *		- XST_SUCCESS on success.
 *		- Error code on failure.
 *
 *************************************************************************************************/
int main(void)
{
	int Status = XST_FAILURE;
	u32 GdCfgWords[XLOADER_GD_CFG_MAX_LEN];
	u32 NumWords = 0U;

	xil_printf("\n\rGlitch detector configure example\n\r");
	xil_printf("Configuring: Depth = %d, Width = %d, RefVoltage = %d\n\r",
		   GD_DEPTH_VAL, GD_WIDTH_VAL, GD_REFVOL_VAL);

#ifdef XLOADER_CACHE_DISABLE
	Xil_DCacheDisable();
#endif

#ifndef SDT
	Status = (int)XMailbox_Initialize(&MailboxInstance, 0U);
#else
	Status = (int)XMailbox_Initialize(&MailboxInstance, XPAR_XIPIPSU_0_BASEADDR);
#endif
	if (Status != XST_SUCCESS) {
		xil_printf("Mailbox initialization failed\n\r");
		goto END;
	}

	Status = XLoader_ClientInit(&LoaderClientInstance, &MailboxInstance);
	if (Status != XST_SUCCESS) {
		xil_printf("XilLoader client initialization failed\n\r");
		goto END;
	}

	/**
	 * Pack one config word per selected detector, then send a single
	 * XLoader_ConfigureGlitchDetector IPI. PMC calibration is applied live in
	 * GD_FUSE_CTRL, while the ASU and LPD detectors are held in reset by the
	 * PLM while calibration is written and released afterwards. Enabling /
	 * test-glitch are left to the application (optional TEST_GLITCH_* path
	 * below).
	 */
	if (CONFIG_PMC_GD != 0U) {
		GdCfgWords[NumWords++] = XLoader_PackGlitchDetectorCfg(
			(u8)XLOADER_GD_TYPE_PMC_GD0, GD_DEPTH_VAL,
			GD_WIDTH_VAL, GD_REFVOL_VAL);
		GdCfgWords[NumWords++] = XLoader_PackGlitchDetectorCfg(
			(u8)XLOADER_GD_TYPE_PMC_GD1, GD_DEPTH_VAL,
			GD_WIDTH_VAL, GD_REFVOL_VAL);
	}
	if (CONFIG_ASU_GD != 0U) {
		GdCfgWords[NumWords++] = XLoader_PackGlitchDetectorCfg(
			(u8)XLOADER_GD_TYPE_ASU_GD, GD_DEPTH_VAL,
			GD_WIDTH_VAL, GD_REFVOL_VAL);
	}
	if (CONFIG_LPD_GD != 0U) {
		GdCfgWords[NumWords++] = XLoader_PackGlitchDetectorCfg(
			(u8)XLOADER_GD_TYPE_LPD_GD0, GD_DEPTH_VAL,
			GD_WIDTH_VAL, GD_REFVOL_VAL);
		GdCfgWords[NumWords++] = XLoader_PackGlitchDetectorCfg(
			(u8)XLOADER_GD_TYPE_LPD_GD1, GD_DEPTH_VAL,
			GD_WIDTH_VAL, GD_REFVOL_VAL);
	}

	xil_printf("Sending %u glitch detector config word(s) in one IPI\n\r", NumWords);
	Status = XLoader_ConfigureGlitchDetector(&LoaderClientInstance, GdCfgWords, NumWords);
	if (Status != XST_SUCCESS) {
		xil_printf("Glitch detector configuration failed\n\r");
		goto END;
	}

#if (TEST_GLITCH_ENABLE != 0U)
	/**
	 * Test-glitch path is example-local direct MMIO. Loader IPI only covers
	 * calibration configure; there is no IPI for test mode / sel / gen.
	 */
	xil_printf("Generating test glitch via direct MMIO (no Loader IPI)\n\r");
	Status = GenerateSelectedTestGlitches();
	if (Status != XST_SUCCESS) {
		xil_printf("EAM status check: expected glitch not latched\n\r");
	}
#endif

END:
	if (Status == XST_SUCCESS) {
		xil_printf("Successfully ran glitch detector configure example\n\r");
	}
	else {
		xil_printf("Glitch detector configure example failed with error code:%08x\n\r",
			   Status);
	}
	return Status;
}
