/******************************************************************************
 * @file vmix_hdmi_bridge.c
 * @brief VMix + HDMI TX Bridge Implementation for ISP Pipeline Output
 *
 * Implements the VMix + HDMI TX bridge module that routes ISP output
 * through the Video Mixer to an HDMI 2.1 TX display.
 *
 * Ported from app_component/Sensor_example/mipi_cfg.c VMix logic and
 * adapted to use the latest HDMI 2.1 SS APIs (XV_HdmiTxSs1 / XV_Tx).
 *
 * Copyright (C) 2024 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
 ******************************************************************************/

#include "vmix_hdmi_bridge.h"
#include "xil_printf.h"
#include "xparameters.h"
#include "xscugic.h"

#ifdef XPAR_XV_FRMBUF_WR_NUM_INSTANCES
#include "xv_frmbufwr_l2.h"
#endif

/*
 * To enable HDMI TX output, define XPAR_XV_HDMITXSS1_NUM_INSTANCES and copy the
 * HDMI example files from HDMI_TEST/src/ into this project:
 *   xhdmi_exdes_sm_tx.h/c, xhdmi_intr.h/c,
 *   video_fmc.h/c, xhdmi_edid.h/c, idt_8t49n24x.h/c,
 *   ti_lmk03318.h/c, onsemi_nb7nq621m.h/c, si5344drv.h/c,
 *   ti_tmds1204.h/c, rc21008adrv.h/c, platform.h/c
 */
#ifdef XPAR_XV_HDMITXSS1_NUM_INSTANCES
#include "xhdmi_exdes_sm_tx.h"
#include "xhdmiphy1.h"
#include "video_fmc.h"
#include "xhdmi_edid.h"
#include "xiic.h"
#include "xtmrctr.h"
#endif

#ifdef XPAR_XV_MIX_NUM_INSTANCES

#define LOGTAG "VMIX-BRIDGE"

/************************** External References ******************************/

extern XScuGic Intc;
#ifdef XPAR_XV_FRMBUF_WR_NUM_INSTANCES
extern XV_FrmbufWr_l2 frmbufwr[];
extern int fbwr_enable[];
extern int fbwr_layerID[];
extern XVidC_ColorFormat FBWR_Cfmt[];
#endif

/************************** Module Variables *********************************/

/** ATM (Address Translation Mode) enable flag - set to 0 for direct addressing */
static int ATM = 1;
#define ATM_HIGH_MEM_PREFIX 0x8

#ifdef XPAR_XV_MIX_NUM_INSTANCES
/** VMix L2 driver instance */
static XV_Mix_l2 mix;

/** Per-layer tracking pool */
static VmixHdmiBridge_LayerPool Layer_Handle[VMIX_BRIDGE_MAX_LAYERS];

/** Instance-to-layer mapping table: InstancetoLayerMap[instId][bufIo] = layerId */
static int InstancetoLayerMap[VMIX_BRIDGE_MAX_INSTANCES][VMIX_BRIDGE_MAX_OUTPUT_PATHS];

/** Counter for assigning display start coordinates */
static int coordinate_counter = 0;

/** Static frame counter for debug prints */
static int vmix_update_cnt = 1;

/** Per-layer window configuration (layer 0 is master, so index = layerId - 1) */
static XVidC_VideoWindow MixLayerConfig[VMIX_BRIDGE_MAX_LAYERS - 1] = {
	/* X     Y      W      H   */
	{   0,    0, 1920, 1080 }, /* Layer 1  */
	{   0,    0, 1920, 1080 }, /* Layer 2  */
	{   0,    0, 1920, 1080 }, /* Layer 3  */
	{   0,    0, 1920, 1080 }, /* Layer 4  */
	{   0,    0, 1920, 1080 }, /* Layer 5  */
	{   0,    0, 1920, 1080 }, /* Layer 6  */
	{   0,    0, 1920, 1080 }, /* Layer 7  */
	{   0,    0, 1920, 1080 }, /* Layer 8  */
	{1920,    0, 1920, 1080 }, /* Layer 9  */
	{   0, 1080, 1920, 1080 }, /* Layer 10 */
	{   0,    0, 1920, 1080 }, /* Layer 11 */
	{   0,    0, 1920, 1080 }, /* Layer 12 */
	{1920,    0, 1920, 1080 }, /* Layer 13 */
	{   0, 1080, 1920, 1080 }, /* Layer 14 */
	{1920, 1080, 1920, 1080 }, /* Layer 15 */
	{1920, 1080, 1920, 1080 }  /* Layer 16 */
};

/** Display start coordinate presets for multi-view layout */
static VmixHdmiBridge_Coordinate start_coords[5] = {
	{    0,    0 },
	{    0, 1080 },
	{ 1920,    0 },
	{ 1920, 1080 },
	{  960,  540 }
};

/** Supported display resolutions */
static VmixHdmiBridge_Resolution Supported_resolutions[VMIX_BRIDGE_NUM_RESOLUTIONS] = {
	{  480,  270 },
	{  640,  480 },
	{  800,  600 },
	{  960,  540 },
	{ 1024,  768 },
	{ 1280,  720 },
	{ 1280, 1024 },
	{ 1600, 1200 },
	{ 1920, 1080 },
	{ 1920, 1200 },
	{ 2560, 1920 },
	{ 3840, 2160 },
	{ 2592, 1944 },
	{  864,  480 },
	{  800,  480 },
	{ 3280, 2464 },
	{ 1928, 1088 },
	{ 2592, 1944 }
};
#endif /* XPAR_XV_MIX_NUM_INSTANCES */

#ifdef XPAR_XV_HDMITXSS1_NUM_INSTANCES
/** HDMI TX 2.1 Subsystem instance */
static XV_HdmiTxSs1 HdmiTxSs;

/** HDMI PHY instance */
static XHdmiphy1 Hdmiphy1;

/** HDMI TX state machine controller */
static XV_Tx xhdmi_tx_controller;

/** Video FMC (mezzanine) controller */
static XVfmc Vfmc[1];

/** AXI IIC instance for HDMI DDC/clock programming */
static XIic Iic;

/** Parsed EDID data from downstream sink (defined as extern in xhdmi_edid.h) */
EdidHdmi EdidHdmi_t;

/** Target video mode for TX output */
static XVidC_VideoMode HdmiTxVideoMode;

/** Flag: TX start requested by state machine */
static u8 TxStartTransmit = FALSE;

/** Flag: TX busy during init */
static u8 TxBusy = TRUE;

/** I2C Repeated Start / Stop defines */
#define I2C_REPEATED_START 0x01
#define I2C_STOP           0x00
#define I2C_CLK_ADDR       0x7C
#define I2C_CLK_ADDR1      0x6C

/************************** HDMI TX Trigger Callback Declarations ************/
static void HdmiTx_CableConnectionChange(void *InstancePtr);
static void HdmiTx_SetupTxTmdsRefClk(void *InstancePtr);
static void HdmiTx_SetupTxFrlRefClk(void *InstancePtr);
static void HdmiTx_GetFrlClk(void *InstancePtr);
static void HdmiTx_SetupAudioVideo(void *InstancePtr);
static void HdmiTx_StreamOn(void *InstancePtr);
static void HdmiTx_StreamOff(void *InstancePtr);
static void HdmiTx_VidSyncRecv(void *InstancePtr);
static void HdmiTx_EnableCableDriver(void *InstancePtr);
static void HdmiTx_FrlFfeConfigDevice(void *InstancePtr);
static void HdmiTx_FrlConfigDeviceSetup(void *InstancePtr);
static void HdmiTx_ReadyToStartTransmit(void *InstancePtr);
static void Hdmiphy1ErrorCallback(void *CallbackRef);

/** EDID downstream sink capability check */
static u32 CheckDwnstrmSinkCaps(void);
static u8 SinkReady(void);
#endif /* XPAR_XV_HDMITXSS1_NUM_INSTANCES */

/************************** Forward Declarations *****************************/

#ifdef XPAR_XV_MIX_NUM_INSTANCES
static int SetVmixLayerandChromaBuffer(XV_Mix_l2 *MixerPtr, int Layer,
					UINTPTR Addr, uint32_t isfbwrbuffer);
#endif

/************************** Function Implementations *************************/

/*****************************************************************************/
/**
 * VmixHdmiBridge_Init - Initialize VMix + HDMI TX bridge
 *
 * Performs the complete initialization sequence:
 *  1. Reset instance-to-layer map and layer pool
 *  2. Initialize VMix IP via XVMix_Initialize
 *  3. (Optional) Initialize HDMI TX 2.1 SS
 *  4. Configure VMix master stream for 4K30 display
 *  5. Start the VMix core
 *
 * @return 0 on success, negative on failure
 *****************************************************************************/
int VmixHdmiBridge_Init(void)
{
	u32 Status;
	int i, j;

	xil_printf("\r\n[VMIX-BRIDGE] Initializing VMix + HDMI Bridge...\r\n");

#ifdef XPAR_XV_MIX_NUM_INSTANCES
	/* Reset mapping tables */
	for (i = 0; i < VMIX_BRIDGE_MAX_INSTANCES; i++) {
		for (j = 0; j < VMIX_BRIDGE_MAX_OUTPUT_PATHS; j++) {
			InstancetoLayerMap[i][j] = -1;
		}
	}
	for (i = 0; i < VMIX_BRIDGE_MAX_LAYERS; i++) {
		Layer_Handle[i].LayerID = 0;
		Layer_Handle[i].LayerCfmt = 0;
		Layer_Handle[i].IsEnabled = 0;
		Layer_Handle[i].IsMapped = 0;
		Layer_Handle[i].MappedInstanceId = -1;
		Layer_Handle[i].Stride = 0;
	}
#ifdef XPAR_XV_FRMBUF_WR_NUM_INSTANCES
	for (i = 0; i < XPAR_XV_FRMBUF_WR_NUM_INSTANCES; i++) {
		fbwr_layerID[i] = -1;
	}
#endif
	coordinate_counter = 0;
	vmix_update_cnt = 1;

	/* Initialize VMix L2 driver */
#ifndef SDT
	Status = XVMix_Initialize(&mix, XPAR_VMIX_SS_V_MIX_DEVICE_ID);
#else
	Status = XVMix_Initialize(&mix, XPAR_VMIX_SS_V_MIX_BASEADDR);
#endif
	if (Status != XST_SUCCESS) {
		xil_printf("[VMIX-BRIDGE] ERROR: VMix device not found (status=%d)\r\n", Status);
		return -1;
	}
	xil_printf("[VMIX-BRIDGE] VMix Initialized\r\n");

	/* Configure VMix master stream for 4K30 output */
	{
		XVidC_VideoStream VidStream;
		XVidC_VideoTiming const *TimingPtr;
		XVidC_VideoMode VideoMode = XVIDC_VM_3840x2160_30_P;
		XVidC_ColorFormat Cfmt = XVIDC_CSF_YCRCB_422;
		XVidC_VideoMode resIdOut;

		/* Query master layer format from HW */
		XVMix_GetLayerColorFormat(&mix, XVMIX_LAYER_MASTER, &Cfmt);

		resIdOut = XVidC_GetVideoModeId(3840, 2160, XVIDC_FR_30HZ, FALSE);
		VidStream.PixPerClk = mix.Mix.Config.PixPerClk;
		VidStream.ColorDepth = mix.Mix.Config.MaxDataWidth;
		VidStream.ColorFormatId = Cfmt;
		VidStream.VmId = resIdOut;

		/* Get mode timing parameters */
		TimingPtr = XVidC_GetTimingInfo(VidStream.VmId);
		VidStream.Timing = *TimingPtr;
		VidStream.FrameRate = XVidC_GetFrameRate(VidStream.VmId);

		xil_printf("[VMIX-BRIDGE] Master Stream: %s (%s)\r\n",
			   XVidC_GetVideoModeStr(VidStream.VmId),
			   XVidC_GetColorFormatStr(Cfmt));

		/* Configure mixer with master stream */
		Status = VmixHdmiBridge_ConfigureMixer(&VidStream);
		if (Status != 0) {
			xil_printf("[VMIX-BRIDGE] ERROR: VMix configuration failed\r\n");
			return -2;
		}
	}
#else
	xil_printf("[VMIX-BRIDGE] WARNING: VMix IP not present in HW design\r\n");
#endif /* XPAR_XV_MIX_NUM_INSTANCES */

#ifdef XPAR_XV_HDMITXSS1_NUM_INSTANCES
	/* Initialize HDMI TX 2.1 Subsystem */
	Status = VmixHdmiBridge_ConfigHdmiTx(XVIDC_VM_3840x2160_30_P);
	if (Status != XST_SUCCESS) {
		xil_printf("[VMIX-BRIDGE] ERROR: HDMI TX init failed (status=%d)\r\n", Status);
		return -3;
	}
	xil_printf("[VMIX-BRIDGE] HDMI TX Initialized\r\n");
#else
	xil_printf("[VMIX-BRIDGE] INFO: HDMI TX not enabled (XPAR_XV_HDMITXSS1_NUM_INSTANCES not defined)\r\n");
#endif

	xil_printf("[VMIX-BRIDGE] Initialization complete\r\n");
	return 0;
}

/*****************************************************************************/
/**
 * VmixHdmiBridge_ConfigHdmiTx - Configure and start HDMI TX output
 *
 * Uses the latest XV_HdmiTxSs1 / XV_Tx APIs from HDMI_TEST to set up
 * the HDMI transmitter with the specified video mode.
 *
 * @param VideoMode - Target video mode (e.g., XVIDC_VM_3840x2160_30_P)
 * @return XST_SUCCESS on success
 *****************************************************************************/
int VmixHdmiBridge_ConfigHdmiTx(XVidC_VideoMode VideoMode)
{
#ifdef XPAR_XV_HDMITXSS1_NUM_INSTANCES
	u32 Status;

	xil_printf("[VMIX-BRIDGE] Configuring HDMI TX for mode: %s\r\n",
		   XVidC_GetVideoModeStr(VideoMode));

	HdmiTxVideoMode = VideoMode;
	TxBusy = TRUE;

	/* ---- Step 1: Initialize AXI IIC for HDMI DDC / clock programming ---- */
	{
		XIic_Config *IicCfgPtr;
		IicCfgPtr = XIic_LookupConfig(XPAR_HDMI_SS_AXI_IIC_HDMI_BASEADDR);
		if (IicCfgPtr != NULL) {
			Status = XIic_CfgInitialize(&Iic, IicCfgPtr,
						    IicCfgPtr->BaseAddress);
			if (Status != XST_SUCCESS) {
				xil_printf("[VMIX-BRIDGE] WARNING: IIC init failed\r\n");
			} else {
				XIic_Start(&Iic);
				xil_printf("[VMIX-BRIDGE] AXI IIC initialized and started\r\n");
			}
		} else {
			xil_printf("[VMIX-BRIDGE] WARNING: AXI IIC config not found\r\n");
		}
	}

	/* ---- Step 2: Initialize Video FMC (mezzanine board) ---- */
	Vfmc[0].IicPtr = &Iic;
	Vfmc[0].Loc = VFMC_HPC0;
	Status = Vfmc_HdmiInit(&Vfmc[0],
			       XPAR_HDMI_SS_VFMC_CTLR_SS_0_VFMC_GPIO_BASEADDR,
			       &Iic, VFMC_HPC0);
	if (Status == XST_FAILURE) {
		xil_printf("[VMIX-BRIDGE] WARNING: VFMC init failed (may not be present)\r\n");
	} else {
		xil_printf("[VMIX-BRIDGE] VFMC initialized (TxMezz=0x%x)\r\n",
			   Vfmc[0].TxMezzType);
	}

	/* ---- Step 3: Initialize HDMI TX SS + PHY via state machine helper ---- */
	xhdmi_tx_controller.HdmiTxSs = &HdmiTxSs;
	xhdmi_tx_controller.VidPhy = &Hdmiphy1;
	xhdmi_tx_controller.Intc = &Intc;

#ifndef SDT
	{
		XV_Tx_IntrVecId IntrVecIds;
		IntrVecIds.IntrVecId_HdmiTxSs = XPAR_FABRIC_XV_HDMITXSS1_0_INTR;
		IntrVecIds.IntrVecId_VPhy = XPAR_FABRIC_XV_HDMIPHY1_0_INTR;
		IntrVecIds.IntrVecId_Hdcp14 = 0;
		IntrVecIds.IntrVecId_Hdcp14Timer = 0;
		IntrVecIds.IntrVecId_Hdcp22Timer = 0;
		Status = XV_Tx_Hdmi_Initialize(&xhdmi_tx_controller,
					       XPAR_XV_HDMITXSS1_0_DEVICE_ID,
					       XPAR_XV_HDMIPHY1_0_DEVICE_ID,
					       IntrVecIds);
	}
#else
	Status = XV_Tx_Hdmi_Initialize(&xhdmi_tx_controller,
				       XPAR_XV_HDMITXSS1_0_BASEADDR,
				       XPAR_XV_HDMIPHY1_0_BASEADDR);
#endif
	if (Status != XST_SUCCESS) {
		xil_printf("[VMIX-BRIDGE] ERROR: XV_Tx_Hdmi_Initialize failed (%d)\r\n", Status);
		return XST_FAILURE;
	}
	xil_printf("[VMIX-BRIDGE] HDMI TX SS + PHY initialized\r\n");

	/* Disable debug prints from the state machine layer */
	XV_Tx_SetDebugPrints(NULL);
	XV_Tx_SetDebugStateMachinePrints(NULL);
	XV_Tx_SetDebugTxNewStreamSetupPrints(NULL);

	/* ---- Step 4: Register TX trigger callbacks ---- */
	Status  = XV_Tx_SetTriggerCallbacks(&xhdmi_tx_controller,
			XV_TX_TRIG_HANDLER_CONNECTION_CHANGE,
			(void *)HdmiTx_CableConnectionChange,
			(void *)&xhdmi_tx_controller);
	Status |= XV_Tx_SetTriggerCallbacks(&xhdmi_tx_controller,
			XV_TX_TRIG_HANDLER_SETUP_TXTMDSREFCLK,
			(void *)HdmiTx_SetupTxTmdsRefClk,
			(void *)&xhdmi_tx_controller);
	Status |= XV_Tx_SetTriggerCallbacks(&xhdmi_tx_controller,
			XV_TX_TRIG_HANDLER_SETUP_TXFRLREFCLK,
			(void *)HdmiTx_SetupTxFrlRefClk,
			(void *)&xhdmi_tx_controller);
	Status |= XV_Tx_SetTriggerCallbacks(&xhdmi_tx_controller,
			XV_TX_GET_FRL_CLOCK,
			(void *)HdmiTx_GetFrlClk,
			(void *)&xhdmi_tx_controller);
	Status |= XV_Tx_SetTriggerCallbacks(&xhdmi_tx_controller,
			XV_TX_TRIG_HANDLER_SETUP_AUDVID,
			(void *)HdmiTx_SetupAudioVideo,
			(void *)&xhdmi_tx_controller);
	Status |= XV_Tx_SetTriggerCallbacks(&xhdmi_tx_controller,
			XV_TX_TRIG_HANDLER_STREAM_ON,
			(void *)HdmiTx_StreamOn,
			(void *)&xhdmi_tx_controller);
	Status |= XV_Tx_SetTriggerCallbacks(&xhdmi_tx_controller,
			XV_TX_TRIG_HANDLER_ENABLE_CABLE_DRIVERS,
			(void *)HdmiTx_EnableCableDriver,
			(void *)&xhdmi_tx_controller);
	Status |= XV_Tx_SetTriggerCallbacks(&xhdmi_tx_controller,
			XV_TX_TRIG_HANDLER_VSYNC_RECV,
			(void *)HdmiTx_VidSyncRecv,
			(void *)&xhdmi_tx_controller);
	Status |= XV_Tx_SetTriggerCallbacks(&xhdmi_tx_controller,
			XV_TX_TRIG_HANDLER_STREAM_OFF,
			(void *)HdmiTx_StreamOff,
			(void *)&xhdmi_tx_controller);
	Status |= XV_Tx_SetTriggerCallbacks(&xhdmi_tx_controller,
			XV_TX_TRIG_HANDLER_READYTOSTARTTX,
			(void *)HdmiTx_ReadyToStartTransmit,
			(void *)&xhdmi_tx_controller);
	Status |= XV_Tx_SetTriggerCallbacks(&xhdmi_tx_controller,
			XV_TX_TRIG_HANDLER_FRL_FFE_CONFIG_DEVICE,
			(void *)HdmiTx_FrlFfeConfigDevice,
			(void *)&xhdmi_tx_controller);
	Status |= XV_Tx_SetTriggerCallbacks(&xhdmi_tx_controller,
			XV_TX_TRIG_HANDLER_FRL_CONFIG_DEVICE_SETUP,
			(void *)HdmiTx_FrlConfigDeviceSetup,
			(void *)&xhdmi_tx_controller);

	if (Status != XST_SUCCESS) {
		xil_printf("[VMIX-BRIDGE] WARNING: Some trigger callback registrations failed\r\n");
	}

	/* Set VPHY error callback */
	XHdmiphy1_SetErrorCallback(&Hdmiphy1,
				   (void *)Hdmiphy1ErrorCallback,
				   (void *)&Hdmiphy1);

	/* ---- Step 5: Configure TX for TMDS, set video mode, start SS ---- */
	/* Set TX channel 4 as data+clock */
	Vfmc_Gpio_Ch4_DataClock_Sel(&Vfmc[0], VFMC_GPIO_TX_CH4_As_DataAndClock);
	XHdmiphy1_Hdmi20Config(&Hdmiphy1, 0, XHDMIPHY1_DIR_TX);

	/* Declare maximum FRL rate supported by TX HW */
	XV_HdmiTxSs1_SetFrlMaxFrlRate(&HdmiTxSs, HdmiTxSs.Config.MaxFrlRate);
	XV_HdmiTxSs1_SetFfeLevels(&HdmiTxSs, 0);

	/* Configure the video stream parameters for initial colorbar mode */
	{
		XVidC_VideoStream *HdmiTxSsVidStreamPtr;
		HdmiTxSsVidStreamPtr = XV_HdmiTxSs1_GetVideoStream(&HdmiTxSs);

		HdmiTxSsVidStreamPtr->VmId = VideoMode;
		HdmiTxSsVidStreamPtr->ColorFormatId = XVIDC_CSF_RGB;
		HdmiTxSsVidStreamPtr->ColorDepth = XVIDC_BPC_8;
		HdmiTxSsVidStreamPtr->PixPerClk = XVIDC_PPC_4;
		HdmiTxSsVidStreamPtr->FrameRate = XVidC_GetFrameRate(VideoMode);
		HdmiTxSsVidStreamPtr->Timing = *XVidC_GetTimingInfo(VideoMode);
	}

	/* Start the TX SS — this enables HPD detection, after which the
	 * state machine will drive the stream setup via trigger callbacks. */
	XV_HdmiTxSs1_Start(&HdmiTxSs);

	TxBusy = FALSE;
	TxStartTransmit = FALSE;

	xil_printf("[VMIX-BRIDGE] HDMI TX configuration done (event-driven mode)\r\n");
	return XST_SUCCESS;
#else
	(void)VideoMode;
	xil_printf("[VMIX-BRIDGE] HDMI TX not enabled\r\n");
	return XST_SUCCESS;
#endif
}

/*****************************************************************************/
/**
 * VmixHdmiBridge_ConfigureMixer - Configure VMix master stream and start
 *
 * Sets the VMix master stream resolution/format, enumerates all layer
 * color formats into the layer pool, and starts the mixer.
 *
 * Ported from VMix_User_defined() in mipi_cfg.c
 *
 * @param StreamPtr - Video stream parameters for the master output
 * @return 0 on success
 *****************************************************************************/
int VmixHdmiBridge_ConfigureMixer(XVidC_VideoStream *StreamPtr)
{
#ifdef XPAR_XV_MIX_NUM_INSTANCES
	XV_Mix_l2 *MixerPtr = &mix;
	int NumLayers;
	XVidC_ColorFormat Cfmt;
	int layer_id = 0;

	/* Disable master layer before reconfiguration */
	XVMix_LayerDisable(MixerPtr, XVMIX_LAYER_MASTER);

	/* Set master video stream */
	XVMix_SetVidStream(MixerPtr, StreamPtr);
	XVMix_SetBackgndColor(MixerPtr, XVMIX_BKGND_BLUE, StreamPtr->ColorDepth);

	/* Enumerate all layer color formats into the tracking pool */
	NumLayers = XVMix_GetNumLayers(MixerPtr);
	xil_printf("[VMIX-BRIDGE] Number of Layers = %d\r\n", NumLayers);

	for (layer_id = XVMIX_LAYER_1; layer_id < NumLayers; layer_id++) {
		XVMix_GetLayerColorFormat(MixerPtr, layer_id, &Cfmt);
		Layer_Handle[layer_id].LayerID = layer_id;
		Layer_Handle[layer_id].LayerCfmt = Cfmt;
		Layer_Handle[layer_id].MappedInstanceId = -1;
		xil_printf("[VMIX-BRIDGE] Layer %d Color Format: %s\r\n",
			   layer_id, XVidC_GetColorFormatStr(Cfmt));
	}

	/* Enable auto-start mode (no interrupt, free-running) */
#ifdef VMIX_AUTOSTART
	XVMix_InterruptDisable(MixerPtr);
#else
	XVMix_InterruptEnable(MixerPtr);
#endif

	/* Start the mixer */
	XVMix_Start(MixerPtr);
	xil_printf("[VMIX-BRIDGE] VMix started\r\n");

	return 0;
#else
	return -1;
#endif
}

/*****************************************************************************/
/**
 * VmixHdmiBridge_MapLayer - Map a VMix layer to an ISP instance + output path
 *
 * Finds a free VMix layer matching the required color format and assigns
 * it to the specified ISP instance/path combination. Computes stride based
 * on the color format and sets the layer window in the VMix HW.
 *
 * Ported from MapVmixLayer() in mipi_cfg.c
 *
 * @param outFormat  - ISP output format (resolution, pixel format, bit depth)
 * @param InstanceID - ISP instance ID (0-5)
 * @param hpId       - Hardware pipeline ID
 * @param bufIo      - Buffer chain ID (MP/SP1)
 * @param dataBits   - Output data bit depth (8 or 10)
 * @param outputType - 0=MO (memory output), 1=LO (live output via FBWR)
 * @return 1 on success, 0 if already mapped, -1 on failure
 *****************************************************************************/
int VmixHdmiBridge_MapLayer(CamDevicePipeOutFmt_t outFormat, u8 InstanceID,
			    u8 hpId, CamDeviceBufChainId_t bufIo,
			    int dataBits, int outputType)
{
#ifdef XPAR_XV_MIX_NUM_INSTANCES
	int fbwr_id = 0;
	XV_Mix_l2 *MixerPtr = &mix;
	int NumLayers, Status;
	XVidC_VideoWindow Win;
	XVidC_ColorFormat Cfmt;
	u32 Stride = 0;
	u16 AXIMMDataWidth = 128;
	u16 MMWidthBytes = AXIMMDataWidth / 8;

#ifdef XPAR_XV_FRMBUF_WR_NUM_INSTANCES
	/* For FBWR (LO) path, compute fbwr_id */
	if (outputType == 1) {
		/* Each ISP has 2 FBWR slots: slot 0 for MP, slot 1 for SP (SP1 or SP2) */
		int lo_slot = (bufIo == CAMDEV_BUFCHAIN_MP) ? 0 : 1;
		fbwr_id = lo_slot + hpId * 2;
		fbwr_enable[fbwr_id] = 1;
	}
#endif

	xil_printf("[VMIX-BRIDGE] MapLayer: instId=%d bufIo=%d dataBits=%d outType=%d\r\n",
		   InstanceID, bufIo, dataBits, outputType);

	/* Select VMix memory format based on ISP format & data bits */
	if ((outFormat.outFormat == CAMDEV_PIX_FMT_YUV422SP) && (dataBits == 8)) {
		Cfmt = XVIDC_CSF_MEM_Y_UV8;
	} else if ((outFormat.outFormat == CAMDEV_PIX_FMT_YUV420SP) && (dataBits == 8)) {
		Cfmt = XVIDC_CSF_MEM_Y_UV8_420;
	} else if ((outFormat.outFormat == CAMDEV_PIX_FMT_YUV400) && (dataBits == 8)) {
		Cfmt = XVIDC_CSF_MEM_Y8;
	} else if ((outFormat.outFormat == CAMDEV_PIX_FMT_RGB888P) ||
		   (outFormat.outFormat == CAMDEV_PIX_FMT_RGB888)) {
		Cfmt = XVIDC_CSF_MEM_RGB8;
	} else if (((outFormat.outFormat == CAMDEV_PIX_FMT_YUV420SP_ALIGNED_MODE0) ||
		    (outFormat.outFormat == CAMDEV_PIX_FMT_YUV420SP_ALIGNED_MODE1)) &&
		   (dataBits == 10)) {
		Cfmt = XVIDC_CSF_MEM_Y_UV10_420;
	} else if (((outFormat.outFormat == CAMDEV_PIX_FMT_YUV422SP_ALIGNED_MODE0) ||
		    (outFormat.outFormat == CAMDEV_PIX_FMT_YUV422SP_ALIGNED_MODE1)) &&
		   (dataBits == 10)) {
		Cfmt = XVIDC_CSF_MEM_Y_UV10;
	} else if ((outFormat.outFormat == CAMDEV_PIX_FMT_YUV400) && (dataBits == 10)) {
		Cfmt = XVIDC_CSF_MEM_Y10;
	} else {
		xil_printf("[VMIX-BRIDGE] ERROR: Unsupported format (ID=%d)\r\n",
			   outFormat.outFormat);
		return -1;
	}

	NumLayers = XVMix_GetNumLayers(MixerPtr);

	/* Check if layer is already mapped for this Instance & bufIo */
	if (InstancetoLayerMap[InstanceID][bufIo] >= 1) {
#ifdef XPAR_XV_FRMBUF_WR_NUM_INSTANCES
		if (outputType == 1) {
			/* LO/FBWR path: reuse the already-mapped layer so FBWR
			 * callbacks can push frame data to the same VMix layer */
			int existing_layer = InstancetoLayerMap[InstanceID][bufIo];
			fbwr_layerID[fbwr_id] = existing_layer;
			fbwr_enable[fbwr_id] = 1;
			/* Don't set IsEnabled here — let the first FBWR callback
			 * set the buffer address first and then enable the layer */
			xil_printf("[VMIX-BRIDGE] FBWR[%d] reusing Layer %d for instId:%d bufIo:%d\r\n",
				   fbwr_id, existing_layer, InstanceID, bufIo);
			return 1;
		}
#endif
		xil_printf("[VMIX-BRIDGE] Layer already enabled for pathId:%d of InstanceID:%d\r\n",
			   bufIo, InstanceID);
		return 0;
	}

	/* Iterate over VMix layers and assign the first free matching layer */
	for (int layer_id = XVMIX_LAYER_1; layer_id < NumLayers; layer_id++) {
		if ((Cfmt == Layer_Handle[layer_id].LayerCfmt) &&
		    !(Layer_Handle[layer_id].IsMapped)) {

			/* Set layer window dimensions */
			MixLayerConfig[layer_id - 1].Width = outFormat.outWidth;
			MixLayerConfig[layer_id - 1].Height = outFormat.outHeight;
			MixLayerConfig[layer_id - 1].StartX = start_coords[coordinate_counter].StartX;
			MixLayerConfig[layer_id - 1].StartY = start_coords[coordinate_counter].StartY;
			coordinate_counter++;

			if (outputType == 0) {
				/* MO path: assign layer to ISP instance */
				Layer_Handle[layer_id].MappedInstanceId = InstanceID;
				InstancetoLayerMap[InstanceID][bufIo] = layer_id;
			} else {
#ifdef XPAR_XV_FRMBUF_WR_NUM_INSTANCES
				/* LO path (FBWR): assign layer to FBWR ID */
				fbwr_layerID[fbwr_id] = layer_id;
				Layer_Handle[layer_id].IsEnabled = 1;
#endif
			}

			Win = MixLayerConfig[layer_id - 1];

			/* Calculate stride based on color format (per PG-243) */
			switch (Cfmt) {
			case XVIDC_CSF_MEM_Y_UV10:
			case XVIDC_CSF_MEM_Y_UV10_420:
			case XVIDC_CSF_MEM_Y10:
				Stride = ((((Win.Width * 4) / 3) + MMWidthBytes - 1) /
					  MMWidthBytes) * MMWidthBytes;
				break;
			case XVIDC_CSF_MEM_Y_UV8:
			case XVIDC_CSF_MEM_Y_UV8_420:
			case XVIDC_CSF_MEM_Y8:
				Stride = ((Win.Width + MMWidthBytes - 1) /
					  MMWidthBytes) * MMWidthBytes;
				break;
			case XVIDC_CSF_MEM_RGB8:
			case XVIDC_CSF_MEM_YUV8:
			case XVIDC_CSF_MEM_BGR8:
				Stride = (((Win.Width * 3) + MMWidthBytes - 1) /
					  MMWidthBytes) * MMWidthBytes;
				break;
			case XVIDC_CSF_MEM_RGB565:
			case XVIDC_CSF_MEM_UYVY8:
			case XVIDC_CSF_MEM_YUYV8:
				Stride = 2 * Win.Width;
				break;
			default:
				Stride = (((Win.Width * 4) + MMWidthBytes - 1) /
					  MMWidthBytes) * MMWidthBytes;
				break;
			}

			Layer_Handle[layer_id].Stride = Stride;

			xil_printf("[VMIX-BRIDGE] Assigning Layer %d Cfmt=%d instId=%d Stride=%d\r\n",
				   layer_id, Cfmt, InstanceID, Stride);

			/* Set layer window in VMix HW */
			Status = XVMix_SetLayerWindow(MixerPtr, layer_id, &Win,
						      Layer_Handle[layer_id].Stride);
			if (Status != XST_SUCCESS) {
				xil_printf("[VMIX-BRIDGE] ERROR: Set layer window failed (%d)\r\n", Status);
				coordinate_counter--;
				if (outputType == 0) {
					InstancetoLayerMap[InstanceID][bufIo] = -1;
				} else {
#ifdef XPAR_XV_FRMBUF_WR_NUM_INSTANCES
					fbwr_layerID[fbwr_id] = -1;
#endif
					Layer_Handle[layer_id].IsEnabled = 0;
				}
				return Status;
			}

			Layer_Handle[layer_id].IsMapped = 1;

			xil_printf("[VMIX-BRIDGE] Layer %d Window (%d, %d, %d, %d) Stride: %d\r\n",
				   layer_id, Win.StartX, Win.StartY,
				   Win.Width, Win.Height, Stride);
			return 1;
		}
	}

	xil_printf("[VMIX-BRIDGE] WARNING: No VMix layer supports format %d (ISP output)\r\n", Cfmt);
	xil_printf("[VMIX-BRIDGE] Available layer formats: ");
	for (int l = XVMIX_LAYER_1; l < NumLayers; l++) {
		xil_printf("Layer%d=%d ", l, Layer_Handle[l].LayerCfmt);
	}
	xil_printf("\r\n");
	xil_printf("[VMIX-BRIDGE] VMix display not supported for this ISP output format\r\n");
	return -1;
#else
	(void)outFormat; (void)InstanceID; (void)hpId; (void)bufIo;
	(void)dataBits; (void)outputType;
	return -1;
#endif
}

/*****************************************************************************/
/**
 * VmixHdmiBridge_UpdateBufferAddr - Update VMix layer buffer with new frame
 *
 * Called per-frame from VsiVvdeviceShowBuffer() to push a new DMA buffer
 * address to the VMix layer assigned to this ISP instance/path.
 *
 * Ported from Update_Buffer_Addr() in mipi_cfg.c
 *
 * @param baseAddress - Physical address of the ISP output buffer
 * @param Vmix_format - Format/resolution of the output buffer
 * @param InstanceID  - ISP instance ID
 * @param bufferIO    - Buffer chain ID (MP/SP1)
 * @return 1 on success, 0 if unsupported resolution, -1 on failure
 *****************************************************************************/
int VmixHdmiBridge_UpdateBufferAddr(u64 baseAddress,
				    CamDevicePipeOutFmt_t Vmix_format,
				    u8 InstanceID,
				    CamDeviceBufChainId_t bufferIO)
{
#ifdef XPAR_XV_MIX_NUM_INSTANCES
	int Status;
	XVidC_ColorFormat Cfmt;
	XV_Mix_l2 *MixerPtr = &mix;

	/* Check layer assignment; if not assigned, return */
	if (InstancetoLayerMap[InstanceID][bufferIO] <= 0) {
		return 1;
	}

	/* Map ISP format to VMix memory format */
	u64 MemAddr = baseAddress;
	/* Apply ATM high-mem prefix for ISP DMA buffers */
	if (ATM) {
		MemAddr = baseAddress | ((u64)ATM_HIGH_MEM_PREFIX << 32);
	}
	if (Vmix_format.outFormat == CAMDEV_PIX_FMT_YUV422SP) {
		Cfmt = XVIDC_CSF_MEM_Y_UV8;
	} else if (Vmix_format.outFormat == CAMDEV_PIX_FMT_YUV422SP_ALIGNED_MODE0 ||
		   Vmix_format.outFormat == CAMDEV_PIX_FMT_YUV422SP_ALIGNED_MODE1) {
		Cfmt = XVIDC_CSF_MEM_Y_UV10;
	} else if (Vmix_format.outFormat == CAMDEV_PIX_FMT_YUV420SP) {
		Cfmt = XVIDC_CSF_MEM_Y_UV8_420;
	} else if (Vmix_format.outFormat == CAMDEV_PIX_FMT_YUV420SP_ALIGNED_MODE0 ||
		   Vmix_format.outFormat == CAMDEV_PIX_FMT_YUV420SP_ALIGNED_MODE1) {
		Cfmt = XVIDC_CSF_MEM_Y_UV10_420;
	} else if (Vmix_format.outFormat == CAMDEV_PIX_FMT_YUV400) {
		Cfmt = XVIDC_CSF_MEM_Y10;
	} else if (Vmix_format.outFormat == CAMDEV_PIX_FMT_RGB888P ||
		   Vmix_format.outFormat == CAMDEV_PIX_FMT_RGB888) {
		Cfmt = XVIDC_CSF_MEM_RGB8;
	} else {
		xil_printf("[VMIX-BRIDGE] ERROR: Invalid format (ID=%d)\r\n",
			   Vmix_format.outFormat);
		return -1;
	}

	int valid_format = 0;
	int layer_id = InstancetoLayerMap[InstanceID][bufferIO];

	/* Verify this layer belongs to the requesting instance */
	if (InstanceID != Layer_Handle[layer_id].MappedInstanceId) {
		return -1;
	}

	/* Semi-planar formats need chroma buffer update */
	if ((Cfmt == XVIDC_CSF_MEM_Y_UV8) || (Cfmt == XVIDC_CSF_MEM_Y_UV8_420) ||
	    (Cfmt == XVIDC_CSF_MEM_Y_UV10) || (Cfmt == XVIDC_CSF_MEM_Y_UV10_420)) {
		/* Validate resolution */
		for (int i = 0; i < VMIX_BRIDGE_NUM_RESOLUTIONS; i++) {
			if (Vmix_format.outWidth == Supported_resolutions[i].Width &&
			    Vmix_format.outHeight == Supported_resolutions[i].Height) {
				Status = SetVmixLayerandChromaBuffer(MixerPtr, layer_id,
								     MemAddr, 1);
				valid_format = 1;
				break;
			}
		}
		if (!valid_format) {
			xil_printf("[VMIX-BRIDGE] WARNING: Unsupported resolution %dx%d\r\n",
				   Vmix_format.outWidth, Vmix_format.outHeight);
			if (Layer_Handle[layer_id].IsMapped) {
				Layer_Handle[layer_id].IsEnabled = 0;
				XVMix_LayerDisable(MixerPtr, layer_id);
			}
			return 0;
		}
	} else if ((Cfmt == XVIDC_CSF_MEM_Y10) || (Cfmt == XVIDC_CSF_MEM_Y8)) {
		/* Luma-only formats */
		Status = SetVmixLayerandChromaBuffer(MixerPtr, layer_id, MemAddr, 1);
		valid_format = 1;
	} else {
		/* Packed formats (RGB, etc.) */
		for (int i = 0; i < VMIX_BRIDGE_NUM_RESOLUTIONS; i++) {
			if (Vmix_format.outWidth == Supported_resolutions[i].Width &&
			    Vmix_format.outHeight == Supported_resolutions[i].Height) {
				Status = XVMix_SetLayerBufferAddr(MixerPtr, layer_id, MemAddr);
				/* Temp fix: write upper 32-bit addr for high mem */
				Xil_Out32(MixerPtr->Mix.Config.BaseAddress +
					  XV_MIX_CTRL_ADDR_HWREG_LAYER1_BUF1_V_DATA +
					  ((layer_id - 1) * 0x100) + 4,
					  ATM_HIGH_MEM_PREFIX);
				valid_format = 1;
				break;
			}
		}
		if (!valid_format) {
			xil_printf("[VMIX-BRIDGE] WARNING: Unsupported resolution %dx%d\r\n",
				   Vmix_format.outWidth, Vmix_format.outHeight);
			if (Layer_Handle[layer_id].IsMapped) {
				Layer_Handle[layer_id].IsEnabled = 0;
				XVMix_LayerDisable(MixerPtr, layer_id);
			}
			return 0;
		}
	}

	if (Status != XST_SUCCESS) {
		xil_printf("[VMIX-BRIDGE] ERROR: Set layer buffer addr failed\r\n");
	} else {
		/* Enable layer if not already enabled */
		if (!(Layer_Handle[layer_id].IsEnabled)) {
			XVMix_LayerEnable(MixerPtr, layer_id);
			XVMix_SetLayerAlpha(MixerPtr, layer_id, XVMIX_ALPHA_MAX);
			Layer_Handle[layer_id].IsEnabled = 1;
		}
	}
	return 1;
#else
	(void)baseAddress; (void)Vmix_format; (void)InstanceID; (void)bufferIO;
	return -1;
#endif
}

/*****************************************************************************/
/**
 * VmixHdmiBridge_UpdateFbwrLayer - Update VMix layer from FBWR callback
 *
 * Called from FBWR done interrupt callbacks to push the latest completed
 * frame buffer read address to the VMix layer assigned to that FBWR.
 *
 * Ported from the FBWR callback VMix update logic in mipi_cfg.c
 *
 * @param fbwr_id     - Frame buffer writer instance ID (0-3)
 * @param readAddr    - Physical address of the latest completed frame
 * @param chromaAddr  - Physical address of chroma plane (0 for packed)
 *****************************************************************************/
void VmixHdmiBridge_UpdateFbwrLayer(int fbwr_id, u64 readAddr, u64 chromaAddr)
{
#if defined(XPAR_XV_MIX_NUM_INSTANCES) && defined(XPAR_XV_FRMBUF_WR_NUM_INSTANCES)
	XV_Mix_l2 *MixerPtr = &mix;
	int index;
	int Status;
	XVidC_ColorFormat Cfmt;

	index = fbwr_layerID[fbwr_id];
	if (index < 0) {
		return; /* No layer mapped for this FBWR */
	}

	Cfmt = FBWR_Cfmt[fbwr_id];

	/* Semi-planar formats require chroma buffer update */
	if ((Cfmt == XVIDC_CSF_MEM_Y_UV8) || (Cfmt == XVIDC_CSF_MEM_Y_UV8_420) ||
	    (Cfmt == XVIDC_CSF_MEM_Y_UV10) || (Cfmt == XVIDC_CSF_MEM_Y_UV10_420)) {
		Status = SetVmixLayerandChromaBuffer(MixerPtr, index, readAddr, 1);
		if (Status != XST_SUCCESS) {
			xil_printf("[VMIX-BRIDGE] ERROR: FBWR layer %d buffer update failed\r\n", index);
		}
	} else {
		/* Packed format - just set buffer address */
		Status = XVMix_SetLayerBufferAddr(MixerPtr, index, readAddr);
		/* Temp fix: write upper 32-bit addr for high mem */
		Xil_Out32(MixerPtr->Mix.Config.BaseAddress +
			  XV_MIX_CTRL_ADDR_HWREG_LAYER1_BUF1_V_DATA +
			  ((index - 1) * 0x100) + 4,
			  ATM_HIGH_MEM_PREFIX);
		if (Status != XST_SUCCESS) {
			xil_printf("[VMIX-BRIDGE] ERROR: FBWR layer %d buffer addr failed\r\n", index);
		}
	}

	/* Enable the layer if not already enabled */
	if (!(Layer_Handle[index].IsEnabled)) {
		XVMix_LayerEnable(MixerPtr, index);
		XVMix_SetLayerAlpha(MixerPtr, index, XVMIX_ALPHA_MAX);
		Layer_Handle[index].IsEnabled = 1;
		xil_printf("[VMIX-BRIDGE] FBWR layer %d enabled (alpha=%d)\r\n", index, XVMIX_ALPHA_MAX);
	}
#else
	(void)fbwr_id; (void)readAddr; (void)chromaAddr;
#endif
}

/*****************************************************************************/
/**
 * VmixHdmiBridge_Cleanup - Clean up all VMix layers and reset mapping
 *
 * Disables all mapped VMix layers, resets layer tracking structures,
 * and clears instance-to-layer mapping tables.
 *
 * Ported from clean_vmix() in mipi_cfg.c
 *****************************************************************************/
void VmixHdmiBridge_Cleanup(void)
{
#ifdef XPAR_XV_MIX_NUM_INSTANCES
	xil_printf("[VMIX-BRIDGE] Cleaning up VMix layers...\r\n");

	for (int layer_id = XVMIX_LAYER_1; layer_id < VMIX_BRIDGE_MAX_LAYERS; layer_id++) {
		if (Layer_Handle[layer_id].IsMapped) {
			XVMix_LayerDisable(&mix, layer_id);
			Layer_Handle[layer_id].IsEnabled = 0;
			Layer_Handle[layer_id].Stride = 0;
			Layer_Handle[layer_id].MappedInstanceId = -1;
			Layer_Handle[layer_id].IsMapped = 0;
		}
	}

	for (int i = 0; i < VMIX_BRIDGE_MAX_INSTANCES; i++) {
		for (int j = 0; j < VMIX_BRIDGE_MAX_OUTPUT_PATHS; j++) {
			InstancetoLayerMap[i][j] = -1;
		}
	}

#ifdef XPAR_XV_FRMBUF_WR_NUM_INSTANCES
	for (int i = 0; i < XPAR_XV_FRMBUF_WR_NUM_INSTANCES; i++) {
		fbwr_layerID[i] = -1;
	}
#endif

	coordinate_counter = 0;
	xil_printf("[VMIX-BRIDGE] Cleanup complete\r\n");
#endif
}

/*****************************************************************************/
/**
 * VmixHdmiBridge_GetFbwrLayerId - Get VMix layer ID for a given FBWR
 *
 * @param fbwr_id - Frame buffer writer ID (0-3)
 * @return VMix layer ID mapped to this FBWR, or -1 if not mapped
 *****************************************************************************/
int VmixHdmiBridge_GetFbwrLayerId(int fbwr_id)
{
#ifdef XPAR_XV_FRMBUF_WR_NUM_INSTANCES
	if (fbwr_id < 0 || fbwr_id >= XPAR_XV_FRMBUF_WR_NUM_INSTANCES) {
		return -1;
	}
	return fbwr_layerID[fbwr_id];
#else
	(void)fbwr_id;
	return -1;
#endif
}

/************************** Static Helper Functions **************************/

#ifdef XPAR_XV_MIX_NUM_INSTANCES
/*****************************************************************************/
/**
 * SetVmixLayerandChromaBuffer - Set VMix layer buffer + chroma address
 *
 * Configures the layer buffer address and computes the chroma plane offset
 * based on stride * height. Supports ATM (Address Translation Mode) for
 * designs that use high-address memory regions.
 *
 * Ported from SetVmixLayerandChromaBuffer() in mipi_cfg.c
 *
 * @param MixerPtr     - Pointer to VMix L2 instance
 * @param Layer        - VMix layer ID
 * @param Addr         - Physical base address of the frame buffer
 * @param isfbwrbuffer - 1 if from FBWR path (direct addr), 0 for ISP DMA
 * @return XST_SUCCESS on success
 *****************************************************************************/
static int SetVmixLayerandChromaBuffer(XV_Mix_l2 *MixerPtr, int Layer,
					UINTPTR Addr, uint32_t isfbwrbuffer)
{
	XVidC_VideoWindow Window_conf;
	u64 MemAddr;
	u64 High_Mem = 8;
	u32 Status = 0;
	int XVMIX_CHROMA_ADDR_OFFSET;

	/* Apply ATM address translation if enabled */
	if (ATM) {
		if (isfbwrbuffer != 0) {
			MemAddr = Addr;
		} else {
			MemAddr = Addr | (High_Mem << 32);
		}
	} else {
		MemAddr = Addr;
	}

	Window_conf = MixLayerConfig[Layer - 1];

	/* Set luma buffer address */
	Status = XVMix_SetLayerBufferAddr(MixerPtr, Layer, MemAddr);
	/* Temp fix: write upper 32-bit addr for high mem */
	Xil_Out32(MixerPtr->Mix.Config.BaseAddress +
		  XV_MIX_CTRL_ADDR_HWREG_LAYER1_BUF1_V_DATA +
		  ((Layer - 1) * 0x100) + 4,
		  ATM_HIGH_MEM_PREFIX);
	if (Status != XST_SUCCESS) {
		xil_printf("[VMIX-BRIDGE] ERROR: Layer %d buffer addr 0x%08X failed\r\n",
			   Layer, (u32)MemAddr);
	}

	/* Compute and set chroma buffer address */
	XVMIX_CHROMA_ADDR_OFFSET = Layer_Handle[Layer].Stride * Window_conf.Height * 1;
	MemAddr += XVMIX_CHROMA_ADDR_OFFSET;

	Status = XVMix_SetLayerChromaBufferAddr(MixerPtr, Layer, MemAddr);
	/* Temp fix: write upper 32-bit addr for high mem (chroma) */
	Xil_Out32(MixerPtr->Mix.Config.BaseAddress +
		  XV_MIX_CTRL_ADDR_HWREG_LAYER1_BUF2_V_DATA +
		  ((Layer - 1) * 0x100) + 4,
		  ATM_HIGH_MEM_PREFIX);
	if (Status != XST_SUCCESS) {
		xil_printf("[VMIX-BRIDGE] ERROR: Layer %d chroma addr 0x%08X failed\r\n",
			   Layer, (u32)MemAddr);
	}

	return Status;
}
#endif /* XPAR_XV_MIX_NUM_INSTANCES */

/************************** HDMI TX Trigger Callbacks ************************/

#ifdef XPAR_XV_HDMITXSS1_NUM_INSTANCES

/**
 * I2cClk - Program external IDT clock generator via I2C
 */
static int I2cClk(u32 InFreq, u32 OutFreq)
{
	int Status = XST_FAILURE;

#if defined (XPS_BOARD_VEK280) || defined (XPS_BOARD_VEK385)
	/* VEK280/VEK385: Direct I2C path, no mux select needed, use addr 0x6C */
	if (OutFreq != 0) {
		Status = IDT_8T49N24x_I2cClk(&Iic, I2C_CLK_ADDR1,
					      InFreq, OutFreq);
	}
#else
	Vfmc_I2cMuxSelect(&Vfmc[0]);

	if (OutFreq != 0) {
		Status = IDT_8T49N24x_I2cClk(&Iic, I2C_CLK_ADDR,
					      InFreq, OutFreq);
	}
#endif
	return Status;
}

/**
 * CheckDwnstrmSinkCaps - Read/parse EDID from downstream sink
 */
static u32 CheckDwnstrmSinkCaps(void)
{
	u32 Status = XST_SUCCESS;
	XV_HdmiTxSs1 *HdmiTxSs1Ptr = (XV_HdmiTxSs1 *)&HdmiTxSs;

	/* Initialize EDID App during cable connect */
	EDIDConnectInit(&EdidHdmi_t);

	/* Read the EDID and the SCDC */
	EdidScdcCheck(HdmiTxSs1Ptr, &EdidHdmi_t);

	/* Check whether the sink is DVI/HDMI Supported */
	if (EdidHdmi_t.EdidCtrlParam.IsHdmi == XVIDC_ISDVI) {
		XVidC_VideoStream *HdmiTxSsVidStreamPtr;
		HdmiTxSsVidStreamPtr = XV_HdmiTxSs1_GetVideoStream(HdmiTxSs1Ptr);
		if (HdmiTxSsVidStreamPtr->ColorDepth != XVIDC_BPC_8 ||
		    HdmiTxSsVidStreamPtr->ColorFormatId != XVIDC_CSF_RGB) {
			xil_printf("[VMIX-BRIDGE] Sink is DVI, format mismatch\r\n");
			return Status;
		} else {
			xil_printf("[VMIX-BRIDGE] Set TX stream to DVI\r\n");
			XV_Tx_SetDviMode(&xhdmi_tx_controller);
		}
	} else {
		if (EdidHdmi_t.EdidCtrlParam.MaxFrlRateSupp !=
		    XVIDC_MAXFRLRATE_NOT_SUPPORTED) {
			xil_printf("[VMIX-BRIDGE] Set TX stream to HDMI FRL\r\n");
			XV_Tx_SetHdmiFrlMode(&xhdmi_tx_controller);
		} else {
			xil_printf("[VMIX-BRIDGE] Set TX stream to HDMI TMDS\r\n");
			XV_Tx_SetHdmiTmdsMode(&xhdmi_tx_controller);
		}
	}

	return Status;
}

/**
 * UpdateAviInfoFrame - Update AVI infoframe for colorbar/TX-only mode
 */
static void UpdateAviInfoFrame(XVidC_VideoStream *HdmiTxSsVidStreamPtr)
{
	XHdmiC_AVI_InfoFrame *AviInfoFramePtr;
	XVidC_ColorFormat Colorformat;

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

/**
 * SendInfoFrame_Colorbar - Send AVI + Audio infoframes for TX-only mode
 */
static void SendInfoFrame_Colorbar(void)
{
	u32 Status;
	XHdmiC_AVI_InfoFrame *AVIInfoFramePtr;
	XHdmiC_AudioInfoFrame *AudioInfoFramePtr;
	XV_HdmiTxSs1 *HdmiTxSs1Ptr = (XV_HdmiTxSs1 *)&HdmiTxSs;
	XHdmiC_Aux AuxFifo;

	AVIInfoFramePtr = XV_HdmiTxSs1_GetAviInfoframe(HdmiTxSs1Ptr);
	AudioInfoFramePtr = XV_HdmiTxSs1_GetAudioInfoframe(HdmiTxSs1Ptr);

	/* Generate and send AVI InfoFrame */
	AuxFifo = XV_HdmiC_AVIIF_GeneratePacket(AVIInfoFramePtr);
	Status = XV_HdmiTxSs1_SendGenericAuxInfoframe(HdmiTxSs1Ptr, &AuxFifo);
	if (Status != XST_SUCCESS) {
		xil_printf("[VMIX-BRIDGE] AVI infoframe send failed\r\n");
	}

	/* Generate and send Audio InfoFrame */
	AuxFifo = XV_HdmiC_AudioIF_GeneratePacket(AudioInfoFramePtr);
	Status = XV_HdmiTxSs1_SendGenericAuxInfoframe(HdmiTxSs1Ptr, &AuxFifo);
	if (Status != XST_SUCCESS) {
		xil_printf("[VMIX-BRIDGE] Audio infoframe send failed\r\n");
	}
}

/**
 * HdmiTx_CableConnectionChange - Called when cable is connected/disconnected
 *
 * On connect: reads sink EDID, determines HDMI/DVI/FRL mode.
 * On disconnect: clears TX present state.
 */
static void HdmiTx_CableConnectionChange(void *InstancePtr)
{
	XV_Tx *txInst = (XV_Tx *)InstancePtr;
	XV_HdmiTxSs1 *HdmiTxSs1Ptr = (XV_HdmiTxSs1 *)txInst->HdmiTxSs;

	if (HdmiTxSs1Ptr->IsStreamConnected == FALSE) {
		xil_printf("[VMIX-BRIDGE] TX cable disconnected\r\n");
	} else {
		xil_printf("[VMIX-BRIDGE] TX cable connected, reading EDID...\r\n");

		if (CheckDwnstrmSinkCaps() == XST_SUCCESS) {
			XV_Tx_SetEdidParsingDone(&xhdmi_tx_controller, TRUE);
			xil_printf("[VMIX-BRIDGE] EDID Parsing Pass\r\n");

			if ((EdidHdmi_t.EdidCtrlParam.MaxFrlRateSupp > 0) &&
			    (EdidHdmi_t.EdidCtrlParam.IsSCDCPresent ==
			     XVIDC_SUPPORTED)) {
				XV_Tx_SetFrlEdidInfo(
					&xhdmi_tx_controller,
					EdidHdmi_t.EdidCtrlParam.IsSCDCPresent,
					EdidHdmi_t.EdidCtrlParam.MaxFrlRateSupp);
			}
		} else {
			XV_Tx_SetEdidParsingDone(&xhdmi_tx_controller, FALSE);
			xil_printf("[VMIX-BRIDGE] EDID Parsing Fails\r\n");
		}
	}
}

/**
 * HdmiTx_SetupTxTmdsRefClk - Program TMDS reference clock via IDT
 */
static void HdmiTx_SetupTxTmdsRefClk(void *InstancePtr)
{
	int Status;
	XV_Tx *XV_TxInst = (XV_Tx *)InstancePtr;

#if !(defined (XPS_BOARD_VEK280) || defined (XPS_BOARD_VEK385))
	/* Non-VEK boards: select IDT as TX reference clock source via VFMC */
	Vfmc_Mezz_HdmiTxRefClock_Sel(&Vfmc[0], VFMC_MEZZ_TxRefclk_From_IDT);
#endif
	XV_Tx_SetFrlIntVidCkeGen(XV_TxInst);

	/* TX-only (colorbar) mode: program clock in free running mode */
	xil_printf("[VMIX-BRIDGE] Setting TX TMDS Ref Clock = %d Hz\r\n",
		   XV_TxInst->VidPhy->HdmiTxRefClkHz);
	Status = I2cClk(0, XV_TxInst->VidPhy->HdmiTxRefClkHz);

	if (Status == XST_FAILURE) {
		xil_printf("[VMIX-BRIDGE] I2cClk Program Failure!\r\n");
	} else {
		usleep(1000000);
	}
}

/**
 * HdmiTx_SetupTxFrlRefClk - Configure FRL reference clock via Si5344
 */
static void HdmiTx_SetupTxFrlRefClk(void *InstancePtr)
{
	xil_printf("[VMIX-BRIDGE] SetupTxFrlRefClk\r\n");
#if !(defined (XPS_BOARD_VEK280) || defined (XPS_BOARD_VEK385))
	/* Non-VEK boards: select Si5344 as FRL reference clock source */
	int Status;
	Status = Vfmc_Mezz_HdmiTxRefClock_Sel(&Vfmc[0],
			VFMC_MEZZ_TxRefclk_From_Si5344);
	XHdmiphy1_ClkDetFreqReset(&Hdmiphy1, 0, XHDMIPHY1_DIR_TX);

	if (Status == XST_FAILURE) {
		xil_printf("[VMIX-BRIDGE] FRL RefClk Program Failure!\r\n");
	}
#endif
}

/**
 * HdmiTx_GetFrlClk - Called during FRL training to get FRL clock
 * In TX-only mode this is typically a no-op.
 */
static void HdmiTx_GetFrlClk(void *InstancePtr)
{
	/* TX-only (no RX): nothing to compute for FRL clock */
	xil_printf("[VMIX-BRIDGE] GetFrlClk (TX-only: no-op)\r\n");
}

/**
 * HdmiTx_SetupAudioVideo - Configure AVI infoframe and video for TX
 */
static void HdmiTx_SetupAudioVideo(void *InstancePtr)
{
	XVidC_VideoStream *HdmiTxSsVidStreamPtr;
	XHdmiC_AVI_InfoFrame *AviInfoFramePtr;
	XHdmiC_VSIF *VSIFPtr;

	HdmiTxSsVidStreamPtr = XV_HdmiTxSs1_GetVideoStream(&HdmiTxSs);

	/* Reset AVI and VSIF */
	AviInfoFramePtr = XV_HdmiTxSs1_GetAviInfoframe(&HdmiTxSs);
	VSIFPtr = XV_HdmiTxSs1_GetVSIF(&HdmiTxSs);
	(void)memset((void *)AviInfoFramePtr, 0, sizeof(XHdmiC_AVI_InfoFrame));
	(void)memset((void *)VSIFPtr, 0, sizeof(XHdmiC_VSIF));

	TxBusy = TRUE;

	/* TX-only mode: update AVI infoframe from stream */
	UpdateAviInfoFrame(HdmiTxSsVidStreamPtr);

	xil_printf("[VMIX-BRIDGE] SetupAudioVideo done\r\n");
}

/**
 * HdmiTx_StreamOn - Called when TX stream is established
 */
static void HdmiTx_StreamOn(void *InstancePtr)
{
	XV_HdmiTxSs1 *HdmiTxSs1Ptr = &HdmiTxSs;
	XVidC_VideoStream *HdmiTxSsVidStreamPtr;

	HdmiTxSsVidStreamPtr = XV_HdmiTxSs1_GetVideoStream(HdmiTxSs1Ptr);

	xil_printf("[VMIX-BRIDGE] HDMI TX Stream ON!\r\n");
	if (HdmiTxSsVidStreamPtr->VmId == XVIDC_VM_CUSTOM) {
		xil_printf("\tMode:        CUSTOM\r\n");
	} else {
		xil_printf("\tMode:        %s\r\n",
			   XVidC_GetVideoModeStr(HdmiTxSsVidStreamPtr->VmId));
	}
	xil_printf("\tColor:       %s\r\n",
		   XVidC_GetColorFormatStr(HdmiTxSsVidStreamPtr->ColorFormatId));
	xil_printf("\tBPC:         %d\r\n",
		   HdmiTxSsVidStreamPtr->ColorDepth);
	xil_printf("\tPPC:         %d\r\n",
		   HdmiTxSsVidStreamPtr->PixPerClk);

	if (XV_HdmiTxSs1_GetTransportMode(HdmiTxSs1Ptr) == TRUE) {
		xil_printf("\tTX Mode:     FRL\r\n");
	} else {
		xil_printf("\tTX Mode:     TMDS\r\n");
	}

	TxBusy = FALSE;
}

/**
 * HdmiTx_StreamOff - Called when TX stream is shut down
 */
static void HdmiTx_StreamOff(void *InstancePtr)
{
	xil_printf("[VMIX-BRIDGE] HDMI TX Stream OFF\r\n");
}

/**
 * HdmiTx_VidSyncRecv - Called on each TX VSync to send infoframes
 */
static void HdmiTx_VidSyncRecv(void *InstancePtr)
{
	XV_Tx *XV_TxInst = (XV_Tx *)InstancePtr;

	/* Send infoframes only if sink supports HDMI */
	if (EdidHdmi_t.EdidCtrlParam.IsHdmi == XVIDC_ISHDMI &&
	    (XV_TxInst->HdmiTxSs->HdmiTx1Ptr->Stream.IsHdmi == TRUE)) {
		SendInfoFrame_Colorbar();
	}
}

/**
 * HdmiTx_EnableCableDriver - Configure VFMC TX cable drivers
 */
static void HdmiTx_EnableCableDriver(void *InstancePtr)
{
	XV_Tx *XV_TxInst = (XV_Tx *)InstancePtr;
	u64 TxLineRate;
	u8 Lanes;

	TxLineRate = XV_Tx_GetLineRate(XV_TxInst);
	Lanes = XV_TxInst->HdmiTxSs->HdmiTx1Ptr->Stream.Frl.Lanes;

	if (SinkReadyCheck(XV_TxInst->HdmiTxSs, &EdidHdmi_t)) {
		xil_printf("[VMIX-BRIDGE] Setting Cable Driver, LineRate=%d%d\r\n",
			   (u32)(TxLineRate >> 32), (u32)TxLineRate);

		if (XV_HdmiTxSs1_GetTransportMode(XV_TxInst->HdmiTxSs) == FALSE) {
			/* TMDS mode */
			Vfmc_Gpio_Mezz_HdmiTxDriver_Reconfig(&Vfmc[0],
							     FALSE,
							     TxLineRate, Lanes);
		} else {
			/* FRL mode */
			Vfmc_Gpio_Mezz_HdmiTxDriver_Reconfig(&Vfmc[0],
							     TRUE,
							     TxLineRate, Lanes);
		}

		/* Configure GT TX voltage swing for VEK385/Versal */
		{
			u8 TxDiffSwingVal = 0x17;
			u8 TxprecursorVal = 0x1;
			u8 TxpostcursorVal = 0x1;
			if (XV_HdmiTxSs1_GetTransportMode(XV_TxInst->HdmiTxSs) == FALSE) {
				/* TMDS mode */
				u32 LineRateMbps = (u32)((u64)TxLineRate / 1000000);
				if (LineRateMbps >= 3400) {
					TxDiffSwingVal = 0x1F;
				} else {
					TxDiffSwingVal = 0x17;
				}
				for (int ChId = 1; ChId <= 4; ChId++) {
					XHdmiphy1_SetTxVoltageSwing(&Hdmiphy1,
								    0, ChId,
								    TxDiffSwingVal);
				}
			} else {
				/* FRL mode */
				TxDiffSwingVal = 0xD;
				TxprecursorVal = 0x1;
				TxpostcursorVal = 0x2;
				for (int ChId = 1; ChId <= 4; ChId++) {
					XHdmiphy1_SetTxVoltageSwing(&Hdmiphy1,
								    0, ChId,
								    TxDiffSwingVal);
					XHdmiphy1_SetTxPreEmphasis(&Hdmiphy1,
								   0, ChId,
								   TxprecursorVal);
					XHdmiphy1_SetTxPostCursor(&Hdmiphy1,
								  0, ChId,
								  TxpostcursorVal);
				}
			}
		}
	}
}

/**
 * HdmiTx_ReadyToStartTransmit - State machine signals ready to start TX
 */
static void HdmiTx_ReadyToStartTransmit(void *InstancePtr)
{
	xil_printf("[VMIX-BRIDGE] TX Ready to Start Transmit!\r\n");
	TxStartTransmit = TRUE;
}

/**
 * HdmiTx_FrlFfeConfigDevice - FRL FFE training device config (placeholder)
 */
static void HdmiTx_FrlFfeConfigDevice(void *InstancePtr)
{
	/* Placeholder for FRL FFE configuration on mezzanine card */
}

/**
 * HdmiTx_FrlConfigDeviceSetup - FRL device setup with driver reconfig
 */
static void HdmiTx_FrlConfigDeviceSetup(void *InstancePtr)
{
	XV_Tx *XV_TxInst = (XV_Tx *)InstancePtr;
	u64 LineRate;
	u8 Lanes;
	u8 Data = 0xD;

	xil_printf("[VMIX-BRIDGE] FRL Config Device Setup\r\n");

	LineRate = ((u64)(XV_TxInst->HdmiTxSs->HdmiTx1Ptr->Stream.Frl.LineRate)) *
		   ((u64)(1e9));
	Lanes = XV_TxInst->HdmiTxSs->HdmiTx1Ptr->Stream.Frl.Lanes;

	Vfmc_Gpio_Mezz_HdmiTxDriver_Reconfig(&Vfmc[0], TRUE, LineRate, Lanes);

	/* Adjust GT TX Diff Swing for FRL */
	for (int ChId = 1; ChId <= 4; ChId++) {
		XHdmiphy1_SetTxVoltageSwing(&Hdmiphy1, 0, ChId, Data);
	}
}

/**
 * Hdmiphy1ErrorCallback - VPHY error callback
 */
static void Hdmiphy1ErrorCallback(void *CallbackRef)
{
	xil_printf("[VMIX-BRIDGE] HDMI PHY Error!\r\n");
}

/*****************************************************************************/
/**
 * VmixHdmiBridge_HdmiPoll - Poll HDMI TX state machine in main loop
 *
 * Must be called periodically from the main while loop to process
 * the TX state machine events and start the transmitter when ready.
 *****************************************************************************/
void VmixHdmiBridge_HdmiPoll(void)
{
	u32 Status;

	if (TxStartTransmit == TRUE) {
		TxStartTransmit = FALSE;
		TxBusy = TRUE;

		xil_printf("[VMIX-BRIDGE] Starting TX video for mode: %s\r\n",
			   XVidC_GetVideoModeStr(HdmiTxVideoMode));

		/* Update stream parameters before starting */
		{
			XVidC_VideoStream *HdmiTxSsVidStreamPtr;
			HdmiTxSsVidStreamPtr = XV_HdmiTxSs1_GetVideoStream(&HdmiTxSs);

			HdmiTxSsVidStreamPtr->VmId = HdmiTxVideoMode;
			HdmiTxSsVidStreamPtr->ColorFormatId = XVIDC_CSF_RGB;
			HdmiTxSsVidStreamPtr->ColorDepth = XVIDC_BPC_8;
			HdmiTxSsVidStreamPtr->PixPerClk = XVIDC_PPC_4;
			HdmiTxSsVidStreamPtr->FrameRate =
				XVidC_GetFrameRate(HdmiTxVideoMode);
			HdmiTxSsVidStreamPtr->Timing =
				*XVidC_GetTimingInfo(HdmiTxVideoMode);
		}

		Status = XV_Tx_VideoSetupAndStart(&xhdmi_tx_controller,
			(XVidC_VideoStream *)XV_HdmiTxSs1_GetVideoStream(&HdmiTxSs));
		if (Status != XST_SUCCESS) {
			xil_printf("[VMIX-BRIDGE] TX VideoSetupAndStart failed (%d)\r\n",
				   Status);
			TxBusy = FALSE;
		}
	}
}

#endif /* XPAR_XV_HDMITXSS1_NUM_INSTANCES */

#endif /* XPAR_XV_MIX_NUM_INSTANCES */
