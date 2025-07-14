/******************************************************************************
* Copyright (C) 2025 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
 *
 * @file xilnvm_efuse_versal_ssit_client_example.c
 * @addtogroup xnvm_efuse_versal_ssit_client_example	XilNvm eFuse API Usage
 * @{
 *
 * This file illustrates Basic eFuse read/write using rows on Primary SLR0, Secondary SLR1, Secondary
 * SLR2 and Secondary SLR3.
 * To build this application, xilmailbox library must be included in BSP and
 * xilnvm library must be in client mode.
 *
 * This example is supported for Versal SSIT devices.
 *
 * Procedure to link and compile the example for the default ddr less designs
 * ------------------------------------------------------------------------------------------------------------
 * The default linker settings places a software stack, heap and data in DDR memory. For this example to work,
 * any data shared between client running on A72/R5/PL and server running on PMC, should be placed in area
 * which is accessible to both client and server.
 *
 * Following is the procedure to compile the example on OCM or any memory region which can be accessed by server
 *
 *		1. Open example linker script(lscript.ld) in Vitis project and section to memory mapping should
 *			be updated to point all the required sections to shared memory(OCM or TCM)
 *			using a memory region drop down selection
 *
 *						OR
 *
 *		1. In linker script(lscript.ld) user can add new memory section in source tab as shown below
 *			.sharedmemory : {
 *   			. = ALIGN(4);
 *   			__sharedmemory_start = .;
 *   			*(.sharedmemory)
 *   			*(.sharedmemory.*)
 *   			*(.gnu.linkonce.d.*)
 *   			__sharedmemory_end = .;
 *  			} > versal_cips_0_pspmc_0_psv_ocm_ram_0_psv_ocm_ram_0
 *
 * 		2. In this example, ".data" section elements that are passed by reference to the server side
 * 			should be stored in the above shared memory section. To make it happen in below example,
 * 			replace ".data" in attribute section with ".sharedmemory". For example,
 * 			static SharedMem[XNVM_TOTAL_SHARED_MEM_SIZE] __attribute__((aligned(64U)))
 * 					__attribute__((section(".data.SharedMem")));
 * 			should be changed to
 * 			static SharedMem[XNVM_TOTAL_SHARED_MEM_SIZE] __attribute__((aligned(64U)))
 * 					__attribute__((section(".sharedmemory.SharedMem")));
 *
 * To keep things simple, by default the cache is disabled for this example
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver	 Who	Date	Changes
 * ----- ---  -------- -------------------------------------------------------
 * 3.5    hj   02/05/2025 First release
 * 3.5    hj   14/04/2025 Added support for unique PPK hash programming for each SLR
 * 3.6    hj   18/06/2025 Added support to program unique value of efuse for each SLR
 *
 * </pre>
 *
 ******************************************************************************/

/***************************** Include Files *********************************/

#include "xnvm_efuseclient.h"
#include "xilnvm_efuse_versal_ssit_input.h"
#include "xil_util.h"
#include "xil_cache.h"
#include "xnvm_common_defs.h"

/***************** Macros (Inline Functions) Definitions *********************/
#define XNVM_EFUSE_AES_KEY_STRING_LEN			(64U)
#define XNVM_EFUSE_PPK_HASH_STRING_LEN			(64U)
#define XNVM_EFUSE_ROW_STRING_LEN			(8U)
#define XNVM_EFUSE_GLITCH_WR_LK_MASK			(0x80000000U)
#define XNVM_EFUSE_DEC_EFUSE_ONLY_MASK			(0x0000ffffU)
#define XNVM_EFUSE_TEMP_VOLT_LIMIT_MAX			(3U)
#define XNVM_EFUSE_VOLT_SOC_LIMIT			(1U)

#define XNVM_256_BITS_AES_KEY_LEN_IN_BYTES (256U / XIL_SIZE_OF_BYTE_IN_BITS)
#define XNVM_256_BITS_AES_KEY_LEN_IN_CHARS (\
					XNVM_256_BITS_AES_KEY_LEN_IN_BYTES * 2U)
#define XNVM_128_BITS_AES_KEY_LEN_IN_BYTES (128U / XIL_SIZE_OF_BYTE_IN_BITS)
#define XNVM_128_BITS_AES_KEY_LEN_IN_CHARS (\
					XNVM_128_BITS_AES_KEY_LEN_IN_BYTES * 2U)
#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define MAX4(a, b, c, d) MAX(MAX(a, b), MAX(c, d))
#define XNVM_MAX_NO_USER_FUSES MAX4(XNVM_EFUSE_SLR_0_NUM_OF_USER_FUSES,	\
				 XNVM_EFUSE_SLR_1_NUM_OF_USER_FUSES,	\
				 XNVM_EFUSE_SLR_2_NUM_OF_USER_FUSES,	\
				 XNVM_EFUSE_SLR_3_NUM_OF_USER_FUSES)

#define XNVM_USER_FUSES_MAX_SIZE_IN_BYTES (XNVM_MAX_NO_USER_FUSES * sizeof(u32))
#define XNVM_MAX_AES_KEY_LEN_IN_CHARS	XNVM_256_BITS_AES_KEY_LEN_IN_CHARS
#define XNVM_CACHE_LINE_LEN		(64U)
#define XNVM_CACHE_ALIGNED_LEN		(XNVM_CACHE_LINE_LEN * 2U)
#define Align(Size)		(Size + (XNVM_WORD_LEN - ((Size % 4 == 0U)?XNVM_WORD_LEN: (Size % XNVM_WORD_LEN))))
#define XNVM_SHARED_BUF_SIZE	(Align(sizeof(XNvm_EfuseIvs)) + Align(sizeof(XNvm_EfuseDataAddr)) + \
				Align(sizeof(XNvm_EfuseGlitchCfgBits)) + Align(sizeof(XNvm_EfuseAesKeys)) + \
				Align(sizeof(XNvm_EfusePpkHash)) + Align(sizeof(XNvm_EfuseDecOnly)) + \
				Align(sizeof(XNvm_EfuseMiscCtrlBits)) + Align(sizeof(XNvm_EfuseSecCtrlBits)) + \
				Align(sizeof(XNvm_EfuseRevokeIds)) + Align(sizeof(XNvm_EfuseBootEnvCtrlBits)) + \
				Align(sizeof(XNvm_EfuseSecMisc1Bits)) + Align(sizeof(XNvm_EfuseUserDataAddr)) + \
				Align(sizeof(XNvm_EfuseOffChipIds)) + \
				XNVM_USER_FUSES_MAX_SIZE_IN_BYTES)
#ifdef XNVM_EN_ADD_PPKS
#define XNVM_SHARED_BUF_TOTAL_SIZE (XNVM_SHARED_BUF_SIZE + Align(sizeof(XNvm_EfuseAdditionalPpkHash)))
#else
#define XNVM_SHARED_BUF_TOTAL_SIZE XNVM_SHARED_BUF_SIZE
#endif

#define XNVM_TOTAL_SHARED_MEM_SIZE	(XNVM_SHARED_BUF_TOTAL_SIZE + XNVM_SHARED_MEM_SIZE)
#define XNVM_EFUSE_PPK_READ_START XNVM_EFUSE_PPK0
#ifdef XNVM_EN_ADD_PPKS
#define XNVM_EFUSE_PPK_READ_END XNVM_EFUSE_PPK4
#else
#define XNVM_EFUSE_PPK_READ_END XNVM_EFUSE_PPK2
#endif
#define XNVM_MAX_SLR_NUM XNVM_SLR_INDEX_3 /**< Max SLR number */


/**************************** Type Definitions *******************************/
XNvm_ClientInstance NvmClientInstance;

/**************************** Variable Definitions ***************************/

/*
 * if cache is enabled, User need to make sure the data is aligned to cache line
 */

/* shared memory allocation */
static u8 SharedMem[XNVM_TOTAL_SHARED_MEM_SIZE] __attribute__((aligned(64U)))
		__attribute__((section(".data.SharedMem")));

#define DEFINE_CHAR_ARR(_Arrname, Size, Param)	\
	static const char _Arrname[XNVM_MAX_SLRS][Size + 1U]	\
	__attribute__((aligned(64U)))__attribute__((section(".data.XNVM_U8"))) = {	\
		XNVM_EFUSE_SLR_0_##Param,	\
		XNVM_EFUSE_SLR_1_##Param,	\
		XNVM_EFUSE_SLR_2_##Param,	\
		XNVM_EFUSE_SLR_3_##Param	\
	};

DEFINE_CHAR_ARR(Ppk0Hash, XNVM_EFUSE_PPK_HASH_STRING_LEN, PPK0_HASH);
DEFINE_CHAR_ARR(Ppk1Hash, XNVM_EFUSE_PPK_HASH_STRING_LEN, PPK1_HASH);
DEFINE_CHAR_ARR(Ppk2Hash, XNVM_EFUSE_PPK_HASH_STRING_LEN, PPK2_HASH);
#ifdef XNVM_EN_ADD_PPKS
DEFINE_CHAR_ARR(Ppk3Hash, XNVM_EFUSE_PPK_HASH_STRING_LEN, PPK3_HASH);
DEFINE_CHAR_ARR(Ppk4Hash, XNVM_EFUSE_PPK_HASH_STRING_LEN, PPK4_HASH);
#endif
DEFINE_CHAR_ARR(GlitchCfg, XNVM_EFUSE_ROW_STRING_LEN, GLITCH_CFG);
DEFINE_CHAR_ARR(AesKey, XNVM_EFUSE_AES_KEY_STRING_LEN, AES_KEY);
DEFINE_CHAR_ARR(UserKey0, XNVM_EFUSE_AES_KEY_STRING_LEN, USER_KEY_0);
DEFINE_CHAR_ARR(UserKey1, XNVM_EFUSE_AES_KEY_STRING_LEN, USER_KEY_1);
DEFINE_CHAR_ARR(MetaHeaderIv, XNVM_IV_STRING_LEN, META_HEADER_IV);
DEFINE_CHAR_ARR(BlackObfusIv, XNVM_IV_STRING_LEN, BLACK_OBFUS_IV);
DEFINE_CHAR_ARR(PlmIv, XNVM_IV_STRING_LEN, PLM_IV);
DEFINE_CHAR_ARR(DataPartitionIv, XNVM_IV_STRING_LEN, DATA_PARTITION_IV);
DEFINE_CHAR_ARR(RevokeId0, XNVM_EFUSE_ROW_STRING_LEN, REVOCATION_ID_0_FUSES);
DEFINE_CHAR_ARR(RevokeId1, XNVM_EFUSE_ROW_STRING_LEN, REVOCATION_ID_1_FUSES);
DEFINE_CHAR_ARR(RevokeId2, XNVM_EFUSE_ROW_STRING_LEN, REVOCATION_ID_2_FUSES);
DEFINE_CHAR_ARR(RevokeId3, XNVM_EFUSE_ROW_STRING_LEN, REVOCATION_ID_3_FUSES);
DEFINE_CHAR_ARR(RevokeId4, XNVM_EFUSE_ROW_STRING_LEN, REVOCATION_ID_4_FUSES);
DEFINE_CHAR_ARR(RevokeId5, XNVM_EFUSE_ROW_STRING_LEN, REVOCATION_ID_5_FUSES);
DEFINE_CHAR_ARR(RevokeId6, XNVM_EFUSE_ROW_STRING_LEN, REVOCATION_ID_6_FUSES);
DEFINE_CHAR_ARR(RevokeId7, XNVM_EFUSE_ROW_STRING_LEN, REVOCATION_ID_7_FUSES);
DEFINE_CHAR_ARR(OffChipRevokeId0, XNVM_EFUSE_ROW_STRING_LEN, OFFCHIP_REVOKE_ID_0_FUSES);
DEFINE_CHAR_ARR(OffChipRevokeId1, XNVM_EFUSE_ROW_STRING_LEN, OFFCHIP_REVOKE_ID_1_FUSES);
DEFINE_CHAR_ARR(OffChipRevokeId2, XNVM_EFUSE_ROW_STRING_LEN, OFFCHIP_REVOKE_ID_2_FUSES);
DEFINE_CHAR_ARR(OffChipRevokeId3, XNVM_EFUSE_ROW_STRING_LEN, OFFCHIP_REVOKE_ID_3_FUSES);
DEFINE_CHAR_ARR(OffChipRevokeId4, XNVM_EFUSE_ROW_STRING_LEN, OFFCHIP_REVOKE_ID_4_FUSES);
DEFINE_CHAR_ARR(OffChipRevokeId5, XNVM_EFUSE_ROW_STRING_LEN, OFFCHIP_REVOKE_ID_5_FUSES);
DEFINE_CHAR_ARR(OffChipRevokeId6, XNVM_EFUSE_ROW_STRING_LEN, OFFCHIP_REVOKE_ID_6_FUSES);
DEFINE_CHAR_ARR(OffChipRevokeId7, XNVM_EFUSE_ROW_STRING_LEN, OFFCHIP_REVOKE_ID_7_FUSES);

#define DEFINE_CHAR_POINTER_ARR(_Arrname, Param)	\
		static const char *_Arrname[XNVM_MAX_SLRS]	\
		__attribute__((aligned(64U)))__attribute__((section(".data.XNVM_U8_POINTER"))) = {	\
			XNVM_EFUSE_SLR_0_##Param,	\
			XNVM_EFUSE_SLR_1_##Param,	\
			XNVM_EFUSE_SLR_2_##Param,	\
			XNVM_EFUSE_SLR_3_##Param	\
		};

DEFINE_CHAR_POINTER_ARR(UserFuses, USER_FUSES);

#ifdef XNVM_ACCESS_PUF_USER_DATA
DEFINE_CHAR_POINTER_ARR(PufFuses, PUF_FUSES);
#endif

#define DEFINE_U8_VAL(_Arrname, Param)	\
	static const u8 _Arrname[XNVM_MAX_SLRS]	\
	__attribute__((aligned(64U)))__attribute__((section(".data.XNVM_U8"))) = {	\
		XNVM_EFUSE_SLR_0_##Param,	\
		XNVM_EFUSE_SLR_1_##Param,	\
		XNVM_EFUSE_SLR_2_##Param,	\
		XNVM_EFUSE_SLR_3_##Param	\
	};

DEFINE_U8_VAL(SysmonTempHotVal, SYSMON_TEMP_HOT_FUSES);
DEFINE_U8_VAL(SysmonVoltPmcVal, SYSMON_VOLT_PMC_FUSES);
DEFINE_U8_VAL(SysmonVoltPslpVal, SYSMON_VOLT_PSLP_FUSE);
DEFINE_U8_VAL(SysmonTempColdVal, SYSMON_TEMP_COLD_FUSES);
DEFINE_U8_VAL(NumOfUserFusesVal, NUM_OF_USER_FUSES);
DEFINE_U8_VAL(ReadNumOfUserFusesVal, READ_NUM_OF_USER_FUSES);

#ifdef XNVM_ACCESS_PUF_USER_DATA
DEFINE_U8_VAL(NumOfPufFusesVal, NUM_OF_PUF_FUSES);
DEFINE_U8_VAL(ReadNumOfPufFusesVal, READ_NUM_OF_PUF_FUSES);

#endif

#define DEFINE_FLAG_ARR(_Arrname, FLAG) 	\
	static const u8 _Arrname[XNVM_MAX_SLRS]	\
	__attribute__((aligned(64U)))__attribute__((section(".data.XNVM_FLAG"))) = {	\
		XNVM_EFUSE_SLR_0_##FLAG,	\
		XNVM_EFUSE_SLR_1_##FLAG,	\
		XNVM_EFUSE_SLR_2_##FLAG,	\
		XNVM_EFUSE_SLR_3_##FLAG	\
	};

DEFINE_FLAG_ARR(EnableProgrammingFlag, ENABLE_PROGRAMMING);
DEFINE_FLAG_ARR(RegInitDisFlag, REG_INIT_DIS);
DEFINE_FLAG_ARR(BootEnvWrLkFlag, BOOT_ENV_WR_LK);
DEFINE_FLAG_ARR(PmcScEnFlag, PMC_SC_EN);
DEFINE_FLAG_ARR(AuthJtagLockDisFlag, AUTH_JTAG_LOCK_DIS);
DEFINE_FLAG_ARR(AuthJtagDisFlag, AUTH_JTAG_DIS);
DEFINE_FLAG_ARR(Ppk0WrLockFlag, PPK0_WR_LK);
DEFINE_FLAG_ARR(Ppk1WrLockFlag, PPK1_WR_LK);
DEFINE_FLAG_ARR(Ppk2WrLockFlag, PPK2_WR_LK);
DEFINE_FLAG_ARR(AesCrcLkFlag, AES_CRC_LK);
DEFINE_FLAG_ARR(AesWrLkFlag, AES_WR_LK);
DEFINE_FLAG_ARR(UserKey0CrcLkFlag, USER_KEY_0_CRC_LK);
DEFINE_FLAG_ARR(UserKey0WrLkFlag, USER_KEY_0_WR_LK);
DEFINE_FLAG_ARR(UserKey1CrcLkFlag, USER_KEY_1_CRC_LK);
DEFINE_FLAG_ARR(UserKey1WrLkFlag, USER_KEY_1_WR_LK);
DEFINE_FLAG_ARR(HwTstBitsDisFlag, HWTSTBITS_DIS);
DEFINE_FLAG_ARR(JtagDisFlag, JTAG_DIS);
DEFINE_FLAG_ARR(JtagErrorOutDisFlag, JTAG_ERROR_OUT_DIS);
DEFINE_FLAG_ARR(AesDisFlag, AES_DIS);
DEFINE_FLAG_ARR(WriteGlitchCfgFlag, WRITE_GLITCH_CFG);
DEFINE_FLAG_ARR(WriteAesKeyFlag, WRITE_AES_KEY);
DEFINE_FLAG_ARR(WriteUserKey0Flag, WRITE_USER_KEY_0);
DEFINE_FLAG_ARR(WriteUserKey1Flag, WRITE_USER_KEY_1);
DEFINE_FLAG_ARR(Ppk0HashFlag, WRITE_PPK0_HASH);
DEFINE_FLAG_ARR(Ppk1HashFlag, WRITE_PPK1_HASH);
DEFINE_FLAG_ARR(Ppk2HashFlag, WRITE_PPK2_HASH);

#ifdef XNVM_EN_ADD_PPKS
DEFINE_FLAG_ARR(AddPpkEnFlag, ADD_PPK_EN);
DEFINE_FLAG_ARR(Ppk3HashFlag, WRITE_PPK3_HASH);
DEFINE_FLAG_ARR(Ppk4HashFlag, WRITE_PPK4_HASH);
DEFINE_FLAG_ARR(Ppk3InvldFlag, PPK3_INVLD);
DEFINE_FLAG_ARR(Ppk4InvldFlag, PPK4_INVLD);
#endif

DEFINE_FLAG_ARR(WriteDecOnlyFlag, WRITE_DEC_EFUSE_ONLY);
DEFINE_FLAG_ARR(WriteMetaHeaderIvFlag, WRITE_METAHEADER_IV);
DEFINE_FLAG_ARR(WriteBlackObfIvFlag, WRITE_BLACK_OBFUS_IV);
DEFINE_FLAG_ARR(WritePlmIvFlag, WRITE_PLM_IV);
DEFINE_FLAG_ARR(WriteDataPartIvFlag, WRITE_DATA_PARTITION_IV);
DEFINE_FLAG_ARR(WriteRevokeId0Flag, WRITE_REVOCATION_ID_0);
DEFINE_FLAG_ARR(WriteRevokeId1Flag, WRITE_REVOCATION_ID_1);
DEFINE_FLAG_ARR(WriteRevokeId2Flag, WRITE_REVOCATION_ID_2);
DEFINE_FLAG_ARR(WriteRevokeId3Flag, WRITE_REVOCATION_ID_3);
DEFINE_FLAG_ARR(WriteRevokeId4Flag, WRITE_REVOCATION_ID_4);
DEFINE_FLAG_ARR(WriteRevokeId5Flag, WRITE_REVOCATION_ID_5);
DEFINE_FLAG_ARR(WriteRevokeId6Flag, WRITE_REVOCATION_ID_6);
DEFINE_FLAG_ARR(WriteRevokeId7Flag, WRITE_REVOCATION_ID_7);
DEFINE_FLAG_ARR(WriteOffChipRevokeId0Flag, WRITE_OFFCHIP_REVOKE_ID_0);
DEFINE_FLAG_ARR(WriteOffChipRevokeId1Flag, WRITE_OFFCHIP_REVOKE_ID_1);
DEFINE_FLAG_ARR(WriteOffChipRevokeId2Flag, WRITE_OFFCHIP_REVOKE_ID_2);
DEFINE_FLAG_ARR(WriteOffChipRevokeId3Flag, WRITE_OFFCHIP_REVOKE_ID_3);
DEFINE_FLAG_ARR(WriteOffChipRevokeId4Flag, WRITE_OFFCHIP_REVOKE_ID_4);
DEFINE_FLAG_ARR(WriteOffChipRevokeId5Flag, WRITE_OFFCHIP_REVOKE_ID_5);
DEFINE_FLAG_ARR(WriteOffChipRevokeId6Flag, WRITE_OFFCHIP_REVOKE_ID_6);
DEFINE_FLAG_ARR(WriteOffChipRevokeId7Flag, WRITE_OFFCHIP_REVOKE_ID_7);
DEFINE_FLAG_ARR(WriteUserFuseFlag, WRITE_USER_FUSES);
DEFINE_FLAG_ARR(EnvMonitorDisFlag, ENV_MONITOR_DISABLE);
DEFINE_FLAG_ARR(GlitchDetWrLkFlag, GLITCH_DET_WR_LK);
DEFINE_FLAG_ARR(GdRomMonEnFlag, GD_ROM_MONITOR_EN);
DEFINE_FLAG_ARR(GdHaltBootEn10Flag, GD_HALT_BOOT_EN_1_0);
DEFINE_FLAG_ARR(GenErrHaltBootEn10Flag, GEN_ERR_HALT_BOOT_EN_1_0);
DEFINE_FLAG_ARR(EnvErrHaltBootEn10Flag, ENV_ERR_HALT_BOOT_EN_1_0);
DEFINE_FLAG_ARR(CryptoKatEnFlag, CRYPTO_KAT_EN);
DEFINE_FLAG_ARR(LbistEnFlag, LBIST_EN);
DEFINE_FLAG_ARR(SafetyMissionEnFlag, SAFETY_MISSION_EN);
DEFINE_FLAG_ARR(Ppk0InvldFlag, PPK0_INVLD);
DEFINE_FLAG_ARR(Ppk1InvldFlag, PPK1_INVLD);
DEFINE_FLAG_ARR(Ppk2InvldFlag, PPK2_INVLD);
DEFINE_FLAG_ARR(SysmonTempEnFlag, SYSMON_TEMP_EN);
DEFINE_FLAG_ARR(SysmonVoltEnFlag, SYSMON_VOLT_EN);
DEFINE_FLAG_ARR(SysmonTempHotFlag, SYSMON_TEMP_HOT);
DEFINE_FLAG_ARR(SysmonVoltPmcFlag, SYSMON_VOLT_PMC);
DEFINE_FLAG_ARR(SysmonVoltPslpFlag, SYSMON_VOLT_PSLP);
DEFINE_FLAG_ARR(SysmonVoltSocFlag, SYSMON_VOLT_SOC);
DEFINE_FLAG_ARR(SysmonTempColdFlag, SYSMON_TEMP_COLD);
DEFINE_FLAG_ARR(LpdMbistEnFlag, LPD_MBIST_EN);
DEFINE_FLAG_ARR(PmcMbistEnFlag, PMC_MBIST_EN);
DEFINE_FLAG_ARR(LpdNocScEnFlag, LPD_NOC_SC_EN);
DEFINE_FLAG_ARR(SysmonVoltMonEnFlag, SYSMON_VOLT_MON_EN);
DEFINE_FLAG_ARR(SysmonTempMonEnFlag, SYSMON_TEMP_MON_EN);

#ifdef XNVM_ACCESS_PUF_USER_DATA
DEFINE_FLAG_ARR(WritePufFusesFlag, WRITE_PUF_FUSES);
#endif
/************************** Function Prototypes ******************************/
static int XilNvm_EfuseWriteFuses(u32 Idx);
static int XilNvm_EfuseReadFuses(u32 Idx);
static int XilNvm_EfuseShowDna(void);
static int XilNvm_EfuseShowPpkHash(XNvm_PpkType PpkType);
static int XilNvm_EfuseShowIv(XNvm_IvType IvType);
static int XilNvm_EfuseShowRevocationId(u8 RevokeIdNum);
static int XilNvm_EfuseShowOffChipId(u8 OffChipIdNum);
static int XilNvm_EfuseShowDecOnly(void);
static int XilNvm_EfuseShowUserFuses(u32 Idx);
static int XilNvm_EfuseShowCtrlBits(void);
static int XilNvm_EfuseShowSecCtrlBits(void);
static int XilNvm_EfuseShowPufSecCtrlBits(void);
static int XilNvm_EfuseShowMiscCtrlBits(void);
static int XilNvm_EfuseShowSecMisc1Bits(void);
static int XilNvm_EfuseShowBootEnvCtrlBits(void);
static int XilNvm_EfuseInitSecCtrl(XNvm_EfuseDataAddr *WriteEfuse,
	XNvm_EfuseSecCtrlBits *SecCtrlBits, u32 Idx);
static int XilNvm_EfuseInitMiscCtrl(XNvm_EfuseDataAddr *WriteEfuse,
	XNvm_EfuseMiscCtrlBits *MiscCtrlBits, u32 Idx);
static int XilNvm_EfuseInitRevocationIds(XNvm_EfuseDataAddr *WriteEfuse,
	XNvm_EfuseRevokeIds *RevokeId, u32 Idx);
static int XilNvm_EfuseInitIVs(XNvm_EfuseDataAddr *WriteEfuse,
	XNvm_EfuseIvs *Ivs, u32 Idx);
static int XilNvm_EfuseInitGlitchData(XNvm_EfuseDataAddr *WriteEfuse,
	XNvm_EfuseGlitchCfgBits *GlitchData, u32 Idx);
static int XilNvm_EfuseInitAesKeys(XNvm_EfuseDataAddr *WriteEfuse,
	XNvm_EfuseAesKeys *AesKeys, u32 Idx);
static int XilNvm_EfuseInitPpkHash(XNvm_EfuseDataAddr *WriteEfuse,
	XNvm_EfusePpkHash *PpkHash, u32 Idx);
static int XilNvm_EfuseInitDecOnly(XNvm_EfuseDataAddr *WriteEfuse,
	XNvm_EfuseDecOnly *DecOnly, u32 Idx);
static int XilNvm_EfuseInitUserFuses(XNvm_EfuseDataAddr *WriteEfuse,
	XNvm_EfuseUserDataAddr *Data, u32 Idx);
static int XilNvm_ValidateUserFuseStr(const char *UserFuseStr);
static int XilNvm_EfuseInitOffChipRevokeIds(XNvm_EfuseDataAddr *WriteEfuse,
	XNvm_EfuseOffChipIds *OffChipIds, u32 Idx);
static int XilNvm_EfuseInitBootEnvCtrl(XNvm_EfuseDataAddr *WriteEfuse,
	XNvm_EfuseBootEnvCtrlBits *BootEnvCtrl, u32 Idx);
static int XilNvm_EfuseInitSecMisc1Ctrl(XNvm_EfuseDataAddr *WriteEfuse,
	XNvm_EfuseSecMisc1Bits *SecMisc1Bits, u32 Idx);

static int XilNvm_PrepareAesKeyForWrite(const char *KeyStr, u8 *Dst, u32 Len);
static int XilNvm_PrepareIvForWrite(const char *IvStr, u8 *Dst, u32 Len);
static int XilNvm_ValidateIvString(const char *IvStr);
static int XilNvm_ValidateHash(const char *Hash, u32 Len);
static void XilNvm_FormatData(const u8 *OrgDataPtr, u8* SwapPtr, u32 Len);
static int XilNvm_PrepareRevokeIdsForWrite(const char *RevokeIdStr,
	u32 *Dst, u32 Len);
static int XNvm_ValidateAesKey(const char *Key);
static int XNvm_InputSlrIndex(XNvm_ClientInstance *InstancePtr, u32 SlrIndex);
#ifdef XNVM_ACCESS_PUF_USER_DATA
static int XilNvm_EfuseWritePufFuses(u32 Idx);
static int XilNvm_EfuseReadPufFuses(u32 Idx);
static int XilNvm_EfuseInitPufFuses(XNvm_EfusePufFuseAddr *PufFuse, u32 Idx);
#endif
#ifdef XNVM_EN_ADD_PPKS
static int XilNvm_EfuseInitAdditionalPpkHash(XNvm_EfuseDataAddr *WriteEfuse,
		XNvm_EfuseAdditionalPpkHash *PpkHash, u32 Idx);
#endif
/*****************************************************************************/
int main(void)
{
	int Status = XST_FAILURE;
	XMailbox MailboxInstance;
	u32 Index;

	#ifdef XNVM_CACHE_DISABLE
		Xil_DCacheDisable();
	#endif

	#ifndef SDT
	Status = XMailbox_Initialize(&MailboxInstance, 0U);
	#else
	Status = XMailbox_Initialize(&MailboxInstance, XPAR_XIPIPSU_0_BASEADDR);
	#endif
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = XNvm_ClientInit(&NvmClientInstance, &MailboxInstance);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	/* Set shared memory */
	Status = XMailbox_SetSharedMem(&MailboxInstance, (u64)(UINTPTR)(SharedMem + XNVM_SHARED_BUF_TOTAL_SIZE),
			XNVM_SHARED_MEM_SIZE);
	if (Status != XST_SUCCESS) {
		xil_printf("\r\n shared memory initialization failed");
		goto END;
	}

	for (Index = XNVM_SLR_INDEX_0; Index <= XNVM_MAX_SLR_NUM; Index++) {
		if (EnableProgrammingFlag[Index] != TRUE) {
			xil_printf("\r\nSkipping SLR %d\r\n", Index);
			continue;
		}

		xil_printf("\r\nRUNNING SLR %d\r\n", Index);


		Status = XNvm_InputSlrIndex(&NvmClientInstance, Index);
		if (Status != XST_SUCCESS) {
			xil_printf("invalid SlrIndex \r\n");
			goto END;
		}

		Status = XilNvm_EfuseWriteFuses(Index);
		if (Status != XST_SUCCESS) {
			goto END;
		}

		Status = XilNvm_EfuseReadFuses(Index);
		if (Status != XST_SUCCESS) {
			goto END;
		}

#ifdef XNVM_ACCESS_PUF_USER_DATA
		Status = XilNvm_EfuseWritePufFuses(Index);
		if (Status != XST_SUCCESS) {
			goto END;
		}
		Status = XilNvm_EfuseReadPufFuses(Index);
		if (Status != XST_SUCCESS) {
			goto END;
		}
#endif
	}


END:
	Status |= XMailbox_ReleaseSharedMem(&MailboxInstance);
	if (Status != XST_SUCCESS) {
		xil_printf("\r\nVersal Efuse example failed with err: %08x\n\r",
									Status);
	}
	else {
		xil_printf("\r\nSuccessfully ran Versal Efuse example");
	}
	return Status;
}

/****************************************************************************/
/**
 * This function is used to send eFuses write request to library.
 * There are individual structures for each set of eFuses in the library and
 * one global structure which holds the pointer to the individual structures
 * to be written.
 * XNvm_EfuseDataAddr is a global structure and members of this structure will
 * be filled with the addresses of the individual structures to be written to
 * eFuse.
 * typedef struct {
 *	u64 EnvMonDisFlag;
 *	u64 AesKeyAddr;
 *	u64 PpkHashAddr;
 *	u64 DecOnlyAddr;
 *	u64 SecCtrlAddr;
 *	u64 MiscCtrlAddr;
 *	u64 RevokeIdAddr;
 *	u64 IvAddr;
 *	u64 UserFuseAddr;
 *	u64 GlitchCfgAddr;
 *	u64 BootEnvCtrlAddr;
 *	u64 Misc1CtrlAddr;
 *	u64 OffChipIdAddr;
 * } XNvm_EfuseDataAddr;
 *
 * Example :
 * IvAddr holds memory address where IVs to be written are stored.
 * If there is no request to write IVs then IvAddr in XNvm_EfuseDataAddr
 * will be NULL.
 *
 * @param 	Idx		SLR index.
 *
 * @return
 *	- XST_SUCCESS - If the write is successful
 *	- Error code - On failure.
 *
 ******************************************************************************/
static int XilNvm_EfuseWriteFuses(u32 Idx)
{
	int Status = XST_FAILURE;

	XNvm_EfuseIvs *Ivs = (XNvm_EfuseIvs*)(UINTPTR)&SharedMem[0U];
	XNvm_EfuseDataAddr *WriteEfuse = (XNvm_EfuseDataAddr*)(UINTPTR)((u8*)Ivs + Align(sizeof(XNvm_EfuseIvs)));
	XNvm_EfuseGlitchCfgBits *GlitchData = (XNvm_EfuseGlitchCfgBits*)(UINTPTR)((u8*)WriteEfuse +
			Align(sizeof(XNvm_EfuseDataAddr)));
	XNvm_EfuseAesKeys *AesKeys = (XNvm_EfuseAesKeys*)(UINTPTR)((u8*)GlitchData +
			Align(sizeof(XNvm_EfuseGlitchCfgBits)));
	XNvm_EfusePpkHash *PpkHash = (XNvm_EfusePpkHash*)(UINTPTR)((u8*)AesKeys +
			Align(sizeof(XNvm_EfuseAesKeys)));
	XNvm_EfuseDecOnly *DecOnly = (XNvm_EfuseDecOnly*)(UINTPTR)((u8*)PpkHash +
			Align(sizeof(XNvm_EfusePpkHash)));
	XNvm_EfuseMiscCtrlBits *MiscCtrlBits = (XNvm_EfuseMiscCtrlBits*)(UINTPTR)((u8*)DecOnly +
			Align(sizeof(XNvm_EfuseDecOnly)));
	XNvm_EfuseSecCtrlBits *SecCtrlBits = (XNvm_EfuseSecCtrlBits*)(UINTPTR)((u8*)MiscCtrlBits +
			Align(sizeof(XNvm_EfuseMiscCtrlBits)));
	XNvm_EfuseRevokeIds *RevokeIds = (XNvm_EfuseRevokeIds*)(UINTPTR)((u8*)SecCtrlBits +
			Align(sizeof(XNvm_EfuseSecCtrlBits)));
	XNvm_EfuseOffChipIds *OffChipIds = (XNvm_EfuseOffChipIds*)(UINTPTR)((u8*)RevokeIds +
			Align(sizeof(XNvm_EfuseRevokeIds)));
	XNvm_EfuseBootEnvCtrlBits *BootEnvCtrl = (XNvm_EfuseBootEnvCtrlBits*)(UINTPTR)((u8*)OffChipIds +
			Align(sizeof(XNvm_EfuseOffChipIds)));
	XNvm_EfuseSecMisc1Bits *SecMisc1Bits = (XNvm_EfuseSecMisc1Bits*)(UINTPTR)((u8*)BootEnvCtrl +
			Align(sizeof(XNvm_EfuseBootEnvCtrlBits)));
	XNvm_EfuseUserDataAddr *UserFuses = (XNvm_EfuseUserDataAddr*)(UINTPTR)((u8*)SecMisc1Bits +
			Align(sizeof(XNvm_EfuseSecMisc1Bits)));
	u32 *UserFusesArr = (u32*)(UINTPTR)((u8*)UserFuses + Align(sizeof(XNvm_EfuseUserDataAddr)));
	u8 *BufAddr;

#ifdef XNVM_EN_ADD_PPKS
	XNvm_EfuseAdditionalPpkHash *AdditionalPpkHash = (XNvm_EfuseAdditionalPpkHash*)(UINTPTR)((u8*)UserFusesArr +
						XNVM_USER_FUSES_MAX_SIZE_IN_BYTES);
	BufAddr = ((u8*)AdditionalPpkHash + Align(sizeof(XNvm_EfuseAdditionalPpkHash)));
#else
	BufAddr = (u8*)UserFusesArr + XNVM_USER_FUSES_MAX_SIZE_IN_BYTES;
#endif

	if (BufAddr > (SharedMem + XNVM_SHARED_BUF_TOTAL_SIZE)) {
		goto END;
	}

	/* Clear total shared memory */
	Status = Xil_SMemSet(&SharedMem[0U], XNVM_TOTAL_SHARED_MEM_SIZE, 0U,
						XNVM_TOTAL_SHARED_MEM_SIZE);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = XilNvm_EfuseInitGlitchData(WriteEfuse, GlitchData, Idx);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = XilNvm_EfuseInitAesKeys(WriteEfuse, AesKeys, Idx);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = XilNvm_EfuseInitPpkHash(WriteEfuse, PpkHash, Idx);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = XilNvm_EfuseInitDecOnly(WriteEfuse, DecOnly, Idx);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = XilNvm_EfuseInitSecCtrl(WriteEfuse, SecCtrlBits, Idx);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = XilNvm_EfuseInitMiscCtrl(WriteEfuse, MiscCtrlBits, Idx);
	if (Status != XST_SUCCESS) {
		goto END;
	}
#ifdef XNVM_EN_ADD_PPKS
	Status = XilNvm_EfuseInitAdditionalPpkHash(WriteEfuse, AdditionalPpkHash, Idx);
	if (Status != XST_SUCCESS) {
		goto END;
	}
#endif
	Status = XilNvm_EfuseInitBootEnvCtrl(WriteEfuse, BootEnvCtrl, Idx);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = XilNvm_EfuseInitSecMisc1Ctrl(WriteEfuse, SecMisc1Bits, Idx);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = XilNvm_EfuseInitRevocationIds(WriteEfuse, RevokeIds, Idx);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = XilNvm_EfuseInitIVs(WriteEfuse, Ivs, Idx);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = XilNvm_EfuseInitOffChipRevokeIds(WriteEfuse, OffChipIds, Idx);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	UserFuses->UserFuseDataAddr = (UINTPTR)UserFusesArr;
	Status = XilNvm_EfuseInitUserFuses(WriteEfuse, UserFuses, Idx);
	if(Status != XST_SUCCESS) {
		goto END;
	}
	Xil_DCacheFlushRange((UINTPTR)UserFusesArr, NumOfUserFusesVal[Idx] * sizeof(u32));

	WriteEfuse->EnvMonDisFlag = EnvMonitorDisFlag[Idx];

	Xil_DCacheFlushRange((UINTPTR)WriteEfuse, sizeof(XNvm_EfuseDataAddr));

	Status = XNvm_EfuseWrite(&NvmClientInstance, (UINTPTR)WriteEfuse);

END:
	return Status;
}

/****************************************************************************/
/**
 * This function reads all eFuses data and displays.
 *
 * @param 	Idx		SLR index.
 *
 * @return
 *	- XST_SUCCESS - If all the read requests are successful.
 *	- Error code - On failure.
 *
 ******************************************************************************/
static int XilNvm_EfuseReadFuses(u32 Idx)
{
	int Status = XST_FAILURE;
	u32 Index;
	s8 Row;

	Status = XilNvm_EfuseShowDna();
	if (Status != XST_SUCCESS) {
		goto END;
	}

	xil_printf("\n\r");

	for (Index = XNVM_EFUSE_PPK_READ_START; Index <= XNVM_EFUSE_PPK_READ_END; Index++) {
		Status = XilNvm_EfuseShowPpkHash(Index);
		if (Status != XST_SUCCESS) {
			goto END;
		}
	}

	xil_printf("\n\r");

	for (Index = XNVM_EFUSE_META_HEADER_IV_RANGE;
			Index <= XNVM_EFUSE_DATA_PARTITION_IV_RANGE; Index++) {
		Status = XilNvm_EfuseShowIv(Index);
		if (Status != XST_SUCCESS) {
			goto END;
		}
	}

	xil_printf("\n\r");

	for (Row = 0; Row < (s8)XNVM_NUM_OF_REVOKE_ID_FUSES; Row++) {
		Status = XilNvm_EfuseShowRevocationId(Row);
		if (Status != XST_SUCCESS) {
			goto END;
		}
	}

	xil_printf("\n\r");

	for (Row = 0; Row < (s8)XNVM_NUM_OF_REVOKE_ID_FUSES; Row++) {
		Status = XilNvm_EfuseShowOffChipId(Row);
		if (Status != XST_SUCCESS) {
			goto END;
		}
	}

	xil_printf("\n\r");

	Status = XilNvm_EfuseShowDecOnly();
	if (Status != XST_SUCCESS) {
		goto END;
	}

	xil_printf("\n\r");

	Status = XilNvm_EfuseShowUserFuses(Idx);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	xil_printf("\n\r");

	Status = XilNvm_EfuseShowCtrlBits();

END:
	return Status;
}

/****************************************************************************/
/**
 * This function reads DNA eFuses data and displays.
 *
 * @return
 *	- XST_SUCCESS - If the read request is successful.
 *	- Error code - On failure.
 *
 ******************************************************************************/
static int XilNvm_EfuseShowDna(void)
{
	int Status = XST_FAILURE;
	XNvm_Dna *EfuseDna = (XNvm_Dna *)(UINTPTR)&SharedMem[0U];

	Xil_DCacheInvalidateRange((UINTPTR)&SharedMem[0U],
		XNVM_CACHE_ALIGNED_LEN);

	Status = XNvm_EfuseReadDna(&NvmClientInstance, (UINTPTR)EfuseDna);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Xil_DCacheInvalidateRange((UINTPTR)&SharedMem[0U],
		XNVM_CACHE_ALIGNED_LEN);

	xil_printf("\r\nDNA:%08x%08x%08x%08x", EfuseDna->Dna[3],
			EfuseDna->Dna[2],
			EfuseDna->Dna[1],
			EfuseDna->Dna[0]);
	Status = XST_SUCCESS;

END:
	return Status;
}

/****************************************************************************/
/**
 * This function reads Ppk Hash eFuses data and displays.
 *
 * @param	PpkType		PpkType PPK0/PPK1/PPK2
 *
 * @return
 *	- XST_SUCCESS - If the read request is successful.
 *	- Error code - On failure.
 *
 ******************************************************************************/
static int XilNvm_EfuseShowPpkHash(XNvm_PpkType PpkType)
{
	int Status = XST_FAILURE;
	XNvm_PpkHash *EfusePpk = (XNvm_PpkHash *)(UINTPTR)&SharedMem[0U];
	u32 ReadPpk[XNVM_EFUSE_PPK_HASH_LEN_IN_WORDS] = {0U};
	s8 Row;

	Xil_DCacheInvalidateRange((UINTPTR)&SharedMem[0U],
			XNVM_CACHE_ALIGNED_LEN);

	Status = XNvm_EfuseReadPpkHash(&NvmClientInstance, (UINTPTR)EfusePpk, PpkType);
	if (Status != XST_SUCCESS) {
		goto END;
	}
	else {
		Xil_DCacheInvalidateRange((UINTPTR)&SharedMem[0U],
				XNVM_CACHE_ALIGNED_LEN);
		xil_printf("\n\rPPK%d:", PpkType);
		XilNvm_FormatData((u8 *)EfusePpk->Hash, (u8 *)ReadPpk,
				XNVM_EFUSE_PPK_HASH_LEN_IN_BYTES);
		for (Row = (XNVM_EFUSE_PPK_HASH_LEN_IN_WORDS - 1U);
				Row >= 0; Row--) {
			xil_printf("%08x", ReadPpk[Row]);
		}
	}
	Status = XST_SUCCESS;

END:
	return Status;
}

/****************************************************************************/
/**
 * This function reads IV eFuses data and displays.
 *
 * @param	IvType		IvType MetaHeader IV or Blk IV or Plm IV or
 * 				Data partition IV
 *
 * @return
 *	- XST_SUCCESS - If the read request is successful.
 *	- Error code - On failure.
 *
 ******************************************************************************/
static int XilNvm_EfuseShowIv(XNvm_IvType IvType)
{
	int Status = XST_FAILURE;
	XNvm_Iv *EfuseIv = (XNvm_Iv *)(UINTPTR)&SharedMem[0U];
	u32 ReadIv[XNVM_EFUSE_IV_LEN_IN_WORDS] = {0U};
	s8 Row;

	Xil_DCacheInvalidateRange((UINTPTR)&SharedMem[0U],
			XNVM_CACHE_ALIGNED_LEN);

	Status = XNvm_EfuseReadIv(&NvmClientInstance, (UINTPTR)EfuseIv, IvType);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Xil_DCacheInvalidateRange((UINTPTR)&SharedMem[0U],
			XNVM_CACHE_ALIGNED_LEN);

	xil_printf("\n\r IV%d:",IvType);

	XilNvm_FormatData((u8 *)EfuseIv->Iv, (u8 *)ReadIv,
			XNVM_EFUSE_IV_LEN_IN_BYTES);
	for (Row = (XNVM_EFUSE_IV_LEN_IN_WORDS - 1U); Row >= 0; Row--) {
		xil_printf("%08x", ReadIv[Row]);
	}
	Status = XST_SUCCESS;

END:
	return Status;
}

/****************************************************************************/
/**
 * This function reads Revocation ID eFuses data and displays.
 *
 * @param	RevokeIdNum	Revocation ID number to read
 *
 * @return
 *	- XST_SUCCESS - If the read request is successful.
 *	- Error code - On failure.
 *
 ******************************************************************************/
static int XilNvm_EfuseShowRevocationId(u8 RevokeIdNum)
{
	int Status = XST_FAILURE;
	u32 *RegData = (u32 *)(UINTPTR)&SharedMem[0U];

	Xil_DCacheInvalidateRange((UINTPTR)&SharedMem[0U],
			XNVM_CACHE_ALIGNED_LEN);

	Status = XNvm_EfuseReadRevocationId(&NvmClientInstance, (UINTPTR)RegData,
			(XNvm_RevocationId)RevokeIdNum);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Xil_DCacheInvalidateRange((UINTPTR)&SharedMem[0U],
			XNVM_CACHE_ALIGNED_LEN);

	xil_printf("RevocationId%d Fuse:%08x\n\r", RevokeIdNum, *RegData);
	Status = XST_SUCCESS;

END:
	return Status;
}

/****************************************************************************/
/**
 * This function reads OffChip ID eFuses data and displays.
 *
 * @param	OffChipIdNum	OffChip ID number to read
 *
 * @return
 *	- XST_SUCCESS - If the read request is successful.
 *	- Error code - On failure.
 *
 ******************************************************************************/
static int XilNvm_EfuseShowOffChipId(u8 OffChipIdNum)
{
	int Status = XST_FAILURE;
	u32 *RegData = (u32 *)(UINTPTR)&SharedMem[0U];

	Xil_DCacheInvalidateRange((UINTPTR)&SharedMem[0U],
			XNVM_CACHE_ALIGNED_LEN);

	Status = XNvm_EfuseReadOffchipRevokeId(&NvmClientInstance, (UINTPTR)RegData,
			(XNvm_OffchipId)OffChipIdNum);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Xil_DCacheInvalidateRange((UINTPTR)&SharedMem[0U],
			XNVM_CACHE_ALIGNED_LEN);

	xil_printf("OffChipId%d Fuse:%08x\n\r", OffChipIdNum, *RegData);
	Status = XST_SUCCESS;

END:
	return Status;
}

/****************************************************************************/
/**
 * This function reads DecOnly eFuses data and displays.
 *
 * @return
 *	- XST_SUCCESS - If the read request is successful.
 *	- Error code - On failure.
 *
 ******************************************************************************/
static int XilNvm_EfuseShowDecOnly(void)
{
	int Status = XST_FAILURE;
	u32 *RegData = (u32 *)(UINTPTR)&SharedMem[0U];

	Xil_DCacheInvalidateRange((UINTPTR)&SharedMem[0U],
			XNVM_CACHE_ALIGNED_LEN);

	Status = XNvm_EfuseReadDecOnly(&NvmClientInstance, (UINTPTR)RegData);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Xil_DCacheInvalidateRange((UINTPTR)&SharedMem[0U],
			XNVM_CACHE_ALIGNED_LEN);

	xil_printf("\r\nDec_only Fuse : %x\r\n", *RegData);
	Status = XST_SUCCESS;

END:
	return Status;
}

/****************************************************************************/
/**
 * This function reads User eFuses data and displays.
 *
 * @param 	Idx		SLR index.
 *
 * @return
 *	- XST_SUCCESS - If the read request is successful.
 *	- Error code - On failure.
 *
 ******************************************************************************/
static int XilNvm_EfuseShowUserFuses(u32 Idx)
{
	int Status = XST_FAILURE;
	XNvm_EfuseUserDataAddr *ReadUserFuses = (XNvm_EfuseUserDataAddr*)(UINTPTR)&SharedMem[0U];
	s8 Row;

	ReadUserFuses->StartUserFuseNum = XNVM_EFUSE_READ_USER_FUSE_NUM;
	ReadUserFuses->NumOfUserFuses = ReadNumOfUserFusesVal[Idx];
	ReadUserFuses->UserFuseDataAddr = (UINTPTR)(SharedMem + Align(sizeof(XNvm_EfuseUserDataAddr)));

	Xil_DCacheFlushRange((UINTPTR)(SharedMem + Align(sizeof(XNvm_EfuseUserDataAddr))),
		(ReadNumOfUserFusesVal[Idx] * sizeof(u32)));
	Xil_DCacheInvalidateRange((UINTPTR)ReadUserFuses, sizeof(XNvm_EfuseUserDataAddr));

	Status = XNvm_EfuseReadUserFuses(&NvmClientInstance, (UINTPTR)ReadUserFuses);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Xil_DCacheFlushRange((UINTPTR)(SharedMem + Align(sizeof(XNvm_EfuseUserDataAddr))),
		(ReadNumOfUserFusesVal[Idx] * sizeof(u32)));
	Xil_DCacheInvalidateRange((UINTPTR)ReadUserFuses,
		sizeof(XNvm_EfuseUserDataAddr));

	for (Row = XNVM_EFUSE_READ_USER_FUSE_NUM;
		Row < (s8)(XNVM_EFUSE_READ_USER_FUSE_NUM +
			ReadNumOfUserFusesVal[Idx]); Row++) {

		xil_printf("User%d Fuse:%08x\n\r",
			Row, *(u32 *)(UINTPTR)(ReadUserFuses->UserFuseDataAddr +
					Row - XNVM_EFUSE_READ_USER_FUSE_NUM));
	}
	xil_printf("\n\r");

	Status = XST_SUCCESS;
END:
	return Status;
}

/****************************************************************************/
/**
 * This function is used to initialize Glitch config structure with user
 * provided data and assign the same to global structure XNvm_EfuseDataAddr to
 * program below eFuses.
 * - Glitch Configuration Row write lock
 * - Glitch configuration data
 *
 * typedef struct {
 *	u8 PrgmGlitch;
 *	u8 GlitchDetWrLk;
 *	u32 GlitchDetTrim;
 *	u8 GdRomMonitorEn;
 *	u8 GdHaltBootEn;
 * }XNvm_EfuseGlitchCfgBits;
 *
 * @param	WriteEfuse	Pointer to XNvm_EfuseDataAddr structure.
 *
 * @param 	GlitchData	Pointer to XNvm_EfuseGlitchCfgBits structure.
 *
 * @param 	Idx		SLR index.
 *
 * @return
 *		- XST_SUCCESS - If initialization is successful
 *		- ErrorCode - On Failure
 *
 ******************************************************************************/
static int XilNvm_EfuseInitGlitchData(XNvm_EfuseDataAddr *WriteEfuse,
	XNvm_EfuseGlitchCfgBits *GlitchData, u32 Idx)
{
	int Status = XST_FAILURE;

	GlitchData->PrgmGlitch = WriteGlitchCfgFlag[Idx];

	if(GlitchData->PrgmGlitch == TRUE) {
		Status = Xil_ConvertStringToHex(GlitchCfg[Idx],
					&(GlitchData->GlitchDetTrim),
					XNVM_EFUSE_ROW_STRING_LEN);
		if (Status != XST_SUCCESS) {
			goto END;
		}

		/**
		 * Config data size is 31 bits Bit[30:0]
		 */
		GlitchData->GlitchDetTrim = GlitchData->GlitchDetTrim &
						(~XNVM_EFUSE_GLITCH_WR_LK_MASK);

		if(GlitchDetWrLkFlag[Idx] == TRUE) {
			GlitchData->GlitchDetWrLk = 1U;
		}
		else {
			GlitchData->GlitchDetWrLk = 0U;
		}

		if(GdRomMonEnFlag[Idx] == TRUE) {
			GlitchData->GdRomMonitorEn = 1U;
		}
		else {
			GlitchData->GdRomMonitorEn = 0U;
		}

		if(GdHaltBootEn10Flag[Idx] == TRUE) {
			GlitchData->GdHaltBootEn = 1U;
		}
		else {
			GlitchData->GdHaltBootEn = 0U;
		}

		Xil_DCacheFlushRange((UINTPTR)GlitchData,
			sizeof(XNvm_EfuseGlitchCfgBits));
		WriteEfuse->GlitchCfgAddr = (UINTPTR)GlitchData;
	}

	Status = XST_SUCCESS;

END:
	return Status;
}

/****************************************************************************/
/**
 * This function is used to initialize XNvm_EfuseAesKeys structure with user
 * provided data and assign it to global structure XNvm_EfuseDataAddr to program
 * below eFuses.
 * - AES key
 * - AES User keys
 *
 * typedef struct {
 *	u8 PrgmAesKey;
 *	u8 PrgmUserKey0;
 *	u8 PrgmUserKey1;
 *	u32 AesKey[XNVM_EFUSE_AES_KEY_LEN_IN_WORDS];
 *	u32 UserKey0[XNVM_EFUSE_AES_KEY_LEN_IN_WORDS];
 *	u32 UserKey1[XNVM_EFUSE_AES_KEY_LEN_IN_WORDS];
 * }XNvm_EfuseAesKeys;
 *
 * @param	WriteEfuse	Pointer to XNvm_EfuseDataAddr structure.
 *
 * @param 	AesKeys		Pointer to XNvm_EfuseAesKeys structure.
 *
 * @param 	Idx		SLR index.
 *
 * @return
 *		- XST_SUCCESS - If initialization of XNvm_EfuseAesKeys structure
 *				is successful
 *		- ErrorCode - On Failure
 *
 ******************************************************************************/
static int XilNvm_EfuseInitAesKeys(XNvm_EfuseDataAddr *WriteEfuse,
	XNvm_EfuseAesKeys *AesKeys, u32 Idx)
{
	int Status = XST_FAILURE;

	AesKeys->PrgmAesKey = WriteAesKeyFlag[Idx];
	AesKeys->PrgmUserKey0 = WriteUserKey0Flag[Idx];
	AesKeys->PrgmUserKey1 = WriteUserKey1Flag[Idx];

	if (AesKeys->PrgmAesKey == TRUE) {
		Status = XilNvm_PrepareAesKeyForWrite(AesKey[Idx],
					(u8 *)AesKeys->AesKey,
					XNVM_EFUSE_AES_KEY_LEN_IN_BITS);
		if (Status != XST_SUCCESS) {
			goto END;
		}
	}
	if (AesKeys->PrgmUserKey0 == TRUE) {
		Status = XilNvm_PrepareAesKeyForWrite(UserKey0[Idx],
					(u8 *)AesKeys->UserKey0,
					XNVM_EFUSE_AES_KEY_LEN_IN_BITS);
		if (Status != XST_SUCCESS) {
			goto END;
		}
	}
	if (AesKeys->PrgmUserKey1 == TRUE) {
		Status = XilNvm_PrepareAesKeyForWrite(UserKey1[Idx],
					(u8 *)AesKeys->UserKey1,
					XNVM_EFUSE_AES_KEY_LEN_IN_BITS);
		if (Status != XST_SUCCESS) {
			goto END;
		}
	}

	if (Status == XST_SUCCESS) {
		Xil_DCacheFlushRange((UINTPTR)AesKeys,
			sizeof(XNvm_EfuseAesKeys));
		WriteEfuse->AesKeyAddr = (UINTPTR)AesKeys;
	}

	Status = XST_SUCCESS;

END:
	return Status;
}

/****************************************************************************/
/**
 * This function is used to initialize the XNvm_EfusePpkHash structure with
 * user provided data and assign it to global structure XNvm_EfuseDataAddr to
 * program PPK0/PPK1/PPK2 hash eFuses
 *
 * typedef struct {
 *	u8 PrgmPpk0Hash;
 *	u8 PrgmPpk1Hash;
 *	u8 PrgmPpk2Hash;
 *	u32 Ppk0Hash[XNVM_EFUSE_PPK_HASH_LEN_IN_WORDS];
 *	u32 Ppk1Hash[XNVM_EFUSE_PPK_HASH_LEN_IN_WORDS];
 *	u32 Ppk2Hash[XNVM_EFUSE_PPK_HASH_LEN_IN_WORDS];
 * }XNvm_EfusePpkHash;
 *
 * @param	WriteEfuse	Pointer to XNvm_EfuseDataAddr structure.
 *
 * @param 	PpkHash		Pointer to XNvm_EfusePpkHash structure.
 *
 * @param 	Idx		SLR index.
 *
 * @return
 *		- XST_SUCCESS - If initialization of XNvm_EfusePpkHash structure
 *				is successful
 *		- ErrorCode - On Failure
 *
 ******************************************************************************/
static int XilNvm_EfuseInitPpkHash(XNvm_EfuseDataAddr *WriteEfuse,
	XNvm_EfusePpkHash *PpkHash, u32 Idx)
{
	int Status = XST_FAILURE;

	PpkHash->PrgmPpk0Hash = Ppk0HashFlag[Idx];
	PpkHash->PrgmPpk1Hash = Ppk1HashFlag[Idx];
	PpkHash->PrgmPpk2Hash = Ppk2HashFlag[Idx];

	if (PpkHash->PrgmPpk0Hash == TRUE) {
		Status = XilNvm_ValidateHash((char *)Ppk0Hash[Idx],
					XNVM_EFUSE_PPK_HASH_STRING_LEN);
		if(Status != XST_SUCCESS) {
			xil_printf("Ppk0Hash string validation failed\r\n");
			goto END;
		}
		Status = Xil_ConvertStringToHexBE((char *)Ppk0Hash[Idx],
						(u8 *)PpkHash->Ppk0Hash,
			XNVM_EFUSE_PPK_HASH_LEN_IN_BITS);
		if (Status != XST_SUCCESS) {
			goto END;
		}
	}

	if (PpkHash->PrgmPpk1Hash == TRUE) {
		Status = XilNvm_ValidateHash((char *)Ppk1Hash[Idx],
					XNVM_EFUSE_PPK_HASH_STRING_LEN);
		if(Status != XST_SUCCESS) {
			xil_printf("Ppk1Hash string validation failed\r\n");
			goto END;
		}
		Status = Xil_ConvertStringToHexBE((char *)Ppk1Hash[Idx],
					(u8 *)PpkHash->Ppk1Hash,
					XNVM_EFUSE_PPK_HASH_LEN_IN_BITS);
		if (Status != XST_SUCCESS) {
			goto END;
		}
	}
	if (PpkHash->PrgmPpk2Hash == TRUE) {
		Status = XilNvm_ValidateHash((char *)Ppk2Hash[Idx],
					XNVM_EFUSE_PPK_HASH_STRING_LEN);
		if(Status != XST_SUCCESS) {
			xil_printf("Ppk1Hash string validation failed\r\n");
			goto END;
		}
		Status = Xil_ConvertStringToHexBE((char *)Ppk2Hash[Idx],
					(u8 *)PpkHash->Ppk2Hash,
					XNVM_EFUSE_PPK_HASH_LEN_IN_BITS);
		if (Status != XST_SUCCESS) {
			goto END;
		}
	}

	if (Status == XST_SUCCESS) {
		Xil_DCacheFlushRange((UINTPTR)PpkHash,
			sizeof(XNvm_EfusePpkHash));
		WriteEfuse->PpkHashAddr = (UINTPTR)PpkHash;
	}

	Status = XST_SUCCESS;

END:
	return Status;
}

/****************************************************************************/
/**
 * This function is used to initialize the XNvm_EfuseDecOnly structure with
 * user provided data and assign the same to global structure XNvm_EfuseDataAddr
 * to program DEC_ONLY eFuses.
 *
 * typedef struct {
 *	u8 PrgmDecOnly;
 * }XNvm_EfuseDecOnly;
 *
 * @param	WriteEfuse	Pointer to XNvm_EfuseDataAddr structure.
 *
 * @param 	DecOnly		Pointer to XNvm_EfuseDecOnly structure.
 *
 * @param 	Idx		SLR index.
 *
 * @return
 *		- XST_SUCCESS - If initialization of XNvm_EfuseDecOnly structure
 *				is successful
 *		- ErrorCode - On failure.
 *
 ******************************************************************************/
static int XilNvm_EfuseInitDecOnly(XNvm_EfuseDataAddr *WriteEfuse,
	XNvm_EfuseDecOnly *DecOnly, u32 Idx)
{
	DecOnly->PrgmDecOnly = WriteDecOnlyFlag[Idx];

	if (DecOnly->PrgmDecOnly == TRUE) {
		Xil_DCacheFlushRange((UINTPTR)DecOnly,
			sizeof(XNvm_EfuseDecOnly));
		WriteEfuse->DecOnlyAddr = (UINTPTR)DecOnly;
	}

	return XST_SUCCESS;
}

/******************************************************************************/
/**
 * This function is used to initialize the XNvm_EfuseSecCtrlBits structure with
 * user provided data and assign the same to global structure XNvm_EfuseDataAddr
 * to program SECURITY_CONTROL eFuses.
 *
typedef struct {
	u8 AesDis;
	u8 JtagErrOutDis;
	u8 JtagDis;
	u8 HwTstBitsDis;
	u8 Ppk0WrLk;
	u8 Ppk1WrLk;
	u8 Ppk2WrLk;
	u8 AesCrcLk;
	u8 AesWrLk;
	u8 UserKey0CrcLk;
	u8 UserKey0WrLk;
	u8 UserKey1CrcLk;
	u8 UserKey1WrLk;
	u8 SecDbgDis;
	u8 SecLockDbgDis;
	u8 PmcScEn;
	u8 BootEnvWrLk;
	u8 RegInitDis;
} XNvm_EfuseSecCtrlBits;
 *
 * @param	WriteEfuse	Pointer to XNvm_EfuseDataAddr structure.
 *
 * @param 	SecCtrlBits	Pointer to XNvm_EfuseSecCtrlBits structure.
 *
 * @param 	Idx		SLR index.
 *
 * @return
 *		- XST_SUCCESS - If the initialization of XNvm_EfuseSecCtrlBits
 *				structure is successful
 *		- ErrCode - On failure
 *
 ******************************************************************************/
static int XilNvm_EfuseInitSecCtrl(XNvm_EfuseDataAddr *WriteEfuse,
	XNvm_EfuseSecCtrlBits *SecCtrlBits, u32 Idx)
{
	int Status = XST_FAILURE;

	SecCtrlBits->AesDis = AesDisFlag[Idx];
	SecCtrlBits->JtagErrOutDis = JtagErrorOutDisFlag[Idx];
	SecCtrlBits->JtagDis = JtagDisFlag[Idx];
	SecCtrlBits->SecDbgDis = AuthJtagDisFlag[Idx];
	SecCtrlBits->SecLockDbgDis = AuthJtagLockDisFlag[Idx];
	SecCtrlBits->BootEnvWrLk = BootEnvWrLkFlag[Idx];
	SecCtrlBits->RegInitDis = RegInitDisFlag[Idx];
	SecCtrlBits->Ppk0WrLk = Ppk0WrLockFlag[Idx];
	SecCtrlBits->Ppk1WrLk = Ppk1WrLockFlag[Idx];
	SecCtrlBits->Ppk2WrLk = Ppk2WrLockFlag[Idx];
	SecCtrlBits->AesCrcLk = AesCrcLkFlag[Idx];
	SecCtrlBits->AesWrLk = AesWrLkFlag[Idx];
	SecCtrlBits->UserKey0CrcLk = UserKey0CrcLkFlag[Idx];
	SecCtrlBits->UserKey0WrLk = UserKey0WrLkFlag[Idx];
	SecCtrlBits->UserKey1CrcLk = UserKey1CrcLkFlag[Idx];
	SecCtrlBits->UserKey1WrLk = UserKey1WrLkFlag[Idx];
	SecCtrlBits->HwTstBitsDis = HwTstBitsDisFlag[Idx];
	SecCtrlBits->PmcScEn = PmcScEnFlag[Idx];

	if ((SecCtrlBits->AesDis == TRUE) ||
		(SecCtrlBits->JtagErrOutDis == TRUE) ||
		(SecCtrlBits->JtagDis == TRUE) ||
		(SecCtrlBits->SecDbgDis == TRUE) ||
		(SecCtrlBits->SecLockDbgDis == TRUE) ||
		(SecCtrlBits->BootEnvWrLk == TRUE) ||
		(SecCtrlBits->RegInitDis == TRUE) ||
		(SecCtrlBits->Ppk0WrLk == TRUE) ||
		(SecCtrlBits->Ppk1WrLk == TRUE) ||
		(SecCtrlBits->Ppk2WrLk == TRUE) ||
		(SecCtrlBits->AesCrcLk == TRUE) ||
		(SecCtrlBits->AesWrLk == TRUE) ||
		(SecCtrlBits->UserKey0CrcLk == TRUE) ||
		(SecCtrlBits->UserKey0WrLk == TRUE) ||
		(SecCtrlBits->UserKey1CrcLk == TRUE) ||
		(SecCtrlBits->UserKey1WrLk == TRUE) ||
		(SecCtrlBits->HwTstBitsDis == TRUE) ||
		(SecCtrlBits->PmcScEn == TRUE)) {
		Xil_DCacheFlushRange((UINTPTR)SecCtrlBits,
			sizeof(XNvm_EfuseSecCtrlBits));
		WriteEfuse->SecCtrlAddr = (UINTPTR)SecCtrlBits;
	}

	Status = XST_SUCCESS;

	return Status;
}

/******************************************************************************/
/**
 * This function is used to initialize XNvm_EfuseMiscCtrlBits structure with
 * user provided data and assign the same to global structure XNvm_EfuseDataAddr
 * to program PPK INVLD eFuses.
 *
 * typedef struct {
 *	u8 GlitchDetHaltBootEn;
 *	u8 GlitchDetRomMonitorEn;
 *	u8 HaltBootError;
 *	u8 HaltBootEnv;
 *	u8 CryptoKatEn;
 *	u8 LbistEn;
 *	u8 SafetyMissionEn;
 *	u8 Ppk0Invalid;
 *	u8 Ppk1Invalid;
 *	u8 Ppk2Invalid;
 * }XNvm_EfuseMiscCtrlBits;
 *
 * @param	MiscCtrlBits	Pointer to XNvm_EfuseMiscCtrlBits structure
 *
 * @param	WriteEfuse	Pointer to XNvm_EfuseDataAddr structure
 *
 * @param 	Idx		SLR index.
 *
 * @return
 *		- XST_SUCCESS - If the initialization XNvm_EfuseMiscCtrlBits
 *				structure is successful
 *		- Error Code - On Failure.
 *
 ******************************************************************************/
static int XilNvm_EfuseInitMiscCtrl(XNvm_EfuseDataAddr *WriteEfuse,
	XNvm_EfuseMiscCtrlBits *MiscCtrlBits, u32 Idx)
{
	MiscCtrlBits->Ppk0Invalid = Ppk0InvldFlag[Idx];
	MiscCtrlBits->Ppk1Invalid = Ppk1InvldFlag[Idx];
	MiscCtrlBits->Ppk2Invalid = Ppk2InvldFlag[Idx];
	MiscCtrlBits->HaltBootError = GenErrHaltBootEn10Flag[Idx];
	MiscCtrlBits->HaltBootEnv = EnvErrHaltBootEn10Flag[Idx];
	MiscCtrlBits->CryptoKatEn = CryptoKatEnFlag[Idx];
	MiscCtrlBits->LbistEn = LbistEnFlag[Idx];
	MiscCtrlBits->SafetyMissionEn = SafetyMissionEnFlag[Idx];
#ifdef XNVM_EN_ADD_PPKS
	MiscCtrlBits->Ppk3Invalid = Ppk3InvldFlag[Idx];
	MiscCtrlBits->Ppk4Invalid = Ppk4InvldFlag[Idx];
	MiscCtrlBits->AdditionalPpkEn = AddPpkEnFlag[Idx];
#endif

	if ((MiscCtrlBits->Ppk0Invalid == TRUE) ||
		(MiscCtrlBits->Ppk1Invalid == TRUE) ||
		(MiscCtrlBits->Ppk2Invalid == TRUE) ||
		(MiscCtrlBits->HaltBootError == TRUE)||
		(MiscCtrlBits->HaltBootEnv == TRUE) ||
		(MiscCtrlBits->CryptoKatEn == TRUE) ||
		(MiscCtrlBits->LbistEn == TRUE) ||
#ifdef XNVM_EN_ADD_PPKS
		(MiscCtrlBits->Ppk3Invalid == TRUE )||
		(MiscCtrlBits->Ppk4Invalid == TRUE) ||
		(MiscCtrlBits->AdditionalPpkEn == TRUE) ||
#endif
		(MiscCtrlBits->SafetyMissionEn == TRUE)) {
		Xil_DCacheFlushRange((UINTPTR)MiscCtrlBits,
			sizeof(XNvm_EfuseMiscCtrlBits));
		WriteEfuse->MiscCtrlAddr = (UINTPTR)MiscCtrlBits;
	}

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
 * This function is used to initialize XNvm_EfuseSecMisc1Bits structure with
 * user provided data and assign the same to global structure XNvm_EfuseDataAddr
 * to program XNvm_EfuseSecMisc1Bits eFuses.
 *
 *typedef struct {
 *	u8 LpdMbistEn;
 *	u8 PmcMbistEn;
 *	u8 LpdNocScEn;
 *	u8 SysmonVoltMonEn;
 *	u8 SysmonTempMonEn;
 *}XNvm_EfuseSecMisc1Bits;

 * @param	WriteEfuse	Pointer to XNvm_EfuseDataAddr structure
 *
 * @param 	SecMisc1Bits	Pointer to XNvm_EfuseSecMisc1Bits structure
 *
 * @param 	Idx		SLR index.
 *
 * @return
 *		- XST_SUCCESS - If the initialization XNvm_EfuseSecMisc1Bits
 *				structure is successful
 *		- Error Code - On Failure.
 *
 ******************************************************************************/
static int XilNvm_EfuseInitSecMisc1Ctrl(XNvm_EfuseDataAddr *WriteEfuse,
	XNvm_EfuseSecMisc1Bits *SecMisc1Bits, u32 Idx)
{
	SecMisc1Bits->LpdMbistEn = LpdMbistEnFlag[Idx];
	SecMisc1Bits->PmcMbistEn = PmcMbistEnFlag[Idx];
	SecMisc1Bits->LpdNocScEn = LpdNocScEnFlag[Idx];
	SecMisc1Bits->SysmonVoltMonEn = SysmonVoltMonEnFlag[Idx];
	SecMisc1Bits->SysmonTempMonEn = SysmonTempMonEnFlag[Idx];

	if ((SecMisc1Bits->LpdMbistEn == TRUE) ||
		(SecMisc1Bits->PmcMbistEn == TRUE) ||
		(SecMisc1Bits->LpdNocScEn == TRUE) ||
		(SecMisc1Bits->SysmonVoltMonEn == TRUE)||
		(SecMisc1Bits->SysmonTempMonEn == TRUE)) {
		Xil_DCacheFlushRange((UINTPTR)SecMisc1Bits,
			sizeof(XNvm_EfuseSecMisc1Bits));
		WriteEfuse->Misc1CtrlAddr = (UINTPTR)SecMisc1Bits;
	}

	return XST_SUCCESS;
}

/******************************************************************************/
/**
 * This function is used to initialize XNvm_EfuseBootEnvCtrlBits structure with
 * user provided data and assign the same to global structure XNvm_EfuseDataAddr
 * to program XNvm_EfuseBootEnvCtrlBits eFuses.
 *
 * typedef struct {
 *	u8 PrgmSysmonTempHot;
 *	u8 PrgmSysmonVoltPmc;
 *	u8 PrgmSysmonVoltPslp;
 * 	u8 PrgmSysmonVoltSoc;
 *	u8 PrgmSysmonTempCold;
 *	u8 SysmonTempEn;
 *	u8 SysmonVoltEn;
 * 	u8 SysmonTempHot;
 *	u8 SysmonVoltPmc;
 *	u8 SysmonVoltPslp;
 *	u8 SysmonVoltSoc;
 *	u8 SysmonTempCold;
 * }XNvm_EfuseBootEnvCtrlBits;
 *
 * @param	WriteEfuse	Pointer to XNvm_EfuseDataAddr structure
 *
 * @param 	BootEnvCtrl	Pointer to XNvm_EfuseBootEnvCtrlBits structure
 *
 * @param 	Idx		SLR index.
 *
 * @return
 *		- XST_SUCCESS - If the initialization XNvm_EfuseBootEnvCtrlBits
 *				structure is successful
 *		- Error Code - On Failure.
 *
 ******************************************************************************/
static int XilNvm_EfuseInitBootEnvCtrl(XNvm_EfuseDataAddr *WriteEfuse,
	XNvm_EfuseBootEnvCtrlBits *BootEnvCtrl, u32 Idx)
{
	int Status = XST_FAILURE;

	BootEnvCtrl->SysmonTempEn = SysmonTempEnFlag[Idx];
	BootEnvCtrl->SysmonVoltEn = SysmonVoltEnFlag[Idx];
	BootEnvCtrl->SysmonVoltSoc = SysmonVoltSocFlag[Idx];
	BootEnvCtrl->PrgmSysmonTempHot = SysmonTempHotFlag[Idx];
	BootEnvCtrl->PrgmSysmonVoltPmc = SysmonVoltPmcFlag[Idx];
	BootEnvCtrl->PrgmSysmonVoltPslp = SysmonVoltPslpFlag[Idx];
	BootEnvCtrl->PrgmSysmonTempCold = SysmonTempColdFlag[Idx];

	if (BootEnvCtrl->PrgmSysmonTempHot == TRUE) {
		if (SysmonTempHotVal[Idx] >
			XNVM_EFUSE_TEMP_VOLT_LIMIT_MAX) {
			xil_printf("Invalid i/p for"
				" XNVM_EFUSE_SYSMON_TEMP_HOT_FUSES\r\n");
			goto END;
		}
		BootEnvCtrl->SysmonTempHot = SysmonTempHotVal[Idx];
	}
	if (BootEnvCtrl->PrgmSysmonVoltPmc == TRUE) {
		if (SysmonVoltPmcVal[Idx] >
			XNVM_EFUSE_TEMP_VOLT_LIMIT_MAX) {
			xil_printf("Invalid i/p for"
				" XNVM_EFUSE_SYSMON_VOLT_PMC_FUSES\r\n");
			goto END;
		}
		BootEnvCtrl->SysmonVoltPmc = SysmonVoltPmcVal[Idx];
	}
	if (BootEnvCtrl->PrgmSysmonVoltPslp == TRUE) {
		if (SysmonVoltPslpVal[Idx] >
			XNVM_EFUSE_TEMP_VOLT_LIMIT_MAX) {
			xil_printf("Invalid i/p for"
					 "XNVM_EFUSE_SYSMON_VOLT_PSLP_FUSE\r\n");
			goto END;
		}
		BootEnvCtrl->SysmonVoltPslp = SysmonVoltPslpVal[Idx];
	}
	if (BootEnvCtrl->PrgmSysmonTempCold == TRUE) {
		if (SysmonTempColdVal[Idx] >
			XNVM_EFUSE_TEMP_VOLT_LIMIT_MAX) {
			xil_printf("Invalid i/p for"
					" XNVM_EFUSE_SYSMON_TEMP_COLD_FUSES\r\n");
			goto END;
		}
		BootEnvCtrl->SysmonTempCold = SysmonTempColdVal[Idx];
	}

	if ((BootEnvCtrl->SysmonTempEn == TRUE) ||
		(BootEnvCtrl->SysmonVoltEn == TRUE) ||
		(BootEnvCtrl->SysmonVoltSoc == TRUE) ||
		(BootEnvCtrl->PrgmSysmonTempHot == TRUE) ||
		(BootEnvCtrl->PrgmSysmonVoltPmc == TRUE) ||
		(BootEnvCtrl->PrgmSysmonVoltPslp == TRUE) ||
		(BootEnvCtrl->PrgmSysmonTempCold == TRUE)) {
		Xil_DCacheFlushRange((UINTPTR)BootEnvCtrl,
			sizeof(XNvm_EfuseBootEnvCtrlBits));
		WriteEfuse->BootEnvCtrlAddr = (UINTPTR)BootEnvCtrl;
	}

	Status = XST_SUCCESS;

END:
	return Status;
}

/****************************************************************************/
/**
 * This function is used to initialize XNvm_EfuseRevokeIds structure with user
 * provided data and assign it to global structure  XNvm_EfuseDataAddr to
 * program revocation ID eFuses
 *
 * typedef struct {
 *	u8 PrgmRevokeId;
 *	u32 RevokeId[XNVM_NUM_OF_REVOKE_ID_FUSES];
 * }XNvm_EfuseRevokeIds;
 *
 * @param	WriteEfuse      Pointer to XNvm_EfuseDataAddr structure
 *
 * @param 	RevokeIds	Pointer to XNvm_EfuseRevokeIds structure
 *
 * @param 	Idx		SLR index.
 *
 * @return
 *		- XST_SUCCESS - If the initialization of XNvm_EfuseRevokeIds
 *				structure is successful
 *		- Error Code - On Failure.
 *
 ******************************************************************************/
static int XilNvm_EfuseInitRevocationIds(XNvm_EfuseDataAddr *WriteEfuse,
	XNvm_EfuseRevokeIds *RevokeIds, u32 Idx)
{
	int Status = XST_FAILURE;

	if ((WriteRevokeId0Flag[Idx] |
	     WriteRevokeId1Flag[Idx] |
	     WriteRevokeId2Flag[Idx] |
	     WriteRevokeId3Flag[Idx] |
	     WriteRevokeId4Flag[Idx] |
	     WriteRevokeId5Flag[Idx] |
	     WriteRevokeId6Flag[Idx] |
	     WriteRevokeId7Flag[Idx]) != 0U) {
		RevokeIds->PrgmRevokeId = TRUE;
	}

	if (RevokeIds->PrgmRevokeId == TRUE) {
		Status = XilNvm_PrepareRevokeIdsForWrite(
			RevokeId0[Idx],
			&RevokeIds->RevokeId[XNVM_EFUSE_REVOCATION_ID_0],
			XNVM_EFUSE_ROW_STRING_LEN);
		if (Status != XST_SUCCESS) {
			goto END;
		}
		Status = XilNvm_PrepareRevokeIdsForWrite(
			RevokeId1[Idx],
			&RevokeIds->RevokeId[XNVM_EFUSE_REVOCATION_ID_1],
			XNVM_EFUSE_ROW_STRING_LEN);
		if (Status != XST_SUCCESS) {
			goto END;
		}
		Status = XilNvm_PrepareRevokeIdsForWrite(
			RevokeId2[Idx],
			&RevokeIds->RevokeId[XNVM_EFUSE_REVOCATION_ID_2],
			XNVM_EFUSE_ROW_STRING_LEN);
		if (Status != XST_SUCCESS) {
			goto END;
		}
		Status = XilNvm_PrepareRevokeIdsForWrite(
			RevokeId3[Idx],
			&RevokeIds->RevokeId[XNVM_EFUSE_REVOCATION_ID_3],
			XNVM_EFUSE_ROW_STRING_LEN);
		if (Status != XST_SUCCESS) {
			goto END;
		}
		Status = XilNvm_PrepareRevokeIdsForWrite(
			RevokeId4[Idx],
			&RevokeIds->RevokeId[XNVM_EFUSE_REVOCATION_ID_4],
			XNVM_EFUSE_ROW_STRING_LEN);
		if (Status != XST_SUCCESS) {
			goto END;
		}
		Status = XilNvm_PrepareRevokeIdsForWrite(
			RevokeId5[Idx],
			&RevokeIds->RevokeId[XNVM_EFUSE_REVOCATION_ID_5],
			XNVM_EFUSE_ROW_STRING_LEN);
		if (Status != XST_SUCCESS) {
			goto END;
		}
		Status = XilNvm_PrepareRevokeIdsForWrite(
			RevokeId6[Idx],
			&RevokeIds->RevokeId[XNVM_EFUSE_REVOCATION_ID_6],
			XNVM_EFUSE_ROW_STRING_LEN);
		if (Status != XST_SUCCESS) {
			goto END;
		}
		Status = XilNvm_PrepareRevokeIdsForWrite(
			RevokeId7[Idx],
			&RevokeIds->RevokeId[XNVM_EFUSE_REVOCATION_ID_7],
			XNVM_EFUSE_ROW_STRING_LEN);
		if (Status != XST_SUCCESS) {
			goto END;
		}
	}

	if (Status == XST_SUCCESS) {
		Xil_DCacheFlushRange((UINTPTR)RevokeIds,
			sizeof(XNvm_EfuseRevokeIds));
		WriteEfuse->RevokeIdAddr = (UINTPTR)RevokeIds;
	}

	Status = XST_SUCCESS;
END:
	return Status;
}

/******************************************************************************/
/**
 * This function is used to initialize XNvm_EfuseOffChipIds structure with user
 * provided data and assign it to global structure XNvm_EfuseDataAddr to program
 * OffChip_Revoke ID eFuses
 *
 * typedef struct {
 *	u8 PrgmOffchipId;
 *	u32 OffChipId[XNVM_NUM_OF_OFFCHIP_ID_FUSES];
 * }XNvm_EfuseOffChipIds;
 *
 * @param	WriteEfuse      Pointer to XNvm_EfuseDataAddr structure
 *
 * @param 	OffChipIds	Pointer to XNvm_EfuseOffChipIds structure
 *
 * @param 	Idx		SLR index.
 *
 * @return
 *		- XST_SUCCESS - If the initialization of XNvm_EfuseOffChipIds
 *				structure is successful
 *		- Error Code - On Failure.
 *
 ******************************************************************************/
static int XilNvm_EfuseInitOffChipRevokeIds(XNvm_EfuseDataAddr *WriteEfuse,
	XNvm_EfuseOffChipIds *OffChipIds, u32 Idx)
{
	int Status = XST_FAILURE;

	if ((WriteOffChipRevokeId0Flag[Idx] |
	     WriteOffChipRevokeId1Flag[Idx] |
	     WriteOffChipRevokeId2Flag[Idx] |
	     WriteOffChipRevokeId3Flag[Idx] |
	     WriteOffChipRevokeId4Flag[Idx] |
	     WriteOffChipRevokeId5Flag[Idx] |
	     WriteOffChipRevokeId6Flag[Idx] |
	     WriteOffChipRevokeId7Flag[Idx]) != 0U) {

		OffChipIds->PrgmOffchipId = TRUE;
	}

	if (OffChipIds->PrgmOffchipId == TRUE) {
		Status = XilNvm_PrepareRevokeIdsForWrite(
			OffChipRevokeId0[Idx],
			&OffChipIds->OffChipId[XNVM_EFUSE_OFFCHIP_REVOKE_ID_0],
			XNVM_EFUSE_ROW_STRING_LEN);
		if (Status != XST_SUCCESS) {
			goto END;
		}
		Status = XilNvm_PrepareRevokeIdsForWrite(
			OffChipRevokeId1[Idx],
			&OffChipIds->OffChipId[XNVM_EFUSE_OFFCHIP_REVOKE_ID_1],
			XNVM_EFUSE_ROW_STRING_LEN);
		if (Status != XST_SUCCESS) {
			goto END;
		}
		Status = XilNvm_PrepareRevokeIdsForWrite(
			OffChipRevokeId2[Idx],
			&OffChipIds->OffChipId[XNVM_EFUSE_OFFCHIP_REVOKE_ID_2],
			XNVM_EFUSE_ROW_STRING_LEN);
		if (Status != XST_SUCCESS) {
			goto END;
		}
		Status = XilNvm_PrepareRevokeIdsForWrite(
			OffChipRevokeId3[Idx],
			&OffChipIds->OffChipId[XNVM_EFUSE_OFFCHIP_REVOKE_ID_3],
			XNVM_EFUSE_ROW_STRING_LEN);
		if (Status != XST_SUCCESS) {
			goto END;
		}
		Status = XilNvm_PrepareRevokeIdsForWrite(
			OffChipRevokeId4[Idx],
			&OffChipIds->OffChipId[XNVM_EFUSE_OFFCHIP_REVOKE_ID_4],
			XNVM_EFUSE_ROW_STRING_LEN);
		if (Status != XST_SUCCESS) {
			goto END;
		}
		Status = XilNvm_PrepareRevokeIdsForWrite(
			OffChipRevokeId5[Idx],
			&OffChipIds->OffChipId[XNVM_EFUSE_OFFCHIP_REVOKE_ID_5],
			XNVM_EFUSE_ROW_STRING_LEN);
		if (Status != XST_SUCCESS) {
			goto END;
		}
		Status = XilNvm_PrepareRevokeIdsForWrite(
			OffChipRevokeId6[Idx],
			&OffChipIds->OffChipId[XNVM_EFUSE_OFFCHIP_REVOKE_ID_6],
			XNVM_EFUSE_ROW_STRING_LEN);
		if (Status != XST_SUCCESS) {
			goto END;
		}
		Status = XilNvm_PrepareRevokeIdsForWrite(
			OffChipRevokeId7[Idx],
			&OffChipIds->OffChipId[XNVM_EFUSE_OFFCHIP_REVOKE_ID_7],
			XNVM_EFUSE_ROW_STRING_LEN);
		if (Status != XST_SUCCESS) {
			goto END;
		}
	}

	if (Status == XST_SUCCESS) {
		Xil_DCacheFlushRange((UINTPTR)OffChipIds,
			sizeof(XNvm_EfuseOffChipIds));
		WriteEfuse->OffChipIdAddr = (UINTPTR)OffChipIds;
	}

	Status = XST_SUCCESS;
END:
	return Status;
}
/****************************************************************************/
/**
 * This function is used to initialize XNvm_EfuseIvs structure with user
 * provided data and assign the same to global structure XNvm_EfuseDataAddr to
 * program IV eFuses.
 *
 * typedef struct {
 *	u8 PrgmMetaHeaderIv;
 *	u8 PrgmBlkObfusIv;
 *	u8 PrgmPlmIv;
 *	u8 PrgmDataPartitionIv;
 *	u32 MetaHeaderIv[XNVM_EFUSE_IV_LEN_IN_WORDS];
 *	u32 BlkObfusIv[XNVM_EFUSE_IV_LEN_IN_WORDS];
 *	u32 PlmIv[XNVM_EFUSE_IV_LEN_IN_WORDS];
 *	u32 DataPartitionIv[XNVM_EFUSE_IV_LEN_IN_WORDS];
 * }XNvm_EfuseIvs;
 *
 * @param	WriteEfuse      Pointer to XNvm_EfuseDataAddr structure
 *
 * @param 	Ivs		Pointer to XNvm_EfuseIvs structure
 *
 * @param 	Idx		SLR index.
 *
 * @return
 *		- XST_SUCCESS - If the initialization of XNvm_EfuseIvs structure
 *				is successful
 *		- Error Code - On Failure.
 *
 ******************************************************************************/
static int XilNvm_EfuseInitIVs(XNvm_EfuseDataAddr *WriteEfuse,
	XNvm_EfuseIvs *Ivs, u32 Idx)
{
	int Status = XST_FAILURE;

	Ivs->PrgmMetaHeaderIv = WriteMetaHeaderIvFlag[Idx];
	Ivs->PrgmBlkObfusIv = WriteBlackObfIvFlag[Idx];
	Ivs->PrgmPlmIv = WritePlmIvFlag[Idx];
	Ivs->PrgmDataPartitionIv = WriteDataPartIvFlag[Idx];

	if (Ivs->PrgmMetaHeaderIv == TRUE) {
		Status = XilNvm_PrepareIvForWrite(MetaHeaderIv[Idx],
						(u8 *)Ivs->MetaHeaderIv,
						XNVM_EFUSE_IV_LEN_IN_BITS);
		if (Status != XST_SUCCESS) {
			goto END;
		}
	}
	if (Ivs->PrgmBlkObfusIv == TRUE) {
		Status = XilNvm_PrepareIvForWrite(BlackObfusIv[Idx],
						(u8 *)Ivs->BlkObfusIv,
						XNVM_EFUSE_IV_LEN_IN_BITS);
		if (Status != XST_SUCCESS) {
			goto END;
		}
	}
	if (Ivs->PrgmPlmIv == TRUE) {
		Status = XilNvm_PrepareIvForWrite(PlmIv[Idx],
						(u8 *)Ivs->PlmIv,
						XNVM_EFUSE_IV_LEN_IN_BITS);
		if (Status != XST_SUCCESS) {
			goto END;
		}
	}
	if (Ivs->PrgmDataPartitionIv == TRUE) {
		Status = XilNvm_PrepareIvForWrite(DataPartitionIv[Idx],
						(u8 *)Ivs->DataPartitionIv,
						XNVM_EFUSE_IV_LEN_IN_BITS);
		if (Status != XST_SUCCESS) {
			goto END;
		}
	}
	if (Status == XST_SUCCESS) {
		Xil_DCacheFlushRange((UINTPTR)Ivs, sizeof(XNvm_EfuseIvs));
		WriteEfuse->IvAddr = (UINTPTR)Ivs;
	}

	Status = XST_SUCCESS;

END:
	return Status;
}

/******************************************************************************/
/**
 * This function is to validate the input User Fuse string
 *
 * @param   UserFuseStr - Pointer to User Fuse String
 *
 * @return
 *	- XST_SUCCESS - On valid input UserFuse string
 *	- XST_INVALID_PARAM - On invalid length of the input string
 ******************************************************************************/
static int XilNvm_ValidateUserFuseStr(const char *UserFuseStr)
{
	int Status = XST_INVALID_PARAM;

	if(UserFuseStr != NULL) {
		if (strlen(UserFuseStr) % XNVM_EFUSE_ROW_STRING_LEN == 0x00U) {
			Status = XST_SUCCESS;
		}
	}

	return Status;
}

/****************************************************************************/
/**
 * This function is used to initialize XNvm_UserEfuseData structure with user
 * provided data and assign the same to global structure XNvm_EfuseDataAddr to
 * program User Fuses.
 *
 * typedef struct {
 *	u32 StartUserFuseNum;
 *	u32 NumOfUserFuses;
 *	u64 UserFuseDataAddr;
 * }XNvm_EfuseUserDataAddr;
 *
 * @param	WriteEfuse      Pointer to XNvm_EfuseDataAddr structure
 *
 * @param 	Data		Pointer to XNvm_UserEfuseData structure
 *
 * @param 	Idx		SLR index.
 *
 * @return
 *		- XST_SUCCESS - If the initialization of XNvm_UserEfuseData
 *				structure is successful
 *		- Error Code - On Failure.
 *
 ******************************************************************************/
static int XilNvm_EfuseInitUserFuses(XNvm_EfuseDataAddr *WriteEfuse,
	XNvm_EfuseUserDataAddr *Data, u32 Idx)
{
	int Status = XST_FAILURE;

	if (WriteUserFuseFlag[Idx] == TRUE) {
		Status = XilNvm_ValidateUserFuseStr(UserFuses[Idx]);
		if (Status != XST_SUCCESS) {
			xil_printf("UserFuse string validation failed\r\n");
			goto END;
		}
		Status = Xil_ConvertStringToHex(
				UserFuses[Idx],
				(u32 *)(UINTPTR)Data->UserFuseDataAddr,
				(NumOfUserFusesVal[Idx] *
				XNVM_EFUSE_ROW_STRING_LEN));
		if (Status != XST_SUCCESS) {
			goto END;
		}

		Data->StartUserFuseNum = XNVM_EFUSE_PRGM_USER_FUSE_NUM;
		Data->NumOfUserFuses = NumOfUserFusesVal[Idx];

		Xil_DCacheFlushRange((UINTPTR)Data,
			sizeof(XNvm_EfuseUserDataAddr));
		WriteEfuse->UserFuseAddr = (UINTPTR)Data;
	}
	Status = XST_SUCCESS;

END:
	return  Status;
}

/****************************************************************************/
/**
 * This API reads secure and control bits from efuse cache and displays here.
 *
 * @return
 * 		- XST_SUCCESS - If read is successful
 * 		- ErrorCode - On failure
 *
 ******************************************************************************/
static int XilNvm_EfuseShowCtrlBits(void)
{
	int Status = XST_FAILURE;

	Status = XilNvm_EfuseShowSecCtrlBits();
	if (Status != XST_SUCCESS) {
		goto END;
	}
	Status = XilNvm_EfuseShowPufSecCtrlBits();
	if (Status != XST_SUCCESS) {
		goto END;
	}
	Status = XilNvm_EfuseShowMiscCtrlBits();
	if (Status != XST_SUCCESS) {
		goto END;
	}
	Status = XilNvm_EfuseShowSecMisc1Bits();
	if (Status != XST_SUCCESS) {
		goto END;
	}
	Status = XilNvm_EfuseShowBootEnvCtrlBits();
	if (Status != XST_SUCCESS) {
		goto END;
	}

END:
	return Status;
}

/****************************************************************************/
/**
 * This API reads secure and control bits from efuse cache and displays here.
 *
 * @return
 * 		- XST_SUCCESS - If read is successful
 * 		- ErrorCode - On failure
 *
 ******************************************************************************/
static int XilNvm_EfuseShowSecCtrlBits(void)
{
	int Status = XST_FAILURE;
	XNvm_EfuseSecCtrlBits *SecCtrlBits =
		(XNvm_EfuseSecCtrlBits *)(UINTPTR)&SharedMem[0U];

	Xil_DCacheInvalidateRange((UINTPTR)&SharedMem[0U],
			XNVM_CACHE_ALIGNED_LEN);

	Status = XNvm_EfuseReadSecCtrlBits(&NvmClientInstance, (UINTPTR)SecCtrlBits);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Xil_DCacheInvalidateRange((UINTPTR)&SharedMem[0U],
			XNVM_CACHE_ALIGNED_LEN);

	xil_printf("\r\nSecurity Control eFuses:\n\r");

	if (SecCtrlBits->AesDis == TRUE) {
		xil_printf("\r\nAES is disabled\n\r");
	}
	else {
		xil_printf("\r\nAES is not disabled\n\r");
	}

	if (SecCtrlBits->JtagErrOutDis == TRUE) {
		xil_printf("JTAG Error Out is disabled\n\r");
	}
	else {
		xil_printf("JTAG Error Out is not disabled\n\r");
	}
	if (SecCtrlBits->JtagDis == TRUE) {
		xil_printf("JTAG is disabled\n\r");
	}
	else {
		xil_printf("JTAG is not disabled\n\r");
	}
	if (SecCtrlBits->HwTstBitsDis == TRUE) {
		xil_printf("HW Testbit mode is disabled\n\r");
	}
	else {
		xil_printf("HW Testbit mode is enabled\n\r");
	}
	if (SecCtrlBits->Ppk0WrLk == TRUE) {
		xil_printf("Locks writing to PPK0 efuse\n\r");
	}
	else {
		xil_printf("Writing to PPK0 efuse is not locked\n\r");
	}
	if (SecCtrlBits->Ppk1WrLk == TRUE) {
		xil_printf("Locks writing to PPK1 efuse\n\r");
	}
	else {
		xil_printf("Writing to PPK1 efuse is not locked\n\r");
	}
	if (SecCtrlBits->Ppk2WrLk == TRUE) {
		xil_printf("Locks writing to PPK2 efuse\n\r");
	}
	else {
		xil_printf("Writing to PPK2 efuse is not locked\n\r");
	}
	if (SecCtrlBits->AesCrcLk != FALSE) {
		xil_printf("CRC check on AES key is disabled\n\r");
	}
	else {
		xil_printf("CRC check on AES key is not disabled\n\r");
	}
	if (SecCtrlBits->AesWrLk == TRUE) {
		xil_printf("Programming AES key is disabled\n\r");
	}
	else {
		xil_printf("Programming AES key is not disabled\n\r");
	}
	if (SecCtrlBits->UserKey0CrcLk == TRUE) {
		xil_printf("CRC check on User key 0 is disabled\n\r");
	}
	else {
		xil_printf("CRC check on User key 0 is enabled\n\r");
	}
	if (SecCtrlBits->UserKey0WrLk == TRUE) {
		xil_printf("Programming User key 0 is disabled\n\r");
	}
	else {
		xil_printf("Programming User key 0 is not disabled\n\r");
	}
	if (SecCtrlBits->UserKey1CrcLk == TRUE) {
		xil_printf("CRC check on User key 1 is disabled\n\r");
	}
	else {
		xil_printf("CRC check on User key 1 is enabled\n\r");
	}
	if (SecCtrlBits->UserKey1WrLk == TRUE) {
		xil_printf("Programming User key 1 is disabled\n\r");
	}
	else {
		xil_printf("Programming User key 1 is not disabled\n\r");
	}
	if (SecCtrlBits->SecDbgDis != FALSE) {
		xil_printf("Secure Debug feature is disabled\n\r");
	}
	else {
		xil_printf("Secure Debug feature is enabled\n\r");
	}
	if (SecCtrlBits->SecLockDbgDis != FALSE) {
		xil_printf("Secure Debug feature in JTAG is disabled\n\r");
	}
	else {
		xil_printf("Secure Debug feature in JTAG is enabled\n\r");
	}
	if (SecCtrlBits->PmcScEn == TRUE) {
		xil_printf("PMC Scan Clear is enabled\n\r");
	}
	else {
		xil_printf("PMC Scan Clear is disabled\n\r");
	}
	if (SecCtrlBits->BootEnvWrLk == TRUE) {
		xil_printf("Update to BOOT_ENV_CTRL row is disabled\n\r");
	}
	else {
		xil_printf("Update to BOOT_ENV_CTRL row is enabled\n\r");
	}
	if(SecCtrlBits->RegInitDis != FALSE) {
		xil_printf("Register Init is disabled\n\r");
	}
	else {
		xil_printf("Register Init is enabled\n\r");
	}
	Status = XST_SUCCESS;

END:
	return Status;
}

/****************************************************************************/
/**
 * This API reads Puf control bits from efuse cache and displays here.
 *
 * @return
 * 		- XST_SUCCESS - If read is successful
 * 		- ErrorCode - On failure
 *
 ******************************************************************************/
static int XilNvm_EfuseShowPufSecCtrlBits(void)
{
	int Status = XST_FAILURE;
	XNvm_EfusePufSecCtrlBits *PufSecCtrlBits =
		(XNvm_EfusePufSecCtrlBits *)(UINTPTR)&SharedMem[0U];

	Xil_DCacheInvalidateRange((UINTPTR)&SharedMem[0U],
			XNVM_CACHE_ALIGNED_LEN);

	Status = XNvm_EfuseReadPufSecCtrlBits(&NvmClientInstance, (UINTPTR)PufSecCtrlBits);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Xil_DCacheInvalidateRange((UINTPTR)&SharedMem[0U],
			XNVM_CACHE_ALIGNED_LEN);

	xil_printf("\r\nPuf Control eFuses:\n\r");

	if (PufSecCtrlBits->PufSynLk == TRUE) {
		xil_printf("Programming Puf Syndrome data is disabled\n\r");
	}
	else {
		xil_printf("Programming Puf Syndrome data is enabled\n\r");
	}
	if(PufSecCtrlBits->PufDis == TRUE) {
		xil_printf("Puf is disabled\n\r");
	}
	else {
		xil_printf("Puf is enabled\n\r");
	}
	if (PufSecCtrlBits->PufRegenDis == TRUE) {
		xil_printf("Puf on demand regeneration is disabled\n\r");
	}
	else {
		xil_printf("Puf on demand regeneration is enabled\n\r");
	}
	if (PufSecCtrlBits->PufHdInvalid == TRUE) {
		xil_printf("Puf Helper data stored in efuse is not valid\n\r");
	}
	else {
		xil_printf("Puf Helper data stored in efuse is valid\n\r");
	}
	if (PufSecCtrlBits->PufTest2Dis == TRUE) {
		xil_printf("Puf test 2 is disabled\n\r");
	}
	else {
		xil_printf("Puf test 2 is enabled\n\r");
	}
	Status = XST_SUCCESS;
END:
	return Status;
}

/****************************************************************************/
/**
 * This API reads Misc control bits from efuse cache and displays here.
 *
 * @return
 * 		- XST_SUCCESS - If read is successful
 * 		- ErrorCode - On failure
 *
 ******************************************************************************/
static int XilNvm_EfuseShowMiscCtrlBits(void)
{
	int Status = XST_FAILURE;
	XNvm_EfuseMiscCtrlBits *MiscCtrlBits =
		(XNvm_EfuseMiscCtrlBits *)(UINTPTR)&SharedMem[0U];

	Xil_DCacheInvalidateRange((UINTPTR)&SharedMem[0U],
			XNVM_CACHE_ALIGNED_LEN);

	Status = XNvm_EfuseReadMiscCtrlBits(&NvmClientInstance, (UINTPTR)MiscCtrlBits);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Xil_DCacheInvalidateRange((UINTPTR)&SharedMem[0U],
			XNVM_CACHE_ALIGNED_LEN);

	xil_printf("\r\nMisc Control eFuses:\n\r");

	if (MiscCtrlBits->GlitchDetHaltBootEn != FALSE) {
		xil_printf("GdHaltBootEn efuse is programmed\r\n");
	}
	else {
		xil_printf("GdHaltBootEn efuse is not programmed\n\r");
	}
	if (MiscCtrlBits->GlitchDetRomMonitorEn == TRUE) {
		xil_printf("GdRomMonitorEn efuse is programmed\n\r");
	}
	else {
		xil_printf("GdRomMonitorEn efuse is not programmed\n\r");
	}
	if (MiscCtrlBits->HaltBootError != FALSE) {
		xil_printf("HaltBootError efuse is programmed\n\r");
	}
	else {
		xil_printf("HaltBootError efuse is not programmed\r\n");
	}
	if (MiscCtrlBits->HaltBootEnv != FALSE) {
		xil_printf("HaltBootEnv efuse is programmed\n\r");
	}
	else {
		xil_printf("HaltBootEnv efuse is not programmed\n\r");
	}
	if (MiscCtrlBits->CryptoKatEn == TRUE) {
		xil_printf("CryptoKatEn efuse is programmed\n\r");
	}
	else {
		xil_printf("CryptoKatEn efuse is not programmed\n\r");
	}
	if (MiscCtrlBits->LbistEn == TRUE) {
		xil_printf("LbistEn efuse is programmed\n\r");
	}
	else {
		xil_printf("LbistEn efuse is not programmed\n\r");
	}
	if (MiscCtrlBits->SafetyMissionEn == TRUE) {
		xil_printf("SafetyMissionEn efuse is programmed\n\r");
	}
	else {
		xil_printf("SafetyMissionEn efuse is not programmed\n\r");
	}
	if(MiscCtrlBits->Ppk0Invalid != FALSE) {
		xil_printf("Ppk0 hash stored in efuse is invalid\n\r");
	}
	else {
		xil_printf("Ppk0 hash stored in efuse is valid\n\r");
	}
	if(MiscCtrlBits->Ppk1Invalid != FALSE) {
		xil_printf("Ppk1 hash stored in efuse is invalid\n\r");
	}
	else {
		xil_printf("Ppk1 hash stored in efuse is valid\n\r");
	}
	if(MiscCtrlBits->Ppk2Invalid != FALSE) {
		xil_printf("Ppk2 hash stored in efuse is invalid\n\r");
	}
	else {
		xil_printf("Ppk2 hash stored in efuse is valid\n\r");
	}
#ifdef XNVM_EN_ADD_PPKS
	if(MiscCtrlBits->Ppk3Invalid != FALSE) {
		xil_printf("Ppk3 hash stored in efuse is not valid\n\r");
	}
	else {
		xil_printf("Ppk3 hash stored in efuse is valid\n\r");
	}
	if(MiscCtrlBits->Ppk4Invalid != FALSE) {
		xil_printf("Ppk4 hash stored in efuse is not valid\n\r");
	}
	else {
		xil_printf("Ppk4 hash stored in efuse is valid\n\r");
	}
#endif

	Status = XST_SUCCESS;

END:
	return Status;
}

/****************************************************************************/
/**
 * This API reads Security Misc1 control bits from efuse cache and displays.
 *
 * @return
 * 		- XST_SUCCESS - If read is successful
 * 		- ErrorCode - On failure
 *
 ******************************************************************************/
static int XilNvm_EfuseShowSecMisc1Bits(void)
{
	int Status = XST_FAILURE;
	XNvm_EfuseSecMisc1Bits *SecMisc1Bits =
		(XNvm_EfuseSecMisc1Bits *)(UINTPTR)&SharedMem[0U];

	Xil_DCacheInvalidateRange((UINTPTR)&SharedMem[0U],
			XNVM_CACHE_ALIGNED_LEN);

	Status = XNvm_EfuseReadSecMisc1Bits(&NvmClientInstance, (UINTPTR)SecMisc1Bits);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Xil_DCacheInvalidateRange((UINTPTR)&SharedMem[0U],
			XNVM_CACHE_ALIGNED_LEN);

	xil_printf("\r\nSecurity Misc1 eFuses:\n\r");

	if (SecMisc1Bits->LpdMbistEn != FALSE) {
		xil_printf("LpdMbistEn efuse is programmed\n\r");
	}
	else {
		xil_printf("LpdMbistEn efuse is not programmed\n\r");
	}
	if (SecMisc1Bits->PmcMbistEn != FALSE) {
		xil_printf("PmcMbistEn efuse is programmed\n\r");
	}
	else {
		xil_printf("PmcMbistEn efuse is not programmed\n\r");
	}
	if (SecMisc1Bits->LpdNocScEn != FALSE) {
		xil_printf("LpdNocScEn efuse is programmed\n\r");
	}
	else {
		xil_printf("LpdNocScEn efuse is not programmed\n\r");
	}
	if (SecMisc1Bits->SysmonVoltMonEn != FALSE) {
		xil_printf("SysmonVoltMonEn efuse is programmed\n\r");
	}
	else {
		xil_printf("SysmonVoltMonEn efuse is not programmed\n\r");
	}
	if (SecMisc1Bits->SysmonTempMonEn != FALSE) {
		xil_printf("SysmonTempMonEn efuse is programmed\n\r");
	}
	else {
		xil_printf("SysmonTempMonEn efuse is not programmed\n\r");
	}
	Status = XST_SUCCESS;

END:
	return Status;
}

/****************************************************************************/
/**
 * This API reads Boot Environmental control bits from efuse cache and
 * displays.
 *
 * @return
 * 		- XST_SUCCESS - If read is successful
 * 		- ErrorCode - On failure
 *
 ******************************************************************************/
static int XilNvm_EfuseShowBootEnvCtrlBits(void)
{
	int Status = XST_FAILURE;
	XNvm_EfuseBootEnvCtrlBits *BootEnvCtrlBits =
		(XNvm_EfuseBootEnvCtrlBits *)(UINTPTR)&SharedMem[0U];

	Xil_DCacheInvalidateRange((UINTPTR)&SharedMem[0U],
			XNVM_CACHE_ALIGNED_LEN);

	Status = XNvm_EfuseReadBootEnvCtrlBits(&NvmClientInstance, (UINTPTR)BootEnvCtrlBits);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Xil_DCacheInvalidateRange((UINTPTR)&SharedMem[0U],
			XNVM_CACHE_ALIGNED_LEN);

	xil_printf("\r\nBoot Environmental Control eFuses:\n\r");

	if (BootEnvCtrlBits->SysmonTempEn == TRUE) {
		xil_printf("SysmonTempEn efuse is programmed\n\r");
	}
	else {
		xil_printf("SysmonTempEn efuse is not programmed\n\r");
	}
	if (BootEnvCtrlBits->SysmonVoltEn == TRUE) {
		xil_printf("SysmonVoltEn efuse is programmed\n\r");
	}
	else {
		xil_printf("SysmonVoltEn efuse is not programmed\n\r");
	}

	xil_printf("SysmonTempHot : %d\n\r", BootEnvCtrlBits->SysmonTempHot);
	xil_printf("SysmonVoltPmc : %d\n\r", BootEnvCtrlBits->SysmonVoltPmc);
	xil_printf("SysmonVoltPslp : %d\n\r", BootEnvCtrlBits->SysmonVoltPslp);
	xil_printf("SysmonVoltSoc : %d\n\r", BootEnvCtrlBits->SysmonVoltSoc);
	xil_printf("SysmonTempCold : %d\n\r", BootEnvCtrlBits->SysmonTempCold);

	Status = XST_SUCCESS;
END:
	return Status;
}

/****************************************************************************/
/**
 * This function is to validate and convert Aes key string to Hex
 *
 * @param	KeyStr	Pointer to Aes Key String
 *
 * @param	Dst	Destination where converted Aes key can be stored
 *
 * @param	Len 	Length of the Aes key in bits
 *
 * @return
 *		- XST_SUCCESS - If validation and conversion of key is success
 *		- Error Code - On Failure.
 *
 ******************************************************************************/
static int XilNvm_PrepareAesKeyForWrite(const char *KeyStr, u8 *Dst, u32 Len)
{
	int Status = XST_FAILURE;

	if ((KeyStr == NULL) ||
		(Dst == NULL) ||
		(Len != XNVM_EFUSE_AES_KEY_LEN_IN_BITS)) {
		goto END;
	}
	Status = XNvm_ValidateAesKey(KeyStr);
	if(Status != XST_SUCCESS) {
		goto END;
	}
	Status = Xil_ConvertStringToHexLE(KeyStr, Dst, Len);

END:
	return Status;
}

/****************************************************************************/
/**
 * This function is to validate and convert IV string to Hex
 *
 * @param	IvStr	Pointer to IV String
 *
 * @param 	Dst	Destination to store the converted IV in Hex
 *
 * @param	Len	Length of the IV in bits
 *
 * @return
 *		- XST_SUCCESS - If validation and conversion of IV success
 *		- Error Code - On Failure.
 *
 ******************************************************************************/
static int XilNvm_PrepareIvForWrite(const char *IvStr, u8 *Dst, u32 Len)
{
	int Status = XST_FAILURE;

	if ((IvStr == NULL) ||
		(Dst == NULL) ||
		(Len != XNVM_EFUSE_IV_LEN_IN_BITS)) {
		goto END;
	}

	Status = XilNvm_ValidateIvString(IvStr);
	if(Status != XST_SUCCESS) {
		xil_printf("IV string validation failed\r\n");
		goto END;
	}
	Status = Xil_ConvertStringToHexBE(IvStr, Dst, Len);

END:
	return Status;
}

/******************************************************************************/
/**
 * This function is validate the input string contains valid Revoke Id String
 *
 * @param	RevokeIdStr - Pointer to Revocation ID/OffChip_Revoke ID String
 *
 * @param 	Dst	Destination to store the converted Revocation ID/
 * 			OffChip ID in Hex
 *
 * @param	Len	Length of the Revocation ID/OffChip ID in bits
 *
 * @return
 *	- XST_SUCCESS - On valid input Revoke Id string
 *	- XST_INVALID_PARAM - On invalid length of the input string
 *
 ******************************************************************************/
static int XilNvm_PrepareRevokeIdsForWrite(const char *RevokeIdStr,
	u32 *Dst, u32 Len)
{
	int Status = XST_INVALID_PARAM;

	if(RevokeIdStr != NULL) {
		if (strnlen(RevokeIdStr, XNVM_EFUSE_ROW_STRING_LEN) ==
			XNVM_EFUSE_ROW_STRING_LEN) {
			Status = Xil_ConvertStringToHex(RevokeIdStr, Dst, Len);
		}
	}

	return Status;
}


/******************************************************************************/
/**
 * This function is used to validate the input string contains valid PPK hash
 *
 * @param	Hash - Pointer to PPK hash
 *
 * @param	Len  - Length of the input string
 *
 * @return
 *	- XST_SUCCESS	- On valid input Ppk Hash string
 *	- XST_INVALID_PARAM - On invalid length of the input string
 *	- XST_FAILURE	- On non hexadecimal character in string
 *
 ******************************************************************************/
static int XilNvm_ValidateHash(const char *Hash, u32 Len)
{
	int Status = XST_FAILURE;
	u32 StrLen = strnlen(Hash, XNVM_EFUSE_PPK_HASH_STRING_LEN);
	u32 Index;

	if ((NULL == Hash) || (Len == 0U)) {
		goto END;
	}

	if (StrLen != Len) {
		goto END;
	}

	for(Index = 0U; Index < StrLen; Index++) {
		if(Xil_IsValidHexChar(&Hash[Index]) != (u32)XST_SUCCESS) {
			goto END;
		}
	}
	Status = XST_SUCCESS;

END :
	return Status;
}

/******************************************************************************/
/**
 * Validate the input string contains valid IV String
 *
 * @param	IvStr - Pointer to Iv String
 *
 * @return
 *	XST_SUCCESS	- On valid input IV string
 *	XST_INVALID_PARAM - On invalid length of the input string
 *
 ******************************************************************************/
static int XilNvm_ValidateIvString(const char *IvStr)
{
	int Status = XST_FAILURE;

	if(NULL == IvStr) {
		goto END;
	}

	if (strnlen(IvStr, XNVM_IV_STRING_LEN) == XNVM_IV_STRING_LEN) {
		Status = XST_SUCCESS;
	}

END:
	return Status;
}

/******************************************************************************/
/**
 *
 * This function reverses the data array
 *
 * @param	OrgDataPtr	- Pointer to the original data
 * @param	SwapPtr	- Pointer to the reversed data
 * @param	Len	- Length of the data in bytes
 *
 ******************************************************************************/
static void XilNvm_FormatData(const u8 *OrgDataPtr, u8* SwapPtr, u32 Len)
{
	u32 Index = 0U;
	u32 ReverseIndex = (Len - 1U);
	for(Index = 0U; Index < Len; Index++)
	{
		SwapPtr[Index] = OrgDataPtr[ReverseIndex];
		ReverseIndex--;
	}
}

/******************************************************************************/
/**
 * @brief	Validate the input string contains valid AES key.
 *
 * @param   	Key - Pointer to AES key.
 *
 * @return	- XST_SUCCESS - On valid input AES key string.
 *		- XST_INVALID_PARAM - On invalid length of the input string.
 *		- XST_FAILURE	- On non hexadecimal character in string
 *
 *******************************************************************************/
static int XNvm_ValidateAesKey(const char *Key)
{
	int Status = XST_INVALID_PARAM;
	u32 Len;

	if(NULL == Key) {
		goto END;
	}

	Len = Xil_Strnlen(Key, XNVM_MAX_AES_KEY_LEN_IN_CHARS + 1U);

	if ((Len != XNVM_256_BITS_AES_KEY_LEN_IN_CHARS) &&
		(Len != XNVM_128_BITS_AES_KEY_LEN_IN_CHARS)) {
		goto END;
	}

	Status = (int)Xil_ValidateHexStr(Key);
END:
	return Status;
}
/******************************************************************************/
/**
 * @brief	Adds the SLR Index.
 *
 * @param  InstancePtr is a pointer to instance XNvm_ClientInstance
 *
 * @param   SlrIndex - Number for slrId
 *
 *@return	- XST_SUCCESS - On valid input SlrIndex.
 *		    - XST_FAILURE - On non valid input SlrIndex
 *
 *******************************************************************************/
static int XNvm_InputSlrIndex(XNvm_ClientInstance *InstancePtr, u32 SlrIndex)
{
	if(SlrIndex <= XNVM_SLR_INDEX_3){
		InstancePtr->SlrIndex = SlrIndex;
	    return XST_SUCCESS;
	}
	else
		return  XST_FAILURE;
}
#ifdef XNVM_ACCESS_PUF_USER_DATA
/****************************************************************************/
/**
 * This function is used to initialize XNvm_EfusePufFuse structure with user
 * provided data and assign the same to global structure XNvm_EfusePufFuse to
 * program PUF Fuses.
 *
 *
 * @param	PufFuse Pointer to XNvm_EfusePufFuse structure
 *
 * @return
 *		- XST_SUCCESS - On successful initialization
 *		- Error Code - On failure.
 *
 ******************************************************************************/
static int XilNvm_EfuseInitPufFuses(XNvm_EfusePufFuseAddr *PufFuse, u32 Idx)
{
	int Status = XST_FAILURE;

	PufFuse->PrgmPufFuse = WritePufFusesFlag[Idx];
	if (PufFuse->PrgmPufFuse == TRUE) {
		Status = XilNvm_ValidateUserFuseStr(PufFuses[Idx]);
		if (Status != XST_SUCCESS) {
			xil_printf("PufFuse string validation failed\r\n");
			goto END;
		}

		if (strlen(PufFuses[Idx]) != (NumOfPufFusesVal[Idx] *
				XNVM_EFUSE_ROW_STRING_LEN)) {
			goto END;
		}

		Status = Xil_ConvertStringToHex(
				PufFuses[Idx],
				(u32 *)(UINTPTR)PufFuse->PufFuseDataAddr,
				(NumOfPufFusesVal[Idx] *
				XNVM_EFUSE_ROW_STRING_LEN));
		if (Status != XST_SUCCESS) {
			goto END;
		}

		PufFuse->StartPufFuseRow = XNVM_EFUSE_PRGM_PUF_FUSE_NUM;
		PufFuse->NumOfPufFusesRows = NumOfPufFusesVal[Idx];
		PufFuse->EnvMonitorDis= EnvMonitorDisFlag[Idx];
	}
	Status = XST_SUCCESS;
END:
	return Status;
}

/******************************************************************************/
/**
 * This function writes user data in to PUF Fuses
 *
 * @return
 *	XST_SUCCESS	- On success
 *	Errorcode - On failure
 *
 ******************************************************************************/
static int XilNvm_EfuseWritePufFuses(u32 Idx)
{
	int Status = XST_FAILURE;
	XNvm_EfusePufFuseAddr *PufFuses = (XNvm_EfusePufFuseAddr*)(UINTPTR)&SharedMem[0U];
	u32 *PufFusesArr = (u32*)(UINTPTR)(PufFuses + Align(sizeof(XNvm_EfusePufFuseAddr)));

	Status = Xil_SMemSet(PufFusesArr, sizeof(u32) * NumOfPufFusesVal[Idx], 0U,
			sizeof(u32) * NumOfPufFusesVal[Idx]);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	PufFuses->PufFuseDataAddr = (UINTPTR)PufFusesArr;
	Status = XilNvm_EfuseInitPufFuses(PufFuses, Idx);
	if(Status != XST_SUCCESS) {
		goto END;
	}
	Xil_DCacheFlushRange((UINTPTR)PufFusesArr, sizeof(u32) * NumOfPufFusesVal[Idx]);
	Xil_DCacheFlushRange((UINTPTR)PufFuses, sizeof(XNvm_EfusePufFuseAddr));

	if (PufFuses->PrgmPufFuse == TRUE) {
		/* Write PUF Fuses */
		Status = XNvm_EfuseWritePufAsUserFuses(&NvmClientInstance, (UINTPTR)PufFuses);
		if (Status != XST_SUCCESS) {
			goto END;
		}
	}
	Status = XST_SUCCESS;
END:
	return Status;
}

/******************************************************************************/
/**
 * This function reads data from PUF Fuses
 *
 * @return
 *	XST_SUCCESS	- On success
 *	Errorcode - On failure
 *
 ******************************************************************************/
static int XilNvm_EfuseReadPufFuses(u32 Idx)
{
	int Status = XST_FAILURE;
	XNvm_EfusePufFuseAddr *PufFuses = (XNvm_EfusePufFuseAddr*)(UINTPTR)&SharedMem[0U];
	u32 Row = 0U;

	/* Init data */
	PufFuses->PufFuseDataAddr = (UINTPTR)(SharedMem + Align(sizeof(XNvm_EfusePufFuseAddr)));
	PufFuses->StartPufFuseRow = XNVM_EFUSE_READ_PUF_FUSE_NUM;
	PufFuses->NumOfPufFusesRows = ReadNumOfPufFusesVal[Idx];

	Xil_DCacheFlushRange((UINTPTR)(SharedMem + Align(sizeof(XNvm_EfusePufFuseAddr))),
		(ReadNumOfPufFusesVal[Idx] * sizeof(u32)));
	Xil_DCacheInvalidateRange((UINTPTR)PufFuses, sizeof(XNvm_EfusePufFuseAddr));

	/* Read PUF Fuses */
	Status = XNvm_EfuseReadPufAsUserFuses(&NvmClientInstance, (UINTPTR)PufFuses);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Xil_DCacheFlushRange((UINTPTR)(SharedMem + Align(sizeof(XNvm_EfusePufFuseAddr))),
		(ReadNumOfPufFusesVal[Idx] * sizeof(u32)));
	Xil_DCacheInvalidateRange((UINTPTR)PufFuses, sizeof(XNvm_EfusePufFuseAddr));

	for (Row = XNVM_EFUSE_READ_PUF_FUSE_NUM;
		Row < (XNVM_EFUSE_READ_PUF_FUSE_NUM +
			ReadNumOfPufFusesVal[Idx]); Row++) {
		xil_printf("User eFuse(PufHd)%d:%08x\n\r",
			Row, *(u32 *)(UINTPTR)(PufFuses->PufFuseDataAddr +
			Row - XNVM_EFUSE_READ_PUF_FUSE_NUM));
	}

	Status = XST_SUCCESS;
END:
	return Status;
}

#endif
#ifdef XNVM_EN_ADD_PPKS
/****************************************************************************/
/**
 * This function is used to initialize the XNvm_EfuseAdditionalPpkHash structure with
 * user provided data and assign it to global structure XNvm_EfuseDataAddr to
 * program PPK3/PPK4 hash eFuses and PPK invalid eFuses
 *
 * typedef struct {
 *	u8 PrgmPpk3Hash;
 *	u8 PrgmPpk4Hash;
 *	u32 Ppk3Hash[XNVM_EFUSE_PPK_HASH_LEN_IN_WORDS];
 *	u32 Ppk4Hash[XNVM_EFUSE_PPK_HASH_LEN_IN_WORDS];
 *  } XNvm_EfuseAdditionalPpkHash;
 *
 * @param	WriteEfuse	Pointer to XNvm_EfuseDataAddr structure.
 *
 * @param 	PpkHash		Pointer to XNvm_EfuseAdditionalPpkHash structure.
 *
 * @return
 *		- XST_SUCCESS - If initialization of XNvm_EfuseAdditionalPpkHash structure
 *				is successful
 *		- ErrorCode - On Failure
 *
 ******************************************************************************/
static int XilNvm_EfuseInitAdditionalPpkHash(XNvm_EfuseDataAddr *WriteEfuse,
		XNvm_EfuseAdditionalPpkHash *PpkHash, u32 Idx)
{
	int Status = XST_FAILURE;

	PpkHash->PrgmPpk3Hash = Ppk3HashFlag[Idx];
	PpkHash->PrgmPpk4Hash = Ppk4HashFlag[Idx];

	if (PpkHash->PrgmPpk3Hash == TRUE) {
		Status = XilNvm_ValidateHash((char *)Ppk3Hash[Idx],
					XNVM_EFUSE_PPK_HASH_STRING_LEN);
		if(Status != XST_SUCCESS) {
			xil_printf("Ppk3Hash string validation failed\r\n");
			goto END;
		}
		Status = Xil_ConvertStringToHexBE((char *)Ppk3Hash[Idx],
						(u8 *)PpkHash->Ppk3Hash,
			XNVM_EFUSE_PPK_HASH_LEN_IN_BITS);
		if (Status != XST_SUCCESS) {
			goto END;
		}
	}

	if (PpkHash->PrgmPpk4Hash == TRUE) {
		Status = XilNvm_ValidateHash((char *)Ppk4Hash[Idx],
					XNVM_EFUSE_PPK_HASH_STRING_LEN);
		if(Status != XST_SUCCESS) {
			xil_printf("Ppk4Hash string validation failed\r\n");
			goto END;
		}
		Status = Xil_ConvertStringToHexBE((char *)Ppk4Hash[Idx],
					(u8 *)PpkHash->Ppk4Hash,
					XNVM_EFUSE_PPK_HASH_LEN_IN_BITS);
		if (Status != XST_SUCCESS) {
			goto END;
		}
	}

	if (Status == XST_SUCCESS) {
		Xil_DCacheFlushRange((UINTPTR)PpkHash,
			sizeof(XNvm_EfuseAdditionalPpkHash));
		WriteEfuse->AdditionalPpkHashAddr = (UINTPTR)PpkHash;
	}

	Status = XST_SUCCESS;

END:
	return Status;
}
#endif /* END OF XNVM_EN_ADD_PPKS */
/** //! [XNvm eFuse example] */
/**@}*/
