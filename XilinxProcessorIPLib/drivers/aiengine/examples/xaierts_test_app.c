/******************************************************************************
* Copyright (C) 2018 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
* @file xaierts_test_app.c
* @{
*
* This file contains the bare-metal test application for AIE C RTS.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who     Date     Changes
* ----- ------  -------- -----------------------------------------------------
* 1.0  Naresh  05/23/2018  Initial creation
* 1.1  Naresh  07/11/2018  Updated copyright info
* 1.2  Nishad  12/05/2018  Renamed ME attributes to AIE
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/
#include "xaiegbl_defs.h"
#include "xaiegbl.h"
#include "xaiedma_tile.h"
#include "xaietile_lock.h"
#include "xaietile_strm.h"
#include "xaietile_core.h"
#include "xaietile_plif.h"

#ifdef __AIEBAREMTL__
/************************** Constant Definitions *****************************/
#define XAIE_NUM_ROWS            4
#define XAIE_NUM_COLS            8
#define XAIE_ADDR_ARRAY_OFF      0x2

/************************** Variable Definitions *****************************/
XAieGbl_Config *MeConfigPtr;	/**< AIE configuration pointer */
XAieGbl MeInst;			/**< AIE global instance */
XAieGbl_HwCfg MeConfig;          /**< AIE HW configuration instance */

XAieGbl_Tile TileInst[XAIE_NUM_COLS][XAIE_NUM_ROWS+1];

/************************** Function Prototypes  *****************************/

/************************** Function Definitions *****************************/
/*****************************************************************************/
/**
*
* This is the main entry point for the AIE RTS driver test.
*
* @param	None.
*
* @return	None.
*
* @note		None.
*
*******************************************************************************/
void main(void)
{
	u32 retval, count, addr, idx1, idx2;
	u32 regval;
        u32 slave;
        u32 master;
        u32 slot;
	XAieDma_Tile TileDmaInst;

        /* Initialize the HW for Sim */
	XAIEGBL_HWCFG_SET_CONFIG((&MeConfig), XAIE_NUM_ROWS, XAIE_NUM_COLS, XAIE_ADDR_ARRAY_OFF);
        XAieGbl_HwInit(&MeConfig);

        XAie_print("Dumping the tile instances\n");
        for(idx1 = 0; idx1 < XAIE_NUM_COLS; idx1++) {
                for(idx2 = 0; idx2 < (XAIE_NUM_ROWS+1); idx2++) {
                        XAie_print("Tile(%d,%d): TileAddr:%x, col:%d, row:%d, Tiletype:%d\n",
                                idx1, idx2, TileInst[idx1][idx2].TileAddr,
                                TileInst[idx1][idx2].ColId,
                                TileInst[idx1][idx2].RowId,
                                TileInst[idx1][idx2].TileType);
                }
        }

	MeConfigPtr = XAieGbl_LookupConfig(XPAR_AIE_DEVICE_ID);
	(void)XAieGbl_CfgInitialize(&MeInst, &TileInst[0][0], MeConfigPtr);

        XAie_print("Dumping the tile instances\n");
        for(idx1 = 0; idx1 < XAIE_NUM_COLS; idx1++) {
                for(idx2 = 0; idx2 < (XAIE_NUM_ROWS+1); idx2++) {
                        XAie_print("Tile(%d,%d): TileAddr:%x, col:%d, row:%d, Tiletype:%d\n",
                                idx1, idx2, TileInst[idx1][idx2].TileAddr,
                                TileInst[idx1][idx2].ColId,
                                TileInst[idx1][idx2].RowId,
                                TileInst[idx1][idx2].TileType);
                }
        }

	/***********************************************************
	 * Tests for sample startup.py
	 **********************************************************/
	XAie_print("\nStart of startup.py commands\n\n");

	/*#  S_SHIM_NORTH_ch0_C0 M_SHIM_SOUTH_ch0_C0 net0
	self.me().tile(0,0).streamswitch().connect_circuit(rt.stream_ext_n(0), rt.stream_ext_s(0))*/
	XAieTile_StrmConnectCct(&(TileInst[0][0]), XAIETILE_STRSW_SPORT_NORTH((&(TileInst[0][0])), 0),
					XAIETILE_STRSW_MPORT_SOUTH((&(TileInst[0][0])), 0), XAIE_ENABLE);
	XAie_print("\n");

	/*# net0 M_SHIM_SOUTH_ch0_C0 meToPlDummy_0
	self.me().tile(0,0).pl_interface().set_me_to_pl_width(0,32)*/
	XAieTile_PlIntfStrmWidCfg(&(TileInst[0][0]), 0, 0, 32);
	XAie_print("\n");

	/*#  S_MM2S_DMA_ch0_C0_R0 M_SOUTH_ch0_C0_R0 net0
	self.me().tile(0,1).streamswitch().connect_circuit(rt.stream_dma(0), rt.stream_ext_s(0))*/
	XAieTile_StrmConnectCct(&(TileInst[0][1]), XAIETILE_STRSW_SPORT_DMA((&(TileInst[0][1])), 0),
					XAIETILE_STRSW_MPORT_SOUTH((&(TileInst[0][1])), 0), XAIE_ENABLE);
	XAie_print("\n");

	/*#  M_SOUTH_ch0_C0_R0 S_SHIM_NORTH_ch0_C0 net0
	#  meToPlDummy_0 i2_pi0 net0*/

	/*# net2 S_SHIM_SOUTH_ch3_C0 M_SHIM_NORTH_ch2_C0
	self.me().tile(0,0).pl_interface().set_pl_to_me_width(3,32)*/
	XAieTile_PlIntfStrmWidCfg(&(TileInst[0][0]), 1, 3, 32);
	XAie_print("\n");

	/*#  S_SHIM_SOUTH_ch3_C0 M_SHIM_NORTH_ch2_C0 net2
	self.me().tile(0,0).streamswitch().connect_circuit(rt.stream_ext_s(3), rt.stream_ext_n(2))*/
	XAieTile_StrmConnectCct(&(TileInst[0][0]), XAIETILE_STRSW_SPORT_SOUTH((&(TileInst[0][0])), 3),
					XAIETILE_STRSW_MPORT_NORTH((&(TileInst[0][0])), 2), XAIE_ENABLE);
	XAie_print("\n");

	/*#  S_SOUTH_ch2_C0_R0 M_S2MM_DMA_ch1_C0_R0 net2
	self.me().tile(0,1).streamswitch().connect_circuit(rt.stream_ext_s(2), rt.stream_dma(1))*/
	XAieTile_StrmConnectCct(&(TileInst[0][1]), XAIETILE_STRSW_SPORT_SOUTH((&(TileInst[0][1])), 2),
					XAIETILE_STRSW_MPORT_DMA((&(TileInst[0][1])), 1), XAIE_ENABLE);
	XAie_print("\n");

	/*#  i1_po0 plTomeDummy_0 net2
	#  M_SHIM_NORTH_ch2_C0 S_SOUTH_ch2_C0_R0 net2

	# net2 plTomeDummy_0 S_SHIM_SOUTH_ch3_C0
	self.me().tile(0,0).pl_interface().set_pl_to_me_width(3,32)*/
	XAieTile_PlIntfStrmWidCfg(&(TileInst[0][0]), 1, 3, 32);
	XAie_print("\n");

	/*#  S_SOUTH_ch5_C0_R0 M_S2MM_DMA_ch0_C0_R0 net1
	self.me().tile(0,1).streamswitch().connect_circuit(rt.stream_ext_s(5), rt.stream_dma(0))*/
	XAieTile_StrmConnectCct(&(TileInst[0][1]), XAIETILE_STRSW_SPORT_SOUTH((&(TileInst[0][1])), 5),
					XAIETILE_STRSW_MPORT_DMA((&(TileInst[0][1])), 0), XAIE_ENABLE);
	XAie_print("\n");

	/*#  i0_po0 plTomeDummy_0 net1
	#  M_SHIM_NORTH_ch5_C0 S_SOUTH_ch5_C0_R0 net1

	# net1 S_SHIM_SOUTH_ch0_C0 M_SHIM_NORTH_ch5_C0
	self.me().tile(0,0).pl_interface().set_pl_to_me_width(0,32)*/
	XAieTile_PlIntfStrmWidCfg(&(TileInst[0][0]), 1, 0, 32);
	XAie_print("\n");

	/*#  S_SHIM_SOUTH_ch0_C0 M_SHIM_NORTH_ch5_C0 net1
	self.me().tile(0,0).streamswitch().connect_circuit(rt.stream_ext_s(0), rt.stream_ext_n(5))*/
	XAieTile_StrmConnectCct(&(TileInst[0][0]), XAIETILE_STRSW_SPORT_SOUTH((&(TileInst[0][0])), 0),
					XAIETILE_STRSW_MPORT_NORTH((&(TileInst[0][0])), 5), XAIE_ENABLE);
	XAie_print("\n");

	/*# net1 plTomeDummy_0 S_SHIM_SOUTH_ch0_C0
	self.me().tile(0,0).pl_interface().set_pl_to_me_width(0,32)*/
	XAieTile_PlIntfStrmWidCfg(&(TileInst[0][0]), 1, 0, 32);
	XAie_print("\n");

	/*rt.elf.load(self.me().tile(0,1).core(),os.path.join(self.args.pkg_dir,"me/0_0/Release/0_0"))*/
#if 0
        XAieGbl_LoadElf(&(TileInst[0][2]), argv[3], XAIE_ENABLE);
	XAie_print("\n");
#endif
	/*self.loadSymbolTable( 0,0,os.path.join(self.args.pkg_dir,"me/0_0/scripts/0_0.bcf"))*/

	/*if args.debug==False:
		self.me().tile(0,1).core().ctrl().enable()*/
	XAieTile_CoreControl(&(TileInst[0][1]), XAIE_ENABLE, XAIE_DISABLE);
	XAie_print("\n");

	/*#Setting buffer mg_0_0_buf2_ping
	self.me().tile(0,1).mm().dma().set_BD_simple(numBD=0,baseAddress=0x0,transLen=2048,nextBD=1,lockID=0,lockAcqValue=0,lockRelValue=1,enable=True)*/
        TileDmaInst.IsReady = 0xFFFFFFFF;
	XAieDma_TileInitialize(&(TileInst[0][1]), &TileDmaInst);
	XAie_print("\n");

	XAieDma_TileBdSetLock(&TileDmaInst, 0, XAIEDMA_TILE_BD_ADDRA, 0, 1, 1, 1, 0);
	XAieDma_TileBdSetAdrLenMod(&TileDmaInst, 0, 0, 0, 2048, 0, 0);
	XAieDma_TileBdSetNext(&TileDmaInst, 0, 1);
	XAieDma_TileBdWrite(&TileDmaInst, 0);
	XAie_print("\n");

	/*#Setting buffer mg_0_0_buf2d_pong
	self.me().tile(0,1).mm().dma().set_BD_simple(numBD=1,baseAddress=0x5000,transLen=2048,nextBD=0,lockID=1,lockAcqValue=0,lockRelValue=1,enable=True)*/
	XAieDma_TileBdSetLock(&TileDmaInst, 1, XAIEDMA_TILE_BD_ADDRA, 1, 1, 1, 1, 0);
	XAieDma_TileBdSetAdrLenMod(&TileDmaInst, 1, 0x5000, 0, 2048, 0, 0);
	XAieDma_TileBdSetNext(&TileDmaInst, 1, 0);
	XAieDma_TileBdWrite(&TileDmaInst, 1);
	XAie_print("\n");

	/*self.me().tile(0,1).mm().dma().start_s2mm_channel(channel=0,startBD=0)*/
	XAieDma_TileSetStartBd((&TileDmaInst), XAIEDMA_TILE_CHNUM_S2MM0, 0);
	XAieDma_TileChControl((&TileDmaInst), XAIEDMA_TILE_CHNUM_S2MM0, XAIE_RESETDISABLE, XAIE_ENABLE);
	XAie_print("\n");
	
	/*#Setting buffer mg_0_0_buf1_ping
	self.me().tile(0,1).mm().dma().set_BD_simple(numBD=2,baseAddress=0x4800,transLen=2048,nextBD=3,lockID=2,lockAcqValue=0,lockRelValue=1,enable=True)*/
	XAieDma_TileBdSetLock(&TileDmaInst, 2, XAIEDMA_TILE_BD_ADDRA, 2, 1, 1, 1, 0);
	XAieDma_TileBdSetAdrLenMod(&TileDmaInst, 2, 0x4800, 0, 2048, 0, 0);
	XAieDma_TileBdSetNext(&TileDmaInst, 2, 3);
	XAieDma_TileBdWrite(&TileDmaInst, 2);
	XAie_print("\n");
	
	/*#Setting buffer mg_0_0_buf1d_pong
	self.me().tile(0,1).mm().dma().set_BD_simple(numBD=3,baseAddress=0x1000,transLen=2048,nextBD=2,lockID=3,lockAcqValue=0,lockRelValue=1,enable=True)*/
	XAieDma_TileBdSetLock(&TileDmaInst, 3, XAIEDMA_TILE_BD_ADDRA, 3, 1, 1, 1, 0);
	XAieDma_TileBdSetAdrLenMod(&TileDmaInst, 3, 0x1000, 0, 2048, 0, 0);
	XAieDma_TileBdSetNext(&TileDmaInst, 3, 2);
	XAieDma_TileBdWrite(&TileDmaInst, 3);
	XAie_print("\n");

	/*self.me().tile(0,1).mm().dma().start_s2mm_channel(channel=1,startBD=2)*/
	XAieDma_TileSetStartBd((&TileDmaInst), XAIEDMA_TILE_CHNUM_S2MM1, 2);
	XAieDma_TileChControl((&TileDmaInst), XAIEDMA_TILE_CHNUM_S2MM1, XAIE_RESETDISABLE, XAIE_ENABLE);
	XAie_print("\n");
	
	/*#Setting buffer mg_0_0_buf0_ping
	self.me().tile(0,1).mm().dma().set_BD_simple(numBD=4,baseAddress=0x4000,transLen=2048,nextBD=5,lockID=4,lockAcqValue=1,lockRelValue=0,enable=True)*/
	XAieDma_TileBdSetLock(&TileDmaInst, 4, XAIEDMA_TILE_BD_ADDRA, 4, 1, 0, 1, 1);
	XAieDma_TileBdSetAdrLenMod(&TileDmaInst, 4, 0x4000, 0, 2048, 0, 0);
	XAieDma_TileBdSetNext(&TileDmaInst, 4, 5);
	XAieDma_TileBdWrite(&TileDmaInst, 4);
	XAie_print("\n");
	
	/*#Setting buffer mg_0_0_buf0d_pong
	self.me().tile(0,1).mm().dma().set_BD_simple(numBD=5,baseAddress=0x800,transLen=2048,nextBD=4,lockID=5,lockAcqValue=1,lockRelValue=0,enable=True)*/
	XAieDma_TileBdSetLock(&TileDmaInst, 5, XAIEDMA_TILE_BD_ADDRA, 5, 1, 0, 1, 1);
	XAieDma_TileBdSetAdrLenMod(&TileDmaInst, 5, 0x800, 0, 2048, 0, 0);
	XAieDma_TileBdSetNext(&TileDmaInst, 5, 4);
	XAieDma_TileBdWrite(&TileDmaInst, 5);
	XAie_print("\n");

	/*self.me().tile(0,1).mm().dma().start_mm2s_channel(channel=0,startBD=4)*/
	XAieDma_TileSetStartBd((&TileDmaInst), XAIEDMA_TILE_CHNUM_MM2S0, 4);
	XAieDma_TileChControl((&TileDmaInst), XAIEDMA_TILE_CHNUM_MM2S0, XAIE_RESETDISABLE, XAIE_ENABLE);
	XAie_print("\n");

	/*self.me().tile(0,0).pl_interface().enable_pl_to_me(0)*/
	XAieTile_PlIntfDownszrEnable(&(TileInst[0][0]), 0);
	XAie_print("\n");

	/*self.me().tile(0,0).pl_interface().enable_pl_to_me(3)*/
	XAieTile_PlIntfDownszrEnable(&(TileInst[0][0]), 3);

	XAie_print("\nAXI-MM Dump for Lock APIs\n");
	/*self.me().tile(0,1).mm().locks().acquire(0, FOR_WRITE)*/
	(void)XAieTile_LockAcquire(&(TileInst[0][1]), 0, 0, 0);
	XAie_print("\n");

	/*self.me().tile(0,1).mm().locks().acquire(1, FOR_WRITE)*/
	(void)XAieTile_LockAcquire(&(TileInst[0][1]), 1, 0, 0);
	XAie_print("\n");

	/*self.me().tile(0,1).mm().locks().release(0, FOR_READ)*/
	(void)XAieTile_LockRelease(&(TileInst[0][1]), 0, 1, 0);
	XAie_print("\n");

	/*self.me().tile(0,1).mm().locks().release(1, FOR_READ)*/
	(void)XAieTile_LockRelease(&(TileInst[0][1]), 1, 1, 0);
	XAie_print("\n");

	XAie_print("\nAXI-MM Dump for the Stream Sw packet config APIs\n");

	//self.me().tile(0,0).streamswitch().configure_packet_slave_slot(rt.stream_ext_s(1), slot=0, mask=0x1f,id=0, msel=0, arbiter=0)
        slave = XAIETILE_STRSW_SPORT_SOUTH((&(TileInst[0][0])), 1);
        slot = 0;
        regval = XAIETILE_STRSW_SLVSLOT_CFG((&(TileInst[0][0])), slave, slot, 0, 0x1F, 1, 0, 0);
	XAieTile_StrmConfigSlvSlot(&(TileInst[0][0]), XAIETILE_STRSW_SPORT_SOUTH((&(TileInst[0][0])), 1), 0, 1, regval);
	XAie_print("\n");

	//self.me().tile(0,0).streamswitch().configure_packet_slave_slot(rt.stream_ext_s(5), slot=0, mask=0x1f,id=1, msel=1, arbiter=0)
        slave = XAIETILE_STRSW_SPORT_SOUTH((&(TileInst[0][0])), 5);
        slot = 0;
	regval = XAIETILE_STRSW_SLVSLOT_CFG((&(TileInst[0][0])), slave, slot, 1, 0x1F, 1, 1, 0);
	XAieTile_StrmConfigSlvSlot(&(TileInst[0][0]), XAIETILE_STRSW_SPORT_SOUTH((&(TileInst[0][0])), 5), 0, 1, regval);
	XAie_print("\n");

	//self.me().tile(0,0).streamswitch().configure_packet_master(rt.stream_ext_n(5), arbiter=0, mask=0x3, drop_header=False)
	master = XAIETILE_STRSW_MPORT_NORTH((&(TileInst[0][0])), 5);
        regval = XAIETILE_STRSW_MPORT_CFGPKT((&(TileInst[0][0])), master, 0, 0x3, 0);
	XAieTile_StrmConfigMstr(&(TileInst[0][0]), XAIETILE_STRSW_MPORT_NORTH((&(TileInst[0][0])), 5), 1, 1, regval);
	XAie_print("\n");

	//self.me().tile(0,1).streamswitch().configure_packet_slave_slot(rt.stream_ext_n(3), slot=0, mask=0x1f,id=0, msel=0, arbiter=0)
        slave = XAIETILE_STRSW_SPORT_NORTH((&(TileInst[0][1])), 3);
        slot = 0;
	regval = XAIETILE_STRSW_SLVSLOT_CFG((&(TileInst[0][1])), slave, slot, 0, 0x1F, 1, 0, 0);
	XAieTile_StrmConfigSlvSlot(&(TileInst[0][1]), XAIETILE_STRSW_SPORT_NORTH((&(TileInst[0][1])), 3), 0, 1, regval);
	XAie_print("\n");

	//self.me().tile(0,1).streamswitch().configure_packet_slave_slot(rt.stream_ext_n(3), slot=1, mask=0x1f,id=1, msel=0, arbiter=1)
        slave = XAIETILE_STRSW_SPORT_NORTH((&(TileInst[0][1])), 3);
        slot = 1;	
        regval = XAIETILE_STRSW_SLVSLOT_CFG((&(TileInst[0][1])), slave, slot, 1, 0x1F, 1, 0, 1);
	XAieTile_StrmConfigSlvSlot(&(TileInst[0][1]), XAIETILE_STRSW_SPORT_NORTH((&(TileInst[0][1])), 3), 1, 1, regval);
	XAie_print("\n");

	//self.me().tile(0,1).streamswitch().configure_packet_slave_slot(rt.stream_ext_s(5), slot=0, mask=0x1f,id=0, msel=0, arbiter=2)
        slave = XAIETILE_STRSW_SPORT_SOUTH((&(TileInst[0][1])), 5);
        slot = 0;
	regval = XAIETILE_STRSW_SLVSLOT_CFG((&(TileInst[0][1])), slave, slot, 0, 0x1F, 1, 0, 2);
	XAieTile_StrmConfigSlvSlot(&(TileInst[0][1]), XAIETILE_STRSW_SPORT_SOUTH((&(TileInst[0][1])), 5), 0, 1, regval);
	XAie_print("\n");

	//self.me().tile(0,1).streamswitch().configure_packet_slave_slot(rt.stream_ext_s(5), slot=1, mask=0x1f,id=1, msel=0, arbiter=3)
        slave = XAIETILE_STRSW_SPORT_SOUTH((&(TileInst[0][1])), 5);
        slot = 1;
	regval = XAIETILE_STRSW_SLVSLOT_CFG((&(TileInst[0][1])), slave, slot, 1, 0x1F, 1, 0, 3);
	XAieTile_StrmConfigSlvSlot(&(TileInst[0][1]), XAIETILE_STRSW_SPORT_SOUTH((&(TileInst[0][1])), 5), 1, 1, regval);
	XAie_print("\n");

	//self.me().tile(0,1).streamswitch().configure_packet_master(rt.stream_ext_n(5), arbiter=2, mask=0x1, drop_header=True)
	master = XAIETILE_STRSW_MPORT_NORTH((&(TileInst[0][1])), 5);
        regval = XAIETILE_STRSW_MPORT_CFGPKT((&(TileInst[0][1])), master, 1, 0x1, 2);
	XAieTile_StrmConfigMstr(&(TileInst[0][1]), master, 1, 1, regval);
	XAie_print("\n");

	//self.me().tile(0,1).streamswitch().configure_packet_master(rt.stream_dma(0), arbiter=3, mask=0x1, drop_header=True)
	master = XAIETILE_STRSW_MPORT_DMA((&(TileInst[0][1])), 0);
        regval = XAIETILE_STRSW_MPORT_CFGPKT((&(TileInst[0][1])), master, 1, 0x1, 3);
	XAieTile_StrmConfigMstr(&(TileInst[0][1]), master, 1, 1, regval);
	XAie_print("\n");

	//self.me().tile(0,1).streamswitch().configure_packet_master(rt.stream_ext_s(1), arbiter=0, mask=0x1, drop_header=True)
	master = XAIETILE_STRSW_MPORT_SOUTH((&(TileInst[0][1])), 1);
        regval = XAIETILE_STRSW_MPORT_CFGPKT((&(TileInst[0][1])), master, 1, 0x1, 0);
	XAieTile_StrmConfigMstr(&(TileInst[0][1]), master, 1, 1, regval);
	XAie_print("\n");

	//self.me().tile(0,1).streamswitch().configure_packet_master(rt.stream_ext_s(3), arbiter=1, mask=0x1, drop_header=True)
	master = XAIETILE_STRSW_MPORT_SOUTH((&(TileInst[0][1])), 3);
        regval = XAIETILE_STRSW_MPORT_CFGPKT((&(TileInst[0][1])), master, 1, 0x1, 1);
	XAieTile_StrmConfigMstr(&(TileInst[0][1]), master, 1, 1, regval);
	XAie_print("\n");

	//self.me().tile(0,2).streamswitch().configure_packet_slave_slot(rt.stream_dma(0), slot=0, mask=0x1f,id=0, msel=0, arbiter=0)
        slave = XAIETILE_STRSW_SPORT_DMA((&(TileInst[0][2])), 0);
        slot = 0;
	regval = XAIETILE_STRSW_SLVSLOT_CFG((&(TileInst[0][2])), slave, slot, 0, 0x1F, 1, 0, 0);
	XAieTile_StrmConfigSlvSlot(&(TileInst[0][2]), XAIETILE_STRSW_SPORT_DMA((&(TileInst[0][2])), 0), 0, 1, regval);
	XAie_print("\n");

	//self.me().tile(0,2).streamswitch().configure_packet_slave_slot(rt.stream_dma(1), slot=0, mask=0x1f,id=1, msel=1, arbiter=0)
        slave = XAIETILE_STRSW_SPORT_DMA((&(TileInst[0][2])), 1);
        slot = 0;
	regval = XAIETILE_STRSW_SLVSLOT_CFG((&(TileInst[0][2])), slave, slot, 1, 0x1F, 1, 1, 0);
	XAieTile_StrmConfigSlvSlot(&(TileInst[0][2]), XAIETILE_STRSW_SPORT_DMA((&(TileInst[0][2])), 1), 0, 1, regval);
	XAie_print("\n");

	//self.me().tile(0,2).streamswitch().configure_packet_master(rt.stream_ext_s(3), arbiter=0, mask=0x3, drop_header=False)
        master = XAIETILE_STRSW_MPORT_CTRL((&(TileInst[0][2])), 0);
        regval = XAIETILE_STRSW_MPORT_CFGPKT((&(TileInst[0][2])), master, 0, 0x3, 0);
	XAieTile_StrmConfigMstr(&(TileInst[0][2]), master, 1, 1, regval);
	XAie_print("\n");

	//self.me().tile(0,2).streamswitch().enable_packet_slave(rt.stream_dma(0))
	XAieTile_StrmConfigSlv(&(TileInst[0][2]), XAIETILE_STRSW_SPORT_DMA((&(TileInst[0][2])), 0), 1, 1);
	XAie_print("\n");

	//self.me().tile(0,2).streamswitch().enable_packet_slave(rt.stream_dma(1))
	XAieTile_StrmConfigSlv(&(TileInst[0][2]), XAIETILE_STRSW_SPORT_DMA((&(TileInst[0][2])), 1), 1, 1);
	XAie_print("\n");

	//self.me().tile(0,1).streamswitch().enable_packet_slave(rt.stream_ext_n(3))
	XAieTile_StrmConfigSlv(&(TileInst[0][1]), XAIETILE_STRSW_SPORT_NORTH((&(TileInst[0][1])), 3), 1, 1);
	XAie_print("\n");

	//self.me().tile(0,0).streamswitch().enable_packet_slave(rt.stream_ext_s(1))
	XAieTile_StrmConfigSlv(&(TileInst[0][0]), XAIETILE_STRSW_SPORT_SOUTH((&(TileInst[0][0])), 1), 1, 1);
	XAie_print("\n");

	//self.me().tile(0,0).streamswitch().enable_packet_slave(rt.stream_ext_s(5))
	XAieTile_StrmConfigSlv(&(TileInst[0][0]), XAIETILE_STRSW_SPORT_SOUTH((&(TileInst[0][0])), 5), 1, 1);
	XAie_print("\n");

	//self.me().tile(0,1).streamswitch().enable_packet_slave(rt.stream_ext_s(5))
	XAieTile_StrmConfigSlv(&(TileInst[0][1]), XAIETILE_STRSW_SPORT_SOUTH((&(TileInst[0][1])), 5), 1, 1);
	XAie_print("\n");

        (void)XAieTile_CoreWaitDone(&(TileInst[0][1]), 1000*1000);
        (void)XAieTile_CoreWaitCycles(&(TileInst[0][1]), 1000);

        XAie_print("\nDM write API tests\n");
        XAieTile_DmWriteWord(&(TileInst[3][3]), 0x0, 0x1);
        XAieTile_DmWriteWord(&(TileInst[3][3]), 0x8, 0x0);

	XAie_print("\nEnd of startup.py commands\n\n");

	/***********************************************************
	 * End of sample startup.py
	 **********************************************************/
}
#endif

/** @} */

