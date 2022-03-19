/******************************************************************************
* Copyright (C) 2020 - 2021 Xilinx, Inc. All rights reserved.
* SPDX-License-Identifier: MIT
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
* 1.00 KU  02/05/19 Initial release.
* 1.01 KU  09/09/19 Added support for unplug, enhanced audio check
* 1.02 ND  02/14/19 mcdp related function call now need dprxss instance address
*                   instead of base address  as first parameter
* 1.03 ND  09/11/20 Added support for New AV pattern generator
* 1.04 ND  02/12/21 Added case 'z' to display menu in PT mode.
* 					Added support to set the MSA values for the tx according to
* 					the selected stream to be displayed front the menu.
* 					Added the enabling of vblank interrupts for all streams in
* 					Dprx_InterruptHandlerActRx() handler.
* 1.05 ND  04/03/21 Moved all global variables declaration from .h to .c
* 				    files due to gcc compiler compilation error.
* 1.06 ND  04/15/21 Added support for stream 2,3 and 4 handler interrupts
* 1.07 ND  02/25/22 Modified the code for de-asserting longer hpd for some
* 					gpu's to retrain our rx.
*
* </pre>
*
******************************************************************************/


#include "xdptxss_zcu102_mst_pt.h"

#define I2C_MUX_device_address 0x74
#define Si570_device_address 0x5D
#define audio_clk_Hz 24.576

#ifdef XPAR_XV_AXI4S_REMAP_NUM_INSTANCES
#define REMAP_RX_BASEADDR  XPAR_DP_RX_HIER_0_REMAP_RX_S_AXI_CTRL_BASEADDR
#define REMAP_TX_BASEADDR  XPAR_DP_TX_HIER_0_REMAP_TX_S_AXI_CTRL_BASEADDR
#define REMAP_RX_DEVICE_ID  XPAR_DP_RX_HIER_0_REMAP_RX_DEVICE_ID
#define REMAP_TX_DEVICE_ID  XPAR_DP_TX_HIER_0_REMAP_TX_DEVICE_ID

XV_axi4s_remap_Config   *rx_remap_Config;
XV_axi4s_remap          rx_remap;
XV_axi4s_remap_Config   *tx_remap_Config;
XV_axi4s_remap          tx_remap;
#endif

/************************** Function Prototypes ******************************/

u32 DpMST_Main(void);
u32 DpMST_PlatformInit(void);
char XUartPs_RecvByte_NonBlocking();

void DpRxSs_VmChangeEventHandler(void *InstancePtr);
void DpRxSs_VmChange2EventHandler(void *InstancePtr);
void DpRxSs_VmChange3EventHandler(void *InstancePtr);
void DpRxSs_VmChange4EventHandler(void *InstancePtr);
void DpRxSs_Video2Handler(void *InstancePtr);
void DpRxSs_Video3Handler(void *InstancePtr);
void DpRxSs_Video4Handler(void *InstancePtr);
void DpRxSs_InfoPacket2Handler(void *InstancePtr);
void DpRxSs_InfoPacket3Handler(void *InstancePtr);
void DpRxSs_InfoPacket4Handler(void *InstancePtr);
void DpRxSs_ExtPacket2Handler(void *InstancePtr);
void DpRxSs_ExtPacket3Handler(void *InstancePtr);
void DpRxSs_ExtPacket4Handler(void *InstancePtr);
void DpRxSs_NoVideo2Handler(void *InstancePtr);
void DpRxSs_NoVideo3Handler(void *InstancePtr);
void DpRxSs_NoVideo4Handler(void *InstancePtr);
/************************** Variable Definitions *****************************/
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

extern u8 rx_unplugged;
u32 bpc = 0;
u8 comp = 0;
u32 dp_msa_hres;
u32 dp_msa_vres;
u32 DpHres_total, DpVres_total;
u8 LaneCount;
u8 LineRate;
u8 pixel = 0;
u32 recv_clk_see = 0;
u32 recv_clk_freq_track=0;
float recv_frame_clk_track=0.0;
u32 recv_frame_clk_int_track =0;
u32 recv_frame_clk_int =0;
u32 recv_clk_freq=0;
float recv_frame_clk=0.0;
u32 rxMsaMVid;
u32 rxMsaNVid;
u32 rxMsamisc0;
XDpTxSs_MainStreamAttributes Msa[4];
u8 Bpc[] = {6, 8, 10, 12, 16};

u32 XVFRMBUFRD_BUFFER_BASEADDR;
u32 XVFRMBUFWR_BUFFER_BASEADDR;

XilAudioInfoFrame_rx AudioinfoFrame;
XilAudioExtFrame  SdpExtFrame;
XilAudioExtFrame  SdpExtFrame_q;

XV_FrmbufRd_l2     frmbufrd;
XV_FrmbufWr_l2     frmbufwr;
XAxis_Switch axis_switch;
XAxis_Switch axis_switch_tx;
XDpRxSs DpRxSsInst;	/* The DPTX Subsystem instance.*/
XDpTxSs DpTxSsInst;	/* The DPTX Subsystem instance.*/
XIic IicInstance;	/* I2C bus for Si570 */
XIic_Config *ConfigPtr_IIC;     /* Pointer to configuration data */
XVphy VPhyInst;	/* The DPRX Subsystem instance.*/
XTmrCtr TmrCtr; /* Timer instance.*/
int tx_is_reconnected; /*This variable to keep track of the status of Tx link*/
u8 prev_line_rate; /*This previous line rate to keep previous info to compare
						with new line rate request*/
u8 hpd_pulse_con_event; /*This variable triggers hpd_pulse_con*/
u8 num_sinks;
XScuGic IntcInst;
XDpTxSs_Config *ConfigPtr;
XDpRxSs_Config *ConfigPtr_rx;
DP_Rx_Training_Algo_Config RxTrainConfig;

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

	xil_printf("---------------------------------------------\r\n");
	xil_printf("DisplayPort MST Example Design with AudioLite\r\n");
	xil_printf("(c) 2018 by Xilinx\r\n");
	xil_printf("----------------------------------------------\r\n\r\n");

	Status = DpMST_Main();
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
u32 DpMST_Main()
{
	u32 Status;
	u8 UserInput;
	u8 LineRate_init_tx = XDP_TX_LINK_BW_SET_540GBPS;


	u8 connected = 0;


	user_config_struct user_config;
	user_config.user_bpc = 8;
	user_config.VideoMode_local = XVIDC_VM_800x600_60_P;
	user_config.user_pattern = 1; /*Color Ramp (Default)*/
	user_config.user_format = XVIDC_CSF_RGB;

	Xil_Out32(HLS_RST_BASE, 0x3);
	Xil_Out32(TX_AUD_RST_BASE, 0x1);
//	Xil_Out32(XPAR_AXI_GPIO_2_BASEADDR, 0x0);


	/* Do platform initialization in this function. This is hardware
	 * system specific. It is up to the user to implement this function.
	 */
	xil_printf("Starting PlatformInit..\r\n");
	Status = DpMST_PlatformInit();
	if (Status != XST_SUCCESS) {
		xil_printf("Platform init failed!\r\n");
	}
	xil_printf("Platform initialization done.\r\n");

#if ENABLE_AUDIO
	// I2C MUX device address : 0x74
	// Si570 device address : 0x5D
	// Setting Si570 on zcu102 to be 24.576MHz for audio
	clk_set(I2C_MUX_device_address, Si570_device_address, audio_clk_Hz);
	Xil_Out32(TX_AUD_RST_BASE, 0x0);
	Xil_Out32(TX_AUD_RST_BASE, 0x0);
	Xil_Out32(TX_AUD_RST_BASE, 0x1);
#endif

	//Disabling TX Interrupts
	XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr, 0x144, 0xFFF);
	//Disabling RX link
    XDp_WriteReg(DpRxSsInst.DpPtr->Config.BaseAddr, XDP_RX_LINK_ENABLE, 0x0);

    Status = Dp_SetupIntrSystem();
    if (Status != XST_SUCCESS) {
		xil_printf("ERR: Interrupt system setup failed.\r\n");
		return XST_FAILURE;
    }

    xil_printf ("\r\n!!! Please ensure that MST capable sources and sinks are \r\n");
    xil_printf ("used for this example application demo !!! \r\n");

    operationMenu();
    XScuGic_Disable(&IntcInst, XINTC_DPRXSS_DP_INTERRUPT_ID);
    XScuGic_Disable(&IntcInst, XINTC_DPTXSS_DP_INTERRUPT_ID);


	while (1) { // for menu loop
		UserInput = XUartPs_RecvByte_NonBlocking();
		if (UserInput!=0) {
			xil_printf("UserInput: %c\r\n",UserInput);
			switch (UserInput) {
				case 't':
					DpTxSsInst.DpPtr->TxInstance.TxSetMsaCallback = NULL;
					DpTxSsInst.DpPtr->TxInstance.TxMsaCallbackRef = NULL;
					main_loop();
					break;

				case 'r':
#if FAST_MST
					DpTxSsInst.UsrOpt.MstSupport = 0;
#endif
				    xil_printf ("DP RX set to: LineRate %x, LaneCount %x\r\n",
						  XDp_ReadReg(DpRxSsInst.DpPtr->Config.BaseAddr, 0x9C), XDp_ReadReg(DpRxSsInst.DpPtr->Config.BaseAddr, 0xA0)&0x7);

					pt_loop();
					break;
			}
		}
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
u8 edid_rx[256];
u32 DpMST_PlatformInit(void)
{

	u32 Status;
	u8 Edid_org[128], Edid1_org[128];
//	u8 edid_rx[256];
	u8 LaneCount_init = XDP_TX_LANE_COUNT_SET_4;
	u8 LaneCount_init_tx = XDP_TX_LANE_COUNT_SET_4;
	u8 LineRate_init = XDP_TX_LINK_BW_SET_540GBPS;

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

    XAxis_Switch_Config *ConfigPtr_AXIS_SWITCH = XAxisScr_LookupConfig(XPAR_DP_RX_HIER_0_AXIS_SWITCH_0_DEVICE_ID);
     if (ConfigPtr_AXIS_SWITCH == NULL) {
             return XST_FAILURE;
     }

     Status = XAxisScr_CfgInitialize(&axis_switch, ConfigPtr_AXIS_SWITCH, ConfigPtr_AXIS_SWITCH->BaseAddress);
     if (Status != XST_SUCCESS) {
             return XST_FAILURE;
     }
#ifdef XPAR_XV_AXI4S_REMAP_NUM_INSTANCES
 	rx_remap_Config = XV_axi4s_remap_LookupConfig(REMAP_RX_DEVICE_ID);
 	Status = XV_axi4s_remap_CfgInitialize(&rx_remap, rx_remap_Config,
 					      rx_remap_Config->BaseAddress);
 	rx_remap.IsReady = XIL_COMPONENT_IS_READY;
 	if (Status != XST_SUCCESS) {
 		xil_printf("ERROR:: AXI4S_REMAP Initialization "
 			   "failed %d\r\n", Status);
 		return (XST_FAILURE);
 	}

 	tx_remap_Config = XV_axi4s_remap_LookupConfig(REMAP_TX_DEVICE_ID);
 	Status = XV_axi4s_remap_CfgInitialize(&tx_remap, tx_remap_Config,
 					      tx_remap_Config->BaseAddress);
 	tx_remap.IsReady = XIL_COMPONENT_IS_READY;
 	if (Status != XST_SUCCESS) {
 		xil_printf("ERROR:: AXI4S_REMAP Initialization "
 			   "failed %d\r\n", Status);
 		return(XST_FAILURE);
 	}

 	XV_axi4s_remap_Set_width(&rx_remap, 7680);
 	XV_axi4s_remap_Set_height(&rx_remap, 4320);
 	XV_axi4s_remap_Set_ColorFormat(&rx_remap, 0);
 	XV_axi4s_remap_Set_inPixClk(&rx_remap, 4);
 	XV_axi4s_remap_Set_outPixClk(&rx_remap, 4);

 	XV_axi4s_remap_Set_width(&tx_remap, 7680);
 	XV_axi4s_remap_Set_height(&tx_remap, 4320);
 	XV_axi4s_remap_Set_ColorFormat(&tx_remap, 0);
 	XV_axi4s_remap_Set_inPixClk(&tx_remap, 4);
 	XV_axi4s_remap_Set_outPixClk(&tx_remap, 4);
#endif
//    XVidFrameCrc_Initialize(&VidFrameCRC);
	/* FrameBuffer initialization. */
	Status = XVFrmbufRd_Initialize(&frmbufrd, FRMBUF_RD_DEVICE_ID);
	if (Status != XST_SUCCESS) {
		xil_printf("ERROR:: Frame Buffer Read "
			   "initialization failed\r\n");
		return (XST_FAILURE);
	}

	Status = XVFrmbufWr_Initialize(&frmbufwr, FRMBUF_WR_DEVICE_ID);
	if(Status != XST_SUCCESS) {
		xil_printf("ERROR:: Frame Buffer Write "
			   "initialization failed\r\n");
		return (XST_FAILURE);
	}

	resetIp_wr();
	resetIp_rd();

	VideoFMC_Init();

	IDT_8T49N24x_SetClock(XPAR_IIC_0_BASEADDR, I2C_IDT8N49_ADDR, 0, 270000000, TRUE);

	i2c_write_dp141(XPAR_IIC_0_BASEADDR, I2C_TI_DP141_ADDR, 0x02, 0x78);
	i2c_write_dp141(XPAR_IIC_0_BASEADDR, I2C_TI_DP141_ADDR, 0x05, 0x78);
	i2c_write_dp141(XPAR_IIC_0_BASEADDR, I2C_TI_DP141_ADDR, 0x08, 0x78);
	i2c_write_dp141(XPAR_IIC_0_BASEADDR, I2C_TI_DP141_ADDR, 0x0B, 0x78);

	/* Setup Video Phy, left to the user for implementation */
	DpTxSs_VideoPhyInit(XVPHY_DEVICE_ID);

	/* Obtain the device configuration for the DisplayPort TX Subsystem */
	ConfigPtr = XDpTxSs_LookupConfig(XDPTXSS_DEVICE_ID);
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


	DpTxSs_Setup(&LineRate_init, &LaneCount_init, Edid_org, Edid1_org);

	LaneCount_init_tx = LaneCount_init_tx & 0x7;

	int i=0;
	for (i=0;i<128;i++) {
		edid_rx[i] = Edid_org[i];
		edid_rx[128+i] = Edid1_org[i];
	}
	// The Edid read from the Monitor (MST Capable) is copied as edid for
	// each stream.
	// User may add his/her own EDID instead of copying the
	// Monitor EDID
	// Currently on 2 block EDID is supported

	DpRxSsInst.EdidSize[0] = 256;
	DpRxSsInst.EdidDataPtr[0] = &edid_rx[0];

	DpRxSsInst.EdidSize[1] = 256;
	DpRxSsInst.EdidDataPtr[1] = &edid_rx[0];

	DpRxSsInst.EdidSize[2] = 256;
	DpRxSsInst.EdidDataPtr[2] = &edid_rx[0];

	DpRxSsInst.EdidSize[3] = 256;
	DpRxSsInst.EdidDataPtr[3] = &edid_rx[0];


	/* Obtain the device configuration
	 * for the DisplayPort RX Subsystem */
	ConfigPtr_rx = XDpRxSs_LookupConfig (XDPRXSS_DEVICE_ID);
	if (!ConfigPtr_rx) {
			return XST_FAILURE;
	}
	/* Copy the device configuration into
	 * the DpRxSsInst's Config structure. */
	Status = XDpRxSs_CfgInitialize(&DpRxSsInst, ConfigPtr_rx,
									ConfigPtr_rx->BaseAddress);
	if (Status != XST_SUCCESS) {
			xil_printf("DPRXSS config initialization failed.\n\r");
			return XST_FAILURE;
	}

	XDpRxSs_McDp6000_init(&DpRxSsInst);

	/* issue HPD at here to inform DP source */
	XDp_RxInterruptDisable(DpRxSsInst.DpPtr, 0xFFF8FFFF);
	XDp_RxInterruptEnable(DpRxSsInst.DpPtr, 0x80000000);
	XDp_RxGenerateHpdInterrupt(DpRxSsInst.DpPtr, 50000);

	/* Start DPRX Subsystem set */
	Status = XDpRxSs_Start(&DpRxSsInst);
	if (Status != XST_SUCCESS) {
		 xil_printf("ERR:DPRX SS start failed\n\r");
		 return;
	}
	XDp_WriteReg(DpRxSsInst.DpPtr->Config.BaseAddr, XDP_RX_LINK_ENABLE, 0x0);

	/* Set Link rate and lane count to maximum */
	XDpRxSs_SetLinkRate(&DpRxSsInst, DPRXSS_LINK_RATE);
	XDpRxSs_SetLaneCount(&DpRxSsInst, DPRXSS_LANE_COUNT);

	xil_printf("DP TX and RX init done...\n\r");



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
u32 Dp_SetupIntrSystem(void)
{
	u32 Status;
	XINTC *IntcInstPtr = &IntcInst;

	/* Set custom timer wait */
	XDpTxSs_SetUserTimerHandler(&DpTxSsInst, &DpPt_CustomWaitUs, &TmrCtr);

	/* Declaration of TX Interrupt Handlers */
	XDpTxSs_SetCallBack(&DpTxSsInst, (XDPTXSS_HANDLER_DP_HPD_EVENT),
						&DpPt_HpdEventHandler, &DpTxSsInst);
	XDpTxSs_SetCallBack(&DpTxSsInst, (XDPTXSS_HANDLER_DP_HPD_PULSE),
						&DpPt_HpdPulseHandler, &DpTxSsInst);
	XDpTxSs_SetCallBack(&DpTxSsInst, (XDPTXSS_HANDLER_DP_LINK_RATE_CHG),
						&DpPt_LinkrateChgHandler, &DpTxSsInst);
	XDpTxSs_SetCallBack(&DpTxSsInst, (XDPTXSS_HANDLER_DP_PE_VS_ADJUST),
						&DpPt_pe_vs_adjustHandler, &DpTxSsInst);

	/* Declaration of RX Interrupt Handlers */
    XDpRxSs_SetCallBack(&DpRxSsInst, XDPRXSS_HANDLER_DP_PWR_CHG_EVENT,
                        &DpRxSs_PowerChangeHandler, &DpRxSsInst);
    XDpRxSs_SetCallBack(&DpRxSsInst, (XDPRXSS_HANDLER_DP_VM_CHG_EVENT),
						&DpRxSs_VmChangeEventHandler, &DpRxSsInst);
    XDpRxSs_SetCallBack(&DpRxSsInst, (XDPRXSS_HANDLER_DP_VM_CHG_STREAM_2_EVENT),
						&DpRxSs_VmChange2EventHandler, &DpRxSsInst);
    XDpRxSs_SetCallBack(&DpRxSsInst, (XDPRXSS_HANDLER_DP_VM_CHG_STREAM_3_EVENT),
						&DpRxSs_VmChange3EventHandler, &DpRxSsInst);
    XDpRxSs_SetCallBack(&DpRxSsInst, (XDPRXSS_HANDLER_DP_VM_CHG_STREAM_4_EVENT),
						&DpRxSs_VmChange4EventHandler, &DpRxSsInst);
    XDpRxSs_SetCallBack(&DpRxSsInst, XDPRXSS_HANDLER_DP_NO_VID_EVENT,
                        &DpRxSs_NoVideoHandler, &DpRxSsInst);
    XDpRxSs_SetCallBack(&DpRxSsInst, XDPRXSS_HANDLER_DP_NO_VID_STREAM_2_EVENT,
                        &DpRxSs_NoVideo2Handler, &DpRxSsInst);
    XDpRxSs_SetCallBack(&DpRxSsInst, XDPRXSS_HANDLER_DP_NO_VID_STREAM_3_EVENT,
                        &DpRxSs_NoVideo3Handler, &DpRxSsInst);
    XDpRxSs_SetCallBack(&DpRxSsInst, XDPRXSS_HANDLER_DP_NO_VID_STREAM_4_EVENT,
                        &DpRxSs_NoVideo4Handler, &DpRxSsInst);
    XDpRxSs_SetCallBack(&DpRxSsInst, XDPRXSS_HANDLER_DP_VBLANK_EVENT,
                        &DpRxSs_VerticalBlankHandler, &DpRxSsInst);

    XDpRxSs_SetCallBack(&DpRxSsInst, XDPRXSS_HANDLER_DP_VBLANK_STREAM_2_EVENT,
                        &DpRxSs_VerticalBlank1Handler, &DpRxSsInst);

    XDpRxSs_SetCallBack(&DpRxSsInst, XDPRXSS_HANDLER_DP_VBLANK_STREAM_3_EVENT,
                        &DpRxSs_VerticalBlank2Handler, &DpRxSsInst);

    XDpRxSs_SetCallBack(&DpRxSsInst, XDPRXSS_HANDLER_DP_VBLANK_STREAM_4_EVENT,
                        &DpRxSs_VerticalBlank3Handler, &DpRxSsInst);

    XDpRxSs_SetCallBack(&DpRxSsInst, XDPRXSS_HANDLER_DP_TLOST_EVENT,
                        &DpRxSs_TrainingLostHandler, &DpRxSsInst);
    XDpRxSs_SetCallBack(&DpRxSsInst, XDPRXSS_HANDLER_DP_VID_EVENT,
                        &DpRxSs_VideoHandler, &DpRxSsInst);
    XDpRxSs_SetCallBack(&DpRxSsInst, XDPRXSS_HANDLER_DP_VID_STREAM_2_EVENT,
                        &DpRxSs_Video2Handler, &DpRxSsInst);
    XDpRxSs_SetCallBack(&DpRxSsInst, XDPRXSS_HANDLER_DP_VID_STREAM_3_EVENT,
                        &DpRxSs_Video3Handler, &DpRxSsInst);
    XDpRxSs_SetCallBack(&DpRxSsInst, XDPRXSS_HANDLER_DP_VID_STREAM_4_EVENT,
                        &DpRxSs_Video4Handler, &DpRxSsInst);
    XDpRxSs_SetCallBack(&DpRxSsInst, XDPRXSS_HANDLER_DP_TDONE_EVENT,
                        &DpRxSs_TrainingDoneHandler, &DpRxSsInst);
    XDpRxSs_SetCallBack(&DpRxSsInst, XDPRXSS_HANDLER_UNPLUG_EVENT,
                        &DpRxSs_UnplugHandler, &DpRxSsInst);
    XDpRxSs_SetCallBack(&DpRxSsInst, XDPRXSS_HANDLER_LINKBW_EVENT,
                        &DpRxSs_LinkBandwidthHandler, &DpRxSsInst);
    XDpRxSs_SetCallBack(&DpRxSsInst, XDPRXSS_HANDLER_PLL_RESET_EVENT,
                        &DpRxSs_PllResetHandler, &DpRxSsInst);
    XDpRxSs_SetCallBack(&DpRxSsInst, XDPRXSS_HANDLER_DP_BW_CHG_EVENT,
                        &DpRxSs_BWChangeHandler, &DpRxSsInst);
    XDpRxSs_SetCallBack(&DpRxSsInst, XDPRXSS_HANDLER_ACCESS_LINK_QUAL_EVENT,
                        &DpRxSs_AccessLinkQualHandler, &DpRxSsInst);
    XDpRxSs_SetCallBack(&DpRxSsInst, XDPRXSS_HANDLER_ACCESS_ERROR_COUNTER_EVENT,
                        &DpRxSs_AccessErrorCounterHandler, &DpRxSsInst);
    XDpRxSs_SetCallBack(&DpRxSsInst, XDPRXSS_HANDLER_DP_CRC_TEST_EVENT,
                        &DpRxSs_CRCTestEventHandler, &DpRxSsInst);
    XDpRxSs_SetCallBack(&DpRxSsInst, XDPRXSS_HANDLER_DP_INFO_PKT_EVENT,
                        &DpRxSs_InfoPacketHandler, &DpRxSsInst);
    XDpRxSs_SetCallBack(&DpRxSsInst, XDPRXSS_HANDLER_DP_INFO_PKT_STREAM_2_EVENT,
                        &DpRxSs_InfoPacket2Handler, &DpRxSsInst);
    XDpRxSs_SetCallBack(&DpRxSsInst, XDPRXSS_HANDLER_DP_INFO_PKT_STREAM_3_EVENT,
                        &DpRxSs_InfoPacket3Handler, &DpRxSsInst);
    XDpRxSs_SetCallBack(&DpRxSsInst, XDPRXSS_HANDLER_DP_INFO_PKT_STREAM_4_EVENT,
                        &DpRxSs_InfoPacket4Handler, &DpRxSsInst);
    XDpRxSs_SetCallBack(&DpRxSsInst, XDPRXSS_HANDLER_DP_EXT_PKT_EVENT,
                    	&DpRxSs_ExtPacketHandler, &DpRxSsInst);
    XDpRxSs_SetCallBack(&DpRxSsInst, XDPRXSS_HANDLER_DP_EXT_PKT_STREAM_2_EVENT,
			&DpRxSs_ExtPacket2Handler, &DpRxSsInst);
    XDpRxSs_SetCallBack(&DpRxSsInst, XDPRXSS_HANDLER_DP_EXT_PKT_STREAM_3_EVENT,
			&DpRxSs_ExtPacket3Handler, &DpRxSsInst);
    XDpRxSs_SetCallBack(&DpRxSsInst, XDPRXSS_HANDLER_DP_EXT_PKT_STREAM_4_EVENT,
			&DpRxSs_ExtPacket4Handler, &DpRxSsInst);
	XDpRxSs_SetCallBack(&DpRxSsInst, XDPRXSS_HANDLER_DP_DWN_REQ_EVENT,
			        	&Dprx_InterruptHandlerDownReq, &DpRxSsInst);
	XDpRxSs_SetCallBack(&DpRxSsInst, XDPRXSS_HANDLER_DP_DWN_REP_EVENT,
			        	&Dprx_InterruptHandlerDownReply, &DpRxSsInst);
	XDpRxSs_SetCallBack(&DpRxSsInst, XDPRXSS_HANDLER_DP_PAYLOAD_ALLOC_EVENT,
			        	&Dprx_InterruptHandlerPayloadAlloc, &DpRxSsInst);
	XDpRxSs_SetCallBack(&DpRxSsInst, XDPRXSS_HANDLER_DP_ACT_RX_EVENT,
			        	&Dprx_InterruptHandlerActRx, &DpRxSsInst);
#if ADAPTIVE
	XDpRxSs_SetCallBack(&DpRxSsInst, XDPRXSS_HANDLER_DP_ADAPTIVESYNC_VBLANK_STREAM_1_EVENT,
			&DpRxSs_AdaptiveVblank1Handler, &DpRxSsInst);
	XDpRxSs_SetCallBack(&DpRxSsInst, XDPRXSS_HANDLER_DP_ADAPTIVESYNC_VBLANK_STREAM_2_EVENT,
			&DpRxSs_AdaptiveVblank2Handler, &DpRxSsInst);
	XDpRxSs_SetCallBack(&DpRxSsInst, XDPRXSS_HANDLER_DP_ADAPTIVESYNC_VBLANK_STREAM_3_EVENT,
			&DpRxSs_AdaptiveVblank3Handler, &DpRxSsInst);
	XDpRxSs_SetCallBack(&DpRxSsInst, XDPRXSS_HANDLER_DP_ADAPTIVESYNC_VBLANK_STREAM_4_EVENT,
			&DpRxSs_AdaptiveVblank4Handler, &DpRxSsInst);

	XDpRxSs_SetCallBack(&DpRxSsInst, XDPRXSS_HANDLER_DP_ADAPTIVESYNC_SDP_STREAM_1_EVENT,
			&DpRxSs_AdaptiveSDP1Handler, &DpRxSsInst);
	XDpRxSs_SetCallBack(&DpRxSsInst, XDPRXSS_HANDLER_DP_ADAPTIVESYNC_SDP_STREAM_2_EVENT,
			&DpRxSs_AdaptiveSDP2Handler, &DpRxSsInst);
	XDpRxSs_SetCallBack(&DpRxSsInst, XDPRXSS_HANDLER_DP_ADAPTIVESYNC_SDP_STREAM_3_EVENT,
			&DpRxSs_AdaptiveSDP3Handler, &DpRxSsInst);
	XDpRxSs_SetCallBack(&DpRxSsInst, XDPRXSS_HANDLER_DP_ADAPTIVESYNC_SDP_STREAM_4_EVENT,
			&DpRxSs_AdaptiveSDP4Handler, &DpRxSsInst);
#endif

	XVFrmbufWr_SetCallback(&frmbufwr, XVFRMBUFWR_HANDLER_DONE,
			            bufferWr_callback, &frmbufwr);

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
	XScuGic_Enable(IntcInstPtr,	XINTC_DPTXSS_DP_INTERRUPT_ID);

    Status = XScuGic_Connect(IntcInstPtr, XINTC_DPRXSS_DP_INTERRUPT_ID,
                            (Xil_InterruptHandler)XDpRxSs_DpIntrHandler,
                            &DpRxSsInst);
    if (Status != XST_SUCCESS) {
            xil_printf("ERR: DP RX SS DP interrupt connect failed!\n\r");
            return XST_FAILURE;
    }
    /* Enable the interrupt for the DP device */
    XScuGic_Enable(IntcInstPtr, XINTC_DPRXSS_DP_INTERRUPT_ID);

	Status = XScuGic_Connect(IntcInstPtr, XPAR_FABRIC_V_FRMBUF_WR_0_VEC_ID,
				 (Xil_InterruptHandler)XVFrmbufWr_InterruptHandler,
				 &frmbufwr);
	if (Status != XST_SUCCESS) {
		xil_printf("ERROR:: FRMBUF WR interrupt connect failed!\r\n");
		return XST_FAILURE;
	}
	/* Enable the interrupt vector at the interrupt controller */
	XScuGic_Enable(IntcInstPtr, XPAR_FABRIC_V_FRMBUF_WR_0_VEC_ID);

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
* This function is the callback function for when the Video mode change event
*  interrupt occurs.
*
* @param    InstancePtr is a pointer to the XDpRxSs instance.
*
* @return    None.
*
* @note        None.
*
******************************************************************************/
void DpRxSs_VmChangeEventHandler(void *InstancePtr)
{
}

/*****************************************************************************/
/**
*
* This function is the callback function for when the Video mode change event
*  interrupt occurs.
*
* @param    InstancePtr is a pointer to the XDpRxSs instance.
*
* @return    None.
*
* @note        None.
*
******************************************************************************/
void DpRxSs_VmChange2EventHandler(void *InstancePtr)
{
}

/*****************************************************************************/
/**
*
* This function is the callback function for when the Video mode change event
*  interrupt occurs.
*
* @param    InstancePtr is a pointer to the XDpRxSs instance.
*
* @return    None.
*
* @note        None.
*
******************************************************************************/
void DpRxSs_VmChange3EventHandler(void *InstancePtr)
{
}

/*****************************************************************************/
/**
*
* This function is the callback function for when the Video mode change event
*  interrupt occurs.
*
* @param    InstancePtr is a pointer to the XDpRxSs instance.
*
* @return    None.
*
* @note        None.
*
******************************************************************************/
void DpRxSs_VmChange4EventHandler(void *InstancePtr)
{
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
#if USE_NO_VIDEO

        DpRxSsInst.VBlankCount = 0;
        XDp_RxInterruptEnable(DpRxSsInst.DpPtr,
                        XDP_RX_INTERRUPT_MASK_VBLANK_MASK);
        XDp_RxInterruptDisable(DpRxSsInst.DpPtr,
                        XDP_RX_INTERRUPT_MASK_NO_VIDEO_MASK);
        XDp_RxDtgDis(DpRxSsInst.DpPtr);
        XDp_RxDtgEn(DpRxSsInst.DpPtr);

        DpRxSsInst.no_video_trigger = 1;

        XDp_RxInterruptEnable(DpRxSsInst.DpPtr,
                        XDP_RX_INTERRUPT_MASK_INFO_PKT_MASK);

#endif
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

void DpRxSs_NoVideo2Handler(void *InstancePtr)
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

void DpRxSs_NoVideo3Handler(void *InstancePtr)
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

void DpRxSs_NoVideo4Handler(void *InstancePtr)
{
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

void DpRxSs_VerticalBlank1Handler(void *InstancePtr)
{
        DpRxSsInst.VBlankCount++;
}

void DpRxSs_VerticalBlank2Handler(void *InstancePtr)
{
        DpRxSsInst.VBlankCount++;
}

void DpRxSs_VerticalBlank3Handler(void *InstancePtr)
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
u32 appx_fs_dup = 0;
extern u8 rx_trained;
extern u8 tx_is_up;
u8 aud_info_rcvd = 0;

void DpRxSs_TrainingLostHandler(void *InstancePtr)
{
        XDp_RxGenerateHpdInterrupt(DpRxSsInst.DpPtr, 750);
        XDpRxSs_AudioDisable(&DpRxSsInst);
    	XDp_RxDtgDis(DpRxSsInst.DpPtr);
        DpRxSsInst.link_up_trigger = 0;
        DpRxSsInst.VBlankCount = 0;
        aud_info_rcvd = 0;
        AudioinfoFrame.frame_count = 0;
        appx_fs_dup = 0;
    	XDp_WriteReg(DpRxSsInst.DpPtr->Config.BaseAddr, 0x1C, 0x80);
    	XDp_WriteReg(DpRxSsInst.DpPtr->Config.BaseAddr, 0x1C, 0x0);
    	tx_is_up = 0;
    	if (rx_trained == 1) {
    		xil_printf ("Training Lost !!\r\n");
    	}
    	rx_trained = 0;
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
void DpRxSs_Video2Handler(void *InstancePtr)
{

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
void DpRxSs_Video3Handler(void *InstancePtr)
{

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
void DpRxSs_Video4Handler(void *InstancePtr)
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
	    u32 LaneCount, LineRate;
	    u32 training_done_lane01, training_done_lane23;
        DpRxSsInst.link_up_trigger = 1;
        DpRxSsInst.VBlankCount = 0;
        XDp_RxInterruptDisable(DpRxSsInst.DpPtr, XDP_RX_INTERRUPT_MASK_DOWN_REPLY_MASK);
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
//	    xil_printf ("Cable Unplugged !!\r\n");
	    rx_unplugged = 1;
        appx_fs_dup = 0;
        aud_info_rcvd = 0;
        XDpRxSs_AudioDisable(&DpRxSsInst);
        AudioinfoFrame.frame_count = 0;
        SdpExtFrame.Header[1] = 0;
        SdpExtFrame_q.Header[1] = 0;
        SdpExtFrame.frame_count = 0;
        SdpExtFrame.frame_count = 0;

        /*Enable Training related interrupts*/
        XDp_RxInterruptDisable(DpRxSsInst.DpPtr,
                               XDP_RX_INTERRUPT_MASK_ALL_MASK);
        XDp_RxInterruptDisable1(DpRxSsInst.DpPtr, 0xFFFFFFFF);

        /*Enable Training related interrupts*/
               XDp_RxInterruptEnable(DpRxSsInst.DpPtr,
                               XDP_RX_INTERRUPT_MASK_TP1_MASK |
                               XDP_RX_INTERRUPT_MASK_TP2_MASK |
                               XDP_RX_INTERRUPT_MASK_TP3_MASK|
                               XDP_RX_INTERRUPT_MASK_POWER_STATE_MASK|
                               XDP_RX_INTERRUPT_MASK_CRC_TEST_MASK|
                               XDP_RX_INTERRUPT_MASK_BW_CHANGE_MASK |
						XDP_RX_INTERRUPT_MASK_VCP_ALLOC_MASK |
						XDP_RX_INTERRUPT_MASK_VCP_DEALLOC_MASK |
						XDP_RX_INTERRUPT_MASK_DOWN_REPLY_MASK |
						XDP_RX_INTERRUPT_MASK_DOWN_REQUEST_MASK);


        XDp_RxInterruptEnable1(DpRxSsInst.DpPtr,
                        XDP_RX_INTERRUPT_MASK_TP4_MASK|
                        XDP_RX_INTERRUPT_MASK_ACCESS_LANE_SET_MASK|
                        XDP_RX_INTERRUPT_MASK_ACCESS_LINK_QUAL_MASK|
                        XDP_RX_INTERRUPT_MASK_ACCESS_ERROR_COUNTER_MASK);

        XDp_RxGenerateHpdInterrupt(DpRxSsInst.DpPtr, 5000);
        DpRxSsInst.link_up_trigger = 0;
        DpRxSsInst.no_video_trigger = 1;
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
        Status = XVphy_ClkInitialize(&VPhyInst, 0,
                                                                        XVPHY_CHANNEL_ID_CHA, XVPHY_DIR_RX);

        if(Status != XST_SUCCESS)
                xil_printf("XVphy_ClkInitialize failed\r\n");

        DpRxSsInst.link_up_trigger = 0;
        DpRxSsInst.no_video_trigger = 1;
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
        XVphy_BufgGtReset(&VPhyInst, XVPHY_DIR_RX,(FALSE));
        XVphy_WaitForResetDone(&VPhyInst, 0, XVPHY_CHANNEL_ID_CHA,
                               XVPHY_DIR_RX);
        XVphy_WaitForPllLock(&VPhyInst, 0, XVPHY_CHANNEL_ID_CHA);

        /*Enable all interrupts except Unplug*/
        XDp_RxInterruptEnable(DpRxSsInst.DpPtr,
                              XDP_RX_INTERRUPT_MASK_ALL_MASK);
        XDp_RxInterruptEnable1(DpRxSsInst.DpPtr,
                              0xFFFFFFFF);

        DpRxSsInst.no_video_trigger = 1;
        DpRxSsInst.VBlankCount = 0;
        DpRxSsInst.link_up_trigger = 0;
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

        //xil_printf("DpRxSs_AccessLinkQualHandler : 0x%x\r\n", ReadVal);

        /*Check for PRBS Mode*/
        if( (ReadVal&0x00000007) == XDP_RX_DPCD_LINK_QUAL_PRBS){
                /*Enable PRBS Mode in Video PHY*/
                DrpVal = XVphy_ReadReg(VPhyInst.Config.BaseAddr,
                                       XVPHY_RX_CONTROL_REG);
                DrpVal = DrpVal | 0x10101010;
                XVphy_WriteReg(VPhyInst.Config.BaseAddr,
                               XVPHY_RX_CONTROL_REG, DrpVal);

                /*Reset PRBS7 Counters*/
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

                /*Set PRBS mode in Retimer*/
                XDpRxSs_MCDP6000_EnablePrbs7_Rx(&DpRxSsInst,
                                        I2C_MCDP6000_ADDR);
                XDpRxSs_MCDP6000_ClearCounter(&DpRxSsInst,
                                      I2C_MCDP6000_ADDR);
        //      MCDP6000_EnableCounter(XPAR_IIC_0_BASEADDR, I2C_MCDP6000_ADDR);
        } else {
            /*Disable PRBS Mode in Video PHY*/
            DrpVal = XVphy_ReadReg(VPhyInst.Config.BaseAddr,
                                   XVPHY_RX_CONTROL_REG);
            DrpVal = DrpVal & 0xEFEFEFEF;
            XVphy_WriteReg(VPhyInst.Config.BaseAddr,
                           XVPHY_RX_CONTROL_REG, DrpVal);

            /*Disable PRBS mode in Retimer*/
            XDpRxSs_MCDP6000_DisablePrbs7_Rx(&DpRxSsInst,
                                                                                    I2C_MCDP6000_ADDR);
            XDpRxSs_MCDP6000_ClearCounter(&DpRxSsInst,
                                  I2C_MCDP6000_ADDR);
    //      MCDP6000_EnableCounter(XPAR_IIC_0_BASEADDR, I2C_MCDP6000_ADDR);
    }
}

void DpRxSs_AccessErrorCounterHandler(void *InstancePtr)
{
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
u32 rx_maud_dup = 0;
u32 rx_naud_dup = 0;
u8 start_i2s_clk = 0;

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
        if (AudioinfoFrame.frame_count > 20) {
		//ensuring audio info frame is captured reliably
		aud_info_rcvd = 1;
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
void DpRxSs_InfoPacket2Handler(void *InstancePtr)
{

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
void DpRxSs_InfoPacket3Handler(void *InstancePtr)
{

}

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
void DpRxSs_InfoPacket4Handler(void *InstancePtr)
{

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
void DpRxSs_ExtPacket2Handler(void *InstancePtr)
{

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
void DpRxSs_ExtPacket3Handler(void *InstancePtr)
{

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
void DpRxSs_ExtPacket4Handler(void *InstancePtr)
{

}

#if ADAPTIVE
void DpRxSs_AdaptiveVblank1Handler(void *InstancePtr)
{

}

void DpRxSs_AdaptiveVblank2Handler(void *InstancePtr)
{

}

void DpRxSs_AdaptiveVblank3Handler(void *InstancePtr)
{

}

void DpRxSs_AdaptiveVblank4Handler(void *InstancePtr)
{

}

void DpRxSs_AdaptiveSDP1Handler(void *InstancePtr)
{

}

void DpRxSs_AdaptiveSDP2Handler(void *InstancePtr)
{

}

void DpRxSs_AdaptiveSDP3Handler(void *InstancePtr)
{

}

void DpRxSs_AdaptiveSDP4Handler(void *InstancePtr)
{

}
#endif

void Dprx_InterruptHandlerPayloadAlloc(void *InstancePtr)
{
//	xil_printf ("payload alloc\r\n");
	/* Virtual Channel Payload allocation,
	 * de-allocation and partial deletion handler
	 */
	XDpRxSs *DpRxSsPtr = (XDpRxSs *)InstancePtr;
	u32 RegVal = 0;
	XDp *DpPtr = DpRxSsPtr->DpPtr;
	RegVal = XDp_ReadReg(DpPtr->Config.BaseAddr, XDP_RX_MST_ALLOC);
//	StreamIdRecv = (RegVal & XDP_RX_MST_ALLOC_VCP_ID_MASK);

	XDp_RxAllocatePayloadStream(DpPtr);
}

void Dprx_InterruptHandlerActRx(void *InstancePtr)
{
	/* ACT Receive Interrupt Handler */
//	ActCount++;
//	xil_printf("ACT \r\n");
    XDp_RxInterruptEnable(DpRxSsInst.DpPtr,
                    XDP_RX_INTERRUPT_MASK_VBLANK_MASK);
    XDp_RxInterruptEnable1(DpRxSsInst.DpPtr,
                    0x3FFFF);

}

void Dprx_InterruptHandlerDownReq(void *InstancePtr)
{
	/* Down Request Buffer Ready handler
	 * (Indicates the availability of the Down request)
	 */
//	xil_printf("Received MST Handler down request \r\n");
//	dwnreq++;
	XDp_RxHandleDownReq(DpRxSsInst.DpPtr);
}

void Dprx_InterruptHandlerDownReply(void *InstancePtr)
{
	/* Down Reply Buffer Read handler (indicates a
	 * read event from down reply buffer by upstream source)
	 */
//	xil_printf("Received MST Handler down Reply \r\n");

	/* Increment the DownRequest Counter (if any) */
}

void bufferWr_callback(void *InstancePtr){
	u32 Status;
	if(XVFRMBUFWR_BUFFER_BASEADDR >= (0 + (0x10000000) + (0x08000000 * 3))){
		XVFRMBUFRD_BUFFER_BASEADDR = (0 + (0x10000000) + (0x08000000 * 2));
		XVFRMBUFWR_BUFFER_BASEADDR = 0 + (0x10000000);
	}else{
		XVFRMBUFRD_BUFFER_BASEADDR = XVFRMBUFWR_BUFFER_BASEADDR;
		XVFRMBUFWR_BUFFER_BASEADDR = XVFRMBUFWR_BUFFER_BASEADDR + 0x08000000;
	}


	Status = XVFrmbufWr_SetBufferAddr(&frmbufwr, XVFRMBUFWR_BUFFER_BASEADDR);
	if(Status != XST_SUCCESS) {
		xil_printf("ERROR:: Unable to configure Frame Buffer "
				"Write buffer address\r\n");
	}
	Status = XVFrmbufRd_SetBufferAddr(&frmbufrd, XVFRMBUFRD_BUFFER_BASEADDR);
	if(Status != XST_SUCCESS) {
		xil_printf("ERROR:: Unable to configure Frame Buffer "
				"Read buffer address\r\n");
	}
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

	PHY_Two_byte_set (&VPhyInst, SET_TX_TO_2BYTE, SET_RX_TO_2BYTE);

    XVphy_ResetGtPll(&VPhyInst, 0, PHY_User_Config_Table[5].TxChId, XVPHY_DIR_TX,(TRUE));
    XVphy_BufgGtReset(&VPhyInst, XVPHY_DIR_TX,(TRUE));

    XVphy_ResetGtPll(&VPhyInst, 0, PHY_User_Config_Table[5].TxChId, XVPHY_DIR_TX,(FALSE));
    XVphy_BufgGtReset(&VPhyInst, XVPHY_DIR_TX,(FALSE));

    XVphy_ResetGtPll(&VPhyInst, 0, PHY_User_Config_Table[5].RxChId, XVPHY_DIR_RX,(TRUE));
    XVphy_BufgGtReset(&VPhyInst, XVPHY_DIR_RX,(TRUE));

    XVphy_ResetGtPll(&VPhyInst, 0, PHY_User_Config_Table[5].RxChId, XVPHY_DIR_RX,(FALSE));
    XVphy_BufgGtReset(&VPhyInst, XVPHY_DIR_RX,(FALSE));

    PHY_Configuration_Tx(&VPhyInst, PHY_User_Config_Table[5]);

	xil_printf("VPHY init done...\n\r");

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
        case 0x6:
                XVphy_CfgQuadRefClkFreq(InstancePtr, 0,
                                        ONBOARD_REF_CLK,
                                        XVPHY_DP_REF_CLK_FREQ_HZ_270);
                XVphy_CfgQuadRefClkFreq(InstancePtr, 0,
                                        ONBOARD_REF_CLK,
                                        XVPHY_DP_REF_CLK_FREQ_HZ_270);
                XVphy_CfgLineRate(InstancePtr, 0, XVPHY_CHANNEL_ID_CHA,
                                  XVPHY_DP_LINK_RATE_HZ_162GBPS);
                XVphy_CfgLineRate(InstancePtr, 0, XVPHY_CHANNEL_ID_CMN1,
                                  XVPHY_DP_LINK_RATE_HZ_162GBPS);
                break;
        case 0x14:
                XVphy_CfgQuadRefClkFreq(InstancePtr, 0,
                                        ONBOARD_REF_CLK,
                                        XVPHY_DP_REF_CLK_FREQ_HZ_270);
                XVphy_CfgQuadRefClkFreq(InstancePtr, 0,
                                        ONBOARD_REF_CLK,
                                        XVPHY_DP_REF_CLK_FREQ_HZ_270);
                XVphy_CfgLineRate(InstancePtr, 0, XVPHY_CHANNEL_ID_CHA,
                                  XVPHY_DP_LINK_RATE_HZ_540GBPS);
                XVphy_CfgLineRate(InstancePtr, 0, XVPHY_CHANNEL_ID_CMN1,
                                  XVPHY_DP_LINK_RATE_HZ_540GBPS);
                break;
        case 0x1E:
                XVphy_CfgQuadRefClkFreq(InstancePtr, 0,
                                        ONBOARD_REF_CLK,
                                        XVPHY_DP_REF_CLK_FREQ_HZ_270);
                XVphy_CfgQuadRefClkFreq(InstancePtr, 0,
                                        ONBOARD_REF_CLK,
                                        XVPHY_DP_REF_CLK_FREQ_HZ_270);
                XVphy_CfgLineRate(InstancePtr, 0, XVPHY_CHANNEL_ID_CHA,
                                  XVPHY_DP_LINK_RATE_HZ_810GBPS);
                XVphy_CfgLineRate(InstancePtr, 0, XVPHY_CHANNEL_ID_CMN1,
                                  XVPHY_DP_LINK_RATE_HZ_810GBPS);
                break;
        default:
                XVphy_CfgQuadRefClkFreq(InstancePtr, 0,
                                    ONBOARD_REF_CLK,
                                    XVPHY_DP_REF_CLK_FREQ_HZ_270);
                XVphy_CfgQuadRefClkFreq(InstancePtr, 0,
//                                  DP159_FORWARDED_CLK,
//                                  XVPHY_DP_REF_CLK_FREQ_HZ_135);
                                    ONBOARD_REF_CLK,
                                    XVPHY_DP_REF_CLK_FREQ_HZ_270);
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
* This function sets GT in 16-bits (2-Byte) or 32-bits (4-Byte) mode.
*
* @param	InstancePtr is a pointer to the Video PHY instance.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void PHY_Two_byte_set (XVphy *InstancePtr, u8 Tx_to_two_byte, u8 Rx_to_two_byte)
{

	u16 DrpVal;
	u16 WriteVal;
	u32 Status;
	u16 TX_DATA_WIDTH_REG = 0x7A;
	u16 TX_INT_DATAWIDTH_REG = 0x85;

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
//            xil_printf ("RX Channel configured for 2byte mode\r\n");
    }


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
//		xil_printf ("TX Channel configured for 2byte mode\r\n");
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



void DpPt_pe_vs_adjustHandler(void *InstancePtr){
//	u8 Buffer[2];
//	u16 RegisterAddress;
	if(PE_VS_ADJUST == 1){
		unsigned char preemp = 0;
		switch(DpTxSsInst.DpPtr->TxInstance.LinkConfig.PeLevel){
			case 0: preemp = XVPHY_GTHE3_PREEMP_DP_L0; break;
			case 1: preemp = XVPHY_GTHE3_PREEMP_DP_L1; break;
			case 2: preemp = XVPHY_GTHE3_PREEMP_DP_L2; break;
			case 3: preemp = XVPHY_GTHE3_PREEMP_DP_L3; break;
		}

		XVphy_SetTxPreEmphasis(&VPhyInst, 0, XVPHY_CHANNEL_ID_CH1, preemp);
		XVphy_SetTxPreEmphasis(&VPhyInst, 0, XVPHY_CHANNEL_ID_CH2, preemp);
		XVphy_SetTxPreEmphasis(&VPhyInst, 0, XVPHY_CHANNEL_ID_CH3, preemp);
		XVphy_SetTxPreEmphasis(&VPhyInst, 0, XVPHY_CHANNEL_ID_CH4, preemp);


		unsigned char diff_swing = 0;
		switch(DpTxSsInst.DpPtr->TxInstance.LinkConfig.VsLevel){
			case 0:
				switch(DpTxSsInst.DpPtr->TxInstance.LinkConfig.PeLevel){
				case 0:
					diff_swing = XVPHY_GTHE3_DIFF_SWING_DP_L0;
					break;
				case 1:
					diff_swing = XVPHY_GTHE3_DIFF_SWING_DP_L1;
					break;
				case 2:
					diff_swing = XVPHY_GTHE3_DIFF_SWING_DP_L2;
					break;
				case 3:
					diff_swing = XVPHY_GTHE3_DIFF_SWING_DP_L3;
					break;
				}
				break;
			case 1:
				switch(DpTxSsInst.DpPtr->TxInstance.LinkConfig.PeLevel){
				case 0:
					diff_swing = XVPHY_GTHE3_DIFF_SWING_DP_L1;
					break;
				case 1:
					diff_swing = XVPHY_GTHE3_DIFF_SWING_DP_L2;
					break;
				case 2:
				case 3:
					diff_swing = XVPHY_GTHE3_DIFF_SWING_DP_L3;
					break;
				}
				break;
			case 2:
				switch(DpTxSsInst.DpPtr->TxInstance.LinkConfig.PeLevel){
				case 0:
					diff_swing = XVPHY_GTHE3_DIFF_SWING_DP_L2;
					break;
				case 1:
				case 2:
				case 3:
					diff_swing = XVPHY_GTHE3_DIFF_SWING_DP_L3;
					break;
				}
				break;
			case 3:
				diff_swing = XVPHY_GTHE3_DIFF_SWING_DP_L3; break;
		}
		XVphy_SetTxVoltageSwing(&VPhyInst, 0, XVPHY_CHANNEL_ID_CH1,
				diff_swing);
		XVphy_SetTxVoltageSwing(&VPhyInst, 0, XVPHY_CHANNEL_ID_CH2,
				diff_swing);
		XVphy_SetTxVoltageSwing(&VPhyInst, 0, XVPHY_CHANNEL_ID_CH3,
				diff_swing);
		XVphy_SetTxVoltageSwing(&VPhyInst, 0, XVPHY_CHANNEL_ID_CH4,
				diff_swing);

	}

	if(DP141_ADJUST == 1){
		u8 data =0;
//		data = i2c_read_dp141(XPAR_IIC_0_BASEADDR, I2C_TI_DP141_ADDR, 0x02);
		unsigned char diff_swing;
		switch(DpTxSsInst.DpPtr->TxInstance.LinkConfig.VsLevel){
			case 0: diff_swing = 0x8; break;
			case 1: diff_swing = 0x8; break;
			case 2: diff_swing = 0x8; break;
			case 3: diff_swing = 0xC; break;
		}

		data = data & 0xF0;
		data |= diff_swing;


//		unsigned char preemp;
//		switch(DpTxSsInst.DpPtr->TxInstance.LinkConfig.PeLevel){
//			case 0: preemp = 0; break;
//			case 1: preemp = 1; break;
//			case 2: preemp = 3; break;
//			case 3: preemp = 7; break;
//		}
//		data |= (preemp << 4);

		i2c_write_dp141(XPAR_IIC_0_BASEADDR, I2C_TI_DP141_ADDR, 0x02, data);
		i2c_write_dp141(XPAR_IIC_0_BASEADDR, I2C_TI_DP141_ADDR, 0x05, data);
		i2c_write_dp141(XPAR_IIC_0_BASEADDR, I2C_TI_DP141_ADDR, 0x08, data);
		i2c_write_dp141(XPAR_IIC_0_BASEADDR, I2C_TI_DP141_ADDR, 0x0B, data);

	}

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

	u32 TimerVal = 0, TimerVal_pre;
	u32 NumTicks = (MicroSeconds * (
			XPAR_PROCESSOR_HIER_0_AXI_TIMER_0_CLOCK_FREQ_HZ / 1000000));

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
extern u8 tx_is_up;
extern u8 aud_started;

void DpPt_HpdEventHandler(void *InstancePtr)
{
	tx_is_up = 0;
	aud_started = 0;
	if (XDpTxSs_IsConnected(&DpTxSsInst)) {
		sink_power_down();
		sink_power_up();
		tx_is_reconnected = 1;
		xil_printf ("Cable Connected !!\r\n");
	}
	else
	{
		frameBuffer_stop_rd();
		xil_printf ("Cable Disconnected !!\r\n");
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
	u32 Status;
// 		Some monitors give HPD pulse repeatedly which causes HPD pulse function to
//		be executed huge number of time. Hence hpd_pulse interrupt is disabled
//		and then enabled when hpd_pulse function is executed
			XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr,
					XDP_TX_INTERRUPT_MASK,
					XDP_TX_INTERRUPT_MASK_HPD_PULSE_DETECTED_MASK);
			hpd_pulse_con_event = 1;
//			Status = XDp_TxDiscoverTopology(DpTxSsInst.DpPtr);
//			        if (Status != XST_SUCCESS) {
//			              printf("SS ERR:MST:Topology failed:"
//			                      "%ld.\n\r", Status);
//			              return XST_FAILURE;
//			        }

			xil_printf ("HPD pulse detected !!\r\n");
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
void hpd_pulse_con(XDpTxSs *InstancePtr, u8 only_tx)
{

    u32 Status;
	u8 lane0_sts = InstancePtr->UsrHpdPulseData.Lane0Sts;
	u8 lane2_sts = InstancePtr->UsrHpdPulseData.Lane2Sts;
	u8 laneAlignStatus = InstancePtr->UsrHpdPulseData.LaneAlignStatus;
	u8 bw_set = InstancePtr->UsrHpdPulseData.BwSet;
	u8 lane_set = InstancePtr->UsrHpdPulseData.LaneSet;

	u8 retrain_link=0;
	u8 NumStreams;

	int i = 0;


	if (!XVidC_EdidIsHeaderValid(InstancePtr->UsrHpdEventData.EdidOrg)) {
		XDp_TxGetEdidBlock(DpTxSsInst.DpPtr,
				DpTxSsInst.UsrHpdEventData.EdidOrg,0);
	}

	u8 checksumMatch = 0;
	while(checksumMatch == 0){
		if(CalculateChecksum(DpTxSsInst.UsrHpdEventData.EdidOrg, 128)){
			XDp_TxGetEdidBlock(DpTxSsInst.DpPtr,
					DpTxSsInst.UsrHpdEventData.EdidOrg,0);
			checksumMatch = 0;
		}else
			checksumMatch = 1;
	}

     lane_set = lane_set & 0x1F;
     bw_set = bw_set & 0x1F;
     laneAlignStatus = laneAlignStatus & 0x1;

     if(bw_set != XDP_TX_LINK_BW_SET_162GBPS
             && bw_set != XDP_TX_LINK_BW_SET_270GBPS
             && bw_set != XDP_TX_LINK_BW_SET_540GBPS
			 && bw_set != XDP_TX_LINK_BW_SET_810GBPS){
        bw_set = DpTxSsInst.DpPtr->TxInstance.LinkConfig.LinkRate;
        retrain_link = 1;
     }
     if(lane_set != XDP_TX_LANE_COUNT_SET_1
             && lane_set != XDP_TX_LANE_COUNT_SET_2
             && lane_set != XDP_TX_LANE_COUNT_SET_4){
        lane_set = DpTxSsInst.DpPtr->TxInstance.LinkConfig.LaneCount;
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



		if (DpTxSsInst.UsrOpt.MstSupport != 0 ) {
			/* Clear node and sink */
			DpTxSsInst.DpPtr->TxInstance.Topology.NodeTotal = 0;
			DpTxSsInst.DpPtr->TxInstance.Topology.SinkTotal = 0;

			/* Discover topology and find total sinks */
			Status = XDp_TxDiscoverTopology(DpTxSsInst.DpPtr);
			if (Status != XST_SUCCESS) {
				  xil_printf("SS ERR:MST:Topology failed:"
						  "%ld.\n\r", Status);
			}

			/* Total number of streams equivalent to number of sinks found */
			NumStreams = DpTxSsInst.DpPtr->TxInstance.Topology.SinkTotal;
			if (NumStreams != num_sinks) {
				if (NumStreams > num_sinks) {
					retrain_link = 1;
					xil_printf ("Number of Sinks increased..retraining\r\n");
				} else {
					num_sinks = NumStreams;
					xil_printf ("Number of Sinks reduced..\r\n");
				}
			} else {
				retrain_link |= retrain_link;
			}

		}

	if(retrain_link == 1 && only_tx == 1){
		aud_started = 0;
		XDpTxSs_SetLinkRate(&DpTxSsInst, bw_set);
		XDpTxSs_SetLaneCount(&DpTxSsInst, lane_set);
		Status = DpTxSubsystem_Start(&DpTxSsInst, 0);
		if (Status != XST_SUCCESS) {
				xdbg_printf(XDBG_DEBUG_GENERAL,"SS ERR: "
						"SS Start failed!\n\r");
		}

		Vpg_StreamSrcConfigure(DpTxSsInst.DpPtr, 0, 1);
		clk_wiz_locked();
		num_sinks = DpTxSsInst.DpPtr->TxInstance.Topology.SinkTotal;
#if (XPAR_XDUALSPLITTER_NUM_INSTANCES > 0)
		Status = XDpTxSs_DsSetup(DpTxSsInst.DsPtr, 0,
				&DpTxSsInst.DpPtr->TxInstance.MsaConfig[0]);
#endif

		for (i=0;i<num_sinks;i++) {
			if (DpTxSsInst.VtcPtr[i]) {
				Status |= XDpTxSs_VtcSetup(DpTxSsInst.VtcPtr[i],
				&DpTxSsInst.DpPtr->TxInstance.MsaConfig[i],
				DpTxSsInst.UsrOpt.VtcAdjustBs);
				if (Status != XST_SUCCESS) {
						xdbg_printf(XDBG_DEBUG_GENERAL,"SS ERR: "
								"VTC setup failed!\n\r");
				}
			}
		}

	}

	if (retrain_link == 1 && only_tx == 0) {
		XDpTxSs_SetLinkRate(&DpTxSsInst, bw_set);
		XDpTxSs_SetLaneCount(&DpTxSsInst, lane_set);
		Status = DpTxSubsystem_Start(&DpTxSsInst, Msa);

#if (XPAR_XDUALSPLITTER_NUM_INSTANCES > 0)
		Status = XDpTxSs_DsSetup(DpTxSsInst.DsPtr, 0,
				&DpTxSsInst.DpPtr->TxInstance.MsaConfig[0]);
#endif

		Status |= XDpTxSs_VtcSetup(DpTxSsInst.VtcPtr[0],
		&DpTxSsInst.DpPtr->TxInstance.MsaConfig[0],
		DpTxSsInst.UsrOpt.VtcAdjustBs);
		if (Status != XST_SUCCESS) {
				xdbg_printf(XDBG_DEBUG_GENERAL,"SS ERR: "
						"VTC setup failed!\n\r");
		}
	}
	tx_is_up = 1;
     XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr,XDP_TX_INTERRUPT_MASK, 0x0);
}



// introduced to address reduced blanking linereset issue from 16.4 release
u32 DpTxSubsystem_Start(XDpTxSs *InstancePtr,
			XDpTxSs_MainStreamAttributes Msa[4]) {
	u32 Status;
//	xil_printf ("MSA in TxSSStart is %d\r\n",Msa);
	if (Msa == 0) {
		Status = XDpTxSs_Start(&DpTxSsInst);
	} else {
		Status = XDpTxSs_StartCustomMsa(&DpTxSsInst, Msa);
	}

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


	DpTxSsInst.DpPtr->TxInstance.MsaConfig[0].ComponentFormat = 0x0;

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

		u8 rData = 0;
		// check the EXTENDED_RECEIVER_CAPABILITY_FIELD_PRESENT bit
		XDp_TxAuxRead(DpTxSsInst.DpPtr, XDP_DPCD_TRAIN_AUX_RD_INTERVAL, 1, &rData);
		if(rData & 0x80){ // if EXTENDED_RECEIVER_CAPABILITY_FIELD is enabled
			XDp_TxAuxRead(DpTxSsInst.DpPtr, 0x2201, 1, &rData); // read maxLineRate
			if(rData == XDP_DPCD_LINK_BW_SET_810GBPS){
				*LineRate_init = 0x1E;
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
			if (Status != XST_SUCCESS)
				xil_printf ("Failed to read sink capabilities\r\n");
		}
	} else {
		xil_printf("Please connect a DP Monitor and try again !!!\r\n");
		return;
	}

	*LineRate_init &= 0xFF;
	*LaneCount_init &= 0xF;

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
			(XVPHY_PLL_RESET_QPLL0_MASK | XVPHY_PLL_RESET_QPLL1_MASK)); // 0x06
	XVphy_WriteReg(InstancePtr->Config.BaseAddr, XVPHY_PLL_RESET_REG, 0x0);


	Status = XVphy_ResetGtPll(InstancePtr, QuadId,
			XVPHY_CHANNEL_ID_CHA, XVPHY_DIR_TX,(FALSE));


	Status += XVphy_WaitForPmaResetDone(InstancePtr, QuadId,
			TxChId, XVPHY_DIR_TX);
	Status += XVphy_WaitForPllLock(InstancePtr, QuadId, TxChId);
	Status += XVphy_WaitForResetDone(InstancePtr, QuadId,
			TxChId, XVPHY_DIR_TX);
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
u32 start_tx(u8 line_rate, u8 lane_count, user_config_struct user_config,
			XDpTxSs_MainStreamAttributes Msa[4])
{
	XVidC_VideoMode res_table = user_config.VideoMode_local;
	u8 bpc = user_config.user_bpc;
	u8 pat = user_config.user_pattern;
	u8 format = user_config.user_format-1;
	u8 C_VideoUserStreamPattern[8] = {0x10, 0x11, 0x12, 0x13, 0x14,
												0x15, 0x16, 0x17}; //Duplicate
	int i = 0;
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

	xil_printf("\r\nTraining TX with: Link rate %x, Lane count %d\r\n",
		   line_rate,lane_count);

	XDpTxSs_SetLinkRate(&DpTxSsInst, line_rate);
	xil_printf (".");
	XDpTxSs_SetLaneCount(&DpTxSsInst, lane_count);
	xil_printf (".");

//	if (res_table !=0) {
//		Status = XDpTxSs_SetVidMode(&DpTxSsInst, res_table);
//		if (Status != XST_SUCCESS) {
//			xil_printf("ERR: Setting resolution failed\r\n");
//		}
//		xil_printf (".");
//	}
//	if (bpc !=0 ) {
//		Status = XDpTxSs_SetBpc(&DpTxSsInst, bpc);
//		if (Status != XST_SUCCESS){
//			xil_printf("ERR: Setting bpc to %d failed\r\n",bpc);
//		}
//		xil_printf (".");
//	}

	Status = DpTxSubsystem_Start(&DpTxSsInst, Msa);
	if (Status != XST_SUCCESS) {
			xdbg_printf(XDBG_DEBUG_GENERAL,"SS ERR: "
					"SS start failed!\n\r");
	}

	xil_printf (".");
	//updates required timing values in Video Pattern Generator
	Vpg_StreamSrcConfigure(DpTxSsInst.DpPtr, 0, 1);
	xil_printf (".");
	// setting video pattern
	Vpg_VidgenSetUserPattern(DpTxSsInst.DpPtr,
				 C_VideoUserStreamPattern[pat]);
    /* Generate the video clock using MMCM
     */
    Gen_vid_clk(DpTxSsInst.DpPtr,(XDP_TX_STREAM_ID1));

	clk_wiz_locked();
	xil_printf (".");
	//Keeping the splitter in false mode
#if (XPAR_XDUALSPLITTER_NUM_INSTANCES > 0)
	Status = XDpTxSs_DsSetup(DpTxSsInst.DsPtr, 0,
			&DpTxSsInst.DpPtr->TxInstance.MsaConfig[0]);
#endif

	Status |= XDpTxSs_VtcSetup(DpTxSsInst.VtcPtr[0],
	&DpTxSsInst.DpPtr->TxInstance.MsaConfig[0],
	DpTxSsInst.UsrOpt.VtcAdjustBs);
	if (Status != XST_SUCCESS) {
			xdbg_printf(XDBG_DEBUG_GENERAL,"SS ERR: "
					"VTC setup failed!\n\r");
	}

//	Status = DpTxSubsystem_Start(&DpTxSsInst, Msa);
	xil_printf (".");
	Status = XDpTxSs_CheckLinkStatus(&DpTxSsInst);
	if (Status != (XST_SUCCESS)) {
		Status = DpTxSubsystem_Start(&DpTxSsInst, Msa);
		if (Status != XST_SUCCESS) {
			xil_printf("ERR:DPTX SS start failed\r\n");
			return (XST_FAILURE);
		}
	}
	xil_printf ("..done !\r\n");

	num_sinks = DpTxSsInst.DpPtr->TxInstance.Topology.SinkTotal;
	if (num_sinks == 0) {
		num_sinks = 1;
	}
	xil_printf ("MST trained with a total %d sinks\r\n", num_sinks);
	return XST_SUCCESS;
}

//
u32 start_tx_only(u8 line_rate, u8 lane_count,user_config_struct user_config){
	XVidC_VideoMode res_table = user_config.VideoMode_local;
	u8 bpc = user_config.user_bpc;
	u8 pat = user_config.user_pattern;
	u8 format = user_config.user_format-1;
	u8 C_VideoUserStreamPattern[8] = {0x10, 0x11, 0x12, 0x13, 0x14,
												0x15, 0x16, 0x17}; //Duplicate

    int i = 0;
	num_sinks = 0;

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

	// VTC requires linkup(video clk) before setting values.
	Status = DpTxSubsystem_Start(&DpTxSsInst, 0);
	if (Status != XST_SUCCESS) {
			xdbg_printf(XDBG_DEBUG_GENERAL,"SS ERR: "
					"SS Start failed!\n\r");
	}
	xil_printf (".");
	//updates required timing values in Video Pattern Generator
	Vpg_StreamSrcConfigure(DpTxSsInst.DpPtr, 0, 1);
    Gen_vid_clk(DpTxSsInst.DpPtr,(XDP_TX_STREAM_ID1));
	xil_printf (".");
	// setting video pattern
	xil_printf (".");
	clk_wiz_locked();

	num_sinks = DpTxSsInst.DpPtr->TxInstance.Topology.SinkTotal;
	// Keeping splitter in False mode
#if (XPAR_XDUALSPLITTER_NUM_INSTANCES > 0)
	Status = XDpTxSs_DsSetup(DpTxSsInst.DsPtr, 0,
			&DpTxSsInst.DpPtr->TxInstance.MsaConfig[0]);
#endif

	for (i=0;i<num_sinks;i++) {
		if (DpTxSsInst.VtcPtr[i]) {
			Status |= XDpTxSs_VtcSetup(DpTxSsInst.VtcPtr[i],
			&DpTxSsInst.DpPtr->TxInstance.MsaConfig[i],
			DpTxSsInst.UsrOpt.VtcAdjustBs);
			if (Status != XST_SUCCESS) {
					xdbg_printf(XDBG_DEBUG_GENERAL,"SS ERR: "
							"VTC setup failed!\n\r");
			}
		}
	}

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
	xil_printf ("..done !\r\n");
	xil_printf ("MST trained with a total %d sinks\r\n", num_sinks);
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

//	volatile u32 res = XDp_ReadReg(XPAR_GPIO_0_BASEADDR,0x0);
//	u32 timer=0;
//
//	while ( res == 0 && timer < 1000) {
//		xil_printf ("~/~/");
//		res = XDp_ReadReg(XPAR_GPIO_0_BASEADDR,0x0);
//		timer++; // timer for timeout. No need to be specific time.
//					// As long as long enough to wait lock
//	}
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

//	xil_printf ("Inside hpd_con...\r\n");
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
// This is moved to driver in 2019.2
#if 0
void sendAudioInfoFrame(XilAudioInfoFrame *xilInfoFrame)
{
    u8 db1, db2, db3, db4;
    u32 temp;
    u8 RSVD=0;

    //Fixed paramaters
    u8  dp_version   = xilInfoFrame->version;

        //Write #1
    db1 = 0x00; //sec packet ID fixed to 0 - SST Mode
    db2 = xilInfoFrame->type;
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
#endif

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
		case XDP_TX_LINK_BW_SET_810GBPS:
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

		case XDP_TX_LINK_BW_SET_810GBPS:
			Status = PHY_Configuration_Tx(&VPhyInst,
						PHY_User_Config_Table[(is_TX_CPLL)?9:10]);
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
//	sink_power_down();
//	// give enough time for monitor to power down
//	usleep(40000);
	DpPt_CustomWaitUs(DpTxSsInst.DpPtr, 4000);
//	DpPt_CustomWaitUs(DpTxSsInst.DpPtr, 40000);
	sink_power_up();
//	// give enough time for monitor to wake up    CR-962717
	DpPt_CustomWaitUs(DpTxSsInst.DpPtr, 50000);
	sink_power_up();//monitor to wake up once again due to CR-962717
	DpPt_CustomWaitUs(DpTxSsInst.DpPtr, 50000);
//	sink_power_up();//monitor to wake up once again due to CR-962717
//	DpPt_CustomWaitUs(DpTxSsInst.DpPtr, 400000);

}


int VideoFMC_Init(void){
	int Status;
	u8 Buffer[2];
	int ByteCount;



    XIic_Config *ConfigPtr_IIC;     /* Pointer to configuration data */
    /*
     * Initialize the IIC driver so that it is ready to use.
     */
    ConfigPtr_IIC = XIic_LookupConfig(IIC_DEVICE_ID);
    if (ConfigPtr_IIC == NULL) {
            return XST_FAILURE;
    }

    Status = XIic_CfgInitialize(&IicInstance, ConfigPtr_IIC, ConfigPtr_IIC->BaseAddress);
    if (Status != XST_SUCCESS) {
            return XST_FAILURE;
    }


    XIic_Reset(&IicInstance);

	/* Set the I2C Mux to select the HPC FMC */
//	Buffer[0] = 0x05;
	Buffer[0] = 0x07; // Enable both HPC0 and HPC1
	ByteCount = XIic_Send(XPAR_IIC_0_BASEADDR, I2C_MUX_ADDR, (u8*)Buffer, 1, XIIC_STOP);
	if (ByteCount != 1) {
		xil_printf("Failed to set the I2C Mux.\n\r");
	    return XST_FAILURE;
	}

	/* Configure VFMC IO Expander 0:
	 * Disable Si5344
	 * Set primary clock source for LMK03318 to IOCLKp(0)
	 * Set secondary clock source for LMK03318 to IOCLKp(1)
	 * Disable LMK61E2*/
	Buffer[0] = 0x52;
	ByteCount = XIic_Send(XPAR_IIC_0_BASEADDR, I2C_VFMCEXP_0_ADDR, (u8*)Buffer, 1, XIIC_STOP);
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
	ByteCount = XIic_Send(XPAR_IIC_0_BASEADDR, I2C_VFMCEXP_1_ADDR, (u8*)Buffer, 1, XIIC_STOP);
	if (ByteCount != 1) {
		xil_printf("Failed to set the I2C IO Expander.\n\r");
		return XST_FAILURE;
	}

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

	xil_printf("VFMC init done...\n\r");

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
//		Buffer[0] = (RegisterAddress >> 8);
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
//			ByteCount = XIic_Recv(I2CBaseAddress, I2CSlaveAddress, (u8*)Buffer, 2, XIIC_STOP);
//			if (ByteCount != 1) {
//				Exit = FALSE;
//				Exit = TRUE;
			//}

//			else {
				Data = Buffer[0];
				Exit = TRUE;
//			}
		}
	} while (!Exit);

	return Data;
}


int i2c_write_dp141(u32 I2CBaseAddress, u8 I2CSlaveAddress, u16 RegisterAddress, u8 Value)
{
	u32 ByteCount = 0;
	u8 Buffer[2];
	u8 Retry = 0;

	// Write data
//	Buffer[0] = (RegisterAddress >> 8);
	Buffer[0] = RegisterAddress & 0xff;
	Buffer[1] = Value;

	while (1) {
		ByteCount = XIic_Send(I2CBaseAddress, I2CSlaveAddress, (u8*)Buffer, 3, XIIC_STOP);

		if (ByteCount != 2) {
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

void read_DP141(){
	u8 Data;
	int i =0;

	for(i=0; i<0xD; i++){
		Data = i2c_read_dp141( XPAR_IIC_0_BASEADDR, I2C_TI_DP141_ADDR, i);
		xil_printf("%x : %02x \r\n",i, Data);
	}

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

        /*Disable All Interrupts*/
        XDp_RxInterruptDisable(DpRxSsInst.DpPtr, 0xFFFFFFFF);
        XDp_RxInterruptDisable1(DpRxSsInst.DpPtr, 0xFFFFFFFF);

        /*Enable Training related interrupts*/
        XDp_RxInterruptEnable(DpRxSsInst.DpPtr,
                        XDP_RX_INTERRUPT_MASK_TP1_MASK |
                        XDP_RX_INTERRUPT_MASK_TP2_MASK |
                        XDP_RX_INTERRUPT_MASK_TP3_MASK|
                        XDP_RX_INTERRUPT_MASK_POWER_STATE_MASK|
                        XDP_RX_INTERRUPT_MASK_CRC_TEST_MASK|
                        XDP_RX_INTERRUPT_MASK_BW_CHANGE_MASK |
						XDP_RX_INTERRUPT_MASK_VCP_ALLOC_MASK |
						XDP_RX_INTERRUPT_MASK_VCP_DEALLOC_MASK |
						XDP_RX_INTERRUPT_MASK_DOWN_REPLY_MASK |
						XDP_RX_INTERRUPT_MASK_DOWN_REQUEST_MASK);

        XDp_RxInterruptEnable1(DpRxSsInst.DpPtr,
                        XDP_RX_INTERRUPT_MASK_TP4_MASK|
                        XDP_RX_INTERRUPT_MASK_ACCESS_LANE_SET_MASK|
                        XDP_RX_INTERRUPT_MASK_ACCESS_LINK_QUAL_MASK|
                        XDP_RX_INTERRUPT_MASK_ACCESS_ERROR_COUNTER_MASK);

        XDp_RxInterruptEnable1(DpRxSsInst.DpPtr,
                        0xF0003FFFF);


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

        /*Disabling timeout */
        ReadVal = XDp_ReadReg(DpRxSsInst.DpPtr->Config.BaseAddr,
			XDP_RX_CDR_CONTROL_CONFIG);

		XDp_WriteReg(DpRxSsInst.DpPtr->Config.BaseAddr,
				XDP_RX_CDR_CONTROL_CONFIG,
				ReadVal |
				XDP_RX_CDR_CONTROL_CONFIG_DISABLE_TIMEOUT);

        if(LINK_TRAINING_DEBUG==1){
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

        /*Enable Rx*/
        XDp_WriteReg(DpRxSsInst.DpPtr->Config.BaseAddr,
                     XDP_RX_LINK_ENABLE, 0x1);

        return XST_SUCCESS;
}

void resetIp_wr()
{


	Xil_Out32(HLS_RST_BASE, 0x1);
	usleep(10000);          //hold reset line
//	power_up_HLSIPs();
	Xil_Out32(HLS_RST_BASE, 0x3);
	usleep(10000);          //hold reset line
//	power_down_HLSIPs();
	Xil_Out32(HLS_RST_BASE, 0x1);
	usleep(10000);          //hold reset line
//	power_up_HLSIPs();
	Xil_Out32(HLS_RST_BASE, 0x3);
	usleep(10000);          //hold reset line

}

void resetIp_rd()
{


	Xil_Out32(HLS_RST_BASE, 0x2);
//	usleep(10000);          //hold reset line
//	power_up_HLSIPs();
	Xil_Out32(HLS_RST_BASE, 0x3);
//	usleep(10000);          //hold reset line
//	power_down_HLSIPs();
	Xil_Out32(HLS_RST_BASE, 0x2);
//	usleep(10000);          //hold reset line
//	power_up_HLSIPs();
	Xil_Out32(HLS_RST_BASE, 0x3);
//	usleep(10000);          //hold reset line

}



/* This process takes in all the MSA values and find out resolution,
 * BPC, refresh rate. Further this sets the pixel_width based on the
 * pixel_clock and lane set this is to ensure that it matches the
 * values in TX driver. Else video cannot be pass-through. Approximation
 * is implemented for refresh rates. Sometimes a refresh rate of 60
 * is detected as 59 and vice-versa. Approximation is done for single digit.
 */

u8 tx_ppc_set = 0;
u8 stream_id_used = 0;
u8 invalid_stream = 0;
void Dppt_DetectResolution(void *InstancePtr, u16 offset,
							XDpTxSs_MainStreamAttributes Msa[4], u8 stream){

	u32 DpHres = 0;
	u32 DpVres = 0;
	int i = 0;
	u8 msa_offset = stream;//1; //Always copy all into MSA[0] as TX is single
	stream_id_used = offset;

	while ((DpHres == 0 && i < 8000) && DpRxSsInst.link_up_trigger == 1) {
	    DpHres = XDp_ReadReg(DpRxSsInst.DpPtr->Config.BaseAddr,
			XDP_RX_MSA_HRES+(0x40*(offset-1)));
	    i++;
	}
	i= 0;
	while ((DpVres == 0 && i < 8000) && DpRxSsInst.link_up_trigger == 1) {
		DpVres = XDp_ReadReg(DpRxSsInst.DpPtr->Config.BaseAddr,
			XDP_RX_MSA_VHEIGHT+(0x40*(offset-1)));
		i++;
	}

	// Assuming rest of the MSA would be stable by this time
	u32 DpHres_total = XDp_ReadReg(DpRxSsInst.DpPtr->Config.BaseAddr,
			XDP_RX_MSA_HTOTAL+(0x40*(offset-1)));
	u32 DpVres_total = XDp_ReadReg(DpRxSsInst.DpPtr->Config.BaseAddr,
			XDP_RX_MSA_VTOTAL+(0x40*(offset-1)));
	u32 rxMsamisc0 = XDp_ReadReg(DpRxSsInst.DpPtr->Config.BaseAddr,
			XDP_RX_MSA_MISC0+(0x40*(offset-1)));
	u32 rxMsamisc1 = XDp_ReadReg(DpRxSsInst.DpPtr->Config.BaseAddr,
			XDP_RX_MSA_MISC1+(0x40*(offset-1)));
	u32 rxMsaMVid = XDp_ReadReg(DpRxSsInst.DpPtr->Config.BaseAddr,
			XDP_RX_MSA_MVID+(0x40*(offset-1)));
	u32 rxMsaNVid = XDp_ReadReg(DpRxSsInst.DpPtr->Config.BaseAddr,
			XDP_RX_MSA_NVID+(0x40*(offset-1)));

	Msa[msa_offset-1].Misc0 = rxMsamisc0;
	Msa[msa_offset-1].Misc1 = rxMsamisc1;
	rxMsamisc0 = ((rxMsamisc0 >> 5) & 0x00000007);

	u8 Bpc[] = {6, 8, 10, 12, 16};

	Msa[msa_offset-1].Vtm.Timing.HActive = DpHres;
	Msa[msa_offset-1].Vtm.Timing.VActive = DpVres;
	Msa[msa_offset-1].Vtm.Timing.HTotal = DpHres_total;
	Msa[msa_offset-1].Vtm.Timing.F0PVTotal = DpVres_total;
	Msa[msa_offset-1].MVid = rxMsaMVid;
	Msa[msa_offset-1].NVid = rxMsaNVid;
	Msa[msa_offset-1].HStart =
			XDp_ReadReg(DpRxSsInst.DpPtr->Config.BaseAddr, XDP_RX_MSA_HSTART+(0x40*(offset-1)));
	Msa[msa_offset-1].VStart =
			XDp_ReadReg(DpRxSsInst.DpPtr->Config.BaseAddr, XDP_RX_MSA_VSTART+(0x40*(offset-1)));

	Msa[msa_offset-1].Vtm.Timing.HSyncWidth =
			XDp_ReadReg(DpRxSsInst.DpPtr->Config.BaseAddr, XDP_RX_MSA_HSWIDTH+(0x40*(offset-1)));
	Msa[msa_offset-1].Vtm.Timing.F0PVSyncWidth =
			XDp_ReadReg(DpRxSsInst.DpPtr->Config.BaseAddr, XDP_RX_MSA_VSWIDTH+(0x40*(offset-1)));

	Msa[msa_offset-1].Vtm.Timing.HSyncPolarity =
			XDp_ReadReg(DpRxSsInst.DpPtr->Config.BaseAddr, XDP_RX_MSA_HSPOL+(0x40*(offset-1)));
	Msa[msa_offset-1].Vtm.Timing.VSyncPolarity =
			XDp_ReadReg(DpRxSsInst.DpPtr->Config.BaseAddr, XDP_RX_MSA_VSPOL+(0x40*(offset-1)));


	Msa[msa_offset-1].SynchronousClockMode = 0; //rxMsamisc0 & 1;
	u8 bpc = Bpc[rxMsamisc0];
	Msa[msa_offset-1].BitsPerColor = bpc;
//	Msa[0].Misc0 = rxMsamisc0;
//	Msa[0].Misc1 = rxMsamisc1;

	/* Check for YUV422, BPP has to be set using component value to 2 */
	if( (Msa[msa_offset-1].Misc0 & 0x6 ) == 0x2  ) {
	//YUV422
		Msa[msa_offset-1].ComponentFormat =
				XDP_TX_MAIN_STREAMX_MISC0_COMPONENT_FORMAT_YCBCR422;
	}
	else if( (Msa[msa_offset-1].Misc0 & 0x6 ) == 0x4  ) {
	//RGB or YUV444
		Msa[msa_offset-1].ComponentFormat =
				XDP_TX_MAIN_STREAMX_MISC0_COMPONENT_FORMAT_YCBCR444;
	}else
		Msa[msa_offset-1].ComponentFormat =
				XDP_TX_MAIN_STREAMX_MISC0_COMPONENT_FORMAT_RGB;


	u32 recv_clk_freq =
		(((int)DpRxSsInst.UsrOpt.LinkRate*27)*rxMsaMVid)/rxMsaNVid;
//	xil_printf ("Rec clock is %d\r\n",recv_clk_freq);

	float recv_frame_clk =
		(int)( (recv_clk_freq*1000000.0)/(DpHres_total * DpVres_total) < 0.0 ?
				(recv_clk_freq*1000000.0)/(DpHres_total * DpVres_total) :
				(recv_clk_freq*1000000.0)/(DpHres_total * DpVres_total)+0.9
				);

	XVidC_FrameRate recv_frame_clk_int = recv_frame_clk;
	//Doing Approximation here
	if (recv_frame_clk_int == 49 || recv_frame_clk_int == 51) {
		recv_frame_clk_int = 50;
	} else if (recv_frame_clk_int == 59 || recv_frame_clk_int == 61) {
		recv_frame_clk_int = 60;
	} else if (recv_frame_clk_int == 29 || recv_frame_clk_int == 31) {
		recv_frame_clk_int = 30;
	} else if (recv_frame_clk_int == 76 || recv_frame_clk_int == 74) {
		recv_frame_clk_int = 75;
	} else if (recv_frame_clk_int == 121 || recv_frame_clk_int == 119) {
		recv_frame_clk_int = 120;
	}

	Msa[msa_offset-1].Vtm.FrameRate = recv_frame_clk_int;
	Msa[msa_offset-1].PixelClockHz = DpHres_total * DpVres_total * recv_frame_clk_int;
	Msa[msa_offset-1].DynamicRange = XDP_DR_CEA;
	Msa[msa_offset-1].YCbCrColorimetry = XDP_TX_MAIN_STREAMX_MISC0_YCBCR_COLORIMETRY_BT601;

	if((recv_clk_freq*1000000)>540000000
			&& (int)DpRxSsInst.UsrOpt.LaneCount==4){
		Msa[msa_offset-1].UserPixelWidth = 0x4;
		tx_ppc_set = 0x4;
	}
	else if((recv_clk_freq*1000000)>270000000
			&& (int)DpRxSsInst.UsrOpt.LaneCount!=1){
		Msa[msa_offset-1].UserPixelWidth = 0x2;
		tx_ppc_set = 0x2;
	}
	else{
		Msa[msa_offset-1].UserPixelWidth = 0x1;
		tx_ppc_set = 0x1;
	}

	Msa[msa_offset-1].OverrideUserPixelWidth = 1;
//	XDp_RxSetLineReset(DpRxSsInst.DpPtr,offset);
//	usleep (400000);

	if (DpHres != 0) {

    xil_printf("*** Stream %d:\r\n", offset);
	xil_printf("*** Detected Resolution: "
				"%lu x %lu @ %luHz, BPC = %lu, PPC = 4***\n\r",
			DpHres, DpVres,recv_frame_clk_int,bpc
		);
	    invalid_stream = 0;
	} else {
		invalid_stream = 1;
	}


}

void Dprx_ResetVideoOutput(void *InstancePtr)
{
        XDp_RxDtgDis(DpRxSsInst.DpPtr);
		XDp_RxDtgEn(DpRxSsInst.DpPtr);

}


void frameBuffer_stop_wr() {
	u32 Status;
	Status = XVFrmbufWr_Stop(&frmbufwr);
	if(Status != XST_SUCCESS) {
		xil_printf("Failed to stop XVFrmbufWr\r\n");
	}
	resetIp_wr();
	Status = XVFrmbufWr_WaitForIdle(&frmbufwr);
	if (Status != XST_SUCCESS) {
		xil_printf ("Frame Buffer is not Idle\r\n");
	}
}


void frameBuffer_stop_rd() {
	u32 Status;
	Status = XVFrmbufRd_Stop(&frmbufrd);
	if(Status != XST_SUCCESS) {
		xil_printf("Failed to stop XVFrmbufRd\r\n");
	}
	resetIp_rd();
	Status = XVFrmbufRd_WaitForIdle(&frmbufrd);
	if (Status != XST_SUCCESS) {
		xil_printf ("Frame Buffer is not Idle\r\n");
	}
}

void frameBuffer_stop() {
	frameBuffer_stop_rd();
	frameBuffer_stop_wr();
}


/*****************************************************************************/
/**
 * This function calculates the stride
 *
 * @returns stride in bytes
 *
 *****************************************************************************/
u32 CalcStride(XVidC_ColorFormat Cfmt,
					  u16 AXIMMDataWidth,
					  XVidC_VideoStream *StreamPtr)
{
	u32 stride;
	int width = StreamPtr->Timing.HActive; //forcing 4 PPC
	u16 MMWidthBytes = AXIMMDataWidth/8;



	if ((Cfmt == XVIDC_CSF_MEM_Y_UV10) || (Cfmt == XVIDC_CSF_MEM_Y_UV10_420)
	  || (Cfmt == XVIDC_CSF_MEM_Y10)) {
	// 4 bytes per 3 pixels (Y_UV10, Y_UV10_420, Y10)
	stride = ((((width*4)/3)+MMWidthBytes-1)/MMWidthBytes)*MMWidthBytes;

	}
	else if ((Cfmt == XVIDC_CSF_MEM_Y_UV8) || (Cfmt == XVIDC_CSF_MEM_Y_UV8_420)
		   || (Cfmt == XVIDC_CSF_MEM_Y8)) {
	// 1 byte per pixel (Y_UV8, Y_UV8_420, Y8)
	stride = ((width+MMWidthBytes-1)/MMWidthBytes)*MMWidthBytes;

	}
	else if ((Cfmt == XVIDC_CSF_MEM_RGB8) || (Cfmt == XVIDC_CSF_MEM_YUV8)) {
	// 3 bytes per pixel (RGB8, YUV8)
	stride = (((width*3)+MMWidthBytes-1)/MMWidthBytes)*MMWidthBytes;
	}  else if (Cfmt == XVIDC_CSF_MEM_YUYV8) {
        stride = (((width*2)+MMWidthBytes-1)/MMWidthBytes)*MMWidthBytes;

	} else {
	// 4 bytes per pixel
	stride = (((width*4)+MMWidthBytes-1)/MMWidthBytes)*MMWidthBytes;

	}

	return(stride);
}

/*****************************************************************************/
/**
 * This function configures Frame BufferWr for defined mode
 *
 * @return XST_SUCCESS if init is OK else XST_FAILURE
 *
 *****************************************************************************/
int ConfigFrmbuf_wr(u32 StrideInBytes,
						XVidC_ColorFormat Cfmt,
						XVidC_VideoStream *StreamPtr){
	int Status;

	/* Stop Frame Buffers */
//	Status = XVFrmbufWr_Stop(&frmbufwr);
//	if(Status != XST_SUCCESS) {
//		xil_printf("Failed to stop XVFrmbufWr\r\n");
//	}
	XVFRMBUFWR_BUFFER_BASEADDR = (0 + (0x10000000) + 0x08000000);

	Status = XVFrmbufWr_SetMemFormat(&frmbufwr, StrideInBytes, Cfmt, StreamPtr);
	if(Status != XST_SUCCESS) {
		xil_printf("ERROR:: Unable to configure Frame Buffer Write\r\n");
		return(XST_FAILURE);
	}

	Status = XVFrmbufWr_SetBufferAddr(&frmbufwr, XVFRMBUFWR_BUFFER_BASEADDR);
	if(Status != XST_SUCCESS) {
		xil_printf("ERROR:: Unable to configure Frame Buffer Write "
			"buffer address\r\n");
		return(XST_FAILURE);
	}


	/* Enable Interrupt */
	XVFrmbufWr_InterruptEnable(&frmbufwr,
			XVFRMBUFWR_HANDLER_READY | XVFRMBUFWR_HANDLER_DONE);

	XV_frmbufwr_EnableAutoRestart(&frmbufwr.FrmbufWr);
	/* Start Frame Buffers */
	XVFrmbufWr_Start(&frmbufwr);

	//xil_printf("INFO: FRMBUFwr configured\r\n");
	return(Status);
}


/*****************************************************************************/
/**
 * This function configures Frame Buffer for defined mode
 *
 * @return XST_SUCCESS if init is OK else XST_FAILURE
 *
 *****************************************************************************/
int ConfigFrmbuf_rd(u32 StrideInBytes,
						XVidC_ColorFormat Cfmt,
						XVidC_VideoStream *StreamPtr)
	{

	int Status;

	/* Stop Frame Buffers */
//	Status = XVFrmbufRd_Stop(&frmbufrd);
//	if(Status != XST_SUCCESS) {
//		xil_printf("Failed to stop XVFrmbufRd\r\n");
//	}

	XVFRMBUFRD_BUFFER_BASEADDR = (0 + (0x10000000));

	/* Configure  Frame Buffers */
	Status = XVFrmbufRd_SetMemFormat(&frmbufrd, StrideInBytes, Cfmt, StreamPtr);
	if(Status != XST_SUCCESS) {
		xil_printf("ERROR:: Unable to configure Frame Buffer Read\r\n");
		return(XST_FAILURE);
	}

	Status = XVFrmbufRd_SetBufferAddr(&frmbufrd, XVFRMBUFRD_BUFFER_BASEADDR);
	if(Status != XST_SUCCESS) {
		xil_printf("ERROR:: Unable to configure Frame Buffer "
				"Read buffer address\r\n");
		return(XST_FAILURE);
	}

	/* Enable Interrupt */
//	XVFrmbufRd_InterruptEnable(&frmbufrd, 0);

	XV_frmbufrd_EnableAutoRestart(&frmbufrd.FrmbufRd);
	/* Start Frame Buffers */
	XVFrmbufRd_Start(&frmbufrd);

	//xil_printf("INFO: FRMBUFrd configured\r\n");
	return(Status);
}


typedef struct {
	XVidC_ColorFormat MemFormat;
	XVidC_ColorFormat StreamFormat;
	u16 FormatBits;
} VideoFormats;

#define NUM_TEST_FORMATS 15

VideoFormats ColorFormats[NUM_TEST_FORMATS] =
{
	//memory format            stream format        bits per component
	{XVIDC_CSF_MEM_RGBX8,      XVIDC_CSF_RGB,       8},
	{XVIDC_CSF_MEM_YUVX8,      XVIDC_CSF_YCRCB_444, 8},
	{XVIDC_CSF_MEM_YUYV8,      XVIDC_CSF_YCRCB_422, 8},
	{XVIDC_CSF_MEM_RGBX10,     XVIDC_CSF_RGB,       10},
	{XVIDC_CSF_MEM_YUVX10,     XVIDC_CSF_YCRCB_444, 10},
	{XVIDC_CSF_MEM_Y_UV8,      XVIDC_CSF_YCRCB_422, 8},
	{XVIDC_CSF_MEM_Y_UV8_420,  XVIDC_CSF_YCRCB_420, 8},
	{XVIDC_CSF_MEM_RGB8,       XVIDC_CSF_RGB,       8},
	{XVIDC_CSF_MEM_YUV8,       XVIDC_CSF_YCRCB_444, 8},
	{XVIDC_CSF_MEM_Y_UV10,     XVIDC_CSF_YCRCB_422, 10},
	{XVIDC_CSF_MEM_Y_UV10_420, XVIDC_CSF_YCRCB_420, 10},
	{XVIDC_CSF_MEM_Y8,         XVIDC_CSF_YCRCB_444, 8},
	{XVIDC_CSF_MEM_Y10,        XVIDC_CSF_YCRCB_444, 10},
	{XVIDC_CSF_MEM_BGRX8,      XVIDC_CSF_RGB,       8},
	{XVIDC_CSF_MEM_UYVY8,      XVIDC_CSF_YCRCB_422, 8}
};


void frameBuffer_start_wr(XDpTxSs_MainStreamAttributes Msa[4],
		u8 downshift4K) {

	XVidC_ColorFormat Cfmt;
	XVidC_VideoTiming const *TimingPtr;
	XVidC_VideoStream VidStream;

//	resetIp_wr();


    /* Get video format to test */
    if(Msa[0].BitsPerColor <= 8){
            VidStream.ColorDepth = XVIDC_BPC_8;
            if (Msa[0].ComponentFormat ==
                         XDP_TX_MAIN_STREAMX_MISC0_COMPONENT_FORMAT_YCBCR422) {
                    Cfmt = ColorFormats[2].MemFormat;
                    VidStream.ColorFormatId = ColorFormats[2].StreamFormat;
            } else if (Msa[0].ComponentFormat ==
                            XDP_TX_MAIN_STREAMX_MISC0_COMPONENT_FORMAT_YCBCR444) {
                    Cfmt = ColorFormats[8].MemFormat;
                    VidStream.ColorFormatId = ColorFormats[8].StreamFormat;
            } else {
                    Cfmt = ColorFormats[7].MemFormat;
                    VidStream.ColorFormatId = ColorFormats[7].StreamFormat;
            }
    }else if(Msa[0].BitsPerColor == 10){
            VidStream.ColorDepth = XVIDC_BPC_10;
            if (Msa[0].ComponentFormat ==
                     XDP_TX_MAIN_STREAMX_MISC0_COMPONENT_FORMAT_YCBCR422) {
                    Cfmt = ColorFormats[9].MemFormat;
                    VidStream.ColorFormatId = ColorFormats[9].StreamFormat;

            } else if (Msa[0].ComponentFormat ==
                            XDP_TX_MAIN_STREAMX_MISC0_COMPONENT_FORMAT_YCBCR444) {
                    Cfmt = ColorFormats[4].MemFormat;
                    VidStream.ColorFormatId = ColorFormats[4].StreamFormat;
            } else {
                    Cfmt = ColorFormats[3].MemFormat;
                    VidStream.ColorFormatId = ColorFormats[3].StreamFormat;
            }
    }

	VidStream.PixPerClk  = 0x4; //Msa[0].UserPixelWidth;
	VidStream.Timing = Msa[0].Vtm.Timing;
	VidStream.FrameRate = Msa[0].Vtm.FrameRate;

#ifdef XPAR_XV_AXI4S_REMAP_NUM_INSTANCES
	remap_start_wr(Msa, downshift4K);
#endif

	/* Configure Frame Buffer */
	// Rx side
	u32 stride = CalcStride(Cfmt,
//					frmbufwr.FrmbufWr.Config.AXIMMDataWidth,
					256,
					&VidStream);
	ConfigFrmbuf_wr(stride, Cfmt, &VidStream);

}


void frameBuffer_start_rd(XDpTxSs_MainStreamAttributes Msa[4],
		u8 downshift4K) {

	XVidC_ColorFormat Cfmt;
	XVidC_VideoTiming const *TimingPtr;
	XVidC_VideoStream VidStream;

//	resetIp_rd();

    /* Get video format to test */
    if(Msa[0].BitsPerColor <= 8){
            VidStream.ColorDepth = XVIDC_BPC_8;
            if (Msa[0].ComponentFormat ==
                         XDP_TX_MAIN_STREAMX_MISC0_COMPONENT_FORMAT_YCBCR422) {
                    Cfmt = ColorFormats[2].MemFormat;
                    VidStream.ColorFormatId = ColorFormats[2].StreamFormat;
            } else if (Msa[0].ComponentFormat ==
                            XDP_TX_MAIN_STREAMX_MISC0_COMPONENT_FORMAT_YCBCR444) {
                    Cfmt = ColorFormats[8].MemFormat;
                    VidStream.ColorFormatId = ColorFormats[8].StreamFormat;
            } else {
                    Cfmt = ColorFormats[7].MemFormat;
                    VidStream.ColorFormatId = ColorFormats[7].StreamFormat;
            }
    }else if(Msa[0].BitsPerColor == 10){
            VidStream.ColorDepth = XVIDC_BPC_10;
            if (Msa[0].ComponentFormat ==
                     XDP_TX_MAIN_STREAMX_MISC0_COMPONENT_FORMAT_YCBCR422) {
                    Cfmt = ColorFormats[9].MemFormat;
                    VidStream.ColorFormatId = ColorFormats[9].StreamFormat;

            } else if (Msa[0].ComponentFormat ==
                            XDP_TX_MAIN_STREAMX_MISC0_COMPONENT_FORMAT_YCBCR444) {
                    Cfmt = ColorFormats[4].MemFormat;
                    VidStream.ColorFormatId = ColorFormats[4].StreamFormat;

            } else {
                    Cfmt = ColorFormats[3].MemFormat;
                    VidStream.ColorFormatId = ColorFormats[3].StreamFormat;
            }
    }


	VidStream.PixPerClk  = 0x4; //Msa[0].UserPixelWidth;
	VidStream.Timing = Msa[0].Vtm.Timing;
	VidStream.FrameRate = Msa[0].Vtm.FrameRate;

#ifdef XPAR_XV_AXI4S_REMAP_NUM_INSTANCES
	remap_start_rd(Msa, downshift4K);
#endif


	/* Configure Frame Buffer */
	// Rx side
	u32 stride = CalcStride(Cfmt,
//					frmbufwr.FrmbufWr.Config.AXIMMDataWidth,
					256,
					&VidStream);
//	ConfigFrmbuf_wr(stride, Cfmt, &VidStream);



	// Tx side may change due to sink monitor capability
//	if(downshift4K == 1){ // if sink is 4K monitor,
//		VidStream.VmId = VmId; // This will be set as 4K60
//		TimingPtr = XVidC_GetTimingInfo(VidStream.VmId);
//		VidStream.Timing = *TimingPtr;
//		VidStream.FrameRate = XVidC_GetFrameRate(VidStream.VmId);
//	}

	ConfigFrmbuf_rd(stride, Cfmt, &VidStream);


}

#ifdef XPAR_XV_AXI4S_REMAP_NUM_INSTANCES

void remap_set(XV_axi4s_remap *remap, u8 in_ppc, u8 out_ppc, u16 width,
		u16 height, u8 color_format){
	XV_axi4s_remap_Set_width(remap, width);
	XV_axi4s_remap_Set_height(remap, height);
	XV_axi4s_remap_Set_ColorFormat(remap, color_format);
	XV_axi4s_remap_Set_inPixClk(remap, in_ppc);
	XV_axi4s_remap_Set_outPixClk(remap, out_ppc);
}


void remap_start_rd(XDpTxSs_MainStreamAttributes Msa[4], u8 downshift4K)
{
    u8 color_format = 0;

    if( Msa[0].ComponentFormat ==
                        XDP_TX_MAIN_STREAMX_MISC0_COMPONENT_FORMAT_YCBCR422) {
        color_format = 0x2;
    } else if (Msa[0].ComponentFormat ==
                        XDP_TX_MAIN_STREAMX_MISC0_COMPONENT_FORMAT_YCBCR444) {
        color_format = 0x1;
    }

	u8 tx_ppc = XDp_ReadReg(DpTxSsInst.DpPtr->Config.BaseAddr, XDP_TX_USER_PIXEL_WIDTH);

	if(downshift4K == 1 && (Msa[0].Vtm.Timing.HActive >= 7680 &&
			Msa[0].Vtm.Timing.VActive >= 4320)){
		remap_set(&tx_remap, 4, tx_ppc, //Msa[0].UserPixelWidth,
			3840,
			2160
			, color_format);
	}
	// 4K120 will be changed to 4K60
	else if(downshift4K == 1 &&
			(Msa[0].Vtm.FrameRate * Msa[0].Vtm.Timing.HActive
			* Msa[0].Vtm.Timing.VActive > 4096*2160*60)){

		remap_set(&tx_remap, 4, tx_ppc, //Msa[0].UserPixelWidth,
			3840,
			2160
			, color_format);

	}else{
		remap_set(&tx_remap, 4, tx_ppc, //Msa[0].UserPixelWidth,
				Msa[0].Vtm.Timing.HActive,
				Msa[0].Vtm.Timing.VActive,
				color_format);

	}

	XV_axi4s_remap_EnableAutoRestart(&tx_remap);
	XV_axi4s_remap_Start(&tx_remap);
}

void remap_start_wr(XDpTxSs_MainStreamAttributes Msa[4], u8 downshift4K)
{
    u8 color_format = 0;

    if( Msa[0].ComponentFormat ==
                        XDP_TX_MAIN_STREAMX_MISC0_COMPONENT_FORMAT_YCBCR422) {
        color_format = 0x2;
    } else if (Msa[0].ComponentFormat ==
                        XDP_TX_MAIN_STREAMX_MISC0_COMPONENT_FORMAT_YCBCR444) {
        color_format = 0x1;
    }

	remap_set(&rx_remap, 4, 4, //Msa[0].UserPixelWidth, 4,
			Msa[0].Vtm.Timing.HActive,  Msa[0].Vtm.Timing.VActive
			, color_format);

	XV_axi4s_remap_EnableAutoRestart(&rx_remap);
	XV_axi4s_remap_Start(&rx_remap);
}
#endif

/*
 * This function is a call back to write the MSA values to Tx as they are
 * read from the Rx, instead of reading them from the Video common library
 */

u32 StreamOffsetAddr[4] = {0, 0x380,
			   0x3D0,
			   0x420};

void DpPt_TxSetMsaValuesImmediate(void *InstancePtr)
{
	int i = 0;

//	for (i=0; i< 4; i++) {

	/* Set the main stream attributes to the associated DisplayPort TX core
	 * registers. */
	XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr, XDP_TX_MAIN_STREAM_HTOTAL +
			StreamOffsetAddr[i], XDp_ReadReg(DpRxSsInst.DpPtr->Config.BaseAddr,
						XDP_RX_MSA_HTOTAL+(0x40*(stream_id_used-1))));

	XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr, XDP_TX_MAIN_STREAM_VTOTAL +
			StreamOffsetAddr[i], XDp_ReadReg(DpRxSsInst.DpPtr->Config.BaseAddr,
						XDP_RX_MSA_VTOTAL+(0x40*(stream_id_used-1))));
	XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr,XDP_TX_MAIN_STREAM_POLARITY+
			StreamOffsetAddr[i],
			XDp_ReadReg(DpRxSsInst.DpPtr->Config.BaseAddr, XDP_RX_MSA_HSPOL+(0x40*(stream_id_used-1)))|
			(XDp_ReadReg(DpRxSsInst.DpPtr->Config.BaseAddr,XDP_RX_MSA_VSPOL+(0x40*(stream_id_used-1))) <<
			XDP_TX_MAIN_STREAMX_POLARITY_VSYNC_POL_SHIFT));
	XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr, XDP_TX_MAIN_STREAM_HSWIDTH+
			StreamOffsetAddr[i], XDp_ReadReg(DpRxSsInst.DpPtr->Config.BaseAddr,
					XDP_RX_MSA_HSWIDTH+(0x40*(stream_id_used-1))));
	XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr, XDP_TX_MAIN_STREAM_VSWIDTH +
			StreamOffsetAddr[i], XDp_ReadReg(DpRxSsInst.DpPtr->Config.BaseAddr,
					XDP_RX_MSA_VSWIDTH+(0x40*(stream_id_used-1))));
	XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr, XDP_TX_MAIN_STREAM_HRES +
			StreamOffsetAddr[i],
			XDp_ReadReg(DpRxSsInst.DpPtr->Config.BaseAddr, XDP_RX_MSA_HRES+(0x40*(stream_id_used-1))));
	XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr, XDP_TX_MAIN_STREAM_VRES +
			StreamOffsetAddr[i],
			XDp_ReadReg(DpRxSsInst.DpPtr->Config.BaseAddr, XDP_RX_MSA_VHEIGHT+(0x40*(stream_id_used-1))));
	XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr, XDP_TX_MAIN_STREAM_HSTART +
			StreamOffsetAddr[i], XDp_ReadReg(DpRxSsInst.DpPtr->Config.BaseAddr,
					XDP_RX_MSA_HSTART+(0x40*(stream_id_used-1))));
	XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr, XDP_TX_MAIN_STREAM_VSTART +
			StreamOffsetAddr[i], XDp_ReadReg(DpRxSsInst.DpPtr->Config.BaseAddr,
					XDP_RX_MSA_VSTART+(0x40*(stream_id_used-1))));
	XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr, XDP_TX_MAIN_STREAM_MISC0 +
			StreamOffsetAddr[i], XDp_ReadReg(DpRxSsInst.DpPtr->Config.BaseAddr,
					XDP_RX_MSA_MISC0+(0x40*(stream_id_used-1))) & 0xFFFFFFFE);
	XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr, XDP_TX_MAIN_STREAM_MISC1 +
			StreamOffsetAddr[i], XDp_ReadReg(DpRxSsInst.DpPtr->Config.BaseAddr,
					XDP_RX_MSA_MISC1+(0x40*(stream_id_used-1))));
	XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr, XDP_TX_USER_PIXEL_WIDTH +
			StreamOffsetAddr[i], tx_ppc_set);
//		XDp_ReadReg(DpRxSsInst.DpPtr->Config.BaseAddr,XDP_RX_USER_PIXEL_WIDTH)
//			);



	/* Check for YUV422, BPP has to be set using component value to 2 */
	if( ( (XDp_ReadReg(DpRxSsInst.DpPtr->Config.BaseAddr, XDP_RX_MSA_MISC0+(0x40*(stream_id_used-1))))
			 & 0x6 ) == 0x2  ) {
	//YUV422
		DpTxSsInst.DpPtr->TxInstance.MsaConfig[i].ComponentFormat =
				XDP_TX_MAIN_STREAMX_MISC0_COMPONENT_FORMAT_YCBCR422;
	}
	else if(( (XDp_ReadReg(DpRxSsInst.DpPtr->Config.BaseAddr,XDP_RX_MSA_MISC0+(0x40*(stream_id_used-1))))
			 & 0x6 ) == 0x4){
	// YUV444
		DpTxSsInst.DpPtr->TxInstance.MsaConfig[i].ComponentFormat =
				XDP_TX_MAIN_STREAMX_MISC0_COMPONENT_FORMAT_YCBCR444;
	}else
	// RGB
		DpTxSsInst.DpPtr->TxInstance.MsaConfig[i].ComponentFormat =
				XDP_TX_MAIN_STREAMX_MISC0_COMPONENT_FORMAT_RGB;

//	}
}

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
	RecievedByte = XUartPs_ReadReg(STDIN_BASEADDRESS, XUARTPS_FIFO_OFFSET);

	/* Return the byte received */
	return (u8)RecievedByte;
}
