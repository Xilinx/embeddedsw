/******************************************************************************
*
* Copyright (C) 2017 Xilinx, Inc. All rights reserved.
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
* @file xdprxss_rxonly.c
*
* This file contains a design example using the XDpRxSs driver in single stream
* (SST) transport mode.
*
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver  Who Date     Changes
* ---- --- -------- --------------------------------------------------
* 1.00 vk 10/04/17 Initial release.
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/
#define DEBUG
#include "xdprxss.h"
#include "xvphy.h"
#include "xil_printf.h"
#include "xil_types.h"
#include "xparameters.h"
#include "xstatus.h"
#include "xdebug.h"
#include "xdprxss_dp159.h"
#include "xuartps_hw.h"
#include "xtmrctr.h"
#ifdef XPAR_INTC_0_DEVICE_ID
/* For MicroBlaze systems. */
#include "xintc.h"
#else
/* For ARM/Zynq SoC systems. */
#include "xscugic.h"
#endif /* XPAR_INTC_0_DEVICE_ID */

/************************** Constant Definitions *****************************/

/*
* The following constants map to the names of the hardware instances.
* They are only defined here such that a user can easily change all the
* needed device IDs in one place.
* There is only one interrupt controlled to be selected from SCUGIC and GPIO
* INTC. INTC selection is based on INTC parameters defined xparameters.h file.
*/
#define XINTC_DPRXSS_DP_INTERRUPT_ID \
	XPAR_FABRIC_DP_RX_HIER_0_DP_RX_SUBSYSTEM_0_DPRXSS_DP_IRQ_INTR
#define XINTC_DEVICE_ID			XPAR_SCUGIC_SINGLE_DEVICE_ID
#define XINTC				    XScuGic
#define XINTC_HANDLER			XScuGic_InterruptHandler

/* The unique device ID of the instances used in example
 */
#define XDPRXSS_DEVICE_ID		XPAR_DPRXSS_0_DEVICE_ID
#define XVPHY_DEVICE_ID		   XPAR_DP_RX_HIER_0_VID_PHY_CONTROLLER_0_DEVICE_ID
#define XTIMER0_DEVICE_ID		XPAR_TMRCTR_0_DEVICE_ID

#define VIDEO_CRC_BASEADDR      XPAR_DP_RX_HIER_0_VIDEO_FRAME_CRC_0_BASEADDR
#define UARTLITE_BASEADDR       XPAR_PSU_UART_0_BASEADDR
#define VIDPHY_BASEADDR         XPAR_DP_RX_HIER_0_VID_PHY_CONTROLLER_0_BASEADDR
#define VID_EDID_BASEADDR       XPAR_DP_RX_HIER_0_VID_EDID_0_BASEADDR

/* DP Specific Defines
 */
#define DPRXSS_LINK_RATE		XDPRXSS_LINK_BW_SET_540GBPS
#define DPRXSS_LANE_COUNT		XDPRXSS_LANE_COUNT_SET_4
#define SET_TX_TO_2BYTE			\
	(XPAR_DP_RX_HIER_0_DP_RX_SUBSYSTEM_0_DP_GT_DATAWIDTH/2)
#define SET_RX_TO_2BYTE			\
	(XPAR_DP_RX_HIER_0_DP_RX_SUBSYSTEM_0_DP_GT_DATAWIDTH/2)
#define XDP_RX_CRC_CONFIG    	0x074
#define XDP_RX_CRC_COMP0    	0x078
#define XDP_RX_CRC_COMP1    	0x07C
#define XDP_RX_CRC_COMP2    	0x080
/*
 * User can tune these variables as per their system
 */

/*Max timeout tuned as per tester - AXI Clock=100 MHz*/
#define DP_BS_IDLE_TIMEOUT      0x0091FFFF
#define VBLANK_WAIT_COUNT       20

/* Video Frame CRC Specific Defines
 */
#define VIDEO_FRAME_CRC_CONFIG			0x00
#define VIDEO_FRAME_CRC_VALUE_G_R		0x04
#define VIDEO_FRAME_CRC_VALUE_B			0x08
#define VIDEO_FRAME_CRC_ACTIVE_COUNTS	0x0C

/* VPHY Specific Defines
 */
#define XVPHY_RX_SYM_ERR_CNTR_CH1_2_REG	0x084
#define XVPHY_RX_SYM_ERR_CNTR_CH3_4_REG	0x088

#define XVPHY_DRP_CPLL_FBDIV		0x28
#define XVPHY_DRP_CPLL_REFCLK_DIV	0x2A
#define XVPHY_DRP_RXOUT_DIV			0x63
#define XVPHY_DRP_RXCLK25			0x6D
#define XVPHY_DRP_TXCLK25			0x7A
#define XVPHY_DRP_TXOUT_DIV			0x7C

/* Timer Specific Defines
 */
#define TIMER_RESET_VALUE		1000

/***************** Macros (Inline Functions) Definitions *********************/


/**************************** Type Definitions *******************************/
typedef enum {
        ONBOARD_REF_CLK = 1,
        DP159_FORWARDED_CLK = 3,
} XVphy_User_GT_RefClk_Src;

typedef struct {
        u8 Index;
        XVphy_PllType  TxPLL;
        XVphy_PllType  RxPLL;
        XVphy_ChannelId TxChId;
        XVphy_ChannelId RxChId;
        u32 LineRate;
        u64 LineRateHz;
        XVphy_User_GT_RefClk_Src QPLLRefClkSrc;
        XVphy_User_GT_RefClk_Src CPLLRefClkSrc;
        u64 QPLLRefClkFreqHz;
        u64 CPLLRefClkFreqHz;
} XVphy_User_Config;

static XVphy_User_Config PHY_User_Config_Table[] =
{
  // Index,         TxPLL,               RxPLL,
 //	TxChId,         RxChId,
// LineRate,              LineRateHz,
// QPLLRefClkSrc,          CPLLRefClkSrc,    QPLLRefClkFreqHz,CPLLRefClkFreqHz
  {   0,     XVPHY_PLL_TYPE_CPLL,   XVPHY_PLL_TYPE_CPLL,
		  XVPHY_CHANNEL_ID_CHA,     XVPHY_CHANNEL_ID_CHA,
		  0x06,    XVPHY_DP_LINK_RATE_HZ_162GBPS,
		  DP159_FORWARDED_CLK,    DP159_FORWARDED_CLK,     270000000,81000000},
  {   1,     XVPHY_PLL_TYPE_CPLL,   XVPHY_PLL_TYPE_CPLL,
		  XVPHY_CHANNEL_ID_CHA,     XVPHY_CHANNEL_ID_CHA,
		  0x0A,    XVPHY_DP_LINK_RATE_HZ_270GBPS,
		  DP159_FORWARDED_CLK,    DP159_FORWARDED_CLK,     270000000,135000000},
  {   2,     XVPHY_PLL_TYPE_CPLL,   XVPHY_PLL_TYPE_CPLL,
		  XVPHY_CHANNEL_ID_CHA,     XVPHY_CHANNEL_ID_CHA,
		  0x14,    XVPHY_DP_LINK_RATE_HZ_540GBPS,
		  DP159_FORWARDED_CLK,    DP159_FORWARDED_CLK,     270000000,270000000},
  {   3,     XVPHY_PLL_TYPE_QPLL1,  XVPHY_PLL_TYPE_CPLL,
		  XVPHY_CHANNEL_ID_CMN1,    XVPHY_CHANNEL_ID_CHA,
		  0x06,    XVPHY_DP_LINK_RATE_HZ_162GBPS,
		  ONBOARD_REF_CLK,        DP159_FORWARDED_CLK,     270000000,81000000},
  {   4,     XVPHY_PLL_TYPE_QPLL1,  XVPHY_PLL_TYPE_CPLL,
		  XVPHY_CHANNEL_ID_CMN1,    XVPHY_CHANNEL_ID_CHA,
		  0x0A,    XVPHY_DP_LINK_RATE_HZ_270GBPS,
		  ONBOARD_REF_CLK,        DP159_FORWARDED_CLK,     270000000,135000000},
  {   5,     XVPHY_PLL_TYPE_QPLL1,  XVPHY_PLL_TYPE_CPLL,
		  XVPHY_CHANNEL_ID_CMN1,    XVPHY_CHANNEL_ID_CHA,
		  0x14,    XVPHY_DP_LINK_RATE_HZ_540GBPS,
		  ONBOARD_REF_CLK,        DP159_FORWARDED_CLK,     270000000,270000000},
  {   6,     XVPHY_PLL_TYPE_CPLL,   XVPHY_PLL_TYPE_CPLL,
		  XVPHY_CHANNEL_ID_CHA,     XVPHY_CHANNEL_ID_CHA,
		  0x06,    XVPHY_DP_LINK_RATE_HZ_162GBPS,
		  ONBOARD_REF_CLK,        ONBOARD_REF_CLK,         270000000,270000000},
  {   7,     XVPHY_PLL_TYPE_CPLL,   XVPHY_PLL_TYPE_CPLL,
		  XVPHY_CHANNEL_ID_CHA,     XVPHY_CHANNEL_ID_CHA,
		  0x0A,    XVPHY_DP_LINK_RATE_HZ_270GBPS,
		  ONBOARD_REF_CLK,        ONBOARD_REF_CLK,         270000000,270000000},
  {   8,     XVPHY_PLL_TYPE_CPLL,   XVPHY_PLL_TYPE_CPLL,
		  XVPHY_CHANNEL_ID_CHA,     XVPHY_CHANNEL_ID_CHA,
		  0x14,    XVPHY_DP_LINK_RATE_HZ_540GBPS,
		  ONBOARD_REF_CLK,        ONBOARD_REF_CLK,         270000000,270000000},

};

typedef struct {
        u8  TEST_CRC_CNT;
        u8  TEST_CRC_SUPPORTED;
        u8  TEST_CRC_START_STOP;
        u16 Pixel_r;
        u16 Pixel_g;
        u16 Pixel_b;
        u8  Mode_422;
} Video_CRC_Config;

typedef struct {
        u8  MinVoltageSwing;
        u8  ClockRecoveryOption;
        u16 VswingLoopCount;
        u16 SetVswing;
        u16 ChEqOption;
        u8  SetPreemp;
        u8  Itr1Premp;
        u8  Itr2Premp;
        u8  Itr3Premp;
        u8  Itr4Premp;
        u8  Itr5Premp;
} DP_Rx_Training_Algo_Config;

/************************** Function Prototypes ******************************/

u32 DpRxSs_Main(u16 DeviceId);
u32 DpRxSs_PlatformInit(void);
u32 DpRxSs_VideoPhyInit(u16 DeviceId);
u32 DpRxSs_Setup(void);
void PHY_Two_byte_set (XVphy *InstancePtr, u8 Rx_to_two_byte);
void PLLRefClkSel (XVphy *InstancePtr, u8 link_rate);
void AppHelp();
void ReportVideoCRC();
void CalculateCRC(void);
void LoadEDID(void);
char XUartPs_RecvByte_NonBlocking();
void CustomWaitUs(void *InstancePtr, u32 MicroSeconds);

/* Interrupt helper functions */
u32 DpRxSs_SetupIntrSystem(void);

void DpRxSs_PowerChangeHandler(void *InstancePtr);
void DpRxSs_NoVideoHandler(void *InstancePtr);
void DpRxSs_VerticalBlankHandler(void *InstancePtr);
void DpRxSs_TrainingLostHandler(void *InstancePtr);
void DpRxSs_VideoHandler(void *InstancePtr);
void DpRxSs_InfoPacketHandler(void *InstancePtr);
void DpRxSs_ExtPacketHandler(void *InstancePtr);
void DpRxSs_TrainingDoneHandler(void *InstancePtr);
void DpRxSs_UnplugHandler(void *InstancePtr);
void DpRxSs_LinkBandwidthHandler(void *InstancePtr);
void DpRxSs_PllResetHandler(void *InstancePtr);


/************************** Variable Definitions *****************************/

XDpRxSs DpRxSsInst;	/* The DPRX Subsystem instance.*/
XINTC IntcInst;		/* The interrupt controller instance. */
XVphy VPhyInst;	/* The DPRX Subsystem instance.*/
XTmrCtr TmrCtr; /* Timer instance.*/
volatile Video_CRC_Config VidFrameCRC;

/************************** Function Definitions *****************************/


/*****************************************************************************/
/**
*
* This is the main function for XDpRxSs interrupt example. If the
* DpRxSs_Main function which setup the system succeeds, this function
* will wait for the interrupts.
*
* @param	None.
*
* @return
*		- XST_FAILURE if the interrupt example was unsuccessful.
*
* @note		Unless setup failed, main will never return since
*		DpRxSs_Main is blocking (it is waiting on interrupts).
*
******************************************************************************/
int main()
{
	u32 Status;

	xil_printf("------------------------------------------\n\r");
	xil_printf("DisplayPort RX Only Example\n\r");
	xil_printf("(c) 2017 by Xilinx\n\r");
	xil_printf("-------------------------------------------\n\r\n\r");

	Status = DpRxSs_Main(XDPRXSS_DEVICE_ID);
	if (Status != XST_SUCCESS) {
		xil_printf("DisplayPort RX Subsystem design example failed.");
		return XST_FAILURE;
	}

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* This function is the main entry point for the design example using the
* XDpRxSs driver. This function will setup the system with interrupts handlers.
*
* @param	DeviceId is the unique device ID of the DisplayPort RX
*		Subsystem core.
*
* @return
*		- XST_FAILURE if the system setup failed.
*		- XST_SUCCESS should never return since this function, if setup
*		  was successful, is blocking.
*
* @note		If system setup was successful, this function is blocking in
*		order to illustrate interrupt handling taking place for
*		different types interrupts.
*		Refer xdprxss.h file for more info.
*
******************************************************************************/
u32 DpRxSs_Main(u16 DeviceId)
{
	u32 Status;
	XDpRxSs_Config *ConfigPtr;
	u8 UserInput;
	u32 ReadVal=0;
	int i;

	/* Do platform initialization in this function. This is hardware
	 * system specific. It is up to the user to implement this function.
	 */
	xil_printf("PlatformInit\n\r");
	Status = DpRxSs_PlatformInit();
	if (Status != XST_SUCCESS) {
		xil_printf("Platform init failed!\n\r");
	}
	xil_printf("Platform initialization done.\n\r");

	/* Obtain the device configuration for the DisplayPort RX Subsystem */
	ConfigPtr = XDpRxSs_LookupConfig(DeviceId);
	if (!ConfigPtr) {
		return XST_FAILURE;
	}
	/* Copy the device configuration into the DpRxSsInst's Config
	 * structure. */
	Status = XDpRxSs_CfgInitialize(&DpRxSsInst, ConfigPtr,
					ConfigPtr->BaseAddress);
	if (Status != XST_SUCCESS) {
		xil_printf("DPRXSS config initialization failed.\n\r");
		return XST_FAILURE;
	}

	/* Check for SST/MST support */
	if (DpRxSsInst.UsrOpt.MstSupport) {
		xil_printf("\n\rINFO:DPRXSS is MST enabled. DPRXSS can be "
			"switched to SST/MST\n\r\n\r");
	}
	else {
		xil_printf("\n\rINFO:DPRXSS is SST enabled. DPRXSS works "
			"only in SST mode.\n\r\n\r");
	}

	Status = DpRxSs_SetupIntrSystem();
	if (Status != XST_SUCCESS) {
		xil_printf("ERR: Interrupt system setup failed.\n\r");
		return XST_FAILURE;
	}

	/* Set Link rate and lane count to maximum */
	XDpRxSs_SetLinkRate(&DpRxSsInst, DPRXSS_LINK_RATE);
	XDpRxSs_SetLaneCount(&DpRxSsInst, DPRXSS_LANE_COUNT);

	/* Start DPRX Subsystem set */
	Status = XDpRxSs_Start(&DpRxSsInst);
	if (Status != XST_SUCCESS) {
		xil_printf("ERR:DPRX SS start failed\n\r");
		return XST_FAILURE;
	}

	XVphy_ResetGtPll(&VPhyInst, 0, XVPHY_CHANNEL_ID_CHA, XVPHY_DIR_RX,(TRUE));
	/* Setup Video Phy, left to the user for implementation */
	DpRxSs_VideoPhyInit(XVPHY_DEVICE_ID);

	/* Setup DPRX SS, left to the user for implementation */
	DpRxSs_Setup();

	AppHelp();
	while (1)
	{
		UserInput = XUartPs_RecvByte_NonBlocking();
		if(UserInput!=0)
		{
			xil_printf("UserInput: %c\r\n",UserInput);

			switch(UserInput)
			{
				case 's':
					xil_printf("DP Link Status --->\r\n");
					XDpRxSs_ReportLinkInfo(&DpRxSsInst);
					break;

				case 'd':
					xil_printf("Video PHY Config/Status --->\r\n");
					xil_printf(" RCS (0x10) = 0x%x\n\r",
					  XVphy_ReadReg(VIDPHY_BASEADDR,XVPHY_REF_CLK_SEL_REG));
					xil_printf(" PR  (0x14) = 0x%x\n\r",
					  XVphy_ReadReg(VIDPHY_BASEADDR,XVPHY_PLL_RESET_REG));
					xil_printf(" PLS (0x18) = 0x%x\n\r",
					  XVphy_ReadReg(VIDPHY_BASEADDR,XVPHY_PLL_LOCK_STATUS_REG));
					xil_printf(" TXI (0x1C) = 0x%x\n\r",
					  XVphy_ReadReg(VIDPHY_BASEADDR,XVPHY_TX_INIT_REG));
					xil_printf(" TXIS(0x20) = 0x%x\n\r",
					  XVphy_ReadReg(VIDPHY_BASEADDR,XVPHY_TX_INIT_STATUS_REG));
					xil_printf(" RXI (0x24) = 0x%x\n\r",
					  XVphy_ReadReg(VIDPHY_BASEADDR,XVPHY_RX_INIT_REG));
					xil_printf(" RXIS(0x28) = 0x%x\n\r",
					  XVphy_ReadReg(VIDPHY_BASEADDR,XVPHY_RX_INIT_STATUS_REG));

					xil_printf(
				" GT DRP Addr (XVPHY_DRP_CPLL_FBDIV) = 0x%x, Val = 0x%x\n\r",
					XVPHY_DRP_CPLL_FBDIV,XVphy_DrpRead(
					&VPhyInst, 0, XVPHY_CHANNEL_ID_CH1, XVPHY_DRP_CPLL_FBDIV));
					xil_printf(
			" GT DRP Addr (XVPHY_DRP_CPLL_REFCLK_DIV) = 0x%x, Val = 0x%x\n\r",
					XVPHY_DRP_CPLL_REFCLK_DIV,XVphy_DrpRead(&VPhyInst, 0,
							XVPHY_CHANNEL_ID_CH1, XVPHY_DRP_CPLL_REFCLK_DIV));
					xil_printf(
					" GT DRP Addr (XVPHY_DRP_RXOUT_DIV) = 0x%x, Val = 0x%x\n\r",
						XVPHY_DRP_RXOUT_DIV,XVphy_DrpRead(&VPhyInst, 0,
								XVPHY_CHANNEL_ID_CH1, XVPHY_DRP_RXOUT_DIV));
					xil_printf(
					" GT DRP Addr (XVPHY_DRP_TXOUT_DIV) = 0x%x, Val = 0x%x\n\r",
						XVPHY_DRP_TXOUT_DIV,XVphy_DrpRead(&VPhyInst, 0,
								XVPHY_CHANNEL_ID_CH1, XVPHY_DRP_TXOUT_DIV));

					break;

				case 'h':
					XDp_RxGenerateHpdInterrupt(DpRxSsInst.DpPtr, 5000);
					break;

				case 'e':
					XDpRxSs_ReportDp159BitErrCount(&DpRxSsInst);
					ReadVal = XVphy_ReadReg(
							VIDPHY_BASEADDR,XVPHY_RX_SYM_ERR_CNTR_CH1_2_REG);
					xil_printf("Video PHY(8B10B): Error Counts [Lane1, Lane0] "
							"= [%d, %d]\n\r", (ReadVal>>16), ReadVal&0xFFFF);
					ReadVal = XVphy_ReadReg(
							VIDPHY_BASEADDR,XVPHY_RX_SYM_ERR_CNTR_CH3_4_REG);
					xil_printf("Video PHY(8B10B): Error Counts [Lane3, Lane2] "
							"= [%d, %d]\n\r", (ReadVal>>16), ReadVal&0xFFFF);
					break;

				case 'm':
					xil_printf(" XDP_RX_USER_FIFO_OVERFLOW (0x110) = 0x%x\n\r",
							XDp_ReadReg(DpRxSsInst.DpPtr->Config.BaseAddr,
												XDP_RX_USER_FIFO_OVERFLOW));
					XDpRxSs_ReportMsaInfo(&DpRxSsInst);
					ReportVideoCRC();
					break;

				case 'r':
					xil_printf(
						"Reset Video DTG in DisplayPort Controller...\r\n");
					XDp_RxDtgDis(DpRxSsInst.DpPtr);
					XDp_RxDtgEn(DpRxSsInst.DpPtr);
					break;

				case 'c':
					XDpRxSs_ReportCoreInfo(&DpRxSsInst);
					break;

				case '.':
					AppHelp();
					break;

				case 'n':
			//Read DRP registor
					xil_printf("\n\r CH \n\r");
			for(i=0; i<0x26A; i++)
			{
				int rData;

				//Channel 0
						Xil_Out32(0xa0020040, (0x1000 + i));
						usleep(1);
						rData = Xil_In32(0xa0020050);
						rData &= 0xFFFF;
						xil_printf("%x : %x\n\r", i, rData);
			}
//					xil_printf("\n\r Comm \n\r");
//
//                	for(i=0; i<0xB1; i++)
//                	{
//                		int rData;
//
//                		//Channel 0
//						Xil_Out32(0xa0020060, (0x1000 + i));
//						usleep(100);
//						rData = Xil_In32(0xa0020064);
//						rData &= 0xFFFF;
//						xil_printf("%x : %x\n\r", i, rData);
//                	}


					break;

				default :
					AppHelp();
				break;
			}
		}//end if

		//CRC Handling
		if(DpRxSsInst.VBlankCount>VBLANK_WAIT_COUNT)
		{
			//VBLANK Management
			DpRxSsInst.VBlankCount = 0;

			//Wait for few frames to ensure valid video is received
			xdbg_printf((XDBG_DEBUG_GENERAL),
					"Video Detected --> Link Config: %dx%d, Frame: %dx%d, "
					"MISC0: 0x%x, Mvid=%d, Nvid=%d\n\r",
					(int)DpRxSsInst.UsrOpt.LinkRate*270,
					(int)DpRxSsInst.UsrOpt.LaneCount,
					(int)XDp_ReadReg(DpRxSsInst.DpPtr->Config.BaseAddr,
															XDP_RX_MSA_HRES),
					(int)XDp_ReadReg(DpRxSsInst.DpPtr->Config.BaseAddr,
															XDP_RX_MSA_VHEIGHT),
					(int)XDp_ReadReg(DpRxSsInst.DpPtr->Config.BaseAddr,
															XDP_RX_MSA_MISC0),
					(int)XDp_ReadReg(DpRxSsInst.DpPtr->Config.BaseAddr,
															XDP_RX_MSA_MVID),
					(int)XDp_ReadReg(DpRxSsInst.DpPtr->Config.BaseAddr,
															XDP_RX_MSA_NVID));

			XDp_RxDtgDis(DpRxSsInst.DpPtr);
			XDp_RxDtgEn(DpRxSsInst.DpPtr);
			XDp_RxInterruptDisable(DpRxSsInst.DpPtr,
											XDP_RX_INTERRUPT_MASK_VBLANK_MASK);

			/*
			 * Disable & Enable Audio
			 */
			XDp_RxAudioDis(DpRxSsInst.DpPtr);
			XDp_RxAudioEn(DpRxSsInst.DpPtr);

		    CalculateCRC();

		}
	}//end while(1)
	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* This function initialize required platform specific peripherals.
*
* @param	None.
*
* @return
*		- XST_SUCCESS if required peripherals are initialized and
*		configured successfully.
*		- XST_FAILURE, otherwise.
*
* @note		None.
*
******************************************************************************/
u32 DpRxSs_PlatformInit(void)
{
	u32 Status;

	/*Initialize CRC & Set default Pixel Mode to 1
	*/
	VidFrameCRC.TEST_CRC_SUPPORTED = 1;
	XDp_WriteReg(VIDEO_CRC_BASEADDR,VIDEO_FRAME_CRC_CONFIG,0x10);
	XDp_WriteReg(VIDEO_CRC_BASEADDR,VIDEO_FRAME_CRC_CONFIG,0x1);

	/*Initialize Timer
	*/
	Status = XTmrCtr_Initialize(&TmrCtr, XTIMER0_DEVICE_ID);
	if (Status != XST_SUCCESS){
		xil_printf("ERR:Timer failed to initialize. \r\n");
		return XST_FAILURE;
	}
	XTmrCtr_SetResetValue(&TmrCtr, XTIMER0_DEVICE_ID, TIMER_RESET_VALUE);
	XTmrCtr_Start(&TmrCtr, XTIMER0_DEVICE_ID);

	return Status;
}

/*****************************************************************************/
/**
*
* This function configures Video Phy.
*
* @param	None.
*
* @return
*		- XST_SUCCESS if Video Phy configured successfully.
*		- XST_FAILURE, otherwise.
*
* @note		None.
*
******************************************************************************/
u32 DpRxSs_VideoPhyInit(u16 DeviceId)
{
	XVphy_Config *ConfigPtr;

	/* Obtain the device configuration for the DisplayPort RX Subsystem */
	ConfigPtr = XVphy_LookupConfig(DeviceId);
	if (!ConfigPtr) {
		return XST_FAILURE;
	}

    PLLRefClkSel (&VPhyInst, PHY_User_Config_Table[5].LineRate);

	XVphy_DpInitialize(&VPhyInst, ConfigPtr, 0,
            PHY_User_Config_Table[5].CPLLRefClkSrc,
            PHY_User_Config_Table[5].QPLLRefClkSrc,
            PHY_User_Config_Table[5].TxPLL,
            PHY_User_Config_Table[5].RxPLL,
            PHY_User_Config_Table[5].LineRate);

	PHY_Two_byte_set (&VPhyInst, SET_RX_TO_2BYTE);

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* This function configures DisplayPort RX Subsystem.
*
* @param	None.
*
* @return
*		- XST_SUCCESS if DP RX Subsystem configured successfully.
*		- XST_FAILURE, otherwise.
*
* @note		None.
*
******************************************************************************/
u32 DpRxSs_Setup(void)
{
	u32 ReadVal;

    /*Disable Rx*/
    XDp_WriteReg(DpRxSsInst.DpPtr->Config.BaseAddr, XDP_RX_LINK_ENABLE, 0x0);

    /* Load Custom EDID */
	LoadEDID();

    /*Disable All Interrupts*/
    XDp_RxInterruptDisable(DpRxSsInst.DpPtr, 0xFFFFFFFF);

    /*Enable Training related interrupts*/
    XDp_RxInterruptEnable(DpRxSsInst.DpPtr,
		XDP_RX_INTERRUPT_MASK_TP1_MASK|XDP_RX_INTERRUPT_MASK_TP2_MASK|
			XDP_RX_INTERRUPT_MASK_TP3_MASK|
			XDP_RX_INTERRUPT_MASK_POWER_STATE_MASK|
			XDP_RX_INTERRUPT_MASK_BW_CHANGE_MASK);

	/* Setting AUX Defer Count of Link Status Reads to 8 during Link Training
	 * 8 Defer counts is chosen to handle worst case time interrupt service
	 * load (PL system working at 100 MHz) when working with R5
	 * */
	ReadVal = XDp_ReadReg(DpRxSsInst.DpPtr->Config.BaseAddr,
												XDP_RX_AUX_CLK_DIVIDER);
	ReadVal |= ReadVal | 0x0F000000;
    XDp_WriteReg(DpRxSsInst.DpPtr->Config.BaseAddr,
										XDP_RX_AUX_CLK_DIVIDER, ReadVal);

    /*Setting BS Idle timeout value to long value*/
	XDp_WriteReg(DpRxSsInst.DpPtr->Config.BaseAddr,
									XDP_RX_BS_IDLE_TIME, DP_BS_IDLE_TIMEOUT);

//    /*Updating Vswing Iteration Count*/
//	RxTrainConfig.ChEqOption = 0;
//	RxTrainConfig.ClockRecoveryOption = 1;
//	RxTrainConfig.Itr1Premp = 0;
//	RxTrainConfig.Itr2Premp = 0;
//	RxTrainConfig.Itr3Premp = 0;
//	RxTrainConfig.Itr4Premp = 0;
//	RxTrainConfig.Itr5Premp = 0;
//	RxTrainConfig.MinVoltageSwing = 1;
//	RxTrainConfig.SetPreemp = 0;
//	RxTrainConfig.SetVswing = 0;
//	RxTrainConfig.VswingLoopCount = 2;
//
//	XDp_WriteReg(DpRxSsInst.DpPtr->Config.BaseAddr, XDP_RX_MIN_VOLTAGE_SWING,
//			RxTrainConfig.MinVoltageSwing |
//			(RxTrainConfig.ClockRecoveryOption << 2) |
//			(RxTrainConfig.VswingLoopCount << 4) |
//			(RxTrainConfig.SetVswing << 8) |
//			(RxTrainConfig.ChEqOption << 10) |
//			(RxTrainConfig.SetPreemp << 12) |
//			(RxTrainConfig.Itr1Premp << 14) |
//			(RxTrainConfig.Itr2Premp << 16) |
//			(RxTrainConfig.Itr3Premp << 18) |
//			(RxTrainConfig.Itr4Premp << 20) |
//			(RxTrainConfig.Itr5Premp << 22)
//			);

    /*Enable CRC Support*/
	XDp_WriteReg(DpRxSsInst.DpPtr->Config.BaseAddr, XDP_RX_CRC_CONFIG,
										VidFrameCRC.TEST_CRC_SUPPORTED<<5);

    /*Enable Rx*/
    XDp_WriteReg(DpRxSsInst.DpPtr->Config.BaseAddr, XDP_RX_LINK_ENABLE, 0x1);

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* This function sets up the interrupt system so interrupts can occur for the
* DisplayPort RX Subsystem core. The function is application-specific since
* the actual system may or may not have an interrupt controller. The DPRX
* Subsystem core could be directly connected to a processor without an
* interrupt controller. The user should modify this function to fit the
* application.
*
* @param	None
*
* @return
*		- XST_SUCCESS if interrupt setup was successful.
*		- A specific error code defined in "xstatus.h" if an error
*		occurs.
*
* @note		None.
*
******************************************************************************/
u32 DpRxSs_SetupIntrSystem(void)
{
	u32 Status;
	XINTC *IntcInstPtr = &IntcInst;

	/* Set callbacks for all the interrupts */
	XDpRxSs_SetCallBack(&DpRxSsInst, XDPRXSS_HANDLER_DP_PWR_CHG_EVENT,
				DpRxSs_PowerChangeHandler, &DpRxSsInst);
	XDpRxSs_SetCallBack(&DpRxSsInst, XDPRXSS_HANDLER_DP_NO_VID_EVENT,
				DpRxSs_NoVideoHandler, &DpRxSsInst);
	XDpRxSs_SetCallBack(&DpRxSsInst, XDPRXSS_HANDLER_DP_VBLANK_EVENT,
				DpRxSs_VerticalBlankHandler, &DpRxSsInst);
	XDpRxSs_SetCallBack(&DpRxSsInst, XDPRXSS_HANDLER_DP_TLOST_EVENT,
				DpRxSs_TrainingLostHandler, &DpRxSsInst);
	XDpRxSs_SetCallBack(&DpRxSsInst, XDPRXSS_HANDLER_DP_VID_EVENT,
				DpRxSs_VideoHandler, &DpRxSsInst);
	XDpRxSs_SetCallBack(&DpRxSsInst, XDPRXSS_HANDLER_DP_INFO_PKT_EVENT,
				DpRxSs_InfoPacketHandler, &DpRxSsInst);
	XDpRxSs_SetCallBack(&DpRxSsInst, XDPRXSS_HANDLER_DP_EXT_PKT_EVENT,
				DpRxSs_ExtPacketHandler, &DpRxSsInst);
	XDpRxSs_SetCallBack(&DpRxSsInst, XDPRXSS_HANDLER_DP_TDONE_EVENT,
				DpRxSs_TrainingDoneHandler, &DpRxSsInst);
	XDpRxSs_SetCallBack(&DpRxSsInst, XDPRXSS_HANDLER_UNPLUG_EVENT,
				DpRxSs_UnplugHandler, &DpRxSsInst);
	XDpRxSs_SetCallBack(&DpRxSsInst, XDPRXSS_HANDLER_LINKBW_EVENT,
				DpRxSs_LinkBandwidthHandler, &DpRxSsInst);
	XDpRxSs_SetCallBack(&DpRxSsInst, XDPRXSS_HANDLER_PLL_RESET_EVENT,
				DpRxSs_PllResetHandler, &DpRxSsInst);

	/* Set custom timer wait */
	XDpRxSs_SetUserTimerHandler(&DpRxSsInst, &CustomWaitUs, &TmrCtr);

	/* The configuration parameters of the interrupt controller */
	XScuGic_Config *IntcConfig;

	/* Initialize the interrupt controller driver so that it is ready to
	 * use.
	 */
	IntcConfig = XScuGic_LookupConfig(XINTC_DEVICE_ID);
	if (NULL == IntcConfig) {
		return XST_FAILURE;
	}

	Status = XScuGic_CfgInitialize(IntcInstPtr, IntcConfig,
				IntcConfig->CpuBaseAddress);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/* Connect the device driver handler that will be called when an
	 * interrupt for the device occurs, the handler defined above performs
	 * the specific interrupt processing for the device
	 */
	Status = XScuGic_Connect(IntcInstPtr, XINTC_DPRXSS_DP_INTERRUPT_ID,
			(Xil_InterruptHandler)XDpRxSs_DpIntrHandler,
				&DpRxSsInst);
	if (Status != XST_SUCCESS) {
		xil_printf("ERR: DP RX SS DP interrupt connect failed!\n\r");
		return XST_FAILURE;
	}
	/* Enable the interrupt for the DP device */
	XScuGic_Enable(IntcInstPtr, XINTC_DPRXSS_DP_INTERRUPT_ID);

	/* Initialize the exception table. */
	Xil_ExceptionInit();

	/* Register the interrupt controller handler with the exception
	 * table.
	 */
	Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_INT,
			(Xil_ExceptionHandler)XINTC_HANDLER, IntcInstPtr);

	/* Enable exceptions. */
	Xil_ExceptionEnable();

	return (XST_SUCCESS);
}

/*****************************************************************************/
/**
*
* This function is the callback function for when the power state interrupt
* occurs.
*
* @param	InstancePtr is a pointer to the XDpRxSs instance.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void DpRxSs_PowerChangeHandler(void *InstancePtr)
{
	u32 rdata;

	rdata = XDp_ReadReg(DpRxSsInst.DpPtr->Config.BaseAddr,
											XDP_RX_DPCD_SET_POWER_STATE);

	if(rdata == 0x2)
	{
	    XDp_RxInterruptDisable(DpRxSsInst.DpPtr,
										XDP_RX_INTERRUPT_MASK_UNPLUG_MASK);
		XDpRxSs_Dp159Config(DpRxSsInst.IicPtr, XDPRXSS_DP159_CT_UNPLUG,
				DpRxSsInst.UsrOpt.LinkRate, DpRxSsInst.UsrOpt.LaneCount);
	}
}

/*****************************************************************************/
/**
*
* This function is the callback function for when a no video interrupt occurs.
*
* @param	InstancePtr is a pointer to the XDpRxSs instance.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void DpRxSs_NoVideoHandler(void *InstancePtr)
{
	DpRxSsInst.VBlankCount = 0;
    XDp_RxInterruptEnable(DpRxSsInst.DpPtr,XDP_RX_INTERRUPT_MASK_VBLANK_MASK);

	XDp_RxDtgDis(DpRxSsInst.DpPtr);
	XDp_RxDtgEn(DpRxSsInst.DpPtr);

	XDp_WriteReg(DpRxSsInst.DpPtr->Config.BaseAddr,
								XDP_RX_VIDEO_UNSUPPORTED, 0x1);
	XDp_WriteReg(DpRxSsInst.DpPtr->Config.BaseAddr,
								XDP_RX_AUDIO_UNSUPPORTED, 0x1);
}

/*****************************************************************************/
/**
*
* This function is the callback function for when a vertical blank interrupt
* occurs.
*
* @param	InstancePtr is a pointer to the XDpRxSs instance.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void DpRxSs_VerticalBlankHandler(void *InstancePtr)
{
	DpRxSsInst.VBlankCount++;
}

/*****************************************************************************/
/**
*
* This function is the callback function for when a training lost interrupt
* occurs.
*
* @param	InstancePtr is a pointer to the XDpRxSs instance.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void DpRxSs_TrainingLostHandler(void *InstancePtr)
{
	XDp_RxGenerateHpdInterrupt(DpRxSsInst.DpPtr, 750);
	XDp_RxAudioDis(DpRxSsInst.DpPtr);
}

/*****************************************************************************/
/**
*
* This function is the callback function for when a valid video interrupt
* occurs.
*
* @param	InstancePtr is a pointer to the XDpRxSs instance.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void DpRxSs_VideoHandler(void *InstancePtr)
{
	XDp_WriteReg(DpRxSsInst.DpPtr->Config.BaseAddr,
										XDP_RX_VIDEO_UNSUPPORTED, 0x0);
	XDp_WriteReg(DpRxSsInst.DpPtr->Config.BaseAddr,
										XDP_RX_AUDIO_UNSUPPORTED, 0x0);
}

/*****************************************************************************/
/**
*
* This function is the callback function for when an info packet interrupt
* occurs.
*
* @param	InstancePtr is a pointer to the XDpRxSs instance.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void DpRxSs_InfoPacketHandler(void *InstancePtr)
{
}

/*****************************************************************************/
/**
*
* This function is the callback function for when arrival of external (audio)
* packet interrupt occurs.
*
* @param	InstancePtr is a pointer to the XDpRxSs instance.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void DpRxSs_ExtPacketHandler(void *InstancePtr)
{
}

/*****************************************************************************/
/**
*
* This function is the callback function for when the training done interrupt
* occurs.
*
* @param	InstancePtr is a pointer to the XDpRxSs instance.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void DpRxSs_TrainingDoneHandler(void *InstancePtr)
{
}

/*****************************************************************************/
/**
*
* This function is the callback function for when the unplug event occurs.
*
* @param	InstancePtr is a pointer to the XDpRxSs instance.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void DpRxSs_UnplugHandler(void *InstancePtr)
{
    /*Enable Training related interrupts*/
    XDp_RxInterruptDisable(DpRxSsInst.DpPtr,XDP_RX_INTERRUPT_MASK_ALL_MASK);
    XDp_RxInterruptEnable(DpRxSsInst.DpPtr,
		XDP_RX_INTERRUPT_MASK_TP1_MASK|XDP_RX_INTERRUPT_MASK_TP2_MASK|
			XDP_RX_INTERRUPT_MASK_TP3_MASK|
			XDP_RX_INTERRUPT_MASK_POWER_STATE_MASK|
			XDP_RX_INTERRUPT_MASK_BW_CHANGE_MASK);
	XDp_RxGenerateHpdInterrupt(DpRxSsInst.DpPtr, 5000);
}

/*****************************************************************************/
/**
*
* This function is the callback function for when the link bandwidth change
* occurs.
*
* @param	InstancePtr is a pointer to the XDpRxSs instance.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void DpRxSs_LinkBandwidthHandler(void *InstancePtr)
{
	u32 Status;

	/*Program Video PHY to requested line rate*/
    PLLRefClkSel (&VPhyInst, DpRxSsInst.UsrOpt.LinkRate);

    XVphy_ResetGtPll(&VPhyInst, 0, XVPHY_CHANNEL_ID_CHA, XVPHY_DIR_RX,(TRUE));


	XVphy_PllInitialize(&VPhyInst, 0, XVPHY_CHANNEL_ID_CHA, ONBOARD_REF_CLK,
			DP159_FORWARDED_CLK, XVPHY_PLL_TYPE_QPLL1, XVPHY_PLL_TYPE_CPLL);
    Status = XVphy_ClkInitialize(&VPhyInst, 0, XVPHY_CHANNEL_ID_CHA,
														XVPHY_DIR_RX);

	if(Status!=XST_SUCCESS)
	{
		xdbg_printf((XDBG_DEBUG_GENERAL),"DpRxSs_LinkBandwidthHandler:: "
			"RX GT configuration encountered an error (TP1)\n\r");
	}
}

/*****************************************************************************/
/**
*
* This function is the callback function for PLL reset request.
*
* @param	InstancePtr is a pointer to the XDpRxSs instance.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void DpRxSs_PllResetHandler(void *InstancePtr)
{
	u32 Status_ResetDone;
	u32 Status_PllLock;

/*Issue resets to Video PHY - This API called after line rate is programmed*/
    XVphy_BufgGtReset(&VPhyInst, XVPHY_DIR_RX,(TRUE));
    XVphy_ResetGtPll(&VPhyInst, 0, XVPHY_CHANNEL_ID_CHA, XVPHY_DIR_RX,(TRUE));
    XVphy_ResetGtPll(&VPhyInst, 0, XVPHY_CHANNEL_ID_CHA, XVPHY_DIR_RX,(FALSE));
    XVphy_BufgGtReset(&VPhyInst, XVPHY_DIR_RX,(FALSE));
    Status_ResetDone = XVphy_WaitForResetDone(&VPhyInst, 0,
									XVPHY_CHANNEL_ID_CHA, XVPHY_DIR_RX);
    Status_PllLock = XVphy_WaitForPllLock(&VPhyInst, 0, XVPHY_CHANNEL_ID_CHA);

    if(Status_ResetDone!=XST_SUCCESS)
    {
		xdbg_printf((XDBG_DEBUG_GENERAL),"DpRxSs_PllResetHandler:: "
			"No Reset Done!\n\r");
    }

    if(Status_PllLock!=XST_SUCCESS)
    {
		xdbg_printf((XDBG_DEBUG_GENERAL),"DpRxSs_PllResetHandler:: "
			"No PLL Lock!\n\r");
    }

    /*Enable all interrupts except Unplug*/
    XDp_RxInterruptEnable(DpRxSsInst.DpPtr,XDP_RX_INTERRUPT_MASK_ALL_MASK);
}

/*****************************************************************************/
/**
*
* This function sets GT in 16-bits (2-Byte) or 32-bits (4-Byte) mode.
*
* @param	InstancePtr is a pointer to the Video PHY instance.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void PHY_Two_byte_set (XVphy *InstancePtr, u8 Rx_to_two_byte)
{

	u32 DrpVal;
	u32 WriteVal;
    if (Rx_to_two_byte == 1) {
		DrpVal = XVphy_DrpRead(InstancePtr, 0, XVPHY_CHANNEL_ID_CH1, 0x03);
		DrpVal &= ~0x1E0;
		WriteVal = 0x0;
		WriteVal = DrpVal | 0x60;
		XVphy_DrpWrite(InstancePtr, 0, XVPHY_CHANNEL_ID_CH1, 0x03, WriteVal);
		XVphy_DrpWrite(InstancePtr, 0, XVPHY_CHANNEL_ID_CH2, 0x03, WriteVal);
		XVphy_DrpWrite(InstancePtr, 0, XVPHY_CHANNEL_ID_CH3, 0x03, WriteVal);
		XVphy_DrpWrite(InstancePtr, 0, XVPHY_CHANNEL_ID_CH4, 0x03, WriteVal);

		DrpVal = XVphy_DrpRead(InstancePtr, 0, XVPHY_CHANNEL_ID_CH1, 0x66);
		DrpVal &= ~0x3;
		WriteVal = 0x0;
		WriteVal = DrpVal | 0x0;
		XVphy_DrpWrite(InstancePtr, 0, XVPHY_CHANNEL_ID_CH1, 0x66, WriteVal);
		XVphy_DrpWrite(InstancePtr, 0, XVPHY_CHANNEL_ID_CH2, 0x66, WriteVal);
		XVphy_DrpWrite(InstancePtr, 0, XVPHY_CHANNEL_ID_CH3, 0x66, WriteVal);
		XVphy_DrpWrite(InstancePtr, 0, XVPHY_CHANNEL_ID_CH4, 0x66, WriteVal);
		xil_printf ("RX Channel configured for 2byte mode\r\n");
    }
}

/*****************************************************************************/
/**
*
* This function sets proper ref clk frequency and line rate
*
* @param	InstancePtr is a pointer to the Video PHY instance.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void PLLRefClkSel (XVphy *InstancePtr, u8 link_rate) {
	switch (link_rate) {
	case 0x6:
		XVphy_CfgQuadRefClkFreq(InstancePtr, 0,
							ONBOARD_REF_CLK, 270000000);
			XVphy_CfgQuadRefClkFreq(InstancePtr, 0,
								DP159_FORWARDED_CLK, 81000000);
			XVphy_CfgLineRate(InstancePtr, 0, XVPHY_CHANNEL_ID_CHA,
								XVPHY_DP_LINK_RATE_HZ_162GBPS);
			XVphy_CfgLineRate(InstancePtr, 0, XVPHY_CHANNEL_ID_CMN1,
								XVPHY_DP_LINK_RATE_HZ_162GBPS);
			break;
	case 0x14:
		XVphy_CfgQuadRefClkFreq(InstancePtr, 0,
							ONBOARD_REF_CLK, 270000000);
			XVphy_CfgQuadRefClkFreq(InstancePtr, 0,
								DP159_FORWARDED_CLK, 270000000);
			XVphy_CfgLineRate(InstancePtr, 0, XVPHY_CHANNEL_ID_CHA,
								XVPHY_DP_LINK_RATE_HZ_540GBPS);
			XVphy_CfgLineRate(InstancePtr, 0, XVPHY_CHANNEL_ID_CMN1,
								XVPHY_DP_LINK_RATE_HZ_540GBPS);
			break;
	default:
		XVphy_CfgQuadRefClkFreq(InstancePtr, 0,
							ONBOARD_REF_CLK, 270000000);
			XVphy_CfgQuadRefClkFreq(InstancePtr, 0,
								DP159_FORWARDED_CLK, 135000000);
			XVphy_CfgLineRate(InstancePtr, 0, XVPHY_CHANNEL_ID_CHA,
								XVPHY_DP_LINK_RATE_HZ_270GBPS);
			XVphy_CfgLineRate(InstancePtr, 0, XVPHY_CHANNEL_ID_CMN1,
								XVPHY_DP_LINK_RATE_HZ_270GBPS);
			break;
	}
}

/*****************************************************************************/
/**
*
* This function prints Menu
*
* @param	None.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void AppHelp()
 {
xil_printf("\n\n-----------------------------------------------------\n\r");
xil_printf("--                       Menu                      --\n\r");
xil_printf("-----------------------------------------------------\n\r");
xil_printf("\n\r");
xil_printf(" Select option\n\r");
xil_printf(" s = Report DP Link status  \n\r");
xil_printf(" d = Report VPHY Config/Status  \n\r");
xil_printf(" h = Assert HPD Pulse (5 ms)  \n\r");
xil_printf(" e = Report VPHY and DP159 Error & Status  \n\r");
xil_printf(" c = Core Info  \n\r");
xil_printf(" r = Reset DTG  \n\r");
xil_printf(" m = Report Audio/Video MSA Attributes, Time Stamps, CRC "
				"Values  \n\r");
xil_printf(" . = Show Menu  \n\r");
xil_printf("\n\r");
xil_printf("-----------------------------------------------------\n\r");
 }

/*****************************************************************************/
/**
*
* This function reports CRC values of Video components
*
* @param	None.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void ReportVideoCRC()
 {
	xil_printf ("==========Frame CRC===========\r\n");
	xil_printf ("CRC Cfg     =  0x%x\r\n",XDp_ReadReg(VIDEO_CRC_BASEADDR,0x0));
	xil_printf ("CRC - R/Y   =  0x%x\r\n",XDp_ReadReg(VIDEO_CRC_BASEADDR,0x4)
																	& 0xFFFF);
	xil_printf ("CRC - G/Cr  =  0x%x\r\n",XDp_ReadReg(VIDEO_CRC_BASEADDR,0x4)
																	>> 16);
	xil_printf ("CRC - B/Cb  =  0x%x\r\n",XDp_ReadReg(VIDEO_CRC_BASEADDR,0x8)
																	& 0xFFFF);
 }

/*****************************************************************************/
/**
*
* This function is a non-blocking UART return byte
*
* @param	None.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
char XUartPs_RecvByte_NonBlocking(){
	u32 RecievedByte;
	RecievedByte = XUartPs_ReadReg(STDIN_BASEADDRESS, XUARTPS_FIFO_OFFSET);
	/* Return the byte received */
	return (u8)RecievedByte;
}

/*****************************************************************************/
/**
*
* This function is called when DisplayPort Subsystem core requires delay
* or sleep. It provides timer with predefined amount of loop iterations.
*
* @param	InstancePtr is a pointer to the XDp instance.
*
* @return	None.
*
*
******************************************************************************/
void CustomWaitUs(void *InstancePtr, u32 MicroSeconds)
{

	u32 TimerVal;
	XDp *DpInstance = (XDp *)InstancePtr;
	u32 NumTicks = (MicroSeconds * (DpInstance->Config.SAxiClkHz /
			1000000));

	XTmrCtr_Reset(DpInstance->UserTimerPtr, 0);
	XTmrCtr_Start(DpInstance->UserTimerPtr, 0);

	/* Wait specified number of useconds. */
	do {
		TimerVal = XTmrCtr_GetValue(DpInstance->UserTimerPtr, 0);
	} while (TimerVal < NumTicks);
}

/*****************************************************************************/
/**
*
* This function Calculates CRC values of Video components
*
* @param	None.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void CalculateCRC(void)
{
	/*
	 * Reset CRC Test Counter in DP DPCD Space
	 */
	VidFrameCRC.TEST_CRC_CNT = 0;
	XDp_WriteReg(DpRxSsInst.DpPtr->Config.BaseAddr, XDP_RX_CRC_CONFIG,
			VidFrameCRC.TEST_CRC_SUPPORTED<<5 | VidFrameCRC.TEST_CRC_CNT);

/*Set pixel mode as per lane count - it is default behavior
  User has to adjust this accordingly if there is change in pixel
  width programming
 */
	XDp_WriteReg(VIDEO_CRC_BASEADDR,VIDEO_FRAME_CRC_CONFIG,
									DpRxSsInst.UsrOpt.LaneCount);

	/*Set pixel mode as per lane count - it is default behavior
	  Reset DTG
	 */
	XDp_RxSetUserPixelWidth(DpRxSsInst.DpPtr, DpRxSsInst.UsrOpt.LaneCount);
	XDp_RxDtgDis(DpRxSsInst.DpPtr);
	XDp_RxDtgEn(DpRxSsInst.DpPtr);

	/* Add delay (~40 ms) for Frame CRC to compute on couple of frames
	 */
	CustomWaitUs(DpRxSsInst.DpPtr, 400000);

	/*Read computed values from Frame CRC module and MISC0 for colorimetry
	 */
	VidFrameCRC.Pixel_r  = XDp_ReadReg(VIDEO_CRC_BASEADDR,
										VIDEO_FRAME_CRC_VALUE_G_R)&0xFFFF;
	VidFrameCRC.Pixel_g  = XDp_ReadReg(VIDEO_CRC_BASEADDR,
										VIDEO_FRAME_CRC_VALUE_G_R)>>16;
	VidFrameCRC.Pixel_b  = XDp_ReadReg(VIDEO_CRC_BASEADDR,
										VIDEO_FRAME_CRC_VALUE_B)&0xFFFF;
	VidFrameCRC.Mode_422 = (XDp_ReadReg(DpRxSsInst.DpPtr->Config.BaseAddr,
										XDP_RX_MSA_MISC0)>>1) & 0x3;

	//Write CRC values to DPCD TEST CRC space
	XDp_WriteReg(DpRxSsInst.DpPtr->Config.BaseAddr, XDP_RX_CRC_COMP0,
		(VidFrameCRC.Mode_422==0x1)?0:VidFrameCRC.Pixel_r);
	XDp_WriteReg(DpRxSsInst.DpPtr->Config.BaseAddr, XDP_RX_CRC_COMP1,
		(VidFrameCRC.Mode_422==0x1)?VidFrameCRC.Pixel_b:VidFrameCRC.Pixel_g);
	//Check for 422 format and move CR/CB calculated CRC to G component
	//  place as tester needs this way
	XDp_WriteReg(DpRxSsInst.DpPtr->Config.BaseAddr, XDP_RX_CRC_COMP2,
		(VidFrameCRC.Mode_422==0x1)?VidFrameCRC.Pixel_r:VidFrameCRC.Pixel_b);

	VidFrameCRC.TEST_CRC_CNT = 1;
	XDp_WriteReg(DpRxSsInst.DpPtr->Config.BaseAddr, XDP_RX_CRC_CONFIG,
			VidFrameCRC.TEST_CRC_SUPPORTED<<5 | VidFrameCRC.TEST_CRC_CNT);

	xdbg_printf((XDBG_DEBUG_GENERAL),
			"[Video CRC] R/Y: 0x%x, G/YCr: 0x%x, B/YCb: 0x%x\r\n\n",
			VidFrameCRC.Pixel_r,VidFrameCRC.Pixel_g,VidFrameCRC.Pixel_b);

}

/*****************************************************************************/
/**
*
* This function load EDID content into EDID Memory. User can change as per
* 	their requirement.
*
* @param	None.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void LoadEDID(void)
 {
	int i=0;
	int j=0;
    u8 edid[256] = {
                0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00,
				0x61, 0x2c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                0x00, 0x17, 0x01, 0x04, 0xa5, 0x3c, 0x22, 0x78,
				0x00, 0x1d, 0xf5, 0xae, 0x4f, 0x35, 0xb3, 0x25,
                0x0d, 0x50, 0x54, 0x21, 0x08, 0x00, 0x81, 0x00,
				0xb3, 0x00, 0xd1, 0x00, 0xd1, 0xc0, 0xa9, 0x40,
                0x81, 0x80, 0x01, 0x01, 0x01, 0x01, 0xbe, 0x6e,
				0x00, 0x68, 0xf1, 0x70, 0x5a, 0x80, 0x64, 0x58,
                0x8a, 0x00, 0xba, 0x89, 0x21, 0x00, 0x00, 0x1a,
				0x00, 0x00, 0x00, 0xff, 0x00, 0x0a, 0x20, 0x20,
                0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
				0x20, 0x20, 0x00, 0x00, 0x00, 0xfc, 0x00, 0x58,
                0x49, 0x4c, 0x20, 0x44, 0x50, 0x0a, 0x20, 0x20,
				0x20, 0x20, 0x20, 0x20, 0x00, 0x00, 0x00, 0xfd,
                0x00, 0x31, 0x56, 0x1d, 0x71, 0x1e, 0x00, 0x0a,
				0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x01, 0xea,
                0x02, 0x03, 0x0e, 0xc1, 0x41, 0x90, 0x23, 0x09,
				0x1f, 0x07, 0x83, 0x01, 0x00, 0x00, 0x02, 0x3a,
                0x80, 0x18, 0x71, 0x38, 0x2d, 0x40, 0x58, 0x2c,
				0x45, 0x00, 0x55, 0x50, 0x21, 0x00, 0x00, 0x1e,
                0x01, 0x1d, 0x80, 0x18, 0x71, 0x1c, 0x16, 0x20,
				0x58, 0x2c, 0x25, 0x00, 0x55, 0x50, 0x21, 0x00,
                0x00, 0x9e, 0x01, 0x1d, 0x00, 0x72, 0x51, 0xd0,
				0x1e, 0x20, 0x6e, 0x28, 0x55, 0x00, 0x55, 0x50,
                0x21, 0x00, 0x00, 0x1e, 0x8c, 0x0a, 0xd0, 0x8a,
				0x20, 0xe0, 0x2d, 0x10, 0x10, 0x3e, 0x96, 0x00,
                0x55, 0x50, 0x21, 0x00, 0x00, 0x18, 0x00, 0x00,
				0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
				0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
				0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xbb
    };

    for(i=0;i<(256*4);i=i+(16*4)){
		for(j=i;j<(i+(16*4));j=j+4){
			XDp_WriteReg (VID_EDID_BASEADDR, j, edid[(i/4)+1]);
		}
    }
    for(i=0;i<(256*4);i=i+4){
	XDp_WriteReg (VID_EDID_BASEADDR, i, edid[i/4]);
    }
 }
