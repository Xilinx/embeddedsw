#include "defines.h"
#include "ddr_phy.h"
#include "ddrc.h"
#include "memtest.h"
#include <stdlib.h>
#include <stdio.h>
#include <xil_printf.h>
#include "xil_exception.h"
#include "xpseudo_asm.h"
#include "xdebug.h"

#define LCDLR0_BASE		DDR_PHY_DX0LCDLR0
#define LCDLR1_BASE		DDR_PHY_DX0LCDLR1
#define GTR0_BASE		DDR_PHY_DX0GTR0
#define MDLR0_BASE		DDR_PHY_DX0MDLR0
#define LANE_OFFSET	0x100

#define WDQSL_MASK	0xF8FFFFFF
#define IPRD_MASK		0xFFFFFE00
#define TPRD_MASK		0xFE00FFFF
#define WLSL_MASK		0xFFF0FFFF
#define WLD_MASK		0xFFFFFE00	
#define DGSL_MASK		0xFFFFFFE0


#define CHECK_BIT(var,pos) ((var) & (1<<(pos)))

unsigned int get_wr_wdqd(unsigned int addr) {
	return ((*(volatile unsigned int*) (UINTPTR)addr) & DDR_PHY_DX0LCDLR1_WDQD_MASK ) >> (DDR_PHY_DX0LCDLR1_WDQD_SHIFT);
}

void set_wr_wdqd(unsigned int addr, unsigned int val) {
	*(volatile unsigned int*) (UINTPTR)addr = val;
	asm("dsb sy");
}

unsigned int get_wr_wdqsl(unsigned int addr) {
	return ((*(volatile unsigned int*) (UINTPTR)addr) & DDR_PHY_DX0GTR0_WDQSL_MASK ) >> (DDR_PHY_DX0GTR0_WDQSL_SHIFT);
}

void set_wr_wdqsl(unsigned int addr, unsigned int val) {
	unsigned int rdval=0, wrval=0;

	rdval = *(volatile unsigned int*) (UINTPTR)addr;
	rdval = rdval & WDQSL_MASK;
	wrval =  rdval | (val << DDR_PHY_DX0GTR0_WDQSL_SHIFT);
	*(volatile unsigned int*) (UINTPTR)addr = wrval;
	asm("dsb sy");
}

unsigned int get_wr_iprd(unsigned int addr) {
	return ((*(volatile unsigned int*) (UINTPTR)addr) & DDR_PHY_DX0MDLR0_IPRD_MASK ) >> (DDR_PHY_DX0MDLR0_IPRD_SHIFT);
}

void set_wr_iprd(unsigned int addr, unsigned int val) {
	unsigned int rdval=0, wrval=0;

	rdval = *(volatile unsigned int*) (UINTPTR)addr;
	rdval = rdval & IPRD_MASK;
	wrval =  wrval | (val << DDR_PHY_DX0MDLR0_IPRD_SHIFT);
	*(volatile unsigned int*) (UINTPTR)addr = wrval;
	asm("dsb sy");
}

unsigned int get_wr_tprd(unsigned int addr) {
	return ((*(volatile unsigned int*) (UINTPTR)addr) & DDR_PHY_DX0MDLR0_TPRD_MASK ) >> (DDR_PHY_DX0MDLR0_TPRD_SHIFT);
}

void set_wr_tprd(unsigned int addr, unsigned int val) {
	unsigned int rdval=0, wrval=0;

	rdval = *(volatile unsigned int*) (UINTPTR)addr;
	rdval = rdval & TPRD_MASK;
	wrval =  wrval | (val << DDR_PHY_DX0MDLR0_TPRD_SHIFT);
	*(volatile unsigned int*) (UINTPTR)addr = wrval;
	asm("dsb sy");
}

unsigned int get_wr_wlsl(unsigned int addr) {
	return ((*(volatile unsigned int*) (UINTPTR)addr) & DDR_PHY_DX0GTR0_WLSL_MASK ) >> (DDR_PHY_DX0GTR0_WLSL_SHIFT);
}

void set_wr_wlsl(unsigned int addr, unsigned int val) {
	unsigned int rdval=0, wrval=0;

	rdval = *(volatile unsigned int*) (UINTPTR)addr;
	rdval = rdval & WLSL_MASK;
	wrval =  wrval | (val << DDR_PHY_DX0GTR0_WLSL_SHIFT);
	*(volatile unsigned int*) (UINTPTR)addr = wrval;
	asm("dsb sy");
}

unsigned int get_wr_wld(unsigned int addr) {
	return ((*(volatile unsigned int*) (UINTPTR)addr) & DDR_PHY_DX0LCDLR0_WLD_MASK ) >> (DDR_PHY_DX0LCDLR0_WLD_SHIFT);
}

void set_wr_wld(unsigned int addr, unsigned int val) {
	unsigned int rdval=0, wrval=0;

	rdval = *(volatile unsigned int*) (UINTPTR)addr;
	rdval = rdval & WLD_MASK;
	wrval =  wrval | (val << DDR_PHY_DX0LCDLR0_WLD_SHIFT);
	*(volatile unsigned int*) (UINTPTR)addr = wrval;
	asm("dsb sy");
}


unsigned int get_wr_dgsl(unsigned int addr) {
	return ((*(volatile unsigned int*) (UINTPTR)addr) & DDR_PHY_DX0GTR0_DGSL_MASK ) >> (DDR_PHY_DX0GTR0_DGSL_SHIFT);
}

void set_wr_dgsl(unsigned int addr, unsigned int val) {
	unsigned int rdval=0, wrval=0;

	rdval = *(volatile unsigned int*) (UINTPTR)addr;
	rdval = rdval & DGSL_MASK;
	wrval =  wrval | (val << DDR_PHY_DX0GTR0_DGSL_SHIFT);
	*(volatile unsigned int*) (UINTPTR)addr = wrval;
	asm("dsb sy");
}

void clear_tap_count(void) {
	int i;

	for(i=0; i<ddr_config__lanes; i++) {
		tap_count[i] = 0;
	}
}

void set_wdqd(unsigned int wdqd_val, unsigned int wdqsl_val) {
	int i;

	for(i=0; i<ddr_config__lanes; i++) {
		set_wr_wdqd(LCDLR1_BASE + (LANE_OFFSET*i), wdqd_val);
		set_wr_wdqsl(GTR0_BASE + (LANE_OFFSET*i), wdqsl_val);
	}
}

void find_wr_center() {
	int i;

	for(i=0; i<ddr_config__lanes; i++) {
		wr_center[i].wdqd = get_wr_wdqd(LCDLR1_BASE + (LANE_OFFSET*i));
		wr_center[i].wdqsl = get_wr_wdqsl(GTR0_BASE + (LANE_OFFSET*i));
		wr_center[i].dgsl = get_wr_dgsl(GTR0_BASE + (LANE_OFFSET*i));
		wr_center[i].iprd = get_wr_iprd(MDLR0_BASE + (LANE_OFFSET*i));
		wr_center[i].tprd = get_wr_tprd(MDLR0_BASE + (LANE_OFFSET*i));
	}
}

void reset_wr_center() {
	int i;

	for(i=0; i<ddr_config__lanes; i++) {
		set_wr_wdqd(LCDLR1_BASE + (LANE_OFFSET*i), wr_center[i].wdqd);
		set_wr_wdqsl(GTR0_BASE + (LANE_OFFSET*i), wr_center[i].wdqsl);
		/*
			 set_wr_dgsl(GTR0_BASE + (LANE_OFFSET*i), wr_center[i].dgsl);
			 set_wr_iprd(MDLR0_BASE + (LANE_OFFSET*i), wr_center[i].iprd);
			 set_wr_tprd(MDLR0_BASE + (LANE_OFFSET*i), wr_center[i].tprd);
		 */
	}
}

void force_reinit() {
	unsigned int rdval, wrval;
	rdval = *(volatile unsigned int*) DDR_PHY_PIR;
	wrval = rdval | (1 << DDR_PHY_PIR_RDEYE_SHIFT);
	*(volatile unsigned int*) DDR_PHY_PIR = wrval;
	asm("dsb sy");

	rdval = *(volatile unsigned int*) DDR_PHY_PIR;
	wrval = rdval | (1 << DDR_PHY_PIR_INIT_SHIFT);
	*(volatile unsigned int*) DDR_PHY_PIR = wrval;
	asm("dsb sy");
}

void populate_wr_ds(void) {
	int i;

	for(i=0; i<ddr_config__lanes; i++) {
		wr_ds[i].wdqd = get_wr_wdqd(LCDLR1_BASE + (LANE_OFFSET*i));
		wr_ds[i].wdqsl = get_wr_wdqsl(GTR0_BASE + (LANE_OFFSET*i));				
		wr_ds[i].wlsl = get_wr_wlsl(GTR0_BASE + (LANE_OFFSET*i));
		wr_ds[i].wld = get_wr_wld(LCDLR0_BASE + (LANE_OFFSET*i));

	}
}

void populate_wr_ds_true(void) {
	int i;

	for(i=0; i<ddr_config__lanes; i++) {
		wr_ds_true[i].wdqd = get_wr_wdqd(LCDLR1_BASE + (LANE_OFFSET*i));
		wr_ds_true[i].wdqsl = get_wr_wdqsl(GTR0_BASE + (LANE_OFFSET*i));		
		wr_ds_true[i].wlsl = get_wr_wlsl(GTR0_BASE + (LANE_OFFSET*i));
		wr_ds_true[i].wld = get_wr_wld(LCDLR0_BASE + (LANE_OFFSET*i));
	}
}

void print_tap_counts(void) {
	int i;

	printf(" TAPS/cycle:\n");
	print_line3();
	for(i=0; i<ddr_config__lanes; i++) {
		printf("    %d   |", tap_count[i]);
	}
}

void poll_training_done(void) {
	unsigned int rdval;
	unsigned int idone=0, redone=0;


	while(!idone && !redone) {
		rdval = *(volatile unsigned int*) DDR_PHY_PGSR0;
		idone = rdval & (1 << DDR_PHY_PGSR0_IDONE_SHIFT);
		redone = rdval & (1 << DDR_PHY_PGSR0_REDONE_SHIFT);
	}
#ifdef DEBUG
	xil_printf("Training DONE\r\n");
#endif
}

void get_wdqd_sw_workaround() {	
	int i;
	unsigned int init_iprd, iprd;

	wr_ds = (struct WR_DS*) malloc(sizeof(struct WR_DS) * ddr_config__lanes);
	if(!wr_ds) {
		xil_printf("Allocating memory for wr_ds failed\r\n");
	}

	#ifdef DEBUG
	xil_printf("\r\n");
	xil_printf("WR_CENTER (WDQD/WDQSL): ");
	for(i=0; i<ddr_config__lanes; i++) {
		xil_printf("L%d(%u/%u), ", i, wr_center[i].wdqd, wr_center[i].wdqsl);
	}
	xil_printf("\r\n");
	#endif

	clear_tap_count();	
	iprd = get_wr_iprd(MDLR0_BASE);
	init_iprd = iprd + 20;
	#ifdef DEBUG
	xil_printf("Init IPRD: %u+20=%u\r\n\r\n", iprd, init_iprd);
	#endif
	
	set_wdqd(init_iprd, 0);
	force_reinit();
	poll_training_done();
	
	populate_wr_ds();
	
	#ifdef DEBUG
	xil_printf("WR_DS (WDQD/WDQSL) after re-training: ");
	#endif

	for(i=0; i<ddr_config__lanes; i++) {
		#ifdef DEBUG
		xil_printf("L%d(%u/%u), ", i, wr_ds[i].wdqd, wr_ds[i].wdqsl);
		#endif
		tap_count[i] = init_iprd - wr_ds[i].wdqd;
		set_wr_wdqd(LCDLR1_BASE + (LANE_OFFSET*i), wr_center[i].wdqd);
		set_wr_wdqsl(GTR0_BASE + (LANE_OFFSET*i), wr_center[i].wdqsl);
	}
	#ifdef DEBUG
	xil_printf("\r\n");
	xil_printf("\r\n");
	#endif

	free(wr_ds);
}

void print_wr_center() {
	int i;
	
	printf(" AUTO CENTER: \n");
	print_line3();
	for(i=0; i<ddr_config__lanes; i++) {
		printf("  %2d,%2d  |", wr_center[i].wdqsl, wr_center[i].wdqd);
	}
}

void set_wdqd_with_incr(int position) {
	int i;
	int xfine, xc, tc;

	for(i=0; i<ddr_config__lanes; i++) {
		tc = tap_count[i];
		xfine = wr_center[i].wdqd + position;
		xc = wr_center[i].wdqsl;

		if(tc > 0) {
			while(xfine >= tc) {
				xfine = xfine - tc;
				xc++;
			}
			while(xfine < 0) {
				xfine = xfine + tc;
				xc--;
			}
			if(xc < 0) {
				xil_printf("Underflow on WR_Lane%d\n", i);
				xfine = 0;
				xc = 0;
			}
			#ifdef DEBUG
			xil_printf("L%d(WDQD=%u,WDQSL=%u), ", i, xfine, xc);
			#endif
			set_wr_wdqd(LCDLR1_BASE + (LANE_OFFSET*i), xfine);
			set_wr_wdqsl(GTR0_BASE + (LANE_OFFSET*i), xc);
		}
	}
	asm("dsb sy");
	#ifdef DEBUG
	xil_printf("\r\n");
	#endif
}

void eye_wr_centering() {
	int i;
	for(i=0; i<ddr_config__lanes; i++) {		
		eye_start[i] = 0 - ( (eye_end[i] - eye_start[i]) / 2);
		eye_end[i] = 0 - eye_start[i];
	} 
}

void eyescan_wrsyncaborthandler(void)
{
        u64 returnadd;
        returnadd = mfelrel3();
        returnadd = returnadd + 4;
        mtelrel3(returnadd);
}
void eyescan_wrserroraborthandler(void)
{
        ;
}

void measure_wr_eye(unsigned long int testaddr, unsigned int len, unsigned int pattern, unsigned int num_iters) {
	extern double tap_ps;
	int i=0, done=0, position=0;	
	int j=0;
	unsigned int* raw_centers;
	Xil_ExceptionHandler SyncHandler = NULL, SerrorHandler = NULL;
	void *syncdata = NULL, *serrordata = NULL;

	Xil_GetExceptionRegisterHandler(XIL_EXCEPTION_ID_SYNC_INT,&SyncHandler, &syncdata);
	Xil_GetExceptionRegisterHandler(XIL_EXCEPTION_ID_SERROR_ABORT_INT,&SerrorHandler, &serrordata);
	Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_SYNC_INT, (Xil_ExceptionHandler)eyescan_wrsyncaborthandler,(void *) 0);
	Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_SERROR_ABORT_INT, (Xil_ExceptionHandler)eyescan_wrserroraborthandler,(void *) 0);

	printf("\nRunning Write Eye Tests\n");

	wr_center = (struct WR_Center*) malloc(sizeof(struct WR_Center) * ddr_config__lanes);
	if(!wr_center) {
		xil_printf("Not enough memory to run write eye measurement\n");
	}
	tap_count = (unsigned int *) malloc(sizeof(unsigned int) * ddr_config__lanes);
	if(!tap_count) {
		xil_printf("Not enough memory to run write eye measurement\n");
	}
	raw_centers = (unsigned int *) malloc(sizeof(unsigned int) * ddr_config__lanes);
	if(!raw_centers) {
		xil_printf("Not enough memory to run write eye measurement\n");
	}

	clear_eye((unsigned int *)&eye_start[0]);
	clear_eye((unsigned int *)&eye_end[0]);
	clear_eye((unsigned int *)&eye_start_temp[0]);
	clear_eye((unsigned int *)&eye_end_temp[0]);
	
	clear_results((unsigned int *)RESULTS_BASE);		// this clears system registers

	find_wr_center();
	disable_vtcomp();
	get_wdqd_sw_workaround();

	printEyeHeader();
	// move towards right edge of eye
	for (j=0; j<num_iters; j++) {
		
		done = 0;
		position = 0;
		
		while(!done) {
			position++;

			xil_printf("%3d    |", position);

			set_wdqd_with_incr(position);
			
			if(ddr_config__lanes == 8) {
				run_memtest_64bit(testaddr, len, pattern);
			}
			else if (ddr_config__lanes == 4) {
				run_memtest_32bit(testaddr, len, pattern);
			}
			else {
				xil_printf("Something went wrong in DDR config?\r\n");
			}

			print_results();

			for(i=0; i<ddr_config__lanes; i++) {
				if(*(volatile unsigned int *)(UINTPTR)(RESULTS_BASE+(i*4))!=0 && eye_end_temp[i]==0) {
					eye_end_temp[i] = position-1;
				}
			}

			done = 1;
			for(i=0; i<ddr_config__lanes; i++) {
				if(eye_end_temp[i] == 0) {
					done = 0;
				}
			}

			clear_results((unsigned int *)RESULTS_BASE);
		}
		
		// move towards left edge of eye
		done = 0;
		position = 0;

		while(!done) {
			position--;
			
			xil_printf("%3d    |", position);

			set_wdqd_with_incr(position);

			if(ddr_config__lanes == 8) {
				run_memtest_64bit(testaddr, len, pattern);
			}
			else if (ddr_config__lanes == 4) {
				run_memtest_32bit(testaddr, len, pattern);
			}
			else {
				xil_printf("Something went wrong in DDR config?\r\n");
				exit(1);
			}

			print_results();

			for(i=0; i<ddr_config__lanes; i++) {
				if(*(volatile unsigned int *)(UINTPTR)(RESULTS_BASE+(i*4))!=0 && eye_start_temp[i]==0) {
					eye_start_temp[i] = position+1;
				}
			}

			done = 1;
			for(i=0; i<ddr_config__lanes; i++) {
				if(eye_start_temp[i] == 0) {
					done = 0;
				}
			}

			clear_results((unsigned int *)RESULTS_BASE);
		}
	
		for(i=0; i<ddr_config__lanes; i++) {
			eye_start[i] = eye_start[i] + eye_start_temp[i];
			eye_end[i] = eye_end[i] + eye_end_temp[i];
		}
	}
	
	// calculate vaerage eye_start and eye_end 
	for(i=0; i<ddr_config__lanes; i++) {
		eye_start[i] = eye_start[i] / num_iters;
		eye_end[i] = eye_end[i] / num_iters;
	}
	print_line2();
	// calculate the average	
	printf("\nWrite Eye Test Results:\n");

	// adjust centering
	//eye_wr_centering();	
	printEyeResultsHeader();
	print_wr_center();
	printf("\n");
	print_line3();
	printf(" TAP value (ps):\n");
	print_line3();
	for(i=0; i<ddr_config__lanes; i++) {
		printf("   %1.2f  |", (float) ((1000000/(ddr_freq*4)) / (float) tap_count[i] ));
	}
	printf("\n");
	print_line3();
	print_tap_counts();
	printf("\n");
	print_line3();
	printf(" EYE WIDTH (ps):\n");
	print_line3();
	for(i=0; i<ddr_config__lanes; i++) {
		calc_per_tap_delay(i);
		printf(" %2.2f  |", (float) (tap_ps*(eye_end[i]-eye_start[i])) );
	}
	printf("\n");
	print_line3();
	printf(" EYE WIDTH (%%):\n");
	print_line3();
	for(i=0; i<ddr_config__lanes; i++) {
		calc_per_tap_delay(i);
		printf("  %2.2f  |", 100*(tap_ps*(eye_end[i]-eye_start[i]))/(1000000/(ddr_freq*4)) );
	}
	printf("\n");
	print_line3();
	printf(" EYE CENTER:\n");
	print_line3();
	for(i=0; i<ddr_config__lanes; i++) {
		raw_centers[i] = (wr_center[i].wdqsl*tap_count[i]) + wr_center[i].wdqd;
		raw_centers[i] = ((raw_centers[i]+eye_start[i]) + (raw_centers[i]+eye_end[i]))/2;
		printf("  %2d,%2d  |", raw_centers[i]/tap_count[i], raw_centers[i]%tap_count[i]);
	}
	printf("\n");
	print_line3();

	reset_wr_center();
	enable_vtcomp();
	free(wr_center);
	free(tap_count);	
	Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_SYNC_INT,
			SyncHandler, syncdata);
	Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_SERROR_ABORT_INT,
			SerrorHandler, serrordata);
}
