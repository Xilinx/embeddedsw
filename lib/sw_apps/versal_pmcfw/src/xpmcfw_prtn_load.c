/******************************************************************************
*
* Copyright (C) 2017-2019 Xilinx, Inc.  All rights reserved.
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
* FITNESS FOR A PRTNICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
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

/*****************************************************************************/
/**
*
* @file xpmcfw_prtn_load.c
*
* This is the file which contains partition load code for the PMCFW.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- -------- -------------------------------------------------------
* 1.00  kc   02/21/2017 Initial release
*
* </pre>
*
* @note
*
******************************************************************************/

/***************************** Include Files *********************************/
#include "xpmcfw_default.h"
#include "xpmcfw_main.h"
#include "xpmcfw_fabric.h"
#include "xpmcfw_misc.h"
#include "xilcdo.h"
#include "xsecure.h"
#include "xilcdo_npi.h"
/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/
extern XilCdo_Prtn XilCdoPrtnInst;
extern XCsuDma CsuDma0;
u32 PlCfiPresent = FALSE;
/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/
static XStatus XPmcFw_PrtnHdrValidation(XPmcFw * PmcFwInstancePtr, u32 PrtnNum);
static XStatus XPmcFw_ProcessPrtn (XPmcFw * PmcFwInstancePtr, u32 PrtnNum);
static XStatus XPmcFw_PrtnCopy(XPmcFw * PmcFwInstancePtr, u32 PrtnNum);
static XStatus XPmcFw_PrtnValidation(XPmcFw * PmcFwInstancePtr, u32 PrtnNum);
static XStatus XPmcFw_CheckHandoffCpu (XPmcFw * PmcFwInstancePtr, u32 DstnCpu);
static void XPmcFw_UpdateHandoffParam(XPmcFw * PmcFwInstancePtr, u32 PrtnNum);

#ifdef NPI_TRACE_ENABLE
/* This function is for internal use, to log NPI/CDO data */
/* TODO Need to remove this function after completion of emulation verification */
static void XilNpi_Trace(u32 Len);
#endif
/************************** Variable Definitions *****************************/

u64 AcBuffer[XSECURE_AUTH_CERT_MIN_SIZE/8];

/*****************************************************************************/
/**
 * This function loads the partition
 *
 * @param	PmcFwInstancePtr is pointer to the XPmcFw Instance
 *
 * @param	PrtnNum is the partition number in the image to be loaded
 *
 * @return	returns the error codes described in xpmcfw_error.h on any error
 *			returns XPMCFW_SUCCESS on success
 *
 *****************************************************************************/
XStatus XPmcFw_PrtnLoad(XPmcFw * PmcFwInstancePtr, u32 PrtnNum)
{
	XStatus Status;

#ifdef XPMCFW_WDT_PRESENT
	/* Restart WDT as partition copy can take more time */
	XPmcFw_RestartWdt();
#endif

	XPmcFw_Printf(DEBUG_GENERAL,
	      "Loading Partition(%d).........\n\r", PrtnNum);

	/* Load and validate the partition */

	/* Prtn Hdr Validation */
	Status = XPmcFw_PrtnHdrValidation(PmcFwInstancePtr, PrtnNum);

	/* PMCFW is not partition owner and skip this partition */
	if (Status == XPMCFW_SUCCESS_NOT_PRTN_OWNER)
	{
		Status = XPMCFW_SUCCESS;
		goto END;
	} else if (XPMCFW_SUCCESS != Status)
	{
		goto END;
	} else
	{
		/* For MISRA C compliance */
	}

	/* Process Partition */
	Status = XPmcFw_ProcessPrtn(PmcFwInstancePtr, PrtnNum);
	if (XPMCFW_SUCCESS != Status)
	{
		goto END;
	}

END:
	return Status;
}

/*****************************************************************************/
/**
 * This function validates the partition header
 *
 * @param	PmcFwInstancePtr is pointer to the XPmcFw Instance
 *
 * @param	PrtnNum is the partition number in the image to be loaded
 *
 * @return	returns the error codes described in xpmcfw_error.h
 *****************************************************************************/
static XStatus XPmcFw_PrtnHdrValidation(XPmcFw * PmcFwInstancePtr,
		u32 PrtnNum)
{
	XStatus Status;
	XilPdi_PrtnHdr * PrtnHdr;

	/* Assign the partition header to local variable */
	PrtnHdr = &(PmcFwInstancePtr->MetaHdr.PrtnHdr[PrtnNum]);

	/* Check if partition belongs to PMCFW */
	if (XilPdi_GetPrtnOwner(PrtnHdr) !=
			XIH_PH_ATTRB_PRTN_OWNER_PMCFW)
	{
		/* If the partition doesn't belong to PMCFW, skip the partition */
		XPmcFw_Printf(DEBUG_GENERAL, "Skipping the Prtn 0x%08x\n\r",
				PrtnNum);
		Status = XPMCFW_SUCCESS_NOT_PRTN_OWNER;
		goto END;
	}

	/* Validate the fields of partition */
	Status = XilPdi_ValidatePrtnHdr(PrtnHdr);
	if (XPMCFW_SUCCESS != Status)
	{
		goto END;
	}

END:
	return Status;
}

/*****************************************************************************/
/**
 * This function initializes the TCM memory
 *
 * @param	PmcFwInstancePtr is pointer to the XPmcFw Instance
 *
 * @param	CpuNo Cpu Number
 *
 * @return	returns the error codes described in xpmcfw_error.h
 *****************************************************************************/
XStatus XPmcFw_TcmInit(XPmcFw * PmcFwInstancePtr, u32 CpuNo)
{
	XStatus Status;

	switch (CpuNo)
	{
		case XIH_PH_ATTRB_DSTN_CPU_R5_0:
		if ((PmcFwInstancePtr->EccStatus &
		     XPMCFW_ECC_R5_0_TCM_INIT) != XPMCFW_ECC_R5_0_TCM_INIT)
		{
			XPmcFw_Printf(DEBUG_INFO, "Initializing R5-0 TCM \r\n");
			Status = XPmcFw_EccInit((u64)XPMCFW_R5_0_TCMA_BASE_ADDR,
						XPMCFW_R5_0_TCM_LEN/2);
			Status |= XPmcFw_EccInit((u64)XPMCFW_R5_0_TCMB_BASE_ADDR,
						 XPMCFW_R5_0_TCM_LEN/2);
		}
		PmcFwInstancePtr->EccStatus |= XPMCFW_ECC_R5_0_TCM_INIT;
		break;

		case XIH_PH_ATTRB_DSTN_CPU_R5_1:
		if ((PmcFwInstancePtr->EccStatus &
		     XPMCFW_ECC_R5_1_TCM_INIT) != XPMCFW_ECC_R5_1_TCM_INIT)
		{
			XPmcFw_Printf(DEBUG_INFO, "Initializing R5-1 TCM \r\n");
			Status = XPmcFw_EccInit((u64)XPMCFW_R5_1_TCMA_BASE_ADDR,
						XPMCFW_R5_0_TCM_LEN/2);
			Status |= XPmcFw_EccInit(XPMCFW_R5_1_TCMB_BASE_ADDR,
						 XPMCFW_R5_0_TCM_LEN/2);
		}
		PmcFwInstancePtr->EccStatus |= XPMCFW_ECC_R5_1_TCM_INIT;
		break;

		case XIH_PH_ATTRB_DSTN_CPU_R5_L:
		if ((PmcFwInstancePtr->EccStatus &
		     XPMCFW_ECC_R5_L_TCM_INIT) != XPMCFW_ECC_R5_L_TCM_INIT)
		{
			XPmcFw_Printf(DEBUG_INFO, "Initializing R5-L TCM \r\n");
			Status = XPmcFw_EccInit((u64)XPMCFW_R5_0_TCMA_BASE_ADDR,
						XPMCFW_R5_0_TCM_LEN*2);
		}
		PmcFwInstancePtr->EccStatus |= XPMCFW_ECC_R5_L_TCM_INIT;
		break;

		default:
		Status=XPMCFW_SUCCESS;
		break;
	}

	return Status;
}

/*****************************************************************************/
/**
 * This function copies the partition to specified destination
 *
 * @param	PmcFwInstancePtr is pointer to the XPmcFw Instance
 *
 * @param	PrtnNum is the partition number in the image to be loaded
 *
 * @return	returns the error codes described in xpmcfw_error.h
 *****************************************************************************/
static XStatus XPmcFw_PrtnCopy(XPmcFw * PmcFwInstancePtr, u32 PrtnNum)
{
	XStatus Status;
	u32 SrcAddr;
	u64 DestAddr;
	u32 Len;
	u32 DstnCpu;
	XilPdi_PrtnHdr * PrtnHdr;
	XCrx Crx={0};
	u8 Sha3CheckSum = FALSE;
#ifdef XPMCFW_SECURE
	u8 Sha3Hash[XSECURE_SHA3_LEN];
	u32 ChecksumAddr;
#endif

	/* Assign the partition header to local variable */
	PrtnHdr = &(PmcFwInstancePtr->MetaHdr.PrtnHdr[PrtnNum]);

	SrcAddr = PmcFwInstancePtr->MetaHdr.FlashOfstAddr +
		((PrtnHdr->DataWordOfst) * XIH_PRTN_WORD_LEN);
	DestAddr = PrtnHdr->DstnLoadAddr;

	if (((PrtnHdr->PrtnAttrb & XIH_PH_ATTRB_CHECKSUM_MASK)
				== XIH_PH_ATTRB_HASH_SHA3)) {
#ifdef XPMCFW_SECURE
		/* If checksum is enabled */
		Sha3CheckSum = TRUE;
#else
		XPmcFw_Printf(DEBUG_INFO, "\n PMCFW_SECURE_EXCLUDE is enabled \n\r");
		Status = XPMCFW_ERR_SECURE_ISNOT_EN;
		goto END;

#endif
	}

	if ((PrtnHdr->PrtnAttrb & XIH_PH_ATTRB_RSA_SIGNATURE) != 0x0) {
#ifdef XPMCFW_SECURE
		if (Sha3CheckSum == TRUE) {
			Status = XPMCFW_ERR_AUTHDEC_NOTALLOW;
			goto END;
		}
		/* If Authentication is enabled and encryption may or many not be enabled */
		Len = (PrtnHdr->TotalDataWordLen) * XIH_PRTN_WORD_LEN - XSECURE_AUTH_CERT_MIN_SIZE;
#else
		XPmcFw_Printf(DEBUG_INFO, "\n PMCFW_SECURE_EXCLUDE is enabled \n\r");
		Status = XPMCFW_ERR_SECURE_ISNOT_EN;
		goto END;
#endif
	}
	else if ((PrtnHdr->PrtnAttrb & XIH_PH_ATTRB_ENCRYPTION) != 0x00) {
#ifdef XPMCFW_SECURE
		if (Sha3CheckSum == TRUE) {
			Status = XPMCFW_ERR_AUTHDEC_NOTALLOW;
			goto END;
		}
		/* If Authentication is not enabled and encryption is only enabled */
		Len = (PrtnHdr->TotalDataWordLen) * XIH_PRTN_WORD_LEN;
#else
		XPmcFw_Printf(DEBUG_INFO, "\n PMCFW_SECURE_EXCLUDE is enabled \n\r");
		Status = XPMCFW_ERR_SECURE_ISNOT_EN;
		goto END;
#endif
	}
	else if (Sha3CheckSum == TRUE) {
		Len = (PrtnHdr->TotalDataWordLen) * XIH_PRTN_WORD_LEN;
	}
	else {
		/* For Non-secure image */
		Len = (PrtnHdr->UnEncDataWordLen) * XIH_PRTN_WORD_LEN;
		/* Make Length 16byte aligned
		 * TODO remove this after partition len is made
		 * 16byte aligned by bootgen*/
		if (Len%XPMCFW_DMA_LEN_ALIGN != 0U) {
			Len = Len + XPMCFW_DMA_LEN_ALIGN - (Len%XPMCFW_DMA_LEN_ALIGN);
		}
	}

	/* For PSM, take PSM out of reset and put in sleep mode */
	DstnCpu = XilPdi_GetDstnCpu(PrtnHdr);
	if (DstnCpu == XIH_PH_ATTRB_DSTN_CPU_PSM)
	{
		XCrx_RelPsm();
	}

	/* Check if R5 App memory is TCM, Copy to global TCM memory MAP */
	if (( (DstnCpu == XIH_PH_ATTRB_DSTN_CPU_R5_0) ||
	         (DstnCpu == XIH_PH_ATTRB_DSTN_CPU_R5_1) ||
	         (DstnCpu == XIH_PH_ATTRB_DSTN_CPU_R5_L) ) &&
	      (DestAddr <= XPMCFW_R5_L_TCM_LEN) )
	{
		/* TODO check for length */
		/* if TCM memory is present, taking R5 out of reset
		 * with LOVEC and placing it in halt mode */
		Crx.Halt = XCRX_CPU_HALT;
		Crx.VInitHi = XCRX_CPU_VINITHI_LOVEC;
		if (DstnCpu == XIH_PH_ATTRB_DSTN_CPU_R5_L)
		{
			Crx.CpuNo = XCRX_RPU_CPU_L;
			DestAddr += XPMCFW_R5_0_TCMA_BASE_ADDR;
		} else if (DstnCpu == XIH_PH_ATTRB_DSTN_CPU_R5_0) {
			Crx.CpuNo = XCRX_RPU_CPU_0;
			DestAddr += XPMCFW_R5_0_TCMA_BASE_ADDR;
		} else {
			Crx.CpuNo = XCRX_RPU_CPU_1;
			DestAddr += XPMCFW_R5_1_TCMA_BASE_ADDR;
		}
		XCrx_RelRpu(&Crx);

		/* Initialize TCM if not done */
		Status = XPmcFw_TcmInit(PmcFwInstancePtr, DstnCpu);
		if (XPMCFW_SUCCESS != Status)
		{
			XPmcFw_Printf(DEBUG_GENERAL, "TCM Init Failed \n\r");
			goto END;
		}
	}

	Status = PmcFwInstancePtr->DeviceOps.Copy(SrcAddr, DestAddr, Len, 0x0U);
	if (XPMCFW_SUCCESS != Status)
	{
		XPmcFw_Printf(DEBUG_GENERAL, "Device Copy Failed \n\r");
		goto END;
	}

	/* Security */
#ifdef XPMCFW_SECURE
	/* check sum calculation */
	if (Sha3CheckSum == TRUE) {
		ChecksumAddr = PmcFwInstancePtr->MetaHdr.FlashOfstAddr +
				((PrtnHdr->ChecksumWordOfst) * XIH_PRTN_WORD_LEN);
		/* Copy checksum hash to local buffer */
		Status = PmcFwInstancePtr->DeviceOps.Copy(ChecksumAddr,
					(u64)(UINTPTR)Sha3Hash,
					XSECURE_SHA3_LEN, 0x0U);
		if (XPMCFW_SUCCESS != Status) {
			XPmcFw_Printf(DEBUG_GENERAL,
				"Device Copy of check sum hash is failed \n\r");
			goto END;
		}
		Status = XSecure_CheckSum(NULL, DestAddr, Len, Sha3Hash);
		if (Status != XST_SUCCESS) {
			XPmcFw_Printf(DEBUG_INFO, "Failed at checksum\n\r");
			goto END;
		}
		else {
			XPmcFw_Printf(DEBUG_INFO,
				 "Checksum verification of the partition is successful \n\r");
		}
	}
	if ((PrtnHdr->PrtnAttrb & XIH_PH_ATTRB_RSA_SIGNATURE) != 0x0) {
		XPmcFw_Printf(DEBUG_GENERAL, "\nAuthenticating the partition \n\r");

		SrcAddr = PmcFwInstancePtr->MetaHdr.FlashOfstAddr +
				((PrtnHdr->AuthCertificateOfst) * XIH_PRTN_WORD_LEN);

		/* Copy AC to local buffer */
		Status = PmcFwInstancePtr->DeviceOps.Copy(SrcAddr, (u64)(UINTPTR)AcBuffer,
					XSECURE_AUTH_CERT_MIN_SIZE, 0x0U);
		if (XPMCFW_SUCCESS != Status)
		{
			XPmcFw_Printf(DEBUG_GENERAL, "Device Copy of Auth certificate is failed \n\r");
			goto END;
		}

		Status = XSecure_PrtnAuth(DestAddr, AcBuffer, Len);
		if (Status != XST_SUCCESS) {
			XPmcFw_Printf(DEBUG_INFO, "Authentication of the partition is failed \n\r");
			goto END;
		}
		XPmcFw_Printf(DEBUG_INFO, "Authentication of the partition is successful \n\r");

	}
	if ((PrtnHdr->PrtnAttrb & XIH_PH_ATTRB_ENCRYPTION) != 0x00) {
		Status = XSecure_PrtnDec((u8 *)(PmcFwInstancePtr->MetaHdr.BootHdr.SecureHdrIv),
				DestAddr,
				PrtnHdr->UnEncDataWordLen * XIH_PRTN_WORD_LEN, PmcFwInstancePtr->MetaHdr.BootHdr.EncStatus);
		if (Status != XST_SUCCESS) {
			XPmcFw_Printf(DEBUG_INFO, "Decryption of the partition is failed \n\r");
			goto END;
		}
		XPmcFw_Printf(DEBUG_INFO, "Decryption of the partition is successful \n\r");
	}
#endif

END:
	return Status;
}

/*****************************************************************************/
/**
 * This function validates the partition
 *
 * @param	PmcFwInstancePtr is pointer to the XPmcFw Instance
 *
 * @param	PrtnNum is the partition number in the image to be loaded
 *
 * @return	returns the error codes described in xpmcfw_error.h on any error
 *			returns XPMCFW_SUCCESS on success
 *
 *****************************************************************************/
static XStatus XPmcFw_PrtnValidation(XPmcFw * PmcFwInstancePtr, u32 PrtnNum)
{
	XStatus Status;
#if 0
	XilPdi_PrtnHdr * PrtnHdr;

	/* Assign the partition header to local variable */
	PrtnHdr = &(PmcFwInstancePtr->ImgHdr.PrtnHdr[PrtnNum]);
#endif
	/* Validate the partition */

	/* Update the handoff values */
	XPmcFw_UpdateHandoffParam(PmcFwInstancePtr, PrtnNum);

	Status = XPMCFW_SUCCESS;
	return Status;
}

/****************************************************************************/
/**
 * This function is used to update the handoff parameters
 *
 * @param	PmcFwInstancePtr is pointer to the PmcFw Instance
 *
 * @param	PrtnNum is the partition number in the image to be loaded
 *
 * @return	None
 *
 * @note
 *
 *****************************************************************************/
static void XPmcFw_UpdateHandoffParam(XPmcFw * PmcFwInstancePtr, u32 PrtnNum)
{
	u32 DstnCpu;
	u32 CpuNo;
	XilPdi_PrtnHdr * PrtnHdr;

	/* Assign the partition header to local variable */
	PrtnHdr = &(PmcFwInstancePtr->MetaHdr.PrtnHdr[PrtnNum]);
	DstnCpu = XilPdi_GetDstnCpu(PrtnHdr);

	if ((DstnCpu > XIH_PH_ATTRB_DSTN_CPU_NONE) &&
	    (DstnCpu <= XIH_PH_ATTRB_DSTN_CPU_PSM))
	{
		CpuNo = PmcFwInstancePtr->NoOfHandoffCpus;
		if (XPmcFw_CheckHandoffCpu(PmcFwInstancePtr,
				DstnCpu) == XPMCFW_SUCCESS)
		{
			/* Update the CPU settings */
			PmcFwInstancePtr->HandoffParam[CpuNo].CpuSettings =
					XilPdi_GetDstnCpu(PrtnHdr) |
					XilPdi_GetA72ExecState(PrtnHdr) |
					XilPdi_GetVecLocation(PrtnHdr);
			PmcFwInstancePtr->HandoffParam[CpuNo].HandoffAddr =
					PrtnHdr->DstnExecutionAddr;
			PmcFwInstancePtr->NoOfHandoffCpus += 1U;
		}
	}
}

/****************************************************************************/
/**
 * This function is used to check whether cpu has handoff address stored
 * in the handoff structure
 *
 * @param PmcFwInstancePtr is pointer to the PmcFw Instance
 *
 * @param DstnCpu is the cpu which needs to be checked
 *
 * @return
 *		- XPMCFW_SUCCESS if cpu handoff address is not present
 *		- XPMCFW_FAILURE if cpu handoff address is present
 *
 * @note
 *
 *****************************************************************************/
static XStatus XPmcFw_CheckHandoffCpu (XPmcFw * PmcFwInstancePtr, u32 DstnCpu)
{
	u32 ValidHandoffCpuNo;
	XStatus Status;
	u32 Index;
	u32 CpuId;


	ValidHandoffCpuNo = PmcFwInstancePtr->NoOfHandoffCpus;

	for (Index=0U;Index<ValidHandoffCpuNo;Index++)
	{
		CpuId = PmcFwInstancePtr->HandoffParam[Index].CpuSettings &
			XIH_PH_ATTRB_DSTN_CPU_MASK;
		if (CpuId == DstnCpu)
		{
			Status = XPMCFW_FAILURE;
			goto END;
		}
	}

	Status = XPMCFW_SUCCESS;
END:
	return Status;
}

/****************************************************************************/
/**
 * This function is used to process the CFI partition. It copies and
 * validates if security is enabled.
 *
 * @param	PmcFwInstancePtr is pointer to the PmcFw Instance
 *
 * @param	PrtnNum is the partition number in the image to be loaded
 *
 * @return
 *		- XPMCFW_SUCCESS on Success
 *		- ErrorCode as specified in xpmcfw_err.h
 *
 * @note
 *
 *****************************************************************************/
static XStatus XPmcFw_LoadFabricData (XPmcFw * PmcFwInstancePtr, u32 PrtnNum)
{
	XStatus Status;
	XilPdi_PrtnHdr * PrtnHdr;
	u32 SrcAddr;
	u32 Len;
	u32 RemLen;
	u32 ChunkSize;
	u8 IsSecure = 0;
	u8 Sha3CheckSum = 0;
#ifdef XPMCFW_SECURE
	XSecure_Partition SecureCfi = {0};
	XSecure_Aes AesInstance;
	u8 Sha3Hash[XSECURE_SHA3_LEN];
	u32 ChecksumAddr;
#endif

	/* Assign the partition header to local variable */
	PrtnHdr = &(PmcFwInstancePtr->MetaHdr.PrtnHdr[PrtnNum]);

	/* Read from CFI after the header */
	SrcAddr = PmcFwInstancePtr->MetaHdr.FlashOfstAddr +
			((PrtnHdr->DataWordOfst) * XIH_PRTN_WORD_LEN);
	Len = ((PrtnHdr->UnEncDataWordLen) * XIH_PRTN_WORD_LEN);
	/* Make Length 16byte aligned
	 * TODO remove this after partition len is made
	 * 16byte aligned by bootgen*/
	if (Len%XPMCFW_DMA_LEN_ALIGN != 0U) {
		Len = Len - (Len%XPMCFW_DMA_LEN_ALIGN) + XPMCFW_DMA_LEN_ALIGN;
	}

	if (((PrtnHdr->PrtnAttrb & XIH_PH_ATTRB_CHECKSUM_MASK)
				== XIH_PH_ATTRB_HASH_SHA3)) {
#ifdef XPMCFW_SECURE
		/* If checksum is enabled */
		Sha3CheckSum = TRUE;
#else
		XPmcFw_Printf(DEBUG_INFO, "\n PMCFW_SECURE_EXCLUDE is enabled \n\r");
		Status = XPMCFW_ERR_SECURE_ISNOT_EN;
		goto END;
#endif
	}

	if ((PrtnHdr->PrtnAttrb & XIH_PH_ATTRB_ENCRYPTION) != 0x00) {
#ifdef XPMCFW_SECURE
		if (Sha3CheckSum == TRUE) {
			Status = XPMCFW_ERR_AUTHDEC_NOTALLOW;
			goto END;
		}
		SecureCfi.IsEncrypted = TRUE;
		IsSecure = TRUE;
#else
		XPmcFw_Printf(DEBUG_INFO, "\n PMCFW_SECURE_EXCLUDE is enabled \n\r");
		Status = XPMCFW_ERR_SECURE_ISNOT_EN;
		goto END;
#endif
	}

	if ((PrtnHdr->PrtnAttrb & XIH_PH_ATTRB_RSA_SIGNATURE) != 0x0) {
#ifdef XPMCFW_SECURE
		SecureCfi.IsAuthenticated = TRUE;
		IsSecure = TRUE;
#else
		XPmcFw_Printf(DEBUG_INFO, "\n PMCFW_SECURE_EXCLUDE is enabled \n\r");
		Status = XPMCFW_ERR_SECURE_ISNOT_EN;
		goto END;
#endif
	}

#ifdef XPMCFW_SECURE
	SecureCfi.TotalSize = ((PrtnHdr->TotalDataWordLen) *
							XIH_PRTN_WORD_LEN);

	/*If authentication is enabled */
	if (SecureCfi.IsAuthenticated != 0x0) {
			SecureCfi.PlAuth.AcOfset = PmcFwInstancePtr->MetaHdr.FlashOfstAddr +
				((PrtnHdr->AuthCertificateOfst) * XIH_PRTN_WORD_LEN);

			SecureCfi.PlAuth.BlockSize =
						((PrtnHdr->PrtnAttrb) & 0x700000) >> 20;
			if (SecureCfi.PlAuth.BlockSize != 0x00U) {
				SecureCfi.PlAuth.BlockSize =
						((u32)2 << SecureCfi.PlAuth.BlockSize) * 1024 * 1024;
			}
	}

	/* If encryption is enabled */
	if (SecureCfi.IsEncrypted != 0x00) {
		/* Initialize AES */
		XSecure_AesInit(&AesInstance, PmcFwInstancePtr->MetaHdr.BootHdr.EncStatus,
			(u8 *)(PmcFwInstancePtr->MetaHdr.BootHdr.SecureHdrIv));
		SecureCfi.PlEncrypt.SecureAes = &AesInstance;
	}

	if ((SecureCfi.IsEncrypted == TRUE) || (SecureCfi.IsAuthenticated == TRUE)) {
		/* COPY cfi data to PRAM */
		if ((PmcFwInstancePtr->PrimaryBootDevice == XPMCFW_SBI_JTAG_BOOT_MODE) ||
			(PmcFwInstancePtr->PrimaryBootDevice == XPMCFW_SMAP_BOOT_MODE)) {
			/* Send the CFI data from SBI to DDR */

			if (SecureCfi.IsAuthenticated == TRUE) {
				/* Discard Authentication Certificate of Metaheader */
				Status = PmcFwInstancePtr->DeviceOps.Copy(SrcAddr,
						(u64 )XPMCFW_PMCRAM_BASEADDR,
						XSECURE_AUTH_CERT_MIN_SIZE, 0);
			}

			Status = PmcFwInstancePtr->DeviceOps.Copy(
					SrcAddr, (u64 )XPMCFW_PMCRAM_BASEADDR, SecureCfi.TotalSize, 0);
			if (XPMCFW_SUCCESS != Status)
			{
				goto END;
			}
			SecureCfi.StartAddress = XPMCFW_PMCRAM_BASEADDR;
			SecureCfi.PlAuth.AcOfset = XPMCFW_PMCRAM_BASEADDR + (SecureCfi.TotalSize - XSECURE_AUTH_CERT_MIN_SIZE);
			SecureCfi.DeviceCopy = XPmcFw_MemCopySecure;

		}
		else {
			SecureCfi.StartAddress = SrcAddr;
			SecureCfi.PlAuth.AcOfset = SecureCfi.PlAuth.AcOfset;
			SecureCfi.DeviceCopy = PmcFwInstancePtr->DeviceOps.Copy;
		}
	}
	if (Sha3CheckSum == TRUE) {
		ChecksumAddr = PmcFwInstancePtr->MetaHdr.FlashOfstAddr +
						((PrtnHdr->ChecksumWordOfst) * XIH_PRTN_WORD_LEN);
		/* Copy checksum hash to local buffer */
		Status = PmcFwInstancePtr->DeviceOps.Copy(ChecksumAddr,
							(u64)(UINTPTR)Sha3Hash,
							XSECURE_SHA3_LEN, 0x0U);
		if (XPMCFW_SUCCESS != Status) {
			XPmcFw_Printf(DEBUG_GENERAL,
				"Device Copy of check sum hash is failed \n\r");
			goto END;
		}
		/* Verify checksum of the partition */
		Status = XSecure_CheckSum(PmcFwInstancePtr->DeviceOps.Copy, SrcAddr,
							((PrtnHdr->TotalDataWordLen) *
							XIH_PRTN_WORD_LEN) , Sha3Hash);
		if (Status != XST_SUCCESS) {
			XPmcFw_Printf(DEBUG_INFO, "Failed at checksum\n\r");
			goto END;
		}
		else {
			XPmcFw_Printf(DEBUG_INFO,
				"Checksum verification of the partition is successful \n\r");
		}
	}
#endif

	/* TODO: For SD/eMMC boot mode, if DDR present
	 * copy from flash to DDR, then DDR to CFU */
	RemLen = Len;
	ChunkSize = MAX_CHUNK_SIZE;
	while (RemLen > 0U)
	{
		if (RemLen < ChunkSize) {
			ChunkSize = RemLen;
		}

		/* To load non-secure bitstream */
		if (IsSecure == FALSE) {
			/* if DDR not present, read data in chunks
			 * copy from flash to PMC RAM, then PMC RAM to CFU */
			Status = PmcFwInstancePtr->DeviceOps.Copy(SrcAddr,
					(u64 )XPMCFW_PMCRAM_BASEADDR, ChunkSize, 0x0U);
		}
		/* To load secure bitstream */
		else {
#ifdef XPMCFW_SECURE
			XPmcFw_Printf(DEBUG_INFO, "Loading secure bitstream\n\r");
			SecureCfi.ChunkBuffer = (u8 *)XPMCFW_PMCRAM_BASEADDR;
			SecureCfi.ChunkSize = MAX_CHUNK_SIZE;
			Status = XPmcFw_ProcessSecurePrtn(&SecureCfi);
			if (Status != XST_SUCCESS) {
				XPmcFw_Printf(DEBUG_INFO, "Error in loading secure bitstream\n\r");
				goto END;
			}
#endif
		}
		Status |= XPmcFw_DmaXfr((u64 )XPMCFW_PMCRAM_BASEADDR, (u64 )CFU_STREAM_ADDR,
				ChunkSize/4U, XPMCFW_PMCDMA_0 | XPMCFW_DST_CH_AXI_FIXED);
		if (XPMCFW_SUCCESS != Status)
		{
			goto END;
		}

		RemLen -= ChunkSize;
		SrcAddr += ChunkSize;
	}


	/* Check for Fabric Errors */
	Status = XPmcFw_CheckFabricErr();
	if (XPMCFW_SUCCESS != Status)
	{
		goto END;
	}
END:
	return Status;
}


/****************************************************************************/
/**
 * This function is used to process the CFI partition. It copies and
 * validates if security is enabled.
 *
 * @param	PmcFwInstancePtr is pointer to the PmcFw Instance
 *
 * @param	PrtnNum is the partition number in the image to be loaded
 *
 * @return
 *		- XPMCFW_SUCCESS on Success
 *		- ErrorCode as specified in xpmcfw_err.h
 *
 * @note
 *
 *****************************************************************************/
static XStatus XPmcFw_ProcessCfi (XPmcFw * PmcFwInstancePtr, u32 PrtnNum)
{
	XStatus Status;

	XPmcFw_Printf(DEBUG_INFO, "Processing CFI partition \n\r");

	if(PmcFwInstancePtr->PlCleaningDone == FALSE)
	{
		 /* Check for PL PowerUp */
         Status = XPmcFw_UtilPollForMask(PMC_GLOBAL_PL_STATUS,
			PMC_GLOBAL_PL_STATUS_POR_PL_B_MASK, 0x10000000U);
        if(Status != XPMCFW_SUCCESS)
        {
		Status = XPMCFW_ERR_PL_NOT_PWRUP;
		goto END;
        }

		Status = XPmcFw_PreCfgPL(PmcFwInstancePtr);
		if(Status != XST_SUCCESS)
		{
			goto END;
		}
	}

	if(PmcFwInstancePtr->PartialPdi == FALSE)
	{
		XPmcFw_FabricGlblSeqInit();
	}

	/* Start the PL global sequence */
	Status = XPmcFw_FabricPrepare();
	if (XPMCFW_SUCCESS != Status)
	{
		goto END;
	}

	/* Process the DMA part of bit stream */
	Status = XPmcFw_LoadFabricData(PmcFwInstancePtr, PrtnNum);
	if (XPMCFW_SUCCESS != Status)
	{
		goto END;
	}

	/* Set the CFU registers after CFI loading */
	Status = XPmcFw_FabricStartSeq();
	if (XPMCFW_SUCCESS != Status)
	{
		goto END;
	}

	PlCfiPresent = TRUE;

END:
	return Status;
}

/****************************************************************************/
/**
 * This function is used to process the CDO partition. It copies and
 * validates if security is enabled.
 *
 * @param	PmcFwInstancePtr is pointer to the PmcFw Instance
 *
 * @param	PrtnNum is the partition number in the image to be loaded
 *
 * @return
 *		- XPMCFW_SUCCESS on Success
 *		- ErrorCode as specified in xpmcfw_err.h
 *
 * @note
 *
 *****************************************************************************/
static XStatus XPmcFw_ProcessCdo (XPmcFw * PmcFwInstancePtr, u32 PrtnNum)
{
	XStatus Status;
	XilPdi_PrtnHdr * PrtnHdr;
	u8 Sha3CheckSum = 0;
#ifdef XPMCFW_SECURE
	XSecure_Partition SecureCdo = {0};
	XSecure_Aes AesInstance;
	u8 Sha3Hash[XSECURE_SHA3_LEN];
	u32 ChecksumAddr;
#endif

	/* Assign the partition header to local variable */
	PrtnHdr = &(PmcFwInstancePtr->MetaHdr.PrtnHdr[PrtnNum]);

	/* Update Source offset and Length */
	XilCdoPrtnInst.SrcAddr = PmcFwInstancePtr->MetaHdr.FlashOfstAddr +
		((PrtnHdr->DataWordOfst) * XIH_PRTN_WORD_LEN);
	XilCdoPrtnInst.ActualLen = XilCdoPrtnInst.Len =
		(PrtnHdr->UnEncDataWordLen * XIH_PRTN_WORD_LEN);
	/* Make Length 16byte aligned.
	 * TODO remove this after partition len is made
	 * 16byte aligned by bootgen*/
	if (XilCdoPrtnInst.Len%XPMCFW_DMA_LEN_ALIGN != 0U) {
		XilCdoPrtnInst.Len = XilCdoPrtnInst.Len -
			(XilCdoPrtnInst.Len%XPMCFW_DMA_LEN_ALIGN) +
			XPMCFW_DMA_LEN_ALIGN;
	}
	XPmcFw_Printf(DEBUG_INFO, "\nLength is %0x \n\r", XilCdoPrtnInst.Len);

	if (((PrtnHdr->PrtnAttrb & XIH_PH_ATTRB_CHECKSUM_MASK)
				== XIH_PH_ATTRB_HASH_SHA3)) {
#ifdef XPMCFW_SECURE
		/* If checksum is enabled */
		Sha3CheckSum = TRUE;
#else
		XPmcFw_Printf(DEBUG_INFO, "\n PMCFW_SECURE_EXCLUDE is enabled \n\r");
		Status = XPMCFW_ERR_SECURE_ISNOT_EN;
		goto END;

#endif
	}
	XilCdoPrtnInst.Offset = 0U;
	XilCdoPrtnInst.CdoCopy = PmcFwInstancePtr->DeviceOps.Copy;
	XilCdoPrtnInst.CdoBuf = (u32*)XPMCFW_PMCRAM_BASEADDR;

#ifdef XPMCFW_SECURE
	SecureCdo.TotalSize = ((PrtnHdr->TotalDataWordLen) *
							XIH_PRTN_WORD_LEN);

	XilCdoPrtnInst.SecureCdo = &SecureCdo;
#endif

	/*If authentication is enabled */
	if ((PrtnHdr->PrtnAttrb & XIH_PH_ATTRB_RSA_SIGNATURE) != 0x0) {
#ifdef XPMCFW_SECURE
		if (Sha3CheckSum == TRUE) {
			Status = XPMCFW_ERR_AUTHDEC_NOTALLOW;
			goto END;
		}
		SecureCdo.IsAuthenticated = TRUE;
		SecureCdo.PlAuth.AcOfset = PmcFwInstancePtr->MetaHdr.FlashOfstAddr +
			((PrtnHdr->AuthCertificateOfst) * XIH_PRTN_WORD_LEN);

		SecureCdo.PlAuth.BlockSize = ((PrtnHdr->PrtnAttrb) & 0x700000) >> 20;
		if (SecureCdo.PlAuth.BlockSize != 0x00U) {
			SecureCdo.PlAuth.BlockSize = ((u32)2 << SecureCdo.PlAuth.BlockSize) * 1024 * 1024;
		}
#else
		XPmcFw_Printf(DEBUG_INFO, "\n PMCFW_SECURE_EXCLUDE is enabled \n\r");
		Status = XPMCFW_ERR_SECURE_ISNOT_EN;
		goto END;
#endif

	}

	/* If encryption is enabled */
	if ((PrtnHdr->PrtnAttrb & XIH_PH_ATTRB_ENCRYPTION) != 0x00) {
#ifdef XPMCFW_SECURE
		if (Sha3CheckSum == TRUE) {
			Status = XPMCFW_ERR_AUTHDEC_NOTALLOW;
			goto END;
		}
		SecureCdo.IsEncrypted = TRUE;
		/* Initialize AES */
		XSecure_AesInit(&AesInstance, PmcFwInstancePtr->MetaHdr.BootHdr.EncStatus,
							(u8 *)(PmcFwInstancePtr->MetaHdr.BootHdr.SecureHdrIv));
		SecureCdo.PlEncrypt.SecureAes = &AesInstance;
#else
		XPmcFw_Printf(DEBUG_INFO, "\n PMCFW_SECURE_EXCLUDE is enabled \n\r");
		Status = XPMCFW_ERR_SECURE_ISNOT_EN;
		goto END;
#endif
	}

#ifdef XPMCFW_SECURE
	if ((SecureCdo.IsEncrypted == TRUE) || (SecureCdo.IsAuthenticated == TRUE)) {
		XPmcFw_Printf(DEBUG_INFO, "Loading secure CDO\n\r");
		/* COPY cdo data to DDR */
		if ((PmcFwInstancePtr->PrimaryBootDevice == XPMCFW_SBI_JTAG_BOOT_MODE) ||
			(PmcFwInstancePtr->PrimaryBootDevice == XPMCFW_SMAP_BOOT_MODE)) {
			/* Send the CFI data from SBI to DDR address */
			Status = PmcFwInstancePtr->DeviceOps.Copy(
					XilCdoPrtnInst.SrcAddr, (u64 )XPMCFW_DDR_TEMP_ADDR, SecureCdo.TotalSize, 0);
			if (XPMCFW_SUCCESS != Status)
			{
				goto END;
			}
			XilCdoPrtnInst.SecureCdo->StartAddress = XPMCFW_DDR_TEMP_ADDR;
			XilCdoPrtnInst.SecureCdo->PlAuth.AcOfset = XPMCFW_DDR_TEMP_ADDR + (SecureCdo.PlAuth.AcOfset - XilCdoPrtnInst.SrcAddr);
			SecureCdo.DeviceCopy = XPmcFw_MemCopySecure;

		}
		else {
			XilCdoPrtnInst.SecureCdo->StartAddress = XilCdoPrtnInst.SrcAddr;
			XilCdoPrtnInst.SecureCdo->PlAuth.AcOfset = SecureCdo.PlAuth.AcOfset;
			XilCdoPrtnInst.SecureCdo->DeviceCopy = XilCdoPrtnInst.CdoCopy;
		}
	}
	if (Sha3CheckSum == TRUE) {
		ChecksumAddr = PmcFwInstancePtr->MetaHdr.FlashOfstAddr +
					((PrtnHdr->ChecksumWordOfst) * XIH_PRTN_WORD_LEN);
		/* Copy checksum hash to local buffer */
		Status = PmcFwInstancePtr->DeviceOps.Copy(ChecksumAddr,
						 (u64)(UINTPTR)Sha3Hash,
						XSECURE_SHA3_LEN, 0x0U);
		if (XPMCFW_SUCCESS != Status) {
			XPmcFw_Printf(DEBUG_GENERAL,
				 "Device Copy of check sum hash is failed \n\r");
			goto END;
		}
		/* Verify checksum of the partition */
		Status = XSecure_CheckSum(PmcFwInstancePtr->DeviceOps.Copy,
						XilCdoPrtnInst.SrcAddr,
						((PrtnHdr->TotalDataWordLen) *
						XIH_PRTN_WORD_LEN), Sha3Hash);
		if (Status != XST_SUCCESS) {
			XPmcFw_Printf(DEBUG_INFO, "Failed at checksum\n\r");
			goto END;
		}
		else {
			XPmcFw_Printf(DEBUG_INFO,
				 "Checksum verification of the partition is successful \n\r");
		}
	}
#endif

	/** Copy the partition chunk to buffer */
	Status = XilCdo_CopyCdoBuf();
	if (XST_SUCCESS != Status)
	{
		goto END;
	}
#ifdef NPI_TRACE_ENABLE
/* This function is for internal use only, to log NPI/CDO data */
/* TODO Need to remove this function after completion of emulation verification */
	XPmcFw_Printf(DEBUG_INFO, "Trace the CDO Data \n\r");
    XilNpi_Trace(XilCdoPrtnInst.Len);
#endif
	Status = XilCdo_ProcessCdo(XilCdoPrtnInst.CdoBuf);
	if (XST_SUCCESS != Status)
	{
		goto END;
	}
END:
	return Status;
}

/****************************************************************************/
/**
 * This function is used to process the partition. It copies and validates if
 * security is enabled.
 *
 * @param	PmcFwInstancePtr is pointer to the PmcFw Instance
 *
 * @param	PrtnNum is the partition number in the image to be loaded
 *
 * @return
 *		- XPMCFW_SUCCESS on Success
 *		- ErrorCode as specified in xpmcfw_err.h
 *
 * @note
 *
 *****************************************************************************/
static XStatus XPmcFw_ProcessPrtn (XPmcFw * PmcFwInstancePtr, u32 PrtnNum)
{
	XStatus Status;
	XilPdi_PrtnHdr * PrtnHdr;
	u32 PrtnType;

	/* Assign the partition header to local variable */
	PrtnHdr = &(PmcFwInstancePtr->MetaHdr.PrtnHdr[PrtnNum]);

	/* Read Partition Type */
	PrtnType = XilPdi_GetPrtnType(PrtnHdr);
	if (PrtnType == XIH_PH_ATTRB_PRTN_TYPE_CFI)
	{
		Status = XPmcFw_ProcessCfi(PmcFwInstancePtr, PrtnNum);
	}
	else if(PrtnType == XIH_PH_ATTRB_PRTN_TYPE_CFI_GSC)
        {
		if (PlCfiPresent == TRUE)
                {
			XilCdo_SetGlobalSignals();
                        XilCdo_AssertGlobalSignals();
		}

		XilCdo_EnableCFUWrite();
		XilCdo_SetGSCWE();
		XilCdo_DisableCFUWrite();
		Status = XPmcFw_ProcessCfi(PmcFwInstancePtr, PrtnNum);
		XilCdo_EnableCFUWrite();
		XilCdo_ClearGSCWE();
		XilCdo_DisableCFUWrite();
		PlCfiPresent = 0U;
        }
	else if(PrtnType == XIH_PH_ATTRB_PRTN_TYPE_CFI_GSC_UNMASK)
        {
                XilCdo_EnableCFUWrite();
                XilCdo_SetGSCWE();
                XilCdo_DisableCFUWrite();
		Status = XPmcFw_ProcessCfi(PmcFwInstancePtr, PrtnNum);
                XilCdo_EnableCFUWrite();
                XilCdo_ClearGSCWE();
                XilCdo_DisableCFUWrite();
                XilCdo_ClearGlobalSignals();
		PlCfiPresent = 0U;
        }
	else if(PrtnType == XIH_PH_ATTRB_PRTN_TYPE_CDO)
	{
		Status = XPmcFw_ProcessCdo(PmcFwInstancePtr, PrtnNum);
	} else {

		XPmcFw_Printf(DEBUG_INFO, "Copying elf/data partition \n\r");
		/* Partition Copy */
		Status = XPmcFw_PrtnCopy(PmcFwInstancePtr, PrtnNum);
	}

	if (XPMCFW_SUCCESS != Status)
	{
		goto END;
	}

	/* Partition Validation */
	Status = XPmcFw_PrtnValidation(PmcFwInstancePtr, PrtnNum);
	if (XPMCFW_SUCCESS != Status)
	{
		goto END;
	}

END:
	return Status;
}

#ifdef NPI_TRACE_ENABLE
/****************************************************************************/
/**
* This function is for internal use only, to log NPI/CDO data
* TODO Need to remove this function after completion of emulation verification
*
* @param	Len length of the NPI/CDO data
*
* @return	None
*
* @note
*
*****************************************************************************/
static void XilNpi_Trace(u32 Len)
{
    u32 Addr_offset;

	for (Addr_offset = (u32)0; Addr_offset < Len; Addr_offset += 4)
		XPmcFw_Printf(DEBUG_INFO,"%08x\n",(*(u32 *)(XPMCFW_PMCRAM_BASEADDR + Addr_offset)));
}
#endif
