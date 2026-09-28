/******************************************************************************
 * Copyright (c) 2026 Advanced Micro Devices, Inc. All Rights Reserved.
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

/**
 * @file vek385_i2c_bus.c
 * @brief VEK385 I2C1 controller setup and byte register access for PMBUS1.
 *
 * Owns the XIicPs instance for I2C1 and the cached PMBUS1 ownership state.
 * The PCA9541 arbiter and PCA9848 mux protocol lives in pca954x.c, driven
 * from the acquire and release paths here.
 */
#include "board_vek385_vmon.h"
#include "vek385_i2c_bus.h"
#include "pca954x.h"
#include "sleep.h"
#include "xiicps.h"
#include "xil_printf.h"
#include "xparameters.h"
#include "xstatus.h"

/** @brief XIicPs driver instance for I2C1. */
static XIicPs IicInst;

/** @brief Non-zero when PCA9541 bus ownership is held. */
static u8 BusAcquired;

/** @brief Non-zero when PCA9848 mux is already pointed at PMBUS1. */
static u8 MuxSelected;

/** @brief Manual XIicPs config when BSP has no entry for I2C1. */
static XIicPs_Config IicManualCfg = {
	.BaseAddress = VEK385_I2C1_BASE_ADDR,
	.InputClockHz = VEK385_I2C_REF_CLK_HZ,
};

/**
 * @brief Wait until I2C bus is idle.
 *
 * @return XST_SUCCESS when idle, XST_FAILURE on timeout.
 */
static XStatus Vek385_I2cIdleWait(void)
{
	u32 Timeout = VEK385_I2C_IDLE_POLL_MAX;
	XStatus Status = XST_FAILURE;

	while (0U != XIicPs_BusIsBusy(&IicInst)) {
		usleep(VEK385_I2C_IDLE_POLL_US);
		if (0U == Timeout--) {
			goto done;
		}
	}

	Status = XST_SUCCESS;

done:
	return Status;
}

/**
 * @brief Thin wrapper over the esw polled I2C write.
 *
 * esw's XIicPs_MasterSendPolled() does the transfer. This wrapper adds two
 * things. It retries when the transfer loses arbitration (XST_IIC_ARB_LOST).
 * That happens when the System Controller takes the shared PMBUS1
 * mid-transfer. The retry count is capped so we cannot spin forever. It also
 * waits for an idle bus if one of our own repeated-start reads still holds
 * it. That is our own state, not the SC.
 *
 * @param SlaveAddr 7-bit slave address.
 * @param Buf Data buffer.
 * @param Len Number of bytes to send.
 *
 * @return XST_SUCCESS on success, otherwise an XStatus error code. A loss of
 *         the shared bus surfaces as XST_IIC_ARB_LOST.
 */
XStatus Vek385_I2cWriteRaw(const u8 SlaveAddr, const u8 *Buf, const s32 Len)
{
	u32 Retry = 0U;
	XStatus Status = XST_FAILURE;

	do {
		if (0U != IicInst.IsRepeatedStart) {
			Status = Vek385_I2cIdleWait();
			if (XST_SUCCESS != Status) {
				goto done;
			}
		}

		/*
		 * XIicPs_MasterSendPolled() takes a non-const u8 *, so the
		 * cast is forced by the driver signature. The buffer is not
		 * written by the send path.
		 */
		Status = XIicPs_MasterSendPolled(&IicInst, (u8 *)Buf, Len,
						 SlaveAddr);
		if (XST_IIC_ARB_LOST != Status) {
			break;
		}
		usleep(VEK385_I2C_ARB_LOST_DELAY_US);
	} while (++Retry < VEK385_I2C_ARB_LOST_RETRY_MAX);

	if (XST_IIC_ARB_LOST == Status) {
		xil_printf("ERROR: I2C write lost arbitration %u times, "
			   "giving up\r\n", Retry);
	}

done:
	return Status;
}

/**
 * @brief Initialize I2C1 controller (XIicPs) for VEK385.
 *
 * @return XST_SUCCESS on success, otherwise an XStatus error code.
 */
XStatus Vek385_I2cBusInit(void)
{
	XStatus Status = XST_FAILURE;
	XIicPs_Config *Cfg = NULL;

	if ((u32)XIL_COMPONENT_IS_READY == IicInst.IsReady) {
		Status = XST_SUCCESS;
		goto done;
	}

	/*
	 * Look up I2C1 by board base, not XPAR index. The SDT BSP exposes
	 * i2c8 @ 0xF1000000 as XIICPS_0, but PMBUS1 (PCA9541 @ 0x72) is on
	 * i2c1 @ 0xF1950000. Using the wrong instance NACKs every transaction.
	 */
	Cfg = XIicPs_LookupConfig(VEK385_I2C1_BASE_ADDR);
	if (NULL == Cfg) {
		Cfg = &IicManualCfg;
		VMON_DBG("DBG: no BSP XIicPs entry for 0x%08x, using manual config\r\n",
			 VEK385_I2C1_BASE_ADDR);
	}
	VMON_DBG("DBG: XIicPs base=0x%08x clk=%u Hz\r\n",
		 (u32)Cfg->BaseAddress, Cfg->InputClockHz);

	Status = XIicPs_CfgInitialize(&IicInst, Cfg, Cfg->BaseAddress);
	if (XST_SUCCESS != Status) {
		xil_printf("ERROR: XIicPs_CfgInitialize failed status=%d\r\n", Status);
		goto done;
	}

	XIicPs_Reset(&IicInst);
	Status = XIicPs_SetSClk(&IicInst, VEK385_I2C_SCLK_HZ);
	if (XST_SUCCESS != Status) {
		xil_printf("ERROR: XIicPs_SetSClk(%u) failed status=%d\r\n",
			   VEK385_I2C_SCLK_HZ, Status);
		goto done;
	}

	VMON_DBG("DBG: I2C1 ready @%u Hz\r\n", VEK385_I2C_SCLK_HZ);
	BusAcquired = 0U;
	MuxSelected = 0U;
	Status = XST_SUCCESS;

done:
	return Status;
}

/**
 * @brief Release PCA9541 bus ownership if acquired.
 */
void Vek385_I2cBusRelease(void)
{
	if (0U != BusAcquired) {
		(void)Pca9541_Release();
		BusAcquired = 0U;
		MuxSelected = 0U;

		/* Mark the end of one acquire/poll/release cycle in the log. */
		VMON_DBG("===================\r\n");
	}
}

/**
 * @brief Acquire I2C1 arbiter and select PMBUS1 on PCA9848 mux.
 *
 * @return XST_SUCCESS on success, otherwise an XStatus error code. A loss of
 *         the shared bus surfaces as XST_IIC_ARB_LOST.
 */
XStatus Vek385_I2cAcquirePmbus1(void)
{
	u8 Ctl = 0U;
	u8 Owned = 0U;
	XStatus Status = XST_FAILURE;

	Status = Vek385_I2cBusInit();
	if (XST_SUCCESS != Status) {
		goto done;
	}

	/*
	 * Do not trust a cached "we own it" flag, because the SC can force PMBUS1
	 * away at any time. Ask the arbiter every call, since its CONTROL is
	 * always reachable. If the bus is off or the SC holds it, drop stale
	 * state and re-acquire.
	 */
	Status = Pca9541_CheckOwnership(&Owned, &Ctl);
	if (XST_SUCCESS != Status) {
		xil_printf("ERROR: PCA9541 CONTROL read failed status=%d\r\n",
			   Status);
		goto done;
	}

	if (0U == Owned) {
		if (0U != BusAcquired) {
			xil_printf("WARN: PMBUS1 taken by other master "
				   "(CTL=0x%02x), re-acquiring\r\n", Ctl);
		}
		/*
		 * We lost (or never had) the bus, so the downstream mux state
		 * is unknown too: force a re-select after re-acquiring.
		 */
		BusAcquired = 0U;
		MuxSelected = 0U;

		Status = Pca9541_Acquire();
		if (XST_SUCCESS != Status) {
			xil_printf("ERROR: PCA9541 arbiter acquire failed\r\n");
			goto done;
		}
	}

	/*
	 * We own the bus now, either by the fast path or a fresh acquire. Mark
	 * it in both cases. Marking only on the acquire path would make the next
	 * release a no-op and leave us sitting on PMBUS1 for good.
	 */
	BusAcquired = 1U;

	if (0U == MuxSelected) {
		Status = Pca9848_Select(VEK385_I2C1_MUX_SELECT);
		if (XST_SUCCESS != Status) {
			xil_printf("ERROR: PCA9848 @0x%02x ch%u select failed status=%d\r\n",
				   VEK385_I2C1_MUX_ADDR, VEK385_I2C1_MUX_CH_PMBUS1, Status);
			goto done;
		}
		MuxSelected = 1U;

		VMON_DBG("DBG: PCA9848 @0x%02x -> PMBUS1 (mask=0x%02x)\r\n",
			 VEK385_I2C1_MUX_ADDR, VEK385_I2C1_MUX_SELECT);
	}

	Status = XST_SUCCESS;

done:
	return Status;
}

/**
 * @brief Read one byte from an I2C slave register while holding PMBUS1.
 *
 * The caller must already own PMBUS1 (via Vek385_I2cAcquirePmbus1).
 * This does not acquire, so a bank-select and this read can run on one
 * ownership without a re-acquire re-selecting the mux in between.
 *
 * @param SlaveAddr 7-bit I2C slave address.
 * @param Reg Register address.
 * @param Val Pointer to receive register value.
 *
 * @return XST_SUCCESS on success, otherwise an XStatus error code. A loss of
 *         the shared bus surfaces as XST_IIC_ARB_LOST.
 */
XStatus Vek385_I2cRegReadLocked(const u8 SlaveAddr, u8 Reg, u8 *const Val)
{
	u32 Retry = 0U;
	XStatus Status = XST_FAILURE;

	/*
	 * SMBus read-byte: write the register, repeated START, then read with a
	 * closing STOP, all in one transfer. The part resets its pointer on a
	 * STOP. The held repeated-start keeps the bus busy on purpose, so do NOT
	 * idle-wait between the write and the read, only before and after. Retry
	 * the whole sequence on arbitration loss, bounded like the write path,
	 * since the SC can grab PMBUS1 mid-transfer.
	 */
	do {
		Status = Vek385_I2cIdleWait();
		if (XST_SUCCESS != Status) {
			goto done;
		}

		(void)XIicPs_SetOptions(&IicInst, XIICPS_REP_START_OPTION);
		Status = XIicPs_MasterSendPolled(&IicInst, &Reg, 1, SlaveAddr);
		(void)XIicPs_ClearOptions(&IicInst, XIICPS_REP_START_OPTION);
		if (XST_IIC_ARB_LOST == Status) {
			usleep(VEK385_I2C_ARB_LOST_DELAY_US);
			continue;
		}
		if (XST_SUCCESS != Status) {
			goto done;
		}

		Status = XIicPs_MasterRecvPolled(&IicInst, Val, 1, SlaveAddr);
		if (XST_IIC_ARB_LOST != Status) {
			break;
		}
		usleep(VEK385_I2C_ARB_LOST_DELAY_US);
	} while (++Retry < VEK385_I2C_ARB_LOST_RETRY_MAX);

	if (XST_IIC_ARB_LOST == Status) {
		xil_printf("ERROR: I2C read lost arbitration %u times, "
			   "giving up\r\n", Retry);
		goto done;
	}
	if (XST_SUCCESS != Status) {
		goto done;
	}

	Status = Vek385_I2cIdleWait();

done:
	return Status;
}
