

#ifndef TIMEARRAY_H_INCLUDED
#define TIMEARRAY_H_INCLUDED

typedef struct
{
    int timeArrCount;
    int timeArrPointer;
    unsigned int *timeArr;
    int timeArrMax;
}TimeArray;

int timeArrInit(TimeArray * arr, int count); //return 0 if error else 1
void timeArrDeinit(TimeArray * arr);
void timeArrInsert(TimeArray * arr, unsigned int tim);
int timeArrIsInited(TimeArray * arr);
unsigned int timeArrGetAverge(TimeArray * arr);

#endif // TIMEARRAY_H_INCLUDED
