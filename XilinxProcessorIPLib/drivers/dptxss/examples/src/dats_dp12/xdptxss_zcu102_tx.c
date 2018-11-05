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
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver  Who Date     Changes
* ---- --- -------- --------------------------------------------------
* 1.00 KI  07/13/17 Initial release.
*
* </pre>
*
******************************************************************************/


#include "xdptxss_zcu102_tx.h"
#include "clk_set.h"


// adding new resolution definition example
// XVIDC_VM_3840x2160_30_P_SB, XVIDC_B_TIMING3_60_P_RB
// and XVIDC_VM_3840x2160_60_P_RB has added
typedef enum {
    XVIDC_VM_1920x1080_60_P_RB = (XVIDC_VM_CUSTOM + 1),
	XVIDC_B_TIMING3_60_P_RB ,
    XVIDC_CM_NUM_SUPPORTED
} XVIDC_CUSTOM_MODES;

// CUSTOM_TIMING: Here is the detailed timing for each custom resolutions.
const XVidC_VideoTimingMode XVidC_MyVideoTimingMode[
					(XVIDC_CM_NUM_SUPPORTED - (XVIDC_VM_CUSTOM + 1))] =
{
    { XVIDC_VM_1920x1080_60_P_RB, "1920x1080@60Hz (RB)", XVIDC_FR_60HZ,
        {1920, 48, 32, 80, 2080, 1,
         1080, 3, 5, 23, 1111, 0, 0, 0, 0, 0}
    },

    { XVIDC_B_TIMING3_60_P_RB, "2560x1440@60Hz (RB)", XVIDC_FR_60HZ,
         {2560, 48, 32, 80, 2720, 1,
          1440, 3, 5, 33, 1481, 0, 0, 0, 0, 0}
    },
};

XVidC_VideoMode resolution_table[] =
{
                XVIDC_VM_640x480_60_P,
                XVIDC_VM_480_60_P,
                XVIDC_VM_800x600_60_P,
                XVIDC_VM_1024x768_60_P,
                XVIDC_VM_720_60_P,
                XVIDC_VM_1600x1200_60_P,
                XVIDC_VM_1366x768_60_P,
                XVIDC_VM_1080_60_P,
                XVIDC_VM_UHD_30_P,
                XVIDC_VM_UHD_60_P,
                XVIDC_VM_2560x1600_60_P,
                XVIDC_VM_1280x1024_60_P,
                XVIDC_VM_1792x1344_60_P,
                XVIDC_VM_848x480_60_P,
                XVIDC_VM_1280x960_60_P,
                XVIDC_VM_1920x1440_60_P,
                XVIDC_VM_USE_EDID_PREFERRED,
				//CUSTOM_TIMING: User need to add new timing here
				XVIDC_VM_1920x1080_60_P_RB,
				//CUSTOM_TIMING: User need to add new timing here
				XVIDC_VM_3840x2160_60_P_RB
};



#define I2C_MUX_device_address 0x74
#define Si570_device_address 0x5D
#define audio_clk_Hz 24.576


/************************** Function Prototypes ******************************/

u32 DpTxSs_Main(u16 DeviceId);
u32 DpTxSs_PlatformInit(void);


/* Interrupt helper functions */
void DpPt_HpdEventHandler(void *InstancePtr);
void DpPt_HpdPulseHandler(void *InstancePtr);
void DpPt_LinkrateChgHandler (void *InstancePtr);

u32 DpTxSs_SetupIntrSystem(void);
u32 DpTxSs_VideoPhyInit(u16 DeviceId);
void DpPt_CustomWaitUs(void *InstancePtr, u32 MicroSeconds);
u32 DpTxSubsystem_Start(XDpTxSs *InstancePtr, int with_msa);
void DpTxSs_Setup(u8 *LineRate_init, u8 *LaneCount_init,
										u8 Edid_org[128], u8 Edid1_org[128]);
void PLLRefClkSel (XVphy *InstancePtr, u8 link_rate);
void PHY_Two_byte_set (XVphy *InstancePtr, u8 Rx_to_two_byte);
void clk_wiz_locked(void);
void hpd_pulse_con(XDpTxSs *InstancePtr);
int Vpg_StreamSrcConfigure(XDp *InstancePtr, u8 VSplitMode, u8 first_time);
void Vpg_VidgenSetUserPattern(XDp *InstancePtr, u8 Pattern);
void LMK04906_init(XSpi *SPI_LMK04906);
static u8 CalculateChecksum(u8 *Data, u8 Size);
XVidC_VideoMode GetPreferredVm(u8 *EdidPtr, u8 cap, u8 lane);
void ReportVideoCRC(void);
extern void main_loop(void);

/************************** Variable Definitions *****************************/

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

	xil_printf("------------------------------------------\r\n");
	xil_printf("DisplayPort TX Subsystem Example Design\r\n");
	xil_printf("(c) 2017 by Xilinx\r\n");
	xil_printf("-------------------------------------------\r\n\r\n");

	Status = DpTxSs_Main(XDPTXSS_DEVICE_ID);
	if (Status != XST_SUCCESS) {
		xil_printf("DisplayPort TX Subsystem interrupt example failed.");
		return XST_FAILURE;
	}

	xil_printf(
			"Successfully ran DisplayPort TX Subsystem interrupt example\r\n");

	return XST_SUCCESS;
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
u32 DpTxSs_Main(u16 DeviceId)
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
	user_config.user_pattern = 1; /*Color Ramp (Default)*/
	user_config.user_format = XVIDC_CSF_RGB;


	// Adding custom resolutions at here.
	xil_printf("INFO> Registering Custom Timing Table with %d entries \r\n",
							(XVIDC_CM_NUM_SUPPORTED - (XVIDC_VM_CUSTOM + 1)));
	Status = XVidC_RegisterCustomTimingModes(XVidC_MyVideoTimingMode,
							(XVIDC_CM_NUM_SUPPORTED - (XVIDC_VM_CUSTOM + 1)));
	if (Status != XST_SUCCESS) {
		xil_printf("ERR: Unable to register custom timing table\r\r\n\n");
	}



	/* Do platform initialization in this function. This is hardware
	 * system specific. It is up to the user to implement this function.
	 */
	xil_printf("PlatformInit\r\n");
	Status = DpTxSs_PlatformInit();
	if (Status != XST_SUCCESS) {
		xil_printf("Platform init failed!\r\n");
	}
	xil_printf("Platform initialization done.\r\n");


	XSpi SPI_LMK04906;  /* SPI Device Point*/

	LMK04906_init(&SPI_LMK04906);
	PLL_init_Seting(&SPI_LMK04906, CLK270MHz_DIVIDER);
	xil_printf("Programming 270 MHz Clock for GTREFCLK0...\r\n");

#if ENABLE_AUDIO
	// I2C MUX device address : 0x74
	// Si570 device address : 0x5D
	//setting Si570 on zcu102 to be 24.576MHz for audio
	clk_set(I2C_MUX_device_address, Si570_device_address, audio_clk_Hz);
#endif

	/* Obtain the device configuration for the DisplayPort TX Subsystem */
	ConfigPtr = XDpTxSs_LookupConfig(DeviceId);
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
	DpTxSs_VideoPhyInit(XVPHY_DEVICE_ID);

	DpTxSs_Setup(&LineRate_init, &LaneCount_init, Edid_org, Edid1_org);



	// check if monitor is connected or not
	// This is intentional infinite while loop
    while (!XDpTxSs_IsConnected(&DpTxSsInst)) {
		if (connected == 0) {
			xil_printf(
				"Please connect a DP Monitor to start the application !!!\r\n");
			connected = 1;
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
	Status = set_vphy(LineRate_init_tx);


	LaneCount_init_tx = LaneCount_init_tx & 0x7;
	//800x600 8bpc as default
	start_tx (LineRate_init_tx, LaneCount_init_tx,user_config);
	// Enabling TX interrupts

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

	// Initialize timer.
	Status = XTmrCtr_Initialize(&TmrCtr, XPAR_TMRCTR_0_DEVICE_ID);
	if (Status != XST_SUCCESS){
		xil_printf("ERR:Timer failed to initialize. \r\n");
		return XST_FAILURE;
	}
	// Set up timer options.
	XTmrCtr_SetResetValue(&TmrCtr, XPAR_TMRCTR_0_DEVICE_ID, TIMER_RESET_VALUE);
	XTmrCtr_Start(&TmrCtr, XPAR_TMRCTR_0_DEVICE_ID);


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
u32 DpTxSs_SetupIntrSystem(void)
{
	u32 Status;
	XINTC *IntcInstPtr = &IntcInst;

	/* Set custom timer wait */
	XDpTxSs_SetUserTimerHandler(&DpTxSsInst, &DpPt_CustomWaitUs, &TmrCtr);
	XDpTxSs_SetCallBack(&DpTxSsInst, (XDPTXSS_HANDLER_DP_HPD_EVENT),
						&DpPt_HpdEventHandler, &DpTxSsInst);
	XDpTxSs_SetCallBack(&DpTxSsInst, (XDPTXSS_HANDLER_DP_HPD_PULSE),
						&DpPt_HpdPulseHandler, &DpTxSsInst);
	XDpTxSs_SetCallBack(&DpTxSsInst, (XDPTXSS_HANDLER_DP_LINK_RATE_CHG),
						&DpPt_LinkrateChgHandler, &DpTxSsInst);


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

	/* Enable the interrupt */
	XScuGic_Enable(IntcInstPtr,
			XINTC_DPTXSS_DP_INTERRUPT_ID);

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
u32 DpTxSs_VideoPhyInit(u16 DeviceId)
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

	// initial line Rate setting
	prev_line_rate = PHY_User_Config_Table[5].LineRate;

	PHY_Two_byte_set (&VPhyInst, SET_TX_TO_2BYTE);

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
		}
		xil_printf ("TX Channel configured for 2byte mode\r\n");
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

	rate = get_LineRate();
	// If the requested rate is same, do not re-program.
	if (rate != prev_line_rate) {
		set_vphy(rate);
	}
	//update the previous link rate info at here
	prev_line_rate = rate;
}

/*****************************************************************************/
/**
*
* This function use h/w timer to count specific Microseconds
*
* @param	pointer to timer
* @param	MicroSeconds to wait
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void DpPt_CustomWaitUs(void *InstancePtr, u32 MicroSeconds)
{

	u32 TimerVal, TimerVal_pre;
	u32 NumTicks = (MicroSeconds * (
				XPAR_PROCESSOR_SYSTEM_AXI_TIMER_0_CLOCK_FREQ_HZ / 1000000));

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
		tx_is_reconnected = 1;
	}
	else
	{

		//DpTxSs_DisableAudio
		XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr,
												XDP_TX_AUDIO_CONTROL, 0x0);

		//on HPD d/c, it is important to bring down the HDCP
		tx_is_reconnected = 0;

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


	u8 lane0_sts = InstancePtr->UsrHpdPulseData.Lane0Sts;
	u8 lane2_sts = InstancePtr->UsrHpdPulseData.Lane2Sts;
	u8 laneAlignStatus = InstancePtr->UsrHpdPulseData.LaneAlignStatus;
	u8 bw_set = InstancePtr->UsrHpdPulseData.BwSet;
	u8 lane_set = InstancePtr->UsrHpdPulseData.LaneSet;

     lane_set = lane_set & 0x1F;
     bw_set = bw_set & 0x1F;
     laneAlignStatus = laneAlignStatus & 0x1;

     if(bw_set != XDP_TX_LINK_BW_SET_162GBPS
             && bw_set != XDP_TX_LINK_BW_SET_270GBPS
             && bw_set != XDP_TX_LINK_BW_SET_540GBPS){
             return;
     }
     if(lane_set != XDP_TX_LANE_COUNT_SET_1
             && lane_set != XDP_TX_LANE_COUNT_SET_2
             && lane_set != XDP_TX_LANE_COUNT_SET_4){
             return;
     }

	u8 retrain_link=0;
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

	u8 connected;
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
											 *LineRate_init,*LaneCount_init);


#if ENABLE_AUDIO
    XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr, XDP_TX_AUDIO_CONTROL, 0x0);
#endif
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
	//Disabling TX interrupts

	XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr,
												XDP_TX_INTERRUPT_MASK, 0xFFF);
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
			format, XVIDC_BT_601, XDP_DR_CEA);


	// VTC requires linkup(video clk) before setting values.
	// This is why we need to linkup once to get proper CLK on VTC.
	Status = DpTxSubsystem_Start(&DpTxSsInst, 0);

	xil_printf (".");
	//updates required timing values in Video Pattern Generator
	Vpg_StreamSrcConfigure(DpTxSsInst.DpPtr, 0, 1);
	xil_printf (".");
	// setting video pattern
	Vpg_VidgenSetUserPattern(DpTxSsInst.DpPtr, C_VideoUserStreamPattern[pat]);
	xil_printf (".");
	clk_wiz_locked();
	Status = DpTxSubsystem_Start(&DpTxSsInst, 0);
	xil_printf (".");
        Status = XDpTxSs_CheckLinkStatus(&DpTxSsInst);
	if (Status != (XST_SUCCESS)) {
		Status = DpTxSubsystem_Start(&DpTxSsInst, 0);
		if (Status != XST_SUCCESS) {
			xil_printf("ERR:DPTX SS start failed\r\n");
			return (XST_FAILURE);
		}
	}
	XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr,XDP_TX_INTERRUPT_MASK, 0x0);



	/*
	 * Initialize CRC
	 */
	/* Reset CRC*/
	XVidFrameCrc_Reset();
	/* Set Pixel width in CRC engine*/
	XDp_WriteReg(XPAR_TX_SUBSYSTEM_CRC_BASEADDR, VIDEO_FRAME_CRC_CONFIG,
			XDp_ReadReg(DpTxSsInst.DpPtr->Config.BaseAddr,
					XDP_TX_USER_PIXEL_WIDTH));

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
// as soon as HPD is connected the application reads the EDID to find out
// if the monitor was changed.

void hpd_con(XDpTxSs *InstancePtr, u8 Edid_org[128],
		u8 Edid1_org[128], u16 res_update){


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


	XDp_ReadReg(DpTxSsInst.DpPtr->Config.BaseAddr,XDP_TX_INTERRUPT_STATUS);
	//Enabling TX interrupts
	XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr,XDP_TX_INTERRUPT_MASK,0xFFF);

	memcpy(Edid_org, InstancePtr->UsrHpdEventData.EdidOrg, 128);

	tx_is_reconnected--;

	if (XVidC_EdidIsHeaderValid(Edid_org)) {
		good_edid_hpd = 1;
	}
	else {
		good_edid_hpd = 0;
	}

	if (!CalculateChecksum(Edid_org, 128)) {
		good_edid_hpd = 1;
	} else {
		good_edid_hpd = 0;
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

	// check if line speed is either 0x6, 0xA, 0x14
	if (max_cap_new != XDP_TX_LINK_BW_SET_540GBPS
				&& max_cap_new != XDP_TX_LINK_BW_SET_270GBPS
				&& max_cap_new != XDP_TX_LINK_BW_SET_162GBPS) {
		// soemthing wrong. Read again
		XDp_TxAuxRead(DpTxSsInst.DpPtr, XDP_DPCD_MAX_LINK_RATE, 1,
														&max_cap_new);
	}

	if (good_edid_hpd == 1) {
		htotal_test_hpd = XVidC_EdidGetStdTimingsH(Edid_org, 1);
		vtotal_test_hpd = XVidC_EdidGetStdTimingsV(Edid_org, 1);
		freq_test_hpd   = XVidC_EdidGetStdTimingsFrr(Edid_org, 1);
		XVidC_UnregisterCustomTimingModes();
		VmId_test_hpd = XVidC_GetVideoModeId(htotal_test_hpd, vtotal_test_hpd,
															freq_test_hpd,0);
		VmId_ptm_hpd = GetPreferredVm(Edid_org, max_cap_new ,
														max_cap_lanes_new&0x1F);
		bpc_hpd = XVidC_EdidGetColorDepth(Edid_org);
		if (VmId_ptm_hpd == XVIDC_VM_NOT_SUPPORTED) {
			VmId_ptm_hpd = XVIDC_VM_640x480_60_P;
			bpc_hpd = 6;
		}

	} else {
		VmId_test_hpd = XVIDC_VM_NOT_SUPPORTED;
		VmId_ptm_hpd = XVIDC_VM_640x480_60_P;
		res_update = XVIDC_VM_640x480_60_P;
		bpc_hpd = 6;
		good_edid_hpd = 0;
	}


	if (max_cap_new == XDP_TX_LINK_BW_SET_540GBPS
			|| max_cap_new == XDP_TX_LINK_BW_SET_270GBPS
			|| max_cap_new == XDP_TX_LINK_BW_SET_162GBPS) {
		Status = set_vphy(max_cap_new);


        XDpTxSs_SetLinkRate(&DpTxSsInst, max_cap_new);
		XDpTxSs_SetLaneCount(&DpTxSsInst, max_cap_lanes_new&0x1F);

        if (good_edid_hpd == 1) {
			if ((VmId_ptm_hpd != XVIDC_VM_NOT_SUPPORTED)
							&& (VmId_test_hpd != XVIDC_VM_NOT_SUPPORTED)) {
				XDpTxSs_SetVidMode(&DpTxSsInst, res_update);
			} else if ((VmId_ptm_hpd != XVIDC_VM_NOT_SUPPORTED)) {
				XDpTxSs_SetVidMode(&DpTxSsInst, VmId_ptm_hpd);
			} else {
				XDpTxSs_SetVidMode(&DpTxSsInst, VmId_test_hpd);
			}
        } else {
             XDpTxSs_SetVidMode(&DpTxSsInst, res_update);
        }

        XDpTxSs_SetBpc(&DpTxSsInst, bpc_hpd);
        XDpTxSs_Start(&DpTxSsInst);
        XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr,
											XDP_TX_INTERRUPT_MASK, 0x0);
        Vpg_StreamSrcConfigure(DpTxSsInst.DpPtr, 0, 1);
        Vpg_VidgenSetUserPattern(DpTxSsInst.DpPtr, C_VideoUserStreamPattern[1]);
        Status = XDpTxSs_Start(&DpTxSsInst);


        Status = XDpTxSs_CheckLinkStatus(&DpTxSsInst);
        if (Status != XST_SUCCESS) {
			Status = XDpTxSs_Start(&DpTxSsInst);
        }
	}



	Status = XDpTxSs_CheckLinkStatus(&DpTxSsInst);
}



/*****************************************************************************/
/**
*
* This function to send Audio Information Frame
*
* @param	XilAudioInfoFrame
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void sendAudioInfoFrame(XilAudioInfoFrame *xilInfoFrame)
{
    u8 db1, db2, db3, db4;
    u32 temp;
    u8 RSVD=0;

    //Fixed paramaters
    u8  dp_version   = 0x11;

        //Write #1
    db1 = 0x00; //sec packet ID fixed to 0 - SST Mode
    db2 = 0x80 + xilInfoFrame->type;
    db3 = xilInfoFrame->info_length&0xFF;
    db4 = (dp_version<<2)|(xilInfoFrame->info_length>>8);
	temp = db4<<24|db3<<16|db2<<8|db1;
	XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr,
										XDP_TX_AUDIO_INFO_DATA(1), temp);

	//Write #2
	db1 = xilInfoFrame->audio_channel_count
					| (xilInfoFrame->audio_coding_type<<4) | (RSVD<<3);
	db2 = (RSVD<<5)| (xilInfoFrame->sampling_frequency<<2)
					| xilInfoFrame->sample_size;
	db3 = RSVD;
	db4 = xilInfoFrame->channel_allocation;
	temp = db4<<24|db3<<16|db2<<8|db1;
	XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr,
										XDP_TX_AUDIO_INFO_DATA(1), temp);

	//Write #3
	db1 = (xilInfoFrame->level_shift<<3) | RSVD
								| (xilInfoFrame->downmix_inhibit <<7);
	db2 = RSVD;
	db3 = RSVD;
	db4 = RSVD;
	temp = db4<<24|db3<<16|db2<<8|db1;
	XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr,
										XDP_TX_AUDIO_INFO_DATA(1), temp);

	//Write #4
	db1 = RSVD;
	db2 = RSVD;
	db3 = RSVD;
	db4 = RSVD;
	temp = 0x00000000;
	XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr,
										XDP_TX_AUDIO_INFO_DATA(1), temp);
	//Write #5
	XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr,
										XDP_TX_AUDIO_INFO_DATA(1), temp);

	//Write #6
	XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr,
										XDP_TX_AUDIO_INFO_DATA(1), temp);
	//Write #7
	XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr,
										XDP_TX_AUDIO_INFO_DATA(1), temp);
	//Write #8
	XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr,
										XDP_TX_AUDIO_INFO_DATA(1), temp);
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
	double pixel_freq, pixel_freq1;
	double max_freq[] = {216.0, 172.8, 360.0, 288.0, 720.0, 576.0};

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
		FrameRate = 60;
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
	if (pixel_freq1 < pixel_freq) {
		VmId = XVIDC_VM_NOT_SUPPORTED;
	} else {
		/* Get video mode ID */
		VmId = XVidC_GetVideoModeId(Timing.HActive, Timing.VActive,
							FrameRate, XVidC_EdidIsDtdPtmInterlaced(EdidPtr));

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
* @param	user_config_struct.
*
* @return	Status.
*
* @note		None.
*
******************************************************************************/
u32 set_vphy(int LineRate_init_tx){
	u32 Status=0;
	switch(LineRate_init_tx){
		case XDP_TX_LINK_BW_SET_162GBPS:
			Status = PHY_Configuration_Tx(&VPhyInst,
						PHY_User_Config_Table[(is_TX_CPLL)?0:3]);
			break;

		case XDP_TX_LINK_BW_SET_270GBPS:
			Status = PHY_Configuration_Tx(&VPhyInst,
						PHY_User_Config_Table[(is_TX_CPLL)?1:4]);
			break;

		case XDP_TX_LINK_BW_SET_540GBPS:
			Status = PHY_Configuration_Tx(&VPhyInst,
						PHY_User_Config_Table[(is_TX_CPLL)?2:5]);
			break;
	}

	if (Status != XST_SUCCESS) {
		xil_printf (
   "+++++++ TX GT configuration encountered a failure +++++++\r\n");
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
