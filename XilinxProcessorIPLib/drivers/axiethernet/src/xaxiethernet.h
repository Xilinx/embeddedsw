/******************************************************************************
* Copyright (C) 2010 - 2021 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/**
*
* @file xaxiethernet.h
* @addtogroup axiethernet_v5_13
* @{
* @details
*
* The Xilinx AXI Ethernet MAC driver component. This driver supports hard
* Ethernet core for Virtex-6(TM) devices and soft Ethernet core for
* Spartan-6(TM) and other supported devices. The supported speed can be
* 10/100/1000 Mbps and can reach up to 2000/2500 Mbps (1000Base-X versions).
*
* For a full description of AXI Ethernet features, please see the hardware
* spec.
* This driver supports the following features:
*   - Memory mapped access to host interface registers
*   - Virtual memory support
*   - Unicast, broadcast, and multicast receive address filtering
*   - Full duplex operation
*   - Automatic source address insertion or overwrite (programmable)
*   - Automatic PAD & FCS insertion and stripping (programmable)
*   - Flow control
*   - VLAN frame support
*   - Pause frame support
*   - Jumbo frame support
*   - Partial and full checksum offload
*   - Extended multicast addresses to 2**23.
*   - Extended VLAN translation, tagging and stripping supports.
*
* <h2>Driver Description</h2>
*
* The device driver enables higher layer software (e.g., an application) to
* configure a Axi Ethernet device. It is intended that this driver be used in
* cooperation with another driver (FIFO or DMA) for data communication. This
* device driver can support multiple devices even when those devices have
* significantly different configurations.
*
* <h2>Initialization & Configuration</h2>
*
* The XAxiEthernet_Config structure can be used by the driver to configure
* itself. This configuration structure is typically created by the tool-chain
* based on hardware build properties, although, other methods are allowed and
* currently used in some systems.
*
* To support multiple runtime loading and initialization strategies employed
* by various operating systems, the driver instance can be initialized using
* the XAxiEthernet_CfgInitialze() routine.
*
* <h2>Interrupts and Asynchronous Callbacks</h2>
*
* The driver has no dependencies on the interrupt controller. It provides
* no interrupt handlers. The application/OS software should set up its own
* interrupt handlers if required.
*
* <h2>Device Reset</h2>
*
* When a Axi Ethernet device is connected up to a FIFO or DMA core in hardware,
* errors may be reported on one of those cores (FIFO or DMA) such that it can
* be determined that the Axi Ethernet device needs to be reset. If a reset is
* performed, the calling code should also reconfigure and reapply the proper
* settings in the Axi Ethernet device.
*
* When a Axi Ethernet device reset is required, XAxiEthernet_Reset() should
* be utilized.
*
* <h2>Virtual Memory</h2>
*
* This driver may be used in systems with virtual memory support by passing
* the appropriate value for the <i>EffectiveAddress</i> parameter to the
* XAxiEthernet_CfgInitialize() routine.
*
* <h2>Transferring Data</h2>
*
* The Axi Ethernet core by itself is not capable of transmitting or receiving
* data in any meaningful way. Instead the Axi Ethernet device need to be
* connected to a FIFO or DMA core in hardware.
*
* This Axi Ethernet driver is modeled in a similar fashion where the
* application code or O/S adapter driver needs to make use of a separate FIFO
* or DMA driver in connection with this driver to establish meaningful
* communication over Ethernet.
*
* <h2>Checksum Offloading</h2>
*
* If configured, the device can compute a 16-bit checksum from frame data. In
* most circumstances this can lead to a substantial gain in throughput.
*
* The checksum offload settings for each frame sent or received are
* transmitted through the AXI4-Stream interface in hardware. What this means
* is that the checksum offload feature is indirectly controlled in the
* Axi Ethernet device through the driver for DMA core connected
* to the Axi Ethernet device.
*
* Refer to the documentation for DMA driver used for data
* communication on how to set the values for the relevant AXI4-Stream control
* words.
*
* Since this hardware implementation is general purpose in nature system
* software must perform pre and post frame processing to obtain the desired
* results for the types of packets being transferred. Most of the time this
* will be TCP/IP traffic.
*
* TCP/IP and UDP/IP frames contain separate checksums for the IP header and
* UDP/TCP header+data.
* For partial checksum offloading (enabled while configuring the hardware),
* the IP header checksum cannot be offloaded. Many stacks that support
* offloading will compute the IP header if required and use hardware to compute
* the UDP/TCP header+data checksum. There are other complications concerning
* the IP pseudo header that must be taken into consideration. Readers should
* consult a TCP/IP design reference for more details.
*
* For full checksum offloading (enabled while configuring the hardware), the
* IPv4 checksum calculation and validation can also be offloaded at the
* harwdare. Full checksum offload is supported only under certain conditions.
* IP checksum offload will be supported on valid IP datagrams that meet the
* following conditions.
*   - If present, the VLAN header is 4 bytes long
*   - Encapsulation into the Ethernet frame is either Ethernet II or Ethernet
*     SNAP format
*   - Only IPv4 is supported. IPv6 is not supported.
*   - IP header is a valid length
* TCP/UDP checksum offloading will be supported on valid TCP/UDP segments that
* meet the following conditions.
*   - Encapsulated in IPv4 (IPv6 is not supported)
*   - Good IP header checksum
*   - No fragmentation
*   - TCP or UDP segment
* When full checksum offload is enabled, the hardware does the following:
*   - Calculates the IP header checksum and inserts it in the IP header.
*   - Calculates the TCP/UDP Pseudo header from IP header.
*   - Calculates TCP/UDP header from, TCP/UDP psedu header, TCP/UDP header
*     and TCP/UDP payload.
*   - On the receive path, it again calculates all the above and validates
*     for IP header checksum and TCP/UDP checksum.
*
* There are certain device options that will affect the checksum calculation
* performed by hardware for Tx:
*
*   - FCS insertion disabled (XAE_FCS_INSERT_OPTION): software is required to
*     calculate and insert the FCS value at the end of the frame, but the
*     checksum must be known ahead of time prior to calculating the FCS.
*     Therefore checksum offloading cannot be used in this situation.
*
* And for Rx:
*
*   - FCS/PAD stripping disabled (XAE_FCS_STRIP_OPTION): The 4 byte FCS at the
*     end of frame will be included in the hardware calculated checksum.
*     software must subtract out this data.
*
*   - FCS/PAD stripping disabled (XAE_FCS_STRIP_OPTION): For frames smaller
*     than 64 bytes, padding will be included in the hardware calculated
*     checksum.
*     software must subtract out this data. It may be better to allow the
*     TCP/IP stack verify checksums for this type of packet.
*
*   - VLAN enabled (XAE_VLAN_OPTION): The 4 extra bytes in the Ethernet header
*     affect the hardware calculated checksum. software must subtract out the
*     1st two 16-bit words starting at the 15th byte.
*
* <h3>Transmit Checksum Offloading</h3>
*
* For partial checksum offloading, for the TX path, the software can specify
* where in the frame the checksum calculation is to start, where the result
* should be inserted, and a seed value. The checksum is calculated from
* the start point through the end of frame.
*
* For full checksum offloading, for the TX path, the software just need to
* enable Full Checksum offload in the appropriate AXI4-Stream Control word on
* a per packet basis.
*
* The checksum offloading settings are sent in the transmit AXI4 Stream control
* words. The relevant control word fields are described in brief below.
* Refer to the Axi Ethernet hardware specification for more details.
*
*<h4>AXI4-Stream Control Word 0:</h4>
*<pre>
*	Bits  1-0  : Transmit Checksum Enable: 	01 - Partial checsum offload,
*						10 - Full checksum offload
*						00 - No checksum offloading
*						11 - Not used, reserved
*	Bits 27-2  : Reserved
*	Bits 31-28 : Used for AXI4-Stream Control Mode flag
*</pre>
*
*<h4>AXI4-Stream Control Word 1:</h4>
*<pre>
*	Bits 31-16 (MSB): Transmit Checksum Calculation Starting Point: Offset
*			  in the frame where checksum calculation should begin.
*			  Relevant only for partial checksum offloading.
*	Bits 15-0  (LSB): Transmit Checksum Insertion Point: Frame offset where
*			  the computed checksum value is stored, which should be
*			  in the TCP or UDP header.
*			  Relevant only for partial checksum offloading.
*   </pre>
*
*<h4>AXI4-Stream Control Word 2:</h4>
*<pre>
*	Bits 31-16 (MSB): Reserved
*	Bits  0-15 (LSB): Transmit Checksum Calculation Initial Value: Checksum
*			  seed value.
*			  Relevant only for partial checksum offloading.
*</pre>
*
* <h3>Receive Checksum Offloading</h3>
*
* For partial checksum offload on the RX path, the 15th byte to end of frame
* is check summed. This range of bytes is the entire Ethernet payload (for
* non-VLAN frames).
*
* For full checksum offload on the RX path, both the IP and TCP checksums are
* validated if the packet meets the specified conditions.
*
* The checksum offloading information is sent in the receive AXI4-Stream
* status words. There are 4 relevant status words available. However
* only the relevant status words are described in brief below.
* Refer to the Axi Ethernet hardware specification for more details.
*
*<h4>AXI4-Stream Status Word 0:</h4>
*<pre>
*	Bits 31-28 (MSB): Always 0x5 to represent receive status frame
*	Bits 27-16	: Undefined
*	Bits 15-0  (LSB): MCAST_ADR_U. Upper 16 bits of the multicast
*			  destination address of the frame.
*
*<h4>AXI4-Stream Status Word 1:</h4>
*</pre>
*	Bits 31-0	: MCAST_ADR_L. The lower 32 bits of the multicast
*			  destination address.
*
*<h4>AXI4-Stream Status Word 2:</h4>
*</pre>
*	Bits 5-3 	: Specifies the receive full checksum status. This
*			  is relevant only for full checksum offloading.
*			  000 -> Neither the IP header nor the TCP/UDP
*				 checksums were checked.
*			  001 -> The IP header checksum was checked and was
*				 correct. The TCP/UDP checksum was not checked.
*			  010 -> Both the IP header checksum and the TCP
*				 checksum were checked and were correct.
*			  011 -> Both the IP header checksum and the UDP
*				 checksum were checked and were correct.
*			  100 -> Reserved.
*			  101 -> The IP header checksum was checked and was
*				 incorrect. The TCP/UDP checksum was not
*				 checked.
*			  110 -> The IP header checksum was checked and is
*				 correct but the TCP checksum was checked
*				 and was incorrect.
*			  111 -> The IP header checksum was checked and is
*				 correct but the UDP checksum was checked and
*				 was incorrect.
*
*<h4>AXI4-Stream Status Word 3:</h4>
*	Bits 31-16	: T_L_TPID. This is the value of 13th and 14th byte
*			  of the frame.
*	Bits 15-0 	: Receive Raw Checksum: Computed checksum value
*
*<h4>AXI4-Stream Status Word 3:</h4>
*	Bits 31-16	: VLAN_TAG. Value of 15th and 16th byte of the
*			 frame.
*	Bits 15-0 	: RX_BYTECNT. Received frame length.
*
*
* <h2>Extended multicast</h2>
* (XAE_EXT_MULTICAST_OPTION): Allow and perform address filtering more than 4
* multicast addresses. Hardware requires to enable promiscuous mode
* (XAE_PROMISCUOUS_OPTION) and disable legacy multicast mode
* (XAE_MULTICAST_OPTION) for this feature to work.
*
* <h2>Extended VLAN</h2>
*
* <h3>TX/RX VLAN stripping</h3>
* (XAE_EXT_[T|R]XVLAN_STRP_OPTION) handles transmit/receive one VLAN tag
* stripping in Ethernet frames. To enable this option, hardware requires to
* build with this feature and enable (XAE_FCS_INSERT_OPTION),
* (XAE_FCS_STRP_OPTION) and disable (XAE_VLAN_OPTION). Supports three modes,
* -XAE_VSTRP_NONE   : no stripping
* -XAE_VSTRP_ALL    : strip one tag from all frames
* -XAE_VSTRP_SELECT : strip one tag from selected frames
*
* <h3>TX/RX VLAN translation</h3>
* (XATE_EXT_[T|R]XVLAN_TRAN_OPTION) handles transmit/receive one VLAN tag
* translation in Ethernet frames. To enable this option, hardware requires to
* build with this feature and enable (XATE_FCS_INSERT_OPTION),
* (XAE_FCS_STRP_OPTION), and disable (XAE_VLAN_OPTION).
*
* <h3>TX/RX VLAN tagging</h3>
* (XAE_EXT_[T|R]XVLAN_TAG_OPTION) adds transmit/receive one VLAN tag in
* Ethernet frames. To enable this option, hardware requires to build with this
* feature and enable (XAE_FCS_INSERT_OPTION), (XAE_FCS_STRP_OPTION),
* (XAE_JUMBO_OPTION) and disable (XAE_VLAN_OPTION). Support four modes,
* -XAE_VTAG_NONE    : no tagging
* -XAE_VTAG_ALL     : tag all frames
* -XAE_VTAG_EXISTED : tag already tagged frames
* -XAE_VTAG_SELECT  : tag selected already tagged frames
*
* <h2>PHY Communication</h2>
*
* Prior to PHY access, the MDIO clock must be setup. This driver will set a
* safe default that should work with AXI4-Lite bus speeds of up to 150 MHz
* and keep the MDIO clock below 2.5 MHz. If the user wishes faster access to
* the PHY then the clock divisor can be set to a different value (see
* XAxiEthernet_PhySetMdioDivisor()).
*
* MII register access is performed through the functions XAxiEthernet_PhyRead()
* and XAxiEthernet_PhyWrite().
*
* <h2>Link Sync</h2>
*
* When the device is used in a multi speed environment, the link speed must be
* explicitly set using XAxiEthernet_SetOperatingSpeed() and must match the
* speed PHY has negotiated. If the speeds are mismatched, then the MAC will not
* pass traffic.
*
* The application/OS software may use the AutoNegotiation interrupt to be
* notified when the PHY has completed auto-negotiation.
*
* <h2>Asserts</h2>
*
* Asserts are used within all Xilinx drivers to enforce constraints on argument
* values. Asserts can be turned off on a system-wide basis by defining, at
* compile time, the NDEBUG identifier. By default, asserts are turned on and it
* is recommended that users leave asserts on during development. For deployment
* use -DNDEBUG compiler switch to remove assert code.
*
* <h2>Driver Errata</h2>
*
*   - A dropped receive frame indication may be reported by the driver after
*     calling XAxiEthernet_Stop() followed by XAxiEthernet_Start(). This can
*     occur if a frame is arriving when stop is called.
*   - On Rx with checksum offloading enabled and FCS/PAD stripping disabled,
*     FCS and PAD data will be included in the checksum result.
*   - On Tx with checksum offloading enabled and auto FCS insertion disabled,
*     the user calculated FCS will be included in the checksum result.
*
* @note
*
* Xilinx drivers are typically composed of two components, one is the driver
* and the other is the adapter.  The driver is independent of OS and processor
* and is intended to be highly portable.  The adapter is OS-specific and
* facilitates communication between the driver and an OS.
* <br><br>
* This driver is intended to be RTOS and processor independent. Any needs for
* dynamic memory management, threads or thread mutual exclusion, or cache
* control must be satisfied by the layer above this driver.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- ---------------------------------------------------------
* 1.00a asa  6/30/10  First release based on the ll temac driver
* 1.01a asa  12/10/10 Added macros XAxiEthernet_IsRxFullCsum and
*		      XAxiEthernet_IsTxFullCsum for supporting full checksum
*		      offload. The full checksum offload is only supported in
*		      newer versions of the core, please refer to the core
*		      HW datasheet.
* 1.02a asa  2/16/11  Inserted a delay in the driver function
*		      XAxiEthernet_Reset in file xaxiethernet.c. This is done
*		      because immediately after a core reset none of the
*		      AxiEthernet registers are accessible for some duration.
*		      Changed the value of XAE_LOOPS_TO_COME_OUT_OF_RST to
*		      10000 in file xaxiethernet_hw.h.
*
* 2.00a asa  8/29/11  A new major version of AxiEthernet driver is being
*		      released to accommodate the change in avb software. The
*		      AxiEthernet hardware v3_00_a has the latest avb
*		      hardware which needs a corresponding change in avb
*		      software (released in examples/avb folder). This change
*		      in avb software is not backwards compatible (which
*		      means this avb software will not work with older
*		      axiethernet hardware).
*		      Hence a new major version of axiethernet is being
*		      released.
*		      Added defines for Ability Reg, Identification Reg, Rx max
*		      Frame and Tx Max Frame registers.
*		      Changed define for TEMAC RGMII/SGMII Config (PHYC) Reg.
*
* 3.00a asa  4/10/12  A new major version of AxiEthernet is being released to
*		      accommodate the change in AVB example. From AxiEthernet
*		      core version 3.01a onwards the AVB implementation has
*		      changed. The AVB module is now a part of AxiEthernet IP.
*		      Because of this change, the AVB example works only
*		      when promiscuous mode is not enabled (unlike earlier
*		      implementation where promiscuous mode was required for
*		      AVB example to work). Hence the file xavb_example.c is
*		      changed so that the core is not put in promiscuous mode.
*		      Also since AVB is a part of AxiEthernet some of the
*		      register mappings in xavb_hw.h has changed.
*		      These changes are not backward compatible which means
*		      this changed example will not work for previous versions
*		      of cores.
*		      Hence a new major version of axiethernet is being
*		      released.
* 3.01a srt  02/03/13 - Added support for SGMII mode (CR 676793)
*	     	      - Added support for IPI designs (CR 698249)
*	     02/14/13 - Added support for Zynq (CR 681136)
* 3.02a srt  04/13/13 - Removed Warnings (CR 704998).
*            04/24/13 - Modified parameter *_SGMII_PHYADDR to *_PHYADDR, the
*                       config parameter C_PHYADDR applies to SGMII/1000BaseX
*	                modes of operation
*	     04/24/13 - Added support for 1000BaseX mode in examples (_util.c)
*		        (CR 704195)
*	     04/24/13 - Added support for RGMII mode in examples (_util.c)
* 3.02a srt  08/06/13 - Fixed CR 727634:
*			  Modified FifoHandler() function logic of FIFO
*			  interrupt example to reflect the bit changes in
*			  the Interrupt Status Register as per the latest
*			  AXI FIFO stream IP.
*		      - Fixed CR 721141:
*			  Added support to handle multiple instances of
*			  AxiEthernet FIFO interface (CR 721141)
*		      - Fixed CR 717949:
*			  Configures external Marvel 88E1111 PHY based on the
*			  AXI Ethernet physical interface type and allows to
*			  operate in specific interface mode without changing
*			  jumpers on the Microblaze boards. This change is in
*			  example_util.c
* 3.02a adk 15/11/13 - Fixed CR 761035 removed dependency with fifo in MDD file
* 4.0   adk 19/12/13 - Updated as per the New Tcl API's
*		asa 30/01/14 - Added defines for 1588 registers and bit masks
*					   Added config parameter for SGMII over LVDS
*
* 4.1  adk 21/04/14 - Fixed CR:780537 Changes are Made in the file
* 		      axiethernet test-app tcl
* 4.2  adk 08/08/14 - Fixed CR:810643 SDK generated 'xparamters.h' erroneously
*		      generated with errors due to part of '#define' misplaced
*		      changes are made in the driver tcl file.
* 4.3 adk 29/10/14 -  Added support for generating parameters for SGMII/1000BaseX
*		      modes When IP is configured with the PCS/PMA core.
*		      Changes are made in the driver tcl file (CR 828796).
* 4.4  adk  8/1/15 -  Fixed TCL errors when axiethernet is configured with the
*		      Axi stream fifo (CR 835605). Changes are made in the
*		      driver tcl file.
* 5.0  adk 13/06/15 - Updated the driver tcl for Hier IP(To support User parameters).
* 5.0  adk 28/07/15 - Fixed CR:870631 AXI Ethernet with FIFO will fail to
*		      Create the BSP if the interrupt pin on the FIFO is unconnected
* 5.1  sk  11/10/15 Used UINTPTR instead of u32 for Baseaddress CR# 867425.
*                   Changed the prototype of XAxiEthernet_CfgInitialize API.
* 5.1  adk 30/01/15  Fix compilation errors in case of zynqmp. CR#933825.
* 5.1  adk 02/11/16  Updated example to Support ZynqMp.
* 5.2  adk 13/05/16  Fixed CR#951669 Fix compilation errors when axi dma
* 		     interrupts are not connected.
* 5.3  adk 05/10/16  Fixed CR#961152 PMU template firmware fails to compile on
* 		     ZynqMP AXI-Ethernet designs.
* 5.4  adk 07/12/16  Added Support for TI PHY DP83867 changes are made in the
* 		     examples xaxiethernet_example_util.c file.
*       ms 01/23/17 Modified xil_printf statement in main function for all
*            examples to ensure that "Successfully ran" and "Failed" strings
*            are available in all examples. This is a fix for CR-965028.
*	adk 03/09/17 Fixed CR#971367 fix race condition in the tcl
*		     for a multi mac design(AXI_CONNECTED_TYPE defined for
*		     only one instance)
*       ms 03/17/17 Modified text file in examples folder for doxygen
*                   generation.
*       ms 04/05/17 Added tabspace for return statements in functions
*                   of axiethernet examples for proper documentation while
*                   generating doxygen.
* 5.5	adk 19/05/17 Increase Timeout value in the driver as per new h/w update
* 		     i.e. Increase of transceiver initialization times in
* 		     ultrascale+ devices (CR#976244).
* 5.6  adk 03/07/17 Fixed issue lwip stops working as soon as something is
*		    plugged to it`s AXI stream bus (CR#979634). Changes
*		    are made in the driver tcl and test app tcl.
* 5.6  adk 03/07/17 Fixed CR#979023 Intr fifo example failed to compile.
*      ms  04/18/17 Modified tcl file to add suffix U for all macro
*                   definitions of axiethernet in xparameters.h
*      adk 08/08/17 Fixed CR#981893 Fix bsp compilation error for axiethernet
*		    mcdma chiscope based designs.
*      ms  08/16/17 Fixed compilation warnings in xaxiethernet_sinit.c
*      adk 08/22/17 Fixed CR#983008 app generation errors for Specific IPI design.
*      adk 08/28/17 Fixed CR#982877 couple of dsv_ced tests are failing in peripheral
*		    app generation.
*      adk 09/21/17 Fixed CR#985686 bsp generation error with specific design.
*		    Changes are made in the driver tcl.
* 5.7  rsp 01/09/18 Add NumTableEntries member in XAxiEthernet_Config.
*                   Instead of #define XAE_MULTI_MAT_ENTRIES derive multicast table
*                   entries max count from ethernet config structure.
*          01/11/18 Fixed CR#976392 Use UINTPTR for DMA base address.
* </pre>
*
******************************************************************************/

#ifndef XAXIETHERNET_H		/* prevent circular inclusions */
#define XAXIETHERNET_H		/* by using protection macros */

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/

#include "xenv.h"
#include "xstatus.h"
#include "xil_assert.h"
#include "xaxiethernet_hw.h"

/************************** Constant Definitions *****************************/

/** @name Configuration options
 *
 * The following are device configuration options. See the
 * <i>XAxiEthernet_SetOptions</i>, <i>XAxiEthernet_ClearOptions</i> and
 * <i>XAxiEthernet_GetOptions</i> routines for information on how to use
 * options.
 *
 * The default state of the options are also noted below.
 *
 * @{
 */

/**< XAE_PROMISC_OPTION specifies the Axi Ethernet device to accept all
 *   incoming packets.
 *   This driver sets this option to disabled (cleared) by default.
 */
#define XAE_PROMISC_OPTION		0x00000001

/**< XAE_JUMBO_OPTION specifies the Axi Ethernet device to accept jumbo
 *   frames for transmit and receive.
 *   This driver sets this option to disabled (cleared) by default.
 */
#define XAE_JUMBO_OPTION		0x00000002

/**< XAE_VLAN_OPTION specifies the Axi Ethernet device to enable VLAN support
 *   for transmit and receive.
 *   This driver sets this option to disabled (cleared) by default.
 */
#define XAE_VLAN_OPTION			0x00000004

/**< XAE_FLOW_CONTROL_OPTION specifies the Axi Ethernet device to recognize
 *   received flow control frames.
 *   This driver sets this option to enabled (set) by default.
 */
#define XAE_FLOW_CONTROL_OPTION		0x00000008

/**< XAE_FCS_STRIP_OPTION specifies the Axi Ethernet device to strip FCS and
 *   PAD from received frames. Note that PAD from VLAN frames is not stripped.
 *   This driver sets this option to enabled (set) by default.
 */
#define XAE_FCS_STRIP_OPTION		0x00000010

/**< XAE_FCS_INSERT_OPTION specifies the Axi Ethernet device to generate the
 *   FCS field and add PAD automatically for outgoing frames.
 *   This driver sets this option to enabled (set) by default.
 */
#define XAE_FCS_INSERT_OPTION		0x00000020

/**< XAE_LENTYPE_ERR_OPTION specifies the Axi Ethernet device to enable
 *   Length/Type error checking (mismatched type/length field) for received
 *   frames.
 *   This driver sets this option to enabled (set) by default.
 */
#define XAE_LENTYPE_ERR_OPTION		0x00000040

/**< XAE_TRANSMITTER_ENABLE_OPTION specifies the Axi Ethernet device
 *   transmitter to be enabled.
 *   This driver sets this option to enabled (set) by default.
 */
#define XAE_TRANSMITTER_ENABLE_OPTION	0x00000080

/**< XAE_RECEIVER_ENABLE_OPTION specifies the Axi Ethernet device receiver to
 *   be enabled.
 *   This driver sets this option to enabled (set) by default.
 */
#define XAE_RECEIVER_ENABLE_OPTION	0x00000100

/**< XAE_BROADCAST_OPTION specifies the Axi Ethernet device to receive frames
 *   sent to the broadcast Ethernet address.
 *   This driver sets this option to enabled (set) by default.
 */
#define XAE_BROADCAST_OPTION		0x00000200

/**< XAE_MULTICAST_OPTION specifies the Axi Ethernet device to receive frames
 *   sent to Ethernet addresses that are programmed into the Multicast Address
 *   Table (MAT).
 *   This driver sets this option to disabled (cleared) by default.
 */
#define XAE_MULTICAST_OPTION		0x00000400

/**< XAE_EXT_MULTICAST_OPTION specifies the Axi Ethernet device to receive
 *   frames sent to Ethernet addresses that are programmed into the Multicast
 *   Address Table.
 *   This driver sets this option to be dependent on HW configuration
 *   by default.
 */
#define XAE_EXT_MULTICAST_OPTION	0x00000800

/**< XAE_EXT_TXVLAN_TRAN_OPTION specifies the Axi Ethernet device to enable
 *   transmit VLAN translation.
 *   This driver sets this option to be dependent on HW configuration
 *   by default.
 */
#define XAE_EXT_TXVLAN_TRAN_OPTION	0x00001000

/**< XAE_EXT_RXVLAN_TRAN_OPTION specifies the Axi Ethernet device to enable
 *   receive VLAN translation.
 *   This driver sets this option to be dependent on HW configuration
 *   by default.
 */
#define XAE_EXT_RXVLAN_TRAN_OPTION	0x00002000

/**< XAE_EXT_TXVLAN_TAG_OPTION specifies the Axi Ethernet device to enable
 *   transmit VLAN tagging.
 *   This driver sets this option to be dependent during HW build time
 *   by default.
 */
#define XAE_EXT_TXVLAN_TAG_OPTION	0x00004000

/**< XAE_EXT_RXVLAN_TAG_OPTION specifies the Axi Ethernet device to enable
 *   receive VLAN tagging.
 *   This driver sets this option to be dependent during HW build time
 *   by default.
 */
#define XAE_EXT_RXVLAN_TAG_OPTION	0x00008000

/**< XAE_EXT_TXVLAN_STRP_OPTION specifies the Axi Ethernet device to enable
 *   transmit VLAN stripping.
 *   This driver sets this option to be dependent during HW build time
 *   by default.
 */
#define XAE_EXT_TXVLAN_STRP_OPTION	0x00010000

/**< XAE_EXT_RXVLAN_STRP_OPTION specifies the Axi Ethernet device to enable
 *   receive VLAN stripping.
 *   This driver sets this option to be dependent during HW build time
 *   by default.
 */
#define XAE_EXT_RXVLAN_STRP_OPTION	0x00020000


#define XAE_DEFAULT_OPTIONS				\
		(XAE_FLOW_CONTROL_OPTION |		\
		 XAE_BROADCAST_OPTION |			\
		 XAE_FCS_INSERT_OPTION |		\
		 XAE_FCS_STRIP_OPTION |			\
		 XAE_LENTYPE_ERR_OPTION |		\
		 XAE_TRANSMITTER_ENABLE_OPTION | 	\
		 XAE_RECEIVER_ENABLE_OPTION)
/**< XAE_DEFAULT_OPTIONS specify the options set in XAxiEthernet_Reset() and
 *   XAxiEthernet_CfgInitialize()
 */
/*@}*/

#define XAE_MDIO_DIV_DFT	29	/**< Default MDIO clock divisor */

/*
 * The next few constants help upper layers determine the size of memory
 * pools used for Ethernet buffers and descriptor lists.
 */
#define XAE_MAC_ADDR_SIZE		6	/* MAC addresses are 6 bytes */
#define XAE_MTU				1500	/* Max MTU size of an Ethernet
						 * frame
						 */
#define XAE_JUMBO_MTU			8982	/* Max MTU size of a jumbo
						 * Ethernet frame
						 */
#define XAE_HDR_SIZE			14	/* Size of an Ethernet header*/
#define XAE_HDR_VLAN_SIZE		18	/* Size of an Ethernet header
						 * with VLAN
						 */
#define XAE_TRL_SIZE			4	/* Size of an Ethernet trailer
						 * (FCS)
						 */
#define XAE_MAX_FRAME_SIZE	 (XAE_MTU + XAE_HDR_SIZE + XAE_TRL_SIZE)
#define XAE_MAX_VLAN_FRAME_SIZE  (XAE_MTU + XAE_HDR_VLAN_SIZE + XAE_TRL_SIZE)
#define XAE_MAX_JUMBO_FRAME_SIZE (XAE_JUMBO_MTU + XAE_HDR_SIZE + XAE_TRL_SIZE)

/*
 * Constant values returned by XAxiEthernet_GetPhysicalInterface(). Note that
 * these values match design parameters from the Axi Ethernet spec.
 */
#define XAE_PHY_TYPE_MII		0
#define XAE_PHY_TYPE_GMII		1
#define XAE_PHY_TYPE_RGMII_1_3		2
#define XAE_PHY_TYPE_RGMII_2_0		3
#define XAE_PHY_TYPE_SGMII		4
#define XAE_PHY_TYPE_1000BASE_X		5

#define XAE_TPID_MAX_ENTRIES		4 /* Number of storable TPIDs in
					   * Table
					   */

/*
 * Constant values pass into XAxiEthernet_SetV[tag|Strp]Mode() and returned by
 * XAxiEthernet_GetV[tag|Strp]Mode().
 */
#define XAE_VTAG_NONE			0	/* No tagging */
#define XAE_VTAG_ALL			1	/* Tag all frames */
#define XAE_VTAG_EXISTED		2	/* Tag already tagged frames */
#define XAE_VTAG_SELECT		  	3	/*
						 * Tag selected already tagged
						 * frames
						 */
#define XAE_DEFAULT_TXVTAG_MODE  	XAE_VTAG_ALL
#define XAE_DEFAULT_RXVTAG_MODE  	XAE_VTAG_ALL

#define XAE_VSTRP_NONE			0	/* No stripping */
#define XAE_VSTRP_ALL			1	/* Strip one tag from all
						 * frames
						 */
#define XAE_VSTRP_SELECT		3	/* Strip one tag from selected
						 * frames
						 */
#define XAE_DEFAULT_TXVSTRP_MODE	XAE_VSTRP_ALL
#define XAE_DEFAULT_RXVSTRP_MODE	XAE_VSTRP_ALL

#define XAE_RX				1 /* Receive direction  */
#define XAE_TX				2 /* Transmit direction */

#define XAE_SOFT_TEMAC_10_100_MBPS	0
#define XAE_SOFT_TEMAC_10_100_1000_MBPS	1
#define XAE_HARD_TEMC			2

/**************************** Type Definitions *******************************/


/**
 * This typedef contains configuration information for a Axi Ethernet device.
 */
typedef struct XAxiEthernet_Config {
	u16 DeviceId;	/**< DeviceId is the unique ID  of the device */
	UINTPTR BaseAddress;/**< BaseAddress is the physical base address of the
			  *  device's registers
			  */
	u8 TemacType;   /**< Temac Type can have 3 possible values. They are
			  *  0 for SoftTemac at 10/100 Mbps, 1 for SoftTemac
			  *  at 10/100/1000 Mbps and 2 for Vitex6 Hard Temac
			  */
	u8 TxCsum;	/**< TxCsum indicates that the device has checksum
			  *  offload on the Tx channel or not.
			  */
	u8 RxCsum;	/**< RxCsum indicates that the device has checksum
			  *  offload on the Rx channel or not.
			  */
	u8 PhyType;	/**< PhyType indicates which type of PHY interface is
			  *  used (MII, GMII, RGMII, etc.
			  */
	u8 TxVlanTran;  /**< TX VLAN Translation indication */
	u8 RxVlanTran;  /**< RX VLAN Translation indication */
	u8 TxVlanTag;   /**< TX VLAN tagging indication */
	u8 RxVlanTag;   /**< RX VLAN tagging indication */
	u8 TxVlanStrp;  /**< TX VLAN stripping indication */
	u8 RxVlanStrp;  /**< RX VLAN stripping indication */
	u8 ExtMcast;    /**< Extend multicast indication */
	u8 Stats;	/**< Statistics gathering option */
	u8 Avb;		/**< Avb option */
	u8 EnableSgmiiOverLvds;	/**< Enable LVDS option */
	u8 Enable_1588;	/**< Enable 1588 option */
	u32 Speed;	/**< Tells whether MAC is 1G or 2p5G */
	u8 NumTableEntries;	/**< Number of table entries */

	u8 TemacIntr;	/**< Axi Ethernet interrupt ID */

	int AxiDevType;  /**< AxiDevType is the type of device attached to the
			  *   Axi Ethernet's AXI4-Stream interface.
			  */
	UINTPTR AxiDevBaseAddress; /**< AxiDevBaseAddress is the base address of
				 *  the device attached to the Axi Ethernet's
				 *  AXI4-Stream interface.
				 */
	u8 AxiFifoIntr;	/**< AxiFifoIntr interrupt ID (unused if DMA) */
	u8 AxiDmaRxIntr;/**< Axi DMA RX interrupt ID (unused if FIFO) */
	u8 AxiDmaTxIntr;/**< Axi DMA TX interrupt ID (unused if FIFO) */
	u8 AxiMcDmaChan_Cnt;  /**< Axi MCDMA Channel Count */
	u8 AxiMcDmaRxIntr[16]; /**< Axi MCDMA Rx interrupt ID (unused if AXI DMA or FIFO) */
	u8 AxiMcDmaTxIntr[16]; /**< AXI MCDMA TX interrupt ID (unused if AXIX DMA or FIFO) */
} XAxiEthernet_Config;


/**
 * struct XAxiEthernet is the type for Axi Ethernet driver instance data.
 * The calling code is required to use a unique instance of this structure
 * for every Axi Ethernet device used in the system. A reference to a structure
 * of this type is then passed to the driver API functions.
 */
typedef struct XAxiEthernet {
	XAxiEthernet_Config Config; /**< Hardware configuration */
	u32 IsStarted;		 /**< Device is currently started */
	u32 IsReady;		 /**< Device is initialized and ready */
	u32 Options;		 /**< Current options word */
	u32 Flags;		 /**< Internal driver flags */
} XAxiEthernet;


/***************** Macros (Inline Functions) Definitions *********************/

/*****************************************************************************/
/**
*
* XAxiEthernet_IsStarted reports if the device is in the started or stopped
* state. To be in the started state, the calling code must have made a
* successful call to <i>XAxiEthernet_Start</i>. To be in the stopped state,
* <i>XAxiEthernet_Stop</i> or <i>XAxiEthernet_CfgInitialize</i> function must
* have been called.
*
* @param	InstancePtr is a pointer to the of Axi Ethernet instance to be
*		worked on.
*
* @return
*		- TRUE if the device has been started.
*		- FALSE.if the device has not been started
*
* @note 	C-style signature:
* 		u32 XAxiEthernet_IsStarted(XAxiEthernet *InstancePtr)
*
 ******************************************************************************/
#define XAxiEthernet_IsStarted(InstancePtr) \
	(((InstancePtr)->IsStarted == XIL_COMPONENT_IS_STARTED) ? TRUE : FALSE)

/*****************************************************************************/
/**
*
* XAxiEthernet_IsDma reports if the device is currently connected to DMA.
*
* @param	InstancePtr is a pointer to the Axi Ethernet instance to be
*		worked on.
* @return
*		- TRUE if the Axi Ethernet device is connected DMA.
*		- FALSE.if the Axi Ethernet device is NOT connected to DMA
*
* @note 	C-style signature:
* 		u32 XAxiEthernet_IsDma(XAxiEthernet *InstancePtr)
*
******************************************************************************/
#define XAxiEthernet_IsDma(InstancePtr) \
	(((InstancePtr)->Config.AxiDevType == XPAR_AXI_DMA) ? TRUE: FALSE)

/*****************************************************************************/
/**
*
* XAxiEthernet_IsFifo reports if the device is currently connected to a fifo
* core.
*
* @param	InstancePtr is a pointer to the Axi Ethernet instance to be
*		worked on.
*
* @return
*		- TRUE if the Axi Ethernet device is connected to a fifo
*		- FALSE if the Axi Ethernet device is NOT connected to a fifo.
*
* @note 	C-style signature:
* 		u32 XAxiEthernet_IsFifo(XAxiEthernet *InstancePtr)
*
******************************************************************************/
#define XAxiEthernet_IsFifo(InstancePtr) \
	(((InstancePtr)->Config.AxiDevType == XPAR_AXI_FIFO) ? TRUE: FALSE)

/*****************************************************************************/
/**
*
* XAxiEthernet_IsMcDma reports if the device is currently connected to MCDMA.
*
* @param	InstancePtr is a pointer to the Axi Ethernet instance to be
*		worked on.
* @return
*		- TRUE if the Axi Ethernet device is connected MCDMA.
*		- FALSE.if the Axi Ethernet device is NOT connected to MCDMA
*
* @note 	C-style signature:
* 		u32 XAxiEthernet_IsMcDma(XAxiEthernet *InstancePtr)
*
******************************************************************************/
#define XAxiEthernet_IsMcDma(InstancePtr) \
	(((InstancePtr)->Config.AxiDevType == XPAR_AXI_MCDMA) ? TRUE: FALSE)

/*****************************************************************************/
/**
*
* XAxiEthernet_AxiDevBaseAddress reports the base address of the core connected
* to the Axi Ethernet's Axi4 Stream interface.
*
* @param	InstancePtr is a pointer to the Axi Ethernet instance to be
*		worked on.
*
* @return	The base address of the core connected to the Axi Ethernet's
*		streaming interface.
*
* @note 	C-style signature:
* 		u32 XAxiEthernet_AxiDevBaseAddress(XAxiEthernet *InstancePtr)
*
******************************************************************************/
#define XAxiEthernet_AxiDevBaseAddress(InstancePtr) \
	((InstancePtr)->Config.AxiDevBaseAddress)

/*****************************************************************************/
/**
*
* XAxiEthernet_IsRecvFrameDropped determines if the device thinks it has
* dropped a receive frame. The device interrupt status register is read to
* determine this.
*
* @param	InstancePtr is a pointer to the Axi Ethernet instance to be
*		worked on.
*
* @return
*		- TRUE if a frame has been dropped
*		- FALSE if a frame has NOT been dropped.
*
* @note 	C-style signature:
* 		u32 XAxiEthernet_IsRecvFrameDropped(XAxiEthernet *InstancePtr)
*
******************************************************************************/
#define XAxiEthernet_IsRecvFrameDropped(InstancePtr)			 \
	((XAxiEthernet_ReadReg((InstancePtr)->Config.BaseAddress,  \
	XAE_IS_OFFSET) & XAE_INT_RXRJECT_MASK) ? TRUE : FALSE)

/*****************************************************************************/
/**
*
* XAxiEthernet_IsRxPartialCsum determines if the device is configured with
* partial checksum offloading on the receive channel.
*
* @param	InstancePtr is a pointer to the Axi Ethernet instance to be
*		worked on.
* @return
*		- TRUE if the device is configured with partial checksum
*		  offloading on the receive channel.
*		- FALSE.if the device is not configured with partial checksum
*		  offloading on the receive side.
*
* @note 	C-style signature:
* 		u32 XAxiEthernet_IsRxPartialCsum(XAxiEthernet *InstancePtr)
*
******************************************************************************/
#define XAxiEthernet_IsRxPartialCsum(InstancePtr)   \
	((((InstancePtr)->Config.RxCsum) == 0x01) ? TRUE : FALSE)

/*****************************************************************************/
/**
*
* XAxiEthernet_IsTxPartialCsum determines if the device is configured with
* partial checksum offloading on the transmit channel.
*
* @param	InstancePtr is a pointer to the Axi Ethernet instance to be
*		worked on.
*
* @return
*		- TRUE if the device is configured with partial checksum
*		  offloading on the transmit side.
*		- FALSE.if the device is not configured with partial checksum
*		  offloading on the transmit side.
*
* @note 	C-style signature:
*		u32 XAxiEthernet_IsTxPartialCsum(XAxiEthernet *InstancePtr)
*
******************************************************************************/
#define XAxiEthernet_IsTxPartialCsum(InstancePtr) \
	((((InstancePtr)->Config.TxCsum) == 0x01) ? TRUE : FALSE)

/*****************************************************************************/
/**
*
* XAxiEthernet_IsRxFullCsum determines if the device is configured with full
* checksum offloading on the receive channel.
*
* @param	InstancePtr is a pointer to the Axi Ethernet instance to be
*		worked on.
* @return
*		- TRUE if the device is configured with full checksum
*		  offloading on the receive channel.
*		- FALSE.if the device is not configured with full checksum
*		  offloading on the receive side.
*
* @note 	C-style signature:
* 		u32 XAxiEthernet_IsRxFullCsum(XAxiEthernet *InstancePtr)
*
******************************************************************************/
#define XAxiEthernet_IsRxFullCsum(InstancePtr)   \
	((((InstancePtr)->Config.RxCsum) == 0x02) ? TRUE : FALSE)

/*****************************************************************************/
/**
*
* XAxiEthernet_IsTxFullCsum determines if the device is configured with full
* checksum offloading on the transmit channel.
*
* @param	InstancePtr is a pointer to the Axi Ethernet instance to be
*		worked on.
*
* @return
*		- TRUE if the device is configured with full checksum
*		  offloading on the transmit side.
*		- FALSE.if the device is not configured with full checksum
*		  offloading on the transmit side.
*
* @note 	C-style signature:
*		u32 XAxiEthernet_IsTxFullCsum(XAxiEthernet *InstancePtr)
*
******************************************************************************/
#define XAxiEthernet_IsTxFullCsum(InstancePtr) \
	((((InstancePtr)->Config.TxCsum) == 0x02) ? TRUE : FALSE)

/*****************************************************************************/
/**
*
* XAxiEthernet_GetPhysicalInterface returns the type of PHY interface being
* used by the given instance, specified by <i>InstancePtr</i>.
*
* @param	InstancePtr is a pointer to the Axi Ethernet instance to be
*		worked on.
*
* @return	The Physical Interface type which is one of XAE_PHY_TYPE_x
*		where x is MII, GMII, RGMII_1_3, RGMII_2_0, SGMII, or
*		1000BASE_X (defined in xaxiethernet.h).
*
* @note 	C-style signature:
* 		int XAxiEthernet_GetPhysicalInterface(XAxiEthernet
*							*InstancePtr)
*
******************************************************************************/
#define XAxiEthernet_GetPhysicalInterface(InstancePtr)	   \
	((InstancePtr)->Config.PhyType)

/****************************************************************************/
/**
*
* XAxiEthernet_GetIntStatus returns a bit mask of the interrupt status
* register (ISR). XAxiEthernet_GetIntStatus can be used to query the status
* without having to have interrupts enabled.
*
* @param	InstancePtr is a pointer to the Axi Ethernet instance to be
*		worked on.
*
* @return	Returns a bit mask of the interrupt status conditions. The
*		mask will be a set of bit wise or'd values from the
*		<code>XAE_INT_*_MASK</code> definitions in xaxitemac_hw.h file.
*
* @note		C-style signature:
*		u32 XAxiEthernet_GetIntStatus(XAxiEthernet *InstancePtr)
*
*****************************************************************************/
#define XAxiEthernet_GetIntStatus(InstancePtr) \
	 XAxiEthernet_ReadReg((InstancePtr)->Config.BaseAddress, XAE_IS_OFFSET)

/****************************************************************************/
/**
*
* XAxiEthernet_IntEnable enables the interrupts specified in <i>Mask</i>. The
* corresponding interrupt for each bit set to 1 in <i>Mask</i>, will be
* enabled.
*
* @param	InstancePtr is a pointer to the Axi Ethernet instance to be
*		worked on.
*
* @param	Mask contains a bit mask of the interrupts to enable. The mask
*		can be formed using a set of bit wise or'd values from the
*		<code>XAE_INT_*_MASK</code> definitions in xaxitemac_hw.h file.
*
* @return	None.
*
* @note		C-style signature:
*		void XAxiEthernet_IntEnable(XAxiEthernet *InstancePtr,
*						u32 Mask)
*
*****************************************************************************/
#define XAxiEthernet_IntEnable(InstancePtr, Mask) 			\
	XAxiEthernet_WriteReg((InstancePtr)->Config.BaseAddress,  	\
		XAE_IE_OFFSET,						\
		XAxiEthernet_ReadReg((InstancePtr)->Config.BaseAddress, \
				XAE_IE_OFFSET) | ((Mask) & XAE_INT_ALL_MASK));

/****************************************************************************/
/**
*
* XAxiEthernet_IntDisable disables the interrupts specified in <i>Mask</i>. The
* corresponding interrupt for each bit set to 1 in <i>Mask</i>, will be
* disabled. In other words, XAxiEthernet_IntDisable uses the "set a bit to
* clear it" scheme.
*
* @param	InstancePtr is a pointer to the Axi Ethernet instance to be
*		worked on.
* @param	Mask contains a bit mask of the interrupts to disable. The mask
*		can be formed using a set of bit wise or'd values from the
*		<code>XAE_INT_*_MASK</code> definitions in xaxitemac_hw.h file
*
* @return	None.
*
* @note		C-style signature:
*		void XAxiEthernet_IntDisable(XAxiEthernet *InstancePtr,
*						u32 Mask)
*
*****************************************************************************/
#define XAxiEthernet_IntDisable(InstancePtr, Mask) 			\
	XAxiEthernet_WriteReg((InstancePtr)->Config.BaseAddress, 	 \
		XAE_IE_OFFSET,						 \
		XAxiEthernet_ReadReg((InstancePtr)->Config.BaseAddress, \
				XAE_IE_OFFSET) & ~((Mask) & XAE_INT_ALL_MASK));

/****************************************************************************/
/**
*
* XAxiEthernet_IntPending returns a bit mask of the pending interrupts. Each
* bit set to 1 in the return value represents a pending interrupt.
*
* @param	InstancePtr is a pointer to the Axi Ethernet instance to be
*		worked on.
*
* @return	Returns a bit mask of the interrupts that are pending. The mask
*		will be a set of bit wise or'd values from the
* 		<code>XAE_INT_*_MASK</code> definitions in xaxitemac_hw.h file.
*
* @note		C-style signature:
*		u32 XAxiEthernet_IntPending(XAxiEthernet *InstancePtr)
*
*****************************************************************************/
#define XAxiEthernet_IntPending(InstancePtr) \
	XAxiEthernet_ReadReg((InstancePtr)->Config.BaseAddress, XAE_IP_OFFSET)

/****************************************************************************/
/**
*
* XAxiEthernet_IntClear clears pending interrupts specified in <i>Mask</i>.
* The corresponding pending interrupt for each bit set to 1 in <i>Mask</i>,
* will be cleared. In other words, XAxiEthernet_IntClear uses the "set a bit to
* clear it" scheme.
*
* @param	InstancePtr is a pointer to the Axi Ethernet instance to be
*		worked on.
* @param	Mask contains a bit mask of the pending interrupts to clear.
*		The mask can be formed using a set of bit wise or'd values from
*		the <code>XAE_INT_*_MASK</code> definitions in xaxitemac_hw.h
*		file.
*
* @note		C-style signature:
*		void XAxiEthernet_IntClear(XAxiEthernet *InstancePtr,
*								u32 Mask)
*
*****************************************************************************/
#define XAxiEthernet_IntClear(InstancePtr, Mask) \
	XAxiEthernet_WriteReg((InstancePtr)->Config.BaseAddress, \
			XAE_IS_OFFSET, ((Mask) & XAE_INT_ALL_MASK))

/*****************************************************************************/
/**
*
* XAxiEthernet_IsExtFuncCap determines if the device is capable of the
* new/extend VLAN and multicast features.
*
* @param	InstancePtr is a pointer to the Axi Ethernet instance to be
*		worked on.
*
* @return
*		- TRUE if the device is capable and configured with extended
*		  Multicast and VLAN Tagging/Stripping and Translation.
*		- TRUE if the device is NOT capable and NOT configured with
*		  extended Multicast and VLAN Tagging/Stripping and
*		  Translation.
*
* @note		C-style signature:
*		u32 XAxiEthernet_IsExtFuncCap(XAxiEthernet *InstancePtr)
*
 *****************************************************************************/
#define XAxiEthernet_IsExtFuncCap(InstancePtr)				\
		((XAxiEthernet_ReadReg((InstancePtr)->Config.BaseAddress, \
			XAE_RAF_OFFSET) & XAE_RAF_NEWFNCENBL_MASK) ?  \
				TRUE : FALSE)

/*****************************************************************************/
/**
*
* XAxiEthernet_IsExtMcastEnable determines if the extended multicast features
* is enabled.
*
* @param	InstancePtr is a pointer to the Axi Ethernet instance to be
*		worked on.
*
* @return
*		- TRUE if the extended multicast features are enabled.
*		- FALSE if the extended multicast features are NOT enabled
*
* @note :	This function indicates when extended Multicast is enabled in
*		HW, extended multicast mode in wrapper can be tested.
*
* C-style signature:
*	 u32 XAxiEthernet_IsExtMcastEnable(XAxiEthernet *InstancePtr)
*
*****************************************************************************/
#define XAxiEthernet_IsExtMcastEnable(InstancePtr)			\
		((XAxiEthernet_ReadReg((InstancePtr)->Config.BaseAddress, \
			XAE_RAF_OFFSET) & XAE_RAF_EMULTIFLTRENBL_MASK) ? \
				TRUE : FALSE)

/*****************************************************************************/
/**
*
* XAxiEthernet_IsExtMcast determines if the device is built with new/extended
* multicast features.
*
* @param	InstancePtr is a pointer to the Axi Ethernet instance to be
*		worked on.
*
* @return
*		- TRUE if the device is built with extended multicast features.
*		- FALSE if the device is not built with the extended multicast
*		  features.
*
* @note		This function indicates when hardware is built with extended
* 		Multicast feature.
*
* C-style signature:
*	 u32 XAxiEthernet_IsExtMcast(XAxiEthernet *InstancePtr)
*
*****************************************************************************/
#define XAxiEthernet_IsExtMcast(InstancePtr)	\
	(((InstancePtr)->Config.ExtMcast) ? TRUE : FALSE)

/*****************************************************************************/
/**
*
* XAxiEthernet_IsTxVlanStrp determines if the device is configured with transmit
* VLAN stripping on the transmit channel.
*
* @param	InstancePtr is a pointer to the Axi Ethernet instance to be
*		worked on.
*
* @return
*	 	- TRUE if the device is configured with
*		  VLAN stripping on the Transmit channel.
*		- FALSE if the device is NOT configured with
*		  VLAN stripping on the Transmit channel.
*
* @note 	C-style signature:
* 		u32 XAxiEthernet_IsTxVlanStrp(XAxiEthernet *InstancePtr)
*
*****************************************************************************/
#define XAxiEthernet_IsTxVlanStrp(InstancePtr)	\
	(((InstancePtr)->Config.TxVlanStrp) ? TRUE : FALSE)

/*****************************************************************************/
/**
*
* XAxiEthernet_IsRxVlanStrp determines if the device is configured with receive
* VLAN stripping on the receive channel.
*
* @param	InstancePtr is a pointer to the Axi Ethernet instance to be
*		worked on.
*
* @return
*	 	- TRUE if the device is configured with
*		  VLAN stripping on the Receive channel.
*		- FALSE if the device is NOT configured with
*		  VLAN stripping on the Receive channel.
*
* @note 	C-style signature:
*		u32 XAxiEthernet_IsRxVlanTran(XAxiEthernet *InstancePtr)
*
 *****************************************************************************/
#define XAxiEthernet_IsRxVlanStrp(InstancePtr)	\
	(((InstancePtr)->Config.RxVlanStrp) ? TRUE : FALSE)

/*****************************************************************************/
/**
*
* XAxiEthernet_IsTxVlanTran determines if the device is configured with
* transmit VLAN translation on the transmit channel.
*
* @param	InstancePtr is a pointer to the Axi Ethernet instance to be
*		worked on.
*
* @return
*	 	- TRUE if the device is configured with
*		 VLAN translation on the Transmit channel.
*		- FALSE if the device is NOT configured with
*		 VLAN translation on the Transmit channel.
*
* @note 	C-style signature:
*		u32 XAxiEthernet_IsTxVlanTran(XAxiEthernet *InstancePtr)
*
*****************************************************************************/
#define XAxiEthernet_IsTxVlanTran(InstancePtr)	\
	(((InstancePtr)->Config.TxVlanTran) ? TRUE : FALSE)

/*****************************************************************************/
/**
*
* XAxiEthernet_IsRxVlanTran determines if the device is configured with receive
* VLAN translation on the receive channel.
*
* @param	InstancePtr is a pointer to the Axi Ethernet instance to be
*		worked on.
*
* @return
*	 	- TRUE if the device is configured with
*		  VLAN translation on the Receive channel.
*		- FALSE if the device is NOT configured with
*		  VLAN translation on the Receive channel.
*
* @note 	C-style signature:
*		u32 XAxiEthernet_IsRxVlanTran(XAxiEthernet *InstancePtr)
*
*****************************************************************************/
#define XAxiEthernet_IsRxVlanTran(InstancePtr)	\
	(((InstancePtr)->Config.RxVlanTran) ? TRUE : FALSE)

/*****************************************************************************/
/**
*
* XAxiEthernet_IsTxVlanTag determines if the device is configured with transmit
* VLAN tagging on the transmit channel.
*
* @param	InstancePtr is a pointer to the Axi Ethernet instance to be
*		worked on.
*
* @return
*	 	- TRUE if the device is configured with
*		  VLAN tagging on the Transmit channel.
*		- FALSE if the device is NOT configured with
*		  VLAN tagging on the Transmit channel.
*
* @note 	C-style signature:
* 		u32 XAxiEthernet_IsTxVlanTag(XAxiEthernet *InstancePtr)
*
*****************************************************************************/
#define XAxiEthernet_IsTxVlanTag(InstancePtr)	\
	(((InstancePtr)->Config.TxVlanTag) ? TRUE : FALSE)

/*****************************************************************************/
/**
*
* XAxiEthernet_IsRxVlanTag determines if the device is configured with receive
* VLAN tagging on the receive channel.
*
* @param	InstancePtr is a pointer to the Axi Ethernet instance to be
*		worked on.
*
* @return
*	 	- TRUE if the device is configured with
*		  VLAN tagging on the Receive channel.
*		- FALSE if the device is NOT configured with
*		  VLAN tagging on the Receive channel.
*
* @note 	C-style signature:
* 		u32 XAxiEthernet_IsRxVlanTag(XAxiEthernet *InstancePtr)
*
 *****************************************************************************/
#define XAxiEthernet_IsRxVlanTag(InstancePtr)	\
	(((InstancePtr)->Config.RxVlanTag) ? TRUE : FALSE)

/*****************************************************************************/
/**
*
* XAxiEthernet_GetTemacType returns the Axi Ethernet type of the core.
*
* @param	InstancePtr is a pointer to the Axi Ethernet instance to be
*		worked on.
*
* @return
*	 	- Returns the values of TemacType, which can be
*		  XAE_SOFT_TEMAC_10_100_MBPS (0) for Soft Temac Core with
*							speed 10/100 Mbps.
*		  XAE_SOFT_TEMAC_10_100_1000_MBPS (1) for Soft Temac core with
*							speed 10/100/1000 Mbps
*		  XAE_HARD_TEMC (2) for Hard Temac Core for Virtex-6
*
* @note 	C-style signature:
* 		u32 XAxiEthernet_GetTemacType(XAxiEthernet *InstancePtr)
*
 *****************************************************************************/
#define XAxiEthernet_GetTemacType(InstancePtr)	\
	((InstancePtr)->Config.TemacType)

/*****************************************************************************/
/**
*
* XAxiEthernet_IsAvbConfigured returns determines if Ethernet AVB.is configured
* in the harwdare or not.
*
* @param	InstancePtr is a pointer to the Axi Ethernet instance to be
*		worked on.
*
* @return
*	 	- TRUE if the device is configured with
*		  Ethernet AVB.
*		- FALSE if the device is NOT configured with
*		  Ethernet AVB.
*
* @note 	C-style signature:
* 		u32 XAxiEthernet_IsAvbConfigured(XAxiEthernet *InstancePtr)
*
 *****************************************************************************/
#define XAxiEthernet_IsAvbConfigured(InstancePtr)	\
	(((InstancePtr)->Config.Avb) ? TRUE : FALSE)

/*****************************************************************************/
/**
*
* XAxiEthernet_IsSgmiiOverLvdsEnabled determines if SGMII over LVDS is enabled
* in the harwdare or not.
*
* @param	InstancePtr is a pointer to the Axi Ethernet instance to be
*		worked on.
*
* @return
*	 	- TRUE if the device is configured with
*		  SGMII over LVDS.
*		- FALSE if the device is NOT configured with
*		  SGMII over LVDS.
*
* @note 	C-style signature:
* 		u32 XAxiEthernet_IsSgmiiOverLvdsEnabled(XAxiEthernet *InstancePtr)
*
 *****************************************************************************/
#define XAxiEthernet_IsSgmiiOverLvdsEnabled(InstancePtr)	\
	(((InstancePtr)->Config.EnableSgmiiOverLvds) ? TRUE : FALSE)

/*****************************************************************************/
/**
*
* XAxiEthernet_IsStatsConfigured returns determines if Statistics gathering.
* is configured in the harwdare or not.
*
* @param	InstancePtr is a pointer to the Axi Ethernet instance to be
*		worked on.
*
* @return
*	 	- TRUE if the device is configured with
*		  statistics gathering.
*		- FALSE if the device is NOT configured with
*		  statistics gathering.
*
* @note 	C-style signature:
* 		u32 XAxiEthernet_IsStatsConfigured(XAxiEthernet *InstancePtr)
*
 *****************************************************************************/
#define XAxiEthernet_IsStatsConfigured(InstancePtr)	\
	(((InstancePtr)->Config.Stats) ? TRUE : FALSE)

/************************** Function Prototypes ******************************/

/*
 * Initialization functions in xaxiethernet.c
 */
int XAxiEthernet_CfgInitialize(XAxiEthernet *InstancePtr,
			XAxiEthernet_Config *CfgPtr,UINTPTR VirtualAddress);
int XAxiEthernet_Initialize(XAxiEthernet *InstancePtr,
			    XAxiEthernet_Config *CfgPtr, UINTPTR VirtualAddress);
void XAxiEthernet_Start(XAxiEthernet *InstancePtr);
void XAxiEthernet_Stop(XAxiEthernet *InstancePtr);
void XAxiEthernet_Reset(XAxiEthernet *InstancePtr);

/*
 * Initialization functions in xaxitemac_sinit.c
 */
XAxiEthernet_Config *XAxiEthernet_LookupConfig(u16 DeviceId);

/*
 * MAC configuration/control functions in xaxitemac_control.c
 */
int XAxiEthernet_SetOptions(XAxiEthernet *InstancePtr, u32 Options);
int XAxiEthernet_ClearOptions(XAxiEthernet *InstancePtr, u32 Options);
u32 XAxiEthernet_GetOptions(XAxiEthernet *InstancePtr);

int XAxiEthernet_SetMacAddress(XAxiEthernet *InstancePtr, void *AddressPtr);
void XAxiEthernet_GetMacAddress(XAxiEthernet *InstancePtr, void *AddressPtr);

int XAxiEthernet_SetMacPauseAddress(XAxiEthernet *InstancePtr,
							void *AddressPtr);
void XAxiEthernet_GetMacPauseAddress(XAxiEthernet *InstancePtr,
							void *AddressPtr);
int XAxiEthernet_SendPausePacket(XAxiEthernet *InstancePtr, u16 PauseValue);

int XAxiEthernet_GetSgmiiStatus(XAxiEthernet *InstancePtr, u16 *SpeedPtr);
int XAxiEthernet_GetRgmiiStatus(XAxiEthernet *InstancePtr, u16 *SpeedPtr,
				  int *IsFullDuplexPtr, int *IsLinkUpPtr);
u16 XAxiEthernet_GetOperatingSpeed(XAxiEthernet *InstancePtr);
int XAxiEthernet_SetOperatingSpeed(XAxiEthernet *InstancePtr, u16 Speed);

void XAxiEthernet_SetBadFrmRcvOption(XAxiEthernet *InstancePtr);
void XAxiEthernet_ClearBadFrmRcvOption(XAxiEthernet *InstancePtr);

void XAxiEthernet_DisableControlFrameLenCheck(XAxiEthernet *InstancePtr);
void XAxiEthernet_EnableControlFrameLenCheck(XAxiEthernet *InstancePtr);

void XAxiEthernet_PhySetMdioDivisor(XAxiEthernet *InstancePtr, u8 Divisor);
void XAxiEthernet_PhyRead(XAxiEthernet *InstancePtr, u32 PhyAddress,
					u32 RegisterNum, u16 *PhyDataPtr);
void XAxiEthernet_PhyWrite(XAxiEthernet *InstancePtr, u32 PhyAddress,
					u32 RegisterNum, u16 PhyData);
int XAxiEthernet_MulticastAdd(XAxiEthernet *InstancePtr, void *AddressPtr,
								int Entry);
void XAxiEthernet_MulticastGet(XAxiEthernet *InstancePtr, void *AddressPtr,
								int Entry);
int XAxiEthernet_MulticastClear(XAxiEthernet *InstancePtr, int Entry);

int XAxiEthernet_SetTpid(XAxiEthernet *InstancePtr, u16 Tpid, u8 Entry);
int XAxiEthernet_ClearTpid(XAxiEthernet *InstancePtr, u8 Entry);
void XAxiEthernet_GetTpid(XAxiEthernet *InstancePtr, u16 *TpidPtr, u8 Entry);

int XAxiEthernet_SetVTagMode(XAxiEthernet *InstancePtr, u32 Mode, int Dir);
void XAxiEthernet_GetVTagMode(XAxiEthernet *InstancePtr, u8 *ModePtr, int Dir);

int XAxiEthernet_SetVStripMode(XAxiEthernet *InstancePtr, u32 Mode, int Dir);
void XAxiEthernet_GetVStripMode(XAxiEthernet *InstancePtr, u8 *ModePtr,
								int Dir);

int XAxiEthernet_SetVTagValue(XAxiEthernet *InstancePtr, u32 VTagValue,
								int Dir);
void XAxiEthernet_GetVTagValue(XAxiEthernet *InstancePtr, u32 *VTagValuePtr,
								int Dir);

int XAxiEthernet_SetVidTable(XAxiEthernet *InstancePtr, u32 Entry, u32 Vid,
						u8 Strip, u8 Tag, int Dir);
void XAxiEthernet_GetVidTable(XAxiEthernet *InstancePtr, u32 Entry, u32 *VidPtr,
					u8 *StripPtr, u8 *TagPtr, int Dir);

int XAxiEthernet_AddExtMulticastGroup(XAxiEthernet *InstancePtr,
							void *AddressPtr);
int XAxiEthernet_ClearExtMulticastGroup(XAxiEthernet *InstancePtr,
							void *AddressPtr);
int XAxiEthernet_GetExtMulticastGroup(XAxiEthernet *InstancePtr,
							void *AddressPtr);
void XAxiEthernet_DumpExtMulticastGroup(XAxiEthernet *InstancePtr);

#ifdef __cplusplus
}
#endif

#endif /* end of protection macro */
/** @} */
