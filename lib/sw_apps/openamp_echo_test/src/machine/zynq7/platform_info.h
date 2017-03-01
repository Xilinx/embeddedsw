#ifndef PLATFORM_INFO_H_
#define PLATFORM_INFO_H_

#include "openamp/hil.h"

/* Interrupt vectors */
#define VRING0_IPI_INTR_VECT              15
#define VRING1_IPI_INTR_VECT              14

#define RPMSG_CHAN_NAME         "rpmsg-openamp-demo-channel"

struct hil_proc *platform_create_proc(int proc_index);

#endif /* PLATFORM_INFO_H_ */
