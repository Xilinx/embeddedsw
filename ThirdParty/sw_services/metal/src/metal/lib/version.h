/*
 * Copyright (c) 2015, Xilinx Inc. and Contributors. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * 3. Neither the name of Xilinx nor the names of its contributors may be used
 *    to endorse or promote products derived from this software without
 *    specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

/*
 * @file	version.h
 * @brief	Library version information for libmetal.
 */

#ifndef __METAL_VERSION__H__
#define __METAL_VERSION__H__

#ifdef __cplusplus
extern "C" {
#endif

/** \defgroup versions Library Version Interfaces
 *  @{ */

/**
 *  @brief	Library major version number.
 *
 *  Return the major version number of the library linked into the application.
 *  This is required to match the value of METAL_VER_MAJOR, which is the major
 *  version of the library that the application was compiled against.
 *
 *  @return	Library major version number.
 *  @see	METAL_VER_MAJOR
 */
extern int metal_ver_major(void);

/**
 *  @brief	Library minor version number.
 *
 *  Return the minor version number of the library linked into the application.
 *  This could differ from the value of METAL_VER_MINOR, which is the minor
 *  version of the library that the application was compiled against.
 *
 *  @return	Library minor version number.
 *  @see	METAL_VER_MINOR
 */
extern int metal_ver_minor(void);

/**
 *  @brief	Library patch level.
 *
 *  Return the patch level of the library linked into the application.  This
 *  could differ from the value of METAL_VER_PATCH, which is the patch level of
 *  the library that the application was compiled against.
 *
 *  @return	Library patch level.
 *  @see	METAL_VER_PATCH
 */
extern int metal_ver_patch(void);

/**
 *  @brief	Library version string.
 *
 *  Return the version string of the library linked into the application.  This
 *  could differ from the value of METAL_VER, which is the version string of
 *  the library that the application was compiled against.
 *
 *  @return	Library version string.
 *  @see	METAL_VER
 */
extern const char *metal_ver(void);

/** @} */

#ifdef __cplusplus
}
#endif

#endif /* __METAL_VERSION__H__ */
