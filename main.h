



void packetHandler(char *buf, int bufLen);

void clearBuf(char * buff);
int packFiltr(char * bufer, int len);

void checkIgmp(char *buf, int bufLen);
void checkUdp(char *buf, int bufLen);


void checkUdp(char *buf, int bufLen);
uint serchIP(char * );
int igmpJoin();
void findNetCardName(char * name);
void quit(int soc1, int soc2, char * str);
void logging(const char* str);
void writeDataToFile(char * data, int len);
