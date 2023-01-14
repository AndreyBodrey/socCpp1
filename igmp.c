
/*#include "main.h"



uint16_t ip_check_sum(uint16_t *buf, uint8_t len)
{
    uint32_t sum = 0;
    while (len>1)
    {
        sum += *buf;
        buf ++;
        len -= 2;
    }
    if (len)
        sum += (uint16_t)((uint8_t)*buf << 8);
    while (sum >> 16)
        sum = (sum & 0xffff) + (sum >> 16);
    return ~((uint16_t)sum);
}
//-------------------------------------------------------------------------


int igmpSeng(uint8_t messageType)
{
/*	int igmpSocket;
	struct sockaddr_in daddr;
	struct sockaddr_in laddr;

	char packet[32] = {0,};
	struct iphdr *ip = (struct iphdr *)packet;

	daddr.sin_family = AF_INET;
-	inet_pton(AF_INET, GROUP_IP, (struct in_addr *)&daddr.sin_addr.s_addr);

	laddr.sin_family = AF_INET;
	inet_pton(AF_INET, SORS, (struct in_addr *)&laddr.sin_addr.s_addr);

    errno = 0;
	if ((igmpSocket = socket(AF_INET, SOCK_RAW, IPPROTO_RAW)) < 0)
    {
		printf("igmpSend error: socket not created. errno = %d \n", errno);
		return -1;
	}

	ip->ihl = 6;
	ip->version = 4;
	ip->tos = 0;
	ip->tot_len = htons(32);
	ip->frag_off = 0;
	ip->ttl = 1;
	ip->protocol = IPPROTO_IGMP;
	ip->check = 0;
	ip->saddr = laddr.sin_addr.s_addr;
	ip->daddr = daddr.sin_addr.s_addr;
    packet[20] = 0x94; packet[21] = 04;

	struct igmphdr *imgpHrd = (struct igmphdr *) (packet + ip->ihl *4);

    imgpHrd->type = messageType;
    imgpHrd->code = 100;//max resp time 10 sec
    imgpHrd->csum = 0;
    imgpHrd->group = daddr.sin_addr.s_addr;
    imgpHrd->csum = ip_check_sum((uint16_t *)imgpHrd, sizeof(struct igmphdr));

    errno = 0;
    int sendedBytes = sendto(igmpSocket, (char *)packet, sizeof(packet), 0,(struct sockaddr *)&daddr, (socklen_t)sizeof(daddr));

	if (sendedBytes <= 0) printf("igmpSend error: sendto(). errno = %d \n", errno);

	return sendedBytes;
}*/
//----------------------------------------------------------------------------------------------------------------------------------

