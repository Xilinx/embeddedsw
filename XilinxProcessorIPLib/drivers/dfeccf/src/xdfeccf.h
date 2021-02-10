/******************************************************************************
* Copyright (C) 2021 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xdfeccf.h
* @addtogroup dfeccf_v1_0
* @{
*
* The Channel Filter IP provides a wrapper around the Channel Filter
* block (dfe_channel_filter). Each instance of the block can support up to 64
* CC, arranged across a maximum of 8 Antennas. The wrapper provides access to
* the underlying blocks via TDMA Axi-stream data interfaces. An AXI memory
* mapped interface is provided, enabling configuration and control of the block
* from a microprocessor.
* The features that the channel filter IP and the driver support are:
* - Supports a maximum sampling rate of 491.52Ms/s.
* - Supports reallocation of TDM slots.
* - Using 18 bit data interface.
* - Using 16 bit coefficients.
* - Can independently configure complex and real coefficients.
* - Enables the user to program the co-efficient sets via a processor
*   interface.
* - Enables the user to change the co-efficient sets that act on the input data
*   via a processor interface
* - Supports TDD power down via a processor interface and TUSER input.
* - Supports the flushing of the internal buffers via a processor interface.
* - Indication of overflow provided via a status register.
* - TUSER/TLAST information accompanying the data is delay matched through
*   the IP.
*
* The channel filter driver provides the following features for each of
* the channels:
* 1. Setting of the co-efficient sets via the s_axi_ctrl processor interface.
* 2. Selection of the co-efficient sets to use for the real and imaginary data
* 3. inputs via the s_axi_ctrl processor interface.
* 4. Register bits to control the following inputs of the dfe_channel_filter
*   blocks:
*     - Flush buffers
*     - Enable
*     - Update coefficients
* 5. Status register indicating over and underflow of the channel filter for
*    both before and after gain stage.
* 6. Software reset.
* 7. tuser and tlast support on the data interfaces:
*
* An API which will read/write registers has been provided for debug purpose.
*
* There are plans to add more features.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who    Date     Changes
* ----- ---    -------- -----------------------------------------------
* 1.0   dc     09/03/20 Initial version
*       dc     02/02/21 Remove hard coded device node name
*       dc     02/08/21 align driver to curent specification
* </pre>
*
******************************************************************************/
#ifndef XDFECCF_H_
#define XDFECCF_H_

#ifdef __cplusplus
extern "C" {
#endif

/**************************** Includes ***************************************/
#ifdef __BAREMETAL__
#include "xil_types.h"
#include "xparameters.h"
#include "xstatus.h"
#else
#include <linux/types.h>
#include <assert.h>
#endif

#include "stdbool.h"
#include <metal/sys.h>

/**************************** Macros Definitions *****************************/
#ifndef __BAREMETAL__
#define XDFECCF_MAX_NUM_INSTANCES 10U
#define Xil_AssertNonvoid(Expression) assert(Expression)
#define Xil_AssertVoid(Expression) assert(Expression)
#define Xil_AssertVoidAlways() assert(0)
#define XST_SUCCESS 0U
#define XST_FAILURE 1U
#else
#define XDFECCF_MAX_NUM_INSTANCES XPAR_XDFECCF_NUM_INSTANCES
#endif

#define XDFECCF_CC_NUM 16U /**< Maximum CC number */
#define XDFECCF_ANT_NUM_MAX 8U /**< Maximum anntena number */
#define XDFECCF_SEQ_LENGTH_MAX 16U /**< Maximum sequence length */

#define XDFECCF_RATE_MAX 5U /**< Maximum rate Id */
#define XDFECCF_NCO_MAX 4U /**< Maximum NCO number */

/**************************** Type Definitions *******************************/
/*********** start - common code to all Logiccores ************/
#ifndef __BAREMETAL__
typedef __u32 u32;
typedef __u16 u16;
typedef __u8 u8;
typedef __s32 s32;
typedef __s16 s16;
typedef __u64 u64;
typedef __s64 s64;
typedef __s8 s8;
#endif

typedef enum {
	XDFECCF_STATE_NOT_READY = 0, /**< Not ready state*/
	XDFECCF_STATE_READY, /**< Ready state*/
	XDFECCF_STATE_RESET, /**< Reset state*/
	XDFECCF_STATE_CONFIGURED, /**< Configured state*/
	XDFECCF_STATE_INITIALISED, /**< Initialised state*/
	XDFECCF_STATE_OPERATIONAL /**< Operational state*/
} XDfeCcf_StateId;

/**
 * Logicore version.
 */
typedef struct {
	u32 Major; /**< Major version number */
	u32 Minor; /**< Minor version number */
	u32 Revision; /**< Revision number */
	u32 Patch; /**< Patch number */
} XDfeCcf_Version;

/**
 * Trigger configuration.
 */
typedef struct {
	u32 Enable; /**< [0,1], 0 = Disabled: Trigger disabled;
		1 = Enabled: Trigger enabled */
	u32 Source; /**< [0,1,2],
		0 = IMMEDIATE: write to the trigger configuration register
			immediately
		1 = TUSER: write on Edge detected on specified TUSER bit
		2 = TLAST: write on Edge detected on TLAST */
	u32 TUSERBit; /**< [0-7], Species which TUSER bit is used by
		the trigger */
	u32 Edge; /**< [0,1,2], 0 = Rising; 1 = Falling; 2 = Both */
	u32 OneShot; /**< [0,1],
		0 = Continuous: Once enabled trigger repeats continuously
		1 = OneShot: Once enabled trigger occurs once and then
			disables */
} XDfeCcf_Trigger;

/**
 * All IP triggers.
 */
typedef struct {
	XDfeCcf_Trigger Activate; /**< Toggle between "Initialized",
		ultra-low power state, and "Operational". One-shot trigger,
		disabled following a single event */
	XDfeCcf_Trigger LowPower; /**< Toggle between "Low-power"
		and "Operational" state */
	XDfeCcf_Trigger CCUpdate; /**< Transition to next CC
		configuration. Will initiate flush based on CC configuration */
} XDfeCcf_TriggerCfg;

/**
 * Defines a CCID sequence.
 */
typedef struct {
	u32 Length; /**< [1-16] Sequence length */
	u32 CCID[XDFECCF_SEQ_LENGTH_MAX]; /**< [0-15].Array of CCID's
		arranged in the order the CCIDs are required to be processed
		in the channel filter */
} XDfeCcf_CCSequence;

/*********** end - common code to all Logiccores ************/
/**
 * Channel Filter model parameters structure. Data defined in Device
 * tree/xparameters.h
 */
typedef struct {
	u32 NumAntenna; /**< [1-8] */
	u32 NumCCPerAntenna; /**< [1-8] */
	u32 NumAntSlot; /**< [1-8] */
} XDfeCcf_ModelParameters;

/**
 * Configuration.
 */
typedef struct {
	XDfeCcf_Version Version; /**< Logicore version */
	XDfeCcf_ModelParameters ModelParams; /**< Logicore
		parameterization */
} XDfeCcf_Cfg;

/**
 * Initialization, "one-time" configuration parameters.
 */
typedef struct {
	u32 GainStage; /**< [0,1] Enable gain stage */
} XDfeCcf_Init;

/**
 * Defines filter coefficients.
 */
typedef struct {
	u32 Num; /**< [0-(128|256)]. True number of coefficients,
		    when non-symmetric max is 128. */
	u32 Symmetric; /**< [0,1] Select the use of symetric or non-symetric
			  filter */
	u32 Value[128]; /**< [Signed real numbers]. Array of coefficients, when
			   symmetric only the first (Num+1)/2 coefficients
			   are provided */
} XDfeCcf_Coefficients;

/**
 * Configuration for a single CC.
 */
typedef struct {
	u32 Enable; /**< [0,1] (Private) Enable/Disable CC while still
			reserving its slot in the TDM - set by helper functions
			when building the configuration the TDM */
	u32 Flush; /**< [0,1] (Private) Indicate CC should be flush following
			configuration update - set by helper functions when
			building the configuration channel_id? */
	u32 MappedId; /**< [0-7] (Private) Defines the hardblock ID value to be
			used for the CC. Used to map arbitary/system CCID
			values to available hard block TID values. Enumerated
			incrementally from 0 to max supported CC for a given
			IP configuration */
	u32 Rate; /**< [1,2,4,8] Sample "rate" (period) of CC */
	u32 Gain; /**< [0-(1<<16)-1] Gain setting for this CC */
	u32 ImagCoeffSet; /**< [0-7] Identify the coefficient set for the
			        complex data on this CC */
	u32 RealCoeffSet; /**< [0-7] Identify the coefficient set for the real
			     data on this CC */
} XDfeCcf_CarrierCfg;

/**
 * Full CC configuration.
 */
typedef struct {
	XDfeCcf_CCSequence Sequence; /**< CCID sequence */
	XDfeCcf_CarrierCfg CarrierCfg[16]; /**< CC configurations */
	u32 AntennaCfg[8]; /**< [0,1] Antenna TDM slot enablement */
} XDfeCcf_CCCfg;

/**
 * Channel Filter Status.
 */
typedef struct {
	u32 OverflowBeforeGainReal; /**< [0,1] Overflow or underflow occurred
		in the FIR stage of the filter on the real channel */
	u32 OverflowBeforeGainImag; /**< [0,1] Overflow or underflow occurred
		in the FIR stage of the filter on the imaginary channel */
	u32 OverflowAfterGainReal; /**< [0,1] Overflow or underflow occurred in
		the GAIN stage of the filter on the real channel */
	u32 OverflowAfterGainImag; /**< [0,1] Overflow or underflow occurred in
		the GAIN stage of the filter on the imaginary channel */
	u32 OveflowAntenna; /**< [0,7] Lowest number antenna on which first
		overflow, or underflow, occurred */
	u32 OverflowCCID; /**< [0,7] CCID on which first overflow, or
		underflow, occurred */
	u32 CCUpdate; /**< [0,1] CC update event has occurred */
	u32 CCSequenceError; /**< [0, 1] CC sequence mismatch has been
		detected */
} XDfeCcf_Status;

/**
 * Interrupt mask.
 */
typedef struct {
	u32 Overflow; /**< [0,1] Mask overflow events */
	u32 CCUpdate; /**< [0,1] Mask CC update events */
	u32 CCSequenceError; /**< [0,1] Mask CC sequence mismatch events */
} XDfeCcf_InterruptMask;

/**
 * CCF Config Structure.
 */
typedef struct {
	u32 DeviceId; /**< The component instance Id */
	metal_phys_addr_t BaseAddr; /**< Instance base address */
	u32 NumAntenna; /**< Number of antenas */
	u32 NumCCPerAntenna; /**< Number of channels per antenna */
	u32 NumAntSlot; /**< Number of Antenna slots */
} XDfeCcf_Config;

/**
 * CCF Structure.
 */
typedef struct {
	XDfeCcf_Config Config; /**< Config Structure */
	XDfeCcf_StateId StateId; /**< StateId */
	struct metal_io_region *Io; /**< Libmetal IO structure */
	struct metal_device *Device; /**< Libmetal device structure */
} XDfeCcf;

/**************************** API declarations *******************************/
/* System initialization API */
XDfeCcf *XDfeCcf_InstanceInit(u16 DeviceId, const char *DeviceNodeName);
void XDfeCcf_InstanceClose(XDfeCcf *InstancePtr);

/* Register access API */
void XDfeCcf_WriteReg(const XDfeCcf *InstancePtr, u32 AddrOffset, u32 Data);
u32 XDfeCcf_ReadReg(const XDfeCcf *InstancePtr, u32 AddrOffset);

/* DFE CCF component initialization API */
void XDfeCcf_Reset(XDfeCcf *InstancePtr);
void XDfeCcf_Configure(XDfeCcf *InstancePtr, XDfeCcf_Cfg *Cfg);
void XDfeCcf_Initialize(XDfeCcf *InstancePtr, const XDfeCcf_Init *Init);
void XDfeCcf_Activate(XDfeCcf *InstancePtr, bool EnableLowPower);
void XDfeCcf_Deactivate(XDfeCcf *InstancePtr);

/* User APIs */
u32 XDfeCcf_AddCC(XDfeCcf *InstancePtr, u32 CCID,
		  const XDfeCcf_CarrierCfg *CarrierCfg);
void XDfeCcf_RemoveCC(XDfeCcf *InstancePtr, u32 CCID);
void XDfeCcf_UpdateCC(XDfeCcf *InstancePtr, u32 CCID,
		      XDfeCcf_CarrierCfg *CarrierCfg);
void XDfeCcf_UpdateAntenna(XDfeCcf *InstancePtr, u32 Ant, bool Enabled);
void XDfeCcf_GetTriggersCfg(const XDfeCcf *InstancePtr,
			    XDfeCcf_TriggerCfg *TriggerCfg);
void XDfeCcf_SetTriggersCfg(const XDfeCcf *InstancePtr,
			    XDfeCcf_TriggerCfg *TriggerCfg);
void XDfeCcf_GetCC(const XDfeCcf *InstancePtr, u32 CCID,
		   XDfeCcf_CarrierCfg *CarrierCfg);
void XDfeCcf_GetActiveSets(const XDfeCcf *InstancePtr, u32 *IsActive);
void XDfeCcf_LoadCoefficients(const XDfeCcf *InstancePtr, u32 Set,
			      const XDfeCcf_Coefficients *Coeffs);
void XDfeCcf_GetEventStatus(const XDfeCcf *InstancePtr, XDfeCcf_Status *Status);
void XDfeCcf_ClearEventStatus(const XDfeCcf *InstancePtr);
void XDfeCcf_SetInterruptMask(const XDfeCcf *InstancePtr,
			      const XDfeCcf_InterruptMask *Mask);
void XDfeCcf_GetDriverVersion(u32 *Major, u32 *Minor);

#ifdef __cplusplus
}
#endif

#endif
/** @} */
