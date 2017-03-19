/** \page example Examples
You can refer to the below stated example applications for more details which gives an idea of
how the USB and its driver can be used for Bulk and Interrupt tranfers.

@section ex1 xusb_types.h
This headerfile contains the constants, type definitions, variables
as used in the USB chapter 9 and mass storage demo application.

For details, see xusb_types.h.

@section ex2 xusb_cp9.c
Contains an example on how to use the XUsb driver directly.
This example contains the USB Chapter 9 related functions.

For details, see xusb_cp9.c.

@section ex3 xusb_cp9.h
This headerfile contains the constants, typedefs, variables and functions
prototypes related to the USB chapter 9 related code.

For details, see xusb_cp9.h.

@section ex4 xusb_storage.c
Contains an example on how to use the XUsb driver directly.
This is the bulk transfer example which contains Mass storage device
application related functions.

For details, see xusb_storage.c.

@section ex5 xusb_storage.h
This headerfile contains the constants, type definitions, variables and
function prototypes used in the mass storage application.

For details, see xusb_storage.h.

@section ex6 xusb_keyboard.c
Contains an example on how to use the XUsb driver directly.
This is the USB keyboard example which uses the USB for interrupt
transfers.

For details, see xusb_keyboard.c.

@section ex7 xusb_keyboard.h
This headerfile contains the constants, type definitions, variables and
function prototypes used in the USB keyboard example.

For details, see xusb_keyboard.h.

@section ex8 xusb_mouse.c
Contains an example on how to use the XUsb driver directly.
This is the USB mouse example which uses the USB for interrupt
transfers.

For details, see xusb_mouse.c.

@section ex9 xusb_mouse.h
This headerfile contains the constants, type definitions, variables and
function prototypes used in the mouse application.

For details, see xusb_mouse.h.

@section ex10 xusb_dma_intr_storage.c
Contains an example on how to use the XUsb driver directly.
This is the bulk transfer example which contains USB mass storage device
application function with SCSI command processing and related response
preparation being implemented as a part of the interrupt handler.

For details, see xusb_dma_intr_storage.c.

@section ex11 xusb_phy_read_write.c
Contains an example on how to use the XUsb driver directly.
This example demonstrates the ULPI PHY read/write register access.

For details, see xusb_phy_read_write.c.

@section ex12 xusb_microphone.c
Contains an example on how to use the XUsb driver directly.
This example contains the usb Microphone application related functions
and provides a reference as to how an isochronous transfer related
application can be written.

For details, see xusb_microphone.c.

@section ex13 xusb_microphone.h
This headerfile contains the constants, type definitions, variables and
function prototypes used in the usb microphone application.

For details, see xusb_microphone.h.

@section ex14 xusb_storage_polled_mode.c
Contains an example on how to use the XUsb driver directly.
This example contains Mass storage device application related functions
implemented in polled mode.

For details, see xusb_storage_polled_mode.c.

@subsection Notes
 - These examples are independent from one another. All the examples except the
the phy_read_write example should include the xusb_cp9.c file.
 - The constants HID_DEVICES, USB_MOUSE, USB_KEYBOARD and MASS_STORAGE_DEVICE
defined in the xusb_types.h file are to be used for testing the examples.
 - To run mass storage examples, the constant definition MASS_STORAGE_DEVICE is
to be defined and the constants HID_DEVICES, USB_KEYBOARD and USB_MOUSE are to
be undefined.
 - To run keyboard example, the constants HID_DEVICES and USB_KEYBOARD are to
be defined and the constant definitions MASS_STORAGE_DEVICE and USB_MOUSE are to
be undefined.
 - To run mouse example, the constants HID_DEVICES and USB_MOUSE are to
be defined and the constant definitions MASS_STORAGE_DEVICE and USB_KEYBOARD are
to be undefined.
 - To run the microphone example the constant definition MICROPHONE has to be
defined and the other example constant definitions should be undefined.
*/
