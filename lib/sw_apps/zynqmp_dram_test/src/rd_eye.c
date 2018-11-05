#include "defines.h"
#include "ddr_phy.h"
#include "memtest.h"
#include <stdlib.h>
#include <stdio.h>
#include <xil_printf.h>
#include "xil_exception.h"
#include "xpseudo_asm.h"
#include "xdebug.h"

#define LANE0LCDLR3_OFFSET	DDR_PHY_DX0LCDLR3
#define LANE0LCDLR4_OFFSET	DDR_PHY_DX0LCDLR4
#define LANEOFFSET					0x100

#define PSEC	1000000000000

unsigned int get_rd_qsd(unsigned int addr) {
	return ((*(volatile unsigned int*) (UINTPTR)addr) & DDR_PHY_DX0LCDLR3_RDQSD_MASK ) >> (DDR_PHY_DX0LCDLR3_RDQSD_SHIFT);
}

unsigned int get_rd_qsnd(unsigned int addr) {
	return ((*(volatile unsigned int*) (UINTPTR)addr) & DDR_PHY_DX0LCDLR4_RDQSND_MASK ) >> (DDR_PHY_DX0LCDLR4_RDQSND_SHIFT);
}

void find_rd_center() {
	int i;

	for(i=0; i<ddr_config__lanes; i++) {
		rd_center[i].qsd = get_rd_qsd( LANE0LCDLR3_OFFSET + (LANEOFFSET*i) );
		rd_center[i].qsnd = get_rd_qsnd( LANE0LCDLR4_OFFSET + (LANEOFFSET*i) );
	}
}

void reset_rd_center() {
	int i;

	for(i=0; i<ddr_config__lanes; i++) {
		*(volatile unsigned int *)(UINTPTR)(LANE0LCDLR3_OFFSET + (LANEOFFSET*i)) = rd_center[i].qsd;
		*(volatile unsigned int *)(UINTPTR)(LANE0LCDLR4_OFFSET + (LANEOFFSET*i)) = rd_center[i].qsnd;
	}
}

void print_rd_center() {
	int i;

	printf(" AUTO CENTER: \n");
	print_line3();
	for(i=0; i<ddr_config__lanes; i++) {
		calc_per_tap_delay(i);
		printf("   %2d,%2d |", rd_center[i].qsd, rd_center[i].qsnd);
	}
	//printf("\n");
}

void set_rd_dqs_delay(int position) {
	int i;

	for(i=0; i<ddr_config__lanes; i++) {
		*(volatile unsigned int *)(UINTPTR)(LANE0LCDLR3_OFFSET + (LANEOFFSET*i)) = ((rd_center[i].qsd) + position);
		asm("dsb sy");
		*(volatile unsigned int *)(UINTPTR)(LANE0LCDLR4_OFFSET + (LANEOFFSET*i)) = ((rd_center[i].qsnd) + position);
		asm("dsb sy");
	}
	
#ifdef DEBUG
	print_qsd_qsnd();
#endif
}	

void print_qsd_qsnd() {
	int i;
	xil_printf("Lane(QSD/QSND): ");
	for(i=0; i<ddr_config__lanes; i++) {
		xil_printf("%d(%d,%d), ",i, get_rd_qsd(LANE0LCDLR3_OFFSET + (LANEOFFSET*i)), get_rd_qsnd(LANE0LCDLR4_OFFSET + (LANEOFFSET*i) ));
	}
	xil_printf("\r\n");
}

unsigned int find_min_val(void) {
	int i;
	unsigned int retval;
	unsigned int min_qsd = rd_center[0].qsd;
	unsigned int min_qsnd = rd_center[0].qsnd;

	for(i=1; i<ddr_config__lanes; i++) {
		if(rd_center[i].qsd < min_qsd) {
			min_qsd = rd_center[i].qsd;
		}
		if(rd_center[i].qsd < min_qsnd) {
			min_qsnd = rd_center[i].qsnd;
		}
	}

	retval = (min_qsd < min_qsnd) ? min_qsd : min_qsnd;

	return retval;
}

void eye_rd_centering() {
	int i;
	for(i=0; i<ddr_config__lanes; i++) {		
		eye_start[i] = 0 - ( (eye_end[i] - eye_start[i]) / 2);
		eye_end[i] = 0 - eye_start[i];
	} 
}

void eyescan_rdsyncaborthandler(void)
{
        u64 returnadd;
        returnadd = mfelrel3();
        returnadd = returnadd + 4;
        mtelrel3(returnadd);
}
void eyescan_rdserroraborthandler(void)
{
	;
}

void measure_rd_eye(unsigned long int testaddr, unsigned int len, unsigned int pattern, unsigned int num_iters) {
	extern double tap_ps;
	extern double ddr_freq;
	int i=0, done=0, position=0;
	int j=0;
	Xil_ExceptionHandler SyncHandler = NULL, SerrorHandler = NULL;
	void *syncdata = NULL, *serrordata = NULL;

	Xil_GetExceptionRegisterHandler(XIL_EXCEPTION_ID_SYNC_INT, &SyncHandler,&syncdata);
	Xil_GetExceptionRegisterHandler(XIL_EXCEPTION_ID_SERROR_ABORT_INT,&SerrorHandler, &serrordata);
	Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_SYNC_INT, (Xil_ExceptionHandler)eyescan_rdsyncaborthandler,(void *) 0);
	Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_SERROR_ABORT_INT, (Xil_ExceptionHandler)eyescan_rdserroraborthandler,(void *) 0);

	printf("\nRunning Read Eye Tests\n");

	// Dynamically allocate space for the following arrays and clear them
	rd_center = (struct RD_Center*) malloc(sizeof(struct RD_Center) * ddr_config__lanes);
	if(!rd_center) {
		xil_printf("Not enough free memory in OCM to run rd_eye_measurement\r\n");
		return;
	}

	//tap_ps = (double *) malloc(sizeof(double) * ddr_config__lanes);
	clear_eye((unsigned int *)&eye_start[0]);
	clear_eye((unsigned int *)&eye_end[0]);
	clear_results((unsigned int *)RESULTS_BASE);		// this clears system registers

	find_rd_center();

	disable_vtcomp();
	printEyeHeader();
	for (j=0; j<num_iters; j++) {
		// move towards right edge of eye
		done = 0;
		position = 0;

		while(!done) {
			position++;

			xil_printf("%3d    |", position);

			set_rd_dqs_delay(position);

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
				if( (*(volatile unsigned int *)(UINTPTR)(RESULTS_BASE+(i*4))>0) && eye_end[i]==0) {
					eye_end[i] = position-1;
				}
			}

			done = 1;
			for(i=0; i<ddr_config__lanes; i++) {
				if(eye_end[i] == 0) {
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
			set_rd_dqs_delay(position);

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
				if( (*(volatile unsigned int *)(UINTPTR)(RESULTS_BASE+(i*4))>0) && eye_start[i]==0) {
					eye_start[i] = position+1;
				}
			}

			done = 1;
			for(i=0; i<ddr_config__lanes; i++) {
				if(eye_start[i] == 0) {
					done = 0;
				}
			}

			clear_results((unsigned int *)RESULTS_BASE);
		}
	}
	print_line2();
	printf("\nRead Eye Test Results :\n");
	//eye_rd_centering();
	
	// calculate the average
	printEyeResultsHeader();
	print_rd_center();
	printf("\n");
	print_line3();
	printf(" TAP value (ps):\n");
	print_line3();
	for(i=0; i<ddr_config__lanes; i++) {
		calc_per_tap_delay(i);
		printf("   %1.2f  |", (float) tap_ps);
	}
	printf("\n");
	print_line3();
	printf(" TAPS/cycle:\n");
	print_line3();
	for(i=0; i<ddr_config__lanes; i++) {
		calc_per_tap_delay(i);
		printf("    %d   |", (int) ((1000000/(ddr_freq*4))/tap_ps));
	}
	printf("\n");
	print_line3();
	printf(" EYE WIDTH (ps):\n");
	print_line3();
	for(i=0; i<ddr_config__lanes; i++) {
		calc_per_tap_delay(i);
		printf("  %2.2f |", (float) (tap_ps*(eye_end[i]-eye_start[i])) );
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
		printf("   %2d,%2d |", ((rd_center[i].qsd+eye_end[i]) + (rd_center[i].qsd+eye_start[i]))/2,
													((rd_center[i].qsnd+eye_end[i]) + (rd_center[i].qsnd+eye_start[i]))/2);
	}
	printf("\n");
	print_line3();
	reset_rd_center();
	enable_vtcomp();
	free(rd_center);
	Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_SYNC_INT,
			SyncHandler, syncdata);
	Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_SERROR_ABORT_INT,
			SerrorHandler, serrordata);
}
