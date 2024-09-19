/******************************************************************************
* Copyright (c) 2018 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2024 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


#include "xil_io.h"

#include "xpm_common.h"
#include "xpm_regs.h"
#include "xpm_pmc.h"
#include "xpm_psfpdomain.h"
#include "xpm_pslpdomain.h"
#include "xpm_debug.h"
#include <stdarg.h>

#define NOT_INITIALIZED 0xFFFFFFFFU
#define DBG_STR_IDX(DebugType) ((((DebugType) & XPM_DEBUG_MASK) >> \
					XPM_DEBUG_SHIFT) - 1U)

u32 XPm_In32(u32 RegAddress)
{
	return Xil_In32(RegAddress);
}

/****************************************************************************/
/**
 * @brief  This function reads one word (32 bit) from 64 bit address
 *
 * @param  RegAddress	64 bit address
 *
 * @return 32 bit value at that address
 *
 * @note   None
 *
 ****************************************************************************/
u32 XPm_In64(u64 RegAddress)
{
	return lwea(RegAddress);
}

void XPm_Out32(u32 RegAddress, u32 l_Val)
{
	const XPm_Pmc *Pmc = (XPm_Pmc *)XPmDevice_GetById(PM_DEV_PMC_PROC);
	const XPm_PsLpDomain *PsLpd = (XPm_PsLpDomain *)XPmPower_GetById(PM_POWER_LPD);
	const XPm_PsFpDomain *PsFpd = (XPm_PsFpDomain *)XPmPower_GetById(PM_POWER_FPD);
	u32 SavedWPROT = 0;

	if ((NULL != Pmc) && ((RegAddress & 0xFFFF0000U) == Pmc->PmcIouSlcrBaseAddr)) {
		SavedWPROT = Xil_In32(Pmc->PmcIouSlcrBaseAddr + PMC_IOU_SLCR_WPROT0_OFFSET);
		Xil_Out32(Pmc->PmcIouSlcrBaseAddr + PMC_IOU_SLCR_WPROT0_OFFSET, 0x0U);
		Xil_Out32(RegAddress, l_Val);
		Xil_Out32(Pmc->PmcIouSlcrBaseAddr + PMC_IOU_SLCR_WPROT0_OFFSET, SavedWPROT);
	} else if ((NULL != PsLpd) && ((RegAddress & 0xFFFF0000U) == PsLpd->LpdIouSlcrBaseAddr)) {
		SavedWPROT = Xil_In32(PsLpd->LpdIouSlcrBaseAddr + LPD_IOU_SLCR_WPROT0_OFFSET);
		Xil_Out32(PsLpd->LpdIouSlcrBaseAddr + LPD_IOU_SLCR_WPROT0_OFFSET, 0x0U);
		Xil_Out32(RegAddress, l_Val);
		Xil_Out32(PsLpd->LpdIouSlcrBaseAddr + LPD_IOU_SLCR_WPROT0_OFFSET, SavedWPROT);
	} else if ((NULL != PsLpd) && ((RegAddress & 0xFFFF0000U) == PsLpd->LpdSlcrBaseAddr)) {
		SavedWPROT = Xil_In32(PsLpd->LpdSlcrBaseAddr + LPD_SLCR_WPROT0_OFFSET);
		Xil_Out32(PsLpd->LpdSlcrBaseAddr + LPD_SLCR_WPROT0_OFFSET, 0x0U);
		Xil_Out32(RegAddress, l_Val);
		Xil_Out32(PsLpd->LpdSlcrBaseAddr + LPD_SLCR_WPROT0_OFFSET, SavedWPROT);
	 } else if ((NULL != PsFpd) && ((RegAddress & 0xFFFF0000U) == PsFpd->FpdSlcrBaseAddr)) {
		SavedWPROT = Xil_In32(PsFpd->FpdSlcrBaseAddr + FPD_SLCR_WPROT0_OFFSET);
		Xil_Out32(PsFpd->FpdSlcrBaseAddr + FPD_SLCR_WPROT0_OFFSET, 0x0U);
		Xil_Out32(RegAddress, l_Val);
		Xil_Out32(PsFpd->FpdSlcrBaseAddr + FPD_SLCR_WPROT0_OFFSET, SavedWPROT);
	} else {
		Xil_Out32(RegAddress, l_Val);
	}
}

/****************************************************************************/
/**
 * @brief  This function writes one word (32 bit) to 64 bit address
 *
 * @param  RegAddress	64 bit address
 * @param  Value	32 bit value
 *
 * @return None
 *
 * @note   None
 *
 ****************************************************************************/
void XPm_Out64(u64 RegAddress, u32 Value)
{
	swea(RegAddress, Value);
}

void XPm_RMW32(u32 RegAddress, u32 Mask, u32 Value)
{
	u32 l_Val;

	l_Val = XPm_In32(RegAddress);
	l_Val = (l_Val & (~Mask)) | (Mask & Value);

	XPm_Out32(RegAddress, l_Val);
}

/****************************************************************************/
/**
 * @brief  This function reads, modifies and writes one word (32 bit) to
 *	   64 bit address
 *
 * @param  RegAddress	64 bit address
 * @param  Value	32 bit mask
 * @param  Value	32 bit value
 *
 * @return None
 *
 * @note   None
 *
 ****************************************************************************/
void XPm_RMW64(u64 RegAddress, u32 Mask, u32 Value)
{
	u32 l_Val;

	l_Val = XPm_In64(RegAddress);
	l_Val = (l_Val & (~Mask)) | (Mask & Value);

	XPm_Out64(RegAddress, l_Val);
}

void XPm_Wait(u32 TimeOutCount)
{
	u32 TimeOut = TimeOutCount;
	while (TimeOut > 0U) {
		TimeOut--;
	}
}

u32 XPm_ComputeParity(u32 CalParity)
{
	u32 Value = CalParity;

	Value ^= (Value >> 16U);
	Value ^= (Value >> 8U);
	Value ^= (Value >> 4U);
	Value ^= (Value >> 2U);
	Value ^= (Value >> 1U);

	return (Value & 1U);
}

u32 XPm_GetPlatform(void)
{
	static u32 DevPlatform = NOT_INITIALIZED;

	if (NOT_INITIALIZED == DevPlatform) {
		DevPlatform = (XPm_In32(PMC_TAP_VERSION) &
			    PMC_TAP_VERSION_PLATFORM_MASK) >>
			    PMC_TAP_VERSION_PLATFORM_SHIFT;
	}

	return DevPlatform;
}

u32 XPm_GetPlatformVersion(void)
{
	static u32 DevPlatformVersion = NOT_INITIALIZED;

	if (NOT_INITIALIZED == DevPlatformVersion) {
		if (PLATFORM_VERSION_SILICON == XPm_GetPlatform()) {
			DevPlatformVersion = (XPm_In32(PMC_TAP_IDCODE) &
				PMC_TAP_IDCODE_SI_REV_MASK) >>
				PMC_TAP_IDCODE_SI_REV_SHIFT;
		} else {
			DevPlatformVersion = (XPm_In32(PMC_TAP_VERSION) &
				PMC_TAP_VERSION_PLATFORM_VERSION_MASK) >>
				PMC_TAP_VERSION_PLATFORM_VERSION_SHIFT;
		}
	}

	return DevPlatformVersion;
}

u32 XPm_GetSlrType(void)
{
	static u32 DevSlrType = NOT_INITIALIZED;

	if (NOT_INITIALIZED == DevSlrType) {
		DevSlrType = PMC_TAP_SLR_TYPE_MASK &
			  XPm_In32(PMC_TAP_SLR_TYPE_OFFSET +
				   PMC_TAP_BASEADDR);
	}

	return DevSlrType;
}

u32 XPm_GetIdCode(void)
{
	static u32 DevIdCode = NOT_INITIALIZED;

	if (NOT_INITIALIZED == DevIdCode) {
		DevIdCode = XPm_In32(PMC_TAP_IDCODE);
	}

	return DevIdCode;
}

u32 XPm_GetPmcVersion(void)
{
	static u32 DevPmcVersion = NOT_INITIALIZED;

	if (NOT_INITIALIZED == DevPmcVersion) {
		DevPmcVersion = (XPm_In32(PMC_TAP_VERSION) &
			    PMC_TAP_VERSION_PMC_VERSION_MASK) >>
			    PMC_TAP_VERSION_PMC_VERSION_SHIFT;
	}

	return DevPmcVersion;
}

void XPm_Printf(u32 DebugType, const char *Fnstr, const char8 *Ctrl1, ...)
{
	va_list Args;
	static const char* const PrefixStr[] = {"ALERT", "ERR", "WARN", "INFO", "DBG"};
	u32 Idx = DBG_STR_IDX(DebugType);

	va_start(Args, Ctrl1);
	if ((((DebugType) & (DebugLog->LogLevel)) != (u8)FALSE) &&
		(Idx < (u32)ARRAY_SIZE(PrefixStr))) {
		xil_printf("%s %s: ", PrefixStr[Idx], Fnstr);
		xil_vprintf(Ctrl1, Args);
	}
	va_end(Args);
}

/*****************************************************************************/
/**
 * @brief This function unlocks the NPI PCSR registers.
 *
 * @param BaseAddr		Base address of the device
 *
 *****************************************************************************/
inline void XPm_UnlockPcsr(u32 BaseAddr)
{
	XPm_Out32(BaseAddr + NPI_PCSR_LOCK_OFFSET, PCSR_UNLOCK_VAL);
}

/*****************************************************************************/
/**
 * @brief This function locks the NPI PCSR registers.
 *
 * @param BaseAddr	Base address of the device
 *
 *****************************************************************************/
inline void XPm_LockPcsr(u32 BaseAddr)
{
	/*
	 * Any value that is not the unlock value will lock the PCSR. For
	 * consistency across all blocks, PCSR_LOCK_VAL is 0.
	 */
	XPm_Out32(BaseAddr + NPI_PCSR_LOCK_OFFSET, PCSR_LOCK_VAL);
}
