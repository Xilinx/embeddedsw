FreeRTOS lwIP IGMP
------------------

The FreeRTOS lwIP IGMP application joins a default multicast group of
224.10.10.3.
Any data sent to this Multicast IP address and default 5001 port, will be
accepted.
Once IGMP_MULTICAST_RXPKT_COUNT (defined in igmp_app.h) packets are
received, then application will send single multicast packet and leave
multicast group.

Following IGMP Mulitcast options can be changed in file igmp_app.h,
1) IGMP_MULTICAST_IP_ADDRESS
2) IGMP_MULTICAST_PORT
3) IGMP_MULTICAST_RXPKT_COUNT

If LWIP_DHCP enabled then board should get IP address from DHCP server.
If DHCP timeout happens or LWIP_DHCP disabled then, the program assigns the
following IP settings to the board:
IP Address: 192.168.1.10
Netmask   : 255.255.255.0
Gateway   : 192.168.1.1
MAC address:  00:0a:35:00:01:02

These settings can be changed in the file main.c.

The main IGMP logic is present in the file igmp_app.c.

Running the IGMP example
------------------------

To connect and test the IGMP, download and run the program on the board,
and then issue the following command from your host machine:

$ iperf -c 224.10.10.3 -t 2 -u -b 1M -B <Host IP address>
