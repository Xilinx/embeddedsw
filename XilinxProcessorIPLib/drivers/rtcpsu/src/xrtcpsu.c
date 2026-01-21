/******************************************************************************
* Copyright (C) 2015 - 2021 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2023 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
 *
 * @file xrtcpsu.c
 * @addtogroup rtcpsu Overview
 * @{
 *
 * Functions in this file are the minimum required functions for the XRtcPsu
 * driver. See xrtcpsu.h for a detailed description of the driver.
 *
 * @notice	None.
 *
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who    Date	Changes
 * ----- -----  -------- -----------------------------------------------
 * 1.00  kvn    04/21/15 First release
 * 1.1   kvn    09/25/15 Modify control register to enable battery
 *                       switching when vcc_psaux is not available.
 * 1.2          02/15/16 Corrected Calibration mask and Fractional
 *                       mask in CalculateCalibration API.
 * 1.3   vak    04/25/16 Corrected the RTC read and write time logic(cr#948833).
 * 1.5   ms     08/27/17 Fixed compilation warnings.
 *       ms     08/29/17 Updated code as per source code style.
 * 1.6	 aru	06/25/18 Modified logic to handle
 *			 the last day of month cotrrecly.(CR#1004282)
 * 1.6	 aru	06/25/18 Remove the checkpatch warnings.
 * 1.6   aru    07/11/18 Resolved cppcheck warnings.
 * 1.6   aru    07/11/18 Resolved doxygen warnings.
 * 1.6   aru    08/17/18 Resolved MISRA-C mandatory violations.(CR#1007752)
 * 1.7   sne    03/01/19 Added Versal support.
 * 1.7   sne    03/01/19 Fixed violations according to MISRAC-2012 standards
 *                       modified the code such as
 *                       No brackets to loop body,Declared the pointer param
 *                       as Pointer to const,No brackets to then/else,
 *                       Literal value requires a U suffix,Casting operation to a pointer
 *			 Array has no bounds specified,Logical conjunctions need brackets.
 * 1.8	 sg	07/13/19 Corrected calibration algorithm
 * 1.13	 ht	06/22/23 Added support for system device-tree flow.
 * 1.16  ht	09/26/25 Remove redundant calibration register write in XRtcPsu_SetTime.
 * 1.17  ht	01/19/26 Modify XRtcPsu_SecToDateTime, XRtcPsu_DateTimeToSec to
 *                       use 1970-01-01 as reference.
 * </pre>
 *
 ******************************************************************************/

/***************************** Include Files *********************************/

#include "xrtcpsu.h"
#include "xrtcpsu_hw.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Variable Definitions *****************************/

static const u32 DaysInMonth[] = {31, 28, 31, 30, 31,
				  30, 31, 31, 30, 31, 30, 31
				 };

/************************** Function Prototypes ******************************/

static void XRtcPsu_StubHandler(void *CallBackRef, u32 Event);

/*****************************************************************************/
/**
 *
 * This function initializes a XRtcPsu instance/driver.
 *
 * The initialization entails:
 * - Initialize all members of the XRtcPsu structure.
 *
 * @param	InstancePtr is a pointer to the XRtcPsu instance.
 * @param	ConfigPtr points to the XRtcPsu device configuration structure.
 * @param	EffectiveAddr is the device base address in the virtual memory
 *		address space. If the address translation is not used then the
 *		physical address is passed.
 *		Unexpected errors may occur if the address mapping is changed
 *		after this function is invoked.
 *
 * @return	XST_SUCCESS always.
 *
 * @note		None.
 *
 ******************************************************************************/
s32 XRtcPsu_CfgInitialize(XRtcPsu *InstancePtr, XRtcPsu_Config *ConfigPtr,
			  UINTPTR EffectiveAddr)
{
	s32 Status;
	u32 ControlRegister;

	Xil_AssertNonvoid(InstancePtr != NULL);

	Xil_AssertNonvoid(ConfigPtr != NULL);

	/*
	 * Set some default values for instance data, don't indicate the device
	 * is ready to use until everything has been initialized successfully.
	 */
	InstancePtr->IsReady = 0U;
	InstancePtr->RtcConfig.BaseAddr = EffectiveAddr;
#ifndef SDT
	InstancePtr->RtcConfig.DeviceId = ConfigPtr->DeviceId;
#endif

	if (InstancePtr->OscillatorFreq == 0U) {
		InstancePtr->CalibrationValue = XRTC_CALIBRATION_VALUE;
		InstancePtr->OscillatorFreq = XRTC_TYPICAL_OSC_FREQ;
	}

	/* Set all handlers to stub values, let user configure this
	 * data later.
	 */
	InstancePtr->Handler = (XRtcPsu_Handler)XRtcPsu_StubHandler;

	InstancePtr->IsPeriodicAlarm = 0U;

	/* Set the calibration value in calibration register. */
	XRtcPsu_WriteReg(InstancePtr->RtcConfig.BaseAddr + XRTC_CALIB_WR_OFFSET,
			 InstancePtr->CalibrationValue);

	/*	Set the Oscillator crystal and Battery switch enable
	 *	in control register.
	 */
	ControlRegister = XRtcPsu_ReadReg(InstancePtr->RtcConfig.BaseAddr
					  + XRTC_CTL_OFFSET);
	XRtcPsu_WriteReg(InstancePtr->RtcConfig.BaseAddr + XRTC_CTL_OFFSET,
			 (ControlRegister | (u32)XRTCPSU_CRYSTAL_OSC_EN |
			  (u32)XRTC_CTL_BATTERY_EN_MASK));

	/* Clear the Interrupt Status and Disable the interrupts. */
	XRtcPsu_WriteReg(InstancePtr->RtcConfig.BaseAddr + XRTC_INT_STS_OFFSET,
			 ((u32)XRTC_INT_STS_ALRM_MASK |
			  (u32)XRTC_INT_STS_SECS_MASK));
	XRtcPsu_WriteReg(InstancePtr->RtcConfig.BaseAddr + XRTC_INT_DIS_OFFSET,
			 ((u32)XRTC_INT_DIS_ALRM_MASK |
			  (u32)XRTC_INT_DIS_SECS_MASK));

	/* Indicate the component is now ready to use. */
	InstancePtr->IsReady = XIL_COMPONENT_IS_READY;

	/* Clear TimeUpdated and CurrTimeUpdated */
	InstancePtr->TimeUpdated = (u32)0U;
	InstancePtr->CurrTimeUpdated = (u32)0U;

	Status = (s32)XST_SUCCESS;
	return Status;
}

/****************************************************************************/
/**
 *
 * This function is a stub handler that is the default handler such that if the
 * application has not set the handler when interrupts are enabled, this
 * function will be called.
 *
 * @param	CallBackRef is unused by this function.
 * @param	Event is unused by this function.
 *
 * @return	None.
 *
 * @note		None.
 *
 *****************************************************************************/
static void XRtcPsu_StubHandler(void *CallBackRef, u32 Event)
{
	(void) CallBackRef;
	(void) Event;
	/* Assert occurs always since this is a stub and should
	 * never be called
	 */
	Xil_AssertVoidAlways();
}

/****************************************************************************/
/**
 *
 * This function sets the RTC time by writing into rtc write register.
 *
 * @param	InstancePtr is a pointer to the XRtcPsu instance.
 * @param	Time that should be updated into RTC write register.
 *
 * @return	None.
 *
 * @note		None.
 *
 *****************************************************************************/
void XRtcPsu_SetTime(XRtcPsu *InstancePtr, u32 Time)
{
	/* clear the RTC secs interrupt from status register */
	XRtcPsu_WriteReg(InstancePtr->RtcConfig.BaseAddr + XRTC_INT_STS_OFFSET,
			 XRTC_INT_STS_SECS_MASK);
	InstancePtr->CurrTimeUpdated = (u32)0U;
	/* Update the flag before setting the time */
	InstancePtr->TimeUpdated = (u32)1U;
	/* Since RTC takes 1 sec to update the time into current
	 * time register, write
	 * load time + 1sec into the set time register.
	 */
	XRtcPsu_WriteSetTime(InstancePtr, Time + (u32)1U);
}

/****************************************************************************/
/**
 *
 * This function gets the current RTC time.
 *
 * @param	InstancePtr is a pointer to the XRtcPsu instance.
 *
 * @return	RTC Current time.
 *
 * @note		None.
 *
 *****************************************************************************/
u32 XRtcPsu_GetCurrentTime(XRtcPsu *InstancePtr)
{
	u32 Status;
	u32 IntMask;
	u32 CurrTime;

	IntMask = XRtcPsu_ReadReg(InstancePtr->RtcConfig.BaseAddr +
				  XRTC_INT_MSK_OFFSET);

	if ((IntMask & XRTC_INT_STS_SECS_MASK) != (u32)0) {
		/* We come here if interrupts are disabled */
		Status = XRtcPsu_ReadReg(InstancePtr->RtcConfig.BaseAddr +
					 XRTC_INT_STS_OFFSET);
		if ((InstancePtr->TimeUpdated == (u32)1) &&
		    ((Status & XRTC_INT_STS_SECS_MASK) == (u32)0)) {
			/* Give the previous written time */
			CurrTime = XRtcPsu_GetLastSetTime(InstancePtr) - (u32)1;
		} else {
			/* Clear TimeUpdated */
			if ((InstancePtr->TimeUpdated == (u32)1) &&
			    ((Status & XRTC_INT_STS_SECS_MASK) == (u32)1)) {
				InstancePtr->TimeUpdated = (u32)0;
			}

			/* RTC time got updated */
			CurrTime = XRtcPsu_ReadCurrentTime(InstancePtr);
		}
	} else {
		/* We come here if interrupts are enabled */
		if ((InstancePtr->TimeUpdated == (u32)1) &&
		    (InstancePtr->CurrTimeUpdated == (u32)0)) {
			/* Give the previous written time -1 sec */
			CurrTime = XRtcPsu_GetLastSetTime(InstancePtr) - (u32)1;
		} else {
			/* Clear TimeUpdated */
			if (InstancePtr->TimeUpdated == (u32)1) {
				InstancePtr->TimeUpdated = (u32)0;
			}
			/* RTC time got updated */
			CurrTime = XRtcPsu_ReadCurrentTime(InstancePtr);
		}
	}
	return CurrTime;
}

/****************************************************************************/
/**
 *
 * This function sets the alarm value of RTC device.
 *
 * @param	InstancePtr is a pointer to the XRtcPsu instance
 * @param	Alarm is the desired alarm time for RTC.
 * @param	Periodic says whether the alarm need to set at periodic
 *		Intervals or a one-time alarm.
 *
 * @return	None.
 *
 * @note		None.
 *
 *****************************************************************************/
void XRtcPsu_SetAlarm(XRtcPsu *InstancePtr, u32 Alarm, u32 Periodic)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(Alarm != 0U);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertVoid((Alarm - XRtcPsu_GetCurrentTime(InstancePtr)) > (u32)0);

	XRtcPsu_WriteReg(InstancePtr->RtcConfig.BaseAddr + XRTC_ALRM_OFFSET,
			 Alarm);
	if (Periodic != 0U) {
		InstancePtr->IsPeriodicAlarm = 1U;
		InstancePtr->PeriodicAlarmTime =
			Alarm - XRtcPsu_GetCurrentTime(InstancePtr);
	}
}


/****************************************************************************/
/**
 *
 * This function translates time in seconds to a YEAR:MON:DAY HR:MIN:SEC
 * format and saves it in the DT structure variable.
 * It also reports the weekday.
 *
 * @param	Seconds is the time value that has to be shown in DateTime
 *		format.
 * @param	dt is the DateTime format variable that stores the translated
 *		time.
 *
 * @return	None.
 *
 * @note	This API uses Unix epoch (1970-01-01 00:00:00 UTC) as reference.
 *		Supports years from 1970 to 2106 (unsigned 32-bit limit).
 *
 *		WEEKDAY CONVENTION:
 *		WeekDay field follows POSIX standard:
 *		  0=Sunday, 1=Monday, 2=Tuesday, 3=Wednesday,
 *		  4=Thursday, 5=Friday, 6=Saturday
 *		1970-01-01 (epoch) was Thursday, so WeekDay = 4.
 *
 *		For u32 input, overflow is impossible. Maximum timestamp
 *		(0xFFFFFFFF) correctly converts to 2106-02-07 06:28:15.
 *
 *****************************************************************************/
void XRtcPsu_SecToDateTime(u32 Seconds, XRtcPsu_DT *dt)
{
	u32 CurrentTime;
	u32 TempDays;
	u32 DaysPerMonth;
	u32 Leap = 0U;

	Xil_AssertVoid(dt != NULL);

	/* Extract time-of-day components */
	CurrentTime = Seconds;
	dt->Sec = CurrentTime % 60U;
	CurrentTime /= 60U;
	dt->Min = CurrentTime % 60U;
	CurrentTime /= 60U;
	dt->Hour = CurrentTime % 24U;
	TempDays = CurrentTime / 24U;

	/* Calculate weekday: POSIX convention (0=Sunday, 4=Thursday) */
	dt->WeekDay = (TempDays + EPOCH_WDAY) % 7U;

	/* Calculate year by iterating from epoch (max 136 iterations) */
	dt->Year = EPOCH_YEAR;
	while (1) {
		Leap = IS_LEAP_YEAR(dt->Year) ? 1U : 0U;

		if (TempDays < (365U + Leap))
			break;

		TempDays -= (365U + Leap);
		dt->Year++;
	}

	/* Calculate month: TempDays is 0-indexed (0=Jan 1st, 30=Jan 31st) */
	dt->Month = 1U;
	while (1) {
		DaysPerMonth = DaysInMonth[dt->Month - 1U];

		/* Adjust February for leap years */
		if ((Leap == 1U) && (dt->Month == 2U))
			DaysPerMonth++;

		if (TempDays < DaysPerMonth)
			break;

		TempDays -= DaysPerMonth;
		dt->Month++;
	}

	/* Convert 0-indexed day to 1-indexed day-of-month */
	dt->Day = TempDays + 1U;
}

/****************************************************************************/
/**
 *
 * This function translates time in YEAR:MON:DAY HR:MIN:SEC format to
 * seconds since Unix epoch (1970-01-01 00:00:00 UTC).
 *
 * @param	dt is a pointer to a DateTime format structure variable
 *		of time that has to be converted to seconds.
 *
 * @return	Seconds value since Unix epoch (1970-01-01).
 *
 * @note	This API uses Unix epoch (1970-01-01 00:00:00 UTC) as reference.
 *		Input must be in range 1970-2106. The input structure is not
 *		modified. This is the inverse of XRtcPsu_SecToDateTime().
 *
 *****************************************************************************/
u32 XRtcPsu_DateTimeToSec(XRtcPsu_DT *dt)
{
	u32 Year;
	u32 Month;
	u32 TotalDays;
	u32 DaysInYear;
	u32 Seconds;
	u32 i;

	Xil_AssertNonvoid(dt != NULL);

	Year = dt->Year;
	Month = dt->Month;

	/* Calculate total days from epoch (1970-01-01) to start of target year */
	TotalDays = 0U;
	for (i = EPOCH_YEAR; i < Year; i++) {
		DaysInYear = IS_LEAP_YEAR(i) ? 366U : 365U;
		TotalDays += DaysInYear;
	}

	/* Add days for complete months in target year */
	for (i = 1U; i < Month; i++) {
		TotalDays += DaysInMonth[i - 1U];

		/* Add leap day if February and target year is leap */
		if ((i == 2U) && IS_LEAP_YEAR(Year))
			TotalDays++;
	}

	/* Add day of month (subtract 1 because Day is 1-indexed) */
	TotalDays += (dt->Day - 1U);

	/* Convert to seconds */
	Seconds = (((((TotalDays * 24U) + dt->Hour) * 60U) + dt->Min) * 60U)
		  + dt->Sec;

	return Seconds;
}

/****************************************************************************/
/**
 *
 * This function calculates the calibration value depending on the actual
 * realworld time and also helps in deriving new calibration value if
 * the user wishes to change his oscillator frequency. TimeReal should be
 * the accurate reference time in seconds since Unix epoch (1970-01-01
 * 00:00:00 UTC). This matches standard internet time sources (NTP, GPS, etc.)
 * and can be used directly without conversion.
 *
 * @param	InstancePtr is a pointer to the XRtcPsu instance.
 * @param	TimeReal is the actual realworld time in seconds since Unix
 *		epoch (1970-01-01). This can be obtained from network time
 *		protocols (NTP) or other accurate time sources. The value
 *		must be greater than the RTC's current time for calibration
 *		to work correctly.
 *
 * @param	CrystalOscFreq is the Oscillator new frequency.
 *		If using the typical 32768Hz crystal, then input
 *		the same frequency value.
 *
 * @return	None.
 *
 * @note	After calculating the calibration register value,
 *		user / application must call CfgInitialize API again
 *		to apply the new calibration into effect.
 *
 *****************************************************************************/
void XRtcPsu_CalculateCalibration(XRtcPsu *InstancePtr, u32 TimeReal,
				  u32 CrystalOscFreq)
{
	u32 ReadTime;
	u32 SetTime;
	u32 Cprev;
	u32 Fprev;
	u32 Cnew;
	u32 Fnew;
	u32 Calibration;

	Xil_AssertVoid(TimeReal != 0U);

	Xil_AssertVoid(CrystalOscFreq != 0U);

	ReadTime = XRtcPsu_GetCurrentTime(InstancePtr);
	SetTime = XRtcPsu_GetLastSetTime(InstancePtr);
	Calibration = XRtcPsu_GetCalibration(InstancePtr);
	/*
	 * When board gets reset, Calibration value is zero
	 * and Last setTime will be marked as 1st  second. This implies
	 * CurrentTime to be in few seconds say something in tens. TimeReal will
	 * be huge, say something in thousands. So to prevent
	 * such reset case, Cnew
	 * and Fnew will not be calculated.
	 */
	if ((Calibration == 0U) ||
	    (CrystalOscFreq != InstancePtr->OscillatorFreq)) {
		Cnew = CrystalOscFreq - (u32)1;
		Fnew = 0U;
	} else {
		float Xf;
		Cprev = Calibration & XRTC_CALIB_RD_MAX_TCK_MASK;
		Fprev = (Calibration & XRTC_CALIB_RD_FRACTN_DATA_MASK) >>
			XRTC_CALIB_RD_FRACTN_DATA_SHIFT;

		Xf = (float)((ReadTime - SetTime) / (TimeReal - SetTime));
		Xf = Xf * (float)((Cprev + 1U) + ((Fprev + 1U) / 16U));

		Cnew = (u32)(Xf) - (u32)1;
		Fnew = XRtcPsu_RoundOff((Xf - (u32)Xf) * (float)16U) - (u32)1;
	}

	Calibration = (Fnew << XRTC_CALIB_RD_FRACTN_DATA_SHIFT) + Cnew;
	Calibration |= XRTC_CALIB_RD_FRACTN_EN_MASK;

	InstancePtr->CalibrationValue = Calibration;
	InstancePtr->OscillatorFreq = CrystalOscFreq;
}

/****************************************************************************/
/**
 *
 * This function returns the seconds event status by reading
 * interrupt status register.
 *
 * @param	InstancePtr is a pointer to the XRtcPsu instance.
 *
 * @return	Returns 1 if a new second event is generated.Else 0..
 *
 * @note		This API is used in polled mode operation of RTC.
 *			This also clears interrupt status seconds bit.
 *
 *****************************************************************************/
u32 XRtcPsu_IsSecondsEventGenerated(XRtcPsu *InstancePtr)
{
	u32 Status;

	/* Loop the interrupt status register for Seconds Event */
	if ((XRtcPsu_ReadReg(InstancePtr->RtcConfig.BaseAddr +
			     XRTC_INT_STS_OFFSET) & (XRTC_INT_STS_SECS_MASK)) == 0U) {
		Status = 0U;
	} else {
		/* Clear the interrupt status register */
		XRtcPsu_WriteReg((InstancePtr)->RtcConfig.BaseAddr +
				 XRTC_INT_STS_OFFSET, XRTC_INT_STS_SECS_MASK);
		Status = 1U;
	}
	return Status;
}

/****************************************************************************/
/**
 *
 * This function returns the alarm event status by reading
 * interrupt status register.
 *
 * @param	InstancePtr is a pointer to the XRtcPsu instance.
 *
 * @return	Returns 1 if the alarm event is generated.Else 0.
 *
 * @note	This API is used in polled mode operation of RTC.
 *		This also clears interrupt status alarm bit.
 *
 *****************************************************************************/
u32 XRtcPsu_IsAlarmEventGenerated(XRtcPsu *InstancePtr)
{
	u32 Status;

	/* Loop the interrupt status register for Alarm Event */
	if ((XRtcPsu_ReadReg(InstancePtr->RtcConfig.BaseAddr +
			     XRTC_INT_STS_OFFSET) & (XRTC_INT_STS_ALRM_MASK)) == 0U) {
		Status = 0U;
	} else {
		/* Clear the interrupt status register */
		XRtcPsu_WriteReg((InstancePtr)->RtcConfig.BaseAddr +
				 XRTC_INT_STS_OFFSET, XRTC_INT_STS_ALRM_MASK);
		Status = 1U;
	}
	return Status;
}
/** @} */
