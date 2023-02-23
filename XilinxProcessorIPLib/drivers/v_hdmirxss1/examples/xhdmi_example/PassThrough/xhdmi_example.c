/******************************************************************************
* Copyright (C) 2018 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright 2022-2023 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xhdmi_example.c
*
* This file demonstrates how to use Xilinx HDMI TX Subsystem, HDMI RX Subsystem
* and Video PHY Controller drivers.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who    Date     Changes
*              dd/mm/yy
* ----- ------ -------- --------------------------------------------------
* 1.00  YB     11/05/19 Initial release.
* 1.01  KU     07/07/20 Support for FrameBuffer
*                       Can use FRL on RX and TMDS on TX
*                       Video will be clipped if RX resolution in more than
*                       4K
*                       Support Revision 2 (Pass 4) of OnSemi retimer
*                       Menu option updated for EDID and Training based on
*                       MaxRate configuration
*                       Added support for 16 BPC
* 1.02  KU     30/11/20 AVI InfoFrame Version set to 3 for resolutions
* 			VIC > 127
* 			Onsemi redriver tweaked for TMDS mode in TX
* 1.03  ssh    03/17/21 Added EdidHdmi20_t, PsIic0 and PsIic1 declarations
* 1.04  ssh    04/20/21 Added support for Dynamic HDR and Versal
* 1.05	ssh    07/14/21 Added support for Native Video
* 1.06  KU     30/08/21 RX FRL settings updated for VCU118
* 1.07  ssh    01/28/22 Updated GT Swing settings for VCK190
* 1.08  ssh    02/01/22 Updated Enable CTS Conversion Function
* 1.09	ssh    02/04/22 Added param to support Tx to train at the same rate as Rx
* 1.10  ssh    03/02/22 Added LCPLL and RPLL config for VCK190 Exdes
* 1.11	ssh    01/25/23 Added support for VEK280
*
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/
#include "xhdmi_example.h"

/***************** Macros (Inline Functions) Definitions *********************/
#define AUX_FIFO_CLEAR 1

/**************************** Type Definitions *******************************/

/************************** Function Prototypes ******************************/
int I2cMux(void);
int I2cClk(u32 InFreq, u32 OutFreq);

#if defined (__arm__) && (!defined(ARMR5))
int OnBoardSi5324Init(void);
#endif

void Info(void);
void DetailedInfo(void);

#ifdef XPAR_XV_HDMITXSS1_NUM_INSTANCES
void UpdateFrameRate(XHdmiphy1 *Hdmiphy1Ptr,
		     XV_HdmiTxSs1 *HdmiTxSs1Ptr,
		     XVidC_FrameRate FrameRate);
void UpdateColorFormat(XHdmiphy1 *Hdmiphy1Ptr,
		       XV_HdmiTxSs1 *HdmiTxSs1Ptr,
		       XVidC_ColorFormat ColorFormat);
void UpdateColorDepth(XHdmiphy1 *Hdmiphy1Ptr,
		      XV_HdmiTxSs1 *HdmiTxSs1Ptr,
		      XVidC_ColorDepth ColorDepth);
void CloneTxEdid(void);
void ResetAuxFifo(void);
#endif

#ifdef XPAR_XV_HDMITXSS1_NUM_INSTANCES
#ifdef XPAR_XV_TPG_NUM_INSTANCES
void Exdes_ConfigureTpgEnableInput(u32 EnableExtSrcInput);
void ResetTpg(void);
#endif

#ifdef XPAR_XV_AXI4S_REMAP_NUM_INSTANCES
void ResetInRemap(void);
void ResetOutRemap(void);
#endif
#endif

#ifdef XPAR_XV_HDMITXSS1_NUM_INSTANCES
void TxInfoFrameReset(void);

/**
 * These functions are used as callbacks to handle the triggers
 * from the hdmi 2.1 tx interrupt controlling layer (state machine)
 * to the example design.
 */
void XV_Tx_HdmiTrigCb_CableConnectionChange(void *InstancePtr);
void XV_Tx_HdmiTrigCb_SetupTxFrlRefClk(void *InstancePtr);
void XV_Tx_HdmiTrigCb_SetupTxTmdsRefClk(void *InstancePtr);
void XV_Tx_HdmiTrigCb_GetFRLClk(void *InstancePtr);
void XV_Tx_HdmiTrigCb_StreamOff(void *InstancePtr);
void XV_Tx_HdmiTrigCb_SetupAudioVideo(void *InstancePtr);
void XV_Tx_HdmiTrigCb_StreamOn(void *InstancePtr);
void XV_Tx_HdmiTrigCb_VidSyncRecv(void *InstancePtr);
void XV_Tx_HdmiTrigCb_EnableCableDriver(void *InstancePtr);
void XV_Tx_HdmiTrigCb_FrlFfeConfigDevice(void *InstancePtr);
void XV_Tx_HdmiTrigCb_FrlConfigDeviceSetup(void *InstancePtr);
void XV_Tx_HdmiTrigCb_ReadyToStartTransmit(void *InstancePtr);

void XV_Tx_Hdcp_EnforceBlanking(void *InstancePtr);
#endif /* XPAR_XV_HDMITXSS1_NUM_INSTANCES */

#ifdef XPAR_XV_HDMIRXSS1_NUM_INSTANCES
/**
 * These functions are used as callbacks to handle the triggers
 * from the hdmi 2.1 rx interrupt controlling layer (state machine)
 * to the example design.
 */
void XV_Rx_HdmiTrigCb_CableConnectionChange(void *InstancePtr);
void XV_Rx_HdmiTrigCb_AudioConfig(void *InstancePtr);
void XV_Rx_HdmiTrigCb_AuxEvent(void *InstancePtr);
void XV_Rx_HdmiTrigCb_StreamOn(void *InstancePtr);
void XV_Rx_HdmiTrigCb_StreamOff(void *InstancePtr);
void XV_Rx_HdmiTrigCb_VfmcDataClkSel(void *InstancePtr);
void XV_Rx_HdmiTrigCb_VfmcRxClkSel(void *InstancePtr);
void XV_Rx_HdmiTrigCb_VrrVfpEvent(void *InstancePtr);
void XV_Rx_HdmiTrigCb_VtemEvent(void *InstancePtr);
void XV_Rx_HdmiTrigCb_DynHdrEvent(void *InstancePtr);

#if (XPAR_V_HDMI_RXSS1_DSC_EN == 1)
void XV_Rx_HdmiTrigCb_DscDdcEvent(void *InstancePtr);
#endif

void XV_Rx_Hdcp_SetContentStreamType(void *InstancePtr);
void XV_Rx_Hdcp_EnforceBlanking(void *InstancePtr);
#endif /* XPAR_XV_HDMIRXSS1_NUM_INSTANCES */

void Hdmiphy1ErrorCallback(void *CallbackRef);
void Hdmiphy1ProcessError(void);

#if defined (XPAR_XV_HDMITXSS1_NUM_INSTANCES) && \
                 defined (XPAR_XV_HDMIRXSS1_NUM_INSTANCES)
void Exdes_CopyRxVidParamstoTx(XV_HdmiRxSs1 *HdmiRxSs1Ptr,
			       XV_HdmiTxSs1 *HdmiTxSs1Ptr);
void Exdes_UpdateVidParamstoTx(XV_HdmiRxSs1 *HdmiRxSs1Ptr,
	       XV_HdmiTxSs1 *HdmiTxSs1Ptr);

#if defined(USE_HDCP_HDMI_RX) || \
	defined(USE_HDCP_HDMI_TX)
static void XHdcp_EnforceBlanking(XV_Rx *UpstreamInstancePtr,
				  XV_Tx *DownstreamInstancePtr);
#endif
#endif
#if defined (XPAR_XV_FRMBUFWR_NUM_INSTANCES) && \
                      (XPAR_XV_FRMBUFRD_NUM_INSTANCES)
u32 Exdes_FBInitialize(XV_FrmbufWr_l2 *WrInstancePtr,
		       XV_FrmbufRd_l2 *RdInstancePtr,
		       XGpioPs *rstInstancePtr);
void VidFrameBufRdDone(void *CallbackRef);
void VidFrameBufWrDone(void *CallbackRef);
void ResetFrameBuf(u8 fb_reset);

void XV_ConfigVidFrameBuf_wr(XV_FrmbufWr_l2 *FrmBufWrPtr);
void XV_ConfigVidFrameBuf_rd(XV_FrmbufRd_l2 *FrmBufRdPtr);

#endif

u32 Exdes_LoadHdcpKeys(void *IicPtr);

u32 Exdes_SysTmrInitialize(XHdmi_Exdes *InstancePtr, u32 TmrId,
		u32 TmrIntrId);
u32 Exdes_UsToTicks(u32 TimeInUs, u32 TmrCtrClkFreq);
void Exdes_StartSysTmr(XHdmi_Exdes *InstancePtr, u32 IntervalInMs);
void Exdes_SysTmrCallback(void *CallbackRef, u8 TmrCtrNumber);
void Exdes_SysTimerIntrHandler(void *CallbackRef);
u8 Exdes_LookupVic(XVidC_VideoMode VideoMode);
#if (defined XPS_BOARD_VEK280_ES)
unsigned Vfmc_I2cSend_RC(void *IicPtr, u16 SlaveAddr, u8 *MsgPtr,
		unsigned ByteCount, u8 Option);
#endif

/************************* Variable Definitions *****************************/
/* VPHY structure */
XHdmiphy1       Hdmiphy1;
u8              Hdmiphy1ErrorFlag;
u8              Hdmiphy1PllLayoutErrorFlag;
XVfmc           Vfmc[1];
XV_HdmiC_VrrInfoFrame VrrInforFrame ;
#ifdef XPAR_XV_HDMITXSS1_NUM_INSTANCES
/* HDMI TX SS structure */
XV_HdmiTxSs1    HdmiTxSs;
u8 HdmiTxErrorFlag = 0;
EdidHdmi EdidHdmi_t;

#ifdef XPAR_XV_TPG_NUM_INSTANCES
/* Test Pattern Generator Structure */
XV_tpg          Tpg;
/* Video Pattern */
XTpg_PatternId  Pattern;
#endif

#ifdef XPAR_XV_AXI4S_REMAP_NUM_INSTANCES
XV_axi4s_remap InRemap;
XV_axi4s_remap OutRemap;
#endif

#ifdef USE_HDMI_AUDGEN
/* Audio Generator structure */
XhdmiAudioGen_t AudioGen;
#endif

u32 VsyncCounter = 0;
u32 AuxHwFullCounter = 0;

XHdmiC_Aux      AuxFifo[AUXFIFOSIZE];
#endif /* XPAR_XV_HDMITXSS1_NUM_INSTANCES */

#ifdef XPAR_XV_HDMIRXSS1_NUM_INSTANCES
/* HDMI RX SS structure */
XV_HdmiRxSs1     HdmiRxSs;
#if defined	(XPAR_V_HDMI_RXSS1_DSC_EN) && \
		defined (XPAR_XAXIS_SWITCH_NUM_INSTANCES)
XAxis_Switch HdmiRxAxiSwitch;
#endif

u32 AuxCounter = 0;
#endif

#if defined (XPAR_XV_HDMITXSS1_NUM_INSTANCES) && \
                 defined (XPAR_XV_HDMIRXSS1_NUM_INSTANCES)
/* Variable for pass-through operation */
u8              AuxFifoStartIndex;
u8              AuxFifoEndIndex;
u8              AuxFifoCount;
u8              AuxFifoOvrFlowCnt;

u8 AuxFifoStartFlag = (FALSE);
u32 FrWrDoneCounter = 0;
u32 FrRdDoneCounter = 0;


#endif

#ifdef XPAR_XGPIO_NUM_INSTANCES
/* GPIO structure */
XGpio            Gpio_Tpg_resetn;
#endif

/* Interrupt Controller Data Structure */
#if defined (ARMR5) || (__aarch64__) || (__arm__)
XScuGic          Intc;
#else
XIntc            Intc;
#endif

#if defined (XPAR_XV_FRMBUFRD_NUM_INSTANCES) && \
                      (XPAR_XV_FRMBUFWR_NUM_INSTANCES)
/* Video Frame Buffer Write */
XV_FrmbufWr_l2       FrameBufWr;
XV_frmbufwr_Config   *FrameBufWr_ConfigPtr;
/* Video Frame Buffer Read */
XV_FrmbufRd_l2	     FrameBufRd;
XV_frmbufrd_Config   *FrameBufRd_ConfigPtr;

XVidC_VideoStream     HdmiTxSsVidStreamPtrFbRd ;
XVidC_VideoStream     HdmiRxSsVidStreamPtrFbWr ;
#ifdef XPAR_XGPIOPS_NUM_INSTANCES
/* GPIO Reset for Video Frame Buffer */
XGpioPs              Gpio_VFRB_resetn;
XGpioPs_Config       *Gpio_VFRB_resetn_ConfigPtr;
#endif
/* Dynamic HDR Info */
XV_HdmiRxSs1_DynHDR_Info RX_DynHDR_Info;
XV_HdmiTxSs1_DynHdr_Config TX_DynHDR_Info;
#endif

/* HDMI Application Menu: Data Structure */
extern XHdmi_Menu       HdmiMenu;

/* General HDMI Application Variable Declaration (Scratch Pad) */
#if defined(XPAR_XV_HDMIRXSS1_NUM_INSTANCES)
extern XV_Rx xhdmi_example_rx_controller;
#endif
#if defined(XPAR_XV_HDMITXSS1_NUM_INSTANCES)
extern XV_Tx xhdmi_example_tx_controller;
#endif
XHdmi_Exdes xhdmi_exdes_ctrlr;

/* Scratch pad for user enabled printing. */
Exdes_Debug_Printf exdes_debug_print = NULL;
Exdes_Debug_Printf exdes_aux_debug_print = NULL;
Exdes_Debug_Printf exdes_hdcp_debug_print = NULL;

#if defined (VTEM2FSYNC) && (! defined XPAR_XVTC_NUM_INSTANCES)
u8 BaseFrameRate_VRR = 0;
#endif

#if defined (XPAR_XV_FRMBUFWR_NUM_INSTANCES) && \
                      (XPAR_XV_FRMBUFRD_NUM_INSTANCES)
/* Scratch pad for HDMI + Frame Buffer ExDes */
u8  StartStream = (FALSE);
u8  StartToRead = (FALSE);
u32 FrameWrCompCnt = 0;
u32 FrameRdCompCnt = 0;
u8 fb_cap = 0;
u8 wr = 1; // start write buffer id
u8 rd = 3; // start read buffer id
u32 offset = 0;
u32 Wr_stride = 0;
u8 DynHdr_wr_buff_offset = 0; // start Dynamic write buffer id
u8 DynHdr_rd_buff_offset = 3; // start Dynamic HDR read buffer id

u32 read_finsihed = 1;
u32 write_finsihed = 1;

/* to update the base frame rate to SPDIF Packet*/
#if defined (VTEM2FSYNC)
u8 BaseFrameRate_VRR = 0;
#endif

/* mapping between memory and streaming video formats */
typedef struct {
	XVidC_ColorFormat MemFormat;
	XVidC_ColorFormat StreamFormat;
	u16               FormatBits;
} VideoFormats;

/** @name HDMI TX FRL training state
* @{
*/
typedef enum {
	XV_FBSTATE_UNINITIALIZED,	/* Frame not initialized */
	XV_FBSTATE_FRESH_F0,
	XV_FBSTATE_FRESH_F0_READ,
	XV_FBSTATE_FRESH_F1,
	XV_FBSTATE_FRESH_F1_WRITE
	/* Additional Frame Buffer States,
	 * XV_FBSTATE_WRITE_IP,		/\* Frame writing in progress *\/
	 * XV_FBSTATE_WRITE_DONE,	/\* Frame write completed *\/
	 * XV_FBSTATE_READ_IP,		/\* Frame reading in progress *\/
	 * XV_FBSTATE_READ_DONE		/\* Frame read completed *\/
	 */
} FbState;

/* Simple Page Data Structure */
typedef struct {
	FbState	          State;
	u32               BaseAddr;
	u32               ChromaBaseAddr;
	u32               ChromaOffset;
	u64               DynHDRBaseAddr;
} FrameBuffer;

/* Additional macro to define maximum frames.
 * #define MAX_BUFF_FRAME 4
 */
#define DDR_BASE_ADDRESS 0x20000000
FrameBuffer VidBuff[5];
u8 stopFBWrFlag = FALSE;
VideoFormats ColorFormats[NUM_MEMORY_COLOR_FORMATS] =
{
	/* memory format */        /* stream format */  /* bpc */
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
	{XVIDC_CSF_MEM_UYVY8,      XVIDC_CSF_YCRCB_422, 8},
	{XVIDC_CSF_MEM_BGR8,       XVIDC_CSF_RGB,       8},
	{XVIDC_CSF_MEM_Y_UV12,     XVIDC_CSF_YCRCB_422, 12}
};
#endif
/************************** Function Definitions *****************************/

/*****************************************************************************/
/**
*
* This function is used to enable and additional debugging prints
* in the applicaiton.
*
* @param  Printfunc is the function to enable printing.
*
* @return None.
*
* @note
*
******************************************************************************/
void Exdes_SetDebugPrintf(Exdes_Debug_Printf PrintFunc)
{
	exdes_debug_print = PrintFunc;
}

/*****************************************************************************/
/**
*
* This function is used to enable and additional debugging prints
* for the Aux Fifo operations in the applicaiton.
*
* @param  Printfunc is the function to enable printing.
*
* @return None.
*
* @note
*
******************************************************************************/
void Exdes_SetAuxFifoDebugPrintf(Exdes_Debug_Printf PrintFunc)
{
	exdes_aux_debug_print = PrintFunc;
}

/*****************************************************************************/
/**
*
* This function is used to enable and additional debugging prints
* for the HDCP operations in the applicaiton.
*
* @param  Printfunc is the function to enable printing.
*
* @return None.
*
* @note
*
******************************************************************************/
void Exdes_SetHdcpDebugPrintf(Exdes_Debug_Printf PrintFunc)
{
	exdes_hdcp_debug_print = PrintFunc;
}
#if defined(XPAR_XV_HDMITXSS1_NUM_INSTANCES)
/*****************************************************************************/
/**
*
* This function is used to initialize the TX controller data strucuture
* for the HDMI 2.1 TX interrupt controller layer (see xhdmi_example_tx_sm.c/.h)
* Here the interrupt ids of the HDMI 2.1 TX SS and related cores are set and
* the HDMI 2.1 TX interrupt controller layer is initialized.
*
* @param  InstancePtr is the instance of the HDMI 2.1 TX interrupt controller
*         layer data structure.
* @param  HdmiTxSsDevId is the device id of the HDMI 2.1 TX SS.
* @param  VPhyDevId is the device if the associated Hdmi Phy.
*
* @return None.
*
* @note
*
******************************************************************************/
u32 XV_Tx_InitController(XV_Tx *InstancePtr, u32 HdmiTxSsDevId,
		u32 VPhyDevId)
{
	u32 Status = XST_SUCCESS;

	/* Set the state machine controller references. */
	InstancePtr->HdmiTxSs = &HdmiTxSs;
	InstancePtr->VidPhy = &Hdmiphy1;
	InstancePtr->Intc = &Intc;

	/* Setup the System Interrupt vector Id references. */
	XV_Tx_IntrVecId IntrVecIds;
#if defined(__arm__) || (__aarch64__)
#if defined(USE_HDCP_HDMI_TX)
	IntrVecIds.IntrVecId_HdmiTxSs = XPAR_FABRIC_V_HDMITXSS1_0_IRQ_VEC_ID;
#else
	IntrVecIds.IntrVecId_HdmiTxSs = XPAR_FABRIC_V_HDMITXSS1_0_VEC_ID;
#endif /* USE_HDCP_HDMI_TX */
#ifdef XPAR_XHDCP_NUM_INSTANCES
	IntrVecIds.IntrVecId_Hdcp14 =
			XPAR_FABRIC_V_HDMITXSS1_0_HDCP14_IRQ_VEC_ID;
	IntrVecIds.IntrVecId_Hdcp14Timer =
			XPAR_FABRIC_V_HDMITXSS1_0_HDCP14_TIMER_IRQ_VEC_ID;
#else
	IntrVecIds.IntrVecId_Hdcp14 = (UINTPTR)(NULL);
	IntrVecIds.IntrVecId_Hdcp14Timer = (UINTPTR)NULL;
#endif
#if (XPAR_XHDCP22_TX_NUM_INSTANCES)
	IntrVecIds.IntrVecId_Hdcp22Timer =
			XPAR_FABRIC_V_HDMITXSS1_0_HDCP22_TIMER_IRQ_VEC_ID;
#else
	IntrVecIds.IntrVecId_Hdcp22Timer = (UINTPTR)NULL;
#endif
	IntrVecIds.IntrVecId_VPhy = XPAR_FABRIC_V_HDMIPHY1_0_VEC_ID;
#else /* microblaze */
#if defined(USE_HDCP_HDMI_TX)
	IntrVecIds.IntrVecId_HdmiTxSs = XPAR_INTC_0_V_HDMITXSS1_0_IRQ_VEC_ID;
#else
	IntrVecIds.IntrVecId_HdmiTxSs = XPAR_INTC_0_V_HDMITXSS1_0_VEC_ID;
#endif /* USE_HDCP_HDMI_TX */
#ifdef XPAR_XHDCP_NUM_INSTANCES
	IntrVecIds.IntrVecId_Hdcp14 =
			XPAR_INTC_0_V_HDMITXSS1_0_HDCP14_IRQ_VEC_ID;
	IntrVecIds.IntrVecId_Hdcp14Timer =
			XPAR_INTC_0_V_HDMITXSS1_0_HDCP14_TIMER_IRQ_VEC_ID;
#else
	IntrVecIds.IntrVecId_Hdcp14 = (UINTPTR)NULL;
	IntrVecIds.IntrVecId_Hdcp14Timer = (UINTPTR)NULL;
#endif
#if (XPAR_XHDCP22_TX_NUM_INSTANCES)
	IntrVecIds.IntrVecId_Hdcp22Timer =
			XPAR_INTC_0_V_HDMITXSS1_0_HDCP22_TIMER_IRQ_VEC_ID;
#else
	IntrVecIds.IntrVecId_Hdcp22Timer = (UINTPTR)NULL;
#endif
	IntrVecIds.IntrVecId_VPhy = XPAR_INTC_0_V_HDMIPHY1_0_VEC_ID;
#endif /* defined(__arm__) || (__aarch64__) */

	/* Initialize the Video Transmitter for HDMI. */
	Status = XV_Tx_Hdmi_Initialize(InstancePtr, HdmiTxSsDevId,
					VPhyDevId, IntrVecIds);
	if (Status != XST_SUCCESS) {
		xil_printf("Initialization of Video "
		           "Transmitter for HDMI failed !!\r\n");
		return XST_FAILURE;
	}

	/* Set the Application version in TXSs driver structure */
	XV_HdmiTxSS1_SetAppVersion(&HdmiTxSs, APP_MAJ_VERSION, APP_MIN_VERSION);

	/* To set the debugging prints in the HDMI 2.1 TX interrupt controlling
	 * layer this function can be initialized by xil_printf or any other
	 * valid printing function. */
	XV_Tx_SetDebugPrints(NULL);
	XV_Tx_SetDebugStateMachinePrints(NULL);
	XV_Tx_SetDebugTxNewStreamSetupPrints(NULL);

	Status |= XV_Tx_SetTriggerCallbacks(InstancePtr,
				XV_TX_TRIG_HANDLER_CONNECTION_CHANGE,
				(void *)XV_Tx_HdmiTrigCb_CableConnectionChange,
				(void *)InstancePtr);
	Status |= XV_Tx_SetTriggerCallbacks(InstancePtr,
				XV_TX_TRIG_HANDLER_SETUP_TXTMDSREFCLK,
				(void *)XV_Tx_HdmiTrigCb_SetupTxTmdsRefClk,
				(void *)InstancePtr);
	Status |= XV_Tx_SetTriggerCallbacks(InstancePtr,
			    XV_TX_TRIG_HANDLER_SETUP_TXFRLREFCLK,
				(void *)XV_Tx_HdmiTrigCb_SetupTxFrlRefClk,
				(void *)InstancePtr);
	Status |= XV_Tx_SetTriggerCallbacks(InstancePtr,
				XV_TX_GET_FRL_CLOCK,
				(void *)XV_Tx_HdmiTrigCb_GetFRLClk,
				(void *)InstancePtr);
	Status |= XV_Tx_SetTriggerCallbacks(InstancePtr,
				XV_TX_TRIG_HANDLER_SETUP_AUDVID,
				(void *)XV_Tx_HdmiTrigCb_SetupAudioVideo,
				(void *)InstancePtr);
	Status |= XV_Tx_SetTriggerCallbacks(InstancePtr,
				XV_TX_TRIG_HANDLER_STREAM_ON,
				(void *)XV_Tx_HdmiTrigCb_StreamOn,
				(void *)InstancePtr);
	Status |= XV_Tx_SetTriggerCallbacks(InstancePtr,
				XV_TX_TRIG_HANDLER_ENABLE_CABLE_DRIVERS,
				(void *)XV_Tx_HdmiTrigCb_EnableCableDriver,
				(void *)InstancePtr);
	Status |= XV_Tx_SetTriggerCallbacks(InstancePtr,
				XV_TX_TRIG_HANDLER_VSYNC_RECV,
				(void *)XV_Tx_HdmiTrigCb_VidSyncRecv,
				(void *)InstancePtr);
	Status |= XV_Tx_SetTriggerCallbacks(InstancePtr,
				XV_TX_TRIG_HANDLER_STREAM_OFF,
				(void *)XV_Tx_HdmiTrigCb_StreamOff,
				(void *)InstancePtr);
	Status |= XV_Tx_SetTriggerCallbacks(InstancePtr,
				XV_TX_TRIG_HANDLER_READYTOSTARTTX,
				(void *)XV_Tx_HdmiTrigCb_ReadyToStartTransmit,
				(void *)InstancePtr);
	Status |= XV_Tx_SetTriggerCallbacks(InstancePtr,
				XV_TX_TRIG_HANDLER_FRL_FFE_CONFIG_DEVICE,
				(void *)XV_Tx_HdmiTrigCb_FrlFfeConfigDevice,
				(void *)InstancePtr);
	Status |= XV_Tx_SetTriggerCallbacks(InstancePtr,
				XV_TX_TRIG_HANDLER_FRL_CONFIG_DEVICE_SETUP,
				(void *)XV_Tx_HdmiTrigCb_FrlConfigDeviceSetup,
				(void *)InstancePtr);
#if defined(USE_HDCP_HDMI_TX)
	Status |= XV_Tx_SetTriggerCallbacks(InstancePtr,
				XV_TX_TRIG_HANDLER_HDCP_FORCE_BLANKING,
				(void *)XV_Tx_Hdcp_EnforceBlanking,
				(void *)InstancePtr);
#endif

	return Status;
}
#endif

#if defined(XPAR_XV_HDMIRXSS1_NUM_INSTANCES)
/*****************************************************************************/
/**
*
* This function is used to initialize the RX controller data strucuture
* for the HDMI 2.1 RX interrupt controller layer (see xhdmi_example_rx_sm.c/.h)
* Here the interrupt ids of the HDMI 2.1 RX SS and related cores are set and
* the HDMI 2.1 RX interrupt controller layer is initialized.
*
* @param  InstancePtr is the instance of the HDMI 2.1 RX interrupt controller
*         layer data structure.
* @param  HdmiTxSsDevId is the device id of the HDMI 2.1 RX SS.
* @param  VPhyDevId is the device if the associated Hdmi Phy.
*
* @return None.
*
* @note
*
******************************************************************************/
u32 XV_Rx_InitController(XV_Rx *InstancePtr, u32 HdmiRxSsDevId,
		u32 VPhyDevId)
{
	u32 Status = XST_SUCCESS;

	/* Set the state machine controller references. */
	InstancePtr->HdmiRxSs = &HdmiRxSs;
	InstancePtr->VidPhy = &Hdmiphy1;
	InstancePtr->Intc = &Intc;

	/* Setup the System Interrupt vector Id references. */
	XV_Rx_IntrVecId IntrVecIds;
#if defined(__arm__) || (__aarch64__)
#if defined(USE_HDCP_HDMI_RX)
	IntrVecIds.IntrVecId_HdmiRxSs =
			XPAR_FABRIC_V_HDMIRXSS1_0_IRQ_VEC_ID;
#else
	IntrVecIds.IntrVecId_HdmiRxSs = XPAR_FABRIC_V_HDMIRXSS1_0_VEC_ID;
#endif /* USE_HDCP_HDMI_RX */
#ifdef XPAR_XHDCP_NUM_INSTANCES
	IntrVecIds.IntrVecId_Hdcp14 =
			XPAR_FABRIC_V_HDMIRXSS1_0_HDCP14_IRQ_VEC_ID;
	IntrVecIds.IntrVecId_Hdcp14Timer =
			XPAR_FABRIC_V_HDMIRXSS1_0_HDCP14_TIMER_IRQ_VEC_ID;
#else
	IntrVecIds.IntrVecId_Hdcp14 = (UINTPTR)NULL;
	IntrVecIds.IntrVecId_Hdcp14Timer = (UINTPTR)NULL;
#endif
#if (XPAR_XHDCP22_TX_NUM_INSTANCES)
	IntrVecIds.IntrVecId_Hdcp22Timer =
			XPAR_FABRIC_V_HDMIRXSS1_0_HDCP22_TIMER_IRQ_VEC_ID;
#else
	IntrVecIds.IntrVecId_Hdcp22Timer = (UINTPTR)NULL;
#endif
	IntrVecIds.IntrVecId_VPhy = XPAR_FABRIC_V_HDMIPHY1_0_VEC_ID;
#else /* microblaze */
#if defined(USE_HDCP_HDMI_RX)
	IntrVecIds.IntrVecId_HdmiRxSs = XPAR_INTC_0_V_HDMIRXSS1_0_IRQ_VEC_ID;
#else
	IntrVecIds.IntrVecId_HdmiRxSs = XPAR_INTC_0_V_HDMIRXSS1_0_VEC_ID;
#endif /* USE_HDCP_HDMI_RX */
#ifdef XPAR_XHDCP_NUM_INSTANCES
	IntrVecIds.IntrVecId_Hdcp14 =
			XPAR_INTC_0_V_HDMIRXSS1_0_HDCP14_IRQ_VEC_ID;
	IntrVecIds.IntrVecId_Hdcp14Timer =
			XPAR_INTC_0_V_HDMIRXSS1_0_HDCP14_TIMER_IRQ_VEC_ID;
#else
	IntrVecIds.IntrVecId_Hdcp14 = (UINTPTR)NULL;
	IntrVecIds.IntrVecId_Hdcp14Timer = (UINTPTR)NULL;
#endif
#if (XPAR_XHDCP22_TX_NUM_INSTANCES)
	IntrVecIds.IntrVecId_Hdcp22Timer =
			XPAR_INTC_0_V_HDMIRXSS1_0_HDCP22_TIMER_IRQ_VEC_ID;
#else
	IntrVecIds.IntrVecId_Hdcp22Timer = (UINTPTR)NULL;
#endif
	IntrVecIds.IntrVecId_VPhy = XPAR_INTC_0_V_HDMIPHY1_0_VEC_ID;
#endif /* defined(__arm__) || (__aarch64__) */

#if defined (XPAR_XV_FRMBUFWR_NUM_INSTANCES) && \
                      (XPAR_XV_FRMBUFRD_NUM_INSTANCES) || (!defined XPAR_XVTC_NUM_INSTANCES)

#if (EDID_INIT == 0) // 2.1 Default
	memcpy(&(InstancePtr->Edid), &SampleEdid,
		         sizeof(InstancePtr->Edid));
	 xil_printf("HDMI 2.1 Default EDID is Initialized !!\r\n");
#elif (EDID_INIT == 1) // 2.1 VRR
	memcpy(&(InstancePtr->Edid), &SampleEdid_1,
		         sizeof(InstancePtr->Edid));
	 xil_printf("HDMI 2.1 VRR EDID is Initialized !!\r\n");
#elif (EDID_INIT == 2)
	 memcpy(&(InstancePtr->Edid), &SampleEdid_2,
		         sizeof(InstancePtr->Edid));
	 xil_printf("FreeSync EDID is Initialized !!\r\n");
#elif (EDID_INIT == 3)
		memcpy(&(InstancePtr->Edid), &SampleEdid_3,
			         sizeof(InstancePtr->Edid));
		 xil_printf("HDMI 2.1 VRR TMDS EDID is Initialized !!\r\n");
#endif

#else
#if (XPAR_V_HDMI_RXSS1_DSC_EN == 1)
	memcpy(&(InstancePtr->Edid), &SampleEdid_4,
		         sizeof(InstancePtr->Edid));
	xil_printf("HDMI 2.1 DSC EDID is Initialized !!\r\n");
#else
	memcpy(&(InstancePtr->Edid), &SampleEdid,
		         sizeof(InstancePtr->Edid));
	 xil_printf("HDMI 2.1 Default EDID is Initialized !!\r\n");
#endif
#endif

	/* Get User Edid Info */
	XV_HdmiRxSs1_SetEdidParam(&HdmiRxSs, (u8*)&(InstancePtr->Edid),
			sizeof(InstancePtr->Edid) / sizeof(InstancePtr->Edid[0]));

	/* Initialize the Video Receiver for HDMI. */
	Status = XV_Rx_Hdmi_Initialize(InstancePtr, HdmiRxSsDevId,
			VPhyDevId, IntrVecIds);
	if (Status != XST_SUCCESS) {
		xil_printf("Initialization of Video "
		           "Receiver for HDMI failed !!\r\n");
		return XST_FAILURE;
	}
/* update EDID based on MAX FRL Rate */
#if (!defined (XPAR_XV_FRMBUFWR_NUM_INSTANCES)) && \
			(!defined (XPAR_XV_FRMBUFRD_NUM_INSTANCES)) && \
				(defined XPAR_XVTC_NUM_INSTANCES)


	 switch (HdmiRxSs.Config.MaxFrlRate) {
	 case 6: // 12 Gbps @ 4 Lanes
			 InstancePtr->Edid[189] = 0x63;
			 InstancePtr->Edid[255] = 0xCB;
			 break;
	 case 5: // 10 Gbps @ 4 Lanes
			InstancePtr->Edid[189] = 0x53;
			InstancePtr->Edid[255] = 0xDB;
			break;
	 case 4: // 8 Gbps @ 4 Lanes
			InstancePtr->Edid[189] = 0x43;
			InstancePtr->Edid[255] = 0xEB;
			break;
	 case 3: // 6 Gbps @ 4 Lanes
			InstancePtr->Edid[189] = 0x33;
			InstancePtr->Edid[255] = 0xFB;
			break;
	 case 2: // 6 Gbps @ 3 Lanes
			InstancePtr->Edid[189] = 0x23;
			InstancePtr->Edid[255] = 0x0B;
			break;
	 case 1: // 3 Gbps @ 4 Lanes
			InstancePtr->Edid[189] = 0x13;
			InstancePtr->Edid[255] = 0x1B;
			break;
	 case 0: // TMDS
			InstancePtr->Edid[189] = 0x03;
			InstancePtr->Edid[255] = 0x2B;
			break;
	 default:
			InstancePtr->Edid[189] = 0x63;
			InstancePtr->Edid[255] = 0xCB;
			break;
	 }

#endif

	/* Set the Application version in RXSS driver structure */
	XV_HdmiRxSS1_SetAppVersion(&HdmiRxSs, APP_MAJ_VERSION, APP_MIN_VERSION);

	/* To set the debugging prints in the HDMI 2.1 RX interrupt controlling
	 * layer this function can be initialized by xil_printf or any other
	 * valid printing function. */
	XV_Rx_SetDebugPrints(NULL);
	XV_Rx_SetDebugStateMachinePrints(NULL);

	Status |= XV_Rx_SetTriggerCallbacks(InstancePtr,
				XV_RX_TRIG_HANDLER_CONNECTION_CHANGE,
				(void *)XV_Rx_HdmiTrigCb_CableConnectionChange,
				(void *)InstancePtr);
	Status |= XV_Rx_SetTriggerCallbacks(InstancePtr,
				XV_RX_TRIG_HANDLER_STREAM_OFF,
				(void *)XV_Rx_HdmiTrigCb_StreamOff,
				(void *)InstancePtr);
	Status |= XV_Rx_SetTriggerCallbacks(InstancePtr,
				XV_RX_TRIG_HANDLER_STREAM_ON,
				(void *)XV_Rx_HdmiTrigCb_StreamOn,
				(void *)InstancePtr);
	Status |= XV_Rx_SetTriggerCallbacks(InstancePtr,
				XV_RX_TRIG_HANDLER_AUDIOCONFIG,
				(void *)XV_Rx_HdmiTrigCb_AudioConfig,
				(void *)InstancePtr);
	Status |= XV_Rx_SetTriggerCallbacks(InstancePtr,
				XV_RX_TRIG_HANDLER_AUXEVENT,
				(void *)XV_Rx_HdmiTrigCb_AuxEvent,
				(void *)InstancePtr);
	Status |= XV_Rx_SetTriggerCallbacks(InstancePtr,
				XV_RX_TRIG_HANDLER_CLKSRC_CONFIG,
				(void *)XV_Rx_HdmiTrigCb_VfmcDataClkSel,
				(void *)InstancePtr);
	Status |= XV_Rx_SetTriggerCallbacks(InstancePtr,
				XV_RX_TRIG_HANDLER_CLKSRC_SEL,
				(void *)XV_Rx_HdmiTrigCb_VfmcRxClkSel,
				(void *)InstancePtr);

	Status |= XV_Rx_SetTriggerCallbacks(InstancePtr,
			    XV_RX_TRIG_HANDLER_VRRVFPEVENT,
				(void *)XV_Rx_HdmiTrigCb_VrrVfpEvent,
				(void *)InstancePtr);

	Status |= XV_Rx_SetTriggerCallbacks(InstancePtr,
			    XV_RX_TRIG_HANDLER_VTEMEVENT,
				(void *)XV_Rx_HdmiTrigCb_VtemEvent,
				(void *)InstancePtr);
#if defined (XPAR_XV_FRMBUFWR_NUM_INSTANCES) && \
                      (XPAR_XV_FRMBUFRD_NUM_INSTANCES)
	Status |= XV_Rx_SetTriggerCallbacks(InstancePtr,
			    XV_RX_TRIG_HANDLER_DYNHDREVENT,
				(void *)XV_Rx_HdmiTrigCb_DynHdrEvent,
				(void *)InstancePtr);
#endif

#if (XPAR_V_HDMI_RXSS1_DSC_EN == 1)
	Status |= XV_Rx_SetTriggerCallbacks(InstancePtr,
				XV_RX_TRIG_HANDLER_DSCDDCSTSUPDTEVNT,
				(void *)XV_Rx_HdmiTrigCb_DscDdcEvent,
				(void *)InstancePtr);
#endif
#if defined(USE_HDCP_HDMI_RX)
	Status |= XV_Rx_SetTriggerCallbacks(InstancePtr,
				XV_RX_TRIG_HANDLER_HDCP_FORCE_BLANKING,
				(void *)XV_Rx_Hdcp_EnforceBlanking,
				(void *)InstancePtr);
	Status |= XV_Rx_SetTriggerCallbacks(InstancePtr,
				XV_RX_TRIG_HANDLER_HDCP_SET_CONTENTSTREAMTYPE,
				(void *)XV_Rx_Hdcp_SetContentStreamType,
				(void *)InstancePtr);
#endif

	return Status;
}
#endif
/*****************************************************************************/
/**
*
* This function is used to initialize the example desing controller
* data strucuture that is used to track the presence and instances
* of the hdmi 2.1 receiver and transmitter.
*
* @param  InstancePtr is the instance of the example design handle
*         data structure.
*
* @return None.
*
* @note
*
******************************************************************************/
void Exdes_InitController(XHdmi_Exdes *InstancePtr)
{
#if defined(XPAR_XV_HDMIRXSS1_NUM_INSTANCES)
	InstancePtr->hdmi_rx_ctlr = &xhdmi_example_rx_controller;
#endif
#if defined(XPAR_XV_HDMITXSS1_NUM_INSTANCES)
	InstancePtr->hdmi_tx_ctlr = &xhdmi_example_tx_controller;
#endif

	xhdmi_exdes_ctrlr.IsRxPresent = FALSE;
	xhdmi_exdes_ctrlr.IsTxPresent = FALSE;

	xhdmi_exdes_ctrlr.SystemEvent = FALSE;

	xhdmi_exdes_ctrlr.TxStartTransmit = FALSE;

	Exdes_SetDebugPrintf(NULL);
	Exdes_SetAuxFifoDebugPrintf(NULL);
	Exdes_SetHdcpDebugPrintf(NULL);
}

/*****************************************************************************/
/**
*
* This function is used to initialize the system timer that is used to
* count and generate given periodic interrupts.
*
* @param    InstancePtr is a pointer to the example design handler
* @param    TmrId is the device id of the system timer
* @param    TmrIntrId is the interrupt vector id of the system timer
*
* @return   XST_SUCCESS if successful.
*
* @note	    None.
*
******************************************************************************/
u32 Exdes_SysTmrInitialize(XHdmi_Exdes *InstancePtr, u32 TmrId,
		u32 TmrIntrId)
{
	u32 Status = XST_SUCCESS;

	/* Initialize the system timer */
	Status = XTmrCtr_Initialize(&InstancePtr->SysTmrInst, TmrId);
#if defined(XPAR_XV_HDMITXSS1_NUM_INSTANCES)
	/* Setup the timer interrupt */
#if defined(__arm__) || (__aarch64__)
	Status |= XScuGic_Connect(InstancePtr->hdmi_tx_ctlr->Intc,
			TmrIntrId,
			(XInterruptHandler)Exdes_SysTimerIntrHandler,
			(void *)InstancePtr);

	XScuGic_Enable(InstancePtr->hdmi_tx_ctlr->Intc, TmrIntrId);
#else
	Status |= XIntc_Connect(InstancePtr->hdmi_tx_ctlr->Intc,
			TmrIntrId,
			(XInterruptHandler)Exdes_SysTimerIntrHandler,
			(void *)&xhdmi_exdes_ctrlr);

	XIntc_Enable(InstancePtr->hdmi_tx_ctlr->Intc, TmrIntrId);
#endif
#endif
	return Status;
}

/*****************************************************************************/
/**
*
* This function time specified in Us to ticks for a specified timer frequency.
*
* @param    TimeInUs is the time in microseconds.
* @param    TmrCtrClkFreq is the frequency of the clock to be used for
*           counting the ticks.
*
* @return   Number of ticks.
*
* @note	    None.
*
******************************************************************************/
u32 Exdes_UsToTicks(u32 TimeInUs, u32 TmrCtrClkFreq)
{
	u32 TimeoutFreq = 0;
	u32 NumTicks = 0;

	/* Check for greater than one second */
	if (TimeInUs > 1000000ul) {
		u32 NumSeconds = 0;

		/* Determine theNumSeconds */
		NumSeconds = (TimeInUs/1000000ul);

		/* Update theNumTicks */
		NumTicks = (NumSeconds*TmrCtrClkFreq);

		/* Adjust theTimeoutInUs */
		TimeInUs -= (NumSeconds*1000000ul);
	}

	/* Convert TimeoutFreq to a frequency */
	TimeoutFreq  = 1000;
	TimeoutFreq *= 1000;
	TimeoutFreq /= TimeInUs;

	/* Update NumTicks */
	NumTicks += ((TmrCtrClkFreq / TimeoutFreq) + 1);

	return (NumTicks);
}

/*****************************************************************************/
/**
*
* This function is used to start a the timer to generate periodic pulses of
* tmrctr interrupt.
*
* @param    InstancePtr is a pointer to the example design handler.
* @param    IntervalInMs is the user specified period for the timer pulses.
*
* @return   None.
*
* @note	    None.
*
******************************************************************************/
void Exdes_StartSysTmr(XHdmi_Exdes *InstancePtr, u32 IntervalInMs)
{
	u32 NumTicks = 0;
	u32 TimerOptions = 0;
	u8 TmrCtrNumber = 0;

	NumTicks = Exdes_UsToTicks((IntervalInMs * 1000),
			InstancePtr->SysTmrInst.Config.SysClockFreqHz);

	/* Calculate the number of maximum pulses that can counted upto
	 * at which point we need to reset the counters.
	 */
	InstancePtr->SysTmrPulseIntervalinMs = IntervalInMs;

	/* Set the number of pulses in 1 second time. */
	/* The user can add more counts here for 500ms, 2 seconds etc.
	 * and use them to control any time based operations in
	 * Exdes_SysTmrCallback tmrctr callback handler.
	 */
	InstancePtr->TmrPulseCnt1second =
				1000 / InstancePtr->SysTmrPulseIntervalinMs;

	/* Stop the timer */
	XTmrCtr_Stop(&InstancePtr->SysTmrInst, TmrCtrNumber);

	/* Configure the callback */
	XTmrCtr_SetHandler(&InstancePtr->SysTmrInst,
			   &Exdes_SysTmrCallback,
			   (void*)InstancePtr);

	/* Configure the timer options to generate a pulse train of
	 * interrupts of IntervalInMs periods. */
	TimerOptions  = XTmrCtr_GetOptions(&InstancePtr->SysTmrInst,
					   TmrCtrNumber);
	TimerOptions |= (XTC_DOWN_COUNT_OPTION |
					XTC_INT_MODE_OPTION |
					XTC_AUTO_RELOAD_OPTION);
	XTmrCtr_SetOptions(&InstancePtr->SysTmrInst,
			   TmrCtrNumber, TimerOptions);

	xil_printf("System timer configured to generate (and count)"
			"pulses at %dms intervals.\r\n",
			InstancePtr->SysTmrPulseIntervalinMs);

	/* Set the timeout and start */
	XTmrCtr_SetResetValue(&InstancePtr->SysTmrInst,
			      TmrCtrNumber, NumTicks);
	XTmrCtr_Start(&InstancePtr->SysTmrInst, TmrCtrNumber);
}

/*****************************************************************************/
/**
*
* This function is set as the callback handler from the timer counter
* interrupt handler. The counters that are used in the application for
* timer based control and exclusion are be handled in the this function.
*
* The user needs to modify this callback to set the counters and
* exclusion flags based on the pulse counts in the pulse train generated
* by the system timer.
*
* @param    CallbackRef is the function callback reference.
* @param    TmrCtrNumber is the timer counter number.
*
* @return   None.
*
* @note	    None.
*
******************************************************************************/
void Exdes_SysTmrCallback(void *CallbackRef, u8 TmrCtrNumber)
{
	XHdmi_Exdes *InstancePtr = (XHdmi_Exdes *)CallbackRef;

	/* Increment the hdcp pulse counters. */
	InstancePtr->HdcpPulseCounter++;

	/* Check if the Hdcp pulse count has exceeded the reset value
	 * based on the required time out for exception in HDCP polling to
	 * check downstream authentication. */
	if (InstancePtr->HdcpPulseCounter > InstancePtr->TmrPulseCnt1second) {
		/* Reset the hdcp pulse counter */
		InstancePtr->HdcpPulseCounter = 0;
#if defined(XPAR_XV_HDMITXSS1_NUM_INSTANCES)
		/* Free the Hdcp to attempt a authentication on
		 * the transmitter if needed. */
		XV_Tx_SetHdcpAuthReqExclusion(InstancePtr->hdmi_tx_ctlr, FALSE);
#endif
	}

	/* Stop the timer */
	/* XTmrCtr_Stop(&InstancePtr->SysTmrInst, TmrCtrNumber); */
}

/*****************************************************************************/
/**
*
* This function is set as the callback handler from the interrupt controller
* for the system timer interrupts.
*
* @param    InstancePtr is the callback reference.
*
* @return   None.
*
* @note	    None.
*
******************************************************************************/
void Exdes_SysTimerIntrHandler(void *CallbackRef)
{
	XHdmi_Exdes *InstancePtr = (XHdmi_Exdes *)CallbackRef;
	XTmrCtr_InterruptHandler(&InstancePtr->SysTmrInst);
}

#if defined (XPAR_XV_FRMBUFWR_NUM_INSTANCES) && \
                      (XPAR_XV_FRMBUFRD_NUM_INSTANCES)
/*****************************************************************************/
/**
*
* This function is used to initialize the frame buffer.
*
* @param  WrInstancePtr is the instance of the write frame buffer.
* @param  RdInstancePtr is the instance of the read frame buffer.
* @param  rstInstancePtr is the instance of the GPIO.
*
* @return XST_SUCCESS on successful initialization.
*         XAT_FAILURE on initialization failure.
*
* @note
*
******************************************************************************/
u32 Exdes_FBInitialize(XV_FrmbufWr_l2 *WrInstancePtr,
		       XV_FrmbufRd_l2 *RdInstancePtr,
		       XGpioPs *rstInstancePtr)
{
	u32 Status = XST_SUCCESS;
#ifdef XPAR_XV_FRMBUFRD_NUM_INSTANCES
	/* Video Frame Buffer Write */
	XV_frmbufwr_Config *FrameBufWr_ConfigPtr;
#endif
#ifdef XPAR_XV_FRMBUFRD_NUM_INSTANCES
	/* Video Frame Buffer Read */
	XV_frmbufrd_Config *FrameBufRd_ConfigPtr;
#ifdef XPAR_XGPIOPS_NUM_INSTANCES
	/* GPIO Reset for Video Frame Buffer */
	XGpioPs_Config     *Gpio_VFRB_resetn_ConfigPtr;
#endif
#endif

	/* Initialize Buffer */
	for (u8 i = 0; i < 5; i++) {
		VidBuff[i].BaseAddr = DDR_BASE_ADDRESS + ((0x10000000) * (i + 1));
		VidBuff[i].ChromaBaseAddr = VidBuff[i].BaseAddr + (0x05000000U);
		VidBuff[i].DynHDRBaseAddr = VidBuff[i].BaseAddr + (0x0D000000U);
	}

	/* Initialize Video Frame Buffer Read */
	FrameBufRd_ConfigPtr =
			XV_frmbufrd_LookupConfig(XPAR_XV_FRMBUFRD_0_DEVICE_ID);

	if (FrameBufRd_ConfigPtr == NULL) {
		RdInstancePtr->FrmbufRd.IsReady = 0;
		return (XST_DEVICE_NOT_FOUND);
	}

	Status = XV_frmbufrd_CfgInitialize(&RdInstancePtr->FrmbufRd,
					   FrameBufRd_ConfigPtr,
					   FrameBufRd_ConfigPtr->BaseAddress);

	if (Status != XST_SUCCESS) {
		xil_printf("ERR:: Video Frame Buffer Read ");
		xil_printf("Initialization failed %d\r\n", Status);
		return(XST_FAILURE);
	}

	if (Status != XST_SUCCESS) {
		xil_printf("ERR:: Video Frame Buffer Read ");
		xil_printf("Initialization failed %d\r\n", Status);
		return(XST_FAILURE);
	}

#if defined(__arm__) || (__aarch64__)
	/* Register Video Frame Buffer Read Interrupt Handler
	 *  with Interrupt Controller
	 */
	Status |= XScuGic_Connect(&Intc,
				  XPAR_FABRIC_V_FRMBUF_RD_0_VEC_ID,
				  (XInterruptHandler)XVFrmbufRd_InterruptHandler,
				  (void *)RdInstancePtr);
#else
	/* Register Video Frame Buffer Read Interrupt Handler
	 *  with Interrupt Controller
	 */
	Status |= XIntc_Connect(&Intc,
				XPAR_INTC_0_V_FRMBUF_RD_0_VEC_ID,
				(XInterruptHandler)XVFrmbufRd_InterruptHandler,
				(void *)RdInstancePtr);
#endif

	if (Status == XST_SUCCESS) {
#if defined(__arm__) || (__aarch64__)
		XScuGic_Enable(&Intc, XPAR_FABRIC_V_FRMBUF_RD_0_VEC_ID);
#else
		XIntc_Enable(&Intc, XPAR_INTC_0_V_FRMBUF_RD_0_VEC_ID);
#endif
	} else {
		xil_printf("ERR:: Unable to register Vid. Frame "
			   "Buff. Rd interrupt handler\r\n");
		xil_printf("Vid. Frame Buff. Rd initialization error\r\n");
		return XST_FAILURE;
	}

	XVFrmbufRd_SetCallback(RdInstancePtr,
			       XVFRMBUFRD_HANDLER_DONE,
			       (void *)VidFrameBufRdDone,
			       (void *)RdInstancePtr);

	xil_printf("Video Frame Buffer Read Initialization Complete\r\n");

	/* Initialize Video Frame Buffer Write */
	FrameBufWr_ConfigPtr =
			XV_frmbufwr_LookupConfig(XPAR_XV_FRMBUFWR_0_DEVICE_ID);

	if (FrameBufWr_ConfigPtr == NULL) {
		WrInstancePtr->FrmbufWr.IsReady = 0;
		return (XST_DEVICE_NOT_FOUND);
	}

	Status = XV_frmbufwr_CfgInitialize(&WrInstancePtr->FrmbufWr,
					   FrameBufWr_ConfigPtr,
					   FrameBufWr_ConfigPtr->BaseAddress);

	if (Status != XST_SUCCESS) {
		xil_printf("ERR:: Video Frame Buffer Write ");
		xil_printf("Initialization failed %d\r\n", Status);
		return(XST_FAILURE);
	}


#if defined(__arm__) || (__aarch64__)
	/* Register Video Frame Buffer Write Interrupt Handler
	 *  with Interrupt Controller
	 */
	Status |= XScuGic_Connect(&Intc,
				  XPAR_FABRIC_V_FRMBUF_WR_0_VEC_ID,
				  (XInterruptHandler)XVFrmbufWr_InterruptHandler,
				  (void *)WrInstancePtr);
#else
	/* Register Video Frame Buffer Write Interrupt Handler
	 *  with Interrupt Controller
	 */
	Status |= XIntc_Connect(&Intc,
				XPAR_INTC_0_V_FRMBUF_WR_0_VEC_ID,
				(XInterruptHandler)XVFrmbufWr_InterruptHandler,
				(void *)WrInstancePtr);
#endif

	if (Status == XST_SUCCESS) {
#if defined(__arm__) || (__aarch64__)
		XScuGic_Enable(&Intc, XPAR_FABRIC_V_FRMBUF_WR_0_VEC_ID);
#else
		XIntc_Enable(&Intc, XPAR_INTC_0_V_FRMBUF_WR_0_VEC_ID);
#endif
	} else {
		xil_printf("ERR:: Unable to register Vid. Frame "
			   "Buff. Wr interrupt handler");
		return XST_FAILURE;
	}

	XVFrmbufWr_SetCallback(WrInstancePtr,
			       XVFRMBUFWR_HANDLER_DONE,
			       (void *)VidFrameBufWrDone,
			       (void *)WrInstancePtr);

	xil_printf("Video Frame Buffer Write Initialization Complete\r\n");

#ifdef XPAR_XGPIOPS_NUM_INSTANCES
	/* Initialize GPIO for VFRB Reset */
	Gpio_VFRB_resetn_ConfigPtr =
			XGpioPs_LookupConfig(XPAR_PSU_GPIO_0_DEVICE_ID);

	if (Gpio_VFRB_resetn_ConfigPtr == NULL) {
		rstInstancePtr->IsReady = 0;
		return (XST_DEVICE_NOT_FOUND);
	}

	Status = XGpioPs_CfgInitialize(rstInstancePtr,
				       Gpio_VFRB_resetn_ConfigPtr,
				       Gpio_VFRB_resetn_ConfigPtr->BaseAddr);

	if (Status != XST_SUCCESS) {
		xil_printf("ERR:: GPIO for VFRB Reset ");
		xil_printf("Initialization failed %d\r\n", Status);
		return(XST_FAILURE);
	}
#endif

	return (Status);
}

#if defined (XPAR_XV_FRMBUFWR_NUM_INSTANCES) && \
	    (XPAR_XV_FRMBUFRD_NUM_INSTANCES)

/*****************************************************************************/
/**
*
* This function calculates and returns the stride value in bytes.
*
* @param  Cfmt is the color format.
* @param  AxiMMDataWidht is the data width.
* @param  StreamPtr is a pointer to the video stream.
*
* @return stride in bytes.
*
* @note
*
******************************************************************************/
static u32 CalcStride(XVidC_ColorFormat Cfmt,
                      u16 AXIMMDataWidth,
                      XVidC_VideoStream *StreamPtr)
{
	u32 stride;
	int width = StreamPtr->Timing.HActive;
	u16 MMWidthBytes = AXIMMDataWidth/8;

	if ((Cfmt == XVIDC_CSF_MEM_Y_UV10) ||
	    (Cfmt == XVIDC_CSF_MEM_Y_UV10_420) ||
	    (Cfmt == XVIDC_CSF_MEM_Y10)) {
		/* 4 bytes per 3 pixels (Y_UV10, Y_UV10_420, Y10) */
		stride = ((((width * 4) / 3) + MMWidthBytes - 1) /
		          MMWidthBytes) *
		         MMWidthBytes;
	} else if ((Cfmt == XVIDC_CSF_MEM_Y_UV8) ||
	           (Cfmt == XVIDC_CSF_MEM_Y_UV8_420) ||
	           (Cfmt == XVIDC_CSF_MEM_Y8)) {
		/* 1 byte per pixel (Y_UV8, Y_UV8_420, Y8) */
		stride = ((width + MMWidthBytes - 1 ) / MMWidthBytes) *
		         MMWidthBytes;
	} else if ((Cfmt == XVIDC_CSF_MEM_RGB8) ||
	           (Cfmt == XVIDC_CSF_MEM_YUV8) ||
	           (Cfmt == XVIDC_CSF_MEM_BGR8)) {
		/* 3 bytes per pixel (RGB8, YUV8, BGR8) */
		stride = (((width * 3) + MMWidthBytes - 1) / MMWidthBytes) *
		         MMWidthBytes;
	} else if ((Cfmt == XVIDC_CSF_MEM_RGBX10) ||
	           (Cfmt == XVIDC_CSF_MEM_YUVX10)) {
		/* 4 bytes per pixel */
		stride = (((width * 4) + MMWidthBytes - 1) / MMWidthBytes) *
		         MMWidthBytes;
	} else if ((Cfmt == XVIDC_CSF_MEM_Y_UV12) ||
			   (Cfmt == XVIDC_CSF_MEM_Y_UV12_420)) {
		/* 4 bytes per pixel */
		stride = ((((width * 3) / 2) + MMWidthBytes - 1) / MMWidthBytes) *
		         MMWidthBytes;
	} else if ((Cfmt == XVIDC_CSF_MEM_RGBX12) ||
			   (Cfmt == XVIDC_CSF_MEM_YUVX12)) {
		/* 4 bytes per pixel */
		stride = (((width * 5) + MMWidthBytes - 1) / MMWidthBytes) *
				 MMWidthBytes;
	} else {
		xil_printf ("Unsupported format passed to stride???\r\n");
	}

	return(stride);
}

/*****************************************************************************/
/**
*
* This function configures the frames buffer(s).
*
* @param  FrmBufWrPtr is the write frame buffer.
* @param  FrmBufRdPtr is the read frame buffer.
*
* @return None.
*
* @note
*
******************************************************************************/
void XV_ConfigVidFrameBuf_rd(XV_FrmbufRd_l2 *FrmBufRdPtr)
{

	XV_FrmbufRd_l2        *pFrameBufRd = FrmBufRdPtr;
	u32                   Status, width, height, stride;
	XVidC_VideoStream     *HdmiTxSsVidStreamPtr;
	XVidC_VideoMode       VideoMode;
	XVidC_ColorFormat     MemColorFmt;
	XVidC_ColorDepth      ConfigColorDepth ;

	offset = 0;
	StartToRead = FALSE;

#ifdef XPAR_XV_HDMIRXSS1_NUM_INSTANCES
	XHdmiC_AVI_InfoFrame  *AVIInfoFramePtr;
#endif

	xil_printf(ANSI_COLOR_GREEN"XV_ConfigVidFrameBuf_rd "
				   "Start!"ANSI_COLOR_RESET"\r\n");

	/* Set Value */
	HdmiTxSsVidStreamPtr = XV_HdmiTxSs1_GetVideoStream(&HdmiTxSs);
	memcpy(&HdmiTxSsVidStreamPtrFbRd, HdmiTxSsVidStreamPtr, sizeof(XVidC_VideoStream));
	VideoMode = HdmiTxSsVidStreamPtrFbRd.VmId;

	/* If Color bar, the 480i/576i HActive need to be divided by 2 */
	/* 1440x480i/1440x576i --> 720x480i/720x576i */
#ifdef XPAR_XV_HDMIRXSS1_NUM_INSTANCES
	if (xhdmi_exdes_ctrlr.ForceIndependent == TRUE) {
#endif
		/* NTSC/PAL Support */
		if ((VideoMode == XVIDC_VM_1440x480_60_I) ||
		    (VideoMode == XVIDC_VM_1440x576_50_I) ) {
			width  = HdmiTxSsVidStreamPtrFbRd.Timing.HActive/2;
			height = HdmiTxSsVidStreamPtrFbRd.Timing.VActive;
		} else {
			/* If not NTSC/PAL, the HActive,
			* and VActive remain as it is
			*/
			width  = HdmiTxSsVidStreamPtrFbRd.Timing.HActive;
			height = HdmiTxSsVidStreamPtrFbRd.Timing.VActive;
		}
#ifdef XPAR_XV_HDMIRXSS1_NUM_INSTANCES
	} else {
		/*
		 * Get the Current HDMI RX AVI Info Frame
		 * As this design is pass-through, is save to read the incoming
		 * stream Info frame
		 */
		AVIInfoFramePtr = XV_HdmiRxSs1_GetAviInfoframe(&HdmiRxSs);

		/* If the incoming (HDMI RX) info frame (pixel repetition) = 2
		* The 480i/576i HActive need to be divided by 2
		*/
		if (AVIInfoFramePtr->PixelRepetition ==
		    XHDMIC_PIXEL_REPETITION_FACTOR_2) {
			width  = HdmiTxSsVidStreamPtrFbRd.Timing.HActive/2;
			height = HdmiTxSsVidStreamPtrFbRd.Timing.VActive;
		} else {
			/* If Pixel Repetition != 2, the HActive, and VActive
			* remain as it is */
			width  = HdmiTxSsVidStreamPtrFbRd.Timing.HActive;
			height = HdmiTxSsVidStreamPtrFbRd.Timing.VActive;
		}
	}
#endif

	/* Set Start Address of the Buffer */
	FrameRdCompCnt = 0;

	/* Update the width and height */
	HdmiTxSsVidStreamPtrFbRd.Timing.HActive = width;
	HdmiTxSsVidStreamPtrFbRd.Timing.VActive = height;

	ConfigColorDepth = HdmiRxSs.Config.MaxBitsPerPixel ;

	/* Map Stream Color Format with Memory Color Format */
	switch (HdmiTxSsVidStreamPtrFbRd.ColorFormatId) {
		case XVIDC_CSF_RGB:
			if ((HdmiTxSsVidStreamPtrFbRd.ColorDepth == XVIDC_BPC_8) ||
				(ConfigColorDepth == XVIDC_BPC_8)){
				MemColorFmt = XVIDC_CSF_MEM_RGB8;
			} else if ((HdmiTxSsVidStreamPtrFbRd.ColorDepth == XVIDC_BPC_10)||
				(ConfigColorDepth == XVIDC_BPC_10)) {
				MemColorFmt = XVIDC_CSF_MEM_RGBX10;
			} else if (HdmiTxSsVidStreamPtrFbRd.ColorDepth == XVIDC_BPC_12) {
				MemColorFmt = XVIDC_CSF_MEM_RGBX12;
			}
			break;
		case XVIDC_CSF_YCRCB_444:
			if ((HdmiTxSsVidStreamPtrFbRd.ColorDepth == XVIDC_BPC_8) ||
				(ConfigColorDepth == XVIDC_BPC_8)){
				MemColorFmt = XVIDC_CSF_MEM_YUV8;
			} else if ((HdmiTxSsVidStreamPtrFbRd.ColorDepth == XVIDC_BPC_10)||
				(ConfigColorDepth == XVIDC_BPC_10)) {
				MemColorFmt = XVIDC_CSF_MEM_YUVX10;
			} else if (HdmiTxSsVidStreamPtrFbRd.ColorDepth == XVIDC_BPC_12) {
				MemColorFmt = XVIDC_CSF_MEM_YUVX12;
			}
			break;
		case XVIDC_CSF_YCRCB_422:
			//always 12bpc
			if ((HdmiTxSsVidStreamPtrFbRd.ColorDepth == XVIDC_BPC_8) ||
				(ConfigColorDepth == XVIDC_BPC_8)){
				MemColorFmt = XVIDC_CSF_MEM_Y_UV8;
			} else if ((HdmiTxSsVidStreamPtrFbRd.ColorDepth == XVIDC_BPC_10)||
				(ConfigColorDepth == XVIDC_BPC_10)) {
				MemColorFmt = XVIDC_CSF_MEM_Y_UV10;
			} else if (HdmiTxSsVidStreamPtrFbRd.ColorDepth == XVIDC_BPC_12) {
				MemColorFmt = XVIDC_CSF_MEM_Y_UV12;
			}
			break;
		case XVIDC_CSF_YCRCB_420:


			if ((HdmiTxSsVidStreamPtrFbRd.ColorDepth == XVIDC_BPC_8) ||
				(ConfigColorDepth == XVIDC_BPC_8)){
				MemColorFmt = XVIDC_CSF_MEM_Y_UV8_420;
			} else if ((HdmiTxSsVidStreamPtrFbRd.ColorDepth == XVIDC_BPC_10)||
				(ConfigColorDepth == XVIDC_BPC_10)) {
				MemColorFmt = XVIDC_CSF_MEM_Y_UV10_420;
			} else if (HdmiTxSsVidStreamPtrFbRd.ColorDepth == XVIDC_BPC_12) {
				MemColorFmt = XVIDC_CSF_MEM_Y_UV12_420;
			}
			break;
		default:
			MemColorFmt = XVIDC_CSF_MEM_RGB8;
			break;
	}

	XVFrmbufRd_Stop(pFrameBufRd);
	ResetFrameBuf(0x1);

	Status = XVFrmbufRd_WaitForIdle(pFrameBufRd);
	if (Status != XST_SUCCESS) {
		xil_printf("Frame Buffer Read not Idle\r\n");
	}

	/* Initialize Frame Buffer Read on Pass-through */
	/* Calculate the Stride */
	stride = CalcStride(MemColorFmt,
			    pFrameBufRd->FrmbufRd.Config.AXIMMDataWidth,
				&HdmiTxSsVidStreamPtrFbRd);

	/* When crop to TMDS use the stride of Wr FB */
	if (xhdmi_exdes_ctrlr.crop &&
			HdmiTxSs.HdmiTx1Ptr->Stream.IsFrl != TRUE) {
		stride = Wr_stride;
	}

	/* Configure Frame Buffer Read */
	Status = XVFrmbufRd_SetMemFormat(pFrameBufRd,
					 stride,
					 MemColorFmt,
					 &HdmiTxSsVidStreamPtrFbRd);

	if (Status != XST_SUCCESS) {
		xil_printf ("XVFrmbufRd_SetMemFormat Failed: %d\r\n", Status);
		return;
	}

	/* Set Buffer Base Address */
	XVFrmbufRd_SetBufferAddr(pFrameBufRd, VidBuff[rd].BaseAddr);

	/* Set Chroma Buffer Base Address */
	if (HdmiTxSsVidStreamPtrFbRd.ColorFormatId == XVIDC_CSF_YCRCB_422 ||
		HdmiTxSsVidStreamPtrFbRd.ColorFormatId == XVIDC_CSF_YCRCB_420) {
		XVFrmbufRd_SetChromaBufferAddr(pFrameBufRd,
					       VidBuff[rd].ChromaBaseAddr);
	}

	/* Enable Frame Buffer Read Interrupt */
	XVFrmbufRd_InterruptEnable(pFrameBufRd,
				   (XVFRMBUFRD_IRQ_DONE_MASK |
				    XVFRMBUFRD_IRQ_READY_MASK));
#if ((VRR_MODE == 0) || (VRR_MODE ==1)) // No VRR  || Manual
	XV_frmbufrd_EnableAutoRestart(&FrameBufRd.FrmbufRd);
	XVFrmbufRd_Start(pFrameBufRd);
#endif
	StartToRead = TRUE;
	read_finsihed = 1 ;

	xil_printf(ANSI_COLOR_GREEN"XV_ConfigVidFrameBuf "
				   "End!"ANSI_COLOR_RESET"\r\n");
}

/*****************************************************************************/
/**
*
* This function configures the frames buffer(s).
*
* @param  FrmBufWrPtr is the write frame buffer.
* @param  FrmBufRdPtr is the read frame buffer.
*
* @return None.
*
* @note
*
******************************************************************************/
void XV_ConfigVidFrameBuf_wr(XV_FrmbufWr_l2 *FrmBufWrPtr)
{

	XV_FrmbufWr_l2        *pFrameBufWr = FrmBufWrPtr;

	u32                   Status, width, height, stride;
	XVidC_VideoStream     *HdmiRxSsVidStreamPtr;
	XVidC_VideoMode       VideoMode;
	XVidC_ColorFormat     MemColorFmt;
	XVidC_ColorDepth      ConfigColorDepth ;

	u32 hdmi20_limit = 0;
	u8 bits_per_comp = 0;
	u8 bits_per_comp_div = 0;

#ifdef XPAR_XV_HDMIRXSS1_NUM_INSTANCES
	XHdmiC_AVI_InfoFrame  *AVIInfoFramePtr;
#endif

	xil_printf(ANSI_COLOR_GREEN"XV_ConfigVidFrameBuf_wr "
				   "Start!"ANSI_COLOR_RESET"\r\n");

	/* Set Value */
	HdmiRxSsVidStreamPtr = XV_HdmiRxSs1_GetVideoStream(&HdmiRxSs);
	memcpy (&HdmiRxSsVidStreamPtrFbWr, HdmiRxSsVidStreamPtr, sizeof(XVidC_VideoStream));
	VideoMode = HdmiRxSsVidStreamPtrFbWr.VmId;

	/* If Color bar, the 480i/576i HActive need to be divided by 2 */
	/* 1440x480i/1440x576i --> 720x480i/720x576i */
#ifdef XPAR_XV_HDMIRXSS1_NUM_INSTANCES
	if (xhdmi_exdes_ctrlr.ForceIndependent == TRUE) {
#endif
		/* NTSC/PAL Support */
		if ((VideoMode == XVIDC_VM_1440x480_60_I) ||
		    (VideoMode == XVIDC_VM_1440x576_50_I) ) {
			width  = HdmiRxSsVidStreamPtrFbWr.Timing.HActive/2;
			height = HdmiRxSsVidStreamPtrFbWr.Timing.VActive;
		} else {
			/* If not NTSC/PAL, the HActive,
			* and VActive remain as it is
			*/
			width  = HdmiRxSsVidStreamPtrFbWr.Timing.HActive;
			height = HdmiRxSsVidStreamPtrFbWr.Timing.VActive;
		}
#ifdef XPAR_XV_HDMIRXSS1_NUM_INSTANCES
	} else {
		/* Get the Current HDMI RX AVI Info Frame
		* As this design is pass-through, is save to read the incoming
		* stream Info frame
		*/
		AVIInfoFramePtr = XV_HdmiRxSs1_GetAviInfoframe(&HdmiRxSs);

		/* If the incoming (HDMI RX) info frame (pixel repetition) = 2
		* The 480i/576i HActive need to be divided by 2
		*/
		if (AVIInfoFramePtr->PixelRepetition ==
		    XHDMIC_PIXEL_REPETITION_FACTOR_2) {
			width  = HdmiRxSsVidStreamPtrFbWr.Timing.HActive/2;
			height = HdmiRxSsVidStreamPtrFbWr.Timing.VActive;
		} else {
			/* If Pixel Repetition != 2, the HActive, and VActive
			* remain as it is */
			width  = HdmiRxSsVidStreamPtrFbWr.Timing.HActive;
			height = HdmiRxSsVidStreamPtrFbWr.Timing.VActive;
		}
	}
#endif

	/* Set Start Address of the Buffer */
	StartStream = (FALSE);
	FrameWrCompCnt = 0;



	/* Update the width and height */
	HdmiRxSsVidStreamPtrFbWr.Timing.HActive = width;
	HdmiRxSsVidStreamPtrFbWr.Timing.VActive = height;
	ConfigColorDepth = HdmiRxSs.Config.MaxBitsPerPixel ;
	/* Map Stream Color Format with Memory Color Format */
	switch (HdmiRxSsVidStreamPtrFbWr.ColorFormatId) {
		case XVIDC_CSF_RGB:
			bits_per_comp = 3;
			bits_per_comp_div = 1;
			if ((HdmiRxSsVidStreamPtrFbWr.ColorDepth == XVIDC_BPC_8) ||
				(ConfigColorDepth == XVIDC_BPC_8)){
				MemColorFmt = XVIDC_CSF_MEM_RGB8;
			} else if ((HdmiRxSsVidStreamPtrFbWr.ColorDepth == XVIDC_BPC_10)||
				(ConfigColorDepth == XVIDC_BPC_10)) {
				MemColorFmt = XVIDC_CSF_MEM_RGBX10;
			} else if (HdmiRxSsVidStreamPtrFbWr.ColorDepth == XVIDC_BPC_12) {
				MemColorFmt = XVIDC_CSF_MEM_RGBX12;
			}
			break;
		case XVIDC_CSF_YCRCB_444:
			bits_per_comp = 3;
			bits_per_comp_div = 1;

			if ((HdmiRxSsVidStreamPtrFbWr.ColorDepth == XVIDC_BPC_8) ||
				(ConfigColorDepth == XVIDC_BPC_8)){
				MemColorFmt = XVIDC_CSF_MEM_YUV8;
			} else if ((HdmiRxSsVidStreamPtrFbWr.ColorDepth == XVIDC_BPC_10)||
				(ConfigColorDepth == XVIDC_BPC_10)) {
				MemColorFmt = XVIDC_CSF_MEM_YUVX10;
			} else if (HdmiRxSsVidStreamPtrFbWr.ColorDepth == XVIDC_BPC_12) {
				MemColorFmt = XVIDC_CSF_MEM_YUVX12;
			}
			break;
		case XVIDC_CSF_YCRCB_422:
			bits_per_comp = 2;
			bits_per_comp_div = 1;
			/* always 12bpc */
			if ((HdmiRxSsVidStreamPtrFbWr.ColorDepth == XVIDC_BPC_8) ||
				(ConfigColorDepth == XVIDC_BPC_8)){
				MemColorFmt = XVIDC_CSF_MEM_Y_UV8;
			} else if ((HdmiRxSsVidStreamPtrFbWr.ColorDepth == XVIDC_BPC_10)||
				(ConfigColorDepth == XVIDC_BPC_10)) {
				MemColorFmt = XVIDC_CSF_MEM_Y_UV10;
			} else if (HdmiRxSsVidStreamPtrFbWr.ColorDepth == XVIDC_BPC_12) {
				MemColorFmt = XVIDC_CSF_MEM_Y_UV12;
			}
			break;
		case XVIDC_CSF_YCRCB_420:
			bits_per_comp = 3;
			bits_per_comp_div = 2;
			if ((HdmiRxSsVidStreamPtrFbWr.ColorDepth == XVIDC_BPC_8) ||
				(ConfigColorDepth == XVIDC_BPC_8)){
				MemColorFmt = XVIDC_CSF_MEM_Y_UV8_420;
			} else if ((HdmiRxSsVidStreamPtrFbWr.ColorDepth == XVIDC_BPC_10)||
				(ConfigColorDepth == XVIDC_BPC_10)) {
				MemColorFmt = XVIDC_CSF_MEM_Y_UV10_420;
			} else if (HdmiRxSsVidStreamPtrFbWr.ColorDepth == XVIDC_BPC_12) {
				MemColorFmt = XVIDC_CSF_MEM_Y_UV12_420;
			}
			break;
		default:
			MemColorFmt = XVIDC_CSF_MEM_RGB8;
			break;
	}

	hdmi20_limit = (HdmiRxSsVidStreamPtrFbWr.Timing.HTotal * HdmiRxSsVidStreamPtrFbWr.Timing.F0PVTotal *
			HdmiRxSsVidStreamPtrFbWr.FrameRate) / 1000;
	hdmi20_limit = (hdmi20_limit * HdmiRxSsVidStreamPtrFbWr.ColorDepth);
	hdmi20_limit = (hdmi20_limit * bits_per_comp) / bits_per_comp_div;
	hdmi20_limit = hdmi20_limit * 10 /8;

    /* Bandwidth of the incoming video is calculated above
     * If this BW exceeds 18G and the TX is non FRL then the
     * flag crop is set.
     * The TX TMDS will only display 4K video
     */

	if ((hdmi20_limit < 18000000) ||
			(xhdmi_exdes_ctrlr.IsTxPresent && HdmiTxSs.HdmiTx1Ptr->Stream.IsFrl)){
		/* Video possible to be sent over TMDS */
		xhdmi_exdes_ctrlr.crop = FALSE;
	} else {
		/* Video not possible to be sent over TMDS */
		xhdmi_exdes_ctrlr.crop = TRUE;
	}

	XVFrmbufWr_Stop(pFrameBufWr);
	ResetFrameBuf(0x0);
	HdmiRxSsVidStreamPtrFbWr.Timing.HActive = width;
	HdmiRxSsVidStreamPtrFbWr.Timing.VActive = height;

	/* Reset Frame Buffer Write */
	Status = XVFrmbufWr_WaitForIdle(pFrameBufWr);
	if (Status != XST_SUCCESS) {
		xil_printf("Frame Buffer Write not Idle\r\n");
	}

	/* Initialize Frame Buffer Write only on Pass-through */
	/* Calculate the Stride */
	stride = CalcStride(MemColorFmt,
			    pFrameBufWr->FrmbufWr.Config.AXIMMDataWidth,
				&HdmiRxSsVidStreamPtrFbWr);
	Wr_stride = stride;

	/* Configure Frame Buffer Write */
	Status = XVFrmbufWr_SetMemFormat(pFrameBufWr,
					 stride,
					 MemColorFmt,
					 &HdmiRxSsVidStreamPtrFbWr);

	if (Status != XST_SUCCESS) {
		xil_printf ("XVFrmbufWr_SetMemFormat Failed: %d\r\n", Status);
		return;
	}

	/* Set Buffer Base Address */
	XVFrmbufWr_SetBufferAddr(pFrameBufWr, VidBuff[0].BaseAddr);

	/* Set Chroma Buffer Base Address */
	if (HdmiRxSsVidStreamPtrFbWr.ColorFormatId == XVIDC_CSF_YCRCB_422 ||
		HdmiRxSsVidStreamPtrFbWr.ColorFormatId == XVIDC_CSF_YCRCB_420) {
		XVFrmbufWr_SetChromaBufferAddr(pFrameBufWr,
					       VidBuff[0].ChromaBaseAddr);
	}

	/* Enable Frame Buffer Write Interrupt */
	XVFrmbufWr_InterruptEnable(pFrameBufWr,
				   (XVFRMBUFWR_IRQ_DONE_MASK |
				    XVFRMBUFWR_IRQ_READY_MASK));

	/* Start Frame Buffer Write */
	XV_frmbufwr_EnableAutoRestart(&FrameBufWr.FrmbufWr);
	XVFrmbufWr_Start(pFrameBufWr);
	read_finsihed = 1 ;
	write_finsihed = 0;
	xil_printf(ANSI_COLOR_GREEN"XV_ConfigVidFrameBuf Wr "
				   "End!"ANSI_COLOR_RESET"\r\n");
}

/*****************************************************************************/
/**
*
* This function is called when the Video Frame Buffer Read Done
*
* @param  CallbackRef is the reference passes to this function.
*
* @return None.
*
* @note   None.
*
******************************************************************************/
void VidFrameBufRdDone(void *CallbackRef)
{
		/* Read Buffer Address is decided in Wr interrupt for simplicity */
		FrRdDoneCounter++;
}

/*****************************************************************************/
/**
*
* This function is called when the Video Frame Buffer Write Done
* "ap_done" is triggered after the frame processing is complete
* frame
*
* @param  CallbackRef is the reference passes to this function.
*
* @return None.
*
* @note   None.
*
******************************************************************************/
void VidFrameBufWrDone(void *CallbackRef)
{
	u8 CurrWrPage = wr;
	u8 CurrRdPage = rd;

	if (CurrWrPage == 3) {
		wr = 0;
	} else {
		wr++;
	}

	if (CurrRdPage == 3) {
		rd = 0;
	} else {
		rd++;
	}

	/* Writing the Buf Addr for FB Write */
	XVFrmbufWr_SetBufferAddr(&FrameBufWr, VidBuff[wr].BaseAddr);
	XVFrmbufWr_SetChromaBufferAddr(&FrameBufWr,
				       VidBuff[wr].ChromaBaseAddr);

	/* Writing the Buf Addr for FB Read */
	if (StartToRead) {
		XVFrmbufRd_SetBufferAddr(&FrameBufRd,
					 (VidBuff[rd].BaseAddr + offset));
		XVFrmbufRd_SetChromaBufferAddr(&FrameBufRd,
					       (VidBuff[rd].ChromaBaseAddr + offset));
#if (VRR_MODE == 2) // Auto Stretch
		XVFrmbufRd_Start(&FrameBufRd);
#endif
	}
	FrWrDoneCounter++;
}

#endif
#endif

u8 Exdes_LookupVic(XVidC_VideoMode VideoMode)
{
	XHdmiC_VicTable const *Entry;
	u8 Index;

	for (Index = 0; Index < sizeof(VicTable)/sizeof(XHdmiC_VicTable);
		Index++) {
	  Entry = &VicTable[Index];
	  if (Entry->VmId == VideoMode)
		return (Entry->Vic);
	}
	return 0;
}

#if defined(XPAR_XV_HDMITXSS1_NUM_INSTANCES)
/*****************************************************************************/
/**
*
* This function sets the colorbar output of the TX with new stream values.
*
* @param  VideoMode is the video mode value for the new stream.
* @param  ColorFormat is the color format value for the new stream.
* @param  Bpc is the bit per color value for the new stream.
*
* @return None.
*
* @note   This API can alternatively be implemented as a generic API in the
*         HDMI 2.1 TX interrupt controlling layer.
*         An example of such an API maybe,
*         XV_Tx_UpdateVidStream(XV_Tx *InstancePtr, VideoMode, Colorformat, Bpc)
*
******************************************************************************/
void Exdes_ChangeColorbarOutput(XVidC_VideoMode   VideoMode,
				XVidC_ColorFormat ColorFormat,
				XVidC_ColorDepth  Bpc)
{

	XVidC_VideoStream *HdmiTxSsVidStreamPtr;
	const XVidC_VideoTimingMode *VidStreamPtr;
	u8 Vic;

	HdmiTxSsVidStreamPtr = XV_HdmiTxSs1_GetVideoStream(&HdmiTxSs);
	VidStreamPtr = XVidC_GetVideoModeData(VideoMode);

	/* Set the new parameters in the video stream. */
	HdmiTxSsVidStreamPtr->Timing = VidStreamPtr->Timing;
	HdmiTxSsVidStreamPtr->FrameRate = VidStreamPtr->FrameRate;
	HdmiTxSsVidStreamPtr->VmId = VideoMode;
	HdmiTxSsVidStreamPtr->ColorFormatId = ColorFormat;
	HdmiTxSsVidStreamPtr->ColorDepth = Bpc;

	Vic = Exdes_LookupVic(VideoMode);
	XV_Tx_SetVic(&xhdmi_example_tx_controller, Vic);

	/* Check if Tx and Rx are set to run independently.
	 * If so then disable writing to the aux fifo in Rx Aux,
	 * reset the AUX fifo, reset the AVIInfoframe and
	 * reset the VSInfoframe.
	 */
	if (xhdmi_exdes_ctrlr.ForceIndependent == TRUE) {
		XHdmiC_AVI_InfoFrame *AviInfoFramePtr =
				XV_HdmiTxSs1_GetAviInfoframe(&HdmiTxSs);
		XHdmiC_VSIF *VSIFPtr = XV_HdmiTxSs1_GetVSIF(&HdmiTxSs);

		/* Reset Avi InfoFrame */
		(void)memset((void *)AviInfoFramePtr, 0,
			     sizeof(XHdmiC_AVI_InfoFrame));
		/* Reset Vendor Specific InfoFrame */
		(void)memset((void *)VSIFPtr, 0, sizeof(XHdmiC_VSIF));

		/* Update AVI InfoFrame */
		AviInfoFramePtr->ColorSpace =
				XV_HdmiC_XVidC_To_IfColorformat(ColorFormat);
		AviInfoFramePtr->VIC = HdmiTxSs.HdmiTx1Ptr->Stream.Vic;

		if ((AviInfoFramePtr->VIC > 127) || (AviInfoFramePtr->ColorSpace > 3)) {
			AviInfoFramePtr->Version = 3;
		} else {
			AviInfoFramePtr->Version = 2;
		}

	}
}
#endif
#if defined (XPAR_XV_HDMITXSS1_NUM_INSTANCES) && \
                 defined (XPAR_XV_HDMIRXSS1_NUM_INSTANCES)
/*****************************************************************************/
/**
*
* This function sets the colorbar output of the TX with new stream values.
*
* @param  XV_HdmiRxSs1 is the instance of the HDMI 2.1 RX controller.
* @param  XV_HdmiTxSs1 is the instance of the HDMI 2.1 TX controller.
*
* @return None.
*
* @note   This API can alternatively be implemented as a generic API in the
*         HDMI 2.1 TX interrupt controlling layer.
*         An example of such an API maybe,
*         XV_Tx_SetVidStream(XV_Tx *InstancePtr,
*                            XVidC_VideoStream *VidStreamPtr)
*
******************************************************************************/
void Exdes_CopyRxVidParamstoTx(XV_HdmiRxSs1 *HdmiRxSs1Ptr,
		XV_HdmiTxSs1 *HdmiTxSs1Ptr)
{

	XVidC_VideoStream *HdmiRxSsVidStreamPtr;
	XVidC_VideoStream *HdmiTxSsVidStreamPtr;

	HdmiRxSsVidStreamPtr = XV_HdmiRxSs1_GetVideoStream(HdmiRxSs1Ptr);
	HdmiTxSsVidStreamPtr = XV_HdmiTxSs1_GetVideoStream(HdmiTxSs1Ptr);

	/* Copy video parameters */
	*HdmiTxSsVidStreamPtr = *HdmiRxSsVidStreamPtr;

	/* Set the Base Video Timing parameters in the video stream. */
	HdmiTxSsVidStreamPtr->Timing = HdmiRxSsVidStreamPtr->BaseTiming;
	HdmiTxSsVidStreamPtr->FrameRate = HdmiRxSsVidStreamPtr->BaseFrameRate;

}


/*****************************************************************************/
/**
*
* This function updates the TX with new stream values. When RX is in FRL mode
* and TX is in TMDS Mode. TMDS is fixed at 4K@30 so that a portion of
* RX video can be displayed.
*
* @param  XV_HdmiRxSs1 is the instance of the HDMI 2.1 RX controller.
* @param  XV_HdmiTxSs1 is the instance of the HDMI 2.1 TX controller.
*
* @return None.
*
* @note   This API can alternatively be implemented as a generic API in the
*         HDMI 2.1 TX interrupt controlling layer.
*         An example of such an API maybe,
*         XV_Tx_SetVidStream(XV_Tx *InstancePtr,
*                            XVidC_VideoStream *VidStreamPtr)
*
******************************************************************************/


void Exdes_UpdateVidParamstoTx(XV_HdmiRxSs1 *HdmiRxSs1Ptr,
		XV_HdmiTxSs1 *HdmiTxSs1Ptr)
{

	XVidC_VideoStream *HdmiRxSsVidStreamPtr;
	XVidC_VideoStream *HdmiTxSsVidStreamPtr;
	const XVidC_VideoTimingMode *VidStreamPtr;


	HdmiRxSsVidStreamPtr = XV_HdmiRxSs1_GetVideoStream(HdmiRxSs1Ptr);
	HdmiTxSsVidStreamPtr = XV_HdmiTxSs1_GetVideoStream(HdmiTxSs1Ptr);

	/* Copy video parameters */
	*HdmiTxSsVidStreamPtr = *HdmiRxSsVidStreamPtr;
	HdmiTxSsVidStreamPtr->VmId = XVIDC_VM_3840x2160_30_P;

	VidStreamPtr = XVidC_GetVideoModeData(HdmiTxSsVidStreamPtr->VmId);

	/* Set the new parameters in the video stream. */
	HdmiTxSsVidStreamPtr->Timing = VidStreamPtr->Timing;
	HdmiTxSsVidStreamPtr->FrameRate = VidStreamPtr->FrameRate;

}

#endif
#if defined(XPAR_XV_HDMITXSS1_NUM_INSTANCES)
/*****************************************************************************/
/**
*
* This function updates the Gaming Feature Settings for the TX stream.
*
* @param  XV_HdmiTxSsVidStreamPtr is the video stream of the
*         HDMI 2.1 TX controller.
* @param VrrEnable is VRR Mode Enabled or not.
* @param FvaFactor is Fva Factor Value.
* @param CnmVrrEnable is CNMVRR is Enabled or not.
*
* @return None.
*
* @note
*
******************************************************************************/
void Exdes_UpdateGamingFeatures(XV_HdmiTxSs1 *HdmiTxSs1Ptr,
				u8 VrrEnable,u8 FvaFactor,u8 CnmVrrEnable)
{
#if (VRR_MODE == 0) // NO VRR
	XV_HdmiTxSS1_SetVrrMode(HdmiTxSs1Ptr,
				FALSE,FALSE,
				FvaFactor,CnmVrrEnable);
#elif (VRR_MODE == 1) // MANUAL STRETCH
	XV_HdmiTxSS1_SetVrrMode(HdmiTxSs1Ptr,
				FALSE,VrrEnable,
				FvaFactor,CnmVrrEnable);
#else
	XV_HdmiTxSS1_SetVrrMode(HdmiTxSs1Ptr,
				TRUE,VrrEnable,
				FvaFactor,CnmVrrEnable);
#endif
}

/*****************************************************************************/
/**
*
* This function updates the AVI Info frame for the TX stream.
*
* @param  XV_HdmiTxSsVidStreamPtr is the video stream of the
*         HDMI 2.1 TX controller.
*
* @return None.
*
* @note
*
******************************************************************************/
void Exdes_UpdateAviInfoFrame(XVidC_VideoStream *HdmiTxSsVidStreamPtr)
{
	XHdmiC_AVI_InfoFrame *AviInfoFramePtr;
	XVidC_ColorFormat Colorformat;

	/* Update AVI InfoFrame */
	AviInfoFramePtr = XV_HdmiTxSs1_GetAviInfoframe(&HdmiTxSs);
	Colorformat = HdmiTxSs.HdmiTx1Ptr->Stream.Video.ColorFormatId;

	AviInfoFramePtr->ColorSpace = XV_HdmiC_XVidC_To_IfColorformat(Colorformat);
	AviInfoFramePtr->VIC = HdmiTxSs.HdmiTx1Ptr->Stream.Vic;

	if ((AviInfoFramePtr->VIC > 127) || (AviInfoFramePtr->ColorSpace > 3)) {
		AviInfoFramePtr->Version = 3;
	} else {
		AviInfoFramePtr->Version = 2;
	}

	if ((HdmiTxSsVidStreamPtr->VmId == XVIDC_VM_1440x480_60_I) ||
	    (HdmiTxSsVidStreamPtr->VmId == XVIDC_VM_1440x576_50_I)) {
		AviInfoFramePtr->PixelRepetition =
				XHDMIC_PIXEL_REPETITION_FACTOR_2;
	} else {
		AviInfoFramePtr->PixelRepetition =
				XHDMIC_PIXEL_REPETITION_FACTOR_1;
	}
}
#endif
#if defined (XPAR_XV_HDMITXSS1_NUM_INSTANCES) && \
                 defined (XPAR_XV_HDMIRXSS1_NUM_INSTANCES)
/*****************************************************************************/
/**
*
* This function copies the RX AVI Info frame to TX.
*
* @param  None.
*
* @return None.
*
* @note
*
******************************************************************************/
void Exdes_CopyRxAVIInfoFrameToTx()
{
	XHdmiC_AVI_InfoFrame *AVIInfoFramePtr;

	AVIInfoFramePtr = XV_HdmiRxSs1_GetAviInfoframe(&HdmiRxSs);
	(void)memset((void *)&(HdmiTxSs.AVIInfoframe),
		     0x00, sizeof(XHdmiC_AVI_InfoFrame));
	memcpy(&(HdmiTxSs.AVIInfoframe), AVIInfoFramePtr,
	       sizeof(XHdmiC_AVI_InfoFrame));
}
#endif
#if defined(XPAR_XV_HDMITXSS1_NUM_INSTANCES)
/*****************************************************************************/
/**
*
* This function check the capabilities of the sink connected downstream to
* the HDMI 2.1 TX.
*
* @return XST_SUCCESS if the capabilities are successfully read.
*         XST_FAILURE otherwise.
*
* @note
*
******************************************************************************/
u32 Exdes_CheckDwnstrmSinkCaps()
{
	u32 Status = XST_SUCCESS;

	XVidC_VideoStream *HdmiTxSsVidStreamPtr;
	XV_HdmiTxSs1 *HdmiTxSs1Ptr = (XV_HdmiTxSs1 *)&HdmiTxSs;
#ifdef XPAR_XV_TPG_NUM_INSTANCES
	/* Reset the TPG */
	ResetTpg();
#endif
#ifdef XPAR_XV_AXI4S_REMAP_NUM_INSTANCES
	ResetInRemap();
	ResetOutRemap();
#endif

	/* Initialize EDID App during cable connect */
	EDIDConnectInit(&EdidHdmi_t);

	/* Read the EDID and the SCDC */
	EdidScdcCheck(HdmiTxSs1Ptr, &EdidHdmi_t);

	/* Obtain the stream information:
	 * Notes: XV_HdmiTxSs1_GetVideoStream are with updated
	 * value, either colorbar or pass-through.
	 */
	HdmiTxSsVidStreamPtr = XV_HdmiTxSs1_GetVideoStream(HdmiTxSs1Ptr);

	/* Check whether the sink is DVI/HDMI Supported */
	if (EdidHdmi_t.EdidCtrlParam.IsHdmi == XVIDC_ISDVI) {
		if (HdmiTxSsVidStreamPtr->ColorDepth != XVIDC_BPC_8 ||
		    HdmiTxSsVidStreamPtr->ColorFormatId != XVIDC_CSF_RGB) {
			xil_printf(ANSI_COLOR_YELLOW"Un-able to set TX stream,"
					" sink is DVI\r\n"
					ANSI_COLOR_RESET"\r\n");
			/* Don't set TX, if the Sink is DVI, but the source
			 * properties are:
			 *      - Color Depth more than 8 BPC
			 *      - Color Space not RGB
			 */
			return Status;
		} else {
			xil_printf(ANSI_COLOR_YELLOW"Set TX stream to DVI,"
					" sink is DVI\r\n"ANSI_COLOR_RESET"\r\n");
			XV_Tx_SetDviMode(&xhdmi_example_tx_controller);
		}
	} else {
		if (EdidHdmi_t.EdidCtrlParam.MaxFrlRateSupp !=
		    XVIDC_MAXFRLRATE_NOT_SUPPORTED) {
			xil_printf(ANSI_COLOR_YELLOW"Set TX stream to HDMI FRL,"
					" sink is HDMI\r\n"
					ANSI_COLOR_RESET"\r\n");
			XV_Tx_SetHdmiFrlMode(&xhdmi_example_tx_controller);
		} else {
			xil_printf(ANSI_COLOR_YELLOW"Set TX stream to HDMI TMDS,"
					" sink is HDMI\r\n"
					ANSI_COLOR_RESET"\r\n");
			XV_Tx_SetHdmiTmdsMode(&xhdmi_example_tx_controller);
		}
	}

	return Status;
}

/*****************************************************************************/
/**
*
* This function configures and enables the TPG input.
*
* @param  EnableExtSrcInput is a truth value that is used to decide if the
*         TPG should be configured in colorbar or pass-through.
*
* @return None.
*
* @note
*
******************************************************************************/
void Exdes_ConfigureTpgEnableInput(u32 EnableExtSrcInput)
{
#ifdef XPAR_XV_TPG_NUM_INSTANCES
	XVidC_VideoStream *HdmiTxSsVidStreamPtr;
#if defined(XPAR_XV_HDMIRXSS1_NUM_INSTANCES)
	XHdmiC_AVI_InfoFrame  *AVIInfoFramePtr;
#endif

	HdmiTxSsVidStreamPtr = XV_HdmiTxSs1_GetVideoStream(&HdmiTxSs);
#if defined(XPAR_XV_HDMIRXSS1_NUM_INSTANCES)
	AVIInfoFramePtr = XV_HdmiRxSs1_GetAviInfoframe(&HdmiRxSs);
#endif

	XV_tpg *pTpg = &Tpg;
	u32 width, height;
	XVidC_VideoMode VideoMode;
	VideoMode = HdmiTxSsVidStreamPtr->VmId;
	ResetTpg();
#endif
#ifdef XPAR_XV_AXI4S_REMAP_NUM_INSTANCES
	ResetInRemap();
	ResetOutRemap();
#endif
#ifdef XPAR_XV_TPG_NUM_INSTANCES
#if defined(XPAR_XV_HDMIRXSS1_NUM_INSTANCES)
	if (EnableExtSrcInput) {
		/* If the incoming (HDMI RX) info frame (pixel repetition) = 2
		 * The 480i/576i HActive need to be divided by 2
		 */
		if (AVIInfoFramePtr->PixelRepetition ==
		    XHDMIC_PIXEL_REPETITION_FACTOR_2) {
			width  = HdmiTxSsVidStreamPtr->Timing.HActive/2;
			height = HdmiTxSsVidStreamPtr->Timing.VActive;
		} else {
			/* If Pixel Repetition != 2, the HActive, and VActive
			 * remain as it is */
			width  = HdmiTxSsVidStreamPtr->Timing.HActive;
			height = HdmiTxSsVidStreamPtr->Timing.VActive;
		}
	} else {
#endif
		/* If Color bar, the 480i/576i HActive need to be divided by 2 */
		/* 1440x480i/1440x576i --> 720x480i/720x576i */
		/* NTSC/PAL Support */
		if ((VideoMode == XVIDC_VM_1440x480_60_I) ||
		    (VideoMode == XVIDC_VM_1440x576_50_I)) {
			width  = HdmiTxSsVidStreamPtr->Timing.HActive/2;
			height = HdmiTxSsVidStreamPtr->Timing.VActive;
		} else {
			/* If not NTSC/PAL, the HActive,
			 * and VActive remain as it is
			 */
			width  = HdmiTxSsVidStreamPtr->Timing.HActive;
			height = HdmiTxSsVidStreamPtr->Timing.VActive;
		}
#if defined(XPAR_XV_HDMIRXSS1_NUM_INSTANCES)
	}
#endif
	/* Work around:
	 * Can't set TPG to pass-through mode if the width or height = 0
	 */
	if (!(width == 0 || height == 0)) {
		/* Stop TPG */
		XV_tpg_DisableAutoRestart(pTpg);

		XV_tpg_Set_height(pTpg, height);
		XV_tpg_Set_width(pTpg,  width);
		XV_tpg_Set_colorFormat(pTpg,
				       HdmiTxSsVidStreamPtr->ColorFormatId);
		XV_tpg_Set_bckgndId(pTpg, Pattern);
		XV_tpg_Set_ovrlayId(pTpg, 0);
#if defined(XPAR_XV_HDMIRXSS1_NUM_INSTANCES)
		if (EnableExtSrcInput) {
			xil_printf("TPG Input enabled ... \r\n");
			XV_tpg_Set_enableInput(pTpg, TRUE);

			XV_tpg_Set_passthruStartX(pTpg,0);
			XV_tpg_Set_passthruStartY(pTpg,0);
			XV_tpg_Set_passthruEndX(pTpg,width);
			XV_tpg_Set_passthruEndY(pTpg,height);

		} else {
#endif
			XV_tpg_Set_enableInput(pTpg, FALSE);
#if defined(XPAR_XV_HDMIRXSS1_NUM_INSTANCES)
		}
#endif
		/* Start TPG */
		EXDES_DBG_PRINT("%s, Starting Tpg ... width : %d, "
				"height = %d. \r\n", __func__, width, height);
		XV_tpg_EnableAutoRestart(pTpg);
		XV_tpg_Start(pTpg);
	} else {
		/* If the width = 0 and height = 0 don't proceed configuring
		 * other HLS core.
		 */
		return;
	}
#endif

#ifdef XPAR_XV_AXI4S_REMAP_NUM_INSTANCES
#if !XPAR_XV_HDMITXSS1_0_INCLUDE_YUV420_SUP
	/* Configuring the InRemap which performs
	 *    PPC Conversion from PPC4 --> PPC8 for InRemap
	 *.   PPC Conversion from PPC8 --> PPC4 for OutRemap
	 *    YUV AXI4-Stream Packet on InRemap --> TPG --> OutRemap (8 PPC)
	 * Below configuring following Video HLS IP (TPG Flow)
	 */
	XV_axi4s_remap_Set_height(&InRemap,height);
	XV_axi4s_remap_Set_width(&InRemap,width);

	XV_axi4s_remap_Set_inPixClk(&InRemap,
			XPAR_XV_HDMIRXSS1_0_INPUT_PIXELS_PER_CLOCK);
	XV_axi4s_remap_Set_outPixClk(&InRemap,
			XPAR_XV_AXI4S_REMAP_0_OUT_SAMPLES_PER_CLOCK);

	XV_axi4s_remap_Set_height(&OutRemap,height);
	XV_axi4s_remap_Set_width(&OutRemap,width);

	XV_axi4s_remap_Set_inPixClk(&OutRemap,
			XPAR_XV_AXI4S_REMAP_1_IN_SAMPLES_PER_CLOCK);
	XV_axi4s_remap_Set_outPixClk(&OutRemap,
			XPAR_XV_HDMITXSS1_0_INPUT_PIXELS_PER_CLOCK);

	if (EnableExtSrcInput) {
		XV_axi4s_remap_Set_ColorFormat(&InRemap,
				HdmiTxSsVidStreamPtr->ColorFormatId);
		XV_axi4s_remap_Set_ColorFormat(&OutRemap,
				HdmiTxSsVidStreamPtr->ColorFormatId);

		if (HdmiTxSsVidStreamPtr->ColorFormatId==XVIDC_CSF_YCRCB_420) {
			XV_axi4s_remap_Set_inHDMI420(&OutRemap, 0);
			XV_axi4s_remap_Set_outHDMI420(&OutRemap, 1);

			XV_axi4s_remap_Set_inHDMI420(&InRemap, 1);
			XV_axi4s_remap_Set_outHDMI420(&InRemap, 0);
		} else {
			XV_axi4s_remap_Set_inHDMI420(&OutRemap, 0);
			XV_axi4s_remap_Set_outHDMI420(&OutRemap, 0);

			XV_axi4s_remap_Set_inHDMI420(&InRemap, 0);
			XV_axi4s_remap_Set_outHDMI420(&InRemap, 0);
		}

	} else {
		if (HdmiTxSsVidStreamPtr->ColorFormatId==XVIDC_CSF_YCRCB_420) {
			XV_axi4s_remap_Set_ColorFormat(&InRemap,
                                    XVIDC_CSF_YCRCB_420);
			XV_axi4s_remap_Set_ColorFormat(&OutRemap,
                                    XVIDC_CSF_YCRCB_420);

			XV_axi4s_remap_Set_inHDMI420(&OutRemap, 0);
			XV_axi4s_remap_Set_outHDMI420(&OutRemap, 1);
		} else {
			XV_axi4s_remap_Set_ColorFormat(&InRemap,
					HdmiTxSsVidStreamPtr->ColorFormatId);
			XV_axi4s_remap_Set_ColorFormat(&OutRemap,
					HdmiTxSsVidStreamPtr->ColorFormatId);

			XV_axi4s_remap_Set_inHDMI420(&OutRemap, 0);
			XV_axi4s_remap_Set_outHDMI420(&OutRemap, 0);
		}
	}
	XV_axi4s_remap_EnableAutoRestart(&InRemap);
	XV_axi4s_remap_EnableAutoRestart(&OutRemap);

	XV_axi4s_remap_Start(&InRemap);
	XV_axi4s_remap_Start(&OutRemap);
#endif
#endif
}
#endif
#if defined (XPAR_XV_HDMITXSS1_NUM_INSTANCES) && \
                 defined (XPAR_XV_HDMIRXSS1_NUM_INSTANCES)
/*****************************************************************************/
/**
*
* This function configures the ACR conversion modules based on incoming
* ACR CTS/N Value in Pass-through Mode
*
* @param  None.
*
* @return None.
*
* @note
*
******************************************************************************/
void Exdes_AcrConvCfg_Passthru()
{
#ifdef USE_HDMI_AUDGEN
	XHdmiC_SamplingFrequencyVal AudSampFreq;
	u8 TxTransportIsFrl;
	u8 RxTransportIsFrl;
	u8 TxRate = 0;
	u8 RxRate = 0;

	RxTransportIsFrl = HdmiRxSs.HdmiRx1Ptr->Stream.IsFrl;
	TxTransportIsFrl = HdmiTxSs.HdmiTx1Ptr->Stream.IsFrl;

	/* RX Mode */
	if (RxTransportIsFrl == TRUE) {
		XhdmiACRCtrl_RxMode(&AudioGen, ACR_FRL_MODE);
		AudSampFreq =
			XV_Rx_GetFrlAudSampFreq(&xhdmi_example_rx_controller);
		RxRate = HdmiRxSs.HdmiRx1Ptr->Stream.Frl.LineRate;
	} else {
		XhdmiACRCtrl_RxMode(&AudioGen, ACR_TMDS_MODE);
		AudSampFreq =
			XV_Rx_GetTmdsAudSampFreq(&xhdmi_example_rx_controller);
	}

	/* TX Mode */
	if (TxTransportIsFrl == TRUE) {
		XhdmiACRCtrl_TxMode(&AudioGen, ACR_FRL_MODE);
		TxRate = HdmiTxSs.HdmiTx1Ptr->Stream.Frl.LineRate;
	} else {
		if (XV_HdmiTxSs1_GetTmdsClockFreqHz(&HdmiTxSs) > 340000000) {
			XhdmiACRCtrl_TMDSClkRatio(&AudioGen, TRUE);
		} else {
			XhdmiACRCtrl_TMDSClkRatio(&AudioGen, FALSE);
		}
		XhdmiACRCtrl_TxMode(&AudioGen, ACR_TMDS_MODE);
	}

	if (TxTransportIsFrl == TRUE) {
		if (TxRate == RxRate) {
			/* Disable CTS Conversion */
			XhdmiACRCtrl_EnableCtsConv(&AudioGen, FALSE);
			XhdmiACRCtrl_SetNVal(&AudioGen,
					0);
		} else {
			/* Enable CTS Conversion */
			if(HdmiRxSs.TMDSClockRatio) {
				XhdmiACRCtrl_TMDSClkRatio(&AudioGen, TRUE);
			} else {
				XhdmiACRCtrl_TMDSClkRatio(&AudioGen, FALSE);
			}
			XhdmiACRCtrl_EnableCtsConv(&AudioGen, TRUE);
			XhdmiACRCtrl_SetNVal(&AudioGen,
				XV_Tx_GetNVal(&xhdmi_example_tx_controller,
				AudSampFreq));
		}
	} else {
		if (TxTransportIsFrl == RxTransportIsFrl) {
			/* Disable CTS Conversion */
			XhdmiACRCtrl_EnableCtsConv(&AudioGen, FALSE);
			XhdmiACRCtrl_SetNVal(&AudioGen,
					0);
		} else {
			/* Enable CTS Conversion */
			XhdmiACRCtrl_EnableCtsConv(&AudioGen, TRUE);
			XhdmiACRCtrl_SetNVal(&AudioGen,
				XV_Tx_GetNVal(&xhdmi_example_tx_controller,
				AudSampFreq));
		}
	}
#endif
}

/*****************************************************************************/
/**
*
* This function configures the audio for the example design when it is in
* pass-through mode.
*
* @param  None.
*
* @return None.
*
* @note
*
******************************************************************************/
void Exdes_AudioConfig_Passthru()
{
	XV_HdmiTx1_AudioFormatType AudioFormat;

	/* Get the audio format from Hdmi Rx */
	/* HBR audio */
	if (XV_HdmiRxSs1_GetAudioFormat(&HdmiRxSs) == XV_HDMIRX1_AUDFMT_HBR) {
		AudioFormat = XV_HDMITX1_AUDFMT_HBR;
	} else {
		AudioFormat = XV_HDMITX1_AUDFMT_LPCM;
	}

	/* Set tha audio format and channels. */
	XV_Tx_SetAudioFormatAndChannels(&xhdmi_example_tx_controller,
				AudioFormat,
				XV_HdmiRxSs1_GetAudioChannels(&HdmiRxSs));
#ifdef USE_HDMI_AUDGEN
	/* Disable audio generator */
	XhdmiAudGen_Start(&AudioGen, FALSE);

	/* Select ACR from RX */
	XhdmiACRCtrl_Sel(&AudioGen, ACR_SEL_IN);
#endif
	/* Configure the ACR Conversion Module */
	Exdes_AcrConvCfg_Passthru();

	/* Set to use the External ACR/HDMI RX ACR out module*/
	XV_Tx_SetUseExternalACR(&xhdmi_example_tx_controller);
#ifdef USE_HDMI_AUDGEN
	/* Re-program audio clock */
	XhdmiAudGen_SetAudClk(&AudioGen, XAUD_SRATE_192K);
#endif
}
#endif
#if defined(XPAR_XV_HDMITXSS1_NUM_INSTANCES)
/*****************************************************************************/
/**
*
* This function configures the audio for the example design when it is in
* colorbar mode.
*
* @param  None.
*
* @return None.
*
* @note
*
******************************************************************************/
void Exdes_AudioConfig_Colorbar()
{
	XHdmiC_AudioInfoFrame *AudioInfoFramePtr;

	AudioInfoFramePtr = XV_HdmiTxSs1_GetAudioInfoframe(&HdmiTxSs);

	/* Reset Audio InfoFrame */
	(void)memset((void *)AudioInfoFramePtr, 0, sizeof(XHdmiC_AudioInfoFrame));

	/* Set audio format and number of audio channels */
	XV_Tx_SetAudioFormatAndChannels(&xhdmi_example_tx_controller,
					XV_HDMITX1_AUDFMT_LPCM, 2);
#ifdef USE_HDMI_AUDGEN
	/* Enable audio generator */
	XhdmiAudGen_Start(&AudioGen, TRUE);

	/* Select ACR from ACR Ctrl */
	XhdmiACRCtrl_Sel(&AudioGen, ACR_SEL_GEN);

	/* Enable 2-channel audio */
	XhdmiAudGen_SetEnabChannels(&AudioGen, 2);
	XhdmiAudGen_SetPattern(&AudioGen, 1, XAUD_PAT_PING);
	XhdmiAudGen_SetPattern(&AudioGen, 2, XAUD_PAT_PING);
	XhdmiAudGen_SetSampleRate(&AudioGen,
			   XV_HdmiTxSs1_GetTmdsClockFreqHz(&HdmiTxSs),
			   XAUD_SRATE_48K);
#endif
	/* Set to use the Internal ACR module of the HDMI TX */
	XV_Tx_SetUseInternalACR(&xhdmi_example_tx_controller);
	/* Set the Audio Sampling Frequency to 48kHz */
	XV_Tx_SetAudSamplingFreq(&xhdmi_example_tx_controller,
			XHDMIC_SAMPLING_FREQ_48K);

	/* Refer to CEA-861-D for Audio InfoFrame Channel Allocation
	 * - - - - - - FR FL
	 */
	AudioInfoFramePtr->ChannelAllocation = 0x0;
	/* Refer to Stream Header */
	AudioInfoFramePtr->SampleFrequency = 0x0;
}
#endif
/*****************************************************************************/
/**
*
* This function updates the global array for AUX fifo with the AUX data from
* the hdmi receiver.
*
* @param  None.
*
* @return None.
*
* @note
*
******************************************************************************/
void Exdes_UpdateAuxFifo()
{
#if defined (XPAR_XV_HDMITXSS1_NUM_INSTANCES) && \
                 defined (XPAR_XV_HDMIRXSS1_NUM_INSTANCES)
	XHdmiC_Aux *AuxPtr;
	AuxPtr = XV_HdmiRxSs1_GetAuxiliary(&HdmiRxSs);

	/* In pass-through mode copy some aux packets into local buffer.
	 * GCP does not need to be sent out because GCP packets on the TX side
	 * is handled by the HDMI TX core fully. Starts storing Aux only
	 * when TX stream has started to prevent AuxFifo Overflow.
	 */
#if defined (VTEM2FSYNC)
	if ( (AuxPtr->Header.Byte[0] != AUX_GENERAL_CONTROL_PACKET_TYPE) &&
	     (AuxPtr->Header.Byte[0] != 0x83) &&
	     AuxFifoStartFlag == TRUE) {
#else
	if (AuxPtr->Header.Byte[0] != AUX_GENERAL_CONTROL_PACKET_TYPE &&
	    AuxFifoStartFlag == TRUE) {
#endif
		EXDES_AUXFIFO_DBG_PRINT("AuxPtr %d Type - 0x%x\r\n",
					AuxFifoEndIndex,
					AuxPtr->Header.Byte[0]);

		memcpy(&(AuxFifo[AuxFifoEndIndex]), AuxPtr, sizeof(XHdmiC_Aux));
		if (AuxFifoEndIndex < (AUXFIFOSIZE - 1)) {
			AuxFifoEndIndex++;
		} else {
			AuxFifoEndIndex = 0;
		}

		if (AuxFifoCount >= AUXFIFOSIZE) {
			AuxFifoOvrFlowCnt++;
		 }
		 AuxFifoCount++;
	}
#endif
}

/*****************************************************************************/
/**
*
* This function updates the  Reads the VRR Video Timing Values from HDMI RX
* Receiver
*
* @param  None.
*
* @return None.
*
* @note
*
******************************************************************************/
void Exdes_ReadVRRTimingChange()
{
#if defined  (XPAR_XV_HDMIRXSS1_NUM_INSTANCES)

#endif
}

/*****************************************************************************/
/**
*
* This function updates the  Reads the VRR Video Timing Values from HDMI RX
* Receiver and updates to HDMI TX when VRR Manual Mode is enabled
*
* @param  None.
*
* @return None.
*
* @note
*
******************************************************************************/
void Exdes_ProcessVRRTimingChange()
{
#if defined (XPAR_XV_HDMITXSS1_NUM_INSTANCES) && \
	defined (XPAR_XV_HDMIRXSS1_NUM_INSTANCES)
	XVidC_VideoStream *HdmiRxSsVidStreamPtr;
	u16 vfp_diff = 0;

	HdmiRxSsVidStreamPtr = XV_HdmiRxSs1_GetVideoStream(&HdmiRxSs);
	vfp_diff = HdmiRxSsVidStreamPtr->Timing.F0PVFrontPorch ;

#if defined (XPAR_XVTC_NUM_INSTANCES)
#if (VRR_MODE == 1) // Manual Stretch
	XV_HdmiTxSS1_SetVrrVfpStretch(&HdmiTxSs,vfp_diff);
#endif
#endif

#endif
}

/****************************************
 *
* This function converts the VTEM Packet to SPDIF Packet format

* @param  None.
*
* @return None.
*
* @note
*
 ****************************************/

void Exdes_VTEMPToSPDIFConversion()
{
#if defined (XPAR_XV_HDMITXSS1_NUM_INSTANCES) && \
	defined (XPAR_XV_HDMIRXSS1_NUM_INSTANCES)

#if defined (VTEM2FSYNC)

	XVidC_VideoStream *HdmiRxSsVidStreamPtr_l;

	HdmiRxSsVidStreamPtr_l = XV_HdmiRxSs1_GetVideoStream(&HdmiRxSs);
	BaseFrameRate_VRR = HdmiRxSsVidStreamPtr_l->BaseFrameRate ;
	if(VrrInforFrame.VrrIfType == XV_HDMIC_VRRINFO_TYPE_VTEM) {

		if (VrrInforFrame.VidTimingExtMeta.VRREnabled == 0x1) {
			VrrInforFrame.SrcProdDescIF.FreeSync.FreeSyncActive     = 0x1;
		}
		else {

			VrrInforFrame.SrcProdDescIF.FreeSync.FreeSyncActive     = 0x0;
		}
	VrrInforFrame.SrcProdDescIF.FreeSync.Version                    = 0x2;
	VrrInforFrame.SrcProdDescIF.FreeSync.FreeSyncSupported          = 0x1;
	VrrInforFrame.SrcProdDescIF.FreeSync.FreeSyncEnabled            = 0x1;
		//Setting Minimum refresh rate to 40Hz
		VrrInforFrame.SrcProdDescIF.FreeSync.FreeSyncMinRefreshRate     = 0x28;
		VrrInforFrame.SrcProdDescIF.FreeSync.FreeSyncMaxRefreshRate     = BaseFrameRate_VRR;
		VrrInforFrame.SrcProdDescIF.FreeSyncPro.NativeColorSpaceActive  = 0;
		VrrInforFrame.SrcProdDescIF.FreeSyncPro.BrightnessControlActive = 0;
		VrrInforFrame.SrcProdDescIF.FreeSyncPro.LocalDimControlActive   = 0;
		VrrInforFrame.SrcProdDescIF.FreeSyncPro.sRGBEOTFActive          = 0;
		VrrInforFrame.SrcProdDescIF.FreeSyncPro.BT709EOTFActive         = 0;
		VrrInforFrame.SrcProdDescIF.FreeSyncPro.Gamma22EOTFActive       = 0;
		VrrInforFrame.SrcProdDescIF.FreeSyncPro.Gamma26EOTFActive       = 0;
		VrrInforFrame.SrcProdDescIF.FreeSyncPro.PQEOTFActive            = 0;
		VrrInforFrame.SrcProdDescIF.FreeSyncPro.BrightnessControl       = 0;
		VrrInforFrame.VrrIfType                                         = XV_HDMIC_VRRINFO_TYPE_SPDIF;
	}
#endif
#endif
}
/*****************************************************************************/
/**
*
* This function updates the  Reads the VRR VTEM Packet from HDMI RX
* Receiver
*
* @param  None.
*
* @return None.
*
* @note
*
******************************************************************************/
void Exdes_ReadVTEMPacket()
{
#if defined  (XPAR_XV_HDMIRXSS1_NUM_INSTANCES)
	XV_HdmiC_VrrInfoFrame  *HdmiRxVrrInfoFramePtr;
	HdmiRxVrrInfoFramePtr = XV_HdmiRxSs1_GetVrrIf(&HdmiRxSs);
	memcpy(&VrrInforFrame,HdmiRxVrrInfoFramePtr,sizeof(XV_HdmiC_VrrInfoFrame));
#if defined (VTEM2FSYNC)
          Exdes_VTEMPToSPDIFConversion();
#endif
#endif
}

/*****************************************************************************/
/**
*
* This function updates the  R Reads the VRR VTEM Packet from HDMI RX
* Receiver and updates to HDMI TX
*
* @param  None.
*
* @return None.
*
* @note
*
******************************************************************************/
void Exdes_ProcessVTEMPacket()
{
#if defined (XPAR_XV_HDMITXSS1_NUM_INSTANCES) && \
	defined (XPAR_XV_HDMIRXSS1_NUM_INSTANCES)
	XV_HdmiC_VrrInfoFrame  *HdmiRxVrrInfoFramePtr;
	HdmiRxVrrInfoFramePtr = XV_HdmiRxSs1_GetVrrIf(&HdmiRxSs);

	memcpy(&VrrInforFrame, HdmiRxVrrInfoFramePtr,
	       sizeof(XV_HdmiC_VrrInfoFrame));

#if defined (VTEM2FSYNC)
	Exdes_VTEMPToSPDIFConversion();
#endif
	/*Copy and Enable the TX VTEM/FSYNC Packet */
	XV_HdmiTxSs1_SetVrrIf(&HdmiTxSs,&VrrInforFrame);
	/* Update FYNC/VTEM Control */
	if (VrrInforFrame.VrrIfType == XV_HDMIC_VRRINFO_TYPE_VTEM) {
		XV_HdmiTxSs1_VrrControl(&HdmiTxSs,0x1);
	}
	else if (VrrInforFrame.VrrIfType == XV_HDMIC_VRRINFO_TYPE_SPDIF) {
		XV_HdmiTxSs1_FSyncControl(&HdmiTxSs,0x1);
	}
	/* Some Source are first changing VFP and  then No VTEM to VTEM
	   To Handle this situation, When VFP interrupt is called, read and update
	   VFP */
	Exdes_ProcessVRRTimingChange();
#endif
}




#if defined (XPAR_XV_HDMITXSS1_NUM_INSTANCES) && \
                 defined (XPAR_XV_HDMIRXSS1_NUM_INSTANCES)
/*****************************************************************************/
/**
*
* This function is used to send the info frame when the example design is in
* pass-through mode.
*
* @param  None.
*
* @return None.
*
* @note
*
******************************************************************************/
void Exdes_SendInfoFrame_Passthru()
{
	XHdmiC_AVI_InfoFrame  *AVIInfoFramePtr;
	XHdmiC_AudioInfoFrame *AudioInfoFramePtr;
	XHdmiC_VSIF *VSIFPtr;

	XV_HdmiTxSs1 *HdmiTxSs1Ptr = (XV_HdmiTxSs1 *)&HdmiTxSs;
	u32 Status;
	u32 RegValue;

	/* When the TX stream is confirmed to have started, start accepting
	 * Aux from RxAuxCallback
	 */
	AuxFifoStartFlag = (TRUE);
	EXDES_AUXFIFO_DBG_PRINT("%s,%d. AuxFifoStartFlag = %d, ",
			__func__, __LINE__, AuxFifoStartFlag);
	/* For more details use,
	 * EXDES_AUXFIFO_DBG_PRINT("%s,%d. AuxFifoStartFlag = %d, "
	 *		"AuxFifoCount = %d,"
	 *		"AuxFifo[%d(%d)].Header.Byte[0] = 0x%x(0x%x)\r\n",
	 *		__func__, __LINE__,
	 *		AuxFifoStartFlag, AuxFifoCount,
	 *		AuxFifoStartIndex, (AuxFifoEndIndex -1),
	 *		AuxFifo[AuxFifoStartIndex].Header.Byte[0],
	 *		AuxFifo[AuxFifoEndIndex -1].Header.Byte[0]);
	 */

	if (AuxFifoCount > AUXFIFOSIZE) {
		AuxFifoStartIndex = AuxFifoEndIndex;
	}

	VSIFPtr = XV_HdmiTxSs1_GetVSIF(HdmiTxSs1Ptr);
	AVIInfoFramePtr = XV_HdmiTxSs1_GetAviInfoframe(HdmiTxSs1Ptr);
	AudioInfoFramePtr = XV_HdmiTxSs1_GetAudioInfoframe(HdmiTxSs1Ptr);

	/* If PassThrough, update TX's InfoFrame Data Structure
	 * from AuxFiFO
	 */
	while (AuxFifoStartIndex != AuxFifoEndIndex) {
		if (AuxFifo[AuxFifoStartIndex].Header.Byte[0] ==
		    AUX_VSIF_TYPE) {
			/* Reset Vendor Specific InfoFrame */
			(void)memset((void *)VSIFPtr, 0, sizeof(XHdmiC_VSIF));

			XV_HdmiC_VSIF_ParsePacket(&AuxFifo[AuxFifoStartIndex],
						  VSIFPtr);
		} else if (AuxFifo[AuxFifoStartIndex].Header.Byte[0] ==
		           AUX_AVI_INFOFRAME_TYPE) {
			/* Reset Avi InfoFrame */
			(void)memset((void *)AVIInfoFramePtr, 0,
				     sizeof(XHdmiC_AVI_InfoFrame));

			XV_HdmiC_ParseAVIInfoFrame(&AuxFifo[AuxFifoStartIndex],
						   AVIInfoFramePtr);

			/* Modify the TX's InfoFrame here before sending out
			 * E.g:
			 *     AviInfoFramePtr->VIC = 107;
			 */

			/* Generate Aux from the modified TX's InfoFrame before
			 * sending out
			 * E.g:
			 * 	AuxFifo[AuxFifoStartIndex] =
			 * 	XV_HdmiC_AVIIF_GeneratePacket(AviInfoFramePtr);
			 */
		} else if (AuxFifo[AuxFifoStartIndex].Header.Byte[0] ==
		           AUX_AUDIO_INFOFRAME_TYPE) {
			/* Reset Audio InfoFrame */
			(void)memset((void *)AudioInfoFramePtr, 0,
				     sizeof(XHdmiC_AudioInfoFrame));

			XV_HdmiC_ParseAudioInfoFrame(&AuxFifo[AuxFifoStartIndex],
						     AudioInfoFramePtr);

			/* Modify the TX's InfoFrame here
			 * before sending out
			 * E.g:
			 * 	AudioInfoFramePtr->ChannelCount =
			 * 		XHDMIC_AUDIO_CHANNEL_COUNT_3;
			 */

			/* Generate Aux from the modified TX's InfoFrame
			 * before sending out
			 * E.g :
			 * 	AuxFifo[AuxFifoStartIndex] =
			 * 	XV_HdmiC_AudioIF_GeneratePacket(AudioInfoFramePtr);
			 */

			/* Check for current ACR N Value with previous N Value
			 * If different Update the ACR Generation on TX, with
			 * the update incoming N Value
			 */
			if (XV_Rx_AcrNValDiffCheck(
					&xhdmi_example_rx_controller) ==
			    XST_SUCCESS) {
				Exdes_AcrConvCfg_Passthru();
			}
		}

		EXDES_AUXFIFO_DBG_PRINT("(%d)0x%x .", AuxFifoStartIndex,
				AuxFifo[AuxFifoStartIndex].Header.Byte[0]);
		/* For more details use,
		 * EXDES_AUXFIFO_DBG_PRINT("Sending Aux %d = 0x%x \r\n",
		 *		AuxFifoStartIndex,
		 *		AuxFifo[AuxFifoStartIndex].Header.Byte[0]);
		 */

		Status = XV_HdmiTxSs1_SendGenericAuxInfoframe(HdmiTxSs1Ptr,
						&(AuxFifo[AuxFifoStartIndex]));

		/* If TX Core's hardware Aux FIFO is full, from the while loop,
		 * retry during the next main while iteration.
		 */
		if (Status != (XST_SUCCESS)) {
			RegValue = XV_HdmiTx1_ReadReg(HdmiTxSs.Config.BaseAddress,
					(XV_HDMITX1_AUX_STA_OFFSET));

			xil_printf(ANSI_COLOR_RED "%s,%d. HW Aux Full"
					ANSI_COLOR_RESET "\r\n",
					__func__, __LINE__);
			/* It is possible that the user runs into a scenario,
			 * where the AUX Fifo overflows and the com port or
			 * uart log is flooded with "HW Aux Full" messages.
			 * In such a case, the user can use the following
			 * workaround here,
			 * xhdmi_exdes_ctrlr.SystemEvent = TRUE;
			 */
			xil_printf("failing Aux Status = 0x%x \r\n", RegValue);
		}

		if (AuxFifoStartIndex < (AUXFIFOSIZE - 1)) {
			AuxFifoStartIndex++;
		} else {
			AuxFifoStartIndex = 0;
		}
	}

	EXDES_AUXFIFO_DBG_PRINT("\r\n");

	AuxFifoCount = 0;
}
#endif
#if defined(XPAR_XV_HDMITXSS1_NUM_INSTANCES)
/*****************************************************************************/
/**
*
* This function is used to send the info frame when the example design is in
* colorbar mode.
*
* @param  None.
*
* @return None.
*
* @note
*
******************************************************************************/
void Exdes_SendInfoFrame_Colorbar()
{
	u32 Status = XST_SUCCESS;
	u32 RegValue = 0;

	XHdmiC_AVI_InfoFrame  *AVIInfoFramePtr;
	XHdmiC_AudioInfoFrame *AudioInfoFramePtr;

	XV_HdmiTxSs1 *HdmiTxSs1Ptr = (XV_HdmiTxSs1 *)&HdmiTxSs;

	AVIInfoFramePtr = XV_HdmiTxSs1_GetAviInfoframe(HdmiTxSs1Ptr);
	AudioInfoFramePtr = XV_HdmiTxSs1_GetAudioInfoframe(HdmiTxSs1Ptr);

	/* Generate Aux from the current TX InfoFrame */
	AuxFifo[0] = XV_HdmiC_AVIIF_GeneratePacket(AVIInfoFramePtr);
	EXDES_AUXFIFO_DBG_PRINT("%s,%d. Independent TX ? = %d ; "
			"(AVIIF_generatePacket)AuxFifo[0].Header.Byte[0] = "
			"0x%x\r\n", __func__, __LINE__,
			xhdmi_exdes_ctrlr.ForceIndependent,
			AuxFifo[0].Header.Byte[0]);

	RegValue = XV_HdmiTx1_ReadReg(HdmiTxSs.Config.BaseAddress,
			(XV_HDMITX1_AUX_STA_OFFSET));
	Status = XV_HdmiTxSs1_SendGenericAuxInfoframe(HdmiTxSs1Ptr,
						      &(AuxFifo[0]));
	/* If TX Core's hardware Aux FIFO is full, from the while loop,
	 * retry during the next main while iteration.
	 */
	if (Status != (XST_SUCCESS)) {
		EXDES_AUXFIFO_DBG_PRINT(ANSI_COLOR_RED"%s,%d. HW Aux Full "
				"(AuxStatus = 0x%x)"ANSI_COLOR_RESET"\r\n",
				__func__, __LINE__, RegValue);
		AuxHwFullCounter++;
	}

	/* GCP does not need to be sent out because GCP packets on
	 * the TX side is handled by the HDMI TX core fully.
	 */

	AuxFifo[0] = XV_HdmiC_AudioIF_GeneratePacket(AudioInfoFramePtr);
	Status = XV_HdmiTxSs1_SendGenericAuxInfoframe(HdmiTxSs1Ptr,
						      &(AuxFifo[0]));
	/* If TX Core's hardware Aux FIFO is full, from the while loop,
	 * retry during the next main while iteration.
	 */
	if (Status != (XST_SUCCESS)) {
		EXDES_AUXFIFO_DBG_PRINT(ANSI_COLOR_RED"%s,%d. HW Aux (AudioIF) "
				"Full (AuxStatus = 0x%x)"ANSI_COLOR_RESET"\r\n",
				__func__, __LINE__, RegValue);
		AuxHwFullCounter++;
	}

	/* SendVSInfoframe(HdmiTxSs1Ptr); */
}
#endif
/*****************************************************************************/
/**
*
* This function checks if there has been a change in resolution on the incoming
* rx stream.
*
* @param  None.
*
* @return None.
*
* @note
*
******************************************************************************/
u32 Exdes_CheckforResChange()
{
#if defined(XPAR_XV_HDMIRXSS1_NUM_INSTANCES)
	/* To further debug the Aux fifo behaviour, the global flag
	 * AuxFifoStartFlag can be checked here,
	 * EXDES_AUXFIFO_DBG_PRINT("%s,%d. AuxFifoStartFlag = %d "
				   "before clearing. \r\n", __func__, __LINE__,
				   AuxFifoStartFlag);
	* /\* Reset the AUX fifo.*\/
	* ResetAuxFifo();
	*/

	/* RX disconnect can come from stream off or cable unplug */
	if (HdmiRxSs.IsStreamConnected == (FALSE)) {
		/* Cable has been unplugged. */
		xhdmi_exdes_ctrlr.TxStartTransmit = TRUE;

		return FALSE;
	} else {
		/* Resolution has changed. */
		xil_printf("Rx connected but down !! "
				"(Waiting for new stream ...) \r\n");
#if defined (XPAR_XV_HDMITXSS1_NUM_INSTANCES) && \
                 defined (XPAR_XV_HDMIRXSS1_NUM_INSTANCES)
		/* Reset the AUX fifo. */
		ResetAuxFifo();
#endif

		/* Mute any RX stream.
		 * Important to do this here - without this, we will get
		 * overwhelmed with RX Bridge Overflow interrupts,
		 * while going from colorbar to pass-through. */
		EXDES_DBG_PRINT("%s,%d: Rx VRST - true \r\n", __func__, __LINE__);
		XV_HdmiRxSs1_VRST(&HdmiRxSs, TRUE);
#endif
#if defined(XPAR_XV_HDMITXSS1_NUM_INSTANCES)
#ifdef XPAR_XV_TPG_NUM_INSTANCES
		/* Disable the input to Tpg, until the stream
		 * starts with the new resolution video
		 * params again.
		 */
		XV_tpg_Set_enableInput(&Tpg, FALSE);
		/* Alternatively,
		 * Exdes_ConfigureTpgEnableInput(FALSE);
		 */
#endif
#endif
		return TRUE;
#if defined(XPAR_XV_HDMIRXSS1_NUM_INSTANCES)
	}
#endif
}
#if defined(XPAR_XV_HDMITXSS1_NUM_INSTANCES)
/*****************************************************************************/
/**
*
* This function updates the parameters of the transmitter stream based on
* the input source for the tx stream (i.e. if the input for tx stream is the
* rx stream or previously set or default colorbar).
* rx stream.
*
* @param  InstancePtr is the instance of the example design handle
*         data structure.
* @param  TxInputSrc is the input source for the tx.
*
* @return None.
*
* @note
*
******************************************************************************/
u32 Exdes_UpdateTxParams(XHdmi_Exdes *ExdesInstance,
			 TxInputSourceType TxInputSrc)
{
	u32 Status = XST_SUCCESS;
#if defined (XPAR_XV_HDMITXSS1_NUM_INSTANCES) && \
                 defined (XPAR_XV_HDMIRXSS1_NUM_INSTANCES)
	u64 LineRate;
#endif
#if (!defined XPAR_XVTC_NUM_INSTANCES) || (defined (XPAR_XV_FRMBUFWR_NUM_INSTANCES) && \
                      (XPAR_XV_FRMBUFRD_NUM_INSTANCES))

   u8 FVaFactor = FVA_FACTOR;
#endif
	XVidC_VideoStream *HdmiTxSsVidStreamPtr;

	HdmiTxSsVidStreamPtr = XV_HdmiTxSs1_GetVideoStream(&HdmiTxSs);
	XVidC_VideoMode	CurrentVmId = HdmiTxSsVidStreamPtr->VmId;

	/* There is no need to crop when TX is FRL */
	if (xhdmi_exdes_ctrlr.IsTxPresent && HdmiTxSs.HdmiTx1Ptr->Stream.IsFrl) {
		xhdmi_exdes_ctrlr.crop = FALSE;
	}

	EXDES_AUXFIFO_DBG_PRINT("%s,%d: Aux Fifo Reset \r\n",
				__func__, __LINE__);
	ResetAuxFifo();
#if defined(XPAR_XV_HDMIRXSS1_NUM_INSTANCES)
	/* Mute any RX stream.
	 * Important to do this here - without this, we will get
	 * overwhelmed with RX Bridge Overflow interrupts. */
#if defined (XPAR_XV_FRMBUFWR_NUM_INSTANCES) && \
                      (XPAR_XV_FRMBUFRD_NUM_INSTANCES)
#else
	XVidC_VideoStream *HdmiRxSsVidStreamPtr;
	HdmiRxSsVidStreamPtr = XV_HdmiRxSs1_GetVideoStream(&HdmiRxSs);
	if(HdmiRxSsVidStreamPtr->IsDSCompressed)
		XV_HdmiRxSs1_VRST(&HdmiRxSs, FALSE);
	else
		XV_HdmiRxSs1_VRST(&HdmiRxSs, TRUE);
#endif
	EXDES_DBG_PRINT("%s,%d: Rx VRST - true \r\n", __func__, __LINE__);
#endif
	switch (TxInputSrc) {
	case EXDES_TX_INPUT_NONE_WAITFORNEWSTREAM:
	case EXDES_TX_INPUT_NONE_NOCONNECTIONS:
	case EXDES_TX_INPUT_NONE_RXONLY:
		Status = XST_FAILURE;
		break;

	case EXDES_TX_INPUT_TPG:
		/* If the VmId is not set for the stream or is invalid, then
		 * set  a 1080p stream.
		 * Check if this is a bringup condition where the stream timing
		 * VActive and HActive are 0.
		 * Otherwise keep, as is, the existing / last set stream
		 * information in the transmitter.
		 */
		if ((CurrentVmId > XVIDC_VM_NUM_SUPPORTED) ||
		    (CurrentVmId == XVIDC_VM_NO_INPUT) ||
		    (CurrentVmId == XVIDC_VM_NOT_SUPPORTED) ||
		    (CurrentVmId == XVIDC_VM_CUSTOM &&
		     CurrentVmId < (XVidC_VideoMode)XVIDC_CM_NUM_SUPPORTED) ||
		    (HdmiTxSsVidStreamPtr->ColorDepth < XVIDC_BPC_6) ||
		    (HdmiTxSsVidStreamPtr->Timing.HActive == 0 ||
		     HdmiTxSsVidStreamPtr->Timing.VActive == 0)) {

			/* Check if we have selected the custom
			 * resolution from the colorbar menu. */
			if (CurrentVmId > (XVidC_VideoMode)XVIDC_VM_CUSTOM &&
			    CurrentVmId < (XVidC_VideoMode)XVIDC_CM_NUM_SUPPORTED) {
				/* Do nothing, and continue with the
				 * custom resolution stream information.
				 */
			} else {
				xil_printf(ANSI_COLOR_YELLOW "Video format is"
						" not supported. Reverting to "
						"default video format.\r\n"
						ANSI_COLOR_RESET);
				Exdes_ChangeColorbarOutput(XVIDC_VM_1920x1080_60_P,
						XVIDC_CSF_RGB, XVIDC_BPC_8);
			}
		}

		/* Set the FRL Cke source to internal. */
		XV_Tx_SetFRLCkeSrcToExternal(xhdmi_exdes_ctrlr.hdmi_tx_ctlr, FALSE);
		break;
#if defined (XPAR_XV_HDMITXSS1_NUM_INSTANCES) && \
                 defined (XPAR_XV_HDMIRXSS1_NUM_INSTANCES)
	case EXDES_TX_INPUT_RX:
		/* It is possible that the user runs into a scenario,
		 * where the AUX Fifo overflows and the com port or
		 * uart log is flooded with "HW Aux Full" messages.
		 * A previous work around for such a failure is mentioned
		 * in function Exdes_SendInfoFrame_Passthru in this file.
		 *
		 * Another option, in such a scenario, would be to reset the
		 * TX before Tx is updated with new parameters from the RX in
		 * pass-through mode.
		 */

		/* Copy the RX video parameters to TX. */
		if (xhdmi_exdes_ctrlr.crop) {
			/* this means RX is FRL with larger video than TMDS can support
			 * TX video set to 4K@30
			 */
			Exdes_UpdateVidParamstoTx(&HdmiRxSs, &HdmiTxSs);
		} else {
			Exdes_CopyRxVidParamstoTx(&HdmiRxSs, &HdmiTxSs);
			XV_Tx_SetVic(&xhdmi_example_tx_controller,
					XV_HdmiRxSs1_GetVideoIDCode(&HdmiRxSs));
			xil_printf(ANSI_COLOR_YELLOW "Entering pass-thru. Setting"
					" TX stream to RX stream. Rx.IsFrl = %d\r\n"
					ANSI_COLOR_RESET "\r\n",
					XV_HdmiRxSs1_GetTransportMode(&HdmiRxSs));

#if (!defined XPAR_XVTC_NUM_INSTANCES) || (defined (XPAR_XV_FRMBUFWR_NUM_INSTANCES) && \
                      (XPAR_XV_FRMBUFRD_NUM_INSTANCES))
		 /* Read FVA factor from VTEM */
			if(VrrInforFrame.VrrIfType == XV_HDMIC_VRRINFO_TYPE_VTEM) {
				FVaFactor = VrrInforFrame.VidTimingExtMeta.FVAFactorMinus1+1;
			}
		 /*update Gaming Features */
		 Exdes_UpdateGamingFeatures(&HdmiTxSs,TRUE,FVaFactor,CNMVRR);

#endif
		}

		if (xhdmi_exdes_ctrlr.crop)
			LineRate = 0;
		else
			LineRate = XV_Rx_GetLineRate(&xhdmi_example_rx_controller);

		/* Check GT line rate
		 * For 4k60pm Tmds clock ratio should be set to 1 and
		 * scrambler should be enabled.
		 * For other resolutions, disable the scrambler and set the
		 * Tmds clock ratio to 0.
		 */
		XV_Tx_SetLineRate(&xhdmi_example_tx_controller, LineRate);

		/* Set the FRL Cke source to Internal as this has FB. */
#if defined (XPAR_XV_FRMBUFWR_NUM_INSTANCES) && \
                      (XPAR_XV_FRMBUFRD_NUM_INSTANCES)
		/* Set the FRL Cke source to Internal as this has FB. */
		XV_Tx_SetFRLCkeSrcToExternal(xhdmi_exdes_ctrlr.hdmi_tx_ctlr, FALSE);
#else
		/* Set the FRL Cke source to external. */
		XV_Tx_SetFRLCkeSrcToExternal(xhdmi_exdes_ctrlr.hdmi_tx_ctlr, TRUE);
#endif

		break;
#endif
	default:
		break;
	}

	return Status;
}

/*****************************************************************************/
/**
*
* This function is used to determine the source for tx stream based on
* the configuration of the system and the presence/absence of rx and tx steams.
*
* @param  None.
*
* @return Returns the Tx Input source.
*
* @note
*
******************************************************************************/
TxInputSourceType Exdes_DetermineTxSrc()
{
#if defined(XPAR_XV_HDMIRXSS1_NUM_INSTANCES)
	u8 IsTx;
	u8 IsRx;
#endif
	TxInputSourceType TxInputSrc;
#if defined(XPAR_XV_HDMIRXSS1_NUM_INSTANCES)
	IsTx = xhdmi_exdes_ctrlr.IsTxPresent;
	/* Alternatively,
	 * IsTx = XV_Tx_IsConnected(xhdmi_exdes_ctrlr.hdmi_tx_ctlr);
	 */
	IsRx = xhdmi_exdes_ctrlr.IsRxPresent;
	/* Alternatively,
	 * IsRx = XV_Rx_IsStreamOn(xhdmi_exdes_ctrlr.hdmi_rx_ctlr);
	 */

	/**
	 * All possible scenarios :-
	 * (Independent. 0 - PT; 1- Independent)
	 * (RX SteamOn. 0 - Rx StreamDown/Disconnect; 1 - Rx Stream Up)
	 * (TX Connected. 0 - Tx Connected and Edid read; 1- Tx Disconnected)
	 * -------------- ------------ -------------- ---------------
	 *  Independent  | RX SteamOn | TX Connected |   TX Source   |
	 * -------------- ------------ -------------- ---------------
	 *       1       |      0     |      0       |   NONE_NOCON  |
	 *       1       |      0     |      1       |      TPG      |
	 *       1       |      1     |      0       |  NONE_RXONLY  |
	 *       1       |      1     |      1       |      TPG      |
	 *       0       |      0     |      0       |   NONE_NOCON  |
	 *       0       |      0     |      1       | RESCHANGE/TPG |
	 *       0       |      1     |      0       |  NONE_RXONLY  |
	 *       0       |      1     |      1       |   INPUT_RX    |
	 */
	if (xhdmi_exdes_ctrlr.ForceIndependent == TRUE) {
		xil_printf("Independent Mode - RX : %d, Tx : %d\r\n", IsRx, IsTx);
		if (!IsRx && !IsTx) {
			TxInputSrc = EXDES_TX_INPUT_NONE_NOCONNECTIONS;
		} else if (!IsRx && IsTx) {
#endif
			/* Since we are in independent mode, the RX can
			 * be disconnected or be waiting on a new stream.
			 * In either case we should start TX regardless.
			 */
			TxInputSrc = EXDES_TX_INPUT_TPG;
#if defined(XPAR_XV_HDMIRXSS1_NUM_INSTANCES)
		} else if (IsRx && !IsTx) {
			TxInputSrc = EXDES_TX_INPUT_NONE_RXONLY;
		} else if (IsRx && IsTx) {
			TxInputSrc = EXDES_TX_INPUT_TPG;
		}
#endif
#if defined(XPAR_XV_HDMIRXSS1_NUM_INSTANCES)
	} else {
		xil_printf("Pass-through Mode - RX : %d, Tx : %d\r\n", IsRx, IsTx);
		if (!IsRx && !IsTx) {
			TxInputSrc = EXDES_TX_INPUT_NONE_NOCONNECTIONS;
		} else if (!IsRx && IsTx) {
			/* Check for Resolution change condition. */
			if (Exdes_CheckforResChange() == TRUE) {
				/* wait for Rx to come up again. */
				TxInputSrc = EXDES_TX_INPUT_NONE_WAITFORNEWSTREAM;
			} else {
				/* Start Tx in colorbar. */
				TxInputSrc = EXDES_TX_INPUT_TPG;
			}
		} else if (IsRx && !IsTx) {
			TxInputSrc = EXDES_TX_INPUT_NONE_RXONLY;
		} else if (IsRx && IsTx) {
			if (XV_HdmiRxSs1_GetTransportMode(xhdmi_exdes_ctrlr.hdmi_rx_ctlr->HdmiRxSs) ==
					XV_HdmiTxSs1_GetTransportMode(xhdmi_exdes_ctrlr.hdmi_tx_ctlr->HdmiTxSs)) {
			} else {
#if defined (XPAR_XV_FRMBUFWR_NUM_INSTANCES) && \
                      (XPAR_XV_FRMBUFRD_NUM_INSTANCES)
				xil_printf(ANSI_COLOR_YELLOW "RX and TX"
						" transport mode (FRL or TMDS)"
						" mismatch. Video may be cropped to match"
						" TMDS bandwidth\r\n."
						ANSI_COLOR_RESET);
#else
				xil_printf(ANSI_COLOR_YELLOW "RX and TX"
						" transport mode (FRL or TMDS)"
						" mismatch. Please set them"
						" to the same transport mode."
						ANSI_COLOR_RESET);
#endif
			}
			TxInputSrc = EXDES_TX_INPUT_RX;
		}
	}
#endif
	EXDES_DBG_PRINT("TxInputSrc = %d\r\n", TxInputSrc);
	return TxInputSrc;
}
#endif
#ifdef XPAR_XV_HDMITXSS1_NUM_INSTANCES
/*****************************************************************************/
/**
*
* This function clones the EDID of the connected sink device to the HDMI RX
* @return None.
*
* @param  None.
*
* @return None.
*
* @note   None.
*
******************************************************************************/
void CloneTxEdid(void)
{
#ifdef XPAR_XV_HDMIRXSS1_NUM_INSTANCES
	u8 Buffer[256];
	u32 Status;

	/* Read TX edid */
	Status = XV_HdmiTxSs1_ReadEdid(&HdmiTxSs, (u8*)&Buffer);

	/* Check if read was successful */
	if (Status == (XST_SUCCESS)) {
		/* Load new EDID */
		XV_HdmiRxSs1_LoadEdid(&HdmiRxSs, (u8*)&Buffer, sizeof(Buffer));

		/* Toggle HPD after loading new HPD */
		ToggleHdmiRxHpd(&Hdmiphy1, &HdmiRxSs);

		xil_printf("\r\n");
		xil_printf("Successfully cloned EDID and toggled HPD.\r\n");
	}
#else
	xil_printf("\r\nEdid Cloning no possible with HDMI RX SS.\r\n");
#endif
}

#ifdef XPAR_XV_TPG_NUM_INSTANCES
/*****************************************************************************/
/**
*
* This function resets the TPG.
*
* @param  None.
*
* @return None.
*
* @note   None.
*
******************************************************************************/
void ResetFrameBuf(u8 fb_reset)
{
	u32 RegValue;
	/*
	 * fb_reset 0 or 2 resets wr fb
	 * fb_reset 1 or 2 resets rd fb
	 */

	if (fb_reset == 0 || fb_reset == 2) {
		RegValue = XGpio_DiscreteRead(&Gpio_Tpg_resetn, 2);

		XGpio_SetDataDirection(&Gpio_Tpg_resetn, 2, 0);
		XGpio_DiscreteWrite(&Gpio_Tpg_resetn, 2, RegValue&0xE);
		usleep(1000);

		XGpio_DiscreteWrite(&Gpio_Tpg_resetn, 2, RegValue|0x1);
		usleep(1000);
	}

	if (fb_reset == 1 || fb_reset == 2) {
		RegValue = XGpio_DiscreteRead(&Gpio_Tpg_resetn, 2);

		XGpio_SetDataDirection(&Gpio_Tpg_resetn, 2, 0);
		XGpio_DiscreteWrite(&Gpio_Tpg_resetn, 2, RegValue&0xD);
		usleep(1000);

		XGpio_DiscreteWrite(&Gpio_Tpg_resetn, 2, RegValue|0x2);
		usleep(1000);
	}

}




/*****************************************************************************/
/**
*
* This function resets the TPG.
*
* @param  None.
*
* @return None.
*
* @note   None.
*
******************************************************************************/
void ResetTpg(void)
{
	u32 RegValue;

	RegValue = XGpio_DiscreteRead(&Gpio_Tpg_resetn, 1);

	XGpio_SetDataDirection(&Gpio_Tpg_resetn, 1, 0);
	XGpio_DiscreteWrite(&Gpio_Tpg_resetn, 1, RegValue&0xE);
	usleep(1000);

	XGpio_DiscreteWrite(&Gpio_Tpg_resetn, 1, RegValue|0x1);
	usleep(1000);
}
#endif

#ifdef XPAR_XV_AXI4S_REMAP_NUM_INSTANCES
/*****************************************************************************/
/**
*
* This function resets the InRemap.
*
* @param  None.
*
* @return None.
*
* @note   None.
*
******************************************************************************/
void ResetInRemap(void){
	u32 RegValue;

	RegValue = XGpio_DiscreteRead(&Gpio_Tpg_resetn, 1);

	XGpio_SetDataDirection(&Gpio_Tpg_resetn, 1, 0);
	XGpio_DiscreteWrite(&Gpio_Tpg_resetn, 1, RegValue&0xD);
	usleep(1000);

	XGpio_DiscreteWrite(&Gpio_Tpg_resetn, 1, RegValue|0x2);
	usleep(1000);
}

/*****************************************************************************/
/**
*
* This function resets the OutRemap.
*
* @param  None.
*
* @return None.
*
* @note   None.
*
******************************************************************************/
void ResetOutRemap(void){
	u32 RegValue;

	RegValue = XGpio_DiscreteRead(&Gpio_Tpg_resetn, 1);

	XGpio_SetDataDirection(&Gpio_Tpg_resetn, 1, 0);
	XGpio_DiscreteWrite(&Gpio_Tpg_resetn, 1, RegValue&0xB);
	usleep(1000);

	XGpio_DiscreteWrite(&Gpio_Tpg_resetn, 1, RegValue|0x4);
	usleep(1000);
}
#endif

#ifdef XPAR_XV_HDMITXSS1_NUM_INSTANCES
/*****************************************************************************/
/**
*
* This function resets the AuxFifo.
*
* @param  None.
*
* @return None.
*
* @note   None.
*
******************************************************************************/
void ResetAuxFifo(void)
{
#if defined (XPAR_XV_HDMITXSS1_NUM_INSTANCES) && \
                 defined (XPAR_XV_HDMIRXSS1_NUM_INSTANCES)
	AuxFifoStartFlag   = (FALSE);
	AuxFifoStartIndex  = 0;
	AuxFifoEndIndex    = 0;
	AuxFifoCount	   = 0;
	AuxFifoOvrFlowCnt  = 0;
#endif
}
#endif

/*****************************************************************************/
/**
*
* This function sends vendor specific infoframe.
*
* @param  HdmiTxSs1Ptr is a pointer to the HDMI 2.1 TX SS.
*
* @return None.
*
* @note   None.
*
******************************************************************************/
void SendVSInfoframe(XV_HdmiTxSs1 *HdmiTxSs1Ptr)
{
	u32 Status = XST_SUCCESS;
	XHdmiC_VSIF *VSIFPtr;
	VSIFPtr = XV_HdmiTxSs1_GetVSIF(HdmiTxSs1Ptr);

	XHdmiC_Aux Aux;

	(void)memset((void *)VSIFPtr, 0, sizeof(XHdmiC_VSIF));
	(void)memset((void *)&Aux, 0, sizeof(XHdmiC_Aux));

	VSIFPtr->Version = 0x1;
	VSIFPtr->IEEE_ID = 0xC03;

	if (XVidC_IsStream3D(&(HdmiTxSs1Ptr->HdmiTx1Ptr->Stream.Video))) {
		VSIFPtr->Format = XHDMIC_VSIF_VF_3D;
		VSIFPtr->Info_3D.Stream =
				HdmiTxSs1Ptr->HdmiTx1Ptr->Stream.Video.Info_3D;
		VSIFPtr->Info_3D.MetaData.IsPresent = FALSE;
	} else if (HdmiTxSs1Ptr->HdmiTx1Ptr->Stream.Video.VmId ==
					   XVIDC_VM_3840x2160_24_P ||
		   HdmiTxSs1Ptr->HdmiTx1Ptr->Stream.Video.VmId ==
					   XVIDC_VM_3840x2160_25_P ||
		   HdmiTxSs1Ptr->HdmiTx1Ptr->Stream.Video.VmId ==
					   XVIDC_VM_3840x2160_30_P ||
		   HdmiTxSs1Ptr->HdmiTx1Ptr->Stream.Video.VmId ==
					   XVIDC_VM_4096x2160_24_P) {
		VSIFPtr->Format = XHDMIC_VSIF_VF_EXTRES;

		/* Set HDMI VIC */
		switch(HdmiTxSs1Ptr->HdmiTx1Ptr->Stream.Video.VmId) {
			case XVIDC_VM_4096x2160_24_P :
				VSIFPtr->HDMI_VIC = 4;
				break;
			case XVIDC_VM_3840x2160_24_P :
				VSIFPtr->HDMI_VIC = 3;
				break;
			case XVIDC_VM_3840x2160_25_P :
				VSIFPtr->HDMI_VIC = 2;
				break;
			case XVIDC_VM_3840x2160_30_P :
				VSIFPtr->HDMI_VIC = 1;
				break;
			default :
				break;
		}
	} else {
		VSIFPtr->Format = XHDMIC_VSIF_VF_NOINFO;
	}

	Aux = XV_HdmiC_VSIF_GeneratePacket(VSIFPtr);

	Status = XV_HdmiTxSs1_SendGenericAuxInfoframe(HdmiTxSs1Ptr, &Aux);
	/* If TX Core's hardware Aux FIFO is full, from the while loop,
	 * retry during the next main while iteration.
	 */
	if (Status != (XST_SUCCESS)) {
		/* Enable this print to profile the overflow of HW AUX Fifo.
		 * However, in case of overlow this print will flood
		 * the UART output port.
		 * xil_printf(ANSI_COLOR_RED "%s,%d. HW Aux Full"
		 *            ANSI_COLOR_RESET "\r\n", __func__, __LINE__);
		 */
	}

}

/*****************************************************************************/
/**
*
* This function resets the Tx Info Frame.
*
* @param  None.
*
* @return None.
*
* @note   None.
*
******************************************************************************/
void TxInfoFrameReset(void)
{
	XHdmiC_AVI_InfoFrame *AviInfoFramePtr;
	XHdmiC_AudioInfoFrame *AudioInfoFramePtr;

	AviInfoFramePtr = XV_HdmiTxSs1_GetAviInfoframe(&HdmiTxSs);
	AudioInfoFramePtr = XV_HdmiTxSs1_GetAudioInfoframe(&HdmiTxSs);

	/* Reset Avi InfoFrame */
	(void)memset((void *)AviInfoFramePtr,
	             0,
		     sizeof(XHdmiC_AVI_InfoFrame));

	/* Reset Audio InfoFrame */
	(void)memset((void *)AudioInfoFramePtr,
	             0,
		     sizeof(XHdmiC_AudioInfoFrame));

	AviInfoFramePtr->Version = 2;
	AviInfoFramePtr->ColorSpace = XHDMIC_COLORSPACE_RGB;
	AviInfoFramePtr->VIC = 16;
	AviInfoFramePtr->PicAspectRatio = XHDMIC_PIC_ASPECT_RATIO_16_9;
	/* To set the channel count, do,
	 * AudioInfoFramePtr->ChannelCount = XHDMIC_AUDIO_CHANNEL_COUNT_3;
	 */
}
#endif

/*****************************************************************************/
/**
*
* This function setup sets up the On Board IIC MUX to select device
*
* @param  None.
*
* @return The number of bytes sent.
*
* @note   None.
*
******************************************************************************/
int I2cMuxSel(void *IicPtr, XOnBoard_IicDev Dev)
{
	u8 Iic_Mux_Addr;
	u8 Buffer;
	int Status;

#if defined (XPS_BOARD_ZCU102) || \
	defined (XPS_BOARD_ZCU106) || \
    defined (XPS_BOARD_VCK190)
	XIicPs *Iic_Ptr = IicPtr;

	/* Set operation to 7-bit mode */
	XIicPs_SetOptions(Iic_Ptr, XIICPS_7_BIT_ADDR_OPTION);
	XIicPs_ClearOptions(Iic_Ptr, XIICPS_10_BIT_ADDR_OPTION);
	/* Clear Repeated Start option */
	XIicPs_ClearOptions(Iic_Ptr, XIICPS_REP_START_OPTION);

#if defined (XPS_BOARD_ZCU102)
	if (Dev == ZCU102_MGT_SI570) {
		Iic_Mux_Addr = ZCU102_U34_MUX_I2C_ADDR;
		Buffer = ZCU102_U34_MUX_SEL_SI570;
	}
#elif defined (XPS_BOARD_ZCU106)
	if (Dev == ZCU106_MGT_SI570) {
		Iic_Mux_Addr = ZCU106_U34_MUX_I2C_ADDR;
		Buffer = ZCU106_U34_MUX_SEL_SI570;
	}
#elif defined (XPS_BOARD_VCK190)
	if (Dev == VCK190_MGT_SI570) {
		Iic_Mux_Addr = VCK190_U34_MUX_I2C_ADDR;
		Buffer = VCK190_U34_MUX_SEL_SI570;
	}
#endif

	Status = XIicPs_MasterSendPolled(Iic_Ptr, (u8 *)&Buffer, 1, Iic_Mux_Addr);

	/*
	 * Wait until bus is idle to start another transfer.
	 */
	if (!(Iic_Ptr->IsRepeatedStart)) {
		while (XIicPs_BusIsBusy(Iic_Ptr));
	}

	if (Status == XST_SUCCESS) {
		return 1;
	} else {
		return 0;
	}
#else
	XIic *Iic_Ptr = IicPtr;

#if defined (XPS_BOARD_VCU118)
	if (Dev == VCU118_FMCP) {
		Iic_Mux_Addr = VCU118_U80_MUX_I2C_ADDR;
		Buffer = VCU118_U80_MUX_SEL_FMCP;
	}
#elif defined (XPS_BOARD_VEK280_ES)
	if (Dev == VCK190_MGT_SI570) {
		Iic_Mux_Addr = VCK190_U34_MUX_I2C_ADDR;
		Buffer = VCK190_U135_MUX_I2C_ADDR;
	}
#else

	if (Dev == KCU105_SI570) {
		Iic_Mux_Addr = KCU105_U28_MUX_I2C_ADDR;
		Buffer = KCU105_U28_MUX_SEL_SI570;
	}
#endif

	Status =  XIic_Send(Iic_Ptr->BaseAddress, Iic_Mux_Addr,(u8 *)&Buffer,
							1, XIIC_STOP);
#endif

	return Status;
}

/*****************************************************************************/
/**
*
* This function setup SI5324 clock generator either in free or locked mode.
*
* @param  Index specifies an index for selecting mode frequency.
* @param  Mode specifies either free or locked mode.
*
* @return
*   - Zero if error in programming external clock.
*   - One if programmed external clock.
*
* @note   None.
*
******************************************************************************/
int I2cClk(u32 InFreq, u32 OutFreq)
{
	int Status;

#if defined (XPS_BOARD_VEK280_ES)
	if (OutFreq != 0) {
	/* VFMC TX Clock Source */
		Status = IDT_8T49N24x_I2cClk(&Iic, I2C_CLK_ADDR1,
						InFreq, OutFreq);
	}
#else
	Vfmc_I2cMuxSelect(&Vfmc[0]);

	if (OutFreq != 0) {
	/* VFMC TX Clock Source */
		Status = IDT_8T49N24x_I2cClk(&Iic, I2C_CLK_ADDR,
						InFreq, OutFreq);
	}
#endif

	/* To profile the in frequency and out frequency, each time a new
	 * clock is set, do,
	 * xil_printf("===\r\n===IN: %d, OUT: %d, Status: %d===\r\n===\r\n",
	 *            InFreq, OutFreq, Status);
	 */
	return Status;
}
#if defined (XPAR_XV_FRMBUFWR_NUM_INSTANCES) && \
                      (XPAR_XV_FRMBUFRD_NUM_INSTANCES)
/*****************************************************************************/
/**
*
* This function outputs the Dynamic HDR info of HDMI RX
*
* @param  None.
*
* @return None.
*
* @note   None.
*
******************************************************************************/
void XV_ReportDynamicHDR(void)
{
    xil_printf("\r\n");
    xil_printf("Dynamic HDR \r\n");
    xil_printf("------------\r\n");

	xil_printf("Packet type: 0x%x (%s) \r\n",RX_DynHDR_Info.pkt_type,
			Dynamic_hdr_type[RX_DynHDR_Info.pkt_type]);
	xil_printf("Packet Length: 0x%x \r\n",RX_DynHDR_Info.pkt_length);
	xil_printf("Graphics Overlay Flag: 0x%x \r\n",RX_DynHDR_Info.gof);
	xil_printf("Packet Error: 0x%x \r\n",RX_DynHDR_Info.err);
}
#endif
/*****************************************************************************/
/**
*
* This function outputs the video timing , Audio, Link Status, HDMI RX state of
* HDMI RX core. In addition, it also prints information about HDMI TX, and
* HDMI GT cores.
*
* @param  None.
*
* @return None.
*
* @note   None.
*
******************************************************************************/
void Info(void)
{
	xil_printf("\r\n-----\r\n");
	xil_printf("Info\r\n");
	xil_printf("-----\r\n\r\n");

#ifdef XPAR_XV_HDMITXSS1_NUM_INSTANCES
    xil_printf("------------\r\n");
    xil_printf("HDMI TX SubSystem\r\n");
    xil_printf("------------\r\n");
    xil_printf("HDMI TX timing\r\n");
    xil_printf("------------\r\n");
    XV_HdmiTxSs1_ReportTiming(&HdmiTxSs);
    xil_printf("Audio\r\n");
    xil_printf("---------\r\n");
    XV_HdmiTxSs1_ReportAudio(&HdmiTxSs);
    xil_printf("Static HDR DRM Infoframe\r\n");
    xil_printf("---------\r\n");
    XV_HdmiTxSs1_ReportDRMInfo(&HdmiTxSs);
#if defined(USE_HDCP_HDMI_TX)
    HdmiTxSs.EnableHDCPLogging = (FALSE);
    XV_HdmiTxSs1_HdcpInfo(&HdmiTxSs);
#endif
#endif
#ifdef XPAR_XV_HDMIRXSS1_NUM_INSTANCES
    xil_printf("\r\n------------\r\n");
    xil_printf("HDMI RX SubSystem\r\n");
    xil_printf("------------\r\n");
    xil_printf("HDMI RX timing\r\n");
    xil_printf("------------\r\n");
    XV_HdmiRxSs1_ReportTiming(&HdmiRxSs);
    xil_printf("\r\nLink quality\r\n");
    xil_printf("---------\r\n");
    XV_HdmiRxSs1_ReportLinkQuality(&HdmiRxSs);
    xil_printf("Audio\r\n");
    xil_printf("---------\r\n");
    XV_HdmiRxSs1_ReportAudio(&HdmiRxSs);
    xil_printf("Static HDR DRM Infoframe\r\n");
    xil_printf("---------\r\n");
    XV_HdmiRxSs1_ReportDRMInfo(&HdmiRxSs);
#if defined(USE_HDCP_HDMI_RX)
    HdmiRxSs.EnableHDCPLogging = (FALSE);
    XV_HdmiRxSs1_HdcpInfo(&HdmiRxSs);
#endif
#endif

#if defined (XPAR_XV_FRMBUFWR_NUM_INSTANCES) && \
                      (XPAR_XV_FRMBUFRD_NUM_INSTANCES)
    XV_ReportDynamicHDR();
#endif

	/* GT */
	xil_printf("\r\n------------\r\n");
	xil_printf("HDMI PHY\r\n");
	xil_printf("------------\r\n");
	xil_printf("GT status\r\n");
	xil_printf("---------\r\n");
#ifdef XPAR_XV_HDMITXSS1_NUM_INSTANCES
	xil_printf("TX reference clock frequency: %0d Hz\r\n",
		   XHdmiphy1_ClkDetGetRefClkFreqHz(&Hdmiphy1,
						   XHDMIPHY1_DIR_TX));
#endif
#ifdef XPAR_XV_HDMIRXSS1_NUM_INSTANCES
	xil_printf("RX reference clock frequency: %0d Hz\r\n",
		   XHdmiphy1_ClkDetGetRefClkFreqHz(&Hdmiphy1,
						   XHDMIPHY1_DIR_RX));
	if (Hdmiphy1.Config.DruIsPresent == (TRUE)) {
		xil_printf("DRU reference clock frequency: %0d Hz\r\n",
			   XHdmiphy1_DruGetRefClkFreqHz(&Hdmiphy1));
	}
#endif
	XHdmiphy1_HdmiDebugInfo(&Hdmiphy1, 0, XHDMIPHY1_CHANNEL_ID_CH1);

}

/*****************************************************************************/
/**
*
* This function prints additional details of the system for debug purpose
*
* @param  None.
*
* @return None.
*
* @note   None.
*
******************************************************************************/
void DetailedInfo(void)
{
	u32 Data = 0;
#if defined(XPAR_XV_HDMIRXSS1_NUM_INSTANCES)
	u32 TotalPixFRLRatio;
	u32 ActivePixFRLRatio;
#endif
#if defined(XPAR_XV_HDMITXSS1_NUM_INSTANCES)
	u8* EdCounters;
#endif
	xil_printf("\r\n------------\r\n");
	xil_printf("Additional Info\r\n");
	xil_printf("------------\r\n");
	xil_printf("System Status\r\n");
	xil_printf("------------\r\n");

#if defined (XPAR_XV_HDMITXSS1_NUM_INSTANCES) && \
    defined (XPAR_XV_HDMIRXSS1_NUM_INSTANCES)
	FrWrDoneCounter = 0;
	FrRdDoneCounter = 0;
	xil_printf("AuxFifo Overflow Count: %d\r\n", AuxFifoOvrFlowCnt);
#endif
#if defined(XPAR_XV_HDMIRXSS1_NUM_INSTANCES)
	xil_printf("AuxCounter: %d\r\n", AuxCounter);
	AuxCounter = 0;
#endif
#if defined(XPAR_XV_HDMITXSS1_NUM_INSTANCES)
	xil_printf("VsyncCounter: %d\r\n",
			VsyncCounter);
	xil_printf ("TX HW AUX FULL Counter: %d\r\n", AuxHwFullCounter);
	VsyncCounter = 0;
	AuxHwFullCounter = 0;
#endif
#if defined(XPAR_XV_HDMIRXSS1_NUM_INSTANCES)
	/* Get the ratio and print. */
	ActivePixFRLRatio = XV_HdmiRx1_GetFrlActivePixRatio(HdmiRxSs.HdmiRx1Ptr);
	TotalPixFRLRatio  = XV_HdmiRx1_GetFrlTotalPixRatio(HdmiRxSs.HdmiRx1Ptr);
	xil_printf ("ActivePixFRLRatio ,%d\r\n", ActivePixFRLRatio);
	xil_printf ("TotalPixFRLRatio ,%d\r\n", TotalPixFRLRatio);
#endif
	xil_printf("------------\r\n");
	xil_printf("Bridge Status\r\n");
	xil_printf("------------\r\n");
#if defined(XPAR_XV_HDMITXSS1_NUM_INSTANCES)
	xil_printf("TX BRIDGE OVERFLOW ,%d\r\n",
		   xhdmi_exdes_ctrlr.hdmi_tx_ctlr->ErrorStats.TxBrdgOverflowCnt);
	xil_printf("TX BRIDGE UNDERFLOW ,%d\r\n",
		   xhdmi_exdes_ctrlr.hdmi_tx_ctlr->ErrorStats.TxBrdgUnderflowCnt);
#endif
#if defined(XPAR_XV_HDMIRXSS1_NUM_INSTANCES)
	xil_printf("RX BRIDGE OVERFLOW ,%d\r\n",
		   xhdmi_exdes_ctrlr.hdmi_rx_ctlr->ErrorStats.RxBrdgOverflowCnt);
#endif

#if defined(XPAR_XV_HDMITXSS1_NUM_INSTANCES)
	{
	u8 status[2];
	XV_HdmiTx1_DdcReadReg(HdmiTxSs.HdmiTx1Ptr,
			      XV_HDMITX1_DDC_ADDRESS,
			      1,
			      0x40,
			      (u8*)&(status));
	xil_printf("SCDC status : %x\r\n",status[0]);
	}
	/* Note: Reading the SCDC Character Error Detection and Reed-Solomon
	 * Corrections Counter will clear their values at the sink */
	xil_printf("------------\r\n");
	xil_printf("Error Detection Status From Sink\r\n");
	xil_printf("------------\r\n");
	EdCounters = XV_HdmiTxSs1_GetScdcEdRegisters(&HdmiTxSs);
	xil_printf("Channel 0 CED: ");

	/* Valid only when the valid bit (0x80) is set */
	if (EdCounters[1] & 0x80) {
		Data = EdCounters[1];
		Data = ((EdCounters[1] & 0x7F) << 8) | EdCounters[0];
		xil_printf("%d\r\n", Data);
	} else {
		xil_printf("Invalid\r\n", Data);
	}

	xil_printf("Channel 1 CED: ");

	/* Valid only when the valid bit (0x80) is set */
	if (EdCounters[3] & 0x80) {
		Data = ((EdCounters[3] & 0x7F) << 8) | EdCounters[2];
		xil_printf("%d\r\n", Data);
	} else {
		xil_printf("Invalid\r\n", Data);
	}

	xil_printf("Channel 2 CED: ");

	/* Valid only when the valid bit (0x80) is set */
	if (EdCounters[5] & 0x80) {
		Data = ((EdCounters[1] & 0x7F) << 8) | EdCounters[4];
		xil_printf("%d\r\n", Data);
	} else {
		xil_printf("Invalid\r\n", Data);
	}

	if (HdmiTxSs.HdmiTx1Ptr->Stream.IsFrl == TRUE) {
		xil_printf("Channel 3 CED: ");
		/* Valid only when the valid bit (0x80) is set */
		if (EdCounters[8] & 0x80) {
			Data = ((EdCounters[8] & 0x7F) << 8) | EdCounters[7];
			xil_printf("%d\r\n", Data);
		} else {
			xil_printf("Invalid\r\n", Data);
		}

		xil_printf("RSED: ");
		/* Valid only when the valid bit (0x80) is set */
		if (EdCounters[10] & 0x80) {
			Data = ((EdCounters[10] & 0x7F) << 8) | EdCounters[9];
			xil_printf("%d\r\n", Data);
		} else {
			xil_printf("Invalid\r\n", Data);
		}
	}
#endif
	xil_printf("------------\r\n");
	xil_printf("System: IP Version\r\n");
	xil_printf("------------\r\n");
#if defined(XPAR_XV_HDMITXSS1_NUM_INSTANCES)
	xil_printf("HDMI TX SS\r\n");
	xil_printf("------------");
	XV_HdmiTxSs1_ReportCoreInfo(&HdmiTxSs);
	XV_HdmiTxSs1_ReportSubcoreVersion(&HdmiTxSs);
#endif

#if defined(XPAR_XV_HDMIRXSS1_NUM_INSTANCES)
	xil_printf("------------\r\n");
	xil_printf("HDMI RX SS\r\n");
	xil_printf("------------");
	XV_HdmiRxSs1_ReportCoreInfo(&HdmiRxSs);
	XV_HdmiRxSs1_ReportSubcoreVersion(&HdmiRxSs);
#endif

#if defined(XPAR_XHDMIPHY1_NUM_INSTANCES)
	xil_printf("------------\r\n");
	xil_printf("HDMI PHY\r\n");
	xil_printf("------------\r\n");
	Data = XHdmiphy1_GetVersion(&Hdmiphy1);
	xil_printf("  HDMI Phy version : %02d.%02d (%04x)\r\n",
		   ((Data >> 24) & 0xFF),
		   ((Data >> 16) & 0xFF),
		   (Data & 0xFFFF));
#endif

#if defined(XPAR_XV_HDMITXSS1_NUM_INSTANCES)
	xil_printf("\r\n------------\r\n");
	xil_printf("TX Core Status\r\n");
	xil_printf("------------\r\n");
	XV_HdmiTxSs1_DebugInfo(&HdmiTxSs);
#endif

#if defined(XPAR_XV_HDMIRXSS1_NUM_INSTANCES)
	xil_printf("\r\n------------\r\n");
	xil_printf("RX Core Status\r\n");
	xil_printf("------------\r\n");
	XV_HdmiRxSs1_DebugInfo(&HdmiRxSs);

	if(VrrInforFrame.VrrIfType == XV_HDMIC_VRRINFO_TYPE_VTEM) {
		xil_printf("\r\n------------\r\n");
		xil_printf("RX Core VTEM Info Frame\r\n");
		xil_printf("------------\r\n");
		xil_printf("VRR_EN : %d \r\n",VrrInforFrame.VidTimingExtMeta.VRREnabled);
		xil_printf("M_CONST : %d \r\n",VrrInforFrame.VidTimingExtMeta.MConstEnabled);
		xil_printf("QMS_EN : %d \r\n",VrrInforFrame.VidTimingExtMeta.QMSEnabled);
		xil_printf("FVA_FACTOR_M1 : %d \r\n",VrrInforFrame.VidTimingExtMeta.FVAFactorMinus1);
		xil_printf("Base_Vfront : %dHz \r\n",VrrInforFrame.VidTimingExtMeta.BaseVFront);
		xil_printf("Base_Refresh_Rate : %dHz \r\n",VrrInforFrame.VidTimingExtMeta.BaseRefreshRate);
		xil_printf("RB %d \r\n",VrrInforFrame.VidTimingExtMeta.RBEnabled);
		xil_printf("NEXT_TFR %d \r\n",VrrInforFrame.VidTimingExtMeta.NextTransferRate);
	} else if(VrrInforFrame.VrrIfType == XV_HDMIC_VRRINFO_TYPE_SPDIF) {
		xil_printf("\r\n------------\r\n");
		xil_printf("RX Core AMD VSIF Info Frame\r\n");
		xil_printf("------------\r\n");
		xil_printf("Version %d \r\n",VrrInforFrame.SrcProdDescIF.FreeSync.Version);
		xil_printf("FreeSync Supported : %d \r\n",VrrInforFrame.SrcProdDescIF.FreeSync.FreeSyncSupported);
		xil_printf("FreeSync Enabled : %d \r\n",VrrInforFrame.SrcProdDescIF.FreeSync.FreeSyncEnabled);
		xil_printf("FreeSync Active : %d \r\n",VrrInforFrame.SrcProdDescIF.FreeSync.FreeSyncActive);
		xil_printf("FreeSync MinRefreshRate : %dHz \r\n",VrrInforFrame.SrcProdDescIF.FreeSync.FreeSyncMinRefreshRate);
		xil_printf("FreeSync MaxRefreshRate :%dHz \r\n",VrrInforFrame.SrcProdDescIF.FreeSync.FreeSyncMaxRefreshRate);

		if(VrrInforFrame.SrcProdDescIF.FreeSync.Version == 0x2){
			xil_printf("FreeSync Pro Native ColorSpace Active : %d \r\n",VrrInforFrame.SrcProdDescIF.FreeSyncPro.NativeColorSpaceActive);
			xil_printf("FreeSync Pro Brightness Control Active : %d \r\n",VrrInforFrame.SrcProdDescIF.FreeSyncPro.BrightnessControlActive);
			xil_printf("FreeSync Pro Seamless Local Dimming DisableControl : %d \r\n",VrrInforFrame.SrcProdDescIF.FreeSyncPro.LocalDimControlActive);
			xil_printf("FreeSync Pro sRGB EOTF Active : %d \r\n",VrrInforFrame.SrcProdDescIF.FreeSyncPro.sRGBEOTFActive);
			xil_printf("FreeSync Pro BT709 EOTF Active : %d \r\n",VrrInforFrame.SrcProdDescIF.FreeSyncPro.BT709EOTFActive);
			xil_printf("FreeSync Pro Gamma 2.2 EOTF Active : %d \r\n",VrrInforFrame.SrcProdDescIF.FreeSyncPro.Gamma22EOTFActive);
			xil_printf("FreeSync Pro Gamma 2.6 EOTF Active : %d \r\n",VrrInforFrame.SrcProdDescIF.FreeSyncPro.Gamma26EOTFActive);
			xil_printf("FreeSync Pro PQ EOTF Active : %d \r\n",VrrInforFrame.SrcProdDescIF.FreeSyncPro.PQEOTFActive);
			xil_printf("FreeSync Pro Brightness Control : %d \r\n",VrrInforFrame.SrcProdDescIF.FreeSyncPro.BrightnessControl);
		}

	}
	xil_printf("\r\n------------\r\n");

#endif
}

#if defined(USE_HDCP_HDMI_RX) || \
	defined(USE_HDCP_HDMI_TX)
/*****************************************************************************/
/**
*
* This function is responsible for executing the state machine for the
* upstream interface and each connected downstream interface. The
* state machines are executed using round robin scheduling. Interface
* poll functions are non-blocking, so starvation should not occur, but
* fairness is not guaranteed.
*
* @param    InstancePtr is a pointer to the XHdcp_Repeater instance.
*
* @return   None.
*
* @note	    None.
*
******************************************************************************/
void XHdcp_Poll()
{
	/* Verify arguments */

#if defined(XPAR_XV_HDMIRXSS1_NUM_INSTANCES) && \
	defined(USE_HDCP_HDMI_RX)
	/* Call the upstream interface Poll function */
	XV_Rx_Hdcp_Poll(&xhdmi_example_rx_controller);
#endif

#if defined(XPAR_XV_HDMITXSS1_NUM_INSTANCES) && \
	defined(USE_HDCP_HDMI_TX)
	/* Call downstream interface Poll function */
	XV_Tx_Hdcp_Poll(&xhdmi_example_tx_controller);

	/*
	 * The stream-up event pushes an authentication request, but
	 * some sinks have a delay in setting the HDCP capability ;
	 * therefore, we must periodically attempt to authenticate.
	 * We trigger these authentication attempts by periodically
	 * setting an exclusion flag, and this helps to ensure
	 * that the processor is not stalled with excessive
	 * I2C transactions.
	 */
	/* Trigger authentication */
	if (XV_Tx_GetHdcpAuthReqExclusion(&xhdmi_example_tx_controller) == FALSE) {
		XV_Tx_Hdcp_Authenticate(&xhdmi_example_tx_controller);
#if defined (XPAR_XV_HDMITXSS1_NUM_INSTANCES) && \
                 defined (XPAR_XV_HDMIRXSS1_NUM_INSTANCES)
		XHdcp_EnforceBlanking(&xhdmi_example_rx_controller,
				      &xhdmi_example_tx_controller);
#endif

		XV_Tx_SetHdcpAuthReqExclusion(&xhdmi_example_tx_controller, TRUE);
	}
#endif

}
#endif
#if defined (XPAR_XV_HDMITXSS1_NUM_INSTANCES) && \
                 defined (XPAR_XV_HDMIRXSS1_NUM_INSTANCES)
#if defined(USE_HDCP_HDMI_RX) || \
	defined(USE_HDCP_HDMI_TX)
/*****************************************************************************/
/**
*
* This function enforces downstream content blocking based on the upstream
* encryption status and stream type information. When the content is required
* to be blocked, cipher output blanking is enabled.
*
* @param    HdcpInstancePtr is a pointer to the XHdcp_Repeater instance.
*
* @return   - XST_SUCCESS if blocking enforced successfully.
*           - XST_FAILURE if blocking could not be enforced.
*
* @note	    None.
*
******************************************************************************/
static void XHdcp_EnforceBlanking(XV_Rx *UpstreamInstancePtr,
		XV_Tx *DownstreamInstancePtr)
{
#ifdef XPAR_XV_HDMIRXSS1_NUM_INSTANCES
	u8 IsEncrypted;
	u8 EnforceBlocking = TRUE;
#endif
	int Status;

	/* Verify arguments */
	Xil_AssertVoid(UpstreamInstancePtr != NULL);
	Xil_AssertVoid(DownstreamInstancePtr != NULL);

#ifdef XPAR_XV_HDMIRXSS1_NUM_INSTANCES
	/* Update encryption status */
	IsEncrypted = XV_HdmiRxSs1_HdcpIsEncrypted(UpstreamInstancePtr->HdmiRxSs);
	u8 StreamType = XV_Rx_Hdcp_GetStreamType(UpstreamInstancePtr);
	EXDES_HDCP_DBG_PRINT(ANSI_COLOR_YELLOW"%s,%d :: StreamType = %d (%d)"
		  ANSI_COLOR_RESET"\r\n", __func__, __LINE__, StreamType,
		  XV_HdmiRxSs1_HdcpGetContentStreamType(
					UpstreamInstancePtr->HdmiRxSs));
#endif

	/* Enforce downstream content blocking */
#ifdef XPAR_XV_HDMIRXSS1_NUM_INSTANCES
	if (IsEncrypted && EnforceBlocking) {
#endif
		EXDES_HDCP_DBG_PRINT(ANSI_COLOR_YELLOW"%s,%d :: RxEncrypted"
				ANSI_COLOR_RESET"\r\n",	__func__, __LINE__);
		if (XV_HdmiTxSs1_HdcpIsAuthenticated(
					DownstreamInstancePtr->HdmiTxSs)) {
			EXDES_HDCP_DBG_PRINT(ANSI_COLOR_YELLOW"%s,%d :: "
					"TxAuthenticated"
					ANSI_COLOR_RESET"\r\n",
					__func__, __LINE__);
			/* Check the downstream interface protocol */
			Status = XV_HdmiTxSs1_HdcpGetProtocol(
					DownstreamInstancePtr->HdmiTxSs);

			EXDES_HDCP_DBG_PRINT(ANSI_COLOR_YELLOW"%s,%d :: "
					"TxProtocol = %d"
					ANSI_COLOR_RESET"\r\n", __func__,
					__LINE__, Status);
			switch (Status) {

			/* HDCP 2.2
			 * Allow content under the following conditions:
			 * - Stream type is 0
			 * - Stream type is 1, and no HDCP 1.x devices are downstream,
			 *   and no HDCP2 legacy devices are downstream. */
			case XV_HDMITXSS1_HDCP_22:
				Status = XV_HdmiTxSs1_HdcpGetTopologyField(
						DownstreamInstancePtr->HdmiTxSs,
						XV_HDMITXSS1_HDCP_TOPOLOGY_HDCP2LEGACYDEVICEDOWNSTREAM);
				Status |= XV_HdmiTxSs1_HdcpGetTopologyField(
						DownstreamInstancePtr->HdmiTxSs,
						XV_HDMITXSS1_HDCP_TOPOLOGY_HDCP1DEVICEDOWNSTREAM);
				if ((StreamType ==
				     XV_HDMITXSS1_HDCP_STREAMTYPE_0) ||
				    (Status == FALSE)) {
					EXDES_HDCP_DBG_PRINT(ANSI_COLOR_YELLOW" :: "
						"HDCP RX encrypted and TX (2.2)"
						" auth : Stream0 -> Blanking Off"
						ANSI_COLOR_RESET"\r\n");
					XV_HdmiTxSs1_HdcpDisableBlank(
						DownstreamInstancePtr->HdmiTxSs);
				} else {
					EXDES_HDCP_DBG_PRINT(ANSI_COLOR_YELLOW" :: "
						"HDCP RX encrypted and TX (2.2)"
						" auth : Stream1 or (2.2 rep or"
						"1.4 dwn = 1) -> Blanking On"
						ANSI_COLOR_RESET"\r\n");
					Status = XV_HdmiTxSs1_HdcpEnableBlank(
							DownstreamInstancePtr->HdmiTxSs);
					if (Status != XST_SUCCESS) {
						xdbg_printf(XDBG_DEBUG_GENERAL,
							"Error: Problem "
							"blocking downstream "
							"content.\r\n");
					}
				}
				break;

			/* HDCP 1.4
			 * Allow content when the stream type is 0 */
			case XV_HDMITXSS1_HDCP_14:
				if (StreamType == XV_HDMITXSS1_HDCP_STREAMTYPE_0) {
					EXDES_HDCP_DBG_PRINT(ANSI_COLOR_YELLOW
						" :: HDCP RX encrypted and TX "
						"(1.4) auth : Stream0 -> "
						"Blanking Off"
						ANSI_COLOR_RESET"\r\n");
				XV_HdmiTxSs1_HdcpDisableBlank(
						DownstreamInstancePtr->HdmiTxSs);
				} else {
					EXDES_HDCP_DBG_PRINT(ANSI_COLOR_YELLOW
						" :: HDCP RX encrypted and TX "
						"(1.4) auth : Stream1 -> "
						"Blanking On"
						ANSI_COLOR_RESET"\r\n");
					Status = XV_HdmiTxSs1_HdcpEnableBlank(
							DownstreamInstancePtr->HdmiTxSs);
					if (Status != XST_SUCCESS) {
						xdbg_printf(XDBG_DEBUG_GENERAL,
							"Error: Problem "
							"blocking downstream "
							"content.\r\n");
					}
				}
				break;
			}
		} else {
			EXDES_HDCP_DBG_PRINT(ANSI_COLOR_YELLOW" :: HDCP TX not "
					"authenticated -> Blanking "
					"On"ANSI_COLOR_RESET"\r\n");
			Status = XV_HdmiTxSs1_HdcpEnableBlank(
					DownstreamInstancePtr->HdmiTxSs);
			if (Status != XST_SUCCESS) {
				xdbg_printf(XDBG_DEBUG_GENERAL, "Error: Problem "
					"blocking downstream content.\r\n");
			}
		}
#ifdef XPAR_XV_HDMIRXSS1_NUM_INSTANCES
	} else {
		EXDES_HDCP_DBG_PRINT(ANSI_COLOR_YELLOW" :: HDCP RX not "
			"encrypted -> Blanking Off"ANSI_COLOR_RESET"\r\n");
		XV_HdmiTxSs1_HdcpDisableBlank(DownstreamInstancePtr->HdmiTxSs);
	}
#endif

}
#endif
#endif
#ifdef XPAR_XV_HDMITXSS1_NUM_INSTANCES
/*****************************************************************************/
/**
*
* This function is called from the TX State machine layer.
*
* @param  InstancePtr is the callback.
*
* @return None.
*
* @note   None.
*
******************************************************************************/
void XV_Tx_HdmiTrigCb_CableConnectionChange(void *InstancePtr)
{
	XV_Tx *txInst = (XV_Tx *)InstancePtr;
	XV_HdmiTxSs1 *HdmiTxSs1Ptr = (XV_HdmiTxSs1 *)txInst->HdmiTxSs;

	xhdmi_exdes_ctrlr.SystemEvent = FALSE;

	if (HdmiTxSs1Ptr->IsStreamConnected == (FALSE)) {
#if AUX_FIFO_CLEAR
		/* If the RX is present we assume that before the tx
		 * was disconnected the system was in pass-through.
		 */
		if (xhdmi_exdes_ctrlr.IsRxPresent == TRUE &&
		    xhdmi_exdes_ctrlr.ForceIndependent != TRUE) {
			/* Reset the AUX fifo. */
			ResetAuxFifo();
		}
#endif
		xhdmi_exdes_ctrlr.IsTxPresent = FALSE;
	} else {

		if (Exdes_CheckDwnstrmSinkCaps() == XST_SUCCESS) {
			XV_Tx_SetEdidParsingDone(xhdmi_exdes_ctrlr.hdmi_tx_ctlr,
						 TRUE);
			xil_printf("EDID Parsing Pass\r\n");

			if ((EdidHdmi_t.EdidCtrlParam.MaxFrlRateSupp > 0) &&
			    (EdidHdmi_t.EdidCtrlParam.IsSCDCPresent ==
			     XVIDC_SUPPORTED)) {
				/* System event has already been set to
				 * true by default.
				 * When the downstream supports FRL, we must
				 * complete FRL training first and then resume
				 * the system operations.
				 */
				XV_Tx_SetFrlEdidInfo(
					xhdmi_exdes_ctrlr.hdmi_tx_ctlr,
					EdidHdmi_t.EdidCtrlParam.IsSCDCPresent,
					EdidHdmi_t.EdidCtrlParam.MaxFrlRateSupp);
				xhdmi_exdes_ctrlr.SystemEvent = FALSE;
			}
		} else {
			XV_Tx_SetEdidParsingDone(xhdmi_exdes_ctrlr.hdmi_tx_ctlr,
						 FALSE);
			xil_printf("EDID Parsing Fails\r\n");
		}

		xhdmi_exdes_ctrlr.IsTxPresent = TRUE;
	}

	EXDES_DBG_PRINT("sysEventDebug:%s:%d:::%d\r\n", __func__,
			__LINE__, xhdmi_exdes_ctrlr.SystemEvent);
}

/*****************************************************************************/
/**
*
* This function is called from the TX State machine layer.
*
* @param  InstancePtr is the callback.
*
* @return None.
*
* @note   None.
*
******************************************************************************/
void XV_Tx_HdmiTrigCb_GetFRLClk(void *InstancePtr)
{
#if defined (XPAR_XV_HDMITXSS1_NUM_INSTANCES) && \
                 defined (XPAR_XV_HDMIRXSS1_NUM_INSTANCES)
	XV_Tx *XV_TxInst = (XV_Tx *)InstancePtr;
	u8 IsTx = xhdmi_exdes_ctrlr.IsTxPresent;
	u8 IsRx = xhdmi_exdes_ctrlr.IsRxPresent;

	u32 TotalPixFRLRatio;
	u32 ActivePixFRLRatio;
	u32 HdmiTxRefClkHz;

	if ((xhdmi_exdes_ctrlr.ForceIndependent == FALSE) && IsRx && IsTx) {
		ActivePixFRLRatio =
			XV_HdmiRx1_GetFrlActivePixRatio(HdmiRxSs.HdmiRx1Ptr) / 1000;
		TotalPixFRLRatio =
			XV_HdmiRx1_GetFrlTotalPixRatio(HdmiRxSs.HdmiRx1Ptr) / 1000;
		EXDES_DBG_PRINT("sysEventDebug:%s :: ActivePixFRLRatio = %d | ",
				__func__, ActivePixFRLRatio);
		EXDES_DBG_PRINT("TotalPixFRLRatio = %d | ",TotalPixFRLRatio);

		HdmiTxRefClkHz =
			(((u64)ActivePixFRLRatio * 450000) / TotalPixFRLRatio);
		EXDES_DBG_PRINT("HdmiTxRefClkHz = %d | ",HdmiTxRefClkHz);
		HdmiTxRefClkHz = HdmiTxRefClkHz*1000;
		EXDES_DBG_PRINT("HdmiTxRefClkHz*1000 = %d\r\n",HdmiTxRefClkHz);

		if (XV_TxInst->HdmiTxSs->HdmiTx1Ptr->Stream.TMDSClockRatio) {
			XV_TxInst->VidPhy->HdmiTxRefClkHz = HdmiTxRefClkHz >> 2;
		} else {
			XV_TxInst->VidPhy->HdmiTxRefClkHz = HdmiTxRefClkHz;
		}

		XV_TxInst->VidPhy->HdmiTxRefClkHz = HdmiTxRefClkHz;
	}
#endif
}

/*****************************************************************************/
/**
*
* This function is called from the TX State machine layer.
*
* @param  InstancePtr is the callback.
*
* @return None.
*
* @note   None.
*
******************************************************************************/
void XV_Tx_HdmiTrigCb_SetupTxFrlRefClk(void *InstancePtr)
{
	int Status;
#if (!defined XPS_BOARD_VEK280_ES)
	Status = XST_FAILURE;
	xil_printf("XV_Tx_HdmiTrigCb_SetupTxFrlRefClk\r\n");
	Status = Vfmc_Mezz_HdmiTxRefClock_Sel(&Vfmc[0],
			VFMC_MEZZ_TxRefclk_From_Si5344);
	XHdmiphy1_ClkDetFreqReset(&Hdmiphy1, 0, XHDMIPHY1_DIR_TX);
	if (Status == XST_FAILURE) {
		EXDES_DBG_PRINT("I2cClk " ANSI_COLOR_RED
				"Program Failure!\r\n" ANSI_COLOR_RESET);
	}
#endif

}

/*****************************************************************************/
/**
*
* This function is called from the TX State machine layer.
*
* @param  InstancePtr is the callback.
*
* @return None.
*
* @note   None.
*
******************************************************************************/
void XV_Tx_HdmiTrigCb_SetupTxTmdsRefClk(void *InstancePtr)
{
	int Status;

	Status = XST_FAILURE;
	XV_Tx *XV_TxInst = (XV_Tx *)InstancePtr;
	u8 IsTx = xhdmi_exdes_ctrlr.IsTxPresent;
	u8 IsRx = xhdmi_exdes_ctrlr.IsRxPresent;

#if (!defined XPS_BOARD_VEK280_ES)
	Status = Vfmc_Mezz_HdmiTxRefClock_Sel(&Vfmc[0],
			VFMC_MEZZ_TxRefclk_From_IDT);
#endif

#if 0
	/* Reset the AUX fifo so it is cleared of any previous TX Vsync upates,
	 * written either from Rx AUX in pass-through or set in Tx Only.
	 *
	 * This will ensure that from Tx clock starting to Tx Vsync the
	 * Aux Fifo remains empty.
	 */
	ResetAuxFifo();
#endif
	XV_Tx_SetFrlIntVidCkeGen(XV_TxInst);

	if ((xhdmi_exdes_ctrlr.ForceIndependent == FALSE) && IsRx && IsTx) {
		EXDES_DBG_PRINT("%s : triggering tx reference clock in "
				"pass-through. \r\n Tx Oversampling "
				"rate = %d.\r\n",__func__,
				Hdmiphy1.HdmiTxSampleRate);
		/* We use the Rx reference clock to set the clock alignment
		 * betweent the RX and the TX in pass-through mode.
		 * Starting the reference clock for the Hdmi Tx with the
		 * Hdmi Rx Reference clock and a multiplying factor
		 * of the Tx oversampling rate.
		 */

#if defined (XPAR_XV_FRMBUFWR_NUM_INSTANCES) && \
                      (XPAR_XV_FRMBUFRD_NUM_INSTANCES)
		if (XV_HdmiRxSs1_GetTransportMode(xhdmi_exdes_ctrlr.hdmi_rx_ctlr->HdmiRxSs) ==
				XV_HdmiTxSs1_GetTransportMode(xhdmi_exdes_ctrlr.hdmi_tx_ctlr->HdmiTxSs)) {
			Status = I2cClk(XV_TxInst->VidPhy->HdmiRxRefClkHz,
					(XV_TxInst->VidPhy->HdmiRxRefClkHz *
					 XV_TxInst->VidPhy->HdmiTxSampleRate));
		} else if (xhdmi_exdes_ctrlr.crop &&
				XV_HdmiRxSs1_GetTransportMode(xhdmi_exdes_ctrlr.hdmi_rx_ctlr->HdmiRxSs)) {
				Status = I2cClk(0,XV_TxInst->VidPhy->HdmiTxRefClkHz);

		} else {
			Status = I2cClk(0,XV_TxInst->VidPhy->HdmiTxRefClkHz);
		}
#else
		Status = I2cClk(XV_TxInst->VidPhy->HdmiRxRefClkHz,
				(XV_TxInst->VidPhy->HdmiRxRefClkHz *
				 XV_TxInst->VidPhy->HdmiTxSampleRate));
#endif
	} else if ((xhdmi_exdes_ctrlr.ForceIndependent == FALSE) && !IsRx && IsTx) {
		EXDES_DBG_PRINT("%s : triggering tx reference "
				"clock in colorbar \r\n", __func__);
		/* Program external clock generator in free running mode */
		Status = I2cClk(0, XV_TxInst->VidPhy->HdmiTxRefClkHz);
	} else if (xhdmi_exdes_ctrlr.ForceIndependent == TRUE) {
		EXDES_DBG_PRINT(ANSI_COLOR_YELLOW "%s : triggering tx reference "
				"clock independently from rx, %d \r\n"
				ANSI_COLOR_RESET, __func__,
				XV_TxInst->VidPhy->HdmiTxRefClkHz);
		/* Program external clock generator in free running mode */
		Status = I2cClk(0, XV_TxInst->VidPhy->HdmiTxRefClkHz);
	} else {
		EXDES_DBG_PRINT("%s,%d : No Transmitter present, cannot "
				"program clock !!", __func__, __LINE__);
	}

	if (Status == XST_FAILURE) {
		EXDES_DBG_PRINT("I2cClk " ANSI_COLOR_RED
				"Program Failure!\r\n" ANSI_COLOR_RESET);
	} else {
		usleep(1000000);
	}
}

/*****************************************************************************/
/**
*
* This function is called from the TX State machine layer.
*
* @param  InstancePtr is the callback.
*
* @return None.
*
* @note   None.
*
******************************************************************************/
void XV_Tx_HdmiTrigCb_StreamOff(void *InstancePtr)
{
#if defined (XPAR_XV_HDMITXSS1_NUM_INSTANCES) && \
                 defined (XPAR_XV_HDMIRXSS1_NUM_INSTANCES)
	u8 IsTx = xhdmi_exdes_ctrlr.IsTxPresent;
	u8 IsRx = xhdmi_exdes_ctrlr.IsRxPresent;

	if (IsRx && IsTx) {
#if AUX_FIFO_CLEAR
		/* Reset the AUX fifo. */
		ResetAuxFifo();
#endif
		XV_HdmiRxSs1_VRST(&HdmiRxSs, TRUE);
	}
#endif
/**
 *	Additionally, the AVI Info frame and Vendor specific info frame
 *	cane also be reset here,
 *	XHdmiC_AVI_InfoFrame *AviInfoFramePtr;
 *	XHdmiC_VSIF *VSIFPtr;
 *	AviInfoFramePtr = XV_HdmiTxSs1_GetAviInfoframe(&HdmiTxSs);
 *	VSIFPtr = XV_HdmiTxSs1_GetVSIF(&HdmiTxSs);
 *	/\* Reset Avi InfoFrame *\/
 *	(void)memset((void *)AviInfoFramePtr, 0, sizeof(XHdmiC_AVI_InfoFrame));
 *	/\* Reset Vendor Specific InfoFrame *\/
 *	(void)memset((void *)VSIFPtr, 0, sizeof(XHdmiC_VSIF));
 */

}

/*****************************************************************************/
/**
*
* This function is called from the TX State machine layer.
*
* @param  InstancePtr is the callback.
*
* @return None.
*
* @note   None.
*
******************************************************************************/
void XV_Tx_HdmiTrigCb_SetupAudioVideo(void *InstancePtr)
{
	XVidC_VideoStream *HdmiTxSsVidStreamPtr;
#if defined(XPAR_XV_HDMIRXSS1_NUM_INSTANCES)
	u8 IsTx = xhdmi_exdes_ctrlr.IsTxPresent;
	u8 IsRx = xhdmi_exdes_ctrlr.IsRxPresent;
#endif
	XHdmiC_AVI_InfoFrame *AviInfoFramePtr;
	XHdmiC_VSIF *VSIFPtr;

	HdmiTxSsVidStreamPtr = XV_HdmiTxSs1_GetVideoStream(&HdmiTxSs);
#if AUX_FIFO_CLEAR
	/* Get the AVI Info frame and Vendor specific
	 * info-frame and reset them.
	 */
	AviInfoFramePtr = XV_HdmiTxSs1_GetAviInfoframe(&HdmiTxSs);
	VSIFPtr = XV_HdmiTxSs1_GetVSIF(&HdmiTxSs);
	/* Reset Avi InfoFrame */
	(void)memset((void *)AviInfoFramePtr, 0, sizeof(XHdmiC_AVI_InfoFrame));
	/* Reset Vendor Specific InfoFrame */
	(void)memset((void *)VSIFPtr, 0, sizeof(XHdmiC_VSIF));
#endif

#ifdef XPAR_XV_TPG_NUM_INSTANCES
	/* Set colorbar pattern */
	Pattern = XTPG_BKGND_COLOR_BARS;
#endif
	xhdmi_exdes_ctrlr.TxBusy = TRUE;

#if defined(XPAR_XV_HDMIRXSS1_NUM_INSTANCES)
	if (xhdmi_exdes_ctrlr.ForceIndependent == TRUE) {
#endif
		Exdes_UpdateAviInfoFrame(HdmiTxSsVidStreamPtr);
		Exdes_ConfigureTpgEnableInput(FALSE);
		Exdes_AudioConfig_Colorbar();
#if defined(XPAR_XV_HDMIRXSS1_NUM_INSTANCES)
	} else {
		if (IsRx && IsTx) {
			if (xhdmi_exdes_ctrlr.crop &&
					!XV_HdmiTxSs1_GetTransportMode(&HdmiTxSs)) {
				Exdes_UpdateAviInfoFrame(HdmiTxSsVidStreamPtr);
			} else {
				Exdes_CopyRxAVIInfoFrameToTx();
			}
			Exdes_ConfigureTpgEnableInput(TRUE);
			Exdes_AudioConfig_Passthru();
		} else if (!IsRx && IsTx) {
			Exdes_UpdateAviInfoFrame(HdmiTxSsVidStreamPtr);
			Exdes_ConfigureTpgEnableInput(FALSE);
			Exdes_AudioConfig_Colorbar();
		}
	}
#endif
}

/*****************************************************************************/
/**
*
* This function is called from the TX State machine layer.
*
* @param  InstancePtr is the callback.
*
* @return None.
*
* @note   None.
*
******************************************************************************/
void XV_Tx_HdmiTrigCb_StreamOn(void *InstancePtr)
{
	XVidC_VideoStream *HdmiTxSsVidStreamPtr;
#if defined(XPAR_XV_HDMIRXSS1_NUM_INSTANCES)
	u8 IsTx = xhdmi_exdes_ctrlr.IsTxPresent;
	u8 IsRx = xhdmi_exdes_ctrlr.IsRxPresent;
#endif
#if defined(XPAR_XV_HDMIRXSS1_NUM_INSTANCES)
	if (xhdmi_exdes_ctrlr.ForceIndependent == TRUE) {
		/* Mute the RX after loopback to avoid any push-back
		 * from the TPG as RX data is not forwarded to TX. */
		EXDES_DBG_PRINT("%s,%d: Rx VRST - true \r\n",
				__func__, __LINE__);
#if defined (XPAR_XV_FRMBUFWR_NUM_INSTANCES) && \
                      (XPAR_XV_FRMBUFRD_NUM_INSTANCES)
#else
		XVidC_VideoStream *HdmiRxSsVidStreamPtr;
		HdmiRxSsVidStreamPtr = XV_HdmiRxSs1_GetVideoStream(&HdmiRxSs);
		if(HdmiRxSsVidStreamPtr->IsDSCompressed) {
			XV_HdmiRxSs1_VRST(&HdmiRxSs, FALSE);
		} else {
			XV_HdmiRxSs1_VRST(&HdmiRxSs, TRUE);
		}
#endif
		xil_printf("Tx stream is up independently from Rx \r\n");
		xil_printf("--------\r\nIndependent TX :\r\n");
	} else {
		if (IsRx && IsTx) {
			/* Un-mute the RX after TX comes up in pass-through. */
			EXDES_DBG_PRINT("%s,%d : Rx VRST - false \r\n",
					__func__, __LINE__);

#if defined (XPAR_XV_FRMBUFWR_NUM_INSTANCES) && \
            (XPAR_XV_FRMBUFRD_NUM_INSTANCES)
			xil_printf("%s,%d: Config Video Frame Buffer - "
				   "True \r\n", __func__, __LINE__);
			/* Config and Run the Video Frame Buffer */
			XV_ConfigVidFrameBuf_rd(&FrameBufRd);
#endif
			XV_HdmiRxSs1_VRST(&HdmiRxSs, FALSE);

			xil_printf("Tx stream is up in pass-through \r\n");
			xil_printf("--------\r\nPass-through :\r\n");
		} else if (!IsRx && IsTx) {
#endif
			xil_printf("Tx stream is up in colorbar \r\n");
			xil_printf("--------\r\nColorbar :\r\n");
#if defined(XPAR_XV_HDMIRXSS1_NUM_INSTANCES)
		}
	}
#endif
	HdmiTxSsVidStreamPtr = XV_HdmiTxSs1_GetVideoStream(&HdmiTxSs);
	XVidC_ReportStreamInfo(HdmiTxSsVidStreamPtr);
	if (HdmiTxSs.HdmiTx1Ptr->Stream.IsFrl == TRUE) {
		xil_printf("\tTX FRL Rate:              %d lanes @ %d Gbps\r\n",
				HdmiTxSs.HdmiTx1Ptr->Stream.Frl.Lanes,
				HdmiTxSs.HdmiTx1Ptr->Stream.Frl.LineRate);
	} else {
		xil_printf("\tTX     Mode:              TMDS\r\n");
	}
#if defined(XPAR_XV_HDMIRXSS1_NUM_INSTANCES)
	if ((HdmiRxSs.HdmiRx1Ptr->Stream.IsFrl == TRUE) &&
		(IsRx && IsTx) && (xhdmi_exdes_ctrlr.ForceIndependent == FALSE)) {
		xil_printf("\tRX FRL Rate:              %d lanes @ %d Gbps\r\n",
				HdmiRxSs.HdmiRx1Ptr->Stream.Frl.Lanes,
				HdmiRxSs.HdmiRx1Ptr->Stream.Frl.LineRate);
	}
#endif
	xil_printf("--------\r\n");

	xhdmi_exdes_ctrlr.TxBusy = FALSE;
	VsyncCounter = 0;
}

/*****************************************************************************/
/**
*
* This function is called from the TX State machine layer.
*
* @param  InstancePtr is the callback.
*
* @return None.
*
* @note   None.
*
******************************************************************************/
void XV_Tx_HdmiTrigCb_VidSyncRecv(void *InstancePtr)
{
#if defined(XPAR_XV_HDMIRXSS1_NUM_INSTANCES)
	u8 IsTx = xhdmi_exdes_ctrlr.IsTxPresent;
	u8 IsRx = xhdmi_exdes_ctrlr.IsRxPresent;
#endif
	XV_Tx *XV_TxInst = (XV_Tx *)InstancePtr;

	/* Check whether the sink is DVI/HDMI Supported
	 * If the sink is DVI, don't send Info-frame
	 */
	if (EdidHdmi_t.EdidCtrlParam.IsHdmi == XVIDC_ISHDMI &&
	    (XV_TxInst->HdmiTxSs->HdmiTx1Ptr->Stream.IsHdmi == TRUE)) {
#if defined(XPAR_XV_HDMIRXSS1_NUM_INSTANCES)
		if ((xhdmi_exdes_ctrlr.ForceIndependent == TRUE) ||
		    (!IsRx && IsTx)) {
#endif
			Exdes_SendInfoFrame_Colorbar();
#if defined(XPAR_XV_HDMIRXSS1_NUM_INSTANCES)
		} else if (IsRx && IsTx) {
			Exdes_SendInfoFrame_Passthru();
		}
#endif
	} else {
		/* To ensure that the downstream is DVI, or never transitions
		 * to HDMI use the following print to check.
		 * Additonally, re-check the videomode of the stream here.
		 * EXDES_DBG_PRINT("%s,%d. Downstream is not HDMI !!\r\n",
		 *		__func__, __LINE__);
		 */
	}

	VsyncCounter++;
}

/*****************************************************************************/
/**
*
* This function is called from the TX State machine layer.
*
* @param  InstancePtr is the callback.
*
* @return None.
*
* @note   None.
*
******************************************************************************/
void XV_Tx_HdmiTrigCb_EnableCableDriver(void *InstancePtr)
{
	XV_Tx *XV_TxInst = (XV_Tx *)InstancePtr;
	u64 TxLineRate;
	u8 TxDiffSwingVal;

	TxLineRate = XV_Tx_GetLineRate(XV_TxInst);

	u8 Lanes = XV_TxInst->HdmiTxSs->HdmiTx1Ptr->Stream.Frl.Lanes;

	if (SinkReadyCheck(XV_TxInst->HdmiTxSs, &EdidHdmi_t)) {
		EXDES_DBG_PRINT("Setting Cable Driver, TxLineRate = %d%d\r\n",
				(u32)(TxLineRate >> 32), (u32)TxLineRate);

		/* if hdmi mode is TMDS then we need to configure the
		 * tx fmc driver for each resolution, othersie for FRL
		 * tx fmc drivers will be configured in FRL config.
		 */
		if (XV_HdmiTxSs1_GetTransportMode(XV_TxInst->HdmiTxSs) ==
				FALSE) {
			Vfmc_Gpio_Mezz_HdmiTxDriver_Reconfig(&Vfmc[0],
							     FALSE,
							     TxLineRate, Lanes);

#if defined (XPS_BOARD_ZCU106) || \
	defined (XPS_BOARD_VCU118) || \
	defined (XPS_BOARD_ZCU102) || \
	defined (XPS_BOARD_VCK190) || \
	defined (XPS_BOARD_VEK280_ES)
			/* Adjust GT TX Diff Swing based on Line rate */
			if (Vfmc[0].TxMezzType >= VFMC_MEZZ_HDMI_ONSEMI_R0 &&
				Vfmc[0].TxMezzType <  VFMC_MEZZ_INVALID) {
				/*Convert Line Rate to Mbps */
				TxLineRate = (u32)((u64) TxLineRate / 1000000);

				/* HDMI 2.0 */
				if ((TxLineRate >= 3400) &&
				    (TxLineRate < 6000)) {
					if (Vfmc[0].TxMezzType ==
						VFMC_MEZZ_HDMI_ONSEMI_R0) {
						/* Set Tx Diff Swing to
						 * 963 mV */
						TxDiffSwingVal = 0x1F;
					}
					else if (Vfmc[0].TxMezzType >=
						VFMC_MEZZ_HDMI_ONSEMI_R1) {
						/* Set Tx Diff Swing to
						 * 1000 mV */
						TxDiffSwingVal = 0x1F;
					}
				}
				/* HDMI 1.4 1.65-3.4 Gbps */
				else if ((TxLineRate >= 1650) &&
				         (TxLineRate < 3400)) {
					/* Set Tx Diff Swing to 1000 mV */
					TxDiffSwingVal = 0x17;
				}
				/* HDMI 1.4 0.25-1.65 Gbps */
				else {
					/* Set Tx Diff Swing to 822 mV */
					TxDiffSwingVal = 0x17;
				}

				for (int ChId=1; ChId <= 4; ChId++) {
					XHdmiphy1_SetTxVoltageSwing(&Hdmiphy1,
								0,
								ChId,
								TxDiffSwingVal);
				}
			}
#endif
		}
	}
}

/*****************************************************************************/
/**
*
* This function is called from the TX State machine layer.
*
* @param  InstancePtr is the callback.
*
* @return None.
*
* @note   None.
*
******************************************************************************/
void XV_Tx_HdmiTrigCb_ReadyToStartTransmit(void *InstancePtr)
{
	xhdmi_exdes_ctrlr.SystemEvent = TRUE;
}

/*****************************************************************************/
/**
*
* This function is called from the TX State machine layer.
*
* @param  InstancePtr is the callback.
*
* @return None.
*
* @note   None.
*
******************************************************************************/
void XV_Tx_HdmiTrigCb_FrlFfeConfigDevice(void *InstancePtr)
{
	/* Nothing to be done here.
	 * This function is available as a place holder
	 * for the users to configure the on board or
	 * external mezzanine cards for HDMI 2.1
	 * during the FRL ffe training.
	 */
}

/*****************************************************************************/
/**
*
* This function is called from the TX State machine layer.
*
* @param  InstancePtr is the callback.
*
* @return None.
*
* @note   None.
*
******************************************************************************/
void XV_Tx_HdmiTrigCb_FrlConfigDeviceSetup(void *InstancePtr)
{
	u8 Data = 0;

	EXDES_DBG_PRINT("sysEventDebug:%s,%d: Setting device configurations "
			"at Frl Config.\r\n", __func__, __LINE__);
	XV_Tx *XV_TxInst = (XV_Tx *)InstancePtr;

	u64 LineRate =
		((u64)(XV_TxInst->HdmiTxSs->HdmiTx1Ptr->Stream.Frl.LineRate)) *
			((u64)(1e9));

	u8 Lanes = XV_TxInst->HdmiTxSs->HdmiTx1Ptr->Stream.Frl.Lanes;

	Vfmc_Gpio_Mezz_HdmiTxDriver_Reconfig(&Vfmc[0],
					     TRUE,
					     LineRate, Lanes);

	/* Adjust GT TX Diff Swing based on Mode */
	for (int ChId=1; ChId <= 4; ChId++) {
		if (Vfmc[0].TxMezzType == VFMC_MEZZ_HDMI_ONSEMI_R0) {
#if defined (XPS_BOARD_ZCU102) || \
	defined (XPS_BOARD_ZCU106) || \
	defined (XPS_BOARD_VCK190) || \
	defined (XPS_BOARD_VEK280_ES)
			Data = 0xD;
#elif defined (XPS_BOARD_VCU118)
			Data = ChId==4 ? 0x1C : 0x1A;
#endif
		} else if (Vfmc[0].TxMezzType == VFMC_MEZZ_HDMI_ONSEMI_R1) {
#if defined (XPS_BOARD_ZCU106)
			/* Set TxDiffSwing to 1000 mV for all channels */
			Data = 0xF;
#elif defined (XPS_BOARD_VCU118)
			Data = ChId==4 ? 0x1C : 0x1A;
#elif defined (XPS_BOARD_ZCU102)
			Data = 0xD;
#elif defined (XPS_BOARD_VCK190)
			Data = 0xD;
#endif
		} else if (Vfmc[0].TxMezzType >= VFMC_MEZZ_HDMI_ONSEMI_R2) {
#if defined (XPS_BOARD_ZCU106)
//			if ((ChId == 2) || (ChId == 3)) {
				Data = 0xD;
//			} else {
//				Data = 0xA;
//			}
#elif defined (XPS_BOARD_VCU118)
			Data = 0xD; //0x1F;
#elif defined (XPS_BOARD_ZCU102)
			Data = 0xD;
#elif defined (XPS_BOARD_VCK190)
			Data = 0xD;
#elif defined (XPS_BOARD_VEK280_ES)
			Data = 0xD;
#endif
		}
#if defined (XPS_BOARD_ZCU106) || \
	defined (XPS_BOARD_VCU118) || \
	defined (XPS_BOARD_ZCU102) || \
	defined (XPS_BOARD_VCK190) || \
	defined (XPS_BOARD_VEK280_ES)
		XHdmiphy1_SetTxVoltageSwing(&Hdmiphy1, 0, ChId, Data);
#endif
	}
}

/*****************************************************************************/
/**
*
* This function is used to blank out the outgoing data by enabling
* blanking in the HDCP cipher.
*
* @param  InstancePtr is the callback.
*
* @return None.
*
* @note   None.
*
******************************************************************************/
void XV_Tx_Hdcp_EnforceBlanking(void *InstancePtr)
{
#if defined (XPAR_XV_HDMITXSS1_NUM_INSTANCES) && \
                 defined (XPAR_XV_HDMIRXSS1_NUM_INSTANCES)
#if defined(USE_HDCP_HDMI_RX) || defined(USE_HDCP_HDMI_TX)
	/**
	 * Alternatively, we can recover the xhdmi_exdes_ctlr
	 * from the callback by ensuring it is passed as the
	 * reference pointer,
	 * XHdmi_Exdes *xhdmi_exdes_ctrlr = (XHdmi_Exdes *)InstancePtr;
	 */

	XHdcp_EnforceBlanking(xhdmi_exdes_ctrlr.hdmi_rx_ctlr,
			xhdmi_exdes_ctrlr.hdmi_tx_ctlr);
#endif
#endif
}
#endif

#ifdef XPAR_XV_HDMIRXSS1_NUM_INSTANCES
/*****************************************************************************/
/**
*
* This function is used to toggle RX's HPD Line
*
* @param  Hdmiphy1Ptr is a pointer to the VPHY instance.
* @param  HdmiRxSs1Ptr is a pointer to the HDMI RX Subsystem instance.
*
* @return None.
*
* @note   None.
*
******************************************************************************/
void ToggleHdmiRxHpd(XHdmiphy1 *Hdmiphy1Ptr, XV_HdmiRxSs1 *HdmiRxSs1Ptr)
{
	SetHdmiRxHpd(Hdmiphy1Ptr, HdmiRxSs1Ptr, FALSE);
	/* Wait 500 ms */
	usleep(500000);
	SetHdmiRxHpd(Hdmiphy1Ptr, HdmiRxSs1Ptr, TRUE);
}

/*****************************************************************************/
/**
*
* This function sets the HPD on the HDMI RXSS.
*
* @param  Hdmiphy1Ptr is a pointer to the VPHY instance.
* @param  HdmiRxSs1Ptr is a pointer to the HDMI RX Subsystem instance.
* @param  Hpd is a flag used to set the HPD.
*   - TRUE drives HPD high
*   - FALSE drives HPD low
*
* @return None.
*
* @note   None.
*
******************************************************************************/
void SetHdmiRxHpd(XHdmiphy1 *Hdmiphy1Ptr, XV_HdmiRxSs1 *HdmiRxSs1Ptr, u8 Hpd)
{
	if (Hpd == TRUE) {
		XV_HdmiRxSs1_SetHpd(HdmiRxSs1Ptr, Hpd);
		XHdmiphy1_IBufDsEnable(Hdmiphy1Ptr, 0,
				       XHDMIPHY1_DIR_RX, (TRUE));
	} else {
		XHdmiphy1_MmcmPowerDown(Hdmiphy1Ptr, 0,
					XHDMIPHY1_DIR_RX, FALSE);
		XHdmiphy1_Clkout1OBufTdsEnable(Hdmiphy1Ptr,
					       XHDMIPHY1_DIR_RX, (FALSE));
		XHdmiphy1_IBufDsEnable(Hdmiphy1Ptr, 0,
				       XHDMIPHY1_DIR_RX, (FALSE));
		XV_HdmiRxSs1_SetHpd(HdmiRxSs1Ptr, Hpd);
	}
}
#endif /* XPAR_XV_HDMIRXSS1_NUM_INSTANCES */

#ifdef XPAR_XV_HDMIRXSS1_NUM_INSTANCES
/*****************************************************************************/
/**
*
* This function is called from the RX State machine layer.
*
* @param  InstancePtr is the callback reference.
*
* @return None.
*
* @note   None.
*
******************************************************************************/
void XV_Rx_HdmiTrigCb_CableConnectionChange(void *InstancePtr)
{
	XV_HdmiRxSs1 *HdmiRxSs1Ptr = (XV_HdmiRxSs1 *)&HdmiRxSs;

	/* RX cable is disconnected */
	if (HdmiRxSs1Ptr->IsStreamConnected == (FALSE)) {
		xhdmi_exdes_ctrlr.crop = FALSE;
		/* Push Rx Connect to the state machine. */
		/* Check if the system might be in pass-through or loopback
		 * mode, i.e. there is a downstream connection present. */
		if (xhdmi_exdes_ctrlr.IsTxPresent) {
#if 0
			/* Reset the AUX fifo. */
			ResetAuxFifo();
#endif
		}

		xhdmi_exdes_ctrlr.IsRxPresent = FALSE;
		xhdmi_exdes_ctrlr.SystemEvent = TRUE;
		xil_printf("XV_Rx_HdmiTrigCb_CableConnectionChange - "
			   "Disonnected\r\n");
	} else {
		xhdmi_exdes_ctrlr.crop = FALSE;

		/* Reset the menu to main */
		XHdmi_MenuReset(&HdmiMenu);

		/* Mute/Stop the HDMI RX SS Output
		 * Enable only when the downstream is ready
		 * (TPG & HDMI TX)
		 */
		EXDES_DBG_PRINT("%s,%d: Rx VRST - true \r\n",
				__func__, __LINE__);
		XV_HdmiRxSs1_VRST(&HdmiRxSs, TRUE);
#if 0
		EXDES_AUXFIFO_DBG_PRINT("%s,%d: Aux Fifo Reset \r\n",
					__func__, __LINE__);
		ResetAuxFifo();
#endif
		/* We will push the RX "Connect" on RX stream up so that
		 * the pass-through system can stabilize when Rx connect and
		 * Tx connect happen quickly and back-to-back creating
		 * race conditions.
		 */
	}

	/* EXDES_DBG_PRINT("sysEventDebug:%s:%d:::%d\r\n", __func__,
	 *		   __LINE__, xhdmi_exdes_ctrlr.SystemEvent);
	 */
}

/*****************************************************************************/
/**
*
* This function is called from the RX State machine layer.
*
* @param  InstancePtr is the callback reference.
*
* @return None.
*
* @note   None.
*
******************************************************************************/
void XV_Rx_HdmiTrigCb_AudioConfig(void *InstancePtr)
{
#if defined (XPAR_XV_HDMITXSS1_NUM_INSTANCES)
	XV_HdmiTx1_AudioFormatType AudioFormat;

	/* Configure the TX audio with RX audio configuration
	 * during PassThrough mode */
	if (xhdmi_exdes_ctrlr.ForceIndependent == FALSE) {
		/* Get the audio format from Hdmi Rx */
		/* HBR audio */
		if (XV_HdmiRxSs1_GetAudioFormat(&HdmiRxSs) ==
				XV_HDMIRX1_AUDFMT_HBR) {
			AudioFormat = XV_HDMITX1_AUDFMT_HBR;
		} else {
			AudioFormat = XV_HDMITX1_AUDFMT_LPCM;
		}

		/* Set the Audio format and channels. */
		XV_HdmiTxSs1_SetAudioChannels(&HdmiTxSs,
				XV_HdmiRxSs1_GetAudioChannels(&HdmiRxSs));
		XV_HdmiTxSs1_SetAudioFormat(&HdmiTxSs,
				AudioFormat);
	}
#endif
}

/*****************************************************************************/
/**
*
* This function is called from the RX State machine layer.
*
* @param  InstancePtr is the callback reference.
*
* @return None.
*
* @note   None.
*
******************************************************************************/
void XV_Rx_HdmiTrigCb_AuxEvent(void *InstancePtr)
{
	/* Check for pass-through */
	if (xhdmi_exdes_ctrlr.IsTxPresent &&
		(xhdmi_exdes_ctrlr.ForceIndependent == FALSE)) {
		/* For profiling AUX callbacks in pass-through,
		 * EXDES_DBG_PRINT("%s,%d(Updating AUX Fifo)\r\n",
		 *		__func__, __LINE__);
		 */
		Exdes_UpdateAuxFifo();
	}

	/* To profile the AuxCounter use,
	 *  if (HdmiRxSs.HdmiRx1Ptr->Aux.Header.Byte[0] ==
	 *      AUX_GENERAL_CONTROL_PACKET_TYPE) {
	 *	AuxCounter--;
	 *	xil_printf("R%d\r\n", AuxCounter);
	 *  }
	 */

	AuxCounter++;
}

/*****************************************************************************/
/**
*
* This function is called from the RX State machine layer.
*
* @param  InstancePtr is the callback reference.
*
* @return None.
*
* @note   None.
*
******************************************************************************/
void XV_Rx_HdmiTrigCb_StreamOn(void *InstancePtr)
{
	xhdmi_exdes_ctrlr.IsRxPresent = TRUE;
	u8 PrintRxInfo = FALSE;
	xhdmi_exdes_ctrlr.crop = FALSE;

	/* If the system is in independent mode, then stream up on
	 * the RX should not affect the restart of the TX at all.
	 * In loop back mode the RX programming ends here.
	 */
	if (xhdmi_exdes_ctrlr.ForceIndependent != TRUE) {
		xhdmi_exdes_ctrlr.SystemEvent = TRUE;
	} else {
		/* Mute any RX stream.
		 * Important to do this here - without this, we will get
		 * overwhelmed with RX Bridge Overflow interrupts. */
		XV_HdmiRxSs1_VRST(&HdmiRxSs, TRUE);
	}

	XVidC_VideoStream *HdmiRxSsVidStreamPtr;
	u8 IsTx = xhdmi_exdes_ctrlr.IsTxPresent;
	u8 IsRx = xhdmi_exdes_ctrlr.IsRxPresent;

	HdmiRxSsVidStreamPtr = XV_HdmiRxSs1_GetVideoStream(&HdmiRxSs);

	/* If incoming stream is DSC then Select M01 AXI else
	 * Select M00 AXI.
	 */
#if defined	(XPAR_V_HDMI_RXSS1_DSC_EN) && \
		defined (XPAR_XAXIS_SWITCH_NUM_INSTANCES)
	XAxisScr_RegUpdateDisable(&HdmiRxAxiSwitch);
	if(HdmiRxSsVidStreamPtr->IsDSCompressed) {
		XAxisScr_MiPortDisable(&HdmiRxAxiSwitch,0);
		XAxisScr_MiPortEnable(&HdmiRxAxiSwitch,1,0);

	} else {
		XAxisScr_MiPortDisable(&HdmiRxAxiSwitch,1);
		XAxisScr_MiPortEnable(&HdmiRxAxiSwitch,0,0);
		xil_printf("DSC Disabled \r\n");
	}
	XAxisScr_RegUpdateEnable(&HdmiRxAxiSwitch);

#endif

	if (IsRx && !IsTx) {
		xil_printf("--------\r\nRX Only :\r\n");
		PrintRxInfo = TRUE;
#if defined (XPAR_XV_HDMITXSS1_NUM_INSTANCES)
		/* Update the Tx stream with the new RX stream if the
		 * application is in pass-through mode.
		 * In case of pass-through transition being true, i.e.,
		 * RX and TX are both up, this will happen when the
		 * SystemEvent is recovered in the while (1) loop in main,
		 * so need only do this for the RX-Only condition.
		 */
		if (xhdmi_exdes_ctrlr.ForceIndependent != TRUE) {
			Exdes_CopyRxVidParamstoTx(
				xhdmi_exdes_ctrlr.hdmi_rx_ctlr->HdmiRxSs,
				xhdmi_exdes_ctrlr.hdmi_tx_ctlr->HdmiTxSs);
		}
#endif
	} else if (IsRx && IsTx && (xhdmi_exdes_ctrlr.ForceIndependent == TRUE)) {
		xil_printf("--------\r\nIndependent RX :\r\n");
		xhdmi_exdes_ctrlr.SystemEvent = FALSE;
		PrintRxInfo = TRUE;
	} else {
#if (TX_RX_RATE == 1)
		if (EdidHdmi_t.EdidCtrlParam.MaxFrlRateSupp >=
				HdmiRxSs.HdmiRx1Ptr->Stream.Frl.CurFrlRate) {
		    XV_HdmiTxSs1_StartFrlTraining(&HdmiTxSs,
				HdmiRxSs.HdmiRx1Ptr->Stream.Frl.CurFrlRate);
		} else {
			xil_printf (ANSI_COLOR_YELLOW"WARNING: "
					"Tx does not support more than %d rate\r\n"ANSI_COLOR_RESET"\r\n",
										EdidHdmi_t.EdidCtrlParam.MaxFrlRateSupp);
			XV_HdmiTxSs1_StartFrlTraining(&HdmiTxSs,
				    EdidHdmi_t.EdidCtrlParam.MaxFrlRateSupp);
		}
#endif
		xil_printf("RX stream is up.\r\n");
		PrintRxInfo = FALSE;
	}

	if (PrintRxInfo == TRUE) {
		XVidC_ReportStreamInfo(HdmiRxSsVidStreamPtr);
		if (HdmiRxSs.HdmiRx1Ptr->Stream.IsFrl == TRUE) {
			xil_printf("\tRX FRL Rate:              %d lanes @ %d Gbps\r\n",
					HdmiRxSs.HdmiRx1Ptr->Stream.Frl.Lanes,
					HdmiRxSs.HdmiRx1Ptr->Stream.Frl.LineRate);
		}
		xil_printf("--------\r\n");
	}
#if (VRR_MODE ==1) // Manual Mode
	/* Enable VFP Change Interuppt */
	XV_HdmiRxSs1_VfpControl(&HdmiRxSs, TRUE);
#endif

#if defined (XPAR_XV_FRMBUFWR_NUM_INSTANCES) && \
                      (XPAR_XV_FRMBUFRD_NUM_INSTANCES)
	//Start FB write and consume
	XV_ConfigVidFrameBuf_wr(&FrameBufWr);
	XV_HdmiRxSs1_VRST(&HdmiRxSs, FALSE);

	//Determined that xhdmi_exdes_ctrlr.crop is enabled
	//update the timing params into TX only if
	// TX is non FRL
	if (xhdmi_exdes_ctrlr.crop && IsTx && HdmiTxSs.HdmiTx1Ptr->Stream.IsFrl != TRUE) {
		if (xhdmi_exdes_ctrlr.ForceIndependent != TRUE) {
			Exdes_UpdateVidParamstoTx(
					xhdmi_exdes_ctrlr.hdmi_rx_ctlr->HdmiRxSs,
					xhdmi_exdes_ctrlr.hdmi_tx_ctlr->HdmiTxSs);
		}
	}
#else
	xhdmi_exdes_ctrlr.crop = FALSE;
	if(HdmiRxSsVidStreamPtr->IsDSCompressed)
		XV_HdmiRxSs1_VRST(&HdmiRxSs, FALSE);
#endif
	EXDES_DBG_PRINT("sysEventDebug:%s:%d:::%d\r\n", __func__,
			__LINE__, xhdmi_exdes_ctrlr.SystemEvent);
}
/*****************************************************************************/
/**
*
* This function is called from the RX State machine layer.
*
* @param  InstancePtr is the callback reference.
*
* @return None.
*
* @note   None.
*
******************************************************************************/
void XV_Rx_HdmiTrigCb_VrrVfpEvent(void *InstancePtr)
{
#if defined (XPAR_XV_HDMITXSS1_NUM_INSTANCES)
	if (xhdmi_exdes_ctrlr.IsTxPresent &&
		(xhdmi_exdes_ctrlr.ForceIndependent == FALSE) &&
		(xhdmi_exdes_ctrlr.TxStartTransmit == FALSE) &&
		(HdmiTxSs.IsStreamUp == TRUE)) {
		Exdes_ProcessVRRTimingChange();
	} else {
		Exdes_ReadVRRTimingChange();
	}
#else
	Exdes_ReadVRRTimingChange();
#endif

//	 xil_printf(" App : VFP Changed \r\n");
}
/*****************************************************************************/
/**
*
* This function is called from the RX State machine layer.
*
* @param  InstancePtr is the callback reference.
*
* @return None.
*
* @note   None.
*
******************************************************************************/
void XV_Rx_HdmiTrigCb_VtemEvent(void *InstancePtr)
{
#if defined (XPAR_XV_HDMITXSS1_NUM_INSTANCES)
	if (xhdmi_exdes_ctrlr.IsTxPresent &&
		(xhdmi_exdes_ctrlr.ForceIndependent == FALSE) &&
		(xhdmi_exdes_ctrlr.TxStartTransmit == FALSE) &&

		(HdmiTxSs.IsStreamUp == TRUE)) {
		Exdes_ProcessVTEMPacket();
	} else {
		Exdes_ReadVTEMPacket();
	}
#else
	Exdes_ReadVTEMPacket();
#endif
}

#if defined (XPAR_XV_FRMBUFWR_NUM_INSTANCES) && \
                      (XPAR_XV_FRMBUFRD_NUM_INSTANCES)
/*****************************************************************************/
/**
*
* This function is called from the RX State machine layer.
*
* @param  InstancePtr is the callback reference.
*
* @return None.
*
* @note   None.
******************************************************************************/
void XV_Rx_HdmiTrigCb_DynHdrEvent(void *InstancePtr)
{

	XV_HdmiRxSs1_DynHDR_GetInfo(&HdmiRxSs,&RX_DynHDR_Info);

	DynHdr_rd_buff_offset = DynHdr_wr_buff_offset;
	if(DynHdr_wr_buff_offset == 3)
		DynHdr_wr_buff_offset = 0;
	else
		DynHdr_wr_buff_offset++;

/*	if(DynHdr_rd_buff_offset == 3)
		DynHdr_rd_buff_offset = 0;
	else
		DynHdr_rd_buff_offset++;
*/
	XV_HdmiRxSs1_DynHDR_SetAddr(&HdmiRxSs,VidBuff[DynHdr_wr_buff_offset].DynHDRBaseAddr);

	if (xhdmi_exdes_ctrlr.IsTxPresent &&
		(xhdmi_exdes_ctrlr.ForceIndependent == FALSE) &&
		(xhdmi_exdes_ctrlr.TxStartTransmit == FALSE) &&
		(HdmiTxSs.IsStreamUp == TRUE)) {
		TX_DynHDR_Info.Address = VidBuff[DynHdr_rd_buff_offset].DynHDRBaseAddr;
		TX_DynHDR_Info.FAPA = 0;
		TX_DynHDR_Info.PktLength = RX_DynHDR_Info.pkt_length;
		TX_DynHDR_Info.PktType = RX_DynHDR_Info.pkt_type;


	     /* if Dynamic HDR is not present
	         * set Dynamic HDR EMP generation or  as FALSE
	         */
		if(TX_DynHDR_Info.PktType == DYNAMIC_HDR_NOT_PRESENT) // NO HDR PKT
			XV_HdmiTxSs1_DynHdr_Control(&HdmiTxSs,FALSE);
		else {
			XV_HdmiTxSs1_DynHdr_Cfg(&HdmiTxSs,&TX_DynHDR_Info);
			XV_HdmiTxSs1_DynHdr_Control(&HdmiTxSs,TRUE);
		}

        /* if Dynamic HDR packet is HDR10+ VSIF or Dynamic HDR is not present
         * set Graphic overlay EMP generation as FALSE
         */
		if((TX_DynHDR_Info.PktType == DYNAMIC_HDR_HDR10P_VSIF) || (TX_DynHDR_Info.PktType == DYNAMIC_HDR_NOT_PRESENT))
			XV_HdmiTxSs1_DynHdr_GOF_Control(&HdmiTxSs,FALSE);
		else
			XV_HdmiTxSs1_DynHdr_GOF_Control(&HdmiTxSs,TRUE);

	}
}
#endif

#if (XPAR_V_HDMI_RXSS1_DSC_EN == 1)
void XV_Rx_HdmiTrigCb_DscDdcEvent(void *InstancePtr) {
  xil_printf(ANSI_COLOR_YELLOW"SCDCS(0x10) Status update is Cleared by Source\r\n");
  xil_printf("DSC_DecodeFail(bit 7 of 0x40) is cleared by Sink "ANSI_COLOR_RESET"\r\n");
}
#endif
/*****************************************************************************/
/**
*
* This function is called from the RX State machine layer.
*
* @param  InstancePtr is the callback reference.
*
* @return None.
*
* @note   None.
*
******************************************************************************/
void XV_Rx_Hdcp_SetContentStreamType(void *InstancePtr)
{
#if defined (XPAR_XV_HDMITXSS1_NUM_INSTANCES) && \
                 defined (XPAR_XV_HDMIRXSS1_NUM_INSTANCES)
	/* Verify arguments */
	Xil_AssertVoid(InstancePtr != NULL);
#if defined(USE_HDCP_HDMI_RX) || defined(USE_HDCP_HDMI_TX)
	u8 StreamType = XV_Rx_Hdcp_GetStreamType(&xhdmi_example_rx_controller);

	xdbg_printf(XDBG_DEBUG_GENERAL, "HDCP StreamType: %d\r\n", StreamType);
	XV_HdmiTxSs1_HdcpSetContentStreamType(&HdmiTxSs, StreamType);
#endif
#endif
}

/*****************************************************************************/
/**
*
* This function is called from the RX State machine layer.
*
* @param  InstancePtr is the callback reference.
*
* @return None.
*
* @note   None.
*
******************************************************************************/
void XV_Rx_Hdcp_EnforceBlanking(void *InstancePtr)
{
#if defined(USE_HDCP_HDMI_RX) || defined(USE_HDCP_HDMI_TX)
#if defined(XPAR_XV_HDMITXSS1_NUM_INSTANCES)
	XHdcp_EnforceBlanking(xhdmi_exdes_ctrlr.hdmi_rx_ctlr,
				xhdmi_exdes_ctrlr.hdmi_tx_ctlr);
#endif
#endif
}

/*****************************************************************************/
/**
*
* This function is called from the RX State machine layer.
*
* @param  InstancePtr is the callback reference.
*
* @return None.
*
* @note   None.
*
******************************************************************************/
void XV_Rx_HdmiTrigCb_StreamOff(void *InstancePtr)
{
#if defined (XPAR_XV_HDMITXSS1_NUM_INSTANCES) && \
                 defined (XPAR_XV_HDMIRXSS1_NUM_INSTANCES)
#if AUX_FIFO_CLEAR
	/* Reset the AUX fifo. */
	ResetAuxFifo();
#endif
#endif
	xil_printf("RX Stream is down.\r\n");
	xhdmi_exdes_ctrlr.IsRxPresent = FALSE;
	xhdmi_exdes_ctrlr.crop = FALSE;


	/* clear VFP Change Interuppt */
	XV_HdmiRxSs1_VfpControl(&HdmiRxSs, FALSE);

	/* Clear VRR Info Frame */

	(void)memset((void *)(&VrrInforFrame), 0,
				     sizeof(XV_HdmiC_VrrInfoFrame));

	/* If the system is in independent mode, then stream down on
	 * the RX should not affect the restart of the TX at all.
	 */
	if (xhdmi_exdes_ctrlr.ForceIndependent != TRUE) {
		xhdmi_exdes_ctrlr.SystemEvent = TRUE;
	}

	EXDES_DBG_PRINT("sysEventDebug:%s:%d:::%d\r\n", __func__,
			__LINE__, xhdmi_exdes_ctrlr.SystemEvent);
#if defined (XPAR_XV_FRMBUFWR_NUM_INSTANCES) && \
                      (XPAR_XV_FRMBUFRD_NUM_INSTANCES)
	DynHdr_wr_buff_offset = 0;
	DynHdr_rd_buff_offset = 3;
	XV_HdmiRxSs1_DynHDR_SetAddr(&HdmiRxSs,VidBuff[0].DynHDRBaseAddr);
#endif
}

/*****************************************************************************/
/**
*
* This function is called from the RX State machine layer.
*
* @param  InstancePtr is the callback reference.
*
* @return None.
*
* @note   None.
*
******************************************************************************/
void XV_Rx_HdmiTrigCb_VfmcDataClkSel(void *InstancePtr)
{
	if (xhdmi_exdes_ctrlr.hdmi_rx_ctlr->HdmiRxSs->HdmiRx1Ptr->Stream.IsFrl ==
	    TRUE) {
		EXDES_DBG_PRINT("sysEventDebug:%s  FRL Mode -> RX Ch4 as Data\r\n",
				__func__);
		Vfmc_Gpio_Ch4_DataClock_Sel(&Vfmc[0], VFMC_GPIO_RX_CH4_As_Data);
	} else {
		EXDES_DBG_PRINT("sysEventDebug:%s: TMDS/DVI Mode -> RX "
				"Ch4 as Clock\r\n", __func__);
		Vfmc_Gpio_Ch4_DataClock_Sel(&Vfmc[0], VFMC_GPIO_RX_CH4_As_Clock);
	}
}

/*****************************************************************************/
/**
*
* This function is called from the RX State machine layer.
*
* @param  InstancePtr is the callback reference.
*
* @return None.
*
* @note   None.
*
******************************************************************************/
void XV_Rx_HdmiTrigCb_VfmcRxClkSel(void *InstancePtr)
{

	XV_Rx *XV_RxInst = (XV_Rx *)InstancePtr;

	u64 LineRate =
		((u64)(XV_RxInst->HdmiRxSs->HdmiRx1Ptr->Stream.Frl.LineRate)) *
			((u64)(1e9));

	u8 Lanes = XV_RxInst->HdmiRxSs->HdmiRx1Ptr->Stream.Frl.Lanes;

	if (xhdmi_exdes_ctrlr.hdmi_rx_ctlr->HdmiRxSs->HdmiRx1Ptr->Stream.IsFrl ==
	    TRUE) {
#if (!defined XPS_BOARD_VEK280_ES)
		Vfmc_Mezz_HdmiRxRefClock_Sel(&Vfmc[0],
			VFMC_MEZZ_RxRefclk_From_Si5344);
		XHdmiphy1_ClkDetFreqReset(&Hdmiphy1, 0, XHDMIPHY1_DIR_RX);
#endif

		Vfmc_Gpio_Mezz_HdmiRxDriver_Reconfig(&Vfmc[0], 1, LineRate, Lanes);
	} else if (xhdmi_exdes_ctrlr.hdmi_rx_ctlr->HdmiRxSs->HdmiRx1Ptr->
			Stream.IsFrl == FALSE) {
#if (!defined XPS_BOARD_VEK280_ES)
		Vfmc_Mezz_HdmiRxRefClock_Sel(&Vfmc[0],
			VFMC_MEZZ_RxRefclk_From_Cable);
#endif
		Vfmc_Gpio_Mezz_HdmiRxDriver_Reconfig(&Vfmc[0], 0, LineRate, Lanes);
	}
}
#endif /* XPAR_XV_HDMIRXSS1_NUM_INSTANCES */

/*****************************************************************************/
/**
*
* This function is called whenever an error condition in VPHY occurs.
* This will fill the FIFO of VPHY error events which will be processed outside
* the ISR.
*
* @param  CallbackRef is the VPHY instance pointer
* @param  ErrIrqType is the VPHY error type
*
* @return None.
*
* @note   None.
*
******************************************************************************/
void Hdmiphy1ErrorCallback(void *CallbackRef)
{
	Hdmiphy1ErrorFlag = TRUE;
}

/*****************************************************************************/
/**
*
* This function is called in the application to process the pending
* VPHY errors
*
* @param  None.
*
* @return None.
*
* @note   This function can be expanded to perform necessary actions depending
*         on the error type. For example, XHDMIPHY1_ERR_PLL_LAYOUT can be
*         used to automatically switch in and out of bonded mode for
*         GTXE2 devices
*
******************************************************************************/
void Hdmiphy1ProcessError(void)
{
	if (Hdmiphy1ErrorFlag == TRUE) {
		xil_printf(ANSI_COLOR_RED"VPHY Error: See log for details"
				ANSI_COLOR_RESET "\r\n");
	}
	/* Clear Flag */
	Hdmiphy1ErrorFlag = FALSE;

}

#ifdef XPAR_XV_HDMITXSS1_NUM_INSTANCES
/*****************************************************************************/
/**
*
* This function is called whenever an error condition in HDMI TX occurs.
* This will fill the FIFO of HDMI TX error events which will be processed
* outside the ISR.
*
* @param  CallbackRef is the HDMI TXSS instance pointer
*
* @return None.
*
* @note   None.
*
******************************************************************************/
void HdmiTxErrorCallback(void *CallbackRef)
{
	HdmiTxErrorFlag = TRUE;
}

/*****************************************************************************/
/**
*
* This function is called in the application to process the pending
* HDMI TX errors
*
* @param  None.
*
* @return None.
*
* @note   This function can be expanded to perform necessary actions depending
*         on the error type.
*
******************************************************************************/
void HdmiTxProcessError(void)
{
	if (HdmiTxErrorFlag == TRUE) {
		xil_printf(ANSI_COLOR_RED"HDMI TX Error: See log for details"
				ANSI_COLOR_RESET "\r\n");
	}

	/* Clear Flag */
	HdmiTxErrorFlag = FALSE;
}

/*****************************************************************************/
/**
*
* This function updates the ColorFormat for the current video stream
*
* @param Hdmiphy1Ptr is a pointer to the VPHY core instance.
* @param HdmiTxSs1Ptr is a pointer to the XV_HdmiTxSs1 instance.
* @param Requested ColorFormat
*
* @return None.
*
* @note   None.
*
******************************************************************************/
void UpdateColorFormat(XHdmiphy1         *Hdmiphy1Ptr,
		       XV_HdmiTxSs1      *HdmiTxSs1Ptr,
		       XVidC_ColorFormat ColorFormat)
{

	XVidC_VideoStream *HdmiTxSsVidStreamPtr;
	HdmiTxSsVidStreamPtr = XV_HdmiTxSs1_GetVideoStream(HdmiTxSs1Ptr);

	/* Check passthrough */
	if ((xhdmi_exdes_ctrlr.IsRxPresent && xhdmi_exdes_ctrlr.IsTxPresent) &&
	    xhdmi_exdes_ctrlr.ForceIndependent == FALSE) {
		xil_printf("Error: Color space conversion in "
			   "pass-through mode is not supported!\r\n");
		return;
	}

	/* Inform user that pixel repetition is not supported */
	if (((HdmiTxSsVidStreamPtr->VmId == XVIDC_VM_1440x480_60_I) ||
	     (HdmiTxSsVidStreamPtr->VmId == XVIDC_VM_1440x576_50_I)) &&
	    (ColorFormat == XVIDC_CSF_YCRCB_422)) {

		xil_printf("The video bridge is unable to support "
			   "pixel repetition in YUV 422 Color space\r\n");

	}

	Exdes_ChangeColorbarOutput(HdmiTxSsVidStreamPtr->VmId,
				   ColorFormat,
				   HdmiTxSsVidStreamPtr->ColorDepth);

	/* Trigger transmitter (re)start */
	xhdmi_exdes_ctrlr.SystemEvent = TRUE;
}

/*****************************************************************************/
/**
*
* This function updates the ColorDepth for the current video stream
*
* @param Hdmiphy1Ptr is a pointer to the VPHY core instance.
* @param HdmiTxSs1Ptr is a pointer to the XV_HdmiTxSs1 instance.
* @param Requested ColorFormat
*
* @return None.
*
* @note   None.
*
******************************************************************************/
void UpdateColorDepth(XHdmiphy1         *Hdmiphy1Ptr,
		      XV_HdmiTxSs1      *HdmiTxSs1Ptr,
		      XVidC_ColorDepth  ColorDepth)
{

	XVidC_VideoStream *HdmiTxSsVidStreamPtr;
	HdmiTxSsVidStreamPtr = XV_HdmiTxSs1_GetVideoStream(HdmiTxSs1Ptr);

	/* Check if the exdes is in Passthrough mode.
	 * If not, then check if Color Space is YUV422.
	 * If still not, then check if Rate is more than 5.94 Gbps */
	if ((xhdmi_exdes_ctrlr.IsRxPresent && xhdmi_exdes_ctrlr.IsTxPresent) &&
	    (xhdmi_exdes_ctrlr.ForceIndependent == FALSE)) {
		xil_printf("Color depth conversion in pass-through "
			   "mode not supported!\r\n");
		return;
	} else if ((HdmiTxSsVidStreamPtr->ColorFormatId ==
	            XVIDC_CSF_YCRCB_422) &&
	           (ColorDepth != XVIDC_BPC_12)) {
		xil_printf("YUV422 only supports 36-bits color depth!\r\n");
		return;
	} else if ((HdmiTxSs1Ptr->HdmiTx1Ptr->Stream.IsFrl != TRUE) &&
	           (((HdmiTxSsVidStreamPtr->VmId == XVIDC_VM_3840x2160_60_P) ||
	             (HdmiTxSsVidStreamPtr->VmId == XVIDC_VM_3840x2160_50_P)) &&
	            ((HdmiTxSsVidStreamPtr->ColorFormatId == XVIDC_CSF_RGB) ||
	             (HdmiTxSsVidStreamPtr->ColorFormatId ==
	              XVIDC_CSF_YCRCB_444)) &&
	            (ColorDepth != XVIDC_BPC_8))) {
		xil_printf("2160p60 & 2160p50 on RGB & YUV444 only"
			   " supports 24-bits colordepth!\r\n");
		return;
	}

	Exdes_ChangeColorbarOutput(HdmiTxSsVidStreamPtr->VmId,
				   HdmiTxSsVidStreamPtr->ColorFormatId,
				   ColorDepth);

	/* Trigger transmitter (re)start */
	xhdmi_exdes_ctrlr.SystemEvent = TRUE;

}

/*****************************************************************************/
/**
*
* This function updates the FrameRate for the current video stream
*
* @param Hdmiphy1Ptr is a pointer to the VPHY core instance.
* @param HdmiTxSs1Ptr is a pointer to the XV_HdmiTxSs1 instance.
* @param Requested FrameRate
*
* @return None.
*
* @note   None.
*
******************************************************************************/
void UpdateFrameRate(XHdmiphy1       *Hdmiphy1Ptr,
		     XV_HdmiTxSs1    *HdmiTxSs1Ptr,
		     XVidC_FrameRate FrameRate)
{

	XVidC_VideoStream *HdmiTxSsVidStreamPtr;
	HdmiTxSsVidStreamPtr = XV_HdmiTxSs1_GetVideoStream(HdmiTxSs1Ptr);

	/* Check pass through */
	if ((xhdmi_exdes_ctrlr.IsRxPresent && xhdmi_exdes_ctrlr.IsTxPresent) &&
	    xhdmi_exdes_ctrlr.ForceIndependent == FALSE) {
		xil_printf("Frame rate conversion in pass-through"
			   " mode not supported!\r\n");
		return;
	}

	/* Check if requested video mode is available */
	XVidC_VideoMode VmId =
		XVidC_GetVideoModeIdExtensive(&HdmiTxSsVidStreamPtr->Timing,
					FrameRate,
					HdmiTxSsVidStreamPtr->IsInterlaced,
					(FALSE));

	HdmiTxSsVidStreamPtr->VmId = VmId;

	Exdes_ChangeColorbarOutput(HdmiTxSsVidStreamPtr->VmId,
				HdmiTxSsVidStreamPtr->ColorFormatId,
				HdmiTxSsVidStreamPtr->ColorDepth);

	/* Trigger transmitter (re)start */
	xhdmi_exdes_ctrlr.SystemEvent = TRUE;
}
#endif /* XPAR_XV_HDMITXSS1_NUM_INSTANCES */

/*****************************************************************************/
/**
*
* This is the callback for assertion error.
*
* @param File is string name of the file where the assertion failure occured.
* @param LIne is the line wheer the assertion failure occured.
*
* @return None.
*
* @note   None.
*
******************************************************************************/
void Xil_AssertCallbackRoutine(u8 *File, s32 Line)
{
	xil_printf("Assertion in File %s, on line %0d\r\n", File, Line);
}

/*****************************************************************************/
/**
*
* This function updates the FrameRate for the current video stream
*
* @param  ps_iic0_deviceid is the device id of the ps iic device.
* @param  ps_iic1_deviceid is the device id of the ps iic device.
*
* @return XST_SUCCESS if the clock source is successfuly set.
*         XST_FAILURE otherwise.
*
* @note   None.
*
******************************************************************************/
u32 Exdes_SetupClkSrc(u32 ps_iic0_deviceid, u32 ps_iic1_deviceid)
{
	u32 Status = XST_SUCCESS;
	u8 Buffer;
#if (defined XPS_BOARD_ZCU102) || (defined XPS_BOARD_ZCU106) || \
	    defined (XPS_BOARD_VCK190)

	XIicPs_Config *XIic0Ps_ConfigPtr;
	XIicPs_Config *XIic1Ps_ConfigPtr;

	/* Initialize IIC */
	/* Initialize PS IIC0 */
	XIic0Ps_ConfigPtr = XIicPs_LookupConfig(ps_iic0_deviceid);
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

	/* Initialize PS IIC1 */
	XIic1Ps_ConfigPtr = XIicPs_LookupConfig(ps_iic1_deviceid);
	if (NULL == XIic1Ps_ConfigPtr) {
		return XST_FAILURE;
	}

	Status = XIicPs_CfgInitialize(&Iic, XIic1Ps_ConfigPtr,
				XIic1Ps_ConfigPtr->BaseAddress);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	XIicPs_Reset(&Iic);
	/*
	 * Set the IIC serial clock rate.
	 */
	XIicPs_SetSClk(&Iic, PS_IIC_CLK);

#if (defined XPS_BOARD_ZCU102)
	I2cMuxSel(&Iic, ZCU102_MGT_SI570);
#elif (defined XPS_BOARD_ZCU106)
	I2cMuxSel(&Iic, ZCU106_MGT_SI570);
#elif (defined XPS_BOARD_VCK190)
	I2cMuxSel(&Ps_Iic0, VCK190_MGT_SI570);
#endif

#elif defined(XPS_BOARD_VEK280_ES) /* VEK280*/
	XIicPs_Config *XIic0Ps_ConfigPtr;
	/* Initialize IIC */
	/* Initialize PS IIC0 */
	XIic0Ps_ConfigPtr = XIicPs_LookupConfig(ps_iic0_deviceid);
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

	XIicPs *Ps_IicPtr = &Ps_Iic0;

	/*RC21008A Initialization */
	Buffer = 0x20;
	Status = Vfmc_I2cSend_RC(Ps_IicPtr, 0x74,
				   (u8 *)&Buffer, 1, (I2C_STOP));

	Status = RC21008A_Init(Ps_IicPtr, RC21008A_ADDR);
	if (Status != XST_SUCCESS) {
		xil_printf("Failed to initialize RC21008A.\r\n");
		return XST_FAILURE;
	} else if (Status == XST_SUCCESS){
		xil_printf("RC21008A initialization done.\r\n");
	}

	Status = XIic_Initialize(&Iic, XPAR_IIC_0_DEVICE_ID);
	Status |= XIic_Start(&Iic);

#else
	Status = XIic_Initialize(&Iic, XPAR_IIC_0_DEVICE_ID);
	Status |= XIic_Start(&Iic);

#if defined (XPS_BOARD_VEK280_ES)
	I2cMuxSel(&Iic, VCK190_MGT_SI570);
#else
	I2cMuxSel(&Iic, VCU118_FMCP);
#endif
#endif



#if (defined XPS_BOARD_VCK190)
#ifdef XPAR_XV_HDMIRXSS1_NUM_INSTANCES
	/* Set DRU MGT REFCLK Frequency */
	Si570_SetFreq(&Ps_Iic0, 0x5F, 400.00);
	/* Delay 50ms to allow SI chip to lock */
	usleep (50000);
#endif
#elif (!defined XPS_BOARD_VCU118) && (!defined XPS_BOARD_VEK280_ES)
#ifdef XPAR_XV_HDMIRXSS1_NUM_INSTANCES
	/* Set DRU MGT REFCLK Frequency */
	Si570_SetFreq(&Iic, 0x5D, 400.00);
	/* Delay 50ms to allow SI chip to lock */
	usleep (50000);
#endif
#endif

	return Status;
}

#if defined (XPAR_XHDCP_NUM_INSTANCES) || \
	defined (XPAR_XHDCP22_RX_NUM_INSTANCES) || \
	defined (XPAR_XHDCP22_TX_NUM_INSTANCES)
/*****************************************************************************/
/**
*
* This function loads the HDCP keys from the global arrays to the HDMI 2.1 TX
* and TX SS HDCP modules as applicable.
*
* @param  IicPtr is the on board iic device used to access the hdcp keys
*         from storage.
*
* @return XST_SUCCESS if the clock source is successfuly set.
*         XST_FAILURE otherwise.
*
* @note   None.
*
******************************************************************************/
u32 Exdes_LoadHdcpKeys(void *IicPtr)
{
#if defined (XPS_BOARD_ZCU102) || \
    defined (XPS_BOARD_ZCU104) || \
    defined (XPS_BOARD_ZCU106) || \
    defined (XPS_BOARD_VCK190)
    XIicPs *Iic_Ptr = IicPtr;
#else
    XIic *Iic_Ptr = IicPtr;
#endif

	u32 Status = XST_SUCCESS;

	/* Load HDCP keys from EEPROM */
	if (XHdcp_LoadKeys(Iic_Ptr,
			Hdcp22Lc128,	sizeof(Hdcp22Lc128),
			Hdcp22RxPrivateKey,	sizeof(Hdcp22RxPrivateKey),
			Hdcp14KeyA,	sizeof(Hdcp14KeyA),
			Hdcp14KeyB,	sizeof(Hdcp14KeyB)) == XST_SUCCESS) {

		/* Set pointers to HDCP 2.2 Keys */
#ifdef XPAR_XV_HDMITXSS1_NUM_INSTANCES
#if XPAR_XHDCP22_TX_NUM_INSTANCES
		XV_HdmiTxSs1_HdcpSetKey(&HdmiTxSs, XV_HDMITXSS1_KEY_HDCP22_LC128,
				Hdcp22Lc128);
		XV_HdmiTxSs1_HdcpSetKey(&HdmiTxSs, XV_HDMITXSS1_KEY_HDCP22_SRM,
				Hdcp22Srm);
#endif
#endif
#ifdef XPAR_XV_HDMIRXSS1_NUM_INSTANCES
#if XPAR_XHDCP22_RX_NUM_INSTANCES
		XV_HdmiRxSs1_HdcpSetKey(&HdmiRxSs, XV_HDMIRXSS1_KEY_HDCP22_LC128,
				Hdcp22Lc128);
		XV_HdmiRxSs1_HdcpSetKey(&HdmiRxSs, XV_HDMIRXSS1_KEY_HDCP22_PRIVATE,
				Hdcp22RxPrivateKey);
#endif
#endif

		/* Set pointers to HDCP 1.4 keys */
#if XPAR_XHDCP_NUM_INSTANCES
#ifdef XPAR_XV_HDMITXSS1_NUM_INSTANCES
		XV_HdmiTxSs1_HdcpSetKey(&HdmiTxSs, XV_HDMITXSS1_KEY_HDCP14,
				Hdcp14KeyA);
#endif
#ifdef XPAR_XV_HDMIRXSS1_NUM_INSTANCES
		XV_HdmiRxSs1_HdcpSetKey(&HdmiRxSs, XV_HDMIRXSS1_KEY_HDCP14,
				Hdcp14KeyB);
#endif

		/* Initialize key manager */
#ifdef XPAR_XV_HDMITXSS1_NUM_INSTANCES
		Status = XHdcp_KeyManagerInit(XPAR_HDCP_KEYMNGMT_BLK_0_BASEADDR,
						HdmiTxSs.Hdcp14KeyPtr);
		if (Status != XST_SUCCESS) {
			xil_printf ("HDCP 1.4 TX Key Manager Initialization error\r\n");
			return XST_FAILURE;
		}
#endif

#ifdef XPAR_XV_HDMIRXSS1_NUM_INSTANCES
		Status = XHdcp_KeyManagerInit(XPAR_HDCP_KEYMNGMT_BLK_1_BASEADDR,
						HdmiRxSs.Hdcp14KeyPtr);
		if (Status != XST_SUCCESS) {
			xil_printf("HDCP 1.4 RX Key Manager Initialization error\r\n");
			return XST_FAILURE;
		}
#endif
#endif

	} else {
		/* Clear pointers */

#ifdef XPAR_XV_HDMITXSS1_NUM_INSTANCES
		/* Set pointer to NULL */
		XV_HdmiTxSs1_HdcpSetKey(&HdmiTxSs, XV_HDMITXSS1_KEY_HDCP22_LC128,
				(NULL));

		/* Set pointer to NULL */
		XV_HdmiTxSs1_HdcpSetKey(&HdmiTxSs, XV_HDMITXSS1_KEY_HDCP14,
				(NULL));

		/* Set pointer to NULL */
		XV_HdmiTxSs1_HdcpSetKey(&HdmiTxSs, XV_HDMITXSS1_KEY_HDCP22_SRM,
				(NULL));
#endif

#ifdef XPAR_XV_HDMIRXSS1_NUM_INSTANCES
		/* Set pointer to NULL */
		XV_HdmiRxSs1_HdcpSetKey(&HdmiRxSs, XV_HDMIRXSS1_KEY_HDCP22_LC128,
				(NULL));

		/* Set pointer to NULL */
		XV_HdmiRxSs1_HdcpSetKey(&HdmiRxSs, XV_HDMIRXSS1_KEY_HDCP22_PRIVATE,
				(NULL));

		/* Set pointer to NULL */
		XV_HdmiRxSs1_HdcpSetKey(&HdmiRxSs, XV_HDMIRXSS1_KEY_HDCP14,
				(NULL));
#endif
	}

	return Status;
}
#endif

/*****************************************************************************/
/**
*
* This function setups the interrupt system so interrupts can occur for the
* HDMI cores. The function is application-specific since the actual system
* may or may not have an interrupt controller. The HDMI cores could be
* directly connected to a processor without an interrupt controller.
* The user should modify this function to fit the application.
*
* @param  None.
*
* @return
*   - XST_SUCCESS if interrupt setup was successful.
*   - A specific error code defined in "xstatus.h" if an error
*   occurs.
*
* @note   This function assumes a Microblaze system and no operating
*   system is used.
*
******************************************************************************/
int SetupInterruptSystem(void)
{
	int Status;
#if defined (ARMR5) || (__aarch64__) || (__arm__)
	XScuGic *IntcInstPtr = &Intc;
#else
	XIntc *IntcInstPtr = &Intc;
#endif

	/*
	 * Initialize the interrupt controller driver so that it's ready to
	 * use, specify the device ID that was generated in xparameters.h
	 */
#if defined (ARMR5) || (__aarch64__) || (__arm__)
	XScuGic_Config *IntcCfgPtr;
	IntcCfgPtr = XScuGic_LookupConfig(XPAR_SCUGIC_0_DEVICE_ID);
	if (IntcCfgPtr == NULL) {
		xil_printf("ERR:: Interrupt Controller not found");
		return (XST_DEVICE_NOT_FOUND);
	}
	Status = XScuGic_CfgInitialize(IntcInstPtr,
				IntcCfgPtr,
				IntcCfgPtr->CpuBaseAddress);
#else
	Status = XIntc_Initialize(IntcInstPtr, XPAR_INTC_0_DEVICE_ID);
#endif
	if (Status != XST_SUCCESS) {
		xil_printf("Intc initialization failed!\r\n");
		return XST_FAILURE;
	}


	/*
	 * Start the interrupt controller such that interrupts are recognized
	 * and handled by the processor
	 */
#if defined (__MICROBLAZE__)
	Status = XIntc_Start(IntcInstPtr, XIN_REAL_MODE);
        /* Status = XIntc_Start(IntcInstPtr, XIN_SIMULATION_MODE); */
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}
#endif

	Xil_ExceptionInit();

	/*
	 * Register the interrupt controller handler with the exception table.
	 */
#if defined (ARMR5) || (__aarch64__) || (__arm__)
	Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_INT,
				(Xil_ExceptionHandler)XScuGic_InterruptHandler,
				(XScuGic *)IntcInstPtr);
#else
	Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_INT,
				(Xil_ExceptionHandler)XIntc_InterruptHandler,
				(XIntc *)IntcInstPtr);
#endif

	return (XST_SUCCESS);
}

#ifdef XPAR_XV_HDMITXSS1_NUM_INSTANCES

#ifdef XPAR_XV_AXI4S_REMAP_NUM_INSTANCES
/*****************************************************************************/
/**
*
* This function initialized the In Remapper and Out Remapper.
*
* @param  inremap_deviceid is the In Remapper Device ID
* @param  outremap_deviceid is the Out Remapper Device ID
*
* @return XST_SUCCESS if the clock source is successfuly set.
*         XST_FAILURE otherwise.
*
* @note   None.
*
******************************************************************************/
u32 Exdes_RemapInitialize(u32 inremap_deviceid, u32 outremap_deviceid)
{
	u32 Status = XST_SUCCESS;
	XV_axi4s_remap_Config *InRemapPtr, *OutRemapPtr;
	InRemapPtr = XV_axi4s_remap_LookupConfig(inremap_deviceid);
	if (InRemapPtr == NULL) {
		InRemap.IsReady = 0;
		return (XST_DEVICE_NOT_FOUND);
	}
	Status = XV_axi4s_remap_CfgInitialize(&InRemap,
			InRemapPtr, InRemapPtr->BaseAddress);
	if (Status != XST_SUCCESS) {
		xil_printf("ERR:: InRemap Initialization failed %d\r\n",
			Status);
		return(XST_FAILURE);
	}

	OutRemapPtr = XV_axi4s_remap_LookupConfig(outremap_deviceid);
	if (OutRemapPtr == NULL) {
		OutRemap.IsReady = 0;
		return (XST_DEVICE_NOT_FOUND);
	}
	Status = XV_axi4s_remap_CfgInitialize(&OutRemap,
			OutRemapPtr, OutRemapPtr->BaseAddress);
	if (Status != XST_SUCCESS) {
		xil_printf("ERR:: OutRemap Initialization failed %d\r\n",
			Status);
		return(XST_FAILURE);
	}
	return Status;
}
#endif

#ifdef XPAR_XV_TPG_NUM_INSTANCES
/*****************************************************************************/
/**
*
* This function initialized the TPG.
*
* @param  Gpio_tpg_resetn_deviceid is the GPIO devive id used to reset the TPG.
* @param  tpg_deviceid is the TPG devive id.
*
* @return XST_SUCCESS if the clock source is successfuly set.
*         XST_FAILURE otherwise.
*
* @note   None.
*
******************************************************************************/
u32 Exdes_TpgInitialize(u32 Gpio_tpg_resetn_deviceid, u32 tpg_deviceid)
{
	u32 Status = XST_SUCCESS;
	XGpio_Config *Gpio_Tpg_resetn_ConfigPtr;
	XV_tpg_Config *Tpg_ConfigPtr;

	/* Initialize GPIO for Tpg Reset */
	Gpio_Tpg_resetn_ConfigPtr = XGpio_LookupConfig(Gpio_tpg_resetn_deviceid);
	if (Gpio_Tpg_resetn_ConfigPtr == NULL) {
		Gpio_Tpg_resetn.IsReady = 0;
		return (XST_DEVICE_NOT_FOUND);
	}

	Status = XGpio_CfgInitialize(&Gpio_Tpg_resetn,
				Gpio_Tpg_resetn_ConfigPtr,
				Gpio_Tpg_resetn_ConfigPtr->BaseAddress);
	if (Status != XST_SUCCESS) {
		xil_printf("ERR:: GPIO for TPG Reset \r\n "
				"Initialization failed %d\r\n", Status);
		return (XST_FAILURE);
	}

	Tpg_ConfigPtr = XV_tpg_LookupConfig(tpg_deviceid);
	if (Tpg_ConfigPtr == NULL) {
		Tpg.IsReady = 0;
		return (XST_DEVICE_NOT_FOUND);
	}

	Status = XV_tpg_CfgInitialize(&Tpg,
				Tpg_ConfigPtr, Tpg_ConfigPtr->BaseAddress);
	if (Status != XST_SUCCESS) {
		xil_printf("ERR:: TPG Initialization failed %d\r\n", Status);
		return(XST_FAILURE);
	}
	return Status;
}
#endif
#endif

#ifdef XPAR_XAXIS_SWITCH_NUM_INSTANCES
/*****************************************************************************/
/**
*
* This function initialized the AXI Stream Switch.
*
* @param  Gpio_tpg_resetn_deviceid is the GPIO devive id used to reset the TPG.
* @param  tpg_deviceid is the TPG devive id.
*
* @return XST_SUCCESS if the clock source is successfuly set.
*         XST_FAILURE otherwise.
*
* @note   None.
*
******************************************************************************/
u32 Exdes_AxisSwitchInitialize(XAxis_Switch *AxisSwitchPtr,u32 deviceid)
{
	u32 Status = XST_SUCCESS;
	XAxis_Switch_Config *AxisSwitchConfigPtr;

	/* Initialize GPIO for Tpg Reset */
	AxisSwitchConfigPtr = XAxisScr_LookupConfig(deviceid);
	if (AxisSwitchConfigPtr == NULL) {
		AxisSwitchPtr->IsReady = 0;
		return (XST_DEVICE_NOT_FOUND);
	}

	Status = XAxisScr_CfgInitialize(AxisSwitchPtr, AxisSwitchConfigPtr,
					AxisSwitchConfigPtr->BaseAddress);
	if (Status != XST_SUCCESS) {
		xil_printf("ERR:: AXIS Stream Switch \r\n "
				"Initialization failed for Device ID:%d,Status:\r\n",deviceid,Status);
		return (XST_FAILURE);
	}
	return Status;
}
#endif


/*****************************************************************************/
/**
*
* Main function to call example with HDMI TX, HDMI RX and HDMI GT drivers.
*
* @param  None.
*
* @return
*   - XST_SUCCESS if HDMI example was successfully.
*   - XST_FAILURE if HDMI example failed.
*
* @note   None.
*
******************************************************************************/
int main()
{
	u32 Status = XST_FAILURE;
#ifdef XPAR_XV_HDMITXSS1_NUM_INSTANCES
	TxInputSourceType TxInputSrc;
	XVidC_VideoStream *HdmiTxSsVidStreamPtr;

	/**
	 * Reset the Fifo buffer which will be used for holding
	 * incoming auxillary packets.
	 */
	ResetAuxFifo();
#endif
#if (!defined XPAR_XVTC_NUM_INSTANCES) || (defined (XPAR_XV_FRMBUFWR_NUM_INSTANCES) && \
                      (XPAR_XV_FRMBUFRD_NUM_INSTANCES))
#if defined (VTEM2FSYNC)
	XV_HdmiC_VrrInfoFrame  *HdmiRxVrrInfoFrameVRRPtr;
	XVidC_VideoStream      *HdmiRxSsVidStreamVRRPtr;
#endif
#endif

	xil_printf("\r\n\r\n");
	xil_printf("------------------------------------------\r\n");
#if defined (XPAR_XV_FRMBUFRD_NUM_INSTANCES) && \
                      (XPAR_XV_FRMBUFWR_NUM_INSTANCES)
	xil_printf("--- HDMI 2.1 SS + HdmiPhy VRR Example v%d.%d ---\r\n",
			APP_MAJ_VERSION, APP_MIN_VERSION);
#else
	xil_printf("--- HDMI 2.1 SS + HdmiPhy Example v%d.%d---\r\n",
			APP_MAJ_VERSION, APP_MIN_VERSION);
#endif
	xil_printf("---    (c) 2019 by Xilinx, Inc.        ---\r\n");
	xil_printf("------------------------------------------\r\n");
	xil_printf("Build %s - %s\r\n", __DATE__, __TIME__);
	xil_printf("------------------------------------------\r\n");
#ifdef XPAR_XV_HDMITXSS1_NUM_INSTANCES
	xhdmi_exdes_ctrlr.TxBusy            = (TRUE);
	HdmiTxErrorFlag = FALSE;
#endif
	Hdmiphy1ErrorFlag = FALSE;
	Hdmiphy1PllLayoutErrorFlag = FALSE;



	/* Initialize platform */
	xil_printf("Initializing platform. \r\n");
	init_platform();

	/**
	 * Setup IIC devices and clock sources.
	 */
	xil_printf("Initializing IIC and clock sources. \r\n");
#if (defined (ARMR5) || (__aarch64__)) && (!defined XPS_BOARD_ZCU104)
	/* Initialize the PS_I2C and Initialize the clocks */
	Status = Exdes_SetupClkSrc(XPAR_XIICPS_0_DEVICE_ID,
			XPAR_XIICPS_1_DEVICE_ID);
	if (Status != XST_SUCCESS) {
		xil_printf("PS IIC initialization failed. \r\n");
	}
#elif (__microblaze__)
	/* Initialize the IIC and Initialize the clocks */
	Status = Exdes_SetupClkSrc((u32)NULL, (u32)NULL);
	if (Status != XST_SUCCESS) {
		xil_printf("PS IIC initialization failed. \r\n");
	}
#endif

	/**
	 * Load the HDCP keys.
	 */
#if defined (XPAR_XHDCP_NUM_INSTANCES) || \
	defined (XPAR_XHDCP22_RX_NUM_INSTANCES) || \
	defined (XPAR_XHDCP22_TX_NUM_INSTANCES)
	xil_printf("Initializing HDCP keys. \r\n");

	Vfmc[0].IicPtr = &Iic;
	Vfmc[0].Loc = VFMC_HPC0;

	/* Select the */
	Vfmc_I2cMuxSelect(&Vfmc[0]);

	Status = Exdes_LoadHdcpKeys(&Iic);
	if (Status != XST_SUCCESS) {
		xil_printf("Loading HDCP keys failed !!\r\n");
	}
#endif

	/**
	 *  Initialize the Audio Generator.
	 */
#if defined(XPAR_XV_HDMITXSS1_NUM_INSTANCES)
#if defined(USE_HDMI_AUDGEN)
	xil_printf("Initializing Audio Generator. \r\n");
#if defined (XPS_BOARD_VCK190) ||\
	defined (XPS_BOARD_VEK280_ES)
	XhdmiAudGen_Init(&AudioGen,
			XPAR_AUDIO_SS_0_AUD_PAT_GEN_BASEADDR,
			XPAR_AUDIO_SS_0_HDMI_ACR_CTRL_BASEADDR,
			XPAR_AUDIO_SS_0_CLK_WIZARD_BASEADDR);
#else
	XhdmiAudGen_Init(&AudioGen,
			XPAR_AUDIO_SS_0_AUD_PAT_GEN_BASEADDR,
			XPAR_AUDIO_SS_0_HDMI_ACR_CTRL_BASEADDR,
			XPAR_AUDIO_SS_0_CLK_WIZ_BASEADDR);
#endif
#endif
#endif

	/**
	 * Initialize the Interrupt Controller.
	 */
	xil_printf("Initializing Interrupt controller. \r\n");
	Status = SetupInterruptSystem();
	if (Status == XST_FAILURE) {
		xil_printf("IRQ init failed.\r\n\r\n");
		return XST_FAILURE;
	} else {
		xil_printf("Interrupt Controller setup successful.\r\n");
	}

	/**
	 * Initialize the HDMI Video Transmitter "state machine"
	 * functionality and the downstream HDCP device.
	 */
#ifdef XPAR_XV_HDMITXSS1_NUM_INSTANCES
	xil_printf("Initializing HDMI Video Transmitter. \r\n");

	/* Initialize the controller for the Video TX for HDMI */
	Status = XV_Tx_InitController(&xhdmi_example_tx_controller,
			XPAR_XV_HDMITXSS1_0_DEVICE_ID,
			XPAR_HDMIPHY1_0_DEVICE_ID);
	if (Status != XST_SUCCESS) {
		xil_printf("HDMI Video Transmitter system setup failed !!\r\n");
	} else {
		xil_printf("HDMI Video Transmitter system setup successful.\r\n");
	}
#endif /* XPAR_XV_HDMITXSS1_NUM_INSTANCES */

	/**
	 * Initialize the HDMI Video Receiver "state machine"
	 * functionality and the downstream HDCP device.
	 */
#ifdef XPAR_XV_HDMIRXSS1_NUM_INSTANCES
	xil_printf("Initializing HDMI Video Receiver. \r\n");

	/* Initialize the controller for the Video RX for HDMI. */
	Status = XV_Rx_InitController(&xhdmi_example_rx_controller,
			XPAR_XV_HDMIRXSS1_0_DEVICE_ID,
			XPAR_HDMIPHY1_0_DEVICE_ID);
	if (Status != XST_SUCCESS) {
		xil_printf("HDMI Video Receiver system setup failed !!\r\n");
	} else {
		xil_printf("HDMI Video Receiver system setup successful.\r\n");
	}
#endif /* XPAR_XV_HDMIRXSS1_NUM_INSTANCES */

	/**
	 * Set the Video Phy Error callbacks.
	 */
	XHdmiphy1_SetErrorCallback(&Hdmiphy1,
				(void *)Hdmiphy1ErrorCallback,
				(void *)&Hdmiphy1);

#ifdef XPAR_XV_HDMITXSS1_NUM_INSTANCES
	Status |= XV_HdmiTxSs1_SetCallback(&HdmiTxSs,
			XV_HDMITXSS1_HANDLER_ERROR,
			(void *)HdmiTxErrorCallback,
			(void *)&HdmiTxSs);
#endif

	/* Initialize Video FMC and GT TX output*/
	Status = Vfmc_HdmiInit(&Vfmc[0], XPAR_VFMC_CTLR_SS_0_VFMC_GPIO_DEVICE_ID,
						&Iic, VFMC_HPC0);
	if (Status == XST_FAILURE) {
		xil_printf("VFMC Initialization Error! Exiting Program...\r\n");
		return 0;
	} else {
		for (int ChId=1; ChId <= 4; ChId++) {
#ifdef XPAR_XV_HDMITXSS1_NUM_INSTANCES
#if defined (XPS_BOARD_ZCU102)
			XHdmiphy1_SetTxVoltageSwing(&Hdmiphy1, 0, ChId,
					(Vfmc[0].TxMezzType == VFMC_MEZZ_HDMI_PASSIVE) ? 0xC : 0xD);/*0xc 0xb */
			XHdmiphy1_SetTxPreEmphasis(&Hdmiphy1, 0, ChId,
					(Vfmc[0].TxMezzType == VFMC_MEZZ_HDMI_PASSIVE) ? 0x1 : 0x5);/*1, A */
			XHdmiphy1_SetTxPostCursor(&Hdmiphy1, 0, ChId,
					(Vfmc[0].TxMezzType == VFMC_MEZZ_HDMI_PASSIVE) ? 0x1 : 0x5);/*1, B */
#elif defined (XPS_BOARD_ZCU106)
			XHdmiphy1_SetTxVoltageSwing(&Hdmiphy1, 0, ChId,
					(Vfmc[0].TxMezzType == VFMC_MEZZ_HDMI_PASSIVE) ? 0xC : 0xD);/*0xc 0xb */
			XHdmiphy1_SetTxPreEmphasis(&Hdmiphy1, 0, ChId,
					(Vfmc[0].TxMezzType == VFMC_MEZZ_HDMI_PASSIVE) ? 0x1 : 0x5);/*1, A */
			XHdmiphy1_SetTxPostCursor(&Hdmiphy1, 0, ChId,
					(Vfmc[0].TxMezzType == VFMC_MEZZ_HDMI_PASSIVE) ? 0x1 : 0x5);/*1, B */
#elif defined (XPS_BOARD_VCU118)
			XHdmiphy1_SetTxVoltageSwing(&Hdmiphy1, 0, ChId,
					(Vfmc[0].TxMezzType == VFMC_MEZZ_HDMI_PASSIVE) ? 0xC : 0xD);/*0xc 0xb */
			XHdmiphy1_SetTxPreEmphasis(&Hdmiphy1, 0, ChId,
					(Vfmc[0].TxMezzType == VFMC_MEZZ_HDMI_PASSIVE) ? 0x1 : 0x3);/*1, A */
			XHdmiphy1_SetTxPostCursor(&Hdmiphy1, 0, ChId,
					(Vfmc[0].TxMezzType == VFMC_MEZZ_HDMI_PASSIVE) ? 0x1 : 0x3);/*1, B */
#elif defined (XPS_BOARD_VCK190)
			XHdmiphy1_SetTxVoltageSwing(&Hdmiphy1, 0, ChId,
					(Vfmc[0].TxMezzType == VFMC_MEZZ_HDMI_PASSIVE) ? 0xD : 0xD);/*0xc 0xb */
			XHdmiphy1_SetTxPreEmphasis(&Hdmiphy1, 0, ChId,
					(Vfmc[0].TxMezzType == VFMC_MEZZ_HDMI_PASSIVE) ? 0x1 : 0x1);/*1, A */
			XHdmiphy1_SetTxPostCursor(&Hdmiphy1, 0, ChId,
					(Vfmc[0].TxMezzType == VFMC_MEZZ_HDMI_PASSIVE) ? 0x1 : 0x2);/*1, B */
#elif defined (XPS_BOARD_VEK280_ES)
			XHdmiphy1_SetTxVoltageSwing(&Hdmiphy1, 0, ChId,
					(Vfmc[0].TxMezzType == VFMC_MEZZ_HDMI_PASSIVE) ? 0xD : 0xD);/*0xc 0xb */
			XHdmiphy1_SetTxPreEmphasis(&Hdmiphy1, 0, ChId,
					(Vfmc[0].TxMezzType == VFMC_MEZZ_HDMI_PASSIVE) ? 0x1 : 0x1);/*1, A */
			XHdmiphy1_SetTxPostCursor(&Hdmiphy1, 0, ChId,
					(Vfmc[0].TxMezzType == VFMC_MEZZ_HDMI_PASSIVE) ? 0x1 : 0x2);/*1, B */
#else
/* Place holder for future board support, Below Value just a random value */
			XHdmiphy1_SetTxVoltageSwing(&Hdmiphy1, 0, ChId, 0xD);
			XHdmiphy1_SetTxPreEmphasis(&Hdmiphy1, 0, ChId,
					(Vfmc[0].TxMezzType == VFMC_MEZZ_HDMI_PASSIVE) ? 0x1 : 0x5);
			XHdmiphy1_SetTxPostCursor(&Hdmiphy1, 0, ChId,
					(Vfmc[0].TxMezzType == VFMC_MEZZ_HDMI_PASSIVE) ? 0x1 : 0x5);
#endif
#endif
		}
	}

	/* Initialize IDT to make sure LoL is LOW */
	I2cClk(0, 400000000);
	/* Delay 50ms to allow clockgen to lock */
	usleep (50000);

	/**
	 *  Initialize the controller for the example design mode.
	 */
	xil_printf("Initializing Example design controller. \r\n");
	Exdes_InitController(&xhdmi_exdes_ctrlr);

	/**
	 * Initialize the TPG and associated GPIO to reset the TPG.
	 */
#ifdef XPAR_XV_HDMITXSS1_NUM_INSTANCES
#ifdef XPAR_XV_TPG_NUM_INSTANCES
	xil_printf("Initializing TPG. \r\n");

	Status = Exdes_TpgInitialize(XPAR_V_TPG_SS_0_AXI_GPIO_DEVICE_ID,
			XPAR_V_TPG_SS_0_V_TPG_DEVICE_ID);

	if (Status != XST_SUCCESS) {
		xil_printf("Error in initializing TPG and "
				"GPIO to reset the TPG !! \r\n");
	} else {
		xil_printf("TPG and connected GPIO "
				"successfully initialized. \r\n");
	}
#endif

#ifdef XPAR_XV_AXI4S_REMAP_NUM_INSTANCES
	xil_printf("Initializing In-Remapper and Out-Remapper. \r\n");

	Status = Exdes_RemapInitialize(XPAR_XV_AXI4S_REMAP_0_DEVICE_ID,
			XPAR_XV_AXI4S_REMAP_1_DEVICE_ID);

	if (Status != XST_SUCCESS) {
		xil_printf("Error in initializing InRemap and "
				"OutRemap for 8PPC & 8kp60@YUV420!! \r\n");
	} else {
		xil_printf("InRemap and OutRemap "
				"successfully initialized. \r\n");
	}
#endif
#endif
#if defined (XPAR_XV_FRMBUFWR_NUM_INSTANCES) && \
                      (XPAR_XV_FRMBUFWR_NUM_INSTANCES)
	Status = Exdes_FBInitialize (&FrameBufWr,&FrameBufRd,&Gpio_VFRB_resetn);
	if (Status != XST_SUCCESS) {
		xil_printf("Error in initializing Video Frame Buffer and "
				"GPIO to reset the Video Frame Buffer !! \r\n");
	} else {
		xil_printf("Video Frame Buffer and GPIO to reset the Video Frame Buffer"
				" successfully initialized\r\n");
	}
#endif

#if defined	(XPAR_V_HDMI_RXSS1_DSC_EN) && \
		defined (XPAR_XAXIS_SWITCH_NUM_INSTANCES)
	Status = Exdes_AxisSwitchInitialize (&HdmiRxAxiSwitch,XPAR_AXIS_SWITCH_0_DEVICE_ID);
	if (Status != XST_SUCCESS) {
		xil_printf("Error in initializing HDMI 2.1 RX AXI Stream Switch !! \r\n");
	} else {
		xil_printf("HDMI 2.1 RX AXI Stream Switch"
				" successfully initialized\r\n");
	}
#endif

	xil_printf("---------------------------------\r\n");

	/* Enable exceptions. */
	Xil_AssertSetCallback((Xil_AssertCallback) Xil_AssertCallbackRoutine);
	Xil_ExceptionEnable();

	/* Initialize the system timer */
	Exdes_SysTmrInitialize(&xhdmi_exdes_ctrlr,
					XPAR_TMRCTR_1_DEVICE_ID,
#if defined(__arm__) || (__aarch64__)
					XPAR_FABRIC_TMRCTR_1_VEC_ID);
#else
					XPAR_INTC_0_TMRCTR_1_VEC_ID);
#endif

	/* Initialize menu */
	XHdmi_MenuInitialize(&HdmiMenu, UART_BASEADDR,
			&xhdmi_exdes_ctrlr.ForceIndependent,
			&xhdmi_exdes_ctrlr.SystemEvent,
			&xhdmi_exdes_ctrlr.IsTxPresent,
			&xhdmi_exdes_ctrlr.IsRxPresent,
#if defined (XPAR_XV_HDMITXSS1_NUM_INSTANCES) && \
	defined (XPAR_XV_HDMIRXSS1_NUM_INSTANCES)
			Exdes_ChangeColorbarOutput,
			Exdes_ConfigureTpgEnableInput,
			ToggleHdmiRxHpd);
#elif defined(XPAR_XV_HDMIRXSS1_NUM_INSTANCES)
			NULL,
			NULL,
			ToggleHdmiRxHpd);
#elif defined(XPAR_XV_HDMITXSS1_NUM_INSTANCES)
			Exdes_ChangeColorbarOutput,
			Exdes_ConfigureTpgEnableInput,
			NULL);
#endif

#if defined(XPAR_XV_HDMITXSS1_NUM_INSTANCES)
	/* Reset the TX Info Frame. */
	TxInfoFrameReset();
#endif
#if defined (XPAR_XV_FRMBUFWR_NUM_INSTANCES) && \
                      (XPAR_XV_FRMBUFWR_NUM_INSTANCES)
	ResetFrameBuf(0x2);
#endif
#if defined(XPAR_XV_HDMITXSS1_NUM_INSTANCES)
	/* Enable Scrambling Override
	 * Note: Setting the override to TRUE will allow scrambling to be
	 *       disabled for video where TMDS Clock > 340 MHz which breaks the
	 *       HDMI Specification
	 * E.g.:
	 *   XV_HdmiTxSs1_SetVideoStreamScramblingOverrideFlag(&HdmiTxSs, TRUE);
	 */

	u8 SinkReady = FALSE;
#endif
#if defined(XPAR_XV_HDMIRXSS1_NUM_INSTANCES) && defined(XPAR_XV_HDMITXSS1_NUM_INSTANCES)
	/* Start the system in pass-through mode. */
	xhdmi_exdes_ctrlr.ForceIndependent = FALSE;
#else
	xhdmi_exdes_ctrlr.ForceIndependent = TRUE;
#if defined(XPAR_XV_HDMIRXSS1_NUM_INSTANCES)
	xhdmi_exdes_ctrlr.IsTxPresent = 0;
#endif
#if defined(XPAR_XV_HDMITXSS1_NUM_INSTANCES)
	xhdmi_exdes_ctrlr.IsRxPresent = 0;
#endif
#endif
#if defined(XPAR_XV_HDMIRXSS1_NUM_INSTANCES)
	/* Set the default Link Training Patterns to be requested by RX
	 * during FRL Link Training
	 */
	XV_HdmiRx1_FrlLtp DefaultLtp;
	DefaultLtp.Byte[0] = XV_HDMIRX1_LTP_LFSR0;
	DefaultLtp.Byte[1] = XV_HDMIRX1_LTP_LFSR1;
	DefaultLtp.Byte[2] = XV_HDMIRX1_LTP_LFSR2;
	DefaultLtp.Byte[3] = XV_HDMIRX1_LTP_LFSR3;
	XV_HdmiRxSs1_FrlModeEnable(&HdmiRxSs, 150, DefaultLtp, TRUE);
#endif
#if defined(XPAR_XV_HDMITXSS1_NUM_INSTANCES)
	/* Set the data and clock selection on channel 4
	 * on the TX FMC Mezzanine card. */
	Vfmc_Gpio_Ch4_DataClock_Sel(&Vfmc[0], VFMC_GPIO_TX_CH4_As_DataAndClock);
	XHdmiphy1_Hdmi20Config(&Hdmiphy1, 0, XHDMIPHY1_DIR_TX);
#endif
#if defined(XPAR_XV_HDMIRXSS1_NUM_INSTANCES)
	/* Set the clock selection on channel 4
	 * on the RX FMC Mezzanine card. */
	Vfmc_Gpio_Ch4_DataClock_Sel(&Vfmc[0], VFMC_GPIO_RX_CH4_As_Clock);
	XHdmiphy1_Hdmi20Config(&Hdmiphy1, 0, XHDMIPHY1_DIR_RX);
#endif
#if defined(XPAR_XV_HDMITXSS1_NUM_INSTANCES)
	Exdes_ChangeColorbarOutput(XVIDC_VM_1920x1080_60_P,
			XVIDC_CSF_RGB, XVIDC_BPC_8);

	/* Declare the maximum FRL_Rate supported by TX */
	XV_HdmiTxSs1_SetFrlMaxFrlRate(&HdmiTxSs, HdmiTxSs.Config.MaxFrlRate);

	/* Declare the FFE_Levels supported by TX */
	XV_HdmiTxSs1_SetFfeLevels(&HdmiTxSs, 0);
	XV_HdmiTxSs1_Start(&HdmiTxSs);
#endif
#if defined(XPAR_XV_HDMIRXSS1_NUM_INSTANCES)
	XV_HdmiRxSs1_Start(&HdmiRxSs);
#endif
	/* Start the system timer to generate a repetitive pulse to
	 * handle exceptions on counters for HDMI TX.
	 */
	/* Setting the periodic interval to 100ms. */
	Exdes_StartSysTmr(&xhdmi_exdes_ctrlr, 100);
#if defined (XPAR_XV_FRMBUFWR_NUM_INSTANCES) && \
                      (XPAR_XV_FRMBUFRD_NUM_INSTANCES)
	/* Set Default Address for Dynamic HDR */
	XV_HdmiRxSs1_DynHDR_SetAddr(&HdmiRxSs,VidBuff[DynHdr_wr_buff_offset].DynHDRBaseAddr);
#endif
	/* Main loop */
	do {

#if defined(USE_HDCP_HDMI_RX) || defined(USE_HDCP_HDMI_TX)
#if defined (XPAR_XV_HDMITXSS1_NUM_INSTANCES) && \
	defined (XPAR_XV_HDMIRXSS1_NUM_INSTANCES)
		if (XV_HdmiRxSs1_HdcpIsReady(&HdmiRxSs) &&
		    XV_HdmiTxSs1_HdcpIsReady(&HdmiTxSs)) {
#elif defined (XPAR_XV_HDMITXSS1_NUM_INSTANCES)
		if (XV_HdmiTxSs1_HdcpIsReady(&HdmiTxSs)) {
#elif defined (XPAR_XV_HDMIRXSS1_NUM_INSTANCES)
		if (XV_HdmiRxSs1_HdcpIsReady(&HdmiRxSs)) {
#endif
			/* Poll HDCP */
			XHdcp_Poll();/*&HdcpRepeater); */
		}
#endif
#if defined(XPAR_XV_HDMITXSS1_NUM_INSTANCES)
		SinkReady = SinkReadyCheck(&HdmiTxSs, &EdidHdmi_t);
#endif
		/* Check if the example design has been triggered from
		 * the Hdmi Rx/Tx state machine layer.l
		 */
		if (xhdmi_exdes_ctrlr.SystemEvent == TRUE) {
			/* Clear the 'System Event' trigger. */
			xhdmi_exdes_ctrlr.SystemEvent = FALSE;
#if defined(XPAR_XV_HDMITXSS1_NUM_INSTANCES)
			/* Determine the source for the transmitter. */
			TxInputSrc = Exdes_DetermineTxSrc();

			/* Update the transmitter stream parameters. */
			if (Exdes_UpdateTxParams(&xhdmi_exdes_ctrlr,
			                         TxInputSrc) ==
			    XST_SUCCESS) {
				xhdmi_exdes_ctrlr.TxStartTransmit = TRUE;
			}
#endif
		}
#if defined(XPAR_XV_HDMITXSS1_NUM_INSTANCES)
		/* Poll the Example design controller for the need
		 * to [re]start TX. */
		if (xhdmi_exdes_ctrlr.TxStartTransmit == TRUE && SinkReady) {
			/* xil_printf("xhdmi_exdes_ctrlr.TxStartTransmit\r\n"); */

			/* Ideally, here, we should get the video stream
			 * from Rx, but we have already copied the Rx stream
			 * to Tx, so getting the stream from TX should be an
			 * additional check for the stability
			 * of the software flow. */

			HdmiTxSsVidStreamPtr =
				XV_HdmiTxSs1_GetVideoStream(
					xhdmi_exdes_ctrlr.hdmi_tx_ctlr->HdmiTxSs);


			/* /\* Setup the AVI Info Frame. *\/ */
			/* Exdes_UpdateAviInfoFrame(HdmiTxSsVidStreamPtr); */

			/* Setup the Tx Stream and trigger it */
			Status = XV_Tx_VideoSetupAndStart(
						xhdmi_exdes_ctrlr.hdmi_tx_ctlr,
						HdmiTxSsVidStreamPtr);

			if (Status != XST_SUCCESS) {
				xil_printf("XV_Tx_VideoSetupAndStart Failure\r\n");
			} else {
				xil_printf("XV_Tx_VideoSetupAndStart Success\r\n");
			}

			/* Set VRR Mode */
#if (!defined XPAR_XVTC_NUM_INSTANCES) || (defined (XPAR_XV_FRMBUFRD_NUM_INSTANCES) && \
                      (XPAR_XV_FRMBUFWR_NUM_INSTANCES))
#if defined (VTEM2FSYNC)
			HdmiRxVrrInfoFrameVRRPtr = XV_HdmiRxSs1_GetVrrIf(&HdmiRxSs);
			HdmiRxSsVidStreamVRRPtr  = XV_HdmiRxSs1_GetVideoStream(&HdmiRxSs);
			if(HdmiRxVrrInfoFrameVRRPtr->VrrIfType == XV_HDMIC_VRRINFO_TYPE_VTEM) {

				  BaseFrameRate_VRR = HdmiRxSsVidStreamVRRPtr->BaseFrameRate ;
				  VrrInforFrame.SrcProdDescIF.FreeSync.FreeSyncMaxRefreshRate  = BaseFrameRate_VRR;
			}

#endif
            /* Copy VTEM/FSYNC Packet Data */
			if(VrrInforFrame.VrrIfType == XV_HDMIC_VRRINFO_TYPE_VTEM) {
				XV_HdmiTxSs1_VrrControl(&HdmiTxSs,TRUE);
				XV_HdmiTxSs1_SetVrrIf(&HdmiTxSs,&VrrInforFrame);
			} else if(VrrInforFrame.VrrIfType == XV_HDMIC_VRRINFO_TYPE_SPDIF) {

				XV_HdmiTxSs1_FSyncControl(&HdmiTxSs,TRUE);

				XV_HdmiTxSs1_SetVrrIf(&HdmiTxSs,&VrrInforFrame);
			} else {
				XV_HdmiTxSs1_VrrControl(&HdmiTxSs,FALSE);
				XV_HdmiTxSs1_FSyncControl(&HdmiTxSs,FALSE);
			}

#if defined (XPAR_XV_FRMBUFRD_NUM_INSTANCES) && \
                      (XPAR_XV_FRMBUFWR_NUM_INSTANCES)
            // Copy only for PT
			if ( (xhdmi_exdes_ctrlr.ForceIndependent == FALSE) &&
				 (xhdmi_exdes_ctrlr.IsTxPresent == TRUE) &&
				 (HdmiTxSs.IsStreamUp == TRUE)  &&
				 (xhdmi_exdes_ctrlr.IsRxPresent == TRUE)
			 ) {

#if (VRR_MODE ==2) // AUTO STRETCH
		 XV_HdmiTxSS1_SetVrrVfpStretch(&HdmiTxSs,0xFFF);
#elif (VRR_MODE == 1) // MANUAL STRETCH
		 Exdes_ProcessVRRTimingChange();
#endif
			}
#endif

#endif

			/* Disable the TxStartTransmit flag */
			xhdmi_exdes_ctrlr.TxStartTransmit = FALSE;
#if defined (XPAR_XV_HDMITXSS1_NUM_INSTANCES) && \
                 defined (XPAR_XV_HDMIRXSS1_NUM_INSTANCES)
			EXDES_AUXFIFO_DBG_PRINT("%s,%d. AuxFifoStartFlag = %d\r\n",
					__func__, __LINE__,
					AuxFifoStartFlag);
			/* Reset the AUX fifo to avoid "HW Aux Full" condition in case
			 * the Rx is receiving aux and the Tx is coming up.
			 *
			 * If the Aux fifo is reset in setting the tx ref clk,
			 * resetting it here is not required.
			 *
			 * ResetAuxFifo();
			 */
#endif
		}
#endif

		/* HDMI Menu */
		XHdmi_MenuProcess(&HdmiMenu, xhdmi_exdes_ctrlr.TxBusy);

		/* VPHY error */
		Hdmiphy1ProcessError();

#ifdef XPAR_XV_HDMITXSS1_NUM_INSTANCES
		/* HDMI TX error */
		HdmiTxProcessError();
#endif

	} while (1);

	return 0;
}
