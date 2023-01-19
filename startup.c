


#include "main.h"
#include "startup.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>


/*

struct Status
{
    uint8_t igmpSubscibe;
    char ipLocalStr[20];
    char ipIgmpGroupStr[20];
    char nameEthernetCard[20];

    int socketFd;
	uint8_t *packetData;

    struct sockaddr_in groupAddr;
	struct sockaddr_in localAddr;

	enum WorkMode workMode;   // record some time or ressive some packets :default mode_video
	uint32_t workModeParam;   // if workMode set as record video here time to rec in sec, if packet mode here count packets to ressive : default 10 min video

};


*/

 #define FILE_NAME "savedData"
 #define IGMP_FILE_NAME "igmpPackets"

int prepareSettings(struct Status * state)
{
    state->startTime = time(NULL);
    state->igmpSubscibe = 0;
    memset( state->ipLocalStr, 0, sizeof(state->ipLocalStr));
    memset( state->ipIgmpGroupStr, 0, sizeof(state->ipIgmpGroupStr));
    memset( state->nameEthernetCard, 0, sizeof(state->nameEthernetCard));
    memset( state->fileName,0, sizeof(state->fileName));
    memcpy(state->fileName, FILE_NAME,sizeof(FILE_NAME));
    memset( state->igmpFileName,0, sizeof(state->igmpFileName));
    memcpy(state->igmpFileName, IGMP_FILE_NAME,sizeof(IGMP_FILE_NAME));
    state->fileSize = 0;
    state->socketFd = -1;
    state->workMode = mode_video;
    state->workModeParam = 600; // default 10 min video

    state->localAddr.sin_family = AF_INET;
    state->localAddr.sin_addr.s_addr = serchIP(state->ipLocalStr);

    findNetCardName(state->nameEthernetCard);
    state->groupAddr.sin_family = AF_INET;
    if (inet_pton(AF_INET, GROUP_IP, (struct in_addr *)&(state->groupAddr.sin_addr.s_addr)) == 0)
    return 0;


    return 999;
}
//************************************************************************************************************************


uint32_t serchIP(char * ipLoc)  // поиск локального айпи компа в сети, пусть будет пригодится гденить)
{
	const int MAX_IFR = 20;

	// вот эот кусок получает список локальных айпи адесов и отсеивает не нужный 127.х.х.х. типа локалхост
	// работает не удалать!!!
	// надо тестить на конечнов устройстве

								//это нашел в сети не много доработал для фильтрации 127,х,х,х
  struct ifreq ifr[MAX_IFR];
  struct ifconf ifconf;
  struct sockaddr_in *sin;

  int sock = socket(AF_INET, SOCK_DGRAM, 0);
  ifconf.ifc_len = sizeof(ifr);
  ifconf.ifc_req = ifr;
  ioctl(sock, SIOCGIFCONF, &ifconf); // заполняет структуру ifconf чтоб из нее получить что надо
  close(sock);
  int n = ifconf.ifc_len / sizeof(struct ifreq);

  for(int i = 0; i < n; i++)
  {
    sin = (struct sockaddr_in *) (&ifr[i].ifr_addr);

	// приведение к типу указатель на первый байт айпишника и проверка не локалхост ли он
	//тут надо написать проверку на принадлежнаость нужному диапазону айпи что выбрать привильный
	uint8_t *b = (uint8_t *) &(sin->sin_addr.s_addr);
	if (*b == 127) continue;
    // printf("found network interface = %s -> %s\n",ifr[i].ifr_name,inet_ntoa(sin->sin_addr)); // отладочное

	char * temp = inet_ntoa(sin->sin_addr);
	//преписывем в строку айпи для возврата в виде строки
	memcpy(ipLoc, temp, strlen(temp)+1);
  }
	//возвращаем йапи в типе int
  return sin->sin_addr.s_addr;

}
//-------------------------------------------------------------------------------------

void findNetCardName(char * name) // ищем имя сетевухи хватаем первую что не "lo" локалхост 127.х.х.х
									// этот код то же с просторов, но с небольшой доработкой выбора
{
	struct if_nameindex *ni;
    int i, selItem, choisenFlag = 0;
    ni = if_nameindex();
    if (ni == NULL)
    {
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
//-----------------------------------------------------------------------------------------------------------------



