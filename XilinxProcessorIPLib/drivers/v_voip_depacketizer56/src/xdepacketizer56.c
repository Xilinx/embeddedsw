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
* @file xpacketizer56.c
*
* This is the main file for Xilinx VoIP ST2022-6 Packetizer core. Please see
* xpacketizer56.h for more details of the driver.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who    Date     Changes
* ----- ------ -------- -------------------------------------------------------
* 1.00   mmo   02/12/16 Initial release.

* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xdepacketizer56.h"
#include "string.h"

/************************** Constant Definitions *****************************/

/***************** Macros (Inline Functions) Definitions *********************/


/**************************** Type Definitions *******************************/


/************************** Function Prototypes ******************************/

static void StubCallback(void *CallbackRef);

/************************** Variable Definitions *****************************/


/************************** Function Definitions *****************************/

/*****************************************************************************/
/**
*
* This function initializes the VoIP ST2022-6 Depacketizer core. This function
* must be called prior to using the VoIP ST2022-6 Depacketizer core.
* Initialization of the VoIP ST2022-6 Depacketizer includes setting up the
* instance data, and ensuring the hardware is in a quiescent state.
*
* @param    InstancePtr is a pointer to the VoIP ST2022-6 Depacketizer core
        instance.
* @param    CfgPtr points to the configuration structure associated with
*       the VoIP ST2022-6 Depacketizer core.
* @param    EffectiveAddr is the base address of the device. If address
*       translation is being used, then this parameter must reflect the
*       virtual base address. Otherwise, the physical address should be
*       used.
*
* @return
*       - XST_SUCCESS if XDepacketizer56_CfgInitialize was successful.
*
* @note     None.
*
******************************************************************************/
int XDepacketizer56_CfgInitialize(XDepacketizer56 *InstancePtr,
                                     XDepacketizer56_Config *CfgPtr,
                                                   UINTPTR EffectiveAddr)
{
    /* Verify arguments. */
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(CfgPtr != NULL);
    Xil_AssertNonvoid(EffectiveAddr != (UINTPTR)0x0);

    /* Setup the instance */
    (void)memset((void *)InstancePtr, 0, sizeof(XDepacketizer56));
    (void)memcpy((void *)&(InstancePtr->Config), (const void *)CfgPtr,
                                               sizeof(XDepacketizer56_Config));
    InstancePtr->Config.BaseAddress = EffectiveAddr;

    /*
        Callbacks
        These are placeholders pointing to the StubCallback
        The actual callback pointers will be assigned by the SetCallback
        function
    */

    InstancePtr->DatagramMismatchCallback =
                              (XDepacketizer56_Callback)((void *)StubCallback);
    InstancePtr->IsDatagramMismatchCallbackSet = (FALSE);

    /* Clear XVModular SDI2AXIS Register Space */
    XDepacketizer56_RegClear(InstancePtr);

    /* Clear connected flag */
    InstancePtr->Stream.IsConnected = (FALSE);

    /* Set Decoding Mode */
    InstancePtr->Stream.FrameCfg = XDEPACKETIZER56_DECODE_FROM_VID_SRC_FMT;

    /* Module Enable */
    InstancePtr->ModuleEnable = XDEPACKETIZER56_MODULE_DISABLE;

    /* Module Disable and Buffer Reset & Statistic Reset */


    /* Set Mask Interrupt/Enable Interrupt */

    /* Reset the hardware and set the flag to indicate the driver is ready */
    InstancePtr->IsReady = (u32)(XIL_COMPONENT_IS_READY);

    return (XST_SUCCESS);
}

/*****************************************************************************/
/**
*
* This function obtained the current statistic of the ST2022-6 Depacketizer
*
* @param    InstancePtr is a pointer to the XDepacketizer56 core instance.
*
* @return   XDepacketizer56_ModuleStatistic_RegValue.
*
* @note     None.
*
******************************************************************************/
XDepacketizer56_ModuleStatistic XDepacketizer56_ModuleStatisticRegValue
                                                 (XDepacketizer56 *InstancePtr)
{
    XDepacketizer56_ModuleStatistic XDepacketizer56_ModuleStatistic_RegValue;

    XDepacketizer56_ModuleStatistic_RegValue.peak_buf_level =
      (XDepacketizer56_ReadReg((InstancePtr->Config.BaseAddress),
          (XDEPACKETIZER56_BUF_LEVEL_REG_OFFSET)) &
              (XDEPACKETIZER56_BUF_LEVEL_PEAK_BUFFER_LEVEL_MASK));

    XDepacketizer56_ModuleStatistic_RegValue.curr_buf_level =
       ((XDepacketizer56_ReadReg((InstancePtr->Config.BaseAddress),
            (XDEPACKETIZER56_BUF_LEVEL_REG_OFFSET)) &
                (XDEPACKETIZER56_BUF_LEVEL_CURR_BUFFER_LEVEL_MASK)) >>
                    (XDEPACKETIZER56_BUF_LEVEL_CURR_BUFFER_LEVEL_SHIFT));

    XDepacketizer56_ModuleStatistic_RegValue.input_pkt_cnt =
       XDepacketizer56_ReadReg((InstancePtr->Config.BaseAddress),
          (XDEPACKETIZER56_INPUT_PKT_CNT_REG_OFFSET));

    XDepacketizer56_ModuleStatistic_RegValue.sdi_tx_frame_cnt =
       XDepacketizer56_ReadReg((InstancePtr->Config.BaseAddress),
          (XDEPACKETIZER56_TRANSMIT_FRAME_CNT_REG_OFFSET));

    return (XDepacketizer56_ModuleStatistic_RegValue);
}

/*****************************************************************************/
/**
*
* This function perforemed software reset or Register Clear on the ST2022-6
* Depacketizer Module
*
* @param    InstancePtr is a pointer to the XDepacketizer56 core instance.
*
* @return   XDepacketizer56_ModuleStatistic_RegValue.
*
* @note     None.
*
******************************************************************************/
void XDepacketizer56_RegClear(XDepacketizer56 *InstancePtr)
{
    u32 RegValue;

    RegValue = XDepacketizer56_ReadReg((InstancePtr)->Config.BaseAddress,
            (XDEPACKETIZER56_CONTROL_REG_OFFSET)) &
                ((~(XDEPACKETIZER56_CONTROL_SOFT_RESET_MASK)));

    /* Set the Soft Reset Bit */
    XDepacketizer56_WriteReg((InstancePtr)->Config.BaseAddress,
       (XDEPACKETIZER56_CONTROL_REG_OFFSET),
           (RegValue | XDEPACKETIZER56_CONTROL_SOFT_RESET_MASK));

    RegValue = XDepacketizer56_ReadReg((InstancePtr)->Config.BaseAddress,
            (XDEPACKETIZER56_CONTROL_REG_OFFSET)) &
                ((~(XDEPACKETIZER56_CONTROL_SOFT_RESET_MASK)));

    /* Clear the Soft Reset Bit */
    XDepacketizer56_WriteReg((InstancePtr)->Config.BaseAddress,
       (XDEPACKETIZER56_CONTROL_REG_OFFSET),
           (RegValue & (~(XDEPACKETIZER56_CONTROL_SOFT_RESET_MASK))));
}

/*****************************************************************************/
/**
*
* This function perform Module Enable/Disable the ST2022-6 Depacketizer Module
*
* @param    InstancePtr is a pointer to the XDepacketizer56 core instance.
*
* @return   XDepacketizer56_ModuleStatistic_RegValue.
*
* @note     None.
*
******************************************************************************/
void XDepacketizer56_ModEnable (XDepacketizer56 *InstancePtr)
{
    u32 RegValue;

    RegValue = XDepacketizer56_ReadReg((InstancePtr)->Config.BaseAddress,
            (XDEPACKETIZER56_MODULE_CONTROL_REG_OFFSET)) &
                (~(XDEPACKETIZER56_MODULE_CONTROL_ENABLE_MASK));


    XDepacketizer56_WriteReg((InstancePtr)->Config.BaseAddress,
       (XDEPACKETIZER56_MODULE_CONTROL_REG_OFFSET),
            (RegValue | InstancePtr->ModuleEnable));
}

/*****************************************************************************/
/**
*
* This function configures the Video Information in to the ST2022-6
* Depacketizer Module
*
* @param    InstancePtr is a pointer to the XDepacketizer56 core instance.
*
* @return   XDepacketizer56_ModuleStatistic_RegValue.
*
* @note     None.
*
******************************************************************************/
void XDepacketizer56_VidFormat (XDepacketizer56 *InstancePtr)
{
    u32 RegValue;

    RegValue = XDepacketizer56_ReadReg((InstancePtr->Config.BaseAddress),
            (XDEPACKETIZER56_MEDIA_HEADER_REG_OFFSET));

    /* TS Include */
    XDepacketizer56_WriteReg((InstancePtr->Config.BaseAddress),
     (XDEPACKETIZER56_MEDIA_HEADER_REG_OFFSET),
      (((RegValue) & (~(XDEPACKETIZER56_MEDIA_HEADER_VIDEO_TS_INCLUDE_MASK))) |
       (((InstancePtr)->Stream.video_ts_include) &
         (XDEPACKETIZER56_MEDIA_HEADER_VIDEO_TS_INCLUDE_MASK))));

    /*************************************************************************/
    /* Setting User Cfg or Decode from Vid Src Fmt */

    RegValue = XDepacketizer56_ReadReg((InstancePtr->Config.BaseAddress),
            (XDEPACKETIZER56_FRAME_SIZE_REG_OFFSET));

    /* Last Datagram Length */
    XDepacketizer56_WriteReg((InstancePtr->Config.BaseAddress),
     (XDEPACKETIZER56_FRAME_SIZE_REG_OFFSET),
      (((RegValue) & (~(XDEPACKETIZER56_FRAME_SIZE_LAST_DATAGRAM_LEN_MASK))) |
       ((((InstancePtr)->Stream.last_datagram_len) <<
        (XDEPACKETIZER56_FRAME_SIZE_LAST_DATAGRAM_LEN_SHIFT)) &
         (XDEPACKETIZER56_FRAME_SIZE_LAST_DATAGRAM_LEN_MASK))));

    /*************************************************************************/

    RegValue = XDepacketizer56_ReadReg((InstancePtr->Config.BaseAddress),
            (XDEPACKETIZER56_FRAME_SIZE_REG_OFFSET));

    /* Datagram Per Frame */
    XDepacketizer56_WriteReg((InstancePtr->Config.BaseAddress),
     (XDEPACKETIZER56_FRAME_SIZE_REG_OFFSET),
      (((RegValue) & (~(XDEPACKETIZER56_FRAME_SIZE_LAST_DATAGRAM_LEN_MASK))) |
       ((((InstancePtr)->Stream.FrameCfg) <<
        (XDEPACKETIZER56_FRAME_SIZE_USER_CONFIG_SHIFT)) &
         (XDEPACKETIZER56_FRAME_SIZE_USER_CONFIG_MASK))));

    /*************************************************************************/

    RegValue = XDepacketizer56_ReadReg((InstancePtr->Config.BaseAddress),
            (XDEPACKETIZER56_FRAME_SIZE_REG_OFFSET));

    /* Last Datagram Length */
    XDepacketizer56_WriteReg((InstancePtr->Config.BaseAddress),
     (XDEPACKETIZER56_FRAME_SIZE_REG_OFFSET),
      (((RegValue) & (~(XDEPACKETIZER56_FRAME_SIZE_LAST_DATAGRAM_LEN_MASK))) |
       ((((InstancePtr)->Stream.last_datagram_len) <<
        (XDEPACKETIZER56_FRAME_SIZE_LAST_DATAGRAM_LEN_SHIFT)) &
         (XDEPACKETIZER56_FRAME_SIZE_LAST_DATAGRAM_LEN_MASK))));

    /*************************************************************************/

    RegValue = XDepacketizer56_ReadReg((InstancePtr->Config.BaseAddress),
            (XDEPACKETIZER56_VIDEO_FORMAT_REG_OFFSET));

    /* Bit Rate */
    XDepacketizer56_WriteReg((InstancePtr->Config.BaseAddress),
     (XDEPACKETIZER56_VIDEO_FORMAT_REG_OFFSET),
     (((RegValue) & (~(XDEPACKETIZER56_VIDEO_FORMAT_BITRATE_DIV_1_001_MASK))) |
       ((((InstancePtr)->Stream.rx_bitrate) <<
        (XDEPACKETIZER56_VIDEO_FORMAT_BITRATE_DIV_1_001_SHIFT)) &
         (XDEPACKETIZER56_VIDEO_FORMAT_BITRATE_DIV_1_001_MASK))));

    RegValue = XDepacketizer56_ReadReg((InstancePtr->Config.BaseAddress),
            (XDEPACKETIZER56_VIDEO_FORMAT_REG_OFFSET));

    /* 3G Level B */
    XDepacketizer56_WriteReg((InstancePtr->Config.BaseAddress),
     (XDEPACKETIZER56_VIDEO_FORMAT_REG_OFFSET),
      (((RegValue) & (~(XDEPACKETIZER56_VIDEO_FORMAT_3G_LEVEL_B_MASK))) |
       ((((InstancePtr)->Stream.level_b_3g) <<
        (XDEPACKETIZER56_VIDEO_FORMAT_3G_LEVEL_B_SHIFT)) &
         (XDEPACKETIZER56_VIDEO_FORMAT_3G_LEVEL_B_MASK))));

    RegValue = XDepacketizer56_ReadReg((InstancePtr->Config.BaseAddress),
            (XDEPACKETIZER56_VIDEO_FORMAT_REG_OFFSET));

    /* Video Mode */
    XDepacketizer56_WriteReg((InstancePtr->Config.BaseAddress),
     (XDEPACKETIZER56_VIDEO_FORMAT_REG_OFFSET),
      (((RegValue) & (~(XDEPACKETIZER56_VIDEO_FORMAT_VIDEO_MODE_MASK))) |
       (((InstancePtr)->Stream.sdi_mode) &
        (XDEPACKETIZER56_VIDEO_FORMAT_VIDEO_MODE_MASK))));
}

/*****************************************************************************/
/**
*
* This function configures the RTP Media Header/ST2022-6 Header into
* ST2022-6 Depacketizer Module
*
* @param    InstancePtr is a pointer to the XDepacketizer56 core instance.
*
* @return   XDepacketizer56_ModuleStatistic_RegValue.
*
* @note     None.
*
******************************************************************************/
void XDepacketizer56_RTPMediaHeader (XDepacketizer56 *InstancePtr)
{
    u32 RegValue;

    RegValue = XDepacketizer56_ReadReg((InstancePtr->Config.BaseAddress),
            (XDEPACKETIZER56_MAP_FRAME_FRATE_SAMPLE_FMT_REG_OFFSET));

    /* MAP */
    XDepacketizer56_WriteReg((InstancePtr->Config.BaseAddress),
    (XDEPACKETIZER56_MAP_FRAME_FRATE_SAMPLE_FMT_REG_OFFSET),
     (((RegValue) & (~(XDEPACKETIZER56_MAP_FRAME_FRATE_SAMPLE_FMT_MAP_MASK))) |
       ((((InstancePtr)->Stream.MAP) <<
        (XDEPACKETIZER56_MAP_FRAME_FRATE_SAMPLE_FMT_MAP_SHIFT)) &
         (XDEPACKETIZER56_MAP_FRAME_FRATE_SAMPLE_FMT_MAP_MASK))));

    RegValue = XDepacketizer56_ReadReg((InstancePtr->Config.BaseAddress),
            (XDEPACKETIZER56_MAP_FRAME_FRATE_SAMPLE_FMT_REG_OFFSET));

    /* FRAME */
    XDepacketizer56_WriteReg((InstancePtr->Config.BaseAddress),
     (XDEPACKETIZER56_MAP_FRAME_FRATE_SAMPLE_FMT_REG_OFFSET),
   (((RegValue) & (~(XDEPACKETIZER56_MAP_FRAME_FRATE_SAMPLE_FMT_FRAME_MASK))) |
        ((((InstancePtr)->Stream.FRAME) <<
         (XDEPACKETIZER56_MAP_FRAME_FRATE_SAMPLE_FMT_FRAME_SHIFT)) &
          (XDEPACKETIZER56_MAP_FRAME_FRATE_SAMPLE_FMT_FRAME_MASK))));

    RegValue = XDepacketizer56_ReadReg((InstancePtr->Config.BaseAddress),
            (XDEPACKETIZER56_MAP_FRAME_FRATE_SAMPLE_FMT_REG_OFFSET));

    /* FRATE */
    XDepacketizer56_WriteReg((InstancePtr->Config.BaseAddress),
     (XDEPACKETIZER56_MAP_FRAME_FRATE_SAMPLE_FMT_REG_OFFSET),
      (((RegValue) &
       (~(XDEPACKETIZER56_MAP_FRAME_FRATE_SAMPLE_FMT_FRATE_MASK))) |
        ((((InstancePtr)->Stream.FRATE) <<
         (XDEPACKETIZER56_MAP_FRAME_FRATE_SAMPLE_FMT_FRATE_SHIFT)) &
          (XDEPACKETIZER56_MAP_FRAME_FRATE_SAMPLE_FMT_FRATE_MASK))));

    RegValue = XDepacketizer56_ReadReg((InstancePtr->Config.BaseAddress),
            (XDEPACKETIZER56_MAP_FRAME_FRATE_SAMPLE_FMT_REG_OFFSET));

    /* SAMPLE */
    XDepacketizer56_WriteReg((InstancePtr->Config.BaseAddress),
     (XDEPACKETIZER56_MAP_FRAME_FRATE_SAMPLE_FMT_REG_OFFSET),
      (((RegValue) &
       (~(XDEPACKETIZER56_MAP_FRAME_FRATE_SAMPLE_FMT_SAMPLE_MASK))) |
        ((((InstancePtr)->Stream.SAMPLE) <<
         (XDEPACKETIZER56_MAP_FRAME_FRATE_SAMPLE_FMT_SAMPLE_SHIFT)) &
          (XDEPACKETIZER56_MAP_FRAME_FRATE_SAMPLE_FMT_SAMPLE_MASK))));
}

/*****************************************************************************/
/**
*
* This function obtains the current Module Status of ST2022-6 Depacketizer
* Module
*
* @param    InstancePtr is a pointer to the XDepacketizer56 core instance.
*
* @return   XDepacketizer56_ModuleStatistic_RegValue.
*
* @note     None.
*
******************************************************************************/
XDepacketizer56_ModuleStatus XDepacketizer56_ErroStatus
                                               (XDepacketizer56 *InstancePtr)
{
    XDepacketizer56_ModuleStatus ModuleStatusRegValue;
    u32 RegValue;

    RegValue = XDepacketizer56_ReadReg((InstancePtr->Config.BaseAddress),
            (XDEPACKETIZER56_ERROR_STATUS_REG_OFFSET));

    ModuleStatusRegValue.ErrDetected_Datagram_per_Frame =
          ((RegValue) &
               (XDEPACKETIZER56_ERROR_STATUS_DET_DATAGRAM_PER_FRAME_MASK));

    ModuleStatusRegValue.ErrDetected_Datagram_per_Frame = (((RegValue) &
          (XDEPACKETIZER56_ERROR_STATUS_BUFFER_LEVEL_MASK)) >>
              (XDEPACKETIZER56_ERROR_STATUS_BUFFER_LEVEL_SHIFT));

    return(ModuleStatusRegValue);
}

/*****************************************************************************/
/**
*
* This function obtains the current Module Statistic of ST2022-6 Depacketizer
* Module
*
* @param    InstancePtr is a pointer to the XDepacketizer56 core instance.
*
* @return   XDepacketizer56_ModuleStatistic_RegValue.
*
* @note     None.
*
******************************************************************************/
XDepacketizer56_ModuleStatistic XDepacketizer56_Statistics
                                                (XDepacketizer56 *InstancePtr)
{
    XDepacketizer56_ModuleStatistic ModuleStatisticRegValue;
    u32 RegValue;

    RegValue = XDepacketizer56_ReadReg((InstancePtr->Config.BaseAddress),
            (XDEPACKETIZER56_INPUT_PKT_CNT_REG_OFFSET));

    ModuleStatisticRegValue.input_pkt_cnt = RegValue;

    RegValue = XDepacketizer56_ReadReg((InstancePtr->Config.BaseAddress),
            (XDEPACKETIZER56_TRANSMIT_FRAME_CNT_REG_OFFSET));

    ModuleStatisticRegValue.sdi_tx_frame_cnt = RegValue;

    RegValue = (XDepacketizer56_ReadReg((InstancePtr->Config.BaseAddress),
            (XDEPACKETIZER56_BUF_LEVEL_REG_OFFSET)) &
                (XDEPACKETIZER56_BUF_LEVEL_PEAK_BUFFER_LEVEL_MASK));

    ModuleStatisticRegValue.peak_buf_level = RegValue;

    RegValue = ((XDepacketizer56_ReadReg((InstancePtr->Config.BaseAddress),
            (XDEPACKETIZER56_BUF_LEVEL_REG_OFFSET)) &
                (XDEPACKETIZER56_BUF_LEVEL_CURR_BUFFER_LEVEL_MASK)) >>
                    (XDEPACKETIZER56_BUF_LEVEL_CURR_BUFFER_LEVEL_SHIFT));

    ModuleStatisticRegValue.curr_buf_level = RegValue;


    return(ModuleStatisticRegValue);
}

/*****************************************************************************/
/**
*
* This function perform Module Statistic reset of ST2022-6 Depacketizer
* Module
*
* @param    InstancePtr is a pointer to the XDepacketizer56 core instance.
*
* @return   XDepacketizer56_ModuleStatistic_RegValue.
*
* @note     None.
*
******************************************************************************/
void XDepacketizer56_ResetStatistic (XDepacketizer56 *InstancePtr){

    XDepacketizer56_WriteReg((InstancePtr->Config.BaseAddress),
       (XDEPACKETIZER56_STATISTIC_CLEAR_REG_OFFSET),
           (XDEPACKETIZER56_STATISTIC_CLEAR_MASK));
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
static void StubCallback(void *CallbackRef)
{
    Xil_AssertVoid(CallbackRef != NULL);
    Xil_AssertVoidAlways();
}
