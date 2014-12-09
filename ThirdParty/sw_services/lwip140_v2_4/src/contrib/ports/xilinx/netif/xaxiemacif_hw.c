/******************************************************************************
*
* Copyright (C) 2010 - 2014 Xilinx, Inc.  All rights reserved.
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

#include "netif/xaxiemacif.h"
#include "lwipopts.h"

XAxiEthernet_Config *xaxiemac_lookup_config(unsigned mac_base)
{
	extern XAxiEthernet_Config XAxiEthernet_ConfigTable[];
	XAxiEthernet_Config *CfgPtr = NULL;
	int i;

	for (i = 0; i < XPAR_XAXIETHERNET_NUM_INSTANCES; i++) {
		if (XAxiEthernet_ConfigTable[i].BaseAddress == mac_base) {
			CfgPtr = &XAxiEthernet_ConfigTable[i];
			break;
		}
	}

	return (CfgPtr);
}

void init_axiemac(xaxiemacif_s *xaxiemac, struct netif *netif)
{
	unsigned mac_address = (unsigned)(netif->state);
	unsigned link_speed = 1000;
	unsigned options;
	XAxiEthernet *xaxiemacp;
	XAxiEthernet_Config *mac_config;

	/* obtain config of this emac */
	mac_config = xaxiemac_lookup_config(mac_address);

	xaxiemacp = &xaxiemac->axi_ethernet;

	XAxiEthernet_CfgInitialize(xaxiemacp, mac_config, mac_config->BaseAddress);

	options = XAxiEthernet_GetOptions(xaxiemacp);
	options |= XAE_FLOW_CONTROL_OPTION;
#ifdef USE_JUMBO_FRAMES
	options |= XAE_JUMBO_OPTION;
#endif
	options |= XAE_TRANSMITTER_ENABLE_OPTION;
	options |= XAE_RECEIVER_ENABLE_OPTION;
	options |= XAE_FCS_STRIP_OPTION;
	options |= XAE_MULTICAST_OPTION;
	XAxiEthernet_SetOptions(xaxiemacp, options);
	XAxiEthernet_ClearOptions(xaxiemacp, ~options);

	/* set mac address */
	XAxiEthernet_SetMacAddress(xaxiemacp, (unsigned char*)(netif->hwaddr));
	link_speed = Phy_Setup(xaxiemacp);
	XAxiEthernet_SetOperatingSpeed(xaxiemacp, link_speed);

	/* Setting the operating speed of the MAC needs a delay. */
	{
		volatile int wait;
		for (wait=0; wait < 100000; wait++);
		for (wait=0; wait < 100000; wait++);
	}

#ifdef NOTNOW
        /* in a soft temac implementation, we need to explicitly make sure that
         * the RX DCM has been locked. See xps_ll_temac manual for details.
         * This bit is guaranteed to be 1 for hard temac's
         */
        lock_message_printed = 0;
        while (!(XAxiEthernet_ReadReg(xaxiemacp->Config.BaseAddress, XAE_IS_OFFSET)
                    & XAE_INT_RXDCMLOCK_MASK)) {
                int first = 1;
                if (first) {
                        print("Waiting for RX DCM to lock..");
                        first = 0;
                        lock_message_printed = 1;
                }
        }

        if (lock_message_printed)
                print("RX DCM locked.\r\n");
#endif

	/* start the temac */
	XAxiEthernet_Start(xaxiemacp);

	/* enable MAC interrupts */
	XAxiEthernet_IntEnable(xaxiemacp, XAE_INT_RECV_ERROR_MASK);
}

void xaxiemac_error_handler(XAxiEthernet * Temac)
{
	unsigned Pending;

	Pending = XAxiEthernet_IntPending(Temac);
	XAxiEthernet_IntClear(Temac, Pending);
}
