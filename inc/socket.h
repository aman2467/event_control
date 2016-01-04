#ifndef _SOCKET_H
#define _SOCKET_H
#include <stdio.h>

int init_udp(char *d_ip, int s_port, int d_port);
int transmit_udp_data(void *data, ssize_t len);
int receive_udp_data(void *data, ssize_t len);
void term_udp(void);

#endif
