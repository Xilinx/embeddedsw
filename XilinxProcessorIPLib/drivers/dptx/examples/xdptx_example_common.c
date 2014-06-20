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
 * @file xdptx_example_common.c
 *
 * Contains a design example using the XDptx driver. It performs a self test on
 * the DisplayPort TX core by training the main link at the maximum common
 * capabilities between the TX and RX and checking the lane status.
 *
 * @note        The DisplayPort TX core does not work alone. Some platform
 *              initialization will need to happen prior to calling XDptx driver
 *              functions. See XAPP1178 as a reference.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- -----------------------------------------------
 * 1.00a als  06/17/14 Initial creation.
 * </pre>
 *
*******************************************************************************/

/******************************* Include Files ********************************/

#include "xdptx_example_common.h"
#include "xstatus.h"

/**************************** Function Prototypes *****************************/

static u32 Dptx_StartLink(XDptx *InstancePtr, u8 LaneCount, u8 LinkRate);
static void Dptx_StartVideoStream(XDptx *InstancePtr);

/**************************** Function Definitions ****************************/

u32 Dptx_Run(XDptx *InstancePtr, u8 LaneCount, u8 LinkRate)
{
        u32 Status;

        Status = Dptx_StartLink(InstancePtr, LaneCount, LinkRate);
        if (Status == XST_SUCCESS) {
                Dptx_StartVideoStream(InstancePtr);
        } else {
                xil_printf("<-- Failed to train.\n");
                return XST_FAILURE;
        }

        return XST_SUCCESS;
}

u32 Dptx_SetupExample(XDptx *InstancePtr, u16 DeviceId)
{
        XDptx_Config *ConfigPtr;
        u32 Status;

        /* Obtain the device configuration for the DisplayPort TX core. */
        ConfigPtr = XDptx_LookupConfig(DeviceId);
        if (!ConfigPtr) {
                return XST_FAILURE;
        }
        XDptx_CfgInitialize(InstancePtr, ConfigPtr, ConfigPtr->BaseAddr);

        /* Initialize the DisplayPort TX core. */
        Status = XDptx_InitializeTx(InstancePtr);
        if (Status != XST_SUCCESS) {
                return XST_FAILURE;
        }

        return XST_SUCCESS;
}

static u32 Dptx_StartLink(XDptx *InstancePtr, u8 LaneCount, u8 LinkRate)
{
        u32 VsLevelTx;
        u32 PeLevelTx;
        u32 Status;

        /* Obtain the capabilities of the sink by reading the monitor's DPCD. */
        Status = XDptx_GetSinkCapabilities(InstancePtr);
        if (Status != XST_SUCCESS) {
                return XST_FAILURE;
        }

#if defined(USE_MAX_LINK)
        Status = XDptx_CheckLinkStatus(InstancePtr,
                                        InstancePtr->LinkConfig.MaxLaneCount);
#else
        Status = XDptx_CheckLinkStatus(InstancePtr, LaneCount);
#endif
        if (Status == XST_SUCCESS) {
                xil_printf("-> Does not need training.\n");
                if (XDptx_ReadReg(InstancePtr->TxConfig.BaseAddr,
                        XDPTX_LINK_BW_SET) == LinkRate) {
                        return XST_SUCCESS;
                }
        }
        else if (Status == XST_FAILURE) {
                xil_printf("-> Needs training.\n");
        }
        else {
                xil_printf("-> Error checking link status.\n");
                return XST_FAILURE;
        }

#if defined(USE_MAX_LINK)
        /* Configure the main link based on the common capabilities of the
         * transmitter core and the sink monitor. */
        Status = XDptx_CfgMainLinkMax(InstancePtr);
        if (Status != XST_SUCCESS) {
                return XST_FAILURE;
        }
#else
        XDptx_SetLinkRate(InstancePtr, LinkRate);
        XDptx_SetLaneCount(InstancePtr, LaneCount);
        XDptx_SetEnhancedFrameMode(InstancePtr, 1);
        XDptx_SetDownspread(InstancePtr, 0);
#endif

        xil_printf("******************************************\n");
        Status = XDptx_EstablishLink(InstancePtr);
        if (Status != XST_SUCCESS) {
                xil_printf("!!! Training failed !!!\n");
                xil_printf("******************************************\n");
                return XST_FAILURE;
        }
        VsLevelTx = XDptx_ReadReg(InstancePtr->TxConfig.BaseAddr,
                                                XDPTX_PHY_VOLTAGE_DIFF_LANE_0);
        PeLevelTx = XDptx_ReadReg(InstancePtr->TxConfig.BaseAddr,
                                                XDPTX_PHY_POSTCURSOR_LANE_0);

        xil_printf("!!! Training passed at LR:0x%02lx LC:%d !!!\n",
                                        InstancePtr->LinkConfig.LinkRate,
                                        InstancePtr->LinkConfig.LaneCount);
        xil_printf("VS:%d (TX:%d) PE:%d (TX:%d)\n",
                                InstancePtr->LinkConfig.VsLevel, VsLevelTx,
                                InstancePtr->LinkConfig.PeLevel, PeLevelTx);
        xil_printf("******************************************\n");

        return XST_SUCCESS;
}

static void Dptx_StartVideoStream(XDptx *InstancePtr)
{
        u32 Status;
        u8 AuxData[1];

        /* Set the bits per color. If not set, the default is 6. */
        XDptx_CfgMsaSetBpc(InstancePtr, 8);

/* Choose a method for selecting the video mode. There are 3 ways to do this:
 * 1) Use the preferred timing from the monitor's EDID:
 *      XDptx_GetEdid(InstancePtr);
 *      XDptx_CfgMsaUseEdidPreferredTiming(InstancePtr);
 *
 * 2) Use a standard video timing mode (see mode_table.h):
 *      XDptx_CfgMsaUseStandardVideoMode(InstancePtr, XDPTX_VM_640x480_60_P);
 *
 * 3) Use a custom configuration for the main stream attributes:
 *      XDptx_MainStreamAttributes MsaConfigCustom;
 *      MsaConfigCustom.MVid = 108000;
 *      MsaConfigCustom.HSyncPolarity = 0;
 *      MsaConfigCustom.VSyncPolarity = 0;
 *      MsaConfigCustom.HSyncPulseWidth = 112;
 *      MsaConfigCustom.VSyncPulseWidth = 3;
 *      MsaConfigCustom.HResolution = 1280;
 *      MsaConfigCustom.VResolution = 1024;
 *      MsaConfigCustom.VBackPorch = 38;
 *      MsaConfigCustom.VFrontPorch = 1;
 *      MsaConfigCustom.HBackPorch = 248;
 *      MsaConfigCustom.HFrontPorch = 48;
 *      XDptx_CfgMsaUseCustom(InstancePtr, &MsaConfigCustom, 1);
 */
        Status = XDptx_GetEdid(InstancePtr);
        if (Status == XST_SUCCESS) {
                XDptx_CfgMsaUseEdidPreferredTiming(InstancePtr);
                XDptx_CfgMsaUseStandardVideoMode(InstancePtr,
                                                        XDPTX_VM_640x480_60_P);
        }
        else {
                XDptx_CfgMsaUseStandardVideoMode(InstancePtr,
                                                        XDPTX_VM_640x480_60_P);
        }

        /* Disable MST for this example. */
        AuxData[0] = 0;
        XDptx_AuxWrite(InstancePtr, XDPTX_DPCD_MSTM_CTRL, 1, AuxData);
        XDptx_WriteReg(InstancePtr->TxConfig.BaseAddr, XDPTX_TX_MST_CONFIG,
                                                                        0x0);

        /* Disable main stream to force sending of IDLE patterns. */
        XDptx_DisableMainLink(InstancePtr);

        /* Reset the transmitter. */
        XDptx_WriteReg(InstancePtr->TxConfig.BaseAddr, XDPTX_SOFT_RESET,
                                        XDPTX_SOFT_RESET_VIDEO_STREAM_ALL_MASK);
        XDptx_WriteReg(InstancePtr->TxConfig.BaseAddr, XDPTX_SOFT_RESET, 0x0);

        /* Configure video stream source or generator here. This function needs
         * to be implemented in order for video to be displayed and is hardware
         * system specific. It is up to the user to implement this function. */
        Dptx_ConfigureVidgen(InstancePtr);
        /*********************************/

        XDptx_EnableMainLink(InstancePtr);
}
