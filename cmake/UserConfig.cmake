# Copyright (C) 2023 Advanced Micro Devices, Inc.  All rights reserved.
# SPDX-License-Identifier: MIT
cmake_minimum_required(VERSION 3.16)

###    USER SETTINGS  START    ###
# Below settings can be customized
# User need to edit it manually as per their needs.

# Add any compiler definitions, they will be added as extra definitions
# Example adding VERBOSE=1 will pass -DVERBOSE=1 to the compiler.
set(USER_COMPILE_DEFINITIONS
""
)

# Add any compiler options, they will be added as extra compiler options
# Example adding -Wall , will pass -Wall to the compiler.
# Brief description of options which can be passed.
# To enable all warning pass -Wall
# To set optimizations   -O0 [None] , -O1 [Optimize] , -O2 [Optimize More], -O3 [Optimize Most] or -Os [Optimize Size]
# To enable profiling -pg [ for gprof ]  or -p [ for prof information ]
# To enable verbose -v
# Note: Do not delete the default option below as the code IntelliSense will not function.
set(USER_COMPILE_OPTIONS
" -U__clang__"
)


# Add any directories below, they will be added as extra include directories.
# Example 1: Adding /proj/data/include will pass -I/proj/data/include
# Example 2: Adding ../../common/include will consider the path as relative to this component directory.
# Example 3: Adding ${CMAKE_SOURCE_DIR}/data/include to add data/include from this project.

set(USER_INCLUDE_DIRECTORIES
""
)

# Add any libraries to be linked below, they will be added as extra libraries.
# User need to update USER_LINK_DIRECTORIES below with these library paths.
set(USER_LINK_LIBRARIES
""
)

# Add any directories to look for the libraries to be linked.
# Example 1: Adding /proj/compression/lib will pass -L/proj/compression/lib to the linker.
# Example adding Adding ../../common/lib will consider the path as relative to this directory. and will pass the path to -L option.
set(USER_LINK_DIRECTORIES
""
)

# Add linker options to be passed, they will be added as extra linker options
# Example : adding -s will pass -s to the linker.
set(USER_LINK_OPTIONS
""
)


###   END OF USER SETTINGS SECTION ####
