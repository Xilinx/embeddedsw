/******************************************************************************
*
* Copyright (C) 2018 Xilinx, Inc.  All rights reserved.
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

#include <stddef.h>
#include <xdp.h>
#include <xdp_hw.h>
#include <xdprxss.h>
#include <xdprxss_mcdp6000.h>
#include <xiic.h>
#include <xiic_l.h>
#include <xil_exception.h>
#include <xil_printf.h>
#include <xil_types.h>
#include <xparameters.h>
#include <xstatus.h>
#include <xtmrctr.h>
#include <xuartlite_l.h>
#include <xvphy.h>
#include <xvphy_dp.h>
#include <xvphy_hw.h>

#include "xvidframe_crc.h"

#ifdef XPAR_INTC_0_DEVICE_ID
/* For MicroBlaze systems. */
#include "xintc.h"
#else
/* For ARM/Zynq SoC systems. */
#include "xscugic.h"
#endif /* XPAR_INTC_0_DEVICE_ID */

#include "xiic.h"
#include "videofmc_defs.h"
#include "ti_lmk03318.h"
#include "idt_8t49n24x.h"
#include "xlib_string.h"
//#include "./MC/mcdp6000.h"
/************************** Constant Definitions *****************************/

/*
* The following constants map to the names of the hardware instances.
* They are only defined here such that a user can easily change all the
* needed device IDs in one place.
* There is only one interrupt controlled to be selected from SCUGIC and GPIO
* INTC. INTC selection is based on INTC parameters defined xparameters.h file.
*/
#define XINTC_DPRXSS_DP_INTERRUPT_ID \
	XPAR_MICROBLAZE_0_AXI_INTC_DP_RX_HIER_0_V_DP_RXSS1_0_DPRXSS_DP_IRQ_INTR

#ifdef XPAR_INTC_0_DEVICE_ID
 #define XINTC_DEVICE_ID	XPAR_INTC_0_DEVICE_ID
 #define IIC_INTR_ID	XPAR_INTC_0_IIC_0_VEC_ID
 #define XINTC			XIntc
 #define XINTC_HANDLER	XIntc_InterruptHandler
#else
 #define INTC_DEVICE_ID		XPAR_SCUGIC_SINGLE_DEVICE_ID
 #define IIC_INTR_ID		XPAR_FABRIC_IIC_0_VEC_ID
 #define INTC			 	XScuGic
 #define INTC_HANDLER		XScuGic_InterruptHandler
#endif


/* The unique device ID of the instances used in example
 */
#define XDPRXSS_DEVICE_ID 	XPAR_DPRXSS_0_DEVICE_ID
#define XVPHY_DEVICE_ID 	XPAR_DP_RX_HIER_0_VID_PHY_CONTROLLER_0_DEVICE_ID
#define XTIMER0_DEVICE_ID 	XPAR_TMRCTR_0_DEVICE_ID

#define VIDEO_CRC_BASEADDR 	XPAR_DP_RX_HIER_0_VIDEO_FRAME_CRC_0_BASEADDR
#define UARTLITE_BASEADDR 	XPAR_PSU_UART_0_BASEADDR
#define VIDPHY_BASEADDR 	XPAR_DP_RX_HIER_0_VID_PHY_CONTROLLER_0_BASEADDR
#define VID_EDID_BASEADDR 	XPAR_DP_RX_HIER_0_VID_EDID_0_BASEADDR
#define IIC_DEVICE_ID 		XPAR_IIC_0_DEVICE_ID

/* DP Specific Defines
 */
#define DPRXSS_LINK_RATE 		XDPRXSS_LINK_BW_SET_810GBPS
#define DPRXSS_LANE_COUNT 		XDPRXSS_LANE_COUNT_SET_4
#define SET_TX_TO_2BYTE            \
	(XPAR_XDP_0_GT_DATAWIDTH/2)
#define SET_RX_TO_2BYTE            \
	(XPAR_XDP_0_GT_DATAWIDTH/2)
#define XDP_RX_CRC_CONFIG 		0x074
#define XDP_RX_CRC_COMP0 		0x078
#define XDP_RX_CRC_COMP1 		0x07C
#define XDP_RX_CRC_COMP2 		0x080
#define XDP_RX_DPC_LINK_QUAL_CONFIG 	0x454
#define XDP_RX_DPC_L01_PRBS_CNTR 	0x45C
#define XDP_RX_DPC_L23_PRBS_CNTR 	0x460
#define XDP_RX_DPCD_LINK_QUAL_PRBS 	0x3

/*
 * User can tune these variables as per their system
 */

/*Max timeout tuned as per tester - AXI Clock=100 MHz
 *Some GPUs may need larger value, So user may tune if needed
 */
#define DP_BS_IDLE_TIMEOUT      0x047868C0//0x0091FFFF
#define VBLANK_WAIT_COUNT       200

/*For compliance, please set AUX_DEFER_COUNT to be 8
 * (Only for ZCU102-ARM R5 based Rx system).
  For Interop, set this to 6.
*/
#define AUX_DEFER_COUNT         6
/* DEFAULT VALUE=0. Enabled programming of
 *Rx Training Algo Register for Debugging Purpose
 */
#define LINK_TRAINING_DEBUG     0

/*EDID Selection*/
#define DP12_EDID_ENABLED 0

/* VPHY Specific Defines
 */
#define XVPHY_RX_SYM_ERR_CNTR_CH1_2_REG    0x084
#define XVPHY_RX_SYM_ERR_CNTR_CH3_4_REG    0x088

#define XVPHY_DRP_CPLL_FBDIV 		0x28
#define XVPHY_DRP_CPLL_REFCLK_DIV 	0x2A
#define XVPHY_DRP_RXOUT_DIV 		0x63
#define XVPHY_DRP_RXCLK25 		0x6D
#define XVPHY_DRP_TXCLK25 		0x7A
#define XVPHY_DRP_TXOUT_DIV 		0x7C
#define XVPHY_DRP_RX_DATA_WIDTH 	0x03
#define XVPHY_DRP_RX_INT_DATA_WIDTH 	0x66
#define XVPHY_DRP_GTHE4_PRBS_ERR_CNTR_LOWER 0x25E
#define XVPHY_DRP_GTHE4_PRBS_ERR_CNTR_UPPER 0x25F

/* Timer Specific Defines
 */
#define TIMER_RESET_VALUE        1000

/***************** Macros (Inline Functions) Definitions *********************/


/**************************** Type Definitions *******************************/
typedef enum {
        ONBOARD_REF_CLK = 1,
        DP159_FORWARDED_CLK = 1, // no DP159 for DP1.4
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
/* Index,         TxPLL,               RxPLL,
 * TxChId,         RxChId,
 * LineRate,         LineRateHz,
 * QPLLRefClkSrc,          CPLLRefClkSrc,    QPLLRefClkFreqHz,CPLLRefClkFreqHz
 * */
  {   0,     XVPHY_PLL_TYPE_CPLL,   XVPHY_PLL_TYPE_CPLL,
          XVPHY_CHANNEL_ID_CHA,     XVPHY_CHANNEL_ID_CHA,
          0x06,    XVPHY_DP_LINK_RATE_HZ_162GBPS,
		  ONBOARD_REF_CLK,    ONBOARD_REF_CLK,     270000000,270000000},
  {   1,     XVPHY_PLL_TYPE_CPLL,   XVPHY_PLL_TYPE_CPLL,
          XVPHY_CHANNEL_ID_CHA,     XVPHY_CHANNEL_ID_CHA,
          0x0A,    XVPHY_DP_LINK_RATE_HZ_270GBPS,
		  ONBOARD_REF_CLK,    ONBOARD_REF_CLK,     270000000,270000000},
  {   2,     XVPHY_PLL_TYPE_CPLL,   XVPHY_PLL_TYPE_CPLL,
          XVPHY_CHANNEL_ID_CHA,     XVPHY_CHANNEL_ID_CHA,
          0x14,    XVPHY_DP_LINK_RATE_HZ_540GBPS,
		  ONBOARD_REF_CLK,    ONBOARD_REF_CLK,     270000000,270000000},
  {   3,     XVPHY_PLL_TYPE_QPLL1,  XVPHY_PLL_TYPE_CPLL,
          XVPHY_CHANNEL_ID_CMN1,    XVPHY_CHANNEL_ID_CHA,
          0x06,    XVPHY_DP_LINK_RATE_HZ_162GBPS,
          ONBOARD_REF_CLK,        ONBOARD_REF_CLK,     270000000,270000000},
  {   4,     XVPHY_PLL_TYPE_QPLL1,  XVPHY_PLL_TYPE_CPLL,
          XVPHY_CHANNEL_ID_CMN1,    XVPHY_CHANNEL_ID_CHA,
          0x0A,    XVPHY_DP_LINK_RATE_HZ_270GBPS,
          ONBOARD_REF_CLK,        ONBOARD_REF_CLK,     270000000,270000000},
  {   5,     XVPHY_PLL_TYPE_QPLL1,  XVPHY_PLL_TYPE_CPLL,
          XVPHY_CHANNEL_ID_CMN1,    XVPHY_CHANNEL_ID_CHA,
          0x14,    XVPHY_DP_LINK_RATE_HZ_540GBPS,
          ONBOARD_REF_CLK,        ONBOARD_REF_CLK,     270000000,270000000},
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
  {   9,     XVPHY_PLL_TYPE_CPLL,   XVPHY_PLL_TYPE_CPLL,
		  XVPHY_CHANNEL_ID_CHA,     XVPHY_CHANNEL_ID_CHA,
		  0x14,    XVPHY_DP_LINK_RATE_HZ_810GBPS,
		  ONBOARD_REF_CLK,        ONBOARD_REF_CLK,         270000000,270000000},

};

/*The structure defines sub-fields of Register 0x214*/
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


typedef struct
{
	u8 sec_id;//DP Specific
	u8 type;
	u8 version;
	u8 length;
	u8 audio_coding_type;
	u8 audio_channel_count;
	u8 sampling_frequency;
	u8 sample_size;
	u8 level_shift;
	u8 downmix_inhibit;
	u8 channel_allocation;
	u16 info_length;
	u8 frame_count;
} XilAudioInfoFrame;

/*The structure defines Generic Frame Packet fields*/
typedef struct
{
	u32 frame_count;
	u32 frame_count_q;
	u8 Header[4];
	u8 Payload[32];
} XilAudioExtFrame;

XilAudioInfoFrame AudioinfoFrame;
XilAudioExtFrame  SdpExtFrame;
XilAudioExtFrame  SdpExtFrame_q;

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
void DpRxSs_BWChangeHandler(void *InstancePtr);
void DpRxSs_AccessLaneSetHandler(void *InstancePtr);
void DpRxSs_AccessLinkQualHandler(void *InstancePtr);
void DpRxSs_AccessErrorCounterHandler(void *InstancePtr);
void DpRxSs_CRCTestEventHandler(void *InstancePtr);
void XDp_RxInterruptDisable1(XDp *InstancePtr, u32 Mask);
void XDp_RxInterruptEnable1(XDp *InstancePtr, u32 Mask);

int i2c_write_dp141(u32 I2CBaseAddress, u8 I2CSlaveAddress, u16 RegisterAddress,
		u8 Value);
int VideoFMC_Init(void);

void Print_InfoPkt();
void Print_ExtPkt();
/************************** Variable Definitions *****************************/

XDpRxSs DpRxSsInst;    /* The DPRX Subsystem instance.*/
XINTC IntcInst;        /* The interrupt controller instance. */
XVphy VPhyInst;    /* The DPRX Subsystem instance.*/
XTmrCtr TmrCtr; /* Timer instance.*/
Video_CRC_Config VidFrameCRC; /* Video Frame CRC instance */
DP_Rx_Training_Algo_Config RxTrainConfig;
XIic IicInstance;	/* I2C bus for MC6000 and IDT */
/************************** Function Definitions *****************************/


/*****************************************************************************/
/**
*
* This function initializes VFMC.
*
* @param    None.
*
* @return    None.
*
* @note        None.
*
******************************************************************************/
int VideoFMC_Init(void){
	int Status;
	u8 Buffer[2];
	int ByteCount;

	xil_printf("VFMC: Setting IO Expanders...\n\r");

    XIic_Config *ConfigPtr_IIC;     /* Pointer to configuration data */
    /*
     * Initialize the IIC driver so that it is ready to use.
     */
    ConfigPtr_IIC = XIic_LookupConfig(IIC_DEVICE_ID);
    if (ConfigPtr_IIC == NULL) {
            return XST_FAILURE;
    }

    Status = XIic_CfgInitialize(&IicInstance, ConfigPtr_IIC,
		ConfigPtr_IIC->BaseAddress);
    if (Status != XST_SUCCESS) {
            return XST_FAILURE;
    }


    XIic_Reset(&IicInstance);

	/* Set the I2C Mux to select the HPC FMC */
	Buffer[0] = 0x02;
	ByteCount = XIic_Send(XPAR_IIC_0_BASEADDR, I2C_MUX_ADDR,
			(u8*)Buffer, 1, XIIC_STOP);
	if (ByteCount != 1) {
		xil_printf("Failed to set the I2C Mux.\n\r");
	    return XST_FAILURE;
	}

//	I2C_Scan(XPAR_IIC_0_BASEADDR);

	/* Configure VFMC IO Expander 0:
	 * Disable Si5344
	 * Set primary clock source for LMK03318 to IOCLKp(0)
	 * Set secondary clock source for LMK03318 to IOCLKp(1)
	 * Disable LMK61E2*/
	Buffer[0] = 0x52;
	ByteCount = XIic_Send(XPAR_IIC_0_BASEADDR, I2C_VFMCEXP_0_ADDR,
			(u8*)Buffer, 1, XIIC_STOP);
	if (ByteCount != 1) {
		xil_printf("Failed to set the I2C IO Expander.\n\r");
		return XST_FAILURE;
	}

	/* Configure VFMC IO Expander 1:
	 * Enable LMK03318 -> In a power-down state the I2C bus becomes unusable.
	 * Select IDT8T49N241 clock as source for FMC_GT_CLKp(0)
	 * Select IDT8T49N241 clock as source for FMC_GT_CLKp(1)
	 * Enable IDT8T49N241 */
	Buffer[0] = 0x1E;
	ByteCount = XIic_Send(XPAR_IIC_0_BASEADDR, I2C_VFMCEXP_1_ADDR,
			(u8*)Buffer, 1, XIIC_STOP);
	if (ByteCount != 1) {
		xil_printf("Failed to set the I2C IO Expander.\n\r");
		return XST_FAILURE;
	}

	xil_printf(" done!\n\r");

	Status = IDT_8T49N24x_Init(XPAR_IIC_0_BASEADDR, I2C_IDT8N49_ADDR);
	if (Status != XST_SUCCESS) {
		xil_printf("Failed to initialize IDT 8T49N241.\n\r");
		return XST_FAILURE;
	}

//	Status = TI_LMK03318_Init(XPAR_IIC_0_BASEADDR, I2C_LMK03318_ADDR);
	Status = TI_LMK03318_PowerDown(XPAR_IIC_0_BASEADDR, I2C_LMK03318_ADDR);
	if (Status != XST_SUCCESS) {
		xil_printf("Failed to initialize TI LMK03318.\n\r");
		return XST_FAILURE;
	}

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* This is the main function for XDpRxSs interrupt example. If the
* DpRxSs_Main function which setup the system succeeds, this function
* will wait for the interrupts.
*
* @param    None.
*
* @return
*        - XST_FAILURE if the interrupt example was unsuccessful.
*
* @note        Unless setup failed, main will never return since
*        DpRxSs_Main is blocking (it is waiting on interrupts).
*
******************************************************************************/
int main()
{
	u32 Status;

	xil_printf("------------------------------------------\n\r");
	xil_printf("DisplayPort RX Only Example\n\r");
	xil_printf("(c) 2018 by Xilinx\n\r");
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
* @param    DeviceId is the unique device ID of the DisplayPort RX
*        Subsystem core.
*
* @return
*        - XST_FAILURE if the system setup failed.
*        - XST_SUCCESS should never return since this function, if setup
*          was successful, is blocking.
*
* @note        If system setup was successful, this function is blocking in
*        order to illustrate interrupt handling taking place for
*        different types interrupts.
*        Refer xdprxss.h file for more info.
*
******************************************************************************/
u32 DpRxSs_Main(u16 DeviceId)
{
	u32 Status;
	XDpRxSs_Config *ConfigPtr;
	u8 UserInput;
	u32 ReadVal=0;
	u16 DrpVal;
	char CommandKey;
	char CmdKey[2];
	unsigned int Command;

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
		xil_printf("\n\rINFO:DPRXSS is MST enabled. DPRXSS can "
				"be switched to SST/MST\n\r\n\r");
	} else {
		xil_printf("\n\rINFO:DPRXSS is SST enabled. DPRXSS works "
				"only in SST mode.\n\r\n\r");
	}
	Status = DpRxSs_SetupIntrSystem();
	if (Status != XST_SUCCESS) {
		xil_printf("ERR: Interrupt system setup failed.\n\r");
		return XST_FAILURE;
	}
	/*Megachips Retimer Initialization*/
	XDpRxSs_McDp6000_init(&DpRxSsInst, DpRxSsInst.IicPtr->BaseAddress); //XPAR_IIC_0_BASEADDR);
	/* Set Link rate and lane count to maximum */
	// Setting Max capability as 5.4G, A D1.4 source would read the
	// extended cap and train for 8.1G
	XDpRxSs_SetLinkRate(&DpRxSsInst, 0x14) ;//DPRXSS_LINK_RATE);
	XDpRxSs_SetLaneCount(&DpRxSsInst, DPRXSS_LANE_COUNT);
	/* Start DPRX Subsystem set */
	Status = XDpRxSs_Start(&DpRxSsInst);
	if (Status != XST_SUCCESS) {
		xil_printf("ERR:DPRX SS start failed\n\r");
		return XST_FAILURE;
	}

	/* Setup Video Phy, left to the user for implementation */
	DpRxSs_VideoPhyInit(XVPHY_DEVICE_ID);

	/* Setup DPRX SS, left to the user for implementation */
	DpRxSs_Setup();

	AppHelp();
	while (1) {
		CommandKey = xil_getc(0xff);
//		Command = atoi(&CommandKey);
//		UserInput = XUartPs_RecvByte_NonBlocking();
		if (CommandKey != 0) {
			xil_printf("UserInput: %c\r\n", CommandKey);

			switch(CommandKey) {
			case '2':
				/* Reset the AUX logic from DP RX */
			    XDp_WriteReg(DpRxSsInst.DpPtr->Config.BaseAddr,
				     XDP_RX_SOFT_RESET,
				     XDP_RX_SOFT_RESET_AUX_MASK);
			    break;

			case 's':
				xil_printf("DP Link Status --->\r\n");
				XDpRxSs_ReportLinkInfo(&DpRxSsInst);
				break;

			case 'd':
				xil_printf("Video PHY Config/Status --->\r\n");
				xil_printf(" RCS (0x10) = 0x%x\n\r",
					   XVphy_ReadReg(VIDPHY_BASEADDR,
							 XVPHY_REF_CLK_SEL_REG));
				xil_printf(" PR  (0x14) = 0x%x\n\r",
					   XVphy_ReadReg(VIDPHY_BASEADDR,
							 XVPHY_PLL_RESET_REG));
				xil_printf(" PLS (0x18) = 0x%x\n\r",
					   XVphy_ReadReg(VIDPHY_BASEADDR,
							 XVPHY_PLL_LOCK_STATUS_REG));
				xil_printf(" TXI (0x1C) = 0x%x\n\r",
					   XVphy_ReadReg(VIDPHY_BASEADDR,
							 XVPHY_TX_INIT_REG));
				xil_printf(" TXIS(0x20) = 0x%x\n\r",
				XVphy_ReadReg(VIDPHY_BASEADDR,
					      XVPHY_TX_INIT_STATUS_REG));
				xil_printf(" RXI (0x24) = 0x%x\n\r",
					   XVphy_ReadReg(VIDPHY_BASEADDR,
							 XVPHY_RX_INIT_REG));
				xil_printf(" RXIS(0x28) = 0x%x\n\r",
					   XVphy_ReadReg(VIDPHY_BASEADDR,
							 XVPHY_RX_INIT_STATUS_REG));

				XVphy_DrpRd(&VPhyInst, 0, XVPHY_CHANNEL_ID_CH1,
					XVPHY_DRP_CPLL_FBDIV,&DrpVal);
				xil_printf(" GT DRP Addr (XVPHY_DRP_CPLL_FBDIV) "
					   "= 0x%x, Val = 0x%x\r\n",
					   XVPHY_DRP_CPLL_FBDIV,DrpVal);

				XVphy_DrpRd(&VPhyInst, 0, XVPHY_CHANNEL_ID_CH1,
					XVPHY_DRP_CPLL_REFCLK_DIV,&DrpVal);
				xil_printf(" GT DRP Addr (XVPHY_DRP_CPLL_REFCLK_DIV) "
					   "= 0x%x, Val = 0x%x\r\n",
					   XVPHY_DRP_CPLL_REFCLK_DIV,DrpVal);

				XVphy_DrpRd(&VPhyInst, 0, XVPHY_CHANNEL_ID_CH1,
					XVPHY_DRP_RXOUT_DIV,&DrpVal);
				xil_printf(" GT DRP Addr (XVPHY_DRP_RXOUT_DIV) = 0x%x, "
					   "Val = 0x%x\r\n",
					   XVPHY_DRP_RXOUT_DIV,DrpVal);

				XVphy_DrpRd(&VPhyInst, 0, XVPHY_CHANNEL_ID_CH1,
					XVPHY_DRP_TXOUT_DIV,&DrpVal);
				xil_printf(" GT DRP Addr (XVPHY_DRP_TXOUT_DIV) = 0x%x, "
					   "Val = 0x%x\r\n",
					    XVPHY_DRP_TXOUT_DIV,DrpVal);

				break;

			case 'h':
				XDp_RxGenerateHpdInterrupt(DpRxSsInst.DpPtr, 5000);
				break;

			case 'e':
//				XDpRxSs_ReportDp159BitErrCount(&DpRxSsInst);
				ReadVal = XVphy_ReadReg(VIDPHY_BASEADDR,
							XVPHY_RX_SYM_ERR_CNTR_CH1_2_REG);
				xil_printf("Video PHY(8B10B): Error Counts [Lane1, Lane0] "
				            "= [%d, %d]\n\r", (ReadVal>>16), ReadVal&0xFFFF);
				ReadVal = XVphy_ReadReg(VIDPHY_BASEADDR,
							XVPHY_RX_SYM_ERR_CNTR_CH3_4_REG);
				xil_printf("Video PHY(8B10B): Error Counts [Lane3, Lane2] "
					   "= [%d, %d]\n\r", (ReadVal>>16), ReadVal&0xFFFF);

				XVphy_DrpRd(&VPhyInst, 0, XVPHY_CHANNEL_ID_CH1,
					    XVPHY_DRP_GTHE4_PRBS_ERR_CNTR_LOWER, &DrpVal);
				xil_printf ("Lane0 (Lower) is %d,\r\n", DrpVal);
				XVphy_DrpRd(&VPhyInst, 0, XVPHY_CHANNEL_ID_CH1,
						XVPHY_DRP_GTHE4_PRBS_ERR_CNTR_UPPER, &DrpVal);
				xil_printf ("Lane0 (Upper) is %d,\r\n", DrpVal);

				XVphy_DrpRd(&VPhyInst, 0, XVPHY_CHANNEL_ID_CH2,
						XVPHY_DRP_GTHE4_PRBS_ERR_CNTR_LOWER, &DrpVal);
				xil_printf ("Lane1 (Lower) is %d,\r\n", DrpVal);
				XVphy_DrpRd(&VPhyInst, 0, XVPHY_CHANNEL_ID_CH2,
						XVPHY_DRP_GTHE4_PRBS_ERR_CNTR_UPPER, &DrpVal);
				xil_printf ("Lane1 (Upper) is %d,\r\n", DrpVal);;


				XVphy_DrpRd(&VPhyInst, 0, XVPHY_CHANNEL_ID_CH3,
						XVPHY_DRP_GTHE4_PRBS_ERR_CNTR_LOWER, &DrpVal);
				xil_printf ("Lane2 (Lower) is %d,\r\n", DrpVal);
				XVphy_DrpRd(&VPhyInst, 0, XVPHY_CHANNEL_ID_CH3,
						XVPHY_DRP_GTHE4_PRBS_ERR_CNTR_UPPER, &DrpVal);
				xil_printf ("Lane2 (Upper) is %d,\r\n", DrpVal);
				XVphy_DrpRd(&VPhyInst, 0, XVPHY_CHANNEL_ID_CH4,
						XVPHY_DRP_GTHE4_PRBS_ERR_CNTR_LOWER, &DrpVal);
				xil_printf ("Lane3 (Lower) is %d,\r\n", DrpVal);
				XVphy_DrpRd(&VPhyInst, 0, XVPHY_CHANNEL_ID_CH4,
						XVPHY_DRP_GTHE4_PRBS_ERR_CNTR_UPPER, &DrpVal);
				xil_printf ("Lane3 (Upper) is %d,\r\n", DrpVal);


				xil_printf ("==========MCDP6000 Debug Data===========\r\n");
				XDpRxSs_MCDP6000_Read_ErrorCounters(XPAR_IIC_0_BASEADDR,
						I2C_MCDP6000_ADDR);
				xil_printf("0x0754: %08x\n\r", XDpRxSs_MCDP6000_GetRegister(
						XPAR_IIC_0_BASEADDR, I2C_MCDP6000_ADDR, 0x0754));
				xil_printf("0x0B20: %08x\n\r", XDpRxSs_MCDP6000_GetRegister(
						XPAR_IIC_0_BASEADDR, I2C_MCDP6000_ADDR, 0x0B20));
				xil_printf("0x0B24: %08x\n\r", XDpRxSs_MCDP6000_GetRegister(
						XPAR_IIC_0_BASEADDR, I2C_MCDP6000_ADDR, 0x0B24));
				xil_printf("0x0B28: %08x\n\r", XDpRxSs_MCDP6000_GetRegister(
						XPAR_IIC_0_BASEADDR, I2C_MCDP6000_ADDR, 0x0B28));
				xil_printf("0x0B2C: %08x\n\r", XDpRxSs_MCDP6000_GetRegister(
						XPAR_IIC_0_BASEADDR, I2C_MCDP6000_ADDR, 0x0B2C));
				xil_printf("0x0B2C: %08x\n\r", XDpRxSs_MCDP6000_GetRegister(
						XPAR_IIC_0_BASEADDR, I2C_MCDP6000_ADDR, 0x061C));
				xil_printf("0x0B2C: %08x\n\r", XDpRxSs_MCDP6000_GetRegister(
						XPAR_IIC_0_BASEADDR, I2C_MCDP6000_ADDR, 0x0504));
				xil_printf("0x0B2C: %08x\n\r", XDpRxSs_MCDP6000_GetRegister(
						XPAR_IIC_0_BASEADDR, I2C_MCDP6000_ADDR, 0x0604));
				break;

			case 'm':
				xil_printf(" XDP_RX_USER_FIFO_OVERFLOW (0x110) = 0x%x\n\r",
				XDp_ReadReg(DpRxSsInst.DpPtr->Config.BaseAddr,
					    XDP_RX_USER_FIFO_OVERFLOW));
				XDpRxSs_ReportMsaInfo(&DpRxSsInst);
				ReportVideoCRC();
				xil_printf(" XDP_RX_LINE_RESET_DISABLE (0x008) = 0x%x\n\r",
				XDp_ReadReg(DpRxSsInst.DpPtr->Config.BaseAddr,
					    XDP_RX_LINE_RESET_DISABLE));
				break;

			case 'r':
				xil_printf("Reset Video DTG in DisplayPort Controller...\r\n");
				XDp_RxDtgDis(DpRxSsInst.DpPtr);
				XDp_RxDtgEn(DpRxSsInst.DpPtr);
				break;

			case 'c':
				XDpRxSs_ReportCoreInfo(&DpRxSsInst);
				break;

			case '.':
			    AppHelp();
			    break;

			default :
			    AppHelp();
				break;
			}
		}//end if

		/* Info Frame Handling
		 * Info frame is sent once per frame and
		 * is static for that config
		 * Capture new Info Frame whenever config changes
		 * */
		if (AudioinfoFrame.frame_count != 0) {
			Print_InfoPkt();
			XDp_RxInterruptDisable(DpRxSsInst.DpPtr,
					XDP_RX_INTERRUPT_MASK_INFO_PKT_MASK);
			AudioinfoFrame.frame_count=0;
		}

		/* Ext Frame Handling */
		if (SdpExtFrame.Header[1] != SdpExtFrame_q.Header[1]) {
			Print_ExtPkt();
			SdpExtFrame_q = SdpExtFrame;
		}

		/* CRC Handling */
		if(DpRxSsInst.VBlankCount>VBLANK_WAIT_COUNT) {
			/* VBLANK Management */
			DpRxSsInst.VBlankCount = 0;

			/* Wait for few frames to ensure valid video is received */
			xil_printf("Video Detected --> Link Config: %dx%d, "
			           "Frame: %dx%d, MISC0: 0x%x, Mvid=%d, Nvid=%d \r",
				(int)DpRxSsInst.UsrOpt.LinkRate * 270,
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

			/* Check for RB Resolution */
			XDp_RxSetLineReset(DpRxSsInst.DpPtr,XDP_TX_STREAM_ID1);
			xil_printf(" Line Reset=0x%x\n\r",
				(int)XDp_ReadReg(DpRxSsInst.DpPtr->Config.BaseAddr,
						 XDP_RX_LINE_RESET_DISABLE));

			XDp_RxDtgDis(DpRxSsInst.DpPtr);
			XDp_RxDtgEn(DpRxSsInst.DpPtr);
			XDp_RxInterruptDisable(DpRxSsInst.DpPtr,
					   XDP_RX_INTERRUPT_MASK_VBLANK_MASK);

			/* Disable & Enable Audio */
			XDpRxSs_AudioDisable(&DpRxSsInst);
			XDpRxSs_AudioEnable(&DpRxSsInst);

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
* @param    None.
*
* @return
*        - XST_SUCCESS if required peripherals are initialized and
*        configured successfully.
*        - XST_FAILURE, otherwise.
*
* @note        None.
*
******************************************************************************/
u32 DpRxSs_PlatformInit(void)
{
	u32 Status;

	/* Initialize CRC & Set default Pixel Mode to 1 */
	XVidFrameCrc_Initialize(&VidFrameCRC);

	/* Initialize Timer */
	Status = XTmrCtr_Initialize(&TmrCtr, XTIMER0_DEVICE_ID);
	if (Status != XST_SUCCESS) {
		xil_printf("ERR:Timer failed to initialize. \r\n");
		return XST_FAILURE;
	}

	XTmrCtr_SetResetValue(&TmrCtr, XTIMER0_DEVICE_ID, TIMER_RESET_VALUE);
	XTmrCtr_Start(&TmrCtr, XTIMER0_DEVICE_ID);

	VideoFMC_Init();
	IDT_8T49N24x_SetClock(XPAR_IIC_0_BASEADDR, I2C_IDT8N49_ADDR,
			      0, 270000000, TRUE);

	return Status;
}

/*****************************************************************************/
/**
*
* This function configures Video Phy.
*
* @param    None.
*
* @return
*        - XST_SUCCESS if Video Phy configured successfully.
*        - XST_FAILURE, otherwise.
*
* @note        None.
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

	PHY_Two_byte_set(&VPhyInst, SET_RX_TO_2BYTE);

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* This function configures DisplayPort RX Subsystem.
*
* @param    None.
*
* @return
*        - XST_SUCCESS if DP RX Subsystem configured successfully.
*        - XST_FAILURE, otherwise.
*
* @note        None.
*
******************************************************************************/
u32 DpRxSs_Setup(void)
{
	u32 ReadVal;

	/*Disable Rx*/
	XDp_WriteReg(DpRxSsInst.DpPtr->Config.BaseAddr,
		     XDP_RX_LINK_ENABLE, 0x0);

	/* Load Custom EDID */
	LoadEDID();

	/* Disable All Interrupts*/
	XDp_RxInterruptDisable(DpRxSsInst.DpPtr, 0xFFFFFFFF);
	XDp_RxInterruptDisable1(DpRxSsInst.DpPtr, 0xFFFFFFFF);

	/* Enable Training related interrupts*/
	XDp_RxInterruptEnable(DpRxSsInst.DpPtr,
			  XDP_RX_INTERRUPT_MASK_TP1_MASK |
			  XDP_RX_INTERRUPT_MASK_TP2_MASK |
			  XDP_RX_INTERRUPT_MASK_TP3_MASK |
			  XDP_RX_INTERRUPT_MASK_POWER_STATE_MASK |
			  XDP_RX_INTERRUPT_MASK_CRC_TEST_MASK |
			  XDP_RX_INTERRUPT_MASK_BW_CHANGE_MASK);

	XDp_RxInterruptEnable1(DpRxSsInst.DpPtr,
				   XDP_RX_INTERRUPT_MASK_TP4_MASK |
			   XDP_RX_INTERRUPT_MASK_ACCESS_LANE_SET_MASK |
			   XDP_RX_INTERRUPT_MASK_ACCESS_LINK_QUAL_MASK |
			   XDP_RX_INTERRUPT_MASK_ACCESS_ERROR_COUNTER_MASK);

	/* Setting AUX Defer Count of Link Status Reads to 8 during Link
	 * Training 8 Defer counts is chosen to handle worst case time
	 * interrupt service load (PL system working at 100 MHz) when
	 * working with R5.
	 * */
	ReadVal = XDp_ReadReg(DpRxSsInst.DpPtr->Config.BaseAddr,
			      XDP_RX_AUX_CLK_DIVIDER);
	ReadVal = ReadVal & 0xF0FF00FF;
	ReadVal = ReadVal | (AUX_DEFER_COUNT<<24);
	XDp_WriteReg(DpRxSsInst.DpPtr->Config.BaseAddr,
		     XDP_RX_AUX_CLK_DIVIDER, ReadVal);

	/*Setting BS Idle timeout value to long value*/
	XDp_WriteReg(DpRxSsInst.DpPtr->Config.BaseAddr,
		     XDP_RX_BS_IDLE_TIME, DP_BS_IDLE_TIMEOUT);

	if (LINK_TRAINING_DEBUG==1) {
		/*Updating Vswing Iteration Count*/
		RxTrainConfig.ChEqOption = 0;
		RxTrainConfig.ClockRecoveryOption = 1;
		RxTrainConfig.Itr1Premp = 0;
		RxTrainConfig.Itr2Premp = 0;
		RxTrainConfig.Itr3Premp = 0;
		RxTrainConfig.Itr4Premp = 0;
		RxTrainConfig.Itr5Premp = 0;
		RxTrainConfig.MinVoltageSwing = 1;
		RxTrainConfig.SetPreemp = 1;
		RxTrainConfig.SetVswing = 0;
		RxTrainConfig.VswingLoopCount = 3;

		XDp_WriteReg(DpRxSsInst.DpPtr->Config.BaseAddr,
			     XDP_RX_MIN_VOLTAGE_SWING,
			     RxTrainConfig.MinVoltageSwing |
			     (RxTrainConfig.ClockRecoveryOption << 2) |
			     (RxTrainConfig.VswingLoopCount << 4) |
			     (RxTrainConfig.SetVswing << 8) |
			     (RxTrainConfig.ChEqOption << 10) |
			     (RxTrainConfig.SetPreemp << 12) |
			     (RxTrainConfig.Itr1Premp << 14) |
			     (RxTrainConfig.Itr2Premp << 16) |
			     (RxTrainConfig.Itr3Premp << 18) |
			     (RxTrainConfig.Itr4Premp << 20) |
			     (RxTrainConfig.Itr5Premp << 22)
			     );
	}

	/*Enable CRC Support*/
	XDp_WriteReg(DpRxSsInst.DpPtr->Config.BaseAddr, XDP_RX_CRC_CONFIG,
		     VidFrameCRC.TEST_CRC_SUPPORTED << 5);

	/*Enable Rx*/
	XDp_WriteReg(DpRxSsInst.DpPtr->Config.BaseAddr,
		     XDP_RX_LINK_ENABLE, 0x1);

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
* @param    None
*
* @return
*        - XST_SUCCESS if interrupt setup was successful.
*        - A specific error code defined in "xstatus.h" if an error
*        occurs.
*
* @note        None.
*
******************************************************************************/
u32 DpRxSs_SetupIntrSystem(void)
{
	u32 Status;
	XINTC *IntcInstPtr = &IntcInst;

	Status = XIntc_Initialize(IntcInstPtr, XINTC_DEVICE_ID);
	        if (Status != XST_SUCCESS) {
	                xil_printf("Intc initialization failed!\n\r");
	                return XST_FAILURE;
	        }

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
	XDpRxSs_SetCallBack(&DpRxSsInst, XDPRXSS_HANDLER_DP_TDONE_EVENT,
			    DpRxSs_TrainingDoneHandler, &DpRxSsInst);
	XDpRxSs_SetCallBack(&DpRxSsInst, XDPRXSS_HANDLER_UNPLUG_EVENT,
			    DpRxSs_UnplugHandler, &DpRxSsInst);
	XDpRxSs_SetCallBack(&DpRxSsInst, XDPRXSS_HANDLER_LINKBW_EVENT,
			    DpRxSs_LinkBandwidthHandler, &DpRxSsInst);
	XDpRxSs_SetCallBack(&DpRxSsInst, XDPRXSS_HANDLER_PLL_RESET_EVENT,
			    DpRxSs_PllResetHandler, &DpRxSsInst);
	XDpRxSs_SetCallBack(&DpRxSsInst, XDPRXSS_HANDLER_DP_BW_CHG_EVENT,
			    DpRxSs_BWChangeHandler, &DpRxSsInst);
//	XDpRxSs_SetCallBack(&DpRxSsInst, XDPRXSS_HANDLER_ACCESS_LANE_SET_EVENT,
//			    DpRxSs_AccessLaneSetHandler, &DpRxSsInst);
	XDpRxSs_SetCallBack(&DpRxSsInst, XDPRXSS_HANDLER_ACCESS_LINK_QUAL_EVENT,
			    DpRxSs_AccessLinkQualHandler, &DpRxSsInst);
	XDpRxSs_SetCallBack(&DpRxSsInst,
			    XDPRXSS_HANDLER_ACCESS_ERROR_COUNTER_EVENT,
			    DpRxSs_AccessErrorCounterHandler, &DpRxSsInst);
	XDpRxSs_SetCallBack(&DpRxSsInst, XDPRXSS_HANDLER_DP_CRC_TEST_EVENT,
			    DpRxSs_CRCTestEventHandler, &DpRxSsInst);
	XDpRxSs_SetCallBack(&DpRxSsInst, XDPRXSS_HANDLER_DP_INFO_PKT_EVENT,
			&DpRxSs_InfoPacketHandler, &DpRxSsInst);
	XDpRxSs_SetCallBack(&DpRxSsInst, XDPRXSS_HANDLER_DP_EXT_PKT_EVENT,
			&DpRxSs_ExtPacketHandler, &DpRxSsInst);

	/* Set custom timer wait */
	XDpRxSs_SetUserTimerHandler(&DpRxSsInst, &CustomWaitUs, &TmrCtr);


#ifdef XPAR_INTC_0_DEVICE_ID

	/*
	 * Initialize the interrupt controller driver so that it's ready to use.
	 */
	Status = XIntc_Initialize(&IntcInst, XINTC_DEVICE_ID);

	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
//	 * Connect the device driver handler that will be called when an
//	 * interrupt for the device occurs, the handler defined above performs
//	 * the specific interrupt processing for the device.
//	 */
//	Status = XIntc_Connect(&IntcInst, IIC_INTR_ID,
//				   (XInterruptHandler) XIic_InterruptHandler,
//				   IicInstance);
//	if (Status != XST_SUCCESS) {
//		return XST_FAILURE;
//	}
//
//	/*
//	 * Start the interrupt controller so interrupts are enabled for all
//	 * devices that cause interrupts.
//	 */
//	Status = XIntc_Start(&IntcInst, XIN_REAL_MODE);
//	if (Status != XST_SUCCESS) {
//		return XST_FAILURE;
//	}
//
//	/*
//	 * Enable the interrupts for the IIC device.
//	 */
//	XIntc_Enable(&IntcInst, IIC_INTR_ID);
	/* Connect the device driver handler that will be called when an
	 * interrupt for the device occurs, the handler defined above performs
	 * the specific interrupt processing for the device
	 */
	Status = XIntc_Connect(IntcInstPtr, XINTC_DPRXSS_DP_INTERRUPT_ID, \
				(XInterruptHandler) XDpRxSs_DpIntrHandler, \
				&DpRxSsInst);
	if (Status != XST_SUCCESS) {
		xil_printf("ERR: DP RX SS DP interrupt connect failed!\n\r");
		return XST_FAILURE;
	}

		/*
		 * Start the interrupt controller so interrupts are enabled for all
		 * devices that cause interrupts.
		 */
		Status = XIntc_Start(IntcInstPtr, XIN_REAL_MODE);
		if (Status != XST_SUCCESS) {
			return XST_FAILURE;
		}

	/* Enable the interrupt for the DP device */
	XIntc_Enable(IntcInstPtr, XINTC_DPRXSS_DP_INTERRUPT_ID);

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

	Status = XScuGic_CfgInitialize(&IntcInst, IntcConfig,
					IntcConfig->CpuBaseAddress);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	XScuGic_SetPriorityTriggerType(&IntcInst, IIC_INTR_ID,
					0xA0, 0x3);

	/*
	 * Connect the interrupt handler that will be called when an
	 * interrupt occurs for the device.
	 */
	Status = XScuGic_Connect(&IntcInst, IIC_INTR_ID,
				 (Xil_InterruptHandler)XIic_InterruptHandler,
				 IicInstance);
	if (Status != XST_SUCCESS) {
		return Status;
	}

	/*
	 * Enable the interrupt for the IIC device.
	 */
	XScuGic_Enable(&IntcInst, IIC_INTR_ID);

#endif

	/* Initialize the exception table. */
	Xil_ExceptionInit();

	/* Register the interrupt controller handler with the
	 * exception table.
	 */
	Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_INT,
				     (Xil_ExceptionHandler)XINTC_HANDLER,
				     IntcInstPtr);

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
* @param    InstancePtr is a pointer to the XDpRxSs instance.
*
* @return    None.
*
* @note        None.
*
******************************************************************************/
void DpRxSs_PowerChangeHandler(void *InstancePtr)
{

}

/*****************************************************************************/
/**
*
* This function is the callback function for when a no video interrupt occurs.
*
* @param    InstancePtr is a pointer to the XDpRxSs instance.
*
* @return    None.
*
* @note        None.
*
******************************************************************************/
void DpRxSs_NoVideoHandler(void *InstancePtr)
{
	DpRxSsInst.VBlankCount = 0;
	XDp_RxInterruptEnable(DpRxSsInst.DpPtr,
			      XDP_RX_INTERRUPT_MASK_VBLANK_MASK);

	XDp_RxDtgDis(DpRxSsInst.DpPtr);
	XDp_RxDtgEn(DpRxSsInst.DpPtr);

	/* Reset CRC Test Counter in DP DPCD Space */
	XVidFrameCrc_Reset();
	VidFrameCRC.TEST_CRC_CNT = 0;
	XDp_WriteReg(DpRxSsInst.DpPtr->Config.BaseAddr,
		     XDP_RX_CRC_CONFIG,
		     (VidFrameCRC.TEST_CRC_SUPPORTED << 5 |
		      VidFrameCRC.TEST_CRC_CNT));

	AudioinfoFrame.frame_count=0;
	XDp_RxInterruptEnable(DpRxSsInst.DpPtr,
			XDP_RX_INTERRUPT_MASK_INFO_PKT_MASK);
}

/*****************************************************************************/
/**
*
* This function is the callback function for when a vertical blank interrupt
* occurs.
*
* @param    InstancePtr is a pointer to the XDpRxSs instance.
*
* @return    None.
*
* @note        None.
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
* @param    InstancePtr is a pointer to the XDpRxSs instance.
*
* @return    None.
*
* @note        None.
*
******************************************************************************/
void DpRxSs_TrainingLostHandler(void *InstancePtr)
{
	XDp_RxGenerateHpdInterrupt(DpRxSsInst.DpPtr, 750);
	XDpRxSs_AudioDisable(&DpRxSsInst);
	DpRxSsInst.VBlankCount = 0;
//	xil_printf ("Training Lost\r\n");
}

/*****************************************************************************/
/**
*
* This function is the callback function for when a valid video interrupt
* occurs.
*
* @param    InstancePtr is a pointer to the XDpRxSs instance.
*
* @return    None.
*
* @note        None.
*
******************************************************************************/
void DpRxSs_VideoHandler(void *InstancePtr)
{
}

/*****************************************************************************/
/**
*
* This function is the callback function for when the training done interrupt
* occurs.
*
* @param    InstancePtr is a pointer to the XDpRxSs instance.
*
* @return    None.
*
* @note        None.
*
******************************************************************************/
void DpRxSs_TrainingDoneHandler(void *InstancePtr)
{
//	xil_printf ("Training done\r\n");
}

/*****************************************************************************/
/**
*
* This function is the callback function for when the unplug event occurs.
*
* @param    InstancePtr is a pointer to the XDpRxSs instance.
*
* @return    None.
*
* @note        None.
*
******************************************************************************/
void DpRxSs_UnplugHandler(void *InstancePtr)
{
	/* Disable & Enable Audio */
	XDpRxSs_AudioDisable(&DpRxSsInst);
	AudioinfoFrame.frame_count = 0;
	SdpExtFrame.Header[1] = 0;
	SdpExtFrame_q.Header[1] = 0;
	SdpExtFrame.frame_count = 0;
	SdpExtFrame.frame_count = 0;
	DpRxSsInst.VBlankCount = 0;

	/*Enable Training related interrupts*/
	XDp_RxInterruptDisable(DpRxSsInst.DpPtr,
			       XDP_RX_INTERRUPT_MASK_ALL_MASK);
	XDp_RxInterruptDisable1(DpRxSsInst.DpPtr, 0xFFFFFFFF);

	XDp_RxInterruptEnable(DpRxSsInst.DpPtr,
			      XDP_RX_INTERRUPT_MASK_TP1_MASK |
			      XDP_RX_INTERRUPT_MASK_TP2_MASK |
			      XDP_RX_INTERRUPT_MASK_TP3_MASK |
			      XDP_RX_INTERRUPT_MASK_POWER_STATE_MASK |
			      XDP_RX_INTERRUPT_MASK_CRC_TEST_MASK |
			      XDP_RX_INTERRUPT_MASK_BW_CHANGE_MASK);

	XDp_RxInterruptEnable1(DpRxSsInst.DpPtr,
			       XDP_RX_INTERRUPT_MASK_TP4_MASK |
			       XDP_RX_INTERRUPT_MASK_ACCESS_LANE_SET_MASK |
			       XDP_RX_INTERRUPT_MASK_ACCESS_LINK_QUAL_MASK |
			       XDP_RX_INTERRUPT_MASK_ACCESS_ERROR_COUNTER_MASK);

	XDp_RxGenerateHpdInterrupt(DpRxSsInst.DpPtr, 5000);
	xil_printf ("Cable Unplugged !!!\r\n");
}

/*****************************************************************************/
/**
*
* This function is the callback function for when the link bandwidth change
* occurs.
*
* @param    InstancePtr is a pointer to the XDpRxSs instance.
*
* @return    None.
*
* @note        None.
*
******************************************************************************/
void DpRxSs_LinkBandwidthHandler(void *InstancePtr)
{
	u32 Status;
	/*Program Video PHY to requested line rate*/
	PLLRefClkSel (&VPhyInst, DpRxSsInst.UsrOpt.LinkRate);

	XVphy_ResetGtPll(&VPhyInst, 0, XVPHY_CHANNEL_ID_CHA,
			 XVPHY_DIR_RX,(TRUE));

	XVphy_PllInitialize(&VPhyInst, 0, XVPHY_CHANNEL_ID_CHA,
			    ONBOARD_REF_CLK, ONBOARD_REF_CLK,
			    XVPHY_PLL_TYPE_QPLL1, XVPHY_PLL_TYPE_CPLL);
	XVphy_ClkInitialize(&VPhyInst, 0, XVPHY_CHANNEL_ID_CHA, XVPHY_DIR_RX);

	/*Overriding MC programming to work in DP mode always:
	 * Paranoid operation to ensure MC is in good state*/
        Status = XDpRxSs_MCDP6000_SetRegister(XPAR_IIC_0_BASEADDR, I2C_MCDP6000_ADDR,
	                                                      0x0504, 0x0000705E);

}

/*****************************************************************************/
/**
*
* This function is the callback function for PLL reset request.
*
* @param    InstancePtr is a pointer to the XDpRxSs instance.
*
* @return    None.
*
* @note        None.
*
******************************************************************************/
void DpRxSs_PllResetHandler(void *InstancePtr)
{

	/* Issue resets to Video PHY - This API
	 * called after line rate is programmed */
	XVphy_BufgGtReset(&VPhyInst, XVPHY_DIR_RX,(TRUE));
	XVphy_ResetGtPll(&VPhyInst, 0, XVPHY_CHANNEL_ID_CHA,
			 XVPHY_DIR_RX,(TRUE));
	XVphy_ResetGtPll(&VPhyInst, 0, XVPHY_CHANNEL_ID_CHA,
			 XVPHY_DIR_RX, (FALSE));
	XVphy_BufgGtReset(&VPhyInst, XVPHY_DIR_RX, (FALSE));
	XVphy_WaitForResetDone(&VPhyInst, 0, XVPHY_CHANNEL_ID_CHA,
			       XVPHY_DIR_RX);
	XVphy_WaitForPllLock(&VPhyInst, 0, XVPHY_CHANNEL_ID_CHA);

	/*Enable all interrupts except Unplug*/
	XDp_RxInterruptEnable(DpRxSsInst.DpPtr,
			      XDP_RX_INTERRUPT_MASK_ALL_MASK);
}

/*****************************************************************************/
/**
*
* This function is the callback function for PLL reset request.
*
* @param    InstancePtr is a pointer to the XDpRxSs instance.
*
* @return    None.
*
* @note        None.
*
******************************************************************************/
void DpRxSs_BWChangeHandler(void *InstancePtr)
{
//    MCDP6000_ResetDpPath(XPAR_IIC_0_BASEADDR, I2C_MCDP6000_ADDR);
}

/*****************************************************************************/
/**
*
* This function is the callback function for Access lane set request.
*
* @param    InstancePtr is a pointer to the XDpRxSs instance.
*
* @return    None.
*
* @note        None.
*
******************************************************************************/
void DpRxSs_AccessLaneSetHandler(void *InstancePtr)
{
}

/*****************************************************************************/
/**
*
* This function is the callback function for Test CRC Event request.
*
* @param    InstancePtr is a pointer to the XDpRxSs instance.
*
* @return    None.
*
* @note        None.
*
******************************************************************************/
void DpRxSs_CRCTestEventHandler(void *InstancePtr)
{
	u16 ReadVal;
	u32 TrainingAlgoValue;

	ReadVal = XDp_ReadReg(DpRxSsInst.DpPtr->Config.BaseAddr,
			      XDP_RX_CRC_CONFIG);

	/*Record Training Algo Value - to be restored in non-phy test mode*/
	TrainingAlgoValue = XDp_ReadReg(DpRxSsInst.Config.BaseAddress,
					XDP_RX_MIN_VOLTAGE_SWING);

	/*Refer to DPCD 0x270 Register*/
	if((ReadVal & 0x8000) == 0x8000) {
		/*Enable PHY test mode - Set Min voltage swing to 0*/
		XDp_WriteReg(DpRxSsInst.DpPtr->Config.BaseAddr,
			     XDP_RX_MIN_VOLTAGE_SWING,
			     (TrainingAlgoValue & 0xFFFFFFFC) | 0x80000000);

		/*Disable Training timeout*/
		ReadVal = XDp_ReadReg(DpRxSsInst.Config.BaseAddress,
				      XDP_RX_CDR_CONTROL_CONFIG);
		XDp_WriteReg(DpRxSsInst.DpPtr->Config.BaseAddr,
			     XDP_RX_CDR_CONTROL_CONFIG, ReadVal | 0x40000000);
	} else {
		/* Disable PHY test mode & Set min
		 * voltage swing back to level 1 */
		XDp_WriteReg(DpRxSsInst.DpPtr->Config.BaseAddr,
			     XDP_RX_MIN_VOLTAGE_SWING,
			     (TrainingAlgoValue & 0x7FFFFFFF) | 0x1);

		/*Enable Training timeout*/
		ReadVal = XDp_ReadReg(DpRxSsInst.Config.BaseAddress,
				      XDP_RX_CDR_CONTROL_CONFIG);
		XDp_WriteReg(DpRxSsInst.DpPtr->Config.BaseAddr,
			     XDP_RX_CDR_CONTROL_CONFIG, ReadVal & 0xBFFFFFFF);
	}
}


/*****************************************************************************/
/**
*
* This function is the callback function for Access link qual request.
*
* @param    InstancePtr is a pointer to the XDpRxSs instance.
*
* @return    None.
*
* @note        None.
*
******************************************************************************/
void DpRxSs_AccessLinkQualHandler(void *InstancePtr)
{
	u32 ReadVal;
	u32 DrpVal;

	ReadVal = XDp_ReadReg(DpRxSsInst.DpPtr->Config.BaseAddr,
			      XDP_RX_DPC_LINK_QUAL_CONFIG);

	xil_printf("DpRxSs_AccessLinkQualHandler : 0x%x\r\n", ReadVal);

	/* Check for PRBS Mode*/
	if ((ReadVal & 0x00000007) == XDP_RX_DPCD_LINK_QUAL_PRBS) {
		/* Enable PRBS Mode in Video PHY*/
		DrpVal = XVphy_ReadReg(VPhyInst.Config.BaseAddr,
				       XVPHY_RX_CONTROL_REG);
		DrpVal = DrpVal | 0x10101010;
		XVphy_WriteReg(VPhyInst.Config.BaseAddr,
			       XVPHY_RX_CONTROL_REG, DrpVal);

		/* Reset PRBS7 Counters*/
		DrpVal = XVphy_ReadReg(VPhyInst.Config.BaseAddr,
				       XVPHY_RX_CONTROL_REG);
		DrpVal = DrpVal | 0x08080808;
		XDp_WriteReg(VPhyInst.Config.BaseAddr,
			     XVPHY_RX_CONTROL_REG, DrpVal);
		DrpVal = XVphy_ReadReg(VPhyInst.Config.BaseAddr,
				       XVPHY_RX_CONTROL_REG);
		DrpVal = DrpVal & 0xF7F7F7F7;
		XVphy_WriteReg(VPhyInst.Config.BaseAddr,
			       XVPHY_RX_CONTROL_REG, DrpVal);

		/* Set PRBS mode in Retimer*/
		XDpRxSs_MCDP6000_EnablePrbs7_Rx(XPAR_IIC_0_BASEADDR,
					I2C_MCDP6000_ADDR);
		XDpRxSs_MCDP6000_ClearCounter(XPAR_IIC_0_BASEADDR,
				      I2C_MCDP6000_ADDR);
//    	MCDP6000_EnableCounter(XPAR_IIC_0_BASEADDR, I2C_MCDP6000_ADDR);
	} else {
		/*Disable PRBS Mode in Video PHY*/
		DrpVal = XVphy_ReadReg(VPhyInst.Config.BaseAddr,
				       XVPHY_RX_CONTROL_REG);
		DrpVal = DrpVal & 0xEFEFEFEF;
		XVphy_WriteReg(VPhyInst.Config.BaseAddr,
			       XVPHY_RX_CONTROL_REG, DrpVal);

		/*Disable PRBS mode in Retimer*/
		XDpRxSs_MCDP6000_DisablePrbs7_Rx(XPAR_IIC_0_BASEADDR,
					 I2C_MCDP6000_ADDR);
		XDpRxSs_MCDP6000_ClearCounter(XPAR_IIC_0_BASEADDR,
				      I2C_MCDP6000_ADDR);
//    	MCDP6000_EnableCounter(XPAR_IIC_0_BASEADDR, I2C_MCDP6000_ADDR);
	}
}

/*****************************************************************************/
/**
*
* This function is the callback function for Access prbs error count.
*
* @param    InstancePtr is a pointer to the XDpRxSs instance.
*
* @return    None.
*
* @note        None.
*
******************************************************************************/
void DpRxSs_AccessErrorCounterHandler(void *InstancePtr)
{
	u16 DrpVal;
	u16 DrpVal_lower_lane0;
	u16 DrpVal_lower_lane1;
	u16 DrpVal_lower_lane2;
	u16 DrpVal_lower_lane3;

	u32 DrpVal1;

	xil_printf("DpRxSs_AccessErrorCounterHandler\r\n");


	/*Read PRBS Error Counter Value from Video PHY*/

	/*Lane 0 - Store only lower 16 bits*/
	XVphy_DrpRd(&VPhyInst, 0, XVPHY_CHANNEL_ID_CH1,
		    XVPHY_DRP_GTHE4_PRBS_ERR_CNTR_LOWER,
		    &DrpVal_lower_lane0);
	XVphy_DrpRd(&VPhyInst, 0, XVPHY_CHANNEL_ID_CH1,
		    XVPHY_DRP_GTHE4_PRBS_ERR_CNTR_UPPER,
		    &DrpVal);

	/*Lane 1 - Store only lower 16 bits*/
	XVphy_DrpRd(&VPhyInst, 0, XVPHY_CHANNEL_ID_CH2,
		    XVPHY_DRP_GTHE4_PRBS_ERR_CNTR_LOWER,
		    &DrpVal_lower_lane1);
	XVphy_DrpRd(&VPhyInst, 0, XVPHY_CHANNEL_ID_CH2,
		    XVPHY_DRP_GTHE4_PRBS_ERR_CNTR_UPPER, &DrpVal);

	/*Lane 2 - Store only lower 16 bits*/
	XVphy_DrpRd(&VPhyInst, 0, XVPHY_CHANNEL_ID_CH3,
		    XVPHY_DRP_GTHE4_PRBS_ERR_CNTR_LOWER,
		    &DrpVal_lower_lane2);
	XVphy_DrpRd(&VPhyInst, 0, XVPHY_CHANNEL_ID_CH3,
		    XVPHY_DRP_GTHE4_PRBS_ERR_CNTR_UPPER, &DrpVal);

	/*Lane 3 - Store only lower 16 bits*/
	XVphy_DrpRd(&VPhyInst, 0, XVPHY_CHANNEL_ID_CH4,
		    XVPHY_DRP_GTHE4_PRBS_ERR_CNTR_LOWER,
		    &DrpVal_lower_lane3);
	XVphy_DrpRd(&VPhyInst, 0, XVPHY_CHANNEL_ID_CH4,
		    XVPHY_DRP_GTHE4_PRBS_ERR_CNTR_UPPER, &DrpVal);

	/*Write into DP Core - Validity bit and lower 15 bit counter value*/
	XDp_WriteReg(DpRxSsInst.DpPtr->Config.BaseAddr,
		     XDP_RX_DPC_L01_PRBS_CNTR,
		     (0x8000 | DrpVal_lower_lane0) |
		     ((0x8000 | DrpVal_lower_lane1) << 16));
	XDp_WriteReg(DpRxSsInst.DpPtr->Config.BaseAddr,
		     XDP_RX_DPC_L23_PRBS_CNTR,
		     (0x8000 | DrpVal_lower_lane2) |
		     ((0x8000 | DrpVal_lower_lane3) << 16));

	XDpRxSs_MCDP6000_Read_ErrorCounters(XPAR_IIC_0_BASEADDR, I2C_MCDP6000_ADDR);

	xil_printf("0x061C: %08x\n\r", XDpRxSs_MCDP6000_GetRegister(XPAR_IIC_0_BASEADDR,
							    I2C_MCDP6000_ADDR,
							    0x061C));
	xil_printf("0x0504: %08x\n\r", XDpRxSs_MCDP6000_GetRegister(XPAR_IIC_0_BASEADDR,
							    I2C_MCDP6000_ADDR,
							    0x0504));
	xil_printf("0x0604: %08x\n\r", XDpRxSs_MCDP6000_GetRegister(XPAR_IIC_0_BASEADDR,
							    I2C_MCDP6000_ADDR,
							    0x0604));
	xil_printf("0x12BC: %08x\n\r", XDpRxSs_MCDP6000_GetRegister(XPAR_IIC_0_BASEADDR,
							    I2C_MCDP6000_ADDR,
							    0x12BC));
	xil_printf("0x12E4: %08x\n\r", XDpRxSs_MCDP6000_GetRegister(XPAR_IIC_0_BASEADDR,
							    I2C_MCDP6000_ADDR,
							    0x12E4));

	/* Reset PRBS7 Counters */
	DrpVal1 = XVphy_ReadReg(VPhyInst.Config.BaseAddr,
			       XVPHY_RX_CONTROL_REG);
	DrpVal1 = DrpVal1 | 0x08080808;
	XDp_WriteReg(VPhyInst.Config.BaseAddr,
		     XVPHY_RX_CONTROL_REG, DrpVal1);
	DrpVal1 = XVphy_ReadReg(VPhyInst.Config.BaseAddr,
			       XVPHY_RX_CONTROL_REG);
	DrpVal1 = DrpVal1 & 0xF7F7F7F7;
	XVphy_WriteReg(VPhyInst.Config.BaseAddr,
		       XVPHY_RX_CONTROL_REG, DrpVal1);
}

/*****************************************************************************/
/**
*
* This function sets GT in 16-bits (2-Byte) or 32-bits (4-Byte) mode.
*
* @param    InstancePtr is a pointer to the Video PHY instance.
*
* @return    None.
*
* @note        None.
*
******************************************************************************/
void PHY_Two_byte_set (XVphy *InstancePtr, u8 Rx_to_two_byte)
{

    u16 DrpVal;
    u16 WriteVal;

    if (Rx_to_two_byte == 1) {
        XVphy_DrpRd(InstancePtr, 0, XVPHY_CHANNEL_ID_CH1,
			XVPHY_DRP_RX_DATA_WIDTH,&DrpVal);
        DrpVal &= ~0x1E0;
        WriteVal = 0x0;
        WriteVal = DrpVal | 0x60;
        XVphy_DrpWr(InstancePtr, 0, XVPHY_CHANNEL_ID_CH1,
			XVPHY_DRP_RX_DATA_WIDTH, WriteVal);
        XVphy_DrpWr(InstancePtr, 0, XVPHY_CHANNEL_ID_CH2,
			XVPHY_DRP_RX_DATA_WIDTH, WriteVal);
        XVphy_DrpWr(InstancePtr, 0, XVPHY_CHANNEL_ID_CH3,
			XVPHY_DRP_RX_DATA_WIDTH, WriteVal);
        XVphy_DrpWr(InstancePtr, 0, XVPHY_CHANNEL_ID_CH4,
			XVPHY_DRP_RX_DATA_WIDTH, WriteVal);

        XVphy_DrpRd(InstancePtr, 0, XVPHY_CHANNEL_ID_CH1,
			XVPHY_DRP_RX_INT_DATA_WIDTH,&DrpVal);
        DrpVal &= ~0x3;
        WriteVal = 0x0;
        WriteVal = DrpVal | 0x0;
        XVphy_DrpWr(InstancePtr, 0, XVPHY_CHANNEL_ID_CH1,
			XVPHY_DRP_RX_INT_DATA_WIDTH, WriteVal);
        XVphy_DrpWr(InstancePtr, 0, XVPHY_CHANNEL_ID_CH2,
			XVPHY_DRP_RX_INT_DATA_WIDTH, WriteVal);
        XVphy_DrpWr(InstancePtr, 0, XVPHY_CHANNEL_ID_CH3,
			XVPHY_DRP_RX_INT_DATA_WIDTH, WriteVal);
        XVphy_DrpWr(InstancePtr, 0, XVPHY_CHANNEL_ID_CH4,
			XVPHY_DRP_RX_INT_DATA_WIDTH, WriteVal);
        xil_printf ("RX Channel configured for 2byte mode\r\n");
    }
}

/*****************************************************************************/
/**
*
* This function sets proper ref clk frequency and line rate
*
* @param    InstancePtr is a pointer to the Video PHY instance.
*
* @return    None.
*
* @note        None.
*
******************************************************************************/
void PLLRefClkSel (XVphy *InstancePtr, u8 link_rate) {
	switch (link_rate) {
	case 0x6:
		XVphy_CfgQuadRefClkFreq(InstancePtr, 0, ONBOARD_REF_CLK,
					XVPHY_DP_REF_CLK_FREQ_HZ_270);
		XVphy_CfgQuadRefClkFreq(InstancePtr, 0, DP159_FORWARDED_CLK,
					XVPHY_DP_REF_CLK_FREQ_HZ_270);
		XVphy_CfgLineRate(InstancePtr, 0, XVPHY_CHANNEL_ID_CHA,
				  XVPHY_DP_LINK_RATE_HZ_162GBPS);
		XVphy_CfgLineRate(InstancePtr, 0, XVPHY_CHANNEL_ID_CMN1,
				  XVPHY_DP_LINK_RATE_HZ_162GBPS);
		break;
	case 0x14:
		XVphy_CfgQuadRefClkFreq(InstancePtr, 0, ONBOARD_REF_CLK,
					XVPHY_DP_REF_CLK_FREQ_HZ_270);
		XVphy_CfgQuadRefClkFreq(InstancePtr, 0, DP159_FORWARDED_CLK,
					XVPHY_DP_REF_CLK_FREQ_HZ_270);
		XVphy_CfgLineRate(InstancePtr, 0, XVPHY_CHANNEL_ID_CHA,
				  XVPHY_DP_LINK_RATE_HZ_540GBPS);
		XVphy_CfgLineRate(InstancePtr, 0, XVPHY_CHANNEL_ID_CMN1,
				  XVPHY_DP_LINK_RATE_HZ_540GBPS);
		break;
	case 0x1E:
		XVphy_CfgQuadRefClkFreq(InstancePtr, 0, ONBOARD_REF_CLK,
					XVPHY_DP_REF_CLK_FREQ_HZ_270);
		XVphy_CfgQuadRefClkFreq(InstancePtr, 0, DP159_FORWARDED_CLK,
					XVPHY_DP_REF_CLK_FREQ_HZ_270);
		XVphy_CfgLineRate(InstancePtr, 0, XVPHY_CHANNEL_ID_CHA,
				  XVPHY_DP_LINK_RATE_HZ_810GBPS);
		XVphy_CfgLineRate(InstancePtr, 0, XVPHY_CHANNEL_ID_CMN1,
				  XVPHY_DP_LINK_RATE_HZ_810GBPS);
		break;
	default:
		XVphy_CfgQuadRefClkFreq(InstancePtr, 0, ONBOARD_REF_CLK,
					XVPHY_DP_REF_CLK_FREQ_HZ_270);
		XVphy_CfgQuadRefClkFreq(InstancePtr, 0,
					DP159_FORWARDED_CLK, XVPHY_DP_REF_CLK_FREQ_HZ_270);
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
* @param    None.
*
* @return    None.
*
* @note        None.
*
******************************************************************************/
void AppHelp()
{
	xil_printf("\n\n\r");
	xil_printf("-----------------------------------------------------\n\r");
	xil_printf("--                       Menu                      --\n\r");
	xil_printf("-----------------------------------------------------\n\r");
	xil_printf("\n\r");
	xil_printf(" Select option\n\r");
	xil_printf(" 2 = Reset AUX Logic  \n\r");
	xil_printf(" s = Report DP Link status  \n\r");
	xil_printf(" d = Report VPHY Config/Status  \n\r");
	xil_printf(" h = Assert HPD Pulse (5 ms)  \n\r");
	xil_printf(" e = Report VPHY Error & Status  \n\r");
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
* @param    None.
*
* @return    None.
*
* @note        None.
*
******************************************************************************/
void ReportVideoCRC()
{
	XVidFrameCrc_Report();
}

#if 1
/*****************************************************************************/
/**
*
* This function is a non-blocking UART return byte
*
* @param    None.
*
* @return    None.
*
* @note        None.
*
******************************************************************************/
char XUartPs_RecvByte_NonBlocking()
{
	u32 RecievedByte;
	RecievedByte = XUartLite_ReadReg(STDIN_BASEADDRESS,
			XUL_RX_FIFO_OFFSET);
	/* Return the byte received */
	return (u8)RecievedByte;
}

#endif
/*****************************************************************************/
/**
*
* This function is called when DisplayPort Subsystem core requires delay
* or sleep. It provides timer with predefined amount of loop iterations.
*
* @param    InstancePtr is a pointer to the XDp instance.
*
* @return    None.
*
*
******************************************************************************/
void CustomWaitUs(void *InstancePtr, u32 MicroSeconds)
{
	u32 TimerVal;
	XDp *DpInstance = (XDp *)InstancePtr;
	u32 NumTicks = (MicroSeconds *
			(DpInstance->Config.SAxiClkHz / 1000000));

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
* @param    None.
*
* @return    None.
*
* @note        None.
*
******************************************************************************/
void CalculateCRC(void)
{
	/* Reset CRC Test Counter in DP DPCD Space */
	VidFrameCRC.TEST_CRC_CNT = 0;
	XDp_WriteReg(DpRxSsInst.DpPtr->Config.BaseAddr, XDP_RX_CRC_CONFIG,
		     (VidFrameCRC.TEST_CRC_SUPPORTED << 5 |
		      VidFrameCRC.TEST_CRC_CNT));

	/* Set pixel mode as per lane count - it is default behavior
	 * User has to adjust this accordingly if there is change in pixel
	 * width programming
	 * */
	XVidFrameCrc_WriteReg(VIDEO_CRC_BASEADDR, VIDEO_FRAME_CRC_CONFIG,
			      DpRxSsInst.UsrOpt.LaneCount);

	/* Set pixel mode as per lane count - it
	 * is default behavior Reset DTG */
	XDp_RxSetUserPixelWidth(DpRxSsInst.DpPtr,
				DpRxSsInst.UsrOpt.LaneCount);
	XDp_RxDtgDis(DpRxSsInst.DpPtr);
	XDp_RxDtgEn(DpRxSsInst.DpPtr);

	/* Add delay (~40 ms) for Frame CRC to compute on couple of frames */
	CustomWaitUs(DpRxSsInst.DpPtr, 400000);

	/* Read computed values from Frame CRC
	 * module and MISC0 for colorimetry */
	VidFrameCRC.Pixel_r  =
		XVidFrameCrc_ReadReg(VIDEO_CRC_BASEADDR,
				    VIDEO_FRAME_CRC_VALUE_G_R) & 0xFFFF;
	VidFrameCRC.Pixel_g  =
		XVidFrameCrc_ReadReg(VIDEO_CRC_BASEADDR,
				    VIDEO_FRAME_CRC_VALUE_G_R) >> 16;
	VidFrameCRC.Pixel_b  =
		XVidFrameCrc_ReadReg(VIDEO_CRC_BASEADDR,
				    VIDEO_FRAME_CRC_VALUE_B) & 0xFFFF;
	VidFrameCRC.Mode_422 =
		(XVidFrameCrc_ReadReg(DpRxSsInst.DpPtr->Config.BaseAddr,
				      XDP_RX_MSA_MISC0) >> 1) & 0x3;

	/* Write CRC values to DPCD TEST CRC space */
	XDp_WriteReg(DpRxSsInst.DpPtr->Config.BaseAddr, XDP_RX_CRC_COMP0,
		     (VidFrameCRC.Mode_422 == 0x1) ? 0 : VidFrameCRC.Pixel_r);
	XDp_WriteReg(DpRxSsInst.DpPtr->Config.BaseAddr, XDP_RX_CRC_COMP1,
		     (VidFrameCRC.Mode_422 == 0x1) ?
		      VidFrameCRC.Pixel_b : VidFrameCRC.Pixel_g);
	/* Check for 422 format and move CR/CB calculated CRC
	 * to G component place as tester needs this way
	 * */
	XDp_WriteReg(DpRxSsInst.DpPtr->Config.BaseAddr, XDP_RX_CRC_COMP2,
		     (VidFrameCRC.Mode_422==0x1) ?
		      VidFrameCRC.Pixel_r : VidFrameCRC.Pixel_b);

	VidFrameCRC.TEST_CRC_CNT = 1;
	XDp_WriteReg(DpRxSsInst.DpPtr->Config.BaseAddr, XDP_RX_CRC_CONFIG,
		     (VidFrameCRC.TEST_CRC_SUPPORTED << 5 |
		      VidFrameCRC.TEST_CRC_CNT));

	xil_printf("[Video CRC] R/Cr: 0x%x, G/Y: 0x%x, B/Cb: 0x%x\r\n\n",
		   VidFrameCRC.Pixel_r, VidFrameCRC.Pixel_g,
		   VidFrameCRC.Pixel_b);
}

/*****************************************************************************/
/**
*
* This function load EDID content into EDID Memory. User can change as per
*     their requirement.
*
* @param    None.
*
* @return    None.
*
* @note        None.
*
******************************************************************************/
void LoadEDID(void)
{
	int i=0;
	int j=0;

#if(DP12_EDID_ENABLED)
	unsigned char edid[256] = {
		0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00,
		0x61, 0x2C, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x0E, 0x19, 0x01, 0x04, 0xB5, 0x3C, 0x22, 0x78,
		0x3A, 0x4D, 0xD5, 0xA7, 0x55, 0x4A, 0x9D, 0x24,
		0x0E, 0x50, 0x54, 0xBF, 0xEF, 0x00, 0xD1, 0xC0,
		0x81, 0x40, 0x81, 0x80, 0x95, 0x00, 0xB3, 0x00,
		0x71, 0x4F, 0x81, 0xC0, 0x01, 0x01, 0x4D, 0xD0,
		0x00, 0xA0, 0xF0, 0x70, 0x3E, 0x80, 0x30, 0x20,
		0x35, 0x00, 0x54, 0x4F, 0x21, 0x00, 0x00, 0x1A,
		0x04, 0x74, 0x00, 0x30, 0xF2, 0x70, 0x5A, 0x80,
		0xB0, 0x58, 0x8A, 0x00, 0x54, 0x4F, 0x21, 0x00,
		0x00, 0x1A, 0x00, 0x00, 0x00, 0xFD, 0x00, 0x1D,
		0x50, 0x18, 0xA0, 0x3C, 0x04, 0x11, 0x00, 0xF0,
		0xF8, 0x38, 0xF0, 0x3C, 0x00, 0x00, 0x00, 0xFC,
		0x00, 0x58, 0x49, 0x4C, 0x49, 0x4E, 0x58, 0x20,
		0x44, 0x50, 0x0A, 0x20, 0x20, 0x20, 0x01, 0x19,

		0x02, 0x03, 0x27, 0x71, 0x4F, 0x01, 0x02, 0x03,
		0x11, 0x12, 0x13, 0x04, 0x14, 0x05, 0x1F, 0x90,
		0x0E, 0x0F, 0x1D, 0x1E, 0x23, 0x09, 0x17, 0x07,
		0x83, 0x01, 0x00, 0x00, 0x6A, 0x03, 0x0C, 0x00,
		0x00, 0x00, 0x00, 0x78, 0x20, 0x00, 0x00, 0x56,
		0x5E, 0x00, 0xA0, 0xA0, 0xA0, 0x29, 0x50, 0x30,
		0x20, 0x35, 0x00, 0x54, 0x4F, 0x21, 0x00, 0x00,
		0x1E, 0xE2, 0x68, 0x00, 0xA0, 0xA0, 0x40, 0x2E,
		0x60, 0x30, 0x20, 0x36, 0x00, 0x54, 0x4F, 0x21,
		0x00, 0x00, 0x1A, 0x01, 0x1D, 0x00, 0xBC, 0x52,
		0xD0, 0x1E, 0x20, 0xB8, 0x28, 0x55, 0x40, 0x54,
		0x4F, 0x21, 0x00, 0x00, 0x1E, 0x8C, 0x0A, 0xD0,
		0x90, 0x20, 0x40, 0x31, 0x20, 0x0C, 0x40, 0x55,
		0x00, 0x54, 0x4F, 0x21, 0x00, 0x00, 0x18, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xF0
	};
#else
	// 8K30, 8K24, 5K, 4K120, 4K100 + Audio
	unsigned char edid[256] = {
		0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x61, 0x98, 0x23, 0x01,
		0x00, 0x00, 0x00, 0x00, 0x28, 0x1C, 0x01, 0x04, 0xB5, 0x3C, 0x22, 0x78,
		0x26, 0x61, 0x50, 0xA6, 0x56, 0x50, 0xA0, 0x00, 0x0D, 0x50, 0x54, 0xA5,
		0x6B, 0x80, 0xD1, 0xC0, 0x81, 0xC0, 0x81, 0x00, 0x81, 0x80, 0xA9, 0x00,
		0xB3, 0x00, 0xD1, 0xFC, 0x01, 0x01, 0x04, 0x74, 0x00, 0x30, 0xF2, 0x70,
		0x5A, 0x80, 0xB0, 0x58, 0x8A, 0x00, 0x54, 0x4F, 0x21, 0x00, 0x00, 0x1A,
		0x4D, 0xD0, 0x00, 0xA0, 0xF0, 0x70, 0x3E, 0x80, 0x30, 0x20, 0x35, 0x00,
		0x56, 0x50, 0x21, 0x00, 0x00, 0x1A, 0x00, 0x00, 0x00, 0xFD, 0x00, 0x1E,
		0x3C, 0x32, 0xB4, 0x66, 0x01, 0x0A, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
		0x00, 0x00, 0x00, 0xFC, 0x00, 0x58, 0x69, 0x6C, 0x69, 0x6E, 0x78, 0x20,
		0x73, 0x69, 0x6E, 0x6B, 0x0A, 0x20, 0x01, 0x17, 0x70, 0x12, 0x6E, 0x00,
		0x00, 0x81, 0x00, 0x04, 0x23, 0x09, 0x03, 0x07, 0x03, 0x00, 0x64, 0xEB,
		0xA0, 0x01, 0x04, 0xFF, 0x0E, 0xA0, 0x00, 0x2F, 0x80, 0x21, 0x00, 0x6F,
		0x08, 0x3E, 0x00, 0x03, 0x00, 0x05, 0x00, 0xFD, 0x68, 0x01, 0x04, 0xFF,
		0x13, 0x4F, 0x00, 0x27, 0x80, 0x1F, 0x00, 0x3F, 0x0B, 0x51, 0x00, 0x43,
		0x00, 0x07, 0x00, 0x65, 0x8E, 0x01, 0x04, 0xFF, 0x1D, 0x4F, 0x00, 0x07,
		0x80, 0x1F, 0x00, 0xDF, 0x10, 0x3C, 0x00, 0x2E, 0x00, 0x07, 0x00, 0x86,
		0x3D, 0x01, 0x04, 0xFF, 0x1D, 0x4F, 0x00, 0x07, 0x80, 0x1F, 0x00, 0xDF,
		0x10, 0x30, 0x00, 0x22, 0x00, 0x07, 0x00, 0x5C, 0x7F, 0x01, 0x00, 0xFF,
		0x0E, 0x4F, 0x00, 0x07, 0x80, 0x1F, 0x00, 0x6F, 0x08, 0x73, 0x00, 0x65,
		0x00, 0x07, 0x00, 0x60, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x90
	};
#endif

	for (i = 0 ; i < (256 * 4) ; i = i + (16 * 4)) {
		for (j = i ; j < (i + (16 * 4)) ; j = j + 4) {
			XDp_WriteReg (VID_EDID_BASEADDR, j, edid[(i/4)+1]);
		}
	}
	for (i = 0 ; i < (256 * 4) ; i = i + 4) {
		XDp_WriteReg (VID_EDID_BASEADDR, i, edid[i/4]);
	}
}

/*****************************************************************************/
/**
*
* This function scans VFMC- IIC.
*
* @param    None.
*
* @return    None.
*
* @note        None.
*
******************************************************************************/
void I2C_Scan(u32 BaseAddress)
{
	u8 Buffer[2];
	int BytesRecvd;
	int i;

	print("\n\r");
	print("---------------------\n\r");
	print("- I2C Scan: \n\r");
	print("---------------------\n\r");

	for (i = 0; i < 128; i++) {
		BytesRecvd = XIic_Recv(BaseAddress, i,
				       (u8*)Buffer, 1, XIIC_STOP);
		if (BytesRecvd == 0) {
			continue;
		}
		xil_printf("Found device: 0x%02x\n\r",i);
	}
	print("\n\r");
}

/*****************************************************************************/
/**
*
* This function reads DP141 VFMC- IIC.
*
* @param    None.
*
* @return    None.
*
* @note        None.
*
******************************************************************************/
u8 i2c_read_dp141(u32 I2CBaseAddress, u8 I2CSlaveAddress,
		u16 RegisterAddress)
{
	u32 ByteCount = 0;
	u8 Buffer[1];
	u8 Data;
	u8 Retry = 0;
	u8 Exit;

	Exit = FALSE;
	Data = 0;

	do {
		/* Set Address */
//		Buffer[0] = (RegisterAddress >> 8);
		Buffer[0] = RegisterAddress & 0xff;
		ByteCount = XIic_Send(I2CBaseAddress, I2CSlaveAddress,
				      (u8*)Buffer, 1, XIIC_REPEATED_START);

		if (ByteCount != 1) {
			Retry++;

			/* Maximum retries */
			if (Retry == 255) {
				Exit = TRUE;
			}
		} else {
			/* Read data */
			ByteCount = XIic_Recv(I2CBaseAddress, I2CSlaveAddress,
					      (u8*)Buffer, 1, XIIC_STOP);
//			ByteCount = XIic_Recv(I2CBaseAddress, I2CSlaveAddress,
//					      (u8*)Buffer, 2, XIIC_STOP);
//			if (ByteCount != 1) {
//				Exit = FALSE;
//				Exit = TRUE;
//			} else {
				Data = Buffer[0];
				Exit = TRUE;
//			}
		}
	} while (!Exit);

	return Data;
}

int i2c_write_dp141(u32 I2CBaseAddress, u8 I2CSlaveAddress,
		u16 RegisterAddress, u8 Value)
{
	u32 ByteCount = 0;
	u8 Buffer[2];
	u8 Retry = 0;

	/* Write data */
//	Buffer[0] = (RegisterAddress >> 8);
	Buffer[0] = RegisterAddress & 0xff;
	Buffer[1] = Value;

	while (1) {
		ByteCount = XIic_Send(I2CBaseAddress, I2CSlaveAddress,
				      (u8*)Buffer, 3, XIIC_STOP);

		if (ByteCount != 2) {
			Retry++;

			/* Maximum retries */
			if (Retry == 255) {
				return XST_FAILURE;
			}
		} else {
			return XST_SUCCESS;
		}
	}
}

void read_DP141()
{
	u8 Data;
	int i =0;
//	u8 Buffer[1];

	for (i = 0 ; i < 0xD ; i++) {
		Data = i2c_read_dp141(XPAR_IIC_0_BASEADDR,
				      I2C_TI_DP141_ADDR, i);
		xil_printf("%x : %02x \r\n",i, Data);
	}
}


/*****************************************************************************/
/**
*
* This function is the callback function for Info Packet Handling.
*
* @param    InstancePtr is a pointer to the XDpRxSs instance.
*
* @return    None.
*
* @note        None.
*
******************************************************************************/
void DpRxSs_InfoPacketHandler(void *InstancePtr)
{
	u32 InfoFrame[9];
	int i=1;

	for(i = 1 ; i < 9 ; i++) {
		InfoFrame[i] = XDp_ReadReg(DpRxSsInst.DpPtr->Config.BaseAddr,
				XDP_RX_AUDIO_INFO_DATA(i));
	}

	AudioinfoFrame.frame_count++;

	AudioinfoFrame.version = InfoFrame[1]>>26;
	AudioinfoFrame.type = (InfoFrame[1]>>8)&0xFF;
	AudioinfoFrame.sec_id = InfoFrame[1]&0xFF;
	AudioinfoFrame.info_length = (InfoFrame[1]>>16)&0x3FF;

	AudioinfoFrame.audio_channel_count = InfoFrame[2]&0x7;
	AudioinfoFrame.audio_coding_type = (InfoFrame[2]>>4)&0xF;
	AudioinfoFrame.sample_size = (InfoFrame[2]>>8)&0x3;
	AudioinfoFrame.sampling_frequency = (InfoFrame[2]>>10)&0x7;
	AudioinfoFrame.channel_allocation = (InfoFrame[2]>>24)&0xFF;

	AudioinfoFrame.level_shift = (InfoFrame[3]>>3)&0xF;
	AudioinfoFrame.downmix_inhibit = (InfoFrame[3]>>7)&0x1;

//	Print_InfoPkt();
}

/*****************************************************************************/
/**
*
* This function is the callback function for Generic Packet Handling of
* 32-Bytes payload.
*
* @param    InstancePtr is a pointer to the XDpRxSs instance.
*
* @return    None.
*
* @note        None.
*
******************************************************************************/
void DpRxSs_ExtPacketHandler(void *InstancePtr)
{
	u32 ExtFrame[9];
	int i=1;

	SdpExtFrame.frame_count++;

	/*Header Information*/
	ExtFrame[0] = XDp_ReadReg(DpRxSsInst.DpPtr->Config.BaseAddr,
			XDP_RX_AUDIO_EXT_DATA(1));
	SdpExtFrame.Header[0] =  ExtFrame[0]&0xFF;
	SdpExtFrame.Header[1] = (ExtFrame[0]&0xFF00)>>8;
	SdpExtFrame.Header[2] = (ExtFrame[0]&0xFF0000)>>16;
	SdpExtFrame.Header[3] = (ExtFrame[0]&0xFF000000)>>24;

	/*Payload Information*/
	for (i = 0 ; i < 8 ; i++)
	{
		ExtFrame[i+1] = XDp_ReadReg(DpRxSsInst.DpPtr->Config.BaseAddr,
				XDP_RX_AUDIO_EXT_DATA(i+2));
		SdpExtFrame.Payload[(i*4)]   =  ExtFrame[i+1]&0xFF;
		SdpExtFrame.Payload[(i*4)+1] = (ExtFrame[i+1]&0xFF00)>>8;
		SdpExtFrame.Payload[(i*4)+2] = (ExtFrame[i+1]&0xFF0000)>>16;
		SdpExtFrame.Payload[(i*4)+3] = (ExtFrame[i+1]&0xFF000000)>>24;
	}

}

/*****************************************************************************/
/**
*
* This function is the callback function for Info Packet Handling.
*
* @param    InstancePtr is a pointer to the XDpRxSs instance.
*
* @return    None.
*
* @note        None.
*
******************************************************************************/
void Print_InfoPkt()
{
	xil_printf("Received Audio Info Packet::\r\n");
	xil_printf(" -frame_count		 	 : 0x%x \r\n",
			AudioinfoFrame.frame_count);
	xil_printf(" -version			 	 : 0x%x \r\n",
			AudioinfoFrame.version);
	xil_printf(" -type				 	 : 0x%x \r\n",
			AudioinfoFrame.type);
	xil_printf(" -sec_id				 : 0x%x \r\n",
			AudioinfoFrame.sec_id);
	xil_printf(" -info_length			 : 0x%x \r\n",
			AudioinfoFrame.info_length);
	xil_printf(" -audio_channel_count	 : 0x%x \r\n",
			AudioinfoFrame.audio_channel_count);
	xil_printf(" -audio_coding_type		 : 0x%x \r\n",
			AudioinfoFrame.audio_coding_type);
	xil_printf(" -sample_size			 : 0x%x \r\n",
			AudioinfoFrame.sample_size);
	xil_printf(" -sampling_frequency	 : 0x%x \r\n",
			AudioinfoFrame.sampling_frequency);
	xil_printf(" -channel_allocation	 : 0x%x \r\n",
			AudioinfoFrame.channel_allocation);
	xil_printf(" -level_shift			 : 0x%x \r\n",
			AudioinfoFrame.level_shift);
	xil_printf(" -downmix_inhibit		 : 0x%x \r\n",
			AudioinfoFrame.downmix_inhibit);
}

/*****************************************************************************/
/**
*
* This function is the callback function for Ext Packet Handling.
*
* @param    InstancePtr is a pointer to the XDpRxSs instance.
*
* @return    None.
*
* @note        None.
*
******************************************************************************/
void Print_ExtPkt()
{
	int i=0;

	xil_printf("Received SDP Packet Type::\r\n");
	switch(SdpExtFrame.Header[1])
	{
		case 0x04: xil_printf(" -> Extension\r\n"); break;
		case 0x05: xil_printf(" -> Audio_CopyManagement\r\n"); break;
		case 0x06: xil_printf(" -> ISRC\r\n"); break;
		case 0x07: xil_printf(" -> Video Stream Configuration (VSC)\r\n");break;
		case 0x20: xil_printf(" -> Video Stream Configuration Extension"
				" for VESA (VSC_EXT_VESA) - Used for HDR Metadata\r\n"); break;
		case 0x21: xil_printf(" -> VSC_EXT_CEA for future CEA INFOFRAME with "
				"payload of more than 28 bytes\r\n"); break;
		default: xil_printf(" -> Reserved/Not Defined\r\n"); break;
	}
	xil_printf(" Header Bytes : 0x%x, 0x%x, 0x%x, 0x%x \r\n",
			SdpExtFrame.Header[0],
			SdpExtFrame.Header[1],
			SdpExtFrame.Header[2],
			SdpExtFrame.Header[3]);
	for(i=0;i<8;i++)
	{
		xil_printf(" Payload Bytes : 0x%x, 0x%x, 0x%x, 0x%x \r\n",
				SdpExtFrame.Payload[(i*4)],
				SdpExtFrame.Payload[(i*4)+1],
				SdpExtFrame.Payload[(i*4)+2],
				SdpExtFrame.Payload[(i*4)+3]);
	}
	xil_printf(" Frame Count : %d \r\n",SdpExtFrame.frame_count);
}
