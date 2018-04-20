#ifndef __DEFINES_H
#define __DEFINES_H

#include "stdbool.h"

#define KB	1024
#define MB	KB*KB
#define GB	MB*KB

#define RESULTS_BASE	0xFF410020
#define DDR_ECC_CONFIG0	0xFD070070

enum patterns {
	aggressor_pattern32,
	aggressor_pattern64,
	aggressor_pattern128,
	prbs230,
	prbs231,
	prbs232,
	prbs233,
	prbs234,
	prbs235,
	prbs236
};

//unsigned int tlen[] = { 256, 512, 1024, 2048, 4096, 8192, 16384, 32768, 65536 };
//unsigned int tap_counts[] = { 85, 85, 85, 85, 85, 85, 85, 85, 85 };  // tap counts per UI

unsigned int ddr_config__device;
unsigned int ddr_config__lanes;
unsigned int ddr_config__ranks;
//unsigned int ddr_config__bl;
//
struct RD_Center {
	// stores the center points from auto train
	unsigned int qsd;
	unsigned int qsnd;
} *rd_center;

struct WR_Center {
	unsigned int wdqd;
	unsigned int wdqsl;
	unsigned int iprd;
	unsigned int tprd;
	unsigned int dgsl;
} *wr_center;

struct WR_DS {
	unsigned int wdqd;
	unsigned int wdqsl;
	unsigned int wlsl;
	unsigned int wld;
} *wr_ds, *wr_ds_true;

unsigned int *tap_count;
int *eye_start, *eye_end, *eye_start_temp, *eye_end_temp;

bool ecc_enabled;
double tap_ps;
double ddr_freq;
#endif
