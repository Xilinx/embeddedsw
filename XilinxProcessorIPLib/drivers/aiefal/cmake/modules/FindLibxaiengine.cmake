# FindLibxaiengine
# --------
#
# Find libxaiengine
#
# Find the native libxaiengine includes and library this module defines
#
# ::
#
#   LIBXAIENGINE_INCLUDE_DIR, where to find xaiengine.h, etc.

find_path(LIBXAIENGINE_INCLUDE_DIR NAMES xaiengine.h PATHS ${CMAKE_FIND_ROOT_PATH})
find_library(LIBXAIENGINE_LIB NAMES xaiengine PATHS ${CMAKE_FIND_ROOT_PATH})
get_filename_component(LIBXAIENGINE_LIB_DIR ${LIBXAIENGINE_LIB} DIRECTORY)

include (FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS (LIBXAIENGINE DEFAULT_MSG LIBXAIENGINE_LIB LIBXAIENGINE_INCLUDE_DIR)

mark_as_advanced (LIBXAIENGINE_LIB LIBXAIENGINE_INCLUDE_DIR LIBXAIENGINE_LIB_DIR)
