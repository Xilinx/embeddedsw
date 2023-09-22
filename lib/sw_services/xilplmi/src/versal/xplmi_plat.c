/******************************************************************************
* Copyright (c) 2022 Xilinx, Inc. All rights reserved.
* Copyright (c) 2022 - 2023, Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
*
* @file versal/xplmi_plat.c
*
* This file contains the PLMI versal platform specific code.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- -------- -------------------------------------------------------
* 1.00  bm   07/06/2022 Initial release
*       kpt  07/21/2022 Added XPlmi_GetBootKatStatus
*       bm   07/22/2022 Update EAM logic for In-Place PLM Update
*       bm   07/22/2022 Retain critical data structures after In-Place PLM Update
*       bm   07/22/2022 Shutdown modules gracefully during update
*       ma   07/29/2022 Replaced XPAR_XIPIPSU_0_DEVICE_ID macro with
*                       XPLMI_IPI_DEVICE_ID
* 1.01  ng   11/11/2022 Fixed doxygen file name error
*       bm   01/18/2023 Fix CFI readback logic with correct keyhole size
*       bm   03/11/2023 Refactored XPlmi_VerifyAddrRange logic
*       bm   03/11/2023 Added check for blind write in UpdateResetReason
*		dd   03/28/2023 Updated doxygen comments
*       ng   03/30/2023 Updated algorithm and return values in doxygen comments
* 1.02  bm   04/28/2023 Update Trim related macros
*       ng   07/05/2023 added system device tree support
* 1.03  sk   07/18/2023 Updated error codes in VerifyAddrRange function
*       bm   09/07/2023 Allow loading of ELFs into XRAM
*       dd   09/12/2023 MISRA-C violation Rule 10.3 fixed
*       ng   09/22/2023 Fixed missing header for microblaze sleep
*
* </pre>
*
* @note
*
******************************************************************************/

/***************************** Include Files *********************************/
#include "xplmi_dma.h"
#include "xplmi_hw.h"
#include "xplmi_ssit.h"
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

/************************** Constant Definitions *****************************/
#define XPLMI_DIGEST_PMC_1_0_ROM_1_0	(0x2B004AC7U) /**< PMC1 ROM version 1
								digest */
#define XPLMI_DIGEST_PMC_2_0_ROM_2_0	(0xB576B550U) /**< PMC2 ROM version 2
								digest */

#define XPLMI_ROM_VERSION_1_0		(0x10U) /**< ROM version 1 */
#define XPLMI_ROM_VERSION_2_0		(0x20U) /**< ROM version 2 */
#define XPLMI_INVALID_ROM_VERSION	(0x0U) /**< Invalid ROM version */

/* SSIT SLR related macros */
#define XPLMI_CFU_STREAM_2_SLR_OFFSET	\
	(CFU_STREAM_2_ADDR - XPLMI_PMC_LOCAL_BASEADDR) /**< SLR offset for CFU stream 2 register */
#define XPLMI_CFU_FDRO_2_SLR_OFFSET		\
	(CFU_FDRO_2_ADDR - XPLMI_PMC_LOCAL_BASEADDR) /**< SLR offset for CFU FDRO 2 register */

#define XPLMI_SLR1_CFU_FDRO_2_ADDR	\
	(XPLMI_PMC_ALIAS1_BASEADDR + XPLMI_CFU_FDRO_2_SLR_OFFSET) /**< SLR1 CFU FDRO 2 address */
#define XPLMI_SLR2_CFU_FDRO_2_ADDR	\
	(XPLMI_PMC_ALIAS2_BASEADDR + XPLMI_CFU_FDRO_2_SLR_OFFSET) /**< SLR2 CFU FDRO 2 address */
#define XPLMI_SLR3_CFU_FDRO_2_ADDR	\
	(XPLMI_PMC_ALIAS3_BASEADDR + XPLMI_CFU_FDRO_2_SLR_OFFSET) /**< SLR3 CFU FDRO 2 address */

#define XPLMI_SLR1_CFU_STREAM_2_ADDR	\
	(XPLMI_PMC_ALIAS1_BASEADDR + XPLMI_CFU_STREAM_2_SLR_OFFSET) /**< SLR1 CFU Stream 2 address */
#define XPLMI_SLR2_CFU_STREAM_2_ADDR	\
	(XPLMI_PMC_ALIAS2_BASEADDR + XPLMI_CFU_STREAM_2_SLR_OFFSET) /**< SLR2 CFU Stream 2 address */
#define XPLMI_SLR3_CFU_STREAM_2_ADDR	\
	(XPLMI_PMC_ALIAS3_BASEADDR + XPLMI_CFU_STREAM_2_SLR_OFFSET) /**< SLR3 CFU Stream 2 address */

#define XPLMI_PMC_VOLTAGE_MULTIPLIER	(32768.0f) /**< PMC voltage multiplier */
#define XPLMI_PMC_VERSION_1_0		(0x10U) /**< PMC version 1.0 */

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/*****************************************************************************/
/**
 * @brief	This function converts voltage to raw voltage value
 *
 * @param	Voltage is the floating point voltage value
 *
 * @return	32-bit voltage value
 *
 ******************************************************************************/
static inline u32 XPlmi_GetRawVoltage(float Voltage)
{
	float RawVoltage = Voltage * XPLMI_PMC_VOLTAGE_MULTIPLIER;

	return (u32)RawVoltage;
}

/************************** Function Prototypes ******************************/
static int XPlmi_UpdateResetReason(void);
static int XPlmi_SsitWaitForDmaDone(XPmcDma *DmaPtr, XPmcDma_Channel Channel);

/************************** Variable Definitions *****************************/
/* Structure for Top level interrupt table */
static XInterruptHandler g_TopLevelInterruptTable[] = {
	XPlmi_GicIntrHandler,
	XPlmi_IntrHandler,
	XPlmi_ErrIntrHandler,
};
static u8 XRamAvailable = (u8)FALSE; /** Flag to indicate XRAM is available */

/*****************************************************************************/
/**
 * @brief	This function sets XRAM Available Flag
 *
 * @return	None
 *
 *****************************************************************************/
void XPlmi_SetXRamAvailable(void) {
	XRamAvailable = (u8)TRUE;
}

/*****************************************************************************/
/**
 * @brief	This function provides pointer to g_TopLevelInterruptTable
 *
 * @return	Pointer to g_TopLevelInterruptTable structure
 *
 *****************************************************************************/
XInterruptHandler *XPlmi_GetTopLevelIntrTbl(void)
{
	return g_TopLevelInterruptTable;
}

/*****************************************************************************/
/**
 * @brief	This function provides size of g_TopLevelInterruptTable
 *
 * @return	Size g_TopLevelInterruptTable structure
 *
 *****************************************************************************/
u8 XPlmi_GetTopLevelIntrTblSize(void)
{
	return (u8)XPLMI_ARRAY_SIZE(g_TopLevelInterruptTable);
}

/*****************************************************************************/
/**
 * @brief	This function provides pointer to BoardParams
 *
 * @return	Pointer to BoardParams
 *
 *****************************************************************************/
XPlmi_BoardParams *XPlmi_GetBoardParams(void)
{
	static XPlmi_BoardParams BoardParams __attribute__ ((aligned(4U))) = {
		.Name = {0U,},
		.Len = 0U,
	};

	return &BoardParams;
}

/*****************************************************************************/
/**
 * @brief	This function provides LpdInitialized variable pointer
 *
 * @return	Pointer to LpdInitialized variable
 *
 *****************************************************************************/
u32 *XPlmi_GetUartBaseAddr(void)
{
	static u32 UartBaseAddr = XPLMI_INVALID_UART_BASE_ADDR; /**< Base address of Uart */

	return &UartBaseAddr;
}

/*****************************************************************************/
/**
 * @brief	This function provides LpdInitialized variable pointer
 *
 * @return	Pointer to LpdInitialized variable
 *
 *****************************************************************************/
u32 *XPlmi_GetLpdInitialized(void)
{
	static u32 LpdInitialized = 0U;

	return &LpdInitialized;
}

/*****************************************************************************/
/**
 * @brief	This function performs initialization of platform specific RCTA
 *			registers
 *
 * @return
 * 			- None
 *
 *****************************************************************************/
void XPlmi_RtcaPlatInit(void)
{
	/* Versal Net specific RTCA registers Init */
	/* MIO flush RTCFG init */
	XPlmi_Out32(XPLMI_RTCFG_MIO_WA_BANK_500_ADDR, XPLMI_MIO_FLUSH_ALL_PINS);
	XPlmi_Out32(XPLMI_RTCFG_MIO_WA_BANK_501_ADDR, XPLMI_MIO_FLUSH_ALL_PINS);
	XPlmi_Out32(XPLMI_RTCFG_MIO_WA_BANK_502_ADDR, XPLMI_MIO_FLUSH_ALL_PINS);
	XPlmi_Out32(XPLMI_RTCFG_RST_PL_POR_WA, 0U);
}

/*****************************************************************************/
/**
 * @brief	This function prints ROM version using ROM digest value.
 *
 * @return
 * 			- None
 *
 *****************************************************************************/
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
		XPlmi_Printf(DEBUG_INFO, "ROM Version: v%u.%u\n\r",
			(RomVersion >> 4U), (RomVersion & 15U));
	}
}

/*****************************************************************************/
/**
 * @brief	This function performs plmi pre-initializaton.
 *
 * @return
 * 			- Reset reason status.
 *
 *****************************************************************************/
int XPlmi_PreInit(void)
{
	return XPlmi_UpdateResetReason();
}

/*****************************************************************************/
/**
 * @brief	This function updates reset reason.
 *
 * @return
 * 			- XST_SUCCESS on success and error code on failure
 *
 *****************************************************************************/
static int XPlmi_UpdateResetReason(void)
{
	int Status = XST_FAILURE;
	u32 AccResetReason = XPlmi_In32(PMC_GLOBAL_PERS_GEN_STORAGE2) &
				PERS_GEN_STORAGE2_ACC_RR_MASK;
	u32 ResetReason = XPlmi_In32(CRP_RESET_REASON) &
				CRP_RESET_REASON_MASK;

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

/*****************************************************************************/
/**
 * @brief	This function is used to check and wait for DMA done when sending
 *          data to SSIT Slave SLRs.
 *
 * @param	DmaPtr is pointer to DMA structure
 * @param	Channel is DMA source or destination channel
 *
 * @return
 * 			- XST_SUCCESS on success.
 * 			- XPLMI_ERR_FROM_SSIT_SLAVE if error received from slave SLR.
 *
 *****************************************************************************/
static int XPlmi_SsitWaitForDmaDone(XPmcDma *DmaPtr, XPmcDma_Channel Channel)
{
	int Status = XST_FAILURE;

	Status = XPmcDma_WaitForDoneTimeout(DmaPtr, Channel);
	if (Status != XST_SUCCESS) {
		XPlmi_Printf(DEBUG_GENERAL, "SSIT Wait for DMA Done Timed Out\r\n");
		Status = (int)XPLMI_ERR_FROM_SSIT_SLAVE;
	}

	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function is used to check and wait for DMA done when sending
 *          data to SSIT Slave SLRs.
 *
 * @param	DestAddr holds the address of the destination buffer
 *
 * @return
 * 			- XST_SUCCESS on success and error code on failure
 *
 *****************************************************************************/
XPlmi_WaitForDmaDone_t XPlmi_GetPlmiWaitForDone(u64 DestAddr)
{
	XPlmi_WaitForDmaDone_t XPlmi_WaitForDmaDone;

	if ((DestAddr >= XPLMI_PMC_ALIAS1_BASEADDR) &&
		(DestAddr < XPLMI_PMC_ALIAS_MAX_ADDR)) {
		/*
		 * Call XPlmi_SsitWaitForDmaDone() if DMA transfer is to
		 * SSIT Slave SLRs
		 */
		XPlmi_WaitForDmaDone = XPlmi_SsitWaitForDmaDone;
	} else {
		XPlmi_WaitForDmaDone = XPmcDma_WaitForDone;
	}

	return XPlmi_WaitForDmaDone;
}

/*****************************************************************************/
/**
 * @brief	This function provides TraceLog instance
 *
 * @return	Pointer to TraceLog variable
 *
 *****************************************************************************/
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

/*****************************************************************************/
/**
 * @brief	This function processes and provides SrcAddr and DestAddr for
 *			cfi readback
 *
 * @param	SlrType is the type of Slr passed in readback cmd
 * @param	SrcAddr is the pointer to the SrcAddr variable
 * @param	DestAddrRead is the pointer to the DestAddrRead variable
 *
 * @return
 * 			- None
 *
 *****************************************************************************/
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

/*****************************************************************************/
/**
 * @brief	This will add the GIC interrupt task handler to the TaskQueue.
 *
 * @param	PlmIntrId is the GIC interrupt ID of the task
 *
 * @return
 * 			- None
 *
 *****************************************************************************/
void XPlmi_GicAddTask(u32 PlmIntrId)
{
#ifdef XPLMI_IPI_DEVICE_ID
	u16 IpiIntrVal;
	u16 IpiMaskVal;
	u8 IpiIndex;
	u16 IpiIndexMask;

	/* Check if the received interrupt is IPI */
	if (PlmIntrId == XPLMI_IPI_INTR_ID) {
		IpiIntrVal = (u16)Xil_In32(IPI_PMC_ISR);
		IpiMaskVal = (u16)Xil_In32(IPI_PMC_IMR);
		XPlmi_Out32(IPI_PMC_IDR, IpiIntrVal);
		/*
		 * Check IPI source channel and add channel specific task to
		 * task queue according to the channel priority
		 */
		for (IpiIndex = 0U; IpiIndex < XPLMI_IPI_MASK_COUNT; ++IpiIndex) {
			IpiIndexMask = (u16)1U << IpiIndex;
			if (((IpiIntrVal & IpiIndexMask) != 0U) &&
				((IpiMaskVal & IpiIndexMask) == 0U)) {
				XPlmi_GicIntrAddTask(XPlmi_GetIpiIntrId((u32)IpiIndex));
			}
		}
	} else
#endif /* XPLMI_IPI_DEVICE_ID */
	{
		/* Add task to the task queue */
		XPlmi_GicIntrAddTask(PlmIntrId | XPLMI_IOMODULE_PMC_GIC_IRQ);
	}
}

/*****************************************************************************/
/**
 * @brief	This function registers and enables IPI interrupt
 *
 * @return
 * 			- XST_SUCCESS on success and XST_FAILURE on failure
 *
 *****************************************************************************/
int XPlmi_RegisterNEnableIpi(void)
{
	/*
	 * Only Enable Ipi Interrupt for versal, since GIC handler as the logic
	 * to add IPI task
	 */
	XPlmi_EnableIpiIntr();

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
 * @brief	This function registers and enables IPI interrupt
 *
 * @return
 * 			- None
 *
 *****************************************************************************/
void XPlmi_EnableIomoduleIntr(void)
{
	XPlmi_PlmIntrEnable(XPLMI_IOMODULE_PMC_GIC_IRQ);
	XPlmi_PlmIntrEnable(XPLMI_IOMODULE_PPU1_MB_RAM);
	XPlmi_PlmIntrEnable(XPLMI_IOMODULE_ERR_IRQ);
	XPlmi_PlmIntrEnable(XPLMI_IOMODULE_PMC_GPI);
	XPlmi_PlmIntrEnable(XPLMI_IOMODULE_PMC_PIT3_IRQ);
}

/*****************************************************************************/
/**
* @brief	It sets the PMC IRO frequency.
*
* @return
* 			- XST_SUCCESS on success and error code failure
*
*****************************************************************************/
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
	}
	else {
		RawVoltage = Xil_In32(XPLMI_SYSMON_SUPPLY0_ADDR);
		RawVoltage &= XPLMI_SYSMON_SUPPLYX_MASK;
		/* Update IR0 frequency to 400MHz for MP and HP parts */
		XPlmi_Out32(EFUSE_CTRL_WR_LOCK, XPLMI_EFUSE_CTRL_UNLOCK_VAL);
		if (RawVoltage >= XPlmi_GetRawVoltage(XPLMI_VCC_PMC_MP_MIN)) {
			*PmcIroFreq = XPLMI_PMC_IRO_FREQ_400_MHZ;
			XPlmi_Out32(EFUSE_CTRL_ANLG_OSC_SW_1LP,
				XPLMI_EFUSE_IRO_TRIM_FAST);
		}
		else {
			*PmcIroFreq = XPLMI_PMC_IRO_FREQ_320_MHZ;
			XPlmi_Out32(EFUSE_CTRL_ANLG_OSC_SW_1LP,
				XPLMI_EFUSE_IRO_TRIM_SLOW);
		}
		XPlmi_Out32(EFUSE_CTRL_WR_LOCK, XPLMI_EFUSE_CTRL_LOCK_VAL);
	}
	Status = (int)Xil_SetMBFrequency(*PmcIroFreq);

	return Status;
}

/*****************************************************************************/
/**
* @brief	This functions provides the PIT1 and PIT2 reset values
*
* @return
* 			- XST_SUCCESS always.
*
*****************************************************************************/
int XPlmi_GetPitResetValues(u32 *Pit1ResetValue, u32 *Pit2ResetValue)
{
	*Pit1ResetValue = XPLMI_PIT1_RESET_VALUE;
	*Pit2ResetValue = XPLMI_PIT2_RESET_VALUE;

	return XST_SUCCESS;
}

/****************************************************************************/
/**
* @brief	This function is used to check if the given address range is
* 			valid. This function can be called before loading any elf or
* 			assigning any buffer in that address range
*
* @param	StartAddr is the starting address
* @param	EndAddr is the ending address
*
* @return
* 			- XST_SUCCESS on success and error code on failure
*
*****************************************************************************/
int XPlmi_VerifyAddrRange(u64 StartAddr, u64 EndAddr)
{
	volatile int Status = XST_FAILURE;

	if (EndAddr < StartAddr) {
		Status = (int)XPLMI_ERROR_INVALID_ADDRESS;
		goto END;
	}

	if ((EndAddr <= (u64)XPLMI_M_AXI_FPD_MEM_HIGH_ADDR) ||
		(StartAddr > (u64)XPLMI_4GB_END_ADDR)) {
		if ((StartAddr >= (u64)XPLMI_RSVD_BASE_ADDR) &&
			(EndAddr <= (u64)XPLMI_RSVD_HIGH_ADDR)) {
			Status = (int)XPLMI_ERROR_INVALID_ADDRESS;
		}
		else {
			/* Addr range less than AXI FPD high addr or greater
				than 2GB is considered valid */
			Status = XST_SUCCESS;
		}
	}
	else {
		if (XPlmi_IsLpdInitialized() == (u8)TRUE) {
			if ((StartAddr >= (u64)XPLMI_PSM_RAM_BASE_ADDR) &&
				(EndAddr <= (u64)XPLMI_PSM_RAM_HIGH_ADDR)) {
				/* PSM RAM is valid */
				Status = XST_SUCCESS;
			}
			else if ((StartAddr >= (u64)XPLMI_TCM0_BASE_ADDR) &&
				(EndAddr <= (u64)XPLMI_TCM0_HIGH_ADDR)) {
				/* TCM0 is valid */
				Status = XST_SUCCESS;
			}
			else if ((StartAddr >= (u64)XPLMI_TCM1_BASE_ADDR) &&
				(EndAddr <= (u64)XPLMI_TCM1_HIGH_ADDR)) {
				/* TCM1 is valid */
				Status = XST_SUCCESS;
			}
			else if ((StartAddr >= (u64)XPLMI_OCM_BASE_ADDR) &&
				(EndAddr <= (u64)XPLMI_OCM_HIGH_ADDR)) {
				/* OCM is valid */
				Status = XST_SUCCESS;
			}
			else if ((StartAddr >= (u64)XPLMI_XRAM_BASE_ADDR) &&
				(EndAddr <= (u64)XPLMI_XRAM_HIGH_ADDR)) {
				/* If XRAM is available, it is valid */
				if (XRamAvailable == (u8)TRUE) {
					Status = XST_SUCCESS;
				}
			}
			else {
				/* Rest of the Addr range is treated as invalid */
				Status = (int)XPLMI_ERROR_INVALID_ADDRESS;
			}
		}
		else {
			Status = (int)XPLMI_ERROR_LPD_NOT_INITIALIZED;
		}
	}

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function provides Gic interrupt id
 *
 * @param	GicPVal indicates GICP source
 * @param	GicPxVal indicates GICPx source

 * @return	Gic Interrupt Id
 *
 *****************************************************************************/
u32 XPlmi_GetGicIntrId(u32 GicPVal, u32 GicPxVal)
{
	u32 IntrId;

	IntrId = (GicPVal << XPLMI_GICP_INDEX_SHIFT) |
			(GicPxVal << XPLMI_GICPX_INDEX_SHIFT);

	return IntrId | XPLMI_IOMODULE_PMC_GIC_IRQ;
}

/*****************************************************************************/
/**
 * @brief	This function provides IPI interrupt id
 *
 * @param	BufferIndex is the Ipi target buffer index

 * @return	IPI interrupt id
 *
 *****************************************************************************/
u32 XPlmi_GetIpiIntrId(u32 BufferIndex)
{
	return XPLMI_GIC_IPI_INTR_ID | (BufferIndex << XPLMI_IPI_INDEX_SHIFT);
}

/*****************************************************************************/
/**
 * @brief	This function enables IPI interrupt
 *
 * @return
 * 			- None
 *
 *****************************************************************************/
void XPlmi_EnableIpiIntr(void)
{
	XPlmi_GicIntrEnable(XPLMI_PMC_GIC_IRQ_GICP0, XPLMI_GICP0_SRC27);
}

/*****************************************************************************/
/**
 * @brief	This function clears IPI interrupt
 *
 * @return
 * 			- None
 *
 *****************************************************************************/
void XPlmi_ClearIpiIntr(void)
{
	XPlmi_GicIntrClearStatus(XPLMI_PMC_GIC_IRQ_GICP0, XPLMI_GICP0_SRC27);
}

/*****************************************************************************/
/**
 * @brief	This function Disables CFRAME Isolation
 *
 * @return
 * 			- None
 *
 *****************************************************************************/
void XPlmi_DisableCFrameIso(void)
{
	u8 PmcVersion = (u8)(XPlmi_In32(PMC_TAP_VERSION) &
			PMC_TAP_VERSION_PMC_VERSION_MASK);

	if (PmcVersion == XPLMI_SILICON_ES1_VAL) {
		XPlmi_UtilRMW(PMC_GLOBAL_DOMAIN_ISO_CNTRL,
		 PMC_GLOBAL_DOMAIN_ISO_CNTRL_PMC_PL_CFRAME_MASK, 0U);
	}
}

/*****************************************************************************/
/**
 * @brief	This function returns KAT status from RCTA area.
 *
 * @param	PlmKatStatus is the pointer to the variable which holds kat status
 *
 * @return
 * 			- None
 *
 *****************************************************************************/
void XPlmi_GetBootKatStatus(volatile u32 *PlmKatStatus)
{
	volatile u8 CryptoKatEn = TRUE;
	volatile u8 CryptoKatEnTmp = TRUE;

	*PlmKatStatus = 0U;
	CryptoKatEn = XPlmi_IsCryptoKatEn();
	CryptoKatEnTmp = XPlmi_IsCryptoKatEn();
	if((CryptoKatEn == TRUE) || (CryptoKatEnTmp == TRUE)) {
		*PlmKatStatus = XPlmi_GetKatStatus();
	} else {
		*PlmKatStatus = XPLMI_KAT_MASK;
	}
}
