/******************************************************************************
* Copyright (C) 2015 - 2021 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
 * @file xipipsu.h
* @addtogroup ipipsu Overview
* @{
* @details
 *
 * This section explains the implementation of IPIPSU driver. Inter Processor
 * Interrupt (IPI) is used for communication between different processors on
 * ZynqMP SoC. Each IPI register set has Trigger, Status and Observation
 * registers for communication between processors. Each IPI path has a 32 byte
 * buffer associated with it and these buffers are located in the XPPU RAM.
 *
 * This driver supports the following operations:
 *
 * - Trigger IPIs to CPUs on the SoC
 * - Write and Read Message buffers
 * - Read the status of Observation Register to get status of Triggered IPI
 * - Enable/Disable IPIs from selected Masters
 * - Read the Status register to get the source of an incoming IPI
 *
 * <b>Initialization & Configuration</b>
 *
 * The XIpiPsu_Config structure is used by the driver to configure itself.
 * Fields inside this structure are properties of XIpiPsu based on its
 * hardware build.
 *
 * To support multiple runtime loading and initialization strategies employed
 * by various operating systems, the driver instance can be initialized in
 * the following way:
 *
 *   - XIpiPsu_CfgInitialize(InstancePtr, CfgPtr, EffectiveAddr) - Uses a
 *	 configuration structure provided by the caller. If running in a
 *	 system with address translation, the parameter EffectiveAddr should
 *	 be the virtual address.
 *
 * The config data for the driver is loaded and is based on the HW build. The
 * XIpiPsu_Config data structure contains all the data related to the
 * IPI driver instance and also the available Target CPUs.
 *
 * <b>Sending an IPI</b>
 *
 * The following steps can be followed to send an IPI:
 * - Write the Message into Message Buffer using XIpiPsu_WriteMessage()
 * - Trigger IPI using XIpiPsu_TriggerIpi()
 * - Wait for Ack using XIpiPsu_PollForAck()
 * - Read response using XIpiPsu_ReadMessage()
 *
 * @note	XIpiPsu_GetObsStatus() before sending an IPI to ensure that the
 * previous IPI was serviced by the target
 *
 * <b>Receiving an IPI</b>
 *
 * To receive an IPI, the following sequence can be followed:
 * - Register an interrupt handler for the IPIs interrupt ID
 * - Enable the required sources using XIpiPsu_InterruptEnable()
 * - In the interrupt handler, Check for source using XIpiPsu_GetInterruptStatus
 * - Read the message form source using XIpiPsu_ReadMessage()
 * - Write the response using XIpiPsu_WriteMessage()
 * - Ack the IPI using XIpiPsu_ClearInterruptStatus()
 *
 * @note	XIpiPsu_Reset can be used at startup to clear the status and
 * disable all sources
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
#define XIPIPSU_MAX_MSG_LEN		XIPIPSU_MSG_BUF_SIZE /**< Maximum message length */
#define XIPIPSU_CRC_INDEX		(0x7U) /**< Index where the CRC is stored */
#define XIPIPSU_W0_TO_W6_SIZE	(28U)	       /**< Size of the word 0 to word 6 */

/* CRC Mismatch error code */
#define XIPIPSU_CRC_ERROR		(0xFL) /**< CRC error occurred */

/* Enable CRC check for IPI messages */
#define ENABLE_IPI_CRC_VAL	(0x0U) /**< Enable CRC */

#if ENABLE_IPI_CRC_VAL
#define ENABLE_IPI_CRC
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
	u32 DeviceId; /**< Unique ID  of device */
	UINTPTR BaseAddress; /**< Base address of the device */
	u32 BitMask; /**< BitMask to be used to identify this CPU */
	u32 BufferIndex; /**< Index of the IPI Message Buffer */
	u32 IntId; /**< Interrupt ID on GIC **/
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
* Read the register specified by the base address and offset
*
* @param	BaseAddress is the base address of the IPI instance
* @param	RegOffset is the offset of the register relative to base
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
* Write a value into a register specified by base address and offset
*
* @param BaseAddress is the base address of the IPI instance
* @param RegOffset is the offset of the register relative to base
* @param Data is a 32-bit value that is to be written into the specified register
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
* @param	InstancePtr is a pointer to the instance to be worked on.
* @param	Mask contains a bit mask of interrupts to enable. The mask can
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
* @param	InstancePtr is a pointer to the instance to be worked on.
* @param	Mask contains a bit mask of interrupts to disable. The mask can
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
* Get the <i>STATUS REGISTER</i> of the current IPI instance.
*
* @param InstancePtr is a pointer to the instance to be worked on.
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
* Clear the <i>STATUS REGISTER</i> of the current IPI instance.
* The corresponding interrupt status for
* each bit set to 1 in <i>Mask</i>, will be cleared
*
* @param InstancePtr is a pointer to the instance to be worked on.
* @param Mask corresponding to the source CPU*
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
* Get the <i>OBSERVATION REGISTER</i> of the current IPI instance.
*
* @param	InstancePtr is a pointer to the instance to be worked on.
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
extern XIpiPsu_Config XIpiPsu_ConfigTable[XPAR_XIPIPSU_NUM_INSTANCES];

/************************** Function Prototypes *****************************/

/* Static lookup function implemented in xipipsu_sinit.c */

XIpiPsu_Config *XIpiPsu_LookupConfig(u32 DeviceId);

/* Interface Functions implemented in xipipsu.c */

XStatus XIpiPsu_CfgInitialize(XIpiPsu *InstancePtr, XIpiPsu_Config * CfgPtr,
		UINTPTR EffectiveAddress);

void XIpiPsu_Reset(XIpiPsu *InstancePtr);

XStatus XIpiPsu_TriggerIpi(XIpiPsu *InstancePtr, u32 DestCpuMask);

XStatus XIpiPsu_PollForAck(const XIpiPsu *InstancePtr, u32 DestCpuMask,
		u32 TimeOutCount);

XStatus XIpiPsu_ReadMessage(XIpiPsu *InstancePtr, u32 SrcCpuMask, u32 *MsgPtr,
		u32 MsgLength, u8 BufferType);

XStatus XIpiPsu_WriteMessage(XIpiPsu *InstancePtr, u32 DestCpuMask,const u32 *MsgPtr,
		u32 MsgLength, u8 BufferType);

void XIpiPsu_SetConfigTable(u32 DeviceId, XIpiPsu_Config *ConfigTblPtr);

#ifdef __cplusplus
}
#endif

#endif /* XIPIPSU_H_ */
/** @} */
