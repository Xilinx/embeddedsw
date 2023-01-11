/******************************************************************************
* Copyright (C) 2023, Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
 * @file xilpki.c
 * @addtogroup xilpki Overview
 * This file contains the implementation of the interface functions for the
 * PKI ECDSA hardware module.
 *
 *
 * @{
 * @details
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who   Date      Changes
 * ----- ----  --------  ------------------------------------------------------
 * 1.0   Nava  12/05/22  Initial Release
 *</pre>
 *
 *@note
 *****************************************************************************/
/***************************** Include Files *********************************/
#include "xilpki.h"

/***************** Macros (Inline Functions) Definitions ********************/
#define XPKI_RESET_DELAY_US		10U
#define	XPKI_ASSERT_RESET		1U
#define XPKI_DEASSERT_RESET		0U
#define FPD_CLEAR_WRITE_PROTECT		0U
#define XPKI_MUX_SELECT			1U
#define XPKI_ADDR_WORD_ALIGN_MASK	(0x3U)

#define XPKI_TRNG_INSTANCE		1U
#define XPKI_TRNG_BUF_SIZE		32U
/************************** Function Prototypes ******************************/
static int XPki_TrngInit(void);
static XTrngpsx_Instance *XPki_Get_Trng_InstancePtr(u8 DeviceId);

/*****************************************************************************/
/**
 * @brief	This function reset the PKI module.
 *
******************************************************************************/
void XPki_Reset(void)
{
	/* Reset PKI module */
	Xil_Out32(PSX_CRF_RST_PKI, XPKI_ASSERT_RESET);
	usleep(XPKI_RESET_DELAY_US);
	Xil_Out32(PSX_CRF_RST_PKI, XPKI_DEASSERT_RESET);
}

/*****************************************************************************/
/**
 * @brief	This function performs the PKI module soft reset.
 *
******************************************************************************/
void XPki_SoftReset(void)
{
	/* PKI Soft reset */
	Xil_Out32(FPD_PKI_SOFT_RESET, XPKI_ASSERT_RESET);
	usleep(XPKI_RESET_DELAY_US);
	Xil_Out32(FPD_PKI_SOFT_RESET, XPKI_DEASSERT_RESET);
}

/*****************************************************************************/
/**
 * @brief	This function performs the PKI module initialization.
 *
 * @param	InstancePtr	Pointer to the XPki instance
 *
 * @return
 *	-	XST_SUCCESS 			- On success
 *	-	XPKI_INVALID_PARAM		- On invalid argument
 *	-	XPKI_ERROR_UNALIGN_ADDR		- On Request/Completion queue
 *						  Address are not Word aligned.
 *	-	XST_FAILURE			- On failure
******************************************************************************/
int XPki_Initialize(XPki_Instance *InstancePtr)
{
	volatile int Status = XST_FAILURE;
	u64 RegVal;

	/* Validate the input arguments */
	if (InstancePtr == NULL) {
	Status = XPKI_INVALID_PARAM;
		goto END;
	}

	if (((InstancePtr->RQInputAddr & XPKI_ADDR_WORD_ALIGN_MASK) != 0U) ||
	    ((InstancePtr->RQOutputAddr & XPKI_ADDR_WORD_ALIGN_MASK) != 0U) ||
	    ((InstancePtr->CQAddr & XPKI_ADDR_WORD_ALIGN_MASK) != 0U)) {
		Status = XPKI_ERROR_UNALIGN_ADDR;
		goto END;
	}

	/* Clear fpd slcr write protection reg */
	Xil_Out32(FPD_SLCR_WPROT0, FPD_CLEAR_WRITE_PROTECT);

	/* Release pki reset */
	XPki_Reset();

	/* Pki mux selection */
	Xil_UtilRMW32(FPD_SLCR_PKI_MUX_SEL, FPD_SLCR_PKI_MUX_SEL_FULLRWMASK,
		      XPKI_MUX_SELECT);

	/* Enable/Disable CM */
	RegVal = Xil_In64(FPD_PKI_ENGINE_CTRL);

	if (InstancePtr->Is_Cm_Enabled) {
		RegVal &=  ~(FPD_PKI_ENGINE_CM_MASK);
	} else {
		RegVal |= FPD_PKI_ENGINE_CM_MASK;
	}

	Xil_Out64(FPD_PKI_ENGINE_CTRL, RegVal);

	Status = XPki_TrngInit();
END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function is used to enable the PKI Module write protection.
 *		(fpd slcr write protection)
 *
******************************************************************************/
void XPki_Close(void)
{
	/* Enable fpd slcr write protection reg */
	Xil_Out32(FPD_SLCR_WPROT0, FPD_SLCR_WPROT0_DEFVAL);
}

/*****************************************************************************/
/**
 * @brief	This function is used to get the TRNG Instance.
 *
 * @return	returns NULL or Pointer to the TRNG Instance.
 *
 *****************************************************************************/
static XTrngpsx_Instance *XPki_Get_Trng_InstancePtr(u8 DeviceId)
{
	static XTrngpsx_Instance Trngpsx[XPAR_XTRNGPSX_NUM_INSTANCES];
	if ((DeviceId == 0) || ( DeviceId >= XPAR_XTRNGPSX_NUM_INSTANCES)) {
		return NULL;
	}

	return &Trngpsx[DeviceId];
}

/*****************************************************************************/
/**
 * @brief       This function is used to initialize the TRNG driver
 *
 * @return      returns the error code on any error
 *              returns XST_SUCCESS on success
 *
 *****************************************************************************/
static int XPki_TrngInit(void)
{
	volatile int Status = XST_FAILURE;
	XTrngpsx_Config *Config;
	XTrngpsx_Instance *Trngpsx;
	XTrngpsx_UserConfig UsrCfg = {
			.Mode = XTRNGPSX_HRNG_MODE,
			.SeedLife = XTRNGPSX_USER_CFG_SEED_LIFE,
			.DFLength = XTRNGPSX_USER_CFG_DF_LENGTH,
			.AdaptPropTestCutoff = XTRNGPSX_USER_CFG_ADAPT_TEST_CUTOFF,
			.RepCountTestCutoff = XTRNGPSX_USER_CFG_REP_TEST_CUTOFF,
			};

	for (u8 DeviceId = 1U; DeviceId < XPAR_XTRNGPSX_NUM_INSTANCES; DeviceId++) {
		/*
		 * Initialize the TRNGPSX driver so that it's ready to use look up
		 * configuration in the config table, then initialize it.
		 */
		Config = XTrngpsx_LookupConfig(DeviceId);
		if (NULL == Config) {
			goto END;
		}

		Trngpsx = XPki_Get_Trng_InstancePtr(DeviceId);
		if (Trngpsx == NULL) {
			goto END;
		}

		/* Initialize the TRNGPSX driver so that it is ready to use. */
		XSECURE_TEMPORAL_CHECK(END, Status, XTrngpsx_CfgInitialize,
				       Trngpsx, Config, Config->BaseAddress);

		XSECURE_TEMPORAL_CHECK(END, Status, XTrngpsx_PreOperationalSelfTests, Trngpsx);

		/* Instantiate to complete initialization and for (initial) reseeding */
		XSECURE_TEMPORAL_CHECK(END, Status, XTrngpsx_Instantiate,Trngpsx,
				       NULL, 0U, NULL, &UsrCfg);
	}
END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function is used to generate the Random number for the give
 *		size.
 * @param       GenSize - Required Random Number length in Bytes.
 * @Param	RandBuf - Pointer to store the Random Number.
 *
 * @return
 *      -       XST_SUCCESS                     - On success
 *      -       XST_FAILURE                     - On failure
******************************************************************************/
int XPki_TrngGenerateRandomNum(u8 GenSize, u8 *RandBuf)
{
	XTrngpsx_Instance *Trngpsx = XPki_Get_Trng_InstancePtr(XPKI_TRNG_INSTANCE);
	u8 Count = (GenSize / XPKI_TRNG_BUF_SIZE);
	volatile int Status = XST_FAILURE;

	if (GenSize % XPKI_TRNG_BUF_SIZE != 0) {
		Count++;
	}

	for (u8 i = 0U; i < Count; i++) {
		XSECURE_TEMPORAL_CHECK(END, Status, XTrngpsx_Generate, Trngpsx,
				       RandBuf, XPKI_TRNG_BUF_SIZE, FALSE);
		RandBuf += XPKI_TRNG_BUF_SIZE;
	}
END:
	return Status;
}
