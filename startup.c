



#include "startup.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

struct Status
{
    uint8_t igmpSubscibe;
    uint8_t levelDataSave; // heades included in to file or not
    char ipLocalStr[20];
    char ipIgmpGroupStr[20];
    char nameEthernetCard[20];

    int socketFd;
	uint8_t *packetData;

    struct sockaddr_in GroupAddr;
	struct sockaddr_in localAddr;

	uint8_t workMode;   // record some time or ressive some packets :default video
	uint32_t workModeParam;   // if workMode set as record video here time to rec in sec, if packet mode here count packets to ressive : default 10 min video

};





int prepareSettings(struct Status * state)
{
    state->igmpSubscibe = 0;
    memset( state->ipLocalStr, 0, sizeof(state->ipLocalStr));
    memset( state->ipIgmpGroupStr, 0, sizeof(state->ipIgmpGroupStr));
    memset( state->nameEthernetCard, 0, sizeof(state->nameEthernetCard));
    state->socketFd = -1;
    state->workMode = mode_video;
    state->workModeParam = 600; // default 10 min video

    state->localAddr.sin_family = AF_INET;
    state->localAddr.sin_addr.s_addr = serchIP(GROUP_IP);

    state->groupAddr.sin_family = AF_INET;
    if (inet_aton(GROUP_IP,&(state->groupAddr)));





}
//************************************************************************************************************************
