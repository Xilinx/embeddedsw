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
* @file xvoipfec_rx.c
*
* This is the main file for Xilinx VoIP FEC Receiver core. Please see
* xvoipfec_rx.h for more details of the driver.
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

#include "xvoipfec_rx.h"
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
* This function initializes the VoIP FEC Receiver core. This function must
* be called prior to using the VoIP FEC Receiver core. Initialization of the
* VoIP FEC Receiver includes setting up the instance data and ensuring the
* hardware is in a quiescent state.
*
* @param    InstancePtr is a pointer to the XVoipFEC_RX core instance.
* @param    CfgPtr points to the configuration structure associated with
*       the VoIP FEC Receiver core.
* @param    EffectiveAddr is the base address of the device. If address
*       translation is being used, then this parameter must reflect the
*       virtual base address. Otherwise, the physical address should be
*       used.
*
* @return
*       - XST_SUCCESS if XVoipFEC_RX_CfgInitialize was successful.
*       - XST_FAILURE
*
* @note     None.
*
******************************************************************************/
int XVoipFEC_RX_CfgInitialize(XVoipFEC_RX *InstancePtr,
                                          XVoipFEC_RX_Config *CfgPtr,
                                                          UINTPTR EffectiveAddr)
{
    u16 Index;

    /* Verify arguments. */
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(CfgPtr != NULL);
    Xil_AssertNonvoid(EffectiveAddr != (u32)0x0);

    /* Setup the instance */
    (void)memset((void *)InstancePtr, 0, sizeof(XVoipFEC_RX));
    (void)memcpy((void *)&(InstancePtr->Config), (const void *)CfgPtr,
                                                   sizeof(XVoipFEC_RX_Config));
    InstancePtr->Config.BaseAddress = EffectiveAddr;

    /* Resets all the FEC RX Register */
    XVoipFEC_RX_SoftReset(InstancePtr);

    /* Setting Default Channel Parameters */
    for (Index = (u16)0x00; Index <
                 (u16)((InstancePtr)->Config.HWChannelNumber); Index++) {
        /* Setting Channels to Recovery Disable */
        InstancePtr->ChannelCfg[Index].FECParams.FEC_Recovery_Disable =
                                               XVOIPFEC_RX_FEC_RECOVERY_ENABLE;
        /* Setting Channels to ReOrder the Incoming Packet Based on Incoming
        * RTP Sequence Number */
        InstancePtr->ChannelCfg[Index].FECParams.Media_Packet_Bypass =
                                            XVOIPFEC_RX_MEDIA_PACKET_PROCESSED;
        /* Setting Channels to Disable */
        InstancePtr->ChannelCfg[Index].FECParams.Channel_Enable =
                                                   XVOIPFEC_RX_CHANNEL_DISABLE;
        /* Configure the Register Based on Above Configuration */
        XVoipFEC_RX_SetFECParams(InstancePtr,Index);
    }

    /* Reset the hardware and set the flag to indicate the driver is ready */
    InstancePtr->IsReady = (u32)(XIL_COMPONENT_IS_READY);

    return (XST_SUCCESS);

}

/*****************************************************************************/
/**
*
* This function obtains the core configuration value through register. Debug
* Function to compare with generated xvoipfec_rx_g.c generated file.
*
* @param    InstancePtr is a pointer to the VoIP FEC Receiver core instance.
*
* @return   None.
*
* @note     None.
*
******************************************************************************/
XVoipFEC_RX_Config XVoipFEC_RX_CoreInfo(XVoipFEC_RX *InstancePtr)
{

    XVoipFEC_RX_Config XVoipFEC_RX_ConfigRegValue;

    /* Get Core Info */
    /* Get Maximum Supported Channel in Hardware  */
    XVoipFEC_RX_ConfigRegValue.HWChannelNumber =
         (XVoipFEC_RX_ReadReg(InstancePtr->Config.BaseAddress,
               XVOIPFEC_RX_SYSTEM_CONFIG_REG_OFFSET) &
                    XVOIPFEC_RX_SYS_CONFIG_C_CHANNELS_MASK);

    /* Get FEC Recovery Enabled in Hardware  */
    XVoipFEC_RX_ConfigRegValue.FECEnable =
         ((XVoipFEC_RX_ReadReg(InstancePtr->Config.BaseAddress,
            XVOIPFEC_RX_SYSTEM_CONFIG_REG_OFFSET) &
                 XVOIPFEC_RX_SYS_CONFIG_FEC_RECOVERY_SUPP_MASK) >>
                      (XVOIPFEC_RX_SYS_CONFIG_FEC_RECOVERY_SUPP_SHIFT));

    /* Get Seamless Protection Enabled in Hardware  */
    XVoipFEC_RX_ConfigRegValue.SeamlessEnable =
         ((XVoipFEC_RX_ReadReg(InstancePtr->Config.BaseAddress,
                XVOIPFEC_RX_SYSTEM_CONFIG_REG_OFFSET) &
                     XVOIPFEC_RX_SYS_CONFIG_SEAMLESS_SWITCHING_SUPP_MASK) >>
                     (XVOIPFEC_RX_SYS_CONFIG_SEAMLESS_SWITCHING_SUPP_SHIFT));

    return XVoipFEC_RX_ConfigRegValue;
}

/*****************************************************************************/
/**
*
* This function obtains the VoIP FEC Receiver ST2022-1/5 Status (FEC Status)
*
* @param    InstancePtr is a pointer to the XVoipFEC_RX core instance.
*
* @param    Channels is current configured channel
*
* @return   None.
*
* @note     None.
*
******************************************************************************/
XVoipFEC_RX_FECStatus XVoipFEC_RX_FECStatusRegValue(XVoipFEC_RX *InstancePtr,
                                                                  u16 Channels)
{
    u32 RegValue = 0x00;
    XVoipFEC_RX_FECStatus XVoipFEC_RX_FECStatus_RegValue;

    /* Select Channel */
    XVoipFEC_RX_ChannelAccess(InstancePtr,Channels);

    /* Collumn Detected */
    RegValue = ((XVoipFEC_RX_ReadReg(InstancePtr->Config.BaseAddress,
            XVOIPFEC_RX_CHANNEL_STATUS_REG_OFFSET) &
                 (XVOIPFEC_RX_CHANNEL_STATUS_COL_FEC_DETECTED_MASK)) >>
                       XVOIPFEC_RX_CHANNEL_STATUS_COL_FEC_DETECTED_SHIFT);


    XVoipFEC_RX_FECStatus_RegValue.Col_Detected = (u8)RegValue;


    /* Row Detected */
    RegValue = ((XVoipFEC_RX_ReadReg(InstancePtr->Config.BaseAddress,
            XVOIPFEC_RX_CHANNEL_STATUS_REG_OFFSET) &
                 (XVOIPFEC_RX_CHANNEL_STATUS_ROW_FEC_DETECTED_MASK)) >>
                       XVOIPFEC_RX_CHANNEL_STATUS_ROW_FEC_DETECTED_SHIFT);


    XVoipFEC_RX_FECStatus_RegValue.Row_Detected = (u8)RegValue;

    /* FEC D */
    RegValue = ((XVoipFEC_RX_ReadReg(InstancePtr->Config.BaseAddress,
            XVOIPFEC_RX_CHANNEL_STATUS_REG_OFFSET) &
                 (XVOIPFEC_RX_CHANNEL_STATUS_FEC_D_MASK)) >>
                       XVOIPFEC_RX_CHANNEL_STATUS_FEC_D_SHIFT);


    XVoipFEC_RX_FECStatus_RegValue.FEC_D = (u16)RegValue;

    /* FEC L */
    RegValue = (XVoipFEC_RX_ReadReg(InstancePtr->Config.BaseAddress,
            XVOIPFEC_RX_CHANNEL_STATUS_REG_OFFSET) &
                 (XVOIPFEC_RX_CHANNEL_STATUS_FEC_L_MASK));


    XVoipFEC_RX_FECStatus_RegValue.FEC_L = (u16)RegValue;


    /* Buffer Depth Status */
    RegValue = (XVoipFEC_RX_ReadReg(InstancePtr->Config.BaseAddress,
            XVOIPFEC_RX_CURR_BUFFER_DEPTH_REG_OFFSET) &
                    (XVOIPFEC_RX_CURR_BUFFER_DEPTH_MASK));


    XVoipFEC_RX_FECStatus_RegValue.Buffer_Depth = (u16)RegValue;


    return (XVoipFEC_RX_FECStatus_RegValue);
}

/*****************************************************************************/
/**
*
* This function obtains the VoIP FEC Receiver Current Channel Statistic
*
* @param    InstancePtr is a pointer to the XVoipFEC_RX core instance.
*
* @param    Channels is current configured channel
*
* @return   None.
*
* @note     None.
*
******************************************************************************/
XVoipFEC_RX_FECStatistics XVoipFEC_RX_FECStatisticsRegValue
                                      (XVoipFEC_RX *InstancePtr, u16 Channels)
{
    XVoipFEC_RX_FECStatistics XVoipFEC_RX_FECStatistics_RegValue;

    /* Select Channel */
    XVoipFEC_RX_ChannelAccess(InstancePtr,Channels);

    /* Valids Packet Count */
    XVoipFEC_RX_FECStatistics_RegValue.valid_pkts_cnt =
            (XVoipFEC_RX_ReadReg(InstancePtr->Config.BaseAddress,
                  XVOIPFEC_RX_VALID_PKTS_CNT_REG_OFFSET));


    /* Unrecoverable Packet Count */
    XVoipFEC_RX_FECStatistics_RegValue.unrecv_pkts_cnt =
         (XVoipFEC_RX_ReadReg(InstancePtr->Config.BaseAddress,
               XVOIPFEC_RX_UNRECV_PKTS_CNT_REG_OFFSET));

    /* Corrected Packet Count */
    XVoipFEC_RX_FECStatistics_RegValue.corr_pkts_cnt =
         (XVoipFEC_RX_ReadReg(InstancePtr->Config.BaseAddress,
               XVOIPFEC_RX_CORR_PKTS_CNT_REG_OFFSET));

    /* Duplicated Packet Count */
    XVoipFEC_RX_FECStatistics_RegValue.dup_pkts_cnt =
         (XVoipFEC_RX_ReadReg(InstancePtr->Config.BaseAddress,
               XVOIPFEC_RX_DUP_PKTS_CNT_REG_OFFSET));

    /* Out of Range Packet */
    XVoipFEC_RX_FECStatistics_RegValue.oor_pkts_cnt =
         (XVoipFEC_RX_ReadReg(InstancePtr->Config.BaseAddress,
               XVOIPFEC_RX_OOR_PKTS_CNT_REG_OFFSE));

    return (XVoipFEC_RX_FECStatistics_RegValue);
}


/*****************************************************************************/
/**
*
* This function obtains the VoIP FEC Receiver Current Hitless Status.
* Currently checking the RTP Timestamp Difference between 2 Links on a
* RTP Sequence Number
*
* @param    InstancePtr is a pointer to the XVoipFEC_RX core instance.
*
* @param    Channels is current configured channel
*
* @return   None.
*
* @note     None.
*
******************************************************************************/
XVoipFEC_RX_HitlessStatus XVoipFEC_HitlessStatusRegValue
                                      (XVoipFEC_RX *InstancePtr, u16 Channels)
{
    XVoipFEC_RX_HitlessStatus XVoipFEC_RX_HitlessStatus_RegValue;

    XVoipFEC_RX_ChannelAccess(InstancePtr,Channels);

    XVoipFEC_RX_HitlessStatus_RegValue.RTP_TS_Diff =
            (XVoipFEC_RX_ReadReg(InstancePtr->Config.BaseAddress,
                  XVOIPFEC_RX_RTP_TS_DIFFERENCE_REG_OFFSET) &
                       (XVOIPFEC_RX_SEAMLESS_PROTECT_RTP_TS_DIFF_MASK));

    return (XVoipFEC_RX_HitlessStatus_RegValue);
}

/*****************************************************************************/
/**
*
* This function obtains the VoIP FEC Receiver Current Channel Configured
* Parameters
*
* @param    InstancePtr is a pointer to the XVoipFEC_RX core instance.
*
* @param    Channels is current configured channel
*
* @return   None.
*
* @note     None.
*
******************************************************************************/
XVoipFEC_RX_FECParams XVoipFEC_RX_FECParamsRegValue(XVoipFEC_RX *InstancePtr,
                                                                u16 Channels)
{
    XVoipFEC_RX_FECParams XVoipFEC_RX_FECParams_RegValue;

    /* Select Channel */
    XVoipFEC_RX_ChannelAccess(InstancePtr,Channels);

    /* FEC Recovery Enable/Disable */
    XVoipFEC_RX_FECParams_RegValue.FEC_Recovery_Disable =
         (XVoipFEC_RX_ReadReg(InstancePtr->Config.BaseAddress,
               XVOIPFEC_RX_FEC_CHAN_CONFIG_REG_OFFSET) &
                    (XVOIPFEC_RX_FEC_CHAN_CONFIG_FEC_RECV_DISABLE_MASK)) >>
                          XVOIPFEC_RX_FEC_CHAN_CONFIG_FEC_RECV_DISABLE_SHIFT;


    /* Media Packet Processed/Bypass */
    XVoipFEC_RX_FECParams_RegValue.Media_Packet_Bypass =
         (XVoipFEC_RX_ReadReg(InstancePtr->Config.BaseAddress,
               XVOIPFEC_RX_FEC_CHAN_CONFIG_REG_OFFSET) &
                    (XVOIPFEC_RX_FEC_CHAN_CONFIG_MEDIA_PKT_BYPASS_MASK)) >>
                          XVOIPFEC_RX_FEC_CHAN_CONFIG_MEDIA_PKT_BYPASS_SHIFT;

    /* Channel Enable/Disable */
    XVoipFEC_RX_FECParams_RegValue.Channel_Enable =
         (XVoipFEC_RX_ReadReg(InstancePtr->Config.BaseAddress,
               XVOIPFEC_RX_FEC_CHAN_CONFIG_REG_OFFSET) &
                    (XVOIPFEC_RX_FEC_CHAN_CONFIG_CHANNEL_ENABLE_MASK));

    return (XVoipFEC_RX_FECParams_RegValue);
}

/*****************************************************************************/
/**
*
* This function configures the VoIP FEC Receiver Current Core
*
* @param    InstancePtr is a pointer to the XVoipFEC_RX core instance.
*
* @return   None.
*
* @note     None.
*
******************************************************************************/
void XVoipFEC_RX_CoreChannelConfig(XVoipFEC_RX *InstancePtr)
{
    u16 Index;

    /* Set FEC Processing Delay */
    XVoipFEC_RX_SetFECProcessingDelay(InstancePtr);

    /* Setting Parameters based on Drivers */
    for (Index = (u16)0x00; Index < (u16)(InstancePtr)->OpChannel; Index++) {
        /* Select Channels */
        XVoipFEC_RX_ChannelAccess(InstancePtr,Index);

        /* Set FEC Configuration */
        XVoipFEC_RX_SetFECParams(InstancePtr, Index);

        /* Update the Channels */
        XVoipFEC_RX_ChannelUpdate(InstancePtr);
    }
}

/*****************************************************************************/
/**
*
* This function configures the VoIP FEC Receiver Channels Parameters
*
* @param    InstancePtr is a pointer to the XVoipFEC_RX core instance.
*
* @param    Channels is current configured channel
*
* @return   None.
*
* @note     None.
*
******************************************************************************/
void XVoipFEC_RX_SetFECParams(XVoipFEC_RX *InstancePtr, u16 Channels)
{
    /* Select Channels */
    XVoipFEC_RX_ChannelAccess(InstancePtr,Channels);

    /* Clear Channels */
    XVoipFEC_RX_ClearChannel(InstancePtr,Channels);

    /* Set FEC Recovery Disable */
    XVoipFEC_RX_FECRecoveryDisable(InstancePtr,Channels);

    /* Set Media Bypass */
    XVoipFEC_RX_MediaPacketBypass(InstancePtr,Channels);

    /* Set OOR Management */
    XVoipFEC_RX_OORCfg(InstancePtr,Channels);

    /* Set Channel Enable */
    XVoipFEC_RX_ChannelEnable(InstancePtr,Channels);

    /* Channel Update */
    XVoipFEC_RX_ChannelUpdate(InstancePtr);
}

/*****************************************************************************/
/**
*
* This function clears the Current Channel State.
* Note: This Function Should be called when the channel is disabled
*
* @param    InstancePtr is a pointer to the XVoipFEC_RX core instance.
*
* @param    Channels is current configured channel
*
* @return   None.
*
* @note     None.
*
******************************************************************************/
void XVoipFEC_RX_ClearChannel(XVoipFEC_RX *InstancePtr, u16 Channels)
{
    u16 Index;

    /* Select Channels */
    XVoipFEC_RX_ChannelAccess(InstancePtr,Channels);

    /* Set Clear Channel */
    XVoipFEC_RX_ChannelClear(InstancePtr);

    /* Check The Busy Bit - Wait the Busy Bit = 0 */
    Index = 0x00;
    while (XVoipFEC_RX_BusyBit(InstancePtr)) {
        if (Index == 65535){
            xdbg_printf(XDBG_DEBUG_GENERAL,"Error: VoIP FEC RX Busy Bit Longer \
            than Expected\n\r");
            break;
        }
        Index ++;
    }
}

/*****************************************************************************/
/**
*
* This function perform software resets on VoIP FEC Receiver, which clears
* all the registers.
*
* @param    InstancePtr is a pointer to the XVoipFEC_RX core instance.
*
* @return   None.
*
* @note     None.
*
******************************************************************************/
void XVoipFEC_RX_SoftReset(XVoipFEC_RX *InstancePtr)
{
    u32 RegValue;

    RegValue = XVoipFEC_RX_ReadReg((InstancePtr)->Config.BaseAddress,
            (XVOIPFEC_RX_CONTROL_REG_OFFSET)) &
                  ((~(XVOIPFEC_RX_CONTROL_SOFT_RESET_MASK)));

    /* Set the Channel Update */
    XVoipFEC_RX_WriteReg((InstancePtr)->Config.BaseAddress,
         (XVOIPFEC_RX_CONTROL_REG_OFFSET),(RegValue |
               XVOIPFEC_RX_CONTROL_SOFT_RESET_MASK));

    RegValue = XVoipFEC_RX_ReadReg((InstancePtr)->Config.BaseAddress,
            (XVOIPFEC_RX_CONTROL_REG_OFFSET)) &
                  ((~(XVOIPFEC_RX_CONTROL_SOFT_RESET_MASK)));

    /* Clear the Channel Update */
    XVoipFEC_RX_WriteReg((InstancePtr)->Config.BaseAddress,
         (XVOIPFEC_RX_CONTROL_REG_OFFSET),(RegValue &
               (~(XVOIPFEC_RX_CONTROL_SOFT_RESET_MASK))));
}

/*****************************************************************************/
/**
*
* This function send a pulse on VoIP FEC Receiver to update the configured
* channel
*
* @param    InstancePtr is a pointer to the XVoipFEC_RX core instance.
*
* @return   None.
*
* @note     None.
*
******************************************************************************/
void XVoipFEC_RX_ChannelUpdate(XVoipFEC_RX *InstancePtr)
{
    u32 RegValue;

    RegValue = XVoipFEC_RX_ReadReg((InstancePtr)->Config.BaseAddress,
            (XVOIPFEC_RX_CONTROL_REG_OFFSET)) &
                  ((~(XVOIPFEC_RX_CONTROL_CHANNEL_UPDATE_MASK)));

    /* Set the Channel Update */
    XVoipFEC_RX_WriteReg((InstancePtr)->Config.BaseAddress,
         (XVOIPFEC_RX_CONTROL_REG_OFFSET),(RegValue |
               XVOIPFEC_RX_CONTROL_CHANNEL_UPDATE_MASK));

    RegValue = XVoipFEC_RX_ReadReg((InstancePtr)->Config.BaseAddress,
            (XVOIPFEC_RX_CONTROL_REG_OFFSET)) &
                  ((~(XVOIPFEC_RX_CONTROL_CHANNEL_UPDATE_MASK)));

    /* Clear the Channel Update */
    XVoipFEC_RX_WriteReg((InstancePtr)->Config.BaseAddress,
         (XVOIPFEC_RX_CONTROL_REG_OFFSET),(RegValue &
               (~(XVOIPFEC_RX_CONTROL_CHANNEL_UPDATE_MASK))));
}

/*****************************************************************************/
/**
*
* This function send a pulse on VoIP FEC Receiver to clear the current
* accessed Channel.
*
* @param    InstancePtr is a pointer to the XVoipFEC_RX core instance.
*
* @return   None.
*
* @note     None.
*
******************************************************************************/
void XVoipFEC_RX_ChannelClear(XVoipFEC_RX *InstancePtr)
{
    u32 RegValue;

    RegValue = XVoipFEC_RX_ReadReg((InstancePtr)->Config.BaseAddress,
            (XVOIPFEC_RX_CONTROL_REG_OFFSET)) &
                 ((~(XVOIPFEC_RX_CONTROL_CHANNEL_CLEAR_MASK)));

    /* Set the Channel Update */
    XVoipFEC_RX_WriteReg((InstancePtr)->Config.BaseAddress,
         (XVOIPFEC_RX_CONTROL_REG_OFFSET),(RegValue |
               XVOIPFEC_RX_CONTROL_CHANNEL_CLEAR_MASK));

    RegValue = XVoipFEC_RX_ReadReg((InstancePtr)->Config.BaseAddress,
            (XVOIPFEC_RX_CONTROL_REG_OFFSET)) &
                  ((~(XVOIPFEC_RX_CONTROL_CHANNEL_CLEAR_MASK)));

    /* Clear the Channel Update */
    XVoipFEC_RX_WriteReg((InstancePtr)->Config.BaseAddress,
         (XVOIPFEC_RX_CONTROL_REG_OFFSET),(RegValue &
               (~(XVOIPFEC_RX_CONTROL_CHANNEL_CLEAR_MASK))));
}

/*****************************************************************************/
/**
*
* This function Accesses the Configurable Channel
*
* @param    InstancePtr is a pointer to the XVoipFEC_RX core instance.
*
* @param    Channels is current configured channel
*
* @return   None.
*
* @note     None.
*
******************************************************************************/
void XVoipFEC_RX_ChannelAccess(XVoipFEC_RX *InstancePtr, u16 Channels)
{
    /* Set the Channel Update */
    XVoipFEC_RX_WriteReg((InstancePtr)->Config.BaseAddress,
         (XVOIPFEC_RX_CHANNEL_ACCESS_REG_OFFSET),
               (Channels & XVOIPFEC_RX_CHANNEL_ACCESS_MASK));
}

/*****************************************************************************/
/**
*
* This function configures the FEC Processing Delay
*
* @param    InstancePtr is a pointer to the XVoipFEC_RX core instance.
*
* @return   None.
*
* @note     None.
*
******************************************************************************/
void XVoipFEC_RX_SetFECProcessingDelay(XVoipFEC_RX *InstancePtr)
{
    /* Set the Channel Update */
    XVoipFEC_RX_WriteReg((InstancePtr)->Config.BaseAddress,
         (XVOIPFEC_RX_FEC_PROCESSING_DELAY_REG_OFFSET),
               (InstancePtr->FECProcessingDelay));
}

/*****************************************************************************/
/**
*
* This function Gets the configured FEC Processing Delay
*
* @param    InstancePtr is a pointer to the XVoipFEC_RX core instance.
*
* @return   None.
*
* @note     None.
*
******************************************************************************/
u32 XVoipFEC_RX_GetFECProcessingDelay(XVoipFEC_RX *InstancePtr)
{
    u32 RegValue;

    /* Set the Channel Update */
    RegValue = XVoipFEC_RX_ReadReg((InstancePtr)->Config.BaseAddress,
         (XVOIPFEC_RX_FEC_PROCESSING_DELAY_REG_OFFSET));

    return RegValue;
}

/*****************************************************************************/
/**
*
* This function Gets the configured FEC Packet Drop
*
* @param    InstancePtr is a pointer to the XVoipFEC_RX core instance.
*
* @return   None.
*
* @note     None.
*
******************************************************************************/
u32 XVoipFEC_RX_GetFECPacketDropCnt(XVoipFEC_RX *InstancePtr)
{
    u32 RegValue;

    /* Set the Channel Update */
    RegValue = XVoipFEC_RX_ReadReg((InstancePtr)->Config.BaseAddress,
         (XVOIPFEC_RX_FEC_PACKET_DROP_REG_OFFSET));

    return RegValue;
}

/*****************************************************************************/
/**
*
* This function Gets the configured FEC Peak Buffer Data Count
*
* @param    InstancePtr is a pointer to the XVoipFEC_RX core instance.
*
* @return   None.
*
* @note     None.
*
******************************************************************************/
u32 XVoipFEC_RX_GetFECBufferPeakDataCnt(XVoipFEC_RX *InstancePtr)
{
    u32 RegValue;

    /* Set the Channel Update */
    RegValue = XVoipFEC_RX_ReadReg((InstancePtr)->Config.BaseAddress,
         (XVOIPFEC_RX_FEC_PEAK_DATA_CNT_REG_OFFSET));

    return RegValue;
}

/*****************************************************************************/
/**
*
* This function Enables or Disable the Channel Based on User Input to the
* core structure
*
* @param    InstancePtr is a pointer to the XVoipFEC_RX core instance.
*
* @param    Channels is current configured channel
*
* @return   None.
*
* @note     None.
*
******************************************************************************/
void XVoipFEC_RX_ChannelEnable(XVoipFEC_RX *InstancePtr, u16 Channels)
{
    u32 RegValue;

    XVoipFEC_RX_ChannelAccess(InstancePtr,Channels);

    RegValue = (XVoipFEC_RX_ReadReg(InstancePtr->Config.BaseAddress,
            XVOIPFEC_RX_FEC_CHAN_CONFIG_REG_OFFSET) &
                 (~(XVOIPFEC_RX_FEC_CHAN_CONFIG_CHANNEL_ENABLE_MASK)));

    XVoipFEC_RX_WriteReg(InstancePtr->Config.BaseAddress,
         XVOIPFEC_RX_FEC_CHAN_CONFIG_REG_OFFSET,
              (RegValue | ((InstancePtr->ChannelCfg[Channels].
                  FECParams.Channel_Enable) &
                     XVOIPFEC_RX_FEC_CHAN_CONFIG_CHANNEL_ENABLE_MASK)));

    /* Update the Channels */
    XVoipFEC_RX_ChannelUpdate(InstancePtr);
}

/*****************************************************************************/
/**
*
* This function Enables or Disable the Packet Processing Based on User Input
* to the core structure
*
* @param    InstancePtr is a pointer to the XVoipFEC_RX core instance.
*
* @param    Channels is current configured channel
*
* @return   None.
*
* @note     None.
*
******************************************************************************/
void XVoipFEC_RX_MediaPacketBypass(XVoipFEC_RX *InstancePtr, u16 Channels)
{
    u32 RegValue;

    XVoipFEC_RX_ChannelAccess(InstancePtr,Channels);

    RegValue = (XVoipFEC_RX_ReadReg(InstancePtr->Config.BaseAddress,
            XVOIPFEC_RX_FEC_CHAN_CONFIG_REG_OFFSET) &
                 ~(XVOIPFEC_RX_FEC_CHAN_CONFIG_MEDIA_PKT_BYPASS_MASK));

    XVoipFEC_RX_WriteReg(InstancePtr->Config.BaseAddress,
            XVOIPFEC_RX_FEC_CHAN_CONFIG_REG_OFFSET,
        (RegValue | (((InstancePtr->ChannelCfg[Channels].
                FECParams.Media_Packet_Bypass) <<
                XVOIPFEC_RX_FEC_CHAN_CONFIG_MEDIA_PKT_BYPASS_SHIFT) &
                     (XVOIPFEC_RX_FEC_CHAN_CONFIG_MEDIA_PKT_BYPASS_MASK))));

    XVoipFEC_RX_ChannelUpdate(InstancePtr);
}


/*****************************************************************************/
/**
*
* This function Enables or Disable the FEC Recovery Based on User Input to the
* core structure
*
* @param    InstancePtr is a pointer to the XVoipFEC_RX core instance.
*
* @param    Channels is current configured channel
*
* @return   None.
*
* @note     None.
*
******************************************************************************/
void XVoipFEC_RX_FECRecoveryDisable(XVoipFEC_RX *InstancePtr, u16 Channels)
{
    u32 RegValue;

    XVoipFEC_RX_ChannelAccess(InstancePtr,Channels);

    RegValue = (XVoipFEC_RX_ReadReg(InstancePtr->Config.BaseAddress,
            XVOIPFEC_RX_FEC_CHAN_CONFIG_REG_OFFSET) &
                ~(XVOIPFEC_RX_FEC_CHAN_CONFIG_FEC_RECV_DISABLE_MASK));

    XVoipFEC_RX_WriteReg(InstancePtr->Config.BaseAddress,
            XVOIPFEC_RX_FEC_CHAN_CONFIG_REG_OFFSET,
        (RegValue | (((InstancePtr->ChannelCfg[Channels].
                FECParams.FEC_Recovery_Disable) <<
                XVOIPFEC_RX_FEC_CHAN_CONFIG_FEC_RECV_DISABLE_SHIFT) &
                     (XVOIPFEC_RX_FEC_CHAN_CONFIG_FEC_RECV_DISABLE_MASK))));

    XVoipFEC_RX_ChannelUpdate(InstancePtr);
}

/*****************************************************************************/
/**
*
* This function configures the OOR Handling Management
*
* @param    InstancePtr is a pointer to the XVoipFEC_RX core instance.
*
* @param    Channels is current configured channel
*
* @return   None.
*
* @note     None.
*
******************************************************************************/
void XVoipFEC_RX_OORCfg(XVoipFEC_RX *InstancePtr, u16 Channels)
{
    u32 RegValue;

    XVoipFEC_RX_ChannelAccess(InstancePtr,Channels);

    /* Set the OOR Management Selection */
    RegValue = (XVoipFEC_RX_ReadReg(InstancePtr->Config.BaseAddress,
            XVOIPFEC_RX_OOR_MANAGEMENT_REG_OFFSET) &
                ~(XVOIPFEC_RX_OOR_MANAGEMENT_OOR_TS_SELECT_MASK));

    XVoipFEC_RX_WriteReg(InstancePtr->Config.BaseAddress,
            XVOIPFEC_RX_OOR_MANAGEMENT_REG_OFFSET,
        (RegValue | (((InstancePtr->ChannelCfg[Channels].
                OORMgmt.OORTimestampEnable) <<
                XVOIPFEC_RX_OOR_MANAGEMENT_OOR_TS_SELECT_SHIFT) &
                     (XVOIPFEC_RX_OOR_MANAGEMENT_OOR_TS_SELECT_MASK))));

    /* Set the OOR TS Window */
    RegValue = (XVoipFEC_RX_ReadReg(InstancePtr->Config.BaseAddress,
            XVOIPFEC_RX_OOR_MANAGEMENT_REG_OFFSET) &
                ~(XVOIPFEC_RX_OOR_MANAGEMENT_OOR_TS_SELECT_MASK));

    XVoipFEC_RX_WriteReg(InstancePtr->Config.BaseAddress,
            XVOIPFEC_RX_OOR_MANAGEMENT_REG_OFFSET,
        (RegValue | ((InstancePtr->ChannelCfg[Channels].
                OORMgmt.RTPTimestampWindow) &
                     (XVOIPFEC_RX_OOR_MANAGEMENT_OOR_TS_SELECT_MASK))));

    XVoipFEC_RX_ChannelUpdate(InstancePtr);
}