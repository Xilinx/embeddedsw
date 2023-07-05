/******************************************************************************
* Copyright (C) 2021-2022 Xilinx, Inc. All rights reserved.
* Copyright (C) 2022-2023 Advanced Micro Devices, Inc. All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xdfeprach.h
* @addtogroup dfeprach Overview
* @{
*
* @cond nocomments
*
* The RFSoC DFE PRACH LogiCORE IP provides a wrapper around the PRACH
* primitive. The wrapper provides access to the underlying primitives via
* TDM AXI-stream data interfaces. Output from the primitive is arranged as an
* AXI4-Stream, per antenna. A memory mapped AXI interface is provided, which
* enables configuration and control of the core from a microprocessor. The AXI
* memory map also provides access to the IP core's status.
*
* The features that the PRACH IP and the driver support are:
*
* - Support for up to 4x491.52 MSPS aggregated input sample rate distributed
*   over up to 8 component carriers and 8 antenna paths (e.g. 200MHz oBW over
*   8 antennas or 400MHz oBW over 4 antennas).
* - Support for up to 16 PRACH channels per antenna with a maximum aggregated
*   down-sampling rate of up to 122.88MSPS. The 16 channels can be dynamically
*   allocated accross up to 8 component carriers.
* - UL Input TDM pattern programmed via register interface.
* - Output TDM pattern is driven by the RACH primitive. There is no option to
*   programme the order. The expectation is that the buffering in the PRACH FFT
*   chain will recombine the outputs into their respective blocks.
* - Supports 16 NCO channels.
* - Each NCO channel is available to any input TDM slot modulo 16.
* - NCO signal can be replicated across multiple antennas for a given CCID.
* - Each CCID can access more than one NCO/Filter channel.
* - Supports up to 16 separate decimation channels, replicated across
*   eight antennas for a total of 128 available decimation channels.
* - Each decimation channel can perform by 2,4,8,16,32,64,3,6,12,24,48, or 96.
* - Supports Down conversion rate set on a per RACH channel basis, programmed
*   via processor interface.
* - Supports power down between RACH slots and/or for TDD via a processor
*   interface and TUSER input.
* - Supports seamless removal, addition, and movement of RACH channel to allow
*   for resource management.
* - TUSER/TLAST information accompanying the data is delay matched through
*   the IP.
*
* An API which will reads/writes registers is provided to allow software to
* access the DFE PRACH core.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who    Date     Changes
* ----- ---    -------- -----------------------------------------------
* 1.0   dc     03/08/21 Initial version
*       dc     04/06/21 Register with full node name
*       dc     04/10/21 Set sequence length only once
*       dc     04/21/21 Update due to restructured registers
*       dc     05/08/21 Update to common trigger
*       dc     05/18/21 Handling RachUpdate trigger
* 1.1   dc     06/30/21 Doxygen documentation update
*       dc     07/13/21 Update to common latency requirements
* 1.2   dc     10/29/21 Update doxygen comments
*       dc     11/01/21 Add multi AddCC, RemoveCC and UpdateCC
*       dc     11/05/21 Align event handlers
*       dc     11/19/21 Update doxygen documentation
*       dc     11/26/21 Add SetAntennaCfgInCCCfg API
*       dc     12/17/21 Update after documentation review
* 1.3   dc     01/31/22 Add CORE_SETTINGS register
*       dc     03/21/22 Add prefix to global variables
* 1.4   dc     04/04/22 Correct PatternPeriod represantion
*       dc     03/28/22 Update documentation
* 1.5   dc     12/14/22 Update multiband register arithmetic
*       dc     01/02/23 Multiband registers update
* 1.6   dc     08/06/23 Support dynamic and static modes of operation
*       cog    07/04/23 Add support for SDT
*
* </pre>
* @endcond
******************************************************************************/
#ifndef XDFEPRACH_H_
#define XDFEPRACH_H_

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
#define XDFEPRACH_MAX_NUM_INSTANCES                                            \
	(1U) /**< Maximum number of driver instances running at the same time. */
#define XDFEPRACH_INSTANCE_EXISTS(X) (X < XDFEPRACH_MAX_NUM_INSTANCES)
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
#define XDFEPRACH_MAX_NUM_INSTANCES XPAR_XDFEPRACH_NUM_INSTANCES
#define XDFEPRACH_INSTANCE_EXISTS(X) (X < XDFEPRACH_MAX_NUM_INSTANCES)
#else
#define XDFEPRACH_MAX_NUM_INSTANCES                                            \
	(1U) /**< Maximum number of driver instances running at the same time. */
#define XDFEPRACH_INSTANCE_EXISTS(X) (XDfePrach_ConfigTable[X].Name != NULL)
#endif
#endif

#define XDFEPRACH_NODE_NAME_MAX_LENGTH (50U) /**< Node name maximum length. */

#define XDFEPRACH_CC_NUM_MAX (16) /**< Maximum CC number. */
#define XDFEPRACH_RC_NUM_MAX (16U) /**< Maximum number of RACH channels. */
#define XDFEPRACH_NCO_NUM_MAX (16U) /**< Maximum NCO number */
#define XDFEPRACH_ANT_NUM_MAX (8U) /**< Maximum anntena number. */
#define XDFEPRACH_SEQ_LENGTH_MAX (16U) /**< Maximum CCID sequence length. */
#define XDFEPRACH_BAND_ID_MAX (3U) /**< Maximum Band Id number. */

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
#define XDFEPRACH_CUSTOM_DEV(_dev_name, _baseaddr, _idx)                       \
	{                                                                      \
		.name = _dev_name, .bus = NULL, .num_regions = 1,              \
		.regions = { {                                                 \
			.virt = (void *)_baseaddr,                             \
			.physmap = &XDfePrach_metal_phys[_idx],                \
			.size = 0x10000,                                       \
			.page_shift = (u32)(-1),                               \
			.page_mask = (u32)(-1),                                \
			.mem_flags = 0x0,                                      \
			.ops = { NULL },                                       \
		} },                                                           \
		.node = { NULL }, .irq_num = 0, .irq_info = NULL,              \
	}
#endif

typedef enum XDfePrach_StateId {
	XDFEPRACH_STATE_NOT_READY = 0, /**< Not ready state*/
	XDFEPRACH_STATE_READY, /**< Ready state*/
	XDFEPRACH_STATE_RESET, /**< Reset state*/
	XDFEPRACH_STATE_CONFIGURED, /**< Configured state*/
	XDFEPRACH_STATE_INITIALISED, /**< Initialised state*/
	XDFEPRACH_STATE_OPERATIONAL /**< Operational state*/
} XDfePrach_StateId;

/**
 * Logicore version.
 */
typedef struct {
	u32 Major; /**< Major version number */
	u32 Minor; /**< Minor version number */
	u32 Revision; /**< Revision number */
	u32 Patch; /**< Patch number */
} XDfePrach_Version;

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
} XDfePrach_Trigger;

/**
 * All IP triggers.
 */
typedef struct {
	XDfePrach_Trigger Activate; /**< Toggle between "Initialized",
		ultra-low power state, and "Operational". One-shot trigger,
		disabled following a single event */
	XDfePrach_Trigger LowPower; /**< Toggle between "Low-power"
		and "Operational" state */
	XDfePrach_Trigger RachUpdate; /**< Transition to next Rach/CC
		configuration. Will initiate flush of Rach channel */
	XDfePrach_Trigger FrameInit[XDFEPRACH_BAND_ID_MAX]; /**< Indicate
		the boundary of a frame */
} XDfePrach_TriggerCfg;

/**
 * Defines a CCID sequence.
 */
typedef struct {
	u32 Length; /**< [1-16] Sequence length */
	s32 CCID[XDFEPRACH_SEQ_LENGTH_MAX]; /**< [0-15].Array of CCID's
		arranged in the order the CCIDs are required to be processed
		in the PRACH. May contain duplicate entries depending
		on the rate channel rates. */
} XDfePrach_CCSequence;

/*********** end - common code to all Logiccores ************/
/**
 * PRACH model parameters. Data defined in Device tree/xparameters.h
 */
typedef struct {
	u32 NumAntenna
		[XDFEPRACH_BAND_ID_MAX]; /**< [1-8] CORE.MODEL_PARAM.NUM_ANTENNA */
	u32 NumCCPerAntenna
		[XDFEPRACH_BAND_ID_MAX]; /**< [1-16] CORE.MODEL_PARAM.NUM_CC_PER_ANTENNA */
	u32 NumAntennaChannels
		[XDFEPRACH_BAND_ID_MAX]; /**< [1-4] CORE.MODEL_PARAM.NUM_SLOT_CHANNELS */
	u32 NumAntennaSlots
		[XDFEPRACH_BAND_ID_MAX]; /**< [1-8] CORE.MODEL_PARAM.NUM_SLOTS */
	u32 NumRachLanes; /**< [1-2] CORE.MODEL_PARAM.NUM_RACH_LANES */
	u32 NumRachChannels; /**< [1-16] CORE.MODEL_PARAM.NUM_RACH_CHANNELS */
	u32 HasAxisCtrl; /**< [0,1] CORE.MODEL_PARAM.HAS_AXIS_CTRL */
	u32 HasIrq; /**< [0,1] CORE.MODEL_PARAM.HAS_IRQ */
	u32 NumBands; /**< [1-3] CORE.MODEL_PARAM.NUM_BANDS */
} XDfePrach_ModelParameters;

/**
 * Configuration.
 */
typedef struct {
	XDfePrach_Version Version; /**< Logicore version */
	XDfePrach_ModelParameters ModelParams; /**< Logicore
		parameterization */
} XDfePrach_Cfg;

/**
 * Initialization, "one-time" configuration parameters.
 */
typedef struct {
	XDfePrach_CCSequence
		Sequence[XDFEPRACH_BAND_ID_MAX]; /**< CCID sequence. */
	bool EnableStaticSchedule; /**< Static schedule enable flag. */
	bool EnableUseFreqOffset; /**< Enable the use of frequency offset. */
} XDfePrach_Init;

/**
 * Internal Configuration for a single CC.
 */
typedef struct {
	u32 Enable; /**< [0,1] Enable/Disable CC while still reserving its
		slot in the TDM - set by helper functions when building
		the configuration. */
	u32 SCS; /**< [0-4] Array of SCS values, one  for each CCID number
		(different index from the location that the sequence is
		mapped to). Sub carrier Spacing for each CC - required to
		determine Slot boundaries:
				- 0: 15KHz spacing
				- 1: 30KHz spacing
				- 2: 60KHz spacing
				- 3: 120KHz spacing
				- 4: 240KHz spacing */
	u32 CCRate; /**< [0-3] Array of Sample rate values values, one for
		each CCID number. The sample rate for the CC:
		- 0: 30.72Ms/s, implies 1x decimation to get to 30.72Ms/s)
		- 1: 61.44MS/s, implies 2x decimation to get to 30.72Ms/s)
		- 2: 122.88MS/s, implies 4x decimation to get to 30.72Ms/s)
		- 3: 245.76MS/s, implies 8x decimation to get to 30.72Ms/s)
		This is also the Decimation rate required to decimate the
		CC down to 30.72Ms/s */
} XDfePrach_InternalCarrierCfg;

/**
 * Configuration for a single CC.
 */
typedef struct {
	u32 SCS; /**< [0-4] Array of SCS values, one  for each CCID number
		(different index from the location that the sequence is
		mapped to). Sub carrier Spacing for each CC - required to
		determine Slot boundaries:
				- 0: 15KHz spacing
				- 1: 30KHz spacing
				- 2: 60KHz spacing
				- 3: 120KHz spacing
				- 4: 240KHz spacing */
} XDfePrach_CarrierCfg;

/**
 * Full CC configuration.
 */
typedef struct {
	XDfePrach_CCSequence Sequence
		[XDFEPRACH_BAND_ID_MAX]; /**< CCID sequence, this needs to match
		the CCID sequence generated by the Mixer connected to
		the PRACH. Maximum CC number is 16. */
	XDfePrach_InternalCarrierCfg CarrierCfg[XDFEPRACH_BAND_ID_MAX]
					       [XDFEPRACH_CC_NUM_MAX]; /**< Array
		of [16] CC configurations */
	u32 AntennaCfg[XDFEPRACH_ANT_NUM_MAX]; /**< [0,1] Array of [8] antenna
		TDM slot enablement */
} XDfePrach_CCCfg;

/**
 * NCO Config.
 */
typedef struct {
	u32 PhaseOffset; /**< [0-2^32-1] Phase offset value which can be
		 applied to the NCO's phase accumulator */
	u32 PhaseAcc; /**< [0-2^32-1] Initial Phase accumulator value, used to
		set the NCO phase accumualtor to a specific phase at startup */
	u32 DualModCount; /**< [0-2^32-1] The phase accumulator allows for dual
		modulus accumulation to create fractional frequencies. This
		field allows for initialisation of the dual mod count to
		a known value */
	u32 DualModSel; /**< [0,1] Allows initialisation of the Dual mod select
		to a known value */
	s32 UserFreq; /**< [-(2^23)-2^23] User defined frequency container */
	s32 Frequency; /**< [0-2^32-1] Frequency control word (FCW) */
	u32 FreqSingleModCount; /**< [0-2^32-1] Single modulus cycle count (S) */
	u32 FreqDualModCount; /**< [0-2^32-1] Dual modulus cycle count (T-S) */
	u32 FreqPhaseOffset; /**< [0-2^18-1] Phase offset */
	u32 NcoGain; /**< [0-3] Scaling of NCO output (0=0dB, 1=-3dB,
		2=-6dB, 3=-9dB) */
} XDfePrach_NCO;

/**
 * Decimator Config.
 */
typedef struct {
	u32 DecimationRate; /**< [0,1,2,3,4,8,9,10,11] Decimation Rate required
			to go from 30.72MS/s to the RACH sample rate:
				- 0: 1x decimation(not allowed when
					XDfePrach_CarrierCfg.CCRate==0)
				- 1: 2x decimation
				- 2: 4x decimation
				- 3: 8x decimation(not allowed when
					XDfePrach_CarrierCfg.CCRate==3)
				- 4: 16x decimation(not allowed when
					XDfePrach_CarrierCfg.CCRate==3 or
					XDfePrach_CarrierCfg.CCRate==2)
				- 8: 3x decimation
				- 9: 6x decimation
				- 10: 12x decimation(not allowed when
					XDfePrach_CarrierCfg.CCRate==3)
				- 11: 24x decimation(not allowed when
					XDfePrach_CarrierCfg.CCRate==3 or
					XDfePrach_CarrierCfg.CCRate==2) */
	u32 UserSCS; /**< [0-4,12-15] SubCarrier spacing of the RACH
			transmission this DDC is decimating and set by user.
			Required to determine phase increment:
				- 0: 15KHz spacing
				- 1: 30KHz spacing
				- 2: 60KHz spacing
				- 3: 120KHz spacing
				- 4: 240KHz spacing
				- 12: 1.25KHz spacing
				- 13: 3.75KHz spacing
				- 14: 5KHz spacing
				- 15: 7.5KHz spacing */
	u32 RachGain[6]; /**< [0-3] The array of [6] Decimation Gains. Gain is
		applied to all active Decimation filters. Decimation filters
		are enabled depending upon the total decimation rate.
		The total_decimation_rate is the product of the decoded values
		of DecRate and XDfePrach_CarrierCfg.CCRate. Gain is applied on
		a per filter basis:

		- Always applies:
			- CCDecGain[0]=0: Gain of 0dB in last decimating filter
			- CCDecGain[0]=1: Gain of 6dB in last decimating filter
			- CCDecGain[0]=2: Gain of 12dB in last decimating filter
			- CCDecGain[0]=3: Gain of 18dB in last decimating filter
		- Only applies if total_decimation_rate = {4, 8, 16, 32, 6, 12,
		 24, 48, 96)
			- CCDecGain[1]=0: Gain of 0dB in fifth decimating filter
			- CCDecGain[1]=1: Gain of 6dB in fifth decimating filter
		- Only applies if total_decimation_rate = {8, 16, 32, 12, 24, 48,
		 96)
			- CCDecGain[2]=0: Gain of 0dB in fourth decimating filter
			- CCDecGain[2]=1: Gain of 6dB in fourth decimating filter
		- Only applies if total_decimation_rate = {16, 32, 24, 48, 96)
			- CCDecGain[3]=0: Gain of 0dB in third decimating filter
			- CCDecGain[3]=1: Gain of 6dB in third decimating filter
		- Only applies if total_decimation_rate = {32, 48, 96)
			- CCDecGain[4]=0: Gain of 0dB in second decimating filter
			- CCDecGain[4]=1: Gain of 6dB in second decimating filter
		- Only applies if total_decimation_rate = {96)
			- CCDecGain[5]=0: Gain of 0dB in first decimating filter
			- CCDecGain[5]=1: Gain of 6dB in first decimating filter
		*/
} XDfePrach_DDCCfg;

/**
 * Static Schedule for a RACH Channel.
 */
typedef struct {
	u32 PatternPeriod; /**< [1-256] Duration, in Frames, of the repeating
		pattern of enables. Internal frame count runs from 0 to
		PatternPeriod-1 */
	u32 FrameID; /**< [0-255] First frame within the pattern period which
		is enabled for a RACH capture. Cannot exceed  PatternPeriod */
	u32 SubframeID; /**< [0-9] Subframe number which denotes the subframe
		at which a RACH capture should begin. Only valid when
		FrameID==frame count */
	u32 SlotId; /**< [0-7] Slot number whcih denotes the slot at which
		a RACH Capture should begin. Only valid when FrameID==frame
		count and only valid when SubframeID==sub frame count:
			- CC SCS restricts the range of slots available:
			- CC_SCS == 15KHz => slotID=={0}
			- CC_SCS == 30KHz => slotID=={0,1}
			- CC_SCS == 60KHz => slotID=={0,1,2,3}
			- CC_SCS == 120KHz => slotID=={0,1,2,3,4,5,6,7}
			- CC_SCS == 240KHz => slotID=={0,1,2,3,4,5,6,7,8,9,10,
				11,12,13,14,15} (unused) */
	u32 Duration; /**< [1-2^12] Specifies the duration of a single RACH
		capture, in slots */
	u32 Repeats; /**< [1-256] Specifies the number of consecutive captures
		to execute. New capture will begin on the sl`ot immediately
		after "Duration" has ended */
} XDfePrach_Schedule;

/**
 * Full RC configuration.
 */
typedef struct {
	u32 Enable; /**< [0,1] Indicates if this RCID is enabled. */
	u32 RCId; /**< [0-15] RCCID number allocated to this RACH channel
		configuration. This is the TID identifier on the RACH/FFT
		interface. */
	u32 RachChannel; /**< [0-15] The physical RACH channel used by this
		RCID. */
	s32 CCID; /**< [0-15] The CCID channel (within the band defined in
		field BAND) from which this RACH channel is to be extracted. */
	u32 BandId; /**< [0-2] The Band from which this RACH channel is to be
		extracted. */
} XDfePrach_InternalChannelCfg;

typedef struct {
	u32 RCId; /**< [0-15] RCCID number allocated to this RACH channel
		configuration. This is the TID identifier on the RACH/FFT
		interface. */
	u32 RachChannel; /**< [0-15] The physical RACH channel used by this
		RCID. */
	s32 CCID; /**< [0-15] The CCID channel (within the band defined in
		field BAND) from which this RACH channel is to be extracted. */
	u32 BandId; /**< [0-2] The Band from which this RACH channel is to be
		extracted. */
} XDfePrach_ChannelCfg;

typedef struct {
	XDfePrach_InternalChannelCfg InternalRCCfg
		[XDFEPRACH_RC_NUM_MAX]; /**< Internal RACH configuration */
	XDfePrach_NCO NcoCfg[XDFEPRACH_NCO_NUM_MAX]; /**< NCO configuration */
	XDfePrach_DDCCfg DdcCfg[XDFEPRACH_RC_NUM_MAX]; /**< DDC configuration */
	XDfePrach_Schedule StaticSchedule
		[XDFEPRACH_RC_NUM_MAX]; /**< Static Schedule configuration */
} XDfePrach_RCCfg;

/**
 * PRACH status.
 */
typedef struct {
	u32 MixerOverflow; /**< [0-1] Source of first occurrence of overflow
		as signalled in ISR. That is, register only updated when
		MIXER_OVERFLOW flag is 0. */
	u32 FirstAntennaOverflowing; /**< [0-7] Lowest numbered antenna on
		which first mixer overflow occurred. */
	u32 FirstRCIdOverflowing; /**< [0-15] The physical mixer on which
		the first mixer overflow occurred. Each mixer handles 4 rach
		channels, and the overflow indicator is agregated, so an
		overflow on physical channel 0-3 will indicate as an overflow
		on Channel 0, 4-7: channel 1, etc. */
} XDfePrach_MixerStatusOverflow;

/**
 * PRACH decimator overflow status.
 */
typedef struct {
	u32 DecimatorOverflow; /**< [0-1] Source of first occurrence of
		overflow as signalled in ISR. That is, register only
		updated when DECIMATOR_OVERFLOW flag is 0. */
	u32 FirstAntennaOverflowing; /**< [0-7] Lowest number antenna on which
		first overflow occurred. */
	u32 FirstRCIdOverflowing; /**< [0-15] Lowest numbered Physical RACH
		channel on which first overrun occurred. */
} XDfePrach_DecimatorStatusOverflow;

/**
 * PRACH mixer overflow status.
 */
typedef struct {
	u32 MixerOverrun; /**< [0-1] Source of first occurrence of selector
		overrun as signalled in ISR. That is, register only updated
		when SELECTOR_OVERRUN flag is 0. */
	u32 FirstAntennaOverruning; /**< [0-7] Earliest antenna in which
		selector overrun occurred. */
	u32 FirstRCIdOverruning; /**< [0-15] Lowest numbered Physical RACH
		channel on which first selector overrun occurred. */
} XDfePrach_MixerStatusOverrun;

/**
 * PRACH decimator overrun status.
 */
typedef struct {
	u32 DecimatorOverrun; /**< [0-1] Source of first occurrence of overflow
		as signalled in ISR. That is, register only updated when
		DECIMATOR_OVERRUN flag is 0. */
	u32 FirstAntennaOverruning; /**< [0-7] Lowest number antenna on which
		first overrun occurred. */
	u32 FirstRCIdOverruning; /**< [0-15] Lowest numbered Physical RACH
		channel on which first overrun occurred. */
} XDfePrach_DecimatorStatusOverrun;

/**
 * PRACH mixer overrun status.
 */
typedef struct {
	XDfePrach_MixerStatusOverflow MixerOverflow; /**< Mixer Overflow Status */
	XDfePrach_DecimatorStatusOverflow
		DecimatorOverflow; /**< Decimator Overflow Status */
	XDfePrach_MixerStatusOverrun MixerOverrun; /**< Mixer Overrun Status */
	XDfePrach_DecimatorStatusOverrun
		DecimatorOverrun; /**< Decimator Overrun Status */
} XDfePrach_Status;

/**
 * Event status and interrupt mask.
 *
 * @note The structure for XDfePrach_InterruptMask is the same as that of
 *       XDfePrach_StatusMask
 */
typedef struct {
	u32 DecimatorOverflow; /**< [0,1] Mask overflow in Decimator */
	u32 MixerOverflow; /**< [0,1] Mask overflow in mixer */
	u32 DecimatorOverrun; /**< [0,1] Mask overrun in the decimator */
	u32 SelectorOverrun; /**< [0,1] Mask overrun in the selector */
	u32 RachUpdate; /**< [0,1] Mask RACH configuration update */
	u32 CCSequenceError
		[XDFEPRACH_BAND_ID_MAX]; /**< [0,1] Mask CC sequence error */
	u32 FrameInitTrigger
		[XDFEPRACH_BAND_ID_MAX]; /**< [0,1] Mask Frame initialisation Trigger */
	u32 FrameError[XDFEPRACH_BAND_ID_MAX]; /**< [0,1] Mask Frame error */
} XDfePrach_StatusMask;

typedef XDfePrach_StatusMask XDfePrach_InterruptMask;
/**
 * PRACH Config Structure.
 */
typedef struct {
#ifndef SDT
	u32 DeviceId; /**< The component instance Id */
#else
       char *Name; /**< Unique name of the device */
#endif
	metal_phys_addr_t BaseAddr; /**< Instance base address */
	u32 NumAntenna
		[XDFEPRACH_BAND_ID_MAX]; /**< [1-8] CORE.MODEL_PARAM.NUM_ANTENNA */
	u32 NumCCPerAntenna
		[XDFEPRACH_BAND_ID_MAX]; /**< [1-16] CORE.MODEL_PARAM.NUM_CC_PER_ANTENNA */
	u32 NumAntennaChannels
		[XDFEPRACH_BAND_ID_MAX]; /**< [1-4] CORE.MODEL_PARAM.NUM_SLOT_CHANNELS */
	u32 NumAntennaSlots
		[XDFEPRACH_BAND_ID_MAX]; /**< [1-8] CORE.MODEL_PARAM.NUM_SLOTS */
	u32 NumRachLanes; /**< [1-2] CORE.MODEL_PARAM.NUM_RACH_LANES */
	u32 NumRachChannels; /**< [1-16] CORE.MODEL_PARAM.NUM_RACH_CHANNELS */
	u32 HasAxisCtrl; /**< [0,1] CORE.MODEL_PARAM.HAS_AXIS_CTRL */
	u32 HasIrq; /**< [0,1] CORE.MODEL_PARAM.HAS_IRQ */
	u32 NumBands; /**< [1-3] CORE.MODEL_PARAM.NUM_BANDS */
} XDfePrach_Config;

/**
 * PRACH driver object - global data storage.
 */
typedef struct {
	XDfePrach_Config Config; /**< Configuration data storage */
	XDfePrach_StateId StateId; /**< State machine state Id */
	s32 NotUsedCCID
		[XDFEPRACH_BAND_ID_MAX]; /**< Storage for 'Not used CCID' value */
	u32 SequenceLength[XDFEPRACH_BAND_ID_MAX]; /**< Sequence length 'storage' */
	char NodeName[XDFEPRACH_NODE_NAME_MAX_LENGTH]; /**< Node name storage */
	struct metal_io_region *Io; /**< Libmetal IO structure */
	struct metal_device *Device; /**< Libmetal device structure */
} XDfePrach;

/************************** Variable Definitions *****************************/
#ifdef __BAREMETAL__
extern XDfePrach_Config XDfePrach_ConfigTable[XDFEPRACH_MAX_NUM_INSTANCES];
#endif

/**************************** API declarations *******************************/
/* System initialization API */
XDfePrach *XDfePrach_InstanceInit(const char *DeviceNodeName);
void XDfePrach_InstanceClose(XDfePrach *InstancePtr);

/* Register access API */
/**
* @cond nocomments
*/
void XDfePrach_WriteReg(const XDfePrach *InstancePtr, u32 AddrOffset, u32 Data);
u32 XDfePrach_ReadReg(const XDfePrach *InstancePtr, u32 AddrOffset);
/**
* @endcond
*/

/* DFE PRACH component initialization API */
void XDfePrach_Reset(XDfePrach *InstancePtr);
void XDfePrach_Configure(XDfePrach *InstancePtr, XDfePrach_Cfg *Cfg);
void XDfePrach_Initialize(XDfePrach *InstancePtr, XDfePrach_Init *Init);
void XDfePrach_Activate(XDfePrach *InstancePtr, bool EnableLowPower);
void XDfePrach_Deactivate(XDfePrach *InstancePtr);
XDfePrach_StateId XDfePrach_GetStateID(XDfePrach *InstancePtr);

/* User APIs */
void XDfePrach_GetCurrentCCCfg(const XDfePrach *InstancePtr,
			       XDfePrach_CCCfg *CCCfg);
void XDfePrach_GetEmptyCCCfg(const XDfePrach *InstancePtr,
			     XDfePrach_CCCfg *CCCfg);
void XDfePrach_GetCarrierCfgMB(const XDfePrach *InstancePtr,
			       XDfePrach_CCCfg *CCCfg, s32 CCID,
			       u32 *CCSeqBitmap,
			       XDfePrach_CarrierCfg *CarrierCfg,
			       const u32 BandId);
void XDfePrach_GetCarrierCfg(const XDfePrach *InstancePtr,
			     XDfePrach_CCCfg *CCCfg, s32 CCID, u32 *CCSeqBitmap,
			     XDfePrach_CarrierCfg *CarrierCfg);
void XDfePrach_SetAntennaCfgInCCCfg(const XDfePrach *InstancePtr,
				    XDfePrach_CCCfg *CCCfg, u32 *AntennaCfg);
u32 XDfePrach_AddCCtoCCCfgMB(XDfePrach *InstancePtr, XDfePrach_CCCfg *CCCfg,
			     s32 CCID, u32 CCSeqBitmap,
			     const XDfePrach_CarrierCfg *CarrierCfg,
			     const u32 BandId);
u32 XDfePrach_AddCCtoCCCfg(XDfePrach *InstancePtr, XDfePrach_CCCfg *CCCfg,
			   s32 CCID, u32 CCSeqBitmap,
			   const XDfePrach_CarrierCfg *CarrierCfg);
u32 XDfePrach_RemoveCCfromCCCfgMB(XDfePrach *InstancePtr,
				  XDfePrach_CCCfg *CCCfg, s32 CCID,
				  const u32 BandId);
u32 XDfePrach_RemoveCCfromCCCfg(XDfePrach *InstancePtr, XDfePrach_CCCfg *CCCfg,
				s32 CCID);
u32 XDfePrach_UpdateCCinCCCfgMB(const XDfePrach *InstancePtr,
				XDfePrach_CCCfg *CCCfg, s32 CCID,
				const XDfePrach_CarrierCfg *CarrierCfg,
				const u32 BandId);
u32 XDfePrach_UpdateCCinCCCfg(const XDfePrach *InstancePtr,
			      XDfePrach_CCCfg *CCCfg, s32 CCID,
			      const XDfePrach_CarrierCfg *CarrierCfg);
u32 XDfePrach_SetNextCfg(const XDfePrach *InstancePtr,
			 const XDfePrach_CCCfg *NextCCCfg,
			 XDfePrach_RCCfg *NextRCCfg);
u32 XDfePrach_AddCC(XDfePrach *InstancePtr, s32 CCID, u32 CCSeqBitmap,
		    const XDfePrach_CarrierCfg *CarrierCfg);
u32 XDfePrach_RemoveCC(XDfePrach *InstancePtr, s32 CCID);
u32 XDfePrach_UpdateCC(const XDfePrach *InstancePtr, s32 CCID,
		       const XDfePrach_CarrierCfg *CarrierCfg);
void XDfePrach_GetCurrentRCCfg(const XDfePrach *InstancePtr,
			       XDfePrach_RCCfg *RCCfg);
void XDfePrach_GetEmptyRCCfg(const XDfePrach *InstancePtr,
			     XDfePrach_RCCfg *RCCfg);
void XDfePrach_GetChannelCfg(const XDfePrach *InstancePtr,
			     XDfePrach_RCCfg *RCCfg, s32 RCId,
			     XDfePrach_ChannelCfg *ChannelCfg);
u32 XDfePrach_AddRCtoRCCfgMB(const XDfePrach *InstancePtr,
			     XDfePrach_RCCfg *CurrentRCCfg, s32 CCID, u32 RCId,
			     u32 RachChan, XDfePrach_DDCCfg *DdcCfg,
			     XDfePrach_NCO *NcoCfg,
			     XDfePrach_Schedule *StaticSchedule,
			     XDfePrach_CCCfg *NextCCCfg, u32 BandId);
u32 XDfePrach_AddRCtoRCCfgMBDynamic(const XDfePrach *InstancePtr,
				    XDfePrach_RCCfg *CurrentRCCfg, s32 CCID,
				    u32 RCId, u32 RachChan,
				    XDfePrach_CCCfg *NextCCCfg, u32 BandId);
u32 XDfePrach_AddRCtoRCCfg(const XDfePrach *InstancePtr,
			   XDfePrach_RCCfg *CurrentRCCfg, s32 CCID, u32 RCId,
			   u32 RachChan, XDfePrach_DDCCfg *DdcCfg,
			   XDfePrach_NCO *NcoCfg,
			   XDfePrach_Schedule *StaticSchedule,
			   XDfePrach_CCCfg *NextCCCfg);
u32 XDfePrach_AddRCtoRCCfgDynamic(const XDfePrach *InstancePtr,
				  XDfePrach_RCCfg *CurrentRCCfg, s32 CCID,
				  u32 RCId, u32 RachChan,
				  XDfePrach_CCCfg *NextCCCfg);
u32 XDfePrach_RemoveRCfromRCCfg(const XDfePrach *InstancePtr,
				XDfePrach_RCCfg *CurrentRCCfg, u32 RCId);
void XDfePrach_UpdateRCinRCCfgMB(const XDfePrach *InstancePtr,
				 XDfePrach_RCCfg *CurrentRCCfg, s32 CCID,
				 u32 RCId, u32 RachChan,
				 XDfePrach_DDCCfg *DdcCfg,
				 XDfePrach_NCO *NcoCfg,
				 XDfePrach_Schedule *StaticSchedule,
				 XDfePrach_CCCfg *NextCCCfg, u32 BandId);
void XDfePrach_UpdateRCinRCCfg(const XDfePrach *InstancePtr,
			       XDfePrach_RCCfg *CurrentRCCfg, s32 CCID,
			       u32 RCId, u32 RachChan, XDfePrach_DDCCfg *DdcCfg,
			       XDfePrach_NCO *NcoCfg,
			       XDfePrach_Schedule *StaticSchedule,
			       XDfePrach_CCCfg *NextCCCfg);
u32 XDfePrach_AddRCCfg(const XDfePrach *InstancePtr, s32 CCID, u32 RCId,
		       u32 RachChan, XDfePrach_DDCCfg *DdcCfg,
		       XDfePrach_NCO *NcoCfg,
		       XDfePrach_Schedule *StaticSchedule);
u32 XDfePrach_RemoveRC(const XDfePrach *InstancePtr, u32 RCId);
u32 XDfePrach_UpdateRCCfg(const XDfePrach *InstancePtr, s32 CCID, u32 RCId,
			  u32 RachChan, XDfePrach_DDCCfg *DdcCfg,
			  XDfePrach_NCO *NcoCfg,
			  XDfePrach_Schedule *StaticSchedule);
u32 XDfePrach_MoveRC(const XDfePrach *InstancePtr, u32 RCId, u32 ToChannel);
void XDfePrach_GetTriggersCfg(const XDfePrach *InstancePtr,
			      XDfePrach_TriggerCfg *TriggerCfg);
void XDfePrach_SetTriggersCfg(const XDfePrach *InstancePtr,
			      XDfePrach_TriggerCfg *TriggerCfg);
void XDfePrach_GetCC(const XDfePrach *InstancePtr, bool Next, s32 CCID,
		     XDfePrach_CarrierCfg *CarrierCfg);
void XDfePrach_GetCCMB(const XDfePrach *InstancePtr, bool Next, s32 CCID,
		       XDfePrach_CarrierCfg *CarrierCfg, u32 BandId);
void XDfePrach_GetStatus(const XDfePrach *InstancePtr,
			 XDfePrach_Status *Status);
void XDfePrach_ClearStatus(const XDfePrach *InstancePtr);
void XDfePrach_CapturePhase(const XDfePrach *InstancePtr);
void XDfePrach_GetCapturePhase(const XDfePrach *InstancePtr, u32 RachChan,
			       XDfePrach_NCO *CapturedPhase);
void XDfePrach_GetInterruptMask(const XDfePrach *InstancePtr,
				XDfePrach_InterruptMask *Mask);
void XDfePrach_SetInterruptMask(const XDfePrach *InstancePtr,
				const XDfePrach_InterruptMask *Mask);
void XDfePrach_GetEventStatus(const XDfePrach *InstancePtr,
			      XDfePrach_StatusMask *Status);
void XDfePrach_ClearEventStatus(const XDfePrach *InstancePtr,
				const XDfePrach_StatusMask *Status);
void XDfePrach_SetTUserDelay(const XDfePrach *InstancePtr, u32 Delay);
void XDfePrach_SetTUserDelayMB(const XDfePrach *InstancePtr, u32 Delay,
			       u32 BandId);
u32 XDfePrach_GetTUserDelay(const XDfePrach *InstancePtr);
u32 XDfePrach_GetTUserDelayMB(const XDfePrach *InstancePtr, u32 BandId);
u32 XDfePrach_GetTDataDelay(const XDfePrach *InstancePtr);
u32 XDfePrach_GetTDataDelayMB(const XDfePrach *InstancePtr, u32 BandId);
void XDfePrach_GetVersions(const XDfePrach *InstancePtr,
			   XDfePrach_Version *SwVersion,
			   XDfePrach_Version *HwVersion);

#ifdef __cplusplus
}
#endif

#endif
/** @} */
