/******************************************************************************
*
* Copyright (C) 2002 - 2014 Xilinx, Inc. All rights reserved.
*
* This file contains confidential and proprietary information  of Xilinx, Inc.
* and is protected under U.S. and  international copyright and other
* intellectual property  laws.
*
* DISCLAIMER
* This disclaimer is not a license and does not grant any  rights to the
* materials distributed herewith. Except as  otherwise provided in a valid
* license issued to you by  Xilinx, and to the maximum extent permitted by
* applicable law:
* (1) THESE MATERIALS ARE MADE AVAILABLE "AS IS" AND  WITH ALL FAULTS, AND
* XILINX HEREBY DISCLAIMS ALL WARRANTIES  AND CONDITIONS, EXPRESS, IMPLIED,
* OR STATUTORY, INCLUDING  BUT NOT LIMITED TO WARRANTIES OF MERCHANTABILITY,
* NON-INFRINGEMENT, OR FITNESS FOR ANY PARTICULAR PURPOSE
* and
* (2) Xilinx shall not be liable (whether in contract or tort,  including
* negligence, or under any other theory of liability) for any loss or damage of
* any kind or nature  related to, arising under or in connection with these
* materials, including for any direct, or any indirect,  special, incidental,
* or consequential loss or damage  (including loss of data, profits, goodwill,
* or any type of  loss or damage suffered as a result of any action brought
* by a third party) even if such damage or loss was  reasonably foreseeable
* or Xilinx had been advised of the  possibility of the same.
*
* CRITICAL APPLICATIONS
* Xilinx products are not designed or intended to be fail-safe, or for use in
* any application requiring fail-safe  performance, such as life-support or
* safety devices or  systems, Class III medical devices, nuclear facilities,
* applications related to the deployment of airbags, or any  other applications
* that could lead to death, personal  injury, or severe property or environmental
* damage  (individually and collectively, "Critical  Applications").
* Customer assumes the sole risk and liability of any use of Xilinx products in
* Critical  Applications, subject only to applicable laws and  regulations
* governing limitations on product liability.
*
* THIS COPYRIGHT NOTICE AND DISCLAIMER MUST BE RETAINED AS PART OF THIS FILE
* AT ALL TIMES.
*
******************************************************************************/

#include "profile.h"
#include "_profile_timer_hw.h"
#ifdef PROC_MICROBLAZE
#include "mblaze_nt_types.h"
#endif

/*
 * The mcount fucntion is excluded from the library, if the user defines
 * PROFILE_NO_GRAPH.
 */
#ifndef PROFILE_NO_GRAPH

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef PROFILE_NO_FUNCPTR
s32 searchpc(const struct fromto_struct *cgtable, s32 cgtable_size, u32 frompc );
#else
s32 searchpc(const struct fromstruct *froms, s32 fromssize, u32 frompc );
#endif

/*extern struct gmonparam *_gmonparam, */

#ifdef PROFILE_NO_FUNCPTR
s32 searchpc(const struct fromto_struct *cgtable, s32 cgtable_size, u32 frompc )
{
	s32 index = 0 ;

	while( (index < cgtable_size) && (cgtable[index].frompc != frompc) ){
		index++ ;
	}
	if( index == cgtable_size ) {
		return -1 ;
	} else {
		return index ;
	}
}
#else
s32 searchpc(const struct fromstruct *froms, s32 fromssize, u32 frompc )
{
	s32 index = 0 ;
	s32 Status;

	while( (index < fromssize) && (froms[index].frompc != frompc) ){
		index++ ;
	}
	if( index == fromssize ) {
		Status = -1 ;
	} else {
		Status = index ;
	}
	return Status;
}
#endif		/* PROFILE_NO_FUNCPTR */


void mcount( u32 frompc, u32 selfpc )
{
	register struct gmonparam *p = NULL;
	register s32 toindex, fromindex;
	s32 j;

	disable_timer();

	/*print("CG: "), putnum(frompc), print("->"), putnum(selfpc), print("\r\n") ,
	 * check that frompcindex is a reasonable pc value.
	 * for example:	signal catchers get called from the stack,
	 *		not from text space.  too bad.
	*/
	for(j = 0; j < n_gmon_sections; j++ ){
		if((frompc >= _gmonparam[j].lowpc) && (frompc < _gmonparam[j].highpc)) {
			p = &_gmonparam[j];
			break;
		}
	}
	if( j == n_gmon_sections ) {
		goto done;
	}

#ifdef PROFILE_NO_FUNCPTR
	fromindex = searchpc( p->cgtable, p->cgtable_size, frompc ) ;
	if( fromindex == -1 ) {
		fromindex = p->cgtable_size ;
		p->cgtable_size++ ;
		p->cgtable[fromindex].frompc = frompc ;
		p->cgtable[fromindex].selfpc = selfpc ;
		p->cgtable[fromindex].count = 1 ;
		goto done ;
	}
	p->cgtable[fromindex].count++ ;
#else
	fromindex = (s32)searchpc( p->froms, ((s32)p->fromssize), frompc ) ;
	if( fromindex == -1 ) {
		fromindex = (s32)p->fromssize ;
		p->fromssize++ ;
		/*if( fromindex >= N_FROMS ) {
		* print("Error : From PC table overflow\r\n")
		* goto overflow
		*}*/
		p->froms[fromindex].frompc = frompc ;
		p->froms[fromindex].link = -1 ;
	}else {
		toindex = ((s32)(p->froms[fromindex].link));
		while(toindex != -1) {
			toindex = (((s32)p->tossize) - toindex)-1 ;
			if( p->tos[toindex].selfpc == selfpc ) {
				p->tos[toindex].count++ ;
				goto done ;
			}
			toindex = ((s32)(p->tos[toindex].link)) ;
		}
	}

	/*if( toindex == -1 ) { */
	p->tos-- ;
	p->tossize++ ;
	/* if( toindex >= N_TOS ) {
	* print("Error : To PC table overflow\r\n")
	* goto overflow
	*} */
	p->tos[0].selfpc = selfpc ;
	p->tos[0].count = 1 ;
	p->tos[0].link = p->froms[fromindex].link ;
	p->froms[fromindex].link = ((s32)(p->tossize))-((s32)1);
#endif

 done:
	p->state = GMON_PROF_ON;
	goto enable_timer_label ;
 /* overflow: */
	/*p->state = GMON_PROF_ERROR */
 enable_timer_label:
	enable_timer();
	return ;
}


#endif		/* PROFILE_NO_GRAPH */

