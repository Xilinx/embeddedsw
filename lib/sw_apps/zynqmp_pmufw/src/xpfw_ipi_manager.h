/******************************************************************************
* Copyright (c) 2015 - 2021 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


#ifndef XPFW_IPI_MANAGER_H_
#define XPFW_IPI_MANAGER_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "xstatus.h"
#include "xpfw_default.h"
#include "xpfw_module.h"
#include "xipipsu.h"
#include "xparameters.h"

#define XPFW_IPI_MASK_COUNT XIPIPSU_MAX_TARGETS
extern XIpiPsu *Ipi0InstPtr;

#define XPFW_IPI_MAX_MSG_LEN XIPIPSU_MAX_MSG_LEN

#ifdef XPAR_XIPIPS_TARGET_PSU_CORTEXA53_0_CH0_MASK
#define IPI_PMU_0_IER_APU_MASK	  XPAR_XIPIPS_TARGET_PSU_CORTEXA53_0_CH0_MASK
#else
#define IPI_PMU_0_IER_APU_MASK 0U
#endif

#ifdef XPAR_XIPIPS_TARGET_PSU_CORTEXR5_0_CH0_MASK
#define IPI_PMU_0_IER_RPU_0_MASK  XPAR_XIPIPS_TARGET_PSU_CORTEXR5_0_CH0_MASK
#else
#define IPI_PMU_0_IER_RPU_0_MASK 0U
#endif

#ifdef XPAR_XIPIPS_TARGET_PSU_CORTEXR5_1_CH0_MASK
#define IPI_PMU_0_IER_RPU_1_MASK  XPAR_XIPIPS_TARGET_PSU_CORTEXR5_1_CH0_MASK
#else
#define IPI_PMU_0_IER_RPU_1_MASK 0U
#endif

#ifdef XPAR_XIPIPS_TARGET_PSU_PMU_0_CH0_MASK
#define IPI_PMU_0_IER_PMU_0_MASK XPAR_XIPIPS_TARGET_PSU_PMU_0_CH0_MASK
#else
#define IPI_PMU_0_IER_PMU_0_MASK 0U
#endif

/**
 * Initialize the IPI driver instance
 * This should be called in the core init
 */
s32 XPfw_IpiManagerInit(void);

/**
 * Write a message to IPI Message Buffer
 * @param ModPtr is the pointer to module that is sending the message
 * @param DestCpuMask is mask for the destination CPU
 * @param MsgPtr is pointer to the buffer containing the message to be sent
 * @param MsgLen is the number of 32-bit words to be sent
 * @return XST_SUCCESS if success
 *         XST_FAILURE if failure
 */
s32 XPfw_IpiWriteMessage(const XPfw_Module_t *ModPtr, u32 DestCpuMask, u32 *MsgPtr, u32 MsgLen);

/**
 * Write a message to IPI Response Buffer
 * This function is preferably called from with in a IPI interrupt handler to send a response
 * for that IPI request
 * @param ModPtr is the pointer to module that is sending the message
 * @param DestCpuMask is mask for the destination CPU
 * @param MsgPtr is pointer to the buffer containing the message to be sent
 * @param MsgLen is the number of 32-bit words to be sent
 * @return XST_SUCCESS if success
 *         XST_FAILURE if failure
 */
s32 XPfw_IpiWriteResponse(const XPfw_Module_t *ModPtr, u32 DestCpuMask, u32 *MsgPtr, u32 MsgLen);

/**
 * Read Message buffer contents (Used only by Core)
 * @param SrcCpuMask is mask for the Source CPU
 * @param MsgPtr is pointer to the buffer to which message is to be retrieved
 * @param MsgLen is the number of 32-bits to be retrieved
 * @return XST_SUCCESS if IPI ID of the module matches and the message is read into buffer
 *         XST_FAILURE in case of an error
 */
s32 XPfw_IpiReadMessage(u32 SrcCpuMask, u32 *MsgPtr, u32 MsgLen);

/**
 * Read Response buffer contents
 * @param ModPtr is the pointer to module that is requesting the message
 * @param SrcCpuMask is mask for the Source CPU
 * @param MsgPtr is pointer to the buffer to which message is to be retrieved
 * @param MsgLen is the number of 32-bits to be retrieved
 * @return XST_SUCCESS if the message is read into buffer
 *         XST_FAILURE if a mismatch in IPI ID or failure to read message
 */
s32 XPfw_IpiReadResponse(const XPfw_Module_t *ModPtr, u32 SrcCpuMask, u32 *MsgPtr, u32 MsgLen);

/**
 * Trigger an IPI interrupt to a target processor
 *
 * @param DestCpuMask is the mask corresponding to Dest CPU
 *
 * @return XST_SUCCESS if IPI was triggered
 *         XST_FAILURE if an error occurred while triggering
 */
s32 XPfw_IpiTrigger(u32 DestCpuMask);

/**
 * Poll for an acknowledgment from target processor
 *
 * @param DestCpuMask is the Mask of the destination CPU from which ACK is expected
 * @param TimeOutCount is the Count after which the routines returns failure
 *
 * @return	XST_SUCCESS if successful
 * 			XST_FAILURE if a timeout occurred
 */
s32 XPfw_IpiPollForAck(u32 DestCpuMask, u32 TimeOutCount);

#ifdef __cplusplus
}
#endif

#endif /* XPFW_IPI_MANAGER_H_ */
