###############################################################################
# Copyright (C) 2021 Xilinx, Inc.  All rights reserved.
# SPDX-License-Identifier: MIT
#
###############################################################################
# Steps to create BSP, FSBL application and Test application.

## Creating the BSP:
1. File > New > Board Support Package.
2. Click on 'Specify' and Browse the design file(hdf) contains rfdc IP and Click 'Finish'.
3. Rename the Project name (if required) and Click 'Finish'.
3. Select the Libmetal library in the Supported Libraries (Libmetal library is required to compile rfdc driver) and Click 'OK'.

## Creating the FSBL:
1. File > New > Application Project
2. Give some Project name.
3. Under Board Support Package select Use existing and select the existing bsp in the drop down list.
4. Click 'Next', select the ZynqMP FSBL and Click 'Finish'.

## Creating the Application:
1. File > New > Application Project
2. Give some Project name.
3. Under Board Support Package select Use existing and select the existing bsp in the drop down list.
4. Click 'Next', select the Empty Application and Click 'Finish'.
5. Copy the test application in the src directory of the application project to create the binary file(.elf)
