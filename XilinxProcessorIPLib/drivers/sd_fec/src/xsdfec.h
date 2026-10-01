/******************************************************************************
* Copyright (C) 2022 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* Copyright (C) 2016 - 2022 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

#ifndef XSDFEC_H
#define XSDFEC_H

/****************************************************************************/
/**
*
* @file xsdfec.h
* @addtogroup sd_fec Overview
* @{
* @brief Provides the Soft Decision FEC driver.
*
* The driver uses the XSdFec_Config structure to configure the FEC mode
* and interface parameters defined for each SD-FEC device. The
* configuration structure is created by the tool-chain based on HW build
* properties.
*
* To support multiple runtime loading and initialization strategies employed
* by various operating systems, the driver instance can be initialized in
* one of two ways:
* - XSdFecInitialize(InstancePtr, DeviceId) - The driver looks up its own
*   configuration structure created by the tool-chain based on an ID (for
*   XSCT build flow) or Base Address (for SDT flow) provided by the
*   tool-chain.
* - XSdFecCfgInitialize(InstancePtr, CfgPtr) - Uses a configuration
*   structure provided by the caller.
*
* <b>Data Structures</b>
*
* One or more device specific headers are produced during generation of the
* board support package, defining further device specific configuration
* parameters:
* - When the device is configured for Turbo Decode, the header
*   "x<ipinst_name>_turbo_params.h" contains an XSdFecTurboParameters
*   structure which is populated to match the corresponding IP GUI
*   configuration.
* - When the device is configured for LDPC a header is generated per LDPC
*   code specified on the corresponding IP GUI,
*   "x<ipinst_name>_<code_id>_params.h". Each header defines an
*   XSdFecLdpcParameters structure populated with the configuration data
*   required for the corresponding LDPC code.
*
* <b>API</b>
*
* The driver provides the following functions:
* - XSdFecSetTurboParams() - Set Turbo parameters on a device
* - XSdFecAddLdpcParams() - Add LDPC parameters to a device
* - XSdFecShareTableSize() - Calculate share table size for an LDPC code
* - XSdFecInterruptClassifier() - Classify interrupts
*
* In addition, the driver provides set and get functions for all the
* individual registers defined for the SD-FEC.
*
* <b>Example</b>
*
* The processor based example design output by the SD-FEC IP instance also
* includes an example application demonstrating a basic use case of the
* software driver.
*
*****************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/

#include "xil_types.h"
#include "xil_assert.h"
#include "xstatus.h"
#include "xil_io.h"

#include "xsdfec_hw.h"

/************************** Constant Definitions ****************************/

/** @name FEC standard identifiers
 * @{
 */
#define XSDFEC_STANDARD_OTHER 0 /**< Device is not configured for 5G NR. */
/** Device is configured for the 5G NR standard. */
#define XSDFEC_STANDARD_5G    1
/** @} */

/**************************** Type Definitions *******************************/

/**
* @brief Holds device configuration.
*
* Contains configuration information for the device.
*/
typedef struct {
#ifndef SDT
    u16     DeviceId; /**< Unique device ID. */
#else
	char *Name; /**< Device name in the SDT flow. */
#endif
    UINTPTR BaseAddress; /**< Device base address. */
    u32     Standard; /**< Configured standard. */
    u32     Initialization[4]; /**< Fixed register address and data pairs. */
} XSdFec_Config;

/**
* @brief Holds state information for an SD-FEC instance.
*
* Contains state information for each device.
*/
typedef struct {
    UINTPTR BaseAddress; /**< Device base address. */
    u32 IsReady; /**< Device is initialized and ready. */
    u32 Standard; /**< Configured standard. */
    u32 SCOffset[128]; /**< Lookup to SC table offsets for each code ID. */
    u32 LAOffset[128]; /**< Lookup to LA table offsets for each code ID. */
    u32 QCOffset[128]; /**< Lookup to QC table offsets for each code ID. */
} XSdFec;

/**
* @brief Holds LDPC code parameters.
*
* Member values are defined in the device specific header
* "x<ipinst_name>_<code_id>_params.h" as per IP GUI configuration.
*/
typedef struct {
  u32  N; /**< Codeword length N. */
  u32  K; /**< Information length K. */
  u32  PSize; /**< Sub-matrix size. */
  u32  NLayers; /**< Number of layers. */
  u32  NQC; /**< Number of QC entries. */
  u32  NMQC; /**< Number of QC operations. */
  u32  NM; /**< Number of circulants. */
  u32  NormType; /**< Normalization type. */
  u32  NoPacking; /**< Disable packing. */
  u32  SpecialQC; /**< Special QC flag. */
  u32  NoFinalParity; /**< Disable final parity check. */
  u32  MaxSchedule; /**< Maximum schedule value. */
  u32* SCTable; /**< Pointer to the scale table. */
  u32* LATable; /**< Pointer to the layered architecture table. */
  u32* QCTable; /**< Pointer to the QC table. */
} XSdFecLdpcParameters;

/**
* @brief Holds Turbo Decode parameters.
*
* Member values are defined in the device specific header
* "x<ipinst_name>_turbo_params.h" as per IP GUI configuration.
*/
typedef struct {
  u8  Alg; /**< Turbo algorithm. */
  u16 Scale; /**< Turbo scale factor. */
} XSdFecTurboParameters;

/** @name Default Turbo parameter initializers
 * @{
 */
/** Default MaxScale Turbo configuration. */
#define XSDFEC_TD_PARAM_MAX_DEFAULT     { 0 , 12 }
/** Default MaxStar Turbo configuration with no scaling. */
#define XSDFEC_TD_PARAM_MAXSTAR_DEFAULT { 1 , 0 }
/** @} */

/**
* @brief Holds interrupt class and recovery action.
*
* Members define interrupt class and action required.
*/
typedef struct {
  u8 Intf;         /**< Triggered due to interface or control error (ISR). */
  u8 ECCSBit;      /**< Triggered due to single-bit ECC error (ECC_ISR). */
  u8 ECCMBit;      /**< Triggered due to multi-bit ECC error (ECC_ISR). */
  u8 RstReq;       /**< Device requires reset. */
  u8 ReprogReq;    /**< Device requires reprogramming. */
  u8 ReCfgReq;     /**< FPGA requires reprogramming. */
} XSdFecInterruptClass;

/************************** Function Prototypes ******************************/

#ifndef SDT
int XSdFecInitialize(XSdFec *InstancePtr, u16 DeviceId);
#else
int XSdFecInitialize(XSdFec *InstancePtr, UINTPTR BaseAddress);
#endif

#ifndef SDT
XSdFec_Config* XSdFecLookupConfig(u16 DeviceId);
#else
/*****************************************************************************/
/**
* @brief Looks up the XSdFec configuration by base address.
*
* Returns the configuration structure for a given base address.
*
* @param[in]	BaseAddress Base address of the device being looked up.
*
* @return
*        - A pointer to the configuration found.
*        - NULL if the specified base address was not found.
*
******************************************************************************/
XSdFec_Config *XSdFec_LookupConfig(UINTPTR BaseAddress);
#endif
int XSdFecCfgInitialize(XSdFec *InstancePtr, XSdFec_Config *ConfigPtr);

void XSdFecAddLdpcParams(XSdFec *InstancePtr, u32 CodeId, u32 SCOffset, u32 LAOffset, u32 QCOffset, const XSdFecLdpcParameters* ParamsPtr);
void XSdFecSetTurboParams(XSdFec *InstancePtr, const XSdFecTurboParameters* ParamsPtr);
void XSdFecShareTableSize(const XSdFecLdpcParameters* ParamsPtr, u32* SCSizePtr, u32* LASizePtr, u32* QCSizePtr);
XSdFecInterruptClass XSdFecInterruptClassifier(XSdFec *InstancePtr);

/* CORE_AXI_WR_PROTECT access functions */
void XSdFecSet_CORE_AXI_WR_PROTECT(UINTPTR BaseAddress, u32 Data);
u32 XSdFecGet_CORE_AXI_WR_PROTECT(UINTPTR BaseAddress);

/* CORE_CODE_WR_PROTECT access functions */
void XSdFecSet_CORE_CODE_WR_PROTECT(UINTPTR BaseAddress, u32 Data);
u32 XSdFecGet_CORE_CODE_WR_PROTECT(UINTPTR BaseAddress);

/* CORE_ACTIVE access functions */
u32 XSdFecGet_CORE_ACTIVE(UINTPTR BaseAddress);

/* CORE_AXIS_WIDTH access functions */
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

/* CORE_AXIS_ENABLE access functions */
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

/* CORE_ORDER access functions */
void XSdFecSet_CORE_ORDER(UINTPTR BaseAddress, u32 Data);
u32 XSdFecGet_CORE_ORDER(UINTPTR BaseAddress);

/* CORE_ISR access functions */
void XSdFecSet_CORE_ISR(UINTPTR BaseAddress, u32 Data);
u32 XSdFecGet_CORE_ISR(UINTPTR BaseAddress);

/* CORE_IER access functions */
void XSdFecSet_CORE_IER(UINTPTR BaseAddress, u32 Data);

/* CORE_IDR access functions */
void XSdFecSet_CORE_IDR(UINTPTR BaseAddress, u32 Data);

/* CORE_IMR access functions */
u32 XSdFecGet_CORE_IMR(UINTPTR BaseAddress);

/* CORE_ECC_ISR access functions */
void XSdFecSet_CORE_ECC_ISR(UINTPTR BaseAddress, u32 Data);
u32 XSdFecGet_CORE_ECC_ISR(UINTPTR BaseAddress);

/* CORE_ECC_IER access functions */
void XSdFecSet_CORE_ECC_IER(UINTPTR BaseAddress, u32 Data);

/* CORE_ECC_IDR access functions */
void XSdFecSet_CORE_ECC_IDR(UINTPTR BaseAddress, u32 Data);

/* CORE_ECC_IMR access functions */
u32 XSdFecGet_CORE_ECC_IMR(UINTPTR BaseAddress);

/* CORE_BYPASS access functions */
void XSdFecSet_CORE_BYPASS(UINTPTR BaseAddress, u32 Data);
u32 XSdFecGet_CORE_BYPASS(UINTPTR BaseAddress);

/* CORE_VERSION access functions */
u32 XSdFecGet_CORE_VERSION(UINTPTR BaseAddress);

/* TURBO access functions */
void XSdFecSet_TURBO_ALG(UINTPTR BaseAddress, u32 Data);
u32 XSdFecGet_TURBO_ALG(UINTPTR BaseAddress);
void XSdFecSet_TURBO_SCALE_FACTOR(UINTPTR BaseAddress, u32 Data);
u32 XSdFecGet_TURBO_SCALE_FACTOR(UINTPTR BaseAddress);
void XSdFecSet_TURBO(UINTPTR BaseAddress, u32 Data);
u32 XSdFecGet_TURBO(UINTPTR BaseAddress);

/* LDPC_CODE_REG0 access functions */
u32 XSdFecWrite_LDPC_CODE_REG0_N_Words(UINTPTR BaseAddress, u32 WordOffset, const u32 *DataArrayPtr, u32 NumData);
u32 XSdFecRead_LDPC_CODE_REG0_N_Words(UINTPTR BaseAddress, u32 WordOffset, u32 *DataArrayPtr, u32 NumData);
u32 XSdFecWrite_LDPC_CODE_REG0_K_Words(UINTPTR BaseAddress, u32 WordOffset, const u32 *DataArrayPtr, u32 NumData);
u32 XSdFecRead_LDPC_CODE_REG0_K_Words(UINTPTR BaseAddress, u32 WordOffset, u32 *DataArrayPtr, u32 NumData);
u32 XSdFecWrite_LDPC_CODE_REG0_Words(UINTPTR BaseAddress, u32 WordOffset, const u32 *DataArrayPtr, u32 NumData);
u32 XSdFecRead_LDPC_CODE_REG0_Words(UINTPTR BaseAddress, u32 WordOffset, u32 *DataArrayPtr, u32 NumData);

/* LDPC_CODE_REG1 access functions */
u32 XSdFecWrite_LDPC_CODE_REG1_PSIZE_Words(UINTPTR BaseAddress, u32 WordOffset, const u32 *DataArrayPtr, u32 NumData);
u32 XSdFecRead_LDPC_CODE_REG1_PSIZE_Words(UINTPTR BaseAddress, u32 WordOffset, u32 *DataArrayPtr, u32 NumData);
u32 XSdFecWrite_LDPC_CODE_REG1_NO_PACKING_Words(UINTPTR BaseAddress, u32 WordOffset, const u32 *DataArrayPtr, u32 NumData);
u32 XSdFecRead_LDPC_CODE_REG1_NO_PACKING_Words(UINTPTR BaseAddress, u32 WordOffset, u32 *DataArrayPtr, u32 NumData);
u32 XSdFecWrite_LDPC_CODE_REG1_NM_Words(UINTPTR BaseAddress, u32 WordOffset, const u32 *DataArrayPtr, u32 NumData);
u32 XSdFecRead_LDPC_CODE_REG1_NM_Words(UINTPTR BaseAddress, u32 WordOffset, u32 *DataArrayPtr, u32 NumData);
u32 XSdFecWrite_LDPC_CODE_REG1_Words(UINTPTR BaseAddress, u32 WordOffset, const u32 *DataArrayPtr, u32 NumData);
u32 XSdFecRead_LDPC_CODE_REG1_Words(UINTPTR BaseAddress, u32 WordOffset, u32 *DataArrayPtr, u32 NumData);

/* LDPC_CODE_REG2 access functions */
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

/* LDPC_CODE_REG3 access functions */
u32 XSdFecWrite_LDPC_CODE_REG3_SC_OFF_Words(UINTPTR BaseAddress, u32 WordOffset, const u32 *DataArrayPtr, u32 NumData);
u32 XSdFecRead_LDPC_CODE_REG3_SC_OFF_Words(UINTPTR BaseAddress, u32 WordOffset, u32 *DataArrayPtr, u32 NumData);
u32 XSdFecWrite_LDPC_CODE_REG3_LA_OFF_Words(UINTPTR BaseAddress, u32 WordOffset, const u32 *DataArrayPtr, u32 NumData);
u32 XSdFecRead_LDPC_CODE_REG3_LA_OFF_Words(UINTPTR BaseAddress, u32 WordOffset, u32 *DataArrayPtr, u32 NumData);
u32 XSdFecWrite_LDPC_CODE_REG3_QC_OFF_Words(UINTPTR BaseAddress, u32 WordOffset, const u32 *DataArrayPtr, u32 NumData);
u32 XSdFecRead_LDPC_CODE_REG3_QC_OFF_Words(UINTPTR BaseAddress, u32 WordOffset, u32 *DataArrayPtr, u32 NumData);
u32 XSdFecWrite_LDPC_CODE_REG3_Words(UINTPTR BaseAddress, u32 WordOffset, const u32 *DataArrayPtr, u32 NumData);
u32 XSdFecRead_LDPC_CODE_REG3_Words(UINTPTR BaseAddress, u32 WordOffset, u32 *DataArrayPtr, u32 NumData);

/* LDPC_SC_TABLE access functions */
u32 XSdFecWrite_LDPC_SC_TABLE_Words(UINTPTR BaseAddress, u32 WordOffset, const u32 *DataArrayPtr, u32 NumData);
u32 XSdFecRead_LDPC_SC_TABLE_Words(UINTPTR BaseAddress, u32 WordOffset, u32 *DataArrayPtr, u32 NumData);

/* LDPC_LA_TABLE access functions */
u32 XSdFecWrite_LDPC_LA_TABLE_Words(UINTPTR BaseAddress, u32 WordOffset, const u32 *DataArrayPtr, u32 NumData);
u32 XSdFecRead_LDPC_LA_TABLE_Words(UINTPTR BaseAddress, u32 WordOffset, u32 *DataArrayPtr, u32 NumData);

/* LDPC_QC_TABLE access functions */
u32 XSdFecWrite_LDPC_QC_TABLE_Words(UINTPTR BaseAddress, u32 WordOffset, const u32 *DataArrayPtr, u32 NumData);
u32 XSdFecRead_LDPC_QC_TABLE_Words(UINTPTR BaseAddress, u32 WordOffset, u32 *DataArrayPtr, u32 NumData);

/***************** Macros (Inline Functions) Definitions *********************/

/*****************************************************************************/
/**
* @brief Writes a 32-bit SD-FEC register.
*
* @param[in]	BaseAddress Base address of the device.
* @param[in]	RegOffset Offset of the register from BaseAddress.
* @param[in]	Data Value to write to the register.
*
* @note
* C-style signature
*	void XSdFecWriteReg(UINTPTR BaseAddress, u32 RegOffset, u32 Data)
*
******************************************************************************/
#define XSdFecWriteReg(BaseAddress, RegOffset, Data) \
    Xil_Out32((BaseAddress) + (RegOffset), (u32)(Data))

/*****************************************************************************/
/**
* @brief Reads a 32-bit SD-FEC register.
*
* @param[in]	BaseAddress Base address of the device.
* @param[in]	RegOffset Offset of the register from BaseAddress.
*
* @return 32-bit value read from the register.
*
* @note
* C-style signature
*	u32 XSdFecReadReg(UINTPTR BaseAddress, u32 RegOffset)
*
******************************************************************************/
#define XSdFecReadReg(BaseAddress, RegOffset) \
    Xil_In32((BaseAddress) + (RegOffset))

#ifdef __cplusplus
}
#endif

#endif /* XSDFEC_H */
/** @} */
