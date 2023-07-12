/******************************************************************************
* Copyright (c) 2021 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2023 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
 ******************************************************************************/

/*
   Zynq Memory test 1
   12/22/11

   This memory test runs on a Cortex A9 processor and executes out of OCM.
   This test uses UART for interactive operation and provides the following
   functions.

    - Memory test
    - Read eye measurement
    - Write eye measurement
    

   This test assumes that DDRC and DDR PLL are initialized.
   To run the test from xmd:
        connect arm hw -debugdevice cpunr 1
        source <path/to/hw_platform>/ps7_init.tcl
        ps7_init
        dow test.elf
        con

   When running this test from SDK, DDRC and DDR PLL are automatically
   initialized during debug launch.

   Do not do ddrremap or ramremap.

   -------------------------------------------------------------------------------
   This test assumes that the reference clock is 33.33333MHz. Please update the
   REF_CLOCK macro, if the reference clock is different.
   -------------------------------------------------------------------------------

   UPDATES
   -------
   03/22/12 - added measure read eye
   03/23/12 - added manual/ddr2 support to read eye measurement
   05/16/12 - added 16-bit bus width support
   05/30/12 - added eye status, auto-detect bus width
   10/14/13 - added info of first error in 3 gem0 regs, see ERR_INFO. Also added err_buf in OCM
              that can hold the first 80 errors and passed the ptr to it via GEM0 reg.
              Then, using the error info and forcing rd error on bits[15:0], I noticed
              that errors did not start at the 1st word - so I added cache flush after writes
              and this fixed it. It turns out that the L1 has a pseudo-random cache replace policy
              which can cause some write data to stay in the cache and be read. The L2 has 2 modes
              selected by bit25 in reg1_aux_ctrl, but it is set by default to round robin.
              Anyway, now I flush both between wr and rd and it works fine
   10/15/13 - Also added epp
   10/25/13 - set TERM_DISABLE_MODE and IBUF_DISABLE_MODE to 0 for eye measurements to work
   05/18/14 - Added D-Cache enable and disable sub-tests, Removed PLL writes.
   10/15/14 - Clean up. Better results display


   This test can communicate with register viewer via 12 mailbox registers
   at 0xF8001024.

   xregv mode can be selected thru the 'test_mode' variable. Default is
   'standalone' mode.

   Run the test from xmd as shown below:
        connect arm hw -debugdevice cpunr 1
        source <path/to/hw_platform>/ps7_init.tcl
        ps7_init
        dow test1.elf
        con
        disconnect 64
        connect arm hw -debugdevice cpunr 2

   Then start the test from xregv.

Pattern 9 confirmed via xregv.
Here is the 128-word test 10 pattern (read back from ddr via xregv):

0x00200000:   0x01010101 0x01010101 0xFEFEFEFE 0x01010101 0x01010101 0xFEFEFEFE 0x01010101 0xFEFEFEFE
0x00200020:   0xFEFEFEFE 0xFEFEFEFE 0x01010101 0xFEFEFEFE 0xFEFEFEFE 0x01010101 0xFEFEFEFE 0x01010101
0x00200040:   0x02020202 0x02020202 0xFDFDFDFD 0x02020202 0x02020202 0xFDFDFDFD 0x02020202 0xFDFDFDFD
0x00200060:   0xFDFDFDFD 0xFDFDFDFD 0x02020202 0xFDFDFDFD 0xFDFDFDFD 0x02020202 0xFDFDFDFD 0x02020202
0x00200080:   0x04040404 0x04040404 0xFBFBFBFB 0x04040404 0x04040404 0xFBFBFBFB 0x04040404 0xFBFBFBFB
0x002000A0:   0xFBFBFBFB 0xFBFBFBFB 0x04040404 0xFBFBFBFB 0xFBFBFBFB 0x04040404 0xFBFBFBFB 0x04040404
0x002000C0:   0x08080808 0x08080808 0xF7F7F7F7 0x08080808 0x08080808 0xF7F7F7F7 0x08080808 0xF7F7F7F7
0x002000E0:   0xF7F7F7F7 0xF7F7F7F7 0x08080808 0xF7F7F7F7 0xF7F7F7F7 0x08080808 0xF7F7F7F7 0x08080808
0x00200100:   0x10101010 0x10101010 0xEFEFEFEF 0x10101010 0x10101010 0xEFEFEFEF 0x10101010 0xEFEFEFEF
0x00200120:   0xEFEFEFEF 0xEFEFEFEF 0x10101010 0xEFEFEFEF 0xEFEFEFEF 0x10101010 0xEFEFEFEF 0x10101010
0x00200140:   0x20202020 0x20202020 0xDFDFDFDF 0x20202020 0x20202020 0xDFDFDFDF 0x20202020 0xDFDFDFDF
0x00200160:   0xDFDFDFDF 0xDFDFDFDF 0x20202020 0xDFDFDFDF 0xDFDFDFDF 0x20202020 0xDFDFDFDF 0x20202020
0x00200180:   0x40404040 0x40404040 0xBFBFBFBF 0x40404040 0x40404040 0xBFBFBFBF 0x40404040 0xBFBFBFBF
0x002001A0:   0xBFBFBFBF 0xBFBFBFBF 0x40404040 0xBFBFBFBF 0xBFBFBFBF 0x40404040 0xBFBFBFBF 0x40404040
0x002001C0:   0x80808080 0x80808080 0x7F7F7F7F 0x80808080 0x80808080 0x7F7F7F7F 0x80808080 0x7F7F7F7F
0x002001E0:   0x7F7F7F7F 0x7F7F7F7F 0x80808080 0x7F7F7F7F 0x7F7F7F7F 0x80808080 0x7F7F7F7F 0x80808080
 */


// -------------------------------------------------------------- INCLUDES ----
#include <xil_printf.h>
#include <stdlib.h>
#include <stdio.h>
#include "xpseudo_asm.h"
#include "xl2cc.h"
#include "xil_io.h"
#include "testDefines.h"
#include "xil_cache.h"




// ------------------------------------------------------------ PROTOTYPES ----
extern void outbyte(char c);
extern char inbyte();




// --------------------------------------------------------- PREPROCESSORS ----
// Macros
#define REG_READ(addr) \
    ({int val;int a=addr; asm volatile ("ldr   %0,[%1]\n" : "=r"(val) : "r"(a)); val;})

#define REG_WRITE(addr,val) \
    ({int v = val; int a = addr; __asm volatile ("str  %1,[%0]\n" :: "r"(a),"r"(v)); v;})

#define min(a,b)        (((a) < (b)) ? (a) : (b))
#define max(a,b)        (((a) > (b)) ? (a) : (b))
#define lim(a,lo,hi)    max(min(a,hi),lo)
#define YLFSR(a)        ((a << 1) + (((a >> 30) & 1) ^ ((a >> 27) & 1) ^ 1))

// Reference clock
#define REF_CLOCK     33.33333

// for communication with xregv use 12 sequential 16-bit regs in ttc0 starting at 0xf8001024
#define MAILBOX_XREGV     0xF8001000       /* xregv mode here, a 7-bit reg */
#define MAILBOX           0xF8001024
#define MAILBOX_GO        MAILBOX+0x00     /* incr to start a test */
#define MAILBOX_DONE      MAILBOX+0x04     /* incr to indicate done */
#define MAILBOX_STAT      MAILBOX+0x08     /* status */
#define MAILBOX_START     MAILBOX+0x0C     /* test start addr in MB units */
#define MAILBOX_SIZE      MAILBOX+0x10     /* [9:0]=test size in MB, [15:10]=loop_cnt */
#define MAILBOX_RESULT    MAILBOX+0x14     /* 4 error counts per byte lane, + total */
#define MAILBOX_MODE      MAILBOX+0x28     /* mode, 1 bit per test, if msb=0 */
#define MAILBOX_LAST      MAILBOX+0x2C     /* last of the 12 words */

// Added error info in 3 gem0 regs: these are useable: 80,84,88,--,90,--,98,a0
#define ERR_INFO          0xE000B080

// PLL
#define bit_PLL_BYPASS_FORCE  4
#define bit_PLL_RESET         0
#define bit_PLL_FDIV          12
#define mask_PLL_FDIV         0x07f
#define pll_select_cpu        0
#define pll_select_ddr        1

#define reg_drive_addr    0xf8000b5c
#define reg_drive_data    0xf8000b60
#define reg_drive_diff    0xf8000b64
#define reg_drive_clock   0xf8000b68

// XADC Register Space
#define XADC_CFG0         0x40
#define XADC_CFG1         0x41
#define XADC_CFG2         0x42
#define XADC_TEMP         0
#define XADC_VCCINT       1
#define XADC_VCCAUX       2
#define XADC_VPN          3
#define XADC_VREEP        4
#define XADC_VREEN        5
#define XADC_VCCBRAM      6
#define XADC_VCCPINT      0xd
#define XADC_VCCPAUX      0xe
#define XADC_VCCPDRO      0xf
#define XADC_ALARM        0x3f




// ------------------------------------------------------------ VARIABLES ----
int werr;
int epp;                             // epp = errors per pattern, 1 bit per subtest pattern
int errcnt[4];
int gresult[200];                    // enough space for 20 write eye results
int err_buf[256];                    // 10/14/2013: capture the first ~80 errors: format is:  addr,wr,rd, addr,wr,rd,...
int q1[21];
int q2[21];
int q3[21];
int q4[21];
int qmask              = 0xffffffff; // default
int bus_width          = 32;         // 32 or 16 bits
int test_mode          = 1;          // 1 standalone menu for eye measure etc, 0 for xregv memtest.
int verbose            = 1;          // 1=memtest, 2=writeeye, 4='after', 8=print errors, 0x10=qual, 0x20 eye info, 0x40-lppdr2/drive test
int rseed              = 0;          // random seed
int memtest_stat       = 1;          // enables memtest status upload
int fast_is_bc         = 0;          // set to 0 for normal operation, 1= fast is best-case eye
int csv_mode           = 0;
int do_ddr_pll_init    = 1;          // 1=do, 0=don't init ddr pll
int do_standalone      = 0;          // 1=do pll/ddr init per test1_config.c, 0=assume it's already done
char cr[4]             = "\r\n";
double pclk            = 111.11111;  // pclk freq in mhz for cpu at 667
double test_time;
double total_test_time;



// vars for multi-loop memory tests
int cum_errcnt[4];                   // cumulative
int test_sizes[8]      = { 16, 32, 64, 127,  128, 255, 511, 1023 };
int test_size_sel      = 0;
int test_loop_cnt      = 1;

// vars for xadc access
int alist[8]           = {XADC_TEMP, XADC_VCCINT, XADC_VCCAUX, XADC_VCCBRAM, XADC_VCCPINT, XADC_VCCPAUX, XADC_VCCPDRO, 0};
int dlist[8];


// config vars
char config_id[20];
double ddrclk          = 400.0;      // DDR clock
int wr_data_offset[4]  = { 56, 56, 56, 56 };  // default wr data offset config

int ddriob_reg_values[2*15] = {
    0xf8000b40, 0,      //    1: DDRIOB_ADDR0 at 0xB40
    0xf8000b44, 0,      //    2: DDRIOB_ADDR1 at 0xB44
    0xf8000b48, 0,      //    3: DDRIOB_DATA0 at 0xB48
    0xf8000b4c, 0,      //    4: DDRIOB_DATA1 at 0xB4C
    0xf8000b50, 0,      //    5: DDRIOB_DIFF0 at 0xB50
    0xf8000b54, 0,      //    6: DDRIOB_DIFF1 at 0xB54
    0xf8000b58, 0,      //    7: DDRIOB_CLOCK at 0xB58
    0xf8000b5c, 0,      //    8: DDRIOB_DRIVE_SLEW_ADDR at 0xB5C
    0xf8000b60, 0,      //    9: DDRIOB_DRIVE_SLEW_DATA at 0xB60
    0xf8000b64, 0,      //   10: DDRIOB_DRIVE_SLEW_DIFF at 0xB64
    0xf8000b68, 0,      //   11: DDRIOB_DRIVE_SLEW_CLOCK at 0xB68
    0xf8000b6c, 0,      //   12: DDRIOB_DDR_CTRL at 0xB6C
    0xf8000b70, 0,      //   13: DDRIOB_DCI_CTRL at 0xB70
    0xf8000b70, 0,      //   14: DDRIOB_DCI_CTRL at 0xB70
    0xf8000b70, 0       //   15: DDRIOB_DCI_CTRL at 0xB70
};

int ddrc_reg_values[2*80] = {
    0xf8006000, 0,      //   16: ddrc_ctrl at 0x0
    0xf8006004, 0,      //   17: Two_rank_cfg at 0x4
    0xf8006008, 0,      //   18: HPR_reg at 0x8
    0xf800600c, 0,      //   19: LPR_reg at 0xC
    0xf8006010, 0,      //   20: WR_reg at 0x10
    0xf8006014, 0,      //   21: DRAM_param_reg0 at 0x14
    0xf8006018, 0,      //   22: DRAM_param_reg1 at 0x18
    0xf800601c, 0,      //   23: DRAM_param_reg2 at 0x1C
    0xf8006020, 0,      //   24: DRAM_param_reg3 at 0x20
    0xf8006024, 0,      //   25: DRAM_param_reg4 at 0x24
    0xf8006028, 0,      //   26: DRAM_init_param at 0x28
    0xf800602c, 0,      //   27: DRAM_EMR_reg at 0x2C
    0xf8006030, 0,      //   28: DRAM_EMR_MR_reg at 0x30
    0xf8006034, 0,      //   29: DRAM_burst8_rdwr at 0x34
    0xf8006038, 0,      //   30: DRAM_disable_DQ at 0x38
    0xf800603c, 0,      //   31: DRAM_addr_map_bank at 0x3C
    0xf8006040, 0,      //   32: DRAM_addr_map_col at 0x40
    0xf8006044, 0,      //   33: DRAM_addr_map_row at 0x44
    0xf8006048, 0,      //   34: DRAM_ODT_reg at 0x48
    0xf8006050, 0,      //   35: phy_cmd_timeout_rddata_cpt at 0x50
    0xf8006058, 0,      //   36: DLL_calib at 0x58
    0xf800605c, 0,      //   37: ODT_delay_hold at 0x5C
    0xf8006060, 0,      //   38: ctrl_reg1 at 0x60
    0xf8006064, 0,      //   39: ctrl_reg2 at 0x64
    0xf8006068, 0,      //   40: ctrl_reg3 at 0x68
    0xf800606c, 0,      //   41: ctrl_reg4 at 0x6C
    0xf80060a0, 0,      //   42: CHE_REFRESH_TIMER01 at 0xA0
    0xf80060a4, 0,      //   43: CHE_T_ZQ at 0xA4
    0xf80060a8, 0,      //   44: CHE_T_ZQ_Short_Interval_Reg at 0xA8
    0xf80060ac, 0,      //   45: deep_pwrdwn_reg at 0xAC
    0xf80060b0, 0,      //   46: reg_2c at 0xB0
    0xf80060b4, 0,      //   47: reg_2d at 0xB4
    0xf80060b8, 0,      //   48: dfi_timing at 0xB8
    0xf80060bc, 0,      //   49: refresh_timer_2 at 0xBC
    0xf80060c4, 0,      //   50: CHE_ECC_CONTROL_REG_OFFSET at 0xC4
    0xf80060c8, 0,      //   51: CHE_CORR_ECC_LOG_REG_OFFSET at 0xC8
    0xf80060dc, 0,      //   52: CHE_UNCORR_ECC_LOG_REG_OFFSET at 0xDC
    0xf80060f0, 0,      //   53: CHE_ECC_STATS_REG_OFFSET at 0xF0
    0xf80060f4, 0,      //   54: ECC_scrub at 0xF4
    0xf8006114, 0,      //   55: phy_rcvr_enable at 0x114
    0xf8006118, 0,      //   56: PHY_Config0 at 0x118
    0xf800611c, 0,      //   57: PHY_Config1 at 0x11C
    0xf8006120, 0,      //   58: PHY_Config2 at 0x120
    0xf8006124, 0,      //   59: PHY_Config3 at 0x124
    0xf800612c, 0,      //   60: phy_init_ratio0 at 0x12C
    0xf8006130, 0,      //   61: phy_init_ratio1 at 0x130
    0xf8006134, 0,      //   62: phy_init_ratio2 at 0x134
    0xf8006138, 0,      //   63: phy_init_ratio3 at 0x138
    0xf8006140, 0,      //   64: phy_rd_dqs_cfg0 at 0x140
    0xf8006144, 0,      //   65: phy_rd_dqs_cfg1 at 0x144
    0xf8006148, 0,      //   66: phy_rd_dqs_cfg2 at 0x148
    0xf800614c, 0,      //   67: phy_rd_dqs_cfg3 at 0x14C
    0xf8006154, 0,      //   68: phy_wr_dqs_cfg0 at 0x154
    0xf8006158, 0,      //   69: phy_wr_dqs_cfg1 at 0x158
    0xf800615c, 0,      //   70: phy_wr_dqs_cfg2 at 0x15C
    0xf8006160, 0,      //   71: phy_wr_dqs_cfg3 at 0x160
    0xf8006168, 0,      //   72: phy_we_cfg0 at 0x168
    0xf800616c, 0,      //   73: phy_we_cfg1 at 0x16C
    0xf8006170, 0,      //   74: phy_we_cfg2 at 0x170
    0xf8006174, 0,      //   75: phy_we_cfg3 at 0x174
    0xf800617c, 0,      //   76: wr_data_slv0 at 0x17C
    0xf8006180, 0,      //   77: wr_data_slv1 at 0x180
    0xf8006184, 0,      //   78: wr_data_slv2 at 0x184
    0xf8006188, 0,      //   79: wr_data_slv3 at 0x188
    0xf8006190, 0,      //   80: reg_64 at 0x190
    0xf8006194, 0,      //   81: reg_65 at 0x194
    0xf8006204, 0,      //   82: page_mask at 0x204
    0xf8006208, 0,      //   83: axi_priority_wr_port0 at 0x208
    0xf800620c, 0,      //   84: axi_priority_wr_port1 at 0x20C
    0xf8006210, 0,      //   85: axi_priority_wr_port2 at 0x210
    0xf8006214, 0,      //   86: axi_priority_wr_port3 at 0x214
    0xf8006218, 0,      //   87: axi_priority_rd_port0 at 0x218
    0xf800621c, 0,      //   88: axi_priority_rd_port1 at 0x21C
    0xf8006220, 0,      //   89: axi_priority_rd_port2 at 0x220
    0xf8006224, 0,      //   90: axi_priority_rd_port3 at 0x224
    0xf80062a8, 0,      //   91: lpddr_ctrl0 at 0x2A8
    0xf80062ac, 0,      //   92: lpddr_ctrl1 at 0x2AC
    0xf80062b0, 0,      //   93: lpddr_ctrl2 at 0x2B0
    0xf80062b4, 0,      //   94: lpddr_ctrl3 at 0x2B4
    0xf8006000, 0       //   95: ddrc_ctrl at 0x0
};

int ddrc_reg_values_save[2*80];


char pll_id[3][8] = { "CPU", "DDR", "IOU" };
int R00 = DDR_LOW_BASE + 0;
int R2C = DDR_LOW_BASE + 0x0B0;
int R46 = DDR_LOW_BASE + 0x118;
int R50 = DDR_LOW_BASE + 0x140;
int R55 = DDR_LOW_BASE + 0x154; // R055_reg_phy_wr_dqs_slave_ratio  R055[9:0]
int R5F = DDR_LOW_BASE + 0x17C;
int R65 = DDR_LOW_BASE + 0x194;
int R5A = DDR_LOW_BASE + 0x168; // 245: R05A_reg_phy_fifo_we_slave_ratio  R05A[10:0] , type=rw , defval=0x40


// PLL_CFG table: the res,cp,cnt values.
int pll_config_table[100] = {
    0x001772C0,    // fbdiv=0:  res=12,  cp=2,  cnt=375
    0x001772C0,    // fbdiv=1:  res=12,  cp=2,  cnt=375
    0x001772C0,    // fbdiv=2:  res=12,  cp=2,  cnt=375
    0x001772C0,    // fbdiv=3:  res=12,  cp=2,  cnt=375
    0x001772C0,    // fbdiv=4:  res=12,  cp=2,  cnt=375
    0x001772C0,    // fbdiv=5:  res=12,  cp=2,  cnt=375
    0x001772C0,    // fbdiv=6:  res=12,  cp=2,  cnt=375
    0x001772C0,    // fbdiv=7:  res=12,  cp=2,  cnt=375
    0x001772C0,    // fbdiv=8:  res=12,  cp=2,  cnt=375
    0x001772C0,    // fbdiv=9:  res=12,  cp=2,  cnt=375
    0x001772C0,    // fbdiv=10:  res=12,  cp=2,  cnt=375
    0x001772C0,    // fbdiv=11:  res=12,  cp=2,  cnt=375
    0x001772C0,    // fbdiv=12:  res=12,  cp=2,  cnt=375
    0x001772C0,    // fbdiv=13:  res=12,  cp=2,  cnt=375
    0x001772C0,    // fbdiv=14:  res=12,  cp=2,  cnt=375
    0x001772C0,    // fbdiv=15:  res=12,  cp=2,  cnt=375
    0x001772C0,    // fbdiv=16:  res=12,  cp=2,  cnt=375
    0x001772C0,    // fbdiv=17:  res=12,  cp=2,  cnt=375
    0x001772C0,    // fbdiv=18:  res=12,  cp=2,  cnt=375
    0x001772C0,    // fbdiv=19:  res=12,  cp=2,  cnt=375
    0x001772C0,    // fbdiv=20:  res=12,  cp=2,  cnt=375
    0x001772C0,    // fbdiv=21:  res=12,  cp=2,  cnt=375
    0x001772C0,    // fbdiv=22:  res=12,  cp=2,  cnt=375
    0x001772C0,    // fbdiv=23:  res=12,  cp=2,  cnt=375
    0x001772C0,    // fbdiv=24:  res=12,  cp=2,  cnt=375
    0x001772C0,    // fbdiv=25:  res=12,  cp=2,  cnt=375
    0x001772C0,    // fbdiv=26:  res=12,  cp=2,  cnt=375
    0x0015E2C0,    // fbdiv=27:  res=12,  cp=2,  cnt=350
    0x0015E2C0,    // fbdiv=28:  res=12,  cp=2,  cnt=350
    0x001452C0,    // fbdiv=29:  res=12,  cp=2,  cnt=325
    0x001452C0,    // fbdiv=30:  res=12,  cp=2,  cnt=325
    0x0012C220,    // fbdiv=31:  res=2,  cp=2,  cnt=300
    0x0012C220,    // fbdiv=32:  res=2,  cp=2,  cnt=300
    0x0012C220,    // fbdiv=33:  res=2,  cp=2,  cnt=300
    0x00113220,    // fbdiv=34:  res=2,  cp=2,  cnt=275
    0x00113220,    // fbdiv=35:  res=2,  cp=2,  cnt=275
    0x00113220,    // fbdiv=36:  res=2,  cp=2,  cnt=275
    0x000FA220,    // fbdiv=37:  res=2,  cp=2,  cnt=250
    0x000FA220,    // fbdiv=38:  res=2,  cp=2,  cnt=250
    0x000FA220,    // fbdiv=39:  res=2,  cp=2,  cnt=250
    0x000FA220,    // fbdiv=40:  res=2,  cp=2,  cnt=250
    0x000FA3C0,    // fbdiv=41:  res=12,  cp=3,  cnt=250
    0x000FA3C0,    // fbdiv=42:  res=12,  cp=3,  cnt=250
    0x000FA3C0,    // fbdiv=43:  res=12,  cp=3,  cnt=250
    0x000FA3C0,    // fbdiv=44:  res=12,  cp=3,  cnt=250
    0x000FA3C0,    // fbdiv=45:  res=12,  cp=3,  cnt=250
    0x000FA3C0,    // fbdiv=46:  res=12,  cp=3,  cnt=250
    0x000FA3C0,    // fbdiv=47:  res=12,  cp=3,  cnt=250
    0x000FA240,    // fbdiv=48:  res=4,  cp=2,  cnt=250
    0x000FA240,    // fbdiv=49:  res=4,  cp=2,  cnt=250
    0x000FA240,    // fbdiv=50:  res=4,  cp=2,  cnt=250
    0x000FA240,    // fbdiv=51:  res=4,  cp=2,  cnt=250
    0x000FA240,    // fbdiv=52:  res=4,  cp=2,  cnt=250
    0x000FA240,    // fbdiv=53:  res=4,  cp=2,  cnt=250
    0x000FA240,    // fbdiv=54:  res=4,  cp=2,  cnt=250
    0x000FA240,    // fbdiv=55:  res=4,  cp=2,  cnt=250
    0x000FA240,    // fbdiv=56:  res=4,  cp=2,  cnt=250
    0x000FA240,    // fbdiv=57:  res=4,  cp=2,  cnt=250
    0x000FA240,    // fbdiv=58:  res=4,  cp=2,  cnt=250
    0x000FA240,    // fbdiv=59:  res=4,  cp=2,  cnt=250
    0x000FA240,    // fbdiv=60:  res=4,  cp=2,  cnt=250
    0x000FA240,    // fbdiv=61:  res=4,  cp=2,  cnt=250
    0x000FA240,    // fbdiv=62:  res=4,  cp=2,  cnt=250
    0x000FA240,    // fbdiv=63:  res=4,  cp=2,  cnt=250
    0x000FA240,    // fbdiv=64:  res=4,  cp=2,  cnt=250
    0x000FA240,    // fbdiv=65:  res=4,  cp=2,  cnt=250
    0x000FA240,    // fbdiv=66:  res=4,  cp=2,  cnt=250
    0x000FA240,    // fbdiv=67:  res=4,  cp=2,  cnt=250
    0x000FA240,    // fbdiv=68:  res=4,  cp=2,  cnt=250
    0x000FA240,    // fbdiv=69:  res=4,  cp=2,  cnt=250
    0x000FA240,    // fbdiv=70:  res=4,  cp=2,  cnt=250
    0x000FA240,    // fbdiv=71:  res=4,  cp=2,  cnt=250
    0x000FA240,    // fbdiv=72:  res=4,  cp=2,  cnt=250
    0x000FA240,    // fbdiv=73:  res=4,  cp=2,  cnt=250
    0x000FA240,    // fbdiv=74:  res=4,  cp=2,  cnt=250
    0x000FA240,    // fbdiv=75:  res=4,  cp=2,  cnt=250
    0x000FA240,    // fbdiv=76:  res=4,  cp=2,  cnt=250
    0x000FA240,    // fbdiv=77:  res=4,  cp=2,  cnt=250
    0x000FA240,    // fbdiv=78:  res=4,  cp=2,  cnt=250
    0x000FA240,    // fbdiv=79:  res=4,  cp=2,  cnt=250
    0x000FA240,    // fbdiv=80:  res=4,  cp=2,  cnt=250
    0x000FA240,    // fbdiv=81:  res=4,  cp=2,  cnt=250
    0x000FA240,    // fbdiv=82:  res=4,  cp=2,  cnt=250
    0x000FA240,    // fbdiv=83:  res=4,  cp=2,  cnt=250
    0x000FA240,    // fbdiv=84:  res=4,  cp=2,  cnt=250
    0x000FA240,    // fbdiv=85:  res=4,  cp=2,  cnt=250
    0x000FA240,    // fbdiv=86:  res=4,  cp=2,  cnt=250
    0x000FA240,    // fbdiv=87:  res=4,  cp=2,  cnt=250
    0x000FA240,    // fbdiv=88:  res=4,  cp=2,  cnt=250
    0x000FA240,    // fbdiv=89:  res=4,  cp=2,  cnt=250
    0x000FA240,    // fbdiv=90:  res=4,  cp=2,  cnt=250
    0x000FA240,    // fbdiv=91:  res=4,  cp=2,  cnt=250
    0x000FA240,    // fbdiv=92:  res=4,  cp=2,  cnt=250
    0x000FA240,    // fbdiv=93:  res=4,  cp=2,  cnt=250
    0x000FA240,    // fbdiv=94:  res=4,  cp=2,  cnt=250
    0x000FA240,    // fbdiv=95:  res=4,  cp=2,  cnt=250
    0x000FA240,    // fbdiv=96:  res=4,  cp=2,  cnt=250
    0x000FA240,    // fbdiv=97:  res=4,  cp=2,  cnt=250
    0x000FA240,    // fbdiv=98:  res=4,  cp=2,  cnt=250
    0x000FA240     // fbdiv=99:  res=4,  cp=2,  cnt=250
};

// DCI divisor values for getting 10MHz DCI clock
int dci_ctrl_table[181] = {
    0x02300201,    // 70: 2 x 35 = 70
    0x02400201,    // 71: 2 x 36 = 72
    0x02400201,    // 72: 2 x 36 = 72
    0x02500201,    // 73: 2 x 37 = 74
    0x02500201,    // 74: 2 x 37 = 74
    0x01900301,    // 75: 3 x 25 = 75
    0x02600201,    // 76: 2 x 38 = 76
    0x00B00701,    // 77: 7 x 11 = 77
    0x02700201,    // 78: 2 x 39 = 78
    0x02800201,    // 79: 2 x 40 = 80
    0x02800201,    // 80: 2 x 40 = 80
    0x01B00301,    // 81: 3 x 27 = 81
    0x02900201,    // 82: 2 x 41 = 82
    0x02A00201,    // 83: 2 x 42 = 84
    0x02A00201,    // 84: 2 x 42 = 84
    0x01100501,    // 85: 5 x 17 = 85
    0x02B00201,    // 86: 2 x 43 = 86
    0x01D00301,    // 87: 3 x 29 = 87
    0x02C00201,    // 88: 2 x 44 = 88
    0x02D00201,    // 89: 2 x 45 = 90
    0x02D00201,    // 90: 2 x 45 = 90
    0x00D00701,    // 91: 7 x 13 = 91
    0x02E00201,    // 92: 2 x 46 = 92
    0x01F00301,    // 93: 3 x 31 = 93
    0x02F00201,    // 94: 2 x 47 = 94
    0x01300501,    // 95: 5 x 19 = 95
    0x03000201,    // 96: 2 x 48 = 96
    0x03100201,    // 97: 2 x 49 = 98
    0x03100201,    // 98: 2 x 49 = 98
    0x02100301,    // 99: 3 x 33 = 99
    0x03200201,    // 100: 2 x 50 = 100
    0x03300201,    // 101: 2 x 51 = 102
    0x03300201,    // 102: 2 x 51 = 102
    0x03400201,    // 103: 2 x 52 = 104
    0x03400201,    // 104: 2 x 52 = 104
    0x02300301,    // 105: 3 x 35 = 105
    0x03500201,    // 106: 2 x 53 = 106
    0x03600201,    // 107: 2 x 54 = 108
    0x03600201,    // 108: 2 x 54 = 108
    0x03700201,    // 109: 2 x 55 = 110
    0x03700201,    // 110: 2 x 55 = 110
    0x02500301,    // 111: 3 x 37 = 111
    0x03800201,    // 112: 2 x 56 = 112
    0x03900201,    // 113: 2 x 57 = 114
    0x03900201,    // 114: 2 x 57 = 114
    0x01700501,    // 115: 5 x 23 = 115
    0x03A00201,    // 116: 2 x 58 = 116
    0x02700301,    // 117: 3 x 39 = 117
    0x03B00201,    // 118: 2 x 59 = 118
    0x01100701,    // 119: 7 x 17 = 119
    0x03C00201,    // 120: 2 x 60 = 120
    0x00B00B01,    // 121: 11 x 11 = 121
    0x03D00201,    // 122: 2 x 61 = 122
    0x02900301,    // 123: 3 x 41 = 123
    0x03E00201,    // 124: 2 x 62 = 124
    0x01900501,    // 125: 5 x 25 = 125
    0x03F00201,    // 126: 2 x 63 = 126
    0x02000401,    // 127: 4 x 32 = 128
    0x02000401,    // 128: 4 x 32 = 128
    0x02B00301,    // 129: 3 x 43 = 129
    0x01A00501,    // 130: 5 x 26 = 130
    0x02C00301,    // 131: 3 x 44 = 132
    0x02C00301,    // 132: 3 x 44 = 132
    0x01300701,    // 133: 7 x 19 = 133
    0x02D00301,    // 134: 3 x 45 = 135
    0x02D00301,    // 135: 3 x 45 = 135
    0x02200401,    // 136: 4 x 34 = 136
    0x02E00301,    // 137: 3 x 46 = 138
    0x02E00301,    // 138: 3 x 46 = 138
    0x02300401,    // 139: 4 x 35 = 140
    0x02300401,    // 140: 4 x 35 = 140
    0x02F00301,    // 141: 3 x 47 = 141
    0x00D00B01,    // 142: 11 x 13 = 143
    0x00D00B01,    // 143: 11 x 13 = 143
    0x03000301,    // 144: 3 x 48 = 144
    0x01D00501,    // 145: 5 x 29 = 145
    0x03100301,    // 146: 3 x 49 = 147
    0x03100301,    // 147: 3 x 49 = 147
    0x02500401,    // 148: 4 x 37 = 148
    0x03200301,    // 149: 3 x 50 = 150
    0x03200301,    // 150: 3 x 50 = 150
    0x02600401,    // 151: 4 x 38 = 152
    0x02600401,    // 152: 4 x 38 = 152
    0x03300301,    // 153: 3 x 51 = 153
    0x01600701,    // 154: 7 x 22 = 154
    0x01F00501,    // 155: 5 x 31 = 155
    0x03400301,    // 156: 3 x 52 = 156
    0x03500301,    // 157: 3 x 53 = 159
    0x03500301,    // 158: 3 x 53 = 159
    0x03500301,    // 159: 3 x 53 = 159
    0x02800401,    // 160: 4 x 40 = 160
    0x01700701,    // 161: 7 x 23 = 161
    0x03600301,    // 162: 3 x 54 = 162
    0x02900401,    // 163: 4 x 41 = 164
    0x02900401,    // 164: 4 x 41 = 164
    0x03700301,    // 165: 3 x 55 = 165
    0x03800301,    // 166: 3 x 56 = 168
    0x03800301,    // 167: 3 x 56 = 168
    0x03800301,    // 168: 3 x 56 = 168
    0x00D00D01,    // 169: 13 x 13 = 169
    0x02200501,    // 170: 5 x 34 = 170
    0x03900301,    // 171: 3 x 57 = 171
    0x02B00401,    // 172: 4 x 43 = 172
    0x03A00301,    // 173: 3 x 58 = 174
    0x03A00301,    // 174: 3 x 58 = 174
    0x02300501,    // 175: 5 x 35 = 175
    0x02C00401,    // 176: 4 x 44 = 176
    0x03B00301,    // 177: 3 x 59 = 177
    0x03C00301,    // 178: 3 x 60 = 180
    0x03C00301,    // 179: 3 x 60 = 180
    0x03C00301,    // 180: 3 x 60 = 180
    0x01A00701,    // 181: 7 x 26 = 182
    0x01A00701,    // 182: 7 x 26 = 182
    0x03D00301,    // 183: 3 x 61 = 183
    0x02E00401,    // 184: 4 x 46 = 184
    0x02500501,    // 185: 5 x 37 = 185
    0x03E00301,    // 186: 3 x 62 = 186
    0x01100B01,    // 187: 11 x 17 = 187
    0x02F00401,    // 188: 4 x 47 = 188
    0x03F00301,    // 189: 3 x 63 = 189
    0x02600501,    // 190: 5 x 38 = 190
    0x03000401,    // 191: 4 x 48 = 192
    0x03000401,    // 192: 4 x 48 = 192
    0x02700501,    // 193: 5 x 39 = 195
    0x02700501,    // 194: 5 x 39 = 195
    0x02700501,    // 195: 5 x 39 = 195
    0x03100401,    // 196: 4 x 49 = 196
    0x02100601,    // 197: 6 x 33 = 198
    0x02100601,    // 198: 6 x 33 = 198
    0x03200401,    // 199: 4 x 50 = 200
    0x03200401,    // 200: 4 x 50 = 200
    0x01D00701,    // 201: 7 x 29 = 203
    0x01D00701,    // 202: 7 x 29 = 203
    0x01D00701,    // 203: 7 x 29 = 203
    0x03300401,    // 204: 4 x 51 = 204
    0x02900501,    // 205: 5 x 41 = 205
    0x01700901,    // 206: 9 x 23 = 207
    0x01700901,    // 207: 9 x 23 = 207
    0x03400401,    // 208: 4 x 52 = 208
    0x01300B01,    // 209: 11 x 19 = 209
    0x02A00501,    // 210: 5 x 42 = 210
    0x03500401,    // 211: 4 x 53 = 212
    0x03500401,    // 212: 4 x 53 = 212
    0x02B00501,    // 213: 5 x 43 = 215
    0x02B00501,    // 214: 5 x 43 = 215
    0x02B00501,    // 215: 5 x 43 = 215
    0x03600401,    // 216: 4 x 54 = 216
    0x01F00701,    // 217: 7 x 31 = 217
    0x03700401,    // 218: 4 x 55 = 220
    0x03700401,    // 219: 4 x 55 = 220
    0x03700401,    // 220: 4 x 55 = 220
    0x01100D01,    // 221: 13 x 17 = 221
    0x02500601,    // 222: 6 x 37 = 222
    0x03800401,    // 223: 4 x 56 = 224
    0x03800401,    // 224: 4 x 56 = 224
    0x02D00501,    // 225: 5 x 45 = 225
    0x03900401,    // 226: 4 x 57 = 228
    0x03900401,    // 227: 4 x 57 = 228
    0x03900401,    // 228: 4 x 57 = 228
    0x02E00501,    // 229: 5 x 46 = 230
    0x02E00501,    // 230: 5 x 46 = 230
    0x02100701,    // 231: 7 x 33 = 231
    0x03A00401,    // 232: 4 x 58 = 232
    0x02700601,    // 233: 6 x 39 = 234
    0x02700601,    // 234: 6 x 39 = 234
    0x02F00501,    // 235: 5 x 47 = 235
    0x03B00401,    // 236: 4 x 59 = 236
    0x02200701,    // 237: 7 x 34 = 238
    0x02200701,    // 238: 7 x 34 = 238
    0x03C00401,    // 239: 4 x 60 = 240
    0x03C00401,    // 240: 4 x 60 = 240
    0x01600B01,    // 241: 11 x 22 = 242
    0x01600B01,    // 242: 11 x 22 = 242
    0x01B00901,    // 243: 9 x 27 = 243
    0x03D00401,    // 244: 4 x 61 = 244
    0x03100501,    // 245: 5 x 49 = 245
    0x02900601,    // 246: 6 x 41 = 246
    0x01300D01,    // 247: 13 x 19 = 247
    0x03E00401,    // 248: 4 x 62 = 248
    0x03200501,    // 249: 5 x 50 = 250
    0x03200501,    // 250: 5 x 50 = 250
};


/****************************************************************************
 * Function: L1DCacheInvalidate
 * Description: Invalidate the level 1 Data cache.
        In Cortex A9, there is no cp instruction for invalidating
            the whole D-cache. This function invalidates each line by
        set/way.
 ****************************************************************************/
void L1DCacheInvalidate(void)
{
  register unsigned int CsidReg, C7Reg;
  unsigned int CacheSize, LineSize, NumWays;
  unsigned int Way, WayIndex, Set, SetIndex, NumSet;

  /* Select cache level 0 and D cache in CSSR */
  mtcp(XREG_CP15_CACHE_SIZE_SEL, 0);
  isb();
#ifdef __GNUC__
  CsidReg = mfcp(XREG_CP15_CACHE_SIZE_ID);
#else
  { volatile register unsigned int Reg __asm(XREG_CP15_CACHE_SIZE_ID);
  CsidReg = Reg; }
#endif
  /* Determine Cache Size */
  CacheSize = (CsidReg >> 13) & 0x1FF;
  CacheSize +=1;
  CacheSize *=128;    /* to get number of bytes */

  /* Number of Ways */
  NumWays = (CsidReg & 0x3ff) >> 3;
  NumWays += 1;

  /* Get the cacheline size, way size, index size from csidr */
  LineSize = (CsidReg & 0x07) + 4;

  NumSet = CacheSize/NumWays;
  NumSet /= (1 << LineSize);

  Way = 0UL;
  Set = 0UL;

  /* Invalidate all the cachelines */
  for (WayIndex =0; WayIndex < NumWays; WayIndex++)
  {
    for (SetIndex =0; SetIndex < NumSet; SetIndex++)
    {
      C7Reg = Way | Set;
#ifdef __GNUC__
      /* Invalidate by Set/Way */
      __asm__ __volatile__("mcr " \
          XREG_CP15_INVAL_DC_LINE_SW :: "r" (C7Reg));
#else
      //mtcp(XREG_CP15_INVAL_DC_LINE_SW, C7Reg);
      {
        volatile register unsigned int Reg
          __asm(XREG_CP15_INVAL_DC_LINE_SW);
        Reg = C7Reg;
      }
#endif
      Set += (1 << LineSize);
    }
    Way += 0x40000000;
  }

  /* Wait for L1 invalidate to complete */
  dsb();
}


/****************************************************************************
 * Function: L2CacheInvalidate
 * Description: Invalidate the level 2 cache.
 ****************************************************************************/
void L2CacheInvalidate(void)
{
  register unsigned int L2CCReg;

  /* Invalidate the caches */
  Xil_Out32(XPS_L2CC_BASEADDR + XPS_L2CC_CACHE_INVLD_WAY_OFFSET,
      0x0000FFFF);

  /* Wait for the invalidate to complete */
  do
  {
    L2CCReg = Xil_In32(XPS_L2CC_BASEADDR +
        XPS_L2CC_CACHE_SYNC_OFFSET);
  }
  while (L2CCReg != 0);

  /* synchronize the processor */
  dsb();
}





/****************************************************************************
 * Function: L2CacheFlush
 * Description: Flush the level 2 cache.
 ****************************************************************************/
void L2CacheFlush(void)
{
  register unsigned int L2CCReg;

  /* Flush the caches */
  Xil_Out32(XPS_L2CC_BASEADDR + XPS_L2CC_CACHE_INV_CLN_WAY_OFFSET,
      0x0000FFFF);

  /* Wait for the flush to complete */
  do
  {
    L2CCReg = Xil_In32(XPS_L2CC_BASEADDR +
        XPS_L2CC_CACHE_SYNC_OFFSET);
  }
  while (L2CCReg != 0);

  /* synchronize the processor */
  dsb();
}





/****************************************************************************
 * Function: L1DCacheFlush
 * Description: Flush the level 1 Data cache.
 ****************************************************************************/
void L1DCacheFlush(void)
{
  register unsigned int CsidReg, C7Reg;
  unsigned int CacheSize, LineSize, NumWays;
  unsigned int Way, WayIndex, Set, SetIndex, NumSet;

  /* Select cache level 0 and D cache in CSSR */
  mtcp(XREG_CP15_CACHE_SIZE_SEL, 0);
  isb();
#ifdef __GNUC__
  CsidReg = mfcp(XREG_CP15_CACHE_SIZE_ID);
#else
  {
    volatile register unsigned int Reg __asm(XREG_CP15_CACHE_SIZE_ID);
    CsidReg = Reg;
  }
#endif

  /* Determine Cache Size */
  CacheSize = (CsidReg >> 13) & 0x1FF;
  CacheSize +=1;
  CacheSize *=128;    /* to get number of bytes */

  /* Number of Ways */
  NumWays = (CsidReg & 0x3ff) >> 3;
  NumWays += 1;

  /* Get the cacheline size, way size, index size from csidr */
  LineSize = (CsidReg & 0x07) + 4;

  NumSet = CacheSize/NumWays;
  NumSet /= (1 << LineSize);

  Way = 0UL;
  Set = 0UL;

  /* Invalidate all the cachelines */
  for (WayIndex =0; WayIndex < NumWays; WayIndex++)
  {
    for (SetIndex =0; SetIndex < NumSet; SetIndex++)
    {
      C7Reg = Way | Set;
      /* Flush by Set/Way */
#ifdef __GNUC__
      __asm__ __volatile__("mcr " \
          XREG_CP15_CLEAN_INVAL_DC_LINE_SW :: "r" (C7Reg));
#else
      {
        volatile register unsigned int Reg
          __asm(XREG_CP15_CLEAN_INVAL_DC_LINE_SW);
        Reg = C7Reg;
      }
#endif
      Set += (1 << LineSize);
    }
    Way += 0x40000000;
  }

  /* Wait for L1 flush to complete */
  dsb();
}



/****************************************************************************
 * Function: DCacheInvalidate
 * Description: Invalidate the Data caches.
 ****************************************************************************/
void DCacheInvalidate(void)
{
  dsb();
  L1DCacheFlush();
  L2CacheFlush();
}


/****************************************************************************
 * Function: cache_ctrl
 * Description:
 ****************************************************************************/
void cache_ctrl(int d, int i)
{
  unsigned int cr1;

  asm volatile ("mrc 15,0,%0,cr1,cr0,0":"=r"(cr1));
  if (d)
    cr1 |= 3;
  else
    cr1 &= ~3;

  if (i)
    cr1 |= 1 << 12;
  else
    cr1 &= ~(1 << 12);

  asm volatile ("mcr  15,0,%0,cr1,cr0,0"::"r"(cr1));
}



/****************************************************************************
 * Function: l2_ctrl
 * Description:
 ****************************************************************************/
void l2_ctrl(int val)
{
  REG_WRITE(L2CCCrtl,val);
}




//# - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
//   X A D C    A C C E S S     R O U T I N E S
//# - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -


/****************************************************************************
 * Function: rd_cmd
 * Description:
 ****************************************************************************/
int rd_cmd(int addr)
{
  return (1 << 26) + (addr << 16);
}


/****************************************************************************
 * Function: wr_cmd
 * Description:
 ****************************************************************************/
int wr_cmd(int addr, int data)
{
  return (2 << 26) + (addr << 16) + (data & 0x0ffff);
}


/****************************************************************************
 * Function: voltage
 * Description:
 ****************************************************************************/
double voltage(int x)
{
  return ((double)x*3.0)/(4096.0*16.0);
}


/****************************************************************************
 * Function: temperature
 * Description:
 ****************************************************************************/
double temperature(int x)
{
  return ((double)x*503.975)/(4096.0*16.0) - 273.15;
}


/****************************************************************************
 * Function: get_xadc_regs
 * Description:
 ****************************************************************************/
int get_xadc_regs(int *list, int cnt)
{
  int i;
  for (i=0; i<cnt; i++)
  {
    REG_WRITE(0xf8007110, rd_cmd(list[i]));
  }
  //one extra write
  REG_WRITE(0xf8007110, rd_cmd(list[cnt-1]));

  //# now read, discaring first value
  i = REG_READ(0xf8007114);
  for (i=0; i<cnt; i++)
  {
    dlist[i] = REG_READ(0xf8007114);
  }
  return cnt;
}


/****************************************************************************
 * Function: init_xadc
 * Description:
 ****************************************************************************/
int init_xadc()
{
  int j, cnt, data;
  //# enable xadc IF: setreg(['pele_ps','devcfg','XADCIF_CFG','ENABLE'], 1);
  REG_WRITE(0xf8007100, 0x80001114);
  //# take xadc out of reset: setreg(['pele_ps','devcfg','XADCIF_MCTL'], 0);
  REG_WRITE(0xf8007118, 0);
  //# flush read fifo.  cnt = getreg(['pele_ps','devcfg','XADCIF_MSTS', 'DFIFO_LVL']);
  cnt = REG_READ(0xf800710c);
  cnt = (cnt >> 12) & 0x0f;
  for (j=0; j<cnt; j++)
  {
    data = REG_READ(0xf8007114);  //data = getreg(['pele_ps','devcfg','XADCIF_RDFIFO']);
  }
  //# read xadc config regs
  return data;
}


/****************************************************************************
 * Function: read_xadc
 * Description:
 ****************************************************************************/
int read_xadc()
{
  double tempr, vccint, vccaux, vccbram, vccpint, vccpaux, vccpdro;
  init_xadc();
  get_xadc_regs(&alist[0], 7 );
  tempr   = temperature(dlist[0]);
  vccint  = voltage(dlist[1]);
  vccaux  = voltage(dlist[2]);
  vccbram = voltage(dlist[3]);
  vccpint = voltage(dlist[4]);
  vccpaux = voltage(dlist[5]);
  vccpdro = voltage(dlist[6]);
  printf( "On-Die Temperature = %g | VCCINT = %g | VCCAUX = %g | VCCBRAM = %g | VCCPINT = %g | VCCPAUX = %g | VCCPDRO = %g \r\n",
      tempr,vccint,vccaux,vccbram,vccpint,vccpaux,vccpdro);
  return 0;
}


/****************************************************************************
 * Function: get_xadc_temperature
 * Description:
 ****************************************************************************/
double get_xadc_temperature()
{
  init_xadc();
  get_xadc_regs(&alist[0], 1 );
  return temperature(dlist[0]);
}





//# - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
//   D D R    R O U T I N E S
//# - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

/****************************************************************************
 * Function: unlock_slcr
 * Description:
 ****************************************************************************/
void unlock_slcr()
{
  REG_WRITE(SLCR_LOW_BASE+SLCR_UNLOCK,  SLCR_UNLOCK_VALUE);
}


/****************************************************************************
 * Function: ddr_nonsecure
 * Description:
 ****************************************************************************/
 void ddr_nonsecure()
 {
  // Make DRAM space nonsecure
  REG_WRITE(0xf8000430, 0xffffffff);
}


/****************************************************************************
 * Function: noop
 * Description:
 ****************************************************************************/
int noop(int n)
 {
  // a noop delay loop
  int a,b, i;
  a = 0x12345678;
  b = 0x9f3d1046;
  for (i=0; i<n; i++) {  // wait here for a while
    a = (b >> 5) ^ (b << 6);
    b = (a >> 3) ^ (a << 5);
  }
  return b;
}


/****************************************************************************
 * Function: get_bit_field
 * Description: get a bitfield
 ****************************************************************************/
 int get_bit_field(int x, int bstart, int bwidth)
 {
  int i, mask, dout;
  mask = 0;
  for (i=0; i<bwidth; i++)
    mask = (mask << 1) | 1;
  dout = (x >> bstart) & mask;
  return dout;
}


/****************************************************************************
 * Function: set_bit_field
 * Description: set a bitfield
 ****************************************************************************/
int set_bit_field(int data, int lsb, int width, int val)
{
  int i, mask, sh_mask, dout;
  mask = 0;
  for (i=0; i<width; i++)
    mask = (mask << 1) | 1;
  sh_mask = mask << lsb;
  dout = (data & ~sh_mask) + ((val & mask) << lsb);
  return dout;
}


/****************************************************************************
 * Function: update_reglist
 * Description: update a bitfield in a reglist: addr-data pairs
 ****************************************************************************/
 void update_reglist(int *list, int listsize, int addr, int lsb, int width, int value)
 {

  int i, laddr, ldata;
  for (i=0; i<listsize; i++)
  {
    laddr = list[2*i + 0];
    ldata = list[2*i + 1];
    if (laddr == addr)
    {
      ldata = set_bit_field(ldata,lsb,width,value);
      list[2*i + 1] = ldata;
      break;
    }
  }
}


/****************************************************************************
 * Function: get_reglist_value
 * Description: get a bitfield in a reglist: addr-data pairs
 ****************************************************************************/
int get_reglist_value(int *list, int listsize, int addr, int lsb, int width)
{
  // get a bitfield in a reglist: addr-data pairs
  int i, laddr, ldata;
  ldata = -1;
  for (i=0; i<listsize; i++)
  {
    laddr = list[2*i + 0];
    ldata = list[2*i + 1];
    if (laddr == addr)
    {
      ldata = get_bit_field(ldata,lsb,width);
      break;
    }
  }
  return ldata;
}





/****************************************************************************
 * Function: ddriob_init
 * Description: Init the ddriob
 ****************************************************************************/
#if 0
int ddriob_init()
{
  int i, addr, data;
  int done, k;

  // init regs
  for (i=0; i<15; i++)
  {
    addr = ddriob_reg_values[2*i + 0];
    data = ddriob_reg_values[2*i + 1];
    REG_WRITE(addr, data);
  }
  // wait for dci done
  done = 0;
  k = 0;

  while (done == 0)
  {
    done = (REG_READ(0xf8000b74) >> 13) & 1;
    k += 1;
    if ((k > 200000) && (done == 0))
    {
      done = -1;
    }
  }
  if (done == -1)
  {
    printf("error - DCI done timeout \r\n");
    return 1;
  }
  return 0;
}
#endif


/****************************************************************************
 * Function: ddrc_init
 * Description: Init the DDR controller
 ****************************************************************************/
int ddrc_init()
 {
  int i, addr, data;
  // init regs
  for (i=0; i<80; i++)
  {
    addr = ddrc_reg_values[2*i + 0];
    data = ddrc_reg_values[2*i + 1];
    REG_WRITE(addr, data);
  }
  return 0;
}

/****************************************************************************
 * Function: ddriob_get
 * Description: Get Current state of the ddriob regs
 ****************************************************************************/
int ddriob_get()
{
  int i, addr, data;
  // init regs
  for (i=0; i<15; i++)
  {
    addr = ddriob_reg_values[2*i + 0];
    data = REG_READ(addr);
    ddriob_reg_values[2*i + 1] = data;
    REG_WRITE(addr, data);
  }
  printf("    ** read all ddriob regs \r\n");
  return 0;
}


/****************************************************************************
 * Function: ddrc_get
 * Description: Get Current state of the DDR controller regs
 ****************************************************************************/
int ddrc_get()
{
  int i, addr, data;
  // init regs
  for (i=0; i<80; i++)
  {
    addr = ddrc_reg_values[2*i + 0];
    data = REG_READ(addr);
    ddrc_reg_values[2*i + 1] = data;
  }
  // ctrl reg is read twice, clr the en bit in the 1st instance
  update_reglist(&ddrc_reg_values[0], 80, ddrc_reg_values[0], 0,  1, 0);
  printf("    ** read all ddrc regs \r\n");
  return 0;
}


/****************************************************************************
 * Function: set_drive_strength
 * Description:  Set a drive-strength register
          data_n = dn ^ 0x028;    bit[13:7]
          data_p = dp ^ 0x02c;    bit[6:0]
          do a read-modify-write
 ****************************************************************************/
void set_drive_strength(int addr, int driven, int drivep)
{
  int dat;
  dat = REG_READ(addr);
  dat = set_bit_field(dat, 7, 7, driven ^ 0x028);
  dat = set_bit_field(dat, 0, 7, drivep ^ 0x02c);
  REG_WRITE(addr, dat);
}


/****************************************************************************
 * Function: set_slew_rate
 * Description:  Set a slew rate register
          slew_n = n ;    bit[23:19]  (5 bits) lpddr2 def = 0x1f(31)
          slew_p = p ;    bit[18:14]  (5 bits) lpddr2 def = 0x07( 7)
          do a read-modify-write
 ****************************************************************************/
void set_slew_rate(int addr, int n, int p)
{
  int dat;
  dat = REG_READ(addr);
  dat = set_bit_field(dat, 19, 5, n );
  dat = set_bit_field(dat, 14, 5, p );
  REG_WRITE(addr, dat);
}







//# - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
//   P L L   &   T I M E R    R O U T I N E S
//# - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -


/****************************************************************************
 * Function: timer_init
 * Description:  Initialize the ttc1 timer 1 to measure test duration.
 ****************************************************************************/
int timer_init()
{
  int base;

  base = 0xf8002000;

  // Set clock_control_1 at 0xf8002000:
  //   b6=0 ex_e
  //   b5=0 selects pclk
  //   b4-1 = 15 predivide by 64k
  //   b0=1 prescale enable
  REG_WRITE(base+0,  1 + (15 << 1));  // 0x1f

  // Set counter_control_1
  //   b6=0 wave_pol
  //   b5=1 wave_en active low
  //   b4=1 restarts counter
  //   b3=0 match mode
  //   b2=0 decr mode
  //   b1=0 overflow mode
  //   b0=0 disable counter
  REG_WRITE(base+0x0c,  (1 << 4) + (1 << 5));  // 0x30
  return 0;
}


/****************************************************************************
 * Function: timer_read
 * Description:  Read the timer
 ****************************************************************************/
int timer_read()
{
  int base, a;
  base = 0xf8002000;
  a = REG_READ(base+0x18);
  return a;
}


/****************************************************************************
 * Function: timer_value
 * Description:  returns timer value in seconds
          t in timer units, fpclk in mhz
 ****************************************************************************/
double timer_value(int t, double fpclk)
{
  double x;
  x = ((double) t) * 65536.0 / (fpclk * 1000000.0);
  return x;
}



/****************************************************************************
 * Function: my_set_pll
 * Description:  PLL init routine
          pll: 0=cpu, 1=ddr, 2=IOU
 ****************************************************************************/
int my_set_pll(int pll, int fbdiv, int outdiv, double fref)
{
  int pll_ctrl, pll_cfg, pll_stat;
  int val, sh_mask, cval, i, k, locked;
  double fvco, fout;

  fout = fref * (double) fbdiv / (double) outdiv;

  pll_stat = SLCR_LOW_BASE + PLL_STATUS;

  switch (pll)
  {
    case 0: pll_ctrl = SLCR_LOW_BASE + ARM_PLL_CTRL;
        pll_cfg  = SLCR_LOW_BASE + ARM_PLL_CFG;
        break;

    case 1: pll_ctrl = SLCR_LOW_BASE + DDR_PLL_CTRL;
        pll_cfg  = SLCR_LOW_BASE + DDR_PLL_CFG;
        break;

    default: pll_ctrl = SLCR_LOW_BASE + IO_PLL_CTRL;
        pll_cfg  = SLCR_LOW_BASE + IO_PLL_CFG;
        break;
  }

  // 0. unlock slcr
  unlock_slcr();

  // 1. put the pll in bypass
  val = REG_READ(pll_ctrl) | (1 << bit_PLL_BYPASS_FORCE);
  REG_WRITE(pll_ctrl, val);

  // 2. put the pll in reset
  val = val | (1 << bit_PLL_RESET);
  REG_WRITE(pll_ctrl, val);

  // 3. set the new feedbak divider value
  sh_mask = mask_PLL_FDIV << bit_PLL_FDIV;
  val = (val & ~sh_mask) + ((fbdiv & mask_PLL_FDIV) << bit_PLL_FDIV);
  REG_WRITE(pll_ctrl, val);

  // 4. set the new cp,res,cnt
  i = min(99, fbdiv);
  cval = pll_config_table[i];
  REG_WRITE(pll_cfg, cval);

  // 5. take out of reset
  val = val ^ (1 << bit_PLL_RESET);
  REG_WRITE(pll_ctrl, val);

  // 6. poll for lock. the lock bit index is 0=cpu, 1=ddr, 2=io
  locked = 0;
  k = 0;
  while (locked == 0)
  {
    locked = (REG_READ(pll_stat) >> pll) & 1;
    k += 1;
    if ((k > 2000) && (locked == 0))
    {
      locked = -1;
    }
  }
  if (locked == -1)
  {
    printf("ERROR: PLL lock timeout - leaving PLL in bypass\r\n");
    return 0;
  }
  locked = k;

  // 7. set output divider.
  if (pll == 1)
  { // ddr, [25:20]
    cval = REG_READ(SLCR_LOW_BASE + DDR_CLK_CTRL);
    sh_mask = 0x3f << 20;
    cval = (cval & ~sh_mask) | ((outdiv & 0x3f) << 20);
    REG_WRITE(SLCR_LOW_BASE + DDR_CLK_CTRL, cval);
  }
  else if (pll == 0)
  { // cpu, [13:8]
    cval = REG_READ(SLCR_LOW_BASE + CPU_CLK_CTRL);
    sh_mask = 0x3f << 8;
    cval = (cval & ~sh_mask) | ((outdiv & 0x3f) << 8);
    REG_WRITE(SLCR_LOW_BASE + CPU_CLK_CTRL, cval);
  }
  else
  { // IO
  }

  // 8. take out of bypass.
  val = val ^ (1 << bit_PLL_BYPASS_FORCE);
  REG_WRITE(pll_ctrl, val);
  noop( 1000000 );  // wait a while
  printf("**** Run my_set_pll.\r\n");
  printf("**** OK, %s PLL is set to %g MHz \r\n", pll_id[pll], fout);
  printf("**** PLL locked after %d polls \r\n", locked);

  // if ddr, set DCI clock to 10MHz, it has 2 6-bit divisors. vco can be 800-1600
  // so the total divisor can be 80-160
  if (pll == 1)
  {
    fvco = fref * (double)fbdiv;
    val = (int) (fvco / 10.0);
    k = lim(val, 70, 250);
    cval = dci_ctrl_table[k-70];
    REG_WRITE(SLCR_LOW_BASE + DCI_CLK_CTRL, cval);
    printf("**** OK - changed DCI divisor\r\n");
  }

  if (pll == 0)
    pclk = fout * 0.16666;  // update pclk for timer

  return 0;
}


/****************************************************************************
 * Function: calc_value
 * Description:  calc next lfsr value
          Based on XAPP052: for 32 bit: 32, 22, 2, 1
                              for 31 bit: 31, 28
              do XNOR.
          In verilog in would be:
          lfsr = {lfsr[30:0] , lfsr_xnor = (lfsr[30] ^ lfsr[27]) ? 1'd0 : 1'd1;}
          In one line:
             ((a << 1) + (((a >> 30) & 1) ^ ((a >> 27) & 1) ^ 1))
 ****************************************************************************/
int calc_value(int lfsr)
{
  int lsb, rslt;
  lsb = ((lfsr >> 30) & 1) ^ ((lfsr >> 27) & 1);
  lsb = lsb ^ 1;
  rslt = ((lfsr << 1) + lsb);  // & 0x0ffffffff;
  return rslt;
}









//# - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
//   M E M O R Y   T E S T S    R O U T I N E S
//# - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void printMemTestHeader(void)
{
  if (verbose & 3)
  {
      printf("------------------------------------------------------------------------------------------\n");
      printf("    TEST           WORD ERROR             PER-BYTE-LANE ERROR COUNT              TIME\n");
      printf("                     COUNT        [ LANE-0 ] [ LANE-1 ] [ LANE-2 ] [ LANE-3 ]    (sec)\n");
      printf("------------------------------------------------------------------------------------------\n");
  }
}

void printEyeTestHeader(void)
{
  printf("\n[128 units = 1 bit time (ideal eye width)] \r\n");
  printf("--------------------------------------------------------\n");
  printf("Description        LANE-0    LANE-1    LANE-2    LANE-3    \n");
  printf("--------------------------------------------------------\n");
}

void printDriveTestHeader(void)
{
  printf("\n[128 units = 1 bit time (ideal eye width)] \r\n");
  printf("--------------------------------------------------------------\n");
  printf("SNo. Drive [n,p]        LANE-0    LANE-1    LANE-2    LANE-3    \n");
  printf("--------------------------------------------------------------\n");
}


/****************************************************************************
 * Function: memtest_readonly
 * Description:
 ****************************************************************************/
int memtest_readonly(int start, int size)
{
  int i, addr;
  for (i=0; i<size; i+=4)
  {
    addr = start + i;
    REG_READ(addr);
  }
  return 0;
}


/****************************************************************************
 * Function: error_info
 * Description:
 ****************************************************************************/
void error_info(int addr, int wrdat, int rddat)
{
  int n;
  // put in xregv mailbox regs
  if (werr <= 1)
  {  // init or 1st error
    REG_WRITE(ERR_INFO,   addr);
    REG_WRITE(ERR_INFO+4, wrdat);
    REG_WRITE(ERR_INFO+8, rddat);
  }
  // also store first 80 errors in err_buf
  if (werr <= 80)
  {
    n = werr - 1;
    err_buf[3*n]   = addr;
    err_buf[3*n+1] = wrdat;
    err_buf[3*n+2] = rddat;
  }
}


/****************************************************************************
 * Function: memtest_simple
 * Description:  Do a simple memory test
          start and size are in bytes
          if loop==-1, do write only
 ****************************************************************************/
int memtest_simple(int start, int size, int mode, int loop, int d0, int d1, int d2, int d3)
{
  int i, j, addr, data, ref;
  int merr = 0;    // per test word errors
  int lerrcnt[4];  // local errcnt per test
  int dat[4];

  dat[0] = d0 & qmask;
  dat[1] = d1 & qmask;
  dat[2] = d2 & qmask;
  dat[3] = d3 & qmask;

  for (i=0; i<4; i++)
    lerrcnt[i] = 0;

  if (memtest_stat)
    REG_WRITE(MAILBOX_STAT, 100+mode);

  timer_init();

  // write then read memory
  for (i=0; i<size; i+=16)
  {
    addr = start + i;
    REG_WRITE(addr,    dat[0]);
    REG_WRITE(addr+4,  dat[1]);
    REG_WRITE(addr+8,  dat[2]);
    REG_WRITE(addr+12, dat[3]);
    //progress bar, print dot every 32MB if the total size > 32MB
    if((i%0x4000000==0) && (size>0x4000000))
    {
      fflush(stdout);
      printf(".");
    }
  }

  DCacheInvalidate();
  if (loop == -1)
    return 0;

  for (i=0; i<size; i+=16)
  {
    addr = start + i;
    for (j=0; j<4; j++)
    {
      //progress bar, print dot every 32MB if the total size > 32MB
      if((i%0x4000000==0) && (size>0x4000000))
      {
        fflush(stdout);
        printf(".");
      }
      data = REG_READ(addr+4*j);
      ref = dat[j];
      if (data != ref)
      {
        werr++;
        merr++;

        if ((data & 0x000000ff) != (ref & 0x000000ff))
          lerrcnt[0]++;
        if ((data & 0x0000ff00) != (ref & 0x0000ff00))
          lerrcnt[1]++;
        if ((data & 0x00ff0000) != (ref & 0x00ff0000))
          lerrcnt[2]++;
        if ((data & 0xff000000) != (ref & 0xff000000))
          lerrcnt[3]++;

        if ((verbose & 8) && (merr <= 10))
          printf("\rMemtest_s ERROR: addr=0x%X rd/ref/xor = 0x%08X 0x%08X 0x%08X \r\n", addr+4*j, data, ref, data ^ ref);

        error_info(addr+4+j, ref, data);
      }
    } // j

    if (merr != 0)
    {
      if ((lerrcnt[0] > 1000) && (lerrcnt[1] > 1000) && (lerrcnt[2] > 1000) && (lerrcnt[3] > 1000))
        break;  // to save time when there are lots of errors
    }
  } // i

  i = timer_read();
  test_time = timer_value(i, pclk);
  total_test_time += test_time;

  if (verbose & 1)
  {
    printf("\rMemtest_s (%3d:%2d)      %d         [%8d] [%8d] [%8d] [%8d]    %g\n",
        loop, mode, merr, lerrcnt[0], lerrcnt[1], lerrcnt[2], lerrcnt[3],
        test_time);
  }
  //printf(".");

  // add local to global
  for (i=0; i<4; i++)
    errcnt[i] += lerrcnt[i];

  if (merr > 0)
    epp += 1 << mode;

  return 0;
}



/****************************************************************************
 * Function: memtest_lfsr
 * Description: Do a simple memory test
        start and size are in bytes
 ****************************************************************************/
int memtest_lfsr(int start, int size, int mode, int loop)
{
  int i, addr, data, ref;
  int merr = 0;   // per test word errors
  int randval;
  int lerrcnt[4]; // local errcnt per test

  // change random seed
  rseed += 0x017c1e23;

  for (i=0; i<4; i++)
    lerrcnt[i] = 0;

  if (memtest_stat)
    REG_WRITE(MAILBOX_STAT, 100+mode);

  timer_init();

  // write then read memory
  // wr
  randval = 0x12345678 + loop + 19*mode + rseed;
  for (i=0; i<size; i+=4)
  {
    addr = start + i;
    randval = YLFSR(randval);
    ref = randval & qmask;
    REG_WRITE(addr, ref);
    //progress bar, print dot every 32MB if the total size > 32MB
    if((i%0x2000000==0) && (size>0x2000000))
    {
      fflush(stdout);
      printf(".");
    }
  }

  DCacheInvalidate();

  // rd
  randval = 0x12345678 + loop + 19*mode + rseed;
  for (i=0; i<size; i+=4)
  {
    //progress bar, print dot every 32MB if the total size > 32MB
    if((i%0x2000000==0) && (size>0x2000000))
    {
      fflush(stdout);
      printf(".");
    }
    addr = start + i;
    randval = YLFSR(randval);
    ref = randval & qmask;
    data = REG_READ(addr);
    if (data != ref)
    {
      werr++;
      merr++;
      if ((data & 0x000000ff) != (ref & 0x000000ff))
        lerrcnt[0]++;
      if ((data & 0x0000ff00) != (ref & 0x0000ff00))
        lerrcnt[1]++;
      if ((data & 0x00ff0000) != (ref & 0x00ff0000))
        lerrcnt[2]++;
      if ((data & 0xff000000) != (ref & 0xff000000))
        lerrcnt[3]++;

      if ((verbose & 8) && (merr <= 10))
        printf("\rMemtest_l ERROR: addr=0x%X rd/ref/xor = 0x%08X 0x%08X 0x%08X \r\n", addr, data, ref, data ^ ref);

      error_info(addr, ref, data);
    }
    if (merr != 0) {
      if ((lerrcnt[0] > 1000) && (lerrcnt[1] > 1000) && (lerrcnt[2] > 1000) && (lerrcnt[3] > 1000))
        break;  // to save time when there are lots of errors
    }
  } // i

  i = timer_read();
  test_time = timer_value(i, pclk);
  total_test_time += test_time;
  if (verbose & 1)
  {
    printf("\rMemtest_l (%3d:%2d)      %d         [%8d] [%8d] [%8d] [%8d]    %g\n",
        loop, mode, merr, lerrcnt[0], lerrcnt[1], lerrcnt[2], lerrcnt[3],
        test_time );
  }
  //printf(".");
  // add local to global
  for (i=0; i<4; i++)
    errcnt[i] += lerrcnt[i];

  if (merr > 0)
    epp += 1 << mode;

  return 0;
}


/****************************************************************************
 * Function: memtest_pat128
 * Description: Do a simple memory test using a 128-word pattern
        start and size are in bytes
 ****************************************************************************/
int memtest_pat128(int start, int size, int mode, int loop, int *pat)
{
  int i, addr, data, ref, mod128;
  int merr = 0;   // per test word errors
  int lerrcnt[4];  // local errcnt per test

  for (i=0; i<4; i++)
    lerrcnt[i] = 0;
  if (memtest_stat)
    REG_WRITE(MAILBOX_STAT, 100+mode);

  timer_init();

  // write then read memory
  // wr
  for (i=0; i<size; i+=4)
  {
    mod128 = (i >> 2) & 0x07f;
    addr = start + i;
    ref = pat[mod128] & qmask;
    REG_WRITE(addr, ref);
    //progress bar, print dot every 32MB if the total size > 32MB
    if((i%0x2000000==0) && (size>0x2000000))
    {
      fflush(stdout);
      printf(".");
    }

  }
  DCacheInvalidate();

  // rd
  for (i=0; i<size; i+=4)
  {
    //progress bar, print dot every 32MB if the total size > 32MB
    if((i%0x2000000==0) && (size>0x2000000))
    {
      fflush(stdout);
      printf(".");
    }
    mod128 = (i >> 2) & 0x07f;
    addr = start + i;
    ref = pat[mod128] & qmask;
    data = REG_READ(addr);
    if (data != ref)
    {
      werr++;
      merr++;
      if ((data & 0x000000ff) != (ref & 0x000000ff))
        lerrcnt[0]++;
      if ((data & 0x0000ff00) != (ref & 0x0000ff00))
        lerrcnt[1]++;
      if ((data & 0x00ff0000) != (ref & 0x00ff0000))
        lerrcnt[2]++;
      if ((data & 0xff000000) != (ref & 0xff000000))
        lerrcnt[3]++;

      if ((verbose & 8) && (merr <= 10))
        printf("\rMemtest_p ERROR: addr=0x%X rd/ref/xor = 0x%08X 0x%08X 0x%08X \r\n", addr, data, ref, data ^ ref);

      error_info(addr, ref, data);
    }

    if (merr != 0)
    {
      if ((lerrcnt[0] > 1000) && (lerrcnt[1] > 1000) && (lerrcnt[2] > 1000) && (lerrcnt[3] > 1000))
        break;  // to save time when there are lots of errors
    }
  } // i

  i = timer_read();
  test_time = timer_value(i, pclk);
  total_test_time += test_time;

  if (verbose & 1)
  {
    printf("\rMemtest_p (%3d:%2d)      %d         [%8d] [%8d] [%8d] [%8d]    %g\n",
        loop,mode, merr, lerrcnt[0], lerrcnt[1], lerrcnt[2], lerrcnt[3],
        test_time );
  }
  //printf(".");


  // add local to global
  for (i=0; i<4; i++)
    errcnt[i] += lerrcnt[i];
  if (merr > 0)
    epp += 1 << mode;

  return 0;
}



// -------------------------------------------

// per bit test pattern, repeats every 16 words.
// __-__-_-   --_--_-_
// then every 16 words invert a different bit per byte.
int pattern1[16] = { // for a 32-bit memory
    0x00000000, 0x00000000, 0xFFFFFFFF, 0x00000000,
    0x00000000, 0xFFFFFFFF, 0x00000000, 0xFFFFFFFF,
    0xFFFFFFFF, 0xFFFFFFFF, 0x00000000, 0xFFFFFFFF,
    0xFFFFFFFF, 0x00000000, 0xFFFFFFFF, 0x00000000
};

int pattern1_16bit[16] = { // for a 16-bit memory
    0x00000000, 0x0000FFFF, 0xFFFF0000, 0xFFFF0000,
    0xFFFFFFFF, 0xFFFF0000, 0x0000FFFF, 0x0000FFFF,
    0x00000000, 0x0000FFFF, 0xFFFF0000, 0xFFFF0000,
    0xFFFFFFFF, 0xFFFF0000, 0x0000FFFF, 0x0000FFFF
};

int invertmask[8] = {
    0x01010101, 0x02020202, 0x04040404, 0x08080808,
    0x10101010, 0x20202020, 0x40404040, 0x80808080
};

// repeating 128-word patterns
int pat1[128];
int pat2[128];

// Test Modes
//  0 - data = addr
//  1 - data = 00
//  2 - data = FF
//  3 - data = AA
//  4 - data = 55
//  5 - data = 00 FF
//  6 - data = FF 00
//  7 - data = 55 aa
//  8 - data = aa 55
//  9 - data = pattern1 without bit inversion
// 10 - data = pattern1 WITH    bit inversion
// 11 - data = p-random lfsr
// -------------------------------------------



/****************************************************************************
 * Function: memtest
 * Description: Do a simple memory test
        start and size are in bytes
 ****************************************************************************/
int memtest(int start, int size, int mode, int loop)
{
  int i, rd, addr, data, ref, mod16, imaski;
  int merr = 0;   // per test word errors
  int randval;
  int lerrcnt[4];  // local errcnt per test


  for (i=0; i<4; i++)
    lerrcnt[i] = 0;
  if (memtest_stat)
    REG_WRITE(MAILBOX_STAT, 100+mode);

  timer_init();

  // write then read memory
  for (rd=0; rd<2; rd++)
  {  // 0=wr, 1=rd
    randval = 0x12345678 + loop + 19*mode;

    for (i=0; i<size; i+=4)
    {
      mod16 = (i >> 2) & 0x0f;
      imaski = (i >> 6) & 0x07;  // invert mask index
      addr = start + i;
      if (mode == 0)
        ref = addr & qmask;
      else if (mode == 1)
        ref = 0;
      else if (mode == 2)
        ref = 0xffffffff;
      else if (mode == 3)
        ref = 0xAAAAAAAA;
      else if (mode == 4)
        ref = 0x55555555;
      else if (mode == 5)
        ref = (i & 4) ? 0xffffffff : 0;
      else if (mode == 6)
        ref = (i & 4) ? 0 : 0xffffffff;
      else if (mode == 7)
        ref = (i & 4) ? 0xaaaaaaaa : 0x55555555;
      else if (mode == 8)
        ref = (i & 4) ? 0x55555555 : 0xaaaaaaaa;
      else if (mode == 9)
        ref = pattern1[mod16];
      else if (mode == 10)
        ref = pattern1[mod16] ^ invertmask[imaski];
      else
      {
        // random
        randval = calc_value(randval);
        ref = randval;
      }

      if (rd == 0)
      {
        REG_WRITE(addr, ref);
        //progress bar, print dot every 32MB if the total size > 32MB
        if((i%0x2000000==0) && (size>0x2000000))
        {
          fflush(stdout);
          printf(".");
        }
      }
      else // read and compare
      {
        //progress bar, print dot every 32MB if the total size > 32MB
        if((i%0x2000000==0) && (size>0x2000000))
        {
          fflush(stdout);
          printf(".");
        }
        data = REG_READ(addr);
        if (data != ref)
        {
          werr++;
          merr++;
          if ((data & 0x000000ff) != (ref & 0x000000ff))
            lerrcnt[0]++;
          if ((data & 0x0000ff00) != (ref & 0x0000ff00))
            lerrcnt[1]++;
          if ((data & 0x00ff0000) != (ref & 0x00ff0000))
            lerrcnt[2]++;
          if ((data & 0xff000000) != (ref & 0xff000000))
            lerrcnt[3]++;

          if ((verbose & 8) && (merr <= 10))
            printf("\rMemtest_0 ERROR: addr=0x%X rd/ref/xor = 0x%08X 0x%08X 0x%08X \r\n", addr, data, ref, data ^ ref);

          error_info(addr, ref, data);
        }

        if ((lerrcnt[0] > 1000) && (lerrcnt[1] > 1000) && (lerrcnt[2] > 1000) && (lerrcnt[3] > 1000))
          break;  // to save time when there are lots of errors
      } // read and compare
    } // i
    DCacheInvalidate();
  }  // rd

  i = timer_read();
  test_time = timer_value(i, pclk);
  total_test_time += test_time;

  if (verbose & 1)
  {
    printf("\rMemtest_0 (%3d:%2d)      %d         [%8d] [%8d] [%8d] [%8d]    %g\n",
        loop,mode, merr, lerrcnt[0], lerrcnt[1], lerrcnt[2], lerrcnt[3],
        test_time);
  }

  // add local to global
  for (i=0; i<4; i++)
    errcnt[i] += lerrcnt[i];
  if (merr > 0)
    epp += 1 << mode;
  return 0;
}



/****************************************************************************
 * Function: memtest_all
 * Description: Do a simple memory test
        start and size are in bytes
 ****************************************************************************/
int memtest_all(int test_start, int test_size, int sel, int lp)
{
  int j, rc=0;
  werr = 0;
  epp  = 0;
  error_info(0, 0, 0);  // init to 0
  REG_WRITE(ERR_INFO+16, (int)&err_buf[0]);   // put ptr to err_buf in err_info+16

  for (j=0; j<250; j++)
    err_buf[j] = 0;
  for (j=0; j<4; j++)
    errcnt[j] = 0;

  total_test_time = 0.0;

  if (sel & 0x0001)
    rc = memtest(test_start, test_size, 0, lp);
  if (sel & 0x0002)
    rc = memtest_simple(test_start, test_size, 1, lp, 0,0,0,0);
  if (sel & 0x0004)
    rc = memtest_simple(test_start, test_size, 2, lp, 0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF);
  if (sel & 0x0008)
    rc = memtest_simple(test_start, test_size, 3, lp, 0xAAAAAAAA,0xAAAAAAAA,0xAAAAAAAA,0xAAAAAAAA);
  if (sel & 0x0010)
    rc = memtest_simple(test_start, test_size, 4, lp, 0x55555555,0x55555555,0x55555555,0x55555555);
  if (bus_width == 32)
  {
    if (sel & 0x0020)
      rc = memtest_simple(test_start, test_size, 5, lp, 0,0xFFFFFFFF,0,0xFFFFFFFF);
    if (sel & 0x0040)
      rc = memtest_simple(test_start, test_size, 6, lp, 0xFFFFFFFF,0,0xFFFFFFFF,0);
    if (sel & 0x0080)
      rc = memtest_simple(test_start, test_size, 7, lp, 0x55555555,0xAAAAAAAA,0x55555555,0xAAAAAAAA);
    if (sel & 0x0100)
      rc = memtest_simple(test_start, test_size, 8, lp, 0xAAAAAAAA,0x55555555,0xAAAAAAAA,0x55555555);
  }
  else
  {
    if (sel & 0x0020)
      rc = memtest_simple(test_start, test_size, 5, lp, 0xFFFF0000,0xFFFF0000,0xFFFF0000,0xFFFF0000);
    if (sel & 0x0040)
      rc = memtest_simple(test_start, test_size, 6, lp, 0x0000FFFF,0x0000FFFF,0x0000FFFF,0x0000FFFF);
    if (sel & 0x0080)
      rc = memtest_simple(test_start, test_size, 7, lp, 0xAAAA5555,0xAAAA5555,0xAAAA5555,0xAAAA5555);
    if (sel & 0x0100)
      rc = memtest_simple(test_start, test_size, 8, lp, 0x5555AAAA,0x5555AAAA,0x5555AAAA,0x5555AAAA);
  }
  if (sel & 0x0200)
    rc = memtest_pat128(test_start, test_size,  9, lp, &pat1[0]);
  if (sel & 0x0400)
    rc = memtest_pat128(test_start, test_size, 10, lp, &pat2[0]);
  if (sel & 0x0800)
    rc = memtest_lfsr(test_start, test_size, 11, lp);
  if (sel & 0x1000)
    rc = memtest_lfsr(test_start, test_size, 12, lp);
  if (sel & 0x2000)
    rc = memtest_lfsr(test_start, test_size, 13, lp);
  if (sel & 0x4000)
    rc = memtest_lfsr(test_start, test_size, 14, lp);

  REG_WRITE(ERR_INFO+24, epp);   // put epp in err_info+24

  return rc;
}


/****************************************************************************
 * Function: measure_write_eye
 * Description: Measure DDR write eye.
        Works for both auto and manual training.
              (x0,x1) is the search range, step is step size.
        Assumes that:
         - ddrc is initialized, wr_data_offset values are set to
               default.
         - ddriob is configured
         - ddr pll is configured
        Need to modify 2 regs per lane:
        R046_reg_phy_dq_offset = cfg.wr_data_offset[0]; //  R046[30:24]
        R05F_reg_phy_wr_data_slave_ratio = R055_reg_phy_wr_dqs_slave_ratio
                                            + cfg.wr_data_offset[0]; // R05F[9:0]

        ** UPDATED to start in the middle till ends fail,
           no longer uses x0,x1,
 ****************************************************************************/
int measure_write_eye(int x0, int x1, int step, int *result, double scl, int sz_scl, int fast, int fix_ctr, int save)
{
  int i, j, k, dqs, sel, rc, lp, cc, ww, m;
  int mmax, mstep;
  int test_start, test_size;
  int mineye[4] = {9999,9999,9999,9999};
  int maxeye[4] = {-9999,-9999,-9999,-9999};

  memtest_stat = 0;
  rc = 0;
  lp = 0;
  test_start = 1024*1024;
  test_size  = 1024*1024*sz_scl;

  // save regs
  for (i=0; i<2*80; i++)
    ddrc_reg_values_save[i] = ddrc_reg_values[i];

  // mmax, mstep;
  if (step == 2)
  {
    mmax = 32;
    mstep = 2;
  }
  else if (step == 1)
  {
    mmax = 64;
    mstep = 1;
  }
  else
  {  // step=4 is default
    mmax = 16;
    mstep = 4;
  }

  // measure
  for (k=0; k<2; k++)
  {
    for (m=0; m<=mmax; m++)
    {
      if (k == 0)
        i = 64 + mstep*m;
      else
        i = 60 - mstep*m;

      if ((i < 0) || (i > 128))
        break;

      REG_WRITE(MAILBOX_STAT, i);

      // load new offset values
      for (j=0; j<4; j++)
      {
        update_reglist(&ddrc_reg_values[0], 80, R46 + 4*j, 24,  7, i);
        dqs = get_reglist_value(&ddrc_reg_values[0], 80, R55 + 4*j, 0, 10);
        update_reglist(&ddrc_reg_values[0], 80, R5F + 4*j,  0, 10, dqs+i);
      }

      if (verbose & 4)
        printf(" after load offset %d \r\n", i);

      // run ddr init
      ddrc_init();
      if (verbose & 4)
        printf(" after ddrc_init %d \r\n", i);

      // wait a while
      noop(1000000);

      if (verbose & 4)
        printf(" after noop %d \r\n", i);

      // run a memory test
      if (fast_is_bc)
        sel = (fast) ? 0x0180 : 0x0E00;
      else
        sel = (fast) ? 0x0E00 : 0x1E60;

      rc = memtest_all(test_start, test_size, sel, lp);

      if (verbose & 2)
      {
        printf("\rTest offset %3d   %8d        [%8d] [%8d] [%8d] [%8d]    %g\n",
          i, werr, errcnt[0], errcnt[1], errcnt[2], errcnt[3], total_test_time);
      }

      // check results
      for (j=0; j<4; j++)
      {
        if (errcnt[j] == 0)
        {
          mineye[j] = min(mineye[j], i);
          maxeye[j] = max(maxeye[j], i);
        }
      }

      // break if all errors already at the end
      if ((errcnt[0] > 0) && (errcnt[1] > 0) && (errcnt[2] > 0) && (errcnt[3] > 0))
        break;
    }  // m
  }  // k

  // restore settings to default, but don't re-init
  for (i=0; i<2*80; i++)
    ddrc_reg_values[i] = ddrc_reg_values_save[i];

  // provide the result: min max interleaved
  if(verbose & 2)
  {
    printf("\nWrite Eye Result:");
    printEyeTestHeader();
    printf("EYE [MIN-MAX]  :  ");
  }

  for (j=0; j<4; j++)
  {
    result[2*j + 0] = mineye[j];
    result[2*j + 1] = maxeye[j];
    printf("[%d,%3d]  ", mineye[j], maxeye[j]);
  }
  printf("\n");
    // Save result for xregv access. b[15:8]=maxeye, b[7:0]=mineye.
  if (save)
  {
    for (j=0; j<4; j++)
    {
      cc = mineye[j];
      ww = min(maxeye[j], 255);

      if ((cc > 255) || (ww < 0))
        m = 0;
      else
        m = cc + (ww << 8);

      REG_WRITE(MAILBOX_RESULT + 4*j, m);
    }
  }
  REG_WRITE(MAILBOX_RESULT + 4*4, 0);   // 1 for read, 0 for wr

  // width and center
  if (verbose & 0x20)
  {

    printf("EYE CENTER     :   ");
    for (j=0; j<4; j++)
    {
      cc = (mineye[j] + maxeye[j]) >> 1;  // center
      printf("%d/128    ", cc);
    }
    printf("\nEYE WIDTH      :   ");
    for (j=0; j<4; j++)
    {
      ww = maxeye[j] - mineye[j];         // width
      printf("%3.2f%%    ", (double)ww*100/128);
    }
    printf("\nEYE ADJUSTED   :      ");
    for (j=0; j<4; j++)
    {
      printf("%d         ", fix_ctr);  //adjusted
    }
    if (fix_ctr)
    {  // adjust the center
      update_reglist(&ddrc_reg_values[0], 80, R46 + 4*j, 24,  7, cc);
      dqs = get_reglist_value(&ddrc_reg_values[0], 80, R55 + 4*j, 0, 10);
      update_reglist(&ddrc_reg_values[0], 80, R5F + 4*j,  0, 10, dqs+cc);
    }

  }
  // done
  // csv printout
  if(csv_mode)
  {
    printf("\nfor_csv , %g , %d , %d , %d , %d , %d , %d , %d , %d  \r\n",
        scl, mineye[0], maxeye[0], mineye[1], maxeye[1], mineye[2], maxeye[2],
        mineye[3], maxeye[3] );
  }
  memtest_stat = 1;
  return rc;
}



/****************************************************************************
 * Function: measure_read_eye
 * Description: Measure read eye using manual rd dqs
        Works for both auto and manual training.
        Assumes that:
         - ddriob is configured
         - ddrc is initialized
         - ddr pll is configured

        1. To disable rd data training:
          - set R02C[28] to 0    (R02C_reg_ddrc_dfi_rd_data_eye_train)
          - set R065[16] to 0    (R065_reg_phy_use_rd_data_eye_level)

        2. then vary R050-053 rd_dqs_slave_ratio 0-128.
              (default is 0x35)  bits[9:0]

        Added: support manual rd gate training for ddr2 etc.
          - need to save/restore rd_gate_slave_ratios
          - vary it as sum of rd_gate+rd_dqs

  245: R05A_reg_phy_fifo_we_slave_ratio  R05A[10:0] , type=rw , defval=0x40
  151: R02C_reg_ddrc_dfi_rd_dqs_gate_level  R02C[27] , type=rw , defval=0x0
 ****************************************************************************/
int measure_read_eye(int *result, int test_start, int test_size, int fast, int istep)
{
  // prepare step 1
  int ctrl_reg = REG_READ(R00);
  int reg_2c   = REG_READ(R2C);
  int reg_65   = REG_READ(R65);
  int reg_50[4];                      // rd_dqs_slave_ratio
  int reg_5a[4];                      // fifo_we_slave_ratio + rd_dqs_slave_ratio
  int fwsr[4];                        // the net manual computed fifo_we_slave_ratio
  int mineye[4] = {9999,9999,9999,9999};
  int maxeye[4] = {-9999,-9999,-9999,-9999};
  int i, j, k, m, dqs_ratio, sel, rc, cc, ww;   // rd_gate_auto;
  int mmax, mstep;
  double scl = 1.0;

  memtest_stat = 0;

  // save regs
  for (j=0; j<4; j++)
  {
    reg_50[j] = REG_READ(R50 + 4*j);
    reg_5a[j] = REG_READ(R5A + 4*j);
    fwsr[j] = reg_5a[j] - reg_50[j];
  }

  // disable_ddrc(setreg);
  REG_WRITE(R00, ctrl_reg & 0xfffffffe);
  REG_WRITE(R2C, reg_2c & ~(1 << 28)  );
  REG_WRITE(R65, reg_65 & ~(1 << 16)  );

  // mmax, mstep;
  if (istep == 2)
  {
    mmax = 32;
    mstep = 2;
  }
  else if (istep == 1)
  {
    mmax = 64;
    mstep = 1;
  }
  else
  {  // step=4 is default
    mmax = 16;
    mstep = 4;
  }

  for (k=0; k<2; k++)
  {
    for (m=0; m<=mmax; m++)
    {
      if (k == 0)
        i = 64 + mstep*m;
      else
        i = 60 - mstep*m;

      if ((i < 0) || (i > 128))
        break;
      dqs_ratio = i;

      REG_WRITE(MAILBOX_STAT, i);

      REG_WRITE(R00, ctrl_reg & 0xfffffffe);

      for (j=0; j<4; j++)
      {
        REG_WRITE(R50 + 4*j, dqs_ratio );  // always needed
        REG_WRITE(R5A + 4*j, dqs_ratio + fwsr[j] );  // needed for ddr2 or manual
      }
      REG_WRITE(R00, ctrl_reg);

      // wait a while
      noop(1000000);

      // run a memory test
      if (fast_is_bc)
        sel = (fast) ? 0x0180 : 0x0E00;
      else
        sel = (fast) ? 0x0E00 : 0x1E60;

      rc = memtest_all(test_start, test_size, sel, 0);

      if (verbose & 2)
      {
        printf("\rTest offset %3d   %8d        [%8d] [%8d] [%8d] [%8d]    %g\n",
          dqs_ratio, werr, errcnt[0], errcnt[1], errcnt[2], errcnt[3], total_test_time);
      }

      // check results
      for (j=0; j<4; j++)
      {
        if (errcnt[j] == 0)
        {
          mineye[j] = min(mineye[j], dqs_ratio);
          maxeye[j] = max(maxeye[j], dqs_ratio);
        }
      }

      // break if all errors already at the end
      if ((errcnt[0] > 0) && (errcnt[1] > 0) && (errcnt[2] > 0) && (errcnt[3] > 0))
        break;
    }
  }

  // Restore regs and re-init
  REG_WRITE(R00, ctrl_reg & 0xfffffffe);  // disable ddrc
  REG_WRITE(R2C, reg_2c );
  REG_WRITE(R65, reg_65 );

  for (j=0; j<4; j++)
  {
    REG_WRITE(R50 + 4*j, reg_50[j] );
    REG_WRITE(R5A + 4*j, reg_5a[j] );
  }

  REG_WRITE(R00, ctrl_reg);   // enable_ddrc(setreg);
  noop(1000000);

  // provide the result: min max interleaved
  printf("\nRead Eye Result:");
  printEyeTestHeader();
  printf("EYE [MIN-MAX]  :  ");
  for (j=0; j<4; j++)
  {
    result[2*j + 0] = mineye[j];
    result[2*j + 1] = maxeye[j];
    printf("[%d,%3d]  ", mineye[j], maxeye[j]);
  }

  // Save result for xregv access. b[15:8]=maxeye, b[7:0]=mineye.
  for (j=0; j<4; j++)
  {
    cc = mineye[j];
    ww = min(maxeye[j], 255);
    if ((cc > 255) || (ww < 0))
      m = 0;
    else
      m = cc + (ww << 8);
    REG_WRITE(MAILBOX_RESULT + 4*j, m);
  }
  REG_WRITE(MAILBOX_RESULT + 4*4, 1);   // 1 for read, 0 for wr


  // width and center
  if (verbose & 0x20)
  {

    printf("\nEYE CENTER     :   ");
    for (j=0; j<4; j++)
    {
      cc = (mineye[j] + maxeye[j]) >> 1;  // center
      printf("%d/128    ", cc);
    }
    printf("\nEYE WIDTH      :   ");
    for (j=0; j<4; j++)
    {
      ww = maxeye[j] - mineye[j];         // width
      printf("%3.2f%%    ", (double)ww*100/128);
    }
  }
  // done


  // csv printout
  if(csv_mode)
  {
    printf("\n\nfor_csv , %g , %d , %d , %d , %d , %d , %d , %d , %d  \r\n",
      scl,
      mineye[0], maxeye[0],
      mineye[1], maxeye[1],
      mineye[2], maxeye[2],
      mineye[3], maxeye[3]  );
  }

  memtest_stat = 1;
  return rc;
}




/****************************************************************************
 * Function: print_qual
 * Description:
 ****************************************************************************/
void print_qual(int *qual, int cnt, int step)
{
  int i;
  if (verbose & 0x10)
  {
    printf("**** Qual %d:", step);
    for (i=0; i<cnt; i++)
      printf(" %5d", qual[i]);
    printf("%s", cr);
  }
}




/****************************************************************************
 * Function: find_best_eye
 * Description: Find the best eye
        INPUTS:
          x   - pointer to results array containing 'cnt' experiments,
                      each experiment is 8 values: 4 min,max pairs,
            one per lane.
          cnt - number of experiments.

        Assign a quality value to each experiment based on the
        following criteria
        - more: sum of widths of the 4 lanes eyes
        - less: variation between lanes
        - more: good neigbours
 ****************************************************************************/
int find_best_eye(int *x, int cnt, int *qual1, int *qual2, int *qual3, int *qual4)
{
  int i,j, lo, hi, bad, sum, avg, a, choice, n0, n1, n2;
  int maxval[6], maxpos[6];
  int width[4];

  // 1. Analyze eye width and assign qual1 based on width sum
  for (i=0; i<cnt; i++)
  {
    // 1: a measure of eye width
    bad = 0;
    sum = 0;
    for (j=0; j<4; j++)
    {
      lo = x[8*i + 2*j + 0];
      hi = x[8*i + 2*j + 1];
      width[j] = hi - lo;
      if (width[j] <= 0)
        bad++;
      sum += width[j];
    }
    if (bad > 0)
      qual1[i] = 0;
    else
      qual1[i] = sum;

    // 2: a measure of eye width variance
    sum = 0;
    avg = qual1[i];  // average eye width
    for (j=0; j<4; j++)
    {
      lo = x[8*i + 2*j + 0];
      hi = x[8*i + 2*j + 1];
      width[j] = hi - lo;
      if (width[j] <= 0)
        width[j] = 0;
      a = abs(avg - width[j]);
      sum += a*a;
    }
    qual2[i] = sum/4;
  }

  // Subtract qual2 from qual1
  for (i=0; i<cnt; i++)
    qual3[i] = qual1[i]*qual1[i] - qual2[i];
  for (i=0; i<cnt; i++)
    qual4[i] = qual3[i];  // copy

  // debug
  print_qual(&qual1[0], cnt, 1);
  print_qual(&qual2[0], cnt, 2);
  print_qual(&qual3[0], cnt, 3);

  // find max val
  maxval[0] = -9999;
  maxpos[0] = -1;
  for (i=0; i<cnt; i++)
  {
    a = qual3[i];
    if (a > maxval[0])
    {
      maxval[0] = a;
      maxpos[0] = i;
    }
  }

  // Find 2nd, 3rd best
  maxval[1] = -9999;
  maxpos[1] = -1;
  for (i=0; i<cnt; i++)
  {
    a = qual3[i];
    if ((a > maxval[1]) && (i != maxpos[0]))
    {
      maxval[1] = a;
      maxpos[1] = i;
    }
  }

  maxval[2] = -9999;
  maxpos[2] = -1;
  for (i=0; i<cnt; i++)
  {
    a = qual3[i];
    if ((a > maxval[2]) && (i != maxpos[0]) && (i != maxpos[1]))
    {
      maxval[2] = a;
      maxpos[2] = i;
    }
  }

  maxval[3] = -9999;
  maxpos[3] = -1;
  for (i=0; i<cnt; i++)
  {
    a = qual3[i];
    if ((a > maxval[3]) && (i != maxpos[0]) && (i != maxpos[1]) && (i != maxpos[2]))
    {
      maxval[3] = a;
      maxpos[3] = i;
    }
  }


  // take the best choice:
  // a. if top 3 are identical and sequential, take the middle one
  if ((maxval[0] == maxval[1]) && (maxval[1] == maxval[2]))
  {
    if ((maxpos[1] == maxpos[0]+1) && (maxpos[2] == maxpos[1]+1))
    { // sequential
      choice = maxpos[1];
    }
    // pick one with better neighbour
    else
    {
      n0 = qual3[ maxpos[0] - 1] + qual3[ maxpos[0] + 1];
      n1 = qual3[ maxpos[1] - 1] + qual3[ maxpos[1] + 1];
      n2 = qual3[ maxpos[2] - 1] + qual3[ maxpos[2] + 1];

      if ((n0 >= n1) && (n0 >= n2))
        choice = maxpos[0];
      else if ((n1 >= n0) && (n1 >= n2))
        choice = maxpos[1];
      else if ((n2 >= n0) && (n2 >= n1))
        choice = maxpos[2];
      else
        choice = maxpos[0];
    }
  }
  else if (maxval[0] == maxval[1])
  {
    n0 = qual3[ maxpos[0] - 1] + qual3[ maxpos[0] + 1];
    n1 = qual3[ maxpos[1] - 1] + qual3[ maxpos[1] + 1];
    if (n0 >= n1)
      choice = maxpos[0];
    else if (n1 >= n0)
      choice = maxpos[1];
    else
      choice = maxpos[0];
  }
  else
  {
    choice = maxpos[0];
  }


  printf("\n  * Four best results (test no.: quality value) \n\t(%d: %d), (%d: %d), (%d: %d), (%d: %d)\n\tBest Choice = %d %s",
      maxpos[0], maxval[0],
      maxpos[1], maxval[1],
      maxpos[2], maxval[2],
      maxpos[3], maxval[3],
      choice,
      cr);

  return choice;
}



/****************************************************************************
 * Function: weye_test1
 * Description:
 ****************************************************************************/
void weye_test1(int do_all)
{
  // write eye test
  double scl0, scl, incr;
  int dn, dp, rslti, choice, cnt;
  int lo, hi, ctr, dqs, j;
  int wdo[4];

  ddrc_get();
  ddriob_get();

  printDriveTestHeader();

  // 4. measure at a lower drive
  scl0 = 0.6;
  scl = scl0;
  incr = 0.05;
  rslti = 0;
  cnt = 0;

  while (scl <= 1.51)
  {
    dn = (int) (28.0 * scl + 0.5);
    dp = (int) (64.0 * scl + 0.5);
    set_drive_strength(reg_drive_data, dn, dp);
    if (do_all)
    {
      set_drive_strength(reg_drive_addr, dn, dp);
      set_drive_strength(reg_drive_diff, dn, dp);
      set_drive_strength(reg_drive_clock, dn, dp);
    }
    noop(1000000);
    cnt++;
    printf("%2d    [%2d,%3d]         ", cnt, dn, dp);
    measure_write_eye(16, 112, 4, &gresult[rslti], scl, 1, 0, 0, 0);
    scl += incr;
    rslti += 8;
  }
  choice = find_best_eye(&gresult[0], cnt, &q1[0],&q2[0],&q3[0],&q4[0]);

  // 5. implement the best choice wr data offset
  for (j=0; j<4; j++)
  {
    lo = gresult[ 8*choice + 2*j + 0 ];
    hi = gresult[ 8*choice + 2*j + 1 ];
    ctr = (lo + hi) >> 1;
    wdo[j] = ctr;
    update_reglist(&ddrc_reg_values[0], 80, R46 + 4*j, 24,  7, ctr);
    dqs = get_reglist_value(&ddrc_reg_values[0], 80, R55 + 4*j, 0, 10);
    update_reglist(&ddrc_reg_values[0], 80, R5F + 4*j,  0, 10, dqs+ctr);
  }
  printf("\n  * Setting wr_data_offset on all four lanes to [%d]  [%d]  [%d]  [%d] \r\n", wdo[0],wdo[1],wdo[2],wdo[3]);

  // 6. implement the best choice drive strength
  scl = scl0 + incr*choice;
  dn = (int) (28.0 * scl + 0.5);
  dp = (int) (64.0 * scl + 0.5);
  set_drive_strength(reg_drive_data, dn, dp);
  if (do_all)
  {
    set_drive_strength(reg_drive_addr, dn, dp);
    set_drive_strength(reg_drive_diff, dn, dp);
    set_drive_strength(reg_drive_clock, dn, dp);
  }
  noop(1000000);
  printf("\n  * Setting drive strength to [n = %d] & [p = %d]  \r\n", dn, dp);

  // 7. init ddr
  ddrc_init();
  noop(1000000);

  // 8. do a quick memory test
  memtest_all(1024*1024, 1024*1024, 0x7fff, 0);
  printf("\n  * Memory Test on all four lanes: \n\tPer-byte-lane-errors=[%d %d %d %d],   Total=%d errors,   Time=%g sec \r\n",
      werr, errcnt[0], errcnt[1], errcnt[2], errcnt[3], total_test_time);


  printf("\n  * Done LPDDR2 write eye and drive strength optimization\r\n");
}





/****************************************************************************
 * Function: upload_memtest_results
 * Description: Save results to mailbox, limit to 16-bits
 ****************************************************************************/
void upload_memtest_results()
{
  int i, a;

  for (i=0; i<4; i++)
  {
    a = errcnt[i];
    if (a > 0x0ffff)
      a = 0x0ffff;

    REG_WRITE(MAILBOX_RESULT + 4*i, a);
  }
  a = werr;

  if (a > 0x0ffff)
    a = 0x0ffff;

  REG_WRITE(MAILBOX_RESULT + 4*4, a);
}


/****************************************************************************
 * Function: update_patterns
 * Description: Create the 128-word patterns for 32-bit and 16-bit tests
 ****************************************************************************/
void update_patterns()
{
  int i, imaski;
  if (bus_width == 32)
  {
    for (i=0; i<128; i++)
    {
      imaski = (i >> 4) & 0x07;  // invert mask index
      pat1[i] = pattern1[i & 15];
      pat2[i] = pattern1[i & 15] ^ invertmask[imaski];
    }
  }
  else
  {
    for (i=0; i<128; i++)
    {
      imaski = (i >> 4) & 0x07;  // invert mask index
      pat1[i] = pattern1_16bit[i & 15];
      pat2[i] = pattern1_16bit[i & 15] ^ invertmask[imaski];
    }
  }
}



/****************************************************************************
 * Function: check_bus_width
 * Description: Bus Width, R00.reg_ddrc_data_bus_width,  R00[3:2]
        bw = getreg(['pele_ps','ddrc','ddrc_ctrl','reg_ddrc_data_bus_width'])
        if (bw == 1): ln = '16';
        else: ln = '32'
 ****************************************************************************/
void check_bus_width()
{

  int a, bw;

  a = REG_READ(R00);
  bw = get_bit_field(a,2,2);
  if (bw == 1)
    bus_width = 16;
  else
    bus_width = 32;

  update_patterns();
}



/****************************************************************************
 * Function: clear_tdm_idm
 * Description: TERM_DISABLE_MODE and IBUF_DISABLE_MODE must be set to 0 for
           eye measurements to work, so set bits[8,7] to 0 in:
             DDRIOB_DATA0  0xF8000B48
             DDRIOB_DATA1  0xF8000B4C
             DDRIOB_DIFF0  0xF8000B50
             DDRIOB_DIFF1  0xF8000B54
 ****************************************************************************/
void clear_tdm_idm()
{
  int i, data;
  for (i=0; i<4; i++)
  {
    data = REG_READ(0xF8000B48 + 4*i);
    REG_WRITE(0xF8000B48 + 4*i, data & 0xFFFFFE7F );
  }
}




/****************************************************************************
 * Function: main
 * Description:
 ****************************************************************************/
int main(void)
{
  int mbyte = 1024*1024;
  int i,j, k, lp, a, go, rc, imaski, sel, last;
  int test_start, test_size, loop_cnt;
  int imin, imax, istep, testsize_scl, fast, fix_center, printerr;
  int pll_div, remote_mode, mem_test, cmd;
  char c;
  int cache_enable = 1;

  rc = 0;

  // unlock slcr
  REG_WRITE(SLCR_LOW_BASE+SLCR_UNLOCK,  SLCR_UNLOCK_VALUE);
  ddr_nonsecure();
  // disable TERM_DIS_MODE, IBUF_DIS_MODE
  clear_tdm_idm();

  // init done, go to 0
  REG_WRITE(MAILBOX_GO,    0);
  REG_WRITE(MAILBOX_DONE,  0);
  REG_WRITE(MAILBOX_START, 1);
  REG_WRITE(MAILBOX_SIZE,  4);
  REG_WRITE(MAILBOX_MODE,  0x07FFF);   // enable all 15 tests
  REG_WRITE(MAILBOX_LAST,  0x08000);
  go = 0;
  // check xregv mode
  k = REG_READ(MAILBOX_XREGV);
  remote_mode = 0;

  if (k == 0x55)
    remote_mode = 1;

  // Create the 128-word patterns
  for (i=0; i<128; i++)
  {
    imaski = (i >> 4) & 0x07;  // invert mask index
    pat1[i] = pattern1[i & 15];
    pat2[i] = pattern1[i & 15] ^ invertmask[imaski];
  }

  if (test_mode == 1) // standalone diag
  {
    istep = 4;
    imin = 8;
    imax = 120;
    testsize_scl = 1;
    fast = 1;
    fix_center = 0;
    printerr = 0;

    while (1)
    {
      // get keyboard input or remote input from mailbox start
      cmd = 1;
      if (remote_mode)
      {
        printf("**** Waiting for remote Go cmd (bus=%d) \r\n", bus_width);
        k = go;
        while (k == go)
        {
          k = REG_READ(MAILBOX_GO);
          // introduce some delay between loops
          j = 123456;
          for (i=0; i<5000; i++)
            j = calc_value(j);
        }
        go = k;
        printf("**** Got a Go cmd. \r\n");
        c = (char) 0;
        rc = REG_READ(MAILBOX_START);

        if ((rc & 0x8000) == 0)
          cmd = 0;    // b15=0, so it's a start/size memtest, not a menu comd.
        else
          c  = (char) (rc & 0x0ff);      // b15=1, so it is a menu cmd
      }
      else
      {
        // keys used: abcdef_hi_klmn_pqrst_vwx_z  123456789
        printf("\n\n\n\n-----------------------------------------------------------------\n");
        printf("------------------- ZYNQ DRAM DIAGNOSTICS TEST ------------------\n");
        printf("-----------------------------------------------------------------\n");
        printf(" Select one of the options below:\r\n");
        printf(" ## Memory Test ##\r\n");
        printf(" Bus Width = %d,   XADC Temperature = %g\r\n", bus_width, get_xadc_temperature());
        printf("    's' - Test 1MB length from address 0x100000\r\n");
        printf("    '1' - Test 32MB length from address 0x100000\r\n");
        printf("    '2' - Test 64MB length from address 0x100000\r\n");
        printf("    '3' - Test 128MB length from address 0x100000\r\n");
        printf("    '4' - Test 255MB length from address 0x100000\r\n");
        printf("    '5' - Test 511MB length from address 0x100000\r\n");
        printf("    '6' - Test 1023MB length from address 0x100000\r\n");
        printf(" ## Read Data Eye Measurement Test\r\n");
        printf("    'r' - Measure Read Data Eye\r\n");
        printf(" ## Write Data Eye Measurement Test\r\n");
        printf("    'i' - Measure Write Data Eye\r\n");
        printf("    Other options for Write Eye Data Test:\r\n");
        printf("         'f' - Fast Mode: Toggles Fast mode - ON/OFF\r\n");
        printf("         'c' - Centre Mode: Toggles Centre mode - ON/OFF\r\n");
        printf("         'e' - Vary the size of memory test for Read/Write Eye Measurement tests\r\n");
        printf(" ## Data Cache Enable / Disable Option:\r\n");
        printf("     'z' - D-Cache Enable / Disable\r\n");
        printf(" ## Other options\n");
        printf("     'v' - Verbose Mode ON/OFF\n");


        c = inbyte();
        if(c == '\r')
        {
          outbyte('\n');
        }
        printf("\n\rOption Selected : %c",c);
        outbyte(c);
        printf("\n\n");
      }

      // before execution, check bus width and update patterns
      check_bus_width();

      // check command
      mem_test = 0;
      if (cmd == 0)
      {
        // a start/size memtest
        mem_test = 1;
        test_start = REG_READ(MAILBOX_START)*mbyte;
        a  = REG_READ(MAILBOX_SIZE);
        test_size  = (a & 0x3ff)*mbyte;

        if (test_start < mbyte)
          test_start = mbyte;
        if (test_start > 1023*mbyte)
          test_start = mbyte;
        if (test_size  < mbyte)
          test_size  = mbyte;
        if ((test_start+test_size) > 1024*mbyte)
        {
          test_start  = mbyte;
          test_size   = 16*mbyte;
        }

        sel = REG_READ(MAILBOX_MODE);
        verbose = 1 + printerr*8;
        mem_test=1;
        memtest_all(test_start, test_size, sel, 0);
      }
      else if ((c == 's') || (c == 'S'))
      {
        printf("\r\nStarting Memory Test 's' - Testing 1MB length from address 0x100000...\r\n");
        verbose = 1 + printerr*8;
        mem_test=1;
        printMemTestHeader();
        memtest_all(mbyte, 1*mbyte, 0x7fff, 0);
      }
      else if (c == '1')
      {
        printf("\r\nStarting Memory Test '1' - Testing 32MB length from address 0x100000...\r\n");
        verbose = 1 + printerr*8;
        mem_test=1;
        printMemTestHeader();
        memtest_all(mbyte, 32*mbyte, 0x7fff, 0);
      }
      else if (c == '2')
      {
        printf("\r\nStarting Memory Test '2' - Testing 64MB length from address 0x100000...\r\n");
        verbose = 1 + printerr*8;
        mem_test=1;
        printMemTestHeader();
        memtest_all(mbyte, 64*mbyte, 0x7fff, 0);
      }
      else if (c == '3')
      {
        printf("\r\nStarting Memory Test '3' - Testing 128MB length from address 0x100000...\r\n");
        verbose = 1 + printerr*8;
        mem_test=1;
        printMemTestHeader();
        memtest_all(mbyte, 128*mbyte, 0x7fff, 0);
      }
      else if (c == '4')
      {
        printf("\r\nStarting Memory Test '4' - Testing 255MB length from address 0x100000...\r\n");
        verbose = 1 + printerr*8;
        mem_test=1;
        printMemTestHeader();
        memtest_all(mbyte, 255*mbyte, 0x7fff, 0);
      }
      else if (c == '5')
      {
        printf("\r\nStarting Memory Test '5' - Testing 511MB length from address 0x100000...\r\n");
        verbose = 1 + printerr*8;
        mem_test=1;
        printMemTestHeader();
        memtest_all(mbyte, 511*mbyte, 0x7fff, 0);
      }
      else if (c == '6')
      {
        printf("\r\nStarting Memory Test '6' - Testing 1023MB length from address 0x100000...\r\n");
        verbose = 1 + printerr*8;
        mem_test=1;
        printMemTestHeader();
        memtest_all(mbyte, 1023*mbyte, 0x7fff, 0);
      }
      else if (c == '7')
      {
        verbose = 1 + printerr*8;

        for (i=0; i<4; i++)
          cum_errcnt[i] = 0;

        for (lp=0; lp<test_loop_cnt; lp++)
        {
          memtest_all(mbyte, mbyte*test_sizes[test_size_sel], 0x7fff, lp);

          for (i=0; i<4; i++)
            cum_errcnt[i] += errcnt[i];

          printf("Memtest Loop %d, cumulative errors: (%d %d %d %d) *l,m,t* \r\n",
              lp, cum_errcnt[0], cum_errcnt[1], cum_errcnt[2], cum_errcnt[3]);
        }
      }
      // Write Eye / Drive Test 1
      else if ((c == 'd') || (c == 'D'))
      {
        verbose = 0x40;
        printf("\nRunning Write Eye / Drive Test 1 now... \r\n");

        weye_test1(0);
      }

      // Write Eye / Drive Test 1 (All)
      else if ((c == 'a') || (c == 'A'))
      {
        verbose = 0;
        printf("\nRunning Write Eye / Drive Test 1 (all) now... \r\n");
        weye_test1(1);
      }

      // Set Bus width
      else if ((c == 'b') || (c == 'B'))
      {
        if (bus_width == 32)
        {
          bus_width = 16;
        }
        else
        {
          bus_width = 32;
        }
        update_patterns();
        printf("\nBus width = %d \r\n", bus_width);
      }

      //Cache Enable Disable
      else if ((c == 'z') || (c == 'Z'))
      {
        if (cache_enable == 1)
        {
          Xil_DCacheDisable();
          cache_enable = 0;
        }
        else
        {
          Xil_DCacheEnable();
          cache_enable = 1;
        }
        (cache_enable) ? printf("\nD Cache enabled\r\n") : printf("\nD Cache disabled\r\n");
      }

      // Write Eye measurement test
      else if ((c == 'i') || (c == 'I'))
      {
        verbose = 0x22 | (printerr*8) ;
        printf("\nRunning Write Eye Measurement now ... \r\n");
        ddrc_get();
        ddriob_get();
        printMemTestHeader();
        rc = measure_write_eye(imin, imax, istep, &gresult[0], 1.0, testsize_scl, fast, fix_center, 1);
        ddrc_init();
      }

      // fast_is_bc
      else if ((c == 'j') || (c == 'J'))
      {
        fast_is_bc ^= 1;
        printf("**** fast is bc = %d \r\n", fast_is_bc);
      }

       // default params
      else if ((c == 'k') || (c == 'K'))
      {
        istep = 4;
        testsize_scl = 1;
        fast = 1;
      }

      else if ((c == 'l') || (c == 'L'))
      {
        test_loop_cnt *= 2;
        printf("**** test loop cnt = %d \r\n", test_loop_cnt);
      }

      else if ((c == 'm') || (c == 'M'))
      {
        test_loop_cnt = max(1, test_loop_cnt/2);
        printf("**** test loop cnt = %d \r\n", test_loop_cnt);
      }

      else if ((c == 't') || (c == 'T'))
      {
        test_size_sel = (test_size_sel + 1) % 8;
        printf("**** test size = %d \r\n", test_sizes[test_size_sel]);
      }

      // Read Eye Measurement Test
      else if ((c == 'r') || (c == 'R'))
      {
        verbose = 0x22 | (printerr*8);
        printf("\nRunning Read Eye Measurement now ... \r\n");
        printMemTestHeader();
        rc = measure_read_eye(&gresult[0], mbyte, mbyte*testsize_scl, fast, istep);
      }

      // Wider Mode for Write Eye measurement
      else if ((c == 'w') || (c == 'W'))
      {
        imin = max(imin-4, 0);
        imax = min(imax+4, 128);
        printf("\nWider Mode ON - Widens the Write Eye Measurement range by 4 units\r\n");
      }

      // Narrow Mode for Write Eye measurement
      else if ((c == 'n') || (c == 'N'))
      {
        imin = min(imin+4, 60);
        imax = max(imax-4, 68);
        printf("Narrow Mode ON - Narrows the Write Eye Measurement range by 4 units\r\n");
      }

      // Fast Mode for Write Eye measurement - toggle
      else if ((c == 'f') || (c == 'F'))
      {
        fast = fast ^ 1;
        if(fast)
          printf("Fast Mode ON - Runs twice fast (fewer data patterns are tested)\r\n");
        else
          printf("Fast Mode OFF - Runs all the data patterns\r\n");
      }

      // Help menu
      else if ((c == 'h') || (c == 'H'))
      {
        printf("**** for HELP see test1_help.txt \r\n");
      }

      // Center Mode for Write Eye measurement - toggle
      else if ((c == 'c') || (c == 'C'))
      {
        fix_center = fix_center ^ 1;
        if(fix_center)
          printf("Centre Mode ON\r\n");
        else
          printf("Centre Mode OFF\r\n");
      }

      // Memory test size scaler for eye measurement
      else if ((c == 'e') || (c == 'E'))
      {
        static int temp = 1;
        temp = temp*2; // allow max 32MB
        if(temp > 128)
          testsize_scl = temp - 1;
        else
          testsize_scl = temp;
        if(testsize_scl > 1023)
          temp = testsize_scl = 1;
        printf("Memory Test Size Scalar = %d \r\n", testsize_scl);
      }

      // Verbose Mode
      else if ((c == 'v') || (c == 'V'))
      {
        printerr = printerr ^ 1;
        if(printerr)
          printf("Verbose Mode ON\r\n");
        else
          printf("Verbose Mode OFF\r\n");
      }

      // CSV Mode
      else if ((c == 'y') || (c == 'Y'))
      {
        csv_mode = csv_mode ^ 1;
        if(csv_mode)
          printf("CSV Mode ON - prints information for CSV files\r\n");
        else
          printf("CSV Mode OFF - prints information for CSV files\r\n");
      }

      // Read ADC
      else if ((c == 'x') || (c == 'X'))
      {
        read_xadc();
      }

      //Remote
      else if ((c == 'z') || (c == 'Z'))
      {
        remote_mode ^= 1;
        if(remote_mode)
          printf("Remote Mode ON\r\n");
        else
          printf("Remote Mode OFF\r\n");
        REG_WRITE(MAILBOX_GO,    0);
        REG_WRITE(MAILBOX_DONE,  0);
        REG_WRITE(MAILBOX_START, 0);
        go = 0;
      }

      // Write only test
      else if (c == '8')
      {
        printf("Write only test...\r\n");
        rc = memtest_simple(mbyte, 64*mbyte, 0, -1, 0xFFFFFFFF,0,0xFFFFFFFF,0);
      }

      // Read only test
      else if (c == '9')
      {
        printf("Read only test...\r\n");
        rc = memtest_readonly(mbyte, 64*mbyte);
      }

      // Set PLL
      else if ((c == 'p') || (c == 'P'))
      {
        pll_div += 4;
        if (pll_div > 48) pll_div = 32; // 533.hz
        printf("Setting PLL, pll_div = %d\r\n", pll_div);
        my_set_pll(pll_select_cpu, pll_div, 2, REF_CLOCK);  // 40=667mhz, 48=800mhz

      }

      // Read Eye Step (Quality)
      else if ((c == 'q') || (c == 'Q'))
      {
        istep /= 2;
        if (istep < 1) istep = 4;
        printf("Eye step = %d \r\n", istep);
      }

      // Report done
      if (remote_mode)
      {
        if (mem_test)
          upload_memtest_results();
        // incr done
        k = REG_READ(MAILBOX_DONE);
        REG_WRITE(MAILBOX_DONE, k+1);
      }

    }
  }

  if (test_mode == 2)
  {
    // Do a continuous 10101010 write, no reads, for waveform probing
    k = 0;
    while (1)
    {
      rc = memtest_simple(mbyte, 4*mbyte, k, -1, 0xFFFFFFFF,0,0xFFFFFFFF,0);
      k = (k + 1) & 0x0ff;
    }
  }

  // test_mode == 0: memtest driven by xregv
  while (1)
  {
    // wait for go to change
    rc = REG_READ(L2CCCrtl);
    printf("**** Waiting for Go (3/17/12) l2_en=%d \r\n", rc);
    k = go;
    while (k == go)
    {
      k = REG_READ(MAILBOX_GO);
      // introduce some delay between loops
      j = 123456;
      for (i=0; i<5000; i++)
        j = calc_value(j);
    }
    go = k;

    // start a new test
    printf("**** Starting a new test \r\n");
    test_start = REG_READ(MAILBOX_START)*mbyte;
    a  = REG_READ(MAILBOX_SIZE);
    verbose = (a & 1) ? 9 : 1;      // print errors if sz is odd
    test_size  = (a & 0x3ff)*mbyte;
    loop_cnt = (a >> 10) & 0x3f;

    if (loop_cnt < 1)
      loop_cnt = 1;
    if (test_start < mbyte)
      test_start = mbyte;
    if (test_start > 1023*mbyte)
      test_start = mbyte;
    if (test_size  < mbyte)
      test_size  = mbyte;
    if ((test_start+test_size) > 1024*mbyte)
    {
      test_start  = mbyte;
      test_size   = 16*mbyte;
    }

    sel = REG_READ(MAILBOX_MODE);
    if (sel & 0x08000)
    {  // other functions
      loop_cnt = 0;
      if (sel == 0x8001)
        weye_test1(0);
    }

    // MAILBOX_LAST - last word - special modes
    //  b0 - disable caches
    //  b1 - print errors (verbose 8)
    last = REG_READ(MAILBOX_LAST);
    if (last & 1)
    {
      cache_ctrl(0, 0);
      l2_ctrl(0);
    }
    else
    {
      cache_ctrl(1, 1);
      l2_ctrl(1);
    }
    verbose = ((last & 2) != 0) ? 9 : 1;

    werr = 0;
    for (i=0; i<4; i++)
      errcnt[i] = 0;
    total_test_time = 0.0;

    for (lp=0; lp<loop_cnt; lp++)
    {
      if (sel & 0x0001)
        rc = memtest(test_start, test_size, 0, lp);
      if (sel & 0x0002)
        rc = memtest_simple(test_start, test_size, 1, lp, 0,0,0,0);
      if (sel & 0x0004)
        rc = memtest_simple(test_start, test_size, 2, lp, 0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF);
      if (sel & 0x0008)
        rc = memtest_simple(test_start, test_size, 3, lp, 0xAAAAAAAA,0xAAAAAAAA,0xAAAAAAAA,0xAAAAAAAA);
      if (sel & 0x0010)
        rc = memtest_simple(test_start, test_size, 4, lp, 0x55555555,0x55555555,0x55555555,0x55555555);
      if (sel & 0x0020)
        rc = memtest_simple(test_start, test_size, 5, lp, 0,0xFFFFFFFF,0,0xFFFFFFFF);
      if (sel & 0x0040)
        rc = memtest_simple(test_start, test_size, 6, lp, 0xFFFFFFFF,0,0xFFFFFFFF,0);
      if (sel & 0x0080)
        rc = memtest_simple(test_start, test_size, 7, lp, 0x55555555,0xAAAAAAAA,0x55555555,0xAAAAAAAA);
      if (sel & 0x0100)
        rc = memtest_simple(test_start, test_size, 8, lp, 0xAAAAAAAA,0x55555555,0xAAAAAAAA,0x55555555);
      if (sel & 0x0200)
        rc = memtest_pat128(test_start, test_size,  9, lp, &pat1[0]);
      if (sel & 0x0400)
        rc = memtest_pat128(test_start, test_size, 10, lp, &pat2[0]);
      if (sel & 0x0800)
        rc = memtest_lfsr(test_start, test_size, 11, lp);
      if (sel & 0x1000)
        rc = memtest_lfsr(test_start, test_size, 12, lp);
      if (sel & 0x2000)
        rc = memtest_lfsr(test_start, test_size, 13, lp);
      if (sel & 0x4000)
        rc = memtest_lfsr(test_start, test_size, 14, lp);

      printf("**** Total so far: %d errors (%d %d %d %d) , %g sec \r\n",
          werr, errcnt[0], errcnt[1], errcnt[2], errcnt[3], total_test_time);
    }

    // save results to mailbox, limit to 16-bits
    upload_memtest_results();

    // incr done
    k = REG_READ(MAILBOX_DONE);
    REG_WRITE(MAILBOX_DONE, k+1);
  }

  while(1)
  {
    c = inbyte();
    //if(c == '\r'){
      outbyte('\n');
    //}
    outbyte(c);
  }

  return rc;
}

