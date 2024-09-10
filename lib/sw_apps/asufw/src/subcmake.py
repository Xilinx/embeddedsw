import os
import sys
import glob
from optparse import OptionParser

parser = OptionParser()
parser.add_option(
    "-d", "--drv-path", action="store", help="Specify the driver relative source path"
)
(options, args) = parser.parse_args()


def sub_cmake():
    path = options.drv_path
    os.chdir(path)
    with open("CMakeLists.txt", "w") as f:
        f.write(
            "# Copyright (C) 2024 Advanced Micro Devices, Inc.  All rights reserved.\n# SPDX-License-Identifier: MIT\n\n"
        )
        for file in os.listdir(os.getcwd()):
            if file.endswith(".c"):
                f.write("collect (PROJECT_LIB_SOURCES %s)\n" % file)
            if file.endswith(".h"):
                f.write("collect (PROJECT_LIB_HEADERS %s)\n" % file)


def main():
    sub_cmake()


if __name__ == "__main__":
    main()
