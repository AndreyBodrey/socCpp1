
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <linux/if_ether.h>
#include <net/ethernet.h>
#include <netinet/in.h>
#include <strings.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h>
#include <netinet/igmp.h>
#include <netinet/udp.h>
#include <time.h>
//#include <linux/if.h>
#include <sys/ioctl.h>
#include <net/if.h>
//#include <linux/ip.h>
//#include <netinet/ether.h>




//#include  "main.h"

uint32_t serchIP(char * );

 enum WorkMode
{
    mode_video = 1,
    mode_udp,
    mode_ip,
    mode_ethernet

};



struct Status
{
    uint8_t igmpSubscibe;
    char ipLocalStr[20];
    char ipIgmpGroupStr[20];
    char nameEthernetCard[20];
    time_t startTime;

    int socketFd;
	char *packetData;

    struct sockaddr_in groupAddr;
	struct sockaddr_in localAddr;

	enum WorkMode workMode;   // record some time or ressive some packets :default mode_video
	uint32_t workModeParam;   // if workMode set as record video here time to rec in sec, if packet mode here count packets to ressive : default 10 min video

};


int prepareSettings(struct Status *);
uint32_t serchIP(char * );
void findNetCardName(char * name);

