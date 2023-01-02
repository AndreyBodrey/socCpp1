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
	char buf[PACK_BUF_LEN] = {0,};

	struct ehter_header * ehH = (struct ehter_header *) buf;
	
	struct iphdr * ipH = (struct iphdr *)(buf + 14);


	int countLoop = 200;
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
	int rc = setsockopt(soc,SOL_SOCKET, SO_BINDTODEVICE,"enp7s0\x00", strlen("enp7s0\x00") + 1);
	printf ("%d \n" , rc);
	strlen(" ");

	/*						// раскоментировать это для  отлавливание всех пакетов даже не для нашего мака
	struct ifreq interface;
	interface.ifr_flags |= IFF_PROMISC;
	errno = 0;
	if (ioctl(soc,SIOCSIFFLAGS,&interface) < 0)
	{
		
		printf (" eerror in ioctl  error num = %d \n" , errno);
		close(soc);
	}
	*/
	
	int reciveBytes = 0;
	while (countLoop--)
	{
		reciveBytes = 0;
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


}
//-------------------------------------------------------------------------------------

void checkIgmp(char *buf, int bufLen)
{
	//  if qwery for us, then send addmembership
}

//-------------------------------------------------------------------------------------

struct ethhdr * packFiltr(char * bufer, int len)
{
    static int l = 0;
    struct ethhdr *ethernetHeader;
    struct iphdr *ipH ;
    ethernetHeader = (struct ethhdr *)bufer;
    ipH = (struct iphdr*)(bufer + sizeof(struct ethhdr));
    
    if (ipH->protocol == IPPROTO_UDP || ipH->protocol == IPPROTO_IGMP) return ipH;
       
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

