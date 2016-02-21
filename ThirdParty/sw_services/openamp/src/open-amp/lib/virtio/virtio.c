/*-
 * Copyright (c) 2011, Bryan Venteicher <bryanv@FreeBSD.org>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice unmodified, this list of conditions, and the following
 *    disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
#include "openamp/virtio.h"

static const char *virtio_feature_name(unsigned long feature,
				       struct virtio_feature_desc *);

//TODO : This structure may change depending on the types of devices we support.
static struct virtio_ident {
	unsigned short devid;
	const char *name;
} virtio_ident_table[] = {
	{
	VIRTIO_ID_NETWORK, "Network"}, {
	VIRTIO_ID_BLOCK, "Block"}, {
	VIRTIO_ID_CONSOLE, "Console"}, {
	VIRTIO_ID_ENTROPY, "Entropy"}, {
	VIRTIO_ID_BALLOON, "Balloon"}, {
	VIRTIO_ID_IOMEMORY, "IOMemory"}, {
	VIRTIO_ID_SCSI, "SCSI"}, {
	VIRTIO_ID_9P, "9P Transport"}, {
	0, NULL}
};

/* Device independent features. */
static struct virtio_feature_desc virtio_common_feature_desc[] = {
	{VIRTIO_F_NOTIFY_ON_EMPTY, "NotifyOnEmpty"},
	{VIRTIO_RING_F_INDIRECT_DESC, "RingIndirect"},
	{VIRTIO_RING_F_EVENT_IDX, "EventIdx"},
	{VIRTIO_F_BAD_FEATURE, "BadFeature"},

	{0, NULL}
};

const char *virtio_dev_name(unsigned short devid)
{
	struct virtio_ident *ident;

	for (ident = virtio_ident_table; ident->name != NULL; ident++) {
		if (ident->devid == devid)
			return (ident->name);
	}

	return (NULL);
}

static const char *virtio_feature_name(unsigned long val,
				       struct virtio_feature_desc *desc)
{
	int i, j;
	struct virtio_feature_desc *descs[2] = { desc,
		virtio_common_feature_desc
	};

	for (i = 0; i < 2; i++) {
		if (descs[i] == NULL)
			continue;

		for (j = 0; descs[i][j].vfd_val != 0; j++) {
			if (val == descs[i][j].vfd_val)
				return (descs[i][j].vfd_str);
		}
	}

	return (NULL);
}

void virtio_describe(struct virtio_device *dev, const char *msg,
		     uint32_t features, struct virtio_feature_desc *desc)
{
	(void)dev;
	(void)msg;
	(void)features;

	// TODO: Not used currently - keeping it for future use
	virtio_feature_name(0, desc);
}
