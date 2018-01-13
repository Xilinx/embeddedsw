/** \page example Examples
You can refer to the below stated example applications for more details which
gives an idea of how the USB and its driver can be used for Bulk and Interrupt
transfers.

@section ex1 xusb_cp9.c
Contains an example on how to use the Usb driver directly.
This example contains the USB Chapter 9 related functions.

For details, see xusb_cp9.c.

@section ex2 xusb_cp9.h
This headerfile contains the constants, typedefs, variables and functions
prototypes related to the USB Chapter 9 related code.

For details, see xusb_cp9.h.

@section ex3 xusb_ch9_storage.c
Contains an example on how to use the Usb driver directly.
This is USB Bulk Transfer example which contains Mass storage Class related
Chapter 9 functions.

For details, see xusb_ch9_storage.c.

@section ex4 xusb_ch9_storage.h
This headerfile contains the constants, typedefs, variables and functions
prototypes related to the USB Mass Storage Class Chapter 9 related code.

For details, see xusb_ch9_storage.h.

@section ex5 xusb_class_storage.c
Contains an example on how to use the Usb driver directly.
This is USB Bulk Transfer example which contains Mass storage Class related
functions.

For details, see xusb_class_storage.c.

@section ex6 xusb_class_storage.h
This headerfile contains the constants, type definitions, variables and
function prototypes used in the Mass Storage Class related code.

For details, see xusb_class_storage.h.

@section ex7 xusb_intr_example.c
Contains an example on how to use the Usb driver directly.
This example contains Mass storage device application which provides a
reference to create a new Bulk Transfer related application.

For details, see xusb_intr_example.c

@section ex8 xusb_ch9_dfu.c
Contains an example on how to use the Usb driver directly.
This is the USB DFU example which contains USB DFU Class related
Chapter 9 functions.

For details, see xusb_ch9_dfu.c.

@section ex9 xusb_ch9_dfu.h
This headerfile contains the constants, type definitions, variables and
function prototypes used in the USB DFU Class related Chapter 9 code.

For details, see xusb_ch9_dfu.h.

@section ex10 xusb_class_dfu.c
Contains an example on how to use the Usb driver directly.
This is the USB DFU example which contains USB DFU Class related
functions.

For details, see xusb_class_dfu.c.

@section ex11 xusb_class_dfu.h
This headerfile contains the constants, type definitions, variables and
function prototypes used in the USB DFU Class related code.

For details, see xusb_class_dfu.h.


@subsection Notes
 - These examples are independent from one another. All the examples should
include the xusb_cp9.c file.
 - For Mass Storage example, storage disk will mounted on the host and user
can use "dd" command or can create a file system to transfer the data on the
mounted storage disk.
*/
