/******************************************************************************
* Copyright (C) 2016 - 2020 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2022 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/****************************************************************************/
/**
*
* @file xsdfec.c
* @brief Implements the XSdFec interface functions.
*
* Contains the driver API functions that access the Soft Decision FEC device.
*
* Refer to the xsdfec.h header file for more information about this driver.
*
*****************************************************************************/

/**
* @addtogroup sd_fec_api SD_FEC APIs
* @{
*/

/***************************** Include Files *********************************/
#include "xsdfec.h"

/************************** Function Implementation *************************/
/*****************************************************************************/
/**
* @brief Initializes a specific XSdFec instance.
*
* Uses a configuration structure provided by the caller. Writes the fixed
* register initialization values and marks the instance ready.
*
* @param[out]	InstancePtr Pointer to the XSdFec instance.
* @param[in]	ConfigPtr Pointer to the device configuration structure with
*		the required HW build data.
*
* @return XST_SUCCESS if initialization succeeds.
*
******************************************************************************/
int XSdFecCfgInitialize(XSdFec *InstancePtr, XSdFec_Config *ConfigPtr) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(ConfigPtr != NULL);

    InstancePtr->BaseAddress  = ConfigPtr->BaseAddress;
    InstancePtr->Standard     = ConfigPtr->Standard;
    /* Initialize fixed register configurations */
    for (int i=0;i<4;i+=2) {
      u32 addr  = ConfigPtr->Initialization[i];
      u32 wdata = ConfigPtr->Initialization[i+1];
      XSdFecWriteReg(InstancePtr->BaseAddress, addr, wdata);
    }
    InstancePtr->IsReady = XIL_COMPONENT_IS_READY;

    return XST_SUCCESS;
}

/*****************************************************************************/
/**
* @brief Adds LDPC parameters to a device.
*
* Updates LDPC code parameter registers and share tables using the specified
* CodeId and offsets with the specified parameters. The offset arrays in the
* given XSdFec instance structure are updated with the supplied offsets for the
* specified CodeId. Has no effect if CodeId is 128 or greater.
*
* @param[in,out]	InstancePtr Pointer to the XSdFec instance.
* @param[in]	CodeId Code number of the LDPC code.
* @param[in]	SCOffset Scale table offset of the LDPC code.
* @param[in]	LAOffset LA table offset of the LDPC code.
* @param[in]	QCOffset QC table offset of the LDPC code.
* @param[in]	ParamsPtr Pointer to the LDPC parameters to add to the device.
*
* @note When the device has been configured to support the 5G NR standard, the
*       IP directly supports the 5G NR codes and it is not necessary to add the
*       codes at run-time. Generates an assertion if used on an instance
*       configured to support the 5G NR standard.
*
******************************************************************************/
void XSdFecAddLdpcParams(XSdFec *InstancePtr, u32 CodeId, u32 SCOffset, u32 LAOffset, u32 QCOffset, const XSdFecLdpcParameters* ParamsPtr) {
  Xil_AssertVoid(InstancePtr != NULL);
  Xil_AssertVoid(ParamsPtr   != NULL);
  Xil_AssertVoid(InstancePtr->IsReady  == XIL_COMPONENT_IS_READY);
  Xil_AssertVoid(InstancePtr->Standard == XSDFEC_STANDARD_OTHER);

  u32 wr_data = 0;
  if (CodeId < 128) {
    wr_data = 0;
    wr_data |= (XSDFEC_LDPC_CODE_REG0_N_MASK & (ParamsPtr->N << XSDFEC_LDPC_CODE_REG0_N_LSB));
    wr_data |= (XSDFEC_LDPC_CODE_REG0_K_MASK & (ParamsPtr->K << XSDFEC_LDPC_CODE_REG0_K_LSB));
    XSdFecWrite_LDPC_CODE_REG0_Words(InstancePtr->BaseAddress,CodeId,&wr_data,1);
    wr_data = 0;
    wr_data |= (XSDFEC_LDPC_CODE_REG1_PSIZE_MASK       & (ParamsPtr->PSize      << XSDFEC_LDPC_CODE_REG1_PSIZE_LSB));
    wr_data |= (XSDFEC_LDPC_CODE_REG1_NO_PACKING_MASK  & (ParamsPtr->NoPacking  << XSDFEC_LDPC_CODE_REG1_NO_PACKING_LSB));
    wr_data |= (XSDFEC_LDPC_CODE_REG1_NM_MASK          & (ParamsPtr->NM         << XSDFEC_LDPC_CODE_REG1_NM_LSB));
    XSdFecWrite_LDPC_CODE_REG1_Words(InstancePtr->BaseAddress,CodeId,&wr_data,1);
    wr_data = 0;
    wr_data |= (XSDFEC_LDPC_CODE_REG2_NLAYERS_MASK               & (ParamsPtr->NLayers        << XSDFEC_LDPC_CODE_REG2_NLAYERS_LSB));
    wr_data |= (XSDFEC_LDPC_CODE_REG2_NMQC_MASK                  & (ParamsPtr->NMQC           << XSDFEC_LDPC_CODE_REG2_NMQC_LSB));
    wr_data |= (XSDFEC_LDPC_CODE_REG2_NORM_TYPE_MASK             & (ParamsPtr->NormType       << XSDFEC_LDPC_CODE_REG2_NORM_TYPE_LSB));
    wr_data |= (XSDFEC_LDPC_CODE_REG2_SPECIAL_QC_MASK            & (ParamsPtr->SpecialQC      << XSDFEC_LDPC_CODE_REG2_SPECIAL_QC_LSB));
    wr_data |= (XSDFEC_LDPC_CODE_REG2_NO_FINAL_PARITY_CHECK_MASK & (ParamsPtr->NoFinalParity  << XSDFEC_LDPC_CODE_REG2_NO_FINAL_PARITY_CHECK_LSB));
    wr_data |= (XSDFEC_LDPC_CODE_REG2_MAX_SCHEDULE_MASK          & (ParamsPtr->MaxSchedule    << XSDFEC_LDPC_CODE_REG2_MAX_SCHEDULE_LSB));
    XSdFecWrite_LDPC_CODE_REG2_Words(InstancePtr->BaseAddress,CodeId,&wr_data,1);
    wr_data = 0;
    wr_data |= (XSDFEC_LDPC_CODE_REG3_SC_OFF_MASK & (SCOffset << XSDFEC_LDPC_CODE_REG3_SC_OFF_LSB));
    wr_data |= (XSDFEC_LDPC_CODE_REG3_LA_OFF_MASK & (LAOffset << XSDFEC_LDPC_CODE_REG3_LA_OFF_LSB));
    wr_data |= (XSDFEC_LDPC_CODE_REG3_QC_OFF_MASK & (QCOffset << XSDFEC_LDPC_CODE_REG3_QC_OFF_LSB));
    XSdFecWrite_LDPC_CODE_REG3_Words(InstancePtr->BaseAddress,CodeId,&wr_data,1);

    XSdFecWrite_LDPC_SC_TABLE_Words(InstancePtr->BaseAddress,SCOffset  , ParamsPtr->SCTable,(ParamsPtr->NLayers+3)>>2); // Scale is packed, 4 per reg
    XSdFecWrite_LDPC_LA_TABLE_Words(InstancePtr->BaseAddress,LAOffset*4, ParamsPtr->LATable,ParamsPtr->NLayers); // Further 4x applied to offset in function
    XSdFecWrite_LDPC_QC_TABLE_Words(InstancePtr->BaseAddress,QCOffset*4, ParamsPtr->QCTable,ParamsPtr->NQC);

    /* Store offsets */
    InstancePtr->SCOffset[CodeId] = SCOffset;
    InstancePtr->LAOffset[CodeId] = LAOffset;
    InstancePtr->QCOffset[CodeId] = QCOffset;
  }
}

/*****************************************************************************/
/**
* @brief Calculates the share table sizes of an LDPC code.
*
* Populates SCSizePtr, LASizePtr, and QCSizePtr with the effective table size
* occupied by the specified LDPC code. These values can be used to increment the
* table offsets. A size pointer that is NULL is skipped.
*
* @param[in]	ParamsPtr Pointer to the LDPC parameters being queried.
* @param[out]	SCSizePtr Pointer to the effective scale table size of the
*		LDPC code.
* @param[out]	LASizePtr Pointer to the effective LA table size of the LDPC
*		code.
* @param[out]	QCSizePtr Pointer to the effective QC table size of the LDPC
*		code.
*
******************************************************************************/
void XSdFecShareTableSize(const XSdFecLdpcParameters* ParamsPtr, u32* SCSizePtr, u32* LASizePtr, u32* QCSizePtr) {
  Xil_AssertVoid(ParamsPtr != NULL);
  if (SCSizePtr) {
    *SCSizePtr = (ParamsPtr->NLayers+3)>>2;
  }
  if (LASizePtr) {
    *LASizePtr = ((ParamsPtr->NLayers<<2)+15)>>4; /* Multiple of 16 */
  }
  if (QCSizePtr) {
    *QCSizePtr = ((ParamsPtr->NQC<<2)+15)>>4;
  }
}

/*****************************************************************************/
/**
* @brief Sets Turbo parameters on a device.
*
* Writes the ALG and SCALE_FACTOR fields of the TURBO register from the
* specified parameters.
*
* @param[in]	InstancePtr Pointer to the XSdFec instance.
* @param[in]	ParamsPtr Pointer to the Turbo parameters to set on the device.
*
* @note Generates an assertion if used on an instance configured to support
*       the 5G NR standard.
*
******************************************************************************/
void XSdFecSetTurboParams(XSdFec *InstancePtr, const XSdFecTurboParameters* ParamsPtr) {
  Xil_AssertVoid(InstancePtr != NULL);
  Xil_AssertVoid(ParamsPtr   != NULL);
  Xil_AssertVoid(InstancePtr->IsReady  == XIL_COMPONENT_IS_READY);
  Xil_AssertVoid(InstancePtr->Standard == XSDFEC_STANDARD_OTHER);
  XSdFecSet_TURBO_ALG(InstancePtr->BaseAddress,ParamsPtr->Alg);
  XSdFecSet_TURBO_SCALE_FACTOR(InstancePtr->BaseAddress,ParamsPtr->Scale);
}

/*****************************************************************************/
/**
* @brief Classifies SD-FEC interrupts.
*
* Queries interrupt status registers and classifies the interrupt and reports
* the recovery action.
*
* @param[in]	InstancePtr Pointer to the XSdFec instance.
*
* @return An XSdFecInterruptClass structure that describes the interrupt class
*         and recovery action.
*
******************************************************************************/
XSdFecInterruptClass XSdFecInterruptClassifier(XSdFec *InstancePtr) {
  XSdFecInterruptClass IntClass;

  IntClass.Intf       = 0;
  IntClass.ECCSBit    = 0;
  IntClass.ECCMBit    = 0;
  IntClass.RstReq     = 0;
  IntClass.ReprogReq  = 0;
  IntClass.ReCfgReq   = 0;

  u32 isr     = XSdFecGet_CORE_ISR(InstancePtr->BaseAddress);
  u32 isr_ecc = XSdFecGet_CORE_ECC_ISR(InstancePtr->BaseAddress);

  if (isr) {
    IntClass.Intf   = 1;
    IntClass.RstReq = 1;
  }
  /* Hard block ECC error (single or multi-bit) */
  u32 ecc_errbits  = isr_ecc & 0x0007FF;
  /* Hard block multi-bit */
  u32 ecc_errmbits = (isr_ecc & 0x3FF800) >> 11;
  /* PL logic ECC error (single or multi-bit) */
  u32 soft_ecc_errbits  = (isr_ecc & 0x03C00000) >> 22;
  /* PL logic multi-bit */
  u32 soft_ecc_errmbits = (isr_ecc & 0x3C000000) >> 26;
  /* XOR ecc error bits with multi-bit errors to determine any single bit */
  if (ecc_errbits ^ ecc_errmbits || soft_ecc_errbits ^ soft_ecc_errmbits) {
    IntClass.ECCSBit = 1;
  }
  if (ecc_errmbits) {
    IntClass.ECCMBit    = 1;
    IntClass.RstReq     = 1;
    IntClass.ReprogReq  = 1;
  }
  if (soft_ecc_errmbits) {
    IntClass.ReCfgReq   = 1;
  }
  return IntClass;
}

/*****************************************************************************/
/**
* @brief Writes the CORE_AXI_WR_PROTECT register.
*
* @param[in]	BaseAddress Base address of the device.
* @param[in]	Data Value to write to the register.
*
******************************************************************************/
void XSdFecSet_CORE_AXI_WR_PROTECT(UINTPTR BaseAddress, u32 Data) {
  XSdFecWriteReg(BaseAddress, XSDFEC_CORE_AXI_WR_PROTECT_ADDR, Data);
}

/*****************************************************************************/
/**
* @brief Reads the CORE_AXI_WR_PROTECT register.
*
* @param[in]	BaseAddress Base address of the device.
*
* @return The current register value.
*
******************************************************************************/
u32 XSdFecGet_CORE_AXI_WR_PROTECT(UINTPTR BaseAddress) {
  return XSdFecReadReg(BaseAddress, XSDFEC_CORE_AXI_WR_PROTECT_ADDR);
}

/*****************************************************************************/
/**
* @brief Writes the CORE_CODE_WR_PROTECT register.
*
* @param[in]	BaseAddress Base address of the device.
* @param[in]	Data Value to write to the register.
*
******************************************************************************/
void XSdFecSet_CORE_CODE_WR_PROTECT(UINTPTR BaseAddress, u32 Data) {
  XSdFecWriteReg(BaseAddress, XSDFEC_CORE_CODE_WR_PROTECT_ADDR, Data);
}

/*****************************************************************************/
/**
* @brief Reads the CORE_CODE_WR_PROTECT register.
*
* @param[in]	BaseAddress Base address of the device.
*
* @return The current register value.
*
******************************************************************************/
u32 XSdFecGet_CORE_CODE_WR_PROTECT(UINTPTR BaseAddress) {
  return XSdFecReadReg(BaseAddress, XSDFEC_CORE_CODE_WR_PROTECT_ADDR);
}

/*****************************************************************************/
/**
* @brief Reads the CORE_ACTIVE register.
*
* @param[in]	BaseAddress Base address of the device.
*
* @return The current register value.
*
******************************************************************************/
u32 XSdFecGet_CORE_ACTIVE(UINTPTR BaseAddress) {
  return XSdFecReadReg(BaseAddress, XSDFEC_CORE_ACTIVE_ADDR);
}

/*****************************************************************************/
/**
* @brief Writes the DIN field of the CORE_AXIS_WIDTH register.
*
* @param[in]	BaseAddress Base address of the device.
* @param[in]	Data Value to write to the field.
*
******************************************************************************/
void XSdFecSet_CORE_AXIS_WIDTH_DIN(UINTPTR BaseAddress, u32 Data) {
  u32 rdata = XSdFecReadReg(BaseAddress, XSDFEC_CORE_AXIS_WIDTH_ADDR);
  rdata = rdata & ~XSDFEC_CORE_AXIS_WIDTH_DIN_MASK;
  u32 wdata = (rdata & ~XSDFEC_CORE_AXIS_WIDTH_DIN_MASK) | (XSDFEC_CORE_AXIS_WIDTH_DIN_MASK & (Data << XSDFEC_CORE_AXIS_WIDTH_DIN_LSB));
  XSdFecWriteReg(BaseAddress, XSDFEC_CORE_AXIS_WIDTH_ADDR, wdata);
}

/*****************************************************************************/
/**
* @brief Reads the DIN field of the CORE_AXIS_WIDTH register.
*
* @param[in]	BaseAddress Base address of the device.
*
* @return The current DIN field value.
*
******************************************************************************/
u32 XSdFecGet_CORE_AXIS_WIDTH_DIN(UINTPTR BaseAddress) {
  u32 rdata = XSdFecReadReg(BaseAddress, XSDFEC_CORE_AXIS_WIDTH_ADDR);
  return (rdata & XSDFEC_CORE_AXIS_WIDTH_DIN_MASK) >> XSDFEC_CORE_AXIS_WIDTH_DIN_LSB;
}

/*****************************************************************************/
/**
* @brief Writes the DIN_WORDS field of the CORE_AXIS_WIDTH register.
*
* @param[in]	BaseAddress Base address of the device.
* @param[in]	Data Value to write to the field.
*
******************************************************************************/
void XSdFecSet_CORE_AXIS_WIDTH_DIN_WORDS(UINTPTR BaseAddress, u32 Data) {
  u32 rdata = XSdFecReadReg(BaseAddress, XSDFEC_CORE_AXIS_WIDTH_ADDR);
  rdata = rdata & ~XSDFEC_CORE_AXIS_WIDTH_DIN_WORDS_MASK;
  u32 wdata = (rdata & ~XSDFEC_CORE_AXIS_WIDTH_DIN_WORDS_MASK) | (XSDFEC_CORE_AXIS_WIDTH_DIN_WORDS_MASK & (Data << XSDFEC_CORE_AXIS_WIDTH_DIN_WORDS_LSB));
  XSdFecWriteReg(BaseAddress, XSDFEC_CORE_AXIS_WIDTH_ADDR, wdata);
}

/*****************************************************************************/
/**
* @brief Reads the DIN_WORDS field of the CORE_AXIS_WIDTH register.
*
* @param[in]	BaseAddress Base address of the device.
*
* @return The current DIN_WORDS field value.
*
******************************************************************************/
u32 XSdFecGet_CORE_AXIS_WIDTH_DIN_WORDS(UINTPTR BaseAddress) {
  u32 rdata = XSdFecReadReg(BaseAddress, XSDFEC_CORE_AXIS_WIDTH_ADDR);
  return (rdata & XSDFEC_CORE_AXIS_WIDTH_DIN_WORDS_MASK) >> XSDFEC_CORE_AXIS_WIDTH_DIN_WORDS_LSB;
}

/*****************************************************************************/
/**
* @brief Writes the DOUT field of the CORE_AXIS_WIDTH register.
*
* @param[in]	BaseAddress Base address of the device.
* @param[in]	Data Value to write to the field.
*
******************************************************************************/
void XSdFecSet_CORE_AXIS_WIDTH_DOUT(UINTPTR BaseAddress, u32 Data) {
  u32 rdata = XSdFecReadReg(BaseAddress, XSDFEC_CORE_AXIS_WIDTH_ADDR);
  rdata = rdata & ~XSDFEC_CORE_AXIS_WIDTH_DOUT_MASK;
  u32 wdata = (rdata & ~XSDFEC_CORE_AXIS_WIDTH_DOUT_MASK) | (XSDFEC_CORE_AXIS_WIDTH_DOUT_MASK & (Data << XSDFEC_CORE_AXIS_WIDTH_DOUT_LSB));
  XSdFecWriteReg(BaseAddress, XSDFEC_CORE_AXIS_WIDTH_ADDR, wdata);
}

/*****************************************************************************/
/**
* @brief Reads the DOUT field of the CORE_AXIS_WIDTH register.
*
* @param[in]	BaseAddress Base address of the device.
*
* @return The current DOUT field value.
*
******************************************************************************/
u32 XSdFecGet_CORE_AXIS_WIDTH_DOUT(UINTPTR BaseAddress) {
  u32 rdata = XSdFecReadReg(BaseAddress, XSDFEC_CORE_AXIS_WIDTH_ADDR);
  return (rdata & XSDFEC_CORE_AXIS_WIDTH_DOUT_MASK) >> XSDFEC_CORE_AXIS_WIDTH_DOUT_LSB;
}

/*****************************************************************************/
/**
* @brief Writes the DOUT_WORDS field of the CORE_AXIS_WIDTH register.
*
* @param[in]	BaseAddress Base address of the device.
* @param[in]	Data Value to write to the field.
*
******************************************************************************/
void XSdFecSet_CORE_AXIS_WIDTH_DOUT_WORDS(UINTPTR BaseAddress, u32 Data) {
  u32 rdata = XSdFecReadReg(BaseAddress, XSDFEC_CORE_AXIS_WIDTH_ADDR);
  rdata = rdata & ~XSDFEC_CORE_AXIS_WIDTH_DOUT_WORDS_MASK;
  u32 wdata = (rdata & ~XSDFEC_CORE_AXIS_WIDTH_DOUT_WORDS_MASK) | (XSDFEC_CORE_AXIS_WIDTH_DOUT_WORDS_MASK & (Data << XSDFEC_CORE_AXIS_WIDTH_DOUT_WORDS_LSB));
  XSdFecWriteReg(BaseAddress, XSDFEC_CORE_AXIS_WIDTH_ADDR, wdata);
}

/*****************************************************************************/
/**
* @brief Reads the DOUT_WORDS field of the CORE_AXIS_WIDTH register.
*
* @param[in]	BaseAddress Base address of the device.
*
* @return The current DOUT_WORDS field value.
*
******************************************************************************/
u32 XSdFecGet_CORE_AXIS_WIDTH_DOUT_WORDS(UINTPTR BaseAddress) {
  u32 rdata = XSdFecReadReg(BaseAddress, XSDFEC_CORE_AXIS_WIDTH_ADDR);
  return (rdata & XSDFEC_CORE_AXIS_WIDTH_DOUT_WORDS_MASK) >> XSDFEC_CORE_AXIS_WIDTH_DOUT_WORDS_LSB;
}

/*****************************************************************************/
/**
* @brief Writes the CORE_AXIS_WIDTH register.
*
* @param[in]	BaseAddress Base address of the device.
* @param[in]	Data Value to write to the register.
*
******************************************************************************/
void XSdFecSet_CORE_AXIS_WIDTH(UINTPTR BaseAddress, u32 Data) {
  XSdFecWriteReg(BaseAddress, XSDFEC_CORE_AXIS_WIDTH_ADDR, Data);
}

/*****************************************************************************/
/**
* @brief Reads the CORE_AXIS_WIDTH register.
*
* @param[in]	BaseAddress Base address of the device.
*
* @return The current register value.
*
******************************************************************************/
u32 XSdFecGet_CORE_AXIS_WIDTH(UINTPTR BaseAddress) {
  return XSdFecReadReg(BaseAddress, XSDFEC_CORE_AXIS_WIDTH_ADDR);
}

/*****************************************************************************/
/**
* @brief Writes the CTRL field of the CORE_AXIS_ENABLE register.
*
* @param[in]	BaseAddress Base address of the device.
* @param[in]	Data Value to write to the field.
*
******************************************************************************/
void XSdFecSet_CORE_AXIS_ENABLE_CTRL(UINTPTR BaseAddress, u32 Data) {
  u32 rdata = XSdFecReadReg(BaseAddress, XSDFEC_CORE_AXIS_ENABLE_ADDR);
  rdata = rdata & ~XSDFEC_CORE_AXIS_ENABLE_CTRL_MASK;
  u32 wdata = (rdata & ~XSDFEC_CORE_AXIS_ENABLE_CTRL_MASK) | (XSDFEC_CORE_AXIS_ENABLE_CTRL_MASK & (Data << XSDFEC_CORE_AXIS_ENABLE_CTRL_LSB));
  XSdFecWriteReg(BaseAddress, XSDFEC_CORE_AXIS_ENABLE_ADDR, wdata);
}

/*****************************************************************************/
/**
* @brief Reads the CTRL field of the CORE_AXIS_ENABLE register.
*
* @param[in]	BaseAddress Base address of the device.
*
* @return The current CTRL field value.
*
******************************************************************************/
u32 XSdFecGet_CORE_AXIS_ENABLE_CTRL(UINTPTR BaseAddress) {
  u32 rdata = XSdFecReadReg(BaseAddress, XSDFEC_CORE_AXIS_ENABLE_ADDR);
  return (rdata & XSDFEC_CORE_AXIS_ENABLE_CTRL_MASK) >> XSDFEC_CORE_AXIS_ENABLE_CTRL_LSB;
}

/*****************************************************************************/
/**
* @brief Writes the DIN field of the CORE_AXIS_ENABLE register.
*
* @param[in]	BaseAddress Base address of the device.
* @param[in]	Data Value to write to the field.
*
******************************************************************************/
void XSdFecSet_CORE_AXIS_ENABLE_DIN(UINTPTR BaseAddress, u32 Data) {
  u32 rdata = XSdFecReadReg(BaseAddress, XSDFEC_CORE_AXIS_ENABLE_ADDR);
  rdata = rdata & ~XSDFEC_CORE_AXIS_ENABLE_DIN_MASK;
  u32 wdata = (rdata & ~XSDFEC_CORE_AXIS_ENABLE_DIN_MASK) | (XSDFEC_CORE_AXIS_ENABLE_DIN_MASK & (Data << XSDFEC_CORE_AXIS_ENABLE_DIN_LSB));
  XSdFecWriteReg(BaseAddress, XSDFEC_CORE_AXIS_ENABLE_ADDR, wdata);
}

/*****************************************************************************/
/**
* @brief Reads the DIN field of the CORE_AXIS_ENABLE register.
*
* @param[in]	BaseAddress Base address of the device.
*
* @return The current DIN field value.
*
******************************************************************************/
u32 XSdFecGet_CORE_AXIS_ENABLE_DIN(UINTPTR BaseAddress) {
  u32 rdata = XSdFecReadReg(BaseAddress, XSDFEC_CORE_AXIS_ENABLE_ADDR);
  return (rdata & XSDFEC_CORE_AXIS_ENABLE_DIN_MASK) >> XSDFEC_CORE_AXIS_ENABLE_DIN_LSB;
}

/*****************************************************************************/
/**
* @brief Writes the DIN_WORDS field of the CORE_AXIS_ENABLE register.
*
* @param[in]	BaseAddress Base address of the device.
* @param[in]	Data Value to write to the field.
*
******************************************************************************/
void XSdFecSet_CORE_AXIS_ENABLE_DIN_WORDS(UINTPTR BaseAddress, u32 Data) {
  u32 rdata = XSdFecReadReg(BaseAddress, XSDFEC_CORE_AXIS_ENABLE_ADDR);
  rdata = rdata & ~XSDFEC_CORE_AXIS_ENABLE_DIN_WORDS_MASK;
  u32 wdata = (rdata & ~XSDFEC_CORE_AXIS_ENABLE_DIN_WORDS_MASK) | (XSDFEC_CORE_AXIS_ENABLE_DIN_WORDS_MASK & (Data << XSDFEC_CORE_AXIS_ENABLE_DIN_WORDS_LSB));
  XSdFecWriteReg(BaseAddress, XSDFEC_CORE_AXIS_ENABLE_ADDR, wdata);
}

/*****************************************************************************/
/**
* @brief Reads the DIN_WORDS field of the CORE_AXIS_ENABLE register.
*
* @param[in]	BaseAddress Base address of the device.
*
* @return The current DIN_WORDS field value.
*
******************************************************************************/
u32 XSdFecGet_CORE_AXIS_ENABLE_DIN_WORDS(UINTPTR BaseAddress) {
  u32 rdata = XSdFecReadReg(BaseAddress, XSDFEC_CORE_AXIS_ENABLE_ADDR);
  return (rdata & XSDFEC_CORE_AXIS_ENABLE_DIN_WORDS_MASK) >> XSDFEC_CORE_AXIS_ENABLE_DIN_WORDS_LSB;
}

/*****************************************************************************/
/**
* @brief Writes the STATUS field of the CORE_AXIS_ENABLE register.
*
* @param[in]	BaseAddress Base address of the device.
* @param[in]	Data Value to write to the field.
*
******************************************************************************/
void XSdFecSet_CORE_AXIS_ENABLE_STATUS(UINTPTR BaseAddress, u32 Data) {
  u32 rdata = XSdFecReadReg(BaseAddress, XSDFEC_CORE_AXIS_ENABLE_ADDR);
  rdata = rdata & ~XSDFEC_CORE_AXIS_ENABLE_STATUS_MASK;
  u32 wdata = (rdata & ~XSDFEC_CORE_AXIS_ENABLE_STATUS_MASK) | (XSDFEC_CORE_AXIS_ENABLE_STATUS_MASK & (Data << XSDFEC_CORE_AXIS_ENABLE_STATUS_LSB));
  XSdFecWriteReg(BaseAddress, XSDFEC_CORE_AXIS_ENABLE_ADDR, wdata);
}

/*****************************************************************************/
/**
* @brief Reads the STATUS field of the CORE_AXIS_ENABLE register.
*
* @param[in]	BaseAddress Base address of the device.
*
* @return The current STATUS field value.
*
******************************************************************************/
u32 XSdFecGet_CORE_AXIS_ENABLE_STATUS(UINTPTR BaseAddress) {
  u32 rdata = XSdFecReadReg(BaseAddress, XSDFEC_CORE_AXIS_ENABLE_ADDR);
  return (rdata & XSDFEC_CORE_AXIS_ENABLE_STATUS_MASK) >> XSDFEC_CORE_AXIS_ENABLE_STATUS_LSB;
}

/*****************************************************************************/
/**
* @brief Writes the DOUT field of the CORE_AXIS_ENABLE register.
*
* @param[in]	BaseAddress Base address of the device.
* @param[in]	Data Value to write to the field.
*
******************************************************************************/
void XSdFecSet_CORE_AXIS_ENABLE_DOUT(UINTPTR BaseAddress, u32 Data) {
  u32 rdata = XSdFecReadReg(BaseAddress, XSDFEC_CORE_AXIS_ENABLE_ADDR);
  rdata = rdata & ~XSDFEC_CORE_AXIS_ENABLE_DOUT_MASK;
  u32 wdata = (rdata & ~XSDFEC_CORE_AXIS_ENABLE_DOUT_MASK) | (XSDFEC_CORE_AXIS_ENABLE_DOUT_MASK & (Data << XSDFEC_CORE_AXIS_ENABLE_DOUT_LSB));
  XSdFecWriteReg(BaseAddress, XSDFEC_CORE_AXIS_ENABLE_ADDR, wdata);
}

/*****************************************************************************/
/**
* @brief Reads the DOUT field of the CORE_AXIS_ENABLE register.
*
* @param[in]	BaseAddress Base address of the device.
*
* @return The current DOUT field value.
*
******************************************************************************/
u32 XSdFecGet_CORE_AXIS_ENABLE_DOUT(UINTPTR BaseAddress) {
  u32 rdata = XSdFecReadReg(BaseAddress, XSDFEC_CORE_AXIS_ENABLE_ADDR);
  return (rdata & XSDFEC_CORE_AXIS_ENABLE_DOUT_MASK) >> XSDFEC_CORE_AXIS_ENABLE_DOUT_LSB;
}

/*****************************************************************************/
/**
* @brief Writes the DOUT_WORDS field of the CORE_AXIS_ENABLE register.
*
* @param[in]	BaseAddress Base address of the device.
* @param[in]	Data Value to write to the field.
*
******************************************************************************/
void XSdFecSet_CORE_AXIS_ENABLE_DOUT_WORDS(UINTPTR BaseAddress, u32 Data) {
  u32 rdata = XSdFecReadReg(BaseAddress, XSDFEC_CORE_AXIS_ENABLE_ADDR);
  rdata = rdata & ~XSDFEC_CORE_AXIS_ENABLE_DOUT_WORDS_MASK;
  u32 wdata = (rdata & ~XSDFEC_CORE_AXIS_ENABLE_DOUT_WORDS_MASK) | (XSDFEC_CORE_AXIS_ENABLE_DOUT_WORDS_MASK & (Data << XSDFEC_CORE_AXIS_ENABLE_DOUT_WORDS_LSB));
  XSdFecWriteReg(BaseAddress, XSDFEC_CORE_AXIS_ENABLE_ADDR, wdata);
}

/*****************************************************************************/
/**
* @brief Reads the DOUT_WORDS field of the CORE_AXIS_ENABLE register.
*
* @param[in]	BaseAddress Base address of the device.
*
* @return The current DOUT_WORDS field value.
*
******************************************************************************/
u32 XSdFecGet_CORE_AXIS_ENABLE_DOUT_WORDS(UINTPTR BaseAddress) {
  u32 rdata = XSdFecReadReg(BaseAddress, XSDFEC_CORE_AXIS_ENABLE_ADDR);
  return (rdata & XSDFEC_CORE_AXIS_ENABLE_DOUT_WORDS_MASK) >> XSDFEC_CORE_AXIS_ENABLE_DOUT_WORDS_LSB;
}

/*****************************************************************************/
/**
* @brief Writes the CORE_AXIS_ENABLE register.
*
* @param[in]	BaseAddress Base address of the device.
* @param[in]	Data Value to write to the register.
*
******************************************************************************/
void XSdFecSet_CORE_AXIS_ENABLE(UINTPTR BaseAddress, u32 Data) {
  XSdFecWriteReg(BaseAddress, XSDFEC_CORE_AXIS_ENABLE_ADDR, Data);
}

/*****************************************************************************/
/**
* @brief Reads the CORE_AXIS_ENABLE register.
*
* @param[in]	BaseAddress Base address of the device.
*
* @return The current register value.
*
******************************************************************************/
u32 XSdFecGet_CORE_AXIS_ENABLE(UINTPTR BaseAddress) {
  return XSdFecReadReg(BaseAddress, XSDFEC_CORE_AXIS_ENABLE_ADDR);
}

/*****************************************************************************/
/**
* @brief Writes the CORE_ORDER register.
*
* @param[in]	BaseAddress Base address of the device.
* @param[in]	Data Value to write to the register.
*
******************************************************************************/
void XSdFecSet_CORE_ORDER(UINTPTR BaseAddress, u32 Data) {
  XSdFecWriteReg(BaseAddress, XSDFEC_CORE_ORDER_ADDR, Data);
}

/*****************************************************************************/
/**
* @brief Reads the CORE_ORDER register.
*
* @param[in]	BaseAddress Base address of the device.
*
* @return The current register value.
*
******************************************************************************/
u32 XSdFecGet_CORE_ORDER(UINTPTR BaseAddress) {
  return XSdFecReadReg(BaseAddress, XSDFEC_CORE_ORDER_ADDR);
}

/*****************************************************************************/
/**
* @brief Writes the CORE_ISR register.
*
* @param[in]	BaseAddress Base address of the device.
* @param[in]	Data Value to write to the register.
*
******************************************************************************/
void XSdFecSet_CORE_ISR(UINTPTR BaseAddress, u32 Data) {
  XSdFecWriteReg(BaseAddress, XSDFEC_CORE_ISR_ADDR, Data);
}

/*****************************************************************************/
/**
* @brief Reads the CORE_ISR register.
*
* @param[in]	BaseAddress Base address of the device.
*
* @return The current register value.
*
******************************************************************************/
u32 XSdFecGet_CORE_ISR(UINTPTR BaseAddress) {
  return XSdFecReadReg(BaseAddress, XSDFEC_CORE_ISR_ADDR);
}

/*****************************************************************************/
/**
* @brief Writes the CORE_IER register.
*
* @param[in]	BaseAddress Base address of the device.
* @param[in]	Data Value to write to the register.
*
******************************************************************************/
void XSdFecSet_CORE_IER(UINTPTR BaseAddress, u32 Data) {
  XSdFecWriteReg(BaseAddress, XSDFEC_CORE_IER_ADDR, Data);
}

/*****************************************************************************/
/**
* @brief Writes the CORE_IDR register.
*
* @param[in]	BaseAddress Base address of the device.
* @param[in]	Data Value to write to the register.
*
******************************************************************************/
void XSdFecSet_CORE_IDR(UINTPTR BaseAddress, u32 Data) {
  XSdFecWriteReg(BaseAddress, XSDFEC_CORE_IDR_ADDR, Data);
}

/*****************************************************************************/
/**
* @brief Reads the CORE_IMR register.
*
* @param[in]	BaseAddress Base address of the device.
*
* @return The current register value.
*
******************************************************************************/
u32 XSdFecGet_CORE_IMR(UINTPTR BaseAddress) {
  return XSdFecReadReg(BaseAddress, XSDFEC_CORE_IMR_ADDR);
}

/*****************************************************************************/
/**
* @brief Writes the CORE_ECC_ISR register.
*
* @param[in]	BaseAddress Base address of the device.
* @param[in]	Data Value to write to the register.
*
******************************************************************************/
void XSdFecSet_CORE_ECC_ISR(UINTPTR BaseAddress, u32 Data) {
  XSdFecWriteReg(BaseAddress, XSDFEC_CORE_ECC_ISR_ADDR, Data);
}

/*****************************************************************************/
/**
* @brief Reads the CORE_ECC_ISR register.
*
* @param[in]	BaseAddress Base address of the device.
*
* @return The current register value.
*
******************************************************************************/
u32 XSdFecGet_CORE_ECC_ISR(UINTPTR BaseAddress) {
  return XSdFecReadReg(BaseAddress, XSDFEC_CORE_ECC_ISR_ADDR);
}

/*****************************************************************************/
/**
* @brief Writes the CORE_ECC_IER register.
*
* @param[in]	BaseAddress Base address of the device.
* @param[in]	Data Value to write to the register.
*
******************************************************************************/
void XSdFecSet_CORE_ECC_IER(UINTPTR BaseAddress, u32 Data) {
  XSdFecWriteReg(BaseAddress, XSDFEC_CORE_ECC_IER_ADDR, Data);
}

/*****************************************************************************/
/**
* @brief Writes the CORE_ECC_IDR register.
*
* @param[in]	BaseAddress Base address of the device.
* @param[in]	Data Value to write to the register.
*
******************************************************************************/
void XSdFecSet_CORE_ECC_IDR(UINTPTR BaseAddress, u32 Data) {
  XSdFecWriteReg(BaseAddress, XSDFEC_CORE_ECC_IDR_ADDR, Data);
}

/*****************************************************************************/
/**
* @brief Reads the CORE_ECC_IMR register.
*
* @param[in]	BaseAddress Base address of the device.
*
* @return The current register value.
*
******************************************************************************/
u32 XSdFecGet_CORE_ECC_IMR(UINTPTR BaseAddress) {
  return XSdFecReadReg(BaseAddress, XSDFEC_CORE_ECC_IMR_ADDR);
}

/*****************************************************************************/
/**
* @brief Writes the CORE_BYPASS register.
*
* @param[in]	BaseAddress Base address of the device.
* @param[in]	Data Value to write to the register.
*
******************************************************************************/
void XSdFecSet_CORE_BYPASS(UINTPTR BaseAddress, u32 Data) {
  XSdFecWriteReg(BaseAddress, XSDFEC_CORE_BYPASS_ADDR, Data);
}

/*****************************************************************************/
/**
* @brief Reads the CORE_BYPASS register.
*
* @param[in]	BaseAddress Base address of the device.
*
* @return The current register value.
*
******************************************************************************/
u32 XSdFecGet_CORE_BYPASS(UINTPTR BaseAddress) {
  return XSdFecReadReg(BaseAddress, XSDFEC_CORE_BYPASS_ADDR);
}

/*****************************************************************************/
/**
* @brief Reads the CORE_VERSION register.
*
* @param[in]	BaseAddress Base address of the device.
*
* @return The current register value.
*
******************************************************************************/
u32 XSdFecGet_CORE_VERSION(UINTPTR BaseAddress) {
  return XSdFecReadReg(BaseAddress, XSDFEC_CORE_VERSION_ADDR);
}

/*****************************************************************************/
/**
* @brief Writes the ALG field of the TURBO register.
*
* @param[in]	BaseAddress Base address of the device.
* @param[in]	Data Value to write to the field.
*
******************************************************************************/
void XSdFecSet_TURBO_ALG(UINTPTR BaseAddress, u32 Data) {
  u32 rdata = XSdFecReadReg(BaseAddress, XSDFEC_TURBO_ADDR);
  rdata = rdata & ~XSDFEC_TURBO_ALG_MASK;
  u32 wdata = (rdata & ~XSDFEC_TURBO_ALG_MASK) | (XSDFEC_TURBO_ALG_MASK & (Data << XSDFEC_TURBO_ALG_LSB));
  XSdFecWriteReg(BaseAddress, XSDFEC_TURBO_ADDR, wdata);
}

/*****************************************************************************/
/**
* @brief Reads the ALG field of the TURBO register.
*
* @param[in]	BaseAddress Base address of the device.
*
* @return The current ALG field value.
*
******************************************************************************/
u32 XSdFecGet_TURBO_ALG(UINTPTR BaseAddress) {
  u32 rdata = XSdFecReadReg(BaseAddress, XSDFEC_TURBO_ADDR);
  return (rdata & XSDFEC_TURBO_ALG_MASK) >> XSDFEC_TURBO_ALG_LSB;
}

/*****************************************************************************/
/**
* @brief Writes the SCALE_FACTOR field of the TURBO register.
*
* @param[in]	BaseAddress Base address of the device.
* @param[in]	Data Value to write to the field.
*
******************************************************************************/
void XSdFecSet_TURBO_SCALE_FACTOR(UINTPTR BaseAddress, u32 Data) {
  u32 rdata = XSdFecReadReg(BaseAddress, XSDFEC_TURBO_ADDR);
  rdata = rdata & ~XSDFEC_TURBO_SCALE_FACTOR_MASK;
  u32 wdata = (rdata & ~XSDFEC_TURBO_SCALE_FACTOR_MASK) | (XSDFEC_TURBO_SCALE_FACTOR_MASK & (Data << XSDFEC_TURBO_SCALE_FACTOR_LSB));
  XSdFecWriteReg(BaseAddress, XSDFEC_TURBO_ADDR, wdata);
}

/*****************************************************************************/
/**
* @brief Reads the SCALE_FACTOR field of the TURBO register.
*
* @param[in]	BaseAddress Base address of the device.
*
* @return The current SCALE_FACTOR field value.
*
******************************************************************************/
u32 XSdFecGet_TURBO_SCALE_FACTOR(UINTPTR BaseAddress) {
  u32 rdata = XSdFecReadReg(BaseAddress, XSDFEC_TURBO_ADDR);
  return (rdata & XSDFEC_TURBO_SCALE_FACTOR_MASK) >> XSDFEC_TURBO_SCALE_FACTOR_LSB;
}

/*****************************************************************************/
/**
* @brief Writes the TURBO register.
*
* @param[in]	BaseAddress Base address of the device.
* @param[in]	Data Value to write to the register.
*
******************************************************************************/
void XSdFecSet_TURBO(UINTPTR BaseAddress, u32 Data) {
  XSdFecWriteReg(BaseAddress, XSDFEC_TURBO_ADDR, Data);
}

/*****************************************************************************/
/**
* @brief Reads the TURBO register.
*
* @param[in]	BaseAddress Base address of the device.
*
* @return The current register value.
*
******************************************************************************/
u32 XSdFecGet_TURBO(UINTPTR BaseAddress) {
  return XSdFecReadReg(BaseAddress, XSDFEC_TURBO_ADDR);
}

/*****************************************************************************/
/**
* @brief Writes N field words of the LDPC_CODE_REG0 register.
*
* @param[in]	BaseAddress Base address of the device.
* @param[in]	WordOffset Index of the first register word to access.
* @param[in]	DataArrayPtr Pointer to the array of values to write.
* @param[in]	NumData Number of words to transfer.
*
* @return
*        - NumData if the transfer completed.
*        - 0 if WordOffset plus NumData exceeds the register depth.
*
******************************************************************************/
u32 XSdFecWrite_LDPC_CODE_REG0_N_Words(UINTPTR BaseAddress, u32 WordOffset, const u32 *DataArrayPtr, u32 NumData) {
  u32 MaxDataDepth = WordOffset + NumData;
  if ( MaxDataDepth > XSDFEC_LDPC_CODE_REG0_DEPTH) {
    return 0;
  }
  u32 idx;
  for(idx = 0; idx < NumData; idx++) {
    u32 addr =  XSDFEC_LDPC_CODE_REG0_ADDR_BASE+(WordOffset + idx)*XSDFEC_LDPC_CODE_REG0_STEP;
    u32 rdata = XSdFecReadReg(BaseAddress, addr);
    u32 wdata = (rdata & ~XSDFEC_LDPC_CODE_REG0_N_MASK) | (XSDFEC_LDPC_CODE_REG0_N_MASK & (DataArrayPtr[idx] << XSDFEC_LDPC_CODE_REG0_N_LSB));
    XSdFecWriteReg(BaseAddress, addr, wdata);
  }
  return NumData;
}

/*****************************************************************************/
/**
* @brief Reads N field words of the LDPC_CODE_REG0 register.
*
* @param[in]	BaseAddress Base address of the device.
* @param[in]	WordOffset Index of the first register word to access.
* @param[out]	DataArrayPtr Pointer to the array that receives the values read.
* @param[in]	NumData Number of words to transfer.
*
* @return
*        - NumData if the transfer completed.
*        - 0 if WordOffset plus NumData exceeds the register depth.
*
******************************************************************************/
u32 XSdFecRead_LDPC_CODE_REG0_N_Words(UINTPTR BaseAddress, u32 WordOffset, u32 *DataArrayPtr, u32 NumData) {
  u32 MaxDataDepth = WordOffset + NumData;
  if (MaxDataDepth > XSDFEC_LDPC_CODE_REG0_DEPTH) {
    return 0;
  }
  u32 idx;
  for(idx = 0; idx < NumData; idx++) {
    u32 addr =  XSDFEC_LDPC_CODE_REG0_ADDR_BASE+(WordOffset + idx)*XSDFEC_LDPC_CODE_REG0_STEP;
    u32 rdata = XSdFecReadReg(BaseAddress, addr);
    DataArrayPtr[idx] = (rdata & XSDFEC_LDPC_CODE_REG0_N_MASK) >> XSDFEC_LDPC_CODE_REG0_N_LSB;
  }
  return NumData;
}

/*****************************************************************************/
/**
* @brief Writes K field words of the LDPC_CODE_REG0 register.
*
* @param[in]	BaseAddress Base address of the device.
* @param[in]	WordOffset Index of the first register word to access.
* @param[in]	DataArrayPtr Pointer to the array of values to write.
* @param[in]	NumData Number of words to transfer.
*
* @return
*        - NumData if the transfer completed.
*        - 0 if WordOffset plus NumData exceeds the register depth.
*
******************************************************************************/
u32 XSdFecWrite_LDPC_CODE_REG0_K_Words(UINTPTR BaseAddress, u32 WordOffset, const u32 *DataArrayPtr, u32 NumData) {
  u32 MaxDataDepth = WordOffset + NumData;
  if ( MaxDataDepth > XSDFEC_LDPC_CODE_REG0_DEPTH) {
    return 0;
  }
  u32 idx;
  for(idx = 0; idx < NumData; idx++) {
    u32 addr =  XSDFEC_LDPC_CODE_REG0_ADDR_BASE+(WordOffset + idx)*XSDFEC_LDPC_CODE_REG0_STEP;
    u32 rdata = XSdFecReadReg(BaseAddress, addr);
    u32 wdata = (rdata & ~XSDFEC_LDPC_CODE_REG0_K_MASK) | (XSDFEC_LDPC_CODE_REG0_K_MASK & (DataArrayPtr[idx] << XSDFEC_LDPC_CODE_REG0_K_LSB));
    XSdFecWriteReg(BaseAddress, addr, wdata);
  }
  return NumData;
}

/*****************************************************************************/
/**
* @brief Reads K field words of the LDPC_CODE_REG0 register.
*
* @param[in]	BaseAddress Base address of the device.
* @param[in]	WordOffset Index of the first register word to access.
* @param[out]	DataArrayPtr Pointer to the array that receives the values read.
* @param[in]	NumData Number of words to transfer.
*
* @return
*        - NumData if the transfer completed.
*        - 0 if WordOffset plus NumData exceeds the register depth.
*
******************************************************************************/
u32 XSdFecRead_LDPC_CODE_REG0_K_Words(UINTPTR BaseAddress, u32 WordOffset, u32 *DataArrayPtr, u32 NumData) {
  u32 MaxDataDepth = WordOffset + NumData;
  if (MaxDataDepth > XSDFEC_LDPC_CODE_REG0_DEPTH) {
    return 0;
  }
  u32 idx;
  for(idx = 0; idx < NumData; idx++) {
    u32 addr =  XSDFEC_LDPC_CODE_REG0_ADDR_BASE+(WordOffset + idx)*XSDFEC_LDPC_CODE_REG0_STEP;
    u32 rdata = XSdFecReadReg(BaseAddress, addr);
    DataArrayPtr[idx] = (rdata & XSDFEC_LDPC_CODE_REG0_K_MASK) >> XSDFEC_LDPC_CODE_REG0_K_LSB;
  }
  return NumData;
}

/*****************************************************************************/
/**
* @brief Writes words of the LDPC_CODE_REG0 register.
*
* @param[in]	BaseAddress Base address of the device.
* @param[in]	WordOffset Index of the first register word to access.
* @param[in]	DataArrayPtr Pointer to the array of values to write.
* @param[in]	NumData Number of words to transfer.
*
* @return
*        - NumData if the transfer completed.
*        - 0 if WordOffset plus NumData exceeds the register depth.
*
******************************************************************************/
u32 XSdFecWrite_LDPC_CODE_REG0_Words(UINTPTR BaseAddress, u32 WordOffset, const u32 *DataArrayPtr, u32 NumData) {
  u32 MaxDataDepth = (WordOffset + NumData);
  if (MaxDataDepth > XSDFEC_LDPC_CODE_REG0_DEPTH) {
    return 0;
  }
  u32 idx;
  for(idx = 0; idx < NumData; idx++) {
    u32 addr =  XSDFEC_LDPC_CODE_REG0_ADDR_BASE+(WordOffset + idx)*XSDFEC_LDPC_CODE_REG0_STEP;
    XSdFecWriteReg(BaseAddress, addr, DataArrayPtr[idx]);
  }
  return NumData;
}

/*****************************************************************************/
/**
* @brief Reads words of the LDPC_CODE_REG0 register.
*
* @param[in]	BaseAddress Base address of the device.
* @param[in]	WordOffset Index of the first register word to access.
* @param[out]	DataArrayPtr Pointer to the array that receives the values read.
* @param[in]	NumData Number of words to transfer.
*
* @return
*        - NumData if the transfer completed.
*        - 0 if WordOffset plus NumData exceeds the register depth.
*
******************************************************************************/
u32 XSdFecRead_LDPC_CODE_REG0_Words(UINTPTR BaseAddress, u32 WordOffset, u32 *DataArrayPtr, u32 NumData) {
  u32 MaxDataDepth = (WordOffset + NumData);
  if (MaxDataDepth > XSDFEC_LDPC_CODE_REG0_DEPTH) {
    return 0;
  }
  u32 idx;
  for(idx = 0; idx < NumData; idx++) {
    u32 addr =  XSDFEC_LDPC_CODE_REG0_ADDR_BASE+(WordOffset + idx)*XSDFEC_LDPC_CODE_REG0_STEP;
    DataArrayPtr[idx] = XSdFecReadReg(BaseAddress, addr);
  }
  return NumData;
}

/*****************************************************************************/
/**
* @brief Writes PSIZE field words of the LDPC_CODE_REG1 register.
*
* @param[in]	BaseAddress Base address of the device.
* @param[in]	WordOffset Index of the first register word to access.
* @param[in]	DataArrayPtr Pointer to the array of values to write.
* @param[in]	NumData Number of words to transfer.
*
* @return
*        - NumData if the transfer completed.
*        - 0 if WordOffset plus NumData exceeds the register depth.
*
******************************************************************************/
u32 XSdFecWrite_LDPC_CODE_REG1_PSIZE_Words(UINTPTR BaseAddress, u32 WordOffset, const u32 *DataArrayPtr, u32 NumData) {
  u32 MaxDataDepth = WordOffset + NumData;
  if ( MaxDataDepth > XSDFEC_LDPC_CODE_REG1_DEPTH) {
    return 0;
  }
  u32 idx;
  for(idx = 0; idx < NumData; idx++) {
    u32 addr =  XSDFEC_LDPC_CODE_REG1_ADDR_BASE+(WordOffset + idx)*XSDFEC_LDPC_CODE_REG1_STEP;
    u32 rdata = XSdFecReadReg(BaseAddress, addr);
    u32 wdata = (rdata & ~XSDFEC_LDPC_CODE_REG1_PSIZE_MASK) | (XSDFEC_LDPC_CODE_REG1_PSIZE_MASK & (DataArrayPtr[idx] << XSDFEC_LDPC_CODE_REG1_PSIZE_LSB));
    XSdFecWriteReg(BaseAddress, addr, wdata);
  }
  return NumData;
}

/*****************************************************************************/
/**
* @brief Reads PSIZE field words of the LDPC_CODE_REG1 register.
*
* @param[in]	BaseAddress Base address of the device.
* @param[in]	WordOffset Index of the first register word to access.
* @param[out]	DataArrayPtr Pointer to the array that receives the values read.
* @param[in]	NumData Number of words to transfer.
*
* @return
*        - NumData if the transfer completed.
*        - 0 if WordOffset plus NumData exceeds the register depth.
*
******************************************************************************/
u32 XSdFecRead_LDPC_CODE_REG1_PSIZE_Words(UINTPTR BaseAddress, u32 WordOffset, u32 *DataArrayPtr, u32 NumData) {
  u32 MaxDataDepth = WordOffset + NumData;
  if (MaxDataDepth > XSDFEC_LDPC_CODE_REG1_DEPTH) {
    return 0;
  }
  u32 idx;
  for(idx = 0; idx < NumData; idx++) {
    u32 addr =  XSDFEC_LDPC_CODE_REG1_ADDR_BASE+(WordOffset + idx)*XSDFEC_LDPC_CODE_REG1_STEP;
    u32 rdata = XSdFecReadReg(BaseAddress, addr);
    DataArrayPtr[idx] = (rdata & XSDFEC_LDPC_CODE_REG1_PSIZE_MASK) >> XSDFEC_LDPC_CODE_REG1_PSIZE_LSB;
  }
  return NumData;
}

/*****************************************************************************/
/**
* @brief Writes NO_PACKING field words of the LDPC_CODE_REG1 register.
*
* @param[in]	BaseAddress Base address of the device.
* @param[in]	WordOffset Index of the first register word to access.
* @param[in]	DataArrayPtr Pointer to the array of values to write.
* @param[in]	NumData Number of words to transfer.
*
* @return
*        - NumData if the transfer completed.
*        - 0 if WordOffset plus NumData exceeds the register depth.
*
******************************************************************************/
u32 XSdFecWrite_LDPC_CODE_REG1_NO_PACKING_Words(UINTPTR BaseAddress, u32 WordOffset, const u32 *DataArrayPtr, u32 NumData) {
  u32 MaxDataDepth = WordOffset + NumData;
  if ( MaxDataDepth > XSDFEC_LDPC_CODE_REG1_DEPTH) {
    return 0;
  }
  u32 idx;
  for(idx = 0; idx < NumData; idx++) {
    u32 addr =  XSDFEC_LDPC_CODE_REG1_ADDR_BASE+(WordOffset + idx)*XSDFEC_LDPC_CODE_REG1_STEP;
    u32 rdata = XSdFecReadReg(BaseAddress, addr);
    u32 wdata = (rdata & ~XSDFEC_LDPC_CODE_REG1_NO_PACKING_MASK) | (XSDFEC_LDPC_CODE_REG1_NO_PACKING_MASK & (DataArrayPtr[idx] << XSDFEC_LDPC_CODE_REG1_NO_PACKING_LSB));
    XSdFecWriteReg(BaseAddress, addr, wdata);
  }
  return NumData;
}

/*****************************************************************************/
/**
* @brief Reads NO_PACKING field words of the LDPC_CODE_REG1 register.
*
* @param[in]	BaseAddress Base address of the device.
* @param[in]	WordOffset Index of the first register word to access.
* @param[out]	DataArrayPtr Pointer to the array that receives the values read.
* @param[in]	NumData Number of words to transfer.
*
* @return
*        - NumData if the transfer completed.
*        - 0 if WordOffset plus NumData exceeds the register depth.
*
******************************************************************************/
u32 XSdFecRead_LDPC_CODE_REG1_NO_PACKING_Words(UINTPTR BaseAddress, u32 WordOffset, u32 *DataArrayPtr, u32 NumData) {
  u32 MaxDataDepth = WordOffset + NumData;
  if (MaxDataDepth > XSDFEC_LDPC_CODE_REG1_DEPTH) {
    return 0;
  }
  u32 idx;
  for(idx = 0; idx < NumData; idx++) {
    u32 addr =  XSDFEC_LDPC_CODE_REG1_ADDR_BASE+(WordOffset + idx)*XSDFEC_LDPC_CODE_REG1_STEP;
    u32 rdata = XSdFecReadReg(BaseAddress, addr);
    DataArrayPtr[idx] = (rdata & XSDFEC_LDPC_CODE_REG1_NO_PACKING_MASK) >> XSDFEC_LDPC_CODE_REG1_NO_PACKING_LSB;
  }
  return NumData;
}

/*****************************************************************************/
/**
* @brief Writes NM field words of the LDPC_CODE_REG1 register.
*
* @param[in]	BaseAddress Base address of the device.
* @param[in]	WordOffset Index of the first register word to access.
* @param[in]	DataArrayPtr Pointer to the array of values to write.
* @param[in]	NumData Number of words to transfer.
*
* @return
*        - NumData if the transfer completed.
*        - 0 if WordOffset plus NumData exceeds the register depth.
*
******************************************************************************/
u32 XSdFecWrite_LDPC_CODE_REG1_NM_Words(UINTPTR BaseAddress, u32 WordOffset, const u32 *DataArrayPtr, u32 NumData) {
  u32 MaxDataDepth = WordOffset + NumData;
  if ( MaxDataDepth > XSDFEC_LDPC_CODE_REG1_DEPTH) {
    return 0;
  }
  u32 idx;
  for(idx = 0; idx < NumData; idx++) {
    u32 addr =  XSDFEC_LDPC_CODE_REG1_ADDR_BASE+(WordOffset + idx)*XSDFEC_LDPC_CODE_REG1_STEP;
    u32 rdata = XSdFecReadReg(BaseAddress, addr);
    u32 wdata = (rdata & ~XSDFEC_LDPC_CODE_REG1_NM_MASK) | (XSDFEC_LDPC_CODE_REG1_NM_MASK & (DataArrayPtr[idx] << XSDFEC_LDPC_CODE_REG1_NM_LSB));
    XSdFecWriteReg(BaseAddress, addr, wdata);
  }
  return NumData;
}

/*****************************************************************************/
/**
* @brief Reads NM field words of the LDPC_CODE_REG1 register.
*
* @param[in]	BaseAddress Base address of the device.
* @param[in]	WordOffset Index of the first register word to access.
* @param[out]	DataArrayPtr Pointer to the array that receives the values read.
* @param[in]	NumData Number of words to transfer.
*
* @return
*        - NumData if the transfer completed.
*        - 0 if WordOffset plus NumData exceeds the register depth.
*
******************************************************************************/
u32 XSdFecRead_LDPC_CODE_REG1_NM_Words(UINTPTR BaseAddress, u32 WordOffset, u32 *DataArrayPtr, u32 NumData) {
  u32 MaxDataDepth = WordOffset + NumData;
  if (MaxDataDepth > XSDFEC_LDPC_CODE_REG1_DEPTH) {
    return 0;
  }
  u32 idx;
  for(idx = 0; idx < NumData; idx++) {
    u32 addr =  XSDFEC_LDPC_CODE_REG1_ADDR_BASE+(WordOffset + idx)*XSDFEC_LDPC_CODE_REG1_STEP;
    u32 rdata = XSdFecReadReg(BaseAddress, addr);
    DataArrayPtr[idx] = (rdata & XSDFEC_LDPC_CODE_REG1_NM_MASK) >> XSDFEC_LDPC_CODE_REG1_NM_LSB;
  }
  return NumData;
}

/*****************************************************************************/
/**
* @brief Writes words of the LDPC_CODE_REG1 register.
*
* @param[in]	BaseAddress Base address of the device.
* @param[in]	WordOffset Index of the first register word to access.
* @param[in]	DataArrayPtr Pointer to the array of values to write.
* @param[in]	NumData Number of words to transfer.
*
* @return
*        - NumData if the transfer completed.
*        - 0 if WordOffset plus NumData exceeds the register depth.
*
******************************************************************************/
u32 XSdFecWrite_LDPC_CODE_REG1_Words(UINTPTR BaseAddress, u32 WordOffset, const u32 *DataArrayPtr, u32 NumData) {
  u32 MaxDataDepth = (WordOffset + NumData);
  if (MaxDataDepth > XSDFEC_LDPC_CODE_REG1_DEPTH) {
    return 0;
  }
  u32 idx;
  for(idx = 0; idx < NumData; idx++) {
    u32 addr =  XSDFEC_LDPC_CODE_REG1_ADDR_BASE+(WordOffset + idx)*XSDFEC_LDPC_CODE_REG1_STEP;
    XSdFecWriteReg(BaseAddress, addr, DataArrayPtr[idx]);
  }
  return NumData;
}

/*****************************************************************************/
/**
* @brief Reads words of the LDPC_CODE_REG1 register.
*
* @param[in]	BaseAddress Base address of the device.
* @param[in]	WordOffset Index of the first register word to access.
* @param[out]	DataArrayPtr Pointer to the array that receives the values read.
* @param[in]	NumData Number of words to transfer.
*
* @return
*        - NumData if the transfer completed.
*        - 0 if WordOffset plus NumData exceeds the register depth.
*
******************************************************************************/
u32 XSdFecRead_LDPC_CODE_REG1_Words(UINTPTR BaseAddress, u32 WordOffset, u32 *DataArrayPtr, u32 NumData) {
  u32 MaxDataDepth = (WordOffset + NumData);
  if (MaxDataDepth > XSDFEC_LDPC_CODE_REG1_DEPTH) {
    return 0;
  }
  u32 idx;
  for(idx = 0; idx < NumData; idx++) {
    u32 addr =  XSDFEC_LDPC_CODE_REG1_ADDR_BASE+(WordOffset + idx)*XSDFEC_LDPC_CODE_REG1_STEP;
    DataArrayPtr[idx] = XSdFecReadReg(BaseAddress, addr);
  }
  return NumData;
}

/*****************************************************************************/
/**
* @brief Writes NLAYERS field words of the LDPC_CODE_REG2 register.
*
* @param[in]	BaseAddress Base address of the device.
* @param[in]	WordOffset Index of the first register word to access.
* @param[in]	DataArrayPtr Pointer to the array of values to write.
* @param[in]	NumData Number of words to transfer.
*
* @return
*        - NumData if the transfer completed.
*        - 0 if WordOffset plus NumData exceeds the register depth.
*
******************************************************************************/
u32 XSdFecWrite_LDPC_CODE_REG2_NLAYERS_Words(UINTPTR BaseAddress, u32 WordOffset, const u32 *DataArrayPtr, u32 NumData) {
  u32 MaxDataDepth = WordOffset + NumData;
  if ( MaxDataDepth > XSDFEC_LDPC_CODE_REG2_DEPTH) {
    return 0;
  }
  u32 idx;
  for(idx = 0; idx < NumData; idx++) {
    u32 addr =  XSDFEC_LDPC_CODE_REG2_ADDR_BASE+(WordOffset + idx)*XSDFEC_LDPC_CODE_REG2_STEP;
    u32 rdata = XSdFecReadReg(BaseAddress, addr);
    u32 wdata = (rdata & ~XSDFEC_LDPC_CODE_REG2_NLAYERS_MASK) | (XSDFEC_LDPC_CODE_REG2_NLAYERS_MASK & (DataArrayPtr[idx] << XSDFEC_LDPC_CODE_REG2_NLAYERS_LSB));
    XSdFecWriteReg(BaseAddress, addr, wdata);
  }
  return NumData;
}

/*****************************************************************************/
/**
* @brief Reads NLAYERS field words of the LDPC_CODE_REG2 register.
*
* @param[in]	BaseAddress Base address of the device.
* @param[in]	WordOffset Index of the first register word to access.
* @param[out]	DataArrayPtr Pointer to the array that receives the values read.
* @param[in]	NumData Number of words to transfer.
*
* @return
*        - NumData if the transfer completed.
*        - 0 if WordOffset plus NumData exceeds the register depth.
*
******************************************************************************/
u32 XSdFecRead_LDPC_CODE_REG2_NLAYERS_Words(UINTPTR BaseAddress, u32 WordOffset, u32 *DataArrayPtr, u32 NumData) {
  u32 MaxDataDepth = WordOffset + NumData;
  if (MaxDataDepth > XSDFEC_LDPC_CODE_REG2_DEPTH) {
    return 0;
  }
  u32 idx;
  for(idx = 0; idx < NumData; idx++) {
    u32 addr =  XSDFEC_LDPC_CODE_REG2_ADDR_BASE+(WordOffset + idx)*XSDFEC_LDPC_CODE_REG2_STEP;
    u32 rdata = XSdFecReadReg(BaseAddress, addr);
    DataArrayPtr[idx] = (rdata & XSDFEC_LDPC_CODE_REG2_NLAYERS_MASK) >> XSDFEC_LDPC_CODE_REG2_NLAYERS_LSB;
  }
  return NumData;
}

/*****************************************************************************/
/**
* @brief Writes NMQC field words of the LDPC_CODE_REG2 register.
*
* @param[in]	BaseAddress Base address of the device.
* @param[in]	WordOffset Index of the first register word to access.
* @param[in]	DataArrayPtr Pointer to the array of values to write.
* @param[in]	NumData Number of words to transfer.
*
* @return
*        - NumData if the transfer completed.
*        - 0 if WordOffset plus NumData exceeds the register depth.
*
******************************************************************************/
u32 XSdFecWrite_LDPC_CODE_REG2_NMQC_Words(UINTPTR BaseAddress, u32 WordOffset, const u32 *DataArrayPtr, u32 NumData) {
  u32 MaxDataDepth = WordOffset + NumData;
  if ( MaxDataDepth > XSDFEC_LDPC_CODE_REG2_DEPTH) {
    return 0;
  }
  u32 idx;
  for(idx = 0; idx < NumData; idx++) {
    u32 addr =  XSDFEC_LDPC_CODE_REG2_ADDR_BASE+(WordOffset + idx)*XSDFEC_LDPC_CODE_REG2_STEP;
    u32 rdata = XSdFecReadReg(BaseAddress, addr);
    u32 wdata = (rdata & ~XSDFEC_LDPC_CODE_REG2_NMQC_MASK) | (XSDFEC_LDPC_CODE_REG2_NMQC_MASK & (DataArrayPtr[idx] << XSDFEC_LDPC_CODE_REG2_NMQC_LSB));
    XSdFecWriteReg(BaseAddress, addr, wdata);
  }
  return NumData;
}

/*****************************************************************************/
/**
* @brief Reads NMQC field words of the LDPC_CODE_REG2 register.
*
* @param[in]	BaseAddress Base address of the device.
* @param[in]	WordOffset Index of the first register word to access.
* @param[out]	DataArrayPtr Pointer to the array that receives the values read.
* @param[in]	NumData Number of words to transfer.
*
* @return
*        - NumData if the transfer completed.
*        - 0 if WordOffset plus NumData exceeds the register depth.
*
******************************************************************************/
u32 XSdFecRead_LDPC_CODE_REG2_NMQC_Words(UINTPTR BaseAddress, u32 WordOffset, u32 *DataArrayPtr, u32 NumData) {
  u32 MaxDataDepth = WordOffset + NumData;
  if (MaxDataDepth > XSDFEC_LDPC_CODE_REG2_DEPTH) {
    return 0;
  }
  u32 idx;
  for(idx = 0; idx < NumData; idx++) {
    u32 addr =  XSDFEC_LDPC_CODE_REG2_ADDR_BASE+(WordOffset + idx)*XSDFEC_LDPC_CODE_REG2_STEP;
    u32 rdata = XSdFecReadReg(BaseAddress, addr);
    DataArrayPtr[idx] = (rdata & XSDFEC_LDPC_CODE_REG2_NMQC_MASK) >> XSDFEC_LDPC_CODE_REG2_NMQC_LSB;
  }
  return NumData;
}

/*****************************************************************************/
/**
* @brief Writes NORM_TYPE field words of the LDPC_CODE_REG2 register.
*
* @param[in]	BaseAddress Base address of the device.
* @param[in]	WordOffset Index of the first register word to access.
* @param[in]	DataArrayPtr Pointer to the array of values to write.
* @param[in]	NumData Number of words to transfer.
*
* @return
*        - NumData if the transfer completed.
*        - 0 if WordOffset plus NumData exceeds the register depth.
*
******************************************************************************/
u32 XSdFecWrite_LDPC_CODE_REG2_NORM_TYPE_Words(UINTPTR BaseAddress, u32 WordOffset, const u32 *DataArrayPtr, u32 NumData) {
  u32 MaxDataDepth = WordOffset + NumData;
  if ( MaxDataDepth > XSDFEC_LDPC_CODE_REG2_DEPTH) {
    return 0;
  }
  u32 idx;
  for(idx = 0; idx < NumData; idx++) {
    u32 addr =  XSDFEC_LDPC_CODE_REG2_ADDR_BASE+(WordOffset + idx)*XSDFEC_LDPC_CODE_REG2_STEP;
    u32 rdata = XSdFecReadReg(BaseAddress, addr);
    u32 wdata = (rdata & ~XSDFEC_LDPC_CODE_REG2_NORM_TYPE_MASK) | (XSDFEC_LDPC_CODE_REG2_NORM_TYPE_MASK & (DataArrayPtr[idx] << XSDFEC_LDPC_CODE_REG2_NORM_TYPE_LSB));
    XSdFecWriteReg(BaseAddress, addr, wdata);
  }
  return NumData;
}

/*****************************************************************************/
/**
* @brief Reads NORM_TYPE field words of the LDPC_CODE_REG2 register.
*
* @param[in]	BaseAddress Base address of the device.
* @param[in]	WordOffset Index of the first register word to access.
* @param[out]	DataArrayPtr Pointer to the array that receives the values read.
* @param[in]	NumData Number of words to transfer.
*
* @return
*        - NumData if the transfer completed.
*        - 0 if WordOffset plus NumData exceeds the register depth.
*
******************************************************************************/
u32 XSdFecRead_LDPC_CODE_REG2_NORM_TYPE_Words(UINTPTR BaseAddress, u32 WordOffset, u32 *DataArrayPtr, u32 NumData) {
  u32 MaxDataDepth = WordOffset + NumData;
  if (MaxDataDepth > XSDFEC_LDPC_CODE_REG2_DEPTH) {
    return 0;
  }
  u32 idx;
  for(idx = 0; idx < NumData; idx++) {
    u32 addr =  XSDFEC_LDPC_CODE_REG2_ADDR_BASE+(WordOffset + idx)*XSDFEC_LDPC_CODE_REG2_STEP;
    u32 rdata = XSdFecReadReg(BaseAddress, addr);
    DataArrayPtr[idx] = (rdata & XSDFEC_LDPC_CODE_REG2_NORM_TYPE_MASK) >> XSDFEC_LDPC_CODE_REG2_NORM_TYPE_LSB;
  }
  return NumData;
}

/*****************************************************************************/
/**
* @brief Writes SPECIAL_QC field words of the LDPC_CODE_REG2 register.
*
* @param[in]	BaseAddress Base address of the device.
* @param[in]	WordOffset Index of the first register word to access.
* @param[in]	DataArrayPtr Pointer to the array of values to write.
* @param[in]	NumData Number of words to transfer.
*
* @return
*        - NumData if the transfer completed.
*        - 0 if WordOffset plus NumData exceeds the register depth.
*
******************************************************************************/
u32 XSdFecWrite_LDPC_CODE_REG2_SPECIAL_QC_Words(UINTPTR BaseAddress, u32 WordOffset, const u32 *DataArrayPtr, u32 NumData) {
  u32 MaxDataDepth = WordOffset + NumData;
  if ( MaxDataDepth > XSDFEC_LDPC_CODE_REG2_DEPTH) {
    return 0;
  }
  u32 idx;
  for(idx = 0; idx < NumData; idx++) {
    u32 addr =  XSDFEC_LDPC_CODE_REG2_ADDR_BASE+(WordOffset + idx)*XSDFEC_LDPC_CODE_REG2_STEP;
    u32 rdata = XSdFecReadReg(BaseAddress, addr);
    u32 wdata = (rdata & ~XSDFEC_LDPC_CODE_REG2_SPECIAL_QC_MASK) | (XSDFEC_LDPC_CODE_REG2_SPECIAL_QC_MASK & (DataArrayPtr[idx] << XSDFEC_LDPC_CODE_REG2_SPECIAL_QC_LSB));
    XSdFecWriteReg(BaseAddress, addr, wdata);
  }
  return NumData;
}

/*****************************************************************************/
/**
* @brief Reads SPECIAL_QC field words of the LDPC_CODE_REG2 register.
*
* @param[in]	BaseAddress Base address of the device.
* @param[in]	WordOffset Index of the first register word to access.
* @param[out]	DataArrayPtr Pointer to the array that receives the values read.
* @param[in]	NumData Number of words to transfer.
*
* @return
*        - NumData if the transfer completed.
*        - 0 if WordOffset plus NumData exceeds the register depth.
*
******************************************************************************/
u32 XSdFecRead_LDPC_CODE_REG2_SPECIAL_QC_Words(UINTPTR BaseAddress, u32 WordOffset, u32 *DataArrayPtr, u32 NumData) {
  u32 MaxDataDepth = WordOffset + NumData;
  if (MaxDataDepth > XSDFEC_LDPC_CODE_REG2_DEPTH) {
    return 0;
  }
  u32 idx;
  for(idx = 0; idx < NumData; idx++) {
    u32 addr =  XSDFEC_LDPC_CODE_REG2_ADDR_BASE+(WordOffset + idx)*XSDFEC_LDPC_CODE_REG2_STEP;
    u32 rdata = XSdFecReadReg(BaseAddress, addr);
    DataArrayPtr[idx] = (rdata & XSDFEC_LDPC_CODE_REG2_SPECIAL_QC_MASK) >> XSDFEC_LDPC_CODE_REG2_SPECIAL_QC_LSB;
  }
  return NumData;
}

/*****************************************************************************/
/**
* @brief Writes NO_FINAL_PARITY_CHECK field words of LDPC_CODE_REG2 register.
*
* @param[in]	BaseAddress Base address of the device.
* @param[in]	WordOffset Index of the first register word to access.
* @param[in]	DataArrayPtr Pointer to the array of values to write.
* @param[in]	NumData Number of words to transfer.
*
* @return
*        - NumData if the transfer completed.
*        - 0 if WordOffset plus NumData exceeds the register depth.
*
******************************************************************************/
u32 XSdFecWrite_LDPC_CODE_REG2_NO_FINAL_PARITY_CHECK_Words(UINTPTR BaseAddress, u32 WordOffset, const u32 *DataArrayPtr, u32 NumData) {
  u32 MaxDataDepth = WordOffset + NumData;
  if ( MaxDataDepth > XSDFEC_LDPC_CODE_REG2_DEPTH) {
    return 0;
  }
  u32 idx;
  for(idx = 0; idx < NumData; idx++) {
    u32 addr =  XSDFEC_LDPC_CODE_REG2_ADDR_BASE+(WordOffset + idx)*XSDFEC_LDPC_CODE_REG2_STEP;
    u32 rdata = XSdFecReadReg(BaseAddress, addr);
    u32 wdata = (rdata & ~XSDFEC_LDPC_CODE_REG2_NO_FINAL_PARITY_CHECK_MASK) | (XSDFEC_LDPC_CODE_REG2_NO_FINAL_PARITY_CHECK_MASK & (DataArrayPtr[idx] << XSDFEC_LDPC_CODE_REG2_NO_FINAL_PARITY_CHECK_LSB));
    XSdFecWriteReg(BaseAddress, addr, wdata);
  }
  return NumData;
}

/*****************************************************************************/
/**
* @brief Reads NO_FINAL_PARITY_CHECK field words of the LDPC_CODE_REG2 register.
*
* @param[in]	BaseAddress Base address of the device.
* @param[in]	WordOffset Index of the first register word to access.
* @param[out]	DataArrayPtr Pointer to the array that receives the values read.
* @param[in]	NumData Number of words to transfer.
*
* @return
*        - NumData if the transfer completed.
*        - 0 if WordOffset plus NumData exceeds the register depth.
*
******************************************************************************/
u32 XSdFecRead_LDPC_CODE_REG2_NO_FINAL_PARITY_CHECK_Words(UINTPTR BaseAddress, u32 WordOffset, u32 *DataArrayPtr, u32 NumData) {
  u32 MaxDataDepth = WordOffset + NumData;
  if (MaxDataDepth > XSDFEC_LDPC_CODE_REG2_DEPTH) {
    return 0;
  }
  u32 idx;
  for(idx = 0; idx < NumData; idx++) {
    u32 addr =  XSDFEC_LDPC_CODE_REG2_ADDR_BASE+(WordOffset + idx)*XSDFEC_LDPC_CODE_REG2_STEP;
    u32 rdata = XSdFecReadReg(BaseAddress, addr);
    DataArrayPtr[idx] = (rdata & XSDFEC_LDPC_CODE_REG2_NO_FINAL_PARITY_CHECK_MASK) >> XSDFEC_LDPC_CODE_REG2_NO_FINAL_PARITY_CHECK_LSB;
  }
  return NumData;
}

/*****************************************************************************/
/**
* @brief Writes MAX_SCHEDULE field words of the LDPC_CODE_REG2 register.
*
* @param[in]	BaseAddress Base address of the device.
* @param[in]	WordOffset Index of the first register word to access.
* @param[in]	DataArrayPtr Pointer to the array of values to write.
* @param[in]	NumData Number of words to transfer.
*
* @return
*        - NumData if the transfer completed.
*        - 0 if WordOffset plus NumData exceeds the register depth.
*
******************************************************************************/
u32 XSdFecWrite_LDPC_CODE_REG2_MAX_SCHEDULE_Words(UINTPTR BaseAddress, u32 WordOffset, const u32 *DataArrayPtr, u32 NumData) {
  u32 MaxDataDepth = WordOffset + NumData;
  if ( MaxDataDepth > XSDFEC_LDPC_CODE_REG2_DEPTH) {
    return 0;
  }
  u32 idx;
  for(idx = 0; idx < NumData; idx++) {
    u32 addr =  XSDFEC_LDPC_CODE_REG2_ADDR_BASE+(WordOffset + idx)*XSDFEC_LDPC_CODE_REG2_STEP;
    u32 rdata = XSdFecReadReg(BaseAddress, addr);
    u32 wdata = (rdata & ~XSDFEC_LDPC_CODE_REG2_MAX_SCHEDULE_MASK) | (XSDFEC_LDPC_CODE_REG2_MAX_SCHEDULE_MASK & (DataArrayPtr[idx] << XSDFEC_LDPC_CODE_REG2_MAX_SCHEDULE_LSB));
    XSdFecWriteReg(BaseAddress, addr, wdata);
  }
  return NumData;
}

/*****************************************************************************/
/**
* @brief Reads MAX_SCHEDULE field words of the LDPC_CODE_REG2 register.
*
* @param[in]	BaseAddress Base address of the device.
* @param[in]	WordOffset Index of the first register word to access.
* @param[out]	DataArrayPtr Pointer to the array that receives the values read.
* @param[in]	NumData Number of words to transfer.
*
* @return
*        - NumData if the transfer completed.
*        - 0 if WordOffset plus NumData exceeds the register depth.
*
******************************************************************************/
u32 XSdFecRead_LDPC_CODE_REG2_MAX_SCHEDULE_Words(UINTPTR BaseAddress, u32 WordOffset, u32 *DataArrayPtr, u32 NumData) {
  u32 MaxDataDepth = WordOffset + NumData;
  if (MaxDataDepth > XSDFEC_LDPC_CODE_REG2_DEPTH) {
    return 0;
  }
  u32 idx;
  for(idx = 0; idx < NumData; idx++) {
    u32 addr =  XSDFEC_LDPC_CODE_REG2_ADDR_BASE+(WordOffset + idx)*XSDFEC_LDPC_CODE_REG2_STEP;
    u32 rdata = XSdFecReadReg(BaseAddress, addr);
    DataArrayPtr[idx] = (rdata & XSDFEC_LDPC_CODE_REG2_MAX_SCHEDULE_MASK) >> XSDFEC_LDPC_CODE_REG2_MAX_SCHEDULE_LSB;
  }
  return NumData;
}

/*****************************************************************************/
/**
* @brief Writes words of the LDPC_CODE_REG2 register.
*
* @param[in]	BaseAddress Base address of the device.
* @param[in]	WordOffset Index of the first register word to access.
* @param[in]	DataArrayPtr Pointer to the array of values to write.
* @param[in]	NumData Number of words to transfer.
*
* @return
*        - NumData if the transfer completed.
*        - 0 if WordOffset plus NumData exceeds the register depth.
*
******************************************************************************/
u32 XSdFecWrite_LDPC_CODE_REG2_Words(UINTPTR BaseAddress, u32 WordOffset, const u32 *DataArrayPtr, u32 NumData) {
  u32 MaxDataDepth = (WordOffset + NumData);
  if (MaxDataDepth > XSDFEC_LDPC_CODE_REG2_DEPTH) {
    return 0;
  }
  u32 idx;
  for(idx = 0; idx < NumData; idx++) {
    u32 addr =  XSDFEC_LDPC_CODE_REG2_ADDR_BASE+(WordOffset + idx)*XSDFEC_LDPC_CODE_REG2_STEP;
    XSdFecWriteReg(BaseAddress, addr, DataArrayPtr[idx]);
  }
  return NumData;
}

/*****************************************************************************/
/**
* @brief Reads words of the LDPC_CODE_REG2 register.
*
* @param[in]	BaseAddress Base address of the device.
* @param[in]	WordOffset Index of the first register word to access.
* @param[out]	DataArrayPtr Pointer to the array that receives the values read.
* @param[in]	NumData Number of words to transfer.
*
* @return
*        - NumData if the transfer completed.
*        - 0 if WordOffset plus NumData exceeds the register depth.
*
******************************************************************************/
u32 XSdFecRead_LDPC_CODE_REG2_Words(UINTPTR BaseAddress, u32 WordOffset, u32 *DataArrayPtr, u32 NumData) {
  u32 MaxDataDepth = (WordOffset + NumData);
  if (MaxDataDepth > XSDFEC_LDPC_CODE_REG2_DEPTH) {
    return 0;
  }
  u32 idx;
  for(idx = 0; idx < NumData; idx++) {
    u32 addr =  XSDFEC_LDPC_CODE_REG2_ADDR_BASE+(WordOffset + idx)*XSDFEC_LDPC_CODE_REG2_STEP;
    DataArrayPtr[idx] = XSdFecReadReg(BaseAddress, addr);
  }
  return NumData;
}

/*****************************************************************************/
/**
* @brief Writes SC_OFF field words of the LDPC_CODE_REG3 register.
*
* @param[in]	BaseAddress Base address of the device.
* @param[in]	WordOffset Index of the first register word to access.
* @param[in]	DataArrayPtr Pointer to the array of values to write.
* @param[in]	NumData Number of words to transfer.
*
* @return
*        - NumData if the transfer completed.
*        - 0 if WordOffset plus NumData exceeds the register depth.
*
******************************************************************************/
u32 XSdFecWrite_LDPC_CODE_REG3_SC_OFF_Words(UINTPTR BaseAddress, u32 WordOffset, const u32 *DataArrayPtr, u32 NumData) {
  u32 MaxDataDepth = WordOffset + NumData;
  if ( MaxDataDepth > XSDFEC_LDPC_CODE_REG3_DEPTH) {
    return 0;
  }
  u32 idx;
  for(idx = 0; idx < NumData; idx++) {
    u32 addr =  XSDFEC_LDPC_CODE_REG3_ADDR_BASE+(WordOffset + idx)*XSDFEC_LDPC_CODE_REG3_STEP;
    u32 rdata = XSdFecReadReg(BaseAddress, addr);
    u32 wdata = (rdata & ~XSDFEC_LDPC_CODE_REG3_SC_OFF_MASK) | (XSDFEC_LDPC_CODE_REG3_SC_OFF_MASK & (DataArrayPtr[idx] << XSDFEC_LDPC_CODE_REG3_SC_OFF_LSB));
    XSdFecWriteReg(BaseAddress, addr, wdata);
  }
  return NumData;
}

/*****************************************************************************/
/**
* @brief Reads SC_OFF field words of the LDPC_CODE_REG3 register.
*
* @param[in]	BaseAddress Base address of the device.
* @param[in]	WordOffset Index of the first register word to access.
* @param[out]	DataArrayPtr Pointer to the array that receives the values read.
* @param[in]	NumData Number of words to transfer.
*
* @return
*        - NumData if the transfer completed.
*        - 0 if WordOffset plus NumData exceeds the register depth.
*
******************************************************************************/
u32 XSdFecRead_LDPC_CODE_REG3_SC_OFF_Words(UINTPTR BaseAddress, u32 WordOffset, u32 *DataArrayPtr, u32 NumData) {
  u32 MaxDataDepth = WordOffset + NumData;
  if (MaxDataDepth > XSDFEC_LDPC_CODE_REG3_DEPTH) {
    return 0;
  }
  u32 idx;
  for(idx = 0; idx < NumData; idx++) {
    u32 addr =  XSDFEC_LDPC_CODE_REG3_ADDR_BASE+(WordOffset + idx)*XSDFEC_LDPC_CODE_REG3_STEP;
    u32 rdata = XSdFecReadReg(BaseAddress, addr);
    DataArrayPtr[idx] = (rdata & XSDFEC_LDPC_CODE_REG3_SC_OFF_MASK) >> XSDFEC_LDPC_CODE_REG3_SC_OFF_LSB;
  }
  return NumData;
}

/*****************************************************************************/
/**
* @brief Writes LA_OFF field words of the LDPC_CODE_REG3 register.
*
* @param[in]	BaseAddress Base address of the device.
* @param[in]	WordOffset Index of the first register word to access.
* @param[in]	DataArrayPtr Pointer to the array of values to write.
* @param[in]	NumData Number of words to transfer.
*
* @return
*        - NumData if the transfer completed.
*        - 0 if WordOffset plus NumData exceeds the register depth.
*
******************************************************************************/
u32 XSdFecWrite_LDPC_CODE_REG3_LA_OFF_Words(UINTPTR BaseAddress, u32 WordOffset, const u32 *DataArrayPtr, u32 NumData) {
  u32 MaxDataDepth = WordOffset + NumData;
  if ( MaxDataDepth > XSDFEC_LDPC_CODE_REG3_DEPTH) {
    return 0;
  }
  u32 idx;
  for(idx = 0; idx < NumData; idx++) {
    u32 addr =  XSDFEC_LDPC_CODE_REG3_ADDR_BASE+(WordOffset + idx)*XSDFEC_LDPC_CODE_REG3_STEP;
    u32 rdata = XSdFecReadReg(BaseAddress, addr);
    u32 wdata = (rdata & ~XSDFEC_LDPC_CODE_REG3_LA_OFF_MASK) | (XSDFEC_LDPC_CODE_REG3_LA_OFF_MASK & (DataArrayPtr[idx] << XSDFEC_LDPC_CODE_REG3_LA_OFF_LSB));
    XSdFecWriteReg(BaseAddress, addr, wdata);
  }
  return NumData;
}

/*****************************************************************************/
/**
* @brief Reads LA_OFF field words of the LDPC_CODE_REG3 register.
*
* @param[in]	BaseAddress Base address of the device.
* @param[in]	WordOffset Index of the first register word to access.
* @param[out]	DataArrayPtr Pointer to the array that receives the values read.
* @param[in]	NumData Number of words to transfer.
*
* @return
*        - NumData if the transfer completed.
*        - 0 if WordOffset plus NumData exceeds the register depth.
*
******************************************************************************/
u32 XSdFecRead_LDPC_CODE_REG3_LA_OFF_Words(UINTPTR BaseAddress, u32 WordOffset, u32 *DataArrayPtr, u32 NumData) {
  u32 MaxDataDepth = WordOffset + NumData;
  if (MaxDataDepth > XSDFEC_LDPC_CODE_REG3_DEPTH) {
    return 0;
  }
  u32 idx;
  for(idx = 0; idx < NumData; idx++) {
    u32 addr =  XSDFEC_LDPC_CODE_REG3_ADDR_BASE+(WordOffset + idx)*XSDFEC_LDPC_CODE_REG3_STEP;
    u32 rdata = XSdFecReadReg(BaseAddress, addr);
    DataArrayPtr[idx] = (rdata & XSDFEC_LDPC_CODE_REG3_LA_OFF_MASK) >> XSDFEC_LDPC_CODE_REG3_LA_OFF_LSB;
  }
  return NumData;
}

/*****************************************************************************/
/**
* @brief Writes QC_OFF field words of the LDPC_CODE_REG3 register.
*
* @param[in]	BaseAddress Base address of the device.
* @param[in]	WordOffset Index of the first register word to access.
* @param[in]	DataArrayPtr Pointer to the array of values to write.
* @param[in]	NumData Number of words to transfer.
*
* @return
*        - NumData if the transfer completed.
*        - 0 if WordOffset plus NumData exceeds the register depth.
*
******************************************************************************/
u32 XSdFecWrite_LDPC_CODE_REG3_QC_OFF_Words(UINTPTR BaseAddress, u32 WordOffset, const u32 *DataArrayPtr, u32 NumData) {
  u32 MaxDataDepth = WordOffset + NumData;
  if ( MaxDataDepth > XSDFEC_LDPC_CODE_REG3_DEPTH) {
    return 0;
  }
  u32 idx;
  for(idx = 0; idx < NumData; idx++) {
    u32 addr =  XSDFEC_LDPC_CODE_REG3_ADDR_BASE+(WordOffset + idx)*XSDFEC_LDPC_CODE_REG3_STEP;
    u32 rdata = XSdFecReadReg(BaseAddress, addr);
    u32 wdata = (rdata & ~XSDFEC_LDPC_CODE_REG3_QC_OFF_MASK) | (XSDFEC_LDPC_CODE_REG3_QC_OFF_MASK & (DataArrayPtr[idx] << XSDFEC_LDPC_CODE_REG3_QC_OFF_LSB));
    XSdFecWriteReg(BaseAddress, addr, wdata);
  }
  return NumData;
}

/*****************************************************************************/
/**
* @brief Reads QC_OFF field words of the LDPC_CODE_REG3 register.
*
* @param[in]	BaseAddress Base address of the device.
* @param[in]	WordOffset Index of the first register word to access.
* @param[out]	DataArrayPtr Pointer to the array that receives the values read.
* @param[in]	NumData Number of words to transfer.
*
* @return
*        - NumData if the transfer completed.
*        - 0 if WordOffset plus NumData exceeds the register depth.
*
******************************************************************************/
u32 XSdFecRead_LDPC_CODE_REG3_QC_OFF_Words(UINTPTR BaseAddress, u32 WordOffset, u32 *DataArrayPtr, u32 NumData) {
  u32 MaxDataDepth = WordOffset + NumData;
  if (MaxDataDepth > XSDFEC_LDPC_CODE_REG3_DEPTH) {
    return 0;
  }
  u32 idx;
  for(idx = 0; idx < NumData; idx++) {
    u32 addr =  XSDFEC_LDPC_CODE_REG3_ADDR_BASE+(WordOffset + idx)*XSDFEC_LDPC_CODE_REG3_STEP;
    u32 rdata = XSdFecReadReg(BaseAddress, addr);
    DataArrayPtr[idx] = (rdata & XSDFEC_LDPC_CODE_REG3_QC_OFF_MASK) >> XSDFEC_LDPC_CODE_REG3_QC_OFF_LSB;
  }
  return NumData;
}

/*****************************************************************************/
/**
* @brief Writes words of the LDPC_CODE_REG3 register.
*
* @param[in]	BaseAddress Base address of the device.
* @param[in]	WordOffset Index of the first register word to access.
* @param[in]	DataArrayPtr Pointer to the array of values to write.
* @param[in]	NumData Number of words to transfer.
*
* @return
*        - NumData if the transfer completed.
*        - 0 if WordOffset plus NumData exceeds the register depth.
*
******************************************************************************/
u32 XSdFecWrite_LDPC_CODE_REG3_Words(UINTPTR BaseAddress, u32 WordOffset, const u32 *DataArrayPtr, u32 NumData) {
  u32 MaxDataDepth = (WordOffset + NumData);
  if (MaxDataDepth > XSDFEC_LDPC_CODE_REG3_DEPTH) {
    return 0;
  }
  u32 idx;
  for(idx = 0; idx < NumData; idx++) {
    u32 addr =  XSDFEC_LDPC_CODE_REG3_ADDR_BASE+(WordOffset + idx)*XSDFEC_LDPC_CODE_REG3_STEP;
    XSdFecWriteReg(BaseAddress, addr, DataArrayPtr[idx]);
  }
  return NumData;
}

/*****************************************************************************/
/**
* @brief Reads words of the LDPC_CODE_REG3 register.
*
* @param[in]	BaseAddress Base address of the device.
* @param[in]	WordOffset Index of the first register word to access.
* @param[out]	DataArrayPtr Pointer to the array that receives the values read.
* @param[in]	NumData Number of words to transfer.
*
* @return
*        - NumData if the transfer completed.
*        - 0 if WordOffset plus NumData exceeds the register depth.
*
******************************************************************************/
u32 XSdFecRead_LDPC_CODE_REG3_Words(UINTPTR BaseAddress, u32 WordOffset, u32 *DataArrayPtr, u32 NumData) {
  u32 MaxDataDepth = (WordOffset + NumData);
  if (MaxDataDepth > XSDFEC_LDPC_CODE_REG3_DEPTH) {
    return 0;
  }
  u32 idx;
  for(idx = 0; idx < NumData; idx++) {
    u32 addr =  XSDFEC_LDPC_CODE_REG3_ADDR_BASE+(WordOffset + idx)*XSDFEC_LDPC_CODE_REG3_STEP;
    DataArrayPtr[idx] = XSdFecReadReg(BaseAddress, addr);
  }
  return NumData;
}

/*****************************************************************************/
/**
* @brief Writes words of the LDPC_SC_TABLE register.
*
* @param[in]	BaseAddress Base address of the device.
* @param[in]	WordOffset Index of the first register word to access.
* @param[in]	DataArrayPtr Pointer to the array of values to write.
* @param[in]	NumData Number of words to transfer.
*
* @return
*        - NumData if the transfer completed.
*        - 0 if WordOffset plus NumData exceeds the register depth.
*
******************************************************************************/
u32 XSdFecWrite_LDPC_SC_TABLE_Words(UINTPTR BaseAddress, u32 WordOffset, const u32 *DataArrayPtr, u32 NumData) {
  u32 MaxDataDepth = (WordOffset + NumData);
  if (MaxDataDepth > XSDFEC_LDPC_SC_TABLE_DEPTH) {
    return 0;
  }
  u32 idx;
  for(idx = 0; idx < NumData; idx++) {
    u32 addr =  XSDFEC_LDPC_SC_TABLE_ADDR_BASE+(WordOffset + idx)*XSDFEC_LDPC_SC_TABLE_STEP;
    XSdFecWriteReg(BaseAddress, addr, DataArrayPtr[idx]);
  }
  return NumData;
}

/*****************************************************************************/
/**
* @brief Reads words of the LDPC_SC_TABLE register.
*
* @param[in]	BaseAddress Base address of the device.
* @param[in]	WordOffset Index of the first register word to access.
* @param[out]	DataArrayPtr Pointer to the array that receives the values read.
* @param[in]	NumData Number of words to transfer.
*
* @return
*        - NumData if the transfer completed.
*        - 0 if WordOffset plus NumData exceeds the register depth.
*
******************************************************************************/
u32 XSdFecRead_LDPC_SC_TABLE_Words(UINTPTR BaseAddress, u32 WordOffset, u32 *DataArrayPtr, u32 NumData) {
  u32 MaxDataDepth = (WordOffset + NumData);
  if (MaxDataDepth > XSDFEC_LDPC_SC_TABLE_DEPTH) {
    return 0;
  }
  u32 idx;
  for(idx = 0; idx < NumData; idx++) {
    u32 addr =  XSDFEC_LDPC_SC_TABLE_ADDR_BASE+(WordOffset + idx)*XSDFEC_LDPC_SC_TABLE_STEP;
    DataArrayPtr[idx] = XSdFecReadReg(BaseAddress, addr);
  }
  return NumData;
}

/*****************************************************************************/
/**
* @brief Writes words of the LDPC_LA_TABLE register.
*
* @param[in]	BaseAddress Base address of the device.
* @param[in]	WordOffset Index of the first register word to access.
* @param[in]	DataArrayPtr Pointer to the array of values to write.
* @param[in]	NumData Number of words to transfer.
*
* @return
*        - NumData if the transfer completed.
*        - 0 if WordOffset plus NumData exceeds the register depth.
*
******************************************************************************/
u32 XSdFecWrite_LDPC_LA_TABLE_Words(UINTPTR BaseAddress, u32 WordOffset, const u32 *DataArrayPtr, u32 NumData) {
  u32 MaxDataDepth = (WordOffset + NumData);
  if (MaxDataDepth > XSDFEC_LDPC_LA_TABLE_DEPTH) {
    return 0;
  }
  u32 idx;
  for(idx = 0; idx < NumData; idx++) {
    u32 addr =  XSDFEC_LDPC_LA_TABLE_ADDR_BASE+(WordOffset + idx)*XSDFEC_LDPC_LA_TABLE_STEP;
    XSdFecWriteReg(BaseAddress, addr, DataArrayPtr[idx]);
  }
  return NumData;
}

/*****************************************************************************/
/**
* @brief Reads words of the LDPC_LA_TABLE register.
*
* @param[in]	BaseAddress Base address of the device.
* @param[in]	WordOffset Index of the first register word to access.
* @param[out]	DataArrayPtr Pointer to the array that receives the values read.
* @param[in]	NumData Number of words to transfer.
*
* @return
*        - NumData if the transfer completed.
*        - 0 if WordOffset plus NumData exceeds the register depth.
*
******************************************************************************/
u32 XSdFecRead_LDPC_LA_TABLE_Words(UINTPTR BaseAddress, u32 WordOffset, u32 *DataArrayPtr, u32 NumData) {
  u32 MaxDataDepth = (WordOffset + NumData);
  if (MaxDataDepth > XSDFEC_LDPC_LA_TABLE_DEPTH) {
    return 0;
  }
  u32 idx;
  for(idx = 0; idx < NumData; idx++) {
    u32 addr =  XSDFEC_LDPC_LA_TABLE_ADDR_BASE+(WordOffset + idx)*XSDFEC_LDPC_LA_TABLE_STEP;
    DataArrayPtr[idx] = XSdFecReadReg(BaseAddress, addr);
  }
  return NumData;
}

/*****************************************************************************/
/**
* @brief Writes words of the LDPC_QC_TABLE register.
*
* @param[in]	BaseAddress Base address of the device.
* @param[in]	WordOffset Index of the first register word to access.
* @param[in]	DataArrayPtr Pointer to the array of values to write.
* @param[in]	NumData Number of words to transfer.
*
* @return
*        - NumData if the transfer completed.
*        - 0 if WordOffset plus NumData exceeds the register depth.
*
******************************************************************************/
u32 XSdFecWrite_LDPC_QC_TABLE_Words(UINTPTR BaseAddress, u32 WordOffset, const u32 *DataArrayPtr, u32 NumData) {
  u32 MaxDataDepth = (WordOffset + NumData);
  if (MaxDataDepth > XSDFEC_LDPC_QC_TABLE_DEPTH) {
    return 0;
  }
  u32 idx;
  for(idx = 0; idx < NumData; idx++) {
    u32 addr =  XSDFEC_LDPC_QC_TABLE_ADDR_BASE+(WordOffset + idx)*XSDFEC_LDPC_QC_TABLE_STEP;
    XSdFecWriteReg(BaseAddress, addr, DataArrayPtr[idx]);
  }
  return NumData;
}

/*****************************************************************************/
/**
* @brief Reads words of the LDPC_QC_TABLE register.
*
* @param[in]	BaseAddress Base address of the device.
* @param[in]	WordOffset Index of the first register word to access.
* @param[out]	DataArrayPtr Pointer to the array that receives the values read.
* @param[in]	NumData Number of words to transfer.
*
* @return
*        - NumData if the transfer completed.
*        - 0 if WordOffset plus NumData exceeds the register depth.
*
******************************************************************************/
u32 XSdFecRead_LDPC_QC_TABLE_Words(UINTPTR BaseAddress, u32 WordOffset, u32 *DataArrayPtr, u32 NumData) {
  u32 MaxDataDepth = (WordOffset + NumData);
  if (MaxDataDepth > XSDFEC_LDPC_QC_TABLE_DEPTH) {
    return 0;
  }
  u32 idx;
  for(idx = 0; idx < NumData; idx++) {
    u32 addr =  XSDFEC_LDPC_QC_TABLE_ADDR_BASE+(WordOffset + idx)*XSDFEC_LDPC_QC_TABLE_STEP;
    DataArrayPtr[idx] = XSdFecReadReg(BaseAddress, addr);
  }
  return NumData;
}
/** @} */
