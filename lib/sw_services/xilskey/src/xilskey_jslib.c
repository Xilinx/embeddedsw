/******************************************************************************
* Copyright (c) 2013 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <assert.h>
#include "xilskey_jslib.h"

/**
* Maximum TAPS
*/
#define MAX_TAPS 16


js_lib_state_info_t js_lib_state_info[] = {
    /* JS_RESET */
    {
        "reset",
        { 0x01, 0x00, 0x02, 0x0a, 0x06, 0x16 },
        {    1,    1,    4,    5,    5,    6 },
        JS_RESET
    },

    /* JS_IDLE */
    {
        "idle",
        { 0x07, 0x00, 0x01, 0x05, 0x03, 0x0b },
        {    3,    1,    3,    4,    4,    5 },
        JS_IDLE
    },

    /* JS_SHIFT_DR */
    {
        "shift dr",
        { 0x1f, 0x03, 0x05, 0x01, 0x0f, 0x2f },
        {    5,    3,    4,    2,    6,    7 },
        JS_DRPAUSE
    },

    /* JS_PAUSE_DR */
    {
        "pause dr",
        { 0x1f, 0x03, 0x01, 0x00, 0x0f, 0x2f },
        {    5,    3,    2,    1,    6,    7 },
        JS_DRPAUSE
    },

    /* JS_SHIFT_IR */
    {
        "shift ir",
        { 0x1f, 0x03, 0x07, 0x01, 0x05, 0x01 },
        {    5,    4,    5,    6,    4,    2 },
        JS_IRPAUSE
    },

    /* JS_PAUSE_IR */
    {
        "pause ir",
        { 0x1f, 0x03, 0x07, 0x17, 0x01, 0x00 },
        {    5,    3,    5,    6,    2,    1 },
        JS_IRPAUSE
    }
};


const char *js_get_last_error(
    js_server_t *server)
{
    js_lib_server_t *js = (js_lib_server_t *)server;

    if (js->last_error[0] == '\0')
        return NULL;
    return js->last_error;
}


void js_set_last_error(
    js_server_t *server,
    const char *fmt,
    ...)
{
    js_lib_server_t *js = (js_lib_server_t *)server;
    va_list ap;

    va_start(ap, fmt);
    vsnprintf(js->last_error, sizeof js->last_error, fmt, ap);
    va_end(ap);
}


js_command_sequence_t *js_create_command_sequence(
    js_node_t *node)
{
    js_lib_command_sequence_t *cmds;

    cmds = (js_lib_command_sequence_t *)malloc(sizeof *cmds);
    if (cmds == NULL)
        return NULL;
    memset(cmds, 0, sizeof *cmds);
    cmds->base.node = node;
    cmds->cmd_size = sizeof *cmds->cmd_list;
    return &cmds->base;
}

/****************************************************************************/
/**
*
* This function frees the buffer
*
*****************************************************************************/
static void free_buffers(
    js_lib_command_buffer_t *buf)
{
    while (buf != NULL) {
        js_lib_command_buffer_t *next = buf->next;
        free(buf);
        buf = next;
    }
}


int js_clear_command_sequence(
    js_command_sequence_t *cmdseq)
{
    js_lib_command_sequence_t *cmds = (js_lib_command_sequence_t *)cmdseq;
    js_lib_command_buffer_t *buf = cmds->buf_head;

    cmds->cmd_count = 0;
    if (buf) {
        /* Free all buffers except to first one.  Keep the first one
         * since it is the larges and can be reused for future
         * commands */
        free_buffers(buf->next);
        buf->next = NULL;
        buf->buf_free = buf->buf_start;
    }
    return 0;
}


int js_delete_command_sequence(
    js_command_sequence_t *cmdseq)
{
    js_lib_command_sequence_t *cmds = (js_lib_command_sequence_t *)cmdseq;
    if (cmds->cmd_list)
        free(cmds->cmd_list);
    free_buffers(cmds->buf_head);
    free(cmds);
    return 0;
}

/****************************************************************************/
/**
*
* This function performs get command operation
*
* @return	returns return value of type js_lib_command_t
*
*****************************************************************************/
static js_lib_command_t *get_command(
    js_lib_command_sequence_t *cmds)
{
    js_lib_command_t *list = cmds->cmd_list;
    unsigned int count = cmds->cmd_count;
    js_lib_command_t *new_list = NULL;

    assert(cmds->cmd_size >= sizeof *list);
    if (count == cmds->cmd_max) {
        unsigned int max = count ? count * 2 : 1;
        new_list = (js_lib_command_t *)realloc(list, max * cmds->cmd_size);
        if (new_list == NULL) {
            js_set_last_error(cmds->base.node->port->server, "end of memory");
            return NULL;
        }
	list = new_list;
        cmds->cmd_list = list;
        cmds->cmd_max = max;
    }
    cmds->cmd_count = count + 1;
    return (js_lib_command_t *)((char *)list + count * cmds->cmd_size);
}


int js_add_state_change(
    js_command_sequence_t *cmdseq,
    js_state_t state,
    unsigned int clocks)
{
    js_lib_command_sequence_t *cmds = (js_lib_command_sequence_t *)cmdseq;
    js_lib_command_t *cmd = get_command(cmds);
    if (cmd == NULL)
        return -1;
    if (clocks != 0 && js_lib_state_info[state].stable_state != state) {
        js_set_last_error(cmds->base.node->port->server, "non zero clocks in unstable state");
        return -1;
    }
    cmd->kind = JS_CMD_SET_STATE;
    cmd->state = state;
    cmd->count = clocks;
    cmd->tdi_buf = NULL;
    cmd->tdo_buf = NULL;
    return 0;
}

/****************************************************************************/
/**
*
* This function performs get buffer operation
*
* @return	returns ret value
*
*****************************************************************************/
static unsigned char *get_buffer(
    js_lib_command_sequence_t *cmds,
    size_t size)
{
    js_lib_command_buffer_t *buf = cmds->buf_head;
    unsigned char *ret;

    assert(buf == NULL || buf->buf_start <= buf->buf_free);
    assert(buf == NULL || buf->buf_free < buf->buf_end);
    if (buf == NULL || buf->buf_free + size > buf->buf_end) {
        size_t buf_size = buf ? buf->buf_end - buf->buf_start : 1;
        while (buf_size < size * 2)
            buf_size *= 2;
        buf = (js_lib_command_buffer_t *)malloc(buf_size + sizeof *buf);
        if (buf == NULL)
            return NULL;
        buf->buf_free = buf->buf_start;
        buf->buf_end = buf->buf_start + buf_size;
        buf->next = cmds->buf_head;
        cmds->buf_head = buf;
    }
    ret = buf->buf_free;
    buf->buf_free += size;
    return ret;
}


int js_add_shift(
    js_command_sequence_t *cmdseq,
    unsigned int flags,
    size_t bits,
    unsigned char **tdi_buffer,
    unsigned char **tdo_buffer,
    js_state_t state)
{
    js_lib_command_sequence_t *cmds = (js_lib_command_sequence_t *)cmdseq;
    js_lib_command_t *cmd = get_command(cmds);
    if (cmd == NULL)
        return -1;
    if (bits == 0) {
        js_set_last_error(cmds->base.node->port->server, "bits argument must be > 0");
        return -1;
    }
    cmd->kind = JS_CMD_SHIFT;
    cmd->flags = flags;
    cmd->state = state;
    cmd->count = bits;
    if (tdi_buffer != NULL) {
        *tdi_buffer = cmd->tdi_buf = get_buffer(cmds, (bits + 7) / 8 + 2);
    } else {
        cmd->tdi_buf = NULL;
    }
    if (tdo_buffer != NULL) {
        *tdo_buffer = cmd->tdo_buf = get_buffer(cmds, (bits + 7) / 8 + 2);
    } else {
        cmd->tdo_buf = NULL;
    }
    return 0;
}


int js_detect_taps(
    js_port_t *port,
    uint32_t **resultp)
{
    js_command_sequence_t *cmds;
    unsigned char *dummycodes;
    unsigned char *idcodes;
    uint32_t *result = NULL;
    int max = 0;
    int ind = 0;
    int i;

    cmds = js_create_command_sequence(port->root_node);
    if (cmds == NULL)
        return -1;

    if (js_add_state_change(cmds, JS_RESET, 0) < 0 ||
        js_add_shift(cmds, 0, MAX_TAPS * 4 * 8, &dummycodes, &idcodes, JS_IDLE) < 0)
        goto return_error;

    for (i = 0; i < MAX_TAPS; i += 4) {
        dummycodes[i+0] = (JS_DUMMY_IDCODE) & 0xff;
        dummycodes[i+1] = (JS_DUMMY_IDCODE >> 8) & 0xff;
        dummycodes[i+2] = (JS_DUMMY_IDCODE >> 16) & 0xff;
        dummycodes[i+3] = (JS_DUMMY_IDCODE >> 24) & 0xff;
    }

    if (js_run_command_sequence(cmds) < 0)
        goto return_error;

    i = 0;
    while (i < MAX_TAPS * 4 * 8) {
        int lsb = (idcodes[i / 8] >> (i % 8)) & 1;
        uint32_t idcode;
        if (lsb) {
            int j;
            idcode = lsb;
            i++;
            for (j = 1; j < 32; j++) {
                lsb = (idcodes[i / 8] >> (i % 8)) & 1;
                idcode |= (uint32_t)lsb << j;
                i++;
            }
            if (idcode == JS_DUMMY_IDCODE)
                break;
        } else {
            /* TAP in BYPASS mode */
            idcode = JS_DUMMY_IDCODE;
            i++;
        }

        if (ind == max) {
            uint32_t *new_result;
            max = max ? max * 2 : 1;
            new_result = (uint32_t *)realloc(result, max * sizeof *result);
            if (new_result == NULL) {
                free(result);
                goto return_error;
            }
            result = new_result;
        }
        result[ind++] = idcode;
    }
    js_delete_command_sequence(cmds);
    *resultp = result;
    return ind;

return_error:
    js_delete_command_sequence(cmds);
    return -1;
}


int js_get_port_descr_list(
    js_server_t *server,
    js_port_descr_t **port_listp)
{
    js_lib_server_t *js = (js_lib_server_t *)server;

    return js->get_port_descr_list(js, port_listp);
}


int js_open_port(
    js_server_t *server,
    js_port_descr_t *port_descr,
    js_port_t **port)
{
    js_lib_server_t *js = (js_lib_server_t *)server;
    js_lib_port_t *result;
    int ret;

    ret = js->open_port(js, port_descr, &result);
    *port = (js_port_t *)result;
    return ret;
}


void js_deinit_server(
    js_server_t *server)
{
    js_lib_server_t *js = (js_lib_server_t *)server;

    js->deinit_server(js);
}


int js_get_property(
    js_port_t *port_arg,
    js_property_kind_t kind,
    js_property_value_t *valuep)
{
    js_lib_port_t *port = (js_lib_port_t *)port_arg;

    return port->get_property(port, kind, valuep);
}


int js_set_property(
    js_port_t *port_arg,
    js_property_kind_t kind,
    js_property_value_t value)
{
    js_lib_port_t *port = (js_lib_port_t *)port_arg;

    return port->set_property(port, kind, value);
}


int js_run_command_sequence(
    js_command_sequence_t *commands)
{
    js_lib_command_sequence_t *cmds = (js_lib_command_sequence_t *)commands;
    js_lib_port_t *port = (js_lib_port_t *)commands->node->port;

    return port->run_command_sequence(cmds);
}


int js_close_port(
        js_port_t *port_arg)
{
    js_lib_port_t *port = (js_lib_port_t *)port_arg;

    return port->close_port(port);
}

/****************************************************************************/
/**
*
* This function performs normalize command sequence operation
*
* @return	returns 0
*
*****************************************************************************/
int js_lib_normalize_command_sequence(
    js_lib_command_sequence_t *dest_seq,
    js_lib_command_sequence_t *src_seq)
{
    js_lib_command_t *src_cmd = src_seq->cmd_list;
    js_lib_command_t *src_end = src_cmd + src_seq->cmd_count;

    while (src_cmd < src_end) {
        js_lib_command_t *new_cmd = get_command(dest_seq);
        *new_cmd = *src_cmd;
        src_cmd++;
    }
    return 0;
}
