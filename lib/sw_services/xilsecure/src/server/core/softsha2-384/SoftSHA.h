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

/* Sizes for SHA-256 */
#define SHA_BLKSIZE	512	/**< Block size in bits */
#define SHA_BLKBYTES	(SHA_BLKSIZE/8)	/**< Block size in bytes */
#define SHA_BLKWORDS	(SHA_BLKBYTES/4)	/**< Block size in words */

#define SHA_VALSIZE	256	/**< Hash value size in bits */
#define SHA_VALBYTES	(SHA_VALSIZE/8)	/**< Hash value size in bytes */
#define SHA_VALWORDS	(SHA_VALBYTES/4)	/**< Hash value size in words */

/* Sizes for SHA-384 */
#define SHA384_BLKSIZE	1024	/**< Block size in bits */
#define SHA384_BLKBYTES	(SHA384_BLKSIZE/8)	/**< Block size in bytes */
#define SHA384_BLKWORDS	(SHA384_BLKBYTES/8)	/**< Block size in words */

#define SHA384_VALSIZE	384	/**< Hash value size in bits */
#define SHA384_VALBYTES	(SHA384_VALSIZE/8)	/**< Hash value size in bytes */
#define SHA384_VALWORDS	(SHA384_VALBYTES/8)	/**< Hash value size in words */

/* 32-bit SHA data type */
typedef unsigned int SHA32;

/**
 * SHA-256 context structure
 * Includes SHA-256 state, coalescing buffer to collect the processed strings, and
 * total byte length counter (used both to manage the buffer and for padding)
 */
typedef struct
{
	SHA32 state[8];	/**< SHA-256 state */
	unsigned char buffer[SHA_BLKBYTES];	/**< Buffer for input data */
	unsigned long long bytes;	/**< Total number of bytes */
} sha2_context;

/* SHA-384/512 data types */
typedef unsigned long long SHADATA;

/**
 * SHA-384 context structure
 * Includes SHA-384 state, coalescing buffer to collect the processed strings, and
 * total byte length counter (used both to manage the buffer and for padding)
 */
typedef struct
{
	SHADATA state[8];	/**< SHA-384 state */
	unsigned char buffer[SHA384_BLKBYTES];	/**< Buffer for input data */
	unsigned long long bytes;	/**< Total number of bytes */
} sha384_context;


#ifdef __cplusplus
extern "C" {
#endif

/*****************************************************************************/
/**
 *
 * @brief SHA-256 user interfaces
 *
 * @param	in		Pointer to input data
 * @param	size	Input data size in bytes
 * @param	out		Pointer to output data (32 bytes)
 *
 * @return	None
 *
 ******************************************************************************/
extern void sha_256(const unsigned char *in, const SHA32 size, unsigned char *out);

/*****************************************************************************/
/**
 *
 * @brief Initialize SHA-256 context
 *
 * @param	ctx	Pointer to the SHA-256 context
 *
 * @return	None
 *
 ******************************************************************************/
extern void sha2_starts(sha2_context *ctx);

/*****************************************************************************/
/**
 * @brief Update SHA-256 context with input data
 *
 * @param	ctx	Pointer to the SHA-256 context
 * @param	input	Pointer to input
 * @param	ilen	Input length in bytes
 *
 * @return	None
 *
 ******************************************************************************/
extern void sha2_update(sha2_context *ctx, unsigned char* input, SHA32 ilen);

/*****************************************************************************/
/**
 * @brief SHA-256 final digest
 *
 * @param	ctx	Pointer to the SHA-256 context
 * @param	output	Pointer to output (32 bytes)
 *
 * @return	None
 *
 ******************************************************************************/
extern void sha2_finish(sha2_context *ctx, unsigned char* output);

/* SHA-384 user interfaces */
/*****************************************************************************/
/**
 * @brief SHA-384 hash function
 *
 * @param	in	Pointer to input data
 * @param	size	Input data size in bytes
 * @param	out	Pointer to output data (48 bytes)
 *
 * @return	None
 *
 ******************************************************************************/
extern void sha_384(const unsigned char* in, const SHA32 size, unsigned char* out);

/*****************************************************************************/
/**
 * @brief Initialize SHA-384 context
 *
 * @param	ctx	Pointer to the SHA-384 context
 *
 * @return	None
 *
 ******************************************************************************/
extern void sha384_starts(sha384_context* ctx);

/*****************************************************************************/
/**
 * @brief Update SHA-384 context with input data
 *
 * @param	ctx	Pointer to the SHA-384 context
 * @param	input	Pointer to input data
 * @param	ilen	Input data length in bytes
 *
 * @return	None
 *
 ******************************************************************************/
extern void sha384_update(sha384_context* ctx, const unsigned char* input, SHA32 ilen);

/*****************************************************************************/
/**
 * @brief SHA-384 final digest
 *
 * @param	ctx	Pointer to the SHA-384 context
 * @param	output	Pointer to output (48 bytes)
 *
 * @return	None
 *
 ******************************************************************************/
extern void sha384_finish(sha384_context* ctx, unsigned char* output);

#ifdef __cplusplus
}
#endif
#endif // SOFTSHA384_H
