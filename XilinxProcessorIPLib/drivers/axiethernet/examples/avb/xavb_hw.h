/*****************************************************************************
* Copyright (C) 2008 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2022 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/****************************************************************************/
/**
*
* @file xavb_hw.h
* @addtogroup axiethernet Overview
*
* @brief Defines AVB hardware register access macros for AXI Ethernet.
* Defines the identifiers and basic driver functions (or
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
/** @brief Include guard for the AVB hardware header. */
#define XAVB_HW_H    /* by using protection macros */

#ifdef __cplusplus
extern "C" {
#endif


/**************************** Include Files *********************************/

#include "xil_types.h"
#include "xil_io.h"

/************************* Constant Definitions *****************************/

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
/** @} */

/** @name Register offsets for the Ethernet Audio Video Endpoint. Each register
 * is 32 bits.
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
/** @} */

/** @name Packet base address offsets for the Ethernet Audio Video Endpoint Tx
 * Precise Timing Protocol (PTP) frame buffer.  Each PTP frames is.
 *        stored in 256-byte chunks of BRAM.  This BRAM can store 8 PTP frames
 *        of which only 6 are currently in use.
 *  @{
 */
#define XAVB_PTP_TX_SYNC_OFFSET                 0x00011000 /**< Tx PTP Sync frame buffer offset. */
#define XAVB_PTP_TX_FOLLOW_UP_OFFSET            0x00011100 /**< Tx PTP Follow_Up frame buffer offset. */
#define XAVB_PTP_TX_PDELAYREQ_OFFSET            0x00011200 /**< Tx PTP Pdelay_Req frame buffer offset. */
#define XAVB_PTP_TX_PDELAYRESP_OFFSET           0x00011300 /**< Tx PTP Pdelay_Resp frame buffer offset. */
#define XAVB_PTP_TX_PDELAYRESP_FOLLOW_UP_OFFSET 0x00011400 /**< Tx PTP Pdelay_Resp_Follow_Up frame buffer offset. */
#define XAVB_PTP_TX_ANNOUNCE_OFFSET             0x00011500 /**< Tx PTP Announce frame buffer offset. */
/** @} */

/** @name Base address offset for the Ethernet Audio Video Endpoint Rx
 * Precise Timing Protocol (PTP) frame buffer.  These PTP frames are.
 *        stored in 256-byte chunks of BRAM.  This BRAM can store 16 PTP frames.
 *  @{
 */
#define XAVB_PTP_RX_BASE_OFFSET                 0x00010000 /**< Rx PTP frame buffer base offset. */
/** @} */

/** @name AVB Tx PTP Control Register
 *  @{
 */
#define XAVB_PTP_TX_SEND_SYNC_FRAME_MASK                     0x00000001 /**< Tx send Sync frame bit mask. */
#define XAVB_PTP_TX_SEND_FOLLOWUP_FRAME_MASK                 0x00000002 /**< Tx send Follow_Up frame bit mask. */
#define XAVB_PTP_TX_SEND_PDELAYREQ_FRAME_MASK                0x00000004 /**< Tx send Pdelay_Req frame bit mask. */
#define XAVB_PTP_TX_SEND_PDELAYRESP_FRAME_MASK               0x00000008 /**< Tx send Pdelay_Resp frame bit mask. */
#define XAVB_PTP_TX_SEND_PDELAYRESPFOLLOWUP_FRAME_MASK       0x00000010 /**< Tx send Pdelay_Resp_Follow_Up frame bit mask. */
#define XAVB_PTP_TX_SEND_ANNOUNCE_FRAME_MASK                 0x00000020 /**< Tx send Announce frame bit mask. */
#define XAVB_PTP_TX_SEND_FRAME6_BIT_MASK                     0x00000040 /**< Tx send frame 6 bit mask. */
#define XAVB_PTP_TX_SEND_FRAME7_BIT_MASK                     0x00000080 /**< Tx send frame 7 bit mask. */
#define XAVB_PTP_TX_WAIT_SYNC_FRAME_MASK                     0x00000100 /**< Tx wait Sync frame bit mask. */
#define XAVB_PTP_TX_WAIT_FOLLOWUP_FRAME_MASK                 0x00000200 /**< Tx wait Follow_Up frame bit mask. */
#define XAVB_PTP_TX_WAIT_PDELAYREQ_FRAME_MASK                0x00000400 /**< Tx wait Pdelay_Req frame bit mask. */
#define XAVB_PTP_TX_WAIT_PDELAYRESP_FRAME_MASK               0x00000800 /**< Tx wait Pdelay_Resp frame bit mask. */
#define XAVB_PTP_TX_WAIT_PDELAYRESPFOLLOWUP_FRAME_MASK       0x00001000 /**< Tx wait Pdelay_Resp_Follow_Up frame bit mask. */
#define XAVB_PTP_TX_WAIT_ANNOUNCE_FRAME_MASK                 0x00002000 /**< Tx wait Announce frame bit mask. */
#define XAVB_PTP_TX_WAIT_FRAME6_BIT_MASK                     0x00004000 /**< Tx wait frame 6 bit mask. */
#define XAVB_PTP_TX_WAIT_FRAME7_BIT_MASK                     0x00008000 /**< Tx wait frame 7 bit mask. */
#define XAVB_PTP_TX_WAIT_ALL_FRAMES_MASK                     0x0000FF00 /**< Tx wait all frames bit mask. */
#define XAVB_PTP_TX_PACKET_FIELD_MASK                        0x00070000 /**< Tx PTP packet field bit mask. */

/** @} */

/** @name AVB Rx PTP Control Register
 *  @{
 */
#define XAVB_PTP_RX_CLEAR_BIT_MASK                           0x00000001 /**< Rx PTP clear bit mask. */
#define XAVB_PTP_RX_PACKET_FIELD_MASK                        0x00000F00 /**< Rx PTP packet field bit mask. */
/** @} */

/** @name AVB Rx Filter Control Register
 *  @{
 */
#define XAVB_RX_AV_VLAN_PRIORITY_A_MASK                      0x00000007 /**< Rx AV VLAN priority A bit mask. */
#define XAVB_RX_AV_VLAN_VID_A_MASK                           0x00007FF8 /**< Rx AV VLAN VID A bit mask. */
#define XAVB_RX_AV_VLAN_MATCH_MODE_MASK                      0x00008000 /**< Rx AV VLAN match mode bit mask. */
#define XAVB_RX_AV_VLAN_PRIORITY_B_MASK                      0x00070000 /**< Rx AV VLAN priority B bit mask. */
#define XAVB_RX_AV_VLAN_VID_B_MASK                           0x7FF80000 /**< Rx AV VLAN VID B bit mask. */
#define XAVB_RX_LEGACY_PROMISCUOUS_MODE_MASK                 0x80000000 /**< Rx legacy promiscuous mode bit mask. */
/** @} */

/** @name AVB Tx rate control sendSlope
 *  @{
 */
#define XAVB_TX_SENDSLOPE_MASK                               0X000FFFFF /**< Tx rate control sendSlope bit mask. */
/** @} */

/** @name AVB Tx rate control idleSlope
 *  @{
 */
#define XAVB_TX_IDLESLOPE_MASK                               0X000FFFFF /**< Tx rate control idleSlope bit mask. */
/** @} */

/** @name AVB Tx rate control hiLimit
 *  @{
 */
#define XAVB_TX_HILIMIT_MASK                                 0X01FFFFFF /**< Tx rate control hiLimit bit mask. */
/** @} */

/** @name AVB Tx rate control loLimit
 *  @{
 */
#define XAVB_TX_LOLIMIT_MASK                                 0X01FFFFFF /**< Tx rate control loLimit bit mask. */
/** @} */

/** @name RTC field Registers
 *  @{
 */
#define XAVB_RTC_NS_MASK                                     0x3FFFFFFF /**< RTC nanoseconds field bit mask. */
#define XAVB_RTC_SEC_LOWER_MASK                              0xFFFFFFFF /**< RTC lower seconds field bit mask. */
#define XAVB_RTC_SEC_UPPER_MASK                              0x0000FFFF /**< RTC upper seconds field bit mask. */
/** @} */

/** @name RTC Increment Register
 *  @{
 */
#define XAVB_RTC_INCREMENT_VALUE_MASK                        0x03FFFFFF /**< RTC increment value bit mask. */
#define XAVB_RTC_INCREMENT_NOMINAL_RATE                      0x00800000 /**< RTC increment nominal rate for a 125 MHz clock. */
/** Add rails so recovery is possible after a string of bad pDelay values.
 * The RTC should lock within 100 ppm of the slowest allowable clock (25 MHz).
 * This equates to +/-4 ps. The rails are set to 400 ppm (+/-16 ps). The lowest
 * 20 bits of NewIncrement are fractions of a nanosecond (+/- 0x04189).
 */
#define XAVB_RTC_400PPM_OFFSET                               0x00004189 /**< RTC increment offset for approximately 400 ppm. */
/** @} */

/** @name RTC Interrupt Clear Register
 *  @{
 */
#define XAVB_RTC_INCREMENT_VALUE_MASK                        0x03FFFFFF /**< RTC increment value bit mask. */
/** @} */


/** @name RTC Interrupt Clear Register
 *  @{
 */
#define XAVB_RTC_CLEAR_INT_MASK                              0x00000000 /**< RTC interrupt clear bit mask. */
/** @} */


/** @name RTC 8k phase offset Register
 *  @{
 */
#define XAVB_RTC_8K_PHASE_OFFSET_MASK                        0x3FFFFFFF /**< RTC 8 kHz phase offset bit mask. */
/** @} */

/** @name S/W Reset  Register
 *  @{
 */
#define XAVB_SW_RESET_TX_AND_RX_PATHS                        0x00000003 /**< Software reset for Tx and Rx paths. */
/** @} */


/** @name AVB MAC MDIO register address space.
 *  @{
 */
#define XAVB_MAC_MDIO_BASE_OFFSET                            0x00006000 /**< MAC MDIO register base address offset. */
/** @} */


/** @name MDIO valid data mask (MDIO registers are all 16-bits).
 *  @{
 */
#define XAVB_MAC_MDIO_DATA_MASK                              0x0000FFFF /**< MAC MDIO 16-bit data bit mask. */
/** @} */


/** @name MAC Statistic Counter names and CounterID.
 *  @{
 */
#define XAVB_STATS_BYTES_TRANSMITTED                         0x00000000 /**< Stats counter ID for bytes transmitted. */
#define XAVB_STATS_BYTES_RECEIVED                            0x00000001 /**< Stats counter ID for bytes received. */
#define XAVB_STATS_UNDERSIZED_FRAMES_RECEIVED                0x00000002 /**< Stats counter ID for undersized frames received. */
#define XAVB_STATS_FRAGMENT_FRAMES_RECEIVED                  0x00000003 /**< Stats counter ID for fragment frames received. */
#define XAVB_STATS_64_BYTE_FRAMES_RECEIVED_OK                0x00000004 /**< Stats counter ID for 64-byte frames received OK. */
#define XAVB_STATS_65_TO_127_BYTE_FRAMES_RECEIVED_OK         0x00000005 /**< Stats counter ID for 65 to 127 byte frames received OK. */
#define XAVB_STATS_128_TO_255_BYTE_FRAMES_RECEIVED_OK        0x00000006 /**< Stats counter ID for 128 to 255 byte frames received OK. */
#define XAVB_STATS_256_TO_511_BYTE_FRAMES_RECEIVED_OK        0x00000007 /**< Stats counter ID for 256 to 511 byte frames received OK. */
#define XAVB_STATS_512_TO_1023_BYTE_FRAMES_RECEIVED_OK       0x00000008 /**< Stats counter ID for 512 to 1023 byte frames received OK. */
#define XAVB_STATS_1024_TO_1518_BYTE_FRAMES_RECEIVED_OK      0x00000009 /**< Stats counter ID for 1024 to 1518 byte frames received OK. */
#define XAVB_STATS_OVERSIZED_FRAMES_RECEIVED_OK              0x0000000A /**< Stats counter ID for oversized frames received OK. */
#define XAVB_STATS_64_BYTE_FRAMES_TRANSMITTED_OK             0x0000000B /**< Stats counter ID for 64-byte frames transmitted OK. */
#define XAVB_STATS_65_TO_127_BYTE_FRAMES_TRANSMITTED_OK      0x0000000C /**< Stats counter ID for 65 to 127 byte frames transmitted OK. */
#define XAVB_STATS_128_TO_255_BYTE_FRAMES_TRANSMITTED_OK     0x0000000D /**< Stats counter ID for 128 to 255 byte frames transmitted OK. */
#define XAVB_STATS_256_TO_511_BYTE_FRAMES_TRANSMITTED_OK     0x0000000E /**< Stats counter ID for 256 to 511 byte frames transmitted OK. */
#define XAVB_STATS_512_TO_1023_BYTE_FRAMES_TRANSMITTED_OK    0x0000000F /**< Stats counter ID for 512 to 1023 byte frames transmitted OK. */
#define XAVB_STATS_1024_TO_1518_BYTE_FRAMES_TRANSMITTED_OK   0x00000010 /**< Stats counter ID for 1024 to 1518 byte frames transmitted OK. */
#define XAVB_STATS_OVERSIZE_FRAMES_TRANSMITTED_OK            0x00000011 /**< Stats counter ID for oversize frames transmitted OK. */
#define XAVB_STATS_FRAMES_RECEIVED_OK                        0x00000012 /**< Stats counter ID for frames received OK. */
#define XAVB_STATS_FCS_ERRORS                                0x00000013 /**< Stats counter ID for FCS errors. */
#define XAVB_STATS_BROADCAST_FRAMES_RECEIVED_OK              0x00000014 /**< Stats counter ID for broadcast frames received OK. */
#define XAVB_STATS_MULTICAST_FRAMES_RECEIVED_OK              0x00000015 /**< Stats counter ID for multicast frames received OK. */
#define XAVB_STATS_CONTROL_FRAMES_RECEIVED_OK                0x00000016 /**< Stats counter ID for control frames received OK. */
#define XAVB_STATS_LENGTH_TYPE_OUT_OF_RANGE                  0x00000017 /**< Stats counter ID for length or type out of range. */
#define XAVB_STATS_VLAN_TAGGED_FRAMES_RECEIVED_OK            0x00000018 /**< Stats counter ID for VLAN tagged frames received OK. */
#define XAVB_STATS_PAUSE_FRAMES_RECEIVED_OK                  0x00000019 /**< Stats counter ID for pause frames received OK. */
#define XAVB_STATS_CONTROL_FRAMES_WITH_UNSUPPORTED_OPCODE    0x0000001A /**< Stats counter ID for control frames with unsupported opcode. */
#define XAVB_STATS_FRAMES_TRANSMITTED                        0x0000001B /**< Stats counter ID for frames transmitted. */
#define XAVB_STATS_BROADCAST_FRAMES_TRANSMITTED              0x0000001C /**< Stats counter ID for broadcast frames transmitted. */
#define XAVB_STATS_MULTICAST_FRAMES_TRANSMITTED              0x0000001D /**< Stats counter ID for multicast frames transmitted. */
#define XAVB_STATS_UNDERRUN_ERRORS                           0x0000001E /**< Stats counter ID for underrun errors. */
#define XAVB_STATS_CONTROL_FRAMES_TRANSMITTED_OK             0x0000001F /**< Stats counter ID for control frames transmitted OK. */
#define XAVB_STATS_VLAN_TAGGED_FRAMES_TRANSMITTED_OK         0x00000020 /**< Stats counter ID for VLAN tagged frames transmitted OK. */
#define XAVB_STATS_PAUSE_FRAMES_TRANSMITTED_OK               0x00000021 /**< Stats counter ID for pause frames transmitted OK. */
#define XAVB_STATS_SINGLE_COLLISION_FRAMES                   0x00000022 /**< Stats counter ID for single collision frames. */
#define XAVB_STATS_MULTI_COLLISION_FRAMES                    0x00000023 /**< Stats counter ID for multiple collision frames. */
#define XAVB_STATS_DEFERRAL_FRAMES                           0x00000024 /**< Stats counter ID for deferral frames. */
#define XAVB_STATS_LATE_COLLISION_FRAMES                     0x00000025 /**< Stats counter ID for late collision frames. */
#define XAVB_STATS_EXCESS_COLLISION_FRAMES                   0x00000026 /**< Stats counter ID for excess collision frames. */
#define XAVB_STATS_EXCESS_DEFERRAL_FRAMES                    0x00000027 /**< Stats counter ID for excess deferral frames. */
#define XAVB_STATS_CARRIER_SENSE_ERRORS                      0x00000028 /**< Stats counter ID for carrier sense errors. */
#define XAVB_STATS_ALIGNMENT_ERRORS                          0x00000029 /**< Stats counter ID for alignment errors. */
/** @} */


/*************************** Type Definitions *******************************/

/**
 * Defines the format for the Real Time Clock (RTC).  The RTC.
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
 * Describes a 64-bit un-signed integer in terms of 2 u32s.
 */
typedef struct
{
  u32 Upper;        /**< Upper 32 bits */
  u32 Lower;        /**< Lower 32 bits */
} XAvb_Uint64;

/**************** Macros (Inline Functions) Definitions *********************/

/***************************************************************************/
/**
*
* @brief Reads from the given AVB core register.
*
* @param[in] BaseAddress is the base address of the device
* @param[in] RegOffset is the register offset to be read
*
* @return The 32-bit value of the register
*
*****************************************************************************/
#define XAvb_ReadReg(BaseAddress, RegOffset) \
      Xil_In32((BaseAddress) + (RegOffset))


/***************************************************************************/
/**
*
* @brief Writes to the given AVB core register.
*
* @param[in] BaseAddress is the base address of the device
* @param[in] RegOffset is the register offset to be written
* @param[in] Data is the 32-bit value to write to the register
*
*****************************************************************************/
#define XAvb_WriteReg(BaseAddress, RegOffset, Data) \
      Xil_Out32((BaseAddress) + (RegOffset), (Data))


/***************************************************************************/
/**
*
* @brief Reads from the given PTP frame buffer.
*
* @param[in] BaseAddress is the base address of the device
* @param[in] PtpPacketBaseAddress is the base address of the frame in the PTP
*         BRAM packet buffer
* @param[in] PtpPacketOffset is the offset address within the PTP frame
*
* @return The 32-bit value of the register
*
*****************************************************************************/
#define XAvb_ReadPtpBuffer(BaseAddress, PtpPacketBaseAddress, PtpPacketOffset)\
      Xil_In32(BaseAddress + PtpPacketBaseAddress + PtpPacketOffset)


/***************************************************************************/
/**
*
* @brief Writes to the given PTP frame buffer.
*
* @param[in] BaseAddress is the base address of the device
* @param[in] PtpPacketBaseAddress is the base address of the frame in the PTP
*         packet buffer
* @param[in] PtpPacketOffset is the offset address within the PTP frame
* @param[in] Data is the 32-bit value to write to the register
*
*****************************************************************************/
#define XAvb_WritePtpBuffer(BaseAddress, PtpPacketBaseAddress, \
                             PtpPacketOffset, Data) \
      Xil_Out32(BaseAddress + PtpPacketBaseAddress + PtpPacketOffset, (Data))


/***************************************************************************/
/**
*
* @brief Reads from the given TEMAC Configuration Register.
*
* @param[in] BaseAddress is the base address of the device
* @param[in] RegOffset is the register offset to be read
*
* @return The 32-bit value of the register
*
*****************************************************************************/
#define XAvbMac_ReadConfig(BaseAddress, RegOffset) \
      Xil_In32((BaseAddress) + (RegOffset))


/***************************************************************************/
/**
*
* @brief Writes to the given TEMAC Configuration Register.
*
* @param[in] BaseAddress is the base address of the device
* @param[in] RegOffset is the register offset to be written
* @param[in] Data is the 32-bit value to write to the register
*
*****************************************************************************/
#define XAvbMac_WriteConfig(BaseAddress, RegOffset, Data) \
      Xil_Out32((BaseAddress) + (RegOffset), (Data))


/***************************************************************************/
/**
*
* @brief Reads from the given MDIO Register using the TEMAC.
*
* @param[in] BaseAddress is the base address of the device
* @param[in] Phyad is the Physical Address of the PHY
* @param[in] Regad is the Address of the MDIO register within the addressed PHY
*
* @return The 32-bit value of the register (upper 16-bits will be all 0's)
*
*****************************************************************************/
#define XAvbMac_ReadMdio(BaseAddress, Phyad, Regad) \
  Xil_In32((BaseAddress) + XAVB_MAC_MDIO_BASE_OFFSET         \
                                  + (((Phyad) & 0x1F) << 8)  \
                                  + (((Regad) & 0x1F) << 3))


/***************************************************************************/
/**
*
* @brief Writes to the given MDIO Register using the TEMAC.
*
* @param[in] BaseAddress is the base address of the device
* @param[in] Phyad is the Physical Address of the PHY
* @param[in] Regad is the Address of the MDIO register within the addressed PHY
* @param[in] Data is the 32-bit value to write
*
*****************************************************************************/
#define XAvbMac_WriteMdio(BaseAddress, Phyad, Regad, Data) \
  Xil_Out32(BaseAddress + XAVB_MAC_MDIO_BASE_OFFSET \
                        + (((Phyad) & 0x1F) << 8)   \
                        + (((Regad) & 0x1F) << 3),  \
           (Data & XAVB_MAC_MDIO_DATA_MASK))


/************************* Function Prototypes ******************************/

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
