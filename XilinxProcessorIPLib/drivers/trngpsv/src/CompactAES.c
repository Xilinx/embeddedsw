/******************************************************************************
*
* Copyright (C) 2021 IP Cores, Inc.  All rights reserved.
* Copyright (C) 2021 Xilinx, Inc.  All rights reserved.
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
*
* Use of the Software is limited solely to applications:
* (a) running on a Xilinx device, or
* (b) that interact with a Xilinx device through a bus or interconnect.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* XILINX CONSORTIUM BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
* WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
* OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*
* Except as contained in this notice, the name of the Xilinx shall not be used
* in advertising or otherwise to promote the sale, use or other dealings in
* this Software without prior written authorization from Xilinx.
*
******************************************************************************/
//
// Compact AES crypto
//
//  Rev. 1.0    Initial release for 256 bit encryption
//
#include "CompactAES.h"
#include "xil_types.h"
#include <string.h>

#define AES_MAX_ROUND	14U

//
// GF8 multiplication follows the well-known table route. See, for example,
// the Wikipedia article https://en.wikipedia.org/wiki/Rijndael_MixColumns
//

// Multiplication macro by small constants in GF8
#define MulBy1(x)   (x)
#define MulBy2(x)   ((unsigned char)(((x) << 1U) ^ ((((x) >> 7U) & 1U) * 0x1BU))) // Reduction by x^8 + x^4 + x^3 + x + 1
#define MulBy3(x)   (MulBy2(x) ^ MulBy1(x))

//
// The following long macros are used to fill the tables with GF8 multiples of Sbox values
// The idea of using the preprocessor to calculate the tables is well-known in
// the public-domain code, but no actual code is re-used here. The macro argument is the
// multiplication macro from the preceding set.
//

#define SB(func) { \
    func(0x63), func(0x7c), func(0x77), func(0x7b), func(0xf2), func(0x6b), func(0x6f), func(0xc5), \
    func(0x30), func(0x01), func(0x67), func(0x2b), func(0xfe), func(0xd7), func(0xab), func(0x76), \
    func(0xca), func(0x82), func(0xc9), func(0x7d), func(0xfa), func(0x59), func(0x47), func(0xf0), \
    func(0xad), func(0xd4), func(0xa2), func(0xaf), func(0x9c), func(0xa4), func(0x72), func(0xc0), \
    func(0xb7), func(0xfd), func(0x93), func(0x26), func(0x36), func(0x3f), func(0xf7), func(0xcc), \
    func(0x34), func(0xa5), func(0xe5), func(0xf1), func(0x71), func(0xd8), func(0x31), func(0x15), \
    func(0x04), func(0xc7), func(0x23), func(0xc3), func(0x18), func(0x96), func(0x05), func(0x9a), \
    func(0x07), func(0x12), func(0x80), func(0xe2), func(0xeb), func(0x27), func(0xb2), func(0x75), \
    func(0x09), func(0x83), func(0x2c), func(0x1a), func(0x1b), func(0x6e), func(0x5a), func(0xa0), \
    func(0x52), func(0x3b), func(0xd6), func(0xb3), func(0x29), func(0xe3), func(0x2f), func(0x84), \
    func(0x53), func(0xd1), func(0x00), func(0xed), func(0x20), func(0xfc), func(0xb1), func(0x5b), \
    func(0x6a), func(0xcb), func(0xbe), func(0x39), func(0x4a), func(0x4c), func(0x58), func(0xcf), \
    func(0xd0), func(0xef), func(0xaa), func(0xfb), func(0x43), func(0x4d), func(0x33), func(0x85), \
    func(0x45), func(0xf9), func(0x02), func(0x7f), func(0x50), func(0x3c), func(0x9f), func(0xa8), \
    func(0x51), func(0xa3), func(0x40), func(0x8f), func(0x92), func(0x9d), func(0x38), func(0xf5), \
    func(0xbc), func(0xb6), func(0xda), func(0x21), func(0x10), func(0xff), func(0xf3), func(0xd2), \
    func(0xcd), func(0x0c), func(0x13), func(0xec), func(0x5f), func(0x97), func(0x44), func(0x17), \
    func(0xc4), func(0xa7), func(0x7e), func(0x3d), func(0x64), func(0x5d), func(0x19), func(0x73), \
    func(0x60), func(0x81), func(0x4f), func(0xdc), func(0x22), func(0x2a), func(0x90), func(0x88), \
    func(0x46), func(0xee), func(0xb8), func(0x14), func(0xde), func(0x5e), func(0x0b), func(0xdb), \
    func(0xe0), func(0x32), func(0x3a), func(0x0a), func(0x49), func(0x06), func(0x24), func(0x5c), \
    func(0xc2), func(0xd3), func(0xac), func(0x62), func(0x91), func(0x95), func(0xe4), func(0x79), \
    func(0xe7), func(0xc8), func(0x37), func(0x6d), func(0x8d), func(0xd5), func(0x4e), func(0xa9), \
    func(0x6c), func(0x56), func(0xf4), func(0xea), func(0x65), func(0x7a), func(0xae), func(0x08), \
    func(0xba), func(0x78), func(0x25), func(0x2e), func(0x1c), func(0xa6), func(0xb4), func(0xc6), \
    func(0xe8), func(0xdd), func(0x74), func(0x1f), func(0x4b), func(0xbd), func(0x8b), func(0x8a), \
    func(0x70), func(0x3e), func(0xb5), func(0x66), func(0x48), func(0x03), func(0xf6), func(0x0e), \
    func(0x61), func(0x35), func(0x57), func(0xb9), func(0x86), func(0xc1), func(0x1d), func(0x9e), \
    func(0xe1), func(0xf8), func(0x98), func(0x11), func(0x69), func(0xd9), func(0x8e), func(0x94), \
    func(0x9b), func(0x1e), func(0x87), func(0xe9), func(0xce), func(0x55), func(0x28), func(0xdf), \
    func(0x8c), func(0xa1), func(0x89), func(0x0d), func(0xbf), func(0xe6), func(0x42), func(0x68), \
    func(0x41), func(0x99), func(0x2d), func(0x0f), func(0xb0), func(0x54), func(0xbb), func(0x16), \
}

//
// Three tables (SBOX*1, 2, 3) are all that is needed for encryption
//
static const unsigned char SBx1[256] = SB(MulBy1);
static const unsigned char SBx2[256] = SB(MulBy2);
static const unsigned char SBx3[256] = SB(MulBy3);

//
// Only one key is supported at a time, set up by aesSetupKey()
// Key schedule and rounds are static variables
//
static unsigned char schedule[AES_BLK_SIZE * (AES_MAX_ROUND + 1)];
static unsigned int rounds;

//
// XOR two blocks res^= in
//
static void xorb(AES_BLOCK res,  const AES_BLOCK in)
{
    unsigned int i;
    for (i = 0U; i < AES_BLK_SIZE; ++i)
    {
        res[i] ^= in[i];
    }
 }

//
// Encrypt few blocks in the CBC mode. If out==NULL, just calculate
// the CBC checksum
//
// Updates the IV to allow the chaining operation
// Key shall be set up prior to invocation
//

void aesCbcEncrypt(unsigned char* in, unsigned char* out, AES_BLOCK iv, int max_blk)
{

    int max_blk_int = max_blk;

    while (max_blk_int > 0)
    {
        xorb(iv, in);
        aesEncrypt(iv, iv);
        if (out != NULL)
        {
            (void)memcpy(out, iv, AES_BLK_SIZE);
            out += AES_BLK_SIZE;
        }
        in += AES_BLK_SIZE;
        max_blk_int -= 1;
    }
}

static void applyKey(AES_BLOCK res, const AES_BLOCK src, unsigned int roundval)
{
    (void)memcpy(res, src, AES_BLK_SIZE);
    xorb(res, schedule + roundval * AES_BLK_SIZE);
}

//
// The split between mixColumnsSbox() and shiftRowsSbox() follows the Karl Malbrain's
// public-domain code. No actual public-domain (or other third-party) code is re-used here.
//

static void mixColumnsSbox(AES_BLOCK dst, const AES_BLOCK state)
{
    dst[0]  = SBx2[state[0]]  ^ SBx3[state[5]]  ^ SBx1[state[10]] ^ SBx1[state[15]];
    dst[1]  = SBx1[state[0]]  ^ SBx2[state[5]]  ^ SBx3[state[10]] ^ SBx1[state[15]];
    dst[2]  = SBx1[state[0]]  ^ SBx1[state[5]]  ^ SBx2[state[10]] ^ SBx3[state[15]];
    dst[3]  = SBx3[state[0]]  ^ SBx1[state[5]]  ^ SBx1[state[10]] ^ SBx2[state[15]];
    dst[4]  = SBx2[state[4]]  ^ SBx3[state[9]]  ^ SBx1[state[14]] ^ SBx1[state[3]];
    dst[5]  = SBx1[state[4]]  ^ SBx2[state[9]]  ^ SBx3[state[14]] ^ SBx1[state[3]];
    dst[6]  = SBx1[state[4]]  ^ SBx1[state[9]]  ^ SBx2[state[14]] ^ SBx3[state[3]];
    dst[7]  = SBx3[state[4]]  ^ SBx1[state[9]]  ^ SBx1[state[14]] ^ SBx2[state[3]];
    dst[8]  = SBx2[state[8]]  ^ SBx3[state[13]] ^ SBx1[state[2]]  ^ SBx1[state[7]];
    dst[9]  = SBx1[state[8]]  ^ SBx2[state[13]] ^ SBx3[state[2]]  ^ SBx1[state[7]];
    dst[10] = SBx1[state[8]]  ^ SBx1[state[13]] ^ SBx2[state[2]]  ^ SBx3[state[7]];
    dst[11] = SBx3[state[8]]  ^ SBx1[state[13]] ^ SBx1[state[2]]  ^ SBx2[state[7]];
    dst[12] = SBx2[state[12]] ^ SBx3[state[1]]  ^ SBx1[state[6]]  ^ SBx1[state[11]];
    dst[13] = SBx1[state[12]] ^ SBx2[state[1]]  ^ SBx3[state[6]]  ^ SBx1[state[11]];
    dst[14] = SBx1[state[12]] ^ SBx1[state[1]]  ^ SBx2[state[6]]  ^ SBx3[state[11]];
    dst[15] = SBx3[state[12]] ^ SBx1[state[1]]  ^ SBx1[state[6]]  ^ SBx2[state[11]];
}

#define SBOX4(a, b, c, d)   do   { a = SBx1[a]; b = SBx1[b]; c = SBx1[c]; d = SBx1[d]; } while(0)
#define ROTATE4(a, b, c, d) do   { unsigned char t; t = a; a = SBx1[b]; b = SBx1[c]; c = SBx1[d]; d = SBx1[t]; } while(0)
#define ROTATE2(a, b)       do   { unsigned char t; t = a; a = SBx1[b]; b = SBx1[t]; } while(0)

static void shiftRowsSbox(AES_BLOCK state)
{
    // Row 0 - use Sbox only
    SBOX4(state[0], state[4], state[8], state[12]);

    // Rows 1-3: rotate and apply the Sbox
    ROTATE4(state[1], state[5], state[9], state[13]);

    ROTATE2(state[2], state[10]);
    ROTATE2(state[6], state[14]);

    ROTATE4(state[15], state[11], state[7], state[3]);
}

//
// Encrypt a single AES data block in ECB mode
//
void aesEncrypt(AES_BLOCK in, AES_BLOCK out)
{
    AES_BLOCK stateA, stateB;
    unsigned int roundval;
    applyKey(stateA, in, 0);

    for (roundval = 1; roundval < rounds; ++roundval)
    {
        mixColumnsSbox(stateB, stateA);
        applyKey(stateA, stateB, roundval);
    }
    shiftRowsSbox(stateA);
    applyKey(out, stateA, roundval);
}

//
// Setup a new key
// Returns:
//   0 = OK
//   1 = Unsupported key length
//

int aesSetupKey(const unsigned char *k, int klen)
{
    unsigned char rcon = 1;
    int i, sch_size = 240;

    if (klen != 32)
    {
        return 1;
    }
    rounds = 14;

    //
    // Use a traditional byte-wide expansion scheme
    //
    (void)memcpy(schedule, k, (unsigned int)klen);
    for (i = klen; i < sch_size; i += 4)
    {
        unsigned char t0, t1, t2, t3;
        int ik;

        t0 = schedule[i - 4];
        t1 = schedule[i - 3];
        t2 = schedule[i - 2];
        t3 = schedule[i - 1];
        if (i % klen == 0)
        {
            ROTATE4(t0, t1, t2, t3);
            t0 ^= rcon;
            rcon = MulBy2(rcon);
        }
        else if (i % klen == 16)
        {
            SBOX4(t0, t1, t2, t3);
        }
        else
        {
            /* for MISRA-C */
        }
        ik = i - klen;
        schedule[i + 0] = schedule[ik + 0] ^ t0;
        schedule[i + 1] = schedule[ik + 1] ^ t1;
        schedule[i + 2] = schedule[ik + 2] ^ t2;
        schedule[i + 3] = schedule[ik + 3] ^ t3;
    }
    return 0;
}
