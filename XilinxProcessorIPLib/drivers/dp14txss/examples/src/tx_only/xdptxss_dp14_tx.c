/******************************************************************************
* Copyright (C) 2018 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright 2022-2023 Advanced Micro Devices, Inc. All Rights Reserved.
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
* 1.00 Nishant  19/12/19  	Added support for vck190, VCU118
* 			  				DpTxSs_VideoPhyInit() renamed to DpTxSs_PhyInit()
* 			  				set_vphy() renamed to config_phy() and has two
* 			  				parameters for linerate and lanecount.
* 			  				The application files are common for ZCU102, VCU118
* 			  				and VCK190 TX Only design
* 1.01 KU		22/10/20	Added support for fabric 8b10b implementation of
* 			  				of DP1.4
* 1.02 ND		01/12/21	Added support for VSC in application menu.
* 							Changed options for Format menu.
* 1.03 ND		04/01/21	Moved all global variables declaration from .h to .c
* 							files due to gcc compiler compilation error.
* 							Setting tx_is_reconnected=0 and setting
* 							max_cap_new=XDP_TX_LINK_BW_SET_810GBPS in hpd_con
* 							for TX CTS test case.
* 1.04 KU       04/12/21    Updated Versal GT programming to get /20 clk
* 1.05 ND       05/07/21    Setting bit[12] of Reg 0x1A4 to 1 for VSC
* 							Colorimetry support
* 1.06 ND 	    07/21/22    Updated the LMK03318 address.
* 1.07 ND       08/11/22    Added support for gpio to monitor gtpowergood
*                           for versal GT
* 1.08 ND 		08/26/22    Added DELAY macro to increase delay in IDT_8T49N24x_SetClock()
* 					   		if encountering iic race condition.
* </pre>
*
******************************************************************************/
#include "xdptxss_dp14_tx.h"
#include "xvidframe_crc.h"
#include "xil_cache.h"
#ifdef SDT
#include "xinterrupt_wrap.h"
#endif
XTmrCtr TmrCtr; /* Timer instance.*/

#ifdef SDT
#define INTRNAME_DPTX   0
#endif

#ifndef versal
XIic_Config *ConfigPtr_IIC;     /* Pointer to configuration data */
XVphy VPhyInst;	/* The DPRX Subsystem instance.*/
XIic IicInstance;	/* I2C bus for Si570 */
#else
void* VPhyInst;
#endif
extern volatile u8 hpd_pulse_con_event;
volatile u8 prev_line_rate; /*This previous line rate to keep previous info to compare
						with new line rate request*/
#ifndef PLATFORM_MB
XScuGic IntcInst;
#else
XIntc IntcInst;
#endif


extern XDpTxSs DpTxSsInst;	/* The DPTX Subsystem instance.*/
extern XTmrCtr TmrCtr; /* Timer instance.*/

XDp_TxVscExtPacket VscPkt;	/* VSC Packet to populate the vsc data to be sent by
								tx */

#ifdef PLATFORM_MB
XIic  IicPtr;
typedef u8 AddressType;
u8 si570_reg_value[NUM_MODES][NUM_CLOCK_REGS] = {
        /* As per Si570 programmable oscillator calculator. */
        // 7,     8,     9,    10,      11,      12,
        {0x4C, 0x42, 0xB0, 0x21, 0xDE, 0x77 }, // = {32kHz * 512)
        {0xA5, 0xC2, 0xAA, 0xCC, 0x9D, 0x51 }, // = (44.1kHz * 512)
        {0xE4, 0xC2, 0xF4, 0xB9, 0x4A, 0xA7 }, // = (48kHz * 512)
        {0xA2, 0XC2, 0XAA, 0XCC, 0X9D, 0X51 }, // = {88.2khZ * 512)
        {0x24, 0xC2, 0xB0, 0x21, 0xDE, 0x77 }, // = {96kHz * 512)
        {0xA1, 0x42, 0xAA, 0xCC, 0x9D, 0x51 }, // = (176.4kHz * 512)
        {0x22, 0x42, 0xB0, 0x21, 0xDE, 0x77 }  // = {192kHz * 512)
};
u8 UpdateBuffer[sizeof(AddressType) + PAGE_SIZE];
#else
XIicPs Ps_Iic0, Ps_Iic1;
XIicPs_Config *XIic0Ps_ConfigPtr;
XIicPs_Config *XIic1Ps_ConfigPtr;
#ifdef versal
XClk_Wiz_Config *CfgPtr_Dynamic;
XClk_Wiz ClkWiz_Dynamic;
XGpio Gpio; /* The Instance of the GPIO Driver */
#endif
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

#ifndef versal
void PLLRefClkSel (XVphy *InstancePtr, u8 link_rate);
void PHY_Two_byte_set (XVphy *InstancePtr, u8 Rx_to_two_byte);
#endif

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
#ifndef versal
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
		  0x1E,    XVPHY_DP_LINK_RATE_HZ_810GBPS,
		  ONBOARD_REF_CLK,        ONBOARD_REF_CLK,         270000000,270000000},
  {   10,     XVPHY_PLL_TYPE_QPLL1,  XVPHY_PLL_TYPE_CPLL,
		  XVPHY_CHANNEL_ID_CMN1,    XVPHY_CHANNEL_ID_CHA,
		  0x1E,    XVPHY_DP_LINK_RATE_HZ_810GBPS,
		  ONBOARD_REF_CLK,        ONBOARD_REF_CLK,     270000000,270000000},

};
#endif

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
	xil_printf("DisplayPort TX Subsystem Example Design\r\n");
	xil_printf("(c) 2021 by Xilinx\r\n");
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

int i2c_write_dp141(u32 I2CBaseAddress, u8 I2CSlaveAddress, u16 RegisterAddress, u8 Value)
{
    u32 Status;
	u32 ByteCount = 0;
	u8 Buffer[2];
	u8 Retry = 0;

	// Write data
	Buffer[0] = RegisterAddress & 0xff;
	Buffer[1] = Value;

	while (1) {
#ifndef versal
		ByteCount = XIic_Send(I2CBaseAddress, I2CSlaveAddress, (u8*)Buffer, 3, XIIC_STOP);

		if (ByteCount == 2) {
			Status=XST_SUCCESS;
		}
		else{
			Status=XST_FAILURE;
		}
#else
	    Status = XIicPs_MasterSendPolled(&Ps_Iic0,
	                                             (u8 *)&Buffer,
	                                             2,
												 I2CSlaveAddress);
#endif
		if (Status != XST_SUCCESS) {
			Retry++;

			// Maximum retries
			if (Retry == 255) {
				return XST_FAILURE;
			}
		}

		else {
			return XST_SUCCESS;
		}
	}
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
#if !PHY_COMP
	set_phy = 0;
#else
//	set_phy = 1;
	xil_printf("\n*********APPLICATION IS IN COMPLIANCE MODE***********\n\r");
	xil_printf ("Do you want to enable PHY compliance? (y/n)\r\n");
	xil_printf ("y = Enable PHY compliance mode\r\n");
	xil_printf ("n = Enable Link Layer compliance mode\r\n");
	CmdKey_m[0] = 0;
	CommandKey_m = 0;


	CommandKey_m = xil_getc(0x0);
	Command_m = atoi(&CommandKey_m);
	if (Command_m != 0) {
		xil_printf("You have selected command %d\r\n", Command_m);
	}

	if (CommandKey_m == 'y') {
		set_phy = 1;
	} else {
		set_phy = 0;
	}
#ifdef versal
	xil_printf ("Compliance mode not supported for Versal\r\n");
	set_phy = 0;
#endif
#ifdef PLATFORM_MB
	xil_printf ("Compliance mode not supported for VCU118\r\n");
	set_phy = 0;
#endif

#endif

	/* Do platform initialization in this function. This is hardware
	 * system specific. It is up to the user to implement this function.
	 */
	xil_printf("PlatformInit\r\n");
	Status = DpTxSs_PlatformInit();
	if (Status != XST_SUCCESS) {
		xil_printf("Platform init failed!\r\n");
	}
	xil_printf("Platform initialization done.\r\n");

#if ENABLE_AUDIO
	// I2C MUX device address : 0x74
	// Si570 device address : 0x5D
	//setting Si570 on zcu102 to be 24.576MHz for audio
#ifndef PLATFORM_MB
#ifdef versal
	I2cMux_Ps(0x40);
#endif
	clk_set(I2C_MUX_ADDR2, IIC_SI570_ADDRESS, audio_clk_Hz);
	I2cMux_Ps(0x04);
#else
	u8 i = 0;
    for( i = 0; i < 6; i++ ) {
            UpdateBuffer[i] = si570_reg_value[2][i];
    }
    Status = write_si570(UpdateBuffer);
    if (Status != XST_SUCCESS) {
	xil_printf ("Failed to program Si750\r\n");
    }
#endif
#endif

	VideoFMC_Init();

	IDT_8T49N24x_SetClock(IIC_BASE_ADDR, I2C_IDT8N49_ADDR, 0,270000000, TRUE);
//
//	//Keeping 0db gain on RX
//	//Adding 6db gain on TX
	i2c_write_dp141(IIC_BASE_ADDR, I2C_TI_DP141_ADDR, 0x02, 0x3C);
	i2c_write_dp141(IIC_BASE_ADDR, I2C_TI_DP141_ADDR, 0x05, 0x3C);
	i2c_write_dp141(IIC_BASE_ADDR, I2C_TI_DP141_ADDR, 0x08, 0x3C);
	i2c_write_dp141(IIC_BASE_ADDR, I2C_TI_DP141_ADDR, 0x0B, 0x3C);

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

#ifdef versal
	/*Wait till gtpowergood for accessing gt and its registers*/
	while(!((XGpio_DiscreteRead(&Gpio, 1)) & 0x1));

#if (VERSAL_FABRIC_8B10B == 1)
	//Unlocking NPI space to modify GT parameters
	XDp_WriteReg(GT_QUAD_BASE, 0xC, 0xF9E8D7C6);
	ReadVal = XDp_ReadReg(GT_QUAD_BASE, TXCLKDIV_REG);
	ReadVal &= ~DIV_MASK;
	ReadVal |= DIV;
	XDp_WriteReg(GT_QUAD_BASE, TXCLKDIV_REG, ReadVal);
#endif
#endif

	/* Setup Video Phy, left to the user for implementation */
#ifndef SDT
	DpTxSs_PhyInit(XVPHY_DEVICE_ID);
#else
#ifndef versal
	DpTxSs_PhyInit(XPAR_XVPHY_0_BASEADDR);
#else
	DpTxSs_PhyInit(0);
#endif
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

#ifndef versal
	XVphy_BufgGtReset(&VPhyInst, XVPHY_DIR_TX,(FALSE));
	// This configures the vid_phy for line rate to start with
	//Even though CPLL can be used in limited case,
	//using QPLL is recommended for more coverage.
#else
#endif
	Status = config_phy(LineRate_init_tx);
	LaneCount_init_tx = LaneCount_init_tx & 0x7;
	//800x600 8bpc as default
#if !PHY_COMP
	start_tx (LineRate_init_tx, LaneCount_init_tx,user_config);
	// Enabling TX interrupts
#endif

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
	if (Status != XST_SUCCESS){
		xil_printf("ERR:Timer failed to initialize. \r\n");
		return XST_FAILURE;
	}
	// Set up timer options.
	XTmrCtr_SetResetValue(&TmrCtr, XTC_TIMER_0, TIMER_RESET_VALUE);
	XTmrCtr_Start(&TmrCtr, XTC_TIMER_0);
#endif
#ifndef PLATFORM_MB
#ifdef versal
     /*
      * Get the CLK_WIZ Dynamic reconfiguration driver instance
      */
#ifndef SDT
     CfgPtr_Dynamic = XClk_Wiz_LookupConfig(XPAR_CLK_WIZ_0_DEVICE_ID);
#else
     CfgPtr_Dynamic = XClk_Wiz_LookupConfig(XPAR_XCLK_WIZ_0_BASEADDR);
#endif
     if (!CfgPtr_Dynamic) {
             return XST_FAILURE;
     }

     /*
      * Initialize the CLK_WIZ Dynamic reconfiguration driver
      */
     Status = XClk_Wiz_CfgInitialize(&ClkWiz_Dynamic, CfgPtr_Dynamic,
              CfgPtr_Dynamic->BaseAddr);
     if (Status != XST_SUCCESS) {
             return XST_FAILURE;
     }
	/* Initialize the GPIO driver */
#ifndef SDT
	Status = XGpio_Initialize(&Gpio, XPAR_GPIO_1_DEVICE_ID);
#else
	Status = XGpio_Initialize(&Gpio, XPAR_XGPIO_1_BASEADDR);
#endif
	if (Status != XST_SUCCESS) {
		xil_printf("Gpio Initialization Failed\r\n");
		return XST_FAILURE;
	}
	/* Set the direction for gpio channel 1 bit 0 as input */
	XGpio_SetDataDirection(&Gpio, 1, 0x01);
#endif
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

#ifndef versal
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
	if (ConfigPtr_IIC == NULL) {
            return XST_FAILURE;
	}
#endif
    Status = XIic_CfgInitialize(&IicInstance, ConfigPtr_IIC,
											ConfigPtr_IIC->BaseAddress);
    if (Status != XST_SUCCESS) {
            return XST_FAILURE;
    }
#endif

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


	/* Set custom timer wait */
//	XDpRxSs_SetUserTimerHandler(&DpRxSsInst, &CustomWaitUs, &TmrCtr);

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
#ifndef versal
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

#if TX_BUFFER_BYPASS
	XVphy_DrpWr(&VPhyInst, 0, XVPHY_CHANNEL_ID_CH1, 0x3E, DIVIDER_540);
	XVphy_DrpWr(&VPhyInst, 0, XVPHY_CHANNEL_ID_CH2, 0x3E, DIVIDER_540);
	XVphy_DrpWr(&VPhyInst, 0, XVPHY_CHANNEL_ID_CH3, 0x3E, DIVIDER_540);
	XVphy_DrpWr(&VPhyInst, 0, XVPHY_CHANNEL_ID_CH4, 0x3E, DIVIDER_540);
#endif

	PHY_Two_byte_set (&VPhyInst, SET_TX_TO_2BYTE);
#else
	//set vswing of value of 5
	ReadModifyWrite(0x1F00,(5 << 8));
//      releasing reset. bit[0] = > 0
	ReadModifyWrite(0x1, (0 << 0));
	u32 dptx_sts = 0;
	u32 retry=0;
	dptx_sts = 0;
	//Checking the status for 4 lanes
	while ((dptx_sts != ALL_LANE) && retry < 10000) {
	   dptx_sts = XDp_ReadReg(DpTxSsInst.DpPtr->Config.BaseAddr, 0x280);
	   dptx_sts &= ALL_LANE;
	   retry++;
	}
	if(retry==10000)
	{
	xil_printf ("+\r\n");
			//This reset is needed for Versal GT. Sometimes the GT does not come out
			//automatically and needs a reset
			//This is because the refclk is not present at start
			ReadModifyWrite(0x1, (1 << 0));
			ReadModifyWrite(0x1, (0 << 0));
	}

	dptx_sts = 0;
	retry=0;
	//Checking the status for 4 lanes
	while ((dptx_sts != ALL_LANE) && retry < 10000) {
	   dptx_sts = XDp_ReadReg(DpTxSsInst.DpPtr->Config.BaseAddr, 0x280);
	   dptx_sts &= ALL_LANE;
	   retry++;
	}

	if(retry==10000)
	{
	   prev_line_rate = XDP_TX_LINK_BW_SET_162GBPS;
	   xil_printf (
		"+++++++ TX GT configuration encountered a failure init2 +++++++ \r\n");
	//	return XST_FAILURE;
	} else {
	   prev_line_rate = XDP_TX_LINK_BW_SET_540GBPS;
	//    	xil_printf ("second time pass %x\r\n",dptx_sts1);
	}
#endif
	return XST_SUCCESS;
}

#ifndef versal
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
											DP159_FORWARDED_CLK, 81000000);
			XVphy_CfgLineRate(InstancePtr, 0,
						XVPHY_CHANNEL_ID_CHA, XVPHY_DP_LINK_RATE_HZ_162GBPS);
			XVphy_CfgLineRate(InstancePtr, 0,
						XVPHY_CHANNEL_ID_CMN1, XVPHY_DP_LINK_RATE_HZ_162GBPS);
			break;
	case XDP_TX_LINK_BW_SET_540GBPS:
		XVphy_CfgQuadRefClkFreq(InstancePtr, 0,
										ONBOARD_REF_CLK, 270000000);
			XVphy_CfgQuadRefClkFreq(InstancePtr, 0,
											DP159_FORWARDED_CLK, 270000000);
			XVphy_CfgLineRate(InstancePtr, 0,
						XVPHY_CHANNEL_ID_CHA, XVPHY_DP_LINK_RATE_HZ_540GBPS);
			XVphy_CfgLineRate(InstancePtr, 0,
						XVPHY_CHANNEL_ID_CMN1, XVPHY_DP_LINK_RATE_HZ_540GBPS);
			break;
	case XDP_TX_LINK_BW_SET_810GBPS:
		XVphy_CfgQuadRefClkFreq(InstancePtr, 0,
										ONBOARD_REF_CLK, 270000000);
			XVphy_CfgQuadRefClkFreq(InstancePtr, 0,
											DP159_FORWARDED_CLK, 270000000);
			XVphy_CfgLineRate(InstancePtr, 0,
						XVPHY_CHANNEL_ID_CHA, XVPHY_DP_LINK_RATE_HZ_810GBPS);
			XVphy_CfgLineRate(InstancePtr, 0,
						XVPHY_CHANNEL_ID_CMN1, XVPHY_DP_LINK_RATE_HZ_810GBPS);
			break;
	default:
		XVphy_CfgQuadRefClkFreq(InstancePtr, 0,
										ONBOARD_REF_CLK, 270000000);
			XVphy_CfgQuadRefClkFreq(InstancePtr, 0,
											DP159_FORWARDED_CLK, 135000000);
			XVphy_CfgLineRate(InstancePtr, 0,
						XVPHY_CHANNEL_ID_CHA, XVPHY_DP_LINK_RATE_HZ_270GBPS);
			XVphy_CfgLineRate(InstancePtr, 0,
						XVPHY_CHANNEL_ID_CMN1, XVPHY_DP_LINK_RATE_HZ_270GBPS);
			break;
	}
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
void PHY_Two_byte_set (XVphy *InstancePtr, u8 Tx_to_two_byte)
{

	u16 DrpVal;
	u16 WriteVal;
	u32 Status;
	u16 TX_DATA_WIDTH_REG = 0x7A;
	u16 TX_INT_DATAWIDTH_REG = 0x85;

    if (Tx_to_two_byte == 1) {


		Status = XVphy_DrpRd(InstancePtr, 0, XVPHY_CHANNEL_ID_CH1,
											TX_DATA_WIDTH_REG, &DrpVal);

		if(Status != XST_SUCCESS){
			xil_printf("DRP access failed\r\n");
			return;
		}
		DrpVal &= ~0xF;
		WriteVal = 0x0;
		WriteVal = DrpVal | 0x3;
		Status  =XVphy_DrpWr(InstancePtr, 0, XVPHY_CHANNEL_ID_CH1,
												TX_DATA_WIDTH_REG, WriteVal);
		Status +=XVphy_DrpWr(InstancePtr, 0, XVPHY_CHANNEL_ID_CH2,
												TX_DATA_WIDTH_REG, WriteVal);
		Status +=XVphy_DrpWr(InstancePtr, 0, XVPHY_CHANNEL_ID_CH3,
												TX_DATA_WIDTH_REG, WriteVal);
		Status +=XVphy_DrpWr(InstancePtr, 0, XVPHY_CHANNEL_ID_CH4,
												TX_DATA_WIDTH_REG, WriteVal);
		if(Status != XST_SUCCESS){
			xil_printf("DRP access failed\r\n");
			return;
		}

		Status = XVphy_DrpRd(InstancePtr, 0, XVPHY_CHANNEL_ID_CH1,
											TX_INT_DATAWIDTH_REG, &DrpVal);
		if(Status != XST_SUCCESS){
			xil_printf("DRP access failed\r\n");
			return;
		}

		DrpVal &= ~0xC00;
		WriteVal = 0x0;
		WriteVal = DrpVal | 0x0;
		Status  =XVphy_DrpWr(InstancePtr, 0, XVPHY_CHANNEL_ID_CH1,
										TX_INT_DATAWIDTH_REG, WriteVal);
		Status +=XVphy_DrpWr(InstancePtr, 0, XVPHY_CHANNEL_ID_CH2,
										TX_INT_DATAWIDTH_REG, WriteVal);
		Status +=XVphy_DrpWr(InstancePtr, 0, XVPHY_CHANNEL_ID_CH3,
										TX_INT_DATAWIDTH_REG, WriteVal);
		Status +=XVphy_DrpWr(InstancePtr, 0, XVPHY_CHANNEL_ID_CH4,
										TX_INT_DATAWIDTH_REG, WriteVal);
		if(Status != XST_SUCCESS){
			xil_printf("DRP access failed\r\n");
			return;
		} else {

		}
		xil_printf ("TX Channel configured for 2byte mode\r\n");
    }


}
#endif

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
    lanes = XPAR_TX_SUBSYSTEM_V_DP_TXSS1_0_LANE_COUNT;//get_Lanecounts();

	// If the requested rate is same, do not re-program.
	if (rate != prev_line_rate) {
		config_phy(rate);
	}
	//update the previous link rate info at here
	prev_line_rate = rate;
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

#ifdef versal
//		vswing = (diff_swing << 16) | diff_swing;
//		postcursor = (preemp << 26) | (preemp << 10);
//		value = vswing | postcursor;

		ReadModifyWrite(0x1F00 ,(diff_swing << 8));
		ReadModifyWrite(0x7C0000,(preemp << 18));
#else
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

#if PHY_COMP
#if 0
		if (set_phy == 1) {
		xil_printf ("Setting Vswing (GT) = %d\r\n",diff_swing);
		xil_printf ("Setting Preemp (GT) = %d\r\n",preemp);
		} else {
			xil_printf ("Vswing requested is %d\r\n",
					DpTxSsInst.DpPtr->TxInstance.LinkConfig.VsLevel);
			xil_printf ("Preemp requested is %d\r\n",
					DpTxSsInst.DpPtr->TxInstance.LinkConfig.PeLevel);
		}
#endif
#endif
#endif
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
	if (XDpTxSs_IsConnected(&DpTxSsInst)) { // && tx_is_reconnected == 0) {
		sink_power_down();
		sink_power_up();
		tx_is_reconnected++;// = 1; //++;
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
#if !PHY_COMP
			XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr,
					XDP_TX_INTERRUPT_MASK,
					XDP_TX_INTERRUPT_MASK_HPD_PULSE_DETECTED_MASK);
#endif
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
             && bw_set != XDP_TX_LINK_BW_SET_540GBPS){
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
//		XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr, XDP_TX_ENABLE, 0x0);
		retrain_link = 1;
	}

	if(retrain_link == 1){
//		sink_power_cycle();
		XDpTxSs_SetLinkRate(&DpTxSsInst, bw_set);
		XDpTxSs_SetLaneCount(&DpTxSsInst, lane_set);
		XDpTxSs_Start(&DpTxSsInst);

		if(DpTxSsInst.DpPtr->TxInstance.ColorimetryThroughVsc){
			Readval=XDp_ReadReg(DpTxSsInst.DpPtr->Config.BaseAddr,
					XDP_TX_MAIN_STREAM_MISC0);
			Readval=(Readval|(1<<12));
			XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr,
					XDP_TX_MAIN_STREAM_MISC0,Readval);
		}
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


#if ENABLE_AUDIO
    XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr, XDP_TX_AUDIO_CONTROL, 0x0);
#endif
}


#ifndef versal
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
	//XVphy_ChannelId RxChId;
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
			xil_printf ("exhausted\r\n");
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
	Status += XVphy_WaitForPllLock(InstancePtr, QuadId, TxChId);
	Status += XVphy_WaitForResetDone(InstancePtr, QuadId,
						XVPHY_CHANNEL_ID_CHA, XVPHY_DIR_TX);
	if (Status  != XST_SUCCESS) {
		xil_printf ("++++TX GT config encountered error++++\r\n");
	}

	return (Status);
}
#endif

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

	//Disabling TX interrupts
//	XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr,
//												XDP_TX_INTERRUPT_MASK, 0xFFF);
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
    //Populate Color format and BPC to vsc packet
    if(DpTxSsInst.DpPtr->TxInstance.ColorimetryThroughVsc){
			u32 data=0;
			data|=((user_config.user_format)<<COLOR_FORMAT_SHIFT);
			if(user_config.user_bpc==6)
				data|=(0<<BPC_SHIFT);
			else if(user_config.user_bpc==8)
				data|=(1<<BPC_SHIFT);
			else if(user_config.user_bpc==10)
				data|=(2<<BPC_SHIFT);
			else if(user_config.user_bpc==12)
				data|=(3<<BPC_SHIFT);
			else if(user_config.user_bpc==16)
				data|=(4<<BPC_SHIFT);
			data|=(1<<DYNAMIC_RANGE_SHIFT);
			VscPkt.Payload[4]=data;
			XDpTxSs_SetVscExtendedPacket(&DpTxSsInst, VscPkt);
    }

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
			format, XVIDC_BT_601, XDP_DR_CEA);

	if (set_phy == 0) {
		Status = DpTxSubsystem_Start(&DpTxSsInst, 0);

		/* When sending colorimetry info through VSC
		 * Setting bit[12] of Reg 0x1A4 to 1
		 * This ensures that VSC pkt is sent every frame
		 */

		if(DpTxSsInst.DpPtr->TxInstance.ColorimetryThroughVsc){
			Readval = XDp_ReadReg(DpTxSsInst.DpPtr->Config.BaseAddr,
				XDP_TX_MAIN_STREAM_MISC0);
			Readval = (Readval|(1<<12));
			XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr,
				XDP_TX_MAIN_STREAM_MISC0,Readval);
		}

	if (Status != XST_SUCCESS) {
#if !PHY_COMP
		//Compliance tests do some crazy things.
		//Keeping this out for compliance
		xil_printf("ERR:1DPTX SS start failed\r\n");
		return (XST_FAILURE);
#endif
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
//				 xdbg_printf(XDBG_DEBUG_GENERAL,"SS ERR: "
//						 "VTC%d setup failed!\n\r", Index);
         }
    }

	if (set_phy == 0) {
    Status = XDpTxSs_CheckLinkStatus(&DpTxSsInst);
	if (Status != (XST_SUCCESS)) {
		Status = DpTxSubsystem_Start(&DpTxSsInst, 0);
		if (Status != XST_SUCCESS) {
#if !PHY_COMP
			//Compliance tests do some crazy things.
			//Keeping this out for compliance
			xil_printf("ERR:DPTX SS start failed\r\n");
			return (XST_FAILURE);
#endif
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
			0x4);
    } else {
	XDp_WriteReg(XPAR_TX_SUBSYSTEM_CRC_BASEADDR, VIDEO_FRAME_CRC_CONFIG,
			0x4 | 0x80000000);
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
void clk_wiz_locked(void) {
	volatile u32 res = XDp_ReadReg(XPAR_XGPIO_0_BASEADDR,0x0);
	u32 timer=0;

	while ( res == 0 && timer < 1000) {
		xil_printf ("~/~/");
		res = XDp_ReadReg(XPAR_XGPIO_0_BASEADDR,0x0);
		timer++; // timer for timeout. No need to be specific time.
					// As long as long enough to wait lock
	}
	xil_printf ("^^");
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

	// All the compliance related DPCD reads have been moved to driver

	u32 Status=XST_SUCCESS;
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

	user_config_struct user_config;



	XDp_ReadReg(DpTxSsInst.DpPtr->Config.BaseAddr,XDP_TX_INTERRUPT_STATUS);
	//Enabling TX interrupts
//	XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr,XDP_TX_INTERRUPT_MASK,0xFFF);

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

	// check if line speed is either 0x6, 0xA, 0x14, 0x1E
	if (max_cap_new != XDP_TX_LINK_BW_SET_810GBPS
			&& max_cap_new != XDP_TX_LINK_BW_SET_540GBPS
				&& max_cap_new != XDP_TX_LINK_BW_SET_270GBPS
				&& max_cap_new != XDP_TX_LINK_BW_SET_162GBPS) {
		// soemthing wrong. Read again
		XDp_TxAuxRead(DpTxSsInst.DpPtr, XDP_DPCD_MAX_LINK_RATE, 1,
														&max_cap_new);
	}

	if (max_cap_new != XDP_TX_LINK_BW_SET_810GBPS
			&& max_cap_new != XDP_TX_LINK_BW_SET_540GBPS
				&& max_cap_new != XDP_TX_LINK_BW_SET_270GBPS
				&& max_cap_new != XDP_TX_LINK_BW_SET_162GBPS) {

		max_cap_new = XDP_TX_LINK_BW_SET_810GBPS;
	}
	/**********************************************************/
	/* Since the CTS suite is for 5.4G, we cannot go for 8.1
	 * hence setting max_cap_new to 5.4G, when DPCD read is 8.1
	 */
//	if (max_cap_new >= XDP_TX_LINK_BW_SET_540GBPS) {
//		max_cap_new = XDP_TX_LINK_BW_SET_540GBPS;
//	}
	/**********************************************************/

	res_update = XVIDC_VM_640x480_60_P;

	if (good_edid_hpd == 1) {
		htotal_test_hpd = XVidC_EdidGetStdTimingsH(Edid_org, 1);
		vtotal_test_hpd = XVidC_EdidGetStdTimingsV(Edid_org, 1);
		freq_test_hpd   = XVidC_EdidGetStdTimingsFrr(Edid_org, 1);
//		XVidC_UnregisterCustomTimingModes();
		VmId_test_hpd = XVidC_GetVideoModeId(htotal_test_hpd, vtotal_test_hpd,
															freq_test_hpd,0);
		VmId_ptm_hpd = GetPreferredVm(Edid_org, max_cap_new ,
														max_cap_lanes_new&0x1F);
		bpc_hpd = XVidC_EdidGetColorDepth(Edid_org);
#ifndef SDT
	if(bpc_hpd > XPAR_TX_SUBSYSTEM_V_DP_TXSS1_0_DP_MAX_BITS_PER_COLOR)
			bpc_hpd=XPAR_TX_SUBSYSTEM_V_DP_TXSS1_0_DP_MAX_BITS_PER_COLOR;
#else

		if(bpc_hpd > XPAR_DPTXSS_0_BITS_PER_COLOR)
			bpc_hpd=XPAR_DPTXSS_0_BITS_PER_COLOR;
#endif
//		xil_printf ("BPC from EDID is %d\r\n",bpc_hpd);
		if (VmId_ptm_hpd == XVIDC_VM_NOT_SUPPORTED) { //Fail Safe mode
			VmId_ptm_hpd = XVIDC_VM_640x480_60_P;
//			res_update = XVIDC_VM_640x480_60_P;
			bpc_hpd = 6;
		}

	} else {
		VmId_test_hpd = XVIDC_VM_NOT_SUPPORTED;
		VmId_ptm_hpd = XVIDC_VM_640x480_60_P;
//		res_update = XVIDC_VM_640x480_60_P;
		bpc_hpd = 6;
		good_edid_hpd = 0;
	}


	if (max_cap_new == XDP_TX_LINK_BW_SET_810GBPS
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

#if PHY_COMP
        // UCD400 seems to have some issues with the EDID due to which
        // some of the compliance tests fail. Hence setting bpc=8 and
        // resolution to 640x480
		user_config.user_bpc=8;
		user_config.VideoMode_local=XVIDC_VM_640x480_60_P;
#else
		user_config.user_bpc=bpc_hpd;
		user_config.VideoMode_local=VmId_ptm_hpd;
#endif
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
		FrameRate = XVIDC_FR_60HZ; //60;
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
	return DpTxSsInst.DpPtr->TxInstance.LinkConfig.LinkRate;
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

#if TX_BUFFER_BYPASS

	switch(LineRate_init_tx){
		case XDP_TX_LINK_BW_SET_162GBPS:
			XVphy_DrpWr(&VPhyInst, 0, XVPHY_CHANNEL_ID_CH1, 0x3E, DIVIDER_162); //57423);
			XVphy_DrpWr(&VPhyInst, 0, XVPHY_CHANNEL_ID_CH2, 0x3E, DIVIDER_162); //57423);
			XVphy_DrpWr(&VPhyInst, 0, XVPHY_CHANNEL_ID_CH3, 0x3E, DIVIDER_162); //57423);
			XVphy_DrpWr(&VPhyInst, 0, XVPHY_CHANNEL_ID_CH4, 0x3E, DIVIDER_162); //57423);

			break;

		case XDP_TX_LINK_BW_SET_270GBPS:
			XVphy_DrpWr(&VPhyInst, 0, XVPHY_CHANNEL_ID_CH1, 0x3E, DIVIDER_270); //57415);
			XVphy_DrpWr(&VPhyInst, 0, XVPHY_CHANNEL_ID_CH2, 0x3E, DIVIDER_270); //57415);
			XVphy_DrpWr(&VPhyInst, 0, XVPHY_CHANNEL_ID_CH3, 0x3E, DIVIDER_270); //57415);
			XVphy_DrpWr(&VPhyInst, 0, XVPHY_CHANNEL_ID_CH4, 0x3E, DIVIDER_270); //57415);

			break;

		case XDP_TX_LINK_BW_SET_540GBPS:
			XVphy_DrpWr(&VPhyInst, 0, XVPHY_CHANNEL_ID_CH1, 0x3E, DIVIDER_540); //57442);
			XVphy_DrpWr(&VPhyInst, 0, XVPHY_CHANNEL_ID_CH2, 0x3E, DIVIDER_540); //57442);
			XVphy_DrpWr(&VPhyInst, 0, XVPHY_CHANNEL_ID_CH3, 0x3E, DIVIDER_540); //57442);
			XVphy_DrpWr(&VPhyInst, 0, XVPHY_CHANNEL_ID_CH4, 0x3E, DIVIDER_540); //57442);

			break;

		case XDP_TX_LINK_BW_SET_810GBPS:
			XVphy_DrpWr(&VPhyInst, 0, XVPHY_CHANNEL_ID_CH1, 0x3E, DIVIDER_810); //57440);
			XVphy_DrpWr(&VPhyInst, 0, XVPHY_CHANNEL_ID_CH2, 0x3E, DIVIDER_810); //57440);
			XVphy_DrpWr(&VPhyInst, 0, XVPHY_CHANNEL_ID_CH3, 0x3E, DIVIDER_810); //57440);
			XVphy_DrpWr(&VPhyInst, 0, XVPHY_CHANNEL_ID_CH4, 0x3E, DIVIDER_810); //57440);

			break;
	}

#endif

	switch(LineRate_init_tx){

		case XDP_TX_LINK_BW_SET_162GBPS:

#ifndef versal
			Status = PHY_Configuration_Tx(&VPhyInst,
						PHY_User_Config_Table[(is_TX_CPLL)?0:3]);
#else
			linerate = VERSAL_162G;
#endif
			break;

		case XDP_TX_LINK_BW_SET_270GBPS:
#ifndef versal
			Status = PHY_Configuration_Tx(&VPhyInst,
						PHY_User_Config_Table[(is_TX_CPLL)?1:4]);
#else
			linerate = VERSAL_270G;
#endif
			break;

		case XDP_TX_LINK_BW_SET_540GBPS:
#ifndef versal
			Status = PHY_Configuration_Tx(&VPhyInst,
						PHY_User_Config_Table[(is_TX_CPLL)?2:5]);
#else
			linerate = VERSAL_540G;
#endif
			break;

		case XDP_TX_LINK_BW_SET_810GBPS:
#ifndef versal
			Status = PHY_Configuration_Tx(&VPhyInst,
						PHY_User_Config_Table[(is_TX_CPLL)?9:10]);
#else
			linerate = VERSAL_810G;
#endif
			break;
	}


#ifdef versal
	ReadModifyWrite(0xE,(linerate << 1));
    u8 retry=0;
    while ((dptx_sts != ALL_LANE) && retry < 255) {
         dptx_sts = XDp_ReadReg(DpTxSsInst.DpPtr->Config.BaseAddr, 0x280);
         dptx_sts &= ALL_LANE;
         DpPt_CustomWaitUs(DpTxSsInst.DpPtr, 100);
         retry++;
      }
    if(retry==255)
    {
	Status = XST_FAILURE;
    }

#endif

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
//	// give enough time for monitor to wake up    CR-962717
	usleep(30000);
	sink_power_up();//monitor to wake up once again due to CR-962717
	usleep(4000);
}


int VideoFMC_Init(void){
	int Status;
	u8 Buffer[2];
	int ByteCount= 0;

	xil_printf("VFMC: Setting IO Expanders...\n\r");

#ifndef versal
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
#endif
	/* Configure VFMC IO Expander 0:
	 * Disable Si5344
	 * Set primary clock source for LMK03318 to IOCLKp(0)
	 * Set secondary clock source for LMK03318 to IOCLKp(1)
	 * Disable LMK61E2*/
	Buffer[0] = 0x52;

#ifdef versal
    Status = XIicPs_MasterSendPolled(&Ps_Iic0,
            (u8 *)&Buffer,
            1,
			 I2C_VFMCEXP_0_ADDR);
    if(Status == XST_SUCCESS)
    {
	ByteCount=1;
    }
    else
    {
	ByteCount=0;
    }
#else
	ByteCount = XIic_Send(IIC_BASE_ADDR, I2C_VFMCEXP_0_ADDR, (u8*)Buffer, 1, XIIC_STOP);
#endif
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
#ifdef versal
    Status = XIicPs_MasterSendPolled(&Ps_Iic0,
             (u8 *)&Buffer,
             1,
			  I2C_VFMCEXP_1_ADDR);
    if(Status == XST_SUCCESS)
    {
	ByteCount=1;
    }
    else
    {
	ByteCount=0;
    }
#else
	ByteCount = XIic_Send(IIC_BASE_ADDR, I2C_VFMCEXP_1_ADDR, (u8*)Buffer, 1, XIIC_STOP);
#endif
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

	return XST_SUCCESS;
}

#ifndef versal
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





u8 i2c_read_dp141(u32 I2CBaseAddress, u8 I2CSlaveAddress, u16 RegisterAddress)
{
	u32 ByteCount = 0;
	u8 Buffer[1];
	u8 Data;
	u8 Retry = 0;
	u8 Exit;


	Exit = FALSE;
	Data = 0;

	do {
		// Set Address
		Buffer[0] = RegisterAddress & 0xff;
		ByteCount = XIic_Send(I2CBaseAddress, I2CSlaveAddress, (u8*)Buffer, 1, XIIC_REPEATED_START);

		if (ByteCount != 1) {
			Retry++;

			// Maximum retries
			if (Retry == 255) {
				Exit = TRUE;
			}
		}

		// Read data
		else {
			//Read data
			ByteCount = XIic_Recv(I2CBaseAddress, I2CSlaveAddress, (u8*)Buffer, 1, XIIC_STOP);

				Data = Buffer[0];
				Exit = TRUE;
		}
	} while (!Exit);

	return Data;
}
#endif

void read_DP141(){
	u8 Data;
	int i =0;

#ifndef versal
	for(i=0; i<0xD; i++){
		Data = i2c_read_dp141( IIC_BASE_ADDR, I2C_TI_DP141_ADDR, i);
		xil_printf("%x : %02x \r\n",i, Data);
	}
#else
    u8 Buffer;
    u32 Status;
    Buffer = 0x02 & 0xff;
    Status = XIicPs_MasterSendPolled(&Ps_Iic0,
                                             (u8 *)&Buffer,
                                             1,
											 I2C_TI_DP141_ADDR);



    XIicPs_MasterRecvPolled(&Ps_Iic0, &Data, 1, I2C_TI_DP141_ADDR);
#endif

}
