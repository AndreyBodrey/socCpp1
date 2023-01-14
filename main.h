
#include "stdint.h"



 #define GROUP_IP "239.255.10.101"
#define __USE_MISC


void packetHandler(char *buf, int bufLen);
void clearBuf(char * buff);
int packFiltr(char * bufer, int len);
void checkIgmp(char *buf, int bufLen);
void checkUdp(char *buf, int bufLen);

void quit(int soc1, char * str);
void logging(const char* str);
void writeDataToFile(char * data, int len);
