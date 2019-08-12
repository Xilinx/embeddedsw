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
* THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
* THE SOFTWARE.
*
*
*
******************************************************************************/

/*****************************************************************************/
/**
* @file xaiegbl.h
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
* 1.7  Nishad  12/05/2018  Renamed ME attributes to AIE
* 1.6  Hyun    12/13/2018  Added the core PC event registers
* 1.9  Jubaer  02/26/2019  Added the Group Event registers
* 2.0  Hyun    07/01/2019  Added XAIETILE_TIMER_MODULE_* macros
* </pre>
*
******************************************************************************/
#ifndef XAIEGBL_REGINIT_H /* prevent circular inclusions */
#define XAIEGBL_REGINIT_H /* by using protection macros */

/***************************** Include Files *********************************/
#include "xaiegbl_defs.h"
#include "xaiegbl.h"
#include "xaiegbl_params.h"

/************************** Constant Definitions *****************************/
#define XAIEGBL_TILE_PROGMEM_BASE        XAIEGBL_CORE_PRGMEM
#define XAIEGBL_TILE_DATAMEM_BASE        XAIEGBL_MEM_DATMEM

/**************************** Type Definitions *******************************/
/**
 * This typedef contains the attributes for the register bit fields.
 */
typedef struct {
	u32 Lsb;                /**< Bit position of the register bit-field in the 32-bit value */
	u32 Mask;               /**< Bit mask of the register bit-field in the 32-bit value */
} XAieGbl_RegFldAttr;

/**
 * This typedef contains the attributes for the Release/Acquire register bit fields.
 */
typedef struct {
	u32 RelNvOff;                   /**< Release NV register offset */
	XAieGbl_RegFldAttr RelNv;        /**< Release NV filed attributes */
	u32 RelV0Off;                   /**< Release V0 register offset */
	XAieGbl_RegFldAttr RelV0;        /**< Release V0 field attributes */
	u32 RelV1Off;                   /**< Release V1 register offset */
	XAieGbl_RegFldAttr RelV1;        /**< Release V1 field attributes */
	u32 AcqNvOff;                   /**< Acquire NV register offset */
	XAieGbl_RegFldAttr AcqNv;        /**< Acquire NV field attributes */
	u32 AcqV0Off;                   /**< Acquire V0 register offset */
	XAieGbl_RegFldAttr AcqV0;        /**< Acquire V0 field attributes */
	u32 AcqV1Off;                   /**< Acquire V1 register offset */
	XAieGbl_RegFldAttr AcqV1;        /**< Acquire V1 field attributes */
} XAieGbl_RegLocks;

/**
 * This typedef contains the attributes for the Core control register.
 */
typedef struct {
	u32 RegOff;                     /**< Register offset */
	XAieGbl_RegFldAttr CtrlEn;       /**< Enable field attributes */
	XAieGbl_RegFldAttr CtrlRst;      /**< Reset field attributes */
} XAieGbl_RegCoreCtrl;

/**
 * This typedef contains the attributes for the Core status register.
 */
typedef struct {
	u32 RegOff;                     /**< Register offset */
        XAieGbl_RegFldAttr Done;         /**< Done value field attributes */
	XAieGbl_RegFldAttr Rst;          /**< Reset value field attributes */
	XAieGbl_RegFldAttr En;           /**< Enable value field attributes */
} XAieGbl_RegCoreSts;

/**
 * This typedef contains the attributes for Timer control and value regs.
 */
typedef struct {
	u32 CtrlOff;                    /**< Timer control Register offset */
	u32 LowOff;                     /**< Timer low value Register offset */
	u32 HighOff;                    /**< Timer high value Register offset */
} XAieGbl_RegTimer;

/* Index values to TimerReg[] */
#define XAIETILE_TIMER_MODULE_CORE		0x0U
#define XAIETILE_TIMER_MODULE_PL		0x1U
#define XAIETILE_TIMER_MODULE_MEM		0x2U

/**
 * This typedef contains the attributes for the PL upsizer config register.
 */
typedef struct {
	u32 RegOff;                                                     /**< Register offset */
	XAieGbl_RegFldAttr Wid3264[XAIEGBL_TILE_PLIF_AIE2PL_MAX_STRMS];    /**< 32/64 bit width bit field attributes */
	XAieGbl_RegFldAttr Wid128[XAIEGBL_TILE_PLIF_AIE2PL_MAX_STRMS128];  /**< 128 bit width bit field attributes */
} XAieGbl_RegPlUpsz;

/**
 * This typedef contains the attributes for the PL downsizer config register.
 */
typedef struct {
	u32 RegOff;                                                     /**< Register offset */
	XAieGbl_RegFldAttr Wid3264[XAIEGBL_TILE_PLIF_PL2AIE_MAX_STRMS];    /**< 32/64 bit width bit field attributes */
	XAieGbl_RegFldAttr Wid128[XAIEGBL_TILE_PLIF_PL2AIE_MAX_STRMS128];  /**< 128 bit width bit field attributes */
} XAieGbl_RegPlDwsz;

/**
 * This typedef contains the attributes for the PL downsizer enable register.
 */
typedef struct {
	u32 RegOff;                                                     /**< Register offset */
	XAieGbl_RegFldAttr En[XAIEGBL_TILE_PLIF_PL2AIE_MAX_STRMS];         /**< Enable field attributes */
} XAieGbl_RegPlDwszEn;

/**
 * This typedef contains the attributes for the PL downsizer enable register.
 */
typedef struct {
	u32 RegOff;                                                         /**< Register offset */
	XAieGbl_RegFldAttr Bypass[XAIEGBL_TILE_PLIF_PL2AIE_MAX_BYPASS_STRMS];  /**< Bypass field attributes */
} XAieGbl_RegPlDwszBypass;

/**
 * This typedef contains the attributes for Stream switch master port config register.
 */
typedef struct {
	u32 RegOff;                     /**< Register offset */
	XAieGbl_RegFldAttr MstrEn;       /**< Enable bit field attributes */
	XAieGbl_RegFldAttr PktEn;        /**< Packet enable bit field attributes */
	XAieGbl_RegFldAttr DrpHdr;       /**< Drop header bit field attributes */
	XAieGbl_RegFldAttr Config;       /**< Configuration bit field attributes */
} XAieGbl_RegStrmMstr;

/**
 * This typedef contains the attributes for Stream switch slave port config register.
 */
typedef struct {
	u32 RegOff;                     /**< Register offset */
	XAieGbl_RegFldAttr SlvEn;        /**< Enable bit field attributes */
	XAieGbl_RegFldAttr PktEn;        /**< Packet enable bit field attributes */
} XAieGbl_RegStrmSlv;

/**
 * This typedef contains the attributes for Stream switch slave slot config register.
 */
typedef struct {
	u32 RegOff;                     /**< Register offset */
	XAieGbl_RegFldAttr Id;           /**< Slot ID bit field attributes */
	XAieGbl_RegFldAttr Mask;         /**< Slot Mask bit field attributes */
	XAieGbl_RegFldAttr En;           /**< Slot enable bit field attributes */
	XAieGbl_RegFldAttr Msel;         /**< Slot msel bit field attributes */
	XAieGbl_RegFldAttr Arb;          /**< Slot arbitrator bit field attributes */
} XAieGbl_RegStrmSlot;

/**
 * This typedef contains the attributes for Tile DMA BD address word register.
 */
typedef struct {
	XAieGbl_RegFldAttr LkId;         /**< Lock ID bit field attributes */
	XAieGbl_RegFldAttr RelEn;        /**< Lock release enable bit field attributes */
	XAieGbl_RegFldAttr RelVal;       /**< Lock release value bit field attributes */
	XAieGbl_RegFldAttr RelValEn;     /**< Lock release value enable bit field attributes */
	XAieGbl_RegFldAttr AcqEn;        /**< Lock acquire enable bit field attributes */
	XAieGbl_RegFldAttr AcqVal;       /**< Lock acquire value bit field attributes */
	XAieGbl_RegFldAttr AcqValEn;     /**< Lock acquire value enable bit field attributes */
	XAieGbl_RegFldAttr Base;         /**< Base address bit field attributes */
} XAieGbl_RegTileBdAdd;

/**
 * This typedef contains the attributes for Tile DMA BD 2D word register.
 */
typedef struct {
	XAieGbl_RegFldAttr Incr;         /**< X/Y Incr bit field attributes */
	XAieGbl_RegFldAttr Wrap;         /**< X/Y Wrap bit field attributes */
	XAieGbl_RegFldAttr Off;          /**< X/Y Off bit field attributes */
} XAieGbl_RegTileBd2D;

/**
 * This typedef contains the attributes for Tile DMA BD Packet word register.
 */
typedef struct {
	XAieGbl_RegFldAttr Type;         /**< Packet type bit field attributes */
	XAieGbl_RegFldAttr Id;           /**< Packet ID bit field attributes */
} XAieGbl_RegTileBdPkt;

/**
 * This typedef contains the attributes for Tile DMA BD Interleave word register.
 */
typedef struct {
	XAieGbl_RegFldAttr Sts;          /**< Interleave status bit field attributes */
	XAieGbl_RegFldAttr Curr;         /**< Interleave current pointer bit field attributes */
} XAieGbl_RegTileBdInt;

/**
 * This typedef contains the attributes for Tile DMA BD Control word register.
 */
typedef struct {
	XAieGbl_RegFldAttr Valid;        /**< Valid bit field attributes */
	XAieGbl_RegFldAttr Ab;           /**< AB mode bit field attributes */
	XAieGbl_RegFldAttr Fifo;         /**< FIFO mode bit field attributes */
	XAieGbl_RegFldAttr Pkt;          /**< Packet mode bit field attributes */
	XAieGbl_RegFldAttr Intlv;        /**< Interleave mode bit field attributes */
	XAieGbl_RegFldAttr Cnt;          /**< Interleave count bit field attributes */
	XAieGbl_RegFldAttr NexEn;        /**< Next BD enable bit field attributes */
	XAieGbl_RegFldAttr NexBd;        /**< Next BD bit field attributes */
	XAieGbl_RegFldAttr Len;          /**< Length bit field attributes */
} XAieGbl_RegTileBdCtrl;

/**
 * This typedef contains the attributes for Tile DMA BD data structure.
 */
typedef struct {
	u32 RegOff[7U];                 /**< BD word offset */
	XAieGbl_RegTileBdAdd AddA;       /**< Address A attributes */
	XAieGbl_RegTileBdAdd AddB;       /**< Address B attributes */
	XAieGbl_RegTileBd2D Xinc;        /**< X addressing attributes */
	XAieGbl_RegTileBd2D Yinc;        /**< Y addressing attributes */
	XAieGbl_RegTileBdPkt Pkt;        /**< Packet attributes */
	XAieGbl_RegTileBdInt Intlv;      /**< Interleave attributes */
	XAieGbl_RegTileBdCtrl Ctrl;      /**< Control word attributes */
} XAieGbl_RegTileDmaBd;

/**
 * This typedef contains the attributes for Tile DMA Channel registers.
 */
typedef struct {
	u32 CtrlOff;                    /**< Control register offset */
	u32 StatQOff;                   /**< Start BD register offset */
	u32 StsOff;                     /**< Status register offset */
	XAieGbl_RegFldAttr Rst;          /**< Reset bit field attributes */
	XAieGbl_RegFldAttr En;           /**< Enable bit field attributes */
	XAieGbl_RegFldAttr StatQ;        /**< Start BD bit field attributes */
	XAieGbl_RegFldAttr Sts;          /**< Channel status field attributes */
} XAieGbl_RegTileDmaCh;

/**
 * This typedef contains the attributes for Shim DMA BD Control word register.
 */
typedef struct {
        XAieGbl_RegFldAttr Addh;         /**< Address high bit field attributes */
        XAieGbl_RegFldAttr NexEn;        /**< Next BD enable bit field attributes */
        XAieGbl_RegFldAttr NexBd;        /**< Next BD bit field attributes */
        XAieGbl_RegFldAttr Lock;         /**< Lock ID bit field attributes */
        XAieGbl_RegFldAttr RelEn;        /**< Lock release enable bit field attributes */
        XAieGbl_RegFldAttr RelVal;       /**< Lock release value bit field attributes */
        XAieGbl_RegFldAttr RelValEn;     /**< Lock release value enable bit field attributes */
        XAieGbl_RegFldAttr AcqEn;        /**< Lock acquire enable bit field attributes */
        XAieGbl_RegFldAttr AcqVal;       /**< Lock acquire value bit field attributes */
        XAieGbl_RegFldAttr AcqValEn;     /**< Lock acquire value enable bit field attributes */
        XAieGbl_RegFldAttr Valid;        /**< Valid bit field attributes */
} XAieGbl_RegShimBdCtrl;

/**
 * This typedef contains the attributes for Shim DMA BD AXI word register.
 */
typedef struct {
        XAieGbl_RegFldAttr Smid;         /**< SMID bit field attributes */
        XAieGbl_RegFldAttr Blen;         /**< BURST length bit field attributes */
        XAieGbl_RegFldAttr Qos;          /**< QoS bit field attributes */
        XAieGbl_RegFldAttr Sec;          /**< Secure bit field attributes */
        XAieGbl_RegFldAttr Cache;        /**< Cache bit field attributes */
} XAieGbl_RegShimBdAxi;

/**
 * This typedef contains the attributes for Shim DMA BD packet word register.
 */
typedef struct {
        XAieGbl_RegFldAttr En;           /**< Packet Enable bit field attributes */
        XAieGbl_RegFldAttr Type;         /**< Packet type bit field attributes */
        XAieGbl_RegFldAttr Id;           /**< Packet ID bit field attributes */
} XAieGbl_RegShimBdPkt;

/**
 * This typedef contains the attributes for Shim DMA BD data structure.
 */
typedef struct {
        u32 RegOff[5U];                 /**< BD word offset */
        XAieGbl_RegFldAttr Addl;         /**< Address Low bit field attributes */
        XAieGbl_RegFldAttr Len;          /**< Length bit field attributes */
        XAieGbl_RegShimBdCtrl Ctrl;      /**< COntrol word attributes */
        XAieGbl_RegShimBdAxi Axi;        /**< Axi word attributes */
        XAieGbl_RegShimBdPkt Pkt;        /**< Packet word attributes */
} XAieGbl_RegShimDmaBd;

/**
 * This typedef contains the attributes for Shim DMA channel registers.
 */
typedef struct {
        u32 CtrlOff;                    /**< Control offset */
	u32 StatQOff;                   /**< Start BD offset */
	XAieGbl_RegFldAttr PzStr;        /**< Pause stream bit field attributes */
	XAieGbl_RegFldAttr PzMem;        /**< Pause memory bit field attributes */
	XAieGbl_RegFldAttr En;           /**< Enable bit field attributes */
	XAieGbl_RegFldAttr StatQ;        /**< Start BD bit field attributes */
} XAieGbl_RegShimDmaCh;

/**
 * This typedef contains the attributes for Shim DMA status registers.
 */
typedef struct {
	u32 RegOff;                                              /**< Register offset */
	XAieGbl_RegFldAttr StartQueueOverflow;                    /**< Start queue overflow */
	XAieGbl_RegFldAttr CurrentBd;                             /**< Current Bd */
	XAieGbl_RegFldAttr StartQSize;                            /**< Start queue size */
	XAieGbl_RegFldAttr Stalled;                               /**< If stalled due to lock */
	XAieGbl_RegFldAttr Sts;                                   /**< Status */
} XAieGbl_RegShimDmaSts;

/**
 * This typedef contains the attributes for Shim colum reset register.
 */
typedef struct {
	u32 RegOff;                    /**< Register offset */
	XAieGbl_RegFldAttr Reset;       /**< Reset register */
} XAieGbl_RegShimColumnReset;

/**
 * This typedef contains the attributes for Shim stream Mux config registers.
 */
typedef struct {
	u32 CtrlOff;                    /**< Control offset */
	XAieGbl_RegFldAttr Port[4U];     /**< Port configuration */
} XAieGbl_RegShimMuxCfg;

/**
 * This typedef contains the attributes for Shim stream Demux config registers.
 */
typedef struct {
	u32 CtrlOff;                    /**< Control offset */
	XAieGbl_RegFldAttr Port[4U];     /**< Port configuration */
} XAieGbl_RegShimDemCfg;

/**
 * This typedef contains the attributes for stream switch event port select registers.
 */
typedef struct {
	u32 RegOff[2U];                 /**< Register offset */
	XAieGbl_RegFldAttr MstrSlv[8U];  /**< Master / slave configuration */
	XAieGbl_RegFldAttr Port[8U];     /**< Port configuration */
} XAieGbl_RegStrmEvtPort;

/**
 * This typedef contains the attributes for all Performance Counter ctrl registers.
 */
typedef struct {
	u32 RegOff[4U];                  /**< Register offset */
	XAieGbl_RegFldAttr Start[4U];     /**< Start control */
	XAieGbl_RegFldAttr Stop[4U];      /**< Stop control */
} XAieGbl_RegPerfCtrls;

/**
 * This typedef contains the attributes for Performance Counter reset registers.
 */
typedef struct {
	u32 RegOff[4U];                     /**< Register offset */
	XAieGbl_RegFldAttr Reset[4U];        /**< Reset */
} XAieGbl_RegPerfCtrlReset;

/**
 * This typedef contains the attributes for Performance Counter.
 */
typedef struct {
	u32 RegOff[4U];                     /**< Register offset */
	XAieGbl_RegFldAttr Counter[4U];      /**< Counter */
} XAieGbl_RegPerfCounter;

/**
 * This typedef contains the attributes for Performance Counter Event.
 */
typedef struct {
	u32 RegOff[4U];                         /**< Register offset */
	XAieGbl_RegFldAttr CounterEvent[4U];     /**< Counter Event */
} XAieGbl_RegPerfCounterEvent;

/**
 * This typedef contains the attributes for Event generate registers.
 */
typedef struct {
	u32 RegOff;                 /**< Register offset */
	XAieGbl_RegFldAttr Event;    /**< Event */
} XAieGbl_RegEventGenerate;

/**
 * This typedef contains the attributes for Event broadcast registers.
 */
typedef struct {
	u32 RegOff;                      /**< Register offset */
	XAieGbl_RegFldAttr Event;         /**< Event */
} XAieGbl_RegEventBroadcast;

/**
 * This typedef contains the attributes for Event broadcast set registers.
 */
typedef struct {
	u32 RegOff[8U];                 /**< Register offset */
	XAieGbl_RegFldAttr Event[8U];    /**< Event */
} XAieGbl_RegEventBroadcastSet;

/**
 * This typedef contains the attributes for Event broadcast clear registers.
 */
typedef struct {
	u32 RegOff[8U];                 /**< Register offset */
	XAieGbl_RegFldAttr Event[8U];    /**< Event */
} XAieGbl_RegEventBroadcastClear;

/**
 * This typedef contains the attributes for Event broadcast value registers.
 */
typedef struct {
	u32 RegOff[8U];                 /**< Register offset */
	XAieGbl_RegFldAttr Event[8U];    /**< Event */
} XAieGbl_RegEventBroadcastValue;

/**
 * This typedef contains the attributes for Trace ctrl registers.
 */
typedef struct {
	u32 RegOff[2U];              /**< Register offset */
	XAieGbl_RegFldAttr Mode;      /**< Mode */
	XAieGbl_RegFldAttr Start;     /**< Start control */
	XAieGbl_RegFldAttr Stop;      /**< Stop control */
	XAieGbl_RegFldAttr Id;        /**< Packet ID */
	XAieGbl_RegFldAttr Packet;    /**< Packet type */
} XAieGbl_RegTraceCtrls;

/**
 * This typedef contains the attributes for Trace event registers.
 */
typedef struct {
	u32 RegOff[2U];                  /**< Register offset */
	XAieGbl_RegFldAttr Event[8U];     /**< Event */
} XAieGbl_RegTraceEvent;

/**
 * This typedef contains the attributes for Core PC events.
 */
typedef struct {
	u32 RegOff;                             /**< Register offset */
	XAieGbl_RegFldAttr PCAddr;               /**< PC address */
	XAieGbl_RegFldAttr Valid;                /**< Valid bit */
} XAieGbl_RegCorePCEvent;

/**
 * This typedef contains the mask attributes for Group Events
 */
typedef struct {
	u32 Mask[9U];  /* Mask Bits*/
} XAieGbl_GroupEvents;

typedef struct {
	u32 Lsb[4U];		/**< Bit position of the register bit-field in the 32-bit value */
	u32 Mask[4U];		/**< Bit mask of the register bit-field in the 32-bit value */

} XAieGbl_ComboEventInput;

typedef struct {
	u32 Lsb[3U];		/**< Bit position of the register bit-field in the 32-bit value */
	u32 Mask[3U];		/**< Bit mask of the register bit-field in the 32-bit value */

} XAieGbl_ComboEventControl;

/**
 * This typedef contains the attributes for Shim reset register.
 */
typedef struct {
	u32 RegOff;			/**< Reset offset */
	XAieGbl_RegFldAttr Reset;	/**< Reset register */
} XAieGbl_RegShimReset;

/**
 * This typedef contains the attributes for Stream Switch Event Port Selection register.
 */
typedef struct {
	u32 RegAddr;				/**< Register Address */
	XAieGbl_RegFldAttr PortIndex[8U];	/**< Port index */
	XAieGbl_RegFldAttr PortMode[8U];	/**< Port type */
} XAieGbl_RegStrmSwEventPortSelect;

/**************************** Macro Definitions *****************************/

/**************************** Function prototypes ***************************/

#endif            /* end of protection macro */
/** @} */
