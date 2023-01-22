#ifndef SAVETOFILES_H_INCLUDED
#define SAVETOFILES_H_INCLUDED

#include <stdlib.h>
#include <stdint.h>
#include <time.h>


/*
Some of the most common link-layer type values are:

01 00 00 00 = IEEE 802.3 Ethernet
65 00 00 00 = Raw IP packets (no layer 2 header)
69 00 00 00 = IEEE 802.11 (WiFi)
71 00 00 00 = SLL (Linux "cooked" capture encapsulation)
77 00 00 00 = Prism header + IEEE 802.11 (WiFi)
7f 00 00 00 = Radiotap header + IEEE 802.11 (WiFi)
c3 00 00 00 = IEEE 802.15.4 (Zigbee)
c5 00 00 00 = Endace ERF
e4 00 00 00 = Raw IPv4 (no layer 2 header)
*/

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

/*   packet header
Timestamp Seconds (4 bytes)
Timestamp Microseconds (4 bytes)
Captured Length (4 bytes)
Original Length (4 bytes)
*/

struct pcapPacketHeader
{
    uint32_t timestampSeconds;
    uint32_t timestampMicroseconds;
    uint32_t capturedLength;
    uint32_t originalLength;
};

int createPcapFile();
int writePackToPcap(char *, int );




#endif // SAVETOFILES_H_INCLUDED
