



void packetHandler(char *buf, int bufLen);

void clearBuf(char * buff);
int packFiltr(char * bufer, int len);

void checkIgmp(char *buf, int bufLen);
void checkUdp(char *buf, int bufLen);


void checkUdp(char *buf, int bufLen);
uint serchIP(char * );
void igmpJoy();
void findNetCardName(char * name);