/******************************************************************************
*
* Copyright (C) 2014 - 2015 Xilinx, Inc.  All rights reserved.
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
* XILINX BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
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
* @file keymgmt_loader.c
*
* This file contains the implementation for the key management loader module.
*
******************************************************************************/


/* Include Files */
//#include "hdcp.h"
#include "keymgmt.h"
#include "keymgmt_debug.h"
#include "keymgmt_device.h"
#include "keymgmt_keyfile.h"
#include "keymgmt_loader.h"
#include <stdint.h>
#include <string.h>


/* Module Configuration */
#define PLATFORM_HANDLER_MAX           (2)


/* Constant Definitions */


/* Local Type Definitions */
typedef struct
{
  uint32_t        fFlags;
  KEYMGMT_tDevID  fDevID;

} tHandler;


/* Local Globals */
static int       gIsInitialized = FALSE;
static tHandler  gMyHandler[PLATFORM_HANDLER_MAX] =
{
 {
  0,
  KEYMGMT_DEVID_0,
 },
{
  0,
  KEYMGMT_DEVID_1,
 },
};


/* External Declarations */
extern const uint64_t  KEYMGMT_TESTKEYS_A1[41];
extern const uint64_t  KEYMGMT_TESTKEYS_A2[41];
extern const uint64_t  KEYMGMT_TESTKEYS_B1[41];
extern const uint64_t  KEYMGMT_TESTKEYS_B2[41];


/*****************************************************************************/
/**
*
* This function loads a specific table of a handler
*
* @param theHandler  the handler to load the table of
* @param theTableID  the table to load
* @param theBuf      the buffer of data to load
* @param theBufSize  the size of the buffer (in bytes)
*
* @return
*   KEYMGMT_ERROR_NONE if successful.
*
* @note
*   None.
*
******************************************************************************/
static KEYMGMT_tError
doLoadTable(
  tHandler*        theHandler,
  KEYMGMT_tTableID theTableID,
  const uint64_t*  theBuf,
  int              theBufSize
)
{
  /* Locals */
  KEYMGMT_tError theError = KEYMGMT_ERROR_NONE;

  /* Load it */
  if (KEYMGMTDEV_Load(theHandler->fDevID, theTableID, theBuf, theBufSize)
		  == KEYMGMT_ERROR_NONE)
  {
    /* Verify it */
    if (KEYMGMTDEV_Verify(theHandler->fDevID, theTableID, theBuf, theBufSize)
		!= KEYMGMT_ERROR_NONE)
    {
      /* Update theError */
      theError = -2;
    }
  }
  /* Otherwise */
  else
  {
    /* Update theError */
    theError = -1;
  }

  /* Return */
  return (theError);
}


/*****************************************************************************/
/**
*
* This function initializes a handler
*
* @param theHandler  the handler to initialize
*
* @return
*   KEYMGMT_ERROR_NONE if successful.
*
* @note
*   The name of the file specified in the call to KEYFILE_Validate is
*   currently not used.  Any file name can be specified.
*
******************************************************************************/
static KEYMGMT_tError
doInit(
  tHandler* theHandler
)
{
  /* Locals */
  KEYFILE_tHandle theHandle = KEYFILE_HANDLE_NULL;
  KEYMGMT_tError  theError = KEYMGMT_ERROR_NONE;

  /* Disable the key management device */
  KEYMGMTDEV_Disable(theHandler->fDevID);

  /* Validate the encapsulated key file data */
  //file name here is redundant, in the code the check is merely
  // 	if this "filename" argument passed is NULL or not
  theHandle = KEYFILE_Validate("keyfile.bin", NULL);

  /* Sanity Check */
  if (theHandle != KEYFILE_HANDLE_NULL)
  {
    /* More locals */
    KEYFILE_tDevKeyTable theDevKeyTable;
    KEYMGMT_tTableID     theTableID = KEYMGMT_TABLEID_0;

    /* Log */


    /* Iterate through theTables */
    while ((KEYFILE_Read(theHandle, &theDevKeyTable) == 0)
		&& (theError == KEYMGMT_ERROR_NONE))
    {
#if DEBUG_HDCP_INIT
    KEYMGMT_DEBUG_LOG("\t\t\t[Inside fn doInit for Keymgmt handler] : In");
    KEYMGMT_DEBUG_LOG(" while loop (keyfile read is successful) and theError");
    KEYMGMT_DEBUG_LOG(" is 0\r\n");
    KEYMGMT_DEBUG_LOG("\t\t\t[Inside fn doInit for Keymgmt handler] : ");
    KEYMGMT_DEBUG_LOG("Loading theDevKeyTable value \r\n");
#endif
	/* Load it */
      if (doLoadTable(theHandler, theTableID, theDevKeyTable.fData.fU64,
		  sizeof(theDevKeyTable.fData.fU64)) != KEYMGMT_ERROR_NONE)
        theError = -2;

#if DEBUG_HDCP_INIT
    KEYMGMT_DEBUG_LOG("\t\t\t[Inside fn doInit for Keymgmt handler] :");
    KEYMGMT_DEBUG_LOG("After loading theError = %d \r\n",theError);
#endif
      /* Update for loop */
      theTableID++;
    }

    /* Close the key file */
    KEYFILE_Close(&theHandle);


    /* Clear theDevKeyTable to ensure decrypted data not still on the stack */
    memset(&theDevKeyTable, 0, sizeof(theDevKeyTable));
  }
  /* Otherwise */
  else
  {
    /* Update theError */
    theError = -1;
  }

  /* Check for error */
  if (theError != KEYMGMT_ERROR_NONE)
  {
    /* Log */
	KEYMGMT_CONSOLE_PRINTF("\r\nError :%s:%s:%d: \r\n\t\tLoading from "
			"Encrypted data provided failed, theError = %d\r\n",
			__FILE__,__func__,__LINE__,theError);

    /* Load the test keys */
    doLoadTable(theHandler, KEYMGMT_TABLEID_0, KEYMGMT_TESTKEYS_A2,
		sizeof(KEYMGMT_TESTKEYS_A2));
    doLoadTable(theHandler, KEYMGMT_TABLEID_1, KEYMGMT_TESTKEYS_B2,
		sizeof(KEYMGMT_TESTKEYS_B2));
    doLoadTable(theHandler, KEYMGMT_TABLEID_2, KEYMGMT_TESTKEYS_A1,
		sizeof(KEYMGMT_TESTKEYS_A1));
    doLoadTable(theHandler, KEYMGMT_TABLEID_3, KEYMGMT_TESTKEYS_B1,
		sizeof(KEYMGMT_TESTKEYS_B1));

    /* Log */
    KEYMGMT_CONSOLE_PRINTF("Error :%s:%s:%d:",__FILE__,__func__,__LINE__);
    KEYMGMT_CONSOLE_PRINTF(" Loaded bogus key tables instead\r\n");
  }

  /* Enable the key management device */
  KEYMGMTDEV_Enable(theHandler->fDevID);

  /* Return */
  return (theError);
}


/*****************************************************************************/
/**
*
* This function initializes the module
*
* @return
*   KEYMGMT_ERROR_NONE if successful.
*
* @note
*   None.
*
******************************************************************************/
KEYMGMT_tError
KEYMGMTLDR_Init(
  void
)
{
#if DEBUG_HDCP_INIT
	xil_printf("\t\t\t[Inside fn KEYMGMTLDR_Init] \r\n");
#endif
	/* Locals */
  KEYMGMT_tError theError = KEYMGMT_ERROR_NONE;

  /* Sanity Check */
  if (!gIsInitialized)
  {
    /* More locals */
    tHandler* theHandler = gMyHandler;
    int       theNumLeft = PLATFORM_HANDLER_MAX;
#if DEBUG_HDCP_INIT
	xil_printf("\t\t\t[Inside fn KEYMGMTLDR_Init]: KEYFILE_Init \r\n");
#endif
    /* Initialize the key file module */
    KEYFILE_Init();

    /* Iterate through theHandlers */
    do
    {
#if DEBUG_HDCP_INIT
	xil_printf("\t\t\t[Inside fn KEYMGMTLDR_Init]: do-while;; iterate through");
	xil_printf("keymgmt handlers - doInit(theHandler) \r\n");
#endif
	xil_printf("Loading the keys for Key management module %d \r\n",theNumLeft);
		/* Initialize it */
      if (doInit(theHandler) != KEYMGMT_ERROR_NONE)
        theError = -1;
#if DEBUG_HDCP_INIT
	xil_printf("\t\t\t[Inside fn KEYMGMTLDR_Init]: doInit(theHandler) done"):
			xil_printf("for time : %d, error = %d\r\n",theNumLeft,theError);
#endif
      /* Update for loop */
      theHandler++;
      theNumLeft--;
    }
    while (theNumLeft > 0);

    /* Update gIsInitialized */
    gIsInitialized = TRUE;
  }

  /* Return */
  return (theError);
}


/*****************************************************************************/
/**
*
* This function polls the module
*
* @param theUptime  the system up time in seconds
*
* @return
*   void
*
* @note
*   None.
*
******************************************************************************/
void
KEYMGMTLDR_Poll(
  uint32_t theUpTime
)
{

  /* Return */
  return;
}


/*****************************************************************************/
/**
*
* This function implements the "keymgmt loader" debug command
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
******************************************************************************/
int
KEYMGMTLDR_Debug(
  int         argc,
  const char* argv[]
)
{
  /* Locals */
  int thereWasAProblem = FALSE;

  /* Sanity Check */
  if (argc > 0)
  {
//    /* Check for xxx */
//    if (0)
//    /* Otherwise */
//    else
      thereWasAProblem = TRUE;
  }
  /* Otherwise */
  else
  {
    thereWasAProblem = TRUE;
  }

  /* Check for help */
  if (thereWasAProblem)
  {
    /* Display it */
    KEYMGMT_CONSOLE_PRINTF("keymgmt loader <...>\r\n");
  }

  /* Return */
  return (0);
}
