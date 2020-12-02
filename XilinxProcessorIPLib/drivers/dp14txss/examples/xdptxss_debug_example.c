/******************************************************************************
* Copyright (C) 2015 - 2020 Xilinx, Inc. All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xdptxss_debug_example.c
*
* This file contains a design example using the XDpTxSs driver in single stream
* (SST) transport or multi-stream transport (MST) mode and provides DisplayPort
* Subsystem debug information at runtime.
*
* @note		For this example to display output, the user need to implement
*		initialization of the system (DpTxSs_PlatformInit) and after
*		DisplayPort TX subsystem start (XDpTxSs_Start) is complete,
*		implement configuration of the video stream source in order to
*		provide the DisplayPort TX Subsystem HIP input.
*		The functions DpTxSs_PlatformInit and DpTxSs_StreamSrc are
*		declared and are left up to the user implement.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver  Who Date     Changes
* ---- --- -------- --------------------------------------------------
* 1.00 sha 07/01/15 Initial release.
* 2.00 sha 09/28/15 Added HDCP debug info function call.
* 4.1  ms  01/23/17 Modified xil_printf statement in main function to
*                   ensure that "Successfully ran" and "Failed" strings
*                   are available in all examples. This is a fix for
*                   CR-965028.
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xdptxss.h"
#include "xil_printf.h"
#include "xil_types.h"
#include "xparameters.h"
#include "xstatus.h"
#include "xvidc_edid.h"
#include "xvidc.h"
#include "string.h"

/************************** Constant Definitions *****************************/

/* The unique device ID of the DisplayPort Transmitter Subsystem HIP instance
 * to be used
 */
#define XDPTXSS_DEVICE_ID		XPAR_DPTXSS_0_DEVICE_ID

/* If set to 1, example will run in MST mode. Otherwise, in SST mode.
 * In MST mode, this example reads the EDID of RX devices if connected in
 * daisy-chain.
 */
#define DPTXSS_MST			1
#define DPTXSS_LINK_RATE		XDPTXSS_LINK_BW_SET_540GBPS
#define DPTXSS_LANE_COUNT		XDPTXSS_LANE_COUNT_SET_4

/* The video resolution from the display mode timings (DMT) table to use for
 * DisplayPort TX Subsystem. It can be set to use preferred video mode for
 * EDID of RX device.
 */
#define DPTXSS_VID_MODE			XVIDC_VM_USE_EDID_PREFERRED

/* The color depth (bits per color component) to use DisplayPort TX
 * Subsystem.
 */
#define DPTXSS_BPC			8

/***************** Macros (Inline Functions) Definitions *********************/

/* Convert float to unsigned */
#define FLOAT_FRAC_TO_U32(V, D) ((u32)(V * D) - (((u32)V) * D))

/**************************** Type Definitions *******************************/


/************************** Function Prototypes ******************************/

u32 DpTxSs_DebugExample(u16 DeviceId);
u32 DpTxSs_PlatformInit(void);
u32 DpTxSs_StreamSrc(u8 VerticalSplit);

/* Debug helper functions */
void DpTxSs_ReportEdidInfo(XDpTxSs *InstancePtr);

/* EDID helper functions */
u32 DpTxSs_EdidDecodeBase(u8 *EdidRaw);
void DpTxSs_EdidBaseVPId(u8 *EdidRaw);
void DpTxSs_EdidBaseVerRev(u8 *EdidRaw);
void DpTxSs_EdidBaseBasicDisp(u8 *EdidRaw);
void DpTxSs_EdidColorChar(u8 *EdidRaw);
void DpTxSs_EdidEstTimings(u8 *EdidRaw);
void DpTxSs_EdidStdTimings(u8 *EdidRaw);
void DpTxSs_EdidPtm(u8 *EdidRaw);
u8 DpTxSs_EdidCalculateChecksum(u8 *Data, u8 Size);
void DpTxSs_PrintEdid(u8 *EdidRaw);

/************************** Variable Definitions *****************************/

XDpTxSs DpTxSsInst;	/* The DPTX Subsystem instance.*/

/************************** Function Definitions *****************************/


/*****************************************************************************/
/**
*
* This is the main function for XDpTxSs SST/MST example.
*
* @param	None.
*
* @return
*		- XST_SUCCESS if the MST/SST example passed.
*		- XST_FAILURE if the MST/SST example was unsuccessful.
*
* @note		None.
*
******************************************************************************/
int main()
{
	u32 Status;

	xil_printf("-------------------------------------------\n\r");
	xil_printf("DisplayPort TX Subsystem debug example\n\r");
	xil_printf("(c) 2015 by Xilinx\n\r");
	xil_printf("-------------------------------------------\n\r\n\r");

	Status = DpTxSs_DebugExample(XDPTXSS_DEVICE_ID);
	if (Status != XST_SUCCESS) {
		xil_printf("DisplayPort TX Subsystem debug example failed."
				"\n\r");
		return XST_FAILURE;
	}

	xil_printf("Successfully ran DisplayPort TX Subsystem debug example\n\r");

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* This function is the main entry point for the debug example using the
* XDpTxSs driver. This function will print values set during DisplayPort TX
* Subsystem set up to work in MST/SST based upon RX device capabilities.
*
* @param	DeviceId is the unique device ID of the DisplayPort TX
*		Subsystem core.
*
* @return
*		- XST_SUCCESS if DisplayPort TX Subsystem prints debug
*		information successfully.
*		- XST_FAILURE, otherwise.
*
* @note		None.
*
******************************************************************************/
u32 DpTxSs_DebugExample(u16 DeviceId)
{
	u32 Status;
	u8 VSplitMode = 0;
	XDpTxSs_Config *ConfigPtr;

	/* Do platform initialization in this function. This is hardware
	 * system specific. It is up to the user to implement this function.
	 */
	xil_printf("PlatformInit\n\r");
	Status = DpTxSs_PlatformInit();
	if (Status != XST_SUCCESS) {
		xil_printf("Platform init failed!\n\r");
	}
	xil_printf("Platform initialization done.\n\r");

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
		xil_printf("DPTXSS config initialization failed.\n\r");
		return XST_FAILURE;
	}

	/* Check for SST/MST support */
	if (DpTxSsInst.UsrOpt.MstSupport) {
		xil_printf("\n\rINFO:DPTXSS is MST enabled. DPTXSS can be "
			"switched to SST/MST\n\r\n\r");
	}
	else {
		xil_printf("\n\rINFO:DPTXSS is  SST enabled. DPTXSS works "
			"only in SST mode.\n\r\n\r");
	}

	/* Disable interrupts. */
	Xil_ExceptionDisable();

	/* Read capabilities of RX device */
	Status = XDpTxSs_GetRxCapabilities(&DpTxSsInst);
	if (Status != XST_SUCCESS) {
		xil_printf("ERR: RX device is not connected.\r\n");

		/* Enable interrupts. */
		Xil_ExceptionEnable();

		return XST_FAILURE;
	}

	/* Set Link rate and lane count to maximum */
	XDpTxSs_SetLinkRate(&DpTxSsInst, DPTXSS_LINK_RATE);
	XDpTxSs_SetLaneCount(&DpTxSsInst, DPTXSS_LANE_COUNT);

	/* Set video mode */
	XDpTxSs_SetVidMode(&DpTxSsInst, DPTXSS_VID_MODE);

	/* Set BPC */
	XDpTxSs_SetBpc(&DpTxSsInst, DPTXSS_BPC);

	/* Check whether DPTXSS and RX device is in MST */
	Status = XDpTxSs_IsMstCapable(&DpTxSsInst);
	if (DpTxSsInst.UsrOpt.MstSupport && DPTXSS_MST &&
		(Status == XST_SUCCESS)) {
		/* Set DPTXSS in MST mode */
		XDpTxSs_SetTransportMode(&DpTxSsInst, 1);
	}
	else {
		/* Set DPTXSS in SST mode */
		XDpTxSs_SetTransportMode(&DpTxSsInst, 0);
	}

	/* Start DPTXSS parameters set */
	Status = XDpTxSs_Start(&DpTxSsInst);
	if (Status != XST_SUCCESS) {
		xil_printf("ERR:DPTX SS start failed\n\r");

		/* Enable interrupts. */
		Xil_ExceptionEnable();

		return XST_FAILURE;
	}

	if ((DpTxSsInst.UsrOpt.VmId == (XVIDC_VM_UHD2_60_P)) &&
				(DpTxSsInst.UsrOpt.MstSupport)) {
		VSplitMode = 1;
	}
	else {
		VSplitMode = 0;
	}

	/* Do stream setup in this function. It is up to the user to implement
	* this function.
	 */
	DpTxSs_StreamSrc(VSplitMode);

	/* Print available sub-cores of subsystem */
	XDpTxSs_ReportCoreInfo(&DpTxSsInst);

	/* Print link info that was configured during link training */
	XDpTxSs_ReportLinkInfo(&DpTxSsInst);

	/* Print Multi-stream attributes being set during subsystem set up */
	XDpTxSs_ReportMsaInfo(&DpTxSsInst);

	/* Print RX device capability info */
	XDpTxSs_ReportSinkCapInfo(&DpTxSsInst);

	/* Print current Dual Splitter values set */
	XDpTxSs_ReportSplitterInfo(&DpTxSsInst);

	/* Print current VTC(s) values set */
	XDpTxSs_ReportVtcInfo(&DpTxSsInst);

	/* Print EDID info of RX device(s) */
	DpTxSs_ReportEdidInfo(&DpTxSsInst);

	/* Print HDCP debug info */
	XDpTxSs_ReportHdcpInfo(&DpTxSsInst);

	/* Enable interrupts. */
	Xil_ExceptionEnable();

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
	/* User is responsible to setup platform specific initialization */

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* This function setup stream source to input DisplayPort TX Subsystem.
*
* @param	VerticalSplit specifies whether to split video frame
*		vertically into two different vertical halves.
*		- 1 = Vertically split input frame
*		- 0 = No vertically split input frame.
*
* @return
*		- XST_SUCCESS if stream source is configured successfully.
*		- XST_FAILURE, otherwise.
*
* @note		None.
*
******************************************************************************/
u32 DpTxSs_StreamSrc(u8 VerticalSplit)
{
	/* User is responsible to setup stream source to input DPTXSS */

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* This function prints EDID info of RX device.
*
* @param	InstancePtr is a pointer to the XDpTxSs core instance.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void DpTxSs_ReportEdidInfo(XDpTxSs *InstancePtr)
{
	u8 Edid[128];
	u32 IntrMask;
	u32 Index;
	u32 Index1;

	/* Verify argument. */
	Xil_AssertVoid(InstancePtr != NULL);

	/* Read interrupt */
	IntrMask = XDpTxSs_ReadReg(InstancePtr->DpPtr->Config.BaseAddr,
			(XDPTXSS_INTERRUPT_MASK));

	/* Disable HPD interrupts. */
	XDpTxSs_WriteReg(InstancePtr->DpPtr->Config.BaseAddr,
			(XDPTXSS_INTERRUPT_MASK), IntrMask |
			(XDPTXSS_INTERRUPT_MASK_HPD_PULSE_DETECTED_MASK) |
			(XDPTXSS_INTERRUPT_MASK_HPD_EVENT_MASK));

	/* SST mode sink is accessed directly. */
	if (InstancePtr->UsrOpt.MstSupport == 0) {
		/* Get EDID */
		XDpTxSs_GetEdid(InstancePtr, Edid);

		/* Print the content of EDID block */
		DpTxSs_PrintEdid(Edid);

		/* Parse EDID and print */
		DpTxSs_EdidDecodeBase(Edid);
	}
	/* EDID in MST mode */
	else {
		u8 TotalSink;

		XDp_TxTopologyNode *SelSink;

		TotalSink = InstancePtr->DpPtr->TxInstance.Topology.SinkTotal;

		xil_printf("Total # of sinks found: %d\n\r", TotalSink);

		for (Index = 0; Index < TotalSink; Index++) {
			SelSink = InstancePtr->DpPtr->TxInstance.Topology.
							SinkList[Index];
			xil_printf("> Sink #%ld: LCT = %d ; RAD = ", Index,
						SelSink->LinkCountTotal);
			for (Index1 = 0; Index1 < SelSink->LinkCountTotal;
								Index1++) {
				xil_printf("%d ",
					SelSink->RelativeAddress[Index1]);
			}
			xil_printf("\n\r");

			XDpTxSs_GetRemoteEdid(InstancePtr, Index, Edid);

			DpTxSs_PrintEdid(Edid);
			DpTxSs_EdidDecodeBase(Edid);
		}
	}

	/* Re-enable interrupts */
	XDpTxSs_WriteReg(InstancePtr->DpPtr->Config.BaseAddr,
		(XDPTXSS_INTERRUPT_MASK), IntrMask);
}

/*****************************************************************************/
/**
*
* This function decodes and prints EDID header and tag information.
*
* @param	EdidRaw is a pointer to u8 variable that will be used for
*		parsing EDID blocks.
*
* @return
*		- XST_SUCCESS, if EDID blocks parsed successfully.
*
* @note		None.
*
******************************************************************************/
u32 DpTxSs_EdidDecodeBase(u8 *EdidRaw)
{
	/* Check valid header. */
	if (XVidC_EdidIsHeaderValid(EdidRaw)) {
		xil_printf("\n\rEDID header is: 00 FF FF FF FF FF FF 00\n\r");
	}
	else {
		xil_printf("\n\rNot an EDID base block...\n\r");
		return XST_FAILURE;
	}

	/* Obtain vendor and product identification information. */
	DpTxSs_EdidBaseVPId(EdidRaw);
	DpTxSs_EdidBaseVerRev(EdidRaw);
	DpTxSs_EdidBaseBasicDisp(EdidRaw);
	DpTxSs_EdidColorChar(EdidRaw);
	DpTxSs_EdidEstTimings(EdidRaw);
	DpTxSs_EdidStdTimings(EdidRaw);

	xil_printf("Descriptors:\n\r");
	xil_printf("First tag: 0x%02x 0x%02x\n\r", EdidRaw[0x36],
						EdidRaw[0x38]);
	xil_printf("Second tag: 0x%02x 0x%02x\n\r", EdidRaw[0x48],
						EdidRaw[0x4A]);
	xil_printf("Third tag: 0x%02x 0x%02x\n\r", EdidRaw[0x5A],
						EdidRaw[0x5C]);
	xil_printf("Fourth tag: 0x%02x 0x%02x\n\r", EdidRaw[0x6C],
						EdidRaw[0x6E]);

	DpTxSs_EdidPtm(EdidRaw);

	xil_printf("Number of extensions:%d\n\r",
			XVidC_EdidGetExtBlkCount(EdidRaw));
	xil_printf("Checksum:0x%02x -> Calculated sum = 0x%02x\n\r",
			XVidC_EdidGetChecksum(EdidRaw),
			DpTxSs_EdidCalculateChecksum(EdidRaw, 128));

	xil_printf("\n\r");

	return (XST_SUCCESS);
}

/*****************************************************************************/
/**
*
* This function prints RX device identification data.
*
* @param	EdidRaw is a pointer to u8 variable that will be used for
*		parsing EDID blocks.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void DpTxSs_EdidBaseVPId(u8 *EdidRaw)
{
	char ManName[4];

	/* Get manufacturer name as specified in the vendor and product ID */
	XVidC_EdidGetManName(EdidRaw, ManName);

	/* Vendor and product identification. */
	xil_printf("Vendor and product identification:\n\r");
	xil_printf("ID manufacturer name:%s\n\r", ManName);
	xil_printf("ID product code:0x%04x\n\r",
				XVidC_EdidGetIdProdCode(EdidRaw));
	xil_printf("ID serial number:0x%08lx\n\r",
				XVidC_EdidGetIdSn(EdidRaw));
	if (XVidC_EdidIsYearModel(EdidRaw)) {
		xil_printf("Model year:%d\n\r",
				XVidC_EdidGetModManYear(EdidRaw));
	}
	else if (XVidC_EdidGetManWeek(EdidRaw) == 0x00) {
		xil_printf("Manufactured:Year = %d ; Week N/A\n\r",
				XVidC_EdidGetModManYear(EdidRaw));
	}
	else {
		xil_printf("Manufactured:Year = %d ; Week = %d\n\r",
				XVidC_EdidGetModManYear(EdidRaw),
				XVidC_EdidGetManWeek(EdidRaw));
	}
}

/*****************************************************************************/
/**
*
* This function prints structure version and revision of RX EDID.
*
* @param	EdidRaw is a pointer to u8 variable that will be used for
*		parsing EDID blocks.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void DpTxSs_EdidBaseVerRev(u8 *EdidRaw)
{
	/* EDID structure version and revision. */
	xil_printf("EDID structure version and revision: %d.%d\n\r",
			XVidC_EdidGetStructVer(EdidRaw),
				XVidC_EdidGetStructRev(EdidRaw));
	xil_printf("\n\r");
}

/*****************************************************************************/
/**
*
* This function prints basic display information of RX device.
*
* @param	EdidRaw is a pointer to u8 variable that will be used for
*		parsing EDID blocks.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void DpTxSs_EdidBaseBasicDisp(u8 *EdidRaw)
{
	/* Basic display parameters and features. */
	xil_printf("Basic display parameters and features:\n\r");
	if (XVidC_EdidIsDigitalSig(EdidRaw)) {
		/* Input is a digital video signal interface. */
		xil_printf("Video signal interface is digital.\n\r");

		if (XVidC_EdidGetColorDepth(EdidRaw) !=
				(XVIDC_EDID_BDISP_VID_DIG_BPC_UNDEF)) {
			xil_printf("Color bit depth:%d\n\r",
				XVidC_EdidGetColorDepth(EdidRaw));
		}
		else {
			xil_printf("Color bit depth is undefined.\n\r");
		}

		switch (XVidC_EdidGetDigitalSigIfaceStd(EdidRaw)) {
			case (XVIDC_EDID_BDISP_VID_DIG_VIS_DVI):
				xil_printf("DVI is supported.\n\r");
				break;
			case (XVIDC_EDID_BDISP_VID_DIG_VIS_HDMIA):
				xil_printf("HDMI-a is supported.\n\r");
				break;
			case (XVIDC_EDID_BDISP_VID_DIG_VIS_HDMIB):
				xil_printf("HDMI-b is supported.\n\r");
				break;
			case (XVIDC_EDID_BDISP_VID_DIG_VIS_MDDI):
				xil_printf("MDDI is supported.\n\r");
				break;
			case (XVIDC_EDID_BDISP_VID_DIG_VIS_DP):
				xil_printf("DisplayPort is supported.\n\r");
				break;
			default:
				xil_printf("Digital interface undefined.\n\r");
				break;
		}
	}
	else {
		/* Input is an analog video signal interface. */
		xil_printf("Video signal interface is analog.\n\r");

		xil_printf("Signal level standard:");

		switch (XVidC_EdidGetAnalogSigLvlStd(EdidRaw)) {
			case (XVIDC_EDID_BDISP_VID_ANA_SLS_0700_0300_1000):
				xil_printf("0.700 : 0.300 : 1.000 Vp-p ");
				break;
			case (XVIDC_EDID_BDISP_VID_ANA_SLS_0714_0286_1000):
				xil_printf("0.714 : 0.286 : 1.000 Vp-p ");
				break;
			case (XVIDC_EDID_BDISP_VID_ANA_SLS_1000_0400_1400):
				xil_printf("1.000 : 0.400 : 1.400 Vp-p ");
				break;
			case (XVIDC_EDID_BDISP_VID_ANA_SLS_0700_0000_0700):
			default:
				xil_printf("0.700 : 0.000 : 0.700 V p-p");
				break;
		}
		xil_printf("(Video : Sync : Total)\n\r");

		xil_printf("Video setup:");
		if (XVidC_EdidGetAnalogSigVidSetup(EdidRaw)) {
			xil_printf("Blank-to-black setup or pedestal.\n\r");
		}
		else {
			xil_printf("Blank level = black level.\n\r");
		}

		xil_printf("Synchronization types:\n\r");
		xil_printf("Separate sync H & V signals ");
		if (XVidC_EdidSuppAnalogSigSepSyncHv(EdidRaw)) {
			xil_printf("are supported.\n\r");
		}
		else {
			xil_printf("are not supported.\n\r");
		}
		xil_printf("Composite sync signal on horizontal ");
		if (XVidC_EdidSuppAnalogSigCompSyncH(EdidRaw)) {
			xil_printf("is supported.\n\r");
		}
		else {
			xil_printf("is not supported.\n\r");
		}
		xil_printf("Composite sync signal on green video ");
		if (XVidC_EdidSuppAnalogSigCompSyncG(EdidRaw)) {
			xil_printf("is supported.\n\r");
		}
		else {
			xil_printf("is not supported.\n\r");
		}

		xil_printf("Serrations on the vertical sync ");
		if (XVidC_EdidSuppAnalogSigSerrVsync(EdidRaw)) {
			xil_printf("is supported.\n\r");
		}
		else {
			xil_printf("is not supported.\n\r");
		}
	}

	if (XVidC_EdidIsSsArSs(EdidRaw)) {
		xil_printf("Screen size (HxV):%dx%d(cm)\n\r",
				XVidC_EdidGetSsArH(EdidRaw),
					XVidC_EdidGetSsArV(EdidRaw));
	}
	else if (XVidC_EdidIsSsArArL(EdidRaw)) {
		xil_printf("Aspect ratio (H:V):");
		switch(XVidC_EdidGetSsArH(EdidRaw)) {
			case 0x4F:
				xil_printf("16:9 ");
				break;
			case 0x3D:
				xil_printf("16:10 ");
				break;
			case 0x22:
				xil_printf("4:3 ");
				break;
			case 0x1A:
				xil_printf("5:4 ");
				break;
			default:
				xil_printf("%ld.%03lu:1 ",
					(u32)XVidC_EdidGetSsArArL(EdidRaw),
					FLOAT_FRAC_TO_U32(
					XVidC_EdidGetSsArArL(EdidRaw),
					1000));
				break;
		}
		xil_printf("(landscape)\n\r");
	}
	else if (XVidC_EdidIsSsArArP(EdidRaw)) {
		xil_printf("Aspect ratio (H:V):");
		switch(XVidC_EdidGetSsArV(EdidRaw)) {
			case 0x4F:
				xil_printf("9:16 ");
				break;
			case 0x3D:
				xil_printf("10:16 ");
				break;
			case 0x22:
				xil_printf("3:4 ");
				break;
			case 0x1A:
				xil_printf("4:5 ");
				break;
			default:
				xil_printf("%ld.%03lu:1 ",
					(u32)XVidC_EdidGetSsArArP(EdidRaw),
					FLOAT_FRAC_TO_U32(
					XVidC_EdidGetSsArArP(EdidRaw),
					1000));
				break;
		}
		xil_printf("(portrait)\n\r");
	}
	else {
		xil_printf("Screen size and aspect ratio are undefined.\n\r");
	}

	if (XVidC_EdidIsGammaInExt(EdidRaw)) {
		xil_printf("Gamma is defined in an extension block.\n\r");
	}
	else {
		xil_printf("Gamma:%ld.%02lu\n\r",
			(u32)XVidC_EdidGetGamma(EdidRaw),
			FLOAT_FRAC_TO_U32(XVidC_EdidGetGamma(EdidRaw), 100));
	}

	xil_printf("\n\r");
	xil_printf("Display power management:\n\r");
	xil_printf("Standby mode ");
	if (XVidC_EdidSuppFeaturePmStandby(EdidRaw)) {
		xil_printf("is supported.\n\r");
	}
	else {
		xil_printf("is not supported.\n\r");
	}
	xil_printf("Suspend mode ");
	if (XVidC_EdidSuppFeaturePmSuspend(EdidRaw)) {
		xil_printf("is supported.\n\r");
	}
	else {
		xil_printf("is not supported.\n\r");
	}
	xil_printf("Active off = very low power ");
	if (XVidC_EdidSuppFeaturePmOffVlp(EdidRaw)) {
		xil_printf("is supported.\n\r");
	}
	else {
		xil_printf("is not supported.\n\r");
	}

	xil_printf("\n\r");

	if (XVidC_EdidIsDigitalSig(EdidRaw)) {
		/* Input is a digital video signal interface. */
		xil_printf("Supported color encoding format(s):\n\r");
		xil_printf("RGB 4:4:4\n\r");
		if (XVidC_EdidSuppFeatureDigColorEncYCrCb444(EdidRaw)) {
			xil_printf("YCrCb 4:4:4\n\r");
		}
		if (XVidC_EdidSuppFeatureDigColorEncYCrCb422(EdidRaw)) {
			xil_printf("YCrCb 4:2:2\n\r");
		}
	}
	else {
		/* Input is an analog video signal interface. */
		xil_printf("Display color type:");
		switch (XVidC_EdidGetFeatureAnaColorType(EdidRaw)) {
		case (XVIDC_EDID_BDISP_FEATURE_ANA_COLORTYPE_MCG):
			xil_printf("Monochrome or grayscale display.\n\r");
			break;
		case (XVIDC_EDID_BDISP_FEATURE_ANA_COLORTYPE_RGB):
			xil_printf("RGB color display.\n\r");
			break;
		case (XVIDC_EDID_BDISP_FEATURE_ANA_COLORTYPE_NRGB):
			xil_printf("Non-RGB color display.\n\r");
			break;
		case (XVIDC_EDID_BDISP_FEATURE_ANA_COLORTYPE_UNDEF):
		default:
			xil_printf("Display color type is undefined.\n\r");
			break;
		}
	}

	xil_printf("\n\r");
	xil_printf("Other supported features:\n\r");
	/* sRGB standard is the default color space. */
	xil_printf("sRGB standard ");
	if (XVidC_EdidIsFeatureSrgbDef(EdidRaw)) {
		xil_printf("is ");
	}
	else {
		xil_printf("is not ");
	}
	xil_printf("the default color space.\n\r");
	/* Preferred timing mode includes. */
	xil_printf("Ptm ");
	if (XVidC_EdidIsFeaturePtmInc(EdidRaw)) {
		xil_printf("includes ");
	}
	else {
		xil_printf("does not include ");
	}
	xil_printf("the native pixel format and preferred refresh rate.\n\r");
	/* Continuous frequency. */
	xil_printf("Display ");
	if (XVidC_EdidIsFeatureContFreq(EdidRaw)) {
		xil_printf("is ");
	}
	else {
		xil_printf("is non-");
	}
	xil_printf("continuous frequency.\n\r\n\r");
}

/*****************************************************************************/
/**
*
* This function prints color characteristics of RX device.
*
* @param	EdidRaw is a pointer to u8 variable that will be used for
*		parsing EDID blocks.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void DpTxSs_EdidColorChar(u8 *EdidRaw)
{
	xil_printf("Color characteristics:\n\r");
	xil_printf("Red_x:%ld.%09lu +- 0.0005\n\r",
		(u32)XVidC_EdidGetCcRedX(EdidRaw),
		FLOAT_FRAC_TO_U32(XVidC_EdidGetCcRedX(EdidRaw), 1000000000));
	xil_printf("Red_y:%ld.%09lu+- 0.0005\n\r",
		(u32)XVidC_EdidGetCcRedY(EdidRaw),
		FLOAT_FRAC_TO_U32(XVidC_EdidGetCcRedY(EdidRaw), 1000000000));
	xil_printf("Green_x:%ld.%09lu +- 0.0005\n\r",
		(u32)XVidC_EdidGetCcGreenX(EdidRaw),
		FLOAT_FRAC_TO_U32(XVidC_EdidGetCcGreenX(EdidRaw), 1000000000));
	xil_printf("Green_y:%ld.%09lu +- 0.0005\n\r",
		(u32)XVidC_EdidGetCcGreenY(EdidRaw),
		FLOAT_FRAC_TO_U32(XVidC_EdidGetCcGreenY(EdidRaw), 1000000000));
	xil_printf("Blue_x:%ld.%09lu +- 0.0005\n\r",
		(u32)XVidC_EdidGetCcBlueX(EdidRaw),
		FLOAT_FRAC_TO_U32(XVidC_EdidGetCcBlueX(EdidRaw), 1000000000));
	xil_printf("Blue_y:%ld.%09lu +- 0.0005\n\r",
		(u32)XVidC_EdidGetCcBlueY(EdidRaw),
		FLOAT_FRAC_TO_U32(XVidC_EdidGetCcBlueY(EdidRaw), 1000000000));
	xil_printf("White_x:%ld.%09lu +- 0.0005\n\r",
		(u32)XVidC_EdidGetCcWhiteX(EdidRaw),
		FLOAT_FRAC_TO_U32(XVidC_EdidGetCcWhiteX(EdidRaw), 1000000000));
	xil_printf("White_y:%ld.%09lu +- 0.0005\n\r",
		(u32)XVidC_EdidGetCcWhiteY(EdidRaw),
		FLOAT_FRAC_TO_U32(XVidC_EdidGetCcWhiteY(EdidRaw), 1000000000));

	xil_printf("\n\r");
}

/*****************************************************************************/
/**
*
* This function prints supported timings by the RX device.
*
* @param	EdidRaw is a pointer to u8 variable that will be used for
*		parsing EDID blocks.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void DpTxSs_EdidEstTimings(u8 *EdidRaw)
{
	xil_printf("Established timings:\n\r");
	if (XVidC_EdidSuppEstTimings720x400_70(EdidRaw)) {
		xil_printf("720x400 @ 70Hz supported.\n\r");
	}

	if (XVidC_EdidSuppEstTimings720x400_88(EdidRaw)) {
		xil_printf("720x400 @ 88Hz supported.\n\r");
	}

	if (XVidC_EdidSuppEstTimings640x480_60(EdidRaw)) {
		xil_printf("640x480 @ 60Hz supported.\n\r");
	}

	if (XVidC_EdidSuppEstTimings640x480_67(EdidRaw)) {
		xil_printf("640x480 @ 67Hz supported.\n\r");
	}

	if (XVidC_EdidSuppEstTimings640x480_72(EdidRaw)) {
		xil_printf("640x480 @ 72Hz supported.\n\r");
	}

	if (XVidC_EdidSuppEstTimings640x480_75(EdidRaw)) {
		xil_printf("640x480 @ 75Hz supported.\n\r");
	}

	if (XVidC_EdidSuppEstTimings800x600_56(EdidRaw)) {
		xil_printf("800x600 @ 56Hz supported.\n\r");
	}

	if (XVidC_EdidSuppEstTimings800x600_60(EdidRaw)) {
		xil_printf("800x600 @ 60Hz supported.\n\r");
	}

	if (XVidC_EdidSuppEstTimings800x600_72(EdidRaw)) {
		xil_printf("800x600 @ 72Hz supported.\n\r");
	}

	if (XVidC_EdidSuppEstTimings800x600_75(EdidRaw)) {
		xil_printf("800x600 @ 75Hz supported.\n\r");
	}

	if (XVidC_EdidSuppEstTimings832x624_75(EdidRaw)) {
		xil_printf("832x624 @ 75Hz supported.\n\r");
	}

	if (XVidC_EdidSuppEstTimings1024x768_87(EdidRaw)) {
		xil_printf("1024x768 @ 87Hz supported.\n\r");
	}

	if (XVidC_EdidSuppEstTimings1024x768_60(EdidRaw)) {
		xil_printf("1024x768 @ 60Hz supported.\n\r");
	}

	if (XVidC_EdidSuppEstTimings1024x768_70(EdidRaw)) {
		xil_printf("1024x768 @ 70Hz supported.\n\r");
	}

	if (XVidC_EdidSuppEstTimings1024x768_75(EdidRaw)) {
		xil_printf("1024x768 @ 75Hz supported.\n\r");
	}

	if (XVidC_EdidSuppEstTimings1280x1024_75(EdidRaw)) {
		xil_printf("1280x1024 @ 75Hz supported.\n\r");
	}

	if (XVidC_EdidSuppEstTimings1152x870_75(EdidRaw)) {
		xil_printf("1152x870 @ 75Hz supported.\n\r");
	}

	xil_printf("Manufacturer specified timings field: 0x%02d.\n\r",
			XVidC_EdidGetTimingsMan(EdidRaw));

	xil_printf("\n\r");
}

/*****************************************************************************/
/**
*
* This function prints standard timings of RX device.
*
* @param	EdidRaw is a pointer to u8 variable that will be used for
*		parsing EDID blocks.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void DpTxSs_EdidStdTimings(u8 *EdidRaw)
{
	/* Standard timings. */
	u8 Index;

	xil_printf("Standard timings:\n\r");
	for (Index = 0; Index < 8; Index++) {
		if (EdidRaw[XVIDC_EDID_STD_TIMINGS_H(Index + 1)] <= 1) {
			/* Not a valid standard timing. */
			continue;
		}

		xil_printf("%dx%d @ %dHz supported.\n\r",
			XVidC_EdidGetStdTimingsH(EdidRaw, Index + 1),
			XVidC_EdidGetStdTimingsV(EdidRaw, Index + 1),
			XVidC_EdidGetStdTimingsFrr(EdidRaw, Index + 1));
	}

	xil_printf("\n\r");
}

/*****************************************************************************/
/**
*
* This function prints preferred timing values of RX device.
*
* @param	EdidRaw is a pointer to u8 variable that will be used for
*		parsing EDID blocks.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void DpTxSs_EdidPtm(u8 *EdidRaw)
{
	u8 *Ptm;

	Ptm = &EdidRaw[XVIDC_EDID_PTM];

	u16 HBlank = ((Ptm[XVIDC_EDID_DTD_PTM_HRES_HBLANK_U4] &
		(XVIDC_EDID_DTD_PTM_XRES_XBLANK_U4_XBLANK_MASK)) << 8) |
		Ptm[XVIDC_EDID_DTD_PTM_HBLANK_LSB];

	u16 VBlank = ((Ptm[XVIDC_EDID_DTD_PTM_VRES_VBLANK_U4] &
		(XVIDC_EDID_DTD_PTM_XRES_XBLANK_U4_XBLANK_MASK)) << 8) |
		Ptm[XVIDC_EDID_DTD_PTM_VBLANK_LSB];

	u32 HActive = (((Ptm[XVIDC_EDID_DTD_PTM_HRES_HBLANK_U4] &
		(XVIDC_EDID_DTD_PTM_XRES_XBLANK_U4_XRES_MASK)) >>
		(XVIDC_EDID_DTD_PTM_XRES_XBLANK_U4_XRES_SHIFT)) << 8) |
		Ptm[XVIDC_EDID_DTD_PTM_HRES_LSB];

	u32 VActive = (((Ptm[XVIDC_EDID_DTD_PTM_VRES_VBLANK_U4] &
		(XVIDC_EDID_DTD_PTM_XRES_XBLANK_U4_XRES_MASK)) >>
		(XVIDC_EDID_DTD_PTM_XRES_XBLANK_U4_XRES_SHIFT)) << 8) |
		Ptm[XVIDC_EDID_DTD_PTM_VRES_LSB];

	u32 PixelClkKhz = ((Ptm[XVIDC_EDID_DTD_PTM_PIXEL_CLK_KHZ_MSB] <<
			8) | Ptm[XVIDC_EDID_DTD_PTM_PIXEL_CLK_KHZ_LSB]) * 10;

	u32 HFrontPorch = (((Ptm[XVIDC_EDID_DTD_PTM_XFPORCH_XSPW_U2] &
		(XVIDC_EDID_DTD_PTM_XFPORCH_XSPW_U2_HFPORCH_MASK)) >>
		(XVIDC_EDID_DTD_PTM_XFPORCH_XSPW_U2_HFPORCH_SHIFT)) << 8) |
		Ptm[XVIDC_EDID_DTD_PTM_HFPORCH_LSB];

	u32 HSyncWidth = (((Ptm[XVIDC_EDID_DTD_PTM_XFPORCH_XSPW_U2] &
		(XVIDC_EDID_DTD_PTM_XFPORCH_XSPW_U2_HSPW_MASK)) >>
		(XVIDC_EDID_DTD_PTM_XFPORCH_XSPW_U2_HSPW_SHIFT)) << 8) |
		Ptm[XVIDC_EDID_DTD_PTM_HSPW_LSB];

	u32 VFrontPorch = (((Ptm[XVIDC_EDID_DTD_PTM_XFPORCH_XSPW_U2] &
		(XVIDC_EDID_DTD_PTM_XFPORCH_XSPW_U2_VFPORCH_MASK)) >>
		(XVIDC_EDID_DTD_PTM_XFPORCH_XSPW_U2_VFPORCH_SHIFT)) << 8) |
		((Ptm[XVIDC_EDID_DTD_PTM_VFPORCH_VSPW_L4] &
		(XVIDC_EDID_DTD_PTM_VFPORCH_VSPW_L4_VFPORCH_MASK)) >>
		(XVIDC_EDID_DTD_PTM_VFPORCH_VSPW_L4_VFPORCH_SHIFT));

	u32 VSyncWidth = ((Ptm[XVIDC_EDID_DTD_PTM_XFPORCH_XSPW_U2] &
		(XVIDC_EDID_DTD_PTM_XFPORCH_XSPW_U2_VSPW_MASK)) << 8) |
		(Ptm[XVIDC_EDID_DTD_PTM_VFPORCH_VSPW_L4] &
		(XVIDC_EDID_DTD_PTM_VFPORCH_VSPW_L4_VSPW_MASK));

	u32 HBackPorch = HBlank - (HFrontPorch + HSyncWidth);

	u32 VBackPorch = VBlank - (VFrontPorch + VSyncWidth);

	u8 HPolarity = (Ptm[XVIDC_EDID_DTD_PTM_SIGNAL] &
		(XVIDC_EDID_DTD_PTM_SIGNAL_HPOLARITY_MASK)) >>
		(XVIDC_EDID_DTD_PTM_SIGNAL_HPOLARITY_SHIFT);

	u8 VPolarity = (Ptm[XVIDC_EDID_DTD_PTM_SIGNAL] &
		(XVIDC_EDID_DTD_PTM_SIGNAL_VPOLARITY_MASK)) >>
		(XVIDC_EDID_DTD_PTM_SIGNAL_VPOLARITY_SHIFT);

	xil_printf("\n\rPreferred timing mode:\n\r");
	xil_printf("Horizontal resolution:%ld px\n\r"
		"Vertical resolution:%ld lines\n\r"
		"Pixel clock:%ld KHz\n\r"
		"Horizontal front porch:%ld px\n\r"
		"Horizontal sync width:%ld px\n\r"
		"Horizontal back porch:%ld px\n\r"
		"Horizontal blanking:%d px\n\r"
		"Horizontal polarity:%d\n\r"
		"Vertical front porch:%ld px\n\r"
		"Vertical sync width:%ld px\n\r"
		"Vertical back porch:%ld px\n\r"
		"Vertical blanking:%d px\n\r"
		"Vertical polarity:%d\n\r"
		,HActive, VActive, PixelClkKhz,
		HFrontPorch, HSyncWidth, HBackPorch, HBlank, HPolarity,
		VFrontPorch, VSyncWidth, VBackPorch, VBlank, VPolarity);

	xil_printf("Interlaced:%s\n\r",
		XVidC_EdidIsDtdPtmInterlaced(EdidRaw) ?
		"Yes." : "No (progressive).");

	xil_printf("\n\r");
}

/*****************************************************************************/
/**
*
* This function calculates the EDID checksum.
*
* @param	Data is a pointer to u8 variable that will be used to retrive
*		data.
* @param	Size specifies the number of bytes.
*
* @return	Calculated checksum.
*
* @note		None.
*
******************************************************************************/
u8 DpTxSs_EdidCalculateChecksum(u8 *Data, u8 Size)
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
* This function prints 128 byte content from EDID.
*
* @param	EdidRaw is a pointer to u8 variable that will be used for
*		parsing EDID blocks.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void DpTxSs_PrintEdid(u8 *EdidRaw)
{
	u16 Index;

	xil_printf("Edid contents:");
	for (Index = 0; Index < 128; Index++) {
		if ((Index % 16) == 0) {
			xil_printf("\n\r\t");
		}

		xil_printf("%02X ", EdidRaw[Index]);
	}

	xil_printf("\n\r");
}
