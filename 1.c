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
#include "main.h"

#include <linux/ip.h>
#include <netinet/ether.h>

#define PACK_BUF_LEN 0xffff

#define MC_GROUP_ADDRES "239.255.10.101"
#define MC_GROUP_PORT 2015

//global varibles
	int countLoop = 20000;
	char ipLockalStrFormat[20]; 
	in_addr_t ipLockal_int;
	in_addr_t ipIgmpGroup_int;
	struct in_addr ipIgmpGroup_inadr;

int main ()
{

	ipLockal_int = serchIP(ipLockalStrFormat);
	inet_pton(AF_INET, MC_GROUP_ADDRES, &ipIgmpGroup_inadr);
	ipIgmpGroup_int = ipIgmpGroup_inadr.s_addr;
	 // допустим айпи мы нашли и получили

	
	char buf[PACK_BUF_LEN] = {0,};	
	struct ehter_header * ehH = (struct ehter_header *) buf;	
	struct iphdr * ipH = (struct iphdr *)(buf + 14);
	int soc = -1;
	int socIgmp = -1;
	struct sockaddr_in addrLocal;

	

	printf("hello nigga !\n\n");


	serchIP(ipLockalStrFormat);



	socIgmp = igmpJoy();
	if ( socIgmp < 0 ) quit(soc, socIgmp,"igmp soc not created\n"); //error

	
	soc = socket(  PF_PACKET, SOCK_PACKET, htons(ETH_P_ALL) );   // works only whith root user
	
	if (soc > 0) printf ("socket crated\n");
	else  quit(soc, socIgmp,"sniff soc not created\n");

			//enp4s0 spirovo
	char netCardName[20] = {0,};
	findNetCardName(netCardName);
	int rc = setsockopt(soc,SOL_SOCKET, SO_BINDTODEVICE, netCardName, strlen(netCardName) + 1);
	if (rc != 0) quit(soc, socIgmp,"setsocopt not bind\n");
	printf ("setSock Opt %d, error %d \n" , rc, errno);



	/*	// раскоментировать это для  отлавливание всех пакетов даже не для нашего мака
	struct ifreq interface;
	interface.ifr_flags |= IFF_PROMISC;
	errno = 0;
	if (ioctl(soc,SIOCSIFFLAGS,&interface) < 0)
	{
		
		printf (" eerror in ioctl  error num = %d \n" , errno);
		close(soc);
	}
	*/


	while (countLoop--)
	{
		int reciveBytes = 0;
		reciveBytes = recvfrom(soc,buf,sizeof(buf),0,0,0);
		// 20 минимальный размер пакета
		if (reciveBytes > 20 && reciveBytes < PACK_BUF_LEN)	packetHandler(buf, reciveBytes);
	}


	printf("over\n");	
	close(soc);

}

//**************************************************************************************


void packetHandler(char *buf, int bufLen)
{
	//printf ("got packet, size = %d \n", bufLen);

	int typeProtocol = packFiltr(buf,bufLen);
	if (!typeProtocol) return clearBuf(buf);

	if (typeProtocol == IPPROTO_IGMP) checkIgmp(buf, bufLen);
	if (typeProtocol == IPPROTO_UDP)  checkUdp(buf,bufLen);


}
//-------------------------------------------------------------------------------------

void checkIgmp(char *buf, int bufLen)
{
	//  if qwery for us, then send addmembership
	printf("ressived IGMP pack \n");
}
//-------------------------------------------------------------------------------------

void checkUdp(char *buf, int bufLen)
{
	//   checking dup packets here
	printf("ressived UDP pack \n");
}
//-------------------------------------------------------------------------------------

int packFiltr(char * bufer, int len)
{
	time_t timeNow = time(NULL);
	struct tm * timeS = localtime(&timeNow);
	char packetInfo[200] = {0,};
	//char adr[20] = {0,};
    static int l = 0;
    struct ethhdr *ethernetHeader;
    struct iphdr *ipH ;
	struct igmp *igmpHdr;
	struct udphdr *udpHdr;
	struct in_addr sorceAdr;
	struct in_addr destAdr;
	// получаем адреса заголовков
    ethernetHeader = (struct ethhdr*)bufer;
    ipH = (struct iphdr*)(bufer + sizeof(struct ethhdr));
	igmpHdr = (struct igmp*)(bufer + sizeof(struct ethhdr) + ipH->ihl* 4);
	udpHdr = (struct udphdr*)(bufer + sizeof(struct ethhdr) + ipH->ihl* 4);
	sorceAdr.s_addr = ipH->saddr;
	destAdr.s_addr = ipH->daddr;
	switch(ipH->protocol)
	{
		case IPPROTO_UDP:
			//	отсеиваем локальные пакеты с 127.х.х.х
			if (*((uint8_t*) &sorceAdr.s_addr) == 127) break;
			//если пакет не от группы на которую подписались, то на хер
			//if (sorceAdr.s_addr != ipIgmpGroup_int) break;

			sprintf(packetInfo, "%02d:%02d:%02d %d.%02d.%02d ",timeS->tm_hour,timeS->tm_min,
								timeS->tm_sec,timeS->tm_year+1900,timeS->tm_mon+1,timeS->tm_mday);
			strcpy(packetInfo + strlen(packetInfo), "UDP Lenght ");
			sprintf((packetInfo + strlen(packetInfo)),"% d : ", htons(ipH->tot_len));
			strcpy (packetInfo + strlen(packetInfo), " from ");				
			strcpy (packetInfo + strlen(packetInfo), inet_ntoa(sorceAdr));
			strcpy (packetInfo + strlen(packetInfo), " to ");
			strcpy (packetInfo + strlen(packetInfo), inet_ntoa(destAdr));
			sprintf((packetInfo + strlen(packetInfo))," destPort %d ", htons(udpHdr->uh_dport));
			sprintf((packetInfo + strlen(packetInfo))," sorcePort %d ", htons(udpHdr->uh_sport));
			logging(packetInfo);
			char *udpData = bufer + sizeof(struct ethhdr) + ipH->ihl* 4 + sizeof(struct udphdr);
			int udpDataLen = udpHdr->len - sizeof(struct udphdr);
			//writeDataToFile( udpData,  udpDataLen);
			break;
		case IPPROTO_IGMP:
			logging("got IGMP pack");
			sprintf(packetInfo, "%02d:%02d:%02d %d.%02d.%02d ",timeS->tm_hour,timeS->tm_min,
								timeS->tm_sec,timeS->tm_year+1900,timeS->tm_mon+1,timeS->tm_mday);
			strcpy(packetInfo, "Lenght ");
			sprintf((packetInfo + strlen(packetInfo)),"%d : ", htons(ipH->tot_len));
			if (igmpHdr->igmp_type == IGMP_V2_MEMBERSHIP_REPORT)
			{				
				strcpy (packetInfo + strlen(packetInfo), "IGMP_MEMBERSHIP_REPORT GROUP ");								
			}
			if (igmpHdr->igmp_type == IGMP_MEMBERSHIP_QUERY)
			{
				strcpy (packetInfo + strlen(packetInfo), "IGMP_MEMBERSHIP_QUERY ");
			}
			if (igmpHdr->igmp_type == IGMP_V2_LEAVE_GROUP)
			{
				strcpy (packetInfo + strlen(packetInfo), "IGMP_LEAVE_GROUP ");
			}

			strcpy (packetInfo + strlen(packetInfo), inet_ntoa(igmpHdr->igmp_group));
			logging(packetInfo);
			break;
		case IPPROTO_TCP:
			//logging("got TCP pack");
			break;	
		case IPPROTO_ICMP:
			logging("got icmp pack");
			break;		
		default:
			logging("got some packet");
	}


    
   // if (ipH->protocol == IPPROTO_UDP || ipH->protocol == IPPROTO_IGMP) return ipH->protocol;
       
	return 0;
}
//-----------------------------------------------------------------------

void clearBuf(char * buff)
{
	int i = PACK_BUF_LEN;
	while(i--) 
	{
		*buff = 0;
		++buff;
	}
}
//----------------------------------------------------------------------

uint32_t serchIP(char * ipLoc)
{
	const int MAX_IFR = 20;
	
	// вот эот кусок получает список локальных айпи адесов и отсеивает не нужный 127.х.х.х. типа локадхост
	// работает не удалать!!!
	// надо тестить на конечнов устройстве
  
  struct ifreq ifr[MAX_IFR];
  struct ifconf ifconf;
  struct sockaddr_in *sin;

  int sock = socket(AF_INET, SOCK_DGRAM, 0);
  ifconf.ifc_len = sizeof(ifr);
  ifconf.ifc_req = ifr;
  ioctl(sock, SIOCGIFCONF, &ifconf);
  close(sock); 
  int n = ifconf.ifc_len / sizeof(struct ifreq);
  for(int i = 0; i < n; i++) 
  {
    sin = (struct sockaddr_in *) (&ifr[i].ifr_addr);

	// приведение к типу указатель на первый байт айпишника и проверка не локалхост ли он
	//тут надо написать проверку на принадлежнаость нужному диапазону айпи что выбрать привильный
	uint8_t *b = (uint8_t *) &(sin->sin_addr.s_addr);	
	if (*b == 127) continue;
    // printf("found network interface = %s -> %s\n",ifr[i].ifr_name,inet_ntoa(sin->sin_addr));

	char * temp = inet_ntoa(sin->sin_addr);
	//преписывем в строку айпи для возврата в виде строки
	memcpy(ipLoc, temp, strlen(temp)+1);
  }
	//возвращаем йапи в типе int
  return sin->sin_addr.s_addr;
	

}
//------------------------------------------------------------------------------------

int igmpJoy(void)
{
	int socI = socket( AF_INET, SOCK_DGRAM, 0 );
	if ( socI < 0 ) return 0; //error

	const int optval = 1;
	setsockopt(socI, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));
	struct sockaddr_in addr;
	bzero(&addr, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(MC_GROUP_PORT);
	addr.sin_addr.s_addr = htonl(INADDR_ANY);	
	//bind(socIgmp, (struct sockaddr*)&addr, sizeof(addr));
	struct ip_mreq mreq;
	inet_aton(MC_GROUP_ADDRES, &(mreq.imr_multiaddr));
	mreq.imr_interface.s_addr = htonl(INADDR_ANY);
		// эта хрень посылает MEMBERSHIP репорт 
	setsockopt(socI, IPPROTO_IP, IP_ADD_MEMBERSHIP, &mreq, sizeof(mreq));
	return socI;
 
}
//-------------------------------------------------------------------------------------

void findNetCardName(char * name)
{
	struct if_nameindex *ni;
    int i, selItem, choisenFlag = 0;
    ni = if_nameindex();

    if (ni == NULL) {
        perror("if_nameindex()");
        return;
    }

    for (i = 0; ni[i].if_index != 0 && ni[i].if_name != NULL; i++)
	{
		printf("%d: %s\n", ni[i].if_index, ni[i].if_name);
		if ( strcmp((ni[i].if_name), "lo"))  
			if (!choisenFlag)
			{
			selItem = i;
			choisenFlag = 1;
			}					//выбирает первую что не localhost 127.0.0.1	
	}
	strcpy(name, (ni[selItem].if_name));	// переносим имя сетевухи в выходнуй массив
}
//-------------------------------------------------------------------------------------

void quit(int soc1, int soc2, char * message)
{
	if (soc1 > 0) close(soc1);
	if (soc2 > 0) close(soc2);
	printf("%s", message);
	exit(0);
}
//-------------------------------------------------------------------------------------

void logging(const char* str)
{
	printf("%s",str);
	printf("\n");
}
//--------------------------------------------------------------------------------------

void writeDataToFile(char * data, int len)
{
	const int  MAX_FILE_SIZE = 100000;
	char fileName[20];
	static int numFile = 0;
	static int lenFile = 0;
	if (lenFile + len > MAX_FILE_SIZE) 
	{
		++numFile;
		lenFile = 0;
	}
	sprintf(fileName,"dataFile_%d", numFile);
	FILE *dataFile = fopen(fileName, "a");	// for writing at end of file)
	if (dataFile == NULL)
	{
		logging("file open error");
		return;
	}
	lenFile += len;
	fwrite(data, sizeof(char), len, dataFile);
	close(dataFile);

}