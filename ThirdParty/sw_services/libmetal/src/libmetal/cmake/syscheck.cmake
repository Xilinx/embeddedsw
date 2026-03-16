# use "Generic" as CMAKE_SYSTEM_NAME

if (WITH_ZEPHYR)
  set (CMAKE_SYSTEM_NAME "Generic"             CACHE STRING "")
  string (TOLOWER "Zephyr"                PROJECT_SYSTEM)
  string (TOUPPER "Zephyr"                PROJECT_SYSTEM_UPPER)
  if (NOT WITH_ZEPHYR_LIB)
    include($ENV{ZEPHYR_BASE}/cmake/app/boilerplate.cmake NO_POLICY_SCOPE)
  endif()
  # map zephyr arch to libmetal machine
  set (MACHINE "${CONFIG_ARCH}" CACHE STRING "")

endif (WITH_ZEPHYR)
