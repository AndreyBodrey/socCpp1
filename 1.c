#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <linux/if_ether.h> 
#include <net/ethernet.h>
#include <netinet/in.h>
#include <strings.h>
#include <arpa/inet.h>
#include <unistd.h>

int main ()
{
	printf("heloo nigga !\n\n");
	int soc = -1;
	struct sockaddr_in addrLocal;
	char *ipAdr = "192.168.0.112";   //hardcoded sheet, you must change to your IP

	bzero(&addrLocal, sizeof(addrLocal));
	addrLocal.sin_family = AF_PACKET;
	addrLocal.sin_port = 0;
	addrLocal.sin_addr.s_addr = inet_addr(ipAdr);


	soc = socket(  AF_PACKET, SOCK_RAW, htons(ETH_P_ALL) );   // works only whith root user

	if (soc > 0) printf ("socket crated\n");
	else 
	{
		printf("error to cerate socket!\n");
		return 1;
	}
	// now bind to local IP

	///bind(soc, &addrLocal, sizeof(addrLocal));
	if ( bind(soc, (struct sockaddr*)&addrLocal, sizeof(addrLocal)) != 0)
	{		
		printf ("bind() error!");
		close(soc);
		return 1;
	}

	printf("over\n");
	
	close(soc);

}
