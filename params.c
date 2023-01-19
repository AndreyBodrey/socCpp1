//#include "startup.h"
#include "params.h"


int paramHanle(int argc, char *argv[], struct Status *state)
{
    if ( argc <= 1 )return 0;
    if ( argc > 3 )
    {
        printf( "too many options, use \"help\" \n" );
    }
    int count = 0;

    if ( argc == 3 )
    {
        count = atoi(argv[2]);
        if ( count > 0 ) state->workModeParam = count;
        else
        {
            printf("error in patametr %s \n", argv[2]);
            return -1;
        }
    }

    if ( ! strcmp(argv[1], "vid") )         state->workMode = mode_video;

    else if ( ! strcmp(argv[1], "ip") )      state->workMode = mode_ip;

    else if (  ! strcmp(argv[1], "udp") )    state->workMode = mode_udp;

    else if ( ! strcmp(argv[1], "eth") )     state->workMode = mode_ethernet;

    else if ( ! strcmp(argv[1], "help") )    { printHelp(); return -1; }

    else
    {
        printf("error in parametr %s \n", argv[1]);
        printHelp();
        return -1;
    }
    return 1;
}
//----------------------------------------------------------------------------------------

void printHelp(void)
{
    char * helpStr0 = "\n\tfirst option\n";
    char helpStr1[] = "\t\'vid\'\t\tfor save data as vidoe stream\n \'ip\'\t\tfor save data start from ip header\n\t\'udp\'\t\tfor save from udp headers\n\t\'eth\'\t\t for save from ethernet header\n";
    char helpStr2[] = "\n\tsecond option\n";
    char helpStr3[] = "if 1th option is \'vid\', number of minutes to save video\n";
    char helpStr4[] = "if 1th option \'ip\', \'udp\', \'eth\', number of packages to save data\n";
    char defaultStr[] = "\ndefault no options, mode \'vid\' time 10 min\n";

    printf("%s", helpStr0);
    printf("%s", helpStr1);
    printf("%s", helpStr2);
    printf("%s", helpStr3);
    printf("%s", helpStr4);
    printf("%s", defaultStr);

}
