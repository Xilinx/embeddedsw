/******************************************************************************
* Copyright (C) 2021-2022 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xdfeequ.h
* @addtogroup Overview
* @{
*
* The RFSoC DFE Equalizer LogiCore builds upon the Complex Equalizer primitive
* (dfe_complex_eq). Each instance of the LogiCore supports up to 8
* antennas, each with its instance of the dfe_complex_eq primitive. The LogiCore
* provides access to the AXI stream data interfaces on each of the primitives.
* An AXI memory-mapped interface is provided, enabling the Equalizer driver to
* configure the IP from a microprocessor. TUSER and TLAST inputs are present
* on the AXI stream interfaces and delay matched with the data through the
* LogiCore. The features that the Equalizer IP and the driver support are:
* - Can operate in complex and real modes.
* - Enables the user to program the coefficient sets via a processor interface.
* - Enables the user to change the coefficient sets that act on the input data
*   via a processor interface.
* - Supports TDD power down via a processor interface.
* - Supports the flushing of the internal buffers via a processor interface.
* - Indication of overflow provided via a status register.
* - TUSER and TLAST information accompanying the data is delay matched through
*   the IP.
* - TUSER and TLAST can optionally be used to synchronize coefficient selection,
*   power up/down, and the buffers' flushing.
* Features which are not provided are:
* - Does not support the dynamic changing of the coefficient sets that act on
*   the input data via the AXI stream interface.
* - Does not provide direct programming of the coefficient sets via an AXI
*   stream interface.
* - Does not currently support configuration of the filter coefficients at
*   startup.
* @cond nocomments
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who    Date     Changes
* ----- ---    -------- -----------------------------------------------
* 1.0   dc     09/03/20 Initial version
*       dc     02/02/21 Remove hard coded device node name
*       dc     02/22/21 align driver to current specification
*       dc     03/15/21 Add data latency api
*       dc     04/06/21 Register with full node name
*       dc     04/20/21 Doxygen documentation update
*       dc     05/08/21 Update to common trigger
* 1.1   dc     07/13/21 Update to common latency requirements
* 1.2   dc     10/29/21 Update doxygen comments
*       dc     11/09/21 Add GetStateId API
*       dc     11/05/21 Align event handlers
*       dc     11/19/21 Update doxygen documentation
*       dc     12/17/21 Update after documentation review
*       dc     01/24/22 Auto-compute number of units
* 1.3   dc     02/18/22 Write 1 clears event status
*       dc     03/21/22 Add prefix to global variables
*
* </pre>
* @endcond
******************************************************************************/
#ifndef XDFEEQU_H_
#define XDFEEQU_H_

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
#define XDFEEQU_MAX_NUM_INSTANCES                                              \
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
#define XST_SUCCESS (0L) /**< Success flag */
#endif
#ifndef XST_FAILURE
#define XST_FAILURE (1L) /**< Failure flag */
#endif
#else
#define XDFEEQU_MAX_NUM_INSTANCES XPAR_XDFEEQU_NUM_INSTANCES
#endif

#define XDFEEQU_NODE_NAME_MAX_LENGTH (50U) /**< Node name maximum length */

#define XDFEEQU_ANT_NUM_MAX (8U) /**< Maximum anntena number */
#define XDFEEQU_CHANNEL_NUM (8U) /**< Maximum channel number */
#define XDFEEQU_MAX_NUMBER_OF_UNITS_COMPLEX (0x3U) /**< Complex units number */
#define XDFEEQU_MAX_NUMBER_OF_UNITS_REAL (0x6U) /**< Real units number */
#define XDFEEQU_NUM_COEFF (24U) /**< Maximum number of coefficents */
#define XDFEEQU_DATAPATH_MODE_REAL (0U) /**< Real mode */
#define XDFEEQU_DATAPATH_MODE_COMPLEX (1U) /**< Complex mode */
/**
* @cond nocomments
*/
#define XDFEEQU_DATAPATH_MODE_MATRIX (2U) /**< Matrix mode, obsolete for now */
/**
* @endcond
*/

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
#define XDFEEQU_CUSTOM_DEV(_dev_name, _baseaddr, _idx)                         \
	{                                                                      \
		.name = _dev_name, .bus = NULL, .num_regions = 1,              \
		.regions = { {                                                 \
			.virt = (void *)_baseaddr,                             \
			.physmap = &XDfeEqu_metal_phys[_idx],                  \
			.size = 0x10000,                                       \
			.page_shift = (u32)(-1),                               \
			.page_mask = (u32)(-1),                                \
			.mem_flags = 0x0,                                      \
			.ops = { NULL },                                       \
		} },                                                           \
		.node = { NULL }, .irq_num = 0, .irq_info = NULL,              \
	}
#endif

typedef enum XDfeEqu_StateId {
	XDFEEQU_STATE_NOT_READY = 0, /**< Not ready state.*/
	XDFEEQU_STATE_READY, /**< Ready state.*/
	XDFEEQU_STATE_RESET, /**< Reset state.*/
	XDFEEQU_STATE_CONFIGURED, /**< Configured state.*/
	XDFEEQU_STATE_INITIALISED, /**< Initialised state.*/
	XDFEEQU_STATE_OPERATIONAL /**< Operational state.*/
} XDfeEqu_StateId;

/**
 * Logicore version.
 */
typedef struct {
	u32 Major; /**< Major version number. */
	u32 Minor; /**< Minor version number. */
	u32 Revision; /**< Revision number. */
	u32 Patch; /**< Patch number. */
} XDfeEqu_Version;

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
} XDfeEqu_Trigger;

/**
 * All IP triggers.
 */
typedef struct {
	XDfeEqu_Trigger Activate; /**< Controls the activation and configuration
		updates of the IP. */
	XDfeEqu_Trigger LowPower; /**< Switch between "Low-power"
		and "Operational" state. */
	XDfeEqu_Trigger
		Update; /**< Controls the update of the IP configuration. */
} XDfeEqu_TriggerCfg;

/*********** end - common code to all Logiccores ************/
/**
 * Equalizer Filter model parameters structure. Data defined in Device
 * tree/xparameters.h.
 */
typedef struct {
	u32 NumChannels; /**< Number of channels */
	u32 SampleWidth; /**< Sample width */
	u32 ComplexModel; /**< Complex mode flag */
	u32 TuserWidth; /**< Tuser width */
} XDfeEqu_ModelParameters;

/**
 * Configuration.
 */
typedef struct {
	XDfeEqu_Version Version; /**< Logicore version */
	XDfeEqu_ModelParameters ModelParams; /**< Logicore
		parameterization */
} XDfeEqu_Cfg;

/**
 * Equalizer Coefficients Structure.
 */
typedef struct {
	u32 Num; /**< [1-12|24] True number of coefficients,
		[1 - 12] in complex mode. */
	u32 Set; /**< [0-3] Coefficient set that the coefficients apply to */
	s16 Coefficients[XDFEEQU_NUM_COEFF]; /**< Signed real numbers. Array of
		Coefficients. */
} XDfeEqu_Coefficients;

/**
 * Equalizer Configuration Structure.
 */
typedef struct {
	u32 Flush; /**< [0,1] Set high to flush the buffers. */
	u32 DatapathMode; /**< [real, complex]
		Set depending on whether the equalizer is running in real,
		complex mode.
		Each of the eight channels consists of 2 sub-channels.
		In complex mode, the 2 sub-channels form a single filter
		channel acting on the data's real and imaginary parts.
		In real mode, the two sub-channels act as independent
		filter channels acting on the two real samples at the input.*/
	u32 RealDatapathSet; /**< [0-3] Coefficient set to use for real
		data path. In complex mode the datapath set is limited to
		0 or 2. */
	u32 ImDatapathSet; /**< [0-1] Coefficient set to use for imaginary
		datapath. */
} XDfeEqu_EqConfig;

/**
 * Equalizer Status.
 */
typedef struct {
	u32 IStatus[XDFEEQU_CHANNEL_NUM]; /**< bit0 is overflow of I datapath
		and bit 1 is overflow of Q datapath in real mode only. */
	u32 QStatus[XDFEEQU_CHANNEL_NUM]; /**< Overflow of Q datapath in
		complex mode. */
} XDfeEqu_Status;

/**
 * Status Mask.
 */
typedef struct {
	u32 Mask[XDFEEQU_CHANNEL_NUM]; /**< [0,1] Mask status events. */
} XDfeEqu_InterruptMask;

/**
 * Equalizer Config Structure.
 */
typedef struct {
	u32 DeviceId; /**< The component instance Id */
	metal_phys_addr_t BaseAddr; /**< Instance base address */
	u32 NumChannels; /**< Number of channels */
	u32 SampleWidth; /**< Sample width */
	u32 ComplexModel; /**< Complex mode flag */
	u32 TuserWidth; /**< Tuser width */
} XDfeEqu_Config;

/**
 * Equalizer Structure.
 */
typedef struct {
	XDfeEqu_Config Config; /**< Config Structure */
	XDfeEqu_StateId StateId; /**< StateId */
	char NodeName[XDFEEQU_NODE_NAME_MAX_LENGTH]; /**< Node name */
	struct metal_io_region *Io; /**< Libmetal IO structure */
	struct metal_device *Device; /**< Libmetal device structure */
} XDfeEqu;

/**************************** API declarations *******************************/
/* System initialization API */
XDfeEqu *XDfeEqu_InstanceInit(const char *DeviceNodeName);
void XDfeEqu_InstanceClose(XDfeEqu *InstancePtr);

/* Register access API */
/**
* @cond nocomments
*/
void XDfeEqu_WriteReg(const XDfeEqu *InstancePtr, u32 AddrOffset, u32 Data);
u32 XDfeEqu_ReadReg(const XDfeEqu *InstancePtr, u32 AddrOffset);
/**
* @endcond
*/

/* DFE Equalizer component initialization API */
void XDfeEqu_Reset(XDfeEqu *InstancePtr);
void XDfeEqu_Configure(XDfeEqu *InstancePtr, XDfeEqu_Cfg *Cfg);
void XDfeEqu_Initialize(XDfeEqu *InstancePtr, const XDfeEqu_EqConfig *Config);
void XDfeEqu_Activate(XDfeEqu *InstancePtr, bool EnableLowPower);
void XDfeEqu_Deactivate(XDfeEqu *InstancePtr);
XDfeEqu_StateId XDfeEqu_GetStateID(XDfeEqu *InstancePtr);

/* User APIs */
void XDfeEqu_Update(const XDfeEqu *InstancePtr, const XDfeEqu_EqConfig *Config);
void XDfeEqu_GetTriggersCfg(const XDfeEqu *InstancePtr,
			    XDfeEqu_TriggerCfg *TriggerCfg);
void XDfeEqu_SetTriggersCfg(const XDfeEqu *InstancePtr,
			    XDfeEqu_TriggerCfg *TriggerCfg);
void XDfeEqu_LoadCoefficients(const XDfeEqu *InstancePtr, u32 ChannelField,
			      u32 Mode, u32 Shift,
			      const XDfeEqu_Coefficients *EqCoeffs);
void XDfeEqu_GetEventStatus(const XDfeEqu *InstancePtr, XDfeEqu_Status *Status);
void XDfeEqu_ClearEventStatus(const XDfeEqu *InstancePtr,
			      const XDfeEqu_Status *Status);
void XDfeEqu_SetInterruptMask(const XDfeEqu *InstancePtr,
			      const XDfeEqu_InterruptMask *InterruptMask);
void XDfeEqu_GetInterruptMask(const XDfeEqu *InstancePtr,
			      XDfeEqu_InterruptMask *InterruptMask);
void XDfeEqu_GetActiveSets(const XDfeEqu *InstancePtr, u32 *RealSet,
			   u32 *ImagSet);
void XDfeEqu_SetTUserDelay(const XDfeEqu *InstancePtr, u32 Delay);
u32 XDfeEqu_GetTUserDelay(const XDfeEqu *InstancePtr);
u32 XDfeEqu_GetTDataDelay(const XDfeEqu *InstancePtr, u32 Tap);
void XDfeEqu_GetVersions(const XDfeEqu *InstancePtr, XDfeEqu_Version *SwVersion,
			 XDfeEqu_Version *HwVersion);

#ifdef __cplusplus
}
#endif

#endif
/** @} */
