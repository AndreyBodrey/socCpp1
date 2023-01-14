
#include "stdint.h"

enum
{
    mode_video = 1,
    mode_udp,
    mode_ip
}WorkMode;

struct
{

} Status;


void packetHandler(char *buf, int bufLen);
void clearBuf(char * buff);
int packFiltr(char * bufer, int len);
void checkIgmp(char *buf, int bufLen);
void checkUdp(char *buf, int bufLen);
uint8_t serchIP2(char * );
int igmpJoin();
void findNetCardName(char * name);
void quit(int soc1, int soc2, char * str);
void logging(const char* str);
void writeDataToFile(char * data, int len);
