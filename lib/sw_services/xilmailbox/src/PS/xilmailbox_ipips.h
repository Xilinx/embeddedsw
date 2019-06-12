/******************************************************************************
 * Copyright (C) 2019 Xilinx, Inc.  All rights reserved.
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
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMANGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 *
 *
 *
 ******************************************************************************/
/*****************************************************************************/
/**
 *
 * @file xilmailbox_ipips.h
 * @addtogroup xilmailbox_v1_0
 * @{
 * @details
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date        Changes
 * ----- ---- -------- -------------------------------------------------------
 * 1.0   adk  12/02/19    Initial Release
 *</pre>
 *
 *@note
 *****************************************************************************/
#ifndef XILMAILBOX_IPIPS_H
#define XILMAILBOX_IPIPS_H

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/
#include "xilmailbox.h"
#include "xipipsu.h"
#include "xscugic.h"

/**************************** Type Definitions *******************************/
typedef struct {
	XIpiPsu IpiInst;
	XScuGic GicInst;
	u32 SourceId;
	u32 RemoteId;
} XMailbox_Agent;
/************************** Constant Definitions *****************************/
#define BIT(x)                 	(1 << (x))
#define XIPI_DONE_TIMEOUT_VAL	3000000

#ifdef versal
#define XMAILBOX_IPIPSM		BIT(0)
#define XMAILBOX_IPIPMC		BIT(1)
#define XMAILBOX_IPI0		BIT(2)
#define XMAILBOX_IPI1		BIT(3)
#define XMAILBOX_IPI2		BIT(4)
#define XMAILBOX_IPI3		BIT(5)
#define XMAILBOX_IPI4		BIT(6)
#define XMAILBOX_IPI5		BIT(7)
#define XMAILBOX_IPIPMC_NOBUF	BIT(8)
#define XMAILBOX_IPI6_NOBUF	BIT(9)
#define XMAILBOX_MAX_CHANNELS	10U
#define XMAILBOX_INTR_ID	42U
#else
#define XMAILBOX_IPI0		BIT(0)
#define XMAILBOX_IPI1		BIT(8)
#define XMAILBOX_IPI2		BIT(9)
#define XMAILBOX_IPI3		BIT(16)
#define XMAILBOX_IPI4		BIT(17)
#define XMAILBOX_IPI5		BIT(18)
#define XMAILBOX_IPI6		BIT(19)
#define XMAILBOX_IPI7		BIT(24)
#define XMAILBOX_IPI8		BIT(25)
#define XMAILBOX_IPI9		BIT(26)
#define XMAILBOX_IPI10		BIT(27)
#define XMAILBOX_MAX_CHANNELS	11U
#define XMAILBOX_INTR_ID	22U
#endif

/* Error Handling */
#ifdef versal
#define IPI_BASEADDRESS		0xFF300000U
#define IPI_ADDRDECODE_ERROR	BIT(0)
#define IPI_MID_MISS_ERROR	BIT(1)
#define IPI_MID_RO_ERROR	BIT(2)
#define IPI_MID_PARITY_ERROR	BIT(3)
#define IPI_APER_PERM_ERROR	BIT(4)
#define IPI_APER_TZ_ERROR	BIT(5)
#define IPI_APER_ECC_UE_ERROR	BIT(6)
#define IPI_APER_ECC_CE_ERROR	BIT(7)
#else
#define IPI_BASEADDRESS		0xFF380000U
#define IPI_ADDRDECODE_ERROR	BIT(0)
#endif

/************************** Function Prototypes ******************************/

#ifdef __cplusplus
}
#endif

#endif /* XILMAILBOX_IPIPS_H */
