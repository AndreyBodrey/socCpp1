
#include <stdlib.h>
#include "timeArray.h"
/*
static int timeArrCount;
static int timeArrPointer;
static unsigned int *timeArr;
static int timeArrMax;
*/

int timeArrInit(TimeArray * arr, int count) //return 0 if error else 1
{
    arr->timeArrMax = count;
    arr->timeArrCount = 0;
    arr->timeArrPointer = 0;
    arr->timeArr = (unsigned int*) calloc(count, sizeof(int));
    if (arr->timeArr == NULL) return 0;
    return 1;
}
//--------------------------------------------------------------
void timeArrDeinit(TimeArray * arr)
{
    free(arr->timeArr);
    arr->timeArr = NULL;
}
//--------------------------------------------------------------
void timeArrInsert(TimeArray * arr, unsigned int tim)
{
    arr->timeArr[arr->timeArrPointer] = tim;
    arr->timeArrPointer++;
    if (arr->timeArrPointer >= arr->timeArrMax) arr->timeArrPointer = 0;
    if (arr->timeArrCount < arr->timeArrMax) arr->timeArrCount++;
}
//--------------------------------------------------------------
unsigned int timeArrGetAverge(TimeArray * arr)
{
    unsigned int result = 0;
    int i = 0;
    do
    {
        result += arr->timeArr[i];
        i++;
    } while(i < arr->timeArrCount);
    return result / arr->timeArrCount;
}
//--------------------------------------------------------------
int timeArrIsInited(TimeArray * arr)
{
    if (arr->timeArr != NULL) return 1;
    else return 0;
}
