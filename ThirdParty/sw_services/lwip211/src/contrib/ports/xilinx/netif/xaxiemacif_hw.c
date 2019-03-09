/*
 * Copyright (C) 2010 - 2019 Xilinx, Inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT
 * SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT
 * OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY
 * OF SUCH DAMAGE.
 *
 * This file is part of the lwIP TCP/IP stack.
 *
 */

#include "netif/xaxiemacif.h"
#include "lwipopts.h"

extern enum ethernet_link_status eth_link_status;

XAxiEthernet_Config *xaxiemac_lookup_config(unsigned mac_base)
{
	extern XAxiEthernet_Config XAxiEthernet_ConfigTable[];
	XAxiEthernet_Config *CfgPtr = NULL;
	unsigned int i;

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
	unsigned mac_address = (unsigned)(UINTPTR)(netif->state);
	unsigned link_speed = 1000;
	unsigned options;
	XAxiEthernet *xaxiemacp;

	xaxiemacp = &xaxiemac->axi_ethernet;

	XAxiEthernet_Reset(xaxiemacp);

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
	link_speed = phy_setup_axiemac(xaxiemacp);
	XAxiEthernet_SetOperatingSpeed(xaxiemacp, link_speed);

	if (link_speed == 0)
		eth_link_status = ETH_LINK_DOWN;
	else
		eth_link_status = ETH_LINK_UP;

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
