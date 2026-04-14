# The MIT License (MIT)
# Copyright (c) 2014-2026 Vivantec Corporation


collect (PROJECT_LIB_SOURCES cJSON.c)

add_subdirectory(submoduels)

collect (PROJECT_LIB_SOURCES vvbench.c)

collect (PROJECT_LIB_SOURCES vvbparser.c)

collect (PROJECT_LIB_SOURCES vvdevice.c)

collect (PROJECT_LIB_SOURCES vvformat.c)

collect (PROJECT_LIB_SOURCES vvpath.c)
