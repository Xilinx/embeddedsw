/*******************************************************************************
 *
 * Copyright (C) 2014 Xilinx, Inc.  All rights reserved.
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
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * XILINX CONSORTIUM BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
 * OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 * Except as contained in this notice, the name of the Xilinx shall not be used
 * in advertising or otherwise to promote the sale, use or other dealings in
 * this Software without prior written authorization from Xilinx.
 *
*******************************************************************************/
/******************************************************************************/
/**
 *
 * @file xdptx.c
 *
 * Contains a minimal wset of functions for the XDptx driver that allow access
 * to all the DisplayPort transmitter's functionality. See xdptx.h for a
 * detailed description of the driver.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- -----------------------------------------------
 * 1.00a als  05/17/14 Initial release.
 * </pre>
 *
*******************************************************************************/

/******************************* Include Files ********************************/

#include "xdptx.h"
#include "xstatus.h"
#if defined(__MICROBLAZE__)
#include "microblaze_sleep.h"
#elif defined(__arm__)
#include "sleep.h"
#endif
#include "xenv.h"

/**************************** Constant Definitions ****************************/

#define XDPTX_MAXIMUM_VS_LEVEL 3
#define XDPTX_MAXIMUM_PE_LEVEL 3

#define XDPTX_AUX_MAX_DEFER_COUNT 50
#define XDPTX_AUX_MAX_TIMEOUT_COUNT 50

/****************************** Type Definitions ******************************/

/**
 * This typedef enumerates the list of training states used in the state machine
 * during the link training process.
 */
typedef enum {
        XDPTX_TS_CLOCK_RECOVERY,
        XDPTX_TS_CHANNEL_EQUALIZATION,
        XDPTX_TS_ADJUST_LINK_RATE,
        XDPTX_TS_ADJUST_LANE_COUNT,
        XDPTX_TS_FAILURE,
        XDPTX_TS_SUCCESS
} XDptx_TrainingState;

/**
 * This typedef describes an AUX transaction.
 */
typedef struct {
        u16 CmdCode;
        u8 NumBytes;
        u32 Address;
        u8 *Data;
} XDptx_AuxTransaction;

/**************************** Function Prototypes *****************************/

/* Training functions. */
static u32 XDptx_RunTraining(XDptx *InstancePtr);
static XDptx_TrainingState XDptx_TrainingStateClockRecovery(XDptx *InstancePtr);
static XDptx_TrainingState XDptx_TrainingStateChannelEqualization(
                                        XDptx *InstancePtr, u32 MaxIterations);
static XDptx_TrainingState XDptx_TrainingStateAdjustLinkRate(
                                                        XDptx *InstancePtr);
static XDptx_TrainingState XDptx_TrainingStateAdjustLaneCount(
                                                        XDptx *InstancePtr);
static u32 XDptx_GetLaneStatusAdjReqs(XDptx *InstancePtr);
static u32 XDptx_CheckClockRecovery(XDptx *InstancePtr, u8 LaneCount);
static u32 XDptx_CheckChannelEqualization(XDptx *InstancePtr, u8 LaneCount);
static u32 XDptx_SetVswingPreemp(XDptx *InstancePtr);
static u32 XDptx_AdjVswingPreemp(XDptx *InstancePtr);
static u32 XDptx_SetTrainingPattern(XDptx *InstancePtr, u32 Pattern);
static u32 XDptx_GetTrainingDelay(XDptx *InstancePtr,
                                        XDptx_TrainingState TrainingState);
/* AUX transaction functions. */
static u32 XDptx_AuxCommon(XDptx *InstancePtr, u32 CmdType, u32 Address,
                                                        u32 NumBytes, u8 *Data);
static u32 XDptx_AuxRequest(XDptx *InstancePtr, XDptx_AuxTransaction *Request);
static u32 XDptx_AuxRequestSend(XDptx *InstancePtr,
                                                XDptx_AuxTransaction *Request);
static u32 XDptx_AuxWaitReply(XDptx *InstancePtr);
static u32 XDptx_AuxWaitReady(XDptx *InstancePtr);
/* Miscellaneous functions. */
static u32 XDptx_SetClkSpeed(XDptx *InstancePtr, u32 Speed);
static u32 XDptx_WaitPhyReady(XDptx *InstancePtr);

/**************************** Function Definitions ****************************/

/******************************************************************************/
/**
 * This function prepares the DisplayPort TX core for use.
 *
 * @param       InstancePtr is a pointer to the XDptx instance.
 *
 * @return
 *              - XST_SUCCESS if the DisplayPort TX core was successfully
 *                initialized.
 *              - XST_INVALID_PARAM if the supplied link rate does not
 *                correspond to either 1.62, 2.70, or 5.40 Gbps.
 *              - XST_FAILURE otherwise.
 *
*******************************************************************************/
u32 XDptx_InitializeTx(XDptx *InstancePtr)
{
        u32 Status;
        u32 RegVal;
        XDptx_Config *TxConfig = &InstancePtr->TxConfig;

        /* Verify arguments. */
        Xil_AssertNonvoid(InstancePtr != NULL);

        /* Place the PHY (and GTTXRESET) into reset. */
        XDptx_WriteReg(TxConfig->BaseAddr, XDPTX_PHY_CONFIG,
                                        XDPTX_PHY_CONFIG_GT_ALL_RESET_MASK);

        /* Disable the transmitter. */
        XDptx_WriteReg(TxConfig->BaseAddr, XDPTX_ENABLE, 0);

        /* Set the clock divider. */
        RegVal = (XDptx_ReadReg(TxConfig->BaseAddr, XDPTX_AUX_CLK_DIVIDER) &
                                        ~XDPTX_AUX_CLK_DIVIDER_VAL_MASK) |
                                        (TxConfig->SAxiClkHz / 1000000);
        XDptx_WriteReg(TxConfig->BaseAddr, XDPTX_AUX_CLK_DIVIDER, RegVal);

        /* Set the transmitter's clock speed. */
        switch (TxConfig->MaxLinkRate) {
        case XDPTX_LINK_BW_SET_540GBPS:
                XDptx_WriteReg(TxConfig->BaseAddr, XDPTX_PHY_CLOCK_SELECT,
                                                XDPTX_PHY_CLOCK_SELECT_540GBPS);
                break;
        case XDPTX_LINK_BW_SET_270GBPS:
                XDptx_WriteReg(TxConfig->BaseAddr, XDPTX_PHY_CLOCK_SELECT,
                                                XDPTX_PHY_CLOCK_SELECT_270GBPS);
                break;
        case XDPTX_LINK_BW_SET_162GBPS:
                XDptx_WriteReg(TxConfig->BaseAddr, XDPTX_PHY_CLOCK_SELECT,
                                                XDPTX_PHY_CLOCK_SELECT_162GBPS);
                break;
        default:
                return XST_INVALID_PARAM;
        }

        /* Bring the PHY (and GTTXRESET) out of reset. */
        XDptx_WriteReg(TxConfig->BaseAddr, XDPTX_PHY_CONFIG,
                                        XDPTX_PHY_CONFIG_PHY_RESET_ENABLE_MASK);

        /* Wait for the PHY to be ready. */
        Status = XDptx_WaitPhyReady(InstancePtr);
        if (Status != XST_SUCCESS) {
                return XST_FAILURE;
        }

        /* Enable the transmitter. */
        XDptx_WriteReg(TxConfig->BaseAddr, XDPTX_ENABLE, 1);

        /* Unmask hot-plug-detect (HPD) interrupts. */
        XDptx_WriteReg(TxConfig->BaseAddr, XDPTX_INTERRUPT_MASK,
                                ~XDPTX_INTERRUPT_MASK_HPD_PULSE_DETECTED_MASK &
                                ~XDPTX_INTERRUPT_MASK_HPD_EVENT_MASK &
                                ~XDPTX_INTERRUPT_MASK_HPD_IRQ_MASK);

        return XST_SUCCESS;
}

/******************************************************************************/
/**
 * This function retrieves the configuration for this DisplayPort TX instance
 * and fills in the InstancePtr->TxConfig structure.
 *
 * @param       InstancePtr is a pointer to the XDptx instance.
 * @param       ConfigPtr is a pointer to the configuration structure that will
 *              be used to copy the settings from.
 * @param       EffectiveAddr is the device base address in the virtual memory
 *              space. If the address translation is not used, then the physical
 *              address is passed.
 *
 * @note        Unexpected errors may occur if the address mapping is changed
 *              after this function is invoked.
 *
*******************************************************************************/
void XDptx_CfgInitialize(XDptx *InstancePtr, XDptx_Config *ConfigPtr,
                                                        u32 EffectiveAddr)
{
        /* Verify arguments. */
        Xil_AssertVoid(InstancePtr != NULL);
        Xil_AssertVoid(ConfigPtr != NULL);
        Xil_AssertVoid(EffectiveAddr != 0x0);

        InstancePtr->IsReady = 0;

        InstancePtr->TxConfig.DeviceId = ConfigPtr->DeviceId;
        InstancePtr->TxConfig.BaseAddr = EffectiveAddr;
        InstancePtr->TxConfig.SAxiClkHz = ConfigPtr->SAxiClkHz;

        InstancePtr->TxConfig.MaxLinkRate = ConfigPtr->MaxLinkRate;
        InstancePtr->TxConfig.MaxLaneCount = ConfigPtr->MaxLaneCount;

        InstancePtr->IsReady = XIL_COMPONENT_IS_READY;
}

/******************************************************************************/
/**
 * This function retrieves sink device capabilities from the receiver's DPCD.
 *
 * @param       InstancePtr is a pointer to the XDptx instance.
 *
 * @return      
 *              - XST_SUCCESS if the DPCD was read successfully.
 *              - XST_DEVICE_NOT_FOUND if no receiver is connected.
 *              - XST_FAILURE otherwise.
 *
*******************************************************************************/
u32 XDptx_GetSinkCapabilities(XDptx *InstancePtr)
{
        u32 Status;
        u8 *Dpcd = InstancePtr->RxConfig.DpcdRxCapsField;
        XDptx_LinkConfig *LinkConfig = &InstancePtr->LinkConfig;
        XDptx_Config *TxConfig = &InstancePtr->TxConfig;
        u8 RxMaxLinkRate;
        u8 RxMaxLaneCount;

        /* Verify arguments. */
        Xil_AssertNonvoid(InstancePtr != NULL);
        Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

        if (!XDptx_IsConnected(InstancePtr)) {
                return XST_DEVICE_NOT_FOUND;
        }

        Status = XDptx_AuxRead(InstancePtr, XDPTX_DPCD_RECEIVER_CAP_FIELD_START,
                                XDPTX_DPCD_RECEIVER_CAP_FIELD_SIZE, Dpcd);
        if (Status != XST_SUCCESS) {
                return XST_FAILURE;
        }

        RxMaxLinkRate = Dpcd[XDPTX_DPCD_MAX_LINK_RATE];
        RxMaxLaneCount = Dpcd[XDPTX_DPCD_MAX_LANE_COUNT] &
                                        XDPTX_DPCD_MAX_LANE_COUNT_MASK;
        LinkConfig->MaxLinkRate = (RxMaxLinkRate > TxConfig->MaxLinkRate) ?
                        TxConfig->MaxLinkRate : RxMaxLinkRate;
        LinkConfig->MaxLaneCount = (RxMaxLaneCount > TxConfig->MaxLaneCount) ?
                                        TxConfig->MaxLaneCount : RxMaxLaneCount;

        LinkConfig->SupportEnhancedFramingMode =
                                        Dpcd[XDPTX_DPCD_MAX_LANE_COUNT] &
                                        XDPTX_DPCD_ENHANCED_FRAME_SUPPORT_MASK;
        LinkConfig->SupportDownspreadControl =
                                        Dpcd[XDPTX_DPCD_MAX_DOWNSPREAD] &
                                        XDPTX_DPCD_MAX_DOWNSPREAD_MASK;

        return XST_SUCCESS;
}

/******************************************************************************/
/**
 * This function retrieves the receiver's EDID.
 *
 * @param       InstancePtr is a pointer to the XDptx instance.
 *
*******************************************************************************/
u32 XDptx_GetEdid(XDptx *InstancePtr)
{
        u32 Status;

        Status = XDptx_IicRead(InstancePtr, XDPTX_EDID_ADDR, 0, XDPTX_EDID_SIZE,
                                                InstancePtr->RxConfig.Edid);
        if (Status != XST_SUCCESS) {
                return XST_FAILURE;
        }

        return XST_SUCCESS;
}

/******************************************************************************/
/**
 * This function determines the common capabilities between the DisplayPort TX
 * core and the receiver.
 *
 * @param       InstancePtr is a pointer to the XDptx instance.
 *
 * @return      
 *              - XST_SUCCESS if main link settings were successfully set.
 *              - XST_DEVICE_NOT_FOUND if no receiver is connected.
 *              - XST_INVALID_PARAM if the specified link configuration
 *                specifies a link rate or lane count that isn't valid.
 *              - XST_FAILURE otherwise.
 *
*******************************************************************************/
u32 XDptx_CfgMainLinkMax(XDptx *InstancePtr)
{
        u32 Status;
        XDptx_LinkConfig *LinkConfig = &InstancePtr->LinkConfig;

        /* Verify arguments. */
        Xil_AssertNonvoid(InstancePtr != NULL);
        Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

        if (!XDptx_IsConnected(InstancePtr)) {
                return XST_DEVICE_NOT_FOUND;
        }

        /* Configure the main link to the maximum common link rate between the
         * transmitter and the sink device. */
        Status = XDptx_SetLinkRate(InstancePtr, LinkConfig->MaxLinkRate);
        if (Status != XST_SUCCESS) {
                return Status;
        }

        /* Configure the main link to the maximum common lane count between the
         * transmitter and the sink device. */
        Status = XDptx_SetLaneCount(InstancePtr, LinkConfig->MaxLaneCount);
        if (Status != XST_SUCCESS) {
                return Status;
        }

        /* Configure enhanced frame mode for the main link if both the
         * transmitter and sink device. */
        Status = XDptx_SetEnhancedFrameMode(InstancePtr,
                                        LinkConfig->SupportEnhancedFramingMode);
        if (Status != XST_SUCCESS) {
                return Status;
        }

        /* Configure downspreading for the main link if both the transmitter
         * and sink device. */
        Status = XDptx_SetDownspread(InstancePtr,
                                        LinkConfig->SupportDownspreadControl);
        if (Status != XST_SUCCESS) {
                return Status;
        }

        return XST_SUCCESS;
}

/******************************************************************************/
/**
 * This function determines the common capabilities between the DisplayPort TX
 * core and the receiver.
 *
 * @param       InstancePtr is a pointer to the XDptx instance.
 *
 * @return      
 *              - XST_SUCCESS was either already trained, or has been
 *                trained successfully.
 *              - XST_DEVICE_NOT_FOUND if no receiver is connected.
 *              - XST_INVALID_PARAM if the current link rate or lane count
 *                isn't valid.
 *              - XST_FAILURE otherwise.
 *
*******************************************************************************/
u32 XDptx_EstablishLink(XDptx *InstancePtr)
{
        u32 Status;

        /* Verify arguments. */
        Xil_AssertNonvoid(InstancePtr != NULL);
        Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

        switch (InstancePtr->LinkConfig.LinkRate) {
        case XDPTX_LINK_BW_SET_540GBPS:
        case XDPTX_LINK_BW_SET_270GBPS:
        case XDPTX_LINK_BW_SET_162GBPS:
                /* Link rate is valid. */
                break;
        default:
                return XST_INVALID_PARAM;
        }
        /* Lane count gets verified while checking the link status. */

        Status = XDptx_CheckLinkStatus(InstancePtr,
                                        InstancePtr->LinkConfig.LaneCount);
        if (Status != XST_FAILURE) {
                return Status;
        }

        XDptx_ResetPhy(InstancePtr, XDPTX_PHY_CONFIG_PHY_RESET_MASK);

        XDptx_DisableMainLink(InstancePtr);

        /* Train main link. */
        Status = XDptx_RunTraining(InstancePtr);
        if (Status != XST_SUCCESS) {
                return XST_FAILURE;
        }

        XDptx_EnableMainLink(InstancePtr);

        return XST_SUCCESS;
}

/******************************************************************************/
/**
 * This function checks if the reciever's DPCD indicates the reciever has
 * achieved and maintained clock recovery, channel equalization, symbol lock,
 * and interlane alignment for all lanes currently in use.
 *
 * @param       InstancePtr is a pointer to the XDptx instance.
 * @param       LaneCount is the number of lanes to check.
 *
 * @return
 *              - XST_SUCCESS if the receiver has maintained clock recovery,
 *                channel equalization, symbol lock, and interlane alignment.
 *              - XST_DEVICE_NOT_FOUND if no receiver is connected.
 *              - XST_INVALID_PARAM if the number of lanes to check does not
                  match 1, 2, or 4 lanes.
 *              - XST_FAILURE otherwise.
 *
*******************************************************************************/
u32 XDptx_CheckLinkStatus(XDptx *InstancePtr, u8 LaneCount)
{
        u8 RetryCount = 0;
        XDptx_LinkConfig *LinkConfig = &InstancePtr->LinkConfig;
        u32 Status;

        /* Verify arguments. */
        Xil_AssertNonvoid(InstancePtr != NULL);
        Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

        if ((LaneCount != 1) && (LaneCount != 2) && (LaneCount != 4)) {
                return XST_INVALID_PARAM;
        }

        if (!XDptx_IsConnected(InstancePtr)) {
                return XST_DEVICE_NOT_FOUND;
        }

        /* Retrieve AUX info. */
        do {
                /* Get lane and adjustment requests. */
                Status = XDptx_GetLaneStatusAdjReqs(InstancePtr);
                if (Status != XST_SUCCESS) {
                        /* The AUX read failed. */
                        return XST_FAILURE;
                }

                /* Check if the link needs training. */
                if ((XDptx_CheckClockRecovery(
                                InstancePtr, LaneCount) == XST_SUCCESS) &&
                        (XDptx_CheckChannelEqualization(
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
 * @param       InstancePtr is a pointer to the XDptx instance.
 * @param       Enable controls the downshift feature in the training process.
 *
*******************************************************************************/
void XDptx_EnableTrainAdaptive(XDptx *InstancePtr, u8 Enable)
{
        /* Verify arguments. */
        Xil_AssertVoid(InstancePtr != NULL);
        Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

        InstancePtr->TrainAdaptive = Enable;
}

/******************************************************************************/
/**
 * This function sets a software switch that signifies whether or not a redriver
 * exists on the DisplayPort output path. XDptx_SetVswingPreemp uses this switch
 * to determine which set of voltage swing and pre-emphasis values to use in the
 * TX core.
 *
 * @param       InstancePtr is a pointer to the XDptx instance.
 * @param       Set establishes that a redriver exists in the DisplayPort output
 *              path.
 *
*******************************************************************************/
void XDptx_SetHasRedriverInPath(XDptx *InstancePtr, u8 Set)
{
        /* Verify arguments. */
        Xil_AssertVoid(InstancePtr != NULL);
        Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

        InstancePtr->HasRedriverInPath = Set;
}

/******************************************************************************/
/**
 * This function issues a read request over the AUX channel.
 *
 * @param       InstancePtr is a pointer to the XDptx instance.
 * @param       Address is the starting address to read from the receiver.
 * @param       NumBytes is the number of bytes to read from the receiver.
 * @param       Data is a pointer to the data buffer that will be filled with
 *              read data.
 *
 * @return
 *              - XST_SUCCESS if the AUX read request was successfully
 *                acknowledged.
 *              - XST_DEVICE_NOT_FOUND if no receiver is connected.
 *              - XST_NO_DATA if no data was provided.
 *              - XST_ERROR_COUNT_MAX if the AUX request timed out.
 *              - XST_FAILURE otherwise.
 *
*******************************************************************************/
u32 XDptx_AuxRead(XDptx *InstancePtr, u32 Address, u32 NumBytes, void *Data)
{
        u32 Status;
        XDptx_AuxTransaction Request;

        /* Verify arguments. */
        Xil_AssertNonvoid(InstancePtr != NULL);
        Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
        Xil_AssertNonvoid(Address <= 0xFFFFF);
        Xil_AssertNonvoid(NumBytes <= 0xFFFFF);
        Xil_AssertNonvoid(Data != NULL);

        if (!XDptx_IsConnected(InstancePtr)) {
                return XST_DEVICE_NOT_FOUND;
        }

        if (Data == NULL) {
                return XST_NO_DATA;
        }

        /* Send AUX read transaction. */
        Status = XDptx_AuxCommon(InstancePtr, XDPTX_AUX_CMD_READ, Address,
                                                        NumBytes, (u8 *)Data);

        return Status;
}

/******************************************************************************/
/**
 * This function issues a write request over the AUX channel.
 *
 * @param       InstancePtr is a pointer to the XDptx instance.
 * @param       Address is the starting address to write to the receiver.
 * @param       NumBytes is the number of bytes to write to the receiver.
 * @param       Data is a pointer to the data buffer that contains the data
 *              to be written to the receiver.
 *
 * @return
 *              - XST_SUCCESS if AUX write request was successfully
 *                acknowledged.
 *              - XST_DEVICE_NOT_FOUND if no receiver is connected.
 *              - XST_NO_DATA if no data was provided.
 *              - XST_ERROR_COUNT_MAX if the AUX request timed out.
 *              - XST_FAILURE otherwise.
 *
*******************************************************************************/
u32 XDptx_AuxWrite(XDptx *InstancePtr, u32 Address, u32 NumBytes, void *Data)
{
        u32 Status;
        XDptx_AuxTransaction Request;

        /* Verify arguments. */
        Xil_AssertNonvoid(InstancePtr != NULL);
        Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
        Xil_AssertNonvoid(Address <= 0xFFFFF);
        Xil_AssertNonvoid(NumBytes <= 0xFFFFF);
        Xil_AssertNonvoid(Data != NULL);

        if (!XDptx_IsConnected(InstancePtr)) {
                return XST_DEVICE_NOT_FOUND;
        }

        if (Data == NULL) {
                return XST_NO_DATA;
        }

        /* Send AUX write transaction. */
        Status = XDptx_AuxCommon(InstancePtr, XDPTX_AUX_CMD_WRITE, Address,
                                                        NumBytes, (u8 *)Data);

        return Status;
}

/******************************************************************************/
/**
 * This function performs an I2C read over the AUX channel.
 *
 * @param       InstancePtr is a pointer to the XDptx instance.
 * @param       IicAddress is the address on the I2C bus of the target device.
 * @param       RegStartAddress is the subaddress of the targeted I2C device
 *              that the read will start from.
 * @param       NumBytes is the number of bytes to read.
 * @param       Data is a pointer to a buffer that will be filled with the I2C
 *              read data.
 *
 * @return
 *              - XST_SUCCESS if the I2C read has successfully completed with no
 *                errors.
 *              - XST_ERROR_COUNT_MAX if the AUX request timed out.
 *              - XST_DEVICE_NOT_FOUND if no receiver is connected.
 *              - XST_FAILURE otherwise.
 *
*******************************************************************************/
u32 XDptx_IicRead(XDptx *InstancePtr, u8 IicAddress, u8 RegStartAddress,
                                                        u8 NumBytes, void *Data)
{
        u32 Status;
        XDptx_AuxTransaction Request;
        u8 AuxData[2];

        /* Verify arguments. */
        Xil_AssertNonvoid(InstancePtr != NULL);
        Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
        Xil_AssertNonvoid(IicAddress <= 0xFFFFF);
        Xil_AssertNonvoid(RegStartAddress <= 256);
        Xil_AssertNonvoid(NumBytes <= 256);
        Xil_AssertNonvoid(Data != NULL);

        if (!XDptx_IsConnected(InstancePtr)) {
                return XST_DEVICE_NOT_FOUND;
        }

        /* Setup the I2C-over-AUX read transaction with the address. */
        Request.Address = IicAddress;
        Request.CmdCode = XDPTX_AUX_CMD_I2C_WRITE_MOT;
        Request.NumBytes = 2;
        AuxData[0] = RegStartAddress;
        AuxData[1] = 0;
        Request.Data = AuxData;
        Status = XDptx_AuxRequest(InstancePtr, &Request);
        if (Status != XST_SUCCESS) {
                return Status;
        }

        /* Send I2C-over-AUX read transaction. */
        Status = XDptx_AuxCommon(InstancePtr, XDPTX_AUX_CMD_I2C_READ,
                                        IicAddress, NumBytes, (u8 *)Data);

        return Status;
}

/******************************************************************************/
/**
 * This function performs an I2C write over the AUX channel.
 *
 * @param       InstancePtr is a pointer to the XDptx instance.
 * @param       IicAddress is the address on the I2C bus of the target device.
 * @param       RegStartAddress is the sub-address of the targeted I2C device
 *              that the write will start at.
 * @param       NumBytes is the number of bytes to write.
 * @param       Data is a pointer to a buffer which will be used as the data
 *              source for the write.
 *
 * @return
 *              - XST_SUCCESS if the I2C write has successfully completed with
 *                no errors.
 *              - XST_DEVICE_NOT_FOUND if no receiver is connected.
 *              - XST_ERROR_COUNT_MAX if the AUX request timed out.
 *              - XST_FAILURE otherwise.
 *
*******************************************************************************/
u32 XDptx_IicWrite(XDptx *InstancePtr, u8 IicAddress, u8 RegStartAddress,
                                                        u8 NumBytes, void *Data)
{
        u32 Status;
        XDptx_AuxTransaction Request;
        u8 AuxData[2];

        /* Verify arguments. */
        Xil_AssertNonvoid(InstancePtr != NULL);
        Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
        Xil_AssertNonvoid(IicAddress <= 0xFFFFF);
        Xil_AssertNonvoid(RegStartAddress <= 256);
        Xil_AssertNonvoid(NumBytes <= 256);
        Xil_AssertNonvoid(Data != NULL);

        if (!XDptx_IsConnected(InstancePtr)) {
                return XST_DEVICE_NOT_FOUND;
        }

        /* Setup the I2C-over-AUX write transaction with the address. */
        Request.Address = IicAddress;
        Request.CmdCode = XDPTX_AUX_CMD_I2C_WRITE_MOT;
        Request.NumBytes = 2;
        AuxData[0] = RegStartAddress;
        AuxData[1] = 0;
        Request.Data = AuxData;
        Status = XDptx_AuxRequest(InstancePtr, &Request);
        if (Status != XST_SUCCESS) {
                return Status;
        }

        /* Send I2C-over-AUX read transaction. */
        Status = XDptx_AuxCommon(InstancePtr, XDPTX_AUX_CMD_I2C_READ,
                                        IicAddress, NumBytes, (u8 *)Data);

        return Status;
}

/******************************************************************************/
/**
 * This function enables or disables 0.5% spreading of the clock for both the
 * DisplayPort and the sink device.
 *
 * @param       InstancePtr is a pointer to the XDptx instance.
 * @param       Enable will enable or disable down-spread control.
 *
 * @return
 *              - XST_SUCCESS if setting the downspread control enable was
 *                successful.
 *              - XST_DEVICE_NOT_FOUND if no receiver is connected.
 *              - XST_FAILURE otherwise.
 *
*******************************************************************************/
u32 XDptx_SetDownspread(XDptx *InstancePtr, u8 Enable)
{
        u32 Status;
        u8 RegVal;

        /* Verify arguments. */
        Xil_AssertNonvoid(InstancePtr != NULL);
        Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

        if (!XDptx_IsConnected(InstancePtr)) {
                return XST_DEVICE_NOT_FOUND;
        }

        InstancePtr->LinkConfig.DownspreadControl = (Enable) ? 1 : 0;

        /* Write downspread enable to the transmitter. */
        XDptx_WriteReg(InstancePtr->TxConfig.BaseAddr, XDPTX_DOWNSPREAD_CTRL,
                                InstancePtr->LinkConfig.DownspreadControl);

        /* Preserve the current receiver settings. */
        Status = XDptx_AuxRead(InstancePtr, XDPTX_DPCD_DOWNSPREAD_CTRL, 1,
                                                                &RegVal);
        if (Status != XST_SUCCESS) {
                return XST_FAILURE;
        }
        if (InstancePtr->LinkConfig.DownspreadControl) {
                RegVal |= XDPTX_DPCD_SPREAD_AMP_MASK;
        }
        else {
                RegVal &= ~XDPTX_DPCD_SPREAD_AMP_MASK;
        }

        /* Write downspread enable to the sink device. */
        Status = XDptx_AuxWrite(InstancePtr, XDPTX_DPCD_DOWNSPREAD_CTRL, 1,
                                                                &RegVal);
        if (Status != XST_SUCCESS) {
                return XST_FAILURE;
        }

        return XST_SUCCESS;
}

/******************************************************************************/
/**
 * This function enables or disables the enhanced framing symbol sequence for
 * both the DisplayPort TX core and the sink device.
 *
 * @param       InstancePtr is a pointer to the XDptx instance.
 * @param       Enable will enable or disable enhanced frame mode.
 *
 * @return
 *              - XST_SUCCESS if setting the enhanced frame mode enable was
 *                successful.
 *              - XST_DEVICE_NOT_FOUND if no receiver is connected.
 *              - XST_FAILURE otherwise.
 *
*******************************************************************************/
u32 XDptx_SetEnhancedFrameMode(XDptx *InstancePtr, u8 Enable)
{
        u32 Status;
        u8 RegVal;

        /* Verify arguments. */
        Xil_AssertNonvoid(InstancePtr != NULL);
        Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

        if (!XDptx_IsConnected(InstancePtr)) {
                return XST_DEVICE_NOT_FOUND;
        }

        InstancePtr->LinkConfig.EnhancedFramingMode = (Enable) ? 1 : 0;

        /* Write enhanced frame mode enable to the transmitter. */
        XDptx_WriteReg(InstancePtr->TxConfig.BaseAddr, XDPTX_ENHANCED_FRAME_EN,
                                InstancePtr->LinkConfig.EnhancedFramingMode);

        /* Preserve the current receiver settings. */
        Status = XDptx_AuxRead(InstancePtr, XDPTX_DPCD_LANE_COUNT_SET, 1,
                                                                &RegVal);
        if (Status != XST_SUCCESS) {
                return XST_FAILURE;
        }
        if (InstancePtr->LinkConfig.EnhancedFramingMode) {
                RegVal |= XDPTX_DPCD_ENHANCED_FRAME_EN_MASK;
        }
        else {
                RegVal &= ~XDPTX_DPCD_ENHANCED_FRAME_EN_MASK;
        }

        /* Write enhanced frame mode enable to the sink device. */
        Status = XDptx_AuxWrite(InstancePtr, XDPTX_DPCD_LANE_COUNT_SET, 1,
                                                                &RegVal);
        if (Status != XST_SUCCESS) {
                return XST_FAILURE;
        }

        return XST_SUCCESS;
}

/******************************************************************************/
/**
 * This function sets the number of lanes to be used by the main link for both
 * the DisplayPort TX core and the sink device.
 *
 * @param       InstancePtr is a pointer to the XDptx instance.
 * @param       LaneCount is the number of lanes to be used over the main link.
 *
 * @return
 *              - XST_SUCCESS if setting the new lane count was successful.
 *              - XST_DEVICE_NOT_FOUND if no receiver is connected.
 *              - XST_INVALID_PARAM if the supplied lane count is not either 1,
 *                2, or 4 lanes.
 *              - XST_FAILURE otherwise.
 *
*******************************************************************************/
u32 XDptx_SetLaneCount(XDptx *InstancePtr, u8 LaneCount)
{
        u32 Status;
        u8 RegVal;

        /* Verify arguments. */
        Xil_AssertNonvoid(InstancePtr != NULL);
        Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

        if ((LaneCount != 1) && (LaneCount != 2) && (LaneCount != 4)) {
                return XST_INVALID_PARAM;
        }

        if (!XDptx_IsConnected(InstancePtr)) {
                return XST_DEVICE_NOT_FOUND;
        }

        InstancePtr->LinkConfig.LaneCount = LaneCount;

        /* Write the new lane count to the transmitter. */
        XDptx_WriteReg(InstancePtr->TxConfig.BaseAddr, XDPTX_LANE_COUNT_SET,
                                        InstancePtr->LinkConfig.LaneCount);

        /* Preserve the current receiver settings. */
        Status = XDptx_AuxRead(InstancePtr, XDPTX_DPCD_LANE_COUNT_SET, 1,
                                                                &RegVal);
        if (Status != XST_SUCCESS) {
                return XST_FAILURE;
        }
        RegVal &= ~XDPTX_DPCD_LANE_COUNT_SET_MASK;
        RegVal |= InstancePtr->LinkConfig.LaneCount;

        /* Write the new lane count to the sink device. */
        Status = XDptx_AuxWrite(InstancePtr, XDPTX_DPCD_LANE_COUNT_SET, 1,
                                                                &RegVal);
        if (Status != XST_SUCCESS) {
                return XST_FAILURE;
        }

        return XST_SUCCESS;
}

/******************************************************************************/
/**
 * This function sets the data rate to be used by the main link for both the
 * DisplayPort TX core and the sink device.
 *
 * @param       InstancePtr is a pointer to the XDptx instance.
 * @param       LinkRate is the link rate to be used over the main link based on
 *              one of the following selects:
 *              - XDPTX_LINK_BW_SET_162GBPS = 0x06 (for a 1.62 Gbps data rate)
 *              - XDPTX_LINK_BW_SET_270GBPS = 0x0A (for a 2.70 Gbps data rate)
 *              - XDPTX_LINK_BW_SET_540GBPS = 0x14 (for a 5.40 Gbps data rate)
 *
 * @return
 *              - XST_SUCCESS if setting the new link rate was successful.
 *              - XST_DEVICE_NOT_FOUND if no receiver is connected.
 *              - XST_INVALID_PARAM if the supplied link rate does not
 *                correspond to either 1.62, 2.70, or 5.40 Gbps.
 *              - XST_FAILURE otherwise.
 *
*******************************************************************************/
u32 XDptx_SetLinkRate(XDptx *InstancePtr, u8 LinkRate)
{
        u32 Status;

        /* Verify arguments. */
        Xil_AssertNonvoid(InstancePtr != NULL);
        Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

        if (!XDptx_IsConnected(InstancePtr)) {
                return XST_DEVICE_NOT_FOUND;
        }

        /* Write a corresponding clock frequency to the transmitter. */
        switch (LinkRate) {
        case XDPTX_LINK_BW_SET_162GBPS:
                Status = XDptx_SetClkSpeed(InstancePtr,
                                                XDPTX_PHY_CLOCK_SELECT_162GBPS);
                break;
        case XDPTX_LINK_BW_SET_270GBPS:
                Status = XDptx_SetClkSpeed(InstancePtr,
                                                XDPTX_PHY_CLOCK_SELECT_270GBPS);
                break;
        case XDPTX_LINK_BW_SET_540GBPS:
                Status = XDptx_SetClkSpeed(InstancePtr,
                                                XDPTX_PHY_CLOCK_SELECT_540GBPS);
                break;
        default:
                return XST_INVALID_PARAM;
        }
        if (Status != XST_SUCCESS) {
                return XST_FAILURE;
        }

        InstancePtr->LinkConfig.LinkRate = LinkRate;

        /* Write new link rate to transmitter. */
        XDptx_WriteReg(InstancePtr->TxConfig.BaseAddr, XDPTX_LINK_BW_SET,
                                        InstancePtr->LinkConfig.LinkRate);

        /* Write new link rate to sink device. */
        Status = XDptx_AuxWrite(InstancePtr, XDPTX_DPCD_LINK_BW_SET, 1,
                                        &InstancePtr->LinkConfig.LinkRate);
        if (Status != XST_SUCCESS) {
                return XST_FAILURE;
        }

        return XST_SUCCESS;
}

/******************************************************************************/
/**
 * This function enables or disables scrambling of symbols for both the
 * DisplayPort and the sink device.
 *
 * @param       InstancePtr is a pointer to the XDptx instance.
 * @param       Enable will enable or disable scrambling.
 *
 * @return
 *              - XST_SUCCESS if setting the scrambling enable was successful.
 *              - XST_FAILURE otherwise.
 *
*******************************************************************************/
u32 XDptx_SetScrambler(XDptx *InstancePtr, u8 Enable)
{
        u32 Status;
        u8 RegVal;

        /* Verify arguments. */
        Xil_AssertNonvoid(InstancePtr != NULL);
        Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

        InstancePtr->LinkConfig.ScramblerEn = (Enable) ? 1 : 0;

        /* Write scrambler disable to the transmitter. */
        XDptx_WriteReg(InstancePtr->TxConfig.BaseAddr, XDPTX_SCRAMBLING_DISABLE,
                                                                Enable ? 0 : 1);

        /* Preserve the current receiver settings. */
        Status = XDptx_AuxRead(InstancePtr, XDPTX_DPCD_TP_SET, 1, &RegVal);
        if (Status != XST_SUCCESS) {
                return XST_FAILURE;
        }
        if (Enable) {
                RegVal &= ~XDPTX_DPCD_TP_SET_SCRAMB_DIS_MASK;
        }
        else {
                RegVal |= XDPTX_DPCD_TP_SET_SCRAMB_DIS_MASK;
        }

        /* Write scrambler disable to the sink device. */
        Status = XDptx_AuxWrite(InstancePtr, XDPTX_DPCD_TP_SET, 1, &RegVal);
        if (Status != XST_SUCCESS) {
                return XST_FAILURE;
        }

        return XST_SUCCESS;
}

/******************************************************************************/
/**
 * This function enables the main link.
 *
 * @param       InstancePtr is a pointer to the XDptx instance.
 *
*******************************************************************************/
void XDptx_EnableMainLink(XDptx *InstancePtr)
{
        /* Verify arguments. */
        Xil_AssertVoid(InstancePtr != NULL);
        Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

        /* Reset the scrambler. */
        XDptx_WriteReg(InstancePtr->TxConfig.BaseAddr,
                                                XDPTX_FORCE_SCRAMBLER_RESET, 1);

        /* Enable the main stream. */
        XDptx_WriteReg(InstancePtr->TxConfig.BaseAddr,
                                                XDPTX_ENABLE_MAIN_STREAM, 1);
}

/******************************************************************************/
/**
 * This function disables the main link.
 *
 * @param       InstancePtr is a pointer to the XDptx instance.
 *
*******************************************************************************/
void XDptx_DisableMainLink(XDptx *InstancePtr)
{
        /* Verify arguments. */
        Xil_AssertVoid(InstancePtr != NULL);
        Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

        /* Reset the scrambler. */
        XDptx_WriteReg(InstancePtr->TxConfig.BaseAddr,
                                                XDPTX_FORCE_SCRAMBLER_RESET, 1);

        /* Disable the main stream. */
        XDptx_WriteReg(InstancePtr->TxConfig.BaseAddr,
                                                XDPTX_ENABLE_MAIN_STREAM, 0);
}

/******************************************************************************/
/**
 * This function does a PHY reset.
 *
 * @param       InstancePtr is a pointer to the XDptx instance.
 & @param       Reset is the type of reset to assert.
 *
*******************************************************************************/
void XDptx_ResetPhy(XDptx *InstancePtr, u32 Reset)
{
        /* Verify arguments. */
        Xil_AssertVoid(InstancePtr != NULL);
        Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

        XDptx_WriteReg(InstancePtr->TxConfig.BaseAddr, XDPTX_ENABLE, 0);

        XDptx_WriteReg(InstancePtr->TxConfig.BaseAddr, XDPTX_PHY_CONFIG, Reset);
        XDptx_WriteReg(InstancePtr->TxConfig.BaseAddr, XDPTX_PHY_CONFIG,
                                        XDPTX_PHY_CONFIG_PHY_RESET_ENABLE_MASK);
        XDptx_WaitPhyReady(InstancePtr);

        XDptx_WriteReg(InstancePtr->TxConfig.BaseAddr, XDPTX_ENABLE, 1);
}

/******************************************************************************/
/**
 * This function installs a custom delay/sleep function to be used by the XDdptx
 * driver.
 *
 * @param       InstancePtr is a pointer to the XDptx instance.
 * @param       CallbackFunc is the address to the callback function.
 * @param       CallbackRef is the user data item (microseconds to delay) that
 *              will be passed to the custom sleep/delay function when it is
 *              invoked.
 *
*******************************************************************************/
void XDptx_SetUserTimerHandler(XDptx *InstancePtr,
                        XDptx_TimerHandler CallbackFunc, void *CallbackRef)
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
 * This function is the delay/sleep function for the XDptx driver. For the Zynq
 * family, there exists native sleep functionality. For MicroBlaze however,
 * there does not exist such functionality. In the MicroBlaze case, the default
 * method for delaying is to use a predetermined amount of loop iterations. This
 * method is prone to inaccuracy and dependent on system configuration; for
 * greater accuracy, the user may supply their own delay/sleep handler, pointed
 * to by InstancePtr->UserTimerWaitUs, which may have better accuracy if a
 * hardware timer is used.
 *
 * @param       InstancePtr is a pointer to the XDptx instance.
 * @param       MicroSeconds is the number of microseconds to delay/sleep for.
 *
*******************************************************************************/
void XDptx_WaitUs(XDptx *InstancePtr, u32 MicroSeconds)
{
        /* Verify arguments. */
        Xil_AssertVoid(InstancePtr != NULL);
        Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

#if defined(__MICROBLAZE__)
        if (InstancePtr->UserTimerWaitUs != NULL) {
                /* Use the timer handler specified by the user for better
                 * accuracy. */
                InstancePtr->UserTimerWaitUs(InstancePtr, MicroSeconds);
        }
        else {
                /* MicroBlaze sleep only has millisecond accuracy. Round up. */
                u32 MilliSeconds = (MicroSeconds + 999) / 1000;
                MB_Sleep(MilliSeconds);
        }
#elif defined(__arm__)
        /* Wait the requested amount of time. */
        usleep(MicroSeconds);
#endif
}

/******************************************************************************/
/**
 * This function runs the link training process. It is implemented as a state
 * machine, with each state returning the next state. First, the clock recovery
 * sequence will be run; if successful, the channel equalization sequence will
 * run. If either the clock recovery or channel equalization sequence failed,
 * the data rate or the number of lanes used will be reduced and training will
 * be re-attempted. If training fails at the minimal data rate, 1.62 Gbps with
 * a single lane, training will no longer re-attempt and fail.
 *
 * @param       InstancePtr is a pointer to the XDptx instance.
 *
 * @return      The next training state:
 *              - XST_SUCCESS if the training process succeeded.
 *              - XST_FAILURE otherwise.
 *
*******************************************************************************/
static u32 XDptx_RunTraining(XDptx *InstancePtr)
{
        u32 Status;
        XDptx_TrainingState TrainingState = XDPTX_TS_CLOCK_RECOVERY;

        /* Verify arguments. */
        Xil_AssertNonvoid(InstancePtr != NULL);
        Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

        /* Disable scrambler. */
        Status = XDptx_SetScrambler(InstancePtr, 0);
        if (Status != XST_SUCCESS) {
                return XST_FAILURE;
        }

        while (1) {
                switch (TrainingState) {
                case XDPTX_TS_CLOCK_RECOVERY:
                        TrainingState = XDptx_TrainingStateClockRecovery(
                                                                InstancePtr);
                        break;
                case XDPTX_TS_CHANNEL_EQUALIZATION:
                        TrainingState = XDptx_TrainingStateChannelEqualization(
                                                                InstancePtr, 5);
                        break;
                case XDPTX_TS_ADJUST_LINK_RATE:
                        TrainingState = XDptx_TrainingStateAdjustLinkRate(
                                                                InstancePtr);
                        break;
                case XDPTX_TS_ADJUST_LANE_COUNT:
                        TrainingState = XDptx_TrainingStateAdjustLaneCount(
                                                                InstancePtr);
                        break;
                case XDPTX_TS_FAILURE:
                        return XST_FAILURE;
                default:
                        break;
                }

                if (TrainingState == XDPTX_TS_SUCCESS) {
                        break;
                }

                if ((InstancePtr->TrainAdaptive == 0) &&
                        ((TrainingState == XDPTX_TS_ADJUST_LANE_COUNT) ||
                                (TrainingState == XDPTX_TS_ADJUST_LINK_RATE))) {
                        return XST_FAILURE;
                }
        }

        /* Turn off the training pattern. */
        Status = XDptx_SetTrainingPattern(InstancePtr,
                                                XDPTX_TRAINING_PATTERN_SET_OFF);
        if (Status != XST_SUCCESS) {
                return XST_FAILURE;
        }

        /* Enable scrambler. */
        Status = XDptx_SetScrambler(InstancePtr, 1);
        if (Status != XST_SUCCESS) {
                return XST_FAILURE;
        }

        /* Final status check. */
        Status = XDptx_CheckLinkStatus(InstancePtr,
                                        InstancePtr->LinkConfig.LaneCount);
        if (Status != XST_SUCCESS) {
                return XST_FAILURE;
        }

        return XST_SUCCESS;
}

/******************************************************************************/
/**
 * This function runs the clock recovery sequence as part of link training. The
 * sequence is as follows:
 *      0) Start signaling at the minimum voltage swing, pre-emphasis, and post-
 *         cursor levels.
 *      1) Transmit training pattern 1 over the main link with symbol scrambling
 *         disabled.
 *      2) The clock recovery loop. If clock recovery is unsuccessful after
 *         MaxIterations loop iterations, return.
 *      2a) Wait for at least the period of time specified in the receiver's
 *          DPCD register, TRAINING_AUX_RD_INTERVAL.
 *      2b) Check if all lanes have achieved clock recovery lock. If so, return.
 *      2c) Check if the same voltage swing level has been used 5 consecutive
 *          times or if the maximum level has been reached. If so, return.
 *      2d) Adjust the voltage swing, pre-emphasis, and post-cursor levels as
 *          requested by the receiver.
 *      2e) Loop back to 2a.
 * For a more detailed description of the clock recovery sequence, see section
 * 3.5.1.2.1 of the DisplayPort 1.2a specification document.
 * 
 * @param       InstancePtr is a pointer to the XDptx instance.
 *
 * @return
 *              - XDPTX_TS_CHANNEL_EQUALIZATION if the clock recovery sequence
 *                completed successfully.
 *              - XDPTX_TS_FAILURE if writing the drive settings to the receiver
 *                was unsuccesful.
 *              - XDPTX_TS_ADJUST_LINK_RATE if CR is unsuccessful.
 *
*******************************************************************************/
static XDptx_TrainingState XDptx_TrainingStateClockRecovery(XDptx *InstancePtr)
{
        u32 Status;
        u32 DelayUs;
        u8 PrevVsLevel = 0;
        u8 SameVsLevelCount = 0;
        XDptx_LinkConfig *LinkConfig = &InstancePtr->LinkConfig;

        /* Verify arguments. */
        Xil_AssertNonvoid(InstancePtr != NULL);
        Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

        /* Obtain the required delay for CR as specified in the sink's DPCD. */
        DelayUs = XDptx_GetTrainingDelay(InstancePtr, XDPTX_TS_CLOCK_RECOVERY);

        /* Start CRLock. */

        /* Transmit training pattern 1. */
        Status = XDptx_SetTrainingPattern(InstancePtr,
                                                XDPTX_TRAINING_PATTERN_SET_TP1);
        if (Status != XST_SUCCESS) {
                return XDPTX_TS_FAILURE;
        }

        /* Start from minimal voltage swing and pre-emphasis levels. */
        LinkConfig->VsLevel = 0;
        LinkConfig->PeLevel = 0;
        Status = XDptx_SetVswingPreemp(InstancePtr);
        if (Status != XST_SUCCESS) {
                return XDPTX_TS_FAILURE;
        }

        while (1) {
                /* Wait delay specified in TRAINING_AUX_RD_INTERVAL. */
                XDptx_WaitUs(InstancePtr, DelayUs);

                /* Get lane and adjustment requests. */
                Status = XDptx_GetLaneStatusAdjReqs(InstancePtr);
                if (Status != XST_SUCCESS) {
                        /* The AUX read failed. */
                        return XDPTX_TS_FAILURE;
                }

                /* Check if all lanes have realized and maintained the frequency
                 * loc and get adjustment requests. */
                Status = XDptx_CheckClockRecovery(InstancePtr,
                                InstancePtr->LinkConfig.LaneCount);
                if (Status == XST_SUCCESS) {
                        return XDPTX_TS_CHANNEL_EQUALIZATION;
                }

                /* Check if training has tried, and failed, with the maximum
                 * voltage swing. */
                if (LinkConfig->VsLevel == XDPTX_MAXIMUM_VS_LEVEL) {
                        break;
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

                /* Adjust the drive settings as requested by the sink device. */
                Status = XDptx_AdjVswingPreemp(InstancePtr);
                if (Status != XST_SUCCESS) {
                        /* The AUX write failed. */
                        return XDPTX_TS_FAILURE;
                }
        }

        return XDPTX_TS_ADJUST_LINK_RATE;
}

/******************************************************************************/
/**
 * This function runs the channel equalization sequence as part of link
 * training. The sequence is as follows:
 *      0) Start signaling with the same drive settings used at the end of the
 *         clock recovery sequence.
 *      1) Transmit training pattern 2 (or 3) over the main link with symbol
 *         scrambling disabled.
 *      2) The channel equalization loop. If channel equalization is
 *         unsuccessful after MaxIterations loop iterations, return.
 *      2a) Wait for at least the period of time specified in the receiver's
 *          DPCD register, TRAINING_AUX_RD_INTERVAL.
 *      2b) Check if all lanes have achieved channel equalization, symbol lock,
 *          and interlane alignment. If so, return.
 *      2c) Check if the same voltage swing level has been used 5 consecutive
 *          times or if the maximum level has been reached. If so, return.
 *      2d) Adjust the voltage swing, pre-emphasis, and post-cursor levels as
 *          requested by the receiver.
 *      2e) Loop back to 2a.
 * For a more detailed description of the channel equalization sequence, see
 * section 3.5.1.2.2 of the DisplayPort 1.2a specification document.
 *
 * @param       InstancePtr is a pointer to the XDptx instance.
 * @param       MaxIterations is the maximum number of times to loop through the
 *              clock recovery sequence before down-shifting to a reduced data
 *              rate or a reduced number of lanes.
 *
 * @return
 *              - XDPTX_TS_SUCCESS if training succeeded.
 *              - XDPTX_TS_FAILURE if writing the drive settings to the receiver
 *                was unsuccesful.
 *              - XDPTX_TS_ADJUST_LINK_RATE if, after MaxIterations loop
 *                iterations, clock recovery is unsuccessful.
 *
*******************************************************************************/
static XDptx_TrainingState XDptx_TrainingStateChannelEqualization(
                                        XDptx *InstancePtr, u32 MaxIterations)
{
        u32 Status;
        u32 DelayUs;
        u32 IterationCount = 0;
        u8 PrevVsLevel = 0;
        u8 SameVsLevelCount = 0;
        XDptx_LinkConfig *LinkConfig = &InstancePtr->LinkConfig;

        /* Verify arguments. */
        Xil_AssertNonvoid(InstancePtr != NULL);
        Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

        /* Obtain the required delay for CE as specified in the sink's DPCD. */
        DelayUs = XDptx_GetTrainingDelay(InstancePtr,
                                                XDPTX_TS_CHANNEL_EQUALIZATION);

        /* Start channel equalization. */

        /* Write the current drive settings to the sink device. */
        Status = XDptx_SetVswingPreemp(InstancePtr);
        if (Status != XST_SUCCESS) {
                return XDPTX_TS_FAILURE;
        }

        /* Transmit training pattern 2/3. */
        if (InstancePtr->RxConfig.DpcdRxCapsField[XDPTX_DPCD_MAX_LANE_COUNT] &
                                                XDPTX_DPCD_TPS3_SUPPORT_MASK) {
                Status = XDptx_SetTrainingPattern(InstancePtr,
                                                XDPTX_TRAINING_PATTERN_SET_TP3);
        }
        else {
                Status = XDptx_SetTrainingPattern(InstancePtr,
                                                XDPTX_TRAINING_PATTERN_SET_TP2);
        }
        if (Status != XST_SUCCESS) {
                return XDPTX_TS_FAILURE;
        }

        while (IterationCount < MaxIterations) {
                /* Wait delay specified in TRAINING_AUX_RD_INTERVAL. */
                XDptx_WaitUs(InstancePtr, DelayUs);

                /* Get lane and adjustment requests. */
                Status = XDptx_GetLaneStatusAdjReqs(InstancePtr);
                if (Status != XST_SUCCESS) {
                        /* The AUX read failed. */
                        return XDPTX_TS_FAILURE;
                }

                /* Check that all lanes still have their clocks locked. */
                Status = XDptx_CheckClockRecovery(InstancePtr,
                                InstancePtr->LinkConfig.LaneCount);
                if (Status != XST_SUCCESS) {
                        break;
                }

                /* Check that all lanes stihave accomplished channel equalization,
                 * symbol lock, and interlane alignment. */
                Status = XDptx_CheckChannelEqualization(InstancePtr,
                                        InstancePtr->LinkConfig.LaneCount);
                if (Status == XST_SUCCESS) {
                        return XDPTX_TS_SUCCESS;
                }

                /* Check if training has tried, and failed, with the maximum
                 * voltage swing. */
                if (LinkConfig->VsLevel == XDPTX_MAXIMUM_VS_LEVEL) {
                        break;
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

                /* Adjust the drive settings as requested by the sink device. */
                Status = XDptx_AdjVswingPreemp(InstancePtr);
                if (Status != XST_SUCCESS) {
                        /* The AUX write failed. */
                        return XDPTX_TS_FAILURE;
                }

                IterationCount++;
        }

        /* Tried MaxIteration times with no success. Try a reduced bitrate
         * first, then reduce the number of lanes. */
        return XDPTX_TS_ADJUST_LINK_RATE;
}

/******************************************************************************/
/**
 * This function is reached if either the clock recovery or the channel
 * equalization process failed during training. As a result, the data rate will
 * be downshifted, and training will be re-attempted (starting with clock
 * recovery) at the reduced data rate. If the data rate is already at 1.62 Gbps,
 * a downshift in lane count will be attempted.
 *
 * @param       InstancePtr is a pointer to the XDptx instance.
 *
 * @return      The next training state:
 *              - XDPTX_TS_ADJUST_LANE_COUNT if the minimal data rate is already
 *                in use. Re-attempt training at a reduced lane count.
 *              - XDPTX_TS_CLOCK_RECOVERY otherwise. Re-attempt training.
 *
*******************************************************************************/
static XDptx_TrainingState XDptx_TrainingStateAdjustLinkRate(XDptx *InstancePtr)
{
        u32 Status;

        /* Verify arguments. */
        Xil_AssertNonvoid(InstancePtr != NULL);
        Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

        switch (InstancePtr->LinkConfig.LinkRate) {
        case XDPTX_LINK_BW_SET_540GBPS:
                Status = XDptx_SetLinkRate(InstancePtr,
                                                XDPTX_LINK_BW_SET_270GBPS);
                if (Status != XST_SUCCESS) {
                        return XDPTX_TS_FAILURE;
                }

                return XDPTX_TS_CLOCK_RECOVERY;
        case XDPTX_LINK_BW_SET_270GBPS:
                Status = XDptx_SetLinkRate(InstancePtr,
                                                XDPTX_LINK_BW_SET_162GBPS);
                if (Status != XST_SUCCESS) {
                        return XDPTX_TS_FAILURE;
                }

                return XDPTX_TS_CLOCK_RECOVERY;
        default:
                /* Already at the lowest link rate. Try reducing the lane
                 * count next. */
                break;
        }

        return XDPTX_TS_ADJUST_LANE_COUNT;
}

/******************************************************************************/
/**
 * This function is reached if either the clock recovery or the channel
 * equalization process failed during training, and a minimal data rate of 1.62
 * Gbps was being used. As a result, the number of lanes in use will be reduced,
 * and training will be re-attempted (starting with clock recovery) at this
 * lower lane count.
 *
 * @note        Training will be re-attempted with the maximum data rate being
 *              used with the reduced lane count to train at the main link at
 *              the maximum bandwidth possible.
 *
 * @param       InstancePtr is a pointer to the XDptx instance.
 *
 * @return      The next training state:
 *              - XDPTX_TS_FAILURE if only one lane is already in use.
 *              - XDPTX_TS_CLOCK_RECOVERY otherwise. Re-attempt training.
 *
*******************************************************************************/
static XDptx_TrainingState XDptx_TrainingStateAdjustLaneCount(
                                                        XDptx *InstancePtr)
{
        u32 Status;
        XDptx_LinkConfig *LinkConfig = &InstancePtr->LinkConfig;

        /* Verify arguments. */
        Xil_AssertNonvoid(InstancePtr != NULL);
        Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

        switch (LinkConfig->LaneCount) {
        case 4:
                Status = XDptx_SetLaneCount(InstancePtr, 2);
                if (Status != XST_SUCCESS) {
                        return XDPTX_TS_FAILURE;
                }

                Status = XDptx_SetLinkRate(InstancePtr,
                                                LinkConfig->MaxLinkRate);
                if (Status != XST_SUCCESS) {
                        return XDPTX_TS_FAILURE;
                }

                return XDPTX_TS_CLOCK_RECOVERY;
        case 2:
                Status = XDptx_SetLaneCount(InstancePtr, 1);
                if (Status != XST_SUCCESS) {
                        return XDPTX_TS_FAILURE;
                }

                Status = XDptx_SetLinkRate(InstancePtr,
                                                LinkConfig->MaxLinkRate);
                if (Status != XST_SUCCESS) {
                        return XDPTX_TS_FAILURE;
                }

                return XDPTX_TS_CLOCK_RECOVERY;
        default:
                /* Already at the lowest lane count. Training has failed at the
                 * lowest lane count and link rate. */
                break;
        }

        return XDPTX_TS_FAILURE;
}

/******************************************************************************/
/**
 * This function will do a burst AUX read from the receiver over the AUX
 * channel. The contents of the status registers will be stored for later use by
 * XDptx_CheckClockRecovery, XDptx_CheckChannelEqualization, and
 * XDptx_AdjVswingPreemp.
 *
 * @param       InstancePtr is a pointer to the XDptx instance.
 *
 * @return
 *              - XST_SUCCESS if the AUX read was successful.
 *              - XST_FAILURE otherwise.
 *
*******************************************************************************/
static u32 XDptx_GetLaneStatusAdjReqs(XDptx *InstancePtr)
{
        u32 Status;

        /* Verify arguments. */
        Xil_AssertNonvoid(InstancePtr != NULL);
        Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

        /* Read and store 4 bytes of lane status and 2 bytes of adjustment
         * requests. */
        Status = XDptx_AuxRead(InstancePtr, XDPTX_DPCD_STATUS_LANE_0_1,
                                6, InstancePtr->RxConfig.LaneStatusAdjReqs);
        if (Status != XST_SUCCESS) {
                return XST_FAILURE;
        }

        return XST_SUCCESS;
}

/******************************************************************************/
/**
 * This function checks if the receiver's DPCD indicates that the clock recovery
 * sequence during link training was successful - the receiver's link clock and
 * data recovery unit has realized and maintained the frequency lock for all
 * lanes currently in use.
 *
 * @param       InstancePtr is a pointer to the XDptx instance.
 * @param       LaneCount is the number of lanes to check.
 *
 * @return
 *              - XST_SUCCESS if the receiver's clock recovery PLL has achieved
 *                frequency lock for all lanes in use.
 *              - XST_FAILURE otherwise.
 *
*******************************************************************************/
static u32 XDptx_CheckClockRecovery(XDptx *InstancePtr, u8 LaneCount)
{
        u32 Status;
        u8 AuxData[6];

        u8 *LaneStatus = InstancePtr->RxConfig.LaneStatusAdjReqs;

        /* Verify arguments. */
        Xil_AssertNonvoid(InstancePtr != NULL);
        Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
        Xil_AssertNonvoid(LaneStatus != NULL);

        /* Check that all LANEx_CR_DONE bits are set. */
        switch (LaneCount) {
        case 4:
                if (!(LaneStatus[1] &
                                XDPTX_DPCD_STATUS_LANE_3_CR_DONE_MASK)) {
                        return XST_FAILURE;
                }
                if (!(LaneStatus[1] &
                                XDPTX_DPCD_STATUS_LANE_2_CR_DONE_MASK)) {
                        return XST_FAILURE;
                }
                /* Drop through and check lane 1. */
        case 2:
                if (!(LaneStatus[0] &
                                XDPTX_DPCD_STATUS_LANE_1_CR_DONE_MASK)) {
                        return XST_FAILURE;
                }
                /* Drop through and check lane 0. */
        case 1:
                if (!(LaneStatus[0] &
                                XDPTX_DPCD_STATUS_LANE_0_CR_DONE_MASK)) {
                        return XST_FAILURE;
                }
        default:
                /* All (LaneCount) lanes have achieved clock recovery. */
                break;
        }

        return XST_SUCCESS;
}

/******************************************************************************/
/**
 * This function checks if the receiver's DPCD indicates that the channel
 * equalization sequence during link training was successful - the receiver has
 * achieved channel equalization, symbol lock, and interlane alignment for all
 * lanes currently in use.
 *
 * @param       InstancePtr is a pointer to the XDptx instance.
 * @param       LaneCount is the number of lanes to check.
 *
 * @return
 *              - XST_SUCCESS if the receiver has achieved channel equalization
 *                symbol lock, and interlane alignment for all lanes in use.
 *              - XST_FAILURE otherwise.
 *
*******************************************************************************/
static u32 XDptx_CheckChannelEqualization(XDptx *InstancePtr, u8 LaneCount)
{
        u32 Status;
        u8 AuxData[6];
        u8 *LaneStatus = InstancePtr->RxConfig.LaneStatusAdjReqs;

        /* Verify arguments. */
        Xil_AssertNonvoid(InstancePtr != NULL);
        Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
        Xil_AssertNonvoid(LaneStatus != NULL);

        /* Check that all LANEx_CHANNEL_EQ_DONE bits are set. */
        switch (LaneCount) {
        case 4:
                if (!(LaneStatus[1] &
                                XDPTX_DPCD_STATUS_LANE_3_CE_DONE_MASK)) {
                        return XST_FAILURE;
                }
                if (!(LaneStatus[1] &
                                XDPTX_DPCD_STATUS_LANE_2_CE_DONE_MASK)) {
                        return XST_FAILURE;
                }
                /* Drop through and check lane 1. */
        case 2:
                if (!(LaneStatus[0] &
                                XDPTX_DPCD_STATUS_LANE_1_CE_DONE_MASK)) {
                        return XST_FAILURE;
                }
                /* Drop through and check lane 0. */
        case 1:
                if (!(LaneStatus[0] &
                                XDPTX_DPCD_STATUS_LANE_0_CE_DONE_MASK)) {
                        return XST_FAILURE;
                }
        default:
                /* All (LaneCount) lanes have achieved channel equalization. */
                break;
        }

        /* Check that all LANEx_SYMBOL_LOCKED bits are set. */
        switch (LaneCount) {
        case 4:
                if (!(LaneStatus[1] &
                                XDPTX_DPCD_STATUS_LANE_3_SL_DONE_MASK)) {
                        return XST_FAILURE;
                }
                if (!(LaneStatus[1] &
                                XDPTX_DPCD_STATUS_LANE_2_SL_DONE_MASK)) {
                        return XST_FAILURE;
                }
                /* Drop through and check lane 1. */
        case 2:
                if (!(LaneStatus[0] &
                                XDPTX_DPCD_STATUS_LANE_1_SL_DONE_MASK)) {
                        return XST_FAILURE;
                }
                /* Drop through and check lane 0. */
        case 1:
                if (!(LaneStatus[0] &
                                XDPTX_DPCD_STATUS_LANE_0_SL_DONE_MASK)) {
                        return XST_FAILURE;
                }
        default:
                /* All (LaneCount) lanes have achieved symbol lock. */
                break;
        }

        /* Check that interlane alignment is done. */
        if (!(LaneStatus[2] &
                        XDPTX_DPCD_LANE_ALIGN_STATUS_UPDATED_IA_DONE_MASK)) {
                return XST_FAILURE;
        }

        return XST_SUCCESS;
}

/******************************************************************************/
/**
 * This function sets current voltage swing and pre-emphasis level settings from
 * the LinkConfig structure to hardware.
 *
 * @param       InstancePtr is a pointer to the XDptx instance.
 *
 * @return
 *              - XST_SUCCESS if writing the settings was successful.
 *              - XST_FAILURE otherwise.
 *
*******************************************************************************/
static u32 XDptx_SetVswingPreemp(XDptx *InstancePtr)
{
        u32 Status;
        u8 Data;
        u8 AuxData[4];
        u8 Index;
        u8 VsLevelRx = InstancePtr->LinkConfig.VsLevel;
        u8 PeLevelRx = InstancePtr->LinkConfig.PeLevel;
        u32 VsLevel;
        u32 PeLevel;
        u32 VsLevels[4] = {XDPTX_VS_LEVEL_0, XDPTX_VS_LEVEL_1,
                                        XDPTX_VS_LEVEL_2, XDPTX_VS_LEVEL_3};
        u32 PeLevels[4] = {XDPTX_PE_LEVEL_0, XDPTX_PE_LEVEL_1,
                                        XDPTX_PE_LEVEL_2, XDPTX_PE_LEVEL_3};

        /* Verify arguments. */
        Xil_AssertNonvoid(InstancePtr != NULL);
        Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

        if (InstancePtr->HasRedriverInPath == 0) {
                PeLevel = PeLevels[PeLevelRx];
                VsLevel = VsLevels[VsLevelRx];

                /* Need to compensate due to lack of redriver. */
                if (PeLevelRx != 0) {
                        VsLevel += XDPTX_VS_LEVEL_OFFSET;
                }
        }
        else {
                /* No need to compensate since redriver does that. Can evenly
                 * disperse the voltage swing and pre-emphasis levels. */

                /* Map 16 possible voltage swing levels in TX to 4 in RX. */
                VsLevel = VsLevelRx * 4 + 2;
                /* Map 32 possible pre-emphasis levels in TX to 4 in RX. */
                PeLevel = PeLevelRx * 8 + 4;
        }

        /* Set up the data buffer for writing to the sink device. */
        Data = (PeLevelRx << XDPTX_DPCD_TRAINING_LANEX_SET_PE_SHIFT) |
                                                                VsLevelRx;
        /* The maximum voltage swing has been reached. */
        if (VsLevelRx == XDPTX_MAXIMUM_VS_LEVEL) {
                Data |= XDPTX_DPCD_TRAINING_LANEX_SET_MAX_VS_MASK;
        }
        /* The maximum pre-emphasis level has been reached. */
        if (PeLevelRx == XDPTX_MAXIMUM_PE_LEVEL) {
                Data |= XDPTX_DPCD_TRAINING_LANEX_SET_MAX_PE_MASK;
        }
        memset(AuxData, Data, InstancePtr->LinkConfig.LaneCount);

        for (Index = 0; Index < InstancePtr->LinkConfig.LaneCount; Index++) {
                /* Disable pre-cursor levels. */
                XDptx_WriteReg(InstancePtr->TxConfig.BaseAddr,
                        XDPTX_PHY_PRECURSOR_LANE_0 + 4 * Index, 0);

                /* Write new voltage swing levels to the TX registers. */
                XDptx_WriteReg(InstancePtr->TxConfig.BaseAddr,
                        XDPTX_PHY_VOLTAGE_DIFF_LANE_0 + 4 * Index, VsLevel);

                /* Write new pre-emphasis levels to the TX registers. */
                XDptx_WriteReg(InstancePtr->TxConfig.BaseAddr,
                        XDPTX_PHY_POSTCURSOR_LANE_0 + 4 * Index, PeLevel);
        }

        /* Write the voltage swing and pre-emphasis levels for each lane to the
         * sink device. */
        Status = XDptx_AuxWrite(InstancePtr, XDPTX_DPCD_TRAINING_LANE0_SET,
                                InstancePtr->LinkConfig.LaneCount, AuxData);
        if (Status != XST_SUCCESS) {
                return XST_FAILURE;
        }

        return XST_SUCCESS;
}

/******************************************************************************/
/**
 * This function obtains adjustment requests for voltage swing and pre-emphasis
 * levels from the receiver and sets these new settings.
 *
 * @param       InstancePtr is a pointer to the XDptx instance.
 *
 * @return
 *              - XST_SUCCESS if the adjustment request from the receiver and
 *                the new settings were written successfully.
 *              - XST_FAILURE otherwise (an AUX transaction failed).
 *
*******************************************************************************/
static u32 XDptx_AdjVswingPreemp(XDptx *InstancePtr)
{
        u32 Status;
        u8 Index;
        u8 VsLevelAdjReq[4];
        u8 PeLevelAdjReq[4];
        u8 *AdjReqs = &InstancePtr->RxConfig.LaneStatusAdjReqs[4];

        /* Verify arguments. */
        Xil_AssertNonvoid(InstancePtr != NULL);
        Xil_AssertNonvoid(InstancePtr != NULL);
        Xil_AssertNonvoid(AdjReqs != NULL);

        /* Analyze the adjustment requests for changes in voltage swing and
         * pre-emphasis levels. */
        VsLevelAdjReq[0] = AdjReqs[0] & XDPTX_DPCD_ADJ_REQ_LANE_0_2_VS_MASK;
        VsLevelAdjReq[1] = (AdjReqs[0] & XDPTX_DPCD_ADJ_REQ_LANE_1_3_VS_MASK) >>
                                        XDPTX_DPCD_ADJ_REQ_LANE_1_3_VS_SHIFT;
        VsLevelAdjReq[2] = AdjReqs[1] & XDPTX_DPCD_ADJ_REQ_LANE_0_2_VS_MASK;
        VsLevelAdjReq[3] = (AdjReqs[1] & XDPTX_DPCD_ADJ_REQ_LANE_1_3_VS_MASK) >>
                                        XDPTX_DPCD_ADJ_REQ_LANE_1_3_VS_SHIFT;
        PeLevelAdjReq[0] = (AdjReqs[0] & XDPTX_DPCD_ADJ_REQ_LANE_0_2_PE_MASK) >>
                                        XDPTX_DPCD_ADJ_REQ_LANE_0_2_PE_SHIFT;
        PeLevelAdjReq[1] = (AdjReqs[0] & XDPTX_DPCD_ADJ_REQ_LANE_1_3_PE_MASK) >>
                                        XDPTX_DPCD_ADJ_REQ_LANE_1_3_PE_SHIFT;
        PeLevelAdjReq[2] = (AdjReqs[1] & XDPTX_DPCD_ADJ_REQ_LANE_0_2_PE_MASK) >>
                                        XDPTX_DPCD_ADJ_REQ_LANE_0_2_PE_SHIFT;
        PeLevelAdjReq[3] = (AdjReqs[1] & XDPTX_DPCD_ADJ_REQ_LANE_1_3_PE_MASK) >>
                                        XDPTX_DPCD_ADJ_REQ_LANE_1_3_PE_SHIFT;

        /* Change the drive settings to match the adjustment requests. Use the
         * greatest level requested. */
        InstancePtr->LinkConfig.VsLevel = 0;
        InstancePtr->LinkConfig.PeLevel = 0;
        for (Index = 0; Index < InstancePtr->LinkConfig.LaneCount; Index++) {
                if (VsLevelAdjReq[Index] >InstancePtr->LinkConfig.VsLevel) {
                        InstancePtr->LinkConfig.VsLevel = VsLevelAdjReq[Index];
                }
                if (PeLevelAdjReq[Index] >InstancePtr->LinkConfig.PeLevel) {
                        InstancePtr->LinkConfig.PeLevel = PeLevelAdjReq[Index];
                }
        }

        /* Verify that the voltage swing and pre-emphasis combination is
         * allowed. Some combinations will result in a differential peak-to-peak
         * voltage that is outside the permissable range. See the VESA
         * DisplayPort v1.2 Specification, section 3.1.5.2.
         * The valid combinations are:
         *      PE=0    PE=1    PE=2    PE=3
         * VS=0 Valid   Valid   Valid   Valid
         * VS=1 Valid   Valid   Valid
         * VS=2 Valid   Valid
         * VS=3 Valid
         */
        if (InstancePtr->LinkConfig.PeLevel >
                                        (4 - InstancePtr->LinkConfig.VsLevel)) {
                InstancePtr->LinkConfig.PeLevel =
                                        4 - InstancePtr->LinkConfig.VsLevel;
        }

        /* Make the adjustments to both the transmitter and the sink device. */
        Status = XDptx_SetVswingPreemp(InstancePtr);
        if (Status != XST_SUCCESS) {
                return XST_FAILURE;
        }

        return XST_SUCCESS;
}

/******************************************************************************/
/**
 * This function sets the training pattern to be used during link training for
 * both the DisplayPort TX core and the sink device.
 *
 * @param       InstancePtr is a pointer to the XDptx instance.
 * @param       Pattern selects the pattern to be used. One of the following:
 *              - XDPTX_TRAINING_PATTERN_SET_OFF
 *              - XDPTX_TRAINING_PATTERN_SET_TP1
 *              - XDPTX_TRAINING_PATTERN_SET_TP2
 *              - XDPTX_TRAINING_PATTERN_SET_TP3
 *
 * @return
 *              - XST_SUCCESS if setting the pattern was successful.
 *              - XST_INVALID_PARAM if the supplied pattern select isn't valid.
 *              - XST_FAILURE otherwise.
 *
*******************************************************************************/
static u32 XDptx_SetTrainingPattern(XDptx *InstancePtr, u32 Pattern)
{
        u32 Status;
        u8 RegVal;

        /* Verify arguments. */
        Xil_AssertNonvoid(InstancePtr != NULL);
        Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

        switch (Pattern) {
        case XDPTX_TRAINING_PATTERN_SET_OFF:
        case XDPTX_TRAINING_PATTERN_SET_TP1:
        case XDPTX_TRAINING_PATTERN_SET_TP2:
        case XDPTX_TRAINING_PATTERN_SET_TP3:
                break;
        default:
                return XST_INVALID_PARAM;
        }

        /* Write to the transmitter. */
        XDptx_WriteReg(InstancePtr->TxConfig.BaseAddr,
                                        XDPTX_TRAINING_PATTERN_SET, Pattern);

        /* Preserve the current receiver settings. */
        Status = XDptx_AuxRead(InstancePtr, XDPTX_DPCD_TP_SET, 1, &RegVal);
                        RegVal &= ~XDPTX_DPCD_TP_SEL_MASK;
                        RegVal |= (Pattern & XDPTX_DPCD_TP_SEL_MASK);
        if (Status != XST_SUCCESS) {
                return XST_FAILURE;
        }

        /* Write to the sink device. */
        Status = XDptx_AuxWrite(InstancePtr, XDPTX_DPCD_TP_SET, 1,
                                                                &RegVal);
        if (Status != XST_SUCCESS) {
                return XST_FAILURE;
        }

        return XST_SUCCESS;
}

/******************************************************************************/
/**
 * This function determines what the receiver's required training delay is for
 * link training.
 *
 * @param       InstancePtr is a pointer to the XDptx instance.
 * @param       TrainingState is the current training state; either clock
 *              recovery or channel equalization.
 *
 * @return      The training delay specified in the receiver's DPCD register,
 *              XDPTX_DPCD_TRAIN_AUX_RD_INTERVAL.
 *
*******************************************************************************/
static u32 XDptx_GetTrainingDelay(XDptx *InstancePtr,
                                        XDptx_TrainingState TrainingState)
{
        u8 *Dpcd = InstancePtr->RxConfig.DpcdRxCapsField;

        /* Verify arguments. */
        Xil_AssertNonvoid(InstancePtr != NULL);
        Xil_AssertNonvoid(Dpcd != NULL);

        switch (Dpcd[XDPTX_DPCD_TRAIN_AUX_RD_INTERVAL]) {
        case XDPTX_DPCD_TRAIN_AUX_RD_INT_100_400US:
                if (TrainingState == XDPTX_TS_CLOCK_RECOVERY) {
                        /* Delay for the clock recovery phase. */
                        return 100;
                }
                else {
                        /* Delay for the channel equalization phase. */
                        return 400;
                }
        case XDPTX_DPCD_TRAIN_AUX_RD_INT_4MS:
                return 4000;
        case XDPTX_DPCD_TRAIN_AUX_RD_INT_8MS:
                return 8000;
        case XDPTX_DPCD_TRAIN_AUX_RD_INT_12MS:
                return 12000;
        case XDPTX_DPCD_TRAIN_AUX_RD_INT_16MS:
                return 16000;
        default:
                break;
        }

        /* A DPCD register value corresponding with an unknown delay should
         * default to 20 ms. */
        return 20000;
}

static u32 XDptx_AuxCommon(XDptx *InstancePtr, u32 CmdType, u32 Address,
                                                        u32 NumBytes, u8 *Data)
{
        u32 Status;
        XDptx_AuxTransaction Request;
        u32 BytesLeft;

        /* Set the start address for AUX transactions. For I2C transactions,
         * this is the address of the I2C bus. */
        Request.Address = Address;

        BytesLeft = NumBytes;
        while (BytesLeft > 0) {
                Request.CmdCode = CmdType;

                if ((CmdType == XDPTX_AUX_CMD_READ) ||
                                        (CmdType == XDPTX_AUX_CMD_WRITE)) {
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

                if ((CmdType == XDPTX_AUX_CMD_I2C_READ) && (BytesLeft > 0)) {
                        /* Middle of a transaction I2C read request. Override
                         * the command code that was set to CmdType. */
                        Request.CmdCode = XDPTX_AUX_CMD_I2C_READ_MOT;
                }
                else if ((CmdType == XDPTX_AUX_CMD_I2C_WRITE) &&
                                                        (BytesLeft > 0)) {
                        /* Middle of a transaction I2C write request. Override
                         * the command code that was set to CmdType. */
                        Request.CmdCode = XDPTX_AUX_CMD_I2C_WRITE_MOT;
                }

                Status = XDptx_AuxRequest(InstancePtr, &Request);
                if (Status != XST_SUCCESS) {
                        return Status;
                }
        }

        return XST_SUCCESS;
}

/******************************************************************************/
/**
 * This function submits the supplied AUX request to the sink device over the
 * AUX channel. If waiting for a reply times out, or if the DisplayPort TX core
 * indicates that the request was deferred, the request is sent again(up to a
 * maximum specified by XDPTX_AUX_MAX_DEFER_COUNT|XDPTX_AUX_MAX_TIMEOUT_COUNT).
 *
 * @param       InstancePtr is a pointer to the XDptx instance.
 * @param       Request is a pointer to an initialized XDptx_AuxTransaction
 *              structure containing the required information for issuing an
 *              AUX command, as well as a write buffer used for write commands,
 *              and a read buffer for read commands.
 *
 * @return
 *              - XST_SUCCESS if the request was acknowledged.
 *              - XST_ERROR_COUNT_MAX if resending the request exceeded the
 *                maximum for deferral and timeout.
 *              - XST_FAILURE otherwise (if the transmitter sees a NACK
 *                reply code or if the AUX transaction failed).
 *
*******************************************************************************/
static u32 XDptx_AuxRequest(XDptx *InstancePtr, XDptx_AuxTransaction *Request)
{
        u32 Status;
        u32 DeferCount = 0;
        u32 TimeoutCount = 0;

        /* Verify arguments. */
        Xil_AssertNonvoid(InstancePtr != NULL);
        Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
        Xil_AssertNonvoid(Request != NULL);

        while ((DeferCount < XDPTX_AUX_MAX_DEFER_COUNT) &&
                                (TimeoutCount < XDPTX_AUX_MAX_TIMEOUT_COUNT)) {
                Status = XDptx_AuxWaitReady(InstancePtr);
                if (Status != XST_SUCCESS) {
                        /* The receiver isn't ready yet. */
                        TimeoutCount++;
                        continue;
                }

                /* Send the request. */
                Status = XDptx_AuxRequestSend(InstancePtr, Request);
                switch (Status) {
                case XST_SEND_ERROR:
                        /* The request was deferred. */
                        DeferCount++;
                        break;
                case XST_ERROR_COUNT_MAX:
                        /* Waiting for a reply timed out. */
                        TimeoutCount++;
                        break;
                case XST_FAILURE:
                        /* The request was NACK'ed. */
                        return XST_FAILURE;
                default:
                        /* The request was ACK'ed. */
                        return XST_SUCCESS;
                }

                XDptx_WaitUs(InstancePtr, 100);
        }

        /* The request was not successfully received by the sink device. */
        return XST_ERROR_COUNT_MAX;
}

/******************************************************************************/
/**
 * This function submits the supplied AUX request to the sink device over the
 * AUX channel by writing the command, the destination sink address, (the write
 * buffer for write commands), and the data size to the DisplayPort TX core.
 *
 * @param       InstancePtr is a pointer to the XDptx instance.
 * @param       Request is a pointer to an initialized XDptx_AuxTransaction
 *              structure containing the required information for issuing an AUX
 *              command. 
 *
 * @return
 *              - XST_SUCCESS if the request was acknowledged.
 *              - XST_ERROR_COUNT_MAX if waiting for a reply timed out.
 *              - XST_SEND_ERROR if the request was deferred.
 *              - XST_FAILURE otherwise, if the request was NACK'ed.
 *
*******************************************************************************/
static u32 XDptx_AuxRequestSend(XDptx *InstancePtr,
                                                XDptx_AuxTransaction *Request)
{
        u32 Status;
        u8 Index;

        /* Set the address for the request. */
        XDptx_WriteReg(InstancePtr->TxConfig.BaseAddr, XDPTX_AUX_ADDRESS,
                                                        Request->Address);

        switch (Request->CmdCode) {
        case XDPTX_AUX_CMD_WRITE:
        case XDPTX_AUX_CMD_I2C_WRITE:
        case XDPTX_AUX_CMD_I2C_WRITE_MOT:
                /* Feed write data into the transmitter FIFO. */
                for (Index = 0; Index < Request->NumBytes; Index++) {
                        XDptx_WriteReg(InstancePtr->TxConfig.BaseAddr,
                                                XDPTX_AUX_WRITE_FIFO,
                                                Request->Data[Index]);
                }
        default:
                /* Not a write command. */
                break;
        }

        /* Submit the command and the data size. */
        XDptx_WriteReg(InstancePtr->TxConfig.BaseAddr, XDPTX_AUX_CMD,
                                ((Request->CmdCode << XDPTX_AUX_CMD_SHIFT) |
                                ((Request->NumBytes - 1) &
                                XDPTX_AUX_CMD_NBYTES_TRANSFER_MASK)));

        /* Check for a receiver reply to the submitted request. */
        Status = XDptx_AuxWaitReply(InstancePtr);
        if (Status != XST_SUCCESS) {
                /* Waiting for a reply timed out. */
                return XST_ERROR_COUNT_MAX;
        }

        /* Analyze the reply. */
        Status = XDptx_ReadReg(InstancePtr->TxConfig.BaseAddr,
                                                        XDPTX_AUX_REPLY_CODE);
        switch (Status) {
        case XDPTX_AUX_REPLY_CODE_DEFER:
        case XDPTX_AUX_REPLY_CODE_I2C_DEFER:
                /* The request was deferred. */
                return XST_SEND_ERROR;
        case XDPTX_AUX_REPLY_CODE_NACK:
        case XDPTX_AUX_REPLY_CODE_I2C_NACK:
                /* The request was not acknowledged. */
                return XST_FAILURE;
        default:
                /* The request was acknowledged. */
                break;
        }

        switch (Request->CmdCode) {
        case XDPTX_AUX_CMD_READ:
        case XDPTX_AUX_CMD_I2C_READ:
        case XDPTX_AUX_CMD_I2C_READ_MOT:
                /* Obtain the read data from the reply FIFO. */
                for (Index = 0; Index < Request->NumBytes; Index++) {
                        Request->Data[Index] = XDptx_ReadReg(
                                                InstancePtr->TxConfig.BaseAddr,
                                                XDPTX_AUX_REPLY_DATA);
                }
        default:
                /* Not a read command. */
                break;
        }

        return XST_SUCCESS;
}

/******************************************************************************/
/**
 * This function waits for a reply indicating that the most recent AUX request
 * has been received by the sink device.
 *
 * @param       InstancePtr is a pointer to the XDptx instance.
 *
 * @return
 *              - XST_SUCCESS if a reply from the sink device was received.
 *              - XST_ERROR_COUNT_MAX otherwise, if a timeout has occurred.
 *
*******************************************************************************/
static u32 XDptx_AuxWaitReply(XDptx *InstancePtr)
{
        u32 Timeout = 100;
        u32 Status;

        /* Verify arguments. */
        Xil_AssertNonvoid(InstancePtr != NULL);
        Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

        while (0 < Timeout) {
                Status = XDptx_ReadReg(InstancePtr->TxConfig.BaseAddr,
                                                XDPTX_INTERRUPT_STATUS);

                /* Check for a timeout. */
                if (Status & XDPTX_INTERRUPT_STATUS_REPLY_TIMEOUT_MASK) {
                        return XST_ERROR_COUNT_MAX;
                }

                /* Check for a reply. */
                if (Status & XDPTX_INTERRUPT_STATUS_REPLY_RECEIVED_MASK) {
                        return XST_SUCCESS;
                }

                Timeout--;
                XDptx_WaitUs(InstancePtr, 20);
        }

        return XST_ERROR_COUNT_MAX;
}

/******************************************************************************/
/**
 * This function waits until another request is no longer in progress.
 *
 * @param       InstancePtr is a pointer to the XDptx instance.
 *
 * @return
 *              - XST_SUCCESS if the the receiver is no longer busy.
 *              - XST_ERROR_COUNT_MAX otherwise, if a timeout has occurred.
 *
*******************************************************************************/
static u32 XDptx_AuxWaitReady(XDptx *InstancePtr)
{
        u32 Status;
        u32 Timeout = 100;

        /* Wait until the transmitter is ready. */
        do {
                Status = XDptx_ReadReg(InstancePtr->TxConfig.BaseAddr,
                                        XDPTX_INTERRUPT_SIG_STATE);

                /* Protect against an infinite loop. */
                if (!Timeout--) {
                        return XST_ERROR_COUNT_MAX;
                }
                XDptx_WaitUs(InstancePtr, 20);
        }
        while (Status & XDPTX_REPLY_STATUS_REPLY_IN_PROGRESS_MASK);

        return XST_SUCCESS;
}

/******************************************************************************/
/**
 * This function sets the clock frequency for the DisplayPort PHY corresponding
 * to a desired data rate.
 *
 * @param       InstancePtr is a pointer to the XDptx instance.
 * @param       Speed determines what clock frequency will be used based on one
 *              of the following selects:
 *              - XDPTX_PHY_CLOCK_SELECT_162GBPS = 0x01
 *              - XDPTX_PHY_CLOCK_SELECT_270GBPS = 0x03
 *              - XDPTX_PHY_CLOCK_SELECT_540GBPS = 0x05
 *
 * @return
 *              - XST_SUCCESS if the reset for each lane is done after the clock
 *                frequency has been set.
 *              - XST_INVALID_PARAM if the clock frequency doesn't correspond to
 *                an associated 1.62, 2.70, or 5.40 Gbps link rate.
 *              - XST_FAILURE otherwise.
 *
*******************************************************************************/
static u32 XDptx_SetClkSpeed(XDptx *InstancePtr, u32 Speed)
{
        u32 Status;
        u32 RegVal;

        /* Verify arguments. */
        Xil_AssertNonvoid(InstancePtr != NULL);
        Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

        if (Speed != XDPTX_PHY_CLOCK_SELECT_162GBPS &&
                                Speed != XDPTX_PHY_CLOCK_SELECT_270GBPS &&
                                Speed != XDPTX_PHY_CLOCK_SELECT_540GBPS) {
                return XST_INVALID_PARAM;
        }

        /* Disable the transmitter first. */
        RegVal = XDptx_ReadReg(InstancePtr->TxConfig.BaseAddr, XDPTX_ENABLE);
        XDptx_WriteReg(InstancePtr->TxConfig.BaseAddr, XDPTX_ENABLE, 0);

        /* Change speed of the feedback clock. */
        XDptx_WriteReg(InstancePtr->TxConfig.BaseAddr,
                                                XDPTX_PHY_CLOCK_SELECT, Speed);

        /* Re-enable the transmitter if it was previously. */
        if (RegVal != 0) {
                XDptx_WriteReg(InstancePtr->TxConfig.BaseAddr, XDPTX_ENABLE, 1);
        }

        /* Wait until the PHY is ready. */
        Status = XDptx_WaitPhyReady(InstancePtr);
        if (Status != XST_SUCCESS) {
                return XST_FAILURE;
        }

        return XST_SUCCESS;
}

/******************************************************************************/
/**
 * This function waits for the DisplayPort PHY to come out of reset.
 *
 * @param       InstancePtr is a pointer to the XDptx instance.
 *
 * @return
 *              - XST_ERROR_COUNT_MAX if the PHY failed to be ready.
 *              - XST_SUCCESS otherwise.
 *
*******************************************************************************/
static u32 XDptx_WaitPhyReady(XDptx *InstancePtr)
{
        u32 Timeout = 100;
        u32 PhyStatus;

        /* Verify arguments. */
        Xil_AssertNonvoid(InstancePtr != NULL);
        Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

        /* Wait until the PHY is ready. */
        do {
                PhyStatus = XDptx_ReadReg(InstancePtr->TxConfig.BaseAddr,
                                        XDPTX_PHY_STATUS) &
                                        XDPTX_PHY_STATUS_ALL_LANES_READY_MASK;

                /* Protect against an infinite loop. */
                if (!Timeout--) {
                        return XST_ERROR_COUNT_MAX;
                }
                XDptx_WaitUs(InstancePtr, 20);
        }
        while (PhyStatus != XDPTX_PHY_STATUS_ALL_LANES_READY_MASK);

        return XST_SUCCESS;
}
