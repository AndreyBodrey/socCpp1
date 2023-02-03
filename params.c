//#include "startup.h"
#include "params.h"
#include "igmp.h"
#include "linux/igmp.h"

int paramHanle(int argc, thrd_data *data, struct Status *state)
{

    state->workModeParam = atoi(data->countd);

    if ( ! strcmp(data->command, "vid") ) state->workMode = mode_video;

    else if ( ! strcmp(data->command, "pack") ) state->workMode = mode_ethernet;

    else if ( ! strcmp(data->command, "setup") ) return 100;

    else if ( ! strcmp(data->command, "help") ) { printHelp(); return 0; }

    else if ( ! strcmp(data->command, "work") ) state->workMode = mode_infinity;

    else if ( ! strcmp(data->command, "l") )
    {
		igmpSend(IGMP_V2_LEAVE_GROUP, state);
		igmpSend(IGMP_V2_LEAVE_GROUP, state);
		return 0;
	}
    else  return -1;
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
