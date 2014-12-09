/******************************************************************************
*
* Copyright (C) 2007 - 2014 Xilinx, Inc.  All rights reserved.
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
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* XILINX CONSORTIUM BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
* WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
* OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*
* Except as contained in this notice, the name of the Xilinx shall not be used
* in advertising or otherwise to promote the sale, use or other dealings in
* this Software without prior written authorization from Xilinx.
*
******************************************************************************/

#include "netif/xlltemacif.h"
#include "lwipopts.h"

static XLlTemac_Config *
xlltemac_lookup_config(unsigned mac_base)
{
	extern XLlTemac_Config XLlTemac_ConfigTable[];
	XLlTemac_Config *CfgPtr = NULL;
	int i;

	for (i = 0; i < XPAR_XLLTEMAC_NUM_INSTANCES; i++) {
		if (XLlTemac_ConfigTable[i].BaseAddress == mac_base) {
			CfgPtr = &XLlTemac_ConfigTable[i];
			break;
		}
	}

	return (CfgPtr);
}

void
init_lltemac(xlltemacif_s *xlltemacif, struct netif *netif)
{
	int rdy;
	unsigned mac_address = (unsigned)(netif->state);
	unsigned link_speed = 1000;
	unsigned options;
        unsigned lock_message_printed = 0;

	/* obtain config of this emac */
	XLlTemac_Config *mac_config = xlltemac_lookup_config(mac_address);

	XLlTemac *xlltemacp = &xlltemacif->lltemac;

	XLlTemac_CfgInitialize(xlltemacp, mac_config, mac_config->BaseAddress);

	options = XLlTemac_GetOptions(xlltemacp);
	options |= XTE_FLOW_CONTROL_OPTION;
	options |= XTE_JUMBO_OPTION;
	options |= XTE_TRANSMITTER_ENABLE_OPTION;
	options |= XTE_RECEIVER_ENABLE_OPTION;
	options |= XTE_FCS_STRIP_OPTION;
	options |= XTE_MULTICAST_OPTION;
	XLlTemac_SetOptions(xlltemacp, options);
	XLlTemac_ClearOptions(xlltemacp, ~options);

	/* set mac address */
	XLlTemac_SetMacAddress(xlltemacp, (Xuint8*)(netif->hwaddr));

	/* make sure the hard TEMAC is ready */
	rdy = XLlTemac_ReadReg(xlltemacp->Config.BaseAddress,
			XTE_RDY_OFFSET);
	while ((rdy & XTE_RDY_HARD_ACS_RDY_MASK) == 0) {
		rdy = XLlTemac_ReadReg(xlltemacp->Config.BaseAddress,
				XTE_RDY_OFFSET);
	}

	link_speed = Phy_Setup(xlltemacp);
	XLlTemac_SetOperatingSpeed(xlltemacp, link_speed);

	/* Setting the operating speed of the MAC needs a delay. */
	{
		volatile int wait;
		for (wait=0; wait < 100000; wait++);
		for (wait=0; wait < 100000; wait++);
	}

        /* in a soft temac implementation, we need to explicitly make sure that
         * the RX DCM has been locked. See xps_ll_temac manual for details.
         * This bit is guaranteed to be 1 for hard temac's
         */
        lock_message_printed = 0;
        while (!(XLlTemac_ReadReg(xlltemacp->Config.BaseAddress, XTE_IS_OFFSET)
                    & XTE_INT_RXDCM_LOCK_MASK)) {
                int first = 1;
                if (first) {
                        print("Waiting for RX DCM to lock..");
                        first = 0;
                        lock_message_printed = 1;
                }
        }

        if (lock_message_printed)
                print("RX DCM locked.\r\n");

	/* start the temac */
	XLlTemac_Start(xlltemacp);

	/* enable TEMAC interrupts */
	XLlTemac_IntEnable(xlltemacp, XTE_INT_RECV_ERROR_MASK);
}

void
xlltemac_error_handler(XLlTemac * Temac)
{
	unsigned Pending = XLlTemac_IntPending(Temac);
	if (Pending & XTE_INT_RXFIFOOVR_MASK) {
		print("Temac error interrupt: Rx fifo over run\r\n");
	}
	XLlTemac_IntClear(Temac, Pending);
}
