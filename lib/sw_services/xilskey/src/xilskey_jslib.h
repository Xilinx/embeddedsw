/******************************************************************************
*
* Copyright (C) 2013 - 2014 Xilinx, Inc.  All rights reserved.
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
* Use of the Software is limited solely to applications:
* (a) running on a Xilinx device, or
* (b) that interact with a Xilinx device through a bus or interconnect.
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

/*
 * JTAG server implementation library interface
 *
 * This library interface is intended to be use by JTAG server
 * implementations only, i.e. not used by JTAG server clients.
 */

#include "xilskey_js.h"

#ifdef __cplusplus
extern "C" {
#endif


typedef struct js_lib_server_struct js_lib_server_t;
typedef struct js_lib_port_struct js_lib_port_t;
typedef struct js_lib_command_sequence_struct js_lib_command_sequence_t;
typedef struct js_lib_command_struct js_lib_command_t;
typedef struct js_lib_command_buffer_struct js_lib_command_buffer_t;
typedef struct js_lib_state_info_struct js_lib_state_info_t;



/*
 * JTAG server object
 *
 * Server instances are created by implementation specific create
 * functions, typically named js_create_<implementation-name>.
 *
 * TODO: Add a way to schedule a command sequence to be repeated at
 * regular intervals.  This is useful to poll for events, like
 * breakpoint hit.
 */
struct js_server_struct {
    char dummy;
};


/*
 * JTAG server library object
 */
struct js_lib_server_struct {
    /* Public part of object */
    js_server_t base;

    /* See js_get_port_descr_list() */
    int (*get_port_descr_list)(
        js_lib_server_t *server,
        js_port_descr_t **port_listp);

    /* See js_open_port() */
    int (*open_port)(
        js_lib_server_t *server,
        js_port_descr_t *port_descr,
        js_lib_port_t **port);

    /* See js_deinit_server() */
    void (*deinit_server)(
        js_lib_server_t *server);

    /* Error of last command */
    char last_error[100];
};

struct js_lib_port_struct {
    /* Base class - must be first. */
    js_port_t base;

    /* See js_get_property() */
    int (*get_property)(
        js_lib_port_t *port,
        js_property_kind_t kind,
        js_property_value_t *valuep);

    /* See js_set_property() */
    int (*set_property)(
        js_lib_port_t *port,
        js_property_kind_t kind,
        js_property_value_t value);

    /* See js_run_command_sequence() */
    int (*run_command_sequence)(
        js_lib_command_sequence_t *commands);

    /* See js_close_port() */
    int (*close_port)(
        js_lib_port_t *port);
};


struct js_lib_command_buffer_struct {
    js_lib_command_buffer_t *next;

    /* First available byte in buffer */
    unsigned char *buf_free;

    /* End of buffer */
    unsigned char *buf_end;

    /* Start of buffer (must be last) */
    unsigned char buf_start[1];
};


/*
 * JTAG command sequence library object
 */
struct js_lib_command_sequence_struct {
    /* Client visible part */
    js_command_sequence_t base;

    /* Array of commands */
    js_lib_command_t *cmd_list;

    /* Count of added commands */
    unsigned int cmd_count;

    /* Command list capacity */
    unsigned int cmd_max;

    /* Size of command objects  */
    unsigned int cmd_size;

    /* Command buffer pool */
    js_lib_command_buffer_t *buf_head;
};


/*
 * JTAG command object
 *
 * Command object are added to JTAG command sequence object using
 * js_add_* functions.
 *
 * TODO: Add commands to get/set GPIO pins, conditional execution and
 * loops.
 */
typedef enum {
    JS_CMD_SET_STATE,
    JS_CMD_SHIFT
} js_command_kind_t;

struct js_lib_command_struct {
    /* Command to execute */
    js_command_kind_t kind;

    /* Flags for command */
    unsigned int flags;

    /* State after command */
    js_state_t state;

    /* Command specific count */
    size_t count;

    /* TDI buffer pointer */
    unsigned char *tdi_buf;

    /* TDO buffer pointer */
    unsigned char *tdo_buf;
};


struct js_lib_state_info_struct {
    /* Name of state */
    const char *name;

    /* TMS bits to move to other state */
    unsigned char state_tms[JS_STATE_MAX];

    /* Clocks to move to other state */
    unsigned char state_clk[JS_STATE_MAX];

    /* Nearest stable state */
    js_state_t stable_state;
};


extern js_lib_state_info_t js_lib_state_info[JS_STATE_MAX];

extern int js_lib_normalize_command_sequence(
    js_lib_command_sequence_t *dest,
    js_lib_command_sequence_t *src);

#ifdef __cplusplus
}
#endif
