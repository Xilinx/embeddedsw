###############################################################################
#
# Copyright (C) 2017 Xilinx, Inc.  All rights reserved.
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
#
# Use of the Software is limited solely to applications:
# (a) running on a Xilinx device, or
# (b) that interact with a Xilinx device through a bus or interconnect.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
# XILINX  BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
# WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
# OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.
#
# Except as contained in this notice, the name of the Xilinx shall not be used
# in advertising or otherwise to promote the sale, use or other dealings in
# this Software without prior written authorization from Xilinx.
#
###############################################################################
#uses "xillib.tcl"

proc generate {drv_handle} {
    generate_include_file $drv_handle "xparameters.h" "XRFdc" "NUM_INSTANCES" "DEVICE_ID" "C_BASEADDR" "C_HIGHADDR" "C_High_Speed_ADC" "C_Sysref_Master" "C_Sysref_Master" "C_Sysref_Source" "C_Sysref_Source" "C_DAC0_Enable" "C_DAC0_PLL_Enable" "C_DAC0_Sampling_Rate" "C_DAC0_Refclk_Freq" "C_DAC0_Fabric_Freq" "C_DAC_Slice00_Enable" "C_DAC_Invsinc_Ctrl00" "C_DAC_Mixer_Mode00" "C_DAC_Decoder_Mode00" "C_DAC_Slice01_Enable" "C_DAC_Invsinc_Ctrl01" "C_DAC_Mixer_Mode01" "C_DAC_Decoder_Mode01" "C_DAC_Slice02_Enable" "C_DAC_Invsinc_Ctrl02" "C_DAC_Mixer_Mode02" "C_DAC_Decoder_Mode02" "C_DAC_Slice03_Enable" "C_DAC_Invsinc_Ctrl03" "C_DAC_Mixer_Mode03" "C_DAC_Decoder_Mode03" "C_DAC_Data_Type00" "C_DAC_Data_Width00" "C_DAC_Interpolation_Mode00" "C_DAC_Fifo00_Enable" "C_DAC_Adder00_Enable" "C_DAC_Data_Type01" "C_DAC_Data_Width01" "C_DAC_Interpolation_Mode01" "C_DAC_Fifo01_Enable" "C_DAC_Adder01_Enable" "C_DAC_Data_Type02" "C_DAC_Data_Width02" "C_DAC_Interpolation_Mode02" "C_DAC_Fifo02_Enable" "C_DAC_Adder02_Enable" "C_DAC_Data_Type03" "C_DAC_Data_Width03" "C_DAC_Interpolation_Mode03" "C_DAC_Fifo03_Enable" "C_DAC_Adder03_Enable" "C_DAC1_Enable" "C_DAC1_PLL_Enable" "C_DAC1_Sampling_Rate" "C_DAC1_Refclk_Freq" "C_DAC1_Fabric_Freq" "C_DAC_Slice10_Enable" "C_DAC_Invsinc_Ctrl10" "C_DAC_Mixer_Mode10" "C_DAC_Decoder_Mode10" "C_DAC_Slice11_Enable" "C_DAC_Invsinc_Ctrl11" "C_DAC_Mixer_Mode11" "C_DAC_Decoder_Mode11" "C_DAC_Slice12_Enable" "C_DAC_Invsinc_Ctrl12" "C_DAC_Mixer_Mode12" "C_DAC_Decoder_Mode12" "C_DAC_Slice13_Enable" "C_DAC_Invsinc_Ctrl13" "C_DAC_Mixer_Mode13" "C_DAC_Decoder_Mode13" "C_DAC_Data_Type10" "C_DAC_Data_Width10" "C_DAC_Interpolation_Mode10" "C_DAC_Fifo10_Enable" "C_DAC_Adder10_Enable" "C_DAC_Data_Type11" "C_DAC_Data_Width11" "C_DAC_Interpolation_Mode11" "C_DAC_Fifo11_Enable" "C_DAC_Adder11_Enable" "C_DAC_Data_Type12" "C_DAC_Data_Width12" "C_DAC_Interpolation_Mode12" "C_DAC_Fifo12_Enable" "C_DAC_Adder12_Enable" "C_DAC_Data_Type13" "C_DAC_Data_Width13" "C_DAC_Interpolation_Mode13" "C_DAC_Fifo13_Enable" "C_DAC_Adder13_Enable" "C_DAC2_Enable" "C_DAC2_PLL_Enable" "C_DAC2_Sampling_Rate" "C_DAC2_Refclk_Freq" "C_DAC2_Fabric_Freq" "C_DAC_Slice20_Enable" "C_DAC_Invsinc_Ctrl20" "C_DAC_Mixer_Mode20" "C_DAC_Decoder_Mode20" "C_DAC_Slice21_Enable" "C_DAC_Invsinc_Ctrl21" "C_DAC_Mixer_Mode21" "C_DAC_Decoder_Mode21" "C_DAC_Slice22_Enable" "C_DAC_Invsinc_Ctrl22" "C_DAC_Mixer_Mode22" "C_DAC_Decoder_Mode22" "C_DAC_Slice23_Enable" "C_DAC_Invsinc_Ctrl23" "C_DAC_Mixer_Mode23" "C_DAC_Decoder_Mode23" "C_DAC_Data_Type20" "C_DAC_Data_Width20" "C_DAC_Interpolation_Mode20" "C_DAC_Fifo20_Enable" "C_DAC_Adder20_Enable" "C_DAC_Data_Type21" "C_DAC_Data_Width21" "C_DAC_Interpolation_Mode21" "C_DAC_Fifo21_Enable" "C_DAC_Adder21_Enable" "C_DAC_Data_Type22" "C_DAC_Data_Width22" "C_DAC_Interpolation_Mode22" "C_DAC_Fifo22_Enable" "C_DAC_Adder22_Enable" "C_DAC_Data_Type23" "C_DAC_Data_Width23" "C_DAC_Interpolation_Mode23" "C_DAC_Fifo23_Enable" "C_DAC_Adder23_Enable" "C_DAC3_Enable" "C_DAC3_PLL_Enable" "C_DAC3_Sampling_Rate" "C_DAC3_Refclk_Freq" "C_DAC3_Fabric_Freq" "C_DAC_Slice30_Enable" "C_DAC_Invsinc_Ctrl30" "C_DAC_Mixer_Mode30" "C_DAC_Decoder_Mode30" "C_DAC_Slice31_Enable" "C_DAC_Invsinc_Ctrl31" "C_DAC_Mixer_Mode31" "C_DAC_Decoder_Mode31" "C_DAC_Slice32_Enable" "C_DAC_Invsinc_Ctrl32" "C_DAC_Mixer_Mode32" "C_DAC_Decoder_Mode32" "C_DAC_Slice33_Enable" "C_DAC_Invsinc_Ctrl33" "C_DAC_Mixer_Mode33" "C_DAC_Decoder_Mode33" "C_DAC_Data_Type30" "C_DAC_Data_Width30" "C_DAC_Interpolation_Mode30" "C_DAC_Fifo30_Enable" "C_DAC_Adder30_Enable" "C_DAC_Data_Type31" "C_DAC_Data_Width31" "C_DAC_Interpolation_Mode31" "C_DAC_Fifo31_Enable" "C_DAC_Adder31_Enable" "C_DAC_Data_Type32" "C_DAC_Data_Width32" "C_DAC_Interpolation_Mode32" "C_DAC_Fifo32_Enable" "C_DAC_Adder32_Enable" "C_DAC_Data_Type33" "C_DAC_Data_Width33" "C_DAC_Interpolation_Mode33" "C_DAC_Fifo33_Enable" "C_DAC_Adder33_Enable" "C_ADC0_Enable" "C_ADC0_PLL_Enable" "C_ADC0_Sampling_Rate" "C_ADC0_Refclk_Freq" "C_ADC0_Fabric_Freq" "C_ADC_Slice00_Enable" "C_ADC_Mixer_Mode00" "C_ADC_Slice01_Enable" "C_ADC_Mixer_Mode01" "C_ADC_Slice02_Enable" "C_ADC_Mixer_Mode02" "C_ADC_Slice03_Enable" "C_ADC_Mixer_Mode03" "C_ADC_Data_Type00" "C_ADC_Data_Width00" "C_ADC_Decimation_Mode00" "C_ADC_Fifo00_Enable" "C_ADC_Data_Type01" "C_ADC_Data_Width01" "C_ADC_Decimation_Mode01" "C_ADC_Fifo01_Enable" "C_ADC_Data_Type02" "C_ADC_Data_Width02" "C_ADC_Decimation_Mode02" "C_ADC_Fifo02_Enable" "C_ADC_Data_Type03" "C_ADC_Data_Width03" "C_ADC_Decimation_Mode03" "C_ADC_Fifo03_Enable" "C_ADC1_Enable" "C_ADC1_PLL_Enable" "C_ADC1_Sampling_Rate" "C_ADC1_Refclk_Freq" "C_ADC1_Fabric_Freq" "C_ADC_Slice10_Enable" "C_ADC_Mixer_Mode10" "C_ADC_Slice11_Enable" "C_ADC_Mixer_Mode11" "C_ADC_Slice12_Enable" "C_ADC_Mixer_Mode12" "C_ADC_Slice13_Enable" "C_ADC_Mixer_Mode13" "C_ADC_Data_Type10" "C_ADC_Data_Width10" "C_ADC_Decimation_Mode10" "C_ADC_Fifo10_Enable" "C_ADC_Data_Type11" "C_ADC_Data_Width11" "C_ADC_Decimation_Mode11" "C_ADC_Fifo11_Enable" "C_ADC_Data_Type12" "C_ADC_Data_Width12" "C_ADC_Decimation_Mode12" "C_ADC_Fifo12_Enable" "C_ADC_Data_Type13" "C_ADC_Data_Width13" "C_ADC_Decimation_Mode13" "C_ADC_Fifo13_Enable" "C_ADC2_Enable" "C_ADC2_PLL_Enable" "C_ADC2_Sampling_Rate" "C_ADC2_Refclk_Freq" "C_ADC2_Fabric_Freq" "C_ADC_Slice20_Enable" "C_ADC_Mixer_Mode20" "C_ADC_Slice21_Enable" "C_ADC_Mixer_Mode21" "C_ADC_Slice22_Enable" "C_ADC_Mixer_Mode22" "C_ADC_Slice23_Enable" "C_ADC_Mixer_Mode23" "C_ADC_Data_Type20" "C_ADC_Data_Width20" "C_ADC_Decimation_Mode20" "C_ADC_Fifo20_Enable" "C_ADC_Data_Type21" "C_ADC_Data_Width21" "C_ADC_Decimation_Mode21" "C_ADC_Fifo21_Enable" "C_ADC_Data_Type22" "C_ADC_Data_Width22" "C_ADC_Decimation_Mode22" "C_ADC_Fifo22_Enable" "C_ADC_Data_Type23" "C_ADC_Data_Width23" "C_ADC_Decimation_Mode23" "C_ADC_Fifo23_Enable" "C_ADC3_Enable" "C_ADC3_PLL_Enable" "C_ADC3_Sampling_Rate" "C_ADC3_Refclk_Freq" "C_ADC3_Fabric_Freq" "C_ADC_Slice30_Enable" "C_ADC_Mixer_Mode30" "C_ADC_Slice31_Enable" "C_ADC_Mixer_Mode31" "C_ADC_Slice32_Enable" "C_ADC_Mixer_Mode32" "C_ADC_Slice33_Enable" "C_ADC_Mixer_Mode33" "C_ADC_Data_Type30" "C_ADC_Data_Width30" "C_ADC_Decimation_Mode30" "C_ADC_Fifo30_Enable" "C_ADC_Data_Type31" "C_ADC_Data_Width31" "C_ADC_Decimation_Mode31" "C_ADC_Fifo31_Enable" "C_ADC_Data_Type32" "C_ADC_Data_Width32" "C_ADC_Decimation_Mode32" "C_ADC_Fifo32_Enable" "C_ADC_Data_Type33" "C_ADC_Data_Width33" "C_ADC_Decimation_Mode33" "C_ADC_Fifo33_Enable"

    generate_config_file $drv_handle "xrfdc_g.c" "XRFdc" "DEVICE_ID" "C_BASEADDR" "C_High_Speed_ADC" "C_Sysref_Master" "C_Sysref_Master" "C_Sysref_Source" "C_Sysref_Source" "C_DAC0_Enable" "C_DAC0_PLL_Enable" "C_DAC0_Sampling_Rate" "C_DAC0_Refclk_Freq" "C_DAC0_Fabric_Freq" "C_DAC_Slice00_Enable" "C_DAC_Invsinc_Ctrl00" "C_DAC_Mixer_Mode00" "C_DAC_Decoder_Mode00" "C_DAC_Slice01_Enable" "C_DAC_Invsinc_Ctrl01" "C_DAC_Mixer_Mode01" "C_DAC_Decoder_Mode01" "C_DAC_Slice02_Enable" "C_DAC_Invsinc_Ctrl02" "C_DAC_Mixer_Mode02" "C_DAC_Decoder_Mode02" "C_DAC_Slice03_Enable" "C_DAC_Invsinc_Ctrl03" "C_DAC_Mixer_Mode03" "C_DAC_Decoder_Mode03" "C_DAC_Data_Type00" "C_DAC_Data_Width00" "C_DAC_Interpolation_Mode00" "C_DAC_Fifo00_Enable" "C_DAC_Adder00_Enable" "C_DAC_Data_Type01" "C_DAC_Data_Width01" "C_DAC_Interpolation_Mode01" "C_DAC_Fifo01_Enable" "C_DAC_Adder01_Enable" "C_DAC_Data_Type02" "C_DAC_Data_Width02" "C_DAC_Interpolation_Mode02" "C_DAC_Fifo02_Enable" "C_DAC_Adder02_Enable" "C_DAC_Data_Type03" "C_DAC_Data_Width03" "C_DAC_Interpolation_Mode03" "C_DAC_Fifo03_Enable" "C_DAC_Adder03_Enable" "C_DAC1_Enable" "C_DAC1_PLL_Enable" "C_DAC1_Sampling_Rate" "C_DAC1_Refclk_Freq" "C_DAC1_Fabric_Freq" "C_DAC_Slice10_Enable" "C_DAC_Invsinc_Ctrl10" "C_DAC_Mixer_Mode10" "C_DAC_Decoder_Mode10" "C_DAC_Slice11_Enable" "C_DAC_Invsinc_Ctrl11" "C_DAC_Mixer_Mode11" "C_DAC_Decoder_Mode11" "C_DAC_Slice12_Enable" "C_DAC_Invsinc_Ctrl12" "C_DAC_Mixer_Mode12" "C_DAC_Decoder_Mode12" "C_DAC_Slice13_Enable" "C_DAC_Invsinc_Ctrl13" "C_DAC_Mixer_Mode13" "C_DAC_Decoder_Mode13" "C_DAC_Data_Type10" "C_DAC_Data_Width10" "C_DAC_Interpolation_Mode10" "C_DAC_Fifo10_Enable" "C_DAC_Adder10_Enable" "C_DAC_Data_Type11" "C_DAC_Data_Width11" "C_DAC_Interpolation_Mode11" "C_DAC_Fifo11_Enable" "C_DAC_Adder11_Enable" "C_DAC_Data_Type12" "C_DAC_Data_Width12" "C_DAC_Interpolation_Mode12" "C_DAC_Fifo12_Enable" "C_DAC_Adder12_Enable" "C_DAC_Data_Type13" "C_DAC_Data_Width13" "C_DAC_Interpolation_Mode13" "C_DAC_Fifo13_Enable" "C_DAC_Adder13_Enable" "C_DAC2_Enable" "C_DAC2_PLL_Enable" "C_DAC2_Sampling_Rate" "C_DAC2_Refclk_Freq" "C_DAC2_Fabric_Freq" "C_DAC_Slice20_Enable" "C_DAC_Invsinc_Ctrl20" "C_DAC_Mixer_Mode20" "C_DAC_Decoder_Mode20" "C_DAC_Slice21_Enable" "C_DAC_Invsinc_Ctrl21" "C_DAC_Mixer_Mode21" "C_DAC_Decoder_Mode21" "C_DAC_Slice22_Enable" "C_DAC_Invsinc_Ctrl22" "C_DAC_Mixer_Mode22" "C_DAC_Decoder_Mode22" "C_DAC_Slice23_Enable" "C_DAC_Invsinc_Ctrl23" "C_DAC_Mixer_Mode23" "C_DAC_Decoder_Mode23" "C_DAC_Data_Type20" "C_DAC_Data_Width20" "C_DAC_Interpolation_Mode20" "C_DAC_Fifo20_Enable" "C_DAC_Adder20_Enable" "C_DAC_Data_Type21" "C_DAC_Data_Width21" "C_DAC_Interpolation_Mode21" "C_DAC_Fifo21_Enable" "C_DAC_Adder21_Enable" "C_DAC_Data_Type22" "C_DAC_Data_Width22" "C_DAC_Interpolation_Mode22" "C_DAC_Fifo22_Enable" "C_DAC_Adder22_Enable" "C_DAC_Data_Type23" "C_DAC_Data_Width23" "C_DAC_Interpolation_Mode23" "C_DAC_Fifo23_Enable" "C_DAC_Adder23_Enable" "C_DAC3_Enable" "C_DAC3_PLL_Enable" "C_DAC3_Sampling_Rate" "C_DAC3_Refclk_Freq" "C_DAC3_Fabric_Freq" "C_DAC_Slice30_Enable" "C_DAC_Invsinc_Ctrl30" "C_DAC_Mixer_Mode30" "C_DAC_Decoder_Mode30" "C_DAC_Slice31_Enable" "C_DAC_Invsinc_Ctrl31" "C_DAC_Mixer_Mode31" "C_DAC_Decoder_Mode31" "C_DAC_Slice32_Enable" "C_DAC_Invsinc_Ctrl32" "C_DAC_Mixer_Mode32" "C_DAC_Decoder_Mode32" "C_DAC_Slice33_Enable" "C_DAC_Invsinc_Ctrl33" "C_DAC_Mixer_Mode33" "C_DAC_Decoder_Mode33" "C_DAC_Data_Type30" "C_DAC_Data_Width30" "C_DAC_Interpolation_Mode30" "C_DAC_Fifo30_Enable" "C_DAC_Adder30_Enable" "C_DAC_Data_Type31" "C_DAC_Data_Width31" "C_DAC_Interpolation_Mode31" "C_DAC_Fifo31_Enable" "C_DAC_Adder31_Enable" "C_DAC_Data_Type32" "C_DAC_Data_Width32" "C_DAC_Interpolation_Mode32" "C_DAC_Fifo32_Enable" "C_DAC_Adder32_Enable" "C_DAC_Data_Type33" "C_DAC_Data_Width33" "C_DAC_Interpolation_Mode33" "C_DAC_Fifo33_Enable" "C_DAC_Adder33_Enable" "C_ADC0_Enable" "C_ADC0_PLL_Enable" "C_ADC0_Sampling_Rate" "C_ADC0_Refclk_Freq" "C_ADC0_Fabric_Freq" "C_ADC_Slice00_Enable" "C_ADC_Mixer_Mode00" "C_ADC_Slice01_Enable" "C_ADC_Mixer_Mode01" "C_ADC_Slice02_Enable" "C_ADC_Mixer_Mode02" "C_ADC_Slice03_Enable" "C_ADC_Mixer_Mode03" "C_ADC_Data_Type00" "C_ADC_Data_Width00" "C_ADC_Decimation_Mode00" "C_ADC_Fifo00_Enable" "C_ADC_Data_Type01" "C_ADC_Data_Width01" "C_ADC_Decimation_Mode01" "C_ADC_Fifo01_Enable" "C_ADC_Data_Type02" "C_ADC_Data_Width02" "C_ADC_Decimation_Mode02" "C_ADC_Fifo02_Enable" "C_ADC_Data_Type03" "C_ADC_Data_Width03" "C_ADC_Decimation_Mode03" "C_ADC_Fifo03_Enable" "C_ADC1_Enable" "C_ADC1_PLL_Enable" "C_ADC1_Sampling_Rate" "C_ADC1_Refclk_Freq" "C_ADC1_Fabric_Freq" "C_ADC_Slice10_Enable" "C_ADC_Mixer_Mode10" "C_ADC_Slice11_Enable" "C_ADC_Mixer_Mode11" "C_ADC_Slice12_Enable" "C_ADC_Mixer_Mode12" "C_ADC_Slice13_Enable" "C_ADC_Mixer_Mode13" "C_ADC_Data_Type10" "C_ADC_Data_Width10" "C_ADC_Decimation_Mode10" "C_ADC_Fifo10_Enable" "C_ADC_Data_Type11" "C_ADC_Data_Width11" "C_ADC_Decimation_Mode11" "C_ADC_Fifo11_Enable" "C_ADC_Data_Type12" "C_ADC_Data_Width12" "C_ADC_Decimation_Mode12" "C_ADC_Fifo12_Enable" "C_ADC_Data_Type13" "C_ADC_Data_Width13" "C_ADC_Decimation_Mode13" "C_ADC_Fifo13_Enable" "C_ADC2_Enable" "C_ADC2_PLL_Enable" "C_ADC2_Sampling_Rate" "C_ADC2_Refclk_Freq" "C_ADC2_Fabric_Freq" "C_ADC_Slice20_Enable" "C_ADC_Mixer_Mode20" "C_ADC_Slice21_Enable" "C_ADC_Mixer_Mode21" "C_ADC_Slice22_Enable" "C_ADC_Mixer_Mode22" "C_ADC_Slice23_Enable" "C_ADC_Mixer_Mode23" "C_ADC_Data_Type20" "C_ADC_Data_Width20" "C_ADC_Decimation_Mode20" "C_ADC_Fifo20_Enable" "C_ADC_Data_Type21" "C_ADC_Data_Width21" "C_ADC_Decimation_Mode21" "C_ADC_Fifo21_Enable" "C_ADC_Data_Type22" "C_ADC_Data_Width22" "C_ADC_Decimation_Mode22" "C_ADC_Fifo22_Enable" "C_ADC_Data_Type23" "C_ADC_Data_Width23" "C_ADC_Decimation_Mode23" "C_ADC_Fifo23_Enable" "C_ADC3_Enable" "C_ADC3_PLL_Enable" "C_ADC3_Sampling_Rate" "C_ADC3_Refclk_Freq" "C_ADC3_Fabric_Freq" "C_ADC_Slice30_Enable" "C_ADC_Mixer_Mode30" "C_ADC_Slice31_Enable" "C_ADC_Mixer_Mode31" "C_ADC_Slice32_Enable" "C_ADC_Mixer_Mode32" "C_ADC_Slice33_Enable" "C_ADC_Mixer_Mode33" "C_ADC_Data_Type30" "C_ADC_Data_Width30" "C_ADC_Decimation_Mode30" "C_ADC_Fifo30_Enable" "C_ADC_Data_Type31" "C_ADC_Data_Width31" "C_ADC_Decimation_Mode31" "C_ADC_Fifo31_Enable" "C_ADC_Data_Type32" "C_ADC_Data_Width32" "C_ADC_Decimation_Mode32" "C_ADC_Fifo32_Enable" "C_ADC_Data_Type33" "C_ADC_Data_Width33" "C_ADC_Decimation_Mode33" "C_ADC_Fifo33_Enable"
	generate_canonical_xpars $drv_handle "xparameters.h" "XRFdc" "DEVICE_ID" "C_BASEADDR" "C_HIGHADDR" "C_High_Speed_ADC" "C_Sysref_Master" "C_Sysref_Master" "C_Sysref_Source" "C_Sysref_Source" "C_DAC0_Enable" "C_DAC0_PLL_Enable" "C_DAC0_Sampling_Rate" "C_DAC0_Refclk_Freq" "C_DAC0_Fabric_Freq" "C_DAC_Slice00_Enable" "C_DAC_Invsinc_Ctrl00" "C_DAC_Mixer_Mode00" "C_DAC_Decoder_Mode00" "C_DAC_Slice01_Enable" "C_DAC_Invsinc_Ctrl01" "C_DAC_Mixer_Mode01" "C_DAC_Decoder_Mode01" "C_DAC_Slice02_Enable" "C_DAC_Invsinc_Ctrl02" "C_DAC_Mixer_Mode02" "C_DAC_Decoder_Mode02" "C_DAC_Slice03_Enable" "C_DAC_Invsinc_Ctrl03" "C_DAC_Mixer_Mode03" "C_DAC_Decoder_Mode03" "C_DAC_Data_Type00" "C_DAC_Data_Width00" "C_DAC_Interpolation_Mode00" "C_DAC_Fifo00_Enable" "C_DAC_Adder00_Enable" "C_DAC_Data_Type01" "C_DAC_Data_Width01" "C_DAC_Interpolation_Mode01" "C_DAC_Fifo01_Enable" "C_DAC_Adder01_Enable" "C_DAC_Data_Type02" "C_DAC_Data_Width02" "C_DAC_Interpolation_Mode02" "C_DAC_Fifo02_Enable" "C_DAC_Adder02_Enable" "C_DAC_Data_Type03" "C_DAC_Data_Width03" "C_DAC_Interpolation_Mode03" "C_DAC_Fifo03_Enable" "C_DAC_Adder03_Enable" "C_DAC1_Enable" "C_DAC1_PLL_Enable" "C_DAC1_Sampling_Rate" "C_DAC1_Refclk_Freq" "C_DAC1_Fabric_Freq" "C_DAC_Slice10_Enable" "C_DAC_Invsinc_Ctrl10" "C_DAC_Mixer_Mode10" "C_DAC_Decoder_Mode10" "C_DAC_Slice11_Enable" "C_DAC_Invsinc_Ctrl11" "C_DAC_Mixer_Mode11" "C_DAC_Decoder_Mode11" "C_DAC_Slice12_Enable" "C_DAC_Invsinc_Ctrl12" "C_DAC_Mixer_Mode12" "C_DAC_Decoder_Mode12" "C_DAC_Slice13_Enable" "C_DAC_Invsinc_Ctrl13" "C_DAC_Mixer_Mode13" "C_DAC_Decoder_Mode13" "C_DAC_Data_Type10" "C_DAC_Data_Width10" "C_DAC_Interpolation_Mode10" "C_DAC_Fifo10_Enable" "C_DAC_Adder10_Enable" "C_DAC_Data_Type11" "C_DAC_Data_Width11" "C_DAC_Interpolation_Mode11" "C_DAC_Fifo11_Enable" "C_DAC_Adder11_Enable" "C_DAC_Data_Type12" "C_DAC_Data_Width12" "C_DAC_Interpolation_Mode12" "C_DAC_Fifo12_Enable" "C_DAC_Adder12_Enable" "C_DAC_Data_Type13" "C_DAC_Data_Width13" "C_DAC_Interpolation_Mode13" "C_DAC_Fifo13_Enable" "C_DAC_Adder13_Enable" "C_DAC2_Enable" "C_DAC2_PLL_Enable" "C_DAC2_Sampling_Rate" "C_DAC2_Refclk_Freq" "C_DAC2_Fabric_Freq" "C_DAC_Slice20_Enable" "C_DAC_Invsinc_Ctrl20" "C_DAC_Mixer_Mode20" "C_DAC_Decoder_Mode20" "C_DAC_Slice21_Enable" "C_DAC_Invsinc_Ctrl21" "C_DAC_Mixer_Mode21" "C_DAC_Decoder_Mode21" "C_DAC_Slice22_Enable" "C_DAC_Invsinc_Ctrl22" "C_DAC_Mixer_Mode22" "C_DAC_Decoder_Mode22" "C_DAC_Slice23_Enable" "C_DAC_Invsinc_Ctrl23" "C_DAC_Mixer_Mode23" "C_DAC_Decoder_Mode23" "C_DAC_Data_Type20" "C_DAC_Data_Width20" "C_DAC_Interpolation_Mode20" "C_DAC_Fifo20_Enable" "C_DAC_Adder20_Enable" "C_DAC_Data_Type21" "C_DAC_Data_Width21" "C_DAC_Interpolation_Mode21" "C_DAC_Fifo21_Enable" "C_DAC_Adder21_Enable" "C_DAC_Data_Type22" "C_DAC_Data_Width22" "C_DAC_Interpolation_Mode22" "C_DAC_Fifo22_Enable" "C_DAC_Adder22_Enable" "C_DAC_Data_Type23" "C_DAC_Data_Width23" "C_DAC_Interpolation_Mode23" "C_DAC_Fifo23_Enable" "C_DAC_Adder23_Enable" "C_DAC3_Enable" "C_DAC3_PLL_Enable" "C_DAC3_Sampling_Rate" "C_DAC3_Refclk_Freq" "C_DAC3_Fabric_Freq" "C_DAC_Slice30_Enable" "C_DAC_Invsinc_Ctrl30" "C_DAC_Mixer_Mode30" "C_DAC_Decoder_Mode30" "C_DAC_Slice31_Enable" "C_DAC_Invsinc_Ctrl31" "C_DAC_Mixer_Mode31" "C_DAC_Decoder_Mode31" "C_DAC_Slice32_Enable" "C_DAC_Invsinc_Ctrl32" "C_DAC_Mixer_Mode32" "C_DAC_Decoder_Mode32" "C_DAC_Slice33_Enable" "C_DAC_Invsinc_Ctrl33" "C_DAC_Mixer_Mode33" "C_DAC_Decoder_Mode33" "C_DAC_Data_Type30" "C_DAC_Data_Width30" "C_DAC_Interpolation_Mode30" "C_DAC_Fifo30_Enable" "C_DAC_Adder30_Enable" "C_DAC_Data_Type31" "C_DAC_Data_Width31" "C_DAC_Interpolation_Mode31" "C_DAC_Fifo31_Enable" "C_DAC_Adder31_Enable" "C_DAC_Data_Type32" "C_DAC_Data_Width32" "C_DAC_Interpolation_Mode32" "C_DAC_Fifo32_Enable" "C_DAC_Adder32_Enable" "C_DAC_Data_Type33" "C_DAC_Data_Width33" "C_DAC_Interpolation_Mode33" "C_DAC_Fifo33_Enable" "C_DAC_Adder33_Enable" "C_ADC0_Enable" "C_ADC0_PLL_Enable" "C_ADC0_Sampling_Rate" "C_ADC0_Refclk_Freq" "C_ADC0_Fabric_Freq" "C_ADC_Slice00_Enable" "C_ADC_Mixer_Mode00" "C_ADC_Slice01_Enable" "C_ADC_Mixer_Mode01" "C_ADC_Slice02_Enable" "C_ADC_Mixer_Mode02" "C_ADC_Slice03_Enable" "C_ADC_Mixer_Mode03" "C_ADC_Data_Type00" "C_ADC_Data_Width00" "C_ADC_Decimation_Mode00" "C_ADC_Fifo00_Enable" "C_ADC_Data_Type01" "C_ADC_Data_Width01" "C_ADC_Decimation_Mode01" "C_ADC_Fifo01_Enable" "C_ADC_Data_Type02" "C_ADC_Data_Width02" "C_ADC_Decimation_Mode02" "C_ADC_Fifo02_Enable" "C_ADC_Data_Type03" "C_ADC_Data_Width03" "C_ADC_Decimation_Mode03" "C_ADC_Fifo03_Enable" "C_ADC1_Enable" "C_ADC1_PLL_Enable" "C_ADC1_Sampling_Rate" "C_ADC1_Refclk_Freq" "C_ADC1_Fabric_Freq" "C_ADC_Slice10_Enable" "C_ADC_Mixer_Mode10" "C_ADC_Slice11_Enable" "C_ADC_Mixer_Mode11" "C_ADC_Slice12_Enable" "C_ADC_Mixer_Mode12" "C_ADC_Slice13_Enable" "C_ADC_Mixer_Mode13" "C_ADC_Data_Type10" "C_ADC_Data_Width10" "C_ADC_Decimation_Mode10" "C_ADC_Fifo10_Enable" "C_ADC_Data_Type11" "C_ADC_Data_Width11" "C_ADC_Decimation_Mode11" "C_ADC_Fifo11_Enable" "C_ADC_Data_Type12" "C_ADC_Data_Width12" "C_ADC_Decimation_Mode12" "C_ADC_Fifo12_Enable" "C_ADC_Data_Type13" "C_ADC_Data_Width13" "C_ADC_Decimation_Mode13" "C_ADC_Fifo13_Enable" "C_ADC2_Enable" "C_ADC2_PLL_Enable" "C_ADC2_Sampling_Rate" "C_ADC2_Refclk_Freq" "C_ADC2_Fabric_Freq" "C_ADC_Slice20_Enable" "C_ADC_Mixer_Mode20" "C_ADC_Slice21_Enable" "C_ADC_Mixer_Mode21" "C_ADC_Slice22_Enable" "C_ADC_Mixer_Mode22" "C_ADC_Slice23_Enable" "C_ADC_Mixer_Mode23" "C_ADC_Data_Type20" "C_ADC_Data_Width20" "C_ADC_Decimation_Mode20" "C_ADC_Fifo20_Enable" "C_ADC_Data_Type21" "C_ADC_Data_Width21" "C_ADC_Decimation_Mode21" "C_ADC_Fifo21_Enable" "C_ADC_Data_Type22" "C_ADC_Data_Width22" "C_ADC_Decimation_Mode22" "C_ADC_Fifo22_Enable" "C_ADC_Data_Type23" "C_ADC_Data_Width23" "C_ADC_Decimation_Mode23" "C_ADC_Fifo23_Enable" "C_ADC3_Enable" "C_ADC3_PLL_Enable" "C_ADC3_Sampling_Rate" "C_ADC3_Refclk_Freq" "C_ADC3_Fabric_Freq" "C_ADC_Slice30_Enable" "C_ADC_Mixer_Mode30" "C_ADC_Slice31_Enable" "C_ADC_Mixer_Mode31" "C_ADC_Slice32_Enable" "C_ADC_Mixer_Mode32" "C_ADC_Slice33_Enable" "C_ADC_Mixer_Mode33" "C_ADC_Data_Type30" "C_ADC_Data_Width30" "C_ADC_Decimation_Mode30" "C_ADC_Fifo30_Enable" "C_ADC_Data_Type31" "C_ADC_Data_Width31" "C_ADC_Decimation_Mode31" "C_ADC_Fifo31_Enable" "C_ADC_Data_Type32" "C_ADC_Data_Width32" "C_ADC_Decimation_Mode32" "C_ADC_Fifo32_Enable" "C_ADC_Data_Type33" "C_ADC_Data_Width33" "C_ADC_Decimation_Mode33" "C_ADC_Fifo33_Enable"
}

proc generate_config_file {drv_handle file_name drv_string args} {
    set args [::hsi::utils::get_exact_arg_list $args]
    set filename [file join "src" $file_name]
    #Fix for CR 784758
    #file delete $filename
    set config_file [open $filename w]
    ::hsi::utils::write_c_header $config_file "Driver configuration"
    puts $config_file "#include \"xparameters.h\""
    puts $config_file "#include \"[string tolower $drv_string].h\""
    puts $config_file "\n/*"
    puts $config_file "* The configuration table for devices"
    puts $config_file "*/\n"
    set num_insts [::hsi::utils::get_driver_param_name $drv_string "NUM_INSTANCES"]
    puts $config_file [format "%s_Config %s_ConfigTable\[%s\] =" $drv_string $drv_string $num_insts]
    puts $config_file "\{"
    set periphs [::hsi::utils::get_common_driver_ips $drv_handle]
    set start_comma ""
    foreach periph $periphs {
        puts $config_file [format "%s\t\{" $start_comma]
        set comma ""
		set len [llength $args]
        foreach arg $args {
            if {[string compare -nocase "DEVICE_ID" $arg] == 0} {
                puts -nonewline $config_file [format "%s\t\t%s,\n" $comma [::hsi::utils::get_ip_param_name $periph $arg]]
                continue
            }
            # Check if this is a driver parameter or a peripheral parameter
            set value [common::get_property CONFIG.$arg $drv_handle]
            if {[llength $value] == 0} {
                set local_value [common::get_property CONFIG.$arg $periph ]
                # If a parameter isn't found locally (in the current
                # peripheral), we will (for some obscure and ancient reason)
                # look in peripherals connected via point to point links
                if { [string compare -nocase $local_value ""] == 0} {
                    set p2p_name [::hsi::utils::get_p2p_name $periph $arg]
                    if { [string compare -nocase $p2p_name ""] == 0} {
                        puts -nonewline $config_file [format "%s\t\t%s" $comma [::hsi::utils::get_ip_param_name $periph $arg]]
                    } else {
                        puts -nonewline $config_file [format "%s\t\t%s" $comma $p2p_name]
                    }
                } else {
					if {[string compare -nocase "C_DAC0_Enable" $arg] == 0} {
						puts $config_file ",\n\t\t{"
						for {set tile 0} {$tile < 4} {incr tile} {
							puts $config_file "\t\t\t{"
							puts -nonewline $config_file [format "\t\t\t\t%s,\n" [::hsi::utils::get_ip_param_name $periph $arg]]
							set tile_index 0
							set pos [lsearch $args $arg]
							set nest_list [lrange $args $pos+1 $len-1]
							foreach arg $nest_list {
								if {$tile_index < 4} {
									puts -nonewline $config_file [format "\t\t\t\t%s,\n" [::hsi::utils::get_ip_param_name $periph $arg]]
									incr tile_index
									continue
								} else {
									puts $config_file "\t\t\t\t{"
									for {set block 0} {$block < 4} {incr block} {
										puts $config_file "\t\t\t\t\t{"
										puts -nonewline $config_file [format "\t\t\t\t\t\t%s,\n" [::hsi::utils::get_ip_param_name $periph $arg]]
										set block_index 0
										set pos [lsearch $args $arg]
										set nest_list [lrange $args $pos+1 $len-1]
										foreach arg $nest_list {
											if {$block_index < 3} {
												puts -nonewline $config_file [format "\t\t\t\t\t\t%s,\n" [::hsi::utils::get_ip_param_name $periph $arg]]
												incr block_index
												continue
											}
											break
										}
										puts -nonewline $config_file "\t\t\t\t\t},\n"
									}
									puts -nonewline $config_file "\t\t\t\t},\n"

									puts $config_file "\t\t\t\t{"
									for {set block 0} {$block < 4} {incr block} {
										puts $config_file "\t\t\t\t\t{"
										puts -nonewline $config_file [format "\t\t\t\t\t\t%s,\n" [::hsi::utils::get_ip_param_name $periph $arg]]
										set block_index 0
										set pos [lsearch $args $arg]
										set nest_list [lrange $args $pos+1 $len-1]
										#puts "rdfc_sai $nest_list arg $arg\n\r"
										foreach arg $nest_list {
											if {$block_index < 4} {
												puts -nonewline $config_file [format "\t\t\t\t\t\t%s,\n" [::hsi::utils::get_ip_param_name $periph $arg]]
												incr block_index
												continue
											}
											break
										}
										puts -nonewline $config_file "\t\t\t\t\t},\n"
									}
									puts -nonewline $config_file "\t\t\t\t},\n"
								}
								break
							}
							puts -nonewline $config_file "\t\t\t},\n"
						}
						puts -nonewline $config_file "\t\t}"
					}
					if {[string compare -nocase "C_ADC0_Enable" $arg] == 0} {
						puts $config_file ",\n\t\t{"
						for {set tile 0} {$tile < 4} {incr tile} {
							puts $config_file "\t\t\t{"
							puts -nonewline $config_file [format "\t\t\t\t%s,\n" [::hsi::utils::get_ip_param_name $periph $arg]]
							set tile_index 0
							set pos [lsearch $args $arg]
							set nest_list [lrange $args $pos+1 $len-1]
							#puts "rdfc_sai $nest_list arg $arg\n\r"
							foreach arg $nest_list {
								if {$tile_index < 4} {
									puts -nonewline $config_file [format "\t\t\t\t%s,\n" [::hsi::utils::get_ip_param_name $periph $arg]]
									incr tile_index
									continue
								} else {
									puts $config_file "\t\t\t\t{"
									for {set block 0} {$block < 4} {incr block} {
										puts $config_file "\t\t\t\t\t{"
										puts -nonewline $config_file [format "\t\t\t\t\t\t%s,\n" [::hsi::utils::get_ip_param_name $periph $arg]]
										set block_index 0
										set pos [lsearch $args $arg]
										set nest_list [lrange $args $pos+1 $len-1]
										foreach arg $nest_list {
											if {$block_index < 1} {
												puts -nonewline $config_file [format "\t\t\t\t\t\t%s,\n" [::hsi::utils::get_ip_param_name $periph $arg]]
												incr block_index
												continue
											}
											break
										}
										puts -nonewline $config_file "\t\t\t\t\t},\n"
									}
									puts -nonewline $config_file "\t\t\t\t},\n"

									puts $config_file "\t\t\t\t{"
									for {set block 0} {$block < 4} {incr block} {
										puts $config_file "\t\t\t\t\t{"
										puts -nonewline $config_file [format "\t\t\t\t\t\t%s,\n" [::hsi::utils::get_ip_param_name $periph $arg]]
										set block_index 0
										set pos [lsearch $args $arg]
										set nest_list [lrange $args $pos+1 $len-1]
										foreach arg $nest_list {
											if {$block_index < 3} {
												puts -nonewline $config_file [format "\t\t\t\t\t\t%s,\n" [::hsi::utils::get_ip_param_name $periph $arg]]
												incr block_index
												continue
											}
											break
										}
										puts -nonewline $config_file "\t\t\t\t\t},\n"
									}
									puts -nonewline $config_file "\t\t\t\t},\n"
								}
								break
							}
							puts -nonewline $config_file "\t\t\t},\n"
						}
						puts -nonewline $config_file "\t\t}"
					} else {
						puts -nonewline $config_file [format "%s\t\t%s" $comma [::hsi::utils::get_ip_param_name $periph $arg]]
					}
                }
            } else {
				puts -nonewline $config_file [format "%s\t\t%s" $comma [::hsi::utils::get_driver_param_name $drv_string $arg]]
            }
            set comma ",\n"
			set pos [lsearch $args $arg]
			set args [lrange $args $pos+1 $len-1]
			if {$args == ""} {break}
        }
        puts -nonewline $config_file "\n\t\}"
        set start_comma ",\n"
    }
    puts $config_file "\n\};"

    puts $config_file "\n";

    close $config_file
}

#
# generate_canonical_xpars - Used to print out canonical defines for a driver.
# Given a list of arguments, define each as a canonical constant name, using
# the driver name, in an include file.
#
proc generate_canonical_xpars {drv_handle file_name drv_string args} {
    set args [::hsi::utils::get_exact_arg_list $args]
   # Open include file
   set file_handle [::hsi::utils::open_include_file $file_name]

   # Get all the peripherals connected to this driver
   set periphs [::hsi::utils::get_common_driver_ips $drv_handle]

   # Get the names of all the peripherals connected to this driver
   foreach periph $periphs {
       set peripheral_name [string toupper [common::get_property NAME $periph]]
       lappend peripherals $peripheral_name
   }

   # Get possible canonical names for all the peripherals connected to this
   # driver
   set device_id 0
   foreach periph $periphs {
       set canonical_name [string toupper [format "%s_%s" $drv_string $device_id]]
       lappend canonicals $canonical_name

       # Create a list of IDs of the peripherals whose hardware instance name
       # doesn't match the canonical name. These IDs can be used later to
       # generate canonical definitions
       if { [lsearch $peripherals $canonical_name] < 0 } {
           lappend indices $device_id
       }
       incr device_id
   }

   set i 0
   foreach periph $periphs {
       set periph_name [string toupper [common::get_property NAME $periph]]

       # Generate canonical definitions only for the peripherals whose
       # canonical name is not the same as hardware instance name
       if { [lsearch $canonicals $periph_name] < 0 } {
           puts $file_handle "/* Canonical definitions for peripheral $periph_name */"
           set canonical_name [format "%s_%s" $drv_string [lindex $indices $i]]

           foreach arg $args {
               set lvalue [::hsi::utils::get_driver_param_name $canonical_name $arg]

               # The commented out rvalue is the name of the instance-specific constant
               # set rvalue [::hsi::utils::get_ip_param_name $periph $arg]
               # The rvalue set below is the actual value of the parameter
               set rvalue [::hsi::utils::get_param_value $periph $arg]
               if {[llength $rvalue] == 0} {
                   set rvalue 0
               }
               set rvalue [::hsi::utils::format_addr_string $rvalue $arg]

				if {[string compare -nocase "false" $rvalue] == 0} {
					set rvalue 0
				}
				if {[string compare -nocase "true" $rvalue] == 0} {
					set rvalue 1
				}
				if {[string compare -nocase "design1usprfdataconverter00" $rvalue] == 0} {
					set rvalue 1
				}
               puts $file_handle "#define $lvalue $rvalue"

           }
           puts $file_handle ""
           incr i
       }
   }

   puts $file_handle "\n/******************************************************************/\n"
   close $file_handle
}

#
# Given a list of arguments, define them all in an include file.
# Handles IP model/user parameters, as well as the special parameters NUM_INSTANCES,
# DEVICE_ID
# Will not work for a processor.
#
proc generate_include_file {drv_handle file_name drv_string args} {
    set args [::hsi::utils::get_exact_arg_list $args]
    # Open include file
    set file_handle [::hsi::utils::open_include_file $file_name]

    # Get all peripherals connected to this driver
    set periphs [::hsi::utils::get_common_driver_ips $drv_handle]

    # Handle special cases
    set arg "NUM_INSTANCES"
    set posn [lsearch -exact $args $arg]
    if {$posn > -1} {
        puts $file_handle "/* Definitions for driver [string toupper [common::get_property name $drv_handle]] */"
        # Define NUM_INSTANCES
        puts $file_handle "#define [::hsi::utils::get_driver_param_name $drv_string $arg] [llength $periphs]"
        set args [lreplace $args $posn $posn]
    }

    # Check if it is a driver parameter
    lappend newargs
    foreach arg $args {
        set value [common::get_property CONFIG.$arg $drv_handle]
        if {[llength $value] == 0} {
            lappend newargs $arg
        } else {
            puts $file_handle "#define [::hsi::utils::get_driver_param_name $drv_string $arg] [common::get_property $arg $drv_handle]"
        }
    }
    set args $newargs

    # Print all parameters for all peripherals
    set device_id 0
    foreach periph $periphs {
        puts $file_handle ""
        puts $file_handle "/* Definitions for peripheral [string toupper [common::get_property NAME $periph]] */"
        foreach arg $args {
            if {[string compare -nocase "DEVICE_ID" $arg] == 0} {
                set value $device_id
                incr device_id
            } else {
                set value [common::get_property CONFIG.$arg $periph]
            }
            if {[llength $value] == 0} {
                set value 0
            }
			if {[string compare -nocase "false" $value] == 0} {
				set value 0
			}
			if {[string compare -nocase "true" $value] == 0} {
				set value 1
			}
			if {[string compare -nocase "design_1_usp_rf_data_converter_0_0" $value] == 0} {
				set value 1
			}
            set value [::hsi::utils::format_addr_string $value $arg]
            if {[string compare -nocase "HW_VER" $arg] == 0} {
                puts $file_handle "#define [::hsi::utils::get_ip_param_name $periph $arg] \"$value\""
            } else {
                puts $file_handle "#define [::hsi::utils::get_ip_param_name $periph $arg] $value"
            }
        }
        puts $file_handle ""
    }
    puts $file_handle "\n/******************************************************************/\n"
    close $file_handle
}