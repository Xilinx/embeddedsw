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
 * @file	log.h
 * @brief	Logging support for libmetal.
 */

#ifndef __METAL_METAL_LOG__H__
#define __METAL_METAL_LOG__H__

#ifdef __cplusplus
extern "C" {
#endif

/** \defgroup logging Library Logging Interfaces
 *  @{ */

/** Log message priority levels for libmetal. */
enum metal_log_level {
	METAL_LOG_EMERGENCY,	/**< system is unusable.               */
	METAL_LOG_ALERT,	/**< action must be taken immediately. */
	METAL_LOG_CRITICAL,	/**< critical conditions.              */
	METAL_LOG_ERROR,	/**< error conditions.                 */
	METAL_LOG_WARNING,	/**< warning conditions.               */
	METAL_LOG_NOTICE,	/**< normal but significant condition. */
	METAL_LOG_INFO,		/**< informational messages.           */
	METAL_LOG_DEBUG,	/**< debug-level messages.             */
};

/** Log message handler type. */
typedef void (*metal_log_handler)(enum metal_log_level level,
				  const char *format, ...);

/**
 * @brief	Set libmetal log handler.
 * @param[in]	handler	log message handler.
 * @return	0 on success, or -errno on failure.
 */
extern void metal_set_log_handler(metal_log_handler handler);

/**
 * @brief	Get the current libmetal log handler.
 * @return	Current log handler.
 */
extern metal_log_handler metal_get_log_handler(void);

/**
 * @brief	Set the level for libmetal logging.
 * @param[in]	level	log message level.
 */
extern void metal_set_log_level(enum metal_log_level level);

/**
 * @brief	Get the current level for libmetal logging.
 * @return	Current log level.
 */
extern enum metal_log_level metal_get_log_level(void);

/**
 * @brief	Default libmetal log handler.  This handler prints libmetal log
 *		mesages to stderr.
 * @param[in]	level	log message level.
 * @param[in]	format	log message format string.
 * @return	0 on success, or -errno on failure.
 */
extern void metal_default_log_handler(enum metal_log_level level,
				      const char *format, ...);


/**
 * Emit a log message if the log level permits.
 *
 * @param	level	Log level.
 * @param	...	Format string and arguments.
 */
#define metal_log(level, ...)						       \
	((level <= _metal.common.log_level && _metal.common.log_handler) \
	       ? (void)_metal.common.log_handler(level, __VA_ARGS__)	       \
	       : (void)0)

/** @} */

#ifdef __cplusplus
}
#endif

#endif /* __METAL_METAL_LOG__H__ */
