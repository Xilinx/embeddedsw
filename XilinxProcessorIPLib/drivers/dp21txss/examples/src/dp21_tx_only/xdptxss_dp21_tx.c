/******************************************************************************
 * Copyright (C) 2020 - 2022 Xilinx, Inc. All rights reserved.
 * Copyright (C) 2023 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

/*****************************************************************************/
/**
 *
 * @file xdptxss_dp21_tx.c
 *
 * This file contains a design example using the XDpTxSs driver.
 * This example application is designed to run on the ZynqMP evaluation board
 * (ZCU102) and the MicroBlaze evaluation board (VCU118).
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver  Who      Date      Changes
 * ---- ---      --------  --------------------------------------------------.
 * 1.00  ND	18/10/22  Common DP 2.1 tx only application for zcu102 and
 *			  vcu118.
 * 1.01  ND	26/02/24  Added support for 13.5 and 20G.
 * 1.02  ND	24/03/25  Added support for PARRETO fmc.
 * 1.03  ND	02/05/25  Changed the VESA color encode format to CEA.
 * 1.04  KU	09/24/25  Optimization. Added gain for TDP2004, Audio support.
 * 2.7   GM	01/05/26  Improved formatting and removed non-PARRETTO FMC.
 * </pre>
 *
 ******************************************************************************/
#include "xdptxss_dp21_tx.h"
#include "xvidframe_crc.h"
#include "xil_cache.h"
#ifdef SDT
#include "xinterrupt_wrap.h"
#endif

XTmrCtr TmrCtr;			/**< Timer instance */
XIic_Config *ConfigPtr_IIC;     /**< Pointer to configuration data */
XVphy VPhyInst;			/**< The DPRX Subsystem instance */
XIic IicInstance;		/**< I2C bus for Si570 */
extern volatile u8 HpdPulseConEvent;
volatile u8 PrevLineRate;	/**< This previous line rate to keep previous info
				  to compare with new line rate request */

#if !defined (PLATFORM_MB) && !defined (__riscv)
XScuGic IntcInst;
#else
XIntc IntcInst;
#endif

#define INTRNAME_DPTX   0

extern XDpTxSs DpTxSsInst;	/**< The DPTX Subsystem instance */
extern XTmrCtr TmrCtr;		/**< Timer instance */

#if defined (PLATFORM_MB) || defined (__riscv)
XIic  IicPtr;
typedef u8 AddressType;

u8 Si570_RegValue[NUM_MODES][NUM_CLOCK_REGS] = {
	/*
	 *  As per Si570 programmable oscillator calculator
	 * 7,    8,    9,   10,   11,   12,
	 */
	{0x4C, 0x42, 0xB0, 0x21, 0xDE, 0x77 }, /**< = {32kHz * 512)   */
	{0xA5, 0xC2, 0xAA, 0xCC, 0x9D, 0x51 }, /**< = (44.1kHz * 512) */
	{0xE4, 0xC2, 0xF4, 0xB9, 0x4A, 0xA7 }, /**< = (48kHz * 512)   */
	{0xA2, 0XC2, 0XAA, 0XCC, 0X9D, 0X51 }, /**< = {88.2khZ * 512) */
	{0x24, 0xC2, 0xB0, 0x21, 0xDE, 0x77 }, /**< = {96kHz * 512)   */
	{0xA1, 0x42, 0xAA, 0xCC, 0x9D, 0x51 }, /**< = (176.4kHz * 512)*/
	{0x22, 0x42, 0xB0, 0x21, 0xDE, 0x77 }  /**< = {192kHz * 512)  */
};

u8 UpdateBuffer[sizeof(AddressType) + PAGE_SIZE];

#else
XIicPs Ps_Iic1;
XIicPs_Config *XIic1Ps_ConfigPtr;
#endif

#define PS_IIC_CLK 100000

/************************** Function Prototypes ******************************/
#ifndef SDT
u32 DpTxSs_Main(u16 DeviceId);
#else
u32 DpTxSs_Main(UINTPTR BaseAddress);
#endif
u32 DpTxSs_PlatformInit(void);

/*
 * Interrupt helper functions
 */
void DpTxSs_HpdEventHandler(void *InstancePtr);
void DpTxSs_HpdPulseHandler(void *InstancePtr);
void DpTxSs_LinkrateChgHandler(void *InstancePtr);
void DpTxSs_Pe_Vs_AdjustHandler(void *InstancePtr);
void DpTxSs_Ffe_AdjustHandler(void *InstancePtr);
void DpTxSs_ExtPkt_Handler(void *InstancePtr);
void DpTxSs_Vblank_Handler(void *InstancePtr);

u32 DpTxSs_SetupIntrSystem(void);
#ifndef SDT
u32 DpTxSs_PhyInit(u16 DeviceId);
#else
u32 DpTxSs_PhyInit(UINTPTR BaseAddress);
#endif
void DpTxSs_CustomWaitUs(void *InstancePtr, u32 MicroSeconds);
void DpTxSs_Setup(u8 *LineRate_init, u8 *LaneCount_init, u8 Edid_org[128],
		u8 Edid1_org[128]);
void PLLRefClkSel(XVphy *InstancePtr, u8 link_rate, XVphy_ChannelId Tx_Channel);
void ClkWiz_Locked(void);
void DpTxSs_HpdPulseCon(XDpTxSs *InstancePtr);
extern void Gen_vid_clk(XDp *InstancePtr, u8 Stream);
int Vpg_StreamSrcConfigure(XDp *InstancePtr, u8 VSplitMode, u8 first_time);
void Vpg_VidgenSetUserPattern(XDp *InstancePtr, u8 Pattern);
static u8 CalculateChecksum(u8 *Data, u8 Size);
XVidC_VideoMode GetPreferredVm(u8 *EdidPtr, u8 cap, u8 lane);
void ReportVideoCRC(void);
extern void main_loop(void);
int Si570_Write(u8 *UpdateBuffer);

volatile u8 TxReconnected = 0;
Video_CRC_Config VidFrameCRC; /**< Video Frame CRC instance */

/************************** Variable Definitions *****************************/
#define XVPHY_DP_LINK_RATE_HZ_1000GBPS	10000000000LL
#define XVPHY_DP_LINK_RATE_HZ_1350GBPS	13500000000LL
#define XVPHY_DP_LINK_RATE_HZ_2000GBPS	20000000000LL

/*
 * Set the TX PLL and Channel based on the VPHY config
 */
#if (XPAR_XVPHY_0_TX_PLL_SELECTION == 0x1)
XVphy_PllType VPHY_TX_PLL_TYPE = XVPHY_PLL_TYPE_QPLL0;
XVphy_ChannelId VPHY_TX_CHANNEL_TYPE = XVPHY_CHANNEL_ID_CMN0;
#endif

#if (XPAR_XVPHY_0_TX_PLL_SELECTION == 0x2)
XVphy_PllType VPHY_TX_PLL_TYPE = XVPHY_PLL_TYPE_QPLL1;
XVphy_ChannelId VPHY_TX_CHANNEL_TYPE = XVPHY_CHANNEL_ID_CMN1;
#endif

/*
 * These are the REFCLK sources for VCU118 and ZCU102
 */
#if defined (PLATFORM_MB) || defined (__riscv) /**< VCU118 (270Mhz on REFCLK0, 400Mhz on REFCLK1) */
XVphy_PllRefClkSelType VPHY_TX_REFCLK_SEL_270 = XVPHY_REF_CLK_SEL_XPLL_GTREFCLK0;
XVphy_PllRefClkSelType VPHY_TX_REFCLK_SEL_400 = XVPHY_REF_CLK_SEL_XPLL_GTREFCLK1;
#else /**< ZCU102 (270Mhz on REFCLK0, 400Mhz on NORTHREFCLK0) */
XVphy_PllRefClkSelType VPHY_TX_REFCLK_SEL_270 = XVPHY_REF_CLK_SEL_XPLL_GTREFCLK0;
XVphy_PllRefClkSelType VPHY_TX_REFCLK_SEL_400 = XVPHY_REF_CLK_SEL_XPLL_GTNORTHREFCLK0;
#endif

void enable_caches()
{
#ifdef __PPC__
	Xil_ICacheEnableRegion(CACHEABLE_REGION_MASK);
	Xil_DCacheEnableRegion(CACHEABLE_REGION_MASK);
#elif __MICROBLAZE__
#ifdef XPAR_MICROBLAZE_USE_ICACHE
	Xil_ICacheEnable();
#endif
#ifdef XPAR_MICROBLAZE_USE_DCACHE
	Xil_DCacheEnable();
#endif
#endif
}

void disable_caches()
{
#ifdef __MICROBLAZE__
#ifdef XPAR_MICROBLAZE_USE_DCACHE
	Xil_DCacheDisable();
#endif
#ifdef XPAR_MICROBLAZE_USE_ICACHE
	Xil_ICacheDisable();
#endif
#endif
}

/************************** Function Definitions *****************************/
/*****************************************************************************/
/**
 *
 * This is the main function for XDpTxSs interrupt example. If the
 * DpTxSs_IntrExample function which sets up the system succeeds, this function
 * will wait for the interrupts. Once a connection event or pulse is detected,
 * DpTxSs will RX device capabilities and re-start the subsystem.
 *
 * @param	None.
 *
 * @return
 *		- XST_FAILURE if the interrupt example was unsuccessful.
 *
 * @note		Unless setup failed, main will never return since
 *		DpTxSs_IntrExample is blocking (it is waiting on interrupts
 *		for Hot-Plug-Detect (HPD) events.
 *
 ******************************************************************************/
int main()
{
	u32 Status;

	enable_caches();

	xil_printf("------------------------------------------\r\n");
	xil_printf("DisplayPort 2.x TX Subsystem Example Design\r\n");
	xil_printf("(c) 2026 by AMD\r\n");
	xil_printf("-------------------------------------------\r\n\r\n");
#ifndef SDT
	Status = DpTxSs_Main(XDPTXSS_DEVICE_ID);
#else
	Status = DpTxSs_Main(XPAR_DPTXSS_0_BASEADDR);
#endif
	if (Status != XST_SUCCESS) {
		xil_printf("DisplayPort TX Subsystem interrupt example failed.\n");
		return XST_FAILURE;
	}

	xil_printf("Successfully ran DisplayPort TX Subsystem interrupt example\r\n");

	disable_caches();

	return XST_SUCCESS;
}

#if !defined (PLATFORM_MB) && !defined (__riscv)
int I2cMux_Ps(u8 Data)
{
	u8 Buffer;
	int Status;

	Buffer = Data;

	Status = XIicPs_MasterSendPolled(&Ps_Iic1, (u8 *)&Buffer, 1, I2C_MUX_ADDR2);
	if (Status != XST_SUCCESS) {
		xil_printf ("mux failed\r\n");
	}

	return Status;
}
#endif

static int DpTxSs_ConfigAudioClock(u32 AudioClkHz)
{
	int Status = XST_SUCCESS;
#if defined (PLATFORM_MB) || defined (__riscv)
	u8 i;
#endif

	/*
	 * I2C MUX device address : 0x74
	 * Si570 device address : 0x5D
	 * setting Si570 on zcu102 to be 24.576MHz for audio
	 */
#if !defined (PLATFORM_MB) && !defined (__riscv)
	/*
	 * opening the mux for on board Si570 programming
	 */
	I2cMux_Ps(0x04);

	clk_set(I2C_MUX_ADDR2, IIC_SI570_ADDRESS, AudioClkHz);

	/*
	 * Closing the Mux
	 */
	I2cMux_Ps(0x0);
#else
	(void)AudioClkHz;

	/*
	 * MB path: use precomputed register values (index 2 -> 48kHz*512 = 24.576MHz)
	 */
	for (i = 0; i < 6; i++)
		UpdateBuffer[i] = Si570_RegValue[2][i];

	Status = Si570_Write(UpdateBuffer);
	if (Status != XST_SUCCESS) {
		xil_printf("Failed to program Si570\r\n");
		return Status;
	} else {
		xil_printf("Programmed Si570..\r\n");
	}
#endif

	return Status;
}

/*****************************************************************************/
/**
 *
 * This function is the main entry point for the interrupt example using the
 * XDpTxSs driver. This function will set up the system with interrupts and
 * set up Hot-Plug-Event (HPD) handlers.
 *
 * @param	DeviceId is the unique device ID of the DisplayPort TX
 *		Subsystem core.
 *
 * @return
 *		- XST_FAILURE if the system setup failed.
 *		- XST_SUCCESS should never return since this function, if setup
 *		  was successful, is blocking.
 *
 * @note		If system setup was successful, this function is blocking in
 *		order to illustrate interrupt handling taking place for HPD
 *		events.
 *
 ******************************************************************************/
#ifndef SDT
u32 DpTxSs_Main(u16 DeviceId)
#else
u32 DpTxSs_Main(UINTPTR BaseAddress)
#endif
{
	u32 Status;
	XDpTxSs_Config *ConfigPtr;
	u8 LineRate_init = XDP_TX_LINK_BW_SET_540GBPS;
	u8 LineRate_init_tx = XDP_TX_LINK_BW_SET_540GBPS;
	u8 LaneCount_init = XDP_TX_LANE_COUNT_SET_4;
	u8 LaneCount_init_tx = XDP_TX_LANE_COUNT_SET_4;
	u8 Edid_org[128], Edid1_org[128];
	u8 connected = 0;

	user_config_struct user_config;
	user_config.user_bpc = 8;
	user_config.VideoMode_local = XVIDC_VM_800x600_60_P;
	user_config.user_pattern = 1; /**< Color Ramp (Default) */
	user_config.user_format = XVIDC_CSF_RGB;

	/*
	 * Do platform initialization in this function. This is hardware
	 * system specific. It is up to the user to implement this function.
	 */
	xil_printf("PlatformInit\r\n");

	Status = DpTxSs_PlatformInit();
	if (Status != XST_SUCCESS) {
		xil_printf("Platform init failed!\r\n");
	}

	xil_printf("Platform initialization done.\r\n");

#if ENABLE_AUDIO
	Status = DpTxSs_ConfigAudioClock(audio_clk_Hz);
	if (Status != XST_SUCCESS) {
		xil_printf("ERR: Configuring audio clock failed\r\n");
	}
#endif

	VideoFMC_Init();

	/*
	 * Do platform initialization in this function. This is hardware
	 * system specific. It is up to the user to implement this function.
	 *
	 * Obtain the device configuration for the DisplayPort TX Subsystem
	 */
#ifndef SDT
	ConfigPtr = XDpTxSs_LookupConfig(DeviceId);
#else
	ConfigPtr = XDpTxSs_LookupConfig(BaseAddress);
#endif
	if (!ConfigPtr) {
		return XST_FAILURE;
	}

	/*
	 * Copy the device configuration into the DpTxSsInst's Config structure.
	 */
	Status = XDpTxSs_CfgInitialize(&DpTxSsInst, ConfigPtr, ConfigPtr->BaseAddress);
	if (Status != XST_SUCCESS) {
		xil_printf("DPTXSS config initialization failed.\r\n");
		return XST_FAILURE;
	}

	/*
	 * Check for SST/MST support
	 */
	if (DpTxSsInst.UsrOpt.MstSupport) {
		xil_printf("\r\nINFO:DPTXSS is MST enabled. DPTXSS can be "
				"switched to SST/MST\r\n");
	} else {
		xil_printf("\r\nINFO:DPTXSS is  SST enabled. DPTXSS works "
				"only in SST mode.\r\n");
	}

	Status = DpTxSs_SetupIntrSystem();
	if (Status != XST_SUCCESS) {
		xil_printf("ERR: Interrupt system setup failed.\r\n");
		return XST_FAILURE;
	}

	/*
	 * Setup Video Phy, left to the user for implementation
	 */
#ifndef SDT
	DpTxSs_PhyInit(XVPHY_DEVICE_ID);
#else
	DpTxSs_PhyInit(XPAR_XVPHY_0_BASEADDR);
#endif
	DpTxSs_Setup(&LineRate_init, &LaneCount_init, Edid_org, Edid1_org);

	/*
	 * Check if monitor is connected or not
	 * This is intentional infinite while loop
	 */

	while (!XDpTxSs_IsConnected(&DpTxSsInst)) {
		if (connected == 0) {
			xil_printf("Please connect a DP Monitor to start the application !!!\r\n");
			connected = 1;
		}
	}

	/*
	 * Waking up the monitor
	 */
	DpTxSs_Sink_PowerCycle();

	/*
	 * Do not return in order to allow interrupt handling to run. HPD events
	 * (connect, disconnect, and pulse) will be detected and handled.
	 */
	DpTxSsInst.DpPtr->TxInstance.TxSetMsaCallback = NULL;
	DpTxSsInst.DpPtr->TxInstance.TxMsaCallbackRef = NULL;
	DpTxSsInst.DpPtr->TxInstance.MsaConfig[0].ComponentFormat = 0x0;

	XVphy_BufgGtReset(&VPhyInst, XVPHY_DIR_TX,(FALSE));

	/*
	 * This configures the vid_phy for line rate to start with
	 * Even though CPLL can be used in limited case,
	 * using QPLL is recommended for more coverage.
	 */
	DpTxSs_PhyConfig(LineRate_init_tx, VPHY_TX_PLL_TYPE, VPHY_TX_CHANNEL_TYPE);

	LaneCount_init_tx = LaneCount_init_tx & 0x7;

	XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr, XDP_TX_INTERRUPT_MASK, 0xFFFFFFFF);

	XDpTxSs_SetLinkRate(&DpTxSsInst, 0x1E);
	XDpTxSs_SetLaneCount(&DpTxSsInst, 0x4);

	DpTxSs_StartTx(LineRate_init_tx, LaneCount_init_tx, user_config);

	while(1) { /**< For menu loop */
		main_loop();
	}

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
 *
 * This function initialize required platform-specifc peripherals.
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
u32 DpTxSs_PlatformInit(void)
{

	u32 Status;

#ifndef SDT
	/*
	 * Initialize timer.
	 */
	Status = XTmrCtr_Initialize(&TmrCtr, XPAR_TMRCTR_0_DEVICE_ID);
	if (Status != XST_SUCCESS) {
		xil_printf("ERR:Timer failed to initialize. \r\n");
		return XST_FAILURE;
	}

	/*
	 * Set up timer options.
	 */
	XTmrCtr_SetResetValue(&TmrCtr, XTC_TIMER_0, TIMER_RESET_VALUE);
	XTmrCtr_Start(&TmrCtr, XTC_TIMER_0);
#else
	/*
	 * Initialize timer.
	 */
	Status = XTmrCtr_Initialize(&TmrCtr, XPAR_XTMRCTR_0_BASEADDR);
	if (Status != XST_SUCCESS) {
		xil_printf("ERR:Timer failed to initialize. \r\n");
		return XST_FAILURE;
	}

	/*
	 * Set up timer options.
	 */
	XTmrCtr_SetResetValue(&TmrCtr, XTC_TIMER_0, TIMER_RESET_VALUE);
	XTmrCtr_Start(&TmrCtr, XTC_TIMER_0);
#endif

#if !defined (PLATFORM_MB) && !defined (__riscv)
#ifndef SDT
	XIic1Ps_ConfigPtr = XIicPs_LookupConfig(XPAR_XIICPS_1_DEVICE_ID);
#else
	XIic1Ps_ConfigPtr = XIicPs_LookupConfig(XPAR_XIICPS_1_BASEADDR);
#endif
	if (NULL == XIic1Ps_ConfigPtr)
		return XST_FAILURE;

	Status = XIicPs_CfgInitialize(&Ps_Iic1, XIic1Ps_ConfigPtr,
			XIic1Ps_ConfigPtr->BaseAddress);
	if (Status != XST_SUCCESS)
		return XST_FAILURE;

	XIicPs_Reset(&Ps_Iic1);

	/*
	 * Set the IIC serial clock rate.
	 */
	XIicPs_SetSClk(&Ps_Iic1, PS_IIC_CLK);
#endif

	/*
	 * Initialize the IIC driver so that it is ready to use.
	 */
#ifndef SDT
	ConfigPtr_IIC = XIic_LookupConfig(IIC_DEVICE_ID);
	if (ConfigPtr_IIC == NULL)
		return XST_FAILURE;
#else
	ConfigPtr_IIC = XIic_LookupConfig(XPAR_XIIC_0_BASEADDR);
	if (!ConfigPtr_IIC)
		return XST_FAILURE;
#endif
	Status = XIic_CfgInitialize(&IicInstance, ConfigPtr_IIC,
			ConfigPtr_IIC->BaseAddress);
	if (Status != XST_SUCCESS)
		return XST_FAILURE;

	XVidFrameCrc_Initialize(&VidFrameCRC);

	return XST_SUCCESS;
}

void DpTxSs_SetUserCallBackHandlers(void)
{
	/*
	 * Set custom timer wait
	 */
	XDpTxSs_SetUserTimerHandler(&DpTxSsInst, &DpTxSs_CustomWaitUs, &TmrCtr);

	XDpTxSs_SetCallBack(&DpTxSsInst, XDPTXSS_HANDLER_DP_HPD_EVENT,
			(void *)DpTxSs_HpdEventHandler, &DpTxSsInst);
	XDpTxSs_SetCallBack(&DpTxSsInst, XDPTXSS_HANDLER_DP_HPD_PULSE,
			(void *)DpTxSs_HpdPulseHandler, &DpTxSsInst);
	XDpTxSs_SetCallBack(&DpTxSsInst, XDPTXSS_HANDLER_DP_LINK_RATE_CHG,
			(void *)DpTxSs_LinkrateChgHandler, &DpTxSsInst);
	XDpTxSs_SetCallBack(&DpTxSsInst, XDPTXSS_HANDLER_DP_PE_VS_ADJUST,
			(void *)DpTxSs_Pe_Vs_AdjustHandler, &DpTxSsInst);
	XDpTxSs_SetCallBack(&DpTxSsInst, XDPTXSS_HANDLER_DP_EXT_PKT_EVENT,
			(void *)DpTxSs_ExtPkt_Handler, &DpTxSsInst);
	XDpTxSs_SetCallBack(&DpTxSsInst, XDPTXSS_HANDLER_DP_VSYNC,
			(void *)DpTxSs_Vblank_Handler, &DpTxSsInst);
	XDpTxSs_SetCallBack(&DpTxSsInst, XDPTXSS_HANDLER_DP_FFE_PRESET_ADJUST,
			(void *)DpTxSs_Ffe_AdjustHandler, &DpTxSsInst);
}

/*****************************************************************************/
/**
 *
 * This function sets up the interrupt system so interrupts can occur for the
 * DisplayPort TX Subsystem core. The function is application-specific since
 * the actual system may or may not have an interrupt controller. The DPTX
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
#ifndef SDT
u32 DpTxSs_SetupIntrSystem(void)
{
	u32 Status;
#if !defined (PLATFORM_MB) && !defined (__riscv)
	XINTC *IntcInstPtr = &IntcInst;
#else
	XIntc *IntcInstPtr = &IntcInst;
#endif

	/*
	 * Set user callbacks for all the interrupts
	 */
	DpTxSs_SetUserCallBackHandlers();

#if !defined (PLATFORM_MB) && !defined (__riscv)
	/*
	 * The configuration parameters of the interrupt controller
	 */
	XScuGic_Config *IntcConfig;

	/*
	 * Initialize the interrupt controller driver so that it is ready to use.
	 */
	IntcConfig = XScuGic_LookupConfig(XINTC_DEVICE_ID);
	if (NULL == IntcConfig)
		return XST_FAILURE;

	Status = XScuGic_CfgInitialize(IntcInstPtr, IntcConfig,
			IntcConfig->CpuBaseAddress);
	if (Status != XST_SUCCESS)
		return XST_FAILURE;

	/*
	 * Connect the device driver handler that will be called when an
	 * interrupt for the device occurs, the handler defined above performs
	 * the specific interrupt processing for the device
	 */
	Status = XScuGic_Connect(IntcInstPtr, XINTC_DPTXSS_DP_INTERRUPT_ID,
			(Xil_InterruptHandler)XDpTxSs_DpIntrHandler,
			&DpTxSsInst);
	if (Status != XST_SUCCESS) {
		xil_printf("ERR: DP TX SS DP interrupt connect failed!\r\n");
		return XST_FAILURE;
	}

	/*
	 * Enable the interrupt
	 */
	XScuGic_Enable(IntcInstPtr, XINTC_DPTXSS_DP_INTERRUPT_ID);
#else

	/*
	 * Initialize the interrupt controller driver so that it is ready to use.
	 */
	Status = XIntc_Initialize(IntcInstPtr, XINTC_DEVICE_ID);
	if (Status != XST_SUCCESS)
		return XST_FAILURE;

	/*
	 * Connect the device driver handler that will be called when an
	 * interrupt for the device occurs, the handler defined above performs
	 * the specific interrupt processing for the device
	 */
	Status = XIntc_Connect(IntcInstPtr, XINTC_DPTXSS_DP_INTERRUPT_ID,
			(XInterruptHandler)XDpTxSs_DpIntrHandler,
			&DpTxSsInst);
	if (Status != XST_SUCCESS) {
		xil_printf("ERR: DP TX SS DP interrupt connect failed!\r\n");
		return XST_FAILURE;
	}

	/*
	 * Enable the interrupt
	 */
	XIntc_Enable(IntcInstPtr, XINTC_DPTXSS_DP_INTERRUPT_ID);

	Status = XIntc_Start(IntcInstPtr, XIN_REAL_MODE);
	if (Status != XST_SUCCESS)
		return XST_FAILURE;
#endif

	/*
	 * Initialize the exception table.
	 */
	Xil_ExceptionInit();

	/*
	 * Register the interrupt controller handler with the exception
	 * table.
	 */
	Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_INT,
			(Xil_ExceptionHandler)XINTC_HANDLER, IntcInstPtr);

	/*
	 * Enable exceptions.
	 */
	Xil_ExceptionEnable();

	return (XST_SUCCESS);
}
#else
u32 DpTxSs_SetupIntrSystem(void)
{
	u32 Status;
	/*
	 * Set user callbacks for all the interrupts
	 */
	DpTxSs_SetUserCallBackHandlers();

	Status = XSetupInterruptSystem(&DpTxSsInst, XDpTxSs_DpIntrHandler,
			DpTxSsInst.Config.IntrId[INTRNAME_DPTX],
			DpTxSsInst.Config.IntrParent,
			XINTERRUPT_DEFAULT_PRIORITY);
	if (Status != XST_SUCCESS) {
		xil_printf("ERR: DP TX SS DP interrupt connect failed!\r\n");
		return XST_FAILURE;
	}

	return XST_SUCCESS;
}
#endif
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
#ifndef SDT
u32 DpTxSs_PhyInit(u16 DeviceId)
#else
u32 DpTxSs_PhyInit(UINTPTR BaseAddress)
#endif
{
	XVphy_Config *ConfigPtr;

	/*
	 * Obtain the device configuration for the DisplayPort RX Subsystem
	 */
#ifndef SDT
	ConfigPtr = XVphy_LookupConfig(DeviceId);
#else
	ConfigPtr = XVphy_LookupConfig(BaseAddress);
#endif
	if (!ConfigPtr)
		return XST_FAILURE;

	PLLRefClkSel(&VPhyInst, XDP_TX_LINK_BW_SET_540GBPS, VPHY_TX_CHANNEL_TYPE);

	XVphy_DpInitialize(&VPhyInst, ConfigPtr, 0,
			VPHY_TX_REFCLK_SEL_270,
			VPHY_TX_REFCLK_SEL_270,
			VPHY_TX_PLL_TYPE,
			XVPHY_PLL_TYPE_CPLL,
			XDP_TX_LINK_BW_SET_540GBPS);

	/*
	 * Initial line rate setting
	 */
	PrevLineRate = XDP_TX_LINK_BW_SET_540GBPS;

	return XST_SUCCESS;
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
void PLLRefClkSel(XVphy *InstancePtr, u8 link_rate, XVphy_ChannelId Tx_Channel)
{
	XVphy_CfgQuadRefClkFreq(InstancePtr, 0, VPHY_TX_REFCLK_SEL_270, 270000000);
	XVphy_CfgQuadRefClkFreq(InstancePtr, 0, VPHY_TX_REFCLK_SEL_400, 400000000);

	switch (link_rate) {
		case XDP_TX_LINK_BW_SET_162GBPS:
			XVphy_CfgLineRate(InstancePtr, 0, XVPHY_CHANNEL_ID_CHA,
					XVPHY_DP_LINK_RATE_HZ_162GBPS);
			XVphy_CfgLineRate(InstancePtr, 0, Tx_Channel, XVPHY_DP_LINK_RATE_HZ_162GBPS);
			break;

		case XDP_TX_LINK_BW_SET_540GBPS:
			XVphy_CfgLineRate(InstancePtr, 0, XVPHY_CHANNEL_ID_CHA,
					XVPHY_DP_LINK_RATE_HZ_540GBPS);
			XVphy_CfgLineRate(InstancePtr, 0, Tx_Channel, XVPHY_DP_LINK_RATE_HZ_540GBPS);
			break;

		case XDP_TX_LINK_BW_SET_810GBPS:
			XVphy_CfgLineRate(InstancePtr, 0, XVPHY_CHANNEL_ID_CHA,
					XVPHY_DP_LINK_RATE_HZ_810GBPS);
			XVphy_CfgLineRate(InstancePtr, 0, Tx_Channel, XVPHY_DP_LINK_RATE_HZ_810GBPS);
			break;

		case XDP_TX_LINK_BW_SET_UHBR10:
			XVphy_CfgLineRate(InstancePtr, 0, XVPHY_CHANNEL_ID_CHA,
					XVPHY_DP_LINK_RATE_HZ_1000GBPS);
			XVphy_CfgLineRate(InstancePtr, 0, Tx_Channel,
					XVPHY_DP_LINK_RATE_HZ_1000GBPS);
			break;

		case XDP_TX_LINK_BW_SET_UHBR20:
			XVphy_CfgLineRate(InstancePtr, 0, XVPHY_CHANNEL_ID_CHA,
					XVPHY_DP_LINK_RATE_HZ_2000GBPS);
			XVphy_CfgLineRate(InstancePtr, 0, Tx_Channel,
					XVPHY_DP_LINK_RATE_HZ_2000GBPS);
			break;

		case XDP_TX_LINK_BW_SET_UHBR135:
			XVphy_CfgLineRate(InstancePtr, 0, XVPHY_CHANNEL_ID_CHA,
					XVPHY_DP_LINK_RATE_HZ_1350GBPS);
			XVphy_CfgLineRate(InstancePtr, 0, Tx_Channel,
					XVPHY_DP_LINK_RATE_HZ_1350GBPS);
			break;

		default:
			XVphy_CfgLineRate(InstancePtr, 0, XVPHY_CHANNEL_ID_CHA,
					XVPHY_DP_LINK_RATE_HZ_270GBPS);
			XVphy_CfgLineRate(InstancePtr, 0, Tx_Channel, XVPHY_DP_LINK_RATE_HZ_270GBPS);
			break;
	}
}

/*****************************************************************************/
/**
 *
 * This function sets link line rate
 *
 * @param
 *
 * @return	None.
 *
 * @note		None.
 *
 ******************************************************************************/
void DpTxSs_LinkrateChgHandler(void *InstancePtr)
{
	/*
	 * If TX is unable to train at what it has been asked then
	 * necessary down shift handling has to be done here
	 * eg. reconfigure GT to new rate etc
	 */

	u8 rate;
	rate = get_LineRate();

	(void)InstancePtr;

	get_Lanecounts();
	DpTxSs_PhyConfig(rate, VPHY_TX_PLL_TYPE, VPHY_TX_CHANNEL_TYPE);

	/*
	 * update the previous link rate info at here
	 */
	PrevLineRate = rate;
}

void DpTxSs_Ffe_AdjustHandler(void *InstancePtr)
{
	u16 preemp = 0;
	u16 diff_swing = 0;
	XDpTxSs *DpTxSsPtr = (XDpTxSs *)InstancePtr;

	switch (DpTxSsPtr->DpPtr->TxInstance.LinkConfig.PresetVal[0]) {
		case 0:
			preemp = 0x5; /**< XVPHY_GTHE4_PREEMP_DP_L0 */
			diff_swing = XVPHY_GTHE4_DIFF_SWING_DP_DP20;
			break;
		case 1:
			preemp = XVPHY_GTHE4_PREEMP_DP_L1;
			diff_swing = XVPHY_GTHE4_DIFF_SWING_DP_DP20;
			break;
		case 2:
			preemp = XVPHY_GTHE4_PREEMP_DP_L2;
			diff_swing = XVPHY_GTHE4_DIFF_SWING_DP_DP20;
			break;
		case 3:
			preemp = XVPHY_GTHE4_PREEMP_DP_L3;
			diff_swing = XVPHY_GTHE4_DIFF_SWING_DP_DP20;
			break;
		default:
			preemp = XVPHY_GTHE4_PREEMP_DP_L3;
			diff_swing = XVPHY_GTHE4_DIFF_SWING_DP_DP20;
			break;
	}

	XVphy_SetTxVoltageSwing(&VPhyInst, 0, XVPHY_CHANNEL_ID_CH1,
			diff_swing);
	XVphy_SetTxVoltageSwing(&VPhyInst, 0, XVPHY_CHANNEL_ID_CH2,
			diff_swing);
	XVphy_SetTxVoltageSwing(&VPhyInst, 0, XVPHY_CHANNEL_ID_CH3,
			diff_swing);
	XVphy_SetTxVoltageSwing(&VPhyInst, 0, XVPHY_CHANNEL_ID_CH4,
			diff_swing);

	XVphy_SetTxPostCursor(&VPhyInst, 0, XVPHY_CHANNEL_ID_CH1, preemp);
	XVphy_SetTxPostCursor(&VPhyInst, 0, XVPHY_CHANNEL_ID_CH2, preemp);
	XVphy_SetTxPostCursor(&VPhyInst, 0, XVPHY_CHANNEL_ID_CH3, preemp);
	XVphy_SetTxPostCursor(&VPhyInst, 0, XVPHY_CHANNEL_ID_CH4, preemp);
}

void DpTxSs_Pe_Vs_AdjustHandler(void *InstancePtr)
{
	XDpTxSs *DpTxSsPtr = (XDpTxSs *)InstancePtr;

	if (PE_VS_ADJUST == 1) {
		u16 preemp = 0;

		switch (DpTxSsPtr->DpPtr->TxInstance.LinkConfig.PeLevel) {
			case 0:
				preemp = XVPHY_GTHE4_PREEMP_DP_L0;
				break;
			case 1:
				preemp = XVPHY_GTHE4_PREEMP_DP_L1;
				break;
			case 2:
				preemp = XVPHY_GTHE4_PREEMP_DP_L2;
				break;
			case 3:
				preemp = XVPHY_GTHE4_PREEMP_DP_L3;
				break;
		}

		u16 diff_swing = 0;

		switch (DpTxSsPtr->DpPtr->TxInstance.LinkConfig.VsLevel) {
			case 0:
				switch(DpTxSsPtr->DpPtr->TxInstance.LinkConfig.PeLevel) {
					case 0:
						diff_swing = XVPHY_GTHE4_DIFF_SWING_DP_V0P0;
						break;
					case 1:
						diff_swing = XVPHY_GTHE4_DIFF_SWING_DP_V0P1;
						break;
					case 2:
						diff_swing = XVPHY_GTHE4_DIFF_SWING_DP_V0P2;
						break;
					case 3:
						diff_swing = XVPHY_GTHE4_DIFF_SWING_DP_V0P3;
						break;
				}
				break;
			case 1:
				switch (DpTxSsPtr->DpPtr->TxInstance.LinkConfig.PeLevel) {
					case 0:
						diff_swing = XVPHY_GTHE4_DIFF_SWING_DP_V1P0;
						break;
					case 1:
						diff_swing = XVPHY_GTHE4_DIFF_SWING_DP_V1P1;
						break;
					case 2:
					case 3:
						diff_swing = XVPHY_GTHE4_DIFF_SWING_DP_V1P2;
						break;
				}
				break;
			case 2:
				switch (DpTxSsPtr->DpPtr->TxInstance.LinkConfig.PeLevel) {
					case 0:
						diff_swing = XVPHY_GTHE4_DIFF_SWING_DP_V2P0;
						break;
					case 1:
					case 2:
					case 3:
						diff_swing = XVPHY_GTHE4_DIFF_SWING_DP_V2P1;
						break;
				}
				break;
			case 3:
				diff_swing = XVPHY_GTHE4_DIFF_SWING_DP_V3P0;
				break;
		}

		/*
		 * setting vswing
		 */
		XVphy_SetTxVoltageSwing(&VPhyInst, 0, XVPHY_CHANNEL_ID_CH1, diff_swing);
		XVphy_SetTxVoltageSwing(&VPhyInst, 0, XVPHY_CHANNEL_ID_CH2, diff_swing);
		XVphy_SetTxVoltageSwing(&VPhyInst, 0, XVPHY_CHANNEL_ID_CH3, diff_swing);
		XVphy_SetTxVoltageSwing(&VPhyInst, 0, XVPHY_CHANNEL_ID_CH4, diff_swing);

		/*
		 * setting postcursor
		 */
		XVphy_SetTxPostCursor(&VPhyInst, 0, XVPHY_CHANNEL_ID_CH1, preemp);
		XVphy_SetTxPostCursor(&VPhyInst, 0, XVPHY_CHANNEL_ID_CH2, preemp);
		XVphy_SetTxPostCursor(&VPhyInst, 0, XVPHY_CHANNEL_ID_CH3, preemp);
		XVphy_SetTxPostCursor(&VPhyInst, 0, XVPHY_CHANNEL_ID_CH4, preemp);
	}
}

void DpTxSs_ExtPkt_Handler(void *InstancePtr)
{
	/*
	 * Empty
	 */
	(void)InstancePtr;
}

void DpTxSs_Vblank_Handler(void *InstancePtr)
{
	/*
	 * Empty
	 */
	(void)InstancePtr;
}

void DpTxSs_CustomWaitUs(void *InstancePtr, u32 MicroSeconds)
{
	u32 TimerVal = 0, TimerVal_pre;
	u32 NumTicks = (MicroSeconds * (
#ifndef SDT
				XPAR_PROCESSOR_SYSTEM_AXI_TIMER_0_CLOCK_FREQ_HZ / 1000000));
#else
	XPAR_PROCESSOR_SYSTEM_AXI_TIMER_0_CLOCK_FREQUENCY / 1000000));
#endif
	(void)InstancePtr;

	XTmrCtr_Reset(&TmrCtr, 0);
	XTmrCtr_Start(&TmrCtr, 0);

	/*
	 * Wait specified number of useconds.
	 */
	do {
		TimerVal_pre = TimerVal;
		TimerVal = XTmrCtr_GetValue(&TmrCtr, 0);
		if (TimerVal_pre == TimerVal) {
			break;
		}
	} while (TimerVal < NumTicks);
}

/*****************************************************************************/
/**
 *
 * This function takes care HPD event
 *
 *
 * @return	None.
 *
 * @note		None.
 *
 ******************************************************************************/
void DpTxSs_HpdEventHandler(void *InstancePtr)
{
	XDpTxSs *DpTxSsPtr = (XDpTxSs *)InstancePtr;

	if (XDpTxSs_IsConnected(DpTxSsPtr)) {
		DpTxSs_Sink_PowerDown();
		DpTxSs_Sink_PowerUp();
		TxReconnected++;
		xil_printf("Cable Connected\r\n");
	} else {
		xil_printf("Cable Disconnected\r\n");
		XDp_WriteReg(DpTxSsPtr->DpPtr->Config.BaseAddr, XDP_TX_ENABLE, 0x0);

		/*
		 * DpTxSs_DisableAudio
		 */
		XDp_WriteReg(DpTxSsPtr->DpPtr->Config.BaseAddr, XDP_TX_AUDIO_CONTROL, 0x0);

		TxReconnected = 0;

		/*
		 * Setting Vs and Pe to 0,0 on Cable disconnect
		 */
		DpTxSsPtr->DpPtr->TxInstance.LinkConfig.VsLevel = 0;
		DpTxSsPtr->DpPtr->TxInstance.LinkConfig.PeLevel = 0;

		DpTxSs_Pe_Vs_AdjustHandler(&VPhyInst);
	}
}

/*****************************************************************************/
/**
 *
 * This function takes care HPD pulse interrupt
 *
 *
 * @return	None.
 *
 * @note		None.
 *
 ******************************************************************************/
void DpTxSs_HpdPulseHandler(void *InstancePtr)
{
	XDpTxSs *DpTxSsPtr = (XDpTxSs *)InstancePtr;

	/*
	 * Some monitors give HPD pulse repeatedly which causes HPD pulse function to
	 * be executed huge number of times. Hence hpd_pulse interrupt is disabled
	 * and then enabled when hpd_pulse function is executed
	 */
	XDp_WriteReg(DpTxSsPtr->DpPtr->Config.BaseAddr, XDP_TX_INTERRUPT_MASK,
			0xFFFFF010);

	HpdPulseConEvent = 1;
	xil_printf("HPD Pulse..\r\n");
	DpTxSs_HpdPulseCon(DpTxSsPtr);
}

/*****************************************************************************/
/**
 *
 * This function is the main hpd pulse process.
 *
 *
 * @return	None.
 *
 * @note		None.
 *
 ******************************************************************************/

void DpTxSs_HpdPulseCon(XDpTxSs *InstancePtr)
{
	u8 lane0_sts = InstancePtr->UsrHpdPulseData.Lane0Sts;
	u8 lane2_sts = InstancePtr->UsrHpdPulseData.Lane2Sts;
	u8 laneAlignStatus = InstancePtr->UsrHpdPulseData.LaneAlignStatus;
	u8 bw_set = InstancePtr->UsrHpdPulseData.BwSet;
	u8 lane_set = InstancePtr->UsrHpdPulseData.LaneSet;
	u8 down_strm = InstancePtr->UsrHpdPulseData.AuxValues[4];
	u8 retrain_link = 0;

	lane_set = lane_set & 0x1F;
	bw_set = bw_set & 0x1F;
	laneAlignStatus = laneAlignStatus & 0x1;

	if(bw_set != XDP_TX_LINK_BW_SET_810GBPS
			&& bw_set != XDP_TX_LINK_BW_SET_162GBPS
			&& bw_set != XDP_TX_LINK_BW_SET_270GBPS
			&& bw_set != XDP_TX_LINK_BW_SET_540GBPS
			&& bw_set != XDP_TX_LINK_BW_SET_UHBR10
			&& bw_set != XDP_TX_LINK_BW_SET_UHBR20
			&& bw_set != XDP_TX_LINK_BW_SET_UHBR135) {
		bw_set = InstancePtr->DpPtr->Config.MaxLinkRate;
		retrain_link = 1;
	}

	if(lane_set != XDP_TX_LANE_COUNT_SET_1
			&& lane_set != XDP_TX_LANE_COUNT_SET_2
			&& lane_set != XDP_TX_LANE_COUNT_SET_4) {
		lane_set = InstancePtr->DpPtr->Config.MaxLaneCount;
		retrain_link = 1;
	}

	lane0_sts = lane0_sts & 0x77;
	lane2_sts = lane2_sts & 0x77;

	if (lane_set == XDP_TX_LANE_COUNT_SET_4) {
		if ((lane0_sts != (XDP_DPCD_STATUS_LANE_0_CR_DONE_MASK |
						XDP_DPCD_STATUS_LANE_0_CE_DONE_MASK |
						XDP_DPCD_STATUS_LANE_0_SL_DONE_MASK |
						XDP_DPCD_STATUS_LANE_1_CR_DONE_MASK |
						XDP_DPCD_STATUS_LANE_1_CE_DONE_MASK |
						XDP_DPCD_STATUS_LANE_1_SL_DONE_MASK)
		    ) || (lane2_sts != (XDP_DPCD_STATUS_LANE_2_CR_DONE_MASK |
				    XDP_DPCD_STATUS_LANE_2_CE_DONE_MASK |
				    XDP_DPCD_STATUS_LANE_2_SL_DONE_MASK |
				    XDP_DPCD_STATUS_LANE_3_CR_DONE_MASK |
				    XDP_DPCD_STATUS_LANE_3_CE_DONE_MASK |
				    XDP_DPCD_STATUS_LANE_3_SL_DONE_MASK)
			 ) || (laneAlignStatus != 1)) {
			retrain_link = 1;
		}
	} else if (lane_set == XDP_TX_LANE_COUNT_SET_2) {
		if ((lane0_sts != (XDP_DPCD_STATUS_LANE_0_CR_DONE_MASK |
						XDP_DPCD_STATUS_LANE_0_CE_DONE_MASK |
						XDP_DPCD_STATUS_LANE_0_SL_DONE_MASK |
						XDP_DPCD_STATUS_LANE_1_CR_DONE_MASK |
						XDP_DPCD_STATUS_LANE_1_CE_DONE_MASK |
						XDP_DPCD_STATUS_LANE_1_SL_DONE_MASK)
		    ) || (laneAlignStatus != 1)) {
			retrain_link = 1;
		}
	} else if (lane_set == XDP_TX_LANE_COUNT_SET_1) {
		lane0_sts = lane0_sts & 0x7;

		if ((lane0_sts != (XDP_DPCD_STATUS_LANE_0_CR_DONE_MASK |
						XDP_DPCD_STATUS_LANE_0_CE_DONE_MASK |
						XDP_DPCD_STATUS_LANE_0_SL_DONE_MASK)
		    ) || (laneAlignStatus != 1)) {
			retrain_link = 1;
		}
	}

	if ((down_strm & 0x40) == 0x40) {
		retrain_link = 1;
	}

	if(retrain_link == 1){
		XDpTxSs_SetLinkRate(&DpTxSsInst, bw_set);
		XDpTxSs_SetLaneCount(&DpTxSsInst, lane_set);
		XDpTxSs_Start(&DpTxSsInst);
	}

	XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr, XDP_TX_INTERRUPT_MASK, 0xFFFFF000);
}

/*****************************************************************************/
/**
 *
 * This function sets up DPTxSubsystem
 *
 * @param	LineRate
 * @param	LaneCount
 * @param	edid 1st block
 * @param	edid 2nd block
 *
 * @return	None.
 *
 * @note		None.
 *
 ******************************************************************************/
void DpTxSs_Setup(u8 *LineRate_init, u8 *LaneCount_init,
		u8 Edid_org[128], u8 Edid1_org[128])
{
	u8 Status;
	u8 connected = 0;

	XDp_ReadReg(DpTxSsInst.DpPtr->Config.BaseAddr, XDP_TX_INTERRUPT_STATUS);

	DpTxSsInst.DpPtr->TxInstance.TxSetMsaCallback = NULL;
	DpTxSsInst.DpPtr->TxInstance.TxMsaCallbackRef = NULL;

	/*
	 * This is an intentional infinite loop
	 */
	while (!XDpTxSs_IsConnected(&DpTxSsInst)) {
		if (connected == 0) {
			xil_printf("Please connect a DP Monitor to start the application !!!\r\n");
			connected = 1;
		}
	}

	/*
	 * Waking up the monitor
	 */
	DpTxSs_Sink_PowerCycle();

	/*
	 * Reading the first block of EDID
	 */
	if (XDpTxSs_IsConnected(&DpTxSsInst)) {
		XDp_TxGetEdidBlock(DpTxSsInst.DpPtr, Edid_org, 0);

		/*
		 * Reading the second block of EDID
		 */
		XDp_TxGetEdidBlock(DpTxSsInst.DpPtr, Edid1_org, 1);

		xil_printf("Reading EDID contents of the DP Monitor..\r\n");

		Status  = XDp_TxAuxRead(DpTxSsInst.DpPtr, XDP_DPCD_MAX_LINK_RATE,
				1, LineRate_init);
		Status |= XDp_TxAuxRead(DpTxSsInst.DpPtr, XDP_DPCD_MAX_LANE_COUNT,
				1, LaneCount_init);

		u8 rData = 0;

		/*
		 * Check the EXTENDED_RECEIVER_CAPABILITY_FIELD_PRESENT bit
		 */
		XDp_TxAuxRead(DpTxSsInst.DpPtr, XDP_DPCD_TRAIN_AUX_RD_INTERVAL, 1, &rData);

		if (rData & 0x80) { /**< if EXTENDED_RECEIVER_CAPABILITY_FIELD is enabled */
			XDp_TxAuxRead(DpTxSsInst.DpPtr, 0x2201, 1, &rData); /**< read maxLineRate */
			if (rData == XDP_DPCD_LINK_BW_SET_810GBPS) {
				*LineRate_init = 0x1E;
				xil_printf ("Capability is %x\r\n", *LineRate_init);
			}
		}

		if (Status != XST_SUCCESS) { /**< Give another chance to monitor. */
			/*
			 * Waking up the monitor
			 */
			DpTxSs_Sink_PowerCycle();

			XDp_TxGetEdidBlock(DpTxSsInst.DpPtr, Edid_org, 0);

			/*
			 * Reading the second block of EDID
			 */
			XDp_TxGetEdidBlock(DpTxSsInst.DpPtr, Edid1_org, 1);

			xil_printf("Reading EDID contents of the DP Monitor..\r\n");

			Status = XDp_TxAuxRead(DpTxSsInst.DpPtr, XDP_DPCD_MAX_LINK_RATE,
					1, LineRate_init);
			Status |= XDp_TxAuxRead(DpTxSsInst.DpPtr, XDP_DPCD_MAX_LANE_COUNT,
					1, LaneCount_init);
			/*
			 * Check the EXTENDED_RECEIVER_CAPABILITY_FIELD_PRESENT bit
			 */
			XDp_TxAuxRead(DpTxSsInst.DpPtr, XDP_DPCD_TRAIN_AUX_RD_INTERVAL, 1, &rData);

			if (rData & 0x80) { /**< if EXTENDED_RECEIVER_CAPABILITY_FIELD is enabled */
				XDp_TxAuxRead(DpTxSsInst.DpPtr, 0x2201, 1, &rData); /**< read maxLineRate */

				if (rData == XDP_DPCD_LINK_BW_SET_810GBPS) {
					*LineRate_init = 0x1E;
					xil_printf ("Monitor Capability is %x\r\n", *LineRate_init);
				}
			}

			if (Status != XST_SUCCESS)
				xil_printf ("Failed to read sink capabilities\r\n");
		}
	} else {
		xil_printf("Please connect a DP Monitor and try again !!!\r\n");
		return;
	}

	*LineRate_init &= 0xFF;
	*LaneCount_init &= 0xF;

	xil_printf("System capabilities set to: LineRate %x, LaneCount %x\r\n",
			*LineRate_init, *LaneCount_init);
#if ENABLE_AUDIO
	XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr, XDP_TX_AUDIO_CONTROL, 0x0);
#endif
}

/*****************************************************************************/
/**
 *
 * This function starts tx process
 *
 * @param	line rate
 * @param	lane counts
 * @param	pointer to resolution table
 * @param	bit per components
 * @param	video pattern to output
 *
 * @return
 *		- XST_SUCCESS if interrupt setup was successful.
 *		- A specific error code defined in "xstatus.h" if an error
 *		occurs.
 *
 * @note		None.
 *
 ******************************************************************************/
u32 DpTxSs_StartTx(u8 line_rate, u8 lane_count, user_config_struct user_config)
{
	XVidC_VideoMode res_table = user_config.VideoMode_local;
	u8 bpc = user_config.user_bpc;
	u8 pat = user_config.user_pattern;
	u8 format = user_config.user_format;
	u8 C_VideoUserStreamPattern[8] = {0x10, 0x11, 0x12, 0x13, 0x14,
		0x15, 0x16, 0x17}; /**< Duplicate */
	u32 Status;

	/*
	 * Stop the Patgen
	 */
	XDp_WriteReg(XPAR_TX_SUBSYSTEM_AV_PAT_GEN_0_BASEADDR, 0x0, 0x0);

	/*
	 * Waking up the monitor
	 */
	DpTxSs_Sink_PowerCycle();

	XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr, XDP_TX_ENABLE, 0x0);

	/*
	 * Give a bit of time for DP IP after monitor came up and starting link training
	 */
	usleep(100000);

	XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr, XDP_TX_ENABLE, 0x1);

	xil_printf("Training TX with: Link rate %x, Lane count %d\r\n",
			line_rate,lane_count);

	XDpTxSs_SetLinkRate(&DpTxSsInst, line_rate);

	xil_printf(".");

	XDpTxSs_SetLaneCount(&DpTxSsInst, lane_count);

	xil_printf(".");

	if (res_table != 0) {
		Status = XDpTxSs_SetVidMode(&DpTxSsInst, res_table);
		if (Status != XST_SUCCESS) {
			xil_printf("ERR: Setting resolution failed\r\n");
		}
		xil_printf(".");
	}

	if (bpc != 0) {
		Status = XDpTxSs_SetBpc(&DpTxSsInst, bpc);
		if (Status != XST_SUCCESS){
			xil_printf("ERR: Setting bpc to %d failed\r\n",bpc);
		}
		xil_printf(".");
	}

	/*
	 * Setting Color Format
	 * User can change coefficients here - By default 601 is used for YCbCr
	 */
	XDp_TxCfgSetColorEncode(DpTxSsInst.DpPtr, XDP_TX_STREAM_ID1, format,
			XVIDC_BT_601, XDP_DR_CEA);

	Status = XDpTxSs_Start(&DpTxSsInst);

	if (Status != XST_SUCCESS) {
		xil_printf("ERR:1DPTX SS start failed\r\n");
		return (XST_FAILURE);
	}

	xil_printf(".");

	/*
	 * Updates required timing values in Video Pattern Generator
	 */
	Vpg_StreamSrcConfigure(DpTxSsInst.DpPtr, 0, 1);

	xil_printf(".");

	/*
	 * Setting video pattern
	 */
	Vpg_VidgenSetUserPattern(DpTxSsInst.DpPtr, C_VideoUserStreamPattern[pat]);

	xil_printf(".");

	/*
	 * Generate the video clock using MMCM
	 */
	Gen_vid_clk(DpTxSsInst.DpPtr,(XDP_TX_STREAM_ID1));

	ClkWiz_Locked();

	if (DpTxSsInst.VtcPtr[0]) {
		Status = XDpTxSs_VtcSetup(DpTxSsInst.VtcPtr[0],
				&DpTxSsInst.DpPtr->TxInstance.MsaConfig[0],
				DpTxSsInst.UsrOpt.VtcAdjustBs);
		if (Status != XST_SUCCESS) {
			xdbg_printf(XDBG_DEBUG_GENERAL,"SS ERR: VTC %d setup failed!\n\r", Index);
		}
	}

	Status = XDpTxSs_CheckLinkStatus(&DpTxSsInst);
	if (Status != (XST_SUCCESS)) {
		Status = XDpTxSs_Start(&DpTxSsInst);
		if (Status != XST_SUCCESS) {
			xil_printf("ERR:DPTX SS start failed\r\n");
			return (XST_FAILURE);
		}
	}

	XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr, XDP_TX_INTERRUPT_MASK,
			0xFFFFF000);

	/*
	 * Initialize CRC
	 *
	 * Reset CRC
	 */
	XVidFrameCrc_Reset();

	/*
	 * Set Pixel width in CRC engine
	 */
	if (format != 2) {
		XDp_WriteReg(XPAR_TX_SUBSYSTEM_CRC_BASEADDR, VIDEO_FRAME_CRC_CONFIG, CRC_CFG);
	} else {
		XDp_WriteReg(XPAR_TX_SUBSYSTEM_CRC_BASEADDR, VIDEO_FRAME_CRC_CONFIG,
				CRC_CFG | 0x80000000);
	}

	xil_printf("..done !\r\n");

	int lr, lc = 0;

	/*
	 * Read Link rate over through channel
	 */
	XDp_TxAuxRead(DpTxSsInst.DpPtr, XDP_DPCD_LINK_BW_SET, 1, &lr);

	/*
	 * Read Lane count through AUX channel
	 */
	XDp_TxAuxRead(DpTxSsInst.DpPtr, XDP_DPCD_LANE_COUNT_SET, 1, &lc);

	xil_printf("Link trained at %x x %x\r\n", (lr & 0xFF),
			(lc & XDP_DPCD_LANE_COUNT_SET_MASK));

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
 *
 * This function to check MMCM lock status
 *
 * @param	None.
 *
 * @return	None.
 *
 * @note		None.
 *
 ******************************************************************************/
void ClkWiz_Locked(void)
{
	volatile u32 res;
	u32 timer = 0;

#ifndef SDT
	res = XDp_ReadReg(XPAR_GPIO_0_BASEADDR, 0x0);
#else
	res = XDp_ReadReg(XPAR_XGPIO_0_BASEADDR, 0x0);
#endif

	while (res == 0 && timer < 1000) {
		xil_printf("~/~/");
#ifndef SDT
		res = XDp_ReadReg(XPAR_GPIO_0_BASEADDR, 0x0);
#else
		res = XDp_ReadReg(XPAR_XGPIO_0_BASEADDR, 0x0);
#endif

		timer++;	/**< timer for timeout. No need to be specific time. As long as long enough to wait lock */
        }
        xil_printf("^^");
}

/*****************************************************************************/
/**
 *
 * This function is called when a Hot-Plug-Detect (HPD) event is received by
 * the DisplayPort TX Subsystem core.
 *
 * @param	InstancePtr is a pointer to the XDpTxSs instance.
 *
 * @return	None.
 *
 * @note		Use the XDpTxSs_SetCallback driver function to set this
 *		function as the handler for HPD event.
 *
 ******************************************************************************/
void DpTxSs_HpdCon(XDpTxSs *InstancePtr, u8 Edid_org[128], u16 ResUpdate)
{
	u32 Status = XST_SUCCESS;
	u8 Channel_Encoding = 0;
	u8 MaxCapNew = InstancePtr->UsrHpdEventData.MaxCapNew;
	u8 MaxCapLanesNew = InstancePtr->UsrHpdEventData.MaxCapLanesNew;
	u32 Hpd_HTotalTest;
	u32 Hpd_VTotalTest;
	u32 Hpd_FreqTest;
	u8 Hpd_GoodEdid = 1;
	XVidC_VideoMode Hpd_VmIdTest;
	XVidC_VideoMode VmId_ptm_hpd;
	u8 Hpd_Bpc;
	u8 Skip = 0;

	if ((XPAR_XVPHY_0_TRANSCEIVER_TYPE == XVPHY_GT_TYPE_GTHE4)) {
		/*
		 * ZCU102 does not support 20G
		 */
		Skip = 1;
	}

	/*
	 * read channel encoding if 128/132b supported
	 */
	XDp_TxAuxRead(DpTxSsInst.DpPtr, XDP_DPCD_ML_CH_CODING_CAP, 1, &Channel_Encoding);

	if ((Channel_Encoding & 0x2) == 0x2) { /**< supports 128/132b hence UHBR rates */
		XDp_TxAuxRead(DpTxSsInst.DpPtr, XDP_DPCD_128B_132B_SUPPORTED_LINK_RATE, 1,
				&MaxCapNew);

		if (((MaxCapNew & 0x2) == 0x2) && (Skip == 0)) {
			MaxCapNew = XDP_TX_LINK_BW_SET_UHBR20;
		} else if ((MaxCapNew & 0x4) == 0x4) {
			MaxCapNew = XDP_TX_LINK_BW_SET_UHBR135;
		} else if ((MaxCapNew & 0x1) == 0x1) {
			MaxCapNew = XDP_TX_LINK_BW_SET_UHBR10;
		}
	}

	user_config_struct user_config;

	XDp_ReadReg(DpTxSsInst.DpPtr->Config.BaseAddr,XDP_TX_INTERRUPT_STATUS);

	memcpy(Edid_org, InstancePtr->UsrHpdEventData.EdidOrg, 128);

	TxReconnected = 0;

	if (XVidC_EdidIsHeaderValid(Edid_org)) {
		Hpd_GoodEdid = 1;

		if (!CalculateChecksum(Edid_org, 128)) {
			Hpd_GoodEdid = 1;
		} else {
			Hpd_GoodEdid = 0;
			xil_printf ("Bad EDID Checksum\r\n");
		}
	} else {
		Hpd_GoodEdid = 0;
		xil_printf("Bad EDID Header\r\n");
	}

	/*
	 * Till here is the requirement per DP spec
	 */

	//////////////////////////////////////////////
	// From here is optional per application
	//////////////////////////////////////////////

	/*
	 * check if lane is 1/2/4 or something else
	 */
	if (MaxCapLanesNew != 1
			&& MaxCapLanesNew != 2
			&& MaxCapLanesNew != 4) {
		/*
		 * Something wrong. Read again
		 */
		XDp_TxAuxRead(DpTxSsInst.DpPtr, XDP_DPCD_MAX_LANE_COUNT, 1,
				&MaxCapLanesNew);
	}

	/*
	 * Check if line speed is either 0x6, 0xA, 0x14, 0x1E, 0x1, 0x4, 0x2
	 */
	if (MaxCapNew != XDP_TX_LINK_BW_SET_UHBR20
			&&MaxCapNew != XDP_TX_LINK_BW_SET_UHBR135
			&&MaxCapNew != XDP_TX_LINK_BW_SET_UHBR10
			&& MaxCapNew != XDP_TX_LINK_BW_SET_810GBPS
			&& MaxCapNew != XDP_TX_LINK_BW_SET_540GBPS
			&& MaxCapNew != XDP_TX_LINK_BW_SET_270GBPS
			&& MaxCapNew != XDP_TX_LINK_BW_SET_162GBPS) {
		/*
		 * read channel encoding if 128/132b supported
		 */
		XDp_TxAuxRead(DpTxSsInst.DpPtr, XDP_DPCD_ML_CH_CODING_CAP, 1, &Channel_Encoding);

		if ((Channel_Encoding & 0x2) == 0x2) { /**< supports 128/132b hence UHBR rates */
			XDp_TxAuxRead(DpTxSsInst.DpPtr, XDP_DPCD_128B_132B_SUPPORTED_LINK_RATE, 1, &MaxCapNew);

			if (((MaxCapNew & 0x2) == 0x2) && (Skip == 0)) {
				MaxCapNew = XDP_TX_LINK_BW_SET_UHBR20;
			} else if ((MaxCapNew & 0x4) == 0x4) {
				MaxCapNew = XDP_TX_LINK_BW_SET_UHBR135;
			} else if ((MaxCapNew & 0x1) == 0x1) {
				MaxCapNew = XDP_TX_LINK_BW_SET_UHBR10;
			}
		} else {
			/*
			 * something wrong. Read again
			 */
			XDp_TxAuxRead(DpTxSsInst.DpPtr, XDP_DPCD_MAX_LINK_RATE, 1, &MaxCapNew);
		}
	}

	if (MaxCapNew != XDP_TX_LINK_BW_SET_UHBR20
			&&MaxCapNew != XDP_TX_LINK_BW_SET_UHBR135
			&&MaxCapNew != XDP_TX_LINK_BW_SET_UHBR10
			&&MaxCapNew != XDP_TX_LINK_BW_SET_810GBPS
			&& MaxCapNew != XDP_TX_LINK_BW_SET_540GBPS
			&& MaxCapNew != XDP_TX_LINK_BW_SET_270GBPS
			&& MaxCapNew != XDP_TX_LINK_BW_SET_162GBPS) {
		/*
		 * Read channel encoding if 128/132b supported
		 */
		XDp_TxAuxRead(DpTxSsInst.DpPtr, XDP_DPCD_ML_CH_CODING_CAP, 1,
				&Channel_Encoding);
		if ((Channel_Encoding & 0x2) == 0x2) { /**< Supports 128/132b hence UHBR rates */
			XDp_TxAuxRead(DpTxSsInst.DpPtr, XDP_DPCD_128B_132B_SUPPORTED_LINK_RATE, 1,
					&MaxCapNew);
			if (((MaxCapNew & 0x2) == 0x2) && (Skip == 0)) {
				MaxCapNew = XDP_TX_LINK_BW_SET_UHBR20;
			} else if ((MaxCapNew & 0x4) == 0x4) {
				MaxCapNew = XDP_TX_LINK_BW_SET_UHBR135;
			} else if ((MaxCapNew & 0x1) == 0x1) {
				MaxCapNew = XDP_TX_LINK_BW_SET_UHBR10;
			}
		} else {
			MaxCapNew = XDP_TX_LINK_BW_SET_810GBPS;
		}
	}
	ResUpdate = XVIDC_VM_640x480_60_P;

	if (Hpd_GoodEdid == 1) {
		Hpd_HTotalTest = XVidC_EdidGetStdTimingsH(Edid_org, 1);
		Hpd_VTotalTest = XVidC_EdidGetStdTimingsV(Edid_org, 1);
		Hpd_FreqTest   = XVidC_EdidGetStdTimingsFrr(Edid_org, 1);
		Hpd_VmIdTest = XVidC_GetVideoModeId(Hpd_HTotalTest, Hpd_VTotalTest,
				Hpd_FreqTest,0);

		VmId_ptm_hpd = GetPreferredVm(Edid_org, MaxCapNew,
				MaxCapLanesNew & 0x1F);

		Hpd_Bpc = XVidC_EdidGetColorDepth(Edid_org);

		if (Hpd_Bpc > XPAR_TX_SUBSYSTEM_V_DP_TXSS2_0_BITS_PER_COLOR)
			Hpd_Bpc=XPAR_TX_SUBSYSTEM_V_DP_TXSS2_0_BITS_PER_COLOR;

		if (VmId_ptm_hpd == XVIDC_VM_NOT_SUPPORTED) { /**< Fail Safe mode */
			VmId_ptm_hpd = XVIDC_VM_640x480_60_P;
			Hpd_Bpc = 8;
		}
	} else {
		Hpd_VmIdTest = XVIDC_VM_NOT_SUPPORTED;
		VmId_ptm_hpd = XVIDC_VM_640x480_60_P;
		Hpd_Bpc = 8;
		Hpd_GoodEdid = 0;
	}

	if (MaxCapNew == XDP_TX_LINK_BW_SET_UHBR20
			||MaxCapNew == XDP_TX_LINK_BW_SET_UHBR135
			||MaxCapNew == XDP_TX_LINK_BW_SET_UHBR10
			||MaxCapNew == XDP_TX_LINK_BW_SET_810GBPS
			|| MaxCapNew == XDP_TX_LINK_BW_SET_540GBPS
			|| MaxCapNew == XDP_TX_LINK_BW_SET_270GBPS
			|| MaxCapNew == XDP_TX_LINK_BW_SET_162GBPS) {
		if (Hpd_GoodEdid == 1) {
			if ((VmId_ptm_hpd != XVIDC_VM_NOT_SUPPORTED)
					&& (Hpd_VmIdTest != XVIDC_VM_NOT_SUPPORTED)) {
				user_config.VideoMode_local = VmId_ptm_hpd;
			} else if ((VmId_ptm_hpd != XVIDC_VM_NOT_SUPPORTED)) {
				user_config.VideoMode_local = VmId_ptm_hpd;
			} else {
				user_config.VideoMode_local = Hpd_VmIdTest;
			}
		} else {
			user_config.VideoMode_local = ResUpdate;
		}

		user_config.user_bpc = Hpd_Bpc;
		user_config.VideoMode_local = VmId_ptm_hpd;
		user_config.user_pattern = 1;
		user_config.user_format = XVIDC_CSF_RGB;

		Status = DpTxSs_PhyConfig(MaxCapNew, VPHY_TX_PLL_TYPE, VPHY_TX_CHANNEL_TYPE);
		if (Status != XST_SUCCESS) {
			xil_printf("GT Configuration failed !!\r\n");
		}

		DpTxSs_StartTx(MaxCapNew, MaxCapLanesNew & 0x1F, user_config);
	}
}

/*****************************************************************************/
/**
 *
 * This function to calculate EDID checksum
 *
 * @param	edid data
 * @param	size of the edid data
 *
 * @return	checksum number
 *
 * @note		None.
 *
 ******************************************************************************/
static u8 CalculateChecksum(u8 *Data, u8 Size)
{
	u8 Index;
	u8 Sum = 0;

	for (Index = 0; Index < Size; Index++) {
		Sum += Data[Index];
	}

	return Sum;
}

/*****************************************************************************/
/**
 *
 * This function to find out preferred Video Mode ID
 *
 * @param	pointer to edid data
 * @param	maximum capability of line speed
 * @param	number of lane
 *
 * @return	Video Mode ID
 *
 * @note		None.
 *
 ******************************************************************************/
XVidC_VideoMode GetPreferredVm(u8 *EdidPtr, u8 cap, u8 lane)
{
	u8 *Ptm;
	u16 HBlank;
	u16 VBlank;
	u32 PixelClockHz;
	XVidC_FrameRate FrameRate;
	XVidC_VideoTiming Timing;
	XVidC_VideoMode VmId;
	u8 bpp;
	double pixel_freq, pixel_freq1 = 0;
	double max_freq[] = {216.0, 172.8, 360.0, 288.0, 720.0, 576.0, 1440, 1152};

	(void)memset((void *)&Timing, 0, sizeof(XVidC_VideoTiming));

	Ptm = &EdidPtr[XDP_EDID_PTM];

	bpp = XVidC_EdidGetColorDepth(EdidPtr) *3;

	HBlank = ((Ptm[XDP_EDID_DTD_HRES_HBLANK_U4] &
				XDP_EDID_DTD_XRES_XBLANK_U4_XBLANK_MASK) << 8) |
		Ptm[XDP_EDID_DTD_HBLANK_LSB];

	VBlank = ((Ptm[XDP_EDID_DTD_VRES_VBLANK_U4] &
				XDP_EDID_DTD_XRES_XBLANK_U4_XBLANK_MASK) << 8) |
		Ptm[XDP_EDID_DTD_VBLANK_LSB];

	Timing.HActive = (((Ptm[XDP_EDID_DTD_HRES_HBLANK_U4] &
					XDP_EDID_DTD_XRES_XBLANK_U4_XRES_MASK) >>
				XDP_EDID_DTD_XRES_XBLANK_U4_XRES_SHIFT) << 8) |
		Ptm[XDP_EDID_DTD_HRES_LSB];

	Timing.VActive = (((Ptm[XDP_EDID_DTD_VRES_VBLANK_U4] &
					XDP_EDID_DTD_XRES_XBLANK_U4_XRES_MASK) >>
				XDP_EDID_DTD_XRES_XBLANK_U4_XRES_SHIFT) << 8) |
		Ptm[XDP_EDID_DTD_VRES_LSB];

	PixelClockHz = (((Ptm[XDP_EDID_DTD_PIXEL_CLK_KHZ_MSB] << 8) |
				Ptm[XDP_EDID_DTD_PIXEL_CLK_KHZ_LSB]) * 10) * 1000;

	Timing.HFrontPorch = (((Ptm[XDP_EDID_DTD_XFPORCH_XSPW_U2] &
					XDP_EDID_DTD_XFPORCH_XSPW_U2_HFPORCH_MASK) >>
				XDP_EDID_DTD_XFPORCH_XSPW_U2_HFPORCH_SHIFT) << 8) |
		Ptm[XDP_EDID_DTD_HFPORCH_LSB];

	Timing.HSyncWidth = (((Ptm[XDP_EDID_DTD_XFPORCH_XSPW_U2] &
					XDP_EDID_DTD_XFPORCH_XSPW_U2_HSPW_MASK) >>
				XDP_EDID_DTD_XFPORCH_XSPW_U2_HSPW_SHIFT) << 8) |
		Ptm[XDP_EDID_DTD_HSPW_LSB];

	Timing.F0PVFrontPorch = (((Ptm[XDP_EDID_DTD_XFPORCH_XSPW_U2] &
					XDP_EDID_DTD_XFPORCH_XSPW_U2_VFPORCH_MASK) >>
				XDP_EDID_DTD_XFPORCH_XSPW_U2_VFPORCH_SHIFT) << 8) |
		((Ptm[XDP_EDID_DTD_VFPORCH_VSPW_L4] &
		  XDP_EDID_DTD_VFPORCH_VSPW_L4_VFPORCH_MASK) >>
		 XDP_EDID_DTD_VFPORCH_VSPW_L4_VFPORCH_SHIFT);

	Timing.F0PVSyncWidth = ((Ptm[XDP_EDID_DTD_XFPORCH_XSPW_U2] &
				XDP_EDID_DTD_XFPORCH_XSPW_U2_VSPW_MASK) << 8) |
		(Ptm[XDP_EDID_DTD_VFPORCH_VSPW_L4] &
		 XDP_EDID_DTD_VFPORCH_VSPW_L4_VSPW_MASK);

	/*
	 * Compute video mode timing values.
	 */
	Timing.HBackPorch = HBlank - (Timing.HFrontPorch + Timing.HSyncWidth);
	Timing.F0PVBackPorch = VBlank - (Timing.F0PVFrontPorch + Timing.F0PVSyncWidth);
	Timing.HTotal = (Timing.HSyncWidth + Timing.HFrontPorch + Timing.HActive +
			Timing.HBackPorch);
	Timing.F0PVTotal = (Timing.F0PVSyncWidth + Timing.F0PVFrontPorch +
			Timing.VActive + Timing.F0PVBackPorch);
	FrameRate = PixelClockHz / (Timing.HTotal * Timing.F0PVTotal);

	/*
	 * Few monitors returns 59 HZ. Hence, setting to 60.
	 */
	if (FrameRate == 59)
		FrameRate = XVIDC_FR_60HZ;

	pixel_freq = (FrameRate * Timing.HTotal * Timing.F0PVTotal) / 1000000.0;

	switch (cap) {
		case XDP_TX_LINK_BW_SET_162GBPS:
			if (bpp == 24) {
				pixel_freq1 = max_freq[0];
			} else {
				pixel_freq1 = max_freq[1];
			}
			break;
		case XDP_TX_LINK_BW_SET_270GBPS:
			if (bpp == 24) {
				pixel_freq1 = max_freq[2];
			} else {
				pixel_freq1 = max_freq[3];
			}
			break;
		case XDP_TX_LINK_BW_SET_540GBPS:
			if (bpp == 24) {
				pixel_freq1 = max_freq[4];
			} else {
				pixel_freq1 = max_freq[5];
			}
			break;
		case XDP_TX_LINK_BW_SET_810GBPS:
			if (bpp == 24) {
				pixel_freq1 = max_freq[6];
			} else {
				pixel_freq1 = max_freq[7];
			}
			break;
	}

	switch (lane) {
		case 0x1:
			pixel_freq1 = pixel_freq1/4.0;
			break;

		case 0x2:
			pixel_freq1 = pixel_freq1/2.0;
			break;

		case 0x4:
			pixel_freq1 = pixel_freq1;
			break;

		default:
			break;
	}

	/*
	 * There is no good way to find the timing table
	 * The following function would return the video timing table from our library
	 * only if the parameters match
	 * however some monitors may work with different timing params
	 * as such the video timing returned from the library may not work.
	 */

	if (pixel_freq1 < pixel_freq) {
		VmId = XVIDC_VM_NOT_SUPPORTED;
	} else {
		/*
		 * Get video mode ID
		 */
		if (HBlank == 80 || HBlank == 160) {
			VmId = 	XVidC_GetVideoModeIdRb(Timing.HActive, Timing.VActive,
					FrameRate, XVidC_EdidIsDtdPtmInterlaced(EdidPtr), (160/HBlank));
		} else {
			VmId = XVidC_GetVideoModeId(Timing.HActive, Timing.VActive,
					FrameRate, XVidC_EdidIsDtdPtmInterlaced(EdidPtr));
		}
	}

	return VmId;
}

/*****************************************************************************/
/**
 *
 * This function powers down sink
 *
 * @param	None.
 *
 * @return	None.
 *
 * @note		None.
 *
 ******************************************************************************/
void DpTxSs_Sink_PowerDown(void)
{
	u8 Data[8];

	Data[0] = 0x2;
	XDp_TxAuxWrite(DpTxSsInst.DpPtr,
			XDP_DPCD_SET_POWER_DP_PWR_VOLTAGE, 1, Data);
}

/*****************************************************************************/
/**
 *
 * This function powers up sink
 *
 * @param	None.
 *
 * @return	None.
 *
 * @note		None.
 *
 ******************************************************************************/
void DpTxSs_Sink_PowerUp(void)
{
	u8 Data[8];

	Data[0] = 0x1;
	XDp_TxAuxWrite(DpTxSsInst.DpPtr,
			XDP_DPCD_SET_POWER_DP_PWR_VOLTAGE, 1, Data);
}

/*****************************************************************************/
/**
 *
 * This function returns current line rate
 *
 * @param	None.
 *
 * @return	None.
 *
 * @note		None.
 *
 ******************************************************************************/
u8 get_LineRate(void)
{
	return XDpTxSs_GetLinkRate(&DpTxSsInst);
}

/*****************************************************************************/
/**
 *
 * This function returns current lane counts
 *
 * @param	None.
 *
 * @return	None.
 *
 * @note		None.
 *
 ******************************************************************************/
u8 get_Lanecounts(void)
{
	return DpTxSsInst.DpPtr->TxInstance.LinkConfig.LaneCount;
}

/*****************************************************************************/
/**
 *
 * This function sets VPHY based on the linerate
 *
 * @param	LineRate_init_tx - Linerate
 * 			LaneCount_init_tx - Lanecount
 *
 * @return	Status.
 *
 * @note		None.
 *
 ******************************************************************************/
u32 DpTxSs_PhyConfig(int LineRate_init_tx, XVphy_PllType Tx_Pll, XVphy_ChannelId Tx_Channel)
{
	u32 Status=XST_SUCCESS;

	if (XPAR_XVPHY_0_TRANSCEIVER_TYPE == XVPHY_GT_TYPE_GTHE4) {
		if (LineRate_init_tx == XDP_TX_LINK_BW_SET_UHBR20) {
			xil_printf ("GTHE4 is not capable of running at 20G..skipping\r\n");
			Status = XST_FAILURE;
			return Status;
		}
	}

	XVphy_PllRefClkSelType Refclk;

	if ((LineRate_init_tx == XDP_TX_LINK_BW_SET_810GBPS) ||
			(LineRate_init_tx == XDP_TX_LINK_BW_SET_540GBPS) ||
			(LineRate_init_tx == XDP_TX_LINK_BW_SET_270GBPS) ||
			(LineRate_init_tx == XDP_TX_LINK_BW_SET_162GBPS)) {
		Refclk = VPHY_TX_REFCLK_SEL_270;
	} else {
		Refclk = VPHY_TX_REFCLK_SEL_400;
	}

	/*
	 * Reconfiguring the PHY based on the line rate requested
	 */
	PLLRefClkSel(&VPhyInst, LineRate_init_tx, Tx_Channel);

	XVphy_SetupDP21Phy(&VPhyInst, 0, Tx_Channel,
			XVPHY_DIR_TX, LineRate_init_tx, Refclk,
			Tx_Pll);

	Status = XVphy_DP21PhyReset(&VPhyInst, 0, Tx_Channel, XVPHY_DIR_TX);
	if (Status != XST_SUCCESS) {
		xil_printf("+++ TX GT configuration encountered a failure config +++\r\n");
	}
	return Status;
}

/*****************************************************************************/
/**
 *
 * This function power cycle the sink
 *
 * @param	user_config_struct.
 *
 * @return	Status.
 *
 * @note		None.
 *
 ******************************************************************************/
void DpTxSs_Sink_PowerCycle(void)
{
	/*
	 * Waking up the monitor
	 */
	DpTxSs_Sink_PowerDown();

	/*
	 * Give enough time for monitor to power down
	 */
	usleep(400);
	DpTxSs_Sink_PowerUp();

	/*
	 * Give enough time for monitor to wake up
	 */
	usleep(30000);
	DpTxSs_Sink_PowerUp(); /**< Monitor to wake up once again */
	usleep(4000);
}

int VideoFMC_Init(void)
{
	u8 Buffer[2];
	int ByteCount= 0;

	xil_printf("VFMC: Setting IO Expanders...\n\r");
	XIic_Reset(&IicInstance);

	/*
	 * Set the I2C Mux to select the HPC FMC
	 */
#if !defined (PLATFORM_MB) && !defined (__riscv)
	Buffer[0] = 0x01; /**< Enable HPC0 */
#else
	Buffer[0] = 0x02; /**< Enable HPC0 */
#endif
	ByteCount = XIic_Send(IIC_BASE_ADDR, I2C_MUX_ADDR, (u8*)Buffer, 1, XIIC_STOP);
	if (ByteCount != 1) {
		xil_printf("Failed to set the I2C Mux.\n\r");
		return XST_FAILURE;
	}

#if 0
	/*
	 * Required for previous version of FMC cards.
	 */
	u32 Freq;

	Freq = i2c_read_freq(IIC_BASE_ADDR, 0x4D, 0x0);
	xil_printf("Freq lock = %x\r\n", Freq);
#endif

	(void)i2c_read_tdp2004(IIC_BASE_ADDR, 0x18, 0xF0);
	(void)i2c_read_tdp2004(IIC_BASE_ADDR, 0x18, 0xF1);

	/* Setting the gain to 2.6 dB */
	i2c_write_tdp2004(IIC_BASE_ADDR, 0x18, 0x83, 0x7);
	i2c_write_tdp2004(IIC_BASE_ADDR, 0x18, 0x84, 0x4);

	return XST_SUCCESS;
}

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

		if (BytesRecvd == 0) {
			continue;
		}
		xil_printf("Found device: 0x%02x\n\r", i);
	}
	print("\n\r");
}

int i2c_write_freq(u32 I2CBaseAddress, u8 I2CSlaveAddress, u8 RegisterAddress, u32 Value)
{
	u32 Status;
	u32 ByteCount = 0;
	u8 Buffer[4];
	u8 Retry = 0;

	/*
	 *  Write data
	 */
	Buffer[0] = RegisterAddress;
	Buffer[1] = (Value & 0x000000FF);
	Buffer[2] = (Value & 0x0000FF00) >> 8;
	Buffer[3] = (Value & 0x00FF0000) >> 16;
	Buffer[4] = (Value & 0xFF000000) >> 24;

	xil_printf ("%x, %x, %x, %x\r\n",Buffer[1], Buffer[2], Buffer[3], Buffer[4]);

	while (1) {
#ifndef versal
		ByteCount = XIic_Send(I2CBaseAddress, I2CSlaveAddress, (u8*)Buffer, 5, XIIC_STOP);

		if (ByteCount == 5)
			Status=XST_SUCCESS;
		else
			Status=XST_FAILURE;
#else
		Status = XIicPs_MasterSendPolled(&Ps_Iic1, (u8 *)&Buffer, 2, I2CSlaveAddress);
#endif
		if (Status != XST_SUCCESS) {
			Retry++;

			/*
			 * Maximum retries
			 */
			if (Retry == 255) {
				return XST_FAILURE;
			}
		} else {
			return XST_SUCCESS;
		}
	}
}

u8 i2c_read_freq(u32 I2CBaseAddress, u8 I2CSlaveAddress, u16 RegisterAddress)
{
	u32 ByteCount = 0;
	u8 Buffer[1];
	u8 Data;
	u8 Retry = 0;
	u8 Exit;

	Exit = FALSE;
	Data = 0;

	do {
		/*
		 *  Set Address
		 */
		Buffer[0] = RegisterAddress & 0xff;
		ByteCount = XIic_Send(I2CBaseAddress, I2CSlaveAddress, (u8*)Buffer, 1, XIIC_REPEATED_START);

		if (ByteCount != 1) {
			Retry++;

			/*
			 * Maximum retries
			 */
			if (Retry == 255) {
				Exit = TRUE;
			}
		}

		/*
		 * Read data
		 */
		else {
			/*
			 * Read data
			 */
			ByteCount = XIic_Recv(I2CBaseAddress, I2CSlaveAddress, (u8*)Buffer, 1, XIIC_STOP);

			Data = Buffer[0];
			Exit = TRUE;
		}
	} while (!Exit);

	return Data;
}

int i2c_write_tdp2004(u32 I2CBaseAddress, u8 I2CSlaveAddress, u8 RegisterAddress, u8 Value)
{
	u32 Status;
	u32 ByteCount = 0;
	u8 Buffer[1];
	u8 Retry = 0;

	/*
	 * Write data
	 */
	Buffer[0] = RegisterAddress;
	Buffer[1] = Value;

	while (1) {
#ifndef versal
		ByteCount = XIic_Send(I2CBaseAddress, I2CSlaveAddress, (u8*)Buffer, 2, XIIC_STOP);
		if (ByteCount == 2) {
			Status=XST_SUCCESS;
		}
		else{
			Status=XST_FAILURE;
		}
#else
		Status = XIicPs_MasterSendPolled(&Ps_Iic1, (u8 *)&Buffer, 2,
				I2CSlaveAddress);
#endif
		if (Status != XST_SUCCESS) {
			Retry++;
			/*
			 * Maximum retries
			 */
			if (Retry == 255) {
				return XST_FAILURE;
			}
		}
		else {
			return XST_SUCCESS;
		}
	}
}

u8 i2c_read_tdp2004(u32 I2CBaseAddress, u8 I2CSlaveAddress, u16 RegisterAddress)
{
	u32 ByteCount = 0;
	u8 Buffer[1];
	u8 Data;
	u8 Retry = 0;
	u8 Exit;

	Exit = FALSE;
	Data = 0;

	do {
		/*
		 * Set Address
		 */
		Buffer[0] = RegisterAddress & 0xff;
		ByteCount = XIic_Send(I2CBaseAddress, I2CSlaveAddress, (u8*)Buffer, 1, XIIC_REPEATED_START);

		if (ByteCount != 1) {
			Retry++;

			/*
			 * Maximum retries
			 */
			if (Retry == 255) {
				Exit = TRUE;
			}
		}

		/*
		 * Read data
		 */
		else {
			/*
			 * Read data
			 */
			ByteCount = XIic_Recv(I2CBaseAddress, I2CSlaveAddress, (u8*)Buffer, 1, XIIC_STOP);

			Data = Buffer[0];
			Exit = TRUE;
		}
	} while (!Exit);

	return Data;
}
