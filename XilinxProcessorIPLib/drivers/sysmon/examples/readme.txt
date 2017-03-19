/** \page example Examples
You can refer to the below stated example applications for more details on how to use sysmon driver.

@section ex1 xsysmon_extmux_example.c
Contains an example on how to use the XSysmon driver directly.
This example shows the usage of the driver/device in interrupt
mode with external mux and XADC in Simulateneous Sequencer mode.

For details, see xsysmon_extmux_example.c.

@section ex2 xsysmon_intr_example.c
Contains an example on how to use the XSysmon driver directly.
This example shows the usage of the driver/device in interrupt mode to
handle on-chip temperature and voltage alarm interrupts.

For details, see xsysmon_intr_example.c.

@section ex3 xsysmon_intr_printf_example.c
Contains an example on how to use the XSysmon driver directly.
This example shows the usage of the driver/device in interrupt mode to
handle on-chip temperature and voltage alarm interrupts and assumes that
there is a STDIO device in the system. This example has floating point
calculations and uses printfs for outputting floating point data.

For details, see xsysmon_intr_printf_example.c.

@section ex4 xsysmon_low_level_example.c
Contains an example on how to use the XSysmon driver directly.
This example contains the basic driver functions of the sysmon driver to check
the on-chip temperature and voltages.

For details, see xsysmon_low_level_example.c.

@section ex5 xsysmon_polled_example.c
Contains an example on how to use the XSysmon driver directly.
This example uses the driver/device in polled mode to check
the on-chip temperature and voltages.

For details, see xsysmon_polled_example.c.

@section ex6 xsysmon_polled_printf_example.c
Contains an example on how to use the XSysmon driver directly.
This example uses the driver/device in polled mode to check
the on-chip temperature and voltages and assumes that there is a
STDIO device in the system.

For details, see xsysmon_polled_printf_example.c.

@section ex7 xsysmon_single_ch_intr_example.c
Contains an example on how to use the XSysmon driver directly.
This example shows the usage of the driver/device in single channel interrupt
mode to handle End of Conversion(EOC) and VCCINT alarm interrupts.

For details, see xsysmon_single_ch_intr_example.c.
*/
