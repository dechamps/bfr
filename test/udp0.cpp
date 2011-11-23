//
//  UDP Sample
//

#include <stdio.h>

#include "BACnet.h"
#include "BACnetTask.h"
#include "BACnetPort.h"
#include "BACnetUDP.h"
#include "BACnetConsole.h"

//

bool gBFRRunning;

int main(int argc, char* argv[], char* envp[])
{
    struct timeval      delta
    ;
    BACnetUDPPtr        udpp
    ;
    BACnetConsolePtr    cp
    ;

    try {
        // network port
        udpp = new BACnetUDP("128.253.109.40/24");

        // debugger
        cp = new BACnetConsole();

        // bind them together
        Bind(cp, udpp);

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
            , err->errError, err->GetDescription()
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
