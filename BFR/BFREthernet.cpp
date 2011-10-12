//
//  BFREthernet
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

#include "BFREthernet.h"

//
//  BFREthernet::BFREthernet
//

BFREthernet::BFREthernet( void )
    : ethDevice(0), ethPromiscuous(false)
{
}

//
//	BFREthernet::~BFREthernet
//

BFREthernet::~BFREthernet( void )
{
    if (ethDevice)
        delete[] ethDevice;
}

//
//  BFREthernet::Init
//

void BFREthernet::Init( void )
{
    int                 ifindex
    ;
    struct ifreq        ifr
    ;
    struct sockaddr_ll  my_addr
    ;
    
    // create a socket
    if ( (portSocket = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_ALL))) < 0) {
        portStatus = 1;
        perror("socket");
    }

    // get the interface index
    strcpy( ifr.ifr_name, ethDevice );
    if (ioctl( portSocket, SIOCGIFINDEX, &ifr) < 0) {
        portStatus = 2;
        perror("SIOCGIFINDEX");
    }
    ifindex = ifr.ifr_ifindex;
    
    // bind to the interface
    bzero((char *) &my_addr, sizeof(my_addr));
    my_addr.sll_family = AF_PACKET;
    my_addr.sll_ifindex = ifr.ifr_ifindex;
    if ((bind(portSocket, (struct sockaddr *) &my_addr, sizeof(my_addr)) < 0) ) {
        portStatus = 3;
        perror("bind");
    }
    
    // get the MAC address
    if (ioctl(portSocket, SIOCGIFHWADDR, &ifr) < 0) {
        portStatus = 4;
        perror("SIOCGIFHWADDR");
    }
    portLocalAddr.LocalStation( (unsigned char *)ifr.ifr_hwaddr.sa_data, 6 );
    
    // also need the broadcast address
    portBroadcastAddr.LocalStation( (unsigned char *)"\xFF\xFF\xFF\xFF\xFF\xFF", 6 );
    
    // everything OK for now
    portStatus = 0;
}

//
//  BFREthernet::Read
//

void BFREthernet::Read( void )
{
    int                     stat, len
    ;
    static BACnetOctet      buff[ETH_FRAME_LEN], *pbuff = buff
    ;
    BACnetPDUListElementPtr lep
    ;
    struct ethhdr           *ep
    ;
    
    // read the stuff in from the socket
    stat = recv( portSocket, buff, ETH_FRAME_LEN, 0 );
    if (stat < 0) {
        portStatus = 5;
        perror("recv");
    }

    // point to the header
    ep = (struct ethhdr *)buff;

    // get the data length, which is not the same as the number 
    // of octets read since it may have been padded
    len = ntohs(ep->h_proto);
    
    // make sure we're dealing with 802.3 messages
    if (len > ETH_FRAME_LEN)
        return;
        
    // make sure we continue with BACnet messages
    stat = (buff[14] == 0x82) && (buff[15] == 0x82) && (buff[16] == 0x03);
    if (!stat)
        return;
    
    // only version 1
    if (buff[17] != 0x01)
        return;
        
    // check to see if it's for us
    if (!ethPromiscuous)
        if ((memcmp(portLocalAddr.addrAddr,ep->h_dest,6) != 0) && (memcmp(portBroadcastAddr.addrAddr,ep->h_dest,6) != 0))
            return;
    
    // get outselves a buffer
    lep = gFreePDUList.Read();
    if (!lep) {
        lep = new BACnetPDUListElement();
        
        // could run out of RAM, packet dropped
        if (!lep) return;
    }
    
    // save the source and destination
    lep->listPDU.pduDestination.LocalStation( (unsigned char *)ep->h_dest, 6 );
    lep->listPDU.pduSource.LocalStation( (unsigned char *)ep->h_source, 6 );
    
    // check for broadcast
    if (lep->listPDU.pduDestination == portBroadcastAddr)
        lep->listPDU.pduDestination.LocalBroadcast();
        
    // extract the priority
    lep->listPDU.pduNetworkPriority = (buff[18] & 0x03);
    lep->listPDU.pduExpectingReply = 0;
    
    // copy the rest of the data
    lep->listPDU.Copy( buff+17, len-3 );
    
    // this port gets it
    lep->listPort = this;
    
    // queue it for processing
    gProcessPDUList.Write( lep );
}

//
//  BFREthernet::Write
//

void BFREthernet::Write( void )
{
    int                     stat, len
    ;
    static BACnetOctet      buff[1500], *pbuff = buff
    ;
    BACnetPDUListElementPtr lep
    ;
    struct ethhdr           *ep
    ;
    
    // get outselves a packet
    lep = portWriteList.Read();
    if (!lep)
        return;

    // set the destination
    if (lep->listPDU.pduDestination.addrType == localStationAddr)
        memcpy( buff, lep->listPDU.pduDestination.addrAddr, 6 );
    else
    if (lep->listPDU.pduDestination.addrType == localBroadcastAddr)
        memcpy( buff, portBroadcastAddr.addrAddr, 6 );
    else {
        // ### should signal some kind of error
        return;
    }
    
    // set the source
    if (lep->listPDU.pduSource.addrType == nullAddr)
        memcpy( buff+6, portLocalAddr.addrAddr, 6 );
    else
    if (lep->listPDU.pduSource.addrType == localStationAddr)
        memcpy( buff+6, lep->listPDU.pduSource.addrAddr, 6 );
    else {
        // ### should signal some kind of error
        return;
    }
    
    // set correct length
    len = lep->listPDU.pduLen;
    buff[12] = (len + 3) >> 8;
    buff[13] = (len + 3) & 0xFF;

    // set the 802.3 header
    buff[14] = 0x82;         // BACnet packet type
    buff[15] = 0x82;
    buff[16] = 0x03;
    
    // copy the contents
    memcpy( buff+17, lep->listPDU.pduData, len );
    
    // update the length and pad as necessary
    len += 17;
    if (len < 60) {
        memset( buff+len, 0, 60 - len );
        len = 60;
    }
    
    /* send the message */
    stat = send( portSocket, buff, len, 0 );
    if (stat < 0) {
        portStatus = 6;
        perror ("send");
    }

    // save it on the free list
    gFreePDUList.Write( lep );
}

//
//  BFREthernetFactory
//

BFREthernetFactory gBFREthernetFactory;

//
//  BFREthernetFactory::StartElement
//

voidPtr BFREthernetFactory::StartElement( const char *name, const MinML::AttributeList& attrs )
{
    BFREthernetPtr  ethp = new BFREthernet()
    ;
    const char      *s
    ;

    // find the device
    if ((s = SubstituteArgs(attrs["device"])) != 0) {
        ethp->ethDevice = new char[strlen(s)+1];
        strcpy( ethp->ethDevice, s );
    }

    if ((s = SubstituteArgs(attrs["promiscuous"])) != 0)
        ethp->ethPromiscuous = (toupper(*s) == 'Y');

    // register as a server
    if ((s = SubstituteArgs(attrs["server"])) != 0)
        gBFRRegistration.RegisterServer( s, ethp );

    // initialize it
    ethp->Init();
    
    // return the btr
    return ethp;
}
