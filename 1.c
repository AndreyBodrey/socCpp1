#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <linux/if_ether.h> 
#include <net/ethernet.h>
#include <netinet/in.h>

int main ()
{
	printf("heloo nigga !\n\n");
	int soc = -1;

	soc = socket(  AF_PACKET, SOCK_RAW, htons(ETH_P_ALL) );   // works only whith root user

	if (soc > 0) printf ("socket crated\n");
	else 
	{
		printf("error to cerate socket!\n");
		return 1;
	}
	
	

	printf("over\n");
	close(soc);

}
