/******************************************************************************
* Copyright (C) 2014 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
* @file keymgmt.c
*
* This file contains the implementation for the key management module.
*
******************************************************************************/


/* Include Files */
#include "keymgmt.h"
#include "keymgmt_debug.h"
#include "keymgmt_device.h"
#include "keymgmt_loader.h"
//#include <stdlib.h>
//#include <string.h>

#include "eeprom.h"

/* Local Globals */
static int  gIsInitialized = FALSE;
XStatus Init_IIC();

/*****************************************************************************/
/**
*
* This function initializes the key management module
*
* @return
*   HDCP_ERROR_NONE if successful.
*
* @note
*   None.
*
******************************************************************************/
KEYMGMT_tError
KEYMGMT_Init(
  void
)
{
#if DEBUG_KEYMGMT_INIT
	xil_printf(">>>> Initializing the Key Management Module ... \r\n");
#endif
  /* Locals */
  KEYMGMT_tError theError = KEYMGMT_ERROR_NONE;

  /* Sanity Check */
  if (!gIsInitialized)
  {
	if(gIsKeyWrittenInEeeprom)
	{
		if (Init_IIC() != XST_SUCCESS){
			theError = -3;
	#if DEBUG_KEYMGMT_INIT
		xil_printf("IIC Init done: %s:%s:%d: Error = %d \r\n",
				__FILE__,__func__,__LINE__,theError);
	#endif
		}
	}
    /* Initialize the key management device module */
    if (KEYMGMTDEV_Init() != KEYMGMT_ERROR_NONE){
        theError = -1;
#if DEBUG_KEYMGMT_INIT
      xil_printf("Key Mgmt DEVICE Init done: \r\n\t%s:%s:%d: Error = %d \r\n",
		  __FILE__,__func__,__LINE__,theError);
#endif
    }
    /* Initialize the key management loader module */
    else if (KEYMGMTLDR_Init() != KEYMGMT_ERROR_NONE){
        theError = -2;
#if DEBUG_KEYMGMT_INIT
	xil_printf("Key Mgmt Loader module Init done: \r\n\t");
	xil_printf("%s:%s:%d: Error = %d \r\n",__FILE__,__func__,__LINE__,theError);
#endif
    }

#if DEBUG_KEYMGMT_INIT
    xil_printf("KEYMGMTDEV_Init done , error = %d \r\n",theError);
    xil_printf("KEYMGMTLDR_Init done , error = %d \r\n",theError);
#endif

    /* Update gIsInitialized */
    gIsInitialized = TRUE;
  }

  /* Return */
  return (theError);
}


/*****************************************************************************/
/**
*
* This function polls the key management module
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
KEYMGMT_Poll(
  uint32_t theUptime
)
{
  /* Poll the key management loader module */
  KEYMGMTLDR_Poll(theUptime);

  /* Return */
  return;
}


/*****************************************************************************/
/**
*
* This function implements the "keymgmt" debug command
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
KEYMGMT_Debug(
  int         argc,
  const char* argv[]
)
{
  /* Locals */
  int thereWasAProblem = FALSE;

  /* CR/LF */
  KEYMGMT_CONSOLE_PRINTF("\r\n");

  /* Sanity Check */
  if (argc > 1)
  {
    /* Check for device */
    if (strcmp(argv[1], "device") == 0)
      KEYMGMTDEV_Debug(argc-2, argv+2);
    /* Otherwise */
    else
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
    KEYMGMT_CONSOLE_PRINTF("keymgmt <device>\r\n");
  }

  /* CR/LF */
  KEYMGMT_CONSOLE_PRINTF("\r\n");

  /* Return */
  return (0);
}
