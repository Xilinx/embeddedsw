/******************************************************************************
* Copyright (c) 2015 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2023 Advanced Micro Devices, Inc. All rights reserved.
*
* SPDX-License-Identifier: MIT
******************************************************************************/

/****************************************************************************/
/**
*
* @file xil_exception.c
*
* This file contains low-level driver functions for the Cortex A53,A9,R5 exception
* Handler.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who      Date     Changes
* ----- -------- -------- -----------------------------------------------
* 5.2	pkp  	 28/05/15 First release
* 6.0   mus      27/07/16 Consolidated exceptions for a53,a9 and r5
*                         processors and added Xil_UndefinedExceptionHandler
*                         for a53 32 bit and r5 as well.
* 6.4   mus      08/06/17 Updated debug prints to replace %x with the %lx, to
*                         fix the warnings.
* 6.7   mna      26/04/18 Add an API to obtain a corresponding
*                         Xil_ExceptionHandler entry from XExc_VectorTable.
* 6.7  asa       18/05/18 Fix bugs in the API Xil_GetExceptionRegisterHandler.
* 7.0  mus       07/03/19 Tweak Xil_ExceptionRegisterHandler and
*                         Xil_GetExceptionRegisterHandler to support legacy
*                         examples for Cortexa72 EL3 exception level.
* 7.3  mus       07/27/20 Updated Xil_ExceptionRegisterHandler and
*                         Xil_GetExceptionRegisterHandler to ignore
*                         Exception_id, only if its pointing to IRQ.
*                         It fixes CR#1069524
* 8.0  mus       02/24/22 Updated Xil_ExceptionRegisterHandler and
*                         Xil_GetExceptionRegisterHandler to support legacy
*                         driver examples for CortexR52. This is needed, as
*                         by default scugic driver configures interrupts as
*                         group0, and CortexR52 GIC triggers FIQ for group0
*                         interrupts.
* 8.0  sk	 03/02/22 Move XExc_VectorTableEntry structure to header
* 			  file to fix misra_c_2012_rule_5_6 violation.
* 8.1  asa       02/12/23 Updated data abort and prefetch abort fault
*                         status reporting for ARMv7.
*						  Updated Sync and SError fault status reporting
*						  for ARMv8.
*
*
* </pre>
*
*****************************************************************************/

/***************************** Include Files ********************************/

#include "xil_types.h"
#include "xil_assert.h"
#include "xil_exception.h"
#include "xpseudo_asm.h"
#include "xdebug.h"
#include "bspconfig.h"
/************************** Constant Definitions ****************************/

/**************************** Type Definitions ******************************/

/***************** Macros (Inline Functions) Definitions ********************/
#ifdef DEBUG
#if defined (__aarch64__)
static u32 NotifyFaultStatusDetails(u32 Fault_Type, u64 FaultStatus);
static void DecodeSyncAbortWithIss(u32 Type, u64 IssVal);
static void DecodeSErrorWithIss(u64 IssVal);
#else
static u32 NotifyFaultStatusDetails(u32 Fault_Type, u32 FaultStatus);
#endif
#endif
/************************** Function Prototypes *****************************/
static void Xil_ExceptionNullHandler(void *Data);
#if defined (__aarch64__)
static void Xil_SyncErrorHandler(void *CallBackRef);
static void Xil_SErrorHandler(void *CallBackRef);
#else
static void Xil_DataAbortHandler(void *CallBackRef);
static void Xil_PrefetchAbortHandler(void *CallBackRef);
static void Xil_UndefinedExceptionHandler(void *CallBackRef);
#endif
/************************** Variable Definitions *****************************/
/*
 * Exception vector table to store handlers for each exception vector.
 */
#if defined (__aarch64__)
XExc_VectorTableEntry XExc_VectorTable[XIL_EXCEPTION_ID_LAST + 1] =
{
        {Xil_ExceptionNullHandler, NULL},
        {Xil_SyncErrorHandler, NULL},
        {Xil_ExceptionNullHandler, NULL},
        {Xil_ExceptionNullHandler, NULL},
        {Xil_SErrorHandler, NULL},

};
#else
XExc_VectorTableEntry XExc_VectorTable[XIL_EXCEPTION_ID_LAST + 1] =
{
	{Xil_ExceptionNullHandler, NULL},
	{Xil_UndefinedExceptionHandler, NULL},
	{Xil_ExceptionNullHandler, NULL},
	{Xil_PrefetchAbortHandler, NULL},
	{Xil_DataAbortHandler, NULL},
	{Xil_ExceptionNullHandler, NULL},
	{Xil_ExceptionNullHandler, NULL},
};
#endif /* #if defined (__aarch64__) */
#if !defined (__aarch64__)
u32 DataAbortAddr;       /* Address of instruction causing data abort */
u32 PrefetchAbortAddr;   /* Address of instruction causing prefetch abort */
u32 UndefinedExceptionAddr;   /* Address of instruction causing Undefined
							     exception */
#endif

/*****************************************************************************/

/****************************************************************************/
/**
*
* This function is a stub Handler that is the default Handler that gets called
* if the application has not setup a Handler for a specific  exception. The
* function interface has to match the interface specified for a Handler even
* though none of the arguments are used.
*
* @param	Data is unused by this function.
*
* @return	None.
*
*****************************************************************************/
static void Xil_ExceptionNullHandler(void *Data)
{
	(void) Data;
DieLoop: goto DieLoop;
}

/****************************************************************************/
/**
* @brief	The function is a common API used to initialize exception handlers
*			across all supported arm processors. For ARM Cortex-A53, Cortex-R5,
*			and Cortex-A9, the exception handlers are being initialized
*			statically and this function does not do anything.
* 			However, it is still present to take care of backward compatibility
*			issues (in earlier versions of BSPs, this API was being used to
*			initialize exception handlers).
*
* @return	None.
*
*****************************************************************************/
void Xil_ExceptionInit(void)
{
	return;
}

/*****************************************************************************/
/**
* @brief	Register a handler for a specific exception. This handler is being
*			called when the processor encounters the specified exception.
*
* @param	Exception_id contains the ID of the exception source and should
*			be in the range of 0 to XIL_EXCEPTION_ID_LAST.
*			See xil_exception.h for further information.
* @param	Handler to the Handler for that exception.
* @param	Data is a reference to Data that will be passed to the
*			Handler when it gets called.
*
* @return	None.
*
****************************************************************************/
void Xil_ExceptionRegisterHandler(u32 Exception_id,
				    Xil_ExceptionHandler Handler,
				    void *Data)
{
#if (defined (versal) && !defined(ARMR5) && EL3) || defined(ARMR52)
	if ( XIL_EXCEPTION_ID_IRQ_INT == Exception_id )
	{
	/*
	 * Cortexa72 processor in versal is coupled with GIC-500, and
	 * GIC-500 supports only FIQ at EL3. Hence, tweaking this API
	 * to act on IRQ, if Exception_id is pointing to IRQ
	 */
		Exception_id = XIL_EXCEPTION_ID_FIQ_INT;
	}
#endif
	XExc_VectorTable[Exception_id].Handler = Handler;
	XExc_VectorTable[Exception_id].Data = Data;
}

/*****************************************************************************/
/**
* @brief	Get a handler for a specific exception. This handler is being
*			called when the processor encounters the specified exception.
*
* @param	Exception_id contains the ID of the exception source and should
*			be in the range of 0 to XIL_EXCEPTION_ID_LAST.
*			See xil_exception.h for further information.
* @param	Handler to the Handler for that exception.
* @param	Data is a reference to Data that will be passed to the
*			Handler when it gets called.
*
* @return	None.
*
****************************************************************************/
void Xil_GetExceptionRegisterHandler(u32 Exception_id,
					Xil_ExceptionHandler *Handler,
					void **Data)
{
#if (defined (versal) && !defined(ARMR5) && EL3) || defined(ARMR52)
	if ( XIL_EXCEPTION_ID_IRQ_INT == Exception_id )
	{
	/*
	 * Cortexa72 processor in versal is coupled with GIC-500, and
	 * GIC-500 supports only FIQ at EL3. Hence, tweaking this API
	 * to act on IRQ, if Exception_id is pointing to IRQ
	 */

		Exception_id = XIL_EXCEPTION_ID_FIQ_INT;
	}
#endif

	*Handler = XExc_VectorTable[Exception_id].Handler;
	*Data = XExc_VectorTable[Exception_id].Data;
}

/*****************************************************************************/
/**
*
* @brief	Removes the Handler for a specific exception Id. The stub Handler
*			is then registered for this exception Id.
*
* @param	Exception_id contains the ID of the exception source and should
*			be in the range of 0 to XIL_EXCEPTION_ID_LAST.
*			See xil_exception.h for further information.
*
* @return	None.
*
****************************************************************************/
void Xil_ExceptionRemoveHandler(u32 Exception_id)
{
	Xil_ExceptionRegisterHandler(Exception_id,
				       Xil_ExceptionNullHandler,
				       NULL);
}

#if defined (__aarch64__)
/*****************************************************************************/
/**
*
* Default Synchronous abort handler which prints a debug message on console if
* Debug flag is enabled
*
* @return       None.
*
****************************************************************************/

static void Xil_SyncErrorHandler(void *CallBackRef){
	(void) CallBackRef;

#ifdef __GNUC__
#ifdef DEBUG
	volatile u64 EsrVal;
	volatile u64 ElrVal;
	volatile u64 FarVal;
	u32 Far_Status;
#if (EL3 == 1)
	EsrVal = mfesrel3();
	ElrVal = mfelrel3();
#else
	EsrVal = mfesrel1();
	ElrVal = mfelrel1();
#endif
	xdbg_exception_printf(XDBG_DEBUG_ERROR,
		"-------------------------------------------------------------\n");
	Far_Status = NotifyFaultStatusDetails(ARMV8_SYNC_ERROR, EsrVal);
	xdbg_exception_printf(XDBG_DEBUG_ERROR,
		"-------------------------------------------------------------\n");
	xdbg_exception_printf(XDBG_DEBUG_ERROR,
		"Address of Instruction causing Sync error is: 0x%lx\n", ElrVal);
	if (Far_Status) {
#if (EL3 == 1)
		FarVal = mffarel3();
#else
		FarVal = mffarel1();
#endif
		xdbg_exception_printf(XDBG_DEBUG_ERROR,
		"Address of Memory whose access caused the Sync Error is: 0x%lx\n",
		FarVal);
	}
	xdbg_exception_printf(XDBG_DEBUG_ERROR,
		"-------------------------------------------------------------\n");
#endif
#else
	xdbg_printf(XDBG_DEBUG_ERROR, "Synchronous Error \n");
#endif
	while(1) {
		;
	}
}

/*****************************************************************************/
/**
*
* Default SError abort handler which prints a debug message on console if
* Debug flag is enabled
*
* @return       None.
*
****************************************************************************/
static void Xil_SErrorHandler(void *CallBackRef){
	(void) CallBackRef;

#ifdef __GNUC__
#ifdef DEBUG
	volatile u64 EsrVal;
#if (EL3 == 1)
	EsrVal = mfesrel3();
#else
	EsrVal = mfesrel1();
#endif
	xdbg_exception_printf(XDBG_DEBUG_ERROR,
		"-------------------------------------------------------------\n");
	(void)NotifyFaultStatusDetails(ARMV8_SERROR, EsrVal);
	xdbg_exception_printf(XDBG_DEBUG_ERROR,
		"-------------------------------------------------------------\n");
#endif
#else
	xdbg_printf(XDBG_DEBUG_ERROR, "Asynchronous SError \n");
#endif

	while(1) {
		;
	}
}

#if defined (__aarch64__)
#ifdef DEBUG
static u32 NotifyFaultStatusDetails(u32 Fault_Type, u64 EsrVal)
{
	u32 Far_Status = 0x0;
	u64 EcVal = ARMV8_EXTRACT_ESR_EC(EsrVal);
	u64 IssVal = ARMV8_EXTRACT_ESR_ISS(EsrVal);

	if (Fault_Type == ARMV8_SYNC_ERROR) {
		xdbg_exception_printf(XDBG_DEBUG_ERROR,
				"Sync Error with ESR = 0x%lx, EC = 0x%lx, ISS = 0x%lx \n",
				EsrVal, EcVal, IssVal);

		xdbg_exception_printf(XDBG_DEBUG_ERROR,
			"-------------------------------------------------------------\n");

		xdbg_exception_printf(XDBG_DEBUG_ERROR,
				"Exception Type based on Exception Class (EC val) : ");
		switch (EcVal) {
			case ARMV8_ESR_EC_UNKNOWN_ERR:
				xdbg_exception_printf(XDBG_DEBUG_ERROR, "Unknown Error\n");
				break;
			case ARMV8_ESR_EC_FP_ASIMD:
				xdbg_exception_printf(XDBG_DEBUG_ERROR,
				"Access to ASIMD or floating point functionality based "
				"on CPACR_EL1 FPEN configuration\n");
				Far_Status = ARMV8_FAR_VALUE_VALID;
				break;
			case ARMV8_ESR_EC_DATA_ABORT_LOWER:
				xdbg_exception_printf(XDBG_DEBUG_ERROR, "Data Abort From a Lower "
						"Exception Level\n");
				xdbg_exception_printf(XDBG_DEBUG_ERROR,
					"Exception Details based on Instruction Specific Syndrome "
						"(ISS val) : ");
				DecodeSyncAbortWithIss(ARMV8_ESR_EC_DATA_ABORT, IssVal);
				Far_Status = ARMV8_FAR_VALUE_VALID;
				break;
			case ARMV8_ESR_EC_DATA_ABORT:
				xdbg_exception_printf(XDBG_DEBUG_ERROR, "Data Abort \n");
				xdbg_exception_printf(XDBG_DEBUG_ERROR,
					"Exception Details based on Instruction Specific Syndrome "
					"(ISS val) : ");
				DecodeSyncAbortWithIss(ARMV8_ESR_EC_DATA_ABORT, IssVal);
				Far_Status = ARMV8_FAR_VALUE_VALID;
				break;
			case ARMV8_ESR_EC_INS_ABORT_LOWER:
				xdbg_exception_printf(XDBG_DEBUG_ERROR,
						"Instruction Abort Taken From a Lower Exception Level\n");
				xdbg_exception_printf(XDBG_DEBUG_ERROR,
					"Exception Details based on Instruction Specific Syndrome "
					"(ISS val) : ");
				DecodeSyncAbortWithIss(ARMV8_ESR_EC_INS_ABORT, IssVal);
				Far_Status = ARMV8_FAR_VALUE_VALID;
				break;
			case ARMV8_ESR_EC_INS_ABORT:
				xdbg_exception_printf(XDBG_DEBUG_ERROR,
						"Instruction Abort\n");
				xdbg_exception_printf(XDBG_DEBUG_ERROR,
					"Exception Details based on Instruction Specific Syndrome "
					"(ISS val) : ");
				DecodeSyncAbortWithIss(ARMV8_ESR_EC_INS_ABORT, IssVal);
				Far_Status = ARMV8_FAR_VALUE_VALID;
				break;
			case ARMV8_ESR_EC_PC_ALIGNMENT_FAULT:
				xdbg_exception_printf(XDBG_DEBUG_ERROR,
						"PC Alignment Fault\n");
				Far_Status = ARMV8_FAR_VALUE_VALID;
				break;
			case ARMV8_ESR_EC_SP_ALIGNMENT_FAULT:
				xdbg_exception_printf(XDBG_DEBUG_ERROR,
						"SP Alignment Fault\n");
				break;
			case ARMV8_ESR_ILL_EXECUTION_STATE:
				xdbg_exception_printf(XDBG_DEBUG_ERROR,
						"Illegal Execution State\n");
				break;
			default:
				xdbg_exception_printf(XDBG_DEBUG_ERROR, "Other error\n");
				break;
		}
	} else {
		xdbg_exception_printf(XDBG_DEBUG_ERROR,
			"Asynchronous SError with ESR = 0x%lx, EC = 0x%lx, ISS = 0x%lx \n",
						EsrVal, EcVal, IssVal);

		xdbg_exception_printf(XDBG_DEBUG_ERROR,
			"-------------------------------------------------------------\n");
		xdbg_exception_printf(XDBG_DEBUG_ERROR,
			"Exception Details (based on Instruction Specific Syndrome "
			"(ISS val) : ");

		DecodeSErrorWithIss(IssVal);
	}
	return Far_Status;
}

static void DecodeSyncAbortWithIss(u32 Type, u64 IssVal)
{
	u64 FscVal;

	FscVal = IssVal & ARMV8_ESR_ISS_FSC;
	switch (FscVal) {
		case ARMV8_LEVEL_0_ADDR_FAULT:
			xdbg_exception_printf(XDBG_DEBUG_ERROR,
				"Address Size fault at Level 0 translation \n");
			break;
		case ARMV8_LEVEL_1_ADDR_FAULT:
			xdbg_exception_printf(XDBG_DEBUG_ERROR,
				"Address Size fault at Level 1 translation \n");
			break;
		case ARMV8_LEVEL_2_ADDR_FAULT:
			xdbg_exception_printf(XDBG_DEBUG_ERROR,
				"Address Size fault at Level 2 translation \n");
			break;
		case ARMV8_LEVEL_3_ADDR_FAULT:
			xdbg_exception_printf(XDBG_DEBUG_ERROR,
				"Address Size fault at Level 3 translation \n");
			break;
		case ARMV8_LEVEL_0_TRANS_FAULT:
			xdbg_exception_printf(XDBG_DEBUG_ERROR,
				"Translation fault at Level 0 translation \n");
			break;
		case ARMV8_LEVEL_1_TRANS_FAULT:
			xdbg_exception_printf(XDBG_DEBUG_ERROR,
				"Translation fault at Level 1 translation \n");
			break;
		case ARMV8_LEVEL_2_TRANS_FAULT:
			xdbg_exception_printf(XDBG_DEBUG_ERROR,
				"Translation fault at Level 2 translation \n");
			break;
		case ARMV8_LEVEL_3_TRANS_FAULT:
			xdbg_exception_printf(XDBG_DEBUG_ERROR,
				"Translation fault at Level 3 translation \n");
			break;
		case ARMV8_LEVEL_0_ACCS_FLAG_FAULT:
			xdbg_exception_printf(XDBG_DEBUG_ERROR,
				"Access Flag Fault at Level 0 translation \n");
			break;
		case ARMV8_LEVEL_1_ACCS_FLAG_FAULT:
			xdbg_exception_printf(XDBG_DEBUG_ERROR,
				"Access Flag Fault at Level 1 translation \n");
			break;
		case ARMV8_LEVEL_2_ACCS_FLAG_FAULT:
			xdbg_exception_printf(XDBG_DEBUG_ERROR,
				"Access Flag Fault at Level 2 translation \n");
			break;
		case ARMV8_LEVEL_3_ACCS_FLAG_FAULT:
			xdbg_exception_printf(XDBG_DEBUG_ERROR,
				"Access Flag Fault at Level 3 translation \n");
			break;
		case ARMV8_LEVEL_0_PERMISSION_FAULT:
			xdbg_exception_printf(XDBG_DEBUG_ERROR,
				"Permission Fault at Level 0 translation \n");
			break;
		case ARMV8_LEVEL_1_PERMISSION_FAULT:
			xdbg_exception_printf(XDBG_DEBUG_ERROR,
				"Permission Fault at Level 1 translation \n");
			break;
		case ARMV8_LEVEL_2_PERMISSION_FAULT:
			xdbg_exception_printf(XDBG_DEBUG_ERROR,
				"Permission Fault at Level 2 translation \n");
			break;
		case ARMV8_LEVEL_3_PERMISSION_FAULT:
			xdbg_exception_printf(XDBG_DEBUG_ERROR,
				"Permission Fault at Level 3 translation \n");
			break;

		case ARMV8_SYNC_EXT_ABORT_NOT_ON_TTW:
			xdbg_exception_printf(XDBG_DEBUG_ERROR,
				"Synchronous External Abort, not on translation table walk or "
				"hw update of translation table \n");
			break;
		case ARMV8_LEVEL_0_SYNC_EXT_ABORT:
			xdbg_exception_printf(XDBG_DEBUG_ERROR,
				"Synchronous External Abort at Level 0 translation on "
				"translation table walk or hw update of translation table \n");
			break;
		case ARMV8_LEVEL_1_SYNC_EXT_ABORT:
			xdbg_exception_printf(XDBG_DEBUG_ERROR,
				"Synchronous External Abort at Level 1 translation on "
				"translation table walk or hw update of translation table \n");
			break;
		case ARMV8_LEVEL_2_SYNC_EXT_ABORT:
			xdbg_exception_printf(XDBG_DEBUG_ERROR,
				"Synchronous External Abort at Level 2 translation on "
				"translation table walk or hw update of translation table \n");
			break;
		case ARMV8_LEVEL_3_SYNC_EXT_ABORT:
			xdbg_exception_printf(XDBG_DEBUG_ERROR,
				"Synchronous External Abort at Level 3 translation on "
				"translation table walk or hw update of translation table \n");
			break;
		case ARMV8_SYNC_PAR_OR_ECC_ERROR_NOT_ON_TTW:
			xdbg_exception_printf(XDBG_DEBUG_ERROR,
				"Synchronous Parity or ECC error for memory access, "
				"not on translation table walk \n");
			break;
		case ARMV8_LEVEL_0_SYNC_PAR_OR_ECC_ERROR:
			xdbg_exception_printf(XDBG_DEBUG_ERROR,
				"Synchronous Parity or ECC error for memory access, "
				"for level 0 translation \n");
			break;
		case ARMV8_LEVEL_1_SYNC_PAR_OR_ECC_ERROR:
			xdbg_exception_printf(XDBG_DEBUG_ERROR,
				"Synchronous Parity or ECC error for memory access, "
				"for level 1 translation \n");
			break;
		case ARMV8_LEVEL_2_SYNC_PAR_OR_ECC_ERROR:
			xdbg_exception_printf(XDBG_DEBUG_ERROR,
				"Synchronous Parity or ECC error for memory access, "
				"for level 2 translation \n");
			break;
		case ARMV8_LEVEL_3_SYNC_PAR_OR_ECC_ERROR:
			xdbg_exception_printf(XDBG_DEBUG_ERROR,
				"Synchronous Parity or ECC error for memory access, "
				"for level 3 translation \n");
			break;
		case ARMV8_ALIGNMENT_FAULT:
			if (Type == ARMV8_ESR_EC_DATA_ABORT) {
				xdbg_exception_printf(XDBG_DEBUG_ERROR,
					"Alignment Fault \n");
			} else {
				xdbg_exception_printf(XDBG_DEBUG_ERROR,
					"Unknown fault \n");
			}
			break;
		case ARMV8_TLB_CONFLICT_ABORT:
			xdbg_exception_printf(XDBG_DEBUG_ERROR,
					"TLB Conflict Abort \n");
			break;
		default:
			xdbg_exception_printf(XDBG_DEBUG_ERROR,
					"Unknown fault \n");
	}

	if (Type == ARMV8_ESR_EC_DATA_ABORT) {
		if (IssVal & ARMV8_ESR_ISS_WNR_MASK) {
			xdbg_exception_printf(XDBG_DEBUG_ERROR,
				"Data abort caused by instruction writing to a memory location.\n ");
		} else {
			xdbg_exception_printf(XDBG_DEBUG_ERROR,
				"Data abort caused by instruction reading from a memory location.\n");
		}
		if (IssVal & ARMV8_ESR_ISS_CM_MASK) {
			xdbg_exception_printf(XDBG_DEBUG_ERROR,
				"Data abort caused by a cache maintenance instruction  \n");
		}
	}
}

static void DecodeSErrorWithIss(u64 IssVal)
{
	u64 FscVal;
	u64 MaskVal;

	FscVal = IssVal & ARMV8_ESR_ISS_FSC;
	if (FscVal == ARMV8_ASYNC_SEEROR_INTERRUPT) {
		xdbg_exception_printf(XDBG_DEBUG_ERROR,
			"SError caused by Asynchronous Serror Interrupt \n ");

		MaskVal = IssVal & ARMV8_ESR_ISS_AET_MASK;
		switch (MaskVal) {
			case ARMV8_ESR_ISS_AET_UC:
				xdbg_exception_printf(XDBG_DEBUG_ERROR,
					"Uncontainable Error State \n ");
				break;
			case ARMV8_ESR_ISS_AET_UEU:
				xdbg_exception_printf(XDBG_DEBUG_ERROR,
					"Unrecoverable Error State \n ");
				break;
			case ARMV8_ESR_ISS_AET_UEO:
				xdbg_exception_printf(XDBG_DEBUG_ERROR,
					"Restartable Error State \n ");
				break;
			case ARMV8_ESR_ISS_AET_UER:
				xdbg_exception_printf(XDBG_DEBUG_ERROR,
					"Recoverable Error State \n ");
				break;
			case ARMV8_ESR_ISS_AET_CE:
				xdbg_exception_printf(XDBG_DEBUG_ERROR,
					"Corrected Error State \n ");
				break;
			default:
				break;

		}

	} else {
		xdbg_exception_printf(XDBG_DEBUG_ERROR,
				"SError caused by uncategorized error \n ");
	}
}


#endif /* #ifdef DEBUG */
#endif /* #if defined (__aarch64__) */

#else /* #if defined (__aarch64__) */
/*****************************************************************************/
/**
*
* Default Data abort handler which prints data fault status register through
* which information about data fault can be acquired
*
* @return	None.
*
****************************************************************************/

static void Xil_DataAbortHandler(void *CallBackRef){
	(void) CallBackRef;
#ifdef DEBUG
	u32 FaultStatus;
#ifdef __GNUC__
	u32 DataAbortMemAddr;
	u32 DfarStatus;
#endif

#ifdef __GNUC__
	xdbg_exception_printf(XDBG_DEBUG_ERROR,
		"-------------------------------------------------------------\n");
	FaultStatus = mfcp(XREG_CP15_DATA_FAULT_STATUS);
#elif defined (__ICCARM__)
	mfcp(XREG_CP15_DATA_FAULT_STATUS,FaultStatus);
#else
	{ volatile register u32 Reg __asm(XREG_CP15_DATA_FAULT_STATUS);
	  FaultStatus = Reg; }
#endif /* #ifdef __GNUC__ */

#ifdef __GNUC__
	DfarStatus = NotifyFaultStatusDetails(DATA_ABORT, FaultStatus);
	if (DfarStatus) {
		DataAbortMemAddr = mfcp(XREG_CP15_DATA_FAULT_ADDRESS);
	}
	xdbg_exception_printf(XDBG_DEBUG_ERROR,
			"Address of Instruction causing data abort is: 0x%lx\n",
			DataAbortAddr);
	if (DfarStatus) {
		xdbg_exception_printf(XDBG_DEBUG_ERROR,
			"Address of Memory whose access caused the data abort is: 0x%lx\n",
			DataAbortMemAddr);
	}
	xdbg_exception_printf(XDBG_DEBUG_ERROR,
		"-------------------------------------------------------------\n");
#else
	xdbg_printf(XDBG_DEBUG_GENERAL, "Data abort with Data Fault Status Register  %lx\n",FaultStatus);
	xdbg_printf(XDBG_DEBUG_GENERAL, "Address of Instruction causing Data abort %lx\n",DataAbortAddr);
#endif
#endif /* #ifdef DEBUG */
	while(1) {
		;
	}
}

/*****************************************************************************/
/**
*
* Default Prefetch abort handler which prints prefetch fault status register through
* which information about instruction prefetch fault can be acquired
*
* @return	None.
*
****************************************************************************/
static void Xil_PrefetchAbortHandler(void *CallBackRef){
	(void) CallBackRef;
#ifdef DEBUG
	u32 FaultStatus;
#ifdef __GNUC__
	u32 IfarStatus;
#endif

#ifdef __GNUC__
	xdbg_exception_printf(XDBG_DEBUG_ERROR,
		"-------------------------------------------------------------\n");
	FaultStatus = mfcp(XREG_CP15_INST_FAULT_STATUS);
#elif defined (__ICCARM__)
	mfcp(XREG_CP15_INST_FAULT_STATUS,FaultStatus);
#else
	{ volatile register u32 Reg __asm(XREG_CP15_INST_FAULT_STATUS);
	FaultStatus = Reg; }
#endif /* #ifdef __GNUC__ */

#ifdef __GNUC__
	IfarStatus = NotifyFaultStatusDetails(INS_PREFETCH_ABORT, FaultStatus);
	if (IfarStatus) {
		xdbg_exception_printf(XDBG_DEBUG_ERROR,
		"Address of Instruction Causing Prefetch Abort is: 0x%lx\n",
							PrefetchAbortAddr);
	}
	xdbg_exception_printf(XDBG_DEBUG_ERROR,
		"-------------------------------------------------------------\n");
#else
	xdbg_printf(XDBG_DEBUG_GENERAL, "Prefetch abort with Instruction Fault Status Register  %lx\n",FaultStatus);
	xdbg_printf(XDBG_DEBUG_GENERAL, "Address of Instruction causing Prefetch abort %lx\n",PrefetchAbortAddr);
#endif
#endif /* #ifdef DEBUG */
	while(1) {
		;
	}
}

/*****************************************************************************/
/**
*
* Default undefined exception handler which prints address of the undefined
* instruction if debug prints are enabled
*
* @return	None.
*
****************************************************************************/
static void Xil_UndefinedExceptionHandler(void *CallBackRef){
	(void) CallBackRef;
#ifdef DEBUG
	xdbg_exception_printf(XDBG_DEBUG_ERROR,
			"Address of the undefined instruction 0x%lx\n",
			UndefinedExceptionAddr);
#endif
	while(1) {
		;
	}
}

#ifdef DEBUG
static u32 NotifyFaultStatusDetails(u32 Fault_Type, u32 FaultStatus)
{
	u32 Far_Status = 0x0;

	if (Fault_Type == DATA_ABORT) {
		xdbg_exception_printf(XDBG_DEBUG_ERROR,
				"Data Abort with Fault Status Register (DFSR) =  0x%lx\n",
				FaultStatus);
	} else {
		xdbg_exception_printf(XDBG_DEBUG_ERROR,
		"Instruction Prefetch Abort with Fault Status Register (IFSR) =  0x%lx\n",
		FaultStatus);
	}

	/* Extract bit 10 and bits 3:0 to form the 5 bit error identifier */

	FaultStatus = EXTRACT_BITS_10_AND_3_TO_0(FaultStatus);

	switch (FaultStatus) {
#if defined(ARMR5)
	case ARMV7_BACKGROUND_FAULT:
		xdbg_exception_printf(XDBG_DEBUG_ERROR, "Background Fault\n");
		Far_Status = ARMV7_DFAR_VALUE_VALID;
		break;
#endif
	case ARMV7_ALIGNMENT_FAULT:
		xdbg_exception_printf(XDBG_DEBUG_ERROR, "Alignment Fault\n");
		Far_Status = ARMV7_DFAR_VALUE_VALID;
		break;
	case ARMV7_DEBUG_EVENT:
		xdbg_exception_printf(XDBG_DEBUG_ERROR, "Debug Event\n");
		break;
#if !defined(ARMR5)
	case ARMV7_ACCESS_FLAG_FAULT:
		xdbg_exception_printf(XDBG_DEBUG_ERROR, "Debug Event\n");
		Far_Status = ARMV7_DFAR_VALUE_VALID;
		break;
	case ARMV7_ICACHE_MAINTENANCE_FAULT:
		if (Fault_Type == DATA_ABORT) {
			xdbg_exception_printf(XDBG_DEBUG_ERROR,
				"Fault on Instruction Cache Maintenance\n");
			Far_Status = ARMV7_DFAR_VALUE_VALID;
		}
		break;
	case ARMV7_TRANSLATION_FAULT:
		xdbg_exception_printf(XDBG_DEBUG_ERROR, "Translation Fault\n");
		break;
	case ARMV7_DOMAIN_FAULT:
		xdbg_exception_printf(XDBG_DEBUG_ERROR, "Domain Fault\n");
		break;
	case ARMV7_SYNC_ABORT_TRANSTAB_WALK:
		xdbg_exception_printf(XDBG_DEBUG_ERROR,
				"Synchronous External Abort on Translation Table Walk\n");
		Far_Status = ARMV7_DFAR_VALUE_VALID;
		break;
#endif
	case ARMV7_PERMISSION_FAULT:
		xdbg_exception_printf(XDBG_DEBUG_ERROR, "Permission Fault\n");
		Far_Status = ARMV7_DFAR_VALUE_VALID;
		break;
#if !defined(ARMR5)
	case ARMV7_TLB_CONFLICT_ABORT:
		xdbg_exception_printf(XDBG_DEBUG_ERROR, "TLB Conflict Abort\n");
		Far_Status = ARMV7_DFAR_VALUE_VALID;
		break;
#endif
	case ARMV7_ASYNC_EXT_ABORT:
		xdbg_exception_printf(XDBG_DEBUG_ERROR, "Asynchronous External Abort\n");
		break;
	case ARMV7_SYNC_EXT_ABORT:
		xdbg_exception_printf(XDBG_DEBUG_ERROR, "Synchronous External Abort\n");
		Far_Status = ARMV7_DFAR_VALUE_VALID;
		break;
	case ARMV7_MEM_ACS_SYNC_PAR_ERR:
		if (Fault_Type == DATA_ABORT) {
			xdbg_exception_printf(XDBG_DEBUG_ERROR,
				"Synchronous Parity Error on Memory Access\n");
			Far_Status = ARMV7_DFAR_VALUE_VALID;
		}
		break;
	case ARMV7_MEM_ACS_ASYNC_PAR_ERR:
		if (Fault_Type == DATA_ABORT) {
			xdbg_exception_printf(XDBG_DEBUG_ERROR,
				"Asynchronous Parity Error on Memory Access\n");
		}
		break;
#if !defined(ARMR5)
	case ARMV7_SYNCPAR_ERR_TRANSTAB_WALK:
		xdbg_exception_printf(XDBG_DEBUG_ERROR,
			"Synchronous Parity Error on Translation Table Walk\n");
		Far_Status = ARMV7_DFAR_VALUE_VALID;
		break;
#endif
	default:
		break;
	}

	return Far_Status;
}
#endif /* #ifdef DEBUG */
#endif /* #if defined (__aarch64__) */
