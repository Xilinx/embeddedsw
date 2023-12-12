/*
 * Copyright (c) 2015-2022 Xilinx, Inc. and Contributors. All rights reserved.
 * Copyright (c) 2022-2023 Advanced Micro Devices, Inc. All Rights Reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
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
 *  @{
 */

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
 *		messages to stderr.
 * @param[in]	level	log message level.
 * @param[in]	format	log message format string.
 * @return	0 on success, or -errno on failure.
 */
extern void metal_default_log_handler(enum metal_log_level level,
				      const char *format, ...);

/**
 * @internal
 *
 * @brief	used by the metal_log() macro to update the format string
 *
 * If ML_FUNC_LINE is defined this macro generates a unified format
 * string for metal_log() and its convenience metal_*() macros, i.e. it
 * adds function-name:line-number prefix to all log messages.
 *
 * @param[in]	fmt	format string passed from the metal_log() macro
 */
#if defined(ML_FUNC_LINE)
#define metal_fmt(fmt) "%s:%u " fmt, __func__, __LINE__
#else	/* ML_FUNC_LINE */
#define metal_fmt(fmt) fmt
#endif	/* ML_FUNC_LINE */

/**
 * @brief	Emit a log message if the log level permits.
 *
 * @param	level	Log level.
 * @param	fmt	Format string.
 * @param	args... Variable number of arguments.
 */
#define metal_log(level, fmt, args...) ({				   \
	if (_metal.common.log_handler && level <= _metal.common.log_level) \
		_metal.common.log_handler(level, metal_fmt(fmt), ##args);  \
})

#define metal_err(fmt, args...) metal_log(METAL_LOG_ERROR, fmt, ##args)
#define metal_warn(fmt, args...) metal_log(METAL_LOG_WARNING, fmt, ##args)
#define metal_info(fmt, args...) metal_log(METAL_LOG_INFO, fmt, ##args)
#define metal_dbg(fmt, args...) metal_log(METAL_LOG_DEBUG, fmt, ##args)

/**
 * Convenience macros ML_ERR, ML_INFO, ML_DBG add source
 * function name and the line number before the message.
 * Inspired by pr_err, pr_info, etc. in the kernel's printk.h.
 * Keep the original ML_ERR, ML_INFO, ML_DBG until the open-amp
 * code is converted to use the new metal_*() macros.
 */
#define ML_ERR(fmt, args...) metal_err(fmt, ##args)
#define ML_INFO(fmt, args...) metal_info(fmt, ##args)
#define ML_DBG(fmt, args...) metal_dbg(fmt, ##args)

/** @} */

#ifdef __cplusplus
}
#endif

#include <metal/system/@PROJECT_SYSTEM@/log.h>

#endif /* __METAL_METAL_LOG__H__ */
