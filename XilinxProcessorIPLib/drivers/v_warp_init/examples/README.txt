Example application design source files (contained within "examples" folder) are tightly coupled with the warp example design available in Vivado Catalog.
libm library should be linked before building the application project in Vitis. Go to "C/C++ Build settings"-> "ARM v8 gcc linker"->"Libraries" and add -lm.

When executed on the board the example application will perform following operations
1. Program warp_init descriptor.
2. Start AXI timer
3. Start warp_init IP
4. After getting interrupt stop AXI timer and measure the time taken to generate the Remap Vector.
5. Program warp_filter descriptor
6. Start AXI timer
7. Start warp_filter IP
8. After getting interrupt stop AXI timer and measure the time taken to generate the warped output.
9. Calculate CRC and compare with the golden output CRC value for that configuration.