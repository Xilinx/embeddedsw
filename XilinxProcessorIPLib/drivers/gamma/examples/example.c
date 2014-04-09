/******************************************************************************
* (c) Copyright 2010 Xilinx, Inc. All rights reserved.
*
* This file contains confidential and proprietary information
* of Xilinx, Inc. and is protected under U.S. and
* international copyright and other intellectual property
* laws.
*
* DISCLAIMER
* This disclaimer is not a license and does not grant any
* rights to the materials distributed herewith. Except as
* otherwise provided in a valid license issued to you by
* Xilinx, and to the maximum extent permitted by applicable
* law: (1) THESE MATERIALS ARE MADE AVAILABLE "AS IS" AND
* WITH ALL FAULTS, AND XILINX HEREBY DISCLAIMS ALL WARRANTIES
* AND CONDITIONS, EXPRESS, IMPLIED, OR STATUTORY, INCLUDING
* BUT NOT LIMITED TO WARRANTIES OF MERCHANTABILITY, NON-
* INFRINGEMENT, OR FITNESS FOR ANY PARTICULAR PURPOSE; and
* (2) Xilinx shall not be liable (whether in contract or tort,
* including negligence, or under any other theory of
* liability) for any loss or damage of any kind or nature
* related to, arising under or in connection with these
* materials, including for any direct, or any indirect,
* special, incidental, or consequential loss or damage
* (including loss of data, profits, goodwill, or any type of
* loss or damage suffered as a result of any action brought
* by a third party) even if such damage or loss was
* reasonably foreseeable or Xilinx had been advised of the
* possibility of the same.
*
* CRITICAL APPLICATIONS
* Xilinx products are not designed or intended to be fail-
* safe, or for use in any application requiring fail-safe
* performance, such as life-support or safety devices or
* systems, Class III medical devices, nuclear facilities,
* applications related to the deployment of airbags, or any
* other applications that could lead to death, personal
* injury, or severe property or environmental damage
* (individually and collectively, "Critical
* Applications"). Customer assumes the sole risk and
* liability of any use of Xilinx products in Critical
* Applications, subject only to applicable laws and
* regulations governing limitations on product liability.
*
* THIS COPYRIGHT NOTICE AND DISCLAIMER MUST BE RETAINED AS
* PART OF THIS FILE AT ALL TIMES.
******************************************************************************/

/*****************************************************************************/
/**
 *
 * @file example.c
 *
 * This file demonstrates how to use Xilinx Gamma Correction (Gamma)
 * driver of the Xilinx Gamma Correction v3.0 core. This code does not 
 * cover the Gamma v3.0 setup and any other configuration which might be
 * required to get the Gamma device working properly.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- -------------------------------------------------------
 * 3.00a jo   07/20/09 First release
 * 3.00a gz   07/27/09 Adapted for gamma
 * </pre>
 *
 * ***************************************************************************
 */

#include "gamma.h"
#include "xparameters.h"

static unsigned char gamma_tables[5*256] = {
// linear table
     0,     1,     2,     3,     4,     5,     6,     7,
     8,     9,    10,    11,    12,    13,    14,    15,
    16,    17,    18,    19,    20,    21,    22,    23,
    24,    25,    26,    27,    28,    29,    30,    31,
    32,    33,    34,    35,    36,    37,    38,    39,
    40,    41,    42,    43,    44,    45,    46,    47,
    48,    49,    50,    51,    52,    53,    54,    55,
    56,    57,    58,    59,    60,    61,    62,    63,
    64,    65,    66,    67,    68,    69,    70,    71,
    72,    73,    74,    75,    76,    77,    78,    79,
    80,    81,    82,    83,    84,    85,    86,    87,
    88,    89,    90,    91,    92,    93,    94,    95,
    96,    97,    98,    99,   100,   101,   102,   103,
   104,   105,   106,   107,   108,   109,   110,   111,
   112,   113,   114,   115,   116,   117,   118,   119,
   120,   121,   122,   123,   124,   125,   126,   127,
   128,   129,   130,   131,   132,   133,   134,   135,
   136,   137,   138,   139,   140,   141,   142,   143,
   144,   145,   146,   147,   148,   149,   150,   151,
   152,   153,   154,   155,   156,   157,   158,   159,
   160,   161,   162,   163,   164,   165,   166,   167,
   168,   169,   170,   171,   172,   173,   174,   175,
   176,   177,   178,   179,   180,   181,   182,   183,
   184,   185,   186,   187,   188,   189,   190,   191,
   192,   193,   194,   195,   196,   197,   198,   199,
   200,   201,   202,   203,   204,   205,   206,   207,
   208,   209,   210,   211,   212,   213,   214,   215,
   216,   217,   218,   219,   220,   221,   222,   223,
   224,   225,   226,   227,   228,   229,   230,   231,
   232,   233,   234,   235,   236,   237,   238,   239,
   240,   241,   242,   243,   244,   245,   246,   247,
   248,   249,   250,   251,   252,   253,   254,   255,
// gamma = 1/2.2
     0,    21,    28,    34,    39,    43,    46,    50,
    53,    56,    59,    61,    64,    66,    68,    70,
    72,    74,    76,    78,    80,    82,    84,    85,
    87,    89,    90,    92,    93,    95,    96,    98,
    99,   101,   102,   103,   105,   106,   107,   109,
   110,   111,   112,   114,   115,   116,   117,   118,
   119,   120,   122,   123,   124,   125,   126,   127,
   128,   129,   130,   131,   132,   133,   134,   135,
   136,   137,   138,   139,   140,   141,   142,   143,
   144,   144,   145,   146,   147,   148,   149,   150,
   151,   151,   152,   153,   154,   155,   156,   156,
   157,   158,   159,   160,   160,   161,   162,   163,
   164,   164,   165,   166,   167,   167,   168,   169,
   170,   170,   171,   172,   173,   173,   174,   175,
   175,   176,   177,   178,   178,   179,   180,   180,
   181,   182,   182,   183,   184,   184,   185,   186,
   186,   187,   188,   188,   189,   190,   190,   191,
   192,   192,   193,   194,   194,   195,   195,   196,
   197,   197,   198,   199,   199,   200,   200,   201,
   202,   202,   203,   203,   204,   205,   205,   206,
   206,   207,   207,   208,   209,   209,   210,   210,
   211,   212,   212,   213,   213,   214,   214,   215,
   215,   216,   217,   217,   218,   218,   219,   219,
   220,   220,   221,   221,   222,   223,   223,   224,
   224,   225,   225,   226,   226,   227,   227,   228,
   228,   229,   229,   230,   230,   231,   231,   232,
   232,   233,   233,   234,   234,   235,   235,   236,
   236,   237,   237,   238,   238,   239,   239,   240,
   240,   241,   241,   242,   242,   243,   243,   244,
   244,   245,   245,   246,   246,   247,   247,   248,
   248,   249,   249,   249,   250,   250,   251,   251,
   252,   252,   253,   253,   254,   254,   255,   255,
// gamma = 1/1.6
     0,     8,    12,    16,    19,    22,    24,    27,
    29,    32,    34,    36,    38,    40,    42,    43,
    45,    47,    49,    50,    52,    54,    55,    57,
    58,    60,    61,    63,    64,    66,    67,    68,
    70,    71,    72,    74,    75,    76,    78,    79,
    80,    81,    83,    84,    85,    86,    87,    89,
    90,    91,    92,    93,    94,    96,    97,    98,
    99,   100,   101,   102,   103,   104,   105,   106,
   107,   109,   110,   111,   112,   113,   114,   115,
   116,   117,   118,   119,   120,   121,   122,   123,
   124,   125,   125,   126,   127,   128,   129,   130,
   131,   132,   133,   134,   135,   136,   137,   138,
   138,   139,   140,   141,   142,   143,   144,   145,
   146,   146,   147,   148,   149,   150,   151,   152,
   152,   153,   154,   155,   156,   157,   158,   158,
   159,   160,   161,   162,   162,   163,   164,   165,
   166,   167,   167,   168,   169,   170,   171,   171,
   172,   173,   174,   175,   175,   176,   177,   178,
   178,   179,   180,   181,   181,   182,   183,   184,
   185,   185,   186,   187,   188,   188,   189,   190,
   191,   191,   192,   193,   194,   194,   195,   196,
   196,   197,   198,   199,   199,   200,   201,   202,
   202,   203,   204,   204,   205,   206,   207,   207,
   208,   209,   209,   210,   211,   211,   212,   213,
   214,   214,   215,   216,   216,   217,   218,   218,
   219,   220,   220,   221,   222,   222,   223,   224,
   225,   225,   226,   227,   227,   228,   229,   229,
   230,   231,   231,   232,   233,   233,   234,   235,
   235,   236,   236,   237,   238,   238,   239,   240,
   240,   241,   242,   242,   243,   244,   244,   245,
   246,   246,   247,   247,   248,   249,   249,   250,
   251,   251,   252,   252,   253,   254,   254,   255,
// gamma = 1.6
     0,     0,     0,     0,     0,     0,     1,     1,
     1,     1,     1,     2,     2,     2,     2,     3,
     3,     3,     4,     4,     4,     5,     5,     5,
     6,     6,     7,     7,     7,     8,     8,     9,
     9,    10,    10,    11,    11,    12,    12,    13,
    13,    14,    14,    15,    15,    16,    16,    17,
    18,    18,    19,    19,    20,    21,    21,    22,
    23,    23,    24,    25,    25,    26,    27,    27,
    28,    29,    29,    30,    31,    31,    32,    33,
    34,    34,    35,    36,    37,    38,    38,    39,
    40,    41,    42,    42,    43,    44,    45,    46,
    46,    47,    48,    49,    50,    51,    52,    53,
    53,    54,    55,    56,    57,    58,    59,    60,
    61,    62,    63,    64,    64,    65,    66,    67,
    68,    69,    70,    71,    72,    73,    74,    75,
    76,    77,    78,    79,    80,    81,    83,    84,
    85,    86,    87,    88,    89,    90,    91,    92,
    93,    94,    95,    97,    98,    99,   100,   101,
   102,   103,   104,   106,   107,   108,   109,   110,
   111,   113,   114,   115,   116,   117,   119,   120,
   121,   122,   123,   125,   126,   127,   128,   130,
   131,   132,   133,   135,   136,   137,   138,   140,
   141,   142,   143,   145,   146,   147,   149,   150,
   151,   153,   154,   155,   157,   158,   159,   161,
   162,   163,   165,   166,   167,   169,   170,   171,
   173,   174,   176,   177,   178,   180,   181,   183,
   184,   185,   187,   188,   190,   191,   193,   194,
   196,   197,   198,   200,   201,   203,   204,   206,
   207,   209,   210,   212,   213,   215,   216,   218,
   219,   221,   222,   224,   225,   227,   228,   230,
   231,   233,   235,   236,   238,   239,   241,   242,
   244,   245,   247,   249,   250,   252,   253,   255,
// gamma = 2.2
     0,     0,     0,     0,     0,     0,     0,     0,
     0,     0,     0,     0,     0,     0,     0,     1,
     1,     1,     1,     1,     1,     1,     1,     1,
     1,     2,     2,     2,     2,     2,     2,     2,
     3,     3,     3,     3,     3,     4,     4,     4,
     4,     5,     5,     5,     5,     6,     6,     6,
     6,     7,     7,     7,     8,     8,     8,     9,
     9,     9,    10,    10,    11,    11,    11,    12,
    12,    13,    13,    13,    14,    14,    15,    15,
    16,    16,    17,    17,    18,    18,    19,    19,
    20,    20,    21,    22,    22,    23,    23,    24,
    25,    25,    26,    26,    27,    28,    28,    29,
    30,    30,    31,    32,    33,    33,    34,    35,
    35,    36,    37,    38,    39,    39,    40,    41,
    42,    43,    43,    44,    45,    46,    47,    48,
    49,    49,    50,    51,    52,    53,    54,    55,
    56,    57,    58,    59,    60,    61,    62,    63,
    64,    65,    66,    67,    68,    69,    70,    71,
    73,    74,    75,    76,    77,    78,    79,    81,
    82,    83,    84,    85,    87,    88,    89,    90,
    91,    93,    94,    95,    97,    98,    99,   100,
   102,   103,   105,   106,   107,   109,   110,   111,
   113,   114,   116,   117,   119,   120,   121,   123,
   124,   126,   127,   129,   130,   132,   133,   135,
   137,   138,   140,   141,   143,   145,   146,   148,
   149,   151,   153,   154,   156,   158,   159,   161,
   163,   165,   166,   168,   170,   172,   173,   175,
   177,   179,   181,   182,   184,   186,   188,   190,
   192,   194,   196,   197,   199,   201,   203,   205,
   207,   209,   211,   213,   215,   217,   219,   221,
   223,   225,   227,   229,   231,   234,   236,   238,
   240,   242,   244,   246,   248,   251,   253,   255
};

unsigned char gamma_names[5][24] = {
    "Linear Table           ", 
    "Compression Table 1/1.6",
    "Compression Table 1/2.2",
    "Expansion Table 1.6    ",
    "Expansion Table 2.2    "}; 

unsigned char gamma_table_value(int TABLE_ID, int addr)
{
  return( gamma_tables[(TABLE_ID<<8)+(addr & 255)] );
}

unsigned char* gamma_table_name(int TABLE_ID)
{
  return( gamma_names[TABLE_ID<<8] );
}

/***************************************************************************/
// Gamma Correction Register Reading Example
// This function provides an example of how to read the current configuration
// settings of the Gamma core.
/***************************************************************************/
void report_gamma_settings(u32 BaseAddress) {
  
  u32 size, cols, rows;

  xil_printf("Gamma Correction Core Configuration:\r\n");
  xil_printf(" Gamma Enable Bit: %1d\r\n", GAMMA_ReadReg(BaseAddress, GAMMA_CONTROL) & GAMMA_CTL_EN_MASK);  
  xil_printf(" Gamma Register Update Bit: %1d\r\n", (GAMMA_ReadReg(BaseAddress, GAMMA_CONTROL) & GAMMA_CTL_RUE_MASK) >> 1);	  
  xil_printf(" Gamma Reset Bit: %1d\r\n", (GAMMA_ReadReg(BaseAddress, GAMMA_CONTROL) & GAMMA_RST_RESET) >> 31);	  
  xil_printf(" Gamma AutoReset Bit: %1d\r\n", (GAMMA_ReadReg(BaseAddress, GAMMA_CONTROL) & GAMMA_RST_AUTORESET) >> 30);	
  xil_printf(" Gamma Status: 0x%08x \r\n", GAMMA_ReadReg(BaseAddress, GAMMA_STATUS));
  xil_printf(" Gamma Version:   0x%8x\r\n", GAMMA_ReadReg(BaseAddress, GAMMA_VERSION));

  size = GAMMA_ReadReg(BaseAddress, GAMMA_ACTIVE_SIZE);
  cols = size & 0xffff;
  rows = (size>>16) & 0xffff;
  xil_printf(" Gamma Frame Size: Columns = %d, Rows = %d\r\n",cols,rows);
}

/***************************************************************************/
// Gamma Correction Register Update Example
//  This function provides an example of the process used to update
// the coefficient and offset registers in the Gamma core.
//  In most video systems, it is expected that this process would be executed 
// in response to an interrupt connected to the VBlank video timing signal
// or a timeout signal associated with a watchdog timer.
/***************************************************************************/
void GAMMA_Update_Example(u32 BaseAddress, u32 IWIDTH, u32 OWIDTH, u32 INTPOL, u32 LUTS) {
  
  u32 i, addr, data;
  u32 LutDepth;
  const u32 GAMMA_TABLE_ID = 3;  // gamma = 1.6
    
  GAMMA_Enable(BaseAddress);              //Enable the Gamma software enable

  LutDepth = (IWIDTH == 12 && INTPOL ==1) ? 1024 : 1<<IWIDTH;
	
  GAMMA_RegUpdateDisable(BaseAddress);    //Disable register updates.
    //This is the default operating mode for the Gamma core, and allows
    //registers to be updated without effecting the core's behavior.
	

  //Download Gamma Tables
  for(i=0; i<LutDepth; i++) {
    addr = i>>(IWIDTH-8);
    data = gamma_tables[i+GAMMA_TABLE_ID*256] * (1<<(OWIDTH-8));
    GAMMA_WriteReg(BaseAddress, GAMMA_ADDR_DATA,  (addr << 16) +  data );
    if (LUTS > 1) {
      addr += LutDepth;
      GAMMA_WriteReg(BaseAddress, GAMMA_ADDR_DATA,  (addr << 16) +  data );
    }
    if (LUTS > 2) {
      addr += LutDepth;
      GAMMA_WriteReg(BaseAddress, GAMMA_ADDR_DATA,  (addr << 16) +  data );
    }
  }

  GAMMA_WriteReg(BaseAddress, GAMMA_TABLE_UPDATE,  1);

  xil_printf("Gamma initialized with %s\r\n", gamma_names[GAMMA_TABLE_ID]);
   
  //Enable register updates.
  //This mode will cause the coefficient and offset registers internally
  //to the Gamma core to automatically be updated on the next SOF.
  GAMMA_RegUpdateEnable(BaseAddress);

}


/*****************************************************************************/
//
// This is the main function for the Gamma example.
//
/*****************************************************************************/
//int gamma_example(void)
int main(void)
{
    // Print the current settings for the Gamma core
    report_gamma_settings(XPAR_GAMMA_0_BASEADDR);
	 
    // Call the Gamma example, specify the Device ID generated in xparameters.h
    GAMMA_Update_Example(XPAR_GAMMA_0_BASEADDR,XPAR_GAMMA_0_S_AXIS_VIDEO_DATA_WIDTH, XPAR_GAMMA_0_M_AXIS_VIDEO_DATA_WIDTH, XPAR_GAMMA_0_INTPOL, XPAR_GAMMA_0_LUTS);
	 
    return 0;
}
