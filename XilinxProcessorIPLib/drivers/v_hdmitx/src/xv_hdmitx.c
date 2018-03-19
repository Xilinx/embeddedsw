/******************************************************************************
*
* Copyright (C) 2016 Xilinx, Inc. All rights reserved.
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
* @file xv_hdmitx.c
*
* This is the main file for Xilinx HDMI TX core. Please see xv_hdmitx.h for
* more details of the driver.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who    Date     Changes
* ----- ------ -------- -------------------------------------------------------
* 1.00         10/07/15 Initial release.
* 1.01  yh     15/01/16 Add 3D Support
* 1.02  MG     09/03/16 Added XV_HdmiTx_SetHdmiMode and XV_HdmiTx_SetDviMode
* 1.03  YH     25/07/16 Used UINTPTR instead of u32 for BaseAddress
*                       XV_HdmiTx_CfgInitialize
* 1.04  YH     27/07/16 Remove checking VideoMode<(XVIDC_VM_NUM_SUPPORTED));
* 1.05  YH     17/08/16 Add XV_HdmiTx_SetAxiClkFreq
*                       Move XV_HdmiTx_DdcInit to XV_HdmiTx_SetAxiClkFreq
*                       squash unused variable compiler warning
* 1.06  MG     07/03/17 Updated XV_HdmiTx_Auxsend with packet ready check
* 1.07  YH     19/07/17 Added Video Masking Check API
* 1.08  YH     06/10/17 Added function XV_HdmiTx_SetAudioFormat and function
*                           XV_HdmiTx_GetAudioFormat
*       EB     10/10/17 Updated XV_HdmiTx_Scrambler to always enable scrambler
*                           for HDMI 2.0 resolutions
* 1.09  MMO    19/12/17 Added XV_HdmiTx_SetTmdsClk API
* 2.00  YH     16/01/18 Added dedicated reset for each clock domain
*                       Added bridge unlock interrupt
*                       Added PIO_OUT to set GCP_AVMUTE
*       EB     18/01/18 Moved VicTable to Hdmi Common library
*                       Updated function XV_HdmiTx_SetTmdsClk and renamed to
*                           XV_HdmiTx_GetTmdsClk
*                       Updated function XV_HdmiTx_SetStream
*                       Moved VicTable, XV_HdmiTx_Aux to Hdmi Common library
*       EB     23/01/18 Updated XV_HdmiTx_SetAudioChannels to fix an issue
*                           where setting audio channel value will unmute the
*                           audio regardless of the current status
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xv_hdmitx.h"
#include "xparameters.h"
#include "string.h"

/************************** Constant Definitions *****************************/


/***************** Macros (Inline Functions) Definitions *********************/


/**************************** Type Definitions *******************************/

/************************** Function Prototypes ******************************/

static void StubCallback(void *Callback);

/************************** Variable Definitions *****************************/


/************************** Function Definitions *****************************/

/*****************************************************************************/
/**
*
* This function initializes the HDMI TX core. This function must be called
* prior to using the HDMI TX core. Initialization of the HDMI TX includes
* setting up the instance data and ensuring the hardware is in a quiescent
* state.
*
* @param    InstancePtr is a pointer to the XV_HdmiTx core instance.
* @param    CfgPtr points to the configuration structure associated with
*       the HDMI TX core.
* @param    EffectiveAddr is the base address of the device. If address
*       translation is being used, then this parameter must reflect the
*       virtual base address. Otherwise, the physical address should be
*       used.
*
* @return
*       - XST_SUCCESS if XV_HdmiTx_CfgInitialize was successful.
*       - XST_FAILURE if HDMI TX PIO ID mismatched.
*
* @note     None.
*
******************************************************************************/
int XV_HdmiTx_CfgInitialize(XV_HdmiTx *InstancePtr, XV_HdmiTx_Config *CfgPtr,
    UINTPTR EffectiveAddr)
{
    u32 RegValue;

    /* Verify arguments. */
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(CfgPtr != NULL);
    Xil_AssertNonvoid(EffectiveAddr != (UINTPTR)0x0);

    /* Setup the instance */
    (void)memset((void *)InstancePtr, 0, sizeof(XV_HdmiTx));
    (void)memcpy((void *)&(InstancePtr->Config), (const void *)CfgPtr,
        sizeof(XV_HdmiTx_Config));
    InstancePtr->Config.BaseAddress = EffectiveAddr;

    /* Set all handlers to stub values, let user configure this data later */
    InstancePtr->ConnectCallback = (XV_HdmiTx_Callback)((void *)StubCallback);
    InstancePtr->IsConnectCallbackSet = (FALSE);

    InstancePtr->ToggleCallback = (XV_HdmiTx_Callback)((void *)StubCallback);
    InstancePtr->IsToggleCallbackSet = (FALSE);

    InstancePtr->VsCallback = (XV_HdmiTx_Callback)((void *)StubCallback);
    InstancePtr->IsVsCallbackSet = (FALSE);

    InstancePtr->StreamDownCallback =(XV_HdmiTx_Callback)((void *)StubCallback);
    InstancePtr->IsStreamDownCallbackSet = (FALSE);

    InstancePtr->StreamUpCallback = (XV_HdmiTx_Callback)((void *)StubCallback);
    InstancePtr->IsStreamUpCallbackSet = (FALSE);

    /* Clear HDMI variables */
    XV_HdmiTx_Clear(InstancePtr);

    /* Disable scrambler override function */
    InstancePtr->Stream.OverrideScrambler = (FALSE);

    // Set stream status
    InstancePtr->Stream.State = XV_HDMITX_STATE_STREAM_DOWN;
    // The stream is down

    // Clear connected flag
    InstancePtr->Stream.IsConnected = (FALSE);

    /* Reset all peripherals */
    XV_HdmiTx_PioDisable(InstancePtr);
    XV_HdmiTx_DdcDisable(InstancePtr);
    XV_HdmiTx_AudioDisable(InstancePtr);
    XV_HdmiTx_AuxDisable(InstancePtr);

    XV_HdmiTx_PioIntrClear(InstancePtr);
    XV_HdmiTx_DdcIntrClear(InstancePtr);

    /* Read PIO peripheral Identification register */
    RegValue = XV_HdmiTx_ReadReg(InstancePtr->Config.BaseAddress,
    (XV_HDMITX_PIO_ID_OFFSET));

    RegValue = ((RegValue) >> (XV_HDMITX_SHIFT_16)) &
    (XV_HDMITX_MASK_16);
    if (RegValue != (XV_HDMITX_PIO_ID)) {
        return (XST_FAILURE);
    }

    /* PIO: Set event rising edge masks */
    XV_HdmiTx_WriteReg(InstancePtr->Config.BaseAddress,
    (XV_HDMITX_PIO_IN_EVT_RE_OFFSET),
            (XV_HDMITX_PIO_IN_HPD_TOGGLE_MASK) |
            (XV_HDMITX_PIO_IN_HPD_MASK) |
            (XV_HDMITX_PIO_IN_VS_MASK) |
            (XV_HDMITX_PIO_IN_LNK_RDY_MASK)
        );

    /* PIO: Set event falling edge masks */
    XV_HdmiTx_WriteReg(InstancePtr->Config.BaseAddress,
    (XV_HDMITX_PIO_IN_EVT_FE_OFFSET),
            (XV_HDMITX_PIO_IN_BRDG_LOCKED_MASK) |
            (XV_HDMITX_PIO_IN_HPD_MASK) |
            (XV_HDMITX_PIO_IN_LNK_RDY_MASK)
        );

    /* Enable the PIO peripheral interrupt */
    XV_HdmiTx_PioIntrEnable(InstancePtr);

    /* Enable the PIO peripheral */
    XV_HdmiTx_PioEnable(InstancePtr);

    /* Set HDMI mode */
    XV_HdmiTx_SetHdmiMode(InstancePtr);

    /* Enable the AUX peripheral */
    /* The aux peripheral is enabled at stream up */
    //XV_HdmiTx_AuxEnable(InstancePtr);

    /* Enable audio */
    /* The audio peripheral is enabled at stream up */
    //XV_HdmiTx_AudioEnable(InstancePtr);

    /* Reset the hardware and set the flag to indicate the driver is ready */
    InstancePtr->IsReady = (u32)(XIL_COMPONENT_IS_READY);

    return (XST_SUCCESS);
}

/*****************************************************************************/
/**
*
* This function sets the AXI4-Lite Clock Frequency
*
* @param    InstancePtr is a pointer to the XV_HdmiTx core instance.
* @param    ClkFreq specifies the value that needs to be set.
*
* @return
*
*
* @note     This is required after a reset or init.
*
******************************************************************************/
void XV_HdmiTx_SetAxiClkFreq(XV_HdmiTx *InstancePtr, u32 ClkFreq)
{
	InstancePtr->CpuClkFreq = ClkFreq;

    /* Initialize DDC */
    XV_HdmiTx_DdcInit(InstancePtr, InstancePtr->CpuClkFreq);
}

/*****************************************************************************/
/**
*
* This function sets the core into HDMI mode.
*
* @param    InstancePtr is a pointer to the XV_HdmiTx core instance.
*
* @return
*
*
* @note     This is required after a reset or init.
*
******************************************************************************/
void XV_HdmiTx_SetHdmiMode(XV_HdmiTx *InstancePtr)
{

    /* Verify argument. */
    Xil_AssertVoid(InstancePtr != NULL);

    /* Set mode bit in core */
    XV_HdmiTx_SetMode(InstancePtr);

    /* Set flag in structure */
    InstancePtr->Stream.IsHdmi = TRUE;

    /* Enable the AUX peripheral */
    /* The aux peripheral is enabled at stream up */
    XV_HdmiTx_AuxEnable(InstancePtr);

    /* Enable audio */
    /* The audio peripheral is enabled at stream up */
    XV_HdmiTx_AudioEnable(InstancePtr);
}

/*****************************************************************************/
/**
*
* This function sets the core into DVI mode.
*
* @param    InstancePtr is a pointer to the XV_HdmiTx core instance.
*
* @return
*
*
* @note     This is required after a reset or init.
*
******************************************************************************/
void XV_HdmiTx_SetDviMode(XV_HdmiTx *InstancePtr)
{

    /* Verify argument. */
    Xil_AssertVoid(InstancePtr != NULL);

    /* Disable audio peripheral */
    XV_HdmiTx_AudioDisable(InstancePtr);

    /* Disable aux peripheral */
    XV_HdmiTx_AuxDisable(InstancePtr);

    /* Clear mode bit in core */
    XV_HdmiTx_ClearMode(InstancePtr);

    /* Clear flag in structure */
    InstancePtr->Stream.IsHdmi = FALSE;
}

/*****************************************************************************/
/**
*
* This function clear the HDMI TX variables and sets it to the defaults.
*
* @param    InstancePtr is a pointer to the XV_HdmiTx core instance.
*
* @return
*
*
* @note     This is required after a reset or init.
*
******************************************************************************/
void XV_HdmiTx_Clear(XV_HdmiTx *InstancePtr)
{

    /* Verify argument. */
    Xil_AssertVoid(InstancePtr != NULL);
}

/*****************************************************************************/
/**
*
* This function provides video identification code of video mode.
*
* @param    VideoMode specifies resolution identifier.
*
* @return   Video identification code defined in the VIC table.
*
* @note     None.
*
******************************************************************************/
u8 XV_HdmiTx_LookupVic(XVidC_VideoMode VideoMode)
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

/*****************************************************************************/
/**
*
* This function sets and return the TMDS Clock based on Video Parameter
*
*
* @param    InstancePtr is a pointer to the XV_HdmiTx core instance.
* @param    VideoMode specifies resolution identifier.
* @param    ColorFormat specifies the type of color format.
*       - 0 = XVIDC_CSF_RGB
*       - 1 = XVIDC_CSF_YCRCB_444
*       - 2 = XVIDC_CSF_YCRCB_422
*       - 3 = XVIDC_CSF_YCRCB_420
* @param    Bpc specifies the color depth/bits per color component.
*       - 6 = XVIDC_BPC_6
*       - 8 = XVIDC_BPC_8
*       - 10 = XVIDC_BPC_10
*       - 12 = XVIDC_BPC_12
*       - 16 = XVIDC_BPC_16
*
* @return
*       - TMDS Clock
*
* @note     None.
*
******************************************************************************/
u32 XV_HdmiTx_GetTmdsClk (XV_HdmiTx *InstancePtr,
    XVidC_VideoMode VideoMode,
    XVidC_ColorFormat ColorFormat,
    XVidC_ColorDepth Bpc) {

    u32 TmdsClock;

    /* Calculate reference clock. First calculate the pixel clock */
    TmdsClock = XVidC_GetPixelClockHzByVmId(VideoMode);

    /* Store the pixel clock in the structure */
    InstancePtr->Stream.PixelClk = TmdsClock;

    /* YUV420 */
    if (ColorFormat == (XVIDC_CSF_YCRCB_420)) {
        /* In YUV420 the tmds clock is divided by two*/
        TmdsClock = TmdsClock / 2;
    }

    /* RGB, YUV444 and YUV420 */
    if ( ColorFormat != XVIDC_CSF_YCRCB_422 ) {

        switch (Bpc) {

            // 10-bits
            case XVIDC_BPC_10 :
                TmdsClock = TmdsClock * 5 / 4;
                break;

            // 12-bits
            case XVIDC_BPC_12 :
                TmdsClock = TmdsClock * 3 / 2;
                break;

            // 16-bits
            case XVIDC_BPC_16 :
                TmdsClock = TmdsClock * 2;
                break;

            // 8-bits
            default:
                TmdsClock = TmdsClock;
                break;
        }
    }

    return TmdsClock;
}

/*****************************************************************************/
/**
*
* This function controls the scrambler. Requires TMDSClock to be up to date in
* order to force enable scrambler when TMDSClock > 340MHz.
*
* @param    InstancePtr is a pointer to the XV_HdmiTx core instance.
*
* @return
*       - XST_SUCCESS if HDMI 2.0
*       - XST_FAILURE if HDMI 1.4
*
* @note     None.
*
******************************************************************************/
int XV_HdmiTx_Scrambler(XV_HdmiTx *InstancePtr) {
    u8 DdcBuf[2];
    u32 Status;

    /* Verify argument. */
    Xil_AssertNonvoid(InstancePtr != NULL);

    // Check if the sink is HDMI 2.0
    // Check if the TMDS Clock is higher than 340MHz
    // Check scrambler flag
	if (InstancePtr->Stream.IsHdmi20 &&
			((InstancePtr->Stream.TMDSClock > 340000000 &&
					InstancePtr->Stream.OverrideScrambler != (TRUE))
					|| InstancePtr->Stream.IsScrambled)) {
		XV_HdmiTx_SetScrambler(InstancePtr, (TRUE));
	}
	// Clear
	else {
		XV_HdmiTx_SetScrambler(InstancePtr, (FALSE));
	}

    // Update TMDS configuration
    // Only when it is a HDMI 2.0 sink device
    if (InstancePtr->Stream.IsHdmi20) {

        DdcBuf[0] = 0x20;   // Offset scrambler status
        Status = XV_HdmiTx_DdcWrite(InstancePtr, 0x54, 1,
        (u8*)&DdcBuf, (FALSE));

        // Check if write was successful
        if (Status == (XST_SUCCESS)) {

            // Read TMDS configuration
            Status = XV_HdmiTx_DdcRead(InstancePtr, 0x54, 1,
            (u8*)&DdcBuf, (TRUE));

            // The result is in ddc_buf[0]
            // Clear scrambling enable bit
            DdcBuf[0] &= 0xfe;

            // Set scrambler bit if scrambler is enabled
            if (InstancePtr->Stream.IsScrambled)
                DdcBuf[0] |= 0x01;

            // Copy buf[0] to buf[1]
            DdcBuf[1] = DdcBuf[0];

            // Offset
            DdcBuf[0] = 0x20;   // Offset scrambler status

            // Write back TMDS configuration
            Status = XV_HdmiTx_DdcWrite(InstancePtr, 0x54, 2,
            (u8*)&DdcBuf, (TRUE));
        }

        // Write failed
        else {
            return XST_FAILURE;
        }
    }
    return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* This function controls the TMDS clock ratio
*
*
* @param    InstancePtr is a pointer to the XV_HdmiTx core instance.
*
* @return
*       - XST_SUCCESS if HDMI 2.0
*       - XST_FAILURE if HDMI 1.4
*
* @note     None.
*
******************************************************************************/
int XV_HdmiTx_ClockRatio(XV_HdmiTx *InstancePtr) {
    u8 DdcBuf[2];
    u32 Status;

    /* Verify argument. */
    Xil_AssertNonvoid(InstancePtr != NULL);

    // Update TMDS configuration
    // Only when it is a HDMI 2.0 sink device
    if (InstancePtr->Stream.IsHdmi20) {

        DdcBuf[0] = 0x20;   // Offset scrambler status
        Status = XV_HdmiTx_DdcWrite(InstancePtr, 0x54, 1, (u8*)&DdcBuf, (FALSE));

        // Check if write was successful
        if (Status == (XST_SUCCESS)) {

            // Read TMDS configuration
            Status = XV_HdmiTx_DdcRead(InstancePtr, 0x54, 1,
                (u8*)&DdcBuf, (TRUE));

            // The result is in ddc_buf[0]
            // Clear TMDS clock ration bit (1)
            DdcBuf[0] &= 0xfd;

            /* Set the TMDS clock ratio bit if the bandwidth is
                higher than 3.4 Gbps */
            if (InstancePtr->Stream.TMDSClockRatio) {
                DdcBuf[0] |= 0x02;
            }

            // Copy buf[0] to buf[1]
            DdcBuf[1] = DdcBuf[0];

            // Offset
            DdcBuf[0] = 0x20;   // Offset scrambler status

            // Write back TMDS configuration
            Status = XV_HdmiTx_DdcWrite(InstancePtr, 0x54, 2,
                (u8*)&DdcBuf, (TRUE));
        }
    return XST_SUCCESS;
    }
    return XST_FAILURE;
}

/*****************************************************************************/
/**
*
* This function detects connected sink is a HDMI 2.0/HDMI 1.4 sink device
* and sets appropriate flag in the TX stream.
*
* @param    InstancePtr is a pointer to the XV_HdmiTx core instance.
*
* @return
*       - XST_SUCCESS if HDMI 2.0
*       - XST_FAILURE if HDMI 1.4
*
* @note     None.
*
******************************************************************************/
int XV_HdmiTx_DetectHdmi20(XV_HdmiTx *InstancePtr)
{
    u8 DdcBuf[2];
    u32 Status;

    /* Verify argument. */
    Xil_AssertNonvoid(InstancePtr != NULL);

    /* Write source version. Offset (Source version) */
    DdcBuf[0] = 0x02;

    /* Version 1 */
    DdcBuf[1] = 0x01;
    Status = XV_HdmiTx_DdcWrite(InstancePtr, 0x54, 2, (u8*)&DdcBuf, (TRUE));

    /* If the write was successful, then the sink is HDMI 2.0 */
    if (Status == (XST_SUCCESS)) {
        InstancePtr->Stream.IsHdmi20 = (TRUE);
        Status = (XST_SUCCESS);
    }

    /* Else it is a HDMI 1.4 device */
    else {
        InstancePtr->Stream.IsHdmi20 = (FALSE);
        Status = (XST_FAILURE);
    }

    return Status;
}

/*****************************************************************************/
/**
*
* This function shows the sinks SCDC registers.
*
* @param    InstancePtr is a pointer to the XV_HdmiTx core instance.
*
* @return   None.
*
* @note     None.
*
******************************************************************************/
void XV_HdmiTx_ShowSCDC(XV_HdmiTx *InstancePtr)
{
    u8 DdcBuf[2];
    u32 Status;

    /* Verify argument. */
    Xil_AssertVoid(InstancePtr != NULL);

    /* Sink version. Offset Scrambler status */
    DdcBuf[0] = 0x01;
    Status = XV_HdmiTx_DdcWrite(InstancePtr, 0x54, 1, (u8*)&DdcBuf, (FALSE));

    /* Check if write was successful */
    if (Status == (XST_SUCCESS)) {
        Status = XV_HdmiTx_DdcRead(InstancePtr, 0x54, 1, (u8*)&DdcBuf, (TRUE));
        xil_printf("HDMI TX: SCDC 0x01 : %0x\r\n", DdcBuf[0]);
    }

    /* TMDS configuration. Offset Scrambler status */
    DdcBuf[0] = 0x20;
    Status = XV_HdmiTx_DdcWrite(InstancePtr, 0x54, 1, (u8*)&DdcBuf, (FALSE));

    /* Check if write was successful */
    if (Status == (XST_SUCCESS)) {
        Status = XV_HdmiTx_DdcRead(InstancePtr, 0x54, 1, (u8*)&DdcBuf, (TRUE));
        xil_printf("HDMI TX: SCDC 0x20 : %0x\r\n", DdcBuf[0]);
    }

    /* Scrambler status. Offset Scrambler status */
    DdcBuf[0] = 0x21;
    Status = XV_HdmiTx_DdcWrite(InstancePtr, 0x54, 1, (u8*)&DdcBuf, (FALSE));

    /* Check if write was successful */
    if (Status == (XST_SUCCESS)) {
        Status = XV_HdmiTx_DdcRead(InstancePtr, 0x54, 1, (u8*)&DdcBuf, (TRUE));
        xil_printf("HDMI TX: SCDC 0x21 : %0x\r\n", DdcBuf[0]);
    }

    /* Status flags. Offset Scrambler status */
    DdcBuf[0] = 0x40;
    Status = XV_HdmiTx_DdcWrite(InstancePtr, 0x54, 1, (u8*)&DdcBuf, (FALSE));

    /* Check if write was successful */
    if (Status == (XST_SUCCESS)) {
        Status = XV_HdmiTx_DdcRead(InstancePtr, 0x54, 1, (u8*)&DdcBuf, (TRUE));
        xil_printf("HDMI TX: SCDC 0x40 : %0x\r\n", DdcBuf[0]);
    }
}

/*****************************************************************************/
/**
*
* This function sets the HDMI TX stream parameters.
*
* @param    InstancePtr is a pointer to the XV_HdmiTx core instance.
* @param    VideoMode specifies resolution identifier.
* @param    ColorFormat specifies the type of color format.
*       - 0 = XVIDC_CSF_RGB
*       - 1 = XVIDC_CSF_YCRCB_444
*       - 2 = XVIDC_CSF_YCRCB_422
*       - 3 = XVIDC_CSF_YCRCB_420
* @param    Bpc specifies the color depth/bits per color component.
*       - 6 = XVIDC_BPC_6
*       - 8 = XVIDC_BPC_8
*       - 10 = XVIDC_BPC_10
*       - 12 = XVIDC_BPC_12
*       - 16 = XVIDC_BPC_16
* @param    Ppc specifies the pixel per clock.
*       - 1 = XVIDC_PPC_1
*       - 2 = XVIDC_PPC_2
*       - 4 = XVIDC_PPC_4
*
* @return   TmdsClock, reference clock calculated based on the input
*       parameters.
*
* @note     None.
*
******************************************************************************/
u32 XV_HdmiTx_SetStream(XV_HdmiTx *InstancePtr, XVidC_VideoMode VideoMode,
XVidC_ColorFormat ColorFormat, XVidC_ColorDepth Bpc, XVidC_PixelsPerClock Ppc,
XVidC_3DInfo *Info3D)
{
    u32 TmdsClock;

    /* Verify arguments. */
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid((ColorFormat == (XVIDC_CSF_RGB))       ||
                      (ColorFormat == (XVIDC_CSF_YCRCB_444)) ||
                      (ColorFormat == (XVIDC_CSF_YCRCB_422)) ||
                      (ColorFormat == (XVIDC_CSF_YCRCB_420)));
    Xil_AssertNonvoid((Bpc == (XVIDC_BPC_8))  ||
                      (Bpc == (XVIDC_BPC_10)) ||
                      (Bpc == (XVIDC_BPC_12)) ||
                      (Bpc == (XVIDC_BPC_16)));
    Xil_AssertNonvoid((Ppc == (XVIDC_PPC_1)) ||
                      (Ppc == (XVIDC_PPC_2)) ||
                      (Ppc == (XVIDC_PPC_4)));

    if(Info3D == NULL)
        XVidC_SetVideoStream(&InstancePtr->Stream.Video, VideoMode, ColorFormat, Bpc, Ppc);
    else
        XVidC_Set3DVideoStream(&InstancePtr->Stream.Video, VideoMode, ColorFormat, Bpc, Ppc, Info3D);

    /** In HDMI the colordepth in YUV422 is always 12 bits,
    * although on the link itself it is being transmitted as 8-bits.
    * Therefore if the colorspace is YUV422, then force the colordepth
    * to 12 bits. */
    if (ColorFormat == XVIDC_CSF_YCRCB_422) {
        InstancePtr->Stream.Video.ColorDepth = XVIDC_BPC_12;
    }

    InstancePtr->Stream.Vic = XV_HdmiTx_LookupVic(
        InstancePtr->Stream.Video.VmId);

    // Set TX pixel rate
    XV_HdmiTx_SetPixelRate(InstancePtr);

    // Set TX color space
    XV_HdmiTx_SetColorFormat(InstancePtr);

    // Set TX color depth
    XV_HdmiTx_SetColorDepth(InstancePtr);

    /* Calculate reference clock. First calculate the pixel clock */
    TmdsClock = XV_HdmiTx_GetTmdsClk(InstancePtr,
                                     InstancePtr->Stream.Video.VmId,
                                     ColorFormat,
                                     Bpc);

    /* Store TMDS clock for future reference */
	InstancePtr->Stream.TMDSClock = TmdsClock;

    /* HDMI 2.0 */
    if (InstancePtr->Stream.IsHdmi20 && TmdsClock > 340000000) {
            InstancePtr->Stream.IsScrambled = (TRUE);
            InstancePtr->Stream.TMDSClockRatio  = 1;
    }
    /* HDMI 1.4 */
    else {
		InstancePtr->Stream.IsScrambled = (FALSE);
        InstancePtr->Stream.TMDSClockRatio  = 0;
    }

    XV_HdmiTx_Scrambler(InstancePtr);
    XV_HdmiTx_ClockRatio(InstancePtr);

    if ((InstancePtr->Stream.IsHdmi20 == (FALSE)) && (TmdsClock > 340000000)) {
        TmdsClock = 0;
    }

    return TmdsClock;
}

/*****************************************************************************/
/**
*
*  This function asserts or releases the HDMI TX Internal VRST.
*
* @param    InstancePtr is a pointer to the XV_HdmiTx core instance.
* @param    Reset specifies TRUE/FALSE value to either assert or
*       release HDMI TX Internal VRST.
*
* @return   None.
*
* @note     The reset output of the PIO is inverted. When the system is
*       in reset, the PIO output is cleared and this will reset the
*       HDMI TX. Therefore, clearing the PIO reset output will assert
*       the HDMI Internal video reset.
*       C-style signature:
*       void XV_HdmiTx_INT_VRST(XV_HdmiTx *InstancePtr, u8 Reset)
*
******************************************************************************/
void XV_HdmiTx_INT_VRST(XV_HdmiTx *InstancePtr, u8 Reset)
{

    /* Verify argument. */
    Xil_AssertVoid(InstancePtr != NULL);
		
    if (Reset) { 
        XV_HdmiTx_WriteReg((InstancePtr)->Config.BaseAddress, 
        (XV_HDMITX_PIO_OUT_CLR_OFFSET), (XV_HDMITX_PIO_OUT_INT_VRST_MASK));
    } 
    else { 
        XV_HdmiTx_WriteReg((InstancePtr)->Config.BaseAddress, 
        (XV_HDMITX_PIO_OUT_SET_OFFSET), (XV_HDMITX_PIO_OUT_INT_VRST_MASK)); 
    } 
}

/*****************************************************************************/
/**
*
*  This function asserts or releases the HDMI TX Internal LRST.
*
* @param    InstancePtr is a pointer to the XV_HdmiTx core instance.
* @param    Reset specifies TRUE/FALSE value to either assert or
*       release HDMI TX Internal LRST.
*
* @return   None.
*
* @note     The reset output of the PIO is inverted. When the system is
*       in reset, the PIO output is cleared and this will reset the
*       HDMI TX. Therefore, clearing the PIO reset output will assert
*       the HDMI Internal link reset.
*       C-style signature:
*       void XV_HdmiTx_INT_VRST(XV_HdmiTx *InstancePtr, u8 Reset)
*
******************************************************************************/
void XV_HdmiTx_INT_LRST(XV_HdmiTx *InstancePtr, u8 Reset)
{
	/* Verify argument. */
    Xil_AssertVoid(InstancePtr != NULL);
		
    if (Reset) { 
        XV_HdmiTx_WriteReg((InstancePtr)->Config.BaseAddress, 
        (XV_HDMITX_PIO_OUT_CLR_OFFSET), (XV_HDMITX_PIO_OUT_INT_LRST_MASK));
    } 
    else { 
        XV_HdmiTx_WriteReg((InstancePtr)->Config.BaseAddress, 
        (XV_HDMITX_PIO_OUT_SET_OFFSET), (XV_HDMITX_PIO_OUT_INT_LRST_MASK)); 
    } 
}

/*****************************************************************************/
/**
*
*  This function asserts or releases the HDMI TX External VRST.
*
* @param    InstancePtr is a pointer to the XV_HdmiTx core instance.
* @param    Reset specifies TRUE/FALSE value to either assert or
*       release HDMI TX External VRST.
*
* @return   None.
*
* @note     The reset output of the PIO is inverted. When the system is
*       in reset, the PIO output is cleared and this will reset the
*       HDMI TX. Therefore, clearing the PIO reset output will assert
*       the HDMI external video reset.
*       C-style signature:
*       void XV_HdmiTx_EXT_VRST(XV_HdmiTx *InstancePtr, u8 Reset)
*
******************************************************************************/
void XV_HdmiTx_EXT_VRST(XV_HdmiTx *InstancePtr, u8 Reset)
{
	/* Verify argument. */
    Xil_AssertVoid(InstancePtr != NULL);
		
    if (Reset) { 
        XV_HdmiTx_WriteReg((InstancePtr)->Config.BaseAddress, 
        (XV_HDMITX_PIO_OUT_CLR_OFFSET), (XV_HDMITX_PIO_OUT_EXT_VRST_MASK));
    } 
    else { 
        XV_HdmiTx_WriteReg((InstancePtr)->Config.BaseAddress, 
        (XV_HDMITX_PIO_OUT_SET_OFFSET), (XV_HDMITX_PIO_OUT_EXT_VRST_MASK)); 
    } 
}

/*****************************************************************************/
/**
*
*  This function asserts or releases the HDMI TX External SYSRST.
*
* @param    InstancePtr is a pointer to the XV_HdmiTx core instance.
* @param    Reset specifies TRUE/FALSE value to either assert or
*       release HDMI TX External SYSRST.
*
* @return   None.
*
* @note     The reset output of the PIO is inverted. When the system is
*       in reset, the PIO output is cleared and this will reset the
*       HDMI TX. Therefore, clearing the PIO reset output will assert
*       the HDMI External system reset.
*       C-style signature:
*       void XV_HdmiTx_EXT_SYSRST(XV_HdmiTx *InstancePtr, u8 Reset)
*
******************************************************************************/
void XV_HdmiTx_EXT_SYSRST(XV_HdmiTx *InstancePtr, u8 Reset)
{
	/* Verify argument. */
    Xil_AssertVoid(InstancePtr != NULL);
		
    if (Reset) { 
        XV_HdmiTx_WriteReg((InstancePtr)->Config.BaseAddress, 
        (XV_HDMITX_PIO_OUT_CLR_OFFSET), (XV_HDMITX_PIO_OUT_EXT_SYSRST_MASK));
    } 
    else { 
        XV_HdmiTx_WriteReg((InstancePtr)->Config.BaseAddress, 
        (XV_HDMITX_PIO_OUT_SET_OFFSET), (XV_HDMITX_PIO_OUT_EXT_SYSRST_MASK)); 
    } 
}

/*****************************************************************************/
/**
*
*  This function sets the HDMI TX AUX GCP register AVMUTE bit.
*
* @param    InstancePtr is a pointer to the XV_HdmiTx core instance.
*
* @return   None.
*
* @note     None.
*
******************************************************************************/
void XV_HdmiTx_SetGcpAvmuteBit(XV_HdmiTx *InstancePtr)
{
	/* Verify argument. */
    Xil_AssertVoid(InstancePtr != NULL);
	
    XV_HdmiTx_WriteReg((InstancePtr)->Config.BaseAddress, 
        (XV_HDMITX_PIO_OUT_SET_OFFSET), (XV_HDMITX_PIO_OUT_GCP_AVMUTE_MASK));
}

/*****************************************************************************/
/**
*
*  This function clears the HDMI TX AUX GCP register AVMUTE bit.
*
* @param    InstancePtr is a pointer to the XV_HdmiTx core instance.
*
* @return   None.
*
* @note     None.
*
******************************************************************************/
void XV_HdmiTx_ClearGcpAvmuteBit(XV_HdmiTx *InstancePtr)
{
	/* Verify argument. */
    Xil_AssertVoid(InstancePtr != NULL);
	
    XV_HdmiTx_WriteReg((InstancePtr)->Config.BaseAddress, 
        (XV_HDMITX_PIO_OUT_CLR_OFFSET), (XV_HDMITX_PIO_OUT_GCP_AVMUTE_MASK));

}

/*****************************************************************************/
/**
*
*  This function sets the HDMI TX AUX GCP register CLEAR_AVMUTE bit.
*
* @param    InstancePtr is a pointer to the XV_HdmiTx core instance.
*
* @return   None.
*
* @note     None.
*
******************************************************************************/
void XV_HdmiTx_SetGcpClearAvmuteBit(XV_HdmiTx *InstancePtr)
{
	/* Verify argument. */
    Xil_AssertVoid(InstancePtr != NULL);

    XV_HdmiTx_WriteReg((InstancePtr)->Config.BaseAddress,
        (XV_HDMITX_PIO_OUT_SET_OFFSET), (XV_HDMITX_PIO_OUT_GCP_CLEARAVMUTE_MASK));
}

/*****************************************************************************/
/**
*
*  This function clears the HDMI TX AUX GCP register CLEAR_AVMUTE bit.
*
* @param    InstancePtr is a pointer to the XV_HdmiTx core instance.
*
* @return   None.
*
* @note     None.
*
******************************************************************************/
void XV_HdmiTx_ClearGcpClearAvmuteBit(XV_HdmiTx *InstancePtr)
{
	/* Verify argument. */
    Xil_AssertVoid(InstancePtr != NULL);

    XV_HdmiTx_WriteReg((InstancePtr)->Config.BaseAddress,
        (XV_HDMITX_PIO_OUT_CLR_OFFSET), (XV_HDMITX_PIO_OUT_GCP_CLEARAVMUTE_MASK));

}

/*****************************************************************************/
/**
*
* This function sets the pixel rate at output.
*
* @param    InstancePtr is a pointer to the XV_HdmiTx core instance.
*
* @return   None.
*
* @note     None.
*
******************************************************************************/
void XV_HdmiTx_SetPixelRate(XV_HdmiTx *InstancePtr)
{
    u32 RegValue;

    /* Verify argument. */
    Xil_AssertVoid(InstancePtr != NULL);

    /* Mask PIO Out Mask register */
    XV_HdmiTx_WriteReg(InstancePtr->Config.BaseAddress,
        (XV_HDMITX_PIO_OUT_MSK_OFFSET),
        (XV_HDMITX_PIO_OUT_PIXEL_RATE_MASK));

    /* Check for pixel width */
    switch (InstancePtr->Stream.Video.PixPerClk) {

        case (XVIDC_PPC_2):
            RegValue = 1;
            break;

        case (XVIDC_PPC_4):
            RegValue = 2;
            break;

        default:
            RegValue = 0;
            break;
    }

    /* Write pixel rate into PIO Out register */
    XV_HdmiTx_WriteReg(InstancePtr->Config.BaseAddress,
        (XV_HDMITX_PIO_OUT_OFFSET),
        (RegValue << (XV_HDMITX_PIO_OUT_PIXEL_RATE_SHIFT)));
}

/*****************************************************************************/
/**
*
* This function sets the sample rate at output.
*
* @param    InstancePtr is a pointer to the XV_HdmiTx core instance.
* @param    SampleRate specifies the value that needs to be set.
*       - 3 samples per clock.
*       - 5 samples per clock.
*
* @return   None.
*
* @note     None.
*
******************************************************************************/
void XV_HdmiTx_SetSampleRate(XV_HdmiTx *InstancePtr, u8 SampleRate)
{
    u32 RegValue;

    /* Verify argument. */
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(SampleRate < 0xFF);

    // Store sample rate in structure
    InstancePtr->Stream.SampleRate = SampleRate;

    // Mask PIO Out Mask register
    XV_HdmiTx_WriteReg(InstancePtr->Config.BaseAddress,
        (XV_HDMITX_PIO_OUT_MSK_OFFSET),
        (XV_HDMITX_PIO_OUT_SAMPLE_RATE_MASK));

    // Check for sample rate
    switch (SampleRate) {
        case 3:
            RegValue = 1;
            break;

        case 4:
            RegValue = 2;
            break;

        case 5:
            RegValue = 3;
            break;

        default:
            RegValue = 0;
            break;
    }

    // Write sample rate into PIO Out register
    XV_HdmiTx_WriteReg(InstancePtr->Config.BaseAddress,
        (XV_HDMITX_PIO_OUT_OFFSET),
        (RegValue << (XV_HDMITX_PIO_OUT_SAMPLE_RATE_SHIFT)));
}

/*****************************************************************************/
/**
*
* This function sets the color format
*
* @param    InstancePtr is a pointer to the XV_HdmiTx core instance.
*
* @return   None.
*
* @note     None.
*
******************************************************************************/
void XV_HdmiTx_SetColorFormat(XV_HdmiTx *InstancePtr)
{
    u32 RegValue;

    /* Verify argument. */
    Xil_AssertVoid(InstancePtr != NULL);

    /* Mask PIO Out Mask register */
    XV_HdmiTx_WriteReg(InstancePtr->Config.BaseAddress,
        (XV_HDMITX_PIO_OUT_MSK_OFFSET),
        (XV_HDMITX_PIO_OUT_COLOR_SPACE_MASK));

    /* Check for color format */
    switch (InstancePtr->Stream.Video.ColorFormatId) {
        case (XVIDC_CSF_YCRCB_444):
            RegValue = 1;
            break;

        case (XVIDC_CSF_YCRCB_422):
            RegValue = 2;
            break;

        case (XVIDC_CSF_YCRCB_420):
            RegValue = 3;
            break;

        default:
            RegValue = 0;
            break;
    }

    /* Write color space into PIO Out register */
    XV_HdmiTx_WriteReg(InstancePtr->Config.BaseAddress,
        (XV_HDMITX_PIO_OUT_OFFSET),
        (RegValue << (XV_HDMITX_PIO_OUT_COLOR_SPACE_SHIFT)));
}

/*****************************************************************************/
/**
*
* This function sets the color depth
*
* @param    InstancePtr is a pointer to the XV_HdmiTx core instance.
*
* @return   None.
*
* @note     None.
*
******************************************************************************/
void XV_HdmiTx_SetColorDepth(XV_HdmiTx *InstancePtr)
{
    u32 RegValue;

    /* Verify argument. */
    Xil_AssertVoid(InstancePtr != NULL);

    /* Mask PIO Out Mask register */
    XV_HdmiTx_WriteReg(InstancePtr->Config.BaseAddress,
        (XV_HDMITX_PIO_OUT_MSK_OFFSET), (XV_HDMITX_PIO_OUT_COLOR_DEPTH_MASK));

    // Color depth
    switch (InstancePtr->Stream.Video.ColorDepth) {
        // 10 bits
        case (XVIDC_BPC_10):
            RegValue = 1;
            break;

        // 12 bits
        case (XVIDC_BPC_12):
            RegValue = 2;
            break;

        // 16 bits
        case (XVIDC_BPC_16):
            RegValue = 3;
            break;

        // 8 bits
        default:
            RegValue = 0;
            break;
    }

    /* Write color depth into PIO Out register */
    XV_HdmiTx_WriteReg(InstancePtr->Config.BaseAddress,
        (XV_HDMITX_PIO_OUT_OFFSET),
        (RegValue << (XV_HDMITX_PIO_OUT_COLOR_DEPTH_SHIFT)));
}

/*****************************************************************************/
/**
*
* This function prepares TX DDC peripheral to use.
*
* @param    InstancePtr is a pointer to the XV_HdmiTx core instance.
* @param    Frequency specifies the value that needs to be set.
*
* @return   None.
*
* @note     None.
*
******************************************************************************/
void XV_HdmiTx_DdcInit(XV_HdmiTx *InstancePtr, u32 Frequency)
{
    u32 RegValue;

    /* Verify arguments. */
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(Frequency > 0x0);

    RegValue = (Frequency / 100000) / 2;
    RegValue = ((RegValue) << (XV_HDMITX_DDC_CTRL_CLK_DIV_SHIFT)) &
     ((XV_HDMITX_DDC_CTRL_CLK_DIV_MASK) << (XV_HDMITX_DDC_CTRL_CLK_DIV_SHIFT));

    /* Update DDC Control register */
    XV_HdmiTx_WriteReg(InstancePtr->Config.BaseAddress,
        (XV_HDMITX_DDC_CTRL_OFFSET), RegValue);
}

/*****************************************************************************/
/**
*
* This function gets the acknowledge flag
*
* @param    InstancePtr is a pointer to the XV_HdmiTx core instance.
*
* @note     None.
*
******************************************************************************/
int XV_HdmiTx_DdcGetAck(XV_HdmiTx *InstancePtr)
{
    u32 Status;

    // Read status register
    Status = XV_HdmiTx_ReadReg(InstancePtr->Config.BaseAddress,
        (XV_HDMITX_DDC_STA_OFFSET));
    return (Status & XV_HDMITX_DDC_STA_ACK_MASK);
}

/*****************************************************************************/
/**
*
* This function waits for the done flag to be set
*
* @param    InstancePtr is a pointer to the XV_HdmiTx core instance.
*
* @note     None.
*
******************************************************************************/
int XV_HdmiTx_DdcWaitForDone(XV_HdmiTx *InstancePtr)
{
    u32 Data;
    u32 Status;
    u32 Exit;

    Exit = (FALSE);

    // Default status, assume failure
    Status = XST_FAILURE;

    do {
        // Read control register
        Data = XV_HdmiTx_ReadReg(InstancePtr->Config.BaseAddress,
            (XV_HDMITX_DDC_CTRL_OFFSET));

        if (Data & (XV_HDMITX_DDC_CTRL_RUN_MASK)) {

            // Read status register
            Data = XV_HdmiTx_ReadReg(InstancePtr->Config.BaseAddress,
                (XV_HDMITX_DDC_STA_OFFSET));

            // Done
            if (Data & (XV_HDMITX_DDC_STA_DONE_MASK)) {
                // Clear done flag
                XV_HdmiTx_WriteReg(InstancePtr->Config.BaseAddress,
                    (XV_HDMITX_DDC_STA_OFFSET), (XV_HDMITX_DDC_STA_DONE_MASK));
                Exit = (TRUE);
                Status = XST_SUCCESS;
            }

            // Time out
            else if (Data & (XV_HDMITX_DDC_STA_TIMEOUT_MASK)) {
                // Clear time out flag
                XV_HdmiTx_WriteReg(InstancePtr->Config.BaseAddress,
                    (XV_HDMITX_DDC_STA_OFFSET), (XV_HDMITX_DDC_STA_TIMEOUT_MASK));
                Exit = (TRUE);
                Status = XST_FAILURE;
            }
        }
        else {
            Status = (XST_FAILURE);
            Exit = (TRUE);
        }

    } while (!Exit);

    return Status;
}

/*****************************************************************************/
/**
*
* This function writes data into the command fifo.
*
* @param    InstancePtr is a pointer to the XV_HdmiTx core instance.
*
* @note     None.
*
******************************************************************************/
void XV_HdmiTx_DdcWriteCommand(XV_HdmiTx *InstancePtr, u32 Cmd)
{
    u32 Status;
    u32 Exit;

    Exit = (FALSE);

    do {
        // Read control register
        Status = XV_HdmiTx_ReadReg(InstancePtr->Config.BaseAddress,
            (XV_HDMITX_DDC_CTRL_OFFSET));

        if (Status & (XV_HDMITX_DDC_CTRL_RUN_MASK)) {
            // Read status register
            Status = XV_HdmiTx_ReadReg(InstancePtr->Config.BaseAddress,
                (XV_HDMITX_DDC_STA_OFFSET));

            // Mask command fifo full flag
            Status &= XV_HDMITX_DDC_STA_CMD_FULL;

            // Check if the command fifo isn't full
            if (!Status) {
                XV_HdmiTx_WriteReg(InstancePtr->Config.BaseAddress,
                    (XV_HDMITX_DDC_CMD_OFFSET), (Cmd));
                Exit = (TRUE);
            }
        }
        else {
            Status = (XST_FAILURE);
            Exit = (TRUE);
        }
    } while (!Exit);
}

/*****************************************************************************/
/**
*
* This function reads data from the data fifo.
*
* @param    InstancePtr is a pointer to the XV_HdmiTx core instance.
*
* @note     None.
*
******************************************************************************/
u8 XV_HdmiTx_DdcReadData(XV_HdmiTx *InstancePtr)
{
    u32 Status;
    u32 Exit;
    u32 Data;

    Exit = (FALSE);

    do {
        // Read control register
        Data = XV_HdmiTx_ReadReg(InstancePtr->Config.BaseAddress,
            (XV_HDMITX_DDC_CTRL_OFFSET));

        if (Data & (XV_HDMITX_DDC_CTRL_RUN_MASK)) {
            // Read status register
            Status = XV_HdmiTx_ReadReg(InstancePtr->Config.BaseAddress,
                (XV_HDMITX_DDC_STA_OFFSET));

            // Mask data fifo empty flag
            Status &= XV_HDMITX_DDC_STA_DAT_EMPTY;

            // Check if the data fifo has data
            if (!Status) {
                Data = XV_HdmiTx_ReadReg(InstancePtr->Config.BaseAddress,
                    (XV_HDMITX_DDC_DAT_OFFSET));
                Exit = (TRUE);
            }
        }
        else {
            Exit = (TRUE);
            Data = 0;
        }
    } while (!Exit);

    return (Data);
}

/*****************************************************************************/
/**
*
* This function writes data from DDC peripheral from given slave address.
*
* @param    InstancePtr is a pointer to the XV_HdmiTx core instance.
* @param    Slave specifies the slave address from where data needs to be
*       read.
* @param    Length specifies number of bytes to be read.
* @param    Buffer specifies a pointer to u8 variable that will be
*       filled with data.
* @param    Stop specifies the stop flag which is either TRUE/FALSE.
*
* @return
*       - XST_SUCCESS if an acknowledgement received and timeout.
*       - XST_FAILURE if no acknowledgement received.
*
* @note     None.
*
******************************************************************************/
int XV_HdmiTx_DdcWrite(XV_HdmiTx *InstancePtr, u8 Slave,
    u16 Length, u8 *Buffer, u8 Stop)
{
    u32 Status;
    u32 Data;
    u32 Index;

    /* Verify arguments. */
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(Slave > 0x0);
    Xil_AssertNonvoid(Length > 0x0);
    Xil_AssertNonvoid(Buffer != NULL);
    Xil_AssertNonvoid((Stop == (TRUE)) || (Stop == (FALSE)));

    // Status default, assume failure
    Status = XST_FAILURE;

    // Enable DDC peripheral
    XV_HdmiTx_DdcEnable(InstancePtr);

    // Disable interrupt in DDC peripheral
    // Polling is used
    XV_HdmiTx_DdcIntrDisable(InstancePtr);

    // Write start token
    XV_HdmiTx_DdcWriteCommand(InstancePtr, (XV_HDMITX_DDC_CMD_STR_TOKEN));

    // First check if the slave can be addressed
    // Write write token
    XV_HdmiTx_DdcWriteCommand(InstancePtr, (XV_HDMITX_DDC_CMD_WR_TOKEN));

    // Write length (high)
    XV_HdmiTx_DdcWriteCommand(InstancePtr, 0);

    // Write length (low)
    XV_HdmiTx_DdcWriteCommand(InstancePtr, 1);

    // Slave address
    Data = Slave << 1;

    // Set write bit (low)
    Data &= 0xFE;

    // Write slave address
    XV_HdmiTx_DdcWriteCommand(InstancePtr, (Data));

    // Wait for done flag
    if (XV_HdmiTx_DdcWaitForDone(InstancePtr) == XST_SUCCESS) {

        // Check acknowledge
        if (XV_HdmiTx_DdcGetAck(InstancePtr)) {

            // Now write the data
            // Write write token
            XV_HdmiTx_DdcWriteCommand(InstancePtr,
                (XV_HDMITX_DDC_CMD_WR_TOKEN));

            // Write length (high)
            Data = ((Length >> 8) & 0xFF);
            XV_HdmiTx_DdcWriteCommand(InstancePtr, Data);

            // Write length (low)
            Data = (Length & 0xFF);
            XV_HdmiTx_DdcWriteCommand(InstancePtr, Data);

            // Write Data
            for (Index = 0; Index < Length; Index++) {
                XV_HdmiTx_DdcWriteCommand(InstancePtr, *Buffer++);
            }

            // Wait for done flag
            if (XV_HdmiTx_DdcWaitForDone(InstancePtr) == XST_SUCCESS) {

                // Check acknowledge
                // ACK
                if (XV_HdmiTx_DdcGetAck(InstancePtr)) {

                    // Stop condition
                    if (Stop) {
                        // Write stop token
                        XV_HdmiTx_DdcWriteCommand(InstancePtr,
                            (XV_HDMITX_DDC_CMD_STP_TOKEN));

                        // Wait for done flag
                        XV_HdmiTx_DdcWaitForDone(InstancePtr);

                    }

                // Update status flag
                Status = XST_SUCCESS;
                }
            }
        }
    }

    // Disable DDC peripheral
    XV_HdmiTx_DdcDisable(InstancePtr);

    return Status;
}

/*****************************************************************************/
/**
*
* This function reads data from DDC peripheral from given slave address.
*
* @param    InstancePtr is a pointer to the XV_HdmiTx core instance.
* @param    Slave specifies the slave address from where data needs to be
*       read.
* @param    Length specifies number of bytes to be read.
* @param    Buffer specifies a pointer to u8 variable that will be
*       filled with data.
* @param    Stop specifies the stop flag which is either TRUE/FALSE.
*
* @return
*       - XST_SUCCESS if an acknowledgement received and timeout.
*       - XST_FAILURE if no acknowledgement received.
*
* @note     None.
*
******************************************************************************/
int XV_HdmiTx_DdcRead(XV_HdmiTx *InstancePtr, u8 Slave, u16 Length,
    u8 *Buffer, u8 Stop)
{
    u32 Status;
    u32 Data;
    u32 Index;

    /* Verify arguments. */
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(Slave > 0x0);
    Xil_AssertNonvoid(Length > 0x0);
    Xil_AssertNonvoid(Buffer != NULL);
    Xil_AssertNonvoid((Stop == (TRUE)) || (Stop == (FALSE)));

    // Status default, assume failure
    Status = XST_FAILURE;

    // Enable DDC peripheral
    XV_HdmiTx_DdcEnable(InstancePtr);

    // Disable interrupt in DDC peripheral
    // Polling is used
    XV_HdmiTx_DdcIntrDisable(InstancePtr);

    // Write start token
    XV_HdmiTx_DdcWriteCommand(InstancePtr, (XV_HDMITX_DDC_CMD_STR_TOKEN));

    // First check if the slave can be addressed
    // Write write token
    XV_HdmiTx_DdcWriteCommand(InstancePtr, (XV_HDMITX_DDC_CMD_WR_TOKEN));

    // Write length (high)
    XV_HdmiTx_DdcWriteCommand(InstancePtr, 0);

    // Write length (low)
    XV_HdmiTx_DdcWriteCommand(InstancePtr, 1);

    // Slave address
    Data = Slave << 1;

    // Set read bit (high)
    Data |= 0x01;

    // Write slave address
    XV_HdmiTx_DdcWriteCommand(InstancePtr, (Data));

    // Wait for done flag
    if (XV_HdmiTx_DdcWaitForDone(InstancePtr) == XST_SUCCESS) {

        // Check acknowledge
        if (XV_HdmiTx_DdcGetAck(InstancePtr)) {

            // Write read token
            XV_HdmiTx_DdcWriteCommand(InstancePtr,
                (XV_HDMITX_DDC_CMD_RD_TOKEN));

            // Write read length (high)
            Data = (Length >> 8) & 0xFF;
            XV_HdmiTx_DdcWriteCommand(InstancePtr, (Data));

            // Write read length (low)
            Data = Length & 0xFF;
            XV_HdmiTx_DdcWriteCommand(InstancePtr, (Data));

            // Read Data
            for (Index = 0; Index < Length; Index++) {
                *Buffer++ = XV_HdmiTx_DdcReadData(InstancePtr);
            }

            // Wait for done flag
            if (XV_HdmiTx_DdcWaitForDone(InstancePtr) == XST_SUCCESS) {

                // Stop condition
                if (Stop) {
                    // Write stop token
                    XV_HdmiTx_DdcWriteCommand(InstancePtr,
                        (XV_HDMITX_DDC_CMD_STP_TOKEN));

                    // Wait for done flag
                    XV_HdmiTx_DdcWaitForDone(InstancePtr);

                }

                // Update status
                Status = XST_SUCCESS;
            }
        }
    }

    // Disable DDC peripheral
    XV_HdmiTx_DdcDisable(InstancePtr);

    return Status;
}

/*****************************************************************************/
/**
*
* This function transmits the infoframes generated by the processor.
*
* @param    InstancePtr is a pointer to the XV_HdmiTx core instance.
*
* @return
*       - XST_SUCCESS if infoframes transmitted successfully.
*       - XST_FAILURE if AUX FIFO is full.
*
* @note     None.
*
******************************************************************************/
u32 XV_HdmiTx_AuxSend(XV_HdmiTx *InstancePtr)
{
    u32 Index;
    u32 Status;
    u32 RegValue;

    /* Verify argument. */
    Xil_AssertNonvoid(InstancePtr != NULL);

    // Default
	Status = (XST_FAILURE);

    /* Read the AUX status register */
    RegValue = XV_HdmiTx_ReadReg(InstancePtr->Config.BaseAddress,
        (XV_HDMITX_AUX_STA_OFFSET));

    // First check if the AUX packet is ready
    if (RegValue & (XV_HDMITX_AUX_STA_PKT_RDY_MASK)) {

	// Check if the fifo is full
		if (RegValue & (XV_HDMITX_AUX_STA_FIFO_FUL_MASK)) {
			RegValue = XV_HdmiTx_ReadReg(InstancePtr->Config.BaseAddress,
			        (XV_HDMITX_AUX_STA_OFFSET));

			xdbg_printf((XDBG_DEBUG_GENERAL), "HDMI TX AUX FIFO full\r\n");
		}
		else {
			/* Update AUX with header data */
			XV_HdmiTx_WriteReg(InstancePtr->Config.BaseAddress,
				(XV_HDMITX_AUX_DAT_OFFSET), InstancePtr->Aux.Header.Data);

			/* Update AUX with actual data */
			for (Index = 0x0; Index < 8; Index++) {
				XV_HdmiTx_WriteReg(InstancePtr->Config.BaseAddress,
					(XV_HDMITX_AUX_DAT_OFFSET), InstancePtr->Aux.Data.Data[Index]);
			}

			Status = (XST_SUCCESS);
		}
    }
    return Status;
}

/******************************************************************************/
/**
*
* This function prints stream and timing information on STDIO/Uart console.
*
* @param    InstancePtr is a pointer to the XV_HdmiTx core instance.
*
* @return   None.
*
* @note     None.
*
******************************************************************************/
void XV_HdmiTx_DebugInfo(XV_HdmiTx *InstancePtr)
{

    /* Verify argument. */
    Xil_AssertVoid(InstancePtr != NULL);

    /* Print stream information */
    XVidC_ReportStreamInfo(&InstancePtr->Stream.Video);

    /* Print timing information */
    XVidC_ReportTiming(&InstancePtr->Stream.Video.Timing,
                InstancePtr->Stream.Video.IsInterlaced);
}

/*****************************************************************************/
/**
*
* This function provides status of the stream
*
* @param    InstancePtr is a pointer to the XV_HdmiTx core instance.
*
* @return
*       - TRUE = Scrambled.
*       - FALSE = Not scrambled.
*
* @note     None.
*
******************************************************************************/
int XV_HdmiTx_IsStreamScrambled(XV_HdmiTx *InstancePtr)
{

    /* Verify argument. */
    Xil_AssertNonvoid(InstancePtr != NULL);

    return (InstancePtr->Stream.IsScrambled);
}

/*****************************************************************************/
/**
*
* This function provides the stream connected status
*
* @param    InstancePtr is a pointer to the XV_HdmiTx core instance.
*
* @return
*       - TRUE = Stream is connected.
*       - FALSE = Stream is connected.
*
* @note     None.
*
******************************************************************************/
int XV_HdmiTx_IsStreamConnected(XV_HdmiTx *InstancePtr)
{

    /* Verify argument. */
    Xil_AssertNonvoid(InstancePtr != NULL);

    return (InstancePtr->Stream.IsConnected);
}

/*****************************************************************************/
/**
*
* This function sets the active audio channels
*
* @param    InstancePtr is a pointer to the XV_HdmiTx core instance.
*
* @return
*       - XST_SUCCESS if active channels were set.
*       - XST_FAILURE if no active channles were set.
*
* @note     None.
*
******************************************************************************/
int XV_HdmiTx_SetAudioChannels(XV_HdmiTx *InstancePtr, u8 Value)
{
    u32 Data;
    u32 Status;
    u8 AudioStatus;

    AudioStatus = XV_HdmiTx_ReadReg((InstancePtr)->Config.BaseAddress,
        		XV_HDMITX_AUD_CTRL_OFFSET) & XV_HDMITX_AUD_CTRL_RUN_MASK;

    // Stop peripheral
    XV_HdmiTx_WriteReg((InstancePtr)->Config.BaseAddress,
        (XV_HDMITX_AUD_CTRL_CLR_OFFSET), (XV_HDMITX_AUD_CTRL_RUN_MASK));

    switch (Value) {

        // 8 Channels
        case 8:
            Data = 3 << XV_HDMITX_AUD_CTRL_CH_SHIFT;
            Status = (XST_SUCCESS);
            break;

        // 6 Channels
        case 6:
            Data = 2 << XV_HDMITX_AUD_CTRL_CH_SHIFT;
            Status = (XST_SUCCESS);
            break;

        // 4 Channels
        case 4:
            Data = 1 << XV_HDMITX_AUD_CTRL_CH_SHIFT;
            Status = (XST_SUCCESS);
            break;

        // 2 Channels
        case 2:
            Data = 0 << XV_HDMITX_AUD_CTRL_CH_SHIFT;
            Status = (XST_SUCCESS);
            break;

        default :
            Status = (XST_FAILURE);
            break;
    }

    if (Status == (XST_SUCCESS)) {
        // Set active channels
        XV_HdmiTx_WriteReg((InstancePtr)->Config.BaseAddress,
            (XV_HDMITX_AUD_CTRL_OFFSET), (Data));

        // Store active channel in structure
        (InstancePtr)->Stream.Audio.Channels = Value;

        // Start peripheral
        if (AudioStatus) {
			XV_HdmiTx_WriteReg((InstancePtr)->Config.BaseAddress,
					(XV_HDMITX_AUD_CTRL_SET_OFFSET),
					(XV_HDMITX_AUD_CTRL_RUN_MASK));
        }
    }

    return Status;
}

/*****************************************************************************/
/**
*
* This function sets the active audio format
*
* @param    InstancePtr is a pointer to the XV_HdmiTx core instance.
*
* @return
*       - XST_SUCCESS if active channels were set.
*       - XST_FAILURE if no active channles were set.
*
* @note     None.
*
******************************************************************************/
int XV_HdmiTx_SetAudioFormat(XV_HdmiTx *InstancePtr, XV_HdmiTx_AudioFormatType Value)
{
    u32 Status;

    // Stop peripheral
    XV_HdmiTx_WriteReg((InstancePtr)->Config.BaseAddress,
        (XV_HDMITX_AUD_CTRL_CLR_OFFSET), (XV_HDMITX_AUD_CTRL_RUN_MASK));

    // HBR audio
    if (Value) {
        XV_HdmiTx_WriteReg((InstancePtr)->Config.BaseAddress,
            (XV_HDMITX_AUD_CTRL_SET_OFFSET), (XV_HDMITX_AUD_CTRL_AUDFMT_MASK));
    }

    // L-PCM
    else {
        XV_HdmiTx_WriteReg((InstancePtr)->Config.BaseAddress,
            (XV_HDMITX_AUD_CTRL_CLR_OFFSET), (XV_HDMITX_AUD_CTRL_AUDFMT_MASK));
    }

	// Start peripheral
	XV_HdmiTx_WriteReg((InstancePtr)->Config.BaseAddress,
		(XV_HDMITX_AUD_CTRL_SET_OFFSET), (XV_HDMITX_AUD_CTRL_RUN_MASK));

    Status = (XST_SUCCESS);

    return Status;
}

/*****************************************************************************/
/**
*
* This function gets the active audio format
*
* @param    InstancePtr is a pointer to the XV_HdmiTx core instance.
*
* @return   Active audio format of HDMI Tx
*
* @note     None.
*
******************************************************************************/
XV_HdmiTx_AudioFormatType XV_HdmiTx_GetAudioFormat(XV_HdmiTx *InstancePtr)
{
	XV_HdmiTx_AudioFormatType RegValue;

    Xil_AssertNonvoid(InstancePtr != NULL);

    RegValue = XV_HdmiTx_ReadReg((InstancePtr)->Config.BaseAddress, (XV_HDMITX_AUD_CTRL_OFFSET));
	RegValue = (RegValue & (XV_HDMITX_AUD_CTRL_AUDFMT_MASK)) >> XV_HDMITX_AUD_CTRL_AUDFMT_SHIFT;

    return RegValue;
}

/*****************************************************************************/
/**
*
* This function is a stub for the asynchronous callback. The stub is here in
* case the upper layer forgot to set the handlers. On initialization, all
* handlers are set to this callback. It is considered an error for this
* handler to be invoked.
*
* @param    CallbackRef is a callback reference passed in by the upper
*       layer when setting the callback functions, and passed back to
*       the upper layer when the callback is invoked.
*
* @return   None.
*
* @note     None.
*
******************************************************************************/
static void StubCallback(void *Callback)
{
    Xil_AssertVoid(Callback != NULL);
    Xil_AssertVoidAlways();
}
