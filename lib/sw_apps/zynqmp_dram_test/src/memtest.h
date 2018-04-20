/*
 * mailbox.h
 *
 *  Created on: Sep 16, 2015
 *      Author: dharmes
 */

#ifndef MAILBOX_H_
#define MAILBOX_H_

// for communication with xregv use 12 sequential 16-bit regs in ttc0 starting at 0xf8001024
#define MAILBOX_XREGV    0xFF110000       /* xregv mode here, a 7-bit reg */
#define MAILBOX          0xFF110024
#define MAILBOX_GO       (MAILBOX+0x0)    /* incr to start a test */
#define MAILBOX_DONE     (MAILBOX+0x04)     /* incr to indicate done */
#define MAILBOX_STAT     (MAILBOX+0x08)     /* status */
#define MAILBOX_START    (MAILBOX+0x0C)     /* test start addr in MB units */
#define MAILBOX_SIZE     (MAILBOX+0x10)     /* [9:0]=test size in MB, [15:10]=loop_cnt */
#define MAILBOX_RESULT   (0xFF130024+0x00)  /* 4 error counts per byte lane, + total */
#define MAILBOX_MODE     (MAILBOX+0x28)     /* mode, 1 bit per test, if msb=0 */
#define MAILBOX_LAST     (MAILBOX+0x2C)     /* last of the 12 words */

#define DDRC_MSTR_OFFSET  					0XFD070000
#define DDRC_MSTR_DATA_BUS_WIDTH_SHIFT      12

void select_rank(int rank);
void calc_per_tap_delay(unsigned int lane);
void clear_eye(unsigned int *addr);
void clear_results(unsigned int *addr);
void disable_vtcomp(void);
void enable_vtcomp(void);
void print_line2(void);
void print_line3(void);
void print_results();
void printEyeHeader(void);
void printEyeResultsHeader(void);
void run_memtest_32bit(unsigned int long startaddr, unsigned int len, unsigned int pattern);
void run_memtest_64bit(unsigned long int startaddr, unsigned int len, unsigned int pattern);

#endif /* MAILBOX_H_ */
