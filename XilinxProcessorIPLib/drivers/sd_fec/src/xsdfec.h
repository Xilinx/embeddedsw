/******************************************************************************
*
* Copyright (C) 2016 - 2018 Xilinx, Inc.  All rights reserved.
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
* THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
* THE SOFTWARE.
*
*
*
******************************************************************************/

/** \mainpage
 * SD-FEC standalone driver software API summary 
 * 
 * \section sec_init Initialization and Configuration
 * The XSdFec_Config structure is used by the driver to configure the FEC mode and interface parameters defined for each 
 * SD-FEC device. The configuration structure is created by the tool-chain based on HW build properties.
 *
 * To support multiple runtime loading and initialization strategies employed by various
 * operating systems, the driver instance can be initialized in one of two ways:
 * - XSdFecInitialize(InstancePtr, DeviceId)  - The driver looks up its own configuration structure created by the tool-chain 
 *                                              based on an ID provided by the tool-chain.
 * - XSdFecCfgInitialize(InstancePtr, CfgPtr) - Uses a configuration structure provided by the caller.
 *
 * \section sec_data Data Structures
 * One or more device specific headers are produced during the generation of the board support package, defining further device specific
 * configuration parameters:
 * - When the device is configured for Turbo Decode, the header x<ipinst_name>_turbo_params.h, contains an XSdFecturbo_parameters
 * structure which is populated to match the corresponding IP GUI configuration.
 * - When the device is configured for LDPC a header is generated per LDPC code specified on the corresponding IP GUI;
 * x<ipinst_name >_<code_id>_params.h. Each header defines an XSdFecldpc_parameters structure populated with the configuration 
 * data required for the corresponding LDPC code.
 *
 * \section sec_api API
 * The driver provides the following functions:
 * - XSdFecSetTurboParams(InstancePtr, ParamsPtr)                                        - Set Turbo parameters on a device
 * - XSdFecadd_ldpc_params(InstancePtr, CodeId, SCOffset, LAOffset, QCOffset, ParamsPtr) - Add LDPC parameters to a device
 * - XSdFecShareTableSize(ParamsPtr, SCSizePtr, LASizePtr, QCSizePtr)                    - Calculate share table size for a LDPC code
 * - XSdFecInterruptClassifier(InstancePtr)                                              - Classify interrupts
 *
 * In addition, the driver provides set and get functions for all the individual registers defined for the SD-FEC.
 *
 * \section sec_ex Example
 * The processor based example design output by the SD-FEC IP instance also includes an example application demonstrating a 
 * basic use case of the software driver.
 */

#ifndef XSDFEC_H
#define XSDFEC_H

#ifdef __cplusplus
extern "C" {
#endif

// Include Files
#include "xil_types.h"
#include "xil_assert.h"
#include "xstatus.h"
#include "xil_io.h"

#include "xsdfec_hw.h"

// Standard constants
#define XSDFEC_STANDARD_OTHER 0
#define XSDFEC_STANDARD_5G    1

// Type Definitions

/** \brief Device configuration
 *
 * Contains configuration information for the device.
 */
typedef struct {
    u16     DeviceId;
    UINTPTR BaseAddress;
    u32     Standard;
    u32     Initialization[4];
} XSdFec_Config;

/** \brief SD-FEC driver instance
 *
 * Contains state information for each device.
 */
typedef struct {
    UINTPTR BaseAddress;
    u32 IsReady;
    u32 Standard;
    u32 SCOffset[128]; /**< Lookup to SC table offsets for each code ID */
    u32 LAOffset[128]; /**< Lookup to LA table offsets for each code ID */
    u32 QCOffset[128]; /**< Lookup to QC table offsets for each code ID */
} XSdFec;

/** \brief Struct defining LDPC code parameters
 *
 * Member values are defined in device specific header x<ipinst_name >_<code_id>_params.h. as per IP GUI configuration
 */
typedef struct {
  u32  N;
  u32  K;
  u32  PSize;
  u32  NLayers;
  u32  NQC;
  u32  NMQC;
  u32  NM;
  u32  NormType;
  u32  NoPacking;
  u32  SpecialQC;
  u32  NoFinalParity;
  u32  MaxSchedule;
  u32* SCTable;
  u32* LATable;
  u32* QCTable;
} XSdFecLdpcParameters;

/** \brief Struct defining Turbo Decode parameters
 *
 * Member values defined in device specific header x<ipinst_name>_turbo_params.h as per IP GUI configuration
 */
typedef struct {
  u8  Alg;
  u16 Scale;
} XSdFecTurboParameters;

/// Default MaxScale Turbo configuration
#define XSDFEC_TD_PARAM_MAX_DEFAULT     { 0 , 12 }
/// Default MaxScale Turbo configuration
#define XSDFEC_TD_PARAM_MAXSTAR_DEFAULT { 1 , 0 }  // Set no scaling

/** \brief Interrupt class
 *
 * Members define interrupt class and action required
 */
typedef struct {
  u8 Intf;         /**< Triggered due to interface or control error (ISR) */
  u8 ECCSBit;      /**< Triggered due to single-bit ECC error (ECC_ISR)   */
  u8 ECCMBit;      /**< Triggered due to multi-bit ECC error (ECC_ISR)    */
  u8 RstReq;       /**< Device requires reset                             */
  u8 ReprogReq;    /**< Device requires reprogrammed                      */
  u8 ReCfgReq;     /**< FPGA requires reprogrammed                        */
} XSdFecInterruptClass;

// API Function Prototypes
/** \brief Device initialization
 *
 * The driver looks up its own configuration structure created by the tool-chain based on an ID provided by the tool-chain.
 */
int XSdFecInitialize(XSdFec *InstancePtr, u16 DeviceId);

/** \brief Configuration lookup
 *
 * Returns the configuration struct for a given device ID
 */
XSdFec_Config* XSdFecLookupConfig(u16 DeviceId);

/**\brief Device initialization
 *
 * Uses a configuration structure provided by the caller
 */
int XSdFecCfgInitialize(XSdFec *InstancePtr, XSdFec_Config *ConfigPtr);

/**\brief Add LDPC parameters to a device
 *
 * Updates LDPC code parameter registers and share tables using the specified CodeId and offsets with the specified parameters. 
 * The offsets arrays in the given XSdFec instance structure are updated with the supplied offsets for specified CodeId.  
 *
 * NOTE: When the device/IP has been configured to support the 5G NR standard the IP directly supports the 5G NR codes
 * and it is not necessary to add the codes at run-time. This function will generate an assertion if used on a instance
 * configured to support the 5G NR standard.
 *
 * @param InstancePtr Pointer to device instance struct
 * @param CodeId      Code number to be used for the specified LDPC code
 * @param SCOffset    Scale table offset to use for specified LDPC code
 * @param LAOffset    LA table offset to use for specified LDPC code
 * @param QSCOffset   QC table offset to use for specified LDPC code
 * @param ParamsPtr   Pointer to parameters struct for the LDPC code to be added to the device
 *
 */
void XSdFecAddLdpcParams(XSdFec *InstancePtr, u32 CodeId, u32 SCOffset, u32 LAOffset, u32 QCOffset, const XSdFecLdpcParameters* ParamsPtr);

/** \brief Set Turbo parameters on a device
 *
 * Updates Turbo code parameter registers
 *
 * @param InstancePtr Pointer to device instance struct
 * @param ParamsPtr   Pointer to Turbo parameters struct to be set on the device
 */
void XSdFecSetTurboParams(XSdFec *InstancePtr, const XSdFecTurboParameters* ParamsPtr);

/**\brief Calculate share table size for a LDPC code
 * 
 * Populates SCSizePtr, LASizePtr and QCSizePtr variables with the effective table size occupied by the specified 
 * LDPC code. These values can be used to increment the table offsets.
 *
 * @param ParamsPtr   Pointer to parameters struct for the LDPC code being queried
 * @param SCSizePtr   Pointer to variable to populate with the effective scale table size for the specified LDPC code
 * @param LASizePtr   Pointer to variable to populate with the effective LA table size for the specified LDPC code
 * @param QCSizePtr   Pointer to variable to populate with the effective QC table size for the specified LDPC code
 *
 */
void XSdFecShareTableSize(const XSdFecLdpcParameters* ParamsPtr, u32* SCSizePtr, u32* LASizePtr, u32* QCSizePtr);

/**\brief Classify interrupts
 * 
 * Queries interrupt status registers and classifies interrupt and reports recovery action
 * 
 * @param   InstancePtr       Pointer to device instance struct
 *
 * @returns Interrupt Class   Struct defining interrupt class and recover action
 */
XSdFecInterruptClass XSdFecInterruptClassifier(XSdFec *InstancePtr);

// Base API Function Prototypes
/**
 * CORE_AXI_WR_PROTECT access functions
 */
void XSdFecSet_CORE_AXI_WR_PROTECT(UINTPTR BaseAddress, u32 Data);
u32 XSdFecGet_CORE_AXI_WR_PROTECT(UINTPTR BaseAddress);

/**
 * CORE_CODE_WR_PROTECT access functions
 */
void XSdFecSet_CORE_CODE_WR_PROTECT(UINTPTR BaseAddress, u32 Data);
u32 XSdFecGet_CORE_CODE_WR_PROTECT(UINTPTR BaseAddress);

/**
 * CORE_ACTIVE access functions
 */
u32 XSdFecGet_CORE_ACTIVE(UINTPTR BaseAddress);

/**
 * CORE_AXIS_WIDTH access functions
 */
void XSdFecSet_CORE_AXIS_WIDTH_DIN(UINTPTR BaseAddress, u32 Data);
u32 XSdFecGet_CORE_AXIS_WIDTH_DIN(UINTPTR BaseAddress);
void XSdFecSet_CORE_AXIS_WIDTH_DIN_WORDS(UINTPTR BaseAddress, u32 Data);
u32 XSdFecGet_CORE_AXIS_WIDTH_DIN_WORDS(UINTPTR BaseAddress);
void XSdFecSet_CORE_AXIS_WIDTH_DOUT(UINTPTR BaseAddress, u32 Data);
u32 XSdFecGet_CORE_AXIS_WIDTH_DOUT(UINTPTR BaseAddress);
void XSdFecSet_CORE_AXIS_WIDTH_DOUT_WORDS(UINTPTR BaseAddress, u32 Data);
u32 XSdFecGet_CORE_AXIS_WIDTH_DOUT_WORDS(UINTPTR BaseAddress);
void XSdFecSet_CORE_AXIS_WIDTH(UINTPTR BaseAddress, u32 Data);
u32 XSdFecGet_CORE_AXIS_WIDTH(UINTPTR BaseAddress);

/**
 * CORE_AXIS_ENABLE access functions
 */
void XSdFecSet_CORE_AXIS_ENABLE_CTRL(UINTPTR BaseAddress, u32 Data);
u32 XSdFecGet_CORE_AXIS_ENABLE_CTRL(UINTPTR BaseAddress);
void XSdFecSet_CORE_AXIS_ENABLE_DIN(UINTPTR BaseAddress, u32 Data);
u32 XSdFecGet_CORE_AXIS_ENABLE_DIN(UINTPTR BaseAddress);
void XSdFecSet_CORE_AXIS_ENABLE_DIN_WORDS(UINTPTR BaseAddress, u32 Data);
u32 XSdFecGet_CORE_AXIS_ENABLE_DIN_WORDS(UINTPTR BaseAddress);
void XSdFecSet_CORE_AXIS_ENABLE_STATUS(UINTPTR BaseAddress, u32 Data);
u32 XSdFecGet_CORE_AXIS_ENABLE_STATUS(UINTPTR BaseAddress);
void XSdFecSet_CORE_AXIS_ENABLE_DOUT(UINTPTR BaseAddress, u32 Data);
u32 XSdFecGet_CORE_AXIS_ENABLE_DOUT(UINTPTR BaseAddress);
void XSdFecSet_CORE_AXIS_ENABLE_DOUT_WORDS(UINTPTR BaseAddress, u32 Data);
u32 XSdFecGet_CORE_AXIS_ENABLE_DOUT_WORDS(UINTPTR BaseAddress);
void XSdFecSet_CORE_AXIS_ENABLE(UINTPTR BaseAddress, u32 Data);
u32 XSdFecGet_CORE_AXIS_ENABLE(UINTPTR BaseAddress);

/**
 * CORE_ORDER access functions
 */
void XSdFecSet_CORE_ORDER(UINTPTR BaseAddress, u32 Data);
u32 XSdFecGet_CORE_ORDER(UINTPTR BaseAddress);

/**
 * CORE_ISR access functions
 */
void XSdFecSet_CORE_ISR(UINTPTR BaseAddress, u32 Data);
u32 XSdFecGet_CORE_ISR(UINTPTR BaseAddress);

/**
 * CORE_IER access functions
 */
void XSdFecSet_CORE_IER(UINTPTR BaseAddress, u32 Data);

/**
 * CORE_IDR access functions
 */
void XSdFecSet_CORE_IDR(UINTPTR BaseAddress, u32 Data);

/**
 * CORE_IMR access functions
 */
u32 XSdFecGet_CORE_IMR(UINTPTR BaseAddress);

/**
 * CORE_ECC_ISR access functions
 */
void XSdFecSet_CORE_ECC_ISR(UINTPTR BaseAddress, u32 Data);
u32 XSdFecGet_CORE_ECC_ISR(UINTPTR BaseAddress);

/**
 * CORE_ECC_IER access functions
 */
void XSdFecSet_CORE_ECC_IER(UINTPTR BaseAddress, u32 Data);

/**
 * CORE_ECC_IDR access functions
 */
void XSdFecSet_CORE_ECC_IDR(UINTPTR BaseAddress, u32 Data);

/**
 * CORE_ECC_IMR access functions
 */
u32 XSdFecGet_CORE_ECC_IMR(UINTPTR BaseAddress);

/**
 * CORE_BYPASS access functions
 */
void XSdFecSet_CORE_BYPASS(UINTPTR BaseAddress, u32 Data);
u32 XSdFecGet_CORE_BYPASS(UINTPTR BaseAddress);

/**
 * CORE_VERSION access functions
 */
u32 XSdFecGet_CORE_VERSION(UINTPTR BaseAddress);

/**
 * TURBO access functions
 */
void XSdFecSet_TURBO_ALG(UINTPTR BaseAddress, u32 Data);
u32 XSdFecGet_TURBO_ALG(UINTPTR BaseAddress);
void XSdFecSet_TURBO_SCALE_FACTOR(UINTPTR BaseAddress, u32 Data);
u32 XSdFecGet_TURBO_SCALE_FACTOR(UINTPTR BaseAddress);
void XSdFecSet_TURBO(UINTPTR BaseAddress, u32 Data);
u32 XSdFecGet_TURBO(UINTPTR BaseAddress);

/**
 * LDPC_CODE_REG0 access functions
 */
u32 XSdFecWrite_LDPC_CODE_REG0_N_Words(UINTPTR BaseAddress, u32 WordOffset, const u32 *DataArrayPtr, u32 NumData);
u32 XSdFecRead_LDPC_CODE_REG0_N_Words(UINTPTR BaseAddress, u32 WordOffset, u32 *DataArrayPtr, u32 NumData);
u32 XSdFecWrite_LDPC_CODE_REG0_K_Words(UINTPTR BaseAddress, u32 WordOffset, const u32 *DataArrayPtr, u32 NumData);
u32 XSdFecRead_LDPC_CODE_REG0_K_Words(UINTPTR BaseAddress, u32 WordOffset, u32 *DataArrayPtr, u32 NumData);
u32 XSdFecWrite_LDPC_CODE_REG0_Words(UINTPTR BaseAddress, u32 WordOffset, const u32 *DataArrayPtr, u32 NumData);
u32 XSdFecRead_LDPC_CODE_REG0_Words(UINTPTR BaseAddress, u32 WordOffset, u32 *DataArrayPtr, u32 NumData);

/**
 * LDPC_CODE_REG1 access functions
 */
u32 XSdFecWrite_LDPC_CODE_REG1_PSIZE_Words(UINTPTR BaseAddress, u32 WordOffset, const u32 *DataArrayPtr, u32 NumData);
u32 XSdFecRead_LDPC_CODE_REG1_PSIZE_Words(UINTPTR BaseAddress, u32 WordOffset, u32 *DataArrayPtr, u32 NumData);
u32 XSdFecWrite_LDPC_CODE_REG1_NO_PACKING_Words(UINTPTR BaseAddress, u32 WordOffset, const u32 *DataArrayPtr, u32 NumData);
u32 XSdFecRead_LDPC_CODE_REG1_NO_PACKING_Words(UINTPTR BaseAddress, u32 WordOffset, u32 *DataArrayPtr, u32 NumData);
u32 XSdFecWrite_LDPC_CODE_REG1_NM_Words(UINTPTR BaseAddress, u32 WordOffset, const u32 *DataArrayPtr, u32 NumData);
u32 XSdFecRead_LDPC_CODE_REG1_NM_Words(UINTPTR BaseAddress, u32 WordOffset, u32 *DataArrayPtr, u32 NumData);
u32 XSdFecWrite_LDPC_CODE_REG1_Words(UINTPTR BaseAddress, u32 WordOffset, const u32 *DataArrayPtr, u32 NumData);
u32 XSdFecRead_LDPC_CODE_REG1_Words(UINTPTR BaseAddress, u32 WordOffset, u32 *DataArrayPtr, u32 NumData);

/**
 * LDPC_CODE_REG2 access functions
 */
u32 XSdFecWrite_LDPC_CODE_REG2_NLAYERS_Words(UINTPTR BaseAddress, u32 WordOffset, const u32 *DataArrayPtr, u32 NumData);
u32 XSdFecRead_LDPC_CODE_REG2_NLAYERS_Words(UINTPTR BaseAddress, u32 WordOffset, u32 *DataArrayPtr, u32 NumData);
u32 XSdFecWrite_LDPC_CODE_REG2_NMQC_Words(UINTPTR BaseAddress, u32 WordOffset, const u32 *DataArrayPtr, u32 NumData);
u32 XSdFecRead_LDPC_CODE_REG2_NMQC_Words(UINTPTR BaseAddress, u32 WordOffset, u32 *DataArrayPtr, u32 NumData);
u32 XSdFecWrite_LDPC_CODE_REG2_NORM_TYPE_Words(UINTPTR BaseAddress, u32 WordOffset, const u32 *DataArrayPtr, u32 NumData);
u32 XSdFecRead_LDPC_CODE_REG2_NORM_TYPE_Words(UINTPTR BaseAddress, u32 WordOffset, u32 *DataArrayPtr, u32 NumData);
u32 XSdFecWrite_LDPC_CODE_REG2_SPECIAL_QC_Words(UINTPTR BaseAddress, u32 WordOffset, const u32 *DataArrayPtr, u32 NumData);
u32 XSdFecRead_LDPC_CODE_REG2_SPECIAL_QC_Words(UINTPTR BaseAddress, u32 WordOffset, u32 *DataArrayPtr, u32 NumData);
u32 XSdFecWrite_LDPC_CODE_REG2_NO_FINAL_PARITY_CHECK_Words(UINTPTR BaseAddress, u32 WordOffset, const u32 *DataArrayPtr, u32 NumData);
u32 XSdFecRead_LDPC_CODE_REG2_NO_FINAL_PARITY_CHECK_Words(UINTPTR BaseAddress, u32 WordOffset, u32 *DataArrayPtr, u32 NumData);
u32 XSdFecWrite_LDPC_CODE_REG2_MAX_SCHEDULE_Words(UINTPTR BaseAddress, u32 WordOffset, const u32 *DataArrayPtr, u32 NumData);
u32 XSdFecRead_LDPC_CODE_REG2_MAX_SCHEDULE_Words(UINTPTR BaseAddress, u32 WordOffset, u32 *DataArrayPtr, u32 NumData);
u32 XSdFecWrite_LDPC_CODE_REG2_Words(UINTPTR BaseAddress, u32 WordOffset, const u32 *DataArrayPtr, u32 NumData);
u32 XSdFecRead_LDPC_CODE_REG2_Words(UINTPTR BaseAddress, u32 WordOffset, u32 *DataArrayPtr, u32 NumData);

/**
 * LDPC_CODE_REG3 access functions
 */
u32 XSdFecWrite_LDPC_CODE_REG3_SC_OFF_Words(UINTPTR BaseAddress, u32 WordOffset, const u32 *DataArrayPtr, u32 NumData);
u32 XSdFecRead_LDPC_CODE_REG3_SC_OFF_Words(UINTPTR BaseAddress, u32 WordOffset, u32 *DataArrayPtr, u32 NumData);
u32 XSdFecWrite_LDPC_CODE_REG3_LA_OFF_Words(UINTPTR BaseAddress, u32 WordOffset, const u32 *DataArrayPtr, u32 NumData);
u32 XSdFecRead_LDPC_CODE_REG3_LA_OFF_Words(UINTPTR BaseAddress, u32 WordOffset, u32 *DataArrayPtr, u32 NumData);
u32 XSdFecWrite_LDPC_CODE_REG3_QC_OFF_Words(UINTPTR BaseAddress, u32 WordOffset, const u32 *DataArrayPtr, u32 NumData);
u32 XSdFecRead_LDPC_CODE_REG3_QC_OFF_Words(UINTPTR BaseAddress, u32 WordOffset, u32 *DataArrayPtr, u32 NumData);
u32 XSdFecWrite_LDPC_CODE_REG3_Words(UINTPTR BaseAddress, u32 WordOffset, const u32 *DataArrayPtr, u32 NumData);
u32 XSdFecRead_LDPC_CODE_REG3_Words(UINTPTR BaseAddress, u32 WordOffset, u32 *DataArrayPtr, u32 NumData);

/**
 * LDPC_SC_TABLE access functions
 */
u32 XSdFecWrite_LDPC_SC_TABLE_Words(UINTPTR BaseAddress, u32 WordOffset, const u32 *DataArrayPtr, u32 NumData);
u32 XSdFecRead_LDPC_SC_TABLE_Words(UINTPTR BaseAddress, u32 WordOffset, u32 *DataArrayPtr, u32 NumData);

/**
 * LDPC_LA_TABLE access functions
 */
u32 XSdFecWrite_LDPC_LA_TABLE_Words(UINTPTR BaseAddress, u32 WordOffset, const u32 *DataArrayPtr, u32 NumData);
u32 XSdFecRead_LDPC_LA_TABLE_Words(UINTPTR BaseAddress, u32 WordOffset, u32 *DataArrayPtr, u32 NumData);

/**
 * LDPC_QC_TABLE access functions
 */
u32 XSdFecWrite_LDPC_QC_TABLE_Words(UINTPTR BaseAddress, u32 WordOffset, const u32 *DataArrayPtr, u32 NumData);
u32 XSdFecRead_LDPC_QC_TABLE_Words(UINTPTR BaseAddress, u32 WordOffset, u32 *DataArrayPtr, u32 NumData);

// Macros (Inline Functions) Definitions
#define XSdFecWriteReg(BaseAddress, RegOffset, Data) \
    Xil_Out32((BaseAddress) + (RegOffset), (u32)(Data))
#define XSdFecReadReg(BaseAddress, RegOffset) \
    Xil_In32((BaseAddress) + (RegOffset))

#ifdef __cplusplus
}
#endif

#endif
