#include <stdio.h>
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
//#include <linux/if.h>
#include <sys/ioctl.h>
#include <net/if.h>  
#include "main.h"

#include <linux/ip.h>
#include <netinet/ether.h>

#define PACK_BUF_LEN 0xffff


int main ()
{
	char ipLockalStrFormat[20];
	uint32_t ipLockalInetFormat = serchIP(ipLockalStrFormat);
	 // допустим айпи мы нашли и получили

	
	char buf[PACK_BUF_LEN] = {0,};
	
	struct ehter_header * ehH = (struct ehter_header *) buf;
	
	struct iphdr * ipH = (struct iphdr *)(buf + 14);


	int countLoop = 20000;
	printf("heloo nigga !\n\n");
	int soc = -1;
	struct sockaddr_in addrLocal;
	char *ipAdr = "192.168.0.112";   //hardcoded sheet, you must change to your IP

	bzero(&addrLocal, sizeof(addrLocal));
	addrLocal.sin_family = AF_PACKET;
	addrLocal.sin_port = htons(0);
	addrLocal.sin_addr.s_addr = inet_addr(ipAdr);


	soc = socket(  PF_PACKET, SOCK_PACKET, htons(ETH_P_ALL) );   // works only whith root user

	if (soc > 0) printf ("socket crated\n");
	else 
	{
		printf("error to cerate socket!\n");
		return 1;
	}

			//enp4s0 spirovo
	char netCardName[20] = {0,};
	findNetCardName(netCardName);
	int rc = setsockopt(soc,SOL_SOCKET, SO_BINDTODEVICE, netCardName, strlen(netCardName) + 1);

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
//	printf("ressived UDP pack \n");
}
//-------------------------------------------------------------------------------------

int packFiltr(char * bufer, int len)
{
    static int l = 0;
    struct ethhdr *ethernetHeader;
    struct iphdr *ipH ;
    ethernetHeader = (struct ethhdr *)bufer;
    ipH = (struct iphdr*)(bufer + sizeof(struct ethhdr));
    
    if (ipH->protocol == IPPROTO_UDP || ipH->protocol == IPPROTO_IGMP) return ipH->protocol;
       
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
	

//var2 not tested
/* 
#include   "unp.h"
 #include    <sys/utsname.h>
 char **  my_addrs(int *addrtype)
 {
    struct hostent *hptr;
    struct utsname myname;
    if (uname(&myname) < 0)    return (NULL);
 if ( (hptr = gethostbyname(myname.nodename)) == NULL)
 return (NULL);
  *addrtype = hptr->h_addrtype;
   return (hptr->h_addr_list);
 }
 */

}

//------------------------------------------------------------------------------------

void igmpJoy()
{
	//sending igmp membership report
	
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