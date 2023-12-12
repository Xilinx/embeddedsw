/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * $FreeBSD$
 */

#ifndef _RPMSG_INTERNAL_H_
#define _RPMSG_INTERNAL_H_

#include <stdint.h>
#include <openamp/rpmsg.h>

#if defined __cplusplus
extern "C" {
#endif

#ifdef RPMSG_DEBUG
#include <metal/log.h>

#define RPMSG_ASSERT(_exp, _msg) do { \
		if (!(_exp)) { \
			metal_log(METAL_LOG_EMERGENCY, \
				  "FATAL: %s - "_msg, __func__); \
			metal_assert(_exp); \
		} \
	} while (0)
#else
#define RPMSG_ASSERT(_exp, _msg) metal_assert(_exp)
#endif

#define RPMSG_BUF_HELD (1U << 31) /* Flag to suggest to hold the buffer */

#define RPMSG_LOCATE_HDR(p) \
	((struct rpmsg_hdr *)((unsigned char *)(p) - sizeof(struct rpmsg_hdr)))
#define RPMSG_LOCATE_DATA(p) ((unsigned char *)(p) + sizeof(struct rpmsg_hdr))

/**
 * enum rpmsg_ns_flags - dynamic name service announcement flags
 *
 * @RPMSG_NS_CREATE: a new remote service was just created
 * @RPMSG_NS_DESTROY: a known remote service was just destroyed
 * @RPMSG_NS_CREATE_WITH_ACK: a new remote service was just created waiting
 *                            acknowledgment.
 */
enum rpmsg_ns_flags {
	RPMSG_NS_CREATE = 0,
	RPMSG_NS_DESTROY = 1,
};

/**
 * @brief Common header for all RPMsg messages
 *
 * Every message sent(/received) on the RPMsg bus begins with this header.
 */
METAL_PACKED_BEGIN
struct rpmsg_hdr {
	/** Source address */
	uint32_t src;

	/** Destination address */
	uint32_t dst;

	/** Reserved for future use */
	uint32_t reserved;

	/** Length of payload (in bytes) */
	uint16_t len;

	/** Message flags */
	uint16_t flags;
} METAL_PACKED_END;

/**
 * @brief Dynamic name service announcement message
 *
 * This message is sent across to publish a new service, or announce
 * about its removal. When we receive these messages, an appropriate
 * RPMsg channel (i.e device) is created/destroyed. In turn, the ->probe()
 * or ->remove() handler of the appropriate RPMsg driver will be invoked
 * (if/as-soon-as one is registered).
 */
METAL_PACKED_BEGIN
struct rpmsg_ns_msg {
	/** Name of the remote service that is being published */
	char name[RPMSG_NAME_SIZE];

	/** Endpoint address of the remote service that is being published */
	uint32_t addr;

	/** Indicates whether service is created or destroyed */
	uint32_t flags;
} METAL_PACKED_END;

int rpmsg_send_ns_message(struct rpmsg_endpoint *ept, unsigned long flags);

struct rpmsg_endpoint *rpmsg_get_endpoint(struct rpmsg_device *rvdev,
					  const char *name, uint32_t addr,
					  uint32_t dest_addr);
void rpmsg_register_endpoint(struct rpmsg_device *rdev,
			     struct rpmsg_endpoint *ept,
			     const char *name,
			     uint32_t src, uint32_t dest,
			     rpmsg_ept_cb cb,
			     rpmsg_ns_unbind_cb ns_unbind_cb);

static inline struct rpmsg_endpoint *
rpmsg_get_ept_from_addr(struct rpmsg_device *rdev, uint32_t addr)
{
	return rpmsg_get_endpoint(rdev, NULL, addr, RPMSG_ADDR_ANY);
}

#if defined __cplusplus
}
#endif

#endif /* _RPMSG_INTERNAL_H_ */
