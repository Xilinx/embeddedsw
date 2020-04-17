/******************************************************************************
* Copyright (c) 2013 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
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
*  		In Zynq PL eFUSE holds the AES key, user key and some feature bits and
*		and in Ultrascale eFUSE holds AES key, User key, RSA key Hash and
*		some feature bits.
*  		User has the provision to write PS eFUSE & PL eFUSE independently or
*  		can combine together. This can be selected by using the compilation
*  		switch provided in xilskey_input.h. XSK_EFUSEPS_DRIVER should be
*  		defined to enable PS functionality & XSK_EFUSEPL_DRIVER for PL
*  		functionality.
*		However for programming eFuse Ultrascale only XSK_EFUSEPL_DRIVER should be
*		enabled.
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
*		For Zynq
*		-----------------------------------------------------------------------
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
*
*		For Ultrascale
*		-----------------------------------------------------------------------
*		Accessing Ultrascale microblaze 's eFuse is done by using block RAM
*		initialization.
*		Ultrascale eFUSE programming is done through MASTER JTAG. Crucial
*		Programming sequence will be taken care by Hardware module. So Hardware
*		module should be added compulsory in the design. Please use hardware
*		module's vhd code and instructions provided to add Hardware module
*		in the design.
*
*		Software will handover jtag navigation to Hardware module before
*		entering JTAG IDLE state, by setting START pin of Hardware module to
*		high and once programming of specified bit is done Hardware module
*		will set END pin to high.
*
*		Master Jtag primitive has to added to design i.e MASTER_JTAG_inst
*		instantiation have to performed and AXI GPIO pins has to be connected
*		to TDO, TDI, TMS and TCK signals of MASTER_JTAG	primitive.
*
*		Hardware module has to be added to our design and respective pins of
*		the hardware module needs to be connected to AXI GPIO pins.
*		Hardware module has three signals READY, START and END.
*
*		All Inputs(TDO, READY and END) and All Outputs(TDI, TMS, TCK, START) of
*		Hardware module, MASTER_JTAG can be connected as follows.
*		1) All Inputs to one channel
*			All Outputs to other channel
*		Valid example: All Outputs connected to Channel 1
*			Input signal TDO also connected to channel 2
*		2) All Inputs and All Outputs to same channel.
*		Valid example: All Outputs connected to Channel 1
*				Input signal TDO also connected to channel 1
*		3) But some of the Outputs in one channel and some of them in different
*		channel is not accepted.
*		Invalid example: All Outputs connected to Channel 1
*			Input signals (TDI, TMS) connected to Channel 1
*			Input signal TCK also connected to channel 2
*		The design should only contain AXI bram Ctrl memory mapped(1MB).
*		System management wizard should be operated in DRP interface.
*		Note: MASTER_JTAG will disable all other JTAGs
*
*		Procedure to access efuse of Ultrascale:
*		1) After providing the required inputs in xilskey_input.h, compile the
*		project.
*		2) Generate a memory mapped interface(mmi) file using TCL command
*		write_mem_info $Outfilename
*		3) Create an associate elf file.
*				(OR)
*		Update memory has to be done using the tcl command updatemem.
*		updatemem -meminfo $file.mmi -data $Outfilename.elf -bit $design.bit
*					-proc design_1_i/microblaze_0 -out $Final.bit
*		4) Program the board using $Final.bit bitstream
*		5) Output can be seen in UART terminal.
*		6) For calculating CRC of AES key reverse polynomial is 0x82F63B78 or
*		one can use the API u32 XilSKey_CrcCalculation(u8 *Key)
*
* MODIFICATION HISTORY:
*
* Ver   Who  	Date     Changes
* ----- ---- 	-------- --------------------------------------------------------
* 1.00a rpoolla 04/26/13 First release
* 1.02a hk      10/28/13 Added use of API's read status and key. PR# 735957.
* 3.00  vns     31/07/15 Modified XSK_EFUSEPL_RSA_KEY_HASH_STRING_SIZE macro
*                        name to XSK_EFUSEPS_RSA_KEY_HASH_STRING_SIZE.
*                        Added missing goto statement.
*                        Modified init function, intialisation of instance is
*                        done based on the platform and Modified example
*                        to support both Zynq PL's eFuse and also Ultrascale's
*                        eFuse.
* 4.00  vns     09/10/15 Added DFT JTAG disable and DFT MODE disable
*                        programming and reading options for Zynq eFuse PS.
* 6.0   vns     07/07/16 Added Gpio pin numbers connected to hardware module.
* 6.4   vns     02/27/18 Added support for programming secure bit 6 -
*                        enable obfuscation feature for eFUSE AES key
*               03/09/17 Corrected status bits of Ultrascale plus
* 6.7   arc     10/29/18 Fixed ARMCC compiler warnings and errors
*       psl     03/20/19 Added eFuse key write support for SSIT devices.
*       psl     03/29/19 Added support for user configurable GPIO for
*                        jtag control.
*       psl     04/29/19 Resolved GCC warnings.
* 6.8   psl     05/21/19 Added platform dependent macros for variables and
*                        initialized PlStatus.
*                        Added print for current SLR and CRC.
*               07/17/19 Added print to display CRC of AES key during CRC
*                        verification.
*               08/30/19 Corrected length in bits parameter while converting
*                        PPK hash string(zynq efuse ps)
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
 * user key size in bits
 */
#define XSK_EFUSEPL_USER_KEY_SIZE_IN_BITS			(32)
/**
 * PL eFUSE RSA key size in characters
 */
#define XSK_EFUSEPL_RSA_KEY_HASH_STRING_SIZE		(96)
/**
 *  RSA Key size in Bytes
 */
#define XSK_EFUSEPL_RSA_KEY_SIZE_IN_BITS		(384)
/**
 * PL eFuse user key size in characters
 */
#define XSK_EFUSEPL_USER_KEY_STRING_SIZE			(8)

/*
 * PS efuse status bit definitions
 */
#define XSK_EFUSEPS_STATUS_WP_BIT_LOW	0x1000
#define XSK_EFUSEPS_STATUS_WP_BIT_HIGH	0x2000
#define XSK_EFUSEPS_STATUS_RSA_EN		0x400
#define XSK_EFUSEPS_STATUS_ROM_128_CRC	0x800
#define XSK_EFUSEPS_STATUS_DFT_JTAG_DISABLE	0x200
#define XSK_EFUSEPS_STATUS_DFT_MODE_DISABLE	0x100

/*
 * PL efuse status bit definitions of Zynq
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
u32 XilSKey_EfusePl_LoadData_Slr(XilSKey_EPl *PlInstancePtr, u32 SlrNum);
u32 XilSKey_EfusePl_ReadnCheck(XilSKey_EPl *PlInstancePtr);
u32 XilSKey_EfusePl_ReadorWrite(XilSKey_EPl *PlInstancePtr);

#ifdef XSK_EFUSEPL_DRIVER
XilSKey_EPl PlInstance; /*JTAG Tap Instance*/
#endif
/***************************************************************************/
int main()
{
	u32 PlStatus = 0xFFFF;
	u32 PsStatus = 0xFFFF;
	u32 Status = 0;
#if defined (XSK_MICROBLAZE_PLATFORM) || defined (XSK_EFUSEPS_DRIVER)
	u32 Index;
#endif

#if defined (XSK_EFUSEPL_DRIVER) && defined (XSK_MICROBLAZE_PLATFORM)
	u32 StatusSum[XSK_TARGET_MAX_4_SLRS] = {0};
#endif

    /*ps7_init();*/
#ifdef XSK_EFUSEPS_DRIVER
	XilSKey_EPs PsInstancePtr;
	u32 PsStatusBits = 0;
#ifdef XSK_MICROBLAZE_PLATFORM
	xil_printf("Kintex ultrascale will not have PS please disable"
			"XSK_EFUSEPS_DRIVER\n\r");
	goto EFUSE_ERROR;
#endif
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

	if (PsStatusBits & XSK_EFUSEPS_STATUS_DFT_JTAG_DISABLE) {
		xil_printf("EfusePS status bits : DFT JTAG is disabled\n\r");
	}
	else {
	xil_printf("EfusePS status bits : DFT JTAG is enabled\n\r");
	}

	if (PsStatusBits & XSK_EFUSEPS_STATUS_DFT_MODE_DISABLE) {
		xil_printf("EfusePS status bits : DFT mode is disabled\n\r");
	}
	else {
		xil_printf("EfusePS status bits : DFT mode is enabled\n\r");
	}


    /**
     * Write the PS eFUSE as defined in xilskeyinput.h
     */
   PsStatus = XilSKey_EfusePs_Write(&PsInstancePtr);
    if (PsStatus != XST_SUCCESS) {
	printf("PS EFUSE writing failed\n");
	goto EFUSE_ERROR;
    }

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

	/**
	 * Initialize the PL data structure based on the xilskey_input.h values
	 */
	PlStatus = XilSKey_EfusePl_InitData(&PlInstance);
	if( PlStatus != XST_SUCCESS) {
		printf("PL Data Initialization Failed\r\n");
		goto EFUSE_ERROR;
	}


#ifdef XSK_ARM_PLATFORM
	PlStatus = XilSKey_EfusePl_Program(&PlInstance);
	if(PlStatus != XST_SUCCESS)	{
		printf("PL eFUSE programming failed\r\n");
		goto EFUSE_ERROR;
	}

	PlStatus = XilSKey_EfusePl_ReadnCheck(&PlInstance);
	if(PlStatus != XST_SUCCESS){
		printf("Read Status/Check failed\r\n");
		goto EFUSE_ERROR;
	}
#else

	/*
	 * Initialize jtag and scan device ID
	 */
	PlStatus = XilSKey_EfusePl_SystemInit(&PlInstance);
	if( PlStatus != XST_SUCCESS) {
		printf("jtag and scan device ID Failed\r\n");
		goto EFUSE_ERROR;
	}

	/**
	 * Verify the SLR requested to program against target
	 * validity
	 */
	switch(PlInstance.NumSlr)
	{

		case XSK_TARGET_MAX_1_SLRS:
			if(((XSK_EFUSEPL_PGM_SLR_CONFIG_ORDER_1 == TRUE) ||
					(XSK_EFUSEPL_PGM_SLR_CONFIG_ORDER_2 == TRUE) ||
					( XSK_EFUSEPL_PGM_SLR_CONFIG_ORDER_3 == TRUE)))
			{
				PlStatus = (u32)XST_FAILURE;
			}
			break;
		case XSK_TARGET_MAX_2_SLRS:
			if((XSK_EFUSEPL_PGM_SLR_CONFIG_ORDER_2 == TRUE) ||
					(XSK_EFUSEPL_PGM_SLR_CONFIG_ORDER_3 == TRUE))
			{
				PlStatus = (u32)XST_FAILURE;
			}
			break;
		case XSK_TARGET_MAX_3_SLRS:
			if(XSK_EFUSEPL_PGM_SLR_CONFIG_ORDER_3 == TRUE) {
				PlStatus = (u32)XST_FAILURE;
			}
			break;
		case XSK_TARGET_MAX_4_SLRS:
			PlStatus = (u32)XST_SUCCESS;
			break;
		default:
			Status = (u32)XST_FAILURE;
			break;
	}

	if(PlStatus != (u32)XST_SUCCESS)
	{
		Status = XST_FAILURE;
		goto FINAL_EFUSE_ERROR;
	}
	else
	{
		xil_printf("App: SLR Count verification passed\r\n");
	}

	for(Index=0U; Index< PlInstance.NumSlr; Index++)
	{
		/**
		 * Program and Read for SLRX
		 * Programming will not happen until corresponding PGM_SLR1 is not TRUE
		 */
		PlInstance.SlrConfigOrderIndex = Index;
		xil_printf("-----------------------SLR CONFIG ORDER INDEX %d-----------------------\n\r",
				PlInstance.SlrConfigOrderIndex);
		PlStatus=XilSKey_EfusePl_ReadorWrite(&PlInstance);
		if(PlStatus != XST_SUCCESS)
		{
			StatusSum[Index] = PlStatus;
			goto EFUSE_ERROR;
		}
	}

#endif /*XSK_EFUSEPL_DRIVER*/
#endif
EFUSE_ERROR:

#if defined (XSK_EFUSEPL_DRIVER) && defined (XSK_MICROBLAZE_PLATFORM)
FINAL_EFUSE_ERROR:
	PlStatus =  (StatusSum[0] |
				 StatusSum[1] |
				 StatusSum[2] |
				 StatusSum[3]);
#endif

	Status = ((PsStatus << 16) + PlStatus);
	xil_printf("\r\neFUSE operations exit status: %08X *****\r\n", Status);
#ifdef XSK_ARM_PLATFORM
		/**
		* Writing the Exit Status to Reboot Status Register
		*/
		Xil_Out32(REBOOT_STATUS_REG_ADDR,Status);
#endif

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
	PsInstancePtr->DisableDftJtag = XSK_EFUSEPS_DISABLE_DFT_JTAG;
	PsInstancePtr->DisableDftMode = XSK_EFUSEPS_DISABLE_DFT_MODE;

	if (PsInstancePtr->EnableRsaKeyHash == TRUE) {
		/**
		 * Validation of RSA Hash
		 */
		PsStatus = XilSKey_Efuse_ValidateKey(
						(char *)XSK_EFUSEPS_RSA_KEY_HASH_VALUE,
						XSK_EFUSEPS_RSA_KEY_HASH_STRING_SIZE);
		if(PsStatus != XST_SUCCESS)	{
			return PsStatus;
		}

		/**
		 * Convert the input RSA Key Hash string into Hex buffer
		 */
		PsStatus = XilSKey_Efuse_ConvertStringToHexBE(
						XSK_EFUSEPS_RSA_KEY_HASH_VALUE,
						&(PsInstancePtr->RsaKeyHashValue[0]), 
						256U);
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
*  Wrapper functions to program and read keys
*
*
* @param	PlInstancePtr - Structure Address to update the structure elements
* @param	SlrNum - Number to differentiate the data targeted to specific SLR
*
*
* @return
*
*	- XST_SUCCESS - In case of Success
*	- Error - If initialization fails
*
* @note
*
*****************************************************************************/
#if defined (XSK_MICROBLAZE_ULTRA_PLUS) || defined (XSK_MICROBLAZE_ULTRA)
u32 XilSKey_EfusePl_ReadorWrite(XilSKey_EPl *PlInstancePtr)
{
	u32 PlStatus;


	PlStatus = XilSKey_EfusePl_LoadData_Slr(PlInstancePtr, PlInstancePtr->SlrConfigOrderIndex);
	if(PlStatus != XST_SUCCESS)
	{
		xil_printf("Loading data for SLR%x failed\r\n", PlInstancePtr->SlrConfigOrderIndex);
		goto END;
	}

	if(((PlInstancePtr->SlrConfigOrderIndex == XSK_SLR_CONFIG_ORDER_0) &&
						(XSK_EFUSEPL_PGM_SLR_CONFIG_ORDER_0 == TRUE)) ||
	   ((PlInstancePtr->SlrConfigOrderIndex == XSK_SLR_CONFIG_ORDER_1) &&
						  (XSK_EFUSEPL_PGM_SLR_CONFIG_ORDER_1 == TRUE)) ||
	   ((PlInstancePtr->SlrConfigOrderIndex == XSK_SLR_CONFIG_ORDER_2) &&
						   (XSK_EFUSEPL_PGM_SLR_CONFIG_ORDER_2 == TRUE)) ||
	   ((PlInstancePtr->SlrConfigOrderIndex == XSK_SLR_CONFIG_ORDER_3) &&
					   (XSK_EFUSEPL_PGM_SLR_CONFIG_ORDER_3 == TRUE)))
	{
		/**
		* Call the PL eFUSE programming function to program the eFUSE
		* based on the user input
		*/
		PlStatus = XilSKey_EfusePl_Program(PlInstancePtr);
		if(PlStatus != XST_SUCCESS)	{
			xil_printf("PL eFUSE programming failed for SLR%lx\r\n", PlInstancePtr->SlrConfigOrderIndex);
			goto END;
		}
	}
	else
	{
		xil_printf("Programming not enabled for SLR(%x)\r\n", PlInstancePtr->SlrConfigOrderIndex);
	}
	PlStatus = XilSKey_EfusePl_ReadnCheck(PlInstancePtr);
	if(PlStatus != XST_SUCCESS)
	{
		xil_printf("Read Status/Check failed for SLR%x\r\n", PlInstancePtr->SlrConfigOrderIndex);
		goto END;
	}

END:
	return PlStatus;
}
#endif
/****************************************************************************/
/**
*
*
*  Helper functions to validate the keys to be written
*
*
* @param	PlInstancePtr - Structure Address to update the structure elements
* @param	SlrNum - Number to differentiate the data targeted to specific SLR
*
*
* @return
*
*	- XST_SUCCESS - In case of Success
*	- Error - If initialization fails
*
* @note
*
*****************************************************************************/
u32 XilSKey_EfusePl_ReadnCheck(XilSKey_EPl *PlInstancePtr)
{
    u32 PlStatusBits = 0;
    u32 PlStatus;
    s32 KeyCnt;

    /*
     * Read Efuse PL status bits
     */
	PlStatus = XilSKey_EfusePl_ReadStatus(PlInstancePtr, &PlStatusBits);
    if( PlStatus != XST_SUCCESS) {
		printf("PL efuse status read failed\r\n");
        goto EFUSE_ERROR;
	}

    /*
     * Print Efuse PL status bits
     */
    xil_printf("EfusePL status bits : 0x%x \n\r", PlStatusBits);
    /* Status bits for Zynq */
	if (PlInstancePtr->FpgaFlag == XSK_FPGA_SERIES_ZYNQ) {
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
	}
	else {  /* for Ultrascale */
		if(PlStatusBits & (1 << XSK_EFUSEPL_STATUS_DISABLE_KEY_READ_ULTRA)) {
			xil_printf("EfusePL status bits : AES key programming"
					"and CRC read  disabled \n\r");
		}else {
			xil_printf("EfusePL status bits : AES key CRC read enabled\n\r");
		}
		if(PlStatusBits & (1 << XSK_EFUSEPL_STATUS_DISABLE_USER_KEY_READ_ULTRA)) {
			xil_printf("EfusePL status bits : 32 bit User key read and programming disabled \n\r");
		}else {
			xil_printf("EfusePL status bits : 32 bit User key read enabled\n\r");
		}
		if(PlStatusBits & (1 << XSK_EFUSEPL_STATUS_DISABLE_SECURE_READ_ULTRA)) {
			xil_printf("EfusePL status bits : Secure bits read and programming disabled \n\r");
		}else {
			xil_printf("EfusePL status bits : Secure bits read enabled\n\r");
		}
		if(PlStatusBits & (1<< XSK_EFUSEPL_STATUS_DISABLE_CNTRL_WRITE_ULTRA)) {
			xil_printf("EfusePL status bits : Cntrol bits write disabled \n\r");
		}else {
			xil_printf("EfusePL status bits : Control bits write enabled\n\r");
		}
		if(PlStatusBits & (1 << XSK_EFUSEPL_STATUS_DISABLE_RSA_KEY_READ_ULTRA)) {
			xil_printf("EfusePL status bits : RSA key read and programming disabled \n\r");
		}else {
			xil_printf("EfusePL status bits : RSA key read enabled\n\r");
		}
		if(PlStatusBits & (1 << XSK_EFUSEPL_STATUS_DISABLE_KEY_WRITE_ULTRA)) {
			xil_printf("EfusePL status bits : AES key programming disabled \n\r");
		}else {
			xil_printf("EfusePL status bits : AES key programming enabled\n\r");
		}
		if(PlStatusBits & (1 << XSK_EFUSEPL_STATUS_DISABLE_USER_KEY_WRITE_ULTRA)) {
			xil_printf("EfusePL status bits : 32 bit User key programming disabled \n\r");
		}else {
			xil_printf("EfusePL status bits : 32 bit User key programming enabled\n\r");
		}
		if(PlStatusBits & (1 << XSK_EFUSEPL_STATUS_DISABLE_SECURE_WRITE_ULTRA)) {
			xil_printf("EfusePL status bits : Secure bits programming disabled \n\r");
		}else {
			xil_printf("EfusePL status bits : Secure bits programming enabled\n\r");
		}
		if(PlStatusBits & (1 << XSK_EFUSEPL_STATUS_DISABLE_RSA_KEY_WRITE_ULTRA)) {
			xil_printf("EfusePL status bits : RSA key programming disabled \n\r");
		}else {
			xil_printf("EfusePL status bits : RSA key programming enabled\n\r");
		}
		if(PlStatusBits & (1 << XSK_EFUSEPL_STATUS_DIABLE_128BIT_USER_KEY_WRITE_ULTRA)) {
			xil_printf("EfusePL status bits : 128 bit User key programming disabled \n\r");
		}else {
			xil_printf("EfusePL status bits : 128 bit User Key programming enabled\n\r");
		}
#ifdef XSK_MICROBLAZE_ULTRA
		if(PlStatusBits & (1 << XSK_EFUSEPL_STATUS_FUSE_LOGIC_IS_BUSY_ULTRA)) {
			xil_printf("EfusePL status bits : FUSE logic is busy \n\r");
		}else {
			xil_printf("EfusePL status bits : FUSE logic is free \n\r");
		}
		if(PlStatusBits & (1 << XSK_EFUSEPL_STATUS_ALLOW_ENCRYPTED_ONLY_ULTRA)) {
			xil_printf("EfusePL status bits : Only allows encrypted bitstreams\n\r");
		}else {
			xil_printf("EfusePL status bits : Non encrypted bitstream allowed\n\r");
		}
#endif
		if(PlStatusBits & (1 << XSK_EFUSEPL_STATUS_AES_ONLY_ENABLED_ULTRA)) {
			xil_printf("EfusePL status bits : Decryption only by AES of FUSE \n\r");
		}else {
			xil_printf("EfusePL status bits : Decryption can be AES of FUSE or BBRAM \n\r");
		}
		if(PlStatusBits & (1 << XSK_EFUSEPL_STATUS_RSA_AUTH_ENABLED_ULTRA)) {
			xil_printf("EfusePL status bits : RSA authentication is enabled \n\r");
		}else {
			xil_printf("EfusePL status bits : RSA authentication is disabled \n\r");
		}
#ifdef XSK_MICROBLAZE_ULTRA
		if(PlStatusBits & (1 << XSK_EFUSEPL_STATUS_DISABLE_JTAG_ULTRA)) {
			xil_printf("EfusePL status bits : External Jtag pins are disabled\n\r");
		}else {
			xil_printf("EfusePL status bits : Jtag is not disabled\n\r");
		}
		if(PlStatusBits & (1 << XSK_EFUSEPL_STATUS_DISABLE_TEST_ACCESS_ULTRA)) {
			xil_printf("EfusePL status bits : Disables test access\n\r");
		}else {
			xil_printf("EfusePL status bits : Xilinx test access is enabled\n\r");
		}
#else
		if(PlStatusBits & (1 << XSK_EFUSEPL_STATUS_SECURITY_ENABLE_ULTRA)) {
			xil_printf("EfusePL status bits : Security enabled \n\r");
		}
		else {
			xil_printf("EfusePL status bits : Security is not been enabled\n\r");
		}
#endif
		if(PlStatusBits & (1 << XSK_EFUSEPL_STATUS_DISABLE_DCRPTR_ULTRA)) {
			xil_printf("EfusePL status bits : Decryptor disabled \n\r");
		}else {
			xil_printf("EfusePL status bits : Decryptor enabled\n\r");
		}
		if(PlStatus & (1U <<
			XSK_EFUSEPL_STATUS_ENABLE_OBFUSCATED_EFUSE_KEY)) {
			xil_printf("EfusePL status bits :"
			" Enabled obfucation feature for eFUSE AES key\n\r");
		}
		else {
			xil_printf("EfusePL status bits :"
				" Obfuscation disabled\n\r");
		}

	}

    /*
     * Read Efuse PL key
     */
    PlStatus = XilSKey_EfusePl_ReadKey(PlInstancePtr);
    if( PlStatus != XST_SUCCESS) {
		xil_printf("PL efuse key read failed\r\n");
        goto EFUSE_ERROR;
	}
#ifdef XSK_ARM_PLATFORM
		/*
		 * Print Efuse PL key
		 */
		xil_printf("EfusePL User key : 0x");
		for(KeyCnt = 3; KeyCnt >= 0; KeyCnt--)
		{
		xil_printf("%02x", PlInstancePtr->UserKeyReadback[KeyCnt]);
		}
		xil_printf("\n\r");

		xil_printf("EfusePL AES key : 0x");
		for(KeyCnt = 31; KeyCnt >= 0; KeyCnt--)
		{
		xil_printf("%02x", PlInstancePtr->AESKeyReadback[KeyCnt]);
		}
		xil_printf("\n\r");
#else

		if (XSK_EFUSEPL_CHECK_AES_KEY_CRC == TRUE){
			xil_printf("AES Key's CRC provided for verification : %08x\n\r",
					PlInstancePtr->CrcOfAESKey);
			if (PlInstancePtr->AESKeyMatched == TRUE) {
				xil_printf("AES key matched with expected AES Key's CRC\n\r");
			}
			else {
				xil_printf("AES key not matched with expected AES's CRC "
					"Please provide key's valid CRC \n\r");
			}
		}
		xil_printf("\n\r");
		if (XSK_EFUSEPL_READ_USER_KEY == TRUE) {
			  xil_printf("\r\nEfusePL User key : 0x");
			   for(KeyCnt = (XSK_EFUSEPL_USER_KEY_SIZE_IN_BYTES - 1); KeyCnt >= 0; KeyCnt--) {
				   xil_printf("%02x", PlInstancePtr->UserKeyReadback[KeyCnt]);
			   }
			   xil_printf("\n\r");
		}
		if (XSK_EFUSEPL_READ_RSA_KEY_HASH == TRUE) {
			  xil_printf("\r\nEfusePL RSA hash value : 0x");
			   for(KeyCnt = (XSK_EFUSEPL_RSA_KEY_HASH_SIZE_IN_BYTES - 1); KeyCnt >= 0; KeyCnt--) {
				   xil_printf("%02x", PlInstancePtr->RSAHashReadback[KeyCnt]);
			   }
			   xil_printf("\n\r");
		}
		xil_printf("\r\n");
		if (XSK_EFUSEPL_READ_USER_KEY128_BIT == TRUE) {
			xil_printf("\r\nEfusePL 128 Bit User key : 0x");
			for(KeyCnt = (XSK_EFUSEPL_128BIT_USERKEY_SIZE_IN_BYTES - 1);
						KeyCnt >= 0; KeyCnt--) {
				   xil_printf("%02x", PlInstancePtr->User128BitReadBack[KeyCnt]);
			   }
			xil_printf("\n\r");
		}
#endif
EFUSE_ERROR:
    return PlStatus;
}
/****************************************************************************/
/**
*
*
*  Helper functions to validate the keys to be written
*
*
* @param	PlInstancePtr - Structure Address to update the structure elements
* @param	SlrNum - Number to differentiate the data targeted to specific SLR
*
*
* @return
*
*	- XST_SUCCESS - In case of Success
*	- Error - If initialization fails
*
* @note
*
*****************************************************************************/
#if defined (XSK_MICROBLAZE_ULTRA_PLUS) || defined (XSK_MICROBLAZE_ULTRA)
u32 XilSKey_EfusePl_LoadData_Slr(XilSKey_EPl *PlInstancePtr, u32 SlrNum)
{
	u32 PlStatus = XST_SUCCESS;

	if (PlInstancePtr->ProgUserKeyUltra == TRUE) {

		memset(&PlInstancePtr->UserKey[0], 0, sizeof(PlInstancePtr->UserKey));
		switch(SlrNum)
		{
			case XSK_SLR_CONFIG_ORDER_0:			/* Validation of 32 bit User Key */
				PlStatus = XilSKey_Efuse_ValidateKey(
					(char *)XSK_EFUSEPL_USER_KEY_CONFIG_ORDER_0,
					XSK_EFUSEPL_USER_KEY_STRING_SIZE);

				if(PlStatus != XST_SUCCESS)
					goto PL_INIT_ERROR;


				/* Assign the User key [31:0]bits */
				XilSKey_Efuse_ConvertStringToHexLE(
					(char *)XSK_EFUSEPL_USER_KEY_CONFIG_ORDER_0,
					&PlInstancePtr->UserKey[0],
					XSK_EFUSEPL_USER_KEY_SIZE_IN_BITS);
				break;
			case XSK_SLR_CONFIG_ORDER_1:
				PlStatus = XilSKey_Efuse_ValidateKey(
					(char *)XSK_EFUSEPL_USER_KEY_CONFIG_ORDER_1,
					XSK_EFUSEPL_USER_KEY_STRING_SIZE);

				if(PlStatus != XST_SUCCESS)
				goto PL_INIT_ERROR;


				/* Assign the User key [31:0]bits */
				XilSKey_Efuse_ConvertStringToHexLE(
					(char *)XSK_EFUSEPL_USER_KEY_CONFIG_ORDER_1 ,
					&PlInstancePtr->UserKey[0],
					XSK_EFUSEPL_USER_KEY_SIZE_IN_BITS);
				break;
			case XSK_SLR_CONFIG_ORDER_2:
				PlStatus = XilSKey_Efuse_ValidateKey(
					(char *)XSK_EFUSEPL_USER_KEY_CONFIG_ORDER_2,
					XSK_EFUSEPL_USER_KEY_STRING_SIZE);

				if(PlStatus != XST_SUCCESS)
				goto PL_INIT_ERROR;


				/* Assign the User key [31:0]bits */
				XilSKey_Efuse_ConvertStringToHexLE(
					(char *)XSK_EFUSEPL_USER_KEY_CONFIG_ORDER_2 ,
					&PlInstancePtr->UserKey[0],
					XSK_EFUSEPL_USER_KEY_SIZE_IN_BITS);
				break;
			case XSK_SLR_CONFIG_ORDER_3:
				PlStatus = XilSKey_Efuse_ValidateKey(
					(char *)XSK_EFUSEPL_USER_KEY_CONFIG_ORDER_3,
					XSK_EFUSEPL_USER_KEY_STRING_SIZE);

				if(PlStatus != XST_SUCCESS)
				goto PL_INIT_ERROR;


				/* Assign the User key [31:0]bits */
				XilSKey_Efuse_ConvertStringToHexLE(
					(char *)XSK_EFUSEPL_USER_KEY_CONFIG_ORDER_3 ,
					&PlInstancePtr->UserKey[0],
					XSK_EFUSEPL_USER_KEY_SIZE_IN_BITS);
				break;
			default:
				PlStatus = XST_FAILURE;
				goto PL_INIT_ERROR;
				break;
		}

	}

	if (PlInstancePtr->ProgAESKeyUltra == TRUE)
	{

		memset(&PlInstancePtr->AESKey[0], 0, sizeof(PlInstancePtr->AESKey));
		switch(SlrNum)
		{/* Validation of AES Key */
			case XSK_SLR_CONFIG_ORDER_0:
				PlStatus = XilSKey_Efuse_ValidateKey(
					(char *)XSK_EFUSEPL_AES_KEY_CONFIG_ORDER_0,
					XSK_EFUSEPL_AES_KEY_STRING_SIZE);
				if(PlStatus != XST_SUCCESS) {
					goto PL_INIT_ERROR;
				}

				/* Assign the AES Key Value */
				XilSKey_Efuse_ConvertStringToHexLE(
					(char *)XSK_EFUSEPL_AES_KEY_CONFIG_ORDER_0,
					&PlInstancePtr->AESKey[0],
					XSK_EFUSEPL_AES_KEY_SIZE_IN_BITS);

				PlInstancePtr->CrcToVerify =
						XilSKey_CrcCalculation((u8 *)XSK_EFUSEPL_AES_KEY_CONFIG_ORDER_0);
				xil_printf("Expected CRC : %08x\n",PlInstancePtr->CrcToVerify);

				if(PlStatus != XST_SUCCESS)
				goto PL_INIT_ERROR;
				break;
			case XSK_SLR_CONFIG_ORDER_1:

				PlStatus = XilSKey_Efuse_ValidateKey(
					(char *)XSK_EFUSEPL_AES_KEY_CONFIG_ORDER_1,
					XSK_EFUSEPL_AES_KEY_STRING_SIZE);
				if(PlStatus != XST_SUCCESS) {
					goto PL_INIT_ERROR;
				}
				/* Assign the AES Key Value */
				XilSKey_Efuse_ConvertStringToHexLE(
					(char *)XSK_EFUSEPL_AES_KEY_CONFIG_ORDER_1,
					&PlInstancePtr->AESKey[0],
					XSK_EFUSEPL_AES_KEY_SIZE_IN_BITS);

				PlInstancePtr->CrcToVerify =
						XilSKey_CrcCalculation((u8 *)XSK_EFUSEPL_AES_KEY_CONFIG_ORDER_1);
				xil_printf("Expected CRC : %08x\n",PlInstancePtr->CrcToVerify);
				break;
			case XSK_SLR_CONFIG_ORDER_2:
				PlStatus = XilSKey_Efuse_ValidateKey(
					(char *)XSK_EFUSEPL_AES_KEY_CONFIG_ORDER_2,
					XSK_EFUSEPL_AES_KEY_STRING_SIZE);
				if(PlStatus != XST_SUCCESS) {
					goto PL_INIT_ERROR;
				}
				/* Assign the AES Key Value */
				XilSKey_Efuse_ConvertStringToHexLE(
					(char *)XSK_EFUSEPL_AES_KEY_CONFIG_ORDER_2,
					&PlInstancePtr->AESKey[0],
					XSK_EFUSEPL_AES_KEY_SIZE_IN_BITS);

				PlInstancePtr->CrcToVerify =
						XilSKey_CrcCalculation((u8 *)XSK_EFUSEPL_AES_KEY_CONFIG_ORDER_2);
				xil_printf("Expected CRC : %08x\n",PlInstancePtr->CrcToVerify);
				break;
			case XSK_SLR_CONFIG_ORDER_3:
				PlStatus = XilSKey_Efuse_ValidateKey(
					(char *)XSK_EFUSEPL_AES_KEY_CONFIG_ORDER_3,
					XSK_EFUSEPL_AES_KEY_STRING_SIZE);
				if(PlStatus != XST_SUCCESS) {
					goto PL_INIT_ERROR;
				}
				/* Assign the AES Key Value */
				XilSKey_Efuse_ConvertStringToHexLE(
					(char *)XSK_EFUSEPL_AES_KEY_CONFIG_ORDER_3,
					&PlInstancePtr->AESKey[0],
					XSK_EFUSEPL_AES_KEY_SIZE_IN_BITS);

				PlInstancePtr->CrcToVerify =
						XilSKey_CrcCalculation((u8 *)XSK_EFUSEPL_AES_KEY_CONFIG_ORDER_3);
				xil_printf("Expected CRC : %08x\n",PlInstancePtr->CrcToVerify);
				break;
			default:
				PlStatus = XST_FAILURE;
				goto PL_INIT_ERROR;
				break;

		}
	}

	if (PlInstancePtr->ProgRSAKeyUltra == TRUE)
	{
		memset(&PlInstancePtr->RSAKeyHash[0], 0, sizeof(PlInstancePtr->RSAKeyHash));
		switch(SlrNum)
		{
			case XSK_SLR_CONFIG_ORDER_0:
				/* Validation of RSA hash */
				PlStatus = XilSKey_Efuse_ValidateKey(
					(char *)XSK_EFUSEPL_RSA_KEY_HASH_VALUE_CONFIG_ORDER_0,
					XSK_EFUSEPL_RSA_KEY_HASH_STRING_SIZE);
				if(PlStatus != XST_SUCCESS) {
					goto PL_INIT_ERROR;
				}
				/* Assign the RSA hash */
				XilSKey_Efuse_ConvertStringToHexLE(
					(char *)XSK_EFUSEPL_RSA_KEY_HASH_VALUE_CONFIG_ORDER_0,
					&PlInstancePtr->RSAKeyHash[0],
					XSK_EFUSEPL_RSA_KEY_SIZE_IN_BITS);
				break;
			case XSK_SLR_CONFIG_ORDER_1:
				/* Validation of RSA hash */
				PlStatus = XilSKey_Efuse_ValidateKey(
					(char *)XSK_EFUSEPL_RSA_KEY_HASH_VALUE_CONFIG_ORDER_1,
					XSK_EFUSEPL_RSA_KEY_HASH_STRING_SIZE);
				if(PlStatus != XST_SUCCESS) {
					goto PL_INIT_ERROR;
				}
				/* Assign the RSA hash */
				XilSKey_Efuse_ConvertStringToHexLE(
					(char *)XSK_EFUSEPL_RSA_KEY_HASH_VALUE_CONFIG_ORDER_1,
					&PlInstancePtr->RSAKeyHash[0],
					XSK_EFUSEPL_RSA_KEY_SIZE_IN_BITS);
				break;
			case XSK_SLR_CONFIG_ORDER_2:
				/* Validation of RSA hash */
				PlStatus = XilSKey_Efuse_ValidateKey(
					(char *)XSK_EFUSEPL_RSA_KEY_HASH_VALUE_CONFIG_ORDER_2,
					XSK_EFUSEPL_RSA_KEY_HASH_STRING_SIZE);
				if(PlStatus != XST_SUCCESS) {
					goto PL_INIT_ERROR;
				}
				/* Assign the RSA hash */
				XilSKey_Efuse_ConvertStringToHexLE(
					(char *)XSK_EFUSEPL_RSA_KEY_HASH_VALUE_CONFIG_ORDER_2,
					&PlInstancePtr->RSAKeyHash[0],
					XSK_EFUSEPL_RSA_KEY_SIZE_IN_BITS);
				break;
			case XSK_SLR_CONFIG_ORDER_3:
				/* Validation of RSA hash */
				PlStatus = XilSKey_Efuse_ValidateKey(
					(char *)XSK_EFUSEPL_RSA_KEY_HASH_VALUE_CONFIG_ORDER_3,
					XSK_EFUSEPL_RSA_KEY_HASH_STRING_SIZE);
				if(PlStatus != XST_SUCCESS) {
					goto PL_INIT_ERROR;
				}
				/* Assign the RSA hash */
				XilSKey_Efuse_ConvertStringToHexLE(
					(char *)XSK_EFUSEPL_RSA_KEY_HASH_VALUE_CONFIG_ORDER_3,
					&PlInstancePtr->RSAKeyHash[0],
					XSK_EFUSEPL_RSA_KEY_SIZE_IN_BITS);
				break;
			default:
				PlStatus = XST_FAILURE;
				goto PL_INIT_ERROR;
				break;
		}
	}

	if (PlInstancePtr->ProgUser128BitUltra == TRUE)
	{
		memset(&PlInstancePtr->User128Bit[0], 0, sizeof(PlInstancePtr->User128Bit));
		switch(SlrNum)
		{
			case XSK_SLR_CONFIG_ORDER_0:
				/* Validation of 128 bit User Key */
				PlStatus = XilSKey_Efuse_ValidateKey(
						(char *)XSK_EFUSEPL_USER_KEY_128BIT_0_CONFIG_ORDER_0,
						XSK_EFUSEPL_USER_KEY_STRING_SIZE);
				if(PlStatus != XST_SUCCESS) {
					goto PL_INIT_ERROR;
				}
				PlStatus = XilSKey_Efuse_ValidateKey(
						(char *)XSK_EFUSEPL_USER_KEY_128BIT_1_CONFIG_ORDER_0,
						XSK_EFUSEPL_USER_KEY_STRING_SIZE);
				if(PlStatus != XST_SUCCESS) {
					goto PL_INIT_ERROR;
				}
				PlStatus = XilSKey_Efuse_ValidateKey(
						(char *)XSK_EFUSEPL_USER_KEY_128BIT_2_CONFIG_ORDER_0,
						XSK_EFUSEPL_USER_KEY_STRING_SIZE);
				if(PlStatus != XST_SUCCESS) {
					goto PL_INIT_ERROR;
				}
				PlStatus = XilSKey_Efuse_ValidateKey(
						(char *)XSK_EFUSEPL_USER_KEY_128BIT_3_CONFIG_ORDER_0,
						XSK_EFUSEPL_USER_KEY_STRING_SIZE);
				if(PlStatus != XST_SUCCESS) {
					goto PL_INIT_ERROR;
				}
				/* Assign the 128 bit User key [31:0]bits */
				XilSKey_Efuse_ConvertStringToHexLE(
					(char *)XSK_EFUSEPL_USER_KEY_128BIT_0_CONFIG_ORDER_0,
					&PlInstancePtr->User128Bit[0],
					XSK_EFUSEPL_USER_KEY_SIZE_IN_BITS);
				/* Assign the 128 bit User key [63:32]bits */
				XilSKey_Efuse_ConvertStringToHexLE(
					(char *)XSK_EFUSEPL_USER_KEY_128BIT_1_CONFIG_ORDER_0,
					&PlInstancePtr->User128Bit[4],
					XSK_EFUSEPL_USER_KEY_SIZE_IN_BITS);
				/* Assign the 128 bit User key [95:64]bits */
				XilSKey_Efuse_ConvertStringToHexLE(
					(char *)XSK_EFUSEPL_USER_KEY_128BIT_2_CONFIG_ORDER_0,
					&PlInstancePtr->User128Bit[8],
					XSK_EFUSEPL_USER_KEY_SIZE_IN_BITS);
				/* Assign the 128 bit User key [128:95]bits */
				XilSKey_Efuse_ConvertStringToHexLE(
					(char *)XSK_EFUSEPL_USER_KEY_128BIT_3_CONFIG_ORDER_0,
					&PlInstancePtr->User128Bit[12],
					XSK_EFUSEPL_USER_KEY_SIZE_IN_BITS);
				break;
			case XSK_SLR_CONFIG_ORDER_1:
				/* Validation of 128 bit User Key */
				PlStatus = XilSKey_Efuse_ValidateKey(
						(char *)XSK_EFUSEPL_USER_KEY_128BIT_0_CONFIG_ORDER_1,
						XSK_EFUSEPL_USER_KEY_STRING_SIZE);
				if(PlStatus != XST_SUCCESS) {
					goto PL_INIT_ERROR;
				}
				PlStatus = XilSKey_Efuse_ValidateKey(
						(char *)XSK_EFUSEPL_USER_KEY_128BIT_1_CONFIG_ORDER_1,
						XSK_EFUSEPL_USER_KEY_STRING_SIZE);
				if(PlStatus != XST_SUCCESS) {
					goto PL_INIT_ERROR;
				}
				PlStatus = XilSKey_Efuse_ValidateKey(
						(char *)XSK_EFUSEPL_USER_KEY_128BIT_2_CONFIG_ORDER_1,
						XSK_EFUSEPL_USER_KEY_STRING_SIZE);
				if(PlStatus != XST_SUCCESS) {
					goto PL_INIT_ERROR;
				}
				PlStatus = XilSKey_Efuse_ValidateKey(
						(char *)XSK_EFUSEPL_USER_KEY_128BIT_3_CONFIG_ORDER_1,
						XSK_EFUSEPL_USER_KEY_STRING_SIZE);
				if(PlStatus != XST_SUCCESS) {
					goto PL_INIT_ERROR;
				}
				/* Assign the 128 bit User key [31:0]bits */
				XilSKey_Efuse_ConvertStringToHexLE(
					(char *)XSK_EFUSEPL_USER_KEY_128BIT_0_CONFIG_ORDER_1,
					&PlInstancePtr->User128Bit[0],
					XSK_EFUSEPL_USER_KEY_SIZE_IN_BITS);
				/* Assign the 128 bit User key [63:32]bits */
				XilSKey_Efuse_ConvertStringToHexLE(
					(char *)XSK_EFUSEPL_USER_KEY_128BIT_1_CONFIG_ORDER_1,
					&PlInstancePtr->User128Bit[4],
					XSK_EFUSEPL_USER_KEY_SIZE_IN_BITS);
				/* Assign the 128 bit User key [95:64]bits */
				XilSKey_Efuse_ConvertStringToHexLE(
					(char *)XSK_EFUSEPL_USER_KEY_128BIT_2_CONFIG_ORDER_1,
					&PlInstancePtr->User128Bit[8],
					XSK_EFUSEPL_USER_KEY_SIZE_IN_BITS);
				/* Assign the 128 bit User key [128:95]bits */
				XilSKey_Efuse_ConvertStringToHexLE(
					(char *)XSK_EFUSEPL_USER_KEY_128BIT_3_CONFIG_ORDER_1,
					&PlInstancePtr->User128Bit[12],
					XSK_EFUSEPL_USER_KEY_SIZE_IN_BITS);
				break;
			case XSK_SLR_CONFIG_ORDER_2:
				/* Validation of 128 bit User Key */
				PlStatus = XilSKey_Efuse_ValidateKey(
						(char *)XSK_EFUSEPL_USER_KEY_128BIT_0_CONFIG_ORDER_2,
						XSK_EFUSEPL_USER_KEY_STRING_SIZE);
				if(PlStatus != XST_SUCCESS) {
					goto PL_INIT_ERROR;
				}
				PlStatus = XilSKey_Efuse_ValidateKey(
						(char *)XSK_EFUSEPL_USER_KEY_128BIT_1_CONFIG_ORDER_2,
						XSK_EFUSEPL_USER_KEY_STRING_SIZE);
				if(PlStatus != XST_SUCCESS) {
					goto PL_INIT_ERROR;
				}
				PlStatus = XilSKey_Efuse_ValidateKey(
						(char *)XSK_EFUSEPL_USER_KEY_128BIT_2_CONFIG_ORDER_2,
						XSK_EFUSEPL_USER_KEY_STRING_SIZE);
				if(PlStatus != XST_SUCCESS) {
					goto PL_INIT_ERROR;
				}
				PlStatus = XilSKey_Efuse_ValidateKey(
						(char *)XSK_EFUSEPL_USER_KEY_128BIT_3_CONFIG_ORDER_2,
						XSK_EFUSEPL_USER_KEY_STRING_SIZE);
				if(PlStatus != XST_SUCCESS) {
					goto PL_INIT_ERROR;
				}
				/* Assign the 128 bit User key [31:0]bits */
				XilSKey_Efuse_ConvertStringToHexLE(
					(char *)XSK_EFUSEPL_USER_KEY_128BIT_0_CONFIG_ORDER_2,
					&PlInstancePtr->User128Bit[0],
					XSK_EFUSEPL_USER_KEY_SIZE_IN_BITS);
				/* Assign the 128 bit User key [63:32]bits */
				XilSKey_Efuse_ConvertStringToHexLE(
					(char *)XSK_EFUSEPL_USER_KEY_128BIT_1_CONFIG_ORDER_2,
					&PlInstancePtr->User128Bit[4],
					XSK_EFUSEPL_USER_KEY_SIZE_IN_BITS);
				/* Assign the 128 bit User key [95:64]bits */
				XilSKey_Efuse_ConvertStringToHexLE(
					(char *)XSK_EFUSEPL_USER_KEY_128BIT_2_CONFIG_ORDER_2,
					&PlInstancePtr->User128Bit[8],
					XSK_EFUSEPL_USER_KEY_SIZE_IN_BITS);
				/* Assign the 128 bit User key [128:95]bits */
				XilSKey_Efuse_ConvertStringToHexLE(
					(char *)XSK_EFUSEPL_USER_KEY_128BIT_3_CONFIG_ORDER_2,
					&PlInstancePtr->User128Bit[12],
					XSK_EFUSEPL_USER_KEY_SIZE_IN_BITS);

				break;
			case XSK_SLR_CONFIG_ORDER_3:
				/* Validation of 128 bit User Key */
				PlStatus = XilSKey_Efuse_ValidateKey(
						(char *)XSK_EFUSEPL_USER_KEY_128BIT_0_CONFIG_ORDER_3,
						XSK_EFUSEPL_USER_KEY_STRING_SIZE);
				if(PlStatus != XST_SUCCESS) {
					goto PL_INIT_ERROR;
				}
				PlStatus = XilSKey_Efuse_ValidateKey(
						(char *)XSK_EFUSEPL_USER_KEY_128BIT_1_CONFIG_ORDER_3,
						XSK_EFUSEPL_USER_KEY_STRING_SIZE);
				if(PlStatus != XST_SUCCESS) {
					goto PL_INIT_ERROR;
				}
				PlStatus = XilSKey_Efuse_ValidateKey(
						(char *)XSK_EFUSEPL_USER_KEY_128BIT_2_CONFIG_ORDER_3,
						XSK_EFUSEPL_USER_KEY_STRING_SIZE);
				if(PlStatus != XST_SUCCESS) {
					goto PL_INIT_ERROR;
				}
				PlStatus = XilSKey_Efuse_ValidateKey(
						(char *)XSK_EFUSEPL_USER_KEY_128BIT_3_CONFIG_ORDER_3,
						XSK_EFUSEPL_USER_KEY_STRING_SIZE);
				if(PlStatus != XST_SUCCESS) {
					goto PL_INIT_ERROR;
				}
				/* Assign the 128 bit User key [31:0]bits */
				XilSKey_Efuse_ConvertStringToHexLE(
					(char *)XSK_EFUSEPL_USER_KEY_128BIT_0_CONFIG_ORDER_3,
					&PlInstancePtr->User128Bit[0],
					XSK_EFUSEPL_USER_KEY_SIZE_IN_BITS);
				/* Assign the 128 bit User key [63:32]bits */
				XilSKey_Efuse_ConvertStringToHexLE(
					(char *)XSK_EFUSEPL_USER_KEY_128BIT_1_CONFIG_ORDER_3,
					&PlInstancePtr->User128Bit[4],
					XSK_EFUSEPL_USER_KEY_SIZE_IN_BITS);
				/* Assign the 128 bit User key [95:64]bits */
				XilSKey_Efuse_ConvertStringToHexLE(
					(char *)XSK_EFUSEPL_USER_KEY_128BIT_2_CONFIG_ORDER_3,
					&PlInstancePtr->User128Bit[8],
					XSK_EFUSEPL_USER_KEY_SIZE_IN_BITS);
				/* Assign the 128 bit User key [128:95]bits */
				XilSKey_Efuse_ConvertStringToHexLE(
					(char *)XSK_EFUSEPL_USER_KEY_128BIT_3_CONFIG_ORDER_3,
					&PlInstancePtr->User128Bit[12],
					XSK_EFUSEPL_USER_KEY_SIZE_IN_BITS);
				break;
			default:
				PlStatus = XST_FAILURE;
				goto PL_INIT_ERROR;
				break;

		}
	}

	if (PlInstancePtr->CheckAESKeyUltra == TRUE)
	{

		switch(SlrNum)
		{/* Set expected AES Key CRC */
			case XSK_SLR_CONFIG_ORDER_0:
				PlInstancePtr->CrcOfAESKey = XSK_EFUSEPL_CRC_OF_EXPECTED_AES_KEY_CONFIG_ORDER_0;
				break;
			case XSK_SLR_CONFIG_ORDER_1:
				PlInstancePtr->CrcOfAESKey = XSK_EFUSEPL_CRC_OF_EXPECTED_AES_KEY_CONFIG_ORDER_1;
				break;
			case XSK_SLR_CONFIG_ORDER_2:
				PlInstancePtr->CrcOfAESKey = XSK_EFUSEPL_CRC_OF_EXPECTED_AES_KEY_CONFIG_ORDER_2;
				break;
			case XSK_SLR_CONFIG_ORDER_3:
				PlInstancePtr->CrcOfAESKey = XSK_EFUSEPL_CRC_OF_EXPECTED_AES_KEY_CONFIG_ORDER_3;
				break;
			default:
				PlStatus = XST_FAILURE;
				goto PL_INIT_ERROR;
				break;

		}
	}

PL_INIT_ERROR:
	return PlStatus;
}
#endif
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
	/* For Zynq PL efuse */
    /**
	 * Assign FUSE CNTRL bits[1:5] to the PL eFUSE structure elements.
	 */
#ifdef XSK_ARM_PLATFORM
		/* Assign FUSE CNTRL bits[1:5] to the PL eFUSE structure elements.*/

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

#else
		/* For Ultrascle efuse */
	/* eFuse control bits [2:0] */
	PlInstancePtr->AESKeyRead= XSK_EFUSEPL_DISABLE_AES_KEY_READ;
	PlInstancePtr->UserKeyRead = XSK_EFUSEPL_DISABLE_USER_KEY_READ;
	PlInstancePtr->SecureRead = XSK_EFUSEPL_DISABLE_SECURE_READ;
	/* eFuse Control bits [9:5] */
	PlInstancePtr->CtrlWrite = XSK_EFUSEPL_DISABLE_FUSE_CNTRL_WRITE;
	PlInstancePtr->RSARead = XSK_EFUSEPL_DISABLE_RSA_KEY_READ;
	PlInstancePtr->KeyWrite = XSK_EFUSEPL_DISABLE_KEY_WRITE;
	PlInstancePtr->UserKeyWrite = XSK_EFUSEPL_DISABLE_USER_KEY_WRITE;
	PlInstancePtr->SecureWrite = XSK_EFUSEPL_DISABLE_SECURE_WRITE;
	/* eFuse control bit 15 */
	PlInstancePtr->RSAWrite = XSK_EFUSEPL_DISABLE_RSA_HASH_WRITE;
	/* eFUSE control bit 16 */
	PlInstancePtr->User128BitWrite = XSK_EFUSEPL_DISABLE_128BIT_USER_KEY_WRITE;

	/* eFuse secure bits [5:0] */
	PlInstancePtr->EncryptOnly = XSK_EFUSEPL_ALLOW_ENCRYPTED_ONLY;
	PlInstancePtr->UseAESOnly = XSK_EFUSEPL_FORCE_USE_FUSE_AES_ONLY;
	PlInstancePtr->RSAEnable = XSK_EFUSEPL_ENABLE_RSA_AUTH;
	PlInstancePtr->JtagDisable = XSK_EFUSEPL_DISABLE_JTAG_CHAIN;
	PlInstancePtr->IntTestAccessDisable = XSK_EFUSEPL_DISABLE_TEST_ACCESS;
	PlInstancePtr->DecoderDisable = XSK_EFUSEPL_DISABLE_AES_DECRYPTOR;
	PlInstancePtr->FuseObfusEn = XSK_EFUSEPL_ENABLE_OBFUSCATION_EFUSEAES;

	PlInstancePtr->ProgAESKeyUltra = XSK_EFUSEPL_PROGRAM_AES_KEY;
	PlInstancePtr->ProgUserKeyUltra = XSK_EFUSEPL_PROGRAM_USER_KEY;
	PlInstancePtr->ProgRSAKeyUltra = XSK_EFUSEPL_PROGRAM_RSA_KEY_HASH;
	PlInstancePtr->ProgUser128BitUltra = XSK_EFUSEPL_PROGRAM_USER_KEY_128BIT;

	PlInstancePtr->ReadUserKeyUltra = XSK_EFUSEPL_READ_USER_KEY;
	PlInstancePtr->ReadRSAKeyUltra = XSK_EFUSEPL_READ_RSA_KEY_HASH;
	PlInstancePtr->CheckAESKeyUltra = XSK_EFUSEPL_CHECK_AES_KEY_CRC;
	PlInstancePtr->ReadUser128BitUltra = XSK_EFUSEPL_READ_USER_KEY128_BIT;


	PlInstancePtr->JtagGpioID  = XSK_EFUSEPL_AXI_GPIO_DEVICE_ID;
	PlInstancePtr->JtagGpioTDI = XSK_EFUSEPL_AXI_GPIO_JTAG_TDI;
	PlInstancePtr->JtagGpioTMS = XSK_EFUSEPL_AXI_GPIO_JTAG_TMS;
	PlInstancePtr->JtagGpioTCK = XSK_EFUSEPL_AXI_GPIO_JTAG_TCK;
	PlInstancePtr->JtagGpioTDO = XSK_EFUSEPL_AXI_GPIO_JTAG_TDO;

	PlInstancePtr->HwmGpioReady = XSK_EFUSEPL_AXI_GPIO_HWM_READY;
	PlInstancePtr->HwmGpioStart = XSK_EFUSEPL_AXI_GPIO_HWM_START;
	PlInstancePtr->HwmGpioEnd = XSK_EFUSEPL_AXI_GPIO_HWM_END;

	PlInstancePtr->GpioInputCh = XSK_EFUSEPL_GPIO_INPUT_CH;
	PlInstancePtr->GpioOutPutCh = XSK_EFUSEPL_GPIO_OUTPUT_CH;

	/* Variable to check whether internal system initialization is done.*/
	PlInstancePtr->SystemInitDone = 0;
	goto PL_INIT_ERROR; /*To avoid warning only when PL driver is enabled*/

#endif

PL_INIT_ERROR:
	return PlStatus;
}

#endif /*XSK_EFUSEPL_DRIVER*/
