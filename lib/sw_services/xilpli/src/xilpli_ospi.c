/******************************************************************************
* Copyright (C) 2018 Xilinx, Inc. All rights reserved.
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
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xilpli_ospi.c
*
* This is the file which contains ospi related code for the PLM.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- -------- -------------------------------------------------------
* 1.00  bsv   08/23/2018 Initial release
*
* </pre>
*
* @note
*
******************************************************************************/
/***************************** Include Files *********************************/
#include "xilpli_ospi.h"
#include "xilpli.h"
#include "xplmi_hw.h"

#ifdef XILPLI_OSPI

/************************** Constant Definitions *****************************/
/*
 * The following constants map to the XPAR parameters created in the
 * xparameters.h file. They are defined here such that a user can easily
 * change all the needed parameters in one place.
 */

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions ********************/
#define XILPLI_OSPI_DEVICE_ID  XPAR_PSU_OSPI_0_DEVICE_ID
#define XILPLI_OSPI_BASEADDR   XPAR_PSU_OSPI_0_BASEADDR 
/************************** Function Prototypes ******************************/
static int FlashReadID(XOspiPs *OspiPsPtr);

/************************** Variable Definitions *****************************/
static XOspiPs OspiPsInstance;
static XOspiPs_Msg FlashMsg;
u32 StatusCmd;
u32 OspiFlashMake;
u32 OspiFlashSize;
static u8 ReadBuffer[10] __attribute__ ((aligned(32)));
static u8 WriteBuffer[10] __attribute__ ((aligned(32)));
/******************************************************************************
*
* This function reads serial FLASH ID connected to the SPI interface.
* It then deduces the make and size of the flash and obtains the
* connection mode to point to corresponding parameters in the flash
* configuration table. The flash driver will function based on this and
* it presently supports Micron and Spansion - 128, 256 and 512Mbit and
* Winbond 128Mbit
*
* @param	none
*
* @return	XST_SUCCESS if read id, otherwise XST_FAILURE.
*
* @note		None.
*
******************************************************************************/
static int FlashReadID(XOspiPs *OspiPsPtr)
{
	int Status;

	/*
	 * Read ID
	 */
	FlashMsg.Opcode = READ_ID;
	FlashMsg.Addrsize = 0;
	FlashMsg.Addrvalid = 0;
	FlashMsg.TxBfrPtr = NULL;
	FlashMsg.RxBfrPtr = ReadBuffer;
	FlashMsg.ByteCount = 8U;
	FlashMsg.Flags = XOSPIPS_MSG_FLAG_RX;
	Status = XOspiPs_PollTransfer(OspiPsPtr, &FlashMsg);
	if (Status != XST_SUCCESS) {
		goto END;
	}
	
	XPli_Printf(DEBUG_GENERAL, "FlashID=0x%x 0x%x 0x%x\n\r", ReadBuffer[0],
                                        ReadBuffer[1], ReadBuffer[2]);

        /*
         * Deduce flash make
         */
       	if (ReadBuffer[0] != MICRON_OCTAL_ID_BYTE0) {
                Status = XILPLI_ERR_UNSUPPORTED_OSPI;
                XPli_Printf(DEBUG_GENERAL,"XILPLI_ERR_UNSUPPORTED_OSPI\r\n");
                goto END;
        }
	else
	{
		OspiFlashMake = MICRON_OCTAL_ID_BYTE0;
		StatusCmd = READ_FLAG_STATUS_CMD;
	}

	/*
	 * If valid flash ID, then check connection mode & size and
	 * assign corresponding index in the Flash configuration table
	 */
	if(ReadBuffer[2] == MICRON_OCTAL_ID_BYTE2_512) {
		OspiFlashSize = FLASH_SIZE_512M;
	} else {
            Status = XILPLI_ERR_UNSUPPORTED_OSPI_SIZE;
            XPli_Printf(DEBUG_GENERAL,"XILPLI_ERR_UNSUPPORTED_OSPI_SIZE\r\n");
            goto END;
        }

	FlashEnterExit4BAddMode(OspiPsPtr);

END:
	return Status;
}


/*****************************************************************************/
/**
 * This function is used to initialize the qspi controller and driver
 *
 * @param	None
 *
 * @return	None
 *
 *****************************************************************************/
int XPli_OspiInit(u32 DeviceFlags)
{
	XOspiPs_Config *OspiConfig;
	XOspiPs* OspiPsInstancePtr = &OspiPsInstance;
	int Status;
	u32 OspiMode;


	/**
	 * Initialize the QSPI driver so that it's ready to use
	 */
	OspiConfig =  XOspiPs_LookupConfig(XILPLI_OSPI_DEVICE_ID);
	if (NULL == OspiConfig) {
		Status = XILPLI_ERR_OSPI_INIT;
		XPli_Printf(DEBUG_GENERAL,"XILPLI_ERR_OSPI_INIT\r\n");
		goto END;
	}

	Status =  XOspiPs_CfgInitialize(OspiPsInstancePtr, OspiConfig);
	if (Status != XILPLI_SUCCESS) {
		Status = XILPLI_ERR_OSPI_CFG;
		XPli_Printf(DEBUG_GENERAL,"XILPLI_ERR_OSPI_CFG\r\n");
		goto END;
	}

	/*
	 * Enable IDAC controller in OSPI
	 */
	XOspiPs_SetOptions(OspiPsInstancePtr, XOSPIPS_IDAC_EN_OPTION);
	
	/*
	 * Set the prescaler for OSPIPS clock
	 */

	XOspiPs_SetClkPrescaler(OspiPsInstancePtr, XOSPIPS_CLK_PRESCALE_15);
	Status = XOspiPs_SelectFlash(OspiPsInstancePtr, XOSPIPS_SELECT_FLASH_CS0);
	if (Status != XST_SUCCESS)
		goto END;
	/*
	 * Read flash ID and obtain all flash related information
	 * It is important to call the read id function before
	 * performing proceeding to any operation, including
	 * preparing the WriteBuffer
	 */

	Status = FlashReadID(OspiPsInstancePtr);
	if (Status != XST_SUCCESS) {
		goto END;
	}


END:
	return Status;
}

/*****************************************************************************/
/**
 * This function is used to copy the data from OSPI flash to destination
 * address
 *
 * @param SrcAddr is the address of the QSPI flash where copy should
 * start from
 *
 * @param DestAddr is the address of the destination where it
 * should copy to
 *
 * @param Length Length of the bytes to be copied
 *
 * @return
 *		- XILPLI_SUCCESS for successful copy
 *		- errors as mentioned in xilpli_error.h
 *
 *****************************************************************************/
int XPli_OspiCopy(u32 SrcAddr, u64 DestAddr, u32 Length, u32 Flags)
{
	
	int Status;

	XPli_Printf(DEBUG_INFO,"OSPI Reading Src 0x%0x, Dest 0x%0x," 
		"Length 0x%0x, Flags 0x%0x\r\n", SrcAddr, (u32)(DestAddr&(0XFFFFFFFFU)), 
		Length, Flags);
			
	/*
	 * Read ID
	 */
	FlashMsg.Opcode = READ_CMD_OCTAL_4B;
	FlashMsg.Addrsize = 4U;
	FlashMsg.Addrvalid = 1U;
	FlashMsg.TxBfrPtr = NULL;
	FlashMsg.RxBfrPtr = DestAddr;
	FlashMsg.ByteCount = Length;
	FlashMsg.Flags = XOSPIPS_MSG_FLAG_RX;
	FlashMsg.Addr = SrcAddr;
	FlashMsg.Proto = XOSPIPS_READ_1_1_8;
	FlashMsg.Dummy = 8U;
	
	Status = XOspiPs_PollTransfer(&OspiPsInstance, &FlashMsg);
	if (Status != XST_SUCCESS) {
		goto END;
	}

END:
	return Status;
}

/*****************************************************************************/
/**
 * This function is used to release the Qspi settings
 *
 * @param	None
 *
 * @return	None
 *
 *****************************************************************************/
int XPli_OspiRelease(void)
{
	int Status = XILPLI_SUCCESS;

	return Status;
}

/*****************************************************************************/
/**
* This API enters the flash device into 4 bytes addressing mode.
* As per the Micron spec, before issuing the command to enter into 4 byte addr
* mode, a write enable command is issued.
*
* @param        QspiPtr is a pointer to the QSPIPSU driver component to use.
* @param        Enable is a either 1 or 0 if 1 then enters 4 byte if 0 exits.
*
* @return        - XST_SUCCESS if successful.
*                - XST_FAILURE if it fails.
*
*
******************************************************************************/
int FlashEnterExit4BAddMode(XOspiPs *OspiPsPtr)
{
        u32 Status;
        u32 Command = ENTER_4B_ADDR_MODE;
        u32 FlashStatus;

        switch (OspiFlashMake) {
                case MICRON_OCTAL_ID_BYTE0:
                        FlashMsg.Opcode = WRITE_ENABLE_CMD;
                        FlashMsg.Addrsize = 0;
                        FlashMsg.Addrvalid = 0;
                        FlashMsg.TxBfrPtr = NULL;
                        FlashMsg.RxBfrPtr = NULL;
                        FlashMsg.ByteCount = 0;
                        FlashMsg.Flags = XOSPIPS_MSG_FLAG_TX;

                        Status = XOspiPs_PollTransfer(OspiPsPtr, &FlashMsg);
                        if (Status != XST_SUCCESS)
                                return XST_FAILURE;
                        break;

                default:
                        break;
        }
        FlashMsg.Opcode = Command;
        FlashMsg.Addrvalid = 0;
        FlashMsg.TxBfrPtr = NULL;
        FlashMsg.RxBfrPtr = NULL;
        FlashMsg.ByteCount = 0;
        FlashMsg.Flags = XOSPIPS_MSG_FLAG_TX;
        FlashMsg.Addrsize = 3;

        Status = XOspiPs_PollTransfer(OspiPsPtr, &FlashMsg);
        if (Status != XST_SUCCESS)
                return XST_FAILURE;

        while (1) {
                FlashMsg.Opcode = StatusCmd;
                FlashMsg.Addrsize = 3;
                FlashMsg.Addrvalid = 0;
                FlashMsg.TxBfrPtr = NULL;
                FlashMsg.RxBfrPtr = &FlashStatus;
                FlashMsg.ByteCount = 1;
                FlashMsg.Flags = XOSPIPS_MSG_FLAG_RX;

                Status = XOspiPs_PollTransfer(OspiPsPtr, &FlashMsg);
                if (Status != XST_SUCCESS)
                        return XST_FAILURE;

                if ((FlashStatus & 0x80) != 0)
                        break;

        }
     switch (OspiFlashMake) {
        case MICRON_OCTAL_ID_BYTE0:
                FlashMsg.Opcode = WRITE_DISABLE_CMD;
                FlashMsg.Addrsize = 0;
                FlashMsg.Addrvalid = 0;
                FlashMsg.TxBfrPtr = NULL;
                FlashMsg.RxBfrPtr = NULL;
                FlashMsg.ByteCount = 0;
                FlashMsg.Flags = XOSPIPS_MSG_FLAG_TX;

                Status = XOspiPs_PollTransfer(OspiPsPtr, &FlashMsg);
                if (Status != XST_SUCCESS)
                        return XST_FAILURE;
                break;

                default:
                        break;
        }
        return Status;

}

#endif
