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
* @file xdecap.c
*
* This is the main file for Xilinx VoIP Decapsulator core. Please see
* xdecap.h for more details of the driver.
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

#include "xdecap.h"
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
* This function initializes the VoIP Decapsulator core. This function must
* be called prior to using the VoIP Decapsulator core. Initialization of the
* VoIP Decapsulator includes setting up the instance data and ensuring the
* hardware is in a quiescent state.
*
* @param    InstancePtr is a pointer to the XDecap core instance.
* @param    CfgPtr points to the configuration structure associated with
*       the VoIP Decapsulator core.
* @param    EffectiveAddr is the base address of the device. If address
*       translation is being used, then this parameter must reflect the
*       virtual base address. Otherwise, the physical address should be
*       used.
*
* @return
*       - XST_SUCCESS if XDecap_CfgInitialize was successful.
*       - XST_FAILURE if Channel Number has mismatched with value reflected
*       on the register, and if the operating channel number is higher tha
*       hardware supported channel number
*
* @note     None.
*
******************************************************************************/
int XDecap_CfgInitialize(XDecap *InstancePtr, XDecap_Config *CfgPtr,
                UINTPTR EffectiveAddr)
{
    u32 Index;
    XDecap_Config XDecap_Config_RegValue;

    /* Verify arguments. */
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(CfgPtr != NULL);
    Xil_AssertNonvoid(EffectiveAddr != (UINTPTR)0x0);

    /* Setup the instance */
    (void)memset((void *)InstancePtr, 0, sizeof(XDecap));
    (void)memcpy((void *)&(InstancePtr->Config), (const void *)CfgPtr,
            sizeof(XDecap_Config));
    InstancePtr->Config.BaseAddress = EffectiveAddr;

    /*
      Callbacks
      These are placeholders pointing to the StubCallback
      The actual callback pointers will be assigned by the SetCallback function
    */

    InstancePtr->PacketLockCallback = (XDecap_Callback)((void *)StubCallback);
    InstancePtr->IsPacketLockCallbackSet = (FALSE);

    InstancePtr->PacketUnLockCallback = (XDecap_Callback)((void *)StubCallback);
    InstancePtr->IsPacketUnLockCallbackSet = (FALSE);

    InstancePtr->PacketStopCallback = (XDecap_Callback)((void *)StubCallback);
    InstancePtr->IsPacketStopCallbackSet = (FALSE);

    /* Set the data structure based on hardware settings */
    InstancePtr->Config.HWChannelNumber  = CfgPtr->HWChannelNumber;

    XDecap_Config_RegValue = XDecap_CoreInfo(InstancePtr);
    /* Verify the Hardware Setting = Register Value */
    if ((u16)InstancePtr->Config.HWChannelNumber !=
             (u16)XDecap_Config_RegValue.HWChannelNumber) {
        xil_printf("Ch Number: Hardware: %d <-> Register: %d\n\r",
            XDecap_Config_RegValue.HWChannelNumber,
                InstancePtr->Config.HWChannelNumber);
        return (XST_FAILURE);
    }

    /*Verify the Current Operation Channel is less than HW Supported Channel */
    if ((u16)InstancePtr->Config.HWChannelNumber <
        (u16)InstancePtr->OpChannel) {
        xil_printf("Ch Number: Operation: %d <-> HW Supported: %d\n\r",
        InstancePtr->OpChannel,InstancePtr->Config.HWChannelNumber);
        return (XST_FAILURE);
    }

    /* Clear the registers */
    XDecap_SoftReset(InstancePtr);

    /* Check The Busy Bit - Wait the Busy Bit = 0 */
    Index = 0x00;
    while(XDecap_BusyBit(InstancePtr)){
        if (Index == 65535){
            xil_printf("Error: VoIP Decapsulator Busy Bit Longer than \
            Expected\n\r");
            break;
        }
        Index ++;
    }

    /* Current Interrupt Status */
    InstancePtr->PacketLockInterrupt       = (FALSE);
    InstancePtr->PacketUnLockInterrupt     = (FALSE);
    InstancePtr->PacketStopInterrupt       = (FALSE);

    /* Disable the VoIP Decapsulator */
    InstancePtr->ModuleEnable = XDECAP_MODULE_DISABLE;
    XDecap_ModuleEn(InstancePtr);

    /* Assign Necessary Parameters on Decap */
    for (Index = 0x00; Index < InstancePtr->OpChannel; Index++) {
        InstancePtr->ChannelCfg[Index].LosslessMode      = XDECAP_NORMAL_MODE;
        InstancePtr->ChannelCfg[Index].is_MatchVlanPckt  =
                                             XDECAP_MATCH_PACKET_WITHOUT_VLAN;
        InstancePtr->ChannelCfg[Index].Channel_Enable    =
                                                       XDECAP_CHANNEL_DISABLE;
    }

    /* Reset the hardware and set the flag to indicate the driver is ready */
    InstancePtr->IsReady = (u32)(XIL_COMPONENT_IS_READY);

    return (XST_SUCCESS);

}

/*****************************************************************************/
/**
*
* This function configures the Operating channel of VoIP Decapsulator and
* the general setting of VoIP Decapsulator
*
* @param    InstancePtr is a pointer to the XDecap core instance.
*
* @return   None.
*
* @note     None.
*
******************************************************************************/
void XDecap_CoreChannelConfig(XDecap *InstancePtr)
{
    u16 Index;

    /* Setting Parameters based on Drivers */
    for (Index=(u16)0x00; Index<(u16)((InstancePtr)->OpChannel); Index++) {
        /* Perform Channel Config */
        XDecap_ChannelConfig(InstancePtr, Index);

        /* Update the Channels */
        XDecap_ChannelUpdate(InstancePtr);
    }

    /* Set the lock/unlock window parameter */
    XDecap_PacketLockWindow(InstancePtr);
    XDecap_PacketUnLockWindow(InstancePtr);

    /* Enable the Decap */
    XDecap_ModuleEn(InstancePtr);
}


/*****************************************************************************/
/**
*
* This function Configures the VoIP Decapsulator Channels Parameter
*
* @param    InstancePtr is a pointer to the XDecap core instance.
*
* @param    Channels is current configured channel
*
* @return   None.
*
* @note     None.
*
******************************************************************************/
void XDecap_ChannelConfig(XDecap *InstancePtr, u16 Channels)
{

    /* Select Channels */
    XDecap_ChannelAccess(InstancePtr,Channels);

    /* Set Decap Configuration */
    /* Set Operation Mode (Normal or Lossless) */
    XDecap_SetOperationMode(InstancePtr,Channels);

    /* Set Match Select */
    XDecap_SetMatchSelect(InstancePtr, Channels);

    /* Set Match Parameters */
    XDecap_MatchIPv4Dest(InstancePtr,Channels);
    XDecap_MatchIPv4Src(InstancePtr,Channels);
    XDecap_MatchUDPDest(InstancePtr,Channels);
    XDecap_MatchUDPSrc(InstancePtr,Channels);
    XDecap_MatchVLANID(InstancePtr,Channels);
    XDecap_MatchSSRC(InstancePtr,Channels);

    /* Set other Decap Modes */
    XDecap_ToMatchVLANID(InstancePtr,Channels);

    /* Set Packet Timer Stop (Only for Incoming ST2022-6 Packets */
    XDecap_PacketStopTimer(InstancePtr,Channels);

    /* Set Marker Packet Detection Enable/Disable */
    XDecap_ControlMPktDetEn(InstancePtr,Channels);

    /* Set Marker Packet Drop Enable/Disable */
    XDecap_ControlMPktDropEn(InstancePtr,Channels);

    /* Set Channel Enable/Disable */
    XDecap_ControlChannelEn(InstancePtr,Channels);
}


/*****************************************************************************/
/**
*
* This function reads the channel configured value of VoIP Decapsulator
* from register
*
* @param    InstancePtr is a pointer to the XDecap core instance.
*
* @param    Channels is current configured channel
*
* @return   XDecap_ChannelCfg_RegValue, which contains the channel
*           configuration component
*
* @note     None.
*
******************************************************************************/
XDecap_ChannelCfg XDecap_ChStatus(XDecap *InstancePtr, u16 Channels)
{
    XDecap_ChannelCfg XDecap_ChannelCfg_RegValue;

    /* Select Channels */
    XDecap_ChannelAccess(InstancePtr,Channels);

    /* Channel Enable */
    XDecap_ChannelCfg_RegValue.Channel_Enable =
        (XDecap_ReadReg(InstancePtr->Config.BaseAddress,
            XDECAP_CHANNEL_CONTROL_OFFSET) &
                (XDECAP_CHANNEL_CONTROL_CHANNEL_ENABLE_MASK));

    /* Operation Mode */
    XDecap_ChannelCfg_RegValue.LosslessMode =
        ((XDecap_ReadReg(InstancePtr->Config.BaseAddress,
            XDECAP_CHANNEL_CONTROL_OFFSET) &
                (XDECAP_CHANNEL_CONTROL_LOSSLESS_MODE_MASK)) >>
                    (XDECAP_CHANNEL_CONTROL_LOSSLESS_MODE_SHIFT));

    /* Filter Setting */
    XDecap_ChannelCfg_RegValue.MatchSelect.match_vlan_reg =
        (XDecap_ReadReg(InstancePtr->Config.BaseAddress,
            XDECAP_MATCH_SELECT_OFFSET) &
                (XDECAP_MATCH_SELECT_TO_MATCH_VLAN_ID_REG_MASK));

    XDecap_ChannelCfg_RegValue.MatchSelect.match_ip_src =
        (XDecap_ReadReg(InstancePtr->Config.BaseAddress,
            XDECAP_MATCH_SELECT_OFFSET) &
                (XDECAP_MATCH_SELECT_TO_MATCH_SRC_IP_MASK)) >>
                    XDECAP_MATCH_SELECT_TO_MATCH_SRC_IP_SHIFT;

    XDecap_ChannelCfg_RegValue.MatchSelect.match_ip_dst =
        (XDecap_ReadReg(InstancePtr->Config.BaseAddress,
            XDECAP_MATCH_SELECT_OFFSET) &
                (XDECAP_MATCH_SELECT_TO_MATCH_DEST_IP_MASK)) >>
                    XDECAP_MATCH_SELECT_TO_MATCH_DEST_IP_SHIFT;

    XDecap_ChannelCfg_RegValue.MatchSelect.match_udp_src =
        (XDecap_ReadReg(InstancePtr->Config.BaseAddress,
            XDECAP_MATCH_SELECT_OFFSET) &
                (XDECAP_MATCH_SELECT_TO_MATCH_UDP_SRC_MASK)) >>
                    XDECAP_MATCH_SELECT_TO_MATCH_UDP_SRC_SHIFT;

    XDecap_ChannelCfg_RegValue.MatchSelect.match_udp_dst =
        (XDecap_ReadReg(InstancePtr->Config.BaseAddress,
            XDECAP_MATCH_SELECT_OFFSET) &
                (XDECAP_MATCH_SELECT_TO_MATCH_UDP_DEST_MASK)) >>
                    XDECAP_MATCH_SELECT_TO_MATCH_UDP_DEST_SHIFT;

    XDecap_ChannelCfg_RegValue.MatchSelect.match_ssrc =
        (XDecap_ReadReg(InstancePtr->Config.BaseAddress,
            XDECAP_MATCH_SELECT_OFFSET) &
                (XDECAP_MATCH_SELECT_TO_MATCH_SSRC_MASK)) >>
                    XDECAP_MATCH_SELECT_TO_MATCH_SSRC_SHIFT;

    /* Register XDECAP_MATCH_DEST_IP0 */
    XDecap_ChannelCfg_RegValue.MatchHeader.ipv4_dest_address =
        XDecap_ReadReg(InstancePtr->Config.BaseAddress,
            XDECAP_MATCH_DEST_IP0_OFFSET);

    /* Register XDECAP_MATCH_SRC_IP0 */
    XDecap_ChannelCfg_RegValue.MatchHeader.ipv4_src_address =
        XDecap_ReadReg(InstancePtr->Config.BaseAddress,
            XDECAP_MATCH_SRC_IP0_OFFSET);

    /* Register XGENERIC_DECAP_MATCH_UDP_PORT */
    XDecap_ChannelCfg_RegValue.MatchHeader.udp_dest_port =
        (XDecap_ReadReg(InstancePtr->Config.BaseAddress,
            XDECAP_MATCH_UDP_DEST_PORT_OFFSET) &
                (XDECAP_MATCH_UDP_DEST_PORT_MASK));

    XDecap_ChannelCfg_RegValue.MatchHeader.udp_src_port =
        (XDecap_ReadReg(InstancePtr->Config.BaseAddress,
            XDECAP_MATCH_UDP_SRC_PORT_OFFSET) &
                (XDECAP_MATCH_UDP_SRC_PORT_MASK));

    /* Register XDECAP_VLAN_ID */
    XDecap_ChannelCfg_RegValue.is_MatchVlanPckt =
        (XDecap_ReadReg(InstancePtr->Config.BaseAddress,
            XDECAP_MATCH_VLAN_ID_OFFSET) &
                (XDECAP_MATCH_VLAN_ID_VLAN_FILTER_ENABLE_MASK)) >>
                    XDECAP_MATCH_VLAN_ID_VLAN_FILTER_ENABLE_SHIFT;

    XDecap_ChannelCfg_RegValue.MatchHeader.vlan_pcp_cfi_vid =
        (XDecap_ReadReg(InstancePtr->Config.BaseAddress,
            XDECAP_MATCH_VLAN_ID_OFFSET) &
                (XDECAP_MATCH_VLAN_ID_VLAN_TAG_MASK));

    /* Register XDECAP_MATCH_SSRC */
    XDecap_ChannelCfg_RegValue.MatchHeader.ssrc =
        XDecap_ReadReg(InstancePtr->Config.BaseAddress,
            XDECAP_MATCH_SSRC_OFFSET);

    return (XDecap_ChannelCfg_RegValue);
}

/*****************************************************************************/
/**
*
* This function reads the Core Information (Configured by user through GUI),
* in the VoIP Decapsulator Register
*
* @param    InstancePtr is a pointer to the XDecap core instance.
*
* @return   XDecap_Config of the HW Channel Number.
*
* @note     None.
*
******************************************************************************/
XDecap_Config XDecap_CoreInfo(XDecap *InstancePtr)
{

    XDecap_Config XDecap_Config_RegValue;

    /* Get Core Info */
    XDecap_Config_RegValue.HWChannelNumber =
        (XDecap_ReadReg(InstancePtr->Config.BaseAddress,
            XDECAP_SYS_CONFIG_OFFSET) & XDECAP_SYS_CONFIG_C_CHANNELS_MASK);

    return XDecap_Config_RegValue;

}

/*****************************************************************************/
/**
*
* This function perform software resets on VoIP Decapsulator, which clears
* all the registers.
*
* @param    InstancePtr is a pointer to the XDecap core instance.
*
* @return   None.
*
* @note     None.
*
******************************************************************************/
void XDecap_SoftReset(XDecap *InstancePtr)
{
    u32 RegValue;

    RegValue = XDecap_ReadReg((InstancePtr)->Config.BaseAddress,
        (XDECAP_CONTROL_OFFSET));

    /* Set the Reset */
    XDecap_WriteReg((InstancePtr)->Config.BaseAddress,
        (XDECAP_CONTROL_OFFSET),(RegValue | XDECAP_CONTROL_SOFT_RESET_MASK));

    RegValue = XDecap_ReadReg((InstancePtr)->Config.BaseAddress,
        (XDECAP_CONTROL_OFFSET));

    /* Clear the Reset */
    XDecap_WriteReg((InstancePtr)->Config.BaseAddress,(XDECAP_CONTROL_OFFSET),
        (RegValue & ~(XDECAP_CONTROL_SOFT_RESET_MASK)));
}

/*****************************************************************************/
/**
*
* This function send a pulse on VoIP Decapsulator to update the configured
* channel
*
* @param    InstancePtr is a pointer to the XDecap core instance.
*
* @return   None.
*
* @note     None.
*
******************************************************************************/
void XDecap_ChannelUpdate(XDecap *InstancePtr)
{
    u32 RegValue;

    RegValue = XDecap_ReadReg((InstancePtr)->Config.BaseAddress,
        (XDECAP_CONTROL_OFFSET));

    /* Set the Channel Update */
    XDecap_WriteReg((InstancePtr)->Config.BaseAddress,(XDECAP_CONTROL_OFFSET),
        (RegValue | XDECAP_CONTROL_CHANNEL_UPDATE_MASK));

    RegValue = XDecap_ReadReg((InstancePtr)->Config.BaseAddress,
        (XDECAP_CONTROL_OFFSET));

    /* Clear the Channel Update */
    XDecap_WriteReg((InstancePtr)->Config.BaseAddress,(XDECAP_CONTROL_OFFSET),
        (RegValue & ~(XDECAP_CONTROL_CHANNEL_UPDATE_MASK)));
}

/*****************************************************************************/
/**
*
* This function accesses the current configuration channel
*
* @param    InstancePtr is a pointer to the XDecap core instance.
*
* @param    Channels is current configured channel
*
* @return   None.
*
* @note     None.
*
******************************************************************************/
void XDecap_ChannelAccess(XDecap *InstancePtr, u16 Channels)
{
    XDecap_WriteReg((InstancePtr)->Config.BaseAddress,
        (XDECAP_CHANNEL_ACCESS_OFFSET),((u32)(Channels) &
            (XDECAP_CHANNEL_ACCESS_MASK)));
}

/*****************************************************************************/
/**
*
* This function reads current number of received packet at input interface of
* VoIP Decapsulator
*
* @param    InstancePtr is a pointer to the XDecap core instance.
*
* @return   Number of Received Packet
*
* @note     None.
*
******************************************************************************/
u32 XDecap_RXPacketsCnt(XDecap *InstancePtr)
{
    u32 RegValue;

    /* Set the Channel Update */
    RegValue = XDecap_ReadReg((InstancePtr)->Config.BaseAddress,
        (XDECAP_RX_PKT_CNT_OFFSET));

    InstancePtr->RXPcktCnt = RegValue;

    return(RegValue);
}

/*****************************************************************************/
/**
*
* This function reads the current number of discarded packet due to header
* (From Register) mismatch
*
* @param    InstancePtr is a pointer to the XDecap core instance.
*
* @return   Number of Mismatched Packet
*
* @note     None.
*
******************************************************************************/
u32 XDecap_MisMatchedPacketsCnt(XDecap *InstancePtr)
{
    u32 RegValue;

    /* Set the Channel Update */
    RegValue = XDecap_ReadReg((InstancePtr)->Config.BaseAddress,
        (XDECAP_MISMATCHED_PKT_CNT_OFFSET));

    InstancePtr->MisMatchedPcktCnt = RegValue;

    return(RegValue);
}

/*****************************************************************************/
/**
*
* This function reads the current number of discarded packet due to Error
* Packet, where received packet has  tuser = 0
*
* @param    InstancePtr is a pointer to the XDecap core instance.
*
* @return   Number of Error Packet.
*
* @note     None.
*
******************************************************************************/
u32 XDecap_ErrorPacketsCnt(XDecap *InstancePtr)
{
    u32 RegValue;

    /* Set the Channel Update */
    RegValue = XDecap_ReadReg((InstancePtr)->Config.BaseAddress,
        (XDECAP_ERROR_PKT_CNT_OFFSET));

    InstancePtr->ErrPcktCnt = RegValue;

    return(RegValue);
}

/*****************************************************************************/
/**
*
* This function send a pulse on Generic Decap Channel Update
*
* @param    InstancePtr is a pointer to the XGeneric_Decap core instance.
*
* @return   None.
*
* @note     None.
*
******************************************************************************/
u32 XDecap_MismatchPacketCnt(XDecap *InstancePtr)
{
    u32 RegValue;

    /* Set the Channel Update */
    RegValue = XDecap_ReadReg((InstancePtr)->Config.BaseAddress,
        (XDECAP_MISMATCHED_PKT_CNT_OFFSET));

    return(RegValue);
}

/*****************************************************************************/
/**
*
* This function clears the general register space statistic registers. It's a
* self clear register.
*
* @param    InstancePtr is a pointer to the XDecap core instance.
*
* @return   None.
*
* @note     None.
*
******************************************************************************/
void XDecap_ClearGeneralStatistic(XDecap *InstancePtr)
{
    /* Write the value to the register */
    XDecap_WriteReg(InstancePtr->Config.BaseAddress,
        XDECAP_CLEAR_STATISTIC_OFFSET, (XDECAP_CLEAR_STATISTIC_MASK));
}

/*****************************************************************************/
/**
*
* This function sets the Packet Lock Window, where if consecutive Incoming
* Packet of the Channel has same SDI Video Format (Extracted from ST2022-6
* Packet), VoIP Decapsulator will trigger the Packet Lock Interrupt Process
*
* @param    InstancePtr is a pointer to the XDecap core instance.
*
* @return   None.
*
* @note     None.
*
******************************************************************************/
void XDecap_PacketLockWindow(XDecap *InstancePtr)
{
    u32 RegValue;

    RegValue = (XDecap_ReadReg(InstancePtr->Config.BaseAddress,
        XDECAP_STREAM_DETECT_WINDOW_OFFSET) &
            ~(XDECAP_STREAM_DETECT_WINDOW_VIDEO_LOCK_MASK));

    /* Write the value to the register */
    XDecap_WriteReg(InstancePtr->Config.BaseAddress,
        XDECAP_STREAM_DETECT_WINDOW_OFFSET,
            (RegValue | ((InstancePtr->PacketLockWindow) &
                        (XDECAP_STREAM_DETECT_WINDOW_VIDEO_LOCK_MASK))));
}

/*****************************************************************************/
/**
*
* This function sets the Packet Unlock Window, where if consecutive Incoming
* Packet of the Channel has different SDI Video Format (Extracted from ST2022-6
* Packet) from the Packet Lock SDI Video Format, VoIP Decapsulator will trigger
* the Packet Unlock Interrupt Process
*
* @param    InstancePtr is a pointer to the XDecap core instance.
*
* @return   None.
*
* @note     None.
*
******************************************************************************/
void XDecap_PacketUnLockWindow(XDecap *InstancePtr)
{
    u32 RegValue;

    RegValue = (XDecap_ReadReg(InstancePtr->Config.BaseAddress,
        XDECAP_STREAM_DETECT_WINDOW_OFFSET) &
            ~(XDECAP_STREAM_DETECT_WINDOW_VIDEO_UNLOCK_MASK));

    /* Write the value to the register */
    XDecap_WriteReg(InstancePtr->Config.BaseAddress,
        XDECAP_STREAM_DETECT_WINDOW_OFFSET,\
           (RegValue | (((InstancePtr->PacketUnLockWindow) <<
               XDECAP_STREAM_DETECT_WINDOW_VIDEO_UNLOCK_SHIFT) &
                  (XDECAP_STREAM_DETECT_WINDOW_VIDEO_UNLOCK_MASK))));
}

/*****************************************************************************/
/**
*
* This function reads the Peak Buffer Level of the VoIP Decapsulator. (Debug
* Register)
*
* @param    InstancePtr is a pointer to the XDecap core instance.
*
* @return   Peak Buffer Level.
*
* @note     None.
*
******************************************************************************/
u8 XDecap_PeakBufferLv(XDecap *InstancePtr)
{
    u8 RegValue;

    RegValue = (XDecap_ReadReg(InstancePtr->Config.BaseAddress,
        XDECAP_PEAK_BUFFER_LEVEL_OFFSET) &
            (XDECAP_PEAK_BUFFER_LEVEL_MASK));

    return (RegValue);
}

/*****************************************************************************/
/**
*
* This function Enables/Disable the VoIP Decapsulator based on user Input at
* XDecap Structure. XDecap->ModuleEnable
*
* @param    InstancePtr is a pointer to the XDecap core instance.
*
* @return   None.
*
* @note     None.
*
******************************************************************************/
void XDecap_ModuleEn(XDecap *InstancePtr)
{
    u32 RegValue;

    RegValue = (XDecap_ReadReg(InstancePtr->Config.BaseAddress,
        XDECAP_MODULE_CONTROL_OFFSET) &
            ~(XDECAP_MODULE_CONTROL_MODULE_ENABLE_MASK));

    /* Write the value to the register */
    XDecap_WriteReg(InstancePtr->Config.BaseAddress,
       XDECAP_MODULE_CONTROL_OFFSET,\
          (RegValue | ((InstancePtr->ModuleEnable) &
             (XDECAP_MODULE_CONTROL_MODULE_ENABLE_MASK))));
}

/* Channel Space */
/*****************************************************************************/
/**
*
* This function set the Operation Mode of the VoIP Decapsulator based on user
* Input at XDecap Structure.
* Normal Mode:   VoIP Decapsulator doesn't have any information of incoming
*                packet before operation
* Lossless Mode: VoIP Decapsulator have configured with Incoming Packet
*                Information before operation.
*
* @param    InstancePtr is a pointer to the XDecap core instance.
*
* @param    Channels is current configured channel
*
* @return   None.
*
* @note     None.
*
******************************************************************************/
void XDecap_SetOperationMode(XDecap *InstancePtr, u16 Channels)
{
    u32 RegValue;

    /* Select the Channel */
    XDecap_ChannelAccess(InstancePtr, Channels);

    RegValue = ((XDecap_ReadReg(InstancePtr->Config.BaseAddress,
        XDECAP_CHANNEL_CONTROL_OFFSET) &
            (~(XDECAP_CHANNEL_CONTROL_LOSSLESS_MODE_MASK))));

    /* Write the value to the register */
    XDecap_WriteReg(InstancePtr->Config.BaseAddress,
        XDECAP_CHANNEL_CONTROL_OFFSET,
           (RegValue | (((InstancePtr)->ChannelCfg[Channels].LosslessMode) &
                (XDECAP_CHANNEL_CONTROL_LOSSLESS_MODE_MASK))));

    /* Update the Channels */
    XDecap_ChannelUpdate(InstancePtr);
}

/*****************************************************************************/
/**
*
* This function Clears the Channel Statistic registers.
*
* @param    InstancePtr is a pointer to the XDecap core instance.
*
* @param    Channels is current configured channel
*
* @return   None.
*
* @note     None.
*
******************************************************************************/
void XDecap_ChannelClearStatistic(XDecap *InstancePtr, u16 Channels)
{
    u32 RegValue;

    /* Select the Channel */
    XDecap_ChannelAccess(InstancePtr, Channels);

    RegValue = (XDecap_ReadReg(InstancePtr->Config.BaseAddress,
        XDECAP_STATISTIC_RESET_OFFSET) &
            (~(XDECAP_STATISTIC_RESET_ALL_PACKET_COUNTERS_MASK)));

    /* Write the value to the register */
    XDecap_WriteReg(InstancePtr->Config.BaseAddress,
       XDECAP_STATISTIC_RESET_OFFSET,
          (RegValue | XDECAP_STATISTIC_RESET_ALL_PACKET_COUNTERS_MASK));

    /* Update the Channels */
    XDecap_ChannelUpdate(InstancePtr);
}

/*****************************************************************************/
/**
*
* This function Enable/Disable the Channel
*
* @param    InstancePtr is a pointer to the XDecap core instance.
*
* @param    Channels is current configured channel
*
* @return   None.
*
* @note     None.
*
******************************************************************************/
void XDecap_ControlChannelEn(XDecap *InstancePtr, u16 Channels)
{
    u32 RegValue;

    /* Select the Channel */
    XDecap_ChannelAccess(InstancePtr, Channels);

    RegValue = (XDecap_ReadReg(InstancePtr->Config.BaseAddress,
        XDECAP_CHANNEL_CONTROL_OFFSET) &
            (~(XDECAP_CHANNEL_CONTROL_CHANNEL_ENABLE_MASK)));

    /* Write the value to the register */
    XDecap_WriteReg(InstancePtr->Config.BaseAddress,
       XDECAP_CHANNEL_CONTROL_OFFSET,
          (RegValue |
              (((InstancePtr)->ChannelCfg[Channels].Channel_Enable) &
                 XDECAP_CHANNEL_CONTROL_CHANNEL_ENABLE_MASK)));

    /* Update the Channels */
    XDecap_ChannelUpdate(InstancePtr);
}

/*****************************************************************************/
/**
*
* This function Enable/Disable the Marker Packet Detection
*
* @param    InstancePtr is a pointer to the XDecap core instance.
*
* @param    Channels is current configured channel
*
* @return   None.
*
* @note     None.
*
******************************************************************************/
void XDecap_ControlMPktDetEn(XDecap *InstancePtr, u16 Channels)
{
    u32 RegValue;

    /* Select the Channel */
    XDecap_ChannelAccess(InstancePtr, Channels);

    /* Obtain Current Register Value */
    RegValue = (XDecap_ReadReg(InstancePtr->Config.BaseAddress,
        XDECAP_CHANNEL_CONTROL_OFFSET) &
            (~(XDECAP_CHANNEL_CONTROL_M_PKT_DET_EN_MASK)));

    /* Write the value to the register */
    XDecap_WriteReg(InstancePtr->Config.BaseAddress,
        XDECAP_CHANNEL_CONTROL_OFFSET,
           (RegValue |
              ((((InstancePtr)->ChannelCfg[Channels].MPkt_DetEn) <<
                 XDECAP_CHANNEL_CONTROL_M_PKT_DET_EN_SHIFT) &
                    XDECAP_CHANNEL_CONTROL_M_PKT_DET_EN_MASK)));

    /* Update the Channels */
    XDecap_ChannelUpdate(InstancePtr);
}

/*****************************************************************************/
/**
*
* This function Enable/Disable the Marker Packet Drop
*
* @param    InstancePtr is a pointer to the XDecap core instance.
*
* @param    Channels is current configured channel
*
* @return   None.
*
* @note     None.
*
******************************************************************************/
void XDecap_ControlMPktDropEn(XDecap *InstancePtr, u16 Channels)
{
    u32 RegValue;

    /* Select the Channel */
    XDecap_ChannelAccess(InstancePtr, Channels);

    /* Obtain Current Register Value */
    RegValue = (XDecap_ReadReg(InstancePtr->Config.BaseAddress,
        XDECAP_CHANNEL_CONTROL_OFFSET) &
            (~(XDECAP_CHANNEL_CONTROL_DROP_M_PKT_EN_MASK)));

    /* Write the value to the register */
    XDecap_WriteReg(InstancePtr->Config.BaseAddress,
        XDECAP_CHANNEL_CONTROL_OFFSET,
           (RegValue |
              ((((InstancePtr)->ChannelCfg[Channels].MPkt_DropEn) <<
                 XDECAP_CHANNEL_CONTROL_DROP_M_PKT_EN_SHIFT) &
                    XDECAP_CHANNEL_CONTROL_DROP_M_PKT_EN_MASK)));

    /* Update the Channels */
    XDecap_ChannelUpdate(InstancePtr);
}

/*****************************************************************************/
/**
*
* This function sets the Header Filter Enable Setting
*
* @param    InstancePtr is a pointer to the XDecap core instance.
*
* @param    Channels is current configured channel
*
* @return   None.
*
* @note     None.
*
******************************************************************************/
void XDecap_SetMatchSelect(XDecap *InstancePtr, u16 Channels)
{
    u32 RegValue;
    u32 Match_Select;

    /* Select the Channel */
    XDecap_ChannelAccess(InstancePtr, Channels);

    RegValue = ((XDecap_ReadReg(InstancePtr->Config.BaseAddress,
        XDECAP_MATCH_SELECT_OFFSET) & (~(XDECAP_MATCH_SELECT_FILTER_ALL))));

    Match_Select =
          ((InstancePtr)->ChannelCfg[Channels].MatchSelect.match_ssrc    |
          (InstancePtr)->ChannelCfg[Channels].MatchSelect.match_udp_dst  |
          (InstancePtr)->ChannelCfg[Channels].MatchSelect.match_udp_src  |
          (InstancePtr)->ChannelCfg[Channels].MatchSelect.match_ip_dst   |
          (InstancePtr)->ChannelCfg[Channels].MatchSelect.match_ip_src   |
          (InstancePtr)->ChannelCfg[Channels].MatchSelect.match_vlan_reg);

    /* Write the value to the register */
    XDecap_WriteReg(InstancePtr->Config.BaseAddress,
        XDECAP_MATCH_SELECT_OFFSET,((RegValue) | (Match_Select)));

    /* Update the Channels */
    XDecap_ChannelUpdate(InstancePtr);
}

/*****************************************************************************/
/**
*
* This function Sets the IP Destination Address of Incoming Packet for
* Matching/Filtering if the Filter is enabled
*
* @param    InstancePtr is a pointer to the XDecap core instance.
*
* @param    Channels is current configured channel
*
* @return   None.
*
* @note     None.
*
******************************************************************************/
void XDecap_MatchIPv4Dest(XDecap *InstancePtr, u16 Channels)
{
     /* Select the Channel */
     XDecap_ChannelAccess(InstancePtr, Channels);

     /* Write the value to the register */
     XDecap_WriteReg(InstancePtr->Config.BaseAddress,
        XDECAP_MATCH_DEST_IP0_OFFSET,
           ((InstancePtr)->ChannelCfg[Channels].MatchHeader.ipv4_dest_address));

     /* Update the Channels */
     XDecap_ChannelUpdate(InstancePtr);
}

/*****************************************************************************/
/**
*
* This function Sets the IP Source Address of Incoming Packet for
* Matching/Filtering if the Filter is enabled
*
* @param    InstancePtr is a pointer to the XDecap core instance.
*
* @param    Channels is current configured channel
*
* @return   None.
*
* @note     None.
*
******************************************************************************/
void XDecap_MatchIPv4Src(XDecap *InstancePtr, u16 Channels)
{
    /* Select the Channel */
    XDecap_ChannelAccess(InstancePtr, Channels);

    /* Write the value to the register */
    XDecap_WriteReg(InstancePtr->Config.BaseAddress,
       XDECAP_MATCH_SRC_IP0_OFFSET,
          ((InstancePtr)->ChannelCfg[Channels].MatchHeader.ipv4_src_address));

    /* Update the Channels */
    XDecap_ChannelUpdate(InstancePtr);
}

/*****************************************************************************/
/**
*
* This function Sets the UDP Destination Port of Incoming Packet for
* Matching/Filtering if the Filter is enabled
*
* @param    InstancePtr is a pointer to the XDecap core instance.
*
* @param    Channels is current configured channel
*
* @return   None.
*
* @note     None.
*
******************************************************************************/
void XDecap_MatchUDPDest(XDecap *InstancePtr, u16 Channels)
{
    u32 RegValue;

    /* Select the Channel */
    XDecap_ChannelAccess(InstancePtr, Channels);

    RegValue = (XDecap_ReadReg(InstancePtr->Config.BaseAddress,
        XDECAP_MATCH_UDP_DEST_PORT_OFFSET) &
            ~(XDECAP_MATCH_UDP_DEST_PORT_MASK));

    /* Write the value to the register */
    XDecap_WriteReg(InstancePtr->Config.BaseAddress,
       XDECAP_MATCH_UDP_DEST_PORT_OFFSET,\
          (RegValue |
             ((InstancePtr)->ChannelCfg[Channels].MatchHeader.udp_dest_port)));

    /* Update the Channels */
    XDecap_ChannelUpdate(InstancePtr);
}

/*****************************************************************************/
/**
*
* This function Sets the UDP Source Port of Incoming Packet for
* Matching/Filtering if the Filter is enabled
*
* @param    InstancePtr is a pointer to the XDecap core instance.
*
* @param    Channels is current configured channel
*
* @return   None.
*
* @note     None.
*
******************************************************************************/
void XDecap_MatchUDPSrc(XDecap *InstancePtr, u16 Channels)
{
    u32 RegValue;

    /* Select the Channel */
    XDecap_ChannelAccess(InstancePtr, Channels);

    RegValue = (XDecap_ReadReg(InstancePtr->Config.BaseAddress,
        XDECAP_MATCH_UDP_SRC_PORT_OFFSET) &
            ~(XDECAP_MATCH_UDP_SRC_PORT_MASK));

    /* Write the value to the register */
    XDecap_WriteReg(InstancePtr->Config.BaseAddress,
       XDECAP_MATCH_UDP_SRC_PORT_OFFSET,\
          (RegValue |
              ((InstancePtr)->ChannelCfg[Channels].MatchHeader.udp_src_port)));

    /* Update the Channels */
    XDecap_ChannelUpdate(InstancePtr);

}

/*****************************************************************************/
/**
*
* This function Sets the VLAN Packet Filtering. This function is valid if the
* VLAN Match Select is Enabled.
*
* @param    InstancePtr is a pointer to the XDecap core instance.
*
* @param    Channels is current configured channel
*
* @return   None.
*
* @note     None.
*
******************************************************************************/
void XDecap_ToMatchVLANID(XDecap *InstancePtr, u16 Channels)
{
    u32 RegValue;

    /* This register/driver is valid when base address
    ** + offset(0xC0), Bit 0 = 1 */

    RegValue = (XDecap_ReadReg(InstancePtr->Config.BaseAddress,
        XDECAP_MATCH_VLAN_ID_OFFSET) &
            ~(XDECAP_MATCH_VLAN_ID_VLAN_FILTER_ENABLE_MASK));

    /* Write the value to the register */
    XDecap_WriteReg(InstancePtr->Config.BaseAddress,
        XDECAP_MATCH_VLAN_ID_OFFSET,
           (RegValue |
              ((((InstancePtr)->ChannelCfg[Channels].is_MatchVlanPckt) <<
                 XDECAP_MATCH_VLAN_ID_VLAN_FILTER_ENABLE_SHIFT) &
                    XDECAP_MATCH_VLAN_ID_VLAN_FILTER_ENABLE_MASK)));
}

/*****************************************************************************/
/**
*
* This function Sets the VLAN ID (12 Bits) of Incoming Packet for
* Matching/Filtering if the Filter is enabled
*
* @param    InstancePtr is a pointer to the XDecap core instance.
*
* @param    Channels is current configured channel
*
* @return   None.
*
* @note     None.
*
******************************************************************************/
void XDecap_MatchVLANID(XDecap *InstancePtr, u16 Channels)
{
    u32 RegValue;

    /* Select the Channel */
    XDecap_ChannelAccess(InstancePtr, Channels);

    RegValue = (XDecap_ReadReg(InstancePtr->Config.BaseAddress,
        XDECAP_MATCH_VLAN_ID_OFFSET) &
            ~(XDECAP_MATCH_VLAN_ID_VLAN_TAG_MASK));

    /* Write the value to the register */
    XDecap_WriteReg(InstancePtr->Config.BaseAddress,
      XDECAP_MATCH_VLAN_ID_OFFSET,
        (RegValue |
           (((InstancePtr)->ChannelCfg[Channels].MatchHeader.vlan_pcp_cfi_vid)
             & XDECAP_MATCH_VLAN_ID_VLAN_TAG_MASK)));

    /* Update the Channels */
    XDecap_ChannelUpdate(InstancePtr);
}

/*****************************************************************************/
/**
*
* This function Sets the RTP-SSRC Field of Incoming Packet for
* Matching/Filtering if the Filter is enabled
*
* @param    InstancePtr is a pointer to the XDecap core instance.
*
* @param    Channels is current configured channel
*
* @return   None.
*
* @note     None.
*
******************************************************************************/
void XDecap_MatchSSRC(XDecap *InstancePtr, u16 Channels)
{
    /* Select the Channel */
    XDecap_ChannelAccess(InstancePtr, Channels);

    /* Write the value to the register */
    XDecap_WriteReg(InstancePtr->Config.BaseAddress, XDECAP_MATCH_SSRC_OFFSET,\
          (((InstancePtr)->ChannelCfg[Channels].MatchHeader.ssrc)));

    /* Update the Channels */
    XDecap_ChannelUpdate(InstancePtr);

}

/*****************************************************************************/
/**
*
* This function <>
*
* @param    InstancePtr is a pointer to the XDecap core instance.
*
* @param    Channels is current configured channel
*
* @return   None.
*
* @note     None.
*
******************************************************************************/
u8 XDecap_SDIPacketLockStatus (XDecap *InstancePtr, u16 Channels)
{
   u8 RegValue;

   /* Select the Channel */
   XDecap_ChannelAccess(InstancePtr, Channels);

   /* Read the Video Format Register & Mask the SDI Packet Lock Status */
   RegValue = ((XDecap_ReadReg(InstancePtr->Config.BaseAddress,
         (XDECAP_VIDEO_FORMAT_OFFSET)) &
             (XDECAP_VIDEO_FORMAT_SDI_PACKET_LOCK_STATUS_MASK)) >>
                 (XDECAP_VIDEO_FORMAT_SDI_PACKET_LOCK_STATUS_SHIFT));

   return (RegValue);
}

/*****************************************************************************/
/**
*
* This function reads the ST2022-6 Packet Information
*
* @param    InstancePtr is a pointer to the XDecap core instance.
*
* @param    Channels is current configured channel
*
* @return   None.
*
* @note     None.
*
******************************************************************************/
XDecap_StreamStatus XDecap_StreamStat(XDecap *InstancePtr, u16 Channels)
{
   u32 RegValue;

   XDecap_StreamStatus XDecap_StreamStatus_RegValue;

   /* Select the Channel */
   XDecap_ChannelAccess(InstancePtr, Channels);

   /* Read the Video Format Register */
   RegValue = XDecap_ReadReg(InstancePtr->Config.BaseAddress,
               (XDECAP_VIDEO_FORMAT_OFFSET));

   XDecap_StreamStatus_RegValue.rx_bitrate      = ((RegValue) &
        (XDECAP_VIDEO_FORMAT_BITRATE_DIV_1_001_MASK)) >>
           (XDECAP_VIDEO_FORMAT_BITRATE_DIV_1_001_SHIFT);
   XDecap_StreamStatus_RegValue.level_b_3g      = ((RegValue) &
        (XDECAP_VIDEO_FORMAT_3G_LEVEL_B_MASK)) >>
           (XDECAP_VIDEO_FORMAT_3G_LEVEL_B_SHIFT);
   XDecap_StreamStatus_RegValue.sdi_mode        = ((RegValue) &
           (XDECAP_VIDEO_FORMAT_VIDEO_MODE_MASK));

   /* Read the Map,Frame,Frate,Sample Register */
   RegValue = XDecap_ReadReg(InstancePtr->Config.BaseAddress,
               (XDECAP_MAP_FRAME_FRATE_SAMPLE_FMT_OFFSET));

   XDecap_StreamStatus_RegValue.MAP            = ((RegValue) &
        (XDECAP_MAP_FRAME_FRATE_SAMPLE_FMT_MAP_MASK)) >>
            (XDECAP_MAP_FRAME_FRATE_SAMPLE_FMT_MAP_SHIFT);
   XDecap_StreamStatus_RegValue.FRAME          = ((RegValue) &
        (XDECAP_MAP_FRAME_FRATE_SAMPLE_FMT_FRAME_MASK)) >>
            (XDECAP_MAP_FRAME_FRATE_SAMPLE_FMT_FRAME_SHIFT);
   XDecap_StreamStatus_RegValue.FRATE          = ((RegValue) &
        (XDECAP_MAP_FRAME_FRATE_SAMPLE_FMT_FRATE_MASK)) >>
            (XDECAP_MAP_FRAME_FRATE_SAMPLE_FMT_FRATE_SHIFT);
   XDecap_StreamStatus_RegValue.SAMPLE         = ((RegValue) &
        (XDECAP_MAP_FRAME_FRATE_SAMPLE_FMT_SAMPLE_MASK)) >>
            (XDECAP_MAP_FRAME_FRATE_SAMPLE_FMT_SAMPLE_SHIFT);

   return (XDecap_StreamStatus_RegValue);
}

/*****************************************************************************/
/**
*
* This function reads current number of valid RTP packet (ST2022-6) per
* channel of VoIP Decapsulator
*
* @param    InstancePtr is a pointer to the XDecap core instance.
*
* @param    Channels is current configured channel
*
* @return   None.
*
* @note     None.
*
******************************************************************************/
u32 XDecap_MediaValidPcktCnt(XDecap *InstancePtr, u16 Channels)
{
    u32 RegValue;

    /* Select the Channel */
    XDecap_ChannelAccess(InstancePtr, Channels);

    /* Read the Current Status */
    RegValue = XDecap_ReadReg(InstancePtr->Config.BaseAddress,
                   (XDECAP_MEDIA_VALID_PACKET_COUNT_OFFSET));

    InstancePtr->MediaValid_pkt_cnt[Channels] = RegValue;

    return (RegValue);
}

/*****************************************************************************/
/**
*
* This function reads current number of valid FEC packet (ST2022-5) per
* channel of VoIP Decapsulator
*
* @param    InstancePtr is a pointer to the XDecap core instance.
*
* @param    Channels is current configured channel
*
* @return   None.
*
* @note     None.
*
******************************************************************************/
u32 XDecap_FECValidPcktCnt(XDecap *InstancePtr, u16 Channels)
{
    u32 RegValue;

    /* Select the Channel */
    XDecap_ChannelAccess(InstancePtr, Channels);

    /* Read the Current Status */
    RegValue = XDecap_ReadReg(InstancePtr->Config.BaseAddress,
                   (XDECAP_FEC_VALID_PACKET_COUNT_OFFSET));

    InstancePtr->FECValid_pkt_cnt[Channels] = RegValue;

    return (RegValue);
}

/*****************************************************************************/
/**
*
* This function reads current number of Re-Ordered packet per channel of
* VoIP Decapsulator
*
* @param    InstancePtr is a pointer to the XDecap core instance.
*
* @param    Channels is current configured channel
*
* @return   None.
*
* @note     None.
*
******************************************************************************/
u32 XDecap_ReOrderedPcktCnt(XDecap *InstancePtr, u16 Channels)
{
    u32 RegValue;

    /* Select the Channel */
    XDecap_ChannelAccess(InstancePtr, Channels);

    /* Read the Current Status */
    RegValue = XDecap_ReadReg(InstancePtr->Config.BaseAddress,
                   (XDECAP_FEC_REORDERED_PACKET_COUNT_OFFSET));

    InstancePtr->ReOrdered_pkt_cnt[Channels] = RegValue;

    return (RegValue);
}

/*****************************************************************************/
/**
*
* This function reads current number of Drop packet per channel of
* VoIP Decapsulator due to Buffer Full (Debug Register)
*
* @param    InstancePtr is a pointer to the XDecap core instance.
*
* @param    Channels is current configured channel
*
* @return   None.
*
* @note     None.
*
******************************************************************************/
u32 XDecap_DropPcktCnt(XDecap *InstancePtr, u16 Channels)
{
    u32 RegValue;

    /* Select the Channel */
    XDecap_ChannelAccess(InstancePtr, Channels);

    /* Read the Current Status */
    RegValue = XDecap_ReadReg(InstancePtr->Config.BaseAddress,
                   (XDECAP_DROP_PACKET_COUNT_OFFSET));

    InstancePtr->Drop_pkt_cnt[Channels] = RegValue;

    return (RegValue);
}


/*****************************************************************************/
/**
*
* This function reads the Packet/Video Lock Interrupt Status
*
* @param    InstancePtr is a pointer to the XDecap core instance.
*
* @param    Channels is current configured channel
*
* @return   None.
*
* @note     None.
*
******************************************************************************/
u8 XDecap_VideoLockIntrStatus(XDecap *InstancePtr, u16 Channels)
{
   u32 RegValue;

   /* Select the Channel */
   XDecap_ChannelAccess(InstancePtr, Channels);

   /* Read the Current Status */
   RegValue = ((XDecap_ReadReg(InstancePtr->Config.BaseAddress,
        (XDECAP_INTERRUPT_STATUS_OFFSET)) &
            (XDECAP_INTERRUPT_STATUS_PACKET_LOCKED_MASK)));

   return (RegValue);
}

/*****************************************************************************/
/**
*
* This function reads the Packet/Video Unlock Interrupt Status
*
* @param    InstancePtr is a pointer to the XDecap core instance.
*
* @param    Channels is current configured channel
*
* @return   None.
*
* @note     None.
*
******************************************************************************/
u8 XDecap_VideoUnLockIntrStatus(XDecap *InstancePtr, u16 Channels)
{
   u32 RegValue;

   /* Select the Channel */
   XDecap_ChannelAccess(InstancePtr, Channels);

   /* Read the Current Status */
   RegValue = ((XDecap_ReadReg(InstancePtr->Config.BaseAddress,
      (XDECAP_INTERRUPT_STATUS_OFFSET)) &
         (XDECAP_INTERRUPT_STATUS_PACKET_UNLOCKED_MASK)) >>\
             (XDECAP_INTERRUPT_STATUS_PACKET_UNLOCKED_SHIFT));

   return (RegValue);
}

/*****************************************************************************/
/**
*
* This function reads the Packet Timeout Interrupt Status
*
* @param    InstancePtr is a pointer to the XDecap core instance.
*
* @param    Channels is current configured channel
*
* @return   None.
*
* @note     None.
*
******************************************************************************/
u8 XDecap_StreamStopIntrStatus(XDecap *InstancePtr, u16 Channels)
{
   u32 RegValue;

   /* Select the Channel */
   XDecap_ChannelAccess(InstancePtr, Channels);

   /* Read the Current Status */
   RegValue = ((XDecap_ReadReg(InstancePtr->Config.BaseAddress,
      (XDECAP_INTERRUPT_STATUS_OFFSET)) &
          (XDECAP_INTERRUPT_STATUS_STREAM_STOP_INTERRUPT_MASK)) >>
              (XDECAP_INTERRUPT_STATUS_STREAM_STOP_INTERRUPT_SHIFT));

   return (RegValue);
}

/*****************************************************************************/
/**
*
* This function Enables/Un-Mask the Packet/Video Lock Interrupt
*
* @param    InstancePtr is a pointer to the XDecap core instance.
*
* @param    Channels is current configured channel
*
* @return   None.
*
* @note     None.
*
******************************************************************************/
void XDecap_EnableVideoLockIntr(XDecap *InstancePtr, u16 Channels)
{
    u32 RegValue;

    /* Select the Channel */
    XDecap_ChannelAccess(InstancePtr, Channels);

    /* Read the Current Status */
    RegValue = (XDecap_ReadReg(InstancePtr->Config.BaseAddress,
        XDECAP_INTERRUPT_MASK_OFFSET) &
            (~(XDECAP_INTERRUPT_STATUS_PACKET_LOCKED_MASK)));

    /* Clear the register (Mask Out) to Disable interrupt */
    XDecap_WriteReg(InstancePtr->Config.BaseAddress,
        XDECAP_INTERRUPT_MASK_OFFSET,(RegValue));

    /* Update the Channels */
    XDecap_ChannelUpdate(InstancePtr);
}

/*****************************************************************************/
/**
*
* This function Enables/Un-Mask the Packet/Video Un-Lock Interrupt
*
* @param    InstancePtr is a pointer to the XDecap core instance.
*
* @param    Channels is current configured channel
*
* @return   None.
*
* @note     None.
*
******************************************************************************/
void XDecap_EnableVideoUnLockIntr(XDecap *InstancePtr, u16 Channels)
{
    u32 RegValue;

    /* Select the Channel */
    XDecap_ChannelAccess(InstancePtr, Channels);

    /* Read the Current Status */
    RegValue = (XDecap_ReadReg(InstancePtr->Config.BaseAddress,
        XDECAP_INTERRUPT_MASK_OFFSET) &
            (~(XDECAP_INTERRUPT_STATUS_PACKET_UNLOCKED_MASK)));

    /* Clear the register (Mask Out) to Disable interrupt */
    XDecap_WriteReg(InstancePtr->Config.BaseAddress,
        XDECAP_INTERRUPT_MASK_OFFSET,(RegValue));

    /* Update the Channels */
    XDecap_ChannelUpdate(InstancePtr);
}

/*****************************************************************************/
/**
*
* This function Enables/Un-Mask the Packet Timeout Interrupt
*
* @param    InstancePtr is a pointer to the XDecap core instance.
*
* @param    Channels is current configured channel
*
* @return   None.
*
* @note     None.
*
******************************************************************************/
void XDecap_EnablePacketStopIntr(XDecap *InstancePtr, u16 Channels)
{
    u32 RegValue;

    /* Select the Channel */
    XDecap_ChannelAccess(InstancePtr, Channels);

    /* Read the Current Status */
    RegValue = (XDecap_ReadReg(InstancePtr->Config.BaseAddress,
       XDECAP_INTERRUPT_MASK_OFFSET) &
          (~(XDECAP_INTERRUPT_STATUS_STREAM_STOP_INTERRUPT_MASK)));

    /* Clear the register (Mask Out) to Disable interrupt */
    XDecap_WriteReg(InstancePtr->Config.BaseAddress,
        XDECAP_INTERRUPT_MASK_OFFSET,(RegValue));

    /* Update the Channels */
    XDecap_ChannelUpdate(InstancePtr);
}

/*****************************************************************************/
/**
*
* This function Disable/Mask the Packet Lock Interrupt
*
* @param    InstancePtr is a pointer to the XDecap core instance.
*
* @param    Channels is current configured channel
*
* @return   None.
*
* @note     None.
*
******************************************************************************/
void XDecap_DisableVideoLockIntr(XDecap *InstancePtr, u16 Channels)
{
    u32 RegValue;

    /* Select the Channel */
    XDecap_ChannelAccess(InstancePtr, Channels);

    /* Read the Current Status */
    RegValue = (XDecap_ReadReg(InstancePtr->Config.BaseAddress,
        XDECAP_INTERRUPT_MASK_OFFSET) &
            (~(XDECAP_INTERRUPT_STATUS_PACKET_LOCKED_MASK)));

    /* Set the register (Mask Out) to Disable interrupt */
    XDecap_WriteReg(InstancePtr->Config.BaseAddress,
        XDECAP_INTERRUPT_MASK_OFFSET,(RegValue |
            (XDECAP_INTERRUPT_STATUS_PACKET_LOCKED_MASK)));

    /* Update the Channels */
    XDecap_ChannelUpdate(InstancePtr);
}

/*****************************************************************************/
/**
*
* This function Disable/Mask the Packet Un-Lock Interrupt
*
* @param    InstancePtr is a pointer to the XDecap core instance.
*
* @param    Channels is current configured channel
*
* @return   None.
*
* @note     None.
*
******************************************************************************/
void XDecap_DisableVideoUnLockIntr(XDecap *InstancePtr, u16 Channels)
{
    u32 RegValue;

    /* Select the Channel */
    XDecap_ChannelAccess(InstancePtr, Channels);

    /* Read the Current Status */
    RegValue = (XDecap_ReadReg(InstancePtr->Config.BaseAddress,
       XDECAP_INTERRUPT_MASK_OFFSET) &
          (~(XDECAP_INTERRUPT_STATUS_PACKET_UNLOCKED_MASK)));

    /* Set the register (Mask Out) to Disable interrupt */
    XDecap_WriteReg(InstancePtr->Config.BaseAddress,
       XDECAP_INTERRUPT_MASK_OFFSET,
          (RegValue | (XDECAP_INTERRUPT_STATUS_PACKET_UNLOCKED_MASK)));

    /* Update the Channels */
    XDecap_ChannelUpdate(InstancePtr);
}

/*****************************************************************************/
/**
*
* This function Disable/Mask the Packet Timeout Interrupt
*
* @param    InstancePtr is a pointer to the XDecap core instance.
*
* @param    Channels is current configured channel
*
* @return   None.
*
* @note     None.
*
******************************************************************************/
void XDecap_DisablePacketStopIntr(XDecap *InstancePtr, u16 Channels)
{
   u32 RegValue;

   /* Select the Channel */
   XDecap_ChannelAccess(InstancePtr, Channels);

   /* Read the Current Status */
   RegValue = (XDecap_ReadReg(InstancePtr->Config.BaseAddress,
      XDECAP_INTERRUPT_MASK_OFFSET) &
         (~(XDECAP_INTERRUPT_STATUS_STREAM_STOP_INTERRUPT_MASK)));

   /* Set the register (Mask Out) to Disable interrupt */
   XDecap_WriteReg(InstancePtr->Config.BaseAddress,
     XDECAP_INTERRUPT_MASK_OFFSET,
       (RegValue | (XDECAP_INTERRUPT_STATUS_STREAM_STOP_INTERRUPT_MASK)));

   /* Update the Channels */
   XDecap_ChannelUpdate(InstancePtr);
}

/*****************************************************************************/
/**
*
* This function Clears All Active Interrupt Status Bit
*
* @param    InstancePtr is a pointer to the XDecap core instance.
*
* @param    Channels is current configured channel
*
* @return   None.
*
* @note     None.
*
******************************************************************************/
void XDecap_IntrClear(XDecap *InstancePtr, u16 Channels)
{

    /* Select the Channel */
    XDecap_ChannelAccess(InstancePtr, Channels);

    /* Write the value to the register (Self Clear Register) */
    XDecap_WriteReg(InstancePtr->Config.BaseAddress,
       XDECAP_INTERRUPT_CLEAR_OFFSET,
          (XDECAP_INTERRUPT_STATUS_PACKET_LOCKED_MASK |
              XDECAP_INTERRUPT_STATUS_PACKET_UNLOCKED_MASK |
                 XDECAP_INTERRUPT_STATUS_STREAM_STOP_INTERRUPT_MASK));

    /* Update the Channels */
    XDecap_ChannelUpdate(InstancePtr);
}

/*****************************************************************************/
/**
*
* This function Sets the Timeout Value to trigger Packet Timeout Interrupt,
* if there are no packet received within the timeout period.
*
* @param    InstancePtr is a pointer to the XDecap core instance.
*
* @param    Channels is current configured channel
*
* @return   None.
*
* @note     None.
*
******************************************************************************/
void XDecap_PacketStopTimer(XDecap *InstancePtr, u16 Channels)
{
    /* Select the Channel */
    XDecap_ChannelAccess(InstancePtr, Channels);

    /* Write the value to the register */
    XDecap_WriteReg(InstancePtr->Config.BaseAddress,
        XDECAP_VIDEO_STOP_TIMER_OFFSET,
            ((InstancePtr)->ChannelCfg[Channels].PacketStopTimer));

    /* Update the Channels */
    XDecap_ChannelUpdate(InstancePtr);
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
