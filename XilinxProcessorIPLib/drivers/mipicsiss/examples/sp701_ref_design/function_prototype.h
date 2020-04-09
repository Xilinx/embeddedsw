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

#ifdef XPAR_INTC_0_DEVICE_ID
 #include "xintc.h"
#else
 #include "xscugic.h"
#endif

/************************** Constant Definitions *****************************/
/* Uncomment or comment the following depending on the board used*/
#define KC705

#ifdef KC705
	#define IIC_FMC_DEVICE_ID	XPAR_IIC_0_DEVICE_ID
	#define IIC_ADAPTER_DEVICE_ID	XPAR_IIC_1_DEVICE_ID

	#ifdef XPAR_INTC_0_DEVICE_ID
	 #define INTC_DEVICE_ID		XPAR_INTC_0_DEVICE_ID
	 #define IIC_FMC_INTR_ID	XPAR_INTC_0_IIC_0_VEC_ID
	 #define IIC_ADAPTER_INTR_ID	XPAR_INTC_0_IIC_1_VEC_ID
	 #define INTC			XIntc
	 #define INTC_HANDLER		XIntc_InterruptHandler
	#else
	 #define INTC_DEVICE_ID		XPAR_SCUGIC_SINGLE_DEVICE_ID
	 #define IIC_FMC_INTR_ID	XPAR_FABRIC_IIC_0_VEC_ID
	 #define IIC_ADAPTER_INTR_ID	XPAR_FABRIC_IIC_0_VEC_ID
	 #define INTC			XScuGic
	 #define INTC_HANDLER		XScuGic_InterruptHandler
	#endif

	#define FMC_ADDRESS 		0x3C
	#define ADAPTER_ADDRESS 	0x3C
	#define SENSOR_ADDRESS 		0x3C

	#define IIC_MUX_ADDRESS 	0x74
	#define IIC_FMC_CHANNEL		0x07

	#define IIC_MUX_ENABLE		0
	#define PAGE_SIZE   16
	#define FMC_TEST_START_ADDRESS   	128
	#define ADAPTER_TEST_START_ADDRESS   	01

	#define CSI_SS_BOARD 0
	#define CSI_SS_SENSOR 		XPAR_CSISS_0_BASEADDR
	#define CSI_TIMER_SENSOR 	XPAR_AXI_TIMER_0_BASEADDR
	#define WIDTH 1920
	#define HEIGHT 1080
	#define LANES 2
#endif


/*****************************************************************************/

extern int InitIIC();

extern int FmcWriteData(u16 ByteCount);

extern int FmcReadData(u8 *BufferPtr, u16 ByteCount);

extern int AdapterWriteData(u16 ByteCount);

extern int AdapterReadData(u8 *BufferPtr, u16 ByteCount);

extern void SetupIICIntrHandlers();

extern int SetupFmcInterruptSystem(XIic *IicFmcInstPtr);

extern int SetupAdapterInterruptSystem(XIic *IicAdapterInstPtr);

extern void SendHandler(XIic *InstancePtr);

extern void ReceiveHandler(XIic *InstancePtr);

extern void StatusHandler(XIic *InstancePtr, int Event);

extern int SetFmcIICAddress();

extern int SetAdapterIICAddress();

extern int SetSensorIICAddress();

extern void Sensor_Delay();
 extern void resetIp();
 extern void resetVIP();
 extern void HaltVDMA();
 extern void RunVDMA();

extern int SensorPreConfig(pcam5c_mode);

extern int SensorReg();

extern void GPIOSelect(int);
 extern int vdma_dsi();
 extern int vdma_hdmi();
 extern int vtpg_hdmi();
 extern getchar();
 extern void enablecsi();
 extern void DisableDSI();
 extern void DisableCSI();
 extern void EnableCSI();
 extern void InitDSI();
 extern void EnableDSI();
//extern void SetupDSI();
//extern void InitializeCsiRxSs();

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

#ifdef IIC_MUX_ENABLE
extern int MuxInit(void);
#endif
