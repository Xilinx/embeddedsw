/***************************************************************************************************
* Copyright (c) 2025 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
***************************************************************************************************/


/**************************************************************************************************/
/**
*
* @file xplmi_plat.c
*
* This file contains the PLMI versal_2vp platform specific code.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- ----------------------------------------------------------------------------
* 2.3    sd  10/13/25 Initial release
*        sd  03/03/26 Replaced XPlmi_GetRawVoltage inline function with macro
* 2.4    tvp 03/06/26 Add SSS config masks for SHA2 engine
* 2.5    sd  03/13/26 Added changes to enable hardware interrupts
*        sd  08/03/26 Clear SSS Cfg Error set during ROM PCR Extension
*        sd  08/07/26 Removed unused XPlmi_VerifyAddrRange function
*        sri 08/24/26 Add XPlmi_RomISR for OCP DME ROM service
*        pre  09/19/26 Fixed IPI serving logic
*        rama 09/22/26 Add no-op implementations to fix build errors
*                      for libxilsem.a
*        ng   09/25/26 Added XPlmi_VerifyAddrRangeRedundant
*        abh  09/22/26 Fixed Doxygen Warnings
*        sd   10/27/26 Added XPlmi_RunProc command handler
*
* </pre>
*
***************************************************************************************************/

/**
 * @addtogroup xilplmi_server_apis XilPlmi server APIs
 * @{
 */

/*************************************** Include Files ********************************************/
#include "xplmi_dma.h"
#include "xplmi_hw.h"
#include "xplmi.h"
#include "xplmi_event_logging.h"
#include "xcfupmc.h"
#include "xplmi_gic_interrupts.h"
#include "xplmi_plat.h"
#ifndef SDT
#include "microblaze_sleep.h"
#else
#include "sleep.h"
#endif
#include "xplmi_err_common.h"
#include "xplmi_generic.h"
#include "xplmi_ssit.h"
#include "xplmi_tamper.h"
#include "xplmi_modules.h"
#ifdef PLM_ENABLE_ADDR_RANGE_VALIDATION
#include "xil_sutil.h"	/* for XSECURE_REDUNDANT_CALL */
#endif

/************************************ Constant Definitions ****************************************/
#define XPLMI_DIGEST_PMC_1_0_ROM_1_0	(0x2B004AC7U) /**< PMC1 ROM version 1 digest */
#define XPLMI_DIGEST_PMC_2_0_ROM_2_0	(0xB576B550U) /**< PMC2 ROM version 2 digest */

#define XPLMI_ROM_VERSION_1_0		(0x10U) /**< ROM version 1 */
#define XPLMI_ROM_VERSION_2_0		(0x20U) /**< ROM version 2 */
#define XPLMI_INVALID_ROM_VERSION	(0x0U) /**< Invalid ROM version */

/* SSIT SLR related macros */
#define XPLMI_CFU_STREAM_2_SLR_OFFSET	(CFU_STREAM_2_ADDR - XPLMI_PMC_LOCAL_BASEADDR)
						/**< SLR offset for CFU stream 2 register */
#define XPLMI_CFU_FDRO_2_SLR_OFFSET	(CFU_FDRO_2_ADDR - XPLMI_PMC_LOCAL_BASEADDR)
						/**< SLR offset for CFU FDRO 2 register */

#define XPLMI_SLR1_CFU_FDRO_2_ADDR	(XPLMI_PMC_ALIAS1_BASEADDR + XPLMI_CFU_FDRO_2_SLR_OFFSET)
						/**< SLR1 CFU FDRO 2 address */
#define XPLMI_SLR2_CFU_FDRO_2_ADDR	(XPLMI_PMC_ALIAS2_BASEADDR + XPLMI_CFU_FDRO_2_SLR_OFFSET)
						/**< SLR2 CFU FDRO 2 address */
#define XPLMI_SLR3_CFU_FDRO_2_ADDR	(XPLMI_PMC_ALIAS3_BASEADDR + XPLMI_CFU_FDRO_2_SLR_OFFSET)
						/**< SLR3 CFU FDRO 2 address */

#define XPLMI_SLR1_CFU_STREAM_2_ADDR	(XPLMI_PMC_ALIAS1_BASEADDR + XPLMI_CFU_STREAM_2_SLR_OFFSET)
						/**< SLR1 CFU Stream 2 address */
#define XPLMI_SLR2_CFU_STREAM_2_ADDR	(XPLMI_PMC_ALIAS2_BASEADDR + XPLMI_CFU_STREAM_2_SLR_OFFSET)
						/**< SLR2 CFU Stream 2 address */
#define XPLMI_SLR3_CFU_STREAM_2_ADDR	(XPLMI_PMC_ALIAS3_BASEADDR + XPLMI_CFU_STREAM_2_SLR_OFFSET)
						/**< SLR3 CFU Stream 2 address */

#define XPLMI_PMC_VOLTAGE_MULTIPLIER	(32768.0f) /**< PMC voltage multiplier */
#define XPLMI_PMC_VERSION_1_0		(0x10U) /**< PMC version 1.0 */

/************************************** Type Definitions ******************************************/

/************************************** Macros (Inline Functions) Definitions *********************/

/**************************************************************************************************/
/**
 * This macro converts voltage to raw voltage value.
 *
 * @param	Voltage is the voltage value in volts.
 *
 * @return
 * 		- 32-bit voltage value.
 *
 **************************************************************************************************/
#define XPlmi_GetRawVoltage(Voltage)	((u32)((Voltage) * XPLMI_PMC_VOLTAGE_MULTIPLIER))

/************************************ Function Prototypes *****************************************/
static int XPlmi_UpdateResetReason(void);
static void XPlmi_HwIntrHandler(void *CallbackRef);
static int XPlmi_SsitWaitForDmaDone(XPmcDma *DmaPtr, XPmcDma_Channel Channel);
static void XPlmi_DisableIOmodule(void);
static u32 XPlmi_GetIoIntrMask(void);
static void XPlmi_SetIoIntrMask(u32 Value);

/************************************ Variable Definitions ****************************************/
/* Structure for Top level interrupt table */
static XInterruptHandler g_TopLevelInterruptTable[] = {
	XPlmi_HwIntrHandler,
	XPlmi_IntrHandler,
	XPlmi_ErrIntrHandler,
};
static u8 XRamAvailable = (u8)FALSE; /** Flag to indicate XRAM is available */

/**************************************************************************************************/
/**
 * @brief	This function sets XRAM Available Flag.
 *
 **************************************************************************************************/
void XPlmi_SetXRamAvailable(void)
{
	XRamAvailable = (u8)TRUE;
}

/**************************************************************************************************/
/**
 * @brief	This function provides pointer to g_TopLevelInterruptTable.
 *
 * @return
 * 		- Pointer to g_TopLevelInterruptTable structure.
 *
 **************************************************************************************************/
XInterruptHandler *XPlmi_GetTopLevelIntrTbl(void)
{
	return g_TopLevelInterruptTable;
}

/**************************************************************************************************/
/**
 * @brief	This function provides size of g_TopLevelInterruptTable.
 *
 * @return
 * 		- Size g_TopLevelInterruptTable structure.
 *
 **************************************************************************************************/
u8 XPlmi_GetTopLevelIntrTblSize(void)
{
	return (u8)XPLMI_ARRAY_SIZE(g_TopLevelInterruptTable);
}

/**************************************************************************************************/
/**
 * @brief	This function provides pointer to BoardParams.
 *
 * @return
 * 		- Pointer to BoardParams.
 *
 **************************************************************************************************/
XPlmi_BoardParams *XPlmi_GetBoardParams(void)
{
	static XPlmi_BoardParams BoardParams __attribute__ ((aligned(4U))) = {
		.Name = {0U,},
		.Len = 0U,
	};

	return &BoardParams;
}

/**************************************************************************************************/
/**
 * @brief	This function provides pointer to IOModule structure.
 *
 * @return
 * 		- Pointer to XIOModule.
 *
 **************************************************************************************************/
XIOModule *XPlmi_GetIOModuleInst(void)
{
	static XIOModule IOModule __attribute__ ((aligned(4U))) = { 0U };

	return &IOModule;
}

/**************************************************************************************************/
/**
 * @brief	This function provides LpdInitialized variable pointer.
 *
 * @return
 * 		- Pointer to LpdInitialized variable.
 *
 **************************************************************************************************/
u32 *XPlmi_GetUartBaseAddr(void)
{
	static u32 UartBaseAddr = XPLMI_INVALID_UART_BASE_ADDR; /**< Base address of Uart */

	return &UartBaseAddr;
}

/**************************************************************************************************/
/**
 * @brief	This function provides LpdInitialized variable pointer.
 *
 * @return
 * 		- Pointer to LpdInitialized variable.
 *
 **************************************************************************************************/
u32 *XPlmi_GetLpdInitialized(void)
{
	static u32 LpdInitialized = 0U;

	return &LpdInitialized;
}

/**************************************************************************************************/
/**
 * @brief	This function performs initialization of platform specific RCTA registers.
 *
 **************************************************************************************************/
void XPlmi_RtcaPlatInit(void)
{
	/* Versal Net specific RTCA registers Init */
	/* MIO flush RTCFG init */
	XPlmi_Out32(XPLMI_RTCFG_MIO_WA_BANK_500_ADDR, XPLMI_MIO_FLUSH_ALL_PINS);
	XPlmi_Out32(XPLMI_RTCFG_MIO_WA_BANK_501_ADDR, XPLMI_MIO_FLUSH_ALL_PINS);
	XPlmi_Out32(XPLMI_RTCFG_MIO_WA_BANK_502_ADDR, XPLMI_MIO_FLUSH_ALL_PINS);
	XPlmi_Out32(XPLMI_RTCFG_RST_PL_POR_WA, 0U);
}

/**************************************************************************************************/
/**
 * @brief	This function prints ROM version using ROM digest value.
 *
 **************************************************************************************************/
void XPlmi_PrintRomVersion(void)
{
	u32 RomDigest;
	u8 RomVersion;

	RomDigest = XPlmi_In32(PMC_GLOBAL_ROM_VALIDATION_DIGEST_0);
	switch (RomDigest) {
		case XPLMI_DIGEST_PMC_1_0_ROM_1_0:
			RomVersion = XPLMI_ROM_VERSION_1_0;
			break;
		case XPLMI_DIGEST_PMC_2_0_ROM_2_0:
			RomVersion = XPLMI_ROM_VERSION_2_0;
			break;
		default:
			RomVersion = XPLMI_INVALID_ROM_VERSION;
			break;
	}

	if (RomVersion != XPLMI_INVALID_ROM_VERSION) {
		XPlmi_Printf(DEBUG_INFO, "ROM Version: v%u.%u\n\r", (RomVersion >> 4U),
			     (RomVersion & 15U));
	}
}

/**************************************************************************************************/
/**
 * @brief	This function performs plmi pre-initializaton.
 *
 * @return
 * 		- Reset reason status.
 *
 **************************************************************************************************/
int XPlmi_PreInit(void)
{
	return XPlmi_UpdateResetReason();
}

/**************************************************************************************************/
/**
 * @brief	This function masks the secure stream switch value.
 *
 * @param	DmaSrc DMA0 or DMA1.
 *
 **************************************************************************************************/
void XPlmi_SssMask(u32 DmaSrc)
{
	u32 Mask = 0U;
	u32 RegVal = XPlmi_In32(PMC_GLOBAL_PMC_SSS_CFG);

	if (DmaSrc == XPLMI_PMCDMA_0) {
		if ((RegVal & XPLMI_SSSCFG_SBI_MASK) == XPLMI_SSS_SBI_DMA0) {
			Mask |= XPLMI_SSSCFG_SBI_MASK;
		}
		if ((RegVal & XPLMI_SSSCFG_SHA0_MASK) == XPLMI_SSS_SHA0_DMA0) {
			Mask |= XPLMI_SSSCFG_SHA0_MASK;
		}
		if ((RegVal & XPLMI_SSSCFG_SHA1_MASK) == XPLMI_SSS_SHA1_DMA0) {
			Mask |= XPLMI_SSSCFG_SHA1_MASK;
		}
		if ((RegVal & XPLMI_SSSCFG_AES_MASK) == XPLMI_SSS_AES_DMA0) {
			Mask |= XPLMI_SSSCFG_AES_MASK;
		}
		if ((RegVal & XPLMI_SSSCFG_DMA0_MASK) != 0U) {
			Mask |= XPLMI_SSSCFG_DMA0_MASK;
		}
	}

	if (DmaSrc == XPLMI_PMCDMA_1) {
		if ((RegVal & XPLMI_SSSCFG_SBI_MASK) == XPLMI_SSS_SBI_DMA1) {
			Mask |= XPLMI_SSSCFG_SBI_MASK;
		}
		if ((RegVal & XPLMI_SSSCFG_SHA0_MASK) == XPLMI_SSS_SHA0_DMA1) {
			Mask |= XPLMI_SSSCFG_SHA0_MASK;
		}
		if ((RegVal & XPLMI_SSSCFG_SHA1_MASK) == XPLMI_SSS_SHA1_DMA1) {
			Mask |= XPLMI_SSSCFG_SHA1_MASK;
		}
		if ((RegVal & XPLMI_SSSCFG_AES_MASK) == XPLMI_SSS_AES_DMA1) {
			Mask |= XPLMI_SSSCFG_AES_MASK;
		}
		if ((RegVal & XPLMI_SSSCFG_DMA1_MASK) != 0U) {
			Mask |= XPLMI_SSSCFG_DMA1_MASK;
		}
	}

	RegVal &= ~Mask;
	XSECURE_REDUNDANT_IMPL(XPlmi_Out32, PMC_GLOBAL_PMC_SSS_CFG, RegVal);
}

/**************************************************************************************************/
/**
 * @brief	This function updates reset reason.
 *
 * @return
 * 		- XST_SUCCESS on success.
 * 		- Error code on failure.
 *
 **************************************************************************************************/
static int XPlmi_UpdateResetReason(void)
{
	int Status = XST_FAILURE;
	u32 AccResetReason = XPlmi_In32(PMC_GLOBAL_PERS_GEN_STORAGE2) &
			     PERS_GEN_STORAGE2_ACC_RR_MASK;
	u32 ResetReason = XPlmi_In32(CRP_RESET_REASON) & CRP_RESET_REASON_MASK;

	/* Accumulate previous reset reasons and add last reset reason */
	AccResetReason |= (ResetReason << CRP_RESET_REASON_SHIFT) | ResetReason;

	/* Store Reset Reason to Persistent2 address */
	Status = Xil_SecureOut32(PMC_GLOBAL_PERS_GEN_STORAGE2, AccResetReason);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	/* Clear Reset Reason register, by writing the same value */
	XPlmi_Out32(CRP_RESET_REASON, ResetReason);

END:
	return Status;
}

/**************************************************************************************************/
/**
 * @brief	This function is used to check and wait for DMA done when sending data to SSIT Slave
 * 		SLRs.
 *
 * @param	DmaPtr is pointer to DMA structure.
 * @param	Channel is DMA source or destination channel.
 *
 * @return
 * 		- XST_SUCCESS on success.
 * 		- XPLMI_ERR_FROM_SSIT_SLAVE if error received from slave SLR.
 *
 **************************************************************************************************/
static int XPlmi_SsitWaitForDmaDone(XPmcDma *DmaPtr, XPmcDma_Channel Channel)
{
	int Status = XST_FAILURE;

	Status = XPmcDma_WaitForDoneTimeout(DmaPtr, Channel);
	if (Status != XST_SUCCESS) {
		XPlmi_Printf(DEBUG_GENERAL, "SSIT Wait for DMA Done Timed Out\r\n");
		Status = XPLMI_ERR_FROM_SSIT_SLAVE;
	}

	return Status;
}

/**************************************************************************************************/
/**
 * @brief	This function is used to check and wait for DMA done when sending data to SSIT Slave
 * 		SLRs.
 *
 * @param	DestAddr holds the address of the destination buffer.
 *
 * @return
 * 		- XST_SUCCESS on success.
 * 		- Error code on failure.
 *
 **************************************************************************************************/
XPlmi_WaitForDmaDone_t XPlmi_GetPlmiWaitForDone(u64 DestAddr)
{
	XPlmi_WaitForDmaDone_t XPlmi_WaitForDmaDone;

	if ((DestAddr >= XPLMI_PMC_ALIAS1_BASEADDR) && (DestAddr < XPLMI_PMC_ALIAS_MAX_ADDR)) {
		/*
		 * Call XPlmi_SsitWaitForDmaDone() if DMA transfer is to SSIT Slave SLRs
		 */
		XPlmi_WaitForDmaDone = XPlmi_SsitWaitForDmaDone;
	} else {
		XPlmi_WaitForDmaDone = XPmcDma_WaitForDone;
	}

	return XPlmi_WaitForDmaDone;
}

/**************************************************************************************************/
/**
 * @brief	This function provides TraceLog instance.
 *
 * @return
 * 		- Pointer to TraceLog variable.
 *
 **************************************************************************************************/
XPlmi_CircularBuffer *XPlmi_GetTraceLogInst(void)
{
	/* Trace log buffer */
	static XPlmi_CircularBuffer TraceLog = {
		.StartAddr = XPLMI_TRACE_LOG_BUFFER_ADDR,
		.Len = XPLMI_TRACE_LOG_BUFFER_LEN,
		.Offset = 0x0U,
		.IsBufferFull = (u32)FALSE,
	};

	return &TraceLog;
}

/**************************************************************************************************/
/**
 * @brief	This function processes and provides SrcAddr and DestAddr for cfi readback
 *
 * @param	SlrType is the type of Slr passed in readback cmd.
 * @param	SrcAddr is the pointer to the SrcAddr variable.
 * @param	DestAddrRead is the pointer to the DestAddrRead variable.
 *
 **************************************************************************************************/
void XPlmi_GetReadbackSrcDest(u32 SlrType, u64 *SrcAddr, u64 *DestAddrRead)
{
	if ((SrcAddr != NULL) && (DestAddrRead != NULL)) {
		if (SlrType == XPLMI_READBACK_SLR_TYPE_1) {
			*SrcAddr = XPLMI_SLR1_CFU_FDRO_2_ADDR;
			*DestAddrRead = XPLMI_SLR1_CFU_STREAM_2_ADDR;
		} else if (SlrType == XPLMI_READBACK_SLR_TYPE_2) {
			*SrcAddr = XPLMI_SLR2_CFU_FDRO_2_ADDR;
			*DestAddrRead = XPLMI_SLR2_CFU_STREAM_2_ADDR;
		} else if (SlrType == XPLMI_READBACK_SLR_TYPE_3) {
			*SrcAddr = XPLMI_SLR3_CFU_FDRO_2_ADDR;
			*DestAddrRead = XPLMI_SLR3_CFU_STREAM_2_ADDR;
		} else {
			*SrcAddr = (u64)CFU_FDRO_2_ADDR;
			*DestAddrRead = (u64)CFU_STREAM_2_ADDR;
		}
	}
}

/**************************************************************************************************/
/**
 * @brief	This will add the GIC interrupt task handler to the TaskQueue.
 *
 * @param	PlmIntrId is the GIC interrupt ID of the task.
 *
 **************************************************************************************************/
void XPlmi_GicAddTask(u32 PlmIntrId)
{
#ifdef XPLMI_IPI_DEVICE_ID
	u32 IpiIntrVal;
	u32 IpiMaskVal;
	u32 Index;
	u32 IpiIndexMask;
	u32 IpiIndex;
	const XIpiPsu *IpiInstPtr = XPlmi_GetIpiInstance();

	/* Check if the received interrupt is IPI */
	if (PlmIntrId == XPLMI_IPI_INTR_ID) {
		IpiIntrVal = Xil_In32(IPI_PMC_ISR);
		IpiMaskVal = Xil_In32(IPI_PMC_IMR);
		XPlmi_Out32(IPI_PMC_IDR, IpiIntrVal);
		/*
		 * Check IPI source channel and add channel specific task to task queue according to
		 * the channel priority
		 */
		for (Index = 0U; Index < (u32)XPLMI_IPI_MASK_COUNT; ++Index) {
			IpiIndexMask = IpiInstPtr->Config.TargetList[Index].Mask;
			IpiIndex = IpiInstPtr->Config.TargetList[Index].BufferIndex;
			if (((IpiIntrVal & IpiIndexMask) != 0U) &&
			    ((IpiMaskVal & IpiIndexMask) == 0U)) {
				XPlmi_GicIntrAddTask(XPlmi_GetIpiIntrId(IpiIndex));
			}
		}
	} else
#endif /* XPLMI_IPI_DEVICE_ID */
	{
		/* Add task to the task queue */
		XPlmi_GicIntrAddTask(PlmIntrId | XPLMI_IOMODULE_PPU1_HW_INT |
			(XPLMI_HW_INT_GIC_IRQ << XPLMI_HW_SW_INTR_SHIFT));
	}
}

/**************************************************************************************************/
/**
 * @brief	This function registers and enables IPI interrupt.
 *
 * @return
 * 		- XST_SUCCESS on success.
 * 		- XST_FAILURE on failure.
 *
 **************************************************************************************************/
int XPlmi_RegisterNEnableIpi(void)
{
	/*
	 * Only Enable Ipi Interrupt for versal_2vp, since GIC handler as the logic to add IPI
	 * task
	 */
	XPlmi_EnableIpiIntr();

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
 * @brief	This function provides handler for HW interrupts
 *
 * @param	CallbackRef is presently the interrupt number that is received
 *
 *****************************************************************************/
static void XPlmi_HwIntrHandler(void *CallbackRef)
{
	u32 HwIntStatus;
	u32 HwIntMask;

	HwIntStatus = XPlmi_In32(PMC_GLOBAL_PPU1_HW_INT_ADDR);
	HwIntMask = XPlmi_In32(PMC_GLOBAL_PPU1_HW_INT_MASK_ADDR);

	if ((HwIntStatus & PMC_GLOBAL_PPU1_HW_INT_GICP_IRQ_MASK) ==
			PMC_GLOBAL_PPU1_HW_INT_GICP_IRQ_MASK) {
		if ((HwIntMask & PMC_GLOBAL_PPU1_HW_INT_GICP_IRQ_MASK) == 0U) {
			XPlmi_GicIntrHandler(CallbackRef);
			XPlmi_Out32(PMC_GLOBAL_PPU1_HW_INT_ADDR,
				PMC_GLOBAL_PPU1_HW_INT_GICP_IRQ_MASK);
			HwIntStatus &= ~PMC_GLOBAL_PPU1_HW_INT_GICP_IRQ_MASK;
		}
	}
	/* Call XPlmiIntrHandler if any other interrupt is set */
	if ((HwIntStatus & (~PMC_GLOBAL_PPU1_HW_INT_GICP_IRQ_MASK)) != 0U){
		if ((HwIntMask & (~PMC_GLOBAL_PPU1_HW_INT_GICP_IRQ_MASK)) == 0U){
			XPlmi_IntrHandler(CallbackRef);
			XPlmi_Out32(PMC_GLOBAL_PPU1_HW_INT_ADDR,
				HwIntStatus & (~HwIntMask));
		}
	}
}

/**************************************************************************************************/
/**
 * @brief	This function registers and enables IPI interrupt.
 *
 **************************************************************************************************/
void XPlmi_EnableIomoduleIntr(void)
{
	XPlmi_PlmIntrEnable(XPLMI_IOMODULE_PPU1_HW_INT);
	XPlmi_PlmIntrEnable(XPLMI_IOMODULE_ERR_IRQ);
	XPlmi_PlmIntrEnable(XPLMI_IOMODULE_PMC_PIT3_IRQ);
	XPlmi_Out32(PMC_GLOBAL_PPU1_HW_INT_ENABLE_ADDR,
			PMC_GLOBAL_PPU1_HW_INT_GICP_IRQ_MASK);
	XPlmi_Out32(PMC_GLOBAL_PPU1_HW_INT_ENABLE_ADDR,
			PMC_GLOBAL_PPU1_HW_INT_MB_DATA_MASK);
	XPlmi_Out32(PMC_GLOBAL_PPU1_HW_INT_ENABLE_ADDR,
			PMC_GLOBAL_PPU1_HW_INT_MB_INSTR_MASK);
	XPlmi_Out32(PMC_GLOBAL_PPU1_PL_INT_ENABLE_ADDR,
			PMC_GLOBAL_PPU1_PL_INT_GPI_MASK);
}

/**************************************************************************************************/
/**
 * @brief	It sets the PMC IRO frequency.
 *
 * @return
 * 		- XST_SUCCESS on success.
 * 		- Error code failure.
 *
 **************************************************************************************************/
int XPlmi_SetPmcIroFreq(void)
{
	int Status = XST_FAILURE;
	u32 RawVoltage;
	u32 PmcVersion = XPlmi_In32(PMC_TAP_VERSION);
	u32 *PmcIroFreq = XPlmi_GetPmcIroFreq();

	PmcVersion = ((PmcVersion & PMC_TAP_VERSION_PMC_VERSION_MASK) >>
		      PMC_TAP_VERSION_PMC_VERSION_SHIFT);
	if (PmcVersion == XPLMI_PMC_VERSION_1_0) {
		*PmcIroFreq = (XPLMI_PMC_IRO_FREQ_320_MHZ);
	} else {
		RawVoltage = Xil_In32(XPLMI_SYSMON_SUPPLY0_ADDR);
		RawVoltage &= XPLMI_SYSMON_SUPPLYX_MASK;
		/* Update IR0 frequency to 400MHz for MP and HP parts */
		XPlmi_Out32(EFUSE_CTRL_WR_LOCK, XPLMI_EFUSE_CTRL_UNLOCK_VAL);
		if (RawVoltage >= XPlmi_GetRawVoltage(XPLMI_VCC_PMC_MP_MIN)) {
			*PmcIroFreq = XPLMI_PMC_IRO_FREQ_400_MHZ;
			XPlmi_Out32(EFUSE_CTRL_ANLG_OSC_SW_1LP, XPLMI_EFUSE_IRO_TRIM_FAST);
		} else {
			*PmcIroFreq = XPLMI_PMC_IRO_FREQ_320_MHZ;
			XPlmi_Out32(EFUSE_CTRL_ANLG_OSC_SW_1LP, XPLMI_EFUSE_IRO_TRIM_SLOW);
		}
		XPlmi_Out32(EFUSE_CTRL_WR_LOCK, XPLMI_EFUSE_CTRL_LOCK_VAL);
	}
	Status = (int)Xil_SetMBFrequency(*PmcIroFreq);

	return Status;
}

/**************************************************************************************************/
/**
 * @brief	This functions provides the PIT1 and PIT2 reset values.
 *
 * @param	Pit1ResetValue is the pointer to store the PIT1 reset value.
 * @param	Pit2ResetValue is the pointer to store the PIT12 reset value.
 *
 * @return
 * 		- XST_SUCCESS always.
 *
 **************************************************************************************************/
int XPlmi_GetPitResetValues(u32 *Pit1ResetValue, u32 *Pit2ResetValue)
{
	*Pit1ResetValue = XPLMI_PIT1_RESET_VALUE;
	*Pit2ResetValue = XPLMI_PIT2_RESET_VALUE;

	return XST_SUCCESS;
}

/**************************************************************************************************/
/**
 * @brief	This function provides Gic interrupt id.
 *
 * @param	GicPVal indicates GICP source.
 * @param	GicPxVal indicates GICPx source.
 *
 * @return
 * 		- Gic Interrupt Id.
 *
 **************************************************************************************************/
u32 XPlmi_GetGicIntrId(u32 GicPVal, u32 GicPxVal)
{
	u32 IntrId;

	IntrId = (GicPVal << XPLMI_GICP_INDEX_SHIFT) |
			(GicPxVal << XPLMI_GICPX_INDEX_SHIFT);
	IntrId |= (XPLMI_HW_INT_GIC_IRQ << XPLMI_HW_SW_INTR_SHIFT);

	return IntrId | XPLMI_IOMODULE_PPU1_HW_INT;
}

/**************************************************************************************************/
/**
 * @brief	This function provides IPI interrupt id.
 *
 * @param	BufferIndex is the Ipi target buffer index.
 *
 * @return
 * 		- IPI interrupt id.
 *
 **************************************************************************************************/
u32 XPlmi_GetIpiIntrId(u32 BufferIndex)
{
	return XPLMI_GIC_IPI_INTR_ID | (BufferIndex << XPLMI_IPI_INDEX_SHIFT);
}

/**************************************************************************************************/
/**
 * @brief	This function enables IPI interrupt.
 *
 **************************************************************************************************/
void XPlmi_EnableIpiIntr(void)
{
	XPlmi_GicIntrEnable(XPLMI_PMC_GIC_IRQ_GICP0, XPLMI_GICP0_SRC27);
}

/**************************************************************************************************/
/**
 * @brief	This function clears IPI interrupt.
 *
 **************************************************************************************************/
void XPlmi_ClearIpiIntr(void)
{
	XPlmi_GicIntrClearStatus(XPLMI_PMC_GIC_IRQ_GICP0, XPLMI_GICP0_SRC27);
}

/**************************************************************************************************/
/**
 * @brief	This function Disables CFRAME Isolation.
 *
 **************************************************************************************************/
void XPlmi_DisableCFrameIso(void)
{
	u8 PmcVersion = (u8)(XPlmi_In32(PMC_TAP_VERSION) & PMC_TAP_VERSION_PMC_VERSION_MASK);

	if (PmcVersion == XPLMI_SILICON_ES1_VAL) {
		XPlmi_UtilRMW(PMC_GLOBAL_DOMAIN_ISO_CNTRL,
			      PMC_GLOBAL_DOMAIN_ISO_CNTRL_PMC_PL_CFRAME_MASK, 0U);
	}
}

/**************************************************************************************************/
/**
 * @brief	This function returns KAT status from RCTA area.
 *
 * @param	PlmKatStatus is the pointer to the variable which holds kat status.
 *
 **************************************************************************************************/
void XPlmi_GetBootKatStatus(volatile u32 *PlmKatStatus)
{
	volatile u32 CryptoKatEn = (u32)TRUE;
	volatile u32 CryptoKatEnTmp = (u32)TRUE;

	*PlmKatStatus = 0U;
	CryptoKatEn = XPlmi_IsCryptoKatEn();
	CryptoKatEnTmp = XPlmi_IsCryptoKatEn();
	if((CryptoKatEn == (u32)TRUE) || (CryptoKatEnTmp == (u32)TRUE)) {
		*PlmKatStatus = XPlmi_GetKatStatus();
	} else {
		*PlmKatStatus = XPLMI_KAT_MASK;
	}
}

/**************************************************************************************************/
/**
 * @brief	This function clears SSS Cfg error set during ROM PCR Extension.
 *
 **************************************************************************************************/
void XPlmi_ClearSSSCfgErr(void)
{
	static u32 SSSCfgErrCleared = (u32)FALSE;
	u32 PmcIsr;

	if ((SSSCfgErrCleared != (u32)TRUE) && (XPlmi_IsPlmUpdateDone() != (u8)TRUE)) {
		PmcIsr = XPlmi_In32(PMC_GLOBAL_ISR);
		if ((PmcIsr & PMC_GLOBAL_SSS_CFG_ERR_MASK) ==
				PMC_GLOBAL_SSS_CFG_ERR_MASK) {
			XPlmi_Out32(PMC_GLOBAL_ISR, PMC_GLOBAL_SSS_CFG_ERR_MASK);
			SSSCfgErrCleared = (u32)TRUE;
		}
	}
}

/**************************************************************************************************/
/**
 * @brief	This function defines BufferList and returns the address of the same.
 *
 * @param	BufferListType is the proc list type if it is stored in PMC or PSM RAM.
 *
 * @return
 * 		- BufferList is the address of BufferList structure.
 *
 **************************************************************************************************/
XPlmi_BufferList* XPlmi_GetBufferList(u32 BufferListType)
{
	/**
	 * - Create static BufferList structure and initialize with zero during
	 * initial call.
	 */
	static XPlmi_BufferList PsmBufferList = {0U};
	static XPlmi_BufferData PsmBuffers[XPLMI_MAX_PSM_BUFFERS + 1U] = {0U};
	static XPlmi_BufferList PmcBufferList = {0U};
	static XPlmi_BufferData PmcBuffers[XPLMI_MAX_PMC_BUFFERS + 1U] = {0U};
	XPlmi_BufferList *BufferList = &PsmBufferList;

	PsmBufferList.Data = PsmBuffers;
	PsmBufferList.MaxBufferCount = XPLMI_MAX_PSM_BUFFERS;
	PmcBufferList.Data = PmcBuffers;
	PmcBufferList.MaxBufferCount = XPLMI_MAX_PMC_BUFFERS;

	if (BufferListType == XPLMI_PMC_BUFFER_LIST) {
		BufferList = &PmcBufferList;

		/**
		 * - Initialize first Data address of the PmcBufferList to the PMC RAM
		 * reserved address and BufferMemSize with the Max Size allocated
		 */
		PmcBufferList.Data[0U].Addr = XPLMI_PMCRAM_BUFFER_MEMORY;
		PmcBufferList.BufferMemSize = XPLMI_PMCRAM_BUFFER_MEMORY_LENGTH;
		PmcBufferList.IsBufferMemAvailable = (u8)TRUE;
	}

	return BufferList;
}

/**************************************************************************************************/
/**
 * @brief	This function will return KAT status of given mask.
 *
 * @param	PlmKatMask contains the KAT mask
 *
 * @return
 *		- TRUE  If KAT ran.
 *		- FALSE If KAT didn't ran.
 *
 **************************************************************************************************/
u8 XPlmi_IsKatRan(u32 PlmKatMask)
{
	volatile u32 CryptoKatEn = XPlmi_IsCryptoKatEn();
	volatile u32 CryptoKatEnTmp = XPlmi_IsCryptoKatEn();
	u8 IsKatRan = FALSE;

	if ((CryptoKatEn == (u32)TRUE) || (CryptoKatEnTmp == (u32)TRUE)) {
		IsKatRan = (((XPlmi_In32(XPLMI_RTCFG_PLM_KAT_ADDR) & PlmKatMask) != 0U)?
					(u8)TRUE: (u8)FALSE);
	} else {
		IsKatRan = TRUE;
	}

	return IsKatRan;
}

/****************************************************************************/
/**
 * @brief	This function will disable the IOmodule interrupts.
 *
 ****************************************************************************/
static void XPlmi_DisableIOmodule(void)
{
	const XIOModule *IOModule = XPlmi_GetIOModuleInst();

	XIomodule_Out32(IOModule->BaseAddress + XIN_IER_OFFSET, 0U);
}

/*****************************************************************************/
/**
 * @brief	This function returns the current enabled interrupt mask of
 *		IOmodule.
 *
 * @return	Current interrupt enable mask
 *
 *****************************************************************************/
static u32 XPlmi_GetIoIntrMask(void)
{
	const XIOModule *IOModule = XPlmi_GetIOModuleInst();

	return (IOModule->CurrentIER);
}

/*****************************************************************************/
/**
 * @brief	This function enables the IOModule interrupts with provided value.
 *
 * @param	Value is the interrupt enable mask to write.
 *
 *****************************************************************************/
static void XPlmi_SetIoIntrMask(u32 Value)
{
	const XIOModule *IOModule = XPlmi_GetIOModuleInst();

	XPlmi_Out32(IOModule->BaseAddress + XIN_IER_OFFSET, Value);
}

/**************************************************************************************************/
/**
 * @brief	This function raises an interrupt request to ROM and waits for
 *		completion. Before calling this API all pre-requisites for ROM service shall
 *		be completed.
 *
 * @param	RomServiceReq ROM service interrupt request identifier
 *
 * @return
 *		- XST_SUCCESS on success.
 *		- XPLMI_ERR_INVALID_ROM_INT_REQ on invalid interrupt request for ROM.
 *
 **************************************************************************************************/
int XPlmi_RomISR(XPlmi_RomIntr RomServiceReq)
{
	int Status = XST_FAILURE;
	u32 IntrMask;
	u32 IoMask;

	if ((RomServiceReq >= XPLMI_INVALID_INT) ||
		(RomServiceReq == XPLMI_PLM_UPDT_REQ)) {
		Status = XPLMI_ERR_INVALID_ROM_INT_REQ;
		goto END;
	}
	IntrMask = (u32)1U << (u32)RomServiceReq;
	XPlmi_Out32(PMC_GLOBAL_ROM_INT_REASON, IntrMask);
	/* Generate ROM interrupt */
	XPlmi_Out32(PMC_GLOBAL_ROM_INT, IntrMask);

	/* For DME request keeping Microblaze into sleep state */
	if (RomServiceReq == XPLMI_DME_CHL_SIGN_GEN) {
		/* Disable Interrupts */
		microblaze_disable_interrupts();
		/* Storing current interrupt enable mask of IOModule's IER */
		IoMask = XPlmi_GetIoIntrMask();
		XPlmi_DisableIOmodule();
		mb_sleep();
		Status = (int)Xil_WaitForEvent((UINTPTR)PMC_GLOBAL_PPU_1_RST_MODE,
			PMC_GLOBAL_PPU_1_RST_MODE_WAKEUP_MASK,
			PMC_GLOBAL_PPU_1_RST_MODE_WAKEUP_MASK,
			XPLMI_ROM_SERVICE_TIMEOUT);
		XPlmi_PpuWakeUpDis();
		XPlmi_SetIoIntrMask(IoMask);
		microblaze_enable_interrupts();
		if (Status != XST_SUCCESS) {
			goto END;
		}
	}
	Status = (int)Xil_WaitForEvent((UINTPTR)PMC_GLOBAL_ROM_INT_REASON,
		IntrMask, IntrMask, XPLMI_ROM_SERVICE_TIMEOUT);

	XPlmi_Out32(PMC_GLOBAL_ROM_INT_REASON, IntrMask);
END:
	return Status;
}

/**************************************************************************************************/
/**
* @brief    This function is used to get the local SLR index
*
* @return   Returns Local SLR index
*
***************************************************************************************************/
u8 XPlmi_GetSlrIndex(void)
{
	return 0x0U;
}

/**************************************************************************************************/
/**
* @brief    This function is used to get the combined mask of all slave SLRs
*
* @return   Returns combined mask of all slave SLRs
*
***************************************************************************************************/
u32 XPlmi_GetSlavesSlrMask(void)
{
	return 0x0U;
}

/**************************************************************************************************/
/**
* @brief    This function is used to register any event between Master and Slave SLRs
*
* @param    EventIndex is the Index of the event to be registered
* @param    Handler is the handler to be executed when this Event occurs
* @param    EventOrigin is the SLR origin which can trigger the event
*
* @return
* 		- Returns XST_SUCCESS for non-SSIT designs
*
***************************************************************************************************/
int XPlmi_SsitRegisterEvent(u32 EventIndex, XPlmi_EventHandler_t Handler, u8 EventOrigin)
{
	(void)EventIndex;
	(void)Handler;
	(void)EventOrigin;

	return XST_SUCCESS;
}

/**************************************************************************************************/
/**
* @brief    This function is used to write the event buffer data and trigger the Message Event
*
* @param    SlrIndex is the index of the SLR to which the event buffer need to be written
* @param    ReqBuf is the buffer from where the event buffer data to be written to Slave SLRs
*		event buffer
* @param    ReqBufSize is the size of the ReqBuf
*
* @return
* 		- Returns XST_SUCCESS for non-SSIT designs
*
***************************************************************************************************/
int XPlmi_SsitWriteEventBufferAndTriggerMsgEvent(u8 SlrIndex, u32* ReqBuf, u32 ReqBufSize)
{
	(void)SlrIndex;
	(void)ReqBuf;
	(void)ReqBufSize;

	return XST_SUCCESS;
}

/**************************************************************************************************/
/**
* @brief    This function is used to read the event buffer of Master SLR in Slave SLRs where the
*		message event is received
*
* @param    ReqBuf is the buffer to which the event buffer data to be written
* @param    ReqBufSize is the size of the ReqBuf
*
* @return
* 		- Returns XST_SUCCESS for non-SSIT designs
*
***************************************************************************************************/
int XPlmi_SsitReadEventBuffer(u32* ReqBuf, u32 ReqBufSize)
{
	(void)ReqBuf;
	(void)ReqBufSize;

	return XST_SUCCESS;
}

/**************************************************************************************************/
/**
* @brief    This function is used to trigger the given event to the given SLR
*
* @param    SlrIndex is the index of the SLR to which the event to be triggered
* @param    EventIndex is the Index of the event to be triggered
*
* @return
* 		- Returns XST_SUCCESS for non-SSIT designs
*
***************************************************************************************************/
int XPlmi_SsitTriggerEvent(u8 SlrIndex, u32 EventIndex)
{
	(void)SlrIndex;
	(void)EventIndex;

	return XST_SUCCESS;
}

/**************************************************************************************************/
/**
* @brief    This function is used to wait for the event to be acknowledged by the given SLR after
*		triggering
*
* @param    SlrIndex is the SLR index where the event is triggered
* @param    EventIndex is the Index of the event triggered
* @param    TimeOut is the timeout in us to wait for the acknowledgment
*
* @return
* 		- Returns XST_SUCCESS for non-SSIT designs
*
***************************************************************************************************/
int XPlmi_SsitWaitForEvent(u8 SlrIndex, u32 EventIndex, u32 TimeOut)
{
	(void)SlrIndex;
	(void)EventIndex;
	(void)TimeOut;

	return XST_SUCCESS;
}

/**************************************************************************************************/
/**
* @brief    This function is used to read the response (if applicable) in Master SLR once the
*		event is acknowledged by Slave SLR
*
* @param    SlrIndex is the SLR index from the response need to be read
* @param    RespBuf is the Response Buffer to where the response to be written
* @param    RespBufSize is the size of the response buffer
*
* @return
* 		- Returns XST_SUCCESS for non-SSIT designs
*
***************************************************************************************************/
int XPlmi_SsitReadResponse(u8 SlrIndex, u32* RespBuf, u32 RespBufSize)
{
	(void)SlrIndex;
	(void)RespBuf;
	(void)RespBufSize;

	return XST_SUCCESS;
}

/**************************************************************************************************/
/**
* @brief    This function is used to write the response (if applicable) in Slave SLR once the
*		event is handled and acknowledge the message event
*
* @param    RespBuf is the Response Buffer data to be written
* @param    RespBufSize is the size of the response buffer
*
* @return
* 		- Returns XST_SUCCESS for non-SSIT designs
*
***************************************************************************************************/
int XPlmi_SsitWriteResponseAndAckMsgEvent(u32 *RespBuf, u32 RespBufSize)
{
	(void)RespBuf;
	(void)RespBufSize;

	return XST_SUCCESS;
}

/**************************************************************************************************/
/**
* @brief    This function is used to acknowledge the event after handling
*
* @param    SlrIndex is index of the SLR for which the event was triggered
* @param    EventIndex is the event to acknowledge
*
* @return
* 		- Returns XST_SUCCESS for non-SSIT designs
*
***************************************************************************************************/
int XPlmi_SsitAcknowledgeEvent(u8 SlrIndex, u32 EventIndex)
{
	(void)SlrIndex;
	(void)EventIndex;

	return XST_SUCCESS;
}

/**************************************************************************************************/
/**
* @brief    This function is used to get the global SLR address for the given local address
*
* @param    Address is the local address for which the global SLR address need to be calculated
* @param    SlrIndex is the index of the SLR for which the address need to be calculated
*
* @return
* 		- Returns 0x0 for non-SSIT designs
*
***************************************************************************************************/
u64 XPlmi_SsitGetSlrAddr(u32 Address, u8 SlrIndex)
{
	(void)Address;
	(void)SlrIndex;

	return 0x0U;
}

/**************************************************************************************************/
/**
* @brief    This function sends a message event with given request buffer to a slave SLR, waits
*		for event completion, reads the response from the slave SLR and returns it to the
*		caller
*
* @param    SlrIndex is the SLR index from which the response need to be read
* @param    ReqBuf is the buffer to which the event buffer data to be written
* @param    ReqBufSize is the size of the ReqBuf
* @param    RespBuf is the Response Buffer to where the response to be written
* @param    RespBufSize is the size of the response buffer
* @param    WaitForEventCompletion is the maximum time to wait before the requested message event
*		is handled by a slave SLR [in microseconds]
*
* @return
* 		- Returns XST_SUCCESS for non-SSIT designs
*
***************************************************************************************************/
int XPlmi_SsitSendMsgEventAndGetResp(u8 SlrIndex, u32 *ReqBuf, u32 ReqBufSize, u32 *RespBuf,
		u32 RespBufSize, u32 WaitForEventCompletion)
{
	(void)SlrIndex;
	(void)ReqBuf;
	(void)ReqBufSize;
	(void)RespBuf;
	(void)RespBufSize;
	(void)WaitForEventCompletion;

	return XST_SUCCESS;
}

#ifdef PLM_ENABLE_ADDR_RANGE_VALIDATION
/**************************************************************************************************/
/**
 * @brief	Out-of-line redundant wrapper around the registered address-range
 *		validation handler. The redundant (double) call keeps the glitch
 *		protection while avoiding the per-site code-size blow-up.
 *
 * @param	SubsystemId is the subsystem requesting access
 * @param	Addr is the start address of the range to validate
 * @param	Size is the size of the range to validate
 *
 * @return	XST_SUCCESS if both calls report the range valid, XST_FAILURE
 *		otherwise
 *
 **************************************************************************************************/
__attribute__((noinline)) int XPlmi_VerifyAddrRangeRedundant(u32 SubsystemId, u64 Addr, u64 Size)
{
	volatile int Status = (int)XST_FAILURE;
	volatile int StatusTmp = (int)XST_FAILURE;

	/* Redundancy to guard against glitches skipping/altering the check */
	XSECURE_REDUNDANT_CALL(Status, StatusTmp, XPlmi_IsAddrRangeValid,
			       SubsystemId, Addr, Size);
	if ((Status != (int)XST_SUCCESS) || (StatusTmp != (int)XST_SUCCESS)) {
		Status = (int)XST_FAILURE;
	}

	return Status;
}
#else
int XPlmi_VerifyAddrRangeRedundant(u32 SubsystemId, u64 Addr, u64 Size)
{
	(void)SubsystemId;
	(void)Addr;
	(void)Size;

	return (int)XST_SUCCESS;
}
#endif

/*****************************************************************************/
/**
 * @brief	This function will run the already stored proc if present
 *
 * @param	Cmd is pointer to the command structure
 *
 * @return
 * 		- XST_SUCCESS if success and error code if failure
 *
 *****************************************************************************/
int XPlmi_RunProc(XPlmi_Cmd *Cmd)
{
	int Status = XST_FAILURE;

	XPLMI_EXPORT_CMD(XPLMI_RUN_PROC_CMD_ID, XPLMI_MODULE_GENERIC_ID,
			XPLMI_CMD_ARG_CNT_ONE, XPLMI_CMD_ARG_CNT_ONE);

	/* Execute Proc with the given Proc ID */
	Status = XPlmi_ExecuteProc(Cmd->Payload[INDEX_ZERO]);
	if ((Status == (int)XPLMI_ERR_PROCID_NOT_VALID) &&
	    (XPlmi_SldState() == XPLMI_SLD_IN_PROGRESS)) {
		/*
		 * During Secure Lockdown a proc not configured on this device is an
		 * expected condition; failing here would desynchronise ProcessedCdoLen
		 * and trip the END length check of the enclosing BEGIN/END block.
		 */
		XPlmi_Printf(DEBUG_GENERAL, "ProcId 0x%x not configured\n\r", Cmd->Payload[INDEX_ZERO]);
		Status = XST_SUCCESS;
	}

	return Status;
}
/** @} end of xilplmi_server_apis */
