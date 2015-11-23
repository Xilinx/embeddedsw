# 
#*****************************************************************************
#
#       XILINX IS PROVIDING THIS DESIGN, CODE, OR INFORMATION "AS IS"
#       AS A COURTESY TO YOU, SOLELY FOR USE IN DEVELOPING PROGRAMS AND
#       SOLUTIONS FOR XILINX DEVICES.  BY PROVIDING THIS DESIGN, CODE,
#       OR INFORMATION AS ONE POSSIBLE IMPLEMENTATION OF THIS FEATURE,
#       APPLICATION OR STANDARD, XILINX IS MAKING NO REPRESENTATION
#       THAT THIS IMPLEMENTATION IS FREE FROM ANY CLAIMS OF INFRINGEMENT,
#       AND YOU ARE RESPONSIBLE FOR OBTAINING ANY RIGHTS YOU MAY REQUIRE
#       FOR YOUR IMPLEMENTATION.  XILINX EXPRESSLY DISCLAIMS ANY
#       WARRANTY WHATSOEVER WITH RESPECT TO THE ADEQUACY OF THE
#       IMPLEMENTATION, INCLUDING BUT NOT LIMITED TO ANY WARRANTIES OR
#       REPRESENTATIONS THAT THIS IMPLEMENTATION IS FREE FROM CLAIMS OF
#       INFRINGEMENT, IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
#       FOR A PARTICULAR PURPOSE.
#
#       (c) Copyright 2002, 2003 Xilinx Inc.
#       All rights reserved.
#
#*****************************************************************************/
#
# Flash Writer for the Toshiba TH50VSF2580/2581AASB and
# TH50VSF3680/3681AASB
#
#
# Flash Memory Base Address. This should be set to the value in the current
# system as specified in .mhs file.
#
set flash_base_addr 0x1f800000
set flash_cur_write_addr $flash_base_addr
set flash_generic_target -1
set flash_board_target 0

# points to the next unerased block
set flash_new_block_addr $flash_base_addr
set flash_max_empty_addr $flash_new_block_addr
set flash_num_blocks 0
set flash_block_size 0
set flash_max_addr 0
set flash_init 0 

# All the Flash command addresses are shifted by 2 since the addr 
# bits 30 and 31 are not connected to the Flash/SRAM modules
# Ex : 0x1554 = 0x555 (ID code addr (bytemode)) << 2

#..... From the .ucf file..
# ..."addr bits 30 and 31 are not used since the four "flash_sram_ben" ...
# take care of addressing the two Flash/SRAM Multi-chip modules" ...
#
set command_addr_1 [expr $flash_base_addr + 0x1554]
set command_addr_2 [expr $flash_base_addr + 0xaa8]
set command_addr_3 [expr $flash_base_addr + 0x1554]
set command_1 0x00aa00aa
set command_2 0x00550055

set manufacturer_code -1
set device_code -1

# TODO get it from the user
set board_tgt 0

#
# Set the flash to Read or Reset mode.
#
proc flash_read_reset { } {
    global flash_base_addr
    global command_addr_1 command_addr_2 command_addr_3
    global command_1 command_2

    mwr $command_addr_1 $command_1
    mwr $command_addr_2 $command_2
    mwr $command_addr_3 0x00f000f0
}


#
# Read the Device and Manufacturer ID of the flash
#
proc id_test { } {
    global flash_base_addr
    global manufacturer_code device_code
    global command_addr_1 command_addr_2 command_addr_3
    global command_1 command_2
    global board_tgt
    
    mwr $command_addr_1 $command_1
    mwr $command_addr_2 $command_2
    mwr $command_addr_3 0x00900090
    set manufacturer_code [mem_read_word $flash_base_addr]
    set device_code [mem_read_word [expr $flash_base_addr + 4]]
    flash_read_reset
}


#
# Initialize the flash. 
# - Creates the flash target and registers the funtion with xmd.
# - Initializes the parameters for the respective flash
#
proc flash_init {flash_addr board_target} {
    global flash_base_addr flash_board_target flash_generic_target
    global manufacturer_code device_code
    global flash_num_blocks flash_block_size
    global flash_max_addr

    set flash_base_addr $flash_addr

    set tgtlist [xtargets $board_target]
    if { [llength $tgtlist] == 0 } {
	error "Target no $board_target is not a valid xmd target"
    }
    set flash_board_target $board_target

    set flash_generic_target [xconnect generic 34 dummy flash_wrreg dummy flash_data_wrmem dummy dummy dummy]

    # get dev and manu codes
    id_test

    if { $device_code == 0x009c009c && $manufacturer_code == 0x00980098 } {
	puts "\tFlash Manufacturer = Toshiba"
	puts "\tFlash Part Number  = TH50VSF2581AASB"
	set flash_num_blocks 71
	set flash_block_size 0x1000
	set flash_max_addr [expr 0x200000 + $flash_base_addr]
    } elseif { $device_code == 0x00950095 && $manufacturer_code == 0x00980098 } {
	puts "\tFlash Manufacturer = Toshiba"
	puts "\tFlash Part Number  = TH50VSF3681AASB"
	set flash_num_blocks 135
	set flash_block_size 0x1000
	set flash_max_addr [expr 0x400000 + $flash_base_addr]
    } else {
	puts [format "Unknown Manufacturer/Device code 0x%08x 0x%08x" $manufacturer_code $device_code]
    }
    
    return 
}


#
# 
proc flash_dow { filename args } {
    global flash_generic_target $flash_base_addr
    if { $args == "" } {
    	xdownload $flash_generic_target $filename $flash_base_addr
    } else  {
    	xdownload $flash_generic_target $filename $args
    }
}
proc flash_data_dow { filename args } {
    global flash_generic_target
    if { $args == "" } {
    	xdownload $flash_generic_target -data $filename $flash_base_addr
    } else  {
    	xdownload $flash_generic_target -data $filename $args
    }
}

proc flash_chip_erase { } {
    global command_addr_1 command_addr_2 command_addr_3
    global command_1 command_2
    
    mwr $command_addr_1 $command_1
    mwr $command_addr_2 $command_2
    mwr $command_addr_3 0x00800080    
    mwr $command_addr_1 $command_1
    mwr $command_addr_2 $command_2
    mwr $command_addr_3 0x00100010
    d6_toggle $command_addr_1
}    
 
proc d6_toggle { addr } {
	set status_flag 0
	set prev_flag 0
	set next_flag 0
	
	set status_flag [mem_read_word $addr]
    	while { 1 } {
		set prev_flag [expr $status_flag & 0x00400040]
		set status_flag [mem_read_word $addr]
		set next_flag [expr $status_flag & 0x00400040]
		if { [expr $prev_flag ^ $next_flag] } {
			if { [expr $status_flag & 0x00200020] == 0x00200020 } {
				set status_flag	[mem_read_word $addr] 
				set prev_flag [expr $status_flag & 0x00400040]
				set status_flag	[mem_read_word $addr] 
				set next_flag [expr $status_flag & 0x00400040]
				if { [expr $prev_flag ^ $next_flag] } {
					break 
				} else {
					flash_read_reset
					error "Operation failed"
				}
			}
		} else {
			break 
		}
	}
}



proc dummy { } {
    puts "Generic Flash target does not support this command"
}


# Write a single word using the Auto Program mode
proc flash_auto_program {addr data} {
    global command_addr_1 command_addr_2 command_addr_3
    global command_1 command_2

    mwr $command_addr_1 $command_1
    mwr $command_addr_2 $command_2
    mwr $command_addr_3 0x00a000a0
    mwr $addr $data
    d6_toggle $addr 
}


# fast program mode set
proc flash_fast_program_set { } {
    global command_addr_1 command_addr_2 command_addr_3
    global command_1 command_2

#    puts "Starting Fast Program Mode"

    mwr $command_addr_1 $command_1
    mwr $command_addr_2 $command_2
    mwr $command_addr_3 0x00200020    
}

# fast program mode write
proc flash_fast_program { addr data } {
    mwr $addr 0x00a000a0
    mwr $addr $data
    d6_toggle $addr 
}


# fast program mode reset
proc flash_fast_program_reset { } {
    global flash_base_addr 
    #   puts "Stopping Fast Program Mode"

    mwr $flash_base_addr 0x00900090
    mwr $flash_base_addr 0x00f000f0
}


proc flash_block_erase { num_blocks } {
    global flash_num_blocks 
    global flash_block_size 
    global flash_new_block_addr
    global flash_max_addr
    global flash_max_empty_addr 
    global flash_base_addr
    global command_addr_1 command_addr_2 command_addr_3
    global command_1 command_2
    
#    puts [format "new_addr = 0x%08x, max_addr = 0x%08x" $flash_new_block_addr $flash_max_empty_addr]
    if { $flash_new_block_addr >= $flash_max_addr } {
	error "Error: Not enough space in flash"
    }

    flash_read_reset
    for { set i 0 } { $i < $num_blocks } { incr i } {
    	mwr $command_addr_1 $command_1
	mwr $command_addr_2 $command_2
	mwr $command_addr_3 0x00800080    
    	mwr $command_addr_1 $command_1
    	mwr $command_addr_2 $command_2
    	mwr $flash_new_block_addr 0x00300030
    	d6_toggle $flash_new_block_addr

    	if { $flash_new_block_addr < [expr 0x8000 + $flash_base_addr] } {
	   incr flash_new_block_addr $flash_block_size
    	} else {
	   set flash_block_size 0x8000
	   incr flash_new_block_addr $flash_block_size
    	}
    	set flash_max_empty_addr [expr $flash_max_empty_addr + $flash_block_size*2] 
    }
}



# Write the data to flash Memory
# - bytes: The data to be written to flash
# - addr:  The target memory address
# - num:   Size of the data in bytes
proc flash_data_wrmem {bytes addr num } {
    global flash_cur_write_addr 
    global flash_board_target 
    global flash_new_block_addr 
    global flash_max_empty_addr 
    global flash_max_addr
    global flash_block_size
    global flash_init

    if { $flash_init == 0 } {
	    flash_read_reset
	    puts "\nDownloading data to flash...\n"
            set flash_init 1
    }
    
    # Write the chunk size
    puts [format "Chunk Length: 0x%08x, Addr: 0x%08x" $num $flash_cur_write_addr]
    
    # flash_new_block_addr is addr range in word mode and we have 2 flash chips
    # Check if space available in the current flash block to accomadate the
    # section. If not, then call block erase on the next flash block.
    set num_unaligned_bytes [expr $num%4]
    set num_pad_bytes [expr (4 - $num_unaligned_bytes)%4]
    if { [expr $flash_cur_write_addr + $num + $num_pad_bytes] >= $flash_max_empty_addr } { 
	set free_blk_size [expr (($num+$num_pad_bytes)-($flash_max_empty_addr-$flash_cur_write_addr))]
	set num_block_erase [ expr int( ceil([expr double($free_blk_size)/double($flash_block_size)]) ) ]
	flash_block_erase $num_block_erase
    }
    
    flash_fast_program_set

    set cur_word 0
    set temp_bytes 0
    set i 0
    for { } {$i < [expr $num - $num_unaligned_bytes] } {incr i 4} {
	set fmt_str [format "@%dI" $i]
	binary scan $bytes $fmt_str cur_word
	flash_fast_program $flash_cur_write_addr $cur_word
	# puts [format "writing Addr: 0x%08x" $flash_cur_write_addr]
	incr addr 4
	incr flash_cur_write_addr 4
    }
    
    if { $num_pad_bytes != 0 } {
	set cur_byte 0
	set aligned_word 0
	for {} {$i < $num } { incr i } {
	    set fmt_str [format "@%dc" [expr $i]]
	    binary scan $bytes $fmt_str cur_byte
	    set aligned_word [expr ($aligned_word << 8) + $cur_byte]
	}
	for {set i 0} { $i < $num_pad_bytes } { incr i } {
	    set aligned_word [expr ($aligned_word << 8)]

	}
	flash_fast_program $flash_cur_write_addr $aligned_word
	incr addr 4
	incr flash_cur_write_addr 4
    }
    flash_fast_program_reset
    puts [format "Completed Section Length: 0x%08x, Addr: 0x%08x" $num $flash_cur_write_addr]
    return
}

# Write the section to flash Memory
# - bytes: The data to be written to flash
# - addr:  The target memiry address of the section.
# - num:   Size of the section in bytes
proc flash_wrmem {bytes addr num } {
    global flash_cur_write_addr 
    global flash_board_target 
    global flash_new_block_addr 
    global flash_max_empty_addr 
    global flash_max_addr
    global flash_block_size
    global flash_init

    if { $flash_init == 0 } {
	    flash_read_reset
	    puts "\nDownloading program to flash...\n"
    }
    
    # Write the Section size, 
    puts [format "Section Length: 0x%08x, Addr: 0x%08x" $num $flash_cur_write_addr]
    
    # The following data is written to the flash in the following order:
    #
    # - Size of the section in bytes (4)
    # - Target address for the section (4)
    # - Section data. The data is padded so that it is Word aligned. 

    # flash_new_block_addr is addr range in word mode and we have 2 flash chips
    # Check if space available in the current flash block to accomadate the
    # section. If not, then call block erase on the next flash block.
    set num_unaligned_bytes [expr $num%4]
    set num_pad_bytes [expr (4 - $num_unaligned_bytes)%4]
    if { [expr $flash_cur_write_addr + 8 + $num + $num_pad_bytes] >= $flash_max_empty_addr } { 
	set free_blk_size [expr ((8+$num+$num_pad_bytes)-($flash_max_empty_addr-$flash_cur_write_addr))]
	set num_block_erase [ expr int( ceil([expr double($free_blk_size)/double($flash_block_size)]) ) ]
	flash_block_erase $num_block_erase
    }
    
    flash_fast_program_set

    #
    # Write the magic numbers at the beginning of flash.
    #
    if { $flash_init == 0 } {
            flash_fast_program $flash_cur_write_addr 0xf0f0f0f0
            incr flash_cur_write_addr 4
            flash_fast_program $flash_cur_write_addr 0x0f0f0f0f
            incr flash_cur_write_addr 4
	    set flash_init 1
    }

    flash_fast_program $flash_cur_write_addr $num
    incr flash_cur_write_addr 4
    flash_fast_program $flash_cur_write_addr $addr
    incr flash_cur_write_addr 4

    set cur_word 0
    set temp_bytes 0
    set i 0
    for { } {$i < [expr $num - $num_unaligned_bytes] } {incr i 4} {
	set fmt_str [format "@%dI" $i]
	binary scan $bytes $fmt_str cur_word
	flash_fast_program $flash_cur_write_addr $cur_word
	incr addr 4
	incr flash_cur_write_addr 4
    }
    
    if { $num_pad_bytes != 0 } {
	set cur_byte 0
	set aligned_word 0
	for {} {$i < $num } { incr i } {
	    set fmt_str [format "@%dc" [expr $i]]
	    binary scan $bytes $fmt_str cur_byte
	    set aligned_word [expr ($aligned_word << 8) + $cur_byte]
	}
	for {set i 0} { $i < $num_pad_bytes } { incr i } {
	    set aligned_word [expr ($aligned_word << 8)]

	}
	flash_fast_program $flash_cur_write_addr $aligned_word
	incr addr 4
	incr flash_cur_write_addr 4
    }
    flash_fast_program_reset
    puts [format "Completed Section Length: 0x%08x, Addr: 0x%08x" $num $flash_cur_write_addr]
    return
}


proc flash_wrreg {reg value} {
    global flash_cur_write_addr 
    global flash_new_block_addr 
    global flash_max_empty_addr 
    global flash_block_size

    puts [format "Program Start address: 0x%08x" $value]
    
    # flash_new_block_addr is addr range in word mode and we have 2 flash chips
    # Check if space available in the current flash block to accomadate the
    # section. If not, then call block erase on the next flash block.

    if { [expr $flash_cur_write_addr + 4] >= $flash_max_empty_addr } {
	flash_block_erase 1
    }
    
    if { $reg != 32 } {
	error "Error: Dont know how to write register $reg"
    }

    flash_fast_program_set
    flash_fast_program $flash_cur_write_addr 0
    incr flash_cur_write_addr 4
    flash_fast_program $flash_cur_write_addr $value
    flash_fast_program_reset

    incr flash_cur_write_addr 4    
    flash_read_reset
    return 
}
