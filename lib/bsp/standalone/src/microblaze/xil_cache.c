/******************************************************************************
* Copyright (c) 2009 - 2021 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2023 Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xil_cache.c
*
* @addtogroup microblaze_cache_apis Microblaze Cache APIs
* @{
*
* This contains implementation of cache related driver functions.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -------------------------------------------------------
* 1.00  hbm  07/28/09 Initial release
* 3.10  asa  05/04/13 This version of MicroBlaze BSP adds support for system
*					  cache/L2 cache. Existing APIs in this file are modified
*					  to add support for L2 cache.
*					  These changes are done for implementing PR #697214.
* </pre>
*
*
******************************************************************************/

#include "xil_cache.h"
#include "mb_interface.h"
#include "bspconfig.h"

#define XIL_MICROBLAZE_EXT_CACHE_LINE_LEN	16 /**< Size of Cache line */

/************************** Variable Definitions *****************************/
#ifdef SDT
static XMicroblaze_Config *CfgPtr = XGet_CpuCfgPtr();
#endif

/****************************************************************************/
/**
*
* @brief    Disable the data cache.
*
* @return   None.
*
****************************************************************************/
void Xil_DCacheDisable(void)
{
	Xil_DCacheFlush();
	Xil_DCacheInvalidate();
	Xil_L1DCacheDisable();
}

/****************************************************************************/
/**
*
* @brief    Disable the instruction cache.
*
* @return   None.
*
*
****************************************************************************/
void Xil_ICacheDisable(void)
{
	Xil_ICacheInvalidate();
	Xil_L1ICacheDisable();
}

#ifdef SDT
void microblaze_flush_dcache(void) {
#ifndef VERSAL_PLM
	UINTPTR startadr=0, endadr=0;

	if (CfgPtr->UseDcache && CfgPtr->AllowDcaheWr) {

		/*
		 * Align start and end address with cache line length
		 */
		startadr = ( CfgPtr->DcacheBaseaddr & (- (CfgPtr->DcacheLineLen*4)));
		endadr = ((CfgPtr->DcacheBaseaddr + CfgPtr->DcacheByteSize) & (- (CfgPtr->DcacheLineLen*4)));

		while ( startadr < endadr) {
			mtwdcflush(startadr);
			startadr += (CfgPtr->DcacheLineLen * 4);
		}
	}
#endif
}

void microblaze_flush_dcache_range(UINTPTR cacheaddr, u32 len) {
#ifndef VERSAL_PLM
	 UINTPTR startadr=0, endadr=0, temp=0;

	 if (CfgPtr->UseDcache && CfgPtr->AllowDcaheWr) {
		if (CfgPtr->DcacheUseWriteback) {
			temp = mfmsr();
			mtmsr(temp & ~(XIL_INTERRUPTS_MASK | XMICROBLAZE_DCACHE_MASK));
		}

		/*
		 * Align start and end address with cache line length
		 */
		startadr = ( cacheaddr & (- (CfgPtr->DcacheLineLen * 4)));
		endadr = cacheaddr + len - 1;
		endadr = (endadr) & (- (CfgPtr->DcacheLineLen * 4));

		while ( startadr < endadr) {
			if (CfgPtr->DcacheUseWriteback) {
				mtwdcflush(startadr);
				startadr += (CfgPtr->DcacheLineLen * 4);
			} else {
				mtwdc(startadr);
				startadr += (CfgPtr->DcacheLineLen * 4);
			}
		}
		if (CfgPtr->DcacheUseWriteback) {
			mtmsr(temp);
		}
	}
#endif
}

void microblaze_invalidate_dcache(void) {
#ifndef VERSAL_PLM
	 UINTPTR startadr=0, endadr=0, temp=0;

	 if (CfgPtr->UseDcache && CfgPtr->AllowDcaheWr) {
		if (CfgPtr->DcacheUseWriteback) {
			temp = mfmsr();
			mtmsr(temp & ~(XIL_INTERRUPTS_MASK | XMICROBLAZE_DCACHE_MASK));
		}

		/*
		 * Align start and end address with cache line length
		 */
		startadr = ( CfgPtr->DcacheBaseaddr & (- (CfgPtr->DcacheLineLen*4)));
		endadr = ((CfgPtr->DcacheBaseaddr + CfgPtr->DcacheByteSize) & (- (CfgPtr->DcacheLineLen*4)));

		while ( startadr < endadr) {
			if (CfgPtr->DcacheUseWriteback) {
				mtwdc(startadr);
				startadr += (CfgPtr->DcacheLineLen * 4);
			}
		}
		if (CfgPtr->DcacheUseWriteback) {
			mtmsr(temp);
		}
	}
#endif
}

void microblaze_invalidate_dcache_range(UINTPTR cacheaddr, u32 len) {
#ifndef VERSAL_PLM
	 UINTPTR startadr=0, endadr=0, temp=0;
	 INTPTR count=0;

	 if (CfgPtr->UseDcache && CfgPtr->AllowDcaheWr) {
		if (CfgPtr->DcacheUseWriteback) {
			temp = mfmsr();
			mtmsr(temp & ~(XIL_INTERRUPTS_MASK | XMICROBLAZE_DCACHE_MASK));
		}

		/*
		 * Align start and end address with cache line length
		 */
		startadr = ( cacheaddr & (- (CfgPtr->DcacheLineLen * 4)));
		endadr = cacheaddr + len - 1;
		endadr = (endadr & (- (CfgPtr->DcacheLineLen * 4)));

		count  = (endadr - startadr);
		if (CfgPtr->DcacheUseWriteback) {
			while ( count >  0) {
				mtwdcclear(startadr, count);
				count -= (CfgPtr->DcacheLineLen * 4);
			}
		} else {
			while ( startadr < endadr) {
				mtwdc(startadr);
				startadr += (CfgPtr->DcacheLineLen * 4);
			}
		}
		if (CfgPtr->DcacheUseWriteback) {
			mtmsr(temp);
		}
	}
#endif
}

void microblaze_flush_cache_ext(void) {
#ifndef VERSAL_PLM
	UINTPTR startadr=0, endadr=0;

	if ((CfgPtr->Interconnect > 3) && CfgPtr->AllowDcaheWr) {

		/*
		 * Align start and end address with cache line length
		 */
		startadr = ( CfgPtr->DcacheBaseaddr & (- (CfgPtr->DcacheLineLen*4)));
		endadr = (( CfgPtr->DcacheHighaddr - CfgPtr->DcacheBaseaddr) );
		endadr = (endadr & (- (4 * XIL_MICROBLAZE_EXT_CACHE_LINE_LEN)));

		while ( startadr < endadr) {
			mtwdcextflush(startadr);
			startadr += (CfgPtr->DcacheLineLen * 4);
		}
	}
#endif
}

void microblaze_flush_cache_ext_range(UINTPTR cacheaddr, u32 len) {
#ifndef VERSAL_PLM
	UINTPTR startadr=0, endadr=0;

	if ((CfgPtr->Interconnect > 3) && CfgPtr->AllowDcaheWr) {

		/*
		 * Align start and end address with cache line length
		 */
		startadr = ( cacheaddr & (- (XIL_MICROBLAZE_EXT_CACHE_LINE_LEN*4)));
		endadr = cacheaddr + len -1;
		endadr = (endadr & (- (4 * XIL_MICROBLAZE_EXT_CACHE_LINE_LEN)));

		while ( startadr < endadr) {
			mtwdcextflush(startadr);
			startadr += (CfgPtr->DcacheLineLen * 4);
		}
	}
#endif
}

void microblaze_invalidate_cache_ext(void) {
#ifndef VERSAL_PLM
	UINTPTR startadr=0, endadr=0;

	if ((CfgPtr->Interconnect > 3) && CfgPtr->AllowDcaheWr) {

		/*
		 * Align start and end address with cache line length
		 */
		startadr = ( CfgPtr->DcacheBaseaddr & (- (CfgPtr->DcacheLineLen*4)));
		endadr = (( CfgPtr->DcacheHighaddr - CfgPtr->DcacheBaseaddr) );
		endadr = (endadr & (- (4 * XIL_MICROBLAZE_EXT_CACHE_LINE_LEN)));

		while ( startadr < endadr) {
			mtwdcextclear(startadr);
			startadr += (CfgPtr->DcacheLineLen * 4);
		}
	}
#endif
}

void microblaze_invalidate_cache_ext_range(UINTPTR cacheaddr, u32 len) {
#ifndef VERSAL_PLM
	UINTPTR startadr=0, endadr=0;

	if ((CfgPtr->Interconnect > 3) && CfgPtr->AllowDcaheWr) {

		/*
		 * Align start and end address with cache line length
		 */
		startadr = ( cacheaddr & (- (XIL_MICROBLAZE_EXT_CACHE_LINE_LEN*4)));
		endadr = cacheaddr + len -1;
		endadr = (endadr & (- (4 * XIL_MICROBLAZE_EXT_CACHE_LINE_LEN)));

		while ( startadr < endadr) {
			mtwdcextclear(startadr);
			startadr += (CfgPtr->DcacheLineLen * 4);
		}
	}
#endif
}

void microblaze_invalidate_icache(void) {
#ifndef VERSAL_PLM
	 UINTPTR startadr=0, endadr=0, temp=0;

	 if (CfgPtr->UseIcache && CfgPtr->AllowIcacheWr) {
		if (CfgPtr->DcacheUseWriteback) {
			temp = mfmsr();
			mtmsr(temp & ~(XIL_INTERRUPTS_MASK | XMICROBLAZE_ICACHE_MASK));
		}

		/*
		 * Align start and end address with cache line length
		 */
		startadr = ( CfgPtr->IcacheBaseaddr & (- (CfgPtr->IcacheLineLen*4)));
		endadr = ((CfgPtr->IcacheBaseaddr + CfgPtr->CacheByteSize) & (- (CfgPtr->IcacheLineLen*4)));

		while ( startadr < endadr) {
				mtwic(startadr);
				startadr += (CfgPtr->IcacheLineLen * 4);
		}
		if (CfgPtr->DcacheUseWriteback) {
			mtmsr(temp);
		}
	}
#endif
}

void microblaze_invalidate_icache_range(UINTPTR cacheaddr, u32 len) {
#ifndef VERSAL_PLM
	 UINTPTR startadr=0, endadr=0, temp=0;

	 if (CfgPtr->UseIcache && CfgPtr->AllowIcacheWr) {
		if (CfgPtr->DcacheUseWriteback) {
			temp = mfmsr();
			mtmsr(temp & ~(XIL_INTERRUPTS_MASK | XMICROBLAZE_ICACHE_MASK));
		}

		/*
		 * Align start and end address with cache line length
		 */
		startadr = ( CfgPtr->IcacheBaseaddr & (- (CfgPtr->IcacheLineLen*4)));
		endadr = cacheaddr + len - 1;
		endadr = (endadr) & (- (CfgPtr->IcacheLineLen * 4));

		while ( startadr < endadr) {
				mtwic(startadr);
				startadr += (CfgPtr->IcacheLineLen * 4);
		}
		if (CfgPtr->DcacheUseWriteback) {
			mtmsr(temp);
		}
	}
#endif
}

void microblaze_disable_dcache(void) {
#ifndef VERSAL_PLM
	UINTPTR val=0;

	if (CfgPtr->AllowDcaheWr) {
		microblaze_flush_dcache();
	}

	if (CfgPtr->UseMsrInstr) {
		msrclr(XMICROBLAZE_DCACHE_MASK);
	} else {
		val = mfmsr();
		mtmsr(val & (~XMICROBLAZE_DCACHE_MASK));
	}
#endif
}

void microblaze_enable_dcache(void) {
#ifndef VERSAL_PLM
	UINTPTR val=0;

	if (CfgPtr->UseMsrInstr) {
		msrset(XMICROBLAZE_DCACHE_MASK);
	} else {
		val = mfmsr();
		mtmsr(val | (XMICROBLAZE_DCACHE_MASK));
	}
#endif
}
void microblaze_disable_icache(void) {
#ifndef VERSAL_PLM
	UINTPTR val=0;

	if (CfgPtr->UseMsrInstr) {
		msrclr(XMICROBLAZE_ICACHE_MASK);
	} else {
		val = mfmsr();
		mtmsr(val & (~XMICROBLAZE_ICACHE_MASK));
	}
#endif
}

void microblaze_enable_icache(void) {
#ifndef VERSAL_PLM
	UINTPTR val=0;

	if (CfgPtr->UseMsrInstr) {
		msrset(XMICROBLAZE_ICACHE_MASK);
	} else {
		val = mfmsr();
		mtmsr(val | (XMICROBLAZE_ICACHE_MASK));
	}
#endif
}

void Xil_L1DCacheFlushRange(UINTPTR Addr,u32 Len) {
#ifndef VERSAL_PLM
	XMicroblaze_Config *CfgPtr = XGet_CpuCfgPtr();
	if (CfgPtr->DcacheUseWriteback) {
		microblaze_flush_dcache_range((Addr), (Len));
	} else {
		microblaze_invalidate_dcache_range((Addr), (Len));
	}
#endif
}

void Xil_L1DCacheFlush(void) {
#ifndef VERSAL_PLM
	XMicroblaze_Config *CfgPtr = XGet_CpuCfgPtr();
	if (CfgPtr->DcacheUseWriteback) {
		microblaze_flush_dcache();
	} else {
		microblaze_invalidate_dcache();
	}
#endif
}
#endif
