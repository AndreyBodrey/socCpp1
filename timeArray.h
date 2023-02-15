

#ifndef TIMEARRAY_H_INCLUDED
#define TIMEARRAY_H_INCLUDED

int timeArrInit(int count); //return 0 if error else 1
void timeArrDeinit();
void timeArrInsert(unsigned int tim);
int timeArrIsInited();
unsigned int timeArrGetAverge();

#endif // TIMEARRAY_H_INCLUDED
