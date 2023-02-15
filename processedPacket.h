#ifndef PROC_PACK_H
#define PROC_PACK_H

#include "stdint.h"
#include <time.h>

enum errorsHandledPacket
{
	pack_error_size = 1,
	pack_error_scrambled,
	pack_error_time
};
#pragma pack(push,1)    // борьба с выравниваниванием струкур
typedef struct
{
	uint8_t protocol;                   // 1
	struct timeval timePackRecive;      // 16
	struct tm localTime; //added
	int countScrambled;                 // 4
	uint8_t igmpMessage;                // 1
	uint8_t checkSummError;             // 1
	int late;            //added
	int error;                          // 4
	int over;                           // 4

}HandledPacket;
#pragma pack(pop)










#endif //PROC_PACK_H
