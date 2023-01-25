


#include "main.h"
#include "startup.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <net/if.h>


int prepareSettings(struct Status * state)
{

    state->startTime = time(NULL);
    state->igmpSubscibe = 0;
    memset( state->ipLocalStr, 0, sizeof(state->ipLocalStr));
    memset( state->ipIgmpGroupStr, 0, sizeof(state->ipIgmpGroupStr));
    memset( state->nameEthernetCard, 0, sizeof(state->nameEthernetCard));
    strcpy( state->ipIgmpGroupStr, GROUP_IP);

    memset( state->igmpFileName,0, sizeof(state->igmpFileName));
    memcpy(state->igmpFileName, IGMP_FILE_NAME,sizeof(IGMP_FILE_NAME));

    state->fileSize = 0;
    state->socketFd = -1;
    state->workMode = mode_video;
    state->workModeParam = 600; // default 10 min video

    state->localAddr.sin_family = AF_INET;
    state->localAddr.sin_addr.s_addr = serchIP(state->ipLocalStr);

    state->igmpGroupPort = htons(MC_GROUP_PORT);

    findNetCardName(state->nameEthernetCard);
    state->groupAddr.sin_family = AF_INET;
    if (inet_pton(AF_INET, GROUP_IP, (struct in_addr *)&(state->groupAddr.sin_addr.s_addr)) == 0) return 0;

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
  return (sin->sin_addr.s_addr);

}
//-------------------------------------------------------------------------------------

void findNetCardName(char * name) // ищем имя сетевухи хватаем первую что не "lo" локалхост 127.х.х.х
									// этот код то же с просторов, но с небольшой доработкой выбора
{
	struct if_nameindex *ni;
    int i, selItem = 0, choisenFlag = 0;
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
//---------------------------------------------------------------------------------------------------------------------

void setFileName(struct Status * state)
{
    char name[50] = {0,};
    memcpy(name, state->fileName, strlen(state->fileName));
    if (state->workMode == mode_ethernet)
    {
        char pcapFileName[strlen(name) + sizeof(PCAP_EXTENSION) + 1];
        memset(pcapFileName, 0, sizeof(pcapFileName));
        strcpy(pcapFileName, name);
        strcpy(pcapFileName + strlen(name), PCAP_EXTENSION);
        strcpy(state->fileName, pcapFileName);
    }
    else
    {
        char vidFileName[strlen(name) + sizeof(VID_EXTENSION) + 1];
        memset(vidFileName, 0, sizeof(vidFileName));
        strcpy(vidFileName, name);
        strcpy(vidFileName + strlen(name), VID_EXTENSION);
        strcpy(state->fileName, vidFileName);
    }

}


void changeSettingsInterface(struct Status * state)
{
                                // mode
pos1:
    int temp = 0;
    char tempStr[50];

   // goto pos5;

    system("clear");
    printf("\33[37;1;42m manual settings to set as default. \33[0m");
    printf("\n\n\33[1m step 1.\n\t Enter number for save data as...\n\33[0m");
    printf("\t \'1\' as video \n");
    printf("\t \'2\' as pcap format \n");
    int input = getchar();
    if (input == '1')
    {
        state->workMode = mode_video;
        printf(" you choose save video \n");
    }
    else if (input == '2')
    {
        state->workMode = mode_ethernet;
        printf(" you choose  pcap \n");
    }
    else goto pos1;
p1:
    printf(" enter to continue, \'1\' for repeat \n");
    while (temp = getchar() != 10);
    temp = getchar();
    if (temp == '1') goto pos1;
    if (temp != 13 && temp != 10) goto p1;


pos2:                                           // mode param
    system("clear");

    printf("\33[37;1;42m manual settings to set as default. \33[0m");
    if (state->workMode == mode_video)
    {
         printf("\n\n\33[1m step 2.\n\t Enter the count of seconds to save video\n\33[0m");
         printf("\33[1m \t now default is %u\n\33[0m", state->workModeParam);
    }
    else
    {
        printf("\n\n\33[1m step 2.\n\t Enter the count of packets to save in pcap\n\33[0m");
        printf("\33[1m \t now default is %u\n\33[0m", state->workModeParam);
    }

    char str[50] = {0,};
    if (!scanf("%s", str)) goto pos2;
    temp = atoi(str);
    if (!temp) goto pos2;
    state->workModeParam = temp;
    printf("you enter %i  \n", state->workModeParam);

p2:
    printf(" enter to continue, \'1\' for over and exit setups \n");
    while (temp = getchar() != 10);
    temp = getchar();
    if (temp == '1') return;
    if (temp != 13 && temp != 10) goto p2;


pos3:                                       //igmp file name
    system("clear");
    printf("\33[37;1;42m manual settings to set as default. \33[0m");
    printf("\n\n\33[1m step 3.\n\t Enter file name for imgp packs \n\33[0m");
    printf("\tnow name is %s \n", state->igmpFileName);
    scanf("%[^\n]", state->igmpFileName);
    printf("\n you enter %s\n", state->igmpFileName);
p3:
    printf("\n enter to continue, \'1\' for repeat \n");
    while (temp = getchar() != 10);
    temp = getchar();
    if (temp == '1') goto pos3;
    if (temp != 13 && temp != 10) goto p3;


pos4:                                       //data file name
    system("clear");
    printf("\33[37;1;42m manual settings to set as default. \33[0m");
    printf("\n\n\33[1m step 4.\n\t Enter file name for data video & packs without extention \n\33[0m");
    printf("\tnow name is %s \n", state->fileName);
    scanf("%[^\n]", state->fileName);
    printf("\n you enter %s\n", state->fileName);
p4:
    printf("\n enter to continue, \'1\' for repeat \n");
    while (temp = getchar() != 10);
    temp = getchar();
    if (temp == '1') goto pos4;
    if (temp != 13 && temp != 10) goto p4;

pos5:                                       // IP address IGMP
    memset(tempStr, 0, sizeof(tempStr));
    struct sockaddr_in tempSadr = state->groupAddr;
    system("clear");
    printf("\33[37;1;42m manual settings to set as default. \33[0m");
    printf("\n\n\33[1m step 5.\n\t Enter IP address igmp group \n\33[0m");
    printf("\tnow IP is %s \n", state->ipIgmpGroupStr);
    scanf("%[^\n]", tempStr);
    printf("\n you enter %s\n", tempStr);
    if (inet_pton(AF_INET, tempStr, (struct in_addr *)&(state->groupAddr.sin_addr.s_addr)) <= 0) // if error
    {
        printf("\n \33[37;1;41m ERROR IP try again! Or continue for leave as default\33[0m \n");
        state->groupAddr = tempSadr;
    }
    else
    {
        printf("IP changed .\n");
    }
p5:
    printf("\n enter to continue, \'1\' for repeat\n");
    while (temp = getchar() != 10);
    temp = getchar();
    if (temp == '1') { getchar(); goto pos5; }
    if (temp != 13 && temp != 10) goto p5;

                                            // local IP addres
pos6:
    memset(tempStr, 0, sizeof(tempStr));
    system("clear");
    printf("\33[37;1;42m manual settings to set as default. \33[0m");
    printf("\n\n\33[1m step 6.\n\t Choose number local IP address \33[0m \n\n");

	const int MAX_IFR = 20;
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
        printf("\t%i -> %s\n", i, inet_ntoa(sin->sin_addr)); // отладочное
        state->localAddr = *sin;
        //char * temp = inet_ntoa(sin->sin_addr);
        //memcpy(ipLoc, temp, strlen(temp)+1);
    }
   // getchar();
   int enter = -1;
sc1:
    printf("enter number");
    enter = scanf("%1i",&temp);
    if (!enter) { getchar(); goto sc1; }
    if ( temp < 0 || temp >= n)
    {
        printf("wrong number %i\n",temp);
        goto sc1;
    }
    else
    {
        sin = (struct sockaddr_in *) (&ifr[temp].ifr_addr);
        char * p = inet_ntoa(sin->sin_addr);
        memcpy(state->ipLocalStr, p, strlen(p)+1);
        state->localAddr = *sin;
    }

p6:
    printf("\n enter to continue, \'r\' for repeat\n");
    while (temp = getchar() != 10);
    temp = getchar();
    if (temp == '1') { getchar(); goto pos6; }
    if (temp != 13 && temp != 10) goto p6;


pos7:                                       //igmp port
    memset(tempStr, 0, sizeof(tempStr));
    system("clear");
    printf("\33[37;1;42m manual settings to set as default. \33[0m");
    printf("\n\n\33[1m step 6.\n\t Enter IPGP port \33[0m \n\n");

    if (!scanf("%s", tempStr)) goto pos7;
    temp = atoi(tempStr);
    if (temp <= 0) goto pos7;
    state->igmpGroupPort = htons(temp);
    printf("you enter %i \n",temp);
    printf("igmp porn now is %i  \n", state->igmpGroupPort);

p7:
    printf("\n enter to over, \'r\' for repeat\n");
    while (temp = getchar() != 10);
    temp = getchar();
    if (temp == '1') { getchar(); goto pos7; }
    if (temp != 13 && temp != 10) goto p7;


pos8:
    memset(tempStr, 0, sizeof(tempStr));
    system("clear");
    printf("\33[37;1;42m manual settings to set as default. \33[0m");
    printf("\n\n\33[1m step 6.\n\t Choose name ethernet card \33[0m \n\n");

    struct if_nameindex *ni;
    int i;
    ni = if_nameindex();
    if (ni == NULL)
    {
        perror("if_nameindex()");
        return;
    }

    for (i = 0; ni[i].if_index != 0 && ni[i].if_name != NULL; i++)
	{
		printf("\t %d: %s\n", i, ni[i].if_name);

	}

    enter = -1;
sc8:
    printf("enter number");
    enter = scanf("%1i",&temp);
    if (!enter) { getchar(); goto sc8; }
    if ( temp < 0 || temp >= i)
    {
        printf("wrong number %i\n",temp);
        goto sc8;
    }
    else
    {
        strcpy(state->nameEthernetCard , (ni[temp].if_name));	// переносим имя сетевухи в выходнуй массив
    }

p8:
    printf("\n enter to continue, \'r\' for repeat\n");
    while (temp = getchar() != 10);
    temp = getchar();
    if (temp == '1') { getchar(); goto pos8; }
    if (temp != 13 && temp != 10) goto p8;



}

