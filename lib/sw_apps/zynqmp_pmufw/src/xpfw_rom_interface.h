/*
* Copyright (c) 2015 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
 */


/**
 *
 * PMU_ROM SERVICE EXTENSION IDCODES
 *
 *  All Services implemented in the ROM can be extended or overridden by
 *  firmware loaded into the PMU_RAM. Including the IRQ routing infrastructure.
 *  Aside from the PMU_ROM Service Functions, other extendable/overridable
 *  `hooks` are provided to the firmware. These PMU_ROM Service Hooks are
 *  included in the Extension Table but also have their IDCODES highlighted
 *  below.
 *
 * Before calling the default implementation of these services, ROM checks the
 * index indicated below in the Service Extension Table for a function address.
 * If a function pointer is found, ROM will call that function /instead/ of the
 * default ROM function. However, as an argument to the FW function, is a
 * callback to the default ROM function, thus allowing the overriding FW to
 * implmentent wrapping logic around existing ROM behavior.
 *
 * @note: These Identifiers are also used to identify the service mode
 *        error.
 */

#ifndef XPFW_ROM_INTERFACE_H_
#define XPFW_ROM_INTERFACE_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "xil_types.h"
#include "xil_io.h"
#include "xstatus.h"
#include "efuse.h"

#define IS_IPDISABLED(mask)  ((Xil_In32(EFUSE_IPDISABLE) & (mask)) != 0U)

enum xpbr_serv_ext_id {
        XPBR_SERV_EXT_TBL_BASE                    = 0U,
        /*       RESERVED                           1U   */
        /*       RESERVED                           2U   */
        XPBR_SERV_EXT_PIT0                        = 3U,
        XPBR_SERV_EXT_PIT1                        = 4U,
        XPBR_SERV_EXT_PIT2                        = 5U,
        XPBR_SERV_EXT_PIT3                        = 6U,
        /*       RESERVED                           7U   */
        /*       RESERVED                           8U   */
        /*       RESERVED                           9U   */
        /*       RESERVED                           10U  */
        XPBR_SERV_EXT_TMRFAULT                    = 11U,
        XPBR_SERV_EXT_GPI1                        = 12U,
        XPBR_SERV_EXT_GPI2                        = 13U,
        XPBR_SERV_EXT_GPI3                        = 14U,
        /*       RESERVED                           15U  */
        XPBR_SERV_EXT_COR_ECC_HANDLER             = 16U,
        XPBR_SERV_EXT_RTCEVERYSECOND              = 17U,
        XPBR_SERV_EXT_RTCALARM                    = 18U,
        XPBR_SERV_EXT_IPI0                        = 19U,
        XPBR_SERV_EXT_IPI1                        = 20U,
        XPBR_SERV_EXT_IPI2                        = 21U,
        XPBR_SERV_EXT_IPI3                        = 22U,
        XPBR_SERV_EXT_FW_REQS                     = 23U,
        XPBR_SERV_EXT_ISO_REQS                    = 24U,
        XPBR_SERV_EXT_HWRST                       = 25U,
        XPBR_SERV_EXT_SWRST_REQS                  = 26U,
        XPBR_SERV_EXT_PWRUP_REQS                  = 27U,
        XPBR_SERV_EXT_PWRDN_REQS                  = 28U,
        XPBR_SERV_EXT_INVADDR                     = 29U,
        /*       RESERVED                           30U  */
        /*       RESERVED                           31U  */
        XPBR_SERV_EXT_ACPU0WAKE                   = 32U,
        XPBR_SERV_EXT_ACPU1WAKE                   = 33U,
        XPBR_SERV_EXT_ACPU2WAKE                   = 34U,
        XPBR_SERV_EXT_ACPU3WAKE                   = 35U,
        XPBR_SERV_EXT_R50WAKE                     = 36U,
        XPBR_SERV_EXT_R51WAKE                     = 37U,
        XPBR_SERV_EXT_USB0WAKE                    = 38U,
        XPBR_SERV_EXT_USB1WAKE                    = 39U,
        XPBR_SERV_EXT_DAPFPDWAKE                  = 40U,
        XPBR_SERV_EXT_DAPRPUWAKE                  = 41U,
        XPBR_SERV_EXT_MIO0WAKE                    = 42U,
        XPBR_SERV_EXT_MIO1WAKE                    = 43U,
        XPBR_SERV_EXT_MIO2WAKE                    = 44U,
        XPBR_SERV_EXT_MIO3WAKE                    = 45U,
        XPBR_SERV_EXT_MIO4WAKE                    = 46U,
        XPBR_SERV_EXT_MIO5WAKE                    = 47U,
        XPBR_SERV_EXT_FPDGICPROXYWAKE             = 48U,
        /*       RESERVED                           49U  */
        /*       RESERVED                           50U  */
        /*       RESERVED                           51U  */
        XPBR_SERV_EXT_ACPU0DBGPWRUP               = 52U,
        XPBR_SERV_EXT_ACPU1DBGPWRUP               = 53U,
        XPBR_SERV_EXT_ACPU2DBGPWRUP               = 54U,
        XPBR_SERV_EXT_ACPU3DBGPWRUP               = 55U,
        /*       RESERVED                           56U  */
        /*       RESERVED                           57U  */
        /*       RESERVED                           58U  */
        /*       RESERVED                           59U  */
        XPBR_SERV_EXT_ERROR1                      = 60U,
        XPBR_SERV_EXT_ERROR2                      = 61U,
        XPBR_SERV_EXT_AXIAIBERR                   = 62U,
        XPBR_SERV_EXT_APBAIBERR                   = 63U,
        XPBR_SERV_EXT_ACPU0SLEEP                  = 64U,
        XPBR_SERV_EXT_ACPU1SLEEP                  = 65U,
        XPBR_SERV_EXT_ACPU2SLEEP                  = 66U,
        XPBR_SERV_EXT_ACPU3SLEEP                  = 67U,
        XPBR_SERV_EXT_R50SLEEP                    = 68U,
        XPBR_SERV_EXT_R51SLEEP                    = 69U,
        /*       RESERVED                           70U  */
        /*       RESERVED                           71U  */
        XPBR_SERV_EXT_RCPU0_DBG_RST               = 72U,
        XPBR_SERV_EXT_RCPU1_DBG_RST               = 73U,
        /*       RESERVED                           74U  */
        /*       RESERVED                           75U  */
        /*       RESERVED                           76U  */
        /*       RESERVED                           77U  */
        /*       RESERVED                           78U  */
        /*       RESERVED                           79U  */
        XPBR_SERV_EXT_ACPU0_CP_RST                = 80U,
        XPBR_SERV_EXT_ACPU1_CP_RST                = 81U,
        XPBR_SERV_EXT_ACPU2_CP_RST                = 82U,
        XPBR_SERV_EXT_ACPU3_CP_RST                = 83U,
        XPBR_SERV_EXT_ACPU0_DBG_RST               = 84U,
        XPBR_SERV_EXT_ACPU1_DBG_RST               = 85U,
        XPBR_SERV_EXT_ACPU2_DBG_RST               = 86U,
        XPBR_SERV_EXT_ACPU3_DBG_RST               = 87U,
        /*       RESERVED                           88U  */
        /*       RESERVED                           89U  */
        /*       RESERVED                           90U  */
        /*       RESERVED                           91U  */
        /*       RESERVED                           92U  */
        XPBR_SERV_EXT_VCCAUX_DISCONNECT           = 93U,
        XPBR_SERV_EXT_VCCINT_DISCONNECT           = 94U,
        XPBR_SERV_EXT_VCCINTFP_DISCONNECT         = 95U,
        XPBR_SERV_EXT_PWRUPACPU0                  = 96U,
        XPBR_SERV_EXT_PWRUPACPU1                  = 97U,
        XPBR_SERV_EXT_PWRUPACPU2                  = 98U,
        XPBR_SERV_EXT_PWRUPACPU3                  = 99U,
        XPBR_SERV_EXT_PWRUPPP0                    = 100U,
        XPBR_SERV_EXT_PWRUPPP1                    = 101U,
        /*       RESERVED                           102U */
        XPBR_SERV_EXT_PWRUPL2BANK0                = 103U,
        /*       RESERVED                           104U */
        /*       RESERVED                           105U */
        XPBR_SERV_EXT_PWRUPRPU                    = 106U,
        /*       RESERVED                           107U */
        XPBR_SERV_EXT_PWRUPTCM0A                  = 108U,
        XPBR_SERV_EXT_PWRUPTCM0B                  = 109U,
        XPBR_SERV_EXT_PWRUPTCM1A                  = 110U,
        XPBR_SERV_EXT_PWRUPTCM1B                  = 111U,
        XPBR_SERV_EXT_PWRUPOCMBANK0               = 112U,
        XPBR_SERV_EXT_PWRUPOCMBANK1               = 113U,
        XPBR_SERV_EXT_PWRUPOCMBANK2               = 114U,
        XPBR_SERV_EXT_PWRUPOCMBANK3               = 115U,
        XPBR_SERV_EXT_PWRUPUSB0                   = 116U,
        XPBR_SERV_EXT_PWRUPUSB1                   = 117U,
        XPBR_SERV_EXT_PWRUPFPD                    = 118U,
        XPBR_SERV_EXT_PWRUPPLD                    = 119U,
        /*       RESERVED                           120U */
        /*       RESERVED                           121U */
        /*       RESERVED                           122U */
        /*       RESERVED                           123U */
        /*       RESERVED                           124U */
        /*       RESERVED                           125U */
        /*       RESERVED                           126U */
        /*       RESERVED                           127U */
        XPBR_SERV_EXT_PWRDNACPU0                  = 128U,
        XPBR_SERV_EXT_PWRDNACPU1                  = 129U,
        XPBR_SERV_EXT_PWRDNACPU2                  = 130U,
        XPBR_SERV_EXT_PWRDNACPU3                  = 131U,
        XPBR_SERV_EXT_PWRDNPP0                    = 132U,
        XPBR_SERV_EXT_PWRDNPP1                    = 133U,
        /*       RESERVED                           134U */
        XPBR_SERV_EXT_PWRDNL2BANK0                = 135U,
        /*       RESERVED                           136U */
        /*       RESERVED                           137U */
        XPBR_SERV_EXT_PWRDNRPU                    = 138U,
        /*       RESERVED                           139U */
        XPBR_SERV_EXT_PWRDNTCM0A                  = 140U,
        XPBR_SERV_EXT_PWRDNTCM0B                  = 141U,
        XPBR_SERV_EXT_PWRDNTCM1A                  = 142U,
        XPBR_SERV_EXT_PWRDNTCM1B                  = 143U,
        XPBR_SERV_EXT_PWRDNOCMBANK0               = 144U,
        XPBR_SERV_EXT_PWRDNOCMBANK1               = 145U,
        XPBR_SERV_EXT_PWRDNOCMBANK2               = 146U,
        XPBR_SERV_EXT_PWRDNOCMBANK3               = 147U,
        XPBR_SERV_EXT_PWRDNUSB0                   = 148U,
        XPBR_SERV_EXT_PWRDNUSB1                   = 149U,
        XPBR_SERV_EXT_PWRDNFPD                    = 150U,
        XPBR_SERV_EXT_PWRDNPLD                    = 151U,
        /*       RESERVED                           152U */
        /*       RESERVED                           153U */
        /*       RESERVED                           154U */
        /*       RESERVED                           155U */
        /*       RESERVED                           156U */
        /*       RESERVED                           157U */
        /*       RESERVED                           158U */
        /*       RESERVED                           159U */
        XPBR_SERV_EXT_FPISOLATION                 = 160U,
        XPBR_SERV_EXT_PLISOLATION                 = 161U,
        XPBR_SERV_EXT_PLNONPCAPISO                = 162U,
        /*       RESERVED                           163U */
        XPBR_SERV_EXT_FPLOCKISO                   = 164U,
        /*       RESERVED                           165U */
        /*       RESERVED                           166U */
        /*       RESERVED                           167U */
        /*       RESERVED                           168U */
        /*       RESERVED                           169U */
        /*       RESERVED                           170U */
        /*       RESERVED                           171U */
        /*       RESERVED                           172U */
        /*       RESERVED                           173U */
        /*       RESERVED                           174U */
        /*       RESERVED                           175U */
        /*       RESERVED                           176U */
        /*       RESERVED                           177U */
        /*       RESERVED                           178U */
        /*       RESERVED                           179U */
        /*       RESERVED                           180U */
        /*       RESERVED                           181U */
        /*       RESERVED                           182U */
        /*       RESERVED                           183U */
        /*       RESERVED                           184U */
        /*       RESERVED                           185U */
        /*       RESERVED                           186U */
        /*       RESERVED                           187U */
        /*       RESERVED                           188U */
        /*       RESERVED                           189U */
        /*       RESERVED                           190U */
        /*       RESERVED                           191U */
        XPBR_SERV_EXT_RSTACPU0                    = 192U,
        XPBR_SERV_EXT_RSTACPU1                    = 193U,
        XPBR_SERV_EXT_RSTACPU2                    = 194U,
        XPBR_SERV_EXT_RSTACPU3                    = 195U,
        XPBR_SERV_EXT_RSTAPU                      = 196U,
        /*       RESERVED                           197U */
        XPBR_SERV_EXT_RSTPP0                      = 198U,
        XPBR_SERV_EXT_RSTPP1                      = 199U,
        XPBR_SERV_EXT_RSTGPU                      = 200U,
        XPBR_SERV_EXT_RSTPCIE                     = 201U,
        XPBR_SERV_EXT_RSTSATA                     = 202U,
        /*       RESERVED                           203U */
        XPBR_SERV_EXT_RSTDISPLAYPORT              = 204U,
        /*       RESERVED                           205U */
        /*       RESERVED                           206U */
        /*       RESERVED                           207U */
        XPBR_SERV_EXT_RSTR50                      = 208U,
        XPBR_SERV_EXT_RSTR51                      = 209U,
        XPBR_SERV_EXT_RSTLSRPU                    = 210U,
        /*       RESERVED                           211U */
        XPBR_SERV_EXT_RSTGEM0                     = 212U,
        XPBR_SERV_EXT_RSTGEM1                     = 213U,
        XPBR_SERV_EXT_RSTGEM2                     = 214U,
        XPBR_SERV_EXT_RSTGEM3                     = 215U,
        XPBR_SERV_EXT_RSTUSB0                     = 216U,
        XPBR_SERV_EXT_RSTUSB1                     = 217U,
        /*       RESERVED                           218U */
        XPBR_SERV_EXT_RSTIOU                      = 219U,
        XPBR_SERV_EXT_RSTPSONLY                   = 220U,
        XPBR_SERV_EXT_RSTLPD                      = 221U,
        XPBR_SERV_EXT_RSTFPD                      = 222U,
        XPBR_SERV_EXT_RSTPLD                      = 223U,
        XPBR_SERV_EXT_FW_REQ_0                    = 224U,
        XPBR_SERV_EXT_FW_REQ_1                    = 225U,
        XPBR_SERV_EXT_FW_REQ_2                    = 226U,
        XPBR_SERV_EXT_FW_REQ_3                    = 227U,
        /*       RESERVED                           228U */
        /*       RESERVED                           229U */
        XPBR_SERV_EXT_FW_REQ_4                    = 230U,
        XPBR_SERV_EXT_FW_REQ_5                    = 231U,
        /*       RESERVED                           232U */
        /*       RESERVED                           233U */
        XPBR_SERV_EXT_FW_REQ_6                    = 234U,
        /*       RESERVED                           235U */
        XPBR_SERV_EXT_FW_REQ_7                    = 236U,
        XPBR_SERV_EXT_FW_REQ_8                    = 237U,
        /*       RESERVED                           238U */
        /*       RESERVED                           239U */
        XPBR_SERV_EXT_FW_REQ_9                    = 240U,
        XPBR_SERV_EXT_FW_REQ_10                   = 241U,
        /*       RESERVED                           242U */
        /*       RESERVED                           243U */
        /*       RESERVED                           244U */
        /*       RESERVED                           245U */
        /*       RESERVED                           246U */
        /*       RESERVED                           247U */
        /*       RESERVED                           248U */
        /*       RESERVED                           249U */
        XPBR_SERV_EXT_FPD_SUPPLYENABLE            = 250U,
        XPBR_SERV_EXT_FPD_SUPPLYDISABLE           = 251U,
        XPBR_SERV_EXT_FPD_SUPPLYCHECK             = 252U,
        XPBR_SERV_EXT_PLD_SUPPLYENABLE            = 253U,
        XPBR_SERV_EXT_PLD_SUPPLYDISABLE           = 254U,
        XPBR_SERV_EXT_PLD_SUPPLYCHECK             = 255U,
        XPBR_SERV_EXT_TBL_MAX                     = 256U
};

typedef u32 (*XpbrServHndlr_t) (void);
typedef u32 (*XpbrServExtHndlr_t) (XpbrServHndlr_t RomHandler);

extern const XpbrServHndlr_t XpbrServHndlrTbl[XPBR_SERV_EXT_TBL_MAX];
extern XpbrServExtHndlr_t XpbrServExtTbl[XPBR_SERV_EXT_TBL_MAX];

static inline u32 XpbrACPU0SleepHandler(void)
{
	return XpbrServHndlrTbl[XPBR_SERV_EXT_ACPU0SLEEP]();
}

static inline u32 XpbrACPU0WakeHandler(void)
{
	return IS_IPDISABLED(EFUSE_IPDISABLE_APU0_DIS_MASK) ?
		(u32)XST_FAILURE : XpbrServHndlrTbl[XPBR_SERV_EXT_ACPU0WAKE]();
}

static inline u32 XpbrACPU1SleepHandler(void)

{
	return XpbrServHndlrTbl[XPBR_SERV_EXT_ACPU1SLEEP]();
}

static inline u32 XpbrACPU1WakeHandler(void)
{
	return IS_IPDISABLED(EFUSE_IPDISABLE_APU1_DIS_MASK) ?
		(u32)XST_FAILURE : XpbrServHndlrTbl[XPBR_SERV_EXT_ACPU1WAKE]();
}

static inline u32 XpbrACPU2SleepHandler(void)
{
	return XpbrServHndlrTbl[XPBR_SERV_EXT_ACPU2SLEEP]();
}

static inline u32 XpbrACPU2WakeHandler(void)
{
	return IS_IPDISABLED(EFUSE_IPDISABLE_APU2_DIS_MASK) ?
		(u32)XST_FAILURE : XpbrServHndlrTbl[XPBR_SERV_EXT_ACPU2WAKE]();
}

static inline u32 XpbrACPU3SleepHandler(void)
{
	return XpbrServHndlrTbl[XPBR_SERV_EXT_ACPU3SLEEP]();
}

static inline u32 XpbrACPU3WakeHandler(void)
{
	return IS_IPDISABLED(EFUSE_IPDISABLE_APU3_DIS_MASK) ?
		(u32)XST_FAILURE : XpbrServHndlrTbl[XPBR_SERV_EXT_ACPU3WAKE]();
}

static inline u32 XpbrRstFpdHandler(void)
{
	return XpbrServHndlrTbl[XPBR_SERV_EXT_RSTFPD]();
}

static inline u32 XpbrPwrDnFpdHandler(void)
{
	return XpbrServHndlrTbl[XPBR_SERV_EXT_PWRDNFPD]();
}

static inline u32 XpbrPwrUpFpdHandler(void)
{
	return XpbrServHndlrTbl[XPBR_SERV_EXT_PWRUPFPD]();
}

static inline u32 XpbrPwrDnRpuHandler(void)
{
	return XpbrServHndlrTbl[XPBR_SERV_EXT_PWRDNRPU]();
}

static inline u32 XpbrPwrUpRpuHandler(void)
{
	return XpbrServHndlrTbl[XPBR_SERV_EXT_PWRUPRPU]();
}

static inline u32 XpbrRstR50Handler(void)
{
	return XpbrServHndlrTbl[XPBR_SERV_EXT_RSTR50]();
}

static inline u32 XpbrRstR51Handler(void)
{
	return XpbrServHndlrTbl[XPBR_SERV_EXT_RSTR51]();
}

static inline u32 XpbrPwrDnOcmBank0Handler(void)
{
	return XpbrServHndlrTbl[XPBR_SERV_EXT_PWRDNOCMBANK0]();
}

static inline u32 XpbrPwrDnOcmBank1Handler(void)
{
	return XpbrServHndlrTbl[XPBR_SERV_EXT_PWRDNOCMBANK1]();
}

static inline u32 XpbrPwrDnOcmBank2Handler(void)
{
	return XpbrServHndlrTbl[XPBR_SERV_EXT_PWRDNOCMBANK2]();
}

static inline u32 XpbrPwrDnOcmBank3Handler(void)
{
	return XpbrServHndlrTbl[XPBR_SERV_EXT_PWRDNOCMBANK3]();
}

static inline u32 XpbrPwrUpOcmBank0Handler(void)
{
	return XpbrServHndlrTbl[XPBR_SERV_EXT_PWRUPOCMBANK0]();
}

static inline u32 XpbrPwrUpOcmBank1Handler(void)
{
	return XpbrServHndlrTbl[XPBR_SERV_EXT_PWRUPOCMBANK1]();
}

static inline u32 XpbrPwrUpOcmBank2Handler(void)
{
	return XpbrServHndlrTbl[XPBR_SERV_EXT_PWRUPOCMBANK2]();
}

static inline u32 XpbrPwrUpOcmBank3Handler(void)
{
	return XpbrServHndlrTbl[XPBR_SERV_EXT_PWRUPOCMBANK3]();
}

static inline u32 XpbrPwrDnTcm0AHandler(void)
{
	return XpbrServHndlrTbl[XPBR_SERV_EXT_PWRDNTCM0A]();
}

static inline u32 XpbrPwrDnTcm0BHandler(void)
{
	return XpbrServHndlrTbl[XPBR_SERV_EXT_PWRDNTCM0B]();
}

static inline u32 XpbrPwrDnTcm1AHandler(void)
{
	return XpbrServHndlrTbl[XPBR_SERV_EXT_PWRDNTCM1A]();
}

static inline u32 XpbrPwrDnTcm1BHandler(void)
{
	return XpbrServHndlrTbl[XPBR_SERV_EXT_PWRDNTCM1B]();
}

static inline u32 XpbrPwrUpTcm0AHandler(void)
{
	return XpbrServHndlrTbl[XPBR_SERV_EXT_PWRUPTCM0A]();
}

static inline u32 XpbrPwrUpTcm0BHandler(void)
{
	return XpbrServHndlrTbl[XPBR_SERV_EXT_PWRUPTCM0B]();
}

static inline u32 XpbrPwrUpTcm1AHandler(void)
{
	return XpbrServHndlrTbl[XPBR_SERV_EXT_PWRUPTCM1A]();
}

static inline u32 XpbrPwrUpTcm1BHandler(void)
{
	return XpbrServHndlrTbl[XPBR_SERV_EXT_PWRUPTCM1B]();
}

static inline u32 XpbrPwrDnL2Bank0Handler(void)
{
	return XpbrServHndlrTbl[XPBR_SERV_EXT_PWRDNL2BANK0]();
}

static inline u32 XpbrPwrUpL2Bank0Handler(void)
{
	return XpbrServHndlrTbl[XPBR_SERV_EXT_PWRUPL2BANK0]();
}

static inline u32 XpbrPwrDnUsb0Handler(void)
{
	return XpbrServHndlrTbl[XPBR_SERV_EXT_PWRDNUSB0]();
}

static inline u32 XpbrPwrDnUsb1Handler(void)
{
	return XpbrServHndlrTbl[XPBR_SERV_EXT_PWRDNUSB1]();
}

static inline u32 XpbrPwrUpUsb0Handler(void)
{
	return XpbrServHndlrTbl[XPBR_SERV_EXT_PWRUPUSB0]();
}

static inline u32 XpbrPwrUpUsb1Handler(void)
{
	return XpbrServHndlrTbl[XPBR_SERV_EXT_PWRUPUSB1]();
}

static inline u32 XpbrPwrDnPp0Handler(void)
{
	return XpbrServHndlrTbl[XPBR_SERV_EXT_PWRDNPP0]();
}

static inline u32 XpbrPwrDnPp1Handler(void)
{
	return XpbrServHndlrTbl[XPBR_SERV_EXT_PWRDNPP1]();
}

static inline u32 XpbrPwrUpPp0Handler(void)
{
	return IS_IPDISABLED(EFUSE_IPDISABLE_GPU_DIS_MASK) ?
		(u32)XST_FAILURE : XpbrServHndlrTbl[XPBR_SERV_EXT_PWRUPPP0]();
}

static inline u32 XpbrPwrUpPp1Handler(void)
{
	return IS_IPDISABLED(EFUSE_IPDISABLE_GPU_DIS_MASK) ?
		(u32)XST_FAILURE : XpbrServHndlrTbl[XPBR_SERV_EXT_PWRUPPP1]();
}

static inline u32 XpbrRstACPU0Handler(void)
{
	return XpbrServHndlrTbl[XPBR_SERV_EXT_RSTACPU0]();
}

static inline u32 XpbrRstACPU1Handler(void)
{
	return XpbrServHndlrTbl[XPBR_SERV_EXT_RSTACPU1]();
}

static inline u32 XpbrRstACPU2Handler(void)
{
	return XpbrServHndlrTbl[XPBR_SERV_EXT_RSTACPU2]();
}

static inline u32 XpbrRstACPU3Handler(void)
{
	return XpbrServHndlrTbl[XPBR_SERV_EXT_RSTACPU3]();
}

static inline u32 XpbrRstACPU0CPHandler(void)
{
	return XpbrServHndlrTbl[XPBR_SERV_EXT_RSTACPU0]();
}

static inline u32 XpbrRstACPU1CPHandler(void)
{
	return XpbrServHndlrTbl[XPBR_SERV_EXT_RSTACPU1]();
}

static inline u32 XpbrRstACPU2CPHandler(void)
{
	return XpbrServHndlrTbl[XPBR_SERV_EXT_RSTACPU2]();
}

static inline u32 XpbrRstACPU3CPHandler(void)
{
	return XpbrServHndlrTbl[XPBR_SERV_EXT_RSTACPU3]();
}

static inline u32 XpbrRstPp0Handler(void)
{
	return XpbrServHndlrTbl[XPBR_SERV_EXT_RSTPP0]();
}

static inline u32 XpbrRstPp1Handler(void)
{
	return XpbrServHndlrTbl[XPBR_SERV_EXT_RSTPP1]();
}

static inline u32 XpbrRstGpuHandler(void)
{
	return XpbrServHndlrTbl[XPBR_SERV_EXT_RSTGPU]();
}

static inline u32 XpbrRstPCIeHandler(void)
{
	return XpbrServHndlrTbl[XPBR_SERV_EXT_RSTPCIE]();
}

static inline u32 XpbrRstSataHandler(void)
{
	return XpbrServHndlrTbl[XPBR_SERV_EXT_RSTSATA]();
}

static inline u32 XpbrRstDisplayPortHandler(void)
{
	return XpbrServHndlrTbl[XPBR_SERV_EXT_RSTDISPLAYPORT]();
}

static inline u32 XpbrRstGem0Handler(void)
{
	return XpbrServHndlrTbl[XPBR_SERV_EXT_RSTGEM0]();
}

static inline u32 XpbrRstGem1Handler(void)
{
	return XpbrServHndlrTbl[XPBR_SERV_EXT_RSTGEM1]();
}

static inline u32 XpbrRstGem2Handler(void)
{
	return XpbrServHndlrTbl[XPBR_SERV_EXT_RSTGEM2]();
}

static inline u32 XpbrRstGem3Handler(void)
{
	return XpbrServHndlrTbl[XPBR_SERV_EXT_RSTGEM3]();
}

static inline u32 XpbrRstUsb0Handler(void)
{
	return XpbrServHndlrTbl[XPBR_SERV_EXT_RSTUSB0]();
}

static inline u32 XpbrRstUsb1Handler(void)
{
	return XpbrServHndlrTbl[XPBR_SERV_EXT_RSTUSB1]();
}

static inline u32 XpbrPwrDnPldHandler(void)
{
	return XpbrServHndlrTbl[XPBR_SERV_EXT_PWRDNPLD]();
}

static inline u32 XpbrPwrUpPldHandler(void)
{
	return XpbrServHndlrTbl[XPBR_SERV_EXT_PWRUPPLD]();
}

#ifdef __cplusplus
}
#endif

#endif /* XPFW_ROM_INTERFACE_H_ */
