/******************************************************************************
* Copyright (C) 2022 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2022-2023 Advanced Micro Devices, Inc. All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xdfeofdm.h
* @addtogroup dfeofdm Overview
* @{
*
* @cond nocomments
* The RFSoC DFE Orthogonal Frequency Division Multiplexing IP performs
* translation between frequency domain to the time domain and vice versa using
* the hardened FFT IP. Each instance of the IP supports up to eight
* antennas in 1, 2, 4 and 8 combinations and up to 4 ANTENNA INTERLEAVE in
* combinations of 1, 2 and 4. Each instance can also support up to 8 component
* carriers (CC) to be aggregated on a particular antenna. The block performs
* buffering, FFT/IFFT conversion, scaling, and cyclic prefix insertion/removal.
* The block outputs in the case of DL (or accepts in the case of UL) IFFTed
* data as per the time domain sampling requirements using TDMA axi-stream data
* interfaces. An AXI memory mapped interface is provided for configuration and
* control.
*
* The features that the OFDM IP and the driver support are:
*
* - Supports a maximum sampling rate of 491.52 MS/s
* - Supports up to 8 CC both LTE and NR.
* - Supports SCS spacing of 15 KHz and 30 KHz.
* - Currently supports 1K, 2K and 4K FFT sizes.
* - Supports up to 8 DL or UL paths (or antennas)
* - Supports both TDD and FDD modes
* - Using 16 or 18 bit data interface.
* - Enable the user to program CCs via a process interface
* - Supports TDD power down via a processor interface and TUSER input
* - Indication of overflow of FD buffer provided via a status register
* - TUSER/TLAST information accompanying the data is delay match through the IP
* - Does not support SCS above 30 KHz for the time being
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who    Date     Changes
* ----- ---    -------- -----------------------------------------------
* 1.0   dc     11/21/22 Initial version
* 1.1   dc     04/05/23 Update documentation
*       dc     05/22/23 State and status upgrades
*       dc     06/28/23 Add phase compensation calculation
*
* </pre>
* @endcond
******************************************************************************/
#ifndef XDFEOFDM_H_
#define XDFEOFDM_H_

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
#define XDFEOFDM_MAX_NUM_INSTANCES                                             \
	(10U) /**< Maximum number of driver instances running at the same time. */
#define XDFEOFDM_INSTANCE_EXISTS(X) (X < XDFEOFDM_MAX_NUM_INSTANCES)
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
#define XDFEOFDM_MAX_NUM_INSTANCES XPAR_XDFEOFDM_NUM_INSTANCES
#define XDFEOFDM_INSTANCE_EXISTS(X) (X < XDFEOFDM_MAX_NUM_INSTANCES)
#else
#define XDFEOFDM_MAX_NUM_INSTANCES                                              \
	(10U) /**< Maximum number of driver instances running at the same time. */
#define XDFEOFDM_INSTANCE_EXISTS(X) (XDfeOfdm_ConfigTable[X].Name != NULL)
#endif
#endif

#define XDFEOFDM_NODE_NAME_MAX_LENGTH (50U) /**< Node name maximum length */

#define XDFEOFDM_CC_NUM (16) /**< Maximum CC sequence number */
#define XDFEOFDM_FT_NUM (16) /**< Maximum FT sequence number */
#define XDFEOFDM_CC_SEQ_LENGTH_MAX (16U) /**< Maximum sequence length */
#define XDFEOFDM_FT_SEQ_LENGTH_MAX                                             \
	(16U) /**< Maximum Fourier transform sequence length */
#define XDFEOFDM_PHASE_COMPENSATION_MAX                                        \
	(112U) /**< Maximum phase compensation weight */

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
#define XDFEOFDM_CUSTOM_DEV(_dev_name, _baseaddr, _idx)                        \
	{                                                                      \
		.name = _dev_name, .bus = NULL, .num_regions = 1,              \
		.regions = { {                                                 \
			.virt = (void *)_baseaddr,                             \
			.physmap = &XDfeOfdm_metal_phys[_idx],                 \
			.size = 0x10000,                                       \
			.page_shift = (u32)(-1),                               \
			.page_mask = (u32)(-1),                                \
			.mem_flags = 0x0,                                      \
			.ops = { NULL },                                       \
		} },                                                           \
		.node = { NULL }, .irq_num = 0, .irq_info = NULL,              \
	}
#endif

typedef enum XDfeOfdm_StateId {
	XDFEOFDM_STATE_NOT_READY = 0, /**< Not ready state*/
	XDFEOFDM_STATE_READY, /**< Ready state*/
	XDFEOFDM_STATE_RESET, /**< Reset state*/
	XDFEOFDM_STATE_CONFIGURED, /**< Configured state*/
	XDFEOFDM_STATE_INITIALISED, /**< Initialised state*/
	XDFEOFDM_STATE_OPERATIONAL /**< Operational state*/
} XDfeOfdm_StateId;

/**
 * Logicore version.
 */
typedef struct {
	u32 Major; /**< Major version number. */
	u32 Minor; /**< Minor version number. */
	u32 Revision; /**< Revision number. */
	u32 Patch; /**< Patch number. */
} XDfeOfdm_Version;

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
} XDfeOfdm_Trigger;

/**
 * All IP triggers.
 */
typedef struct {
	XDfeOfdm_Trigger Activate; /**< Switch between "Initialized",
		ultra-low power state, and "Operational". One-shot trigger,
		disabled following a single event. */
	XDfeOfdm_Trigger LowPower; /**< Switch between "Low-power"
		and "Operational" state. */
	XDfeOfdm_Trigger CCUpdate; /**< Transition to next CC
		configuration. Will initiate flush based on CC configuration. */
} XDfeOfdm_TriggerCfg;

/**
 * Defines a CCID sequence.
 */
typedef struct {
	u32 Length; /**< [1-16] Sequence length. */
	s32 CCID[XDFEOFDM_CC_SEQ_LENGTH_MAX]; /**< Array of CCID's arranged in
		the order the CC samples are output on CC Interfaces */
} XDfeOfdm_CCSequence;

/**
 * Defines a FT sequence.
 */
typedef struct {
	u32 Length; /**< [1-16] Sequence length. */
	s32 CCID[XDFEOFDM_FT_SEQ_LENGTH_MAX]; /**< Array of CCID's arranged in
		the order of Fourier Transforms output on FT Interfaces */
} XDfeOfdm_FTSequence;

/*********** end - common code to all Logiccores ************/
/**
 * Orthogonal Frequency Division Multiplexing model parameters structure. Data
 * defined in Device tree or xparameters.h for BM.
 */
typedef struct {
	u32 NumAntenna; /**< [1-8] Number of antenas */
	u32 AntennaInterleave; /**< [1-8] Antenna interleave */
	u32 PhaseCompensation; /**< [0,1] Phase compesation
				0 - Phase compesation disabled
				1 - Phase compesation enabled */
} XDfeOfdm_ModelParameters;

/**
 * Configuration.
 */
typedef struct {
	XDfeOfdm_Version Version; /**< Logicore version */
	XDfeOfdm_ModelParameters ModelParams; /**< Logicore
		parameterization */
} XDfeOfdm_Cfg;

/**
 * Initialization, "one-time" configuration parameters.
 */
typedef struct {
	u32 CCSequenceLength; /**< CC Sequence Length */
} XDfeOfdm_Init;

/**
 * Configuration for a single CC.
 */
typedef struct {
	/* FT CONFIG */
	u32 Numerology; /**< [0-6] Numerology (Mu) value for this CC.
		- 0 = 15 kHz
		- 1 = 30 kHz
		- 2 = 60 KHz
		- 3 = 120 KHz
		- 4 = 240 KHz
		- 5 = 480 KHz
		- 6 = 960 KHz
		Numerology must be 0 for LTE. */
	u32 FftSize; /**< [10,11,12] FFT size to be used for FFT of the CC.
		Valid sizes are:
		- 1024 = 0xA
		- 2048 = 0xB
		- 4096 = 0xC */
	u32 NumSubcarriers; /**< [0-14] Number of non-null subcarriers
		in this CC. */
	u32 ScaleFactor; /**< [0-1023] Scaling factor for FFT output for this
		CC, represented as a fixed-point value in 0.10 fixed-point
		format. 0=>(multiple of 0), 1=>(multiple of 0.999) */
	u32 CommsStandard; /**< [0-1]
		- 1 = LTE
		- 0 = 5G NR */
	/* CC slot delay */
	u32 OutputDelay; /** [0-2047] Delay required before outputting CC
		in order to balance CC Filter group delay. */
	u32 PhaseCompensation[XDFEOFDM_PHASE_COMPENSATION_MAX]; /** Phase weight is
		a complex number with 0 to 15 bits providing the I and 16 to 31
		bits the Q part of the weight. */
} XDfeOfdm_CarrierCfg;

/**
 * Internal configuration for a single CC.
 */
typedef struct {
	u32 Enable; /**< [0,1] (Private) Enable the CC. This is distinct from
		removing from the CC from the sequence. When enable is not set,
		the associated CCID will appear in sequence, but its valid will
		not propagate. */
	/* FT CONFIG */
	u32 Numerology; /**< [0-6] Numerology (Mu) value for this CC.
		- 0 = 15 kHz
		- 1 = 30 kHz
		- 2 = 60 KHz
		- 3 = 120 KHz
		- 4 = 240 KHz
		- 5 = 480 KHz
		- 6 = 960 KHz
		Numerology must be 0 for LTE. */
	u32 FftSize; /**< [10,11,12] FFT size to be used for FFT of the CC.
		Valid sizes are:
		- 1024 = 0xA
		- 2048 = 0xB
		- 4096 = 0xC */
	u32 NumSubcarriers; /**< [0-14] Number of non-null subcarriers in this
		 CC. */
	u32 ScaleFactor; /**< [0-1023] Scaling factor for FFT output for this
		CC, represented as a fixed-point value in 0.10 fixed-point
		format. 0=>(multiple of 0), 1=>(multiple of 0.999) */
	u32 CommsStandard; /**< [0-1]
		- 1 = LTE
		- 0 = 5G NR */
	/* CC slot delay */
	u32 OutputDelay; /** [0-2047] Delay required before outputting CC
		in order to balance CC Filter group delay. */
	u32 PhaseCompensation[XDFEOFDM_PHASE_COMPENSATION_MAX]; /** Phase weight is
		a complex number with 0 to 15 bits providing the I and 16 to 31
		bits the Q part of the weight. */
} XDfeOfdm_InternalCarrierCfg;

/**
 * Full CC configuration.
 */
typedef struct {
	XDfeOfdm_CCSequence CCSequence; /**< CCID sequence */
	XDfeOfdm_FTSequence FTSequence; /**< CCID sequence */
	XDfeOfdm_InternalCarrierCfg
		CarrierCfg[XDFEOFDM_CC_NUM]; /**< CC configurations */
} XDfeOfdm_CCCfg;

/**
 * Orthogonal Division Multiplexing Status.
 */
typedef struct {
	u32 SaturationCCID; /**< [0,15] CCID in which saturation event
		occurred. */
	u32 SaturationCount; /**< [0,2**14-1] Saturation events count across
		real and imaginary components. */
	u32 CCUpdate; /**< [0,1] There has been an overflow or underflow in
		the filter for one or more of the antennas and CCIDs. */
	u32 FTCCSequenceError; /**< [0,1] TRIGGER.CC_UPDATE has been
		triggered. */
	u32 Saturation; /**< [0,1] A difference between CC_CONFIGURATION.
		SEQUENCE and DIN TID has been detected. */
	u32 Overflow; /**< [0,1] UL OFDM receives tready low during packet
		transaction */
} XDfeOfdm_Status;

/**
 * Interrupt mask.
 */
typedef struct {
	u32 CCUpdate; /**< [0,1] Mask CC update events */
	u32 FTCCSequenceError; /**< [0,1] Mask sequence mismatch events */
	u32 Saturation; /**< [0,1] Mask Saturation events */
	u32 Overflow; /**< [0,1] Mask Overflow events */
} XDfeOfdm_InterruptMask;

/**
 * OFDM Config Structure.
 */
typedef struct {
#ifndef SDT
	u32 DeviceId; /**< The component instance Id */
#else
	char *Name; /**< Unique name of the device */
#endif
	metal_phys_addr_t BaseAddr; /**< Instance base address */
	u32 NumAntenna; /**< Number of antenas */
	u32 AntennaInterleave; /**< Antenna interleave */
	u32 PhaseCompensation; /**< [0,1] Phase compesation
				0 - Phase compesation disabled
				1 - Phase compesation enabled */
} XDfeOfdm_Config;

/**
 * OFDM Structure.
 */
typedef struct {
	XDfeOfdm_Config Config; /**< Config Structure */
	XDfeOfdm_StateId StateId; /**< StateId */
	s32 NotUsedCCID; /**< Not used CCID */
	u32 CCSequenceLength; /**< Exact sequence length */
	char NodeName[XDFEOFDM_NODE_NAME_MAX_LENGTH]; /**< Node name */
	struct metal_io_region *Io; /**< Libmetal IO structure */
	struct metal_device *Device; /**< Libmetal device structure */
} XDfeOfdm;

/************************** Variable Definitions *****************************/
#ifdef __BAREMETAL__
extern XDfeOfdm_Config XDfeOfdm_ConfigTable[XDFEOFDM_MAX_NUM_INSTANCES];
#endif

/**************************** API declarations *******************************/
/* System initialization API */
XDfeOfdm *XDfeOfdm_InstanceInit(const char *DeviceNodeName);
void XDfeOfdm_InstanceClose(XDfeOfdm *InstancePtr);

/**
* @cond nocomments
*/
/* Register access API */
void XDfeOfdm_WriteReg(const XDfeOfdm *InstancePtr, u32 AddrOffset, u32 Data);
u32 XDfeOfdm_ReadReg(const XDfeOfdm *InstancePtr, u32 AddrOffset);
/**
* @endcond
*/

/* DFE OFDM component initialization API */
void XDfeOfdm_Reset(XDfeOfdm *InstancePtr);
void XDfeOfdm_Configure(XDfeOfdm *InstancePtr, XDfeOfdm_Cfg *Cfg);
void XDfeOfdm_Initialize(XDfeOfdm *InstancePtr, XDfeOfdm_Init *Init);
void XDfeOfdm_Activate(XDfeOfdm *InstancePtr, bool EnableLowPower);
void XDfeOfdm_Deactivate(XDfeOfdm *InstancePtr);
XDfeOfdm_StateId XDfeOfdm_GetStateID(XDfeOfdm *InstancePtr);

/* User APIs */
void XDfeOfdm_GetCurrentCCCfg(const XDfeOfdm *InstancePtr,
			      XDfeOfdm_CCCfg *CCCfg);
void XDfeOfdm_GetEmptyCCCfg(const XDfeOfdm *InstancePtr, XDfeOfdm_CCCfg *CCCfg);
void XDfeOfdm_GetCarrierCfg(const XDfeOfdm *InstancePtr, XDfeOfdm_CCCfg *CCCfg,
			    s32 CCID, u32 *CCSeqBitmap,
			    XDfeOfdm_CarrierCfg *CarrierCfg);

u32 XDfeOfdm_AddCCtoCCCfg(XDfeOfdm *InstancePtr, XDfeOfdm_CCCfg *CCCfg,
			  s32 CCID, u32 CCSeqBitmap,
			  const XDfeOfdm_CarrierCfg *CarrierCfg,
			  XDfeOfdm_FTSequence *FTSeq);
u32 XDfeOfdm_RemoveCCfromCCCfg(XDfeOfdm *InstancePtr, XDfeOfdm_CCCfg *CCCfg,
			       s32 CCID, XDfeOfdm_FTSequence *FTSeq);
u32 XDfeOfdm_UpdateCCinCCCfg(XDfeOfdm *InstancePtr, XDfeOfdm_CCCfg *CCCfg,
			     s32 CCID, const XDfeOfdm_CarrierCfg *CarrierCfg,
			     XDfeOfdm_FTSequence *FTSeq);

void XDfeOfdm_SetNextCCCfg(const XDfeOfdm *InstancePtr,
			   const XDfeOfdm_CCCfg *NextCCCfg);
u32 XDfeOfdm_EnableCCUpdateTrigger(const XDfeOfdm *InstancePtr);
u32 XDfeOfdm_SetNextCCCfgAndTrigger(const XDfeOfdm *InstancePtr,
				    XDfeOfdm_CCCfg *CCCfg);

u32 XDfeOfdm_AddCC(XDfeOfdm *InstancePtr, s32 CCID, u32 CCSeqBitmap,
		   const XDfeOfdm_CarrierCfg *CarrierCfg,
		   XDfeOfdm_FTSequence *FTSeq);
u32 XDfeOfdm_RemoveCC(XDfeOfdm *InstancePtr, s32 CCID,
		      XDfeOfdm_FTSequence *FTSeq);
u32 XDfeOfdm_UpdateCC(XDfeOfdm *InstancePtr, s32 CCID,
		      const XDfeOfdm_CarrierCfg *CarrierCfg,
		      XDfeOfdm_FTSequence *FTSeq);

void XDfeOfdm_GetTriggersCfg(const XDfeOfdm *InstancePtr,
			     XDfeOfdm_TriggerCfg *TriggerCfg);
void XDfeOfdm_SetTriggersCfg(const XDfeOfdm *InstancePtr,
			     XDfeOfdm_TriggerCfg *TriggerCfg);

void XDfeOfdm_SetTuserOutFrameLocation(const XDfeOfdm *InstancePtr,
				       u32 TuserOutFrameLocation);
u32 XDfeOfdm_GetTuserOutFrameLocation(const XDfeOfdm *InstancePtr);

void XDfeOfdm_GetEventStatus(const XDfeOfdm *InstancePtr,
			     XDfeOfdm_Status *Status);
void XDfeOfdm_ClearEventStatus(const XDfeOfdm *InstancePtr,
			       const XDfeOfdm_Status *Status);
void XDfeOfdm_SetInterruptMask(const XDfeOfdm *InstancePtr,
			       const XDfeOfdm_InterruptMask *Mask);
void XDfeOfdm_GetInterruptMask(const XDfeOfdm *InstancePtr,
			       XDfeOfdm_InterruptMask *Mask);
void XDfeOfdm_SetTUserDelay(const XDfeOfdm *InstancePtr, u32 Delay);
u32 XDfeOfdm_GetTUserDelay(const XDfeOfdm *InstancePtr);
u32 XDfeOfdm_GetDataLatency(const XDfeOfdm *InstancePtr);
void XDfeOfdm_GetVersions(const XDfeOfdm *InstancePtr,
			  XDfeOfdm_Version *SwVersion,
			  XDfeOfdm_Version *HwVersion);

#ifdef __cplusplus
}
#endif

#endif
/** @} */
