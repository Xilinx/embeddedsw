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

#include "xpacketizer56.h"
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
* This function initializes the VoIP ST2022-6 Packetizer core. This function
* must be called prior to using the VoIP ST2022-6 Packetizer core.
* Initialization of the VoIP ST2022-6 Packetizer includes setting up the
* instance data, and ensuring the hardware is in a quiescent state.
*
* @param    InstancePtr is a pointer to the VoIP ST2022-6 Packetizer core
        instance.
* @param    CfgPtr points to the configuration structure associated with
*       the HDMI RX core.
* @param    EffectiveAddr is the base address of the device. If address
*       translation is being used, then this parameter must reflect the
*       virtual base address. Otherwise, the physical address should be
*       used.
*
* @return
*       - XST_SUCCESS if XPacketizer56_CfgInitialize was successful.
*
* @note     None.
*
******************************************************************************/
int XPacketizer56_CfgInitialize(XPacketizer56 *InstancePtr,
                                XPacketizer56_Config *CfgPtr,
                                          UINTPTR EffectiveAddr)
{
    /* Verify arguments. */
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(CfgPtr != NULL);
    Xil_AssertNonvoid(EffectiveAddr != (UINTPTR)0x0);

    /* Setup the instance */
    (void)memset((void *)InstancePtr, 0, sizeof(XPacketizer56));
    (void)memcpy((void *)&(InstancePtr->Config), (const void *)CfgPtr,
                                                  sizeof(XPacketizer56_Config));
    InstancePtr->Config.BaseAddress = EffectiveAddr;

    /*
      Callbacks
      These are placeholders pointing to the StubCallback
      The actual callback pointers will be assigned by the SetCallback function
    */
    InstancePtr->DatagramMismatchCallback =
                                 (XPacketizer56_Callback)((void *)StubCallback);
    InstancePtr->IsDatagramMismatchCallbackSet = (FALSE);

    /* Clear VoIP ST2022-6 Packetizer Register Space*/
    XPacketizer56_RegClear(InstancePtr);

    /* Clear connected flag */
    InstancePtr->Stream->IsConnected = (FALSE);

    /* Disable the Module */
    InstancePtr->ModuleEnable = XPACKETIZER56_MODULE_DISABLE;

    /* Set Operating Mode to Normal Mode */
    InstancePtr->LoslessMode = XPACKETIZER56_NORMAL;


    /* Set Mask Interrupt/Enable Interrupt */

    /* Reset the hardware and set the flag to indicate the driver is ready */
    InstancePtr->IsReady = (u32)(XIL_COMPONENT_IS_READY);

    return (XST_SUCCESS);
}

/*****************************************************************************/
/**
*
* This function Clears all the VoIP ST2022-6 Packetizer Register Space
*
* @param    InstancePtr is a pointer to the XPacketizer56 core instance.
*
* @return   None.
*
* @note     None.
*
******************************************************************************/
void XPacketizer56_RegClear(XPacketizer56 *InstancePtr)
{
    u32 RegValue;

    RegValue = XPacketizer56_ReadReg((InstancePtr)->Config.BaseAddress,
        (XPACKETIZER56_CONTROL_REG_OFFSET)) &
            ((~(XPACKETIZER56_CONTROL_SOFT_RESET_MASK)));

    /* Set the Clear Bit */
    XPacketizer56_WriteReg((InstancePtr)->Config.BaseAddress,
        (XPACKETIZER56_CONTROL_REG_OFFSET),
            (RegValue | XPACKETIZER56_CONTROL_SOFT_RESET_MASK));

    RegValue = XPacketizer56_ReadReg((InstancePtr)->Config.BaseAddress,
        (XPACKETIZER56_CONTROL_REG_OFFSET)) &
            ((~(XPACKETIZER56_CONTROL_SOFT_RESET_MASK)));

    /* Clear the Clear Bit */
    XPacketizer56_WriteReg((InstancePtr)->Config.BaseAddress,
        (XPACKETIZER56_CONTROL_REG_OFFSET),
            (RegValue & (~(XPACKETIZER56_CONTROL_SOFT_RESET_MASK))));
}

/*****************************************************************************/
/**
*
* This function Sets the Operating Mode based on User Configure Value in
* VoIP ST2022-6 Packetizer Structure of LoslessMode
*   LoslessMode  : XPACKETIZER56_NORMAL    : Normal Mode
*                  XPACKETIZER56_LOSSLESS  : Lossless Mode
*
* @param    InstancePtr is a pointer to the XPacketizer56 core instance.
*
* @return   None.
*
* @note     None.
*
******************************************************************************/
void XPacketizer56_LosslessEnable (XPacketizer56 *InstancePtr)
{
    u32 RegValue;

    RegValue = XPacketizer56_ReadReg((InstancePtr)->Config.BaseAddress,
        (XPACKETIZER56_MODULE_CONTROL_REG_OFFSET)) &
            (~(XPACKETIZER56_MODULE_CONTROL_LOSSLESS_MASK));


    XPacketizer56_WriteReg((InstancePtr)->Config.BaseAddress,
        (XPACKETIZER56_MODULE_CONTROL_REG_OFFSET),
            (RegValue | ((InstancePtr->LoslessMode <<
                (XPACKETIZER56_MODULE_CONTROL_LOSSLESS_SHIFT) &
                    (XPACKETIZER56_MODULE_CONTROL_LOSSLESS_MASK)))));
}

/*****************************************************************************/
/**
*
* This function Enable or Disable the Module based on User Configured Structure
* of ModuleEnable
*
* @param    InstancePtr is a pointer to the XPacketizer56 core instance.
*
* @return   None.
*
* @note     None.
*
******************************************************************************/
void XPacketizer56_ModEnable (XPacketizer56 *InstancePtr)
{
    u32 RegValue;

    RegValue = XPacketizer56_ReadReg((InstancePtr)->Config.BaseAddress,
        (XPACKETIZER56_MODULE_CONTROL_REG_OFFSET)) &
            (~(XPACKETIZER56_MODULE_CONTROL_ENABLE_MASK));

    XPacketizer56_WriteReg((InstancePtr)->Config.BaseAddress,
        (XPACKETIZER56_MODULE_CONTROL_REG_OFFSET),
            (RegValue | (InstancePtr->ModuleEnable &
                (XPACKETIZER56_MODULE_CONTROL_ENABLE_MASK))));
}

/*****************************************************************************/
/**
*
* This function Writes the Video Format into the VoIP ST2022-6 Register.
* This register is valid if the core is set to obtained video format from the
* Register space.
*
* @param    InstancePtr is a pointer to the XPacketizer56 core instance.
*
* @return   None.
*
* @note     None.
*
******************************************************************************/
void XPacketizer56_VidFormat (XPacketizer56 *InstancePtr)
{
    u32 RegValue;

    RegValue = XPacketizer56_ReadReg((InstancePtr->Config.BaseAddress),
                                       (XPACKETIZER56_VIDEO_FORMAT_REG_OFFSET));

    /* Bit Rate */
    XPacketizer56_WriteReg((InstancePtr->Config.BaseAddress),
         (XPACKETIZER56_VIDEO_FORMAT_REG_OFFSET),
            (((RegValue) &
               (~(XPACKETIZER56_VIDEO_FORMAT_BITRATE_DIV_1_001_MASK))) |
                  ((((InstancePtr)->Stream->rx_bitrate) <<
                     (XPACKETIZER56_VIDEO_FORMAT_BITRATE_DIV_1_001_SHIFT)) &
                         (XPACKETIZER56_VIDEO_FORMAT_BITRATE_DIV_1_001_MASK))));

    RegValue = XPacketizer56_ReadReg((InstancePtr->Config.BaseAddress),
                                       (XPACKETIZER56_VIDEO_FORMAT_REG_OFFSET));

    /* 3G Level B */
    XPacketizer56_WriteReg((InstancePtr->Config.BaseAddress),
         (XPACKETIZER56_VIDEO_FORMAT_REG_OFFSET),
            (((RegValue) & (~(XPACKETIZER56_VIDEO_FORMAT_3G_LEVEL_B_MASK))) |
                    ((((InstancePtr)->Stream->level_b_3g) <<
                       (XPACKETIZER56_VIDEO_FORMAT_3G_LEVEL_B_SHIFT)) &
                            (XPACKETIZER56_VIDEO_FORMAT_3G_LEVEL_B_MASK))));

    RegValue = XPacketizer56_ReadReg((InstancePtr->Config.BaseAddress),
                                       (XPACKETIZER56_VIDEO_FORMAT_REG_OFFSET));

    /* Video Mode */
    XPacketizer56_WriteReg((InstancePtr->Config.BaseAddress),
        (XPACKETIZER56_VIDEO_FORMAT_REG_OFFSET),
            (((RegValue) & (~(XPACKETIZER56_VIDEO_FORMAT_VIDEO_MODE_MASK))) |
                    (((InstancePtr)->Stream->sdi_mode) &
                               (XPACKETIZER56_VIDEO_FORMAT_VIDEO_MODE_MASK))));
}

/*****************************************************************************/
/**
*
* This function Writes the RTP-Media Header into the VoIP ST2022-6 Register.
*
* @param    InstancePtr is a pointer to the XPacketizer56 core instance.
*
* @return   None.
*
* @note     None.
*
******************************************************************************/
void XPacketizer56_RTPMediaHeader (XPacketizer56 *InstancePtr)
{
    u32 RegValue;

    RegValue = XPacketizer56_ReadReg((InstancePtr->Config.BaseAddress),
                                        (XPACKETIZER56_VID_SRC_FMT_REG_OFFSET));

    /* MAP */
    XPacketizer56_WriteReg((InstancePtr->Config.BaseAddress),
        (XPACKETIZER56_VID_SRC_FMT_REG_OFFSET),
            (((RegValue) &
              (~(XPACKETIZER56_MAP_FRAME_FRATE_SAMPLE_FMT_MAP_MASK))) |
                ((((InstancePtr)->Stream->MAP) <<
                  (XPACKETIZER56_MAP_FRAME_FRATE_SAMPLE_FMT_MAP_SHIFT)) &
                     (XPACKETIZER56_MAP_FRAME_FRATE_SAMPLE_FMT_MAP_MASK))));

    RegValue = XPacketizer56_ReadReg((InstancePtr->Config.BaseAddress),
                                        (XPACKETIZER56_VID_SRC_FMT_REG_OFFSET));

    /* FRAME */
    XPacketizer56_WriteReg((InstancePtr->Config.BaseAddress),
       (XPACKETIZER56_VID_SRC_FMT_REG_OFFSET),
            (((RegValue) &
               (~(XPACKETIZER56_MAP_FRAME_FRATE_SAMPLE_FMT_FRAME_MASK))) |
                 ((((InstancePtr)->Stream->FRAME) <<
                   (XPACKETIZER56_MAP_FRAME_FRATE_SAMPLE_FMT_FRAME_SHIFT)) &
                      (XPACKETIZER56_MAP_FRAME_FRATE_SAMPLE_FMT_FRAME_MASK))));

    RegValue = XPacketizer56_ReadReg((InstancePtr->Config.BaseAddress),
                                        (XPACKETIZER56_VID_SRC_FMT_REG_OFFSET));

    /* FRATE */
    XPacketizer56_WriteReg((InstancePtr->Config.BaseAddress),
       (XPACKETIZER56_VID_SRC_FMT_REG_OFFSET),
           (((RegValue) &
             (~(XPACKETIZER56_MAP_FRAME_FRATE_SAMPLE_FMT_FRATE_MASK))) |
                ((((InstancePtr)->Stream->FRATE) <<
                   (XPACKETIZER56_MAP_FRAME_FRATE_SAMPLE_FMT_FRATE_SHIFT)) &
                       (XPACKETIZER56_MAP_FRAME_FRATE_SAMPLE_FMT_FRATE_MASK))));

    RegValue = XPacketizer56_ReadReg((InstancePtr->Config.BaseAddress),
                                        (XPACKETIZER56_VID_SRC_FMT_REG_OFFSET));

    /* SAMPLE */
    XPacketizer56_WriteReg((InstancePtr->Config.BaseAddress),
         (XPACKETIZER56_VID_SRC_FMT_REG_OFFSET),
            (((RegValue) &
              (~(XPACKETIZER56_MAP_FRAME_FRATE_SAMPLE_FMT_SAMPLE_MASK))) |
                ((((InstancePtr)->Stream->SAMPLE) <<
                  (XPACKETIZER56_MAP_FRAME_FRATE_SAMPLE_FMT_SAMPLE_SHIFT)) &
                     (XPACKETIZER56_MAP_FRAME_FRATE_SAMPLE_FMT_SAMPLE_MASK))));

    /* SSRC */
    XPacketizer56_WriteReg((InstancePtr->Config.BaseAddress),
       (XPACKETIZER56_SSRC_REG_OFFSET),
           ((InstancePtr)->Stream->ssrc));
}


/*****************************************************************************/
/**
*
* This function Writes the RTP-Media Header into the VoIP ST2022-6 Register.
*
* @param    InstancePtr is a pointer to the XPacketizer56 core instance.
*
* @return   None.
*
* @note     None.
*
******************************************************************************/
XPacketizer56_Stream XPacketizer56_MediaDatagramInfo(XPacketizer56 *InstancePtr)
{
    u32 map_frame_frate_sample;

    map_frame_frate_sample =
               (((((InstancePtr->Stream->MAP) <<
                (XPACKETIZER56_MAP_FRAME_FRATE_SAMPLE_FMT_MAP_SHIFT))) &
                    (XPACKETIZER56_MAP_FRAME_FRATE_SAMPLE_FMT_MAP_MASK)) |
               ((((InstancePtr->Stream->FRAME) <<
                (XPACKETIZER56_MAP_FRAME_FRATE_SAMPLE_FMT_FRAME_SHIFT))) &
                    (XPACKETIZER56_MAP_FRAME_FRATE_SAMPLE_FMT_FRAME_MASK)) |
               ((((InstancePtr->Stream->FRATE) <<
                (XPACKETIZER56_MAP_FRAME_FRATE_SAMPLE_FMT_FRATE_SHIFT))) &
                    (XPACKETIZER56_MAP_FRAME_FRATE_SAMPLE_FMT_FRATE_MASK)) |
               ((((InstancePtr->Stream->SAMPLE) <<
                (XPACKETIZER56_MAP_FRAME_FRATE_SAMPLE_FMT_SAMPLE_SHIFT))) &
                    (XPACKETIZER56_MAP_FRAME_FRATE_SAMPLE_FMT_SAMPLE_MASK)) &
                        0xFFFFFF00);
}

/*****************************************************************************/
/**
*
* This function Sets the Channel Number Information which will be added in to
* TUSER Master AXI4-Stream of the Module
*
* @param    InstancePtr is a pointer to the XPacketizer56 core instance.
*
* @return   None.
*
* @note     None.
*
******************************************************************************/
void XPacketizer56_SetChannel (XPacketizer56 *InstancePtr)
{
    u32 RegValue;

    RegValue = XPacketizer56_ReadReg((InstancePtr->Config.BaseAddress),
                                     (XPACKETIZER56_MODULE_CONTROL_REG_OFFSET));

    XPacketizer56_WriteReg((InstancePtr)->Config.BaseAddress,
       (XPACKETIZER56_MODULE_CONTROL_REG_OFFSET),
            (RegValue | (((InstancePtr)->Channel_Number <<
                (XPACKETIZER56_MODULE_CONTROL_CHANNEL_NUM_SHIFT)) &
                    (XPACKETIZER56_MODULE_CONTROL_CHANNEL_NUM_MASK))));
}

/*****************************************************************************/
/**
*
* This function Sets the RTP-SSRC Header
*
* @param    InstancePtr is a pointer to the XPacketizer56 core instance.
*
* @return   None.
*
* @note     None.
*
******************************************************************************/
void XPacketizer56_SetSSRC (XPacketizer56 *InstancePtr)
{
    /* SSRC */
    XPacketizer56_WriteReg((InstancePtr->Config.BaseAddress),
        (XPACKETIZER56_SSRC_REG_OFFSET),
            ((InstancePtr)->Stream->ssrc));
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
