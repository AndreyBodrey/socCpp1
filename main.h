
#include "stdint.h"
#include "params.h"


 #define GROUP_IP "239.255.10.101"
 #define MC_GROUP_PORT 2015

 //#define __USE_MISC 1




void* mainWork(void *thData);
int packetHandler(char *buf, int bufLen);
void clearBuf(char * buff);
int packFiltr(char * bufer, int len);
void checkIgmp(char *buf, int bufLen);
void checkUdp(char *buf, int bufLen);

void quit(char * str);



