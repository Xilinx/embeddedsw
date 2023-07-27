/******************************************************************************
* Copyright (c) 2019 - 2021 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2023 Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
 *
 * @file xilmailbox_ipips.h
 * @addtogroup xilmailbox Overview
 * @{
 * @details
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date        Changes
 * ----- ---- -------- -------------------------------------------------------
 * 1.0   adk  12/02/19    Initial Release
 * 1.3   sd   03/03/21    Doxygen Fixes
 * 1.6   sd   28/02/21    Add support for microblaze
 * 1.8   ht   07/24/23    Restructure the code for more modularity
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
#ifndef __MICROBLAZE__
#include "xscugic.h"
#endif

/**************************** Type Definitions *******************************/
/**
 * Data structure used to refer Xilmailbox agents
 */
typedef struct {
	XIpiPsu IpiInst; /**< Ipi instance */
#ifndef __MICROBLAZE__
	XScuGic GicInst; /**< Interrupt instance */
#endif
	u32 SourceId; /**< Source id */
	u32 RemoteId; /**< Remote id */
} XMailbox_Agent; /**< Xilmailbox agent */
/************************** Constant Definitions *****************************/
#define BIT(x)                 	(1 << (x)) /**< Bit position */
#define XIPI_DONE_TIMEOUT_VAL	2000000    /**< Timeout for IPI */
#define XIPI_IPI_DONE_BIT_SLEEP_IN_US	(1U)	/**< Sleep for 1Us */

#ifdef versal
#define XMAILBOX_IPIPSM		BIT(0)  /**< PSM channel */
#define XMAILBOX_IPIPMC		BIT(1)	/**< PMC channel */
#define XMAILBOX_IPI0		BIT(2)  /**< IPI0 channel */
#define XMAILBOX_IPI1		BIT(3)  /**< IPI1 channel */
#define XMAILBOX_IPI2		BIT(4)  /**< IPI2 channel */
#define XMAILBOX_IPI3		BIT(5)  /**< IPI3 channel */
#define XMAILBOX_IPI4		BIT(6)  /**< IPI4 channel */
#define XMAILBOX_IPI5		BIT(7)  /**< IPI5 channel */
#define XMAILBOX_IPIPMC_NOBUF	BIT(8)  /**< PMC bufferless channel */
#define XMAILBOX_IPI6_NOBUF	BIT(9)  /**< IPI6 bufferless channel */
#define XMAILBOX_MAX_CHANNELS	10U     /**< Maximum channels */
#define XMAILBOX_INTR_ID	42U     /**< Interrupt id */
#else
#define XMAILBOX_IPI0		BIT(0)  /**< IPI0 channel */
#define XMAILBOX_IPI1		BIT(8)  /**< IPI1 channel */
#define XMAILBOX_IPI2		BIT(9)  /**< IPI2 channel */
#define XMAILBOX_IPI3		BIT(16) /**< IPI3 channel */
#define XMAILBOX_IPI4		BIT(17) /**< IPI4 channel */
#define XMAILBOX_IPI5		BIT(18) /**< IPI5 channel */
#define XMAILBOX_IPI6		BIT(19) /**< IPI6 channel */
#define XMAILBOX_IPI7		BIT(24) /**< IPI7 channel */
#define XMAILBOX_IPI8		BIT(25) /**< IPI8 channel */
#define XMAILBOX_IPI9		BIT(26) /**< IPI9 channel */
#define XMAILBOX_IPI10		BIT(27) /**< IPI10 channel */
#define XMAILBOX_MAX_CHANNELS	11U     /**< Maximum channel */
#define XMAILBOX_INTR_ID	22U     /**< Interrupt id */
#endif

/* Error Handling */
#ifdef versal
#define IPI_BASEADDRESS		0xFF300000U  /**< Base address */
#define IPI_ADDRDECODE_ERROR	BIT(0)       /**< Address decode error */
#define IPI_MID_MISS_ERROR	BIT(1)       /**< Master id not found */
#define IPI_MID_RO_ERROR	BIT(2)       /**< Read permission error */
#define IPI_MID_PARITY_ERROR	BIT(3)	     /**< Master id parity error */
#define IPI_APER_PERM_ERROR	BIT(4)	     /**< Master id access violation */
#define IPI_APER_TZ_ERROR	BIT(5)       /**< Trustzone violation */
#define IPI_APER_ECC_UE_ERROR	BIT(6)       /**< ECC uncorrectable error */
#define IPI_APER_ECC_CE_ERROR	BIT(7)       /**< ECC correctable error */
#else
#define IPI_BASEADDRESS		0xFF380000U  /**< Base Address */
#define IPI_ADDRDECODE_ERROR	BIT(0)       /**< Address decode error */
#endif

/************************** Function Prototypes ******************************/

#ifdef __cplusplus
}
#endif

#endif /* XILMAILBOX_IPIPS_H */
