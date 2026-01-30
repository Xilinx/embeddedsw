# Copyright (C) 2024 - 2026 Advanced Micro Devices, Inc.  All rights reserved.
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
if("${CMAKE_MACHINE}" STREQUAL "Versal")
  if("Versal_2VP" IN_LIST VARIANT)
    add_subdirectory(ecc_keypair)
  endif()
endif()
