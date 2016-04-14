#ifndef _EVENT_CONTROL_H
#define _EVENT_CONTROL_H

#define DEBUG
#define HOST_PORT 5000
#define TARGET_PORT 5005
#define LOOPBACK_IP "127.0.0.1"
#define DEFAULT_EVENT_DEV "/dev/input/event5"

#ifdef DEBUG
#define DBG(format, args...) printf(format, ## args)
#else
#define DBG(format, args...)
#endif
#define ERR(format, args...) printf(format, ## args)

enum device_job {
	DEVICE_JOB_RX = 0,
	DEVICE_JOB_TX,
};

struct device {
	int job;
	int host_port;
	int target_port;
	int ev_desc;
	char end_ip[16];
	char ev_dev[20];
};

#endif
