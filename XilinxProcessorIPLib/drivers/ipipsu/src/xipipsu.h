/******************************************************************************
* Copyright (C) 2015 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2022 - 2025 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
 * @file xipipsu.h
* @addtogroup ipipsu Overview
* @{
* @details
 *
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver  Who Date     Changes
 * ---- --- -------- --------------------------------------------------
 * 2.2  ms  01/23/17 Modified xil_printf statement in main function for all
 *                    examples to ensure that "Successfully ran" and "Failed"
 *                    strings are available in all examples. This is a fix
 *                    for CR-965028.
 *  	kvn 02/17/17  Add support for updating ConfigTable at run time
 *      ms  03/17/17  Added readme.txt file in examples folder for doxygen
 *                    generation.
 * 2.3  ms  04/11/17  Modified tcl file to add suffix U for all macro
 *                    definitions of ipipsu in xparameters.h
 *      ms  03/28/17  Add index.html to provide support for importing
 *                    examples in SDK.
 * 2.5  sdd 12/17/18  Add the cpp extern macro.
 * 2.6  sdd 04/09/20  Restructure the code for modularity and readability
 * 		      Added file  xipipsu_buf.c and xipipsu_buf.h as part of it.
 * 2.7  sdd 09/03/20  Makefile update for parallel execution.
 * 2.8  nsk 12/14/20  Modified the driver tcl to not to use the instance names.
 * 2.8  nsk 01/19/21  Updated the driver tcl to use IP_NAME for IPIs mapped.
 * 2.9  ma  02/12/21  Added IPI CRC functionality
 *	sdd 02/17/21 Fixed doxygen warnings.
 *	sdd 03/10/21 Fixed misrac warnings.
 *		     Fixed doxygen warnings.
 * 2.11 sdd 11/17/21 Updated tcl to check for microblaze processors
 * 2.14 adk 04/14/23 Added support for system device-tree flow.
 * 2.14 adk 05/22/23 Added IPI Mask's for referring to processor IPI Targets
 * 		     in system device-tree flow.
 * 2.14 sd 07/27/23  Update the target count.
 * 2.15 ht 01/11/24  Add PMC, PSM bitmasks macros for versal-net
 * 2.16 ma 09/10/24  Updated to support VERSAL_2VE_2VM platform
 * 2.17 ht 11/25/24  Update Max Message length to accommodate for CRC bytes
 *                   when IPI CRC is enabled
 *	jb 12/26/24 Fixed misrac warnings
 * </pre>
 *
 *****************************************************************************/
/*****************************************************************************/
#ifndef XIPIPSU_H_
#define XIPIPSU_H_

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/
#include "xil_io.h"
#include "xstatus.h"
#include "xipipsu_hw.h"

/************************** Constant Definitions *****************************/
#define XIPIPSU_BUF_TYPE_MSG	(0x001U) /**< Message type buffer */
#define XIPIPSU_BUF_TYPE_RESP	(0x002U) /**< Response buffer */
#define XIPIPSU_CRC_INDEX		(0x7U) /**< Index where the CRC is stored */
#define XIPIPSU_W0_TO_W6_SIZE	(28U)	       /**< Size of the word 0 to word 6 */

/* CRC Mismatch error code */
#define XIPIPSU_CRC_ERROR		(0xFL) /**< CRC error occurred */

/* Enable CRC check for IPI messages */
#define ENABLE_IPI_CRC_VAL	(0x0U) /**< Enable CRC */

#if ENABLE_IPI_CRC_VAL
#define ENABLE_IPI_CRC  /**< Enable CalculateCRC API*/
#endif

#ifdef ENABLE_IPI_CRC
/*
 * Subtracting 1 from the message length to accommodate for the CRC bytes when
 * IPI CRC is enabled.
 * */
#define XIPIPSU_MAX_MSG_LEN             (XIPIPSU_MSG_BUF_SIZE - 1U)
#else
#define XIPIPSU_MAX_MSG_LEN		XIPIPSU_MSG_BUF_SIZE	/**< Maximum message length */
#endif

/*
 * Target List for referring to processor IPI Targets
 * FIXME: This is a workaround for system device-tree flow, the proper solution
 * would be generating the defines using lopper.
 */
#ifdef	SDT
#if defined (VERSAL_2VE_2VM)
#define XPAR_XIPIPS_TARGET_PMC_0_CH0_MASK			0x00000002U
#define XPAR_XIPIPS_TARGET_PMC_0_CH1_MASK			0x00000100U
#define XPAR_XIPIPS_TARGET_ASU_0_CH0_MASK			0x00000001U
#elif defined (VERSAL_NET)
#define XPAR_XIPIPS_TARGET_PSX_PMC_0_CH0_MASK		0x00000002U
#define XPAR_XIPIPS_TARGET_PSX_PMC_0_CH1_MASK		0x00000100U
#define XPAR_XIPIPS_TARGET_PSX_PSM_0_CH0_MASK		0x00000001U
#elif defined (versal)
#define XPAR_XIPIPS_TARGET_PSV_PMC_0_CH0_MASK		0x00000002U
#define XPAR_XIPIPS_TARGET_PSV_PMC_0_CH1_MASK		0x00000100U
#define XPAR_XIPIPS_TARGET_PSV_PSM_0_CH0_MASK		0x00000001U
#else
#define XPAR_XIPIPS_TARGET_PSU_CORTEXA53_0_CH0_MASK	0x00000001U
#define XPAR_XIPIPS_TARGET_PSU_CORTEXA53_1_CH0_MASK	0x00000001U
#define XPAR_XIPIPS_TARGET_PSU_CORTEXA53_2_CH0_MASK	0x00000001U
#define XPAR_XIPIPS_TARGET_PSU_CORTEXA53_3_CH0_MASK	0x00000001U
#define XPAR_XIPIPS_TARGET_PSU_CORTEXR5_0_CH0_MASK	0x00000100U
#define XPAR_XIPIPS_TARGET_PSU_CORTEXR5_1_CH0_MASK	0x00000200U
#define XPAR_XIPIPS_TARGET_PSU_PMU_0_CH0_MASK		0x00010000U
#define XPAR_XIPIPS_TARGET_PSU_PMU_1_CH0_MASK		0x00020000U
#define XPAR_XIPIPS_TARGET_PSU_PMU_2_CH0_MASK		0x00040000U
#define XPAR_XIPIPS_TARGET_PSU_PMU_3_CH0_MASK		0x00080000U
#endif
#endif

/**************************** Type Definitions *******************************/
/**
 * Data structure used to refer IPI Targets
 */
typedef struct {
	u32 Mask; /**< Bit Mask for the target */
	u32 BufferIndex; /**< Buffer Index used for calculating buffer address */
} XIpiPsu_Target;

/**
 * This typedef contains configuration information for the device.
 */
typedef struct {
#ifndef SDT
	u32 DeviceId; /**< Unique ID  of device */
#else
	char *Name;
#endif
	UINTPTR BaseAddress; /**< Base address of the device */
	u32 BitMask; /**< BitMask to be used to identify this CPU */
	u32 BufferIndex; /**< Index of the IPI Message Buffer */
	u32 IntId; /**< Interrupt ID on GIC **/
#ifdef SDT
	UINTPTR IntrParent; 	/** Bit[0] Interrupt parent type Bit[64/32:1]
				 * Parent base address */
#endif
	u32 TargetCount; /**< Number of available IPI Targets */
	XIpiPsu_Target TargetList[XIPIPSU_MAX_TARGETS] ; /**< List of IPI Targets */
} XIpiPsu_Config;

/**
 * The XIpiPsu driver instance data. The user is required to allocate a
 * variable of this type for each IPI device in the system. A pointer
 * to a variable of this type is then passed to the driver API functions.
 */
typedef struct {
	XIpiPsu_Config Config; /**< Configuration structure */
	u32 IsReady; /**< Device is initialized and ready */
	u32 Options; /**< Options set in the device */
} XIpiPsu;

/***************** Macros (Inline Functions) Definitions *********************/
/**
*
* Reads the register specified by the base address and offset
*
* @param	BaseAddress Base address of the IPI instance
* @param	RegOffset Offset of the register relative to base
*
* @return	Value of the specified register
* @note
* C-style signature
*	u32 XIpiPsu_ReadReg(u32 BaseAddress, u32 RegOffset)
*
*****************************************************************************/

#define XIpiPsu_ReadReg(BaseAddress, RegOffset) \
	Xil_In32((BaseAddress) + (RegOffset))

/****************************************************************************/
/**
*
* Writes a value into a register specified by base address and offset
*
* @param BaseAddress Base address of the IPI instance
* @param RegOffset Offset of the register relative to base
* @param Data 32-bit value that is to be written into the specified register
*
* @note
* C-style signature
*	void XIpiPsu_WriteReg(u32 BaseAddress, u32 RegOffset, u32 Data)
*
*****************************************************************************/

#define XIpiPsu_WriteReg(BaseAddress, RegOffset, Data) \
	Xil_Out32(((BaseAddress) + (RegOffset)), (Data))

/****************************************************************************/
/**
*
* Enable interrupts specified in <i>Mask</i>. The corresponding interrupt for
* each bit set to 1 in <i>Mask</i>, will be enabled.
*
* @param	InstancePtr Pointer to the instance to be worked on.
* @param	Mask Contains a bit mask of interrupts to enable. The mask can
*			be formed using a set of bitwise or'd values of individual CPU masks
*
* @note
* C-style signature
*	void XIpiPsu_InterruptEnable(XIpiPsu *InstancePtr, u32 Mask)
*
*****************************************************************************/
#define XIpiPsu_InterruptEnable(InstancePtr, Mask) \
	XIpiPsu_WriteReg((InstancePtr)->Config.BaseAddress, \
			 XIPIPSU_IER_OFFSET, \
			 ((Mask) & XIPIPSU_ALL_MASK));

/****************************************************************************/
/**
*
* Disable interrupts specified in <i>Mask</i>. The corresponding interrupt for
* each bit set to 1 in <i>Mask</i>, will be disabled.
*
* @param	InstancePtr Pointer to the instance to be worked on.
* @param	Mask Contains a bit mask of interrupts to disable. The mask can
*			be formed using a set of bitwise or'd values of individual CPU masks
*
* @note
* C-style signature
*	void XIpiPsu_InterruptDisable(XIpiPsu *InstancePtr, u32 Mask)
*
*****************************************************************************/
#define XIpiPsu_InterruptDisable(InstancePtr, Mask)  \
	XIpiPsu_WriteReg((InstancePtr)->Config.BaseAddress, \
			 XIPIPSU_IDR_OFFSET, \
			 ((Mask) & XIPIPSU_ALL_MASK));
/****************************************************************************/
/**
*
* Gets the <i>STATUS REGISTER</i> of the current IPI instance.
*
* @param InstancePtr Pointer to the instance to be worked on.
* @return Returns the Interrupt Status register(ISR) contents
* @note User needs to parse this 32-bit value to check the source CPU
* C-style signature
*	u32 XIpiPsu_GetInterruptStatus(XIpiPsu *InstancePtr)
*
*****************************************************************************/
#define XIpiPsu_GetInterruptStatus(InstancePtr)  \
	XIpiPsu_ReadReg((InstancePtr)->Config.BaseAddress, \
			XIPIPSU_ISR_OFFSET)
/****************************************************************************/
/**
*
* Clears the <i>STATUS REGISTER</i> of the current IPI instance.
* The corresponding interrupt status for
* each bit set to 1 in <i>Mask</i> is cleared.
*
* @param InstancePtr Pointer to the instance to be worked on.
* @param Mask Mask corresponding to the source CPU*
*
* @note This function should be used after handling the IPI.
* Clearing the status will automatically clear the corresponding bit in
* OBSERVATION register of Source CPU
* C-style signature
*	void XIpiPsu_ClearInterruptStatus(XIpiPsu *InstancePtr, u32 Mask)
*
*****************************************************************************/

#define XIpiPsu_ClearInterruptStatus(InstancePtr, Mask)  \
	XIpiPsu_WriteReg((InstancePtr)->Config.BaseAddress, \
			 XIPIPSU_ISR_OFFSET, \
			 ((Mask) & XIPIPSU_ALL_MASK));
/****************************************************************************/
/**
*
* Gets the <i>OBSERVATION REGISTER</i> of the current IPI instance.
*
* @param	InstancePtr Pointer to the instance to be worked on.
* @return	Returns the Observation register(OBS) contents
* @note		User needs to parse this 32-bit value to check the status of
*			individual CPUs
* C-style signature
*	u32 XIpiPsu_GetObsStatus(XIpiPsu *InstancePtr)
*
*****************************************************************************/
#define XIpiPsu_GetObsStatus(InstancePtr)  \
	XIpiPsu_ReadReg((InstancePtr)->Config.BaseAddress, \
			XIPIPSU_OBS_OFFSET)
/****************************************************************************/
/************************** Variable Definitions *****************************/
/**
 * The IPIPSU configuration table, sized by the number of instances
 * defined in xparameters.h.
 */
#ifndef SDT
extern XIpiPsu_Config XIpiPsu_ConfigTable[XPAR_XIPIPSU_NUM_INSTANCES];
#else
extern XIpiPsu_Config XIpiPsu_ConfigTable[];
#endif

/************************** Function Prototypes *****************************/

/* Static lookup function implemented in xipipsu_sinit.c */
#ifndef SDT
XIpiPsu_Config *XIpiPsu_LookupConfig(u32 DeviceId);
#else
XIpiPsu_Config *XIpiPsu_LookupConfig(u32 BaseAddress);
#endif

/* Interface Functions implemented in xipipsu.c */

XStatus XIpiPsu_CfgInitialize(XIpiPsu *InstancePtr, XIpiPsu_Config *CfgPtr,
			      UINTPTR EffectiveAddress);

void XIpiPsu_Reset(XIpiPsu *InstancePtr);

XStatus XIpiPsu_TriggerIpi(XIpiPsu *InstancePtr, u32 DestCpuMask);

XStatus XIpiPsu_PollForAck(const XIpiPsu *InstancePtr, u32 DestCpuMask,
			   u32 TimeOutCount);

XStatus XIpiPsu_ReadMessage(XIpiPsu *InstancePtr, u32 SrcCpuMask, u32 *MsgPtr,
			    u32 MsgLength, u8 BufferType);

XStatus XIpiPsu_WriteMessage(XIpiPsu *InstancePtr, u32 DestCpuMask, const u32 *MsgPtr,
			     u32 MsgLength, u8 BufferType);

void XIpiPsu_SetConfigTable(u32 DeviceId, XIpiPsu_Config *ConfigTblPtr);

#ifdef __cplusplus
}
#endif

#endif /* XIPIPSU_H_ */
/** @} */
