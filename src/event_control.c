/* ==========================================================================
 * @file    : event_control.c
 *
 * @description : This file contains program to control input events remotely.
 *
 * @author  : Aman Kumar (2016)
 *
 * @copyright   : The code contained herein is licensed under the GNU General
 *		Public License. You may obtain a copy of the GNU General
 *		Public License Version 2 or later at the following locations:
 *              http://www.opensource.org/licenses/gpl-license.html
 *              http://www.gnu.org/copyleft/gpl.html
 * ========================================================================*/

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>
#include <linux/input.h>
#include <fcntl.h>
#include <sys/time.h>
#include <event_control.h>
#include <socket.h>

static void usage(char *prog)
{
	printf("\n"
	       " Usage : %s <OPTIONS>\t\t[root permission may require]\n"
	       "\n"
	       "     OPTIONS :\n"
	       "        -d | --device      : Event device file\n"
	       "                   Default : /dev/input/event5\n"
	       "\n"
	       "        -i | --ipaddr      : IP address of end terminal\n"
	       "                   Default : 127.0.0.1(loopback)\n"
	       "\n"
	       "        -j | --job         : Job to perform by application\n"
	       "                         0 : DEVICE_JOB_RX\n"
	       "                         1 : DEVICE_JOB_TX\n"
	       "                   Default : DEVICE_JOB_TX\n"
	       "\n"
	       "        -s | --source-port : UDP source port\n"
	       "                   Default : 5000\n"
	       "\n"
	       "        -t | --target-port : UDP destination port\n"
	       "                   Default : 5005\n"
	       "\n"
	       "        -h | --help        : Prints this and exits\n"
	       "\n"
	       "     Example :\n"
	       "         %s -j 0 -s 5005 -t 5000\n"
	       "         %s -s 5005 -t 5000 -i 192.168.24.67\n"
	       "         %s -s 5005 -t 5000 -d /dev/input/event4\n"
	       "         %s --help\n"
	       "\n",
		prog, prog, prog, prog, prog);
}

static int init_device(int argc, char **argv,
			struct device *dev)
{
	int opt;
	int long_index;

	/* initilize device with default params */
	dev->job = DEVICE_JOB_TX;
	dev->host_port = HOST_PORT;
	dev->target_port = TARGET_PORT;
	dev->ev_desc = -1;
	memcpy(dev->end_ip, LOOPBACK_IP, 16);
	memcpy(dev->ev_dev, DEFAULT_EVENT_DEV, 20);

	/* Available Command line options */
	static struct option longopts[] = {
		{"device", required_argument, NULL, 'd'},
		{"help", no_argument, NULL, 'h'},
		{"ipaddr", required_argument, NULL, 'i'},
		{"job", required_argument, NULL, 'j'},
		{"source-port", required_argument, NULL, 's'},
		{"target-port", required_argument, NULL, 't'},
		{NULL, 0, NULL, 0}
	};

	/* loop till all input arguments dispatched(if any) */
	for(ever) {
		opt = getopt_long(argc, argv, "+d:h::i:j:s:t:",
				  longopts, &long_index);
		if(-1 == opt)
			break;

		switch(opt) {
			case 'd':
				memcpy(dev->ev_dev, optarg, 20);
				break;
			case 'i':
				memcpy(dev->end_ip, optarg, 16);
				break;
			case 'j':
				dev->job = atoi(optarg);
				break;
			case 's':
				dev->host_port = atoi(optarg);
				break;
			case 't':
				dev->target_port = atoi(optarg);
				break;
			case 'h':
			default:
				usage(argv[0]);
				exit(0);
		}
	}
	optind = 1;

	return open(dev->ev_dev, O_RDWR);
}

static void dispatch_data(struct input_event *event,
			  int fd, int obj)
{
	int i;

	for(i = 0; i < obj; i++) {
		struct input_event *ev;
		int valid = 1;
		int obj_len = sizeof(struct input_event);

		ev  = event + i*obj_len;
		switch(ev->type) {
			case EV_KEY:
				if(ev->value != 0
				   && ev->value != 1)
					valid = 0;
				break;
			case EV_REL:
				if((ev->code != REL_X)
				   && (ev->code != REL_Y)
				   && (ev->code != REL_WHEEL))
					valid = 0;
				break;
			case EV_SYN:
				if(ev->value != 0)
					valid = 0;
				break;
			default:
				valid = 0;
				break;
		}
		if(valid) {
			if(obj_len != write(fd, ev, obj_len)) {
				DBG("ERROR : WRITING DATA\n");
			}
		}
	}
}

static void device_job_tx_loop(struct device *dev)
{
	ssize_t len;
	struct input_event event;

	for(ever) {
		memset(&event, 0, sizeof(event));
		len = read(dev->ev_desc, &event, sizeof(event));
		if(len != sizeof(event)) {
			DBG("ERROR : READING DATA\n");
			continue;
		}
		if(event.type < EV_MSC) {
			len = transmit_udp_data(&event, len);
			if(len < 0) {
				DBG("ERROR : TRANSMITTING DATA\n");
				continue;
			}
		}
	}
}

static void device_job_rx_loop(struct device *dev)
{
	int obj = 0;
	ssize_t len = 0;
	struct input_event event[10];

	for(ever) {
		memset(event, 0, sizeof(event));
		len = receive_udp_data(event, sizeof(event));
		if(len < 0) {
			DBG("ERROR : RECEIVING DATA\n");
			continue;
		} else {
			obj = len/sizeof(struct input_event);
			dispatch_data(event, dev->ev_desc, obj);
		}
	}
}

/**
 * Main routine of the application
 */
int main(int argc, char **argv)
{
	struct device *dev = calloc(1, sizeof(struct device));

	if(system("clear") < 0) {
		ERR("ERROR : COULD NOT CLEAR THE SCREEN\n");;
	}
	if(NULL != dev) {
		dev->ev_desc = init_device(argc, argv, dev);
		if(0 > dev->ev_desc) {
			ERR("ERROR : INIT DEVICE\n");
			return -1;
		}
	} else {
		ERR("ERROR : DEVICE ALLOCATION\n");
		return -1;
	}

	if(init_udp(dev->end_ip,
		    dev->host_port,
		    dev->target_port) < 0) {
		ERR("ERROR : INIT UDP SOCKET\n");
		return -1;
	}

	DBG("\n#*=*=*=*=*=*> DEVICE INFO <*=*=*=*=*=*#\n");
	DBG(" dev->job         = %s\n", dev->job?"TX":"RX");
	DBG(" dev->host_port   = %d\n", dev->host_port);
	DBG(" dev->target_port = %d\n", dev->target_port);
	DBG(" dev->end_ip      = %s\n", dev->end_ip);
	DBG(" dev->ev_dev      = %s\n", dev->ev_dev);
	DBG("#*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*#\n");
	switch(dev->job) {
		case DEVICE_JOB_TX:
			device_job_tx_loop(dev);
			break;
		case DEVICE_JOB_RX:
			device_job_rx_loop(dev);
			break;
		default:
			ERR("ERROR : UNKNOWN JOB\n");
			break;
	}
	close(dev->ev_desc);
	term_udp();
	free(dev);

	return 0;
}
