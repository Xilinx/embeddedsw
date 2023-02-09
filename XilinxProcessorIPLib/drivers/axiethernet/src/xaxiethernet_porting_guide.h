/******************************************************************************
* Copyright (C) 2010 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2022 - 2023 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xaxiethernet_porting_guide.h
* @addtogroup axiethernet Overview
* @{
*
* This is a guide on how to move from using the ll temac driver to use the
* xaxiethernet driver.
*
* The AXI Ethernet IP is based on the XPS_LL TEMAC IP.
* There are few changes in the IP and some of them are listed below
*	- The Interface to access the IP is now AXI  instead of the PLBV46
*	- AXI4 streaming interfaces is used instead of LL (Local Link)
*	- All indirect accesses to Ethernet core registers have been removed.
*	- The AxiEthernet reset line is connected to the reset line of the
*	  device connected to the AXI4-Stream interface. Hence any reset
*	  of the connected device would reset AxiEthernet.
*
* Please read the HW Device specification of the AXI Ethernet IP for further
* information.
*
*
* The AXI Ethernet can be used in a DMA mode using the AXI DMA or
* used in a FIFO mode using the AXI Streaming FIFO.
*
* There is a new driver for AXI DMA which is used by the AXi Ethernet. Please
* refer the xaxidma_porting_guide.h provided as a part of the AXI DMA driver to
* see the differences from the LL DMA driver.
*
* The LL FIFO driver is a common driver for the LL FIFO and the
* AXI Streaming FIFO.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -------------------------------------------------------
* 1.00a ASA   07/18/10 First release
*
* </pre>
*
* <b>Overview</b>
*
* The API for xaxiethernet driver are similar to xlltemac driver. The prefix for the
* API functions and structures is XAxiEthernet_ for the xaxiethernet driver.
*
* The prefix for all hash-defines (option masks or generic hash-defined constants)
* start with XAE_.
*
* The Axi Ethernet API "XAxiEthernet_Reset" is different from the corresponding
* LLTEMAC API "XLlTemac_Reset". The AxiEthernet version does not do a soft
* reset of the AxiEthernet hardware. Since AxiEthernet hardware could only be
* reset through the device connected to the AxiEthernet AXI4-Stream interface,
* the user must ensure that AxiEthernet hardware initialization happens after
* the initialization of the other device.
*
* These are the classification of the APIs
* - APIs that only have prefix changes
* - APIs that have been renamed
* - APIs that are new

*
* Note that data structures have different prefix of XAxiEthernet_. Those API
* functions, that have data structures with prefix change, are considered as
* prefix change.
*
* <b>API Functions That Only Have Prefix Changes</b>
*
* Most of the functions have the prefix change and are given below
* <pre>
*         xlltemac driver		|	xaxiethernet driver
* ---------------------------------------------------------------------------------
*    XLlTemac_IsStarted(...)		|	XAxiEthernet_IsStarted(...)
*    XLlTemac_IsDma(...)		|	XAxiEthernet_IsDma(...)
*    XLlTemac_IsFifo(...)		|	XAxiEthernet_IsFifo(...)
*    XLlTemac_LlDevBaseAddress(...)	|	XAxiEthernet_AxiDevBaseAddress(...)
*    XLlTemac_IsRecvFrameDropped(...)	|	XAxiEthernet_IsRecvFrameDropped(...)
*    XLlTemac_GetPhysicalInterface(...)	|	XAxiEthernet_GetPhysicalInterface(...)
*    XLlTemac_IntEnable(...)		|	XAxiEthernet_IntEnable(...)
*    XLlTemac_IntDisable(...)		|	XAxiEthernet_IntDisable(...)
*    XLlTemac_IntPending(...)		|	XAxiEthernet_IntPending(...)
*    XLlTemac_IntClear(...)		|	XAxiEthernet_IntClear(...)
*    XLlTemac_IsExtFuncCap(...)		|	XAxiEthernet_IsExtFuncCap(...)
*    XLlTemac_IsExtMcastEnable(...)	|	XAxiEthernet_IsExtMcastEnable(...)
*    XLlTemac_IsExtMcast(...)		|	XAxiEthernet_IsExtMcast(...)
*    XLlTemac_IsTxVlanStrp(...)		|	XAxiEthernet_IsTxVlanStrp(...)
*    XLlTemac_IsRxVlanStrp(...)		|	XAxiEthernet_IsRxVlanStrp(...)
*    XLlTemac_IsTxVlanTran(...)		|	XAxiEthernet_IsTxVlanTran(...)
*    XLlTemac_IsRxVlanTran(...)		|	XAxiEthernet_IsRxVlanTran(...)
*    XLlTemac_IsTxVlanTag(...)		|	XAxiEthernet_IsTxVlanTag(...)
*    XLlTemac_IsRxVlanTag(...)		|	XAxiEthernet_IsRxVlanTag(...)
*    XLlTemac_SetOptions (...)		|	XAxiEthernet_SetOptions(...)
*    XLlTemac_ClearOptions(...)		|	XAxiEthernet_ClearOptions(...)
*    XLlTemac_GetOptions(...)		|	XAxiEthernet_GetOptions(...)
*    XLlTemac_SetMacAddress(...)	|	XAxiEthernet_SetMacAddress(...)
*    XLlTemac_GetMacAddress(...)	|	XAxiEthernet_GetMacAddress(...)
*    XLlTemac_SetMacPauseAddress(...)	|	XAxiEthernet_SetMacPauseAddress(...)
*    XLlTemac_GetMacPauseAddress(...)	|	XAxiEthernet_GetMacPauseAddress(...)
*    XLlTemac_SendPausePacket(...)	|	XAxiEthernet_SendPausePacket(...)
*    XLlTemac_GetSgmiiStatus(...)	|	XAxiEthernet_GetSgmiiStatus(...)
*    XLlTemac_GetRgmiiStatus(...)	|	XAxiEthernet_GetRgmiiStatus(...)
*... XLlTemac_GetOperatingSpeed(...)	|	XAxiEthernet_GetOperatingSpeed(...)
*    XLlTemac_SetOperatingSpeed(...)	|	XAxiEthernet_SetOperatingSpeed(...)
*    XLlTemac_PhySetMdioDivisor(...)	|	XAxiEthernet_PhySetMdioDivisor(...)
*    XLlTemac_PhyRead(...)		|	XAxiEthernet_PhyRead(...)
*    XLlTemac_PhyWrite(...)		|	XAxiEthernet_PhyWrite(...)
*    XLlTemac_MulticastAdd(...)		|	XAxiEthernet_MulticastAdd(...)
*    XLlTemac_MulticastGet(...)		|	XAxiEthernet_MulticastGet(...)
*    XLlTemac_MulticastClear(...)	|	XAxiEthernet_MulticastClear(...)
*    XLlTemac_SetTpid(...)		|	XAxiEthernet_SetTpid(...)
*    XLlTemac_ClearTpid(...)		|	XAxiEthernet_ClearTpid(...)
*    XLlTemac_GetTpid(...)		|	XAxiEthernet_GetTpid(...)
*    XLlTemac_SetVTagMode(...)		|	XAxiEthernet_SetVTagMode(...)
*    XLlTemac_GetVTagMode(...)		|	XAxiEthernet_GetVTagMode(...)
*    XLlTemac_SetVStripMode(...)	|	XAxiEthernet_SetVStripMode(...)
*    XLlTemac_GetVStripMode(...)	|	XAxiEthernet_GetVStripMode(...)
*    XLlTemac_SetVTagValue(...)		|	XAxiEthernet_SetVTagValue(...)
*    XLlTemac_GetVTagValue(...)		|	XAxiEthernet_GetVTagValue(...)
*    XLlTemac_SetVidTable(...)		|	XAxiEthernet_SetVidTable(...)
*    XLlTemac_GetVidTable(...)		|	XAxiEthernet_GetVidTable(...)
*    XLlTemac_AddExtMulticastGroup(...)	|	XAxiEthernet_AddExtMulticastGroup(...)
*    XLlTemac_ClearExtMulticastGroup(..)|	XAxiEthernet_ClearExtMulticastGroup(...)
*    XLlTemac_GetExtMulticastGroup(...)	|	XAxiEthernet_GetExtMulticastGroup(...)
*    XLlTemac_DumpExtMulticastGroup(..)	|	XAxiEthernet_DumpExtMulticastGroup(...)

*</pre>
*
* <b>API Function names that have changed </b>
*
* <pre>
*         xlltemac driver		|	xaaxiethernet driver
* -----------------------------------------------------------------------
* XLlTemac_IsRxCsum(...)		|	XAxiEthernet_IsRxPartialCsum(...)
* XLlTemac_IsTxCsum(...)		|	XAxiEthernet_IsTxPartialCsum(...)
* XLlTemac_Status(...)			|	XAxiEthernet_GetIntStatus
* </pre>
*
*
* <b>API Functions That Are New API Functions</b>
*
* - XAxiEthernet_GetTemacType(...)
* - XAxiEthernet_IsAvbConfigured(...)
* - XAxiEthernet_IsStatsConfigured(...)
* - XAxiEthernet_SetBadFrmRcvOption((...);
* - XAxiEthernet_ClearBadFrmRcvOption(...);
* - XAxiEthernet_DisableControlFrameLenCheck(...)
* - XAxiEthernet_EnableControlFrameLenCheck(...)
*
*
******************************************************************************/
/** @} */
