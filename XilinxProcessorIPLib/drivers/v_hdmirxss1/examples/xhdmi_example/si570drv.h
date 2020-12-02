/*
 * si570_cmds.h
 *
 *  Created on: Nov 26, 2013
 *      Author: mcgett
 */

#include "xil_types.h"
#include "xparameters.h"
#if defined (XPS_BOARD_ZCU102) || \
	defined (XPS_BOARD_ZCU104) || \
	defined (XPS_BOARD_ZCU106)
#include "xiicps.h"
#else
#include "xiic.h"
#endif

#ifndef SI570_CMDS_H_
#define SI570_CMDS_H_

#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_YELLOW  "\x1b[33m"
#define ANSI_COLOR_RESET   "\x1b[0m"

#define two_to_37 (double) 137438953472.0
#define two_to_32 (double)   4294967296.0
#define two_to_28 (double)    268435456.0
#define two_to_24 (double)     16777216.0
#define two_to_16 (double)        65536.0
#define two_to_8  (double)          256.0

#define SI570_KCU105 0
#define SI570_XM101_0_HPC 1
#define SI570_XM101_1_HPC 2
#define SI570_XM104_0_HPC 3
#define SI570_XM105_0_HPC 4
#define SI570_XM107_0_HPC 5
#define SI570_XM101_0_LPC 6
#define SI570_XM101_1_LPC 7
#define SI570_XM104_0_LPC 8
#define SI570_XM105_0_LPC 9
#define SI570_XM107_0_LPC 10

void Si570_SetFreq(void *IicPtr, u8 Si_Addr, double Freq);
void Si570_ReadCal(void *IicPtr, u8 Addr, u8 *RFreq_Cal,
                        u8 *HSDIV_Cal, u8 *N1_Cal);
void Si570_XtalFreqCalc(double *XtalFreq, u8 *RFreq_Cal,
                        u8 *HSDIV_Set, u8 *N1_Set);
void Si570_RfreqCalc(double Freq, u8 *RFreq_Cal, u8 *RFreq_Set,
                        u8 *HSDIV_Set, u8 *N1_Set, double *XtalFreq);
void Si570_WriteRfreq(void *IicPtr, u8 Addr, u8 *RFreq_Set,
                        u8 HSDIV_Set, u8 N1_Set);

#endif /* SI570_CMDS_H_ */
