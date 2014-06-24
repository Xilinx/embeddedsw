#ifndef __XLLTEMACIF_FIFO_H_
#define __XLLTEMACIF_FIFO_H_

#include "xparameters.h"
#include "netif/xlltemacif.h"
#include "xlwipconfig.h"

#ifdef __cplusplus
extern "C" { 
#endif

XStatus init_ll_fifo(struct xemac_s *xemac);
XStatus xllfifo_send(xlltemacif_s *xlltemacif, struct pbuf *p);

#ifdef __cplusplus
}
#endif

#endif
