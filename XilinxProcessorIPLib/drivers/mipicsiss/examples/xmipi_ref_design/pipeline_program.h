/******************************************************************************
* Copyright (C) 2017 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
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
extern u32 InitStreamMuxGpio(void);
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
