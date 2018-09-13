HDCP REPEATER PROFESSIONAL EXAMPLE DESIGN README
------------------------------------------------

CONTENTS OF THIS FILE
---------------------

 * Introduction
 * Features
 * System illustration
 * Required updates
 * Building the example design


1. INTRODUCTION
------------

This example application is built to demonstrate the functionality of Xilinx
HDMI HDCP 2.2 Professinal Repeater running on the same hardware with HDCP
configured as a Repeater.

For details, see xhdmi_example.c and xhdcp.c.


2. FEATURES
------------
* This application implements the HDCP Professional, Revision 2.2
  specification.
* The application implements the HDCP Professional Repeater built on a
  HDCP Repeater. In this implmentation, by default, HDMI TX is to transmit
  HDMI stream received from the SOURCE by the RX to the SINK.
* If Color Bar module is selected (via UART menu), a test pattern generator IP
  will generate video stream according to user selection (via UART), then HDMI
  TX will transmit HDMI stream to the downstream SINK.
* There is an additional option to not start the colorbar mode when HDCP
  Repeater is enabled. This will make the HDCP authentication process over the
  Repeater more robust.
* If the HDCP (1.4, 2.2 or both) is enabled by the hardware, the application
  will supports HDCP handling in both HDMI TX and RX. This will be implemented
  as a Repeater. The HDCP TX will be driven by the HDCP RX.

For details, please refer to PG235, PG236 and PG230.


3. SYSTEM ILLUSTRATION
---------------------

A system illustration is shown as below.

         ----------     -------------------------     --------
         |        |     |           |           |     |      |
         | SOURCE |---->|      RX   |     TX    |---->| SINK |
         |        |     | PROFESSIONAL REPEATER |     |      |
         ----------     -------------------------     --------


4. REQUIRED UPDATES
----------------------
In order to build the system the user is required to do a few extra updates to
the drivers in order to support the HDCP Professional requirements in the low
level drivers and to import the example design in SDK.

Updated Required to hdcp1x drivers
----------------------
Please make the following updates in the hdcp1x driver.
1. In file xhdcp1x.h,
	1.1. In the structure XHdcp1x, add the following variable and
	     it's description.
	     |   u8  IsProRepeater;  /**< The IsProRepeater flag determines if
	     |                         *  HDCP is part of a Professional Repeater
	     |                         *  system. */
	1.2. Add the declaration of the following function.
	     |   int XHdcp1x_SetProRepeater(XHdcp1x *InstancePtr, u8 State);
2. In file xhdcp1x_port_hdmi_rx.c,
	2.1. In the definition of the function,
	     |   static int XHdcp1x_PortHdmiRxEnable(XHdcp1x *InstancePtr)
	     update the following lines with a new check for the value of the
	     IsProRepeater flag. (Update marked as >>>>>)
	     |  /* Initialize the Bcaps register */
	     |   memset(Buf, 0, 4);
	     |   Buf[0] |= XHDCP1X_PORT_BIT_BCAPS_HDMI;
	     |   Buf[0] |= XHDCP1X_PORT_BIT_BCAPS_FAST_REAUTH;
	     |   if(InstancePtr->IsRepeater &&
	>>>>>|      !InstancePtr->IsProRepeater) {
	     |   	Buf[0] |= XHDCP1X_PORT_BIT_BCAPS_REPEATER;
	     |   }
3. In the file xhdcp1x_rx.c,
	3.1. In the definition of the function,
	     |   static void XHdcp1x_RxStartComputations(XHdcp1x *InstancePtr,
	     |   			XHdcp1x_StateType *NextStatePtr)
	     update the following lines with a new check for the value of the
	     IsProRepeater flag. (Update marked as >>>>>)
	     |   if (InstancePtr->IsRepeater &&
	>>>>>|       !InstancePtr->IsProRepeater) {
	     |   	Z = (u32)((Value | 0x00000100) & 0x000001FFul);
	     |   } else {
	     |   	Z = (u32)(Value & 0x000000FFul);
	     |   }
4. In the file xhdcp1x.c,
	4.1. Add the definition the following new function,
	     |   /*****************************************************************************/
	     |   /**
	     |     * This function sets the Professional Repeater functionality for an
	     |     * HDCP interface.
	     |     *
	     |     * @param	InstancePtr is the interface to disable.
	     |     * @param	State is either TRUE or FALSE
	     |     *
	     |     * @return
	     |     *		- XST_SUCCESS if successful.
	     |     *		- XST_FAILURE otherwise.
	     |     *
	     |     * @note		None.
	     |     *
	     |     ******************************************************************************/
	     |   int XHdcp1x_SetProRepeater(XHdcp1x *InstancePtr, u8 State)
	     |   {
	     |   	int Status = XST_SUCCESS;
	     |
	     |   	/* Verify argument. */
	     |   	Xil_AssertNonvoid(InstancePtr != NULL);
	     |
	     |   	InstancePtr->IsProRepeater = State;
	     |
	     |   	/* Update the Repeater bit in BCaps. */
	     |   	XHdcp1x_RxSetRepeaterBcaps(InstancePtr, !State);
	     |
	     |   	return (Status);
	     |   }

Updated Required to v_hdmirxss drivers
----------------------
1. In the xv_hdmirxss.h file,
	1.1. In the structure XV_HdmiRxSs add the following variable,
	     |   #if defined(XPAR_XHDCP_NUM_INSTANCES) || defined(XPAR_XHDCP22_RX_NUM_INSTANCES)
	     |     u8 IsHdcpProRepeater;
	     |   #endif
	1.2. Add the declaration of the function.
	     |  int XV_HdmiRxSs_HdcpSetProRepeater(XV_HdmiRxSs *InstancePtr, u8 Set);
	1.3. Add the declaration of the function.
	     |   int XV_HdmiRxSs_HdcpIsProRepeater(XV_HdmiRxSs *InstancePtr);
2. In the xv_hdmirxss_hdcp.c,
	2.1. Add the definition the following function,
	     |   #ifdef USE_HDCP_RX
	     |   /*****************************************************************************/
	     |   /**
	     |     *
	     |     * This function checks if the HDMI receiver is an HDCP professional repeater
	     |     * upstream interface for the active protocol.
	     |     *
	     |     * @param  InstancePtr is a pointer to the XV_HdmiRxSs instance.
	     |     *
	     |     * @return 'IsHdcpProRepeater' flag.
	     |     *
	     |     * @note   None.
	     |     *
	     |     ******************************************************************************/
	     |   int XV_HdmiRxSs_HdcpIsProRepeater(XV_HdmiRxSs *InstancePtr)
	     |   {
	     |     /* Verify argument. */
	     |     Xil_AssertNonvoid(InstancePtr != NULL);
	     |
	     |     return InstancePtr->IsHdcpProRepeater;
	     |   }
	     |   #endif
	2.2. Add the definition of the following function,
	     |   #ifdef USE_HDCP_RX
	     |   /*****************************************************************************/
	     |   /**
	     |     *
	     |     * This function enables the Professional Repeater functionality for
	     |     * the HDCP protocol.
	     |     *
	     |     * @param InstancePtr is a pointer to the XV_HdmiRxSs instance.
	     |     * @param Set is TRUE to enable and FALSE to disable repeater.
	     |     *
	     |     * @return
	     |     *  - XST_SUCCESS if action was successful
	     |     *  - XST_FAILURE if action was not successful
	     |     *
	     |     * @note   None.
	     |     *
	     |     ******************************************************************************/
	     |   int XV_HdmiRxSs_HdcpSetProRepeater(XV_HdmiRxSs *InstancePtr, u8 Set)
	     |   {
	     |     /* Verify argument. */
	     |     Xil_AssertNonvoid(InstancePtr != NULL);
	     |
	     |   #ifdef XPAR_XHDCP_NUM_INSTANCES
	     |     // HDCP 1.4
	     |     if (InstancePtr->Hdcp14Ptr) {
	     |       XHdcp1x_SetProRepeater(InstancePtr->Hdcp14Ptr, Set);
	     |     }
	     |   #endif
	     |
	     |   #ifdef XPAR_XHDCP22_RX_NUM_INSTANCES
	     |     // HDCP 2.2
	     |     if (InstancePtr->Hdcp22Ptr) {
	     |       /* Nothing to do. */
	     |     }
	     |   #endif
	     |
	     |     InstancePtr->IsHdcpProRepeater = Set;
	     |
	     |     return XST_SUCCESS;
	     |   }
	     |   #endif

5. BUILDING THE EXAMPLE DESIGN
----------------------
Before you can import this application as an example design in SDK, you need to
create a placeholder/pointer text file for it. To do this go to the following
directory.
<path_to_drivers>/v_hdmitxss/examples/xhdmi_example/dummy/
Here create a new file, "RepeaterPro_A53.txt" and copy the contents of this
file into it.

Next, in order to import the example design in SDK, please open the following
file in a text editor.
<path_to_drivers>/v_hdmitxss/data/dependencies.props

Next append the following line to the end of the file.
xhdmi_example/dummy/RepeaterPro_A53.txt=xhdmi_example/RepeaterProfessional/xhdmi_example.c,xhdmi_example/RepeaterProfessional/xhdmi_example.h,xhdmi_example/PassThrough/xhdmi_edid.h,xhdmi_example/PassThrough/xhdmi_edid.c,xhdmi_example/xhdmi_hdcp_keys_table.h,xhdmi_example/idt_8t49n24x.c,xhdmi_example/idt_8t49n24x.h,xhdmi_example/xvidframe_crc.c,xhdmi_example/xvidframe_crc.h,xhdmi_example/PassThrough/xhdmi_menu.c,xhdmi_example/PassThrough/xhdmi_menu.h,xhdmi_example/RepeaterProfessional/xhdcp.c,xhdmi_example/RepeaterProfessional/xhdcp.h,xhdmi_example/aes256.c,xhdmi_example/aes256.h,xhdmi_example/audiogen_drv.c,xhdmi_example/audiogen_drv.h,xhdmi_example/dp159.c,xhdmi_example/dp159.h,xhdmi_example/sha256.c,xhdmi_example/sha256.h,xhdmi_example/si5324drv.c,xhdmi_example/si5324drv.h,xhdmi_example/xhdmi_hdcp_keys.c,xhdmi_example/xhdmi_hdcp_keys.h,platforms/platform.c,platforms/platform.h,platforms/platform_config.h,platforms/zynq_us_a53_64/lscript.ld

Include the drivers in the Xilinx->Repositories in SDK GUI.

Build the board support package.

You should now see the option to build the RepeaterPro_A53 example design in
the selection box that appears.
