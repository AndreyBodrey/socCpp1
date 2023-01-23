#ifndef SAVETOFILES_H_INCLUDED
#define SAVETOFILES_H_INCLUDED

#include <stdlib.h>
#include <stdint.h>
#include <time.h>
#include "startup.h"


struct pcapFileHeader
{
    uint8_t magicDig[4];
    uint8_t varsionMajor[2];
    uint8_t varsionMinor[2];
    uint8_t timeZone[4];
    uint8_t timestampAccuracy[4];
    uint8_t snapLength[4];
    uint8_t linkLayerType[4];

};


struct pcapPacketHeader
{
    uint32_t timestampSeconds;
    uint32_t timestampMicroseconds;
    uint32_t capturedLength;
    uint32_t originalLength;
};

int createPcapFile();
int writePackToPcap(char *, int );
void saveIgmpPacket(char *buf, int len);
void writeDataToFile(char * data, int len);
int readSettings(struct Status * state);
int saveSettings(struct Status * state);



#endif // SAVETOFILES_H_INCLUDED
