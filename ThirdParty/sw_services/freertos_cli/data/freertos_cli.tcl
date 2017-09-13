#
#
proc freertos_cli_drc {os_handle} {

    puts "FreeRTOS CLI DRC ..."
    #global env

    #set sw_proc_handle [get_sw_processor]
    #set hw_proc_handle [get_cells [common::get_property HW_INSTANCE $sw_proc_handle] ]
    #set proctype [common::get_property IP_NAME $hw_proc_handle]
    #set procname [common::get_property NAME    $hw_proc_handle]

}

proc generate_freertos_cli_opts {libhandle} {
    # generate an opts file
    #

    set cliopts_file "src/Source/cliopts.h"
    set cliopts_fd [open $cliopts_file w]

    puts $cliopts_fd "\#ifndef _CLIOPTS_H"
    puts $cliopts_fd "\#define _CLIOPTS_H"
    puts $cliopts_fd ""

    set user_buffer [expr [common::get_property CONFIG.user_buffer $libhandle]]
    if {$user_buffer == "true"} {
        puts $cliopts_fd "\#define configAPPLICATION_PROVIDES_cOutputBuffer 1"
    }
    set buffer_size [common::get_property CONFIG.buffer_size $libhandle]
    puts $cliopts_fd "\#define configCOMMAND_INT_MAX_OUTPUT_SIZE ($buffer_size)"
    puts $cliopts_fd ""

    set cli_enable [expr [common::get_property CONFIG.cli_enable $libhandle] == true]
    set cli_close [expr [common::get_property CONFIG.cli_close $libhandle] == true]
    puts $cliopts_fd "\#define CLI_ENABLE $cli_enable"
    puts $cliopts_fd "\#define CLI_CLOSE $cli_close"
    puts $cliopts_fd ""

    puts $cliopts_fd "\#endif"
    close $cliopts_fd

}

#-------
# generate: called after OS and library files are copied into project dir
# we need to generate the following:
# 1. Makefile options
# 2. System Arch settings for lwIP to use
#-------
proc generate {libhandle} {
    generate_freertos_cli_opts $libhandle
}

#-------
# post_generate: called after generate called on all libraries
#-------
proc post_generate {libhandle} {
}

#-------
# execs_generate: called after BSP's, libraries and drivers have been compiled
# This procedure builds the liblwip4.a library
#-------
proc execs_generate {libhandle} {
}
