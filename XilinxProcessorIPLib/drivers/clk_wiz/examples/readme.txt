/** \page example Examples
You can refer to the below stated example applications for more details on how to use clk_wiz driver.

@section ex1 xclk_wiz_intr_example.c
Contains an example on how to use the XClk_wiz driver directly.
This example uses the XClk_Wiz driver with interrupts. It will generate interrupt
for clok glitch, clock overflow and underflow. The user should have setup with 2
clocking wizard instances, one instance act as clocking monitor (Enable clock
monitor in GUI), In another instance enable dynamic clock reconfiguration.

For details, see xclk_wiz_intr_example.c.
*/
