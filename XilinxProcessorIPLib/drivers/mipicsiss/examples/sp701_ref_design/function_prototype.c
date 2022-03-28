/******************************************************************************
* Copyright (C) 2018 - 2022 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/

/*****************************************************************************/
/**
*
* @file function_prototype.c
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

/***************************** Include Files *******************************/

#include "xparameters.h"
#include "xiic.h"
#include "xil_exception.h"
#include "function_prototype.h"
#include "pcam_5C_cfgs.h"
#include "xstatus.h"
#include "xaxivdma.h"
#include "xil_printf.h"
#include "xil_types.h"
#include "xcsiss.h"
#include "xdsitxss.h"
#include "xvprocss.h"
#include "sleep.h"
#include "xbasic_types.h"
//HDMI
#include "xv_tpg.h"
#include "xvtc.h"
#include "xvidc.h"
//END
#include "xv_demosaic.h"

#ifdef XPAR_INTC_0_DEVICE_ID
 #include "xintc.h"
#else
 #include "xscugic.h"
#endif
//HDMI
XDsiTxSs DsiTxSs;
XCsiSs CsiRxSs;
XVprocSs scaler_new_inst;
XVprocSs csc_new_inst;
XAxiVdma AxiVdma;

XV_tpg_Config		*tpg1_Config;
XV_tpg				tpg1;

XVtc				vtc;
XVtc_Config			*vtc_Config;
XVtc_Timing			vtc_timing;

//END


XV_demosaic InstancePtr;
XV_demosaic_Config  *demosaic_Config;
XGpio Gpio;


/************************** Constant Definitions *****************************/





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


#define DSI_BASE XPAR_MIPI_DSI_TX_SUBSYSTEM_0_BASEADDR
#define VDMA_BASE XPAR_AXI_VDMA_0_BASEADDR
#define XCSIRXSS_DEVICE_ID      XPAR_CSISS_0_DEVICE_ID
#define XDSITXSS_DEVICE_ID      XPAR_DSITXSS_0_DEVICE_ID

#define DEMOSAIC_DEVICE_ID XPAR_XV_DEMOSAIC_0_DEVICE_ID


#define GPIO_CHANNEL 1

#define GPIO_0_DEVICE_ID XPAR_AXI_GPIO_0_DEVICE_ID
#define GPIO_1_DEVICE_ID XPAR_AXI_GPIO_1_DEVICE_ID
#define GPIO_2_DEVICE_ID XPAR_AXI_GPIO_2_DEVICE_ID
#define GPIO_3_DEVICE_ID XPAR_AXI_GPIO_3_DEVICE_ID
#define GPIO_4_DEVICE_ID XPAR_AXI_GPIO_4_DEVICE_ID





#define NO_BYTES_PER_PIXEL		3
#define HORIZONTAL_RESOLUTION		1920 //1280

#define HORIZONTAL_RESOLUTION_BYTES HORIZONTAL_RESOLUTION * NO_BYTES_PER_PIXEL
#define VERTICAL_RESOLUTION		1080 //720
#define FRAME_COUNTER			3


/* Debug Constants */
#define MM2S_HALT_SUCCESS	121
#define MM2S_HALT_FAILURE	122
#define S2MM_HALT_SUCCESS	131
#define S2MM_HALT_FAILURE	132


#define DSI_BYTES_PER_PIXEL     (3)
#define DSI_H_RES               (1920)
#define DSI_V_RES               (1200)
#define DSI_DISPLAY_HORI_VAL    (DSI_H_RES * DSI_BYTES_PER_PIXEL)
#define DSI_DISPLAY_VERT_VAL    (DSI_V_RES)
#define DSI_HBACK_PORCH                 (0x39D)
#define DSI_HFRONT_PORCH                (0x00B9)
#define DSI_VSYNC_WIDTH                 (0x05)
#define DSI_VBACK_PORCH                 (0x04)
#define DSI_VFRONT_PORCH                (0x03)
#define SET            (0x01)
#define VDMA_MM2S    (VDMA_BASE + 0x00)
#define VDMA_S2MM    (VDMA_BASE + 0x30)


#define TPG0_W 1920
#define TPG0_H 1080
#define SCLR_OW  1920
#define SCLR_OH  1200

#define S2MM	1
#define MM2S	2




/******************** Data structure Declarations *****************************/

typedef struct vdma_handle
{
	/* The device ID of the VDMA */
	unsigned int device_id;
	/* The state variable to keep track if the initialization is done*/
	unsigned int init_done;
	/** The XAxiVdma driver instance data. */
	XAxiVdma* InstancePtr;
	/* The XAxiVdma_DmaSetup structure contains all the necessary information to
	 * start a frame write or read. */
	XAxiVdma_DmaSetup ReadCfg;
	XAxiVdma_DmaSetup WriteCfg;
	/* Horizontal size of frame */
	unsigned int hsize;
	/* Vertical size of frame */
	unsigned int vsize;
	/* Buffer address from where read and write will be done by VDMA */
	unsigned int buffer_address;
	/* Flag to tell VDMA to interrupt on frame completion*/
	unsigned int enable_frm_cnt_intr;
	/* The counter to tell VDMA on how many frames the interrupt should happen*/
	unsigned int number_of_frame_count;
}vdma_handle;

/******************** Constant Definitions **********************************/

/*
 * Device related constants. These need to defined as per the HW system.
 */
vdma_handle vdma_context[XPAR_XAXIVDMA_NUM_INSTANCES];
static unsigned int context_init=0;

/******************* Function Prototypes ************************************/

static int ReadSetup(vdma_handle *vdma_context);
static int WriteSetup(vdma_handle *vdma_context);
static int StartTransfer(XAxiVdma *InstancePtr);


/**************************** Type Definitions *******************************/

typedef u8 AddressType;

XIic IicFmc, IicAdapter;
XIic *IicFmcInstPtr, *IicAdapterInstPtr;/* The instance of the IIC device. */
INTC IntcFmc, IntcAdapter ;
/*
 * Write buffer for writing a page.
 */
u8 WriteBuffer[sizeof(AddressType) + PAGE_SIZE];

u8 ReadBuffer[PAGE_SIZE];	/* Read buffer for reading a page. */

volatile u8 TransmitComplete;	/* Flag to check completion of Transmission */
volatile u8 ReceiveComplete;	/* Flag to check completion of Reception */

u8 FmcIicAddr;		/* Variable for storing FMC IIC address */
u8 AdapterIicAddr;	/* Variable for storing Adapter IIC address */
u8 SensorIicAddr;	/* Variable for storing Sensor IIC address */



#define VDMA_BASEADDR	XPAR_AXI_VDMA_0_BASEADDR

volatile u32 *Mm2sStatusReg = (u32*)(VDMA_BASEADDR + 0x4);
volatile u32 *S2mmStatusReg = (u32*)(VDMA_BASEADDR + 0x34);

volatile u32 *VdmaParkPtrReg = (u32 *)(VDMA_BASEADDR + 0x28);

volatile u32 *VdmaS2MMCrReg = (u32 *)(VDMA_BASEADDR + 0x30);
volatile u32 *VdmaS2MMStatusReg = (u32 *)(VDMA_BASEADDR + 0x34);
volatile u32 *VdmaS2MMVertSizeReg = (u32 *)(VDMA_BASEADDR + 0xA0);
volatile u32 *VdmaS2MMHoriSizeReg = (u32 *)(VDMA_BASEADDR + 0xA4);
volatile u32 *VdmaS2MMFrmDlrStrideReg = (u32 *)(VDMA_BASEADDR + 0xA8);

volatile u32 *VdmaS2MMFrameBuffer0Reg = (u32 *)(VDMA_BASEADDR + 0xAC);

volatile u32 *VdmaMM2SCrReg = (u32 *)(VDMA_BASEADDR + 0x00);
volatile u32 *VdmaMMS2StatusReg = (u32 *)(VDMA_BASEADDR + 0x4);
volatile u32 *VdmaMM2SVertSizeReg = (u32 *)(VDMA_BASEADDR + 0x50);
volatile u32 *VdmaMM2SHoriSizeReg = (u32 *)(VDMA_BASEADDR + 0x54);
volatile u32 *VdmaMM2SFrmDlrStrideReg = (u32 *)(VDMA_BASEADDR + 0x58);

volatile u32 *VdmaMM2SFrameBuffer0Reg = (u32 *)(VDMA_BASEADDR + 0x5C);

unsigned int srcBuffer = (0x80000000U + 0x1000000);

int driverInit()
{
  int status;

  vtc_Config = XVtc_LookupConfig(XPAR_V_TC_0_DEVICE_ID);
  if(vtc_Config == NULL)
  {
    xil_printf("ERR:: VTC device not found\r\n");
	return(XST_DEVICE_NOT_FOUND);
  }
  status = XVtc_CfgInitialize(&vtc, vtc_Config, vtc_Config->BaseAddress);
  if(status != XST_SUCCESS)
  {
	xil_printf("ERR:: VTC Initialization failed %d\r\n", status);
	return(XST_FAILURE);
  }


  tpg1_Config = XV_tpg_LookupConfig(XPAR_V_TPG_0_DEVICE_ID);
  if(tpg1_Config == NULL)
  {
	xil_printf("ERR:: TPG device not found\r\n");
	return(XST_DEVICE_NOT_FOUND);
  }
  status = XV_tpg_CfgInitialize(&tpg1, tpg1_Config, tpg1_Config->BaseAddress);
  if(status != XST_SUCCESS)
  {
    xil_printf("ERR:: TPG Initialization failed %d\r\n", status);
	return(XST_FAILURE);
  }

  return(XST_SUCCESS);

}


void videoIpConfig(XVidC_VideoMode videoMode)
{
  XVidC_VideoTiming const *timing = XVidC_GetTimingInfo(videoMode);
  u16 PixelsPerClk;


  XV_tpg_Set_height(&tpg1, timing->VActive);
  XV_tpg_Set_width(&tpg1, timing->HActive);
  XV_tpg_Set_colorFormat(&tpg1, 0);
  XV_tpg_Set_bckgndId(&tpg1, XTPG_BKGND_COLOR_BARS);
  XV_tpg_Set_ovrlayId(&tpg1, 0);
  XV_tpg_Set_enableInput(&tpg1, 1);
  XV_tpg_Set_passthruStartX(&tpg1, 0);
  XV_tpg_Set_passthruStartY(&tpg1, 0);
  XV_tpg_Set_passthruEndX(&tpg1, timing->HActive);
  XV_tpg_Set_passthruEndY(&tpg1, timing->VActive);
  XV_tpg_WriteReg(tpg1_Config->BaseAddress, XV_TPG_CTRL_ADDR_AP_CTRL, 0x81);

  PixelsPerClk = tpg1.Config.PixPerClk;

  vtc_timing.HActiveVideo  = timing->HActive/PixelsPerClk;
  vtc_timing.HFrontPorch   = timing->HFrontPorch/PixelsPerClk;
  vtc_timing.HSyncWidth    = timing->HSyncWidth/PixelsPerClk;
  vtc_timing.HBackPorch    = timing->HBackPorch/PixelsPerClk;
  vtc_timing.HSyncPolarity = timing->HSyncPolarity;
  vtc_timing.VActiveVideo  = timing->VActive;
  vtc_timing.V0FrontPorch  = timing->F0PVFrontPorch;
  vtc_timing.V0SyncWidth   = timing->F0PVSyncWidth;
  vtc_timing.V0BackPorch   = timing->F0PVBackPorch;
  vtc_timing.VSyncPolarity = timing->VSyncPolarity;
  XVtc_SetGeneratorTiming(&vtc, &vtc_timing);
  XVtc_Enable(&vtc);
  XVtc_EnableGenerator(&vtc);
  XVtc_RegUpdateEnable(&vtc);

}



/***************************************************************************/
/**
 * This function programs MIPI DSI SS with the required timing paramters.
 *
 * @return      None.
 *
 * @note        None.
 *
***************************************************************************/

u32 SetupDSI(void)
{
  XDsiTxSs_Config *DsiTxSsCfgPtr = NULL;
  u32 Status;
  u32 PixelFmt;

  DsiTxSsCfgPtr = XDsiTxSs_LookupConfig(XDSITXSS_DEVICE_ID);

  if (!DsiTxSsCfgPtr) {
    xil_printf("DSI Tx SS Device Id not found\r\n");
    return XST_FAILURE;
  }

  Status = XDsiTxSs_CfgInitialize(&DsiTxSs, DsiTxSsCfgPtr,
                        DsiTxSsCfgPtr->BaseAddr);
  if (Status != XST_SUCCESS) {
    xil_printf("DSI Tx Ss Cfg Init failed status = %d \r\n", Status);
    return Status;
  }

  PixelFmt = XDsiTxSs_GetPixelFormat(&DsiTxSs);

  if (PixelFmt != 0x3E) {
    xil_printf("DSI Pixel format is not correct ");
    switch (PixelFmt) {
      case 0x0E:
        xil_printf("Packed RGB565");
        break;
      case 0x1E:
        xil_printf("Packed RGB666");
        break;
      case 0x2E:
        xil_printf("Loosely packed RGB666");
        break;
      case 0x3E:
        xil_printf("Packed RGB888");
        break;
      case 0x0B:
        xil_printf("Compressed Pixel Stream");
        break;
      default:
        xil_printf("Invalid data type");
    }
    xil_printf("\r\n");
    xil_printf("Expected is 0x3E for RGB888\r\n");
    return XST_FAILURE;
  }

  return Status;
}

/***************************************************************************/
/**
 * This function programs MIPI CSI SS with the required timing paramters.
 *
 * @return      None.
 *
 * @note        None.
 *
 ***************************************************************************/

u32 InitializeCsiRxSs(void)
{
  u32 Status = 0;
  XCsiSs_Config *CsiRxSsCfgPtr = NULL;

  CsiRxSsCfgPtr = XCsiSs_LookupConfig(XCSIRXSS_DEVICE_ID);
  if (!CsiRxSsCfgPtr) {
    xil_printf("CSI2RxSs LookupCfg failed\r\n");
    return XST_FAILURE;
  }

  Status = XCsiSs_CfgInitialize(&CsiRxSs, CsiRxSsCfgPtr,
		                               CsiRxSsCfgPtr->BaseAddr);

  if (Status != XST_SUCCESS) {
    xil_printf("CsiRxSs Cfg init failed - %x\r\n", Status);
    return Status;
  }


  return XST_SUCCESS;

}

/***************************************************************************/
/**
 *  * This function enables MIPI CSI IP
 *   *
 *    * @return      None.
 *     *
 *      * @note        None.
 *       *
****************************************************************************/

void EnableCSI(void)
{
  XCsiSs_Activate(&CsiRxSs, XCSI_ENABLE);
}


/*****************************************************************************/
/**
 * This function programs MIPI DSI SS with the required timing paramters.
 *
 * @return	None.
 *
 * @note	None.
 *
 *****************************************************************************/

void InitDSI(void)
{
  u32 Status;
  XDsi_VideoTiming Timing = { 0 };
  /* Disable DSI core only. So removed DPHY register interface in design*/
  Status = XDsiTxSs_Activate(&DsiTxSs, XDSITXSS_DSI, XDSITXSS_DISABLE);
  Status = XDsiTxSs_Activate(&DsiTxSs, XDSITXSS_PHY, XDSITXSS_DISABLE);


  XDsiTxSs_Reset(&DsiTxSs);

  usleep(100000);
  Status = XDsiTxSs_Activate(&DsiTxSs, XDSITXSS_PHY, XDSITXSS_ENABLE);

  do {
	Status = XDsiTxSs_IsControllerReady(&DsiTxSs);
  } while (!Status);


  Timing.HActive = DSI_DISPLAY_HORI_VAL;
  Timing.VActive = DSI_DISPLAY_VERT_VAL;
  Timing.HBackPorch = DSI_HBACK_PORCH;
  Timing.HFrontPorch = DSI_HFRONT_PORCH;

  Timing.VSyncWidth = DSI_VSYNC_WIDTH;
  Timing.VBackPorch = DSI_VBACK_PORCH;
  Timing.VFrontPorch = DSI_VFRONT_PORCH;

  XDsiTxSs_SetCustomVideoInterfaceTiming(&DsiTxSs,
	                       XDSI_VM_NON_BURST_SYNC_EVENT, &Timing);
  usleep(50000);



}

/***************************************************************************/
/**
 *  * This function disables MIPI CSI IP
 *   *
 *    * @return      None.
 *     *
 *      * @note        None.
 *       *
****************************************************************************/

void DisableCSI(void)
{
  XCsiSs_Reset(&CsiRxSs);
}


/***************************************************************************/
/**
 *  * This function disables MIPI DSI SS.
 *   *
 *    * @return      None.
 *     *
 *      * @note        None.
 *       *
 ****************************************************************************/
void DisableDSI(void)
{
  XDsiTxSs_Activate(&DsiTxSs, XDSITXSS_DSI, XDSITXSS_DISABLE);
  usleep(100000);

}

/*****************************************************************************/
/**
* This function resets IPs.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void resetIp(void)
{

  DisableCSI();
  HaltVDMA();
  DisableDSI();
  resetVIP();
  xil_printf("\n\rReset Done\n\r");

}

/*
* The configuration table for devices
*/

XV_demosaic_Config XV_demosaic_ConfigTable[] =
{
	{
#ifdef XPAR_XV_DEMOSAIC_NUM_INSTANCES
		XPAR_XV_DEMOSAIC_0_DEVICE_ID,
		XPAR_XV_DEMOSAIC_0_S_AXI_CTRL_BASEADDR,
		XPAR_XV_DEMOSAIC_0_SAMPLES_PER_CLOCK,
		XPAR_XV_DEMOSAIC_0_MAX_COLS,
		XPAR_XV_DEMOSAIC_0_MAX_ROWS,
		XPAR_XV_DEMOSAIC_0_MAX_DATA_WIDTH,
		XPAR_XV_DEMOSAIC_0_ALGORITHM
#endif
	}
};

XV_demosaic_Config *XV_demosaic_LookupConfig(u16 DeviceId) {
	XV_demosaic_Config *ConfigPtr = NULL;

	int Index;

	for (Index = 0; Index < XPAR_XV_DEMOSAIC_NUM_INSTANCES; Index++) {
		if (XV_demosaic_ConfigTable[Index].DeviceId == DeviceId) {
			ConfigPtr = &XV_demosaic_ConfigTable[Index];
			break;
		}
	}

	return ConfigPtr;
}



/*****************************************************************************/
/**
 * This function programs colour space converter with the given width and height
 *
 * @param	width is Hsize of a packet in pixels.
 * @param	height is number of lines of a packet.
 *
 * @return	None.
 *
 * @note	None.
 *
 *****************************************************************************/
int demosaic()
{
  demosaic_Config = XV_demosaic_LookupConfig(DEMOSAIC_DEVICE_ID);
  XV_demosaic_CfgInitialize(&InstancePtr, demosaic_Config,
		                           demosaic_Config->BaseAddress);
  XV_demosaic_Set_HwReg_width(&InstancePtr, 1920);
  XV_demosaic_Set_HwReg_height(&InstancePtr, 1080);
  XV_demosaic_Set_HwReg_bayer_phase(&InstancePtr, 0x3);
  XV_demosaic_EnableAutoRestart(&InstancePtr);
  XV_demosaic_Start(&InstancePtr);
  return XST_SUCCESS;

}


/***************************************************************************/
/**
 *  *  * This function resets and releases IPs
 *   *   *
 *    *    * @return      None.
 *     *     *
 *      *      * @note        None.
 *       *       *
 *****************************************************************************/
void resetVIP(void)
{
  XGpio_Initialize(&Gpio, GPIO_4_DEVICE_ID);
  XGpio_DiscreteClear(&Gpio, GPIO_CHANNEL, SET);
  usleep(300);
  XGpio_DiscreteWrite(&Gpio, GPIO_CHANNEL, SET);
}

int vtpg_hdmi()
{
  XVidC_VideoMode TestMode;

  int status;

  status = driverInit();
  if(status != XST_SUCCESS) {
    return(XST_FAILURE);
  }

  resetVIP();
  TestMode = XVIDC_VM_1080_60_P;
  videoIpConfig(TestMode);
  usleep(300);
  return XST_SUCCESS;

}

/***************************************************************************/
/**
* This function writes, reads, and verifies the data to the IIC EEPROM. It
* does the write as a single page write, performs a buffered read.
*
* @param	None.
*
* @return	XST_SUCCESS if successful else XST_FAILURE.
*
* @note		None.
*
****************************************************************************/
extern int InitIIC()
{
  int Status;
  XIic_Config *ConfigPtr ;	/* Pointer to configuration data */
  FmcIicAddr = FMC_ADDRESS;
  AdapterIicAddr = ADAPTER_ADDRESS;

  /*
   * Initialize the FMC IIC so that it is ready to use.
   */
  ConfigPtr = XIic_LookupConfig(IIC_FMC_DEVICE_ID);
  if (ConfigPtr == NULL) {
    return XST_FAILURE;
  }

  Status = XIic_CfgInitialize(&IicFmc, ConfigPtr, ConfigPtr->BaseAddress);
  if (Status != XST_SUCCESS) {
	return XST_FAILURE;
  }

  /*
   * Initialize the Adapter IIC so that it is ready to use.
   */
  ConfigPtr = XIic_LookupConfig(IIC_ADAPTER_DEVICE_ID);
  if (ConfigPtr == NULL) {
    return XST_FAILURE;
  }

  Status = XIic_CfgInitialize(&IicAdapter, ConfigPtr,
			ConfigPtr->BaseAddress);
  if (Status != XST_SUCCESS) {
    return XST_FAILURE;
  }

  return XST_SUCCESS;

}



/***************************************************************************/
/***************************************************************************/
extern int SetupFmcInterruptSystem(XIic *IicFmcInstPtr)
{
  int Status;
#ifdef XPAR_INTC_0_DEVICE_ID


  /*
   * Initialize the interrupt controller driver so that it's ready to use.
   */

  Status = XIntc_Initialize(&IntcFmc, INTC_DEVICE_ID);

  if (Status != XST_SUCCESS) {
	return XST_FAILURE;
  }

  /*
   * Connect the device driver handler that will be called when an
   * interrupt for the device occurs, the handler defined above performs
   * the specific interrupt processing for the device.
   */
  Status = XIntc_Connect(&IntcFmc, IIC_FMC_INTR_ID,
				   (XInterruptHandler) XIic_InterruptHandler,
				   IicFmcInstPtr);
  if (Status != XST_SUCCESS) {
	return XST_FAILURE;
  }

  /*
   * Start the interrupt controller so interrupts are enabled for all
   * devices that cause interrupts.
   */
  Status = XIntc_Start(&IntcFmc, XIN_REAL_MODE);
  if (Status != XST_SUCCESS) {
    return XST_FAILURE;
  }


  /*
   * Enable the interrupts for the IIC device.
   */
  XIntc_Enable(&IntcFmc, IIC_FMC_INTR_ID);

#else

  XScuGic_Config *IntcConfig;

  /*
   * Initialize the interrupt controller driver so that it is ready to
   * use.
   */
  IntcConfig = XScuGic_LookupConfig(INTC_DEVICE_ID);
  if (NULL == IntcConfig) {
	return XST_FAILURE;
  }

  Status = XScuGic_CfgInitialize(&IntcFmc, IntcConfig,
					IntcConfig->CpuBaseAddress);
  if (Status != XST_SUCCESS) {
	return XST_FAILURE;
  }
  XScuGic_SetPriorityTriggerType(&IntcFmc, IIC_FMC_INTR_ID,
					0xA0, 0x3);
  /*
   * Connect the interrupt handler that will be called when an
   * interrupt occurs for the device.
   */
  Status = XScuGic_Connect(&IntcFmc, IIC_FMC_INTR_ID,
				 (Xil_InterruptHandler)XIic_InterruptHandler,
				 IicFmcInstPtr);

  if (Status != XST_SUCCESS) {
	return Status;
  }

  /*
   * Enable the interrupt for the IIC device.
   */
  XScuGic_Enable(&IntcFmc, IIC_FMC_INTR_ID);

#endif

  /*
   * Initialize the exception table and register the interrupt
   * controller handler with the exception table
   */
  Xil_ExceptionInit();

  Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_INT,
			 (Xil_ExceptionHandler)INTC_HANDLER, &IntcFmc);

  /* Enable non-critical exceptions */
  Xil_ExceptionEnable();

  return XST_SUCCESS;

}

/**************************************************************************/
/**************************************************************************/

extern int SetupAdapterInterruptSystem(XIic *IicAdapterInstPtr)
{
  int Status;

#ifdef XPAR_INTC_0_DEVICE_ID

  /*
   * Initialize the interrupt controller driver so that it's ready to use.
   */
  Status = XIntc_Initialize(&IntcAdapter, INTC_DEVICE_ID);

  if (Status != XST_SUCCESS) {
	return XST_FAILURE;
  }

  /*
   * Connect the device driver handler that will be called when an
   * interrupt for the device occurs, the handler defined above performs
   * the specific interrupt processing for the device.
   */
  Status = XIntc_Connect(&IntcAdapter, IIC_ADAPTER_INTR_ID,
				   (XInterruptHandler) XIic_InterruptHandler,
				   IicAdapterInstPtr);
  if (Status != XST_SUCCESS) {
	return XST_FAILURE;
  }

  /*
   *
   *
   * Start the interrupt controller so interrupts are enabled for all
   * devices that cause interrupts.
   */
  Status = XIntc_Start(&IntcAdapter, XIN_REAL_MODE);
  if (Status != XST_SUCCESS) {
	return XST_FAILURE;
  }


  /*
   * Enable the interrupts for the IIC device.
   */
  XIntc_Enable(&IntcAdapter, IIC_ADAPTER_INTR_ID);

#else

  XScuGic_Config *IntcConfig;

  /*
   * Initialize the interrupt controller driver so that it is ready to
   * use.
   */
  IntcConfig = XScuGic_LookupConfig(INTC_DEVICE_ID);
  if (NULL == IntcConfig) {
	return XST_FAILURE;
  }

  Status = XScuGic_CfgInitialize(&IntcAdapter, IntcConfig,
						IntcConfig->CpuBaseAddress);
  if (Status != XST_SUCCESS) {
	return XST_FAILURE;
  }
  XScuGic_SetPriorityTriggerType(&IntcAdapter, IIC_ADAPTER_INTR_ID,
					0xA0, 0x3);
  /*
   * Connect the interrupt handler that will be called when an
   * interrupt occurs for the device.
   */
  Status = XScuGic_Connect(&IntcAdapter, IIC_ADAPTER_INTR_ID,
				 (Xil_InterruptHandler)XIic_InterruptHandler,
				 IicAdapterInstPtr);

  if (Status != XST_SUCCESS) {
	return Status;
  }

  /*
   * Enable the interrupt for the IIC device.
   */
  XScuGic_Enable(&IntcAdapter, IIC_ADAPTER_INTR_ID);

#endif

  /*
   * Initialize the exception table and register the interrupt
   * controller handler with the exception table
   */
  Xil_ExceptionInit();

  Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_INT,
			 (Xil_ExceptionHandler)INTC_HANDLER, &IntcAdapter);

  /* Enable non-critical exceptions */
  Xil_ExceptionEnable();

  return XST_SUCCESS;

}


/***************************************************************************/
extern void SetupIICIntrHandlers() {
  /*
   * Set the Handlers for transmit and reception.
   */
  XIic_SetSendHandler(&IicFmc, &IicFmc, (XIic_Handler) SendHandler);
  XIic_SetRecvHandler(&IicFmc, &IicFmc, (XIic_Handler) ReceiveHandler);
  XIic_SetStatusHandler(&IicFmc, &IicFmc,
		                           (XIic_StatusHandler) StatusHandler);

  XIic_SetSendHandler(&IicAdapter, &IicAdapter, (XIic_Handler) SendHandler);
  XIic_SetRecvHandler(&IicAdapter, &IicAdapter,
		                                (XIic_Handler) ReceiveHandler);
  XIic_SetStatusHandler(&IicAdapter, &IicAdapter,
                                 (XIic_StatusHandler) StatusHandler);
}
/***************************************************************************/
/**
* This Send handler is called asynchronously from an interrupt
* context and indicates that data in the specified buffer has been sent.
*
* @param	InstancePtr is not used, but contains a pointer to the IIC
*		device driver instance which the handler is being called for.
*
* @return	None.
*
* @note		None.
*
****************************************************************************/
extern void SendHandler(XIic *InstancePtr)
{
  TransmitComplete = 0;
}

/***************************************************************************/
/**
* This Receive handler is called asynchronously from an interrupt
* context and indicates that data in the specified buffer has been Received.
*
* @param	InstancePtr is not used, but contains a pointer to the IIC
*		device driver instance which the handler is being called for.
*
* @return	None.
*
* @note		None.
*
****************************************************************************/
extern void ReceiveHandler(XIic *InstancePtr)
{
  ReceiveComplete = 0;
}

/***************************************************************************/
/**
* This Status handler is called asynchronously from an interrupt
* context and indicates the events that have occurred.
*
* @param	InstancePtr is a pointer to the IIC driver instance for which
*		the handler is being called for.
* @param	Event indicates the condition that has occurred.
*
* @return	None.
*
* @note		None.
*
****************************************************************************/
extern void StatusHandler(XIic *InstancePtr, int Event)
{

}


/***************************************************************************/
extern int SetFmcIICAddress() {

  int Status;
  FmcIicAddr = FMC_ADDRESS;
  /*
   * Set Address for FMC IIC
   */
  Status = XIic_SetAddress(&IicFmc, XII_ADDR_TO_SEND_TYPE, FmcIicAddr);
  if (Status != XST_SUCCESS) {
	return XST_FAILURE;
  }

  return XST_SUCCESS;

}

/****************************************************************************/

extern int SetAdapterIICAddress() {

  int Status;
  AdapterIicAddr = ADAPTER_ADDRESS;
  /*
   * Set Address for Adapter IIC
   */
  Status = XIic_SetAddress(&IicAdapter, XII_ADDR_TO_SEND_TYPE,
				 AdapterIicAddr);
  if (Status != XST_SUCCESS) {
	return XST_FAILURE;
  }


  return XST_SUCCESS;

}



/*************************************************************************/
/**
* This function writes a buffer of data to the Adapter IIC
* @param	ByteCount contains the number of bytes in the buffer to be
*		written.
*
* @return	XST_SUCCESS if successful else XST_FAILURE.
*
* @note		The Byte count should not exceed the page size of the EEPROM as
*		noted by the constant PAGE_SIZE.
*
**************************************************************************/
extern int AdapterWriteData(u16 ByteCount)
{
  int Status;

  u8 count = 0;

  /*
   * Set the defaults.
   */
  TransmitComplete = 1;
  IicAdapter.Stats.TxErrors = 0;

  /*
   * Start the IIC device.
   */
  Status = XIic_Start(&IicAdapter);
  if (Status != XST_SUCCESS) {
	return XST_FAILURE;
  }

  /*
   * Send the Data.
   */
  Status = XIic_MasterSend(&IicAdapter, WriteBuffer, ByteCount);
  if (Status != XST_SUCCESS) {
	return XST_FAILURE;
  }

  /*
   * Wait till the transmission is completed.
   */
  while ((TransmitComplete) || (XIic_IsIicBusy(&IicAdapter) == TRUE)) {

	if(count == 200) TransmitComplete = 0;

	/*
	 * This condition is required to be checked in the case where we
	 * are writing two consecutive buffers of data to the EEPROM.
	 * The EEPROM takes about 2 milliseconds time to update the data
	 * internally after a STOP has been sent on the bus.
	 * A NACK will be generated in the case of a second write before
	 * the EEPROM updates the data internally resulting in a
	 * Transmission Error.
	 */
	if (IicAdapter.Stats.TxErrors != 0) {


	  /*
	   * Enable the IIC device.
	   */
	  Status = XIic_Start(&IicAdapter);
	  if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	  }


	  if (!XIic_IsIicBusy(&IicAdapter)) {
		/*
		 * Send the Data.
		 */
	    Status = XIic_MasterSend(&IicAdapter, WriteBuffer,
							 ByteCount);
		if (Status == XST_SUCCESS) {
		  IicAdapter.Stats.TxErrors = 0;
		}
		else {

		}
	  }
	}
  }

  /*
   * Stop the IIC device.
   */
  Status = XIic_Stop(&IicAdapter);
  if (Status != XST_SUCCESS) {
	return XST_FAILURE;
  }

  return XST_SUCCESS;

}


//1ms delay; processor clk = 100 MHz
void Sensor_Delay()
{
  int cnt;
  for(cnt = 0; cnt < 100; cnt++) {

  }
}



int WritetoReg(u8 buf1, u8 buf2, u8 buf3){

  WriteBuffer[0] = buf1;
  WriteBuffer[1] = buf2;
  WriteBuffer[2] = buf3;

  for (int Index = 0; Index < PAGE_SIZE; Index++) {
	WriteBuffer[3 + Index] = 0xFF;
	ReadBuffer[Index] = 0;
  }

  AdapterWriteData(3);

  return XST_SUCCESS;

}


/***************************************************************************/
/***************************************************************************/
/***************************************************************************/

extern int SensorPreConfig(int pcam5c_mode) {


  u32 Index, MaxIndex, MaxIndex1, MaxIndex2;
  int Status;
  SensorIicAddr = SENSOR_ADDRESS;
  Status = XIic_SetAddress(&IicAdapter, XII_ADDR_TO_SEND_TYPE, SensorIicAddr);
  if (Status != XST_SUCCESS) {
	return XST_FAILURE;
  }


  WritetoReg(0x31, 0x03, 0x11);
  WritetoReg(0x30, 0x08, 0x82);

  Sensor_Delay();


  MaxIndex = length_sensor_pre;
  for(Index = 0; Index < (MaxIndex - 0); Index++)
  {
    WriteBuffer[0] = sensor_pre[Index].Address >> 8;
	WriteBuffer[1] = sensor_pre[Index].Address;
	WriteBuffer[2] = sensor_pre[Index].Data;

    Sensor_Delay();

	Status = AdapterWriteData(3);
	if (Status != XST_SUCCESS) {
	  return XST_FAILURE;
	}
  }


  WritetoReg(0x30, 0x08, 0x42);


  MaxIndex1 = length_pcam5c_mode1;

  for(Index = 0; Index < (MaxIndex1 - 0); Index++)
  {
    WriteBuffer[0] = pcam5c_mode1[Index].Address >> 8;
	WriteBuffer[1] = pcam5c_mode1[Index].Address;
	WriteBuffer[2] = pcam5c_mode1[Index].Data;

    Sensor_Delay();

	Status = AdapterWriteData(3);
	if (Status != XST_SUCCESS) {
	  return XST_FAILURE;
	}
  }


  WritetoReg(0x30, 0x08, 0x02);
  Sensor_Delay();
  WritetoReg(0x30, 0x08, 0x42);


  MaxIndex2 = length_sensor_list;

  for(Index = 0; Index < (MaxIndex2 - 0); Index++)
  {
    WriteBuffer[0] = sensor_list[Index].Address >> 8;
	WriteBuffer[1] = sensor_list[Index].Address;
	WriteBuffer[2] = sensor_list[Index].Data;

    Sensor_Delay();

	Status = AdapterWriteData(3);
	  if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	  }
  }


  if(Status != XST_SUCCESS) {
    xil_printf("Error: in Writing entry status = %x \r\n", Status);
    return XST_FAILURE;
  }

  return XST_SUCCESS;

}

/**************************************************************************/
void GPIOSelect(int dsi)
{
  if (dsi) {
    XGpio_Initialize(&Gpio, GPIO_3_DEVICE_ID);
	XGpio_DiscreteClear(&Gpio, GPIO_CHANNEL, SET);
  } else {
	XGpio_Initialize(&Gpio, GPIO_3_DEVICE_ID);
	XGpio_DiscreteSet(&Gpio, GPIO_CHANNEL, SET);
  }
}

void CamReset(void)
{
  XGpio_Initialize(&Gpio, GPIO_0_DEVICE_ID);
  XGpio_DiscreteClear(&Gpio, GPIO_CHANNEL, SET);
  usleep(10000);
  XGpio_DiscreteSet(&Gpio, GPIO_CHANNEL, SET);
  usleep(10000);
}
/*****************************************************************************/

void InitVprocSs_Scaler(int count) {
  XVprocSs_Config* p_vpss_cfg;
  int status;
  int widthIn, heightIn, widthOut, heightOut;

  widthOut = SCLR_OW;
  heightOut = SCLR_OH;

  // Local variables
  XVidC_VideoMode resIdIn, resIdOut;
  XVidC_VideoStream StreamIn, StreamOut;

  widthIn = TPG0_W;
  heightIn =TPG0_H;
  StreamIn.FrameRate = 60; //rao

  if (count) {
    p_vpss_cfg = XVprocSs_LookupConfig(XPAR_XVPROCSS_0_DEVICE_ID);
	if (p_vpss_cfg == NULL) {
	  xil_printf("ERROR! Failed to find VPSS-based scaler.\n\r");
      return;
	}

	status = XVprocSs_CfgInitialize(&scaler_new_inst, p_vpss_cfg,
				p_vpss_cfg->BaseAddress);
	if (status != XST_SUCCESS) {
	  xil_printf("ERROR! Failed to initialize VPSS-based scaler.\n\r");
	  return;
	}
  }

  XVprocSs_Stop(&scaler_new_inst);

  // Get resolution ID from frame size
  resIdIn = XVidC_GetVideoModeId(widthIn, heightIn, StreamIn.FrameRate,
			FALSE);

  // Setup Video Processing Subsystem
  StreamIn.VmId = resIdIn;
  StreamIn.Timing.HActive = widthIn;
  StreamIn.Timing.VActive = heightIn;
  StreamIn.ColorFormatId = XVIDC_CSF_RGB;
  StreamIn.ColorDepth = scaler_new_inst.Config.ColorDepth;
  StreamIn.PixPerClk = scaler_new_inst.Config.PixPerClock;
  StreamIn.IsInterlaced = 0;

  status = XVprocSs_SetVidStreamIn(&scaler_new_inst, &StreamIn);
  if (status != XST_SUCCESS) {
	xil_printf("Unable to set input video stream parameters correctly\r\n");
	return;
  }

  // Get resolution ID from frame size
  resIdOut = XVidC_GetVideoModeId(widthOut, heightOut, 60, FALSE);

  if (resIdOut != XVIDC_VM_1920x1200_60_P) {
	xil_printf("resIdOut %d doesn't match XVIDC_VM_1920x1200_60_P\r\n",
				resIdOut);
  }

  StreamOut.VmId = resIdOut;
  StreamOut.Timing.HActive = widthOut;
  StreamOut.Timing.VActive = heightOut;
  StreamOut.ColorFormatId = XVIDC_CSF_RGB;
  StreamOut.ColorDepth = scaler_new_inst.Config.ColorDepth;
  StreamOut.PixPerClk = scaler_new_inst.Config.PixPerClock;
  StreamOut.FrameRate = 60;
  StreamOut.IsInterlaced = 0;

  XVprocSs_SetVidStreamOut(&scaler_new_inst, &StreamOut);
  if (status != XST_SUCCESS) {
	xil_printf("Unable to set output video stream parameters correctly\r\n");
	return;
  }

  status = XVprocSs_SetSubsystemConfig(&scaler_new_inst);
  if (status != XST_SUCCESS) {
    xil_printf("xvprocss_SetSubsystemConfig failed %d\r\n", status);
    return;
  }

  XVprocSs_ReportSubsystemConfig(&scaler_new_inst);
  XVprocSs_Start(&scaler_new_inst);


}


/*****************************************************************************/
/**
*
* RunVDMA API
*
* This API is the interface between application and other API.
* When application will call this API with right argument, This API will call
* rest of the API to configure the read and write path of VDMA,based on ID.
* After that it will start both the read and write path of VDMA
*
* @param	InstancePtr is the handle to XAxiVdma data structure.
* @param	DeviceId is the device ID of current VDMA
* @param	hsize is the horizontal size of the frame. It will be in Pixels.
* 		The actual size of frame will be calculated by multiplying this
* 		with tdata width.
* @param 	vsize is the Vertical size of the frame.
* @param	buf_base_addr is the buffer address where frames will be written
*		and read by VDMA.
* @param 	number_frame_count specifies after how many frames the interrupt
*		should come.
* @param 	enable_frm_cnt_intr is for enabling frame count interrupt
*		when set to 1.
* @return
*		- XST_SUCCESS if example finishes successfully
*		- XST_FAILURE if example fails.
*
******************************************************************************/
int RunVDMA(XAxiVdma* InstancePtr, int DeviceId, int hsize,
		int vsize, int buf_base_addr, int number_frame_count,
		int enable_frm_cnt_intr)
{
  int Status,i;
 XAxiVdma_Config *Config;
 XAxiVdma_FrameCounter FrameCfgPtr;

  /* This is one time initialization of state machine context.
   * In first call it will be done for all VDMA instances in the system.
   */
  if(context_init==0) {
	for(i=0; i < XPAR_XAXIVDMA_NUM_INSTANCES; i++) {
	  vdma_context[i].InstancePtr = NULL;
	  vdma_context[i].device_id = -1;
	  vdma_context[i].hsize = 0;
	  vdma_context[i].vsize = 0;
	  vdma_context[i].init_done = 0;
	  vdma_context[i].buffer_address = 0;
	  vdma_context[i].enable_frm_cnt_intr = 0;
	  vdma_context[i].number_of_frame_count = 0;
	}
	context_init = 1;
  }

  /* The below initialization will happen for each VDMA. The API argument
   * will be stored in internal data structure
   */

  /* The information of the XAxiVdma_Config comes from hardware build.
   * The user IP should pass this information to the AXI DMA core.
   */
  Config = XAxiVdma_LookupConfig(DeviceId);
  if (!Config) {
	xil_printf("No video DMA found for ID %d\r\n",DeviceId );
	return XST_FAILURE;
  }

  if(vdma_context[DeviceId].init_done ==0) {
	vdma_context[DeviceId].InstancePtr = InstancePtr;

	/* Initialize DMA engine */
	Status = XAxiVdma_CfgInitialize(vdma_context[DeviceId].InstancePtr,
						Config, Config->BaseAddress);
	if (Status != XST_SUCCESS) {
	  xil_printf("Configuration Initialization failed %d\r\n",
					Status);
	  return XST_FAILURE;
	}

	vdma_context[DeviceId].init_done = 1;
  }

  vdma_context[DeviceId].device_id = DeviceId;
  vdma_context[DeviceId].vsize = vsize;
  vdma_context[DeviceId].enable_frm_cnt_intr = enable_frm_cnt_intr;
  vdma_context[DeviceId].buffer_address = buf_base_addr;
  vdma_context[DeviceId].number_of_frame_count = number_frame_count;
  vdma_context[DeviceId].hsize = hsize * (Config->Mm2SStreamWidth>>3);

  /* Setup the write channel */
  Status = WriteSetup(&vdma_context[DeviceId]);
  if (Status != XST_SUCCESS) {
	xil_printf("Write channel setup failed %d\r\n", Status);
	if(Status == XST_VDMA_MISMATCH_ERROR)
	  xil_printf("DMA Mismatch Error\r\n");
	return XST_FAILURE;
  }

  /* Setup the read channel */
  Status = ReadSetup(&vdma_context[DeviceId]);
  if (Status != XST_SUCCESS) {
	xil_printf("Read channel setup failed %d\r\n", Status);
	if(Status == XST_VDMA_MISMATCH_ERROR)
	  xil_printf("DMA Mismatch Error\r\n");
	return XST_FAILURE;
  }

  /* The frame counter interrupt is enabled, setting VDMA for same */
  if(vdma_context[DeviceId].enable_frm_cnt_intr) {
	FrameCfgPtr.ReadDelayTimerCount = 1;
	FrameCfgPtr.ReadFrameCount = number_frame_count;
	FrameCfgPtr.WriteDelayTimerCount = 1;
	FrameCfgPtr.WriteFrameCount = number_frame_count;

	XAxiVdma_SetFrameCounter(vdma_context[DeviceId].InstancePtr,
			&FrameCfgPtr);
	/* Enable DMA read and write channel interrupts.
	 * The configuration for interrupt
	 * controller will be done by application	 */
	XAxiVdma_IntrEnable(vdma_context[DeviceId].InstancePtr,
				XAXIVDMA_IXR_ERROR_MASK |
				XAXIVDMA_IXR_FRMCNT_MASK,XAXIVDMA_WRITE);
	XAxiVdma_IntrEnable(vdma_context[DeviceId].InstancePtr,
				XAXIVDMA_IXR_ERROR_MASK |
				XAXIVDMA_IXR_FRMCNT_MASK,XAXIVDMA_READ);
  } else	{
	  /* Enable DMA read and write channel interrupts.
	   * The configuration for interrupt
	   * controller will be done by application	 */
	  XAxiVdma_IntrEnable(vdma_context[DeviceId].InstancePtr,
					XAXIVDMA_IXR_ERROR_MASK,XAXIVDMA_WRITE);
	  XAxiVdma_IntrEnable(vdma_context[DeviceId].InstancePtr,
					XAXIVDMA_IXR_ERROR_MASK ,XAXIVDMA_READ);
  }

  /* Start the DMA engine to transfer */
  Status = StartTransfer(vdma_context[DeviceId].InstancePtr);
  if (Status != XST_SUCCESS) {
	if(Status == XST_VDMA_MISMATCH_ERROR)
	  xil_printf("DMA Mismatch Error\r\n");
	return XST_FAILURE;
  }
#if DEBUG_MODE
  xil_printf("Code is in Debug mode,\
		  Make sure that buffer addresses are at valid memory \r\n");
  xil_printf("In triple mode, \
		  there has to be six consecutive buffers for Debug mode \r\n");
  {
    u32 pixels,j,Addr = vdma_context[DeviceId].buffer_address;
	u8 *dst,*src;
	u32 total_pixel = vdma_context[DeviceId].stride * \
			vdma_context[DeviceId].vsize;
	src = (unsigned char *)Addr;
	dst = (unsigned char *)Addr + (total_pixel * \
			vdma_context->InstancePtr->MaxNumFrames);

	for(j=0;j<vdma_context->InstancePtr->MaxNumFrames;j++) {
	  for(pixels=0;pixels<total_pixel;pixels++) {
		if(src[pixels] != dst[pixels]) {
		  xil_printf("VDMA transfer failed: SRC=0x%x, DST=0x%x\r\n",
							src[pixels],dst[pixels]);
		  exit(-1);
		}
	  }
	  src = src + total_pixel;
	  dst = dst + total_pixel;
	}
  }
  xil_printf("VDMA transfer is happening and checked for 3 frames \r\n");
#endif

  return XST_SUCCESS;

}




/*****************************************************************************/
/**
*
* This function sets up the read channel
*
* @param	vdma_context is the context pointer to the VDMA engine.
*
* @return	XST_SUCCESS if the setup is successful, XST_FAILURE otherwise.
*
* @note		None.
*
******************************************************************************/
static int ReadSetup(vdma_handle *vdma_context)
{
  int Index;
  u32 Addr;
  int Status;

  vdma_context->ReadCfg.VertSizeInput = vdma_context->vsize;
  vdma_context->ReadCfg.HoriSizeInput = vdma_context->hsize;

  vdma_context->ReadCfg.Stride = vdma_context->hsize;
  /* This example does not test frame delay */
  vdma_context->ReadCfg.FrameDelay = 0;

  vdma_context->ReadCfg.EnableCircularBuf = 1;
  vdma_context->ReadCfg.EnableSync = 1;  /* Gen-Lock */

  vdma_context->ReadCfg.PointNum = 0;
  vdma_context->ReadCfg.EnableFrameCounter = 0; /* Endless transfers */

  vdma_context->ReadCfg.FixedFrameStoreAddr = 0;/* We are not doing parking */
  /* Configure the VDMA is per fixed configuration,
   *  This configuration is being used by majority
  * of customer. Expert users can play around with this
  * if they have different configurations */

  Status = XAxiVdma_DmaConfig(vdma_context->InstancePtr,
		      XAXIVDMA_READ, &vdma_context->ReadCfg);
  if (Status != XST_SUCCESS) {
	xil_printf("Read channel config failed %d\r\n", Status);
	return XST_FAILURE;
  }

  /* Initialize buffer addresses
   *
   * These addresses are physical addresses
   */
  Addr = vdma_context->buffer_address;

  for(Index = 0; Index < vdma_context->InstancePtr->MaxNumFrames; Index++) {
    vdma_context->ReadCfg.FrameStoreStartAddr[Index] = Addr;

    /* Initializing the buffer in case of Debug mode */

#if DEBUG_MODE
	{
	  u32 i;
	  u8 *src;
	  u32 total_pixel = vdma_context->stride * vdma_context->vsize;
	  src = (unsigned char *)Addr;
	  xil_printf("Read Buffer %d address: 0x%x \r\n",Index,Addr);
	  for(i=0;i<total_pixel;i++)
	  {
	    src[i] = i & 0xFF;
	  }
	}
#endif
	Addr +=  vdma_context->hsize * vdma_context->vsize;
  }

  /* Set the buffer addresses for transfer in the DMA engine
   * The buffer addresses are physical addresses
   */
  Status = XAxiVdma_DmaSetBufferAddr(vdma_context->InstancePtr,
		  XAXIVDMA_READ, vdma_context->ReadCfg.FrameStoreStartAddr);
  if (Status != XST_SUCCESS) {
	xil_printf("Read channel set buffer address failed %d\r\n", Status);
    return XST_FAILURE;
  }

  return XST_SUCCESS;

}

/*****************************************************************************/
/**
*
* This function sets up the write channel
*
* @param	dma_context is the context pointer to the VDMA engine..
*
* @return	XST_SUCCESS if the setup is successful, XST_FAILURE otherwise.
*
* @note		None.
*
******************************************************************************/
static int WriteSetup(vdma_handle *vdma_context)
{
  int Index;
  u32 Addr;
  int Status;

  vdma_context->WriteCfg.VertSizeInput = vdma_context->vsize;
  vdma_context->WriteCfg.HoriSizeInput = vdma_context->hsize;

  vdma_context->WriteCfg.Stride = vdma_context->hsize;
  /* This example does not test frame delay */
  vdma_context->WriteCfg.FrameDelay = 0;

  vdma_context->WriteCfg.EnableCircularBuf = 1;
  vdma_context->WriteCfg.EnableSync = 1;  /*  Gen-Lock */

  vdma_context->WriteCfg.PointNum = 0;
  vdma_context->WriteCfg.EnableFrameCounter = 0; /* Endless transfers */

  vdma_context->WriteCfg.FixedFrameStoreAddr = 0; /* We are not doing parking */
  /* Configure the VDMA is per fixed configuration, This configuration
   * is being used by majority of customers. Expert users can play around
   * with this if they have different configurations
   */

  Status = XAxiVdma_DmaConfig(vdma_context->InstancePtr,
		          XAXIVDMA_WRITE, &vdma_context->WriteCfg);
  if (Status != XST_SUCCESS) {
	xil_printf("Write channel config failed %d\r\n", Status);
    return Status;
  }

  /* Initialize buffer addresses
   *
   * Use physical addresses
   */
  Addr = vdma_context->buffer_address;
  /* If Debug mode is enabled write frame is shifted 3 Frames
   * store ahead to compare read and write frames
   */
#if DEBUG_MODE
  Addr = Addr + vdma_context->InstancePtr->MaxNumFrames * \
			(vdma_context->stride * vdma_context->vsize);
#endif

  for(Index = 0; Index < vdma_context->InstancePtr->MaxNumFrames; Index++){
	vdma_context->WriteCfg.FrameStoreStartAddr[Index] = Addr;
#if DEBUG_MODE
	xil_printf("Write Buffer %d address: 0x%x \r\n",Index,Addr);
#endif

	Addr += (vdma_context->hsize * vdma_context->vsize);
  }

  /* Set the buffer addresses for transfer in the DMA engine */
  Status = XAxiVdma_DmaSetBufferAddr(vdma_context->InstancePtr,
			XAXIVDMA_WRITE,
			vdma_context->WriteCfg.FrameStoreStartAddr);
  if (Status != XST_SUCCESS) {
	xil_printf("Write channel set buffer address failed %d\r\n", Status);
    return XST_FAILURE;
  }

  /* Clear data buffer
   */
#if DEBUG_MODE
  memset((void *)vdma_context->buffer_address, 0,
			vdma_context->ReadCfg.Stride * \
			vdma_context->ReadCfg.VertSizeInput * \
			vdma_context->InstancePtr->MaxNumFrames);
#endif
  return XST_SUCCESS;

}

/*****************************************************************************/
/**
*
* This function starts the DMA transfers. Since the DMA engine is operating
* in circular buffer mode, video frames will be transferred continuously.
*
* @param	InstancePtr points to the DMA engine instance
*
* @return
*		- XST_SUCCESS if both read and write start successfully
*		- XST_FAILURE if one or both directions cannot be started
*
* @note		None.
*
******************************************************************************/
static int StartTransfer(XAxiVdma *InstancePtr)
{

  int Status;
  /* Start the write channel of VDMA */
  Status = XAxiVdma_DmaStart(InstancePtr, XAXIVDMA_WRITE);
  if (Status != XST_SUCCESS) {
	xil_printf("Start Write transfer failed %d\r\n", Status);
    return XST_FAILURE;
  }
  /* Start the Read channel of VDMA */
  Status = XAxiVdma_DmaStart(InstancePtr, XAXIVDMA_READ);
  if (Status != XST_SUCCESS) {
	xil_printf("Start read transfer failed %d\r\n", Status);
	return XST_FAILURE;
  }

  return XST_SUCCESS;

}



/*****************************************************************************/
/**
*
* This function wait until the DMA channel halts
*
* @param	VdmaChannel specifes VdmaChannel is MM2S or S2MM
*.@param	VdmaBaseAddr VDMA base address
*
* @return
*		MM2S_HALT_SUCCESS, S2MM_HALT_SUCCESS in success
*		MM2S_HALT_FAILURE, S2MM_HALT_FAILURE on failure
*
* @note		None.
*
******************************************************************************/
s32 WaitForCompletion(s32 VdmaChannel, u32 *VdmaBaseAddr)
{
  if (VdmaChannel == MM2S) {
    while (!(*Mm2sStatusReg & 0x1)) {
      xil_printf("Mm2sStatusReg = 0x%x\r\n", *Mm2sStatusReg);
	  xdbg_printf(XDBG_DEBUG_GENERAL,"Waiting for MM2S to halt ..."
			"MM2S SR = 0x%x\r\n", *Mm2sStatusReg);
    }
	if ((*Mm2sStatusReg & 0x1)) {
	  xdbg_printf(XDBG_DEBUG_GENERAL," MM2S_HALT_SUCCESS \r\n");
	  return MM2S_HALT_SUCCESS;
	} else {
	  xil_printf(" returning MM2S_HALT_FAILURE \r\n");
	  return MM2S_HALT_FAILURE;
	}
  }
  else if (VdmaChannel == S2MM) {
    xdbg_printf(XDBG_DEBUG_GENERAL," Poll on s2mm status register\r\n");
	while (!(*S2mmStatusReg & 0x1)) {
      xdbg_printf(XDBG_DEBUG_GENERAL," Waiting for S2MM to halt ..."
				"S2MM SR = 0x%x\r\n", *S2mmStatusReg);
	}
	if((*S2mmStatusReg & 0x1)) {
      xdbg_printf(XDBG_DEBUG_GENERAL," MM2S_HALT_SUCCESS \r\n");
      return S2MM_HALT_SUCCESS;
	}
	else {
	  xil_printf(" returning S2MM_HALT_FAILURE \r\n");
      return S2MM_HALT_FAILURE;
	}
  }
  return XST_FAILURE;
}


/***************************************************************************/
/**
*
* This function ResetVDMA
*
* @param	None
*
* @return	None
*
* @note		None.
*
******************************************************************************/
void ResetVDMA()
{

  XAxiVdma_Reset(&AxiVdma,XAXIVDMA_READ);
  XAxiVdma_Reset(&AxiVdma,XAXIVDMA_WRITE);

}

void HaltVDMA()
{

  ResetVDMA();

  WaitForCompletion(VDMA_MM2S, (u32*)VDMA_BASEADDR);

  WaitForCompletion(VDMA_S2MM, (u32*)VDMA_BASEADDR);

}

void InitVprocSs_CSC(int count) {
  XVprocSs_Config* p_vpss_cfg1;
  int status;
  int widthIn, heightIn, widthOut, heightOut;

  widthOut = 1920;
  heightOut = 1080;

  // Local variables
  XVidC_VideoMode resIdIn, resIdOut;
  XVidC_VideoStream StreamIn, StreamOut;

  widthIn = 1920;
  heightIn = 1080;
  StreamIn.FrameRate = 60; //rao

  if (count) {
    p_vpss_cfg1 = XVprocSs_LookupConfig(XPAR_XVPROCSS_1_DEVICE_ID);
	if (p_vpss_cfg1 == NULL) {
	  xil_printf("ERROR! Failed to find VPSS-based scaler.\n\r");
      return;
	}

	status = XVprocSs_CfgInitialize(&csc_new_inst, p_vpss_cfg1,
				p_vpss_cfg1->BaseAddress);
	if (status != XST_SUCCESS) {
	  xil_printf("ERROR! Failed to initialize VPSS-based scaler.\n\r");
	  return;
	}
  }

  XVprocSs_Stop(&csc_new_inst);

  // Get resolution ID from frame size
  resIdIn = XVidC_GetVideoModeId(widthIn, heightIn, StreamIn.FrameRate,
			FALSE);

  // Setup Video Processing Subsystem
  StreamIn.VmId = resIdIn;
  StreamIn.Timing.HActive = widthIn;
  StreamIn.Timing.VActive = heightIn;
  StreamIn.ColorFormatId = XVIDC_CSF_RGB;
  StreamIn.ColorDepth = csc_new_inst.Config.ColorDepth;
  StreamIn.PixPerClk = csc_new_inst.Config.PixPerClock;
  StreamIn.IsInterlaced = 0;

  status = XVprocSs_SetVidStreamIn(&csc_new_inst, &StreamIn);
  if (status != XST_SUCCESS) {
	xil_printf("Unable to set input video stream parameters correctly\r\n");
	return;
  }

  // Get resolution ID from frame size
  resIdOut = XVidC_GetVideoModeId(widthOut, heightOut, 60, FALSE);

  if (resIdOut != XVIDC_VM_1920x1080_60_P) {
	xil_printf("resIdOut %d doesn't match XVIDC_VM_1920x1080_60_P\r\n",
				resIdOut);
  }


  StreamOut.VmId = resIdOut;
  StreamOut.Timing.HActive = widthOut;
  StreamOut.Timing.VActive = heightOut;
  StreamOut.ColorFormatId = XVIDC_CSF_YCRCB_444;
  StreamOut.ColorDepth = csc_new_inst.Config.ColorDepth;
  StreamOut.PixPerClk = csc_new_inst.Config.PixPerClock;
  StreamOut.FrameRate = 60;
  StreamOut.IsInterlaced = 0;

  XVprocSs_SetVidStreamOut(&csc_new_inst, &StreamOut);
  if (status != XST_SUCCESS) {
	xil_printf("Unable to set output video stream parameters correctly\r\n");
	return;
  }

  status = XVprocSs_SetSubsystemConfig(&csc_new_inst);
  if (status != XST_SUCCESS) {
    xil_printf("xvprocss_SetSubsystemConfig failed %d\r\n", status);
    return;
  }

  XVprocSs_ReportSubsystemConfig(&scaler_new_inst);
  XVprocSs_Start(&scaler_new_inst);


}





/*****************************************************************************/

int vdma_dsi() {


  InitVprocSs_Scaler(1);

  ResetVDMA();

  RunVDMA(&AxiVdma, XPAR_AXI_VDMA_0_DEVICE_ID, HORIZONTAL_RESOLUTION, \
		  VERTICAL_RESOLUTION, srcBuffer, FRAME_COUNTER, 0);

  XDsiTxSs_Activate(&DsiTxSs, XDSITXSS_DSI, XDSITXSS_ENABLE);

  return XST_SUCCESS;

}




int vdma_hdmi() {

  InitVprocSs_CSC(1);

  ResetVDMA();

  RunVDMA(&AxiVdma, XPAR_AXI_VDMA_0_DEVICE_ID, HORIZONTAL_RESOLUTION, \
		  VERTICAL_RESOLUTION, srcBuffer, FRAME_COUNTER, 0);

  return XST_SUCCESS;

}



/*****************************************************************************/
