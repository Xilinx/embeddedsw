/******************************************************************************
*
* Copyright (C) 2015 Xilinx, Inc.  All rights reserved.
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
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* XILINX CONSORTIUM BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
* WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
* OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*
* Except as contained in this notice, the name of the Xilinx shall not be used
* in advertising or otherwise to promote the sale, use or other dealings in
* this Software without prior written authorization from Xilinx.
*
*******************************************************************************/
/*****************************************************************************/
/**
 *
 * @file xfsbl_rsa_sha.c
 *
 * This contains code for the RSA and SHA functionality.
 * If the Hash type is SHA3 then CSU h/w will be used
 * else we will use SoftSHA256 s/w library for SHA2-256.
 * For RSA-4096 we will always use CSU h/w.
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date        Changes
 * ----- ---- -------- -------------------------------------------------------
 * 1.00  kc   07/22/14  Initial release
 *
 * </pre>
 *
 * @note
 *
 ******************************************************************************/

/***************************** Include Files *********************************/
#include "xfsbl_authentication.h"
#include "xfsbl_csu_dma.h"
/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/

static u32 XFsbl_Sha3Len;
/*****************************************************************************
 *
 * @param	None
 *
 * @return	None
 *
 ******************************************************************************/
void XFsbl_Sha3Padd(u8 *Dst, u32 MsgLen)
{
	/**
	 * SHA3 HASH value is not generated correctly
	 * 		when used 2nd time on REMUS 1.9
	 */
	XFsbl_MemSet(Dst, 0, MsgLen);
	Dst[0] = 0x1;
	Dst[MsgLen -1] |= 0x80;
}
/*****************************************************************************
 *
 * @param	None
 *
 * @return	None
 *
 ******************************************************************************/
void XFsbl_Sha3Start(void)
{

	XFsbl_Sha3Len = 0;

	/**
	 * Reset SHA3 engine.
	 */
	XFsbl_Out32(CSU_SHA_RESET, CSU_SHA_RESET_RESET_MASK);
	XFsbl_Out32(CSU_SHA_RESET, 0);

	/**
	 * Configure the SSS for SHA3 hashing.
	 */
	XFsbl_SssSetup(XFsbl_SssInputSha3(XFSBL_CSU_SSS_SRC_SRC_DMA));

	/**
	 * Start SHA3 engine.
	 */
	XFsbl_Out32(CSU_SHA_START, CSU_SHA_START_START_MSG_MASK);
}

/*****************************************************************************
 *
 * @param	None
 *
 * @return	None
 *
 ******************************************************************************/
void XFsbl_Sha3Update(u8 * Data, u32 Size)
{

	XFsbl_Sha3Len += Size;

	XASSERT_VOID((Size & 3) == 0);

	XFsbl_CsuDmaStart(XFSBL_CSU_DMA_SRC, (u32 )(PTRSIZE)Data, Size);

	/* Checking the SHA3 done bit should be enough.  */
	XFsbl_CsuDmaWaitForDone(XFSBL_CSU_DMA_SRC);

}


/*****************************************************************************
 *
 * @param	None
 *
 * @return	None
 *
 ******************************************************************************/
void XFsbl_Sha3WaitForDone(void)
{
	volatile u32 Status;

	do
	{
		Status = XFsbl_In32(CSU_SHA_DONE);
	} while (CSU_SHA_DONE_SHA_DONE_MASK !=
			(Status & CSU_SHA_DONE_SHA_DONE_MASK));
}


/*****************************************************************************
 *
 * @param	None
 *
 * @return	None
 *
 ******************************************************************************/
void XFsbl_Sha3Finish(u8 *Hash)
{
	u32 *HashPtr = (u32 *)Hash;
	u32 PartialLen = XFsbl_Sha3Len % XFSBL_SHA3_BLOCK_LEN;

	/**
	 * DT 780552 (SHA3 HASH value is not generated correctly
	 * 		when used 2nd time on REMUS 1.9)
	 */
	PartialLen = (PartialLen == 0)?(XFSBL_SHA3_BLOCK_LEN) :
		(XFSBL_SHA3_BLOCK_LEN - PartialLen);

	XFsbl_Sha3Padd(XFsbl_RsaSha3Array, PartialLen);

	XFsbl_CsuDmaStart(XFSBL_CSU_DMA_SRC, (u32 )(PTRSIZE)XFsbl_RsaSha3Array,
			PartialLen | XFSBL_CSU_DMA_SIZE_EOP);

	/* Check the SHA3 DONE bit.  */
	XFsbl_Sha3WaitForDone();


	/* If requested, read out the Hash in reverse order.  */
	if (Hash)
	{
		u32 Index = 0;
		u32 Val = 0;
		for (Index=0; Index < 12; Index++)
		{
			Val = XFsbl_In32(CSU_SHA_DIGEST_0 + (Index * 4));
			HashPtr[11 - Index] = Val;
		}
	}

}

/*****************************************************************************
 *
 * @param	None
 *
 * @return	None
 *
 ******************************************************************************/
void XFsbl_Sha3Digest(const u8 *In, const u32 Size, u8 *Out)
{

	XFsbl_Sha3Start();
	XFsbl_Sha3Update((u8 *)In, Size);
	XFsbl_Sha3Finish(Out);

}
/*****************************************************************************
 *
 * @param	None
 *
 * @return	None
 *
 ******************************************************************************/
static void XFsbl_RsaPutData(u32 * WrData, u8 RamOffset)
{
	u32 Index = 0;
	u32 DataOffset = 0;
	u32 TmpIndex = 0;
	u32 Data = 0;


	/** Each of this loop will write 192 bits of data*/
	for (DataOffset = 0; DataOffset < 22; DataOffset++)
	{
		for (Index = 0; Index < 6; Index++)
		{
			TmpIndex = (DataOffset*6) + Index;
			/**
			 * DT 776670: CSU Boot ROM fails with error code 0x30E
			 * (RSA status register give done with error 0x5)
			 * Added this condition as we need only 4 bytes
			 * and rest of the data needs to be 0
			 */
			if(XFSBL_CSU_RSA_RAM_EXPO == RamOffset)
			{
				if(0 == TmpIndex )
				{
					Data = XFsbl_Htonl(*WrData);
				}
				else
				{
					Data = 0x0;
				}
			}
			else
			{
				if(TmpIndex >=128)
				{
					Data = 0x0;
				}
				else
				{
					/**
					 * The RSA data in Image is in Big Endian.
					 * So reverse it before putting in RSA memory,
					 * becasue RSA h/w expects it in Little endian.
					 */

					Data = XFsbl_Htonl(WrData[127-TmpIndex]);
				}
			}
			XFsbl_Out32((RSA_WR_DATA_0 + Index * 4), Data);
		}
		XFsbl_Out32(RSA_WR_ADDR, ((RamOffset * 22) + DataOffset));
	}
}

/*****************************************************************************
 *
 * @param	None
 *
 * @return	None
 *
 ******************************************************************************/
static void XFsbl_RsaGetData(u32 * RdData)
{
	u32 Index = 0;
	u32 DataOffset = 0;
	int TmpIndex = 0;


	/** Each of this loop will write 192 bits of data*/
	for (DataOffset = 0; DataOffset < 22; DataOffset++)
	{
		XFsbl_Out32(RSA_RD_ADDR,
				(XFSBL_CSU_RSA_RAM_RES_Y * 22) + DataOffset);

		Index = (DataOffset == 0) ? 2: 0;
		for (; Index < 6; Index++)
		{
			TmpIndex = 129 - ((DataOffset*6) + Index);
			if(TmpIndex < 0)
			{
				break;
			}
			/**
			 * The Signature digest is compared in Big endian.
			 * So becasue RSA h/w results in Little endian,
			 * reverse it after reading it from RSA memory,
			 */
			RdData[TmpIndex] = XFsbl_Htonl(
						XFsbl_In32(RSA_RD_DATA_0
							+ (Index * 4)));
		}
	}

}

/*****************************************************************************
 *
 * @param	u32 *Mod
 *
 * @return	None
 *
 * @notes	MINV is the 32-bit value of "-M mod 2**32"
 *			where M is LSB 32 bits of the original modulus
 * 			DT 776670: CSU Boot ROM fails with error code 0x30E
 * 			(RSA status register give done with error 0x5)
 *
 ******************************************************************************/

static void XFsbl_Mod32Inverse(u32 *Mod)
{
	/** Calculate the MINV*/
	u8 Count = 0;
	u32 ModVal = XFsbl_Htonl(Mod[127]);
	u32 Inv = 2 - ModVal;

	for (Count = 0; Count < 4; ++Count)
	{
		Inv = (Inv * (2- ( ModVal * Inv ) ) );
	}

	Inv = -Inv;

	/** Put the value in MINV registers*/
	XFsbl_Out32(RSA_CORE_MINV0, (Inv & 0xFF ));
	XFsbl_Out32(RSA_CORE_MINV1, ((Inv >> 8) & 0xFF ));
	XFsbl_Out32(RSA_CORE_MINV2, ((Inv >> 16) & 0xFF ));
	XFsbl_Out32(RSA_CORE_MINV3, ((Inv >> 24) & 0xFF ));
}

/*****************************************************************************
 *
 * @param	None
 *
 * @return	None
 *
 ******************************************************************************/
u32 XFsbl_RsaDecrypt(u8* EncText, u8* Mod, u8* ModExt, u8* ModExpo, u8* Result, u8 Reuse)
{
	volatile u32 Status = XFSBL_SUCCESS;
	u32 ErrorCode = XFSBL_SUCCESS;

	/**
	 * Populate the RSA parameters in h/w.
	 */

	/**
	 * We will reuse the RSA config values,
	 * incase of FSBL Signature Authentication,
	 * because Boot header Signautre Authentication,
	 * also uses the same SPK Key.
	 */
	if(0 == Reuse)
	{
		/**
		 * DT 776670: CSU Boot ROM fails with error code 0x30E
		 * (RSA status register give done with error 0x5)
		 * Initialize Modular exponentiation
		 */
		XFsbl_RsaPutData((u32 *)ModExpo, XFSBL_CSU_RSA_RAM_EXPO);


		/**
		 * Initialize Modular.
		 */
		XFsbl_RsaPutData((u32 *)Mod, XFSBL_CSU_RSA_RAM_MOD);


		/**
		 * DT 776670: CSU Boot ROM fails with error code 0x30E
		 * (RSA status register give done with error 0x5)
		 * Initialize MINV values from Mod.
		 */
		XFsbl_Mod32Inverse((u32 *)Mod);
	}

	/**
	 * Initialize Modular extension (R*R Mod M)
	 */
	XFsbl_RsaPutData((u32 *)ModExt, XFSBL_CSU_RSA_RAM_RES_Y);

	/**
	 * Initialize Digest
	 */
	XFsbl_RsaPutData((u32 *)EncText, XFSBL_CSU_RSA_RAM_DIGEST);


	/**
	 * Start the RSA operation.
	 */
	XFsbl_Out32(RSA_CORE_CTRL, XFSBL_CSU_RSA_CONTROL_MASK);

	/**
	 * Check and wait for status
	 */
	do
	{
		Status = XFsbl_In32(RSA_CORE_STATUS);
		if(RSA_CORE_STATUS_ERROR_MASK ==
			(Status & RSA_CORE_STATUS_ERROR_MASK))
		{
			ErrorCode = XFSBL_FAILURE;
			goto END;
		}
	}while (RSA_CORE_STATUS_DONE_MASK !=
		(Status & RSA_CORE_STATUS_DONE_MASK));


	/**
	 * Copy the result
	 */
	XFsbl_RsaGetData((u32 *)Result);

END:
	return ErrorCode;
}

/*****************************************************************************
 *
 * @param	None
 *
 * @return	None
 *
 ******************************************************************************/
void XFsbl_ShaDigest(const u8 *In, const u32 Size, u8 *Out, u32 HashLen)
{

	if(XFSBL_HASH_TYPE_SHA3 == HashLen)
	{
		XFsbl_Sha3Digest(In, Size, Out);
	}
}


/*****************************************************************************
 *
 * @param	None
 *
 * @return	None
 *
 ******************************************************************************/
void XFsbl_ShaStart( void * Ctx, u32 HashLen)
{
	if(XFSBL_HASH_TYPE_SHA3 == HashLen)
	{
		XFsbl_Sha3Start();
	}
}


/*****************************************************************************
 *
 * @param	None
 *
 * @return	None
 *
 ******************************************************************************/
void XFsbl_ShaUpdate( void * Ctx, u8 * Data, u32 Size, u32 HashLen)
{
	if(XFSBL_HASH_TYPE_SHA3 == HashLen)
	{
		XFsbl_Sha3Update(Data, Size);
	}
}

/*****************************************************************************
 *
 * @param	None
 *
 * @return	None
 *
 ******************************************************************************/
void XFsbl_ShaFinish( void * Ctx, u8 * Hash, u32 HashLen)
{

	if(XFSBL_HASH_TYPE_SHA3 == HashLen)
	{
		XFsbl_Sha3Finish(Hash);
	}
}
