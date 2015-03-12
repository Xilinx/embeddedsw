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
* Use of the Software is limited solely to applications:
* (a) running on a Xilinx device, or
* (b) that interact with a Xilinx device through a bus or interconnect.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* XILINX CONSORTIUM BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
* WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
* OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*
* Except as contained in this notice, the name of the Xilinx shall not be used
* in advertising or otherwise to promote the sale, use or other dealings in
* this Software without prior written authorization from Xilinx.
*
******************************************************************************/

_STACK_SIZE = DEFINED(_STACK_SIZE) ? _STACK_SIZE : 0x1000;

/* Define Memories in the system */

MEMORY
{
	/* This setting considers entire RAM. ROM entities are not considered */
	PMU_RAM : ORIGIN = 0xFFDC0000, LENGTH = 0x0001FF00 /* 128KB - _PERSISTENT256 */
}

/* Specify the default entry point to the program */

ENTRY(XPfw_Init)

/* Define the sections, and where they are mapped in memory */

SECTIONS
{
.vectors.reset 0xFFDC0000 : {
   KEEP (*(.vectors.reset))
} > PMU_RAM

.vectors.sw_exception 0xFFDC0008 : {
   KEEP (*(.vectors.sw_exception))
 } > PMU_RAM

.vectors.interrupt 0xFFDC0010 : {
   KEEP (*(.vectors.interrupt))
} > PMU_RAM

.vectors.hw_exception 0xFFDC0020 : {
   KEEP (*(.vectors.hw_exception))
   _MB_RESERVED_START = .;
} > PMU_RAM

.text 0xFFDC0050: {

   *(.text)
   *(.text.*)
   *(.gnu.linkonce.t.*)
} > PMU_RAM

.rodata : {
   __rodata_start = .;
   *(.rodata)
   *(.rodata.*)
   *(.gnu.linkonce.r.*)
   __rodata_end = .;
} > PMU_RAM

.data : {
   . = ALIGN(4);
   __data_start = .;
   *(.data)
   *(.data.*)
   *(.gnu.linkonce.d.*)
   __data_end = .;
} > PMU_RAM

.sdata2 : {
   . = ALIGN(8);
   __sdata2_start = .;
   *(.sdata2)
   *(.sdata2.*)
   *(.gnu.linkonce.s2.*)
   . = ALIGN(8);
   __sdata2_end = .;
} > PMU_RAM

.sbss2 : {
   __sbss2_start = .;
   *(.sbss2)
   *(.sbss2.*)
   *(.gnu.linkonce.sb2.*)
   __sbss2_end = .;
} > PMU_RAM

.sdata : {
   . = ALIGN(8);
   __sdata_start = .;
   *(.sdata)
   *(.sdata.*)
   *(.gnu.linkonce.s.*)
   __sdata_end = .;
} > PMU_RAM

.sbss (NOLOAD) : {
   . = ALIGN(4);
   __sbss_start = .;
   *(.sbss)
   *(.sbss.*)
   *(.gnu.linkonce.sb.*)
   . = ALIGN(8);
   __sbss_end = .;
} > PMU_RAM

.tdata : {
   __tdata_start = .;
   *(.tdata)
   *(.tdata.*)
   *(.gnu.linkonce.td.*)
   __tdata_end = .;
} > PMU_RAM

.tbss : {
   __tbss_start = .;
   *(.tbss)
   *(.tbss.*)
   *(.gnu.linkonce.tb.*)
   __tbss_end = .;
} > PMU_RAM

.bss (NOLOAD) : {
   . = ALIGN(4);
   __bss_start = .;
   *(.bss)
   *(.bss.*)
   *(.gnu.linkonce.b.*)
   *(COMMON)
   . = ALIGN(4);
   __bss_end = .;
} > PMU_RAM

_SDA_BASE_ = __sdata_start + ((__sbss_end - __sdata_start) / 2 );

_SDA2_BASE_ = __sdata2_start + ((__sbss2_end - __sdata2_start) / 2 );

/* Generate Stack definitions */

.stack (NOLOAD) : {
   _stack_end = .;
   . += _STACK_SIZE;
   . = ALIGN(8);
   _stack = .;
   __stack = _stack;
} > PMU_RAM

_end = .;
}

XpbrACPU0SleepHandler = 0xffd00f98;
XpbrACPU0WakeHandler = 0xffd00d4c;
XpbrACPU1SleepHandler = 0xffd00fbc;
XpbrACPU1WakeHandler = 0xffd00d70;
XpbrACPU2SleepHandler = 0xffd00fe0;
XpbrACPU2WakeHandler = 0xffd00d94;
XpbrACPU3SleepHandler = 0xffd01004;
XpbrACPU3WakeHandler = 0xffd00db8;
XpbrCsuSecureLockdownHandler = 0xffd01e60;
XpbrDapFpdWakeHandler = 0xffd01c70;
XpbrDapRpuWakeHandler = 0xffd01cd8;
XpbrDefaultWakeHandler = 0xffd036d0;
XpbrFpdGicProxyWakeHandler = 0xffd00f04;
XpbrFPIsolationReqHandler = 0xffd01518;
XpbrFPLockIsoHandler = 0xffd0156c;
XpbrGpi1Router = 0xffd038ac;
XpbrGpi2Router = 0xffd03960;
XpbrGpi3Handler = 0xffd00d30;
XpbrHwExceptionHandler = 0xffd03d6c;
XpbrHwRstReqHandler = 0xffd01be0;
XpbrIpi0Handler = 0xffd00cf4;
XpbrIsoReqRouter = 0xffd03b94;
XpbrMio0WakeHandler = 0xffd00e5c;
XpbrMio1WakeHandler = 0xffd00e78;
XpbrMio2WakeHandler = 0xffd00e94;
XpbrMio3WakeHandler = 0xffd00eb0;
XpbrMio4WakeHandler = 0xffd00ecc;
XpbrMio5WakeHandler = 0xffd00ee8;
XpbrNullInterruptHandler = 0xffd03d8c;
XpbrNullServiceHandler = 0xffd020e0;
XpbrPLIsolationReqHandler = 0xffd01534;
XpbrPLNonPCAPIsoReqHandler = 0xffd01550;
XpbrPwrDnACPU0Handler = 0xffd0158c;
XpbrPwrDnACPU1Handler = 0xffd015b0;
XpbrPwrDnACPU2Handler = 0xffd015d4;
XpbrPwrDnACPU3Handler = 0xffd015f8;
XpbrPwrDnFpdHandler = 0xffd01814;
XpbrPwrDnGpuHandler = 0xffd0184c;
XpbrPwrDnL2Bank0Handler = 0xffd01664;
XpbrPwrDnOcmBank0Handler = 0xffd0173c;
XpbrPwrDnOcmBank1Handler = 0xffd01760;
XpbrPwrDnOcmBank2Handler = 0xffd01784;
XpbrPwrDnOcmBank3Handler = 0xffd017a8;
XpbrPwrDnPldHandler = 0xffd01830;
XpbrPwrDnPp0Handler = 0xffd0161c;
XpbrPwrDnPp1Handler = 0xffd01640;
XpbrPwrDnR50Handler = 0xffd01878;
XpbrPwrDnR51Handler = 0xffd01894;
XpbrPwrDnReqRouter = 0xffd03ad4;
XpbrPwrDnRpuHandler = 0xffd01688;
XpbrPwrDnTcm0AHandler = 0xffd016ac;
XpbrPwrDnTcm0BHandler = 0xffd016d0;
XpbrPwrDnTcm1AHandler = 0xffd016f4;
XpbrPwrDnTcm1BHandler = 0xffd01718;
XpbrPwrDnUsb0Handler = 0xffd017cc;
XpbrPwrDnUsb1Handler = 0xffd017f0;
XpbrPwrUpACPU0Handler = 0xffd018b0;
XpbrPwrUpACPU1Handler = 0xffd018d4;
XpbrPwrUpACPU2Handler = 0xffd018f8;
XpbrPwrUpACPU3Handler = 0xffd0191c;
XpbrPwrUpFpdHandler = 0xffd01b38;
XpbrPwrUpL2Bank0Handler = 0xffd01988;
XpbrPwrUpOcmBank0Handler = 0xffd01a60;
XpbrPwrUpOcmBank1Handler = 0xffd01a84;
XpbrPwrUpOcmBank2Handler = 0xffd01aa8;
XpbrPwrUpOcmBank3Handler = 0xffd01acc;
XpbrPwrUpPldHandler = 0xffd01b54;
XpbrPwrUpPp0Handler = 0xffd01940;
XpbrPwrUpPp1Handler = 0xffd01964;
XpbrPwrUpReqRouter = 0xffd03a14;
XpbrPwrUpRpuHandler = 0xffd019ac;
XpbrPwrUpTcm0AHandler = 0xffd019d0;
XpbrPwrUpTcm0BHandler = 0xffd019f4;
XpbrPwrUpTcm1AHandler = 0xffd01a18;
XpbrPwrUpTcm1BHandler = 0xffd01a3c;
XpbrPwrUpUsb0Handler = 0xffd01af0;
XpbrPwrUpUsb1Handler = 0xffd01b14;
XpbrR50SleepHandler = 0xffd01028;
XpbrR50WakeHandler = 0xffd00ddc;
XpbrR51SleepHandler = 0xffd0104c;
XpbrR51WakeHandler = 0xffd00e00;
XpbrROMExceptionHandler = 0xffd03d14;
XpbrRstACPU0Handler = 0xffd01124;
XpbrRstACPU1Handler = 0xffd01158;
XpbrRstACPU2Handler = 0xffd0118c;
XpbrRstACPU3Handler = 0xffd011c0;
XpbrRstACPUxHandler = 0xffd01070;
XpbrRstApuHandler = 0xffd01d48;
XpbrRstDisplayPortHandler = 0xffd0131c;
XpbrRstFpdHandler = 0xffd014e8;
XpbrRstGem0Handler = 0xffd013b8;
XpbrRstGem1Handler = 0xffd013e4;
XpbrRstGem2Handler = 0xffd01410;
XpbrRstGem3Handler = 0xffd0143c;
XpbrRstGpuHandler = 0xffd011f4;
XpbrRstLsRpuHandler = 0xffd01dac;
XpbrRstPCIeHandler = 0xffd012b4;
XpbrRstPp0Handler = 0xffd0123c;
XpbrRstPp1Handler = 0xffd01278;
XpbrRstPsOnlyHandler = 0xffd014c0;
XpbrRstR50Handler = 0xffd01350;
XpbrRstR51Handler = 0xffd01384;
XpbrRstSataHandler = 0xffd012e8;
XpbrRstUsb0Handler = 0xffd01468;
XpbrRstUsb1Handler = 0xffd01494;
XpbrRtcAlarmHandler = 0xffd00d14;
XpbrServiceHandler = 0xffd03dfc;
XpbrServiceMode = 0xffd01b70;
XpbrSwExceptionHandler = 0xffd03d7c;
XpbrSwRstReqRouter = 0xffd03c54;
XpbrTMRFaultDetectionHandler = 0xffd00cec;
XpbrUsb0WakeHandler = 0xffd00e24;
XpbrUsb1WakeHandler = 0xffd00e40;
