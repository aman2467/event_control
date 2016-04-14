/*
 * @file    : socket.c
 *
 * @description : This file contains program for socket communication.
 *
 * @author  : Aman Kumar (2016)
 *
 * @copyright   : The code contained herein is licensed under the GNU General
 *		Public License. You may obtain a copy of the GNU General
 *		Public License Version 2 or later at the following locations:
 *              http://www.opensource.org/licenses/gpl-license.html
 *              http://www.gnu.org/copyleft/gpl.html
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>

static int fd_udp;
static struct sockaddr_in s_addr_udp, d_addr_udp;
static socklen_t d_len_udp;

static int valid_digit(char *ip_dgt)
{
	while (*ip_dgt) {
		if (*ip_dgt >= '0' && *ip_dgt <= '9')
			++ip_dgt;
		else
			return 0;
	}
	return 1;
}

static int ip_is_valid(char *ip_str)
{
	int num, dots = 0;
	char *ptr;

	if (ip_str == NULL)
		return 0;

	ptr = strtok(ip_str, ".");
	if (ptr == NULL)
		return 0;

	while (ptr != NULL) {
		if (!valid_digit(ptr))
			return 0;
		num = atoi(ptr);
		if (num >= 0 && num <= 255) {
			ptr = strtok(NULL, ".");
			if (ptr != NULL)
				++dots;
		} else
			return 0;
	}

	if (dots != 3)
		return 0;
	return 1;
}

int init_udp(char *d_ip, int s_port, int d_port)
{
	char ip[16] = {0};

	strcpy(ip, d_ip);
	if (!ip_is_valid(ip)) {
		printf("Invalid IP : %s\n", d_ip);
		return -1;
	}

	fd_udp = socket(AF_INET, SOCK_DGRAM, 0);
	if (fd_udp < 0) {
		printf("ERROR : SOCKET OPEN\n");
		return -1;
	}

	s_addr_udp.sin_family = AF_INET;
	s_addr_udp.sin_port = htons(s_port);
	s_addr_udp.sin_addr.s_addr = inet_addr("0.0.0.0");

	if (bind(fd_udp, (const struct sockaddr *)&s_addr_udp,
		sizeof(struct sockaddr_in)) < 0) {
		printf("ERROR : SOCKET BIND\n");
		return -1;
	}

	d_addr_udp.sin_family = AF_INET;
	d_addr_udp.sin_port = htons(d_port);
	d_addr_udp.sin_addr.s_addr = inet_addr(d_ip);
	d_len_udp = sizeof(struct sockaddr_in);

	return 0;
}

int transmit_udp_data(void *data, ssize_t len)
{
	return sendto(fd_udp, data, len, 0,
		      (const struct sockaddr *)&d_addr_udp, d_len_udp);
}

int receive_udp_data(void *data, ssize_t len)
{
	return recvfrom(fd_udp, data, len, 0,
			(struct sockaddr *)&d_addr_udp, &d_len_udp);
}

void term_udp(void)
{
	close(fd_udp);
}
