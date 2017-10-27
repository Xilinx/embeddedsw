/******************************************************************************
*
* Copyright (C) 2015 - 17 Xilinx, Inc.  All rights reserved.
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
*
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
*                     Modified endianess of IV as APIs are modified in Xilsecure
*                     While loading bitstream clearing of PL is skipped
*                     when PL is already cleared at initialize.
*                     Updated destination cpu for PMUFW.
*       bv   03/20/17 Removed isolation in PS - PL AXI bus thus allowing
*                     access to BRAM in PS only reset
*       vns  04/04/17 Corrected IV location w.r.t Image offset.
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
/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/
#define XFSBL_IVT_LENGTH	(u32)(0x20U)
#define XFSBL_R5_HIVEC    	(u32)(0xffff0000U)
#define XFSBL_R5_LOVEC		(u32)(0x0U)
#define XFSBL_SET_R5_SCTLR_VECTOR_BIT   (u32)(1<<13)

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

#ifdef XFSBL_SECURE
static u32 XFsbl_CalcualteCheckSum(XFsblPs* FsblInstancePtr,
		PTRSIZE LoadAddress, u32 PartitionNum);
static u32 XFsbl_CalcualteSHA(const XFsblPs* FsblInstancePtr,
		PTRSIZE LoadAddress, u32 PartitionNum, u32 ShaType);
#endif

#ifdef ARMR5
static void XFsbl_SetR5ExcepVectorHiVec(void);
static void XFsbl_SetR5ExcepVectorLoVec(void);
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
u8 AuthBuffer[XFSBL_AUTH_BUFFER_SIZE]__attribute__ ((aligned (4))) = {0};
#ifdef XFSBL_BS
u8 HashsOfChunks[HASH_BUFFER_SIZE] __attribute__((section (".bitstream_buffer")));
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
	/* Restart WDT as partition copy can take more time */
	XFsbl_RestartWdt();
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
			 * so that clock propogates properly.
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
			 * so that clock propogates properly.
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
                         * so that clock propogates properly.
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
 * @param	DestinationCpu is the cpu which partition will run
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

#ifndef XFSBL_PS_DDR
			/* In case of DDR less system, skip copying */
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
	u32 Status;
	u32 IsEncryptionEnabled;
	u32 IsAuthenticationEnabled;
	u32 IsChecksumEnabled;
	u32 DestinationDevice;
	u32 DestinationCpu;
	u32 ExecState;
	u32 CpuNo;
	XFsblPs_PartitionHeader * PartitionHeader;

#if defined(XFSBL_SECURE)
	s32 SStatus;
	u32 FsblIv[XIH_BH_IV_LENGTH / 4U] = { 0 };
	u32 UnencryptedLength = 0U;
	u32 IvLocation;
	u32 Length;
	static XSecure_Aes SecureAes;
#ifdef XFSBL_BS
	XFsblPs_PlPartition PlParams = {0};
#endif
#endif
#if defined(XFSBL_SECURE) ||  defined(XFSBL_BS)
	PTRSIZE LoadAddress;
#endif
#if defined(XFSBL_BS) && defined(XFSBL_PS_DDR)
	u32 BitstreamWordSize;
#endif
#if !defined(XFSBL_PS_DDR) &&  defined(XFSBL_BS)
	u32 SrcAddress = 0U;
#endif

#ifdef XFSBL_PERF
	XTime tCur = 0;
#endif

	/**
	 * Update the variables
	 */
	PartitionHeader =
	    &FsblInstancePtr->ImageHeader.PartitionHeader[PartitionNum];
#ifdef XFSBL_SECURE
	Length = PartitionHeader->TotalDataWordLength * 4U;
#endif

	/**
	 * Check the encryption status
	 */
	if (XFsbl_IsEncrypted(PartitionHeader) ==
			XIH_PH_ATTRB_ENCRYPTION )
	{
		IsEncryptionEnabled = TRUE;

#ifdef XFSBL_SECURE
		/* Copy the Iv from Flash into local memory */
		IvLocation = FsblInstancePtr->ImageOffsetAddress +
						XIH_BH_IV_OFFSET;

		Status = FsblInstancePtr->DeviceOps.DeviceCopy(IvLocation,
				(PTRSIZE) FsblIv, XIH_BH_IV_LENGTH);

		if (Status != XFSBL_SUCCESS) {
			XFsbl_Printf(DEBUG_GENERAL,
					"XFSBL_ERROR_DECRYPTION_IV_COPY_FAIL \r\n");
			Status = XFSBL_ERROR_DECRYPTION_IV_COPY_FAIL;
			goto END;
		}
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
	if (XFsbl_IsRsaSignaturePresent(PartitionHeader) ==
			XIH_PH_ATTRB_RSA_SIGNATURE )
	{
		IsAuthenticationEnabled = TRUE;
	}
	else
	{
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
#if defined(XFSBL_SECURE) ||  defined(XFSBL_BS)
	LoadAddress = (PTRSIZE) PartitionHeader->DestinationLoadAddress;
#endif

#ifdef XFSBL_SECURE
	if ((IsAuthenticationEnabled == TRUE) || (IsEncryptionEnabled == TRUE) ||
			(IsChecksumEnabled == TRUE))
	{
		Length = PartitionHeader->TotalDataWordLength * 4U;
		Status = XFsbl_GetLoadAddress(DestinationCpu,
				&LoadAddress, Length);
		if (XFSBL_SUCCESS != Status)
		{
			goto END;
		}
	}
#endif

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
#if !defined(XFSBL_PS_DDR) &&  defined(XFSBL_BS)
	SrcAddress = FsblInstancePtr->ImageOffsetAddress +
					((PartitionHeader->DataWordOffset) *
						XIH_PARTITION_WORD_LENGTH);
#endif

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
	if (IsChecksumEnabled == TRUE)
	{
#ifdef XFSBL_SECURE
		Status = XFsbl_CalcualteCheckSum(FsblInstancePtr,
				LoadAddress, PartitionNum);
		if (Status != XFSBL_SUCCESS) {
			XFsbl_Printf(DEBUG_GENERAL,
					"XFSBL_ERROR_PARTITION_CHECKSUM_FAILED \r\n");
			Status = XFSBL_ERROR_PARTITION_CHECKSUM_FAILED;
			goto END;
		}
#else
		XFsbl_Printf(DEBUG_GENERAL,"XFSBL_ERROR_SECURE_NOT_ENABLED \r\n");
		Status = XFSBL_ERROR_SECURE_NOT_ENABLED;
		goto END;
#endif
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
				PlParams.PlAuth.AuthType = XFSBL_HASH_TYPE_SHA2;
				PlParams.PlAuth.NoOfHashs =
					HASH_BUFFER_SIZE/XFSBL_HASH_TYPE_SHA2;
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

#ifdef XFSBL_PS_DDR
			PlParams.DeviceCopy = NULL;
			PlParams.StartAddress = LoadAddress;
			PlParams.PlAuth.AcOfset = LoadAddress +
				((PartitionHeader->AuthCertificateOffset * 4) -
				(PartitionHeader->DataWordOffset *
						XIH_PARTITION_WORD_LENGTH));
#else
			PlParams.DeviceCopy =
				FsblInstancePtr->DeviceOps.DeviceCopy;
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
			/*
			 * The secure bitstream would be sent through CSU DMA to AES
			 * and the decrypted bitstream is sent directly to PCAP
			 * by configuring SSS appropriately
			 */
#ifdef XFSBL_PS_DDR

			Status = (u32)XSecure_AesDecrypt(&SecureAes,
					(u8 *) XFSBL_DESTINATION_PCAP_ADDR,
					(u8 *) LoadAddress, UnencryptedLength);
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

			/**
			 * In case of DDR less system, pass the partition source
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
				/* Reset PL */
				XFsbl_Out32(CSU_PCAP_PROG, 0x0);
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

#ifdef XFSBL_PS_DDR
			/* Use CSU DMA to load Bit stream to PL */
			BitstreamWordSize =
				PartitionHeader->UnEncryptedDataWordLength;

			Status = XFsbl_WriteToPcap(BitstreamWordSize, (u8 *) LoadAddress);
			if (Status != XFSBL_SUCCESS) {
				goto END;
			}
#else
			/* In case of DDR less system, do the chunked transfer */
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

#ifdef XFSBL_SECURE
/*****************************************************************************/
/**
 * This function validates the partition
 *
 * @param	FsblInstancePtr is pointer to the XFsbl Instance
 *
 * @param	LoadAddress Load address of partition
 *
 * @param	PartitionNum is the partition number to calculate checksum
 *
 * @return	returns XFSBL_SUCCESS on success
 * 			returns XFSBL_FAILURE on failure
 *
 *****************************************************************************/

static u32 XFsbl_CalcualteCheckSum(XFsblPs * FsblInstancePtr,
		PTRSIZE LoadAddress, u32 PartitionNum)
{

	XFsblPs_PartitionHeader * PartitionHeader;
	u32 Status;
	u32 ChecksumType;
	/**
	 * Update the variables
	 */
	PartitionHeader =
	    &FsblInstancePtr->ImageHeader.PartitionHeader[PartitionNum];
	ChecksumType = XFsbl_GetChecksumType(PartitionHeader);


	/**
	 * Checksum Validation
	 */
	if (ChecksumType == XIH_PH_ATTRB_HASH_SHA3)
	{
		XFsbl_Printf(DEBUG_INFO,"CheckSum Type - SHA3\r\n");
	#ifndef XFSBL_PS_DDR
		u32 DestinationDevice = XFsbl_GetDestinationDevice(PartitionHeader);
		if (DestinationDevice == XIH_PH_ATTRB_DEST_DEVICE_PL)
		{
	#ifdef XFSBL_BS
			void * ShaCtx = (void * )NULL;
			u8  PartitionHash[XFSBL_HASH_TYPE_SHA3] __attribute__ ((aligned (4)));
			u8  Hash[XFSBL_HASH_TYPE_SHA3] __attribute__ ((aligned (4)));
			u32 PartitionOffset;
			u32 PartitionLen;
			u32 HashOffset;
			u32 Index;

			HashOffset = FsblInstancePtr->ImageOffsetAddress + PartitionHeader->ChecksumWordOffset * 4U;
			/**
			 * In case of bitstream in DDR less system, pass the
			 * partition source address from Flash device.
			 */
			PartitionOffset = FsblInstancePtr->ImageOffsetAddress +
					((PartitionHeader->DataWordOffset) *
					XIH_PARTITION_WORD_LENGTH);
			PartitionLen = PartitionHeader->TotalDataWordLength * 4U;

			XFsbl_ShaUpdate_DdrLess(FsblInstancePtr, ShaCtx,
					PartitionOffset, PartitionLen,
					XFSBL_HASH_TYPE_SHA3, PartitionHash);

			XFsbl_ShaFinish(ShaCtx, (u8 *)PartitionHash, XFSBL_HASH_TYPE_SHA3);

			Status = FsblInstancePtr->DeviceOps.DeviceCopy(HashOffset,
						(PTRSIZE) Hash, XFSBL_HASH_TYPE_SHA3);

				if (Status != XFSBL_SUCCESS) {
					XFsbl_Printf(DEBUG_GENERAL,
						"XFSBL_ERROR_HASH_COPY_FAILED DDR LESS SYSTEM\r\n");
					return XFSBL_FAILURE;
				}

			for(Index=0;Index<XFSBL_HASH_TYPE_SHA3;Index++)
			{
				if(PartitionHash[Index]!= Hash[Index])
				{
					XFsbl_Printf(DEBUG_GENERAL,
							"XFSBL_ERROR_HASH_FAILED - DDR LESS SYSTEM\r\n");
					return XFSBL_FAILURE;
				}
			}

	#endif
		}
		else
		{
		/* SHA calculation for non-bitstream, DDR less partitions */
			Status = XFsbl_CalcualteSHA(FsblInstancePtr, LoadAddress,
					PartitionNum, XFSBL_HASH_TYPE_SHA3);
			if (Status != XFSBL_SUCCESS) {
				return XFSBL_FAILURE;
			}
		}
	#else /* SHA calculation in DDRful systems */
		Status = XFsbl_CalcualteSHA(FsblInstancePtr, LoadAddress,
				PartitionNum, XFSBL_HASH_TYPE_SHA3);
		if (Status != XFSBL_SUCCESS) {
			return XFSBL_FAILURE;
		}
	#endif
	}
	else
	{
		/* Check sum type is other than SHA3 */
		Status = XFSBL_ERROR_INVALID_CHECKSUM_TYPE;
	}
	return Status;
}

/*****************************************************************************/
/**
 * This function validates the partition
 *
 * @param	FsblInstancePtr is pointer to the XFsbl Instance
 *
 * @param	LoadAddress Load address of partition
 *
 * @param	PartitionNum is the partition number to calculate checksum
 *
 * @param	ShaType is either SHA2/SHA3
 *
 * @return	returns XFSBL_SUCCESS on success
 * 			returns XFSBL_FAILURE on failure
 *
 *****************************************************************************/

static u32 XFsbl_CalcualteSHA(const XFsblPs * FsblInstancePtr, PTRSIZE LoadAddress,
						u32 PartitionNum, u32 ShaType)
{
	u8 PartitionHash[XFSBL_HASH_TYPE_SHA3] __attribute__ ((aligned (4))) = {0};
	u8 Hash[XFSBL_HASH_TYPE_SHA3] __attribute__ ((aligned (4))) = {0};
	u32 HashOffset;
	u32 Index;
	void * ShaCtx = (void * )NULL;
	u32 Length;
	u32 Status;
	const XFsblPs_PartitionHeader * PartitionHeader;

	/**
	 * Update the variables
	 */
	PartitionHeader =
	    &FsblInstancePtr->ImageHeader.PartitionHeader[PartitionNum];
	Length = PartitionHeader->TotalDataWordLength * 4U;
	HashOffset = FsblInstancePtr->ImageOffsetAddress + PartitionHeader->ChecksumWordOffset * 4U;

	/* Start the SHA engine */
	XFsbl_ShaStart(ShaCtx, ShaType);
	XFsbl_ShaDigest((u8*)LoadAddress,Length, PartitionHash, ShaType);
	Status = FsblInstancePtr->DeviceOps.DeviceCopy(HashOffset,
			(PTRSIZE) Hash, ShaType);

	if (Status != XFSBL_SUCCESS) {
		XFsbl_Printf(DEBUG_GENERAL,
				"XFSBL_ERROR_HASH_COPY_FAILED \r\n");
		return XFSBL_FAILURE;
	}

	for (Index = 0U; Index < ShaType; Index++)
	{
		if(PartitionHash[Index]!= Hash[Index])
		{
			XFsbl_Printf(DEBUG_GENERAL,
					"XFSBL_ERROR_HASH_FAILED \r\n");
			return XFSBL_FAILURE;
		}
	}
	return Status;
}
#endif  /* end of XFSBL_SECURE */

#ifdef ARMR5

/*****************************************************************************/
/**
 * This function set the vector bit of SCTLR.
 * It will configure R5,so that R5 will jump to
 * HIVEC when exeption arise.
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
 * LOVEC when exeption arise.
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
