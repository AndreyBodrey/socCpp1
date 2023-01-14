#include <stdio.h>
#include <stdlib.h>
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
#include <netinet/igmp.h>
#include <netinet/udp.h>
#include <time.h>
//#include <linux/if.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <linux/ip.h>
#include <netinet/ether.h>

#include "main.h"


#define PACK_BUF_LEN 0xffff //макс длинна пакета, может столько и не надо, сколько там в сети максималка?

#define MC_GROUP_ADDRES "239.255.10.101"
#define MC_GROUP_PORT 2015

//global varibles
	int countLoop = 2000000;  // пока для отладки цикл не бесконечный, это сколько раз он прокрутится, 1 остчет = один пакет любой

	//char ipLockalStrFormat[20];  	// для локального айпи в формате строки

	in_addr_t ipIgmpGroup_int;		// айпи группы в int

int main ()
{
	//in_addr_t ipLockal_int;  // для локального айпи ток в формате int пока не нужно

	struct in_addr ipIgmpGroup_inadr; //структурка для хранения адреса группы

	//ipLockal_int = serchIP(ipLockalStrFormat);
	inet_pton(AF_INET, MC_GROUP_ADDRES, &ipIgmpGroup_inadr); //тут заполняет структуру ipIgmpGroup_inadr из строки "239.255.10.101"
	ipIgmpGroup_int = ipIgmpGroup_inadr.s_addr;  //зачем это сделал уже не помню наверно надо удалить )


	char buf[PACK_BUF_LEN] = {0,};	//создаем и заполняем нулями буфер для пакета

	int soc = -1;
	int socIgmp = -1;	// два будущих сокета

	printf("hello nigga !\n\n"); // приветствие


	//serchIP(ipLockalStrFormat);  функция поиска локального айпи пок ане надо


	socIgmp = igmpJoin();	//подписываемся на рассылку
	if ( socIgmp < 0 ) quit(soc, socIgmp,"igmp soc not created\n"); //error

		//этот тип сокета перехватывает все пакеты с заголовком ethernet
		//если я правильно понял, то пакеты с тетевухи летят на прямую и сюда и в ядро
	soc = socket(  PF_PACKET, SOCK_PACKET, htons(ETH_P_ALL) );   // работает только от рута
	if (soc > 0) printf ("socket crated\n");
	else  quit(soc, socIgmp,"sniff soc not created\n");

			//enp4s0 spirovo
	char netCardName[20] = {0,};
	findNetCardName(netCardName); // ищем имя сетевухи для следущ строки
		// привязываем сокет к сетевухе
	int rc = setsockopt(soc,SOL_SOCKET, SO_BINDTODEVICE, netCardName, strlen(netCardName) + 1);
	if (rc != 0) quit(soc, socIgmp,"setsocopt not bind\n");

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
		reciveBytes = recvfrom(soc,buf,sizeof(buf),0,0,0); // получаем
		// 20 минимальный размер пакета
		if (reciveBytes > 20 && reciveBytes < PACK_BUF_LEN)	packetHandler(buf, reciveBytes); //если все ок то обрабатываем
	}


	printf("over\n");
	quit(soc, socIgmp, "game over.");

}

//**************************************************************************************

			// тут пока-что много лишней хрени и функций

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
	printf("ressived UDP pack \n");
}
//-------------------------------------------------------------------------------------

int packFiltr(char * bufer, int len)
{
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
			if (destAdr.s_addr != ipIgmpGroup_int) break;

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



	return ipH->protocol;
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

uint32_t serchIP2(char * ipLoc)  // поиск локального айпи компа в сети, пусть будет пригодится гденить)
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

					// здесь тупо код скопирован с просторов,
					// сокет не закрываю ибо призакрытии от посылает  IGMP_V2_LEAVE_GROUP
					// уже готова своя функция, осталось только приладить
int igmpJoin(void)
{
	int socI = socket( AF_INET, SOCK_DGRAM, 0 );
	if ( socI < 0 ) return 0; //error

	const int optval = 1;
	setsockopt(socI, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)); // хз зачаем это
	struct sockaddr_in addr;
	bzero(&addr, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(MC_GROUP_PORT);
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	//bind(socIgmp, (struct sockaddr*)&addr, sizeof(addr)); // это не обязательно )
	struct ip_mreq mreq;
	inet_aton(MC_GROUP_ADDRES, &(mreq.imr_multiaddr));
	mreq.imr_interface.s_addr = htonl(INADDR_ANY);

							// эта хрень посылает MEMBERSHIP репорт
	setsockopt(socI, IPPROTO_IP, IP_ADD_MEMBERSHIP, &mreq, sizeof(mreq));

	return socI;

}
//-------------------------------------------------------------------------------------

void findNetCardName(char * name) // ищем имя сетевухи хватаем первую что не "lo" локалхост 127.х.х.х
									// этот код то же с просторов, но с небольшой доработкой выбора
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
//-------------------------------------------------------------------------------------

void quit(int soc1, int soc2, char * message)
{
	if (soc1 > 0) close(soc1);
	if (soc2 > 0) close(soc2);
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

//                  пока что все,  далее функция для отправки самодельного igmp пакета пока что в отдельном виде
//					наверно она же будет отправлять и IGMP_LEAVE. проверял ток через wirewshark, в деле еще не  проверил
/*

#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/ip.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <linux/igmp.h>

#define DEST "239.255.10.101"
#define SORS "192.168.0.112" // ))) чисто для проверки) вот тут и пригодится функция поиска локального айпи


								// тут как проверка так и составление контрольной суммы

uint16_t ip_check_sum(uint16_t *buf, uint8_t len)
{
    uint32_t sum = 0;
    while (len>1)
    {
        sum += *buf;
        buf ++;
        len -= 2;
    }
    if (len)
        sum += (uint16_t)((uint8_t)*buf << 8);

    while (sum >> 16)
        sum = (sum & 0xffff) + (sum >> 16);

    return ~((uint16_t)sum);
}
//-------------------------------------------------------------------------

int main(void)
{

	int s;
	struct sockaddr_in daddr; // куда
	struct sockaddr_in laddr; // от куда

	char packet[32] = {0,};


	struct iphdr *ip = (struct iphdr *)packet; //привязка указателя на структуру и началу буфера пакета



	if ((s = socket(AF_INET, SOCK_RAW, IPPROTO_RAW)) < 0) // сырой сокет, в линуксе по умолчанию ip заголовок надо делать в ручную для него
	{
		perror("error:");
		exit(1);
	}
								// наполняе структуры адресов тут половина или даже все не нужно потом попробую упрастить
	daddr.sin_family = AF_INET;
	daddr.sin_port = 2015;
	inet_pton(AF_INET, DEST, (struct in_addr *)&daddr.sin_addr.s_addr);
	memset(daddr.sin_zero, 0, sizeof(daddr.sin_zero));

	laddr.sin_family = AF_INET;
	laddr.sin_port = 0;
	inet_pton(AF_INET, SORS, (struct in_addr *)&laddr.sin_addr.s_addr);
	memset(laddr.sin_zero, 0, sizeof(laddr.sin_zero));


					//заполняем заголовки
	ip->ihl = 6;
	ip->version = 4;
	ip->tos = 0;
	ip->tot_len = htons(32);
	ip->frag_off = 0;
	ip->ttl = 1;
	ip->protocol = IPPROTO_IGMP;
	ip->check = 0;
	ip->saddr = laddr.sin_addr.s_addr;
	ip->daddr = daddr.sin_addr.s_addr;
	packet[20] = 0x94; packet[21] = 04; // ip опции какойто router alert хз зачем оно нужно но нон есть в примерах из wireshark)

				// в указатель на заголовок igmp пихаем адрес буфера после ip заголовка
	struct igmphdr *join = (struct igmphdr *) packet + ip->ihl *4;

				// заполняем структуру
    join->type = IGMPV2_HOST_MEMBERSHIP_REPORT;
    join->code = 100;							//max resp time,  хз какое время, ответа написал 10 сек от балды
    join->csum = 0;								// сначала нули чтоб посчитать сумму и зхаписать
    join->group = daddr.sin_addr.s_addr;
    join->csum = ip_check_sum((uint16_t *)join, sizeof(struct igmphdr));

								//закоментил  первые опыты жесткого заполнения массива)))
	//packet[20] = 0x94; packet[21] = 04;
	//packet[24] = 0x16; packet[26] = 0xef; packet[27] = 0x59; packet[28] = 0xef; packet[29] = 0xff; packet[30] = 0x0a; packet[31] = 0x65;




					//отсылаем, потом буду пробывать отсылать и принимать через один сокет

		if (sendto(s, (char *)packet, sizeof(packet), 0,(struct sockaddr *)&daddr, (socklen_t)sizeof(daddr)) < 0)
			perror("packet send error:");

}




*/
