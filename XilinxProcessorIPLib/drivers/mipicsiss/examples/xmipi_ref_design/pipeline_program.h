/******************************************************************************
 *
 * Copyright (C) 2017 Xilinx, Inc.  All rights reserved.
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
 * Use of the Software is limited solely to applications:
 * (a) running on a Xilinx device, or
 * (b) that interact with a Xilinx device through a bus or interconnect.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * XILINX BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
 * OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 * Except as contained in this notice, the name of the Xilinx shall not be used
 * in advertising or otherwise to promote the sale, use or other dealings in
 * this Software without prior written authorization from Xilinx.
 *
 ******************************************************************************/
/*****************************************************************************/
/**
 *
 * @file pipeline_program.h
 *
 * This header file contains the definitions for structures for video pipeline
 * and extern declarations.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who    Date     Changes
 * ----- ------ -------- --------------------------------------------------
 * 1.00  pg    12/07/17 Initial release.
 * </pre>
 *
 ******************************************************************************/

#ifndef PIPELINE_PROGRAM_H_
#define PIPELINE_PROGRAM_H_

#ifdef __cplusplus
extern "C" {
#endif

extern u32 SetupDSI(void);
extern u32 InitTreadyGpio(void);
extern u32 InitializeCFA(void);
extern u32 InitializeCsiRxSs(void);
extern u32 InitializeVdma(void);

extern void SetColorDepth(void);
extern void SelectDSIOuptut(void);
extern void SelectHDMIOutput(void);

extern int InitIIC(void);
extern void SetupIICIntrHandlers(void);
extern int MuxInit(void);
extern int SetupIOExpander(void);
extern int SetupCameraSensor(void);
extern void InitDemosaicGammaCSC(void);
extern int StartSensor(void);
extern int StopSensor(void);
extern void InitCSI2RxSS2CFA_Vdma(void);
extern void InitCSC2TPG_Vdma(void);

extern int ToggleSensorPattern(void);
extern void PrintPipeConfig(void);

extern void DisableCSI(void);
extern void DisableCFAVdma(void);
extern void DisableCFA(void);
extern void DisableTPGVdma(void);
extern void DisableScaler(void);
extern void DisableDSI(void);
extern void InitDSI(void);
extern void InitVprocSs_Scaler(int count);
extern void EnableCSI(void);

#ifdef __cplusplus
}
#endif

#endif /* PIPELINE_PROGRAM_H_ */
