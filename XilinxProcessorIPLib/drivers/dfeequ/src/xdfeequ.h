/******************************************************************************
* Copyright (C) 2021 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xdfeequ.h
* @addtogroup xdfeequ_v1_0
* @{
*
* The DFE Equalizer LogiCore builds upon the Complex Equalizer block
* (dfe_complex_eq). Each instance of the LogiCore supports up to 8 antennas,
* each with its own instance of the dfe_complex_eq block. The LogiCore provides
* access to the AXI stream data interfaces on each of the blocks. An AXI memory
* mapped interface is provided, enabling the Mixer driver configuring the block
* from a microprocessor. TUSER and TLAST inputs are present on the AXI stream
* interfaces and delay matched with the data through the LogiCore.
* The features that the Mixer IP and the driver support are:
* - Can operate in complex (matrix) and real modes.
* - Enables the user to program the co-efficient sets via a processor interface.
* - Enables the user to change the co-efficient sets that act on the input data
*   via a processor interface.
* - Supports TDD power down via a processor interface.
* - Supports the flushing of the internal buffers via a processor interface.
* - Indication of overflow provided via a status register.
* - TUSER and TLAST information accompanying the data is delay matched through
*   the IP.
* - TUSER and TLAST can optionally be used to synchronize co-efficient
*   selection, power up/down and the flushing of the buffers.
* Features which are not provided are:
* - Does not support the dynamic changing of the co-efficient sets that act on
*   the input data via the AXI stream interface.
* - Does not provide direct programming of the co-efficient sets via an AXI
*   stream interface.
* - Does not currently support configuration of the filter co-efficients
*   at start up.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who    Date     Changes
* ----- ---    -------- -----------------------------------------------
* 1.0   dc     09/03/20 Initial version
*       dc     02/02/21 Remove hard coded device node name
* </pre>
*
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
#define XDFEEQU_MAX_NUM_INSTANCES 10U
#define Xil_AssertNonvoid(Expression) assert(Expression)
#define Xil_AssertVoid(Expression) assert(Expression)
#define Xil_AssertVoidAlways() assert(0)
#define XST_SUCCESS 0U
#define XST_FAILURE 1U
#else
#define XDFEEQU_MAX_NUM_INSTANCES XPAR_XDFEEQU_NUM_INSTANCES
#endif

#define XDFEEQU_CC_NUM 16U /**< Maximum CC number */
#define XDFEEQU_ANT_NUM_MAX 8U /**< Maximum anntena number */
#define XDFEEQU_SEQ_LENGTH_MAX 16U /**< Maximum sequence length */

#define XDFEEQU_RATE_MAX 5U /**< Maximum rate Id */
#define XDFEEQU_NCO_MAX 4U /**< Maximum NCO number */

#define XDFEEQU_CHANNEL_NUM 8U
#define XDFEEQU_MAX_NUMBER_OF_UNITS_COMPLEX 0x3U
#define XDFEEQU_MAX_NUMBER_OF_UNITS_REAL 0x6U

#define XDFEEQU_TRIGGER_SOURCE_IMMEDIATE 0U
#define XDFEEQU_TRIGGER_SOURCE_TUSER 1U
#define XDFEEQU_TRIGGER_SOURCE_TLAST 2U

#define XDFEEQU_DATAPATH_MODE_REAL 0U
#define XDFEEQU_DATAPATH_MODE_COMPLEX 1U
#define XDFEEQU_DATAPATH_MODE_MATRIX 2U

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
	XDFEEQU_STATE_NOT_READY = 0, /**< Not ready state*/
	XDFEEQU_STATE_READY, /**< Ready state*/
	XDFEEQU_STATE_RESET, /**< Reset state*/
	XDFEEQU_STATE_CONFIGURED, /**< Configured state*/
	XDFEEQU_STATE_INITIALISED, /**< Initialised state*/
	XDFEEQU_STATE_OPERATIONAL /**< Operational state*/
} XDfeEqu_StateId;

/**
 * Logicore version.
 */
typedef struct {
	u32 Major; /**< Major version number */
	u32 Minor; /**< Minor version number */
	u32 Revision; /**< Revision number */
	u32 Patch; /**< Patch number */
} XDfeEqu_Version;

/**
 * Trigger configuration.
 */
typedef struct {
	u32 Source; /**< [0,1,2],
		0 = IMMEDIATE: write to the trigger configuration register
			immediately
		1 = TUSER: write on Edge detected on specified TUSER bit
		2 = TLAST: write on Edge detected on TLAST */
	u32 TUSERBit; /**< [0-7], Species which TUSER bit is used by
		the trigger */
	u32 Polarity; /**< [0,1], 0 = Active High, 1 = Active Low */
} XDfeEqu_Trigger;

/**
 * Defines a CCID sequence.
 */
typedef struct {
	u32 Length; /**< [1-16] Sequence length */
	u32 CCID[XDFEEQU_SEQ_LENGTH_MAX]; /**< [0-15].Array of CCID's
		arranged in the order the CCIDs are required to be processed
		in the channel filter */
} XDfeEqu_CCSequence;

/*********** end - common code to all Logiccores ************/
/**
 * Equalizer Filter model parameters structure. Data defined in Device
 * tree/xparameters.h
 */
typedef struct {
	u32 DataIWidth;
	u32 DataOWidth;
	u32 NumChannels;
	u32 TuserWidth;
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
	u32 Coefficients[24]; /**< Signed real numbers. Array of
		Co-efficients */
	u32 Set; /**< [0-3] Co-efficient set that the co-efficients apply to */
	u32 NUnits; /**< [1-6] Number of active units. 1 - 6 in real mode.
		1 - 3 in complex and matrix mode. */
	u32 Shift; /**< [0-7] Shift value. Set by the formula given in
		specification item 10 in complex equalizer */
} XDfeEqu_Coeff;

/**
 * Equalizer Configuration Structure.
 */
typedef struct {
	u32 DatapathMode; /**< [real, complex, matrix]
		Set depending on whether the equalizer is running in real,
		complex or matrix mode.
		Each of the 8 channels consists of 2 sub-channels (shown in
		figure below in xDFEEqualizerCoefficientsT description).
		In complex and matrix modes the 2 sub-channels form a single
		filter channel acting on the real and imaginary parts of
		the data.
		In real mode the 2 sub-channels act as independent filter
		channels acting on the 2 real samples at the input.Set
		depending on whether the equalizer is running in real
		 or complex mode */
	u32 RealDatapathSet; /**< [0-3] Co-efficient set to use for real
		datapath (Ha and Hb in complex and matrix mode. Ha, Hb, Hc
		and Hd in real mode). In complex mode the datapath set is
		limited to 0 or 2. */
	u32 ImDatapathSet; /**< [0-3] Matrix mode only. Co-efficient set to use
		for imaginary datapath (Hc and Hd). */
} XDfeEqu_EqConfig;

/**
 * Equalizer Status.
 */
typedef struct {
	u32 IStatus; /**< Status of the real part of the output. In real mode
		both bits can be populated. In complex mode only bit 0
		is relevant */
	u32 QStatus; /**< Status of the imaginary part of the output. Only
		valid in complex mode */
} XDfeEqu_Status;

/**
 * Status Mask.
 */
typedef struct {
	u32 Status; /**< [0,1] Mask status events */
} XDfeEqu_InterruptMask;

/**
 * Equalizer Config Structure.
 */
typedef struct {
	u32 DeviceId;
	metal_phys_addr_t BaseAddr;
	u32 DataIWidth;
	u32 DataOWidth;
	u32 NumChannels;
	u32 TuserWidth;
} XDfeEqu_Config;

/**
 * Equalizer Structure.
 */
typedef struct {
	XDfeEqu_Config Config; /**< Config Structure */
	XDfeEqu_StateId StateId; /**< StateId */
	struct metal_io_region *Io; /**< Libmetal IO structure */
	struct metal_device *Device; /**< Libmetal device structure */
} XDfeEqu;

/**************************** API declarations *******************************/
/* System initialization API */
XDfeEqu *XDfeEqu_InstanceInit(u16 DeviceId, const char *DeviceNodeName);
void XDfeEqu_InstanceClose(XDfeEqu *InstancePtr);

/* Register access API */
void XDfeEqu_WriteReg(const XDfeEqu *InstancePtr, u32 AddrOffset, u32 Data);
u32 XDfeEqu_ReadReg(const XDfeEqu *InstancePtr, u32 AddrOffset);

/* DFE Equalizer component initialization API */
void XDfeEqu_Reset(XDfeEqu *InstancePtr);
void XDfeEqu_Configure(XDfeEqu *InstancePtr, XDfeEqu_Cfg *Cfg);
void XDfeEqu_Initialize(XDfeEqu *InstancePtr, const XDfeEqu_EqConfig *Config);
void XDfeEqu_Activate(XDfeEqu *InstancePtr,
		      const XDfeEqu_Trigger *TriggerSource);
void XDfeEqu_Deactivate(XDfeEqu *InstancePtr,
			const XDfeEqu_Trigger *TriggerSource);

/* User APIs */
void XDfeEqu_SwitchCoefficientSet(XDfeEqu *InstancePtr,
				  const XDfeEqu_EqConfig *Config,
				  const XDfeEqu_Trigger *TriggerSource);
void XDfeEqu_LoadRealCoefficients(const XDfeEqu *InstancePtr, u32 ChannelField,
				  const XDfeEqu_Coeff *EqCoeffs);
void XDfeEqu_LoadComplexCoefficients(const XDfeEqu *InstancePtr,
				     u32 ChannelField,
				     const XDfeEqu_Coeff *EqCoeffs);
void XDfeEqu_LoadMatrixCoefficients(const XDfeEqu *InstancePtr,
				    u32 ChannelField,
				    const XDfeEqu_Coeff *EqCoeffs);
void XDfeEqu_GetEventStatus(const XDfeEqu *InstancePtr, u32 ChannelId,
			    XDfeEqu_Status *Status);
void XDfeEqu_ClearEventStatus(const XDfeEqu *InstancePtr, u32 ChannelId);
void XDfeEqu_SetInterruptMask(const XDfeEqu *InstancePtr, u32 ChannelField,
			      u32 StatusMask);
void XDfeEqu_GetCoefficientSet(const XDfeEqu *InstancePtr,
			       XDfeEqu_EqConfig *Config);
void XDfeEqu_FlushBuffers(XDfeEqu *InstancePtr,
			  const XDfeEqu_Trigger *TriggerSource);
void XDfeEqu_SetPowerMode(XDfeEqu *InstancePtr,
			  const XDfeEqu_Trigger *TriggerSource, u32 Mode);
void XDfeEqu_GetPowerMode(XDfeEqu *InstancePtr, u32 *Mode);

#ifdef __cplusplus
}
#endif

#endif
/** @} */
