/******************************************************************************
*
* Copyright (C) 2010 - 2017 Xilinx, Inc.  All rights reserved.
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
* XILINX  BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
* WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
* OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*
* Except as contained in this notice, the name of the Xilinx shall not be used
* in advertising or otherwise to promote the sale, use or other dealings in
* this Software without prior written authorization from Xilinx.
*
******************************************************************************/
/**
*
* @file xaxiethernet_example.h
*
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
#define XAXIETHERNET_EXAMPLE_H


/***************************** Include Files *********************************/

#include "xparameters.h"	/* defines XPAR values */
#include "xaxiethernet.h"	/* defines Axi Ethernet APIs */
#include "stdio.h"		/* stdio */

/************************** Constant Definitions ****************************/
#define AXIETHERNET_LOOPBACK_SPEED	100	/* 100Mb/s for Mii */
#define AXIETHERNET_LOOPBACK_SPEED_1G 	1000	/* 1000Mb/s for GMii */
#define AXIETHERNET_LOOPBACK_SPEED_2p5G 2500	/* 2p5G for 2.5G MAC */
#define AXIETHERNET_PHY_DELAY_SEC	4	/*
						 * Amount of time to delay waiting on
						 * PHY to reset.
						 */

#define MAX_MULTICAST_ADDR   (1<<23)	/*
					 * Maximum number of multicast ethernet
					 * mac addresses.
					 */
#ifndef TESTAPP_GEN
#define NUM_PACKETS  50
#else
#define NUM_PACKETS  1
#endif

/***************** Macros (Inline Functions) Definitions *********************/


/**************************** Type Definitions ******************************/

/*
 * Define an aligned data type for an ethernet frame. This declaration is
 * specific to the GNU compiler
 */
typedef unsigned char EthernetFrame[NUM_PACKETS * XAE_MAX_JUMBO_FRAME_SIZE] __attribute__ ((aligned(64)));


/************************** Function Prototypes *****************************/

/*
 * Utility functions implemented in xaxiethernet_example_util.c
 */
void AxiEthernetUtilSetupUart(void);
void AxiEthernetUtilFrameHdrFormatMAC(EthernetFrame * FramePtr,
							char *DestAddr);
void AxiEthernetUtilFrameHdrFormatType(EthernetFrame * FramePtr,
							u16 FrameType);
void AxiEthernetUtilFrameSetPayloadData(EthernetFrame * FramePtr,
							int PayloadSize);
void AxiEthernetUtilFrameHdrVlanFormatVid(EthernetFrame * FramePtr,
						u32 VlanNumber,u32 Vid);
void AxiEthernetUtilFrameHdrVlanFormatType(EthernetFrame * FramePtr,
						u16 FrameType,u32 VlanNumber);
void AxiEthernetUtilFrameSetVlanPayloadData(EthernetFrame * FramePtr,
					int PayloadSize,u32 VlanNumber);
int AxiEthernetUtilFrameVerify(EthernetFrame * CheckFrame,
			 EthernetFrame * ActualFrame);
void AxiEthernetUtilFrameMemClear(EthernetFrame * FramePtr);
int AxiEthernetUtilEnterLoopback(XAxiEthernet * AxiEthernetInstancePtr,
								int Speed);
void AxiEthernetUtilErrorTrap(char *Message);
void AxiEthernetUtilPhyDelay(unsigned int Seconds);
int AxiEthernetUtilConfigureInternalPhy(XAxiEthernet *AxiEthernetInstancePtr,
					int Speed);
int AxiEtherentConfigureTIPhy(XAxiEthernet *AxiEthernetInstancePtr,
			      u32 PhyAddr);

/************************** Variable Definitions ****************************/

extern char AxiEthernetMAC[];		/* Local MAC address */
extern volatile int Padding;
extern volatile int ExternalLoopback;
#endif /* XAXIETHERNET_EXAMPLE_H */
