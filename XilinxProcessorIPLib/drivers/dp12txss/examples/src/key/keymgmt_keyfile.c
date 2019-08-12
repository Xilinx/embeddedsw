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
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
* THE SOFTWARE.
*
*
*
******************************************************************************/



/* Include Files */
#include "keymgmt.h"
#include "keymgmt_keyfile.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "xil_types.h"
#include "xil_io.h"
#include "aes256.h"
#include "keymgmt_debug.h"
//#include "platform_kc705.h"
#include "eeprom.h"
#include "keygen_config.h"
//#include "xiic.h"

/* Constant Definitions */
#define MAX_NUM_HANDLERS               ( 1)

#define READ_CHUNK_SIZE                (16)


/* Type Definitions */
typedef struct
{
  uint8_t  fMagicValue[4];
  uint8_t  fVersion[4];
  uint8_t  fNumDevKey[4];
  uint8_t  fEncryptKey[32];
  uint8_t  fRffu[20];

} KEYFILE_tHeader;

typedef struct
{
  const uint8_t* fBuf;
  uint32_t       fBufSize;
  uint32_t       fReadIdx;
  uint32_t       fFlags;
  aes256_context fContext;

} tHandler;


/* Local Globals */
static int       gIsInitialized = FALSE;
static tHandler  gMyHandlers[MAX_NUM_HANDLERS];

// ******************************************************
// The AES key in gDefaultKey has to be populated by
// user. Xilinx does not provide these
// ******************************************************

static uint8_t   gDefaultKey[32] =
{
  0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
  0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
  0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
  0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
};

extern int gIsKeyWrittenInEeeprom;

/* Extern Declarations */
extern const uint8_t  KEYMGMT_ENCDATA[];
extern const uint32_t KEYMGMT_ENCDATA_SZ;

/******************************************************************************/
/*!
 * \brief  The function implements the ntohll function
 * \param[in] theIn  the input value
 * \return  The converted value
 *
 ******************************************************************************/
static uint64_t
ntohll(
  uint64_t theIn
)
{
  /* Locals */
  uint64_t theOut = 0;

  /* Check for conversion required */
  if (Xil_Htonl(1) != 1)
  {
    /* Convert it */
    theOut   = Xil_Htonl((uint32_t) theIn);
    theOut <<= 32;
    theOut  |= Xil_Htonl((uint32_t) (theIn >> 32));
  }
  /* Otherwise */
  else
  {
    /* Update theOut */
    theOut = theIn;
  }

  /* Return */
  return (theOut);
}


/******************************************************************************/
/*!
 * \brief  This function extracts a uint32_t from a big endian byte array
 * \param[in] theBuf  the byte array
 * \return  The extracted value
 *
 ******************************************************************************/
static uint32_t
doBeBufToU32(
  const uint8_t* theBuf
)
{
  /* Locals */
  uint32_t theValue;

  /* Determine theValue */
  theValue   = *theBuf++;
  theValue <<= 8;
  theValue  |= *theBuf++;
  theValue <<= 8;
  theValue  |= *theBuf++;
  theValue <<= 8;
  theValue  |= *theBuf++;

  /* Return */
  return (theValue);
}


/******************************************************************************/
/*!
 * \brief  This function retrieves a handler instance
 * \param[in] theHandle  the key file handle to look up
 * \return  The corresponding handler.  NULL if not found.
 *
 ******************************************************************************/
static tHandler*
doGetHandler(
  KEYFILE_tHandle theHandle
)
{
  /* Locals */
  tHandler* theHandler = NULL;

  if (theHandle < MAX_NUM_HANDLERS)
    theHandler = &(gMyHandlers[theHandle]);

  /* Return */
  return (theHandler);
}


/******************************************************************************/
/*!
 * \brief  This function initializes a key file handler
 * \param[in] theHandler  the handler to initialize
 * \return  void
 *
 ******************************************************************************/
static void
doInit(
  tHandler* theHandler
)
{
  /* Initialize it */
  memset(theHandler, 0, sizeof(tHandler));

  /* Initialize the AES context */
  aes256_done(&theHandler->fContext);

  /* Return */
  return;
}


/******************************************************************************/
/*!
 * \brief  This function reads a buffer of data from the key file (BRAM)
 * \param[in] theHandler  the handler to use
 * \param[in] theBuf  the buffer to place the data read
 * \param[in] theBufSize  the size of the buffer
 * \return  Zero (0) for success.  Otherwise non-zero.
 *
 ******************************************************************************/
static int
doReadBram(
  tHandler* theHandler,
  void*     theBuf,
  size_t    theBufSize
)
{
  /* Locals */
  int  theStatus = 0;

  /* Sanity Check */
  if (((theBufSize % READ_CHUNK_SIZE) == 0)
		  && (theHandler->fReadIdx < theHandler->fBufSize))
  {
    /* More locals */
    uint8_t* theU8Ptr = (uint8_t*) theBuf;
    uint32_t theNumThisTime = 0;

    /* Iterate through the chunks */
    do
    {
      /* Determine theNumThisTime */
      theNumThisTime = READ_CHUNK_SIZE;
      if ((theHandler->fReadIdx + theNumThisTime) > theHandler->fBufSize)
        theNumThisTime = (theHandler->fBufSize - theHandler->fReadIdx);

      /* Update theU8Ptr */
      memcpy(theU8Ptr, &(theHandler->fBuf[theHandler->fReadIdx]),
			  theNumThisTime);


      /* Decrypt it */
      aes256_decrypt_ecb(&theHandler->fContext, theU8Ptr);

      /* Update for loop */
      theHandler->fReadIdx += theNumThisTime;
      theU8Ptr += theNumThisTime;
      theBufSize -= theNumThisTime;
    }
    while ((theBufSize > 0) && (theNumThisTime != 0));
  }
  /* Otherwise */
  else
  {
    /* Update theStatus */
    theStatus = -1;
  }

  /* Return */
  return (theStatus);
}

/******************************************************************************/
/*!
 * \brief  This function reads a buffer of data from the key file
 * \param[in] theHandler  the handler to use
 * \param[in] theBuf  the buffer to place the data read
 * \param[in] theBufSize  the size of the buffer
 * \return  Zero (0) for success.  Otherwise non-zero.
 *
 ******************************************************************************/
static int
doRead(
  tHandler* theHandler,
  void*     theBuf,
  size_t    theBufSize
)
{
	int  theStatus = 0;

	/* The keys will always be read from the EEPROM.
	 * If the Keys have not been initialized
	 * then the user will get an error */
	if (gIsKeyWrittenInEeeprom != TRUE){
		xil_printf("##_");
		theStatus = doReadBram(theHandler, theBuf, theBufSize);
		return theStatus;
	}

  /* Locals */
  int  eeprom_addr=theHandler->fReadIdx;


  /* Define a 1K Buffer for reading the Keys, this should
   * 	ideally be 1K for KC705 although can be more */
  static u8 MasterReadBuf[1024];

  theStatus = EepromReadData(MasterReadBuf, theHandler->fBufSize);
  if (theStatus != XST_SUCCESS) {
	  xil_printf("%s:%d:Error: Reading Keys from EERPOM\r\n",__func__,__LINE__);
	  return XST_FAILURE;
  }

  /* Sanity Check */
  if (((theBufSize % READ_CHUNK_SIZE) == 0)
		  && (theHandler->fReadIdx < theHandler->fBufSize))
  {
    /* More locals */
    uint8_t* theU8Ptr = (uint8_t*) theBuf;
    uint32_t theNumThisTime = 0;

    /* Iterate through the chunks */
    do
    {
      /* Determine theNumThisTime */
      theNumThisTime = READ_CHUNK_SIZE;
      if ((theHandler->fReadIdx + theNumThisTime) > theHandler->fBufSize)
        theNumThisTime = (theHandler->fBufSize - theHandler->fReadIdx);

#if IIC_EEPROM_READ

      memcpy(theU8Ptr, &MasterReadBuf[theHandler->fReadIdx], theNumThisTime);


	  eeprom_addr += READ_CHUNK_SIZE;

#else
      /* Update theU8Ptr */
      memcpy(theU8Ptr, &(theHandler->fBuf[theHandler->fReadIdx]),
					  theNumThisTime);

#endif	/* IIC_EEPROM_READ */

      /* Decrypt it */
      aes256_decrypt_ecb(&theHandler->fContext, theU8Ptr);

      /* Update for loop */
      theHandler->fReadIdx += theNumThisTime;
      theU8Ptr += theNumThisTime;
      theBufSize -= theNumThisTime;
    }
    while ((theBufSize > 0) && (theNumThisTime != 0));
  }
  /* Otherwise */
  else
  {
    /* Update theStatus */
    theStatus = -1;
  }

  /* Return */
  return (theStatus);
}


/******************************************************************************/
/*!
 * \brief  This function initializes the module
 * \return  void
 *
 ******************************************************************************/
void
KEYFILE_Init(
  void
)
{
  /* Sanity Check */
  if (!gIsInitialized)
  {
    /* Locals */
    tHandler*    theHandler = gMyHandlers;
    unsigned int theNumLeft = MAX_NUM_HANDLERS;

    /* Iterate through theHandlers */
    do
    {
      /* Initialize it */
      doInit(theHandler);

      /* Update for loop */
      theHandler++;
      theNumLeft--;
    }
    while (theNumLeft > 0);

    /* Update gIsInitialized */
    gIsInitialized = TRUE;
  }

  /* Return */
  return;
}


/******************************************************************************/
/*!
 * \brief  This function opens and validates a key file for reading
 * \param[in] theFileName  the name of the file
 * \param[in] theKey  the decryption key to use
 * \return  The key file handle.  KEYFILE_HANDLE_NULL if an error encountered
 *
 ******************************************************************************/
KEYFILE_tHandle
KEYFILE_Validate(
  const char* theFileName,
  void*       theKey
)
{
  /* Locals */
  KEYFILE_tHandle theHandle = KEYFILE_HANDLE_NULL;

  /* Sanity Check */
  if (theFileName != NULL)
  {
    /* More locals */
    tHandler*       theHandler = gMyHandlers;
    uint8_t*        theAesKey = (uint8_t*) theKey;
    KEYFILE_tHeader theHeader;

    /* Update theKey if needed */
    if (theAesKey == NULL)
      theAesKey = gDefaultKey;

    /* Update theHandler */
    if(gIsKeyWrittenInEeeprom)
    {
	theHandler->fBuf = NULL;
	/* The below variable need to be initialised to
	 * 		size of keys in EEPROM */
	theHandler->fBufSize = KEYMGMT_ENCDATA_SZ_EEPROM;
    }
    else
    {
	theHandler->fBuf = KEYMGMT_ENCDATA;
	theHandler->fBufSize = KEYMGMT_ENCDATA_SZ;
    }
    theHandler->fReadIdx = 0;
    theHandler->fFlags = 0;
    aes256_init(&theHandler->fContext, theAesKey);

    /* Read theHeader */
    doRead(theHandler, &theHeader, sizeof(theHeader));

    /* Sanity Check */
    if (doBeBufToU32(theHeader.fMagicValue) == 0xDEADBEEFul)
    {
      /* Check version */
      if (doBeBufToU32(theHeader.fVersion) == 0x01ul)
      {
        /* Update theHandle */
        theHandle = (theHandler - gMyHandlers);

        /* Check if the header contains the number of tables */
        if (doBeBufToU32(theHeader.fNumDevKey) > 0)
        {
          /* Update theHandler */
          theHandler->fBufSize  = sizeof(KEYFILE_tHeader);
          theHandler->fBufSize += sizeof(KEYFILE_tDevKeyTable)
					  * doBeBufToU32(theHeader.fNumDevKey);
        }
      }
    }

    /* Check for error */
    if (theHandle == KEYFILE_HANDLE_NULL)
    {
      /* Re-initialize theHandler */
      doInit(theHandler);
    }
  }

  /* Return */
  return (theHandle);
}


/******************************************************************************/
/*!
 * \brief  This function closes a key file
 * \param[in] theHandle  a handle of the file to close
 * \return  Zero (0) for success.  Otherwise non-zero.
 *
 ******************************************************************************/
int
KEYFILE_Close(
  KEYFILE_tHandle* theHandlePtr
)
{
  /* Locals */
  int theStatus = 0;

  /* Sanity Check */
  if ((theHandlePtr != NULL) && (*theHandlePtr != KEYFILE_HANDLE_NULL))
  {
    /* More locals */
    tHandler* theHandler = doGetHandler(*theHandlePtr);

    /* Sanity Check (again) */
    if (theHandler != NULL)
    {
      /* Close off the encryption */
      aes256_done(&theHandler->fContext);

      /* Re-initialize theHandler */
      doInit(theHandler);

      /* Update theHandlePtr */
      *theHandlePtr = KEYFILE_HANDLE_NULL;
    }
    /* Otherwise */
    else
    {
      /* Update theStatus */
      theStatus = -2;
    }
  }
  /* Otherwise */
  else
  {
    /* Update theStatus */
    theStatus = -1;
  }

  /* Return */
  return (theStatus);
}


/******************************************************************************/
/*!
 * \brief  This function reads a device key table from a key file
 * \param[in] theHandle  a handle of the file to read from
 * \param[out] theTable  the device key table to copy the data read
 * \return  Zero (0) for success.  Otherwise non-zero.
 *
 ******************************************************************************/
int
KEYFILE_Read(
  KEYFILE_tHandle        theHandle,
  KEYFILE_tDevKeyTable*  theTable
)
{
  /* Locals */
  tHandler* theHandler = NULL;
  int       theStatus = 0;

  /* Determine theHandler */
  theHandler = doGetHandler(theHandle);

  /* Sanity Check */
  if (theHandler != NULL)
  {
    /* Read it */
    if (doRead(theHandler, theTable, sizeof(KEYFILE_tDevKeyTable)) == 0)
    {
      /* More locals */
      int theIdx;

      /* Adjust the endian-ness to host order */
      for (theIdx=0; theIdx<42; theIdx++)
        theTable->fData.fU64[theIdx] = ntohll(theTable->fData.fU64[theIdx]);
    }
    /* Otherwise */
    else
    {
      /* Update theStatus */
      theStatus = -2;
    }
  }
  /* Otherwise */
  else
  {
    /* Update theStatus */
    theStatus = -1;
  }

  /* Return */
  return (theStatus);
}
