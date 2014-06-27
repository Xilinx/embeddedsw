;******************************************************************************
;
; Copyright (C) 2009 - 2014 Xilinx, Inc.  All rights reserved.
;
; Permission is hereby granted, free of charge, to any person obtaining a copy
; of this software and associated documentation files (the "Software"), to deal
; in the Software without restriction, including without limitation the rights
; to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
; copies of the Software, and to permit persons to whom the Software is
; furnished to do so, subject to the following conditions:
;
; The above copyright notice and this permission notice shall be included in
; all copies or substantial portions of the Software.
;
; Use of the Software is limited solely to applications:
; (a) running on a Xilinx device, or
; (b) that interact with a Xilinx device through a bus or interconnect.
;
; THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
; IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
; FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
; XILINX CONSORTIUM BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
; WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
; OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
; SOFTWARE.
;
; Except as contained in this notice, the name of the Xilinx shall not be used
; in advertising or otherwise to promote the sale, use or other dealings in
; this Software without prior written authorization from Xilinx.
;
;*****************************************************************************
;****************************************************************************
;**
; @file translation_table.s
;
; This file contains the initialization for the MMU table in RAM
; needed by the Cortex A9 processor
;
; <pre>
; MODIFICATION HISTORY:
;
; Ver   Who  Date     Changes
; ----- ---- -------- ---------------------------------------------------
; 1.00a ecm  10/20/09 Initial version
; 3.07a sgd  07/05/2012 Configuring device address spaces as shareable device
;		       instead of strongly-ordered.
; </pre>
;
; @note
;
; None.
;
;****************************************************************************
	EXPORT  MMUTable

;ARMCC	AREA |.mmu_tbl|,CODE,ALIGN=14
;        RSEG mmu_tbl:CODE:ROOT (14)
	SECTION .mmu_tbl:CODE:ROOT(14)

MMUTable
	; Each table entry occupies one 32-bit word and there are
	; 4096 entries, so the entire table takes up 16KB.
	; Each entry covers a 1MB section.


; 0x00000000 - 0x3ffffff (DDR Cacheable) 
count   SETA  0
sect    SETA  0
  REPT  0x400
  DCD	sect + 0x15de6		; S=1, TEX=b101 AP=b11, Domain=b1111, C=b0, B=b1 
sect    SETA  sect+0x100000
count   SETA  count+1
  ENDR

; 0x40000000 - 0x7fffffff (GpAxi0)
count   SETA  0
   REPT 0x400
   DCD	sect + 0xc06		; S=0, TEX=b010 AP=b11, Domain=b0, C=b0, B=b0 
sect    SETA  sect+0x100000
count   SETA  count+1
   ENDR

; 0x80000000 - 0xbfffffff (GpAxi1)
count   SETA  0
   REPT 0x400
   DCD	sect + 0xc06		; S=0, TEX=b010 AP=b11, Domain=b0, C=b0, B=b0 
sect    SETA  sect+0x100000
count   SETA  count+1
   ENDR

; 0xc0000000 - 0xdfffffff (undef)
count   SETA  0
   REPT 0x200
   DCD	sect 			; S=0, TEX=b000 AP=b00, Domain=b0, C=b0, B=b0 
sect    SETA  sect+0x100000
count   SETA  count+1
   ENDR

; 0xe0000000 - 0xefffffff (IOP dev)
count   SETA  0
   REPT 0x100
   DCD	sect + 0xc06		; S=0, TEX=b010 AP=b11, Domain=b0, C=b0, B=b0 
sect    SETA  sect+0x100000
count   SETA  count+1
   ENDR

; 0xf0000000 - 0xf7ffffff (reserved)
count   SETA  0
   REPT 0x80
   DCD	sect 			; S=0, TEX=b000 AP=b00, Domain=b0, C=b0, B=b0 
sect    SETA  sect+0x100000
count   SETA  count+1
   ENDR

; 0xf8000000 - 0xf9ffffff (APB device regs)
count   SETA  0
   REPT 0x20
   DCD	sect + 0xc06		; S=0, TEX=b010 AP=b11, Domain=b0, C=b0, B=b0 
sect    SETA  sect+0x100000
count   SETA  count+1
   ENDR

; 0xfa000000 - 0xfbffffff (reserved)
count   SETA  0
   REPT 0x20
   DCD	sect 			; S=0, TEX=b000 AP=b00, Domain=b0, C=b0, B=b0 
sect    SETA  sect+0x100000
count   SETA  count+1
   ENDR

; 0xfc000000 - 0xfffffff (OCM/QSPI) 
count   SETA  0
   REPT 0x40
   DCD	sect + 0x15de6		; S=1, TEX=b101 AP=b11, Domain=b1111, C=b0, B=b1 
sect    SETA  sect+0x100000
count   SETA  count+1
   ENDR

   END
