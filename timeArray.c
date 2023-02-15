
#include <stdlib.h>

static int timeArrCount;
static int timeArrPointer;
static unsigned int *timeArr;
static int timeArrMax;

int timeArrInit(int count) //return 0 if error else 1
{
    timeArrMax = count;
    timeArrCount = 0;
    timeArrPointer = 0;
    timeArr = (unsigned int*) calloc(count, sizeof(int));
    if (timeArr == NULL) return 0;

    return 1;
}
//--------------------------------------------------------------
void timeArrDeinit()
{
    free(timeArr);
    timeArr = NULL;
}
//--------------------------------------------------------------
void timeArrInsert(unsigned int tim)
{
    timeArr[timeArrPointer] = tim;
    timeArrPointer++;
    if (timeArrPointer >= timeArrMax) timeArrPointer = 0;
    if (timeArrCount < timeArrMax) timeArrCount++;
}
//--------------------------------------------------------------
unsigned int timeArrGetAverge()
{
    unsigned int result = 0;
    int i = 0;
    do
    {
        result += timeArr[i];
        i++;
    } while(i < timeArrCount);
    return result / timeArrCount;
}
//--------------------------------------------------------------
int timeArrIsInited()
{
    if (timeArr != NULL) return 1;
    else return 0;
}
