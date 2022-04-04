/*******************************************************************************
* Copyright (C) 2015 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/

/******************************************************************************/
/**
 *
 * @file xdp.c
 * @addtogroup dp_v7_6
 * @{
 *
 * Contains a minimal set of functions for the XDp driver that allow access to
 * all of the DisplayPort core's functionality. See xdp.h for a detailed
 * description of the driver.
 *
 * @note	None.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- -----------------------------------------------
 * 1.0   als  01/20/15 Initial release. TX code merged from the dptx driver.
 * 2.0   als  06/08/15 Updated RX initialization with MST support.
 *                     Added callbacks for lane count changes, link rate changes
 *                     and pre-emphasis + voltage swing adjust requests.
 * 5.1   als  08/09/16 Replaced deprecated MB_Sleep with consolidated usleep.
 *            08/11/16 RX to support maximum pre-emphasis level of 1.
 *            08/12/16 Updates to support 64-bit base addresses.
 * 5.2	 aad  01/21/17 Added training timeout disable for RX MST mode for
 *		       soft-disconnect to work.
 * 6.0	 tu   05/14/17 Added AUX defer to 6
 * 6.0   jb   02/19/19 Added HDCP22 functions.
 *            02/21/19 Added returning AUX defers for HDCP22 DPCD offsets
 * 6.0	 jb   08/22/19 Removed returning AUX defers for HDCP22 DPCD offsets
 * 7.4   rg   09/01/20 Added XDp_TxColorimetryVsc API for reading sink device
 *                     capability for receiving colorimetry information through
 *                     VSC SDP packets.
 * </pre>
 *
*******************************************************************************/

/******************************* Include Files ********************************/

#include <string.h>
#include "xdp.h"
#include "sleep.h"
#include "xenv.h"

/**************************** Constant Definitions ****************************/

#if XPAR_XDPTXSS_NUM_INSTANCES
/* The maximum voltage swing level is 3. */
#define XDP_TX_MAXIMUM_VS_LEVEL 3
/* The maximum pre-emphasis level is 3. */
#define XDP_TX_MAXIMUM_PE_LEVEL 3
#endif /* XPAR_XDPTXSS_NUM_INSTANCES */

/* Error out if an AUX request yields a defer reply more than 50 times. */
#define XDP_AUX_MAX_DEFER_COUNT 50
/* Error out if an AUX request times out more than 50 times awaiting a reply. */
#define XDP_AUX_MAX_TIMEOUT_COUNT 50
/* Error out if checking for a connected device times out more than 50 times. */
#define XDP_IS_CONNECTED_MAX_TIMEOUT_COUNT 50

/****************************** Type Definitions ******************************/

#if XPAR_XDPTXSS_NUM_INSTANCES
/**
 * This typedef enumerates the list of training states used in the state machine
 * during the link training process.
 */
typedef enum {
	XDP_TX_TS_CLOCK_RECOVERY,
	XDP_TX_TS_CHANNEL_EQUALIZATION,
	XDP_TX_TS_ADJUST_LINK_RATE,
	XDP_TX_TS_ADJUST_LANE_COUNT,
	XDP_TX_TS_FAILURE,
	XDP_TX_TS_SUCCESS
} XDp_TxTrainingState;
#endif /* XPAR_XDPTXSS_NUM_INSTANCES */

/**
 * This typedef describes an AUX transaction.
 */
typedef struct {
	u16 CmdCode;		/**< The AUX command code that specifies what
					type of AUX transaction is taking
					place. */
	u8 NumBytes;		/**< The number of bytes that the AUX
					transaction will perform work on. */
	u32 Address;		/**< The AUX or I2C start address that the AUX
					transaction will perform work on. */
	u8 *Data;		/**< The data buffer that will store the data
					read from AUX read transactions or the
					data to write for AUX write
					transactions. */
} XDp_AuxTransaction;

/**************************** Function Prototypes *****************************/

/* Initialization functions. */
#if XPAR_XDPTXSS_NUM_INSTANCES
static u32 XDp_TxInitialize(XDp *InstancePtr);
#endif /* XPAR_XDPTXSS_NUM_INSTANCES */
#if XPAR_XDPRXSS_NUM_INSTANCES
static u32 XDp_RxInitialize(XDp *InstancePtr);
#endif /* XPAR_XDPRXSS_NUM_INSTANCES */

#if XPAR_XDPTXSS_NUM_INSTANCES
/* Training functions. */
static u32 XDp_TxRunTraining(XDp *InstancePtr);
static XDp_TxTrainingState XDp_TxTrainingStateClockRecovery(XDp *InstancePtr);
static XDp_TxTrainingState XDp_TxTrainingStateChannelEqualization(
							XDp *InstancePtr);
static XDp_TxTrainingState XDp_TxTrainingStateAdjustLinkRate(
							XDp *InstancePtr);
static XDp_TxTrainingState XDp_TxTrainingStateAdjustLaneCount(
							XDp *InstancePtr);
static u32 XDp_TxGetLaneStatusAdjReqs(XDp *InstancePtr);
static u32 XDp_TxCheckClockRecovery(XDp *InstancePtr, u8 LaneCount);
static u32 XDp_TxCheckChannelEqualization(XDp *InstancePtr, u8 LaneCount);
static void XDp_TxSetVswingPreemp(XDp *InstancePtr, u8 *AuxData);
static u32 XDp_TxAdjVswingPreemp(XDp *InstancePtr);
static u32 XDp_TxSetTrainingPattern(XDp *InstancePtr, u32 Pattern);
static u32 XDp_TxGetTrainingDelay(XDp *InstancePtr,
					XDp_TxTrainingState TrainingState);
/* AUX transaction functions. */
static u32 XDp_TxAuxCommon(XDp *InstancePtr, u32 CmdType, u32 Address,
							u32 NumBytes, u8 *Data);
static u32 XDp_TxAuxRequest(XDp *InstancePtr, XDp_AuxTransaction *Request);
static u32 XDp_TxAuxRequestSend(XDp *InstancePtr, XDp_AuxTransaction *Request);
static u32 XDp_TxAuxWaitReply(XDp *InstancePtr);
static u32 XDp_TxAuxWaitReady(XDp *InstancePtr);
/* Miscellaneous functions. */
static u32 XDp_TxSetClkSpeed(XDp *InstancePtr, u32 Speed);
#endif /* XPAR_XDPTXSS_NUM_INSTANCES */

static u32 XDp_WaitPhyReady(XDp *InstancePtr, u32 Mask);

/**************************** Function Definitions ****************************/

/******************************************************************************/
/**
 * This function retrieves the configuration for this DisplayPort instance and
 * fills in the InstancePtr->Config structure.
 *
 * @param	InstancePtr is a pointer to the XDp instance.
 * @param	ConfigPtr is a pointer to the configuration structure that will
 *		be used to copy the settings from.
 * @param	EffectiveAddr is the device base address in the virtual memory
 *		space. If the address translation is not used, then the physical
 *		address is passed.
 *
 * @return	None.
 *
 * @note	Unexpected errors may occur if the address mapping is changed
 *		after this function is invoked.
 *
*******************************************************************************/
void XDp_CfgInitialize(XDp *InstancePtr, XDp_Config *ConfigPtr,
							UINTPTR EffectiveAddr)
{
	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(ConfigPtr != NULL);
	Xil_AssertVoid(EffectiveAddr != 0x0);

	memset(InstancePtr, 0, sizeof(XDp));

	InstancePtr->Config = *ConfigPtr;
	InstancePtr->Config.BaseAddr = EffectiveAddr;

#if XPAR_XDPTXSS_NUM_INSTANCES
	if (XDp_GetCoreType(InstancePtr) == XDP_TX) {
		/* Set the DisplayPort TX's voltage swing and pre-emphasis
		 * levels to their defaults. */
		XDp_TxCfgTxVsOffset(InstancePtr, XDP_TX_VS_LEVEL_OFFSET);
		XDp_TxCfgTxVsLevel(InstancePtr, 0, XDP_TX_VS_LEVEL_0);
		XDp_TxCfgTxVsLevel(InstancePtr, 1, XDP_TX_VS_LEVEL_1);
		XDp_TxCfgTxVsLevel(InstancePtr, 2, XDP_TX_VS_LEVEL_2);
		XDp_TxCfgTxVsLevel(InstancePtr, 3, XDP_TX_VS_LEVEL_3);
		XDp_TxCfgTxPeLevel(InstancePtr, 0, XDP_TX_PE_LEVEL_0);
		XDp_TxCfgTxPeLevel(InstancePtr, 1, XDP_TX_PE_LEVEL_1);
		XDp_TxCfgTxPeLevel(InstancePtr, 2, XDP_TX_PE_LEVEL_2);
		XDp_TxCfgTxPeLevel(InstancePtr, 3, XDP_TX_PE_LEVEL_3);

		/* Set default to Max lane count */
		InstancePtr->TxInstance.LinkConfig.cr_done_cnt =
			InstancePtr->TxInstance.LinkConfig.MaxLaneCount;
		InstancePtr->TxInstance.LinkConfig.cr_done_oldstate =
			InstancePtr->TxInstance.LinkConfig.MaxLaneCount;
	}
#endif /* XPAR_XDPTXSS_NUM_INSTANCES */

	InstancePtr->IsReady = XIL_COMPONENT_IS_READY;
}

/******************************************************************************/
/**
 * This function prepares the DisplayPort core for use depending on whether the
 * core is operating in TX or RX mode.
 *
 * @param	InstancePtr is a pointer to the XDp instance.
 *
 * @return
 *		- XST_SUCCESS if the DisplayPort core was successfully
 *		  initialized.
 *		- XST_FAILURE otherwise.
 *
 * @note	None.
 *
*******************************************************************************/
u32 XDp_Initialize(XDp *InstancePtr)
{
	u32 Status;

	/* Verify arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

#if XPAR_XDPTXSS_NUM_INSTANCES
	if (XDp_GetCoreType(InstancePtr) == XDP_TX) {
		Status = XDp_TxInitialize(InstancePtr);
	} else
#endif /* XPAR_XDPTXSS_NUM_INSTANCES */
#if XPAR_XDPRXSS_NUM_INSTANCES
	if (XDp_GetCoreType(InstancePtr) == XDP_RX) {
		Status = XDp_RxInitialize(InstancePtr);
	}
#endif /* XPAR_XDPRXSS_NUM_INSTANCES */
	{
		/* Nothing. */
	}

	return Status;
}

#if XPAR_XDPTXSS_NUM_INSTANCES
/******************************************************************************/
/**
 * This function retrieves the RX device's capabilities from the RX device's
 * DisplayPort Configuration Data (DPCD).
 *
 * @param	InstancePtr is a pointer to the XDp instance.
 *
 * @return
 *		- XST_SUCCESS if the DisplayPort Configuration Data was read
 *		  successfully.
 *		- XST_DEVICE_NOT_FOUND if no RX device is connected.
 *		- XST_FAILURE otherwise.
 *
 * @note	None.
 *
*******************************************************************************/
u32 XDp_TxGetRxCapabilities(XDp *InstancePtr)
{
	u32 Status;
	u8 *Dpcd = InstancePtr->TxInstance.RxConfig.DpcdRxCapsField;
	u8 *Dpcd_ext = InstancePtr->TxInstance.RxConfig.DpcdRxCapsField;
	XDp_TxLinkConfig *LinkConfig = &InstancePtr->TxInstance.LinkConfig;
	XDp_Config *ConfigPtr = &InstancePtr->Config;
	u8 RxMaxLinkRate;
	u8 RxMaxLaneCount;

	/* Verify arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertNonvoid(XDp_GetCoreType(InstancePtr) == XDP_TX);
	Xil_AssertNonvoid(Dpcd != NULL);
	Xil_AssertNonvoid(LinkConfig != NULL);
	Xil_AssertNonvoid(ConfigPtr != NULL);

	if (!XDp_TxIsConnected(InstancePtr)) {
		return XST_DEVICE_NOT_FOUND;
	}

	/*Reading the Ext capability for compliance */
	Status = XDp_TxAuxRead(InstancePtr, XDP_DPCD_EXT_DPCD_REV,
								16, Dpcd_ext);
	if ((Dpcd_ext[6] & 0x1) == 0x1) {
		Status = XDp_TxAuxRead(InstancePtr, 0x0080,
									16, Dpcd_ext);

	}

	Status = XDp_TxAuxRead(InstancePtr, XDP_DPCD_RECEIVER_CAP_FIELD_START,
								16, Dpcd);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	RxMaxLinkRate = Dpcd[XDP_DPCD_MAX_LINK_RATE];
	RxMaxLaneCount = Dpcd[XDP_DPCD_MAX_LANE_COUNT] &
						XDP_DPCD_MAX_LANE_COUNT_MASK;
	LinkConfig->MaxLinkRate = (RxMaxLinkRate > ConfigPtr->MaxLinkRate) ?
				ConfigPtr->MaxLinkRate : RxMaxLinkRate;

	/* set MaxLinkRate to TX rate, if sink provides a non-standard value */
	if ((RxMaxLinkRate != XDP_TX_LINK_BW_SET_810GBPS) &&
		(RxMaxLinkRate != XDP_TX_LINK_BW_SET_540GBPS) &&
		(RxMaxLinkRate != XDP_TX_LINK_BW_SET_270GBPS) &&
		(RxMaxLinkRate != XDP_TX_LINK_BW_SET_162GBPS)) {
                LinkConfig->MaxLinkRate = ConfigPtr->MaxLinkRate;
        }
	if (InstancePtr->Config.DpProtocol == XDP_PROTOCOL_DP_1_4) {
		/* Check the EXTENDED_RECEIVER_CAPABILITY_FIELD_PRESENT bit */
		if(Dpcd[XDP_DPCD_TRAIN_AUX_RD_INTERVAL] &
		   XDP_DPCD_TRAIN_AUX_RD_EXT_RX_CAP_FIELD_PRESENT_MASK) {
			/* This is the check if the monitor is not
			 * setting 0x001 to 0x1E, but only setting it
			 * in 0x2201 as maxLinkRate. */
			u8 Data;
			/* Check extended capability register */
			XDp_TxAuxRead(InstancePtr, XDP_EDID_DPCD_MAX_LINK_RATE, 1, &Data);
			if(Data == XDP_TX_LINK_BW_SET_810GBPS) {
				RxMaxLinkRate = XDP_TX_LINK_BW_SET_810GBPS;
				LinkConfig->MaxLinkRate =
				(RxMaxLinkRate > ConfigPtr->MaxLinkRate) ?
				ConfigPtr->MaxLinkRate : RxMaxLinkRate;
			}
		}
	}

	if (!XDp_IsLinkRateValid(InstancePtr, LinkConfig->MaxLinkRate)) {
		return XST_FAILURE;
	}
	LinkConfig->MaxLaneCount = (RxMaxLaneCount > ConfigPtr->MaxLaneCount) ?
				ConfigPtr->MaxLaneCount : RxMaxLaneCount;
	if (!XDp_IsLaneCountValid(InstancePtr, LinkConfig->MaxLaneCount)) {
		return XST_FAILURE;
	}

	LinkConfig->SupportEnhancedFramingMode =
					Dpcd[XDP_DPCD_MAX_LANE_COUNT] &
					XDP_DPCD_ENHANCED_FRAME_SUPPORT_MASK;
	LinkConfig->SupportDownspreadControl =
					Dpcd[XDP_DPCD_MAX_DOWNSPREAD] &
					XDP_DPCD_MAX_DOWNSPREAD_MASK;

	/* Set default to Max lane count */
	InstancePtr->TxInstance.LinkConfig.cr_done_cnt =
		InstancePtr->TxInstance.LinkConfig.MaxLaneCount;
	InstancePtr->TxInstance.LinkConfig.cr_done_oldstate =
		InstancePtr->TxInstance.LinkConfig.MaxLaneCount;

	return XST_SUCCESS;
}

/******************************************************************************/
/**
 * This function will check if the immediate downstream RX device supports
 * TPS4 pattern mode. A DisplayPort Configuration Data (DPCD)
 * version of 1.4 is required TPS4_CAPABLE capability bit in the DPCD
 * must be set for this function to return XST_SUCCESS.
 *
 * @param	InstancePtr is a pointer to the XDp instance.
 *
 * @return
 *		- XST_SUCCESS if the RX device is TPS4 capable.
 *		- XST_NO_FEATURE if the RX device does not support TPS4.
*       - XST_DEVICE_NOT_FOUND if no RX device is connected.
 *		- XST_FAILURE otherwise - if an AUX read transaction failed.
 *
 * @note	None.
 *
*******************************************************************************/
u32 XDp_TxTp4Capable(XDp *InstancePtr)
{
	u32 Status;
	u8 AuxData;

	/* Verify arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertNonvoid(XDp_GetCoreType(InstancePtr) == XDP_TX);

	/* Check that the RX device has a DisplayPort Configuration Data (DPCD)
	 * version greater than or equal to 1.4. */
	Status = XDp_TxAuxRead(InstancePtr, XDP_DPCD_REV, 1, &AuxData);
	if (Status != XST_SUCCESS) {
		/* The AUX read transaction failed. */
		return Status;
	}
	else if (AuxData < 0x14) {
		return XST_NO_FEATURE;
	}

	/* Check if the RX device has TPS4 capabilities.. */
	Status = XDp_TxAuxRead(InstancePtr, XDP_DPCD_MAX_DOWNSPREAD,
				1, &AuxData);
	if (Status != XST_SUCCESS) {
		/* The AUX read transaction failed. */
		return Status;
	} else if ((AuxData & XDP_DPCD_TPS4_SUPPORT_MASK) !=
			XDP_DPCD_TPS4_SUPPORT_MASK) {
		return XST_NO_FEATURE;
	}

	return XST_SUCCESS;
}

/******************************************************************************/
/**
 * This function determines the common capabilities between the DisplayPort TX
 * core and the RX device.
 *
 * @param	InstancePtr is a pointer to the XDp instance.
 *
 * @return
 *		- XST_SUCCESS if main link settings were successfully set.
 *		- XST_DEVICE_NOT_FOUND if no RX device is connected.
 *		- XST_FAILURE otherwise.
 *
 * @note	None.
 *
*******************************************************************************/
u32 XDp_TxCfgMainLinkMax(XDp *InstancePtr)
{
	u32 Status;
	XDp_TxLinkConfig *LinkConfig = &InstancePtr->TxInstance.LinkConfig;

	/* Verify arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertNonvoid(XDp_GetCoreType(InstancePtr) == XDP_TX);
	/* The link rate and lane count will be checked in XDp_TxSetLinkRate and
	 * XDp_TxSetLaneCount. */

	if (!XDp_TxIsConnected(InstancePtr)) {
		return XST_DEVICE_NOT_FOUND;
	}

	/* Configure the main link to the maximum common link rate between the
	 * DisplayPort TX core and the RX device. */
	Status = XDp_TxSetLinkRate(InstancePtr, LinkConfig->MaxLinkRate);
	if (Status != XST_SUCCESS) {
		return Status;
	}

	/* Configure the main link to the maximum common lane count between the
	 * DisplayPort TX core and the RX device. */
	Status = XDp_TxSetLaneCount(InstancePtr, LinkConfig->MaxLaneCount);
	if (Status != XST_SUCCESS) {
		return Status;
	}

	return XST_SUCCESS;
}

/******************************************************************************/
/**
 * This function checks if the link needs training and runs the training
 * sequence if training is required.
 *
 * @param	InstancePtr is a pointer to the XDp instance.
 *
 * @return
 *		- XST_SUCCESS was either already trained, or has been
 *		  trained successfully.
 *		- XST_DEVICE_NOT_FOUND if no RX device is connected.
 *		- XST_FAILURE otherwise.
 *
 * @note	None.
 *
*******************************************************************************/
u32 XDp_TxEstablishLink(XDp *InstancePtr)
{
	u32 Status;
	u32 Status2;
	u32 ReenableMainLink;
	XDp_TxLinkConfig *LinkConfig = &InstancePtr->TxInstance.LinkConfig;

	/* Verify arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertNonvoid(XDp_GetCoreType(InstancePtr) == XDP_TX);
	Xil_AssertNonvoid(XDp_IsLinkRateValid(InstancePtr,
				LinkConfig->LinkRate));
	Xil_AssertNonvoid(XDp_IsLaneCountValid(InstancePtr,
				LinkConfig->LaneCount));

	XDp_TxResetPhy(InstancePtr, XDP_TX_PHY_CONFIG_PHY_RESET_MASK);

	/* Disable main link during training. */
	ReenableMainLink = XDp_ReadReg(InstancePtr->Config.BaseAddr,
						XDP_TX_ENABLE_MAIN_STREAM);
	if (ReenableMainLink) {
		XDp_TxDisableMainLink(InstancePtr);
	}

	/* Wait for the PHY to be ready. */
	Status = XDp_WaitPhyReady(InstancePtr,
			XDP_TX_PHY_STATUS_LANES_READY_MASK(
			InstancePtr->Config.MaxLaneCount));
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/* Train main link. */
	Status = XDp_TxRunTraining(InstancePtr);

	/* Turn off the training pattern and enable scrambler. */
	Status2 = XDp_TxSetTrainingPattern(InstancePtr,
					XDP_TX_TRAINING_PATTERN_SET_OFF);
	if ((Status != XST_SUCCESS) || (Status2 != XST_SUCCESS)) {
		return XST_FAILURE;
	}

	/* Re-enable main link after training if required. */
	if (ReenableMainLink) {
		XDp_TxEnableMainLink(InstancePtr);
	}

	return XST_SUCCESS;
}

/******************************************************************************/
/**
 * This function checks if the receiver's DisplayPort Configuration Data (DPCD)
 * indicates the receiver has achieved and maintained clock recovery, channel
 * equalization, symbol lock, and interlane alignment for all lanes currently in
 * use.
 *
 * @param	InstancePtr is a pointer to the XDp instance.
 * @param	LaneCount is the number of lanes to check.
 *
 * @return
 *		- XST_SUCCESS if the RX device has maintained clock recovery,
 *		  channel equalization, symbol lock, and interlane alignment.
 *		- XST_DEVICE_NOT_FOUND if no RX device is connected.
 *		- XST_FAILURE otherwise.
 *
 * @note	None.
 *
*******************************************************************************/
u32 XDp_TxCheckLinkStatus(XDp *InstancePtr, u8 LaneCount)
{
	u32 Status;
	u8 RetryCount = 0;

	/* Verify arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertNonvoid(XDp_GetCoreType(InstancePtr) == XDP_TX);
	Xil_AssertNonvoid(XDp_IsLaneCountValid(InstancePtr, LaneCount));

	if (!XDp_TxIsConnected(InstancePtr)) {
		return XST_DEVICE_NOT_FOUND;
	}

	/* Retrieve AUX info. */
	do {
		/* Get lane and adjustment requests. */
		Status = XDp_TxGetLaneStatusAdjReqs(InstancePtr);
		if (Status != XST_SUCCESS) {
			/* The AUX read failed. */
			return XST_FAILURE;
		}

		/* Check if the link needs training. */
		if ((XDp_TxCheckClockRecovery(
				InstancePtr, LaneCount) == XST_SUCCESS) &&
				(XDp_TxCheckChannelEqualization(
				InstancePtr, LaneCount) == XST_SUCCESS)) {
			return XST_SUCCESS;
		}

		RetryCount++;
	}
	while (RetryCount < 5); /* Retry up to 5 times. */

	return XST_FAILURE;
}

/******************************************************************************/
/**
 * This function enables or disables downshifting during the training process.
 *
 * @param	InstancePtr is a pointer to the XDp instance.
 * @param	Enable controls the downshift feature in the training process.
 *
 * @return	None.
 *
 * @note	None.
 *
*******************************************************************************/
void XDp_TxEnableTrainAdaptive(XDp *InstancePtr, u8 Enable)
{
	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(XDp_GetCoreType(InstancePtr) == XDP_TX);
	Xil_AssertVoid((Enable == 1) || (Enable == 0));

	InstancePtr->TxInstance.TrainAdaptive = Enable;
}

/******************************************************************************/
/**
 * This function sets a software switch that signifies whether or not a redriver
 * exists on the DisplayPort output path. XDp_TxSetVswingPreemp uses this switch
 * to determine which set of voltage swing and pre-emphasis values to use in the
 * TX core.
 *
 * @param	InstancePtr is a pointer to the XDp instance.
 * @param	Set establishes that a redriver exists in the DisplayPort output
 *		path.
 *
 * @return	None.
 *
 * @note	None.
 *
*******************************************************************************/
void XDp_TxSetHasRedriverInPath(XDp *InstancePtr, u8 Set)
{
	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(XDp_GetCoreType(InstancePtr) == XDP_TX);
	Xil_AssertVoid((Set == 1) || (Set == 0));

	InstancePtr->TxInstance.BoardChar.HasRedriverInPath = Set;
}

/******************************************************************************/
/**
 * This function sets the voltage swing offset to use during training when no
 * redriver exists. The offset will be added to the DisplayPort TX's voltage
 * swing level value when pre-emphasis is used (when the pre-emphasis level not
 * equal to 0).
 *
 * @param	InstancePtr is a pointer to the XDp instance.
 * @param	Offset is the value to set for the voltage swing offset.
 *
 * @return	None.
 *
 * @note	None.
 *
*******************************************************************************/
void XDp_TxCfgTxVsOffset(XDp *InstancePtr, u8 Offset)
{
	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(XDp_GetCoreType(InstancePtr) == XDP_TX);
	Xil_AssertVoid(Offset < 16);

	InstancePtr->TxInstance.BoardChar.TxVsOffset = Offset;
}

/******************************************************************************/
/**
 * This function sets the voltage swing level value in the DisplayPort TX that
 * will be used during link training for a given voltage swing training level.
 *
 * @param	InstancePtr is a pointer to the XDp instance.
 * @param	Level is the voltage swing training level to set the DisplayPort
 *		TX level for.
 * @param	TxLevel is the DisplayPort TX voltage swing level value to be
 *		used during link training.
 *
 * @return	None.
 *
 * @note	There are 16 possible voltage swing levels in the DisplayPort TX
 *		core that map to 4 possible voltage swing training levels in the
 *		RX device.
 *
*******************************************************************************/
void XDp_TxCfgTxVsLevel(XDp *InstancePtr, u8 Level, u8 TxLevel)
{
	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(XDp_GetCoreType(InstancePtr) == XDP_TX);
	Xil_AssertVoid(Level < 4);
	Xil_AssertVoid(TxLevel < 16);

	InstancePtr->TxInstance.BoardChar.TxVsLevels[Level] = TxLevel;
}

/******************************************************************************/
/**
 * This function sets the pre-emphasis level value in the DisplayPort TX that
 * will be used during link training for a given pre-emphasis training level.
 *
 * @param	InstancePtr is a pointer to the XDp instance.
 * @param	Level is the pre-emphasis training level to set the DisplayPort
 *		TX level for.
 * @param	TxLevel is the DisplayPort TX pre-emphasis level value to be
 *		used during link training.
 *
 * @return	None.
 *
 * @note	There are 32 possible pre-emphasis levels in the DisplayPort TX
 *		core that map to 4 possible pre-emphasis training levels in the
 *		RX device.
 *
*******************************************************************************/
void XDp_TxCfgTxPeLevel(XDp *InstancePtr, u8 Level, u8 TxLevel)
{
	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(XDp_GetCoreType(InstancePtr) == XDP_TX);
	Xil_AssertVoid(Level < 4);
	Xil_AssertVoid(TxLevel < 32);

	InstancePtr->TxInstance.BoardChar.TxPeLevels[Level] = TxLevel;
}

/******************************************************************************/
/**
 * This function checks if there is a connected RX device.
 *
 * @param	InstancePtr is a pointer to the XDp instance.
 *
 * @return
 *		- TRUE if there is a connection.
 *		- FALSE if there is no connection.
 *
*******************************************************************************/
u32 XDp_TxIsConnected(XDp *InstancePtr)
{
	u32 Status;
	u8 Retries = 0;

	/* Verify arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertNonvoid(XDp_GetCoreType(InstancePtr) == XDP_TX);

	do {
		Status = XDp_ReadReg(InstancePtr->Config.BaseAddr,
				XDP_TX_INTERRUPT_SIG_STATE) &
				XDP_TX_INTERRUPT_SIG_STATE_HPD_STATE_MASK;

		if (Retries > XDP_IS_CONNECTED_MAX_TIMEOUT_COUNT) {
			return 0;
		}

		Retries++;
		XDp_WaitUs(InstancePtr, 1000);
	} while (Status == 0);

	Retries = 0;
	if (InstancePtr->Config.DpProtocol == XDP_PROTOCOL_DP_1_4) {
		do {
			Status = XDp_ReadReg(InstancePtr->Config.BaseAddr,
					XDP_TX_INTERRUPT_SIG_STATE) &
					XDP_TX_INTERRUPT_SIG_STATE_HPD_STATE_MASK;

			if (Retries > XDP_IS_CONNECTED_MAX_TIMEOUT_COUNT)
				return 0;

			Retries++;
			XDp_WaitUs(InstancePtr, 1000);
		} while (Status == 0);
        }

	return 1;
}

/******************************************************************************/
/**
 * This function issues a read request over the AUX channel that will read from
 * the RX device's DisplayPort Configuration Data (DPCD) address space. The read
 * message will be divided into multiple transactions which read a maximum of 16
 * bytes each.
 *
 * @param	InstancePtr is a pointer to the XDp instance.
 * @param	DpcdAddress is the starting address to read from the RX device.
 * @param	BytesToRead is the number of bytes to read from the RX device.
 * @param	ReadData is a pointer to the data buffer that will be filled
 *		with read data.
 *
 * @return
 *		- XST_SUCCESS if the AUX read request was successfully
 *		  acknowledged.
 *		- XST_DEVICE_NOT_FOUND if no RX device is connected.
 *		- XST_ERROR_COUNT_MAX if the AUX request timed out.
 *		- XST_FAILURE otherwise.
 *
 * @note	None.
 *
*******************************************************************************/
u32 XDp_TxAuxRead(XDp *InstancePtr, u32 DpcdAddress, u32 BytesToRead,
								void *ReadData)
{
	u32 Status;

	/* Verify arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertNonvoid(XDp_GetCoreType(InstancePtr) == XDP_TX);
	Xil_AssertNonvoid(DpcdAddress <= 0xFFFFF);
	Xil_AssertNonvoid(BytesToRead <= 0xFFFFF);
	Xil_AssertNonvoid(ReadData != NULL);

	if (!XDp_TxIsConnected(InstancePtr)) {
		return XST_DEVICE_NOT_FOUND;
	}

	/* Send AUX read transaction. */
	Status = XDp_TxAuxCommon(InstancePtr, XDP_TX_AUX_CMD_READ, DpcdAddress,
						BytesToRead, (u8 *)ReadData);

	return Status;
}

/******************************************************************************/
/**
 * This function issues a write request over the AUX channel that will write to
 * the RX device's DisplayPort Configuration Data (DPCD) address space. The
 * write message will be divided into multiple transactions which write a
 * maximum of 16 bytes each.
 *
 * @param	InstancePtr is a pointer to the XDp instance.
 * @param	DpcdAddress is the starting address to write to the RX device.
 * @param	BytesToWrite is the number of bytes to write to the RX device.
 * @param	WriteData is a pointer to the data buffer that contains the data
 *		to be written to the RX device.
 *
 * @return
 *		- XST_SUCCESS if the AUX write request was successfully
 *		  acknowledged.
 *		- XST_DEVICE_NOT_FOUND if no RX device is connected.
 *		- XST_ERROR_COUNT_MAX if the AUX request timed out.
 *		- XST_FAILURE otherwise.
 *
 * @note	None.
 *
*******************************************************************************/
u32 XDp_TxAuxWrite(XDp *InstancePtr, u32 DpcdAddress, u32 BytesToWrite,
								void *WriteData)
{
	u32 Status;

	/* Verify arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertNonvoid(XDp_GetCoreType(InstancePtr) == XDP_TX);
	Xil_AssertNonvoid(DpcdAddress <= 0xFFFFF);
	Xil_AssertNonvoid(BytesToWrite <= 0xFFFFF);
	Xil_AssertNonvoid(WriteData != NULL);

	if (!XDp_TxIsConnected(InstancePtr)) {
		return XST_DEVICE_NOT_FOUND;
	}

	/* Send AUX write transaction. */
	Status = XDp_TxAuxCommon(InstancePtr, XDP_TX_AUX_CMD_WRITE, DpcdAddress,
						BytesToWrite, (u8 *)WriteData);

	return Status;
}

/******************************************************************************/
/**
 * This function performs an I2C read over the AUX channel. The read message
 * will be divided into multiple transactions if the requested data spans
 * multiple segments. The segment pointer is automatically incremented and the
 * offset is calibrated as needed. E.g. For an overall offset of:
 *	- 128, an I2C read is done on segptr=0; offset=128.
 *	- 256, an I2C read is done on segptr=1; offset=0.
 *	- 384, an I2C read is done on segptr=1; offset=128.
 *	- 512, an I2C read is done on segptr=2; offset=0.
 *	- etc.
 *
 * @param	InstancePtr is a pointer to the XDp instance.
 * @param	IicAddress is the address on the I2C bus of the target device.
 * @param	Offset is the offset at the specified address of the targeted
 *		I2C device that the read will start from.
 * @param	BytesToRead is the number of bytes to read.
 * @param	ReadData is a pointer to a buffer that will be filled with the
 *		I2C read data.
 *
 * @return
 *		- XST_SUCCESS if the I2C read has successfully completed with no
 *		  errors.
 *		- XST_ERROR_COUNT_MAX if the AUX request timed out.
 *		- XST_DEVICE_NOT_FOUND if no RX device is connected.
 *		- XST_FAILURE otherwise.
 *
 * @note	None.
 *
*******************************************************************************/
u32 XDp_TxIicRead(XDp *InstancePtr, u8 IicAddress, u16 Offset,
						u16 BytesToRead, void *ReadData)
{
	u32 Status;
	u8 SegPtr;
	u16 NumBytesLeftInSeg;
	u16 BytesLeft;
	u8 CurrBytesToRead;
	u8 Offset8;

	/* Verify arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertNonvoid(XDp_GetCoreType(InstancePtr) == XDP_TX);
	Xil_AssertNonvoid(ReadData != NULL);

	if (!XDp_TxIsConnected(InstancePtr)) {
		return XST_DEVICE_NOT_FOUND;
	}

	BytesLeft = BytesToRead;

	/* Reposition based on a segment length of 256 bytes. */
	SegPtr = 0;
	if (Offset > 255) {
		SegPtr += Offset / 256;
		Offset %= 256;
	}
	Offset8 = Offset;
	NumBytesLeftInSeg = 256 - Offset8;

	/* Set the segment pointer. */
	XDp_TxAuxCommon(InstancePtr,XDP_TX_AUX_CMD_I2C_WRITE_MOT, 
			XDP_SEGPTR_ADDR, 1, &SegPtr);

	/* Send I2C read message. Multiple transactions are required if the
	 * requested data spans multiple segments. */
	while (BytesLeft > 0) {
		/* Read the remaining number of bytes as requested. */
		if (NumBytesLeftInSeg >= BytesLeft) {
			CurrBytesToRead = BytesLeft;
		}
		/* Read the remaining data in the current segment boundary. */
		else {
			CurrBytesToRead = NumBytesLeftInSeg;
		}

		/* Setup the I2C-over-AUX read transaction with the offset. */
		Status = XDp_TxAuxCommon(InstancePtr,
				XDP_TX_AUX_CMD_I2C_WRITE_MOT, IicAddress, 1,
				&Offset8);
		if (Status != XST_SUCCESS) {
			return Status;
		}

		/* Send I2C-over-AUX read transaction. */
		Status = XDp_TxAuxCommon(InstancePtr, XDP_TX_AUX_CMD_I2C_READ,
				IicAddress, CurrBytesToRead, (u8 *)ReadData);
		if (Status != XST_SUCCESS) {
			return Status;
		}

		/* Previous I2C read was done on the remaining data in the
		 * current segment; prepare for next read. */
		if (BytesLeft > CurrBytesToRead) {
			BytesLeft -= CurrBytesToRead;
			Offset += CurrBytesToRead;
			ReadData += CurrBytesToRead;

			/* Increment the segment pointer to access more I2C
			 * address space, if required. */
			if (BytesLeft > 0) {
				NumBytesLeftInSeg = 256;
				Offset %= 256;
				SegPtr++;

				XDp_TxAuxCommon(InstancePtr,
				XDP_TX_AUX_CMD_I2C_WRITE_MOT, XDP_SEGPTR_ADDR, 1,
				&SegPtr);
			}
			Offset8 = Offset;
		}
		/* Last I2C read. */
		else {
			BytesLeft = 0;
		}
	}

	return Status;
}

/******************************************************************************/
/**
 * This function performs an I2C write over the AUX channel.
 *
 * @param	InstancePtr is a pointer to the XDp instance.
 * @param	IicAddress is the address on the I2C bus of the target device.
 * @param	BytesToWrite is the number of bytes to write.
 * @param	WriteData is a pointer to a buffer which will be used as the
 *		data source for the write.
 *
 * @return
 *		- XST_SUCCESS if the I2C write has successfully completed with
 *		  no errors.
 *		- XST_DEVICE_NOT_FOUND if no RX device is connected.
 *		- XST_ERROR_COUNT_MAX if the AUX request timed out.
 *		- XST_FAILURE otherwise.
 *
 * @note	None.
 *
*******************************************************************************/
u32 XDp_TxIicWrite(XDp *InstancePtr, u8 IicAddress, u8 BytesToWrite,
								void *WriteData)
{
	u32 Status;

	/* Verify arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertNonvoid(XDp_GetCoreType(InstancePtr) == XDP_TX);
	Xil_AssertNonvoid(WriteData != NULL);

	if (!XDp_TxIsConnected(InstancePtr)) {
		return XST_DEVICE_NOT_FOUND;
	}

	/* Send I2C-over-AUX read transaction. */
	Status = XDp_TxAuxCommon(InstancePtr, XDP_TX_AUX_CMD_I2C_WRITE,
				IicAddress, BytesToWrite, (u8 *)WriteData);

	return Status;
}

/******************************************************************************/
/**
 * This function enables or disables 0.5% spreading of the clock for both the
 * DisplayPort and the RX device.
 *
 * @param	InstancePtr is a pointer to the XDp instance.
 * @param	Enable will downspread the main link signal if set to 1 and
 *		disable downspreading if set to 0.
 *
 * @return
 *		- XST_SUCCESS if setting the downspread control enable was
 *		  successful.
 *		- XST_DEVICE_NOT_FOUND if no RX device is connected.
 *		- XST_FAILURE otherwise.
 *
 * @note	None.
 *
*******************************************************************************/
u32 XDp_TxSetDownspread(XDp *InstancePtr, u8 Enable)
{
	u32 Status;
	u8 RegVal;

	/* Verify arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertNonvoid(XDp_GetCoreType(InstancePtr) == XDP_TX);
	Xil_AssertNonvoid((Enable == 1) || (Enable == 0));

	if (!XDp_TxIsConnected(InstancePtr)) {
		return XST_DEVICE_NOT_FOUND;
	}

	InstancePtr->TxInstance.LinkConfig.DownspreadControl = Enable;

	/* Write downspread enable to the DisplayPort TX core. */
	XDp_WriteReg(InstancePtr->Config.BaseAddr, XDP_TX_DOWNSPREAD_CTRL,
			InstancePtr->TxInstance.LinkConfig.DownspreadControl);

	/* Preserve the current RX device settings. */
	Status = XDp_TxAuxRead(InstancePtr, XDP_DPCD_DOWNSPREAD_CTRL, 0x1,
								&RegVal);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}
	if (InstancePtr->TxInstance.LinkConfig.DownspreadControl) {
		RegVal |= XDP_DPCD_SPREAD_AMP_MASK;
	}
	else {
		RegVal &= ~XDP_DPCD_SPREAD_AMP_MASK;
	}

	/* Write downspread enable to the RX device. */
	Status = XDp_TxAuxWrite(InstancePtr, XDP_DPCD_DOWNSPREAD_CTRL, 0x1,
								&RegVal);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	return XST_SUCCESS;
}

/******************************************************************************/
/**
 * This function enables or disables the enhanced framing symbol sequence for
 * both the DisplayPort TX core and the RX device.
 *
 * @param	InstancePtr is a pointer to the XDp instance.
 * @param	Enable will enable enhanced frame mode if set to 1 and disable
 *		it if set to 0.
 *
 * @return
 *		- XST_SUCCESS if setting the enhanced frame mode enable was
 *		  successful.
 *		- XST_DEVICE_NOT_FOUND if no RX is connected.
 *		- XST_FAILURE otherwise.
 *
 * @note	None.
 *
*******************************************************************************/
u32 XDp_TxSetEnhancedFrameMode(XDp *InstancePtr, u8 Enable)
{
	u32 Status;
	u8 RegVal;

	/* Verify arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertNonvoid(XDp_GetCoreType(InstancePtr) == XDP_TX);
	Xil_AssertNonvoid((Enable == 1) || (Enable == 0));

	if (!XDp_TxIsConnected(InstancePtr)) {
		return XST_DEVICE_NOT_FOUND;
	}

	InstancePtr->TxInstance.LinkConfig.EnhancedFramingMode = Enable;

	/* Write enhanced frame mode enable to the DisplayPort TX core. */
	XDp_WriteReg(InstancePtr->Config.BaseAddr, XDP_TX_ENHANCED_FRAME_EN,
			InstancePtr->TxInstance.LinkConfig.EnhancedFramingMode);

	/* Preserve the current RX device settings. */
	Status = XDp_TxAuxRead(InstancePtr, XDP_DPCD_LANE_COUNT_SET, 0x1,
								&RegVal);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}
	if (InstancePtr->TxInstance.LinkConfig.EnhancedFramingMode) {
		RegVal |= XDP_DPCD_ENHANCED_FRAME_EN_MASK;
	}
	else {
		RegVal &= ~XDP_DPCD_ENHANCED_FRAME_EN_MASK;
	}

	/* Write enhanced frame mode enable to the RX device. */
	Status = XDp_TxAuxWrite(InstancePtr, XDP_DPCD_LANE_COUNT_SET, 0x1,
								&RegVal);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	return XST_SUCCESS;
}

/******************************************************************************/
/**
 * This function sets the number of lanes to be used by the main link for both
 * the DisplayPort TX core and the RX device.
 *
 * @param	InstancePtr is a pointer to the XDp instance.
 * @param	LaneCount is the number of lanes to be used over the main link.
 *
 * @return
 *		- XST_SUCCESS if setting the new lane count was successful.
 *		- XST_DEVICE_NOT_FOUND if no RX is connected.
 *		- XST_FAILURE otherwise.
 *
 * @note	None.
 *
*******************************************************************************/
u32 XDp_TxSetLaneCount(XDp *InstancePtr, u8 LaneCount)
{
	u32 Status;
	u8 RegVal;

	/* Verify arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertNonvoid(XDp_GetCoreType(InstancePtr) == XDP_TX);
	Xil_AssertNonvoid(XDp_IsLaneCountValid(InstancePtr, LaneCount));

	if (!XDp_TxIsConnected(InstancePtr)) {
		return XST_DEVICE_NOT_FOUND;
	}

	InstancePtr->TxInstance.LinkConfig.LaneCount = LaneCount;

	/* Write the new lane count to the DisplayPort TX core. */
	XDp_WriteReg(InstancePtr->Config.BaseAddr, XDP_TX_LANE_COUNT_SET,
				InstancePtr->TxInstance.LinkConfig.LaneCount);

	/* Preserve the current RX device settings. */
	Status = XDp_TxAuxRead(InstancePtr, XDP_DPCD_LANE_COUNT_SET, 0x1,
								&RegVal);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}
	RegVal &= ~XDP_DPCD_LANE_COUNT_SET_MASK;
	RegVal |= InstancePtr->TxInstance.LinkConfig.LaneCount;

	/* Write the new lane count to the RX device. */
	Status = XDp_TxAuxWrite(InstancePtr, XDP_DPCD_LANE_COUNT_SET, 0x1,
								&RegVal);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/* Invoke callback, if defined. */
	if (InstancePtr->TxInstance.LaneCountChangeCallback) {
		InstancePtr->TxInstance.LaneCountChangeCallback(
			InstancePtr->TxInstance.LaneCountChangeCallbackRef);
	}

	return XST_SUCCESS;
}

/******************************************************************************/
/**
 * This function sets the data rate to be used by the main link for both the
 * DisplayPort TX core and the RX device.
 *
 * @param	InstancePtr is a pointer to the XDp instance.
 * @param	LinkRate is the link rate to be used over the main link based on
 *		one of the following selects:
 *		- XDP_TX_LINK_BW_SET_162GBPS = 0x06 (for a 1.62 Gbps data rate)
 *		- XDP_TX_LINK_BW_SET_270GBPS = 0x0A (for a 2.70 Gbps data rate)
 *		- XDP_TX_LINK_BW_SET_540GBPS = 0x14 (for a 5.40 Gbps data rate)
 *
 * @return
 *		- XST_SUCCESS if setting the new link rate was successful.
 *		- XST_DEVICE_NOT_FOUND if no RX device is connected.
 *		- XST_FAILURE otherwise.
 *
 * @note	None.
 *
*******************************************************************************/
u32 XDp_TxSetLinkRate(XDp *InstancePtr, u8 LinkRate)
{
	u32 Status;

	/* Verify arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertNonvoid(XDp_GetCoreType(InstancePtr) == XDP_TX);
	Xil_AssertNonvoid(XDp_IsLinkRateValid(InstancePtr, LinkRate));

	if (!XDp_TxIsConnected(InstancePtr)) {
		return XST_DEVICE_NOT_FOUND;
	}

	/* Write a corresponding clock frequency to the DisplayPort TX core. */
	switch (LinkRate) {
	case XDP_TX_LINK_BW_SET_162GBPS:
		Status = XDp_TxSetClkSpeed(InstancePtr,
					XDP_TX_PHY_CLOCK_SELECT_162GBPS);
		break;
	case XDP_TX_LINK_BW_SET_270GBPS:
		Status = XDp_TxSetClkSpeed(InstancePtr,
					XDP_TX_PHY_CLOCK_SELECT_270GBPS);
		break;
	case XDP_TX_LINK_BW_SET_540GBPS:
		Status = XDp_TxSetClkSpeed(InstancePtr,
					XDP_TX_PHY_CLOCK_SELECT_540GBPS);
		break;
	case XDP_TX_LINK_BW_SET_810GBPS:
		if (InstancePtr->Config.DpProtocol == XDP_PROTOCOL_DP_1_4) {
			Status = XDp_TxSetClkSpeed(InstancePtr,
					XDP_TX_PHY_CLOCK_SELECT_810GBPS);
		} else {
			Status = XST_FAILURE;
		}
		break;
	default:
		Status = XST_FAILURE;
		break;
	}
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	InstancePtr->TxInstance.LinkConfig.LinkRate = LinkRate;

	/* Write new link rate to the DisplayPort TX core. */
	XDp_WriteReg(InstancePtr->Config.BaseAddr, XDP_TX_LINK_BW_SET,
				InstancePtr->TxInstance.LinkConfig.LinkRate);

	/* Write new link rate to the RX device. */
	Status = XDp_TxAuxWrite(InstancePtr, XDP_DPCD_LINK_BW_SET, 1,
				&InstancePtr->TxInstance.LinkConfig.LinkRate);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/* Invoke callback, if defined. */
	if (InstancePtr->TxInstance.LinkRateChangeCallback) {
		InstancePtr->TxInstance.LinkRateChangeCallback(
			InstancePtr->TxInstance.LinkRateChangeCallbackRef);
	}

	return XST_SUCCESS;
}

/******************************************************************************/
/**
 * This function enables or disables scrambling of symbols for both the
 * DisplayPort and the RX device.
 *
 * @param	InstancePtr is a pointer to the XDp instance.
 * @param	Enable will enable or disable scrambling.
 *
 * @return
 *		- XST_SUCCESS if setting the scrambling enable was successful.
 *		- XST_FAILURE otherwise.
 *
 * @note	None.
 *
*******************************************************************************/
u32 XDp_TxSetScrambler(XDp *InstancePtr, u8 Enable)
{
	u32 Status;
	u8 RegVal;

	/* Verify arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertNonvoid(XDp_GetCoreType(InstancePtr) == XDP_TX);
	Xil_AssertNonvoid((Enable == 1) || (Enable == 0));

	InstancePtr->TxInstance.LinkConfig.ScramblerEn = Enable;

	/* Write scrambler disable to the DisplayPort TX core. */
	XDp_WriteReg(InstancePtr->Config.BaseAddr, XDP_TX_SCRAMBLING_DISABLE,
							Enable ? 0x0 : 0x1);

	/* Preserve the current RX device settings. */
	Status = XDp_TxAuxRead(InstancePtr, XDP_DPCD_TP_SET, 1, &RegVal);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}
	if (Enable) {
		RegVal &= ~XDP_DPCD_TP_SET_SCRAMB_DIS_MASK;
	}
	else {
		RegVal |= XDP_DPCD_TP_SET_SCRAMB_DIS_MASK;
	}

	/* Write scrambler disable to the RX device. */
	Status = XDp_TxAuxWrite(InstancePtr, XDP_DPCD_TP_SET, 1, &RegVal);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	return XST_SUCCESS;
}

/******************************************************************************/
/**
 * This function enables the main link.
 *
 * @param	InstancePtr is a pointer to the XDp instance.
 *
 * @return	None.
 *
 * @note	None.
 *
*******************************************************************************/
void XDp_TxEnableMainLink(XDp *InstancePtr)
{
	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertVoid(XDp_GetCoreType(InstancePtr) == XDP_TX);

	/* Reset the scrambler. */
	XDp_WriteReg(InstancePtr->Config.BaseAddr,
					XDP_TX_FORCE_SCRAMBLER_RESET, 0x1);

	/* Enable the main stream. */
	XDp_WriteReg(InstancePtr->Config.BaseAddr,
						XDP_TX_ENABLE_MAIN_STREAM, 0x1);
}

/******************************************************************************/
/**
 * This function disables the main link.
 *
 * @param	InstancePtr is a pointer to the XDp instance.
 *
 * @return	None.
 *
 * @note	None.
 *
*******************************************************************************/
void XDp_TxDisableMainLink(XDp *InstancePtr)
{
	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertVoid(XDp_GetCoreType(InstancePtr) == XDP_TX);

	/* Reset the scrambler. */
	XDp_WriteReg(InstancePtr->Config.BaseAddr,
					XDP_TX_FORCE_SCRAMBLER_RESET, 0x1);

	/* Disable the main stream. */
	XDp_WriteReg(InstancePtr->Config.BaseAddr,
						XDP_TX_ENABLE_MAIN_STREAM, 0x0);
}

/******************************************************************************/
/**
 * This function does a PHY reset.
 *
 * @param	InstancePtr is a pointer to the XDp instance.
 * @param	Reset is the type of reset to assert.
 *
 * @return	None.
 *
 * @note	None.
 *
*******************************************************************************/
void XDp_TxResetPhy(XDp *InstancePtr, u32 Reset)
{
	u32 PhyVal;
	u32 RegVal;

	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertVoid(XDp_GetCoreType(InstancePtr) == XDP_TX);

	XDp_WriteReg(InstancePtr->Config.BaseAddr, XDP_TX_ENABLE, 0x0);

	/* Preserve the current PHY settings. */
	PhyVal = XDp_ReadReg(InstancePtr->Config.BaseAddr, XDP_TX_PHY_CONFIG);

	/* Apply reset. */
	RegVal = PhyVal | Reset;
	XDp_WriteReg(InstancePtr->Config.BaseAddr, XDP_TX_PHY_CONFIG, RegVal);

	/* Remove reset. */
	XDp_WriteReg(InstancePtr->Config.BaseAddr, XDP_TX_PHY_CONFIG, PhyVal);

	/* Wait for the PHY to be ready. */
	XDp_WaitPhyReady(InstancePtr, XDP_TX_PHY_STATUS_LANES_READY_MASK(
					InstancePtr->Config.MaxLaneCount));

	XDp_WriteReg(InstancePtr->Config.BaseAddr, XDP_TX_ENABLE, 0x1);
}

/******************************************************************************/
/**
 * This function sets the PHY polarity on all lanes.
 *
 * @param	InstancePtr is a pointer to the XDp instance.
 * @param	Polarity is the value to set for the polarity (0 or 1).
 *
 * @return	None.
 *
 * @note	The individual PHY polarity option will be disabled if set.
 *
*******************************************************************************/
void XDp_TxSetPhyPolarityAll(XDp *InstancePtr, u8 Polarity)
{
	u32 RegVal;

	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertVoid(XDp_GetCoreType(InstancePtr) == XDP_TX);
	Xil_AssertVoid((Polarity == 0) || (Polarity == 1));

	/* Preserve current settings. */
	RegVal = XDp_ReadReg(InstancePtr->Config.BaseAddr, XDP_TX_PHY_CONFIG);

	/* Set the polarity. */
	if (Polarity) {
		RegVal |= XDP_TX_PHY_CONFIG_TX_PHY_POLARITY_MASK;
	}
	else {
		RegVal &= ~XDP_TX_PHY_CONFIG_TX_PHY_POLARITY_MASK;
	}

	/* Disable individual polarity setting. */
	RegVal &= ~XDP_TX_PHY_CONFIG_TX_PHY_POLARITY_IND_LANE_MASK;

	/* Write the new settings. */
	XDp_WriteReg(InstancePtr->Config.BaseAddr, XDP_TX_PHY_CONFIG, RegVal);
}

/******************************************************************************/
/**
 * This function sets the PHY polarity on a specified lane.
 *
 * @param	InstancePtr is a pointer to the XDp instance.
 * @param	Lane is the lane number (0-3) to set the polarity for.
 * @param	Polarity is the value to set for the polarity (0 or 1).
 *
 * @return	None.
 *
 * @note	If individual lane polarity is used, it is recommended that this
 *		function is called for every lane in use.
 *
*******************************************************************************/
void XDp_TxSetPhyPolarityLane(XDp *InstancePtr, u8 Lane, u8 Polarity)
{
	u32 RegVal;
	u32 MaskVal;

	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertVoid(XDp_GetCoreType(InstancePtr) == XDP_TX);
	Xil_AssertVoid(Lane <= 3);
	Xil_AssertVoid((Polarity == 0) || (Polarity == 1));

	/* Preserve current settings. */
	RegVal = XDp_ReadReg(InstancePtr->Config.BaseAddr, XDP_TX_PHY_CONFIG);

	/* Determine bit mask to use. */
	switch (Lane) {
	case 0:
		MaskVal = XDP_TX_PHY_CONFIG_TX_PHY_POLARITY_LANE0_MASK;
		break;
	case 1:
		MaskVal = XDP_TX_PHY_CONFIG_TX_PHY_POLARITY_LANE1_MASK;
		break;
	case 2:
		MaskVal = XDP_TX_PHY_CONFIG_TX_PHY_POLARITY_LANE2_MASK;
		break;
	case 3:
		MaskVal = XDP_TX_PHY_CONFIG_TX_PHY_POLARITY_LANE3_MASK;
		break;
	default:
		break;
	}

	/* Set the polarity. */
	if (Polarity) {
		RegVal |= MaskVal;
	}
	else {
		RegVal &= ~MaskVal;
	}

	/* Enable individual polarity setting. */
	RegVal |= XDP_TX_PHY_CONFIG_TX_PHY_POLARITY_IND_LANE_MASK;

	/* Write the new settings. */
	XDp_WriteReg(InstancePtr->Config.BaseAddr, XDP_TX_PHY_CONFIG, RegVal);
}
#endif /* XPAR_XDPTXSS_NUM_INSTANCES */

#if XPAR_XDPRXSS_NUM_INSTANCES
/******************************************************************************/
/**
 * This function checks if the receiver's internal registers indicate that link
 * training has complete. That is, training has achieved channel equalization,
 * symbol lock, and interlane alignment for all lanes currently in use.
 *
 * @param	InstancePtr is a pointer to the XDp instance.
 *
 * @return
 *		- XST_SUCCESS if the RX device has achieved clock recovery,
 *		  channel equalization, symbol lock, and interlane alignment.
 *		- XST_FAILURE otherwise.
 *
 * @note	None.
 *
*******************************************************************************/
u32 XDp_RxCheckLinkStatus(XDp *InstancePtr)
{
	u8 LaneCount;
	u8 LaneStatus[2];

	/* Verify arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertNonvoid(XDp_GetCoreType(InstancePtr) == XDP_RX);

	LaneCount = XDp_ReadReg(InstancePtr->Config.BaseAddr,
						XDP_RX_DPCD_LANE_COUNT_SET);

	LaneStatus[0] = XDp_ReadReg(InstancePtr->Config.BaseAddr,
						XDP_RX_DPCD_LANE01_STATUS);
	LaneStatus[1] = XDp_ReadReg(InstancePtr->Config.BaseAddr,
						XDP_RX_DPCD_LANE23_STATUS);

	switch (LaneCount) {
	case 4:
		if (LaneStatus[1] != 0x77) {
			return XST_FAILURE;
		}
		/* FALLTHRU */
	case 2:
		if ((LaneStatus[0] & 0x70) != 0x70) {
			return XST_FAILURE;
		}
		/* FALLTHRU */
	case 1:
		if ((LaneStatus[0] & 0x07) != 0x07) {
			return XST_FAILURE;
		}
	}

	return XST_SUCCESS;
}

/******************************************************************************/
/**
 * This function enables the display timing generator (DTG).
 *
 * @param	InstancePtr is a pointer to the XDp instance.
 *
 * @return	None.
 *
 * @note	None.
 *
*******************************************************************************/
void XDp_RxDtgEn(XDp *InstancePtr)
{
	u32 ReadVal;

	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertVoid(XDp_GetCoreType(InstancePtr) == XDP_RX);

	ReadVal = XDp_ReadReg(InstancePtr->Config.BaseAddr, XDP_RX_DTG_ENABLE);

	XDp_WriteReg(InstancePtr->Config.BaseAddr, XDP_RX_DTG_ENABLE,
						(ReadVal | 0x1));
}

/******************************************************************************/
/**
 * This function disables the display timing generator (DTG).
 *
 * @param	InstancePtr is a pointer to the XDp instance.
 *
 * @return	None.
 *
 * @note	None.
 *
*******************************************************************************/
void XDp_RxDtgDis(XDp *InstancePtr)
{
	u32 ReadVal;

	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertVoid(XDp_GetCoreType(InstancePtr) == XDP_RX);

	ReadVal = XDp_ReadReg(InstancePtr->Config.BaseAddr, XDP_RX_DTG_ENABLE);

	XDp_WriteReg(InstancePtr->Config.BaseAddr, XDP_RX_DTG_ENABLE,
						(ReadVal & 0xFFFFFFFE));

	XDp_WriteReg(InstancePtr->Config.BaseAddr, XDP_RX_SOFT_RESET,
						XDP_RX_SOFT_RESET_VIDEO_MASK);
	XDp_WriteReg(InstancePtr->Config.BaseAddr, XDP_RX_SOFT_RESET, 0x0);
}

/******************************************************************************/
/**
 * This function sets the maximum data rate to be exposed in the RX device's
 * DisplayPort Configuration Data (DPCD) registers.
 *
 * @param	InstancePtr is a pointer to the XDp instance.
 * @param	LinkRate is the link rate to be used over the main link based on
 *		one of the following selects:
 *		- XDP_RX_LINK_BW_SET_162GBPS = 0x06 (for a 1.62 Gbps data rate)
 *		- XDP_RX_LINK_BW_SET_270GBPS = 0x0A (for a 2.70 Gbps data rate)
 *		- XDP_RX_LINK_BW_SET_540GBPS = 0x14 (for a 5.40 Gbps data rate)
 *		- XDP_RX_LINK_BW_SET_810GBPS = 0x1E (for a 8.10 Gbps data rate)
 *
 * @return	None.
 *
 * @note	None.
 *
*******************************************************************************/
void XDp_RxSetLinkRate(XDp *InstancePtr, u8 LinkRate)
{
	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertVoid(XDp_GetCoreType(InstancePtr) == XDP_RX);
	Xil_AssertVoid(XDp_IsLinkRateValid(InstancePtr, LinkRate));

	InstancePtr->RxInstance.LinkConfig.LinkRate = LinkRate;

	XDp_WriteReg(InstancePtr->Config.BaseAddr, XDP_RX_OVER_CTRL_DPCD, 0x1);
	if (LinkRate > XDP_LINK_BW_SET_540GBPS){
                XDp_WriteReg(InstancePtr->Config.BaseAddr,
				XDP_RX_OVER_LINK_BW_SET, XDP_LINK_BW_SET_540GBPS);
                XDp_WriteReg(InstancePtr->Config.BaseAddr,
				XDP_RX_EXT_OVER_LINK_BW_SET, LinkRate);
        } else {
                XDp_WriteReg(InstancePtr->Config.BaseAddr,
				XDP_RX_OVER_LINK_BW_SET, LinkRate);
                XDp_WriteReg(InstancePtr->Config.BaseAddr,
				XDP_RX_EXT_OVER_LINK_BW_SET, LinkRate);
        }
	XDp_WriteReg(InstancePtr->Config.BaseAddr, XDP_RX_OVER_CTRL_DPCD, 0x0);
}

/******************************************************************************/
/**
 * This function sets the maximum lane count to be exposed in the RX device's
 * DisplayPort Configuration Data (DPCD) registers.
 *
 * @param	InstancePtr is a pointer to the XDp instance.
 * @param	LaneCount is the number of lanes to be used over the main link.
 *
 * @return	None.
 *
 * @note	None.
 *
*******************************************************************************/
void XDp_RxSetLaneCount(XDp *InstancePtr, u8 LaneCount)
{
	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertVoid(XDp_GetCoreType(InstancePtr) == XDP_RX);
	Xil_AssertVoid(XDp_IsLaneCountValid(InstancePtr, LaneCount));

	InstancePtr->RxInstance.LinkConfig.LaneCount = LaneCount;

	/* Use enhanced framing mode to meet the DisplayPort specification. */
	LaneCount |= XDP_RX_OVER_LANE_COUNT_SET_ENHANCED_FRAME_CAP_MASK;
	/* If the core is a DisplayPort v1.2 or newer core, always support
	 * training pattern 3 to meet the specification. */
	if (InstancePtr->Config.DpProtocol) {
		LaneCount |= XDP_RX_OVER_LANE_COUNT_SET_TPS3_SUPPORTED_MASK;
	}

	XDp_WriteReg(InstancePtr->Config.BaseAddr, XDP_RX_OVER_CTRL_DPCD, 0x1);
	XDp_WriteReg(InstancePtr->Config.BaseAddr, XDP_RX_OVER_LANE_COUNT_SET,
								LaneCount);
	XDp_WriteReg(InstancePtr->Config.BaseAddr, XDP_RX_OVER_CTRL_DPCD, 0x0);
	XDp_WriteReg(InstancePtr->Config.BaseAddr, XDP_RX_LOCAL_EDID_VIDEO,
									0x1);
}

/******************************************************************************/
/**
 * This function enables audio stream packets on the main link.
 *
 * @param	InstancePtr is a pointer to the XDp instance.
 *
 * @return	None.
 *
 * @note	None.
 *
*******************************************************************************/
void XDp_RxAudioEn(XDp *InstancePtr)
{
	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertVoid(XDp_GetCoreType(InstancePtr) == XDP_RX);

	XDp_WriteReg(InstancePtr->Config.BaseAddr, XDP_RX_AUDIO_CONTROL, 0x1);
}

/******************************************************************************/
/**
 * This function disables audio stream packets on the main link.
 *
 * @param	InstancePtr is a pointer to the XDp instance.
 *
 * @return	None.
 *
 * @note	None.
 *
*******************************************************************************/
void XDp_RxAudioDis(XDp *InstancePtr)
{
	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertVoid(XDp_GetCoreType(InstancePtr) == XDP_RX);

	XDp_WriteReg(InstancePtr->Config.BaseAddr, XDP_RX_AUDIO_CONTROL, 0x0);
}

/******************************************************************************/
/**
 * This function enables MST audio for a given stream on the main link.
 *
 * @param      InstancePtr is a pointer to the XDp instance.
 * @param      Stream id
 *
 * @return      None.
 *
 * @note        None.
 *
 **********************************************************************************/
void XDp_Rx_Mst_AudioEn(XDp *InstancePtr, u8 StreamId)
{

	u32 ReadVal;

	/* Verify arguments. */
        Xil_AssertVoid(InstancePtr != NULL);
        Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
        Xil_AssertVoid(XDp_GetCoreType(InstancePtr) == XDP_RX);

	Xil_AssertVoid((StreamId == XDP_RX_STREAM_ID1) ||
                        (StreamId == XDP_RX_STREAM_ID2) ||
                        (StreamId == XDP_RX_STREAM_ID3) ||
                        (StreamId == XDP_RX_STREAM_ID4));

	ReadVal = (StreamId - 1) <<  XDP_RX_AUDIO_CONTROL_LANEX_SET_SHIFT ;
	XDp_WriteReg(InstancePtr->Config.BaseAddr, XDP_RX_AUDIO_CONTROL,
                       ReadVal);

	XDp_WriteReg(InstancePtr->Config.BaseAddr, XDP_RX_AUDIO_CONTROL,
			(ReadVal | 0x1));
}

/******************************************************************************/
/**
 * This function resets the RX core's reception of audio stream packets on the
 * main link.
 *
 * @param	InstancePtr is a pointer to the XDp instance.
 *
 * @return	None.
 *
 * @note	None.
 *
*******************************************************************************/
void XDp_RxAudioReset(XDp *InstancePtr)
{
	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertVoid(XDp_GetCoreType(InstancePtr) == XDP_RX);

	XDp_RxAudioDis(InstancePtr);
	XDp_WaitUs(InstancePtr, 1000);
	XDp_RxAudioEn(InstancePtr);
}

/******************************************************************************/
/**
 * This function enables the Video Stream Configuration.
 *
 * @param	InstancePtr is a pointer to the XDp instance.
 *
 * @return	None.
 *
 * @note	None.
 *
*******************************************************************************/
void XDp_RxVSCEn(XDp *InstancePtr)
{
	u32 ReadVal;

	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertVoid(XDp_GetCoreType(InstancePtr) == XDP_RX);

	ReadVal = XDp_ReadReg(InstancePtr->Config.BaseAddr, XDP_RX_DTG_ENABLE);
	XDp_WriteReg(InstancePtr->Config.BaseAddr,
		     XDP_RX_DTG_ENABLE, ReadVal | 0x4);
}

/******************************************************************************/
/**
 * This function disables the Video Stream Configuration.
 *
 * @param	InstancePtr is a pointer to the XDp instance.
 *
 * @return	None.
 *
 * @note	None.
 *
*******************************************************************************/
void XDp_RxVSCDis(XDp *InstancePtr)
{
	u32 ReadVal;

	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertVoid(XDp_GetCoreType(InstancePtr) == XDP_RX);

	ReadVal = XDp_ReadReg(InstancePtr->Config.BaseAddr, XDP_RX_DTG_ENABLE);
	XDp_WriteReg(InstancePtr->Config.BaseAddr,
		     XDP_RX_DTG_ENABLE, ReadVal & 0xFFFFFFFB);
}
#endif /* XPAR_XDPRXSS_NUM_INSTANCES */

/******************************************************************************/
/**
 * This function installs a custom delay/sleep function to be used by the XDp
 * driver.
 *
 * @param	InstancePtr is a pointer to the XDp instance.
 * @param	CallbackFunc is the address to the callback function.
 * @param	CallbackRef is the user data item (microseconds to delay) that
 *		will be passed to the custom sleep/delay function when it is
 *		invoked.
 *
 * @return	None.
 *
 * @note	None.
 *
*******************************************************************************/
void XDp_SetUserTimerHandler(XDp *InstancePtr, XDp_TimerHandler CallbackFunc,
							void *CallbackRef)
{
	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(CallbackFunc != NULL);
	Xil_AssertVoid(CallbackRef != NULL);

	InstancePtr->UserTimerWaitUs = CallbackFunc;
	InstancePtr->UserTimerPtr = CallbackRef;
}

/******************************************************************************/
/**
 * This function is the delay/sleep function for the XDp driver. For the Zynq
 * family, there exists native sleep functionality. For MicroBlaze however,
 * there does not exist such functionality. In the MicroBlaze case, the default
 * method for delaying is to use a predetermined amount of loop iterations. This
 * method is prone to inaccuracy and dependent on system configuration; for
 * greater accuracy, the user may supply their own delay/sleep handler, pointed
 * to by InstancePtr->UserTimerWaitUs, which may have better accuracy if a
 * hardware timer is used.
 *
 * @param	InstancePtr is a pointer to the XDp instance.
 * @param	MicroSeconds is the number of microseconds to delay/sleep for.
 *
 * @return	None.
 *
 * @note	None.
 *
*******************************************************************************/
void XDp_WaitUs(XDp *InstancePtr, u32 MicroSeconds)
{
	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	if (MicroSeconds == 0) {
		return;
	}

	/* Wait the requested amount of time. */
	if (InstancePtr->UserTimerWaitUs != NULL) {
		/* Use the timer handler specified by the user for better
		 * accuracy. */
		InstancePtr->UserTimerWaitUs(InstancePtr, MicroSeconds);
	}
	else {
		usleep(MicroSeconds);
	}
}

/******************************************************************************/
/**
 * This function checks the validity of the link rate.
 *
 * @param	InstancePtr is a pointer to the XDp instance.
 * @param	LinkRate is the link rate to check if valid.
 *
 * @return
 *		- 1 if specified link rate is valid.
 *		- 0 otherwise, if the link rate specified isn't valid as per
 *		  spec, or if it exceeds the capabilities of the TX core.
 *
 * @note	None.
 *
*******************************************************************************/
u8 XDp_IsLinkRateValid(XDp *InstancePtr, u8 LinkRate)
{
	u8 Valid;

	if ((LinkRate != XDP_LINK_BW_SET_162GBPS) &&
		(LinkRate != XDP_LINK_BW_SET_270GBPS) &&
		(LinkRate != XDP_LINK_BW_SET_540GBPS) &&
		(LinkRate != XDP_LINK_BW_SET_810GBPS)) {
		Valid = 0;
	}
	else if (LinkRate > InstancePtr->Config.MaxLinkRate) {
		Valid = 0;
	}
	else {
		Valid = 1;
	}

	return Valid;
}

/******************************************************************************/
/**
 * This function checks the validity of the lane count.
 *
 * @param	InstancePtr is a pointer to the XDp instance.
 * @param	LaneCount is the number of lanes to check if valid.
 *
 * @return
 *		- 1 if specified lane count is valid.
 *		- 0 otherwise, if the lane count specified isn't valid as per
 *		  spec, or if it exceeds the capabilities of the TX core.
 *
 * @note	None.
 *
*******************************************************************************/
u8 XDp_IsLaneCountValid(XDp *InstancePtr, u8 LaneCount)
{
	u8 Valid;

	if ((LaneCount != XDP_LANE_COUNT_SET_1) &&
			(LaneCount != XDP_LANE_COUNT_SET_2) &&
			(LaneCount != XDP_LANE_COUNT_SET_4)) {
		Valid = 0;
	}
	else if (LaneCount > InstancePtr->Config.MaxLaneCount) {
		Valid = 0;
	}
	else {
		Valid = 1;
	}

	return Valid;
}

#if XPAR_XDPTXSS_NUM_INSTANCES
/******************************************************************************/
/**
 * This function prepares the DisplayPort TX core for use.
 *
 * @param	InstancePtr is a pointer to the XDp instance.
 *
 * @return
 *		- XST_SUCCESS if the DisplayPort TX core was successfully
 *		  initialized.
 *		- XST_FAILURE otherwise.
 *
 * @note	None.
 *
*******************************************************************************/
static u32 XDp_TxInitialize(XDp *InstancePtr)
{
	u32 PhyVal;
	u32 RegVal;
	XDp_Config *ConfigPtr = &InstancePtr->Config;

	/* Preserve the current PHY settings. */
	PhyVal = XDp_ReadReg(ConfigPtr->BaseAddr, XDP_TX_PHY_CONFIG);

	/* Place the PHY (and GTTXRESET) into reset. */
	RegVal = PhyVal | XDP_TX_PHY_CONFIG_GT_ALL_RESET_MASK;
	XDp_WriteReg(ConfigPtr->BaseAddr, XDP_TX_PHY_CONFIG, RegVal);

	/* Reset the video streams and AUX logic. */
	XDp_WriteReg(ConfigPtr->BaseAddr, XDP_TX_SOFT_RESET,
		XDP_TX_SOFT_RESET_VIDEO_STREAM_ALL_MASK |
		XDP_TX_SOFT_RESET_AUX_MASK |
		XDP_TX_SOFT_RESET_HDCP_MASK);

	/* Disable the DisplayPort TX core. */
	XDp_WriteReg(ConfigPtr->BaseAddr, XDP_TX_ENABLE, 0);

	/* Set the clock divider. */
	RegVal = (XDp_ReadReg(ConfigPtr->BaseAddr, XDP_TX_AUX_CLK_DIVIDER) &
					~XDP_TX_AUX_CLK_DIVIDER_VAL_MASK) |
					(ConfigPtr->SAxiClkHz / 1000000);
	XDp_WriteReg(ConfigPtr->BaseAddr, XDP_TX_AUX_CLK_DIVIDER, RegVal);

	/* Set the DisplayPort TX core's clock speed. */
	switch (ConfigPtr->MaxLinkRate) {
	case XDP_TX_LINK_BW_SET_810GBPS:
		if (InstancePtr->Config.DpProtocol == XDP_PROTOCOL_DP_1_4) {
			XDp_WriteReg(ConfigPtr->BaseAddr, XDP_TX_PHY_CLOCK_SELECT,
						XDP_TX_PHY_CLOCK_SELECT_810GBPS);
		} else {
			return XST_FAILURE;
		}
		break;
	case XDP_TX_LINK_BW_SET_540GBPS:
		XDp_WriteReg(ConfigPtr->BaseAddr, XDP_TX_PHY_CLOCK_SELECT,
					XDP_TX_PHY_CLOCK_SELECT_540GBPS);
		break;
	case XDP_TX_LINK_BW_SET_270GBPS:
		XDp_WriteReg(ConfigPtr->BaseAddr, XDP_TX_PHY_CLOCK_SELECT,
					XDP_TX_PHY_CLOCK_SELECT_270GBPS);
		break;
	case XDP_TX_LINK_BW_SET_162GBPS:
		XDp_WriteReg(ConfigPtr->BaseAddr, XDP_TX_PHY_CLOCK_SELECT,
					XDP_TX_PHY_CLOCK_SELECT_162GBPS);
		break;
	default:
		break;
	}

	/* Bring the PHY (and GTTXRESET) out of reset. */
	RegVal = PhyVal & ~XDP_TX_PHY_CONFIG_GT_ALL_RESET_MASK;
	XDp_WriteReg(ConfigPtr->BaseAddr, XDP_TX_PHY_CONFIG, RegVal);

	/* Enable the DisplayPort TX core. */
	XDp_WriteReg(ConfigPtr->BaseAddr, XDP_TX_ENABLE, 1);

	/* Unmask Hot-Plug-Detect (HPD) interrupts. */
	XDp_WriteReg(ConfigPtr->BaseAddr, XDP_TX_INTERRUPT_MASK,
				~XDP_TX_INTERRUPT_MASK_HPD_PULSE_DETECTED_MASK &
				~XDP_TX_INTERRUPT_MASK_HPD_EVENT_MASK &
				~XDP_TX_INTERRUPT_MASK_HPD_IRQ_MASK);

	return XST_SUCCESS;
}
#endif /* XPAR_XDPTXSS_NUM_INSTANCES */

#if XPAR_XDPRXSS_NUM_INSTANCES
/******************************************************************************/
/**
 * This function sets the filter value for AUC_CLOCK_DIVIDER.
 *
 * @param	InstancePtr is a pointer to the XDp instance.
 *
 * @return	None.
 *
 * @note	None.
 *
 ******************************************************************************/
static void XDp_RxSetAuxClkFilterValue(XDp *InstancePtr)
{
	u32 filter_value = 0;
	u32 Regval;
	/*
	 * As per the DP spec the minimum AUX pulse width is 0.4us
	 * so the half clk is 2.5MHz
	 */
	filter_value = InstancePtr->Config.SAxiClkHz / 2500000;

	/*
	 * This is to set the allowable filter values as per the DpRx PG
	 * These are the allowable values
	 * 0(default), 8, 16, 24, 32, 40 and 48
	 */
	filter_value &= ~0x7;

	/*
	 * If filter value is more than the maximum allowable value(48),
	 * set it to max value (48)
	 */
	if (filter_value > 48)
		filter_value = 48;

	/* Set the AUX clock filter value */
	Regval = XDp_ReadReg(InstancePtr->Config.BaseAddr,
			XDP_RX_AUX_CLK_DIVIDER);
	Regval &= ~XDP_RX_AUX_CLK_DIVIDER_AUX_SIG_WIDTH_FILT_MASK;
	Regval |= (filter_value <<
			XDP_RX_AUX_CLK_DIVIDER_AUX_SIG_WIDTH_FILT_SHIFT);
	XDp_WriteReg(InstancePtr->Config.BaseAddr, XDP_RX_AUX_CLK_DIVIDER,
			Regval);
}

/******************************************************************************/
/**
 * This function prepares the DisplayPort RX core for use.
 *
 * @param	InstancePtr is a pointer to the XDp instance.
 *
 * @return
 *		- XST_SUCCESS if the DisplayPort RX core was successfully
 *		  initialized.
 *		- XST_FAILURE otherwise.
 *
 * @note	None.
 *
*******************************************************************************/
static u32 XDp_RxInitialize(XDp *InstancePtr)
{
	u32 Regval;
	/* Disable the main link. */
	XDp_WriteReg(InstancePtr->Config.BaseAddr, XDP_RX_LINK_ENABLE, 0x0);

	/* Set the AUX clock divider. */
	XDp_WriteReg(InstancePtr->Config.BaseAddr, XDP_RX_AUX_CLK_DIVIDER,
				(InstancePtr->Config.SAxiClkHz / 1000000));

	XDp_RxSetAuxClkFilterValue(InstancePtr);

	/* Put both GT RX/TX and CPLL into reset. */
	XDp_WriteReg(InstancePtr->Config.BaseAddr, XDP_RX_PHY_CONFIG,
					XDP_RX_PHY_CONFIG_GTPLL_RESET_MASK |
					XDP_RX_PHY_CONFIG_GTRX_RESET_MASK);

	/* Release CPLL reset. */
	XDp_WriteReg(InstancePtr->Config.BaseAddr, XDP_RX_PHY_CONFIG,
					XDP_RX_PHY_CONFIG_GTRX_RESET_MASK);

	/* Set the CDR tDLOCK timeout value. */
	XDp_WriteReg(InstancePtr->Config.BaseAddr, XDP_RX_CDR_CONTROL_CONFIG,
					XDP_RX_CDR_CONTROL_CONFIG_TDLOCK_DP159);

	/* Remove the reset from the PHY and configure to issue reset after
	 * every training iteration, link rate change, and start of training
	 * pattern. */
	XDp_WriteReg(InstancePtr->Config.BaseAddr, XDP_RX_PHY_CONFIG,
			XDP_RX_PHY_CONFIG_PHY_RESET_ENABLE_MASK |
			XDP_RX_PHY_CONFIG_RESET_AT_TRAIN_ITER_MASK |
			XDP_RX_PHY_CONFIG_RESET_AT_LINK_RATE_CHANGE_MASK |
			XDP_RX_PHY_CONFIG_RESET_AT_TP1_START_MASK);

	if (InstancePtr->Config.MstSupport) {
		XDp_WriteReg(InstancePtr->Config.BaseAddr, XDP_RX_MST_CAP,
						XDP_RX_MST_CAP_ENABLE_MASK |
						XDP_RX_MST_CAP_SOFT_VCP_MASK |
						XDP_RX_MST_CAP_OVER_ACT_MASK);
		XDp_WriteReg(InstancePtr->Config.BaseAddr,
						XDP_RX_INTERRUPT_MASK_1, 0x0);
		XDp_WriteReg(InstancePtr->Config.BaseAddr,
						XDP_RX_LOCAL_EDID_VIDEO, 0x0);

		Regval = XDp_ReadReg(InstancePtr->Config.BaseAddr,
						XDP_RX_CDR_CONTROL_CONFIG);

		XDp_WriteReg(InstancePtr->Config.BaseAddr,
				XDP_RX_CDR_CONTROL_CONFIG,
				Regval |
				XDP_RX_CDR_CONTROL_CONFIG_DISABLE_TIMEOUT);
		/* Sink count is set when exposing ports. */
	}
	else {
		XDp_WriteReg(InstancePtr->Config.BaseAddr,
						XDP_RX_MST_CAP, 0x0);
		XDp_WriteReg(InstancePtr->Config.BaseAddr,
						XDP_RX_LOCAL_EDID_VIDEO, 0x1);
		XDp_WriteReg(InstancePtr->Config.BaseAddr,
						XDP_RX_SINK_COUNT, 0x1);
		Regval = XDp_ReadReg(InstancePtr->Config.BaseAddr,
						XDP_RX_CDR_CONTROL_CONFIG);

		XDp_WriteReg(InstancePtr->Config.BaseAddr,
				XDP_RX_CDR_CONTROL_CONFIG,
				Regval &
				~(XDP_RX_CDR_CONTROL_CONFIG_DISABLE_TIMEOUT));
	}

	/* Set other link training parameters parameters.
	 *	Minimum voltage swing of 1.
	 *	Voltage swing sweep count of 4.
	 *	Maximum pre-emphasis level of 1. */
	XDp_WriteReg(InstancePtr->Config.BaseAddr, XDP_RX_MIN_VOLTAGE_SWING,
		1 | (XDP_RX_MIN_VOLTAGE_SWING_CR_OPT_VS_INC_4CNT <<
			XDP_RX_MIN_VOLTAGE_SWING_CR_OPT_SHIFT) |
		(2 << XDP_RX_MIN_VOLTAGE_SWING_VS_SWEEP_CNT_SHIFT) |
		(1 << XDP_RX_MIN_VOLTAGE_SWING_SET_PE_SHIFT));

	/* Set the AUX training interval. */
	XDp_WriteReg(InstancePtr->Config.BaseAddr, XDP_RX_OVER_CTRL_DPCD, 0x1);
	/* programming AUX defer to 6 */
	Regval = XDp_ReadReg(InstancePtr->Config.BaseAddr,
			     XDP_RX_AUX_CLK_DIVIDER);
	Regval |= Regval | (6 << XDP_RX_AUX_DEFER_SHIFT);
	XDp_WriteReg(InstancePtr->Config.BaseAddr,
		     XDP_RX_AUX_CLK_DIVIDER, Regval);
	if (InstancePtr->Config.DpProtocol == XDP_PROTOCOL_DP_1_4) {
		/* Set 16 ms as AUX read interval and
		 * set extended receiver capability*/
		XDp_WriteReg(InstancePtr->Config.BaseAddr, XDP_RX_OVER_TP_SET,
			(XDP_DPCD_TRAIN_AUX_RD_INT_16MS <<
			 XDP_RX_OVER_TP_SET_TRAINING_AUX_RD_INTERVAL_SHIFT) | 
			0x8000);
	} else {
		XDp_WriteReg(InstancePtr->Config.BaseAddr, XDP_RX_OVER_TP_SET,
			(XDP_DPCD_TRAIN_AUX_RD_INT_8MS <<
			 XDP_RX_OVER_TP_SET_TRAINING_AUX_RD_INTERVAL_SHIFT));
	}
	XDp_WriteReg(InstancePtr->Config.BaseAddr, XDP_RX_OVER_CTRL_DPCD, 0x0);
	/* Set the link configuration.*/
	XDp_RxSetLinkRate(InstancePtr,
			InstancePtr->RxInstance.LinkConfig.LinkRate);
	XDp_RxSetLaneCount(InstancePtr,
			InstancePtr->RxInstance.LinkConfig.LaneCount);

	/* Set the interrupt masks. */
	XDp_WriteReg(InstancePtr->Config.BaseAddr, XDP_RX_INTERRUPT_MASK, 0x0);
	if (InstancePtr->Config.DpProtocol == XDP_PROTOCOL_DP_1_4) {
		XDp_WriteReg(InstancePtr->Config.BaseAddr,
			     XDP_RX_INTERRUPT_MASK_1, 0x0);
	}

	/* Enable the RX core. */
	XDp_WriteReg(InstancePtr->Config.BaseAddr, XDP_RX_LINK_ENABLE, 0x1);

	/* Enable the display timing generator. */
	XDp_RxDtgEn(InstancePtr);

	return XST_SUCCESS;
}
#endif /* XPAR_XDPRXSS_NUM_INSTANCES */

#if XPAR_XDPTXSS_NUM_INSTANCES
/******************************************************************************/
/**
 * This function runs the link training process. It is implemented as a state
 * machine, with each state returning the next state. First, the clock recovery
 * sequence will be run; if successful, the channel equalization sequence will
 * run. If either the clock recovery or channel equalization sequence failed,
 * the link rate or the number of lanes used will be reduced and training will
 * be re-attempted. If training fails at the minimal data rate, 1.62 Gbps with
 * a single lane, training will no longer re-attempt and fail.
 *
 * @param	InstancePtr is a pointer to the XDp instance.
 *
 * @return
 *		- XST_SUCCESS if the training process succeeded.
 *		- XST_FAILURE otherwise.
 *
 * @note	None.
 *
*******************************************************************************/
static u32 XDp_TxRunTraining(XDp *InstancePtr)
{
	u32 Status;
	XDp_TxTrainingState TrainingState = XDP_TX_TS_CLOCK_RECOVERY;

	while (1) {
		switch (TrainingState) {
		case XDP_TX_TS_CLOCK_RECOVERY:
			TrainingState = XDp_TxTrainingStateClockRecovery(
								InstancePtr);
			break;
		case XDP_TX_TS_CHANNEL_EQUALIZATION:
			TrainingState = XDp_TxTrainingStateChannelEqualization(
								InstancePtr);
			break;
		case XDP_TX_TS_ADJUST_LINK_RATE:
			TrainingState = XDp_TxTrainingStateAdjustLinkRate(
								InstancePtr);
			break;
		case XDP_TX_TS_ADJUST_LANE_COUNT:
			TrainingState = XDp_TxTrainingStateAdjustLaneCount(
								InstancePtr);
			break;
		default:
			break;
		}

		if (TrainingState == XDP_TX_TS_SUCCESS) {
			InstancePtr->TxInstance.LinkConfig.cr_done_oldstate =
				InstancePtr->TxInstance.LinkConfig.MaxLaneCount;
			InstancePtr->TxInstance.LinkConfig.cr_done_cnt =
				InstancePtr->TxInstance.LinkConfig.MaxLaneCount;
			break;
		}
		else if (TrainingState == XDP_TX_TS_FAILURE) {
			InstancePtr->TxInstance.LinkConfig.cr_done_oldstate =
				InstancePtr->TxInstance.LinkConfig.MaxLaneCount;
			InstancePtr->TxInstance.LinkConfig.cr_done_cnt =
				InstancePtr->TxInstance.LinkConfig.MaxLaneCount;
			return XST_FAILURE;
		}

		if ((TrainingState == XDP_TX_TS_ADJUST_LINK_RATE) ||
			(TrainingState == XDP_TX_TS_ADJUST_LANE_COUNT)) {
			if (InstancePtr->TxInstance.TrainAdaptive == 0) {
				return XST_FAILURE;
			}

			Status = XDp_TxSetTrainingPattern(InstancePtr,
					XDP_TX_TRAINING_PATTERN_SET_OFF);
			if (Status != XST_SUCCESS) {
				return XST_FAILURE;
			}
		}
	}

	if (InstancePtr->Config.DpProtocol == XDP_PROTOCOL_DP_1_4) {
		/* Post Link Training ; Write 0x101 bit5 to
		 * set POST_LT_ADJ_REQ_GRANTED bit */
		int Data;
		Status = XDp_TxAuxRead(InstancePtr,
				       XDP_DPCD_LANE_COUNT_SET, 1, &Data);
		Data = Data | 0x20;
		Status = XDp_TxAuxWrite(InstancePtr,
					XDP_DPCD_LANE_COUNT_SET, 1, &Data);
	}

	/* Final status check. */
	Status = XDp_TxCheckLinkStatus(InstancePtr,
				InstancePtr->TxInstance.LinkConfig.LaneCount);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	return XST_SUCCESS;
}

/******************************************************************************/
/**
 * This function runs the clock recovery sequence as part of link training. The
 * sequence is as follows:
 *	0) Start signaling at the minimum voltage swing, pre-emphasis, and post-
 *	   cursor levels.
 *	1) Transmit training pattern 1 over the main link with symbol scrambling
 *	   disabled.
 *	2) The clock recovery loop. If clock recovery is unsuccessful after
 *	   MaxIterations loop iterations, return.
 *	2a) Wait for at least the period of time specified in the RX device's
 *	    DisplayPort Configuration Data (DPCD) register,
 *	    TRAINING_AUX_RD_INTERVAL.
 *	2b) Check if all lanes have achieved clock recovery lock. If so, return.
 *	2c) Check if the same voltage swing level has been used 5 consecutive
 *	    times or if the maximum level has been reached. If so, return.
 *	2d) Adjust the voltage swing, pre-emphasis, and post-cursor levels as
 *	    requested by the RX device.
 *	2e) Loop back to 2a.
 * For a more detailed description of the clock recovery sequence, see section
 * 3.5.1.2.1 of the DisplayPort 1.2a specification document.
 *
 * @param	InstancePtr is a pointer to the XDp instance.
 *
 * @return	The next training state:
 *		- XDP_TX_TS_CHANNEL_EQUALIZATION if the clock recovery sequence
 *		  completed successfully.
 *		- XDP_TX_TS_FAILURE if writing the drive settings to the RX
 *		  device was unsuccessful.
 *		- XDP_TX_TS_ADJUST_LINK_RATE if the clock recovery sequence
 *		  did not complete successfully.
 *
 * @note	None.
 *
*******************************************************************************/
static XDp_TxTrainingState XDp_TxTrainingStateClockRecovery(XDp *InstancePtr)
{
	u32 Status;
	u32 DelayUs;
	u8 PrevVsLevel = 0;
	u8 SameVsLevelCount = 0;
	XDp_TxLinkConfig *LinkConfig = &InstancePtr->TxInstance.LinkConfig;

	/* Obtain the required delay for clock recovery as specified by the
	 * RX device. */
	DelayUs = XDp_TxGetTrainingDelay(InstancePtr, XDP_TX_TS_CLOCK_RECOVERY);

	/* Start CRLock. */

	/* Transmit training pattern 1. */
	/* Disable the scrambler. */
	/* Start from minimal voltage swing and pre-emphasis levels. */
	InstancePtr->TxInstance.LinkConfig.VsLevel = 0;
	InstancePtr->TxInstance.LinkConfig.PeLevel = 0;
	Status = XDp_TxSetTrainingPattern(InstancePtr,
					XDP_TX_TRAINING_PATTERN_SET_TP1);
	if (Status != XST_SUCCESS) {
		return XDP_TX_TS_FAILURE;
	}

	while (1) {
		/* Wait delay specified in TRAINING_AUX_RD_INTERVAL. */
		XDp_WaitUs(InstancePtr, DelayUs);

		/* Get lane and adjustment requests. */
		Status = XDp_TxGetLaneStatusAdjReqs(InstancePtr);
		if (Status != XST_SUCCESS) {
			/* The AUX read failed. */
			return XDP_TX_TS_FAILURE;
		}

		/* Check if all lanes have realized and maintained the frequency
		 * lock and get adjustment requests. */
		Status = XDp_TxCheckClockRecovery(InstancePtr,
				InstancePtr->TxInstance.LinkConfig.LaneCount);
		if (Status == XST_SUCCESS) {
			return XDP_TX_TS_CHANNEL_EQUALIZATION;
		}

		/* Check if the same voltage swing for each lane has been used 5
		 * consecutive times. */
		if (PrevVsLevel == LinkConfig->VsLevel) {
			SameVsLevelCount++;
		}
		else {
			SameVsLevelCount = 0;
			PrevVsLevel = LinkConfig->VsLevel;
		}
		if (SameVsLevelCount >= 5) {
			break;
		}
		if (LinkConfig->VsLevel == XDP_TX_MAXIMUM_VS_LEVEL) {
			break;
		}

		/* Only try maximum voltage swing once. */
		if (LinkConfig->VsLevel == XDP_TX_MAXIMUM_VS_LEVEL) {
			break;
		}

		/* Adjust the drive settings as requested by the RX device. */
		Status = XDp_TxAdjVswingPreemp(InstancePtr);
		if (Status != XST_SUCCESS) {
			/* The AUX write failed. */
			return XDP_TX_TS_FAILURE;
		}
	}

	if (InstancePtr->TxInstance.LinkConfig.LinkRate == XDP_TX_LINK_BW_SET_162GBPS) {
		if (InstancePtr->TxInstance.LinkConfig.cr_done_cnt !=
			XDP_LANE_ALL_CR_DONE &&
			InstancePtr->TxInstance.LinkConfig.cr_done_cnt !=
			XDP_LANE_0_CR_DONE) {
			Status = XDp_TxSetTrainingPattern(InstancePtr,
				XDP_TX_TRAINING_PATTERN_SET_OFF);
			Status = XDp_TxSetLinkRate(InstancePtr,
				XDP_TX_LINK_BW_SET_810GBPS);
			Status |= XDp_TxSetLaneCount(InstancePtr,
				InstancePtr->TxInstance.LinkConfig.cr_done_cnt);
			InstancePtr->TxInstance.LinkConfig.cr_done_oldstate =
				InstancePtr->TxInstance.LinkConfig.cr_done_cnt;
			return XDP_TX_TS_CLOCK_RECOVERY;
		}
	}

	return XDP_TX_TS_ADJUST_LINK_RATE;
}

/******************************************************************************/
/**
 * This function runs the channel equalization sequence as part of link
 * training. The sequence is as follows:
 *	0) Start signaling with the same drive settings used at the end of the
 *	   clock recovery sequence.
 *	1) Transmit training pattern 2 (or 3) over the main link with symbol
 *	   scrambling disabled.
 *	2) The channel equalization loop. If channel equalization is
 *	   unsuccessful after 5 loop iterations, return.
 *	2a) Wait for at least the period of time specified in the RX device's
 *	    DisplayPort Configuration Data (DPCD) register,
 *	    TRAINING_AUX_RD_INTERVAL.
 *	2b) Check if all lanes have achieved channel equalization, symbol lock,
 *	    and interlane alignment. If so, return.
 *	2c) Check if the same voltage swing level has been used 5 consecutive
 *	    times or if the maximum level has been reached. If so, return.
 *	2d) Adjust the voltage swing, pre-emphasis, and post-cursor levels as
 *	    requested by the RX device.
 *	2e) Loop back to 2a.
 * For a more detailed description of the channel equalization sequence, see
 * section 3.5.1.2.2 of the DisplayPort 1.2a specification document.
 *
 * @param	InstancePtr is a pointer to the XDp instance.
 *
 * @return	The next training state:
 *		- XDP_TX_TS_SUCCESS if training succeeded.
 *		- XDP_TX_TS_FAILURE if writing the drive settings to the RX
 *		  device was unsuccessful.
 *		- XDP_TX_TS_ADJUST_LINK_RATE if, after 5 loop iterations, the
 *		  channel equalization sequence did not complete successfully.
 *
 * @note	None.
 *
*******************************************************************************/
static XDp_TxTrainingState XDp_TxTrainingStateChannelEqualization(
							XDp *InstancePtr)
{
	u32 Status = XST_SUCCESS;
	u32 DelayUs;
	u32 IterationCount = 0;
	u8 cr_failure = 0;
	u8 ce_failure = 0;

	/* Obtain the required delay for channel equalization as specified by
	 * the RX device. */
	DelayUs = XDp_TxGetTrainingDelay(InstancePtr,
						XDP_TX_TS_CHANNEL_EQUALIZATION);

	/* Start channel equalization. */

	/* Write the current drive settings. */
	/* Transmit training pattern 2/3. */
	if (InstancePtr->TxInstance.RxConfig.
				DpcdRxCapsField[XDP_DPCD_MAX_DOWNSPREAD] &
				XDP_DPCD_TPS4_SUPPORT_MASK) {
		if (InstancePtr->Config.DpProtocol == XDP_PROTOCOL_DP_1_4) {
			Status = XDp_TxSetTrainingPattern(InstancePtr,
					XDP_TX_TRAINING_PATTERN_SET_TP4);
		}
	} else if (InstancePtr->TxInstance.RxConfig.
				DpcdRxCapsField[XDP_DPCD_MAX_LANE_COUNT] &
				XDP_DPCD_TPS3_SUPPORT_MASK) {
		Status = XDp_TxSetTrainingPattern(InstancePtr,
					XDP_TX_TRAINING_PATTERN_SET_TP3);
	} else {
		Status = XDp_TxSetTrainingPattern(InstancePtr,
					XDP_TX_TRAINING_PATTERN_SET_TP2);
	}
	
	if (Status != XST_SUCCESS) {
		return XDP_TX_TS_FAILURE;
	}

	while (IterationCount < 5) {
		/* Wait delay specified in TRAINING_AUX_RD_INTERVAL. */
		XDp_WaitUs(InstancePtr, DelayUs);

		/* Get lane and adjustment requests. */
		Status = XDp_TxGetLaneStatusAdjReqs(InstancePtr);
		if (Status != XST_SUCCESS) {
			/* The AUX read failed. */
			return XDP_TX_TS_FAILURE;
		}

		/* Check that all lanes still have their clocks locked. */
		Status = XDp_TxCheckClockRecovery(InstancePtr,
				InstancePtr->TxInstance.LinkConfig.LaneCount);
		if (Status != XST_SUCCESS) {
			cr_failure = 1;
			break;
		}

		/* Check if all lanes have accomplished channel equalization,
		 * symbol lock, and interlane alignment. */
		Status = XDp_TxCheckChannelEqualization(InstancePtr,
				InstancePtr->TxInstance.LinkConfig.LaneCount);
		if (Status == XST_SUCCESS) {
			ce_failure = 0;
			return XDP_TX_TS_SUCCESS;
		} else {
			ce_failure = 1;
		}

		/* Adjust the drive settings as requested by the RX device. */
		Status = XDp_TxAdjVswingPreemp(InstancePtr);
		if (Status != XST_SUCCESS) {
			/* The AUX write failed. */
			return XDP_TX_TS_FAILURE;
		}

		IterationCount++;
	}

	/* Tried 5 times with no success. Try a reduced bitrate first, then
	 * reduce the number of lanes. */
	if (InstancePtr->Config.DpProtocol != XDP_PROTOCOL_DP_1_4) {
		return XDP_TX_TS_ADJUST_LINK_RATE;
	} else {
		if (cr_failure) {
			/* DP1.4 asks to downlink on CR failure in EQ stage */
			InstancePtr->TxInstance.LinkConfig.cr_done_oldstate =
				InstancePtr->TxInstance.LinkConfig.MaxLaneCount;
			return XDP_TX_TS_ADJUST_LINK_RATE;
		} else if (InstancePtr->TxInstance.LinkConfig.LaneCount == 1 && (ce_failure)) {
			/* needed to set lanecount for next iter */
			InstancePtr->TxInstance.LinkConfig.LaneCount =
				InstancePtr->TxInstance.LinkConfig.MaxLaneCount;
			InstancePtr->TxInstance.LinkConfig.cr_done_oldstate =
				InstancePtr->TxInstance.LinkConfig.MaxLaneCount;
			return XDP_TX_TS_ADJUST_LINK_RATE;
		} else if (ce_failure && InstancePtr->TxInstance.LinkConfig.LaneCount > 1) {
			/* For EQ failure downlink the lane count */
			return XDP_TX_TS_ADJUST_LANE_COUNT;
		} else {
			InstancePtr->TxInstance.LinkConfig.cr_done_oldstate =
				InstancePtr->TxInstance.LinkConfig.MaxLaneCount;
			return XDP_TX_TS_ADJUST_LINK_RATE;
		}
	}
}

/******************************************************************************/
/**
 * This function is reached if either the clock recovery or the channel
 * equalization process failed during training. As a result, the data rate will
 * be downshifted, and training will be re-attempted (starting with clock
 * recovery) at the reduced data rate. If the data rate is already at 1.62 Gbps,
 * a downshift in lane count will be attempted.
 *
 * @param	InstancePtr is a pointer to the XDp instance.
 *
 * @return	The next training state:
 *		- XDP_TX_TS_ADJUST_LANE_COUNT if the minimal data rate is
 *		  already in use. Re-attempt training at a reduced lane count.
 *		- XDP_TX_TS_CLOCK_RECOVERY otherwise. Re-attempt training.
 *
 * @note	None.
 *
*******************************************************************************/
static XDp_TxTrainingState XDp_TxTrainingStateAdjustLinkRate(XDp *InstancePtr)
{
	u32 Status;

	switch (InstancePtr->TxInstance.LinkConfig.LinkRate) {
	case XDP_TX_LINK_BW_SET_810GBPS:
		if (InstancePtr->Config.DpProtocol == XDP_PROTOCOL_DP_1_4) {
			Status = XDp_TxSetLinkRate(InstancePtr,
						XDP_TX_LINK_BW_SET_540GBPS);
			/* UCD400 expects the Lane to be set here
			   it has to match the max cap of Sink */
			Status = XDp_TxSetLaneCount(InstancePtr,
				InstancePtr->TxInstance.LinkConfig.cr_done_oldstate);

			if (Status != XST_SUCCESS) {
				Status = XDP_TX_TS_FAILURE;
				break;
			}
			Status = XDP_TX_TS_CLOCK_RECOVERY;
			break;
		}
		/* FALLTHRU */
	case XDP_TX_LINK_BW_SET_540GBPS:
		Status = XDp_TxSetLinkRate(InstancePtr,
						XDP_TX_LINK_BW_SET_270GBPS);
		/* UCD400 expects the Lane to be set here
		   it has to match the max cap of Sink */
		Status = XDp_TxSetLaneCount(InstancePtr,
			InstancePtr->TxInstance.LinkConfig.cr_done_oldstate);

		if (Status != XST_SUCCESS) {
			Status = XDP_TX_TS_FAILURE;
			break;
		}
		Status = XDP_TX_TS_CLOCK_RECOVERY;
		break;
	case XDP_TX_LINK_BW_SET_270GBPS:
		Status = XDp_TxSetLinkRate(InstancePtr,
						XDP_TX_LINK_BW_SET_162GBPS);
		/* UCD400 expects the Lane to be set here
		   it has to match the max cap of Sink */
		Status = XDp_TxSetLaneCount(InstancePtr,
			InstancePtr->TxInstance.LinkConfig.cr_done_oldstate);

		if (Status != XST_SUCCESS) {
			Status = XDP_TX_TS_FAILURE;
			break;
		}
		Status = XDP_TX_TS_CLOCK_RECOVERY;
		break;
	default:
	/* Already at the lowest link rate. Try reducing the lane
	 * count next. */
		Status = XDP_TX_TS_ADJUST_LANE_COUNT;
		break;
	}

	return Status;
}

/******************************************************************************/
/**
 * This function is reached if either the clock recovery or the channel
 * equalization process failed during training, and a minimal data rate of 1.62
 * Gbps was being used. As a result, the number of lanes in use will be reduced,
 * and training will be re-attempted (starting with clock recovery) at this
 * lower lane count.
 *
 * @note	Training will be re-attempted with the maximum data rate being
 *		used with the reduced lane count to train at the main link at
 *		the maximum bandwidth possible.
 *
 * @param	InstancePtr is a pointer to the XDp instance.
 *
 * @return	The next training state:
 *		- XDP_TX_TS_FAILURE if only one lane is already in use.
 *		- XDP_TX_TS_CLOCK_RECOVERY otherwise. Re-attempt training.
 *
 * @note	None.
 *
*******************************************************************************/
static XDp_TxTrainingState XDp_TxTrainingStateAdjustLaneCount(XDp *InstancePtr)
{
	u32 Status;
	XDp_TxLinkConfig *LinkConfig = &InstancePtr->TxInstance.LinkConfig;

	switch (LinkConfig->LaneCount) {
	case XDP_TX_LANE_COUNT_SET_4:
		Status = XDp_TxSetLaneCount(InstancePtr,
						XDP_TX_LANE_COUNT_SET_2);
		if (Status != XST_SUCCESS) {
			Status = XDP_TX_TS_FAILURE;
			break;
		}

		Status = XDp_TxSetLinkRate(InstancePtr,
						LinkConfig->MaxLinkRate);
		if (Status != XST_SUCCESS) {
			Status = XDP_TX_TS_FAILURE;
			break;
		}
		Status = XDP_TX_TS_CLOCK_RECOVERY;
		break;
	case XDP_TX_LANE_COUNT_SET_2:
		Status = XDp_TxSetLaneCount(InstancePtr,
						XDP_TX_LANE_COUNT_SET_1);
		if (Status != XST_SUCCESS) {
			Status = XDP_TX_TS_FAILURE;
			break;
		}

		Status = XDp_TxSetLinkRate(InstancePtr,
						LinkConfig->MaxLinkRate);
		if (Status != XST_SUCCESS) {
			Status = XDP_TX_TS_FAILURE;
			break;
		}
		Status = XDP_TX_TS_CLOCK_RECOVERY;
		break;
	default:
		/* Already at the lowest lane count. Training has failed at the
		 * lowest lane count and link rate. */
		Status = XDP_TX_TS_FAILURE;
		break;
	}

	return Status;
}

/******************************************************************************/
/**
 * This function will do a burst AUX read from the RX device over the AUX
 * channel. The contents of the status registers will be stored for later use by
 * XDp_TxCheckClockRecovery, XDp_TxCheckChannelEqualization, and
 * XDp_TxAdjVswingPreemp.
 *
 * @param	InstancePtr is a pointer to the XDp instance.
 *
 * @return
 *		- XST_SUCCESS if the AUX read was successful.
 *		- XST_FAILURE otherwise.
 *
 * @note	None.
 *
*******************************************************************************/
static u32 XDp_TxGetLaneStatusAdjReqs(XDp *InstancePtr)
{
	u32 Status;

	/* Read and store 4 bytes of lane status and 2 bytes of adjustment
	 * requests. */
	Status = XDp_TxAuxRead(InstancePtr, XDP_DPCD_STATUS_LANE_0_1,
			6, InstancePtr->TxInstance.RxConfig.LaneStatusAdjReqs);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	return XST_SUCCESS;
}

/******************************************************************************/
/**
 * This function checks if the RX device's DisplayPort Configuration Data (DPCD)
 * indicates that the clock recovery sequence during link training was
 * successful - the RX device's link clock and data recovery unit has realized
 * and maintained the frequency lock for all lanes currently in use.
 *
 * @param	InstancePtr is a pointer to the XDp instance.
 * @param	LaneCount is the number of lanes to check.
 *
 * @return
 *		- XST_SUCCESS if the RX device's clock recovery PLL has
 *		  achieved frequency lock for all lanes in use.
 *		- XST_FAILURE otherwise.
 *
 * @note	None.
 *
*******************************************************************************/
static u32 XDp_TxCheckClockRecovery(XDp *InstancePtr, u8 LaneCount)
{
	u8 *LaneStatus = InstancePtr->TxInstance.RxConfig.LaneStatusAdjReqs;

	/* Check that all LANEx_CR_DONE bits are set. */
	switch (LaneCount) {
	case XDP_TX_LANE_COUNT_SET_4:
		if (!(LaneStatus[0] &
			XDP_DPCD_STATUS_LANE_0_CR_DONE_MASK)) {
			InstancePtr->TxInstance.LinkConfig.cr_done_cnt =
				XDP_LANE_0_CR_DONE;
			return XST_FAILURE;
		}
		if (!(LaneStatus[0] &
				XDP_DPCD_STATUS_LANE_1_CR_DONE_MASK)) {
			InstancePtr->TxInstance.LinkConfig.cr_done_cnt =
				XDP_LANE_1_CR_DONE;
			return XST_FAILURE;
		}
		if (!(LaneStatus[1] &
				XDP_DPCD_STATUS_LANE_2_CR_DONE_MASK)) {
			InstancePtr->TxInstance.LinkConfig.cr_done_cnt =
				XDP_LANE_2_CR_DONE;
			return XST_FAILURE;
		}
		if (!(LaneStatus[1] &
				XDP_DPCD_STATUS_LANE_3_CR_DONE_MASK)) {
			InstancePtr->TxInstance.LinkConfig.cr_done_cnt =
				XDP_LANE_3_CR_DONE;
			return XST_FAILURE;
		}
		InstancePtr->TxInstance.LinkConfig.cr_done_cnt =
			XDP_LANE_ALL_CR_DONE;
		/* Drop through and check lane 1. */
		/* FALLTHRU */
	case XDP_TX_LANE_COUNT_SET_2:
		if (!(LaneStatus[0] &
				XDP_DPCD_STATUS_LANE_0_CR_DONE_MASK)) {
			InstancePtr->TxInstance.LinkConfig.cr_done_cnt =
				XDP_LANE_0_CR_DONE;
			return XST_FAILURE;
		}
		if (!(LaneStatus[0] &
				XDP_DPCD_STATUS_LANE_1_CR_DONE_MASK)) {
			InstancePtr->TxInstance.LinkConfig.cr_done_cnt =
				XDP_LANE_1_CR_DONE;
			return XST_FAILURE;
		}
		InstancePtr->TxInstance.LinkConfig.cr_done_cnt =
				XDP_LANE_2_CR_DONE;
		/* Drop through and check lane 0. */
		/* FALLTHRU */
	case XDP_TX_LANE_COUNT_SET_1:
		if (!(LaneStatus[0] &
				XDP_DPCD_STATUS_LANE_0_CR_DONE_MASK)) {
			InstancePtr->TxInstance.LinkConfig.cr_done_cnt =
				XDP_LANE_0_CR_DONE;
			return XST_FAILURE;
		}
		InstancePtr->TxInstance.LinkConfig.cr_done_cnt =
			XDP_LANE_1_CR_DONE;
		/* FALLTHRU */
	default:
		/* All (LaneCount) lanes have achieved clock recovery. */
		break;
	}

	return XST_SUCCESS;
}

/******************************************************************************/
/**
 * This function checks if the RX device's DisplayPort Configuration Data (DPCD)
 * indicates that the channel equalization sequence during link training was
 * successful - the RX device has achieved channel equalization, symbol lock,
 * and interlane alignment for all lanes currently in use.
 *
 * @param	InstancePtr is a pointer to the XDp instance.
 * @param	LaneCount is the number of lanes to check.
 *
 * @return
 *		- XST_SUCCESS if the RX device has achieved channel
 *		  equalization symbol lock, and interlane alignment for all
 *		  lanes in use.
 *		- XST_FAILURE otherwise.
 *
 * @note	None.
 *
*******************************************************************************/
static u32 XDp_TxCheckChannelEqualization(XDp *InstancePtr, u8 LaneCount)
{
	u8 *LaneStatus = InstancePtr->TxInstance.RxConfig.LaneStatusAdjReqs;

	/* Check that all LANEx_CHANNEL_EQ_DONE bits are set. */
	switch (LaneCount) {
	case XDP_TX_LANE_COUNT_SET_4:
		if (!(LaneStatus[1] &
				XDP_DPCD_STATUS_LANE_3_CE_DONE_MASK)) {
			return XST_FAILURE;
		}
		if (!(LaneStatus[1] &
				XDP_DPCD_STATUS_LANE_2_CE_DONE_MASK)) {
			return XST_FAILURE;
		}
		/* Drop through and check lane 1. */
		/* FALLTHRU */
	case XDP_TX_LANE_COUNT_SET_2:
		if (!(LaneStatus[0] &
				XDP_DPCD_STATUS_LANE_1_CE_DONE_MASK)) {
			return XST_FAILURE;
		}
		/* Drop through and check lane 0. */
		/* FALLTHRU */
	case XDP_TX_LANE_COUNT_SET_1:
		if (!(LaneStatus[0] &
				XDP_DPCD_STATUS_LANE_0_CE_DONE_MASK)) {
			return XST_FAILURE;
		}
		/* FALLTHRU */
	default:
		/* All (LaneCount) lanes have achieved channel equalization. */
		break;
	}

	/* Check that all LANEx_SYMBOL_LOCKED bits are set. */
	switch (LaneCount) {
	case XDP_TX_LANE_COUNT_SET_4:
		if (!(LaneStatus[1] &
				XDP_DPCD_STATUS_LANE_3_SL_DONE_MASK)) {
			return XST_FAILURE;
		}
		if (!(LaneStatus[1] &
				XDP_DPCD_STATUS_LANE_2_SL_DONE_MASK)) {
			return XST_FAILURE;
		}
		/* Drop through and check lane 1. */
		/* FALLTHRU */
	case XDP_TX_LANE_COUNT_SET_2:
		if (!(LaneStatus[0] &
				XDP_DPCD_STATUS_LANE_1_SL_DONE_MASK)) {
			return XST_FAILURE;
		}
		/* Drop through and check lane 0. */
		/* FALLTHRU */
	case XDP_TX_LANE_COUNT_SET_1:
		if (!(LaneStatus[0] &
				XDP_DPCD_STATUS_LANE_0_SL_DONE_MASK)) {
			return XST_FAILURE;
		}
		/* FALLTHRU */
	default:
		/* All (LaneCount) lanes have achieved symbol lock. */
		break;
	}

	/* Check that interlane alignment is done. */
	if (!(LaneStatus[2] &
			XDP_DPCD_LANE_ALIGN_STATUS_UPDATED_IA_DONE_MASK)) {
		return XST_FAILURE;
	}

	return XST_SUCCESS;
}

/******************************************************************************/
/**
 * This function sets current voltage swing and pre-emphasis level settings from
 * the LinkConfig structure to hardware.
 *
 * @param	InstancePtr is a pointer to the XDp instance.
 * @param	AuxData is a pointer to the array used for preparing a burst
 *		write over the AUX channel.
 *
 * @return
 *		- XST_SUCCESS if writing the settings was successful.
 *		- XST_FAILURE otherwise.
 *
 * @note	7-series FPGAs registers use the terminology POSTCURSOR(1) to
 *		represent the DisplayPort pre-emphasis levels.
 *
*******************************************************************************/
static void XDp_TxSetVswingPreemp(XDp *InstancePtr, u8 *AuxData)
{
	u8 Data;
	u8 Index;
	u8 VsLevelRx = InstancePtr->TxInstance.LinkConfig.VsLevel;
	u8 PeLevelRx = InstancePtr->TxInstance.LinkConfig.PeLevel;
	u32 VsLevel;
	u32 PeLevel;

	if (InstancePtr->TxInstance.BoardChar.HasRedriverInPath == 0) {
		PeLevel =
			InstancePtr->TxInstance.BoardChar.TxPeLevels[PeLevelRx];
		VsLevel =
			InstancePtr->TxInstance.BoardChar.TxVsLevels[VsLevelRx];

		/* Need to compensate due to no redriver in the path. */
		if (PeLevelRx != 0) {
			VsLevel += InstancePtr->TxInstance.BoardChar.TxVsOffset;
		}
	}
	else {
		/* No need to compensate since redriver does that. Can evenly
		 * disperse the voltage swing and pre-emphasis levels. */

		/* Map 16 possible voltage swing levels in the DisplayPort TX
		 * core to 4 possible in the RX device. */
		VsLevel = VsLevelRx * 4 + 2;
		/* Map 32 possible pre-emphasis levels in the DisplayPort TX
		 * core to 4 possible in the RX device. */
		PeLevel = PeLevelRx * 8 + 4;
	}

	/* Set up the data buffer for writing to the RX device. */
	Data = (PeLevelRx << XDP_DPCD_TRAINING_LANEX_SET_PE_SHIFT) |
								VsLevelRx;
	/* The maximum voltage swing has been reached. */
	if (VsLevelRx == XDP_TX_MAXIMUM_VS_LEVEL) {
		Data |= XDP_DPCD_TRAINING_LANEX_SET_MAX_VS_MASK;
	}
	/* The maximum pre-emphasis level has been reached. */
	if (PeLevelRx == XDP_TX_MAXIMUM_PE_LEVEL) {
		Data |= XDP_DPCD_TRAINING_LANEX_SET_MAX_PE_MASK;
	}
	memset(AuxData, Data, 4);

	for (Index = 0; Index < InstancePtr->TxInstance.LinkConfig.LaneCount;
								Index++) {
		/* Disable pre-cursor levels. */
		XDp_WriteReg(InstancePtr->Config.BaseAddr,
				XDP_TX_PHY_PRECURSOR_LANE_0 + 4 * Index, 0x0);

		/* Write new voltage swing levels to the TX registers. */
		XDp_WriteReg(InstancePtr->Config.BaseAddr,
			XDP_TX_PHY_VOLTAGE_DIFF_LANE_0 + 4 * Index, VsLevel);

		/* Write new pre-emphasis levels to the TX registers. */
		XDp_WriteReg(InstancePtr->Config.BaseAddr,
			XDP_TX_PHY_POSTCURSOR_LANE_0 + 4 * Index, PeLevel);
	}
}

/******************************************************************************/
/**
 * This function sets new voltage swing and pre-emphasis levels using the
 * adjustment requests obtained from the RX device.
 *
 * @param	InstancePtr is a pointer to the XDp instance.
 *
 * @return
 *		- XST_SUCCESS if the new levels were written successfully.
 *		- XST_FAILURE otherwise (an AUX transaction failed).
 *
 * @note	None.
 *
*******************************************************************************/
static u32 XDp_TxAdjVswingPreemp(XDp *InstancePtr)
{
	u32 Status;
	u8 Index;
	u8 VsLevelAdjReq[4];
	u8 PeLevelAdjReq[4];
	u8 AuxData[4];
	u8 *AdjReqs = &InstancePtr->TxInstance.RxConfig.LaneStatusAdjReqs[4];

	/* Analyze the adjustment requests for changes in voltage swing and
	 * pre-emphasis levels. */
	VsLevelAdjReq[0] = AdjReqs[0] & XDP_DPCD_ADJ_REQ_LANE_0_2_VS_MASK;
	VsLevelAdjReq[1] = (AdjReqs[0] & XDP_DPCD_ADJ_REQ_LANE_1_3_VS_MASK) >>
					XDP_DPCD_ADJ_REQ_LANE_1_3_VS_SHIFT;
	VsLevelAdjReq[2] = AdjReqs[1] & XDP_DPCD_ADJ_REQ_LANE_0_2_VS_MASK;
	VsLevelAdjReq[3] = (AdjReqs[1] & XDP_DPCD_ADJ_REQ_LANE_1_3_VS_MASK) >>
					XDP_DPCD_ADJ_REQ_LANE_1_3_VS_SHIFT;
	PeLevelAdjReq[0] = (AdjReqs[0] & XDP_DPCD_ADJ_REQ_LANE_0_2_PE_MASK) >>
					XDP_DPCD_ADJ_REQ_LANE_0_2_PE_SHIFT;
	PeLevelAdjReq[1] = (AdjReqs[0] & XDP_DPCD_ADJ_REQ_LANE_1_3_PE_MASK) >>
					XDP_DPCD_ADJ_REQ_LANE_1_3_PE_SHIFT;
	PeLevelAdjReq[2] = (AdjReqs[1] & XDP_DPCD_ADJ_REQ_LANE_0_2_PE_MASK) >>
					XDP_DPCD_ADJ_REQ_LANE_0_2_PE_SHIFT;
	PeLevelAdjReq[3] = (AdjReqs[1] & XDP_DPCD_ADJ_REQ_LANE_1_3_PE_MASK) >>
					XDP_DPCD_ADJ_REQ_LANE_1_3_PE_SHIFT;

	/* Change the drive settings to match the adjustment requests. Use the
	 * greatest level requested. */
	InstancePtr->TxInstance.LinkConfig.VsLevel = 0;
	InstancePtr->TxInstance.LinkConfig.PeLevel = 0;
	for (Index = 0; Index < InstancePtr->TxInstance.LinkConfig.LaneCount;
								Index++) {
		if (VsLevelAdjReq[Index] >
				InstancePtr->TxInstance.LinkConfig.VsLevel) {
			InstancePtr->TxInstance.LinkConfig.VsLevel =
							VsLevelAdjReq[Index];
		}
		if (PeLevelAdjReq[Index] >
				InstancePtr->TxInstance.LinkConfig.PeLevel) {
			InstancePtr->TxInstance.LinkConfig.PeLevel =
							PeLevelAdjReq[Index];
		}
	}

	/* Verify that the voltage swing and pre-emphasis combination is
	 * allowed. Some combinations will result in a differential peak-to-peak
	 * voltage that is outside the permissible range. See the VESA
	 * DisplayPort v1.2 Specification, section 3.1.5.2.
	 * The valid combinations are:
	 *      PE=0    PE=1    PE=2    PE=3
	 * VS=0 Valid   Valid   Valid   Valid
	 * VS=1 Valid   Valid   Valid
	 * VS=2 Valid   Valid
	 * VS=3 Valid
	 */
	if (InstancePtr->TxInstance.LinkConfig.PeLevel >
			(4 - InstancePtr->TxInstance.LinkConfig.VsLevel)) {
		InstancePtr->TxInstance.LinkConfig.PeLevel =
				4 - InstancePtr->TxInstance.LinkConfig.VsLevel;
	}

	/* Make the adjustments to both the DisplayPort TX core and the RX
	 * device. */
	XDp_TxSetVswingPreemp(InstancePtr, AuxData);
	/* Write the voltage swing and pre-emphasis levels for each lane to the
	 * RX device. */
	Status = XDp_TxAuxWrite(InstancePtr, XDP_DPCD_TRAINING_LANE0_SET,
				4, AuxData);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/* Invoke callback, if defined. */
	if (InstancePtr->TxInstance.PeVsAdjustCallback) {
		InstancePtr->TxInstance.PeVsAdjustCallback(
			InstancePtr->TxInstance.PeVsAdjustCallbackRef);
	}

	return XST_SUCCESS;
}

/******************************************************************************/
/**
 * This function sets the training pattern to be used during link training for
 * both the DisplayPort TX core and the RX device.
 *
 * @param	InstancePtr is a pointer to the XDp instance.
 * @param	Pattern selects the pattern to be used. One of the following:
 *		- XDP_TX_TRAINING_PATTERN_SET_OFF
 *		- XDP_TX_TRAINING_PATTERN_SET_TP1
 *		- XDP_TX_TRAINING_PATTERN_SET_TP2
 *		- XDP_TX_TRAINING_PATTERN_SET_TP3
 *		- XDP_TX_TRAINING_PATTERN_SET_TP4 (in case of DP 1.4)
 *
 * @return
 *		- XST_SUCCESS if setting the pattern was successful.
 *		- XST_FAILURE otherwise.
 *
 * @note	None.
 *
*******************************************************************************/
static u32 XDp_TxSetTrainingPattern(XDp *InstancePtr, u32 Pattern)
{
	u32 Status;
	u8 AuxData[5];

	/* Write to the DisplayPort TX core. */
	XDp_WriteReg(InstancePtr->Config.BaseAddr,
					XDP_TX_TRAINING_PATTERN_SET, Pattern);

	AuxData[0] = Pattern;

	/* Write scrambler disable to the DisplayPort TX core. */
	switch (Pattern) {
	case XDP_TX_TRAINING_PATTERN_SET_OFF:
		XDp_WriteReg(InstancePtr->Config.BaseAddr,
						XDP_TX_SCRAMBLING_DISABLE, 0);
		InstancePtr->TxInstance.LinkConfig.ScramblerEn = 1;
		break;
	case XDP_TX_TRAINING_PATTERN_SET_TP1:
	case XDP_TX_TRAINING_PATTERN_SET_TP2:
	case XDP_TX_TRAINING_PATTERN_SET_TP3:
		AuxData[0] |= XDP_DPCD_TP_SET_SCRAMB_DIS_MASK;
		XDp_WriteReg(InstancePtr->Config.BaseAddr,
						XDP_TX_SCRAMBLING_DISABLE, 1);
		InstancePtr->TxInstance.LinkConfig.ScramblerEn = 0;
		break;
	case XDP_TX_TRAINING_PATTERN_SET_TP4:
		if (InstancePtr->Config.DpProtocol == XDP_PROTOCOL_DP_1_4) {
			XDp_WriteReg(InstancePtr->Config.BaseAddr,
							XDP_TX_SCRAMBLING_DISABLE, 0);
			InstancePtr->TxInstance.LinkConfig.ScramblerEn = 1;
			break;
		}
	default:
		break;
	}

	/* Make the adjustments to both the DisplayPort TX core and the RX
	 * device. */
	XDp_TxSetVswingPreemp(InstancePtr, &AuxData[1]);
	/* Write the voltage swing and pre-emphasis levels for each lane to the
	 * RX device. */
	if  (Pattern == XDP_TX_TRAINING_PATTERN_SET_OFF) {
		Status = XDp_TxAuxWrite(InstancePtr, XDP_DPCD_TP_SET, 1,
								AuxData);
	}
	else {
		Status = XDp_TxAuxWrite(InstancePtr, XDP_DPCD_TP_SET, 5,
								AuxData);
	}
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	return XST_SUCCESS;
}

/******************************************************************************/
/**
 * This function determines what the RX device's required training delay is for
 * link training.
 *
 * @param	InstancePtr is a pointer to the XDp instance.
 * @param	TrainingState is the current training state; either clock
 *		recovery or channel equalization.
 *
 * @return	The training delay specified in the RX device's DisplayPort
 *		Configuration Data (DPCD) register,
 *		XDP_DPCD_TRAIN_AUX_RD_INTERVAL.
 *
 * @note	None.
 *
*******************************************************************************/
static u32 XDp_TxGetTrainingDelay(XDp *InstancePtr,
					XDp_TxTrainingState TrainingState)
{
	u8 *Dpcd = InstancePtr->TxInstance.RxConfig.DpcdRxCapsField;
	u16 Delay;

	switch ((Dpcd[XDP_DPCD_TRAIN_AUX_RD_INTERVAL] &
			 XDP_DPCD_TRAIN_AUX_RD_INT_MASK)) {
	case XDP_DPCD_TRAIN_AUX_RD_INT_100_400US:
		if (TrainingState == XDP_TX_TS_CLOCK_RECOVERY) {
			/* Delay for the clock recovery phase. */
			Delay = 100;
		}
		else {
			/* Delay for the channel equalization phase. */
			Delay = 400;
		}
		break;
	case XDP_DPCD_TRAIN_AUX_RD_INT_4MS:
		Delay = 4000;
		break;
	case XDP_DPCD_TRAIN_AUX_RD_INT_8MS:
		Delay = 8000;
		break;
	case XDP_DPCD_TRAIN_AUX_RD_INT_12MS:
		Delay = 12000;
		break;
	case XDP_DPCD_TRAIN_AUX_RD_INT_16MS:
		Delay = 16000;
		break;
	default:
		/* Default to 20 ms. */
		Delay = 20000;
		break;
	}

	return Delay;
}

/******************************************************************************/
/**
 * This function contains the common sequence of submitting an AUX command for
 * AUX read, AUX write, I2C-over-AUX read, and I2C-over-AUX write transactions.
 * If required, the reads and writes are split into multiple requests, each
 * acting on a maximum of 16 bytes.
 *
 * @param	InstancePtr is a pointer to the XDp instance.
 * @param	CmdType is the type of AUX command (one of: XDP_TX_AUX_CMD_READ,
 *		XDP_TX_AUX_CMD_WRITE, XDP_TX_AUX_CMD_I2C_READ, or
 *		XDP_TX_AUX_CMD_I2C_WRITE.
 * @param	Address is the starting address that the AUX transaction will
 *		read/write from/to the RX device.
 * @param	NumBytes is the number of bytes to read/write from/to the RX
 *		device.
 * @param	Data is a pointer to the data buffer that contains the data
 *		to be read/written from/to the RX device.
 *
 * @return
 *		- XST_SUCCESS if the AUX transaction request was acknowledged.
 *		- XST_ERROR_COUNT_MAX if the AUX request timed out.
 *		- XST_FAILURE otherwise (if the DisplayPort TX core sees a NACK
 *		  reply code or if the AUX transaction failed).
 *
 * @note	None.
 *
*******************************************************************************/
static u32 XDp_TxAuxCommon(XDp *InstancePtr, u32 CmdType, u32 Address,
							u32 NumBytes, u8 *Data)
{
	u32 Status;
	XDp_AuxTransaction Request;
	u32 BytesLeft;

	/* Set the start address for AUX transactions. For I2C transactions,
	 * this is the address of the I2C bus. */
	Request.Address = Address;

	BytesLeft = NumBytes;
	while (BytesLeft > 0) {
		Request.CmdCode = CmdType;

		if ((CmdType == XDP_TX_AUX_CMD_READ) ||
					(CmdType == XDP_TX_AUX_CMD_WRITE)) {
			/* Increment address for normal AUX transactions. */
			Request.Address = Address + (NumBytes - BytesLeft);
		}

		/* Increment the pointer to the supplied data buffer. */
		Request.Data = &Data[NumBytes - BytesLeft];

		if (BytesLeft > 16) {
			Request.NumBytes = 16;
		}
		else {
			Request.NumBytes = BytesLeft;
		}
		BytesLeft -= Request.NumBytes;

		if ((CmdType == XDP_TX_AUX_CMD_I2C_READ) && (BytesLeft > 0)) {
			/* Middle of a transaction I2C read request. Override
			 * the command code that was set to CmdType. */
			Request.CmdCode = XDP_TX_AUX_CMD_I2C_READ_MOT;
		}
		else if ((CmdType == XDP_TX_AUX_CMD_I2C_WRITE) &&
							(BytesLeft > 0)) {
			/* Middle of a transaction I2C write request. Override
			 * the command code that was set to CmdType. */
			Request.CmdCode = XDP_TX_AUX_CMD_I2C_WRITE_MOT;
		}

		XDp_WaitUs(InstancePtr, InstancePtr->TxInstance.AuxDelayUs);

		Status = XDp_TxAuxRequest(InstancePtr, &Request);
		if (Status != XST_SUCCESS) {
			return Status;
		}
	}

	return XST_SUCCESS;
}

/******************************************************************************/
/**
 * This function submits the supplied AUX request to the RX device over the AUX
 * channel. If waiting for a reply times out, or if the DisplayPort TX core
 * indicates that the request was deferred, the request is sent again (up to a
 * maximum specified by XDP_AUX_MAX_DEFER_COUNT|XDP_AUX_MAX_TIMEOUT_COUNT).
 *
 * @param	InstancePtr is a pointer to the XDp instance.
 * @param	Request is a pointer to an initialized XDp_AuxTransaction
 *		structure containing the required information for issuing an
 *		AUX command, as well as a write buffer used for write commands,
 *		and a read buffer for read commands.
 *
 * @return
 *		- XST_SUCCESS if the request was acknowledged.
 *		- XST_ERROR_COUNT_MAX if resending the request exceeded the
 *		  maximum for deferral and timeout.
 *		- XST_FAILURE otherwise (if the DisplayPort TX core sees a NACK
 *		  reply code or if the AUX transaction failed).
 *
 * @note	None.
 *
*******************************************************************************/
static u32 XDp_TxAuxRequest(XDp *InstancePtr, XDp_AuxTransaction *Request)
{
	u32 Status;
	u32 DeferCount = 0;
	u32 TimeoutCount = 0;

	while ((DeferCount < XDP_AUX_MAX_DEFER_COUNT) &&
				(TimeoutCount < XDP_AUX_MAX_TIMEOUT_COUNT)) {
		Status = XDp_TxAuxWaitReady(InstancePtr);
		if (Status != XST_SUCCESS) {
			/* The RX device isn't ready yet. */
			TimeoutCount++;
			continue;
		}

		/* Send the request. */
		Status = XDp_TxAuxRequestSend(InstancePtr, Request);
		if (Status == XST_SEND_ERROR) {
			/* The request was deferred. */
			DeferCount++;
		}
		else if (Status == XST_ERROR_COUNT_MAX) {
			/* Waiting for a reply timed out. */
			TimeoutCount++;
		}
		else {
			/* XST_FAILURE indicates that the request was NACK'ed,
			 * XST_SUCCESS indicates that the request was ACK'ed. */
			return Status;
		}
		/* Aux request waiting period as per the latest CTS */
		XDp_WaitUs(InstancePtr, 3200);
	}

	/* The request was not successfully received by the RX device. */
	return XST_ERROR_COUNT_MAX;
}

/******************************************************************************/
/**
 * This function submits the supplied AUX request to the RX device over the AUX
 * channel by writing the command, the destination address, (the write buffer
 * for write commands), and the data size to the DisplayPort TX core.
 *
 * @param	InstancePtr is a pointer to the XDp instance.
 * @param	Request is a pointer to an initialized XDp_AuxTransaction
 *		structure containing the required information for issuing an AUX
 *		command.
 *
 * @return
 *		- XST_SUCCESS if the request was acknowledged.
 *		- XST_ERROR_COUNT_MAX if waiting for a reply timed out.
 *		- XST_SEND_ERROR if the request was deferred.
 *		- XST_FAILURE otherwise, if the request was NACK'ed.
 *
 * @note	None.
 *
*******************************************************************************/
static u32 XDp_TxAuxRequestSend(XDp *InstancePtr, XDp_AuxTransaction *Request)
{
	u32 TimeoutCount;
	u32 Status;
	u8 Index;

	/* Ensure that any pending AUX transactions have completed. */
	TimeoutCount = 0;
	do {
		Status = XDp_ReadReg(InstancePtr->Config.BaseAddr,
							XDP_TX_REPLY_STATUS);

		XDp_WaitUs(InstancePtr, 20);
		TimeoutCount++;
		if (TimeoutCount >= XDP_AUX_MAX_TIMEOUT_COUNT) {
			return XST_ERROR_COUNT_MAX;
		}
	} while ((Status & XDP_TX_REPLY_STATUS_REQUEST_IN_PROGRESS_MASK) ||
			(Status & XDP_TX_REPLY_STATUS_REPLY_IN_PROGRESS_MASK));

	/* Set the address for the request. */
	XDp_WriteReg(InstancePtr->Config.BaseAddr, XDP_TX_AUX_ADDRESS,
							Request->Address);

	if ((Request->CmdCode == XDP_TX_AUX_CMD_WRITE) ||
			(Request->CmdCode == XDP_TX_AUX_CMD_I2C_WRITE) ||
			(Request->CmdCode == XDP_TX_AUX_CMD_I2C_WRITE_MOT)) {
		/* Feed write data into the DisplayPort TX core's write FIFO. */
		for (Index = 0; Index < Request->NumBytes; Index++) {
			XDp_WriteReg(InstancePtr->Config.BaseAddr,
				XDP_TX_AUX_WRITE_FIFO, Request->Data[Index]);
		}
	}

	/* Submit the command and the data size. */
	XDp_WriteReg(InstancePtr->Config.BaseAddr, XDP_TX_AUX_CMD,
				((Request->CmdCode << XDP_TX_AUX_CMD_SHIFT) |
				((Request->NumBytes - 1) &
				XDP_TX_AUX_CMD_NBYTES_TRANSFER_MASK)));

	/* Check for a reply from the RX device to the submitted request. */
	Status = XDp_TxAuxWaitReply(InstancePtr);
	if (Status != XST_SUCCESS) {
		/* Waiting for a reply timed out. */
		return XST_ERROR_COUNT_MAX;
	}

	/* Analyze the reply. */
	Status = XDp_ReadReg(InstancePtr->Config.BaseAddr,
							XDP_TX_AUX_REPLY_CODE);
	if ((Status == XDP_TX_AUX_REPLY_CODE_DEFER) ||
				(Status == XDP_TX_AUX_REPLY_CODE_I2C_DEFER)) {
		/* The request was deferred. */
		return XST_SEND_ERROR;
	}
	else if ((Status == XDP_TX_AUX_REPLY_CODE_NACK) ||
				(Status == XDP_TX_AUX_REPLY_CODE_I2C_NACK)) {
		/* The request was not acknowledged. */
		return XST_FAILURE;
	}

	/* The request was acknowledged. */

	if ((Request->CmdCode == XDP_TX_AUX_CMD_READ) ||
			(Request->CmdCode == XDP_TX_AUX_CMD_I2C_READ) ||
			(Request->CmdCode == XDP_TX_AUX_CMD_I2C_READ_MOT)) {

		/* Wait until all data has been received. */
		TimeoutCount = 0;
		do {
			Status = XDp_ReadReg(InstancePtr->Config.BaseAddr,
						XDP_TX_REPLY_DATA_COUNT);

			XDp_WaitUs(InstancePtr, 100);
			TimeoutCount++;
			if (TimeoutCount >= XDP_AUX_MAX_TIMEOUT_COUNT) {
				return XST_ERROR_COUNT_MAX;
			}
		} while (Status != Request->NumBytes);

		/* Obtain the read data from the reply FIFO. */
		for (Index = 0; Index < Request->NumBytes; Index++) {
			Request->Data[Index] = XDp_ReadReg(
						InstancePtr->Config.BaseAddr,
						XDP_TX_AUX_REPLY_DATA);
		}
	}

	return XST_SUCCESS;
}

/******************************************************************************/
/**
 * This function waits for a reply indicating that the most recent AUX request
 * has been received by the RX device.
 *
 * @param	InstancePtr is a pointer to the XDp instance.
 *
 * @return
 *		- XST_SUCCESS if a reply was sent from the RX device.
 *		- XST_ERROR_COUNT_MAX otherwise, if a timeout has occurred.
 *
 * @note	None.
 *
*******************************************************************************/
static u32 XDp_TxAuxWaitReply(XDp *InstancePtr)
{
	u32 Timeout = 100;
	u32 Status;

	while (0 < Timeout) {
		Status = XDp_ReadReg(InstancePtr->Config.BaseAddr,
							XDP_TX_REPLY_STATUS);

		/* Check for error. */
		if (Status & XDP_TX_REPLY_STATUS_REPLY_ERROR_MASK) {
			return XST_ERROR_COUNT_MAX;
		}

		/* Check for a reply. */
		if ((Status & XDP_TX_REPLY_STATUS_REPLY_RECEIVED_MASK) &&
				!(Status &
				XDP_TX_REPLY_STATUS_REQUEST_IN_PROGRESS_MASK) &&
				!(Status &
				XDP_TX_REPLY_STATUS_REPLY_IN_PROGRESS_MASK)) {
			return XST_SUCCESS;
		}

		Timeout--;
		XDp_WaitUs(InstancePtr, 20);
	}

	return XST_ERROR_COUNT_MAX;
}

/******************************************************************************/
/**
 * This function waits until another request is no longer in progress.
 *
 * @param	InstancePtr is a pointer to the XDp instance.
 *
 * @return
 *		- XST_SUCCESS if the the RX device is no longer busy.
 *		- XST_ERROR_COUNT_MAX otherwise, if a timeout has occurred.
 *
 * @note	None.
 *
*******************************************************************************/
static u32 XDp_TxAuxWaitReady(XDp *InstancePtr)
{
	u32 Status;
	u32 Timeout = 100;

	/* Wait until the DisplayPort TX core is ready. */
	do {
		Status = XDp_ReadReg(InstancePtr->Config.BaseAddr,
						XDP_TX_INTERRUPT_SIG_STATE);

		/* Protect against an infinite loop. */
		if (!Timeout--) {
			return XST_ERROR_COUNT_MAX;
		}
		XDp_WaitUs(InstancePtr, 20);
	}
	while (Status & XDP_TX_REPLY_STATUS_REPLY_IN_PROGRESS_MASK);

	return XST_SUCCESS;
}

/******************************************************************************/
/**
 * This function sets the clock frequency for the DisplayPort PHY corresponding
 * to a desired data rate.
 *
 * @param	InstancePtr is a pointer to the XDp instance.
 * @param	Speed determines what clock frequency will be used based on one
 *		of the following selects:
 *		- XDP_TX_PHY_CLOCK_SELECT_162GBPS = 0x01
 *		- XDP_TX_PHY_CLOCK_SELECT_270GBPS = 0x03
 *		- XDP_TX_PHY_CLOCK_SELECT_540GBPS = 0x05
 *
 * @return
 *		- XST_SUCCESS if the reset for each lane is done after the clock
 *		  frequency has been set.
 *		- XST_FAILURE otherwise.
 *
 * @note	None.
 *
*******************************************************************************/
static u32 XDp_TxSetClkSpeed(XDp *InstancePtr, u32 Speed)
{
	u32 Status;
	u32 RegVal;

	/* Disable the DisplayPort TX core first. */
	RegVal = XDp_ReadReg(InstancePtr->Config.BaseAddr, XDP_TX_ENABLE);
	XDp_WriteReg(InstancePtr->Config.BaseAddr, XDP_TX_ENABLE, 0x0);

	/* Change speed of the feedback clock. */
	XDp_WriteReg(InstancePtr->Config.BaseAddr,
						XDP_TX_PHY_CLOCK_SELECT, Speed);

	/* Re-enable the DisplayPort TX core if it was previously enabled. */
	if (RegVal != 0x0) {
		XDp_WriteReg(InstancePtr->Config.BaseAddr, XDP_TX_ENABLE, 0x1);
	}

	/* Wait until the PHY is ready. */
	Status = XDp_WaitPhyReady(InstancePtr,
					XDP_TX_PHY_STATUS_LANES_READY_MASK(
					InstancePtr->Config.MaxLaneCount));
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	return XST_SUCCESS;
}

/******************************************************************************/
/**
 * This function enables MST-TX audio for a given stream on the main link.
 *
 * @param      InstancePtr is a pointer to the XDp instance.
 * @param      Stream id
 *
 * @return      None.
 *
 * @note        None.
 *
 **********************************************************************************/
void XDp_Tx_Mst_AudioEn(XDp *InstancePtr, u8 StreamId)
{

        u32 ReadVal;

        /* Verify arguments. */
        Xil_AssertVoid(InstancePtr != NULL);
        Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
        Xil_AssertVoid(XDp_GetCoreType(InstancePtr) == XDP_TX);

        Xil_AssertVoid((StreamId == XDP_TX_STREAM_ID1) ||
                        (StreamId == XDP_TX_STREAM_ID2) ||
                        (StreamId == XDP_TX_STREAM_ID3) ||
                        (StreamId == XDP_TX_STREAM_ID4));

        ReadVal = (StreamId - 1) <<  XDP_TX_AUDIO_CONTROL_LANEX_SET_SHIFT ;
        XDp_WriteReg(InstancePtr->Config.BaseAddr, XDP_TX_AUDIO_CONTROL,
                       ReadVal);

        XDp_WriteReg(InstancePtr->Config.BaseAddr, XDP_TX_AUDIO_CONTROL,
                        (ReadVal | 0x1));
}

/******************************************************************************/
/**
 * This function disables MST-TX audio stream packets on the main link.
 *
 * @param       InstancePtr is a pointer to the XDp instance.
 *
 * @return      None.
 *
 * @note        None.
 *
 *******************************************************************************/
void XDp_TxAudioDis(XDp *InstancePtr)
{
        /* Verify arguments. */
        Xil_AssertVoid(InstancePtr != NULL);
        Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
        Xil_AssertVoid(XDp_GetCoreType(InstancePtr) == XDP_TX);

        XDp_WriteReg(InstancePtr->Config.BaseAddr, XDP_TX_AUDIO_CONTROL, 0x0);
}

/****************************************************************************/
/**
 * This function sends audio InfoFrame packets on the main link.
 *
 * @param       InstancePtr is a pointer to the XDp instance.
 * @param	xilInfoFrame is a pointer to the InfoFrame buffer.
 *
 * @return      None.
 *
 * @note        None.
 *
 ****************************************************************************/
void XDp_TxSendAudioInfoFrame(XDp *InstancePtr,
		XDp_TxAudioInfoFrame *xilInfoFrame)
{
	u8 db1, db2, db3, db4;
	u32 data;

	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertVoid(XDp_GetCoreType(InstancePtr) == XDP_TX);

	/* Write first 4 bytes (0 to 3)*/
	/* second packet ID fixed to 0 - SST Mode */
	db1 = 0x00;
	db2 = xilInfoFrame->type;
	db3 = xilInfoFrame->info_length & 0xFF;
	db4 = (xilInfoFrame->version << 2) | (xilInfoFrame->info_length >> 8);
	data = (db4 << 24) | (db3 << 16) | (db2 << 8) | db1;
	XDp_WriteReg(InstancePtr->Config.BaseAddr,
			XDP_TX_AUDIO_INFO_DATA(1), data);

	/* Write next 4 bytes (4 to 7)*/
	db1 = xilInfoFrame->audio_channel_count
		| (xilInfoFrame->audio_coding_type << 4);
	db2 = (xilInfoFrame->sampling_frequency << 2)
		| xilInfoFrame->sample_size;
	db3 = 0;
	db4 = xilInfoFrame->channel_allocation;
	data = (db4 << 24) | (db3 << 16) | (db2 << 8) | db1;
	XDp_WriteReg(InstancePtr->Config.BaseAddr,
			XDP_TX_AUDIO_INFO_DATA(1), data);

	/* Write next 4 bytes (8 to 11)*/
	data = (xilInfoFrame->level_shift << 3)
			| (xilInfoFrame->downmix_inhibit << 7);
	XDp_WriteReg(InstancePtr->Config.BaseAddr,
			XDP_TX_AUDIO_INFO_DATA(1), data);

	/* Write next 20 bytes (12 to 31)*/
	data = 0;
	for (db1 = 4; db1 <= 8; db1++) {
		XDp_WriteReg(InstancePtr->Config.BaseAddr,
				XDP_TX_AUDIO_INFO_DATA(1), data);
	}
}

/******************************************************************************/
/**
 * This function will check if the immediate downstream RX device capable
 * of receiving colorimetry information through VSC extended SDP packet.
 *
 * A DisplayPort Configuration Data (DPCD) version of 1.4 is required
 * VSC_SDP_EXTENSION_FOR_COLORIMETRY_SUPPORTED bit in the DPCD
 * DPRX_FEATURE_ENUMERATION_LIST register must be set for this function
 * to return XST_SUCCESS.
 *
 * @param	InstancePtr is a pointer to the XDp instance.
 *
 * @return
 *		- XST_SUCCESS if the RX device is capable of VSC extended packet.
 *		- XST_NO_FEATURE if the RX device does not capable of
 *		  VSC extended packet.
 *
 * @note	None.
 *
*******************************************************************************/
u32 XDp_TxCheckVscColorimetrySupport(XDp *InstancePtr)
{
	u32 Status;
	u8 Data;

	/* Verify arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertNonvoid(XDp_GetCoreType(InstancePtr) == XDP_TX);


	/* Check if the RX device has VSC EXT capabilities.. */
	Status = XDp_TxAuxRead(InstancePtr, XDP_DPCD_FEATURE_ENUMERATION_LIST,
				1, &Data);
	if (Status != XST_SUCCESS) {
		/* The AUX read transaction failed. */
		return Status;
	} else if ((Data & VSC_SDP_EXTENSION_FOR_COLORIMETRY_SUPPORTED) !=
			VSC_SDP_EXTENSION_FOR_COLORIMETRY_SUPPORTED) {
		return XST_NO_FEATURE;
	}

	return XST_SUCCESS;
}

#endif /* XPAR_XDPTXSS_NUM_INSTANCES */

/******************************************************************************/
/**
 * This function waits for the DisplayPort PHY to come out of reset.
 *
 * @param	InstancePtr is a pointer to the XDp instance.
 *
 * @return
 *		- XST_ERROR_COUNT_MAX if the PHY failed to be ready.
 *		- XST_SUCCESS otherwise.
 *
 * @note	None.
 *
*******************************************************************************/
static u32 XDp_WaitPhyReady(XDp *InstancePtr, u32 Mask)
{
	u16 Timeout = 20000;
	u32 PhyStatus;
	u32 RegPhyStatus;

#if XPAR_XDPTXSS_NUM_INSTANCES
	if (XDp_GetCoreType(InstancePtr) == XDP_TX) {
		RegPhyStatus = XDP_TX_PHY_STATUS;
	}
#endif
#if XPAR_XDPRXSS_NUM_INSTANCES
	if (XDp_GetCoreType(InstancePtr) == XDP_RX) {
		RegPhyStatus = XDP_RX_PHY_STATUS;
	}
#endif

	/* Wait until the PHY is ready. */
	do {
		PhyStatus = XDp_ReadReg(InstancePtr->Config.BaseAddr,
						RegPhyStatus) & Mask;

		/* Protect against an infinite loop. */
		if (!Timeout--) {
			return XST_ERROR_COUNT_MAX;
		}
		XDp_WaitUs(InstancePtr, 20);
	}
	while (PhyStatus != Mask);

	return XST_SUCCESS;
}

#if XPAR_XDPRXSS_NUM_INSTANCES
/******************************************************************************/
/**
 * This function raises the CP_IRQ interrupt to the Upstream device.
 *
 * @param	InstancePtr is a pointer to the XDp instance.
 *
 * @return	None
 *
 * @note	None.
 *
*******************************************************************************/
void XDp_GenerateCpIrq(XDp *InstancePtr) {

	XDp_WriteReg(InstancePtr->Config.BaseAddr,
			XDP_RX_HPD_INTERRUPT, 0x00);
	XDp_WriteReg(InstancePtr->Config.BaseAddr,
			XDP_RX_DEVICE_SERVICE_IRQ,
			XDP_RX_DEVICE_SERVICE_IRQ_CP_IRQ_MASK);
}

/******************************************************************************/
/**
 * This function is to enable or disable giving AUX_DEFFERs for
 * HDCP22 DPCD offsets.
 *
 * @param	InstancePtr is a pointer to the XDp instance.
 *
 * @retun	None
 *
 * @note	This function will enable or disable AUX_DEFFERS
 * 			for below DPCD offsets
 * 			0x6900B to 0x6921F
 * 			0x692C0 to 0x692D0
 * 			0x692E0 to 0x692EF.
 *
*******************************************************************************/
void XDp_EnableDisableHdcp22AuxDeffers(XDp *InstancePtr, u8 EnableDisable)
{
	u32 Regval;

	/* programming AUX defer*/
	Regval = XDp_ReadReg(InstancePtr->Config.BaseAddr,
			XDP_RX_AUX_CLK_DIVIDER);
	if (EnableDisable)
		Regval |= (1 << XDP_RX_AUX_DEFER_SHIFT);
	else
		Regval &= ~(1 << XDP_RX_AUX_DEFER_SHIFT);

	XDp_WriteReg(InstancePtr->Config.BaseAddr,
			XDP_RX_AUX_CLK_DIVIDER, Regval);
}
#endif
#if XPAR_XDPTXSS_NUM_INSTANCES
/******************************************************************************/
/**
 * This function Enables Dp Tx video path routes through HDCP22 core.
 *
 * @param	InstancePtr is a pointer to the XDp instance.
 *
 * @return	None
 *
 * @note	None.
 *
 ******************************************************************************/
void XDp_TxHdcp22Enable(XDp *InstancePtr)
{
	XDp_WriteReg(InstancePtr->Config.BaseAddr,
			XDP_TX_HDCP22_ENABLE,
			XDP_TX_HDCP22_ENABLE_BYPASS_DISABLE_MASK);
}

/******************************************************************************/
/**
 * This function Disables Dp Tx video path through HDCP22 core.
 *
 * @param	InstancePtr is a pointer to the XDp instance.
 *
 * @return	None
 *
 * @note	None.
 *
 *******************************************************************************/
void XDp_TxHdcp22Disable(XDp *InstancePtr)
{
	XDp_WriteReg(InstancePtr->Config.BaseAddr,
			XDP_TX_HDCP22_ENABLE, 0);
}
#endif
/** @} */
