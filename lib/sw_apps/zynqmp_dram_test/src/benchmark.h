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
#define MHZ			1000000.			

static inline unsigned int rd_perf_mon_ctrl(void) {
				unsigned int val;
				asm volatile("mrs %0,   pmcr_el0" : "=r" (val));
				return val;
}

static inline void wr_perf_mon_ctrl(unsigned int val) {
				asm volatile("msr pmcr_el0, %0" : : "r" (val));
}

static inline void enable_pmu(void) {
				unsigned int val = 0;
				asm volatile("mrs %0, pmcr_el0" : "=r" (val));
				val = val | 1;
				asm volatile("msr pmcr_el0, %0" :: "r" (val));
}

static inline unsigned long pmu_read_cycle_cntr(void) { 
				unsigned long val = 0;
				asm volatile("mrs %0, pmccntr_el0" : "=r" (val));
				return val;
}

static inline void pmu_enable_cycle_cntr(void) {
				unsigned int val = PMCCNTR_EL0_EN;
				asm volatile("msr pmcntenset_el0, %0" :: "r" (val));
}

static inline void pmu_disable_cycle_cntr(void) {
				unsigned int val = PMCCNTR_EL0_DIS;
				asm volatile("msr pmcntenset_el0, %0" :: "r" (val));
}

static inline void flush_cacheline(unsigned long addr){
	asm volatile("dc civac, %0" : "=r" (addr));
	asm volatile("dsb sy");	
}

// this works okay... for both memset and memcpy to L1
void force_always_write_allocate_L1_L2() {
	#define L1RADIS_VAL	L1RADIS_DISABLE
	#define RADIS_VAL	RADIS_DISABLE
	unsigned long val;
	asm volatile("mrs %0, S3_1_C15_C2_0" : "=r" (val));
	val = val | (L1RADIS_VAL << L1RADIS_SHIFT) | (RADIS_VAL << RADIS_SHIFT);
	asm volatile("msr S3_1_C15_C2_0, %0" :: "r" (val));
	asm volatile("dsb sy");
	#undef L1RADIS_VAL
	#undef RADIS_VAL
}

void enable_L2_write_allocate() {	// looks like it works okay for memset
	#define L1RADIS_VAL	L1RADIS_4
	#define RADIS_VAL	RADIS_DISABLE
	unsigned long val;
	asm volatile("mrs %0, S3_1_C15_C2_0" : "=r" (val));
	val = val | (L1RADIS_VAL << L1RADIS_SHIFT) | (RADIS_VAL << RADIS_SHIFT);
	asm volatile("msr S3_1_C15_C2_0, %0" :: "r" (val));
	asm volatile("dsb sy");
	#undef L1RADIS_VAL
	#undef RADIS_VAL
}

void enable_L1_write_allocate() {	// does not work properly
	#define L1RADIS_VAL	L1RADIS_DISABLE
	#define RADIS_VAL	RADIS_512
	unsigned long val;
	asm volatile("mrs %0, S3_1_C15_C2_0" : "=r" (val));
	val = val | (L1RADIS_VAL << L1RADIS_SHIFT) | (RADIS_VAL << RADIS_SHIFT);
	asm volatile("msr S3_1_C15_C2_0, %0" :: "r" (val));
	asm volatile("dsb sy");	
	#undef L1RADIS_VAL
	#undef RADIS_VAL
}

void minimize_write_allocates() {
	#define L1RADIS_VAL	L1RADIS_4
	#define RADIS_VAL	RADIS_16
	unsigned long val;
	asm volatile("mrs %0, S3_1_C15_C2_0" : "=r" (val));
	val = val | (L1RADIS_VAL << L1RADIS_SHIFT) | (RADIS_VAL << RADIS_SHIFT);
	asm volatile("msr S3_1_C15_C2_0, %0" :: "r" (val));
	asm volatile("dsb sy");		
	#undef L1RADIS_VAL
	#undef RADIS_VAL
}


void benchmark(double cpufreq) {
				unsigned int bm_size[] = { 4*KBYTE, 8*KBYTE, 16*KBYTE, 32*KBYTE, 64*KBYTE, 128*KBYTE, 256*KBYTE, 512*KBYTE, 1*MBYTE, 2*MBYTE, 4*MBYTE };
				int i, lp;
				unsigned int iterations = 100000;
				unsigned long t_end, t_start = 0;
				double t_end_sec=0.0, mbps=0.0;
				unsigned int addr=0, src_addr=0, dest_addr=0;

				printf("CPU running at %g MHz\r\n", cpufreq);
				enable_pmu();	
				
				{
								xil_printf("\r\nmemset() tests\r\n");
								for (i=0; i<11; i++) {
												if(i==0) {
																addr = 0;
												}
												else {
																addr = bm_size[i-1];
												}

												pmu_disable_cycle_cntr();
												pmu_enable_cycle_cntr();

												t_start = pmu_read_cycle_cntr();
												for (lp=0; lp<iterations; lp++) {
																memset((unsigned int *)(UINTPTR)addr, 0xa5a5a5a5, bm_size[i]);
												}
												t_end = pmu_read_cycle_cntr();

												t_end_sec = (t_end-t_start) / (cpufreq*MHZ);
												mbps = (bm_size[i]/(1000.*1000.)) / (t_end_sec/iterations);								
												printf("memset -> size=%d, apu_cycles=%lu, time=%g s, throughput=%g MB/s\r\n", bm_size[i], (t_end-t_start), t_end_sec, mbps);
								}
				}
				{
								xil_printf("\r\nmemcpy() tests\r\n");
								for (i=0; i<11; i++) {
												if(i==0) {
																src_addr = 0;
																dest_addr = 0x10000000;
												}
												else {
																src_addr = bm_size[i-1];
																dest_addr = 0x10000000 + bm_size[i-1];
												}
												pmu_disable_cycle_cntr();
												pmu_enable_cycle_cntr();

												t_start = pmu_read_cycle_cntr();
												for (lp=0; lp<iterations; lp++) {								
															memcpy((unsigned int *)(UINTPTR)src_addr, (unsigned int *)(UINTPTR)dest_addr, bm_size[i]);
												}								
												t_end = pmu_read_cycle_cntr();

												t_end_sec = (t_end-t_start) / (cpufreq*MHZ);
												mbps = (bm_size[i]/(1000.*1000.)) / (t_end_sec/iterations);								
												printf("memcpy -> size=%d, apu_cycles=%lu, time=%g s, throughput=%g MB/s\r\n", bm_size[i], (t_end-t_start), t_end_sec, mbps);
								}
								asm volatile("dsb sy");
				}
				/*
				{
								xil_printf("\r\nBest case memory access test\r\n");
								disable_caches();
								addr = 0;
								*(volatile unsigned int*) addr = 0xDEADBEEF;
								enable_caches();

								pmu_disable_cycle_cntr();
								pmu_enable_cycle_cntr();

								next = *(volatile unsigned int*) addr;
								asm volatile("dsb sy");
								t_start = pmu_read_cycle_cntr();
								next = *(volatile unsigned int*) addr+32;
								t_end = pmu_read_cycle_cntr();
								t_total = t_end-t_start;

								t_end_sec = (t_total) / (cpufreq*MHZ);
								printf("Best case memory access test = %lu cycles, %g s\r\n", (t_total), t_end_sec);
				}

				*/
				/*
				{
								xil_printf("\r\nSequential memory access test\r\n");
								disable_caches();
								for(lp=0; lp<iterations; lp++) {
												next = addr + 4;
												*(volatile unsigned int*) addr = next;
												addr = next;
								}
								enable_caches();

								pmu_disable_cycle_cntr();
								pmu_enable_cycle_cntr();
								t_start = pmu_read_cycle_cntr();
								for(lp=0; lp<iterations; lp+=10) {
												next = *(volatile unsigned int*) addr;
												addr = next;
												next = *(volatile unsigned int*) addr;
												addr = next;
												next = *(volatile unsigned int*) addr;
												addr = next;
												next = *(volatile unsigned int*) addr;
												addr = next;
												next = *(volatile unsigned int*) addr;
												addr = next;
												next = *(volatile unsigned int*) addr;
												addr = next;
												next = *(volatile unsigned int*) addr;
												addr = next;
												next = *(volatile unsigned int*) addr;
												addr = next;
												next = *(volatile unsigned int*) addr;
												addr = next;
												next = *(volatile unsigned int*) addr;
												addr = next;
								}
								t_end = pmu_read_cycle_cntr();
								t_end_sec = (t_end-t_start) / (cpufreq*MHZ);

								printf("Sequential memory access test = %lu cycles (avg) , %g s\r\n", (t_end-t_start), t_end_sec);
				}
				*/
}


