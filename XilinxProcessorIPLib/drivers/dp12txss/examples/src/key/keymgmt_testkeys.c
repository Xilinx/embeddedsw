/******************************************************************************
* Copyright (C) 2020 - 2021 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
* @file keymgmt_testkeys.c
*
* This file contains the table definitions for the four sets of hdcp test keys
* These keys are bogus and HDCP will fail with them
*
******************************************************************************/


/* Include Files */
#include "keymgmt.h"
#include <stdint.h>


/*****************************************************************************/
/**
*
* This table defines the table for the A1 set of test keys and ksv
*
******************************************************************************/
const uint64_t KEYMGMT_TESTKEYS_A1[] =
{
  0x0000B70361F714,
  0x4DA4588F131E69,
  0x1F823558E65009,
  0x8A6A47ABB9980D,
  0xF3181B52CBC5CA,
  0xFB147F6896D8B4,
  0xE08BC978488F81,
  0xA0D064C8112C41,
  0xB39D5A28242044,
  0xB928B2BDAD566B,
  0x91A47B4A6CE4F6,
  0x5600F8205E9D58,
  0x8C7FB706EE3FA0,
  0xC02D8C9D7CBC28,
  0x561261E54B9F05,
  0x74F0DE8CCAC1CB,
  0x3BB8F60EFCDB6A,
  0xA02BFFFFFFFFFF,
  0xA02BFFFFFFFFFF,
  0xA02BFFFFFFFFFF,
  0xA02BFFFFFFFFFF,
  0xA02BFFFFFFFFFF,
  0xA02BFFFFFFFFFF,
  0xA02BFFFFFFFFFF,
  0xA02BFFFFFFFFFF,
  0xA02BFFFFFFFFFF,
  0xA02BFFFFFFFFFF,
  0xA02BFFFFFFFFFF,
  0xA02BFFFFFFFFFF,
  0xA02BFFFFFFFFFF,
  0xA02BFFFFFFFFFF,
  0xA02BFFFFFFFFFF,
  0xA02BFFFFFFFFFF,
  0xA02BFFFFFFFFFF,
  0xA02BFFFFFFFFFF,
  0xA02BFFFFFFFFFF,
  0xA02BFFFFFFFFFF,
  0xA02BFFFFFFFFFF,
  0xA02BFFFFFFFFFF,
  0xA02BFFFFFFFFFF,
  0xA02BFFFFFFFFFF,
};


/*****************************************************************************/
/**
*
* This table defines the table for the A2 set of test keys and ksv
*
******************************************************************************/
const uint64_t KEYMGMT_TESTKEYS_A2[] =
{
  0x000043f72d5066,
  0x9aaba1f9ef907c,
  0x34a0407731d1d0,
  0x97c682992dc5d9,
  0xda80caca68ed15,
  0x1866d9b51462a6,
  0xd9fc9599bb7498,
  0x7a062ac883f528,
  0xf5938c662af454,
  0xec3075e83d3ef2,
  0x536e376e7ffc49,
  0x51c83a6cbeb116,
  0x79d44ae1bd5f50,
  0x674b2563e27393,
  0x7a1357efc538a2,
  0x6486e57ea46b02,
  0xbdf27a1ce8a299,
  0xdc8bd1fa5b46b9,
  0xA02BFFFFFFFFFF,
  0xA02BFFFFFFFFFF,
  0xA02BFFFFFFFFFF,
  0xA02BFFFFFFFFFF,
  0xA02BFFFFFFFFFF,
  0xA02BFFFFFFFFFF,
  0xA02BFFFFFFFFFF,
  0xA02BFFFFFFFFFF,
  0xA02BFFFFFFFFFF,
  0xA02BFFFFFFFFFF,
  0xA02BFFFFFFFFFF,
  0xA02BFFFFFFFFFF,
  0xA02BFFFFFFFFFF,
  0xA02BFFFFFFFFFF,
  0xA02BFFFFFFFFFF,
  0xA02BFFFFFFFFFF,
  0xA02BFFFFFFFFFF,
  0xA02BFFFFFFFFFF,
  0xA02BFFFFFFFFFF,
  0xA02BFFFFFFFFFF,
  0xA02BFFFFFFFFFF,
  0xA02BFFFFFFFFFF,
};


/*****************************************************************************/
/**
*
* This table defines the table for the B1 set of test keys and ksv
*
******************************************************************************/
const uint64_t KEYMGMT_TESTKEYS_B1[] =
{
  0x0000511ef21acd,
  0xbc13e0c75bf0fd,
  0xae0d2c7f76443b,
  0x24bf2185a36c60,
  0xf4bc6cbcd7a32f,
  0xa72e69c5eb6388,
  0x7fa2d27a37d9f8,
  0x32fd3529dea3d1,
  0x485fc240cc9bae,
  0x3b9857797d5103,
  0x0dd170be615250,
  0x1a748be4866bb1,
  0xf9606a7c348cca,
  0x4bbb037899eea1,
  0x190ecf9cc095a9,
  0xa821c46897447f,
  0x1a8a0bc4298a41,
  0xaefc0853e62082,
  0xf75d4a0c497ba4,
  0xA02BFFFFFFFFFF,
  0xA02BFFFFFFFFFF,
  0xA02BFFFFFFFFFF,
  0xA02BFFFFFFFFFF,
  0xA02BFFFFFFFFFF,
  0xA02BFFFFFFFFFF,
  0xA02BFFFFFFFFFF,
  0xA02BFFFFFFFFFF,
  0xA02BFFFFFFFFFF,
  0xA02BFFFFFFFFFF,
  0xA02BFFFFFFFFFF,
  0xA02BFFFFFFFFFF,
  0xA02BFFFFFFFFFF,
  0xA02BFFFFFFFFFF,
  0xA02BFFFFFFFFFF,
  0xA02BFFFFFFFFFF,
  0xA02BFFFFFFFFFF,
  0xA02BFFFFFFFFFF,
  0xA02BFFFFFFFFFF,
  0xA02BFFFFFFFFFF,
  0xA02BFFFFFFFFFF,
  0xA02BFFFFFFFFFF,
};


/*****************************************************************************/
/**
*
* This table defines the table for the B2 set of test keys and ksv
*
******************************************************************************/
const uint64_t KEYMGMT_TESTKEYS_B2[] =
{
  0x0000e72697f401,
  0x93afe1ff4ca0ed,
  0xefb49d4a25a4e4,
  0xe822d8a9335346,
  0x8812c3004e23d2,
  0xdc63ba78d94263,
  0x47ebdf52776fd5,
  0x4bce49472e0464,
  0x0479bed7732682,
  0xc5f800fad716d5,
  0xf53fd67ba9b9ec,
  0x6fb3901e5867f2,
  0x24c46f520f1be5,
  0x2038176d369ed7,
  0x9ba9cd6a077a57,
  0x5f2764b35c5591,
  0xee32f1171f5356,
  0xA02BFFFFFFFFFF,
  0xA02BFFFFFFFFFF,
  0xA02BFFFFFFFFFF,
  0xA02BFFFFFFFFFF,
  0xA02BFFFFFFFFFF,
  0xA02BFFFFFFFFFF,
  0xA02BFFFFFFFFFF,
  0xA02BFFFFFFFFFF,
  0xA02BFFFFFFFFFF,
  0xA02BFFFFFFFFFF,
  0xA02BFFFFFFFFFF,
  0xA02BFFFFFFFFFF,
  0xA02BFFFFFFFFFF,
  0xA02BFFFFFFFFFF,
  0xA02BFFFFFFFFFF,
  0xA02BFFFFFFFFFF,
  0xA02BFFFFFFFFFF,
  0xA02BFFFFFFFFFF,
  0xA02BFFFFFFFFFF,
  0xA02BFFFFFFFFFF,
  0xA02BFFFFFFFFFF,
  0xA02BFFFFFFFFFF,
  0xA02BFFFFFFFFFF,
  0xA02BFFFFFFFFFF,
};
