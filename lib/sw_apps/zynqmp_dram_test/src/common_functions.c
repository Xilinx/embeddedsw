#include "platform_config.h"
#include "defines.h"
#include "ddrc.h"
#include "ddr_phy.h"
#include "memtest.h"
#include <stdio.h>
#include <xil_printf.h>

#define IOPLL_CTRL	    0xFF5E0020
#define RPLL_CTRL		0xFF5E0030
#define APLL_CTRL		0xFD1A0020
#define	DPLL_CTRL		0xFD1A002C
#define	VPLL_CTRL		0xFD1A0038

#define	DDR_CTRL		0xFD1A0080

#define	PLL_FBDIV_SHIFT		8
#define	PLL_FBDIV_MASK		0x00007F00
#define	PLL_DIV2_SHIFT		16
#define PLL_DIV2_MASK		0x00010000

#define SOURCE_DIV0_SHIFT	8
#define SOURCE_DIV0_MASK	0x00003F00
#define SOURCE_SRCSEL_SHIFT	0
#define SOURCE_SRCSEL_MASK	0x00000007

#define LANE0MDLR0		DDR_PHY_DX0MDLR0
#define LANEOFFSET		0x100

#define PMCCNTR_EL0_EN	 	( 1 << 31 )
#define PMCCNTR_EL0_DIS	 	( 0 << 31 )
#define PMCR_EL0_EN		( 1 << 0 )

#define L1RADIS_SHIFT	25
#define RADIS_SHIFT	27

#define L1RADIS_4	0
#define L1RADIS_64	1
#define L1RADIS_128	2
#define L1RADIS_DISABLE	3
#define RADIS_16	0
#define RADIS_128	1
#define RADIS_512	2
#define RADIS_DISABLE	3

#define KBYTE		1024	
#define MBYTE		1024*1024
#define GBYTE		1024*1024*1024
#define MHZ		1000000.			

#define CHECK_BIT(var,pos) ((var) & (1<<(pos)))


unsigned int get_ddr_config_value(unsigned int ddr, unsigned int mask, unsigned int shift) {
	return ((*(volatile unsigned int*) (UINTPTR)ddr) & mask) >> shift;
}

void ddr_config_params() {
	unsigned int ddr_config__lanes__tmp = get_ddr_config_value(DDRC_MSTR, DDRC_MSTR_DATA_BUS_WIDTH_MASK, DDRC_MSTR_DATA_BUS_WIDTH_SHIFT);
	ddr_config__lanes = (ddr_config__lanes__tmp == 0) ? 8 : 4;
	ddr_config__device = get_ddr_config_value(DDRC_MSTR, DDRC_MSTR_DEVICE_CONFIG_MASK, DDRC_MSTR_DEVICE_CONFIG_SHIFT);	
	
	ddr_config__ranks = get_ddr_config_value(DDRC_MSTR, DDRC_MSTR_ACTIVE_RANKS_MASK, DDRC_MSTR_ACTIVE_RANKS_SHIFT);
	if(ddr_config__ranks == 1) {
		ddr_config__ranks = 1;
	}
	if(ddr_config__ranks == 3) {
		ddr_config__ranks = 2;
	}
	if(ddr_config__ranks == 8) {
		ddr_config__ranks = 4;
	}

	/*
	xil_printf("Device information\r\n");
	xil_printf("%d-Rank ", ddr_config__ranks);

	if(ddr_config__lanes == 8) {
		xil_printf("64-bit ");
	}
	else if(ddr_config__lanes == 4) {
		xil_printf("32-bit ");
	}
	else {
		xil_printf("UNKNOWN! ");
	}
	
	if(CHECK_BIT(ddr_mstr_regval, 0)) {
		xil_printf("DDR3 ");
	}
	else if(CHECK_BIT(ddr_mstr_regval, 2)) {
		xil_printf("LPDDR2 ");
	}
	else if(CHECK_BIT(ddr_mstr_regval, 3)) {
		xil_printf("LPDDR3 ");
	}
	else if(CHECK_BIT(ddr_mstr_regval, 4)) {
		xil_printf("DDR4 ");
	}
	else if(CHECK_BIT(ddr_mstr_regval, 5)) {
		xil_printf("LPLPDDR2 ");
	}
	else {
		xil_printf("UNKNOWN! ");
	}
	
	xil_printf("%u\r\n", datarate); 
	*/
}

void print_ddr_config_params() {
	unsigned int datarate = ddr_freq*4;
	unsigned int ddr_mstr_regval = *(volatile unsigned int *) DDRC_MSTR;

	xil_printf("Device information\r\n");
	xil_printf("%d-Rank ", ddr_config__ranks);

	if(ddr_config__lanes == 8) {
		xil_printf("64-bit ");
	}
	else if(ddr_config__lanes == 4) {
		xil_printf("32-bit ");
	}
	else {
		xil_printf("UNKNOWN! ");
	}
	
	if(CHECK_BIT(ddr_mstr_regval, 0)) {
		xil_printf("DDR3 ");
	}
	else if(CHECK_BIT(ddr_mstr_regval, 2)) {
		xil_printf("LPDDR2 ");
	}
	else if(CHECK_BIT(ddr_mstr_regval, 3)) {
		xil_printf("LPDDR3 ");
	}
	else if(CHECK_BIT(ddr_mstr_regval, 4)) {
		xil_printf("DDR4 ");
	}
	else if(CHECK_BIT(ddr_mstr_regval, 5)) {
		xil_printf("LPLPDDR2 ");
	}
	else {
		xil_printf("UNKNOWN! ");
	}
	
	xil_printf("%u\r\n", datarate); 
}


void select_rank(int rank) {
     xil_printf("\nSelecting rank: %d\n", rank);
     unsigned int regval;
     regval = *(volatile unsigned int *) DDR_PHY_RANKIDR;
     regval = regval & !DDR_PHY_RANKIDR_RANKRID_MASK & !DDR_PHY_RANKIDR_RANKWID_MASK;
     regval = regval | ((rank) << DDR_PHY_RANKIDR_RANKRID_SHIFT) | ((rank) << DDR_PHY_RANKIDR_RANKWID_SHIFT);
     *(volatile unsigned int *) DDR_PHY_RANKIDR = regval;
     asm("dsb sy");
}


void disable_vtcomp(void) {
	unsigned int rd_val, wr_val;
	int i, done=0;

	xil_printf("Disabling VT compensation...");

	rd_val = *(volatile unsigned int *) DDR_PHY_PGCR6;
	wr_val = rd_val | (1 << DDR_PHY_PGCR6_INHVT_SHIFT);
	*(volatile unsigned int *) DDR_PHY_PGCR6 = wr_val;
	asm("dsb sy");

	for(i=0; i<10; i++) {
		//sleep(0.1);
		rd_val = *(volatile unsigned int *) DDR_PHY_PGSR1;	
		asm("dsb sy");
		if(rd_val & DDR_PHY_PGSR1_VTSTOP_MASK) {
			done = 1;
		}
	}

	if(!done) {
		xil_printf("FAILED\r\n");
	}
	else {
		xil_printf(" DONE\r\n");
	}
	
}

void enable_vtcomp(void) {
	unsigned int rd_val, wr_val;

	xil_printf("Enabling VT compensation...");

	rd_val = *(volatile unsigned int *) DDR_PHY_PGCR6;
	wr_val = rd_val & (!DDR_PHY_PGCR6_INHVT_MASK);
	*(volatile unsigned int *) DDR_PHY_PGCR6 = wr_val;
	asm("dsb sy");

	xil_printf(" DONE\r\n");
}

double pll_freq(unsigned int ddr) {
	unsigned int val;
	unsigned int multiplier, div2;
	double freq;

	val = *(volatile unsigned int *) (UINTPTR)ddr;
	multiplier = (val & PLL_FBDIV_MASK) >> PLL_FBDIV_SHIFT;
	div2 = (val & PLL_DIV2_MASK) >> PLL_DIV2_SHIFT;

	if(div2) {
		freq = (REF_FREQ * multiplier) / (2.0);
	}
	else {
		freq = (REF_FREQ * multiplier);
	}
	return freq;
}

void read_ddrc_freq(void) {
	unsigned int ddr_ctrl_val, ddr_div0, ddr_srcsel;
	extern double ddr_freq;

	ddr_ctrl_val = *(volatile unsigned int *) DDR_CTRL;
	ddr_div0 = (ddr_ctrl_val & SOURCE_DIV0_MASK) >> SOURCE_DIV0_SHIFT;
	ddr_srcsel = (ddr_ctrl_val & SOURCE_SRCSEL_MASK) >> SOURCE_SRCSEL_SHIFT;

	if(ddr_srcsel == 0x00){
		ddr_freq = pll_freq(DPLL_CTRL) / (ddr_div0*1.0);
	}
	else if (ddr_srcsel == 0x01) {
		ddr_freq = pll_freq(VPLL_CTRL) / (ddr_div0*1.0);
	}
	else {
		xil_printf("Something wrong in ddr pll configuration.  Please reboot the system.\r\n");
	}

	//printf("DDR freq = %g\r\n", ddr_freq); 
	//return ddr_freq;
}

void calc_per_tap_delay(unsigned int lane) {
	//double ddrfreq = read_ddrc_freq();
	extern double ddr_freq;
	extern double tap_ps;
	unsigned int rd_val, iprd;

	rd_val = *(volatile unsigned int *) (UINTPTR)(LANE0MDLR0 + (LANEOFFSET*lane));
	iprd = (rd_val & DDR_PHY_DX0MDLR0_IPRD_MASK) >> DDR_PHY_DX0MDLR0_IPRD_SHIFT;
	tap_ps = (1000000.0/(ddr_freq*2.0*2.0))/((double)iprd);
	//printf("IPRD=%g (TAP_PS=%g), ", (double) iprd, tap_ps);
	//return tap_ps;
}

void clear_results(unsigned int *addr) {
	unsigned int i;
	for(i=0; i<ddr_config__lanes; i++) {
		*(volatile unsigned int*) (addr+i) = 0;
	}
}

void clear_eye(unsigned int *addr) {
	unsigned int i;
	for(i=0; i<ddr_config__lanes; i++) {
		addr[i] = 0;
	}
}

void print_eye(unsigned int *addr) {
	int i;
	xil_printf("eye: ");
	for(i=0; i<ddr_config__lanes; i++) {
		xil_printf("%d(%d), ", i, addr[i] );
	}
	xil_printf("\r\n");
}

/*void printMemTestHeader(void)
{
  if (ddr_config__lanes==4) {
      xil_printf("------------------------------------------------------------------------------------------\n");
      xil_printf("   TEST                   PER-BYTE-LANE ERROR COUNT                                       \n");
      xil_printf("               [ LANE-0 ] [ LANE-1 ] [ LANE-2 ] [ LANE-3 ]                               \n");
      xil_printf("------------------------------------------------------------------------------------------\n");
  }
  if (ddr_config__lanes==8) {
      xil_printf("------------------------------------------------------------------------------------------------------------\n");
      xil_printf("   TEST                                       PER-BYTE-LANE ERROR COUNT                                     \n");
      xil_printf("               [ LANE-0 ] [ LANE-1 ] [ LANE-2 ] [ LANE-3 ] [ LANE-4 ] [ LANE-5 ] [ LANE-6 ] [ LANE-7 ]     \n");
      xil_printf("------------------------------------------------------------------------------------------------------------\n");
  }
}
*/

void print_help(void)
{
		printf("\n********************************************************************************\n");
        printf("   Zynq MPSoC\n");
        printf("   DRAM Diagnostics Test (A53) \n");
        printf("********************************************************************************\n");
        printf("   Select one of the options below:\r\n");
        printf("   +--------------------------------------------------------------------+\n");
        printf("   |  Memory Tests                                                      |\r\n");
        printf("   +-----+--------------------------------------------------------------+\n");
        printf("   | '0' | Test first 2MB region of DDR                                 |\r\n");
        printf("   | '1' | Test first 32MB region of DDR                                |\r\n");
        printf("   | '2' | Test first 64MB region of DDR                                |\r\n");
        printf("   | '3' | Test first 128MB region of DDR                               |\r\n");
        printf("   | '4' | Test first 256MB region of DDR                               |\r\n");
        printf("   | '5' | Test first 512MB region of DDR                               |\r\n");
        printf("   | '6' | Test first 1GB region of DDR                                 |\r\n");
        printf("   | '7' | Test first 2GB region of DDR                                 |\r\n");
        printf("   | '8' | Test first 4GB region of DDR                                 |\r\n");
        printf("   | '9' | Test first 8GB region of DDR                                 |\r\n");
        printf("   +-----+--------------------------------------------------------------+\n");
		printf("   |  Eye Tests                                                         |\r\n");
		printf("   +-----+--------------------------------------------------------------+\n");
		printf("   | 'r' | Perform a read eye analysis test                             |\n");
		printf("   | 'w' | Perform a write eye analysis test                            |\n");
		printf("   | 'a' | Print test start address                                     |\n");
		printf("   | 't' | Specify test start address (default=0x0)                     |\n");
		printf("   | 's' | Select the DRAM rank (default=1)                             |\n");
		printf("   +-----+--------------------------------------------------------------+\n");
        printf("   |  Miscellaneous options                                             |\n");
        printf("   +-----+--------------------------------------------------------------+\n");
        printf("   | 'i' | Print DDR information                                        |\n");
        printf("   | 'v' | Verbose Mode ON/OFF                                          |\n");
        printf("   | 'o' | Toggle cache enable/disable                                  |\n");
        printf("   | 'b' | Toggle between 32/64-bit bus widths                          |\n");
        printf("   | 'h' | Print this help menu                                         |\n");
        printf("   +-----+--------------------------------------------------------------+\n");
}

void printMemTestHeader(void)
{
      printf("---------+--------+------------------------------------------------+-----------\n");
      printf("  TEST   | ERROR  |          PER-BYTE-LANE ERROR COUNT             |  TIME\n");
      printf("         | COUNT  | LANES [ #0,  #1,  #2,  #3,  #4,  #5,  #6,  #7] |  (sec)\n");
      printf("---------+--------+------------------------------------------------+-----------\n");
}

void printEyeHeader(void)
{
  if (ddr_config__lanes==4) {
      printf("------------------------------------------------------------------------------------------\n");
      printf(" Offset   [LANE-0] [LANE-1] [LANE-2] [LANE-3]   \n");
      printf("------------------------------------------------------------------------------------------\n");
  }
  if (ddr_config__lanes==8) {
      printf("-------+--------+--------+--------+--------+--------+--------+--------+--------+\n");
      printf("Offset | LANE-0 | LANE-1 | LANE-2 | LANE-3 | LANE-4 | LANE-5 | LANE-6 | LANE-7 |\n");
      printf("-------+--------+--------+--------+--------+--------+--------+--------+--------+\n");
  }
}

void printEyeResultsHeader(void)
{
  if (ddr_config__lanes==4) {
      printf("------------------------------------------------------------------------------------------\n");
      printf("      [LANE-0] [LANE-1] [LANE-2] [LANE-3]   \n");
      printf("------------------------------------------------------------------------------------------\n");
  }
  if (ddr_config__lanes==8) {
      printf("---------+---------+---------+---------+---------+---------+---------+---------+\n");
      printf("  LANE-0 |  LANE-1 |  LANE-2 |  LANE-3 |  LANE-4 |  LANE-5 |  LANE-6 |  LANE-7 |\n");
      printf("---------+---------+---------+---------+---------+---------+---------+---------+\n");
  }
}

void print_line(void) {
   printf("--------------------------------------------------------------------------------");
}


void print_line2(void) {
   printf("-------+--------+--------+--------+--------+--------+--------+--------+--------+\n");
}

void print_line3(void) {
   printf("---------+---------+---------+---------+---------+---------+---------+----------\n");
}

void print_line4(void) {
	printf("---------+--------+------------------------------------------------+-----------\n");
}



void print_results() {
	int i;
	for(i=0; i<ddr_config__lanes; i++) {
		printf(" %6d |", *(volatile unsigned int *) (UINTPTR)(RESULTS_BASE+(i*4)) );
	}
	printf("\r\n");
}
