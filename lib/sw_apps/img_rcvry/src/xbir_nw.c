/******************************************************************************
* Copyright (c) 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xbir_nw.c
*
* This file contains network configuration functions.
*
******************************************************************************/

/***************************** Include Files *********************************/
#include "xbir_platform.h"
#include "xstatus.h"
#include "netif/xadapter.h"
#include "xbir_config.h"
#include "lwip/init.h"
#include "lwip/inet.h"
#include "lwip/priv/tcp_priv.h"
#include "xbir_nw.h"

/************************** Constant Definitions *****************************/
/* TODO: Read MAC address from EEPROM and assign it */
static const u8 Xbir_NwMacEthAddr[] = { 0x00U, 0x0AU, 0x35U, 0x00U, 0x01U, 0x02U };

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/
static int Xbir_NwSetDefaultIp (ip_addr_t *Ip, ip_addr_t *Mask, ip_addr_t *Gw);

/************************** Variable Definitions *****************************/
extern u8 TcpFastTmrFlag;
extern u8 TcpSlowTmrFlag;

/*****************************************************************************/
/**
 * @brief
 * This function configures the SoM network and assigns default IP address
 *
 * @param	NetIf	Pointer to network interface instance
 *
 * @return	XST_SUCCESS if initialization is successful
 *		XST_FAILURE if initialization is failed
 *
 *****************************************************************************/
int Xbir_NwCfgNetwork (struct netif *NetIf)
{
	int Status = XST_FAILURE;

	/* Initialize lwip network stack */
	lwip_init();

	/* Add network interface to the netif_list, and set it as default */
	if (!xemac_add(NetIf, NULL, NULL, NULL,
			(u8 *)Xbir_NwMacEthAddr, XBIR_PLATFORM_EMAC_BASEADDR)) {
		Xbir_Printf("ERROR: Error adding N/W interface\n\r");
		goto END;
	}

	netif_set_default(NetIf);
	Xbir_Platform_EnableInterrupts();

	/* Specify that the network if(interface) is UP */
	netif_set_up(NetIf);

	/* Assign default and fixed IP address, mask and gateway */
	if (Xbir_NwSetDefaultIp(&NetIf->ip_addr, &NetIf->netmask,
		&NetIf->gw) != XST_SUCCESS) {
		goto END;
	}

	Xbir_NwPrintIpCfg(&NetIf->ip_addr, &NetIf->netmask, &NetIf->gw);
	Status = XST_SUCCESS;

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief
 * This function sets the default fixed IP address for the SoM.
 *
 * @param	Ip	Pointer to IP address
 * @param	Mask	Pointer to network mask
 * @param	Gw	Pointer to gateway address
 *
 * @return	XST_SUCCESS if initialization is successful
 *		XST_FAILURE if initialization is failed
 *
 *****************************************************************************/
static int Xbir_NwSetDefaultIp (ip_addr_t *Ip, ip_addr_t *Mask, ip_addr_t *Gw)
{
	int Status = XST_FAILURE;
	int Error;
	int InvalidCfg = FALSE;

	Xbir_Printf("Configuring default IP %s\r\n", XBIR_NW_DEFAULT_IP_ADDRESS);

	Error = inet_aton(XBIR_NW_DEFAULT_IP_ADDRESS, Ip);
	if (Error == 0) {
		InvalidCfg = TRUE;
		Xbir_Printf("ERROR: Invalid default IP address: %d\r\n", Error);
	}

	Error = inet_aton(XBIR_NW_DEFAULT_IP_MASK, Mask);
	if (Error == 0) {
		InvalidCfg = TRUE;
		Xbir_Printf("ERROR: Invalid default IP MASK: %d\r\n", Error);
	}

	Error = inet_aton(XBIR_NW_DEFAULT_GW_ADDRESS, Gw);
	if (Error == 0) {
		InvalidCfg = TRUE;
		Xbir_Printf("ERROR: Invalid default gateway address: %d\r\n",
			Error);
	}

	if (FALSE == InvalidCfg) {
		Status = XST_SUCCESS;
	}

	return Status;
}

/*****************************************************************************/
/**
 * @brief
 * This function prints the network address
 *
 * @param	Ip	Pointer to IP address
 * @param	Mask	Pointer to network mask
 * @param	Gw	Pointer to gateway address
 *
 * @return	None
 *
 *****************************************************************************/
void Xbir_NwPrintIpCfg (ip_addr_t *Ip, ip_addr_t *Mask, ip_addr_t *Gw)
{
	Xbir_Printf("\r\n-[Network Interface]------------------------\r\n");
	Xbir_Printf("\tBoard IP: %u.%u.%u.%u\r\n", ip4_addr1(Ip), ip4_addr2(Ip),
		ip4_addr3(Ip), ip4_addr4(Ip));
	Xbir_Printf("\tNetmask : %u.%u.%u.%u\r\n", ip4_addr1(Mask), ip4_addr2(Mask),
		ip4_addr3(Mask), ip4_addr4(Mask));
	Xbir_Printf("\tGateway : %u.%u.%u.%u\r\n", ip4_addr1(Gw), ip4_addr2(Gw),
		ip4_addr3(Gw), ip4_addr4(Gw));
	Xbir_Printf("\r\n");
}

/*****************************************************************************/
/**
 * @brief
 * This function receives the input packets and processes them
 *
 * @param	NetIf	Pointer to network interface instance
 *
 * @return	None
 *
 *****************************************************************************/
void Xbir_NwProcessPkts (struct netif* NetIf)
{
	/* Receive and process packets */
	while (TRUE) {
		if (TcpFastTmrFlag) {
			tcp_fasttmr();
			TcpFastTmrFlag = 0U;
		}
		if (TcpSlowTmrFlag) {
			tcp_slowtmr();
			TcpSlowTmrFlag = 0U;
		}
		xemacif_input(NetIf);
	}
}