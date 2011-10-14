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
//  BFRUDP::BFRUDP
//

BFRUDP::BFRUDP( void )
    : ipHost(0), ipPort(0)
{
}

BFRUDP::BFRUDP( const char *s )
    : ipHost(0), ipPort(0)
{
    unsigned long   mask
    ;

    // decode the address
    BACnetAddress::StringToHostPort( s, &ipHost, &mask, &ipPort );

    // initialize ourselves
    Init();

    // build a local address
    portLocalAddr.Pack( ipHost, ipPort );

    // also need the broadcast address
    portBroadcastAddr.Pack( (ipHost | ~mask), ipPort );

    // may need to create a broadcast listener
    if (mask != 0xFFFFFFFF) {
        BFRUDPBroadcastListenerPtr  blp = new BFRUDPBroadcastListener(this)
        ;

        // set it up and start it
        blp->ipHost = (ipHost | ~mask);
        blp->ipPort = ipPort;
        blp->Init();
    }
}

BFRUDP::BFRUDP( unsigned long host, unsigned short port )
    : ipHost(host), ipPort(port)
{
    Init();
}

//
//  BFRUDP::~BFRUDP
//

BFRUDP::~BFRUDP( void )
{
}

//
//  BFRUDP::Init
//

void BFRUDP::Init( void )
{
    int                 on = 1
    ;
    struct sockaddr_in  my_addr
    ;

    // create a socket
    if ( (portSocket = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        portStatus = 1;
        perror("socket");
    }

    // bind to it
    bzero((char *) &my_addr, sizeof(my_addr));
    my_addr.sin_family = AF_INET;
    my_addr.sin_addr.s_addr = htonl(ipHost);
    my_addr.sin_port = htons(ipPort);
    if ( (bind(portSocket, (struct sockaddr *) &my_addr, sizeof(my_addr)) < 0) ) {
        portStatus = 2;
        perror("bind");
    }

    // allow it to broadcast
    if (setsockopt(portSocket, SOL_SOCKET, SO_BROADCAST, (char *)&on, sizeof(on)) < 0) {
        portStatus = 3;
        perror("setsockopt");
    }

    // everything OK for now
    portStatus = 0;
}

//
//  BFRUDP::Read
//

void BFRUDP::Read( void )
{
    int                     stat, len
    ;
    struct sockaddr_in      client_addr
    ;
    unsigned long           host
    ;
    unsigned short          port
    ;
    socklen_t               addrlen = sizeof(client_addr)
    ;
    static BACnetOctet      buff[65536]
    ;
    BACnetPDUListElementPtr lep
    ;

    // read the stuff in from the socket
    len = recvfrom( portSocket, buff, sizeof(buff), 0, (struct sockaddr *)&client_addr, &addrlen );
    if (len < 0) {
        perror("recvfrom");
        return;
    }

    // extract the client host and port
    host = ntohl(client_addr.sin_addr.s_addr);
    port = ntohs(client_addr.sin_port);
    
    // get outselves a buffer
    lep = gFreePDUList.Read();
    if (!lep) {
        lep = new BACnetPDUListElement();

        // could run out of RAM, packet dropped
        if (!lep) return;
    }

    // save the source and destination
    lep->listPDU.pduDestination.LocalStation( ipHost, ipPort );
    lep->listPDU.pduSource.LocalStation( host, port );

    // copy the data
    lep->listPDU.Copy( buff, len );

    // this port gets it
    lep->listPort = this;

    // queue it for processing
    gProcessPDUList.Write( lep );
}

//
//  BFRUDP::Write
//

void BFRUDP::Write( void )
{
    int                     stat, len
    ;
    unsigned long           host
    ;
    unsigned short          port
    ;
    struct sockaddr_in      server_addr
    ;
    BACnetPDUListElementPtr lep
    ;

    // get outselves a packet
    lep = portWriteList.Read();
    if (!lep)
        return;

    // unpack the destination
    if (lep->listPDU.pduDestination.addrType == localStationAddr)
        lep->listPDU.pduDestination.Unpack( &host, &port );
    else
    if (lep->listPDU.pduDestination.addrType == localBroadcastAddr)
        portBroadcastAddr.Unpack( &host, &port );
    else {
        // ### should signal some kind of error
        return;
    }

    // set up the destination address
    bzero((char *) &server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(host);
    server_addr.sin_port = htons(port);

    // send the message
    stat = sendto(portSocket,lep->listPDU.pduData,lep->listPDU.pduLen,0,(struct sockaddr *) &server_addr, sizeof(server_addr));
    if (stat < 0) {
        portStatus = 6;
        perror ("sendto");
    }

    // save it on the free list
    gFreePDUList.Write( lep );
}

//
//  BFRUDPBroadcastListener::BFRUDPBroadcastListener
//

BFRUDPBroadcastListener::BFRUDPBroadcastListener( BFRUDPPtr peer )
    : ipPeer(peer), ipHost(0), ipPort(0)
{
}

//
//  BFRUDPBroadcastListener::~BFRUDPBroadcastListener
//

BFRUDPBroadcastListener::~BFRUDPBroadcastListener( void )
{
}

//
//  BFRUDPBroadcastListener::Init
//

void BFRUDPBroadcastListener::Init( void )
{
    int                 on = 1
    ;
    struct sockaddr_in  my_addr
    ;

    // create a socket
    if ( (portSocket = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        portStatus = 1;
        perror("socket");
    }

    // bind to it
    bzero((char *) &my_addr, sizeof(my_addr));
    my_addr.sin_family = AF_INET;
    my_addr.sin_addr.s_addr = htonl(ipHost);
    my_addr.sin_port = htons(ipPort);
    if ( (bind(portSocket, (struct sockaddr *) &my_addr, sizeof(my_addr)) < 0) ) {
        portStatus = 2;
        perror("bind");
    }

    // everything OK for now
    portStatus = 0;
}

//
//  BFRUDPBroadcastListener::Read
//

void BFRUDPBroadcastListener::Read( void )
{
    int                     stat, len
    ;
    struct sockaddr_in      client_addr
    ;
    unsigned long           host
    ;
    unsigned short          port
    ;
    socklen_t               addrlen = sizeof(client_addr)
    ;
    static BACnetOctet      buff[65536]
    ;
    BACnetPDUListElementPtr lep
    ;

    // read the stuff in from the socket
    len = recvfrom( portSocket, buff, sizeof(buff), 0, (struct sockaddr *)&client_addr, &addrlen );
    if (len < 0) {
        perror("recvfrom");
        return;
    }

    // extract the client host and port
    host = ntohl(client_addr.sin_addr.s_addr);
    port = ntohs(client_addr.sin_port);
    
    // skip packets from ourselves
    if ((host == ipPeer->ipHost) && (port == ipPeer->ipPort))
        return;
    
    // get outselves a buffer
    lep = gFreePDUList.Read();
    if (!lep) {
        lep = new BACnetPDUListElement();

        // could run out of RAM, packet dropped
        if (!lep) return;
    }

    // save the source and destination
    lep->listPDU.pduDestination.LocalBroadcast();
    lep->listPDU.pduSource.LocalStation( host, port );

    // copy the data
    lep->listPDU.Copy( buff, len );

    // the peer port gets it, we're not bound
    lep->listPort = ipPeer;

    // queue it for processing
    gProcessPDUList.Write( lep );
}

//
//  BFRUDPBroadcastListener::Write
//

void BFRUDPBroadcastListener::Write( void )
{
}

//
//  BFRUDPFactory
//

BFRUDPFactory gBFRUDPFactory;

//
//  BFRUDPFactory::StartElement
//

voidPtr BFRUDPFactory::StartElement( const char *name, const MinML::AttributeList& attrs )
{
    BFRUDPPtr        udpp = new BFRUDP()
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
        BFRUDPBroadcastListenerPtr  blp = new BFRUDPBroadcastListener(udpp)
        ;

        // set it up and start it
        blp->ipHost = (udpp->ipHost | ~mask);
        blp->ipPort = udpp->ipPort;
        blp->Init();
    }

    // return the endpoint
    return udpp;
}
