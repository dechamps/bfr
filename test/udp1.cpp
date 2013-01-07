//
//  UDP Router Sample
//

#include <stdio.h>

#include "BACnet.h"
#include "BACnetTask.h"
#include "BACnetPort.h"
#include "BACnetUDP.h"
#include "BACnetBIPSimple.h"
#include "BACnetRouter.h"

//

bool gBFRRunning;

int main(int argc, char* argv[], char* envp[])
{
    struct timeval      delta
    ;
    BACnetUDPPtr        udp1, udp2
    ;
    BACnetBIPSimplePtr  bip1, bip2
    ;
    BACnetRouterPtr rp
    ;

    try {
        // network 1
        udp1 = new BACnetUDP("192.168.0.101/24");
        bip1 = new BACnetBIPSimple();
        Bind(bip1, udp1);

        // network 2
        udp2 = new BACnetUDP("192.168.1.101/24");
        bip2 = new BACnetBIPSimple();
        Bind(bip2, udp2);

        // build a router
        rp = new BACnetRouter();
        rp->dynamicRouting = true;

        // bind to the networks
        rp->BindToEndpoint(bip1, 1);
        rp->BindToEndpoint(bip2, 2);

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
