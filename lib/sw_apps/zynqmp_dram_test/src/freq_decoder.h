#define IOPLL_CTRL	0xFF5E0020
#define RPLL_CTRL		0xFF5E0030
#define APLL_CTRL		0xFD1A0020
#define	DPLL_CTRL		0xFD1A002C
#define	VPLL_CTRL		0xFD1A0038

#define ACPU_CTRL		0xFD1A0060
#define LPD_LSBUS_CTRL	0xFF5E00AC

#define	PLL_FBDIV_SHIFT				8
#define	PLL_FBDIV_MASK				0x00007F00
#define	PLL_DIV2_SHIFT			16
#define PLL_DIV2_MASK				0x00010000

#define SOURCE_DIV0_SHIFT		8
#define SOURCE_DIV0_MASK		0x00003F00
#define SOURCE_SRCSEL_SHIFT	0
#define SOURCE_SRCSEL_MASK	0x00000007

#define REF_FREQ		33.3333

extern double pll_freq(unsigned int ddr);
/*double pll_freq(unsigned int addr) {
				unsigned int val;
				unsigned int multiplier, div2;
				double freq;
				
				val = *(volatile unsigned int *) addr;
				multiplier = (val & PLL_FBDIV_MASK) >> PLL_FBDIV_SHIFT;
				div2 = (val & PLL_DIV2_MASK) >> PLL_DIV2_SHIFT;

				if(div2) {
								freq = (REF_FREQ * multiplier) / (2.0);
				}
				else {
								freq = (REF_FREQ * multiplier);
				}
				return freq;
}*/

void freq_decoder(void) {
				printf("IOPLL output = %g\r\n", pll_freq(IOPLL_CTRL));
				printf("RPLL output = %g\r\n", pll_freq(RPLL_CTRL));
				printf("APLL output = %g\r\n", pll_freq(APLL_CTRL));
				printf("DPLL output = %g\r\n", pll_freq(DPLL_CTRL));
				printf("VPLL output = %g\r\n", pll_freq(VPLL_CTRL));
}

double read_cpu_freq(void) {
				unsigned int acpu_ctrl_val, acpu_div0, acpu_srcsel;
				double cpu_freq=0.0;

				acpu_ctrl_val = *(volatile unsigned int *) ACPU_CTRL;
				acpu_div0 = (acpu_ctrl_val & SOURCE_DIV0_MASK) >> SOURCE_DIV0_SHIFT;
				acpu_srcsel = (acpu_ctrl_val & SOURCE_SRCSEL_MASK) >> SOURCE_SRCSEL_SHIFT;
				
				if(acpu_srcsel == 0x00){
								cpu_freq = pll_freq(APLL_CTRL) / (acpu_div0*1.0);
				}
				else if (acpu_srcsel == 0x10) {
								cpu_freq = pll_freq(DPLL_CTRL) / (acpu_div0*1.0);
				}
				else if (acpu_srcsel == 0x11) {
								cpu_freq = pll_freq(VPLL_CTRL) / (acpu_div0*1.0);
				}
				else {
								xil_printf("Something wrong in cpu pll configuration\r\n");
								exit(1);
				}
				return cpu_freq;
}

double read_lpd_lsbus_freq(void) {
				unsigned int lsbus_ctrl_val, lsbus_div0, lsbus_srcsel;
				double lsbus_freq=0.0;

				lsbus_ctrl_val = *(volatile unsigned int *) LPD_LSBUS_CTRL;
				lsbus_div0 = (lsbus_ctrl_val & SOURCE_DIV0_MASK) >> SOURCE_DIV0_SHIFT;
				lsbus_srcsel = (lsbus_ctrl_val & SOURCE_SRCSEL_MASK) >> SOURCE_SRCSEL_SHIFT;
				//xil_printf("Debug = 0x%08x, %u, %u\r\n", lsbus_ctrl_val, lsbus_div0, lsbus_srcsel);
				if(lsbus_srcsel == 0){
								lsbus_freq = pll_freq(RPLL_CTRL) / (lsbus_div0*1.0);
				}
				else if (lsbus_srcsel == 2) {
								lsbus_freq = pll_freq(IOPLL_CTRL) / (lsbus_div0*1.0);
				}
				else if (lsbus_srcsel == 3) {
								lsbus_freq = pll_freq(DPLL_CTRL) / (lsbus_div0*1.0);
				}
				else {
								xil_printf("Something wrong in LSBUS configuration\r\n");
								exit(1);
				}
				return lsbus_freq;				
}
