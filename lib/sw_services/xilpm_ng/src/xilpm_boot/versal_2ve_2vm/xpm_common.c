/******************************************************************************
* Copyright (c) 2024 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
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
static u32 XPm_In64(u64 RegAddress)
{
	return lwea(RegAddress);
}
#define IS_IN_RANGE(Address, BaseAddress, Size) \
	(((Address) >= (BaseAddress)) && \
	 ((Address) < ((BaseAddress) + (Size))))

/**
 * @brief Write a protected register and restore its write-protection state.
 *
 * @param RegAddress Register address to write.
 * @param Value Register value to write.
 * @param WprotAddress Write-protection register address.
 */
static void XPm_WriteProtected32(const u32 RegAddress, const u32 Value,
				 const u32 WprotAddress)
{
	/* Save and disable write protection before updating the target register. */
	u32 SavedWprot = Xil_In32(WprotAddress);
	Xil_Out32(WprotAddress, 0x0U);
	Xil_Out32(RegAddress, Value);

	/* Restore the write-protection state after the target write completes. */
	Xil_Out32(WprotAddress, SavedWprot);
}

/**
 * @brief Write a register protected by two write-protection registers.
 *
 * Both protection values are saved before unlocking and restored after
 * the write.
 *
 * @param RegAddress Register address to write.
 * @param Value Register value to write.
 * @param WprotSAddress First write-protection register address.
 * @param WprotPAddress Second write-protection register address.
 */
static void XPm_WriteProtected32Dual(const u32 RegAddress, const u32 Value,
				     const u32 WprotSAddress,
				     const u32 WprotPAddress)
{
	/* Preserve both protection registers before disabling them in order. */
	u32 SavedWprotS = Xil_In32(WprotSAddress);
	u32 SavedWprotP = Xil_In32(WprotPAddress);
	Xil_Out32(WprotSAddress, 0x0U);
	Xil_Out32(WprotPAddress, 0x0U);

	/* Perform the target write, then restore both registers in order. */
	Xil_Out32(RegAddress, Value);
	Xil_Out32(WprotSAddress, SavedWprotS);
	Xil_Out32(WprotPAddress, SavedWprotP);
}

void XPm_Out32(u32 RegAddress, u32 l_Val)
{
	const XPm_Pmc *Pmc = (XPm_Pmc *)XPmDevice_GetById(PM_DEV_PMC_PROC);
	const XPm_PsLpDomain *PsLpd = (XPm_PsLpDomain *)XPmPower_GetById(PM_POWER_LPD);
	const XPm_PsFpDomain *PsFpd = (XPm_PsFpDomain *)XPmPower_GetById(PM_POWER_FPD);
	u32 Handled = 0U;

	/* Test protected ranges in priority order and stop after the first match. */
	if ((NULL != Pmc) && (IS_IN_RANGE(RegAddress, Pmc->PmcIouSlcrBaseAddr, PMC_IOU_SLCR_SIZE))) {
		XPm_WriteProtected32(RegAddress, l_Val,
			Pmc->PmcIouSlcrBaseAddr + PMC_IOU_SLCR_WPROT0_OFFSET);
		Handled = 1U;
	}
	if ((0U == Handled) && (NULL != PsLpd) &&
	    (IS_IN_RANGE(RegAddress, PsLpd->LpdIouSlcrBaseAddr, LPD_IOU_SLCR_SIZE))) {
		XPm_WriteProtected32(RegAddress, l_Val,
			PsLpd->LpdIouSlcrBaseAddr + LPD_IOU_SLCR_WPROT0_OFFSET);
		Handled = 1U;
	}
	if ((0U == Handled) && (NULL != PsLpd) &&
	    (IS_IN_RANGE(RegAddress, PsLpd->LpdSlcrBaseAddr, PSXC_LPX_SLCR_SIZE))) {
		XPm_WriteProtected32(RegAddress, l_Val,
			PsLpd->LpdSlcrBaseAddr + LPD_SLCR_WPROT0_OFFSET);
		Handled = 1U;
	}
	if ((0U == Handled) && (NULL != PsFpd) &&
	    (IS_IN_RANGE(RegAddress, PsFpd->FpdSlcrBaseAddr, FPD_SLCR_SIZE))) {
		XPm_WriteProtected32(RegAddress, l_Val,
			PsFpd->FpdSlcrBaseAddr + FPD_SLCR_WPROT0_OFFSET);
		Handled = 1U;
	}
	if ((0U == Handled) && IS_IN_RANGE(RegAddress, PSX_CRL_BASEADDR, PSX_CRL_SIZE)) {
		XPm_WriteProtected32(RegAddress, l_Val,
			PSX_CRL_BASEADDR + PSX_CRL_WPROT_OFFSET);
		Handled = 1U;
	}
	if ((0U == Handled) && IS_IN_RANGE(RegAddress, PSXC_CRF_BASEADDR, (u32)PSXC_CRF_SIZE)) {
		XPm_WriteProtected32(RegAddress, l_Val,
			PSXC_CRF_BASEADDR + PSXC_CRF_WPROT_OFFSET);
		Handled = 1U;
	}
	if ((0U == Handled) && IS_IN_RANGE(RegAddress, CRP_BASEADDR, CRP_SIZE)) {
		XPm_WriteProtected32(RegAddress, l_Val,
			CRP_BASEADDR + CRP_WPROT_OFFSET);
		Handled = 1U;
	}
	if ((0U == Handled) && IS_IN_RANGE(RegAddress, MMI_SLCR_BASEADDR, MMI_SLCR_SIZE)) {
		XPm_WriteProtected32Dual(RegAddress, l_Val,
			MMI_SLCR_BASEADDR + MMI_SLCR_WPROTS_OFFSET,
			MMI_SLCR_BASEADDR + MMI_SLCR_WPROTP_OFFSET);
		Handled = 1U;
	}
	if ((0U == Handled) && IS_IN_RANGE(RegAddress, MMI_UDH_PLL_BASEADDR, MMI_UDH_PLL_SIZE)) {
		XPm_WriteProtected32Dual(RegAddress, l_Val,
			MMI_UDH_PLL_BASEADDR + MMI_UDH_PLL_WPROTS_OFFSET,
			MMI_UDH_PLL_BASEADDR + MMI_UDH_PLL_WPROTP_OFFSET);
		Handled = 1U;
	}
	if ((0U == Handled) && IS_IN_RANGE(RegAddress, MMI_CRX_BASEADDR, MMI_CRX_SIZE)) {
		XPm_WriteProtected32Dual(RegAddress, l_Val,
			MMI_CRX_BASEADDR + MMI_CRX_WPROTS_OFFSET,
			MMI_CRX_BASEADDR + MMI_CRX_WPROTP_OFFSET);
		Handled = 1U;
	}
	if ((0U == Handled) && IS_IN_RANGE(RegAddress, MMI_DC_BASEADDR, MMI_DC_SIZE)) {
		XPm_WriteProtected32(RegAddress, l_Val,
			MMI_DC_BASEADDR + MMI_DC_WPROTS_OFFSET);
		Handled = 1U;
	}
	if ((0U == Handled) && IS_IN_RANGE(RegAddress, MMI_UDH_SLCR_BASEADDR, MMI_UDH_SLCR_SIZE)) {
		XPm_WriteProtected32Dual(RegAddress, l_Val,
			MMI_UDH_SLCR_BASEADDR + MMI_UDH_SLCR_WPROTS_OFFSET,
			MMI_UDH_SLCR_BASEADDR + MMI_UDH_SLCR_WPROTP_OFFSET);
		Handled = 1U;
	}
	if ((0U == Handled) && IS_IN_RANGE(RegAddress, EIO_CRS_BASEADDR, EIO_CRS_SIZE)) {
		XPm_WriteProtected32(RegAddress, l_Val,
			EIO_CRS_BASEADDR + EIO_CRS_WPROT_OFFSET);
		Handled = 1U;
	}

	/* Unprotected addresses are written directly after all ranges are rejected. */
	if (0U == Handled) {
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
static void XPm_Out64(u64 RegAddress, u32 Value)
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
 * 	   64 bit address
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
maybe_unused static void XPm_RMW64(u64 RegAddress, u32 Mask, u32 Value)
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

/****************************************************************************/
/**
 * @brief  This function computes the even parity of a 32 bit value.
 *
 * @param  CalParity	32 bit value whose parity is to be computed
 *
 * @return The parity bit (0 or 1) of the input value
 *
 * @note   None
 *
 ****************************************************************************/
maybe_unused static u32 XPm_ComputeParity(u32 CalParity)
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

/****************************************************************************/
/**
 * @brief  This function returns the platform version of the device.
 *
 * @return The platform version read from the hardware
 *
 * @note   The value is read once and cached for subsequent calls.
 *
 ****************************************************************************/
maybe_unused static u32 XPm_GetPlatformVersion(void)
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

/****************************************************************************/
/**
 * @brief  This function returns the SLR type of the device.
 *
 * @return The SLR type read from the hardware
 *
 * @note   The value is read once and cached for subsequent calls.
 *
 ****************************************************************************/
maybe_unused static u32 XPm_GetSlrType(void)
{
        static u32 DevSlrType = NOT_INITIALIZED;

        if (NOT_INITIALIZED == DevSlrType) {
                DevSlrType = PMC_TAP_SLR_TYPE_MASK &
                          XPm_In32(PMC_TAP_SLR_TYPE_OFFSET +
                                   PMC_TAP_BASEADDR);
        }

        return DevSlrType;
}

/****************************************************************************/
/**
 * @brief  This function returns the IDCODE of the device.
 *
 * @return The IDCODE read from the hardware
 *
 * @note   The value is read once and cached for subsequent calls.
 *
 ****************************************************************************/
maybe_unused static u32 XPm_GetIdCode(void)
{
        static u32 DevIdCode = NOT_INITIALIZED;

        if (NOT_INITIALIZED == DevIdCode) {
                DevIdCode = XPm_In32(PMC_TAP_IDCODE);
        }

        return DevIdCode;
}

/****************************************************************************/
/**
 * @brief  This function returns the PMC version of the device.
 *
 * @return The PMC version read from the hardware
 *
 * @note   The value is read once and cached for subsequent calls.
 *
 ****************************************************************************/
maybe_unused static u32 XPm_GetPmcVersion(void)
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
	static const char* const PrefixStr[] = {"ERR", "WARN", "INFO", "DBG"};
	u32 Idx = DBG_STR_IDX(DebugType);

	va_start(Args, Ctrl1);
	if ((((DebugType) & (DebugLog->LogLevel)) != 0U) &&
		(Idx < (u32)ARRAY_SIZE(PrefixStr))) {
		if ((DebugType & XPLMI_DEBUG_PRINT_TIMESTAMP_MASK) != 0U) {
			XPlmi_PrintPlmTimeStamp();
		}
		xil_printf("%s %s: ", PrefixStr[Idx], Fnstr);
		xil_vprintf(Ctrl1, Args);
	}
	va_end(Args);
}

/*****************************************************************************/
/**
 * @brief This function unlocks the NPI PCSR registers.
 *
 * @param BaseAddr              Base address of the device
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
 * @param BaseAddr      Base address of the device
 *
 * @return XST_SUCCESS if the lock value was written and confirmed via
 *         REG_PCSR_STATUS.PCSRLOCK; XPM_REG_WRITE_FAILED otherwise.
 *
 *****************************************************************************/
inline XStatus XPm_LockPcsr(u32 BaseAddr)
{
	volatile XStatus Status = XST_FAILURE;

	/*
	 * Any value that is not the unlock value will lock the PCSR. For
	 * consistency across all blocks, PCSR_LOCK_VAL is 0. REG_PCSR_LOCK
	 * does not store the written value, so confirm the lock took effect
	 * by reading the read-only PCSRLOCK bit in REG_PCSR_STATUS.
	 */
	XSECURE_REDUNDANT_IMPL(XPm_Out32, BaseAddr + NPI_PCSR_LOCK_OFFSET, PCSR_LOCK_VAL);
	PmChkRegMask32(BaseAddr + NPI_PCSR_STATUS_OFFSET,
		       NPI_PCSR_STATUS_PCSRLOCK_MASK,
		       NPI_PCSR_STATUS_PCSRLOCK_MASK, Status);
	if (XPM_REG_WRITE_FAILED == Status) {
		goto done;
	}

	Status = XST_SUCCESS;

done:
	return Status;
}


/**
 * XPm_PcsrWrite - write a pair control and mask register of a pcsr register.
 *
 * @param BaseAddress: The base address of the register.
 * @param Mask: The mask to apply to the register.
 * @param Value: The value to write to the register.
 *
 * @return XStatus: Returns the status of the operation.
 *
 * This function writes a value to a PCSR register at the given base address,
 * applying the provided mask to ensure only the desired bits are modified.
 */
XStatus XPm_PcsrWrite(u32 BaseAddress, u32 Mask, u32 Value)
{
	XStatus Status = XST_FAILURE;
	u16 DbgErr = XPM_INT_ERR_UNDEFINED;

	XPm_Out32((BaseAddress + NPI_PCSR_MASK_OFFSET), Mask);
	/* Blind write check */
	PmChkRegOut32((BaseAddress + NPI_PCSR_MASK_OFFSET), Mask, Status);
	if (XPM_REG_WRITE_FAILED == Status) {
		DbgErr = XPM_INT_ERR_REG_WRT_NPI_PCSR_MASK;
		goto done;
	}

	XPm_Out32((BaseAddress + NPI_PCSR_CONTROL_OFFSET), Value);
	/* Blind write check */
	PmChkRegMask32((BaseAddress + NPI_PCSR_CONTROL_OFFSET), Mask, Value, Status);
	if (XPM_REG_WRITE_FAILED == Status) {
		DbgErr = XPM_INT_ERR_REG_WRT_NPI_PCSR_CONTROL;
		goto done;
	}

	Status = XST_SUCCESS;

done:
	XPm_PrintDbgErr(Status, DbgErr);
	return Status;
}
