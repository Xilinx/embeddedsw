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

/***************************** Include Files *********************************/
#include "xsdfec.h"

/************************** Function Implementation *************************/
int XSdFecCfgInitialize(XSdFec *InstancePtr, XSdFec_Config *ConfigPtr) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(ConfigPtr != NULL);

    InstancePtr->BaseAddress  = ConfigPtr->BaseAddress;
    InstancePtr->Standard     = ConfigPtr->Standard;
    // Initialize fixed register configurations
    for (int i=0;i<4;i+=2) {
      u32 addr  = ConfigPtr->Initialization[i];
      u32 wdata = ConfigPtr->Initialization[i+1];
      XSdFecWriteReg(InstancePtr->BaseAddress, addr, wdata);
    }
    InstancePtr->IsReady = XIL_COMPONENT_IS_READY;

    return XST_SUCCESS;
}

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
    
    // Store offsets
    InstancePtr->SCOffset[CodeId] = SCOffset;
    InstancePtr->LAOffset[CodeId] = LAOffset;
    InstancePtr->QCOffset[CodeId] = QCOffset;
  }
}

void XSdFecShareTableSize(const XSdFecLdpcParameters* ParamsPtr, u32* SCSizePtr, u32* LASizePtr, u32* QCSizePtr) {
  Xil_AssertVoid(ParamsPtr != NULL);
  if (SCSizePtr) {
    *SCSizePtr = (ParamsPtr->NLayers+3)>>2;
  }
  if (LASizePtr) {
    *LASizePtr = ((ParamsPtr->NLayers<<2)+15)>>4; // Multiple of 16
  }
  if (QCSizePtr) {
    *QCSizePtr = ((ParamsPtr->NQC<<2)+15)>>4;
  }
}

void XSdFecSetTurboParams(XSdFec *InstancePtr, const XSdFecTurboParameters* ParamsPtr) {
  Xil_AssertVoid(InstancePtr != NULL);
  Xil_AssertVoid(ParamsPtr   != NULL);
  Xil_AssertVoid(InstancePtr->IsReady  == XIL_COMPONENT_IS_READY);
  Xil_AssertVoid(InstancePtr->Standard == XSDFEC_STANDARD_OTHER);
  XSdFecSet_TURBO_ALG(InstancePtr->BaseAddress,ParamsPtr->Alg);
  XSdFecSet_TURBO_SCALE_FACTOR(InstancePtr->BaseAddress,ParamsPtr->Scale);
}

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
  // Hard block ECC error (single or multi-bit)
  u32 ecc_errbits  = isr_ecc & 0x0007FF;
  // Hard block multi-bit
  u32 ecc_errmbits = (isr_ecc & 0x3FF800) >> 11;
  // PL logic ECC error (single or multi-bit)
  u32 soft_ecc_errbits  = (isr_ecc & 0x03C00000) >> 22;
  // PL logic multi-bit
  u32 soft_ecc_errmbits = (isr_ecc & 0x3C000000) >> 26;
  // XOR ecc error bits with multi-bit errors to determine any single bit
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

/************************** Base API Function Implementation *************************/
void XSdFecSet_CORE_AXI_WR_PROTECT(UINTPTR BaseAddress, u32 Data) {
  XSdFecWriteReg(BaseAddress, XSDFEC_CORE_AXI_WR_PROTECT_ADDR, Data);
}

u32 XSdFecGet_CORE_AXI_WR_PROTECT(UINTPTR BaseAddress) {
  return XSdFecReadReg(BaseAddress, XSDFEC_CORE_AXI_WR_PROTECT_ADDR);
}

void XSdFecSet_CORE_CODE_WR_PROTECT(UINTPTR BaseAddress, u32 Data) {
  XSdFecWriteReg(BaseAddress, XSDFEC_CORE_CODE_WR_PROTECT_ADDR, Data);
}

u32 XSdFecGet_CORE_CODE_WR_PROTECT(UINTPTR BaseAddress) {
  return XSdFecReadReg(BaseAddress, XSDFEC_CORE_CODE_WR_PROTECT_ADDR);
}

u32 XSdFecGet_CORE_ACTIVE(UINTPTR BaseAddress) {
  return XSdFecReadReg(BaseAddress, XSDFEC_CORE_ACTIVE_ADDR);
}

void XSdFecSet_CORE_AXIS_WIDTH_DIN(UINTPTR BaseAddress, u32 Data) {
  u32 rdata = XSdFecReadReg(BaseAddress, XSDFEC_CORE_AXIS_WIDTH_ADDR);
  rdata = rdata & ~XSDFEC_CORE_AXIS_WIDTH_DIN_MASK;
  u32 wdata = (rdata & ~XSDFEC_CORE_AXIS_WIDTH_DIN_MASK) | (XSDFEC_CORE_AXIS_WIDTH_DIN_MASK & (Data << XSDFEC_CORE_AXIS_WIDTH_DIN_LSB));
  XSdFecWriteReg(BaseAddress, XSDFEC_CORE_AXIS_WIDTH_ADDR, wdata);
}

u32 XSdFecGet_CORE_AXIS_WIDTH_DIN(UINTPTR BaseAddress) {
  u32 rdata = XSdFecReadReg(BaseAddress, XSDFEC_CORE_AXIS_WIDTH_ADDR);
  return (rdata & XSDFEC_CORE_AXIS_WIDTH_DIN_MASK) >> XSDFEC_CORE_AXIS_WIDTH_DIN_LSB;
}

void XSdFecSet_CORE_AXIS_WIDTH_DIN_WORDS(UINTPTR BaseAddress, u32 Data) {
  u32 rdata = XSdFecReadReg(BaseAddress, XSDFEC_CORE_AXIS_WIDTH_ADDR);
  rdata = rdata & ~XSDFEC_CORE_AXIS_WIDTH_DIN_WORDS_MASK;
  u32 wdata = (rdata & ~XSDFEC_CORE_AXIS_WIDTH_DIN_WORDS_MASK) | (XSDFEC_CORE_AXIS_WIDTH_DIN_WORDS_MASK & (Data << XSDFEC_CORE_AXIS_WIDTH_DIN_WORDS_LSB));
  XSdFecWriteReg(BaseAddress, XSDFEC_CORE_AXIS_WIDTH_ADDR, wdata);
}

u32 XSdFecGet_CORE_AXIS_WIDTH_DIN_WORDS(UINTPTR BaseAddress) {
  u32 rdata = XSdFecReadReg(BaseAddress, XSDFEC_CORE_AXIS_WIDTH_ADDR);
  return (rdata & XSDFEC_CORE_AXIS_WIDTH_DIN_WORDS_MASK) >> XSDFEC_CORE_AXIS_WIDTH_DIN_WORDS_LSB;
}

void XSdFecSet_CORE_AXIS_WIDTH_DOUT(UINTPTR BaseAddress, u32 Data) {
  u32 rdata = XSdFecReadReg(BaseAddress, XSDFEC_CORE_AXIS_WIDTH_ADDR);
  rdata = rdata & ~XSDFEC_CORE_AXIS_WIDTH_DOUT_MASK;
  u32 wdata = (rdata & ~XSDFEC_CORE_AXIS_WIDTH_DOUT_MASK) | (XSDFEC_CORE_AXIS_WIDTH_DOUT_MASK & (Data << XSDFEC_CORE_AXIS_WIDTH_DOUT_LSB));
  XSdFecWriteReg(BaseAddress, XSDFEC_CORE_AXIS_WIDTH_ADDR, wdata);
}

u32 XSdFecGet_CORE_AXIS_WIDTH_DOUT(UINTPTR BaseAddress) {
  u32 rdata = XSdFecReadReg(BaseAddress, XSDFEC_CORE_AXIS_WIDTH_ADDR);
  return (rdata & XSDFEC_CORE_AXIS_WIDTH_DOUT_MASK) >> XSDFEC_CORE_AXIS_WIDTH_DOUT_LSB;
}

void XSdFecSet_CORE_AXIS_WIDTH_DOUT_WORDS(UINTPTR BaseAddress, u32 Data) {
  u32 rdata = XSdFecReadReg(BaseAddress, XSDFEC_CORE_AXIS_WIDTH_ADDR);
  rdata = rdata & ~XSDFEC_CORE_AXIS_WIDTH_DOUT_WORDS_MASK;
  u32 wdata = (rdata & ~XSDFEC_CORE_AXIS_WIDTH_DOUT_WORDS_MASK) | (XSDFEC_CORE_AXIS_WIDTH_DOUT_WORDS_MASK & (Data << XSDFEC_CORE_AXIS_WIDTH_DOUT_WORDS_LSB));
  XSdFecWriteReg(BaseAddress, XSDFEC_CORE_AXIS_WIDTH_ADDR, wdata);
}

u32 XSdFecGet_CORE_AXIS_WIDTH_DOUT_WORDS(UINTPTR BaseAddress) {
  u32 rdata = XSdFecReadReg(BaseAddress, XSDFEC_CORE_AXIS_WIDTH_ADDR);
  return (rdata & XSDFEC_CORE_AXIS_WIDTH_DOUT_WORDS_MASK) >> XSDFEC_CORE_AXIS_WIDTH_DOUT_WORDS_LSB;
}

void XSdFecSet_CORE_AXIS_WIDTH(UINTPTR BaseAddress, u32 Data) {
  XSdFecWriteReg(BaseAddress, XSDFEC_CORE_AXIS_WIDTH_ADDR, Data);
}

u32 XSdFecGet_CORE_AXIS_WIDTH(UINTPTR BaseAddress) {
  return XSdFecReadReg(BaseAddress, XSDFEC_CORE_AXIS_WIDTH_ADDR);
}

void XSdFecSet_CORE_AXIS_ENABLE_CTRL(UINTPTR BaseAddress, u32 Data) {
  u32 rdata = XSdFecReadReg(BaseAddress, XSDFEC_CORE_AXIS_ENABLE_ADDR);
  rdata = rdata & ~XSDFEC_CORE_AXIS_ENABLE_CTRL_MASK;
  u32 wdata = (rdata & ~XSDFEC_CORE_AXIS_ENABLE_CTRL_MASK) | (XSDFEC_CORE_AXIS_ENABLE_CTRL_MASK & (Data << XSDFEC_CORE_AXIS_ENABLE_CTRL_LSB));
  XSdFecWriteReg(BaseAddress, XSDFEC_CORE_AXIS_ENABLE_ADDR, wdata);
}

u32 XSdFecGet_CORE_AXIS_ENABLE_CTRL(UINTPTR BaseAddress) {
  u32 rdata = XSdFecReadReg(BaseAddress, XSDFEC_CORE_AXIS_ENABLE_ADDR);
  return (rdata & XSDFEC_CORE_AXIS_ENABLE_CTRL_MASK) >> XSDFEC_CORE_AXIS_ENABLE_CTRL_LSB;
}

void XSdFecSet_CORE_AXIS_ENABLE_DIN(UINTPTR BaseAddress, u32 Data) {
  u32 rdata = XSdFecReadReg(BaseAddress, XSDFEC_CORE_AXIS_ENABLE_ADDR);
  rdata = rdata & ~XSDFEC_CORE_AXIS_ENABLE_DIN_MASK;
  u32 wdata = (rdata & ~XSDFEC_CORE_AXIS_ENABLE_DIN_MASK) | (XSDFEC_CORE_AXIS_ENABLE_DIN_MASK & (Data << XSDFEC_CORE_AXIS_ENABLE_DIN_LSB));
  XSdFecWriteReg(BaseAddress, XSDFEC_CORE_AXIS_ENABLE_ADDR, wdata);
}

u32 XSdFecGet_CORE_AXIS_ENABLE_DIN(UINTPTR BaseAddress) {
  u32 rdata = XSdFecReadReg(BaseAddress, XSDFEC_CORE_AXIS_ENABLE_ADDR);
  return (rdata & XSDFEC_CORE_AXIS_ENABLE_DIN_MASK) >> XSDFEC_CORE_AXIS_ENABLE_DIN_LSB;
}

void XSdFecSet_CORE_AXIS_ENABLE_DIN_WORDS(UINTPTR BaseAddress, u32 Data) {
  u32 rdata = XSdFecReadReg(BaseAddress, XSDFEC_CORE_AXIS_ENABLE_ADDR);
  rdata = rdata & ~XSDFEC_CORE_AXIS_ENABLE_DIN_WORDS_MASK;
  u32 wdata = (rdata & ~XSDFEC_CORE_AXIS_ENABLE_DIN_WORDS_MASK) | (XSDFEC_CORE_AXIS_ENABLE_DIN_WORDS_MASK & (Data << XSDFEC_CORE_AXIS_ENABLE_DIN_WORDS_LSB));
  XSdFecWriteReg(BaseAddress, XSDFEC_CORE_AXIS_ENABLE_ADDR, wdata);
}

u32 XSdFecGet_CORE_AXIS_ENABLE_DIN_WORDS(UINTPTR BaseAddress) {
  u32 rdata = XSdFecReadReg(BaseAddress, XSDFEC_CORE_AXIS_ENABLE_ADDR);
  return (rdata & XSDFEC_CORE_AXIS_ENABLE_DIN_WORDS_MASK) >> XSDFEC_CORE_AXIS_ENABLE_DIN_WORDS_LSB;
}

void XSdFecSet_CORE_AXIS_ENABLE_STATUS(UINTPTR BaseAddress, u32 Data) {
  u32 rdata = XSdFecReadReg(BaseAddress, XSDFEC_CORE_AXIS_ENABLE_ADDR);
  rdata = rdata & ~XSDFEC_CORE_AXIS_ENABLE_STATUS_MASK;
  u32 wdata = (rdata & ~XSDFEC_CORE_AXIS_ENABLE_STATUS_MASK) | (XSDFEC_CORE_AXIS_ENABLE_STATUS_MASK & (Data << XSDFEC_CORE_AXIS_ENABLE_STATUS_LSB));
  XSdFecWriteReg(BaseAddress, XSDFEC_CORE_AXIS_ENABLE_ADDR, wdata);
}

u32 XSdFecGet_CORE_AXIS_ENABLE_STATUS(UINTPTR BaseAddress) {
  u32 rdata = XSdFecReadReg(BaseAddress, XSDFEC_CORE_AXIS_ENABLE_ADDR);
  return (rdata & XSDFEC_CORE_AXIS_ENABLE_STATUS_MASK) >> XSDFEC_CORE_AXIS_ENABLE_STATUS_LSB;
}

void XSdFecSet_CORE_AXIS_ENABLE_DOUT(UINTPTR BaseAddress, u32 Data) {
  u32 rdata = XSdFecReadReg(BaseAddress, XSDFEC_CORE_AXIS_ENABLE_ADDR);
  rdata = rdata & ~XSDFEC_CORE_AXIS_ENABLE_DOUT_MASK;
  u32 wdata = (rdata & ~XSDFEC_CORE_AXIS_ENABLE_DOUT_MASK) | (XSDFEC_CORE_AXIS_ENABLE_DOUT_MASK & (Data << XSDFEC_CORE_AXIS_ENABLE_DOUT_LSB));
  XSdFecWriteReg(BaseAddress, XSDFEC_CORE_AXIS_ENABLE_ADDR, wdata);
}

u32 XSdFecGet_CORE_AXIS_ENABLE_DOUT(UINTPTR BaseAddress) {
  u32 rdata = XSdFecReadReg(BaseAddress, XSDFEC_CORE_AXIS_ENABLE_ADDR);
  return (rdata & XSDFEC_CORE_AXIS_ENABLE_DOUT_MASK) >> XSDFEC_CORE_AXIS_ENABLE_DOUT_LSB;
}

void XSdFecSet_CORE_AXIS_ENABLE_DOUT_WORDS(UINTPTR BaseAddress, u32 Data) {
  u32 rdata = XSdFecReadReg(BaseAddress, XSDFEC_CORE_AXIS_ENABLE_ADDR);
  rdata = rdata & ~XSDFEC_CORE_AXIS_ENABLE_DOUT_WORDS_MASK;
  u32 wdata = (rdata & ~XSDFEC_CORE_AXIS_ENABLE_DOUT_WORDS_MASK) | (XSDFEC_CORE_AXIS_ENABLE_DOUT_WORDS_MASK & (Data << XSDFEC_CORE_AXIS_ENABLE_DOUT_WORDS_LSB));
  XSdFecWriteReg(BaseAddress, XSDFEC_CORE_AXIS_ENABLE_ADDR, wdata);
}

u32 XSdFecGet_CORE_AXIS_ENABLE_DOUT_WORDS(UINTPTR BaseAddress) {
  u32 rdata = XSdFecReadReg(BaseAddress, XSDFEC_CORE_AXIS_ENABLE_ADDR);
  return (rdata & XSDFEC_CORE_AXIS_ENABLE_DOUT_WORDS_MASK) >> XSDFEC_CORE_AXIS_ENABLE_DOUT_WORDS_LSB;
}

void XSdFecSet_CORE_AXIS_ENABLE(UINTPTR BaseAddress, u32 Data) {
  XSdFecWriteReg(BaseAddress, XSDFEC_CORE_AXIS_ENABLE_ADDR, Data);
}

u32 XSdFecGet_CORE_AXIS_ENABLE(UINTPTR BaseAddress) {
  return XSdFecReadReg(BaseAddress, XSDFEC_CORE_AXIS_ENABLE_ADDR);
}

void XSdFecSet_CORE_ORDER(UINTPTR BaseAddress, u32 Data) {
  XSdFecWriteReg(BaseAddress, XSDFEC_CORE_ORDER_ADDR, Data);
}

u32 XSdFecGet_CORE_ORDER(UINTPTR BaseAddress) {
  return XSdFecReadReg(BaseAddress, XSDFEC_CORE_ORDER_ADDR);
}

void XSdFecSet_CORE_ISR(UINTPTR BaseAddress, u32 Data) {
  XSdFecWriteReg(BaseAddress, XSDFEC_CORE_ISR_ADDR, Data);
}

u32 XSdFecGet_CORE_ISR(UINTPTR BaseAddress) {
  return XSdFecReadReg(BaseAddress, XSDFEC_CORE_ISR_ADDR);
}

void XSdFecSet_CORE_IER(UINTPTR BaseAddress, u32 Data) {
  XSdFecWriteReg(BaseAddress, XSDFEC_CORE_IER_ADDR, Data);
}

void XSdFecSet_CORE_IDR(UINTPTR BaseAddress, u32 Data) {
  XSdFecWriteReg(BaseAddress, XSDFEC_CORE_IDR_ADDR, Data);
}

u32 XSdFecGet_CORE_IMR(UINTPTR BaseAddress) {
  return XSdFecReadReg(BaseAddress, XSDFEC_CORE_IMR_ADDR);
}

void XSdFecSet_CORE_ECC_ISR(UINTPTR BaseAddress, u32 Data) {
  XSdFecWriteReg(BaseAddress, XSDFEC_CORE_ECC_ISR_ADDR, Data);
}

u32 XSdFecGet_CORE_ECC_ISR(UINTPTR BaseAddress) {
  return XSdFecReadReg(BaseAddress, XSDFEC_CORE_ECC_ISR_ADDR);
}

void XSdFecSet_CORE_ECC_IER(UINTPTR BaseAddress, u32 Data) {
  XSdFecWriteReg(BaseAddress, XSDFEC_CORE_ECC_IER_ADDR, Data);
}

void XSdFecSet_CORE_ECC_IDR(UINTPTR BaseAddress, u32 Data) {
  XSdFecWriteReg(BaseAddress, XSDFEC_CORE_ECC_IDR_ADDR, Data);
}

u32 XSdFecGet_CORE_ECC_IMR(UINTPTR BaseAddress) {
  return XSdFecReadReg(BaseAddress, XSDFEC_CORE_ECC_IMR_ADDR);
}

void XSdFecSet_CORE_BYPASS(UINTPTR BaseAddress, u32 Data) {
  XSdFecWriteReg(BaseAddress, XSDFEC_CORE_BYPASS_ADDR, Data);
}

u32 XSdFecGet_CORE_BYPASS(UINTPTR BaseAddress) {
  return XSdFecReadReg(BaseAddress, XSDFEC_CORE_BYPASS_ADDR);
}

u32 XSdFecGet_CORE_VERSION(UINTPTR BaseAddress) {
  return XSdFecReadReg(BaseAddress, XSDFEC_CORE_VERSION_ADDR);
}

void XSdFecSet_TURBO_ALG(UINTPTR BaseAddress, u32 Data) {
  u32 rdata = XSdFecReadReg(BaseAddress, XSDFEC_TURBO_ADDR);
  rdata = rdata & ~XSDFEC_TURBO_ALG_MASK;
  u32 wdata = (rdata & ~XSDFEC_TURBO_ALG_MASK) | (XSDFEC_TURBO_ALG_MASK & (Data << XSDFEC_TURBO_ALG_LSB));
  XSdFecWriteReg(BaseAddress, XSDFEC_TURBO_ADDR, wdata);
}

u32 XSdFecGet_TURBO_ALG(UINTPTR BaseAddress) {
  u32 rdata = XSdFecReadReg(BaseAddress, XSDFEC_TURBO_ADDR);
  return (rdata & XSDFEC_TURBO_ALG_MASK) >> XSDFEC_TURBO_ALG_LSB;
}

void XSdFecSet_TURBO_SCALE_FACTOR(UINTPTR BaseAddress, u32 Data) {
  u32 rdata = XSdFecReadReg(BaseAddress, XSDFEC_TURBO_ADDR);
  rdata = rdata & ~XSDFEC_TURBO_SCALE_FACTOR_MASK;
  u32 wdata = (rdata & ~XSDFEC_TURBO_SCALE_FACTOR_MASK) | (XSDFEC_TURBO_SCALE_FACTOR_MASK & (Data << XSDFEC_TURBO_SCALE_FACTOR_LSB));
  XSdFecWriteReg(BaseAddress, XSDFEC_TURBO_ADDR, wdata);
}

u32 XSdFecGet_TURBO_SCALE_FACTOR(UINTPTR BaseAddress) {
  u32 rdata = XSdFecReadReg(BaseAddress, XSDFEC_TURBO_ADDR);
  return (rdata & XSDFEC_TURBO_SCALE_FACTOR_MASK) >> XSDFEC_TURBO_SCALE_FACTOR_LSB;
}

void XSdFecSet_TURBO(UINTPTR BaseAddress, u32 Data) {
  XSdFecWriteReg(BaseAddress, XSDFEC_TURBO_ADDR, Data);
}

u32 XSdFecGet_TURBO(UINTPTR BaseAddress) {
  return XSdFecReadReg(BaseAddress, XSDFEC_TURBO_ADDR);
}

u32 XSdFecWrite_LDPC_CODE_REG0_N_Words(UINTPTR BaseAddress, u32 WordOffset, const u32 *DataArrayPtr, u32 NumData) {
  u32 MaxDataDepth = (WordOffset + NumData)*4;
  if ( MaxDataDepth > XSDFEC_LDPC_CODE_REG0_DEPTH) {
    return 0;
  }
  u32 idx;
  for(idx = 0; idx < NumData; idx++) {
    u32 addr =  XSDFEC_LDPC_CODE_REG0_ADDR_BASE+(WordOffset + idx)*4*4;
    u32 rdata = XSdFecReadReg(BaseAddress, addr);
    u32 wdata = (rdata & ~XSDFEC_LDPC_CODE_REG0_N_MASK) | (XSDFEC_LDPC_CODE_REG0_N_MASK & (DataArrayPtr[idx] << XSDFEC_LDPC_CODE_REG0_N_LSB));
    XSdFecWriteReg(BaseAddress, addr, wdata);
  }
  return NumData;
}

u32 XSdFecRead_LDPC_CODE_REG0_N_Words(UINTPTR BaseAddress, u32 WordOffset, u32 *DataArrayPtr, u32 NumData) {
  u32 MaxDataDepth = (WordOffset + NumData)*4;
  if (MaxDataDepth > XSDFEC_LDPC_CODE_REG0_DEPTH) {
    return 0;
  }
  u32 idx;
  for(idx = 0; idx < NumData; idx++) {
    u32 addr =  XSDFEC_LDPC_CODE_REG0_ADDR_BASE+(WordOffset + idx)*4*4;
    u32 rdata = XSdFecReadReg(BaseAddress, addr);
    DataArrayPtr[idx] = (rdata & XSDFEC_LDPC_CODE_REG0_N_MASK) >> XSDFEC_LDPC_CODE_REG0_N_LSB;
  }
  return NumData;
}

u32 XSdFecWrite_LDPC_CODE_REG0_K_Words(UINTPTR BaseAddress, u32 WordOffset, const u32 *DataArrayPtr, u32 NumData) {
  u32 MaxDataDepth = (WordOffset + NumData)*4;
  if ( MaxDataDepth > XSDFEC_LDPC_CODE_REG0_DEPTH) {
    return 0;
  }
  u32 idx;
  for(idx = 0; idx < NumData; idx++) {
    u32 addr =  XSDFEC_LDPC_CODE_REG0_ADDR_BASE+(WordOffset + idx)*4*4;
    u32 rdata = XSdFecReadReg(BaseAddress, addr);
    u32 wdata = (rdata & ~XSDFEC_LDPC_CODE_REG0_K_MASK) | (XSDFEC_LDPC_CODE_REG0_K_MASK & (DataArrayPtr[idx] << XSDFEC_LDPC_CODE_REG0_K_LSB));
    XSdFecWriteReg(BaseAddress, addr, wdata);
  }
  return NumData;
}

u32 XSdFecRead_LDPC_CODE_REG0_K_Words(UINTPTR BaseAddress, u32 WordOffset, u32 *DataArrayPtr, u32 NumData) {
  u32 MaxDataDepth = (WordOffset + NumData)*4;
  if (MaxDataDepth > XSDFEC_LDPC_CODE_REG0_DEPTH) {
    return 0;
  }
  u32 idx;
  for(idx = 0; idx < NumData; idx++) {
    u32 addr =  XSDFEC_LDPC_CODE_REG0_ADDR_BASE+(WordOffset + idx)*4*4;
    u32 rdata = XSdFecReadReg(BaseAddress, addr);
    DataArrayPtr[idx] = (rdata & XSDFEC_LDPC_CODE_REG0_K_MASK) >> XSDFEC_LDPC_CODE_REG0_K_LSB;
  }
  return NumData;
}

u32 XSdFecWrite_LDPC_CODE_REG0_Words(UINTPTR BaseAddress, u32 WordOffset, const u32 *DataArrayPtr, u32 NumData) {
  u32 MaxDataDepth = (WordOffset + NumData)*4;
  if (MaxDataDepth > XSDFEC_LDPC_CODE_REG0_DEPTH) {
    return 0;
  }
  u32 idx;
  for(idx = 0; idx < NumData; idx++) {
    u32 addr =  XSDFEC_LDPC_CODE_REG0_ADDR_BASE+(WordOffset + idx)*4*4;
    XSdFecWriteReg(BaseAddress, addr, DataArrayPtr[idx]);
  }
  return NumData;
}

u32 XSdFecRead_LDPC_CODE_REG0_Words(UINTPTR BaseAddress, u32 WordOffset, u32 *DataArrayPtr, u32 NumData) {
  u32 MaxDataDepth = (WordOffset + NumData)*4;
  if (MaxDataDepth > XSDFEC_LDPC_CODE_REG0_DEPTH) {
    return 0;
  }
  u32 idx;
  for(idx = 0; idx < NumData; idx++) {
    u32 addr =  XSDFEC_LDPC_CODE_REG0_ADDR_BASE+(WordOffset + idx)*4*4;
    DataArrayPtr[idx] = XSdFecReadReg(BaseAddress, addr);
  }
  return NumData;
}

u32 XSdFecWrite_LDPC_CODE_REG1_PSIZE_Words(UINTPTR BaseAddress, u32 WordOffset, const u32 *DataArrayPtr, u32 NumData) {
  u32 MaxDataDepth = (WordOffset + NumData)*4;
  if ( MaxDataDepth > XSDFEC_LDPC_CODE_REG1_DEPTH) {
    return 0;
  }
  u32 idx;
  for(idx = 0; idx < NumData; idx++) {
    u32 addr =  XSDFEC_LDPC_CODE_REG1_ADDR_BASE+(WordOffset + idx)*4*4;
    u32 rdata = XSdFecReadReg(BaseAddress, addr);
    u32 wdata = (rdata & ~XSDFEC_LDPC_CODE_REG1_PSIZE_MASK) | (XSDFEC_LDPC_CODE_REG1_PSIZE_MASK & (DataArrayPtr[idx] << XSDFEC_LDPC_CODE_REG1_PSIZE_LSB));
    XSdFecWriteReg(BaseAddress, addr, wdata);
  }
  return NumData;
}

u32 XSdFecRead_LDPC_CODE_REG1_PSIZE_Words(UINTPTR BaseAddress, u32 WordOffset, u32 *DataArrayPtr, u32 NumData) {
  u32 MaxDataDepth = (WordOffset + NumData)*4;
  if (MaxDataDepth > XSDFEC_LDPC_CODE_REG1_DEPTH) {
    return 0;
  }
  u32 idx;
  for(idx = 0; idx < NumData; idx++) {
    u32 addr =  XSDFEC_LDPC_CODE_REG1_ADDR_BASE+(WordOffset + idx)*4*4;
    u32 rdata = XSdFecReadReg(BaseAddress, addr);
    DataArrayPtr[idx] = (rdata & XSDFEC_LDPC_CODE_REG1_PSIZE_MASK) >> XSDFEC_LDPC_CODE_REG1_PSIZE_LSB;
  }
  return NumData;
}

u32 XSdFecWrite_LDPC_CODE_REG1_NO_PACKING_Words(UINTPTR BaseAddress, u32 WordOffset, const u32 *DataArrayPtr, u32 NumData) {
  u32 MaxDataDepth = (WordOffset + NumData)*4;
  if ( MaxDataDepth > XSDFEC_LDPC_CODE_REG1_DEPTH) {
    return 0;
  }
  u32 idx;
  for(idx = 0; idx < NumData; idx++) {
    u32 addr =  XSDFEC_LDPC_CODE_REG1_ADDR_BASE+(WordOffset + idx)*4*4;
    u32 rdata = XSdFecReadReg(BaseAddress, addr);
    u32 wdata = (rdata & ~XSDFEC_LDPC_CODE_REG1_NO_PACKING_MASK) | (XSDFEC_LDPC_CODE_REG1_NO_PACKING_MASK & (DataArrayPtr[idx] << XSDFEC_LDPC_CODE_REG1_NO_PACKING_LSB));
    XSdFecWriteReg(BaseAddress, addr, wdata);
  }
  return NumData;
}

u32 XSdFecRead_LDPC_CODE_REG1_NO_PACKING_Words(UINTPTR BaseAddress, u32 WordOffset, u32 *DataArrayPtr, u32 NumData) {
  u32 MaxDataDepth = (WordOffset + NumData)*4;
  if (MaxDataDepth > XSDFEC_LDPC_CODE_REG1_DEPTH) {
    return 0;
  }
  u32 idx;
  for(idx = 0; idx < NumData; idx++) {
    u32 addr =  XSDFEC_LDPC_CODE_REG1_ADDR_BASE+(WordOffset + idx)*4*4;
    u32 rdata = XSdFecReadReg(BaseAddress, addr);
    DataArrayPtr[idx] = (rdata & XSDFEC_LDPC_CODE_REG1_NO_PACKING_MASK) >> XSDFEC_LDPC_CODE_REG1_NO_PACKING_LSB;
  }
  return NumData;
}

u32 XSdFecWrite_LDPC_CODE_REG1_NM_Words(UINTPTR BaseAddress, u32 WordOffset, const u32 *DataArrayPtr, u32 NumData) {
  u32 MaxDataDepth = (WordOffset + NumData)*4;
  if ( MaxDataDepth > XSDFEC_LDPC_CODE_REG1_DEPTH) {
    return 0;
  }
  u32 idx;
  for(idx = 0; idx < NumData; idx++) {
    u32 addr =  XSDFEC_LDPC_CODE_REG1_ADDR_BASE+(WordOffset + idx)*4*4;
    u32 rdata = XSdFecReadReg(BaseAddress, addr);
    u32 wdata = (rdata & ~XSDFEC_LDPC_CODE_REG1_NM_MASK) | (XSDFEC_LDPC_CODE_REG1_NM_MASK & (DataArrayPtr[idx] << XSDFEC_LDPC_CODE_REG1_NM_LSB));
    XSdFecWriteReg(BaseAddress, addr, wdata);
  }
  return NumData;
}

u32 XSdFecRead_LDPC_CODE_REG1_NM_Words(UINTPTR BaseAddress, u32 WordOffset, u32 *DataArrayPtr, u32 NumData) {
  u32 MaxDataDepth = (WordOffset + NumData)*4;
  if (MaxDataDepth > XSDFEC_LDPC_CODE_REG1_DEPTH) {
    return 0;
  }
  u32 idx;
  for(idx = 0; idx < NumData; idx++) {
    u32 addr =  XSDFEC_LDPC_CODE_REG1_ADDR_BASE+(WordOffset + idx)*4*4;
    u32 rdata = XSdFecReadReg(BaseAddress, addr);
    DataArrayPtr[idx] = (rdata & XSDFEC_LDPC_CODE_REG1_NM_MASK) >> XSDFEC_LDPC_CODE_REG1_NM_LSB;
  }
  return NumData;
}

u32 XSdFecWrite_LDPC_CODE_REG1_Words(UINTPTR BaseAddress, u32 WordOffset, const u32 *DataArrayPtr, u32 NumData) {
  u32 MaxDataDepth = (WordOffset + NumData)*4;
  if (MaxDataDepth > XSDFEC_LDPC_CODE_REG1_DEPTH) {
    return 0;
  }
  u32 idx;
  for(idx = 0; idx < NumData; idx++) {
    u32 addr =  XSDFEC_LDPC_CODE_REG1_ADDR_BASE+(WordOffset + idx)*4*4;
    XSdFecWriteReg(BaseAddress, addr, DataArrayPtr[idx]);
  }
  return NumData;
}

u32 XSdFecRead_LDPC_CODE_REG1_Words(UINTPTR BaseAddress, u32 WordOffset, u32 *DataArrayPtr, u32 NumData) {
  u32 MaxDataDepth = (WordOffset + NumData)*4;
  if (MaxDataDepth > XSDFEC_LDPC_CODE_REG1_DEPTH) {
    return 0;
  }
  u32 idx;
  for(idx = 0; idx < NumData; idx++) {
    u32 addr =  XSDFEC_LDPC_CODE_REG1_ADDR_BASE+(WordOffset + idx)*4*4;
    DataArrayPtr[idx] = XSdFecReadReg(BaseAddress, addr);
  }
  return NumData;
}

u32 XSdFecWrite_LDPC_CODE_REG2_NLAYERS_Words(UINTPTR BaseAddress, u32 WordOffset, const u32 *DataArrayPtr, u32 NumData) {
  u32 MaxDataDepth = (WordOffset + NumData)*4;
  if ( MaxDataDepth > XSDFEC_LDPC_CODE_REG2_DEPTH) {
    return 0;
  }
  u32 idx;
  for(idx = 0; idx < NumData; idx++) {
    u32 addr =  XSDFEC_LDPC_CODE_REG2_ADDR_BASE+(WordOffset + idx)*4*4;
    u32 rdata = XSdFecReadReg(BaseAddress, addr);
    u32 wdata = (rdata & ~XSDFEC_LDPC_CODE_REG2_NLAYERS_MASK) | (XSDFEC_LDPC_CODE_REG2_NLAYERS_MASK & (DataArrayPtr[idx] << XSDFEC_LDPC_CODE_REG2_NLAYERS_LSB));
    XSdFecWriteReg(BaseAddress, addr, wdata);
  }
  return NumData;
}

u32 XSdFecRead_LDPC_CODE_REG2_NLAYERS_Words(UINTPTR BaseAddress, u32 WordOffset, u32 *DataArrayPtr, u32 NumData) {
  u32 MaxDataDepth = (WordOffset + NumData)*4;
  if (MaxDataDepth > XSDFEC_LDPC_CODE_REG2_DEPTH) {
    return 0;
  }
  u32 idx;
  for(idx = 0; idx < NumData; idx++) {
    u32 addr =  XSDFEC_LDPC_CODE_REG2_ADDR_BASE+(WordOffset + idx)*4*4;
    u32 rdata = XSdFecReadReg(BaseAddress, addr);
    DataArrayPtr[idx] = (rdata & XSDFEC_LDPC_CODE_REG2_NLAYERS_MASK) >> XSDFEC_LDPC_CODE_REG2_NLAYERS_LSB;
  }
  return NumData;
}

u32 XSdFecWrite_LDPC_CODE_REG2_NMQC_Words(UINTPTR BaseAddress, u32 WordOffset, const u32 *DataArrayPtr, u32 NumData) {
  u32 MaxDataDepth = (WordOffset + NumData)*4;
  if ( MaxDataDepth > XSDFEC_LDPC_CODE_REG2_DEPTH) {
    return 0;
  }
  u32 idx;
  for(idx = 0; idx < NumData; idx++) {
    u32 addr =  XSDFEC_LDPC_CODE_REG2_ADDR_BASE+(WordOffset + idx)*4*4;
    u32 rdata = XSdFecReadReg(BaseAddress, addr);
    u32 wdata = (rdata & ~XSDFEC_LDPC_CODE_REG2_NMQC_MASK) | (XSDFEC_LDPC_CODE_REG2_NMQC_MASK & (DataArrayPtr[idx] << XSDFEC_LDPC_CODE_REG2_NMQC_LSB));
    XSdFecWriteReg(BaseAddress, addr, wdata);
  }
  return NumData;
}

u32 XSdFecRead_LDPC_CODE_REG2_NMQC_Words(UINTPTR BaseAddress, u32 WordOffset, u32 *DataArrayPtr, u32 NumData) {
  u32 MaxDataDepth = (WordOffset + NumData)*4;
  if (MaxDataDepth > XSDFEC_LDPC_CODE_REG2_DEPTH) {
    return 0;
  }
  u32 idx;
  for(idx = 0; idx < NumData; idx++) {
    u32 addr =  XSDFEC_LDPC_CODE_REG2_ADDR_BASE+(WordOffset + idx)*4*4;
    u32 rdata = XSdFecReadReg(BaseAddress, addr);
    DataArrayPtr[idx] = (rdata & XSDFEC_LDPC_CODE_REG2_NMQC_MASK) >> XSDFEC_LDPC_CODE_REG2_NMQC_LSB;
  }
  return NumData;
}

u32 XSdFecWrite_LDPC_CODE_REG2_NORM_TYPE_Words(UINTPTR BaseAddress, u32 WordOffset, const u32 *DataArrayPtr, u32 NumData) {
  u32 MaxDataDepth = (WordOffset + NumData)*4;
  if ( MaxDataDepth > XSDFEC_LDPC_CODE_REG2_DEPTH) {
    return 0;
  }
  u32 idx;
  for(idx = 0; idx < NumData; idx++) {
    u32 addr =  XSDFEC_LDPC_CODE_REG2_ADDR_BASE+(WordOffset + idx)*4*4;
    u32 rdata = XSdFecReadReg(BaseAddress, addr);
    u32 wdata = (rdata & ~XSDFEC_LDPC_CODE_REG2_NORM_TYPE_MASK) | (XSDFEC_LDPC_CODE_REG2_NORM_TYPE_MASK & (DataArrayPtr[idx] << XSDFEC_LDPC_CODE_REG2_NORM_TYPE_LSB));
    XSdFecWriteReg(BaseAddress, addr, wdata);
  }
  return NumData;
}

u32 XSdFecRead_LDPC_CODE_REG2_NORM_TYPE_Words(UINTPTR BaseAddress, u32 WordOffset, u32 *DataArrayPtr, u32 NumData) {
  u32 MaxDataDepth = (WordOffset + NumData)*4;
  if (MaxDataDepth > XSDFEC_LDPC_CODE_REG2_DEPTH) {
    return 0;
  }
  u32 idx;
  for(idx = 0; idx < NumData; idx++) {
    u32 addr =  XSDFEC_LDPC_CODE_REG2_ADDR_BASE+(WordOffset + idx)*4*4;
    u32 rdata = XSdFecReadReg(BaseAddress, addr);
    DataArrayPtr[idx] = (rdata & XSDFEC_LDPC_CODE_REG2_NORM_TYPE_MASK) >> XSDFEC_LDPC_CODE_REG2_NORM_TYPE_LSB;
  }
  return NumData;
}

u32 XSdFecWrite_LDPC_CODE_REG2_SPECIAL_QC_Words(UINTPTR BaseAddress, u32 WordOffset, const u32 *DataArrayPtr, u32 NumData) {
  u32 MaxDataDepth = (WordOffset + NumData)*4;
  if ( MaxDataDepth > XSDFEC_LDPC_CODE_REG2_DEPTH) {
    return 0;
  }
  u32 idx;
  for(idx = 0; idx < NumData; idx++) {
    u32 addr =  XSDFEC_LDPC_CODE_REG2_ADDR_BASE+(WordOffset + idx)*4*4;
    u32 rdata = XSdFecReadReg(BaseAddress, addr);
    u32 wdata = (rdata & ~XSDFEC_LDPC_CODE_REG2_SPECIAL_QC_MASK) | (XSDFEC_LDPC_CODE_REG2_SPECIAL_QC_MASK & (DataArrayPtr[idx] << XSDFEC_LDPC_CODE_REG2_SPECIAL_QC_LSB));
    XSdFecWriteReg(BaseAddress, addr, wdata);
  }
  return NumData;
}

u32 XSdFecRead_LDPC_CODE_REG2_SPECIAL_QC_Words(UINTPTR BaseAddress, u32 WordOffset, u32 *DataArrayPtr, u32 NumData) {
  u32 MaxDataDepth = (WordOffset + NumData)*4;
  if (MaxDataDepth > XSDFEC_LDPC_CODE_REG2_DEPTH) {
    return 0;
  }
  u32 idx;
  for(idx = 0; idx < NumData; idx++) {
    u32 addr =  XSDFEC_LDPC_CODE_REG2_ADDR_BASE+(WordOffset + idx)*4*4;
    u32 rdata = XSdFecReadReg(BaseAddress, addr);
    DataArrayPtr[idx] = (rdata & XSDFEC_LDPC_CODE_REG2_SPECIAL_QC_MASK) >> XSDFEC_LDPC_CODE_REG2_SPECIAL_QC_LSB;
  }
  return NumData;
}

u32 XSdFecWrite_LDPC_CODE_REG2_NO_FINAL_PARITY_CHECK_Words(UINTPTR BaseAddress, u32 WordOffset, const u32 *DataArrayPtr, u32 NumData) {
  u32 MaxDataDepth = (WordOffset + NumData)*4;
  if ( MaxDataDepth > XSDFEC_LDPC_CODE_REG2_DEPTH) {
    return 0;
  }
  u32 idx;
  for(idx = 0; idx < NumData; idx++) {
    u32 addr =  XSDFEC_LDPC_CODE_REG2_ADDR_BASE+(WordOffset + idx)*4*4;
    u32 rdata = XSdFecReadReg(BaseAddress, addr);
    u32 wdata = (rdata & ~XSDFEC_LDPC_CODE_REG2_NO_FINAL_PARITY_CHECK_MASK) | (XSDFEC_LDPC_CODE_REG2_NO_FINAL_PARITY_CHECK_MASK & (DataArrayPtr[idx] << XSDFEC_LDPC_CODE_REG2_NO_FINAL_PARITY_CHECK_LSB));
    XSdFecWriteReg(BaseAddress, addr, wdata);
  }
  return NumData;
}

u32 XSdFecRead_LDPC_CODE_REG2_NO_FINAL_PARITY_CHECK_Words(UINTPTR BaseAddress, u32 WordOffset, u32 *DataArrayPtr, u32 NumData) {
  u32 MaxDataDepth = (WordOffset + NumData)*4;
  if (MaxDataDepth > XSDFEC_LDPC_CODE_REG2_DEPTH) {
    return 0;
  }
  u32 idx;
  for(idx = 0; idx < NumData; idx++) {
    u32 addr =  XSDFEC_LDPC_CODE_REG2_ADDR_BASE+(WordOffset + idx)*4*4;
    u32 rdata = XSdFecReadReg(BaseAddress, addr);
    DataArrayPtr[idx] = (rdata & XSDFEC_LDPC_CODE_REG2_NO_FINAL_PARITY_CHECK_MASK) >> XSDFEC_LDPC_CODE_REG2_NO_FINAL_PARITY_CHECK_LSB;
  }
  return NumData;
}

u32 XSdFecWrite_LDPC_CODE_REG2_MAX_SCHEDULE_Words(UINTPTR BaseAddress, u32 WordOffset, const u32 *DataArrayPtr, u32 NumData) {
  u32 MaxDataDepth = (WordOffset + NumData)*4;
  if ( MaxDataDepth > XSDFEC_LDPC_CODE_REG2_DEPTH) {
    return 0;
  }
  u32 idx;
  for(idx = 0; idx < NumData; idx++) {
    u32 addr =  XSDFEC_LDPC_CODE_REG2_ADDR_BASE+(WordOffset + idx)*4*4;
    u32 rdata = XSdFecReadReg(BaseAddress, addr);
    u32 wdata = (rdata & ~XSDFEC_LDPC_CODE_REG2_MAX_SCHEDULE_MASK) | (XSDFEC_LDPC_CODE_REG2_MAX_SCHEDULE_MASK & (DataArrayPtr[idx] << XSDFEC_LDPC_CODE_REG2_MAX_SCHEDULE_LSB));
    XSdFecWriteReg(BaseAddress, addr, wdata);
  }
  return NumData;
}

u32 XSdFecRead_LDPC_CODE_REG2_MAX_SCHEDULE_Words(UINTPTR BaseAddress, u32 WordOffset, u32 *DataArrayPtr, u32 NumData) {
  u32 MaxDataDepth = (WordOffset + NumData)*4;
  if (MaxDataDepth > XSDFEC_LDPC_CODE_REG2_DEPTH) {
    return 0;
  }
  u32 idx;
  for(idx = 0; idx < NumData; idx++) {
    u32 addr =  XSDFEC_LDPC_CODE_REG2_ADDR_BASE+(WordOffset + idx)*4*4;
    u32 rdata = XSdFecReadReg(BaseAddress, addr);
    DataArrayPtr[idx] = (rdata & XSDFEC_LDPC_CODE_REG2_MAX_SCHEDULE_MASK) >> XSDFEC_LDPC_CODE_REG2_MAX_SCHEDULE_LSB;
  }
  return NumData;
}

u32 XSdFecWrite_LDPC_CODE_REG2_Words(UINTPTR BaseAddress, u32 WordOffset, const u32 *DataArrayPtr, u32 NumData) {
  u32 MaxDataDepth = (WordOffset + NumData)*4;
  if (MaxDataDepth > XSDFEC_LDPC_CODE_REG2_DEPTH) {
    return 0;
  }
  u32 idx;
  for(idx = 0; idx < NumData; idx++) {
    u32 addr =  XSDFEC_LDPC_CODE_REG2_ADDR_BASE+(WordOffset + idx)*4*4;
    XSdFecWriteReg(BaseAddress, addr, DataArrayPtr[idx]);
  }
  return NumData;
}

u32 XSdFecRead_LDPC_CODE_REG2_Words(UINTPTR BaseAddress, u32 WordOffset, u32 *DataArrayPtr, u32 NumData) {
  u32 MaxDataDepth = (WordOffset + NumData)*4;
  if (MaxDataDepth > XSDFEC_LDPC_CODE_REG2_DEPTH) {
    return 0;
  }
  u32 idx;
  for(idx = 0; idx < NumData; idx++) {
    u32 addr =  XSDFEC_LDPC_CODE_REG2_ADDR_BASE+(WordOffset + idx)*4*4;
    DataArrayPtr[idx] = XSdFecReadReg(BaseAddress, addr);
  }
  return NumData;
}

u32 XSdFecWrite_LDPC_CODE_REG3_SC_OFF_Words(UINTPTR BaseAddress, u32 WordOffset, const u32 *DataArrayPtr, u32 NumData) {
  u32 MaxDataDepth = (WordOffset + NumData)*4;
  if ( MaxDataDepth > XSDFEC_LDPC_CODE_REG3_DEPTH) {
    return 0;
  }
  u32 idx;
  for(idx = 0; idx < NumData; idx++) {
    u32 addr =  XSDFEC_LDPC_CODE_REG3_ADDR_BASE+(WordOffset + idx)*4*4;
    u32 rdata = XSdFecReadReg(BaseAddress, addr);
    u32 wdata = (rdata & ~XSDFEC_LDPC_CODE_REG3_SC_OFF_MASK) | (XSDFEC_LDPC_CODE_REG3_SC_OFF_MASK & (DataArrayPtr[idx] << XSDFEC_LDPC_CODE_REG3_SC_OFF_LSB));
    XSdFecWriteReg(BaseAddress, addr, wdata);
  }
  return NumData;
}

u32 XSdFecRead_LDPC_CODE_REG3_SC_OFF_Words(UINTPTR BaseAddress, u32 WordOffset, u32 *DataArrayPtr, u32 NumData) {
  u32 MaxDataDepth = (WordOffset + NumData)*4;
  if (MaxDataDepth > XSDFEC_LDPC_CODE_REG3_DEPTH) {
    return 0;
  }
  u32 idx;
  for(idx = 0; idx < NumData; idx++) {
    u32 addr =  XSDFEC_LDPC_CODE_REG3_ADDR_BASE+(WordOffset + idx)*4*4;
    u32 rdata = XSdFecReadReg(BaseAddress, addr);
    DataArrayPtr[idx] = (rdata & XSDFEC_LDPC_CODE_REG3_SC_OFF_MASK) >> XSDFEC_LDPC_CODE_REG3_SC_OFF_LSB;
  }
  return NumData;
}

u32 XSdFecWrite_LDPC_CODE_REG3_LA_OFF_Words(UINTPTR BaseAddress, u32 WordOffset, const u32 *DataArrayPtr, u32 NumData) {
  u32 MaxDataDepth = (WordOffset + NumData)*4;
  if ( MaxDataDepth > XSDFEC_LDPC_CODE_REG3_DEPTH) {
    return 0;
  }
  u32 idx;
  for(idx = 0; idx < NumData; idx++) {
    u32 addr =  XSDFEC_LDPC_CODE_REG3_ADDR_BASE+(WordOffset + idx)*4*4;
    u32 rdata = XSdFecReadReg(BaseAddress, addr);
    u32 wdata = (rdata & ~XSDFEC_LDPC_CODE_REG3_LA_OFF_MASK) | (XSDFEC_LDPC_CODE_REG3_LA_OFF_MASK & (DataArrayPtr[idx] << XSDFEC_LDPC_CODE_REG3_LA_OFF_LSB));
    XSdFecWriteReg(BaseAddress, addr, wdata);
  }
  return NumData;
}

u32 XSdFecRead_LDPC_CODE_REG3_LA_OFF_Words(UINTPTR BaseAddress, u32 WordOffset, u32 *DataArrayPtr, u32 NumData) {
  u32 MaxDataDepth = (WordOffset + NumData)*4;
  if (MaxDataDepth > XSDFEC_LDPC_CODE_REG3_DEPTH) {
    return 0;
  }
  u32 idx;
  for(idx = 0; idx < NumData; idx++) {
    u32 addr =  XSDFEC_LDPC_CODE_REG3_ADDR_BASE+(WordOffset + idx)*4*4;
    u32 rdata = XSdFecReadReg(BaseAddress, addr);
    DataArrayPtr[idx] = (rdata & XSDFEC_LDPC_CODE_REG3_LA_OFF_MASK) >> XSDFEC_LDPC_CODE_REG3_LA_OFF_LSB;
  }
  return NumData;
}

u32 XSdFecWrite_LDPC_CODE_REG3_QC_OFF_Words(UINTPTR BaseAddress, u32 WordOffset, const u32 *DataArrayPtr, u32 NumData) {
  u32 MaxDataDepth = (WordOffset + NumData)*4;
  if ( MaxDataDepth > XSDFEC_LDPC_CODE_REG3_DEPTH) {
    return 0;
  }
  u32 idx;
  for(idx = 0; idx < NumData; idx++) {
    u32 addr =  XSDFEC_LDPC_CODE_REG3_ADDR_BASE+(WordOffset + idx)*4*4;
    u32 rdata = XSdFecReadReg(BaseAddress, addr);
    u32 wdata = (rdata & ~XSDFEC_LDPC_CODE_REG3_QC_OFF_MASK) | (XSDFEC_LDPC_CODE_REG3_QC_OFF_MASK & (DataArrayPtr[idx] << XSDFEC_LDPC_CODE_REG3_QC_OFF_LSB));
    XSdFecWriteReg(BaseAddress, addr, wdata);
  }
  return NumData;
}

u32 XSdFecRead_LDPC_CODE_REG3_QC_OFF_Words(UINTPTR BaseAddress, u32 WordOffset, u32 *DataArrayPtr, u32 NumData) {
  u32 MaxDataDepth = (WordOffset + NumData)*4;
  if (MaxDataDepth > XSDFEC_LDPC_CODE_REG3_DEPTH) {
    return 0;
  }
  u32 idx;
  for(idx = 0; idx < NumData; idx++) {
    u32 addr =  XSDFEC_LDPC_CODE_REG3_ADDR_BASE+(WordOffset + idx)*4*4;
    u32 rdata = XSdFecReadReg(BaseAddress, addr);
    DataArrayPtr[idx] = (rdata & XSDFEC_LDPC_CODE_REG3_QC_OFF_MASK) >> XSDFEC_LDPC_CODE_REG3_QC_OFF_LSB;
  }
  return NumData;
}

u32 XSdFecWrite_LDPC_CODE_REG3_Words(UINTPTR BaseAddress, u32 WordOffset, const u32 *DataArrayPtr, u32 NumData) {
  u32 MaxDataDepth = (WordOffset + NumData)*4;
  if (MaxDataDepth > XSDFEC_LDPC_CODE_REG3_DEPTH) {
    return 0;
  }
  u32 idx;
  for(idx = 0; idx < NumData; idx++) {
    u32 addr =  XSDFEC_LDPC_CODE_REG3_ADDR_BASE+(WordOffset + idx)*4*4;
    XSdFecWriteReg(BaseAddress, addr, DataArrayPtr[idx]);
  }
  return NumData;
}

u32 XSdFecRead_LDPC_CODE_REG3_Words(UINTPTR BaseAddress, u32 WordOffset, u32 *DataArrayPtr, u32 NumData) {
  u32 MaxDataDepth = (WordOffset + NumData)*4;
  if (MaxDataDepth > XSDFEC_LDPC_CODE_REG3_DEPTH) {
    return 0;
  }
  u32 idx;
  for(idx = 0; idx < NumData; idx++) {
    u32 addr =  XSDFEC_LDPC_CODE_REG3_ADDR_BASE+(WordOffset + idx)*4*4;
    DataArrayPtr[idx] = XSdFecReadReg(BaseAddress, addr);
  }
  return NumData;
}

u32 XSdFecWrite_LDPC_SC_TABLE_Words(UINTPTR BaseAddress, u32 WordOffset, const u32 *DataArrayPtr, u32 NumData) {
  u32 MaxDataDepth = (WordOffset + NumData)*4;
  if (MaxDataDepth > XSDFEC_LDPC_SC_TABLE_DEPTH) {
    return 0;
  }
  u32 idx;
  for(idx = 0; idx < NumData; idx++) {
    u32 addr =  XSDFEC_LDPC_SC_TABLE_ADDR_BASE+(WordOffset + idx)*4;
    XSdFecWriteReg(BaseAddress, addr, DataArrayPtr[idx]);
  }
  return NumData;
}

u32 XSdFecRead_LDPC_SC_TABLE_Words(UINTPTR BaseAddress, u32 WordOffset, u32 *DataArrayPtr, u32 NumData) {
  u32 MaxDataDepth = (WordOffset + NumData)*4;
  if (MaxDataDepth > XSDFEC_LDPC_SC_TABLE_DEPTH) {
    return 0;
  }
  u32 idx;
  for(idx = 0; idx < NumData; idx++) {
    u32 addr =  XSDFEC_LDPC_SC_TABLE_ADDR_BASE+(WordOffset + idx)*4;
    DataArrayPtr[idx] = XSdFecReadReg(BaseAddress, addr);
  }
  return NumData;
}

u32 XSdFecWrite_LDPC_LA_TABLE_Words(UINTPTR BaseAddress, u32 WordOffset, const u32 *DataArrayPtr, u32 NumData) {
  u32 MaxDataDepth = (WordOffset + NumData)*4;
  if (MaxDataDepth > XSDFEC_LDPC_LA_TABLE_DEPTH) {
    return 0;
  }
  u32 idx;
  for(idx = 0; idx < NumData; idx++) {
    u32 addr =  XSDFEC_LDPC_LA_TABLE_ADDR_BASE+(WordOffset + idx)*4;
    XSdFecWriteReg(BaseAddress, addr, DataArrayPtr[idx]);
  }
  return NumData;
}

u32 XSdFecRead_LDPC_LA_TABLE_Words(UINTPTR BaseAddress, u32 WordOffset, u32 *DataArrayPtr, u32 NumData) {
  u32 MaxDataDepth = (WordOffset + NumData)*4;
  if (MaxDataDepth > XSDFEC_LDPC_LA_TABLE_DEPTH) {
    return 0;
  }
  u32 idx;
  for(idx = 0; idx < NumData; idx++) {
    u32 addr =  XSDFEC_LDPC_LA_TABLE_ADDR_BASE+(WordOffset + idx)*4;
    DataArrayPtr[idx] = XSdFecReadReg(BaseAddress, addr);
  }
  return NumData;
}

u32 XSdFecWrite_LDPC_QC_TABLE_Words(UINTPTR BaseAddress, u32 WordOffset, const u32 *DataArrayPtr, u32 NumData) {
  u32 MaxDataDepth = (WordOffset + NumData)*4;
  if (MaxDataDepth > XSDFEC_LDPC_QC_TABLE_DEPTH) {
    return 0;
  }
  u32 idx;
  for(idx = 0; idx < NumData; idx++) {
    u32 addr =  XSDFEC_LDPC_QC_TABLE_ADDR_BASE+(WordOffset + idx)*4;
    XSdFecWriteReg(BaseAddress, addr, DataArrayPtr[idx]);
  }
  return NumData;
}

u32 XSdFecRead_LDPC_QC_TABLE_Words(UINTPTR BaseAddress, u32 WordOffset, u32 *DataArrayPtr, u32 NumData) {
  u32 MaxDataDepth = (WordOffset + NumData)*4;
  if (MaxDataDepth > XSDFEC_LDPC_QC_TABLE_DEPTH) {
    return 0;
  }
  u32 idx;
  for(idx = 0; idx < NumData; idx++) {
    u32 addr =  XSDFEC_LDPC_QC_TABLE_ADDR_BASE+(WordOffset + idx)*4;
    DataArrayPtr[idx] = XSdFecReadReg(BaseAddress, addr);
  }
  return NumData;
}


