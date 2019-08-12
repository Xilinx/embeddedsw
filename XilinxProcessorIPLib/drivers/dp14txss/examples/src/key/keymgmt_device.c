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
* @file keymgmt_device.c
*
* This file contains the implementation for the key management device module.
*
******************************************************************************/



/* Include Files */
#include "keymgmt.h"
#include "keymgmt_debug.h"
#include "keymgmt_device.h"
#include <stdlib.h>
#include <string.h>
#include "xil_io.h"
#include "xparameters.h"

#include "xiic.h"
#include "xintc.h"


/* Module Configuration */
//#define USE_STUB_MEMORY                (1)


/* Constant Definitions */
#define PLATFORM_DEVID_MAX             (2)

#define FLAG_ENABLED                   (1u << 0)
#define FLAG_LOCKED                    (1u << 1)

#define REG_VERSION                    (0x0000u)
#define REG_TYPE                       (0x0004u)
#define REG_SCRATCH                    (0x0008u)
#define REG_CONTROL                    (0x000Cu)
#define REG_STATUS                     (0x0010u)

#define REG_TABLE_CONTROL              (0x0020u)
#define REG_TABLE_STATUS               (0x0024u)
#define REG_TABLE_ADDRESS              (0x0028u)
#define REG_TABLE_DATA_H               (0x002Cu)
#define REG_TABLE_DATA_L               (0x0030u)

#define REG_MAX                        (0x0040u)

extern unsigned int gKeyMGMTBaseAddress[2];
/* Type Definitions */
typedef struct
{
  KEYMGMT_tDevID  fDevID;
  uint8_t         fFlags;
  uint32_t        fBaseAddress;

} tDevice;


/* Local Globals */
static int      gIsInitialized = FALSE;

#if (PLATFORM_DEVID_MAX>2)
#error "Max two keygen devices are supported"
#endif
static tDevice  gMyDevices[2] =
{
  /* Device #0 */
  {
    KEYMGMT_DEVID_0,
    0,
    0
  },
{
    KEYMGMT_DEVID_1,
    0,
    0
  },
};
#if (USE_STUB_MEMORY)
static uint32_t gMyStubMemory[PLATFORM_DEVID_MAX][REG_MAX>>2];
static uint64_t gMyStubTableMemory[PLATFORM_DEVID_MAX][8][64];
#endif


/*****************************************************************************/
/**
*
* This function locks a device
*
* @param theDevice  the device to lock
*
* @return
*   void
*
* @note
*   None.
*
******************************************************************************/
static void
doLock(
  tDevice* theDevice
)
{
  /* Update theDevice */
  theDevice->fFlags |= FLAG_LOCKED;

  /* Return */
  return;
}


/*****************************************************************************/
/**
*
* This function initializes the hdcp port module
*
* @return
*   - HDCP_ERROR_NONE if successful.
*
* @note
*   None.
*
******************************************************************************/
static void
doUnlock(
  tDevice* theDevice
)
{
  /* Update theDevice */
  theDevice->fFlags &= ~FLAG_LOCKED;

  /* Return */
  return;
}


/*****************************************************************************/
/**
*
* This function reads a register from a specific device
*
* @param theDevice  the device to read from
* @param theReg  the register to read
*
* @return
*   The contents of the register
*
* @note
*   This function is implemented as a macro for performance reasons
*
******************************************************************************/
#if (USE_STUB_MEMORY)
#define doRegRead(theDevice, theReg)  \
  (gMyStubMemory[theDevice->fDevID][theReg>>2])
#else
#define doRegRead(theDevice, theReg)  \
  (Xil_In32(theDevice->fBaseAddress+(theReg)))
#endif


/*****************************************************************************/
/**
*
* This function writes a register in a specific device
*
* @param theDevice  the device to write to
* @param theReg  the register to write
* @param theValue  the value to write
*
* @return
*   void
*
* @note
*   This function is implemented as a macro for performance reasons
*
******************************************************************************/
#if (USE_STUB_MEMORY)
#define doRegWrite(theDevice, theReg, theValue)  \
  gMyStubMemory[theDevice->fDevID][theReg>>2] = theValue
#else
#define doRegWrite(theDevice, theReg, theValue)  \
  (Xil_Out32((theDevice->fBaseAddress+(theReg)), (theValue)))
#endif


/*****************************************************************************/
/**
*
* This function reads a specific table entry within a device
*
* @param theDevice  the device to read from
* @param theTableID  the table to read
* @param theRowID  the row within the table to read
* @param theU64ValuePtr  the value read
*
* @return
*   - KEYMGMT_ERROR_NONE if successful.
*
* @note
*   None.
*
******************************************************************************/
static KEYMGMT_tError
doTableRead(
  tDevice*         theDevice,
  KEYMGMT_tTableID theTableID,
  KEYMGMT_tRowID   theRowID,
  uint64_t*        theU64ValuePtr
)
{
  /* Locals */
  KEYMGMT_tError  theError = KEYMGMT_ERROR_NONE;

#if (USE_STUB_MEMORY)
  /* Update gMyStubTableMemory */
  *theU64ValuePtr = gMyStubTableMemory[theDevice->fDevID][theTableID][theRowID];
#else
  /* More Locals */
  uint32_t  theAddress = 0;
  uint32_t  theValue = 0;
  int       theGuard = 0x400;

  /* Determine theAddress */
  theAddress   = theTableID;
  theAddress <<= 8;
  theAddress  |= theRowID;

  /* Set read enable and clear write enable */
  theValue  = doRegRead(theDevice, REG_TABLE_CONTROL);
  theValue &= 0xFFFFFFFCul;
  theValue |= 0x00000002ul;
  doRegWrite(theDevice, REG_TABLE_CONTROL, theValue);

  /* Write theAddress */
  doRegWrite(theDevice, REG_TABLE_ADDRESS, theAddress);

  /* Wait until done */
  while (((doRegRead(theDevice, REG_TABLE_STATUS) & 0x01ul) != 0)
			  && (--theGuard > 0));

  /* Check no timeout */
  if (theGuard != 0)
  {
    /* More loop variables */
    uint64_t theU64Value = 0;

    /* Determine theU64Value */
    theU64Value   = doRegRead(theDevice, REG_TABLE_DATA_H);
    theU64Value <<= 32;
    theU64Value  |= doRegRead(theDevice, REG_TABLE_DATA_L);

    /* Update theU64ValuePtr */
    *theU64ValuePtr = theU64Value;
  }
  /* Otherwise */
  else
  {
    /* Update theError */
    theError = -1;
  }

  /* Clear read enable */
  theValue  = doRegRead(theDevice, REG_TABLE_CONTROL);
  theValue &= 0xFFFFFFFCul;
  doRegWrite(theDevice, REG_TABLE_CONTROL, theValue);

#endif

  /* Return */
  return (theError);
}


/*****************************************************************************/
/**
*
* This function writes a specific table entry within a device
*
* @param theDevice  the device to write to
* @param theTableID  the table to write
* @param theRowID  the row within the table to write
* @param theU64Value  the value to write
*
* @return
*   - KEYMGMT_ERROR_NONE if successful.
*
* @note
*   None.
*
******************************************************************************/
static KEYMGMT_tError
doTableWrite(
  tDevice*         theDevice,
  KEYMGMT_tTableID theTableID,
  KEYMGMT_tRowID   theRowID,
  uint64_t         theU64Value
)
{
  /* Locals */
  KEYMGMT_tError  theError = KEYMGMT_ERROR_NONE;

#if (USE_STUB_MEMORY)
  /* Update gMyStubTableMemory */
  gMyStubTableMemory[theDevice->fDevID][theTableID][theRowID] = theU64Value;
#else
  /* More Locals */
  uint32_t  theAddress = 0;
  uint32_t  theValue = 0;
  int       theGuard = 0x400;

  /* Load the current table entry */
  theValue = (uint32_t) (theU64Value);
  doRegWrite(theDevice, REG_TABLE_DATA_L, theValue);
  theValue = (uint32_t) (theU64Value >> 32);
  doRegWrite(theDevice, REG_TABLE_DATA_H, theValue);

  /* Set write enable and clear read enable */
  theValue  = doRegRead(theDevice, REG_TABLE_CONTROL);
  theValue &= 0xFFFFFFFCul;
  theValue |= 0x00000001ul;
  doRegWrite(theDevice, REG_TABLE_CONTROL, theValue);

  /* Determine theAddress */
  theAddress   = theTableID;
  theAddress <<= 8;
  theAddress  |= theRowID;

  /* Write theAddress */
  doRegWrite(theDevice, REG_TABLE_ADDRESS, theAddress);

  /* Wait until done */
  while (((doRegRead(theDevice, REG_TABLE_STATUS) & 0x01ul) != 0)
			  && (--theGuard > 0));

  /* Check for timeout */
  if (theGuard == 0)
    theError = -1;
#endif

  /* Return */
  return (theError);
}

/*****************************************************************************/
/**
*

* This function enables a device
*
* @param theDevice  the device to enable
*
* @return
*   - KEYMGMT_ERROR_NONE if successful.
*
* @note
*   None.
*
******************************************************************************/
static KEYMGMT_tError
doEnable(
  tDevice* theDevice
)
{
  /* Locals */
  uint32_t       theValue = 0;
  KEYMGMT_tError theError = KEYMGMT_ERROR_NONE;

  /* Enable it */
  theValue  = doRegRead(theDevice, REG_CONTROL);
  theValue |= 0x03ul;
  doRegWrite(theDevice, REG_CONTROL, theValue);

  /* Enable read lockout */
  theValue  = doRegRead(theDevice, REG_TABLE_CONTROL);
  theValue |= 0x80000000ul;
  doRegWrite(theDevice, REG_TABLE_CONTROL, theValue);

  /* Return */
  return (theError);
}


/*****************************************************************************/
/**
*
* This function disables a device
*
* @param theDevice  the device to disable
*
* @return
*   - KEYMGMT_ERROR_NONE if successful.
*
* @note
*   None.
*
******************************************************************************/
static KEYMGMT_tError
doDisable(
  tDevice* theDevice
)
{
  /* Locals */
  uint32_t       theValue = 0;
  KEYMGMT_tError theError = KEYMGMT_ERROR_NONE;

  /* Disable it */
  theValue  = doRegRead(theDevice, REG_CONTROL);
  theValue &= 0xFFFFFFFEul;
  doRegWrite(theDevice, REG_CONTROL, theValue);

  /* Return */
  return (theError);
}


/*****************************************************************************/
/**
*
* This function probes a device
*
* @param theDevice  the device to probe
* @param theNumTables  returns the number of tables
* @param theNumRowsPerTable  returns the number of rows per table
*
* @return
*   void
*
* @note
*   None.
*
******************************************************************************/
static void
doProbe(
  tDevice*          theDevice,
  KEYMGMT_tTableID* theNumTables,
  KEYMGMT_tRowID*   theNumRowsPerTable
)
{
  /* Locals */
  uint32_t theType = 0;

  /* Determine theType */
  theType = doRegRead(theDevice, REG_TYPE);

  /* Sanity Check */
  if (theType != 0)
  {
    /* Update theNumTables */
    if (theNumTables != NULL)
      *theNumTables = (KEYMGMT_tTableID) ((theType >> 8) & 0xFFu);

    /* Update theNumRowsPerTable */
    if (theNumRowsPerTable != NULL)
      *theNumRowsPerTable = (KEYMGMT_tRowID) (theType & 0xFFu);
  }
  /* Otherwise */
  else
  {
    /* Update theNumTables */
    if (theNumTables != NULL)
      *theNumTables = 8;

    /* Update theNumRowsPerTable */
    if (theNumRowsPerTable != NULL)
      *theNumRowsPerTable = 41;
  }

  /* Return */
  return;
}


/*****************************************************************************/
/**
*
* This function initializes the tables within a device
*
* @param theDevice  the device to whose tables are to be initialized
*
* @return
*   KEYMGMT_ERROR_NONE if successful.
*
* @note
*   None.
*
******************************************************************************/
static KEYMGMT_tError
doInitTables(
  tDevice* theDevice
)
{
  KEYMGMT_tTableID theNumTables = 0;
  KEYMGMT_tRowID   theNumRowsPerTable = 0;
  KEYMGMT_tTableID theTableID = KEYMGMT_TABLEID_0;
  KEYMGMT_tRowID   theRowID = KEYMGMT_ROWID_0;
  KEYMGMT_tError   theError = KEYMGMT_ERROR_NONE;

  /* Probe it */
  doProbe(theDevice, &theNumTables, &theNumRowsPerTable);

  /* Iterate through the tables */
  for (theTableID=KEYMGMT_TABLEID_0; theTableID<theNumTables; theTableID++)
  {
    /* Iterate through the rows */
    for (theRowID=KEYMGMT_ROWID_0; theRowID<theNumRowsPerTable; theRowID++)
    {
      /* Write the entry */
      if (doTableWrite(theDevice, theTableID, theRowID, 0)
			  != KEYMGMT_ERROR_NONE)
        theError = -1;
    }
  }

  /* Return */
  return (theError);
}


/*****************************************************************************/
/**
*
* This function initializes a device
*
* @param theDevice  the device to initialize
*
* @return
*   KEYMGMT_ERROR_NONE if successful.
*
* @note
*   None.
*
******************************************************************************/
static KEYMGMT_tError
doInit(
  tDevice* theDevice
)
{
  /* Locals */
  uint32_t       theValue = 0;
  KEYMGMT_tError theError = KEYMGMT_ERROR_NONE;

#if defined(USE_STUB_MEMORY)
  /* Initialize a few things */
  doRegWrite(theDevice, REG_VERSION, 0x00096363ul);
  doRegWrite(theDevice, REG_TYPE,    0x00000840ul);
#endif

  /* Confirm the core is actually present */
  if ((doRegRead(theDevice, REG_VERSION) != 0)
		  || (doRegRead(theDevice, REG_TYPE) != 0))
  {
    /* Assert reset */
    theValue  = doRegRead(theDevice, REG_CONTROL);
    theValue |= 0x80000000ul;
    doRegWrite(theDevice, REG_CONTROL, theValue);

    /* Release reset */
    theValue  = doRegRead(theDevice, REG_CONTROL);
    theValue &= 0x7FFFFFFFul;
    doRegWrite(theDevice, REG_CONTROL, theValue);

    /* Initialize the table contents */
    if (doInitTables(theDevice) != KEYMGMT_ERROR_NONE)
      theError = -2;
  }
  /* Otherwise */
  else
  {
    /* Log */
#if DEBUG_HDCP_INIT
    KEYMGMT_DEBUG_LOG("KEYMGMT_Device: core not present");
#endif
    /* Update theError */
    theError = -1;
  }

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
KEYMGMTDEV_Init(
  void
)
{
  /* Locals */
  KEYMGMT_tError theError = KEYMGMT_ERROR_NONE;
  uint32_t device_id=0;

  /* Sanity Check */
  if (!gIsInitialized)
  {
    /* Locals */
    tDevice* theDevice = gMyDevices;
    int      theNumLeft = PLATFORM_DEVID_MAX;

  #if (USE_STUB_MEMORY)
    /* Initialize gMyStubMemory */
    memset(gMyStubMemory, 0, sizeof(gMyStubMemory));

    /* Initialize gMyStubTableMemory */
    memset(gMyStubTableMemory, 0, sizeof(gMyStubTableMemory));
  #endif

    /* Iterate through the devices */
    do
    {
	xil_printf("Initializing Key Management device %d (%x) \r\n",
			device_id,gKeyMGMTBaseAddress[device_id]);
	theDevice->fBaseAddress = gKeyMGMTBaseAddress[device_id];
	if(theDevice->fBaseAddress==0)
	{
	  xil_printf("Please initialize the base address for key_gen device=%d"
				" in array gKeyMGMTBaseAddress\r\n",device_id);
	}
	device_id++;
      /* Initialize it */
      doLock(theDevice);
      if (doInit(theDevice) != KEYMGMT_ERROR_NONE)
        theError = -1;
      doUnlock(theDevice);

      /* Update for loop */
      theDevice++;
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
* This function probes a device
*
* @param theDevID  the id of the device to probe
*
* @return
*   KEYMGMT_ERROR_NONE if successful.
*
* @note
*   None.
*
******************************************************************************/
KEYMGMT_tError
KEYMGMTDEV_Probe(
  KEYMGMT_tDevID    theDevID,
  KEYMGMT_tTableID* theNumTables,
  KEYMGMT_tRowID*   theNumRowsPerTable
)
{
  /* Locals */
  KEYMGMT_tError theError = KEYMGMT_ERROR_NONE;

  /* Sanity Check */
  if (theDevID < PLATFORM_DEVID_MAX)
  {
    /* More locals */
    tDevice* theDevice = &(gMyDevices[theDevID]);

    /* Lock it */
    doLock(theDevice);

    /* Probe it */
    doProbe(theDevice, theNumTables, theNumRowsPerTable);

    /* Unlock it */
    doUnlock(theDevice);
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
* This function enables a device
*
* @param theDevID  the id of the device to enable
*
* @return
*   KEYMGMT_ERROR_NONE if successful.
*
* @note
*   None.
*
******************************************************************************/
KEYMGMT_tError
KEYMGMTDEV_Enable(
  KEYMGMT_tDevID theDevID
)
{
  /* Locals */
  KEYMGMT_tError theError = KEYMGMT_ERROR_NONE;

  /* Sanity Check */
  if (theDevID < PLATFORM_DEVID_MAX)
  {
    /* More locals */
    tDevice* theDevice = &(gMyDevices[theDevID]);

    /* Lock it */
    doLock(theDevice);

    /* Check for currently disabled */
    if ((theDevice->fFlags & FLAG_ENABLED) == 0)
    {
      /* Enable it */
      doEnable(theDevice);

      /* Update theDevice */
      theDevice->fFlags |= FLAG_ENABLED;
    }

    /* Unlock it */
    doUnlock(theDevice);
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
* This function disables a device
*
* @param theDevID  the id of the device to disable
*
* @return
*   KEYMGMT_ERROR_NONE if successful.
*
* @note
*   None.
*
******************************************************************************/
KEYMGMT_tError
KEYMGMTDEV_Disable(
  KEYMGMT_tDevID theDevID
)
{
  /* Locals */
  KEYMGMT_tError theError = KEYMGMT_ERROR_NONE;

  /* Sanity Check */
  if (theDevID < PLATFORM_DEVID_MAX)
  {
    /* More locals */
    tDevice* theDevice = &(gMyDevices[theDevID]);

    /* Lock it */
    doLock(theDevice);

    /* Disable it */

    doDisable(theDevice);

    /* Update theDevice */
    theDevice->fFlags &= ~FLAG_ENABLED;

    /* Unlock it */
    doUnlock(theDevice);
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
* This function loads a buffer of data into a table
*
* @param theDevID  the id of the device being loaded
* @param theTableD  the id of the table to be loaded
* @param theBuf  the buffer of data to be loaded
* @param theBufSize  the size of the buffer (in bytes)
*
* @return
*   KEYMGMT_ERROR_NONE if successful.
*
* @note
*   None.
*
******************************************************************************/
KEYMGMT_tError
KEYMGMTDEV_Load(
  KEYMGMT_tDevID   theDevID,
  KEYMGMT_tTableID theTableID,
  const uint64_t*  theBuf,
  int              theBufSize
)
{
  /* Locals */
  KEYMGMT_tError theError = KEYMGMT_ERROR_NONE;

  /* Sanity Check */
  if (theDevID < PLATFORM_DEVID_MAX)
  {
    /* More locals */
    tDevice* theDevice = &(gMyDevices[theDevID]);

    /* Lock it */
    doLock(theDevice);

    /* Confirm its disabled */
    if ((theDevice->fFlags & FLAG_ENABLED) == 0)
    {
      /* More lcoals */
      KEYMGMT_tTableID theNumTables = 0;
      KEYMGMT_tRowID   theNumRowsPerTable = 0;

      /* Probe it */
      doProbe(theDevice, &theNumTables, &theNumRowsPerTable);

      /* Sanity Check */
      if (theTableID < theNumTables)
      {
        /* More locals */
        int            theTableSize = 0;
        KEYMGMT_tRowID theRowID = KEYMGMT_ROWID_0;

        /* Determine theTableSize */
        theTableSize  = theNumRowsPerTable;
        theTableSize *= sizeof(uint64_t);

        /* Truncate and align if necessary */
        if (theBufSize > theTableSize)
          theBufSize = theTableSize;
        theBufSize &= ~(sizeof(uint64_t)-1);

        /* Iterate through theBuf */
        do
        {
          /* Write the next value */
          if (doTableWrite(theDevice, theTableID, theRowID, *theBuf)
			  != KEYMGMT_ERROR_NONE)
            theError = -4;

          /* Update for loop */
          theRowID++;
          theBuf++;
          theBufSize -= sizeof(uint64_t);
        }
        while ((theBufSize > 0) && (theError == KEYMGMT_ERROR_NONE));
      }
      /* Otherwise */
      else
      {
        /* Update theError */
        theError = -3;
      }
    }
    /* Otherwise */
    else
    {
      /* Update theError */
      theError = -2;
    }

    /* Unlock it */
    doUnlock(theDevice);
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
* This function verifies the contents of a table against a buffer
*
* @param theDevID  the id of the device being queried
* @param theTableD  the id of the table to be verified
* @param theBuf  the buffer of data to be verified against
* @param theBufSize  the size of the buffer (in bytes)
*
* @return
*   KEYMGMT_ERROR_NONE if successful.
*
* @note
*   None.
*
******************************************************************************/
KEYMGMT_tError
KEYMGMTDEV_Verify(
  KEYMGMT_tDevID   theDevID,
  KEYMGMT_tTableID theTableID,
  const uint64_t*  theBuf,
  int              theBufSize
)
{
  /* Locals */
  KEYMGMT_tError theError = KEYMGMT_ERROR_NONE;

  /* Sanity Check */
  if (theDevID < PLATFORM_DEVID_MAX)
  {
    /* More locals */
    tDevice* theDevice = &(gMyDevices[theDevID]);

    /* Lock it */
    doLock(theDevice);

    /* Confirm its disabled */
    if ((theDevice->fFlags & FLAG_ENABLED) == 0)
    {
      /* More lcoals */
      KEYMGMT_tTableID theNumTables = 0;
      KEYMGMT_tRowID   theNumRowsPerTable = 0;

      /* Probe it */
      doProbe(theDevice, &theNumTables, &theNumRowsPerTable);

      /* Sanity Check */
      if (theTableID < theNumTables)
      {
        /* More locals */
        int            theTableSize = 0;
        KEYMGMT_tRowID theRowID = KEYMGMT_ROWID_0;

        /* Determine theTableSize */
        theTableSize  = theNumRowsPerTable;
        theTableSize *= sizeof(uint64_t);

        /* Truncate and align if necessary */
        if (theBufSize > theTableSize)
          theBufSize = theTableSize;
        theBufSize &= ~(sizeof(uint64_t)-1);

        /* Iterate through theBuf */
        do
        {
          /* Loop variables */
          uint64_t theU64Value = 0;

          /* Read the next value */
          if (doTableRead(theDevice, theTableID, theRowID, &theU64Value)
			  == KEYMGMT_ERROR_NONE)
          {
            /* Check for mismatch */
            if (*theBuf != theU64Value)
              theError = -5;
          }
          /* Otherwise */
          else
          {
            /* Update theError */
            theError = -4;
          }

          /* Update for loop */
          theRowID++;
          theBuf++;
          theBufSize -= sizeof(uint64_t);
        }
        while ((theBufSize > 0) && (theError == KEYMGMT_ERROR_NONE));
      }
      /* Otherwise */
      else
      {
        /* Update theError */
        theError = -3;
      }
    }
    /* Otherwise */
    else
    {
      /* Update theError */
      theError = -2;
    }

    /* Unlock it */
    doUnlock(theDevice);
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
* This function reads the version of the device core
*
* @param theDevID  the id of the device to query
*
* @return
*   The version of the device core
*
* @note
*   None
*
******************************************************************************/
uint32_t
KEYMGMTDEV_GetVersion(
  KEYMGMT_tDevID theDevID
)
{
  /* Locals */
  uint32_t theVersion = 0;

  /* Sanity Check */
  if (theDevID < PLATFORM_DEVID_MAX)
  {
    /* More locals */
    tDevice* theDevice = &(gMyDevices[theDevID]);

    /* Lock it */
    doLock(theDevice);

    /* Determine theVersion */
    theVersion = doRegRead(theDevice, REG_VERSION);

    /* Unlock it */
    doUnlock(theDevice);
  }

  /* Return */
  return (theVersion);
}


/******************************************************************************/
/*!
 * \brief  This function implements the "keymgmt device disable" command
 * \param[in]  argc    the number of arguments
 * \param[in]  argv[]  an array of pointers to the arguments
 * \return  void
 *
 ******************************************************************************/
static void
doDebugDisable(
  int         argc,
  const char* argv[]
)
{
  /* Locals */
  KEYMGMT_tDevID theDevID = KEYMGMT_DEVID_0;
  int            thereWasAProblem = FALSE;

  /* Check for theDevID */
  if (argc > 0)
    theDevID = (KEYMGMT_tDevID) atoi(argv[0]);

  /* Disable it */
  if (KEYMGMTDEV_Disable(theDevID) != KEYMGMT_ERROR_NONE)
    thereWasAProblem = TRUE;

  /* Check for help */
  if (thereWasAProblem)
  {
    /* Display it */
    KEYMGMT_CONSOLE_PRINTF("keymgmt device disable <id>\r\n");
  }

  /* Return */
  return;
}


/******************************************************************************/
/*!
 * \brief  This function implements the "keymgmt device enable" command
 * \param[in]  argc    the number of arguments
 * \param[in]  argv[]  an array of pointers to the arguments
 * \return  void
 *
 ******************************************************************************/
static void
doDebugEnable(
  int         argc,
  const char* argv[]
)
{
  /* Locals */
  KEYMGMT_tDevID theDevID = KEYMGMT_DEVID_0;
  int            thereWasAProblem = FALSE;

  /* Check for theDevID */
  if (argc > 0)
    theDevID = (KEYMGMT_tDevID) atoi(argv[0]);

  /* Enable it */
  if (KEYMGMTDEV_Enable(theDevID) != KEYMGMT_ERROR_NONE)
    thereWasAProblem = TRUE;

  /* Check for help */
  if (thereWasAProblem)
  {
    /* Display it */
    KEYMGMT_CONSOLE_PRINTF("keymgmt device enable <id>\r\n");
  }

  /* Return */
  return;
}


/******************************************************************************/
/*!
 * \brief  This function implements the "keymgmt device read" command
 * \param[in]  argc    the number of arguments
 * \param[in]  argv[]  an array of pointers to the arguments
 * \return  void
 *
 ******************************************************************************/
static void
doDebugRead(
  int         argc,
  const char* argv[]
)
{
  /* Locals */
  KEYMGMT_tDevID theDevID = KEYMGMT_DEVID_0;
  int            thereWasAProblem = FALSE;

  /* Check for theDevID */
  if (argc > 0)
    theDevID = (KEYMGMT_tDevID) atoi(argv[0]);

  /* Sanity Check */
  if (theDevID < PLATFORM_DEVID_MAX)
  {
    /* More locals */
    tDevice*     theDevice = &(gMyDevices[theDevID]);
    uint32_t     theAddress = 0;
    unsigned int theNumToRead = 0x40;
    unsigned int theNumDone = 0;

    /* Check for theAddress */
    if (argc > 1)
      theAddress = strtoul(argv[1], NULL, 16);

    /* Check for theNumToRead */
    if (argc > 2)
      theNumToRead = (unsigned int) strtoul(argv[2], NULL, 16);

    /* Adjust/truncate as needed */
    theAddress &= 0xFFul;
    if ((theAddress + (theNumToRead << 2)) > REG_MAX)
      theNumToRead = ((REG_MAX - theAddress) >> 2);
    if (theNumToRead == 0)
      theNumToRead = 1;

    /* Lock */
    doLock(theDevice);

    /* Iterate through the reads */
    do
    {
      /* Check for start-of-line */
      if ((theNumDone % 4) == 0)
	  KEYMGMT_CONSOLE_PRINTF("%02X: ", theAddress);

      /* Read and display it */
      KEYMGMT_CONSOLE_PRINTF(" %08lX", doRegRead(theDevice, theAddress));

      /* Check for end-of-line */
      if ((theNumDone % 4) == 3)
	  KEYMGMT_CONSOLE_PRINTF("\r\n");

      /* Update for loop */
      theAddress += 4;
      theNumDone++;
      theNumToRead--;
    }
    while (theNumToRead > 0);

    /* Unlock */
    doUnlock(theDevice);

    /* Check for extra end-of-line */
    if ((theNumDone % 4) != 0)
      KEYMGMT_CONSOLE_PRINTF("\r\n");
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
    KEYMGMT_CONSOLE_PRINTF(
		"keymgmt device read <id> <address> <num-to-read>\r\n");
  }

  /* Return */
  return;
}


/******************************************************************************/
/*!
 * \brief  This function implements the "keymgmt device table" command
 * \param[in]  argc    the number of arguments
 * \param[in]  argv[]  an array of pointers to the arguments
 * \return  void
 *
 ******************************************************************************/
static void
doDebugTable(
  int         argc,
  const char* argv[]
)
{
  /* Locals */
  KEYMGMT_tDevID   theDevID = KEYMGMT_DEVID_0;
  KEYMGMT_tRowID   theNumRows = 0;
  KEYMGMT_tTableID theNumTables = 0;
  int              thereWasAProblem = FALSE;

  /* Check for theDevID */
  if (argc > 0)
    theDevID = (KEYMGMT_tDevID) atoi(argv[0]);

  /* Determine theNumRows */
  if (KEYMGMTDEV_Probe(theDevID, &theNumTables, &theNumRows)
		  == KEYMGMT_ERROR_NONE)
  {
    /* More locals */
    KEYMGMT_tTableID theTableID = KEYMGMT_TABLEID_0;

    /* Check for theTableID */
    if (argc > 1)
      theTableID = (KEYMGMT_tTableID) atoi(argv[1]);

    /* Sanity Check */
    if (theTableID < theNumTables)
    {
      /* More locals */
      tDevice*       theDevice = &(gMyDevices[theDevID]);
      KEYMGMT_tRowID theRowID = 0;
      unsigned int   theNumDone = 0;

      /* Lock */
      doLock(theDevice);

      /* Iterate through the reads */
      do
      {
        /* Loop variables */
        uint64_t theU64Value = 0;

        /* Check for start-of-line */
        if ((theNumDone % 4) == 0)
          KEYMGMT_CONSOLE_PRINTF("%02lX: ", theRowID);

        /* Read and display it */
        if (doTableRead(theDevice, theTableID, theRowID, &theU64Value)
			== KEYMGMT_ERROR_NONE)
        {
          /* More locals */
          uint32_t theValueH = (uint32_t) (theU64Value >> 32);
          uint32_t theValueL = (uint32_t) (theU64Value);

          /* Display it */
          KEYMGMT_CONSOLE_PRINTF(" %08lX%08lX", theValueH, theValueL);
        }
        else
        {
          /* Display ???? */
          KEYMGMT_CONSOLE_PRINTF(" ????????????????");
        }

        /* Check for end-of-line */
        if ((theNumDone % 4) == 3)
          KEYMGMT_CONSOLE_PRINTF("\r\n");

        /* Update for loop */
        theRowID++;
        theNumDone++;
        theNumRows--;
      }
      while (theNumRows > 0);

      /* Unlock */
      doUnlock(theDevice);
    }
    /* Otherwise */
    else
    {
      thereWasAProblem = TRUE;
    }
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
    KEYMGMT_CONSOLE_PRINTF(
		"keymgmt device read <id> <address> <num-to-read>\r\n");
  }

  /* Return */
  return;
}


/******************************************************************************/
/*!
 * \brief  This function implements the "keymgmt device write" command
 * \param[in]  argc    the number of arguments
 * \param[in]  argv[]  an array of pointers to the arguments
 * \return  void
 *
 ******************************************************************************/
static void
doDebugWrite(
  int         argc,
  const char* argv[]
)
{
  /* Locals */
  int thereWasAProblem = FALSE;

  /* Sanity Check */
  if (argc > 2)
  {
    /* More locals */
    KEYMGMT_tDevID theDevID = KEYMGMT_DEVID_0;

    /* Determine theDevID */
    theDevID = (KEYMGMT_tDevID) atoi(argv[0]);

    /* Sanity Check */
    if (theDevID < PLATFORM_DEVID_MAX)
    {
      /* More locals */
      tDevice* theDevice = &(gMyDevices[theDevID]);
      uint32_t theAddress = strtoul(argv[1], NULL, 16);
      uint32_t theValue = strtoul(argv[2], NULL, 16);

      /* Lock */
      doLock(theDevice);

      /* Write it */
      doRegWrite(theDevice, theAddress, theValue);

      /* Unlock */
      doUnlock(theDevice);
    }
    /* Otherwise */
    else
    {
      thereWasAProblem = TRUE;
    }
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
    KEYMGMT_CONSOLE_PRINTF("keymgmt device write <id> <address> <value>\r\n");
  }

  /* Return */
  return;
}


/*****************************************************************************/
/**
*
* This function implements the "keymgmt device" debug command
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
KEYMGMTDEV_Debug(
  int         argc,
  const char* argv[]
)
{
  /* Locals */
  int thereWasAProblem = FALSE;

  /* Sanity Check */
  if (argc > 0)
  {
    /* Check for disable */
    if (strcmp(argv[0], "disable") == 0)
      doDebugDisable(argc-1, argv+1);
    /* Check for enable */
    else if (strcmp(argv[0], "enable") == 0)
      doDebugEnable(argc-1, argv+1);
    /* Check for read */
    else if (strcmp(argv[0], "read") == 0)
      doDebugRead(argc-1, argv+1);
    /* Check for table */
    else if (strcmp(argv[0], "table") == 0)
      doDebugTable(argc-1, argv+1);
    /* Check for write */
    else if (strcmp(argv[0], "write") == 0)
      doDebugWrite(argc-1, argv+1);
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
    KEYMGMT_CONSOLE_PRINTF(
		"keymgmt device <disable|enable|read|table|write>\r\n");
  }

  /* Return */
  return (0);
}
