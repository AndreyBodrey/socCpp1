



void packetHandler(char *buf, int bufLen);

void clearBuf(char * buff);
struct ethhdr * packFiltr(char * bufer, int len);

void checkIgmp(char *buf, int bufLen);
void checkUdp(char *buf, int bufLen);