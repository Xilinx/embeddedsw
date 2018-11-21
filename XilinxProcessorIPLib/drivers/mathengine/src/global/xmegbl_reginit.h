/******************************************************************************
*
* Copyright (C) 2018 Xilinx, Inc.  All rights reserved.
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
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
* XILINX  BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
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
* @file xmegbl.h
* @{
*
* Header to include type definitions for the register bit field definitions 
* of Core, Memory, NoC and PL module registers.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who     Date     Changes
* ----- ------  -------- -----------------------------------------------------
* 1.0  Naresh  03/14/2018  Initial creation
* 1.1  Naresh  07/11/2018  Updated copyright info
* 1.2  Naresh  08/13/2018  Added Done attributes to CoreStsReg
* 1.3  Hyun    10/02/2018  Added performance counter registers
* 1.4  Hyun    10/02/2018  Added event registers
* 1.5  Hyun    10/03/2018  Added event port select registers
* 1.6  Hyun    10/12/2018  Added the column reset register
* </pre>
*
******************************************************************************/
#ifndef XMEGBL_REGINIT_H /* prevent circular inclusions */
#define XMEGBL_REGINIT_H /* by using protection macros */

/***************************** Include Files *********************************/
#include "xmegbl_defs.h"
#include "xmegbl.h"
#include "xmegbl_params.h"

/************************** Constant Definitions *****************************/
#define XMEGBL_TILE_PROGMEM_BASE        XMEGBL_CORE_PRGMEM
#define XMEGBL_TILE_DATAMEM_BASE        XMEGBL_MEM_DATMEM

/**************************** Type Definitions *******************************/
/**
 * This typedef contains the attributes for the register bit fields.
 */
typedef struct {
	u32 Lsb;                /**< Bit position of the register bit-field in the 32-bit value */
	u32 Mask;               /**< Bit mask of the register bit-field in the 32-bit value */
} XMeGbl_RegFldAttr;

/**
 * This typedef contains the attributes for the Release/Acquire register bit fields.
 */
typedef struct {
	u32 RelNvOff;                   /**< Release NV register offset */
	XMeGbl_RegFldAttr RelNv;        /**< Release NV filed attributes */
	u32 RelV0Off;                   /**< Release V0 register offset */
	XMeGbl_RegFldAttr RelV0;        /**< Release V0 field attributes */
	u32 RelV1Off;                   /**< Release V1 register offset */
	XMeGbl_RegFldAttr RelV1;        /**< Release V1 field attributes */
	u32 AcqNvOff;                   /**< Acquire NV register offset */
	XMeGbl_RegFldAttr AcqNv;        /**< Acquire NV field attributes */
	u32 AcqV0Off;                   /**< Acquire V0 register offset */
	XMeGbl_RegFldAttr AcqV0;        /**< Acquire V0 field attributes */
	u32 AcqV1Off;                   /**< Acquire V1 register offset */
	XMeGbl_RegFldAttr AcqV1;        /**< Acquire V1 field attributes */
} XMeGbl_RegLocks;

/**
 * This typedef contains the attributes for the Core control register.
 */
typedef struct {
	u32 RegOff;                     /**< Register offset */
	XMeGbl_RegFldAttr CtrlEn;       /**< Enable field attributes */
	XMeGbl_RegFldAttr CtrlRst;      /**< Reset field attributes */
} XMeGbl_RegCoreCtrl;

/**
 * This typedef contains the attributes for the Core status register.
 */
typedef struct {
	u32 RegOff;                     /**< Register offset */
        XMeGbl_RegFldAttr Done;         /**< Done value field attributes */
	XMeGbl_RegFldAttr Rst;          /**< Reset value field attributes */
	XMeGbl_RegFldAttr En;           /**< Enable value field attributes */
} XMeGbl_RegCoreSts;

/**
 * This typedef contains the attributes for Timer control and value regs.
 */
typedef struct {
	u32 CtrlOff;                    /**< Timer control Register offset */
	u32 LowOff;                     /**< Timer low value Register offset */
	u32 HighOff;                    /**< Timer high value Register offset */
} XMeGbl_RegTimer;

/**
 * This typedef contains the attributes for the PL upsizer config register.
 */
typedef struct {
	u32 RegOff;                                                     /**< Register offset */
	XMeGbl_RegFldAttr Wid3264[XMEGBL_TILE_PLIF_ME2PL_MAX_STRMS];    /**< 32/64 bit width bit field attributes */
	XMeGbl_RegFldAttr Wid128[XMEGBL_TILE_PLIF_ME2PL_MAX_STRMS128];  /**< 128 bit width bit field attributes */
} XMeGbl_RegPlUpsz;

/**
 * This typedef contains the attributes for the PL downsizer config register.
 */
typedef struct {
	u32 RegOff;                                                     /**< Register offset */
	XMeGbl_RegFldAttr Wid3264[XMEGBL_TILE_PLIF_PL2ME_MAX_STRMS];    /**< 32/64 bit width bit field attributes */
	XMeGbl_RegFldAttr Wid128[XMEGBL_TILE_PLIF_PL2ME_MAX_STRMS128];  /**< 128 bit width bit field attributes */
} XMeGbl_RegPlDwsz;

/**
 * This typedef contains the attributes for the PL downsizer enable register.
 */
typedef struct {
	u32 RegOff;                                                     /**< Register offset */
	XMeGbl_RegFldAttr En[XMEGBL_TILE_PLIF_PL2ME_MAX_STRMS];         /**< Enable field attributes */
} XMeGbl_RegPlDwszEn;

/**
 * This typedef contains the attributes for the PL downsizer enable register.
 */
typedef struct {
	u32 RegOff;                                                         /**< Register offset */
	XMeGbl_RegFldAttr Bypass[XMEGBL_TILE_PLIF_PL2ME_MAX_BYPASS_STRMS];  /**< Bypass field attributes */
} XMeGbl_RegPlDwszBypass;

/**
 * This typedef contains the attributes for Stream switch master port config register.
 */
typedef struct {
	u32 RegOff;                     /**< Register offset */
	XMeGbl_RegFldAttr MstrEn;       /**< Enable bit field attributes */
	XMeGbl_RegFldAttr PktEn;        /**< Packet enable bit field attributes */
	XMeGbl_RegFldAttr DrpHdr;       /**< Drop header bit field attributes */
	XMeGbl_RegFldAttr Config;       /**< Configuration bit field attributes */
} XMeGbl_RegStrmMstr;

/**
 * This typedef contains the attributes for Stream switch slave port config register.
 */
typedef struct {
	u32 RegOff;                     /**< Register offset */
	XMeGbl_RegFldAttr SlvEn;        /**< Enable bit field attributes */
	XMeGbl_RegFldAttr PktEn;        /**< Packet enable bit field attributes */
} XMeGbl_RegStrmSlv;

/**
 * This typedef contains the attributes for Stream switch slave slot config register.
 */
typedef struct {
	u32 RegOff;                     /**< Register offset */
	XMeGbl_RegFldAttr Id;           /**< Slot ID bit field attributes */
	XMeGbl_RegFldAttr Mask;         /**< Slot Mask bit field attributes */
	XMeGbl_RegFldAttr En;           /**< Slot enable bit field attributes */
	XMeGbl_RegFldAttr Msel;         /**< Slot msel bit field attributes */
	XMeGbl_RegFldAttr Arb;          /**< Slot arbitrator bit field attributes */
} XMeGbl_RegStrmSlot;

/**
 * This typedef contains the attributes for Tile DMA BD address word register.
 */
typedef struct {
	XMeGbl_RegFldAttr LkId;         /**< Lock ID bit field attributes */
	XMeGbl_RegFldAttr RelEn;        /**< Lock release enable bit field attributes */
	XMeGbl_RegFldAttr RelVal;       /**< Lock release value bit field attributes */
	XMeGbl_RegFldAttr RelValEn;     /**< Lock release value enable bit field attributes */
	XMeGbl_RegFldAttr AcqEn;        /**< Lock acquire enable bit field attributes */
	XMeGbl_RegFldAttr AcqVal;       /**< Lock acquire value bit field attributes */
	XMeGbl_RegFldAttr AcqValEn;     /**< Lock acquire value enable bit field attributes */
	XMeGbl_RegFldAttr Base;         /**< Base address bit field attributes */
} XMeGbl_RegTileBdAdd;

/**
 * This typedef contains the attributes for Tile DMA BD 2D word register.
 */
typedef struct {
	XMeGbl_RegFldAttr Incr;         /**< X/Y Incr bit field attributes */
	XMeGbl_RegFldAttr Wrap;         /**< X/Y Wrap bit field attributes */
	XMeGbl_RegFldAttr Off;          /**< X/Y Off bit field attributes */
} XMeGbl_RegTileBd2D;

/**
 * This typedef contains the attributes for Tile DMA BD Packet word register.
 */
typedef struct {
	XMeGbl_RegFldAttr Type;         /**< Packet type bit field attributes */
	XMeGbl_RegFldAttr Id;           /**< Packet ID bit field attributes */
} XMeGbl_RegTileBdPkt;

/**
 * This typedef contains the attributes for Tile DMA BD Interleave word register.
 */
typedef struct {
	XMeGbl_RegFldAttr Sts;          /**< Interleave status bit field attributes */
	XMeGbl_RegFldAttr Curr;         /**< Interleave current pointer bit field attributes */
} XMeGbl_RegTileBdInt;

/**
 * This typedef contains the attributes for Tile DMA BD Control word register.
 */
typedef struct {
	XMeGbl_RegFldAttr Valid;        /**< Valid bit field attributes */
	XMeGbl_RegFldAttr Ab;           /**< AB mode bit field attributes */
	XMeGbl_RegFldAttr Fifo;         /**< FIFO mode bit field attributes */
	XMeGbl_RegFldAttr Pkt;          /**< Packet mode bit field attributes */
	XMeGbl_RegFldAttr Intlv;        /**< Interleave mode bit field attributes */
	XMeGbl_RegFldAttr Cnt;          /**< Interleave count bit field attributes */
	XMeGbl_RegFldAttr NexEn;        /**< Next BD enable bit field attributes */
	XMeGbl_RegFldAttr NexBd;        /**< Next BD bit field attributes */
	XMeGbl_RegFldAttr Len;          /**< Length bit field attributes */
} XMeGbl_RegTileBdCtrl;

/**
 * This typedef contains the attributes for Tile DMA BD data structure.
 */
typedef struct {
	u32 RegOff[7U];                 /**< BD word offset */
	XMeGbl_RegTileBdAdd AddA;       /**< Address A attributes */
	XMeGbl_RegTileBdAdd AddB;       /**< Address B attributes */
	XMeGbl_RegTileBd2D Xinc;        /**< X addressing attributes */
	XMeGbl_RegTileBd2D Yinc;        /**< Y addressing attributes */
	XMeGbl_RegTileBdPkt Pkt;        /**< Packet attributes */
	XMeGbl_RegTileBdInt Intlv;      /**< Interleave attributes */
	XMeGbl_RegTileBdCtrl Ctrl;      /**< Control word attributes */
} XMeGbl_RegTileDmaBd;

/**
 * This typedef contains the attributes for Tile DMA Channel registers.
 */
typedef struct {
	u32 CtrlOff;                    /**< Control register offset */
	u32 StatQOff;                   /**< Start BD register offset */
	u32 StsOff;                     /**< Status register offset */
	XMeGbl_RegFldAttr Rst;          /**< Reset bit field attributes */
	XMeGbl_RegFldAttr En;           /**< Enable bit field attributes */
	XMeGbl_RegFldAttr StatQ;        /**< Start BD bit field attributes */
	XMeGbl_RegFldAttr Sts;          /**< Channel status field attributes */
} XMeGbl_RegTileDmaCh;

/**
 * This typedef contains the attributes for Shim DMA BD Control word register.
 */
typedef struct {
        XMeGbl_RegFldAttr Addh;         /**< Address high bit field attributes */
        XMeGbl_RegFldAttr NexEn;        /**< Next BD enable bit field attributes */
        XMeGbl_RegFldAttr NexBd;        /**< Next BD bit field attributes */
        XMeGbl_RegFldAttr Lock;         /**< Lock ID bit field attributes */
        XMeGbl_RegFldAttr RelEn;        /**< Lock release enable bit field attributes */
        XMeGbl_RegFldAttr RelVal;       /**< Lock release value bit field attributes */
        XMeGbl_RegFldAttr RelValEn;     /**< Lock release value enable bit field attributes */
        XMeGbl_RegFldAttr AcqEn;        /**< Lock acquire enable bit field attributes */
        XMeGbl_RegFldAttr AcqVal;       /**< Lock acquire value bit field attributes */
        XMeGbl_RegFldAttr AcqValEn;     /**< Lock acquire value enable bit field attributes */
        XMeGbl_RegFldAttr Valid;        /**< Valid bit field attributes */
} XMeGbl_RegShimBdCtrl;

/**
 * This typedef contains the attributes for Shim DMA BD AXI word register.
 */
typedef struct {
        XMeGbl_RegFldAttr Smid;         /**< SMID bit field attributes */
        XMeGbl_RegFldAttr Blen;         /**< BURST length bit field attributes */
        XMeGbl_RegFldAttr Qos;          /**< QoS bit field attributes */
        XMeGbl_RegFldAttr Sec;          /**< Secure bit field attributes */
        XMeGbl_RegFldAttr Cache;        /**< Cache bit field attributes */
} XMeGbl_RegShimBdAxi;

/**
 * This typedef contains the attributes for Shim DMA BD packet word register.
 */
typedef struct {
        XMeGbl_RegFldAttr En;           /**< Packet Enable bit field attributes */
        XMeGbl_RegFldAttr Type;         /**< Packet type bit field attributes */
        XMeGbl_RegFldAttr Id;           /**< Packet ID bit field attributes */
} XMeGbl_RegShimBdPkt;

/**
 * This typedef contains the attributes for Shim DMA BD data structure.
 */
typedef struct {
        u32 RegOff[5U];                 /**< BD word offset */
        XMeGbl_RegFldAttr Addl;         /**< Address Low bit field attributes */
        XMeGbl_RegFldAttr Len;          /**< Length bit field attributes */
        XMeGbl_RegShimBdCtrl Ctrl;      /**< COntrol word attributes */
        XMeGbl_RegShimBdAxi Axi;        /**< Axi word attributes */
        XMeGbl_RegShimBdPkt Pkt;        /**< Packet word attributes */
} XMeGbl_RegShimDmaBd;

/**
 * This typedef contains the attributes for Shim DMA channel registers.
 */
typedef struct {
        u32 CtrlOff;                    /**< Control offset */
	u32 StatQOff;                   /**< Start BD offset */
	XMeGbl_RegFldAttr PzStr;        /**< Pause stream bit field attributes */
	XMeGbl_RegFldAttr PzMem;        /**< Pause memory bit field attributes */
	XMeGbl_RegFldAttr En;           /**< Enable bit field attributes */
	XMeGbl_RegFldAttr StatQ;        /**< Start BD bit field attributes */
} XMeGbl_RegShimDmaCh;

/**
 * This typedef contains the attributes for Shim DMA status registers.
 */
typedef struct {
	u32 RegOff;                                              /**< Register offset */
	XMeGbl_RegFldAttr StartQueueOverflow;                    /**< Start queue overflow */
	XMeGbl_RegFldAttr CurrentBd;                             /**< Current Bd */
	XMeGbl_RegFldAttr StartQSize;                            /**< Start queue size */
	XMeGbl_RegFldAttr Stalled;                               /**< If stalled due to lock */
	XMeGbl_RegFldAttr Sts;                                   /**< Status */
} XMeGbl_RegShimDmaSts;

/**
 * This typedef contains the attributes for Shim colum reset register.
 */
typedef struct {
	u32 RegOff;                    /**< Register offset */
	XMeGbl_RegFldAttr Reset;       /**< Reset register */
} XMeGbl_RegShimColumnReset;

/**
 * This typedef contains the attributes for Shim stream Mux config registers.
 */
typedef struct {
	u32 CtrlOff;                    /**< Control offset */
	XMeGbl_RegFldAttr Port[4U];     /**< Port configuration */
} XMeGbl_RegShimMuxCfg;

/**
 * This typedef contains the attributes for Shim stream Demux config registers.
 */
typedef struct {
	u32 CtrlOff;                    /**< Control offset */
	XMeGbl_RegFldAttr Port[4U];     /**< Port configuration */
} XMeGbl_RegShimDemCfg;

/**
 * This typedef contains the attributes for stream switch event port select registers.
 */
typedef struct {
	u32 RegOff[2U];                 /**< Register offset */
	XMeGbl_RegFldAttr MstrSlv[8U];  /**< Master / slave configuration */
	XMeGbl_RegFldAttr Port[8U];     /**< Port configuration */
} XMeGbl_RegStrmEvtPort;

/**
 * This typedef contains the attributes for all Performance Counter ctrl registers.
 */
typedef struct {
	u32 RegOff[4U];                  /**< Register offset */
	XMeGbl_RegFldAttr Start[4U];     /**< Start control */
	XMeGbl_RegFldAttr Stop[4U];      /**< Stop control */
} XMeGbl_RegPerfCtrls;

/**
 * This typedef contains the attributes for Performance Counter reset registers.
 */
typedef struct {
	u32 RegOff[4U];                     /**< Register offset */
	XMeGbl_RegFldAttr Reset[4U];        /**< Reset */
} XMeGbl_RegPerfCtrlReset;

/**
 * This typedef contains the attributes for Performance Counter.
 */
typedef struct {
	u32 RegOff[4U];                     /**< Register offset */
	XMeGbl_RegFldAttr Counter[4U];      /**< Counter */
} XMeGbl_RegPerfCounter;

/**
 * This typedef contains the attributes for Performance Counter Event.
 */
typedef struct {
	u32 RegOff[4U];                         /**< Register offset */
	XMeGbl_RegFldAttr CounterEvent[4U];     /**< Counter Event */
} XMeGbl_RegPerfCounterEvent;

/**
 * This typedef contains the attributes for Event generate registers.
 */
typedef struct {
	u32 RegOff;                 /**< Register offset */
	XMeGbl_RegFldAttr Event;    /**< Event */
} XMeGbl_RegEventGenerate;

/**
 * This typedef contains the attributes for Event broadcast registers.
 */
typedef struct {
	u32 RegOff;                      /**< Register offset */
	XMeGbl_RegFldAttr Event;         /**< Event */
} XMeGbl_RegEventBroadcast;

/**
 * This typedef contains the attributes for Event broadcast set registers.
 */
typedef struct {
	u32 RegOff[8U];                 /**< Register offset */
	XMeGbl_RegFldAttr Event[8U];    /**< Event */
} XMeGbl_RegEventBroadcastSet;

/**
 * This typedef contains the attributes for Event broadcast clear registers.
 */
typedef struct {
	u32 RegOff[8U];                 /**< Register offset */
	XMeGbl_RegFldAttr Event[8U];    /**< Event */
} XMeGbl_RegEventBroadcastClear;

/**
 * This typedef contains the attributes for Event broadcast value registers.
 */
typedef struct {
	u32 RegOff[8U];                 /**< Register offset */
	XMeGbl_RegFldAttr Event[8U];    /**< Event */
} XMeGbl_RegEventBroadcastValue;

/**
 * This typedef contains the attributes for Trace ctrl registers.
 */
typedef struct {
	u32 RegOff[2U];              /**< Register offset */
	XMeGbl_RegFldAttr Mode;      /**< Mode */
	XMeGbl_RegFldAttr Start;     /**< Start control */
	XMeGbl_RegFldAttr Stop;      /**< Stop control */
	XMeGbl_RegFldAttr Id;        /**< Packet ID */
	XMeGbl_RegFldAttr Packet;    /**< Packet type */
} XMeGbl_RegTraceCtrls;

/**
 * This typedef contains the attributes for Trace event registers.
 */
typedef struct {
	u32 RegOff[2U];                  /**< Register offset */
	XMeGbl_RegFldAttr Event[8U];     /**< Event */
} XMeGbl_RegTraceEvent;

/**************************** Macro Definitions *****************************/

/**************************** Function prototypes ***************************/

#endif            /* end of protection macro */
/** @} */
