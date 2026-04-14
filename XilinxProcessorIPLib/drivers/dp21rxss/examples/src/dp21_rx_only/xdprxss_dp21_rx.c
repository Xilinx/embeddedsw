/******************************************************************************
* Copyright (C) 2020 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2023 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xdprxss_dp21_rx.c
*
* This file contains a design example using the XDpRxSs driver in single stream
* (SST) transport mode.
* This example application is designed to run on the ZynqMP evaluation board
* (ZCU102) and the MicroBlaze evaluation board (VCU118).
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver  Who Date     Changes
* ---- --- -------- --------------------------------------------------------
* 1.00 ND  10/18/22  Common DP 2.1 rx only application for zcu102 and vcu118
* 1.01 ND  03/24/25  Added support for PARRETO fmc.
* 1.02 ND  05/02/25  Enhanced the prints for training.
* 1.03 KU  10/07/25  Optimization to work for any system combination
* 1.6  GM  11/11/25  Improved formatting and removed non-PARRETTO FMC.
* 1.6  GM  11/30/25  Used driver APIs to avoid direct register access.
* 		     Moved DpRxSs_Setup() and other initializations to driver.
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/
#include <stddef.h>
#include <xil_cache.h>
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
#if !defined (PLATFORM_MB) && !defined (__riscv)
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

/************************** Constant Definitions *****************************/
/*
 * The following constants map to the names of the hardware instances.
 * They are only defined here such that a user can easily change all the
 * needed device IDs in one place.
 * There is only one interrupt controlled to be selected from SCUGIC and GPIO
 * INTC. INTC selection is based on INTC parameters defined xparameters.h file.
 */
#ifdef SDT
#define XPAR_IIC_0_BASEADDR	XPAR_PROC_HIER_0_AXI_IIC_0_BASEADDR
#define INTRNAME_DPRX		0
#endif

#if !defined (PLATFORM_MB) && !defined (__riscv)
/** DPRX subsystem DP interrupt ID for ARM platforms */
#define XINTC_DPRXSS_DP_INTERRUPT_ID \
	XPAR_FABRIC_DP21RXSS_0_VEC_ID
/** Interrupt controller type for ARM platforms */
#define XINTC 			XScuGic
/** Interrupt controller device ID for ARM platforms */
#define XINTC_DEVICE_ID 	XPAR_SCUGIC_SINGLE_DEVICE_ID
/** Interrupt controller handler for ARM platforms */
#define XINTC_HANDLER 		XScuGic_InterruptHandler
#else
/** DPRX subsystem DP interrupt ID for MicroBlaze platforms */
#define XINTC_DPRXSS_DP_INTERRUPT_ID \
	XPAR_MICROBLAZE_0_AXI_INTC_DP_RX_HIER_0_V_DP_RXSS2_0_DPRXSS_DP_IRQ_INTR
/** Interrupt controller type for MicroBlaze platforms */
#define XINTC			XIntc
/** Interrupt controller device ID for MicroBlaze platforms */
#define XINTC_DEVICE_ID	XPAR_INTC_0_DEVICE_ID
/** Interrupt controller handler for MicroBlaze platforms */
#define XINTC_HANDLER	XIntc_InterruptHandler
#endif

/*
 * The unique device ID of the instances used in example
 */
/** DP RX Subsystem device ID */
#define XDPRXSS_DEVICE_ID 	XPAR_DPRXSS_0_DEVICE_ID
/** Video PHY Controller device ID */
#define XVPHY_DEVICE_ID 	XPAR_DP_RX_HIER_0_VID_PHY_CONTROLLER_0_DEVICE_ID
/** Timer Counter device ID */
#define XTIMER0_DEVICE_ID 	XPAR_TMRCTR_0_DEVICE_ID

/** Video Frame CRC base address */
#define VIDEO_CRC_BASEADDR 	XPAR_DP_RX_HIER_0_VIDEO_FRAME_CRC_0_BASEADDR
/** UART Lite base address */
#define UARTLITE_BASEADDR 	XPAR_PSU_UART_0_BASEADDR
/** Video PHY Controller base address */
#define VIDPHY_BASEADDR 	XPAR_DP_RX_HIER_0_VID_PHY_CONTROLLER_0_BASEADDR
/** Video EDID base address */
#define VID_EDID_BASEADDR 	XPAR_DP_RX_HIER_0_VID_EDID_0_BASEADDR

#ifndef SDT
/** IIC device ID */
#define IIC_DEVICE_ID		XPAR_IIC_0_DEVICE_ID
#else
/** IIC base address for SDT builds */
#define XIIC_BASEADDRESS	XPAR_PROC_HIER_0_AXI_IIC_0_BASEADDR
#endif

/*
 * DP Specific Defines
 */
/** Default DP RX link rate configuration */
#define DPRXSS_LINK_RATE 		XDPRXSS_LINK_BW_SET_810GBPS
/** Default DP RX lane count configuration */
#define DPRXSS_LANE_COUNT 		XDPRXSS_LANE_COUNT_SET_4

/*
 * User can tune these variables as per their system
 *
 * Max timeout tuned as per tester - AXI Clock=100 MHz
 * Some GPUs may need larger value, So user may tune if needed
 */
/** DP blank screen idle timeout value */
#define DP_BS_IDLE_TIMEOUT      0xFFFFFF
/** Vertical blank wait count */
#define VBLANK_WAIT_COUNT       200

/*
 * For compliance, please set AUX_DEFER_COUNT to be 8
 * (Only for ZCU102-ARM R5 based Rx system).
 * For Interop, set this to 6.
 */
/** AUX defer count for compliance/interop */
#define AUX_DEFER_COUNT         6

/*
 * VPHY Specific Defines
 */
/** Video PHY RX symbol error counter for channels 1 and 2 register offset */
#define XVPHY_RX_SYM_ERR_CNTR_CH1_2_REG    0x084
/** Video PHY RX symbol error counter for channels 3 and 4 register offset */
#define XVPHY_RX_SYM_ERR_CNTR_CH3_4_REG    0x088

/** Video PHY DRP CPLL feedback divider register address */
#define XVPHY_DRP_CPLL_FBDIV 		0x28
/** Video PHY DRP CPLL reference clock divider register address */
#define XVPHY_DRP_CPLL_REFCLK_DIV 	0x2A
/** Video PHY DRP RX output divider register address */
#define XVPHY_DRP_RXOUT_DIV 		0x63
/** Video PHY DRP RX clock 25 register address */
#define XVPHY_DRP_RXCLK25 		0x6D
/** Video PHY DRP TX clock 25 register address */
#define XVPHY_DRP_TXCLK25 		0x7A
/** Video PHY DRP TX output divider register address */
#define XVPHY_DRP_TXOUT_DIV 		0x7C
/** Video PHY DRP RX data width register address */
#define XVPHY_DRP_RX_DATA_WIDTH 	0x03
/** Video PHY DRP RX internal data width register address */
#define XVPHY_DRP_RX_INT_DATA_WIDTH 	0x66
/** Video PHY DRP GTHE4 PRBS error counter lower register address */
#define XVPHY_DRP_GTHE4_PRBS_ERR_CNTR_LOWER	0x25E
/** Video PHY DRP GTHE4 PRBS error counter upper register address */
#define XVPHY_DRP_GTHE4_PRBS_ERR_CNTR_UPPER	0x25F
/** Polarity mask for lane polarity configuration */
#define POLARITY_MASK				0x04040404

/*
 * Timer Specific Defines
 */
/** Timer reset value in microseconds */
#define TIMER_RESET_VALUE        1000

#if (XPAR_DP_RX_HIER_0_V_DP_RXSS2_0_DP_OCTA_PIXEL_ENABLE)
#define CRC_CFG		0x5
#endif
#if (XPAR_DP_RX_HIER_0_V_DP_RXSS2_0_DP_QUAD_PIXEL_ENABLE)
#define CRC_CFG		0x4
#endif

/*
 * Set the RX PLL and Channel based on the VPHY config
 */
#if (XPAR_XVPHY_0_RX_PLL_SELECTION == 0x1)
XVphy_PllType VPHY_RX_PLL_TYPE = XVPHY_PLL_TYPE_QPLL0;
XVphy_ChannelId VPHY_RX_CHANNEL_TYPE = XVPHY_CHANNEL_ID_CMN0;
#endif
#if (XPAR_XVPHY_0_RX_PLL_SELECTION == 0x2)
XVphy_PllType VPHY_RX_PLL_TYPE = XVPHY_PLL_TYPE_QPLL1;
XVphy_ChannelId VPHY_RX_CHANNEL_TYPE = XVPHY_CHANNEL_ID_CMN1;
#endif

/*
 * These are the REFCLK sources for VCU118 and ZCU102
 */
#if defined (PLATFORM_MB) || defined (__riscv)	/**< VCU118 (270Mhz on REFCLK0, 400Mhz on REFCLK1) */
XVphy_PllRefClkSelType VPHY_REFCLK_SEL_270 = XVPHY_REF_CLK_SEL_XPLL_GTREFCLK0;
XVphy_PllRefClkSelType VPHY_REFCLK_SEL_400 = XVPHY_REF_CLK_SEL_XPLL_GTREFCLK1;
#else			/**< ZCU102 (270Mhz on REFCLK0, 400Mhz on NORTHREFCLK0) */
XVphy_PllRefClkSelType VPHY_REFCLK_SEL_270 = XVPHY_REF_CLK_SEL_XPLL_GTREFCLK0;
XVphy_PllRefClkSelType VPHY_REFCLK_SEL_400 = XVPHY_REF_CLK_SEL_XPLL_GTNORTHREFCLK0;
#endif

/** Video PHY DP link rate for 10.0 Gbps */
#define XVPHY_DP_LINK_RATE_HZ_1000GBPS  10000000000LL
/** Video PHY DP link rate for 13.5 Gbps */
#define XVPHY_DP_LINK_RATE_HZ_1350GBPS  13500000000LL
/** Video PHY DP link rate for 20.0 Gbps */
#define XVPHY_DP_LINK_RATE_HZ_2000GBPS  20000000000LL

/** Video PHY DP reference clock frequency for 400 MHz */
#define XVPHY_DP_REF_CLK_FREQ_HZ_400	 400000000LL

/**
 * Audio InfoFrame structure
 */
typedef struct
{
	u8 sec_id;	/**< DP Specific */
	u8 type;		/**< InfoFrame type */
	u8 version;		/**< InfoFrame version */
	u8 length;		/**< InfoFrame length */
	u8 audio_coding_type;	/**< Audio coding type */
	u8 audio_channel_count;	/**< Audio channel count */
	u8 sampling_frequency;	/**< Audio sampling frequency */
	u8 sample_size;		/**< Audio sample size */
	u8 level_shift;		/**< Level shift value */
	u8 downmix_inhibit;	/**< Downmix inhibit flag */
	u8 channel_allocation;	/**< Channel allocation */
	u16 info_length;	/**< InfoFrame packet length */
	u8 frame_count;		/**< Frame count */
} XilAudioInfoFrame;

/*
 * The structure defines Generic Frame Packet fields
 */
typedef struct
{
	u32 frame_count;	/**< Frame count */
	u32 frame_count_q;	/**< Frame count queue */
	u8 Header[4];		/**< Packet header (4 bytes) */
	u8 Payload[32];		/**< Packet payload (32 bytes) */
} XilAudioExtFrame;

XilAudioInfoFrame AudioinfoFrame;
XilAudioExtFrame  SdpExtFrame;
XilAudioExtFrame  SdpExtFrame_q;

#if !defined (PLATFORM_MB) && !defined (__riscv)
XIicPs Ps_Iic0;
XIicPs_Config *XIic0Ps_ConfigPtr;
/** PS IIC clock frequency in Hz */
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

void PLLRefClkSel (XVphy *InstancePtr, u8 link_rate);
void AppHelp();
void ReportVideoCRC();
void CalculateCRC(void);
char XUartPs_RecvByte_NonBlocking();
void CustomWaitUs(void *InstancePtr, u32 MicroSeconds);
char xil_getc(u32 timeout_ms);

/*
 * Interrupt helper functions
 */
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
XDpRxSs DpRxSsInst;		/**< The DPRX Subsystem instance. */
XINTC IntcInst;			/**< The interrupt controller instance. */
XVphy VPhyInst;			/**< The DPRX Subsystem instance. */
XTmrCtr TmrCtr;			/**< Timer instance. */
Video_CRC_Config VidFrameCRC;	/**< Video Frame CRC instance. */
XIic IicInstance;		/**< I2C bus for MC6000. */


/**< 8K30, 8K24, 5K, 4K120, 4K100 + Audio */
unsigned char DpRxEdid[384] = {
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

/************************** Function Definitions *****************************/

/*****************************************************************************/
/**
*
* This function enables instruction and data caches.
*
* @note		None.
*
******************************************************************************/
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

/*****************************************************************************/
/**
*
* This function disables instruction and data caches.
*
* @note		None.
*
******************************************************************************/
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
* @return
*		- XST_SUCCESS if initialization was successful.
*		- XST_FAILURE if an error occurs.
*
* @note		None.
*
******************************************************************************/
int VideoFMC_Init(void)
{
	int Status;
	u8 Buffer[2];
	int ByteCount;
	XIic_Config *ConfigPtr_IIC;	/**< Pointer to configuration data */

	xil_printf("VFMC: Setting IO Expanders...\n\r");

	/*
	 * Initialize the IIC driver so that it is ready to use.
	 */
#ifndef SDT
	ConfigPtr_IIC = XIic_LookupConfig(IIC_DEVICE_ID);
#else
	ConfigPtr_IIC = XIic_LookupConfig(XIIC_BASEADDRESS);
#endif
	if (ConfigPtr_IIC == NULL)
		return XST_FAILURE;

	Status = XIic_CfgInitialize(&IicInstance, ConfigPtr_IIC,
				    ConfigPtr_IIC->BaseAddress);
	if (Status != XST_SUCCESS)
		return XST_FAILURE;

	XIic_Reset(&IicInstance);

	/*
	 * Set the I2C Mux to select the HPC FMC
	 */
#if !defined (PLATFORM_MB) && !defined (__riscv)
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
	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* This is the main function for XDpRxSs interrupt example. If the
* DpRxSs_Main function which setup the system succeeds, this function
* will wait for the interrupts.
*
* @return	XST_SUCCESS if successful, XST_FAILURE if unsuccessful.
*
* @note		Unless setup failed, main will never return since
* 		DpRxSs_Main is blocking (it is waiting on interrupts).
*
******************************************************************************/
int main()
{
	u32 Status;
	enable_caches();

	xil_printf("------------------------------------------\n\r");
	xil_printf("DisplayPort 2.x RX Only Example\n\r");
	xil_printf("(c) 2025 by AMD\n\r");
	xil_printf("-------------------------------------------\n\r\n\r");
#ifndef SDT
	Status = DpRxSs_Main(XDPRXSS_DEVICE_ID);
#else
	Status = DpRxSs_Main(XPAR_DPRXSS_0_BASEADDR);
#endif
	if (Status != XST_SUCCESS) {
		xil_printf("DisplayPort RX Subsystem design example failed.\n");
		return XST_FAILURE;
	}

	disable_caches();

	return XST_SUCCESS;
}

void PrintLinkInfo()
/*****************************************************************************/
/**
*
* This function prints the DisplayPort link information.
*
* @note		None.
*
******************************************************************************/
{
	u32 Linkrate=0;
	int lk_int=0,lk_dec=0;

	Linkrate = XDpRxSs_GetLinkRate(&DpRxSsInst);
	if (Linkrate == 0x6) {
		lk_int=1;
		lk_dec=62;
	} else if (Linkrate == 0xA) {
		lk_int=2;
		lk_dec=7;
	} else if (Linkrate == 0x14) {
		lk_int =5;
		lk_dec =4;
	} else if (Linkrate == 0x1E) {
		lk_int = 8;
		lk_dec = 1;
	} else if (Linkrate == 0x1) {
		lk_int = 10;
		lk_dec = 0;
	} else if (Linkrate == 0x4) {
		lk_int = 13;
		lk_dec = 5;
	} else if (Linkrate == 0x2) {
		lk_int = 20;
		lk_dec = 0;
	}

	xil_printf("Video Detected --> Link Config: %d.%dx%d, "
		   "Frame: %dx%d, MISC0: 0x%x,",
		   lk_int,lk_dec,
		   (int)DpRxSsInst.UsrOpt.LaneCount,
		   (int)DpRxSsInst.DpPtr->RxInstance.MsaConfig[0].Vtm.Timing.HActive,
		   (int)DpRxSsInst.DpPtr->RxInstance.MsaConfig[0].Vtm.Timing.VActive,
		   (int)DpRxSsInst.DpPtr->RxInstance.MsaConfig[0].Misc0);

	if ((Linkrate == 0x1) || (Linkrate == 0x2) || (Linkrate == 0x4)) {
		xil_printf("VFreq %u\r\n",
			   DpRxSsInst.DpPtr->RxInstance.MsaConfig[0].VFreq);
	} else {
		xil_printf("Mvid=%d, Nvid=%d \r\n",
			    (int)DpRxSsInst.DpPtr->RxInstance.MsaConfig[0].MVid,
			    (int)DpRxSsInst.DpPtr->RxInstance.MsaConfig[0].NVid);
	}
}

/*****************************************************************************/
/**
*
* This function is the main entry point for the design example using the
* XDpRxSs driver. This function will setup the system with interrupts handlers.
*
* @param	DeviceId is the unique device ID of the DisplayPort RX
* 		Subsystem core.
*
* @return
*		- XST_FAILURE if the system setup failed.
*		- XST_SUCCESS should never return since this function, if setup
*		was successful, is blocking.
*
* @note		If system setup was successful, this function is blocking in
* 		order to illustrate interrupt handling taking place for
* 		different types interrupts.
* 		Refer xdprxss.h file for more info.
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
	XDpRxSs_UserConfig UserConfigData;

	/*
	 * Do platform initialization in this function. This is hardware
	 * system specific. It is up to the user to implement this function.
	 */
	xil_printf("PlatformInit\n\r");
	Status = DpRxSs_PlatformInit();
	if (Status != XST_SUCCESS) {
		xil_printf("Platform init failed!\n\r");
	}
	xil_printf("Platform initialization done.\n\r");

	/*
	 * Obtain the device configuration for the DisplayPort RX Subsystem
	 */
#ifndef SDT
	ConfigPtr = XDpRxSs_LookupConfig(DeviceId);
#else
	ConfigPtr = XDpRxSs_LookupConfig(BaseAddress);
#endif
	if (!ConfigPtr) {
		return XST_FAILURE;
	}

	/*
	 * Copy the device configuration into the DpRxSsInst's Config structure.
	 */
	Status = XDpRxSs_CfgInitialize(&DpRxSsInst, ConfigPtr, ConfigPtr->BaseAddress);
	if (Status != XST_SUCCESS) {
		xil_printf("DPRXSS config initialization failed.\n\r");
		return XST_FAILURE;
	}

	Status = DpRxSs_SetupIntrSystem();
	if (Status != XST_SUCCESS) {
		xil_printf("ERR: Interrupt system setup failed.\n\r");
		return XST_FAILURE;
	}

	/*
	 * Setup Video Phy, left to the user for implementation
	 */
#ifndef SDT
	DpRxSs_VideoPhyInit(XVPHY_DEVICE_ID);
#else
	DpRxSs_VideoPhyInit(XPAR_XVPHY_0_BASEADDR);
#endif

	/*
	 * Disable Mst mode
	 */
	DpRxSsInst.Config.MstSupport = 0;
	XDpRxSs_MstEnable(&DpRxSsInst, 0);

	xil_printf("DPRXSS: SST mode enabled. DPRXSS works only in SST mode.\n\r");

	/*
	 * Setup DpRxSs
	 */
	DpRxSsInst.EdidBaseAddr = VID_EDID_BASEADDR;
	DpRxSsInst.EdidDataPtr[0] = DpRxEdid;

	UserConfigData.BsIdleTimeout = DP_BS_IDLE_TIMEOUT;
	UserConfigData.AuxClkDeferCount = AUX_DEFER_COUNT;
	UserConfigData.CrcEnable = VidFrameCRC.TEST_CRC_SUPPORTED;
	UserConfigData.ChannelCoding = XDPRXSS_8B10B_CHANNEL_CODING;

	XDpRxSs_Setup(&DpRxSsInst, &UserConfigData);

	/*
	 * Print menu
	 */
	AppHelp();

	while (1) {
		CommandKey = 0;

		CommandKey = xil_getc(0xff);
		if (CommandKey != 0) {
			xil_printf("UserInput: %c\r\n", CommandKey);

			switch (CommandKey) {
				case '2':
					/*
					 * Reset the AUX logic from DP RX
					 */
					XDpRxSs_AuxReset(&DpRxSsInst, 1);
					break;

				case 's':
					xil_printf("DP Link Status --->\r\n");
					XDpRxSs_ReportLinkInfo(&DpRxSsInst);
					break;

				case 'd':
					xil_printf("Video PHY Config/Status --->\r\n");
					xil_printf("RCS (0x10) = 0x%x\n\r", XVphy_ReadReg(VIDPHY_BASEADDR,
						   XVPHY_REF_CLK_SEL_REG));
					xil_printf("PR  (0x14) = 0x%x\n\r", XVphy_ReadReg(VIDPHY_BASEADDR,
						   XVPHY_PLL_RESET_REG));
					xil_printf("PLS (0x18) = 0x%x\n\r", XVphy_ReadReg(VIDPHY_BASEADDR,
						   XVPHY_PLL_LOCK_STATUS_REG));
					xil_printf("TXI (0x1C) = 0x%x\n\r", XVphy_ReadReg(VIDPHY_BASEADDR,
						   XVPHY_TX_INIT_REG));
					xil_printf("TXIS(0x20) = 0x%x\n\r", XVphy_ReadReg(VIDPHY_BASEADDR,
						   XVPHY_TX_INIT_STATUS_REG));
					xil_printf("RXI (0x24) = 0x%x\n\r", XVphy_ReadReg(VIDPHY_BASEADDR,
						   XVPHY_RX_INIT_REG));
					xil_printf("RXIS(0x28) = 0x%x\n\r", XVphy_ReadReg(VIDPHY_BASEADDR,
						   XVPHY_RX_INIT_STATUS_REG));

					XVphy_DrpRd(&VPhyInst, 0, XVPHY_CHANNEL_ID_CH1,
						    XVPHY_DRP_CPLL_FBDIV,&DrpVal);

					xil_printf("GT DRP Addr (XVPHY_DRP_CPLL_FBDIV)= 0x%x, Val = 0x%x\r\n",
						   XVPHY_DRP_CPLL_FBDIV,DrpVal);

					XVphy_DrpRd(&VPhyInst, 0, XVPHY_CHANNEL_ID_CH1,
						    XVPHY_DRP_CPLL_REFCLK_DIV,&DrpVal);

					xil_printf("GT DRP Addr (XVPHY_DRP_CPLL_REFCLK_DIV)= 0x%x, Val = 0x%x\r\n",
						    XVPHY_DRP_CPLL_REFCLK_DIV,DrpVal);

					XVphy_DrpRd(&VPhyInst, 0, XVPHY_CHANNEL_ID_CH1,
						    XVPHY_DRP_RXOUT_DIV,&DrpVal);

					xil_printf("GT DRP Addr (XVPHY_DRP_RXOUT_DIV) = 0x%x, Val = 0x%x\r\n",
						   XVPHY_DRP_RXOUT_DIV,DrpVal);

					XVphy_DrpRd(&VPhyInst, 0, XVPHY_CHANNEL_ID_CH1, XVPHY_DRP_TXOUT_DIV,
						    &DrpVal);

					xil_printf("GT DRP Addr (XVPHY_DRP_TXOUT_DIV) = 0x%x, Val = 0x%x\r\n",
						   XVPHY_DRP_TXOUT_DIV,DrpVal);
					break;

				case 'h':
					XDpRxSs_GenerateHpdInterrupt(&DpRxSsInst, 5000);
					break;

				case 'e':
					ReadVal = XVphy_ReadReg(VIDPHY_BASEADDR, XVPHY_RX_SYM_ERR_CNTR_CH1_2_REG);
					xil_printf("Video PHY(8B10B): Error Counts [Lane1, Lane0] = [%d, %d]\n\r",
						   (ReadVal>>16), ReadVal&0xFFFF);

					ReadVal = XVphy_ReadReg(VIDPHY_BASEADDR, XVPHY_RX_SYM_ERR_CNTR_CH3_4_REG);
					xil_printf("Video PHY(8B10B): Error Counts [Lane3, Lane2] = [%d, %d]\n\r",
						   (ReadVal>>16), ReadVal&0xFFFF);

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
					xil_printf ("Lane1 (Upper) is %d,\r\n", DrpVal);

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
					xil_printf("XDP_RX_USER_FIFO_OVERFLOW (0x110) = 0x%x\n\r",
							XDpRxSs_UserFifoOverflowStatus(&DpRxSsInst));

					XDpRxSs_ReportMsaInfo(&DpRxSsInst);
					ReportVideoCRC();

					xil_printf("XDP_RX_LINE_RESET_DISABLE (0x008) = 0x%x\n\r",
							XDpRxSs_GetRxResetDisable(&DpRxSsInst));
					break;

				case 'r':
					xil_printf("Reset Video DTG in DisplayPort Controller...\r\n");

					XDpRxSs_DtgDisable(&DpRxSsInst);
					XDpRxSs_DtgEnable(&DpRxSsInst);
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
		} /**< end if */

		/*
		 * Info Frame Handling
		 * Info frame is sent once per frame and
		 * is static for that config
		 * Capture new Info Frame whenever config changes
		 */
		if (AudioinfoFrame.frame_count != 0) {
			Print_InfoPkt();
			AudioinfoFrame.frame_count=0;
		}

		/*
		 * Ext Frame Handling
		 */
		if (SdpExtFrame.Header[1] != SdpExtFrame_q.Header[1]) {
			Print_ExtPkt();
			SdpExtFrame_q = SdpExtFrame;
		}

		/*
		 * CRC Handling
		 */
		if(DpRxSsInst.VBlankCount>VBLANK_WAIT_COUNT) {
			/*
			 * VBLANK Management
			 */
			DpRxSsInst.VBlankCount = 0;

			XDpRxSs_UpdateCdrTimeOutVal(&DpRxSsInst, XDPRXSS_CDR_CONTROL_CONFIG_TDLOCK_DP159);
			/*
			 * Enable the training timeout
			 */
			XDpRxSs_CdrDisableTimeout(&DpRxSsInst, 0x0);

			/*
			 * Get MSA of incoming video.
			 */
			(void)XDpRxss_GetMsa(&DpRxSsInst);
			PrintLinkInfo();

			XDpRxSs_RxInterruptDisable(&DpRxSsInst, XDPRXSS_INTERRUPT_MASK_VBLANK_MASK);
			/*
			 * Disable & Enable Audio
			 */
			XDpRxSs_AudioDisable(&DpRxSsInst);
			XDpRxSs_AudioEnable(&DpRxSsInst);

			CalculateCRC();
		}
	} /**< end while(1) */

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* This function initialize required platform specific peripherals.
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

	/*
	 * Initialize CRC & Set default Pixel Mode to 1
	 */
	XVidFrameCrc_Initialize(&VidFrameCRC);

	/*
	 * Initialize Timer
	 */
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

#if !defined (PLATFORM_MB) && !defined (__riscv)
#ifndef SDT
	XIic0Ps_ConfigPtr = XIicPs_LookupConfig(XPAR_XIICPS_1_DEVICE_ID);
#else
	XIic0Ps_ConfigPtr = XIicPs_LookupConfig(XPAR_XIICPS_1_BASEADDR);
#endif
	if (NULL == XIic0Ps_ConfigPtr)
		return XST_FAILURE;

	Status = XIicPs_CfgInitialize(&Ps_Iic0, XIic0Ps_ConfigPtr,
				      XIic0Ps_ConfigPtr->BaseAddress);
	if (Status != XST_SUCCESS)
	        return XST_FAILURE;

	XIicPs_Reset(&Ps_Iic0);

	/*
	 * Set the IIC serial clock rate.
	 */
	XIicPs_SetSClk(&Ps_Iic0, PS_IIC_CLK);
#endif
	VideoFMC_Init();

	return Status;
}

/*****************************************************************************/
/**
*
* This function configures Video Phy.
*
* @param	DeviceId is the Video PHY device ID or base address.
*
* @return
*		- XST_SUCCESS if Video Phy configured successfully.
*		- XST_FAILURE, otherwise.
*
* @note		None.
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

	/*
	 * Obtain the device configuration for the DisplayPort RX Subsystem
	 */
#ifndef SDT
	ConfigPtr = XVphy_LookupConfig(DeviceId);
#else
	ConfigPtr = XVphy_LookupConfig(Baseaddress);
#endif
	if (!ConfigPtr)
		return XST_FAILURE;

	PLLRefClkSel (&VPhyInst, 0x4);

	XVphy_DpInitialize(&VPhyInst, ConfigPtr, 0,
			   VPHY_REFCLK_SEL_400,
			   VPHY_REFCLK_SEL_400,
			   VPHY_RX_PLL_TYPE,
			   VPHY_RX_PLL_TYPE,
			   0x4);

	PLLRefClkSel (&VPhyInst, 0x4);

	XVphy_SetupDP21Phy(&VPhyInst, 0, VPHY_RX_CHANNEL_TYPE,
			   XVPHY_DIR_RX, 0x4, VPHY_REFCLK_SEL_400,
			   VPHY_RX_PLL_TYPE);

	Status = XVphy_DP21PhyReset(&VPhyInst, 0, VPHY_RX_CHANNEL_TYPE,
				    XVPHY_DIR_RX);
	if (Status == XST_FAILURE)
		xil_printf ("Issue encountered in PHY config and reset\r\n");

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* This function sets user callback handlers for DP RX Subsystem events.
*
* @note		None.
*
******************************************************************************/
void XDpRxSs_SetUserCallBackHandlers(void)
{
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
* @return
*		- XST_SUCCESS if interrupt setup was successful.
*		- A specific error code defined in "xstatus.h" if an error
*		occurs.
*
* @note		None.
*
******************************************************************************/
#ifndef SDT
u32 DpRxSs_SetupIntrSystem(void)
{
	u32 Status;
	XINTC *IntcInstPtr = &IntcInst;

	/*
	 * Set user callbacks for all the interrupts
	 */
	XDpRxSs_SetUserCallBackHandlers();

	/*
	 * Set custom timer wait
	 */
	XDpRxSs_SetUserTimerHandler(&DpRxSsInst, &CustomWaitUs, &TmrCtr);

#if !defined (PLATFORM_MB) && !defined (__riscv)
	/*
	 * The configuration parameters of the interrupt controller
	 */
	XScuGic_Config *IntcConfig;

	/*
	 * Initialize the interrupt controller driver so that it is ready to use.
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

	/*
	 * Connect the device driver handler that will be called when an
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

	/*
	 * Enable the interrupt for the DP device
	 */
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

	/*
	 * Enable the interrupt for the DP device
	 */
	XIntc_Enable(IntcInstPtr, XINTC_DPRXSS_DP_INTERRUPT_ID);
#endif

	/*
	 * Initialize the exception table
	 */
	Xil_ExceptionInit();

	/*
	 * Register the interrupt controller handler with the exception table.
	 */
	Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_INT,
				     (Xil_ExceptionHandler)XINTC_HANDLER,
				     IntcInstPtr);

	/*
	 * Enable exceptions
	 */
	Xil_ExceptionEnable();

	return (XST_SUCCESS);
}
#else
u32 DpRxSs_SetupIntrSystem(void)
{
	u32 Status;

	/*
	 * Set user callbacks for all the interrupts
	 */
	XDpRxSs_SetUserCallBackHandlers();

	/*
	 * Set custom timer wait
	 */
	XDpRxSs_SetUserTimerHandler(&DpRxSsInst, &CustomWaitUs, &TmrCtr);

	/*
	 * Connect the device driver handler that will be called when an
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

/*****************************************************************************/
/**
*
* This function handles MST payload allocation interrupt.
*
* @param	InstancePtr is a pointer to the XDpRxSs instance.
*
* @note		None.
*
******************************************************************************/
void Dprx_InterruptHandlerPayloadAlloc(void *InstancePtr)
{
	/*
	 * Virtual Channel Payload allocation,
	 * de-allocation and partial deletion handler
	 */
	XDpRxSs *DpRxSsPtr = (XDpRxSs *)InstancePtr;

	(void)XDpRxSs_GetRxMstAlloc(DpRxSsPtr);
	XDpRxSs_RxAllocatePayloadStream(DpRxSsPtr);
}

/*****************************************************************************/
/**
*
* This function handles MST ACT (Allocation Change Trigger) interrupt.
*
* @param	InstancePtr is a pointer to the XDpRxSs instance.
*
* @note		None.
*
******************************************************************************/
void Dprx_InterruptHandlerActRx(void *InstancePtr)
{
    /*
     * ACT (Allocation Change Trigger) Receive Interrupt Handler
     * Indicates MST stream allocation/deallocation has been processed.
     */
    XDpRxSs *DpRxSsPtr = (XDpRxSs *)InstancePtr;

    XDpRxSs_HandleActRxInterrupts(DpRxSsPtr);

    /* Application can add custom processing here if needed */
}

/*****************************************************************************/
/**
*
* This function handles MST Down Request interrupt.
*
* @param	InstancePtr is a pointer to the XDpRxSs instance.
*
* @note		None.
*
******************************************************************************/
void Dprx_InterruptHandlerDownReq(void *InstancePtr)
{
	/*
	 * Down Request Buffer Ready handler
	 * (Indicates the availability of the Down request)
	 */
	XDpRxSs *DpRxSsPtr = (XDpRxSs *)InstancePtr;

	XDpRxSs_HandleDownReq(DpRxSsPtr);
}

/*****************************************************************************/
/**
*
* This function handles MST Down Reply interrupt.
*
* @param	InstancePtr is a pointer to the XDpRxSs instance.
*
* @note		None.
*
******************************************************************************/
void Dprx_InterruptHandlerDownReply(void *InstancePtr)
{
	/*
	 * Down Reply Buffer Read handler (indicates a
	 * read event from down reply buffer by upstream source)
	 *
	 * Increment the DownRequest Counter (if any)
	 */
	(void)InstancePtr;
}

/*****************************************************************************/
/**
*
* This function is the callback function for when the power state interrupt
* occurs.
*
* @param	InstancePtr is a pointer to the XDpRxSs instance.
*
*
* @note		None.
*
******************************************************************************/
void DpRxSs_PowerChangeHandler(void *InstancePtr)
{
	(void)InstancePtr;
}

/*****************************************************************************/
/**
*
* This function is the callback function for when a no video interrupt occurs.
*
* @param	InstancePtr is a pointer to the XDpRxSs instance.
*
* @note		None.
*
******************************************************************************/
void DpRxSs_NoVideoHandler(void *InstancePtr)
{
	XDpRxSs *DpRxSsPtr = (XDpRxSs *)InstancePtr;

	DpRxSsPtr->VBlankCount = 0;
#ifdef DEBUG
	xil_printf("NoVideo Interrupt\r\n");
#endif

	/*
	 * Reset CRC Test Counter in DP DPCD Space
	 */
	XVidFrameCrc_Reset();

	VidFrameCRC.TEST_CRC_CNT = 0;

	AudioinfoFrame.frame_count = 0;

}

/*****************************************************************************/
/**
*
* This function is the callback function for when a vertical blank interrupt
* occurs.
*
* @param	InstancePtr is a pointer to the XDpRxSs instance.
*
* @note		None.
*
******************************************************************************/
void DpRxSs_VerticalBlankHandler(void *InstancePtr)
{
	XDpRxSs *DpRxSsPtr = (XDpRxSs *)InstancePtr;

	DpRxSsPtr->VBlankCount++;
}

/*****************************************************************************/
/**
*
* This function is the callback function for when a training lost interrupt
* occurs.
*
* @param	InstancePtr is a pointer to the XDpRxSs instance.
*
* @note		None.
*
******************************************************************************/
void DpRxSs_TrainingLostHandler(void *InstancePtr)
{
	(void)InstancePtr;
    DpRxSsInst.link_up_trigger = 0;
}

/*****************************************************************************/
/**
*
* This function is the callback function for when a valid video interrupt
* occurs.
*
* @param	InstancePtr is a pointer to the XDpRxSs instance.
*
*
* @note		None.
*
******************************************************************************/
void DpRxSs_VideoHandler(void *InstancePtr)
{
	(void)InstancePtr;

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
* @param	InstancePtr is a pointer to the XDpRxSs instance.
*
*
* @note		None.
*
******************************************************************************/
void DpRxSs_TrainingDoneHandler(void *InstancePtr)
{
	(void)InstancePtr;
    DpRxSsInst.link_up_trigger = 1;
}

/*****************************************************************************/
/**
*
* This function is the callback function for when the unplug event occurs.
*
* @param	InstancePtr is a pointer to the XDpRxSs instance.
*
*
* @note		None.
*
******************************************************************************/
void DpRxSs_UnplugHandler(void *InstancePtr)
{
	XDpRxSs *DpRxSsPtr = (XDpRxSs *)InstancePtr;

	xil_printf("Rx cable Unplugged\r\n");

	/*
	 * Disable & Enable Audio
	 */
	XDpRxSs_AudioDisable(DpRxSsPtr);
	AudioinfoFrame.frame_count = 0;
	SdpExtFrame.Header[1] = 0;
	SdpExtFrame_q.Header[1] = 0;
	SdpExtFrame.frame_count = 0;
	SdpExtFrame.frame_count = 0;

	DpRxSsInst.link_up_trigger = 0;
}

/*****************************************************************************/
/**
*
* This function is the callback function for when the link bandwidth change
* occurs.
*
* @param	InstancePtr is a pointer to the XDpRxSs instance.
*
* @note		None.
*
******************************************************************************/
void DpRxSs_LinkBandwidthHandler(void *InstancePtr)
{
	XDpRxSs *DpRxSsPtr = (XDpRxSs *)InstancePtr;

	XVphy_PllRefClkSelType Refclk;

	if ((DpRxSsPtr->UsrOpt.LinkRate == 0x1E) ||
	    (DpRxSsPtr->UsrOpt.LinkRate == 0x14) ||
	    (DpRxSsPtr->UsrOpt.LinkRate == 0xA) ||
	    (DpRxSsPtr->UsrOpt.LinkRate == 0x6) ) {
		Refclk = VPHY_REFCLK_SEL_270;
	} else {
		Refclk = VPHY_REFCLK_SEL_400;
        }

	/*
	 * Program Video PHY to requested line rate
	 */
	PLLRefClkSel (&VPhyInst, DpRxSsPtr->UsrOpt.LinkRate);

	XVphy_SetupDP21Phy(&VPhyInst, 0, VPHY_RX_CHANNEL_TYPE,
			   XVPHY_DIR_RX, DpRxSsPtr->UsrOpt.LinkRate, Refclk,
			   VPHY_RX_PLL_TYPE);

	DpRxSsInst.link_up_trigger = 0;
}

/*****************************************************************************/
/**
*
* This function is the callback function for PLL reset request.
*
* @param	InstancePtr is a pointer to the XDpRxSs instance.
*
* @note		None.
*
******************************************************************************/
void DpRxSs_PllResetHandler(void *InstancePtr)
{
	/*
	 * Reset CRC Test Counter in DP DPCD Space
	 */
	u32 Status;
	(void)InstancePtr;

	/* Application-specific: Reset CRC tracking */
	XVidFrameCrc_Reset();
	VidFrameCRC.TEST_CRC_CNT = 0;

	/* Application-specific: PHY reset using application's VPhyInst */
	Status = XVphy_DP21PhyReset(&VPhyInst, 0, VPHY_RX_CHANNEL_TYPE,
			XVPHY_DIR_RX);
	if (Status == XST_FAILURE)
		xil_printf("Issue encountered in PHY config and reset\r\n");

	/* Application-specific: Update CRC configuration */

}

/*****************************************************************************/
/**
*
* This function is the callback function for PLL reset request.
*
* @param	InstancePtr is a pointer to the XDpRxSs instance.
*
* @note		None.
*
******************************************************************************/
void DpRxSs_BWChangeHandler(void *InstancePtr)
{
	(void)InstancePtr;
}

/*****************************************************************************/
/**
*
* This function is the callback function for Access lane set request.
*
* @param	InstancePtr is a pointer to the XDpRxSs instance.
*
* @note		None.
*
******************************************************************************/
void DpRxSs_AccessLaneSetHandler(void *InstancePtr)
{
	(void)InstancePtr;
}

/*****************************************************************************/
/**
*
* This function is the callback function for Test CRC Event request.
*
* @param	InstancePtr is a pointer to the XDpRxSs instance.
*
* @note		None.
*
******************************************************************************/
void DpRxSs_CRCTestEventHandler(void *InstancePtr)
{
	u16 ReadVal;
	u32 TrainingAlgoValue;
	XDpRxSs *DpRxSsPtr = (XDpRxSs *)InstancePtr;

	ReadVal = XDp_ReadReg(DpRxSsPtr->DpPtr->Config.BaseAddr, XDPRXSS_CRC_CONFIG);

	/*
	 * Record Training Algo Value - to be restored in non-phy test mode
	 */
	TrainingAlgoValue = XDp_ReadReg(DpRxSsPtr->Config.BaseAddress,
					XDP_RX_MIN_VOLTAGE_SWING);

	/*
	 * Refer to DPCD 0x270 Register
	 */
	if((ReadVal & 0x8000) == 0x8000) {
		/*
		 * Enable PHY test mode - Set Min voltage swing to 0
		 */
		XDp_WriteReg(DpRxSsPtr->DpPtr->Config.BaseAddr,
			     XDP_RX_MIN_VOLTAGE_SWING,
			     (TrainingAlgoValue & 0xFFFFFFFC) | 0x80000000);

		/*
		 * Disable Training timeout
		 */
		ReadVal = XDp_ReadReg(DpRxSsPtr->Config.BaseAddress,
				      XDP_RX_CDR_CONTROL_CONFIG);
		XDp_WriteReg(DpRxSsPtr->DpPtr->Config.BaseAddr,
			     XDP_RX_CDR_CONTROL_CONFIG, ReadVal | 0x40000000);
	} else {
		/*
		 * Disable PHY test mode & Set min voltage swing back to level 1
		 */
		XDp_WriteReg(DpRxSsPtr->DpPtr->Config.BaseAddr, XDP_RX_MIN_VOLTAGE_SWING,
			     (TrainingAlgoValue & 0x7FFFFFFF) | 0x1);

		/*
		 * Enable Training timeout
		 */
		ReadVal = XDp_ReadReg(DpRxSsPtr->Config.BaseAddress,
				      XDP_RX_CDR_CONTROL_CONFIG);
		XDp_WriteReg(DpRxSsPtr->DpPtr->Config.BaseAddr,
			     XDP_RX_CDR_CONTROL_CONFIG, ReadVal & 0xBFFFFFFF);
	}
}

/*****************************************************************************/
/**
*
* This function is the callback function for Access link qual request.
*
* @param	InstancePtr is a pointer to the XDpRxSs instance.
*
* @note		None.
*
******************************************************************************/
void DpRxSs_AccessLinkQualHandler(void *InstancePtr)
{
	u32 DrpVal;
	XDpRxSs *DpRxSsPtr = (XDpRxSs *)InstancePtr;

	/*
	 * Check for PRBS Mode
	 */
	if (DpRxSs_GetLinkQualLane(DpRxSsPtr) == XDPRXSS_DPC_LINK_QUAL_PRBS) {
		/*
		 * Enable PRBS Mode in Video PHY
		 */
		DrpVal = XVphy_ReadReg(VPhyInst.Config.BaseAddr, XVPHY_RX_CONTROL_REG);
		DrpVal = DrpVal | 0x10101010;
		XVphy_WriteReg(VPhyInst.Config.BaseAddr, XVPHY_RX_CONTROL_REG, DrpVal);

		/*
		 * Reset PRBS7 Counters
		 */
		DrpVal = XVphy_ReadReg(VPhyInst.Config.BaseAddr, XVPHY_RX_CONTROL_REG);
		DrpVal = DrpVal | 0x08080808;
		XVphy_WriteReg(VPhyInst.Config.BaseAddr, XVPHY_RX_CONTROL_REG, DrpVal);

		DrpVal = XVphy_ReadReg(VPhyInst.Config.BaseAddr, XVPHY_RX_CONTROL_REG);
		DrpVal = DrpVal & 0xF7F7F7F7;
		XVphy_WriteReg(VPhyInst.Config.BaseAddr, XVPHY_RX_CONTROL_REG, DrpVal);
	} else {
		/*
		 * Disable PRBS Mode in Video PHY
		 */
		DrpVal = XVphy_ReadReg(VPhyInst.Config.BaseAddr, XVPHY_RX_CONTROL_REG);
		DrpVal = DrpVal & 0xEFEFEFEF;
		XVphy_WriteReg(VPhyInst.Config.BaseAddr, XVPHY_RX_CONTROL_REG, DrpVal);
	}
}

/*****************************************************************************/
/**
*
* This function is the callback function for Access prbs error count.
*
* @param	InstancePtr is a pointer to the XDpRxSs instance.
*
*
* @note		None.
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
	XDpRxSs *DpRxSsPtr = (XDpRxSs *)InstancePtr;

	/*
	 * Read PRBS Error Counter Value from Video PHY
	 *
	 * Lane 0 - Store only lower 16 bits
	 */

	XVphy_DrpRd(&VPhyInst, 0, XVPHY_CHANNEL_ID_CH1,
		    XVPHY_DRP_GTHE4_PRBS_ERR_CNTR_LOWER,
		    &DrpVal_lower_lane0);
	XVphy_DrpRd(&VPhyInst, 0, XVPHY_CHANNEL_ID_CH1,
		    XVPHY_DRP_GTHE4_PRBS_ERR_CNTR_UPPER,
		    &DrpVal);

	/*
	 * Lane 1 - Store only lower 16 bits
	 */
	XVphy_DrpRd(&VPhyInst, 0, XVPHY_CHANNEL_ID_CH2,
		    XVPHY_DRP_GTHE4_PRBS_ERR_CNTR_LOWER,
		    &DrpVal_lower_lane1);
	XVphy_DrpRd(&VPhyInst, 0, XVPHY_CHANNEL_ID_CH2,
		    XVPHY_DRP_GTHE4_PRBS_ERR_CNTR_UPPER, &DrpVal);

	/*
	 * Lane 2 - Store only lower 16 bits
	 */
	XVphy_DrpRd(&VPhyInst, 0, XVPHY_CHANNEL_ID_CH3,
		    XVPHY_DRP_GTHE4_PRBS_ERR_CNTR_LOWER,
		    &DrpVal_lower_lane2);
	XVphy_DrpRd(&VPhyInst, 0, XVPHY_CHANNEL_ID_CH3,
		    XVPHY_DRP_GTHE4_PRBS_ERR_CNTR_UPPER, &DrpVal);

	/*
	 * Lane 3 - Store only lower 16 bits
	 */
	XVphy_DrpRd(&VPhyInst, 0, XVPHY_CHANNEL_ID_CH4,
		    XVPHY_DRP_GTHE4_PRBS_ERR_CNTR_LOWER,
		    &DrpVal_lower_lane3);
	XVphy_DrpRd(&VPhyInst, 0, XVPHY_CHANNEL_ID_CH4,
		    XVPHY_DRP_GTHE4_PRBS_ERR_CNTR_UPPER, &DrpVal);

	/*
	 * Update PRBS counters of DP Core.
	 */
	DpRxSs_UpdatePrbsErrCouters(DpRxSsPtr, DrpVal_lower_lane0, DrpVal_lower_lane1,
				    DrpVal_lower_lane2, DrpVal_lower_lane3);

	/*
	 * Reset PRBS7 Counters
	 */
	DrpVal1 = XVphy_ReadReg(VPhyInst.Config.BaseAddr, XVPHY_RX_CONTROL_REG);
	DrpVal1 = DrpVal1 | 0x08080808;

	XVphy_WriteReg(VPhyInst.Config.BaseAddr, XVPHY_RX_CONTROL_REG, DrpVal1);

	DrpVal1 = XVphy_ReadReg(VPhyInst.Config.BaseAddr, XVPHY_RX_CONTROL_REG);
	DrpVal1 = DrpVal1 & 0xF7F7F7F7;

	XVphy_WriteReg(VPhyInst.Config.BaseAddr, XVPHY_RX_CONTROL_REG, DrpVal1);
}

/*****************************************************************************/
/**
*
* This function sets proper ref clk frequency and line rate
*
* @param	InstancePtr is a pointer to the Video PHY instance.
* @param	link_rate is the link rate to be configured.
*
*
* @note		None.
*
******************************************************************************/
void PLLRefClkSel (XVphy *InstancePtr, u8 link_rate)
{
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
			XVphy_CfgLineRate(InstancePtr, 0, XVPHY_CHANNEL_ID_CHA,
					  XVPHY_DP_LINK_RATE_HZ_540GBPS);
			XVphy_CfgLineRate(InstancePtr, 0, VPHY_RX_CHANNEL_TYPE,
					  XVPHY_DP_LINK_RATE_HZ_540GBPS);
			break;
		case 0x1E:
			XVphy_CfgLineRate(InstancePtr, 0, XVPHY_CHANNEL_ID_CHA,
					  XVPHY_DP_LINK_RATE_HZ_810GBPS);
			XVphy_CfgLineRate(InstancePtr, 0, VPHY_RX_CHANNEL_TYPE,
					  XVPHY_DP_LINK_RATE_HZ_810GBPS);
			break;

		case 0x1:
			XVphy_CfgLineRate(InstancePtr, 0, XVPHY_CHANNEL_ID_CHA,
					  XVPHY_DP_LINK_RATE_HZ_1000GBPS);
			XVphy_CfgLineRate(InstancePtr, 0, VPHY_RX_CHANNEL_TYPE,
					  XVPHY_DP_LINK_RATE_HZ_1000GBPS);
			break;

		case 0x4:
			XVphy_CfgLineRate(InstancePtr, 0, XVPHY_CHANNEL_ID_CHA,
					  XVPHY_DP_LINK_RATE_HZ_1350GBPS);
			XVphy_CfgLineRate(InstancePtr, 0, VPHY_RX_CHANNEL_TYPE,
					  XVPHY_DP_LINK_RATE_HZ_1350GBPS);
			break;

		case 0x2:
			XVphy_CfgLineRate(InstancePtr, 0, XVPHY_CHANNEL_ID_CHA,
					  XVPHY_DP_LINK_RATE_HZ_2000GBPS);
			XVphy_CfgLineRate(InstancePtr, 0, VPHY_RX_CHANNEL_TYPE,
					  XVPHY_DP_LINK_RATE_HZ_2000GBPS);
			break;

		default:
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
*
* @note		None.
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
* @note		None.
*
******************************************************************************/
void ReportVideoCRC()
{
	XVidFrameCrc_Report();
}

#if !defined (PLATFORM_MB) && !defined (__riscv)
/*****************************************************************************/
/**
*
* This function is a non-blocking UART return byte
*
* @return	Received byte from UART.
*
* @note		None.
*
******************************************************************************/
char XUartPs_RecvByte_NonBlocking()
{
	u32 RecievedByte;
	RecievedByte = XUartPs_ReadReg(STDIN_BASEADDRESS,
				       XUARTPS_FIFO_OFFSET);
	/*
	 * Return the byte received
	 */
	return (u8)RecievedByte;
}
#endif

/*****************************************************************************/
/**
*
* This function is called when DisplayPort Subsystem core requires delay
* or sleep. It provides timer with predefined amount of loop iterations.
*
* @param	InstancePtr is a pointer to the XDp instance.
* @param	MicroSeconds is the delay time in microseconds.
*
* @note		None.
*
******************************************************************************/
void CustomWaitUs(void *InstancePtr, u32 MicroSeconds)
{
	u32 TimerVal;
	XDp *DpInstance = (XDp *)InstancePtr;
	u32 NumTicks = (MicroSeconds * (DpInstance->Config.SAxiClkHz / 1000000));

	XTmrCtr_Reset(DpInstance->UserTimerPtr, 0);
	XTmrCtr_Start(DpInstance->UserTimerPtr, 0);

	/*
	 * Wait specified number of useconds.
	 */
	do {
		TimerVal = XTmrCtr_GetValue(DpInstance->UserTimerPtr, 0);
	} while (TimerVal < NumTicks);
}

/*****************************************************************************/
/**
*
* This function Calculates CRC values of Video components
*
*
* @note		None.
*
******************************************************************************/
void CalculateCRC(void)
{
	u32 overflow;
	u32 loop = 0;
	u32 misses = 0;

	/*
	 * Reset CRC Test Counter in DP DPCD Space
	 */
	VidFrameCRC.TEST_CRC_CNT = 0;

	CustomWaitUs(DpRxSsInst.DpPtr, 100000);

	VidFrameCRC.Mode_422 = (XVidFrameCrc_ReadReg(DpRxSsInst.DpPtr->Config.BaseAddr,
				XDPRXSS_MSA_MISC0) >> 1) & 0x3 ;

	if (VidFrameCRC.Mode_422 != 0x1) {
		XVidFrameCrc_WriteReg(VIDEO_CRC_BASEADDR, VIDEO_FRAME_CRC_CONFIG, CRC_CFG);
	} else {
		XVidFrameCrc_WriteReg(VIDEO_CRC_BASEADDR, VIDEO_FRAME_CRC_CONFIG,
				      CRC_CFG | 0x80000000);
	}

	XVidFrameCrc_Reset();

	/*
	 * Add delay (~40 ms) for Frame CRC to compute on couple of frames
	 */
	CustomWaitUs(DpRxSsInst.DpPtr, 400000);
	CustomWaitUs(DpRxSsInst.DpPtr, 400000);

	/*
	 * Reading the overflow twice to clear the bit set
	 */
	overflow = XDpRxSs_UserFifoOverflowStatus(&DpRxSsInst) &
		   XDPRXSS_USER_FIFO_OVERFLOW_STREAM1_MASK;

	overflow = XDpRxSs_UserFifoOverflowStatus(&DpRxSsInst) &
		   XDPRXSS_USER_FIFO_OVERFLOW_STREAM1_MASK;

	overflow = 0;

	/*
	 * Reading overflow in loop of 500 in ideal scenario it should never be set
	 */
	while (loop < 500) {
		loop++;

		overflow |= (XDpRxSs_UserFifoOverflowStatus(&DpRxSsInst) &
			     XDPRXSS_USER_FIFO_OVERFLOW_STREAM1_MASK);

		CustomWaitUs(DpRxSsInst.DpPtr, 800);
	}

	misses = (XVidFrameCrc_ReadReg(VIDEO_CRC_BASEADDR,
				       VIDEO_FRAME_CRC_MISSES)) & 0x00000FFF;

	/*
	 * Read computed values from Frame CRC module and MISC0 for colorimetry
	 */
	VidFrameCRC.Pixel_r = XVidFrameCrc_ReadReg(VIDEO_CRC_BASEADDR,
						   VIDEO_FRAME_CRC_VALUE_G_R) & 0xFFFF;
	VidFrameCRC.Pixel_g = XVidFrameCrc_ReadReg(VIDEO_CRC_BASEADDR,
						   VIDEO_FRAME_CRC_VALUE_G_R) >> 16;
	VidFrameCRC.Pixel_b = XVidFrameCrc_ReadReg(VIDEO_CRC_BASEADDR,
						   VIDEO_FRAME_CRC_VALUE_B) & 0xFFFF;

	/*
	 * Write CRC values to DPCD TEST CRC space
	 */

	if (overflow == 0 && misses == 0) {
		/*
		 * Set CRC only when overflow is not there and no b2b CRC mismatch
		 */
		VidFrameCRC.TEST_CRC_CNT = 1;
	} else if (overflow && misses) {

		/*
		 * Not setting CRC available bit
		 * for overflow CRC forced to 0x0
		 * for b2b miss CRC forced to 0x1
		 * helps in identifying at TX report
		 */
	} else if (overflow) {
		overflow_count++;

	} else if (misses) {
		missed_count++;
	}

	VidFrameCRC.TEST_CRC_CNT = 1;

	xil_printf("[Video CRC] R/Cr: 0x%x, G/Y: 0x%x, B/Cb: 0x%x\r\n\n",
		   VidFrameCRC.Pixel_r, VidFrameCRC.Pixel_g, VidFrameCRC.Pixel_b);
}

/*****************************************************************************/
/**
*
* This function scans VFMC- IIC.
*
* @param	BaseAddress is the IIC base address.
*
* @note		None.
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
		BytesRecvd = XIic_Recv(BaseAddress, i, (u8*)Buffer, 1, XIIC_STOP);
		if (BytesRecvd == 0)
			continue;

		xil_printf("Found device: 0x%02x\n\r", i);
	}

	print("\n\r");
}

/*****************************************************************************/
/**
*
* This function is the callback function for Info Packet Handling.
*
* @param	InstancePtr is a pointer to the XDpRxSs instance.
*
* @note		None.
*
******************************************************************************/
void DpRxSs_InfoPacketHandler(void *InstancePtr)
{
	XDpRxSs *DpRxSsPtr = (XDpRxSs *)InstancePtr;
	u32 InfoFrame[9];
	int i;

	for (i = 1; i < 9; i++) {
		InfoFrame[i] = DpRxSs_GetRxAudioInfo(DpRxSsPtr, i);
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
}

/*****************************************************************************/
/**
*
* This function is the callback function for Generic Packet Handling of
* 32-Bytes payload.
*
* @param	InstancePtr is a pointer to the XDpRxSs instance.
*
* @note		None.
*
******************************************************************************/
void DpRxSs_ExtPacketHandler(void *InstancePtr)
{
	XDpRxSs *DpRxSsPtr = (XDpRxSs *)InstancePtr;
	u32 ExtFrame[9];
	int i;

	SdpExtFrame.frame_count++;

	/*
	 * Header Information
	 */
	ExtFrame[0] = DpRxSs_GetRxAudioExtInfo(DpRxSsPtr, 1);

	SdpExtFrame.Header[0] =  ExtFrame[0]&0xFF;
	SdpExtFrame.Header[1] = (ExtFrame[0]&0xFF00)>>8;
	SdpExtFrame.Header[2] = (ExtFrame[0]&0xFF0000)>>16;
	SdpExtFrame.Header[3] = (ExtFrame[0]&0xFF000000)>>24;

	/*
	 * Payload Information
	 */
	for (i = 0; i < 8; i++) {
		ExtFrame[i+1] = DpRxSs_GetRxAudioExtInfo(DpRxSsPtr, i+2);

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
* @note		None.
*
******************************************************************************/
void Print_InfoPkt()
{
	xil_printf("Received Audio Info Packet::\r\n");
	xil_printf(" -frame_count		: 0x%x \r\n", AudioinfoFrame.frame_count);
	xil_printf(" -version			: 0x%x \r\n", AudioinfoFrame.version);
	xil_printf(" -type			: 0x%x \r\n", AudioinfoFrame.type);
	xil_printf(" -sec_id			: 0x%x \r\n", AudioinfoFrame.sec_id);
	xil_printf(" -info_length		: 0x%x \r\n", AudioinfoFrame.info_length);
	xil_printf(" -audio_channel_count	: 0x%x \r\n",
		   AudioinfoFrame.audio_channel_count);
	xil_printf(" -audio_coding_type		: 0x%x \r\n",
		   AudioinfoFrame.audio_coding_type);
	xil_printf(" -sample_size		: 0x%x \r\n", AudioinfoFrame.sample_size);
	xil_printf(" -sampling_frequency	: 0x%x \r\n",
		   AudioinfoFrame.sampling_frequency);
	xil_printf(" -channel_allocation	: 0x%x \r\n",
		   AudioinfoFrame.channel_allocation);
	xil_printf(" -level_shift		: 0x%x \r\n", AudioinfoFrame.level_shift);
	xil_printf(" -downmix_inhibit		: 0x%x \r\n", AudioinfoFrame.downmix_inhibit);
}

/*****************************************************************************/
/**
*
* This function is the callback function for Ext Packet Handling.
*
* @note		None.
*
******************************************************************************/
void Print_ExtPkt()
{
	int i;

	xil_printf("Received SDP Packet Type::\r\n");

	switch (SdpExtFrame.Header[1]) {
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

	for (i = 0; i < 8; i++) {
		xil_printf(" Payload Bytes : 0x%x, 0x%x, 0x%x, 0x%x \r\n",
			   SdpExtFrame.Payload[(i*4)],
			   SdpExtFrame.Payload[(i*4)+1],
			   SdpExtFrame.Payload[(i*4)+2],
			   SdpExtFrame.Payload[(i*4)+3]);
	}

	xil_printf(" Frame Count : %d \r\n", SdpExtFrame.frame_count);
}

/*****************************************************************************/
/**
*
* This function to get uart input from user
*
* @param	timeout_ms
*
* @return
*		- received character
*
* @note		None.
*
******************************************************************************/
char xil_getc(u32 timeout_ms)
{
	char c;
	u32 timeout = 0;

	extern XTmrCtr TmrCtr;

	/*
	 * Reset and start timer
	 */
	if (timeout_ms > 0 && timeout_ms != 0xff) {
		XTmrCtr_Start(&TmrCtr, 0);
	}

#if !defined (PLATFORM_MB) && !defined (__riscv)
	while ((!XUartPs_IsReceiveData(STDIN_BASEADDRESS)) && (timeout == 0)) {
#else
	while (XUartLite_IsReceiveEmpty(STDIN_BASEADDRESS) && (timeout == 0)) {
#endif
		if (timeout_ms == 0) { /**< no timeout - wait for ever */
			timeout = 0;
		} else if (timeout_ms == 0xff) { /**< no wait - special case */
			timeout = 1;
		} else if (timeout_ms > 0) {
			if (XTmrCtr_GetValue(&TmrCtr, 0) > (timeout_ms * (100000000 / 1000))) {
				timeout = 1;
			}
		}
	}
	if (timeout == 1) {
		c = 0;
	} else {
#if !defined (PLATFORM_MB) && !defined (__riscv)
		c = XUartPs_RecvByte_NonBlocking();
#else
		c = XUartLite_RecvByte(STDIN_BASEADDRESS);
#endif
	}
	return c;
}
