/******************************************************************************
* Copyright (c) 2024 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
 *
 * @file xplm_hooks.h
 *
 * This file consists of hooks table related logic
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- -------------------------------------------------------
 * 1.0   bm   12/28/23 Initial Commit
 * </pre>
 *
 ******************************************************************************/

/**
 * @addtogroup spartanup_plm_apis SpartanUP PLM APIs
 * @{
 */

#ifndef XPLM_HOOKS_H
#define XPLM_HOOKS_H

/* application level headers */
#include "xil_types.h"
#include "xplm_config.h"

/* library includes */

/** Hooks table address in PMC RAM. */
#define XROM_HOOKS_TBL_BASE_ADDR	(0x0402F9C0U)

/** @cond spartanup_plm_internal */
#define XROM_PMCFW_RESERVED_WORDS	(24U)

#define XSECURE_SECURE_HDR_SIZE		(48U) /** Secure Header Size in Bytes*/
#define XSECURE_SECURE_GCM_TAG_SIZE	(16U) /** GCM Tag Size in Bytes */
#define XSECURE_SECURE_HDR_TOTAL_SIZE (XSECURE_SECURE_HDR_SIZE + XSECURE_SECURE_GCM_TAG_SIZE)
#define XSECURE_SHA3_256_HASH_LEN	(32U)
#define XROM_AES_IV_SIZE		(12U) /** AES IV Size */
/** @endcond */

/**
 * Constants to check for the Encryption status.
 */
typedef enum XEncryptionStatus_ {
	XPLM_ENC_STATUS_UN_ENCRYPTED = 0x00000000U,	/**< un-encrypted image */
	XPLM_ENC_STATUS_eFUSE_KEY = 0xA5C3C5A3U,	/**< stored in eFUSE, RED key */
	XPLM_ENC_STATUS_eFUSE_PUF_KEK = 0xA5C3C5A5U,	/**< stored in eFUSE, encrypted with PUF, BLACK key */
	XPLM_ENC_STATUS_eFUSE_FAMILY_KEK = 0xA5C3C5A7U,	/**< stored in eFUSE, encrypted with Family key, GREY key */
	XPLM_ENC_STATUS_BH_PUF_KEK = 0xA35C7C53U,	/**< stored in BH, BLACK Key */
	XPLM_ENC_STATUS_BH_FAMILY_KEK = 0xA35C7CA5U,	/**< stored in BH, GREY Key */
} XEncryptionStatus;

typedef struct XRomAuthHeader_ {
	u32 AuthAttributes;
} XRomAuthHeader;

/** Type definition for Partition Type */
typedef enum XRom_PartitionType_ {
	XROM_HASHBLOCK_PARTITION = 0x0U,/**< Hash Block Partition */
	XROM_PMC_FW_PARTITION = 0x1U,	/**< PMC firmware Partition */
	XROM_DATA_PARTITION = 0x2U,	/**< PMC Data Partition */
} XRom_PartitionType;

typedef struct XRomTmpVar_ {
	/**
	 * BH/eFUSE Auth enabled
	 * XROM_ALLFs - if Authentication is Enabled
	 * XROM_ZERO  - if Authentication is Disabled
	 */
	volatile u32 AuthEnabledTmp;
	/**
	 * eFUSE Authentication status
	 * XROM_ALLFs - if efuse Authentication is enabled
	 * XROM_ZERO  - if efuse Authentication is disabled
	 */
	volatile u32 eFUSEAuthStatusTmp;
	/**
	 * User JTAG Mode indication
	 */
	volatile u32 UserJTAGTmp;
	/**
	 * eFUSE encryption status
	 * XROM_ALLFs - if efuse Encryption is enabled
	 * XROM_ZERO  - if efuse Encryption is disabled
	 */
	volatile u32 eFUSEEncstatusTmp;
	/**
	 * BH/eFUSE encryption status
	 * XROM_ALLFs - if Encryption is Enabled
	 * XROM_ZERO  - if Encryption is Disabled
	 */
	volatile u32 EncstatusTmp;
	/**
	 * PUF Helper data Status
	 * 3 - PUFHD in boot header
	 */
	u32 PUFHDStatusTmp;
	/**
	 * KEK indication
	 */
	u32 KEKTmp;
	/**
	 * This is the bootmode value that is updated
	 * based on the boot mode register
	 */
	volatile u32 BootModeTmp;
	/**
	 * Pull key source status
	 */
	volatile u32 PullKeysourceStatusTmp;
	/**
	 * PPK Choice
	 */
	u32 PPKChTmp;
} XRomTmpVar;

typedef struct XRomSpkHeader_ {
	u32 TotalSPKSize;
	u32 SPKSize;
	u32 TotalSignatureSize;
	u32 SignatureSize;
	u32 SPKId;
	u32 SPKPriv;
	u32 Reserved[2];
} XRomSpkHeader;

typedef enum {
	XSECURE_HASH_INVALID_MODE 	= 0x0U,
	XSECURE_SHA3_256			= 0x1U,
	XSECURE_SHAKE_256			= 0x2U
} XSecure_ShaMode;

typedef struct XRomBootHeader_ {
	/**
	 * SMAP bus width words
	 * Offset:0
	 */

	/**
	 * Width Detection (0xAA995566)
	 * Offset:0x10
	 */
	u32 WidthDetection;
	/**
	 * Image identification ("XLNX")
	 * Offset:0x14
	 */
	u32 ImageId;
	/**
	 * AES Key Source & Type
			0x00000000 Un-Encrypted
			0xA5C3C5A3 eFUSE Key
			0xA5C3C5A5 eFUSE PUF KEK
			0xA5C3C5A7 eFUSE Family KEK
			0x3A5C3C5A BBRAM Key
			0x3A5C3C59 BBRAM PUF KEK
			0x3A5C3C57 BBRAM Family KEK
			0xA35C7C53 BH PUF KEK
			0xA35C7CA5 BH Family KEK
	 * @ref XEncryptionStatus
	 * Offset:0x18
	 */
	volatile XEncryptionStatus EncryptionStatus;
	/**
	 * Source offset of PLM in DPI
	 * Offset:0x1C
	 */
	u32 SourceOffset;
	/**
	 * Data partition load address
	 * Offset:0x20
	 */
	u32 DataPartititonLoadAddr;
	/**
	 * Data partititon length
	 * Offset:0x24
	 */
	u32 DataPartititonLen;
	/**
	 * Total Data Partition lenght
	 * including Auth & encryption overhead
	 * Offset:0x28
	 */
	u32 TotalDataPartititonLen;
	/**
	 * PLM Length
	 * Offset:0x2C
	 */
	u32 PMCFWLen;
	/**
	 * Total PLM length including
	 * Auth & Encryption overhead
	 * Offset:0x30
	 */
	u32 TotalPMCFWLen;
	/**
	 * Image Attributes
	 * [31:22]: Reserved
	 * [21:20]: 0x3 = DICE- CDI generation
	 * [19:18]: 0x3 Signed image
	 * [17:16]: 0x3 PUF 4K Mode
	 * 			All other reserved
	 * [15:14]: 0x3 BH Authentication
	 * 			All others, authentication
	 * 			will be decided by eFUSE Hash
	 * [13:12]: Reserved
	 * [11:10]: DPA CM
	 * [9:8]:	0x3 BI Integrity selection
	 * 			All others invalid
	 * [7:6]:	0x3 PUF Helper data in BH
	 * 			All others, HD is in eFUSE
	 * [5:4]:	0x3 DPI is RSA signed and
	 * 			dont decrypt the image
	 * [3:2]:	0x3 Secure header contains
	 * 			operation key for Block 0
	 * 			All others use the Red key
	 * [1:0]:	Reserved
	 *
	 * Offset:0x34
	 */
	u32 ImageAttributes;
	/**
	 * Grey/Black key
	 * Offset:0x38
	 */
	u32 GreyorBlackKey[8U];
	/**
	 * Grey/Black key IV
	 * Offset:0x58
	 */
	u32 GreyorBlackKeyIV[3U];
	/**
	 * Secure header IV
	 * Offset:0x64
	 */
	u32 SecureheaderIV[3U];
	/**
	 * Encryption revocation ID
	 * Offset:0x70
	 */
	u32 PartRevokeId;
	/**
	 * Authentication Header @ref XRomAuthHeader
	 * Offset:0x74
	 */
	XRomAuthHeader AuthHeader;
	/**
	 * HASH block Size
	 * Offset:0x78
	 */
	u32 HashBlockSize;
	/**
	 * PPK Size
	 * Offset:0x7C
	 */
	u32 TotalPPKSize;
	/**
	 * Total PPK Size
	 * Offset:0x80
	 */
	u32 PPKSize;
	/**
	 * PDI Signature Size
	 * Offset:0x84
	 */
	u32 TotalPDISignSize;
	/**
	 * PDI Signature Size
	 * Offset:0x88
	 */
	u32 PDISignSize;
	/**
	 * PUF Image identification
	 * Offset:0x8C
	 */
	u32 PUFImageId;
	/**
	 * Shutter Value
	 * Offset:0x90
	 */
	u32 PUFShutVal;
	/**
	 * RO Val
	 * Offset:0x94
	 */
	u32 PUFROVal;
	/**
	 * HD Len
	 * Offset:0x98
	 */
	u32 PUFHDLen;
	/**
	 * ROM Reserved Area
	 * Future Use
	 * Offset:0x9C
	 */
	u32 ROMReserved[15U];
	/**
	 * User Defined Revision
	 * Offset:0xD8
	 */
	u32 UserDefRev;
	/**
	 * PLM Reserved Area
	 * Offset:0xD8
	 */
	u32 PMCFWReserved[XROM_PMCFW_RESERVED_WORDS];
	/**
	 * Reg Init
	 * Offset:0x13C
	 */
	u32 RegInitData[128U];
	/**
	 * Checksum from 0x10 to 0x11FB
	 * Offset:0x33C
	 */
	u32 HeaderChecksum;
	/**
	 * End: 0x340
	 */
} XRomBootHeader;
/**
 * Type Definition for Boot ROM struct
 */
typedef struct XRomBootRom_ {
	/**
	 * This is the PLM Image Boot header structure member.
	 * It holds the full boot header.
	 * It will be used through out the PMC flow.
	 */
	XRomBootHeader *ImageHeader;

	volatile u32 BootStage;
	/**
	 * This is the boot device related device
	 * initialization function pointer
	 */
	u32(*DeviceInit)(u32 Flags);
	/**
	 * This is the boot device related device
	 * read access function pointer
	 */
	u32(*DeviceRead)(u64 Src, u32 Dest, u32 Size, u32 Flags);
	/**
	 * This is the boot device related private data
	 */
	u32 DeviceData;
	/**
	 * This is the bootmode value that is updated
	 * based on the boot mode register
	 */
	u32 BootMode;
	/**
	 * eFUSE Authentication status
	 * XROM_ALLFs - if efuse Authentication is enabled
	 * XROM_ZERO  - if efuse Authentication is disabled
	 */
	volatile u32 eFUSEAuthStatus;
	/**
	 * This member tells us the current error code
	 * that will be updated into error status register.
	 */
	u32 ErrorCode;

	/**
	 * This member tells us the the current value
	 * of the multi boot offset. This will be updated based
	 * on the Multi boot offset register.
	 */
	u32 MultibootOffset;

	/**
	 * eFUSE encryption status
	 * XROM_ALLFs - if efuse Encryption is enabled
	 * XROM_ZERO  - if efuse Encryption is disabled
	 */
	volatile u32 eFUSEEncstatus;

	/**
	 * DPA Counter measure status
	 * XROM_ZERO - DPA Counter measure Disabled
	 * otherwise  - Enabled
	 * if enabled through boot header it's value will be 0x3
	 */
	u32 DpaCmStatus;

	/**
	 * KEK indication
	 */
	u32 KEK;
	/**
	 * BH/eFUSE Auth enabled
	 * XROM_ALLFs - if Authentication is Enabled
	 * XROM_ZERO  - if Authentication is Disabled
	 */
	volatile u32 AuthEnabled;
	/**
	 * PUF Helper data Status
	 * 3 - PUFHD in boot header
	 */
	u32 PUFHDStatus;
	/**
	 * BH/eFUSE encryption status
	 * XROM_ALLFs - if Encryption is Enabled
	 * XROM_ZERO  - if Encryption is Disabled
	 */
	volatile u32 Encstatus;
	/**
	 * PUF Mode
	 * 3 - 4K Mode
	 */
	u32 PUFMode;
	/**
	 * User JTAG Mode indication
	 */
	u32 UserJTAG;
	/**
	 * Manufacturing State
	 * Secure boot not allowed indication
	 */
	u32 ManufacState;
	/**
	 * Secure lock down indication
	 */
	u32 SecureLockDown;
	/**
	 * HASH Algo selected
	 */
	XSecure_ShaMode HashAlgo;
	/**
	 * Public Algo
	 */
	u32 PublicAlgo;
	/**
	 * Hash algo digest len
	 */
	u32 HashDigestLen;
	/**
	 * PPK Choice
	 */
	u32 PPKCh;
	/**
	 * Secure Lock down error
	 */
	u32 SLError;

	/**
	 * This member tells us the the current value
	 * of the boot image offset.
	 * This is updated based on the multiboot_offset member.
	 * image_offset = multiboot_offset * 32KByte.
	 */
	u64 ImageOffset;

	/**
	 * This member can be either XROM_ALLFs or XROM_ZERO.
	 * By default it is always XROM_ALLFs.
	 * It is made XROM_ZERO when the ROM flow wants to do
	 * image search again in a POR image search condition.
	 * This member is made XROM_ZERO in STAGE4,
	 * the Secure/Non-Secure checks are
	 * performed on the new values of boot header.
	 */
	u32 FirstImage;

	/**
	* PPK Authentication status
	* XROM_ALLFS - PPK Authentication successful
	* XROM_ZERO -  PPK Authentication unsuccessful
	*/
	u32 PPKAuthStatus;
	/**
	* SPK Authentication status
	* XROM_ALLFS - SPK Authentication successful
	* XROM_ZERO -  SPK Authentication unsuccessful
	*/
	u32 SPKAuthStatus;
	/**
	* Hash Block Authentication status
	* XROM_ALLFS - Hash Block Authentication successful
	* XROM_ZERO -  Hash Block Authentication unsuccessful
	*/
	u32 HashBlockAuthStatus;
	/**
	 * Pull key source status
	 */
	volatile u32 PullKeysourceStatus;

	/**
	 * FW Data - this is used by FW during Hooks
	 */
	void *FwData;
	/**
	 * Init Vector buffer to be used during AES operations
	 */
	u8 InitVector[XROM_AES_IV_SIZE];
	/**
	 * Flash Opcode
	 */
	u32 FlashOpcode;
	/**
	*  PufhdDigestStatus - This is used to skip Helper data digest validation or not
	*/
	volatile u32 PufhdValidateDigest;
	/**
	*  Ppk2OrPufHd - This is used to indicate PPK2 Hash or PUF HD Hash
	*/
	volatile u32 Ppk2OrPufHd;
} XRomBootRom;

/** Instance to process authentication/decryption/integrity of image in chunks */
typedef struct XRomSecureChunk_ {
	u8 SecureHeader[XSECURE_SECURE_HDR_TOTAL_SIZE]; /**< Secure Header Buffer */
	u8 ExpHash[XSECURE_SHA3_256_HASH_LEN]; /**< ExpHash Buffer */
	u32 IsLastChunk;		/**< Is last chunk or not */
	XRom_PartitionType PartitionType;		/**< Is Data partition or PMC FW ? */
	u32 ChunkNum;			/**< The current chunk being processed */
	u64 SrcAddr;			/**< Image offset on boot device */
	u32 RemainingDataLen;	/**< Always holds the total remaining data to be processed */
	u32 DstAddr;			/**< Destination address */
	u32 SecureDataLen;		/**< Always holds the chunk length to be processed,
							 *  after processing holds actual data length by
							 *  removing overheads
							 */
	u32 KeySource; 			/**< Device key source */
	u32 DataCopied;			/**< Size of the data copied to destination */
	u8 *ScratchPadBuf;		/**< pointer to the scratch pad area */
} XRomSecureChunk;

/** Placeholder to reserve unused variables in ROM hooks table. */
typedef void *XUnused_Var_t;

/** Placeholder to reserve unused APIs in ROM hooks table. */
typedef void (*XUnused_Func_t)(void);

typedef struct XRom_HooksTbl_ {
	/* Nested HooksTbl Pointers */
	XUnused_Func_t UnusedA[2U];

	/* Data Variable Pointers */
	XRomBootRom *InstancePtr;
	XUnused_Var_t UnusedB;

	/* Function Pointers */
	void (*XRom_Initialize_Instance)(XRomBootRom *InstancePtr);
	u32 (*XRom_CaptureeFUSEAttribute)(XRomBootRom *InstancePtr);
	u32 (*XRom_PlHouseClean)(void);
	void (*XRom_CaptureImageAttributes)(XRomBootRom *InstancePtr);
	u32 (*XRom_HashAlgoSelectValidation)(XRomBootRom *const InstancePtr);
	u32 (*XRom_AuthDataValidation)(XRomBootRom *InstancePtr);
	XUnused_Func_t UnusedC[3U];
	u32 (*XRom_CheckRevocationID)(u32 RevokeId);
	u32 (*XRom_VerifyAllPPKHash)(XRomBootRom *InstancePtr, const u8 *const KeyPtr);
	u32 (*XRom_InitChunkInstance)(const XRomBootRom *InstancePtr,
				      XRomSecureChunk *ChunkInstPtr, XRom_PartitionType PartitionType);
	u32 (*XRom_ValidateBootheaderIntegrity)(void);
	u32 (*XRom_LoadSecureChunk)(XRomBootRom *InstancePtr, XRomSecureChunk *ChunkInstPtr);
	XUnused_Func_t UnusedD[3U];
	u32 (*XRom_SecureLoad)(XRomBootRom *InstancePtr, XRomSecureChunk *ChunkInstPtr);
	u32 (*XRom_ProcessChunk)(const XRomBootRom *InstancePtr,
				 XRomSecureChunk *ChunkInstPtr);

	XRomTmpVar *(*XRom_GetTemporalInstance)(void);
	XUnused_Func_t UnusedE;
	u32 (*XRom_ShaDigestCalculation)(const u8 *const InData, const u32 Size,
					 const XSecure_ShaMode Mode, u8 *Digest);
	XUnused_Func_t UnusedF[4U];
	u32 (*XRom_ValidateHBAad)(XRomBootRom *InstancePtr, const u8 *GcmTag);
	u32 (*XRom_PullKeySource)(const XRomBootRom *InstancePtr, u32 *KeySource);
	void (*XRom_MBistNScanClear)(void);
	void (*XRom_ClearCrypto)(void);
	XUnused_Func_t UnusedG[2U];
} XRom_HooksTbl;

extern XRom_HooksTbl *HooksTbl;

#endif /* XPLM_HOOKS_H */

/** @} end of spartanup_plm_apis group*/
