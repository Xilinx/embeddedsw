#include "xil_printf.h"
#include "xstatus.h"
#include "xil_io.h"
#include "ipi.h"
#include "pmu_global.h"

extern void outbyte(char c);
extern char inbyte();

void load_pmufw(void);
void ipi_a53(void);

int main(void)
{
	xil_printf("PMU Firmware Loader\n");
	xil_printf("L - Load PMUFW\n");
	xil_printf("T - Run Test IPI Case\n");

	while (1) {
		char c;

		c = inbyte();

		if (c == '\r') {
			outbyte('\n');
		}
		if ((c == 'l') || (c == 'L')) {
			load_pmufw();
		}
		if ((c == 't') || (c == 'T')) {
			ipi_a53();
		}

		outbyte(c);
	}

	return 0;
}

void ipi_a53(void)
{
	xil_printf("#############################################\n");
	xil_printf("##### APU-->PMU IPI-1 PM Request Test #######\n");
	xil_printf("#############################################\n");

	xil_printf("A53_0: Requesting USB0 Power Up via IPI-1\n");
	Xil_Out32(PMU_GLOBAL_GLOBAL_GEN_STORAGE1, 0xD003111B);
	Xil_Out32(IPI_APU_TRIG, IPI_APU_TRIG_PMU_1_MASK);

	while (((Xil_In32(IPI_APU_OBS)) & IPI_APU_OBS_PMU_1_MASK) != 0)
		;
	xil_printf("A53_0: Request has been Acknowledged by PMU\n");

	xil_printf("===========================================\n");

	xil_printf("A53_0: Requesting USB0 Power Down via IPI-1\n");
	Xil_Out32(PMU_GLOBAL_GLOBAL_GEN_STORAGE1, 0xD001111D);
	Xil_Out32(IPI_APU_TRIG, IPI_APU_TRIG_PMU_1_MASK);
	while (((Xil_In32(IPI_APU_OBS)) & IPI_APU_OBS_PMU_1_MASK) != 0)
		;
	xil_printf("A53_0: Request has been Acknowledged by PMU\n");

	xil_printf("================Test Done==================\n");
	xil_printf("================  PASS   ==================\n");
}

void load_pmufw(void)
{
	int i = 0;

	xil_printf("Downloading FW...\n");
	Xil_Out32(PMU_GLOBAL_GLOBAL_CNTRL, 0x0U);
	Xil_Out32(IPI_APU_TRIG, IPI_APU_TRIG_PMU_0_MASK);
	for (i = 0; i < 1000; i++)
		;

	#include "xpfw_loader_inc.c"

	Xil_Out32(PMU_GLOBAL_GLOBAL_CNTRL, 0x01U);
	xil_printf("Done\n");
}
