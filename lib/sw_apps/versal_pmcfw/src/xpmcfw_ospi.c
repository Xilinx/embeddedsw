/******************************************************************************
*
* Copyright (C) 2018 Xilinx, Inc.  All rights reserved.
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
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
* THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
* THE SOFTWARE.
*
* 
*
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xpmcfw_ospi.c
*
* This is the file which contains ospi related code for the PMCFW.
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
#include "xpmcfw_ospi.h"

#ifdef XPMCFW_OSPI

/************************** Constant Definitions *****************************/
/*
 * The following constants map to the XPAR parameters created in the
 * xparameters.h file. They are defined here such that a user can easily
 * change all the needed parameters in one place.
 */

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/
static XStatus FlashReadID(XOspiPsv *OspiPsvPtr);

/************************** Variable Definitions *****************************/
static XOspiPsv OspiPsvInstance;
static XOspiPsv_Msg FlashMsg;
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
* it presently supports Micron 512Mb.
*
* @param	Ospi Instance Pointer
*
* @return	XST_SUCCESS if read id, otherwise XST_FAILURE.
*
* @note		None.
*
******************************************************************************/
static XStatus FlashReadID(XOspiPsv *OspiPsvPtr)
{
	XStatus Status;

	/*
	 * Read ID
	 */
	FlashMsg.Opcode = READ_ID;
	FlashMsg.Addrsize = 0;
	FlashMsg.Addrvalid = 0;
	FlashMsg.TxBfrPtr = NULL;
	FlashMsg.RxBfrPtr = ReadBuffer;
	FlashMsg.ByteCount = 8U;
	FlashMsg.Flags = XOSPIPSV_MSG_FLAG_RX;
	Status = XOspiPsv_PollTransfer(OspiPsvPtr, &FlashMsg);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	XPmcFw_Printf(DEBUG_GENERAL, "FlashID=0x%x 0x%x 0x%x\n\r", ReadBuffer[0],
                                        ReadBuffer[1], ReadBuffer[2]);

        /*
         * Deduce flash make
         */
	if (ReadBuffer[0] != MICRON_OCTAL_ID_BYTE0) {
                Status = XPMCFW_ERR_UNSUPPORTED_OSPI;
                XPmcFw_Printf(DEBUG_GENERAL,"XPMCFW_ERR_UNSUPPORTED_OSPI\r\n");
                goto END;
        }
	else
	{
		OspiFlashMake = MICRON_OCTAL_ID_BYTE0;
		StatusCmd = READ_FLAG_STATUS_CMD;
	}

	if(ReadBuffer[2] == MICRON_OCTAL_ID_BYTE2_512) {
		OspiFlashSize = FLASH_SIZE_512M;
	} else {
            Status = XPMCFW_ERR_UNSUPPORTED_OSPI_SIZE;
            XPmcFw_Printf(DEBUG_GENERAL,"XPMCFW_ERR_UNSUPPORTED_OSPI_SIZE\r\n");
            goto END;
        }

END:
	return Status;
}


/*****************************************************************************/
/**
 * This function is used to initialize the ospi controller and driver
 *
 * @param	None
 *
 * @return	None
 *
 *****************************************************************************/
XStatus XPmcFw_OspiInit(u32 DeviceFlags)
{
	XOspiPsv_Config *OspiConfig;
	XOspiPsv* OspiPsvInstancePtr = &OspiPsvInstance;
	XStatus Status;
	u32 OspiMode;


	/**
	 * Initialize the OSPI driver so that it's ready to use
	 */
	OspiConfig =  XOspiPsv_LookupConfig(XPMCFW_OSPI_DEVICE_ID);
	if (NULL == OspiConfig) {
		Status = XPMCFW_ERR_OSPI_INIT;
		XPmcFw_Printf(DEBUG_GENERAL,"XPMCFW_ERR_OSPI_INIT\r\n");
		goto END;
	}

	Status =  XOspiPsv_CfgInitialize(OspiPsvInstancePtr, OspiConfig);
	if (Status != XPMCFW_SUCCESS) {
		Status = XPMCFW_ERR_OSPI_CFG;
		XPmcFw_Printf(DEBUG_GENERAL,"XPMCFW_ERR_OSPI_CFG\r\n");
		goto END;
	}

	/*
	 * Enable IDAC controller in OSPI
	 */
	XOspiPsv_SetOptions(OspiPsvInstancePtr, XOSPIPSV_IDAC_EN_OPTION);

	/*
	 * Set the prescaler for OSPIPSV clock
	 */

	XOspiPsv_SetClkPrescaler(OspiPsvInstancePtr, XOSPIPSV_CLK_PRESCALE_15);
	Status = XOspiPsv_SelectFlash(OspiPsvInstancePtr, XOSPIPSV_SELECT_FLASH_CS0);
	if (Status != XST_SUCCESS)
		goto END;
	/*
	 * Read flash ID and obtain all flash related information
	 * It is important to call the read id function before
	 * performing proceeding to any operation, including
	 * preparing the WriteBuffer
	 */

	Status = FlashReadID(OspiPsvInstancePtr);
	if (Status != XST_SUCCESS) {
		goto END;
	}
	XPmcFw_FlashEnterExit4BAddMode(&OspiPsvInstance, 1U);

END:
	return Status;
}

/*****************************************************************************/
/**
 * This function is used to copy the data from OSPI flash to destination
 * address
 *
 * @param SrcAddr is the address of the OSPI flash where copy should
 * start from
 *
 * @param DestAddr is the address of the destination where it
 * should copy to
 *
 * @param Length Length of the bytes to be copied
 *
 * @return
 *		- XPMCFW_SUCCESS for successful copy
 *		- errors as mentioned in xpmcfw_error.h
 *
 *****************************************************************************/
XStatus XPmcFw_OspiCopy(u32 SrcAddr, u64 DestAddr, u32 Length, u32 Flags)
{

	XStatus Status;

	XPmcFw_Printf(DEBUG_INFO,"OSPI Reading Src 0x%0x, Dest 0x%0x,"
		"Length 0x%0x, Flags 0x%0x\r\n", SrcAddr, (u32)(DestAddr&(0XFFFFFFFFU)),
		Length, Flags);

	FlashMsg.Opcode = READ_CMD_OCTAL_4B;
	FlashMsg.Addrsize = 4U;
	FlashMsg.Addrvalid = 1U;
	FlashMsg.TxBfrPtr = NULL;
	FlashMsg.RxBfrPtr = DestAddr;
	FlashMsg.ByteCount = Length;
	FlashMsg.Flags = XOSPIPSV_MSG_FLAG_RX;
	FlashMsg.Addr = SrcAddr;
	FlashMsg.Proto = XOSPIPSV_READ_1_1_8;
	FlashMsg.Dummy = 8U;

	Status = XOspiPsv_PollTransfer(&OspiPsvInstance, &FlashMsg);
	if (Status != XST_SUCCESS) {
		goto END;
	}

END:
	return Status;
}

/*****************************************************************************/
/**
 * This function is used to release the Ospi settings
 *
 * @param	None
 *
 * @return	None
 *
 *****************************************************************************/
XStatus XPmcFw_OspiRelease(void)
{
	XStatus Status = XPMCFW_SUCCESS;
	XPmcFw_FlashEnterExit4BAddMode(&OspiPsvInstance, 0U);
	return Status;
}

/*****************************************************************************/
/**
* This API enters the flash device into 4 bytes addressing mode.
* As per the Micron spec, before issuing the command to enter into 4 byte addr
* mode, a write enable command is issued.
*
* @param        OspiPtr is a pointer to the OSPIPSV driver component to use.
* @param        Enable is a either 1 or 0 if 1 then enters 4 byte if 0 exits.
*
* @return        - XST_SUCCESS if successful.
*                - XST_FAILURE if it fails.
*
*
******************************************************************************/
XStatus XPmcFw_FlashEnterExit4BAddMode(XOspiPsv *OspiPsvPtr, u32 Enable)
{
        u32 Status;
        u32 Command;
        u32 FlashStatus;

	if(Enable)
	{
		Command = ENTER_4B_ADDR_MODE;
	}
	else
	{
		Command = EXIT_4B_ADDR_MODE;
	}

        switch (OspiFlashMake) {
                case MICRON_OCTAL_ID_BYTE0:
                        FlashMsg.Opcode = WRITE_ENABLE_CMD;
                        FlashMsg.Addrsize = 0;
                        FlashMsg.Addrvalid = 0;
                        FlashMsg.TxBfrPtr = NULL;
                        FlashMsg.RxBfrPtr = NULL;
                        FlashMsg.ByteCount = 0;
                        FlashMsg.Flags = XOSPIPSV_MSG_FLAG_TX;

                        Status = XOspiPsv_PollTransfer(OspiPsvPtr, &FlashMsg);
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
        FlashMsg.Flags = XOSPIPSV_MSG_FLAG_TX;
        FlashMsg.Addrsize = 3;

        Status = XOspiPsv_PollTransfer(OspiPsvPtr, &FlashMsg);
        if (Status != XST_SUCCESS)
                return XST_FAILURE;

        while (1) {
                FlashMsg.Opcode = StatusCmd;
                FlashMsg.Addrsize = 3;
                FlashMsg.Addrvalid = 0;
                FlashMsg.TxBfrPtr = NULL;
                FlashMsg.RxBfrPtr = &FlashStatus;
                FlashMsg.ByteCount = 1;
                FlashMsg.Flags = XOSPIPSV_MSG_FLAG_RX;

                Status = XOspiPsv_PollTransfer(OspiPsvPtr, &FlashMsg);
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
                FlashMsg.Flags = XOSPIPSV_MSG_FLAG_TX;

                Status = XOspiPsv_PollTransfer(OspiPsvPtr, &FlashMsg);
                if (Status != XST_SUCCESS)
                        return XST_FAILURE;
                break;

                default:
                        break;
        }
       return Status;

}

#endif
