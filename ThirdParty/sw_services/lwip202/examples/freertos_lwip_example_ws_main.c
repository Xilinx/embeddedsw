/******************************************************************************
*
* Copyright (C) 2017 Xilinx, Inc.  All rights reserved.
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

#include "xparameters.h"
#include "netif/xadapter.h"
#include "lwip_example_platform_config.h"
#include "xil_printf.h"
#include "lwip/init.h"
#include "lwip/inet.h"
#if (LWIP_DHCP == 1)
#include "lwip/dhcp.h"
#endif

#define THREAD_STACKSIZE        1024
#define MAIN_THREAD_STACKSIZE   2048

#define DEFAULT_IP_ADDRESS      "192.168.1.10"
#define DEFAULT_IP_MASK         "255.255.255.0"
#define DEFAULT_GW_ADDRESS      "192.168.1.1"

#ifdef XPS_BOARD_ZCU102
#ifdef XPAR_XIICPS_0_DEVICE_ID
int IicPhyReset(void);
#endif
#endif

void print_app_header();
void start_application();

#if (LWIP_DHCP == 1)
extern volatile int dhcp_timoutcntr;
err_t dhcp_start(struct netif *netif);
#endif

struct netif server_netif;
static int nw_thread_done;

void print_ip(char *msg, ip_addr_t *ip)
{
	print(msg);
	xil_printf("%d.%d.%d.%d\r\n", ip4_addr1(ip), ip4_addr2(ip),
				ip4_addr3(ip), ip4_addr4(ip));
}

static void print_ip_settings(ip_addr_t *ip, ip_addr_t *mask, ip_addr_t *gw)
{
	print_ip("Board IP:       ", ip);
	print_ip("Netmask :       ", mask);
	print_ip("Gateway :       ", gw);
	xil_printf("\r\n");
}

static void assign_default_ip(ip_addr_t *ip, ip_addr_t *mask, ip_addr_t *gw)
{
	int err;

	xil_printf("Configuring default IP %s\r\n", DEFAULT_IP_ADDRESS);

	err = inet_aton(DEFAULT_IP_ADDRESS, ip);
	if(!err)
		xil_printf("Invalid default IP address: %d\r\n", err);

	err = inet_aton(DEFAULT_IP_MASK, mask);
	if(!err)
		xil_printf("Invalid default IP MASK: %d\r\n", err);

	err = inet_aton(DEFAULT_GW_ADDRESS, gw);
	if(!err)
		xil_printf("Invalid default gateway address: %d\r\n", err);
}

void network_thread(void *p)
{
#if (LWIP_DHCP == 1)
	int mscnt = 0;
#endif
	/* the mac address of the board. this should be unique per board */
	u8 mac_ethernet_address[] = { 0x00, 0x0a, 0x35, 0x00, 0x01, 0x02 };

	/* Add network interface to the netif_list, and set it as default */
	if (!xemac_add(&server_netif, NULL, NULL, NULL,
			mac_ethernet_address, PLATFORM_EMAC_BASEADDR)) {
		xil_printf("Error adding N/W interface\r\n");
		return;
	}

	netif_set_default(&server_netif);

	/* specify that the network if is up */
	netif_set_up(&server_netif);

	/* start packet receive thread - required for lwIP operation */
	sys_thread_new("xemacif_input_thread",
			(void(*)(void*))xemacif_input_thread, &server_netif,
			THREAD_STACKSIZE, DEFAULT_THREAD_PRIO);

#if (LWIP_DHCP == 1)
	dhcp_start(&server_netif);
	nw_thread_done = 1;
	while (1) {
		vTaskDelay(DHCP_FINE_TIMER_MSECS / portTICK_RATE_MS);
		dhcp_fine_tmr();
		mscnt += DHCP_FINE_TIMER_MSECS;
		if (mscnt >= DHCP_COARSE_TIMER_SECS*1000) {
			dhcp_coarse_tmr();
			mscnt = 0;
		}
	}
#else
	nw_thread_done = 1;
	vTaskDelete(NULL);
#endif
}

int main_thread()
{

#if (LWIP_DHCP == 1)
	int mscnt = 0;
#endif

#ifdef XPS_BOARD_ZCU102
#ifdef XPAR_XIICPS_0_DEVICE_ID
	IicPhyReset();
#endif
#endif

	xil_printf("\r\n\r\n");
	xil_printf("-----lwIP Socket Mode Demo Application ------\r\n");

	/* initialize lwIP before calling sys_thread_new */
	lwip_init();

	/* any thread using lwIP should be created using sys_thread_new */
	sys_thread_new("nw_thread", network_thread, NULL,
			THREAD_STACKSIZE, DEFAULT_THREAD_PRIO);

	/* wait for nw thread to finish initialization */
	while (!nw_thread_done)
		vTaskDelay(100 / portTICK_RATE_MS); /* 100 millisecond */

#if (LWIP_DHCP == 1)
	while (1) {
		vTaskDelay(DHCP_FINE_TIMER_MSECS / portTICK_RATE_MS);

		if (server_netif.ip_addr.addr) {
			xil_printf("DHCP request success\r\n");
			break;
		}

		mscnt += DHCP_FINE_TIMER_MSECS;
		if (mscnt >= 10000) {
			xil_printf("ERROR: DHCP request timed out\r\n");
			assign_default_ip(&(server_netif.ip_addr),
				&(server_netif.netmask), &(server_netif.gw));
			break;
		}
	}
#else
	assign_default_ip(&(server_netif.ip_addr), &(server_netif.netmask),
			&(server_netif.gw));
#endif

	print_ip_settings(&server_netif.ip_addr, &server_netif.netmask,
			&server_netif.gw);

	/* print application header */
	print_app_header();

	/* start the application */
	start_application();

	vTaskDelete(NULL);
	return 0;
}

int main()
{
	sys_thread_new("main_thread", (void(*)(void*))main_thread, 0,
			MAIN_THREAD_STACKSIZE, DEFAULT_THREAD_PRIO);
	vTaskStartScheduler();
	while(1);
	return 0;
}
