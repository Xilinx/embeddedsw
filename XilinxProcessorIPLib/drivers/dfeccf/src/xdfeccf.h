/******************************************************************************
* Copyright (C) 2021-2022 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2022 Advanced Micro Devices, Inc. All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xdfeccf.h
* @addtogroup dfeccf Overview
* @{
*
* @cond nocomments
*
* The RFSoC DFE Channel Filter IP provides a wrapper around the Channel
* filter primitive (dfe_channel_filter). Each instance of the primitive can
* support up to 128 CC, arranged across a maximum of eight antennas.
* The wrapper provides access to the underlying blocks via TDMA AXI-Stream
* data interfaces. A memory mapped AXI interface is provided to enable the
* configuration and control of the core from a microprocessor.
*
* The features that the channel filter IP and the driver support are:
*
* - Supports a maximum sampling rate of 491.52 Ms/s.
* - Supports reallocation of TDM slots.
* - Using 18-bit data interface.
* - Using 16-bit coefficients.
* - Can independently configure complex and real coefficients.
* - Enables to program the coefficient sets via a processor interface.
* - Enables to change the coefficient sets that act on the input data
*     via a processor interface.
* - Supports TDD power down via a processor interface and TUSER input.
* - Supports the flushing of the internal buffers via a processor interface.
* - Indication of overflow provided via a status register.
* - TUSER/TLAST information accompanying the data is delay matched through
*     the IP.
*
* The channel filter driver provides the following features for each of
* the channels:
*
* - Setting of the coefficient sets via the s_axi_ctrl processor interface.
* - Selection of the coefficient sets to use for the real and imaginary data.
* - Inputs via the s_axi_ctrl processor interface.
* - Register bits to control the following inputs of the dfe_channel_filter
*      blocks:
*     - Flush buffers
*     - Enable
*     - Update coefficients
* - Status register indicating over and underflow of the channel filter for
*    both before and after gain stage.
* - Software reset.
* - TUSER and TLAST support on the data interfaces:
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who    Date     Changes
* ----- ---    -------- -----------------------------------------------
* 1.0   dc     09/03/20 Initial version
*       dc     02/02/21 Remove hard coded device node name
*       dc     02/08/21 align driver to current specification
*       dc     02/22/21 include HW in versioning
*       dc     03/25/21 Device tree item name change
*       dc     04/06/21 Register with full node name
*       dc     04/08/21 Set sequence length only once
*       dc     04/20/21 Doxygen documentation update
*       dc     05/08/21 Update to common trigger
*       dc     05/18/21 Handling CCUpdate trigger
* 1.1   dc     07/13/21 Update to common latency requirements
* 1.2   dc     10/29/21 Update doxygen comments
*       dc     11/01/21 Add multi AddCC, RemoveCC and UpdateCC
*       dc     11/05/21 Align event handlers
*       dc     11/19/21 Update doxygen documentation
*       dc     11/26/21 Model parameter NumCCPerAntenna workaround
*       dc     11/26/21 Add SetAntennaCfgInCCCfg API
*       dc     11/30/21 Convert AntennaCfg to structure
*       dc     12/02/21 Add UpdateAntennaCfg API
*       dc     12/17/21 Update after documentation review
*       dc     01/21/22 Symmetric filter Zero-padding
*       dc     01/27/22 Get calculated TDataDelay
*       dc     03/21/22 Add prefix to global variables
* 1.4   dc     04/08/22 Update documentation
* 1.5   dc     09/12/22 Update handling overflow status
*       dc     10/28/22 Switching Uplink/Downlink support
*       dc     11/11/22 Align AddCC to switchable UL/DL algorithm
*
* </pre>
* @endcond
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
#define XDFECCF_MAX_NUM_INSTANCES                                              \
	(10U) /**< Maximum number of driver instances running at the same time. */
/**
* @cond nocomments
*/
#define Xil_AssertNonvoid(Expression)                                          \
	assert(Expression) /**< Assertion for non void return parameter function. */
#define Xil_AssertVoid(Expression)                                             \
	assert(Expression) /**< Assertion for void return parameter function. */
#define Xil_AssertVoidAlways() assert(0) /**< Assertion always. */
/**
* @endcond
*/
#ifndef XST_SUCCESS
#define XST_SUCCESS (0U) /**< Success flag */
#endif
#ifndef XST_FAILURE
#define XST_FAILURE (1U) /**< Failure flag */
#endif
#else
#define XDFECCF_MAX_NUM_INSTANCES XPAR_XDFECCF_NUM_INSTANCES
#endif

#define XDFECCF_NODE_NAME_MAX_LENGTH (50U) /**< Node name maximum length */

#define XDFECCF_CC_NUM (16) /**< Maximum CC number */
#define XDFECCF_ANT_NUM_MAX (8U) /**< Maximum anntena number */
#define XDFECCF_SEQ_LENGTH_MAX (16U) /**< Maximum sequence length */
#define XDFECCF_NUM_COEFF (128U) /**< Maximum number of coefficients */

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
#else
#define XDFECCF_CUSTOM_DEV(_dev_name, _baseaddr, _idx)                         \
	{                                                                      \
		.name = _dev_name, .bus = NULL, .num_regions = 1,              \
		.regions = { {                                                 \
			.virt = (void *)_baseaddr,                             \
			.physmap = &XDfeCcf_metal_phys[_idx],                  \
			.size = 0x10000,                                       \
			.page_shift = (u32)(-1),                               \
			.page_mask = (u32)(-1),                                \
			.mem_flags = 0x0,                                      \
			.ops = { NULL },                                       \
		} },                                                           \
		.node = { NULL }, .irq_num = 0, .irq_info = NULL,              \
	}
#endif

typedef enum XDfeCcf_StateId {
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
	u32 Major; /**< Major version number. */
	u32 Minor; /**< Minor version number. */
	u32 Revision; /**< Revision number. */
	u32 Patch; /**< Patch number. */
} XDfeCcf_Version;

/**
 * Trigger configuration.
 */
typedef struct {
	u32 TriggerEnable; /**< [0,1], Enable Trigger:
		- 0 = DISABLED: Trigger Pulse and State outputs are disabled.
		- 1 = ENABLED: Trigger Pulse and State outputs are enabled and follow
			the settings described below. */
	u32 Mode; /**< [0-3], Specify Trigger Mode. In TUSER_Single_Shot mode as
		soon as the TUSER_Edge_level condition is met the State output will be
		driven to the value specified in STATE_OUTPUT. The Pulse output will
		pulse high at the same time. No further change will occur until the
		trigger register is re-written. In TUSER Continuous mode each time
		a TUSER_Edge_level condition is met the State output will be driven to
		the value specified in STATE_OUTPUT This will happen continuously until
		the trigger register is re-written. The pulse output is disabled in
		Continuous mode:
		- 0 = IMMEDIATE: Applies the value of STATE_OUTPUT immediatetly
			the register is written.
		- 1 = TUSER_SINGLE_SHOT: Applies the value of STATE_OUTPUT once when
			the TUSER_EDGE_LEVEL condition is satisfied.
		- 2 = TUSER_CONTINUOUS: Applies the value of STATE_OUTPUT continually
			when TUSER_EDGE_LEVEL condition is satisfied.
		- 3 = RESERVED: Reserved - will default to 0 behaviour. */
	u32 TuserEdgeLevel; /**< [0-3], Specify either Edge or Level of the TUSER
		input as the source condition of the trigger. Difference between Level
		and Edge is Level will generate a trigger immediately the TUSER level
		is detected. Edge will ensure a TUSER transition has come first:
		- 0 = LOW: Trigger occurs immediately after a low-level is seen on TUSER
			provided tvalid is high.
		- 1 = HIGH: Trigger occurs immediately after a high-level is seen on
			TUSER provided tvalid is high.
		- 2 = FALLING: Trigger occurs immediately after a high to low transition
			on TUSER provided tvalid is high.
		- 3 = RISING: Trigger occurs immediately after a low to high transition
			on TUSER provided tvalid is high. */
	u32 StateOutput; /**< [0,1], Specify the State output value:
		- 0 = DISABLED: Place the State output into the Disabled state.
		- 1 = ENABLED: Place the State output into the Enabled state. */
	u32 TUSERBit; /**< [0-255], Specify which DIN TUSER bit to use as the source
		for the trigger when MODE = 1 or 2. */
} XDfeCcf_Trigger;

/**
 * All IP triggers.
 */
typedef struct {
	XDfeCcf_Trigger Activate; /**< Switch between "Initialized",
		ultra-low power state, and "Operational". One-shot trigger,
		disabled following a single event. */
	XDfeCcf_Trigger LowPower; /**< Switch between "Low-power"
		and "Operational" state. */
	XDfeCcf_Trigger CCUpdate; /**< Transition to next CC
		configuration. Will initiate flush based on CC configuration. */
	XDfeCcf_Trigger Switch; /**< Switch between Downlink and Uplink datapth
		configuration. Will initiate flush based on CC configuration. */
} XDfeCcf_TriggerCfg;

/**
 * Defines a CCID sequence.
 */
typedef struct {
	u32 Length; /**< [1-16] Sequence length. */
	s32 CCID[XDFECCF_SEQ_LENGTH_MAX]; /**< [0-15].Array of CCID's
		arranged in the order the CCIDs are required to be processed
		in the channel filter. */
	s32 NotUsedCCID; /**< Lowest CCID number not allocated */
} XDfeCcf_CCSequence;

/*********** end - common code to all Logiccores ************/
/**
 * Channel Filter model parameters structure. Data defined in Device
 * tree/xparameters.h.
 */
typedef struct {
	u32 NumAntenna; /**< [1-8] Number of antennas */
	u32 NumCCPerAntenna; /**< [1-16] Number of CCs per antenna */
	u32 AntennaInterleave; /**< [1-8] Number of Antenna slots */
	u32 Switchable; /**< [0,1] If true DL/UL switching is supported */
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
	XDfeCcf_CCSequence Sequence; /**< CCID sequence. */
	u32 GainStage; /**< [0,1] Enable gain stage */
	u32 TuserSelect; /**< [0,1] Select DL or UL TUSER */
} XDfeCcf_Init;

/**
 * Defines filter coefficients.
 */
typedef struct {
	u32 Num; /**< [0-(128|256)]. True number of coefficients,
		    when non-symmetric max is 128. */
	u32 Symmetric; /**< [0,1] Select the use of symmetric (1) or
			  non-symmetric (0) filter */
	s16 Value[XDFECCF_NUM_COEFF]; /**< [Signed real numbers]. Array of
			  coefficients, when symmetric only the first
			  (Num+1)/2 coefficients are provided */
} XDfeCcf_Coefficients;

/**
 * Configuration for a single CC.
 */
typedef struct {
	u32 Gain; /**< [0-(1<<16)-1] Gain setting for this CC */
	u32 ImagCoeffSet; /**< [0-7] Identify the coefficient set for the
			        complex data on this CC */
	u32 RealCoeffSet; /**< [0-7] Identify the coefficient set for the real
			     data on this CC */
} XDfeCcf_CarrierCfg;

/**
 * Internal configuration for a single CC.
 */
typedef struct {
	u32 Enable; /**< [0,1] (Private) Enable/Disable CC while still
			reserving its slot in the TDM - set by helper functions
			when building the configuration the TDM */
	u32 Flush; /**< [0,1] (Private) Indicate CC should be flush following
			configuration update - set by helper functions when
			building the configuration channel_id? */
	u32 MappedId; /**< [0-7] (Private) Defines the hardblock ID value to be
			used for the CC. Used to map arbitrary/system CCID
			values to available hard block TID values. Enumerated
			incrementally from 0 to max supported CC for a given
			IP configuration */
	u32 Gain; /**< [0-(1<<16)-1] Gain setting for this CC */
	u32 ImagCoeffSet; /**< [0-7] Identify the coefficient set for the
			        complex data on this CC */
	u32 RealCoeffSet; /**< [0-7] Identify the coefficient set for the real
			     data on this CC */
} XDfeCcf_InternalCarrierCfg;

/**
 * Configuration for a single Antenna.
 */
typedef struct {
	u32 Enable[XDFECCF_ANT_NUM_MAX]; /**< [0: disable, 1: enable] Antenna enablement */
} XDfeCcf_AntennaCfg;

/**
 * Full CC configuration.
 */
typedef struct {
	XDfeCcf_CCSequence Sequence; /**< CCID sequence */
	XDfeCcf_InternalCarrierCfg
		CarrierCfg[XDFECCF_CC_NUM]; /**< CC configurations */
	XDfeCcf_AntennaCfg AntennaCfg; /**< Antenna configuration */
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
	u32 OverflowAntenna; /**< [0,15] Lowest number antenna on which first
		overflow, or underflow, occurred */
	u32 OverflowCCID; /**< [0,15] CCID on which first overflow, or
		underflow, occurred */
	u32 OverflowSwitch; /**< In SWITCHABLE mode the channel in which
		overflow first occurred. This register is ignored in
		NON-SWITCHABLE mode.
		- 0 = DOWNLINK: Overflow occurred in the downlink channel.
		- 1 = UPLINK: Overflow occurred in the uplink channel. */
} XDfeCcf_OverflowStatus;

/**
 * Event status and interrupt mask.
 *
 * @note The structure for XDfeCcf_InterruptMask is the same as that of
 *       XDfeCcf_Status
 */
typedef struct {
	u32 Overflow; /**< [0,1] Mask overflow events */
	u32 CCUpdate; /**< [0,1] Mask CC update events */
	u32 CCSequenceError; /**< [0,1] Mask CC sequence mismatch events */
} XDfeCcf_Status;

typedef XDfeCcf_Status XDfeCcf_InterruptMask;

/**
 * CCF Config Structure.
 */
typedef struct {
	u32 DeviceId; /**< The component instance Id */
	metal_phys_addr_t BaseAddr; /**< Instance base address */
	u32 NumAntenna; /**< Number of antennas */
	u32 NumCCPerAntenna; /**< Number of CCs per antenna */
	u32 AntennaInterleave; /**< Number of Antenna slots */
	u32 Switchable; /**< [0,1] If true DL/UL switching is supported */
} XDfeCcf_Config;

/**
 * CCF Structure.
 */
typedef struct {
	XDfeCcf_Config Config; /**< Config Structure */
	XDfeCcf_StateId StateId; /**< StateId */
	s32 NotUsedCCID; /**< Lowest CCID number not allocated, in
		non-switchable mode, also for DL in switchable mode */
	s32 NotUsedCCID_UL; /**< Lowest CCID number not allocated for UL in
		switchable mode */
	u32 SequenceLength; /**< Exact sequence length */
	char NodeName[XDFECCF_NODE_NAME_MAX_LENGTH]; /**< Node name */
	struct metal_io_region *Io; /**< Libmetal IO structure */
	struct metal_device *Device; /**< Libmetal device structure */
} XDfeCcf;

/**************************** API declarations *******************************/
/* System initialization API */
XDfeCcf *XDfeCcf_InstanceInit(const char *DeviceNodeName);
void XDfeCcf_InstanceClose(XDfeCcf *InstancePtr);

/**
* @cond nocomments
*/
/* Register access API */
void XDfeCcf_WriteReg(const XDfeCcf *InstancePtr, u32 AddrOffset, u32 Data);
u32 XDfeCcf_ReadReg(const XDfeCcf *InstancePtr, u32 AddrOffset);
/**
* @endcond
*/

/* DFE CCF component initialization API */
void XDfeCcf_Reset(XDfeCcf *InstancePtr);
void XDfeCcf_Configure(XDfeCcf *InstancePtr, XDfeCcf_Cfg *Cfg);
void XDfeCcf_Initialize(XDfeCcf *InstancePtr, XDfeCcf_Init *Init);
void XDfeCcf_Activate(XDfeCcf *InstancePtr, bool EnableLowPower);
void XDfeCcf_Deactivate(XDfeCcf *InstancePtr);
XDfeCcf_StateId XDfeCcf_GetStateID(XDfeCcf *InstancePtr);

/* User APIs */
void XDfeCcf_GetCurrentCCCfg(const XDfeCcf *InstancePtr, XDfeCcf_CCCfg *CCCfg);
void XDfeCcf_GetCurrentCCCfgSwitchable(const XDfeCcf *InstancePtr,
				       XDfeCcf_CCCfg *CCCfgDownlink,
				       XDfeCcf_CCCfg *CCCfgUplink);
void XDfeCcf_GetEmptyCCCfg(const XDfeCcf *InstancePtr, XDfeCcf_CCCfg *CCCfg);
void XDfeCcf_GetCarrierCfg(const XDfeCcf *InstancePtr, XDfeCcf_CCCfg *CCCfg,
			   s32 CCID, u32 *CCSeqBitmap,
			   XDfeCcf_CarrierCfg *CarrierCfg);
void XDfeCcf_SetAntennaCfgInCCCfg(const XDfeCcf *InstancePtr,
				  XDfeCcf_CCCfg *CCCfg,
				  XDfeCcf_AntennaCfg *AntennaCfg);
u32 XDfeCcf_AddCCtoCCCfg(XDfeCcf *InstancePtr, XDfeCcf_CCCfg *CCCfg, s32 CCID,
			 u32 CCSeqBitmap, const XDfeCcf_CarrierCfg *CarrierCfg);
void XDfeCcf_RemoveCCfromCCCfg(XDfeCcf *InstancePtr, XDfeCcf_CCCfg *CCCfg,
			       s32 CCID);
void XDfeCcf_UpdateCCinCCCfg(const XDfeCcf *InstancePtr, XDfeCcf_CCCfg *CCCfg,
			     s32 CCID, const XDfeCcf_CarrierCfg *CarrierCfg);
u32 XDfeCcf_SetNextCCCfgAndTrigger(XDfeCcf *InstancePtr, XDfeCcf_CCCfg *CCCfg);
u32 XDfeCcf_SetNextCCCfgAndTriggerSwitchable(XDfeCcf *InstancePtr,
					     XDfeCcf_CCCfg *CCCfgDownlink,
					     XDfeCcf_CCCfg *CCCfgUplink);
u32 XDfeCcf_AddCC(XDfeCcf *InstancePtr, s32 CCID, u32 CCSeqBitmap,
		  const XDfeCcf_CarrierCfg *CarrierCfg);
u32 XDfeCcf_RemoveCC(XDfeCcf *InstancePtr, s32 CCID);
u32 XDfeCcf_UpdateCC(XDfeCcf *InstancePtr, s32 CCID,
		     const XDfeCcf_CarrierCfg *CarrierCfg);
u32 XDfeCcf_UpdateAntenna(XDfeCcf *InstancePtr, u32 Ant, bool Enabled);
u32 XDfeCcf_UpdateAntennaCfg(XDfeCcf *InstancePtr,
			     XDfeCcf_AntennaCfg *AntennaCfg);
u32 XDfeCcf_UpdateAntennaCfgSwitchable(XDfeCcf *InstancePtr,
				       XDfeCcf_AntennaCfg *AntennaCfgDownlink,
				       XDfeCcf_AntennaCfg *AntennaCfgUplink);
void XDfeCcf_GetTriggersCfg(const XDfeCcf *InstancePtr,
			    XDfeCcf_TriggerCfg *TriggerCfg);
void XDfeCcf_SetTriggersCfg(const XDfeCcf *InstancePtr,
			    XDfeCcf_TriggerCfg *TriggerCfg);
void XDfeCcf_GetCC(const XDfeCcf *InstancePtr, s32 CCID,
		   XDfeCcf_CarrierCfg *CarrierCfg);
void XDfeCcf_GetActiveSets(const XDfeCcf *InstancePtr, u32 *IsActive);
void XDfeCcf_LoadCoefficients(XDfeCcf *InstancePtr, u32 Set, u32 Shift,
			      const XDfeCcf_Coefficients *Coeffs);
void XDfeCcf_GetOverflowStatus(const XDfeCcf *InstancePtr,
			       XDfeCcf_OverflowStatus *Status);
void XDfeCcf_GetEventStatus(const XDfeCcf *InstancePtr, XDfeCcf_Status *Status);
void XDfeCcf_ClearEventStatus(const XDfeCcf *InstancePtr,
			      const XDfeCcf_Status *Status);
void XDfeCcf_SetInterruptMask(const XDfeCcf *InstancePtr,
			      const XDfeCcf_InterruptMask *Mask);
void XDfeCcf_GetInterruptMask(const XDfeCcf *InstancePtr,
			      XDfeCcf_InterruptMask *Mask);
void XDfeCcf_SetTUserDelay(const XDfeCcf *InstancePtr, u32 Delay);
u32 XDfeCcf_GetTUserDelay(const XDfeCcf *InstancePtr);
u32 XDfeCcf_GetTDataDelay(XDfeCcf *InstancePtr, u32 Tap, s32 CCID,
			  u32 Symmetric, u32 Num, u32 *TDataDelay);
u32 XDfeCcf_GetTDataDelayFromCCCfg(XDfeCcf *InstancePtr, u32 Tap, s32 CCID,
				   XDfeCcf_CCCfg *CCCfg, u32 Symmetric, u32 Num,
				   u32 *TDataDelay);
void XDfeCcf_SetRegBank(const XDfeCcf *InstancePtr, u32 RegBank);
void XDfeCcf_GetVersions(const XDfeCcf *InstancePtr, XDfeCcf_Version *SwVersion,
			 XDfeCcf_Version *HwVersion);

#ifdef __cplusplus
}
#endif

#endif
/** @} */
