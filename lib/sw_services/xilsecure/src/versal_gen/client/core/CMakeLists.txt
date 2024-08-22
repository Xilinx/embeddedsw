# Copyright (C) 2024 Advanced Micro Devices, Inc.  All rights reserved.
# SPDX-License-Identifier: MIT

add_subdirectory(aes)
add_subdirectory(rsa)
add_subdirectory(ecdsa)
add_subdirectory(crypto_kat)
add_subdirectory(sha)
add_subdirectory(mailbox)
if("${CMAKE_MACHINE}" STREQUAL "VersalNet")
  add_subdirectory(ecc_keypair)
  add_subdirectory(key_zeroize)
  add_subdirectory(trng)
endif()
