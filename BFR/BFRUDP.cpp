//
//  BFRUDP
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>

#include <net/if.h>
#include <net/if_arp.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <linux/if_packet.h>
#include <linux/if_ether.h>

#include "BFRUDP.h"

//
//  BFRUDPFactory
//

BFRUDPFactory gBFRUDPFactory;

//
//  BFRUDPFactory::StartElement
//

voidPtr BFRUDPFactory::StartElement( const char *name, const MinML::AttributeList& attrs )
{
    BACnetUDPPtr        udpp = new BACnetUDP()
    ;
    const char      *s
    ;
    unsigned long   mask
    ;

    // find the device
    s = SubstituteArgs(attrs["address"]);
    if (!s || !*s)
        throw_1(12001);     // address required

    // decode the address
    BACnetAddress::StringToHostPort( s, &udpp->ipHost, &mask, &udpp->ipPort );

    // register as a server
    s = SubstituteArgs(attrs["server"]);
    if (!s || !*s)
        throw_1(12002);     // server required
    gBFRRegistration.RegisterServer( s, udpp );

    // initialize it
    udpp->Init();

    // build a local address
    udpp->portLocalAddr.Pack( udpp->ipHost, udpp->ipPort );

    // also need the broadcast address
    udpp->portBroadcastAddr.Pack( (udpp->ipHost | ~mask), udpp->ipPort );

    // may need to create a broadcast listener
    if (mask != 0xFFFFFFFF) {
        BACnetUDPBroadcastListenerPtr  blp = new BACnetUDPBroadcastListener(udpp)
        ;

        // set it up and start it
        blp->ipHost = (udpp->ipHost | ~mask);
        blp->ipPort = udpp->ipPort;
        blp->Init();
    }

    // return the endpoint
    return udpp;
}

