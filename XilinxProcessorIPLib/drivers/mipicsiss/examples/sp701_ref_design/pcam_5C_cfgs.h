/******************************************************************************
* Copyright (C) 2018 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/

/*****************************************************************************/
/**
*
* @file pcam_5C_cfgs.h
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who    Date     Changes
* ----- ------ -------- --------------------------------------------------
* X.XX  XX     YY/MM/DD
* 1.00  RHe    19/09/20 Initial release.
* </pre>
*
******************************************************************************/

extern struct regval_list {
        u16 Address;
        u16  Data;
};

extern struct regval_list sensor_pre[];
extern const int length_sensor_pre;
const int size_sensor_pre;

extern struct regval_list pcam5c_mode1[];
extern const int length_pcam5c_mode1;

extern struct regval_list seq_shrt[];
extern const int length_seq_shrt;

extern struct regval_list sensor_list[];
extern const int length_sensor_list;
extern const int size_sensor_list;

extern struct regval_list seq[];
extern const int length_seq;
