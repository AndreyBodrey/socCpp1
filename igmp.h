#ifndef IGMP_H_INCLUDED
#define IGMP_H_INCLUDED

#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/ip.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <linux/igmp.h>
//#include <netinet/igmp.h>
#include <errno.h>

#include"main.h"

uint16_t ip_check_sum(uint16_t *buf, uint8_t len);
int igmpSend(uint8_t messageType, struct Status *);



#endif // IGMP_H_INCLUDED

