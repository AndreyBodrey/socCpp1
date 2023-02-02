//#include "startup.h"
#include "params.h"
#include "igmp.h"
#include "linux/igmp.h"

int paramHanle(int argc, thrd_data data, struct Status *state)
{
    if ( argc > 3 )
    {
        printf( "too many options, use \"help\" \n" );
    }
    int count = 0;

    if ( argc == 3 )
    {
        count = atoi(data->count);
        if ( count > 0 ) state->workModeParam = count;
        else
        {
            printf("error in patametr %s \n", argv[2]);
            return -1;
        }
    }

    if ( ! strcmp(argv[1], "vid") )         state->workMode = mode_video;

    else if ( ! strcmp(argv[1], "pack") )     state->workMode = mode_ethernet;

    else if ( ! strcmp(argv[1], "setup") )  return 100;

    else if ( ! strcmp(argv[1], "help") )    { printHelp(); return -1; }

    else if ( ! strcmp(argv[1], "l") )
    {
		igmpSend(IGMP_V2_LEAVE_GROUP, state);
		igmpSend(IGMP_V2_LEAVE_GROUP, state);
		exit(0);
	}

    else
    {
        printf("error in parametr %s \n", argv[1]);
        printHelp();
        return -1;
    }
    return 1;
}
//--------- -------------------------------------------------------------------------------

void printHelp(void)
{
    char * helpStr0 = "\n\tfirst option\n";
    char helpStr1[] = "\t\'vid\'\t\tfor save data as video stream\n\t\'pack\'\t\t for save from ethernet header use pcap format\n";
    char helpStr2[] = "\n\tsecond option\n";
    char helpStr3[] = "if 1th option is \'vid\', number of minutes to save video\n";
    char helpStr4[] = "if 1th option \'pack \', number of packages to save data\n";
    char defaultStr[] = "\ndefault no options, mode \'vid\' time 10 min\n";

    printf("%s", helpStr0);
    printf("%s", helpStr1);
    printf("%s", helpStr2);
    printf("%s", helpStr3);
    printf("%s", helpStr4);
    printf("%s", defaultStr);

}
