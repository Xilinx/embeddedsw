/******************************************************************************
*
* Copyright (C) 2008 - 2018 Xilinx, Inc.  All rights reserved.
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
* @file xavb_hw.h
*
* This header file contains the identifiers and basic driver functions (or
* macros) that can be used to access the device. Other driver functions
* are defined in xavb.h.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -----------------------------------------------
* 1.00a mbr  09/19/08 First release
* 1.01a mbr  06/24/09 PTP frame format updates for IEEE802.1 AS draft 5-0
* 2_02a mbr  09/16/09 Updates for programmable PTP timers
* 2_04a kag  07/23/10 PTP frame format updates for IEEE802.1 AS draft 6-7
* 3_01a kag  08/29/11 Added new APIs to update the RX Filter Control Reg.
*		      Fix for CR:572539. Updated bit map for Rx Filter
*		      control reg.
* 3_01a asa  04/10/12 The AVB core is now brought inside the AxiEthernet
*		      core. Because of this there are changes in the
*		      register map.
*
* </pre>
*
******************************************************************************/

#ifndef XAVB_HW_H    /* prevent circular inclusions */
#define XAVB_HW_H    /* by using protection macros */

#ifdef __cplusplus
extern "C" {
#endif


/***************************** Include Files *********************************/

#include "xil_types.h"
#include "xil_io.h"

/************************** Constant Definitions *****************************/

/** @name Register offsets for the Tri-Mode Ethernet MAC. Each register is 32
 *        bits.  The MAC is addressable through the Ethernet AVB Endpoint core.
 *  @{
 */
#define XAVB_MAC_RX_REG0_OFFSET  0x00000400  /**< MAC Rx Config Register 0    */
#define XAVB_MAC_RX_REG1_OFFSET  0x00000404  /**< MAC Rx Config Register 1    */
#define XAVB_MAC_TX_REG_OFFSET   0x00000408  /**< MAC Tx Config Register      */
#define XAVB_MAC_FC_REG_OFFSET   0x0000040c  /**< MAC Flow Control Register   */
#define XAVB_MAC_SPD_REG_OFFSET  0x00000410  /**< MAC Speed Control Register  */
//#define XAVB_MAC_MGMT_REG_OFFSET 0x0000050C  /**< MAC MDIO Management Register*/
/* @} */

/** @name Register offsets for the Ethernet Audio Video Endpoint. Each register
 *        is 32 bits.
 *  @{
 */
#define XAVB_PTP_TX_CONTROL_OFFSET      0x00012000 /**< Tx PTP Control Reg    */
#define XAVB_PTP_RX_CONTROL_OFFSET      0x00012004 /**< Rx PTP Control Reg    */
#define XAVB_RX_FILTER_CONTROL          0x00012008 /**< Rx Filter Control Reg */
#define XAVB_TX_SENDSLOPE               0x0001200C /**< Tx rate sendSlope Reg */
#define XAVB_TX_IDLESLOPE               0x00012010 /**< Tx rate idleSlope Reg */
#define XAVB_TX_HILIMIT                 0x00012014 /**< Tx rate hiLimit Reg   */
#define XAVB_TX_LOLIMIT                 0x00012018 /**< Tx rate loLimit Reg   */
#define XAVB_RTC_NANOSEC_OFFSET         0x00012800 /**< RTC ns offset Reg     */
#define XAVB_RTC_SEC_LOWER_OFFSET       0x00012808 /**< RTC sec[31:0] offset  */
#define XAVB_RTC_SEC_UPPER_OFFSET       0x0001280C /**< RTC sec[47:32] offset */
#define XAVB_RTC_INCREMENT_OFFSET       0x00012810 /**< RTC Increment Reg     */
#define XAVB_RTC_NANOSEC_VALUE_OFFSET   0x00012814 /**< RTC ns value Reg      */
#define XAVB_RTC_SEC_LOWER_VALUE_OFFSET 0x00012818 /**< RTC sec[31:0] value   */
#define XAVB_RTC_SEC_UPPER_VALUE_OFFSET 0x0001281C /**< RTC sec[47:32] value  */
#define XAVB_RTC_CLEAR_INT_OFFSET       0x00012820 /**< RTC Interrupt Clear   */
#define XAVB_RTC_8K_OFFSET_OFFSET       0x00012824 /**< RTC 8k phase offset   */
#define XAVB_SW_RESET_OFFSET            0x00012828 /**< S/W Reset Reg         */
/* @} */

/** @name Packet base address offsets for the Ethernet Audio Video Endpoint Tx
 *        Precise Timing Protocol (PTP) frame buffer.  Each PTP frames is
 *        stored in 256-byte chunks of BRAM.  This BRAM can store 8 PTP frames
 *        of which only 6 are currently in use.
 *  @{
 */
#define XAVB_PTP_TX_SYNC_OFFSET                 0x00011000
#define XAVB_PTP_TX_FOLLOW_UP_OFFSET            0x00011100
#define XAVB_PTP_TX_PDELAYREQ_OFFSET            0x00011200
#define XAVB_PTP_TX_PDELAYRESP_OFFSET           0x00011300
#define XAVB_PTP_TX_PDELAYRESP_FOLLOW_UP_OFFSET 0x00011400
#define XAVB_PTP_TX_ANNOUNCE_OFFSET             0x00011500
/* @} */

/** @name Base address offset for the Ethernet Audio Video Endpoint Rx
 *        Precise Timing Protocol (PTP) frame buffer.  These PTP frames are
 *        stored in 256-byte chunks of BRAM.  This BRAM can store 16 PTP frames.
 *  @{
 */
#define XAVB_PTP_RX_BASE_OFFSET                 0x00010000
/* @} */

/** @name AVB Tx PTP Control Register
 *  @{
 */
#define XAVB_PTP_TX_SEND_SYNC_FRAME_MASK                     0x00000001
#define XAVB_PTP_TX_SEND_FOLLOWUP_FRAME_MASK                 0x00000002
#define XAVB_PTP_TX_SEND_PDELAYREQ_FRAME_MASK                0x00000004
#define XAVB_PTP_TX_SEND_PDELAYRESP_FRAME_MASK               0x00000008
#define XAVB_PTP_TX_SEND_PDELAYRESPFOLLOWUP_FRAME_MASK       0x00000010
#define XAVB_PTP_TX_SEND_ANNOUNCE_FRAME_MASK                 0x00000020
#define XAVB_PTP_TX_SEND_FRAME6_BIT_MASK                     0x00000040
#define XAVB_PTP_TX_SEND_FRAME7_BIT_MASK                     0x00000080
#define XAVB_PTP_TX_WAIT_SYNC_FRAME_MASK                     0x00000100
#define XAVB_PTP_TX_WAIT_FOLLOWUP_FRAME_MASK                 0x00000200
#define XAVB_PTP_TX_WAIT_PDELAYREQ_FRAME_MASK                0x00000400
#define XAVB_PTP_TX_WAIT_PDELAYRESP_FRAME_MASK               0x00000800
#define XAVB_PTP_TX_WAIT_PDELAYRESPFOLLOWUP_FRAME_MASK       0x00001000
#define XAVB_PTP_TX_WAIT_ANNOUNCE_FRAME_MASK                 0x00002000
#define XAVB_PTP_TX_WAIT_FRAME6_BIT_MASK                     0x00004000
#define XAVB_PTP_TX_WAIT_FRAME7_BIT_MASK                     0x00008000
#define XAVB_PTP_TX_WAIT_ALL_FRAMES_MASK                     0x0000FF00
#define XAVB_PTP_TX_PACKET_FIELD_MASK                        0x00070000

/* @} */

/** @name AVB Rx PTP Control Register
 *  @{
 */
#define XAVB_PTP_RX_CLEAR_BIT_MASK                           0x00000001
#define XAVB_PTP_RX_PACKET_FIELD_MASK                        0x00000F00
/* @} */

/** @name AVB Rx Filter Control Register
 *  @{
 */
#define XAVB_RX_AV_VLAN_PRIORITY_A_MASK                      0x00000007
#define XAVB_RX_AV_VLAN_VID_A_MASK                           0x00007FF8
#define XAVB_RX_AV_VLAN_MATCH_MODE_MASK                      0x00008000
#define XAVB_RX_AV_VLAN_PRIORITY_B_MASK                      0x00070000
#define XAVB_RX_AV_VLAN_VID_B_MASK                           0x7FF80000
#define XAVB_RX_LEGACY_PROMISCUOUS_MODE_MASK                 0x80000000
/* @} */

/** @name AVB Tx rate control sendSlope
 *  @{
 */
#define XAVB_TX_SENDSLOPE_MASK                               0X000FFFFF
/* @} */

/** @name AVB Tx rate control idleSlope
 *  @{
 */
#define XAVB_TX_IDLESLOPE_MASK                               0X000FFFFF
/* @} */

/** @name AVB Tx rate control hiLimit
 *  @{
 */
#define XAVB_TX_HILIMIT_MASK                                 0X01FFFFFF
/* @} */

/** @name AVB Tx rate control loLimit
 *  @{
 */
#define XAVB_TX_LOLIMIT_MASK                                 0X01FFFFFF
/* @} */

/** @name RTC field Registers
 *  @{
 */
#define XAVB_RTC_NS_MASK                                     0x3FFFFFFF
#define XAVB_RTC_SEC_LOWER_MASK                              0xFFFFFFFF
#define XAVB_RTC_SEC_UPPER_MASK                              0x0000FFFF
/* @} */

/** @name RTC Increment Register
 *  @{
 */
#define XAVB_RTC_INCREMENT_VALUE_MASK                        0x03FFFFFF
/** This value assumes a 125MHz rtc_clock */
#define XAVB_RTC_INCREMENT_NOMINAL_RATE                      0x00800000
/**  Add some rails so that recovery is possible after a
 *  string of bad pDelay values.  The RTC should be able to lock
 *  to within 100ppm of the slowest allowable clock (25 MHz).
 *  This equates to +/-4ps.  Let's arbitrarily set the rails to
 *  400ppm (+/-16ps) just in case someone decides to use a
 *  particularly bad oscillator.  The lowest 20 bits of
 *  NewIncrement are fractions of a nanosecond, which equates
 *  to +/- 0x04189 */
#define XAVB_RTC_400PPM_OFFSET                               0x00004189
/* @} */

/** @name RTC Interrupt Clear Register
 *  @{
 */
#define XAVB_RTC_INCREMENT_VALUE_MASK                        0x03FFFFFF
/* @} */


/** @name RTC Interrupt Clear Register
 *  @{
 */
#define XAVB_RTC_CLEAR_INT_MASK                              0x00000000
/* @} */


/** @name RTC 8k phase offset Register
 *  @{
 */
#define XAVB_RTC_8K_PHASE_OFFSET_MASK                        0x3FFFFFFF
/* @} */

/** @name S/W Reset  Register
 *  @{
 */
#define XAVB_SW_RESET_TX_AND_RX_PATHS                        0x00000003
/* @} */


/** @name AVB MAC MDIO register address space.
 *  @{
 */
#define XAVB_MAC_MDIO_BASE_OFFSET                            0x00006000
/* @} */


/** @name MDIO valid data mask (MDIO registers are all 16-bits).
 *  @{
 */
#define XAVB_MAC_MDIO_DATA_MASK                              0x0000FFFF
/* @} */


/** @name MAC Statistic Counter names and CounterID.
 *  @{
 */
#define XAVB_STATS_BYTES_TRANSMITTED                         0x00000000
#define XAVB_STATS_BYTES_RECEIVED                            0x00000001
#define XAVB_STATS_UNDERSIZED_FRAMES_RECEIVED                0x00000002
#define XAVB_STATS_FRAGMENT_FRAMES_RECEIVED                  0x00000003
#define XAVB_STATS_64_BYTE_FRAMES_RECEIVED_OK                0x00000004
#define XAVB_STATS_65_TO_127_BYTE_FRAMES_RECEIVED_OK         0x00000005
#define XAVB_STATS_128_TO_255_BYTE_FRAMES_RECEIVED_OK        0x00000006
#define XAVB_STATS_256_TO_511_BYTE_FRAMES_RECEIVED_OK        0x00000007
#define XAVB_STATS_512_TO_1023_BYTE_FRAMES_RECEIVED_OK       0x00000008
#define XAVB_STATS_1024_TO_1518_BYTE_FRAMES_RECEIVED_OK      0x00000009
#define XAVB_STATS_OVERSIZED_FRAMES_RECEIVED_OK              0x0000000A
#define XAVB_STATS_64_BYTE_FRAMES_TRANSMITTED_OK             0x0000000B
#define XAVB_STATS_65_TO_127_BYTE_FRAMES_TRANSMITTED_OK      0x0000000C
#define XAVB_STATS_128_TO_255_BYTE_FRAMES_TRANSMITTED_OK     0x0000000D
#define XAVB_STATS_256_TO_511_BYTE_FRAMES_TRANSMITTED_OK     0x0000000E
#define XAVB_STATS_512_TO_1023_BYTE_FRAMES_TRANSMITTED_OK    0x0000000F
#define XAVB_STATS_1024_TO_1518_BYTE_FRAMES_TRANSMITTED_OK   0x00000010
#define XAVB_STATS_OVERSIZE_FRAMES_TRANSMITTED_OK            0x00000011
#define XAVB_STATS_FRAMES_RECEIVED_OK                        0x00000012
#define XAVB_STATS_FCS_ERRORS                                0x00000013
#define XAVB_STATS_BROADCAST_FRAMES_RECEIVED_OK              0x00000014
#define XAVB_STATS_MULTICAST_FRAMES_RECEIVED_OK              0x00000015
#define XAVB_STATS_CONTROL_FRAMES_RECEIVED_OK                0x00000016
#define XAVB_STATS_LENGTH_TYPE_OUT_OF_RANGE                  0x00000017
#define XAVB_STATS_VLAN_TAGGED_FRAMES_RECEIVED_OK            0x00000018
#define XAVB_STATS_PAUSE_FRAMES_RECEIVED_OK                  0x00000019
#define XAVB_STATS_CONTROL_FRAMES_WITH_UNSUPPORTED_OPCODE    0x0000001A
#define XAVB_STATS_FRAMES_TRANSMITTED                        0x0000001B
#define XAVB_STATS_BROADCAST_FRAMES_TRANSMITTED              0x0000001C
#define XAVB_STATS_MULTICAST_FRAMES_TRANSMITTED              0x0000001D
#define XAVB_STATS_UNDERRUN_ERRORS                           0x0000001E
#define XAVB_STATS_CONTROL_FRAMES_TRANSMITTED_OK             0x0000001F
#define XAVB_STATS_VLAN_TAGGED_FRAMES_TRANSMITTED_OK         0x00000020
#define XAVB_STATS_PAUSE_FRAMES_TRANSMITTED_OK               0x00000021
#define XAVB_STATS_SINGLE_COLLISION_FRAMES                   0x00000022
#define XAVB_STATS_MULTI_COLLISION_FRAMES                    0x00000023
#define XAVB_STATS_DEFERRAL_FRAMES                           0x00000024
#define XAVB_STATS_LATE_COLLISION_FRAMES                     0x00000025
#define XAVB_STATS_EXCESS_COLLISION_FRAMES                   0x00000026
#define XAVB_STATS_EXCESS_DEFERRAL_FRAMES                    0x00000027
#define XAVB_STATS_CARRIER_SENSE_ERRORS                      0x00000028
#define XAVB_STATS_ALIGNMENT_ERRORS                          0x00000029
/* @} */


/**************************** Type Definitions *******************************/

/**
 * This typedef defines the format for the Real Time Clock (RTC).  The RTC
 * contains a 48-bit seconds field (split into upper and lower sections) and
 * a 32-bit nano-seconds field.
 */
typedef struct
{
  u16 SecondsUpper;  /**< Upper 16-bits of seconds field */
  u32 SecondsLower;  /**< Lower 32-bits of seconds field */
  u32 NanoSeconds;   /**< 32-bit nanoseconds field */
} XAvb_RtcFormat;

/**
 * This typedef describes a 64-bit un-signed integer in terms of 2 u32s
 */
typedef struct
{
  u32 Upper;        /**< Upper 32 bits */
  u32 Lower;        /**< Lower 32 bits */
} XAvb_Uint64;

/***************** Macros (Inline Functions) Definitions *********************/

/****************************************************************************/
/**
*
* This macro reads from the given AVB core register.
*
* @param  BaseAddress is the base address of the device
* @param  RegOffset is the register offset to be read
*
* @return The 32-bit value of the register
*
* @note   None.
*
*****************************************************************************/
#define XAvb_ReadReg(BaseAddress, RegOffset) \
      Xil_In32((BaseAddress) + (RegOffset))


/****************************************************************************/
/**
*
* This macro writes to the given AVB core register.
*
* @param  BaseAddress is the base address of the device
* @param  RegOffset is the register offset to be written
* @param  Data is the 32-bit value to write to the register
*
* @return None.
*
* @note   None.
*
*****************************************************************************/
#define XAvb_WriteReg(BaseAddress, RegOffset, Data) \
      Xil_Out32((BaseAddress) + (RegOffset), (Data))


/****************************************************************************/
/**
*
* This macro reads from the given PTP frame buffer.
*
* @param  BaseAddress is the base address of the device
* @param  PtpPacketBaseAddress is the base address of the frame in the PTP
*         BRAM packet buffer
  @param  PtpPacketOffset is the offset address within the PTP frame
*
* @return The 32-bit value of the register
*
* @note   None.
*
*****************************************************************************/
#define XAvb_ReadPtpBuffer(BaseAddress, PtpPacketBaseAddress, PtpPacketOffset)\
      Xil_In32(BaseAddress + PtpPacketBaseAddress + PtpPacketOffset)


/****************************************************************************/
/**
*
* This macro writes to the given PTP frame buffer.
*
* @param  BaseAddress is the base address of the device
* @param  PtpPacketBaseAddress is the base address of the frame in the PTP
*         packet buffer
  @param  PtpPacketOffset is the offset address within the PTP frame
* @param  Data is the 32-bit value to write to the register
*
* @return None.
*
* @note   None.
*
*****************************************************************************/
#define XAvb_WritePtpBuffer(BaseAddress, PtpPacketBaseAddress, \
                             PtpPacketOffset, Data) \
      Xil_Out32(BaseAddress + PtpPacketBaseAddress + PtpPacketOffset, (Data))


/****************************************************************************/
/**
*
* This macro reads from the given TEMAC Configuration Register.
*
* @param  BaseAddress is the base address of the device
* @param  RegOffset is the register offset to be read
*
* @return The 32-bit value of the register
*
* @note   None.
*
*****************************************************************************/
#define XAvbMac_ReadConfig(BaseAddress, RegOffset) \
      Xil_In32((BaseAddress) + (RegOffset))


/****************************************************************************/
/**
*
* This macro writes to the given TEMAC Configuration Register.
*
* @param  BaseAddress is the base address of the device
* @param  RegOffset is the register offset to be written
* @param  Data is the 32-bit value to write to the register
*
* @return None.
*
* @note   None.
*
*****************************************************************************/
#define XAvbMac_WriteConfig(BaseAddress, RegOffset, Data) \
      Xil_Out32((BaseAddress) + (RegOffset), (Data))


/****************************************************************************/
/**
*
* This macro reads from the given MDIO Register using the TEMAC
*
* @param  BaseAddress is the base address of the device
* @param  Phyad is the Physical Address of the PHY
* @param  Regad is the Address of the MDIO register within the addressed PHY
*
* @return The 32-bit value of the register (upper 16-bits will be all 0's)
*
* @note   None.
*
*****************************************************************************/
#define XAvbMac_ReadMdio(BaseAddress, Phyad, Regad) \
  Xil_In32((BaseAddress) + XAVB_MAC_MDIO_BASE_OFFSET         \
                                  + (((Phyad) & 0x1F) << 8)  \
                                  + (((Regad) & 0x1F) << 3))


/****************************************************************************/
/**
*
* This macro writes to the given MDIO Register using the TEMAC
*
* @param  BaseAddress is the base address of the device
* @param  Phyad is the Physical Address of the PHY
* @param  Regad is the Address of the MDIO register within the addressed PHY
* @param  Data is the 32-bit value to write
*
* @return None.
*
* @note   None.
*
*****************************************************************************/
#define XAvbMac_WriteMdio(BaseAddress, Phyad, Regad, Data) \
  Xil_Out32(BaseAddress + XAVB_MAC_MDIO_BASE_OFFSET \
                        + (((Phyad) & 0x1F) << 8)   \
                        + (((Regad) & 0x1F) << 3),  \
           (Data & XAVB_MAC_MDIO_DATA_MASK))


/************************** Function Prototypes ******************************/

/*
 * Functions in xavb_hw.c
 */
void XAvbMac_ReadStats(u32 BaseAddress, u32 CounterId, XAvb_Uint64* Value);
void XAvb_ReadRtc(u32 BaseAddress, XAvb_RtcFormat* RtcValue);
void XAvb_WriteRtcOffset(u32 BaseAddress, XAvb_RtcFormat* RtcValue);


#ifdef __cplusplus
}
#endif

#endif /* end of protection macro */
