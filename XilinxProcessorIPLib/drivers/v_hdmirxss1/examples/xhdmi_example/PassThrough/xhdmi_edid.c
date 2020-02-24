/******************************************************************************
* Copyright (C) 2018 – 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xhdmi_edid.c
*
* This file demonstrates application usage for EDID
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who    Date     Changes
* ----- ------ -------- --------------------------------------------------
* 1.00         22/05/18 Initial release.
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/
#include "xhdmi_edid.h"

/***************** Macros (Inline Functions) Definitions *********************/


/**************************** Type Definitions *******************************/


/************************** Function Prototypes ******************************/


/************************** Function Definitions *****************************/
#ifdef XPAR_XV_HDMITXSS1_NUM_INSTANCES
/*****************************************************************************/
/**
*
* This function handles EDID reading and accessing the SCDC register during
* cable connect state
*
* @param  HdmiTxSs1Ptr HDMI TX Subsystem Pointer
* @param  CheckHdmi20Param Application Data Structure (EDID & HDMI 2.0 Check)
*
* @return
*   -NA
*
* @note   None.
*
******************************************************************************/
void EdidScdcCheck(XV_HdmiTxSs1          *HdmiTxSs1Ptr,
                   EdidHdmi           *CheckHdmiParam)
{
    u8 Buffer [256];
    int Status;

    /*Below Check executed only when TX Cable Connect*/
    if (CheckHdmiParam->EdidCableConnectRead) {
	/*Read & Parse the EDID upon the Cable Connect to check
		Sink's Capability*/
	Status = XV_HdmiTxSs1_ReadEdid(HdmiTxSs1Ptr, (u8*)&Buffer);
	/* Only Parse the EDID when the Read EDID success */
	if (Status == XST_SUCCESS) {
		XV_VidC_parse_edid((u8*)&Buffer,
				&CheckHdmiParam->EdidCtrlParam,
				XVIDC_VERBOSE_DISABLE);
	}

        /*Check whether the Sink able to support HDMI 2.0 by checking the
        maximum supported video bandwidth*/
        if (CheckHdmiParam->EdidCtrlParam.MaxTmdsMhz >= 340) {
            /*Sink's EDID indicated HDMI 2.0 Capable*/
            CheckHdmiParam->IsHDMI20SinkCapable = (TRUE);

            /*Check whether the SCDC is present or not (HF-VSDB EDID)*/
            if(CheckHdmiParam->EdidCtrlParam.IsSCDCPresent ==
															XVIDC_SUPPORTED) {
                /*Check whether the SCDC is capable of initiating
				Read Request*/
//                if (CheckHdmi20Param->EdidCtrlParam.IsSCDCReadRequestReady ==
//                        XVIDC_SUPPORTED) {
                    Status = XV_HdmiTxSs1_DetectHdmi20(HdmiTxSs1Ptr);
                    if (!Status) {
                        CheckHdmiParam->IsReReadSinkEdid = (FALSE);
                    } else {
                        /*Fail: EDID and the Actual behavior mismatch*/
                        CheckHdmiParam->IsReReadSCDC = (TRUE);
                        CheckHdmiParam->HdmiSinkWarningFlag |=
                        		XV_HDMI_SINK_EDID_20_VSDB20_ACC_SCDC_FAIL;
                    }
//                } else {
//                    /*SCDC register is not ready for initiating read request*/
//                    CheckHdmi20Param->IsReReadSinkEdid = (TRUE);
//                }
            } else {
            	CheckHdmiParam->HdmiSinkWarningFlag |=
            			XV_HDMI_SINK_EDID_20_VSDB20_NA_SCDC_PASS;

                /*Since the SCDC is not present, hence doesn't require,
                re-reading of the EDID.*/
                CheckHdmiParam->IsReReadSinkEdid = (FALSE);

                /*Try to access the SCDC even the EDID declared
                SCDC not present, since it's a HDMI 2.0 capable
                Notes:
                Some commercial sink's has behavior of no EDID:HF-VSDB
                but the SCDC is accessible*/
                Status = XV_HdmiTxSs1_DetectHdmi20(HdmiTxSs1Ptr);
                if (Status) {
                    CheckHdmiParam->IsReReadSCDC = (TRUE);
                }
            }
        } else {
            /*Check whether the SCDC is accessible even though
              the EDID indicates non HDMI 2.0 Capable
              Notes:
              Some commercial sink's has behavior of when the
              EDID declared "HDMI2.0 not supported" but the SCDC are
              accessible*/
            Status = XV_HdmiTxSs1_DetectHdmi20(HdmiTxSs1Ptr);
            if (!Status) {
                /*Since the SCDC is accessible assume the Sink can support
                HDMI 2.0*/
                CheckHdmiParam->IsHDMI20SinkCapable = (TRUE);
                CheckHdmiParam->HdmiSinkWarningFlag |=
                		XV_HDMI_SINK_EDID_14_SCDC_PASS;
            } else {
				/*The Sink are HDMI 1.4*/
				CheckHdmiParam->IsHDMI20SinkCapable = (FALSE);
				CheckHdmiParam->HdmiSinkWarningFlag |=
						XV_HDMI_SINK_20_NOT_CAPABLE;
            }
            CheckHdmiParam->IsReReadSinkEdid = (FALSE);
            CheckHdmiParam->IsReReadSCDC = (FALSE);
        }
        CheckHdmiParam->EdidCableConnectRead =(FALSE);
    } else {
        /*If retry Re Read EDID or Re Access SCDC is enabled*/
        if (CheckHdmiParam->IsReReadSinkEdid) {
            /*Read & Parse the EDID upon the Cable Connect to check
															  Sink Capability*/
            XV_HdmiTxSs1_ReadEdid(HdmiTxSs1Ptr, (u8*)&Buffer);
            XV_VidC_parse_edid((u8*)&Buffer, &CheckHdmiParam->EdidCtrlParam,
                                XVIDC_VERBOSE_DISABLE);

            if(CheckHdmiParam->EdidCtrlParam.IsSCDCPresent ==
															XVIDC_SUPPORTED) {
                if (CheckHdmiParam->EdidCtrlParam.IsSCDCReadRequestReady ==
                        XVIDC_SUPPORTED) {
                    CheckHdmiParam->IsReReadSinkEdid = (FALSE);
                    Status = XV_HdmiTxSs1_DetectHdmi20(HdmiTxSs1Ptr);
                    if (Status != XST_SUCCESS) {
                        CheckHdmiParam->IsReReadSCDC = (TRUE);
                    }
                }
            }
        }

        if (CheckHdmiParam->IsReReadSCDC) {
            Status = XV_HdmiTxSs1_DetectHdmi20(HdmiTxSs1Ptr);
            if (Status == XST_SUCCESS) {
                CheckHdmiParam->IsReReadSCDC = (FALSE);
            }
        }
    }
}

/*****************************************************************************/
/**
*
* This function re-reads EDID and re-parse the SCDC for a period of time
*   There are sink's or monitor have different startup time hence providing
*   multiple retry
*
* @param  HdmiTxSs1Ptr HDMI TX Subsystem Pointer
* @param  CheckHdmi20Param Application Data Structure (EDID & HDMI 2.0 Check)
*
* @return
*   -NA
*
* @note   None.
*
******************************************************************************/
u8 SinkReadyCheck (XV_HdmiTxSs1          *HdmiTxSs1Ptr,
                    EdidHdmi           *CheckHdmiParam)
{
	u8 SinkReadyStatus;

    if (CheckHdmiParam->IsReReadSinkEdid || CheckHdmiParam->IsReReadSCDC) {
    	CheckHdmiParam->SinkCheckRetryCount =
    			CheckHdmiParam->SinkCheckRetryCount + 1;
        if (CheckHdmiParam->SinkCheckRetryCount
        							== READINTERVAL) {
            /* Re-Read the EDID */
            EdidScdcCheck(HdmiTxSs1Ptr,CheckHdmiParam);
            /*High Priority iif the Sink EDID not Ready */
            if (CheckHdmiParam->IsReReadSinkEdid) {
            	CheckHdmiParam->IsReReadSinkEdidRetry =
            			CheckHdmiParam->IsReReadSinkEdidRetry + 1;
            	CheckHdmiParam->SinkCheckRetryCount = 0;
            }
            /* 2nd Priority if the SCDC is not Ready */
            /*Try to read the SCDC Register even the read request not capable
                in EDID is not asserted,
                Notes: Some commercial sink's not updating this
                EDID: HF-VSDB: RR_Capable bit */
            else if (CheckHdmiParam->IsReReadSCDC) {
            	CheckHdmiParam->IsReReadScdcRetry =
            			CheckHdmiParam->IsReReadScdcRetry + 1;
            	CheckHdmiParam->SinkCheckRetryCount = 0;
            }
        }

        if (CheckHdmiParam->IsReReadSinkEdidRetry == READEDIDRETRY) {
        	CheckHdmiParam->IsReReadSinkEdidRetry = 0;
            CheckHdmiParam->IsReReadSinkEdid = (FALSE);
            CheckHdmiParam->IsReReadSCDC = (TRUE);
            CheckHdmiParam->HdmiSinkWarningFlag |=
            		XV_HDMI_SINK_EDID_SCDC_MISMATCH;
        }

        if (CheckHdmiParam->IsReReadScdcRetry == READSCDCRETRY) {
        	CheckHdmiParam->IsReReadScdcRetry = 0;
            CheckHdmiParam->IsReReadSCDC = (FALSE);
            CheckHdmiParam->HdmiSinkWarningFlag |=
            		XV_HDMI_SINK_EDID_20_VSDB20_ACC_SCDC_FAIL;
        }
    }

    SinkReadyStatus = (!CheckHdmiParam->IsReReadSinkEdid &&
    							!CheckHdmiParam->IsReReadSCDC) ?
    									(TRUE):(FALSE);
    return (SinkReadyStatus);
}

/*****************************************************************************/
/**
*
* This function initialize the EDID Application Data Structure
* This function should be called only once when the HDMI cable is connected
*
* @param  CheckHdmi20Param Application Data Structure (EDID & HDMI 2.0 Check)
*
* @return
*   -NA
*
* @note   None.
*
******************************************************************************/
void EDIDConnectInit(EdidHdmi           *CheckHdmiParam){
	/*Enable EDID Read During Cable Connect*/
	CheckHdmiParam->EdidCableConnectRead = (TRUE);

	CheckHdmiParam->IsHDMI20SinkCapable  = (FALSE);
	CheckHdmiParam->IsReReadSCDC         = (FALSE);
	CheckHdmiParam->IsReReadSinkEdid     = (FALSE);
	CheckHdmiParam->SinkCheckRetryCount  = 0;
	CheckHdmiParam->IsReReadSinkEdidRetry= 0;
	CheckHdmiParam->IsReReadScdcRetry    = 0;
	CheckHdmiParam->HdmiSinkWarningFlag  = XV_HDMI_SINK_NO_WARNINGS;
											 /*No Warnings*/
}

/*****************************************************************************/
/**
*
* This function check's parsed EDID value, and perform it's functionality
* based on application.
*
* @param  CheckHdmi20Param Application Data Structure (EDID & HDMI 2.0 Check)
*
* @return
*   -NA
*
* @note   None.
*
******************************************************************************/
void SinkCapabilityCheck(EdidHdmi *CheckHdmiParam){
	if (!CheckHdmiParam->EdidCtrlParam.Is48bppSupp) {
		CheckHdmiParam->HdmiSinkWarningFlag |=
				XV_HDMI_SINK_DEEP_COLOR_16_NOT_SUPP;
	}
	if (!CheckHdmiParam->EdidCtrlParam.Is36bppSupp) {
		CheckHdmiParam->HdmiSinkWarningFlag |=
				XV_HDMI_SINK_DEEP_COLOR_12_NOT_SUPP;
	}
	if (!CheckHdmiParam->EdidCtrlParam.Is30bppSupp) {
		CheckHdmiParam->HdmiSinkWarningFlag |=
				XV_HDMI_SINK_DEEP_COLOR_10_NOT_SUPP;
	}
	if (!CheckHdmiParam->EdidCtrlParam.IsHdmi) {
		CheckHdmiParam->HdmiSinkWarningFlag |=
				XV_SINK_NOT_HDMI;
	}
}

/*****************************************************************************/
/**
*
* This function prints Sink Capability Warning Message
*
* @param  CheckHdmi20Param Application Data Structure (EDID & HDMI 2.0 Check)
*
* @return
*   -NA
*
* @note   None.
*
******************************************************************************/
void SinkCapWarningMsg(EdidHdmi *CheckHdmiParam){
	if (CheckHdmiParam->HdmiSinkWarningFlag &
			XV_HDMI_SINK_EDID_SCDC_MISMATCH) {
			xil_printf(ANSI_COLOR_YELLOW "Warning: Connected Sink's EDID "
			   "indicates HDMI 2.0 capable, but the SCDC read request "
			   "register bit (VSDB:RR_Capable) is not asserted"
			   ANSI_COLOR_RESET "\r\n");
	}
	if (CheckHdmiParam->HdmiSinkWarningFlag &
			XV_HDMI_SINK_EDID_20_VSDB20_NA_SCDC_PASS) {
			xil_printf(ANSI_COLOR_YELLOW "Warning: Connected Sink's "
			   "EDID indicates HDMI 2.0 capable but SCDC is not "
			   "present (EDID: HF-VSDB)" ANSI_COLOR_RESET "\r\n");
	}
	if (CheckHdmiParam->HdmiSinkWarningFlag &
			XV_HDMI_SINK_EDID_20_VSDB20_ACC_SCDC_FAIL) {
			xil_printf(ANSI_COLOR_YELLOW "Warning: Connected Sink's "
				"EDID indicates HDMI 2.0 capable, SCDC present "
				"and capable of initiating read request, however "
				"the SCDC is inaccessible" ANSI_COLOR_RESET "\r\n");
	}
	if (CheckHdmiParam->HdmiSinkWarningFlag &
			XV_HDMI_SINK_EDID_14_SCDC_PASS) {
			xil_printf(ANSI_COLOR_YELLOW "Warning: Connected Sink's "
			"EDID indicates HDMI 2.0 not capable"
					ANSI_COLOR_RESET "\r\n");
	}
	if (CheckHdmiParam->HdmiSinkWarningFlag &
			XV_HDMI_SINK_DEEP_COLOR_10_NOT_SUPP) {
			xil_printf(ANSI_COLOR_YELLOW "Warning: Connected Sink's "
			"EDID indicates Deep Color of 10 BpC Not Supported"
					ANSI_COLOR_RESET "\r\n");
	}
	if (CheckHdmiParam->HdmiSinkWarningFlag &
			XV_HDMI_SINK_DEEP_COLOR_12_NOT_SUPP) {
		xil_printf(ANSI_COLOR_YELLOW "Warning: Connected Sink's "
		"EDID indicates Deep Color of 12 BpC Not Supported"
				ANSI_COLOR_RESET "\r\n");
	}
	if (CheckHdmiParam->HdmiSinkWarningFlag &
			XV_HDMI_SINK_DEEP_COLOR_16_NOT_SUPP) {
		xil_printf(ANSI_COLOR_YELLOW "Warning: Connected Sink's "
		"EDID indicates Deep Color of 16 BpC Not Supported"
				ANSI_COLOR_RESET "\r\n");
	}
	if (CheckHdmiParam->HdmiSinkWarningFlag &
			XV_SINK_NOT_HDMI) {
		xil_printf(ANSI_COLOR_YELLOW "Warning: Connected Sink's "
		"EDID indicates HDMI is not Supported"
				ANSI_COLOR_RESET "\r\n");
	}
}
#endif
