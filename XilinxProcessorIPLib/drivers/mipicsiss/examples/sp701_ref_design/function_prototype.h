/******************************************************************************
* Copyright (C) 2018 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/

/*****************************************************************************/
/**
*
* @file function_prototype.h
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who    Date     Changes
* ----- ------ -------- --------------------------------------------------
* X.XX  XX     YY/MM/DD
* 1.00  RHe    19/09/20 Initial release.
* </pre>
*
******************************************************************************/
/***************************** Include Files *********************************/

#include "xparameters.h"
#include "xiic.h"
#include "xil_exception.h"



/*****************************************************************************/

extern int InitIIC();

extern int AdapterWriteData(u16 ByteCount);

extern void SetupIICIntrHandlers();

extern int SetupFmcInterruptSystem(XIic *IicFmcInstPtr);

extern int SetupAdapterInterruptSystem(XIic *IicAdapterInstPtr);

extern void SendHandler(XIic *InstancePtr);

extern void ReceiveHandler(XIic *InstancePtr);

extern void StatusHandler(XIic *InstancePtr, int Event);

extern int SetFmcIICAddress();

extern int SetAdapterIICAddress();

extern void Sensor_Delay();
extern void resetIp();
extern void resetVIP();
extern void HaltVDMA();
extern int RunVDMA();

extern int SensorPreConfig(int pcam5c_mode);


extern void GPIOSelect(int);
extern int vdma_dsi();
extern int vdma_hdmi();
extern int vtpg_hdmi();
extern int getchar();
extern void DisableDSI();
extern void DisableCSI();
extern void EnableCSI();
extern void InitDSI();

extern u32 SetupDSI(void);
extern u32 InitializeCsiRxSs(void);

extern int InitIIC(void);
extern void SetupIICIntrHandlers(void);

extern int WritetoReg(u8 buf1, u8 buf2, u8 buf3);
extern int demosaic();
extern void CamReset(void);


extern void DisableCSI(void);
extern void DisableDSI(void);
extern void InitDSI(void);
extern void InitVprocSs_Scaler(int count);
extern void EnableCSI(void);
