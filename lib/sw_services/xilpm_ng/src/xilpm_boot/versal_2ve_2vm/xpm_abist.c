/******************************************************************************
 * Copyright (c) 2025 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
 * SPDX-License-Identifier: MIT
 *****************************************************************************/
/*  File name: xpm_abist.c
 *  It is used for PL related Arteris BIST */

#include "xpm_abist.h"

/*****************************************************************************/
/**
 * @brief Runs Arteris built-in self-test (ABIST) for PL-related LPD, FPD, and ASU Domains.
 *
 * Tests only the selected LPD AFIFS, FPD AFIFS, and INTASU PL controllers
 * whose base-register reads are nonzero.
 * Each ABIST is only required to execute once after booting.
 * Skips testing on QEMU and COSIM platforms or when housecleaning disables
 * PLD ABIST.
 *
 * Temporarily enables the RCLK clock outputs and restores CRP_RCLK_CTRL
 * before returning. For each tested controller, disables interrupts, starts
 * BIST, polls for completion, and checks for a latent fault. Clears BIST
 * completion and polls for mission mode before recording successful
 * completion.
 * Subsequent calls skip controllers already marked complete.
 *
 * @param DomainMask OR of XPM_ABIST_LPD_MASK, XPM_ABIST_FPD_MASK, and
 *                   XPM_ABIST_ASU_MASK. Zero selects no controllers.
 *
 * @return
 *         - XST_SUCCESS if all attempted tests succeed or no test runs.
 *         - XST_INVALID_PARAM for unsupported DomainMask bits.
 *         - XST_FAILURE if a latent fault is detected.
 *         - Polling failure status if BIST completion or mission-mode
 *           polling fails.
 *
 * @note A zero base-register read leaves that controller eligible for a
 *       subsequent call. Success does not imply every controller completed
 *       ABIST. Completion flags persist across calls until software
 *       reinitializes them.
 *
 *****************************************************************************/
XStatus XPmPl_Abist(u32 DomainMask)
{
	XStatus Status = XST_FAILURE;
	static u32 CompletedMask = 0U;
	u32 FaultMask;
	u32 RegValue;
	u32 SavedValue;
	/* Do not run ABIST if in qemu environment */
	u32 Platform = XPm_GetPlatform();
	/* Skip ABIST for QEMU and COSIM platforms */
	u32 SkipAbist = ((PLATFORM_VERSION_QEMU == Platform) ||
			(PLATFORM_VERSION_COSIM == Platform)) ? 1U : 0U;

	/* Check input argument */
	if (0U != (DomainMask & ~XPM_ABIST_ALL_MASK)) {
		Status = XST_INVALID_PARAM;
		goto done;
	}

	if (1U == SkipAbist) {
		Status = XST_SUCCESS;
		goto done;
	}

	if (((CompletedMask & DomainMask) == DomainMask) || (!PM_HOUSECLEAN_CHECK(PLD, ABIST))) {
		Status = XST_SUCCESS;
		goto done;
	}

	/* Save RCLK value*/
	SavedValue = XPm_In32(CRP_RCLK_CTRL);

	/* Set RCLK CLKACT bits */
	XPm_RMW32(CRP_RCLK_CTRL, CRP_RCLK_CTRL_CLKACT0_MASK, CRP_RCLK_CTRL_CLKACT0_MASK);
	XPm_RMW32(CRP_RCLK_CTRL, CRP_RCLK_CTRL_CLKACT1_MASK, CRP_RCLK_CTRL_CLKACT1_MASK);

	/* Check and run ABIST for LPD_AFIFS_GPV */
	/* Reading 0 is not a bug, it only means the configure is not done yet */
	if ((0U != (DomainMask & XPM_ABIST_LPD_MASK)) &&
		(0U == (CompletedMask & XPM_ABIST_LPD_MASK)) &&
		(0U != XPm_In32(LPD_AFIFS_GPV_AFIFS_DTP_PL_BASEADDR))) {
		/* 1. Use polling method to detect BIST completion */
		Xil_Out32(LPD_AFIFS_GPV_AFIFS_DTP_PL_INTEN, 0x0U);
		/* 2. Write 1 in register BistCtl field BistStart to start the BIST sequence. */
		Xil_Out32(LPD_AFIFS_GPV_AFIFS_DTP_PL_BISTCTL, LPD_AFIFS_GPV_AFIFS_DTP_PL_BISTCTL_BISTSTART_MASK);
		/* 3. Once BIST completion is detected by polling */
		Status = XPm_PollForMask(LPD_AFIFS_GPV_AFIFS_DTP_PL_BISTDONE,
			LPD_AFIFS_GPV_AFIFS_DTP_PL_BISTDONE_BISTDONE_MASK, XPM_POLL_TIMEOUT);
		if (XST_SUCCESS != Status) {
			goto restore_clk;
		}
		/* 4. Check the value of register Faults field LatentFault. */
		FaultMask = (LPD_AFIFS_GPV_AFIFS_DTP_PL_FAULTS_LATENTFAULT_MASK);
		RegValue = XPm_In32(LPD_AFIFS_GPV_AFIFS_DTP_PL_FAULTS);
		if ((RegValue & FaultMask) != 0x0U) {
			Status = XST_FAILURE;
			goto restore_clk;
		}
		/* 5. Write 1 into register BistCtl field BistDoneClr.*/
		Xil_Out32(LPD_AFIFS_GPV_AFIFS_DTP_PL_BISTCTL, LPD_AFIFS_GPV_AFIFS_DTP_PL_BISTCTL_BISTDONECLR_MASK);
		/* 6. Poll register BistDone fields BistDone and MissionMode
			until they show values of 0 and 1 respectively */
		Status = (XStatus)XPlmi_UtilPoll(LPD_AFIFS_GPV_AFIFS_DTP_PL_BISTDONE,
				LPD_AFIFS_GPV_AFIFS_DTP_PL_BISTDONE_BISTDONE_MASK|
				LPD_AFIFS_GPV_AFIFS_DTP_PL_BISTDONE_MISSIONMODE_MASK,
				LPD_AFIFS_GPV_AFIFS_DTP_PL_BISTDONE_MISSIONMODE_MASK,
				XPM_POLL_TIMEOUT, NULL);
		if (XST_SUCCESS != Status) {
			goto restore_clk;
		}

		/* Set only after the complete LPD sequence succeeds. */
		CompletedMask |= XPM_ABIST_LPD_MASK;
		XPlmi_Printf(DEBUG_PRINT_ALWAYS,"LPD_AFIFS_PL_GPV pass\r\n");
	}

	if ((0U != (DomainMask & XPM_ABIST_FPD_MASK)) &&
		(0U == (CompletedMask & XPM_ABIST_FPD_MASK)) &&
		(0U != XPm_In32(FPD_AFIFS_GPV_AFIFS_DTP_PL_BASEADDR))) {
		/* 1. Use polling method to detect BIST completion */
		Xil_Out32(FPD_AFIFS_GPV_AFIFS_DTP_PL_INTEN, 0x0U);
		/* 2. Write 1 in register BistCtl field BistStart to start the BIST sequence. */
		Xil_Out32(FPD_AFIFS_GPV_AFIFS_DTP_PL_BISTCTL, FPD_AFIFS_GPV_AFIFS_DTP_PL_BISTCTL_BISTSTART_MASK);
		/* 3. Once BIST completion is detected by polling */
		Status = XPm_PollForMask(FPD_AFIFS_GPV_AFIFS_DTP_PL_BISTDONE,
			FPD_AFIFS_GPV_AFIFS_DTP_PL_BISTDONE_BISTDONE_MASK, XPM_POLL_TIMEOUT);
		if (XST_SUCCESS != Status) {
			goto restore_clk;
		}
		/* 4. Check the value of register Faults field LatentFault. */
		FaultMask = (FPD_AFIFS_GPV_AFIFS_DTP_PL_FAULTS_LATENTFAULT_MASK);
		RegValue = XPm_In32(FPD_AFIFS_GPV_AFIFS_DTP_PL_FAULTS);
		if ((RegValue & FaultMask) != 0x0U) {
			Status = XST_FAILURE;
			goto restore_clk;
		}
		/* 5. Write 1 into register BistCtl field BistDoneClr.*/
		Xil_Out32(FPD_AFIFS_GPV_AFIFS_DTP_PL_BISTCTL, FPD_AFIFS_GPV_AFIFS_DTP_PL_BISTCTL_BISTDONECLR_MASK);
		/* 6. Poll register BistDone fields BistDone and MissionMode
			until they show values of 0 and 1 respectively */
		Status = (XStatus)XPlmi_UtilPoll(FPD_AFIFS_GPV_AFIFS_DTP_PL_BISTDONE,
				FPD_AFIFS_GPV_AFIFS_DTP_PL_BISTDONE_BISTDONE_MASK|
				FPD_AFIFS_GPV_AFIFS_DTP_PL_BISTDONE_MISSIONMODE_MASK,
				FPD_AFIFS_GPV_AFIFS_DTP_PL_BISTDONE_MISSIONMODE_MASK,
				XPM_POLL_TIMEOUT, NULL);
		if (XST_SUCCESS != Status) {
			goto restore_clk;
		}

		/* Set only after the complete FPD sequence succeeds. */
		CompletedMask |= XPM_ABIST_FPD_MASK;
		XPlmi_Printf(DEBUG_PRINT_ALWAYS,"FPD_AFIFS_PL_GPV pass\r\n");
	}

	if ((0U != (DomainMask & XPM_ABIST_ASU_MASK)) &&
		(0U == (CompletedMask & XPM_ABIST_ASU_MASK)) &&
		(0U != XPm_In32(INTASU_GPV_INTASU_PL_BASEADDR))) {
		/* 1. Use polling method to detect BIST completion */
		Xil_Out32(INTASU_GPV_INTASU_PL_INTEN, 0x0U);
		/* 2. Write 1 in register BistCtl field BistStart to start the BIST sequence. */
		Xil_Out32(INTASU_GPV_INTASU_PL_BISTCTL, INTASU_GPV_INTASU_PL_BISTCTL_BISTSTART_MASK);
		/* 3. Once BIST completion is detected by polling */
		Status = XPm_PollForMask(INTASU_GPV_INTASU_PL_BISTDONE,
			INTASU_GPV_INTASU_PL_BISTDONE_BISTDONE_MASK, XPM_POLL_TIMEOUT);
		if (XST_SUCCESS != Status) {
			goto restore_clk;
		}
		/* 4. Check the value of register Faults field LatentFault. */
		FaultMask = (INTASU_GPV_INTASU_PL_FAULTS_LATENTFAULT_MASK);
		RegValue = XPm_In32(INTASU_GPV_INTASU_PL_FAULTS);
		if ((RegValue & FaultMask) != 0x0U) {
			Status = XST_FAILURE;
			goto restore_clk;
		}
		/* 5. Write 1 into register BistCtl field BistDoneClr.*/
		Xil_Out32(INTASU_GPV_INTASU_PL_BISTCTL, INTASU_GPV_INTASU_PL_BISTCTL_BISTDONECLR_MASK);
		/* 6. Poll register BistDone fields BistDone and MissionMode
			until they show values of 0 and 1 respectively */
		Status = (XStatus)XPlmi_UtilPoll(INTASU_GPV_INTASU_PL_BISTDONE,
				INTASU_GPV_INTASU_PL_BISTDONE_BISTDONE_MASK|
				INTASU_GPV_INTASU_PL_BISTDONE_MISSIONMODE_MASK,
				INTASU_GPV_INTASU_PL_BISTDONE_MISSIONMODE_MASK,
				XPM_POLL_TIMEOUT, NULL);
		if (XST_SUCCESS != Status) {
			goto restore_clk;
		}

		/* Set only after the complete ASU sequence succeeds. */
		CompletedMask |= XPM_ABIST_ASU_MASK;
		XPlmi_Printf(DEBUG_PRINT_ALWAYS,"INTASU_PL_GPV pass\r\n");
	}

	Status = XST_SUCCESS;

restore_clk:
	/* Restore the value of RCLK_CTRL */
	XPm_Out32(CRP_RCLK_CTRL, SavedValue);
done:
	if (XST_SUCCESS != Status) {
		PmErr("ABIST failed: 0x%x\n\r", (unsigned int)Status);
	}
	return Status;
}
