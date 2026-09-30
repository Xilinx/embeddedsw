/*****************************************************************************
* Copyright (C) 2010 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2022 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/**
*
* @file xaxiethernet_example.h
* @addtogroup axiethernet Overview
* @{
*
* @brief Declares shared helpers for the AXI Ethernet examples.
* Defines common data types, prototypes, and includes the proper headers
* for use with the Axi Ethernet example code residing in this directory.
*
* This file along with xaxiethernet_example_util.c are utilized with the
* specific example code in the other source code files provided.
*
* These examples are designed to be compiled and utilized within the EDK
* standalone BSP development environment. The readme file contains more
* information on build requirements needed by these examples.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -------------------------------------------------------
* 1.00a asa  4/30/10 First release based on the ll temac driver
* 3.02a srt  4/26/13 Added function prototype for *_ConfigureInternalPhy().
*
* </pre>
*
******************************************************************************/
#ifndef XAXIETHERNET_EXAMPLE_H
#define XAXIETHERNET_EXAMPLE_H	/**< XAXIETHERNET EXAMPLE H. */

#ifdef __cplusplus
extern "C" {
#endif

/**************************** Include Files *********************************/

#include "xparameters.h"	/* defines XPAR values */
#include "xaxiethernet.h"	/* defines Axi Ethernet APIs */
#include "stdio.h"		/* stdio */

/************************* Constant Definitions ****************************/
#define AXIETHERNET_LOOPBACK_SPEED	100	/**< 100Mb/s for Mii. */
#define AXIETHERNET_LOOPBACK_SPEED_1G 	1000	/**< 1000Mb/s for GMii. */
#define AXIETHERNET_LOOPBACK_SPEED_2p5G 2500	/**< 2p5G for 2.5G MAC. */
#define AXIETHERNET_PHY_DELAY_SEC	4	/**< Amount of time to delay waiting on PHY to reset. */

#define MAX_MULTICAST_ADDR   (1<<23)	/**< Maximum number of multicast Ethernet MAC addresses. */
#ifndef TESTAPP_GEN
#define NUM_PACKETS  50	/**< Number of packets to send in the example. */
#else
#define NUM_PACKETS  1	/**< Number of packets to send in the example. */
#endif

/**************** Macros (Inline Functions) Definitions *********************/


/*************************** Type Definitions ******************************/

/*
 * Define an aligned data type for an ethernet frame. This declaration is.
 * specific to the GNU compiler
 */
/** @cond INTERNAL */
typedef unsigned char EthernetFrame[NUM_PACKETS * XAE_MAX_JUMBO_FRAME_SIZE] __attribute__ ((aligned(64)));
/** @endcond */


/************************* Function Prototypes *****************************/

/*
 * Utility functions implemented in xaxiethernet_example_util.c.
 */
/**
 * @brief Configures the UART used by the example.
 */
void AxiEthernetUtilSetupUart(void);
/**
 * @brief Formats an Ethernet frame header with the destination MAC address.
 *
 * @param[in] FramePtr Pointer to the Ethernet frame buffer.
 * @param[in] DestAddr Destination MAC address.
 */
void AxiEthernetUtilFrameHdrFormatMAC(EthernetFrame * FramePtr,
							char *DestAddr);
/**
 * @brief Formats an Ethernet frame header with the EtherType value.
 *
 * @param[in] FramePtr Pointer to the Ethernet frame buffer.
 * @param[in] FrameType EtherType value to write into the frame header.
 */
void AxiEthernetUtilFrameHdrFormatType(EthernetFrame * FramePtr,
							u16 FrameType);
/**
 * @brief Fills an Ethernet frame payload with test data.
 *
 * @param[in] FramePtr Pointer to the Ethernet frame buffer.
 * @param[in] PayloadSize Number of payload bytes to write.
 */
void AxiEthernetUtilFrameSetPayloadData(EthernetFrame * FramePtr,
							int PayloadSize);
/**
 * @brief Formats a VLAN Ethernet frame header with the VLAN ID.
 *
 * @param[in] FramePtr Pointer to the Ethernet frame buffer.
 * @param[in] VlanNumber VLAN header index.
 * @param[in] Vid VLAN identifier.
 */
void AxiEthernetUtilFrameHdrVlanFormatVid(EthernetFrame * FramePtr,
						u32 VlanNumber,u32 Vid);
/**
 * @brief Formats a VLAN Ethernet frame header with the EtherType value.
 *
 * @param[in] FramePtr Pointer to the Ethernet frame buffer.
 * @param[in] FrameType EtherType value to write into the frame header.
 * @param[in] VlanNumber VLAN header index.
 */
void AxiEthernetUtilFrameHdrVlanFormatType(EthernetFrame * FramePtr,
						u16 FrameType,u32 VlanNumber);
/**
 * @brief Fills a VLAN Ethernet frame payload with test data.
 *
 * @param[in] FramePtr Pointer to the Ethernet frame buffer.
 * @param[in] PayloadSize Number of payload bytes to write.
 * @param[in] VlanNumber VLAN header index.
 */
void AxiEthernetUtilFrameSetVlanPayloadData(EthernetFrame * FramePtr,
					int PayloadSize,u32 VlanNumber);
/**
 * @brief Verifies that two Ethernet frames contain the same data.
 *
 * @param[in] CheckFrame Pointer to the expected Ethernet frame.
 * @param[in] ActualFrame Pointer to the received Ethernet frame.
 *
 * @return XST_SUCCESS if the frames match, otherwise XST_FAILURE.
 */
int AxiEthernetUtilFrameVerify(EthernetFrame * CheckFrame,
			 EthernetFrame * ActualFrame);
/**
 * @brief Clears an Ethernet frame buffer.
 *
 * @param[in] FramePtr Pointer to the Ethernet frame buffer.
 */
void AxiEthernetUtilFrameMemClear(EthernetFrame * FramePtr);
int AxiEthernetUtilEnterLoopback(XAxiEthernet * AxiEthernetInstancePtr,
								int Speed);
void AxiEthernetUtilErrorTrap(char *Message);
void AxiEthernetUtilPhyDelay(unsigned int Seconds);
int AxiEthernetUtilConfigureInternalPhy(XAxiEthernet *AxiEthernetInstancePtr,
					int Speed);
int AxiEtherentConfigureTIPhy(XAxiEthernet *AxiEthernetInstancePtr,
			      u32 PhyAddr);

/************************* Variable Definitions ****************************/

extern char AxiEthernetMAC[];		/* Local MAC address */
extern volatile int Padding;
extern volatile int ExternalLoopback;

#ifdef __cplusplus
}
#endif

#endif /* XAXIETHERNET_EXAMPLE_H */

/** @} */
