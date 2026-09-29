/******************************************************************************
* Copyright (C) 2015 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2022 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

#ifndef XIPIPSU_H_
#define XIPIPSU_H_

/*****************************************************************************/
/**
 * @file xipipsu.h
 * @addtogroup ipipsu Overview
 * @{
 * @brief Defines the IPIPSU (Inter-Processor Interrupt) driver interface.
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
 * 2.19 vlt 12/30/25 Update Doxygen comments to include SDT flow details.
 * 2.19 vlt 03/15/26 Updated BaseAddress type from u32 to UINTPTR
*                    to support 64-bit addressing
 * 2.20 vn  09/10/26 Align Doxygen comments with AMD standards
 * </pre>
 *
 *****************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/
#include "xil_io.h"
#include "xstatus.h"
#include "xipipsu_hw.h"

/************************** Constant Definitions *****************************/
#define XIPIPSU_BUF_TYPE_MSG	(0x001U) /**< Message type buffer. */
#define XIPIPSU_BUF_TYPE_RESP	(0x002U) /**< Response type buffer. */
#define XIPIPSU_CRC_INDEX		(0x7U) /**< Index where the CRC is
					       * stored. */
#define XIPIPSU_W0_TO_W6_SIZE	(28U) /**< Size of words 0 through 6, in
					      * bytes. */

#define XIPIPSU_CRC_ERROR		(0xFL) /**< CRC-mismatch error code. */

/** IPI CRC enable control. Set nonzero to enable CRC checking. */
#define ENABLE_IPI_CRC_VAL	(0x0U)

#if ENABLE_IPI_CRC_VAL
#define ENABLE_IPI_CRC  /**< Defined when IPI CRC support is enabled. */
#endif

#ifdef ENABLE_IPI_CRC
/*
 * Subtract one word to reserve the last word for the CRC when IPI CRC is
 * enabled.
 */
/** Maximum message length, excluding the CRC word. */
#define XIPIPSU_MAX_MSG_LEN             (XIPIPSU_MSG_BUF_SIZE - 1U)
#else
/** Maximum message length. */
#define XIPIPSU_MAX_MSG_LEN		XIPIPSU_MSG_BUF_SIZE
#endif

/**
 * @name IPI target masks
 *
 * Target masks used to reference processor IPI targets.
 * @{
 */
/*
 * FIXME: Workaround for the system device-tree flow. The final solution
 * generates these defines using lopper.
 */
#ifdef	SDT
#if defined (VERSAL_2VE_2VM)
/** PMC 0 channel 0 target mask. */
#define XPAR_XIPIPS_TARGET_PMC_0_CH0_MASK			0x00000002U
/** PMC 0 channel 1 target mask. */
#define XPAR_XIPIPS_TARGET_PMC_0_CH1_MASK			0x00000100U
/** ASU 0 channel 0 target mask. */
#define XPAR_XIPIPS_TARGET_ASU_0_CH0_MASK			0x00000001U
#elif defined (VERSAL_NET)
/** PSX PMC 0 channel 0 target mask. */
#define XPAR_XIPIPS_TARGET_PSX_PMC_0_CH0_MASK		0x00000002U
/** PSX PMC 0 channel 1 target mask. */
#define XPAR_XIPIPS_TARGET_PSX_PMC_0_CH1_MASK		0x00000100U
/** PSX PSM 0 channel 0 target mask. */
#define XPAR_XIPIPS_TARGET_PSX_PSM_0_CH0_MASK		0x00000001U
#elif defined (versal)
/** PSV PMC 0 channel 0 target mask. */
#define XPAR_XIPIPS_TARGET_PSV_PMC_0_CH0_MASK		0x00000002U
/** PSV PMC 0 channel 1 target mask. */
#define XPAR_XIPIPS_TARGET_PSV_PMC_0_CH1_MASK		0x00000100U
/** PSV PSM 0 channel 0 target mask. */
#define XPAR_XIPIPS_TARGET_PSV_PSM_0_CH0_MASK		0x00000001U
#else
/** PSU Cortex-A53 core 0 channel 0 target mask. */
#define XPAR_XIPIPS_TARGET_PSU_CORTEXA53_0_CH0_MASK	0x00000001U
/** PSU Cortex-A53 core 1 channel 0 target mask. */
#define XPAR_XIPIPS_TARGET_PSU_CORTEXA53_1_CH0_MASK	0x00000001U
/** PSU Cortex-A53 core 2 channel 0 target mask. */
#define XPAR_XIPIPS_TARGET_PSU_CORTEXA53_2_CH0_MASK	0x00000001U
/** PSU Cortex-A53 core 3 channel 0 target mask. */
#define XPAR_XIPIPS_TARGET_PSU_CORTEXA53_3_CH0_MASK	0x00000001U
/** PSU Cortex-R5 core 0 channel 0 target mask. */
#define XPAR_XIPIPS_TARGET_PSU_CORTEXR5_0_CH0_MASK	0x00000100U
/** PSU Cortex-R5 core 1 channel 0 target mask. */
#define XPAR_XIPIPS_TARGET_PSU_CORTEXR5_1_CH0_MASK	0x00000200U
/** PSU PMU 0 channel 0 target mask. */
#define XPAR_XIPIPS_TARGET_PSU_PMU_0_CH0_MASK		0x00010000U
/** PSU PMU 1 channel 0 target mask. */
#define XPAR_XIPIPS_TARGET_PSU_PMU_1_CH0_MASK		0x00020000U
/** PSU PMU 2 channel 0 target mask. */
#define XPAR_XIPIPS_TARGET_PSU_PMU_2_CH0_MASK		0x00040000U
/** PSU PMU 3 channel 0 target mask. */
#define XPAR_XIPIPS_TARGET_PSU_PMU_3_CH0_MASK		0x00080000U
#endif
#endif
/** @} */

/**************************** Type Definitions *******************************/
/**
 * @brief Describes an IPI target.
 */
typedef struct {
	u32 Mask; /**< Bit mask for the target. */
	u32 BufferIndex; /**< Buffer index used for calculating the
			   * buffer address. */
} XIpiPsu_Target;

/**
 * @brief Holds the configuration information for one IPI device.
 */
typedef struct {
#ifndef SDT
	u32 DeviceId; /**< Unique ID of the device. */
#else
	char *Name; /**< Name of the device. */
#endif
	UINTPTR BaseAddress; /**< Base address of the device. */
	u32 BitMask; /**< Bit mask that identifies this CPU. */
	u32 BufferIndex; /**< Index of the IPI message buffer. */
	u32 IntId; /**< Interrupt ID on the Generic Interrupt
		     * Controller (GIC). */
#ifdef SDT
	UINTPTR IntrParent; 	/**< Bit[0] holds the interrupt parent type.
				* Bits[64/32:1] hold the parent base address. */
#endif
	u32 TargetCount; /**< Number of available IPI targets, up to
			   * XIPIPSU_MAX_TARGETS. */
	XIpiPsu_Target TargetList[XIPIPSU_MAX_TARGETS]; /**< IPI target list. */
} XIpiPsu_Config;

/**
 * @brief Holds the XIpiPsu driver instance data.
 *
 * Allocate a variable of this type for each IPI device in the system. Pass a
 * pointer to that variable to the driver API functions.
 */
typedef struct {
	XIpiPsu_Config Config; /**< Configuration structure. */
	u32 IsReady; /**< Device is initialized and ready. */
	u32 Options; /**< Options set in the device. */
} XIpiPsu;

/***************** Macros (Inline Functions) Definitions *********************/
/**
*
* @brief Reads the register specified by the base address and offset.
*
* @param[in]	BaseAddress Base address of the IPI instance.
* @param[in]	RegOffset Offset of the register relative to the base.
*
* @return	Value of the specified register.
* @note
* C-style signature
*	u32 XIpiPsu_ReadReg(UINTPTR BaseAddress, u32 RegOffset)
*
*****************************************************************************/
#define XIpiPsu_ReadReg(BaseAddress, RegOffset) \
	Xil_In32((BaseAddress) + (RegOffset))

/****************************************************************************/
/**
*
* @brief Writes a value into the register specified by the base address
*	and offset.
*
* @param[in] BaseAddress Base address of the IPI instance.
* @param[in] RegOffset Offset of the register relative to the base.
* @param[in] Data 32-bit value to write into the specified register.
*
* @note
* C-style signature
*	void XIpiPsu_WriteReg(UINTPTR BaseAddress, u32 RegOffset, u32 Data)
*
*****************************************************************************/
#define XIpiPsu_WriteReg(BaseAddress, RegOffset, Data) \
	Xil_Out32(((BaseAddress) + (RegOffset)), (Data))

/****************************************************************************/
/**
*
* @brief Enables the interrupts specified in <i>Mask</i>.
*
* Each bit set to 1 in <i>Mask</i> enables the corresponding interrupt.
*
* @param[in]	InstancePtr Pointer to the IPI instance.
* @param[in]	Mask Bit mask of interrupts to enable. Form the mask by
*			combining individual CPU masks with a bitwise OR.
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
* @brief Disables the interrupts specified in <i>Mask</i>.
*
* Each bit set to 1 in <i>Mask</i> disables the corresponding interrupt.
*
* @param[in]	InstancePtr Pointer to the IPI instance.
* @param[in]	Mask Bit mask of interrupts to disable. Form the mask by
*			combining individual CPU masks with a bitwise OR.
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
* @brief Gets the <i>STATUS REGISTER</i> of the current IPI instance.
*
* @param[in] InstancePtr Pointer to the IPI instance.
* @return Contents of the interrupt status register (ISR).
* @note Parse this 32-bit value to identify the source CPU.<br><br>
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
* @brief Clears the <i>STATUS REGISTER</i> of the current IPI instance.
*
* Each bit set to 1 in <i>Mask</i> clears the corresponding interrupt status.
*
* @param[in] InstancePtr Pointer to the IPI instance.
* @param[in] Mask Bit mask identifying the source CPU.
*
* @note Use this macro after handling the IPI. Clearing the status
* automatically clears the corresponding bit in the OBSERVATION register of the
* source CPU.<br><br>
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
* @brief Gets the <i>OBSERVATION REGISTER</i> of the current IPI instance.
*
* @param[in]	InstancePtr Pointer to the IPI instance.
* @return	Contents of the observation register (OBS).
* @note		Parse this 32-bit value to check the status of
*			individual CPUs.<br><br>
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
 * @brief Holds the configuration entry for each IPI instance.
 *
 * In the XSCT/Classic flow, XPAR_XIPIPSU_NUM_INSTANCES from xparameters.h sizes
 * the table. In the system device-tree flow, the table ends with an entry whose
 * Name is NULL.
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
XIpiPsu_Config *XIpiPsu_LookupConfig(UINTPTR BaseAddress);
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

#ifndef SDT
void XIpiPsu_SetConfigTable(u32 DeviceId, XIpiPsu_Config *ConfigTblPtr);
#endif

#ifdef __cplusplus
}
#endif

#endif /* XIPIPSU_H_ */
/** @} */
