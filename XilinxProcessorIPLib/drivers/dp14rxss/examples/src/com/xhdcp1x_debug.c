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
* @file xhdcp_debug.c
*
* This file provides the implement of the HDCP debug commands
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who    Date     Changes
* ----- ------ -------- --------------------------------------------------
* 1.00         07/16/15 Initial release.
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "xdebug.h"
#include "xil_io.h"
#include "xil_types.h"
#include "xparameters.h"
#if (XPAR_XHDCP_NUM_INSTANCES > 0)
#include "xhdcp1x.h"
#include "xhdcp1x_example.h"
#include "xhdcp1x_port.h"
#include "xstatus.h"

/************************** Constant Definitions *****************************/

#define MAX_CMD_LENGTH			(40)
#define MAX_ARG_NUM			(10)

/***************** Macros (Inline Functions) Definitions *********************/

#define PRINTF				xil_printf

#define DO_ECHO

/**************************** Type Definitions *******************************/

/****************************** Local Globals ********************************/
static int gCmdInProgress = FALSE;
static int gCmdBufIdx = 0;
static char gCmdBuf[MAX_CMD_LENGTH];

/**************************** External Globals *******************************/

/************************** Function Prototypes ******************************/

/*****************************************************************************/
/**
*
* This function looks up the HDCP interface from a device id value
*
* @param DeviceID  the device id
*
* @return
*   Pointer to the HDCP interface.  NULL if not found
*
* @note
*   None.
*
******************************************************************************/
#define GetHdcpIf(DeviceID)		XHdcp1xExample_Get(DeviceID)

/*****************************************************************************/
/**
*
* This function looks up the HDCP port device from a device id value
*
* @param DeviceID  the device id
*
* @return
*   Pointer to the HDCP port device.  NULL if not found
*
* @note
*   None.
*
******************************************************************************/
XHdcp1x *GetPort(u16 DeviceID)
{
#if 0
	XHdcp1x *InstancePtr = GetHdcpIf(DeviceID);
	XHdcp1x_Port* PortPtr = NULL;

	if (InstancePtr != NULL) {
		PortPtr = &(InstancePtr->Port);
	}

	return (PortPtr);
#endif
	return GetHdcpIf(DeviceID);
}


/*****************************************************************************/
/**
 *
 * This function implements the "hdcp cipher read" command
 *
 * @param argc    the number of arguments
 * @param argv[]  an array of pointers to the arguments
 *
 * @return
 *   void
 *
 * @note
 *   None.
 *
 *****************************************************************************/
static void DebugCipherRead(int argc, const char* argv[])
{
	XHdcp1x *InstancePtr = NULL;
	int DeviceID = 0;

	/* Check for DeviceID */
	if (argc > 0) {
		DeviceID = atoi(argv[0]);
	}

	/* Determine InstancePtr */
	InstancePtr = GetHdcpIf(DeviceID);

	/* Sanity Check */
	if (InstancePtr != NULL) {

		/* More locals */
		u32 Base = InstancePtr->Config.BaseAddress;
		u32 Address = 0;
		u32 NumToRead = 0xA0u;
		u32 NumDone = 0;

		/* Check for the Address */
		if (argc > 1) {
			Address = strtoul(argv[1], NULL, 16);
		}

		/* Check for the NumToRead */
		if (argc > 2) {
			NumToRead = (unsigned int) strtoul(argv[2], NULL, 16);
		}

		/* Adjust/truncate as needed */
		Address &= 0xFFul;
		if ((Address + (NumToRead << 2)) > 0xA0u) {
			NumToRead = ((0xA0u - Address) >> 2);
		}
		if (NumToRead == 0) {
			NumToRead = 1;
		}

		/* Iterate through the reads */
		do {
			/* Check for start-of-line */
			if ((NumDone % 4) == 0) {
				PRINTF("%02X: ", Address);
			}

			/* Read and display it */
			PRINTF(" %08lX", Xil_In32(Base+Address));

			/* Check for end-of-line */
			if ((NumDone % 4) == 3) {
				PRINTF("\r\n");
			}

			/* Update for loop */
			Address += 4;
			NumDone++;
			NumToRead--;
		}
		while (NumToRead > 0);

		/* Check for extra end-of-line */
		if ((NumDone % 4) != 0) {
			PRINTF("\r\n");
		}

	} else {
		PRINTF("hdcp cipher read <id> <address> <num-to-read>\r\n");
	}

	return;
}

/*****************************************************************************/
/**
 *
 * This function implements the "hdcp cipher write" command
 *
 * @param argc    the number of arguments
 * @param argv[]  an array of pointers to the arguments
 *
 * @return
 *   void
 *
 * @note
 *   None.
 *
 *****************************************************************************/
static void DebugCipherWrite(int argc, const char* argv[])
{
	int ThereWasAProblem = TRUE;

	/* Sanity Check */
	if (argc > 2) {

		/* More locals */
		XHdcp1x *InstancePtr = GetHdcpIf(atoi(argv[0]));

		/* Sanity Check (again) */
		if (InstancePtr != NULL) {

			/* More locals */
			u32 Base = InstancePtr->Config.BaseAddress;
			u32 Address = strtoul(argv[1], NULL, 16);
			u32 Value = strtoul(argv[2], NULL, 16);

			/* Write it */
			Xil_Out32((Base+Address), Value);

			/* Update ThereWasAProblem */
			ThereWasAProblem = FALSE;
		}
	}

	/* Check for help */
	if (ThereWasAProblem) {
		PRINTF("hdcp cipher write <id> <address> <value>\r\n");
	}

	return;
}

/*****************************************************************************/
/**
 *
 * This function implements the "hdcp cipher debug" command
 *
 * @param argc    the number of arguments
 * @param argv[]  an array of pointers to the arguments
 *
 * @return
 *   void
 *
 * @note
 *   None.
 *
 *****************************************************************************/
static void DebugCipher(int argc, const char* argv[])
{
	int ThereWasAProblem = FALSE;

	/* CR/LF */
	PRINTF("\r\n");

	/* Sanity Check */
	if (argc > 1) {

		/* Check for read */
		if (strcmp(argv[1], "read") == 0) {
			DebugCipherRead(argc - 2, argv + 2);
		}
		/* Check for write */
		else if (strcmp(argv[1], "write") == 0) {
			DebugCipherWrite(argc - 2, argv + 2);
		}
		/* Otherwise */
		else {
			ThereWasAProblem = TRUE;
		}

		/* Otherwise */
	} else {
		ThereWasAProblem = TRUE;
	}

	/* Check for help */
	if (ThereWasAProblem) {
		PRINTF("hdcp cipher <read|write>\r\n");
	}

	/* CR/LF */
	PRINTF("\r\n");

	return;
}

/*****************************************************************************/
/**
 *
 * This function implements the "hdcp port read" command
 *
 * @param argc    the number of arguments
 * @param argv[]  an array of pointers to the arguments
 *
 * @return
 *   void
 *
 * @note
 *   None.
 *
 *****************************************************************************/
static void DebugPortRead(int argc, const char* argv[])
{
	XHdcp1x *InstancePtr = NULL;
	int DeviceID = 0;

	/* Check for theIfID */
	if (argc > 0)
		DeviceID = atoi(argv[0]);

	/* Determine InstancePtr */
	InstancePtr = GetPort(DeviceID);

	/* Sanity Check */
	if (InstancePtr != NULL) {

		/* More locals */
		u8 Offset = 0;
		u32 NumToRead = 0x100u;
		u32 NumDone = 0;

		/* Check for Offset */
		if (argc > 1)
			Offset = (u8) strtoul(argv[1], NULL, 16);

		/* Check for NumToRead */
		if (argc > 2)
			NumToRead = (unsigned int) strtoul(argv[2], NULL, 16);

		/* Adjust/truncate as needed */
		if ((NumToRead + Offset) > 0x100u)
			NumToRead = (0x100u - Offset);
		if (NumToRead == 0)
			NumToRead = 1;

		/* Iterate through the reads */
		do {

			/* Loop variables */
			u8 Value = 0;

			/* Check for start-of-line */
			if ((NumDone % 16) == 0)
				PRINTF("%02X: ", Offset);

			/* Attempt to read */
			if (XHdcp1x_PortRead(InstancePtr, Offset, &Value, 1) > 0)
				PRINTF(" %02X", Value);
			/* Otherwise */
			else
				PRINTF(" ??");

			/* Check for end-of-line */
			if ((NumDone % 16) == 15)
				PRINTF("\r\n");

			/* Update for loop */
			Offset++;
			NumDone++;
			NumToRead--;

		} while (NumToRead > 0);

		/* Check for extra end-of-line */
		if ((NumDone % 16) != 0)
			PRINTF("\r\n");

	} else {
		PRINTF("hdcp port read <id> <offset> <num-to-read>\r\n");
	}

	return;
}

/*****************************************************************************/
/**
 *
 * This function implements the "hdcp port write" command
 *
 * @param argc    the number of arguments
 * @param argv[]  an array of pointers to the arguments
 *
 * @return
 *   void
 *
 * @note
 *   None.
 *
 *****************************************************************************/
static void DebugPortWrite(int argc, const char* argv[])
{
	int ThereWasAProblem = TRUE;

	/* Sanity Check */
	if (argc > 2) {

		/* More locals */
		XHdcp1x *InstancePtr = GetPort(atoi(argv[0]));

		/* Sanity Check (again) */
		if (InstancePtr != NULL) {

			/* More locals */
			u8 Offset = (u8) strtoul(argv[1], NULL, 16);
			u8 Value = (u8) strtoul(argv[2], NULL, 16);

			/* Write it */
			XHdcp1x_PortWrite(InstancePtr, Offset, &Value, 1);

			/* Update ThereWasAProblem */
			ThereWasAProblem = FALSE;
		}
	}

	/* Check for help */
	if (ThereWasAProblem) {
		PRINTF("hdcp port write <id> <offset> <value>\r\n");
	}

	return;
}

/*****************************************************************************/
/**
 *
 * This function implements the "hdcp port" command
 *
 * @param argc  the number of command line arguments
 * @param argv  the list of the command line arguments
 *
 * @return
 *   Always returns zero (0)
 *
 * @note
 *   None.
 *
 *****************************************************************************/
static void DebugPort(int argc, const char* argv[])
{
	/* Locals */
	int ThereWasAProblem = FALSE;

	/* Sanity Check */
	if (argc > 1) {

		/* Check for read */
		if (strcmp(argv[1], "read") == 0)
			DebugPortRead(argc - 2, argv + 2);
		/* Check for write */
		else if (strcmp(argv[1], "write") == 0)
			DebugPortWrite(argc - 2, argv + 2);
		/* Otherwise */
		else
			ThereWasAProblem = TRUE;

		/* Otherwise */
	} else {
		ThereWasAProblem = TRUE;
	}

	/* Check for help */
	if (ThereWasAProblem) {
		PRINTF("hdcp port <read|write>");
	}

	return;
}

/*****************************************************************************/
/*!
 * \brief  This function implements the "hdcp authenticate" command
 * \param[in]  argc    the number of arguments
 * \param[in]  argv[]  an array of pointers to the arguments
 * \return  void
 *
 *****************************************************************************/
static void DebugAuthenticate(int argc, const char *argv[])
{
	XHdcp1x *InstancePtr = NULL;
	int DevID = 0;
	int ThereWasAProblem = FALSE;

	/* Check for theIfID */
	if (argc > 0) {
		DevID = atoi(argv[0]);
	}

	/* Determine InstancePtr */
	InstancePtr = GetHdcpIf(DevID);

	/* Sanity Check */
	if (InstancePtr != NULL) {
		if (XHdcp1x_Authenticate(InstancePtr) != XST_SUCCESS) {
			ThereWasAProblem = TRUE;
		}
	}
	else {
		ThereWasAProblem = TRUE;
	}

	/* Check for help */
	if (ThereWasAProblem) {
		PRINTF("hdcp authenticate <id>\r\n");
	}

	return;
}

/*****************************************************************************/
/*!
 * \brief  This function implements the "hdcp display" command
 * \param[in]  argc    the number of arguments
 * \param[in]  argv[]  an array of pointers to the arguments
 * \return  void
 *
 *****************************************************************************/
static void DebugDisplay(int argc, const char *argv[])
{
	const XHdcp1x *InstancePtr = NULL;
	int DevID = 0;
	int ThereWasAProblem = FALSE;

	/* Check for theIfID */
	if (argc > 0) {
		DevID = atoi(argv[0]);
	}

	/* Determine InstancePtr */
	InstancePtr = GetHdcpIf(DevID);

	/* Sanity Check */
	if (InstancePtr != NULL) {
		XHdcp1x_Info(InstancePtr);
	}
	else {
		ThereWasAProblem = TRUE;
	}

	/* Check for help */
	if (ThereWasAProblem) {
		PRINTF("hdcp display <id>\r\n");
	}

	return;
}

/*****************************************************************************/
/*!
 * \brief  This function implements the "hdcp disable" command
 * \param[in]  argc    the number of arguments
 * \param[in]  argv[]  an array of pointers to the arguments
 * \return  void
 *
 *****************************************************************************/
static void DebugDisable(int argc, const char *argv[])
{
	XHdcp1x *InstancePtr = NULL;
	int DevID = 0;
	int ThereWasAProblem = FALSE;

	/* Check for theIfID */
	if (argc > 0) {
		DevID = atoi(argv[0]);
	}

	/* Determine InstancePtr */
	InstancePtr = GetHdcpIf(DevID);

	/* Sanity Check */
	if (InstancePtr != NULL) {
		if (XHdcp1x_Disable(InstancePtr) != XST_SUCCESS) {
			ThereWasAProblem = TRUE;
		}
	}
	else {
		ThereWasAProblem = TRUE;
	}

	/* Check for help */
	if (ThereWasAProblem) {
		PRINTF("hdcp disable <id>\r\n");
	}

	return;
}

/*****************************************************************************/
/*!
 * \brief  This function implements the "hdcp enable" command
 * \param[in]  argc    the number of arguments
 * \param[in]  argv[]  an array of pointers to the arguments
 * \return  void
 *
 *****************************************************************************/
static void DebugEnable(int argc, const char *argv[])
{
	XHdcp1x *InstancePtr = NULL;
	int DevID = 0;
	int ThereWasAProblem = FALSE;

	/* Check for theIfID */
	if (argc > 0) {
		DevID = atoi(argv[0]);
	}

	/* Determine InstancePtr */
	InstancePtr = GetHdcpIf(DevID);

	/* Sanity Check */
	if (InstancePtr != NULL) {
		if (XHdcp1x_Enable(InstancePtr) != XST_SUCCESS) {
			ThereWasAProblem = TRUE;
		}
	}
	else {
		ThereWasAProblem = TRUE;
	}

	/* Check for help */
	if (ThereWasAProblem) {
		PRINTF("hdcp enable <id>\r\n");
	}

	return;
}

/*****************************************************************************/
/*!
 * \brief  This function implements the "hdcp encrypt" command
 * \param[in]  argc    the number of arguments
 * \param[in]  argv[]  an array of pointers to the arguments
 * \return  void
 *
 *****************************************************************************/
static void DebugEncrypt(int argc, const char *argv[])
{
	XHdcp1x *InstancePtr = NULL;
	int DevID = 0;
	int ThereWasAProblem = FALSE;

	/* Check for theIfID */
	if (argc > 0) {
		DevID = atoi(argv[0]);
	}

	/* Determine InstancePtr */
	InstancePtr = GetHdcpIf(DevID);

	/* Sanity Check */
	if (InstancePtr != NULL) {

		/* Check for set of encrypt map */
		if (argc > 1) {
			u64 Map = 0;

			/* Determine Map for what to enable */
			Map = strtoull(argv[1], NULL, 16);

			/* Enable encryption */
			if ((Map != 0) && XHdcp1x_EnableEncryption(InstancePtr,
					Map) != XST_SUCCESS) {
				ThereWasAProblem = TRUE;
			}

			/* Determine Map for what to enable */
			Map = 0;
			if (argc > 2) {
				Map = strtoull(argv[2], NULL, 16);
			}

			/* Disable encryption */
			if ((Map != 0) && XHdcp1x_DisableEncryption(InstancePtr,
					Map) != XST_SUCCESS) {
					ThereWasAProblem = TRUE;
			}
		}
		/* Otherwise - just display */
		else {
			u64 EncryptMap = 0;

			/* Determine EncryptMap */
			EncryptMap = XHdcp1x_GetEncryption(InstancePtr);

			/* Display EncryptMap */
			if (EncryptMap != 0) {
				PRINTF("Encrypt Map:  ");
				PRINTF("%08X", (u32) (EncryptMap >> 32));
				PRINTF("%08X\r\n", (u32) (EncryptMap));
			}
			else {
				PRINTF("Encrypt Map:  None\r\n");
			}
		}
	}
	else {
		ThereWasAProblem = TRUE;
	}

	/* Check for help */
	if (ThereWasAProblem) {
		PRINTF("hdcp encrypt <id> <enable-map> <disable-map>\r\n");
	}

	return;
}

/*****************************************************************************/
/*!
 * \brief  This function implements the "hdcp lane" command
 * \param[in]  argc    the number of arguments
 * \param[in]  argv[]  an array of pointers to the arguments
 * \return  void
 *
 *****************************************************************************/
static void DebugLane(int argc, const char *argv[])
{
	int ThereWasAProblem = FALSE;

	/* Sanity Check */
	if (argc > 1) {
		XHdcp1x *InstancePtr = NULL;
		int DevID = atoi(argv[0]);
		int NumLanes = atoi(argv[1]);

		/* Determine InstancePtr */
		InstancePtr = GetHdcpIf(DevID);

		/* Sanity Check */
		if (InstancePtr != NULL) {
			if (XHdcp1x_SetLaneCount(InstancePtr, NumLanes) != XST_SUCCESS) {
				ThereWasAProblem = TRUE;
			}
		}
	}
	else {
		ThereWasAProblem = TRUE;
	}

	/* Check for help */
	if (ThereWasAProblem) {
		PRINTF("hdcp lane <id> <num-lanes>\r\n");
	}

	return;
}

/*****************************************************************************/
/*!
 * \brief  This function implements the "hdcp list" command
 * \param[in]  argc    the number of arguments
 * \param[in]  argv[]  an array of pointers to the arguments
 * \return  void
 *
 *****************************************************************************/
static void DebugList(int argc, const char *argv[])
{
//	char Name[12];
//	char TypeString[12];
//	int DeviceID = 0;
//	int TxID = 0;
//	int RxID = 0;
//
//	/* Display header */
//	PRINTF("ID  Name        Type\r\n");
//	PRINTF("--  ----------  ----------\r\n");
//
//	/* List the devices */
//	for (DeviceID=0; DeviceID<XPAR_XHDCP_NUM_INSTANCES; DeviceID++) {
//
//		XHdcp1x* HdcpIf = GetHdcpIf(DeviceID);
//
//		/* Determine Name and TypeString */
//		if (HdcpIf->Config.IsRx) {
//			snprintf(Name, 12, "hdcp-rx/%d", RxID++);
//			if (HdcpIf->Config.IsHDMI) {
//				snprintf(TypeString, 12, "hdmi-rx");
//			}
//			else {
//				snprintf(TypeString, 12, "dp-rx");
//			}
//		}
//		else {
//			snprintf(Name, 12, "hdcp-tx/%d", TxID++);
//			if (HdcpIf->Config.IsHDMI) {
//				snprintf(TypeString, 12, "hdmi-tx");
//			}
//			else {
//				snprintf(TypeString, 12, "dp-tx");
//			}
//		}
//
//		/* Display it */
//		PRINTF("%2d  %-10s  %-10s\r\n", DeviceID, Name, TypeString);
//	}
	return;
}

/*****************************************************************************/
/*!
 * \brief  This function implements the "hdcp log" command
 * \param[in]  argc    the number of arguments
 * \param[in]  argv[]  an array of pointers to the arguments
 * \return  void
 *
 *****************************************************************************/
static void DebugLog(int argc, const char *argv[])
{
	int ThereWasAProblem = FALSE;

	/* Sanity Check */
	if (argc > 0) {
		/* Check for disable */
		if (strcmp(argv[0], "disable") == 0) {
			XHdcp1x_SetDebugLogMsg(NULL);
		}
		/* Check for enable */
		else if (strcmp(argv[0], "enable") == 0) {
			XHdcp1x_SetDebugLogMsg(PRINTF);
		}
		else {
			ThereWasAProblem = TRUE;
		}
	}
	else {
		ThereWasAProblem = TRUE;
	}

	/* Check for help */
	if (ThereWasAProblem) {
		PRINTF("hdcp log <disable|enable>\r\n");
	}

	return;
}

/*****************************************************************************/
/*!
 * \brief  This function implements the "hdcp reset" command
 * \param[in]  argc    the number of arguments
 * \param[in]  argv[]  an array of pointers to the arguments
 * \return  void
 *
 *****************************************************************************/
static void DebugReset(int argc, const char *argv[])
{
	XHdcp1x *InstancePtr = NULL;
	int DevID = 0;
	int ThereWasAProblem = FALSE;

	/* Check for theIfID */
	if (argc > 0) {
		DevID = atoi(argv[0]);
	}

	/* Determine InstancePtr */
	InstancePtr = GetHdcpIf(DevID);

	/* Sanity Check */
	if (InstancePtr != NULL) {
		if (XHdcp1x_Reset(InstancePtr) != XST_SUCCESS) {
			ThereWasAProblem = TRUE;
		}
	}
	else {
		ThereWasAProblem = TRUE;
	}

	/* Check for help */
	if (ThereWasAProblem) {
		PRINTF("hdcp reset <id>\r\n");
	}

	return;
}

/*****************************************************************************/
/*!
 * \brief  This function implements the "hdcp version" command
 * \param[in]  argc    the number of arguments
 * \param[in]  argv[]  an array of pointers to the arguments
 * \return  void
 *
 *****************************************************************************/
static void DebugVersion(int argc, const char *argv[])
{
	int DevID = 0;
	u32 SwVersion = 0;
	u32 RxVersion = 0;
	u32 TxVersion = 0;

	/* Determine SwVersion */
	SwVersion = XHdcp1x_GetDriverVersion();

	/* Determine RxVersion */
	DevID = 0;
	while ((RxVersion == 0) && (DevID < XPAR_XHDCP_NUM_INSTANCES)) {
		const XHdcp1x *InstancePtr = GetHdcpIf(DevID++);
		if (InstancePtr->Config.IsRx) {
			RxVersion = XHdcp1x_GetVersion(InstancePtr);
		}
	}

	/* Determine TxVersion */
	DevID = 0;
	while ((TxVersion == 0) && (DevID < XPAR_XHDCP_NUM_INSTANCES)) {
		const XHdcp1x *InstancePtr = GetHdcpIf(DevID++);
		if (!InstancePtr->Config.IsRx) {
			TxVersion = XHdcp1x_GetVersion(InstancePtr);
		}
	}

	/* Display SwVersion if present */
	if (SwVersion != 0) {
		PRINTF("Driver Version:  %d", ((SwVersion >> 16) & 0xFFFFu));
		PRINTF(".%02d", ((SwVersion >> 8) & 0xFFu));
		PRINTF(".%02d (SW)\r\n", (SwVersion & 0xFFu));
	}

	/* Display RxVersion if present */
	if (RxVersion != 0) {
		PRINTF("Cipher Version:  %d", ((RxVersion >> 16) & 0xFFFFu));
		PRINTF(".%02d", ((RxVersion >> 8) & 0xFFu));
		PRINTF(".%02d (RX)\r\n", (RxVersion & 0xFFu));
	}

	/* Display TxVersion if present */
	if (TxVersion != 0) {
		PRINTF("Cipher Version:  %d", ((TxVersion >> 16) & 0xFFFFu));
		PRINTF(".%02d", ((TxVersion >> 8) & 0xFFu));
		PRINTF(".%02d (TX)\r\n", (TxVersion & 0xFFu));
	}

	return;
}

/*****************************************************************************/
/*!
 * \brief  This function implements the "hdcp" debug command
 * \param[in]  argc    the number of arguments
 * \param[in]  argv[]  an array of pointers to the arguments
 * \return  void
 *
 *****************************************************************************/
static void DebugHdcp(int Argc, const char *Argv[])
{
	/* Check for display */
	if ((strcmp("display", Argv[1]) == 0) || (strcmp("d", Argv[1]) == 0)) {
		DebugDisplay(Argc - 2, Argv + 2);
	}
	/* Check for authenticate */
	else if (strcmp("authenticate", Argv[1]) == 0) {
		DebugAuthenticate(Argc - 2, Argv + 2);
	}
	/* Check for cipher */
	else if (strcmp("cipher", Argv[1]) == 0) {
		DebugCipher(Argc - 1, Argv + 1);
	}
	/* Check for disable */
	else if (strcmp("disable", Argv[1]) == 0) {
		DebugDisable(Argc - 2, Argv + 2);
	}
	/* Check for enable */
	else if (strcmp("enable", Argv[1]) == 0) {
		DebugEnable(Argc - 2, Argv + 2);
	}
	/* Check for encrypt */
	else if (strcmp("encrypt", Argv[1]) == 0) {
		DebugEncrypt(Argc - 2, Argv + 2);
	}
	/* Check for lane */
	else if (strcmp("lane", Argv[1]) == 0) {
		DebugLane(Argc - 2, Argv + 2);
	}
	/* Check for list */
	else if (strcmp("list", Argv[1]) == 0) {
		DebugList(Argc - 2, Argv + 2);
	}
	/* Check for log */
	else if (strcmp("log", Argv[1]) == 0) {
		DebugLog(Argc - 2, Argv + 2);
	}
	/* Check for port */
	else if (strcmp("port", Argv[1]) == 0) {
		DebugPort(Argc - 1, Argv + 1);
	}
	/* Check for reset */
	else if (strcmp("reset", Argv[1]) == 0) {
		DebugReset(Argc - 2, Argv + 2);
	}
	/* Check for version */
	else if (strcmp("version", Argv[1]) == 0) {
		DebugVersion(Argc - 2, Argv + 2);
	}
	/* Otherwise */
	else {
//		PRINTF("hdcp <authenticate|cipher|display|disable|enable");
//		PRINTF("|encrypt|lane|list|log|port|reset|version>\r\n");
		PRINTF("hdcp <|cipher|display|disable|enable");
		PRINTF("|lane|list|log|port|reset|version>\r\n");
	}

	return;
}

/*****************************************************************************/
/**
 *
 * This function strips all leading and trailing whitespace from a string
 *
 * @return
 *   void
 *
 * @note
 *   None.
 *
 *****************************************************************************/
static void StripBuf(char** BufPtr)
{
	char* Ptr = *BufPtr;

	/* Strip off any trailing CR/LF/SPACE */
	Ptr += (strlen(Ptr) - 1);
	while ((*Ptr == 0x20) || (*Ptr == 0x0D) || (*Ptr == 0x0A)) {
		*Ptr-- = 0;
	}

	/* Strip off any leading CR/LF/SPACE */
	Ptr = *BufPtr;
	while ((*Ptr == 0x20) || (*Ptr == 0x0D) || (*Ptr == 0x0A)) {
		Ptr++;
	}

	/* Update parameter */
	*BufPtr = Ptr;

	return;
}

/*****************************************************************************/
/**
 *
 * This function parses and dispatches the debug command line
 *
 * @return
 *   void
 *
 * @note
 *   None.
 *
 *****************************************************************************/
static void DispatchCmd(void)
{
	/* Locals */
	char* Ptr = gCmdBuf;

	/* Strip the command line */
	StripBuf(&Ptr);

	/* Sanity Check */
	if (strlen(Ptr)) {

		/* More locals */
		const char* Argv[MAX_ARG_NUM];
		int StartOfArgFound = FALSE;
		int Idx = 0;
		int Argc = 0;

		/* Initialize argv array */
		for (Idx = 0; Idx < MAX_ARG_NUM; Idx++)
			Argv[Idx] = NULL;

		/* Iterate until end of command line or max args has been reached */
		Idx = strlen(Ptr);
		do {

			/* Check for CR/LF/SPACE and replace them with 0x00 */
			if ((*Ptr == 0x20) || (*Ptr == 0x0D) || (*Ptr == 0x0A)) {

				/* Clear character from cmd line and clear start of arg flag */
				StartOfArgFound = FALSE;
				*Ptr = 0x00;
			}
			/* Check to see if we are looking for the start of an arg */
			else if (!StartOfArgFound) {

				/* Set start of arg flag and store ptr in argv */
				StartOfArgFound = TRUE;
				Argv[Argc++] = Ptr;
			}

			/* Update for loop */
			Ptr++;
			Idx--;
		} while ((Idx > 0) && (Argc < MAX_ARG_NUM));

		/* If there are arguments to examine */
		if (Argc > 1) {

			/* CR/LF */
			PRINTF("\r\n");

			/* Check for hdcp */
			if (strcmp("hdcp", Argv[0]) == 0) {
				DebugHdcp(Argc, Argv);
			}
			/* Otherwise */
			else {
				PRINTF("???\r\n");
			}

			/* CR/LF */
			PRINTF("\r\n");
		}
		else {
			PRINTF("???\r\n");
		}
	}

	return;
}

/*****************************************************************************/
/**
*
* This function services a hdcp debug command from the xilinx debug console
*
* @param CmdKey  the command key that was pressed
*
* @return
*   Truth value indicating if the key was consumed (TRUE) or not (FALSE)
*
* @note
*   None.
*
******************************************************************************/
int XHdcp1xDebug_ProcessKey(char CmdKey)
{
	/* Locals */
	int CmdIsComplete = FALSE;

	/* Check for the start of a new command */
	if (!gCmdInProgress) {

		/* Check for character of interest */
		if ((CmdKey == 'h') || (CmdKey == 'k')) {

			/* Initialize gCmdBuf */
			memset(gCmdBuf, 0, MAX_CMD_LENGTH);
			gCmdBufIdx = 0;

			/* Update gCmdInProgress */
			gCmdInProgress = TRUE;
		}
		else {
			return (FALSE);
		}
	}

	/* Check for end-of-command */
	if ((CmdKey == '\r') || (CmdKey == '\n')) {

		/* Update CmdIsComplete */
		CmdIsComplete = TRUE;

	}
	/* Check for backspace */
	else if (CmdKey == '\b') {

		/* Check for something already in the command buffer */
		if (gCmdBufIdx > 0) {

			/* Delete the last character in the buffer */
			gCmdBuf[--gCmdBufIdx] = 0;

			#if defined(DO_ECHO)
			/* Echo it */
			PRINTF("\b");
			#endif

			/* Clear it off the screen */
			PRINTF(" \b");
		}
	}
	/* Check for room */
	else if (gCmdBufIdx < (MAX_CMD_LENGTH - 1)) {

		/* Check for ascii display-able character */
		if ((CmdKey >= 0x20) && (CmdKey <= 0x7E)) {

			/* Update gCmdBuf */
			gCmdBuf[gCmdBufIdx++] = CmdKey;

			#if defined(DO_ECHO)
			/* Echo it */
			PRINTF("%c", CmdKey);
			#endif
		}
	}
	/* Otherwise - assume done and hope for the best */
	else {

		/* Update CmdIsComplete */
		CmdIsComplete = TRUE;
	}

	/* Check theCmdIsComplete */
	if (CmdIsComplete) {

		/* Send CR/LF */
		PRINTF("\r\n");

		/* Dispatch it */
		DispatchCmd();

		/* Update gCmdInProgress */
		gCmdInProgress = FALSE;

		/* Send CR/LF */
		PRINTF("\r\n");
	}

//	return (TRUE);
	return (gCmdInProgress);
}

#endif
