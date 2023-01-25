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
	избавится от глобального статуса
	сделать сщхранение igmp в формате рсар

*/


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

#include "startup.h"
#include "main.h"

#include "igmp.h"
#include "saveToFiles.h"


#define PACK_BUF_LEN 0xffff //макс длинна пакета, может столько и не надо, сколько там в сети максималка?



//global varibles
struct Status status;


//---------------------------------------------
int main (int argc, char *argv[])
{

	char buf[PACK_BUF_LEN] = {0,};	//создаем и заполняем нулями буфер для пакета

	if (readSettings(&status) <= 0)
    {
        printf("settings not readed from file, go default\n");
        if (prepareSettings(&status) < 0)
        {
            printf("startup, inet_aton Group addr is wrong error: %d \n", errno);
            exit(1);
        }
    }
    status.packetData = buf;
    int param = paramHanle(argc, argv, &status);
    if ( param < 0) return 1;
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
        quit(status.socketFd, "sniff soc not created\n");
    }

	int rc = setsockopt(status.socketFd, SOL_SOCKET, SO_BINDTODEVICE, status.nameEthernetCard, strlen(status.nameEthernetCard));
	if (rc != 0) quit(status.socketFd, "setsocopt not bind\n");


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
		perror("ioctl");
		close(status.socketFd);
		exit(1);
	}
	ethreq.ifr_flags |= IFF_PROMISC;
	if (ioctl(status.socketFd, SIOCSIFFLAGS, &ethreq) == -1)
	{
		perror("ioctl");
		close(status.socketFd);
		exit(1);
	}
				///создаем и отправляем igmp join
   if (igmpSend(IGMPV2_HOST_MEMBERSHIP_REPORT, &status) > 0)
        status.igmpSubscibe = 1;
    else
    {
        printf("immgp MEMBERSHIP_REPORT not sended\n");
        return 1;
    }

    int dec = 0;
    uint32_t loop = status.workModeParam;
    time_t lastTime = time(NULL);
    status.ipHeader = (struct iphdr*)(status.packetData + sizeof(struct ethhdr));

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	while (loop)	 // пошла работа
	{
		int reciveBytes = 0;
		reciveBytes = recvfrom(status.socketFd, buf,sizeof(buf),0,0,0); // получаем
		// 20 минимальный размер пакета
		if (reciveBytes < 20 && reciveBytes > PACK_BUF_LEN)	continue;

		status.udpHeader = (struct udphdr*)(buf + sizeof(struct ethhdr) + status.ipHeader->ihl* 4);
		status.igmpHeader = (struct igmp*)(buf + sizeof(struct ethhdr) + status.ipHeader->ihl* 4);

		dec = packetHandler(buf, reciveBytes); //если все ок то обрабатываем
        clearBuf(buf);

        switch( status.workMode )
        {
            case mode_video:
                time_t timeNow = time(NULL);
                if (timeNow != lastTime)
                {
                    lastTime = timeNow;
                    loop--;
                }
                break;
            default:
                loop -= dec;
                break;
        }
	}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	printf("\n\nwork cycle completed\nmessage IGMP LEAVE sent\none minute left\n\n");

	//if (status.igmpSubscibe)
    //{
        igmpSend(IGMP_V2_LEAVE_GROUP, &status);
        status.igmpSubscibe = 0;
    //}
	loop = 20; // or 60 sec or 60 packs

	while (loop)	 // пошла работа
	{

		int reciveBytes = 0;
		reciveBytes = recvfrom(status.socketFd, buf,sizeof(buf),0,0,0); // получаем
		// 20 минимальный размер пакета
		if (reciveBytes < 20 && reciveBytes > PACK_BUF_LEN)	continue;

		status.udpHeader = (struct udphdr*)(buf + sizeof(struct ethhdr) + status.ipHeader->ihl* 4);
		status.igmpHeader = (struct igmp*)(buf + sizeof(struct ethhdr) + status.ipHeader->ihl* 4);

		dec = packetHandler(buf, reciveBytes); //если все ок то обрабатываем
        clearBuf(buf);

        switch( status.workMode )
        {
            case mode_video:
                time_t timeNow = time(NULL);
                if (timeNow != lastTime)
                {
                    lastTime = timeNow;
                    loop--;
                }
                break;
            default:
                loop -= dec;
                break;
        }
	}

	printf("over\n");
	quit(status.socketFd, "game over.");

}
//-------------------------------------------------------------------------------------

int packetHandler(char *bufer, int bufLen)
{

    int result = 0;
    time_t timeNow = time(NULL); // получаем локальное время с секундах (от начала нашей эры  плюс 1900 лет )  ))) если я правильно понял
	struct tm * timeS = localtime(&timeNow); // переводим время в структуру с кторорой можно получить нормальные часы, минуты итд
	char packetInfo[200] = {0,}; // буфер для формирования строки информиции об пакете

								//структуры адресов
	//struct in_addr sorceAdr;
	//struct in_addr destAdr;
								// адреса копируются
	//sorceAdr.s_addr = status.ipHeader->saddr;
	//destAdr.s_addr = status.ipHeader->daddr;

	uint8_t prot = status.ipHeader->protocol;
	switch(prot) //смотри что за пакет пришел
	{
		case IPPROTO_UDP:
					//если пакет не от группы на которую подписались, то на хер

			if (status.ipHeader->daddr != status.groupAddr.sin_addr.s_addr) break;
			if (ntohs(status.udpHeader->dest) != status.igmpGroupPort) break;                   //ports loocking


			// далше проходят пакеты udp от ip группы пока больше проверок нет
			//думаю может стоит дяобвить проверку на наш ли ip пакет пришел чтоб какиенить левые данные не пролезли ?

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
                default:
                    status.saveData = bufer;
                    status.dataLen = bufLen;
                    break;
            }
			break;  //с udp закончили


		case IPPROTO_IGMP:
					// все тоже что и с udp только с выборкой типа сообщения

			sprintf(packetInfo, "%02d:%02d:%02d %d.%02d.%02d ",timeS->tm_hour,timeS->tm_min,
								timeS->tm_sec,timeS->tm_year+1900,timeS->tm_mon+1,timeS->tm_mday);
			strcpy(packetInfo, "Lenght ");
			sprintf((packetInfo + strlen(packetInfo)),"%d : ", htons(status.ipHeader->tot_len));
			if (status.igmpHeader->igmp_type == IGMP_V2_MEMBERSHIP_REPORT)
			{
				strcpy (packetInfo + strlen(packetInfo), "IGMP_MEMBERSHIP_REPORT ");
			}
			if (status.igmpHeader->igmp_type == IGMP_MEMBERSHIP_QUERY)
			{
				strcpy (packetInfo + strlen(packetInfo), "IGMP_MEMBERSHIP_QUERY ");
				if (status.igmpSubscibe == 1)
				{
					if (igmpSend(IGMPV2_HOST_MEMBERSHIP_REPORT, &status) <= 0)
						quit(status.socketFd,"igmp MEMBERSHIP_REPORT not sended\n");
				}
				// тут будет посылатся подтверждение IGMP_V2_MEMBERSHIP_REPORT
				// при условии что не от сюда ушел запрс IGMP_V2_LEAVE_GROUP
			}
			if (status.igmpHeader->igmp_type == IGMP_V2_LEAVE_GROUP)
			{
				strcpy (packetInfo + strlen(packetInfo), "IGMP_LEAVE_GROUP ");
			}

			strcpy (packetInfo + strlen(packetInfo), inet_ntoa(status.igmpHeader->igmp_group));

			if (status.workMode == mode_video) saveIgmpPacket(bufer, bufLen);
			else writePackToPcap(bufer, bufLen);

			printf("%s %s", packetInfo, "\n");
			break;

							// остальное вроде не нужно
		case IPPROTO_TCP:
			//logging("got TCP pack");
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

void quit(int soc1, char * message)
{
	if (soc1 > 0) close(soc1);
	printf("%s", message);
	if (igmpSend(IGMP_HOST_LEAVE_MESSAGE, &status) <= 0)
		printf("immgp MEMBERSHIP_REPORT not sended\n");

	exit(0);
}
//-------------------------------------------------------------------------------------



//-------------------------------------------------------------------------------------

