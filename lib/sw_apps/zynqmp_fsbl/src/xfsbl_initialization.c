/******************************************************************************
* Copyright (c) 2015 - 2021 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
 ******************************************************************************/


/*****************************************************************************/
/**
*
* @file xfsbl_initilization.c
*
* This is the file which contains initialization code for the FSBL.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- -------- -------------------------------------------------------
* 1.00  kc   10/21/13 Initial release
* 2.00  sg   13/03/15 Added QSPI 32Bit bootmode
* 3.0   bv   12/02/16 Made compliance to MISRAC 2012 guidelines
*            12/08/16 Added PL clear at initialization based on user
*                     configuration
*            01/25/17 Updated R5 TCM with lovec value in XFsbl_ProcessorInit
*                     and XFsbl_TcmEccInit is been updated such that R5_L and
*                     R5_0 don't initialize initial 32 bytes of TCM as they
*                     are holding R5 vectors
*       bv   01/29/17 Added USB boot mode initializations
*            02/11/17 Add APU only reset code.
*       vns  02/17/17 Added image header authentication
*       bv   03/17/17 Based on reset reason initializations of system, tcm etc
*                     is done.
*       vns  04/04/17 Corrected image header size.
*       ma   05/10/17 Enable PROG to PL when reset reason is ps-only reset
* 4.0   vns  03/07/18 Added boot header authentication, attributes reading
*                     from boot header local buffer, copying IV to global
*                     variable for using during decryption of partition.
* 5.0   mn   07/06/18 Add DDR initialization support for new DDR DIMM part
*       mus  02/26/19 Added support for armclang compiler
*       vns  03/14/19 Setting AES and SHA hardware engines into reset.
* 6.0   bsv  08/27/19 Added check to ensure padding in image header does not
*                     exceed allotted buffer in OCM
* 7.0   bsv  03/05/20 Restore value of SD_CDN_CTRL register before handoff
*       ma   03/19/20 Update the status of FSBL image encryption in PMU Global
*                     register
* 8.0   bsv  12/16/20 Update print format in XFsbl_EccInit to correctly print
*                     64 bit addresses and lengths
*       bsv  04/01/21 Added TPM support
*       bsv  04/28/21 Added support to ensure authenticated images boot as
*                     non-secure when RSA_EN is not programmed
*       bsv  06/10/21 Mark DDR as memory just after ECC initialization to
*                     avoid speculative accesses
*       bsv  07/07/21 Assign correct values to SecondaryBootDevice in Fsbl
*                     instance pointer
*
* </pre>
*
* @note
*
******************************************************************************/

/***************************** Include Files *********************************/
#include "xfsbl_hw.h"
#include "xfsbl_main.h"
#include "xfsbl_misc_drivers.h"
#include "xfsbl_qspi.h"
#include "xfsbl_csu_dma.h"
#include "xfsbl_board.h"
#include "xil_mmu.h"
#include "xil_cache.h"
#include "xfsbl_hooks.h"
#include "xfsbl_bs.h"
#include "xfsbl_usb.h"
#include "xfsbl_authentication.h"
#include "xfsbl_ddr_init.h"
#include "xfsbl_tpm.h"

/************************** Constant Definitions *****************************/
#define PART_NAME_LEN_MAX		20U
#define XFSBL_APU_RESET_MASK			(1U<<16U)
#define XFSBL_APU_RESET_BIT			16U

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/
static u32 XFsbl_ProcessorInit(XFsblPs * FsblInstancePtr);
static u32 XFsbl_ResetValidation(void);
static u32 XFsbl_SystemInit(XFsblPs * FsblInstancePtr);
static u32 XFsbl_PrimaryBootDeviceInit(XFsblPs * FsblInstancePtr);
static u32 XFsbl_ValidateHeader(XFsblPs * FsblInstancePtr);
static u32 XFsbl_SecondaryBootDeviceInit(XFsblPs * FsblInstancePtr);
static u32 XFsbl_DdrEccInit(void);
static u32 XFsbl_EccInit(u64 DestAddr, u64 LengthBytes);
static u32 XFsbl_TcmInit(XFsblPs * FsblInstancePtr);
static void XFsbl_EnableProgToPL(void);
static void XFsbl_ClearPendingInterrupts(void);
#ifdef XFSBL_TPM
static u32 XFsbl_MeasureFsbl(u8* PartitionHash);
#endif

/* Functions from xfsbl_misc.c */

/**
 * Functions from xfsbl_misc.c
 */
void XFsbl_RegisterHandlers(void);

/**
 *Functions from xfsbl_qspi.c
 */



/************************** Variable Definitions *****************************/
extern XFsblPs FsblInstance;

#ifdef __clang__
extern u8 Image$$DATA_SECTION$$Base;
extern u8 Image$$DATA_SECTION$$Limit;
extern u8 Image$$DUP_DATA_SECTION$$Base;
#ifdef XFSBL_TPM
extern u8 Image$$BSS_SECTION$$Base;
#endif
#else
extern  u8 __data_start;
extern  u8 __data_end;
extern  u8 __dup_data_start;
#ifdef XFSBL_TPM
extern u8 __sbss_start;
#endif
#endif

#ifndef XFSBL_BS
#ifdef __clang__
u8 ReadBuffer[XFSBL_SIZE_IMAGE_HDR]
	__attribute__((section (".bss.bitstream_buffer")));
#else
u8 ReadBuffer[XFSBL_SIZE_IMAGE_HDR]
		__attribute__((section (".bitstream_buffer")));
#endif
#else
extern u8 ReadBuffer[READ_BUFFER_SIZE];
#endif

#ifdef XFSBL_SECURE
u8 *ImageHdr = ReadBuffer;
extern u8 AuthBuffer[XFSBL_AUTH_BUFFER_SIZE];
extern u32 Iv[XIH_BH_IV_LENGTH / 4U];
#endif
u32 SdCdnRegVal;
/****************************************************************************/
/**
 * This function is used to save the data section into duplicate data section
 * so that it can be restored from in case of subsequent warm restarts
 *
 * @param  None
 *
 * @return None
 *
 * @note
 *
 *****************************************************************************/
void XFsbl_SaveData(void)
{
	const u8 *MemPtr;
  #ifdef __clang__
      u8 *ContextMemPtr = (u8 *)&Image$$DATA_SECTION$$Base;
  #else
    u8 *ContextMemPtr = (u8 *)&__dup_data_start;
  #endif

  #ifdef __clang__
    for (MemPtr = &Image$$DATA_SECTION$$Base;
	 MemPtr < &Image$$DATA_SECTION$$Limit; MemPtr++, ContextMemPtr++) {
  #else
    for (MemPtr = &__data_start; MemPtr < &__data_end; MemPtr++, ContextMemPtr++) {
  #endif
	*ContextMemPtr = *MemPtr;
    }
}

/****************************************************************************/
/**
 * This function is used to restore the data section from duplicate data section
 * in case of warm restart.
 *
 * @param  None
 *
 * @return None
 *
 * @note
 *
 *****************************************************************************/
void XFsbl_RestoreData(void)
{
	u8 *MemPtr;
#ifdef __clang__
      u8 *ContextMemPtr = (u8 *)&Image$$DATA_SECTION$$Base;
#else
    u8 *ContextMemPtr = (u8 *)&__dup_data_start;
#endif

#ifdef __clang__
    for (MemPtr = &Image$$DATA_SECTION$$Base;
	 MemPtr < &Image$$DATA_SECTION$$Limit; MemPtr++, ContextMemPtr++) {
#else
    for (MemPtr = &__data_start; MemPtr < &__data_end; MemPtr++, ContextMemPtr++) {
#endif
	*MemPtr = *ContextMemPtr;
    }
}

/****************************************************************************/
/**
 * This function is used to get the Reset Reason
 *
 * @param  None
 *
 * @return Reset Reason
 *
 * @note
 *
 *****************************************************************************/
static u32 XFsbl_GetResetReason (void)
{
	u32 Val;
	u32 Ret;

	Val = XFsbl_In32(CRL_APB_RESET_REASON);

	if ((Val & CRL_APB_RESET_REASON_PSONLY_RESET_REQ_MASK) != 0U) {
		/* Clear the PS Only reset bit as it is sticky */
		Val = CRL_APB_RESET_REASON_PSONLY_RESET_REQ_MASK;
		XFsbl_Out32(CRL_APB_RESET_REASON, Val);
		Ret = XFSBL_PS_ONLY_RESET;
		XFsbl_SaveData();
	}
	else
	{
		Ret = (XFsbl_In32(PMU_GLOBAL_GLOB_GEN_STORAGE4) & XFSBL_APU_RESET_MASK)>>(XFSBL_APU_RESET_BIT);

		if(Ret == XFSBL_SYSTEM_RESET){
			XFsbl_SaveData();
		}
		else
		{
			Ret = XFSBL_MASTER_ONLY_RESET;
			XFsbl_RestoreData();
		}
	}

	return Ret;
}

/*****************************************************************************/
/**
 * This function is initializes the processor and system.
 *
 * @param	FsblInstancePtr is pointer to the XFsbl Instance
 *
 * @return
 *          - returns the error codes described in xfsbl_error.h on any error
 * 			- returns XFSBL_SUCCESS on success
 *
 *****************************************************************************/
u32 XFsbl_Initialize(XFsblPs * FsblInstancePtr)
{
	u32 Status;
#ifdef XFSBL_ENABLE_DDR_SR
	u32 RegValue;
#endif
#ifdef XFSBL_TPM
	u8 PartitionHash[XFSBL_HASH_TYPE_SHA3] __attribute__ ((aligned (4U))) =
		{0U};
#endif

	/**
	 * Place AES and SHA engines in reset
	 */
	XFsbl_Out32(CSU_AES_RESET, CSU_AES_RESET_RESET_MASK);
	XFsbl_Out32(CSU_SHA_RESET, CSU_SHA_RESET_RESET_MASK);
#ifdef XFSBL_TPM
	Status = XFsbl_MeasureFsbl(PartitionHash);
	if (Status != XFSBL_SUCCESS) {
		goto END;
	}
#endif

	FsblInstancePtr->ResetReason = XFsbl_GetResetReason();

	/*
	 * Enables the propagation of the PROG signal to PL
	 */
	if(FsblInstancePtr->ResetReason == XFSBL_PS_ONLY_RESET)
	{
		XFsbl_EnableProgToPL();
	}

	/**
	 * Configure the system as in PSU
	 */
	if (XFSBL_MASTER_ONLY_RESET != FsblInstancePtr->ResetReason) {
		Status = XFsbl_SystemInit(FsblInstancePtr);
		if (XFSBL_SUCCESS != Status) {
			goto END;
		}
	}

	/**
	 * Place AES and SHA engines in reset
	 */
	XFsbl_Out32(CSU_AES_RESET, CSU_AES_RESET_RESET_MASK);

	XFsbl_Out32(CSU_SHA_RESET, CSU_SHA_RESET_RESET_MASK);

	/**
	 * Print the FSBL banner
	 */
	XFsbl_PrintFsblBanner();

	/* Initialize the processor */
	Status = XFsbl_ProcessorInit(FsblInstancePtr);
	if (XFSBL_SUCCESS != Status) {
		goto END;
	}

	if (XFSBL_MASTER_ONLY_RESET == FsblInstancePtr->ResetReason) {

		if (FsblInstancePtr->ProcessorID == XIH_PH_ATTRB_DEST_CPU_A53_0) {
			/* APU only restart with pending interrupts can cause the linux
			 * to hang when it starts the second time. So FSBL clears all
			 * pending interrupts in case of APU only restart.
			 */
			XFsbl_ClearPendingInterrupts();
		}
	}

	if (XFSBL_MASTER_ONLY_RESET != FsblInstancePtr->ResetReason) {
		/* Do ECC Initialization of TCM if required */
		Status = XFsbl_TcmInit(FsblInstancePtr);
		if (XFSBL_SUCCESS != Status) {
			goto END;
		}

#ifdef XFSBL_ENABLE_DDR_SR
		/*
		 * Read PMU register bit value that indicates DDR is in self
		 * refresh mode.
		 */
		RegValue = Xil_In32(XFSBL_DDR_STATUS_REGISTER_OFFSET) &
			   DDR_STATUS_FLAG_MASK;
		if (!RegValue) {
			/*
			 * Skip ECC initialization if DDR is in self refresh
			 * mode.
			 */
			Status = XFsbl_DdrEccInit();
			if (XFSBL_SUCCESS != Status) {
				goto END;
			}
			XFsbl_MarkDdrAsReserved(FALSE);
		}
		else {
			XFsbl_MarkDdrAsReserved(TRUE);
		}
#else
	/* Do ECC Initialization of DDR if required */
	Status = XFsbl_DdrEccInit();
	if (XFSBL_SUCCESS != Status) {
		goto END;
	}
	XFsbl_MarkDdrAsReserved(FALSE);
#endif

#if defined(XFSBL_PL_CLEAR) && defined(XFSBL_BS)
		/* In case of PS only reset and APU only reset skipping PCAP initialization*/
		if ((XFSBL_PS_ONLY_RESET != FsblInstancePtr->ResetReason)&&
			(XFSBL_MASTER_ONLY_RESET != FsblInstancePtr->ResetReason)) {
			Status = XFsbl_PcapInit();
			if (XFSBL_SUCCESS != Status) {
				goto END;
			}
		}
#endif

	/* Do board specific initialization if any */
	Status = XFsbl_BoardInit();
	if (XFSBL_SUCCESS != Status) {
		goto END;
	}

	/**
	 * Validate the reset reason
	 */
	Status = XFsbl_ResetValidation();
	if (XFSBL_SUCCESS != Status) {
		goto END;
		}
	}

	XFsbl_Printf(DEBUG_INFO,"Processor Initialization Done \n\r");
#ifdef XFSBL_TPM
	Status = XFsbl_TpmInit();
	if (XFSBL_SUCCESS != Status) {
		goto END;
	}

	Status = XFsbl_TpmMeasureRom();
	if (XFSBL_SUCCESS != Status) {
		goto END;
	}

	Status = XFsbl_TpmMeasurePartition(XFSBL_TPM_FSBL_PCR_INDEX,
		PartitionHash);
	if (XFSBL_SUCCESS != Status) {
		goto END;
	}
#endif

END:
	return Status;
}


/*****************************************************************************/
/**
 * This function initializes the primary and secondary boot devices
 * and validates the image header
 *
 * @param	FsblInstancePtr is pointer to the XFsbl Instance
 *
 * @return	returns the error codes described in xfsbl_error.h on any error
 * 			returns XFSBL_SUCCESS on success
 ******************************************************************************/
u32 XFsbl_BootDeviceInitAndValidate(XFsblPs * FsblInstancePtr)
{
	u32 Status;

	/**
	 * Configure the primary boot device
	 */
	Status = XFsbl_PrimaryBootDeviceInit(FsblInstancePtr);
	if (XFSBL_SUCCESS != Status) {
		goto END;
	}

	/**
	 * Read and Validate the header
	 */
	Status = XFsbl_ValidateHeader(FsblInstancePtr);
	if (XFSBL_SUCCESS != Status) {
		goto END;
	}

	/**
	 * Update the secondary boot device
	 */
	FsblInstancePtr->SecondaryBootDevice =
	 FsblInstancePtr->ImageHeader.ImageHeaderTable.PartitionPresentDevice;

	/**
	 *  Configure the secondary boot device
	 */
	Status = XFsbl_SecondaryBootDeviceInit(FsblInstancePtr);
	if (XFSBL_SUCCESS != Status) {
		goto END;
	}

END:
	return Status;
}

/*****************************************************************************/
/**
 * This function enables the propagation of the PROG signal to PL after
 * PS-only reset
 *
 * @param	None
 *
 * @return	None
 *
 ******************************************************************************/
void XFsbl_EnableProgToPL(void)
{
	u32 RegVal = 0x0U;

	/*
	 * PROG control to PL.
	 */
	Xil_Out32(CSU_PCAP_PROG, CSU_PCAP_PROG_PCFG_PROG_B_MASK);

	/*
	 * Enable the propagation of the PROG signal to the PL after PS-only reset
	 * */
	RegVal = XFsbl_In32(PMU_GLOBAL_PS_CNTRL);

	RegVal &= ~(PMU_GLOBAL_PS_CNTRL_PROG_GATE_MASK);
	RegVal |= (PMU_GLOBAL_PS_CNTRL_PROG_ENABLE_MASK);

	Xil_Out32 (PMU_GLOBAL_PS_CNTRL, RegVal);
}

/*****************************************************************************/
/**
 * This function initializes the processor and updates the cluster id
 * which indicates CPU on which fsbl is running
 *
 * @param	FsblInstancePtr is pointer to the XFsbl Instance
 *
 * @return	returns the error codes described in xfsbl_error.h on any error
 * 			returns XFSBL_SUCCESS on success
 *
 ******************************************************************************/
static u32 XFsbl_ProcessorInit(XFsblPs * FsblInstancePtr)
{
	u32 Status;
	PTRSIZE ClusterId;
	u32 RegValue;
	u32 Index=0U;
	u32 FsblProcType = 0;
	char DevName[PART_NAME_LEN_MAX];

	/**
	 * Read the cluster ID and Update the Processor ID
	 * Initialize the processor settings that are not done in
	 * BSP startup code
	 */
#ifdef ARMA53_64
	ClusterId = mfcp(MPIDR_EL1);
#else
	ClusterId = mfcp(XREG_CP15_MULTI_PROC_AFFINITY);
#endif

	XFsbl_Printf(DEBUG_INFO,"Cluster ID 0x%0lx\n\r", ClusterId);

	if (XGet_Zynq_UltraMp_Platform_info() == (u32)XPLAT_ZYNQ_ULTRA_MPQEMU) {
		/**
		 * Remmaping for R5 in QEMU
		 */
		if (ClusterId == 0x80000004U) {
			ClusterId = 0xC0000100U;
		} else if (ClusterId == 0x80000005U) {
			/* this corresponds to R5-1 */
			Status = XFSBL_ERROR_UNSUPPORTED_CLUSTER_ID;
			XFsbl_Printf(DEBUG_GENERAL,
					"XFSBL_ERROR_UNSUPPORTED_CLUSTER_ID\n\r");
			goto END;
		} else {
			/* For MISRA C compliance */
		}
	}

	/* store the processor ID based on the cluster ID */
	if ((ClusterId & XFSBL_CLUSTER_ID_MASK) == XFSBL_A53_PROCESSOR) {
		XFsbl_Printf(DEBUG_GENERAL,"Running on A53-0 ");
		FsblInstancePtr->ProcessorID =
				XIH_PH_ATTRB_DEST_CPU_A53_0;
		FsblProcType = XFSBL_RUNNING_ON_A53 << XFSBL_STATE_PROC_SHIFT;
#ifdef __aarch64__
		/* Running on A53 64-bit */
		XFsbl_Printf(DEBUG_GENERAL,"(64-bit) Processor");
		FsblInstancePtr->A53ExecState = XIH_PH_ATTRB_A53_EXEC_ST_AA64;
#else
		/* Running on A53 32-bit */
		XFsbl_Printf(DEBUG_GENERAL,"(32-bit) Processor");
		FsblInstancePtr->A53ExecState = XIH_PH_ATTRB_A53_EXEC_ST_AA32;
#endif

	} else if ((ClusterId & XFSBL_CLUSTER_ID_MASK) == XFSBL_R5_PROCESSOR) {
		/* A53ExecState is not valid for R5 */
		FsblInstancePtr->A53ExecState = XIH_INVALID_EXEC_ST;

		RegValue = XFsbl_In32(RPU_RPU_GLBL_CNTL);
		if ((RegValue & RPU_RPU_GLBL_CNTL_SLSPLIT_MASK) == 0U) {
			XFsbl_Printf(DEBUG_GENERAL,
				"Running on R5 Processor in Lockstep");
			FsblInstancePtr->ProcessorID =
				XIH_PH_ATTRB_DEST_CPU_R5_L;
			FsblProcType = XFSBL_RUNNING_ON_R5_L << XFSBL_STATE_PROC_SHIFT;
		} else {
			XFsbl_Printf(DEBUG_GENERAL,
				"Running on R5-0 Processor");
			FsblInstancePtr->ProcessorID =
				XIH_PH_ATTRB_DEST_CPU_R5_0;
			FsblProcType = XFSBL_RUNNING_ON_R5_0 << XFSBL_STATE_PROC_SHIFT;
		}

		/* Update the Low Vector locations in R5 TCM */
		while (Index<32U) {
			XFsbl_Out32(Index, XFSBL_R5_LOVEC_VALUE);
			Index += 4U;
		}

		/**
		 * Make sure that Low Vector locations are written Properly.
		 * Flush the cache
		 */
		Xil_DCacheFlush();

	} else {
		Status = XFSBL_ERROR_UNSUPPORTED_CLUSTER_ID;
		XFsbl_Printf(DEBUG_GENERAL,
				"XFSBL_ERROR_UNSUPPORTED_CLUSTER_ID\n\r");
		goto END;
	}

	/*
	 * Update FSBL processor information to PMU Global Reg5
	 * as PMU require this during boot for warm-restart feature.
	*/
	FsblProcType |= (XFsbl_In32(PMU_GLOBAL_GLOB_GEN_STORAGE5) & ~(XFSBL_STATE_PROC_INFO_MASK));

	XFsbl_Out32(PMU_GLOBAL_GLOB_GEN_STORAGE5, FsblProcType);

	/* Build Device name and print it */
	(void)XFsbl_Strcpy(DevName, "XCZU");
	(void)XFsbl_Strcat(DevName, XFsbl_GetSiliconIdName());
	(void)XFsbl_Strcat(DevName, XFsbl_GetProcEng());
	XFsbl_Printf(DEBUG_GENERAL, ", Device Name: %s\n\r", DevName);

	/**
	 * Register the exception handlers
	 */
	XFsbl_RegisterHandlers();

	/* Prints for the perf measurement */
#ifdef XFSBL_PERF

#if !defined(ARMR5)
	if (FsblInstancePtr->ProcessorID == XIH_PH_ATTRB_DEST_CPU_A53_0) {
		XFsbl_Printf(DEBUG_PRINT_ALWAYS, "Proc: A53-0 Freq: %d Hz",
				XPAR_CPU_CORTEXA53_0_CPU_CLK_FREQ_HZ);

		if (FsblInstancePtr->A53ExecState == XIH_PH_ATTRB_A53_EXEC_ST_AA32) {
			XFsbl_Printf(DEBUG_PRINT_ALWAYS, " Arch: 32 \r\n");
		}
		else if (FsblInstancePtr->A53ExecState ==
				XIH_PH_ATTRB_A53_EXEC_ST_AA64) {
			XFsbl_Printf(DEBUG_PRINT_ALWAYS, " Arch: 64 \r\n");
		}
	}
#else
	if (FsblInstancePtr->ProcessorID == XIH_PH_ATTRB_DEST_CPU_R5_0) {
		XFsbl_Printf(DEBUG_PRINT_ALWAYS, "Proc: R5-0 Freq: %d Hz \r\n",
				XPAR_PSU_CORTEXR5_0_CPU_CLK_FREQ_HZ)
	}
	else if (FsblInstancePtr->ProcessorID == XIH_PH_ATTRB_DEST_CPU_R5_L) {
		XFsbl_Printf(DEBUG_PRINT_ALWAYS, "Proc: R5-Lockstep "
			"Freq: %d Hz \r\n", XPAR_PSU_CORTEXR5_0_CPU_CLK_FREQ_HZ);
	}
#endif

#endif

	Status = XFSBL_SUCCESS;
END:
	return Status;
}

/*****************************************************************************/
/**
 * This function validates the reset reason
 *
 * @param	FsblInstancePtr is pointer to the XFsbl Instance
 *
 * @return	returns the error codes described in xfsbl_error.h on any error
 * 			returns XFSBL_SUCCESS on success
 *
 ******************************************************************************/

static u32 XFsbl_ResetValidation(void)
{
	u32 Status;
	u32 FsblErrorStatus;
#ifdef XFSBL_WDT_PRESENT
	u32 ResetReasonValue;
	u32 ErrStatusRegValue;
#endif
	/**
	 *  Read the Error Status register
	 *  If WDT reset, do fallback
	 */
	FsblErrorStatus = XFsbl_In32(XFSBL_ERROR_STATUS_REGISTER_OFFSET);

#ifdef XFSBL_WDT_PRESENT
	ResetReasonValue = XFsbl_In32(CRL_APB_RESET_REASON);

	/**
	 * Check if the reset is due to system WDT during
	 * previous FSBL execution
	 */
	if ((ResetReasonValue & CRL_APB_RESET_REASON_PMU_SYS_RESET_MASK)
			== CRL_APB_RESET_REASON_PMU_SYS_RESET_MASK) {
		ErrStatusRegValue = XFsbl_In32(PMU_GLOBAL_ERROR_STATUS_1);
		if(((ErrStatusRegValue & XFSBL_WDT_MASK) == XFSBL_WDT_MASK) &&
			(FsblErrorStatus == XFSBL_RUNNING)) {
			/* Clear the SWDT0/1 reset error */
			XFsbl_Out32(PMU_GLOBAL_ERROR_STATUS_1, XFSBL_WDT_MASK);
		/**
		 * reset is due to System WDT.
		 * Do a fallback
		 */
		Status = XFSBL_ERROR_SYSTEM_WDT_RESET;
		XFsbl_Printf(DEBUG_GENERAL,"XFSBL_ERROR_SYSTEM_WDT_RESET\n\r");
		goto END;
		}
	}
#endif
	/**
	 * Mark FSBL running in error status register to
	 * detect the WDT reset while FSBL execution
	 */
	if (FsblErrorStatus != XFSBL_RUNNING) {
		XFsbl_Out32(XFSBL_ERROR_STATUS_REGISTER_OFFSET,
						  XFSBL_RUNNING);
	}

	/**
	 *  Read system error status register
	 * 	provide FsblHook function for any action
	 */

	Status = XFSBL_SUCCESS;
#ifdef XFSBL_WDT_PRESENT
END:
#endif
	return Status;
}

/*****************************************************************************/
/**
 * This function initializes the system using the psu_init()
 *
 * @param	FsblInstancePtr is pointer to the XFsbl Instance
 *
 * @return	returns the error codes described in xfsbl_error.h on any error
 * 			returns XFSBL_SUCCESS on success
 *
 ******************************************************************************/
static u32 XFsbl_SystemInit(XFsblPs * FsblInstancePtr)
{
	u32 Status;

	if (FsblInstancePtr->ResetReason != XFSBL_PS_ONLY_RESET) {
		/**
		* MIO33 can be used to control power to PL through PMU.
		* For 1.0 and 2.0 Silicon, a workaround is needed to Powerup PL
		* before MIO33 is configured. Hence, before MIO configuration,
		* Powerup PL (but restore isolation).
		*/
		if (XGetPSVersion_Info() <= (u32)XPS_VERSION_2) {

			Status = XFsbl_PowerUpIsland(PMU_GLOBAL_PWR_STATE_PL_MASK);

			if (Status != XFSBL_SUCCESS) {
				Status = XFSBL_ERROR_PL_POWER_UP;
				XFsbl_Printf(DEBUG_GENERAL, "XFSBL_ERROR_PL_POWER_UP\r\n");
				goto END;
			}

			/* For PS only reset, make sure FSBL exits with isolation removed */

			Status  = XFsbl_IsolationRestore(PMU_GLOBAL_REQ_ISO_INT_EN_PL_NONPCAP_MASK);
			if (Status != XFSBL_SUCCESS) {
				Status = XFSBL_ERROR_PMU_GLOBAL_REQ_ISO;
				XFsbl_Printf(DEBUG_GENERAL, "XFSBL_ERROR_PMU_GLOBAL_REQ_ISO_INT_EN_PL\r\n");
				goto END;
			}
		}
	} else if (XFSBL_MASTER_ONLY_RESET == FsblInstancePtr->ResetReason) {
		/*Do nothing*/
	} else {
        /**
        * PMU-fw applied AIB between ps and pl only while ps only reset.
        * Remove the isolation so as to access pl again
        */
         u32 reg = XFsbl_In32(PMU_GLOBAL_AIB_STATUS);
         while (reg) {
		 /* Unblock the FPD and LPD AIB for PS only reset*/
             XFsbl_Out32(PMU_GLOBAL_AIB_CNTRL, 0U);
             reg = XFsbl_In32(PMU_GLOBAL_AIB_STATUS);
         }
     }

	/**
	 * psu initialization
	 */
	Status = XFsbl_HookPsuInit();

	if (XFSBL_SUCCESS != Status) {
		goto END;
	}

#ifdef XFSBL_PS_DDR
#ifdef XPAR_DYNAMIC_DDR_ENABLED
	/*
	 * This function is used for all the ZynqMP boards.
	 * This function initialize the DDR by fetching the SPD data from
	 * EEPROM. This function will determine the type of the DDR and decode
	 * the SPD structure accordingly. The SPD data is used to calculate the
	 * register values of DDR controller and DDR PHY.
	 */
	Status = XFsbl_DdrInit();
	if (XFSBL_SUCCESS != Status) {
		XFsbl_Printf(DEBUG_GENERAL,"XFSBL_DDR_INIT_FAILED\n\r");
		goto END;
	}
#endif
#endif

#ifdef XFSBL_PERF
	XTime_GetTime(&(FsblInstancePtr->PerfTime.tFsblStart));
#endif

	/**
	 * Forcing the SD card detection signal to bypass the debouncing logic.
	 * This will ensure that SD controller doesn't end up waiting for long,
	 * fixed durations for card to be stable.
	 */
	SdCdnRegVal = XFsbl_In32(IOU_SLCR_SD_CDN_CTRL);
	XFsbl_Out32(IOU_SLCR_SD_CDN_CTRL,
			(IOU_SLCR_SD_CDN_CTRL_SD1_CDN_CTRL_MASK |
					IOU_SLCR_SD_CDN_CTRL_SD0_CDN_CTRL_MASK));

	/**
	 * DDR Check if present
	 */


	/**
	 * Poweroff the unused blocks as per PSU
	 */

END:
	return Status;
}

/*****************************************************************************/
/**
 * This function initializes the primary boot device
 *
 * @param	FsblInstancePtr is pointer to the XFsbl Instance
 *
 * @return	returns the error codes described in xfsbl_error.h on any error
 * 			returns XFSBL_SUCCESS on success
 *
 ******************************************************************************/
static u32 XFsbl_PrimaryBootDeviceInit(XFsblPs * FsblInstancePtr)
{
	u32 Status;
	u32 BootMode;

	/**
	 * Read Boot Mode register and update the value
	 */
	BootMode = XFsbl_In32(CRL_APB_BOOT_MODE_USER) &
			CRL_APB_BOOT_MODE_USER_BOOT_MODE_MASK;

	FsblInstancePtr->PrimaryBootDevice = BootMode;

	/**
	 * Enable drivers only if they are device boot modes
	 * Not required for JTAG modes
	 */
	if ( (BootMode == XFSBL_QSPI24_BOOT_MODE) ||
			(BootMode == XFSBL_QSPI32_BOOT_MODE) ||
			(BootMode == XFSBL_NAND_BOOT_MODE) ||
			(BootMode == XFSBL_SD0_BOOT_MODE) ||
			(BootMode == XFSBL_EMMC_BOOT_MODE) ||
			(BootMode == XFSBL_SD1_BOOT_MODE) ||
			(BootMode == XFSBL_SD1_LS_BOOT_MODE) ||
			(BootMode == XFSBL_USB_BOOT_MODE)) {
		/**
		 * Initialize the WDT and CSU drivers
		 */
#ifdef XFSBL_WDT_PRESENT
		/*
		 * Skip watching over APU using WDT during APU only restart
		 * as PMU will watchover APU
		 */
		if (XFSBL_MASTER_ONLY_RESET != FsblInstance.ResetReason) {
			Status = XFsbl_InitWdt();
			if (XFSBL_SUCCESS != Status) {
				XFsbl_Printf(DEBUG_GENERAL,"WDT initialization failed \n\r");
				goto END;
			}
		}
#endif

		/* Initialize CSUDMA driver */
		Status = XFsbl_CsuDmaInit(NULL);
		if (XFSBL_SUCCESS != Status) {
			goto END;
		}
	}

/**
 * If FSBL_PARTITION_LOAD_EXCLUDE macro is defined,then the partition loading
 * will be skipped and irrespective of the actual boot device,FSBL will run the way
 * it runs in JTAG boot mode
 */
#ifdef FSBL_PARTITION_LOAD_EXCLUDE
	FsblInstancePtr->PrimaryBootDevice = XFSBL_JTAG_BOOT_MODE;
	Status = XFSBL_STATUS_JTAG;
#else

	switch(BootMode)
	{
		/**
		 * For JTAG boot mode, it will be in while loop
		 */
		case XFSBL_USB_BOOT_MODE:
		{
#ifdef XFSBL_USB
			FsblInstancePtr->DeviceOps.DeviceInit = XFsbl_UsbInit;
			FsblInstancePtr->DeviceOps.DeviceCopy = XFsbl_UsbCopy;
			FsblInstancePtr->DeviceOps.DeviceRelease = XFsbl_UsbRelease;
			Status = XFSBL_SUCCESS;
#elif !(defined(XFSBL_PS_DDR))
			/**USB boot mode is not supported for DDR less systems*/
			XFsbl_Printf(DEBUG_GENERAL,
					"XFSBL_ERROR_USB_BOOT_WITH_NO_DDR\n\r");
			Status = XFSBL_ERROR_USB_BOOT_WITH_NO_DDR;
#else
			/**
			 * This bootmode is not supported in this release
			 */
			XFsbl_Printf(DEBUG_GENERAL,
				"XFSBL_ERROR_UNSUPPORTED_BOOT_MODE\n\r");
			Status = XFSBL_ERROR_UNSUPPORTED_BOOT_MODE;

#endif
		}
		break;
		case XFSBL_JTAG_BOOT_MODE:
		{
			XFsbl_Printf(DEBUG_GENERAL,"In JTAG Boot Mode \n\r");
			Status = XFSBL_STATUS_JTAG;
		}
		break;

		case XFSBL_QSPI24_BOOT_MODE:
		{
			XFsbl_Printf(DEBUG_GENERAL,"QSPI 24bit Boot Mode \n\r");
#ifdef XFSBL_QSPI
			/**
			 * Update the deviceops structure with necessary values
			 */
			FsblInstancePtr->DeviceOps.DeviceInit = XFsbl_Qspi24Init;
			FsblInstancePtr->DeviceOps.DeviceCopy = XFsbl_Qspi24Copy;
			FsblInstancePtr->DeviceOps.DeviceRelease = XFsbl_Qspi24Release;
			Status = XFSBL_SUCCESS;
#else
			/**
			 * This bootmode is not supported in this release
			 */
			XFsbl_Printf(DEBUG_GENERAL,
				"XFSBL_ERROR_UNSUPPORTED_BOOT_MODE\n\r");
			Status = XFSBL_ERROR_UNSUPPORTED_BOOT_MODE;
#endif
		}
		break;

		case XFSBL_QSPI32_BOOT_MODE:
		{
			XFsbl_Printf(DEBUG_GENERAL,"QSPI 32 bit Boot Mode \n\r");
#ifdef XFSBL_QSPI
			/**
			 * Update the deviceops structure with necessary values
			 *
			 */
            FsblInstancePtr->DeviceOps.DeviceInit = XFsbl_Qspi32Init;
			FsblInstancePtr->DeviceOps.DeviceCopy = XFsbl_Qspi32Copy;
			FsblInstancePtr->DeviceOps.DeviceRelease = XFsbl_Qspi32Release;
			Status = XFSBL_SUCCESS;
#else
			/**
			 * This bootmode is not supported in this release
			 */
			XFsbl_Printf(DEBUG_GENERAL,
					"XFSBL_ERROR_UNSUPPORTED_BOOT_MODE\n\r");
			Status = XFSBL_ERROR_UNSUPPORTED_BOOT_MODE;
#endif
        }
        break;

		case XFSBL_NAND_BOOT_MODE:
		{
			XFsbl_Printf(DEBUG_GENERAL,"NAND Boot Mode \n\r");
#ifdef XFSBL_NAND
			/**
			 * Update the deviceops structure with necessary values
			 *
			 */
			FsblInstancePtr->DeviceOps.DeviceInit = XFsbl_NandInit;
			FsblInstancePtr->DeviceOps.DeviceCopy = XFsbl_NandCopy;
			FsblInstancePtr->DeviceOps.DeviceRelease =
							XFsbl_NandRelease;
			Status = XFSBL_SUCCESS;
#else
			/**
			 * This bootmode is not supported in this release
			 */
			XFsbl_Printf(DEBUG_GENERAL,
				"XFSBL_ERROR_UNSUPPORTED_BOOT_MODE\n\r");
			Status = XFSBL_ERROR_UNSUPPORTED_BOOT_MODE;
#endif
		} break;

		case XFSBL_SD0_BOOT_MODE:
		case XFSBL_EMMC_BOOT_MODE:
		{
			if (BootMode == XFSBL_SD0_BOOT_MODE) {
				XFsbl_Printf(DEBUG_GENERAL,"SD0 Boot Mode \n\r");
			}
			else {
				XFsbl_Printf(DEBUG_GENERAL,"eMMC Boot Mode \n\r");
			}
#ifdef XFSBL_SD_0
			/**
			 * Update the deviceops structure with necessary values
			 */
			FsblInstancePtr->DeviceOps.DeviceInit = XFsbl_SdInit;
			FsblInstancePtr->DeviceOps.DeviceCopy = XFsbl_SdCopy;
			FsblInstancePtr->DeviceOps.DeviceRelease = XFsbl_SdRelease;
			Status = XFSBL_SUCCESS;
#else
			/**
			 * This bootmode is not supported in this release
			 */
			XFsbl_Printf(DEBUG_GENERAL,
				"XFSBL_ERROR_UNSUPPORTED_BOOT_MODE\n\r");
			Status = XFSBL_ERROR_UNSUPPORTED_BOOT_MODE;
#endif
		} break;

		case XFSBL_SD1_BOOT_MODE:
		case XFSBL_SD1_LS_BOOT_MODE:
		{
			if (BootMode == XFSBL_SD1_BOOT_MODE) {
				XFsbl_Printf(DEBUG_GENERAL, "SD1 Boot Mode \n\r");
			}
			else {
				XFsbl_Printf(DEBUG_GENERAL,
						"SD1 with level shifter Boot Mode \n\r");
			}
#ifdef XFSBL_SD_1
			/**
			 * Update the deviceops structure with necessary values
			 */
			FsblInstancePtr->DeviceOps.DeviceInit = XFsbl_SdInit;
			FsblInstancePtr->DeviceOps.DeviceCopy = XFsbl_SdCopy;
			FsblInstancePtr->DeviceOps.DeviceRelease = XFsbl_SdRelease;
			Status = XFSBL_SUCCESS;
#else
			/**
			 * This bootmode is not supported in this release
			 */
			XFsbl_Printf(DEBUG_GENERAL,
				"XFSBL_ERROR_UNSUPPORTED_BOOT_MODE\n\r");
			Status = XFSBL_ERROR_UNSUPPORTED_BOOT_MODE;
#endif
		} break;

		default:
		{
			XFsbl_Printf(DEBUG_GENERAL,
					"XFSBL_ERROR_UNSUPPORTED_BOOT_MODE\n\r");
			Status = XFSBL_ERROR_UNSUPPORTED_BOOT_MODE;
		} break;

	}
#endif
	/**
	 * In case of error or Jtag boot, goto end
	 */
	if (XFSBL_SUCCESS != Status) {
		goto END;
	}

	/**
	 * Initialize the Device Driver
	 */
	Status = FsblInstancePtr->DeviceOps.DeviceInit(BootMode);
	if (XFSBL_SUCCESS != Status) {
		goto END;
	}

#ifdef XFSBL_PERF
	if (BootMode == XFSBL_QSPI24_BOOT_MODE || BootMode == XFSBL_QSPI32_BOOT_MODE)
	{
#if defined (XPAR_XQSPIPSU_0_QSPI_CLK_FREQ_HZ)
		XFsbl_Printf(DEBUG_PRINT_ALWAYS, "Qspi, Freq: %0d Hz\r\n"
				, XPAR_XQSPIPSU_0_QSPI_CLK_FREQ_HZ);
#endif
	}
	else if (BootMode == XFSBL_NAND_BOOT_MODE)
	{
#if defined (XPAR_XNANDPSU_0_NAND_CLK_FREQ_HZ)
		XFsbl_Printf(DEBUG_PRINT_ALWAYS, "Nand, Freq: %0d Hz\r\n"
						, XPAR_XNANDPSU_0_NAND_CLK_FREQ_HZ);
#endif
	}
	else if (BootMode == XFSBL_SD0_BOOT_MODE || BootMode == XFSBL_SD1_BOOT_MODE
		|| BootMode == XFSBL_SD1_LS_BOOT_MODE || BootMode == XFSBL_EMMC_BOOT_MODE)
	{
#if defined (XPAR_XSDPS_0_SDIO_CLK_FREQ_HZ) && defined (XPAR_XSDPS_1_SDIO_CLK_FREQ_HZ)
		XFsbl_Printf(DEBUG_PRINT_ALWAYS, "SD0/eMMC, Freq: %0d Hz \r\n"
						, XPAR_XSDPS_0_SDIO_CLK_FREQ_HZ);
		XFsbl_Printf(DEBUG_PRINT_ALWAYS, "SD1, Freq: %0d Hz \r\n"
						, XPAR_XSDPS_1_SDIO_CLK_FREQ_HZ);
#elif defined (XPAR_XSDPS_0_SDIO_CLK_FREQ_HZ) && !defined (XPAR_XSDPS_1_SDIO_CLK_FREQ_HZ)
		XFsbl_Printf(DEBUG_PRINT_ALWAYS, "SD/eMMC, Freq: %0d Hz \r\n"
						, XPAR_XSDPS_0_SDIO_CLK_FREQ_HZ);

#endif
	}
#endif

END:
	return Status;
}

/*****************************************************************************/
/**
 * This function validates the image header
 *
 * @param	FsblInstancePtr is pointer to the XFsbl Instance
 *
 * @return	returns the error codes described in xfsbl_error.h on any error
 * 			returns XFSBL_SUCCESS on success
 *
 ******************************************************************************/
static u32 XFsbl_ValidateHeader(XFsblPs * FsblInstancePtr)
{
	u32 Status;
	u32 MultiBootOffset;
	u32 BootHdrAttrb=0U;
	u32 FlashImageOffsetAddress;
	u32 EfuseCtrl;
	u32 ImageHeaderTableAddressOffset=0U;
	u32 FsblEncSts = 0U;
#ifdef XFSBL_SECURE
	u32 Size;
	u32 AcOffset=0U;
#endif
	/**
	 * Read the Multiboot Register
	 */
	MultiBootOffset = XFsbl_In32(CSU_CSU_MULTI_BOOT);
	XFsbl_Printf(DEBUG_INFO,"Multiboot Reg : 0x%0lx \n\r", MultiBootOffset);

	/**
	 *  Calculate the Flash Offset Address
	 *  For file system based devices, Flash Offset Address should be 0 always
	 */
	if (FsblInstancePtr->SecondaryBootDevice == 0U) {
		if (!((FsblInstancePtr->PrimaryBootDevice == XFSBL_SD0_BOOT_MODE)
				|| (FsblInstancePtr->PrimaryBootDevice == XFSBL_EMMC_BOOT_MODE)
				|| (FsblInstancePtr->PrimaryBootDevice == XFSBL_SD1_BOOT_MODE)
				|| (FsblInstancePtr->PrimaryBootDevice == XFSBL_SD1_LS_BOOT_MODE)
				|| (FsblInstancePtr->PrimaryBootDevice == XFSBL_USB_BOOT_MODE))) {
			FsblInstancePtr->ImageOffsetAddress = MultiBootOffset
					* XFSBL_IMAGE_SEARCH_OFFSET;
		}
	}
	else
	{
		if (!((FsblInstancePtr->SecondaryBootDevice == XFSBL_SD0_BOOT_MODE)
				|| (FsblInstancePtr->SecondaryBootDevice == XFSBL_EMMC_BOOT_MODE)
				|| (FsblInstancePtr->SecondaryBootDevice == XFSBL_SD1_BOOT_MODE)
				|| (FsblInstancePtr->SecondaryBootDevice == XFSBL_SD1_LS_BOOT_MODE)
				|| (FsblInstancePtr->SecondaryBootDevice == XFSBL_USB_BOOT_MODE))) {
			FsblInstancePtr->ImageOffsetAddress = MultiBootOffset
					* XFSBL_IMAGE_SEARCH_OFFSET;
		}
	}

	FlashImageOffsetAddress = FsblInstancePtr->ImageOffsetAddress;

	/* Copy boot header to internal memory */
	Status = FsblInstancePtr->DeviceOps.DeviceCopy(FlashImageOffsetAddress,
	                   (PTRSIZE )ReadBuffer, XIH_BH_MAX_SIZE);
	if (XFSBL_SUCCESS != Status) {
			XFsbl_Printf(DEBUG_GENERAL,"Device Copy Failed \n\r");
			goto END;
	}
#ifdef XFSBL_SECURE
	/* copy IV to local variable */
	XFsbl_MemCpy(Iv, ReadBuffer + XIH_BH_IV_OFFSET, XIH_BH_IV_LENGTH);
#endif
	/**
	 * Read Boot Image attributes
	 */
	BootHdrAttrb = Xil_In32((UINTPTR)ReadBuffer +
					XIH_BH_IMAGE_ATTRB_OFFSET);
	FsblInstancePtr->BootHdrAttributes = BootHdrAttrb;

	/*
	 * Update PMU Global general storage register5 bit 3 with FSBL encryption
	 * status if either FSBL encryption status in boot header is true or
	 * ENC_ONLY eFuse bit is programmed.
	 *
	 * FSBL encryption information in boot header:
	 * If authenticate only bits 5:4 are set, boot image is only RSA signed
	 * though encryption status in BH is non-zero.
	 * Boot image is decrypted only when BH encryption status is not 0x0 and
	 * authenticate only bits value is other than 0x3
	 */
	if (((Xil_In32((UINTPTR)ReadBuffer + XIH_BH_ENC_STS_OFFSET) != 0x0U) &&
			((BootHdrAttrb & XIH_BH_IMAGE_ATTRB_AUTH_ONLY_MASK) !=
					XIH_BH_IMAGE_ATTRB_AUTH_ONLY_MASK)) ||
			((XFsbl_In32(EFUSE_SEC_CTRL) & EFUSE_SEC_CTRL_ENC_ONLY_MASK) !=
					0x0U)) {
		FsblEncSts = XFsbl_In32(PMU_GLOBAL_GLOB_GEN_STORAGE5) |
				XFSBL_FSBL_ENCRYPTED_MASK;
		XFsbl_Out32(PMU_GLOBAL_GLOB_GEN_STORAGE5, FsblEncSts);
	}

	/**
	 * Read the Image Header Table offset from
	 * Boot Header
	 */
	ImageHeaderTableAddressOffset = Xil_In32((UINTPTR)ReadBuffer +
					XIH_BH_IH_TABLE_OFFSET);

	XFsbl_Printf(DEBUG_INFO,"Image Header Table Offset 0x%0lx \n\r",
			ImageHeaderTableAddressOffset);

	/**
	 * Read Efuse bit and check Boot Header for Authentication
	 */
	EfuseCtrl = XFsbl_In32(EFUSE_SEC_CTRL);

	if (((EfuseCtrl & EFUSE_SEC_CTRL_RSA_EN_MASK) != 0x00)
		&& ((BootHdrAttrb & XIH_BH_IMAGE_ATTRB_RSA_MASK) ==
					XIH_BH_IMAGE_ATTRB_RSA_MASK)) {
		Status = XFSBL_ERROR_BH_AUTH_IS_NOTALLOWED;
		XFsbl_Printf(DEBUG_GENERAL,"XFSBL_ERROR_BH_AUTH_IS_NOTALLOWED"
					" when eFSUE RSA bit is set \n\r");
		goto END;
	}

	/* If authentication is enabled */
	if (((EfuseCtrl & EFUSE_SEC_CTRL_RSA_EN_MASK) != 0U) ||
	    ((BootHdrAttrb & XIH_BH_IMAGE_ATTRB_RSA_MASK)
		== XIH_BH_IMAGE_ATTRB_RSA_MASK)) {
		FsblInstancePtr->AuthEnabled = TRUE;
		XFsbl_Printf(DEBUG_INFO,"Authentication Enabled\r\n");
#ifdef XFSBL_SECURE
		 /* Read AC offset from Image header table */
		Status = FsblInstancePtr->DeviceOps.DeviceCopy(FlashImageOffsetAddress
		            + ImageHeaderTableAddressOffset + XIH_IHT_AC_OFFSET,
			   (PTRSIZE ) &AcOffset, XIH_FIELD_LEN);
		if (XFSBL_SUCCESS != Status) {
			XFsbl_Printf(DEBUG_GENERAL,"Device Copy Failed \n\r");
			goto END;
		}
		if (AcOffset != 0x00U) {
			/* Authentication exists copy AC to OCM */
			Status = FsblInstancePtr->DeviceOps.DeviceCopy(
				(FsblInstancePtr->ImageOffsetAddress +
				(AcOffset * XIH_PARTITION_WORD_LENGTH)),
				(INTPTR)AuthBuffer, XFSBL_AUTH_CERT_MIN_SIZE);
			if (XFSBL_SUCCESS != Status) {
				goto END;
			}

			/* Authenticate boot header */
			/* When eFUSE RSA enable bit is blown */
			if ((EfuseCtrl & EFUSE_SEC_CTRL_RSA_EN_MASK) != 0U) {
				Status = XFsbl_BhAuthentication(FsblInstancePtr,
						ReadBuffer, (PTRSIZE)AuthBuffer, TRUE);
			}
			/* When eFUSE RSA bit is not blown */
			else {
				Status = XFsbl_BhAuthentication(FsblInstancePtr,
						ReadBuffer, (PTRSIZE)AuthBuffer, FALSE);
			}
			if (Status != XST_SUCCESS) {
			    XFsbl_Printf(DEBUG_GENERAL,
					"Failure at boot header authentication\r\n");
				goto END;
			}


			/* Authenticate Image header table */
			/*
			 * Total size of Image header may vary
			 * depending on padding so
			 * size = AC address - Start address;
			 */
			Size = (AcOffset * XIH_PARTITION_WORD_LENGTH) -
				(ImageHeaderTableAddressOffset);
			if(Size > sizeof(ReadBuffer))
			{
				Status = XFSBL_ERROR_IMAGE_HEADER_SIZE;
				goto END;
			}

			/* Copy the Image header to OCM */
			Status = FsblInstancePtr->DeviceOps.DeviceCopy(
					FsblInstancePtr->ImageOffsetAddress +
					ImageHeaderTableAddressOffset,
					(INTPTR)ImageHdr, Size);
			if (Status != XFSBL_SUCCESS) {
				goto END;
			}

			/* Authenticate the image header */
			Status = XFsbl_Authentication(FsblInstancePtr,
					(PTRSIZE)ImageHdr,
					Size + XFSBL_AUTH_CERT_MIN_SIZE,
					(PTRSIZE)(AuthBuffer), 0x00U);
			if (Status != XFSBL_SUCCESS) {
				XFsbl_Printf(DEBUG_GENERAL,
					"Failure at image header"
					" table authentication\r\n");
				goto END;

			}
			/*
			 * As authentication is success
			 * verify ACoffset used for authentication
			 */
			if (AcOffset !=
			 Xil_In32((UINTPTR)ImageHdr + XIH_IHT_AC_OFFSET)) {
				Status = XFSBL_ERROR_IMAGE_HEADER_ACOFFSET;
				XFsbl_Printf(DEBUG_GENERAL,
					"Wrong Authentication "
					"certificate offset\r\n");
				goto END;
			}
		}
		else {
			XFsbl_Printf(DEBUG_GENERAL,
				"XFSBL_ERROR_IMAGE_HEADER_ACOFFSET\r\n");
			Status = XFSBL_ERROR_IMAGE_HEADER_ACOFFSET;
			goto END;
		}
		/*
		 * Read Image Header and validate Image Header Table
		 * Here we need to use memcpy to copy partition headers
		 * from OCM which we already copied.
		 */
		Status = XFsbl_ReadImageHeader(&FsblInstancePtr->ImageHeader,
				NULL, (UINTPTR)ImageHdr,
				FsblInstancePtr->ProcessorID,
				ImageHeaderTableAddressOffset);
		if (XFSBL_SUCCESS != Status) {
			goto END;
		}
#else
                XFsbl_Printf(DEBUG_GENERAL,
			"XFSBL_ERROR_SECURE_NOT_ENABLED\r\n");
                Status = XFSBL_ERROR_SECURE_NOT_ENABLED;
                goto END;
#endif
	}
	else {
		/* Read Image Header and validate Image Header Table */
		Status = XFsbl_ReadImageHeader(&FsblInstancePtr->ImageHeader,
						&FsblInstancePtr->DeviceOps,
						FlashImageOffsetAddress,
						FsblInstancePtr->ProcessorID,
						ImageHeaderTableAddressOffset);
		if (XFSBL_SUCCESS != Status) {
			goto END;
		}
	}
END:
	return Status;
}

/*****************************************************************************/
/**
 * This function initializes secondary boot device
 *
 * @param	FsblInstancePtr is pointer to the XFsbl Instance
 *
 * @return	returns the error codes described in xfsbl_error.h on any error
 * 			returns XFSBL_SUCCESS on success
 *
 ******************************************************************************/
static u32 XFsbl_SecondaryBootDeviceInit(XFsblPs * FsblInstancePtr)
{
	u32 Status = XFSBL_SUCCESS;
	u32 SecBootMode;

	/**
	 * Update the deviceops structure
	 */

	switch (FsblInstancePtr->SecondaryBootDevice) {
	case XIH_IHT_PPD_SAME: {
		goto END;
	}
		break;
	case XIH_IHT_PPD_USB: {
#ifdef XFSBL_USB
		FsblInstancePtr->DeviceOps.DeviceInit = XFsbl_UsbInit;
		FsblInstancePtr->DeviceOps.DeviceCopy = XFsbl_UsbCopy;
		FsblInstancePtr->DeviceOps.DeviceRelease = XFsbl_UsbRelease;
		SecBootMode = XFSBL_USB_BOOT_MODE;
		Status = XFSBL_SUCCESS;
#elif !(defined(XFSBL_PS_DDR))
		/**USB boot mode is not supported for DDR less systems*/
		XFsbl_Printf(DEBUG_GENERAL,
				"XFSBL_ERROR_USB_BOOT_WITH_NO_DDR\n\r");
		Status = XFSBL_ERROR_USB_BOOT_WITH_NO_DDR;
#else
		/**
		 * This bootmode is not supported in this release
		 */
		XFsbl_Printf(DEBUG_GENERAL, "XFSBL_ERROR_UNSUPPORTED_SEC_BOOT_MODE\n\r");
		Status = XFSBL_ERROR_UNSUPPORTED_BOOT_MODE;

#endif
	}
		break;

	case XIH_IHT_PPD_QSPI24: {
		XFsbl_Printf(DEBUG_GENERAL, "QSPI 24bit Boot Mode \n\r");
#ifdef XFSBL_QSPI
		/**
		 * Update the deviceops structure with necessary values
		 */
		FsblInstancePtr->DeviceOps.DeviceInit = XFsbl_Qspi24Init;
		FsblInstancePtr->DeviceOps.DeviceCopy = XFsbl_Qspi24Copy;
		FsblInstancePtr->DeviceOps.DeviceRelease = XFsbl_Qspi24Release;
		SecBootMode = XFSBL_QSPI24_BOOT_MODE;
		Status = XFSBL_SUCCESS;
#else
		/**
		 * This bootmode is not supported in this release
		 */
		XFsbl_Printf(DEBUG_GENERAL,
				"XFSBL_ERROR_UNSUPPORTED_SEC_BOOT_MODE\n\r");
		Status = XFSBL_ERROR_UNSUPPORTED_BOOT_MODE;
#endif
	}
		break;

	case XIH_IHT_PPD_QSPI32: {
		XFsbl_Printf(DEBUG_GENERAL, "QSPI 32bit Boot Mode \n\r");
#ifdef XFSBL_QSPI
		/**
		 * Update the deviceops structure with necessary values
		 */
		FsblInstancePtr->DeviceOps.DeviceInit = XFsbl_Qspi32Init;
		FsblInstancePtr->DeviceOps.DeviceCopy = XFsbl_Qspi32Copy;
		FsblInstancePtr->DeviceOps.DeviceRelease = XFsbl_Qspi32Release;
		SecBootMode = XFSBL_QSPI32_BOOT_MODE;
		Status = XFSBL_SUCCESS;
#else
		/**
		 * This bootmode is not supported in this release
		 */
		XFsbl_Printf(DEBUG_GENERAL,
				"XFSBL_ERROR_UNSUPPORTED_SEC_BOOT_MODE\n\r");
		Status = XFSBL_ERROR_UNSUPPORTED_BOOT_MODE;
#endif
	}
		break;

	case XIH_IHT_PPD_NAND: {
		XFsbl_Printf(DEBUG_GENERAL, "NAND Boot Mode \n\r");
#ifdef XFSBL_NAND
		/**
		 * Update the deviceops structure with necessary values
		 *
		 */
		FsblInstancePtr->DeviceOps.DeviceInit = XFsbl_NandInit;
		FsblInstancePtr->DeviceOps.DeviceCopy = XFsbl_NandCopy;
		FsblInstancePtr->DeviceOps.DeviceRelease =
		XFsbl_NandRelease;
		SecBootMode = XFSBL_NAND_BOOT_MODE;
		Status = XFSBL_SUCCESS;
#else
		/**
		 * This bootmode is not supported in this release
		 */
		XFsbl_Printf(DEBUG_GENERAL, "XFSBL_ERROR_UNSUPPORTED_SEC_BOOT_MODE\n\r");
		Status = XFSBL_ERROR_UNSUPPORTED_BOOT_MODE;
#endif
	}
		break;

	case XIH_IHT_PPD_SD_0: {
#ifdef XFSBL_SD_0
		/**
		 * Update the deviceops structure with necessary values
		 */
		FsblInstancePtr->DeviceOps.DeviceInit = XFsbl_SdInit;
		FsblInstancePtr->DeviceOps.DeviceCopy = XFsbl_SdCopy;
		FsblInstancePtr->DeviceOps.DeviceRelease = XFsbl_SdRelease;
		SecBootMode = XFSBL_SD0_BOOT_MODE;
		Status = XFSBL_SUCCESS;
#else
		/**
		 * This bootmode is not supported in this release
		 */
		XFsbl_Printf(DEBUG_GENERAL, "XFSBL_ERROR_UNSUPPORTED_SEC_BOOT_MODE\n\r");
		Status = XFSBL_ERROR_UNSUPPORTED_BOOT_MODE;
#endif
	}
		break;
	case XIH_IHT_PPD_MMC: {

#ifdef XFSBL_SD_0
		/**
		 * Update the deviceops structure with necessary values
		 */
		FsblInstancePtr->DeviceOps.DeviceInit = XFsbl_SdInit;
		FsblInstancePtr->DeviceOps.DeviceCopy = XFsbl_SdCopy;
		FsblInstancePtr->DeviceOps.DeviceRelease = XFsbl_SdRelease;
		SecBootMode = XFSBL_EMMC_BOOT_MODE;
		Status = XFSBL_SUCCESS;
#else
		/**
		 * This bootmode is not supported in this release
		 */
		XFsbl_Printf(DEBUG_GENERAL, "XFSBL_ERROR_UNSUPPORTED_SEC_BOOT_MODE\n\r");
		Status = XFSBL_ERROR_UNSUPPORTED_BOOT_MODE;
#endif
	}
		break;

	case XIH_IHT_PPD_SD_1: {

#ifdef XFSBL_SD_1
		/**
		 * Update the deviceops structure with necessary values
		 */
		FsblInstancePtr->DeviceOps.DeviceInit = XFsbl_SdInit;
		FsblInstancePtr->DeviceOps.DeviceCopy = XFsbl_SdCopy;
		FsblInstancePtr->DeviceOps.DeviceRelease = XFsbl_SdRelease;
		SecBootMode = XFSBL_SD1_BOOT_MODE;
		Status = XFSBL_SUCCESS;
#else
		/**
		 * This bootmode is not supported in this release
		 */
		XFsbl_Printf(DEBUG_GENERAL,
				"XFSBL_ERROR_UNSUPPORTED_SEC_BOOT_MODE\n\r");
		Status = XFSBL_ERROR_UNSUPPORTED_BOOT_MODE;
#endif

	}
		break;

	case XIH_IHT_PPD_SD_LS: {

#ifdef XFSBL_SD_1
		/**
		 * Update the deviceops structure with necessary values
		 */
		FsblInstancePtr->DeviceOps.DeviceInit = XFsbl_SdInit;
		FsblInstancePtr->DeviceOps.DeviceCopy = XFsbl_SdCopy;
		FsblInstancePtr->DeviceOps.DeviceRelease = XFsbl_SdRelease;
		SecBootMode = XFSBL_SD1_LS_BOOT_MODE;
		Status = XFSBL_SUCCESS;
#else
		/**
		 * This bootmode is not supported in this release
		 */
		XFsbl_Printf(DEBUG_GENERAL,
				"XFSBL_ERROR_UNSUPPORTED_SEC_BOOT_MODE\n\r");
		Status = XFSBL_ERROR_UNSUPPORTED_BOOT_MODE;
#endif
	}
		break;

	default: {
		XFsbl_Printf(DEBUG_GENERAL, "XFSBL_ERROR_UNSUPPORTED_SEC_BOOT_MODE\n\r");
		Status = XFSBL_ERROR_UNSUPPORTED_BOOT_MODE;
	}
		break;

	}

	/**
	 * Initialize the Secondary Boot Device Driver
	 */
	if (Status == XFSBL_SUCCESS) {
		Status = FsblInstancePtr->DeviceOps.DeviceInit(SecBootMode);
		if(Status == XFSBL_SUCCESS) {
			FsblInstancePtr->SecondaryBootDevice = SecBootMode;
			Status = XFsbl_ValidateHeader(FsblInstancePtr);
		}
	}

END:
	return Status;
}

/*****************************************************************************/
/**
 * This function does ECC Initialization of memory
 *
 * @param	DestAddr is start address from where to calculate ECC
 * @param	LengthBytes is length in bytes from start address to calculate ECC
 *
 * @return
 * 		- XFSBL_SUCCESS for successful ECC Initialization
 * 		- errors as mentioned in xfsbl_error.h
 *
 *****************************************************************************/
static u32 XFsbl_EccInit(u64 DestAddr, u64 LengthBytes)
{
	u32 RegVal;
	u32 Status;
	u32 Length;
	u64 StartAddr = DestAddr;
	u64 NumBytes = LengthBytes;

	Xil_DCacheDisable();

	while (NumBytes > 0U) {
		if (NumBytes > ZDMA_TRANSFER_MAX_LEN) {
			Length = ZDMA_TRANSFER_MAX_LEN;
		} else {
			Length = (u32)NumBytes;
		}

		/* Wait until the DMA is in idle state */
		do {
			RegVal = XFsbl_In32(ADMA_CH0_ZDMA_CH_STATUS);
			RegVal &= ADMA_CH0_ZDMA_CH_STATUS_STATE_MASK;
		} while ((RegVal != ADMA_CH0_ZDMA_CH_STATUS_STATE_DONE) &&
				(RegVal != ADMA_CH0_ZDMA_CH_STATUS_STATE_ERR));

		/* Enable Simple (Write Only) Mode */
		RegVal = XFsbl_In32(ADMA_CH0_ZDMA_CH_CTRL0);
		RegVal &= (ADMA_CH0_ZDMA_CH_CTRL0_POINT_TYPE_MASK |
				ADMA_CH0_ZDMA_CH_CTRL0_MODE_MASK);
		RegVal |= (ADMA_CH0_ZDMA_CH_CTRL0_POINT_TYPE_NORMAL |
				ADMA_CH0_ZDMA_CH_CTRL0_MODE_WR_ONLY);
		XFsbl_Out32(ADMA_CH0_ZDMA_CH_CTRL0, RegVal);

		/* Fill in the data to be written */
		XFsbl_Out32(ADMA_CH0_ZDMA_CH_WR_ONLY_WORD0, XFSBL_ECC_INIT_VAL_WORD);
		XFsbl_Out32(ADMA_CH0_ZDMA_CH_WR_ONLY_WORD1, XFSBL_ECC_INIT_VAL_WORD);
		XFsbl_Out32(ADMA_CH0_ZDMA_CH_WR_ONLY_WORD2, XFSBL_ECC_INIT_VAL_WORD);
		XFsbl_Out32(ADMA_CH0_ZDMA_CH_WR_ONLY_WORD3, XFSBL_ECC_INIT_VAL_WORD);

		/* Write Destination Address */
		XFsbl_Out32(ADMA_CH0_ZDMA_CH_DST_DSCR_WORD0,
				(u32)(StartAddr & ADMA_CH0_ZDMA_CH_DST_DSCR_WORD0_LSB_MASK));
		XFsbl_Out32(ADMA_CH0_ZDMA_CH_DST_DSCR_WORD1,
				(u32)((StartAddr >> 32U) &
						ADMA_CH0_ZDMA_CH_DST_DSCR_WORD1_MSB_MASK));

		/* Size to be Transferred. Recommended to set both src and dest sizes */
		XFsbl_Out32(ADMA_CH0_ZDMA_CH_SRC_DSCR_WORD2, Length);
		XFsbl_Out32(ADMA_CH0_ZDMA_CH_DST_DSCR_WORD2, Length);

		/* DMA Enable */
		RegVal = XFsbl_In32(ADMA_CH0_ZDMA_CH_CTRL2);
		RegVal |= ADMA_CH0_ZDMA_CH_CTRL2_EN_MASK;
		XFsbl_Out32(ADMA_CH0_ZDMA_CH_CTRL2, RegVal);

		/* Check the status of the transfer by polling on DMA Done */
		do {
			RegVal = XFsbl_In32(ADMA_CH0_ZDMA_CH_ISR);
			RegVal &= ADMA_CH0_ZDMA_CH_ISR_DMA_DONE_MASK;
		} while (RegVal != ADMA_CH0_ZDMA_CH_ISR_DMA_DONE_MASK);

		/* Clear DMA status */
		RegVal = XFsbl_In32(ADMA_CH0_ZDMA_CH_ISR);
		RegVal |= ADMA_CH0_ZDMA_CH_ISR_DMA_DONE_MASK;
		XFsbl_Out32(ADMA_CH0_ZDMA_CH_ISR, ADMA_CH0_ZDMA_CH_ISR_DMA_DONE_MASK);

		/* Read the channel status for errors */
		RegVal = XFsbl_In32(ADMA_CH0_ZDMA_CH_STATUS);
		if (RegVal == ADMA_CH0_ZDMA_CH_STATUS_STATE_ERR) {
			Status = XFSBL_FAILURE;
			Xil_DCacheEnable();
			goto END;
		}

		NumBytes -= Length;
		StartAddr += Length;
	}

	Xil_DCacheEnable();

	/* Restore reset values for the DMA registers used */
	XFsbl_Out32(ADMA_CH0_ZDMA_CH_CTRL0, 0x00000080U);
	XFsbl_Out32(ADMA_CH0_ZDMA_CH_WR_ONLY_WORD0, 0x00000000U);
	XFsbl_Out32(ADMA_CH0_ZDMA_CH_WR_ONLY_WORD1, 0x00000000U);
	XFsbl_Out32(ADMA_CH0_ZDMA_CH_WR_ONLY_WORD2, 0x00000000U);
	XFsbl_Out32(ADMA_CH0_ZDMA_CH_WR_ONLY_WORD3, 0x00000000U);
	XFsbl_Out32(ADMA_CH0_ZDMA_CH_DST_DSCR_WORD0, 0x00000000U);
	XFsbl_Out32(ADMA_CH0_ZDMA_CH_DST_DSCR_WORD1, 0x00000000U);
	XFsbl_Out32(ADMA_CH0_ZDMA_CH_SRC_DSCR_WORD2, 0x00000000U);
	XFsbl_Out32(ADMA_CH0_ZDMA_CH_DST_DSCR_WORD2, 0x00000000U);
	XFsbl_Out32(ADMA_CH0_ZDMA_CH_CTRL0_TOTAL_BYTE_COUNT,0x00000000U);

	XFsbl_Printf(DEBUG_INFO,
		"Address 0x%0x%08x, Length 0x%0x%08x, ECC initialized \r\n",
		(u32)(DestAddr >> 32U), (u32)DestAddr,
		(u32)(LengthBytes >> 32U), (u32)LengthBytes);

	Status = XFSBL_SUCCESS;
END:
	return Status;
}

/*****************************************************************************/
/**
 * This function does ECC Initialization of DDR memory
 *
 * @param none
 *
 * @return
 * 		- XFSBL_SUCCESS for successful ECC Initialization
 * 		-               or ECC is not enabled for DDR
 * 		- errors as mentioned in xfsbl_error.h
 *
 *****************************************************************************/
static u32 XFsbl_DdrEccInit(void)
{
	u32 Status;
#if XPAR_PSU_DDRC_0_HAS_ECC
	u64 LengthBytes =
			(XFSBL_PS_DDR_END_ADDRESS - XFSBL_PS_DDR_INIT_START_ADDRESS) + 1;
	u64 DestAddr = XFSBL_PS_DDR_INIT_START_ADDRESS;

	XFsbl_Printf(DEBUG_GENERAL,"Initializing DDR ECC\n\r");

	Status = XFsbl_EccInit(DestAddr, LengthBytes);
	if (XFSBL_SUCCESS != Status) {
		Status = XFSBL_ERROR_DDR_ECC_INIT;
		XFsbl_Printf(DEBUG_GENERAL,"XFSBL_ERROR_DDR_ECC_INIT\n\r");
		goto END;
	}

	/* If there is upper PS DDR, initialize its ECC */
#ifdef XFSBL_PS_HI_DDR_START_ADDRESS
	LengthBytes =
		(XFSBL_PS_HI_DDR_END_ADDRESS - XFSBL_PS_HI_DDR_START_ADDRESS) + 1;
	DestAddr = XFSBL_PS_HI_DDR_START_ADDRESS;

	Status = XFsbl_EccInit(DestAddr, LengthBytes);
	if (XFSBL_SUCCESS != Status) {
		Status = XFSBL_ERROR_DDR_ECC_INIT;
		XFsbl_Printf(DEBUG_GENERAL,"XFSBL_ERROR_DDR_ECC_INIT\n\r");
		goto END;
	}
#endif
END:
#else
	Status = XFSBL_SUCCESS;
#endif
	return Status;
}

/*****************************************************************************/
/**
 * This function does ECC Initialization of TCM memory
 *
 * @param FsblInstancePtr is pointer to the XFsbl Instance
 * @param CpuId One of R5-0, R5-1, R5-LS, A53-0
 *
 * @return
 * 		- XFSBL_SUCCESS for successful ECC Initialization
 * 		-               or if ECC Initialization is not enabled
 * 		- errors as mentioned in xfsbl_error.h
 *
 *****************************************************************************/
u32 XFsbl_TcmEccInit(XFsblPs * FsblInstancePtr, u32 CpuId)
{
	u32 Status;
	u32 LengthBytes;
	u32 ATcmAddr;
	u32 BTcmAddr;
	u32 EccInitStatus;
	u8 FlagReduceAtcmLength = FALSE;

	XFsbl_Printf(DEBUG_GENERAL,"Initializing TCM ECC\n\r");

	/**
	 * If for A53, TCM ECC need to be initialized, do it for all banks
	 * of TCM.for R5-L,R5-0 processor don't initialize initial 32 bytes of TCM.
	 * For R5-0,R5-1 initialize corresponding banks of TCM.*/

	 /**
	  * For R5-L,R5-0 don't initialize initial 32 bytes of TCM,
	  * because initial 32 bytes are holding R5 vectors.
	  */

	if(CpuId == XIH_PH_ATTRB_DEST_CPU_A53_0) {

		ATcmAddr = XFSBL_R50_HIGH_ATCM_START_ADDRESS;
		LengthBytes = XFSBL_R5_TCM_BANK_LENGTH * 4U;
		Status = XFsbl_EccInit(ATcmAddr, LengthBytes);
		EccInitStatus = XFSBL_R50_TCM_ECC_INIT_STATUS |
				XFSBL_R51_TCM_ECC_INIT_STATUS;

	}
	else if (CpuId == XIH_PH_ATTRB_DEST_CPU_R5_L) {

		if(FsblInstancePtr->ProcessorID != XIH_PH_ATTRB_DEST_CPU_A53_0) {
			ATcmAddr = XFSBL_R50_HIGH_ATCM_START_ADDRESS + 32U; /* Not to overwrite R5 vectors */
			LengthBytes = (XFSBL_R5_TCM_BANK_LENGTH * 4U) - 32U;
		} else {
			ATcmAddr = XFSBL_R50_HIGH_ATCM_START_ADDRESS;
			LengthBytes = XFSBL_R5_TCM_BANK_LENGTH * 4U;
		}
		Status = XFsbl_EccInit(ATcmAddr, LengthBytes);
		EccInitStatus = XFSBL_R50_TCM_ECC_INIT_STATUS |
				XFSBL_R51_TCM_ECC_INIT_STATUS;
	}
	else
	{
		if (CpuId == XIH_PH_ATTRB_DEST_CPU_R5_0) {

			if(FsblInstancePtr->ProcessorID != XIH_PH_ATTRB_DEST_CPU_A53_0) {
				ATcmAddr = XFSBL_R50_HIGH_ATCM_START_ADDRESS + 32U;/* Not to overwrite R5 vectors */
				BTcmAddr = XFSBL_R50_HIGH_BTCM_START_ADDRESS;
				LengthBytes = XFSBL_R5_TCM_BANK_LENGTH;
				FlagReduceAtcmLength = TRUE;
			} else {

				ATcmAddr = XFSBL_R50_HIGH_ATCM_START_ADDRESS;
				BTcmAddr = XFSBL_R50_HIGH_BTCM_START_ADDRESS;
				LengthBytes = XFSBL_R5_TCM_BANK_LENGTH;
			}
			EccInitStatus = XFSBL_R50_TCM_ECC_INIT_STATUS;

		} else if (CpuId == XIH_PH_ATTRB_DEST_CPU_R5_1) {
			ATcmAddr = XFSBL_R51_HIGH_ATCM_START_ADDRESS;
			BTcmAddr = XFSBL_R51_HIGH_BTCM_START_ADDRESS;
			EccInitStatus = XFSBL_R51_TCM_ECC_INIT_STATUS;
			LengthBytes = XFSBL_R5_TCM_BANK_LENGTH;
		}
		else {
			/* for MISRA-C */
			ATcmAddr=0U;
			BTcmAddr=0U;
			EccInitStatus=0U;
			LengthBytes = 0U;
		}

		if(FlagReduceAtcmLength == TRUE) {
			Status = XFsbl_EccInit(ATcmAddr, LengthBytes - 32U);
		} else {
			Status = XFsbl_EccInit(ATcmAddr, LengthBytes);
		}

		if (XFSBL_SUCCESS == Status) {
			Status = XFsbl_EccInit(BTcmAddr, LengthBytes);
		}
	}

	if (XFSBL_SUCCESS == Status) {
		/* Indicate in flag that TCM ECC is initialized */
		FsblInstancePtr->TcmEccInitStatus = EccInitStatus;
	}
	else {
		Status = XFSBL_ERROR_TCM_ECC_INIT;
		XFsbl_Printf(DEBUG_GENERAL,"XFSBL_ERROR_TCM_ECC_INIT\n\r");
		goto END;
	}

END:
	return Status;
}

/*****************************************************************************/
/**
 * This function adds additional steps for TCM ECC Initialization for A53.
 * These are to power-up TCM before actual ECC calculation and after it is done,
 * to keep RPU in reset
 *
 * @param FsblInstancePtr is pointer to the XFsbl Instance
 *
 * @return
 * 		- XFSBL_SUCCESS for success
 * 		- errors as mentioned in xfsbl_error.h
 *
 *****************************************************************************/
static u32 XFsbl_TcmInit(XFsblPs * FsblInstancePtr)
{
	u32 Status;
	u32 RegValue;

	if (FsblInstancePtr->ProcessorID == XIH_PH_ATTRB_DEST_CPU_A53_0) {
#ifndef XFSBL_A53_TCM_ECC
		Status = XFSBL_SUCCESS;
		goto END;
#endif
	}

	if (FsblInstancePtr->ProcessorID == XIH_PH_ATTRB_DEST_CPU_A53_0) {
		/* If TCM ECC has to be initialized for A53, power it up first */
		Status = XFsbl_PowerUpMemory(XFSBL_R5_L_TCM);
		if (Status != XFSBL_SUCCESS) {
			goto END;
		}
	}

	/* Do ECC Initialization of TCM if required */
	Status = XFsbl_TcmEccInit(FsblInstancePtr, FsblInstancePtr->ProcessorID);
	if (XFSBL_SUCCESS != Status) {
		goto END;
	}

	/* Place the RPU back in reset, to let user power it up when required */
	if (FsblInstancePtr->ProcessorID == XIH_PH_ATTRB_DEST_CPU_A53_0) {
		RegValue = XFsbl_In32(CRL_APB_RST_LPD_TOP);
		RegValue |= (CRL_APB_RST_LPD_TOP_RPU_R50_RESET_MASK |
				CRL_APB_RST_LPD_TOP_RPU_R51_RESET_MASK |
				CRL_APB_RST_LPD_TOP_RPU_AMBA_RESET_MASK);
		XFsbl_Out32(CRL_APB_RST_LPD_TOP, RegValue);
	}

END:
	return Status;
}

/*****************************************************************************/
/**
 * This function clears pending interrupts. This is called only during APU only
 *  reset.
 *
 * @param
 *
 * @return
 *
 *
 *****************************************************************************/
static void XFsbl_ClearPendingInterrupts(void)
{

	u32 InterruptClearVal = 0xFFFFFFFFU;
	/* Clear pending peripheral interrupts */


	XFsbl_Out32 (ACPU_GIC_GICD_ICENBLR0, InterruptClearVal);
	XFsbl_Out32 (ACPU_GIC_GICD_ICPENDR0, InterruptClearVal);
	XFsbl_Out32 (ACPU_GIC_GICD_ICACTIVER0, InterruptClearVal);

	XFsbl_Out32 (ACPU_GIC_GICD_ICENBLR1, InterruptClearVal);
	XFsbl_Out32 (ACPU_GIC_GICD_ICPENDR1, InterruptClearVal);
	XFsbl_Out32 (ACPU_GIC_GICD_ICACTIVER1, InterruptClearVal);

	XFsbl_Out32 (ACPU_GIC_GICD_ICENBLR2, InterruptClearVal);
	XFsbl_Out32 (ACPU_GIC_GICD_ICPENDR2, InterruptClearVal);
	XFsbl_Out32 (ACPU_GIC_GICD_ICACTIVER2, InterruptClearVal);

	XFsbl_Out32 (ACPU_GIC_GICD_ICENBLR3, InterruptClearVal);
	XFsbl_Out32 (ACPU_GIC_GICD_ICPENDR3, InterruptClearVal);
	XFsbl_Out32 (ACPU_GIC_GICD_ICACTIVER3, InterruptClearVal);

	XFsbl_Out32 (ACPU_GIC_GICD_ICENBLR4, InterruptClearVal);
	XFsbl_Out32 (ACPU_GIC_GICD_ICPENDR4, InterruptClearVal);
	XFsbl_Out32 (ACPU_GIC_GICD_ICACTIVER4, InterruptClearVal);

	XFsbl_Out32 (ACPU_GIC_GICD_ICENBLR5, InterruptClearVal);
	XFsbl_Out32 (ACPU_GIC_GICD_ICPENDR5, InterruptClearVal);
	XFsbl_Out32 (ACPU_GIC_GICD_ICACTIVER5, InterruptClearVal);


	/* Clear active software generated interrupts, if any */
	u32 RegVal = XFsbl_In32(ACPU_GIC_GICD_INTR_ACK_REG);
	XFsbl_Out32(ACPU_GIC_GICD_END_INTR_REG, RegVal);

	/* Clear pending software generated interrupts */

	XFsbl_Out32 (ACPU_GIC_GICD_CPENDSGIR0, InterruptClearVal);
	XFsbl_Out32 (ACPU_GIC_GICD_CPENDSGIR1, InterruptClearVal);
	XFsbl_Out32 (ACPU_GIC_GICD_CPENDSGIR2, InterruptClearVal);
	XFsbl_Out32 (ACPU_GIC_GICD_CPENDSGIR3, InterruptClearVal);

}

/*****************************************************************************/
/**
 * This function marks DDR region as "Reserved" or mark as "Memory".
 *
 * @param Cond is the condition to mark DDR region as Reserved or Memory. If
 *	  this parameter is TRUE it marks DDR region as Reserved and if it is
 *	  FALSE it marks DDR as Memory.
 *
 * @return
 *
 *
 *****************************************************************************/
void XFsbl_MarkDdrAsReserved(u8 Cond)
{
#if defined (XPAR_PSU_DDR_0_S_AXI_BASEADDR) && !defined (ARMR5)
	u32 Attrib = ATTRIB_MEMORY_A53_64;
	u64 BlockNum;

	if (TRUE == Cond) {
		Attrib = ATTRIB_RESERVED_A53;
	}

#ifdef ARMA53_64
	/* For A53 64bit*/
	for (BlockNum = 0; BlockNum < NUM_BLOCKS_A53_64; BlockNum++) {
		XFsbl_SetTlbAttributes(BlockNum * BLOCK_SIZE_A53_64, Attrib);
	}
#ifdef XFSBL_PS_HI_DDR_START_ADDRESS
	for (BlockNum = 0; BlockNum < NUM_BLOCKS_A53_64_HIGH; BlockNum++) {
		XFsbl_SetTlbAttributes(XFSBL_PS_HI_DDR_START_ADDRESS +
				       BlockNum * BLOCK_SIZE_A53_64_HIGH,
				       Attrib);
	}
#endif
	Xil_DCacheFlush();
#else
	if (FALSE == Cond) {
		Attrib = ATTRIB_MEMORY_A53_32;
	}
	/* For A53 32bit*/
	for (BlockNum = 0U; BlockNum < NUM_BLOCKS_A53_32; BlockNum++) {
		XFsbl_SetTlbAttributes(BlockNum * BLOCK_SIZE_A53_32, Attrib);
	}
	Xil_DCacheFlush();
#endif
#endif
}
#ifdef XFSBL_TPM

/*****************************************************************************/
/**
 * This function calculates the hash on FSBL. Data section should be untouched
 * at the time of calling this function.
 *
 * @param	PartitionHash is pointer to the instance to store hash of FSBL
 *
 * @return	XFSBL_SUCCESS on success and error code on failure
 *
 *****************************************************************************/
static u32 XFsbl_MeasureFsbl(u8* PartitionHash)
{
	u32 Status = XFSBL_FAILURE;
	u32 Size;
	u32 RegVal;
	u8 ReadBuffer[XFSBL_SHA3_BLOCK_LEN] = {0U};
	u8 PadLen;
	u8 Index;
	XCsuDma CsuDma = {0U};
	u32* HashPtr = (u32 *)PartitionHash;

#ifdef __clang__
        Size = (u32)((PTRSIZE)&Image$$BSS_SECTION$$Base - XFSBL_OCM_START_ADDR);
#else
        Size = (u32)((PTRSIZE)&__sbss_start - XFSBL_OCM_START_ADDR);
#endif

	XFsbl_Out32(CSU_DMA_RESET, CSU_DMA_RESET_RESET_MASK);
	Status = XFsbl_CsuDmaInit(&CsuDma);
	if (Status != XFSBL_SUCCESS) {
		goto END;
	}

	XFsbl_Out32(CSU_CSU_SSS_CFG, CSU_CSU_SSS_CFG_SHA_SSS_DMA_VAL);
	XFsbl_Out32(CSU_SHA_RESET, 0U);
	XFsbl_Out32(CSU_SHA_START, CSU_SHA_START_START_MSG_MASK);

	PadLen = (Size % XFSBL_SHA3_BLOCK_LEN);
	Size = Size - PadLen;
	XCsuDma_Transfer(&CsuDma, XCSUDMA_SRC_CHANNEL, XFSBL_OCM_START_ADDR,
		Size / 4U, 0U);

	/* Checking the CSU DMA done bit should be enough. */
	XCsuDma_WaitForDone(&CsuDma, XCSUDMA_SRC_CHANNEL);

	/* Acknowledge the transfer has completed */
	XCsuDma_IntrClear(&CsuDma, XCSUDMA_SRC_CHANNEL, XCSUDMA_IXR_DONE_MASK);

	memcpy((u8 *)ReadBuffer, (u8 *)(UINTPTR)(XFSBL_OCM_START_ADDR + Size),
		PadLen);
	ReadBuffer[PadLen] = XFSBL_START_KECCAK_PADDING_MASK;
	ReadBuffer[XFSBL_SHA3_BLOCK_LEN - 1U] |= XFSBL_END_KECCAK_PADDING_MASK;

	XCsuDma_Transfer(&CsuDma, XCSUDMA_SRC_CHANNEL, (PTRSIZE)&ReadBuffer[0U],
		XFSBL_SHA3_BLOCK_LEN / 4U, 1U);

	/* Checking the CSU DMA done bit should be enough. */
	XCsuDma_WaitForDone(&CsuDma, XCSUDMA_SRC_CHANNEL);

	/* Acknowledge the transfer has completed */
	XCsuDma_IntrClear(&CsuDma, XCSUDMA_SRC_CHANNEL, XCSUDMA_IXR_DONE_MASK);

	while((Xil_In32(CSU_SHA_DONE) & CSU_SHA_DONE_SHA_DONE_MASK) == 0U);

	/* Copy has from CSU_SHA_DIGEST registers to PartitionHash variable */
	for (Index = 0U; Index < XFSBL_HASH_LENGTH_IN_WORDS; Index++) {
		RegVal = XFsbl_In32(CSU_SHA_DIGEST_0 + (Index * 4U));
		HashPtr[XFSBL_HASH_LENGTH_IN_WORDS - Index - 1U] = RegVal;
	}

END:
	return Status;
}
#endif
