/******************************************************************************
* Copyright (C) 2018 - 2020 Xilinx, Inc.  All rights reserved.
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
//HDMI
#include "xv_tpg.h"
#include "xvtc.h"
#include "xvidc.h"
//#include "pipeline_program.h"
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
XAxiVdma CsiVdma;
XVprocSs scaler_new_inst;


XV_tpg_Config		*tpg1_Config;
XV_tpg				tpg1;

XVtc				vtc;
XVtc_Config			*vtc_Config;
XVtc_Timing			vtc_timing;
u32 volatile		*gpio_hlsIpReset;
u32 volatile		*gpio_videoLockMonitor;

//END

XV_demosaic				Demosaic;
/************************** Constant Definitions *****************************/

/* Uncomment or comment the following depending on the board used*/
#define KC705
//#define ZC702
//#define ZCU102

#ifdef KC705
    #define MIG_7SERIES_BASEADDRESS0  0x80000000
    #define MIG_7SERIES_BASEADDRESS1  0x80000000
    #define CSC_BASEADDRESS  0x44AC0000
    #define VGAMMALUT_BASE  XPAR_V_GAMMA_LUT_0_S_AXI_CTRL_BASEADDR
	#define IIC_FMC_DEVICE_ID	XPAR_IIC_0_DEVICE_ID
	#define IIC_ADAPTER_DEVICE_ID	XPAR_IIC_1_DEVICE_ID
    #define DEMOSAIC_BASE   XPAR_V_DEMOSAIC_0_S_AXI_CTRL_BASEADDR
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

        #define DSI_BASE XPAR_MIPI_DSI_TX_SUBSYSTEM_0_BASEADDR
#define CSI_BASE XPAR_MIPI_CSI2_RX_SUBSYSTEM_0_RX_BASEADDR
#define VDMA_BASE XPAR_AXI_VDMA_0_BASEADDR
#define XPAR_MIPI_CSI2_RX_SUBSYSTEM_0_RX_BASEADDR
#define XCSIRXSS_DEVICE_ID      XPAR_CSISS_0_DEVICE_ID
#define XDSITXSS_DEVICE_ID      XPAR_DSITXSS_0_DEVICE_ID

//#define VDMA_BASE1 XPAR_AXI_VDMA_1_BASEADDR

        #define VPROCSSCSC_BASE XPAR_V_PROC_SS_0_BASEADDR
#define DMA_DEVICE_ID		XPAR_AXI_VDMA_0_DEVICE_ID
//#define DMA_DEVICE_ID1		XPAR_AXI_VDMA_1_DEVICE_ID
        #define V_PRC0 XPAR_V_PROC_SS_0_BASEADDR

        #define BRAM_0_ADDRESS		XPAR_AXI_BRAM_CTRL_0_S_AXI_BASEADDR
        #define BRAM_1_ADDRESS		XPAR_AXI_BRAM_CTRL_1_S_AXI_BASEADDR


        #define NO_BYTES_PER_PIXEL		3
        #define HORIZONTAL_RESOLUTION		1920 //1280

#define HORIZONTAL_RESOLUTION_BYTES HORIZONTAL_RESOLUTION * NO_BYTES_PER_PIXEL
#define VERTICAL_RESOLUTION		1080 //720
#define FRAME_COUNTER			3
#define STRIDE				HORIZONTAL_RESOLUTION_BYTES


/* Specify peripheral Timing parameters */
        #define HACT_VALUE	HORIZONTAL_RESOLUTION_BYTES
        #define HSA_VALUE	60
        #define HFP_VALUE	60
        #define HBP_VALUE	60

        #define VACT_VALUE	VERTICAL_RESOLUTION
        #define VSA_VALUE	60
        #define VFP_VALUE	60
        #define VBP_VALUE	60
        #define BLLP_BURST_VALUE	0x0

        #define VDMA_S2MM	1
        #define VDMA_MM2S	2

/* Debug Constants */
        #define MM2S_HALT_SUCCESS	121
        #define MM2S_HALT_FAILURE	122
        #define S2MM_HALT_SUCCESS	131
        #define S2MM_HALT_FAILURE	132

        #define ENABLE_FRAME_COUNT	(1<<4)
        #define ENABLE_GEN_LOCK	        (1<<3)
        #define ENABLE_CIRCULAR_MODE	(1<<1)
        #define START_VDMA		(1<<0)

        #define DT_RGB444 	0x20
        #define DT_RGB555 	0x21
        #define DT_RGB565 	0x22
        #define DT_RGB666 	0x23
        #define DT_RGB888 	0x24
        #define DT_RAW6 	0x28
        #define DT_RAW7 	0x29
        #define DT_RAW8 	0x2A
        #define DT_RAW10 	0x2B
        #define DT_RAW12 	0x2C
        #define DT_RAW14 	0x2D
        #define DT_YUV422_8BIT 	0x1E

    #define WIDTH 1920
	#define HEIGHT 1080
    #define width 1920
	#define height 1080
#define DSI_BYTES_PER_PIXEL     (3)
#define DSI_H_RES               (1920)
#define DSI_V_RES               (1200)
#define bayerPhase 3
#define DSI_DISPLAY_HORI_VAL    (DSI_H_RES * DSI_BYTES_PER_PIXEL)
#define DSI_DISPLAY_VERT_VAL    (DSI_V_RES)
#define DSI_HBACK_PORCH                 (0x39D)
#define DSI_HFRONT_PORCH                (0x00B9)
#define DSI_VSYNC_WIDTH                 (0x05)
#define DSI_VBACK_PORCH                 (0x04)
#define DSI_VFRONT_PORCH                (0x03)
#define DSI_CONFIG                (0xb)
#define DSI_START                (0x01)
#define DSI_STOP                (0x00)
#define timing1                (0x168004b0)
#define timing2                (0x39d00b9)
#define timing3                (0x050403)
#define demosaic_config           (0x3)
#define demosaic_start            (0x81)
#define SET            (0x01)
#define RESET            (0x00)
#define DEMOSAIC_WIDTH     (DEMOSAIC_BASE + 0x10)
#define DEMOSAIC_HEIGHT    (DEMOSAIC_BASE + 0x18)
#define DEMOSAIC_CFG1     (DEMOSAIC_BASE + 0x20)
#define DEMOSAIC_CFG2     (DEMOSAIC_BASE + 0x28)
#define DSI_CFG     (DSI_BASE + 0x04)
#define DSI_TMG1    (DSI_BASE + 0x54)
#define DSI_TMG2     (DSI_BASE + 0x58)
#define DSI_TMG3     (DSI_BASE + 0x5c)
#define VDMA_MM2S    (VDMA_BASE + 0x00)
#define VDMA_S2MM    (VDMA_BASE + 0x30)
#define VDMA_SET    (0x3008B)
#define VDMA_RESET    (0x4)
#define CSC_SET    (0x01)

#define LANES 2
        #define TPG0_W 1920
        #define TPG0_H 1080
        //#define TPG1_W 640
        //#define TPG1_H 480
        #define SCLR_OW  1920
        #define SCLR_OH  1200
        //#define TPG0_W1 1280
        //#define TPG0_H1 720
        //#define SCLR_OW1  1920
        //#define SCLR_OH1  1080

#endif


/**************************** Type Definitions *******************************/

typedef u8 AddressType;

 XIic IicFmc, IicAdapter;
 XIic *IicFmcInstPtr, *IicAdapterInstPtr;/* The instance of the IIC device. */
 INTC IntcFmc, IntcAdapter, IntcSensor;
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

int videoClockConfig(XVidC_VideoMode videoMode)
{
	u32 DIVCLK_DIVIDE = 4;
	u32 CLKFBOUT_MULT = 37;
	u32 CLKFBOUT_FRAC = 125;
	u32 CLKOUT0_DIVIDE;
	u32 CLKOUT0_FRAC;
	u32 clock_config_reg_0;
	u32 clock_config_reg_2;
	u32 timeout;
	u32 lock;
	u16 PixelsPerClk, mode_index;

    const int ClkOut_Frac[3][XVIDC_PPC_NUM_SUPPORTED] =
    { {250, 500, 0  , 0}, //1080p
      {125, 250, 500, 0}, //4K30
      {0,   125, 250, 500}  //4K60
    };
    const int ClkOut_Div[3][XVIDC_PPC_NUM_SUPPORTED] =
    { {6, 12, 25, 50}, //1080p
      {3, 6 , 12, 25}, //4K30
      {0, 3 , 6 , 12}  //4K60
    };

    /* Validate TPG Parameters */
    Xil_AssertNonvoid((tpg1.Config.PixPerClk == XVIDC_PPC_1) ||
                      (tpg1.Config.PixPerClk == XVIDC_PPC_2) ||
					  (tpg1.Config.PixPerClk == XVIDC_PPC_4) ||
                      (tpg1.Config.PixPerClk == XVIDC_PPC_8));


    mode_index = ((videoMode ==  XVIDC_VM_1080_60_P) ? 0 :
                  (videoMode ==  XVIDC_VM_UHD_30_P)  ? 1 :
                  (videoMode ==  XVIDC_VM_UHD_60_P)  ? 2 : 3);

    if(mode_index > 2)
    {
xil_printf("ERR::Video Mode is unsupported\r\n");
      return(XST_FAILURE);
    }

    //map PPC to array index
PixelsPerClk=((tpg1.Config.PixPerClk==XVIDC_PPC_8)?3:tpg1.Config.PixPerClk>>1);
CLKOUT0_FRAC   =  ClkOut_Frac[mode_index][PixelsPerClk];
CLKOUT0_DIVIDE =  ClkOut_Div[mode_index][PixelsPerClk];

clock_config_reg_0=(1<<26)|(CLKFBOUT_FRAC<<16)|(CLKFBOUT_MULT<<8)|DIVCLK_DIVIDE;
clock_config_reg_2=(1<<18)|(CLKOUT0_FRAC<<8)|CLKOUT0_DIVIDE;

	VideoClockGen_WriteReg(0x200, clock_config_reg_0);
	VideoClockGen_WriteReg(0x208, clock_config_reg_2);

	usleep(300000);

	lock = VideoClockGen_ReadReg(0x4) & 0x1;
	if(!lock) //check for lock
	{
		//Video Clock Generator not locked
		VideoClockGen_WriteReg(0x25C, 0x7);
		VideoClockGen_WriteReg(0x25C, 0x2);
		timeout = 100000;
		while(!lock)
		{
			lock = VideoClockGen_ReadReg(0x4) & 0x1;
			--timeout;
			if(!timeout)
			{
				xil_printf("ERR:: Video Clock Generator failed lock\r\n");
				return(XST_FAILURE);
			}
		}
	}
	xil_printf("Video Clock Generator locked\r\n");

	return(XST_SUCCESS);
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
 * This function programs Demosaic with the required timing paramters.
 *
 * @return      None.
 *
 * @note        None.
 *
 ***************************************************************************/

u32 InitializeDemosaic(void)
{
        u32 Status = 0;
        XCsiSs_Config *DemosaicCfgPtr = NULL;

        DemosaicCfgPtr = XV_demosaic_LookupConfig(XPAR_V_DEMOSAIC_0_DEVICE_ID);
        if (!DemosaicCfgPtr) {
                xil_printf("Demosaic LookupCfg failed\r\n");
                return XST_FAILURE;
        }

        Status = XV_demosaic_CfgInitialize(&Demosaic, DemosaicCfgPtr,
                        DemosaicCfgPtr->BaseAddr);

        if (Status != XST_SUCCESS) {
                xil_printf("Demosaic Cfg init failed - %x\r\n", Status);
                return Status;
        }

        return XST_SUCCESS;
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
void EnableDSI(void)
{
	int Status;
//	XDsiTxSs_Activate(&DsiTxSs, XCSI_ENABLE);
	Status = XDsiTxSs_Activate(&DsiTxSs, XDSITXSS_DSI, XDSITXSS_ENABLE);
	Status = XDsiTxSs_Activate(&DsiTxSs, XDSITXSS_PHY, XDSITXSS_ENABLE);
}

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
	 //	XDphy_Activate(DsiTxSs.DphyPtr, XDSITXSS_ENABLE);

	 /*	if (!XDsiTxSs_IsControllerReady(&DsiTxSs)) {
			xil_printf("DSI Controller NOT Ready!!!!\r\n");
			return;
		}*/
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
	                                                XDSI_VM_NON_BURST_SYNC_EVENT,
	                                                &Timing);
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
        u32 Status;
     //  XDsiTxSs_Reset(&DsiTxSs);
		                     Xil_Out32((DSI_BASE), DSI_STOP);
       /* XDsiTxSs_Activate(&DsiTxSs, XDSITXSS_DISABLE);
        do {
                Status = XDsiTxSs_IsControllerReady(&DsiTxSs);
        } while (!Status);*/

}
void resetIp(void)
{

  DisableCSI();
	      //  xil_printf("  CSI Disabled\r\n");
	//Xil_Out32(XPAR_MIPI_CSI2_RX_SUBSYSTEM_0_BASEADDR, 0x00);
	//*(u32 *)(CSI_BASE + 0x00) = 0x00;

	//Xil_Out32(VDMA_MM2S_BASE, 0x00);
	//Xil_Out32(VDMA_S2MM_BASE, 0x00);
	HaltVDMA();
				//*(u32 *)(VDMA_BASE + 0x00) = 0x00;
				//*(u32 *)(VDMA_BASE + 0x30) = 0x00;
            //   XAxiVdma_DmaStop(&CsiVdma, XAXIVDMA_WRITE);
	       //XAxiVdma_DmaStop(&CsiVdma, XAXIVDMA_READ);
	  DisableDSI();
	//XDsiTxSs_Reset(&DsiTxSs);
	      //  xil_printf("  DSI RESET DONE\r\n");
	//Xil_Out32(XPAR_MIPI_DSI_TX_SUBSYSTEM_0_BASEADDR, 0x00);
	//*(u32 *)(DSI_BASE + 0x00) = 0x00;
	 resetVIP();
	xil_printf("  RESET DONE\r\n");
}

int demosaic(){


	      Xil_Out32((DEMOSAIC_WIDTH), width);
	      Xil_Out32((DEMOSAIC_HEIGHT), height);
	      Xil_Out32((DEMOSAIC_CFG1), demosaic_config);
	     Xil_Out32((DEMOSAIC_CFG2), demosaic_config);
	      Xil_Out32((DEMOSAIC_BASE ), demosaic_start);


		 return XST_SUCCESS;
	}

void resetVIP(void)
{
	Xil_Out32(XPAR_AXI_GPIO_4_BASEADDR, RESET); //reset IPs
         usleep(300);
	Xil_Out32(XPAR_AXI_GPIO_4_BASEADDR, SET); // release reset

}
 int vtpg_hdmi()

 {
	 XVidC_VideoMode TestMode;

int status;
	        xil_printf("Start test\r\n");


	        status = driverInit();
	        if(status != XST_SUCCESS) {
	                return(XST_FAILURE);
	        }

	     resetVIP();
	        TestMode = XVIDC_VM_1080_60_P;
	             xil_printf("\r\nTest: %s\r\n", XVidC_GetVideoModeStr(TestMode));
	               xil_printf("tst1\r\n");
	               videoIpConfig(TestMode);

	               usleep(300);

	               xil_printf("Successfully ran Example\r\n");
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

	Status = XIic_CfgInitialize(&IicFmc, ConfigPtr,
			ConfigPtr->BaseAddress);
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
XIic_SetStatusHandler(&IicFmc, &IicFmc, (XIic_StatusHandler) StatusHandler);

XIic_SetSendHandler(&IicAdapter, &IicAdapter, (XIic_Handler) SendHandler);
XIic_SetRecvHandler(&IicAdapter, &IicAdapter, (XIic_Handler) ReceiveHandler);
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

#ifdef IIC_MUX_ENABLE
/***************************************************************************/
/**
* This function initializes the IIC MUX to select EEPROM.
*
* @param	None.
*
* @return	XST_SUCCESS if pass, otherwise XST_FAILURE.
*
* @note		None.
*
**************************************************************************/
extern int MuxInit(void)
{

	int Status;
	/*
	 * Set the Slave address to the IIC MUC - PCA9543A.
	 */
	Status = XIic_SetAddress(&IicFmc, XII_ADDR_TO_SEND_TYPE,
				 IIC_MUX_ADDRESS);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}


	/*
	 * Enabling all the channels
	 */
	WriteBuffer[0] = IIC_FMC_CHANNEL;

	Status = FmcWriteData(1);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}
	xil_printf("Muxinit-3\r");
	return XST_SUCCESS;
}
#endif
/***************************************************************************/
extern int SetFmcIICAddress() {

	int Status;
	FmcIicAddr = FMC_ADDRESS;
/*
 * Set Address for FMC IIC
 */
	Status = XIic_SetAddress(&IicFmc, XII_ADDR_TO_SEND_TYPE,
				 FmcIicAddr);
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


/***************************************************************************/
/**
* This function writes a buffer of data to the FMC IIC
*
* @param	ByteCount contains the number of bytes in the buffer to be
*		written.
*
* @return	XST_SUCCESS if successful else XST_FAILURE.
*
* @note		The Byte count should not exceed the page size of the EEPROM as
*		noted by the constant PAGE_SIZE.
*
****************************************************************************/
extern int FmcWriteData(u16 ByteCount)
{
	int Status;

	/*
	 * Set the defaults.
	 */
	TransmitComplete = 1;
	IicFmc.Stats.TxErrors = 0;

	/*
	 * Start the IIC device.
	 */
	Status = XIic_Start(&IicFmc);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Send the Data.
	 */
	Status = XIic_MasterSend(&IicFmc, WriteBuffer, ByteCount);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}
	/*
	 * Wait till the transmission is completed.
	 */
	while ((TransmitComplete) || (XIic_IsIicBusy(&IicFmc) == TRUE)) {
	 xil_printf("Muxinit-3\r");
		/*
		 * This condition is required to be checked in the case where we
		 * are writing two consecutive buffers of data to the EEPROM.
		 * The EEPROM takes about 2 milliseconds time to update the data
		 * internally after a STOP has been sent on the bus.
		 * A NACK will be generated in the case of a second write before
		 * the EEPROM updates the data internally resulting in a
		 * Transmission Error.
		 */
		if (IicFmc.Stats.TxErrors != 0) {


			/*
			 * Enable the IIC device.
			 */
			Status = XIic_Start(&IicFmc);
			if (Status != XST_SUCCESS) {
				return XST_FAILURE;
			}


			if (!XIic_IsIicBusy(&IicFmc)) {
				/*
				 * Send the Data.
				 */
				Status = XIic_MasterSend(&IicFmc,
							 WriteBuffer,
							 ByteCount);
				if (Status == XST_SUCCESS) {
					IicFmc.Stats.TxErrors = 0;
				}
				else {
				}
			}
		}
	}

	/*
	 * Stop the IIC device.
	 */
	Status = XIic_Stop(&IicFmc);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	return XST_SUCCESS;
}

/***************************************************************************/
/**
* This function reads data from the FMC IIC into a specified buffer.
*
* @param	BufferPtr contains the address of the data buffer to be filled.
* @param	ByteCount contains the number of bytes in the buffer to be read.
*
* @return	XST_SUCCESS if successful else XST_FAILURE.
*
* @note		None.
*
****************************************************************************/
extern int FmcReadData(u8 *BufferPtr, u16 ByteCount)
{
	int Status;
	AddressType Address = FMC_TEST_START_ADDRESS;

	/*
	 * Set the Defaults.
	 */
	ReceiveComplete = 1;

	/*
	 * Position the Pointer in EEPROM.
	 */
	if (sizeof(Address) == 1) {
		WriteBuffer[0] = (u8) (FMC_TEST_START_ADDRESS);
	}
	else {
		WriteBuffer[0] = (u8) (FMC_TEST_START_ADDRESS >> 8);
		WriteBuffer[1] = (u8) (FMC_TEST_START_ADDRESS);
	}

	Status = FmcWriteData(sizeof(Address));
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Start the IIC device.
	 */
	Status = XIic_Start(&IicFmc);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Receive the Data.
	 */
	Status = XIic_MasterRecv(&IicFmc, BufferPtr, ByteCount);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Wait till all the data is received.
	 */
	while ((ReceiveComplete) || (XIic_IsIicBusy(&IicFmc) == TRUE)) {

	}

	/*
	 * Stop the IIC device.
	 */
	Status = XIic_Stop(&IicFmc);
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

//		xil_printf("Muxinit-3\r");
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
				Status = XIic_MasterSend(&IicAdapter,
							 WriteBuffer,
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
  //xil_printf("\nDelay..\n");
  for(cnt = 0; cnt < 100; cnt++) {
  //xil_printf("cnt=%d.\r",cnt);
  }
}

void Sensor_Delay_small()
{
  int cnt;
  //xil_printf("\nDelay..\n");
  for(cnt = 0; cnt < 10; cnt++) {
  //xil_printf("cnt=%d.\r",cnt);
  }
}
/***************************************************************************/
/**
* This function reads data from the Adapter IIC into a specified buffer.
*
* @param	BufferPtr contains the address of the data buffer to be filled.
* @param	ByteCount contains the number of bytes in the buffer to be read.
*
* @return	XST_SUCCESS if successful else XST_FAILURE.
*
* @note		None.
*
****************************************************************************/
extern int AdapterReadData(u8 *BufferPtr, u16 ByteCount)
{
	int Status;
	AddressType Address = ADAPTER_TEST_START_ADDRESS;

	/*
	 * Set the Defaults.
	 */
	ReceiveComplete = 1;

	/*
	 * Position the Pointer in EEPROM.
	 */
	if (sizeof(Address) == 1) {
		WriteBuffer[0] = (u8) (ADAPTER_TEST_START_ADDRESS);
	}
	else {
		WriteBuffer[0] = (u8) (ADAPTER_TEST_START_ADDRESS >> 8);
		WriteBuffer[1] = (u8) (ADAPTER_TEST_START_ADDRESS);
	}

	Status = AdapterWriteData(sizeof(Address));
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Start the IIC device.
	 */
	Status = XIic_Start(&IicAdapter);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Receive the Data.
	 */
	Status = XIic_MasterRecv(&IicAdapter, BufferPtr, ByteCount);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Wait till all the data is received.
	 */
	while ((ReceiveComplete) || (XIic_IsIicBusy(&IicAdapter) == TRUE)) {

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
unsigned int delay = 0;
extern int InitMIPICsiSs(void);
void sensor_delay() {
  for(int lcnt=0;lcnt <100;lcnt++){
  xil_printf("Delay..%d\r",lcnt);
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

extern int SensorPreConfig(pcam5c_mode) {


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
	xil_printf("\n\rTest Setup Finished .. Watch Display pannel \n\r");

	return XST_SUCCESS;
}

/**************************************************************************/
void GPIOSelect(int dsi)
{
	if (dsi) {
	Xil_Out32(XPAR_AXI_GPIO_3_BASEADDR, RESET);
	} else {
	Xil_Out32(XPAR_AXI_GPIO_3_BASEADDR, SET);
	}
}
void CamReset(void)
{
	Xil_Out32(XPAR_AXI_GPIO_0_BASEADDR, RESET);
	usleep(10000);
	Xil_Out32(XPAR_AXI_GPIO_0_BASEADDR, SET);
	usleep(10000);
}
/*****************************************************************************/

XVprocSs scaler_new_inst;
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
		p_vpss_cfg = XVprocSs_LookupConfig(XPAR_V_PROC_SS_0_DEVICE_ID);
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
	//StreamIn.FrameRate      = 30;
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
	*VdmaS2MMCrReg |= 0x4;
	while (*VdmaS2MMCrReg & 0x4);
	*VdmaMM2SCrReg |= 0x4;
	while (*VdmaMM2SCrReg & 0x4);
}

void HaltVDMA()
{
	Xil_Out32((VDMA_MM2S), RESET);
	Xil_Out32((VDMA_S2MM), RESET);
}

void RunVDMA()
{
		Xil_Out32((VDMA_MM2S), VDMA_SET);
		Xil_Out32((VDMA_S2MM), VDMA_SET);
}
//VDMA 2


/*****************************************************************************/
/**
*
* This function to setup Horizontal size and stride
*
* @param	VdmaChannel specifes VdmaChannel is MM2S or S2MM
*
* @return	None
*
* @note		None.
*
******************************************************************************/
void ConfigureChannel(u32 VdmaChannel)
{
    u8 ii = 0;
    u8 offset = 0;
    volatile u8 *BaseAddress;

    if (VdmaChannel == VDMA_MM2S) {
	*VdmaMM2SHoriSizeReg = HORIZONTAL_RESOLUTION_BYTES;
	*VdmaMM2SFrmDlrStrideReg = STRIDE;
	BaseAddress = (u32)MIG_7SERIES_BASEADDRESS0 + 1000000;
	 while(ii < FRAME_COUNTER) {
		*(VdmaMM2SFrameBuffer0Reg + offset) = (u32)BaseAddress;
		offset = offset + 1;
		BaseAddress = BaseAddress +
		(HORIZONTAL_RESOLUTION_BYTES * VERTICAL_RESOLUTION);
		ii++;
	}
    }

    else if (VdmaChannel == VDMA_S2MM) {
	*VdmaS2MMHoriSizeReg = HORIZONTAL_RESOLUTION_BYTES;
	*VdmaS2MMFrmDlrStrideReg = STRIDE;

	BaseAddress = (u32)MIG_7SERIES_BASEADDRESS1 + 1000000;;
	while(ii < FRAME_COUNTER) {
		*(VdmaS2MMFrameBuffer0Reg + offset) = (u32)BaseAddress;
		offset = offset + 1;
		BaseAddress = BaseAddress +
		(HORIZONTAL_RESOLUTION_BYTES * VERTICAL_RESOLUTION);
		ii++;
		}
	}
}

/*****************************************************************************/
/**
*
* This function to setup the VDMA Control Register
*
* @param	VdmaChannel specifes VdmaChannel is MM2S or S2MM
* @param	FrameCount specifies number of frames to transfer
*
* @return	None
*
* @note		None.
*
******************************************************************************/
void SetupVdmaCR(s32 VdmaChannel, s32 FrameCount)
{
	if (VdmaChannel == VDMA_MM2S) {
		*VdmaMM2SCrReg = (FrameCount << 16) | ENABLE_FRAME_COUNT |
			ENABLE_GEN_LOCK | ENABLE_CIRCULAR_MODE | START_VDMA ;
	}
	else if (VdmaChannel==VDMA_S2MM) {
	        *VdmaS2MMCrReg = ((FrameCount-1) << 16) | ENABLE_FRAME_COUNT |
					ENABLE_GEN_LOCK | ENABLE_CIRCULAR_MODE | START_VDMA ;
	}
}

//VDMA-2

/*****************************************************************************/
/**
*
* This function to start mm2s or s2mm VdmaChannel - write the vertical size
*
* @param	VdmaChannel specifes VdmaChannel is MM2S or S2MM
* @param	VerticalSize specifes No of lines
*
* @return	None
*
* @note		None.
*
******************************************************************************/

void StartChannel(s32 VdmaChannel, s32 VerticalSize) {
	if (VdmaChannel == VDMA_MM2S) {
		*VdmaMM2SVertSizeReg = VerticalSize;
	}else if (VdmaChannel == VDMA_S2MM) {
		*VdmaS2MMVertSizeReg = VerticalSize;
	}
}


/*****************************************************************************/

int vdma_dsi() {

	u32 Status;

	                    Xil_Out32((DSI_CFG), DSI_CONFIG);
	                   Xil_Out32((DSI_TMG1), timing1);
	                    Xil_Out32((DSI_TMG2), timing2);
	                    Xil_Out32((DSI_TMG3), timing3);


		     InitVprocSs_Scaler(1);
		     xil_printf("\n\r Scaler\n\r");

		     ResetVDMA();

		              SetupVdmaCR(VDMA_MM2S,FRAME_COUNTER);
			 SetupVdmaCR(VDMA_S2MM,FRAME_COUNTER);

		              ConfigureChannel(VDMA_MM2S);
			 ConfigureChannel(VDMA_S2MM);

			 StartChannel(VDMA_S2MM, VERTICAL_RESOLUTION);
			 StartChannel(VDMA_MM2S, VERTICAL_RESOLUTION);

			 RunVDMA();


xil_printf(" Transfer Started \r\n");
                                      Status = XST_SUCCESS;
		                     Xil_Out32((DSI_BASE), DSI_START);
		               return XST_SUCCESS;



     }

int vdma_hdmi() {
	u32 Status;
	                    Xil_Out32((CSC_BASEADDRESS),CSC_SET);

	  ResetVDMA();

         SetupVdmaCR(VDMA_MM2S,FRAME_COUNTER);
	 SetupVdmaCR(VDMA_S2MM,FRAME_COUNTER);

         ConfigureChannel(VDMA_MM2S);
	 ConfigureChannel(VDMA_S2MM);

	 StartChannel(VDMA_S2MM, VERTICAL_RESOLUTION);
	 StartChannel(VDMA_MM2S, VERTICAL_RESOLUTION);

	 RunVDMA();

 xil_printf(" Transfer Started  \r\n");
                  Status = XST_SUCCESS;


          return XST_SUCCESS;
}
int SensorReg(){

	unsigned int test_secs;
	unsigned int test_mins;
	unsigned int t_scr;
	unsigned int fps0;
	unsigned int fps1;
	unsigned int fps;
	unsigned int img1;
	unsigned int isr;
	unsigned int img0;
        unsigned int ecc1;
        unsigned int ecc2;
        unsigned int crc;
        unsigned int sot;
        unsigned int sotsyhs;
        unsigned int slbf;
        unsigned int dtype;
        unsigned int lines;
        unsigned int wc;
        unsigned int pxls;

//Timer-Start
#ifdef KC705
*(u32 *)(CSI_TIMER_SENSOR +0x04) = 0xB2D05E00;
xil_printf("LoadTimer\r\n");
*(u32 *)(CSI_TIMER_SENSOR +0x00) = 0x20;
xil_printf("EnableTimer\r\n");
*(u32 *)(CSI_TIMER_SENSOR +0x00) = 0xD2;

test_secs = 0;
#endif

xil_printf("Mins:Frame,FPS,Image,ISR,IMG0,IMG1,SLBF,ECC-1,");
                xil_printf("ECC-2,CRC,SOT,SOTSYNCHS\r\n");

while(1){
fps0 = *(u32 *)(CSI_SS_SENSOR +0x08);
do {
t_scr = *(u32 *)(CSI_TIMER_SENSOR +0x00);
t_scr = (t_scr & 0x00000100) >> 8;
} while(!t_scr);
test_secs = test_secs + 30;
test_mins = test_secs/60;
t_scr = *(u32 *)(CSI_TIMER_SENSOR +0x00);
*(u32 *)(CSI_TIMER_SENSOR +0x00) = t_scr;
fps1 = *(u32 *)(CSI_SS_SENSOR +0x08);
                        fps =(fps1-fps0)/30;
			isr = *(u32 *)(CSI_SS_SENSOR +0x24);
			img0 = *(u32 *)(CSI_SS_SENSOR +0x60);
			img1 = *(u32 *)(CSI_SS_SENSOR +0x64);
			lines = *(u32 *)(CSI_SS_SENSOR +0x0c);
                        slbf     = (isr & 0x00040000)>>18;
                        ecc1     = (isr & 0x00000400)>>10;
                        ecc2     = (isr & 0x00000800)>>11;
                        crc      = (isr & 0x00000200)>>9;
                        sot      = (isr & 0x00002000)>>13;
                        sotsyhs  = (isr & 0x00001000)>>12;
                        dtype = img1 & 0x0000003F; //LSB 6bits
                        wc    = img0 & 0x0000FFFF; //LSB 16bits
                        switch (dtype ){
                         case 0x2A:
                             pxls = (wc*8)/8;
                            break;
                         case 0x2B:
                             pxls = (wc*8)/10;
                            break;
                         case 0x2C:
                             pxls = (wc*8)/12;
                            break;
                         default :
                             pxls = (wc*8)/8;
                            break;
                        }
xil_printf("%d-%d:%d,%dx%d,%x,%x,%x,",test_mins,fps1,fps,pxls,
                                          lines,isr,img0,img1);
xil_printf("  %x,  %x,   %x, %x, %x, %x\r",slbf,ecc1,ecc2,crc,sot,sotsyhs);

			img0 = *(u32 *)(CSI_SS_SENSOR +0x1c);
		for(int dcnt=0;dcnt <5000;dcnt++);
		for(int dcnt=0;dcnt <5000;dcnt++);
		for(int dcnt=0;dcnt <5000;dcnt++);
		for(int dcnt=0;dcnt <5000;dcnt++);
	}
	return XST_SUCCESS;
}


/*****************************************************************************/
