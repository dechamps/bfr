//
//  BFR
//
//  $CVSHeader$
//

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#include "BACnet.h"
#include "BACnetVLAN.h"
#include "BACnetMLAN.h"
#include "BACnetAddressPool.h"
#include "BACnetTask.h"
#include "BACnetBTR.h"
#include "BACnetBIPSimple.h"
#include "BACnetBIPForeign.h"
#include "BACnetBBMD.h"
#include "BACnetRouter.h"

#include "BFRParser.h"
#include "BFRRegistration.h"
#include "BFRSwitch.h"
#include "BFRFilter.h"
#include "BFRDebug.h"
#include "BFREcho.h"
#include "BFRConsole.h"

#include "BFREthernet.h"
#include "BFRUDP.h"

static char *_revision = "$Revision: 1.10 $";

//
//  BFRFactory
//

BFRFactoryChild gBFRMasterFactoryChildren[] =
    { { "Switch", &gBFRSwitchFactory }
    , { "Filter", &gBFRFilterFactory }
    , { "VLAN", &gBACnetVLANFactory }
    , { "MLAN", &gBACnetMLANFactory }
    , { "Debug", &gBFRDebugFactory }
    , { "Ethernet", &gBFREthernetFactory }
    , { "UDP", &gBFRUDPFactory }
    , { "BTR", &gBACnetBTRFactory }
    , { "BIP", &gBACnetBIPSimpleFactory }
    , { "Foreign", &gBACnetBIPForeignFactory }
    , { "BBMD", &gBACnetBBMDFactory }
    , { "Echo", &gBFREchoFactory }
    , { "Console", &gBFRConsoleFactory }
    , { "Router", &gBACnetRouterFactory }
    , { 0, 0 }
    };

BFRFactory gBFRMasterFactory(gBFRMasterFactoryChildren);

BFRParser gBFRMasterParser( "BFR", &gBFRMasterFactory );

//

void ConsoleUsage( void );

void Useage(void)
{
    printf( "Usage: BFR [-h] [-v] [-d] file [ parm... ]\n" );
    printf( "\n" );
    printf( "    -h      help\n" );
    printf( "    -v      version information\n" );
    printf( "    -d      list program arguments and environment\n" );
    printf( "    file    settings file\n" );
    printf( "\n" );
}

//

bool gBFRRunning;

int main(int argc, char* argv[], char* envp[])
{
    int             i, j, len
    ;
    char            line[1024], *addr, *data
    ;
    struct timeval  delta
    ;

    try {
        // no arguments, print useage and exit
        if (argc <= 1) {
            Useage();
            return 0;
        }
        
        // do the arguments
        for (i = 1; i < argc; i++) {
            if (strcmp(argv[i],"-h") == 0) {
                Useage();
            } else
            if (strcmp(argv[i],"-d") == 0) {
                for (j = 0; j < argc; j++)
                    printf( "argv[%d] = \"%s\"\n", j, argv[j] );
                for (j = 0; envp[j]; j++)
                    printf( "envp[%d] = \"%s\"\n", j, envp[j] );
            } else
            if (strcmp(argv[i],"-v") == 0) {
                printf( "BFR Version X.X.X\n" );
            } else
            if (argv[i][0] == '-') {
                printf( "Unknown option: %s\n", argv[i] );
            } else
                break;
        }
        
        // if there's no config file, exit
        if (i >= argc)
            return 0;
            
        // tell the parser the substitution environment
        SubstitutionEnvironment( argv + i, envp );

        // parse the file
        gBFRMasterParser.Parse( argv[i] );

        gBFRRunning = true;
        while (gBFRRunning) {
            delta.tv_sec = 10;
            delta.tv_usec = 0;

            ProcessTasks( delta );
            ProcessPortIO( delta );
        }
    }
    catch (BACnetError *err) {
        // more detailed error message
        fprintf( stderr, "Error %d: %s\nFile %s, line %d\n"
            , err->errError, err->Description()
            , err->errFile, err->errLine
            );

        // done with the error
        delete err;

        // failed
        return 1;
    }
    catch (int err) {
        // not so detailed error message
        fprintf( stderr, "Error %d\n", err );

        return err;
    }

    // success
    return 0;
}
