/*
 * Copyright (c) 2014, Mentor Graphics Corporation
 * All rights reserved.
 * Copyright (c) 2015 Xilinx, Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. Neither the name of Mentor Graphics Corporation nor the names of its
 *    contributors may be used to endorse or promote products derived from this
 *    software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include "openamp/remoteproc.h"
#include "openamp/remoteproc_loader.h"
#include "openamp/rsc_table_parser.h"
#include "openamp/env.h"
#include "openamp/hil.h"

/**
 * remoteproc_resource_init
 *
 * Initializes resources for remoteproc remote configuration. Only
 * remoteproc remote applications are allowed to call this function.
 *
 * @param rsc_info          - pointer to resource table info control
 * 							  block
 * @param channel_created   - callback function for channel creation
 * @param channel_destroyed - callback function for channel deletion
 * @param default_cb        - default callback for channel I/O
 * @param rproc_handle      - pointer to new remoteproc instance
 *
 * @param returns - status of function execution
 *
 */
int remoteproc_resource_init(struct rsc_table_info *rsc_info,
			     rpmsg_chnl_cb_t channel_created,
			     rpmsg_chnl_cb_t channel_destroyed,
			     rpmsg_rx_cb_t default_cb,
			     struct remote_proc **rproc_handle)
{

	struct remote_proc *rproc;
	int status;

	if (!rsc_info) {
		return RPROC_ERR_PARAM;
	}

	/* Initialize environment component */
	status = env_init();
	if (status != RPROC_SUCCESS) {
		return status;
	}

	rproc = env_allocate_memory(sizeof(struct remote_proc));
	if (rproc) {
		env_memset(rproc, 0x00, sizeof(struct remote_proc));
		/* There can be only one master for remote configuration so use the
		 * rsvd cpu id for creating hil proc */
		rproc->proc = hil_create_proc(HIL_RSVD_CPU_ID);
		if (rproc->proc) {
			/* Parse resource table */
			status =
			    handle_rsc_table(rproc, rsc_info->rsc_tab,
					     rsc_info->size);
			if (status == RPROC_SUCCESS) {
				/* Initialize RPMSG "messaging" component */
				*rproc_handle = rproc;
				status =
				    rpmsg_init(rproc->proc->cpu_id,
					       &rproc->rdev, channel_created,
					       channel_destroyed, default_cb,
					       RPMSG_MASTER);
			} else {
				status = RPROC_ERR_NO_RSC_TABLE;
			}
		} else {
			status = RPROC_ERR_CPU_ID;
		}
	} else {
		status = RPROC_ERR_NO_MEM;
	}

	/* Cleanup in case of error */
	if (status != RPROC_SUCCESS) {
		*rproc_handle = 0;
		(void)remoteproc_resource_deinit(rproc);
		return status;
	}
	return status;
}

/**
 * remoteproc_resource_deinit
 *
 * Uninitializes resources for remoteproc "remote" configuration.
 *
 * @param rproc - pointer to rproc instance
 *
 * @param returns - status of function execution
 *
 */

int remoteproc_resource_deinit(struct remote_proc *rproc)
{
	int i = 0;
	struct proc_vring *vring_hw = 0;
	if (rproc) {
		if (rproc->rdev) {
			/* disable IPC interrupts */
			if (rproc->proc->ops->reg_ipi_after_deinit) {
				for (i = 0; i < 2; i++) {
					vring_hw =
					    &rproc->proc->vdev.vring_info[i];
					rproc->proc->ops->
					    reg_ipi_after_deinit(vring_hw);
				}
			}
			rpmsg_deinit(rproc->rdev);
		}
		if (rproc->proc) {
			hil_delete_proc(rproc->proc);
		}

		env_free_memory(rproc);
	}

	env_deinit();

	/*
	 * Flush and Invalidate the caches - When the application is built with
	 * Xilinx Standalone BSP, caches are invalidated as part of boot process.
	 * Even if the master boots firmware multiple times without hard reset on
	 * same core, caches are flushed and invalidated at the end of
	 * remoteproc_resource_deinit for this run and caches would be again
	 * invalidated before starting the main thread of the application on next
	 * run to avoid any cache inconsistencies.
	 */
	 env_flush_invalidate_all_caches();


	return RPROC_SUCCESS;
}

/**
 * remoteproc_init
 *
 * Initializes resources for remoteproc master configuration. Only
 * remoteproc master applications are allowed to call this function.
 *
 * @param fw_name           - name of frimware
 * @param channel_created   - callback function for channel creation
 * @param channel_destroyed - callback function for channel deletion
 * @param default_cb        - default callback for channel I/O
 * @param rproc_handle      - pointer to new remoteproc instance
 *
 * @param returns - status of function execution
 *
 */
int remoteproc_init(char *fw_name, rpmsg_chnl_cb_t channel_created,
		    rpmsg_chnl_cb_t channel_destroyed, rpmsg_rx_cb_t default_cb,
		    struct remote_proc **rproc_handle)
{

	struct remote_proc *rproc;
	struct resource_table *rsc_table;
	unsigned int fw_addr, fw_size, rsc_size;
	int status, cpu_id;

	if (!fw_name) {
		return RPROC_ERR_PARAM;
	}

	/* Initialize environment component */
	status = env_init();
	if (status != RPROC_SUCCESS) {
		return status;
	}

	rproc = env_allocate_memory(sizeof(struct remote_proc));
	if (rproc) {
		env_memset((void *)rproc, 0x00, sizeof(struct remote_proc));
		/* Get CPU ID for the given firmware name */
		cpu_id = hil_get_cpuforfw(fw_name);
		if (cpu_id >= 0) {
			/* Create proc instance */
			rproc->proc = hil_create_proc(cpu_id);
			if (rproc->proc) {
				/* Retrieve firmware attributes */
				status =
				    hil_get_firmware(fw_name, &fw_addr,
						     &fw_size);
				if (!status) {
					/* Initialize ELF loader - currently only ELF format is supported */
					rproc->loader =
					    remoteproc_loader_init(ELF_LOADER);
					if (rproc->loader) {
						/* Attach the given firmware with the ELF parser/loader */
						status =
						    remoteproc_loader_attach_firmware
						    (rproc->loader,
						     (void *)fw_addr);
					} else {
						status = RPROC_ERR_LOADER;
					}
				}
			} else {
				status = RPROC_ERR_NO_MEM;
			}
		} else {
			status = RPROC_ERR_INVLD_FW;
		}
	} else {
		status = RPROC_ERR_NO_MEM;
	}

	if (!status) {
		rproc->role = RPROC_MASTER;

		/* Get resource table from firmware */
		rsc_table =
		    remoteproc_loader_retrieve_resource_section(rproc->loader,
								&rsc_size);
		if (rsc_table) {
			/* Parse resource table */
			status = handle_rsc_table(rproc, rsc_table, rsc_size);
		} else {
			status = RPROC_ERR_NO_RSC_TABLE;
		}
	}

	/* Cleanup in case of error */
	if (status != RPROC_SUCCESS) {
		(void)remoteproc_deinit(rproc);
		return status;
	}

	rproc->channel_created = channel_created;
	rproc->channel_destroyed = channel_destroyed;
	rproc->default_cb = default_cb;

	*rproc_handle = rproc;

	return status;
}

/**
 * remoteproc_deinit
 *
 * Uninitializes resources for remoteproc "master" configuration.
 *
 * @param rproc - pointer to remote proc instance
 *
 * @param returns - status of function execution
 *
 */
int remoteproc_deinit(struct remote_proc *rproc)
{

	if (rproc) {
		if (rproc->loader) {
			(void)remoteproc_loader_delete(rproc->loader);
			rproc->loader = RPROC_NULL;
		}
		if (rproc->proc) {
			hil_delete_proc(rproc->proc);
			rproc->proc = RPROC_NULL;
		}
		env_free_memory(rproc);
	}

	env_deinit();

	return RPROC_SUCCESS;
}

/**
 * remoteproc_boot
 *
 * This function loads the image on the remote processor and starts
 * its execution from image load address.
 *
 * @param rproc - pointer to remoteproc instance to boot
 *
 * @param returns - status of function execution
 */
int remoteproc_boot(struct remote_proc *rproc)
{

	void *load_addr;
	int status;

	if (!rproc) {
		return RPROC_ERR_PARAM;
	}

	/* Stop the remote CPU */
	hil_shutdown_cpu(rproc->proc);

	/* Load the firmware */
	status = remoteproc_loader_load_remote_firmware(rproc->loader);
	if (status == RPROC_SUCCESS) {
		load_addr = remoteproc_get_load_address(rproc->loader);
		if (load_addr != RPROC_ERR_PTR) {
			/* Start the remote cpu */
			status = hil_boot_cpu(rproc->proc,
					      (unsigned int)load_addr);
			if (status == RPROC_SUCCESS) {
				/* Wait for remote side to come up. This delay is arbitrary and may
				 * need adjustment for different configuration of remote systems */
				env_sleep_msec(RPROC_BOOT_DELAY);

				/* Initialize RPMSG "messaging" component */

				/* It is a work-around to work with remote Linux context.
				   Since the upstream Linux rpmsg implementation always
				   assumes itself to be an rpmsg master, we initialize
				   the remote device as an rpmsg master for remote Linux
				   configuration only. */
#if defined (OPENAMP_REMOTE_LINUX_ENABLE)
				status =
				    rpmsg_init(rproc->proc->cpu_id,
					       &rproc->rdev,
					       rproc->channel_created,
					       rproc->channel_destroyed,
					       rproc->default_cb, RPMSG_MASTER);
#else
				status =
				    rpmsg_init(rproc->proc->cpu_id,
					       &rproc->rdev,
					       rproc->channel_created,
					       rproc->channel_destroyed,
					       rproc->default_cb, RPMSG_REMOTE);
#endif
			}
		} else {
			status = RPROC_ERR_LOADER;
		}
	} else {
		status = RPROC_ERR_LOADER;
	}

	return status;
}

/**
 * remoteproc_shutdown
 *
 * This function shutdowns the remote execution context
 *
 * @param rproc - pointer to remote proc instance to shutdown
 *
 * @param returns - status of function execution
 */
int remoteproc_shutdown(struct remote_proc *rproc)
{

	if (rproc) {
		if (rproc->rdev) {
			rpmsg_deinit(rproc->rdev);
			rproc->rdev = RPROC_NULL;
		}
		if (rproc->proc) {
			hil_shutdown_cpu(rproc->proc);
		}
	}

	return RPROC_SUCCESS;
}
