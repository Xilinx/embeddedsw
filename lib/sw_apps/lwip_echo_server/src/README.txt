lwIP Echo Server
----------------

The lwIP Echo server application starts an echo server at port 7. Any data sent to
this port is simply echoed back.

By default, the program assigns the following settings to the board:
IP Address: 192.168.1.10
Netmask   : 255.255.255.0
Gateway   : 192.168.1.1
MAC address:  00:0a:35:00:01:02
These settings can be changed in the file main.c.

The main echo server logic is present in the file echo.c.

platform.c implements certain processor and platform dependent functions.
The file platform_config.h is generated based on the hardware design. It makes two
assumptions: The timer has its interrupt line connected to the interrupt controller,
and all the ethernet peripherals (xps_ethernetlite or xps_ll_temac) accessible from 
the processor can be used with lwIP.

Running the Echo Server example
-------------------------------

To connect and test the echo server, download and run the program on the board, 
and then issue the following command from your host machine:

$ telnet 192.168.1.10 7
Trying 192.168.1.10...
Connected to 192.168.1.10.
Escape character is '^]'.
hello world
hello world
all messages will be echo'ed back
all messages will be echo'ed back
^]
telnet> quit
Connection closed. 
$


lwIPv6 Echo Server
----------------

The lwIPv6 Echo server application starts an echo server at port 7. Any data sent to
this port is simply echoed back.

By default, the program assigns the following settings to the board:
IPv6 address: FE80:0:0:0:20A:35FF:FE00:102
MAC address:  00:0a:35:00:01:02
These settings can be changed in the file main.c.

The main echo server logic is present in the file echo.c.

platform.c implements certain processor and platform dependent functions.
The file platform_config.h is generated based on the hardware design. It makes two
assumptions: The timer has its interrupt line connected to the interrupt controller,
and all the ethernet peripherals (xps_ethernetlite or xps_ll_temac) accessible from
the processor can be used with lwIP.

Running the Echo Server example
-------------------------------

To connect and test the echo server, download and run the program on the board,
and then issue the following command from your host machine:

$ telnet -6 FE80:0:0:0:20A:35FF:FE00:102%eth1 7
Trying fe80::20a:35ff:fe00:102%eth1...
Connected to FE80:0:0:0:20A:35FF:FE00:102%eth1.
Escape character is '^]'.
hello world
hello world
all messages will be echo'ed back
all messages will be echo'ed back
^]
telnet> quit
Connection closed.
$
    
References
----------

More details regarding the echo server can be obtained from Xilinx XAPP 1026:
http://www.xilinx.com/support/documentation/application_notes/xapp1026.pdf
