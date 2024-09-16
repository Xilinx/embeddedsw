Copyright (C) 2024 Advanced Micro Devices, Inc. All Rights Reserved.
Building ASUFW from git:
===============================

ASUFW has 2 directories.
	1. src  - It contains the ASUFW source files
	2. misc - It contains miscellaneous files required to compile ASUFW

How to compile ASUFW
===============================
1. Go to the ASUFW src directory "lib/sw_apps/asufw/src"
2. If executables and other artifacts from previous ASUFW build are present, run "make clean" to
   delete them
3. Give "make" to compile the ASUFW with BSP
4. This will create "asu.elf" in the ASUFW src/ directory
