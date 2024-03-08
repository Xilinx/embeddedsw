/******************************************************************************
* Copyright (C) 2020 - 2022 Xilinx, Inc. All rights reserved.
* Copyright 2023-2024 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver  Who      Date      Changes
* ---- ---      --------  --------------------------------------------------.
* 1.00  ND      18/10/22  Common DP 2.1 tx only application for zcu102 and
* 						  vcu118
* 1.01	ND		26/02/24  Added support for 13.5 and 20G
* </pre>
*
******************************************************************************/
#include "xdptxss_dp21_tx.h"
#include "xvidframe_crc.h"
#include "xil_cache.h"
#ifdef SDT
#include "xinterrupt_wrap.h"
#endif
XTmrCtr TmrCtr; /* Timer instance.*/
XIic_Config *ConfigPtr_IIC;     /* Pointer to configuration data */
XVphy VPhyInst;	/* The DPRX Subsystem instance.*/
XIic IicInstance;	/* I2C bus for Si570 */
extern volatile u8 hpd_pulse_con_event;
volatile u8 prev_line_rate; /*This previous line rate to keep previous info to compare
						with new line rate request*/

#ifndef PLATFORM_MB
XScuGic IntcInst;
#else
XIntc IntcInst;
#endif

#define INTRNAME_DPTX   0

extern XDpTxSs DpTxSsInst;	/* The DPTX Subsystem instance.*/
extern XTmrCtr TmrCtr; /* Timer instance.*/

#ifdef PLATFORM_MB
XIic  IicPtr;
typedef u8 AddressType;
#else
XIicPs Ps_Iic0, Ps_Iic1;
XIicPs_Config *XIic0Ps_ConfigPtr;
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

/* Interrupt helper functions */
void DpPt_HpdEventHandler(void *InstancePtr);
void DpPt_HpdPulseHandler(void *InstancePtr);
void DpPt_LinkrateChgHandler (void *InstancePtr);
void DpPt_pe_vs_adjustHandler(void *InstancePtr);
void DpPt_ffe_adjustHandler(void *InstancePtr);
void DpPt_ExtPkt_Handler(void *InstancePtr);
void DpPt_Vblank_Handler(void *InstancePtr);

u32 DpTxSs_SetupIntrSystem(void);
#ifndef SDT
u32 DpTxSs_PhyInit(u16 DeviceId);
#else
u32 DpTxSs_PhyInit(UINTPTR BaseAddress);
#endif
void DpPt_CustomWaitUs(void *InstancePtr, u32 MicroSeconds);
u32 DpTxSubsystem_Start(XDpTxSs *InstancePtr, int with_msa);
void DpTxSs_Setup(u8 *LineRate_init, u8 *LaneCount_init,
										u8 Edid_org[128], u8 Edid1_org[128]);
void PLLRefClkSel (XVphy *InstancePtr, u8 link_rate);
void clk_wiz_locked(void);
void hpd_pulse_con(XDpTxSs *InstancePtr);
extern void Gen_vid_clk(XDp *InstancePtr, u8 Stream);
int Vpg_StreamSrcConfigure(XDp *InstancePtr, u8 VSplitMode, u8 first_time);
void Vpg_VidgenSetUserPattern(XDp *InstancePtr, u8 Pattern);
static u8 CalculateChecksum(u8 *Data, u8 Size);
XVidC_VideoMode GetPreferredVm(u8 *EdidPtr, u8 cap, u8 lane);
void ReportVideoCRC(void);
extern void main_loop(void);
int set_phy = 0;
volatile u8 tx_is_reconnected = 0;
Video_CRC_Config VidFrameCRC; /* Video Frame CRC instance */

/************************** Variable Definitions *****************************/
#define XVPHY_DP_LINK_RATE_HZ_1000GBPS	10000000000LL
#define XVPHY_DP_LINK_RATE_HZ_1350GBPS	13500000000LL
#define XVPHY_DP_LINK_RATE_HZ_2000GBPS	20000000000LL

static XVphy_User_Config PHY_User_Config_Table[] =
{
  // Index,         TxPLL,               RxPLL,
 //	TxChId,         RxChId,
// LineRate,              LineRateHz,
// QPLLRefClkSrc,          CPLLRefClkSrc,    QPLLRefClkFreqHz,CPLLRefClkFreqHz
  {   0,     XVPHY_PLL_TYPE_CPLL,   XVPHY_PLL_TYPE_CPLL,
		  XVPHY_CHANNEL_ID_CHA,     XVPHY_CHANNEL_ID_CHA,
		  0x06,    XVPHY_DP_LINK_RATE_HZ_162GBPS,
		  ONBOARD_REF_CLK, ONBOARD_REF_CLK,     270000000,270000000},
  {   1,     XVPHY_PLL_TYPE_CPLL,   XVPHY_PLL_TYPE_CPLL,
		  XVPHY_CHANNEL_ID_CHA,     XVPHY_CHANNEL_ID_CHA,
		  0x0A,    XVPHY_DP_LINK_RATE_HZ_270GBPS,
		  ONBOARD_REF_CLK, ONBOARD_REF_CLK,     270000000,270000000},
  {   2,     XVPHY_PLL_TYPE_CPLL,   XVPHY_PLL_TYPE_CPLL,
		  XVPHY_CHANNEL_ID_CHA,     XVPHY_CHANNEL_ID_CHA,
		  0x14,    XVPHY_DP_LINK_RATE_HZ_540GBPS,
		  ONBOARD_REF_CLK, ONBOARD_REF_CLK,     270000000,270000000},
  {   3,     XVPHY_PLL_TYPE_QPLL0,  XVPHY_PLL_TYPE_CPLL,
		  XVPHY_CHANNEL_ID_CMN0,    XVPHY_CHANNEL_ID_CHA,
		  0x06,    XVPHY_DP_LINK_RATE_HZ_162GBPS,
		  ONBOARD_REF_CLK,        ONBOARD_REF_CLK,     270000000,270000000},
  {   4,     XVPHY_PLL_TYPE_QPLL0,  XVPHY_PLL_TYPE_CPLL,
		  XVPHY_CHANNEL_ID_CMN0,    XVPHY_CHANNEL_ID_CHA,
		  0x0A,    XVPHY_DP_LINK_RATE_HZ_270GBPS,
		  ONBOARD_REF_CLK,        ONBOARD_REF_CLK,     270000000,270000000},
  {   5,     XVPHY_PLL_TYPE_QPLL0,  XVPHY_PLL_TYPE_CPLL,
		  XVPHY_CHANNEL_ID_CMN0,    XVPHY_CHANNEL_ID_CHA,
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
		  0x1E,    XVPHY_DP_LINK_RATE_HZ_810GBPS,
		  ONBOARD_REF_CLK,        ONBOARD_REF_CLK,         270000000,270000000},
  {   10,     XVPHY_PLL_TYPE_QPLL0,  XVPHY_PLL_TYPE_CPLL,
		  XVPHY_CHANNEL_ID_CMN0,    XVPHY_CHANNEL_ID_CHA,
		  0x1E,    XVPHY_DP_LINK_RATE_HZ_810GBPS,
		  ONBOARD_REF_CLK,        ONBOARD_REF_CLK,     		270000000,270000000},
  {   11,     XVPHY_PLL_TYPE_CPLL,  XVPHY_PLL_TYPE_CPLL,
		  XVPHY_CHANNEL_ID_CHA,    XVPHY_CHANNEL_ID_CHA,
		  0x01,    XVPHY_DP_LINK_RATE_HZ_1000GBPS,
		  ONBOARD_400_CLK,        ONBOARD_400_CLK,     400000000,400000000},
  {   12,     XVPHY_PLL_TYPE_QPLL0,  XVPHY_PLL_TYPE_CPLL,
		  XVPHY_CHANNEL_ID_CMN0,    XVPHY_CHANNEL_ID_CHA,
		  0x01,    XVPHY_DP_LINK_RATE_HZ_1000GBPS,
		  ONBOARD_400_CLK,        ONBOARD_400_CLK,     400000000,400000000},
  {   13,     XVPHY_PLL_TYPE_QPLL0,  XVPHY_PLL_TYPE_CPLL,
		  XVPHY_CHANNEL_ID_CMN0,    XVPHY_CHANNEL_ID_CHA,
		  0x04,    XVPHY_DP_LINK_RATE_HZ_1350GBPS,
		  ONBOARD_400_CLK,        ONBOARD_400_CLK,     400000000,400000000},
  {   14,     XVPHY_PLL_TYPE_QPLL0,  XVPHY_PLL_TYPE_CPLL,
		  XVPHY_CHANNEL_ID_CMN0,    XVPHY_CHANNEL_ID_CHA,
		  0x04,    XVPHY_DP_LINK_RATE_HZ_1350GBPS,
		  ONBOARD_400_CLK,        ONBOARD_400_CLK,     400000000,400000000},
#if !defined (XPS_BOARD_ZCU102)
  {   15,     XVPHY_PLL_TYPE_QPLL0,  XVPHY_PLL_TYPE_CPLL,
		  XVPHY_CHANNEL_ID_CMN0,    XVPHY_CHANNEL_ID_CHA,
		  0x02,    XVPHY_DP_LINK_RATE_HZ_2000GBPS,
		  ONBOARD_400_CLK,        ONBOARD_400_CLK,     400000000,400000000},
  {   16,     XVPHY_PLL_TYPE_QPLL0,  XVPHY_PLL_TYPE_CPLL,
		  XVPHY_CHANNEL_ID_CMN0,    XVPHY_CHANNEL_ID_CHA,
		  0x02,    XVPHY_DP_LINK_RATE_HZ_2000GBPS,
		  ONBOARD_400_CLK,        ONBOARD_400_CLK,     400000000,400000000},
#endif

};

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
	xil_printf("(c) 2024 by AMD\r\n");
	xil_printf("-------------------------------------------\r\n\r\n");
#ifndef SDT
	Status = DpTxSs_Main(XDPTXSS_DEVICE_ID);
#else
	Status = DpTxSs_Main(XPAR_DPTXSS_0_BASEADDR);
#endif
	if (Status != XST_SUCCESS) {
		xil_printf("DisplayPort TX Subsystem interrupt example failed.");
		return XST_FAILURE;
	}

	xil_printf(
			"Successfully ran DisplayPort TX Subsystem interrupt example\r\n");

	disable_caches();

	return XST_SUCCESS;
}

#ifndef PLATFORM_MB
int I2cMux_Ps(u8 Data)
{
        u8 Buffer;
        int Status;

        Buffer = Data;
        Status = XIicPs_MasterSendPolled(&Ps_Iic0,
                                         (u8 *)&Buffer,
                                         1,
                                         I2C_MUX_ADDR2);

        if (Status != XST_SUCCESS) {
		xil_printf ("mux failed\r\n");
        }


        return Status;
}
#endif

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
	char UserInput;
	char CommandKey_m;
	char CmdKey_m[2];
	unsigned int Command_m;
//	u8 UserInput;
	u32 ReadVal=0;
	u16 DrpVal;
	u32 dptx_sts = 0;
	u32 retval = 0;

	user_config_struct user_config;
	user_config.user_bpc = 8;
	user_config.VideoMode_local = XVIDC_VM_800x600_60_P;
	user_config.user_pattern = 1; /*Color Ramp (Default)*/
	user_config.user_format = XVIDC_CSF_RGB;
	set_phy = 0;

	/* Do platform initialization in this function. This is hardware
	 * system specific. It is up to the user to implement this function.
	 */
	xil_printf("PlatformInit\r\n");
	Status = DpTxSs_PlatformInit();
	if (Status != XST_SUCCESS) {
		xil_printf("Platform init failed!\r\n");
	}
	xil_printf("Platform initialization done.\r\n");
	VideoFMC_Init();

	IDT_8T49N24x_SetClock(IIC_BASE_ADDR, I2C_IDT8N49_ADDR, 0,270000000, TRUE);

	/* Do platform initialization in this function. This is hardware
	 * system specific. It is up to the user to implement this function.
	 */
	/* Obtain the device configuration for the DisplayPort TX Subsystem */
#ifndef SDT
	ConfigPtr = XDpTxSs_LookupConfig(DeviceId);
#else
	ConfigPtr = XDpTxSs_LookupConfig(BaseAddress);
#endif
	if (!ConfigPtr) {
		return XST_FAILURE;
	}

	/* Copy the device configuration into the DpTxSsInst's Config
	 * structure. */
	Status = XDpTxSs_CfgInitialize(&DpTxSsInst, ConfigPtr,
					ConfigPtr->BaseAddress);
	if (Status != XST_SUCCESS) {
		xil_printf("DPTXSS config initialization failed.\r\n");
		return XST_FAILURE;
	}

	/* Check for SST/MST support */
	if (DpTxSsInst.UsrOpt.MstSupport) {
		xil_printf("\r\nINFO:DPTXSS is MST enabled. DPTXSS can be "
			"switched to SST/MST\r\n\r\n");
	}
	else {
		xil_printf("\r\nINFO:DPTXSS is  SST enabled. DPTXSS works "
			"only in SST mode.\r\n\r\n");
	}


	Status = DpTxSs_SetupIntrSystem();
	if (Status != XST_SUCCESS) {
		xil_printf("ERR: Interrupt system setup failed.\r\n");
		return XST_FAILURE;
	}

	/* Setup Video Phy, left to the user for implementation */
#ifndef SDT
	DpTxSs_PhyInit(XVPHY_DEVICE_ID);
#else
	DpTxSs_PhyInit(XPAR_XVPHY_0_BASEADDR);
#endif
	DpTxSs_Setup(&LineRate_init, &LaneCount_init, Edid_org, Edid1_org);

	// check if monitor is connected or not
	// This is intentional infinite while loop
	if (set_phy == 0) {
    while (!XDpTxSs_IsConnected(&DpTxSsInst)) {
		if (connected == 0) {
			xil_printf(
				"Please connect a DP Monitor to start the application !!!\r\n");
			connected = 1;
		}
    }
	}
	//Waking up the monitor
    sink_power_cycle();

	/* Do not return in order to allow interrupt handling to run. HPD events
	 * (connect, disconnect, and pulse) will be detected and handled.
	 */
	DpTxSsInst.DpPtr->TxInstance.TxSetMsaCallback = NULL;
	DpTxSsInst.DpPtr->TxInstance.TxMsaCallbackRef = NULL;
	DpTxSsInst.DpPtr->TxInstance.MsaConfig[0].ComponentFormat = 0x0;

	XVphy_BufgGtReset(&VPhyInst, XVPHY_DIR_TX,(FALSE));
	// This configures the vid_phy for line rate to start with
	//Even though CPLL can be used in limited case,
	//using QPLL is recommended for more coverage.
	Status = config_phy(LineRate_init_tx);
	LaneCount_init_tx = LaneCount_init_tx & 0x7;
	XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr,
											XDP_TX_INTERRUPT_MASK, 0xFFF);
	XDpTxSs_SetLinkRate(&DpTxSsInst, 0x1E);
    XDpTxSs_SetLaneCount(&DpTxSsInst, 0x4);
	start_tx (LineRate_init_tx, LaneCount_init_tx,user_config);

	while (1) { // for menu loop
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
	// Initialize timer.
	Status = XTmrCtr_Initialize(&TmrCtr, XPAR_TMRCTR_0_DEVICE_ID);
	if (Status != XST_SUCCESS){
		xil_printf("ERR:Timer failed to initialize. \r\n");
		return XST_FAILURE;
	}
	// Set up timer options.
	XTmrCtr_SetResetValue(&TmrCtr, XTC_TIMER_0, TIMER_RESET_VALUE);
	XTmrCtr_Start(&TmrCtr, XTC_TIMER_0);
#else
	// Initialize timer.
	Status = XTmrCtr_Initialize(&TmrCtr, XPAR_XTMRCTR_0_BASEADDR);
	if (Status != XST_SUCCESS) {
		xil_printf("ERR:Timer failed to initialize. \r\n");
		return XST_FAILURE;
	}
	// Set up timer options.
	XTmrCtr_SetResetValue(&TmrCtr, XTC_TIMER_0, TIMER_RESET_VALUE);
	XTmrCtr_Start(&TmrCtr, XTC_TIMER_0);
#endif
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

	    I2cMux_Ps(0x04);
#endif

    /*
     * Initialize the IIC driver so that it is ready to use.
     */
#ifndef SDT
    ConfigPtr_IIC = XIic_LookupConfig(IIC_DEVICE_ID);
    if (ConfigPtr_IIC == NULL) {
            return XST_FAILURE;
    }
#else
	ConfigPtr_IIC = XIic_LookupConfig(XPAR_XIIC_0_BASEADDR);
	if (!ConfigPtr_IIC)
		return XST_FAILURE;
#endif
    Status = XIic_CfgInitialize(&IicInstance, ConfigPtr_IIC,
											ConfigPtr_IIC->BaseAddress);
    if (Status != XST_SUCCESS) {
            return XST_FAILURE;
    }

    XVidFrameCrc_Initialize(&VidFrameCRC);

	return XST_SUCCESS;
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
#ifndef PLATFORM_MB
	XINTC *IntcInstPtr = &IntcInst;
#else
	XIntc *IntcInstPtr = &IntcInst;
#endif

	/* Set custom timer wait */
	XDpTxSs_SetUserTimerHandler(&DpTxSsInst, &DpPt_CustomWaitUs, &TmrCtr);
	XDpTxSs_SetCallBack(&DpTxSsInst, (XDPTXSS_HANDLER_DP_HPD_EVENT),
						(void *)DpPt_HpdEventHandler, &DpTxSsInst);
	XDpTxSs_SetCallBack(&DpTxSsInst, (XDPTXSS_HANDLER_DP_HPD_PULSE),
			(void *)DpPt_HpdPulseHandler, &DpTxSsInst);
	XDpTxSs_SetCallBack(&DpTxSsInst, (XDPTXSS_HANDLER_DP_LINK_RATE_CHG),
			(void *)DpPt_LinkrateChgHandler, &DpTxSsInst);
	XDpTxSs_SetCallBack(&DpTxSsInst, (XDPTXSS_HANDLER_DP_PE_VS_ADJUST),
			(void *)DpPt_pe_vs_adjustHandler, &DpTxSsInst);
	XDpTxSs_SetCallBack(&DpTxSsInst, (XDPTXSS_HANDLER_DP_EXT_PKT_EVENT),
			(void *)DpPt_ExtPkt_Handler, &DpTxSsInst);
	XDpTxSs_SetCallBack(&DpTxSsInst, (XDPTXSS_HANDLER_DP_VSYNC),
			(void *)DpPt_Vblank_Handler, &DpTxSsInst);

	XDpTxSs_SetCallBack(&DpTxSsInst, (XDPTXSS_HANDLER_DP_FFE_PRESET_ADJUST),
			(void *)DpPt_ffe_adjustHandler, &DpTxSsInst);

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
	Status = XScuGic_Connect(IntcInstPtr, XINTC_DPTXSS_DP_INTERRUPT_ID,
			(Xil_InterruptHandler)XDpTxSs_DpIntrHandler,
				&DpTxSsInst);
	if (Status != XST_SUCCESS) {
		xil_printf("ERR: DP TX SS DP interrupt connect failed!\r\n");
		return XST_FAILURE;
	}

	if (set_phy == 0) {
		/* Enable the interrupt */
		XScuGic_Enable(IntcInstPtr,
				XINTC_DPTXSS_DP_INTERRUPT_ID);
	} else {
		XScuGic_Disable(IntcInstPtr,
				XINTC_DPTXSS_DP_INTERRUPT_ID);

	}
#else

	/* Initialize the interrupt controller driver so that it is ready to
	 * use.
	 */

	Status = XIntc_Initialize(IntcInstPtr, XINTC_DEVICE_ID);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/* Connect the device driver handler that will be called when an
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


	/* Enable the interrupt */
	XIntc_Enable(IntcInstPtr,
			XINTC_DPTXSS_DP_INTERRUPT_ID);

    Status = XIntc_Start(IntcInstPtr, XIN_REAL_MODE);
    if (Status != XST_SUCCESS) {
            return XST_FAILURE;
    }


#endif

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
#else
u32 DpTxSs_SetupIntrSystem(void)
{
	u32 Status;

	/* Set custom timer wait */
	XDpTxSs_SetUserTimerHandler(&DpTxSsInst, &DpPt_CustomWaitUs, &TmrCtr);
	XDpTxSs_SetCallBack(&DpTxSsInst, XDPTXSS_HANDLER_DP_HPD_EVENT,
			    (void *)DpPt_HpdEventHandler, &DpTxSsInst);
	XDpTxSs_SetCallBack(&DpTxSsInst, XDPTXSS_HANDLER_DP_HPD_PULSE,
			    (void *)DpPt_HpdPulseHandler, &DpTxSsInst);
	XDpTxSs_SetCallBack(&DpTxSsInst, XDPTXSS_HANDLER_DP_LINK_RATE_CHG,
			    (void *)DpPt_LinkrateChgHandler, &DpTxSsInst);
	XDpTxSs_SetCallBack(&DpTxSsInst, XDPTXSS_HANDLER_DP_PE_VS_ADJUST,
			    (void *)DpPt_pe_vs_adjustHandler, &DpTxSsInst);
	XDpTxSs_SetCallBack(&DpTxSsInst, XDPTXSS_HANDLER_DP_EXT_PKT_EVENT,
			    (void *)DpPt_ExtPkt_Handler, &DpTxSsInst);
	XDpTxSs_SetCallBack(&DpTxSsInst, XDPTXSS_HANDLER_DP_VSYNC,
			    (void *)DpPt_Vblank_Handler, &DpTxSsInst);
	XDpTxSs_SetCallBack(&DpTxSsInst, XDPTXSS_HANDLER_DP_FFE_PRESET_ADJUST,
			    (void *)DpPt_ffe_adjustHandler, &DpTxSsInst);

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

	/* Obtain the device configuration for the DisplayPort RX Subsystem */
#ifndef SDT
	ConfigPtr = XVphy_LookupConfig(DeviceId);
#else
	ConfigPtr = XVphy_LookupConfig(BaseAddress);
#endif
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


	// initial line Rate setting
	prev_line_rate = PHY_User_Config_Table[5].LineRate;
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
void PLLRefClkSel (XVphy *InstancePtr, u8 link_rate) {
	switch (link_rate) {
	case XDP_TX_LINK_BW_SET_162GBPS:
		XVphy_CfgQuadRefClkFreq(InstancePtr, 0,
										ONBOARD_REF_CLK, 270000000);
			XVphy_CfgQuadRefClkFreq(InstancePtr, 0,
											ONBOARD_400_CLK, 400000000);
			XVphy_CfgLineRate(InstancePtr, 0,
						XVPHY_CHANNEL_ID_CHA, XVPHY_DP_LINK_RATE_HZ_162GBPS);
			XVphy_CfgLineRate(InstancePtr, 0,
						XVPHY_CHANNEL_ID_CMN0, XVPHY_DP_LINK_RATE_HZ_162GBPS);
			break;
	case XDP_TX_LINK_BW_SET_540GBPS:
		XVphy_CfgQuadRefClkFreq(InstancePtr, 0,
										ONBOARD_REF_CLK, 270000000);
			XVphy_CfgQuadRefClkFreq(InstancePtr, 0,
											ONBOARD_400_CLK, 400000000);
			XVphy_CfgLineRate(InstancePtr, 0,
						XVPHY_CHANNEL_ID_CHA, XVPHY_DP_LINK_RATE_HZ_540GBPS);
			XVphy_CfgLineRate(InstancePtr, 0,
					XVPHY_CHANNEL_ID_CMN0, XVPHY_DP_LINK_RATE_HZ_540GBPS);
			break;
	case XDP_TX_LINK_BW_SET_810GBPS:
		XVphy_CfgQuadRefClkFreq(InstancePtr, 0,
										ONBOARD_REF_CLK, 270000000);
			XVphy_CfgQuadRefClkFreq(InstancePtr, 0,
											ONBOARD_400_CLK, 400000000);
			XVphy_CfgLineRate(InstancePtr, 0,
						XVPHY_CHANNEL_ID_CHA, XVPHY_DP_LINK_RATE_HZ_810GBPS);
			XVphy_CfgLineRate(InstancePtr, 0,
					XVPHY_CHANNEL_ID_CMN0, XVPHY_DP_LINK_RATE_HZ_810GBPS);
			break;
	case 0x01:
		XVphy_CfgQuadRefClkFreq(InstancePtr, 0,
										ONBOARD_REF_CLK, 270000000);
			XVphy_CfgQuadRefClkFreq(InstancePtr, 0,
									ONBOARD_400_CLK, 400000000);
			XVphy_CfgLineRate(InstancePtr, 0,
						XVPHY_CHANNEL_ID_CHA, XVPHY_DP_LINK_RATE_HZ_1000GBPS);
			XVphy_CfgLineRate(InstancePtr, 0,
					XVPHY_CHANNEL_ID_CMN0, XVPHY_DP_LINK_RATE_HZ_1000GBPS);
			break;
#if !defined (XPS_BOARD_ZCU102)
	case 0x02:
		XVphy_CfgQuadRefClkFreq(InstancePtr, 0,
										ONBOARD_REF_CLK, 270000000);
			XVphy_CfgQuadRefClkFreq(InstancePtr, 0,
											ONBOARD_400_CLK, 400000000);
			XVphy_CfgLineRate(InstancePtr, 0,
						XVPHY_CHANNEL_ID_CHA, XVPHY_DP_LINK_RATE_HZ_2000GBPS);
			XVphy_CfgLineRate(InstancePtr, 0,
					XVPHY_CHANNEL_ID_CMN0, XVPHY_DP_LINK_RATE_HZ_2000GBPS);

			break;
#endif
	case 0x04:
		XVphy_CfgQuadRefClkFreq(InstancePtr, 0,
										ONBOARD_REF_CLK, 270000000);
			XVphy_CfgQuadRefClkFreq(InstancePtr, 0,
											ONBOARD_400_CLK, 400000000);
			XVphy_CfgLineRate(InstancePtr, 0,
						XVPHY_CHANNEL_ID_CHA, XVPHY_DP_LINK_RATE_HZ_1350GBPS);
			XVphy_CfgLineRate(InstancePtr, 0,
					XVPHY_CHANNEL_ID_CMN0, XVPHY_DP_LINK_RATE_HZ_1350GBPS);
			break;

	default:
		XVphy_CfgQuadRefClkFreq(InstancePtr, 0,
										ONBOARD_REF_CLK, 270000000);
			XVphy_CfgQuadRefClkFreq(InstancePtr, 0,
									ONBOARD_400_CLK, 400000000);
			XVphy_CfgLineRate(InstancePtr, 0,
						XVPHY_CHANNEL_ID_CHA, XVPHY_DP_LINK_RATE_HZ_270GBPS);
			XVphy_CfgLineRate(InstancePtr, 0,
					XVPHY_CHANNEL_ID_CMN0, XVPHY_DP_LINK_RATE_HZ_270GBPS);
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
void DpPt_LinkrateChgHandler(void *InstancePtr)
{
	// If TX is unable to train at what it has been asked then
	// necessary down shift handling has to be done here
	// eg. reconfigure GT to new rate etc

	u8 rate;
	u8 lanes;
	rate = get_LineRate();
    lanes = get_Lanecounts();
	config_phy(rate);

	//update the previous link rate info at here
	prev_line_rate = rate;
}


void DpPt_ffe_adjustHandler(void *InstancePtr){

u16 preemp = 0;
u16 diff_swing = 0;
	switch(DpTxSsInst.DpPtr->TxInstance.LinkConfig.PresetVal[0]){
	case 0:
		preemp = 0x5; //XVPHY_GTHE4_PREEMP_DP_L0;
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
//

	XVphy_SetTxPostCursor(&VPhyInst, 0, XVPHY_CHANNEL_ID_CH1, preemp);
	XVphy_SetTxPostCursor(&VPhyInst, 0, XVPHY_CHANNEL_ID_CH2, preemp);
	XVphy_SetTxPostCursor(&VPhyInst, 0, XVPHY_CHANNEL_ID_CH3, preemp);
	XVphy_SetTxPostCursor(&VPhyInst, 0, XVPHY_CHANNEL_ID_CH4, preemp);
}

void DpPt_pe_vs_adjustHandler(void *InstancePtr){
	u32 vswing;
	u32 postcursor, value;
	if(PE_VS_ADJUST == 1){
		u16 preemp = 0;
		switch(DpTxSsInst.DpPtr->TxInstance.LinkConfig.PeLevel){
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
		switch(DpTxSsInst.DpPtr->TxInstance.LinkConfig.VsLevel){
			case 0:
				switch(DpTxSsInst.DpPtr->TxInstance.LinkConfig.PeLevel){
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
				switch(DpTxSsInst.DpPtr->TxInstance.LinkConfig.PeLevel){
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
				switch(DpTxSsInst.DpPtr->TxInstance.LinkConfig.PeLevel){
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

		//setting vswing
		XVphy_SetTxVoltageSwing(&VPhyInst, 0, XVPHY_CHANNEL_ID_CH1,
				diff_swing);
		XVphy_SetTxVoltageSwing(&VPhyInst, 0, XVPHY_CHANNEL_ID_CH2,
				diff_swing);
		XVphy_SetTxVoltageSwing(&VPhyInst, 0, XVPHY_CHANNEL_ID_CH3,
				diff_swing);
		XVphy_SetTxVoltageSwing(&VPhyInst, 0, XVPHY_CHANNEL_ID_CH4,
				diff_swing);

		//setting postcursor
		XVphy_SetTxPostCursor(&VPhyInst, 0, XVPHY_CHANNEL_ID_CH1, preemp);
		XVphy_SetTxPostCursor(&VPhyInst, 0, XVPHY_CHANNEL_ID_CH2, preemp);
		XVphy_SetTxPostCursor(&VPhyInst, 0, XVPHY_CHANNEL_ID_CH3, preemp);
		XVphy_SetTxPostCursor(&VPhyInst, 0, XVPHY_CHANNEL_ID_CH4, preemp);
	}
}

void DpPt_ExtPkt_Handler(void *InstancePtr) {
/* Empty
 *
 */
}

void DpPt_Vblank_Handler(void *InstancePtr) {
/*Empty
 *
 */
}

void DpPt_CustomWaitUs(void *InstancePtr, u32 MicroSeconds)
{

	u32 TimerVal = 0, TimerVal_pre;
	u32 NumTicks = (MicroSeconds * (
#ifndef SDT
	XPAR_PROCESSOR_SYSTEM_AXI_TIMER_0_CLOCK_FREQ_HZ / 1000000));
#else
	XPAR_PROCESSOR_SYSTEM_AXI_TIMER_0_CLOCK_FREQUENCY / 1000000));
#endif
	XTmrCtr_Reset(&TmrCtr, 0);
	XTmrCtr_Start(&TmrCtr, 0);

	/* Wait specified number of useconds. */
	do {
		TimerVal_pre = TimerVal;
		TimerVal = XTmrCtr_GetValue(&TmrCtr, 0);
		if(TimerVal_pre == TimerVal){
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
void DpPt_HpdEventHandler(void *InstancePtr)
{
	if (XDpTxSs_IsConnected(&DpTxSsInst)) {
		sink_power_down();
		sink_power_up();
		tx_is_reconnected++;
		xil_printf ("Cable Connected\r\n");
	}
	else
	{
		xil_printf ("Cable Disconnected\r\n");
		XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr, XDP_TX_ENABLE, 0x0);
//		DpTxSs_DisableAudio
		XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr,
												XDP_TX_AUDIO_CONTROL, 0x0);
		tx_is_reconnected = 0;
		//Setting Vs and Pe to 0,0 on Cable disconnect
		DpTxSsInst.DpPtr->TxInstance.LinkConfig.VsLevel = 0;
		DpTxSsInst.DpPtr->TxInstance.LinkConfig.PeLevel = 0;

        DpPt_pe_vs_adjustHandler (&VPhyInst);

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
void DpPt_HpdPulseHandler(void *InstancePtr)
{

// Some monitors give HPD pulse repeatedly which causes HPD pulse function to
//		be executed huge number of time. Hence hpd_pulse interrupt is disabled
//		and then enabled when hpd_pulse function is executed
			XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr,
					XDP_TX_INTERRUPT_MASK,
					XDP_TX_INTERRUPT_MASK_HPD_PULSE_DETECTED_MASK);
			hpd_pulse_con_event = 1;
			xil_printf ("HPD Pulse..\r\n");
			hpd_pulse_con(&DpTxSsInst);
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

void hpd_pulse_con(XDpTxSs *InstancePtr)
{
	u32 Readval;

	u8 lane0_sts = InstancePtr->UsrHpdPulseData.Lane0Sts;
	u8 lane2_sts = InstancePtr->UsrHpdPulseData.Lane2Sts;
	u8 laneAlignStatus = InstancePtr->UsrHpdPulseData.LaneAlignStatus;
	u8 bw_set = InstancePtr->UsrHpdPulseData.BwSet;
	u8 lane_set = InstancePtr->UsrHpdPulseData.LaneSet;
	u8 down_strm = InstancePtr->UsrHpdPulseData.AuxValues[4];

	u8 retrain_link=0;

     lane_set = lane_set & 0x1F;
     bw_set = bw_set & 0x1F;
     laneAlignStatus = laneAlignStatus & 0x1;

     if(bw_set != XDP_TX_LINK_BW_SET_810GBPS
		 && bw_set != XDP_TX_LINK_BW_SET_162GBPS
             && bw_set != XDP_TX_LINK_BW_SET_270GBPS
             && bw_set != XDP_TX_LINK_BW_SET_540GBPS
			 && bw_set != XDP_TX_LINK_BW_SET_UHBR10
			 && bw_set != XDP_TX_LINK_BW_SET_UHBR20 && bw_set != XDP_TX_LINK_BW_SET_UHBR135){
        bw_set = InstancePtr->DpPtr->Config.MaxLinkRate;
        retrain_link = 1;
     }
     if(lane_set != XDP_TX_LANE_COUNT_SET_1
             && lane_set != XDP_TX_LANE_COUNT_SET_2
             && lane_set != XDP_TX_LANE_COUNT_SET_4){
        lane_set = InstancePtr->DpPtr->Config.MaxLaneCount;
        retrain_link = 1;
     }

	lane0_sts = lane0_sts & 0x77;
	lane2_sts = lane2_sts & 0x77;
	if (lane_set == XDP_TX_LANE_COUNT_SET_4) {
		if ((lane0_sts !=
				(XDP_DPCD_STATUS_LANE_0_CR_DONE_MASK |
				XDP_DPCD_STATUS_LANE_0_CE_DONE_MASK |
				XDP_DPCD_STATUS_LANE_0_SL_DONE_MASK |
				XDP_DPCD_STATUS_LANE_1_CR_DONE_MASK |
				XDP_DPCD_STATUS_LANE_1_CE_DONE_MASK |
				XDP_DPCD_STATUS_LANE_1_SL_DONE_MASK)
				) || (lane2_sts !=
				(XDP_DPCD_STATUS_LANE_2_CR_DONE_MASK |
				XDP_DPCD_STATUS_LANE_2_CE_DONE_MASK |
				XDP_DPCD_STATUS_LANE_2_SL_DONE_MASK |
				XDP_DPCD_STATUS_LANE_3_CR_DONE_MASK |
				XDP_DPCD_STATUS_LANE_3_CE_DONE_MASK |
				XDP_DPCD_STATUS_LANE_3_SL_DONE_MASK)
				) || (laneAlignStatus != 1)) {
			retrain_link = 1;
		}
	} else if (lane_set == XDP_TX_LANE_COUNT_SET_2) {
		if ((lane0_sts !=
				(XDP_DPCD_STATUS_LANE_0_CR_DONE_MASK |
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
		if ((lane0_sts !=
				(XDP_DPCD_STATUS_LANE_0_CR_DONE_MASK |
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

     XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr,XDP_TX_INTERRUPT_MASK, 0x0);
}



// introduced to address reduced blanking linereset issue from 16.4 release
u32 DpTxSubsystem_Start(XDpTxSs *InstancePtr, int with_msa){
	u32 Status;
		Status = XDpTxSs_Start(&DpTxSsInst);

	return Status;
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
										u8 Edid_org[128], u8 Edid1_org[128]){
	u8 Status;

	XDp_ReadReg(DpTxSsInst.DpPtr->Config.BaseAddr,XDP_TX_INTERRUPT_STATUS);

	DpTxSsInst.DpPtr->TxInstance.TxSetMsaCallback = NULL;
	DpTxSsInst.DpPtr->TxInstance.TxMsaCallbackRef = NULL;

	u8 connected = 0;

	if (set_phy == 0) {
	// this is intentional infinite while loop
    while (!XDpTxSs_IsConnected(&DpTxSsInst)) {
	if (connected == 0) {
	xil_printf(
			"Please connect a DP Monitor to start the application !!!\r\n");
	connected = 1;
	}
    }

	//Waking up the monitor
    sink_power_cycle();


	//reading the first block of EDID
	if (XDpTxSs_IsConnected(&DpTxSsInst)) {
		XDp_TxGetEdidBlock(DpTxSsInst.DpPtr, Edid_org, 0);
		//reading the second block of EDID
		XDp_TxGetEdidBlock(DpTxSsInst.DpPtr, Edid1_org, 1);
		xil_printf("Reading EDID contents of the DP Monitor..\r\n");

		Status  = XDp_TxAuxRead(DpTxSsInst.DpPtr,
								XDP_DPCD_MAX_LINK_RATE,  1, LineRate_init);
		Status |= XDp_TxAuxRead(DpTxSsInst.DpPtr,
								XDP_DPCD_MAX_LANE_COUNT, 1, LaneCount_init);

		u8 rData = 0;
		// check the EXTENDED_RECEIVER_CAPABILITY_FIELD_PRESENT bit
		XDp_TxAuxRead(DpTxSsInst.DpPtr, XDP_DPCD_TRAIN_AUX_RD_INTERVAL, 1, &rData);
		if(rData & 0x80){ // if EXTENDED_RECEIVER_CAPABILITY_FIELD is enabled
			XDp_TxAuxRead(DpTxSsInst.DpPtr, 0x2201, 1, &rData); // read maxLineRate
			if(rData == XDP_DPCD_LINK_BW_SET_810GBPS){
				*LineRate_init = 0x1E;
				xil_printf ("Capability is %x\r\n", *LineRate_init);
			}
		}


		if (Status != XST_SUCCESS) { // give another chance to monitor.
			//Waking up the monitor
			sink_power_cycle();

			XDp_TxGetEdidBlock(DpTxSsInst.DpPtr, Edid_org, 0);
			//reading the second block of EDID
			XDp_TxGetEdidBlock(DpTxSsInst.DpPtr, Edid1_org, 1);
			xil_printf("Reading EDID contents of the DP Monitor..\r\n");

			Status = XDp_TxAuxRead(DpTxSsInst.DpPtr,
								XDP_DPCD_MAX_LINK_RATE, 1, LineRate_init);
			Status |= XDp_TxAuxRead(DpTxSsInst.DpPtr,
								XDP_DPCD_MAX_LANE_COUNT, 1, LaneCount_init);
			// check the EXTENDED_RECEIVER_CAPABILITY_FIELD_PRESENT bit
			XDp_TxAuxRead(DpTxSsInst.DpPtr, XDP_DPCD_TRAIN_AUX_RD_INTERVAL, 1, &rData);
			if(rData & 0x80){ // if EXTENDED_RECEIVER_CAPABILITY_FIELD is enabled
				XDp_TxAuxRead(DpTxSsInst.DpPtr, 0x2201, 1, &rData); // read maxLineRate
				if(rData == XDP_DPCD_LINK_BW_SET_810GBPS){
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
	}

     xil_printf("System capabilities set to: LineRate %x, LaneCount %x\r\n",
											 *LineRate_init,*LaneCount_init);
}

/*****************************************************************************/
/**
*
* This function sets up PHY
*
* @param	pointer to VideoPHY
* @param	User Config table
*
* @return
*		- XST_SUCCESS if interrupt setup was successful.
*		- A specific error code defined in "xstatus.h" if an error
*		occurs.
*
* @note		None.
*
******************************************************************************/
u32 PHY_Configuration_Tx(XVphy *InstancePtr,
									XVphy_User_Config PHY_User_Config_Table){

	XVphy_PllRefClkSelType QpllRefClkSel;
	XVphy_PllRefClkSelType CpllRefClkSel;
	XVphy_PllType TxPllSelect;
	XVphy_PllType RxPllSelect; // Required for VPHY setting
	XVphy_ChannelId TxChId;
	u8 QuadId = 0;
	u32 Status = XST_FAILURE;
	u32 retries = 0;

	QpllRefClkSel   		= PHY_User_Config_Table.QPLLRefClkSrc;
	CpllRefClkSel   		= PHY_User_Config_Table.CPLLRefClkSrc;
	TxPllSelect             = PHY_User_Config_Table.TxPLL;
	// Required for VPHY setting
	RxPllSelect             = PHY_User_Config_Table.RxPLL;
	TxChId                  = PHY_User_Config_Table.TxChId;



			//Set the Ref Clock Frequency
	XVphy_CfgQuadRefClkFreq(InstancePtr, QuadId, QpllRefClkSel,
						PHY_User_Config_Table.QPLLRefClkFreqHz);
	XVphy_CfgQuadRefClkFreq(InstancePtr, QuadId, CpllRefClkSel,
						PHY_User_Config_Table.CPLLRefClkFreqHz);
	XVphy_CfgLineRate(InstancePtr, QuadId, TxChId,
						PHY_User_Config_Table.LineRateHz);

	XVphy_PllInitialize(InstancePtr, QuadId, TxChId,
					QpllRefClkSel, CpllRefClkSel, TxPllSelect, RxPllSelect);

	// Initialize GT with ref clock and PLL selects
	// GT DRPs may not get completed if GT is busy doing something else
	// hence this is run in loop and retried 100 times
	while (Status != XST_SUCCESS) {
		Status = XVphy_ClkInitialize(InstancePtr, QuadId, TxChId, XVPHY_DIR_TX);
		if (retries > 100) {
			retries = 0;
			break;
		}
		retries++;
	}


	XVphy_WriteReg(InstancePtr->Config.BaseAddr,
			XVPHY_PLL_RESET_REG,
			(XVPHY_PLL_RESET_QPLL0_MASK | XVPHY_PLL_RESET_QPLL1_MASK |
					XVPHY_PLL_RESET_CPLL_MASK)); // 0x06
	XVphy_WriteReg(InstancePtr->Config.BaseAddr, XVPHY_PLL_RESET_REG, 0x0);


	Status = XVphy_ResetGtPll(InstancePtr, QuadId,
			XVPHY_CHANNEL_ID_CHA, XVPHY_DIR_TX,(FALSE));


	Status += XVphy_WaitForPmaResetDone(InstancePtr, QuadId,
						XVPHY_CHANNEL_ID_CHA, XVPHY_DIR_TX);
	if (Status  != XST_SUCCESS) {
		xil_printf ("++++rst failed error++++\r\n");
	}

	Status += XVphy_WaitForPllLock(InstancePtr, QuadId, TxChId);
	if (Status  != XST_SUCCESS) {
		xil_printf ("++++lock failed++++\r\n");
	}

	Status += XVphy_WaitForResetDone(InstancePtr, QuadId,
						XVPHY_CHANNEL_ID_CHA, XVPHY_DIR_TX);
	if (Status  != XST_SUCCESS) {
		xil_printf ("++++TX GT config encountered error++++\r\n");
	}

	return (Status);
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
u32 start_tx(u8 line_rate, u8 lane_count,user_config_struct user_config){
	XVidC_VideoMode res_table = user_config.VideoMode_local;
	u8 bpc = user_config.user_bpc;
	u8 pat = user_config.user_pattern;
	u8 format = user_config.user_format;
	u8 C_VideoUserStreamPattern[8] = {0x10, 0x11, 0x12, 0x13, 0x14,
												0x15, 0x16, 0x17}; //Duplicate


	u32 Status;
	u32 Readval;
	// Stop the Patgen
	XDp_WriteReg(XPAR_TX_SUBSYSTEM_AV_PAT_GEN_0_BASEADDR,
			0x0, 0x0);

	//Waking up the monitor
	sink_power_cycle();


    XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr, XDP_TX_ENABLE, 0x0);
// Give a bit of time for DP IP after monitor came up and starting Link training
	usleep(100000);
    XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr, XDP_TX_ENABLE, 0x1);

    xil_printf ("\r\nTraining TX with: Link rate %x, Lane count %d\r\n",
													line_rate,lane_count);

	XDpTxSs_SetLinkRate(&DpTxSsInst, line_rate);
    xil_printf (".");
    XDpTxSs_SetLaneCount(&DpTxSsInst, lane_count);
    xil_printf (".");

    if (res_table !=0) {
		Status = XDpTxSs_SetVidMode(&DpTxSsInst, res_table);
		if (Status != XST_SUCCESS) {
			xil_printf("ERR: Setting resolution failed\r\n");
		}
		xil_printf (".");
    }
	if (bpc !=0 ) {
		Status = XDpTxSs_SetBpc(&DpTxSsInst, bpc);
		if (Status != XST_SUCCESS){
			xil_printf("ERR: Setting bpc to %d failed\r\n",bpc);
		}
		xil_printf (".");
	}

	/*
	 * Setting Color Format
	 * User can change coefficients here - By default 601 is used for YCbCr
	 * */
	XDp_TxCfgSetColorEncode(DpTxSsInst.DpPtr, XDP_TX_STREAM_ID1, \
			format, XVIDC_BT_601, XDP_DR_VESA);

	if (set_phy == 0) {
		Status = DpTxSubsystem_Start(&DpTxSsInst, 0);

	if (Status != XST_SUCCESS) {
		xil_printf("ERR:1DPTX SS start failed\r\n");
		return (XST_FAILURE);
	}

	}
	xil_printf (".");
	//updates required timing values in Video Pattern Generator
	Vpg_StreamSrcConfigure(DpTxSsInst.DpPtr, 0, 1);
	xil_printf (".");
	// setting video pattern
	Vpg_VidgenSetUserPattern(DpTxSsInst.DpPtr, C_VideoUserStreamPattern[pat]);
	xil_printf (".");
    /* Generate the video clock using MMCM
     */
    Gen_vid_clk(DpTxSsInst.DpPtr,(XDP_TX_STREAM_ID1));
	clk_wiz_locked();

	if (DpTxSsInst.VtcPtr[0]) {
		 Status = XDpTxSs_VtcSetup(DpTxSsInst.VtcPtr[0],
		 &DpTxSsInst.DpPtr->TxInstance.MsaConfig[0],
		 DpTxSsInst.UsrOpt.VtcAdjustBs);
		 if (Status != XST_SUCCESS) {
				 xdbg_printf(XDBG_DEBUG_GENERAL,"SS ERR: "
						 "VTC%d setup failed!\n\r", Index);
         }
    }

	if (set_phy == 0) {
    Status = XDpTxSs_CheckLinkStatus(&DpTxSsInst);
	if (Status != (XST_SUCCESS)) {
		Status = DpTxSubsystem_Start(&DpTxSsInst, 0);
		if (Status != XST_SUCCESS) {
			xil_printf("ERR:DPTX SS start failed\r\n");
			return (XST_FAILURE);
		}

	}
	}
	XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr,XDP_TX_INTERRUPT_MASK, 0x0);

	/*
	 * Initialize CRC
	 */
	/* Reset CRC*/
	XVidFrameCrc_Reset();
//	/* Set Pixel width in CRC engine*/
    if (format != 2) {
	XDp_WriteReg(XPAR_TX_SUBSYSTEM_CRC_BASEADDR, VIDEO_FRAME_CRC_CONFIG,
			CRC_CFG);
    } else {
	XDp_WriteReg(XPAR_TX_SUBSYSTEM_CRC_BASEADDR, VIDEO_FRAME_CRC_CONFIG,
			CRC_CFG | 0x80000000);

    }

	xil_printf ("..done !\r\n");
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
#ifndef SDT
void clk_wiz_locked(void) {
	volatile u32 res = XDp_ReadReg(XPAR_GPIO_0_BASEADDR,0x0);
	u32 timer=0;

	while ( res == 0 && timer < 1000) {
		xil_printf ("~/~/");
		res = XDp_ReadReg(XPAR_GPIO_0_BASEADDR,0x0);
		timer++; // timer for timeout. No need to be specific time.
					// As long as long enough to wait lock
	}
	xil_printf ("^^");
}
#else
void clk_wiz_locked(void)
{
	volatile u32 res = XDp_ReadReg(XPAR_XGPIO_0_BASEADDR, 0x0);
	u32 timer = 0;

	while (res == 0 && timer < 1000) {
		xil_printf("~/~/");
		res = XDp_ReadReg(XPAR_XGPIO_0_BASEADDR, 0x0);
		timer++; // timer for timeout. No need to be specific time.
					// As long as long enough to wait lock
	}
	xil_printf("^^");
}
#endif

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


void hpd_con(XDpTxSs *InstancePtr, u8 Edid_org[128],
		u8 Edid1_org[128], u16 res_update){
	u32 Status=XST_SUCCESS;
	u8 Channel_encoding = 0;
	u8 max_cap_new = InstancePtr->UsrHpdEventData.MaxCapNew;
	u8 max_cap_lanes_new = InstancePtr->UsrHpdEventData.MaxCapLanesNew;
	u32 htotal_test_hpd;
	u32 vtotal_test_hpd;
	u32 freq_test_hpd;
	u8 good_edid_hpd = 1;
	XVidC_VideoMode VmId_test_hpd;
	XVidC_VideoMode VmId_ptm_hpd;
	u8 bpc_hpd;
	u8 C_VideoUserStreamPattern[8] = {0x10, 0x11, 0x12, 0x13, 0x14,
												0x15, 0x16, 0x17}; //Duplicate

//	//read channel encoding if 128/132b supported
	XDp_TxAuxRead(DpTxSsInst.DpPtr, XDP_DPCD_ML_CH_CODING_CAP, 1,
															&Channel_encoding);
	if((Channel_encoding & 0x2) == 0x2){ //supports 128/132b hence UHBR rates
		XDp_TxAuxRead(DpTxSsInst.DpPtr, XDP_DPCD_128B_132B_SUPPORTED_LINK_RATE, 1,
																	&max_cap_new);
		if((max_cap_new & 0x2) == 0x2){
			max_cap_new = XDP_TX_LINK_BW_SET_UHBR20;
		}else if((max_cap_new & 0x4) == 0x4){
			max_cap_new = XDP_TX_LINK_BW_SET_UHBR135;
		}else if((max_cap_new & 0x1) == 0x1){
			max_cap_new = XDP_TX_LINK_BW_SET_UHBR10;
		}
	}

	user_config_struct user_config;
	XDp_ReadReg(DpTxSsInst.DpPtr->Config.BaseAddr,XDP_TX_INTERRUPT_STATUS);

	memcpy(Edid_org, InstancePtr->UsrHpdEventData.EdidOrg, 128);

	tx_is_reconnected=0;

	if (XVidC_EdidIsHeaderValid(Edid_org)) {
		good_edid_hpd = 1;
		if (!CalculateChecksum(Edid_org, 128)) {
				good_edid_hpd = 1;
			} else {
				good_edid_hpd = 0;
				xil_printf ("Bad EDID Checksum\r\n");
			}
	}
	else {
		good_edid_hpd = 0;
		xil_printf ("Bad EDID Header\r\n");
	}

	// Till here is the requirement per DP spec


	//////////////////////////////////////////////
	// From here is optional per application
	//////////////////////////////////////////////

	// check if lane is 1/2/4 or something else
	if(max_cap_lanes_new != 1
			&& max_cap_lanes_new != 2
			&& max_cap_lanes_new != 4){
		// soemthing wrong. Read again
		XDp_TxAuxRead(DpTxSsInst.DpPtr, XDP_DPCD_MAX_LANE_COUNT, 1,
														&max_cap_lanes_new);
	}

	// check if line speed is either 0x6, 0xA, 0x14, 0x1E, 0x1, 0x4, 0x2
	if (max_cap_new != XDP_TX_LINK_BW_SET_UHBR20
			&&max_cap_new != XDP_TX_LINK_BW_SET_UHBR135
			&&max_cap_new != XDP_TX_LINK_BW_SET_UHBR10
			&& max_cap_new != XDP_TX_LINK_BW_SET_810GBPS
			&& max_cap_new != XDP_TX_LINK_BW_SET_540GBPS
				&& max_cap_new != XDP_TX_LINK_BW_SET_270GBPS
				&& max_cap_new != XDP_TX_LINK_BW_SET_162GBPS) {

//		//read channel encoding if 128/132b supported
		XDp_TxAuxRead(DpTxSsInst.DpPtr, XDP_DPCD_ML_CH_CODING_CAP, 1,
																&Channel_encoding);
		if((Channel_encoding & 0x2) == 0x2){ //supports 128/132b hence UHBR rates
			XDp_TxAuxRead(DpTxSsInst.DpPtr, XDP_DPCD_128B_132B_SUPPORTED_LINK_RATE, 1,
																		&max_cap_new);
			if((max_cap_new & 0x2) == 0x2){
				max_cap_new = XDP_TX_LINK_BW_SET_UHBR20;
			}else if((max_cap_new & 0x4) == 0x4){
				max_cap_new = XDP_TX_LINK_BW_SET_UHBR135;
			}else if((max_cap_new & 0x1) == 0x1){
				max_cap_new = XDP_TX_LINK_BW_SET_UHBR10;
			}
		}else{
			// soemthing wrong. Read again
			XDp_TxAuxRead(DpTxSsInst.DpPtr, XDP_DPCD_MAX_LINK_RATE, 1,
														&max_cap_new);
		}
	}

	if (max_cap_new != XDP_TX_LINK_BW_SET_UHBR20
			&&max_cap_new != XDP_TX_LINK_BW_SET_UHBR135
			&&max_cap_new != XDP_TX_LINK_BW_SET_UHBR10
			&&max_cap_new != XDP_TX_LINK_BW_SET_810GBPS
			&& max_cap_new != XDP_TX_LINK_BW_SET_540GBPS
				&& max_cap_new != XDP_TX_LINK_BW_SET_270GBPS
				&& max_cap_new != XDP_TX_LINK_BW_SET_162GBPS) {
//		//read channel encoding if 128/132b supported
		XDp_TxAuxRead(DpTxSsInst.DpPtr, XDP_DPCD_ML_CH_CODING_CAP, 1,
																&Channel_encoding);
		if((Channel_encoding & 0x2) == 0x2){ //supports 128/132b hence UHBR rates
			XDp_TxAuxRead(DpTxSsInst.DpPtr, XDP_DPCD_128B_132B_SUPPORTED_LINK_RATE, 1,
																		&max_cap_new);
			if((max_cap_new & 0x2) == 0x2){
				max_cap_new = XDP_TX_LINK_BW_SET_UHBR20;
			}else if((max_cap_new & 0x4) == 0x4){
				max_cap_new = XDP_TX_LINK_BW_SET_UHBR135;
			}else if((max_cap_new & 0x1) == 0x1){
				max_cap_new = XDP_TX_LINK_BW_SET_UHBR10;
			}
		}else{
			max_cap_new = XDP_TX_LINK_BW_SET_810GBPS;
		}
	}
	res_update = XVIDC_VM_640x480_60_P;

	if (good_edid_hpd == 1) {
		htotal_test_hpd = XVidC_EdidGetStdTimingsH(Edid_org, 1);
		vtotal_test_hpd = XVidC_EdidGetStdTimingsV(Edid_org, 1);
		freq_test_hpd   = XVidC_EdidGetStdTimingsFrr(Edid_org, 1);
		VmId_test_hpd = XVidC_GetVideoModeId(htotal_test_hpd, vtotal_test_hpd,
															freq_test_hpd,0);
		VmId_ptm_hpd = GetPreferredVm(Edid_org, max_cap_new ,
														max_cap_lanes_new&0x1F);
		bpc_hpd = XVidC_EdidGetColorDepth(Edid_org);
		if(bpc_hpd > XPAR_TX_SUBSYSTEM_V_DP_TXSS2_0_BITS_PER_COLOR)
			bpc_hpd=XPAR_TX_SUBSYSTEM_V_DP_TXSS2_0_BITS_PER_COLOR;

		if (VmId_ptm_hpd == XVIDC_VM_NOT_SUPPORTED) { //Fail Safe mode
			VmId_ptm_hpd = XVIDC_VM_640x480_60_P;
			bpc_hpd = 8;
		}

	} else {
		VmId_test_hpd = XVIDC_VM_NOT_SUPPORTED;
		VmId_ptm_hpd = XVIDC_VM_640x480_60_P;
		bpc_hpd = 8;
		good_edid_hpd = 0;
	}

	if (max_cap_new == XDP_TX_LINK_BW_SET_UHBR20
			||max_cap_new == XDP_TX_LINK_BW_SET_UHBR135
			||max_cap_new == XDP_TX_LINK_BW_SET_UHBR10
			||max_cap_new == XDP_TX_LINK_BW_SET_810GBPS
			|| max_cap_new == XDP_TX_LINK_BW_SET_540GBPS
			|| max_cap_new == XDP_TX_LINK_BW_SET_270GBPS
			|| max_cap_new == XDP_TX_LINK_BW_SET_162GBPS) {



        if (good_edid_hpd == 1) {
			if ((VmId_ptm_hpd != XVIDC_VM_NOT_SUPPORTED)
							&& (VmId_test_hpd != XVIDC_VM_NOT_SUPPORTED)) {
				user_config.VideoMode_local = VmId_ptm_hpd;
			} else if ((VmId_ptm_hpd != XVIDC_VM_NOT_SUPPORTED)) {
				user_config.VideoMode_local = VmId_ptm_hpd;
			} else {
				user_config.VideoMode_local = VmId_test_hpd;
			}
        } else {
             user_config.VideoMode_local = res_update;
        }
		user_config.user_bpc=bpc_hpd;
		user_config.VideoMode_local=VmId_ptm_hpd;
		user_config.user_pattern=1;
		user_config.user_format = XVIDC_CSF_RGB;

		Status = config_phy(max_cap_new);
		if (Status != XST_SUCCESS) {
			xil_printf ("GT Configuration failed !!\r\n");
		}
		start_tx (max_cap_new, max_cap_lanes_new&0x1F,user_config);
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

	PixelClockHz = (((Ptm[XDP_EDID_DTD_PIXEL_CLK_KHZ_MSB] <<
					8) | Ptm[XDP_EDID_DTD_PIXEL_CLK_KHZ_LSB]) * 10) * 1000;

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

	/* Compute video mode timing values. */
	Timing.HBackPorch = HBlank - (Timing.HFrontPorch + Timing.HSyncWidth);
	Timing.F0PVBackPorch = VBlank - (Timing.F0PVFrontPorch +
													Timing.F0PVSyncWidth);
	Timing.HTotal = (Timing.HSyncWidth + Timing.HFrontPorch +
									Timing.HActive + Timing.HBackPorch);
	Timing.F0PVTotal = (Timing.F0PVSyncWidth + Timing.F0PVFrontPorch +
										Timing.VActive + Timing.F0PVBackPorch);
	FrameRate = PixelClockHz / (Timing.HTotal * Timing.F0PVTotal);


	/* Few monitors returns 59 HZ. Hence, setting to 60. */
	if (FrameRate == 59) {
		FrameRate = XVIDC_FR_60HZ;
	}

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

// There is no good way to find the timing table
	// The following function would return the video timing table from our library
	// only if the parameters match
	//however some monitors may work with different timing params
	// as such the video timing returned from the library may not work.

	if (pixel_freq1 < pixel_freq) {
		VmId = XVIDC_VM_NOT_SUPPORTED;
	} else {
		/* Get video mode ID */
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
void sink_power_down(void){
	u8 Data[8];
	Data[0] = 0x2;
	XDp_TxAuxWrite(DpTxSsInst.DpPtr,
	XDP_DPCD_SET_POWER_DP_PWR_VOLTAGE, 1, Data);
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
void sink_power_up(void){
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
u8 get_LineRate(void){
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
u8 get_Lanecounts(void){
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
u32 config_phy(int LineRate_init_tx){
	u32 Status=XST_SUCCESS;
	u8 linerate;
	u32 dptx_sts = 0;

	if (LineRate_init_tx == XDP_TX_LINK_BW_SET_810GBPS) {
		PLLRefClkSel (&VPhyInst, PHY_User_Config_Table[(is_TX_CPLL)?9:10].LineRate);
	} else if (LineRate_init_tx == XDP_TX_LINK_BW_SET_540GBPS) {
		PLLRefClkSel (&VPhyInst, PHY_User_Config_Table[(is_TX_CPLL)?2:5].LineRate);
	} else if (LineRate_init_tx == XDP_TX_LINK_BW_SET_270GBPS) {
		PLLRefClkSel (&VPhyInst, PHY_User_Config_Table[(is_TX_CPLL)?1:4].LineRate);
	} else if (LineRate_init_tx == XDP_TX_LINK_BW_SET_162GBPS) {
		PLLRefClkSel (&VPhyInst, PHY_User_Config_Table[(is_TX_CPLL)?0:3].LineRate);
	} else if (LineRate_init_tx == XDP_TX_LINK_BW_SET_UHBR10) {
	PLLRefClkSel (&VPhyInst, PHY_User_Config_Table[(is_TX_CPLL)?11:12].LineRate);

	} else if (LineRate_init_tx == XDP_TX_LINK_BW_SET_UHBR135) {
	PLLRefClkSel (&VPhyInst, PHY_User_Config_Table[(is_TX_CPLL)?13:14].LineRate);
	}
#if !defined (XPS_BOARD_ZCU102)
	else if (LineRate_init_tx == XDP_TX_LINK_BW_SET_UHBR20) {
	PLLRefClkSel (&VPhyInst, PHY_User_Config_Table[(is_TX_CPLL)?15:16].LineRate);
	}
#endif

        if ((LineRate_init_tx == XDP_TX_LINK_BW_SET_810GBPS) ||
                  (LineRate_init_tx == XDP_TX_LINK_BW_SET_540GBPS) ||
                  (LineRate_init_tx == XDP_TX_LINK_BW_SET_270GBPS) ||
                  (LineRate_init_tx == XDP_TX_LINK_BW_SET_162GBPS) ) {

                XVphy_SetupDP21Phy (&VPhyInst, 0, XVPHY_CHANNEL_ID_CMN0,
                                XVPHY_DIR_TX, LineRate_init_tx, ONBOARD_REF_CLK,
                                XVPHY_PLL_TYPE_QPLL0);
        } else {
                XVphy_SetupDP21Phy (&VPhyInst, 0, XVPHY_CHANNEL_ID_CMN0,
                                XVPHY_DIR_TX, LineRate_init_tx, ONBOARD_400_CLK,
                                XVPHY_PLL_TYPE_QPLL0);

        }

	   Status = XVphy_DP21PhyReset (&VPhyInst, 0, XVPHY_CHANNEL_ID_CMN0,
						XVPHY_DIR_TX);
		if (Status == XST_FAILURE) {
				xil_printf ("Issue encountered in PHY config and reset\r\n");
		}

	if (Status != XST_SUCCESS) {
		xil_printf (
   "+++++++ TX GT configuration encountered a failure config+++++++\r\n");
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
void sink_power_cycle(void){
	//Waking up the monitor
	sink_power_down();
	// give enough time for monitor to power down
	usleep(400);
	sink_power_up();
//	// give enough time for monitor to wake up
	usleep(30000);
	sink_power_up();//monitor to wake up once again
	usleep(4000);
}


int VideoFMC_Init(void){
	int Status;
	u8 Buffer[2];
	int ByteCount= 0;

	xil_printf("VFMC: Setting IO Expanders...\n\r");
    XIic_Reset(&IicInstance);

	/* Set the I2C Mux to select the HPC FMC */
#ifndef PLATFORM_MB
	Buffer[0] = 0x01; // Enable HPC0
#else
	Buffer[0] = 0x02; // Enable HPC0
#endif
	ByteCount = XIic_Send(IIC_BASE_ADDR, I2C_MUX_ADDR, (u8*)Buffer, 1, XIIC_STOP);
	if (ByteCount != 1) {
		xil_printf("Failed to set the I2C Mux.\n\r");
	    return XST_FAILURE;
	}

	/* Configure VFMC IO Expander 0:
	 * Disable Si5344
	 * Set primary clock source for LMK03318 to IOCLKp(0)
	 * Set secondary clock source for LMK03318 to IOCLKp(1)
	 * Disable LMK61E2*/
	Buffer[0] = 0x01;
	ByteCount = XIic_Send(IIC_BASE_ADDR, I2C_VFMCEXP_0_ADDR, (u8*)Buffer, 1, XIIC_STOP);
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
	ByteCount = XIic_Send(IIC_BASE_ADDR, I2C_VFMCEXP_1_ADDR, (u8*)Buffer, 1, XIIC_STOP);
	if (ByteCount != 1) {
		xil_printf("Failed to set the I2C IO Expander.\n\r");
		return XST_FAILURE;
	}

	xil_printf(" done!\n\r");

	Status = IDT_8T49N24x_Init(IIC_BASE_ADDR, I2C_IDT8N49_ADDR);

	if (Status != XST_SUCCESS) {
		xil_printf("Failed to initialize IDT 8T49N241.\n\r");
		return XST_FAILURE;
	}

	Status = TI_LMK03318_PowerDown(IIC_BASE_ADDR, I2C_LMK03318_ADDR);
	if (Status != XST_SUCCESS) {
		xil_printf("Failed to initialize TI LMK03318.\n\r");
		return XST_FAILURE;
	}

#ifndef PLATFORM_MB
	Status = SI5344_Init (&Ps_Iic0, I2C_SI5344_ADDR);
#else
    Status = SI5344_Init (&IicInstance, I2C_SI5344_ADDR);
#endif
	if (Status != XST_SUCCESS) {
		xil_printf("Failed to Si5344\n\r");
		return XST_FAILURE;
	}
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
		xil_printf("Found device: 0x%02x\n\r",i);
	}
	print("\n\r");
}
