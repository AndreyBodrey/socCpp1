/*  туду
    параметры в ком строке
    далее режимы работы
    сохранение разных данных
    работа с файлом настроек
    функция ручной настройки



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
//#include <net/if.h>
//#include <linux/ip.h>
#include <netinet/ether.h>

#include "main.h"
#include "startup.h"
#include "igmp.h"


#define PACK_BUF_LEN 0xffff //макс длинна пакета, может столько и не надо, сколько там в сети максималка?

#define MC_GROUP_PORT 2015

//global varibles
	int countLoop = 2000000;  // пока для отладки цикл не бесконечный, это сколько раз он прокрутится, 1 остчет = один пакет любой
    struct Status status;


//---------------------------------------------
int main (int argc, char *argv[])
{

	char buf[PACK_BUF_LEN] = {0,};	//создаем и заполняем нулями буфер для пакета

	if (prepareSettings(&status) == 0)
    {
        printf("startup, inet_aton Group addr is wrong error: %d \n", errno);
        exit(1);
    }
    status.packetData = buf;

    igmpSend(IGMPV2_HOST_MEMBERSHIP_REPORT, &status);

		//этот тип сокета перехватывает все пакеты с заголовком ethernet
		//если я правильно понял, то пакеты с тетевухи летят на прямую и сюда и в ядро
	errno = 0;
	status.socketFd = socket(  PF_PACKET, SOCK_PACKET, htons(ETH_P_ALL) );   // работает только от рута
	if (status.socketFd > 0) printf ("socket crated\n");
	else
    {
        printf("error : %d ", errno);
        quit(status.socketFd, "sniff soc not created\n");
    }

			//enp4s0 spirovo
	char netCardName[20] = {0,};
	findNetCardName(netCardName); // ищем имя сетевухи для следущ строки
		// привязываем сокет к сетевухе
	int rc = setsockopt(status.socketFd,SOL_SOCKET, 25, netCardName, strlen(netCardName) + 1);
	if (rc != 0) quit(status.socketFd, "setsocopt not bind\n");

	printf ("setSock Opt %d, error %d \n" , rc, errno);	 // дебаг инфо )

	/*	// раскоментировать это для  отлавливания всех пакетов даже не для нашего мака
	struct ifreq interface;
	interface.ifr_flags |= IFF_PROMISC;
	errno = 0;
	if (ioctl(soc,SIOCSIFFLAGS,&interface) < 0)
	{
		printf (" eerror in ioctl  error num = %d \n" , errno);
		close(soc);
	}
	*/


	while (countLoop--)	 // пошла работа
	{
		int reciveBytes = 0;
		reciveBytes = recvfrom(status.socketFd,buf,sizeof(buf),0,0,0); // получаем
		// 20 минимальный размер пакета
		if (reciveBytes > 20 && reciveBytes < PACK_BUF_LEN)	packetHandler(buf, reciveBytes); //если все ок то обрабатываем
	}


	printf("over\n");
	quit(status.socketFd, "game over.");

}

//**************************************************************************************



void packetHandler(char *bufer, int bufLen)
{
	//printf ("got packet, size = %d \n", bufLen);
    time_t timeNow = time(NULL); // получаем локальное время с секундах (от начала нашей эры  плюс 1900 лет )  ))) если я правильно понял
	struct tm * timeS = localtime(&timeNow); // переводим время в структуру с кторорой можно получить нормальные часы, минуты итд
	char packetInfo[200] = {0,}; // буфер для формирования строки информиции об пакете

								// указатели на структуры заголовков
  //  struct ethhdr *ethernetHeader;
    struct iphdr *ipH ;
	struct igmp *igmpHdr;
	struct udphdr *udpHdr;
								//структуры адресов
	struct in_addr sorceAdr;
	struct in_addr destAdr;


								// получаем адреса заголовков для скорости рабртаем с указателями без копирования
  //  ethernetHeader = (struct ethhdr*)bufer;
    ipH = (struct iphdr*)(bufer + sizeof(struct ethhdr));
	igmpHdr = (struct igmp*)(bufer + sizeof(struct ethhdr) + ipH->ihl* 4);
	udpHdr = (struct udphdr*)(bufer + sizeof(struct ethhdr) + ipH->ihl* 4);
								// адреса копируются
	sorceAdr.s_addr = ipH->saddr;
	destAdr.s_addr = ipH->daddr;

	switch(ipH->protocol) //смотри что за пакет пришел
	{
		case IPPROTO_UDP:
			//	отсеиваем локальные пакеты с 127.х.х.х
			//if (*((uint8_t*) &sorceAdr.s_addr) == 127) break; // пока не отсеиваем )

					//если пакет не от группы на которую подписались, то на хер
			if (ipH->daddr!= status.groupAddr.sin_addr.s_addr) break;
			if (udpHdr->dest != MC_GROUP_PORT) break;                   //ports loocking

			// далше проходят пакеты udp от ip группы пока больше проверок нет
			//думаю может стоит дяобвить проверку на наш ли ip пакет пришел чтоб какиенить левые данные не пролезли ?

						//заполняем стоку packetInfo инфой о пакете
			sprintf(packetInfo, "%02d:%02d:%02d %d.%02d.%02d ",timeS->tm_hour,timeS->tm_min,
								timeS->tm_sec,timeS->tm_year+1900,timeS->tm_mon+1,timeS->tm_mday); // время
			strcpy(packetInfo + strlen(packetInfo), "UDP Lenght ");
			sprintf((packetInfo + strlen(packetInfo)),"% d : ", htons(ipH->tot_len)); // длинна всего пакета
			strcpy (packetInfo + strlen(packetInfo), " from ");
			strcpy (packetInfo + strlen(packetInfo), inet_ntoa(destAdr)); // от куда ip
			strcpy (packetInfo + strlen(packetInfo), " to ");
			strcpy (packetInfo + strlen(packetInfo), inet_ntoa(sorceAdr)); // куда ip
			sprintf((packetInfo + strlen(packetInfo))," destPort %d ", htons(udpHdr->uh_dport)); // порты от куда
			sprintf((packetInfo + strlen(packetInfo))," sorcePort %d ", htons(udpHdr->uh_sport)); // и куда
				//выводим в терминал, пока что
			logging(packetInfo);


						// получаем указатель на данные в пакете, после udp заголовка
			char *udpData = bufer + sizeof(struct ethhdr) + ipH->ihl* 4 + sizeof(struct udphdr);
						// считаем длинну данных
			int udpDataLen = ntohs(udpHdr->len) - sizeof(struct udphdr);
			printf("udpDatalen %d \n", udpDataLen ); //отладочное

			writeDataToFile( udpData,  udpDataLen); // пишем пакет в файл

			break;  //с udp закончили


		case IPPROTO_IGMP:
			logging("got IGMP pack");
					// все тоже что и с udp только с выборкой типа сообщения
			sprintf(packetInfo, "%02d:%02d:%02d %d.%02d.%02d ",timeS->tm_hour,timeS->tm_min,
								timeS->tm_sec,timeS->tm_year+1900,timeS->tm_mon+1,timeS->tm_mday);
			strcpy(packetInfo, "Lenght ");
			sprintf((packetInfo + strlen(packetInfo)),"%d : ", htons(ipH->tot_len));
			if (igmpHdr->igmp_type == IGMP_V2_MEMBERSHIP_REPORT)
			{
				strcpy (packetInfo + strlen(packetInfo), "IGMP_MEMBERSHIP_REPORT ");
			}
			if (igmpHdr->igmp_type == IGMP_MEMBERSHIP_QUERY)
			{
				strcpy (packetInfo + strlen(packetInfo), "IGMP_MEMBERSHIP_QUERY ");
				// тут будет посылатся подтверждение IGMP_V2_MEMBERSHIP_REPORT
				// при условии что не от сюда ушел запрс IGMP_V2_LEAVE_GROUP
			}
			if (igmpHdr->igmp_type == IGMP_V2_LEAVE_GROUP)
			{
				strcpy (packetInfo + strlen(packetInfo), "IGMP_LEAVE_GROUP ");
			}

			strcpy (packetInfo + strlen(packetInfo), inet_ntoa(igmpHdr->igmp_group));
			logging(packetInfo);
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



    clearBuf(bufer);
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
	exit(0);
}
//-------------------------------------------------------------------------------------

void logging(const char* str)
{
	printf("%s",str);
	printf("\n");
}
//-------------------------------------------------------------------------------------

void writeDataToFile(char * data, int len)
{
	const int  MAX_FILE_SIZE = 10000000; // не маловато ли?
	char fileName[20];
							// статик чтоб не забывались при выходе из функции
	static int numFile = 0;	// тут номер текущего файла
	static int lenFile = 0;	// тут размер текущего файла

	if (lenFile + len > MAX_FILE_SIZE) 	// если размер файла превысит, то начинаем новый
	{									// возможно стоит сделать циклическую перезапись файлов?
		++numFile;
		lenFile = 0;
	}
	sprintf(fileName,"dataFile_%d", numFile); // получаеь имя файла dataFile_0 dataFile_1 dataFile_2 и тд
	FILE *dataFile = fopen(fileName, "a");	// открываем для записи в конец файла
	if (dataFile == NULL)
	{
		logging("file open error");
		return;
	}
	lenFile += len;		// прибавляем длинну пришедших данных к размеру файла
	fwrite(data, sizeof(char), len, dataFile);		// пишем данные

		// была попытка разделить пакеты меж собой наверно это не нужно, хотя и с ним показвает )))
	fwrite("\n\n\n", sizeof(char), 7, dataFile);
	fclose(dataFile);
}
//-------------------------------------------------------------------------------------


