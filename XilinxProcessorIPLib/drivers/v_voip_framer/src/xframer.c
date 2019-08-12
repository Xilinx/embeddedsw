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
* THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
* THE SOFTWARE.
*
*
*
******************************************************************************/
/*****************************************************************************/
/**
*
* @file xframer.c
*
* This is the main file for Xilinx VoIP Framer core. Please see
* xframer.h for more details of the driver.
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


#include "xframer.h"
#include "string.h"

/************************** Constant Definitions *****************************/

/***************** Macros (Inline Functions) Definitions *********************/

/**************************** Type Definitions *******************************/

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/

/************************** Function Definitions *****************************/

/*****************************************************************************/
/**
*
* This function initializes the VoIP Framer core. This function must
* be called prior to using the VoIP Framer core. Initialization of the
* VoIP Framer includes setting up the instance data and ensuring the
* hardware is in a quiescent state.
*
* @param    InstancePtr is a pointer to the XDecap core instance.
* @param    CfgPtr points to the configuration structure associated with
*           the VoIP Framer core.
* @param    EffectiveAddr is the base address of the device. If address
*           translation is being used, then this parameter must reflect the
*           virtual base address. Otherwise, the physical address should be
*           used.
*
* @return
*       - XST_SUCCESS if XFramer_CfgInitialize was successful.
*       - XST_FAILURE if Channel Number has mismatched with value reflected
*         on the register, and if the Overflow strategy has mismatched with the
*         value reflected on the register.
*
* @note     None.
*
******************************************************************************/
int XFramer_CfgInitialize(XFramer *InstancePtr,
                                  XFramer_Config *CfgPtr, UINTPTR EffectiveAddr)
{
       u16 Index;
    XFramer_Config XFramer_Config_RegValue;

    /* Verify arguments. */
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(CfgPtr != NULL);
    Xil_AssertNonvoid(EffectiveAddr != (u32)0x0);

    /* Setup the instance */
    (void)memset((void *)InstancePtr, 0, sizeof(XFramer));
    (void)memcpy((void *)&(InstancePtr->Config), (const void *)CfgPtr,
             sizeof(XFramer_Config));
    InstancePtr->Config.BaseAddress = EffectiveAddr;

    /* Set the data structure based on hardware settings */
    InstancePtr->Config.HWChannelNumber          = CfgPtr->HWChannelNumber;
    InstancePtr->Config.OverflowHandlingStrategy =
                                          CfgPtr->OverflowHandlingStrategy;

    /* Obtained the Hardware Parameter Settings from the Register Space */
    XFramer_Config_RegValue = XFramer_CoreInfo(InstancePtr);

    /* Verify the Hardware Setting = Register Value */
    if ((InstancePtr->Config.HWChannelNumber !=
             XFramer_Config_RegValue.HWChannelNumber) ||
                 (InstancePtr->Config.OverflowHandlingStrategy !=
                      XFramer_Config_RegValue.OverflowHandlingStrategy)) {
        return (XST_FAILURE);
    }

    /* Clear the registers */
    XFramer_SoftReset(InstancePtr);

    /* Check The Busy Bit - Wait the Busy Bit = 0 */
    Index = 0x00;
    while (XFramer_BusyBit(InstancePtr)) {
        if (Index == 65535) {
            xdbg_printf(XDBG_DEBUG_GENERAL,"Error: VoIP Framer Busy Bit Longer \
            than Expected\n\r");
            break;
        }
        Index ++;
    }

    /* Reset the hardware and set the flag to indicate the driver is ready */
    InstancePtr->IsReady = (u32)(XIL_COMPONENT_IS_READY);

    return (XST_SUCCESS);

}

/*****************************************************************************/
/**
*
* This function configures the entire VoIP Framer based on user configured
* value
*
* @param    InstancePtr is a pointer to the XFramer core instance.
*
* @return   None.
*
* @note     None.
*
******************************************************************************/
void XFramer_CoreConfig(XFramer *InstancePtr)
{
    XFramer_EthSrcAddr0(InstancePtr);
    XFramer_EthSrcAddr1(InstancePtr);
    XFramer_CoreChannelConfig(InstancePtr);
}

/*****************************************************************************/
/**
*
* This function configures the entire Channel Space of VoIP Framer
* based on user configured value
*
* @param    InstancePtr is a pointer to the XFramer core instance.
*
* @return   None.
*
* @note     None.
*
******************************************************************************/
void XFramer_CoreChannelConfig(XFramer *InstancePtr)
{
    u16 Index;

    /* Setting Parameters based on Drivers */
    for (Index=(u16)0x00; Index < XFRAMER_MAX_CHANNEL; Index++) {
        /* Select Channels */
        XFramer_ChannelAccess(InstancePtr,Index);

        /* Configure Channel Space Register */
        XFramer_ChannelConfig(InstancePtr, Index);

        /* Update the Channels */
        XFramer_ChannelUpdate(InstancePtr);
    }

}

/*****************************************************************************/
/**
*
* This function Configures the VoIP Framer Channels Parameter
*
* @param    InstancePtr is a pointer to the XFramer core instance.
*
* @param    Channels is current configured channel
*
* @return   None.
*
* @note     None.
*
******************************************************************************/
void XFramer_ChannelConfig(XFramer *InstancePtr, u16 Channels) {

    /* Select Channels */
    XFramer_ChannelAccess(InstancePtr,Channels);

    /* Set Match Parameters */
    XFramer_EthDestAddr0(InstancePtr,Channels);
    XFramer_EthDestAddr1(InstancePtr,Channels);
    XFramer_MediaTOS(InstancePtr,Channels);
    XFramer_MediaTTL(InstancePtr,Channels);
    XFramer_FECTOS(InstancePtr,Channels);
    XFramer_FECTTL(InstancePtr,Channels);
    XFramer_IP0Dest(InstancePtr,Channels);
    XFramer_IP0Src(InstancePtr,Channels);
    XFramer_UDPDest(InstancePtr,Channels);
    XFramer_UDPSrc(InstancePtr,Channels);
    XFramer_VLANID(InstancePtr,Channels);

    /* Set other Framer Modes */
    XFramer_SetVLAN(InstancePtr,Channels);

    /* Set Channel Enable */
    XFramer_TransmitEnable(InstancePtr,Channels);

}

/*****************************************************************************/
/**
*
* This function Reads the VoIP Framer Channels Parameter Register
*
* @param    InstancePtr is a pointer to the XFramer core instance.
*
* @param    Channels is current configured channel
*
* @return   None.
*
* @note     None.
*
******************************************************************************/
XFramer_ChannelCfg XFramer_ChStatus(XFramer *InstancePtr, u16 Channels)
{
    XFramer_ChannelCfg XFramer_ChannelCfg_RegValue;

    /* Select Channels */
    XFramer_ChannelAccess(InstancePtr,Channels);

    /* Register XFRAMER_CHANNEL_CFG */
    XFramer_ChannelCfg_RegValue.Transmit_Enable =
           (XFramer_ReadReg(InstancePtr->Config.BaseAddress,
            XFRAMER_CHANNEL_CTRL) &
                   (XFRAMER_CHANNEL_CTRL_TRANSMIT_ENABLE_MASK));

    /* Register XFRAMER_ETH_DEST_ADDR0 */
    XFramer_ChannelCfg_RegValue.PcktHeader.Dest_MACAddr_Low =
           XFramer_ReadReg(InstancePtr->Config.BaseAddress,
                  XFRAMER_ETH_SRC_ADDR_LOW);

    /* Register XFRAMER_ETH_DEST_ADDR1 */
    XFramer_ChannelCfg_RegValue.PcktHeader.Dest_MACAddr_High =
           (XFramer_ReadReg(InstancePtr->Config.BaseAddress,
            XFRAMER_ETH_SRC_ADDR_HIGH) & (XFRAMER_ETH_SRC_ADDR_HIGH_MASK));

    /* Register XFRAMER_VLAN_TAG_INFO */
    XFramer_ChannelCfg_RegValue.is_vlan =
           (XFramer_ReadReg(InstancePtr->Config.BaseAddress,
            XFRAMER_VLAN_TAG_INFO) &
                   (XFRAMER_VLAN_TAG_INFO_WITH_VLAN_MASK)) >>
                           XFRAMER_VLAN_TAG_INFO_WITH_VLAN_SHIFT;

    XFramer_ChannelCfg_RegValue.PcktHeader.vlan_pcp_cfi_vid =
           (XFramer_ReadReg(InstancePtr->Config.BaseAddress,
                   XFRAMER_VLAN_TAG_INFO) &
                          (XFRAMER_VLAN_TAG_INFO_VLAN_ID_MASK));

    /* Register XFRAMER_MEDIA_IP_VER_TOS_TTL & XFRAMER_FEC_IP_VER_TOS_TTL */
    XFramer_ChannelCfg_RegValue.PcktHeader.media_tos =
           (XFramer_ReadReg(InstancePtr->Config.BaseAddress,
                   XFRAMER_MEDIA_IP_VER_TOS_TTL) &
                          (XFRAMER_IP_VER_TOS_TTL_TOS_MASK)) >>
                                  XFRAMER_IP_VER_TOS_TTL_TOS_SHIFT;

    XFramer_ChannelCfg_RegValue.PcktHeader.media_ttl =
           (XFramer_ReadReg(InstancePtr->Config.BaseAddress,
                   XFRAMER_MEDIA_IP_VER_TOS_TTL) &
                          (XFRAMER_IP_VER_TOS_TTL_TTL_MASK));

    XFramer_ChannelCfg_RegValue.PcktHeader.fec_tos =
           (XFramer_ReadReg(InstancePtr->Config.BaseAddress,
                   XFRAMER_FEC_IP_VER_TOS_TTL) &
                          (XFRAMER_IP_VER_TOS_TTL_TOS_MASK)) >>
                                  XFRAMER_IP_VER_TOS_TTL_TOS_SHIFT;

    XFramer_ChannelCfg_RegValue.PcktHeader.fec_ttl =
           (XFramer_ReadReg(InstancePtr->Config.BaseAddress,
            XFRAMER_FEC_IP_VER_TOS_TTL) &
                   (XFRAMER_IP_VER_TOS_TTL_TTL_MASK));

    /* Register XFRAMER_DEST_IP0 */
    XFramer_ChannelCfg_RegValue.PcktHeader.ip_dest_address =
           XFramer_ReadReg(InstancePtr->Config.BaseAddress,
                  XFRAMER_DEST_IP0);

    /* Register XFRAMER_SRC_IP0 */
    XFramer_ChannelCfg_RegValue.PcktHeader.ip_src_address =
           XFramer_ReadReg(InstancePtr->Config.BaseAddress,
                  XFRAMER_SRC_IP0);

    /* Register XGENERIC_FRAMER_UDP_PORT */
    XFramer_ChannelCfg_RegValue.PcktHeader.udp_dest_port =
           (XFramer_ReadReg(InstancePtr->Config.BaseAddress,
                   XFRAMER_DEST_UDP_PORT) & (XFRAMER_DEST_UDP_PORT_MASK));

    XFramer_ChannelCfg_RegValue.PcktHeader.udp_src_port =
           (XFramer_ReadReg(InstancePtr->Config.BaseAddress,
            XFRAMER_SOURCE_UDP_PORT) & (XFRAMER_SOURCE_UDP_PORT_MASK));

    return (XFramer_ChannelCfg_RegValue);
}

/*****************************************************************************/
/**
*
* This function reads the Core Information (Configured by user through GUI),
* in the VoIP Framer Register
*
* @param    InstancePtr is a pointer to the XFramer core instance.
*
* @return   XFramer_Config_RegValue of the HW Channel Number and the Overflow
*        Handling Strategy
*
* @note     None.
*
******************************************************************************/
XFramer_Config XFramer_CoreInfo(XFramer *InstancePtr)
{

    XFramer_Config XFramer_Config_RegValue;

    /* Get Core Info */
    XFramer_Config_RegValue.HWChannelNumber =
           (XFramer_ReadReg(InstancePtr->Config.BaseAddress,
                 XFRAMER_SYS_CONFIG) & XFRAMER_SYS_CONFIG_C_CHANNELS_MASK);

    XFramer_Config_RegValue.OverflowHandlingStrategy =
           ((XFramer_ReadReg(InstancePtr->Config.BaseAddress,
                  XFRAMER_SYS_CONFIG) & XFRAMER_SYS_CONFIG_C_HITLESS_MASK) >>
                       XFRAMER_SYS_CONFIG_C_HITLESS_SHIFT);

    return XFramer_Config_RegValue;
}

/*****************************************************************************/
/**
*
* This function perform software resets on VoIP Framer, which clears
* all the registers.
*
* @param    InstancePtr is a pointer to the XFramer core instance.
*
* @return   None.
*
* @note     None.
*
******************************************************************************/
void XFramer_SoftReset(XFramer *InstancePtr)
{
    u32 RegValue;

    RegValue = XFramer_ReadReg((InstancePtr)->Config.BaseAddress,
                   (XFRAMER_CONTROL));

    /* Set the Reset */
    XFramer_WriteReg((InstancePtr)->Config.BaseAddress,
           (XFRAMER_CONTROL),(RegValue |
                   XFRAMER_CONTROL_SOFT_RESET_MASK));

    RegValue = XFramer_ReadReg((InstancePtr)->Config.BaseAddress,
                   (XFRAMER_CONTROL));

    /* Clear the Reset */
    XFramer_WriteReg((InstancePtr)->Config.BaseAddress,
          (XFRAMER_CONTROL),(RegValue &
                  ~(XFRAMER_CONTROL_SOFT_RESET_MASK)));
}

/*****************************************************************************/
/**
*
* This function send a pulse on VoIP Framer to update the configured
* channel
*
* @param    InstancePtr is a pointer to the XFramer core instance.
*
* @return   None.
*
* @note     None.
*
******************************************************************************/
void XFramer_ChannelUpdate(XFramer *InstancePtr)
{
    u32 RegValue;

    RegValue = XFramer_ReadReg((InstancePtr)->Config.BaseAddress,
                   (XFRAMER_CONTROL));

    /* Set the Channel Update */
    XFramer_WriteReg((InstancePtr)->Config.BaseAddress,(XFRAMER_CONTROL),
           (RegValue | XFRAMER_CONTROL_CHANNEL_UPDATE_MASK));

    RegValue = XFramer_ReadReg((InstancePtr)->Config.BaseAddress,
                   (XFRAMER_CONTROL));

    /* Clear the Channel Update */
    XFramer_WriteReg((InstancePtr)->Config.BaseAddress,(XFRAMER_CONTROL),
           (RegValue & ~(XFRAMER_CONTROL_CHANNEL_UPDATE_MASK)));
}

/*****************************************************************************/
/**
*
* This function accesses the current configuration channel
*
* @param    InstancePtr is a pointer to the XFramer core instance.
*
* @param    Channels is current configured channel
*
* @return   None.
*
* @note     None.
*
******************************************************************************/
void XFramer_ChannelAccess(XFramer *InstancePtr, u16 Channels)
{
    u16 Index;

    /* Check Busy Bit Before accessing new/next channel */
    /* Check The Busy Bit - Wait the Busy Bit = 0 */
    Index = 0x00;
    while (XFramer_BusyBit(InstancePtr)) {
        if (Index == 65535) {
            xdbg_printf(XDBG_DEBUG_GENERAL,"Error: VoIP Framer Busy Bit Longer \
            than Expected\n\r");
            break;
        }
        Index ++;
    }

    /* Set the Channel Update */
    XFramer_WriteReg((InstancePtr)->Config.BaseAddress,
           (XFRAMER_CHANNEL_ACCESS),(Channels & XFRAMER_CHANNEL_ACCESS_MASK));
}

/*****************************************************************************/
/**
*
* This function Set the Lower Ethernet MAC Source Address
*
* @param    InstancePtr is a pointer to the XFramer core instance.
*
* @return   None.
*
* @note     None.
*
******************************************************************************/
void XFramer_EthSrcAddr0(XFramer *InstancePtr)
{
    /* Write the value to the register */
    XFramer_WriteReg(InstancePtr->Config.BaseAddress,
           XFRAMER_ETH_SRC_ADDR_LOW, (InstancePtr->Src_MACAddr_Low));
}

/*****************************************************************************/
/**
*
* This function Set the Higher Ethernet MAC Source Address
*
* @param    InstancePtr is a pointer to the XFramer core instance.
*
* @return   None.
*
* @note     None.
*
******************************************************************************/
void XFramer_EthSrcAddr1(XFramer *InstancePtr)
{
    u32 RegValue;

    /* Write the value to the register */
    XFramer_WriteReg(InstancePtr->Config.BaseAddress,
           XFRAMER_ETH_SRC_ADDR_HIGH,
             ((InstancePtr->Src_MACAddr_High) &
                       (XFRAMER_ETH_SRC_ADDR_HIGH_MASK)));
}

/*****************************************************************************/
/**
*
* This function Gets the Lower Ethernet MAC Source Address from Register
*
* @param    InstancePtr is a pointer to the XFramer core instance.
*
* @return   RegValue is the Ethernet Lower MAC Address Register Value.
*
* @note     None.
*
******************************************************************************/
u32 XFramer_GetEthSrcAddr0(XFramer *InstancePtr)
{
    u32 RegValue;

    RegValue = (XFramer_ReadReg(InstancePtr->Config.BaseAddress,
            XFRAMER_ETH_SRC_ADDR_LOW));

    return (RegValue);
}

/*****************************************************************************/
/**
*
* This function Gets the Lower Ethernet MAC Source Address from Register
*
* @param    InstancePtr is a pointer to the XFramer core instance.
*
* @return   RegValue is the Ethernet Lower MAC Address Register Value.
*
* @note     None.
*
******************************************************************************/
u16 XFramer_GetEthSrcAddr1(XFramer *InstancePtr)
{
    u16 RegValue;

    RegValue = (XFramer_ReadReg(InstancePtr->Config.BaseAddress,
            XFRAMER_ETH_SRC_ADDR_HIGH) & ((XFRAMER_ETH_SRC_ADDR_HIGH_MASK)));

    return (RegValue);
}


/*****************************************************************************/
/**
*
* This function Gets the Peak Observed Buffer Level. Ensure this value is not
* greater than the Elastic Buffer Set during Core Generation. This is Debug
* Function.
*
* @param    InstancePtr is a pointer to the XFramer core instance.
*
* @return   RegValue is the Peak Observed Buffer Level
*
* @note     None.
*
******************************************************************************/
u8 XFramer_GetPeakBufferLevel(XFramer *InstancePtr)
{
    u8 RegValue;

    RegValue = (XFramer_ReadReg(InstancePtr->Config.BaseAddress,
            XFRAMER_PEAK_BUF_LEVEL) & ((XFRAMER_PEAK_BUF_LEVEL_MASK)));

    return (RegValue);
}

/*****************************************************************************/
/**
*
* This function Clears the Peak Observed Buffer Level Register. This is Self
* Clear Register.
*
* @param    InstancePtr is a pointer to the XFramer core instance.
*
* @return   None.
*
* @note     None.
*
******************************************************************************/
void XFramer_ClearPeakBufferLevel(XFramer *InstancePtr)
{
    XFramer_WriteReg(InstancePtr->Config.BaseAddress, XFRAMER_STAT_RESET,
            (XFRAMER_STAT_RESET_PEAK_BUFF_LEVEL_MASK));
}

/*****************************************************************************/
/**
*
* This function Gets the Current Received Packet Count Statistic
*
* @param    InstancePtr is a pointer to the XFramer core instance.
*
* @return   RegValue is the Current Received Packet Count
*
* @note     None.
*
******************************************************************************/
u32 XFramer_GetRXPcktCnt(XFramer *InstancePtr)
{
    u32 RegValue;

    RegValue = (XFramer_ReadReg(InstancePtr->Config.BaseAddress,
            XFRAMER_RX_PCKT_CNT));

    return (RegValue);
}

/*****************************************************************************/
/**
*
* This function Clears the Received Packet Count Statistic Register. This is
* Self Clear Register.
*
* @param    InstancePtr is a pointer to the XFramer core instance.
*
* @return   None.
*
* @note     None.
*
******************************************************************************/
void XFramer_ClearRXPcktCnt(XFramer *InstancePtr)
{
    XFramer_WriteReg(InstancePtr->Config.BaseAddress, XFRAMER_STAT_RESET,
            (XFRAMER_STAT_RESET_RX_PCKT_CNT_MASK));
}

/*****************************************************************************/
/**
*
* This function Gets the Current Drop Packet Count Statistic
*
* @param    InstancePtr is a pointer to the XFramer core instance.
*
* @return   RegValue is the Current Drop Packet Count
*
* @note     None.
*
******************************************************************************/
u32 XFramer_GetDropPcktCnt(XFramer *InstancePtr)
{
    u32 RegValue;

    RegValue = (XFramer_ReadReg(InstancePtr->Config.BaseAddress,
            XFRAMER_DROP_PCKT_CNT));

    return (RegValue);
}

/*****************************************************************************/
/**
*
* This function Clears the Drop Packet Count Statistic Register. This is
* Self Clear Register.
*
* @param    InstancePtr is a pointer to the XFramer core instance.
*
* @return   None.
*
* @note     None.
*
******************************************************************************/
void XFramer_ClearDropPcktCnt(XFramer *InstancePtr)
{
    XFramer_WriteReg(InstancePtr->Config.BaseAddress, XFRAMER_STAT_RESET,
            (XFRAMER_STAT_RESET_DROP_PCKT_CNT_MASK));
}

/*****************************************************************************/
/**
*
* This function Set the Transmission Enable Bit
*
* @param    InstancePtr is a pointer to the XFramer core instance.
*
* @param    Channels is current configured channel
*
* @return   None.
*
* @note     None.
*
******************************************************************************/
void XFramer_TransmitEnable(XFramer *InstancePtr, u16 Channels)
{

    /* Select the Channel */
    XFramer_ChannelAccess(InstancePtr,Channels);

    /* Write the value to the register */
    XFramer_WriteReg(InstancePtr->Config.BaseAddress, XFRAMER_CHANNEL_CTRL,
        ((InstancePtr->ChannelCfg[Channels].Transmit_Enable) &
                XFRAMER_CHANNEL_CTRL_TRANSMIT_ENABLE_MASK));

    /* Update the Channels */
    XFramer_ChannelUpdate(InstancePtr);
}

/*****************************************************************************/
/**
*
* This function Gets the Current Configure Transmission Enable Bit
*
* @param    InstancePtr is a pointer to the XFramer core instance.
*
* @param    Channels is current configured channel
*
* @return   RegValue is the Transmission Enable Bit Register Value.
*
* @note     None.
*
******************************************************************************/
u8 XFramer_GetTransmitEnable(XFramer *InstancePtr, u16 Channels)
{
    u8 RegValue;

    /* Select the Channel */
    XFramer_ChannelAccess(InstancePtr,Channels);

    RegValue = (XFramer_ReadReg(InstancePtr->Config.BaseAddress,
            XFRAMER_CHANNEL_CTRL) &
                   (XFRAMER_CHANNEL_CTRL_TRANSMIT_ENABLE_MASK));

    return (RegValue);
}

/*****************************************************************************/
/**
*
* This function Sets the Lower 32 Bit Ethernet Destination MAC Address
*
* @param    InstancePtr is a pointer to the XFramer core instance.
*
* @param    Channels is current configured channel
*
* @return   None.
*
* @note     None.
*
******************************************************************************/
void XFramer_EthDestAddr0(XFramer *InstancePtr, u16 Channels)
{

    /* Select the Channel */
    XFramer_ChannelAccess(InstancePtr,Channels);

    XFramer_WriteReg(InstancePtr->Config.BaseAddress,
        XFRAMER_ETH_DEST_ADDR_LOW,
            (InstancePtr->ChannelCfg[Channels].PcktHeader.Dest_MACAddr_Low));

    /* Update the Channels */
    XFramer_ChannelUpdate(InstancePtr);
}

/*****************************************************************************/
/**
*
* This function Sets the Higher 16 Bit Ethernet Destination MAC Address
*
* @param    InstancePtr is a pointer to the XFramer core instance.
*
* @param    Channels is current configured channel
*
* @return   None.
*
* @note     None.
*
******************************************************************************/
void XFramer_EthDestAddr1(XFramer *InstancePtr, u16 Channels)
{
    u32 RegValue;

    XFramer_ChannelAccess(InstancePtr,Channels);

    RegValue = (XFramer_ReadReg(InstancePtr->Config.BaseAddress,
          XFRAMER_ETH_DEST_ADDR_HIGH) & (~(XFRAMER_ETH_DEST_ADDR_HIGH_MASK)));


    XFramer_WriteReg(InstancePtr->Config.BaseAddress,
        XFRAMER_ETH_DEST_ADDR_HIGH, ((RegValue) |
            ((InstancePtr->ChannelCfg[Channels].PcktHeader.Dest_MACAddr_High) &
                   (XFRAMER_ETH_DEST_ADDR_HIGH_MASK))));

    /* Update the Channels */
    XFramer_ChannelUpdate(InstancePtr);
}


/*****************************************************************************/
/**
*
* This function Sets the VLAN ID of the Ethernet Packet
*
* @param    InstancePtr is a pointer to the XFramer core instance.
*
* @param    Channels is current configured channel
*
* @return   None.
*
* @note     None.
*
******************************************************************************/
void XFramer_VLANID(XFramer *InstancePtr, u16 Channels)
{
    u32 RegValue;

    XFramer_ChannelAccess(InstancePtr,Channels);

    RegValue = (XFramer_ReadReg(InstancePtr->Config.BaseAddress,
            XFRAMER_VLAN_TAG_INFO) & (~(XFRAMER_VLAN_TAG_INFO_VLAN_ID_MASK)));

    if (InstancePtr->ChannelCfg[Channels].is_vlan == XFRAMER_VLAN_ENABLE) {
      XFramer_WriteReg(InstancePtr->Config.BaseAddress,
         XFRAMER_VLAN_TAG_INFO, ((RegValue) |
          ((InstancePtr->ChannelCfg[Channels].PcktHeader.vlan_pcp_cfi_vid) &
               (XFRAMER_VLAN_TAG_INFO_VLAN_ID_MASK))));
    }

    /* Update the Channels */
    XFramer_ChannelUpdate(InstancePtr);
}

/*****************************************************************************/
/**
*
* This function Set the Core to Generate Packets with VLAN
*
* @param    InstancePtr is a pointer to the XFramer core instance.
*
* @param    Channels is current configured channel
*
* @return   None.
*
* @note     None.
*
******************************************************************************/
void XFramer_SetVLAN(XFramer *InstancePtr, u16 Channels)
{
    u32 RegValue;

    XFramer_ChannelAccess(InstancePtr,Channels);

    RegValue = (XFramer_ReadReg(InstancePtr->Config.BaseAddress,
            XFRAMER_VLAN_TAG_INFO) &
                   (~(XFRAMER_VLAN_TAG_INFO_WITH_VLAN_MASK)));

    XFramer_WriteReg(InstancePtr->Config.BaseAddress, XFRAMER_VLAN_TAG_INFO,
            ((RegValue) | (((InstancePtr->ChannelCfg[Channels]. is_vlan) <<
                    (XFRAMER_VLAN_TAG_INFO_WITH_VLAN_SHIFT)) &
                        (XFRAMER_VLAN_TAG_INFO_WITH_VLAN_MASK))));

    /* Update the Channels */
    XFramer_ChannelUpdate(InstancePtr);
}

/*****************************************************************************/
/**
*
* This function Set the Media Type of Service (TOS). Component of the
* IP Header.
*
* @param    InstancePtr is a pointer to the XFramer core instance.
*
* @param    Channels is current configured channel
*
* @return   None.
*
* @note     None.
*
******************************************************************************/
void XFramer_MediaTOS(XFramer *InstancePtr, u16 Channels)
{
    u32 RegValue;

    XFramer_ChannelAccess(InstancePtr,Channels);

    RegValue = (XFramer_ReadReg(InstancePtr->Config.BaseAddress,
            XFRAMER_MEDIA_IP_VER_TOS_TTL) &
                   (~(XFRAMER_IP_VER_TOS_TTL_TOS_MASK)));

    XFramer_WriteReg(InstancePtr->Config.BaseAddress,
          XFRAMER_MEDIA_IP_VER_TOS_TTL, ((RegValue) |
                (((InstancePtr->ChannelCfg[Channels].PcktHeader.media_tos) <<
                           (XFRAMER_IP_VER_TOS_TTL_TOS_SHIFT)) &
                                  (XFRAMER_IP_VER_TOS_TTL_TOS_MASK))));

    /* Update the Channels */
    XFramer_ChannelUpdate(InstancePtr);

}

/*****************************************************************************/
/**
*
* This function Set the Media Time-To-Live (TTL). Component of the
* IP Header.
*
* @param    InstancePtr is a pointer to the XFramer core instance.
*
* @param    Channels is current configured channel
*
* @return   None.
*
* @note     None.
*
******************************************************************************/
void XFramer_MediaTTL(XFramer *InstancePtr, u16 Channels)
{
    u32 RegValue;

    XFramer_ChannelAccess(InstancePtr,Channels);

    RegValue = (XFramer_ReadReg(InstancePtr->Config.BaseAddress,
            XFRAMER_MEDIA_IP_VER_TOS_TTL) &
                  (~(XFRAMER_IP_VER_TOS_TTL_TTL_MASK)));

    XFramer_WriteReg(InstancePtr->Config.BaseAddress,
           XFRAMER_MEDIA_IP_VER_TOS_TTL,
            ((RegValue) |
                   ((InstancePtr->ChannelCfg[Channels].PcktHeader.media_ttl) &
                         (XFRAMER_IP_VER_TOS_TTL_TTL_MASK))));

    /* Update the Channels */
    XFramer_ChannelUpdate(InstancePtr);
}

/*****************************************************************************/
/**
*
* This function Set the FEC Type of Service (TOS). Component of the
* IP Header.
*
* @param    InstancePtr is a pointer to the XFramer core instance.
*
* @param    Channels is current configured channel
*
* @return   None.
*
* @note     None.
*
******************************************************************************/
void XFramer_FECTOS(XFramer *InstancePtr, u16 Channels)
{
    u32 RegValue;

    XFramer_ChannelAccess(InstancePtr,Channels);

    RegValue = (XFramer_ReadReg(InstancePtr->Config.BaseAddress,
            XFRAMER_FEC_IP_VER_TOS_TTL) &
                  (~(XFRAMER_IP_VER_TOS_TTL_TOS_MASK)));

    XFramer_WriteReg(InstancePtr->Config.BaseAddress,
           XFRAMER_FEC_IP_VER_TOS_TTL,
                  ((RegValue) |
                  (((InstancePtr->ChannelCfg[Channels].PcktHeader.fec_tos) <<
                         (XFRAMER_IP_VER_TOS_TTL_TOS_SHIFT)) &
                                 (XFRAMER_IP_VER_TOS_TTL_TOS_MASK))));

    /* Update the Channels */
    XFramer_ChannelUpdate(InstancePtr);

}

/*****************************************************************************/
/**
*
* This function Set the FEC Time-To-Live (TTL). Component of the
* IP Header.
*
* @param    InstancePtr is a pointer to the XFramer core instance.
*
* @param    Channels is current configured channel
*
* @return   None.
*
* @note     None.
*
******************************************************************************/
void XFramer_FECTTL(XFramer *InstancePtr, u16 Channels)
{
    u32 RegValue;

    XFramer_ChannelAccess(InstancePtr,Channels);

    RegValue = (XFramer_ReadReg(InstancePtr->Config.BaseAddress,
            XFRAMER_FEC_IP_VER_TOS_TTL) &
                  (~(XFRAMER_IP_VER_TOS_TTL_TTL_MASK)));

    XFramer_WriteReg(InstancePtr->Config.BaseAddress,
           XFRAMER_FEC_IP_VER_TOS_TTL,
                 ((RegValue) |
                 ((InstancePtr->ChannelCfg[Channels].PcktHeader.fec_ttl) &
                           (XFRAMER_IP_VER_TOS_TTL_TTL_MASK))));

    /* Update the Channels */
    XFramer_ChannelUpdate(InstancePtr);
}

/*****************************************************************************/
/**
*
* This function Sets the IP Destination Address.
*
* @param    InstancePtr is a pointer to the XFramer core instance.
*
* @param    Channels is current configured channel
*
* @return   None.
*
* @note     None.
*
******************************************************************************/
void XFramer_IP0Dest(XFramer *InstancePtr, u16 Channels)
{

    XFramer_ChannelAccess(InstancePtr,Channels);


    XFramer_WriteReg(InstancePtr->Config.BaseAddress, XFRAMER_DEST_IP0,
            (InstancePtr->ChannelCfg[Channels].PcktHeader.ip_dest_address));

    /* Update the Channels */
    XFramer_ChannelUpdate(InstancePtr);

}

/*****************************************************************************/
/**
*
* This function Sets the IP Source Address.
*
* @param    InstancePtr is a pointer to the XFramer core instance.
*
* @param    Channels is current configured channel
*
* @return   None.
*
* @note     None.
*
******************************************************************************/
void XFramer_IP0Src(XFramer *InstancePtr, u16 Channels)
{

    XFramer_ChannelAccess(InstancePtr,Channels);

    XFramer_WriteReg(InstancePtr->Config.BaseAddress, XFRAMER_SRC_IP0,
            (InstancePtr->ChannelCfg[Channels].PcktHeader.ip_src_address));

    /* Update the Channels */
    XFramer_ChannelUpdate(InstancePtr);

}

/*****************************************************************************/
/**
*
* This function Sets the UDP Destination Port
*
* @param    InstancePtr is a pointer to the XFramer core instance.
*
* @param    Channels is current configured channel
*
* @return   None.
*
* @note     None.
*
******************************************************************************/
void XFramer_UDPDest(XFramer *InstancePtr, u16 Channels)
{
    u32 RegValue;

    XFramer_ChannelAccess(InstancePtr,Channels);

    RegValue = (XFramer_ReadReg(InstancePtr->Config.BaseAddress,
            XFRAMER_DEST_UDP_PORT) &
                   ~(XFRAMER_DEST_UDP_PORT_MASK));

    XFramer_WriteReg(InstancePtr->Config.BaseAddress, XFRAMER_DEST_UDP_PORT,
            (RegValue |
            ((InstancePtr->ChannelCfg[Channels].PcktHeader.udp_dest_port) &
                    XFRAMER_DEST_UDP_PORT_MASK)));

    /* Update the Channels */
    XFramer_ChannelUpdate(InstancePtr);

}

/*****************************************************************************/
/**
*
* This function Sets the UDP Source Port
*
* @param    InstancePtr is a pointer to the XFramer core instance.
*
* @param    Channels is current configured channel
*
* @return   None.
*
* @note     None.
*
******************************************************************************/
void XFramer_UDPSrc(XFramer *InstancePtr, u16 Channels)
{
    u32 RegValue;

    XFramer_ChannelAccess(InstancePtr,Channels);

    RegValue = (XFramer_ReadReg(InstancePtr->Config.BaseAddress,
            XFRAMER_SOURCE_UDP_PORT) & ~(XFRAMER_SOURCE_UDP_PORT_MASK));

    XFramer_WriteReg(InstancePtr->Config.BaseAddress, XFRAMER_SOURCE_UDP_PORT,
            (RegValue |
            ((InstancePtr->ChannelCfg[Channels].PcktHeader.udp_src_port) &
                    XFRAMER_SOURCE_UDP_PORT_MASK)));

    /* Update the Channels */
    XFramer_ChannelUpdate(InstancePtr);

}


/*****************************************************************************/
/**
*
* This function Gets the current transmitted packet count statistic
*
* @param    InstancePtr is a pointer to the XFramer core instance.
*
* @param    Channels is current configured channel
*
* @return   None.
*
* @note     None.
*
******************************************************************************/
u32 XFramer_GetTXPcktCnt(XFramer *InstancePtr, u16 Channels)
{
    u32 RegValue;

    XFramer_ChannelAccess(InstancePtr,Channels);

    RegValue = (XFramer_ReadReg(InstancePtr->Config.BaseAddress,
            XFRAMER_TX_PKT_CNT));

    return (RegValue);
}

/*****************************************************************************/
/**
*
* This function Clears the current transmitted packet count statistic register.
* This is a self clear register.
*
* @param    InstancePtr is a pointer to the XFramer core instance.
*
* @param    Channels is current configured channel
*
* @return   None.
*
* @note     None.
*
******************************************************************************/
void XFramer_ClearTXPcktCnt(XFramer *InstancePtr, u16 Channels)
{

    XFramer_ChannelAccess(InstancePtr,Channels);

    XFramer_WriteReg(InstancePtr->Config.BaseAddress, XFRAMER_CHAN_STAT_RESET,
            (XFRAMER_CHAN_STAT_RESET_TX_PKT_CNT_MASK));

    /* Update the Channels */
    XFramer_ChannelUpdate(InstancePtr);

}
