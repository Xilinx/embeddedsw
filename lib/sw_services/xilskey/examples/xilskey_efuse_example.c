/******************************************************************************
*
* Copyright (C) 2013 - 2014 Xilinx, Inc.  All rights reserved.
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
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
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
* @file
* 			xilskey_efuse_example.c
* @note
*
*  		Contains the api functions for the PS & PL eFUSE functionality.
*  		eFUSE Application project is capable of programming the PS and PL eFUSE
*  		bits given by the user. PS eFUSE holds the RSA primary key hash bits and
*  		user feature bits, which will enable/disable some features in ZYNQ.
*  		PL eFUSE holds the AES key, user key and some feature bits.
*  		User has the provision to write PS eFUSE & PL eFUSE independently or
*  		can combine together. This can be selected by using the compilation
*  		switch provided in xilskey_input.h. XSK_EFUSEPS_DRIVER should be
*  		defined to enable PS functionality & XSK_EFUSEPL_DRIVER for PL
*  		functionality.
*
*		eFUSE bits are one-time programmable. Once they are burnt, they
*		cannot be changed. Make sure you enter the correct information before
*		you program eFUSE bits.
*
*		POR reset is required for the eFUSE values to become into effect.
*		Please do a POR reset after eFUSE writing.
*
*  		All the user configurable parameters for PS & PL eFUSE writing should be
*  		defined in xilskey_input.h. By default, all the macros will be defined
*  		with FALSE values.
*
*  		For PL eFUSE writing enabling the caches are necessary if the image is
*  		executing from DDR. This will be done in  BSP by default. User has to
*  		take care not to disable caches.
*
*		eFUSE writing procedure running out of DDR as an application:
*		(This sequence is same as the existing flow described below)
*
*		1)	After providing the required inputs in xilskey_input.h, compile the
*		   	project.
*		2)	Take the latest FSBL (.elf) and stitch the <output>.elf	generated
*			to it using the bootgen utility and generate a bootable	image.
*		3)	Write the generated binary image into the flash device.
*			(Ex: QSPI,NAND etc)
*		4)	Execute image from flash which will write the mentioned eFUSE key
*			bits.
*
*  		eFUSE driver compilation procedure for OCM:
*
*		1)	Open the linker script (lscript.ld) in the SDK project.
*		2)	Now map all the sections points to “ps7_ram_0_S_AXI_BASEADDR”
*			instead of “ps7_ddr_0_S_AXI_BASEADDR”.
*
*		Example: 	Click on the “Memory Region” tab for .text section & select
*					“ps7_ram_0_S_AXI_BASEADDR” from the drop down list.
*
*		3)	Copy the ps7_init.c & ps7_init.h files from the hw_platform folder
*			into the example folder.
*		4)	Uncomment calling of “ps7_init()” routine in
*			“xilskey_efuse_example.c”
*		5)	Compile the project.
*		6)	<project name>.elf  will be generated. This will be executed
*			out of OCM.
*
*		SVF File Generation using <output>.elf :
*
*		1)	Use the below xmd to create the svf file from the above generated
*			elf file. Please note that path to the elf file should be given in opt file.
*
*						“xmd –tcl efuse.tcl –opt efuse.opt”
*
*		2)	Output of the above command will be <O/p file name>.svf.
*
*
* MODIFICATION HISTORY:
*
* Ver   Who  	Date     Changes
* ----- ---- 	-------- --------------------------------------------------------
* 1.00a rpoolla 04/26/13 First release
* 1.02a hk      10/28/13 Added use of API's read status and key. PR# 735957.
*
*
****************************************************************************/
/***************************** Include Files *********************************/
#include "stdio.h"
#include "xil_io.h"
#include "xstatus.h"
#include "xilskey_utils.h"
#include "xilskey_eps.h"
#include "xilskey_epl.h"
#include "xilskey_input.h"
/************************** Constant Definitions *****************************/
/**************************** Type Definitions ******************************/
/***************** Macros (Inline Functions) Definitions ********************/
/**
 * Address for the Reboot Status Register
 */
#define REBOOT_STATUS_REG_ADDR 						(0xF8000258)
/**
 *  PL eFUSE aes key size in characters
 */
#define XSK_EFUSEPL_AES_KEY_STRING_SIZE              (64)
/**
 *  PL eFUSE user low key size in characters
 */
#define XSK_EFUSEPL_USER_LOW_KEY_STRING_SIZE         (2)
/**
 *  PL eFUSE user high key size in characters
 */
#define XSK_EFUSEPL_USER_HIGH_KEY_STRING_SIZE        (6)
/**
 *  User AES Key size in Bytes
 */
#define XSK_EFUSEPL_AES_KEY_SIZE_IN_BITS			(256)
/**
 *  User Low Key size in Bytes
 */
#define XSK_EFUSEPL_USER_LOW_KEY_SIZE_IN_BITS		(8)
/**
 *  User High Key size in Bytes
 */
#define XSK_EFUSEPL_USER_HIGH_KEY_SIZE_IN_BITS		(24)
/**
 * Key length definition for RSA KEY Hash
 */
/**
 *  PS eFUSE RSA key Hash size in characters
 */
#define XSK_EFUSEPL_RSA_KEY_HASH_STRING_SIZE        (64)

/*
 * PS efuse status bit definitions
 */
#define XSK_EFUSEPS_STATUS_WP_BIT_LOW	0x1000
#define XSK_EFUSEPS_STATUS_WP_BIT_HIGH	0x2000
#define XSK_EFUSEPS_STATUS_RSA_EN		0x400
#define XSK_EFUSEPS_STATUS_ROM_128_CRC	0x800

/*
 * PL efuse status bit definitions
 */
#define XSK_EFUSEPL_STATUS_FORCE_PCYCLE_RECONFIG	0x002
#define XSK_EFUSEPL_STATUS_DISABLE_KEY_WRITE		0x004
#define XSK_EFUSEPL_STATUS_DISABLE_AES_KEY_READ		0x008
#define XSK_EFUSEPL_STATUS_DISABLE_USER_KEY_READ	0x010
#define XSK_EFUSEPL_STATUS_DISABLE_FUSE_CNTRL_WRITE	0x020
#define XSK_EFUSEPL_STATUS_EFUSE_SEC_ENABLE			0x100
#define XSK_EFUSEPL_STATUS_JTAG_DISABLE				0x200
#define XSK_EFUSEPL_STATUS_BBRAM_KEY_DISABLE		0x400

/************************** Variable Definitions ****************************/
/************************** Function Prototypes *****************************/
/**
 * Function used to convert the string to Hex in Little Endian format
 */
u32 XilSKey_Efuse_ConvertStringToHexLE(const char * Str, u8 * Buf, u32 Len);
/**
 * Function used to convert the string to Hex in Little Endian format
 */
u32 XilSKey_Efuse_ConvertStringToHexBE(const char * Str, u8 * Buf, u32 Len);
/**
 * Function used to validate the AES & RSA key provided as input
 */
u32 XilSKey_Efuse_ValidateKey(const char *Key, u32 Len);
u32 XilSKey_EfusePs_InitData(XilSKey_EPs *PsInstancePtr);
u32 XilSKey_EfusePl_InitData(XilSKey_EPl *PlInstancePtr);
/*extern void ps7_init();*/
/***************************************************************************/
int main()
{
	u32 PlStatus = 0xFFFF;
	u32 PsStatus = 0xFFFF;
	u32 Status = 0;

    /*ps7_init();*/
#ifdef XSK_EFUSEPS_DRIVER
	XilSKey_EPs PsInstancePtr;
	u32 PsStatusBits = 0;

	/**
	 * Initialize the PS instance pointer with the data
	 * populated in xilskey_input.h
	 */
	PsStatus = XilSKey_EfusePs_InitData(&PsInstancePtr);
    if(PsStatus != XST_SUCCESS)	{
		printf("PS Data Initialization failed\r\n");
        goto EFUSE_ERROR;
	}

    /*
     * Read the PS efuse status
     * Change in these status bits will only be reflected after POR
     */
    PsStatus = XilSKey_EfusePs_ReadStatus(&PsInstancePtr, &PsStatusBits);
    if(PsStatus != XST_SUCCESS)	{
		printf("PS status read failed\r\n");
        goto EFUSE_ERROR;
	}

    /*
     * Print Efuse PS status bits
     */
    xil_printf("EfusePS status bits : 0x%x \n\r", PsStatusBits);

    if((PsStatusBits & XSK_EFUSEPS_STATUS_WP_BIT_LOW) ||
    		(PsStatusBits & XSK_EFUSEPS_STATUS_WP_BIT_HIGH)) {
    	xil_printf("EfusePS status bits : Write protect enabled\n\r");
    }else {
    	xil_printf("EfusePS status bits : Write protect disabled\n\r");
    }

    if(PsStatusBits & XSK_EFUSEPS_STATUS_RSA_EN) {
    	xil_printf("EfusePS status bits : RSA authentication of "
    			"fsbl enabled\n\r");
    }else {
    	xil_printf("EfusePS status bits : RSA authentication of "
    			"fsbl disabled\n\r");
    }

    if(PsStatusBits & XSK_EFUSEPS_STATUS_ROM_128_CRC) {
    	xil_printf("EfusePS status bits : 128k CRC check on ROM enabled\n\r");
    }else {
    	xil_printf("EfusePS status bits : 128k CRC check on ROM disabled\n\r");
    }

    /**
     * Write the PS eFUSE as defined in xilskeyinput.h
     */
   PsStatus = XilSKey_EfusePs_Write(&PsInstancePtr);
    if (PsStatus != XST_SUCCESS) {
    	printf("PS EFUSE writing failed\n");
    	goto EFUSE_ERROR;
    }

    u32 Index;
	/**
	 *  Clear the structure element of Rsa Key Hash for reading
	 */
    for(Index=0;Index<32;Index++){
        PsInstancePtr.RsaKeyHashValue[Index]=0;
    }
    /**
     * Read the PS eFUSE RSA Key Hash
     */
    PsStatus = XilSKey_EfusePs_Read(&PsInstancePtr);
    if (PsStatus != XST_SUCCESS){
    	printf("PS EFUSE reading failed\n");
    	goto EFUSE_ERROR;
    }

    /**
     * Print the read PS eFUSE RSA Key Hash
     */
    printf("Read RSA Key Hash: \n");

    for(Index=0;Index<32;Index++){
    	printf("%02x",PsInstancePtr.RsaKeyReadback[Index]);
    }
    printf("\n");

#endif /* XSK_EFUSEPS_DRIVER*/

#ifdef XSK_EFUSEPL_DRIVER

    XilSKey_EPl PlInstancePtr;
    u32 PlStatusBits = 0;
    int KeyCnt;

    /**
	 * Initialize the PL data structure based on the xilskey_input.h values
	 */
    PlStatus = XilSKey_EfusePl_InitData(&PlInstancePtr);
    if( PlStatus != XST_SUCCESS) {
		printf("PL Data Initialization Failed\r\n");
        goto EFUSE_ERROR;
	}

     /**
 	 * Call the PL eFUSE programming function to program the eFUSE
 	 * based on the user input
 	 */
     PlStatus = XilSKey_EfusePl_Program(&PlInstancePtr);
 	if(PlStatus != XST_SUCCESS)	{
 		printf("PL eFUSE programming failed\r\n");
 	}

    /*
     * Read Efuse PL status bits
     */
 	PlStatus = XilSKey_EfusePl_ReadStatus(&PlInstancePtr, &PlStatusBits);
    if( PlStatus != XST_SUCCESS) {
		printf("PL efuse status read failed\r\n");
        goto EFUSE_ERROR;
	}

    /*
     * Print Efuse PL status bits
     */
    xil_printf("EfusePL status bits : 0x%x \n\r", PlStatusBits);

    if(PlStatusBits & XSK_EFUSEPL_STATUS_FORCE_PCYCLE_RECONFIG) {
    	xil_printf("EfusePL status bits : Force power cycle for "
    			"reconfiguration enabled\n\r");
    }else {
    	xil_printf("EfusePL status bits : Force power cycle for "
    			"reconfiguration disabled\n\r");
    }

    if(PlStatusBits & XSK_EFUSEPL_STATUS_DISABLE_KEY_WRITE) {
    	xil_printf("EfusePL status bits : Key write disabled \n\r");
    }else {
    	xil_printf("EfusePL status bits : Key write enabled \n\r");
    }

    if(PlStatusBits & XSK_EFUSEPL_STATUS_DISABLE_AES_KEY_READ) {
    	xil_printf("EfusePL status bits : AES Key read disabled \n\r");
    }else {
    	xil_printf("EfusePL status bits : AES Key read enabled \n\r");
    }

    if(PlStatusBits & XSK_EFUSEPL_STATUS_DISABLE_USER_KEY_READ) {
    	xil_printf("EfusePL status bits : User Key read disabled \n\r");
    }else {
    	xil_printf("EfusePL status bits : User Key read enabled \n\r");
    }

    if(PlStatusBits & XSK_EFUSEPL_STATUS_DISABLE_FUSE_CNTRL_WRITE) {
    	xil_printf("EfusePL status bits : Fuse Control write disabled \n\r");
    }else {
    	xil_printf("EfusePL status bits : Fuse Control write enabled \n\r");
    }

    if(PlStatusBits & XSK_EFUSEPL_STATUS_EFUSE_SEC_ENABLE) {
    	xil_printf("EfusePL status bits : Efuse secure boot enabled \n\r");
    }else {
    	xil_printf("EfusePL status bits : Efuse secure boot disabled \n\r");
    }

    if(PlStatusBits & XSK_EFUSEPL_STATUS_JTAG_DISABLE) {
    	xil_printf("EfusePL status bits : Jtag disabled \n\r");
    }else {
    	xil_printf("EfusePL status bits : Jtag enabled \n\r");
    }

    if(PlStatusBits & XSK_EFUSEPL_STATUS_BBRAM_KEY_DISABLE) {
    	xil_printf("EfusePL status bits : BBRAM key disabled \n\r");
    }else {
    	xil_printf("EfusePL status bits : BBRAM key enabled \n\r");
    }

    /*
     * Read Efuse PL key
     */
    PlStatus = XilSKey_EfusePl_ReadKey(&PlInstancePtr);
    if( PlStatus != XST_SUCCESS) {
		printf("PL efuse key read failed\r\n");
        goto EFUSE_ERROR;
	}
    /*
     * Print Efuse PL key
     */
    xil_printf("EfusePL User key : 0x");
    for(KeyCnt = 3; KeyCnt >= 0; KeyCnt--)
    {
    	xil_printf("%02x", PlInstancePtr.UserKeyReadback[KeyCnt]);
    }
    xil_printf("\n\r");

    xil_printf("EfusePL AES key : 0x");
    for(KeyCnt = 31; KeyCnt >= 0; KeyCnt--)
    {
    	xil_printf("%02x", PlInstancePtr.AESKeyReadback[KeyCnt]);
    }
    xil_printf("\n\r");

#endif /*XSK_EFUSEPL_DRIVER*/

EFUSE_ERROR:
	/**
	 * Write the error returned in the Reboot Status Register
	 * Eg: If the reboot status register value is 0xYYYYZZZZ
	 * then
	 * - YYYY Represents the PS eFUSE Status.
	 * - ZZZZ Represents the PL eFUSE Status.
	 *
	 * - Value 0x0000ZZZZ
	 * 	 represents PS eFUSE is successful & PL eFUSE process
	 * 	 returned with error.
	 * - Value 0xYYYY0000
	 * 	 represents PL eFUSE is successful & PS eFUSE process
	 * 	 returned with error.
	 * - Value 0xFFFF0000
	 * 	 represents PS eFUSE is not initiated & PL eFUSE is successful.
	 * - Value 0x0000FFFF
	 * 	 represents PL eFUSE is not initiated & PS eFUSE is successful.
	 * - Value 0xFFFFZZZZ
	 * 	 represents PS eFUSE is not initiated & PL eFUSE is process
	 * 	 returned with error.
	 * - Value 0xYYYYFFFF
	 * 	 represents PL eFUSE is not initiated & PS eFUSE is process
	 * 	 returned with error.
	 */
	Status = ((PsStatus << 16) + PlStatus);

	printf("eFUSE operations exit status: %08X\r\n", Status);

	/**
	 * Writing the Exit Status to Reboot Status Register
	 */
	Xil_Out32(REBOOT_STATUS_REG_ADDR,Status);
    while(1);
    return 0;
}


#ifdef XSK_EFUSEPS_DRIVER
/****************************************************************************/
/**
*
*
*  Helper functions to properly initialize the PS eFUSE structure instance
*
*
* @param	PsInstancePtr - Structure Address to update the structure elements
*
* @return
*
*	- XST_SUCCESS - In case of Success
*	- XST_FAILURE - If initialization fails
*
* @note
*
*****************************************************************************/

u32 XilSKey_EfusePs_InitData(XilSKey_EPs *PsInstancePtr)
{
	u32 PsStatus;

	PsStatus = XST_SUCCESS;

    /**
     * Copy the xilskeyinput.h values into PS structure elements
     */
	PsInstancePtr->EnableWriteProtect = XSK_EFUSEPS_ENABLE_WRITE_PROTECT;
	PsInstancePtr->EnableRsaAuth = XSK_EFUSEPS_ENABLE_RSA_AUTH;
	PsInstancePtr->EnableRom128Crc = XSK_EFUSEPS_ENABLE_ROM_128K_CRC;
	PsInstancePtr->EnableRsaKeyHash = XSK_EFUSEPS_ENABLE_RSA_KEY_HASH;

	if (PsInstancePtr->EnableRsaKeyHash == TRUE) {
		/**
		 * Validation of RSA Hash
		 */
		PsStatus = XilSKey_Efuse_ValidateKey(
						(char *)XSK_EFUSEPS_RSA_KEY_HASH_VALUE,
						XSK_EFUSEPL_RSA_KEY_HASH_STRING_SIZE);
		if(PsStatus != XST_SUCCESS)	{
			return PsStatus;
		}

		/**
		 * Convert the input RSA Key Hash string into Hex buffer
		 */
		PsStatus = XilSKey_Efuse_ConvertStringToHexBE(
						XSK_EFUSEPS_RSA_KEY_HASH_VALUE,
						&(PsInstancePtr->RsaKeyHashValue[0]), 64);
		if(PsStatus != XST_SUCCESS)	{
			return PsStatus;
		}
	}
	return PsStatus;
}
#endif /* XSK_EFUSEPS_DRIVER*/


#ifdef XSK_EFUSEPL_DRIVER
/****************************************************************************/
/**
*
*
*  Helper functions to properly initialize the PL eFUSE structure instance
*
*
* @param	PlInstancePtr - Structure Address to update the structure elements
*
* @return
*
*	- XST_SUCCESS - In case of Success
*	- XST_FAILURE - If initialization fails
*
* @note
*
*****************************************************************************/

u32 XilSKey_EfusePl_InitData(XilSKey_EPl *PlInstancePtr)
{

	u32 PlStatus;

	PlStatus = XST_SUCCESS;


    /**
     * Copy the xilskeyinput.h values into PL eFUSE structure elements
     */

    /**
	 * Assign FUSE CNTRL bits[1:5] to the PL eFUSE structure elements.
	 */

	PlInstancePtr->ForcePowerCycle		= 	XSK_EFUSEPL_FORCE_PCYCLE_RECONFIG;
	PlInstancePtr->KeyWrite 			= 	XSK_EFUSEPL_DISABLE_KEY_WRITE;
	PlInstancePtr->AESKeyRead 			= 	XSK_EFUSEPL_DISABLE_AES_KEY_READ;
	PlInstancePtr->UserKeyRead 			= 	XSK_EFUSEPL_DISABLE_USER_KEY_READ;
	PlInstancePtr->CtrlWrite 			= 	XSK_EFUSEPL_DISABLE_FUSE_CNTRL_WRITE;
	/**
	 * Assign the FUSE CNTRL bits[8:10] into the PL eFUSE structure elements
	 */
	PlInstancePtr->UseAESOnly 			= 	XSK_EFUSEPL_FORCE_USE_AES_ONLY;
	PlInstancePtr->JtagDisable			= 	XSK_EFUSEPL_DISABLE_JTAG_CHAIN;
	PlInstancePtr->AESKeyExclusive 		= 	XSK_EFUSEPL_BBRAM_KEY_DISABLE;

	PlInstancePtr->JtagMioTDI 			=	XSK_EFUSEPL_MIO_JTAG_TDI;
	PlInstancePtr->JtagMioTDO			=	XSK_EFUSEPL_MIO_JTAG_TDO;
	PlInstancePtr->JtagMioTCK			=	XSK_EFUSEPL_MIO_JTAG_TCK;
	PlInstancePtr->JtagMioTMS			=	XSK_EFUSEPL_MIO_JTAG_TMS;
	PlInstancePtr->JtagMioMuxSel 		=	XSK_EFUSEPL_MIO_JTAG_MUX_SELECT;
	PlInstancePtr->JtagMuxSelLineDefVal	=  XSK_EFUSEPL_MIO_MUX_SEL_DEFAULT_VAL;

	/*
	 * Variable to check whether internal system initialization is done.
	 */
	PlInstancePtr->SystemInitDone	=  0;

	/**
	 * Assign the user selection for AES & USER Low Key (or)
	 * User High Key (or) both
	 */
	PlInstancePtr->ProgAESandUserLowKey	=
			XSK_EFUSEPL_PROGRAM_AES_AND_USER_LOW_KEY;
	PlInstancePtr->ProgUserHighKey 		= XSK_EFUSEPL_PROGRAM_USER_HIGH_KEY;

	if (PlInstancePtr->ProgAESandUserLowKey == TRUE) {
		/**
		 * Validation of AES Key
		 */
		PlStatus = XilSKey_Efuse_ValidateKey((char *)XSK_EFUSEPL_AES_KEY,
											XSK_EFUSEPL_AES_KEY_STRING_SIZE);
		if(PlStatus != XST_SUCCESS) {
			goto PL_INIT_ERROR;
		}
		/**
		 * Validation of User Low Key
		 */
		PlStatus = XilSKey_Efuse_ValidateKey((char *)XSK_EFUSEPL_USER_LOW_KEY,
										XSK_EFUSEPL_USER_LOW_KEY_STRING_SIZE);
		if(PlStatus != XST_SUCCESS)	{
			goto PL_INIT_ERROR;
		}

		/**
		 * Assign the AES Key Value
		 */
		XilSKey_Efuse_ConvertStringToHexLE((char *)XSK_EFUSEPL_AES_KEY ,
				&PlInstancePtr->AESKey[0],
				XSK_EFUSEPL_AES_KEY_SIZE_IN_BITS);

		/**
		 * Assign the User Low key [7:0] bits
		 */
		XilSKey_Efuse_ConvertStringToHexLE((char *)XSK_EFUSEPL_USER_LOW_KEY ,
				&PlInstancePtr->UserKey[0],
				XSK_EFUSEPL_USER_LOW_KEY_SIZE_IN_BITS);
	}

	if (PlInstancePtr->ProgUserHighKey == TRUE) {
		/**
		 * Validation of User High Key
		 */
		PlStatus = XilSKey_Efuse_ValidateKey(
						(char *)XSK_EFUSEPL_USER_HIGH_KEY,
						XSK_EFUSEPL_USER_HIGH_KEY_STRING_SIZE);
		if(PlStatus != XST_SUCCESS) {
			goto PL_INIT_ERROR;
		}
		/**
		 * Assign the User High key [31:8] bits
		 */
		XilSKey_Efuse_ConvertStringToHexLE((char *)XSK_EFUSEPL_USER_HIGH_KEY ,
						&PlInstancePtr->UserKey[1],
						XSK_EFUSEPL_USER_HIGH_KEY_SIZE_IN_BITS);
	}

PL_INIT_ERROR:
	return PlStatus;
}

#endif /*XSK_EFUSEPL_DRIVER*/

