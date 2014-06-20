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
 * @file xdptx_spm.c
 *
 * This file contains the stream policy maker functions for the XDptx driver.
 * These functions set up the DisplayPort TX core's main stream attributes that
 * determine how a video stream will be displayed.
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
#include "xdptx_hw.h"
#include "xstatus.h"

/**************************** Function Prototypes *****************************/

static void XDptx_ClearMsaValues(XDptx *InstancePtr);
static void XDptx_SetMsaValues(XDptx *InstancePtr,
                                        XDptx_MainStreamAttributes *MsaConfig);

/**************************** Function Definitions ****************************/

/******************************************************************************/
/**
 * This function calculates the following main stream attributes:
 *      - Transfer unit size
 *      - User pixel width
 *      - NVid
 *      - Horizontal start
 *      - Vertical start
 *      - Horizontal total clock
 *      - Vertical total clock
 *      - Misc0
 *      - Misc1
 *      - Data per lane
 *      - Average number of bytes per transfer unit
 *      - Number of initial wait cycles
 * These values are derived from:
 *      - Bits per color
 *      - MVid
 *      - Horizontal sync polarity
 *      - Vertical sync polarity
 *      - Horizontal sync pulse width
 *      - Vertical sync pulse width
 *      - Horizontal resolution
 *      - Vertical resolution
 *      - Vertical back porch
 *      - Vertical front porch
 *      - Horizontal back porch
 *      - Horizontal front porch
 *
 * @param       InstancePtr is a pointer to the XDptx instance.
 *
 * @note        The MsaConfig structure is modified with the new, calculated
 *              values. The main stream attributes that were used to derive the
 *              calculated values are untouched in the MsaConfig structure.
 *
*******************************************************************************/
void XDptx_CfgMsaRecalculate(XDptx *InstancePtr)
{
        u32 VideoBw;
        u32 BitsPerPixel;
        XDptx_MainStreamAttributes *MsaConfig = &InstancePtr->MsaConfig;
        XDptx_LinkConfig *LinkConfig = &InstancePtr->LinkConfig;

        /* Verify arguments. */
        Xil_AssertVoid(InstancePtr != NULL);

        /* For SST. */
        MsaConfig->TransferUnitSize = 64;

        /* Set the user pixel width to handle clocks that exceed the
         * capabilities of the DisplayPort TX core. */
        if ((MsaConfig->MVid > 150000) && (LinkConfig->LaneCount == 4)) {
                MsaConfig->UserPixelWidth = 4;
        }
        else if ((MsaConfig->MVid > 80000) && (LinkConfig->LaneCount == 2)) {
                MsaConfig->UserPixelWidth = 2;
        }
        else {
                MsaConfig->UserPixelWidth = 1;
        }

        /* Compute the rest of the MSA values. */
        MsaConfig->NVid = (LinkConfig->LinkRate == XDPTX_LINK_BW_SET_540GBPS) ?
                                540000 :
                        (LinkConfig->LinkRate == XDPTX_LINK_BW_SET_270GBPS) ?
                                270000 :
                        162000;
        MsaConfig->HStart = MsaConfig->HSyncPulseWidth + MsaConfig->HBackPorch;
        MsaConfig->VStart = MsaConfig->VSyncPulseWidth + MsaConfig->VBackPorch;
        MsaConfig->HClkTotal = (MsaConfig->HSyncPulseWidth +
                                MsaConfig->HBackPorch + MsaConfig->HFrontPorch +
                                MsaConfig->HResolution);
        MsaConfig->VClkTotal = (MsaConfig->VSyncPulseWidth +
                                MsaConfig->VBackPorch + MsaConfig->VFrontPorch +
                                MsaConfig->VResolution);
        MsaConfig->Misc0 = (MsaConfig->BitsPerColor == 6) ? 0x00 :
                                (MsaConfig->BitsPerColor == 8) ? 0x01 :
                                (MsaConfig->BitsPerColor == 10) ? 0x02 :
                                (MsaConfig->BitsPerColor == 12) ? 0x03 :
                                (MsaConfig->BitsPerColor == 16) ? 0x04 :
                                0x00;
        MsaConfig->Misc0 = MsaConfig->Misc0 <<
                                        XDPTX_MAIN_STREAMX_MISC0_BDC_SHIFT;
        MsaConfig->Misc0 = MsaConfig->Misc0 | (LinkConfig->ComponentFormat <<
                        XDPTX_MAIN_STREAMX_MISC0_COMPONENT_FORMAT_SHIFT) |
                        (LinkConfig->DynamicRange <<
                        XDPTX_MAIN_STREAMX_MISC0_DYNAMIC_RANGE_SHIFT) |
                        (LinkConfig->YCbCrColorimetry <<
                        XDPTX_MAIN_STREAMX_MISC0_YCBCR_COLORIMETRY_SHIFT) |
                        (LinkConfig->SynchronousClockMode &
                        XDPTX_MAIN_STREAMX_MISC0_SYNC_CLK_MASK);
        MsaConfig->Misc1 = 0;
        MsaConfig->DataPerLane = (MsaConfig->HResolution *
                MsaConfig->BitsPerColor * 3 / 16) - LinkConfig->LaneCount;

        /* If RGB | YCbCr444, * 3 ; If YCbCr422, * 2 ; If YOnly, * 1. */
        BitsPerPixel = (LinkConfig->ComponentFormat == 1) ?
                        MsaConfig->BitsPerColor * 2 :
                        MsaConfig->BitsPerColor * 3;
        VideoBw = (MsaConfig->MVid * BitsPerPixel) / 8;

        MsaConfig->AvgBytesPerTU = (VideoBw * MsaConfig->TransferUnitSize) /
                        (LinkConfig->LaneCount * (MsaConfig->NVid / 1000));

        /* The number of initial wait cycles at the start of a new line by the
         * framing logic. This allows enough data to be buffered in the input
         * FIFO before video is sent. */
        MsaConfig->InitWait = (MsaConfig->TransferUnitSize -
                                        (MsaConfig->AvgBytesPerTU / 1000));
        if ((MsaConfig->AvgBytesPerTU / 1000) > MsaConfig->TransferUnitSize) {
                MsaConfig->InitWait = 0;
        }
        else if (MsaConfig->InitWait > 10) {
                MsaConfig->InitWait -= 10;
        }
        else {
                MsaConfig->InitWait = 0;
        }
}

/******************************************************************************/
/**
 * This function sets the main stream attribute values in the configuration
 * structure to match one of the standard display mode timings from the
 * XDptx_DmtModes[] table. THe XDptx_VideoMode enumeration in xdptx.h lists
 * the available video modes.
 *
 * @param       InstancePtr is a pointer to the XDptx instance.
 * @param       VideoMode is one of the enumerated standard video modes that is
 *              used to determine the main stream attributes to be used.
 *
 * @return
 *              - XST_INVALID_PARAM if the supplied video mode isn't in the DMT
 *                table.
 *              - XST_SUCCESS otherwise.
 *
 * @note        The InstancePtr->MsaConfig structure is modified to reflect the
 *              main stream attribute values associated to the specified video
 *              mode.
 *
*******************************************************************************/
u32 XDptx_CfgMsaUseStandardVideoMode(XDptx *InstancePtr,
                                                XDptx_VideoMode VideoMode)
{
        XDptx_MainStreamAttributes *MsaConfig = &InstancePtr->MsaConfig;

        /* Verify arguments. */
        Xil_AssertVoid(InstancePtr != NULL);

        if (VideoMode > XDPTX_VM_LAST) {
                return XST_INVALID_PARAM;
        }

        /* Configure the main stream attribute values from the display monitor
         * timing (DMT) table. */
        MsaConfig->MVid = XDptx_DmtModes[VideoMode].PixelClkKhz;
        MsaConfig->HSyncPolarity = XDptx_DmtModes[VideoMode].HSyncPolarity;
        MsaConfig->VSyncPolarity = XDptx_DmtModes[VideoMode].VSyncPolarity;
        MsaConfig->HSyncPulseWidth = XDptx_DmtModes[VideoMode].HSyncPulseWidth;
        MsaConfig->VSyncPulseWidth = XDptx_DmtModes[VideoMode].VSyncPulseWidth;
        MsaConfig->HResolution = XDptx_DmtModes[VideoMode].HResolution;
        MsaConfig->VResolution = XDptx_DmtModes[VideoMode].VResolution;
        MsaConfig->VBackPorch = XDptx_DmtModes[VideoMode].VBackPorch;
        MsaConfig->VFrontPorch = XDptx_DmtModes[VideoMode].VFrontPorch;
        MsaConfig->HBackPorch = XDptx_DmtModes[VideoMode].HBackPorch;
        MsaConfig->HFrontPorch = XDptx_DmtModes[VideoMode].HFrontPorch;

        /* Calculate the rest of the MSA values. */
        XDptx_CfgMsaRecalculate(InstancePtr);

        return XST_SUCCESS;
}

/******************************************************************************/
/**
 * This function sets the main stream attribute values in the configuration
 * structure to match the preferred timing of the sink monitor. This preferred
 * timing information is stored in the sink's extended display identification
 * data (EDID).
 *
 * @param       InstancePtr is a pointer to the XDptx instance
 *
 * @note        The InstancePtr->MsaConfig structure is modified to reflect the
 *              main stream attribute values associated to the preferred timing
 *              of the sink monitor.
 *
*******************************************************************************/
void XDptx_CfgMsaUseEdidPreferredTiming(XDptx *InstancePtr)
{
        XDptx_MainStreamAttributes *MsaConfig = &InstancePtr->MsaConfig;
        u8 *Ptm = &InstancePtr->RxConfig.Edid[XDPTX_EDID_PTM];

        /* Verify arguments. */
        Xil_AssertVoid(InstancePtr != NULL);

        /* Configure the MSA values with the preferred timing mode (PTM) as
         * specified by the preferred detailed timing descriptor of the
         * monitor's EDID.
         * Note, the PTM is only required for EDID versions 1.3 a newer. Earlier
         * versions may not contain this information. */
        u16 HBlank = ((Ptm[XDPTX_EDID_DTD_HRES_HBLANK_U4] &
                XDPTX_EDID_DTD_XRES_XBLANK_U4_XBLANK_MASK) << 8) |
                Ptm[XDPTX_EDID_DTD_HBLANK_LSB];

        u16 VBlank = ((Ptm[XDPTX_EDID_DTD_VRES_VBLANK_U4] &
                XDPTX_EDID_DTD_XRES_XBLANK_U4_XBLANK_MASK) << 8) |
                Ptm[XDPTX_EDID_DTD_VBLANK_LSB];

        MsaConfig->MVid = ((Ptm[XDPTX_EDID_DTD_PIXEL_CLK_KHZ_MSB] << 8) |
                Ptm[XDPTX_EDID_DTD_PIXEL_CLK_KHZ_LSB]) * 10;

        MsaConfig->HSyncPulseWidth =
                (((Ptm[XDPTX_EDID_DTD_XFPORCH_XSPW_U2] &
                XDPTX_EDID_DTD_XFPORCH_XSPW_U2_HSPW_MASK) >>
                XDPTX_EDID_DTD_XFPORCH_XSPW_U2_HSPW_SHIFT) << 8) |
                Ptm[XDPTX_EDID_DTD_HSPW_LSB];

        MsaConfig->VSyncPulseWidth =
                ((Ptm[XDPTX_EDID_DTD_XFPORCH_XSPW_U2] &
                XDPTX_EDID_DTD_XFPORCH_XSPW_U2_VSPW_MASK) << 8) |
                (Ptm[XDPTX_EDID_DTD_VFPORCH_VSPW_L4] &
                XDPTX_EDID_DTD_VFPORCH_VSPW_L4_VSPW_MASK);

        MsaConfig->HResolution =
                (((Ptm[XDPTX_EDID_DTD_HRES_HBLANK_U4] &
                XDPTX_EDID_DTD_XRES_XBLANK_U4_XRES_MASK) >>
                XDPTX_EDID_DTD_XRES_XBLANK_U4_XRES_SHIFT) << 8) |
                Ptm[XDPTX_EDID_DTD_HRES_LSB];

        MsaConfig->VResolution = (((Ptm[XDPTX_EDID_DTD_VRES_VBLANK_U4] &
                XDPTX_EDID_DTD_XRES_XBLANK_U4_XRES_MASK) >>
                XDPTX_EDID_DTD_XRES_XBLANK_U4_XRES_SHIFT) << 8) |
                Ptm[XDPTX_EDID_DTD_VRES_LSB];

        MsaConfig->VFrontPorch = (((Ptm[XDPTX_EDID_DTD_XFPORCH_XSPW_U2] &
                XDPTX_EDID_DTD_XFPORCH_XSPW_U2_VFPORCH_MASK) >>
                XDPTX_EDID_DTD_XFPORCH_XSPW_U2_VFPORCH_SHIFT) << 8) |
                ((Ptm[XDPTX_EDID_DTD_VFPORCH_VSPW_L4] &
                XDPTX_EDID_DTD_VFPORCH_VSPW_L4_VFPORCH_MASK) >>
                XDPTX_EDID_DTD_VFPORCH_VSPW_L4_VFPORCH_SHIFT);

        MsaConfig->HFrontPorch = (((Ptm[XDPTX_EDID_DTD_XFPORCH_XSPW_U2] &
                XDPTX_EDID_DTD_XFPORCH_XSPW_U2_HFPORCH_MASK) >>
                XDPTX_EDID_DTD_XFPORCH_XSPW_U2_HFPORCH_SHIFT) << 8) |
                Ptm[XDPTX_EDID_DTD_HFPORCH_LSB];

        MsaConfig->HSyncPolarity = (Ptm[XDPTX_EDID_DTD_SIGNAL] &
                XDPTX_EDID_DTD_SIGNAL_HPOLARITY_MASK) >>
                XDPTX_EDID_DTD_SIGNAL_HPOLARITY_SHIFT;

        MsaConfig->VSyncPolarity = Ptm[XDPTX_EDID_DTD_SIGNAL] &
                XDPTX_EDID_DTD_SIGNAL_VPOLARITY_MASK >>
                XDPTX_EDID_DTD_SIGNAL_VPOLARITY_SHIFT;

        MsaConfig->VBackPorch = VBlank -
                (MsaConfig->VFrontPorch + MsaConfig->VSyncPulseWidth);

        MsaConfig->HBackPorch = HBlank -
                (MsaConfig->HFrontPorch + MsaConfig->HSyncPulseWidth);

        /* Calculate the rest of the MSA values. */
        XDptx_CfgMsaRecalculate(InstancePtr);
}

/******************************************************************************/
/**
 * This function takes a the main stream attributes from MsaConfigCustom and
 * copies them into InstancePtr->MsaConfig. If desired, given a base set of
 * attributes, the rest of the attributes may be derived. The minimal required
 * main stream attributes that must be contained in the MsaConfigCustom
 * structure are:
 *      - MVid
 *      - Horizontal sync polarity
 *      - Vertical sync polarity
 *      - Horizontal sync pulse width
 *      - Vertical sync pulse width
 *      - Horizontal resolution
 *      - Vertical resolution
 *      - Vertical back porch
 *      - Vertical front porch
 *      - Horizontal back porch
 *      - Horizontal front porch
 *
 * @param       InstancePtr is a pointer to the XDptx instance.
 * @param       MsaConfigCustom is the structure that will be used to copy the
 *              main stream attributes from (into InstancePtr->MsaConfig).
 * @param       Recalculate is a boolean enable that determines whether or not
 *              the main stream attributes should be recalculated.
 *
 * @note        The InstancePtr-> MsaConfig structure is modified with the new
 *              values.
 *
*******************************************************************************/
void XDptx_CfgMsaUseCustom(XDptx *InstancePtr,
                XDptx_MainStreamAttributes *MsaConfigCustom, u8 Recalculate)
{
        XDptx_MainStreamAttributes *MsaConfig = &InstancePtr->MsaConfig;

        /* Verify arguments. */
        Xil_AssertVoid(InstancePtr != NULL);
        Xil_AssertVoid(MsaConfigCustom != NULL);

        /* Copy the MSA values from the user configuration structure. */
        MsaConfig->MVid = MsaConfigCustom->MVid;
        MsaConfig->HSyncPolarity = MsaConfigCustom->HSyncPolarity;
        MsaConfig->VSyncPolarity = MsaConfigCustom->VSyncPolarity;
        MsaConfig->HSyncPulseWidth = MsaConfigCustom->HSyncPulseWidth;
        MsaConfig->VSyncPulseWidth = MsaConfigCustom->VSyncPulseWidth;
        MsaConfig->HResolution = MsaConfigCustom->HResolution;
        MsaConfig->VResolution = MsaConfigCustom->VResolution;

        MsaConfig->VBackPorch = MsaConfigCustom->VBackPorch;
        MsaConfig->VFrontPorch = MsaConfigCustom->VFrontPorch;
        MsaConfig->HBackPorch = MsaConfigCustom->HBackPorch;
        MsaConfig->HFrontPorch = MsaConfigCustom->HFrontPorch;

        if (Recalculate) {
                /* Calculate the rest of the MSA values. */
                XDptx_CfgMsaRecalculate(InstancePtr);
        }
        else {
                /* Use the custom values for the rest. */
                MsaConfig->TransferUnitSize = MsaConfigCustom->TransferUnitSize;
                MsaConfig->UserPixelWidth = MsaConfigCustom->UserPixelWidth;
                MsaConfig->NVid = MsaConfigCustom->NVid;
                MsaConfig->HStart = MsaConfigCustom->HStart;
                MsaConfig->VStart = MsaConfigCustom->VStart;
                MsaConfig->HClkTotal = MsaConfigCustom->HClkTotal;
                MsaConfig->VClkTotal = MsaConfigCustom->VClkTotal;
                MsaConfig->Misc0 = MsaConfigCustom->Misc0;
                MsaConfig->Misc1 = MsaConfigCustom->Misc1;
                MsaConfig->DataPerLane = MsaConfigCustom->DataPerLane;
                MsaConfig->AvgBytesPerTU = MsaConfigCustom->AvgBytesPerTU;
                MsaConfig->InitWait = MsaConfigCustom->InitWait;
        }
}

/******************************************************************************/
/**
 * This function sets the bits per color value of the video stream.
 *
 * @param       InstancePtr is a pointer to the XDptx instance
 * @param       BitsPerColor is the new number of bits per color to use.
 *
 * @note        The InstancePtr->MsaConfig structure is modified to reflect the
 *              new main stream attributes associated with a new bits per color
 *              value.
 *
 * @return
 *              - XST_INVALID_PARAM if the supplied bits per color value is not
 *                either 6, 8, 10, 12, or 16.
 *              - XST_SUCCESS otherwise.
 *
*******************************************************************************/
u32 XDptx_CfgMsaSetBpc(XDptx *InstancePtr, u8 BitsPerColor)
{
        /* Verify arguments. */
        Xil_AssertVoid(InstancePtr != NULL);

        switch (BitsPerColor) {
        case 6:
        case 8:
        case 10:
        case 12:
        case 16:
                break;
        default:
                return XST_INVALID_PARAM;
        }

        InstancePtr->MsaConfig.BitsPerColor = BitsPerColor;

        /* Calculate the rest of the MSA values. */
        XDptx_CfgMsaRecalculate(InstancePtr);

        return XST_SUCCESS;
}

/******************************************************************************/
/**
 * This function clears the main stream attributes registers of the DisplayPort
 * TX core and sets them to the values specified in the main stream attributes
 * configuration structure.
 *
 * @param       InstancePtr is a pointer to the XDptx instance
 *
*******************************************************************************/
void XDptx_SetVideoMode(XDptx *InstancePtr)
{
        /* Verify arguments. */
        Xil_AssertVoid(InstancePtr != NULL);

        XDptx_ClearMsaValues(InstancePtr);
        XDptx_SetMsaValues(InstancePtr, &InstancePtr->MsaConfig);
}

/******************************************************************************/
/**
 * This function clears the main stream attributes registers of the DisplayPort
 * TX core.
 *
 * @param       InstancePtr is a pointer to the XDptx instance.
 *
*******************************************************************************/
static void XDptx_ClearMsaValues(XDptx *InstancePtr)
{
        /* Verify arguments. */
        Xil_AssertVoid(InstancePtr != NULL);

        XDptx_Config *TxConfig = &InstancePtr->TxConfig;

	XDptx_WriteReg(TxConfig->BaseAddr, XDPTX_MAIN_STREAM_HTOTAL, 0);
	XDptx_WriteReg(TxConfig->BaseAddr, XDPTX_MAIN_STREAM_VTOTAL, 0);
	XDptx_WriteReg(TxConfig->BaseAddr, XDPTX_MAIN_STREAM_POLARITY, 0);
	XDptx_WriteReg(TxConfig->BaseAddr, XDPTX_MAIN_STREAM_HSWIDTH, 0);
	XDptx_WriteReg(TxConfig->BaseAddr, XDPTX_MAIN_STREAM_VSWIDTH, 0);
	XDptx_WriteReg(TxConfig->BaseAddr, XDPTX_MAIN_STREAM_HRES, 0);
	XDptx_WriteReg(TxConfig->BaseAddr, XDPTX_MAIN_STREAM_VRES, 0);
	XDptx_WriteReg(TxConfig->BaseAddr, XDPTX_MAIN_STREAM_HSTART, 0);
	XDptx_WriteReg(TxConfig->BaseAddr, XDPTX_MAIN_STREAM_VSTART, 0);
	XDptx_WriteReg(TxConfig->BaseAddr, XDPTX_MAIN_STREAM_MISC0, 0);
	XDptx_WriteReg(TxConfig->BaseAddr, XDPTX_MAIN_STREAM_MISC1, 0);
	XDptx_WriteReg(TxConfig->BaseAddr, XDPTX_TU_SIZE, 0);
	XDptx_WriteReg(TxConfig->BaseAddr, XDPTX_USER_PIXEL_WIDTH, 0);
	XDptx_WriteReg(TxConfig->BaseAddr, XDPTX_USER_DATA_COUNT_PER_LANE, 0);
	XDptx_WriteReg(TxConfig->BaseAddr, XDPTX_M_VID, 0);
	XDptx_WriteReg(TxConfig->BaseAddr, XDPTX_N_VID, 0);
	XDptx_WriteReg(TxConfig->BaseAddr, XDPTX_MIN_BYTES_PER_TU, 0);
	XDptx_WriteReg(TxConfig->BaseAddr, XDPTX_FRAC_BYTES_PER_TU, 0);
	XDptx_WriteReg(TxConfig->BaseAddr, XDPTX_INIT_WAIT, 0);
}

/******************************************************************************/
/**
 * This function sets the main stream attributes registers of the DisplayPort TX
 * core with the values specified in the main stream attributes configuration
 * structure.
 *
 * @param       InstancePtr is a pointer to the XDptx instance.
 * @param       MsaConfig is a pointer to the main stream attributes
 *              configuration structure.
 *
*******************************************************************************/
static void XDptx_SetMsaValues(XDptx *InstancePtr,
                                        XDptx_MainStreamAttributes *MsaConfig)
{
        /* Verify arguments. */
        Xil_AssertVoid(InstancePtr != NULL);
        Xil_AssertVoid(MsaConfig != NULL);

        XDptx_Config *TxConfig = &InstancePtr->TxConfig;

        /* Set the main stream attributes to the associated DisplayPort TX core
         * registers. */
        XDptx_WriteReg(TxConfig->BaseAddr, XDPTX_MAIN_STREAM_HTOTAL,
                MsaConfig->HClkTotal);
        XDptx_WriteReg(TxConfig->BaseAddr, XDPTX_MAIN_STREAM_VTOTAL,
                MsaConfig->VClkTotal);
        XDptx_WriteReg(TxConfig->BaseAddr, XDPTX_MAIN_STREAM_POLARITY,
                MsaConfig->HSyncPolarity | (MsaConfig->VSyncPolarity <<
                XDPTX_MAIN_STREAMX_POLARITY_VSYNC_POL_SHIFT));
        XDptx_WriteReg(TxConfig->BaseAddr, XDPTX_MAIN_STREAM_HSWIDTH,
                MsaConfig->HSyncPulseWidth);
        XDptx_WriteReg(TxConfig->BaseAddr, XDPTX_MAIN_STREAM_VSWIDTH,
                MsaConfig->VSyncPulseWidth);
        XDptx_WriteReg(TxConfig->BaseAddr, XDPTX_MAIN_STREAM_HRES,
                MsaConfig->HResolution);
        XDptx_WriteReg(TxConfig->BaseAddr, XDPTX_MAIN_STREAM_VRES,
                MsaConfig->VResolution);
        XDptx_WriteReg(TxConfig->BaseAddr, XDPTX_MAIN_STREAM_HSTART,
                MsaConfig->HStart);
        XDptx_WriteReg(TxConfig->BaseAddr, XDPTX_MAIN_STREAM_VSTART,
                MsaConfig->VStart);
        XDptx_WriteReg(TxConfig->BaseAddr, XDPTX_MAIN_STREAM_MISC0,
                MsaConfig->Misc0);
        XDptx_WriteReg(TxConfig->BaseAddr, XDPTX_MAIN_STREAM_MISC1,
                MsaConfig->Misc1);
        XDptx_WriteReg(TxConfig->BaseAddr, XDPTX_M_VID,
                MsaConfig->MVid);
        XDptx_WriteReg(TxConfig->BaseAddr, XDPTX_N_VID,
                MsaConfig->NVid);
        XDptx_WriteReg(TxConfig->BaseAddr, XDPTX_USER_PIXEL_WIDTH,
                MsaConfig->UserPixelWidth);
        XDptx_WriteReg(TxConfig->BaseAddr, XDPTX_USER_DATA_COUNT_PER_LANE,
                MsaConfig->DataPerLane);

        /* Set the transfer unit values to the associated DisplayPort TX core
         * registers. */
        XDptx_WriteReg(TxConfig->BaseAddr, XDPTX_TU_SIZE,
                MsaConfig->TransferUnitSize);
        XDptx_WriteReg(TxConfig->BaseAddr, XDPTX_MIN_BYTES_PER_TU,
                MsaConfig->AvgBytesPerTU / 1000);
        XDptx_WriteReg(TxConfig->BaseAddr, XDPTX_FRAC_BYTES_PER_TU,
                MsaConfig->AvgBytesPerTU % 1000);
        XDptx_WriteReg(TxConfig->BaseAddr, XDPTX_INIT_WAIT,
                MsaConfig->InitWait);
}
