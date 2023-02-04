/*  туду
    * параметры в ком строке
    * далее режимы работы
    * проверка и подтверждение igmp qwery
	* сохранение разных данных
    * сохранение и считывание настроек
    * функция интерфейса настройки
	* сохранение в pcap формате
	перехват сигналов завершения для отписки
	* пройтись по коду и посмотреть где нужен правильный выход из программы
	* удалить лишние режимы сохранения, оставить только видео и полностью пакеты
	сделать сщхранение igmp в формате рсар
	фильтр igmp пакетов !!!
	далее работа с сокетом юникс

*/

//#include <asm-generic/socket.h>

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <linux/if_ether.h>
#include <net/ethernet.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <strings.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h>
//#include <netinet/igmp.h>
#include <netinet/udp.h>
#include <time.h>
#include "linux/igmp.h"
//#include <linux/if.h>
#include <sys/ioctl.h>
#include <net/if.h>
//#include <linux/ip.h>
#include <netinet/ether.h>
#include <sys/un.h>

#include "startup.h"
#include "main.h"
#include "igmp.h"
#include "saveToFiles.h"
#include "igmpwork.h"
#include "processedPacket.h"


#define PACK_BUF_LEN 0xffff //макс длинна пакета, может столько и не надо, сколько там в сети максималка?
#define ADDRESS "/tmp/mySuperSocket.socket"


//global varibles
struct Status status;
thrd_data* threadData;
HandledPacket hPack;
/*
int main()
{
    int pid = startWork();
    printf("started, pid = %i \n", pid);
    sleep(3);
    stopWork();

    return 1;
}
*/
//--------------------------------------------------------------------------------------------------------
int istartWork()
{
    struct sockaddr_un sadr;
    char command[] = "work";

    memset(&sadr, 0, sizeof(struct sockaddr_un));
    sadr.sun_family = AF_UNIX;
    strncpy(sadr.sun_path, ADDRESS,sizeof(sadr.sun_path)-1);
    status.uxSock = socket(AF_UNIX, SOCK_SEQPACKET, 0);
    if (status.uxSock > 0) printf("Unix socket created = %i \n", status.uxSock);
    else
    {
        quit("soc not created ");
        return -1;
    }
    while (connect(status.uxSock, (struct sockaddr*)(&sadr), sizeof(struct sockaddr_un)) != 0)
    {
        perror("trying to connect...\n");
        sleep(1);
    }

    threadData = (thrd_data*)malloc(sizeof(thrd_data));
    threadData->command = (char*)malloc(strlen(command)+1);
    memset(threadData->command, 0, strlen(command)+1);
    strcpy(threadData->command, command);

    char countstr[] = "200";
    threadData->countd = (char*)malloc(sizeof(countstr)+1);
    memset(threadData->countd, 0, sizeof(countstr)+1);
    strcpy(threadData->countd, countstr);

    threadData->workMode = mode_infinity;

    pthread_t *pthread = (pthread_t*)malloc(sizeof(pthread_t));





    pthread_create(pthread, NULL, mainWork, threadData);

    pthread_detach(*pthread);
    unsigned long int threadPID = *pthread;
    int rez = threadPID & 0xffffffff;

    return rez;
}
//---------------------------------------------------------------------------------------------------------

int istopWork()
{
    printf("try to stop\n");
    threadData->workMode = mode_stop;
    while(threadData->workMode != mode_stoped);
    printf("process stoped\n");
    free(threadData->command);
    free(threadData->countd);
    free(threadData);
    printf("data deleted\n");
    close(status.uxSock);
    return 1;
}

//-------------------------------------------------------------------------------------------------------------------------------
//int main (int argc, char *argv[])

void* mainWork(void *thData)
{
    threadData = thData;
	char buf[PACK_BUF_LEN] = {0,};	//создаем и заполняем нулями буфер для пакета

	if (readSettings(&status) <= 0)
    {
        printf("settings not readed from file, go default\n");
        if (prepareSettings(&status) < 0)
        {
            quit("startup, inet_aton Group addr is wrong error: %d \n");
            return NULL;
        }
    }
    status.packetData = buf;
    int param = paramHanle(2, threadData, &status);
    if ( param < 0) { quit("params error 1"); return NULL;}
    if (param == 0) { quit("params error 2"); return NULL;}
    else if (param == 100)
    {
        changeSettingsInterface(&status);
        saveSettings(&status);
    }

    setFileName(&status);
		//этот тип сокета перехватывает все пакеты с заголовком ethernet
		//если я правильно понял, то пакеты с тетевухи летят на прямую и сюда и в ядро
	errno = 0;
	status.socketFd = socket(  PF_PACKET, SOCK_RAW, htons(ETH_P_ALL) );   // работает только от рута
	if (status.socketFd > 0) printf ("socket crated\n");
	else
    {
        printf("error : %d ", errno);
        quit("sniff soc not created\n");
        return NULL;
    }

	int rc = setsockopt(status.socketFd, SOL_SOCKET, SO_BINDTODEVICE, status.nameEthernetCard, strlen(status.nameEthernetCard));
	if (rc != 0)
    {
        quit("setsocopt not bind\n");
        return NULL;
    }


//  FILES WORKS
    if (status.workMode == mode_ethernet)
        createPcapFile();
    else
    {
        FILE *testFile = fopen(status.fileName,"r");
        if (testFile != NULL)
        {
            errno = 0;
            if (remove(status.fileName))
            {
                printf("file %s not deleted, error  %d \n", status.fileName, errno);
            }
            fclose(testFile);
        }
    }

    FILE *testFile = fopen(status.igmpFileName,"r");
	if (testFile != NULL)
	{
        errno = 0;
        if (remove(status.igmpFileName))
        {
            printf("file %s not deleted, error  %d \n", status.igmpFileName, errno);
        }
        fclose(testFile);
    }

						/// перевод сетевухи в неразборчивый режим
	struct ifreq ethreq;
	strncpy(ethreq.ifr_name, status.nameEthernetCard, IF_NAMESIZE);
	if (ioctl(status.socketFd, SIOCGIFFLAGS, &ethreq) == -1)
	{
		close(status.socketFd);
		quit("ioctl 1");
		return NULL;
	}
	ethreq.ifr_flags |= IFF_PROMISC;
	if (ioctl(status.socketFd, SIOCSIFFLAGS, &ethreq) == -1)
	{
		close(status.socketFd);
		quit("ioctl 2 ");
		return NULL;
	}

				///создаем и отправляем igmp join
   if (igmpSend(IGMPV2_HOST_MEMBERSHIP_REPORT, &status) > 0)
        status.igmpSubscibe = 1;
    else
    {
        quit("immgp MEMBERSHIP_REPORT not sended ");
        return NULL;
    }

    int dec = 0;
    uint32_t loop = status.workModeParam;
    time_t lastTime = time(NULL);
    status.ipHeader = (struct iphdr*)(status.packetData + sizeof(struct ethhdr));

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	while (loop)	 // пошла работа
	{

	    if (threadData->workMode == mode_stop)
        {
            igmpSend(IGMP_V2_LEAVE_GROUP, &status);
            igmpSend(IGMP_V2_LEAVE_GROUP, &status);
            threadData->workMode = mode_stoped;
            quit("stoped ");
            return NULL;
        }
		int reciveBytes = 0;
		reciveBytes = recvfrom(status.socketFd, buf,sizeof(buf),0,0,0); // получаем
		// 20 минимальный размер пакета
		if (reciveBytes < 20 && reciveBytes > PACK_BUF_LEN)	continue;

		if (gettimeofday(&(hPack.timePackRecive)))
			perror ("error to get time - gettimeofday ");
		status.udpHeader = (struct udphdr*)(buf + sizeof(struct ethhdr) + status.ipHeader->ihl* 4);
		status.igmpHeader = (struct igmp*)(buf + sizeof(struct ethhdr) + status.ipHeader->ihl* 4);

		dec = packetHandler(buf, reciveBytes); //если все ок то обрабатываем
        clearBuf(buf);
        time_t timeNow = time(NULL);
        switch( status.workMode )
        {
            case mode_video:
                if (timeNow != lastTime)
                {
                    lastTime = timeNow;
                    loop--;
                }
                break;
            case mode_infinity:

                break;
            default:
                loop -= dec;
                break;
        }
	}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	//printf("\n\nwork cycle completed\nmessage IGMP LEAVE sent\none minute left\n\n");


    igmpSend(IGMP_V2_LEAVE_GROUP, &status);
    igmpSend(IGMP_V2_LEAVE_GROUP, &status);
    status.igmpSubscibe = 0;
    if (threadData->workMode == mode_stop)
    {
        quit("mode_stop ");
        return NULL;
    }

	loop = 20; // or 60 sec or 60 packs

	while (loop)	 // пошла работа
	{

		int reciveBytes = 0;
		reciveBytes = recvfrom(status.socketFd, buf,sizeof(buf),0,0,0); // получаем
		// 20 минимальный размер пакета
		if (reciveBytes < 20 && reciveBytes > PACK_BUF_LEN)	continue;

		if (gettimeofday(&(status.packReciveTime)))
			perror ("error to get time - gettimeofday ");

		status.udpHeader = (struct udphdr*)(buf + sizeof(struct ethhdr) + status.ipHeader->ihl* 4);
		status.igmpHeader = (struct igmp*)(buf + sizeof(struct ethhdr) + status.ipHeader->ihl* 4);


		dec = packetHandler(buf, reciveBytes); //если все ок то обрабатываем
        clearBuf(buf);
        time_t timeNow = time(NULL);
        switch( status.workMode )
        {
            case mode_video:

                if (timeNow != lastTime)
                {
                    lastTime = timeNow;
                    loop--;
                }
                break;
           case mode_infinity:

                break;
            default:
                loop -= dec;
                break;
        }
	}

	printf("over\n");
	quit("game over.");

    return NULL;
}
//-------------------------------------------------------------------------------------

int packetHandler(char *bufer, int bufLen)
{

    int result = 0;
    time_t timeNow = time(NULL); // получаем локальное время с секундах (от начала нашей эры  плюс 1900 лет )  ))) если я правильно понял
	struct tm * timeS = localtime(&timeNow); // переводим время в структуру с кторорой можно получить нормальные часы, минуты итд
	char packetInfo[200] = {0,}; // буфер для формирования строки информиции об пакете

	uint8_t prot = status.ipHeader->protocol;
	switch(prot) //смотри что за пакет пришел
	{
		case IPPROTO_UDP:
					//если пакет не от группы на которую подписались, то на хер

			if (status.ipHeader->daddr != status.groupAddr.sin_addr.s_addr) break;
			if (ntohs(status.udpHeader->dest) != status.igmpGroupPort) break;                   //ports loocking

						//заполняем стоку packetInfo инфой о пакете
			/*sprintf(packetInfo, "%02d:%02d:%02d %d.%02d.%02d ",timeS->tm_hour,timeS->tm_min,
								timeS->tm_sec,timeS->tm_year+1900,timeS->tm_mon+1,timeS->tm_mday); // время
			strcpy(packetInfo + strlen(packetInfo), "UDP Lenght ");
			sprintf((packetInfo + strlen(packetInfo)),"% d : ", ntohs(status.ipHeader->tot_len)); // длинна всего пакета
			strcpy (packetInfo + strlen(packetInfo), " from ");
			strcpy (packetInfo + strlen(packetInfo), inet_ntoa(destAdr)); // от куда ip
			strcpy (packetInfo + strlen(packetInfo), " to ");
			strcpy (packetInfo + strlen(packetInfo), inet_ntoa(sorceAdr)); // куда ip
			sprintf((packetInfo + strlen(packetInfo))," destPort %d ", ntohs(status.udpHeader->uh_dport)); // порты от куда
			sprintf((packetInfo + strlen(packetInfo))," sorcePort %d \n", ntohs(status.udpHeader->uh_sport)); // и куда
				//выводим в терминал, пока что
			printf("%s", packetInfo);*/

			switch( status.workMode )
            {
                case mode_ethernet:
                    status.saveData = status.packetData;
                    status.dataLen = bufLen;
                    writePackToPcap(bufer, bufLen);
                    result = 1;
                    break;
                case mode_video:
                    status.saveData = status.packetData + sizeof(struct ethhdr) + status.ipHeader->ihl* 4 + sizeof(struct udphdr);
                    status.dataLen = bufLen - sizeof(struct ethhdr) - status.ipHeader->ihl* 4 - sizeof(struct udphdr);
                    writeDataToFile( status.saveData, status.dataLen); // пишем пакет в файл
                    break;
                case mode_infinity:
                       // printf(" infinityMode work! ");
						checkPack();
						write(status.uxSock, &hPack, sizeof(HandledPacket));
                    break;
                default:
                    status.saveData = bufer;
                    status.dataLen = bufLen;
                    break;
            }
			//break;  //с udp закончили


		case IPPROTO_IGMP:
					// все тоже что и с udp только с выборкой типа сообщения

			switch( status.workMode )
            {
                case mode_ethernet:
					writePackToPcap(bufer, bufLen);
					break;
                case mode_video:
					saveIgmpPacket(bufer, bufLen);
					break;
                case mode_infinity:
					checkPack();
					write(status.uxSock, &hPack, sizeof(HandledPacket));
                break;
                default:
                break;
			}
			//sprintf(packetInfo, "%02i:%02i:%02i %i.%02i.%02i ",timeS->tm_hour,timeS->tm_min,
			//					timeS->tm_sec,timeS->tm_year+1900,timeS->tm_mon+1,timeS->tm_mday);
			//strcpy(packetInfo, "Lenght ");
			//sprintf((packetInfo + strlen(packetInfo)),"%d : ", htons(status.ipHeader->tot_len));
			//if (status.igmpHeader->igmp_type == IGMP_V2_MEMBERSHIP_REPORT)
			//{
			//	strcpy (packetInfo + strlen(packetInfo), "IGMP_MEMBERSHIP_REPORT ");
			//}
			if (status.igmpHeader->igmp_type == IGMP_MEMBERSHIP_QUERY)
			{
				//strcpy (packetInfo + strlen(packetInfo), "IGMP_MEMBERSHIP_QUERY ");
				if (status.igmpSubscibe == 1)
				{
					if (igmpSend(IGMPV2_HOST_MEMBERSHIP_REPORT, &status) <= 0)
                    {
                        quit("igmp MEMBERSHIP_REPORT not sended\n");
                        return 0;
                    }

				}
				// тут будет посылатся подтверждение IGMP_V2_MEMBERSHIP_REPORT
				// при условии что не от сюда ушел запрс IGMP_V2_LEAVE_GROUP
			}
			//if (status.igmpHeader->igmp_type == IGMP_V2_LEAVE_GROUP)
			//{
			//	strcpy (packetInfo + strlen(packetInfo), "IGMP_LEAVE_GROUP ");
			//}

			//strcpy (packetInfo + strlen(packetInfo), inet_ntoa(status.igmpHeader->igmp_group));


			break;

							// остальное вроде не нужно
		case IPPROTO_TCP:
			//logging("got TCP pack");
			//write(status.uxSock, "got TCP pack", 13);
			break;
		case IPPROTO_ICMP:
			//logging("got icmp pack");
			break;
		default:;
			//logging("got some packet");
	}
    return result;

}
//-------------------------------------------------------------------------------------

void clearBuf(char * buff)
{
	int i = PACK_BUF_LEN;
	while(i--)
	{
		*buff = 0;
		++buff;
	}
}
//-------------------------------------------------------------------------------------

void quit( char * message)
{
	if (status.socketFd > 0) close(status.socketFd);
	if (status.uxSock > 0) close(status.uxSock);
	perror(message);
	if (igmpSend(IGMP_HOST_LEAVE_MESSAGE, &status) <= 0)
		printf("immgp MEMBERSHIP_REPORT not sended\n");

	//pthread_exit(0);
}

//-------------------------------------------------------------------------------------

void checkPack()
{
/*
	* uint8_t protocol;
	* struct timeval timePackRecive;
	* int countScrambled;
	* uint8_t igmpMessage;
	* uint8_t checkSummError;
	* int error;
	* int over;
*/



	memset(&hPack, 0, sizeof(HandledPacket));
	hPack.protocol = status.ipHeader->protocol;
	hPack.checkSummError = ip_check_sum(status.ipHeader, status.ipHeader->ihl*4);
	if (status.ipHeader->protocol == IPPROTO_IGMP)
	{
		hPack.igmpMessage = status.igmpHeader->igmp_type;
		return 1;
	}

	uint8_t *data = status.udpHeader + sizeof(struct udphdr);
	uint8_t * scrambledByte = data + sizeHeaderSubData - 1;
	int dataLen = status.udpHeader->len - sizeof(struct udphdr);
	int countParts = dataLen/sizeOfSubData;
	if ( dataLen%sizeOfSubData ) hPack.error = pack_error_size;

	for (int i = 0; i < countParts; i++)
	{
		if ((*scrambledByte) & ScrambledMask) hPack.countScrambled++;
		scrambledByte += sizeOfSubData;
	}
}


//-------------------------------------------------------------------------------------

