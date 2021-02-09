set (PROJECT_VER_MAJOR  0)
set (PROJECT_VER_MINOR  1)
set (PROJECT_VER_PATCH  0)
set (PROJECT_VER        0.1.0)

if (NOT DEFINED CMAKE_BUILD_TYPE)
  set (CMAKE_BUILD_TYPE Debug)
endif (NOT DEFINED CMAKE_BUILD_TYPE)
message ("-- Build type:  ${CMAKE_BUILD_TYPE}")

if (NOT CMAKE_INSTALL_LIBDIR)
  set (CMAKE_INSTALL_LIBDIR "lib")
endif (NOT CMAKE_INSTALL_LIBDIR)

if (NOT CMAKE_INSTALL_BINDIR)
  set (CMAKE_INSTALL_BINDIR "bin")
endif (NOT CMAKE_INSTALL_BINDIR)

set (_host "${CMAKE_HOST_SYSTEM_NAME}/${CMAKE_HOST_SYSTEM_PROCESSOR}")
message ("-- Host:    ${_host}")

set (_target "${CMAKE_SYSTEM_NAME}/${CMAKE_SYSTEM_PROCESSOR}")
message ("-- Target:  ${_target}")

if (NOT DEFINED MACHINE)
  set (MACHINE "Generic")
endif (NOT DEFINED MACHINE)
message ("-- Machine: ${MACHINE}")

# handle if '-' in machine name
string (REPLACE "-" "_" MACHINE ${MACHINE})

if (NOT DEFINED PROJECT_SYSTEM)
  string (TOLOWER ${CMAKE_SYSTEM_NAME}      PROJECT_SYSTEM)
  string (TOUPPER ${CMAKE_SYSTEM_NAME}      PROJECT_SYSTEM_UPPER)
endif (NOT DEFINED PROJECT_SYSTEM)

string (TOLOWER ${CMAKE_SYSTEM_PROCESSOR} PROJECT_PROCESSOR)
string (TOUPPER ${CMAKE_SYSTEM_PROCESSOR} PROJECT_PROCESSOR_UPPER)
string (TOLOWER ${MACHINE}                PROJECT_MACHINE)
string (TOUPPER ${MACHINE}                PROJECT_MACHINE_UPPER)

option (WITH_STATIC_LIB "Build with a static library" ON)

if ("${PROJECT_SYSTEM}" STREQUAL "linux")
  option (WITH_SHARED_LIB "Build with a shared library" ON)
  option (WITH_TESTS      "Install test applications" ON)
endif ("${PROJECT_SYSTEM}" STREQUAL "linux")

if (WITH_TESTS AND (${_host} STREQUAL ${_target}))
	option (WITH_TESTS_EXEC "Run test applications during build" OFF)
endif (WITH_TESTS AND (${_host} STREQUAL ${_target}))

option (WITH_DOC "Build with documentation" ON)

option (WITH_BUILD_XAIEDRV "Build libxaiengine" OFF)
if (NOT WITH_BUILD_XAIEDRV)
  message ("Will not build libxaiengine, please make sure libxaiengine is in lib/include path")
  option (WITH_XAIEDRV_FIND "Find libxaiegnine depends" ON)
else()
  if ("${PROJECT_SYSTEM}" STREQUAL "linux")
    if (NOT ${_host} STREQUAL ${_target})
      option (WITH_AIEDRV_LINUX "Build with cross compile linux backend" ON)
      option (WITH_AIEDRV_LIBMETAL "Build with libmetal backend" OFF)
      if (WITH_AIEDRV_LIBMETAL)
        set (AIEDRV_BACKEND -D__AIEMETAL__)
      elseif (WITH_AIEDRV_LINUX)
        set (AIEDRV_BACKEND -D__AIELINUX__)
      endif (WITH_AIEDRV_LIBMETAL)
    endif(NOT ${_host} STREQUAL ${_target})
  else()
    option (WITH_AIEDRV_BAREMETAL "Build with baremetal backend" ON)
    if (WITH_AIEDRV_BAREMETAL)
      set (AIEDRV_BACKEND -D__AIEBAREMETAL__)
    endif (WITH_AIEDRV_BAREMETAL)
  endif ("${PROJECT_SYSTEM}" STREQUAL "linux")
  message ("Will build libxaiengine")
endif (NOT WITH_BUILD_XAIEDRV)


set_property (GLOBAL PROPERTY "PROJECT_EC_FLAGS" -Wall -Wextra)
if(CMAKE_BUILD_TYPE STREQUAL "coverage" OR CODE_COVERAGE)
    if("${CMAKE_CXX_COMPILER_ID}" MATCHES "(Apple)?[Cc]lang")
        message("Building with llvm Code Coverage Tools")

        # Warning/Error messages
	find_program (LLVM_COV_PATH llvm-cov)
	if(NOT LLVM_COV_PATH)
	    message(FATAL_ERROR "llvm-cov not found! Aborting.")
	endif()

        # set Flags
	get_property(_ec_flags GLOBAL PROPERTY "PROJECT_EC_FLAGS")
	set_property (GLOBAL PROPERTY "PROJECT_EC_FLAGS" ${_ec_flags} -fprofile-instr-generate -fcoverage-mapping)

    elseif(CMAKE_COMPILER_IS_GNUCXX)
        message("Building with lcov Code Coverage Tools")

        # Warning/Error messages
        if(NOT (CMAKE_BUILD_TYPE STREQUAL "Debug"))
            message(WARNING "Code coverage results with an optimized (non-Debug) build may be misleading")
        endif()
	find_program (LCOV_PATH lcov)
        if(NOT LCOV_PATH)
            message(FATAL_ERROR "lcov not found! Aborting...")
        endif()

	get_property(_ec_flags GLOBAL PROPERTY "PROJECT_EC_FLAGS")
	set_property (GLOBAL PROPERTY "PROJECT_EC_FLAGS" ${_ec_flags} -fprofile-arcs -ftest-coverage)
    else()
        message(FATAL_ERROR "Code coverage requires Clang or GCC. Aborting.")
    endif()
endif()
# vim: expandtab:ts=2:sw=2:smartindent
