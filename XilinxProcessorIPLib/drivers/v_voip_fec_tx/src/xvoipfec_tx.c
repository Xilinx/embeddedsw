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
* @file xvoipfec_tx.c
*
* This is the main file for Xilinx VoIP FEC Transmitter core. Please see
* xvoipfec_tx.h for more details of the driver.
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

#include "xvoipfec_tx.h"
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
* This function initializes the VoIP FEC Transmitter core. This function must
* be called prior to using the VoIP FEC Transmitter core. Initialization of the
* VoIP FEC Transmitter includes setting up the instance data and ensuring the
* hardware is in a quiescent state.
*
* @param    InstancePtr is a pointer to the XVoipFEC_TX core instance.
* @param    CfgPtr points to the configuration structure associated with
*       the VoIP FEC Transmitter core.
* @param    EffectiveAddr is the base address of the device. If address
*       translation is being used, then this parameter must reflect the
*       virtual base address. Otherwise, the physical address should be
*       used.
*
* @return
*       - XST_SUCCESS if XVoipFEC_TX_CfgInitialize was successful.
*       - XST_FAILURE
*
* @note     None.
*
******************************************************************************/
int XVoipFEC_TX_CfgInitialize(XVoipFEC_TX *InstancePtr,
                                  XVoipFEC_TX_Config *CfgPtr,
                                                          UINTPTR EffectiveAddr)
{
    u16 Index;

    /* Verify arguments. */
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(CfgPtr != NULL);
    Xil_AssertNonvoid(EffectiveAddr != (u32)0x0);

    /* Setup the instance */
    (void)memset((void *)InstancePtr, 0, sizeof(XVoipFEC_TX));
    (void)memcpy((void *)&(InstancePtr->Config), (const void *)CfgPtr,
            sizeof(XVoipFEC_TX_Config));
    InstancePtr->Config.BaseAddress = EffectiveAddr;

    XVoipFEC_TX_SoftReset(InstancePtr);

    /* Check The Busy Bit - Wait the Busy Bit = 0 */
    Index = 0x00;
    while(XVoipFEC_TX_BusyBit(InstancePtr)){
        if (Index == 65535){
            xdbg_printf(XDBG_DEBUG_GENERAL,"Error: VoIP FEC TX Busy Bit Longer \
            than Expected\n\r");
            break;
        }
        Index ++;
    }

    /* Setting Parameters based on Drivers */
    for (Index=(u16)0x00; Index<XVOIPFEC_TX_MAX_CHANNEL; Index++) {
        InstancePtr->ChannelCfg[Index].fec_params.non_block_allign =
                   BLOCK_ALLIGN;
        InstancePtr->ChannelCfg[Index].fec_params.fec_mode =
                   FEC_BYPASS;
        XVoipFEC_TX_SetFECParams(InstancePtr,Index);
    }

    /* Reset the hardware and set the flag to indicate the driver is ready */
    InstancePtr->IsReady = (u32)(XIL_COMPONENT_IS_READY);

    return (XST_SUCCESS);
}

/*****************************************************************************/
/**
*
* This function reads the Core Information (Configured by user through GUI),
* in the VoIP FEC Transmitter Register
*
* @param    InstancePtr is a pointer to the XVoipFEC_TX core instance.
*
* @return   XVoipFEC_TX_ConfigRegValue is Core Status Register Value.
*
* @note     None.
*
******************************************************************************/
XVoipFEC_TX_Config XVoipFEC_TX_CoreStatusRegValue(XVoipFEC_TX *InstancePtr)
{
    XVoipFEC_TX_Config XVoipFEC_TX_ConfigRegValue;

    /* Get Maximum Supported Channel Info */
    XVoipFEC_TX_ConfigRegValue.HWChannelNumber =
            (XVoipFEC_TX_ReadReg(InstancePtr->Config.BaseAddress,
                      XVOIPFEC_TX_SYSTEM_CONFIG_REG_OFFSET) &
                               XVOIPFEC_TX_SYS_CONFIG_C_CHANNELS_MASK);

    /* Get Maximum Supported FEC L */
    XVoipFEC_TX_ConfigRegValue.MaximumFECL =
         ((XVoipFEC_TX_ReadReg(InstancePtr->Config.BaseAddress,
                XVOIPFEC_TX_SYSTEM_CONFIG_REG_OFFSET) &
                     XVOIPFEC_TX_SYS_CONFIG_SUPP_MAX_L_MASK) >>
                          (XVOIPFEC_TX_SYS_CONFIG_SUPP_MAX_L_SHIFT));

    return XVoipFEC_TX_ConfigRegValue;
}

/*****************************************************************************/
/**
*
* This function perform Statistic Resets on VoIP FEC Transmitter (General Space)
*
* @param    InstancePtr is a pointer to the XVoipFEC_TX core instance.
*
* @return   XVoipFEC_TX_PktCntRegValue is Return Register Value of General
*           Statistic VoIP FEC Transmitter.
*
* @note     None.
*
******************************************************************************/
XVoipFEC_TX_PktCnt XVoipFEC_TX_CoreStatisics(XVoipFEC_TX *InstancePtr)
{

    XVoipFEC_TX_PktCnt XVoipFEC_TX_PktCntRegValue;

    /* Get Incoming Packet Count */
    XVoipFEC_TX_PktCntRegValue.incoming_packet_count =
         (XVoipFEC_TX_ReadReg(InstancePtr->Config.BaseAddress,
               XVOIPFEC_TX_IN_PACKET_COUNT_REG_OFFSET));

    /* Get Outgoing Packet Count */
    XVoipFEC_TX_PktCntRegValue.outgoing_packet_count =
         (XVoipFEC_TX_ReadReg(InstancePtr->Config.BaseAddress,
               XVOIPFEC_TX_OUT_PACKET_COUNT_REG_OFFSET));

    return XVoipFEC_TX_PktCntRegValue;
}

/*****************************************************************************/
/**
*
* This function perform Reads Configured FEC Parameters
*
* @param    InstancePtr is a pointer to the XVoipFEC_TX core instance.
*
* @param    Channels is current configured channel
*
* @return   XVoipFEC_TX_FECParams_RegValue is configured FEC Parameters.
*
* @note     None.
*
******************************************************************************/
XVoipFEC_TX_FECParams XVoipFEC_TX_FECParamsRegValue(XVoipFEC_TX *InstancePtr,
                                                         u16 Channels)
{
    XVoipFEC_TX_FECParams XVoipFEC_TX_FECParams_RegValue;

    XVoipFEC_ChannelAccess(InstancePtr, Channels);

    /* FEC Non-Block Alligned */
    XVoipFEC_TX_FECParams_RegValue.non_block_allign =
         (XVoipFEC_TX_ReadReg(InstancePtr->Config.BaseAddress,
            XVOIPFEC_TX_FEC_CONFIG_REG_OFFSET) &
                 (XVOIPFEC_TX_FEC_CONFIG_NON_BLOCK_ALIGN_MASK)) >>
                       XVOIPFEC_TX_FEC_CONFIG_NON_BLOCK_ALIGN_SHIFT;


    /* FEC Mode */
    XVoipFEC_TX_FECParams_RegValue.fec_mode =
         (XVoipFEC_TX_ReadReg(InstancePtr->Config.BaseAddress,
            XVOIPFEC_TX_FEC_CONFIG_REG_OFFSET) &
                 (XVOIPFEC_TX_FEC_CONFIG_FEC_MODE_MASK)) >>
                       XVOIPFEC_TX_FEC_CONFIG_FEC_MODE_SHIFT;

    /* FEC L */
    XVoipFEC_TX_FECParams_RegValue.fec_l =
         (XVoipFEC_TX_ReadReg(InstancePtr->Config.BaseAddress,
            XVOIPFEC_TX_FEC_CONFIG_REG_OFFSET) &
                 (XVOIPFEC_TX_FEC_CONFIG_FEC_L_MASK)) >>
                       XVOIPFEC_TX_FEC_CONFIG_FEC_L_SHIFT;

    /* FEC D */
    XVoipFEC_TX_FECParams_RegValue.fec_d =
         (XVoipFEC_TX_ReadReg(InstancePtr->Config.BaseAddress,
               XVOIPFEC_TX_FEC_CONFIG_REG_OFFSET) &
                    (XVOIPFEC_TX_FEC_CONFIG_FEC_D_MASK));

    return (XVoipFEC_TX_FECParams_RegValue);
}

/*****************************************************************************/
/**
*
* This function Configure all the VoIP FEC Transmitter Channel Space based
* on user configured value.
*
* @param    InstancePtr is a pointer to the XVoipFEC_TX core instance.
*
* @return
*
* @note     None.
*
******************************************************************************/
void XVoipFEC_TX_CoreChannelConfig(XVoipFEC_TX *InstancePtr)
{
    u16 Index;

    /* Setting Parameters based on Drivers */
    for (Index=(u16)0x00; Index < XVOIPFEC_TX_MAX_CHANNEL; Index++) {

        /* Select Channels */
        XVoipFEC_ChannelAccess(InstancePtr,Index);

        if (InstancePtr->ChannelCfg[Index].isEnable == 1) {
            /* Set FEC Parameter */
            XVoipFEC_TX_SetFECParams(InstancePtr, Index);

            /* Update the Channels */
            XVoipFEC_TX_ChannelUpdate(InstancePtr);
        }
    }

}

/*****************************************************************************/
/**
*
* This function Configures the FEC Parameters
*
* @param    InstancePtr is a pointer to the XVoipFEC_TX core instance.
*
* @param    Channels is current configured channel
*
* @return
*
* @note     None.
*
******************************************************************************/
void XVoipFEC_TX_SetFECParams(XVoipFEC_TX *InstancePtr, u16 Channels)
{

    /* Select Channels */
    XVoipFEC_ChannelAccess(InstancePtr,Channels);

    /* Set FEC D */
    XVoipFEC_TX_SetFEC_D(InstancePtr,Channels);

    /* Set FEC L */
    XVoipFEC_TX_SetFEC_L(InstancePtr,Channels);

    /* Set FEC Block Aligned */
    XVoipFEC_TX_SetFECMode(InstancePtr,Channels);

    /* Set FEC Mode */
    XVoipFEC_TX_SetFECNonBlockAllign(InstancePtr,Channels);

    /* Perform Channel Update */
    XVoipFEC_TX_ChannelUpdate(InstancePtr);

}

/*****************************************************************************/
/**
*
* This function perform software resets on VoIP FEC Transmitter, which clears
* all the registers.
*
* @param    InstancePtr is a pointer to the XVoipFEC_TX core instance.
*
* @return   None.
*
* @note     None.
*
******************************************************************************/
void XVoipFEC_TX_SoftReset(XVoipFEC_TX *InstancePtr)
{
    u32 RegValue;

    RegValue = XVoipFEC_TX_ReadReg((InstancePtr)->Config.BaseAddress,
            (XVOIPFEC_TX_CONTROL_REG_OFFSET)) &
                  ((~(XVOIPFEC_TX_CONTROL_SOFT_RESET_MASK)));

    /* Set the Channel Update */
    XVoipFEC_TX_WriteReg((InstancePtr)->Config.BaseAddress,
         (XVOIPFEC_TX_CONTROL_REG_OFFSET),(RegValue |
               XVOIPFEC_TX_CONTROL_SOFT_RESET_MASK));

    RegValue = XVoipFEC_TX_ReadReg((InstancePtr)->Config.BaseAddress,
            (XVOIPFEC_TX_CONTROL_REG_OFFSET)) &
                  ((~(XVOIPFEC_TX_CONTROL_SOFT_RESET_MASK)));

    /* Clear the Channel Update */
    XVoipFEC_TX_WriteReg((InstancePtr)->Config.BaseAddress,
         (XVOIPFEC_TX_CONTROL_REG_OFFSET),(RegValue &
               (~(XVOIPFEC_TX_CONTROL_SOFT_RESET_MASK))));
}


/*****************************************************************************/
/**
*
* This function perform Channel on VoIP FEC Transmitter, which updates
* the configured channel space register.
*
* @param    InstancePtr is a pointer to the XVoipFEC_TX core instance.
*
* @return   None.
*
* @note     None.
*
******************************************************************************/
void XVoipFEC_TX_ChannelUpdate(XVoipFEC_TX *InstancePtr)
{
    u32 RegValue;

    RegValue = XVoipFEC_TX_ReadReg((InstancePtr)->Config.BaseAddress,
            (XVOIPFEC_TX_CONTROL_REG_OFFSET)) &
                  ((~(XVOIPFEC_TX_CONTROL_CHANNEL_UPDATE_MASK)));

    /* Set the Channel Update */
    XVoipFEC_TX_WriteReg((InstancePtr)->Config.BaseAddress,
         (XVOIPFEC_TX_CONTROL_REG_OFFSET),(RegValue |
               XVOIPFEC_TX_CONTROL_CHANNEL_UPDATE_MASK));

    RegValue = XVoipFEC_TX_ReadReg((InstancePtr)->Config.BaseAddress,
            (XVOIPFEC_TX_CONTROL_REG_OFFSET)) &
                  ((~(XVOIPFEC_TX_CONTROL_CHANNEL_UPDATE_MASK)));

    /* Clear the Channel Update */
    XVoipFEC_TX_WriteReg((InstancePtr)->Config.BaseAddress,
         (XVOIPFEC_TX_CONTROL_REG_OFFSET),(RegValue &
               (~(XVOIPFEC_TX_CONTROL_CHANNEL_UPDATE_MASK))));
}


/*****************************************************************************/
/**
*
* This function accesses the Channel, need to be called before performing
* channel space register configuration
*
* @param    InstancePtr is a pointer to the XVoipFEC_TX core instance.
*
* @param    Channels is current configured channel
*
* @return   None.
*
* @note     None.
*
******************************************************************************/
void XVoipFEC_ChannelAccess(XVoipFEC_TX *InstancePtr, u16 Channels)
{
    /* Set the Channel Update */
    XVoipFEC_TX_WriteReg((InstancePtr)->Config.BaseAddress,
         (XVOIPFEC_TX_CHANNEL_ACCESS_REG_OFFSET),
               (Channels & XVOIPFEC_TX_CHANNEL_ACCESS_MASK));
}


/*****************************************************************************/
/**
*
* This function sets FEC D Value
*
* @param    InstancePtr is a pointer to the XVoipFEC_TX core instance.
*
* @param    Channels is current configured channel
*
* @return   None.
*
* @note     None.
*
******************************************************************************/
void XVoipFEC_TX_SetFEC_D(XVoipFEC_TX *InstancePtr, u16 Channels)
{
    u32 RegValue;

    XVoipFEC_ChannelAccess(InstancePtr,Channels);

    RegValue = (XVoipFEC_TX_ReadReg(InstancePtr->Config.BaseAddress,
            XVOIPFEC_TX_FEC_CONFIG_REG_OFFSET) &
                 (~(XVOIPFEC_TX_FEC_CONFIG_FEC_D_MASK)));

    /* Write the value to the register */
    XVoipFEC_TX_WriteReg(InstancePtr->Config.BaseAddress,
         XVOIPFEC_TX_FEC_CONFIG_REG_OFFSET,
              (RegValue |
                  ((InstancePtr->ChannelCfg[Channels].fec_params.fec_d) &
                               XVOIPFEC_TX_FEC_CONFIG_FEC_D_MASK)));

    /* Update the Channels */
    XVoipFEC_TX_ChannelUpdate(InstancePtr);
}

/*****************************************************************************/
/**
*
* This function sets FEC L Value
*
* @param    InstancePtr is a pointer to the XVoipFEC_TX core instance.
*
* @param    Channels is current configured channel
*
* @return   None.
*
* @note     None.
*
******************************************************************************/
void XVoipFEC_TX_SetFEC_L(XVoipFEC_TX *InstancePtr, u16 Channels)
{
    u32 RegValue;

    XVoipFEC_ChannelAccess(InstancePtr,Channels);

    RegValue = (XVoipFEC_TX_ReadReg(InstancePtr->Config.BaseAddress,
            XVOIPFEC_TX_FEC_CONFIG_REG_OFFSET) &
                 ~(XVOIPFEC_TX_FEC_CONFIG_FEC_L_MASK));

    /* Write the value to the register */
    XVoipFEC_TX_WriteReg(InstancePtr->Config.BaseAddress,
         XVOIPFEC_TX_FEC_CONFIG_REG_OFFSET,
              (RegValue |
                  (((InstancePtr->ChannelCfg[Channels].fec_params.fec_l) <<
                                XVOIPFEC_TX_FEC_CONFIG_FEC_L_SHIFT) &
                                     (XVOIPFEC_TX_FEC_CONFIG_FEC_L_MASK))));


    /* Update the Channels */
    XVoipFEC_TX_ChannelUpdate(InstancePtr);

}

/*****************************************************************************/
/**
*
* This function sets FEC Mode Value. FEC Bypass, 1D or 2D Mode
*
* @param    InstancePtr is a pointer to the XVoipFEC_TX core instance.
*
* @param    Channels is current configured channel
*
* @return   None.
*
* @note     None.
*
******************************************************************************/
void XVoipFEC_TX_SetFECMode(XVoipFEC_TX *InstancePtr, u16 Channels)
{
    u32 RegValue;

    XVoipFEC_ChannelAccess(InstancePtr,Channels);

    RegValue = (XVoipFEC_TX_ReadReg(InstancePtr->Config.BaseAddress,
            XVOIPFEC_TX_FEC_CONFIG_REG_OFFSET) &
                 ~(XVOIPFEC_TX_FEC_CONFIG_FEC_MODE_MASK));

    /* Write the value to the register */
    XVoipFEC_TX_WriteReg(InstancePtr->Config.BaseAddress,
         XVOIPFEC_TX_FEC_CONFIG_REG_OFFSET,
              (RegValue |
                  (((InstancePtr->ChannelCfg[Channels].fec_params.fec_mode) <<
                             XVOIPFEC_TX_FEC_CONFIG_FEC_MODE_SHIFT) &
                                  (XVOIPFEC_TX_FEC_CONFIG_FEC_MODE_MASK))));

    /* Update the Channels */
    XVoipFEC_TX_ChannelUpdate(InstancePtr);
}

/*****************************************************************************/
/**
*
* This function sets FEC Block Alignment Mode.
*
* @param    InstancePtr is a pointer to the XVoipFEC_TX core instance.
*
* @param    Channels is current configured channel
*
* @return   None.
*
* @note     None.
*
******************************************************************************/
void XVoipFEC_TX_SetFECNonBlockAllign(XVoipFEC_TX *InstancePtr, u16 Channels)
{
    u32 RegValue;

    XVoipFEC_ChannelAccess(InstancePtr,Channels);

    RegValue = (XVoipFEC_TX_ReadReg(InstancePtr->Config.BaseAddress,
            XVOIPFEC_TX_FEC_CONFIG_REG_OFFSET) &
                 ~(XVOIPFEC_TX_FEC_CONFIG_NON_BLOCK_ALIGN_MASK));

    /* Write the value to the register */
    XVoipFEC_TX_WriteReg(InstancePtr->Config.BaseAddress,
      XVOIPFEC_TX_FEC_CONFIG_REG_OFFSET,
        (RegValue |
          (((InstancePtr->ChannelCfg[Channels].fec_params.non_block_allign) <<
                     XVOIPFEC_TX_FEC_CONFIG_NON_BLOCK_ALIGN_SHIFT) &
                          (XVOIPFEC_TX_FEC_CONFIG_NON_BLOCK_ALIGN_MASK))));

    /* Update the Channels */
    XVoipFEC_TX_ChannelUpdate(InstancePtr);

}
