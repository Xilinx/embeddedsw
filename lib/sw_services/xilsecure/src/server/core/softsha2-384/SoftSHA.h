#ifndef SOFTSHA384_H
#define SOFTSHA384_H
//
// Software RSA-2048 and SHA-256/384 implementation
// (C) IP Cores, Inc., 2009-2021
//
//  Revision	1.4	 Added PKCS#1 functions
//				1.5  Separated the signature block generation
//				1.8  Added SHA-384
//				1.81 Cosmetic fixes
//				1.85 Cosmetic fixes for MISRA
//				1.86 Removed non-SHA384 functionality per Xilinx request
//

// Sizes for SHA-256
#define SHA_BLKSIZE		512
#define SHA_BLKBYTES	(SHA_BLKSIZE/8)
#define SHA_BLKWORDS	(SHA_BLKBYTES/4)

#define SHA_VALSIZE		256
#define SHA_VALBYTES	(SHA_VALSIZE/8)
#define SHA_VALWORDS	(SHA_VALBYTES/4)

// Sizes for SHA-384
#define SHA384_BLKSIZE	1024
#define SHA384_BLKBYTES	(SHA384_BLKSIZE/8)
#define SHA384_BLKWORDS	(SHA384_BLKBYTES/8)

#define SHA384_VALSIZE	384
#define SHA384_VALBYTES	(SHA384_VALSIZE/8)
#define SHA384_VALWORDS	(SHA384_VALBYTES/8)

//
// 32-bit SHA data type
typedef unsigned int SHA32;

//
// SHA-256 context structure
// Includes SHA-256 state, coalescing buffer to collect the processed strings, and
// total byte length counter (used both to manage the buffer and for padding)
//
typedef struct
{
	SHA32 state[8];
	unsigned char buffer[SHA_BLKBYTES];
	unsigned long long bytes;
} sha2_context;

//
// SHA-384/512 data types
//
typedef unsigned long long SHADATA;

//
// SHA-384 context structure
// Includes SHA-384 state, coalescing buffer to collect the processed strings, and
// total byte length counter (used both to manage the buffer and for padding)
//
typedef struct
{
	SHADATA state[8];
	unsigned char buffer[SHA384_BLKBYTES];
	unsigned long long bytes;
} sha384_context;


#ifdef __cplusplus
extern "C" {
#endif

//
// SHA-256 user interfaces
//
extern void sha_256(const unsigned char *in, const SHA32 size, unsigned char *out);
extern void sha2_starts(sha2_context *ctx);
extern void sha2_update(sha2_context *ctx, unsigned char* input, SHA32 ilen);
extern void sha2_finish(sha2_context *ctx, unsigned char* output);

//
// SHA-384 user interfaces
//
extern void sha_384(const unsigned char* in, const SHA32 size, unsigned char* out);
extern void sha384_starts(sha384_context* ctx);
extern void sha384_update(sha384_context* ctx, const unsigned char* input, SHA32 ilen);
extern void sha384_finish(sha384_context* ctx, unsigned char* output);

#ifdef __cplusplus
}
#endif
#endif // SOFTSHA384_H
