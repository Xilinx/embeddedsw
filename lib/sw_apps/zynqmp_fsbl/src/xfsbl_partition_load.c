/******************************************************************************
* Copyright (c) 2015 - 2021 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
*
* @file xfsbl_partition_load.c
*
* This is the file which contains partition load code for the FSBL.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- -------- -------------------------------------------------------
* 1.00  kc   10/21/13 Initial release
* 2.0   bv   12/02/16 Made compliance to MISRAC 2012 guidelines
*       bo   01/25/17 Fixed Vector regions overwritten in R5 FSBL
*       vns  03/01/17 Enhanced security of bitstream authentication
*                     Modified endianness of IV as APIs are modified in Xilsecure
*                     While loading bitstream clearing of PL is skipped
*                     when PL is already cleared at initialize.
*                     Updated destination cpu for PMUFW.
*       bv   03/20/17 Removed isolation in PS - PL AXI bus thus allowing
*                     access to BRAM in PS only reset
*       vns  04/04/17 Corrected IV location w.r.t Image offset.
* 3.0   vns  01/03/18 Modified XFsbl_PartitionValidation() API, for each
*                     partition by adding IV of LSB 8 bits with 8 bits of
*                     IV from XFsblPs_PartitionHeader structure.
*       vns  03/07/18 Iv copying is limited to only once from boot header,
*                     and is used for every partition, In authentication case
*                     we are using IV from authenticated header(copied to
*                     internal memory), using same way for non authenticated
*                     case as well.
*       mus  02/26/19 Added support for armclang compiler.
*       skd  02/02/20 Added register writes to PMU GLOBAL to indicate PL configuration
*       har  09/22/20 Removed checks for IsCheckSumEnabled with authentication
*                     and encryption
*       bsv  28/01/21 Fix build issues in case SECURE and BITSTREAM code are
*                     excluded
*       bsv  04/01/21 Added TPM support
*       bsv  04/28/21 Added support to ensure authenticated images boot as
*                     non-secure when RSA_EN is not programmed and boot header
*                     is not authenticated
*       bsv  05/03/21 Add provision to load bitstream from OCM with DDR
*                     present in design
*       bsv  05/15/21 Support to ensure authenticated images boot as
*                     non-secure when RSA_EN is not programmed and boot header
*                     is not authenticated is disabled by default
*
* </pre>
*
* @note
*
******************************************************************************/

/***************************** Include Files *********************************/
#include "xil_cache.h"
#include "xfsbl_hw.h"
#include "xfsbl_main.h"
#include "xfsbl_image_header.h"
#include "xfsbl_hooks.h"
#include "xfsbl_authentication.h"
#include "xfsbl_bs.h"
#include "psu_init.h"
#include "xfsbl_plpartition_valid.h"
#include "xfsbl_tpm.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/
#define XFSBL_IVT_LENGTH	(u32)(0x20U)
#define XFSBL_R5_HIVEC    	(u32)(0xffff0000U)
#define XFSBL_R5_LOVEC		(u32)(0x0U)
#define XFSBL_SET_R5_SCTLR_VECTOR_BIT   (u32)(1<<13)
#define XFSBL_PARTITION_IV_MASK  (0xFFU)
#ifdef XFSBL_BS
#define XFSBL_STATE_MASK	0x00FF0000U
#define XFSBL_STATE_SHIFT	16U
#define XFSBL_FIRMWARE_STATE_UNKNOWN	0U
#define XFSBL_FIRMWARE_STATE_SECURE	1U
#define XFSBL_FIRMWARE_STATE_NONSECURE	2U
#endif
#ifdef XFSBL_TPM
#define XFSBL_EL2_VAL		(4U)
#define XFSBL_EL3_VAL		(6U)
#endif

/************************** Function Prototypes ******************************/
static u32 XFsbl_PartitionHeaderValidation(XFsblPs * FsblInstancePtr,
		u32 PartitionNum);
static u32 XFsbl_PartitionCopy(XFsblPs * FsblInstancePtr, u32 PartitionNum);
static u32 XFsbl_PartitionValidation(XFsblPs * FsblInstancePtr,
		u32 PartitionNum);
static u32 XFsbl_CheckHandoffCpu (const XFsblPs * FsblInstancePtr,
		u32 DestinationCpu);
static u32 XFsbl_ConfigureMemory(XFsblPs * FsblInstancePtr, u32 RunningCpu,
		u32 DestinationCpu, u64 Address);
static u32 XFsbl_GetLoadAddress(u32 DestinationCpu, PTRSIZE * LoadAddressPtr,
		u32 Length);
static void XFsbl_CheckPmuFw(const XFsblPs * FsblInstancePtr, u32 PartitionNum);
static u32 XFsbl_CalculateCheckSum(XFsblPs * FsblInstancePtr,
	PTRSIZE LoadAddress, u32 PartitionNum, u8 * PartitionHash);
static u32 XFsbl_ValidateCheckSum(XFsblPs * FsblInstancePtr,
	PTRSIZE LoadAddress, u32 PartitionNum, u8 * PartitionHash);
static void XFsbl_CalculateSHA(const XFsblPs * FsblInstancePtr,
	PTRSIZE LoadAddress, u32 PartitionNum, u8* PartitionHash);
#ifdef XFSBL_BS
static void XFsbl_SetBSSecureState(u32 State);
#endif

#ifdef XFSBL_ENABLE_DDR_SR
static void XFsbl_PollForDDRReady(void);
#endif
#ifdef ARMR5
static void XFsbl_SetR5ExcepVectorHiVec(void);
static void XFsbl_SetR5ExcepVectorLoVec(void);
#endif
#ifdef XFSBL_TPM
static u8 XFsbl_GetPcrIndex(const XFsblPs * FsblInstancePtr, u32 PartitionNum);
#endif

/************************** Variable Definitions *****************************/
#ifdef ARMR5
	u8 R5LovecBuffer[32] = {0U};
	u8 R5HivecBuffer[32] = {0U};
	u32 TcmSkipLength = 0U;
	PTRSIZE TcmSkipAddress = 0U;
	u8 IsR5IvtBackup = FALSE;
#endif

#ifdef XFSBL_SECURE
u32 Iv[XIH_BH_IV_LENGTH / 4U] = { 0 };
u8 AuthBuffer[XFSBL_AUTH_BUFFER_SIZE]__attribute__ ((aligned (4))) = {0};
#ifdef XFSBL_BS
#ifdef __clang__
u8 HashsOfChunks[HASH_BUFFER_SIZE] __attribute__((section (".bss.bitstream_buffer")));
#else
u8 HashsOfChunks[HASH_BUFFER_SIZE] __attribute__((section (".bitstream_buffer")));
#endif
#endif
#endif

/* buffer for storing chunks for bitstream */
#if defined(XFSBL_BS)
extern u8 ReadBuffer[READ_BUFFER_SIZE];
#endif
/*****************************************************************************/
/**
 * This function loads the partition
 *
 * @param	FsblInstancePtr is pointer to the XFsbl Instance
 *
 * @param	PartitionNum is the partition number in the image to be loaded
 *
 * @return	returns the error codes described in xfsbl_error.h on any error
 * 			returns XFSBL_SUCCESS on success
 *
 *****************************************************************************/
u32 XFsbl_PartitionLoad(XFsblPs * FsblInstancePtr, u32 PartitionNum)
{
	u32 Status;
#ifdef ARMR5
	u32 Index;
#endif

#ifdef XFSBL_WDT_PRESENT
	if (XFSBL_MASTER_ONLY_RESET != FsblInstancePtr->ResetReason) {
		/* Restart WDT as partition copy can take more time */
		XFsbl_RestartWdt();
	}
#endif

#ifdef XFSBL_ENABLE_DDR_SR
	XFsbl_PollForDDRReady();
#endif

	/**
	 * Load and validate the partition
	 */

	/**
	 * Partition Header Validation
	 */
	Status = XFsbl_PartitionHeaderValidation(FsblInstancePtr, PartitionNum);

	/**
	 * FSBL is not partition owner and skip this partition
	 */
	if (Status == XFSBL_SUCCESS_NOT_PARTITION_OWNER)
	{
		Status = XFSBL_SUCCESS;
		goto END;
	} else if (XFSBL_SUCCESS != Status)
	{
		goto END;
	} else {
		/**
		 *  for MISRA C compliance
		 */
	}

	/**
	 * Partition Copy
	 */
	Status = XFsbl_PartitionCopy(FsblInstancePtr, PartitionNum);
	if (XFSBL_SUCCESS != Status)
	{
		goto END;
	}

	/**
	 * Partition Validation
	 */
	Status = XFsbl_PartitionValidation(FsblInstancePtr, PartitionNum);
	if (XFSBL_SUCCESS != Status)
	{
		goto END;
	}

#ifdef ARMR5
	if(IsR5IvtBackup == TRUE) {
		XFsbl_Printf(DEBUG_DETAILED,"XFsbl_PartitionLoad:After Partition Validation\n\r"
				"Going for LOVEC HIGHVEC Mechanism for R5.\n\r");

		/*Store LOVEC 32 bytes data to R5LovecBuffer
		 * This will copy Partition vectors into R5LovecBuffer.
		 */
		(void)XFsbl_MemCpy((u8*)R5LovecBuffer,(u8*)XFSBL_R5_LOVEC,XFSBL_IVT_LENGTH);

		/**
		 * Update the Low Vector locations in R5 TCM.
		 * It will make sure after partition authentication and decryption
		 * R5 will look for exception vectors at LOVEC only.
		 */

		Index = XFSBL_R5_LOVEC;
		while (Index<32U) {
			XFsbl_Out32(Index, XFSBL_R5_LOVEC_VALUE);
			Index += 4U;
		}

		/**
		 * Make sure that Low Vector locations are written Properly.
		 * Flush the cache*/
		Xil_DCacheFlush();

		/*Set exception vector to LOVEC */
		XFsbl_SetR5ExcepVectorLoVec();

		/* Restore R5HivecBuffer to HIVEC
		 * It will make sure that we are not corrupting HIVEC area.
		 */
		(void)XFsbl_MemCpy((u8*)XFSBL_R5_HIVEC,(u8*)R5HivecBuffer,XFSBL_IVT_LENGTH);
	}
#endif

	/* Check if PMU FW load is done and handoff it to Microblaze */
	XFsbl_CheckPmuFw(FsblInstancePtr, PartitionNum);

END:
	return Status;
}

/*****************************************************************************/
/**
 * This function validates the partition header
 *
 * @param	FsblInstancePtr is pointer to the XFsbl Instance
 *
 * @param	PartitionNum is the partition number in the image to be loaded
 *
 * @return	returns the error codes described in xfsbl_error.h on any error
 * 			returns XFSBL_SUCCESS on success
 *****************************************************************************/
static u32 XFsbl_PartitionHeaderValidation(XFsblPs * FsblInstancePtr,
		u32 PartitionNum)
{
	u32 Status;
	XFsblPs_PartitionHeader * PartitionHeader;

	/**
	 * Assign the partition header to local variable
	 */
	PartitionHeader =
		&FsblInstancePtr->ImageHeader.PartitionHeader[PartitionNum];

	/**
	 * Check the check sum of the partition header
	 */
	Status = XFsbl_ValidateChecksum( (u32 *)PartitionHeader, XIH_PH_LEN/4U);
	if (XFSBL_SUCCESS != Status)
	{
		Status = XFSBL_ERROR_PH_CHECKSUM_FAILED;
		XFsbl_Printf(DEBUG_GENERAL,
			"XFSBL_ERROR_PH_CHECKSUM_FAILED\n\r");
		goto END;
	}

	/**
	 * Check if partition belongs to FSBL
	 */
	if (XFsbl_GetPartitionOwner(PartitionHeader) !=
			XIH_PH_ATTRB_PART_OWNER_FSBL)
	{
		/**
		 * If the partition doesn't belong to FSBL, skip the partition
		 */
		XFsbl_Printf(DEBUG_GENERAL,"Skipping the Partition 0x%0lx\n",
				PartitionNum);
		Status = XFSBL_SUCCESS_NOT_PARTITION_OWNER;
		goto END;
	}

	/**
	 * Validate the fields of partition
	 */
	Status = XFsbl_ValidatePartitionHeader(
			PartitionHeader, FsblInstancePtr->ProcessorID, FsblInstancePtr->ResetReason);
	if (XFSBL_SUCCESS != Status)
	{
		goto END;
	}

END:
	return Status;
}

/****************************************************************************/
/**
 * This function is used to check whether cpu has handoff address stored
 * in the handoff structure
 *
 * @param FsblInstancePtr is pointer to the XFsbl Instance
 *
 * @param DestinationCpu is the cpu which needs to be checked
 *
 * @return
 * 		- XFSBL_SUCCESS if cpu handoff address is not present
 * 		- XFSBL_FAILURE if cpu handoff address is present
 *
 * @note
 *
 *****************************************************************************/
static u32 XFsbl_CheckHandoffCpu (const XFsblPs * FsblInstancePtr,
						u32 DestinationCpu)
{
	u32 ValidHandoffCpuNo;
	u32 Status;
	u32 Index;
	u32 CpuId;


	ValidHandoffCpuNo = FsblInstancePtr->HandoffCpuNo;

	for (Index=0U;Index<ValidHandoffCpuNo;Index++)
	{
		CpuId = FsblInstancePtr->HandoffValues[Index].CpuSettings &
			XIH_PH_ATTRB_DEST_CPU_MASK;
		if (CpuId == DestinationCpu)
		{
			Status = XFSBL_FAILURE;
			goto END;
		}
	}

	Status = XFSBL_SUCCESS;
END:
	return Status;
}

/*****************************************************************************/
/**
 * This function checks the power state and reset for the memory type
 * and release the reset if required
 *
 * @param	MemoryType is the memory to be checked
 * 			- XFSBL_R5_0_TCM
 * 			- XFSBL_R5_1_TCM
 *				(to be added)
 *			- XFSBL_R5_0_TCMA
 *			- XFSBL_R5_0_TCMB
 *			- XFSBL_PS_DDR
 *			- XFSBL_PL_DDR
 *
 * @return	none
 *****************************************************************************/
u32 XFsbl_PowerUpMemory(u32 MemoryType)
{
	u32 RegValue;
	u32 Status;
	u32 PwrStateMask;

	/**
	 * Check the power status of the memory
	 * Power up if required
	 *
	 * Release the reset of the memory if present
	 */
	switch (MemoryType)
	{
		case XFSBL_R5_0_TCM:
		{

			PwrStateMask = (PMU_GLOBAL_PWR_STATE_R5_0_MASK |
					PMU_GLOBAL_PWR_STATE_TCM0A_MASK |
					PMU_GLOBAL_PWR_STATE_TCM0B_MASK);

			Status = XFsbl_PowerUpIsland(PwrStateMask);

			if (Status != XFSBL_SUCCESS) {
				Status = XFSBL_ERROR_R5_0_TCM_POWER_UP;
				XFsbl_Printf(DEBUG_GENERAL, "XFSBL_ERROR_R5_0_TCM_POWER_UP\r\n");
				goto END;
			}

			/**
			 * To access TCM,
			 * 	Release reset to R5 and enable the clk
			 * 	R5 is under halt state
			 *
			 * 	If R5 are out of reset and clk is enabled so doing
			 * 	again is no issue. R5 might be under running state
			 */

			/**
			 * Place R5, TCM in split mode
			 */
			RegValue = XFsbl_In32(RPU_RPU_GLBL_CNTL);
			RegValue |= RPU_RPU_GLBL_CNTL_SLSPLIT_MASK;
			RegValue &= ~(RPU_RPU_GLBL_CNTL_TCM_COMB_MASK);
			RegValue &= ~(RPU_RPU_GLBL_CNTL_SLCLAMP_MASK);
			XFsbl_Out32(RPU_RPU_GLBL_CNTL, RegValue);

			/**
			 * Place R5-0 in HALT state
			 */
			RegValue = XFsbl_In32(RPU_RPU_0_CFG);
			RegValue &= ~(RPU_RPU_0_CFG_NCPUHALT_MASK);
			XFsbl_Out32(RPU_RPU_0_CFG, RegValue);

			/**
			 *  Enable the clock
			 */
			RegValue = XFsbl_In32(CRL_APB_CPU_R5_CTRL);
			RegValue |= CRL_APB_CPU_R5_CTRL_CLKACT_MASK;
			XFsbl_Out32(CRL_APB_CPU_R5_CTRL, RegValue);

			/**
			 * Provide some delay,
			 * so that clock propagates properly.
			 */
			(void)usleep(0x50U);

			/**
			 * Release reset to R5-0
			 */
			RegValue = XFsbl_In32(CRL_APB_RST_LPD_TOP);
			RegValue &= ~(CRL_APB_RST_LPD_TOP_RPU_R50_RESET_MASK);
			RegValue &= ~(CRL_APB_RST_LPD_TOP_RPU_AMBA_RESET_MASK);
			XFsbl_Out32(CRL_APB_RST_LPD_TOP, RegValue);
		} break;

		case XFSBL_R5_1_TCM:
		{

			PwrStateMask = (PMU_GLOBAL_PWR_STATE_R5_1_MASK |
					PMU_GLOBAL_PWR_STATE_TCM1A_MASK |
					PMU_GLOBAL_PWR_STATE_TCM1B_MASK);

			Status = XFsbl_PowerUpIsland(PwrStateMask);

			if (Status != XFSBL_SUCCESS) {
				Status = XFSBL_ERROR_R5_1_TCM_POWER_UP;
				XFsbl_Printf(DEBUG_GENERAL, "XFSBL_ERROR_R5_1_TCM_POWER_UP\r\n");
				goto END;
			}

			/**
			 * Place R5 in split mode
			 */
			RegValue = XFsbl_In32(RPU_RPU_GLBL_CNTL);
			RegValue |= RPU_RPU_GLBL_CNTL_SLSPLIT_MASK;
			RegValue &= ~(RPU_RPU_GLBL_CNTL_TCM_COMB_MASK);
			RegValue &= ~(RPU_RPU_GLBL_CNTL_SLCLAMP_MASK);
			XFsbl_Out32(RPU_RPU_GLBL_CNTL, RegValue);

			/**
			 * Place R5-1 in HALT state
			 */
			RegValue = XFsbl_In32(RPU_RPU_1_CFG);
			RegValue &= ~(RPU_RPU_1_CFG_NCPUHALT_MASK);
			XFsbl_Out32(RPU_RPU_1_CFG, RegValue);

			/**
			 *  Enable the clock
			 */
			RegValue = XFsbl_In32(CRL_APB_CPU_R5_CTRL);
			RegValue |= CRL_APB_CPU_R5_CTRL_CLKACT_MASK;
			XFsbl_Out32(CRL_APB_CPU_R5_CTRL, RegValue);

			/**
			 * Provide some delay,
			 * so that clock propagates properly.
			 */
			(void )usleep(0x50U);

			/**
			 * Release reset to R5-1
			 */
			RegValue = XFsbl_In32(CRL_APB_RST_LPD_TOP);
			RegValue &= ~(CRL_APB_RST_LPD_TOP_RPU_R51_RESET_MASK);
			RegValue &= ~(CRL_APB_RST_LPD_TOP_RPU_AMBA_RESET_MASK);
			XFsbl_Out32(CRL_APB_RST_LPD_TOP, RegValue);
		} break;

		case XFSBL_R5_L_TCM:
		{


			PwrStateMask = (PMU_GLOBAL_PWR_STATE_R5_0_MASK |
					PMU_GLOBAL_PWR_STATE_TCM0A_MASK |
					PMU_GLOBAL_PWR_STATE_TCM0B_MASK |
					PMU_GLOBAL_PWR_STATE_TCM1A_MASK |
					PMU_GLOBAL_PWR_STATE_TCM1B_MASK);

			Status = XFsbl_PowerUpIsland(PwrStateMask);

			if (Status != XFSBL_SUCCESS) {
				Status = XFSBL_ERROR_R5_L_TCM_POWER_UP;
				XFsbl_Printf(DEBUG_GENERAL, "XFSBL_ERROR_R5_L_TCM_POWER_UP\r\n");
				goto END;
			}

			/**
			 * Place R5 in lock step mode
			 * Combine TCM's
			 */
			RegValue = XFsbl_In32(RPU_RPU_GLBL_CNTL);
			RegValue |= RPU_RPU_GLBL_CNTL_SLCLAMP_MASK;
			RegValue &= ~(RPU_RPU_GLBL_CNTL_SLSPLIT_MASK);
			RegValue |= RPU_RPU_GLBL_CNTL_TCM_COMB_MASK;
			XFsbl_Out32(RPU_RPU_GLBL_CNTL, RegValue);

			/**
                         * Place R5-0 in HALT state
                         */
                        RegValue = XFsbl_In32(RPU_RPU_0_CFG);
                        RegValue &= ~(RPU_RPU_0_CFG_NCPUHALT_MASK);
                        XFsbl_Out32(RPU_RPU_0_CFG, RegValue);

			/**
                         * Place R5-1 in HALT state
                         */
                        RegValue = XFsbl_In32(RPU_RPU_1_CFG);
                        RegValue &= ~(RPU_RPU_1_CFG_NCPUHALT_MASK);
                        XFsbl_Out32(RPU_RPU_1_CFG, RegValue);

                        /**
                         *  Enable the clock
                         */
                        RegValue = XFsbl_In32(CRL_APB_CPU_R5_CTRL);
                        RegValue |= CRL_APB_CPU_R5_CTRL_CLKACT_MASK;
                        XFsbl_Out32(CRL_APB_CPU_R5_CTRL, RegValue);

                        /**
                         * Provide some delay,
                         * so that clock propagates properly.
                         */
                        (void )usleep(0x50U);

                        /**
                         * Release reset to R5-0,R5-1
                         */
                        RegValue = XFsbl_In32(CRL_APB_RST_LPD_TOP);
                        RegValue &= ~(CRL_APB_RST_LPD_TOP_RPU_R50_RESET_MASK);
                        RegValue &= ~(CRL_APB_RST_LPD_TOP_RPU_R51_RESET_MASK);
                        RegValue &= ~(CRL_APB_RST_LPD_TOP_RPU_AMBA_RESET_MASK);
                        XFsbl_Out32(CRL_APB_RST_LPD_TOP, RegValue);
		} break;

		default:
			/* nothing to do */
			Status = XFSBL_SUCCESS;
			break;
	}

END:
	return Status;
}

/*****************************************************************************/
/**
 * This function validates the load address for R5 elfs and maps it to global
 * physical TCM address space so that any cpu can access it globally.
 *
 * @param       DestinationCpu is the cpu on which partition will run
 *
 * @param       LoadAddress will be updated according to the cpu and address
 *
 * @param       Length of the data to be copied. This is required only to
 *              check for error cases
 *
 * @return      returns the error codes described in xfsbl_error.h on any error
 *                      returns XFSBL_SUCCESS on success
 *****************************************************************************/
static u32 XFsbl_GetLoadAddress(u32 DestinationCpu, PTRSIZE * LoadAddressPtr, u32 Length)
{
	u32 Status;
	PTRSIZE Address;

	Address = *LoadAddressPtr;

	/* Update for R50 TCM address if the partition fits with in a TCM bank */
	if ((DestinationCpu == XIH_PH_ATTRB_DEST_CPU_R5_0) &&
			((Address < (XFSBL_R5_TCM_START_ADDRESS + XFSBL_R5_TCM_BANK_LENGTH))||
					((Address >= XFSBL_R5_BTCM_START_ADDRESS) &&
							(Address < (XFSBL_R5_BTCM_START_ADDRESS +
							XFSBL_R5_TCM_BANK_LENGTH)))))
	{
		/* Check if fits in to a single TCM bank or not */
		if (Length > XFSBL_R5_TCM_BANK_LENGTH)
		{
			Status = XFSBL_ERROR_LOAD_ADDRESS;
			XFsbl_Printf(DEBUG_GENERAL,
				"XFSBL_ERROR_LOAD_ADDRESS\r\n");
			goto END;
		}

		/**
		 * Update Address to the higher TCM address
		 */
		Address = XFSBL_R50_HIGH_ATCM_START_ADDRESS + Address;

	} else
		/* Update for R51 TCM address if the partition fits with in a TCM bank */
		if ((DestinationCpu == XIH_PH_ATTRB_DEST_CPU_R5_1) &&
			((Address < (XFSBL_R5_TCM_START_ADDRESS + XFSBL_R5_TCM_BANK_LENGTH))||
						((Address >= XFSBL_R5_BTCM_START_ADDRESS) &&
								(Address < (XFSBL_R5_BTCM_START_ADDRESS +
							XFSBL_R5_TCM_BANK_LENGTH)))))
	{
		/* Check if fits in to a single TCM bank or not */
		if (Length > XFSBL_R5_TCM_BANK_LENGTH)
		{
			Status = XFSBL_ERROR_LOAD_ADDRESS;
			XFsbl_Printf(DEBUG_GENERAL,
				"XFSBL_ERROR_LOAD_ADDRESS\r\n");
			goto END;
		}
		/**
		 * Update Address to the higher TCM address
		 */
		Address = XFSBL_R51_HIGH_ATCM_START_ADDRESS + Address;
	} else
	/**
	 * Update for the R5-L TCM address
	 */
	if ((DestinationCpu == XIH_PH_ATTRB_DEST_CPU_R5_L) &&
			(Address < (XFSBL_R5_TCM_START_ADDRESS +
					(XFSBL_R5_TCM_BANK_LENGTH * 4U))))
	{
		/**
		 * Check if fits to TCM or not
		 */
		if (Length > (XFSBL_R5_TCM_BANK_LENGTH * 4U))
		{
			Status = XFSBL_ERROR_LOAD_ADDRESS;
			XFsbl_Printf(DEBUG_GENERAL,
				"XFSBL_ERROR_LOAD_ADDRESS\r\n");
			goto END;
		}
		/**
		 * Update Address to the higher TCM address
		 */
		Address = XFSBL_R50_HIGH_ATCM_START_ADDRESS + Address;
	} else
	{
		/**
		 * For MISRA complaince
		 */
	}

	/**
	 * Update the LoadAddress
	 */
	*LoadAddressPtr = Address;

	Status = XFSBL_SUCCESS;
END:
	return Status;
}



/*****************************************************************************/
/**
 * This function calculates the load address based on the destination
 * cpu. For R5 cpu's TCM address is remapped to the higher TCM address
 * so that any cpu can globally access it
 *
 * @param	DestinationCpu is the cpu on which partition will run
 *
 * @param	LoadAddress will be updated according to the cpu and address
 *
 * @param	Length of the data to be copied. This is required only to
 *              check for the error case
 *
 * @return	returns the error codes described in xfsbl_error.h on any error
 * 			returns XFSBL_SUCCESS on success
 *****************************************************************************/
static u32 XFsbl_ConfigureMemory(XFsblPs * FsblInstancePtr, u32 RunningCpu,
		u32 DestinationCpu, u64 Address)
{

	u32 Status;
	/**
	 * Configure R50 TCM Memory
	 */
	if ((DestinationCpu == XIH_PH_ATTRB_DEST_CPU_R5_0) &&
		(((Address >= XFSBL_R50_HIGH_ATCM_START_ADDRESS) &&
		(Address < (XFSBL_R50_HIGH_ATCM_START_ADDRESS +
				XFSBL_R5_TCM_BANK_LENGTH))) ||
				((Address >= XFSBL_R50_HIGH_BTCM_START_ADDRESS) &&
				(Address < (XFSBL_R50_HIGH_BTCM_START_ADDRESS +
						XFSBL_R5_TCM_BANK_LENGTH)))))
	{

		/**
		 * Power up and release reset to the memory
		 */
		if (RunningCpu != DestinationCpu)
		{
			Status = XFsbl_PowerUpMemory(XFSBL_R5_0_TCM);
			if (Status != XFSBL_SUCCESS) {
				goto END;
			}
		}

		/**
		 * ECC initialize TCM
		 */
		if ((FsblInstancePtr->TcmEccInitStatus &
				XFSBL_R50_TCM_ECC_INIT_STATUS) == FALSE)
		{
			Status = XFsbl_TcmEccInit(FsblInstancePtr, DestinationCpu);
			if (XFSBL_SUCCESS != Status) {
				goto END;
			}
		}

	} else
	/**
	 * Update for R5-1 TCM address
	 */
	if ((DestinationCpu == XIH_PH_ATTRB_DEST_CPU_R5_1) &&
		(((Address >= XFSBL_R51_HIGH_ATCM_START_ADDRESS) &&
		(Address < (XFSBL_R51_HIGH_ATCM_START_ADDRESS +
				XFSBL_R5_TCM_BANK_LENGTH))) ||
				((Address >= XFSBL_R51_HIGH_BTCM_START_ADDRESS) &&
				(Address < (XFSBL_R51_HIGH_BTCM_START_ADDRESS +
						XFSBL_R5_TCM_BANK_LENGTH)))))

	{
		/**
		 * Power up and release reset to the memory
		 */
		if (RunningCpu != DestinationCpu)
		{
			Status = XFsbl_PowerUpMemory(XFSBL_R5_1_TCM);
			if (Status != XFSBL_SUCCESS) {
				goto END;
			}
		}

		/**
		 * ECC initialize TCM
		 */
		if ((FsblInstancePtr->TcmEccInitStatus &
				XFSBL_R51_TCM_ECC_INIT_STATUS) == FALSE)
		{
			Status = XFsbl_TcmEccInit(FsblInstancePtr, DestinationCpu);
			if (XFSBL_SUCCESS != Status) {
				goto END;
			}
		}
	} else
	/**
	 * Update for the R5-L TCM address
	 */
	if ((DestinationCpu == XIH_PH_ATTRB_DEST_CPU_R5_L) &&
		(Address >= XFSBL_R50_HIGH_ATCM_START_ADDRESS) &&
		(Address < (XFSBL_R50_HIGH_ATCM_START_ADDRESS +
				(XFSBL_R5_TCM_BANK_LENGTH*4U))))
	{
		/**
		 * Power up and release reset to the memory
		 */
		if (RunningCpu != DestinationCpu)
		{
			Status = XFsbl_PowerUpMemory(XFSBL_R5_L_TCM);
			if (Status != XFSBL_SUCCESS) {
				goto END;
			}

		}

		/**
		 * ECC initialize TCM
		 */
		if ((FsblInstancePtr->TcmEccInitStatus &
			(XFSBL_R50_TCM_ECC_INIT_STATUS | XFSBL_R51_TCM_ECC_INIT_STATUS)) !=
				(XFSBL_R50_TCM_ECC_INIT_STATUS | XFSBL_R51_TCM_ECC_INIT_STATUS))
		{
			Status = XFsbl_TcmEccInit(FsblInstancePtr, DestinationCpu);
			if (XFSBL_SUCCESS != Status) {
				goto END;
			}
		}
	} else
	{
		/**
		 * For MISRA complaince
		 */
	}

	Status = XFSBL_SUCCESS;
END:
	return Status;
}


/*****************************************************************************/
/**
 * This function copies the partition to specified destination
 *
 * @param	FsblInstancePtr is pointer to the XFsbl Instance
 *
 * @param	PartitionNum is the partition number in the image to be loaded
 *
 * @return	returns the error codes described in xfsbl_error.h on any error
 * 			returns XFSBL_SUCCESS on success
 *****************************************************************************/
static u32 XFsbl_PartitionCopy(XFsblPs * FsblInstancePtr, u32 PartitionNum)
{
	u32 Status;
	u32 DestinationCpu;
	u32 CpuNo;
	u32 DestinationDevice;
	u32 ExecState;
	XFsblPs_PartitionHeader * PartitionHeader;
	u32 SrcAddress;
	PTRSIZE LoadAddress;
	u32 Length;
	u32 RunningCpu;
	u32 RegVal;

#ifdef ARMR5
	u32 Index;
#endif

	/**
	 * Assign the partition header to local variable
	 */
	PartitionHeader =
		&FsblInstancePtr->ImageHeader.PartitionHeader[PartitionNum];

	RunningCpu = FsblInstancePtr->ProcessorID;

	/**
	 * Check for XIP image
	 * No need to copy for XIP image
	 */
	DestinationCpu = XFsbl_GetDestinationCpu(PartitionHeader);

	/**
	 * if destination cpu is not present, it means it is for same cpu
	 */
	if (DestinationCpu == XIH_PH_ATTRB_DEST_CPU_NONE)
	{
		DestinationCpu = FsblInstancePtr->ProcessorID;
	}

	if (PartitionHeader->UnEncryptedDataWordLength == 0U)
	{
		/**
		 * Update the Handoff address only for the first application
		 * of that cpu
		 * This is for XIP image. For other partitions it handoff
		 * address is updated after partition validation
		 */
		CpuNo = FsblInstancePtr->HandoffCpuNo;
		if (XFsbl_CheckHandoffCpu(FsblInstancePtr,
				DestinationCpu) == XFSBL_SUCCESS)
		{
			/* Get the execution state */
			ExecState = XFsbl_GetA53ExecState(PartitionHeader);
			FsblInstancePtr->HandoffValues[CpuNo].CpuSettings =
			        DestinationCpu | ExecState;
			FsblInstancePtr->HandoffValues[CpuNo].HandoffAddress =
				PartitionHeader->DestinationExecutionAddress;
			FsblInstancePtr->HandoffCpuNo += 1U;
		} else {
			/**
			 *
			 * if two partitions has same destination cpu, error can
			 * be triggered here
			 */
		}
		Status = XFSBL_SUCCESS;
		goto END;
	}

	/**
	 * Get the source(flash offset) address where it needs to copy
	 */
	SrcAddress = FsblInstancePtr->ImageOffsetAddress +
				((PartitionHeader->DataWordOffset) *
					XIH_PARTITION_WORD_LENGTH);

	/**
	 * Length of the partition to be copied
	 */
	Length  = (PartitionHeader->TotalDataWordLength) *
					XIH_PARTITION_WORD_LENGTH;
	DestinationDevice = XFsbl_GetDestinationDevice(PartitionHeader);

	/**
	 * Copy the authentication certificate to auth. buffer
	 * Update Partition length to be copied.
	 * For bitstream it will be taken care saperately
	 */
#ifdef XFSBL_SECURE
	if ((XFsbl_IsRsaSignaturePresent(PartitionHeader) ==
			XIH_PH_ATTRB_RSA_SIGNATURE) &&
		(DestinationDevice != XIH_PH_ATTRB_DEST_DEVICE_PL)) {

		Length = Length - XFSBL_AUTH_CERT_MIN_SIZE;

		Status = FsblInstancePtr->DeviceOps.DeviceCopy((SrcAddress + Length),
					(INTPTR)AuthBuffer, XFSBL_AUTH_CERT_MIN_SIZE);
		if (XFSBL_SUCCESS != Status)
		{
			goto END;
		}
	}
#endif

	LoadAddress = (PTRSIZE)PartitionHeader->DestinationLoadAddress;
	/**
	 * Copy the PL to temporary DDR Address
	 * Copy the PS to Load Address
	 * Copy the PMU firmware to PMU RAM
	 */

	if (DestinationDevice == XIH_PH_ATTRB_DEST_DEVICE_PL)
	{
#ifdef XFSBL_BS
		/**
		 * In case of PS Only Reset, skip copying
		 * the PL bitstream
		 */
		if (FsblInstancePtr->ResetReason == XFSBL_PS_ONLY_RESET)
		{
			Status = XFSBL_SUCCESS;
			goto END;
		}

		if (LoadAddress == XFSBL_DUMMY_PL_ADDR)
		{
			LoadAddress = XFSBL_DDR_TEMP_ADDRESS;

#ifdef XFSBL_PL_LOAD_FROM_OCM
			/* In case of PL load from OCM, skip copying */
			Status = XFSBL_SUCCESS;
			goto END;
#endif
		}

#else
		XFsbl_Printf(DEBUG_GENERAL,"XFSBL_ERROR_PL_NOT_ENABLED \r\n");
		Status = XFSBL_ERROR_PL_NOT_ENABLED;
		goto END;
#endif
	}

	/**
	 * When destination device is R5-0/R5-1/R5-L and load address is in TCM
	 * copy to high address of TCM address map
	 * Update the LoadAddress
	 */
	Status = XFsbl_GetLoadAddress(DestinationCpu, &LoadAddress, Length);
	if (XFSBL_SUCCESS != Status)
	{
		goto END;
	}

	/**
	 * Configure the memory
	 */
	Status = XFsbl_ConfigureMemory(FsblInstancePtr, RunningCpu, DestinationCpu,
						LoadAddress);
	if (XFSBL_SUCCESS != Status)
	{
		goto END;
	}


#ifdef ARMR5

	/*Disable IsR5IvtBackup */
	IsR5IvtBackup = FALSE;

	/**
	 *
	 * Enable IsR5IvtBackup,if FSBL is running in R5-0/R5-L at 0x0 TCM
	 * Store HIVEC 32 byte data to R5HivecBuffer,
	 * Update the High Vector locations for R5,
	 * set Exception Vector to HIVEC,based on above condition.
	 */
	if (((FsblInstancePtr->ProcessorID ==
			XIH_PH_ATTRB_DEST_CPU_R5_0) ||
		(FsblInstancePtr->ProcessorID ==
				XIH_PH_ATTRB_DEST_CPU_R5_L))  &&
		((LoadAddress >= XFSBL_R50_HIGH_ATCM_START_ADDRESS) &&
		(LoadAddress <
			(XFSBL_R50_HIGH_ATCM_START_ADDRESS + XFSBL_IVT_LENGTH))))
	{


		/**
		 * Enable IsR5IvtBackup,this will used in
		 * XFsbl_PartitionLoad for restoring R5 vectors
		 */
		IsR5IvtBackup = TRUE;

		/**
		 * Get the length of the IVT area to be
		 * skipped from Load Address
		 */
		TcmSkipAddress = LoadAddress%XFSBL_IVT_LENGTH;
		TcmSkipLength = XFSBL_IVT_LENGTH - TcmSkipAddress;
		XFsbl_Printf(DEBUG_DETAILED,"XFsbl_PartitionCopy:Going for LOVEC HIGHVEC Mechanism for R5.\n\r");

		/**
		 * Check if Length is less than SkipLength
		 */
		if (TcmSkipLength > Length) {
			TcmSkipLength = Length;
		}

		/*Store HIVEC 32 bytes data to R5HivecBuffer*/
		(void)XFsbl_MemCpy((u8*)R5HivecBuffer,(u8*)XFSBL_R5_HIVEC,XFSBL_IVT_LENGTH);


		 /* Update the High Vector locations for R5.*/

		Index = XFSBL_R5_HIVEC;
		while (Index < (XFSBL_R5_HIVEC + 32U)) {
			XFsbl_Out32(Index, XFSBL_R5_HIVEC_VALUE);
			Index += 4U;
		}

		/**
		 * Make sure that Low Vector locations are written Properly
		 * Flush the cache
		 */
		Xil_DCacheFlush();

		/*set exception vector to HIVEC */
		XFsbl_SetR5ExcepVectorHiVec();
	}
#endif

	if (DestinationCpu == XIH_PH_ATTRB_DEST_CPU_PMU)
	{
		/* Trigger IPI only for first PMUFW partition */
		if(PartitionHeader->DestinationExecutionAddress != 0U) {
			/* Enable PMU_0 IPI */
			XFsbl_Out32(IPI_PMU_0_IER, IPI_PMU_0_IER_PMU_0_MASK);

			/* Trigger PMU0 IPI in PMU IPI TRIG Reg */
			XFsbl_Out32(IPI_PMU_0_TRIG, IPI_PMU_0_TRIG_PMU_0_MASK);
		}

		/**
		 * Wait until PMU Microblaze goes to sleep state,
		 * before starting firmware download to PMU RAM
		 */
		do {
				RegVal = XFsbl_In32(PMU_GLOBAL_GLOBAL_CNTRL);
				if ((RegVal & PMU_GLOBAL_GLOBAL_CNTRL_MB_SLEEP_MASK)
					== PMU_GLOBAL_GLOBAL_CNTRL_MB_SLEEP_MASK) {
					break;
				}
		} while (1);
	}

#ifdef XFSBL_PERF
	XTime tCur = 0;
	XTime_GetTime(&tCur);
#endif
	/**
	 * Copy the partition to PS_DDR/PL_DDR/TCM
	 */
	Status = FsblInstancePtr->DeviceOps.DeviceCopy(SrcAddress,
					LoadAddress, Length);

#ifdef XFSBL_PERF
	XFsbl_MeasurePerfTime(tCur);
	XFsbl_Printf(DEBUG_PRINT_ALWAYS, ": P%u Copy time, Size: %0u \r\n",
				PartitionNum, Length);
#endif


END:
	return Status;
}

/*****************************************************************************/
/**
 * This function validates the partition
 *
 * @param	FsblInstancePtr is pointer to the XFsbl Instance
 *
 * @param	PartitionNum is the partition number in the image to be loaded
 *
 * @return	returns the error codes described in xfsbl_error.h on any error
 * 			returns XFSBL_SUCCESS on success
 *
 *****************************************************************************/
static u32 XFsbl_PartitionValidation(XFsblPs * FsblInstancePtr,
						u32 PartitionNum)
{
	u32 Status = XFSBL_FAILURE;
	u32 IsEncryptionEnabled;
	u32 IsAuthenticationEnabled;
	u32 IsChecksumEnabled;
	u32 DestinationDevice;
	u32 DestinationCpu;
	u32 ExecState;
	u32 CpuNo;
	XFsblPs_PartitionHeader * PartitionHeader;
	PTRSIZE LoadAddress;
	u32 Length;
	u8 PartitionHash[XFSBL_HASH_TYPE_SHA3] __attribute__ ((aligned (4U))) =
		{0U};
#ifdef XFSBL_TPM
	u8 PcrIndex;
#endif
#if defined(XFSBL_SECURE)
	s32 SStatus;
	u32 FsblIv[XIH_BH_IV_LENGTH / 4U] = { 0 };
	u8 *IvPtr = (u8 *)&FsblIv[2];
	u32 UnencryptedLength = 0U;
	static XSecure_Aes SecureAes;
#ifdef XFSBL_BS
	XFsblPs_PlPartition PlParams = {0};
#ifdef XFSBL_PL_LOAD_FROM_OCM
	u32 SrcAddress = 0U;
#endif
#endif
#endif
#if defined(XFSBL_BS) && (!defined(XFSBL_PL_LOAD_FROM_OCM))
	u32 BitstreamWordSize;
#endif
#ifdef XFSBL_PERF
	XTime tCur = 0;
#endif

	/**
	 * Update the variables
	 */
	PartitionHeader =
	    &FsblInstancePtr->ImageHeader.PartitionHeader[PartitionNum];

	/**
	 * Check the encryption status
	 */
	if (XFsbl_IsEncrypted(PartitionHeader) ==
			XIH_PH_ATTRB_ENCRYPTION )
	{
		IsEncryptionEnabled = TRUE;

#ifdef XFSBL_SECURE
		/* Copy IV to local variable */
		XFsbl_MemCpy(FsblIv, Iv, XIH_BH_IV_LENGTH);
		/* Updating IV of the partition by taking from partition header */
		*(IvPtr + 3) = (*(IvPtr + 3)) +
				(PartitionHeader->Iv & XFSBL_PARTITION_IV_MASK);
#else
		XFsbl_Printf(DEBUG_GENERAL,"XFSBL_ERROR_SECURE_NOT_ENABLED \r\n");
		Status = XFSBL_ERROR_SECURE_NOT_ENABLED;
		goto END;
#endif
	}
	else
	{
		Status = XFSBL_SUCCESS;
		IsEncryptionEnabled = FALSE;
	}

	/**
	 * check the authentication status
	 */
#ifdef FSBL_UNPROVISIONED_AUTH_SIGN_EXCLUDE
	if (XFsbl_IsRsaSignaturePresent(PartitionHeader) ==
			XIH_PH_ATTRB_RSA_SIGNATURE) {
#else
	if ((FsblInstancePtr->AuthEnabled == TRUE) &&
		(XFsbl_IsRsaSignaturePresent(PartitionHeader) ==
			XIH_PH_ATTRB_RSA_SIGNATURE)) {
#endif
		IsAuthenticationEnabled = TRUE;
	}
	else {
		IsAuthenticationEnabled = FALSE;
	}

	/* check the checksum status */
	if (XFsbl_GetChecksumType(PartitionHeader) !=
			XIH_PH_ATTRB_NOCHECKSUM )
	{
		IsChecksumEnabled = TRUE;
	}
	else
	{
		IsChecksumEnabled = FALSE;
	}

	DestinationDevice = XFsbl_GetDestinationDevice(PartitionHeader);

#ifdef XFSBL_BS
	if (DestinationDevice == XIH_PH_ATTRB_DEST_DEVICE_PL)
	{
		/**
		 * In case of PS Only Reset, skip configuring
		 * the PL bitstream
		 */
		if (FsblInstancePtr->ResetReason == XFSBL_PS_ONLY_RESET)
		{
			Status = XFSBL_SUCCESS;
			XFsbl_Printf(DEBUG_INFO,
			"PS Only Reset. Skipping PL configuration\r\n");
			goto END;
		}
	}
#endif

	/**
	 * if destination cpu is not present, it means it is for same cpu
	 */
	DestinationCpu = XFsbl_GetDestinationCpu(PartitionHeader);
	if (DestinationCpu == XIH_PH_ATTRB_DEST_CPU_NONE)
	{
		DestinationCpu = FsblInstancePtr->ProcessorID;
	}

	LoadAddress = (PTRSIZE) PartitionHeader->DestinationLoadAddress;
	Length = PartitionHeader->TotalDataWordLength * 4U;
	Status = XFsbl_GetLoadAddress(DestinationCpu, &LoadAddress, Length);
	if (XFSBL_SUCCESS != Status)
	{
		goto END;
	}

#ifdef XFSBL_BS
	if ((DestinationDevice == XIH_PH_ATTRB_DEST_DEVICE_PL) &&
			(LoadAddress == XFSBL_DUMMY_PL_ADDR))
	{
		XFsbl_Printf(DEBUG_INFO,
			"Destination Device is PL, changing LoadAddress\r\n");
		LoadAddress = XFSBL_DDR_TEMP_ADDRESS;
	}
#endif

	if (IsEncryptionEnabled == TRUE) {
#ifdef XFSBL_SECURE
		/* Initialize the Aes Instance so that it's ready to use */
		SStatus = XSecure_AesInitialize(&SecureAes, &CsuDma,
		 XSECURE_CSU_AES_KEY_SRC_DEV, FsblIv, NULL);
		if (SStatus != XFSBL_SUCCESS)
		{
			Status = XFSBL_ERROR_AES_INITIALIZE;
			XFsbl_Printf(DEBUG_GENERAL, "XFSBL_ERROR_AES_INITIALIZE_FAIL\r\n");
			goto END;
		}

		XFsbl_Printf(DEBUG_INFO, " Aes initialized \r\n");

		UnencryptedLength =
			PartitionHeader->UnEncryptedDataWordLength * 4U;
#endif
	}

#ifdef XFSBL_BS
	if (DestinationDevice == XIH_PH_ATTRB_DEST_DEVICE_PL) {
		/**
		 * Fsbl hook before bit stream download
		 */
		Status = XFsbl_HookBeforeBSDownload();
		if (Status != XFSBL_SUCCESS)
		{
			Status = XFSBL_ERROR_HOOK_BEFORE_BITSTREAM_DOWNLOAD;
			XFsbl_Printf(DEBUG_GENERAL,
			 "XFSBL_ERROR_HOOK_BEFORE_BITSTREAM_DOWNLOAD\r\n");
			goto END;
		}
		/**
		 * PCAP init will be skipped here if it is already done at
		 * FSBL initialization
		 */
#ifndef XFSBL_PL_CLEAR
		Status = XFsbl_PcapInit();
		if (Status != XFSBL_SUCCESS) {
			goto END;
		}
#endif
	}
#endif

	/* Checksum verification */
	if (IsChecksumEnabled == TRUE) {
		Status = XFsbl_CalculateCheckSum(FsblInstancePtr,
			LoadAddress, PartitionNum, PartitionHash);
		if (Status != XFSBL_SUCCESS) {
			XFsbl_Printf(DEBUG_GENERAL,
					"XFSBL_ERROR_PARTITION_CHECKSUM_FAILED \r\n");
			Status = XFSBL_ERROR_PARTITION_CHECKSUM_FAILED;
			goto END;
		}
		Status = XFsbl_ValidateCheckSum(FsblInstancePtr,
			LoadAddress, PartitionNum, PartitionHash);
		if (Status != XFSBL_SUCCESS) {
			goto END;
		}
	}
	/**
	 * Authentication Check
	 */
	if (IsAuthenticationEnabled == TRUE)
	{
		XFsbl_Printf(DEBUG_INFO,"Authentication Enabled\r\n");
#ifdef XFSBL_SECURE

#ifdef XFSBL_PERF
		/* Start time for partition authentication */
		XTime_GetTime(&tCur);
#endif

		if (DestinationDevice != XIH_PH_ATTRB_DEST_DEVICE_PL) {
			/**
			 * Authentication for non bitstream partition in DDR
			 * less system
			 */
			Status = XFsbl_Authentication(FsblInstancePtr, LoadAddress,
					Length, (PTRSIZE)AuthBuffer,
					PartitionNum);
			if (Status != XFSBL_SUCCESS) {
				goto END;
			}
		}
		else {
#ifdef XFSBL_BS
			/* Check whether secure bitstream is in blocks */
			if (XFsbl_GetBlockSize(PartitionHeader) == 0x00U) {
				Status = XFSBL_ERROR_BLOCK_SIZE_SEC_BS;
				XFsbl_Printf(DEBUG_INFO,"Use latest BOOTGEN"
						" to program secure bitsream\r\n");
				goto END;
			}
			if ((FsblInstancePtr->BootHdrAttributes &
					XIH_BH_IMAGE_ATTRB_SHA2_MASK) ==
					XIH_BH_IMAGE_ATTRB_SHA2_MASK) {
				Status = XFSBL_ERROR_SHA2_NOT_SUPPORTED;
				XFsbl_Printf(DEBUG_INFO,"SHA2 is not supported\r\n");
				goto END;
			}
			else {
				PlParams.PlAuth.AuthType = XFSBL_HASH_TYPE_SHA3;
				PlParams.PlAuth.NoOfHashs =
					HASH_BUFFER_SIZE/XFSBL_HASH_TYPE_SHA3;
			}
			PlParams.PlAuth.BlockSize =
				XFsbl_GetBlockSize(PartitionHeader);
			PlParams.PlAuth.AuthCertBuf = AuthBuffer;
			(void) memset(HashsOfChunks, 0U, sizeof(HashsOfChunks));
			PlParams.PlAuth.HashsOfChunks = HashsOfChunks;
			PlParams.Hash = &PartitionHash[0U];
			PlParams.ChunkBuffer = ReadBuffer;
			PlParams.ChunkSize = READ_BUFFER_SIZE;
			PlParams.CsuDmaPtr = &CsuDma;
			PlParams.IsAuthenticated = IsAuthenticationEnabled;

			PlParams.IsEncrypted = IsEncryptionEnabled;
			if (IsEncryptionEnabled == TRUE) {
				PlParams.PlEncrypt.SecureAes = &SecureAes;
			}
			else {
				PlParams.PlEncrypt.SecureAes = NULL;
			}

			PlParams.TotalLen =
				(PartitionHeader->TotalDataWordLength * 4U);
			PlParams.UnEncryptLen =
				(PartitionHeader->UnEncryptedDataWordLength
					* 4U);

#ifndef XFSBL_PL_LOAD_FROM_OCM
			PlParams.DeviceCopy = NULL;
			PlParams.StartAddress = LoadAddress;
			PlParams.PlAuth.AcOfset = LoadAddress +
				((PartitionHeader->AuthCertificateOffset * 4) -
				(PartitionHeader->DataWordOffset *
						XIH_PARTITION_WORD_LENGTH));
#else
			PlParams.DeviceCopy =
				FsblInstancePtr->DeviceOps.DeviceCopy;
			SrcAddress = FsblInstancePtr->ImageOffsetAddress +
				((PartitionHeader->DataWordOffset) *
				XIH_PARTITION_WORD_LENGTH);
			PlParams.StartAddress = SrcAddress;
			PlParams.PlAuth.AcOfset = FsblInstancePtr->ImageOffsetAddress +
				PartitionHeader->AuthCertificateOffset * 4;

#endif
			XFsbl_Printf(DEBUG_GENERAL,
			"Authenticated Bitstream download to start now\r\n");
			Status = XFsbl_SecPlPartition(FsblInstancePtr, &PlParams);
			if (Status != XFSBL_SUCCESS) {
				XFsbl_Printf(DEBUG_GENERAL,
				"XFSBL_ERROR_BITSTREAM_AUTHENTICATION\r\n");
				/* Reset PL */
				XFsbl_Out32(CSU_PCAP_PROG, 0x0);
				if(IsEncryptionEnabled == TRUE) {
					usleep(PL_RESET_PERIOD_IN_US);
					Xil_Out32(CSU_PCAP_PROG, CSU_PCAP_PROG_PCFG_PROG_B_MASK);
				}
				goto END;
			}
#endif
		}

#ifdef XFSBL_PERF
		XFsbl_MeasurePerfTime(tCur);
		XFsbl_Printf(DEBUG_PRINT_ALWAYS, ": P%d Auth. Time \r\n",
					PartitionNum);
#endif

#else
		XFsbl_Printf(DEBUG_GENERAL,"XFSBL_ERROR_SECURE_NOT_ENABLED \r\n");
		Status = XFSBL_ERROR_SECURE_NOT_ENABLED;
		goto END;
#endif
	}

	/**
	 * Decrypt image through CSU DMA
	 */
	if (IsEncryptionEnabled == TRUE) {
		XFsbl_Printf(DEBUG_INFO, "Decryption Enabled\r\n");
#ifdef XFSBL_SECURE

		if (DestinationDevice != XIH_PH_ATTRB_DEST_DEVICE_PL) {
#ifdef XFSBL_PERF
			/* Start time for non bitstream partition decryption */
			XTime_GetTime(&tCur);
#endif
			SStatus = XSecure_AesDecrypt(&SecureAes,
					(u8 *) LoadAddress, (u8 *) LoadAddress,
					UnencryptedLength);

			if (SStatus != XFSBL_SUCCESS) {
				Status = XFSBL_ERROR_DECRYPTION_FAIL;
				XFsbl_Printf(DEBUG_GENERAL,
					"XFSBL_ERROR_DECRYPTION_FAIL\r\n");
				goto END;
			} else {
				XFsbl_Printf(DEBUG_GENERAL,
					"Decryption Successful\r\n");
			}

#ifdef XFSBL_PERF
			XFsbl_MeasurePerfTime(tCur);
			XFsbl_Printf(DEBUG_PRINT_ALWAYS, ": P%d Dec. Time \r\n",
							PartitionNum);
#endif
		}
#else
		XFsbl_Printf(DEBUG_GENERAL,"XFSBL_ERROR_SECURE_NOT_ENABLED \r\n");
		Status = XFSBL_ERROR_SECURE_NOT_ENABLED;
		goto END;
#endif
	}

#ifdef XFSBL_BS
	/**
	 * for PL image use CSU DMA to route to PL
	 * When Authentication is not enabled
	 */
	if ((DestinationDevice == XIH_PH_ATTRB_DEST_DEVICE_PL) &&
			(IsAuthenticationEnabled == FALSE))
	{

		XFsbl_Printf(DEBUG_GENERAL,
		"Non authenticated Bitstream download to start now\r\n");

		if (IsEncryptionEnabled == TRUE) {
#ifdef XFSBL_SECURE

#ifdef XFSBL_PERF
			/* Start time for bitstream decryption */
			XTime_GetTime(&tCur);
#endif
#ifndef XFSBL_PL_LOAD_FROM_OCM
			/*
			 * The secure bitstream would be sent through CSU DMA to AES
			 * and the decrypted bitstream loaded to ReadBuffer from where
			 * it would be written to PCAP by configuring and re-configuring
			 * SSS CFG switch appropriately.
			 */
#ifdef XFSBL_TPM
			XFsbl_Printf(DEBUG_GENERAL,
				"Bitstream will be decrypted and transferred"
				" in chunks\r\n");
			/* Enable chunking in Decryption */
			XSecure_AesSetChunking(&SecureAes,
					XSECURE_CSU_AES_CHUNKING_ENABLED);
			XSecure_AesSetChunkConfig(&SecureAes, ReadBuffer,
					READ_BUFFER_SIZE, FsblInstancePtr->DeviceOps.DeviceCopy);
			SecureAes.IsPlDecryptToMemEnabled =
				XSECURE_PL_DEC_TO_MEM_ENABLED;
			XFsbl_ShaStart(NULL, XFSBL_HASH_TYPE_SHA3);
			SecureAes.ShaUpdate = XFsbl_ShaUpdate;
#endif
			Status = (u32)XSecure_AesDecrypt(&SecureAes,
				(u8 *)XFSBL_DESTINATION_PCAP_ADDR, (u8 *)LoadAddress,
				UnencryptedLength);
#else
			XFsbl_Printf(DEBUG_GENERAL,
				"Bitstream will be decrypted and transferred"
				" in chunks\r\n");
			/* Enable chunking in Decryption */
			XSecure_AesSetChunking(&SecureAes,
					XSECURE_CSU_AES_CHUNKING_ENABLED);
			XSecure_AesSetChunkConfig(&SecureAes, ReadBuffer,
					READ_BUFFER_SIZE,
					FsblInstancePtr->DeviceOps.DeviceCopy);
			SrcAddress = FsblInstancePtr->ImageOffsetAddress +
				((PartitionHeader->DataWordOffset) *
				XIH_PARTITION_WORD_LENGTH);

			/**
			 * In case of PL load from OCM, pass the partition source
			 * address from Flash device.
			 */
			Status = (u32)XSecure_AesDecrypt(&SecureAes,
					(u8 *) XFSBL_DESTINATION_PCAP_ADDR,
					(u8 *)(PTRSIZE) SrcAddress,
					UnencryptedLength);
#endif

#ifdef XFSBL_PERF
			XFsbl_MeasurePerfTime(tCur);
			XFsbl_Printf(DEBUG_PRINT_ALWAYS, ": P%d (sec. bitstream)"
						" Dec. + Pcap Load Time \r\n", PartitionNum);
#endif

			if (Status != XFSBL_SUCCESS) {
				Status = XFSBL_ERROR_BITSTREAM_DECRYPTION_FAIL;
				XFsbl_Printf(DEBUG_GENERAL,
				"XFSBL_ERROR_BITSTREAM_DECRYPTION_FAIL\r\n");
				/* Reset PL and PL zeroization */
				XFsbl_Out32(CSU_PCAP_PROG, 0x0);
				usleep(PL_RESET_PERIOD_IN_US);
				Xil_Out32(CSU_PCAP_PROG, CSU_PCAP_PROG_PCFG_PROG_B_MASK);
				goto END;
			} else {
				XFsbl_Printf(DEBUG_GENERAL,
					"Bitstream decryption Successful\r\n");
			}
#endif
		}
		else {
#ifdef XFSBL_PERF
			/* Start time for non sec. bitstream download */
			XTime_GetTime(&tCur);
#endif

#ifndef XFSBL_PL_LOAD_FROM_OCM
			/* Use CSU DMA to load Bit stream to PL */
			BitstreamWordSize =
				PartitionHeader->UnEncryptedDataWordLength;

			Status = XFsbl_WriteToPcap(BitstreamWordSize, (u8 *) LoadAddress);
			if (Status != XFSBL_SUCCESS) {
				goto END;
			}
#else
			/* In case of PL load from OCM, do the chunked transfer */
			Status = XFsbl_ChunkedBSTxfer(FsblInstancePtr,
							PartitionNum);
			if (Status != XFSBL_SUCCESS) {
				goto END;
			}

#endif

#ifdef XFSBL_PERF
			XFsbl_MeasurePerfTime(tCur);
			XFsbl_Printf(DEBUG_PRINT_ALWAYS, ": P%d "
					"(nsec. bitstream) dwnld Time \r\n", PartitionNum);
#endif
		}
	}
#endif

#ifdef XFSBL_BS
	if (DestinationDevice == XIH_PH_ATTRB_DEST_DEVICE_PL) {
		Status = XFsbl_PLWaitForDone();
		if (Status != XFSBL_SUCCESS) {
			goto END;
		}

		/**
		 * PL is powered-up before its configuration, but will be in isolation.
		 * Now since PL configuration is done, just remove the isolation
		 */
		psu_ps_pl_isolation_removal_data();

		/* Reset PL, if configured for */
		(void)psu_ps_pl_reset_config_data();

		/* Update PMU_GLOBAL_GEN_STORE Register */
#ifdef XFSBL_SECURE
		if ((IsAuthenticationEnabled == TRUE) || (IsEncryptionEnabled == TRUE))
		{
			XFsbl_SetBSSecureState(XFSBL_FIRMWARE_STATE_SECURE);
		} else
#endif
		{
			XFsbl_SetBSSecureState(XFSBL_FIRMWARE_STATE_NONSECURE);
		}

		/**
		 * Fsbl hook after bit stream download
		 */
		Status = XFsbl_HookAfterBSDownload();
		if (Status != XFSBL_SUCCESS)
		{
			Status = XFSBL_ERROR_HOOK_AFTER_BITSTREAM_DOWNLOAD;
			XFsbl_Printf(DEBUG_GENERAL,
			 "XFSBL_ERROR_HOOK_AFTER_BITSTREAM_DOWNLOAD\r\n");
			goto END;
		}
	}
#endif

	/**
	 * Update the handoff details
	 */
	if ((DestinationDevice != XIH_PH_ATTRB_DEST_DEVICE_PL) &&
			(DestinationCpu != XIH_PH_ATTRB_DEST_CPU_PMU))
	{
		CpuNo = FsblInstancePtr->HandoffCpuNo;
		if (XFsbl_CheckHandoffCpu(FsblInstancePtr,
				DestinationCpu) == XFSBL_SUCCESS)
		{
			/* Get the execution state */
			ExecState = XFsbl_GetA53ExecState(PartitionHeader);
			FsblInstancePtr->HandoffValues[CpuNo].CpuSettings =
					DestinationCpu | ExecState;
			FsblInstancePtr->HandoffValues[CpuNo].HandoffAddress =
					PartitionHeader->DestinationExecutionAddress;
			FsblInstancePtr->HandoffCpuNo += 1U;
		}
	}
#ifdef XFSBL_TPM
	PcrIndex = XFsbl_GetPcrIndex(FsblInstancePtr, PartitionNum);
	if (PcrIndex > 0U) {
		if(PcrIndex != XFSBL_TPM_PL_PCR_INDEX) {
			if (IsChecksumEnabled == FALSE) {
				XFsbl_ShaDigest((u8*)LoadAddress, PartitionHeader->UnEncryptedDataWordLength * 4, PartitionHash,
					XFSBL_HASH_TYPE_SHA3);
			}
		}
		else {
			if (IsAuthenticationEnabled == FALSE) {
				if (IsEncryptionEnabled == TRUE) {
					XFsbl_ShaFinish(NULL, PartitionHash, XFSBL_HASH_TYPE_SHA3);
				}
				else if (IsChecksumEnabled == FALSE) {
					XFsbl_ShaDigest((u8*)LoadAddress,
						PartitionHeader->UnEncryptedDataWordLength * 4U,
						PartitionHash, XFSBL_HASH_TYPE_SHA3);
				}
			}
		}
		Status = XFsbl_TpmMeasurePartition(PcrIndex, PartitionHash);
		if (Status != XFSBL_SUCCESS) {
			goto END;
		}
	}
#endif

END:
	return Status;
}

/*****************************************************************************/
/**
 * This function checks if PMU FW is loaded and gives handoff to PMU Microblaze
 *
 * @param	FsblInstancePtr is pointer to the XFsbl Instance
 *
 * @param	PartitionNum is the partition number of the image
 *
 * @return	None
 *
 *****************************************************************************/
static void XFsbl_CheckPmuFw(const XFsblPs* FsblInstancePtr, u32 PartitionNum)
{

	u32 DestinationCpu;
	u32 DestinationCpuNxt;
	u32 PmuFwLoadDone;
	u32 RegVal;

	DestinationCpu =XFsbl_GetDestinationCpu(
			&FsblInstancePtr->ImageHeader.PartitionHeader[PartitionNum]);

	if (DestinationCpu == XIH_PH_ATTRB_DEST_CPU_PMU) {
		if ((PartitionNum + 1U) <=
				(FsblInstancePtr->
						ImageHeader.ImageHeaderTable.NoOfPartitions-1U)) {
			DestinationCpuNxt = XFsbl_GetDestinationCpu(
					&FsblInstancePtr->
					ImageHeader.PartitionHeader[PartitionNum + 1U]);
			if (DestinationCpuNxt != XIH_PH_ATTRB_DEST_CPU_PMU) {
				/* there is a partition after this but that is not PMU FW */
				PmuFwLoadDone = TRUE;
			}
			else
			{
				PmuFwLoadDone = FALSE;
			}
		}
		else
		{
			/* the current partition is last PMU FW partition */
			PmuFwLoadDone = TRUE;
		}
	}
	else
	{
		PmuFwLoadDone = FALSE;
	}

	/* If all partitions of PMU FW loaded, handoff it to PMU MicroBlaze */
	if (PmuFwLoadDone == TRUE)
	{
		/* Wakeup the processor */
		XFsbl_Out32(PMU_GLOBAL_GLOBAL_CNTRL,
				XFsbl_In32(PMU_GLOBAL_GLOBAL_CNTRL) | 0x1);

		/* wait until done waking up */
		do {
				RegVal = XFsbl_In32(PMU_GLOBAL_GLOBAL_CNTRL);
				if ((RegVal & PMU_GLOBAL_GLOBAL_CNTRL_FW_IS_PRESENT_MASK)
					== PMU_GLOBAL_GLOBAL_CNTRL_FW_IS_PRESENT_MASK) {
					break;
				}
		} while(1);
	}

}

#ifdef XFSBL_BS
/*****************************************************************************/
/** Sets the library firmware state
 *
 * @param	State BS firmware state
 *
 * @return	None
 *****************************************************************************/
static void XFsbl_SetBSSecureState(u32 State)
{
	u32 RegVal;

	/* Set Firmware State in PMU GLOBAL GEN STORAGE Register */
	RegVal = Xil_In32(PMU_GLOBAL_GLOB_GEN_STORAGE5);
	RegVal &= ~XFSBL_STATE_MASK;
	RegVal |= State << XFSBL_STATE_SHIFT;
	Xil_Out32(PMU_GLOBAL_GLOB_GEN_STORAGE5, RegVal);
}
#endif

/*****************************************************************************/
/**
 * This function calculates checksum of the partition.
 *
 * @param	FsblInstancePtr is pointer to the XFsbl Instance
 * @param	LoadAddress Load address of partition
 * @param	PartitionNum is the partition number to calculate checksum
 *
 * @return	returns XFSBL_SUCCESS on success
 * 		returns XFSBL_ERROR_INVALID_CHECKSUM_TYPE on failure
 *
 *****************************************************************************/
static u32 XFsbl_CalculateCheckSum(XFsblPs * FsblInstancePtr,
		PTRSIZE LoadAddress, u32 PartitionNum, u8 * PartitionHash)
{
	u32 Status = XFSBL_FAILURE;
	XFsblPs_PartitionHeader * PartitionHeader =
		&FsblInstancePtr->ImageHeader.PartitionHeader[PartitionNum];
	u32 ChecksumType;
#ifdef XFSBL_PL_LOAD_FROM_OCM
	u32 DestinationDevice = XFsbl_GetDestinationDevice(PartitionHeader);
#ifdef XFSBL_BS
	void * ShaCtx = (void * )NULL;
	u32 PartitionOffset;
	u32 PartitionLen;
#endif
#endif

	/**
	 * Update the variables
	 */
	 ChecksumType = XFsbl_GetChecksumType(PartitionHeader);
#ifdef XFSBL_TPM
	if ((ChecksumType != XIH_PH_ATTRB_HASH_SHA3) &&
		(ChecksumType != XIH_PH_ATTRB_NOCHECKSUM))
#else
	if (ChecksumType != XIH_PH_ATTRB_HASH_SHA3)
#endif
	{
		/* Check sum type is other than SHA3 */
		Status = XFSBL_ERROR_INVALID_CHECKSUM_TYPE;
		goto END;
	}

	XFsbl_Printf(DEBUG_INFO,"CheckSum Type - SHA3\r\n");
#ifdef XFSBL_PL_LOAD_FROM_OCM
	if (DestinationDevice == XIH_PH_ATTRB_DEST_DEVICE_PL)
	{
#ifdef XFSBL_BS
		/**
		 * In case of bitstream in DDR less system, pass the
		 * partition source address from Flash device.
		 */
		PartitionOffset = FsblInstancePtr->ImageOffsetAddress +
			((PartitionHeader->DataWordOffset) *
			XIH_PARTITION_WORD_LENGTH);
		PartitionLen = PartitionHeader->TotalDataWordLength * 4U;

		XFsbl_ShaUpdate_DdrLess(FsblInstancePtr, ShaCtx,
			PartitionOffset, PartitionLen, XFSBL_HASH_TYPE_SHA3,
			PartitionHash);

		XFsbl_ShaFinish(ShaCtx, (u8 *)PartitionHash,
			XFSBL_HASH_TYPE_SHA3);
#endif
	}
	else
	{
	/* SHA calculation for non-bitstream, DDR less partitions */
		XFsbl_CalculateSHA(FsblInstancePtr, LoadAddress, PartitionNum,
			PartitionHash);
	}
#else /* SHA calculation in DDRful systems */
	XFsbl_CalculateSHA(FsblInstancePtr, LoadAddress, PartitionNum,
		PartitionHash);
#endif

	Status = XFSBL_SUCCESS;

END:
	return Status;
}

/*****************************************************************************/
/**
 * This function validates the partition.
 *
 * @param	FsblInstancePtr is pointer to the XFsbl Instance
 * @param	LoadAddress Load address of partition
 * @param	PartitionNum is the partition number to calculate checksum
 *
 * @return	returns XFSBL_SUCCESS on success
 * 			returns XFSBL_FAILURE on failure
 *
 *****************************************************************************/
static u32 XFsbl_ValidateCheckSum(XFsblPs * FsblInstancePtr,
		PTRSIZE LoadAddress, u32 PartitionNum, u8 * PartitionHash)
{
	u32 Status = XFSBL_FAILURE;
	u8 Hash[XFSBL_HASH_TYPE_SHA3] __attribute__ ((aligned (4U))) = {0U};
	const XFsblPs_PartitionHeader * PartitionHeader =
		&FsblInstancePtr->ImageHeader.PartitionHeader[PartitionNum];
	u32 HashOffset = FsblInstancePtr->ImageOffsetAddress +
		PartitionHeader->ChecksumWordOffset * 4U;
	u8 Index;

	Status = FsblInstancePtr->DeviceOps.DeviceCopy(HashOffset,
		(PTRSIZE) Hash, XFSBL_HASH_TYPE_SHA3);
	if (Status != XFSBL_SUCCESS) {
		XFsbl_Printf(DEBUG_GENERAL, "XFSBL_ERROR_HASH_COPY_FAILED\r\n");
		goto END;
	}

	for(Index = 0U; Index < XFSBL_HASH_TYPE_SHA3; Index++)
	{
		if(PartitionHash[Index] != Hash[Index])
		{
			XFsbl_Printf(DEBUG_GENERAL,
				"XFSBL_ERROR_HASH_FAILED\r\n");
			Status = XFSBL_FAILURE;
			goto END;
		}
	}

END:
	return Status;
}

/*****************************************************************************/
/**
 * This function calculates SHA3 digest of the partition.
 *
 * @param	FsblInstancePtr is pointer to the XFsbl Instance
 * @param	LoadAddress Load address of partition
 * @param	PartitionNum is the partition number to calculate checksum
 * @param	Partition Hash points to the SHA3 digest of the partition
 *
 * @return	None
 *
 *****************************************************************************/
static void XFsbl_CalculateSHA(const XFsblPs * FsblInstancePtr,
	PTRSIZE LoadAddress, u32 PartitionNum, u8* PartitionHash)
{
	u32 Length;
	const XFsblPs_PartitionHeader * PartitionHeader;

	PartitionHeader =
	    &FsblInstancePtr->ImageHeader.PartitionHeader[PartitionNum];
	Length = PartitionHeader->TotalDataWordLength * 4U;

	/* Calculate SHA hash */
	XFsbl_ShaDigest((u8*)LoadAddress, Length, PartitionHash,
		XFSBL_HASH_TYPE_SHA3);
}

#ifdef XFSBL_ENABLE_DDR_SR
/*****************************************************************************/
/**
 * This function waits for DDR out of self refresh.
 *
 * @param	None
 *
 * @return	None
 *
 *****************************************************************************/
static void XFsbl_PollForDDRSrExit(void)
{
	u32 RegValue;
	/* Timeout count for around 1 second */
	u32 TimeOut = XPAR_PSU_CORTEXA53_0_CPU_CLK_FREQ_HZ;

	/* Wait for DDR exit from self refresh mode within 1 second */
	while (TimeOut > 0) {
		RegValue = Xil_In32(XFSBL_DDR_STATUS_REGISTER_OFFSET);
		if (!(RegValue & DDR_STATUS_FLAG_MASK)) {
			break;
		}
		TimeOut--;
	}
}

/*****************************************************************************/
/**
 * This function removes reserved mark of DDR once it is out of self refresh.
 *
 * @param	None
 *
 * @return	None
 *
 *****************************************************************************/
static void XFsbl_PollForDDRReady(void)
{
	volatile u32 RegValue;

	RegValue = XFsbl_In32(PMU_GLOBAL_GLOBAL_CNTRL);
	if ((RegValue & PMU_GLOBAL_GLOBAL_CNTRL_FW_IS_PRESENT_MASK)
	    == PMU_GLOBAL_GLOBAL_CNTRL_FW_IS_PRESENT_MASK) {
		/*
		 * PMU firmware is ready. Set flag to indicate that DDR
		 * controller is ready, so that the PMU may bring the DDR out
		 * of self refresh if necessary.
		 */
		RegValue = Xil_In32(XFSBL_DDR_STATUS_REGISTER_OFFSET);
		Xil_Out32(XFSBL_DDR_STATUS_REGISTER_OFFSET, RegValue |
				DDRC_INIT_FLAG_MASK);

		/*
		 * Read PMU register bit value that indicates DDR is in self
		 * refresh mode.
		 */
		RegValue = Xil_In32(XFSBL_DDR_STATUS_REGISTER_OFFSET) &
			DDR_STATUS_FLAG_MASK;
		if (RegValue) {
			/* Wait until DDR exits from self refresh */
			XFsbl_PollForDDRSrExit();
			/*
			 * Mark DDR region as "Memory" as DDR initialization is
			 * done
			 */
			XFsbl_MarkDdrAsReserved(FALSE);
		}
	}
}
#endif

#ifdef ARMR5

/*****************************************************************************/
/**
 * This function set the vector bit of SCTLR.
 * It will configure R5,so that R5 will jump to
 * HIVEC when exception arise.
 *
 * @param	None
 *
 * @return	None
 *
 *****************************************************************************/

static void XFsbl_SetR5ExcepVectorHiVec(void)
{
		u32 RegVal;
		RegVal = mfcp(XREG_CP15_SYS_CONTROL);
		RegVal |= XFSBL_SET_R5_SCTLR_VECTOR_BIT;
		mtcp(XREG_CP15_SYS_CONTROL,RegVal);
}

/*****************************************************************************/
/**
 * This function reset the vector bit of SCTLR.
 * It will configure R5,so that R5 will jump to
 * LOVEC when exception arise.
 *
 * @param	None
 *
 * @return	None
 *
 *****************************************************************************/

static void XFsbl_SetR5ExcepVectorLoVec(void)
{
		u32 RegVal;
		RegVal = mfcp(XREG_CP15_SYS_CONTROL);
		RegVal &= (~(XFSBL_SET_R5_SCTLR_VECTOR_BIT));
		mtcp(XREG_CP15_SYS_CONTROL,RegVal);
}

#endif
#ifdef XFSBL_TPM
/*****************************************************************************/
/**
 * This function returns PCR index of the partition. Its 6 for PL,
 * 2 for ATF and 3 for U-boot.
 *
 * @param	FsblInstancePtr is pointer to the XFsbl Instance
 * @param	PartitionNum is the partition number to measure
 *
 * @return	PcrIndex
 *
 *****************************************************************************/
static u8 XFsbl_GetPcrIndex(const XFsblPs * FsblInstancePtr, u32 PartitionNum)
{
	u8 PcrIndex = 0U;
	const XFsblPs_PartitionHeader * PartitionHeader =
		&FsblInstancePtr->ImageHeader.PartitionHeader[PartitionNum];
	u32 DestinationCpu = XFsbl_GetDestinationCpu(PartitionHeader);
	u32 PartitionAttributes = PartitionHeader->PartitionAttributes;
	u32 DestinationDevice = XFsbl_GetDestinationDevice(PartitionHeader);
	u32 ElFlag = PartitionAttributes & XIH_PH_ATTRB_TARGET_EL_MASK;

	if (DestinationDevice == XIH_PH_ATTRB_DEST_DEVICE_PL)
	{
		PcrIndex = XFSBL_TPM_PL_PCR_INDEX;
	}
	else if (DestinationCpu == XIH_PH_ATTRB_DEST_CPU_A53_0)
	{
		if (ElFlag == XFSBL_EL3_VAL)
		{
			PcrIndex = XFSBL_TPM_ATF_PCR_INDEX;
		}
		else if (ElFlag == XFSBL_EL2_VAL)
		{
			PcrIndex = XFSBL_TPM_UBOOT_PCR_INDEX;
		}
	}

	return PcrIndex;
}
#endif
