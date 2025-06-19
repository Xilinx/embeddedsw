# Copyright (C) 2024 - 2025 Advanced Micro Devices, Inc.  All rights reserved.
# SPDX-License-Identifier: MIT

if("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "microblaze_riscv")
  set(XILASU_Mode "server")
else()
  set(XILASU_Mode "client")
endif()

option(XILASU_trng_enable_drbg_mode "Enables/Disables DRBG mode for TRNG." OFF)
if(XILASU_trng_enable_drbg_mode)
    set(XASU_TRNG_ENABLE_DRBG_MODE " ")
endif()

option(XILASU_enable_perf_measurement "Enables/Disables performance measurement APIs." OFF)
if(XILASU_enable_perf_measurement)
    set(XASU_PERF_MEASUREMENT_ENABLE " ")
endif()

if("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "microblaze_riscv")
    option(XILASU_aes_cm_mode_support "Enables/Disables counter measure mode for AES." ON)
    if(XILASU_aes_cm_mode_support)
        set(XASU_AES_CM_ENABLE " ")
    endif()

    option(XILASU_ecc_cm_mode_support "Enables/Disables counter measure mode for ECC." ON)
    if(XILASU_ecc_cm_mode_support)
        set(XASU_ECC_CM_ENABLE " ")
    endif()

    option(XILASU_ecc_p192_support "Enables/Disables NIST P-192 curve support for ECC in ASUFW." OFF)
    if(XILASU_ecc_p192_support )
        set(XASU_ECC_SUPPORT_NIST_P192 " ")
    endif()

    option(XILASU_ecc_p224_support "Enables/Disables NIST P-224 curve support for ECC in ASUFW." OFF)
    if(XILASU_ecc_p224_support )
        set(XASU_ECC_SUPPORT_NIST_P224 " ")
    endif()

    option(XILASU_ecc_p256_support "Enables/Disables NIST P-256 curve support for ECC in ASUFW." ON)
    if(XILASU_ecc_p256_support )
        set(XASU_ECC_SUPPORT_NIST_P256 " ")
    endif()

    option(XILASU_ecc_p384_support "Enables/Disables NIST P-384 curve support for ECC in ASUFW." OFF)
    if(XILASU_ecc_p384_support )
        set(XASU_ECC_SUPPORT_NIST_P384 " ")
    endif()

    option(XILASU_ecc_p521_support "Enables/Disables NIST P-521 curve support for ECC in ASUFW." OFF)
    if(XILASU_ecc_p521_support )
        set(XASU_ECC_SUPPORT_NIST_P521 " ")
    endif()

    option(XILASU_ecc_bp_p256_support "Enables/Disables BRAINPOOL P-256 curve support for ECC in ASUFW." OFF)
    if(XILASU_ecc_bp_p256_support )
        set(XASU_ECC_SUPPORT_BRAINPOOL_P256 " ")
    endif()

    option(XILASU_ecc_bp_p320_support "Enables/Disables BRAINPOOL P-320 curve support for ECC in ASUFW." OFF)
    if(XILASU_ecc_bp_p320_support )
        set(XASU_ECC_SUPPORT_BRAINPOOL_P320 " ")
    endif()

    option(XILASU_ecc_bp_p384_support "Enables/Disables BRAINPOOL P-384 curve support for ECC in ASUFW." OFF)
    if(XILASU_ecc_bp_p384_support )
        set(XASU_ECC_SUPPORT_BRAINPOOL_P384 " ")
    endif()

    option(XILASU_ecc_bp_p512_support "Enables/Disables BRAINPOOL P-512 curve support for ECC in ASUFW." OFF)
    if(XILASU_ecc_bp_p512_support )
        set(XASU_ECC_SUPPORT_BRAINPOOL_P512 " ")
    endif()

    option(XILASU_hmac_support "Enables/Disables HMAC algorithm in ASUFW." ON)
    if(XILASU_hmac_support)
        set(XASU_HMAC_ENABLE " ")
    endif()

    option(XILASU_kdf_support "Enables/Disables KDF algorithm in ASUFW which depends on HMAC." ON)
    if(XILASU_kdf_support)
        set(XASU_KDF_ENABLE " ")
    endif()

    option(XILASU_ecies_support "Enables/Disables ECIES algorithm in ASUFW which depends on HMAC." ON)
    if(XILASU_ecies_support)
        set(XASU_ECIES_ENABLE " ")
    endif()

    option(XILASU_rsa_padding_support "Enables/Disables RSA padding algorithms in ASUFW." ON)
    if(XILASU_rsa_padding_support)
        set(XASU_RSA_PADDING_ENABLE " ")
    endif()

    option(XILASU_keywrap_support "Enables/Disables keywrap algorithm in ASUFW which depends on RSA padding." ON)
    if(XILASU_keywrap_support )
        set(XASU_KEYWRAP_ENABLE " ")
    endif()
endif()
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/xasu_bsp_config.h.in ${CMAKE_BINARY_DIR}/include/xasu_bsp_config.h)
