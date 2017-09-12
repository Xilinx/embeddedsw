#
#
proc freertos_cli_drc {os_handle} {

    puts "FreeRTOS CLI DRC ..."
    #global env

    #set sw_proc_handle [get_sw_processor]
    #set hw_proc_handle [get_cells [get_property HW_INSTANCE $sw_proc_handle] ]
    #set proctype [get_property IP_NAME $hw_proc_handle]
    #set procname [get_property NAME    $hw_proc_handle]

}

proc generate_freertos_cli_opts {libhandle} {
    # generate an opts file
    #

    set cliopts_file "src/cliopts.h"
    set cliopts_fd [open $cliopts_file w]

    puts $cliopts_fd "\#ifndef _FREERTOS_CLI_H"
    puts $cliopts_fd "\#define _FREERTOS_CLI_H"
    puts $cliopts_fd ""

    set cli_enable [expr [get_property CONFIG.cli_enable $libhandle] == true]
    set cli_close [expr [get_property CONFIG.cli_close $libhandle] == true]
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
