/******************************************************************************
*
* Copyright (C) 2017 - 2018 Xilinx, Inc.  All rights reserved.
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
/****************************************************************************/
/**
*
* @file xtmr_inject.c
* @addtogroup tmr_inject_v1_1
* @{
*
* Contains required functions for the XTMR_Inject driver. See the xtmr_inject.h
* header file for more details on this driver.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -----------------------------------------------
* 1.0   sa   04/05/17 First release
* 1.1   mus  10/25/18 Updated XTMR_Inject_CfgInitialize to support
*                     64 bit fault address.
*
* </pre>
*
*****************************************************************************/

/***************************** Include Files ********************************/

#include "xtmr_inject.h"
#include "xtmr_inject_l.h"
#include "xil_io.h"

/************************** Constant Definitions ****************************/


/**************************** Type Definitions ******************************/


/************************** Variable Definitions ****************************/

extern UINTPTR xtmr_inject_instr;
static UINTPTR xtmr_inject_addr = (UINTPTR)&xtmr_inject_instr;


/***************** Macros (Inline Functions) Definitions ********************/

#define xor(data, mask) ({		\
  u32 _rval;				\
  __asm__ __volatile__ (		\
    "\t.global xtmr_inject_instr\n"	\
    "\tnop\n"				\
    "xtmr_inject_instr:\n"		\
    "\txor\t%0,%1,%2\n"			\
    : "=d"(_rval) : "d" (data), "d" (mask) \
  ); \
  _rval; \
})


/************************** Function Prototypes *****************************/

static __attribute__ ((noinline, noclone)) u32 injectmask(u32 value, u32 mask)
{
  return xor(value ^ mask, mask);
}


/****************************************************************************/
/**
*
* Initialize a XTMR_Inject instance. The receive and transmit FIFOs of the
* core are not flushed, so the user may want to flush them. The hardware
* device does not have any way to disable the receiver such that any valid
* data may be present in the receive FIFO.  This function disables the core
* interrupt. The baudrate and format of the data are fixed in the hardware
* at hardware build time.
*
* @param	InstancePtr is a pointer to the XTMR_Inject instance.
* @param	Config is a reference to a structure containing information
*		about a specific TMR Inject device. This function initializes
*		an InstancePtr object for a specific device specified by the
*		contents of Config. This function can initialize multiple
*		instance objects with the use of multiple calls giving different
*		Config information on each call.
* @param 	EffectiveAddr is the device base address in the virtual memory
*		address space. The caller is responsible for keeping the address
*		mapping from EffectiveAddr to the device physical base address
*		unchanged once this function is invoked. Unexpected errors may
*		occur if the address mapping changes after this function is
*		called. If address translation is not used, use
*		Config->BaseAddress for this parameters, passing the physical
*		address instead.
*
* @return
* 		- XST_SUCCESS if everything starts up as expected.
*
* @note		The Config pointer argument is not used by this function,
*		but is provided to keep the function signature consistent
*		with other drivers.
*
*****************************************************************************/
int XTMR_Inject_CfgInitialize(XTMR_Inject *InstancePtr,
			      XTMR_Inject_Config *Config,
			      UINTPTR EffectiveAddr)
{
	(void) Config;
	/*
	 * Assert validates the input arguments
	 */
	Xil_AssertNonvoid(InstancePtr != NULL);

	/*
	 * Set some default values.
	 */
	InstancePtr->MagicByte = Config->MagicByte;

	InstancePtr->Address = xtmr_inject_addr;
	InstancePtr->Instruction = xtmr_inject_instr;
	InstancePtr->Magic = 0x00;
	InstancePtr->CpuId = 0;
	InstancePtr->Inject = 0;

	InstancePtr->IsReady = XIL_COMPONENT_IS_READY;

	InstancePtr->RegBaseAddress = EffectiveAddr;

	/* Write to the control register to disable fault injection.
	 */
	XTMR_Inject_WriteReg(InstancePtr->RegBaseAddress, XTI_CR_OFFSET, 0);

	/* Initialize the address inject and instruction inject registers.
	 */
	if (Config->LMBAddrWidth > XTI_STANDARD_LMB_WIDTH) {
		XTMR_Inject_WriteReg64(InstancePtr->RegBaseAddress, XTI_EAIR_OFFSET,
				xtmr_inject_addr);
	} else {
		XTMR_Inject_WriteReg(InstancePtr->RegBaseAddress, XTI_AIR_OFFSET,
				xtmr_inject_addr);
	}
	XTMR_Inject_WriteReg(InstancePtr->RegBaseAddress, XTI_IIR_OFFSET,
				xtmr_inject_instr);

	return XST_SUCCESS;
}

/****************************************************************************/
/**
*
* This function enables fault injection for the indicated CPU.
*
* @param	InstancePtr is a pointer to the XTMR_Inject instance.
* @param	CpuId is the CPU ID of the CPU where faults are to be injected.
*               Must be 1 to 3.
*
* @note		None.
*
*****************************************************************************/
void XTMR_Inject_Enable(XTMR_Inject *InstancePtr, u8 CpuId)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertVoid(CpuId > 0 && CpuId < 3);

	/*
	 * Write to the control register to enable fault injection for the
	 * required CPU.
	 */
	XTMR_Inject_WriteReg(InstancePtr->RegBaseAddress, XTI_CR_OFFSET,
	    0x400 | (CpuId << XTI_CR_CPU) | InstancePtr->MagicByte);

	InstancePtr->Magic = InstancePtr->MagicByte;
	InstancePtr->CpuId = CpuId;
	InstancePtr->Inject = 1;
}


/****************************************************************************/
/**
*
* This function disables fault injection.
*
* @param	InstancePtr is a pointer to the XTMR_Inject instance.
*
* @note		None.
*
*****************************************************************************/
void XTMR_Inject_Disable(XTMR_Inject *InstancePtr)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	/*
	 * Write to the control register to disable fault injection.
	 */
	XTMR_Inject_WriteReg(InstancePtr->RegBaseAddress, XTI_CR_OFFSET, 0);

	InstancePtr->Magic = 0x00;
	InstancePtr->CpuId = 0;
	InstancePtr->Inject = 0;
}


/****************************************************************************/
/**
*
* This function can be used to inject a fault in either:<ul>
* <li>An address bit of a memory or memory mapped register address for
* any read or write access on AXI or LMB.</li>
* <li>A data bit written to memory or a memory mapped register on AXI or LMB,
* as well as output on an AXI Stream.</li>
< </ul>
*
* @param	InstancePtr is a pointer to the XTMR_Inject instance.
* @param	Value is the memory or register address, oro the data.
* @param	Bit is the bit where the fault is injected. Must be 0 to 31.
*
* @return	Either the unchanged value, or value with the fault injected
*               for the CPU set up for fault injection.
*
* @note		None.
*
*****************************************************************************/
u32 XTMR_Inject_InjectBit(XTMR_Inject *InstancePtr, u32 Value, u8 Bit)
{
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertNonvoid(Bit < 32);

	/*
	 * Call the local function responsible for injecting the fault.
	 */
	return injectmask(Value, 1 << (u32)Bit);
}

/****************************************************************************/
/**
*
* This function can be used to inject any number of faults defined by the
* Mask parameter in either:<ul>
* <li>Any address bits of a memory or memory mapped register address for
* any read or write access on AXI or LMB.</li>
* <li>Any data bits written to memory or a memory mapped register on AXI or
* LMB, as well as output on an AXI Stream.</li>
< </ul>
*
* @param	InstancePtr is a pointer to the XTMR_Inject instance.
* @param	Value is the memory or register address, oro the data.
* @param	Mask defines the bits where faults are injected. A fault is
*               injected for every bit in the mask set to 1.
*
* @return	Either the unchanged value, or value with the faults injected
*               for the CPU set up for fault injection.
*
* @note		None.
*
*****************************************************************************/
u32 XTMR_Inject_InjectMask(XTMR_Inject *InstancePtr, u32 Value, u32 Mask)
{
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	/*
	 * Call the local function responsible for injecting the faults.
	 */
	return injectmask(Value, Mask);
}

/** @} */
