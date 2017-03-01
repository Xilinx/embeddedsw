#ifndef PLATFORM_INFO_H_
#define PLATFORM_INFO_H_

#include "openamp/hil.h"

/* Interrupt vectors */
#define IPI_IRQ_VECT_ID              65

#define RPMSG_CHAN_NAME              "rpmsg-openamp-demo-channel"

struct hil_proc *platform_create_proc(int proc_index);

#endif /* PLATFORM_INFO_H_ */
