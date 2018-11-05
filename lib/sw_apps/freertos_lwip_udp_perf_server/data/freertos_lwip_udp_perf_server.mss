
PARAMETER VERSION = 2.2.0


BEGIN OS
 PARAMETER OS_NAME = freertos10_xilinx
 PARAMETER STDIN =  *
 PARAMETER STDOUT = *
 PARAMETER total_heap_size = 262140
END

BEGIN LIBRARY
 PARAMETER LIBRARY_NAME = lwip202
 PARAMETER API_MODE = SOCKET_API
 PARAMETER dhcp_does_arp_check = true
 PARAMETER lwip_dhcp = true
 PARAMETER mem_size = 524288
 PARAMETER memp_n_pbuf = 1024
 PARAMETER memp_num_netbuf = 4096
 PARAMETER tcpip_mbox_size = 4096
 PARAMETER default_udp_recvmbox_size = 4096
 PARAMETER lwip_tcpip_core_locking_input = true
 PARAMETER n_rx_descriptors = 512
 PARAMETER pbuf_pool_size = 9216
END
