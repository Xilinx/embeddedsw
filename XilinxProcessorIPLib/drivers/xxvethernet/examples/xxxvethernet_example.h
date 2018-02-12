/******************************************************************************
* Copyright (C) 2018 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/**
*
* @file xxxvethernet_example.h
*
* Defines common data types, prototypes, and includes the proper headers
* for use with the Xxv Ethernet and USXGMII example code residing in this
* directory.
*
* This file along with xxxvethernet_example_util.c are utilized with the
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
* 1.0   hk   06/16/17 First release
*       hk   02/15/18 Add support for USXGMII
*
* </pre>
*
******************************************************************************/
#ifndef XXXVETHERNET_EXAMPLE_H
#define XXXVETHERNET_EXAMPLE_H


/***************************** Include Files *********************************/

#include "xparameters.h"	/* defines XPAR values */
#include "xxxvethernet.h"	/* defines Xxv Ethernet APIs */
#include "stdio.h"		/* stdio */

/************************** Constant Definitions ****************************/

/***************** Macros (Inline Functions) Definitions *********************/


/**************************** Type Definitions ******************************/

/*
 * Define an aligned data type for an ethernet frame. This declaration is
 * specific to the GNU compiler
 */
typedef unsigned char EthernetFrame[XXE_MAX_JUMBO_FRAME_SIZE] __attribute__ ((aligned(64)));


/************************** Function Prototypes *****************************/

/*
 * Utility functions implemented in xxxvethernet_example_util.c
 */
void XxvEthernetUtilFrameHdrFormatMAC(EthernetFrame *FramePtr, char *DestAddr, char *SrcAddr);
void XxvEthernetUtilFrameHdrFormatType(EthernetFrame * FramePtr,
							u16 FrameType);
void XxvEthernetUtilFrameSetPayloadData(EthernetFrame * FramePtr,
							int PayloadSize);
int XxvEthernetUtilFrameVerify(EthernetFrame * CheckFrame,
			 EthernetFrame * ActualFrame);
void XxvEthernetUtilFrameMemClear(EthernetFrame * FramePtr);
int XxvEthernetUtilEnterLocalLoopback(XXxvEthernet * XxvEthernetInstancePtr);
void XxvEthernetUtilErrorTrap(char *Message);
int XxvEthernetUtilUsxgmiiSetup(XXxvEthernet *XxvEthernetInstancePtr, u32 Rate, u32 Duplex);
int XxvEthernetUtilUsxgmiiSetupBypassAN(XXxvEthernet *XxvEthernetInstancePtr, u32 Rate, u32 Duplex);

/************************** Variable Definitions ****************************/

extern char XxvEthernetMAC[];		/* Local MAC address */
extern char MacAddr[];
extern char DestAddr[];

#endif /* XXXVETHERNET_EXAMPLE_H */
