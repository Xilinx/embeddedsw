/******************************************************************************
* Copyright (C) 2020 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright 2023-2025 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xdprxss_dp14_rx.c
*
* This file contains a design example using the XDpRxSs driver in single stream
* (SST) transport mode.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver  Who Date     Changes
* ---- --- -------- --------------------------------------------------------
* 1.00 ND  10/18/22  Common DP 2.1 rx only application for zcu102 and vcu118
* 1.01 ND  03/24/25  Added support for PARRETO fmc.
* 1.02 ND  05/02/25  Enhanced the prints for training.
* 1.03 KU  10/07/26  Optimization to work for any system combination
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include <stddef.h>
#include <xil_cache.h>
#include <xdp.h>
#include <xdp_hw.h>
#include <xdprxss.h>
#include <xiic.h>
#include <xiic_l.h>
#include <xil_exception.h>
#include <xil_printf.h>
#include <xil_types.h>
#include <xparameters.h>
#include <xstatus.h>
#include <xtmrctr.h>
#ifdef SDT
#include "xinterrupt_wrap.h"
#endif
#ifndef PLATFORM_MB
#include <xuartps_hw.h>
#include "xscugic.h"
#include "xiicps.h"
#else
#include "xintc.h"
#include "xuartlite.h"
#include "xuartlite_l.h"
#endif
#include <xvphy.h>
#include <xvphy_dp.h>
#include <xvphy_hw.h>
#include "xvidframe_crc.h"
#include "videofmc_defs.h"
#include "ti_lmk03318.h"
#include "idt_8t49n24x.h"
/************************** Constant Definitions *****************************/

/*
* The following constants map to the names of the hardware instances.
* They are only defined here such that a user can easily change all the
* needed device IDs in one place.
* There is only one interrupt controlled to be selected from SCUGIC and GPIO
* INTC. INTC selection is based on INTC parameters defined xparameters.h file.
*/
#ifdef SDT
#define XPAR_IIC_0_BASEADDR XPAR_PROC_HIER_0_AXI_IIC_0_BASEADDR
#define INTRNAME_DPRX   0
#endif
#ifndef PLATFORM_MB
#define XINTC_DPRXSS_DP_INTERRUPT_ID \
	XPAR_FABRIC_DP21RXSS_0_VEC_ID
#define XINTC 			XScuGic
#define XINTC_DEVICE_ID 	XPAR_SCUGIC_SINGLE_DEVICE_ID
#define XINTC_HANDLER 		XScuGic_InterruptHandler
#else
#define XINTC_DPRXSS_DP_INTERRUPT_ID \
	XPAR_MICROBLAZE_0_AXI_INTC_DP_RX_HIER_0_V_DP_RXSS2_0_DPRXSS_DP_IRQ_INTR
#define XINTC			XIntc
#define XINTC_DEVICE_ID	XPAR_INTC_0_DEVICE_ID
#define XINTC_HANDLER	XIntc_InterruptHandler
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
#ifndef SDT
#define IIC_DEVICE_ID	XPAR_IIC_0_DEVICE_ID
#else
#define IIC_DEVICE_ID	XPAR_PROC_HIER_0_AXI_IIC_0_BASEADDR
#endif

/* DP Specific Defines
 */
#define DPRXSS_LINK_RATE 		XDPRXSS_LINK_BW_SET_810GBPS
#define DPRXSS_LANE_COUNT 		XDPRXSS_LANE_COUNT_SET_4
// #ifndef SDT
// #define SET_TX_TO_2BYTE            \
// 	(XPAR_XDP_0_GT_DATAWIDTH/2)
// #define SET_RX_TO_2BYTE            \
// 	(XPAR_XDP_0_GT_DATAWIDTH/2)
// #else
// #define SET_TX_TO_2BYTE            \
// 	(XPAR_XDP_0_GT_DATA_WIDTH / 2)
// #define SET_RX_TO_2BYTE            \
// 	(XPAR_XDP_0_GT_DATA_WIDTH / 2)
// #endif

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
#define DP_BS_IDLE_TIMEOUT      0xFFFFFF
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
#define POLARITY_MASK 0x04040404
/* Timer Specific Defines
 */
#define TIMER_RESET_VALUE        1000

#define PARRETO_FMC //enable for parreto fmc and disable for diode fmc

#if (XPAR_DP_RX_HIER_0_V_DP_RXSS2_0_DP_OCTA_PIXEL_ENABLE)
#define CRC_CFG 0x5
#endif
#if (XPAR_DP_RX_HIER_0_V_DP_RXSS2_0_DP_QUAD_PIXEL_ENABLE)
#define CRC_CFG 0x4
#endif


//Set the RX PLL and Channel based on the VPHY config
#if (XPAR_XVPHY_0_RX_PLL_SELECTION == 0x1)
XVphy_PllType VPHY_RX_PLL_TYPE = XVPHY_PLL_TYPE_QPLL0;
XVphy_ChannelId VPHY_RX_CHANNEL_TYPE = XVPHY_CHANNEL_ID_CMN0;
#endif
#if (XPAR_XVPHY_0_RX_PLL_SELECTION == 0x2)
XVphy_PllType VPHY_RX_PLL_TYPE = XVPHY_PLL_TYPE_QPLL1;
XVphy_ChannelId VPHY_RX_CHANNEL_TYPE = XVPHY_CHANNEL_ID_CMN1;
#endif


//These are the REFCLK sources for VCU118 and ZCU102
#ifdef PLATFORM_MB //VCU118 (270Mhz on REFCLK0, 400Mhz on REFCLK1)
XVphy_PllRefClkSelType VPHY_REFCLK_SEL_270 = XVPHY_REF_CLK_SEL_XPLL_GTREFCLK0;
XVphy_PllRefClkSelType VPHY_REFCLK_SEL_400 = XVPHY_REF_CLK_SEL_XPLL_GTREFCLK1;
#else //ZCU102 (270Mhz on REFCLK0, 400Mhz on NORTHREFCLK0)
XVphy_PllRefClkSelType VPHY_REFCLK_SEL_270 = XVPHY_REF_CLK_SEL_XPLL_GTREFCLK0;
XVphy_PllRefClkSelType VPHY_REFCLK_SEL_400 = XVPHY_REF_CLK_SEL_XPLL_GTNORTHREFCLK0;
#endif


#define XVPHY_DP_LINK_RATE_HZ_1000GBPS  10000000000LL
#define XVPHY_DP_LINK_RATE_HZ_1350GBPS  13500000000LL
#define XVPHY_DP_LINK_RATE_HZ_2000GBPS  20000000000LL

#define XVPHY_DP_REF_CLK_FREQ_HZ_400	 400000000LL


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

#ifndef PLATFORM_MB
XIicPs Ps_Iic0;
XIicPs_Config *XIic0Ps_ConfigPtr;
#define PS_IIC_CLK 100000
#endif

/************************** Function Prototypes ******************************/
#ifndef SDT
u32 DpRxSs_Main(u16 DeviceId);
#else
u32 DpRxSs_Main(u32 BaseAddress);
#endif
u32 DpRxSs_PlatformInit(void);
#ifndef SDT
u32 DpRxSs_VideoPhyInit(u16 DeviceId);
#else
u32 DpRxSs_VideoPhyInit(u32 Baseaddress);
#endif
u32 DpRxSs_Setup(void);
void PLLRefClkSel (XVphy *InstancePtr, u8 link_rate);
void AppHelp();
void ReportVideoCRC();
void CalculateCRC(void);
void LoadEDID(void);
char XUartPs_RecvByte_NonBlocking();
void CustomWaitUs(void *InstancePtr, u32 MicroSeconds);
char xil_getc(u32 timeout_ms);

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
void Dprx_InterruptHandlerDownReq(void *InstancePtr);
void Dprx_InterruptHandlerDownReply(void *InstancePtr);
void Dprx_InterruptHandlerPayloadAlloc(void *InstancePtr);
void Dprx_InterruptHandlerActRx(void *InstancePtr);
int VideoFMC_Init(void);

void Print_InfoPkt();
void Print_ExtPkt();
u32 overflow_count = 0;
u32 missed_count = 0;
/************************** Variable Definitions *****************************/

XDpRxSs DpRxSsInst;    /* The DPRX Subsystem instance.*/
XINTC IntcInst;        /* The interrupt controller instance. */
XVphy VPhyInst;    /* The DPRX Subsystem instance.*/
XTmrCtr TmrCtr; /* Timer instance.*/
Video_CRC_Config VidFrameCRC; /* Video Frame CRC instance */
DP_Rx_Training_Algo_Config RxTrainConfig;
XIic IicInstance;	/* I2C bus for MC6000 and IDT */
/************************** Function Definitions *****************************/

void enable_caches()
{
#ifdef __PPC__
    Xil_ICacheEnableRegion(CACHEABLE_REGION_MASK);
    Xil_DCacheEnableRegion(CACHEABLE_REGION_MASK);
#elif __MICROBLAZE__
#ifndef SDT
#ifdef XPAR_MICROBLAZE_USE_ICACHE
    Xil_ICacheEnable();
#endif
#ifdef XPAR_MICROBLAZE_USE_DCACHE
    Xil_DCacheEnable();
#endif
#else
	Xil_ICacheEnable();
	Xil_DCacheEnable();
#endif
#endif
}

void disable_caches()
{
#ifdef __MICROBLAZE__
#ifndef SDT
#ifdef XPAR_MICROBLAZE_USE_DCACHE
    Xil_DCacheDisable();
#endif
#ifdef XPAR_MICROBLAZE_USE_ICACHE
    Xil_ICacheDisable();
#endif
#else
	Xil_DCacheDisable();
	Xil_ICacheDisable();
#endif
#endif
}



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
#ifndef PLATFORM_MB
	Buffer[0] = 0x01;
#else
	Buffer[0] = 0x02;
#endif
	ByteCount = XIic_Send(XPAR_IIC_0_BASEADDR, I2C_MUX_ADDR,
			(u8*)Buffer, 1, XIIC_STOP);
	if (ByteCount != 1) {
		xil_printf("Failed to set the I2C Mux.\n\r");
	    return XST_FAILURE;
	}

#ifndef PARRETO_FMC
//	I2C_Scan(XPAR_IIC_0_BASEADDR);

	/* Configure VFMC IO Expander 0:
	 * Disable Si5344
	 * Set primary clock source for LMK03318 to IOCLKp(0)
	 * Set secondary clock source for LMK03318 to IOCLKp(1)
	 * Disable LMK61E2*/
	Buffer[0] = 0x01;
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
	Buffer[0] = 0x16;
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

#ifdef PLATFORM_MB
    Status = SI5344_Init (&IicInstance, I2C_SI5344_ADDR);
#else
    Status = SI5344_Init (&Ps_Iic0, I2C_SI5344_ADDR);
#endif
    if (Status != XST_SUCCESS) {
	xil_printf("Failed to Si5344\n\r");
        return XST_FAILURE;
    }
#endif
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
	enable_caches();

	xil_printf("------------------------------------------\n\r");
	xil_printf("DisplayPort 2.x RX Only Example\n\r");
	xil_printf("(c) 2024 by AMD\n\r");
	xil_printf("-------------------------------------------\n\r\n\r");
#ifndef SDT
	Status = DpRxSs_Main(XDPRXSS_DEVICE_ID);
#else
	Status = DpRxSs_Main(XPAR_DPRXSS_0_BASEADDR);
#endif
	if (Status != XST_SUCCESS) {
		xil_printf("DisplayPort RX Subsystem design example failed.");
		return XST_FAILURE;
	}

	disable_caches();

	return XST_SUCCESS;
}

void PrintLinkInfo(){
		u32 Linkrate=0;
		Linkrate = XDpRxSs_ReadReg(DpRxSsInst.DpPtr->Config.BaseAddr,
													XDP_RX_DPCD_LINK_BW_SET);
		int lk_int=0,lk_dec=0;
		if(Linkrate == 0x6){
					lk_int=1;
					lk_dec=62;
		}else if (Linkrate == 0xA){
					lk_int=2;
					lk_dec=7;
		}else if (Linkrate == 0x14){
					lk_int =5;
					lk_dec =4;
		}else if(Linkrate == 0x1E){
					lk_int = 8;
					lk_dec = 1;
		}else if(Linkrate == 0x1){
					lk_int = 10;
					lk_dec = 0;
		}else if(Linkrate == 0x4){
					lk_int = 13;
					lk_dec = 5;
		}else if(Linkrate == 0x2){
					lk_int = 20;
					lk_dec = 0;
		}

		xil_printf("Video Detected --> Link Config: %d.%dx%d, "
			           "Frame: %dx%d, MISC0: 0x%x,",
		lk_int,lk_dec,
		(int)DpRxSsInst.UsrOpt.LaneCount,
		(int)XDp_ReadReg(DpRxSsInst.DpPtr->Config.BaseAddr,
				XDP_RX_MSA_HRES),
		(int)XDp_ReadReg(DpRxSsInst.DpPtr->Config.BaseAddr,
			XDP_RX_MSA_VHEIGHT),
		(int)XDp_ReadReg(DpRxSsInst.DpPtr->Config.BaseAddr,
				XDP_RX_MSA_MISC0));

		if((Linkrate == 0x1) || (Linkrate == 0x2) || (Linkrate == 0x4)){
		xil_printf(
			"VFreq L = %d, VFreq H =%d\r\n",
			(int)XDp_ReadReg(DpRxSsInst.DpPtr->Config.BaseAddr,
					0x1608),
			(int)XDp_ReadReg(DpRxSsInst.DpPtr->Config.BaseAddr,
						 0x160c));
		}else{
			xil_printf("Mvid=%d, Nvid=%d \r\n",
			(int)XDp_ReadReg(DpRxSsInst.DpPtr->Config.BaseAddr,
				 XDP_RX_MSA_MVID),
			(int)XDp_ReadReg(DpRxSsInst.DpPtr->Config.BaseAddr,
				 XDP_RX_MSA_NVID));
		}
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
#ifndef SDT
u32 DpRxSs_Main(u16 DeviceId)
#else
u32 DpRxSs_Main(u32 BaseAddress)
#endif
{
	u32 Status;
	XDpRxSs_Config *ConfigPtr;
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
#ifndef SDT
	ConfigPtr = XDpRxSs_LookupConfig(DeviceId);
#else
	ConfigPtr = XDpRxSs_LookupConfig(BaseAddress);
#endif
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

	/* Set Link rate and lane count to maximum */
	Status=XDpRxSs_SetLinkRate(&DpRxSsInst, DPRXSS_LINK_RATE);
	if(Status!= XST_SUCCESS){
		xil_printf("\r\n8.1 linkrate set failure\r\n");
	}
	XDpRxSs_SetLaneCount(&DpRxSsInst, DPRXSS_LANE_COUNT);

	/* Start DPRX Subsystem set */
	Status = XDpRxSs_Start(&DpRxSsInst);
	if (Status != XST_SUCCESS) {
		xil_printf("ERR:DPRX SS start failed\n\r");
		return XST_FAILURE;
	}

	/* Setup Video Phy, left to the user for implementation */
#ifndef SDT
	DpRxSs_VideoPhyInit(XVPHY_DEVICE_ID);
#else
	DpRxSs_VideoPhyInit(XPAR_XVPHY_0_BASEADDR);
#endif

	/* Setup DPRX SS, left to the user for implementation */
	DpRxSs_Setup();
	u16 mult = 0;

	AppHelp();
	while (1) {
		CommandKey = 0;

		CommandKey = xil_getc(0xff);
		Command = atoi(&CommandKey);

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

            // retoring unplug counter
            XDp_WriteReg(DpRxSsInst.DpPtr->Config.BaseAddr,
			XDP_RX_CDR_CONTROL_CONFIG,
			XDP_RX_CDR_CONTROL_CONFIG_TDLOCK_DP159);

			PrintLinkInfo();
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
#ifndef SDT
	Status = XTmrCtr_Initialize(&TmrCtr, XTIMER0_DEVICE_ID);
#else
	Status = XTmrCtr_Initialize(&TmrCtr, XPAR_XTMRCTR_0_BASEADDR);
#endif
	if (Status != XST_SUCCESS) {
		xil_printf("ERR:Timer failed to initialize. \r\n");
		return XST_FAILURE;
	}

	XTmrCtr_SetResetValue(&TmrCtr, XTC_TIMER_0, TIMER_RESET_VALUE);
	XTmrCtr_Start(&TmrCtr, XTC_TIMER_0);

#ifndef PLATFORM_MB
#ifndef SDT
	XIic0Ps_ConfigPtr = XIicPs_LookupConfig(XPAR_XIICPS_1_DEVICE_ID);
#else
	XIic0Ps_ConfigPtr = XIicPs_LookupConfig(XPAR_XIICPS_1_BASEADDR);
#endif
	if (NULL == XIic0Ps_ConfigPtr) {
		return XST_FAILURE;
	}

	Status = XIicPs_CfgInitialize(&Ps_Iic0, XIic0Ps_ConfigPtr,
	                   XIic0Ps_ConfigPtr->BaseAddress);
	if (Status != XST_SUCCESS) {
	        return XST_FAILURE;
	}

	XIicPs_Reset(&Ps_Iic0);
	/*
	 * Set the IIC serial clock rate.
	 */
	XIicPs_SetSClk(&Ps_Iic0, PS_IIC_CLK);
#endif
	VideoFMC_Init();
#ifndef PARRETO_FMC
	IDT_8T49N24x_SetClock(XPAR_IIC_0_BASEADDR, I2C_IDT8N49_ADDR,
			      0, 270000000, TRUE);
#endif
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
#ifndef SDT
u32 DpRxSs_VideoPhyInit(u16 DeviceId)
#else
u32 DpRxSs_VideoPhyInit(u32 Baseaddress)
#endif
{
	XVphy_Config *ConfigPtr;
    u32 Status;
	/* Obtain the device configuration for the DisplayPort RX Subsystem */
#ifndef SDT
	ConfigPtr = XVphy_LookupConfig(DeviceId);
#else
	ConfigPtr = XVphy_LookupConfig(Baseaddress);
#endif
	if (!ConfigPtr) {
		return XST_FAILURE;
	}

	PLLRefClkSel (&VPhyInst, 0x4);

	XVphy_DpInitialize(&VPhyInst, ConfigPtr, 0,
			   VPHY_REFCLK_SEL_400,
			   VPHY_REFCLK_SEL_400,
			   VPHY_RX_PLL_TYPE,
			   VPHY_RX_PLL_TYPE,
			   0x4);
    PLLRefClkSel (&VPhyInst, 0x4);
	XVphy_SetupDP21Phy (&VPhyInst, 0, VPHY_RX_CHANNEL_TYPE,
		XVPHY_DIR_RX, 0x4, VPHY_REFCLK_SEL_400,
		VPHY_RX_PLL_TYPE);

	Status = XVphy_DP21PhyReset (&VPhyInst, 0, VPHY_RX_CHANNEL_TYPE,
			XVPHY_DIR_RX);
    if (Status == XST_FAILURE) {
        xil_printf ("Issue encountered in PHY config and reset\r\n");
    }

#ifndef PARRETO_FMC
	//Setting polarity (RX) for new DP2.1 FMC
	XVphy_SetPolarity(&VPhyInst, 0, XVPHY_CHANNEL_ID_CHA,
			XVPHY_DIR_RX, 1);
#endif

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
	ReadVal= XDp_ReadReg(DpRxSsInst.DpPtr->Config.BaseAddr,
		     0xD0);
	ReadVal = ReadVal & (0xFFFFFFFE);
	XDp_WriteReg(DpRxSsInst.DpPtr->Config.BaseAddr,
			     0xD0, ReadVal);

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
			  XDP_RX_INTERRUPT_MASK_BW_CHANGE_MASK |
			  XDP_RX_INTERRUPT_MASK_VCP_ALLOC_MASK |
			 XDP_RX_INTERRUPT_MASK_VCP_DEALLOC_MASK |
			 XDP_RX_INTERRUPT_MASK_DOWN_REPLY_MASK |
			 XDP_RX_INTERRUPT_MASK_DOWN_REQUEST_MASK);


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

       /*Disabling timeout */
        ReadVal = XDp_ReadReg(DpRxSsInst.DpPtr->Config.BaseAddr,
                        XDP_RX_CDR_CONTROL_CONFIG);

        XDp_WriteReg(DpRxSsInst.DpPtr->Config.BaseAddr,
                     XDP_RX_CDR_CONTROL_CONFIG,
                     ReadVal |
                     XDP_RX_CDR_CONTROL_CONFIG_DISABLE_TIMEOUT);

        /*Setting 8B10 Mode for backward compatibility */
        XDp_WriteReg(DpRxSsInst.DpPtr->Config.BaseAddr, XDP_RX_OVER_CTRL_DPCD, 0x1);
        XDp_WriteReg(DpRxSsInst.DpPtr->Config.BaseAddr, 0x1600, 0x1);
        XDp_WriteReg(DpRxSsInst.DpPtr->Config.BaseAddr, XDP_RX_OVER_CTRL_DPCD, 0x0);


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
#ifndef SDT
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
    XDpRxSs_SetCallBack(&DpRxSsInst, XDPRXSS_HANDLER_DP_DWN_REQ_EVENT,
            &Dprx_InterruptHandlerDownReq, &DpRxSsInst);
    XDpRxSs_SetCallBack(&DpRxSsInst, XDPRXSS_HANDLER_DP_DWN_REP_EVENT,
            &Dprx_InterruptHandlerDownReply, &DpRxSsInst);
    XDpRxSs_SetCallBack(&DpRxSsInst, XDPRXSS_HANDLER_DP_PAYLOAD_ALLOC_EVENT,
            &Dprx_InterruptHandlerPayloadAlloc, &DpRxSsInst);
    XDpRxSs_SetCallBack(&DpRxSsInst, XDPRXSS_HANDLER_DP_ACT_RX_EVENT,
            &Dprx_InterruptHandlerActRx, &DpRxSsInst);


	/* Set custom timer wait */
	XDpRxSs_SetUserTimerHandler(&DpRxSsInst, &CustomWaitUs, &TmrCtr);

#ifndef PLATFORM_MB
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

#else

	/*
	 * Initialize the interrupt controller driver so that it's ready to use.
	 */
	Status = XIntc_Initialize(&IntcInst, XINTC_DEVICE_ID);

	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

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
#else
u32 DpRxSs_SetupIntrSystem(void)
{
	u32 Status;

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
	XDpRxSs_SetCallBack(&DpRxSsInst, XDPRXSS_HANDLER_DP_DWN_REQ_EVENT,
			    &Dprx_InterruptHandlerDownReq, &DpRxSsInst);
	XDpRxSs_SetCallBack(&DpRxSsInst, XDPRXSS_HANDLER_DP_DWN_REP_EVENT,
			    &Dprx_InterruptHandlerDownReply, &DpRxSsInst);
	XDpRxSs_SetCallBack(&DpRxSsInst, XDPRXSS_HANDLER_DP_PAYLOAD_ALLOC_EVENT,
			    &Dprx_InterruptHandlerPayloadAlloc, &DpRxSsInst);
	XDpRxSs_SetCallBack(&DpRxSsInst, XDPRXSS_HANDLER_DP_ACT_RX_EVENT,
			    &Dprx_InterruptHandlerActRx, &DpRxSsInst);
	XDpRxSs_SetCallBack(&DpRxSsInst, XDPRXSS_HANDLER_DP_DWN_REQ_EVENT,
			    &Dprx_InterruptHandlerDownReq, &DpRxSsInst);
	XDpRxSs_SetCallBack(&DpRxSsInst, XDPRXSS_HANDLER_DP_DWN_REP_EVENT,
			    &Dprx_InterruptHandlerDownReply, &DpRxSsInst);
	XDpRxSs_SetCallBack(&DpRxSsInst, XDPRXSS_HANDLER_DP_PAYLOAD_ALLOC_EVENT,
			    &Dprx_InterruptHandlerPayloadAlloc, &DpRxSsInst);
	XDpRxSs_SetCallBack(&DpRxSsInst, XDPRXSS_HANDLER_DP_ACT_RX_EVENT,
			    &Dprx_InterruptHandlerActRx, &DpRxSsInst);

	/* Set custom timer wait */
	XDpRxSs_SetUserTimerHandler(&DpRxSsInst, &CustomWaitUs, &TmrCtr);

	/* Connect the device driver handler that will be called when an
	 * interrupt for the device occurs, the handler defined above performs
	 * the specific interrupt processing for the device
	 */
	Status = XSetupInterruptSystem(&DpRxSsInst, XDpRxSs_DpIntrHandler,
				       DpRxSsInst.Config.IntrId[INTRNAME_DPRX],
				       DpRxSsInst.Config.IntrParent,
				       XINTERRUPT_DEFAULT_PRIORITY);

	if (Status != XST_SUCCESS) {
		xil_printf("ERR: DP RX SS DP interrupt connect failed!\n\r");
		return XST_FAILURE;
	}

	return (XST_SUCCESS);
}
#endif

void Dprx_InterruptHandlerPayloadAlloc(void *InstancePtr)
{
        /* Virtual Channel Payload allocation,
         * de-allocation and partial deletion handler
         */
        XDpRxSs *DpRxSsPtr = (XDpRxSs *)InstancePtr;
        u32 RegVal = 0;
        XDp *DpPtr = DpRxSsPtr->DpPtr;
        RegVal = XDp_ReadReg(DpPtr->Config.BaseAddr, XDP_RX_MST_ALLOC);
        XDp_RxAllocatePayloadStream(DpPtr);
	XDp_RxInterruptEnable(DpRxSsInst.DpPtr,
                    XDP_RX_INTERRUPT_MASK_ACT_RX_MASK);

}

void Dprx_InterruptHandlerActRx(void *InstancePtr)
{
        /* ACT Receive Interrupt Handler */
    XDp_RxInterruptEnable(DpRxSsInst.DpPtr,
                    XDP_RX_INTERRUPT_MASK_VBLANK_MASK);
    XDp_RxInterruptEnable1(DpRxSsInst.DpPtr,
                    0x3FFFF);
    XDp_RxInterruptDisable(DpRxSsInst.DpPtr,
                    XDP_RX_INTERRUPT_MASK_ACT_RX_MASK);


}


void Dprx_InterruptHandlerDownReq(void *InstancePtr)
{
        /* Down Request Buffer Ready handler
         * (Indicates the availability of the Down request)
         */
        XDp_RxHandleDownReq(DpRxSsInst.DpPtr);
}

void Dprx_InterruptHandlerDownReply(void *InstancePtr)
{
        /* Down Reply Buffer Read handler (indicates a
         * read event from down reply buffer by upstream source)
         */

        /* Increment the DownRequest Counter (if any) */
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
#ifdef DEBUG
	xil_printf("NoVideo Interrupt\r\n");
#endif
	XDp_RxInterruptEnable(DpRxSsInst.DpPtr,
			      XDP_RX_INTERRUPT_MASK_VBLANK_MASK);


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
#ifdef DEBUG
	xil_printf("valid video interrupt\r\n");
#endif
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
	XDp_RxDtgDis(DpRxSsInst.DpPtr);
	XDp_RxDtgEn(DpRxSsInst.DpPtr);
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
	/* Disable All Interrupts*/
	XDp_RxInterruptDisable(DpRxSsInst.DpPtr, 0xFFFFFFFF);
	XDp_RxInterruptDisable1(DpRxSsInst.DpPtr, 0xFFFFFFFF);
	xil_printf("Rx cable Unplugged\r\n");
	/* Disable & Enable Audio */
	XDpRxSs_AudioDisable(&DpRxSsInst);
	AudioinfoFrame.frame_count = 0;
	SdpExtFrame.Header[1] = 0;
	SdpExtFrame_q.Header[1] = 0;
	SdpExtFrame.frame_count = 0;
	SdpExtFrame.frame_count = 0;
	/* Enable Training related interrupts*/
	XDp_RxInterruptEnable(DpRxSsInst.DpPtr,
			  XDP_RX_INTERRUPT_MASK_TP1_MASK |
			  XDP_RX_INTERRUPT_MASK_TP2_MASK |
			  XDP_RX_INTERRUPT_MASK_TP3_MASK |
			  XDP_RX_INTERRUPT_MASK_POWER_STATE_MASK |
			  XDP_RX_INTERRUPT_MASK_CRC_TEST_MASK |
			  XDP_RX_INTERRUPT_MASK_BW_CHANGE_MASK |
			  XDP_RX_INTERRUPT_MASK_VCP_ALLOC_MASK |
			 XDP_RX_INTERRUPT_MASK_VCP_DEALLOC_MASK |
			 XDP_RX_INTERRUPT_MASK_DOWN_REPLY_MASK |
			 XDP_RX_INTERRUPT_MASK_DOWN_REQUEST_MASK);


	XDp_RxInterruptEnable1(DpRxSsInst.DpPtr,
				   XDP_RX_INTERRUPT_MASK_TP4_MASK |
			   XDP_RX_INTERRUPT_MASK_ACCESS_LANE_SET_MASK |
			   XDP_RX_INTERRUPT_MASK_ACCESS_LINK_QUAL_MASK |
			   XDP_RX_INTERRUPT_MASK_ACCESS_ERROR_COUNTER_MASK);
	XDp_RxGenerateHpdInterrupt(DpRxSsInst.DpPtr, 5000);


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

	XVphy_PllRefClkSelType Refclk;

        if ((DpRxSsInst.UsrOpt.LinkRate == 0x1E) ||
                (DpRxSsInst.UsrOpt.LinkRate == 0x14) ||
                (DpRxSsInst.UsrOpt.LinkRate == 0xA) ||
                (DpRxSsInst.UsrOpt.LinkRate == 0x6) ) {
                        Refclk = VPHY_REFCLK_SEL_270;
        } else {
                        Refclk = VPHY_REFCLK_SEL_400;
        }

	/*Program Video PHY to requested line rate*/
	PLLRefClkSel (&VPhyInst, DpRxSsInst.UsrOpt.LinkRate);


	XVphy_SetupDP21Phy (&VPhyInst, 0, VPHY_RX_CHANNEL_TYPE,
			XVPHY_DIR_RX, DpRxSsInst.UsrOpt.LinkRate, Refclk,
			VPHY_RX_PLL_TYPE);
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
	/* Reset CRC Test Counter in DP DPCD Space */
	u32 Status;
	XVidFrameCrc_Reset();
	VidFrameCRC.TEST_CRC_CNT = 0;
	XDp_WriteReg(DpRxSsInst.DpPtr->Config.BaseAddr,
		     XDP_RX_CRC_CONFIG,
		     (VidFrameCRC.TEST_CRC_SUPPORTED << 5 |
		      VidFrameCRC.TEST_CRC_CNT));

    Status = XVphy_DP21PhyReset (&VPhyInst, 0, VPHY_RX_CHANNEL_TYPE,
                XVPHY_DIR_RX);
    if (Status == XST_FAILURE) {
        xil_printf ("Issue encountered in PHY config and reset\r\n");
    }

	/*Enable all interrupts*/
	XDp_RxInterruptEnable(DpRxSsInst.DpPtr,
			      XDP_RX_INTERRUPT_MASK_ALL_MASK);
	XDp_WriteReg(DpRxSsInst.DpPtr->Config.BaseAddr, XDP_RX_CRC_CONFIG,
		     (VidFrameCRC.TEST_CRC_SUPPORTED << 5 |
		      0));
	XDp_WriteReg(DpRxSsInst.DpPtr->Config.BaseAddr, XDP_RX_CRC_COMP0,
			 0x0);
	XDp_WriteReg(DpRxSsInst.DpPtr->Config.BaseAddr, XDP_RX_CRC_COMP1,
			0x0);
	XDp_WriteReg(DpRxSsInst.DpPtr->Config.BaseAddr, XDP_RX_CRC_COMP2,
			0x0);

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
	} else {
		/*Disable PRBS Mode in Video PHY*/
		DrpVal = XVphy_ReadReg(VPhyInst.Config.BaseAddr,
				       XVPHY_RX_CONTROL_REG);
		DrpVal = DrpVal & 0xEFEFEFEF;
		XVphy_WriteReg(VPhyInst.Config.BaseAddr,
			       XVPHY_RX_CONTROL_REG, DrpVal);
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
	u32 DrpVal1;
	u16 DrpVal_lower_lane0;
	u16 DrpVal_lower_lane1;
	u16 DrpVal_lower_lane2;
	u16 DrpVal_lower_lane3;

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

	XVphy_CfgQuadRefClkFreq(InstancePtr, 0, VPHY_REFCLK_SEL_270,
					XVPHY_DP_REF_CLK_FREQ_HZ_270);
		XVphy_CfgQuadRefClkFreq(InstancePtr, 0, VPHY_REFCLK_SEL_400,
				XVPHY_DP_REF_CLK_FREQ_HZ_400);

	switch (link_rate) {
	case 0x6:

		XVphy_CfgLineRate(InstancePtr, 0, XVPHY_CHANNEL_ID_CHA,
				  XVPHY_DP_LINK_RATE_HZ_162GBPS);
		XVphy_CfgLineRate(InstancePtr, 0, VPHY_RX_CHANNEL_TYPE,
				  XVPHY_DP_LINK_RATE_HZ_162GBPS);
		break;
	case 0x14:
		// XVphy_CfgQuadRefClkFreq(InstancePtr, 0, ONBOARD_REF_CLK,
		// 			XVPHY_DP_REF_CLK_FREQ_HZ_270);
		// XVphy_CfgQuadRefClkFreq(InstancePtr, 0, ONBOARD_400_CLK,
		// 		XVPHY_DP_REF_CLK_FREQ_HZ_400);
		XVphy_CfgLineRate(InstancePtr, 0, XVPHY_CHANNEL_ID_CHA,
				  XVPHY_DP_LINK_RATE_HZ_540GBPS);
		XVphy_CfgLineRate(InstancePtr, 0, VPHY_RX_CHANNEL_TYPE,
				  XVPHY_DP_LINK_RATE_HZ_540GBPS);
		break;
	case 0x1E:
		// XVphy_CfgQuadRefClkFreq(InstancePtr, 0, ONBOARD_REF_CLK,
		// 			XVPHY_DP_REF_CLK_FREQ_HZ_270);
		// XVphy_CfgQuadRefClkFreq(InstancePtr, 0, ONBOARD_400_CLK,
		// 		XVPHY_DP_REF_CLK_FREQ_HZ_400);
		XVphy_CfgLineRate(InstancePtr, 0, XVPHY_CHANNEL_ID_CHA,
				  XVPHY_DP_LINK_RATE_HZ_810GBPS);
		XVphy_CfgLineRate(InstancePtr, 0, VPHY_RX_CHANNEL_TYPE,
				  XVPHY_DP_LINK_RATE_HZ_810GBPS);
		break;

	case 0x1:
		// XVphy_CfgQuadRefClkFreq(InstancePtr, 0, ONBOARD_REF_CLK,
		// 			XVPHY_DP_REF_CLK_FREQ_HZ_270);
		// XVphy_CfgQuadRefClkFreq(InstancePtr, 0, ONBOARD_400_CLK,
		// 		XVPHY_DP_REF_CLK_FREQ_HZ_400);
		XVphy_CfgLineRate(InstancePtr, 0, XVPHY_CHANNEL_ID_CHA,
				XVPHY_DP_LINK_RATE_HZ_1000GBPS);
		XVphy_CfgLineRate(InstancePtr, 0, VPHY_RX_CHANNEL_TYPE,
				XVPHY_DP_LINK_RATE_HZ_1000GBPS);
		break;

	case 0x4:
		// XVphy_CfgQuadRefClkFreq(InstancePtr, 0, ONBOARD_REF_CLK,
		// 			XVPHY_DP_REF_CLK_FREQ_HZ_270);
		// XVphy_CfgQuadRefClkFreq(InstancePtr, 0, ONBOARD_400_CLK,
		// 		XVPHY_DP_REF_CLK_FREQ_HZ_400);
		XVphy_CfgLineRate(InstancePtr, 0, XVPHY_CHANNEL_ID_CHA,
				XVPHY_DP_LINK_RATE_HZ_1350GBPS);
		XVphy_CfgLineRate(InstancePtr, 0, VPHY_RX_CHANNEL_TYPE,
				XVPHY_DP_LINK_RATE_HZ_1350GBPS);
		break;

	case 0x2:
		// XVphy_CfgQuadRefClkFreq(InstancePtr, 0, ONBOARD_REF_CLK,
		// 			XVPHY_DP_REF_CLK_FREQ_HZ_270);
		// XVphy_CfgQuadRefClkFreq(InstancePtr, 0, ONBOARD_400_CLK,
		// 		XVPHY_DP_REF_CLK_FREQ_HZ_400);
		XVphy_CfgLineRate(InstancePtr, 0, XVPHY_CHANNEL_ID_CHA,
				XVPHY_DP_LINK_RATE_HZ_2000GBPS);
		XVphy_CfgLineRate(InstancePtr, 0, VPHY_RX_CHANNEL_TYPE,
				XVPHY_DP_LINK_RATE_HZ_2000GBPS);
		break;

	default:
		// XVphy_CfgQuadRefClkFreq(InstancePtr, 0, ONBOARD_REF_CLK,
		// 			XVPHY_DP_REF_CLK_FREQ_HZ_270);
		// XVphy_CfgQuadRefClkFreq(InstancePtr, 0,
		// 		ONBOARD_400_CLK, XVPHY_DP_REF_CLK_FREQ_HZ_400);
		XVphy_CfgLineRate(InstancePtr, 0, XVPHY_CHANNEL_ID_CHA,
				  XVPHY_DP_LINK_RATE_HZ_270GBPS);
		XVphy_CfgLineRate(InstancePtr, 0, VPHY_RX_CHANNEL_TYPE,
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

#ifndef PLATFORM_MB
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
	RecievedByte = XUartPs_ReadReg(STDIN_BASEADDRESS,
				       XUARTPS_FIFO_OFFSET);
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
	u32 overflow;
	u32 loop = 0;
	u32 misses = 0;
	/* Reset CRC Test Counter in DP DPCD Space */
	VidFrameCRC.TEST_CRC_CNT = 0;

	XDp_WriteReg(DpRxSsInst.DpPtr->Config.BaseAddr, XDP_RX_CRC_CONFIG,
		     (VidFrameCRC.TEST_CRC_SUPPORTED << 5 |
		      VidFrameCRC.TEST_CRC_CNT));

	CustomWaitUs(DpRxSsInst.DpPtr, 100000);
	VidFrameCRC.Mode_422 =
			(XVidFrameCrc_ReadReg(DpRxSsInst.DpPtr->Config.BaseAddr,
					      XDP_RX_MSA_MISC0) >> 1) & 0x3 ;

	if(VidFrameCRC.Mode_422 != 0x1 ) {
	XVidFrameCrc_WriteReg(VIDEO_CRC_BASEADDR, VIDEO_FRAME_CRC_CONFIG,
			CRC_CFG);
	}
	else{
	XVidFrameCrc_WriteReg(VIDEO_CRC_BASEADDR, VIDEO_FRAME_CRC_CONFIG,
			CRC_CFG | 0x80000000);
	}

	XVidFrameCrc_Reset();

	/* Add delay (~40 ms) for Frame CRC to compute on couple of frames */
	CustomWaitUs(DpRxSsInst.DpPtr, 400000);
	CustomWaitUs(DpRxSsInst.DpPtr, 400000);

	// reading the overflow twice to clear the bit set
	overflow = (XDp_ReadReg(DpRxSsInst.DpPtr->Config.BaseAddr,
						    XDP_RX_USER_FIFO_OVERFLOW)) & 0x00000111;
	overflow = (XDp_ReadReg(DpRxSsInst.DpPtr->Config.BaseAddr,
						    XDP_RX_USER_FIFO_OVERFLOW)) & 0x00000111;
	overflow = 0;

	//reading overflow in loop of 500
	//in ideal scenario it should never be set
	while (loop < 500) {
		loop++;
		overflow |= (XDp_ReadReg(DpRxSsInst.DpPtr->Config.BaseAddr,
							    XDP_RX_USER_FIFO_OVERFLOW)) & 0x00000111;
		CustomWaitUs(DpRxSsInst.DpPtr, 800);
	}

	misses = (XVidFrameCrc_ReadReg(VIDEO_CRC_BASEADDR,
			VIDEO_FRAME_CRC_MISSES)) & 0x00000FFF;


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

	/* Write CRC values to DPCD TEST CRC space */
	XDp_WriteReg(DpRxSsInst.DpPtr->Config.BaseAddr, XDP_RX_CRC_COMP0,
					VidFrameCRC.Pixel_r);
	XDp_WriteReg(DpRxSsInst.DpPtr->Config.BaseAddr, XDP_RX_CRC_COMP1,
					VidFrameCRC.Pixel_g);
	XDp_WriteReg(DpRxSsInst.DpPtr->Config.BaseAddr, XDP_RX_CRC_COMP2,
					VidFrameCRC.Pixel_b);


	if (overflow == 0 && misses == 0) {
    // Set CRC only when overflow is not there and no b2b CRC mismatch
	VidFrameCRC.TEST_CRC_CNT = 1;
	} else if (overflow && misses) {
		XDp_WriteReg(DpRxSsInst.DpPtr->Config.BaseAddr, XDP_RX_CRC_COMP0,
				 0x2);
		XDp_WriteReg(DpRxSsInst.DpPtr->Config.BaseAddr, XDP_RX_CRC_COMP1,
				0x2);
		XDp_WriteReg(DpRxSsInst.DpPtr->Config.BaseAddr, XDP_RX_CRC_COMP2,
				0x2);
		//not setting CRC available bit
		// for overflow CRC forced to 0x0
		//for b2b miss CRC forced to 0x1
		// helps in identifying at TX report
	} else if (overflow) {
			overflow_count++;
			XDp_WriteReg(DpRxSsInst.DpPtr->Config.BaseAddr, XDP_RX_CRC_COMP0,
					 0x0);
			XDp_WriteReg(DpRxSsInst.DpPtr->Config.BaseAddr, XDP_RX_CRC_COMP1,
					0x0);
			XDp_WriteReg(DpRxSsInst.DpPtr->Config.BaseAddr, XDP_RX_CRC_COMP2,
					0x0);
	} else if (misses) {
			missed_count++;
			XDp_WriteReg(DpRxSsInst.DpPtr->Config.BaseAddr, XDP_RX_CRC_COMP0,
				     0x1);
			XDp_WriteReg(DpRxSsInst.DpPtr->Config.BaseAddr, XDP_RX_CRC_COMP1,
					0x1);
			XDp_WriteReg(DpRxSsInst.DpPtr->Config.BaseAddr, XDP_RX_CRC_COMP2,
					0x1);
	}
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
	unsigned char edid[384] = {
			0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x05, 0xa4, 0x23, 0x01, 0x00, 0x00, 0x00, 0x00,
			0x28, 0x21, 0x01, 0x04, 0xe5, 0x3d, 0x23, 0x78, 0x3a, 0x5f, 0xb1, 0xa2, 0x57, 0x4f, 0xa2, 0x28,
			0x0f, 0x50, 0x54, 0xbf, 0xef, 0x80, 0x71, 0x4f, 0x81, 0x00, 0x81, 0xc0, 0x81, 0x80, 0xa9, 0xc0,
			0xb3, 0x00, 0x95, 0x00, 0xd1, 0xc0, 0x4d, 0xd0, 0x00, 0xa0, 0xf0, 0x70, 0x3e, 0x80, 0x30, 0x20,
			0x35, 0x00, 0x5f, 0x59, 0x21, 0x00, 0x00, 0x1a, 0x56, 0x5e, 0x00, 0xa0, 0xa0, 0xa0, 0x29, 0x50,
			0x30, 0x20, 0x35, 0x00, 0x5f, 0x59, 0x21, 0x00, 0x00, 0x1a, 0x00, 0x00, 0x00, 0xfd, 0x00, 0x38,
			0x4b, 0x1e, 0x86, 0x36, 0x00, 0x0a, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x00, 0x00, 0x00, 0xfc,
			0x00, 0x41, 0x4d, 0x44, 0x20, 0x53, 0x69, 0x6e, 0x6b, 0x0a, 0x20, 0x20, 0x20, 0x20, 0x02, 0x21,
			0x02, 0x03, 0x12, 0x71, 0x83, 0x4f, 0x00, 0x00, 0x29, 0x0f, 0x7f, 0x07, 0x15, 0x06, 0x55, 0x3d,
			0x1f, 0xc0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x5c,
			0x70, 0x20, 0x79, 0x02, 0x00, 0x22, 0x02, 0x3c, 0xb2, 0x90, 0x1f, 0x88, 0xff, 0x1d, 0x4f, 0x00,
			0x07, 0x80, 0x1f, 0x00, 0xdf, 0x10, 0x7a, 0x00, 0x6c, 0x00, 0x07, 0x00, 0x80, 0xfa, 0x29, 0x08,
			0xff, 0x27, 0x4f, 0x00, 0x07, 0x80, 0x1f, 0x00, 0xdf, 0x10, 0x7a, 0x00, 0x6c, 0x00, 0x07, 0x00,
			0x35, 0x9c, 0x7d, 0x08, 0xff, 0x3b, 0x4f, 0x00, 0x07, 0x80, 0x1f, 0x00, 0xbf, 0x21, 0xf5, 0x00,
			0xe7, 0x00, 0x07, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x45, 0x90
	};
#endif

	for (i = 0 ; i < (384 * 4) ; i = i + (16 * 4)) {
		for (j = i ; j < (i + (16 * 4)) ; j = j + 4) {
			XDp_WriteReg (VID_EDID_BASEADDR, j, edid[(i/4)+1]);
		}
	}
	for (i = 0 ; i < (384 * 4) ; i = i + 4) {
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


/*****************************************************************************/
/**
*
* This function to get uart input from user
*
* @param	timeout_ms
*
* @return
*		- received charactor
*
* @note		None.
*
******************************************************************************/
char xil_getc(u32 timeout_ms){
	char c;
	u32 timeout = 0;

	extern XTmrCtr TmrCtr;

	// Reset and start timer
	if ( timeout_ms > 0 && timeout_ms != 0xff ){
	  XTmrCtr_Start(&TmrCtr, 0);
	}

#ifndef PLATFORM_MB
	while((!XUartPs_IsReceiveData(STDIN_BASEADDRESS)) && (timeout == 0)){
#else
	while(XUartLite_IsReceiveEmpty(STDIN_BASEADDRESS) && (timeout == 0)){
#endif

		if ( timeout_ms == 0 ){ // no timeout - wait for ever
		   timeout = 0;
		} else if ( timeout_ms == 0xff ) { // no wait - special case
		   timeout = 1;
		} else if(timeout_ms > 0){
			if(XTmrCtr_GetValue(&TmrCtr, 0)
										> ( timeout_ms * (100000000 / 1000) )){
				timeout = 1;
			}
		}
	}
	if(timeout == 1){
		c = 0;
	} else {
#ifndef PLATFORM_MB
		c = XUartPs_RecvByte_NonBlocking();
#else
		c = XUartLite_RecvByte(STDIN_BASEADDRESS);
#endif
	}
	return c;
}
