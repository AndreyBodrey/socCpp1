#ifndef PROC_PACK_H
#define PROC_PACK_H

#include "stdint.h"
#include <time.h>

enum errorsHandledPacket
{
	pack_error_size = 1
};

typedef struct
{
	uint8_t protocol;
	struct timeval timePackRecive;
	int countScrambled;
	uint8_t igmpMessage;
	uint8_t checkSummError;
	int error;
	int over;

}HandledPacket;











#endif //PROC_PACK_H
