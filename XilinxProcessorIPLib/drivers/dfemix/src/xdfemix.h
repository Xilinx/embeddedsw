/******************************************************************************
* Copyright (C) 2021-2022 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2022 - 2024 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xdfemix.h
* @addtogroup dfemix Overview
* @{
*
* @cond nocomments
*
* The RFSoC DFE DUC-DDC Mixer IP provides a wrapper around the Channel Mixer and
* DUC_DDC primitives (Combined DUC-DDC and Mixer). Each IP instance can
* support up to 64 CC, arranged across a maximum of 8 Antennas using
* four primitives. The wrapper provides access to the underlying primitives via
* TDMA AXI-Stream data interfaces. An AXI memory-mapped interface is provided,
* enabling configuration and control of the block from a microprocessor.
* The features that the DUC-DDC Mixer IP and driver support are:
* - Supports instantaneous bandwidth of up to 400MHz.
* - Supports a maximum sample rate of 491.52MSPS for the antenna signals.
* - Supports CC sequence reconfiguration.
* - Supports up conversion rate set on a per CC basis, programmed
*   via processor interface.
* - UL TDM output pattern programmed via register interface.
* - Supports down conversion rate set on a per CC basis, programmed via
*   processor interface.
* - Supports TDD power down via a processor interface and TUSER input.
* - Supports automatic flushing of the internal buffers.
* - TUSER/TLAST information accompanying the data is delay matched through
*   the IP.
*
* The NUM_ANTENNAS parameter gives the number of antennas supported by
* the IP Core. The NUM_CC parameter gives the Number of CC per antenna
* Combining these two parameters gives the total number of channels the IP core
* needs to process and hence dictate the internal architecture. The IP core and
* driver provides the following features:
* - The AXI Memory map provides settings to enable/disable individual CCs.
* - CC settings apply to all antenna - the only setting that is specific to
*   an antenna is ANTENNA_GAIN.
* - Flushing of channels across all antenna for a particular CC is handled
* by the core, occurring prior to a CC being added.
* - Support for software reset. The IP software reset register allows
*   the assertion of the internal reset.
*
* An API which reads/writes registers is provided for debugging purpose.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who    Date     Changes
* ----- ---    -------- -----------------------------------------------
* 1.0   dc     07/22/20 Initial version
*       dc     02/02/21 Remove hard coded device node name
*       dc     02/15/21 align driver to current specification
*       dc     02/22/21 include HW in versioning
*       dc     03/18/21 New model parameter list
*       dc     04/06/21 Register with full node name
*       dc     04/08/21 Set sequence length only once
*       dc     04/20/21 Doxygen documentation update
*       dc     04/22/21 Add CC_GAIN field
*       dc     04/27/21 Update CARRIER_CONFIGURATION handling
*       dc     05/08/21 Update to common trigger
*       dc     05/18/21 Handling CCUpdate trigger
* 1.1   dc     07/13/21 Update to common latency requirements
* 1.2   dc     10/29/21 Update doxygen comments
*       dc     11/01/21 Add multi AddCC, RemoveCC and UpdateCC
*       dc     11/05/21 Align event handlers
*       dc     11/19/21 Update doxygen documentation
*       dc     11/19/21 Add SetAntennaCfgInCCCfg API
*       dc     11/30/21 Convert AntennaCfg to structure
*       dc     12/02/21 Add UpdateAntennaCfg API
*       dc     12/17/21 Update after documentation review
* 1.3   dc     02/10/22 Add latency information
*       dc     03/21/22 Add prefix to global variables
* 1.4   dc     03/28/22 Update documentation
*       dc     08/19/22 Update register map
* 1.5   dc     09/28/22 Auxiliary NCO support
*       dc     10/24/22 Switching Uplink/Downlink support
*       dc     11/10/22 Align AddCC to switchable UL/DL algorithm
*       dc     11/11/22 Update get overflow status API
*       dc     11/11/22 Update NCOIdx and CCID check
* 1.6   dc     06/15/23 Correct comment about gain
*       cog    07/04/23 Add support for SDT
* 1.7   cog    02/21/24 Fixed SDT runtime issue
*
* </pre>
* @endcond
******************************************************************************/
#ifndef XDFEMIX_H_
#define XDFEMIX_H_

#ifdef __cplusplus
extern "C" {
#endif

/**************************** Includes ***************************************/
#ifdef __BAREMETAL__
#include "xil_types.h"
#ifndef SDT
#include "xparameters.h"
#endif
#include "xstatus.h"
#else
#include <linux/types.h>
#include <assert.h>
#endif

#include "stdbool.h"
#include <metal/sys.h>

/**************************** Macros Definitions *****************************/
#ifndef __BAREMETAL__
#define XDFEMIX_MAX_NUM_INSTANCES                                              \
	(10U) /**< Maximum number of driver instances running at the same time. */
#define XDFEMIX_INSTANCE_EXISTS(X) (X < XDFEMIX_MAX_NUM_INSTANCES)
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
#ifndef SDT
#define XDFEMIX_MAX_NUM_INSTANCES XPAR_XDFEMIX_NUM_INSTANCES
#define XDFEMIX_INSTANCE_EXISTS(X) (X < XDFEMIX_MAX_NUM_INSTANCES)
#else
#define XDFEMIX_MAX_NUM_INSTANCES                                              \
	(11U) /**< Maximum number of driver instances in global config struct. */
#define XDFEMIX_INSTANCE_EXISTS(X) (XDfeMix_ConfigTable[X].Name != NULL)
#endif
#endif

#define XDFEMIX_NODE_NAME_MAX_LENGTH (50U) /**< Node name maximum length */

#define XDFEMIX_CC_NUM (16) /**< Maximum CC number */
#define XDFEMIX_ANT_NUM_MAX (8U) /**< Maximum anntena number */
#define XDFEMIX_SEQ_LENGTH_MAX (16U) /**< Maximum sequence length */

/**
* @cond nocomments
*/
#define XDFEMIX_RATE_MAX (7U) /**< Maximum rate Id */
/**
* @endcond
*/
#define XDFEMIX_AUX_NCO_MAX (4) /**< Maximum Fabric NCO number */
#define XDFEMIX_NCO_MAX (20) /**< Maximum NCO number, 16 CC NCO + 4 aux NCO */
#define XDFEMIX_CC_GAIN_MAX (3U) /**< Maximum CC gain */

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
#define XDFEMIX_CUSTOM_DEV(_dev_name, _baseaddr, _idx)                         \
	{                                                                      \
		.name = _dev_name, .bus = NULL, .num_regions = 1,              \
		.regions = { {                                                 \
			.virt = (void *)_baseaddr,                             \
			.physmap = &XDfeMix_metal_phys[_idx],                  \
			.size = 0x10000,                                       \
			.page_shift = (u32)(-1),                               \
			.page_mask = (u32)(-1),                                \
			.mem_flags = 0x0,                                      \
			.ops = { NULL },                                       \
		} },                                                           \
		.node = { NULL }, .irq_num = 0, .irq_info = NULL,              \
	}
#endif

typedef enum XDfeMix_StateId {
	XDFEMIX_STATE_NOT_READY = 0, /**< Not ready state.*/
	XDFEMIX_STATE_READY, /**< Ready state.*/
	XDFEMIX_STATE_RESET, /**< Reset state.*/
	XDFEMIX_STATE_CONFIGURED, /**< Configured state.*/
	XDFEMIX_STATE_INITIALISED, /**< Initialised state.*/
	XDFEMIX_STATE_OPERATIONAL /**< Operational state.*/
} XDfeMix_StateId;

/**
 * Logicore version.
 */
typedef struct {
	u32 Major; /**< Major version number. */
	u32 Minor; /**< Minor version number. */
	u32 Revision; /**< Revision number. */
	u32 Patch; /**< Patch number. */
} XDfeMix_Version;

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
} XDfeMix_Trigger;

/**
 * All IP triggers.
 */
typedef struct {
	XDfeMix_Trigger Activate; /**< Switch between "Initialized",
		ultra-low power state, and "Operational". One-shot trigger,
		disabled following a single event. */
	XDfeMix_Trigger LowPower; /**< Switch between "Low-power"
		and "Operational" state. */
	XDfeMix_Trigger CCUpdate; /**< Transition to next CC
		configuration. Will initiate flush based on CC configuration. */
	XDfeMix_Trigger Switch; /**< Switch between Downlink and Uplink datapth
		configuration. Will initiate flush based on CC configuration. */
} XDfeMix_TriggerCfg;

/**
 * Defines a CCID sequence.
 */
typedef struct {
	u32 Length; /**< [1-16] Sequence length. */
	s32 CCID[XDFEMIX_SEQ_LENGTH_MAX]; /**< [0-15].Array of CCID's
		arranged in the order the CCIDs are required to be processed
		in the DUC/DDC Mixer. */
	s32 NotUsedCCID; /**< Lowest CCID number not allocated */
} XDfeMix_CCSequence;

/*********** end - common code to all Logiccores ************/
/**
 * Mixer model parameters structure. Data defined in Device tree/xparameters.h.
 */
typedef struct {
	u32 Mode; /**< [0,1] 0=downlink, 1=uplink */
	u32 NumAntenna; /**< [1,2,4,8] Number of antennas */
	u32 MaxUseableCcids; /**< [2,4,8] Maximum CC usable */
	u32 Lanes; /**< [1-8] Number of lanes */
	u32 AntennaInterleave; /**< [1,2,4,8] Number of Antenna slots */
	u32 MixerCps; /**< [1,2,4] */
	u32 NumAuxiliary; /**< [0-3] */
	u32 DataIWidth; /**< [16,24] 16 for 16-bit sample data and 24 for
		18-bit sample data.*/
	u32 DataOWidth; /**< [16,24] 16 for 16-bit sample data and 24 for
		18-bit sample data.*/
	u32 TUserWidth; /**< [0-64] */
} XDfeMix_ModelParameters;

/**
 * Configuration.
 */
typedef struct {
	XDfeMix_Version Version; /**< Logicore version */
	XDfeMix_ModelParameters ModelParams; /**< Logicore
		parameterization */
} XDfeMix_Cfg;

/**
 * Initialization, "one-time" configuration parameters.
 */
typedef struct {
	XDfeMix_CCSequence Sequence; /**< CCID Sequence. */
	u32 TuserSelect; /**< [0,1] Select DL or UL TUSER */
} XDfeMix_Init;

/**
 * Phase Offset.
 */
typedef struct {
	u32 PhaseOffset; /**< [0-2^17-1] Phase offset */
} XDfeMix_PhaseOffset;

/**
 * Defines frequency for single CC's NCO.
 */
typedef struct {
	u32 FrequencyControlWord; /**< [0-2^32-1] Phase increment */
	u32 SingleModCount; /**< [0-2^32-1] Single modulus cycle count (S) */
	u32 DualModCount; /**< [0-2^32-1] Dual modulus cycle count (T-S) */
	XDfeMix_PhaseOffset PhaseOffset; /**< [0-2^17-1] Phase offset */
	u32 TriggerUpdateFlag; /**< Update the associated accumulator from
		the FREQUENCY registers:
		- 0 = NO_UPDATE: Do not update
		- 1 = IMMEDIATE_UPDATE: Apply an immediate update
		- 2 = TRIGGER_UPDATE: Apply an update on the next CC_UPDATE
		      trigger */
} XDfeMix_Frequency;

/**
 * Defined phase for single CC's NCO.
 */
typedef struct {
	u32 PhaseAcc; /**< [0-2^32-1] Phase accumulator value */
	u32 DualModCount; /**< [0-2^32-1] Dual modulus count value (T-S)*/
	u32 DualModSel; /**< [0,1] Dual modulus select value */
	u32 TriggerUpdateFlag; /**< Update the associated accumulator from
		the PHASE registers:
		- 0 = NO_UPDATE: Do not update
		- 1 = IMMEDIATE_UPDATE: Apply an immediate update
		- 2 = TRIGGER_UPDATE: Apply an update on the next CC_UPDATE
		      trigger */
} XDfeMix_Phase;

/**
 * Defines settings for single CC's NCO.
 */
typedef struct {
	XDfeMix_Frequency FrequencyCfg; /**< Frequency configuration */
	XDfeMix_Phase PhaseCfg; /**< Phase configuration */
	u32 NCOGain; /**< [0,1,2,3] Scaling of NCO output:
		- 0 = 0dB
		- 1 = -3dB
		- 2 = -6dB
		- 3 =- 9dB */
} XDfeMix_NCO;

/**
 * Defines settings for internal single CC's DUC/DDC.
 */
typedef struct {
	u32 NCOIdx; /**< [0-7] DUC/DDC NCO assignment */
	u32 Rate; /**< [0-5].Interpolation/decimation rate:
			- 0: This CCID is disabled
			- 1: 1x interpolation/decimation
			- 2: 2x interpolation/decimation
			- 3: 4x interpolation/decimation
			- 4: 8x interpolation/decimation
			- 5: 16x interpolation/decimation */
	u32 CCGain; /**< [0-3] Adjust gain of CCID after mixing (applies to all
			 antennas for that CCID). Only applicable to downlink.
				- 0 = MINUS18DB: Apply -18dB gain.
				- 1 = MINUS12DB: Apply -12dB gain.
				- 2 = MINUS6DB: Apply -6dB gain.
				- 3 = ZERODB: Apply -0dB gain. */
} XDfeMix_InternalDUCDDCCfg;

/**
 * Defines settings for single CC's DUC/DDC.
 */
typedef struct {
	u32 NCOIdx; /**< [0-7] DUC/DDC NCO assignment */
	u32 CCGain; /**< [0-3] Adjust gain of CCID after mixing (applies to all
			 antennas for that CCID). Only applicable to downlink.
				- 0 = MINUS18DB: Apply -18dB gain.
				- 1 = MINUS12DB: Apply -12dB gain.
				- 2 = MINUS6DB: Apply -6dB gain.
				- 3 = ZERODB: Apply -0dB gain. */
} XDfeMix_DUCDDCCfg;

/**
 * Defines settings for single auxiliary NCO.
 */
typedef struct {
	u32 Enable; /**< [0,1] Auxiliary NCO Enable:
		- 0 = disabled
		- 1 = enabled */
	u32 AuxGain; /**< [0-3] Auxiliary NCO gain.
		- 0 = MINUS18DB: Apply -18dB gain.
		- 1 = MINUS12DB: Apply -12dB gain.
		- 2 = MINUS6DB: Apply -6dB gain.
		- 3 = ZERODB: Apply -0dB gain. */
} XDfeMix_AuxiliaryCfg;

/**
 * Configuration for a single CC (implementation note: notice that there are
 * two parts, one part (DUCDDCCfg) mapping to the CCCfg state, and another that
 * is written directly to NCO registers (XDfeMix_NCO).
 */
typedef struct {
	XDfeMix_DUCDDCCfg DUCDDCCfg; /**< Defines settings for single CC's
		DUC/DDC */
} XDfeMix_CarrierCfg;

/**
 * Configuration to all antennas.
 */
typedef struct {
	u32 Gain[XDFEMIX_ANT_NUM_MAX]; /**< [0: -6dB,1:0dB] Antenna gain adjustment */
} XDfeMix_AntennaCfg;

/**
 * Full CC configuration.
 */
typedef struct {
	XDfeMix_CCSequence Sequence; /**< CCID sequence */
	XDfeMix_InternalDUCDDCCfg DUCDDCCfg[XDFEMIX_CC_NUM]; /**< DUC/DDC
		configurations for all CCs */
	XDfeMix_NCO NCO[XDFEMIX_NCO_MAX]; /**< Defines settings for all
		CC's and Auxiliary's NCO */
	XDfeMix_AuxiliaryCfg AuxiliaryCfg[XDFEMIX_AUX_NCO_MAX]; /**< Auxiliary
		configuration */
	XDfeMix_AntennaCfg AntennaCfg; /**< Antenna configuration */
} XDfeMix_CCCfg;

/**
 * DUC/DDC status.
 */
typedef struct {
	u32 Stage; /**< [0-3] Earliest stage in which overflow occurred. */
	u32 Antenna; /**< [0-7] Antenna on which overflow occured, with lowest
		antenna taking priority. */
	u32 NcoId; /**< [0-15] Number of NCO associated with overflowing channel,
		with lowest taking priority. */
	u32 Mode; /** [0-1] In Switchable the mode of core when the overflow
		occured.
		- 0 = DOWNLINK: Overflow occured while core is in downlink.
		- 1 = UPLINK: Overflow occured while core is in uplink. */
} XDfeMix_DUCDDCStatus;

/**
 * Mixer status.
 */
typedef struct {
	u32 Stage; /**< [0-3] Earliest stage in which overflow occurred.
		- 0 = COMPLEX_MULT: Complex multiplier output overflowed and
			has been saturated.
		- 1 = FIRST_ADDER: First antenna adder output overflowed and
			has been saturated. Downlink Only.
		- 2 = SECOND_ADDER: Second antenna adder output overflowed
			 and has been saturated. Downlink MAX_CCIDs > 4 Only.
		- 3 = THIRD_ADDER: Third antenna adder output overflowed
			and has been saturated. Downlink MAX_CCIDs > 8 Only. */
	u32 Antenna; /**< [0-7] Lowest numbered antenna in which overflow
		occurred. */
	u32 NcoId; /**< [0-15] NCO on which overflow occured, with lowest
		antenna taking priority (only relevant to overflow in
		COMPLEX_MULT stage). */
	u32 Mode; /** [0-1] In Switchable the mode of core when the overflow
		occured.
		- 0 = DOWNLINK: Overflow occured while core is in downlink.
		- 1 = UPLINK: Overflow occured while core is in uplink. */
} XDfeMix_MixerStatus;

/**
 * Event status and interrupt mask.
 *
 * @note The structure for XDfeMix_InterruptMask is the same as that of
 *       XDfeMix_Status
 */
typedef struct {
	u32 DUCDDCOverflow; /**< [0,1] Mask overflow in DUC/DDC */
	u32 MixerOverflow; /**< [0,1] Mask overflow in mixer */
	u32 CCUpdate; /**< [0,1] Mask CC update interrupt */
	u32 LowPower; /**< [0,1] Mask low power interrupt */
	u32 Switchable; /**< [0,1] Mask switchable interrupt */
	u32 CCSequenceError; /**< [0,1] Mask sequence error */
	u32 CCUpdateError; /**< [0,1] Mask CC update error */
	u32 LowPowerError; /**< [0,1] Mask low power error */
	u32 SwitchableError; /**< [0,1] Mask switchable error */
} XDfeMix_Status;

typedef XDfeMix_Status XDfeMix_InterruptMask;

/**
 * Mixer Config Structure.
 */
typedef struct {
#ifndef SDT
	u32 DeviceId; /**< Device Id */
#else
	char *Name; /**< Unique name of the device */
#endif
	metal_phys_addr_t BaseAddr; /**< Device base address */
	u32 Mode; /**< [0,1] 0=downlink, 1=uplink, 2=switchable */
	u32 NumAntenna; /**< [1,2,4,8] */
	u32 MaxUseableCcids; /**< [2,4,8] */
	u32 Lanes; /**< [1-8] */
	u32 AntennaInterleave; /**< [1,2,4,8] */
	u32 MixerCps; /**< [1,2,4] */
	u32 NumAuxiliary; /**< [0-3] */
	u32 DataIWidth; /**< [16,24] 16 for 16-bit sample data and 24 for
		18-bit sample data.*/
	u32 DataOWidth; /**< [16,24] 16 for 16-bit sample data and 24 for
		18-bit sample data.*/
	u32 TUserWidth; /**< [0-64] */
} XDfeMix_Config;

/**
 * Mixer Structure.
 */
typedef struct {
	XDfeMix_Config Config; /**< Config Structure */
	XDfeMix_StateId StateId; /**< StateId */
	s32 NotUsedCCID; /**< Lowest CCID number not allocated, in
		non-switchable mode, also for DL in switchable mode */
	s32 NotUsedCCID_UL; /**< Lowest CCID number not allocated for UL in
		switchable mode */
	u32 SequenceLength; /**< Exact sequence length */
	char NodeName[XDFEMIX_NODE_NAME_MAX_LENGTH]; /**< Node name */
	struct metal_io_region *Io; /**< Libmetal IO structure */
	struct metal_device *Device; /**< Libmetal device structure */
} XDfeMix;

/************************** Variable Definitions *****************************/
#ifdef __BAREMETAL__
extern XDfeMix_Config XDfeMix_ConfigTable[XDFEMIX_MAX_NUM_INSTANCES];
#endif

/**************************** API declarations *******************************/
/* System initialization API */
XDfeMix *XDfeMix_InstanceInit(const char *DeviceNodeName);
void XDfeMix_InstanceClose(XDfeMix *InstancePtr);

/* Register access API */
/**
* @cond nocomments
*/
void XDfeMix_WriteReg(const XDfeMix *InstancePtr, u32 AddrOffset, u32 Data);
u32 XDfeMix_ReadReg(const XDfeMix *InstancePtr, u32 AddrOffset);
/**
* @endcond
*/

/* DFE Mixer component initialization API */
void XDfeMix_Reset(XDfeMix *InstancePtr);
void XDfeMix_Configure(XDfeMix *InstancePtr, XDfeMix_Cfg *Cfg);
void XDfeMix_Initialize(XDfeMix *InstancePtr, XDfeMix_Init *Init);
void XDfeMix_Activate(XDfeMix *InstancePtr, bool EnableLowPower);
void XDfeMix_Deactivate(XDfeMix *InstancePtr);
XDfeMix_StateId XDfeMix_GetStateID(XDfeMix *InstancePtr);

/* User APIs */
void XDfeMix_GetCurrentCCCfg(const XDfeMix *InstancePtr, XDfeMix_CCCfg *CCCfg);
void XDfeMix_GetCurrentCCCfgSwitchable(const XDfeMix *InstancePtr,
				       XDfeMix_CCCfg *CCCfgDownlink,
				       XDfeMix_CCCfg *CCCfgUplink);
void XDfeMix_GetEmptyCCCfg(const XDfeMix *InstancePtr, XDfeMix_CCCfg *CCCfg);
void XDfeMix_GetCarrierCfgAndNCO(const XDfeMix *InstancePtr,
				 XDfeMix_CCCfg *CCCfg, s32 CCID,
				 u32 *CCSeqBitmap,
				 XDfeMix_CarrierCfg *CarrierCfg,
				 XDfeMix_NCO *NCO);
void XDfeMix_SetAntennaCfgInCCCfg(const XDfeMix *InstancePtr,
				  XDfeMix_CCCfg *CCCfg,
				  XDfeMix_AntennaCfg *AntennaCfg);
u32 XDfeMix_AddCCtoCCCfg(XDfeMix *InstancePtr, XDfeMix_CCCfg *CCCfg, s32 CCID,
			 u32 CCSeqBitmap, const XDfeMix_CarrierCfg *CarrierCfg,
			 const XDfeMix_NCO *NCO);
void XDfeMix_RemoveCCfromCCCfg(XDfeMix *InstancePtr, XDfeMix_CCCfg *CCCfg,
			       s32 CCID);
void XDfeMix_AddAuxNCOtoCCCfg(XDfeMix *InstancePtr, XDfeMix_CCCfg *CCCfg,
			      const s32 AuxId, const XDfeMix_NCO *NCO,
			      const XDfeMix_AuxiliaryCfg *AuxCfg);
void XDfeMix_RemoveAuxNCOfromCCCfg(XDfeMix *InstancePtr, XDfeMix_CCCfg *CCCfg,
				   const s32 AuxId);

u32 XDfeMix_UpdateCCinCCCfg(XDfeMix *InstancePtr, XDfeMix_CCCfg *CCCfg,
			    s32 CCID, const XDfeMix_CarrierCfg *CarrierCfg);
u32 XDfeMix_SetNextCCCfgAndTrigger(XDfeMix *InstancePtr,
				   const XDfeMix_CCCfg *CCCfg);
u32 XDfeMix_SetNextCCCfgAndTriggerSwitchable(XDfeMix *InstancePtr,
					     XDfeMix_CCCfg *CCCfgDownlink,
					     XDfeMix_CCCfg *CCCfgUplink);
u32 XDfeMix_AddCC(XDfeMix *InstancePtr, s32 CCID, u32 CCSeqBitmap,
		  const XDfeMix_CarrierCfg *CarrierCfg, const XDfeMix_NCO *NCO);
u32 XDfeMix_RemoveCC(XDfeMix *InstancePtr, s32 CCID);
u32 XDfeMix_MoveCC(XDfeMix *InstancePtr, s32 CCID, u32 Rate, u32 FromNCO,
		   u32 ToNCO);
u32 XDfeMix_UpdateCC(XDfeMix *InstancePtr, s32 CCID,
		     const XDfeMix_CarrierCfg *CarrierCfg);
u32 XDfeMix_SetAntennaGain(XDfeMix *InstancePtr, u32 AntennaId,
			   u32 AntennaGain);
u32 XDfeMix_UpdateAntennaCfg(XDfeMix *InstancePtr,
			     XDfeMix_AntennaCfg *AntennaCfg);
void XDfeMix_GetTriggersCfg(const XDfeMix *InstancePtr,
			    XDfeMix_TriggerCfg *TriggerCfg);
void XDfeMix_SetTriggersCfg(const XDfeMix *InstancePtr,
			    XDfeMix_TriggerCfg *TriggerCfg);
void XDfeMix_GetDUCDDCStatus(const XDfeMix *InstancePtr,
			     XDfeMix_DUCDDCStatus *DUCDDCStatus);
void XDfeMix_GetMixerStatus(const XDfeMix *InstancePtr,
			    XDfeMix_MixerStatus *MixerStatus);
void XDfeMix_GetInterruptMask(const XDfeMix *InstancePtr,
			      XDfeMix_InterruptMask *Mask);
void XDfeMix_SetInterruptMask(const XDfeMix *InstancePtr,
			      const XDfeMix_InterruptMask *Mask);
void XDfeMix_GetEventStatus(const XDfeMix *InstancePtr, XDfeMix_Status *Status);
void XDfeMix_ClearEventStatus(const XDfeMix *InstancePtr,
			      const XDfeMix_Status *Status);
void XDfeMix_SetTUserDelay(const XDfeMix *InstancePtr, u32 Delay);
u32 XDfeMix_GetTUserDelay(const XDfeMix *InstancePtr);
u32 XDfeMix_GetTDataDelay(const XDfeMix *InstancePtr, u32 Tap, u32 *TDataDelay);
u32 XDfeMix_GetCenterTap(const XDfeMix *InstancePtr, u32 Rate, u32 *CenterTap);
void XDfeMix_SetRegBank(const XDfeMix *InstancePtr, u32 RegBank);
void XDfeMix_GetVersions(const XDfeMix *InstancePtr, XDfeMix_Version *SwVersion,
			 XDfeMix_Version *HwVersion);

#ifdef __cplusplus
}
#endif

#endif
/** @} */
