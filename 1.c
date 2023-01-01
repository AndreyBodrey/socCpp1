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
int main ()
{
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
	// now bind to local IP

	///bind(soc, &addrLocal, sizeof(addrLocal));
	
	/*if ( bind(soc, (struct sockaddr*)&addrLocal, sizeof(addrLocal)) != 0)
	{		
		printf ("bind() error!  ");
		printf("%d\n", errno);
		close(soc);
		return 1;
	}*/
				///   enp7s0 name ehernet inteface
	
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
	char buf[0xffff] = {0,};
	int rc = 0;
	while (countLoop--)
	{
		rc = 0;
		rc = recvfrom(soc,buf,sizeof(buf),0,0,0);
		if (rc) packetHandler(buf, rc);


		
	}








	printf("over\n");	
	close(soc);

}

//**************************************************************************************


void packetHandler(char *buf, int bufLen)
{
	printf ("got packet, size = %d", bufLen);
}