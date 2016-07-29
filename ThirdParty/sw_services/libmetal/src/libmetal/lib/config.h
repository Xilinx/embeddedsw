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
 * @file	config.h
 * @brief	Generated configuration settings for libmetal.
 */

#ifndef __METAL_CONFIG__H__
#define __METAL_CONFIG__H__

#ifdef __cplusplus
extern "C" {
#endif

/** Library major version number. */
#define METAL_VER_MAJOR		@PROJECT_VER_MAJOR@

/** Library minor version number. */
#define METAL_VER_MINOR		@PROJECT_VER_MINOR@

/** Library patch level. */
#define METAL_VER_PATCH		@PROJECT_VER_PATCH@

/** Library version string. */
#define METAL_VER		"@PROJECT_VER@"

/** System type (linux, generic, ...). */
#define METAL_SYSTEM		"@PROJECT_SYSTEM@"
#define METAL_SYSTEM_@PROJECT_SYSTEM_UPPER@

/** Processor type (arm, x86_64, ...). */
#define METAL_PROCESSOR		"@PROJECT_PROCESSOR@"
#define METAL_PROCESSOR_@PROJECT_PROCESSOR_UPPER@

/** Machine type (zynq, zynqmp, ...). */
#define METAL_MACHINE		"@PROJECT_MACHINE@"
#define METAL_MACHINE_@PROJECT_MACHINE_UPPER@

#cmakedefine HAVE_STDATOMIC_H
#cmakedefine HAVE_FUTEX_H

#ifdef __cplusplus
}
#endif

#endif /* __METAL_CONFIG__H__ */
