/******************************************************************************
* Copyright (C) 2017 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
 ******************************************************************************/

/*****************************************************************************/
/**
 *
 * @file sensor_cfgs.h
 *
 * This header file contains the definitions for structures for video pipeline
 * and extern declarations.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who    Date     Changes
 * ----- ------ -------- --------------------------------------------------
 * 1.00  pg    12/07/17 Initial release.
 * </pre>
 *
 ******************************************************************************/

#ifndef SENSOR_CFGS_H_
#define SENSOR_CFGS_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "xvidc.h"

typedef enum
{
	XVIDSRC_SENSOR,
	XVIDSRC_TPG
} XVideo_Source;

typedef enum
{
	XVIDDES_HDMI,
	XVIDDES_DSI
} XVideo_Destn;

typedef enum
{
	XCSI_PXLFMT_RAW8 = 0x2A,
	XCSI_PXLFMT_RAW10,
	XCSI_PXLFMT_RAW12
} XCsi_PxlFmt;

typedef struct {
	/* Bits per component */
	XVidC_ColorDepth ColorDepth;
	/* Pipeline Video Source CSI or TPG */
	XVideo_Source VideoSrc;
	/* Pipeline Video Destination DSI or HDMI */
	XVideo_Destn VideoDestn;
	/* Resolution */
	XVidC_VideoMode VideoMode;
	/* Number of Active Lanes */
	u8 ActiveLanes;
	/* Live video or sensor pattern */
	u8 Live;
	/* Flip video vertical flag */
	u8 Vflip;
	/* Flip video horizontally flag */
	u8 Hflip;
	/* camera connected flag */
	u8 CameraPresent;
	/* display panel present flag */
	u8 DSIDisplayPresent;
} XPipeline_Cfg;

/* I2C sensor data structure */
struct regval_list {
	u16 Address;
	u8  Data;
};

extern struct regval_list imx274_config_4K_30fps_regs[];
extern const int length_imx274_config_4K_30fps_regs;

extern struct regval_list imx274_config_4K_60fps_regs[];
extern const int length_imx274_config_4K_60fps_regs;

extern struct regval_list imx274_config_1080p_60fps_regs[];
extern const int length_imx274_config_1080p_60fps_regs;

extern struct regval_list imx274_config_1080p_30fps_regs[];
extern const int length_imx274_config_1080p_30fps_regs;

extern struct regval_list imx274_config_720p_60fps_regs[];
extern const int length_imx274_config_720p_60fps_regs;

/* ANSI Colors */
#define TXT_RED     "\x1b[31m"
#define TXT_GREEN   "\x1b[32m"
#define TXT_YELLOW  "\x1b[33m"
#define TXT_BLUE    "\x1b[34m"
#define TXT_MAGENTA "\x1b[35m"
#define TXT_CYAN    "\x1b[36m"
#define TXT_RST   "\x1b[0m"

#ifdef __cplusplus
}
#endif

#endif /* SENSOR_CFGS_H_ */
