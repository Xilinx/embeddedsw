file(READ ${OPENAMP_ROOT_DIR}/VERSION ver)

string(REGEX MATCH "VERSION_MAJOR = ([0-9]*)" _ ${ver})
set(PROJECT_VERSION_MAJOR ${CMAKE_MATCH_1})

string(REGEX MATCH "VERSION_MINOR = ([0-9]*)" _ ${ver})
set(PROJECT_VERSION_MINOR ${CMAKE_MATCH_1})

string(REGEX MATCH "VERSION_PATCH = ([0-9]*)" _ ${ver})
set(PROJECT_VERSION_PATCH ${CMAKE_MATCH_1})

set(PROJECT_VERSION ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH})

message(STATUS "open-amp version: ${PROJECT_VERSION} (${OPENAMP_ROOT_DIR})")

add_definitions( -DOPENAMP_VERSION_MAJOR=${PROJECT_VERSION_MAJOR} )
add_definitions( -DOPENAMP_VERSION_MINOR=${PROJECT_VERSION_MINOR} )
add_definitions( -DOPENAMP_VERSION_PATCH=${PROJECT_VERSION_PATCH} )
add_definitions( -DOPENAMP_VERSION="${PROJECT_VERSION}" )

if (NOT DEFINED CMAKE_BUILD_TYPE)
  set (CMAKE_BUILD_TYPE Debug)
endif (NOT DEFINED CMAKE_BUILD_TYPE)

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

string (TOLOWER ${CMAKE_SYSTEM_NAME}      PROJECT_SYSTEM)
string (TOUPPER ${CMAKE_SYSTEM_NAME}      PROJECT_SYSTEM_UPPER)
string (TOLOWER ${CMAKE_SYSTEM_PROCESSOR} PROJECT_PROCESSOR)
string (TOUPPER ${CMAKE_SYSTEM_PROCESSOR} PROJECT_PROCESSOR_UPPER)
string (TOLOWER ${MACHINE}                PROJECT_MACHINE)
string (TOUPPER ${MACHINE}                PROJECT_MACHINE_UPPER)

# Select which components are in the openamp lib
option (WITH_PROXY          "Build with proxy(access device controlled by other processor)" ON)
option (WITH_APPS           "Build with sample applications" OFF)
option (WITH_PROXY_APPS     "Build with proxy sample applications" OFF)
if (WITH_APPS)
  if (WITH_PROXY)
    set (WITH_PROXY_APPS ON)
  endif (WITH_PROXY)
endif (WITH_APPS)

# LOAD_FW only allowed for R5, otherwise turn off
if (NOT ${MACHINE} STREQUAL "zynqmp_r5")
 set (WITH_LOAD_FW OFF)
endif(NOT ${MACHINE} STREQUAL "zynqmp_r5")

option (WITH_VIRTIO_DRIVER "Build with virtio driver (front end)  enabled" ON)
option (WITH_VIRTIO_DEVICE "Build with virtio device (back end)  enabled" ON)
option (WITH_VIRTIO_MASTER "Build with virtio driver (front end)  enabled" OFF)
option (WITH_VIRTIO_SLAVE "Build with virtio device (back end)  enabled" OFF)

if (WITH_VIRTIO_MASTER)
  message(DEPRECATION "deprecated cmake option replaced by WITH_VIRTIO_DRIVER" ...)
endif (WITH_VIRTIO_MASTER)
if (WITH_VIRTIO_SLAVE)
  message(DEPRECATION "deprecated cmake option replaced by WITH_VIRTIO_DEVICE" ...)
endif (WITH_VIRTIO_SLAVE)

if (NOT WITH_VIRTIO_DRIVER AND NOT WITH_VIRTIO_MASTER)
	add_definitions(-DVIRTIO_DEVICE_ONLY)
endif (NOT WITH_VIRTIO_DRIVER AND NOT WITH_VIRTIO_MASTER)

if (NOT WITH_VIRTIO_DEVICE AND NOT WITH_VIRTIO_SLAVE)
	add_definitions(-DVIRTIO_DRIVER_ONLY)
endif (NOT WITH_VIRTIO_DEVICE AND NOT WITH_VIRTIO_SLAVE)

option (WITH_DCACHE_VRINGS "Build with vrings cache operations enabled" OFF)

if (WITH_DCACHE_VRINGS)
  add_definitions(-DVIRTIO_CACHED_VRINGS)
endif (WITH_DCACHE_VRINGS)

option (WITH_DCACHE_BUFFERS "Build with vrings cache operations enabled" OFF)

if (WITH_DCACHE_BUFFERS)
  add_definitions(-DVIRTIO_CACHED_BUFFERS)
endif (WITH_DCACHE_BUFFERS)

# Set the complication flags
set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra")

option (WITH_STATIC_LIB "Build with a static library" ON)

if ("${PROJECT_SYSTEM}" STREQUAL "linux")
  option (WITH_SHARED_LIB "Build with a shared library" ON)
endif ("${PROJECT_SYSTEM}" STREQUAL "linux")

if (WITH_ZEPHYR)
  option (WITH_ZEPHYR_LIB "Build open-amp as a zephyr library" OFF)
endif (WITH_ZEPHYR)

option (WITH_LIBMETAL_FIND "Check Libmetal library can be found" ON)

if (DEFINED RPMSG_BUFFER_SIZE)
  add_definitions( -DRPMSG_BUFFER_SIZE=${RPMSG_BUFFER_SIZE} )
endif (DEFINED RPMSG_BUFFER_SIZE)

message ("-- C_FLAGS : ${CMAKE_C_FLAGS}")
