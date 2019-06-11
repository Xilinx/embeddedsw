/*
 * Copyright (C) 2007 - 2019 Xilinx, Inc.
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

#ifndef __NETIF_XEMACLITEIF_H__
#define __NETIF_XEMACLITEIF_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "lwip/netif.h"
#include "netif/etharp.h"
#include "netif/xpqueue.h"
#include "xemaclite.h"
#include "xemaclite_i.h"
#include "xstatus.h"

/* structure within each netif, encapsulating all information required for
 * using a particular emaclite instance
 */
typedef struct {
        XEmacLite *instance;

	/* queue to store overflow packets */
	pq_queue_t *recv_q;
	pq_queue_t *send_q;
} xemacliteif_s;

void 	xemacliteif_setmac(u32_t index, u8_t *addr);
u8_t*	xemacliteif_getmac(u32_t index);
err_t 	xemacliteif_init(struct netif *netif);
int 	xemacliteif_input(struct netif *netif);

#ifdef __cplusplus
}
#endif

#endif /* __NETIF_XEMACLITEIF_H__ */
