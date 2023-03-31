/******************************************************************************
* Copyright (c) 2019 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022-2023 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
*
* @file xsecure_kat.c
*
* This file contains known answer tests common for both versal and versalnet
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- ---------- -------------------------------------------------------
* 5.0   kpt  07/15/2022 Initial release
*       kpt  08/03/2022 Added volatile keyword to avoid compiler optimization
*                       of loop redundancy checks
*       dc   08/26/2022 Removed initializations of arrays
*
* </pre>
*
* @note
*
******************************************************************************/

/***************************** Include Files *********************************/
#include "xparameters.h"
#include "xsecure_error.h"
#ifndef PLM_RSA_EXCLUDE
#include "xsecure_rsa.h"
#endif
#include "xsecure_kat.h"
#include "xil_util.h"

/************************** Constant Definitions *****************************/

/**
 * @name AES KAT parameters
 * @{
 */
/**< AES KAT parameters */
#define XSECURE_KAT_AES_SPLIT_DATA_SIZE		(4U)
#define XSECURE_KAT_KEY_SIZE_IN_WORDS		(8U)
#define XSECURE_KAT_OPER_DATA_SIZE_IN_WORDS	(16U)
/** @} */

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

static const u8 KatMessage[XSECURE_KAT_MSG_LEN_IN_BYTES] = {
	0x2FU, 0xBFU, 0x02U, 0x9EU, 0xE9U, 0xFBU, 0xD6U, 0x11U,
	0xC2U, 0x4DU, 0x81U, 0x4EU, 0x6AU, 0xFFU, 0x26U, 0x77U,
	0xC3U, 0x5AU, 0x83U, 0xBCU, 0xE5U, 0x63U, 0x2CU, 0xE7U,
	0x89U, 0x43U, 0x6CU, 0x68U, 0x82U, 0xCAU, 0x1CU, 0x71U
};

static const u8 AesKey[XSECURE_KAT_KEY_SIZE_IN_BYTES] = {
	0xD4U, 0x16U, 0xA6U, 0x93U, 0x1DU, 0x52U, 0xE0U, 0xF5U,
	0x0AU, 0xA0U, 0x89U, 0xA7U, 0x57U, 0xB1U, 0x1AU, 0x89U,
	0x1CU, 0xBDU, 0x1BU, 0x83U, 0x84U, 0x7DU, 0x4BU, 0xEDU,
	0x9EU, 0x29U, 0x38U, 0xCDU, 0x4CU, 0x54U, 0xA8U, 0xBAU
};

static const u8 AesIv[XSECURE_KAT_IV_SIZE_IN_BYTES] = {
	0x85U, 0x36U, 0x5FU, 0x88U, 0xB0U, 0xB5U, 0x62U, 0x98U,
	0xDFU, 0xEAU, 0x5AU, 0xB2U, 0x00U, 0X00U, 0x00U, 0x00U
};

static const u8 AesCt[XSECURE_KAT_MSG_LEN_IN_BYTES] = {
	0x59U, 0x8CU, 0xD1U, 0x9FU, 0x16U, 0x83U, 0xB4U, 0x1BU,
	0x4CU, 0x59U, 0xE1U, 0xC1U, 0x57U, 0xD4U, 0x15U, 0x01U,
	0xA3U, 0xC0U, 0x89U, 0x02U, 0xF0U, 0xEAU, 0x3AU, 0x37U,
	0x6AU, 0x8BU, 0x0DU, 0x99U, 0x88U, 0xCFU, 0xF8U, 0xC1U
};

static const u8 AesGcmTag[XSECURE_SECURE_GCM_TAG_SIZE] = {
	0xADU, 0xCEU, 0xFEU, 0x2FU, 0x6EU, 0xE4U, 0xC7U, 0x06U,
	0x0EU, 0x44U, 0xAAU, 0x5EU, 0xDFU, 0x0DU, 0xBEU, 0xBCU
};

static const u8 AesAadData[XSECURE_KAT_AAD_SIZE_IN_BYTES] = {
	0x9AU, 0x7BU, 0x86U, 0xE7U, 0x82U, 0xCCU, 0xAAU, 0x6AU,
	0xB2U, 0x21U, 0xBDU, 0x03U, 0x47U, 0x0BU, 0xDCU, 0x2EU
};

static const u8 ExpSha3Hash[XSECURE_HASH_SIZE_IN_BYTES] = {
	0xFFU, 0x4EU, 0x69U, 0xA1U, 0x4CU, 0xBCU, 0xBDU, 0x93U,
	0xBEU, 0xAAU, 0xB1U, 0xC4U, 0x7FU, 0x57U, 0x8BU, 0x34U,
	0x6DU, 0x54U, 0x88U, 0x93U, 0xADU, 0xEDU, 0x45U, 0xA3U,
	0x5FU, 0xE1U, 0xCAU, 0x65U, 0xB4U, 0x56U, 0x40U, 0x1EU,
	0xC0U, 0x40U, 0xE5U, 0x67U, 0xD1U, 0x61U, 0x20U, 0xDDU,
	0x9CU, 0x45U, 0x89U, 0x72U, 0x5CU, 0x58U, 0xBFU, 0x02U
};

#ifndef PLM_RSA_EXCLUDE
static const u32 RsaModulus[XSECURE_RSA_4096_SIZE_WORDS] = {
	0x6DABEC96U, 0x097DBAFCU, 0xF6361AA5U, 0x77245773U,
	0x6197AFF4U, 0x5A8BC11AU, 0x5CB32F2EU, 0xADF8F12AU,
	0x16506FD2U, 0xE6F483B4U, 0x3E25DE76U, 0x2E947131U,
	0x6ABB0003U, 0x66FC5B01U, 0x401B7DE9U, 0x5E7F9C21U,
	0x0FA19EAAU, 0xAB82063BU, 0x6116012FU, 0xD06EC1B3U,
	0xF16F5FE0U, 0x71060C1EU, 0x671B92E2U, 0xD6360CB0U,
	0xA3D4088CU, 0x29FD96DBU, 0x905A0309U, 0x16FF9DE0U,
	0xB4BED53FU, 0x6A831031U, 0x7D863E56U, 0x1276B2C6U,
	0x2EF16B23U, 0xC4369382U, 0x73DE2BBBU, 0xD85A4B22U,
	0x99712AC3U, 0xA4CF9419U, 0xC53F343DU, 0x531457C3U,
	0xE90D51D6U, 0xD836829FU, 0x4461612CU, 0xA1FA13D5U,
	0x49D2EED2U, 0xC7733347U, 0x0E752E03U, 0xA6BAD022U,
	0x3BEBEF65U, 0x0C82DE9BU, 0x47FFE0FFU, 0x47FDFC43U,
	0x5D6B4441U, 0xEB5953DDU, 0x1B2FC607U, 0xED2B71E5U,
	0x7B1BFB92U, 0xAEC773BCU, 0xBFDCE535U, 0x08AB3C4AU,
	0x36BBDC83U, 0x8835BCB7U, 0x5CB67DFBU, 0xF92A6823U,
	0xA113ABEBU, 0x51FFC507U, 0x1C49DEC3U, 0xC71A37CEU,
	0x8AB4A8C5U, 0xA6FAE723U, 0xFB3C78C9U, 0xA7E039F1U,
	0x50B08D3DU, 0x08E8D120U, 0x906D956FU, 0xACD1A965U,
	0xA1884EA9U, 0x14D0D384U, 0x65BEC642U, 0xD80BEE76U,
	0x07EC450BU, 0x6626B758U, 0xB8583DEDU, 0x18BAD520U,
	0xAE5E9E97U, 0xEC7207B8U, 0xE1F7C581U, 0xEF2FF5C4U,
	0xEEBA98A4U, 0x2D5DCE76U, 0xA97E9BF5U, 0xFC5E89EAU,
	0xF88A7C98U, 0x147E5BDEU, 0xA0FE3060U, 0xA076011EU,
	0xA4712143U, 0x650281E7U, 0xCB567318U, 0x2348361DU,
	0xCA754CC6U, 0x58AAE4A5U, 0x42D6DEA4U, 0xF8CD5AC0U,
	0x9D2F8190U, 0x5E06659CU, 0xEF2805C0U, 0x88238189U,
	0x484E9678U, 0xC97BC278U, 0x122F0BA1U, 0xCB1795CCU,
	0x76831D6EU, 0x1325A9FDU, 0x8376536CU, 0x9C3BCEE2U,
	0x2727A9E9U, 0x96F83CFEU, 0x5036E4F4U, 0x116DC93AU,
	0x62532B5CU, 0xED96664DU, 0x2F666376U, 0xE77CCA6DU,
	0xA72D26A4U, 0xA2865341U, 0xC2334D7EU, 0x4552DDCCU
};

static const u32 RsaModExt[XSECURE_RSA_4096_SIZE_WORDS] = {
	0x0F175331U, 0xABF5BEDDU, 0x34DE5D96U, 0x09D7BD87U,
	0x78588C6CU, 0x14919551U, 0x42E2552CU, 0xF720F110U,
	0xD98D3E69U, 0xA2503F40U, 0xE13E8B33U, 0x574352DAU,
	0xE5CE6418U, 0x4536D713U, 0xECDDC3ECU, 0x1196F239U,
	0x6AD60D3CU, 0x73BEFAE5U, 0x2096371AU, 0x24607A94U,
	0x0523527BU, 0xDFAADDBDU, 0x42EC55DFU, 0xDA47D661U,
	0xEC94CD1EU, 0xF8A5A6C5U, 0xB0129E13U, 0x570DA3FEU,
	0xA29B1873U, 0x0E0EBFE4U, 0xD78B3075U, 0x3AD43726U,
	0xC7E437EAU, 0x75C7D576U, 0x7B8B55BBU, 0x708E394AU,
	0xC70C9794U, 0x565E5C72U, 0x71A6D27BU, 0xCE6C8E63U,
	0xC9E2CC3DU, 0x2D5A8D41U, 0x502E9674U, 0x33C9F95BU,
	0x1723AD20U, 0xC22699C5U, 0x098F8703U, 0x155A85F2U,
	0xA3F2F679U, 0x5C37A7F1U, 0x89DFEC10U, 0x17393BECU,
	0x7A36F7D4U, 0xF700EEB7U, 0x2B250850U, 0x9477B359U,
	0xBD4D1999U, 0x614C747EU, 0x9EBA2BCEU, 0x218F1413U,
	0xD04B3237U, 0x6DDE20A3U, 0x3D834308U, 0x4B4593E7U,
	0x4946B20BU, 0x1BF7C80DU, 0x257C524AU, 0x967FAF75U,
	0x601EAD3AU, 0xBEBB957DU, 0x5297ABA4U, 0x4C8C268CU,
	0xEA5A7F10U, 0x79F162C6U, 0x5B2BAA5EU, 0xBEE4627EU,
	0x051E0C4FU, 0x0EA9DB95U, 0x4D6BF195U, 0x6DBEFFB3U,
	0x730EC43BU, 0xB37294F4U, 0xE3F1A594U, 0xB50F5C88U,
	0x6EDE6F29U, 0x10F6A67DU, 0x228C42BBU, 0x404203A0U,
	0x35C7B1FEU, 0xFAA50368U, 0x9935562EU, 0xA46369B4U,
	0x15146975U, 0xCCAB5323U, 0x2396613DU, 0x7A5043A8U,
	0xA8349A80U, 0xB78DF0E4U, 0xF1650CDAU, 0xF4026876U,
	0x1458FADCU, 0xFD4B1C39U, 0xF0561B58U, 0xBA54B99BU,
	0x41EBF429U, 0xF7E84CCCU, 0x7F801955U, 0x3077EF0FU,
	0xB77DB572U, 0xA8ED483BU, 0x8D80DA71U, 0x88A3C09DU,
	0xB1E18F16U, 0x79711D2AU, 0xD178B630U, 0xD2F587CDU,
	0xB817D675U, 0x737E1425U, 0x7511C0F3U, 0xDD619F3BU,
	0xB8DBAF4CU, 0x5395AC6EU, 0x18EDA4C6U, 0xC0F19079U,
	0x12BC4D35U, 0x225466D2U, 0x4FEA7C11U, 0x81E5B172U
};

static const u32 RsaData[XSECURE_RSA_4096_SIZE_WORDS] = {
	0xFB571486U, 0xE1249012U, 0x3E761408U, 0x1A20EA70U,
	0xFEC2F788U, 0xB9D26394U, 0x28EDACFAU, 0xF315279AU,
	0x14EDEFE9U, 0x615DDC05U, 0x7CC541E2U, 0xBEDFCBD5U,
	0x10340132U, 0x12D0B62BU, 0xDA0DF681U, 0x1483D434U,
	0x1EC8B752U, 0x84BE665EU, 0xA36B6D8EU, 0x3206E9C3U,
	0x674E9334U, 0x8A9C9A1EU, 0x0C00A671U, 0xC99232C4U,
	0x4EDADD32U, 0x772A1723U, 0x2D6AC26CU, 0x483A595CU,
	0x52396F81U, 0x11ADC94EU, 0x1D51959FU, 0x08F031B0U,
	0x252F738BU, 0x228A0761U, 0x07943828U, 0x6B4FC9F4U,
	0x95C40061U, 0xD12E3B31U, 0xC4AE32F7U, 0x29F5440DU,
	0x62196195U, 0x1251FB63U, 0xF7C9CCF5U, 0x389CC3A0U,
	0x5FDB5FD8U, 0xEC92259AU, 0x15CD9DBBU, 0x5CEB61F9U,
	0xC809EFE5U, 0x164D9BC2U, 0x8015E50EU, 0x2C4CCB5BU,
	0x51974B9DU, 0xC593D13BU, 0x12D1A12EU, 0xA12DF15EU,
	0xB9502495U, 0xA7606F27U, 0xCA39CCA3U, 0xD1E1C5E0U,
	0xF59010DDU, 0xABD85F14U, 0x383D1336U, 0x003CB96AU,
	0xB44E2673U, 0xE5010A85U, 0xEB30A62BU, 0x5EC874C3U,
	0x92D071A2U, 0x2A77897FU, 0x33D175C5U, 0x7E8262EEU,
	0xDB7A4C30U, 0xB905302FU, 0x11B211B0U, 0xBE7686BFU,
	0xB4A310ECU, 0x9860381FU, 0x3FFDB62FU, 0xA9363083U,
	0xC204F6CAU, 0x524D78CDU, 0xED9DEF22U, 0xB6E423ADU,
	0xE2D72240U, 0xC54F6C4AU, 0xBC7534AAU, 0xB5C983ACU,
	0x34905EE3U, 0x1C091B10U, 0xB5BA27E5U, 0x1246388FU,
	0x638DF44FU, 0x4228AA9EU, 0xB26EC356U, 0xAF51C1D4U,
	0x3FE93343U, 0x55416B38U, 0x7CDCDC6EU, 0x4514C6E6U,
	0xF4525F26U, 0x71D1875BU, 0xB51DDE41U, 0x5B29350CU,
	0xFC4324B6U, 0x083794F3U, 0xE17F4C94U, 0xCFAAD44FU,
	0xC7785A1CU, 0xD6C2B762U, 0x484F880EU, 0x3EA80383U,
	0x1EA7DEE1U, 0x6E80A231U, 0x50AE13E4U, 0x4920CD14U,
	0x564316BCU, 0x6787A7B0U, 0x31D0AA8BU, 0x5B2E3027U,
	0xB162BD0CU, 0xEF4D0B8DU, 0x3F202FA6U, 0xF700AA63U,
	0x9846789EU, 0x64187374U, 0xCE81837EU, 0xA1274EC4U
};

static const u32 RsaExpCtData[XSECURE_RSA_4096_SIZE_WORDS] = {
	0xC8C47C10U, 0x28563666U, 0x005C504DU, 0xB5DCE14DU,
	0x01931D25U, 0x40BE6DD3U, 0x17D35960U, 0xD5F0BA6AU,
	0x58D0BBC8U, 0xD3BCEBBCU, 0x4DE97D9DU, 0x3025554DU,
	0x7B3B8139U, 0x55F3EC35U, 0x8A4E9E65U, 0xE9DE8028U,
	0x930C12A1U, 0xDD04B17EU, 0xF80F66BAU, 0x4ECC74C0U,
	0xFFF5B068U, 0x3CBA07B6U, 0x343E3FDAU, 0x96513360U,
	0xBC8EADE6U, 0xC87A9CD4U, 0x5349A60CU, 0xDB028D77U,
	0x4C5984B0U, 0xDFF22EF3U, 0x107E2B23U, 0x1E0EB177U,
	0x2D9B8F00U, 0xC1368502U, 0x7DBA9F9CU, 0xA0841C9EU,
	0x7DF1EC05U, 0x8B6E0E98U, 0xA6CF4DA5U, 0x2F413B44U,
	0x4C9DF445U, 0xD28D872CU, 0x01C97364U, 0xD0DC2616U,
	0x4D0BD1F2U, 0x4FFA35E3U, 0x9BFB8793U, 0x696F1B3AU,
	0x4EA75CAAU, 0x9F0EE444U, 0x64C6603AU, 0xCEAB43FEU,
	0x7492131AU, 0x7E6AA34EU, 0x0B310E31U, 0x47B2DC57U,
	0xB5585B1EU, 0xD5CD1B75U, 0xC6CAAA26U, 0x806D565EU,
	0x9AC9D603U, 0xD29AA0CAU, 0xEE2EA30DU, 0x29868EDCU,
	0x6049B71AU, 0xFCF4C825U, 0x52336F53U, 0xE4B642E8U,
	0xC3ABE4E7U, 0x0EC5B478U, 0xC30036E8U, 0x835EA464U,
	0x27AC8FE6U, 0xB0552DA6U, 0xAA5403F8U, 0xC29CAE33U,
	0x242D908FU, 0xF6BC8D5AU, 0x3D6ECD7AU, 0x663F8235U,
	0x0AD2D02BU, 0x94C35B62U, 0x647159DEU, 0xC8B4D4D4U,
	0xB9C7799DU, 0x92874E45U, 0xC1076303U, 0x095620B3U,
	0x54A914B7U, 0x3378325EU, 0x93A46841U, 0x6B8AC70EU,
	0x53EE5DD7U, 0x26BBE120U, 0x646F0184U, 0xA556EE66U,
	0x17CA737BU, 0x75C64D30U, 0xFFA4E442U, 0x107663B9U,
	0xD3CC8A36U, 0x3D1F9819U, 0x27B89F1FU, 0x49BDF558U,
	0x351D5A03U, 0x9E4002B5U, 0xBA84EF4EU, 0x240E268EU,
	0x6D7D66C3U, 0x5907EE0CU, 0xF4338BE3U, 0x20A0F7CCU,
	0x96874733U, 0xAADBE8A1U, 0x7133D3CDU, 0x7D467ACCU,
	0xEE0CDC6BU, 0x3BAC17BAU, 0xC582C9B0U, 0x1586CDD3U,
	0x56BD0228U, 0x51D760BFU, 0x573D39D2U, 0x83100F6DU,
	0x578DB621U, 0xF6FD8218U, 0x9262BBCBU, 0xE04381CDU
};
#endif

#ifndef PLM_ECDSA_EXCLUDE

/* When Elliptic APIs operate on BIG endian data, KAT inputs are stored in Big Endian */
#if XSECURE_ELLIPTIC_ENDIANNESS
static const u8 Pubkey_P384[XSECURE_ECC_P384_SIZE_IN_BYTES +
			XSECURE_ECC_P384_SIZE_IN_BYTES] = {
	0x53, 0x20, 0x70, 0xB5, 0xA6, 0x78, 0x5C, 0xC6,
	0x1F, 0x5B, 0xD3, 0x08, 0xDA, 0x6A, 0xA5, 0xAB,
	0xDF, 0xF8, 0x19, 0x88, 0xB6, 0x5E, 0x91, 0x2F,
	0x7A, 0xC6, 0x80, 0x77, 0x20, 0x8B, 0xD3, 0x56,
	0x9C, 0x7B, 0xD8, 0x5C, 0x87, 0xEC, 0x02, 0xEC,
	0x2A, 0x23, 0x33, 0x8A, 0xD7, 0x5B, 0x73, 0x39,
	0x2A, 0xDA, 0x1B, 0x44, 0x0E, 0xFF, 0xBD, 0xE9,
	0xAC, 0x52, 0x96, 0x92, 0x05, 0xAA, 0xBE, 0xB1,
	0x31, 0x83, 0xD5, 0xD6, 0x0B, 0xA1, 0xFA, 0x1B,
	0xBA, 0xE5, 0x80, 0x42, 0x07, 0xD2, 0x0D, 0x4F,
	0x05, 0x7D, 0xA1, 0x3F, 0x00, 0x51, 0xB9, 0x7F,
	0x03, 0xFB, 0x01, 0x27, 0x44, 0x7F, 0x33, 0xCF
};

static const u8 Sign_P384[XSECURE_ECC_P384_SIZE_IN_BYTES +
			XSECURE_ECC_P384_SIZE_IN_BYTES] = {
	0x06, 0xCF, 0x53, 0xBF, 0xD9, 0xD2, 0x9A, 0x5F,
	0x22, 0x49, 0x3D, 0x42, 0x58, 0x6E, 0x66, 0xD0,
	0xB6, 0xE6, 0x97, 0x03, 0xAC, 0x7F, 0x24, 0x64,
	0x7B, 0x29, 0x2B, 0xB5, 0xF8, 0x8D, 0x89, 0x1C,
	0x5B, 0x7A, 0x13, 0x74, 0x76, 0x18, 0xCF, 0xCA,
	0xEB, 0x24, 0x06, 0xCA, 0x9F, 0x52, 0x8C, 0xB9,
	0x64, 0x5E, 0x28, 0xE7, 0x65, 0xB2, 0xCE, 0xF2,
	0x50, 0x5D, 0xD8, 0x8C, 0x9A, 0x88, 0x1D, 0x62,
	0xAD, 0x69, 0x02, 0x84, 0x01, 0x58, 0x39, 0xA3,
	0x47, 0xEB, 0x49, 0xD7, 0x8D, 0xB2, 0x41, 0xC8,
	0x5A, 0x1D, 0x3D, 0x12, 0xF7, 0x92, 0x23, 0x4C,
	0x95, 0xE0, 0xDB, 0xD3, 0x10, 0xB7, 0xEA, 0xE2
};

static const u8 D_P384[XSECURE_ECC_P384_SIZE_IN_BYTES] = {
	0x9F, 0x29, 0xCD, 0x59, 0xD3, 0x49, 0xDC, 0x56,
	0x20, 0x4A, 0x0D, 0x1B, 0x24, 0xA8, 0x04, 0xBF,
	0x9D, 0x16, 0xA2, 0x20, 0x9B, 0x04, 0x34, 0xFC,
	0x3E, 0x6F, 0xE8, 0x9F, 0x4E, 0x5D, 0xEE, 0x24,
	0xCC, 0x74, 0x20, 0xBA, 0x10, 0x61, 0xFF, 0xB7,
	0x1D, 0x02, 0x5D, 0x89, 0x74, 0x2A, 0x21, 0x9C
};

static const u8 K_P384[XSECURE_ECC_P521_SIZE_IN_BYTES] = {
	0x00, 0x2E, 0xC0, 0xFA, 0xEB, 0xC1, 0x7E, 0x7F,
	0xA2, 0xDC, 0xDB, 0xC1, 0x1A, 0x94, 0x04, 0x90,
	0x22, 0xB2, 0xCD, 0xC2, 0xAF, 0xCE, 0x60, 0x42,
	0x8A, 0x97, 0x20, 0x6F, 0xB3, 0xF3, 0x51, 0x94,
	0xD7, 0x5E, 0x7B, 0x1D, 0x30, 0x5A, 0x30, 0x69,
	0xE5, 0x90, 0xB1, 0x38, 0x00, 0x25, 0x04, 0x04
};

static const u8 K_P521[XSECURE_ECC_P521_SIZE_IN_BYTES] = {
	0x00, 0x00, 0xC9, 0x1E, 0x23, 0x49, 0xEF, 0x6C,
	0xA2, 0x2D, 0x2D, 0xE3, 0x9D, 0xD5, 0x18, 0x19,
	0xB6, 0xAA, 0xD9, 0x22, 0xD3, 0xAE, 0xCD, 0xEA,
	0xB4, 0x52, 0xBA, 0x17, 0x2F, 0x7D, 0x63, 0xE3,
	0x70, 0xCE, 0xCD, 0x70, 0x57, 0x5F, 0x59, 0x7C,
	0x09, 0xA1, 0x74, 0xBA, 0x76, 0xBE, 0xD0, 0x5A,
	0x48, 0xE5, 0x62, 0xBE, 0x06, 0x25, 0x33, 0x6D,
	0x16, 0xB8, 0x70, 0x31, 0x47, 0xA6, 0xA2, 0x31,
	0x04, 0x04
};

static const u8 ExpEccSha3Hash[XSECURE_KAT_ECC_P521_SHA3_HASH_SIZE_IN_BYTES] = {
	0xFF, 0x4E, 0x69, 0xA1, 0x4C, 0xBC, 0xBD, 0x93,
	0xBE, 0xAA, 0xB1, 0xC4, 0x7F, 0x57, 0x8B, 0x34,
	0x6D, 0x54, 0x88, 0x93, 0xAD, 0xED, 0x45, 0xA3,
	0x5F, 0xE1, 0xCA, 0x65, 0xB4, 0x56, 0x40, 0x1E,
	0xC0, 0x40, 0xE5, 0x67, 0xD1, 0x61, 0x20, 0xDD,
	0x9C, 0x45, 0x89, 0x72, 0x5C, 0x58, 0xBF, 0x02,
	0X00, 0X00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0X00, 0X00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00
};

/* When Elliptic APIs operate on LITTLE endian data, KAT inputs are stored in Little Endian */
#else
static const u8 Pubkey_P384[XSECURE_ECC_P384_SIZE_IN_BYTES +
			XSECURE_ECC_P384_SIZE_IN_BYTES] = {
	0x39U, 0x73U, 0x5BU, 0xD7U, 0x8AU, 0x33U, 0x23U, 0x2AU,
	0xECU, 0x02U, 0xECU, 0x87U, 0x5CU, 0xD8U, 0x7BU, 0x9CU,
	0x56U, 0xD3U, 0x8BU, 0x20U, 0x77U, 0x80U, 0xC6U, 0x7AU,
	0x2FU, 0x91U, 0x5EU, 0xB6U, 0x88U, 0x19U, 0xF8U, 0xDFU,
	0xABU, 0xA5U, 0x6AU, 0xDAU, 0x08U, 0xD3U, 0x5BU, 0x1FU,
	0xC6U, 0x5CU, 0x78U, 0xA6U, 0xB5U, 0x70U, 0x20U, 0x53U,
	0xCFU, 0x33U, 0x7FU, 0x44U, 0x27U, 0x01U, 0xFBU, 0x03U,
	0x7FU, 0xB9U, 0x51U, 0x00U, 0x3FU, 0xA1U, 0x7DU, 0x05U,
	0x4FU, 0x0DU, 0xD2U, 0x07U, 0x42U, 0x80U, 0xE5U, 0xBAU,
	0x1BU, 0xFAU, 0xA1U, 0x0BU, 0xD6U, 0xD5U, 0x83U, 0x31U,
	0xB1U, 0xBEU, 0xAAU, 0x05U, 0x92U, 0x96U, 0x52U, 0xACU,
	0xE9U, 0xBDU, 0xFFU, 0x0EU, 0x44U, 0x1BU, 0xDAU, 0x2AU
};

static const u8 Sign_P384[XSECURE_ECC_P384_SIZE_IN_BYTES +
			XSECURE_ECC_P384_SIZE_IN_BYTES] = {
	0xB9U, 0x8CU, 0x52U, 0x9FU, 0xCAU, 0x06U, 0x24U, 0xEBU,
	0xCAU, 0xCFU, 0x18U, 0x76U, 0x74U, 0x13U, 0x7AU, 0x5BU,
	0x1CU, 0x89U, 0x8DU, 0xF8U, 0xB5U, 0x2BU, 0x29U, 0x7BU,
	0x64U, 0x24U, 0x7FU, 0xACU, 0x03U, 0x97U, 0xE6U, 0xB6U,
	0xD0U, 0x66U, 0x6EU, 0x58U, 0x42U, 0x3DU, 0x49U, 0x22U,
	0x5FU, 0x9AU, 0xD2U, 0xD9U, 0xBFU, 0x53U, 0xCFU, 0x06U,
	0xE2U, 0xEAU, 0xB7U, 0x10U, 0xD3U, 0xDBU, 0xE0U, 0x95U,
	0x4CU, 0x23U, 0x92U, 0xF7U, 0x12U, 0x3DU, 0x1DU, 0x5AU,
	0xC8U, 0x41U, 0xB2U, 0x8DU, 0xD7U, 0x49U, 0xEBU, 0x47U,
	0xA3U, 0x39U, 0x58U, 0x01U, 0x84U, 0x02U, 0x69U, 0xADU,
	0x62U, 0x1DU, 0x88U, 0x9AU, 0x8CU, 0xD8U, 0x5DU, 0x50U,
	0xF2U, 0xCEU, 0xB2U, 0x65U, 0xE7U, 0x28U, 0x5EU, 0x64U
};

static const u8 D_P384[XSECURE_ECC_P384_SIZE_IN_BYTES] = {
	0x9CU, 0x21U, 0x2AU, 0x74U, 0x89U, 0x5DU, 0x02U, 0x1DU,
	0xB7U, 0xFFU, 0x61U, 0x10U, 0xBAU, 0x20U, 0x74U, 0xCCU,
	0x24U, 0xEEU, 0x5DU, 0x4EU, 0x9FU, 0xE8U, 0x6FU, 0x3EU,
	0xFCU, 0x34U, 0x04U, 0x9BU, 0x20U, 0xA2U, 0x16U, 0x9DU,
	0xBFU, 0x04U, 0xA8U, 0x24U, 0x1BU, 0x0DU, 0x4AU, 0x20U,
	0x56U, 0xDCU, 0x49U, 0xD3U, 0x59U, 0xCDU, 0x29U, 0x9FU
};

static const u8 K_P384[XSECURE_ECC_P384_SIZE_IN_BYTES] = {
	0x04U, 0x04U, 0x25U, 0x00U, 0x38U, 0xB1U, 0x90U, 0xE5U,
	0x69U, 0x30U, 0x5AU, 0x30U, 0x1DU, 0x7BU, 0x5EU, 0xD7U,
	0x94U, 0x51U, 0xF3U, 0xB3U, 0x6FU, 0x20U, 0x97U, 0x8AU,
	0x42U, 0x60U, 0xCEU, 0xAFU, 0xC2U, 0xCDU, 0xB2U, 0x22U,
	0x90U, 0x04U, 0x94U, 0x1AU, 0xC1U, 0xDBU, 0xDCU, 0xA2U,
	0x7FU, 0x7EU, 0xC1U, 0xEBU, 0xFAU, 0xC0U, 0x2EU, 0x00U
};

static const u8 K_P521[XSECURE_ECC_P521_SIZE_IN_BYTES] = {
	0x04U, 0x04U, 0x31U, 0xA2U, 0xA6U, 0x47U, 0x31U, 0x70U,
	0xB8U, 0x16U, 0x6DU, 0x33U, 0x25U, 0x06U, 0xBEU, 0x62U,
	0xE5U, 0x48U, 0x5AU, 0xD0U, 0xBEU, 0x76U, 0xBAU, 0x74U,
	0xA1U, 0x09U, 0x7CU, 0x59U, 0x5FU, 0x57U, 0x70U, 0xCDU,
	0xCEU, 0x70U, 0xE3U, 0x63U, 0x7DU, 0x2FU, 0x17U, 0xBAU,
	0x52U, 0xB4U, 0xEAU, 0xCDU, 0xAEU, 0xD3U, 0x22U, 0xD9U,
	0xAAU, 0xB6U, 0x19U, 0x18U, 0xD5U, 0x9DU, 0xE3U, 0x2DU,
	0x2DU, 0xA2U, 0x6CU, 0xEFU, 0x49U, 0x23U, 0x1EU, 0xC9U,
	0x00U, 0x00U
};

static const u8 ExpEccSha3Hash[XSECURE_KAT_ECC_P521_SHA3_HASH_SIZE_IN_BYTES] = {
	0x02, 0xBF, 0x58, 0x5C, 0x72, 0x89, 0x45, 0x9C,
	0xDD, 0x20, 0x61, 0xD1, 0x67, 0xE5, 0x40, 0xC0,
	0x1E, 0x40, 0x56, 0xB4, 0x65, 0xCA, 0xE1, 0x5F,
	0xA3, 0x45, 0xED, 0xAD, 0x93, 0x88, 0x54, 0x6D,
	0x34, 0x8B, 0x57, 0x7F, 0xC4, 0xB1, 0xAA, 0xBE,
	0x93, 0xBD, 0xBC, 0x4C, 0xA1, 0x69, 0x4E, 0xFF,
	0X00, 0X00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0X00, 0X00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00
};
#endif
#endif
/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

static int XSecure_AesDecCmChecks(const u32 *P, const u32 *Q, const u32 *R,
	const u32 *S);

/************************** Variable Definitions *****************************/


/*****************************************************************************/
/**
 * @brief	This function returns message to perform KAT
 *
 * @return
 *			message to perform KAT
 *
 *****************************************************************************/
u8* XSecure_GetKatMessage(void) {
	return (u8*)&KatMessage[0U];
}

/*****************************************************************************/
/**
 * @brief	This function returns AES key for KAT
 *
 * @return
 *			AES key for KAT
 *
 *****************************************************************************/
u8* XSecure_GetKatAesKey(void) {
	return (u8*)&AesKey[0U];
}

/*****************************************************************************/
/**
 * @brief	This function returns expected SHA3 hash for KAT
 *
 * @return
 *			Expected SHA3 hash for KAT
 *
 *****************************************************************************/
u8* XSecure_GetKatSha3ExpHash(void) {
	return (u8*)&ExpSha3Hash[0U];
}

#ifndef PLM_RSA_EXCLUDE
/*****************************************************************************/
/**
 * @brief	This function returns modulus for RSA KAT
 *
 * @return
 *			RSA modulus for KAT
 *
 *****************************************************************************/
u32* XSecure_GetKatRsaModulus(void) {
	return (u32*)&RsaModulus[0U];
}

/*****************************************************************************/
/**
 * @brief	This function returns public modext for RSA KAT
 *
 * @return
 *			RSA modext for KAT
 *
 *****************************************************************************/
u32* XSecure_GetKatRsaModExt(void) {
	return (u32*)&RsaModExt[0U];
}

/*****************************************************************************/
/**
 * @brief	This function returns data for RSA KAT
 *
 * @return
 *			RSA modext for KAT
 *
 *****************************************************************************/
u32* XSecure_GetKatRsaData(void) {
	return (u32*)&RsaData[0U];
}
#endif

#ifndef PLM_ECDSA_EXCLUDE
/*****************************************************************************/
/**
 * @brief	This function returns ECC public key to perform KAT
 *
 * @param	CrvClass ECC curve class
 *
 * @return
 *			ECC public key
 *
 * @note
 *			ECC core expects key in reverse order
 *
 *****************************************************************************/
XSecure_EllipticKey* XSecure_GetKatEccPublicKey(XSecure_EllipticCrvClass CrvClass) {
	static XSecure_EllipticKey ExpPubKey = {0U};

	if (CrvClass == XSECURE_ECC_PRIME) {
		ExpPubKey.Qx = (u8*)&Pubkey_P384[0U];
		ExpPubKey.Qy = (u8*)&Pubkey_P384[XSECURE_ECC_P384_SIZE_IN_BYTES];
	}
	else {
		ExpPubKey.Qx = NULL;
		ExpPubKey.Qy = NULL;
	}

	return &ExpPubKey;
}

/*****************************************************************************/
/**
 * @brief	This function returns ECC expected signature to perform KAT
 *
 * @param	CrvClass ECC curve class
 *
 * @return
 *			ECC expected signature
 *
 * @note
 *			ECC core expects sign in reverse order
 *
 *****************************************************************************/
XSecure_EllipticSign* XSecure_GetKatEccExpSign(XSecure_EllipticCrvClass CrvClass) {
	static XSecure_EllipticSign ExpSign = {0U};

	if (CrvClass == XSECURE_ECC_PRIME) {
		ExpSign.SignR = (u8*)&Sign_P384[0U];
		ExpSign.SignS = (u8*)&Sign_P384[XSECURE_ECC_P384_SIZE_IN_BYTES];
	}
	else {
		ExpSign.SignR = NULL;
		ExpSign.SignS = NULL;
	}

	return &ExpSign;
}

/*****************************************************************************/
/**
 * @brief	This function returns ECC private key to perform KAT
 *
 * @param	CrvClass ECC curve class
 *
 * @return
 *			ECC private key
 * @note
 *			ECC core expects key in reverse order
 *
 *****************************************************************************/
u8* XSecure_GetKatEccPrivateKey(XSecure_EllipticCrvClass CrvClass) {
	static u8 *D;

	if (CrvClass == XSECURE_ECC_PRIME) {
		D = (u8*)D_P384;
	}
	else {
		D = NULL;
	}

	return D;
}

/*****************************************************************************/
/**
 * @brief	This function returns ECC ehimeral key to perform KAT
 *
 * @param	CrvType ECC curve type
 *
 * @return
 *			ECC ehimeral key
 *
 * @note
 *			ECC core expects key in reverse order
 *
 *****************************************************************************/
u8* XSecure_GetKatEccEphimeralKey(XSecure_EllipticCrvTyp CrvType) {
	static u8 *K;

	if (CrvType == XSECURE_ECC_NIST_P384) {
		K = (u8*)K_P384;
	}
	else if (CrvType == XSECURE_ECC_NIST_P521) {
		K = (u8*)K_P521;
	}
	else {
		K = NULL;
	}

	return K;
}
#endif

/*****************************************************************************/
/**
 * @brief	This function performs known answer test(KAT) on AES engine to
 * 		confirm DPA counter measures is working fine
 *
 * @param 	AesInstance	Pointer to the XSecure_Aes instance
 *
 * @return
 *	-	XST_SUCCESS - When KAT Pass
 *	-	XSECURE_AESKAT_INVALID_PARAM - On invalid argument
 *	-	XSECURE_AESDPACM_KAT_CHECK1_FAILED_ERROR - Error when AESDPACM data
 *						not matched with expected data
 *	-	XSECURE_AESDPACM_KAT_CHECK2_FAILED_ERROR - Error when AESDPACM data
 *						not matched with expected data
 *	-	XSECURE_AESDPACM_KAT_CHECK3_FAILED_ERROR - Error when AESDPACM data
 *						not matched with expected data
 *	-	XSECURE_AESDPACM_KAT_CHECK4_FAILED_ERROR - Error when AESDPACM data
 *						not matched with expected data
 *	-	XSECURE_AESDPACM_KAT_CHECK5_FAILED_ERROR - Error when AESDPACM data
 *						not matched with expected data
 *
 *****************************************************************************/
int XSecure_AesDecryptCmKat(const XSecure_Aes *AesInstance)
{
	volatile int Status = XST_FAILURE;
	volatile int SStatus = XST_FAILURE;

	u32 Output0[XSECURE_KAT_OPER_DATA_SIZE_IN_WORDS];
	u32 Output1[XSECURE_KAT_OPER_DATA_SIZE_IN_WORDS];

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
		goto END_CLR;
	}

	Status = XST_FAILURE;

	Status = XSecure_AesDpaCmDecryptKat(AesInstance, Key1, Data1, Output1);
	if (Status != XST_SUCCESS) {
		goto END_CLR;
	}

	Status = XST_FAILURE;
	Status = XSecure_AesDecCmChecks(RM0, RM1, Mm0, Mm1);
	if (Status != XST_SUCCESS) {
		Status = (int)XSECURE_AESDPACM_KAT_CHECK1_FAILED_ERROR;
		goto END_CLR;
	}

	Status = XST_FAILURE;
	Status = XSecure_AesDecCmChecks(RM1, RM0, Mm0, Mm1);
	if (Status != XST_SUCCESS) {
		Status = (int)XSECURE_AESDPACM_KAT_CHECK2_FAILED_ERROR;
		goto END_CLR;
	}

	Status = XST_FAILURE;
	Status = XSecure_AesDecCmChecks(Mm0, RM0, RM1, Mm1);
	if (Status != XST_SUCCESS) {
		Status = (int)XSECURE_AESDPACM_KAT_CHECK3_FAILED_ERROR;
		goto END_CLR;
	}

	Status = XST_FAILURE;
	Status = XSecure_AesDecCmChecks(Mm1, RM0, RM1, Mm0);
	if (Status != XST_SUCCESS) {
		Status = (int)XSECURE_AESDPACM_KAT_CHECK4_FAILED_ERROR;
		goto END_CLR;
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
		goto END_CLR;
	}

	Status = XST_SUCCESS;

END_CLR:
	SStatus = XSecure_AesKeyZero(AesInstance, XSECURE_AES_USER_KEY_7);
	if((Status == XST_SUCCESS) && (Status == XST_SUCCESS)) {
		Status = SStatus;
	}

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function performs checks for AES DPA CM KAT ouptut.
 *
 * @param 	P is the pointer to the data array of size 4 words.
 * @param 	Q is the pointer to the data array of size 4 words.
 * @param 	R is the pointer to the data array of size 4 words.
 * @param 	S is the pointer to the data array of size 4 words.
 *
 * @return
 *	- XST_SUCCESS - When check is passed
 *  - XST_FAILURE - when check is failed
 *****************************************************************************/
static int XSecure_AesDecCmChecks(const u32 *P, const u32 *Q, const u32 *R,
	const u32 *S)
{
	volatile int Status = XST_FAILURE;

	if (((P[0U] == 0U) && (P[1U] == 0U) && (P[2U] == 0U) &&
				(P[3U] == 0U)) ||
			((P[0U] == Q[0U]) && (P[1U] == Q[1U]) &&
				(P[2U] == Q[2U]) && (P[3U] == Q[3U])) ||
			((P[0U] == R[0U]) && (P[1U] == R[1U]) &&
				(P[2U] == R[2U]) && (P[3U] == R[3U])) ||
			((P[0U] == S[0U]) && (P[1U] == S[1U]) &&
				(P[2U] == S[2U]) && (P[3U] == S[3U]))) {
		Status = XST_FAILURE;
	}
	else {
		Status = XST_SUCCESS;
	}

	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function performs decryption known answer test(KAT) on AES engine
 *
 * @param	AesInstance	Pointer to the XSecure_Aes instance
 *
 * @return
 *	-	XST_SUCCESS - When KAT Pass
 *	-	XSECURE_AESKAT_INVALID_PARAM - Invalid Argument
 *	-	XSECURE_AES_KAT_WRITE_KEY_FAILED_ERROR - Error when AES key write fails
 *	-	XSECURE_AES_KAT_DECRYPT_INIT_FAILED_ERROR - Error when AES decrypt init fails
 *	-	XSECURE_AES_KAT_GCM_TAG_MISMATCH_ERROR - Error when GCM tag not matched
 *			with user provided tag
 *	-	XSECURE_AES_KAT_DATA_MISMATCH_ERROR - Error when AES data not matched with
 *			expected data
 *
 *****************************************************************************/
int XSecure_AesDecryptKat(XSecure_Aes *AesInstance)
{
	volatile int Status = XST_FAILURE;
	volatile int SStatus = XST_FAILURE;
	volatile u32 Index;
	u32 *AesExpPt = (u32*)XSecure_GetKatMessage();
	u32 DstVal[XSECURE_KAT_MSG_LEN_IN_WORDS];

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
			XSECURE_AES_KEY_SIZE_256, (UINTPTR)AesKey);
	if (Status != XST_SUCCESS) {
		Status = (int)XSECURE_AES_KAT_WRITE_KEY_FAILED_ERROR;
		goto END_CLR;
	}

	Status = XST_FAILURE;
	Status = XSecure_AesDecryptInit(AesInstance, XSECURE_AES_USER_KEY_7,
			XSECURE_AES_KEY_SIZE_256, (UINTPTR)AesIv);
	if (Status != XST_SUCCESS) {
		Status = (int)XSECURE_AES_KAT_DECRYPT_INIT_FAILED_ERROR;
		goto END_CLR;
	}

	Status = XST_FAILURE;
	Status = XSecure_AesUpdateAad(AesInstance, (UINTPTR)AesAadData,
			XSECURE_KAT_AAD_SIZE_IN_BYTES);
	if (Status != XST_SUCCESS) {
		Status = (int)XSECURE_AES_KAT_UPDATE_AAD_FAILED_ERROR;
		goto END;
	}

	Status = XSecure_AesDecryptUpdate(AesInstance, (UINTPTR)AesCt,
			(UINTPTR)DstVal, XSECURE_KAT_MSG_LEN_IN_BYTES, TRUE);
	if (Status != XST_SUCCESS) {
		Status = (int)XSECURE_AES_KAT_DECRYPT_UPDATE_FAILED_ERROR;
		goto END_CLR;
	}

	Status = XST_FAILURE;
	Status =  XSecure_AesDecryptFinal(AesInstance, (UINTPTR)AesGcmTag);
	if (Status != XST_SUCCESS) {
		Status = (int)XSECURE_AES_KAT_GCM_TAG_MISMATCH_ERROR;
		goto END_CLR;
	}

	/* Initialized to error */
	Status = (int)XSECURE_AES_KAT_DATA_MISMATCH_ERROR;
	for (Index = 0U; Index < XSECURE_KAT_MSG_LEN_IN_WORDS; Index++) {
		if (DstVal[Index] != AesExpPt[Index]) {
			/* Comparison failure of decrypted data */
			Status = (int)XSECURE_AES_KAT_DATA_MISMATCH_ERROR;
			goto END_CLR;
		}
	}

	if (Index == XSECURE_KAT_MSG_LEN_IN_WORDS) {
		Status = XST_SUCCESS;
	}

END_CLR:
	SStatus = XSecure_AesKeyZero(AesInstance, XSECURE_AES_USER_KEY_7);
	if((Status == XST_SUCCESS) && (Status == XST_SUCCESS)) {
		Status = SStatus;
	}
	SStatus = Xil_SMemSet(DstVal, XSECURE_KAT_MSG_LEN_IN_BYTES, 0U,
				XSECURE_KAT_MSG_LEN_IN_BYTES);
	if ((Status == XST_SUCCESS) && (Status == XST_SUCCESS)) {
		Status = SStatus;
	}

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function performs encryption known answer test(KAT) on AES engine
 *
 * @param	AesInstance	Pointer to the XSecure_Aes instance
 *
 * @return
 *	-	XST_SUCCESS - When KAT Pass
 *	-	XSECURE_AESKAT_INVALID_PARAM - Invalid Argument
 *	-	XSECURE_AES_KAT_WRITE_KEY_FAILED_ERROR - Error when AES key write fails
 *	-	XSECURE_AES_KAT_ENCRYPT_INIT_FAILED_ERROR - Error when AES encrypt init fails
 *	-	XSECURE_AES_KAT_ENCRYPT_UPDATE_FAILED_ERROR - Error when AES encrypt update fails
 *	-	XSECURE_AES_KAT_ENCRYPT_FINAL_FAILED_ERROR - Error when AES encrypt final fails
 *	-	XSECURE_AES_KAT_GCM_TAG_MISMATCH_ERROR - Error when GCM tag not matched
 *			with user provided tag
 *	-	XSECURE_AES_KAT_DATA_MISMATCH_ERROR - Error when AES data not matched with
 *			expected data
 *
 *****************************************************************************/
int XSecure_AesEncryptKat(XSecure_Aes *AesInstance)
{
	volatile int Status = XST_FAILURE;
	volatile int SStatus = XST_FAILURE;
	volatile u32 Index;
	u8 *AesPt = (u8*)XSecure_GetKatMessage();
	u32 *AesExpCt = (u32*)&AesCt[0U];
	u8 GcmTag[XSECURE_SECURE_GCM_TAG_SIZE];
	u32 DstVal[XSECURE_KAT_MSG_LEN_IN_WORDS] ;

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
			XSECURE_AES_KEY_SIZE_256, (UINTPTR)AesKey);
	if (Status != XST_SUCCESS) {
		Status = (int)XSECURE_AES_KAT_WRITE_KEY_FAILED_ERROR;
		goto END_CLR;
	}

	Status = XST_FAILURE;
	Status = XSecure_AesEncryptInit(AesInstance, XSECURE_AES_USER_KEY_7,
			XSECURE_AES_KEY_SIZE_256, (UINTPTR)AesIv);
	if (Status != XST_SUCCESS) {
		Status = (int)XSECURE_AES_KAT_ENCRYPT_INIT_FAILED_ERROR;
		goto END_CLR;
	}

	Status = XST_FAILURE;
	Status = XSecure_AesUpdateAad(AesInstance, (UINTPTR)AesAadData,
			XSECURE_KAT_AAD_SIZE_IN_BYTES);
	if (Status != XST_SUCCESS) {
		Status = (int)XSECURE_AES_KAT_UPDATE_AAD_FAILED_ERROR;
		goto END_CLR;
	}

	Status = XSecure_AesEncryptUpdate(AesInstance, (UINTPTR)AesPt,
			(UINTPTR)DstVal, XSECURE_KAT_MSG_LEN_IN_BYTES, TRUE);
	if (Status != XST_SUCCESS) {
		Status = (int)XSECURE_AES_KAT_ENCRYPT_UPDATE_FAILED_ERROR;
		goto END_CLR;
	}

	Status = XST_FAILURE;
	Status =  XSecure_AesEncryptFinal(AesInstance, (UINTPTR)GcmTag);
	if (Status != XST_SUCCESS) {
		Status = (int)XSECURE_AES_KAT_ENCRYPT_FINAL_FAILED_ERROR;
		goto END_CLR;
	}

	/* Initialized to error */
	Status = (int)XSECURE_AES_KAT_DATA_MISMATCH_ERROR;
	for (Index = 0U; Index < XSECURE_KAT_MSG_LEN_IN_WORDS; Index++) {
		if (DstVal[Index] != AesExpCt[Index]) {
			/* Comparison failure of decrypted data */
			Status = (int)XSECURE_AES_KAT_DATA_MISMATCH_ERROR;
			goto END_CLR;
		}
	}

	if (Index == XSECURE_KAT_MSG_LEN_IN_WORDS) {
		Status = (int)XSECURE_KAT_GCM_TAG_MISMATCH_ERROR;
		Status = Xil_SMemCmp_CT(GcmTag, sizeof(GcmTag), AesGcmTag, XSECURE_SECURE_GCM_TAG_SIZE,
			XSECURE_SECURE_GCM_TAG_SIZE);
		if (Status != XST_SUCCESS) {
			Status = (int)XSECURE_KAT_GCM_TAG_MISMATCH_ERROR;
		}
	}

END_CLR:
	SStatus = XSecure_AesKeyZero(AesInstance, XSECURE_AES_USER_KEY_7);
	if((Status == XST_SUCCESS) && (Status == XST_SUCCESS)) {
		Status = SStatus;
	}

	SStatus = Xil_SMemSet(DstVal, XSECURE_KAT_MSG_LEN_IN_BYTES, 0U,
				XSECURE_KAT_MSG_LEN_IN_BYTES);
	SStatus |= Xil_SMemSet(GcmTag, sizeof(GcmTag), 0U, sizeof(GcmTag));
	if ((Status == XST_SUCCESS) && (Status == XST_SUCCESS)) {
		Status = SStatus;
	}

END:
	return Status;
}

/*****************************************************************************/
/**
 *
 * @brief	This function performs known answer test(KAT) on SHA crypto engine
 *
 * @param	SecureSha3 Pointer to the XSecure_Sha3 instance
 *
 * @return
 * 	-	XST_SUCCESS - When KAT Pass
 *	-	XSECURE_SHA3_INVALID_PARAM - On invalid argument
 *	-	XSECURE_SHA3_LAST_UPDATE_ERROR - Error when SHA3 last update fails
 *	-	XSECURE_SHA3_KAT_FAILED_ERROR - Error when SHA3 hash not matched with
 *					expected hash
 *	-	XSECURE_SHA3_PMC_DMA_UPDATE_ERROR - Error when DMA driver fails to update
 *					the data to SHA3
 *	-	XSECURE_SHA3_FINISH_ERROR - Error when SHA3 finish fails
 *
 ******************************************************************************/
int XSecure_Sha3Kat(XSecure_Sha3 *SecureSha3)
{
	volatile int Status = (int)XSECURE_SHA3_KAT_FAILED_ERROR;
	volatile int SStatus = (int)XSECURE_SHA3_KAT_FAILED_ERROR;
	volatile u32 Index;
	XSecure_Sha3Hash OutVal;

	if (SecureSha3 ==  NULL) {
		Status = (int)XSECURE_SHA3_INVALID_PARAM;
		goto END;
	}

	if (SecureSha3->Sha3State == XSECURE_SHA3_ENGINE_STARTED) {
		Status = (int)XSECURE_SHA3_KAT_BUSY;
		goto END;
	}

	Status = XSecure_Sha3Start(SecureSha3);
	if (Status != XST_SUCCESS) {
		goto END_RST;
	}

	Status = (int)XSECURE_SHA3_KAT_FAILED_ERROR;

	Status = XSecure_Sha3Update(SecureSha3, (UINTPTR)KatMessage,
			XSECURE_KAT_MSG_LEN_IN_BYTES);
	if (Status != XST_SUCCESS) {
		Status = (int)XSECURE_SHA3_PMC_DMA_UPDATE_ERROR;
		goto END_RST;
	}

	Status = (int)XSECURE_SHA3_KAT_FAILED_ERROR;

	Status = XSecure_Sha3Finish(SecureSha3, &OutVal);
	if (Status != XST_SUCCESS) {
		Status = (int)XSECURE_SHA3_FINISH_ERROR;
		goto END_RST;
	}

	Status = (int)XSECURE_SHA3_KAT_FAILED_ERROR;
	for(Index = 0U; Index < XSECURE_HASH_SIZE_IN_BYTES; Index++) {
		if (OutVal.Hash[Index] != ExpSha3Hash[Index]) {
			Status = (int)XSECURE_SHA3_KAT_FAILED_ERROR;
			goto END_RST;
		}
	}

	if (Index == XSECURE_HASH_SIZE_IN_BYTES) {
		Status = XST_SUCCESS;
	}

END_RST:
	SStatus = Xil_SMemSet(&OutVal.Hash[0U], XSECURE_HASH_SIZE_IN_BYTES, 0U,
				XSECURE_HASH_SIZE_IN_BYTES);
	if ((Status == XST_SUCCESS) && (Status == XST_SUCCESS)) {
		Status = SStatus;
	}
	XSecure_SetReset(SecureSha3->BaseAddress,
			XSECURE_SHA3_RESET_OFFSET);

END:
	return Status;
}

#ifndef PLM_RSA_EXCLUDE
/*****************************************************************************/
/**
 * @brief	This function performs KAT on RSA core
 *
 * @return
 *	-	XST_SUCCESS - On success
 *	-	XSECURE_RSA_KAT_ENCRYPT_FAILED_ERROR - When RSA KAT fails
 *	-	XSECURE_RSA_KAT_ENCRYPT_DATA_MISMATCH_ERROR - Error when RSA data not
 *							matched with expected data
 *
 *****************************************************************************/
int XSecure_RsaPublicEncryptKat(void)
{
	volatile int Status = XST_FAILURE;
	volatile int SStatus = XST_FAILURE;
	volatile u32 Index;
	XSecure_Rsa XSecureRsaInstance;
	u32 RsaOutput[XSECURE_RSA_4096_SIZE_WORDS];
	u32 PubExp = XSECURE_KAT_RSA_PUB_EXP;

	Status = XSecure_RsaInitialize(&XSecureRsaInstance, (u8 *)RsaModulus,
		(u8 *)RsaModExt, (u8 *)&PubExp);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = XST_FAILURE;
	Status = XSecure_RsaPublicEncrypt(&XSecureRsaInstance, (u8 *)RsaData,
		XSECURE_RSA_4096_KEY_SIZE, (u8 *)RsaOutput);
	if (Status != XST_SUCCESS) {
		Status = (int)XSECURE_RSA_KAT_ENCRYPT_FAILED_ERROR;
		goto END_CLR;
	}

	/* Initialized to error */
	Status = (int)XSECURE_RSA_KAT_ENCRYPT_DATA_MISMATCH_ERROR;
	for (Index = 0U; Index < XSECURE_RSA_4096_SIZE_WORDS; Index++) {
		if (RsaOutput[Index] != RsaExpCtData[Index]) {
			Status = (int)XSECURE_RSA_KAT_ENCRYPT_DATA_MISMATCH_ERROR;
			goto END_CLR;
		}
	}
	if (Index == XSECURE_RSA_4096_SIZE_WORDS) {
		Status = XST_SUCCESS;
	}

END_CLR:
	SStatus = Xil_SMemSet(RsaOutput, XSECURE_RSA_4096_KEY_SIZE, 0U,
				XSECURE_RSA_4096_KEY_SIZE);
	if ((Status == XST_SUCCESS) && (Status == XST_SUCCESS)) {
		Status = SStatus;
	}

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function performs private decrypt KAT on RSA core
 *
 * @return
 *	-	XST_SUCCESS - On success
 *	-	XSECURE_RSA_KAT_DECRYPT_FAILED_ERROR - When RSA KAT fails
 *	-	XSECURE_RSA_KAT_DECRYPT_DATA_MISMATCH_ERROR - Error when RSA data not
 *							matched with expected data
 *
 *****************************************************************************/
int XSecure_RsaPrivateDecryptKat(void)
{
	volatile int Status = XST_FAILURE;
	volatile int SStatus = XST_FAILURE;
	volatile u32 Index;
	XSecure_Rsa XSecureRsaInstance;
	u32 RsaOutput[XSECURE_RSA_4096_SIZE_WORDS];
	static const u32 RsaPrivateExp[XSECURE_RSA_4096_SIZE_WORDS] = {
		0x8CA6428BU, 0x91F57E3EU, 0xA8FFBE48U, 0x19589538U,
		0x402EB637U, 0x25D3A773U, 0x9F4828CBU, 0x0E8B20EBU,
		0x378DF618U, 0x783C29E6U, 0x67FF85CBU, 0xAA09C87BU,
		0xC4892ACBU, 0x21EAAC2EU, 0xDB3CA12BU, 0x7FC727D8U,
		0xAD34805AU, 0xD0D81111U, 0xE8BCF68BU, 0x4B6A2D3AU,
		0xD5AD3D62U, 0xF5919C16U, 0x676DD2D3U, 0xA79D9227U,
		0x959B5F2AU, 0xB743B1F6U, 0xA81E91F1U, 0x9D891A40U,
		0x763C19C6U, 0x935C2F98U, 0x7817316AU, 0xAAB43747U,
		0xFF69F4D7U, 0x240E8D38U, 0x39197149U, 0xA84493B3U,
		0x5463AD6EU, 0x3A2EBE99U, 0x19E81EA6U, 0x8CA63D5BU,
		0x30855D15U, 0x150AF6E4U, 0xDA3C1E3EU, 0xC988A27EU,
		0x08D51079U, 0x5A49A2B8U, 0xF5041D81U, 0x7F6B3A9EU,
		0x76AADFCEU, 0x4B7A44BEU, 0x1E706FF4U, 0xAFFE251AU,
		0xDF401219U, 0x6E7DB1C3U, 0xA28D25D9U, 0x440DFC77U,
		0x11123392U, 0xADF2BA02U, 0xAA5C1E55U, 0x53E59D45U,
		0xC6F6185EU, 0xF243936EU, 0xC02DDCB9U, 0x5B1F0BA7U,
		0x70ACD241U, 0x3EA56098U, 0xF9FB2069U, 0x35343BD5U,
		0x39791DEFU, 0x6666FE76U, 0x8E56186AU, 0xB47F7804U,
		0xE892DD81U, 0x4B06A589U, 0x28AB09A9U, 0xB90FC556U,
		0xEAD3DE59U, 0x07D87638U, 0xAC358D80U, 0x63B012F8U,
		0x8E06CBD8U, 0x0C98228DU, 0xFEF79626U, 0x6D5262ADU,
		0xEE6F78BFU, 0x5167EF43U, 0x3B1BBE38U, 0xA61DEED0U,
		0xD597BC3EU, 0x038E1AF5U, 0x8D13E1C5U, 0x14299D55U,
		0xCC601AE4U, 0x5A4160E5U, 0xF825C074U, 0x37F875D2U,
		0x2EF926CBU, 0x6ADD10F6U, 0xD838ED7CU, 0x84BFFC50U,
		0x4EDE296CU, 0x581FE01CU, 0x211EC4BFU, 0x4A10CA7FU,
		0x7B07189FU, 0x8B88265FU, 0x98917236U, 0xBF30006AU,
		0x2AF8A88CU, 0x3072C6D8U, 0x5B180416U, 0x542463F6U,
		0xF0FF0F87U, 0x8C8D2B4BU, 0x829D1BECU, 0x04C0CB50U,
		0x4DA826F7U, 0x885D8659U, 0x2C23928CU, 0x5B18123EU,
		0xCCD8E1CCU, 0x2561AC54U, 0x438282EFU, 0xDDAF542EU,
		0x71E6ACD2U, 0xFE8AA8C9U, 0x2613625BU, 0x0134DECBU
	};

	Status = XSecure_RsaInitialize(&XSecureRsaInstance, (u8 *)RsaModulus,
		(u8 *)RsaModExt, (u8 *)RsaPrivateExp);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = XST_FAILURE;
	Status = XSecure_RsaPrivateDecrypt(&XSecureRsaInstance, (u8 *)RsaExpCtData,
		XSECURE_RSA_4096_KEY_SIZE, (u8 *)RsaOutput);
	if (Status != XST_SUCCESS) {
		Status = (int)XSECURE_RSA_KAT_DECRYPT_FAILED_ERROR;
		goto END_CLR;
	}

	/* Initialized to error */
	Status = (int)XSECURE_RSA_KAT_ENCRYPT_DATA_MISMATCH_ERROR;
	for (Index = 0U; Index < XSECURE_RSA_4096_SIZE_WORDS; Index++) {
		if (RsaOutput[Index] != RsaData[Index]) {
			Status = (int)XSECURE_RSA_KAT_DECRYPT_DATA_MISMATCH_ERROR;
			goto END_CLR;
		}
	}
	if (Index == XSECURE_RSA_4096_SIZE_WORDS) {
		Status = XST_SUCCESS;
	}

END_CLR:
	SStatus = Xil_SMemSet(RsaOutput, XSECURE_RSA_4096_KEY_SIZE, 0U,
				XSECURE_RSA_4096_KEY_SIZE);
	SStatus |= Xil_SMemSet(&XSecureRsaInstance, sizeof(XSecure_Rsa), 0U, sizeof(XSecure_Rsa));
	if ((Status == XST_SUCCESS) && (Status == XST_SUCCESS)) {
		Status = SStatus;
	}

END:
	return Status;
}
#endif

#ifndef PLM_ECDSA_EXCLUDE
/*****************************************************************************/
/**
 * @brief	This function performs ECC sign verify known answer test(KAT) on ECC core
 *
 * @param	CrvClass - Type of ECC curve class either prime or binary curve
 *
 * @return
 *	-	XST_SUCCESS - On success
 *	-	XSECURE_ELLIPTIC_KAT_KEY_NOTVALID_ERROR - When elliptic key is not valid
 *	-	Errorcode 	- when KAT fails
 *
 *****************************************************************************/
int XSecure_EllipticVerifySignKat(XSecure_EllipticCrvClass CrvClass) {
	volatile int Status = XST_FAILURE;
	XSecure_EllipticKey *PubKey = XSecure_GetKatEccPublicKey(CrvClass);
	XSecure_EllipticSign *ExpSign = XSecure_GetKatEccExpSign(CrvClass);

	if (PubKey->Qx == NULL || PubKey->Qy == NULL || ExpSign->SignR == NULL
		|| ExpSign->SignS == NULL) {
		Status = (int)XSECURE_ELLIPTIC_NON_SUPPORTED_CRV;
		goto END;
	}

	Status = XSecure_EllipticValidateKey(XSECURE_ECC_NIST_P384, PubKey);
	if (Status != XST_SUCCESS) {
		Status = (int)XSECURE_ELLIPTIC_KAT_KEY_NOTVALID_ERROR;
		goto END;
	}

	Status = XST_FAILURE;
	Status = XSecure_EllipticVerifySign(XSECURE_ECC_NIST_P384, (u8*)&ExpEccSha3Hash[0U],
				XSECURE_HASH_SIZE_IN_BYTES, PubKey, ExpSign);

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function performs ECC sign generate known answer test(KAT) on ECC core
 *
 * @param	CrvClass - Type of ECC curve class either prime or binary class
 *
 * @return
 *	-	XST_SUCCESS - when KAT passes
 *	-	Errorcode 	- when KAT fails
 *
 *****************************************************************************/
int XSecure_EllipticSignGenerateKat(XSecure_EllipticCrvClass CrvClass) {
	volatile int Status = XST_FAILURE;
	volatile int SStatus = XST_FAILURE;
	u8 Sign[XSECURE_ECC_P384_SIZE_IN_BYTES +
				XSECURE_ECC_P384_SIZE_IN_BYTES];
	XSecure_EllipticSign GeneratedSign;
	XSecure_EllipticSign *ExpSign = XSecure_GetKatEccExpSign(CrvClass);
	u8 *D = XSecure_GetKatEccPrivateKey(CrvClass);
	u8 *K = XSecure_GetKatEccEphimeralKey(XSECURE_ECC_NIST_P384);
	u32 Size = XSECURE_ECC_P384_SIZE_IN_BYTES;

	if ((ExpSign->SignR == NULL) || (ExpSign->SignS == NULL)
		|| (D == NULL) || (K == NULL)) {
		Status = (int)XSECURE_ELLIPTIC_NON_SUPPORTED_CRV;
		goto END;
	}

	GeneratedSign.SignR = &Sign[0U];
	GeneratedSign.SignS = &Sign[Size];

	Status = XSecure_EllipticGenerateSignature(XSECURE_ECC_NIST_P384, (u8*)&ExpEccSha3Hash[0U],
				Size, D, K, &GeneratedSign);
	if (Status != XST_SUCCESS) {
		goto END_CLR;
	}

	Status = XST_FAILURE;
	Status = Xil_SMemCmp_CT(GeneratedSign.SignR, (Size * 2U), ExpSign->SignR, (Size * 2U),
				(Size * 2U));
	if (Status != XST_SUCCESS) {
		goto END_CLR;
	}

END_CLR:
	SStatus = Xil_SMemSet(Sign, sizeof(Sign), 0U, sizeof(Sign));
	if ((Status == XST_SUCCESS) && (Status == XST_SUCCESS)) {
		Status = SStatus;
	}

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function performs ECC pairwise consistency test on ECC core
 *
 * @param	Curvetype - Type of ECC curve used for authentication
 * @param	DAddr - Address of ECC private key
 * @param	PubKeyAddr - Address of ECC public key
 *
 * @return
 *	-	XST_SUCCESS - when KAT passes
 *	-	Errorcode - when KAT fails
 *
 *****************************************************************************/
int XSecure_EllipticPwct(XSecure_EllipticCrvTyp Curvetype, u64 DAddr, XSecure_EllipticKeyAddr *PubKeyAddr) {
	volatile int Status = XST_FAILURE;
	volatile int SStatus = XST_FAILURE;
	u8 Sign[XSECURE_ECC_P521_SIZE_IN_BYTES + XSECURE_ECC_P521_SIZE_IN_BYTES];
	XSecure_EllipticSignAddr GeneratedSignAddr;
	XSecure_EllipticHashData HashInfo;
	u8 *K = XSecure_GetKatEccEphimeralKey(Curvetype);
	u8 Size = 0U;

	if (Curvetype == XSECURE_ECC_NIST_P384) {
		Size = XSECURE_ECC_P384_SIZE_IN_BYTES;
	}
	else if (Curvetype == XSECURE_ECC_NIST_P521) {
		Size = XSECURE_ECC_P521_SIZE_IN_BYTES;
	}
	else {
		Status = (int)XSECURE_ELLIPTIC_NON_SUPPORTED_CRV;
		goto END;
	}

	HashInfo.Addr = (u64)(UINTPTR)&ExpEccSha3Hash[0U];
	HashInfo.Len = Size;
	GeneratedSignAddr.SignR = (u64)(UINTPTR)&Sign[0U];
	GeneratedSignAddr.SignS = (u64)(UINTPTR)&Sign[Size];

	Status = XSecure_EllipticGenerateSignature_64Bit(Curvetype, &HashInfo,
				DAddr, (u64)(UINTPTR)K, &GeneratedSignAddr);
	if (Status != XST_SUCCESS) {
		goto END_CLR;
	}

	Status = XST_FAILURE;
	Status = XSecure_EllipticVerifySign_64Bit(Curvetype, &HashInfo, PubKeyAddr, &GeneratedSignAddr);

END_CLR:
	SStatus = Xil_SMemSet(Sign, sizeof(Sign), 0U, sizeof(Sign));
	if ((Status == XST_SUCCESS) && (Status == XST_SUCCESS)) {
		Status = SStatus;
	}

END:
	return Status;
}
#endif
