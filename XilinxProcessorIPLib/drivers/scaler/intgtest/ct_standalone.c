/******************************************************************************
*
*       XILINX IS PROVIDING THIS DESIGN, CODE, OR INFORMATION "AS IS"
*       AS A COURTESY TO YOU, SOLELY FOR USE IN DEVELOPING PROGRAMS AND
*       SOLUTIONS FOR XILINX DEVICES.  BY PROVIDING THIS DESIGN, CODE,
*       OR INFORMATION AS ONE POSSIBLE IMPLEMENTATION OF THIS FEATURE,
*       APPLICATION OR STANDARD, XILINX IS MAKING NO REPRESENTATION
*       THAT THIS IMPLEMENTATION IS FREE FROM ANY CLAIMS OF INFRINGEMENT,
*       AND YOU ARE RESPONSIBLE FOR OBTAINING ANY RIGHTS YOU MAY REQUIRE
*       FOR YOUR IMPLEMENTATION.  XILINX EXPRESSLY DISCLAIMS ANY
*       WARRANTY WHATSOEVER WITH RESPECT TO THE ADEQUACY OF THE
*       IMPLEMENTATION, INCLUDING BUT NOT LIMITED TO ANY WARRANTIES OR
*       REPRESENTATIONS THAT THIS IMPLEMENTATION IS FREE FROM CLAIMS OF
*       INFRINGEMENT, IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
*       FOR A PARTICULAR PURPOSE.
*
*       (c) Copyright 2014 Xilinx Inc.
*       All rights reserved.
*
******************************************************************************/
/*****************************************************************************/
/**
*
* @file ct_standalone.c
*
* Implements Code test (CT) utility driver. Test.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who    Date     Changes
* ----- ------ -------- -----------------------------------------------
* 7.0   adk   01/09/14 First release.
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <unistd.h>
#include "ct.h"

/************************** Constant Definitions *****************************/


/**************************** Type Definitions *******************************/


/***************** Macros (Inline Functions) Definitions *********************/


/************************** Function Prototypes ******************************/


/************************** Variable Definitions *****************************/

unsigned CT_TotalFailures;
unsigned CT_TestFailures;
unsigned CT_TestPass;
static int Verbose = 0;   /* Verbosity flag */

/*****************************************************************************/
/**
*
* Initialize the CT subsystem.
*
* @return None
*
******************************************************************************/
void CT_Init(void)
{
    CT_TestFailures = 0;
    CT_TotalFailures = 0;
    CT_TestPass = 0;
}

/*****************************************************************************/
/**
*
* Reset for a new test and display a test specific message. This involves:
*
*   - Zero out test failure counter.
*   - Print a message specified by the fmt parameter and its associated
*     arguments.
*
* @param	Fmt is a "printf" style format string.
* @param	... is a variable number of arguments that match "Fmt"
*
* @return	None
*
******************************************************************************/
void CT_TestReset(char *Fmt, ...)
{
    va_list Args;

    CT_TestFailures = 0;
    CT_TestPass = 1;

    /*
	 * Print out the format and its associated argument list
	 */
    va_start(Args, Fmt);
    vprintf(Fmt, Args);
    CT_Message("\n========================================================");
    va_end(Args);
    CT_Message("\n");
}

/*****************************************************************************/
/**
* Display current test pass number to user
*
* @return	None
*
******************************************************************************/
void CT_NotifyNextPass(void)
{
    CT_Message("\n=====>Pass %d\n", CT_TestPass++);
}

/*****************************************************************************/
/**
*
* Log a test failure. This involves incrementing test failure and total test
* failure counters, and displaying test specific message.
*
* @param	Fmt is a "printf" style format string.
* @param	... is a variable number of arguments that match "fmt"
*
* @return	None
*
******************************************************************************/
void CT_LogFailure(char* Fmt, ...)
{
    va_list Args;

    /*
     * Increment failure counters
     */
    CT_TestFailures++;
    CT_TotalFailures++;

    /*
     * Print out the format and its associated argument list.
     */
    va_start(Args, Fmt);

    vprintf(Fmt, Args);
    va_end(Args);
}

/*****************************************************************************/
/**
*
* Return the number of test failures since CT_TestReset() was called.
*
* @return	Number of failures logged.
*
******************************************************************************/
unsigned CT_GetTestFailures(void)
{
    return(CT_TestFailures);
}

/*****************************************************************************/
/**
*
* Return the number of test failures since CT_Init() was called.
*
* @return	Total number of failures logged.
*
******************************************************************************/
unsigned CT_GetTotalFailures(void)
{
    return(CT_TotalFailures);
}

/*****************************************************************************/
/**
* Return status of the Xilinx driver assert mechanism.
*
* @return	1 if asserts are disabled, 0 otherwise
*
*****************************************************************************/
int CT_IsAssertDisabled(void)
{
#ifdef NDEBUG
    return(1);
#else
    return(0);
#endif
}

/*****************************************************************************/
/**
*
* Print a message based on the given format string and parameters.
*
* @param	Fmt is a "printf" style format string.
* @param	... is a variable number of arguments that match "fmt"
*
* @return	None
*
******************************************************************************/
void CT_Message(char *Fmt, ...)
{
    va_list Args;

    va_start(Args, Fmt);

    vprintf(Fmt, Args);

    va_end(Args);
}

/*****************************************************************************/
/**
*
* Set a series of bytes in memory to a specific value
*
* @param	AddressPtr is the start address in memory to write
* @param	Value is the value to set memory to
* @param    Bytes is the number of bytes to set
*
* @return   None
*
******************************************************************************/
void CT_MemSet(void *AddressPtr, char Value, unsigned Bytes)
{
    char* AdrPtr = AddressPtr;

    while(Bytes--)
    {
        *AdrPtr++ = Value;
    }
}

/*****************************************************************************/
/**
*
* Retrieve a line of input from the user. A line is defined as all characters
* up to a new line.
*
* @param	PromptPtr Printed before string is accepted to que the user to
*         	enter something.
*
* @param	ResponsePtr Null terminated string with new line stripped
*
* @param  	MaxChars Maximum number of characters to read (excluding null)
*
* @return 	Number of characters read (excluding new line)
*
* @note   	None
*
******************************************************************************/
int CT_GetUserInput(char* PromptPtr, char* ResponsePtr, int MaxChars)
{
    u32 Index;
    u8 Finished;

    /*
     * Display prompt
     */
    if (PromptPtr) printf(PromptPtr);

    /*
     * This basically implements a fgets function. The standalone EDK stdin
     * is apparently buggy because it behaves differently when new line is
     * entered by itself vs. when it is entered after a number of regular chars
     * have been entered.Characters entered are echoed back.
     */
    Finished = 0;
    Index = 0;

    while(!Finished)
    {
        /*
         * Flush out any output pending in stdout
         */
        fflush(stdout);

        /*
         * Wait for a character to arrive
         */
        ResponsePtr[Index] = inbyte();

        /*
         * Normal chars, add them to the string and keep going
         */
        if ((ResponsePtr[Index] >= 0x20) && (ResponsePtr[Index] <=0x7E))
        {
            printf("%c", ResponsePtr[Index++]);
            continue;
        }

        /*
         * Control chars
         */
        switch(ResponsePtr[Index])
        {
            /*
             * Carriage return
             */
            case '\r':
            case '\n':
                ResponsePtr[Index] = 0;
                Finished = 1;
                printf("\n");
                break;

            /*
             * Backspace
             */
            case 0x08:
                if (Index != 0)
                {
                    /*
                     * Erase previous character and move cursor back one space
                     */
                    printf("\b \b");
                    ResponsePtr[--Index] = 0;
                }
                break;

            /*
             * Ignore all other control chars
             */
            default:
                continue;
        }
    }

    return(Index);
}

/*****************************************************************************/
/**
*
* Enable output of CT_MessageV().
*
* @return	None
*
* @note		None
*
******************************************************************************/
void CT_VerboseOn(void)
{
    Verbose = 1;
}

/*****************************************************************************/
/**
* Disable output of CT_MessageV().
*
* @return	None
*
* @note		None
*
******************************************************************************/
void CT_VerboseOff(void)
{
    Verbose = 0;
}
