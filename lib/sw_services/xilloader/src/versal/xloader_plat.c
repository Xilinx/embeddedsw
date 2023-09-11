/******************************************************************************
* Copyright (c) 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2023, Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file versal/xloader_plat.c
*
* This file contains the versal specific code related to PDI image loading.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- -------- -------------------------------------------------------
* 1.00  bm   07/06/2022 Initial release
*       bsv  07/21/2022 DDR modelling related changes
*       bm   07/21/2022 Retain critical data structures after In-Place PLM Update
*       is   09/12/2022 Remove PM_CAP_SECURE capability when requesting PSM_PROC,
*                       TCM memory banks
* 1.01  ng   11/11/2022 Updated doxygen comments
*       sk   02/22/2023 Added EoPDI SYNC logic to handle Slave PDI load errors
*       ng   03/12/2023 Fixed Coverity warnings
*       bm   03/16/2023 Added print when DDRMC dump is skipped
*       dd   03/28/2023 Updated doxygen comments
*       ng   03/30/2023 Updated algorithm and return values in doxygen comments
*       sk   05/31/2023 Addded function to get Bootpdiinfo storage
*       sk   06/12/2023 Removed XLoader_GetPdiInstance function definition
*       rama 08/10/2023 Changed DDRMC register dump prints to DEBUG_ALWAYS for
*                       debug level_0 option
*       dd   09/11/2023 MISRA-C violation Directive 4.5 fixed
*       dd	 09/11/2023 MISRA-C violation Rule 10.3 fixed
*
* </pre>
*
* @note
*
******************************************************************************/

/***************************** Include Files *********************************/
#include "xloader.h"
#include "xilpdi.h"
#include "xpm_device.h"
#include "xpm_api.h"
#include "xpm_subsystem.h"
#include "xpm_nodeid.h"
#include "xplmi.h"
#include "xplmi_hw.h"
#include "xplmi_ssit.h"
#include "xplmi_util.h"
#include "xloader_plat.h"
#include "xplmi_err.h"
#include "xloader_ddr.h"
#include "xilpdi.h"

/************************** Constant Definitions *****************************/
#define XLOADER_TCM_0		(0U) /**< TCM 0 */
#define XLOADER_TCM_1		(1U) /**< TCM 1 */
#define XLOADER_RPU_GLBL_CNTL	(0xFF9A0000U) /**< RPU global control */
#define XLOADER_TCMCOMB_MASK	(0x40U) /**< TCM combine mask */
#define XLOADER_TCMCOMB_SHIFT	(6U) /**< TCM combine shift */

#define PLM_VP1802_POR_SETTLE_TIME	(25000U) /**< Flag indicates POR
                                                  * settle time for VP1802 */
/**
 * @{
 * @cond DDR calibration errors
 */
#define DDRMC_OFFSET_CALIB_ERR		(0x840CU)
#define DDRMC_OFFSET_CALIB_ERR_NIBBLE_1	(0x8420U)
#define DDRMC_OFFSET_CALIB_ERR_NIBBLE_2	(0x841CU)
#define DDRMC_OFFSET_CALIB_ERR_NIBBLE_3	(0x8418U)
#define DDRMC_OFFSET_CALIB_STAGE_PTR	(0x8400U)
/**
 * @}
 * @endcond
 */

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/
static void XLoader_A72Config(u32 CpuId, u32 ExecState, u32 VInitHi);
static int XLoader_RequestTCM(u8 TcmId);
static int XLoader_CheckHandoffCpu(const XilPdi* PdiPtr, const u32 DstnCpu);
static int XLoader_GetLoadAddr(u32 DstnCpu, u64 *LoadAddrPtr, u32 Len);
static int XLoader_DumpDdrmcRegisters(void);

/************************** Variable Definitions *****************************/

/*****************************************************************************/
/**
 * @brief	This function provides ImageInfoTbl pointer
 *
 * @return	Pointer to ImageInfoTbl
 *
 *****************************************************************************/
XLoader_ImageInfoTbl *XLoader_GetImageInfoTbl(void)
{
	/* Image Info Table */
	static XLoader_ImageInfoTbl ImageInfoTbl = {
		.Count = 0U,
		.IsBufferFull = FALSE,
	};

	return &ImageInfoTbl;
}

/*****************************************************************************/
/**
 * @brief	This function provides pointer to PdiList
 *
 * @return	Pointer to PdiList
 *
 *****************************************************************************/
XLoader_ImageStore* XLoader_GetPdiList(void)
{
	static XLoader_ImageStore PdiList __attribute__ ((aligned(4U))) = {0U};

	return &PdiList;
}

/****************************************************************************/
/**
* @brief	This function returns the ATFHandoffParams structure address to
*           the caller.
*
* @return	Returns ATFHandoffParams structure address
*
*****************************************************************************/
XilPdi_ATFHandoffParams *XLoader_GetATFHandoffParamsAddr(void)
{
	static XilPdi_ATFHandoffParams ATFHandoffParams = {0U}; /**< Instance containing
								 ATF handoff params */
	/* Return ATF Handoff parameters structure address */
	return &ATFHandoffParams;
}

/*****************************************************************************/
/**
 * @brief	This function provides pointer to BootPDI Info
 *
 * @return	pointer to BootPDI Info
 *
 *****************************************************************************/
XilBootPdiInfo* XLoader_GetBootPdiInfo(void)
{
	static XilBootPdiInfo BootPdiInfo
		__attribute__ ((aligned(4U))) = {0}; /** < BootPDI info Storage */

	return &BootPdiInfo;
}

/*****************************************************************************/
/**
 * @brief	This function is used to start the subsystems in the PDI.
 *
 * @param	PdiPtr Pdi instance pointer
 *
 * @return
 * 			- XST_SUCCESS on success.
 * 			- XLOADER_ERR_WAKEUP_A72_0 if waking up the A72-0 failed during
 * 			handoff.
 * 			- XLOADER_ERR_WAKEUP_A72_1 if waking up the A72-1 failed during
 * 			handoff.
 * 			- XLOADER_ERR_WAKEUP_R5_0 if waking up the R5-0 failed during
 * 			handoff.
 * 			- XLOADER_ERR_WAKEUP_R5_1 if waking up the R5-1 failed during
 * 			handoff.
 * 			- XLOADER_ERR_WAKEUP_R5_L if waking up the R5-L failed during
 * 			handoff.
 * 			- XLOADER_ERR_WAKEUP_PSM if waking up the PSM failed during
 * 			handoff.
 *
 *****************************************************************************/
int XLoader_StartImage(XilPdi *PdiPtr)
{
	int Status = XST_FAILURE;
	u32 Index;
	u32 CpuId;
	u64 HandoffAddr;
	u32 ExecState;
	u32 VInitHi;
	u32 DeviceId;
	u8 SetAddress = 1U;
	u32 ErrorCode;

	/** - Start Handoff to the cpus */
	for (Index = 0U; Index < PdiPtr->NoOfHandoffCpus; Index++) {
		CpuId = PdiPtr->HandoffParam[Index].CpuSettings &
			XIH_PH_ATTRB_DSTN_CPU_MASK;

		HandoffAddr = PdiPtr->HandoffParam[Index].HandoffAddr;
		ExecState = PdiPtr->HandoffParam[Index].CpuSettings &
				XIH_PH_ATTRB_A72_EXEC_ST_MASK;
		VInitHi = PdiPtr->HandoffParam[Index].CpuSettings &
				XIH_PH_ATTRB_HIVEC_MASK;
		Status = XST_FAILURE;
		/** - Wake up each processor */
		switch (CpuId)
		{
			case XIH_PH_ATTRB_DSTN_CPU_A72_0:
				/* APU Core configuration */
				XLoader_A72Config(CpuId, ExecState, VInitHi);
				DeviceId = PM_DEV_ACPU_0;
				ErrorCode = (u32)XLOADER_ERR_WAKEUP_A72_0;
				XLoader_Printf(DEBUG_INFO, "Request APU0 "
						"wakeup\r\n");
				break;
			case XIH_PH_ATTRB_DSTN_CPU_A72_1:
				/* APU Core configuration */
				XLoader_A72Config(CpuId, ExecState, VInitHi);
				DeviceId = PM_DEV_ACPU_1;
				ErrorCode = (u32)XLOADER_ERR_WAKEUP_A72_1;
				XLoader_Printf(DEBUG_INFO, "Request APU1"
						"wakeup\r\n");
				break;
			case XIH_PH_ATTRB_DSTN_CPU_R5_0:
				DeviceId = PM_DEV_RPU0_0;
				ErrorCode = (u32)XLOADER_ERR_WAKEUP_R5_0;
				XLoader_Printf(DEBUG_INFO, "Request RPU 0 "
						"wakeup\r\n");
				break;
			case XIH_PH_ATTRB_DSTN_CPU_R5_1:
				DeviceId = PM_DEV_RPU0_1;
				ErrorCode = (u32)XLOADER_ERR_WAKEUP_R5_1;
				XLoader_Printf(DEBUG_INFO, "Request RPU 1 "
						"wakeup\r\n");
				break;
			case XIH_PH_ATTRB_DSTN_CPU_R5_L:
				DeviceId = PM_DEV_RPU0_0;
				ErrorCode = (u32)XLOADER_ERR_WAKEUP_R5_L;
				XLoader_Printf(DEBUG_INFO, "Request RPU "
						"wakeup\r\n");
				break;
			case XIH_PH_ATTRB_DSTN_CPU_PSM:
				DeviceId = PM_DEV_PSM_PROC;
				ErrorCode = (u32)XLOADER_ERR_WAKEUP_PSM;
				SetAddress = 0U;
				XLoader_Printf(DEBUG_INFO, "Request PSM wakeup\r\n");
				break;

			default:
				Status = XST_SUCCESS;
				break;
		}
		if (Status != XST_SUCCESS) {
			Status = XPm_RequestWakeUp(PM_SUBSYS_PMC, DeviceId,
				SetAddress, HandoffAddr, 0U, XPLMI_CMD_SECURE);
			if (Status != XST_SUCCESS) {
				Status = XPlmi_UpdateStatus((XPlmiStatus_t)ErrorCode, Status);
				goto END;
			}
		}
	}

	Status = XST_SUCCESS;

END:
	/* Make Number of handoff CPUs to zero */
	PdiPtr->NoOfHandoffCpus = 0x0U;
	return Status;
}

/****************************************************************************/
/**
* @brief	This function sets the handoff parameters to the ARM Trusted
* 			Firmware(ATF). Some of the inputs for this are taken from image
* 			partition header. A pointer to the structure containing these
* 			parameters is stored in the PMC_GLOBAL.GLOBAL_GEN_STORAGE4
* 			register, which ATF reads.
*
* @param	PrtnHdr is pointer to Partition header details
*
* @return
* 			- None
*
*****************************************************************************/
void XLoader_SetATFHandoffParameters(const XilPdi_PrtnHdr *PrtnHdr)
{
	u32 PrtnAttrbs;
	u32 PrtnFlags;
	u32 LoopCount = 0U;
	XilPdi_ATFHandoffParams *ATFHandoffParams = XLoader_GetATFHandoffParamsAddr();

	PrtnAttrbs = PrtnHdr->PrtnAttrb;

	/**
	 * - Read partition header and deduce entry point and partition flags.
	 */
	PrtnFlags =
		(((PrtnAttrbs & XIH_PH_ATTRB_A72_EXEC_ST_MASK)
				>> XIH_ATTRB_A72_EXEC_ST_SHIFT_DIFF) |
		((PrtnAttrbs & XIH_PH_ATTRB_ENDIAN_MASK)
				>> XIH_ATTRB_ENDIAN_SHIFT_DIFF) |
		((PrtnAttrbs & XIH_PH_ATTRB_TZ_SECURE_MASK)
				<< XIH_ATTRB_TR_SECURE_SHIFT_DIFF) |
		((PrtnAttrbs & XIH_PH_ATTRB_TARGET_EL_MASK)
				<< XIH_ATTRB_TARGET_EL_SHIFT_DIFF));

	PrtnAttrbs &= XIH_PH_ATTRB_DSTN_CPU_MASK;
	/** - Update CPU number based on destination CPU */
	if (PrtnAttrbs == XIH_PH_ATTRB_DSTN_CPU_A72_0) {
		PrtnFlags |= XIH_PRTN_FLAGS_DSTN_CPU_A72_0;
	}
	else if (PrtnAttrbs == XIH_PH_ATTRB_DSTN_CPU_A72_1) {
		PrtnFlags |= XIH_PRTN_FLAGS_DSTN_CPU_A72_1;
	}
	else if (PrtnAttrbs == XIH_PH_ATTRB_DSTN_CPU_NONE) {
		/*
		 * This is required for u-boot handoff to work
		 * when BOOTGEN_SUBSYSTEM_PDI is set to 0 in bootgen
		 */
		PrtnFlags &= (~(XIH_ATTRB_EL_MASK) | XIH_PRTN_FLAGS_EL_2)
					| XIH_PRTN_FLAGS_DSTN_CPU_A72_0;
	}
	else {
		/* MISRA-C compliance */
	}

	if (ATFHandoffParams->NumEntries == 0U) {
		/* Insert magic string */
		ATFHandoffParams->MagicValue[0U] = 'X';
		ATFHandoffParams->MagicValue[1U] = 'L';
		ATFHandoffParams->MagicValue[2U] = 'N';
		ATFHandoffParams->MagicValue[3U] = 'X';
	}
	else {
		for (; LoopCount < ATFHandoffParams->NumEntries;
			LoopCount++) {
			if (ATFHandoffParams->Entry[LoopCount].PrtnFlags ==
					PrtnFlags) {
				ATFHandoffParams->Entry[LoopCount].EntryPoint =
					PrtnHdr->DstnExecutionAddr;
				break;
			}
		}
	}

	if ((ATFHandoffParams->NumEntries < XILPDI_MAX_ENTRIES_FOR_ATF) &&
		(ATFHandoffParams->NumEntries == LoopCount)) {
		if((PrtnFlags & XIH_ATTRB_EL_MASK) != XIH_PRTN_FLAGS_EL_3) {
			ATFHandoffParams->NumEntries++;
			ATFHandoffParams->Entry[LoopCount].EntryPoint =
					PrtnHdr->DstnExecutionAddr;
			ATFHandoffParams->Entry[LoopCount].PrtnFlags = PrtnFlags;
		}
	}
}

/*****************************************************************************/
/**
 * @brief	This function sets Aarch state and vector location for APU.
 *
 * @param	CpuId CPU ID
 * @param	ExecState CPU execution state
 * @param	VInitHi resembles highvec configuration for CPU
 *
 * @return
 * 			- None
 *
 *****************************************************************************/
static void XLoader_A72Config(u32 CpuId, u32 ExecState, u32 VInitHi)
{
	u32 RegVal = Xil_In32(XLOADER_FPD_APU_CONFIG_0);
	u32 ExecMask = 0U;
	u32 VInitHiMask = 0U;

	switch(CpuId)
	{
		case XIH_PH_ATTRB_DSTN_CPU_A72_0:
			ExecMask = XLOADER_FPD_APU_CONFIG_0_AA64N32_MASK_CPU0;
			VInitHiMask = XLOADER_FPD_APU_CONFIG_0_VINITHI_MASK_CPU0;
			break;
		case XIH_PH_ATTRB_DSTN_CPU_A72_1:
			ExecMask = XLOADER_FPD_APU_CONFIG_0_AA64N32_MASK_CPU1;
			VInitHiMask = XLOADER_FPD_APU_CONFIG_0_VINITHI_MASK_CPU1;
			break;
		default:
			break;
	}
	/** Set Aarch state 64 Vs 32 bit and vection location for 32 bit */
	if (ExecState == XIH_PH_ATTRB_A72_EXEC_ST_AA64) {
		RegVal |= ExecMask;
	}
	else {
		RegVal &= ~(ExecMask);
		if (VInitHi == XIH_PH_ATTRB_HIVEC_MASK) {
			RegVal |= VInitHiMask;
		}
		else {
			RegVal &= ~(VInitHiMask);
		}
	}
	/** Update the APU configuration */
	XPlmi_Out32(XLOADER_FPD_APU_CONFIG_0, RegVal);
}

/*****************************************************************************/
/**
 * @brief	This function is used to run MJTAG solution workaround in which
 * 			JTAG Tap state will be set to reset.
 *
 *
 * @return
 * 			- None
 *
 *****************************************************************************/
void XLoader_SetJtagTapToReset(void)
{
	u8 Index = 0U;
	u32 Flag = 0U;
	u32 Val = 0U;

	/**
	 * - Based on Vivado property, check whether to apply MJTAG workaround
	 * or not. By default vivado property disables MJTAG workaround.
	 */
	Val = XPlmi_In32(XPLMI_RTCFG_PLM_MJTAG_WA);
	Flag = Val & XPLMI_RTCFG_PLM_MJTAG_WA_IS_ENABLED_MASK;
	if (Flag != XPLMI_RTCFG_PLM_MJTAG_WA_IS_ENABLED_MASK) {
		goto END;
	}

	/** - Skip applying MJTAG workaround if already applied */
	Flag = ((Val & XPLMI_RTCFG_PLM_MJTAG_WA_STATUS_MASK) >>
			XPLMI_RTCFG_PLM_MJTAG_WA_STATUS_SHIFT);
	if (Flag != 0U) {
		goto END;
	}

	/** - Check if End of PL Startup is asserted or not */
	Flag = ((XPlmi_In32(CFU_APB_CFU_FGCR) & CFU_APB_CFU_FGCR_EOS_MASK));
	if (Flag != CFU_APB_CFU_FGCR_EOS_MASK) {
		goto END;
	}

	/** - Enable MJTAG */
	XPlmi_Out32(PMC_TAP_JTAG_TEST, 1U);

	/** - Toggle MJTAG ISO to generate clock pulses, default 10 clock pulses */
	for (Index = 0U; Index < XPLMI_MJTAG_WA_GASKET_TOGGLE_CNT; Index++) {
		XPlmi_UtilRMW(PMC_GLOBAL_DOMAIN_ISO_CNTRL,
				PMC_GLOBAL_DOMAIN_ISO_CNTRL_PMC_PL_TEST_MASK,
				0U);
		XPlmi_UtilRMW(PMC_GLOBAL_DOMAIN_ISO_CNTRL,
				PMC_GLOBAL_DOMAIN_ISO_CNTRL_PMC_PL_TEST_MASK,
				0U);

		/* Delay in between low and high states of toggle */
		usleep(XPLMI_MJTAG_WA_DELAY_USED_IN_GASKET_TOGGLE);

		XPlmi_UtilRMW(PMC_GLOBAL_DOMAIN_ISO_CNTRL,
				PMC_GLOBAL_DOMAIN_ISO_CNTRL_PMC_PL_TEST_MASK,
				PMC_GLOBAL_DOMAIN_ISO_CNTRL_PMC_PL_TEST_MASK);
		XPlmi_UtilRMW(PMC_GLOBAL_DOMAIN_ISO_CNTRL,
				PMC_GLOBAL_DOMAIN_ISO_CNTRL_PMC_PL_TEST_MASK,
				PMC_GLOBAL_DOMAIN_ISO_CNTRL_PMC_PL_TEST_MASK);

		/* Delay in between high and low states of toggle */
		usleep(XPLMI_MJTAG_WA_DELAY_USED_IN_GASKET_TOGGLE);
	}

	/** - Disable MJTAG */
	XPlmi_Out32(PMC_TAP_JTAG_TEST, 0U);

	XPlmi_UtilRMW(XPLMI_RTCFG_PLM_MJTAG_WA,
			XPLMI_RTCFG_PLM_MJTAG_WA_STATUS_MASK,
			XPLMI_RTCFG_PLM_MJTAG_WA_STATUS_MASK);
END:
	return;
}

/*****************************************************************************/
/**
 * @brief	This function is used to get PdiSrc and PdiAddr for Secondary
 * 			SD boot modes

 * @param	SecBootMode is the secondary boot mode value
 * @param	PdiSrc is pointer to the source of PDI
 * @param	PdiAddr is the pointer to the address of the Pdi

 @return
 * 			- XST_SUCCESS on success.
 * 			- XLOADER_ERR_UNSUPPORTED_SEC_BOOT_MODE on unsupported secondary
 * 			bootmode.
 *
 *****************************************************************************/
int XLoader_GetSDPdiSrcNAddr(u32 SecBootMode, XilPdi *PdiPtr, u32 *PdiSrc,
		u64 *PdiAddr)
{
	int Status = XST_FAILURE;
	(void)PdiPtr;

	/** - Get the PDI source address for the secondary boot device. */
	switch(SecBootMode)
	{
		case XIH_IHT_ATTR_SBD_SD_0:
		#ifdef XLOADER_SD_0
			*PdiSrc = XLOADER_PDI_SRC_SD0 |
				XLOADER_SD_RAWBOOT_MASK |
				(PdiPtr->MetaHdr.ImgHdrTbl.SBDAddr
				<< XLOADER_SD_ADDR_SHIFT);
		#else
			*PdiSrc = XLOADER_PDI_SRC_SD0;
		#endif
			*PdiAddr = 0U;
			break;
		case XIH_IHT_ATTR_SBD_SD_1:
		#ifdef XLOADER_SD_1
			*PdiSrc = XLOADER_PDI_SRC_SD1 |
				XLOADER_SD_RAWBOOT_MASK |
				(PdiPtr->MetaHdr.ImgHdrTbl.SBDAddr
				<< XLOADER_SD_ADDR_SHIFT);
		#else
			*PdiSrc = XLOADER_PDI_SRC_SD1;
		#endif
			*PdiAddr = 0U;
			break;
		case XIH_IHT_ATTR_SBD_SD_LS:
		#ifdef XLOADER_SD_1
			*PdiSrc = XLOADER_PDI_SRC_SD1_LS |
				XLOADER_SD_RAWBOOT_MASK |
				(PdiPtr->MetaHdr.ImgHdrTbl.SBDAddr
				<< XLOADER_SD_ADDR_SHIFT);
		#else
			*PdiSrc = XLOADER_PDI_SRC_SD1_LS;
		#endif
			*PdiAddr = 0U;
			break;
		case XIH_IHT_ATTR_SBD_SD_0_RAW:
			*PdiSrc = XLOADER_SD_RAWBOOT_VAL | XLOADER_PDI_SRC_SD0;
			break;
		case XIH_IHT_ATTR_SBD_SD_1_RAW:
			*PdiSrc = XLOADER_SD_RAWBOOT_VAL | XLOADER_PDI_SRC_SD1;
			break;
		case XIH_IHT_ATTR_SBD_SD_LS_RAW:
			*PdiSrc = XLOADER_SD_RAWBOOT_VAL | XLOADER_PDI_SRC_SD1_LS;
			break;
		default:
			Status = (int)XLOADER_ERR_UNSUPPORTED_SEC_BOOT_MODE;
			break;
	}

	if (Status != (int)XLOADER_ERR_UNSUPPORTED_SEC_BOOT_MODE) {
		Status = XST_SUCCESS;
	}

	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function copies the elf partitions to specified destinations.
 *
 * @param	PdiPtr is pointer to XilPdi instance
 * @param	PrtnHdr is pointer to the partition header
 * @param	PrtnParams is pointer to the structure variable that contains
 *			parameters required to process the partition
 * @param	SecureParams is pointer to the instance containing security related
 *			params
 *
 * @return
 * 			- XST_SUCCESS on success.
 * 			- XLOADER_ERR_INVALID_ELF_LOAD_ADDR if the load address of the
 * 			elf is invalid.
 * 			- XLOADER_ERR_PM_DEV_PSM_PROC if device requet for PSM is failed.
 * 			- XLOADER_ERR_PM_DEV_IOCTL_RPU0_SPLIT if IOCTL call to set RPU0 in
 * 			split mode fails.
 * 			- XLOADER_ERR_PM_DEV_IOCTL_RPU1_SPLIT if IOCTL call to set RPU1 in
 * 			split mode fails.
 * 			- XLOADER_ERR_PM_DEV_IOCTL_RPU0_LOCKSTEP if IOCTL call to set RPU0
 * 			in lockstep mode fails.
 * 			- XLOADER_ERR_PM_DEV_IOCTL_RPU1_LOCKSTEP if IOCTL call to set RPU1
 * 			in lockstep mode fails.
 * 			- XLOADER_ERR_INVALID_TCM_ADDR on Invalid TCM address for A72 elfs.
 *
 *****************************************************************************/
int XLoader_ProcessElf(XilPdi* PdiPtr, const XilPdi_PrtnHdr * PrtnHdr,
	XLoader_PrtnParams* PrtnParams, XLoader_SecureParams* SecureParams)
{
	int Status = XST_FAILURE;
	u32 CapAccess = (u32)PM_CAP_ACCESS;
	u32 CapContext = (u32)PM_CAP_CONTEXT;
	u32 Len = PrtnHdr->UnEncDataWordLen << XPLMI_WORD_LEN_SHIFT;
	u64 EndAddr = PrtnParams->DeviceCopy.DestAddr + Len - 1U;
	u32 ErrorCode;
	u32 Mode = 0U;
	u8 TcmComb;

	/**
	 * - Verify the load address.
	 */
	Status = XPlmi_VerifyAddrRange(PrtnParams->DeviceCopy.DestAddr, EndAddr);
	if (Status != XST_SUCCESS) {
		Status = XPlmi_UpdateStatus(XLOADER_ERR_INVALID_ELF_LOAD_ADDR,
				Status);
		goto END;
	}
	PrtnParams->DstnCpu = XilPdi_GetDstnCpu(PrtnHdr);

	/**
	 *
	 * - For PSM, PSM should be taken out of reset before loading.
	 * PSM RAM should be ECC initialized
	 *
	 * - For OCM, RAM should be ECC initialized
	 *
	 * - R5 should be taken out of reset before loading.
	 * R5 TCM should be ECC initialized
	 */
	if (PrtnParams->DstnCpu == XIH_PH_ATTRB_DSTN_CPU_PSM) {
		Status = XPm_RequestDevice(PM_SUBSYS_PMC, PM_DEV_PSM_PROC,
			(CapAccess | CapContext), XPM_DEF_QOS, 0U, XPLMI_CMD_SECURE);
		if (Status != XST_SUCCESS) {
			Status = XPlmi_UpdateStatus(XLOADER_ERR_PM_DEV_PSM_PROC, 0);
			goto END;
		}
		goto END1;
	}

	if (PrtnParams->DstnCpu == XIH_PH_ATTRB_DSTN_CPU_R5_0) {
		Status = XPm_DevIoctl(PM_SUBSYS_PMC, PM_DEV_RPU0_0,
			IOCTL_SET_RPU_OPER_MODE, XPM_RPU_MODE_SPLIT, 0U, 0U, &Mode,
			XPLMI_CMD_SECURE);
		ErrorCode = (u32)XLOADER_ERR_PM_DEV_IOCTL_RPU0_SPLIT;
	}
	else if (PrtnParams->DstnCpu == XIH_PH_ATTRB_DSTN_CPU_R5_1) {
		Status = XPm_DevIoctl(PM_SUBSYS_PMC, PM_DEV_RPU0_1,
			IOCTL_SET_RPU_OPER_MODE, XPM_RPU_MODE_SPLIT, 0U, 0U, &Mode,
			XPLMI_CMD_SECURE);
		ErrorCode = (u32)XLOADER_ERR_PM_DEV_IOCTL_RPU1_SPLIT;
	}
	else if (PrtnParams->DstnCpu == XIH_PH_ATTRB_DSTN_CPU_R5_L) {
		Status = XPm_DevIoctl(PM_SUBSYS_PMC, PM_DEV_RPU0_0,
			IOCTL_SET_RPU_OPER_MODE, XPM_RPU_MODE_LOCKSTEP, 0U, 0U,
			&Mode, XPLMI_CMD_SECURE);
		if (Status != XST_SUCCESS) {
			Status = XPlmi_UpdateStatus(
				XLOADER_ERR_PM_DEV_IOCTL_RPU0_LOCKSTEP, 0);
			goto END;
		}
		Status = XPm_DevIoctl(PM_SUBSYS_PMC, PM_DEV_RPU0_1,
			IOCTL_SET_RPU_OPER_MODE, XPM_RPU_MODE_LOCKSTEP, 0U, 0U,
			&Mode, XPLMI_CMD_SECURE);
		ErrorCode = (u32)XLOADER_ERR_PM_DEV_IOCTL_RPU1_LOCKSTEP;
	}
	else {
		/* MISRA-C compliance */
	}
	if (Status != XST_SUCCESS) {
		Status = XPlmi_UpdateStatus((XPlmiStatus_t)ErrorCode, 0);
		goto END;
	}

	if ((PrtnParams->DstnCpu == XIH_PH_ATTRB_DSTN_CPU_R5_0) ||
		(PrtnParams->DstnCpu == XIH_PH_ATTRB_DSTN_CPU_R5_L)) {
		Status = XLoader_RequestTCM(XLOADER_TCM_0);
	}
	if ((PrtnParams->DstnCpu == XIH_PH_ATTRB_DSTN_CPU_R5_1) ||
		(PrtnParams->DstnCpu == XIH_PH_ATTRB_DSTN_CPU_R5_L)) {
		Status = XLoader_RequestTCM(XLOADER_TCM_1);
	}
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = XLoader_GetLoadAddr(PrtnParams->DstnCpu,
		&PrtnParams->DeviceCopy.DestAddr, Len);
	if (XST_SUCCESS != Status) {
		goto END;
	}

	if ((PrtnParams->DstnCpu != XIH_PH_ATTRB_DSTN_CPU_A72_0) &&
		(PrtnParams->DstnCpu != XIH_PH_ATTRB_DSTN_CPU_A72_1)) {
		goto END1;
	}

	EndAddr = PrtnParams->DeviceCopy.DestAddr + Len - 1U;
	if (((PrtnParams->DeviceCopy.DestAddr >= XLOADER_R5_1_TCM_A_BASE_ADDR)
		&& (EndAddr <= XLOADER_R5_1_TCM_A_END_ADDR)) ||
		((PrtnParams->DeviceCopy.DestAddr >= XLOADER_R5_1_TCM_B_BASE_ADDR)
		&& (EndAddr <= XLOADER_R5_1_TCM_B_END_ADDR))) {
		/* TCM 1 is in use */
		/* Only allow if TCM is in split mode */
		TcmComb = (u8)((XPlmi_In32(XLOADER_RPU_GLBL_CNTL) &
			XLOADER_TCMCOMB_MASK) >> XLOADER_TCMCOMB_SHIFT);
		if (TcmComb == (u8)FALSE) {
			Status = XLoader_RequestTCM(XLOADER_TCM_1);
		}
		else {
			Status = XPlmi_UpdateStatus(
				XLOADER_ERR_INVALID_TCM_ADDR, 0);
		}
	}
	else if (((PrtnParams->DeviceCopy.DestAddr >=
		XLOADER_R5_0_TCM_A_BASE_ADDR) &&
		(EndAddr <= XLOADER_R5_0_TCM_A_END_ADDR)) ||
		((PrtnParams->DeviceCopy.DestAddr >=
		  XLOADER_R5_0_TCM_B_BASE_ADDR) &&
		 (EndAddr <= XLOADER_R5_0_TCM_B_END_ADDR))) {
		/* TCM 0 is in use */
		Status = XLoader_RequestTCM(XLOADER_TCM_0);
	}
	else if ((PrtnParams->DeviceCopy.DestAddr >= XLOADER_R5_0_TCM_A_BASE_ADDR)
		&& (EndAddr <= XLOADER_R5_LS_TCM_END_ADDR)) {
		/* TCM COMB is in use */
		TcmComb = (u8)((XPlmi_In32(XLOADER_RPU_GLBL_CNTL) &
			XLOADER_TCMCOMB_MASK) >> XLOADER_TCMCOMB_SHIFT);
		if (TcmComb == (u8)TRUE) {
			Status = XLoader_RequestTCM(XLOADER_TCM_0);
			if (Status != XST_SUCCESS) {
				goto END;
			}
			Status = XLoader_RequestTCM(XLOADER_TCM_1);
		}
		else {
			Status = XPlmi_UpdateStatus(
				XLOADER_ERR_INVALID_TCM_ADDR, 0);
		}
	}
	if (Status != XST_SUCCESS) {
		goto END;
	}

END1:
	/**
	 * - Copy the partition to the load address.
	 */
	Status = XLoader_PrtnCopy(PdiPtr, &PrtnParams->DeviceCopy, SecureParams);
	if (XST_SUCCESS != Status) {
		goto END;
	}

	if ((PrtnParams->DstnCpu == XIH_PH_ATTRB_DSTN_CPU_A72_0) ||
		(PrtnParams->DstnCpu == XIH_PH_ATTRB_DSTN_CPU_A72_1)) {
		/**
		 *  - Populate handoff parameters to ATF.
		 *  These correspond to the partitions of application
		 *  which ATF will be loading.
		 */
		XLoader_SetATFHandoffParameters(PrtnHdr);
	}

	if (PdiPtr->DelayHandoff == (u8)FALSE) {
		/* Update the handoff values */
		Status = XLoader_UpdateHandoffParam(PdiPtr);
	}

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function requests TCM_0_A, TCM_0_B, TCM_1_A and TCM_1_B
 * 			depending upon input param and R5-0 and R5-1 cores as required
 * 			for TCMs.
 *
 * @param	TcmId denotes TCM_0 or TCM_1
 *
 * @return
 * 			- XST_SUCCESS on success.
 * 			- XLOADER_ERR_PM_DEV_TCM_0_A if device request for TCM_0_A is
 * 			failed.
 * 			- XLOADER_ERR_PM_DEV_TCM_0_B if device request for TCM_0_B is
 * 			failed.
 * 			- XLOADER_ERR_PM_DEV_TCM_1_A if device request for TCM_1_A is
 * 			failed.
 * 			- XLOADER_ERR_PM_DEV_TCM_1_B if device request for TCM_1_B is
 * 			failed.
 *
 *****************************************************************************/
static int XLoader_RequestTCM(u8 TcmId)
{
	int Status = XST_FAILURE;
	u32 CapAccess = (u32)PM_CAP_ACCESS;
	u32 CapContext = (u32)PM_CAP_CONTEXT;
	u32 ErrorCode;

	if (TcmId == XLOADER_TCM_0) {
		Status = XPm_RequestDevice(PM_SUBSYS_PMC, PM_DEV_TCM_0_A,
			(CapAccess | CapContext), XPM_DEF_QOS, 0U,
			XPLMI_CMD_SECURE);
		if (Status != XST_SUCCESS) {
			ErrorCode = (u32)XLOADER_ERR_PM_DEV_TCM_0_A;
			goto END;
		}
		Status = XPm_RequestDevice(PM_SUBSYS_PMC, PM_DEV_TCM_0_B,
			(CapAccess | CapContext), XPM_DEF_QOS, 0U,
			XPLMI_CMD_SECURE);
		ErrorCode = (u32)XLOADER_ERR_PM_DEV_TCM_0_B;
	}
	else if (TcmId == XLOADER_TCM_1) {
		Status = XPm_RequestDevice(PM_SUBSYS_PMC, PM_DEV_TCM_1_A,
			(CapAccess | CapContext), XPM_DEF_QOS, 0U,
			XPLMI_CMD_SECURE);
		if (Status != XST_SUCCESS) {
			ErrorCode = (u32)XLOADER_ERR_PM_DEV_TCM_1_A;
			goto END;
		}

		Status = XPm_RequestDevice(PM_SUBSYS_PMC, PM_DEV_TCM_1_B,
			(CapAccess | CapContext), XPM_DEF_QOS, 0U,
			XPLMI_CMD_SECURE);
		ErrorCode = (u32)XLOADER_ERR_PM_DEV_TCM_1_B;
	}

END:
	if (Status != XST_SUCCESS) {
		Status = XPlmi_UpdateStatus((XPlmiStatus_t)ErrorCode, 0);
	}
	return Status;
}

/****************************************************************************/
/**
 * @brief	This function is used to check whether cpu has handoff address
 * 			stored in the handoff structure.
 *
 * @param	PdiPtr is pointer to XilPdi instance
 * @param	DstnCpu is the cpu which needs to be checked
 *
 * @return
 * 			- XST_SUCCESS if the DstnCpu is successfully added to Handoff list
 * 			- XST_FAILURE if the DstnCpu is already added to Handoff list
 *
 *****************************************************************************/
static int XLoader_CheckHandoffCpu(const XilPdi* PdiPtr, const u32 DstnCpu)
{
	int Status = XST_FAILURE;
	u32 Index;
	u32 CpuId;

	for (Index = 0U; Index < PdiPtr->NoOfHandoffCpus; Index++) {
		CpuId = PdiPtr->HandoffParam[Index].CpuSettings &
			XIH_PH_ATTRB_DSTN_CPU_MASK;
		if (CpuId == DstnCpu) {
			goto END;
		}
	}
	Status = XST_SUCCESS;

END:
	return Status;
}

/****************************************************************************/
/**
 * @brief	This function is used to update the handoff parameters.
 *
 * @param	PdiPtr is pointer to XilPdi instance
 *
 * @return
 * 			- XST_SUCCESS on success.
 * 			- XLOADER_ERR_NUM_HANDOFF_CPUS when number of CPUs exceed max count.
 *
 *****************************************************************************/
int XLoader_UpdateHandoffParam(XilPdi* PdiPtr)
{
	int Status = XST_FAILURE;
	u32 DstnCpu = XIH_PH_ATTRB_DSTN_CPU_NONE;
	u32 CpuNo = XLOADER_MAX_HANDOFF_CPUS;
	u32 PrtnNum = PdiPtr->PrtnNum;
	/* Assign the partition header to local variable */
	const XilPdi_PrtnHdr * PrtnHdr =
			&(PdiPtr->MetaHdr.PrtnHdr[PrtnNum]);

	/**
	 * - Get the destination CPU from the partition header.
	*/
	DstnCpu = XilPdi_GetDstnCpu(PrtnHdr);

	if ((DstnCpu > XIH_PH_ATTRB_DSTN_CPU_NONE) &&
	    (DstnCpu <= XIH_PH_ATTRB_DSTN_CPU_PSM)) {
		CpuNo = PdiPtr->NoOfHandoffCpus;
		/**
		 * - Validate the destination CPU.
		*/
		if (XLoader_CheckHandoffCpu(PdiPtr, DstnCpu) == XST_SUCCESS) {
			if (CpuNo >= XLOADER_MAX_HANDOFF_CPUS) {
				Status = XPlmi_UpdateStatus(
					XLOADER_ERR_NUM_HANDOFF_CPUS, 0);
				goto END;
			}
			/**
			 * - Update the CPU settings.
			 */
			PdiPtr->HandoffParam[CpuNo].CpuSettings =
				XilPdi_GetDstnCpu(PrtnHdr) |
				XilPdi_GetA72ExecState(PrtnHdr) |
				XilPdi_GetVecLocation(PrtnHdr);
			PdiPtr->HandoffParam[CpuNo].HandoffAddr =
				PrtnHdr->DstnExecutionAddr;
			PdiPtr->NoOfHandoffCpus += 1U;
		}
	}
	Status = XST_SUCCESS;

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function updates the load address based on the
 * 			destination CPU.
 *
 * @param	DstnCpu is destination CPU
 * @param	LoadAddrPtr is the destination load address pointer
 * @param	Len is the length of the partition
 *
 * @return
 * 			- XST_SUCCESS on success.
 * 			- XLOADER_ERR_TCM_ADDR_OUTOF_RANGE if load address is out of range.
 *
 *****************************************************************************/
static int XLoader_GetLoadAddr(u32 DstnCpu, u64 *LoadAddrPtr, u32 Len)
{
	int Status = XST_FAILURE;
	u64 Address = *LoadAddrPtr;
	u32 Offset = 0U;

	/**
	 * - Validate the address is within the range of R5 TCMA or R5 TCMB.
	*/
	if (((Address < (XLOADER_R5_TCMA_LOAD_ADDRESS +
			XLOADER_R5_TCM_BANK_LENGTH)) ||
			((Address >= XLOADER_R5_TCMB_LOAD_ADDRESS) &&
			(Address < (XLOADER_R5_TCMB_LOAD_ADDRESS +
				XLOADER_R5_TCM_BANK_LENGTH))))) {
		if (DstnCpu == XIH_PH_ATTRB_DSTN_CPU_R5_0) {
			Offset = XLOADER_R5_0_TCM_A_BASE_ADDR;
		}
		else if (DstnCpu == XIH_PH_ATTRB_DSTN_CPU_R5_1) {
			Offset = XLOADER_R5_1_TCM_A_BASE_ADDR;
		}
		else {
			/* MISRA-C compliance */
		}

		if ((DstnCpu == XIH_PH_ATTRB_DSTN_CPU_R5_0) ||
			(DstnCpu == XIH_PH_ATTRB_DSTN_CPU_R5_1)) {
			if (((Address % XLOADER_R5_TCM_BANK_LENGTH) + Len) >
				XLOADER_R5_TCM_BANK_LENGTH) {
				Status = XPlmi_UpdateStatus(XLOADER_ERR_TCM_ADDR_OUTOF_RANGE, 0);
				goto END;
			}
		}
	}

	/**
	 * - Otherwise validate the address if the destination CPU is lockstep R5
	 * and is within the range of it TCM.
	*/
	if ((DstnCpu == XIH_PH_ATTRB_DSTN_CPU_R5_L) &&
		(Address < XLOADER_R5_TCM_TOTAL_LENGTH)) {
		if (((Address % XLOADER_R5_TCM_TOTAL_LENGTH) + Len) >
			XLOADER_R5_TCM_TOTAL_LENGTH) {
			Status = XPlmi_UpdateStatus(XLOADER_ERR_TCM_ADDR_OUTOF_RANGE, 0);
			goto END;
		}
		Offset = XLOADER_R5_0_TCM_A_BASE_ADDR;
	}

	/**
	 * - Update the load address.
	 */
	Address += Offset;
	*LoadAddrPtr = Address;
	Status = XST_SUCCESS;

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function prints DDRMC register details.
 *
 * @return
 * 			- XST_SUCCESS on success and error code on failure
 *
 *****************************************************************************/
static int XLoader_DumpDdrmcRegisters(void)
{
	int Status = XST_FAILURE;
	u32 PcsrStatus;
	u32 CalibErr;
	u32 CalibErrNibble1;
	u32 CalibErrNibble2;
	u32 CalibErrNibble3;
	u32 CalibStage;
	u32 PcsrCtrl;
	u32 DevId;
	u8 Ub = 0U;
	u32 BaseAddr;
	XPm_DeviceStatus DevStatus;

	XPlmi_Printf(DEBUG_PRINT_ALWAYS,"====DDRMC Register Dump Start======\n\r");

	Status = XLoader_DdrInit(XLOADER_PDI_SRC_DDR);
	if (XST_SUCCESS != Status) {
		XPlmi_Printf(DEBUG_PRINT_ALWAYS,
				"Error  0x%0x in requesting DDR.\n\r", Status);
		goto END;
	}

	for (DevId = PM_DEV_DDRMC_0; DevId <= PM_DEV_DDRMC_3; DevId++) {
		DevStatus.Status = (u32)XPM_DEVSTATE_UNUSED;
		/** Get DDRMC UB Base address */
		Status = XPm_GetDeviceStatus(PM_SUBSYS_PMC, DevId, &DevStatus);
		if (Status != XST_SUCCESS) {
			goto END;
		}
		if (DevStatus.Status != XPM_DEVSTATE_RUNNING) {
			XPlmi_Printf(DEBUG_GENERAL, "DDRMC_%u is not enabled,"
					" Skipping its dump...\n\r", Ub);
			continue;
		}
		Status = XPm_GetDeviceBaseAddr(DevId, &BaseAddr);
		if (XST_SUCCESS != Status) {
			XPlmi_Printf(DEBUG_PRINT_ALWAYS,
				"Error 0x%0x in getting DDRMC_%u addr\n",
				Status, Ub);
			goto END;
		}

		XPlmi_Printf(DEBUG_PRINT_ALWAYS,
				"DDRMC_%u (UB 0x%08x)\n\r", Ub, BaseAddr);

		/** Read PCSR Control */
		PcsrCtrl = XPlmi_In32(BaseAddr + DDRMC_PCSR_CONTROL_OFFSET);

		/** Skip DDRMC dump if PComplete is zero */
		if (0U == (PcsrCtrl & DDRMC_PCSR_CONTROL_PCOMPLETE_MASK)) {
			XPlmi_Printf(DEBUG_PRINT_ALWAYS, "PComplete not set\n\r");
			++Ub;
			continue;
		}

		/** Read PCSR Status */
		PcsrStatus = XPlmi_In32(BaseAddr + DDRMC_PCSR_STATUS_OFFSET);
		/** Read Calibration Error */
		CalibErr = XPlmi_In32(BaseAddr + DDRMC_OFFSET_CALIB_ERR);
		/** Read Error Nibble 1 */
		CalibErrNibble1 = XPlmi_In32(BaseAddr +
				DDRMC_OFFSET_CALIB_ERR_NIBBLE_1);
		/** Read Error Nibble 2 */
		CalibErrNibble2 = XPlmi_In32(BaseAddr +
				DDRMC_OFFSET_CALIB_ERR_NIBBLE_2);
		/** Read Error Nibble 3 */
		CalibErrNibble3 = XPlmi_In32(BaseAddr +
				DDRMC_OFFSET_CALIB_ERR_NIBBLE_3);
		/** Read calibration stage */
		CalibStage = XPlmi_In32(BaseAddr +
				DDRMC_OFFSET_CALIB_STAGE_PTR);

		XPlmi_Printf(DEBUG_PRINT_ALWAYS,
				"PCSR Control: 0x%0x\n\r", PcsrCtrl);
		XPlmi_Printf(DEBUG_PRINT_ALWAYS,
				"PCSR Status: 0x%0x\n\r", PcsrStatus);
		XPlmi_Printf(DEBUG_PRINT_ALWAYS,
				"Calibration Error: 0x%0x\n\r", CalibErr);
		XPlmi_Printf(DEBUG_PRINT_ALWAYS,
				"Nibble Location 1: 0x%0x\n\r",
				CalibErrNibble1);
		XPlmi_Printf(DEBUG_PRINT_ALWAYS,
				"Nibble Location 2: 0x%0x\n\r",
				CalibErrNibble2);
		XPlmi_Printf(DEBUG_PRINT_ALWAYS,
				"Nibble Location 3: 0x%0x\n\r",
				CalibErrNibble3);
		XPlmi_Printf(DEBUG_PRINT_ALWAYS,
				"Calibration Stage: 0x%0x\n\r", CalibStage);
		++Ub;
	}
	XPlmi_Printf(DEBUG_PRINT_ALWAYS, "PMC Interrupt Status : 0x%0x\n\r",
			XPlmi_In32(PMC_GLOBAL_PMC_ERR1_STATUS));
	XPlmi_Printf(DEBUG_PRINT_ALWAYS, "====DDRMC Register Dump End======\n\r");

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function checks if MJTAG workaround partition needs to be
 *			skipped
 *
 * @param	PdiPtr is pointer to PDI instance

 * @return	TRUE if MTAG workaround partition needs to be skipped, else FALSE
 *
 *****************************************************************************/
u8 XLoader_SkipMJtagWorkAround(XilPdi *PdiPtr)
{
	u32 RstReason;
	u8 Check = (u8)FALSE;

	if (PdiPtr->MetaHdr.ImgHdr[PdiPtr->ImageNum].ImgID ==
			PM_MISC_MJTAG_WA_IMG) {
		RstReason = XPlmi_In32(PMC_GLOBAL_PERS_GEN_STORAGE2);
		/**
		 * Skip MJTAG WA2 partitions if boot mode is JTAG and
		 * Reset Reason is not external POR
		 */
		if ((PdiPtr->PdiSrc == XLOADER_PDI_SRC_JTAG) ||
			(((RstReason & PERS_GEN_STORAGE2_ACC_RR_MASK) >>
					CRP_RESET_REASON_SHIFT) !=
				CRP_RESET_REASON_EXT_POR_MASK)) {
			Check = (u8)TRUE;
		}
	}

	return Check;
}

/*****************************************************************************/
/**
 * @brief	This function checks if MJTAG workaround is required
 *
 * @return
 * 			- XLOADER_ERR_DEFERRED_CDO_PROCESS on error while processing CDO but
 * error is deferred till whole CDO processing is completed.
 *
 *****************************************************************************/
int XLoader_ProcessDeferredError(void)
{
	int Status = XST_FAILURE;

	Status = XLoader_DumpDdrmcRegisters();
	Status = XPlmi_UpdateStatus(
		XLOADER_ERR_DEFERRED_CDO_PROCESS, Status);

	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function check conditions and perform internal POR
 *			for VP1802 and VP1502 device if required.
 *
 * @return
 * 			- None.
 *
 *****************************************************************************/
void XLoader_PerformInternalPOR(void)
{
	u32 IdCode = XPlmi_In32(PMC_TAP_IDCODE) &
			PMC_TAP_IDCODE_SIREV_DVCD_MASK;
	u32 CrpResetReason = XPlmi_In32(CRP_RESET_REASON);
	u8 SlrType = (u8)(XPlmi_In32(PMC_TAP_SLR_TYPE) &
		PMC_TAP_SLR_TYPE_VAL_MASK);
	u32 DnaBit = XPlmi_In32(EFUSE_CACHE_DNA_1) &
			EFUSE_CACHE_DNA_1_BIT25_MASK;
	PdiSrc_t BootMode = XLoader_GetBootMode();

	if ((IdCode != PMC_TAP_IDCODE_ES1_VP1802) &&
		(IdCode != PMC_TAP_IDCODE_ES1_VP1502)) {
		/**
		 * - If the device is not an VP1802 Or VP1502, then return without
		 * performing IPOR.
		 */
		goto END;
	}

	if (SlrType != XLOADER_SSIT_MASTER_SLR) {
		/**
		 * - If the device is not an master SLR, then return without
		 * performing IPOR.
		 */
		goto END;
	}

	if ((BootMode == XLOADER_PDI_SRC_JTAG) ||
		(BootMode == XLOADER_PDI_SRC_SMAP)) {
		/**
		 * - If the bootmode is JTAG or SMAP, then return without
		 * performing IPOR.
		 */
		goto END;
	}

	if (DnaBit == 0x00U) {
		/**
		 * - Efuse DNA_57 bit should be non-zero for IPOR.
		 */
		goto END;
	}

	/**
	 * - Perform IPOR, if all the pre-conditions are met for
	 * VP1502/VP1802 device.
	 */
	if (CrpResetReason == CRP_RESET_REASON_EXT_POR_MASK) {
		usleep(PLM_VP1802_POR_SETTLE_TIME);
		XPlmi_PORHandler();
	}

END:
	return;
}

/*****************************************************************************/
/**
 * @brief	This function will sync the PDI load status with master
 *              if End of PDI SYNC bit is enabled in IHT
 *
 * @param	PdiPtr is pointer to PDI instance

 * @return
 * 		-XST_SUCCESS if no issue in EoPDI sync
 * 		-XPLMI_ERR_SSIT_EOPDI_SYNC if error in EoPDI Sync
 *
 *****************************************************************************/
int Xloader_SsitEoPdiSync(XilPdi *PdiPtr)
{
	int Status = XST_FAILURE;

	XPlmi_Printf(DEBUG_INFO,"%s \n\r",__func__);

	/**
	 * - Validate if End of PDI SYNC enabled in Slave PDI.
	 * Otherwise return XST_SUCCESS.
	 */
	if ((PdiPtr->SlrType != XLOADER_SSIT_MASTER_SLR) &&
		(PdiPtr->SlrType != XLOADER_SSIT_MONOLITIC) &&
		(PdiPtr->SlrType != XLOADER_SSIT_INVALID_SLR)) {
		/**
		 * - Validate if end of PDI Sync bit set in IHT Attribute.
		 * Otherwise return XST_SUCCESS.
		 */
		if ((PdiPtr->MetaHdr.ImgHdrTbl.Attr & XIH_IHT_ATTR_EOPDI_SYNC_MASK)
					== XIH_IHT_ATTR_EOPDI_SYNC_MASK) {
			/**
			 * - Sync with master to update slave status.
			 */
			Status = XPlmi_SsitSyncMaster(NULL);
			if (Status != XST_SUCCESS) {
				Status = XPlmi_UpdateStatus(XPLMI_ERR_SSIT_EOPDI_SYNC, Status);
			}
		}
		else {
			Status = XST_SUCCESS;
		}
	}
	else {
		Status = XST_SUCCESS;
	}
	return Status;
}
