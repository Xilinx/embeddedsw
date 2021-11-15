/******************************************************************************
* Copyright (c) 2013 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file
* 			xilskey_epl.c
* @note
*
*  			Contains the function definitions for the PL eFUSE functionality.
*
* MODIFICATION HISTORY:
*
* Ver   Who  	Date     Changes
* ----- ---- 	-------- --------------------------------------------------------
* 1.00a rpoolla 04/26/13 First release
* 1.02a hk      10/28/13 Added API's to read status bits and key.PR# 735957
* 2.00  hk      22/01/14 Corrected PL voltage checks to VCCINT and VCCAUX.
*                        CR#768077
* 2.1   kvn     04/01/15 Fixed warnings. CR#716453.
* 3.00  vns     31/07/15 Added efuse functionality for Ultrascale.
* 4.0   vns     10/01/15 provided conditional compilation to support
*                        ZynqMp platform also.
*                        Corrected error code names of Ultrascale efuse PL
* 5.0   vns     07/01/16 Verificaion of programming bits is done by
*                        performing all Margin reads.
*                        Added conditions for programming control and
*                        secure bits.
* 6.00  vns     29/06/16 Added Margin 2 read verification after programming
*                        every Zynq's eFUSE PL bit CR #953052.
*               07/07/16 Modified XilSKey_EfusePl_ProgramBit_Ultra such that
*                        it returns error code when JtagWrite_Ultrascale fails
*                        programming eFUSE bit. Error occurs only when Hardware
*                        Module has encountered timeout.
*               26/07/16 Added 128 bit user key programming and reading.
*                        Provided single bit programming feature for 32 and
*                        128 bit user keys for eFUSE Ultrascale.
* 6.4   vns     02/27/18 Added support for programming secure bit 6 -
*                        enable obfuscation feature for eFUSE AES key
* 6.6   vns     06/06/18 Added doxygen tags
* 6.7   psl     03/20/19 Added eFuse key write support for SSIT devices.
*       arc     04/04/19 Fixed CPP warnings.
*       psl     04/15/19 Added JtagServerInit function.
* 6.8   psl     05/21/19 Added else case to clear UserFuses_TobePrgrmd
*       psl     08/23/19 Added Debug define to avoid writing of eFuse.
*       vns     08/29/19 Initialized Status variables
* 6.9   vns     03/18/20 Fixed Armcc compilation errors
* 7.2   am      07/13/21 Fixed doxygen warnings
* 7.3   har     11/15/21 Removed local variable ErrorCode in
*                        XilSKey_EfusePl_GetRowData_Ultra()
*
* </pre>
*
****************************************************************************/
/***************************** Include Files *********************************/
#include "xparameters.h"
#include "xil_types.h"
#include "xilskey_utils.h"
#include "xilskey_epl.h"
/************************** Constant Definitions *****************************/
#define XSK_EFUSEPL_ARRAY_FUSE_CNTRL_ROW		(0) /**< Fuse Ctrl Row*/
#define XSK_EFUSEPL_ARRAY_FUSE_AES_KEY_SIZE		(256) /**< AES Key size*/
#define XSK_EFUSEPL_ARRAY_FUSE_USER_KEY_SIZE	(32) /**< 32 bit
							* User key size*/
#define XSK_EFUSEPL_ARRAY_FUSE_128BIT_USER_SIZE	(128) /**< 128 bit
							*  User key size*/
#define XSK_EFUSEPL_ARRAY_MAX_ROW				(32) /**< PLeFUSE Max Rows*/
#define XSK_EFUSEPL_ARRAY_MAX_COL				(32) /**< PLeFUSE Max Columns*/
#define XSK_EFUSEPL_ARRAY_AES_DATA_ROW_START	(20) /**< AES Data Start Row*/
#define XSK_EFUSEPL_ARRAY_AES_DATA_ROW_END		(30) /**< AES Data End Row*/
#define XSK_EFUSEPL_ARRAY_USER_DATA_START_ROW	(31) /**< User Data Start Row*/
#define XSK_EFUSEPL_ARRAY_AES_DATA_BITS_IN_30th_ROW	(16) /**< AES Data bits
															* count in 30th Row
															*/
#define XSK_EFUSEPL_ARRAY_MAX_COL_ULTRA_PLUS	(16) /**< Ultrascale plus
						       *  max bits in a row */
#define XSK_EFUSEPL_ARRAY_USER_DATA_BITS_IN_30th_ROW (8) /**< User Data bits
															* count in 30th Row
															*/
#define XSK_EFUSEPL_ARRAY_MAX_PAYLAOD_BITS_IN_A_ROW	(24)/**< Max Pay load
															* in Row
															*/
#define XSK_EFUSEPL_ARRAY_MAX_ECC_BITS_IN_A_ROW		(6)	/**< Max ECC bits
															 * in a Row
															 */
#define XSK_EFUSEPL_ARRAY_ECC_START_BIT_IN_A_ROW	(24)/**< ECC Start Bit
															 * position in
															 * a Row
															 */
#define XSK_EFUSEPL_ARRAY_ECC_END_BIT_IN_A_ROW		(29)/**< ECC End Bit
															 * position in
															 * a Row
															 */
#define XSK_EFUSEPL_ARRAY_FUSE_CNTRL_MAX_BITS		(11)/**< Fuse Control max
															 * bits
															 */
#define XSK_EFUSEPL_ARRAY_FUSE_CNTRL_REDUNDENT_INDEX (14)/**< Redundant bit
															 * Index
															 */
#define XSK_EFUSEPL_ARRAY_FUSE_CNTRL_START_BIT	(0)	 /**< Fuse Control
															* Start bit
															*/
#define XSK_EFUSEPL_ARRAY_FUSE_CNTRL_END_BIT	(10) /**< Fuse Control
															* Start bit
															*/
#define XSK_EFUSEPL_ARRAY_UNSUPPORTED_BIT6		(6)	 /**< Unsupported bit*/
#define XSK_EFUSEPL_ARRAY_UNSUPPORTED_BIT7		(7)	 /**< Unsupporte bit*/
#define XSK_EFUSEPL_ARRAY_FUSE_CNTRL_REDUNDENT_START_BIT	(14)/**< Redundant
															 * bit start Index
															 */
#define XSK_EFUSEPL_ARRAY_UNSUPPORTED_RED_FOR_BIT6	(20)/**< Unsupported bit*/
#define XSK_EFUSEPL_ARRAY_UNSUPPORTED_RED_FOR_BIT7	(21)/**< Unsupported bit*/
#define XSK_EFUSEPL_ARRAY_FUSE_CNTRL_REDUNDENT_END_BIT	(24)/**< Redundant bit
															 * End Index
															 */
#define XSK_EFUSEPL_MAX_REF_CLK_FREQ	60000000 /**< Max Ref Clk Frequency */
#define XSK_EFUSEPL_MIN_REF_CLK_FREQ	20000000 /**< Min Ref Clk Frequency */

/*
 * Definitions for Ultrascale
 */
#define XSK_EFUSEPL_CNTRL_ROW_ULTRA	(1)	/**< Control row of
						  * FUSE for Ultrascale
						  * series */
#define XSK_EFUSEPL_DNA_ROW_ULTRA	(7)	/**< DNA row of
						  * FUSE for Ultrascale
						  * series */
#define XSK_EFUSEPL_AES_ROW_START_ULTRA	(20)	/**< AES key start
						  * row of FUSE for
						  * Ultrascale series */
#define XSK_EFUSEPL_AES_ROW_END_ULTRA	(27)	/**< AES key end
						  * row of FUSE for
						  * Ultrascale series */
#define XSK_EFUSEPL_USER_ROW_ULTRA	(28)	/**< USER key start
						  * row of FUSE for
						  * Ultrascale series */
#define XSK_EFUSEPL_SEC_ROW_ULTRA	(10)	/**< Secure row of
						  * FUSE for Ultrascale
						  * series */
#define XSK_EFUSEPL_RSA_ROW_START_ULTRA	(12)	/**< RSA start row of
						  * FUSE for Ultrascale
						  * series */
#define XSK_EFUSEPL_RSA_ROW_END_ULTRA	(23)	/**< RSA end row of
						  * FUSE for Ultrascale
						  * series */
#define XSK_EFUSEPL_USER_128BIT_ROW_START_ULTRA	(0)/**< 128 bit USER key
						     * start row of FUSE for
						     * Ultrascale series */
#define XSK_EFUSEPL_USER_128BIT_ROW_END_ULTRA	(3)/**< 128 bit USER key
						     * end row of FUSE for
						     * Ultrascale series */
#define XSK_EFUSEPL_DNA_KEY_SIZE_ULTRA	(96)	/**< DNA key size
						 * of Ultrascale
						 * series */
#define XSK_EFUSEPL_RSA_HASH_SIZE_ULTRA	(384)	/**< RSA hash size
						  *  of Ultrascale
						  *  series */
#define XSK_EFUSEPL_SEC_MAX_BITS_ULTRA	(7)	/**< Secure row
						  *  max bits of
						  *  Ultrascale
						  *  series */
#define XSK_EFUSEPL_CNTRL_MAX_BITS_ULTRA	(17)
						/**< Fuse Control max
						  * bits of Ultrascale
						  * series */
#define XSK_EFUSEPL_MAX_BITS_IN_A_ROW_ULTRA	(32)
						/**< Max Pay load
						  * in Row */
#define XSK_EFUSEPL_END_BIT_IN_A_ROW_ULTRA	(31)
						/**< FUSE end bit in
						  *  a row of Ultrascale
						  *  series */
#define XSK_EFUSEPL_CTRL_ROW_END_BIT_ULTRA	(16)
						/**< Control
						  * row end bit
						  * of Ultrascale series */
#define XSK_EFUSEPL_SEC_ROW_END_BIT_ULTRA	(6)
						/**< Secure
						  * row end bit
						  * of Ultrascale series */
/*
 * Definitions for Ultrascale Plus
 */
#define XSK_EFUSEPL_CNTRL_ROW_START_ULTRA_PLUS	(2)	/**< Control row of
						  * FUSE for Ultrascale plus
						  * series */
#define XSK_EFUSEPL_CNTRL_ROW_END_ULTRA_PLUS	(3)	/**< Control row of
						  * FUSE for Ultrascale plus
						  * series */
#define XSK_EFUSEPL_DNA_ROW_START_ULTRA_PLUS	(0)	/**< DNA row of
						  * FUSE for Ultrascale plus
						  * series */
#define XSK_EFUSEPL_DNA_ROW_END_ULTRA_PLUS	(5)	/**< DNA row of
						  * FUSE for Ultrascale plus
						  * series */
#define XSK_EFUSEPL_AES_ROW_START_ULTRA_PLUS	(5)	/**< AES key start
						  * row of FUSE for
						  * Ultrascale plus series */
#define XSK_EFUSEPL_AES_ROW_END_ULTRA_PLUS	(20)	/**< AES key end
						  * row of FUSE for
						  * Ultrascale series */
#define XSK_EFUSEPL_USER_ROW_START_ULTRA_PLUS	(30)	/**< USER key start
						  * row of FUSE for
						  * Ultrascale plus series */
#define XSK_EFUSEPL_USER_ROW_END_ULTRA_PLUS	(31)	/**< USER key start
						  * row of FUSE for
						  * Ultrascale plus series */
#define XSK_EFUSEPL_SEC_ROW_ULTRA_PLUS	(4)	/**< Secure row of
						  * FUSE for Ultrascale
						  * series */
#define XSK_EFUSEPL_RSA_ROW_START_ULTRA_PLUS	(6)	/**< RSA start row of
						  * FUSE for Ultrascale
						  * plus series */
#define XSK_EFUSEPL_RSA_ROW_END_ULTRA_PLUS	(29)	/**< RSA end row of
						  * FUSE for Ultrascale
						  * plus series */
#define XSK_EFUSEPL_USER_128BIT_ROW_START_ULTRA_PLUS	(21)/**< 128 bit USER key
						     * start row of FUSE for
						     * Ultrascale plus series */
#define XSK_EFUSEPL_USER_128BIT_ROW_END_ULTRA_PLUS	(28)/**< 128 bit USER key
						     * end row of FUSE for
						     * Ultrascale plus series */

/**
 * Unsupported bits of Control register
 */
#define XSK_EFUSEPL_CTRL_ROW_UNSUPPORT_BIT3_ULTRA	(3)
							/** < Unsupported bit in
							  *   ctrl register */
#define XSK_EFUSEPL_CTRL_ROW_UNSUPPORT_BIT4_ULTRA	(4)
							/** < Unsupported bit in
							  *   ctrl register */
#define XSK_EFUSEPL_CTRL_ROW_UNSUPPORT_BIT_RANGE_START_ULTRA	(10)
							/** < Unsupported bit in
							  *   ctrl register */
#define XSK_EFUSEPL_CTRL_ROW_UNSUPPORT_BIT_RANGE_END_ULTRA	(14)
							/** < Unsupported bit in
							  *   ctrl register */

#define XSK_EFUSEPL_MAX_REF_CLK_FREQ	60000000 /**< Max Ref Clk Frequency */
#define XSK_EFUSEPL_MIN_REF_CLK_FREQ	20000000 /**< Min Ref Clk Frequency */

#define XSK_EFUSEPL_PAGE_0_ULTRA		(0)	/**< Page 0 for Ultrascale
							  *  series */
#define XSK_EFUSEPL_PAGE_1_ULTRA		(1)	/**< Page 1 for Ultrascale
							  *  series */
#define XSK_EFUSEPL_REDUNDANT_ULTRA		(1)	/**< Redundant read
							  *  selection for
							  *  Ultrascale series */
#define XSK_EFUSEPL_NORMAL_ULTRA		(0)	/**< Normal read
							  *  selection for
							  *  Ultrascale series */

/**************************** Type Definitions ******************************/
/**
 * Read or Write eFUSE Margin Options
 */
typedef enum {
	XSK_EFUSEPL_READ_NORMAL = 0x1,	/**< Margin 1*/
	XSK_EFUSEPL_READ_MARGIN_1 = 0x2, /**< Margin 2*/
	XSK_EFUSEPL_READ_MARGIN_2 = 0x4, /**< Margin 4*/
	XSK_EFUSEPL_READ_MARGIN_MAX = 0x7 /**< Max Margin 7*/
}XSK_EfusePl_MarginOption;
/**
 * Fuse Control Row Bit Indices
 */
typedef enum {
	XSK_EFUSEPL_CNTRL_FORCE_PCYCLE_RECONFIG = 0x01,/**< Bit1 of Fuse Ctrl Row*/
	XSK_EFUSEPL_CNTRL_DISABLE_KEY_WRITE,		   /**< Bit2 of Fuse Ctrl Row*/
	XSK_EFUSEPL_CNTRL_DISABLE_AES_KEY_READ,		   /**< Bit3 of Fuse Ctrl Row*/
	XSK_EFUSEPL_CNTRL_DISABLE_USER_KEY_READ,       /**< Bit4 of Fuse Ctrl Row*/
	XSK_EFUSEPL_CNTRL_DISABLE_FUSE_CNTRL_WRITE,    /**< Bit5 of Fuse Ctrl Row*/
	XSK_EFUSEPL_CNTRL_FORCE_USE_AES_ONLY = 0x08,   /**< Bit8 of Fuse Ctrl Row*/
	XSK_EFUSEPL_CNTRL_JTAG_CHAIN_DISABLE,          /**< Bit9 of Fuse Ctrl Row*/
	XSK_EFUSEPL_CNTRL_BBRAM_KEY_DISABLE           /**< Bit10 of Fuse Ctrl Row*/
}XSKEfusePl_FuseCntrlBits;

/**
 * Fuse Control Row Bit Indices of Ultrascale series
 */
typedef enum {
	XSK_EFUSEPL_CNTRL_DISABLE_KEY_RD_ULTRA,		/**< Bit 0 of Fuse Ctrl
							  *  row of Ultrascale */
	XSK_EFUSEPL_CNTRL_DISABLE_USER_KEY_RD_ULTRA,	/**< Bit 1 of Fuse Ctrl
							  *  row of Ultrascale */
	XSK_EFUSEPL_CNTRL_DISABLE_SEC_RD_ULTRA,		/**< Bit 2 of Fuse Ctrl
							  *  row of Ultrascale */
	XSK_EFUSEPL_CNTRL_DISABLE_CNTRL_WR_ULTRA = 5,/**< Bit 5 of Fuse Ctrl
							  *  row of Ultrascale */
	XSK_EFUSEPL_CNTRL_DISABLE_RSA_KEY_RD_ULTRA,	/**< Bit 6 of Fuse Ctrl
							  *  row of Ultrascale */
	XSK_EFUSEPL_CNTRL_DISABLE_KEY_WR_ULTRA,		/**< Bit 7 of Fuse Ctrl
							  *  row of Ultrascale */
	XSK_EFUSEPL_CNTRL_DISABLE_USER_KEY_WR_ULTRA,	/**< Bit 8 of Fuse Ctrl
							  *  row of Ultrascale */
	XSK_EFUSEPL_CNTRL_DISABLE_SEC_WR_ULTRA,		/**< Bit 8 of Fuse Ctrl
							  *  row of Ultrascale */
	XSK_EFUSEPL_CNTRL_DISABLE_RSA_KEY_WR_ULTRA = 15,/**< Bit 15 of Fuse Ctrl
							  *  row of Ultrascale */
	XSK_EFUSEPL_CNTRL_DISABLE_128BIT_USR_KEY_WR_ULTRA/**< Bit 16 of Fuse Ctrl
								  *  row of Ultrascale */
}XSKEfusePl_FuseCntrlBits_Ultra;

/**
 * Fuse Secure Row Bit Indices of Ultrascale series
 */
typedef enum {
	XSK_EFUSEPL_SEC_ALLOW_ENCRYPT_ONLY,	/**< Bit 0 of Fuse Secure
						 *  row of Ultrascale */
	XSK_EFUSEPL_SEC_FORCE_AES_ONLY_ULTRA,		/**< Bit 1 of Fuse Secure
							  *  row of Ultrascale */
	XSK_EFUSEPL_SEC_RSA_AUTH_EN_ULTRA ,	/**< Bit 2 of Fuse Secure
							  *  row of Ultrascale */
	XSK_EFUSEPL_SEC_JTAG_CHAIN_DISABLE_ULTRA,	/**< Bit 3 of Fuse Secure
							  *  row of Ultrascale */
	XSK_EFUSEPL_SEC_DISABLE_INTRNL_TEST_ACCESS_ULTRA,/**< Bit 4 of Fuse Secure
							  *  row of Ultrascale */
	XSK_EFUSEPL_SEC_DISABLE_DECRPTR_ULTRA,		/**< Bit 5 of Fuse Secure
							  *  row of Ultrascale */
	XSK_EFUSEPL_SEC_ENABLE_OBFUSCATION_ULTRA /**< Bit 6 of fuse secure row
							  * row of Ultrascale */
}XSKEfusePl_FuseSecureBits_Ultra;

/***************** Macros (Inline Functions) Definitions ********************/


/************************** Variable Definitions ****************************/
u32 ErrorCode;	/**< Global variable which holds the error key*/
static u8 AesDataInBytes[XSK_EFUSEPL_ARRAY_FUSE_AES_KEY_SIZE];
static u8 UserDataInBytes[XSK_EFUSEPL_ARRAY_FUSE_USER_KEY_SIZE];
XSKEfusePl_Fpga PlFpgaFlag;		/**< For Storing Fpga series */
#ifdef XSK_MICROBLAZE_PLATFORM
static u8 RsaDataInBytes[XSK_EFUSEPL_RSA_HASH_SIZE_ULTRA];
#endif
static u8 CtrlBitsUltra[XSK_EFUSEPL_ARRAY_MAX_ROW];
#ifdef XSK_MICROBLAZE_PLATFORM
static u8 User128BitData[XSK_EFUSEPL_ARRAY_FUSE_128BIT_USER_SIZE];
#endif
XSKEfusePl_Fpga	PlFpgaFlag;		/**< For Storing Fpga series */
extern XilSKey_JtagSlr XilSKeyJtag;
/************************** Function Prototypes *****************************/
/**
 * @name API declarations
 * @{
 */
/**< Prototype declarations for PL eFUSE interface functions */
static u8 XilSKey_EfusePl_ProgramBit(u8 Row, u8 Bit);

static u8 XilSKey_EfusePl_ProgramRow(u8 Row, u8 *RowData);

static u8 XilSKey_EfusePl_ProgramControlRegister(u8 *CtrlData);

static u8 XilSKey_EfusePl_ReadBit(u8 Row, u8 Bit, u8 MarginOption, u8 *BitData);

static u8 XilSKey_EfusePl_ReadRow(u32 Row, u8 MarginOption, u8 *RowDataBytes);

static u8 XilSKey_EfusePl_ReadControlRegister(u8 *CtrlData);

static u8 XilSKey_EfusePl_VerifyBit(u8 Row, u8 Bit);

static u8 XilSKey_EfusePl_IsVectorAllZeros(u8 *RowDataPtr);

static void XilSKey_EfusePl_CalculateEcc(u8 *RowData, u8 *ECCData);

static INLINE u8 XilSKey_EfusePl_VerifyAES_Ultrascale(u32 CrcValue);

static u32 XilSKey_EfusePl_Program_Zynq(XilSKey_EPl *InstancePtr);

#ifdef XSK_MICROBLAZE_PLATFORM
static INLINE u32 XilSKey_EfusePl_Program_Ultra(XilSKey_EPl *InstancePtr);
#endif

static INLINE u32 XilSKey_EfusePl_Program_AesKey_ultra(void);

static INLINE u32 XilSKey_EfusePl_Program_RowRange_ultra(u8 RowStart, u8 RowEnd,
				u8 *DataPrgrmg, u8 Page);

static INLINE u32 XilSKey_EfusePl_GetRowData_Ultra(u8 Row, u8 *RowData, u8 Page);

static INLINE u32 XilSKey_EfusePl_GetDataRowRange_Ultra(u8 RowStart, u8 RowEnd,
				u8 *KeyRead, u8 Page);

static INLINE u32 XilSKey_EfusePl_ReadKey_Zynq(XilSKey_EPl *InstancePtr);

#ifdef XSK_MICROBLAZE_PLATFORM
static INLINE u32 XilSKey_EfusePl_ReadKey_Ultra(XilSKey_EPl *InstancePtr);
#endif

static INLINE u32 XilSKey_EfusePl_ReadKey_Checks(XilSKey_EPl *InstancePtr);

static INLINE u32 XilSKey_EfusePl_Program_Checks(XilSKey_EPl *InstancePtr);

static INLINE u8 XilSKey_EfusePl_ProgramBit_Ultra(u8 Row, u8 Bit, u8 Redundant,
								u8 Page);

static INLINE u8 XilSKey_EfusePl_ProgramRow_Ultra(u8 Row, u8 *RowData, u8 Redundant,
								u8 Page);
static INLINE u8 XilSKey_EfusePl_ReadSecRegister(u8 *SecData);

static INLINE u8 XilSKey_EfusePl_ProgramSecRegister(u8 *SecData);

static INLINE u8 XilSKey_EfusePl_ReadBit_Ultra(u8 Row, u8 Bit, u8 MarginOption,
					u8 *BitData, u8 Redundant, u8 Page);

static INLINE u8 XilSKey_EfusePl_ReadRow_Ultra(u32 Row, u8 MarginOption,
					u8 *RowDataBytes, u8 Redundant, u8 Page);

static INLINE u8 XilSKey_EfusePl_ProgramControlReg_Ultra(u8 *CtrlData);

static INLINE u8 XilSkey_EfusePl_VerifyBit_Ultra(u8 Row, u8 Bit, u8 Redundant,
								u8 Page);
static INLINE u32 XilSkey_EfusePl_UserFuses_TobeProgrammed(u8 *UserFuses_Write,
					u8 *UserFuses_TobePrgrmd, u8 Size);
static INLINE u8 XilSKey_EfusePl_ProgramControlReg_Ultra_Plus(u8 *CtrlData);
static INLINE u8 XilSKey_EfusePl_Ultra_Check(u8 Row,
		u8 Bit, u8 Redundant, u8 Page);
static INLINE u32 XilSKey_EfusePl_ReadRowData_Ultra(u8 Row,
						u8 *RowData, u8 Page);
/** @} */

/**
 * 	JTAG Server Initialization routine
 */
extern int JtagServerInit(XilSKey_EPl *PlInstancePtr);
/**
 * 	JTAG Server Write routine
 */
extern void JtagWrite(unsigned char row, unsigned char bit);
/**
 * 	JTAG Server Read routine
 */
extern void JtagRead(unsigned char row, unsigned int * row_data, unsigned char marginOption);
extern int JtagWrite_Ultrascale(u8 Row, u8 Bit, u8 Page, u8 Redundant);
extern void JtagRead_Ultrascale(u8 Row, u32 *RowData, u8 MarginOption,
			u8 Page, u8 Redundant);
extern void JtagRead_Status_Ultrascale(u32 *Rowdata);
extern u32 JtagAES_Check_Ultrascale(u32 *Crc, u8 MarginOption);
/***************************************************************************/

/****************************************************************************/
/**
*	Initializes PL eFUSE with input data given
*
*
* @param	InstancePtr - Input data to be written to PL eFUSE
*
* @return
*
*	- XST_FAILURE - In case of failure
*	- XST_SUCCESS - In case of Success
*
*
* @note		Updates the global variable ErrorCode with error code(if any).
*
*****************************************************************************/
u32 XilSKey_EfusePl_SystemInit(XilSKey_EPl *InstancePtr)
{

	ErrorCode = XSK_EFUSEPL_ERROR_NONE;


	if(NULL == InstancePtr)	{
		return XSK_EFUSEPL_ERROR_PL_STRUCT_NULL;
	}

	if(!(InstancePtr->SystemInitDone))
	{

#ifdef XSK_ARM_PLATFORM
		u32 RefClk;
		u32 Status;
		RefClk = XilSKey_Timer_Intialise();
		/**
		 * Return error if the reference clock frequency is not in
		 * between 20 & 60MHz
		 */
		if((RefClk < XSK_EFUSEPL_MIN_REF_CLK_FREQ) ||
				(RefClk > XSK_EFUSEPL_MAX_REF_CLK_FREQ)) {
			return XSK_EFUSEPL_ERROR_INVALID_REF_CLK;
		}
		/**
		 * Initialize the system ,
		 * which means initialize the timer, xadc, and jtag
		 * server using the passed info.
		 */

		XilSKey_Efuse_StartTimer();

		Status = XilSKey_EfusePs_XAdcInit();
		if(Status != XST_SUCCESS) {
			ErrorCode = Status;
			return (XSK_EFUSEPL_ERROR_XADC + ErrorCode);
			}
#else
		if (XilSKey_Timer_Intialise() == XST_FAILURE) {
			return (XSK_EFUSEPL_ERROR_TIMER_INTIALISE_ULTRA);
		}
#endif
		/**
		 * Start using the Jtag server to read the JTAG ID and
		 * compare with the stored ID, if it not matches return with
		 * unique error code.
		 * By reading the Jtag ID we will be sure that the JTAG related
		 * stuff is working as expected.
		 */
		if(JtagServerInit(InstancePtr) != XST_SUCCESS) {
			return XSK_EFUSEPL_ERROR_JTAG_SERVER_INIT;
		}

		InstancePtr->SystemInitDone = 1;
		PlFpgaFlag = InstancePtr->FpgaFlag;
	}

	/**
	 * If everything is ok then return PASS.
	 */
	return XST_SUCCESS;
}

/****************************************************************************/
/**
*
* Programs PL eFUSE with input data given through InstancePtr.
*
* @param	InstancePtr - Pointer to PL eFUSE instance which holds the
*                             input data to be written to PL eFUSE.
*
* @return
*		- XST_FAILURE - In case of failure
*		- XST_SUCCESS - In case of Success
*
* @note		When this API is called: Initializes the timer, XADC/xsysmon
*		and JTAG server subsystems. Returns an error in the following
*		cases, if the reference clock frequency is not in the range or
*		if the PL DAP ID is not identified, if the system is not in a
*		position to write the requested PL eFUSE bits
* 		(because the bits are already written or not allowed to write)
* 		if the temperature and voltage are not within range.
*
*****************************************************************************/
u32 XilSKey_EfusePl_Program(XilSKey_EPl *InstancePtr)
{
	u32 Status = (u32)XST_FAILURE;
	ErrorCode = XSK_EFUSEPL_ERROR_NONE;


	if(NULL == InstancePtr)	{
		return XSK_EFUSEPL_ERROR_PL_STRUCT_NULL;
	}

	if(!(InstancePtr->SystemInitDone))
	{

#ifdef XSK_ARM_PLATFORM
		u32 RefClk;
		RefClk = XilSKey_Timer_Intialise();
		/**
		 * Return error if the reference clock frequency is not in
		 * between 20 & 60MHz
		 */
		if((RefClk < XSK_EFUSEPL_MIN_REF_CLK_FREQ) ||
				(RefClk > XSK_EFUSEPL_MAX_REF_CLK_FREQ)) {
			return XSK_EFUSEPL_ERROR_INVALID_REF_CLK;
		}
		/**
		 * Initialize the system ,
		 * which means initialize the timer, xadc, and jtag
		 * server using the passed info.
		 */

		XilSKey_Efuse_StartTimer();

		Status = XilSKey_EfusePs_XAdcInit();
		if(Status != XST_SUCCESS) {
			ErrorCode = Status;
			return (XSK_EFUSEPL_ERROR_XADC + ErrorCode);
			}
#else
		if (XilSKey_Timer_Intialise() == XST_FAILURE) {
			return (XSK_EFUSEPL_ERROR_TIMER_INTIALISE_ULTRA);
		}
#endif
		/**
		 * Start using the Jtag server to read the JTAG ID and
		 * compare with the stored ID, if it not matches return with
		 * unique error code.
		 * By reading the Jtag ID we will be sure that the JTAG related
		 * stuff is working as expected.
		 */
		if(JtagServerInit(InstancePtr) != XST_SUCCESS) {
			return XSK_EFUSEPL_ERROR_JTAG_SERVER_INIT;
		}

		InstancePtr->SystemInitDone = 1;
		PlFpgaFlag = InstancePtr->FpgaFlag;
	}
	if (PlFpgaFlag == XSK_FPGA_SERIES_ZYNQ) {

		Status = XilSKey_EfusePl_Program_Zynq(InstancePtr);
		if (Status != XST_SUCCESS) {
			return Status;
		}

	}
#ifdef XSK_MICROBLAZE_PLATFORM
	else {
		Status = XilSKey_EfusePl_Program_Ultra(InstancePtr);
		if (Status != XST_SUCCESS) {
			return Status;
		}
	}
#endif

	/**
	 * If everything is ok then return PASS.
	 */
	return XST_SUCCESS;
}
/****************************************************************************/
/**
*
* Checks whether data bits (0-29) of an PL eFUSE row are all zeroes or not
*
*
*
* @param	RowDataPtr - row data pointer
*
* @return
*
*	- XST_FAILURE - In case of failure
*	- XST_SUCCESS - In case of Success
*
* @note		None.
*
*****************************************************************************/
u8 XilSKey_EfusePl_IsVectorAllZeros(u8 *RowDataPtr)
{
	u32 Index;
	if (PlFpgaFlag == XSK_FPGA_SERIES_ZYNQ) {
		for(Index=0; Index<=XSK_EFUSEPL_ARRAY_ECC_END_BIT_IN_A_ROW;
								Index++) {
			if(RowDataPtr[Index] != 0) {
				return XST_FAILURE;
			}
		}
	}
	if (PlFpgaFlag == XSK_FPGA_SERIES_ULTRA) {
		for(Index = 0; Index < XSK_EFUSEPL_ARRAY_MAX_COL; Index++) {
			if(RowDataPtr[Index] != 0) {
				return XST_FAILURE;
			}
		}
	}
	if (PlFpgaFlag == XSK_FPGA_SERIES_ULTRA_PLUS) {
		for(Index = 0; Index < XSK_EFUSEPL_ARRAY_MAX_COL_ULTRA_PLUS;
						Index++) {
			if((RowDataPtr[Index] |
					RowDataPtr[
			XSK_EFUSEPL_ARRAY_MAX_COL_ULTRA_PLUS + Index]) != 0) {
				return XST_FAILURE;
			}
		}
	}

	return XST_SUCCESS;
}

/****************************************************************************/
/**
*
* Programs a bit of PL eFUSE row
*
*
*
* @param Row - row number
* @param Bit - bit position
*
* @return
*
*	- XST_FAILURE - In case of failure
*	- XST_SUCCESS - In case of Success
*
*
* @note		Updates the global variable ErrorCode with error code(if any).
*
*****************************************************************************/
u8 XilSKey_EfusePl_ProgramBit(u8 Row, u8 Bit)
{
#ifdef XSK_ZYNQ_PLATFORM
	XSKEfusePs_XAdc PL_XAdc;
#endif

	/**
	 *Check if the row position is valid.
	 */
	if( (Row > XSK_EFUSEPL_ARRAY_USER_DATA_START_ROW) ||
			( (Row > XSK_EFUSEPL_ARRAY_FUSE_CNTRL_ROW)
		     && (Row < XSK_EFUSEPL_ARRAY_AES_DATA_ROW_START)
			)
	  )
	{
		ErrorCode = XSK_EFUSEPL_ERROR_WRITE_ROW_OUT_OF_RANGE;
		return XST_FAILURE;
	}

	/**
	 * Check if the bit position is valid.
	 */
	if (Bit > XSK_EFUSEPL_ARRAY_ECC_END_BIT_IN_A_ROW) {
		ErrorCode = XSK_EFUSEPL_ERROR_WRITE_BIT_OUT_OF_RANGE;
		return XST_FAILURE;
	}

	/**
	 * If row=0 then bit should be either 1 to 5 and 8 to 10, 15 to 19 and
	 * 22 to 24 rest all are not supported
	 */
	if(Row == XSK_EFUSEPL_ARRAY_FUSE_CNTRL_ROW) {

		if((Bit == XSK_EFUSEPL_ARRAY_FUSE_CNTRL_START_BIT) ||
			(Bit == XSK_EFUSEPL_ARRAY_UNSUPPORTED_BIT6) ||
			(Bit == XSK_EFUSEPL_ARRAY_UNSUPPORTED_BIT7)||
			(Bit == XSK_EFUSEPL_ARRAY_UNSUPPORTED_RED_FOR_BIT6)||
			(Bit == XSK_EFUSEPL_ARRAY_UNSUPPORTED_RED_FOR_BIT7)) {
			ErrorCode = XSK_EFUSEPL_ERROR_WRITE_BIT_OUT_OF_RANGE;
			return XST_FAILURE;
		}

		if((Bit > XSK_EFUSEPL_ARRAY_FUSE_CNTRL_END_BIT) &&
		   (Bit < XSK_EFUSEPL_ARRAY_FUSE_CNTRL_REDUNDENT_START_BIT)) {
			ErrorCode = XSK_EFUSEPL_ERROR_WRITE_BIT_OUT_OF_RANGE;
			return XST_FAILURE;
		}

		if(Bit > XSK_EFUSEPL_ARRAY_FUSE_CNTRL_REDUNDENT_END_BIT){
			ErrorCode = XSK_EFUSEPL_ERROR_WRITE_BIT_OUT_OF_RANGE;
			return XST_FAILURE;
		}
	}

	/**
	 * Monitor the Voltage and temperature using XADC, if out of range return
	 * unique error.
	 */
#ifdef XSK_ZYNQ_PLATFORM

	PL_XAdc.VType = XSK_EFUSEPS_VAUX;
	XilSKey_EfusePs_XAdcReadTemperatureAndVoltage(&PL_XAdc);
	if((PL_XAdc.Temp < XSK_EFUSEPL_WRITE_TEMP_MIN_RAW) ||
		(PL_XAdc.Temp > XSK_EFUSEPL_WRITE_TEMP_MAX_RAW)) {
		ErrorCode = XSK_EFUSEPL_ERROR_WRITE_TMEPERATURE_OUT_OF_RANGE;
		return XST_FAILURE;
	}

	if((PL_XAdc.V < XSK_EFUSEPL_WRITE_VOLTAGE_VCCAUX_MIN_RAW) ||
		(PL_XAdc.V > XSK_EFUSEPL_WRITE_VOLTAGE_VCCAUX_MAX_RAW)) {
		ErrorCode = XSK_EFUSEPL_ERROR_WRITE_VCCAUX_VOLTAGE_OUT_OF_RANGE;
		return XST_FAILURE;
	}

	PL_XAdc.VType = XSK_EFUSEPS_VINT;
	XilSKey_EfusePs_XAdcReadTemperatureAndVoltage(&PL_XAdc);
	if((PL_XAdc.V < XSK_EFUSEPL_WRITE_VOLTAGE_VCCINT_MIN_RAW) ||
	   (PL_XAdc.V > XSK_EFUSEPL_WRITE_VOLTAGE_VCCINT_MAX_RAW)) {
		ErrorCode = XSK_EFUSEPL_ERROR_WRITE_VCCINT_VOLTAGE_OUT_OF_RANGE;
		return XST_FAILURE;
	}
#endif

	JtagWrite(Row, Bit);
	return XST_SUCCESS;
}
/****************************************************************************/
/**
*
* Programs PL eFUSE row
*
* @param Row     - row number
* @param RowData - row data pointer
*
* @return
*
*	- XST_FAILURE - In case of failure
*	- XST_SUCCESS - In case of Success
*
*
* @note		Updates the global variable ErrorCode with error code(if any).
*
*****************************************************************************/

u8 XilSKey_EfusePl_ProgramRow(u8 Row, u8 *RowData)
{
	u32 Bit = 0;
	u8 ECCData[XSK_EFUSEPL_ARRAY_MAX_ECC_BITS_IN_A_ROW] = {0};

	/**
	 * check if row_data is not NULL
	 */
	if(NULL == RowData) {
		ErrorCode = XSK_EFUSEPL_ERROR_READ_BUFFER_NULL;
		return XST_FAILURE;
	}

	if(Row == XSK_EFUSEPL_ARRAY_FUSE_CNTRL_ROW) {
		ErrorCode = XSK_EFUSEPL_ERROR_WRITE_ROW_OUT_OF_RANGE;
		return XST_FAILURE;
	}

	for(Bit=0; Bit < XSK_EFUSEPL_ARRAY_MAX_PAYLAOD_BITS_IN_A_ROW ; Bit++ ) {
		if(RowData[Bit]) {
			if(XilSKey_EfusePl_ProgramBit(Row, Bit) != XST_SUCCESS) {
				return XST_FAILURE;
			}

			if(XilSKey_EfusePl_VerifyBit(Row,Bit) != XST_SUCCESS) {
				return XST_FAILURE;
			}
		}
	}

	XilSKey_EfusePl_CalculateEcc(RowData, ECCData);
	for(Bit=XSK_EFUSEPL_ARRAY_ECC_START_BIT_IN_A_ROW;
		Bit <= XSK_EFUSEPL_ARRAY_ECC_END_BIT_IN_A_ROW;
		Bit++) {
		if(ECCData[Bit - XSK_EFUSEPL_ARRAY_ECC_START_BIT_IN_A_ROW]) {
			if(XilSKey_EfusePl_ProgramBit(Row, Bit) != XST_SUCCESS) {
				return XST_FAILURE;
			}

			if(XilSKey_EfusePl_VerifyBit(Row,Bit) != XST_SUCCESS) {
				return XST_FAILURE;
			}

		}
	}

	return XST_SUCCESS;
}
/****************************************************************************/
/**
*
* Programs PL eFUSE Control Register
*
*
*
* @param	CtrlData - Control data pointer
*
* @return
*
*	- XST_FAILURE - In case of failure
*	- XST_SUCCESS - In case of Success
*
*
* @note		Updates the global variable ErrorCode with error code(if any).
*
*****************************************************************************/
u8 XilSKey_EfusePl_ProgramControlRegister(u8 *CtrlData)
{
	u8 TmpCtrlData[XSK_EFUSEPL_ARRAY_FUSE_CNTRL_MAX_BITS]={0};
	u32 Index = 0;

	/**
	 * check if cntrl_data is not NULL
	 */
	if(NULL == CtrlData) {
		ErrorCode = XSK_EFUSEPL_ERROR_CNTRL_WRITE_BUFFER_NULL;
		return XST_FAILURE;
	}

	if (PlFpgaFlag == XSK_FPGA_SERIES_ZYNQ) {
		/**
		 *	Read the FUSE_CNTRL register
		 */

		if(XilSKey_EfusePl_ReadControlRegister(TmpCtrlData) != XST_SUCCESS)	{
			return XST_FAILURE;
		}

		/**
		 * check if FUSE_CNTRL allows us to write FUSE_CNTRL eFUSE array for Zynq.
		 */
		if(TmpCtrlData[XSK_EFUSEPL_CNTRL_DISABLE_FUSE_CNTRL_WRITE] == TRUE) {
			/**
			 * This means we cannot program FUSE_CNTRL register
			 */
			ErrorCode = XSK_EFUSEPL_ERROR_FUSE_CNTRL_WRITE_DISABLED;
			return XST_FAILURE;
		}

		for(Index=1;Index<XSK_EFUSEPL_ARRAY_FUSE_CNTRL_MAX_BITS;Index++)	{

			if((Index == 6) || (Index == 7)) {
				continue;
			}

			if((CtrlData[Index] == TRUE) && (TmpCtrlData[Index] == FALSE)) {
				if(XilSKey_EfusePl_ProgramBit(
					XSK_EFUSEPL_ARRAY_FUSE_CNTRL_ROW,
						Index) != XST_SUCCESS) {
					return XST_FAILURE;
				}


				if(XilSKey_EfusePl_VerifyBit(
					XSK_EFUSEPL_ARRAY_FUSE_CNTRL_ROW,
					Index) != XST_SUCCESS) {
					return XST_FAILURE;
				}
				if(XilSKey_EfusePl_ProgramBit(
					XSK_EFUSEPL_ARRAY_FUSE_CNTRL_ROW,
					Index +
				XSK_EFUSEPL_ARRAY_FUSE_CNTRL_REDUNDENT_START_BIT)
						!= XST_SUCCESS) {
					return XST_FAILURE;
				}

				if(XilSKey_EfusePl_VerifyBit(
					XSK_EFUSEPL_ARRAY_FUSE_CNTRL_ROW,
					(Index +
				XSK_EFUSEPL_ARRAY_FUSE_CNTRL_REDUNDENT_START_BIT))
							!= XST_SUCCESS) {
					return XST_FAILURE;
				}

			}
		}
	}
	/* Programming for Ultrascale Series */
	if (PlFpgaFlag == XSK_FPGA_SERIES_ULTRA) {
		if (XilSKey_EfusePl_ProgramControlReg_Ultra(CtrlData) != XST_SUCCESS) {
			return XST_FAILURE;
		}
	}
	if (PlFpgaFlag == XSK_FPGA_SERIES_ULTRA_PLUS) {
		if (XilSKey_EfusePl_ProgramControlReg_Ultra_Plus(CtrlData) !=
						XST_SUCCESS) {
			return XST_FAILURE;
		}
	}
	return XST_SUCCESS;
}
/****************************************************************************/
/**
*
*	Reads a PL eFUSE bit & stores.
*
*
* @param	Row	     - row number
* @param	Bit	     - bit position in the specified row
* @param	MarginOption - Margin Option(One of the reading method of PLeFUSE)
* @param	BitData	     - Place holder to store the read value
*
* @return
*
*	- XST_FAILURE - In case of failure
*	- XST_SUCCESS - In case of Success
*
*
* @note		Updates the global variable ErrorCode with error code(if any).
*
*****************************************************************************/

u8 XilSKey_EfusePl_ReadBit(u8 Row, u8 Bit, u8 MarginOption, u8 *BitData)
{
	u8 RowData[XSK_EFUSEPL_ARRAY_MAX_COL]={0};

	/**
	 * check if row_data is not NULL
	 */
	if(NULL == BitData)	{
		ErrorCode = XSK_EFUSEPL_ERROR_READ_BUFFER_NULL;
		return XST_FAILURE;
	}
	/**
	 *Check if the bit position is valid.
	 */

	if (Bit > XSK_EFUSEPL_ARRAY_ECC_END_BIT_IN_A_ROW) {
		ErrorCode = XSK_EFUSEPL_ERROR_READ_BIT_OUT_OF_RANGE;
		return XST_FAILURE;
	}


	/**
	 * If row=0 then bit should be either 1 to 5 and 8 to 10, 15 to 19 and
	 * 22 to 24 rest all are not supported
	 */
	if(Row == XSK_EFUSEPL_ARRAY_FUSE_CNTRL_ROW) {

		if((Bit == XSK_EFUSEPL_ARRAY_FUSE_CNTRL_START_BIT) ||
			(Bit == XSK_EFUSEPL_ARRAY_UNSUPPORTED_BIT6) ||
			(Bit == XSK_EFUSEPL_ARRAY_UNSUPPORTED_BIT7)||
			(Bit == XSK_EFUSEPL_ARRAY_UNSUPPORTED_RED_FOR_BIT6)||
			(Bit == XSK_EFUSEPL_ARRAY_UNSUPPORTED_RED_FOR_BIT7)) {
			ErrorCode = XSK_EFUSEPL_ERROR_READ_BIT_OUT_OF_RANGE;
			return XST_FAILURE;
		}

		if((Bit > XSK_EFUSEPL_ARRAY_FUSE_CNTRL_END_BIT) &&
			(Bit < XSK_EFUSEPL_ARRAY_FUSE_CNTRL_REDUNDENT_START_BIT)) {
			ErrorCode = XSK_EFUSEPL_ERROR_READ_BIT_OUT_OF_RANGE;
			return XST_FAILURE;
		}

		if(Bit > XSK_EFUSEPL_ARRAY_FUSE_CNTRL_REDUNDENT_END_BIT) {
			ErrorCode = XSK_EFUSEPL_ERROR_READ_BIT_OUT_OF_RANGE;
			return XST_FAILURE;
		}
	}

	if(XilSKey_EfusePl_ReadRow(Row, MarginOption,RowData) != XST_SUCCESS) {
		return XST_FAILURE;
	}

	*BitData = RowData[Bit];

	return XST_SUCCESS;
}

/****************************************************************************/
/**
*
*	Reads row data of a specified row
*
*
* @param	Row          - row number
* @param	MarginOption - Margin Option(One of the reading method of PLeFUSE)
* @param	RowDataBytes - To store the read data bytes of specified row
*
* @return
*
*	- XST_FAILURE - In case of failure
*	- XST_SUCCESS - In case of Success
*
*
* @note		Read data will be stored @row_data_bytes
*
*****************************************************************************/

u8 XilSKey_EfusePl_ReadRow(u32 Row, u8 MarginOption, u8* RowDataBytes)
{
#ifdef XSK_ZYNQ_PLATFORM
	XSKEfusePs_XAdc PL_XAdc;
#endif
	u32 RowDataBits=0;

	/**
	 * Check if the row position is valid.
	 */
	if((Row > XSK_EFUSEPL_ARRAY_USER_DATA_START_ROW) ||
		((Row > XSK_EFUSEPL_ARRAY_FUSE_CNTRL_ROW) &&
		(Row < XSK_EFUSEPL_ARRAY_AES_DATA_ROW_START))) {
		ErrorCode = XSK_EFUSEPL_ERROR_READ_ROW_OUT_OF_RANGE;
		return XST_FAILURE;
	}

	/**
	 * Check the read margin option.
	 */
	if( (MarginOption != XSK_EFUSEPL_READ_NORMAL ) &&
		(MarginOption != XSK_EFUSEPL_READ_MARGIN_1)&&
		(MarginOption != XSK_EFUSEPL_READ_MARGIN_2))
	{
		ErrorCode = XSK_EFUSEPL_ERROR_READ_MARGIN_OUT_OF_RANGE;
		return XST_FAILURE;
	}

	/**
	 * check if row_data is not NULL
	 */
	if(NULL == RowDataBytes)
	{
		ErrorCode = XSK_EFUSEPL_ERROR_READ_BUFFER_NULL;
		return XST_FAILURE;
	}


	/**
	 * Monitor the Voltage and temperature using XADC, if out of range return
	 * unique error.
	 */
#ifdef XSK_ZYNQ_PLATFORM
	PL_XAdc.VType = XSK_EFUSEPS_VAUX;
	XilSKey_EfusePs_XAdcReadTemperatureAndVoltage(&PL_XAdc);
	if((PL_XAdc.Temp < XSK_EFUSEPL_READ_TEMP_MIN_RAW) ||
		(PL_XAdc.Temp > XSK_EFUSEPL_READ_TEMP_MAX_RAW)) {
		ErrorCode = XSK_EFUSEPL_ERROR_READ_TMEPERATURE_OUT_OF_RANGE;
		return XST_FAILURE;
	}

	if((PL_XAdc.V < XSK_EFUSEPL_READ_VOLTAGE_VCCAUX_MIN_RAW) ||
		(PL_XAdc.V > XSK_EFUSEPL_READ_VOLTAGE_VCCAUX_MAX_RAW)) {
		ErrorCode = XSK_EFUSEPL_ERROR_READ_VCCAUX_VOLTAGE_OUT_OF_RANGE;
		return XST_FAILURE;
	}

	PL_XAdc.VType = XSK_EFUSEPS_VINT;
	XilSKey_EfusePs_XAdcReadTemperatureAndVoltage(&PL_XAdc);
	if((PL_XAdc.V < XSK_EFUSEPL_READ_VOLTAGE_VCCINT_MIN_RAW) ||
		(PL_XAdc.V > XSK_EFUSEPL_READ_VOLTAGE_VCCINT_MAX_RAW)) {
		ErrorCode = XSK_EFUSEPL_ERROR_READ_VCCINT_VOLTAGE_OUT_OF_RANGE;
		return XST_FAILURE;
	}
#endif
	/**
	 * Here we have to use the impact algorithm to read the eFUSE row.
	 * and return the data in row_data.
	 */

	if((MarginOption & XSK_EFUSEPL_READ_NORMAL) == XSK_EFUSEPL_READ_NORMAL) {

		JtagRead(Row, (unsigned int *)&RowDataBits, 0);
	}
	else if((MarginOption & XSK_EFUSEPL_READ_MARGIN_1) ==
							XSK_EFUSEPL_READ_MARGIN_1) {
		JtagRead(Row, (unsigned int *)&RowDataBits, 1);
	}
	else if((MarginOption & XSK_EFUSEPL_READ_MARGIN_2) ==
							XSK_EFUSEPL_READ_MARGIN_2) {
		JtagRead(Row, (unsigned int *)&RowDataBits, 2);
	}

	XilSKey_Efuse_ConvertBitsToBytes((u8 *)&RowDataBits, RowDataBytes, 32);

	return XST_SUCCESS;
}
/****************************************************************************/
/**
*
*	Reads PL eFUSE Control Register data
*
*
* @param	CtrlData	-	Place holder to store the read data (control register)
*
* @return
*
*	- XST_FAILURE - In case of failure
*	- XST_SUCCESS - In case of Success
*
*
* @note		Updates the global variable ErrorCode with error code (if any)
*
*****************************************************************************/

u8 XilSKey_EfusePl_ReadControlRegister(u8 *CtrlData)
{
	u8 RowData[XSK_EFUSEPL_ARRAY_MAX_COL]={0};
	u32 Index=0;
	u32 CtrlDataUltra;
	u8 CtrlDataUltraPlus[4];

	/**
	 * check if cntrl_data is not NULL
	 */
	if(NULL == CtrlData) {
		ErrorCode = XSK_EFUSEPL_ERROR_READ_BUFFER_NULL;
		return XST_FAILURE;
	}
	if (PlFpgaFlag == XSK_FPGA_SERIES_ZYNQ) {
		if(XilSKey_EfusePl_ReadRow(XSK_EFUSEPL_ARRAY_FUSE_CNTRL_ROW,
			XSK_EFUSEPL_READ_NORMAL, RowData) != XST_SUCCESS)
		{
			return XST_FAILURE;
		}

		for(Index=0; Index < XSK_EFUSEPL_ARRAY_FUSE_CNTRL_MAX_BITS; Index++)
		{
			CtrlData[Index] = RowData[Index] |
					 RowData[Index +
				  XSK_EFUSEPL_ARRAY_FUSE_CNTRL_REDUNDENT_START_BIT];
		}
	}
	if (PlFpgaFlag == XSK_FPGA_SERIES_ULTRA) {
		if (XilSKey_EfusePl_GetRowData_Ultra(XSK_EFUSEPL_CNTRL_ROW_ULTRA,
			(u8 *)&CtrlDataUltra, XSK_EFUSEPL_PAGE_0_ULTRA) !=
								XST_SUCCESS) {
			return XST_FAILURE;
		}
		XilSKey_Efuse_ConvertBitsToBytes((u8 *)&CtrlDataUltra,
							CtrlData, 32);
	}
	if (PlFpgaFlag == XSK_FPGA_SERIES_ULTRA_PLUS) {

		if (XilSKey_EfusePl_GetDataRowRange_Ultra(
				XSK_EFUSEPL_CNTRL_ROW_START_ULTRA_PLUS,
				XSK_EFUSEPL_CNTRL_ROW_END_ULTRA_PLUS,
			(u8 *)CtrlDataUltraPlus, XSK_EFUSEPL_PAGE_0_ULTRA) !=
								XST_SUCCESS) {
			return XST_FAILURE;
		}

		XilSKey_Efuse_ConvertBitsToBytes((u8 *)CtrlDataUltraPlus,
					CtrlData, 32);
	}

	return XST_SUCCESS;
}

/****************************************************************************/
/**
*
*
*	Verify the PL eFUSE bit blown by reading it back in all possible margin
* reads (Normal, Margin 1 and Margin 2).
*
* @param	Row			 - row number
* @param	Bit			 - bit position of the specified row
*
* @return
*
*	- XST_FAILURE - In case of failure
*	- XST_SUCCESS - In case of Success
*
*
* @note		Updates the global variable ErrorCode with error code(if any).
*
*****************************************************************************/
u8 XilSKey_EfusePl_VerifyBit(u8 Row, u8 Bit)
{
	u8 BitData = 0;
	u8 MarginOption;

	for (MarginOption = XSK_EFUSEPL_READ_NORMAL;
			MarginOption <= XSK_EFUSEPL_READ_MARGIN_2;
					MarginOption = MarginOption*2) {

		if(XilSKey_EfusePl_ReadBit(Row, Bit, MarginOption, &BitData)
							!= XST_SUCCESS)	{
			return XST_FAILURE;
		}

		if(BitData == FALSE) {
			ErrorCode = XSK_EFUSEPL_ERROR_READ_BIT_VALUE_NOT_SET;
			return XST_FAILURE;
		}
	}

	return XST_SUCCESS;
}
/****************************************************************************/
/**
*
*	Calculates and stores the ECC data of row data
*
*
* @param	RowData	- Pointer to row data on which ECC will be calculated
* @param	ECCData	- Pointer to store the calculated ECC
*
* @return
*
*	None
*
* @note		None.
*
*****************************************************************************/
void XilSKey_EfusePl_CalculateEcc(u8 *RowData, u8 *ECCData)
{
	ECCData[0] = 	RowData[0] ^ RowData[1] ^ RowData[2] ^
					RowData[3] ^ RowData[8] ^ RowData[9] ^
					RowData[10] ^ RowData[11] ^ RowData[12] ^
					RowData[13] ^ RowData[14] ^ RowData[15] ^
					RowData[16] ^ RowData[17];

	ECCData[1] = 	RowData[1] ^ RowData[2] ^ RowData[3] ^
					RowData[5] ^ RowData[6] ^ RowData[7] ^
					RowData[11] ^ RowData[12] ^ RowData[13] ^
					RowData[14] ^ RowData[18] ^ RowData[19] ^
					RowData[20];

	ECCData[2] = 	RowData[0] ^ RowData[2] ^ RowData[3] ^
					RowData[4] ^ RowData[6] ^ RowData[7] ^
					RowData[9] ^ RowData[10] ^ RowData[13] ^
					RowData[15] ^ RowData[18] ^ RowData[21] ^
					RowData[22];

	ECCData[3] = 	RowData[0] ^ RowData[1] ^ RowData[3] ^
					RowData[4] ^ RowData[5] ^ RowData[7] ^
					RowData[8] ^ RowData[10] ^ RowData[12] ^
					RowData[16] ^ RowData[19] ^ RowData[21] ^
					RowData[23];

	ECCData[4] = 	RowData[0] ^ RowData[1] ^ RowData[2] ^
					RowData[4] ^ RowData[5] ^ RowData[6] ^
					RowData[8] ^ RowData[9] ^ RowData[11] ^
					RowData[17] ^ RowData[20] ^ RowData[22] ^
					RowData[23];

	/**
	 * This is the DED data
	 */

	ECCData[5] = 	RowData[0] ^ RowData[1] ^ RowData[2] ^
					RowData[3] ^ RowData[4] ^ RowData[5] ^
					RowData[6] ^ RowData[7] ^ RowData[8] ^
					RowData[9] ^ RowData[10] ^ RowData[11] ^
					RowData[12] ^ RowData[13] ^ RowData[14] ^
					RowData[15] ^ RowData[16] ^ RowData[17] ^
					RowData[18] ^ RowData[19] ^ RowData[20] ^
					RowData[21] ^ RowData[22] ^ RowData[23] ^
					ECCData[0] ^ ECCData[1] ^ ECCData[2] ^
					ECCData[3] ^ ECCData[4];
}

/****************************************************************************/
/**
*
*
* Reads the PL efuse status bits and gets all secure and control bits.
*
*
* @param	InstancePtr	Pointer to PL eFUSE instance.
* @param	StatusBits	Buffer to store the status bits read.
*
* @return
*
*	- XST_FAILURE - In case of failure
*	- XST_SUCCESS - In case of Success

*
*****************************************************************************/
u32 XilSKey_EfusePl_ReadStatus(XilSKey_EPl *InstancePtr, u32 *StatusBits)
{
#if defined XSK_ZYNQ_PLATFORM || defined XSK_MICROBLAZE_PLATFORM
	unsigned int RowData;
	XSKEfusePs_XAdc PL_XAdc = {0};
#endif
	if(NULL == StatusBits)	{
		return XST_FAILURE;
	}

	if(NULL == InstancePtr)	{
		return XSK_EFUSEPL_ERROR_PL_STRUCT_NULL;
	}

	if(!(InstancePtr->SystemInitDone))
	{

#ifdef XSK_ZYNQ_PLATFORM
		u32 RefClk;
		u32 Status;
		/**
		 * Initialize the variables
		 */
		RefClk = XilSKey_Timer_Intialise();

		/**
		 * Return error if the reference clock frequency is not in
		 * between 20 & 60MHz
		 */
		if((RefClk < XSK_EFUSEPL_MIN_REF_CLK_FREQ) ||
				(RefClk > XSK_EFUSEPL_MAX_REF_CLK_FREQ)) {
			return XSK_EFUSEPL_ERROR_INVALID_REF_CLK;
		}

		/**
		 * Initialize the timer, XADC and jtag server
		 */

		XilSKey_Efuse_StartTimer();

		Status = XilSKey_EfusePs_XAdcInit();
		if(Status != XST_SUCCESS) {
			ErrorCode = Status;
			return (XSK_EFUSEPL_ERROR_XADC + ErrorCode);
		}
#endif


		InstancePtr->SystemInitDone = 1;
		PlFpgaFlag = InstancePtr->FpgaFlag;
	}

#ifdef XSK_ZYNQ_PLATFORM

	/**
	 * Monitor the Voltage and temperature using XADC, if out of range return
	 * unique error.
	 */
	PL_XAdc.VType = XSK_EFUSEPS_VAUX;
	XilSKey_EfusePs_XAdcReadTemperatureAndVoltage(&PL_XAdc);
	if((PL_XAdc.Temp < XSK_EFUSEPL_READ_TEMP_MIN_RAW) ||
		(PL_XAdc.Temp > XSK_EFUSEPL_READ_TEMP_MAX_RAW)) {
		ErrorCode = XSK_EFUSEPL_ERROR_READ_TMEPERATURE_OUT_OF_RANGE;
		return XST_FAILURE;
	}

	if((PL_XAdc.V < XSK_EFUSEPL_READ_VOLTAGE_VCCAUX_MIN_RAW) ||
		(PL_XAdc.V > XSK_EFUSEPL_READ_VOLTAGE_VCCAUX_MAX_RAW)) {
		ErrorCode = XSK_EFUSEPL_ERROR_READ_VCCAUX_VOLTAGE_OUT_OF_RANGE;
		return XST_FAILURE;
	}
	PL_XAdc.VType = XSK_EFUSEPS_VINT;
	XilSKey_EfusePs_XAdcReadTemperatureAndVoltage(&PL_XAdc);
	if((PL_XAdc.V < XSK_EFUSEPL_READ_VOLTAGE_VCCINT_MIN_RAW) ||
		(PL_XAdc.V > XSK_EFUSEPL_READ_VOLTAGE_VCCINT_MAX_RAW)) {
		ErrorCode = XSK_EFUSEPL_ERROR_READ_VCCINT_VOLTAGE_OUT_OF_RANGE;
		return XST_FAILURE;
	}
	XilSKeyJtag.CurSlr = 0;

	/*
	 * Read row 0 for status bits
	 */
	JtagRead(0, &RowData, 0);

	*StatusBits = RowData & 0xFFFFFF;
#endif
	/* For Ultrascale */
#ifdef XSK_MICROBLAZE_PLATFORM
	XilSKey_GetSlrNum(InstancePtr->MasterSlr,
			InstancePtr->SlrConfigOrderIndex, &(XilSKeyJtag.CurSlr));

	/* Monitor Temperature and voltage */
	XilSKey_EfusePs_XAdcReadTemperatureAndVoltage(&PL_XAdc);
	if((PL_XAdc.Temp < XSK_EFUSEPL_TEMP_MIN_RAW_ULTRA) ||
		(PL_XAdc.Temp > XSK_EFUSEPL_TEMP_MAX_RAW_ULTRA)) {
		return XSK_EFUSEPL_ERROR_READ_TMEPERATURE_OUT_OF_RANGE;
	}

	if((PL_XAdc.V < XSK_EFUSEPL_VOL_VCCAUX_MIN_RAW_ULTRA) ||
		(PL_XAdc.V > XSK_EFUSEPL_VOL_VCCAUX_MAX_RAW_ULTRA)) {
		return XSK_EFUSEPL_ERROR_READ_VCCAUX_VOLTAGE_OUT_OF_RANGE;
	}

	JtagRead_Status_Ultrascale((u32 *)&RowData);
	*StatusBits = RowData & 0xFFFFFF;

#endif

	return XST_SUCCESS;

}

/****************************************************************************/
/**
*
*
* Reads the PL efuse keys and stores them in the corresponding
* arrays in instance structure.
*
* @param	InstancePtr	Pointer to PL eFUSE instance.
*
* @return
*
*	- XST_FAILURE - In case of failure
*	- XST_SUCCESS - In case of Success
*
*
* @note		This function initializes the timer, XADC and JTAG server
* 		subsystems, if not already done so.
* 		In Zynq - Reads AES key and User keys.
* 		In Ultrascale - Reads 32 bit and 128 bit User keys and RSA hash
* 		But AES key cannot be read directly it can be verified with
*		CRC check (for that we need to update the instance with
*		32 bit CRC value, API updates whether provided CRC value is
*		matched with actuals or not). To calculate
* 		the CRC of expected AES key one can use any of the following
* 		APIs XilSKey_CrcCalculation() or
*		XilSkey_CrcCalculation_AesKey()
*
*****************************************************************************/
u32 XilSKey_EfusePl_ReadKey(XilSKey_EPl *InstancePtr)
{
	u32 Status = (u32)XST_FAILURE;

	if(NULL == InstancePtr)	{
		return XSK_EFUSEPL_ERROR_PL_STRUCT_NULL;
	}

	if(!(InstancePtr->SystemInitDone))
	{

#ifdef XSK_ARM_PLATFORM
		u32 RefClk;
		RefClk = XilSKey_Timer_Intialise();

		/**
		 * Return error if the reference clock frequency is not in
		 * between 20 & 60MHz
		 */
		if((RefClk < XSK_EFUSEPL_MIN_REF_CLK_FREQ) ||
				(RefClk > XSK_EFUSEPL_MAX_REF_CLK_FREQ)) {
			return XSK_EFUSEPL_ERROR_INVALID_REF_CLK;
		}

		/**
		 * Initialize the timer and jtag server
		 */

		XilSKey_Efuse_StartTimer();

		Status = XilSKey_EfusePs_XAdcInit();
		if(Status != XST_SUCCESS) {
			ErrorCode = Status;
			return (XSK_EFUSEPL_ERROR_XADC + ErrorCode);
		}
#endif
		if(JtagServerInit(InstancePtr) != XST_SUCCESS) {
			return XSK_EFUSEPL_ERROR_JTAG_SERVER_INIT;
		}

		InstancePtr->SystemInitDone = 1;
		PlFpgaFlag = InstancePtr->FpgaFlag;
	}
	if (PlFpgaFlag == XSK_FPGA_SERIES_ZYNQ) {
		Status = XilSKey_EfusePl_ReadKey_Zynq(InstancePtr);
		if (Status != XST_SUCCESS) {
			return Status;
		}
	}
#ifdef XSK_MICROBLAZE_PLATFORM
	else {
		Status = XilSKey_EfusePl_ReadKey_Ultra(InstancePtr);
		if (Status != XST_SUCCESS) {
			return Status;
		}
	}
#endif

	return XST_SUCCESS;

}

/****************************************************************************/
/**
* Reads the PL efuse key (AES and user) of Zynq
*
*
* @param	InstancePtr - Input data to be written to PL eFUSE
*
* @return
*	- XST_FAILURE - In case of failure
*	- XST_SUCCESS - In case of Success
*
*
* @note
*	Updates the global variable ErrorCode with error code(if any).
*
*****************************************************************************/
static INLINE u32 XilSKey_EfusePl_ReadKey_Zynq(XilSKey_EPl *InstancePtr)
{
#ifdef XSK_ZYNQ_PLATFORM
	XSKEfusePs_XAdc PL_XAdc = {0};
#endif
	u32 KeyCnt;
	u32 RowCount;
	unsigned int RowData;
	XilSKeyJtag.CurSlr = 0;

#ifdef XSK_ZYNQ_PLATFORM
	/**
	 * Monitor the Voltage and temperature using XADC, if out of range return
	 * unique error.
	 */
	PL_XAdc.VType = XSK_EFUSEPS_VAUX;
	XilSKey_EfusePs_XAdcReadTemperatureAndVoltage(&PL_XAdc);
	if((PL_XAdc.Temp < XSK_EFUSEPL_READ_TEMP_MIN_RAW) ||
		(PL_XAdc.Temp > XSK_EFUSEPL_READ_TEMP_MAX_RAW)) {
		ErrorCode = XSK_EFUSEPL_ERROR_READ_TMEPERATURE_OUT_OF_RANGE;
		return XST_FAILURE;
	}

	if((PL_XAdc.V < XSK_EFUSEPL_READ_VOLTAGE_VCCAUX_MIN_RAW) ||
		(PL_XAdc.V > XSK_EFUSEPL_READ_VOLTAGE_VCCAUX_MAX_RAW)) {
		ErrorCode = XSK_EFUSEPL_ERROR_READ_VCCAUX_VOLTAGE_OUT_OF_RANGE;
		return XST_FAILURE;
	}

	PL_XAdc.VType = XSK_EFUSEPS_VINT;
	XilSKey_EfusePs_XAdcReadTemperatureAndVoltage(&PL_XAdc);
	if((PL_XAdc.V < XSK_EFUSEPL_READ_VOLTAGE_VCCINT_MIN_RAW) ||
		(PL_XAdc.V > XSK_EFUSEPL_READ_VOLTAGE_VCCINT_MAX_RAW)) {
		ErrorCode = XSK_EFUSEPL_ERROR_READ_VCCINT_VOLTAGE_OUT_OF_RANGE;
		return XST_FAILURE;
	}
#endif
	/*
	 * Read AES key and User Key and
	 * store them in the variables in instance structure
	 */

	/*
	 * AES key 4 bytes
	 */
	KeyCnt = 0;

	/*
	 * Read row 20 to 29
	 */
	for(RowCount = 20; RowCount <= 29; RowCount++)
	{
		JtagRead(RowCount, &RowData, 0);
		InstancePtr->AESKeyReadback[KeyCnt++] = (u8)(RowData & 0xFF);
		RowData = RowData >> 8;
		InstancePtr->AESKeyReadback[KeyCnt++] = (u8)(RowData & 0xFF);
		RowData = RowData >> 8;
		InstancePtr->AESKeyReadback[KeyCnt++] = (u8)(RowData & 0xFF);
	}

	/*
	 * Read row 30
	 */
	JtagRead(30, &RowData, 0);
	InstancePtr->AESKeyReadback[KeyCnt++] = (u8)(RowData & 0xFF);
	RowData = RowData >> 8;
	InstancePtr->AESKeyReadback[KeyCnt] = (u8)(RowData & 0xFF);

	/*
	 * User key 4 bytes
	 */
	KeyCnt = 0;
	RowData = RowData >> 8;
	InstancePtr->UserKeyReadback[KeyCnt++] = (u8)(RowData & 0xFF);

	/*
	 * Read row 31
	 */
	JtagRead(31, &RowData, 0);
	InstancePtr->UserKeyReadback[KeyCnt++] = (u8)(RowData & 0xFF);
	RowData = RowData >> 8;
	InstancePtr->UserKeyReadback[KeyCnt++] = (u8)(RowData & 0xFF);
	RowData = RowData >> 8;
	InstancePtr->UserKeyReadback[KeyCnt++] = (u8)(RowData & 0xFF);

	return XST_SUCCESS;
}

#ifdef XSK_MICROBLAZE_PLATFORM
/****************************************************************************/
/**
* Reads the PL efuse keys AES,user,RSA and RES of Ultrascale based on the
* user selections.
*
* @param	InstancePtr is an instance of PL efuse of ultrascale
*
* @return
*		- XST_FAILURE - In case of failure
*		- XST_SUCCESS - In case of Success
*
*
* @note
*	Updates the global variable ErrorCode with error code(if any).
*
*****************************************************************************/
static INLINE u32 XilSKey_EfusePl_ReadKey_Ultra(XilSKey_EPl *InstancePtr)
{

	u32 Index;
	XilSKey_GetSlrNum(InstancePtr->MasterSlr,
			InstancePtr->SlrConfigOrderIndex, &(XilSKeyJtag.CurSlr));

	/* Checks conditions for read key */
	if (XilSKey_EfusePl_ReadKey_Checks(InstancePtr) != XST_SUCCESS) {
		return ErrorCode;
	}

	/* Initiating all read back arrays to zeros */
	InstancePtr->AESKeyMatched = FALSE;
	for (Index = 0; Index < XSK_EFUSEPL_RSA_KEY_HASH_SIZE_IN_BYTES; Index++) {
		if (Index < XSK_EFUSEPL_USER_KEY_SIZE_IN_BYTES) {
			InstancePtr->UserKeyReadback[Index] = 0;
		}
		if (Index < XSK_EFUSEPL_RSA_KEY_HASH_SIZE_IN_BYTES) {
			InstancePtr->RSAHashReadback[Index] = 0;
		}
		if (Index < XSK_EFUSEPL_128BIT_USERKEY_SIZE_IN_BYTES) {
			InstancePtr->User128BitReadBack[Index] = 0;
		}
	}
	/*
	 * Read AES Key is not possible directly, verifying through CRC
	 * is valid. If CheckAESKeyUltra is not enabled then CRC check
	 * for AES will not be performed
	 */
	if ((InstancePtr->CheckAESKeyUltra == TRUE) &&
	(XilSKey_EfusePl_VerifyAES_Ultrascale(
		InstancePtr->CrcOfAESKey) == XST_SUCCESS)) {
		InstancePtr->AESKeyMatched = TRUE;
	}

	/*
	 * Read User key of row 28
	 */
	if (InstancePtr->ReadUserKeyUltra == TRUE) {
		if (PlFpgaFlag == XSK_FPGA_SERIES_ULTRA) {
			if (XilSKey_EfusePl_GetDataRowRange_Ultra(
				XSK_EFUSEPL_USER_ROW_ULTRA,
				XSK_EFUSEPL_USER_ROW_ULTRA,
				InstancePtr->UserKeyReadback,
				XSK_EFUSEPL_PAGE_0_ULTRA) !=
						XST_SUCCESS) {
				return XST_FAILURE;
			}
		}
		if (PlFpgaFlag == XSK_FPGA_SERIES_ULTRA_PLUS) {
			if (XilSKey_EfusePl_GetDataRowRange_Ultra(
					XSK_EFUSEPL_USER_ROW_START_ULTRA_PLUS,
					XSK_EFUSEPL_USER_ROW_END_ULTRA_PLUS,
					InstancePtr->UserKeyReadback,
					XSK_EFUSEPL_PAGE_1_ULTRA) !=
							XST_SUCCESS) {
				return XST_FAILURE;
			}
		}
	}
	 /*
	  * Read RSA Key
	  */
	if (InstancePtr->ReadRSAKeyUltra == TRUE) {
		if (PlFpgaFlag == XSK_FPGA_SERIES_ULTRA) {
			if (XilSKey_EfusePl_GetDataRowRange_Ultra(
				XSK_EFUSEPL_RSA_ROW_START_ULTRA,
				XSK_EFUSEPL_RSA_ROW_END_ULTRA,
				InstancePtr->RSAHashReadback,
				XSK_EFUSEPL_PAGE_1_ULTRA) != XST_SUCCESS) {
				return XST_FAILURE;
			}
		}
		if (PlFpgaFlag == XSK_FPGA_SERIES_ULTRA_PLUS) {
			if (XilSKey_EfusePl_GetDataRowRange_Ultra(
				XSK_EFUSEPL_RSA_ROW_START_ULTRA_PLUS,
				XSK_EFUSEPL_RSA_ROW_END_ULTRA_PLUS,
				InstancePtr->RSAHashReadback,
				XSK_EFUSEPL_PAGE_1_ULTRA) != XST_SUCCESS) {
				return XST_FAILURE;
			}
		}

	}
	/* Read 128 bit User key */
	if (InstancePtr->ReadUser128BitUltra == TRUE) {
		if (PlFpgaFlag == XSK_FPGA_SERIES_ULTRA) {
			if (XilSKey_EfusePl_GetDataRowRange_Ultra(
				XSK_EFUSEPL_USER_128BIT_ROW_START_ULTRA,
				XSK_EFUSEPL_USER_128BIT_ROW_END_ULTRA,
				InstancePtr->User128BitReadBack,
				XSK_EFUSEPL_PAGE_1_ULTRA) != XST_SUCCESS) {
				return ErrorCode;
			}
		}
		if (PlFpgaFlag == XSK_FPGA_SERIES_ULTRA_PLUS) {
			if (XilSKey_EfusePl_GetDataRowRange_Ultra(
				XSK_EFUSEPL_USER_128BIT_ROW_START_ULTRA_PLUS,
				XSK_EFUSEPL_USER_128BIT_ROW_END_ULTRA_PLUS,
				InstancePtr->User128BitReadBack,
				XSK_EFUSEPL_PAGE_0_ULTRA) != XST_SUCCESS) {
				return ErrorCode;
			}
		}

	}

	return XST_SUCCESS;

}
#endif

/****************************************************************************/
/**
* Reads control register and checks the conditions for reading keys.
*
* @param	InstancePtr is an instance of PL efuse of ultrascale
*
* @return
*		- ErrorCode - In case of failure
*		- XST_SUCCESS - In case of Success
*
*
* @note		None.
*
*****************************************************************************/
static INLINE u32 XilSKey_EfusePl_ReadKey_Checks(XilSKey_EPl *InstancePtr)
{

	if(XilSKey_EfusePl_ReadControlRegister(CtrlBitsUltra) != XST_SUCCESS) {
		ErrorCode = XSK_EFUSEPL_ERROR_READING_FUSE_CNTRL;
		return XST_FAILURE;
	}

	if ((InstancePtr->CheckAESKeyUltra == TRUE) &&
		(CtrlBitsUltra[XSK_EFUSEPL_CNTRL_DISABLE_KEY_RD_ULTRA] == TRUE)) {
		ErrorCode = XSK_EFUSEPL_ERROR_READING_FUSE_AES_ROW;
		return XST_FAILURE;
	}

	if ((InstancePtr->ReadUserKeyUltra == TRUE) &&
	(CtrlBitsUltra[XSK_EFUSEPL_CNTRL_DISABLE_USER_KEY_RD_ULTRA] == TRUE)) {
		ErrorCode = XSK_EFUSEPL_ERROR_READING_FUSE_USER_DATA_ROW;
		return XST_FAILURE;
	}
	if ((InstancePtr->ReadRSAKeyUltra == TRUE) &&
	(CtrlBitsUltra[XSK_EFUSEPL_CNTRL_DISABLE_RSA_KEY_RD_ULTRA] == TRUE)) {
		ErrorCode = XSK_EFUSEPL_ERROR_READING_FUSE_RSA_ROW;
		return XST_FAILURE;
	}

	return XST_SUCCESS;
}

/****************************************************************************/
/**
* Reads the given normal or reduntant row of PL efuse key for Ultrascale.
*
*
* @param	Row is the row number of Fuse array.
* @param	MarginOption is the option to select in which mode read has
*		to be happened
* @param	RowDataBytes is a pointer to an array to store the read value
*		of given row.
* @param	Redundant is the option to be selected either redundant row
*		or normal row.
* @param	Page is the page of Fuse array in which row has to be read
*
* @return
*	- XST_FAILURE - In case of failure
*	- XST_SUCCESS - In case of Success
*
*
* @note
*	Updates the global variable ErrorCode with error code(if any).
*	Using this API can't read AES key rows(20-27) as they can't read
*	directly.
*
*****************************************************************************/
static INLINE u8 XilSKey_EfusePl_ReadRow_Ultra(u32 Row, u8 MarginOption,
				u8 *RowDataBytes, u8 Redundant, u8 Page)
{

	/**
	 *Check if the row position is valid.
	 */
	if (PlFpgaFlag == XSK_FPGA_SERIES_ULTRA) {
		 /* If Row is AES can't read directly need to calculate CRC */
		if ((Page == XSK_EFUSEPL_PAGE_0_ULTRA) &&
			((Row >= XSK_EFUSEPL_AES_ROW_START_ULTRA) &&
			(Row <= XSK_EFUSEPL_AES_ROW_END_ULTRA))) {
			ErrorCode = XSK_EFUSEPL_ERROR_WRITE_ROW_OUT_OF_RANGE;
			return XST_FAILURE;
		}

		if(((Row > XSK_EFUSEPL_USER_ROW_ULTRA) ||
		((Row < XSK_EFUSEPL_AES_ROW_START_ULTRA) &&
		 (Row > XSK_EFUSEPL_SEC_ROW_ULTRA)) ||
		((Row < XSK_EFUSEPL_SEC_ROW_ULTRA) &&
		 (Row > XSK_EFUSEPL_DNA_ROW_ULTRA)) ||
		((Row < XSK_EFUSEPL_DNA_ROW_ULTRA) &&
		 (Row > XSK_EFUSEPL_CNTRL_ROW_ULTRA))) &&
				(Page == XSK_EFUSEPL_PAGE_0_ULTRA)) {
			ErrorCode = XSK_EFUSEPL_ERROR_READ_ROW_OUT_OF_RANGE;
			return XST_FAILURE;
		}

		if (((Row > XSK_EFUSEPL_RSA_ROW_END_ULTRA) ||
			((Row < XSK_EFUSEPL_RSA_ROW_START_ULTRA) &&
			 (Row > XSK_EFUSEPL_USER_128BIT_ROW_END_ULTRA))) &&
				(Page == XSK_EFUSEPL_PAGE_1_ULTRA)) {
			ErrorCode = XSK_EFUSEPL_ERROR_READ_ROW_OUT_OF_RANGE;
			return XST_FAILURE;
		}
	}
	if (PlFpgaFlag == XSK_FPGA_SERIES_ULTRA_PLUS) {
		 /* If Row is AES can't read directly need to calculate CRC */
		 if ((Page == XSK_EFUSEPL_PAGE_0_ULTRA) &&
			((Row >= XSK_EFUSEPL_AES_ROW_START_ULTRA_PLUS) &&
			(Row <= XSK_EFUSEPL_AES_ROW_END_ULTRA_PLUS))) {
			return XST_FAILURE;
		}
		/**
		 *Check if the row position is valid.
		 */
		if (((Row > XSK_EFUSEPL_USER_128BIT_ROW_END_ULTRA_PLUS) ||
			(Row < XSK_EFUSEPL_CNTRL_ROW_START_ULTRA_PLUS)) &&
				 (Page == XSK_EFUSEPL_PAGE_0_ULTRA)) {
			ErrorCode = XSK_EFUSEPL_ERROR_READ_ROW_OUT_OF_RANGE;
			return XST_FAILURE;
		}

		if ((Row > XSK_EFUSEPL_USER_ROW_END_ULTRA_PLUS) &&
					(Page == XSK_EFUSEPL_PAGE_1_ULTRA)) {
			ErrorCode = XSK_EFUSEPL_ERROR_READ_ROW_OUT_OF_RANGE;
			return XST_FAILURE;
		}

	}
	/**
	 * Check the read margin option.
	 */
	if((MarginOption != XSK_EFUSEPL_READ_NORMAL ) &&
		(MarginOption != XSK_EFUSEPL_READ_MARGIN_1)&&
		(MarginOption != XSK_EFUSEPL_READ_MARGIN_2))
	{
		ErrorCode = XSK_EFUSEPL_ERROR_READ_MARGIN_OUT_OF_RANGE;
		return XST_FAILURE;
	}
	/**
	 * check if row_data is not NULL
	 */
	if(NULL == RowDataBytes)
	{
		ErrorCode = XSK_EFUSEPL_ERROR_READ_BUFFER_NULL;
		return XST_FAILURE;
	}

#ifdef XSK_MICROBLAZE_PLATFORM
	XSKEfusePs_XAdc PL_XAdc_Ultra = {0};
	/**
	 * Monitor the Voltage and temperature using XADC, if out of range
	 * return unique error.
	 */
	XilSKey_EfusePs_XAdcReadTemperatureAndVoltage(&PL_XAdc_Ultra);
	if((PL_XAdc_Ultra.Temp < XSK_EFUSEPL_TEMP_MIN_RAW_ULTRA) ||
		(PL_XAdc_Ultra.Temp > XSK_EFUSEPL_TEMP_MAX_RAW_ULTRA)) {
		ErrorCode = XSK_EFUSEPL_ERROR_READ_TMEPERATURE_OUT_OF_RANGE;
		return XST_FAILURE;
	}
	if((PL_XAdc_Ultra.V < XSK_EFUSEPL_VOL_VCCAUX_MIN_RAW_ULTRA) ||
		(PL_XAdc_Ultra.V > XSK_EFUSEPL_VOL_VCCAUX_MAX_RAW_ULTRA)) {
		ErrorCode = XSK_EFUSEPL_ERROR_WRITE_VCCAUX_VOLTAGE_OUT_OF_RANGE;
		return XST_FAILURE;
	}
#endif
	/**
	 * Here we have to use the impact algorithm to read the eFUSE row.
	 * and return the data in row_data.
	 */
	if((MarginOption & XSK_EFUSEPL_READ_NORMAL) ==
					XSK_EFUSEPL_READ_NORMAL) {

		JtagRead_Ultrascale(Row, (u32 *)RowDataBytes, 0,
							Page, Redundant);
	}
	else if((MarginOption & XSK_EFUSEPL_READ_MARGIN_1) ==
					XSK_EFUSEPL_READ_MARGIN_1) {
		JtagRead_Ultrascale(Row,
			(u32 *)RowDataBytes, 1, Page, Redundant);
	}
	else if((MarginOption & XSK_EFUSEPL_READ_MARGIN_2) ==
					XSK_EFUSEPL_READ_MARGIN_2) {
		JtagRead_Ultrascale(Row, (u32 *)RowDataBytes,
				2, Page, Redundant);
	}

	return XST_SUCCESS;

}

/****************************************************************************/
/**
* Reads the bit of given normal or reduntant row of PL efuse key for Ultrascale.
*
*
* @param	Row is the row number of Fuse array.
* @param	Bit is the bit position to be read.
* @param	MarginOption is the option to select in which mode read has
*		to be happened
* @param	BitData is a pointer to a variable to store the read value
*		of given given bit in provided row.
* @param	Redundant is the option to be selected either redundant row
*		or normal row.
* @param	Page is the page of Fuse array in which row has to be read
*
* @return
*	- XST_FAILURE - In case of failure
*	- XST_SUCCESS - In case of Success
*
*
* @note
*	Updates the global variable ErrorCode with error code(if any).
*	Using this API can't read AES key bits of rows(20-27) as they can't
*	read directly.
*
*****************************************************************************/
static INLINE u8 XilSKey_EfusePl_ReadBit_Ultra(u8 Row, u8 Bit, u8 MarginOption,
				u8 *BitData, u8 Redundant, u8 Page)
{
	u8 RowData[XSK_EFUSEPL_ARRAY_MAX_COL]={0};
	u8 RowDataBits[XSK_EFUSEPL_ARRAY_MAX_COL] = {0};
	u32 Status = (u32)XST_FAILURE;

	/**
	 * check if row_data is not NULL
	 */
	if(NULL == BitData)	{
		ErrorCode = XSK_EFUSEPL_ERROR_READ_BUFFER_NULL;
		return XST_FAILURE;
	}
	/**
	 *Check if the row position is valid.
	 */
	if (PlFpgaFlag == XSK_FPGA_SERIES_ULTRA) {
		 /* If Row is AES can't read directly need to calculate CRC */
		 if ((Page == XSK_EFUSEPL_PAGE_0_ULTRA) &&
			((Row >= XSK_EFUSEPL_AES_ROW_START_ULTRA) &&
			(Row <= XSK_EFUSEPL_AES_ROW_END_ULTRA))) {
			return XST_FAILURE;
		}
		Status = XilSKey_EfusePl_Ultra_Check(Row, Bit,
						Redundant, Page);
		if (Status != XST_SUCCESS) {
			return Status;
		}
	}

	if (PlFpgaFlag == XSK_FPGA_SERIES_ULTRA_PLUS) {
		 /* If Row is AES can't read directly need to calculate CRC */
		 if ((Page == XSK_EFUSEPL_PAGE_0_ULTRA) &&
			((Row >= XSK_EFUSEPL_AES_ROW_START_ULTRA_PLUS) &&
			(Row <= XSK_EFUSEPL_AES_ROW_END_ULTRA_PLUS))) {
			 return XST_FAILURE;
		}
		Status = XilSKey_EfusePl_Ultra_Check(Row, Bit,
								Redundant, Page);
		if (Status != XST_SUCCESS) {
				return Status;
		}
	}



	if(XilSKey_EfusePl_ReadRow_Ultra(Row, MarginOption,RowData,
					Redundant, Page) != XST_SUCCESS) {
		return XST_FAILURE;
	}

	XilSKey_Efuse_ConvertBitsToBytes(RowData, RowDataBits, 32);

	if (PlFpgaFlag == XSK_FPGA_SERIES_ULTRA) {
		*BitData = RowDataBits[Bit];
	}
	if (PlFpgaFlag == XSK_FPGA_SERIES_ULTRA_PLUS) {
		if (Redundant == 1) {
			*BitData = RowDataBits[
			   XSK_EFUSEPL_ARRAY_MAX_COL_ULTRA_PLUS + Bit];
		}
		else {
			*BitData = RowDataBits[Bit];
		}
	}

	return XST_SUCCESS;
}

/****************************************************************************/
/**
* Programs the bit of given normal or reduntant row of PL efuse key for
* Ultrascale.
*
*
* @param	Row is the row number of Fuse array.
* @param	Bit is the bit position to be read.
* @param	Redundant is the option to be selected either redundant row
*		or normal row.
* @param	Page is the page of Fuse array in which row has to be read
*
* @return
*	- XST_FAILURE - In case of failure
*	- XST_SUCCESS - In case of Success
*
*
* @note
*	Updates the global variable ErrorCode with error code(if any).
*
*****************************************************************************/
static INLINE u8 XilSKey_EfusePl_ProgramBit_Ultra(u8 Row, u8 Bit, u8 Redundant, u8 Page)
{

	u8 Status = (u32)XST_FAILURE;

	Status = XilSKey_EfusePl_Ultra_Check(Row, Bit, Redundant, Page);
	if (Status != XST_SUCCESS) {
		return Status;
	}
#ifdef XSK_MICROBLAZE_PLATFORM
	 XSKEfusePs_XAdc PL_XAdc = {0};
	/**
	 * Monitor the Voltage and temperature using Sysmon, if out of range
	 * return unique error.
	 */

	XilSKey_EfusePs_XAdcReadTemperatureAndVoltage(&PL_XAdc);
	if ((PL_XAdc.Temp < XSK_EFUSEPL_TEMP_MIN_RAW_ULTRA) ||
			(PL_XAdc.Temp > XSK_EFUSEPL_TEMP_MAX_RAW_ULTRA)) {
		ErrorCode = XSK_EFUSEPL_ERROR_READ_TMEPERATURE_OUT_OF_RANGE;
		return XST_FAILURE;
	}

	if((PL_XAdc.V < XSK_EFUSEPL_VOL_VCCAUX_MIN_RAW_ULTRA) ||
		(PL_XAdc.V > XSK_EFUSEPL_VOL_VCCAUX_MAX_RAW_ULTRA)) {
		ErrorCode = XSK_EFUSEPL_ERROR_WRITE_VCCAUX_VOLTAGE_OUT_OF_RANGE;
		return XST_FAILURE;
	}
#endif
	Status = JtagWrite_Ultrascale(Row, Bit, Page, Redundant);

	if (Status != XST_SUCCESS) {
		ErrorCode = XSK_EFUSEPL_ERROR_HWM_TIMEOUT;
		return XST_FAILURE;
	}

	return XST_SUCCESS;
}
/****************************************************************************/
/**
* Programs the entire row PL efuse key for Ultrascale.
*
* @param	Row is the row number of Fuse array.
* @param	RowData is a pointer to a variable to program the row.
* @param	Redundant is the option to be selected either redundant row
*		or normal row.
* @param	Page is the page of Fuse array in which row has to be read
*
* @return
*	- XST_FAILURE - In case of failure
*	- XST_SUCCESS - In case of Success
*
*
* @note
*	Updates the global variable ErrorCode with error code(if any).
*
*****************************************************************************/
static INLINE u8 XilSKey_EfusePl_ProgramRow_Ultra(u8 Row, u8 *RowData,
						u8 Redundant, u8 Page)
{
	u32 Bit = 0;
	u32 MaxBits;
	u8 AesStartRow;
	u8 AesEndRow;

	/**
	 * check if row_data is not NULL
	 */
	if (NULL == RowData) {
		ErrorCode = XSK_EFUSEPL_ERROR_READ_BUFFER_NULL;
		return XST_FAILURE;
	}

	if (PlFpgaFlag == XSK_FPGA_SERIES_ULTRA) {
		MaxBits = XSK_EFUSEPL_MAX_BITS_IN_A_ROW_ULTRA;
		if (((Row == XSK_EFUSEPL_CNTRL_ROW_ULTRA) ||
			(Row == XSK_EFUSEPL_SEC_ROW_ULTRA)) &&
			(Page == XSK_EFUSEPL_PAGE_0_ULTRA)) {
			ErrorCode = XSK_EFUSEPL_ERROR_WRITE_ROW_OUT_OF_RANGE;
			return XST_FAILURE;
		}
		AesStartRow = XSK_EFUSEPL_AES_ROW_START_ULTRA;
		AesEndRow = XSK_EFUSEPL_AES_ROW_END_ULTRA;
	}
	if (PlFpgaFlag == XSK_FPGA_SERIES_ULTRA_PLUS) {
		MaxBits = XSK_EFUSEPL_ARRAY_MAX_COL_ULTRA_PLUS;
		if (((Row == XSK_EFUSEPL_CNTRL_ROW_START_ULTRA_PLUS) ||
			(Row == XSK_EFUSEPL_SEC_ROW_ULTRA_PLUS) ||
			(Row == XSK_EFUSEPL_CNTRL_ROW_END_ULTRA_PLUS)) &&
			(Page == XSK_EFUSEPL_PAGE_0_ULTRA)) {
			ErrorCode = XSK_EFUSEPL_ERROR_WRITE_ROW_OUT_OF_RANGE;
			return XST_FAILURE;
		}
		AesStartRow = XSK_EFUSEPL_AES_ROW_START_ULTRA_PLUS;
		AesEndRow = XSK_EFUSEPL_AES_ROW_END_ULTRA_PLUS;
	}

	for(Bit = 0; Bit < MaxBits; Bit++ ) {
		if(RowData[Bit]) {
			if(XilSKey_EfusePl_ProgramBit_Ultra(Row,
				Bit, Redundant, Page) != XST_SUCCESS) {
				return XST_FAILURE;
			}
			/*
			 * If programming bit is other than AES key's
			 * bit verify using all 3 margin reads
			 */

			if (! ((Row >= AesStartRow) &&
				(Row <= AesEndRow) &&
				(Page == XSK_EFUSEPL_PAGE_0_ULTRA))) {
#ifndef DEBUG_FUSE_WRITE_DISABLE
				if (XilSkey_EfusePl_VerifyBit_Ultra(Row,
					Bit, Redundant, Page) != XST_SUCCESS) {
					return XST_FAILURE;
				}
#else
				XilsKey_DbgPrint("Skipping XilSkey_EfusePl_VerifyBit_Ultra "
									"as DEBUG_FUSE_WRITE_DISABLE defined\n\r");
#endif
			}
		}
	}

	return XST_SUCCESS;
}

/****************************************************************************/
/**
* Verify's the AES key by passing the CRC of expected key.
*
* @param	CrcValue is CRC of expected AES key
*
* @return
*	- XST_FAILURE - In case of failure
*	- XST_SUCCESS - In case of Success
*
* @note		None.
*
*****************************************************************************/
static INLINE u8 XilSKey_EfusePl_VerifyAES_Ultrascale(u32 CrcValue)
{

	/* For Margin read 0 */
	if (JtagAES_Check_Ultrascale((u32 *)&CrcValue, 0)
							!= XST_SUCCESS) {
		return XST_FAILURE;
	}
	/* For Margin read 1 */
	if (JtagAES_Check_Ultrascale((u32 *)&CrcValue, 1)
							!= XST_SUCCESS) {
		return XST_FAILURE;
	}

	/* For Margin read 2 */
	if (JtagAES_Check_Ultrascale((u32 *)&CrcValue, 2)
							!= XST_SUCCESS) {
		return XST_FAILURE;
	}

	return XST_SUCCESS;
}

/****************************************************************************/
/**
*
* Programs PL eFUSE Secure Register of Ultrascale.
*
* @param	SecData is a pointer to which holds the data to be programmed.
*
* @return
*	- XST_FAILURE - In case of failure
*	- XST_SUCCESS - In case of Success
*
*
* @note
*	Updates the global variable ErrorCode with error code(if any).
*
*****************************************************************************/
static INLINE u8 XilSKey_EfusePl_ProgramSecRegister(u8 *SecData)
{
	u8 TempSecData[XSK_EFUSEPL_MAX_BITS_IN_A_ROW_ULTRA] = {0};
	u32 Index = 0;
	u8 SecRow;

	/**
	 * check if SecData is not NULL
	 */
	if(NULL == SecData) {
		ErrorCode = XSK_EFUSEPL_ERROR_SEC_WRITE_BUFFER_NULL;
		return XST_FAILURE;
	}

	if (PlFpgaFlag == XSK_FPGA_SERIES_ULTRA) {
		SecRow = XSK_EFUSEPL_SEC_ROW_ULTRA;
	}
	if (PlFpgaFlag == XSK_FPGA_SERIES_ULTRA_PLUS) {
		SecRow = XSK_EFUSEPL_SEC_ROW_ULTRA_PLUS;
	}

	/**
	 * check if FUSE_CNTRL allows us to write FUSE_SEC eFUSE
	 * array for Ultrascale series.
	 */
	if ((CtrlBitsUltra[XSK_EFUSEPL_CNTRL_DISABLE_SEC_RD_ULTRA]
							== TRUE) ||
		(CtrlBitsUltra[XSK_EFUSEPL_CNTRL_DISABLE_SEC_WR_ULTRA]
							== TRUE)) {
		/**
		 * This means we cannot program FUSE_CNTRL register
		 */
		ErrorCode = XSK_EFUSEPL_ERROR_FUSE_SEC_WRITE_DISABLED;
		return XST_FAILURE;
	}



	/* Read SecData */
	if (XilSKey_EfusePl_ReadSecRegister(TempSecData) != XST_SUCCESS) {
		ErrorCode = XSK_EFUSEPL_ERROR_READING_FUSE_SEC;
		return XST_FAILURE;
	}

	for (Index = 0; Index < XSK_EFUSEPL_SEC_MAX_BITS_ULTRA;
						Index++) {

		if ((SecData[Index] == TRUE) && (TempSecData[Index] == FALSE)) {
			if (XilSKey_EfusePl_ProgramBit_Ultra(
				SecRow, Index,
				XSK_EFUSEPL_NORMAL_ULTRA,
				XSK_EFUSEPL_PAGE_0_ULTRA) != XST_SUCCESS) {
				return XST_FAILURE;
			}
			if (XilSkey_EfusePl_VerifyBit_Ultra(
					SecRow,
					Index, XSK_EFUSEPL_NORMAL_ULTRA,
					XSK_EFUSEPL_PAGE_0_ULTRA) != XST_SUCCESS) {
				return XST_FAILURE;
			}

			if (XilSKey_EfusePl_ProgramBit_Ultra(
				SecRow,
				Index, XSK_EFUSEPL_REDUNDANT_ULTRA,
				XSK_EFUSEPL_PAGE_0_ULTRA) != XST_SUCCESS) {
				return XST_FAILURE;
			}
			if (XilSkey_EfusePl_VerifyBit_Ultra(
					SecRow,
					Index, XSK_EFUSEPL_REDUNDANT_ULTRA,
					XSK_EFUSEPL_PAGE_0_ULTRA) != XST_SUCCESS) {
				return XST_FAILURE;
			}

		}

	}

	return XST_SUCCESS;

}

/****************************************************************************/
/**
*
* Programs PL eFUSE reads Secure Register of Ultrascale.
*
* @param	SecData is a pointer to which holds the data read from Secure
*		register of Fuse.
*
* @return
*	- XST_FAILURE - In case of failure
*	- XST_SUCCESS - In case of Success
*
*
* @note
*	Updates the global variable ErrorCode with error code(if any).
*
*****************************************************************************/
static INLINE u8 XilSKey_EfusePl_ReadSecRegister(u8 *SecData)
{
	u32 Status = (u32)XST_FAILURE;
	u32 SecDataUltra;

	/**
	 * check if Pointer passed is not NULL
	 */
	if (NULL == SecData) {
		ErrorCode = XSK_EFUSEPL_ERROR_READ_BUFFER_NULL;
		return XST_FAILURE;
	}
	if (PlFpgaFlag == XSK_FPGA_SERIES_ZYNQ) {
		return XST_FAILURE;
	}

	/**
	 * check if FUSE_CNTRL allows us to write FUSE_SEC eFUSE
	 * array for Ultrascale series.
	 */
	if (CtrlBitsUltra[XSK_EFUSEPL_CNTRL_DISABLE_SEC_RD_ULTRA]
							== TRUE) {
		/**
		 * This means we cannot program FUSE_CNTRL register
		 */
		ErrorCode = XSK_EFUSEPL_ERROR_FUSE_SEC_READ_DISABLED;
		return XST_FAILURE;
	}
	else {
		if (PlFpgaFlag == XSK_FPGA_SERIES_ULTRA) {
			Status = XilSKey_EfusePl_GetRowData_Ultra(
					XSK_EFUSEPL_SEC_ROW_ULTRA,
				(u8 *)&SecDataUltra, XSK_EFUSEPL_PAGE_0_ULTRA);
		}
		if (PlFpgaFlag == XSK_FPGA_SERIES_ULTRA_PLUS) {
			Status = XilSKey_EfusePl_GetDataRowRange_Ultra(
				XSK_EFUSEPL_SEC_ROW_ULTRA_PLUS,
					XSK_EFUSEPL_SEC_ROW_ULTRA_PLUS,
				(u8 *)&SecDataUltra, XSK_EFUSEPL_PAGE_0_ULTRA);
		}
		if (Status != XST_SUCCESS) {
			return Status;
		}
		XilSKey_Efuse_ConvertBitsToBytes((u8 *)&SecDataUltra,
							SecData, 32);
	}

	return XST_SUCCESS;

}

/****************************************************************************/
/**
*
* Programs PL eFUSE of Zynq.
*
* @param	InstancePtr is an instance of EFuse PL of Zynq.
*
* @return
*	- XST_FAILURE - In case of failure
*	- XST_SUCCESS - In case of Success
*
*
* @note
*	Updates the global variable ErrorCode with error code(if any).
*
*****************************************************************************/
u32 XilSKey_EfusePl_Program_Zynq(XilSKey_EPl *InstancePtr)
{
	u8 RowData[XSK_EFUSEPL_MAX_BITS_IN_A_ROW_ULTRA]={0};
	u8 CtrlData[XSK_EFUSEPL_MAX_BITS_IN_A_ROW_ULTRA]={0};
	u32 Row = 0;
	u32 Index = 0;

	/* Initialize current SLR */
	XilSKeyJtag.CurSlr = 0;
	/**
	 *	Read the FUSE_CNTL register bits [5:2], and if any of them is found to
	 *	be set to 1 then we can not write to the eFUSE, so return with unique
	 *	error.
	 *
	 *	If everything is well above then start programming with FUSE_AES key as
	 *	passed to this function, followed by FUSE_USER if selected.
	 *
	 *	AES key is 256 bits, but has to be written across many rows, in the PL
	 *	eFUSE each row has 24 bits of data bits and 6 bits of ecc bits.
	 *	So for 256 bits we will need full 10 rows(240 bits) and extra 1 row for
	 *	remaining 16 bits, but rest 8 bits will be taken from the FUSE_USER key
	 *	value[7:0]
	 *
	 *	check if FUSE_CNTRL allows us to read and write the AES and USER eFUSE
	 *	array.
	 */
	if(XilSKey_EfusePl_ReadControlRegister(CtrlData) != XST_SUCCESS) {
		return (XSK_EFUSEPL_ERROR_READING_FUSE_CNTRL + ErrorCode);
	}


	if (((CtrlData[XSK_EFUSEPL_CNTRL_DISABLE_AES_KEY_READ] == TRUE) ||
		(CtrlData[XSK_EFUSEPL_CNTRL_DISABLE_USER_KEY_READ] == TRUE)||
		(CtrlData[XSK_EFUSEPL_CNTRL_DISABLE_KEY_WRITE]== TRUE)) &&
		((InstancePtr->ProgAESandUserLowKey == TRUE) ||
		(InstancePtr->ProgUserHighKey == TRUE))) {
		return (XSK_EFUSEPL_ERROR_DATA_PROGRAMMING_NOT_ALLOWED + ErrorCode);
	}

	if((CtrlData[XSK_EFUSEPL_CNTRL_DISABLE_KEY_WRITE] == TRUE) &&
		((InstancePtr->ForcePowerCycle == TRUE)||
		 (InstancePtr->KeyWrite == TRUE)||
		 (InstancePtr->AESKeyRead == TRUE)||
		 (InstancePtr->UserKeyRead == TRUE)||
		 (InstancePtr->UseAESOnly == TRUE)||
		 (InstancePtr->JtagDisable == TRUE)||
		 (InstancePtr->AESKeyExclusive == TRUE))) {
		return (XSK_EFUSEPL_ERROR_FUSE_CTRL_WRITE_NOT_ALLOWED + ErrorCode);
	}

	/**
	 * convert each aes data bits to bytes.
	 */
	XilSKey_Efuse_ConvertBitsToBytes(&InstancePtr->AESKey[0],
		AesDataInBytes, XSK_EFUSEPL_ARRAY_FUSE_AES_KEY_SIZE);

	/**
	 * convert each user data bits to bytes.
	 */
	XilSKey_Efuse_ConvertBitsToBytes(&InstancePtr->UserKey[0],
			UserDataInBytes, XSK_EFUSEPL_ARRAY_FUSE_USER_KEY_SIZE);

	if(InstancePtr->ProgAESandUserLowKey == TRUE) {
		/**
		 * check if row 20 to 30 are empty before programming AES and
		 * USER low key.
		 */
		for(Row=XSK_EFUSEPL_ARRAY_AES_DATA_ROW_START;
			Row<=XSK_EFUSEPL_ARRAY_AES_DATA_ROW_END;
			Row++) {
			if(XilSKey_EfusePl_ReadRow(Row, XSK_EFUSEPL_READ_NORMAL,
										   RowData) != XST_SUCCESS) {
				return (XSK_EFUSEPL_ERROR_READING_FUSE_AES_ROW + ErrorCode);
			}
			if(XilSKey_EfusePl_IsVectorAllZeros(RowData) != XST_SUCCESS) {
				return (XSK_EFUSEPL_ERROR_AES_ROW_NOT_EMPTY + ErrorCode);
			}
		}

		/**
		 * program AES_KEY 256 bits and USER_KEY lower 8 bits first.
		 */
		for(Row=XSK_EFUSEPL_ARRAY_AES_DATA_ROW_START;
				Row<=XSK_EFUSEPL_ARRAY_AES_DATA_ROW_END;
				Row++) {
			if(Row < XSK_EFUSEPL_ARRAY_AES_DATA_ROW_END) {
				/**
				 * prepare row data for row from 20 to 29.
				 */
				for(Index=0;
					Index<XSK_EFUSEPL_ARRAY_MAX_PAYLAOD_BITS_IN_A_ROW;
					Index++) {
						RowData[Index] = AesDataInBytes[
										 (Index +
										 ((Row -
										 XSK_EFUSEPL_ARRAY_AES_DATA_ROW_START) *
								XSK_EFUSEPL_ARRAY_MAX_PAYLAOD_BITS_IN_A_ROW))];
				}
			}
			else
			{
				/**
				 * prepare row data for row 30.
				 */
				for(Index=0;
					Index<XSK_EFUSEPL_ARRAY_MAX_PAYLAOD_BITS_IN_A_ROW;
					Index++) {
					if(Index < XSK_EFUSEPL_ARRAY_AES_DATA_BITS_IN_30th_ROW) {
						RowData[Index] =
								AesDataInBytes[
									(Index +
								((Row -
									  XSK_EFUSEPL_ARRAY_AES_DATA_ROW_START) *
								 XSK_EFUSEPL_ARRAY_MAX_PAYLAOD_BITS_IN_A_ROW))];
					}
					else {
						RowData[Index] =
								UserDataInBytes[Index -
								   XSK_EFUSEPL_ARRAY_AES_DATA_BITS_IN_30th_ROW];
					}
				}
			}

			if(XilSKey_EfusePl_ProgramRow(Row, RowData) != XST_SUCCESS) {
				return (XSK_EFUSEPL_ERROR_PROGRAMMING_FUSE_AES_ROW +
						ErrorCode);
			}
		}
	}

	if(InstancePtr->ProgUserHighKey == TRUE) {
		/**
		 * check if 31 is empty before programming USER high key.
		 */
		if(XilSKey_EfusePl_ReadRow(XSK_EFUSEPL_ARRAY_USER_DATA_START_ROW,
									   XSK_EFUSEPL_READ_NORMAL, RowData) !=
									   XST_SUCCESS) {
			return (XSK_EFUSEPL_ERROR_READING_FUSE_USER_DATA_ROW + ErrorCode);
		}


		if(XilSKey_EfusePl_IsVectorAllZeros(RowData) != XST_SUCCESS) {
			return (XSK_EFUSEPL_ERROR_USER_DATA_ROW_NOT_EMPTY + ErrorCode);
		}

		/**
		 * Program USER_KEY high 24 bits next.
		 * Prepare row data for row 31.
		 */

		for(Index=0; Index<XSK_EFUSEPL_ARRAY_MAX_PAYLAOD_BITS_IN_A_ROW; Index++) {
			RowData[Index] =
					UserDataInBytes[Index +
								  XSK_EFUSEPL_ARRAY_USER_DATA_BITS_IN_30th_ROW];
		}

		if(XilSKey_EfusePl_ProgramRow(XSK_EFUSEPL_ARRAY_USER_DATA_START_ROW,
										RowData) != XST_SUCCESS) {
			return (XSK_EFUSEPL_ERROR_PROGRAMMING_FUSE_DATA_ROW + ErrorCode);
		}
	}

	CtrlData[XSK_EFUSEPL_CNTRL_FORCE_PCYCLE_RECONFIG] = InstancePtr->ForcePowerCycle;
	CtrlData[XSK_EFUSEPL_CNTRL_DISABLE_KEY_WRITE] 	= InstancePtr->KeyWrite;
	CtrlData[XSK_EFUSEPL_CNTRL_DISABLE_AES_KEY_READ] = InstancePtr->AESKeyRead;
	CtrlData[XSK_EFUSEPL_CNTRL_DISABLE_USER_KEY_READ] = InstancePtr->UserKeyRead;
	CtrlData[XSK_EFUSEPL_CNTRL_DISABLE_FUSE_CNTRL_WRITE] = InstancePtr->CtrlWrite;
	CtrlData[XSK_EFUSEPL_CNTRL_FORCE_USE_AES_ONLY] = InstancePtr->UseAESOnly;
	CtrlData[XSK_EFUSEPL_CNTRL_JTAG_CHAIN_DISABLE] = InstancePtr->JtagDisable;
	CtrlData[XSK_EFUSEPL_CNTRL_BBRAM_KEY_DISABLE] = InstancePtr->AESKeyExclusive;

	if(XilSKey_EfusePl_ProgramControlRegister(CtrlData) != XST_SUCCESS)	{
			return (XSK_EFUSEPL_ERROR_PROGRAMMING_FUSE_CNTRL_ROW + ErrorCode);
	}

	return XST_SUCCESS;

}

#ifdef XSK_MICROBLAZE_PLATFORM
/****************************************************************************/
/**
*
* Programs PL eFUSE of Ultrascale.
*
* @param	InstancePtr is an instance of EFusePL of Ultrascale.
*
* @return
*	- XST_FAILURE - In case of failure
*	- XST_SUCCESS - In case of Success
*
*
* @note
*	Updates the global variable ErrorCode with error code(if any).
*
*****************************************************************************/
static INLINE u32 XilSKey_EfusePl_Program_Ultra(XilSKey_EPl *InstancePtr)
{
	u8 CtrlData[XSK_EFUSEPL_ARRAY_MAX_COL] = {0};
	u8 SecData[XSK_EFUSEPL_ARRAY_MAX_COL] = {0};
	u32 Status = (u32)XST_FAILURE;
	u8 User32Data[XSK_EFUSEPL_ARRAY_FUSE_USER_KEY_SIZE];
	u8 User128Data[XSK_EFUSEPL_ARRAY_FUSE_128BIT_USER_SIZE];

	XilSKey_GetSlrNum(InstancePtr->MasterSlr,
			InstancePtr->SlrConfigOrderIndex, &(XilSKeyJtag.CurSlr));

	Status = XilSKey_EfusePl_Program_Checks(InstancePtr);
	if (Status != XST_SUCCESS) {
		return Status;
	}

	/* Programming AES key */
	if (InstancePtr->ProgAESKeyUltra == TRUE) {
		/**
		 * convert each AES data bits to bytes.
		 */
		XilSKey_Efuse_ConvertBitsToBytes(&(InstancePtr->AESKey[0]),
		AesDataInBytes, XSK_EFUSEPL_ARRAY_FUSE_AES_KEY_SIZE);
		Status = XilSKey_EfusePl_Program_AesKey_ultra();
		if (Status != XST_SUCCESS) {
			return Status;
		}
		/* Verify AES key programmed */
#ifndef DEBUG_FUSE_WRITE_DISABLE
		if (XilSKey_EfusePl_VerifyAES_Ultrascale(
				InstancePtr->CrcToVerify) != XST_SUCCESS) {
			return XSK_EFUSEPL_ERROR_KEY_VALIDATION;
		}
#else
		XilsKey_DbgPrint("AES key verification failed but "
				"DEBUG_FUSE_WRITE_DISABLE set so continuing\r\n");
#endif
	}
	/* Programming USER key */
	if (InstancePtr->ProgUserKeyUltra == TRUE) {
		/**
		 * convert each user data bits to bytes.
		 */
		XilSKey_Efuse_ConvertBitsToBytes(&(InstancePtr->UserKey[0]),
			User32Data, XSK_EFUSEPL_ARRAY_FUSE_USER_KEY_SIZE);
		Status = XilSkey_EfusePl_UserFuses_TobeProgrammed(User32Data,
			UserDataInBytes, XSK_EFUSEPL_ARRAY_FUSE_USER_KEY_SIZE);
		if (Status != XST_SUCCESS) {
			return (Status + XSK_EFUSEPL_ERROR_PRGRMG_USER_KEY);
		}
		if (PlFpgaFlag == XSK_FPGA_SERIES_ULTRA) {
			Status = XilSKey_EfusePl_Program_RowRange_ultra(
				XSK_EFUSEPL_USER_ROW_ULTRA,
				XSK_EFUSEPL_USER_ROW_ULTRA, UserDataInBytes,
				XSK_EFUSEPL_PAGE_0_ULTRA);
			if (Status != XST_SUCCESS) {
				return (XSK_EFUSEPL_ERROR_PRGRMG_USER_KEY +
							Status);
			}
		}
		if (PlFpgaFlag == XSK_FPGA_SERIES_ULTRA_PLUS) {
			Status = XilSKey_EfusePl_Program_RowRange_ultra(
				XSK_EFUSEPL_USER_ROW_START_ULTRA_PLUS,
				XSK_EFUSEPL_USER_ROW_END_ULTRA_PLUS,
				UserDataInBytes,
				XSK_EFUSEPL_PAGE_1_ULTRA);
			if (Status != XST_SUCCESS) {
				return (XSK_EFUSEPL_ERROR_PRGRMG_USER_KEY +
						Status);
			}
		}

	}

	/* Programming RSA key */
	if (InstancePtr->ProgRSAKeyUltra == TRUE) {
		/**
		 * convert each RSA data bits to bytes
		 */
		XilSKey_Efuse_ConvertBitsToBytes(&(InstancePtr->RSAKeyHash[0]),
			RsaDataInBytes, XSK_EFUSEPL_RSA_HASH_SIZE_ULTRA);
		if (PlFpgaFlag == XSK_FPGA_SERIES_ULTRA) {
			Status = XilSKey_EfusePl_Program_RowRange_ultra(
					XSK_EFUSEPL_RSA_ROW_START_ULTRA,
					XSK_EFUSEPL_RSA_ROW_END_ULTRA,
				RsaDataInBytes, XSK_EFUSEPL_PAGE_1_ULTRA);
			if (Status != XST_SUCCESS) {
				return (XSK_EFUSEPL_ERROR_PRGRMG_RSA_HASH +
							Status);
			}
		}
		if (PlFpgaFlag == XSK_FPGA_SERIES_ULTRA_PLUS) {
			Status = XilSKey_EfusePl_Program_RowRange_ultra(
					XSK_EFUSEPL_RSA_ROW_START_ULTRA_PLUS,
					XSK_EFUSEPL_RSA_ROW_END_ULTRA_PLUS,
				RsaDataInBytes, XSK_EFUSEPL_PAGE_1_ULTRA);
			if (Status != XST_SUCCESS) {
				return (XSK_EFUSEPL_ERROR_PRGRMG_RSA_HASH +
						Status);
			}
		}
	}
	/* Program 128 bit User key */
	if (InstancePtr->ProgUser128BitUltra == TRUE) {
		/**
		 * convert each 128 bit user key data into bits
		 */
		XilSKey_Efuse_ConvertBitsToBytes(&(InstancePtr->User128Bit[0]),
			User128Data, XSK_EFUSEPL_ARRAY_FUSE_128BIT_USER_SIZE);
		Status = XilSkey_EfusePl_UserFuses_TobeProgrammed(User128Data,
				User128BitData,
				XSK_EFUSEPL_ARRAY_FUSE_128BIT_USER_SIZE);
		if (Status != XST_SUCCESS) {
			return (Status +
				XSK_EFUSEPL_ERROR_PRGRMG_128BIT_USER_KEY);
		}
		if (PlFpgaFlag == XSK_FPGA_SERIES_ULTRA) {
			Status = XilSKey_EfusePl_Program_RowRange_ultra(
				XSK_EFUSEPL_USER_128BIT_ROW_START_ULTRA,
				XSK_EFUSEPL_USER_128BIT_ROW_END_ULTRA,
				User128BitData, XSK_EFUSEPL_PAGE_1_ULTRA);
			if (Status != XST_SUCCESS) {
				return (Status +
				XSK_EFUSEPL_ERROR_PRGRMG_128BIT_USER_KEY);
			}
		}
		if (PlFpgaFlag == XSK_FPGA_SERIES_ULTRA_PLUS) {
			Status = XilSKey_EfusePl_Program_RowRange_ultra(
				XSK_EFUSEPL_USER_128BIT_ROW_START_ULTRA_PLUS,
				XSK_EFUSEPL_USER_128BIT_ROW_END_ULTRA_PLUS,
				User128BitData, XSK_EFUSEPL_PAGE_0_ULTRA);
			if (Status != XST_SUCCESS) {
				return (Status +
				XSK_EFUSEPL_ERROR_PRGRMG_128BIT_USER_KEY);
			}
		}
	}

	if ((InstancePtr->EncryptOnly == TRUE) ||
		(InstancePtr->UseAESOnly == TRUE) ||
		(InstancePtr->RSAEnable == TRUE) ||
		(InstancePtr->JtagDisable == TRUE) ||
		(InstancePtr->IntTestAccessDisable == TRUE) ||
		(InstancePtr->DecoderDisable == TRUE) ||
		(InstancePtr->FuseObfusEn == TRUE)) {
		/* Programming secure bits */
		SecData[XSK_EFUSEPL_SEC_ALLOW_ENCRYPT_ONLY] = InstancePtr->EncryptOnly;
		SecData[XSK_EFUSEPL_SEC_FORCE_AES_ONLY_ULTRA] =
							InstancePtr->UseAESOnly;
		SecData[XSK_EFUSEPL_SEC_RSA_AUTH_EN_ULTRA] =
							InstancePtr->RSAEnable;
		SecData[XSK_EFUSEPL_SEC_JTAG_CHAIN_DISABLE_ULTRA] =
							InstancePtr->JtagDisable;
		SecData[XSK_EFUSEPL_SEC_DISABLE_INTRNL_TEST_ACCESS_ULTRA] =
					InstancePtr->IntTestAccessDisable;
		SecData[XSK_EFUSEPL_SEC_DISABLE_DECRPTR_ULTRA] =
						InstancePtr->DecoderDisable;
		SecData[XSK_EFUSEPL_SEC_ENABLE_OBFUSCATION_ULTRA] =
						InstancePtr->FuseObfusEn;
		if(XilSKey_EfusePl_ProgramSecRegister(SecData) != XST_SUCCESS) {
			return (XSK_EFUSEPL_ERROR_PRGRMG_FUSE_SEC_ROW +
								ErrorCode);
		}
	}

	if ((InstancePtr->AESKeyRead == TRUE) ||
		(InstancePtr->UserKeyRead == TRUE) ||
		(InstancePtr->SecureRead == TRUE) ||
		(InstancePtr->CtrlWrite == TRUE) ||
		(InstancePtr->RSARead == TRUE) ||
		(InstancePtr->KeyWrite == TRUE) ||
		(InstancePtr->UserKeyWrite == TRUE) ||
		(InstancePtr->SecureWrite == TRUE) ||
		(InstancePtr->RSAWrite == TRUE) ||
		(InstancePtr->User128BitWrite == TRUE)) {
		/* Programming control bits */
		CtrlData[XSK_EFUSEPL_CNTRL_DISABLE_KEY_RD_ULTRA] =
							InstancePtr->AESKeyRead;
		CtrlData[XSK_EFUSEPL_CNTRL_DISABLE_USER_KEY_RD_ULTRA] =
							InstancePtr->UserKeyRead;
		CtrlData[XSK_EFUSEPL_CNTRL_DISABLE_SEC_RD_ULTRA] =
							InstancePtr->SecureRead;
		CtrlData[XSK_EFUSEPL_CNTRL_DISABLE_CNTRL_WR_ULTRA] =
							InstancePtr->CtrlWrite;
		CtrlData[XSK_EFUSEPL_CNTRL_DISABLE_RSA_KEY_RD_ULTRA] =
							InstancePtr->RSARead;
		CtrlData[XSK_EFUSEPL_CNTRL_DISABLE_KEY_WR_ULTRA] =
							InstancePtr->KeyWrite;
		CtrlData[XSK_EFUSEPL_CNTRL_DISABLE_USER_KEY_WR_ULTRA] =
							InstancePtr->UserKeyWrite;
		CtrlData[XSK_EFUSEPL_CNTRL_DISABLE_SEC_WR_ULTRA] =
							InstancePtr->SecureWrite;
		CtrlData[XSK_EFUSEPL_CNTRL_DISABLE_RSA_KEY_WR_ULTRA] =
							InstancePtr->RSAWrite;
		CtrlData[XSK_EFUSEPL_CNTRL_DISABLE_128BIT_USR_KEY_WR_ULTRA] =
							InstancePtr->User128BitWrite;

		if(XilSKey_EfusePl_ProgramControlRegister(CtrlData) !=
								XST_SUCCESS) {
			return (XSK_EFUSEPL_ERROR_PROGRAMMING_FUSE_CNTRL_ROW +
								ErrorCode);
		}
	}

	return XST_SUCCESS;

}
#endif

/****************************************************************************/
/**
*
* This function checks the conditions for keys programming.
*
* @param	InstancePtr is an instance of EFusePL of Ultrascale.
*
* @return
*	- ErrorCode - In case of failure
*	- XST_SUCCESS - In case of Success
*
*
* @note		None.
*
*****************************************************************************/
static INLINE u32 XilSKey_EfusePl_Program_Checks(XilSKey_EPl *InstancePtr)
{
#ifdef XSK_MICROBLAZE_ULTRA
	u32 StatusValues = 0;

	if (XilSKey_EfusePl_ReadStatus(InstancePtr,&StatusValues)
						!= XST_SUCCESS) {
		return (XSK_EFUSEPL_ERROR_READING_FUSE_STATUS +
							ErrorCode);
	}

	if ((StatusValues &
		(1 << XSK_EFUSEPL_STATUS_FUSE_LOGIC_IS_BUSY_ULTRA)) != FALSE) {
		return (XSK_EFUSEPL_ERROR_FUSE_BUSY + ErrorCode);
	}
#endif

	if (XilSKey_EfusePl_ReadControlRegister(CtrlBitsUltra)
						!= XST_SUCCESS) {
		return (XSK_EFUSEPL_ERROR_READING_FUSE_CNTRL +
							ErrorCode);
	}


	if ((((CtrlBitsUltra[XSK_EFUSEPL_CNTRL_DISABLE_KEY_RD_ULTRA] == TRUE) ||
		(CtrlBitsUltra[XSK_EFUSEPL_CNTRL_DISABLE_KEY_WR_ULTRA] == TRUE))
		&& (InstancePtr->ProgAESKeyUltra == TRUE))) {
		return (XSK_EFUSEPL_ERROR_DATA_PROGRAMMING_NOT_ALLOWED +
								ErrorCode);
	}
	if ((((CtrlBitsUltra[XSK_EFUSEPL_CNTRL_DISABLE_USER_KEY_RD_ULTRA] == TRUE) ||
		(CtrlBitsUltra[XSK_EFUSEPL_CNTRL_DISABLE_USER_KEY_WR_ULTRA] == TRUE))
			&& (InstancePtr->ProgUserKeyUltra == TRUE))) {
		return (XSK_EFUSEPL_ERROR_DATA_PROGRAMMING_NOT_ALLOWED +
								ErrorCode);
	}
	if ((((CtrlBitsUltra[XSK_EFUSEPL_CNTRL_DISABLE_RSA_KEY_RD_ULTRA] == TRUE) ||
		(CtrlBitsUltra[XSK_EFUSEPL_CNTRL_DISABLE_RSA_KEY_WR_ULTRA] == TRUE))
		&& (InstancePtr->ProgRSAKeyUltra == TRUE))) {
		return (XSK_EFUSEPL_ERROR_DATA_PROGRAMMING_NOT_ALLOWED +
								ErrorCode);
	}

	if ((CtrlBitsUltra[XSK_EFUSEPL_CNTRL_DISABLE_128BIT_USR_KEY_WR_ULTRA]
								== TRUE)
			&& (InstancePtr->ProgUser128BitUltra == TRUE)) {
		return (XSK_EFUSEPL_ERROR_DATA_PROGRAMMING_NOT_ALLOWED +
								ErrorCode);
	}

	if ((CtrlBitsUltra[XSK_EFUSEPL_CNTRL_DISABLE_CNTRL_WR_ULTRA] == TRUE) &&
		((InstancePtr->AESKeyRead == TRUE) ||
		 (InstancePtr->UserKeyRead == TRUE) ||
		 (InstancePtr->SecureRead == TRUE) ||
		 (InstancePtr->CtrlWrite == TRUE) ||
		 (InstancePtr->RSARead == TRUE) ||
		 (InstancePtr->KeyWrite == TRUE) ||
		 (InstancePtr->UserKeyWrite == TRUE) ||
		 (InstancePtr->SecureWrite == TRUE) ||
		 (InstancePtr->RSAWrite == TRUE) ||
		 (InstancePtr->User128BitWrite == TRUE))) {
		return (XSK_EFUSEPL_ERROR_FUSE_CTRL_WRITE_NOT_ALLOWED +
								ErrorCode);
	}

	if (((CtrlBitsUltra[XSK_EFUSEPL_CNTRL_DISABLE_SEC_RD_ULTRA] == TRUE) ||
		(CtrlBitsUltra[XSK_EFUSEPL_CNTRL_DISABLE_SEC_WR_ULTRA] == TRUE)) &&
		((InstancePtr->EncryptOnly == TRUE) ||
		 (InstancePtr->UseAESOnly == TRUE) ||
		 (InstancePtr->RSAEnable) ||
		 (InstancePtr->JtagDisable == TRUE) ||
		 (InstancePtr->IntTestAccessDisable == TRUE) ||
		 (InstancePtr->DecoderDisable == TRUE))) {
		return (XSK_EFUSEPL_ERROR_FUSE_SEC_WRITE_NOT_ALLOWED +
								ErrorCode);
	}

	return XST_SUCCESS;

}

/****************************************************************************/
/**
*
* Programs a range of rows for PL eFUSE of Ultrascale.
*
* @param	RowStart is the starting row to be programmed.
* @param	RowEnd is the row till which need to be programmed.
* @param	DataPrgrmg is the data to be programmed.
* @param	Page is the Page to be programmed.
*
* @return
*	- XST_FAILURE - In case of failure
*	- XST_SUCCESS - In case of Success
*
*
* @note
*	Updates the global variable ErrorCode with error code(if any).
*
*****************************************************************************/
static INLINE u32 XilSKey_EfusePl_Program_RowRange_ultra(u8 RowStart, u8 RowEnd,
						u8 *DataPrgrmg, u8 Page)
{
	u32 Row;
	u8 RowData[XSK_EFUSEPL_MAX_BITS_IN_A_ROW_ULTRA]={0};
	u32 Status = (u32)XST_FAILURE;
	u32 MaxBits;
	u8 RsaRowStart;
	u8 RsaRowEnd;

	if (PlFpgaFlag == XSK_FPGA_SERIES_ULTRA) {
		MaxBits = XSK_EFUSEPL_MAX_BITS_IN_A_ROW_ULTRA;
		RsaRowStart = XSK_EFUSEPL_RSA_ROW_START_ULTRA;
		RsaRowEnd = XSK_EFUSEPL_RSA_ROW_END_ULTRA;
	}
	if (PlFpgaFlag == XSK_FPGA_SERIES_ULTRA_PLUS) {
		MaxBits = XSK_EFUSEPL_ARRAY_MAX_COL_ULTRA_PLUS;
		RsaRowStart = XSK_EFUSEPL_RSA_ROW_START_ULTRA_PLUS;
		RsaRowEnd = XSK_EFUSEPL_RSA_ROW_END_ULTRA_PLUS;
	}


	/**
	 * check if DataPrgrmg is not NULL
	 */
	if (NULL == DataPrgrmg) {
		ErrorCode = XSK_EFUSEPL_ERROR_READ_BUFFER_NULL;
		return ErrorCode;
	}

	if (RowStart > RowEnd) {
		ErrorCode = XSK_EFUSEPL_ERROR_FUSE_ROW_RANGE;
		return ErrorCode;
	}
	/* This API will not support programming AES key rows */
	if (((RowStart == XSK_EFUSEPL_AES_ROW_START_ULTRA) ||
		(RowEnd == XSK_EFUSEPL_AES_ROW_END_ULTRA))
		&& (Page == XSK_EFUSEPL_PAGE_0_ULTRA) &&
		(PlFpgaFlag == XSK_FPGA_SERIES_ULTRA)) {
		ErrorCode = XSK_EFUSEPL_ERROR_FUSE_ROW_RANGE;
		return XST_FAILURE;

	}

	if (((RowStart == XSK_EFUSEPL_AES_ROW_START_ULTRA_PLUS) ||
		(RowEnd == XSK_EFUSEPL_AES_ROW_END_ULTRA_PLUS))
		&& (Page == XSK_EFUSEPL_PAGE_0_ULTRA) &&
		(PlFpgaFlag == XSK_FPGA_SERIES_ULTRA_PLUS)) {
		ErrorCode = XSK_EFUSEPL_ERROR_FUSE_ROW_RANGE;
		return XST_FAILURE;

	}

	if (Page > XSK_EFUSEPL_PAGE_1_ULTRA) {
		ErrorCode = XSK_EFUSEPL_ERROR_READ_PAGE_OUT_OF_RANGE;
		return ErrorCode;
	}

	/**
	 * Check is RowStart to RowEnd of given Page are empty before
	 * programming.This check is only for RSA hash programming
	 */
	if ((RowStart == RsaRowStart) &&
		(RowEnd == RsaRowEnd) &&
		(Page == XSK_EFUSEPL_PAGE_1_ULTRA)) {
		for (Row = RowStart; Row <= RowEnd; Row++) {
			Status = XilSKey_EfusePl_GetRowData_Ultra(Row,
							RowData, Page);
			if (Status != XST_SUCCESS) {
				return Status;
			}

			if(XilSKey_EfusePl_IsVectorAllZeros(RowData) !=
							XST_SUCCESS) {
				return (
				XSK_EFUSEPL_ERROR_PRGRMG_ROWS_NOT_EMPTY +
						ErrorCode);
			}
		}
	}
	/**
	 * Program rows
	 */
	for (Row = RowStart; Row <= RowEnd; Row++) {

		if(XilSKey_EfusePl_ProgramRow_Ultra(Row,
				&DataPrgrmg[(Row - RowStart) * MaxBits],
			XSK_EFUSEPL_NORMAL_ULTRA, Page) !=
						XST_SUCCESS) {
			return (XSK_EFUSEPL_ERROR_IN_PROGRAMMING_ROW +
						ErrorCode);
		}
		/* Programming redundancy bits */
		if(XilSKey_EfusePl_ProgramRow_Ultra(Row,
				&DataPrgrmg[(Row - RowStart) * MaxBits],
			XSK_EFUSEPL_REDUNDANT_ULTRA, Page) !=
						XST_SUCCESS) {
			return (XSK_EFUSEPL_ERROR_IN_PROGRAMMING_ROW +
							ErrorCode);
		}
	}

	return XST_SUCCESS;

}

/****************************************************************************/
/**
*
* Programs a AES key rows for PL eFUSE of Ultrascale.
*
* @return
*	- XST_FAILURE - In case of failure
*	- XST_SUCCESS - In case of Success
*
*
* @note
*	Updates the global variable ErrorCode with error code(if any).
*
*****************************************************************************/
static INLINE u32 XilSKey_EfusePl_Program_AesKey_ultra(void)
{

	u32 Row;
	u32 CrcOfZeros;
	u32 AesStart;
	u32 AesEnd;
	u32 MaxBits;
	/**
	 * check if row 20 to 27 are empty before programming AES Key
	 */
	if (PlFpgaFlag == XSK_FPGA_SERIES_ULTRA) {
		CrcOfZeros = XSK_EFUSEPL_CRC_FOR_AES_ZEROS;
		AesStart = XSK_EFUSEPL_AES_ROW_START_ULTRA;
		AesEnd = XSK_EFUSEPL_AES_ROW_END_ULTRA;
		MaxBits = XSK_EFUSEPL_MAX_BITS_IN_A_ROW_ULTRA;

	}
	if (PlFpgaFlag == XSK_FPGA_SERIES_ULTRA_PLUS) {
		CrcOfZeros = XSK_EFUSEPL_CRC_FOR_AES_ZEROS_ULTRA_PLUS;
		AesStart = XSK_EFUSEPL_AES_ROW_START_ULTRA_PLUS;
		AesEnd = XSK_EFUSEPL_AES_ROW_END_ULTRA_PLUS;
		MaxBits = XSK_EFUSEPL_ARRAY_MAX_COL_ULTRA_PLUS;
	}
	/**
	 * Verify AES key is zero or not before
	 * programming AES key
	 */
	if (XilSKey_EfusePl_VerifyAES_Ultrascale(CrcOfZeros) !=
					XST_SUCCESS) {
		return (XSK_EFUSEPL_ERROR_AES_ROW_NOT_EMPTY +
						ErrorCode);
	}
	/**
	 * program AES_KEY 256 bits
	 */

	for (Row = AesStart; Row <= AesEnd; Row++) {

		if (XilSKey_EfusePl_ProgramRow_Ultra(Row,
				&AesDataInBytes[(Row - AesStart) * MaxBits],
			XSK_EFUSEPL_NORMAL_ULTRA, XSK_EFUSEPL_PAGE_0_ULTRA) !=
					XST_SUCCESS) {
			return ( XSK_EFUSEPL_ERROR_PROGRAMMING_FUSE_AES_ROW +
							ErrorCode);
		}
		/* Programming redundancy bits */
		if (XilSKey_EfusePl_ProgramRow_Ultra(Row,
				&AesDataInBytes[(Row - AesStart) * MaxBits],
		XSK_EFUSEPL_REDUNDANT_ULTRA, XSK_EFUSEPL_PAGE_0_ULTRA) !=
							XST_SUCCESS) {
			return (XSK_EFUSEPL_ERROR_PROGRAMMING_FUSE_AES_ROW
						+ ErrorCode);
		}
	}

	return (XST_SUCCESS);

}

/****************************************************************************/
/**
*
* This function gets the row data of Ored Normal and Redundant rows for EFUSE
* of Ultrascale and ultrascale plus.
* For ultrascale plus, read row contains both redundant and normal bits
* upper 16 bits contains redundant bits and lower 16 bits contain normal
* bits, each row will have only 16 valid bits.
*
* @param	Row is the row number.
* @param	RowData is the ORed result of both rows.
* @param	Page indicates row belongs to which page.
*
* @return
*	- XST_FAILURE - In case of failure
*	- XST_SUCCESS - In case of Success
*
*
* @note
*	Updates the global variable ErrorCode with error code(if any).
*
*****************************************************************************/
static INLINE u32 XilSKey_EfusePl_ReadRowData_Ultra(u8 Row,
						u8 *RowData, u8 Page)
{
	u32 Column;
	u8 RowDataRedundant[XSK_EFUSEPL_MAX_BITS_IN_A_ROW_ULTRA] = {0};

	if (PlFpgaFlag == XSK_FPGA_SERIES_ULTRA) {
		if (XilSKey_EfusePl_ReadRow_Ultra(Row,
			XSK_EFUSEPL_READ_NORMAL, RowData,
			XSK_EFUSEPL_NORMAL_ULTRA, Page) !=
					XST_SUCCESS) {
			return ErrorCode;
		}

		if (XilSKey_EfusePl_ReadRow_Ultra(Row,
			XSK_EFUSEPL_READ_NORMAL, RowDataRedundant,
			XSK_EFUSEPL_REDUNDANT_ULTRA, Page) !=
					XST_SUCCESS) {
			return ErrorCode;
		}

		for (Column = 0;
			Column < XSK_EFUSEPL_ARRAY_MAX_COL;
						Column++) {
			RowData[Column] = RowData[Column] |
					RowDataRedundant[Column];
		}
	}

	if (PlFpgaFlag == XSK_FPGA_SERIES_ULTRA_PLUS) {
		if (XilSKey_EfusePl_ReadRow_Ultra(Row,
			XSK_EFUSEPL_READ_NORMAL, RowData,
			XSK_EFUSEPL_NORMAL_ULTRA, Page) !=
					XST_SUCCESS) {
			return ErrorCode;
		}
	}

	return XST_SUCCESS;

}

/****************************************************************************/
/**
*
* This API gets the row data of Ored Normal and Redundant rows for EFUSE of
* Ultrascale.
*
* @param	Row is the row number.
* @param	RowData is the ORed result of both rows.
* @param	Page indicates row belongs to which page.
*
* @return
*	- XST_FAILURE - In case of failure
*	- XST_SUCCESS - In case of Success
*
*
* @note
*	Updates the global variable ErrorCode with error code(if any).
*
*****************************************************************************/
static INLINE u32 XilSKey_EfusePl_GetRowData_Ultra(u8 Row, u8 *RowData, u8 Page)
{

	if (((Row >= XSK_EFUSEPL_AES_ROW_START_ULTRA) &&
		(Row <= XSK_EFUSEPL_AES_ROW_END_ULTRA)) &&
			(Page == XSK_EFUSEPL_PAGE_0_ULTRA) &&
			(PlFpgaFlag == XSK_FPGA_SERIES_ULTRA)) {
		ErrorCode = XSK_EFUSEPL_ERROR_WRITE_ROW_OUT_OF_RANGE;
		return ErrorCode;
	}

	if (((Row >= XSK_EFUSEPL_AES_ROW_START_ULTRA_PLUS) &&
			(Row <= XSK_EFUSEPL_AES_ROW_END_ULTRA_PLUS)) &&
				(Page == XSK_EFUSEPL_PAGE_0_ULTRA) &&
				(PlFpgaFlag == XSK_FPGA_SERIES_ULTRA_PLUS)) {
			ErrorCode = XSK_EFUSEPL_ERROR_WRITE_ROW_OUT_OF_RANGE;
			return ErrorCode;
		}

	/**
	 * check if RowData is not NULL
	 */
	if (NULL == RowData) {
		ErrorCode = XSK_EFUSEPL_ERROR_READ_BUFFER_NULL;
		return ErrorCode;
	}

	if (Page > XSK_EFUSEPL_PAGE_1_ULTRA) {
		ErrorCode = XSK_EFUSEPL_ERROR_READ_PAGE_OUT_OF_RANGE;
		return ErrorCode;
	}

	ErrorCode = XilSKey_EfusePl_ReadRowData_Ultra(Row, RowData, Page);
	if (ErrorCode != XST_SUCCESS) {
		return ErrorCode;
	}

	return XST_SUCCESS;

}

/****************************************************************************/
/**
*
* This API gets the rows data of given range of FUSE's array of Ultrascale
*
* @param	RowStart is the starting row to be programmed.
* @param	RowEnd is the row till which need to be programmed.
* @param	KeyRead is a pointer of array in which read data is updated.
* @param	Page is the Page to be programmed.
*
* @return
*	- XST_FAILURE - In case of failure
*	- XST_SUCCESS - In case of Success
*
*
* @note
*	Updates the global variable ErrorCode with error code(if any).
*	Reads all the margin options.
*
*****************************************************************************/
static INLINE u32 XilSKey_EfusePl_GetDataRowRange_Ultra(u8 RowStart, u8 RowEnd,
						u8 *KeyRead, u8 Page)
{
	u8 RowData[4] = {0};
	u8 Row;
	u8 KeyCnt = 0;
	u8 Index = 0;

	if (RowStart > RowEnd) {
		ErrorCode = XSK_EFUSEPL_ERROR_FUSE_ROW_RANGE;
		return ErrorCode;
	}

	for (Row = RowStart; Row <= RowEnd; Row++) {
		if (XilSKey_EfusePl_GetRowData_Ultra(Row, RowData, Page)
						!= XST_SUCCESS) {
			return ErrorCode;
		}
		Index = 0;

		if (PlFpgaFlag == XSK_FPGA_SERIES_ULTRA) {
			KeyRead[KeyCnt++] = RowData[Index++] & 0xFF;
			KeyRead[KeyCnt++] = RowData[Index++] & 0xFF;
			KeyRead[KeyCnt++] = RowData[Index++] & 0xFF;
			KeyRead[KeyCnt++] = RowData[Index] & 0xFF;
		}
		if (PlFpgaFlag == XSK_FPGA_SERIES_ULTRA_PLUS) {
			KeyRead[KeyCnt++] = RowData[0] | RowData[2];
			KeyRead[KeyCnt++] = RowData[1] | RowData[3];
		}

	}

	return XST_SUCCESS;

}

/****************************************************************************/
/**
*
* Programs PL eFUSE Control Register of Ultrascale.
*
*
*
* @param	CtrlData - Control data pointer
*
* @return
*
*	- XST_FAILURE - In case of failure
*	- XST_SUCCESS - In case of Success
*
*
* @note		Updates the global variable ErrorCode with error code(if any).
*
*****************************************************************************/
static INLINE u8 XilSKey_EfusePl_ProgramControlReg_Ultra(u8 *CtrlData)
{
	u32 Index;

	/**
	 * check if FUSE_CNTRL allows us to write FUSE_CNTRL eFUSE
	 * array for Ultrascale series.
	 */
	if (CtrlBitsUltra[XSK_EFUSEPL_CNTRL_DISABLE_CNTRL_WR_ULTRA]
							== TRUE) {
		/**
		 * This means we cannot program FUSE_CNTRL register
		 */
		ErrorCode = XSK_EFUSEPL_ERROR_FUSE_CNTRL_WRITE_DISABLED;
		return XST_FAILURE;
	}

	for(Index = 0; Index < XSK_EFUSEPL_CNTRL_MAX_BITS_ULTRA;
							Index++) {

		if((Index == XSK_EFUSEPL_CTRL_ROW_UNSUPPORT_BIT3_ULTRA) ||
		(Index == XSK_EFUSEPL_CTRL_ROW_UNSUPPORT_BIT4_ULTRA) ||
		((Index >= XSK_EFUSEPL_CTRL_ROW_UNSUPPORT_BIT_RANGE_START_ULTRA)
		&& (Index <
			XSK_EFUSEPL_CTRL_ROW_UNSUPPORT_BIT_RANGE_END_ULTRA))) {
			continue;
		}

		if((CtrlData[Index] == TRUE) &&
			(CtrlBitsUltra[Index] == FALSE)) {
			if(XilSKey_EfusePl_ProgramBit_Ultra(
				XSK_EFUSEPL_CNTRL_ROW_ULTRA,
				Index, XSK_EFUSEPL_NORMAL_ULTRA,
				XSK_EFUSEPL_PAGE_0_ULTRA) != XST_SUCCESS) {
				return XST_FAILURE;
			}
			if (XilSkey_EfusePl_VerifyBit_Ultra(
				XSK_EFUSEPL_CNTRL_ROW_ULTRA,
				Index, XSK_EFUSEPL_NORMAL_ULTRA,
				XSK_EFUSEPL_PAGE_0_ULTRA) != XST_SUCCESS) {
				return XST_FAILURE;
			}

			if(XilSKey_EfusePl_ProgramBit_Ultra(
				XSK_EFUSEPL_CNTRL_ROW_ULTRA,
				Index, XSK_EFUSEPL_REDUNDANT_ULTRA,
				XSK_EFUSEPL_PAGE_0_ULTRA) != XST_SUCCESS) {
				return XST_FAILURE;
			}
			if (XilSkey_EfusePl_VerifyBit_Ultra(
				XSK_EFUSEPL_CNTRL_ROW_ULTRA,
				Index, XSK_EFUSEPL_REDUNDANT_ULTRA,
				XSK_EFUSEPL_PAGE_0_ULTRA) != XST_SUCCESS) {
				return XST_FAILURE;
			}

		}
	}

	return XST_SUCCESS;

}

/****************************************************************************/
/**
*
* Verifies PL eFUSE bit of Ultrascale.
*
* @param	Row specifies the row number to be verified.
* @param	Bit specifies bit number to be verified.
* @param	Redundant is the option to be selected either redundant row
*			or normal row.
* @param	Page is the page of Fuse array in which row has to be read
*
* @return
*
*	- XST_FAILURE - In case of failure
*	- XST_SUCCESS - In case of Success
*
*
* @note		None.
*
*****************************************************************************/
static INLINE u8 XilSkey_EfusePl_VerifyBit_Ultra(u8 Row, u8 Bit, u8 Redundant,
								u8 Page)
{
	u8 BitData = 0;

	/* Normal read verification */
	if (XilSKey_EfusePl_ReadBit_Ultra(Row, Bit, XSK_EFUSEPL_READ_NORMAL,
					&BitData, Redundant, Page) != XST_SUCCESS) {
		return XST_FAILURE;
	}
	if (BitData != 0x1) {
		return XST_FAILURE;
	}
	BitData = 0;
	/* Margin 1 read verification */
	if (XilSKey_EfusePl_ReadBit_Ultra(Row, Bit, XSK_EFUSEPL_READ_MARGIN_1,
			&BitData, Redundant, Page) != XST_SUCCESS) {
		return XST_FAILURE;
	}
	if (BitData != 0x1) {
		return XST_FAILURE;
	}
	BitData = 0;
	/* Margin 2 read verification */
	if (XilSKey_EfusePl_ReadBit_Ultra(Row, Bit, XSK_EFUSEPL_READ_MARGIN_2,
			&BitData, Redundant, Page) != XST_SUCCESS) {
		return XST_FAILURE;
	}
	if (BitData != 0x1) {
		return XST_FAILURE;
	}

	return XST_SUCCESS;

}

/*****************************************************************************/
/**
* This function throws an error if user requests already programmed User FUSE
* bit to revert, and copies the bits to be programmed in particular row into
* provided UserFuses_TobePrgrmd pointer.
*
* @param	UserFuses_Write is a pointer to user requested programming bits
*		of an User FUSE row.
* @param	UserFuses_TobePrgrmd holds User FUSE row bits which needs to be
*		programmed actually.
* @param	Size specifies the User key size.
*
* @return
*		- XST_FAILURE: Returns error if user requests programmed bit to
*		revert
*		- XST_SUCCESS: If User requests valid bits.
*
* @note		If user requests a non-zero bit for making to zero throws an
*		error which is not possible.
*
******************************************************************************/
static INLINE u32 XilSkey_EfusePl_UserFuses_TobeProgrammed(
		u8 *UserFuses_Write, u8 *UserFuses_TobePrgrmd, u8 Size)
{
	u32 UserFuseColumn;
	u8 UserFuses_Read[XSK_EFUSEPL_ARRAY_FUSE_128BIT_USER_SIZE];
	u8 ReadData[XSK_EFUSEPL_128BIT_USERKEY_SIZE_IN_BYTES];
	u32 Status = (u32)XST_FAILURE;

	/* Read 128 bit User Key */
	if (Size == XSK_EFUSEPL_ARRAY_FUSE_128BIT_USER_SIZE) {
		if (PlFpgaFlag == XSK_FPGA_SERIES_ULTRA) {
			Status = XilSKey_EfusePl_GetDataRowRange_Ultra(
				XSK_EFUSEPL_USER_128BIT_ROW_START_ULTRA,
				XSK_EFUSEPL_USER_128BIT_ROW_END_ULTRA, ReadData,
						XSK_EFUSEPL_PAGE_1_ULTRA);
			if (Status != XST_SUCCESS) {
				return Status;
			}
		}
		if (PlFpgaFlag == XSK_FPGA_SERIES_ULTRA_PLUS) {
			Status = XilSKey_EfusePl_GetDataRowRange_Ultra(
				XSK_EFUSEPL_USER_128BIT_ROW_START_ULTRA_PLUS,
				XSK_EFUSEPL_USER_128BIT_ROW_END_ULTRA_PLUS,
				ReadData, XSK_EFUSEPL_PAGE_0_ULTRA);
			if (Status != XST_SUCCESS) {
				return Status;
			}
		}

		XilSKey_Efuse_ConvertBitsToBytes(ReadData, UserFuses_Read,
				XSK_EFUSEPL_ARRAY_FUSE_128BIT_USER_SIZE);
	}

	/* Read 32 bit User key */
	if (Size == XSK_EFUSEPL_ARRAY_FUSE_USER_KEY_SIZE) {
		if (PlFpgaFlag == XSK_FPGA_SERIES_ULTRA) {
			Status = XilSKey_EfusePl_GetDataRowRange_Ultra(
				XSK_EFUSEPL_USER_ROW_ULTRA,
				XSK_EFUSEPL_USER_ROW_ULTRA, ReadData,
					XSK_EFUSEPL_PAGE_0_ULTRA);
			if (Status != XST_SUCCESS) {
				return Status;
			}
		}
		if (PlFpgaFlag == XSK_FPGA_SERIES_ULTRA_PLUS) {
			Status = XilSKey_EfusePl_GetDataRowRange_Ultra(
				XSK_EFUSEPL_USER_ROW_START_ULTRA_PLUS,
				XSK_EFUSEPL_USER_ROW_END_ULTRA_PLUS, ReadData,
					XSK_EFUSEPL_PAGE_1_ULTRA);
			if (Status != XST_SUCCESS) {
				return Status;
			}

		}
		XilSKey_Efuse_ConvertBitsToBytes(ReadData, UserFuses_Read,
				XSK_EFUSEPL_ARRAY_FUSE_USER_KEY_SIZE);
	}


	for (UserFuseColumn = 0; UserFuseColumn < Size; UserFuseColumn++) {
	/* If user requests a non-zero bit for making to zero throws an error*/
		if ((UserFuses_Write[UserFuseColumn] == 0) &&
			(UserFuses_Read[UserFuseColumn] == 1)) {
			return XSK_EFUSEPL_ERROR_USER_FUSE_REVERT;
		}
		if ((UserFuses_Write[UserFuseColumn] == 1) &&
			(UserFuses_Read[UserFuseColumn] == 0)) {
			UserFuses_TobePrgrmd[UserFuseColumn] = 1;
		}else {
            UserFuses_TobePrgrmd[UserFuseColumn] = 0;
       }

	}

	return XST_SUCCESS;
}

/****************************************************************************/
/**
* This API checks whether the parameters passed for programming or reading
* is correct.
*
* @param	Row is the row number of Fuse array.
* @param	Bit is the bit position to be read.
* @param	Redundant is the option to be selected either redundant row
*		or normal row.
* @param	Page is the page of Fuse array in which row has to be read
*
* @return
*	- XST_FAILURE - In case of failure
*	- XST_SUCCESS - In case of Success
*
*
* @note
*	Updates the global variable ErrorCode with error code(if any).
*
*****************************************************************************/
static INLINE u8 XilSKey_EfusePl_Ultra_Check(u8 Row,
		u8 Bit, u8 Redundant, u8 Page)
{
	if (Redundant > XSK_EFUSEPL_REDUNDANT_ULTRA) {
		return XST_FAILURE;
	}
	if (PlFpgaFlag == XSK_FPGA_SERIES_ULTRA) {
		/**
		 *Check if the row position is valid.
		 */
		if (((Row > XSK_EFUSEPL_USER_ROW_ULTRA) ||
			((Row < XSK_EFUSEPL_AES_ROW_START_ULTRA) &&
			(Row > XSK_EFUSEPL_SEC_ROW_ULTRA)) ||
			((Row < XSK_EFUSEPL_SEC_ROW_ULTRA) &&
			 (Row > XSK_EFUSEPL_DNA_ROW_ULTRA)) ||
			 ((Row < XSK_EFUSEPL_DNA_ROW_ULTRA) &&
			 (Row > XSK_EFUSEPL_CNTRL_ROW_ULTRA))) &&
				 (Page == XSK_EFUSEPL_PAGE_0_ULTRA)) {
			ErrorCode = XSK_EFUSEPL_ERROR_READ_ROW_OUT_OF_RANGE;
			return XST_FAILURE;
		}

		if (((Row > XSK_EFUSEPL_RSA_ROW_END_ULTRA) ||
			((Row < XSK_EFUSEPL_RSA_ROW_START_ULTRA) &&
			 (Row > XSK_EFUSEPL_USER_128BIT_ROW_END_ULTRA))) &&
					(Page == XSK_EFUSEPL_PAGE_1_ULTRA)) {
			ErrorCode = XSK_EFUSEPL_ERROR_READ_ROW_OUT_OF_RANGE;
			return XST_FAILURE;
		}

		/**
		 * Check if the bit position is valid.
		 */
		if (Bit > XSK_EFUSEPL_END_BIT_IN_A_ROW_ULTRA) {
			ErrorCode = XSK_EFUSEPL_ERROR_WRITE_BIT_OUT_OF_RANGE;
			return XST_FAILURE;
		}

		/**
		 * If row = 1 then bit should be either 0 to 2 and 5 to 9 and 15
		 * rest all are not supported
		 */
		if((Row == XSK_EFUSEPL_CNTRL_ROW_ULTRA) &&
				(Page == XSK_EFUSEPL_PAGE_0_ULTRA)) {
			if((Bit == XSK_EFUSEPL_CTRL_ROW_UNSUPPORT_BIT3_ULTRA) ||
			(Bit == XSK_EFUSEPL_CTRL_ROW_UNSUPPORT_BIT4_ULTRA) ||
			((Bit >= XSK_EFUSEPL_CTRL_ROW_UNSUPPORT_BIT_RANGE_START_ULTRA) &&
			(Bit <= XSK_EFUSEPL_CTRL_ROW_UNSUPPORT_BIT_RANGE_END_ULTRA)) ||
			(Bit > XSK_EFUSEPL_CTRL_ROW_END_BIT_ULTRA)) {
				ErrorCode = XSK_EFUSEPL_ERROR_WRITE_BIT_OUT_OF_RANGE;
				return XST_FAILURE;
			}
		}
		/**
		 * If row = 10 then bits should be supported from 0 to 5
		 */
		 if ((Row == XSK_EFUSEPL_SEC_ROW_ULTRA) &&
			 (Bit > XSK_EFUSEPL_SEC_ROW_END_BIT_ULTRA) &&
			 (Page == XSK_EFUSEPL_PAGE_0_ULTRA)) {
			ErrorCode = XSK_EFUSEPL_ERROR_WRITE_BIT_OUT_OF_RANGE;
				return XST_FAILURE;
		 }
	}
	if (PlFpgaFlag == XSK_FPGA_SERIES_ULTRA_PLUS) {
		/**
		 *Check if the row position is valid.
		 */
		if (((Row > XSK_EFUSEPL_USER_128BIT_ROW_END_ULTRA_PLUS) ||
			(Row < XSK_EFUSEPL_CNTRL_ROW_START_ULTRA_PLUS)) &&
				 (Page == XSK_EFUSEPL_PAGE_0_ULTRA)) {
			ErrorCode = XSK_EFUSEPL_ERROR_READ_ROW_OUT_OF_RANGE;
			return XST_FAILURE;
		}

		if ((Row > XSK_EFUSEPL_USER_ROW_END_ULTRA_PLUS) &&
					(Page == XSK_EFUSEPL_PAGE_1_ULTRA)) {
			ErrorCode = XSK_EFUSEPL_ERROR_READ_ROW_OUT_OF_RANGE;
			return XST_FAILURE;
		}

		/**
		 * Check if the bit position is valid.
		 */
		if (Bit > XSK_EFUSEPL_ARRAY_MAX_COL_ULTRA_PLUS) {
			ErrorCode = XSK_EFUSEPL_ERROR_WRITE_BIT_OUT_OF_RANGE;
			return XST_FAILURE;
		}

		/**
		 * If row = 2 or 3 then bit should be either 0 to 2 and 5 to 9 and 15
		 * rest all are not supported
		 */
		if((Row == XSK_EFUSEPL_CNTRL_ROW_START_ULTRA_PLUS) &&
				(Page == XSK_EFUSEPL_PAGE_0_ULTRA)) {
			if((Bit == XSK_EFUSEPL_CTRL_ROW_UNSUPPORT_BIT3_ULTRA) ||
			(Bit == XSK_EFUSEPL_CTRL_ROW_UNSUPPORT_BIT4_ULTRA) ||
			((Bit >= XSK_EFUSEPL_CTRL_ROW_UNSUPPORT_BIT_RANGE_START_ULTRA) &&
			(Bit <= XSK_EFUSEPL_CTRL_ROW_UNSUPPORT_BIT_RANGE_END_ULTRA)) ||
			(Bit >= XSK_EFUSEPL_CTRL_ROW_END_BIT_ULTRA)) {
				ErrorCode = XSK_EFUSEPL_ERROR_WRITE_BIT_OUT_OF_RANGE;
				return XST_FAILURE;
			}
		}
		if((Row == XSK_EFUSEPL_CNTRL_ROW_END_ULTRA_PLUS) &&
			(Page == XSK_EFUSEPL_PAGE_0_ULTRA)) {
			if(Bit != 0) {
				ErrorCode = XSK_EFUSEPL_ERROR_WRITE_BIT_OUT_OF_RANGE;
				return XST_FAILURE;
			}
		}
		/**
		 * If row = 4 then bits should be supported from 0 to 5
		 */
		 if ((Row == XSK_EFUSEPL_SEC_ROW_ULTRA_PLUS) &&
			 (Bit > XSK_EFUSEPL_SEC_ROW_END_BIT_ULTRA) &&
			 (Page == XSK_EFUSEPL_PAGE_0_ULTRA)) {
			ErrorCode = XSK_EFUSEPL_ERROR_WRITE_BIT_OUT_OF_RANGE;
			return XST_FAILURE;
		 }

	}

	return XST_SUCCESS;
}

/****************************************************************************/
/**
*
* Programs PL eFUSE Control Register of Ultrascale.
*
*
*
* @param	CtrlData - Control data pointer
*
* @return
*
*	- XST_FAILURE - In case of failure
*	- XST_SUCCESS - In case of Success
*
*
* @note		Updates the global variable ErrorCode with error code(if any).
*
*****************************************************************************/
static INLINE u8 XilSKey_EfusePl_ProgramControlReg_Ultra_Plus(u8 *CtrlData)
{
	u32 Index;
	u32 BitPos;
	u8 Row;

	/**
	 * check if FUSE_CNTRL allows us to write FUSE_CNTRL eFUSE
	 * array for Ultrascale series.
	 */
	if (CtrlBitsUltra[XSK_EFUSEPL_CNTRL_DISABLE_CNTRL_WR_ULTRA]
							== TRUE) {
		/**
		 * This means we cannot program FUSE_CNTRL register
		 */
		ErrorCode = XSK_EFUSEPL_ERROR_FUSE_CNTRL_WRITE_DISABLED;
		return XST_FAILURE;
	}

	for(Index = 0; Index < XSK_EFUSEPL_CNTRL_MAX_BITS_ULTRA;
							Index++) {
		if (Index >= XSK_EFUSEPL_ARRAY_MAX_COL_ULTRA_PLUS) {
			BitPos = Index - XSK_EFUSEPL_ARRAY_MAX_COL_ULTRA_PLUS;
			Row = XSK_EFUSEPL_CNTRL_ROW_END_ULTRA_PLUS;
		}
		else {
			BitPos = Index;
			Row = XSK_EFUSEPL_CNTRL_ROW_START_ULTRA_PLUS;
		}

		if((Index == XSK_EFUSEPL_CTRL_ROW_UNSUPPORT_BIT3_ULTRA) ||
		(Index == XSK_EFUSEPL_CTRL_ROW_UNSUPPORT_BIT4_ULTRA) ||
		((Index >= XSK_EFUSEPL_CTRL_ROW_UNSUPPORT_BIT_RANGE_START_ULTRA)
		&& (Index <
			XSK_EFUSEPL_CTRL_ROW_UNSUPPORT_BIT_RANGE_END_ULTRA))) {
			continue;
		}

		if((CtrlData[Index] == TRUE) &&
			(CtrlBitsUltra[Index] == FALSE)) {
			if(XilSKey_EfusePl_ProgramBit_Ultra(
				Row,
				BitPos, XSK_EFUSEPL_NORMAL_ULTRA,
				XSK_EFUSEPL_PAGE_0_ULTRA) != XST_SUCCESS) {
				return XST_FAILURE;
			}
			if (XilSkey_EfusePl_VerifyBit_Ultra(
				Row,
				BitPos, XSK_EFUSEPL_NORMAL_ULTRA,
				XSK_EFUSEPL_PAGE_0_ULTRA) != XST_SUCCESS) {
				return XST_FAILURE;
			}

			if(XilSKey_EfusePl_ProgramBit_Ultra(
				Row,
				BitPos, XSK_EFUSEPL_REDUNDANT_ULTRA,
				XSK_EFUSEPL_PAGE_0_ULTRA) != XST_SUCCESS) {
				return XST_FAILURE;
			}
			if (XilSkey_EfusePl_VerifyBit_Ultra(
				Row,
				BitPos, XSK_EFUSEPL_REDUNDANT_ULTRA,
				XSK_EFUSEPL_PAGE_0_ULTRA) != XST_SUCCESS) {
				return XST_FAILURE;
			}

		}
	}

	return XST_SUCCESS;

}
