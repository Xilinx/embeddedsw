/******************************************************************************
*
* Copyright (C) 2015 - 2016 Xilinx, Inc. All rights reserved.
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
*
* Use of the Software is limited solely to applications:
* (a) running on a Xilinx device, or
* (b) that interact with a Xilinx device through a bus or interconnect.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
* XILINX BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
* WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
* OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*
* Except as contained in this notice, the name of the Xilinx shall not be used
* in advertising or otherwise to promote the sale, use or other dealings in
* this Software without prior written authorization from Xilinx.
*
******************************************************************************/
/*****************************************************************************/
/**
*
* @file xhdcp1x.h
* @addtogroup hdcp1x_v4_0
* @{
* @details
*
* This is the Xilinx HDCP device drivers. The Xilinx HDCP driver software is
* provided to allow for the integration of HDCP functionality into designs
* that make use of the DisplayPort and HDMI LogiCORE IP video interface
* cores. This software is distributed within the Xilinx SDK (Software
* Development Kit) and is tightly coupled with the SDK drivers for these
* interfaces. This appendix serves as a technical reference describing the
* architecture and functionality of HDCP driver software as well as providing
* guidelines for integrating it into a custom DisplayPort and/or HDMI system
* designs based on Xilinx programmable devices.
*
* <b> Platform Requirements </b>
* The current platform requirements for the HDCP driver software are as
* follows:
* - One or more of the following video interfaces (subject to any and all
*   identified Restrictions and Limitations):
*    - DisplayPort receive
*    - DisplayPort transmit
*    - HDMI receive
*    - HDMI transmit
* - One HDCP 1.x cipher LogiCORE IP instance per video interface that
*   supports HDCP 1.x.
* - One HDCP device key set per video interface that supports HDCP 1.x and
*   the associated RTL/IP to interface these to the HDCP 1.x ciphers.
* - One timer resource per HDCP 1.x transmit interface that can support
*   durations from a few milliseconds up to several seconds in length.
* - Approximately 128 KB of free code space (HDCP specifics only)
* - Approximately 1 KB of free data space per HDCP interface (HDCP specifics
*   only).
*
* <b> Restrictions and Limitations </b>
* The HDCP driver software has the following restrictions and limitations:
* - DisplayPort receiver and HDMI receiver interfaces are not supported
*   within the same platform design.
* - DisplayPort transmitter and HDMI transmitter interfaces are not supported
*   within the same platform design.
*
* -- -- -- -- -- -- -- -- -- -- -- -- --
*
* <b> HDCP Driver Architecture </b>
*
* The architecture of the HDCP software driver can be divided into three
* distinct submodules.
* - Cipher device
* - Port device
* - Authentication state machine(s)
* Each of these modules represents a specific function and/or entity within
* the driver and work in unison to implement the HDCP software support within
* the system design. The design of the driver encapsulates all of these
* submodules into a unified API that is used by the client software
* application with a single instance of an HDCP interface.
*
* <b> Cipher Device </b>
*
* The cipher device is contained within the HDCP driver software and represents
* an instance of the HDCP cipher LogiCORE IP block within a system design.
* Given that the variants of this cipher core provide the same interface to the
* software, a single implementation of this submodule is used to drive all of
* them. In the few places where behaviors differ slightly between variants,
* this software adjusts at runtime based on the contents of a Type/ID
* register within the IP core.
*
* The principal function of the cipher device submodule is to initialize,
* configure, and monitor the physical encryption cipher used within the HDCP
* application. As part of this, it provides an interface that allows the
* software to trigger the reading of the appropriate device key set
* from the custom key management solution that is attached to the cipher core.
*
* The cipher device submodule is used by higher-level authentication state
* machines to run re-authentication requests, generate pseudo-random numbers,
* and enable/disable encryption on individual video streams passing through
* the cipher core.
*
* It also provides a status output from the actual encryption process that can
* be used by the authentication state machines to monitor the correct operation
* of the HDCP function.
*
* The interrupts generated by the cipher core are fully consumed within the
* HDCP software driver; the client software application needs only to register
* the cipher core interrupt with the associated vector within the appropriate
* interrupt controller of the system design. These interrupts are forwarded to
* the cipher submodule through an appropriate external API of the software
* driver (XHdcp1x_CipherIntrHandler).
*
* The software driver source files that are associated with this submodule are
* as follows:
* - xhdcp1x_cipher.c
* - xhdcp1x_cipher.h
* - xhdcp1x_cipher_intr.c
* - xhdcp1x_hw.h
*
* <b> Port Device </b>
*
* The port device(s) that exists within the HDCP software driver represent the
* management and access to the HDCP register sets that are defined within the
* HDCP specifications. While the register sets are similar between HDMI and
* DisplayPort, they do differ, and as a result there are slightly different
* implementations of these port devices between the two. Regardless of the
* physical interface type, the transmit port device is always the master of the
* register set transactions and the receive port device is always the slave.
*
* For transmit port devices, they provide the ability to read and write the
* HDCP register set that exists on their attached receive (slave) device.
* These transactions are always run to completion and are initiated solely by
* the corresponding transmit state machine and/or as part of a debug process
* if explicitly requested.
*
* For receive port devices, they only provide access to the local HDCP register
* set; they cannot be used to read the register set from the attached transmit
* device. Local reads and writes of this register set are performed by the
* appropriate receive state machine (or debug process) and remote reads and
* writes can be performed by the transmit port (master) device.
*
* Remote access (either read or write) to a subset of the register set
* generates notifications (for example, interrupts) to the device that are then
* in turn posted to the corresponding receive state machine for driving
* authentication and related procedures. The underlying implementation of the
* supported HDCP port device modules is completely dependent on the Xilinx BSP
* device driver for the corresponding video interface type. As a result, the
* client application software is required to provide the device handle of the
* video interface that the specific HDCP driver instance is bound to during
* initialization of the HDCP software driver.
*
* For more information on the BSP Standalone driver, see the following
* documents:
* - Xilinx Software Developer Kit Help (UG782)
* - OS and Libraries Document Collection (UG643)
*
* The internals of the port device rely on the use of an adapter structure
* (a jump table) which maps the HDCP port device API into a protocol
* (DisplayPort/HDMI) and direction (RX/TX) specific function that implements
* the associated functionality.
* This adapter structure (XHdcp1x_PortPhyIfAdaptor) allows for the correct
* function to be determined and called at run-time based on the exact type of
* the HDCP interface in question. In some instances (specifically DisplayPort
* TX), there is no specific port device interrupt defined and, as such, the
* relevant interrupt(s) (HPD) must be forwarded to the port device submodule
* through an appropriate external API of the software driver
* (XHdcp1x_PortIntrHandler).
*
* The software driver source files that are associated with this submodule
* are as follows:
* - xhdcp1x_port.c
* - xhdcp1x_port.h
* - xhdcp1x_port_dp.h
* - xhdcp1x_port_dp_rx.c
* - xhdcp1x_port_dp_tx.c
* - xhdcp1x_port_hdmi.h
* - xhdcp1x_port_hdmi_rx.c
* - xhdcp1x_port_hdmi_tx.c
* - xhdcp1x_port_intr.c
*
* <b> Authentication State Machines </b>
*
* The transmit and receive authentication state machines contained within the
* driver software mirror those that are defined within the appropriate HDCP
* specifications. To facilitate integration and debugging, the names of the
* individual states are aligned with those defined within the specifications
* wherever possible.
*
* The state machines are responsible for driving the authentication,
* encryption, and link monitoring processes as requested by the client
* application software.
*
* The state machines co-ordinate the operation of the cipher, port device(s),
* and the physical video interface (HDMI or DisplayPort), and rely upon the
* proper programming and availability of HDCP device keys.
*
* The state machines make use of a simple, OS-independent scheduler to allow
* for messages and requests to be posted to and from them. This requires the
* client application software to regularly call a poll function on each HDCP
* interface to ensure the timely servicing of these messages/requests.
*
* The transmit state machine provides interfaces that allow for the client
* application software to request authentication, enable/disable the encryption
* of video streams, and monitor the progress/outcome of authentication and the
* ongoing integrity of the encrypted video data. The transmit state must be
* notified of changes in the physical state of the interface it is running over
* (either HDMI or DisplayPort) to ensure that any required re-authentication is
* performed without explicit intervention of the client software application.
* It fully supports downstream HDCP receivers and repeaters. For a detailed
* diagram of this state machine, see either figure 2-6 in the High-bandwidth
* Digital Content Protection System v1.3 Amendment for DisplayPort
* specification or figure 2-9 in the High-bandwidth Digital Content
* Protection System v1.4 specification. The transmit state definitions are
* contained within the XHdcp1x_StateType contained within the
* xhdcp1x_tx.c source file.
*
* The receive state machine is much simpler than the transmit in that it is
* not entirely aware of the success/failure of the authentication process.
* It merely initiates the calculations required for authentication and makes
* the result available within its HDCP register set for examination by the
* transmitter. It allows for the client application software to explicitly
* enable/disable the HDCP capability of the underlying video interface and to
* query whether encrypted video is being received at any point in time. It
* also requires that it be notified of changes in the physical interface it is
* running over so that the appropriate side effects can be initiated. For a
* detailed diagram of this state machine, see either figure 2-7 in the the
* High-bandwidth Digital Content Protection System v1.3 Amendment for
* DisplayPort specification or figure 2-10 in the High-bandwidth Digital
* Content Protection System v1.4 specification. The receive state definitions
* are contained within the XHdcp1x_StateType contained within the
* xhdcp1x_rx.c source file.
*
* Both the transmit and receive state machines make use of a link verification
* procedure to ensure that both ends of the encrypted link remain synchronized
* over time. This mechanism differs between DisplayPort and HDMI and is
* detailed in section 2.2.4 of the High-bandwidth Digital Content Protection
* System v1.3 Amendment for DisplayPort specification [Ref 1] and section 2.2.3
* of the High-bandwidth Digital Content Protection System v1.4 specification
* respectively.
*
* The software driver source files that are associated with this submodule are
* as follows:
* - xhdcp1x.c
* - xhdcp1x.h
* - xhdcp1x_intr.c
* - xhdcp1x_platform.c
* - xhdcp1x_platform.h
* - xhdcp1x_rx.c
* - xhdcp1x_rx.h
* - xhdcp1x_selftest.c
* - xhdcp1x_sinit.c
* - xhdcp1x_tx.c
* - xhdcp1x_tx.h
*
* <b> Device Key Management </b>
*
* The device key management software and RTL/IP implementation interact with
* the HDCP software driver but are, in the strictest sense, outside of the
* scope of this driver. The Xilinx HDCP solution is designed such that each
* individual platform integrator is responsible for the security and storage of
* the HDCP device keys that are issued to them by DCP LLC. The physical
* interface between the LogiCORE IP HDCP Cipher Product Guide and the custom
* implemented key storage is described in the High-bandwidth Digital Content
* Protection System v1.3 Amendment for DisplayPort specification and the
* High-bandwidth Digital Content Protection System v1.4 specification.
*
* <b> Dependencies </b>
*
* The only dependencies between the HDCP driver software and the actual device
* key management implementation are, as follows:
*   - The device keys must be available to be loaded into the HDCP cipher
*   LogiCORE IP prior to initializing the corresponding instance of the
*   HDCP software driver.
*   - The HDCP software driver supports the programming of key set selection
*   vector. This can be used in applications where multiple sets of device keys
*   are available for use. This vector allows for one of up to eight different
*   device key sets to be selected as the data to be loaded when requested by
*   the cipher core. The API function used to set this selection vector
*   is XHdcp1x_SetKeySelect.
*
* -- -- -- -- -- -- -- -- -- -- -- -- --
*
* <b> HDCP Driver Porting Guide </b>
*
* The following system resources are required by the HDCP software driver
* software:
* - Approximately 128 KB of free code space
* - Approximately 1 KB of free data space per HDCP driver instance
* - Approximately 1 KB of stack space
* - One dedicated operating system thread/task for HDCP (if an operating
*   system is being used)
* Note: These resource requirements are those needed explicitly for HDCP. The
* requirements associated with the mandatory video interface(s), HDMI or
* DisplayPort (DP) are not included.
*
* <b> Integrating Into Your Application </b>
*
* The HDCP software driver can be integrated into any client software
* application, regardless of whether an operating system is used or not. This
* integration generally consists of a few simple steps.
*
* 1. Allocation of the required number of HDCP driver instances.
*
* 2. Binding of the system timer interface functions (only needed if HDCP
*    transmit interfaces are to be used). These functions are used by the state
*    machines to run guard timers where appropriate within the authentication
*    procedure. These functions are, as follows:
*        - XHdcp1x_SetTimerStart
*        - XHdcp1x_SetTimerStop
*        - XHdcp1x_SetTimerDelay
*
* 3. Binding of the platform-specific timer expiry handling to call
*    XHdcp1x_HandleTimeout when a timer started using the interfaces in step
*    2 expires.
*
* 4. Optional binding of the platform specific function that tests KSV against
*    a stored revocation list (HDCP transmit interfaces only) using
*    XHdcp1x_SetKsvRevokeCheck.
*
* 5. Initialization, self-test, and basic configuration of the HDCP driver
*    instances. The relevant functions are:
*        - XHdcp1x_CfgInitialize
*        - XHdcp1x_LookupConfig
*        - XHdcp1x_SelfTest
*        - XHdcp1x_SetKeySelect
*
* 6. Binding of the HDCP cipher core interrupt lines and callback functions
*    with the platform interrupt controller(s). The relevant function is
*    XHdcp1x_CipherIntrHandler.
*
* 7. Registration of the HDCP 1.x port interrupt handler with the corresponding
*    HDMI or DisplayPort device driver. The relevant function is
*    XHdcp1x_PortIntrHandler.
*        - DisplayPort RX: not used
*        - DisplayPort TX: insert into the HPD event and interrupt callback
*          functions
*        - HDMI RX/TX: not used
*
* 8. Insertion of calls to HDCP set physical state and lane count function(s)
*    in the appropriate physical interface driver callback functions (HDMI or
*    DisplayPort). The functions of interest are:
*        - XHdcp1x_SetPhysicalState
*             - DisplayPort RX: insert into the driver callbacks that handle
*               training done (up), training lost (down), bandwidth change
*               (down), and cable unplug (down)
*             - DisplayPort TX: insert in the function that starts/trains the
*               DisplayPort link (up or down based on the outcome)
*             - HDMI RX/TX: call in the stream up/down callback functions
*        - XHdcp1x_SetLaneCount
*             - DisplayPort RX: insert into the training done driver callback
*             - DisplayPort TX: insert in the function that starts/trains the
*               DisplayPort link
*             - HDMI RX/TX: not used
*
* 9. Insertion of a call to poll each of the HDCP driver instances within
*    either the main loop of the client software application (bare-metal) or
*    within an HDCP specific thread (operating system). The relevant function
*    within the driver software is XHdcp1x_Poll.
*
* 10. Insertion of calls where appropriate within the client specific
*     application software to administratively manage and monitor the operation
*     of the HDCP driver instances.
*        - XHdcp1x_Authenticate (transmit interfaces only)
*        - XHdcp1x_Disable
*        - XHdcp1x_DisableEncryption (transmit interfaces only)
*        - XHdcp1x_Enable
*        - XHdcp1x_EnableEncryption (transmit interfaces only)
*        - XHdcp1x_GetEncryption
*        - XHdcp1x_IsAuthenticated
*        - XHdcp1x_IsInProgress
*
* It is important to note that the physical video interfaces over which HDCP
* is to be run must physically exist and be initialized prior to the
* initialization of the corresponding HDCP driver instance. If this is not the
* case, then the HDCP driver software will not work as designed. In addition,
* interrupts on the physical interface should not be enabled until after the
* corresponding HDCP driver instance has been initialized. While the successful
* operation of this driver relies on the use interrupts, there are no external
* interrupt callbacks to be registered specifically by the client software
* using the HDCP software driver. All of the HDCP-specific interrupts are fully
* consumed within the software driver and all the client software needs to do
* is properly complete integration using step 6, step 7, and step 8.
*
* IMPORTANT: Virtually all of the exported API functions that do not query the
* state of the driver require a subsequent call to the poll function in order
* for the request to be fully serviced by the indicated driver instance. As a
* result, it is crucial that the interface be polled in a timely fashion
* subsequent to such an API call.
*
* <b> Debugging </b>
*
* The HDCP software driver software provides two different mechanisms for
* facilitating the integration and debug into client application software.
* These are:
* - Runtime debug logging information
* - On demand detailed status display
*
* To enable the runtime debug logging of the driver software, the client
* application can register a printf-style debug logging function with the
* driver using the appropriate API.
*
* If this is done, then the authentication state machines log all state
* transitions and periodic status information, providing significant insight
* into their operation. The function that can be used to register this logging
* function is XHdcp1x_SetDebugLogMsg.
*
* Note: This debug logging is enabled/disabled on the driver as a whole (as
* opposed to on a single interface at a time) and does not support priorities
* of any kind.
*
* In addition to runtime logging, the client application can register a
* printf-style debug print function with the driver software.
* This registration permits the driver to perform a one-time dump of detailed
* status information related to a specific HDCP driver instance when triggered
* by the client application software. The relevant functions for this are:
* - XHdcp1x_SetDebugPrintf
* - XHdcp1x_Info
*
* The information displayed using the XHdcp1x_Info function includes details
* on the current/previous state, encryption status, driver/core version(s),
* as well as statistics detailing the operation of the authentication state
* machine (RX and TX) and as its corresponding cipher and port sub-modules.
* The format of the information displayed differs slightly based on whether
* the HDCP interface is in the receive or transmit direction
*
* -- -- -- -- -- -- -- -- -- -- -- -- --
*
* <b> Receive Authentication </b>
*
* The CE register accesses associated with the receiver authentication should
* be as follows:
* - Write of the Control register to enable the core, enable immediate register
*   updates and set the number of lanes to be used.
* - Write the Cipher Control register to set Bit[0] (the XOR function is to be
*   enabled on the receiver for authentication).
* - Write of the Key Management Block Control register to load local KSV. Set
*   and then immediately clear Bit[0].
* - Read of Key Management Block Status register and check Bit[0] to confirm
*   local KSV available (poll as required).
* - Read of the Local KSV (H) and Local KSV (L) registers.
* - Write of the Remote KSV (H) and Remote KSV (L) registers with the KSV
*   value sent by the TX.
* - Write of the Key Management Block Control register to trigger the
*   calculation of Km. Set and then immediately clear Bit[1]. Set Key
*   Management Block control register to abort Km calculation, and then
*   immediately clear Bit[2].
* - Read of the Key Management Status register and check Bit[1] to confirm Km
*   available (poll as required).
* - Write of Cipher Bx, By, and Bz registers (with An).
* - Read the calculated value of Km and write of Cipher Kx, Ky, and Kz
*   registers (with Km) if needed by the Cipher version.
* - Write to Cipher Control register to initiate block cipher. Clear Bit[16]
*   to ensure that the calculated Km value is used. Set and then immediately
*   clear Bit[8].
* - Read of Cipher Status register and check Bit[8] to confirm block cipher
*   complete (poll as required).
* - Read of Cipher Ri register.
*
* Note: Any step that requires the writing of multiple registers are
* "book ended" with the clearing and subsequent setting of the register update
* bit within the Control register.
*
* -- -- -- -- -- -- -- -- -- -- -- -- --
*
* <b> Transmit Authentication </b>
*
* The CE register accesses associated with the transmitter authentication
* should be as follows:
* - Write of the Control register to enable the core, enable immediate register
*   updates and set the number of lanes to be used.
* - Write the Cipher Control register to clear Bit[0] (the XOR function is to
*   be disabled on the transmitter for authentication).
* - Write of the Cipher Control register to generate random number. Bit[10]
*   should be set and immediately cleared.
* - Read of Cipher Status register and check Bit[10] to confirm RNG complete
*   (poll as required).
* - Read of the Cipher Mi (H) and Cipher Mi (L) registers (this value is An).
* - Write of the Key Management Control register to load local KSV. Set and
*   then immediately clear Bit[0].
* - Read of Key Management Status register and check Bit[0] to confirm local
*   KSV available (poll as required).
* - Read of the Local KSV (H) and Local KSV (L) registers.
* - Write of the Remote KSV (H) and Remote KSV (L) registers with the value
*   read from the RX.
* - Write of the Key Management Control register to calculate Km. Set and
*   immediately clear Bit[1]. Set Key Management Block control register to
*   abort Km calculation, and then immediately clear Bit[2]
* - Read of the Key Management Status register and check Bit[1] to confirm Km
*   available (poll as required).
* - Write of Cipher Bx, By, and Bz registers (with An + repeater bit).
* - Read the calculated value of Km and write of Cipher Kx, Ky, and Kz
*   registers (with Km) if needed by the Cipher version.
* - Write to Cipher Control register to initiate block Cipher. Clear Bit[16]
*   to ensure that the calculated Km value is used. Set and then immediately
*   clear Bit[8].
* - Read of Cipher Status register and check Bit[8] to confirm block cipher
*   complete (poll as required).
* - Read of Cipher Ri register.
*
* Note: Any step that requires the writing of multiple registers are
* "book ended" with the clearing and subsequent setting of the register update
* bit within the Control register.
*
* -- -- -- -- -- -- -- -- -- -- -- -- --
*
* <b> HDCP REPEATER </b>
*
* The HDCP Repeater functionality is implemented in the drivers. Here however,
* the user has to ensure that the right setup is available for the HDCP
* repeater drivers.
* - The use must also ensure that the variable IsRepeater in the instance
*   of the structure XHdcp1x defined in the user application is set to 1. For
*   example, DpRxSsInst.Hdcp1x->IsRepeater = 1.
*
* <b> Requirements </b>
*
* - The system has both the receiver and the transmitter interface.
* - The IsRepeater variable of the hdcp core structure, XHdcp1x is set.
* - The callbacks for Repeater are set in the user application.
*
* <b> Repeater State Machine for Transmitter </b>
*
* For the transmitter system, the repeater state machine has the following
* states:
*  - STATE_DISABLED
*  - STATE_UNAUTHENTICATED
*  - STATE_COMPUTATIONS
*  - STATE_WAITFORDOWNSTREAM (in repeater only)
*  - STATE_ASSEMBLEKSVLIST (in repeater only)
*  - STATE_AUTHENTICATED
*  - STATE_LINKINTEGRITYFAILED
*  - STATE_PHYDOWN
*
*  For the receiver system, the repeater state machine has the following
*  states:
*  - STATE_DISABLED
*  - STATE_DETERMINERXCAPABLE
*  - STATE_EXCHANGEKSVS
*  - STATE_COMPUTATIONS
*  - STATE_VALIDATERX
*  - STATE_AUTHENTICATED
*  - STATE_LINKINTEGRITYCHECK
*  - STATE_TESTFORREPEATER
*  - STATE_WAITFORREADY (in repeater only)
*  - STATE_READKSVLIST (in repeater only)
*  - STATE_UNAUTHENTICATED
*  - STATE_PHYDOWN
*
* <b> Repeater Flow </b>
*
* <pre>
*   |> Receiver <|   |> User App <|        |> Transmitter <|
*
* -> AKSV Written
*    to Receiver
*       |
*    Ro' written
*    to upstream
*       |
*       ---->   Trigger Downstream
*               (Hdcp1x.Rx.Repeater
*               DownstreamAuthCallBack) ----> Transmitter writes
*                                             AKSV and An downstream
*                                                       |
*                                             Transmitter reads back
*                                             Ro' and completes
*                                             first part of
*                                             authentication
*                                                       |
*                               -----(No)---- Is Downstream Repeater?
*                               |                       | (Yes)
*                               |             Read the downstream KSV
*                               |             FIFO, calculate the SHA-1
*                               |             value and complete 2nd part
*                               |             of authentication.
*                               |
*                               ------------> Setup the KSV FIFO for
*                                             upstream.
*                                                       |
*               Exchange Repeater <----------------------
*               Values (Hdcp1x.Tx.
*               ExchangeRepeaterCallBack)
*               Here Get the KSV
*               FIFO and update it
*               for the Receiver.
*                       |
*    Assemble the  <-----
*    KSV List and
*    calculate the SHA-1
*    value and write
*    it to upstream.
*          |
*    Goto Authenticated
*    state.
*
* </pre>
*
* The Repeater authentication requires the presence of both RX and TX
* interfaces. The authentication begins when the upstream transmitter writes
* the An and AKSV to the RX interface. After completing the first part of
* authentication the Receiver calls the DownstremaAuthCallBack callback
* handler. This handler should be set in the user application and should post
* the authenticate event to the TX state machine. This will trigger the TX
* authentication process. The TX completes the first part of authentication,
* by writing the AKSV and An downstreama and then reading an comparing the
* downstream Ro' value. If the downstream device is also Repeater, then the
* TX waits for the READY bit to get set in the downstream and reads the KSV
* FIFO. The TX then commences the second part of the authentication, where it
* calculates the SHA-1 value V and compares it with the V' read from the
* downstream. If the SHA-1 value V matches with the V' value read from
* downstream, authentication is successful. Here the TX sets up a
* RepeaterExchange value which it populates with the KSV FIFO of the
* downstream and the depth and device count values read from downstream. At
* this point the KSV FIFO calls the ExchangeRepeaterCallBack, which needs to
* be set in the user application. Here the user application receives the
* KSV FIFO value, depth and device count from the TX of the downstream
* topology. The user applicaition needs to update these value and set them
* for the receiver. At this point the RX is expected to be in the
* WAIT-FOR-READY state. Here the user should post EVENT_DOWNSTREAMREADY to the
* RX state machine. Consequently, the RX state machine will
* calculate the SHA-1 value, update the device count and depth value and set
* the READY bit for the upstream transmitter device to read and complete the
* authentication.
*
* If the authentication fails and the upstream device triggers the
* authentication again, then the entire authenitcation process is re-started.
*
* <b> HDCP (Repeater) Callback functions </b>
* - Transmitter
*   - XHdcp1xInst.Tx.RepeaterExchangeCallback (Repeater)
*   - XHdcp1xInst.Tx.UnauthenticatedCallback
* - Receiver
*   - XHdcp1xInst.Rx.RepeaterDownstreamAuthCallback (Repeater)
*   - XHdcp1xInst.Rx.AuthenticatedCallback
*   - XHdcp1xInst.Rx.UnauthenticatedCallback
*   - XHdcp1xInst.Rx.TopologyUpdateCallback
*   - XHdcp1xInst.Rx.EncryptionUpdateCallback
*
* Handler associated with each callback:
* - XHdcp1xInst.Tx.RepeaterExchangeCallback
*   - XHDCP1X_HANDLER_RPTR_RPTREXCHANGE
*   - RepeaterExchangeCallback has to be defined by the user, in order to
*     set get the KSV values from the downstream transmitter and set the
*     values in the Receiver's Ksv List. This function is called by the
*     drivers.
* - XHdcp1xInst.Tx.UnauthenticatedCallback
*   - XHDCP1X_HANDLER_UNAUTHENTICATED
*   - UnauthenticatedCallback has to be defined by the user , to handle
*     repeater situations where the transmitter state machine goes to the
*     unauthenticate state.
* - XHdcp1xInst.Rx.RepeaterDownstreamAuthCallback
*   - XHDCP1X_HANDLER_RPTR_TRIGDWNSTRMAUTH
*   - RepeaterDownstreamAuthCallback has to be defined by the user in order
*     to ensure that the downstream is ready to begin afer 1st part of
*     authentication is completed on the RX side. The user must ensure that
*     TX is not in physical-layer-down state and consequently post the
*     EVENT_AUTHENTICATE to the TX interface in order to begin the
*     authentication process for the downstream TX interface. Called by the
*     drivers.
* - XHdcp1xInst.Rx.AuthenticatedCallback
*   - XHDCP1X_HANDLER_AUTHENTICATED
*   - AuthenticatedCallback has to be defined by the user and is called
*     when the HDCP state machine has completed authentication.
* - XHdcp1xInst.Rx.UnauthenticatedCallback
*   - XHDCP1X_HANDLER_UNAUTHENTICATED
*   - UnauthenticatedCallback has to be defined by the user , to handle
*     repeater situations where the transmitter state machine goes to the
*     unauthenticate state.
* - XHdcp1xInst.Rx.TopologyUpdateCallback
*   - XHDCP1X_HANDLER_TOPOLOGY_UPDATE
*   - The TopologyUpdateCallback should be set by the user to handle changes
*     in downstream topology. This is currently not supprted for hdcp 1.4.
* - XHdcp1xInst.Rx.EncryptionUpdateCallback
*   - XHDCP1X_HANDLER_ENCRYPTION_UPDATE
*   - The EncryptionUpdateCallback needs to be set by the user and is called
*     if the upstream device stops sending encrypted content to the receiver
*     after authentication or vice-versa.
*
* -- -- -- -- -- -- -- -- -- -- -- -- --
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who    Date     Changes
* ----- ------ -------- --------------------------------------------------
* 1.00  fidus  07/16/15 Initial release.
* 2.00  als    09/30/15 Added EffectiveAddr argument to XHdcp1x_CfgInitialize.
* 2.10  MG     01/18/16 Added function XHdcp1x_IsEnabled.
* 2.20  MG     01/20/16 Added function XHdcp1x_GetHdcpCallback.
* 2.30  MG     02/25/16 Added function XHdcp1x_SetCallback and
*                       AuthenticatedCallback.
* 2.40  MG     01/29/16 Removed function XHdcp1x_GetHdcpCallback and added
*                       function XHdcp1x_ProcessAKsv
* 3.0   yas    02/13/16 Upgraded to support HDCP Repeater functionality.
*                       Added constants:
*                       XHDCP1X_RPTR_MAX_CASCADE and
*                       XHDCP1X_RPTR_MAX_DEVS_COUNT.
*                       Added enumeration type:
*                       XHdcp1x_RepeaterStateMachineHandlerType.
*                       Added typedef data type XHdcp1x_Ksv.
*                       Added structure XHdcp1x_RepeaterExchange.
*                       Updated core structure XHdcp1x_Tx, XHdcp1x_Rx and
*                       XHdcp1x for Repeater support.
*                       Added following functions:
*                       XHdcp1x_DownstreamReady, XHdcp1x_GetRepeaterInfo,
*                       XHdcp1x_SetCallBack, XHdcp1x_ReadDownstream.
* 4.0   yas    06/15/16 Added support for Cipher Blank Value and select.
*                       Extended support for Repeater functionality.
* 4.0   yas    08/16/16 Used UINTPTR instead of u32 for BaseAddress
*                       XHdcp1x_CfgInitialize
* 4.1   yas    11/10/16 Added function XHdcp1x_SetHdmiMode.
* 4.1   yas    08/03/17 Added flag IsAuthReqPending to the XHdcp1x_Tx data
*                       structure to track any pending authentication requests.
* </pre>
*
******************************************************************************/

#ifndef XHDCP1X_H
/**< Prevent circular inclusions by using protection macros */
#define XHDCP1X_H

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/

#include "xil_types.h"
#include "xhdcp1x_hw.h"
#include "xstatus.h"
#include "xtmrctr.h"

/************************** Constant Definitions *****************************/
#define XHDCP1X_KSV_SIZE	5 /**< Size of each hdcp 1.4 Public Key
				    *  in bytes */
#define XHDCP1X_RPTR_MAX_CASCADE	4	/**< Maximum depth that the
						  *  Repeater can support on
						  *  the downstream
						  *  interface */
#define XHDCP1X_RPTR_MAX_DEVS_COUNT	32	/**< Maximum devices that can
						  *  be cascaded to the
						  *  Repeater */

#define XHdcp1x_SetCallBack  XHdcp1x_SetCallback	/**< Alternative name
							  *  for the function
							  *  to set callback
							  *  for HDCP
							  *  functions */

#define UNUSED(x)  ((void)x) /**< Used to remove warnings for
			       *  unused variables. */
#define XHDCP1X_ADDITIONAL_DEBUG  0 /**< Adds extra functions for
				      *  additional debugging. */

/**************************** Type Definitions *******************************/

/**
 * This enumerates the State Types for HDCP Receiver state machine.
 */
enum XHdcp1x_Rx_StateType {
	XHDCP1X_RX_STATE_DISABLED,
	XHDCP1X_RX_STATE_UNAUTHENTICATED,
	XHDCP1X_RX_STATE_COMPUTATIONS,
	XHDCP1X_RX_STATE_WAITFORDOWNSTREAM,
	XHDCP1X_RX_STATE_ASSEMBLEKSVLIST,
	XHDCP1X_RX_STATE_AUTHENTICATED,
	XHDCP1X_RX_STATE_LINKINTEGRITYFAILED,
	XHDCP1X_RX_STATE_PHYDOWN,
};

/**
 * This enumerates the Event Types for HDCP Transmitter state machine.
 */
enum XHdcp1x_Tx_StateType {
	XHDCP1X_TX_STATE_DISABLED,
	XHDCP1X_TX_STATE_DETERMINERXCAPABLE,
	XHDCP1X_TX_STATE_EXCHANGEKSVS,
	XHDCP1X_TX_STATE_COMPUTATIONS,
	XHDCP1X_TX_STATE_VALIDATERX,
	XHDCP1X_TX_STATE_AUTHENTICATED,
	XHDCP1X_TX_STATE_LINKINTEGRITYCHECK,
	XHDCP1X_TX_STATE_TESTFORREPEATER,
	XHDCP1X_TX_STATE_WAITFORREADY,
	XHDCP1X_TX_STATE_READKSVLIST,
	XHDCP1X_TX_STATE_UNAUTHENTICATED,
	XHDCP1X_TX_STATE_PHYDOWN,
};

/**
 * These constants are used to identify callback functions.
 */
typedef enum
{
	XHDCP1X_HANDLER_UNDEFINED,
	XHDCP1X_HANDLER_DDC_SETREGADDR,
	XHDCP1X_HANDLER_DDC_SETREGDATA,
	XHDCP1X_HANDLER_DDC_GETREGDATA,
	XHDCP1X_HANDLER_DDC_WRITE,
	XHDCP1X_HANDLER_DDC_READ,
	XHDCP1X_HANDLER_AUTHENTICATED,
	XHDCP1X_HANDLER_UNAUTHENTICATED,
	XHDCP1X_HANDLER_RPTR_RPTREXCHANGE,
	XHDCP1X_HANDLER_RPTR_TRIGDWNSTRMAUTH,
	XHDCP1X_HANDLER_TOPOLOGY_UPDATE,
	XHDCP1X_HANDLER_ENCRYPTION_UPDATE,
	XHDCP1X_HANDLER_INVALID
} XHdcp1x_HandlerType;

/**
 * This typedef defines the callback interface that is to be used for
 * interrupts within this driver
 */
typedef void (*XHdcp1x_Callback)(void *CallbackRef);

/**
 * This typedef defines the function interface that is to be used for debug
 * print statements within this driver
 */
typedef void (*XHdcp1x_Printf)(const char *fmt, ...);

/**
 * This typedef defines the function interface that is to be used for debug
 * log message statements within this driver
 */
typedef void (*XHdcp1x_LogMsg)(const char *fmt, ...);

/**
 * This enumerates the call back for the HDCP Repeater Tx state machine
 */
typedef enum {
	XHDCP1X_RPTR_HDLR_REPEATER_EXCHANGE =
			XHDCP1X_HANDLER_RPTR_RPTREXCHANGE,
	XHDCP1X_RPTR_HDLR_TRIG_DOWNSTREAM_AUTH =
			XHDCP1X_HANDLER_RPTR_TRIGDWNSTRMAUTH,
} XHdcp1x_RepeaterStateMachineHandlerType;

/**
* These constants are used to identify fields inside the topology structure
*/
typedef enum {
	XHDCP1X_TOPOLOGY_DEPTH,
	XHDCP1X_TOPOLOGY_DEVICECNT,
	XHDCP1X_TOPOLOGY_MAXDEVSEXCEEDED,
	XHDCP1X_TOPOLOGY_MAXCASCADEEXCEEDED,
	XHDCP1X_TOPOLOGY_HDCP20REPEATERDOWNSTREAM,
	XHDCP1X_TOPOLOGY_HDCP1DEVICEDOWNSTREAM,
	XHDCP1X_TOPOLOGY_INVALID
} XHdcp1x_TopologyField;

/**
* Callback type used for calling DDC read and write functions.
*
* @param  DeviceAddress is the (i2c) device address of the HDCP port.
* @param  ByteCount is the amount of data bytes in the buffer to read or write.
* @param  BufferPtr is a pointer to a buffer that is used
*         for reading or writing.
* @param  Stop is a flag to control if a stop token is set or not.
* @param  RefPtr is a callback reference passed in by the
*         upper layer when setting the DDC reading and writing functions,
*         and passed back to the upper layer when the callback is invoked.
*
* @return
*         - XST_SUCCESS The read action was successful.
*         - XST_FAILURE The read action failed.
*
* @note   None.
*
*/
typedef int (*XHdcp1x_RunDdcHandler)(u8 DeviceAddress, u16 ByteCount,
		u8* BufferPtr, u8 Stop, void *RefPtr);

/**
 * This typedef defines the function interface that is to be used for setting
 * the DDC handler for HDMI implementation of HDCP functionality over HDMI
 * within this driver
 */
typedef void (*XHdcp1x_SetDdcHandler)(void *HandlerRef, u32 Data);

/**
 * This typedef defines the function interface that is to be used to get
 * the DDC handler for implementation of HDCP functionality over HDMI
 * within this driver
 */
typedef u32 (*XHdcp1x_GetDdcHandler)(void *HandlerRef);

/**
* This typedef contains configuration information for the HDCP core.
*/
typedef struct {
	u16 DeviceId;		/**< Device instance ID. */
	UINTPTR BaseAddress;	/**< The base address of the core  */
	u32 SysFrequency;	/**< The main clock frequency of the core */
	u16 IsRx;		/**< Flag indicating the core direction */
	u16 IsHDMI;		/**< Flag indicating if the core is meant to
				  *  work with HDMI. */
} XHdcp1x_Config;

/**
* This typedef defines the statistics collected by a cipher instance
*/
typedef struct {
	u32 IntCount;		/**< The number of interrupts detected */
} XHdcp1x_CipherStats;

/**
* This typedef contains an instance of the HDCP cipher core
*/
typedef struct {
	XHdcp1x_Callback LinkFailCallback;	/**< Link fail callback */
	void *LinkFailRef;			/**< Link fail reference */
	u32 IsLinkFailCallbackSet;		/**< Link fail config flag */
	XHdcp1x_Callback RiUpdateCallback;	/**< Ri update callback */
	void *RiUpdateRef;			/**< Ri update reference */
	u32 IsRiUpdateCallbackSet;		/**< Ri update config flag */
	XHdcp1x_CipherStats Stats;		/**< Cipher statistics */
} XHdcp1x_Cipher;

/**
 * This typedef defines the statistics collected by a port instance
 */
typedef struct {
	u32 IntCount;		/**< The number of interrupts detected */
} XHdcp1x_PortStats;

/**
 * Forward declaration of a structure defined within xhdcxp1x_port.h
 */
struct XHdcp1x_PortPhyIfAdaptorS;

/**
 * This typedef defines a memory to store a Key Selection Vector (KSV)
 */
typedef u64 XHdcp1x_Ksv;

/**
 * This typedef contains an instance of the HDCP Repeater values to
 * exchanged between HDCP Tx and HDCP Rx
 */
typedef struct {
	u32 V[5];	/**< 20 bytes value of SHA 1 hash, V'H0, V'H1, V'H2 ,
			  *  V'H3 ,V'H4 read from the downstream Repeater*/
	XHdcp1x_Ksv KsvList[32];	/**< An array of 32 elements each
					  *  of 64 bits to store the KSVs
					  *  for the KSV FIFO*/
	u8 Depth;	/**< Depth of the Repeater's downstream topology*/
	u8 DeviceCount;		/**< Number of downstream devices attached
				  *  to the Repeater*/
} XHdcp1x_RepeaterExchange;

/**
 * This typedef contains an instance of the HDCP port
 */
typedef struct XHdcp1x_PortStruct {
	const struct XHdcp1x_PortPhyIfAdaptorS *Adaptor; /**< Port adaptor */
	void *PhyIfPtr;		/**< The port's physical interface */
	XHdcp1x_Callback AuthCallback;	/**< (Re)Authentication callback */
	void *AuthRef;		/**< (Re)Authentication reference */
	u32 IsAuthCallbackSet;		/**< (Re)Authentication config flag */
	XHdcp1x_PortStats Stats;	/**< Port statistics */
} XHdcp1x_Port;

/**
 * This typedef defines the statistics collected transmit port instance
 */
typedef struct {
	u32 AuthFailed;		/**< Num of failed authentication attempts */
	u32 AuthPassed;		/**< Num of passed authentication attempts */
	u32 ReauthRequested;	/**< Num of rxd re-authentication requests */
	u32 ReadFailures;	/**< Num of remote read failures */
	u32 LinkCheckPassed;	/**< Num of link verifications that passed */
	u32 LinkCheckFailed;	/**< Num of link verifications that failed */
} XHdcp1x_TxStats;

/**
 * This typedef defines the statistics collected receive port instance
 */
typedef struct {
	u32 AuthAttempts;	/**< Num of rxd authentication requests */
	u32 LinkFailures;	/**< Num of link verifications that failed */
	u32 RiUpdates;		/**< Num of Ri updates performed */
} XHdcp1x_RxStats;

/**
 * This typedef is used to keep a tab on the changes in the encryption
 * status of the HDCP receiver, depending on whether the incoming
 * data non the receiver is HDCP protected or not.
 */
typedef struct {
	u8 PreviousState;	/**< Previous state of encryption */
	u8 CurrentState;	/**< Current state of encryption */
} XHdcp1x_RxEncyptionWatch;

/**
 * This typedef contains the transmit HDCP interface
 */
typedef struct {
	u32 CurrentState;	/**< The interface's current state */
	u32 PreviousState;	/**< The interface's previous state */
	u64 StateHelper;	/**< The interface's state helper */
	u16 Flags;		/**< The interface flags */
	u16 PendingEvents;	/**< The bit map of pending events */
	u64 EncryptionMap;	/**< The configured encryption map */
	u16 WaitForReadyPollCntFlag; /**< Count of the times we have
				  * polled the BCaps every 100ms interval. */
	XHdcp1x_TxStats Stats;	/**< The interface's statistics */
	u8 TxIsHdmi;	/**< Flag determines if TX is HDMI or DVI */
	u8 IsAuthReqPending; /**< Flag to track if a auth request is alive */
	XHdcp1x_Callback AuthenticatedCallback;	/**< Authentication callback */
	void *AuthenticatedCallbackRef;	/**< Authentication reference */
	u32 IsAuthenticatedCallbackSet;	/**< Authentication config flag */
	XHdcp1x_RunDdcHandler DdcRead;	/**< Function pointer for reading DDC*/
	void *DdcReadRef;	/**< Reference pointer set with
				  *  XHdcp1x_SetCallback function. */
	u8 IsDdcReadSet;	/**< Set if DdcRead handler is defined. */
	XHdcp1x_RunDdcHandler DdcWrite;	/**< Function pointer for writing DDC*/
	void *DdcWriteRef;	/**< Reference pointer set with
				  *  XHdcp1x_SetCallback function. */
	u8 IsDdcWriteSet;	/**< Set if DdcWrite handler is defined. */
	XHdcp1x_Callback RepeaterExchangeCallback; /**< (Repeater)Exchange
						     *  Repeater Values
						     *  callback */
	void *RepeaterExchangeRef;	/**< (Repeater)Exchange Repeater
					  *  Values reference */
	u32 IsRepeaterExchangeCallbackSet;	/**< (Repeater)Check to
						  *  determine if Exchange
						  *  Repeater Values
						  *  callback is set flag */
	XHdcp1x_Callback UnauthenticatedCallback; /**< Unauthenticated
						    * callback */
	void *UnauthenticatedCallbackRef;	/**< Unauthenticated
						  *  reference */
	u32 IsUnauthenticatedCallbackSet;	/**< Unauthenticated config
						  *  flag */
	u16 DownstreamReady;/**< The downstream interface's status flag */
} XHdcp1x_Tx;

/**
 * This typedef contains the receive HDCP interface
 */
typedef struct {
	u32 CurrentState;		/**< The interface's current state */
	u32 PreviousState;		/**< The interface's previous state */
	u16 Flags;			/**< The interface flags */
	u16 PendingEvents;		/**< The bit map of pending events */
	XHdcp1x_RxStats Stats;		/**< The interface's statistics */
	XHdcp1x_RxEncyptionWatch XORState;	/**< The interface's encryption
						  *  state */
	XHdcp1x_SetDdcHandler DdcSetAddressCallback; /**< Function pointer for
						       *  setting DDC register
						       *  address */
	void *DdcSetAddressCallbackRef;	/**< To be passed to callback
					  *  function */
	u8 IsDdcSetAddressCallbackSet;	/**< This flag is set true when the
					  *  callback has been registered */
	XHdcp1x_SetDdcHandler DdcSetDataCallback; /**< Function pointer for
						    *  setting DDC register
						    *  data */
	void *DdcSetDataCallbackRef;	/**< To be passed to callback
					  *  function */
	u8 IsDdcSetDataCallbackSet;	/**< This flag is set true when the
					  *  callback has been registered */
	XHdcp1x_GetDdcHandler DdcGetDataCallback; /**< Function pointer for
						    *  getting DDC register
						    *  data */
	void *DdcGetDataCallbackRef;	/**< To be passed to callback
					  *  function */
	u8 IsDdcGetDataCallbackSet;	/**< This flag is set true when the
					  *  callback has been registered */
	XHdcp1x_Callback RepeaterDownstreamAuthCallback; /**< (Repeater)Post
							   *  authenticate to
							   *  downstream,
							   *  second part of
							   *  authentication
							   *  start callback*/
	void *RepeaterDownstreamAuthRef; /**< (Repeater)Post authenticate to
					   *  downstream, second part of
					   *  authentication start reference */
	u32 IsRepeaterDownstreamAuthCallbackSet; /**< (Repeater)Is "Post
						   *  authenticate to
						   *  downstream to trigger
						   *  second part of
						   *  authentication" callback
						   *  set flag*/
	XHdcp1x_Callback AuthenticatedCallback; /**< Unauthenticated callback*/
	void *AuthenticatedCallbackRef;	/**< Unauthenticated reference */
	u32 IsAuthenticatedCallbackSet;	/**< Unauthenticated config flag */
	XHdcp1x_Callback UnauthenticatedCallback; /**< Unauthenticated
						    *  callback */
	void *UnauthenticatedCallbackRef;	/**< Unauthenticated
						  *  reference */
	u32 IsUnauthenticatedCallbackSet;	/**< Unauthenticated config
						  *  flag */
	XHdcp1x_Callback TopologyUpdateCallback; /**< Topology Update
						   *  callback */
	void *TopologyUpdateCallbackRef;	/**< Topology Update
						  *  reference */
	u32 IsTopologyUpdateCallbackSet;	/**< Topology Update config
						  *  flag */
	XHdcp1x_Callback EncryptionUpdateCallback; /**< Encryption Update
						     *  callback */
	void *EncryptionUpdateCallbackRef;	/**< Encryption Update
						  *  reference */
	u32 IsEncryptionUpdateCallbackSet;	/**< Encryption Update
						  *  config flag */
} XHdcp1x_Rx;

/**
 * This typedef defines the function interface that is to be used
 * for starting a one shot timer on behalf of an HDCP interface within
 * the underlying platform
 */
typedef int (*XHdcp1x_TimerStart)(void *InstancePtr, u16 TmoInMs);

/**
 * This typedef defines the function interface that is to be used
 * for stopping a timer on behalf of an HDCP interface
 */
typedef int (*XHdcp1x_TimerStop)(void *InstancePtr);

/**
 * This typedef defines the function interface that is to be used for
 * performing a busy delay on behalf of an HDCP interface
 */
typedef int (*XHdcp1x_TimerDelay)(void *InstancePtr, u16 DelayInMs);

/**
 * This typedef contains an instance of an HDCP interface
 */
typedef struct {
	XHdcp1x_Config Config;	/**< The core config */
	XHdcp1x_Cipher Cipher;	/**< The interface's cipher */
	XHdcp1x_Port Port;	/**< The interface's port */
	union {
		XHdcp1x_Tx Tx;	/**< The transmit interface elements */
		XHdcp1x_Rx Rx;	/**< The receive interface elements */
	};
	u32 IsReady;		/**< The ready flag */
	u32 IsRepeater;		/**< The IsRepeater flag determines if the
				  *  HDCP is part of a Repeater system
				  *  or a non-repeater interface */
	XHdcp1x_RepeaterExchange RepeaterValues; /**< The Repeater value to
						   *  be exchanged between
						   *  Tx and Rx */
	void *Hdcp1xRef;	/**< A void reference pointer for
				  *  association of a external core
				  *  in our case a timer with the
				  *  HDCP instance */
	XHdcp1x_TimerStart XHdcp1xTimerStart; 	/**< Instance of function
						*  interface used for
						*  starting a timer on behalf
						*  of an HDCP interface*/
	XHdcp1x_TimerStop XHdcp1xTimerStop; 	/**< Instance of function
						*  interface used for
						*  stopping a timer on behalf
						*  of an HDCP interface*/
	XHdcp1x_TimerDelay XHdcp1xTimerDelay;	/**< Instance of function
						*  interface used for
						*  performing a busy delay on
						*  behalf of an HDCP
						*  interface*/
} XHdcp1x;

/**
 * This typedef defines the function interface that is to be used
 * for checking a specific KSV against the platforms revocation list
 */
typedef int (*XHdcp1x_KsvRevokeCheck)(const XHdcp1x *InstancePtr, u64 Ksv);

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

XHdcp1x_Config *XHdcp1x_LookupConfig(u16 DeviceId);

int XHdcp1x_CfgInitialize(XHdcp1x *InstancePtr, const XHdcp1x_Config *CfgPtr,
		void *PhyIfPtr, UINTPTR EffectiveAddr);

int XHdcp1x_SelfTest(XHdcp1x *InstancePtr);

int XHdcp1x_Poll(XHdcp1x *InstancePtr);

int XHdcp1x_DownstreamReady(XHdcp1x *InstancePtr);
int XHdcp1x_GetRepeaterInfo(XHdcp1x *InstancePtr,
		XHdcp1x_RepeaterExchange *RepeaterInfoPtr);
int XHdcp1x_SetRepeater(XHdcp1x *InstancePtr, u8 State);

int XHdcp1x_Reset(XHdcp1x *InstancePtr);
int XHdcp1x_Enable(XHdcp1x *InstancePtr);
int XHdcp1x_Disable(XHdcp1x *InstancePtr);

int XHdcp1x_SetPhysicalState(XHdcp1x *InstancePtr, int IsUp);
int XHdcp1x_SetLaneCount(XHdcp1x *InstancePtr, int LaneCount);

int XHdcp1x_Authenticate(XHdcp1x *InstancePtr);
int XHdcp1x_ReadDownstream(XHdcp1x *InstancePtr);
int XHdcp1x_IsInProgress(const XHdcp1x *InstancePtr);
int XHdcp1x_IsAuthenticated(const XHdcp1x *InstancePtr);
int XHdcp1x_IsInComputations(const XHdcp1x *InstancePtr);
int XHdcp1x_IsInWaitforready(const XHdcp1x *InstancePtr);
int XHdcp1x_IsDwnstrmCapable(const XHdcp1x *InstancePtr);
int XHdcp1x_IsEnabled(const XHdcp1x *InstancePtr);

u64 XHdcp1x_GetEncryption(const XHdcp1x *InstancePtr);
int XHdcp1x_IsEncrypted(const XHdcp1x *InstancePtr);
int XHdcp1x_EnableEncryption(XHdcp1x *InstancePtr, u64 StreamMap);
int XHdcp1x_DisableEncryption(XHdcp1x *InstancePtr, u64 StreamMap);

int XHdcp1x_SetKeySelect(XHdcp1x *InstancePtr, u8 KeySelect);

void XHdcp1x_HandleTimeout(void *InstancePtr);

int XHdcp1x_SetCallback(XHdcp1x *InstancePtr,
		XHdcp1x_HandlerType HandlerType,
		void *CallbackFunc, void *CallbackRef);
void XHdcp1x_CipherIntrHandler(void *InstancePtr);
void XHdcp1x_PortIntrHandler(void *InstancePtr, u32 IntCause);

void XHdcp1x_SetDebugPrintf(XHdcp1x_Printf PrintfFunc);
void XHdcp1x_SetDebugLogMsg(XHdcp1x_LogMsg LogFunc);

void XHdcp1x_SetKsvRevokeCheck(XHdcp1x_KsvRevokeCheck RevokeCheckFunc);
void XHdcp1x_SetTimerStart(XHdcp1x *InstancePtr,
		XHdcp1x_TimerStart TimerStartFunc);
void XHdcp1x_SetTimerStop(XHdcp1x *InstancePtr,
		XHdcp1x_TimerStop TimerStopFunc);
void XHdcp1x_SetTimerDelay(XHdcp1x *InstancePtr,
		XHdcp1x_TimerDelay TimerDelayFunc);

u32 XHdcp1x_GetDriverVersion(void);
u32 XHdcp1x_GetVersion(const XHdcp1x *InstancePtr);
void XHdcp1x_Info(const XHdcp1x *InstancePtr);
void XHdcp1x_ProcessAKsv(XHdcp1x *InstancePtr);

void *XHdcp1x_GetTopology(XHdcp1x *InstancePtr);
void XHdcp1x_DisableBlank(XHdcp1x *InstancePtr);
void XHdcp1x_EnableBlank(XHdcp1x *InstancePtr);
void XHdcp1x_SetTopologyField(XHdcp1x *InstancePtr,
		XHdcp1x_TopologyField Field, u8 Value);
u32 XHdcp1x_GetTopologyField(XHdcp1x *InstancePtr,
		XHdcp1x_TopologyField Field);
u8 *XHdcp1x_GetTopologyKSVList(XHdcp1x *InstancePtr);
u8 *XHdcp1x_GetTopologyBKSV(XHdcp1x *InstancePtr);
int XHdcp1x_IsRepeater(XHdcp1x *InstancePtr);

void XHdcp1x_SetTopology(XHdcp1x *InstancePtr,
		const XHdcp1x_RepeaterExchange *TopologyPtr);
void XHdcp1x_SetTopologyKSVList(XHdcp1x *InstancePtr, u8 *ListPtr,
		u32 ListSize);
void XHdcp1x_SetTopologyUpdate(XHdcp1x *InstancePtr);
void XHdcp1x_SetHdmiMode(XHdcp1x *InstancePtr, u8 Value);

#ifdef __cplusplus
}
#endif

#endif /* XHDCP1X_H */
/** @} */
