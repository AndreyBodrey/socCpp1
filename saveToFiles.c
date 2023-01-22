

#include "saveToFiles.h"
#include"startup.h"
#include <time.h>
#include <sys/time.h>



extern struct Status status;


int createPcapFile()
{
    struct pcapFileHeader pcapFileHdr;
    size_t result = 0;
    //CRATING FILE HEADER
                                    //  Magic Number (4 bytes) = a1 b2 c3 d4
    pcapFileHdr.magicDig[0] = 0xd4;
    pcapFileHdr.magicDig[1] = 0xc3;
    pcapFileHdr.magicDig[2] = 0xb2;
    pcapFileHdr.magicDig[3] = 0xa1;

                                    //  Version Major (2 bytes) = 00 02   Version Minor (2 bytes) = 00 04
    pcapFileHdr.varsionMajor[0] = 0x02;
    pcapFileHdr.varsionMajor[1] = 0x00;
    pcapFileHdr.varsionMinor[0] = 0x04;
    pcapFileHdr.varsionMinor[1] = 0x00;

                                    //  The timezone and accuracy fields aren’t used in practice, they should therefore be all zeroes.
    pcapFileHdr.timeZone[0] = 0x00;
    pcapFileHdr.timeZone[1] = 0x00;
    pcapFileHdr.timeZone[2] = 0x00;
    pcapFileHdr.timeZone[3] = 0x00;

                                    //  The timezone and accuracy fields aren’t used in practice, they should therefore be all zeroes.
    pcapFileHdr.timestampAccuracy[0] = 0x00;
    pcapFileHdr.timestampAccuracy[1] = 0x00;
    pcapFileHdr.timestampAccuracy[2] = 0x00;
    pcapFileHdr.timestampAccuracy[3] = 0x00;

                                    //  The snap length value is a 32 bit number indicating the maximum packet size
                                    //  that can be stored in the PCAP without truncating the packet data.
                                    //  “ff ff 00 00” (65535 bytes)
    pcapFileHdr.snapLength[0] = 0xff;
    pcapFileHdr.snapLength[1] = 0xff;
    pcapFileHdr.snapLength[2] = 0x00;
    pcapFileHdr.snapLength[3] = 0x00;

                                    //  The link layer type defines which type of packets the capture file contains.
                                    //  01 00 00 00 = IEEE 802.3 Ethernet
    pcapFileHdr.linkLayerType[0] = 0x01;
    pcapFileHdr.linkLayerType[1] = 0x00;
    pcapFileHdr.linkLayerType[2] = 0x00;
    pcapFileHdr.linkLayerType[3] = 0x00;

    // CHECKING FILE EXIST
    FILE *testFile = fopen(status.fileName,"r");
	if (testFile != NULL)
	{
	    printf("file %s exist. deleting... \n", status.fileName);
        errno = 0;
        if (remove(status.fileName))
        {
            printf("file %s not deleted, error  %d \n", status.fileName, errno);
            return -1;
        }
        else printf("file %s deleted \n", status.fileName);
        fclose(testFile);
    }

    //CREATING FILE AND WRITING HEADER
    printf("creating file %s...\n", status.fileName);
    testFile = fopen(status.fileName, "a");
    if (testFile == NULL)
    {
        printf("file %s is not created.\n", status.fileName);
        return -1;
    }
    else
    {
        printf("file %s created \n", status.fileName);
        result = fwrite(&pcapFileHdr, 1, sizeof(pcapFileHdr), testFile);
        fclose(testFile);
    }

    return result;
}
//-------------------------------------------------------------------------------------------------------------------

int writePackToPcap(char *buf, int len)
{

    struct pcapPacketHeader packHdr;
    struct timeval packTimeVal;
    int result;

    gettimeofday(&packTimeVal, NULL);

    packHdr.timestampSeconds = packTimeVal.tv_sec;
    packHdr.timestampMicroseconds = packTimeVal.tv_usec;
    packHdr.capturedLength = len;
    packHdr.originalLength = len;

    FILE *dataFile = fopen(status.fileName, "a");
    if (dataFile == NULL)
    {
        printf("error, can not open file %s for packet write /n",status.fileName);
        return -1;
    }
    fwrite(&packHdr, 1, sizeof(packHdr), dataFile); // write pcap packet header
    result =  fwrite(buf, 1, len, dataFile);        // write packet data
    fclose(dataFile);

    return result;
}
