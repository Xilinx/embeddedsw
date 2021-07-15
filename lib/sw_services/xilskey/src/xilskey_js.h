/******************************************************************************
* Copyright (c) 2013 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*
 * JTAG server interface
 *
 * This interface is intended to be a common abstraction of JTAG cable
 * implementations.
 *
 * Interface usage steps:
 *
 * 1. Initialize server implementation instance using implementation
 *    specific js_init_*() function.
 *
 * 2. Enumerate available ports using js_get_port_descr_list().
 *
 * 3. Open selected port using js_open_port().
 *
 * 4. Optional: get or set properties using js_get_property() or
 *    js_set_property().
 *
 * 5. Optional: enumerate scan chain using js_detect_taps() or
 *    manually.  Add nodes for each TAP controller (API TDB).
 *
 * 6. Build command sequence(s) object using
 *    js_create_command_sequence(), js_add_state_change(), and
 *    js_add_shift().
 *
 * 7. Run sequence using js_run_command_sequence().
 *
 * 8. Repeat #6 and #7 as needed, possibly deleting or clearing
 *    command sequence objects as needed using
 *    js_delete_command_sequence() and js_clear_command_sequence().
 *
 * 9. Close port using js_close_port().
 *
 * 10. Delete server using js_deinit_server().
 */

#ifndef XILSKEY_JS_H
#define XILSKEY_JS_H

#include <stdlib.h>

#if defined(_MSC_VER) && _MSC_VER < 1600
   typedef signed __int8 int8_t;
   typedef unsigned __int8 uint8_t;
   typedef signed __int16 int16_t;
   typedef unsigned __int16 uint16_t;
   typedef signed __int32 int32_t;
   typedef unsigned __int32 uint32_t;
   typedef signed __int64 int64_t;
   typedef unsigned __int64 uint64_t;
#else
#  include <stdint.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

/**
* IEEE 1149.1 defined dummy idcode value.
*/
#define JS_DUMMY_IDCODE 0x000000ff

typedef struct js_server_struct js_server_t;
typedef struct js_port_descr_struct js_port_descr_t;
typedef struct js_port_struct js_port_t;
typedef struct js_node_struct js_node_t;
typedef struct js_command_sequence_struct js_command_sequence_t;

/**
* JTAG property kinds
*/
typedef enum {
    JS_PROP_FREQUENCY /**< JS PROP frequency */
} js_property_kind_t;

typedef long js_property_value_t;

/**
 * JTAG port description
 *
 * This is retrieved by calling js_get_port_descr_list(), and is needed for
 * js_open_port() to indicate which port to use.
*/
struct js_port_descr_struct {
    char manufacturer[64]; /**< manufacturer information */
    char serial[64]; /**< Serial information */
    char port[64]; /**< Port information */
    char description[128]; /**< Description information */
    void *handle; /**< Pointer to handle*/
};

/**
 * JTAG port
 *
 * This is retrieved by calling js_open_port().
*/
struct js_port_struct {
	/**
	* Server associated with port
	*/
    js_server_t *server;
	/**
	* Node representing the whole scan chain
	*/
    js_node_t *root_node;
};


/**
 * JTAG node information
 *
 * Nodes typically represent a TAP controller.  A node can also
 * represent a collection of TAP controllers, i.e. the whole scan
 * chain or a device containing multiple TAP controllers.  For scan
 * chains containing JTAG multiplexers a node can represent the
 * branches on the multiplexer.
*/
struct js_node_struct {
    js_port_t *port;	/**< Associated port */

    int is_tap:1;	/**< Node represents one or more TAP controllerst */

    int is_mux:1;	/**< Node is a multiplexer (child nodes are branches) */

    int is_branch:1;	/**< Node is a branch on a multiplexer */

    int is_active:1;	/**< This branch is part of the scan chain */

    uint32_t idcode;	/**< IDCODE of the TAP
                          * (JS_DUMMY_IDCODE if unknown or NA) */

    int irlen;		/**< Instruction register length in bits */

    const char *name;	/**< Name of node */

    js_node_t **parent;	/**< Parent node */

    js_node_t **child_list;	/**< Child nodes */
    unsigned int child_count;	/**< Child nodes count */
};


/**
 * JTAG states
 *
 * To simply the interface only a subset of the JTAG states are make
 * accessible.
*/
/*typedef enum {
    JS_RESET,
    JS_IDLE,
    JS_SHIFT_DR,
    JS_PAUSE_DR,
    JS_SHIFT_IR,
    JS_PAUSE_IR,
    JS_STATE_MAX
} js_state_t;*/
typedef enum {
    JS_RESET, 		/**< JS Reset */
    JS_IDLE,		/**< JS Idle */
    JS_DRSELECT,	/**< JS DrSelect */
    JS_DRCAPTURE,	/**< JS DrCapture */
    JS_DRSHIFT,		/**< JS DrShift */
    JS_DREXIT1,		/**< JS DrExit1 */
    JS_DRPAUSE,		/**< JS DrPause */
    JS_DREXIT2,		/**< JS DrExit2 */
    JS_DRUPDATE,	/**< JS DrUpdate */
    JS_IRSELECT,	/**< JS IRSelect */
    JS_IRCAPTURE,	/**< JS IRCapture */
    JS_IRSHIFT,		/**< JS IRShift */
    JS_IREXIT1,		/**< JS IRExit1 */
    JS_IRPAUSE,		/**< JS IRPause */
    JS_IREXIT2,		/**< JS IRExit2 */
    JS_IRUPDATE,	/**< JS IRUpdate */
    JS_STATE_MAX	/**< JS State Max */
} js_state_t;



/**
 * JTAG command sequence object
 *
 * Commands are added to this object for later execution.
*/
struct js_command_sequence_struct {
    js_node_t *node; /**< JTAG node */
};


/***********************************************************************
 * JTAG server implementation specific functions
 ***********************************************************************/

/**
 * Get list of available JTAG ports supported by this server
*/
extern int js_get_port_descr_list(
    js_server_t *server,
    js_port_descr_t **port_listp);

/**
 * Open a specific JTAG port for this server instance
*/
extern int js_open_port(
    js_server_t *server,
    js_port_descr_t *port_descr,
    js_port_t **port);

/**
 * Delete server instance
*/
extern void js_deinit_server(
    js_server_t *server);

/**
 * Get property value
*/
extern int js_get_property(
    js_port_t *port,
    js_property_kind_t kind,
    js_property_value_t *valuep);

/**
 * Set property value
*/
extern int js_set_property(
    js_port_t *port,
    js_property_kind_t kind,
    js_property_value_t value);

/**
 * Execute pending commands
*/
extern int js_run_command_sequence(
    js_command_sequence_t *commands);

/**
 * Open a specific JTAG port for this server instance
*/
extern int js_close_port(
        js_port_t *port);


/***********************************************************************
 * JTAG server library functions
 ***********************************************************************/

/**
 * Retrieve information about the last error as a string.
 *
 * Returns NULL if no error occurred since last call to
 * set_last_error(server, NULL).
*/
const char *js_get_last_error(
    js_server_t *server);


/**
 * Set error information
*/
void js_set_last_error(
    js_server_t *server,
    const char *fmt,
    ...);


/**
 * Allocate command sequence object
*/
extern js_command_sequence_t *js_create_command_sequence(
    js_node_t *node);


/**
 * CLear command sequence
 *
 * This function allows a command sequence object to be reused for a
 * difference sequence.
*/
extern int js_clear_command_sequence(
    js_command_sequence_t *cmdseq);


/**
 * Delete command sequence
*/
extern int js_delete_command_sequence(
    js_command_sequence_t *cmdseq);


/**
 * Adds command to move JTAG state machine to <state> and cycle TCK
 * <clock> times.  The <clock> argument is only applicable for when
 * <state> is one of the stable states JS_RESET, JS_IDLE, JS_PAUSE_IR
 * or JS_PAUSE_DR.
*/
extern int js_add_state_change(
    js_command_sequence_t *cmdseq,
    js_state_t state,
    unsigned int clocks);

/**
 * Adds command to
 *  1. move state machine to either JS_SHIFT_IR if (<flags> &
 *     JS_TO_IR) != 0 or JS_SHIFT_DR if (<flags> & JS_TO_IR) == 0,
 *  2. shift <bits> number of bits in from tdi_buffer to TDI and out
 *     from TDO to tdo_buffer, and
 *  3. move state machine to <state>.
 *
 * If <tdi_buffer> is NULL then <bits> number of ones if (<flags> &
 *     JS_ONES) != 0 or zeroes if (<flags> & JS_ONES) == 0 are shifted
 *     to TDI.
 *
 * If <tdo_buffer> is NULL then TDO bits are ignored.
 *
 * Both state transitions are optional if the state machine is
 * already in the correct state.
*/
enum {
    JS_TO_IR = 1, /**< JS to IR */
    JS_ONES = 2	  /**< JS one's */
};

/**
* This function adds shift
*/
extern int js_add_shift(
    js_command_sequence_t *cmdseq,
    unsigned int flags,
    size_t bits,
    unsigned char **tdi_buffer,
    unsigned char **tdo_buffer,
    js_state_t state);


/**
 * Auto detect TAPs on scan chain.
 *
 * This typically involes a reset of the scan chain followed by
 * reading data registers.
 *
 * The result is allocated using malloc() and the caller is responsible
 * for freeing the buffer when it is no longer needed.
*/
extern int js_detect_taps(
    js_port_t *port,
    uint32_t **resultp);

#ifdef __cplusplus
}
#endif

#endif /* XILSKEY_JS_H */
