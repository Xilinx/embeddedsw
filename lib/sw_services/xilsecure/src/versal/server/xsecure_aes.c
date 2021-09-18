/******************************************************************************
* Copyright (c) 2019 - 2021 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
*
* @file xsecure_aes.c
*
* This file contains AES hardware interface APIs
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- ---------- -------------------------------------------------------
* 4.0   vns  04/24/2019 Initial release
* 4.1   vns  08/06/2019 Added AES encryption APIs
*       har  08/21/2019 Fixed MISRA C violations
*       vns  08/23/2019 Initialized status variables
* 4.2   har  01/03/2020 Added checks for return value of XSecure_SssAes
*       vns  02/10/2020 Added DPA CM enable/disable function
*       rpo  02/27/2020 Removed function prototype and static keyword of
*                       XSecure_AesKeyLoad, XSecure_AesWaitForDone functions
*       har  03/01/2020 Added code to soft reset once key decryption is done
*       rpo  03/23/2020 Replaced timeouts with WaitForEvent and code clean up
*       rpo  04/02/2020 Added Crypto KAT APIs
*                       Added support of release and set reset for AES
*       bvi  04/07/2020 Renamed csudma as pmcdma
*       vns  04/12/2020 Reset Versal key clear register in AES initialize call
* 4.3   ana  06/04/2020 Updated NextBlkLen in Xsecure_Aes structure wherever required
*                       Updated Aes state
*       kpt  06/29/2020 Added asserts for input arguments and minor
*                       enhancements on AES state
*       kpt  07/03/2020 Added type casting for the arguments in
*                       XPmcDma_64BitTransfer
*       kpt  07/08/2020 Removed dummy code and Status value reinitialized
*                       to XST_FAILURE
*       har  07/12/2020 Removed magic number from XSecure_AesKeyZero
*       har  07/21/2020 Corrected input parameters for config in
*                       XSecure_AesCfgKupKeynIv
*       kpt  08/06/2020 Replaced magic numbers with macro's
*       kpt  08/18/2020 Added volatile keyword to status variable in case of
*                       status reset
*       td   08/19/2020 Fixed MISRA C violations Rule 10.3
*       rpo  09/10/2020 Asserts are not compiled by default for
*                       secure libraries
*       rpo  09/21/2020 New error code added for crypto state mismatch
*       am   09/24/2020 Resolved MISRA C violations
*       har  09/30/2020 Added blind-write checks for XSecure_AesCfgKupKeynIv
*                       Deprecated Family Key support
*                       Replaced repetitive code for DMA configuration with
*                       XSecure_AesPmcDmaCfgByteSwap function
*       har  10/12/2020 Addressed security review comments
*       am   10/10/2020 Resolved Coverity warnings
* 4.5   am   11/24/2020 Resolved MISRA C and Coverity warnings
*       har  02/12/2021 Separated input validation checks for Instance pointer
*       har  03/02/2021 Added support for AES AAD
*       kpt  03/21/2021 Added volatile keyword for SStatus variable in
*                       XSecure_AesDecryptKat to avoid compiler optimization.
*       am   05/21/2021 Resolved MISRA C violations
* 4.6   har  07/14/2021 Fixed doxygen warnings
*       kpt  07/15/2021 Added 64bit support for XSecure_AesWriteKey
*       kal  08/19/2021 Renamed XSecure_AesPmcDmaCfgByteSwap to
*                       XSecure_AesPmcDmaCfgAndXfer
*       har  08/23/2021 Updated AAD size check
*       kpt  09/18/2021 Added redundancy in XSecure_AesSetDpaCm
*
* </pre>
*
* @note
*
******************************************************************************/

/***************************** Include Files *********************************/
#include "xsecure_utils.h"
#include "xsecure_aes.h"
#include "xsecure_aes_core_hw.h"
#include "xil_util.h"
#include "xsecure_error.h"
#include "xsecure_cryptochk.h"

/************************** Constant Definitions *****************************/
#define XSECURE_MAX_KEY_SOURCES			XSECURE_AES_EXPANDED_KEYS
										/**< Max key source value */

#define XSECURE_KEK_DEC_ENABLE			(0x1U)	/**< Triggers decryption operation
											for black key */

#define XSECURE_AES_DISABLE_KUP_IV_UPDATE	(0x0U)	/**< Disables IV and Key save
											features for KUP */
#define XSECURE_AES_ENABLE_KUP_IV_UPDATE	(0x1U)	/**< Enables IV and Key save
											features for KUP */

#define XSECURE_ENABLE_BYTE_SWAP		(0x1U)	/**< Enables data swap in AES */
#define XSECURE_DISABLE_BYTE_SWAP		(0x0U)	/**< Disables data swap in AES */

/**
 * @name AES KAT parameters
 * @{
 */
/**< AES KAT parameters */
#define XSECURE_KAT_IV_SIZE_IN_WORDS		(4U)
#define XSECURE_KAT_MSG_SIZE_IN_WORDS		(4U)
#define XSECURE_KAT_GCMTAG_SIZE_IN_WORDS	(4U)
#define XSECURE_KAT_AES_SPLIT_DATA_SIZE		(4U)
#define XSECURE_KAT_KEY_SIZE_IN_WORDS		(8U)
#define XSECURE_KAT_OPER_DATA_SIZE_IN_WORDS	(16U)
/** @} */

#define XSECURE_AES_AAD_ENABLE			(0x1U)	/**< Enables authentication of
													data pushed in AES engine*/
#define XSECURE_AES_AAD_DISABLE			(0x0U)	/**< Disables authentication of
													data pushed in AES engine*/

static const u32 KatKey[XSECURE_KAT_KEY_SIZE_IN_WORDS] =
			  {0xD55455D7U, 0x2B247897U, 0xC4BF1CDU , 0x1A2D14EDU,
			   0x4D3B0A53U, 0xF3C6E1AEU, 0xAFC2447AU, 0x7B534D99U};
static const u32 KatIv[XSECURE_KAT_IV_SIZE_IN_WORDS] =
			  {0xCCF8E3B9U, 0x11F11746U, 0xD58C03AFU, 0x00000000U};
static const u32 KatMessage[XSECURE_KAT_MSG_SIZE_IN_WORDS] =
			  {0xF9ECC5AEU, 0x92B9B870U, 0x31299331U, 0xC4182756U};
static const u32 KatGcmTag[XSECURE_KAT_GCMTAG_SIZE_IN_WORDS] =
			  {0xC3CFB3E5U, 0x49D4FBCAU, 0xD90B2BFCU, 0xC87DBE9BU};
static const u32 KatOutput[XSECURE_KAT_MSG_SIZE_IN_WORDS] =
			  {0x9008CFD4U, 0x3882AA74U, 0xD635531U,  0x6C1C1F47U};

static const u32 Key0[XSECURE_KAT_KEY_SIZE_IN_WORDS] =
							{0x98076956U, 0x4f158c97U, 0x78ba50f2U, 0x5f7663e4U,
                             0x97e60c2fU, 0x1b55a409U, 0xdd3acbd8U, 0xb687a0edU};

static const u32 Data0[XSECURE_KAT_OPER_DATA_SIZE_IN_WORDS] =
							  {0U, 0U, 0U, 0U, 0x86c237cfU, 0xead48ac1U,
                               0xa0a60b3dU, 0U, 0U, 0U, 0U, 0U, 0x2481322dU,
                               0x568dd5a8U, 0xed5e77d0U, 0x881ade93U};

static const u32 Key1[XSECURE_KAT_KEY_SIZE_IN_WORDS] =
							{0x3ba3028aU, 0x84e787dfU, 0xe38a7a5dU, 0x707e72c8U,
                             0x8cd04f4fU, 0x2883201fU, 0xa5b38c2dU, 0xe9deced3U};

static const u32 Data1[XSECURE_KAT_OPER_DATA_SIZE_IN_WORDS] =
							{0x0U, 0x0U, 0x0U, 0x0U, 0x96589f59U, 0x8e961c85U,
                               0x3b3208d8U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U,
                               0x328bde4aU, 0xfb2367d5U, 0x40ce658fU, 0xc9275e82U};

static const u32 Ct0[XSECURE_KAT_AES_SPLIT_DATA_SIZE] =
							{0x67020a3bU, 0x3adeecf6U, 0x0309b378U, 0x6ecad4ebU};
static const u32 Ct1[XSECURE_KAT_AES_SPLIT_DATA_SIZE] =
							{0x793391cbU, 0x6575906bU, 0x1a424078U, 0x632b0246U};
static const u32 MiC0[XSECURE_KAT_AES_SPLIT_DATA_SIZE] =
							{0x6400d21fU, 0x6363fc09U, 0x06d4f379U, 0x8809ca7eU};
static const u32 MiC1[XSECURE_KAT_AES_SPLIT_DATA_SIZE] =
							{0x3c459ea7U, 0x5a8aad6fU, 0x878e2a4cU, 0x887f1c82U};

/**************************** Type Definitions *******************************/
typedef struct {
	u32 RegOffset;	/**< Register offset for key source */
	u32 KeySrcSelVal;	/**< Selection value for key source */
	u8  UsrWrAllowed;	/**< User write allowed or not for key source */
	u8  DecAllowed;		/**< Decryption allowed or not for key source */
	u8  EncAllowed;		/**< Encryption allowed or not for key source */
	u8  KeyDecSrcAllowed;	/**< Key decryption source allowed */
	u32 KeyDecSrcSelVal;	/**< Selection value for key decryption source*/
	u32 KeyClearVal;	/**< Key source clear value*/
} XSecure_AesKeyLookup;

typedef struct {
	u64 SrcDataAddr;	/**< Address of source buffer */
	u64 DestDataAddr;	/**< Address of destination buffer */
	u8 SrcChannelCfg;	/**< DMA Source channel configuration */
	u8 DestChannelCfg;	/**< DMA destination channel configuration  */
	u8 IsLastChunkSrc;	/**< Flag for last update in source */
	u8 IsLastChunkDest;	/**< Flag for last update in destination */
} XSecure_AesDmaCfg;

/***************** Macros (Inline Functions) Definitions *********************/
/************************** Function Prototypes ******************************/

static int XSecure_AesWaitForDone(const XSecure_Aes *InstancePtr);
static int XSecure_AesKeyLoad(const XSecure_Aes *InstancePtr,
	XSecure_AesKeySrc KeySrc, XSecure_AesKeySize KeySize);
static void XSecure_AesPmcDmaCfgEndianness(XPmcDma *InstancePtr,
	XPmcDma_Channel Channel, u8 EndianType);
static int XSecure_AesKekWaitForDone(const XSecure_Aes *InstancePtr);
static int XSecure_AesDpaCmDecryptKat(const XSecure_Aes *AesInstance,
	const u32 *KeyPtr, const u32 *DataPtr, u32 *OutputPtr);
static int XSecure_AesOpInit(const XSecure_Aes *InstancePtr,
	XSecure_AesKeySrc KeySrc, XSecure_AesKeySize KeySize, u64 IvAddr);
static int XSecure_AesPmcDmaCfgAndXfer(const XSecure_Aes *InstancePtr,
	XSecure_AesDmaCfg AesDmaCfg, u32 Size);

/************************** Variable Definitions *****************************/
static const XSecure_AesKeyLookup AesKeyLookupTbl [XSECURE_MAX_KEY_SOURCES] =
{
	/* BBRAM_KEY */
	{ XSECURE_AES_INVALID_CFG,
	  XSECURE_AES_KEY_SEL_BBRAM_KEY,
	  FALSE,
	  TRUE,
	  TRUE,
	  TRUE,
	  XSECURE_AES_INVALID_CFG,
	  XSECURE_AES_INVALID_CFG
	},

	/* BBRAM_RED_KEY */
	{ XSECURE_AES_INVALID_CFG,
	  XSECURE_AES_KEY_SEL_BBRAM_RD_KEY,
	  FALSE,
	  TRUE,
	  TRUE,
	  FALSE,
	  XSECURE_AES_KEY_DEC_SEL_BBRAM_RED,
	  XSECURE_AES_KEY_CLEAR_BBRAM_RED_KEY_MASK
	},

	/* BH_KEY */
	{ XSECURE_AES_BH_KEY_0_OFFSET,
	  XSECURE_AES_KEY_SEL_BH_KEY,
	  TRUE,
	  TRUE,
	  TRUE,
	  TRUE,
	  XSECURE_AES_INVALID_CFG,
	  XSECURE_AES_KEY_CLEAR_BH_KEY_MASK
	},

	/* BH_RED_KEY */
	{ XSECURE_AES_INVALID_CFG,
	  XSECURE_AES_KEY_SEL_BH_RD_KEY,
	  FALSE,
	  TRUE,
	  TRUE,
	  FALSE,
	  XSECURE_AES_KEY_DEC_SEL_BH_RED,
	  XSECURE_AES_KEY_CLEAR_BH_RED_KEY_MASK
	},

	/* EFUSE_KEY */
	{ XSECURE_AES_INVALID_CFG,
	  XSECURE_AES_KEY_SEL_EFUSE_KEY,
	  FALSE,
	  TRUE,
	  TRUE,
	  TRUE,
	  XSECURE_AES_INVALID_CFG,
	  XSECURE_AES_KEY_CLEAR_EFUSE_KEY_MASK
	},

	/* EFUSE_RED_KEY */
	{ XSECURE_AES_INVALID_CFG,
	  XSECURE_AES_KEY_SEL_EFUSE_RED_KEY,
	  FALSE,
	  TRUE,
	  TRUE,
	  FALSE,
	  XSECURE_AES_KEY_DEC_SEL_EFUSE_RED,
	  XSECURE_AES_KEY_CLEAR_EFUSE_RED_KEY_MASK
	},

	/* EFUSE_USER_KEY_0 */
	{ XSECURE_AES_INVALID_CFG,
	  XSECURE_AES_KEY_SEL_EFUSE_USR_KEY0,
	  FALSE,
	  TRUE,
	  TRUE,
	  TRUE,
	  XSECURE_AES_INVALID_CFG,
	  XSECURE_AES_KEY_CLEAR_EFUSE_USER_KEY_0_MASK
	},

	/* EFUSE_USER_KEY_1 */
	{ XSECURE_AES_INVALID_CFG,
	  XSECURE_AES_KEY_SEL_EFUSE_USR_KEY1,
	  FALSE,
	  TRUE,
	  TRUE,
	  TRUE,
	  XSECURE_AES_INVALID_CFG,
	  XSECURE_AES_KEY_CLEAR_EFUSE_USER_KEY_1_MASK
	},

	/* EFUSE_USER_RED_KEY_0 */
	{ XSECURE_AES_INVALID_CFG,
	  XSECURE_AES_KEY_SEL_EFUSE_USR_RD_KEY0,
	  FALSE,
	  TRUE,
	  TRUE,
	  FALSE,
	  XSECURE_AES_KEY_DEC_SEL_EFUSE_USR0_RED,
	  XSECURE_AES_KEY_CLEAR_EFUSE_USER_RED_KEY_0_MASK
	},

	/* EFUSE_USER_RED_KEY_1 */
	{ XSECURE_AES_INVALID_CFG,
	  XSECURE_AES_KEY_SEL_EFUSE_USR_RD_KEY1,
	  FALSE,
	  TRUE,
	  TRUE,
	  FALSE,
	  XSECURE_AES_KEY_DEC_SEL_EFUSE_USR1_RED,
	  XSECURE_AES_KEY_CLEAR_EFUSE_USER_RED_KEY_1_MASK
	},

	/* KUP_KEY */
	{ XSECURE_AES_INVALID_CFG,
	  XSECURE_AES_KEY_SEL_KUP_KEY,
	  FALSE,
	  TRUE,
	  TRUE,
	  FALSE,
	  XSECURE_AES_INVALID_CFG,
	  XSECURE_AES_KEY_CLEAR_KUP_KEY_MASK
	},

	/* PUF_KEY */
	{ XSECURE_AES_INVALID_CFG,
	  XSECURE_AES_KEY_SEL_PUF_KEY,
	  FALSE,
	  TRUE,
	  TRUE,
	  FALSE,
	  XSECURE_AES_INVALID_CFG,
	  XSECURE_AES_KEY_CLEAR_PUF_KEY_MASK
	},

	/* USER_KEY_0 */
	{ XSECURE_AES_USER_KEY_0_0_OFFSET,
	  XSECURE_AES_KEY_SEL_USR_KEY_0,
	  TRUE,
	  TRUE,
	  TRUE,
	  FALSE,
	  XSECURE_AES_INVALID_CFG,
	  XSECURE_AES_KEY_CLEAR_USER_KEY_0_MASK
	},

	/* USER_KEY_1 */
	{ XSECURE_AES_USER_KEY_1_0_OFFSET,
	  XSECURE_AES_KEY_SEL_USR_KEY_1,
	  TRUE,
	  TRUE,
	  TRUE,
	  FALSE,
	  XSECURE_AES_INVALID_CFG,
	  XSECURE_AES_KEY_CLEAR_USER_KEY_1_MASK
	},

	/* USER_KEY_2 */
	{ XSECURE_AES_USER_KEY_2_0_OFFSET,
	  XSECURE_AES_KEY_SEL_USR_KEY_2,
	  TRUE,
	  TRUE,
	  TRUE,
	  FALSE,
	  XSECURE_AES_INVALID_CFG,
	  XSECURE_AES_KEY_CLEAR_USER_KEY_2_MASK
	},

	/* USER_KEY_3 */
	{ XSECURE_AES_USER_KEY_3_0_OFFSET,
	  XSECURE_AES_KEY_SEL_USR_KEY_3,
	  TRUE,
	  TRUE,
	  TRUE,
	  FALSE,
	  XSECURE_AES_INVALID_CFG,
	  XSECURE_AES_KEY_CLEAR_USER_KEY_3_MASK
	},

	/* USER_KEY_4 */
	{ XSECURE_AES_USER_KEY_4_0_OFFSET,
	  XSECURE_AES_KEY_SEL_USR_KEY_4,
	  TRUE,
	  TRUE,
	  TRUE,
	  FALSE,
	  XSECURE_AES_INVALID_CFG,
	  XSECURE_AES_KEY_CLEAR_USER_KEY_4_MASK
	},

	/* USER_KEY_5 */
	{ XSECURE_AES_USER_KEY_5_0_OFFSET,
	  XSECURE_AES_KEY_SEL_USR_KEY_5,
	  TRUE,
	  TRUE,
	  TRUE,
	  FALSE,
	  XSECURE_AES_INVALID_CFG,
	  XSECURE_AES_KEY_CLEAR_USER_KEY_5_MASK
	},

	/* USER_KEY_6 */
	{ XSECURE_AES_USER_KEY_6_0_OFFSET,
	  XSECURE_AES_KEY_SEL_USR_KEY_6,
	  TRUE,
	  TRUE,
	  TRUE,
	  FALSE,
	  XSECURE_AES_INVALID_CFG,
	  XSECURE_AES_KEY_CLEAR_USER_KEY_6_MASK
	},

	/* USER_KEY_7 */
	{ XSECURE_AES_USER_KEY_7_0_OFFSET,
	  XSECURE_AES_KEY_SEL_USR_KEY_7,
	  TRUE,
	  TRUE,
	  TRUE,
	  FALSE,
	  XSECURE_AES_INVALID_CFG,
	  XSECURE_AES_KEY_CLEAR_USER_KEY_7_MASK
	}
};

/*****************************************************************************/
/**
 * @brief	This function initializes the AES instance pointer
 *
 * @param	InstancePtr 	- Pointer to the XSecure_Aes instance
 * @param	PmcDmaPtr	- Pointer to the XPmcDma instance
 *
 * @return	XST_SUCCESS	- If initialization was successful
 *		XSECURE_AES_INVALID_PARAM - For invalid parameter
 *
 * @note	All the inputs are accepted in little endian format, but AES
 *		engine accepts the data in big endianness, this will be taken
 *		care while passing data to AES engine
 *
 ******************************************************************************/
int XSecure_AesInitialize(XSecure_Aes *InstancePtr, XPmcDma *PmcDmaPtr)
{
	int Status = XST_FAILURE;

	Status = XSecure_CryptoCheck();
	if (Status != XST_SUCCESS) {
		goto END;
	}

	/* Validate the input arguments */
	if ((InstancePtr == NULL) || (PmcDmaPtr == NULL) ||
			(PmcDmaPtr->IsReady != (u32)(XIL_COMPONENT_IS_READY))) {
		Status = (int)XSECURE_AES_INVALID_PARAM;
		goto END;
	}

	/* Initialize the instance */
	InstancePtr->BaseAddress = XSECURE_AES_BASEADDR;
	InstancePtr->PmcDmaPtr = PmcDmaPtr;
	InstancePtr->NextBlkLen = 0U;

	/* Clear all key zeroization register */
	XSecure_WriteReg(InstancePtr->BaseAddress,
		XSECURE_AES_KEY_CLEAR_OFFSET, XSECURE_AES_KEY_CLR_REG_CLR_MASK);

	Status = XSecure_SssInitialize(&(InstancePtr->SssInstance));
	if (Status != XST_SUCCESS) {
		goto END;
	}

	InstancePtr->AesState = XSECURE_AES_INITIALIZED;

	Status = XST_SUCCESS;

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function enables or disables DPA counter measures in AES engine
 * 		based on user input
 *
 * @param	InstancePtr - Pointer to the XSecure_Aes instance
 * @param	DpaCmCfg    - User choice to enable/disable DPA CM
 *			    - TRUE  - to enable AES DPA counter measure
 *				     (Default setting)
 *			    - FALSE - to disable AES DPA counter measure
 *
 * @return	- XST_SUCCESS - If configuration is success
 *		- XSECURE_AES_INVALID_PARAM        - For invalid parameter
 *		- XSECURE_AES_STATE_MISMATCH_ERROR - If State mismatch is occurred
 *		- XSECURE_AES_DPA_CM_NOT_SUPPORTED - If DPA CM is disabled on chip
 *		  (Enabling/Disabling in AES engine does not impact functionality)
 *
 ******************************************************************************/
int XSecure_AesSetDpaCm(const XSecure_Aes *InstancePtr, u32 DpaCmCfg)
{
	int Status = XST_FAILURE;
	u32 ReadReg;
	volatile u32 DpaCmCfgEn = DpaCmCfg;
	volatile u32 DpaCmCfgEnTmp = DpaCmCfg;

	/* Validate the input arguments */
	if (InstancePtr == NULL) {
		Status = (int)XSECURE_AES_INVALID_PARAM;
		goto END;
	}

	if (InstancePtr->AesState == XSECURE_AES_UNINITIALIZED) {
		Status = (int)XSECURE_AES_STATE_MISMATCH_ERROR;
		goto END;
	}

	/* Chip has DPA CM support */
	if ((XSecure_In32(XSECURE_EFUSE_SECURITY_MISC1) &
		XSECURE_EFUSE_DPA_CM_DIS_MASK) != XSECURE_EFUSE_DPA_CM_DIS_MASK) {

		if ((DpaCmCfgEn != FALSE) || (DpaCmCfgEnTmp != FALSE)) {
			DpaCmCfgEn = TRUE;
			DpaCmCfgEnTmp = TRUE;
		}
		/* Disable/enable DPA CM inside AES engine */
		XSecure_WriteReg(InstancePtr->BaseAddress,
						XSECURE_AES_CM_EN_OFFSET, (DpaCmCfgEn | DpaCmCfgEnTmp));

		/* Verify status of CM */
		ReadReg = XSecure_ReadReg(InstancePtr->BaseAddress,
				XSECURE_AES_STATUS_OFFSET);
		ReadReg = (ReadReg & XSECURE_AES_STATUS_CM_ENABLED_MASK) >>
					XSECURE_AES_STATUS_CM_ENABLED_SHIFT;
		if (ReadReg == (DpaCmCfgEn | DpaCmCfgEnTmp)) {
			Status = XST_SUCCESS;
		}
	}
	else {
		Status = (int)XSECURE_AES_DPA_CM_NOT_SUPPORTED;
	}

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function writes the key provided into the specified
 *		AES key registers
 *
 * @param	InstancePtr 	- Pointer to the XSecure_Aes instance
 * @param	KeySrc		- Key Source to be selected to which provided
 *				  key should be updated by
 *			  	- XSECURE_AES_USER_KEY_0
 *				- XSECURE_AES_USER_KEY_1
 *				- XSECURE_AES_USER_KEY_2
 *				- XSECURE_AES_USER_KEY_3
 *				- XSECURE_AES_USER_KEY_4
 *				- XSECURE_AES_USER_KEY_5
 *				- XSECURE_AES_USER_KEY_6
 *				- XSECURE_AES_USER_KEY_7
 *				- XSECURE_AES_BH_KEY
 * @param	KeySize		- A variable of type XSecure_AesKeySize, which holds
 *				  the size of the input key to be written by
 *			  	- XSECURE_AES_KEY_SIZE_128 for 128 bit key size
 *				- XSECURE_AES_KEY_SIZE_256 for 256 bit key size
 * @param	KeyAddr		- Address of a buffer which should contain the key
 * 				  to be written
 *
 * @return	- XST_SUCCESS - On successful key written on AES registers
 *		- XSECURE_AES_INVALID_PARAM - For invalid parameter
 *		- XSECURE_AES_STATE_MISMATCH_ERROR - If State mismatch is occurred
 *		- XST_FAILURE - On failure
 *
 ******************************************************************************/
int XSecure_AesWriteKey(const XSecure_Aes *InstancePtr,
	XSecure_AesKeySrc KeySrc, XSecure_AesKeySize KeySize, u64 KeyAddr)
{
	int Status = XST_FAILURE;
	int StatusTmp = XST_FAILURE;
	u32 Offset;
	u32 Index = 0U;
	u32 Key[XSECURE_AES_KEY_SIZE_256BIT_WORDS] = {0U};
	u32 KeySizeInWords;

	/* Validate the input arguments */
	if (InstancePtr == NULL) {
		Status = (int)XSECURE_AES_INVALID_PARAM;
		goto END;
	}

	if ((KeySrc < XSECURE_AES_BBRAM_KEY) ||
		(KeySrc >= XSECURE_MAX_KEY_SOURCES)) {
		Status = (int)XSECURE_AES_INVALID_PARAM;
		goto END;
	}

	if ((AesKeyLookupTbl[KeySrc].UsrWrAllowed != TRUE) ||
		(KeyAddr == 0x00U)) {
		Status = (int)XSECURE_AES_INVALID_PARAM;
		goto END;
	}

	if ((XSECURE_AES_KEY_SIZE_128 != KeySize) &&
		 (XSECURE_AES_KEY_SIZE_256 != KeySize)) {
		Status = (int)XSECURE_AES_INVALID_PARAM;
		goto END;
	}

	if ((InstancePtr->AesState == XSECURE_AES_UNINITIALIZED)) {
		Status = (int)XSECURE_AES_STATE_MISMATCH_ERROR;
		goto END;
	}

	if ((XSECURE_AES_BH_KEY == KeySrc) &&
			(XSECURE_AES_KEY_SIZE_128 == KeySize)) {
		Status = (int)XSECURE_AES_INVALID_PARAM;
		goto END;
	}

	Offset = AesKeyLookupTbl[KeySrc].RegOffset;
	if (Offset == XSECURE_AES_INVALID_CFG) {
		Status = (int)XSECURE_AES_INVALID_PARAM;
		goto END;
	}

	if (XSECURE_AES_KEY_SIZE_128 == KeySize) {
		KeySizeInWords = XSECURE_AES_KEY_SIZE_128BIT_WORDS;
	}
	else {
		KeySizeInWords = XSECURE_AES_KEY_SIZE_256BIT_WORDS;
	}

	XSecure_MemCpy64((u64)(UINTPTR)Key, KeyAddr, KeySizeInWords *
				XSECURE_WORD_SIZE);

	Offset = Offset + (KeySizeInWords * XSECURE_WORD_SIZE) -
				XSECURE_WORD_SIZE;

	for (Index = 0U; Index < KeySizeInWords; Index++) {
		XSecure_WriteReg(InstancePtr->BaseAddress, Offset,
					Xil_Htonl(Key[Index]));
		Offset = Offset - XSECURE_WORD_SIZE;
	}
	Status = XST_SUCCESS;

END:
	StatusTmp = Xil_SecureZeroize((u8*)Key, XSECURE_AES_KEY_SIZE_256BIT_BYTES);
	if (Status == XST_SUCCESS) {
		Status = StatusTmp;
	}

	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function is used to update the AES engine with
		Additional Authenticated Data(AAD).
 *
 * @param	InstancePtr	- Pointer to the XSecure_Aes instance
 * @param	AadAddr		- Address of the additional authenticated data
 * @param	AadSize		- Size of additional authenticated data in bytes,
 *				  whereas number of bytes provided should be
 *				  quad-word aligned(multiples of 16 bytes)
 *
 * @return	- XST_SUCCESS - On successful update of AAD
 *		- XSECURE_AES_INVALID_PARAM - On invalid parameter
 *		- XSECURE_AES_STATE_MISMATCH_ERROR - If State mismatch occurs
 *		- XST_FAILURE - On failure to update AAD
 *
 * @note	The API must be called after XSecure_AesEncryptInit() or
 *		XSecure_AesDecryptInit()
 *
 ******************************************************************************/
int XSecure_AesUpdateAad(XSecure_Aes *InstancePtr, u64 AadAddr, u32 AadSize)
{
	int Status = XST_FAILURE;
	XSecure_AesDmaCfg AesDmaCfg = {0U};

	/* Validate the input arguments */
	if (InstancePtr == NULL) {
		Status = (int)XSECURE_AES_INVALID_PARAM;
		goto END;
	}

	if ((AadAddr == 0x00U) || ((AadSize % XSECURE_QWORD_SIZE) != 0x00U)) {
		Status = (int)XSECURE_AES_INVALID_PARAM;
		goto END_RST;
	}

	if ((InstancePtr->AesState != XSECURE_AES_ENCRYPT_INITIALIZED) &&
		(InstancePtr->AesState != XSECURE_AES_DECRYPT_INITIALIZED)) {
		Status = (int)XSECURE_AES_STATE_MISMATCH_ERROR;
		goto END_RST;
	}

	XSecure_WriteReg(InstancePtr->BaseAddress, XSECURE_AES_AAD_OFFSET,
		XSECURE_AES_AAD_ENABLE);

	AesDmaCfg.SrcDataAddr = AadAddr;
	AesDmaCfg.SrcChannelCfg = TRUE;
	AesDmaCfg.IsLastChunkSrc = FALSE;

	Status = XSecure_AesPmcDmaCfgAndXfer(InstancePtr, AesDmaCfg, AadSize);
	if (Status != XST_SUCCESS) {
		goto END_AAD;
	}

END_AAD:
	/* Clear endianness */
	XSecure_AesPmcDmaCfgEndianness(InstancePtr->PmcDmaPtr,
		XPMCDMA_SRC_CHANNEL, XSECURE_DISABLE_BYTE_SWAP);
	XSecure_AesPmcDmaCfgEndianness(InstancePtr->PmcDmaPtr,
		XPMCDMA_DST_CHANNEL, XSECURE_DISABLE_BYTE_SWAP);
	/* Disable AAD */
	XSecure_WriteReg(InstancePtr->BaseAddress, XSECURE_AES_AAD_OFFSET,
		XSECURE_AES_AAD_DISABLE);

END_RST:
	if (Status != XST_SUCCESS) {
		InstancePtr->AesState = XSECURE_AES_INITIALIZED;
		XSecure_SetReset(InstancePtr->BaseAddress,
			XSECURE_AES_SOFT_RST_OFFSET);
	}
END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function decrypts the key which is in KEK/Obfuscated key form
 * 		exists and not exist in either of the boot header/Efuse/BBRAM and
 * 		updates the mentioned destination red key register with
 * 		corresponding red key
 *
 * @param	InstancePtr	- Pointer to the XSecure_Aes instance
 * @param	DecKeySrc	- Select key source which holds KEK and needs to be
 *				  decrypted
 * @param	DstKeySrc	- Select the key in which decrypted red key should be
 * 				  updated
 * @param	IvAddr		- Address of IV holding buffer for decryption
 * 				  of the key
 * @param	KeySize		- A variable of type XSecure_AesKeySize, which
 * 				  specifies the size of the key to be
 *				- XSECURE_AES_KEY_SIZE_128 for 128 bit key size
 *				- XSECURE_AES_KEY_SIZE_256 for 256 bit key size
 *
 * @return	- XST_SUCCESS - On successful key decryption
 *		- XSECURE_AES_INVALID_PARAM - On invalid parameter
 *		- XST_FAILURE - If timeout has occurred
 *
 ******************************************************************************/
int XSecure_AesKekDecrypt(const XSecure_Aes *InstancePtr,
	XSecure_AesKeySrc DecKeySrc, XSecure_AesKeySrc DstKeySrc, u64 IvAddr,
	XSecure_AesKeySize KeySize)
{
	volatile int Status = XST_FAILURE;
	XSecure_AesKeySrc KeySrc;
	XSecure_AesDmaCfg AesDmaCfg = {0U};

	/* Validate the input arguments */
	if (InstancePtr == NULL) {
		Status = (int)XSECURE_AES_INVALID_PARAM;
		goto END;
	}

	if (IvAddr == 0x00U) {
		Status = (int)XSECURE_AES_INVALID_PARAM;
		goto END_RST;
	}

	if ((DecKeySrc >= XSECURE_MAX_KEY_SOURCES) ||
		(DecKeySrc < XSECURE_AES_BBRAM_KEY)) {
		Status = (int)XSECURE_AES_INVALID_PARAM;
		goto END_RST;
	}

	if ((XSECURE_AES_KEY_SIZE_128 != KeySize) &&
		 (XSECURE_AES_KEY_SIZE_256 != KeySize)) {
		Status = (int)XSECURE_AES_INVALID_PARAM;
		goto END_RST;
	}

	if ((AesKeyLookupTbl[DecKeySrc].KeyDecSrcAllowed != TRUE) ||
	    (AesKeyLookupTbl[DstKeySrc].KeyDecSrcSelVal ==
				XSECURE_AES_INVALID_CFG)) {
		Status = (int)XSECURE_AES_INVALID_PARAM;
		goto END_RST;
	}

	if ((InstancePtr->AesState == XSECURE_AES_UNINITIALIZED)) {
		Status = (int)XSECURE_AES_STATE_MISMATCH_ERROR;
		goto END_RST;
	}

	XSecure_ReleaseReset(InstancePtr->BaseAddress,
		XSECURE_AES_SOFT_RST_OFFSET);

	KeySrc = XSECURE_AES_PUF_KEY;

	Status = XST_FAILURE;

	Status = XSecure_AesKeyLoad(InstancePtr, KeySrc, KeySize);
	if (Status != XST_SUCCESS) {
		goto END_RST;
	}

	/* Start the message. */
	XSecure_WriteReg(InstancePtr->BaseAddress,
			XSECURE_AES_START_MSG_OFFSET,
			XSECURE_AES_START_MSG_VAL_MASK);

	/* Enable Byte swap */
	XSecure_WriteReg(InstancePtr->BaseAddress,
		XSECURE_AES_DATA_SWAP_OFFSET, XSECURE_ENABLE_BYTE_SWAP);

	Status = XST_FAILURE;

	AesDmaCfg.SrcChannelCfg = TRUE;
	AesDmaCfg.SrcDataAddr = IvAddr;
	AesDmaCfg.IsLastChunkSrc = TRUE;

	Status = XSecure_AesPmcDmaCfgAndXfer(InstancePtr, AesDmaCfg,
		XSECURE_SECURE_GCM_TAG_SIZE);
	if (Status != XST_SUCCESS) {
		goto END_RST;
	}

	XSecure_WriteReg(InstancePtr->BaseAddress,
			XSECURE_AES_DATA_SWAP_OFFSET, XSECURE_DISABLE_BYTE_SWAP);

	/* Select key decryption */
	XSecure_WriteReg(InstancePtr->BaseAddress,
		XSECURE_AES_KEY_DEC_OFFSET, XSECURE_AES_KEY_DEC_MASK);

	/* Decrypt selection */
	XSecure_WriteReg(InstancePtr->BaseAddress,
		XSECURE_AES_KEY_DEC_SEL_OFFSET,
		AesKeyLookupTbl[DstKeySrc].KeyDecSrcSelVal);

	XSecure_WriteReg(InstancePtr->BaseAddress,
		XSECURE_AES_KEY_SEL_OFFSET,
		AesKeyLookupTbl[DecKeySrc].KeySrcSelVal);

	XSecure_WriteReg(InstancePtr->BaseAddress,
			XSECURE_AES_KEY_DEC_TRIG_OFFSET, XSECURE_KEK_DEC_ENABLE);

	Status = XST_FAILURE;

	/* Wait for AES Decryption completion. */
	Status = XSecure_AesKekWaitForDone(InstancePtr);

END_RST:
	/* Select key decryption */
	XSecure_WriteReg(InstancePtr->BaseAddress,
			XSECURE_AES_KEY_DEC_OFFSET, XSECURE_AES_KEY_DEC_RESET_MASK);

	XSecure_SetReset(InstancePtr->BaseAddress,
		XSECURE_AES_SOFT_RST_OFFSET);

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function initializes the AES engine for decryption
 *
 * @param	InstancePtr	- Pointer to the XSecure_Aes instance
 * @param	KeySrc		- Key Source for decryption of the data
 * @param	KeySize		- Size of the AES key to be used for decryption is
 *		 		- XSECURE_AES_KEY_SIZE_128 for 128 bit key size
 *				- XSECURE_AES_KEY_SIZE_256 for 256 bit key size
 * @param	IvAddr		- Address to the buffer holding IV
 *
 * @return	- XST_SUCCESS - On successful init
 *		- XSECURE_AES_INVALID_PARAM - On invalid parameter
 *		- XSECURE_AES_STATE_MISMATCH_ERROR - If State mismatch is occurred
 *		- XST_FAILURE - On failure to configure switch
 *
 ******************************************************************************/
int XSecure_AesDecryptInit(XSecure_Aes *InstancePtr, XSecure_AesKeySrc KeySrc,
	XSecure_AesKeySize KeySize, u64 IvAddr)
{
	int Status = XST_FAILURE;

	/* Validate the input arguments */
	if (InstancePtr == NULL) {
		Status = (int)XSECURE_AES_INVALID_PARAM;
		goto END;
	}

	if ((KeySrc >= XSECURE_MAX_KEY_SOURCES) ||
		(KeySrc < XSECURE_AES_BBRAM_KEY) || (IvAddr == 0x00U)) {
		Status = (int)XSECURE_AES_INVALID_PARAM;
		goto END_RST;
	}

	if ((XSECURE_AES_KEY_SIZE_128 != KeySize) &&
		 (XSECURE_AES_KEY_SIZE_256 != KeySize)) {
		Status = (int)XSECURE_AES_INVALID_PARAM;
		goto END_RST;
	}

	if (InstancePtr->AesState == XSECURE_AES_UNINITIALIZED) {
		Status = (int)XSECURE_AES_STATE_MISMATCH_ERROR;
		goto END_RST;
	}

	if(InstancePtr->NextBlkLen == 0U) {
		XSecure_ReleaseReset(InstancePtr->BaseAddress,
			XSECURE_AES_SOFT_RST_OFFSET);
	}

	/* Key selected does not allow decryption */
	if (AesKeyLookupTbl[KeySrc].DecAllowed == FALSE) {
		Status = XST_FAILURE;
		goto END_RST;
	}

	/* Configure AES for decryption */
	XSecure_WriteReg(InstancePtr->BaseAddress,
			XSECURE_AES_MODE_OFFSET, XSECURE_AES_MODE_DEC);

	Status = XSecure_AesOpInit(InstancePtr, KeySrc, KeySize, IvAddr);
	if (Status != XST_SUCCESS) {
		goto END_RST;
	}
	/* Update the state */
	InstancePtr->AesState = XSECURE_AES_DECRYPT_INITIALIZED;

	Status = XST_SUCCESS;

END_RST:
	if (Status != XST_SUCCESS) {
		/*
		 * Issue a soft to reset to AES engine and
		 * set the AES state back to initilization state
		 */
		InstancePtr->NextBlkLen = 0U;
		InstancePtr->AesState = XSECURE_AES_INITIALIZED;
		XSecure_SetReset(InstancePtr->BaseAddress,
			XSECURE_AES_SOFT_RST_OFFSET);
	}

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function is used to update the AES engine for decryption with
 * 		provided data and stores the decrypted data at specified address
 *
 * @param	InstancePtr	- Pointer to the XSecure_Aes instance
 * @param	InDataAddr	- Address of the encrypted data which needs to be
 *				  decrypted
 * @param	OutDataAddr	- Address of output buffer where the decrypted
 *				  to be updated
 * @param	Size		- Size of data to be decrypted in bytes, whereas
 *			 	  number of bytes provided should be multiples of 4
 * @param	IsLastChunk	- If this is the last update of data to be decrypted,
 *				  this parameter should be set to TRUE otherwise FALSE
 *
 * @return	- XST_SUCCESS - On successful decryption of the data
 *		- XSECURE_AES_INVALID_PARAM - On invalid parameter
 *		- XSECURE_AES_STATE_MISMATCH_ERROR - If State mismatch is occurred
 *		- XST_FAILURE - On failure to configure switch
 *
 ******************************************************************************/
int XSecure_AesDecryptUpdate(XSecure_Aes *InstancePtr, u64 InDataAddr,
	u64 OutDataAddr, u32 Size, u8 IsLastChunk)
{
	int Status = XST_FAILURE;
	XSecure_AesDmaCfg AesDmaCfg = {0U};

	/* Validate the input arguments */
	if (InstancePtr == NULL) {
		Status = (int)XSECURE_AES_INVALID_PARAM;
		goto END;
	}

	if (((IsLastChunk != TRUE) && (IsLastChunk != FALSE)) ||
		((Size % XSECURE_WORD_SIZE) != 0x00U)) {
		Status = (int)XSECURE_AES_INVALID_PARAM;
		goto END_RST;
	}

	if (InstancePtr->AesState != XSECURE_AES_DECRYPT_INITIALIZED) {
		Status = (int)XSECURE_AES_STATE_MISMATCH_ERROR;
		goto END_RST;
	}

	AesDmaCfg.SrcDataAddr = InDataAddr;
	AesDmaCfg.DestDataAddr = OutDataAddr;
	AesDmaCfg.SrcChannelCfg = TRUE;
	AesDmaCfg.DestChannelCfg = TRUE;
	AesDmaCfg.IsLastChunkSrc = IsLastChunk;
	AesDmaCfg.IsLastChunkDest = FALSE;

	Status = XSecure_AesPmcDmaCfgAndXfer(InstancePtr, AesDmaCfg, Size);

END_RST:
	/* Clear endianness */
	XSecure_AesPmcDmaCfgEndianness(InstancePtr->PmcDmaPtr,
				XPMCDMA_SRC_CHANNEL, XSECURE_DISABLE_BYTE_SWAP);
	XSecure_AesPmcDmaCfgEndianness(InstancePtr->PmcDmaPtr,
				XPMCDMA_DST_CHANNEL, XSECURE_DISABLE_BYTE_SWAP);
	if (Status != XST_SUCCESS) {
		/*
		 * Issue a soft to reset to AES engine and
		 * set the AES state back to initilization state
		 */
		InstancePtr->NextBlkLen = 0U;
		InstancePtr->AesState = XSECURE_AES_INITIALIZED;
		XSecure_SetReset(InstancePtr->BaseAddress,
			XSECURE_AES_SOFT_RST_OFFSET);
	}

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function verifies the GCM tag provided for the data decrypted
 * 		till the point
 *
 * @param	InstancePtr 	- Pointer to the XSecure_Aes instance
 * @param	GcmTagAddr 	- Address of a buffer which should holds GCM Tag
 *
 * @return	- XST_SUCCESS - On successful GCM tag verification
 * 		- XSECURE_AES_GCM_TAG_MISMATCH - user provided GCM tag does not
 *						 match calculated tag
 *		- XSECURE_AES_INVALID_PARAM - On invalid parameter
 *		- XSECURE_AES_STATE_MISMATCH_ERROR - If State mismatch is occurred
 *		- XST_FAILURE - On failure
 *
 ******************************************************************************/
int XSecure_AesDecryptFinal(XSecure_Aes *InstancePtr, u64 GcmTagAddr)
{
	volatile int Status = XST_FAILURE;
	volatile u32 RegVal;
	volatile u32 RegValTmp;
	XSecure_AesDmaCfg AesDmaCfg = {0U};

	/* Validate the input arguments */
	if (InstancePtr == NULL) {
		Status = (int)XSECURE_AES_INVALID_PARAM;
		goto END;
	}

	if (GcmTagAddr == 0x00U) {
		Status = (int)XSECURE_AES_INVALID_PARAM;
		goto END_RST;
	}

	if (InstancePtr->AesState != XSECURE_AES_DECRYPT_INITIALIZED) {
		Status = (int)XSECURE_AES_STATE_MISMATCH_ERROR;
		goto END_RST;
	}

	XSecure_WriteReg(InstancePtr->BaseAddress,
			XSECURE_AES_DATA_SWAP_OFFSET, XSECURE_ENABLE_BYTE_SWAP);

	AesDmaCfg.SrcChannelCfg = TRUE;
	AesDmaCfg.SrcDataAddr = GcmTagAddr;
	AesDmaCfg.IsLastChunkSrc = FALSE;

	Status = XSecure_AesPmcDmaCfgAndXfer(InstancePtr, AesDmaCfg,
		XSECURE_SECURE_GCM_TAG_SIZE);
	if (Status != XST_SUCCESS) {
		goto END_RST;
	}

	Status = XST_FAILURE;

	/* Wait for AES Decryption completion. */
	Status = XSecure_AesWaitForDone(InstancePtr);
	if (Status != XST_SUCCESS) {
		goto END_RST;
	}

	Status = XST_FAILURE;

	/* Get the AES status to know if GCM check passed. */
	RegVal = XSecure_ReadReg(InstancePtr->BaseAddress,
			XSECURE_AES_STATUS_OFFSET);
	RegValTmp = XSecure_ReadReg(InstancePtr->BaseAddress,
			XSECURE_AES_STATUS_OFFSET);
	RegVal &= XSECURE_AES_STATUS_GCM_TAG_PASS_MASK;
	RegValTmp &= XSECURE_AES_STATUS_GCM_TAG_PASS_MASK;

	if ((RegVal != XSECURE_AES_STATUS_GCM_TAG_PASS_MASK) ||
	   (RegValTmp != XSECURE_AES_STATUS_GCM_TAG_PASS_MASK)) {
		Status = (int)XSECURE_AES_GCM_TAG_MISMATCH;
		goto END_RST;
	}

	Status = XST_FAILURE;

	Status = XSecure_AesGetNxtBlkLen(InstancePtr, &InstancePtr->NextBlkLen);

END_RST:
	/* Clear endianness */
	XSecure_AesPmcDmaCfgEndianness(InstancePtr->PmcDmaPtr, XPMCDMA_SRC_CHANNEL,
		XSECURE_DISABLE_BYTE_SWAP);

	XSecure_WriteReg(InstancePtr->BaseAddress,
			XSECURE_AES_DATA_SWAP_OFFSET, XSECURE_DISABLE_BYTE_SWAP);
	InstancePtr->AesState = XSECURE_AES_INITIALIZED;
	if ((InstancePtr->NextBlkLen == 0U) || (Status != XST_SUCCESS)) {
		/*
		 * Issue a soft to reset to AES engine
		 */
		InstancePtr->NextBlkLen = 0U;
		XSecure_SetReset(InstancePtr->BaseAddress,
			XSECURE_AES_SOFT_RST_OFFSET);
	}

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function decrypts the  size (length) number of bytes of the
 * 		passed in InDataAddr (source) buffer and stores the decrypted data
 * 		in the OutDataAddr (destination) buffer and verifies GcmTagAddr
 *
 * @param	InstancePtr	- Pointer to the XSecure_Aes instance
 * @param	InDataAddr	- Address of the encrypted data which needs to be
 *				  decrypted
 * @param	OutDataAddr	- Address of output buffer where the decrypted to be
 *				  updated
 * @param	Size		- Size of data to be decrypted in bytes, whereas number
 *				  of bytes provided should be multiples of 4
 * @param	GcmTagAddr	- Address of a buffer which should contain GCM Tag
 *
 * @return	- XST_SUCCESS - On successful decryption and GCM tag verification
 *		- XSECURE_AES_INVALID_PARAM - On invalid parameter
 *		- XSECURE_AES_STATE_MISMATCH_ERROR - If State mismatch is occurred
 *		- XST_FAILURE - On failure
 *
 ******************************************************************************/
int XSecure_AesDecryptData(XSecure_Aes *InstancePtr, u64 InDataAddr,
	u64 OutDataAddr, u32 Size, u64 GcmTagAddr)
{
	volatile int Status = XST_FAILURE;

	/* Validate the input arguments */
	if ((InstancePtr == NULL) || ((Size % XSECURE_WORD_SIZE) != 0x00U)) {
		Status = (int)XSECURE_AES_INVALID_PARAM;
		goto END;
	}

	if (GcmTagAddr == 0x00U) {
		Status = (int)XSECURE_AES_INVALID_PARAM;
		goto END;
	}

	if (InstancePtr->AesState != XSECURE_AES_DECRYPT_INITIALIZED) {
		Status = (int)XSECURE_AES_STATE_MISMATCH_ERROR;
		goto END;
	}

	/* Update AES engine with data */
	Status = XSecure_AesDecryptUpdate(InstancePtr, InDataAddr, OutDataAddr,
						Size, TRUE);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = XST_FAILURE;

	/* Verify GCM tag */
	Status = XSecure_AesDecryptFinal(InstancePtr, GcmTagAddr);

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function initializes the AES engine for encryption
 *
 * @param	InstancePtr	- Pointer to the XSecure_Aes instance
 * @param	KeySrc		- Key Source for encryption
 * @param	KeySize		- Size of the AES key to be used for encryption is
 *			 	- XSECURE_AES_KEY_SIZE_128 for 128 bit key size
 *				- XSECURE_AES_KEY_SIZE_256 for 256 bit key size
 * @param	IvAddr		- Address to the buffer holding IV
 *
 * @return	- XST_SUCCESS - On successful initialization
 * 		- XSECURE_AES_INVALID_PARAM - On invalid parameter
 * 		- XSECURE_AES_STATE_MISMATCH_ERROR - If State mismatch is occurred
 * 		- XST_FAILURE - On failure
 *
 ******************************************************************************/
int XSecure_AesEncryptInit(XSecure_Aes *InstancePtr, XSecure_AesKeySrc KeySrc,
	XSecure_AesKeySize KeySize, u64 IvAddr)
{
	int Status = XST_FAILURE;

	/* Validate the input arguments */
	if (InstancePtr == NULL) {
		Status = (int)XSECURE_AES_INVALID_PARAM;
		goto END;
	}

	if ((KeySrc >= XSECURE_MAX_KEY_SOURCES) ||
		(KeySrc < XSECURE_AES_BBRAM_KEY) || (IvAddr == 0x00U)) {
		Status = (int)XSECURE_AES_INVALID_PARAM;
		goto END_RST;
	}

	if ((KeySize != XSECURE_AES_KEY_SIZE_128) &&
		(KeySize != XSECURE_AES_KEY_SIZE_256)) {
		Status = (int)XSECURE_AES_INVALID_PARAM;
		goto END_RST;
	}

	if (InstancePtr->AesState == XSECURE_AES_UNINITIALIZED) {
		Status = (int)XSECURE_AES_STATE_MISMATCH_ERROR;
		goto END_RST;
	}

	XSecure_ReleaseReset(InstancePtr->BaseAddress,
				XSECURE_AES_SOFT_RST_OFFSET);

	/* Key selected does not allow Encryption */
	if (AesKeyLookupTbl[KeySrc].EncAllowed == FALSE) {
		Status = XST_FAILURE;
		goto END_RST;
	}

	/* Configure AES for Encryption */
	XSecure_WriteReg(InstancePtr->BaseAddress,
			XSECURE_AES_MODE_OFFSET, XSECURE_AES_MODE_ENC);

	Status = XSecure_AesOpInit(InstancePtr, KeySrc, KeySize, IvAddr);
	if(Status != XST_SUCCESS) {
		goto END_RST;
	}

	InstancePtr->AesState = XSECURE_AES_ENCRYPT_INITIALIZED;

END_RST:
	if (Status != XST_SUCCESS) {
		InstancePtr->AesState = XSECURE_AES_INITIALIZED;
		XSecure_SetReset(InstancePtr->BaseAddress,
			XSECURE_AES_SOFT_RST_OFFSET);
	}

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function is used to update the AES engine for encryption with
 * 		provided data and stores the decrypted data at specified address
 *
 * @param	InstancePtr	- Pointer to the XSecure_Aes instance
 * @param	InDataAddr	- Address of the encrypted data which needs to be
 *				  encrypted
 * @param	OutDataAddr	- Address of output buffer where the encrypted data
 *				  to be updated
 * @param	Size		- Size of data to be encrypted in bytes, whereas number
 *			 	  of bytes provided should be multiples of 4
 * @param	IsLastChunk	- If this is the last update of data to be encrypted,
 *		 		  this parameter should be set to TRUE otherwise FALSE
 *
 * @return	- XST_SUCCESS - On successful encryption of the data
 * 		- XSECURE_AES_INVALID_PARAM - On invalid parameter
 * 		- XSECURE_AES_STATE_MISMATCH_ERROR - If State mismatch is occurred
 * 		- XST_FAILURE - On failure
 *
 ******************************************************************************/
int XSecure_AesEncryptUpdate(XSecure_Aes *InstancePtr, u64 InDataAddr,
	u64 OutDataAddr, u32 Size, u8 IsLastChunk)
{
	int Status = XST_FAILURE;
	XSecure_AesDmaCfg AesDmaCfg = {0U};

	/* Validate the input arguments */
	if ((InstancePtr == NULL) ) {
		Status = (int)XSECURE_AES_INVALID_PARAM;
		goto END;
	}

	if (((IsLastChunk != TRUE) && (IsLastChunk != FALSE)) ||
		((Size % XSECURE_WORD_SIZE) != 0x00U)) {
		Status = (int)XSECURE_AES_INVALID_PARAM;
		goto END_RST;
	}

	if (InstancePtr->AesState != XSECURE_AES_ENCRYPT_INITIALIZED) {
		Status = (int)XSECURE_AES_STATE_MISMATCH_ERROR;
		goto END_RST;
	}

	AesDmaCfg.SrcDataAddr = InDataAddr;
	AesDmaCfg.DestDataAddr = OutDataAddr;
	AesDmaCfg.SrcChannelCfg = TRUE;
	AesDmaCfg.DestChannelCfg = TRUE;
	AesDmaCfg.IsLastChunkSrc = IsLastChunk;
	AesDmaCfg.IsLastChunkDest = FALSE;

	Status = XSecure_AesPmcDmaCfgAndXfer(InstancePtr, AesDmaCfg, Size);
	if (Status != XST_SUCCESS) {
		goto END_RST;
	}

END_RST:
	/* Clear endianness */
	XSecure_AesPmcDmaCfgEndianness(InstancePtr->PmcDmaPtr,
				XPMCDMA_SRC_CHANNEL, XSECURE_DISABLE_BYTE_SWAP);
	XSecure_AesPmcDmaCfgEndianness(InstancePtr->PmcDmaPtr,
				XPMCDMA_DST_CHANNEL, XSECURE_DISABLE_BYTE_SWAP);
	if (Status != XST_SUCCESS) {
		InstancePtr->AesState = XSECURE_AES_INITIALIZED;
		XSecure_SetReset(InstancePtr->BaseAddress,
			XSECURE_AES_SOFT_RST_OFFSET);
	}

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function updates the GCM tag for the encrypted data
 *
 * @param	InstancePtr	- Pointer to the XSecure_Aes instance
 * @param	GcmTagAddr	- Address to the buffer of GCM tag size, where the API
 *				  updates GCM tag
 *
 * @return	- XST_SUCCESS - On successful GCM tag updation
 *		- XSECURE_AES_INVALID_PARAM - On invalid parameter
 *		- XSECURE_AES_STATE_MISMATCH_ERROR - If State mismatch is occurred
 *		- XST_FAILURE - On failure
 *
 ******************************************************************************/
int XSecure_AesEncryptFinal(XSecure_Aes *InstancePtr, u64 GcmTagAddr)
{
	volatile int Status = XST_FAILURE;
	volatile int SStatus = XST_FAILURE;
	XSecure_AesDmaCfg AesDmaCfg = {0U};

	/* Validate the input arguments */
	if (InstancePtr == NULL) {
		Status = (int)XSECURE_AES_INVALID_PARAM;
		goto END;
	}

	if (GcmTagAddr == 0x00U) {
		Status = (int)XSECURE_AES_INVALID_PARAM;
		goto END_RST;
	}

	if (InstancePtr->AesState != XSECURE_AES_ENCRYPT_INITIALIZED) {
		Status = (int)XSECURE_AES_STATE_MISMATCH_ERROR;
		goto END_RST;
	}

	XSecure_WriteReg(InstancePtr->BaseAddress,
			XSECURE_AES_DATA_SWAP_OFFSET, XSECURE_ENABLE_BYTE_SWAP);

	AesDmaCfg.DestDataAddr = GcmTagAddr;
	AesDmaCfg.DestChannelCfg = TRUE;
	AesDmaCfg.IsLastChunkDest = FALSE;

	Status = XSecure_AesPmcDmaCfgAndXfer(InstancePtr, AesDmaCfg,
		XSECURE_SECURE_GCM_TAG_SIZE);
	if (Status != XST_SUCCESS) {
		goto END_RST;
	}

	Status = XST_FAILURE;

	/* Wait for AES Decryption completion. */
	Status = XSecure_AesWaitForDone(InstancePtr);

END_RST:
	InstancePtr->AesState = XSECURE_AES_INITIALIZED;

	XSecure_AesPmcDmaCfgEndianness(InstancePtr->PmcDmaPtr,
			XPMCDMA_DST_CHANNEL, XSECURE_DISABLE_BYTE_SWAP);

	XSecure_WriteReg(InstancePtr->BaseAddress,
			XSECURE_AES_DATA_SWAP_OFFSET, XSECURE_DISABLE_BYTE_SWAP);

	XSecure_SetReset(InstancePtr->BaseAddress,
			XSECURE_AES_SOFT_RST_OFFSET);

	SStatus = XSecure_AesKeyZero(InstancePtr, XSECURE_AES_KUP_KEY);
	if (Status == XST_SUCCESS) {
		Status = SStatus;
	}

	SStatus = XST_FAILURE;
	SStatus = XSecure_AesKeyZero(InstancePtr, XSECURE_AES_EXPANDED_KEYS);
	if (Status == XST_SUCCESS) {
		Status = SStatus;
	}

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function encrypts size (length) number of bytes of the passed
 * 		in InDataAddr (source) buffer and stores the encrypted data along
 * 		with its associated 16 byte tag in the OutDataAddr (destination)
 * 		buffer and GcmTagAddr (buffer) respectively
 *
 * @param	InstancePtr	- Pointer to the XSecure_Aes instance
 * @param	InDataAddr	- Address of the data which needs to be encrypted
 * @param	OutDataAddr	- Address of output buffer where the encrypted data to
 *				  be updated
 * @param	Size		- Size of data to be encrypted in bytes, whereas number
 *				  of bytes provided should be multiples of 4
 * @param	GcmTagAddr	- Address to the buffer of GCM tag size, where the API
 *				  updates GCM tag
 *
 * @return	- XST_SUCCESS - On successful encryption
 *		- XSECURE_AES_INVALID_PARAM - On invalid parameter
 *		- XSECURE_AES_STATE_MISMATCH_ERROR - If State mismatch is occurred
 *		- XST_FAILURE - On failure
 *
 ******************************************************************************/
int XSecure_AesEncryptData(XSecure_Aes *InstancePtr, u64 InDataAddr,
	u64 OutDataAddr, u32 Size, u64 GcmTagAddr)
{
	volatile int Status = XST_FAILURE;

	/* Validate the input arguments */
	if (InstancePtr == NULL) {
		Status = (int)XSECURE_AES_INVALID_PARAM;
		goto END;
	}

	if ((GcmTagAddr == 0x00U) || ((Size % XSECURE_WORD_SIZE) != 0x00U)) {
		Status = (int)XSECURE_AES_INVALID_PARAM;
		goto END;
	}

	if (InstancePtr->AesState != XSECURE_AES_ENCRYPT_INITIALIZED) {
		Status = (int)XSECURE_AES_STATE_MISMATCH_ERROR;
		goto END;
	}

	/* Update the data to AES engine */
	Status = XSecure_AesEncryptUpdate(InstancePtr, InDataAddr, OutDataAddr,
					Size, TRUE);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = XST_FAILURE;

	Status = XSecure_AesEncryptFinal(InstancePtr, GcmTagAddr);

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function sets AES engine to update key and IV during
 * 		decryption of secure header or footer of encrypted partition
 *
 * @param	InstancePtr - Pointer to the XSecure_Aes instance
 * @param	Config	    - XSECURE_AES_ENABLE_KUP_IV_UPDATE to enable KUP and
 *			      IV update
 *			    - XSECURE_AES_DISABLE_KUP_IV_UPDATE to disable KUP and
 *			      IV update
 *
 * @return	- XST_SUCCESS - On successful configuration
 *		- XSECURE_AES_INVALID_PARAM - On invalid parameter
 *		- XSECURE_AES_STATE_MISMATCH_ERROR - If State mismatch is occurred
 *
 ******************************************************************************/
int XSecure_AesCfgKupKeyNIv(const XSecure_Aes *InstancePtr, u8 Config)
{
	int Status = XST_FAILURE;

	/* Validate the input arguments */
	if (InstancePtr == NULL) {
		Status = (int)XSECURE_AES_INVALID_PARAM;
		goto END;
	}

	if ((Config != XSECURE_AES_DISABLE_KUP_IV_UPDATE) &&
		 (Config != XSECURE_AES_ENABLE_KUP_IV_UPDATE)) {
		Status = (int)XSECURE_AES_INVALID_PARAM;
		goto END;
	}

	if (InstancePtr->AesState == XSECURE_AES_UNINITIALIZED) {
		Status = (int)XSECURE_AES_STATE_MISMATCH_ERROR;
		goto END;
	}

	if (Config == XSECURE_AES_DISABLE_KUP_IV_UPDATE) {
		XSecure_WriteReg(InstancePtr->BaseAddress,
			XSECURE_AES_KUP_WR_OFFSET,
			XSECURE_AES_DISABLE_KUP_IV_UPDATE);
	}
	else {
		Status = XSecure_SecureOut32((UINTPTR)(InstancePtr->BaseAddress +
			XSECURE_AES_KUP_WR_OFFSET),
			(XSECURE_AES_KUP_WR_KEY_SAVE_MASK |
			XSECURE_AES_KUP_WR_IV_SAVE_MASK));
		if (Status != XST_SUCCESS) {
			goto END;
		}
	}

	Status = XST_SUCCESS;

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function gives the AES next block length after decryption
 * 		of PDI block
 *
 * @param	InstancePtr	- Pointer to the XSecure_Aes instance
 * @param	Size		- Pointer to a 32 bit variable where next block
 *				  length will be updated
 *
 * @return	- XST_SUCCESS - On success
 *		- XSECURE_AES_INVALID_PARAM - On invalid parameter
 *		- XSECURE_AES_STATE_MISMATCH_ERROR - If State mismatch is occurred
 *
 ******************************************************************************/
int XSecure_AesGetNxtBlkLen(const XSecure_Aes *InstancePtr, u32 *Size)
{
	int Status = XST_FAILURE;

	/* Validate the input arguments */
	if ((InstancePtr == NULL) || (Size == NULL)) {
		Status = (int)XSECURE_AES_INVALID_PARAM;
		goto END;
	}

	if (InstancePtr->AesState == XSECURE_AES_UNINITIALIZED) {
		Status = (int)XSECURE_AES_STATE_MISMATCH_ERROR;
		goto END;
	}

	*Size = Xil_Htonl(XSecure_ReadReg(InstancePtr->BaseAddress,
			XSECURE_AES_IV_3_OFFSET)) * XSECURE_WORD_SIZE;

	Status = XST_SUCCESS;

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function zeroizes the selected AES key storage register
 *
 * @param	InstancePtr	- Pointer to the XSecure_Aes instance
 * @param	KeySrc		- Select the key source which needs to be zeroized
 *
 * @return	- XST_SUCCESS - When key zeroization is success
 * 		- XSECURE_AES_INVALID_PARAM - On invalid parameter
 *		- XSECURE_AES_STATE_MISMATCH_ERROR - If State mismatch is occurred
 * 		- XSECURE_AES_KEY_CLEAR_ERROR - AES key clear error
 *
 ******************************************************************************/
int XSecure_AesKeyZero(const XSecure_Aes *InstancePtr, XSecure_AesKeySrc KeySrc)
{
	int Status = XST_FAILURE;
	u32 Mask;

	/* Validate the input arguments */
	if (InstancePtr == NULL) {
		Status = (int)XSECURE_AES_INVALID_PARAM;
		goto END;
	}

	if ((KeySrc > XSECURE_AES_ALL_KEYS) ||
		(KeySrc < XSECURE_AES_BBRAM_KEY)) {
		Status = (int)XSECURE_AES_INVALID_PARAM;
		goto END_CLR;
	}

	if (InstancePtr->AesState == XSECURE_AES_UNINITIALIZED) {
		Status = (int)XSECURE_AES_STATE_MISMATCH_ERROR;
		goto END_CLR;
	}
	if (KeySrc == XSECURE_AES_ALL_KEYS) {
		Mask = XSECURE_AES_KEY_CLEAR_ALL_KEYS_MASK;
	}
	else if (KeySrc == XSECURE_AES_EXPANDED_KEYS) {
		Mask = XSECURE_AES_KEY_CLEAR_AES_KEY_ZEROIZE_MASK;
	}
	else if (AesKeyLookupTbl[KeySrc].KeyClearVal != XSECURE_AES_INVALID_CFG) {
		Mask = AesKeyLookupTbl[KeySrc].KeyClearVal;
	}
	else {
		Status = XST_INVALID_PARAM;
		goto END_CLR;
	}

	XSecure_WriteReg(InstancePtr->BaseAddress, XSECURE_AES_KEY_CLEAR_OFFSET,
					 Mask);

	Status = (int)Xil_WaitForEvent(((InstancePtr)->BaseAddress +
				XSECURE_AES_KEY_ZEROED_STATUS_OFFSET),
				Mask,
				Mask,
				XSECURE_AES_TIMEOUT_MAX);
	if (Status != XST_SUCCESS) {
		Status = (int)XSECURE_AES_KEY_CLEAR_ERROR;
	}

END_CLR:
	XSecure_WriteReg(InstancePtr->BaseAddress, XSECURE_AES_KEY_CLEAR_OFFSET,
		XSECURE_AES_KEY_CLR_REG_CLR_MASK);

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function performs known answer test(KAT) on AES engine
 *
 * @param	AesInstance	- Pointer to the XSecure_Aes instance
 *
 * @return	- XST_SUCCESS - When KAT Pass
 * 		- XSECURE_AESKAT_INVALID_PARAM - Invalid Argument
 * 		- XSECURE_AES_KAT_WRITE_KEY_FAILED_ERROR - Error when AES key write fails
 * 		- XSECURE_AES_KAT_DECRYPT_INIT_FAILED_ERROR - Error when AES decrypt init fails
 * 		- XSECURE_AES_KAT_GCM_TAG_MISMATCH_ERROR - Error when GCM tag not matched
 *							   with user provided tag
 *		- XSECURE_AES_KAT_DATA_MISMATCH_ERROR - Error when AES data not matched with
 *							expected data
 *
 *****************************************************************************/
int XSecure_AesDecryptKat(XSecure_Aes *AesInstance)
{
	volatile int Status = XST_FAILURE;
	volatile int SStatus = XST_FAILURE;
	u32 Index;

	u32 DstVal[XSECURE_KAT_MSG_SIZE_IN_WORDS] = {0U};

	if (AesInstance == NULL) {
		Status = (int)XSECURE_AESKAT_INVALID_PARAM;
		goto END;
	}

	if ((AesInstance->AesState == XSECURE_AES_ENCRYPT_INITIALIZED) ||
		(AesInstance->AesState == XSECURE_AES_DECRYPT_INITIALIZED)) {
		Status = (int)XSECURE_AES_KAT_BUSY;
		goto END;
	}

	if (AesInstance->AesState != XSECURE_AES_INITIALIZED) {
		Status = (int)XSECURE_AES_STATE_MISMATCH_ERROR;
		goto END;
	}

	/* Write AES key */
	Status = XSecure_AesWriteKey(AesInstance, XSECURE_AES_USER_KEY_7,
			XSECURE_AES_KEY_SIZE_256, (UINTPTR)KatKey);
	if (Status != XST_SUCCESS) {
		Status = (int)XSECURE_AES_KAT_WRITE_KEY_FAILED_ERROR;
		goto END_CLR;
	}

	Status = XST_FAILURE;

	Status = XSecure_AesDecryptInit(AesInstance, XSECURE_AES_USER_KEY_7,
			XSECURE_AES_KEY_SIZE_256, (UINTPTR)KatIv);
	if (Status != XST_SUCCESS) {
		Status = (int)XSECURE_AES_KAT_DECRYPT_INIT_FAILED_ERROR;
		goto END_CLR;
	}

	Status = XST_FAILURE;

	Status = XSecure_AesDecryptData(AesInstance, (UINTPTR)KatMessage,
			(UINTPTR)DstVal, XSECURE_SECURE_GCM_TAG_SIZE, (UINTPTR)KatGcmTag);
	if (Status != XST_SUCCESS) {
		Status = (int)XSECURE_AES_KAT_GCM_TAG_MISMATCH_ERROR;
		goto END_CLR;
	}

	/* Initialized to error */
	Status = (int)XSECURE_AES_KAT_DATA_MISMATCH_ERROR;
	for (Index = 0U; Index < XSECURE_AES_BUFFER_SIZE; Index++) {
		if (DstVal[Index] != KatOutput[Index]) {
			/* Comparison failure of decrypted data */
			Status = (int)XSECURE_AES_KAT_DATA_MISMATCH_ERROR;
			goto END_CLR;
		}
	}

	if (Index == XSECURE_AES_BUFFER_SIZE) {
		Status = XST_SUCCESS;
	}

END_CLR:
	SStatus = XSecure_AesKeyZero(AesInstance, XSECURE_AES_USER_KEY_7);
	if(Status == XST_SUCCESS) {
		Status = SStatus;
	}

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function performs known answer test(KAT) on AES engine to
 * 		confirm DPA counter measures is working fine
 *
 * @param 	AesInstance	- Pointer to the XSecure_Aes instance
 *
 * @return
 * 	- XST_SUCCESS - When KAT Pass
 *	- XSECURE_AESKAT_INVALID_PARAM - On invalid argument
 * 	- XSECURE_AESDPACM_KAT_CHECK1_FAILED_ERROR - Error when AESDPACM data
 *						     not matched with expected data
 * 	- XSECURE_AESDPACM_KAT_CHECK2_FAILED_ERROR - Error when AESDPACM data
 *						     not matched with expected data
 * 	- XSECURE_AESDPACM_KAT_CHECK3_FAILED_ERROR - Error when AESDPACM data
 *						     not matched with expected data
 * 	- XSECURE_AESDPACM_KAT_CHECK4_FAILED_ERROR - Error when AESDPACM data
 *						     not matched with expected data
 * 	- XSECURE_AESDPACM_KAT_CHECK5_FAILED_ERROR - Error when AESDPACM data
 *						     not matched with expected data
 *
 *****************************************************************************/
int XSecure_AesDecryptCmKat(const XSecure_Aes *AesInstance)
{
	volatile int Status = XST_FAILURE;

	u32 Output0[XSECURE_KAT_OPER_DATA_SIZE_IN_WORDS] = {0U};
	u32 Output1[XSECURE_KAT_OPER_DATA_SIZE_IN_WORDS] = {0U};

	const u32 *RM0 = &Output0[0U];
	const u32 *R0 = &Output0[4U];
	const u32 *Mm0 = &Output0[8U];
	const u32 *M0 = &Output0[12U];
	const u32 *RM1 = &Output1[0U];
	const u32 *R1 = &Output1[4U];
	const u32 *Mm1 = &Output1[8U];
	const u32 *M1 = &Output1[12U];

	if (AesInstance == NULL) {
		Status = (int)XSECURE_AESKAT_INVALID_PARAM;
		goto END;
	}

	if ((AesInstance->AesState == XSECURE_AES_ENCRYPT_INITIALIZED) ||
		(AesInstance->AesState == XSECURE_AES_DECRYPT_INITIALIZED)) {
		Status = (int)XSECURE_AES_KAT_BUSY;
		goto END;
	}

	if (AesInstance->AesState != XSECURE_AES_INITIALIZED) {
		Status = (int)XSECURE_AES_STATE_MISMATCH_ERROR;
		goto END;
	}

	/* Test 1 */
	Status = XSecure_AesDpaCmDecryptKat(AesInstance, Key0, Data0, Output0);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = XST_FAILURE;

	Status = XSecure_AesDpaCmDecryptKat(AesInstance, Key1, Data1, Output1);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = XST_FAILURE;

	if (((RM0[0U] == 0U) && (RM0[1U] == 0U) && (RM0[2U] == 0U) &&
				(RM0[3U] == 0U)) ||
			((RM0[0U] == RM1[0U]) && (RM0[1U] == RM1[1U]) &&
				(RM0[2U] == RM1[2U]) && (RM0[3U] == RM1[3U])) ||
			((RM0[0U] == Mm0[0U]) && (RM0[1U] == Mm0[1U]) &&
				(RM0[2U] == Mm0[2U]) && (RM0[3U] == Mm0[3U])) ||
			((RM0[0U] == Mm1[0U]) && (RM0[1U] == Mm1[1U]) &&
				(RM0[2U] == Mm1[2U]) && (RM0[3U] == Mm1[3U]))) {
		Status = (int)XSECURE_AESDPACM_KAT_CHECK1_FAILED_ERROR;
		goto END;
	}

	if (((RM1[0U] == 0U) && (RM1[1U] == 0U) && (RM1[2U] == 0U) &&
				(RM1[3U] == 0U)) ||
			((RM1[0U] == RM0[0U]) && (RM1[1U] == RM0[1U]) &&
				(RM1[2U] == RM0[2U]) && (RM1[3U] == RM0[3U])) ||
			((RM1[0U] == Mm0[0U]) && (RM1[1U] == Mm0[1U]) &&
				(RM1[2U] == Mm0[2U]) && (RM1[3U] == Mm0[3U])) ||
			((RM1[0U] == Mm1[0U]) && (RM1[1U] == Mm1[1]) &&
				(RM1[2U] == Mm1[2U]) && (RM1[3U] == Mm1[3U]))) {
		Status = (int)XSECURE_AESDPACM_KAT_CHECK2_FAILED_ERROR;
		goto END;
	}

	if (((Mm0[0U] == 0U) && (Mm0[1U] == 0U) && (Mm0[2U] == 0U) &&
				(Mm0[3U] == 0U)) ||
			((Mm0[0U] == RM0[0U]) && (Mm0[1U] == RM0[1U]) &&
				(Mm0[2U] == RM0[2U]) && (Mm0[3U] == RM0[3U])) ||
			((Mm0[0U] == RM1[0U]) && (Mm0[1U] == RM1[1U]) &&
				(Mm0[2U] == RM1[2U]) && (Mm0[3U] == RM1[3U])) ||
			((Mm0[0U] == Mm1[0U]) && (Mm0[1U] == Mm1[1U]) &&
				(Mm0[2U] == Mm1[2U]) && (Mm0[3U] == Mm1[3U]))) {
		Status = (int)XSECURE_AESDPACM_KAT_CHECK3_FAILED_ERROR;
		goto END;
	}

	if (((Mm1[0U] == 0U) && (Mm1[1U] == 0U) && (Mm1[2U] == 0U) &&
				(Mm1[3U] == 0U)) ||
			((Mm1[0U] == RM0[0U]) && (Mm1[1U] == RM0[1U]) &&
				(Mm1[2U] == RM0[2U]) && (Mm1[3U] == RM0[3U])) ||
			((Mm1[0U] == RM1[0U]) && (Mm1[1U] == RM1[1U]) &&
				(Mm1[2U] == RM1[2U]) && (Mm1[3U] == RM1[3U])) ||
			((Mm1[0U] == Mm0[0U]) && (Mm1[1U] == Mm0[1U]) &&
				(Mm1[2U] == Mm0[2U]) && (Mm1[3U] == Mm0[3U]))) {
		Status = (int)XSECURE_AESDPACM_KAT_CHECK4_FAILED_ERROR;
		goto END;
	}

	if ((((R0[0U] ^ RM0[0U]) != Ct0[0U])  || ((R0[1U] ^ RM0[1U]) != Ct0[1U]) ||
		 ((R0[2U] ^ RM0[2U]) != Ct0[2U])  || ((R0[3U] ^ RM0[3U]) != Ct0[3U])) ||
		(((M0[0U] ^ Mm0[0U]) != MiC0[0U]) || ((M0[1U] ^ Mm0[1U]) != MiC0[1U]) ||
		 ((M0[2U] ^ Mm0[2U]) != MiC0[2U]) || ((M0[3U] ^ Mm0[3U]) != MiC0[3U])) ||
		(((R1[0U] ^ RM1[0U]) != Ct1[0U])  || ((R1[1U] ^ RM1[1U]) != Ct1[1U]) ||
		((R1[2U] ^ RM1[2U]) != Ct1[2U])  || ((R1[3U] ^ RM1[3U]) != Ct1[3U])) ||
		(((M1[0U] ^ Mm1[0U]) != MiC1[0U]) || ((M1[1U] ^ Mm1[1U]) != MiC1[1U]) ||
		 ((M1[2U] ^ Mm1[2U]) != MiC1[2U]) || ((M1[3U] ^ Mm1[3U]) != MiC1[3U]))) {
		Status = (int)XSECURE_AESDPACM_KAT_CHECK5_FAILED_ERROR;
		goto END;
	}

	Status = XST_SUCCESS;

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function performs KAT on AES core with DPACM enabled
 *
 * @param 	AesInstance	- InstancePtr Pointer to the XSecure_Aes instance
 * @param 	KeyPtr		- Key Pointer
 * @param 	DataPtr		- Data Pointer
 * @param 	OutputPtr	- Output where the decrypted data to be stored
 *
 * @return
 *		- XST_SUCCESS - On success
 * 		- XSECURE_AESKAT_INVALID_PARAM		      - Invalid Argument
 * 		- XSECURE_AESDPACM_KAT_WRITE_KEY_FAILED_ERROR - Error when AESDPACM key
 *								write fails
 * 		- XSECURE_AESDPACM_KAT_KEYLOAD_FAILED_ERROR - Error when AESDPACM key
 *							      load fails
 * 		- XSECURE_AESDPACM_SSS_CFG_FAILED_ERROR - Error when AESDPACM sss
 *							  configuration fails
 * 		- XSECURE_AESDPACM_KAT_FAILED_ERROR - Error when AESDPACM KAT fails
 * 		- XST_FAILURE - On failure
 *
 *****************************************************************************/
static int XSecure_AesDpaCmDecryptKat(const XSecure_Aes *AesInstance,
	const u32 *KeyPtr, const u32 *DataPtr, u32 *OutputPtr)
{
	volatile int Status = XST_FAILURE;
	u32 Index;
	u32 ReadReg = 0U;

	if ((KeyPtr == NULL) ||
	   (DataPtr == NULL) ||
	   (OutputPtr == NULL) ||
	   (AesInstance->PmcDmaPtr == NULL)) {
		Status = (int)XSECURE_AESKAT_INVALID_PARAM;
		goto END;
	}

	XSecure_ReleaseReset(AesInstance->BaseAddress,
				XSECURE_AES_SOFT_RST_OFFSET);

	/* Configure AES for Encryption */
	XSecure_WriteReg(AesInstance->BaseAddress,
		XSECURE_AES_MODE_OFFSET, XSECURE_AES_MODE_ENC);

	/* Configure AES in split mode */
	XSecure_WriteReg(AesInstance->BaseAddress, XSECURE_AES_SPLIT_CFG_OFFSET,
		(XSECURE_AES_SPLIT_CFG_KEY_SPLIT |
		XSECURE_AES_SPLIT_CFG_DATA_SPLIT));

	ReadReg = XSecure_ReadReg(AesInstance->BaseAddress,
		XSECURE_AES_CM_EN_OFFSET);

	XSecure_WriteReg(AesInstance->BaseAddress, XSECURE_AES_CM_EN_OFFSET,
		XSECURE_AES_CM_EN_VAL_MASK);

	/* Write Key mask value */
	for (Index = 0U; Index < XSECURE_AES_KEY_SIZE_256BIT_WORDS; Index++) {
		XSecure_WriteReg(AesInstance->BaseAddress,
			XSECURE_AES_KEY_MASK_INDEX + (u32)(Index * XSECURE_WORD_SIZE),
			0x0U);
	}

	/* Write AES key */
	Status = XSecure_AesWriteKey(AesInstance, XSECURE_AES_USER_KEY_7,
			XSECURE_AES_KEY_SIZE_256, (UINTPTR)KeyPtr);
	if (Status != XST_SUCCESS) {
		Status = (int)XSECURE_AESDPACM_KAT_WRITE_KEY_FAILED_ERROR;
		goto END;
	}

	Status = XST_FAILURE;

	Status = XSecure_AesKeyLoad(AesInstance, XSECURE_AES_USER_KEY_7,
			XSECURE_AES_KEY_SIZE_256);
	if (Status != XST_SUCCESS) {
		Status = (int)XSECURE_AESDPACM_KAT_KEYLOAD_FAILED_ERROR;
		goto END;
	}

	Status = XST_FAILURE;

	Status = XSecure_SssAes(&AesInstance->SssInstance,
			XSECURE_SSS_DMA0, XSECURE_SSS_DMA0);
	if(Status != XST_SUCCESS) {
		Status = (int)XSECURE_AESDPACM_SSS_CFG_FAILED_ERROR;
		goto END;
	}

	/* Start the message. */
	XSecure_WriteReg(AesInstance->BaseAddress, XSECURE_AES_START_MSG_OFFSET,
		XSECURE_AES_START_MSG_VAL_MASK);

	XSecure_WriteReg(AesInstance->BaseAddress, XSECURE_AES_DATA_SWAP_OFFSET,
		XSECURE_AES_DATA_SWAP_VAL_MASK);

	/* Enable PMC DMA Src channel for byte swapping.*/
	XSecure_AesPmcDmaCfgEndianness(AesInstance->PmcDmaPtr,
		XPMCDMA_SRC_CHANNEL, XSECURE_AES_DATA_SWAP_VAL_MASK);

	/* Enable PMC DMA Dst channel for byte swapping.*/
	XSecure_AesPmcDmaCfgEndianness(AesInstance->PmcDmaPtr,
		XPMCDMA_DST_CHANNEL, XSECURE_AES_DATA_SWAP_VAL_MASK);

	/* Configure the PMC DMA Tx/Rx for the incoming Block. */
	XPmcDma_Transfer(AesInstance->PmcDmaPtr, XPMCDMA_DST_CHANNEL,
		(u64)(UINTPTR)OutputPtr, XSECURE_AES_DMA_SIZE,
		XSECURE_AES_DMA_LAST_WORD_DISABLE);

	XPmcDma_Transfer(AesInstance->PmcDmaPtr, XPMCDMA_SRC_CHANNEL,
		(u64)(UINTPTR)DataPtr, XSECURE_AES_DMA_SIZE,
		XSECURE_AES_DMA_LAST_WORD_ENABLE);

	Status = XPmcDma_WaitForDoneTimeout(AesInstance->PmcDmaPtr,
		XPMCDMA_DST_CHANNEL);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = XST_FAILURE;

	Status = XSecure_AesWaitForDone(AesInstance);
	if (Status != XST_SUCCESS) {
		Status = (int)XSECURE_AESDPACM_KAT_FAILED_ERROR;
		goto END;
	}

	XSecure_WriteReg(AesInstance->BaseAddress, XSECURE_AES_DATA_SWAP_OFFSET,
		XSECURE_AES_DATA_SWAP_VAL_DISABLE);

END:
	XPmcDma_IntrClear(AesInstance->PmcDmaPtr, XPMCDMA_DST_CHANNEL,
		XPMCDMA_IXR_DONE_MASK);

	/* Acknowledge the transfer has completed */
	XPmcDma_IntrClear(AesInstance->PmcDmaPtr, XPMCDMA_SRC_CHANNEL,
		XPMCDMA_IXR_DONE_MASK);
	XSecure_AesPmcDmaCfgEndianness(AesInstance->PmcDmaPtr,
		XPMCDMA_SRC_CHANNEL, XSECURE_AES_DATA_SWAP_VAL_DISABLE);

	/* Disable PMC DMA Dst channel for byte swapping. */
	XSecure_AesPmcDmaCfgEndianness(AesInstance->PmcDmaPtr,
		XPMCDMA_DST_CHANNEL, XSECURE_AES_DATA_SWAP_VAL_DISABLE);

	/* Configure AES in split mode */
	XSecure_WriteReg(AesInstance->BaseAddress, XSECURE_AES_SPLIT_CFG_OFFSET,
		XSECURE_AES_SPLIT_CFG_DATA_KEY_DISABLE);

	/* Copy the initial status of DPACM */
	XSecure_WriteReg(AesInstance->BaseAddress, XSECURE_AES_CM_EN_OFFSET,
		ReadReg);

	/* AES reset */
	XSecure_SetReset(AesInstance->BaseAddress, XSECURE_AES_SOFT_RST_OFFSET);

	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function waits for AES engine completes key loading
 *
 * @param	InstancePtr - Pointer to the XSecure_Aes instance
 *
 * @return	- XST_SUCCESS - If the AES engine completes key loading
 *		- XST_FAILURE - If a timeout has occurred
 *
 ******************************************************************************/
static int XSecure_AesWaitKeyLoad(const XSecure_Aes *InstancePtr)
{
	int Status = XST_FAILURE;

	/* Assert validates the input arguments */
	XSecure_AssertNonvoid(InstancePtr != NULL);
	Status = (int)Xil_WaitForEvent(((InstancePtr)->BaseAddress +
					XSECURE_AES_STATUS_OFFSET),
			XSECURE_AES_STATUS_KEY_INIT_DONE_MASK,
			XSECURE_AES_STATUS_KEY_INIT_DONE_MASK,
			XSECURE_AES_TIMEOUT_MAX);

	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function configures and loads AES key from selected
 *		key source

 * @param	InstancePtr - Pointer to the XSecure_Aes instance
 * @param	KeySrc	    - Variable is of type XSecure_AesKeySrc which
 *			      mentions the key source to be loaded into AES engine
 * @param	KeySize	    - Size of the key selected
 *
 * @return	- XST_SUCCESS - On successful key load
 *		- XST_FAILURE - If a timeout has occurred
 *
 ******************************************************************************/
static int XSecure_AesKeyLoad(const XSecure_Aes *InstancePtr,
	XSecure_AesKeySrc KeySrc, XSecure_AesKeySize KeySize)
{
	int Status = XST_FAILURE;

	/* Assert validates the input arguments */
	XSecure_AssertNonvoid(InstancePtr != NULL);
	XSecure_AssertNonvoid((KeySrc < XSECURE_MAX_KEY_SOURCES) &&
		(KeySrc >= XSECURE_AES_BBRAM_KEY));
	XSecure_AssertNonvoid((KeySize == XSECURE_AES_KEY_SIZE_128) ||
			  (KeySize == XSECURE_AES_KEY_SIZE_256));

	/* Load Key Size */
	XSecure_WriteReg(InstancePtr->BaseAddress, XSECURE_AES_KEY_SIZE_OFFSET,
		(u32)KeySize);

	/* AES key source selection */
	XSecure_WriteReg(InstancePtr->BaseAddress,
			XSECURE_AES_KEY_SEL_OFFSET,
			AesKeyLookupTbl[KeySrc].KeySrcSelVal);

	/* Trig loading of key. */
	XSecure_WriteReg(InstancePtr->BaseAddress,
			XSECURE_AES_KEY_LOAD_OFFSET,
			XSECURE_AES_KEY_LOAD_VAL_MASK);

	/* Wait for AES key loading.*/
	Status = XSecure_AesWaitKeyLoad(InstancePtr);

	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function waits for AES completion
 *
 * @param	InstancePtr - Pointer to the XSecure_Aes instance
 *
 * @return	- XST_SUCCESS - On successful key load
 *		- XST_FAILURE - On failure
 *
 ******************************************************************************/
static int XSecure_AesWaitForDone(const XSecure_Aes *InstancePtr)
{
	int Status = XST_FAILURE;

	/* Assert validates the input arguments */
	XSecure_AssertNonvoid(InstancePtr != NULL);

	Status = (int)Xil_WaitForEvent(((InstancePtr)->BaseAddress +
				XSECURE_AES_STATUS_OFFSET),
				XSECURE_AES_STATUS_DONE_MASK,
				XSECURE_AES_STATUS_DONE_MASK,
				XSECURE_AES_TIMEOUT_MAX);

	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function waits for AES key decryption completion
 *
 * @param	InstancePtr - Pointer to the XSecure_Aes instance
 *
 * @return	- XST_SUCCESS - On success
 *		- XST_FAILURE - On failure
 *
 ******************************************************************************/
static int XSecure_AesKekWaitForDone(const XSecure_Aes *InstancePtr)
{
	int Status = XST_FAILURE;

	/* Assert validates the input arguments */
	XSecure_AssertNonvoid(InstancePtr != NULL);

	Status = (int)Xil_WaitForEvent(((InstancePtr)->BaseAddress +
				XSECURE_AES_STATUS_OFFSET),
				XSECURE_AES_STATUS_BLK_KEY_DEC_DONE_MASK,
				XSECURE_AES_STATUS_BLK_KEY_DEC_DONE_MASK,
				XSECURE_AES_TIMEOUT_MAX);

	return Status;
}

/******************************************************************************/
/**
 * @brief	This is a helper function to enable/disable byte swapping feature
 * 		of PMC DMA
 *
 * @param	InstancePtr 	- Pointer to the XPmcDma instance
 * @param	Channel 	- Channel Type 	- XPMCDMA_SRC_CHANNEL
 *						- XPMCDMA_DST_CHANNEL
 * @param	EndianType 	- 1 : Enable Byte Swapping
 *				- 0 : Disable Byte Swapping
 *
 * @return	None
 *
 ******************************************************************************/
static void XSecure_AesPmcDmaCfgEndianness(XPmcDma *InstancePtr,
	XPmcDma_Channel Channel, u8 EndianType)
{
	XPmcDma_Configure ConfigValues = {0U};

	/* Assert validates the input arguments */
	XSecure_AssertVoid(InstancePtr != NULL);

	XPmcDma_GetConfig(InstancePtr, Channel, &ConfigValues);
	ConfigValues.EndianType = EndianType;
	XPmcDma_SetConfig(InstancePtr, Channel, &ConfigValues);
}

/*****************************************************************************/
/**
 * @brief	This function initializes AES engine for encryption or decryption
 *
 * @param	InstancePtr	- Pointer to the XSecure_Aes instance
 * @param	KeySrc		- Key Source for decryption of the data
 * @param	KeySize		- Size of the AES key to be used for decryption
 *              		- XSECURE_AES_KEY_SIZE_128 for 128 bit key size
 *              		- XSECURE_AES_KEY_SIZE_256 for 256 bit key size
 * @param	IvAddr		- Address to the buffer holding IV
 *
 * @return	- XST_SUCCESS - On successful initialization
 * 		- XST_FAILURE - On failure
 *
 ******************************************************************************/
static int XSecure_AesOpInit(const XSecure_Aes *InstancePtr,
	XSecure_AesKeySrc KeySrc, XSecure_AesKeySize KeySize, u64 IvAddr)
{
	volatile int Status = XST_FAILURE;
	XSecure_AesDmaCfg AesDmaCfg = {0U};

	Status = XSecure_AesKeyLoad(InstancePtr, KeySrc, KeySize);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	/* Start the message. */
	XSecure_WriteReg(InstancePtr->BaseAddress,
			XSECURE_AES_START_MSG_OFFSET,
			XSECURE_AES_START_MSG_VAL_MASK);

	XSecure_WriteReg(InstancePtr->BaseAddress,
			XSECURE_AES_DATA_SWAP_OFFSET, XSECURE_ENABLE_BYTE_SWAP);

	AesDmaCfg.SrcChannelCfg = TRUE;
	AesDmaCfg.SrcDataAddr = IvAddr;
	AesDmaCfg.IsLastChunkSrc = FALSE;
	Status = XSecure_AesPmcDmaCfgAndXfer(InstancePtr, AesDmaCfg,
			XSECURE_SECURE_GCM_TAG_SIZE);

	XSecure_AesPmcDmaCfgEndianness(InstancePtr->PmcDmaPtr, XPMCDMA_SRC_CHANNEL,
		XSECURE_DISABLE_BYTE_SWAP);

END:
	return Status;
}

/*****************************************************************************/
/**
 *
 * @brief
 * This function configures the PMC DMA channels
 *
 * @param       InstancePtr             Pointer to the XSecure_Aes instance.
 * @param       AesDmaCfg       DMA SRC and DEST channel configuration
 * @param       Size                    Size of data in bytes.
 *
 * @return
 *              - XST_SUCCESS on successful configuration
 *              - Error code on failure
 *
 ******************************************************************************/
static int XSecure_AesPmcDmaCfgAndXfer(const XSecure_Aes *InstancePtr,
	XSecure_AesDmaCfg AesDmaCfg, u32 Size)
{
	int Status = XST_FAILURE;

	/* Configure the SSS for AES. */
	if (InstancePtr->PmcDmaPtr->Config.DeviceId == (u16)PMCDMA_0_DEVICE_ID) {
		Status = XSecure_SssAes(&InstancePtr->SssInstance,
				XSECURE_SSS_DMA0, XSECURE_SSS_DMA0);
	}
	else {
		Status = XSecure_SssAes(&InstancePtr->SssInstance,
				XSECURE_SSS_DMA1, XSECURE_SSS_DMA1);
	}
	if (Status != XST_SUCCESS) {
		goto END;
	}

	/* Enable PMC DMA Src and Dst channels for byte swapping.*/
	if (AesDmaCfg.SrcChannelCfg == TRUE) {
		XSecure_AesPmcDmaCfgEndianness(InstancePtr->PmcDmaPtr,
			XPMCDMA_SRC_CHANNEL, XSECURE_ENABLE_BYTE_SWAP);
	}

	if ((AesDmaCfg.DestChannelCfg == TRUE) &&
		((u32)AesDmaCfg.DestDataAddr != XSECURE_AES_NO_CFG_DST_DMA)) {
		XSecure_AesPmcDmaCfgEndianness(InstancePtr->PmcDmaPtr,
			XPMCDMA_DST_CHANNEL, XSECURE_ENABLE_BYTE_SWAP);
	}

	if ((AesDmaCfg.DestChannelCfg == TRUE) &&
		((u32)AesDmaCfg.DestDataAddr != XSECURE_AES_NO_CFG_DST_DMA)) {
		XPmcDma_64BitTransfer(InstancePtr->PmcDmaPtr, XPMCDMA_DST_CHANNEL,
			(u32)AesDmaCfg.DestDataAddr, (u32)(AesDmaCfg.DestDataAddr >> 32U),
			Size / XSECURE_WORD_SIZE, AesDmaCfg.IsLastChunkDest);
	}

	if (AesDmaCfg.SrcChannelCfg == TRUE) {
		XPmcDma_64BitTransfer(InstancePtr->PmcDmaPtr, XPMCDMA_SRC_CHANNEL,
			(u32)AesDmaCfg.SrcDataAddr, (u32)(AesDmaCfg.SrcDataAddr >> 32U),
			Size / XSECURE_WORD_SIZE, AesDmaCfg.IsLastChunkSrc);
	}

	if (AesDmaCfg.SrcChannelCfg == TRUE) {
		/* Wait for the SRC DMA completion. */
		Status = XPmcDma_WaitForDoneTimeout(InstancePtr->PmcDmaPtr,
			XPMCDMA_SRC_CHANNEL);
		if (Status != XST_SUCCESS) {
			goto END;
		}

		/* Acknowledge the transfer has completed */
		XPmcDma_IntrClear(InstancePtr->PmcDmaPtr, XPMCDMA_SRC_CHANNEL,
			XPMCDMA_IXR_DONE_MASK);
	}

	if ((AesDmaCfg.DestChannelCfg == TRUE) &&
		((u32)AesDmaCfg.DestDataAddr != XSECURE_AES_NO_CFG_DST_DMA)) {
		/* Wait for the DEST DMA completion. */
		Status = XPmcDma_WaitForDoneTimeout(InstancePtr->PmcDmaPtr,
			XPMCDMA_DST_CHANNEL);
		if (Status != XST_SUCCESS) {
			goto END;
		}

		/* Acknowledge the transfer has completed */
		XPmcDma_IntrClear(InstancePtr->PmcDmaPtr, XPMCDMA_DST_CHANNEL,
			XPMCDMA_IXR_DONE_MASK);
	}

END:
	return Status;
}
