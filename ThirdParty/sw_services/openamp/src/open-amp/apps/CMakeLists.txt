collector_create (APP_COMMON_SOURCES "")
collector_create (APP_LIB_DIRS "")
collector_create (APP_INC_DIRS "")
collector_create (APP_LIB_DEPS "")

set (APPS_ROOT_DIR "${CMAKE_CURRENT_SOURCE_DIR}")

set (APPS_SHARE_DIR "${CMAKE_CURRENT_BINARY_DIR}/share")

option (WITH_FUNCTION_TEST_SUITE "Include function test suite" OFF)

add_subdirectory (machine)
add_subdirectory (system)
if (WITH_LINUXREMOTE)
  add_subdirectory (linux_firmware_remoteproc_slave)
endif (WITH_LINUXREMOTE)

add_subdirectory (echo_test)
add_subdirectory (matrix_multiply)
if (WITH_PROXY_APPS)
if (NOT WITH_REMOTEPROC_MASTER)
add_subdirectory (rpc_demo)
endif (NOT WITH_REMOTEPROC_MASTER)
endif (WITH_PROXY_APPS)

if (WITH_FUNCTION_TEST_SUITE)
  add_subdirectory (func_test_suite)
endif (WITH_FUNCTION_TEST_SUITE)
