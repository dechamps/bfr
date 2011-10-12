//
//  BACnetBTR
//

#include <stdlib.h>
#include <string.h>

#include "BACnetBTR.h"

//
//  BACnetBTR::BACnetBTR
//

BACnetBTR::BACnetBTR( void )
    : btrPeer(0), btrPeerLen(0)
{
}

//
//	BACnetBTR::~BACnetBTR
//

BACnetBTR::~BACnetBTR( void )
{
    if (btrPeer)
        delete[] btrPeer;
}

//
//  BACnetBTR::Indication
//
//  This is called when the router has a message to send out.  This function 
//  forwards directed messages to the peer BTR.  When a broadcast is being 
//  sent out, it is sent as a series of directed messages to the peers.
//

void BACnetBTR::Indication( const BACnetPDU &pdu )
{
    int         i
    ;
    BACnetPDU   newpdu( pdu )
    ;

    switch (pdu.pduDestination.addrType) {
        case localStationAddr:
            // make sure the destination is one of our peers
            for (i = 0; i < btrPeerLen; i++)
                if (pdu.pduDestination == btrPeer[i]) {
                    Request( pdu );
                    break;
                }
            break;

        case localBroadcastAddr:
            // send a copy to every peer
            for (i = 0; i < btrPeerLen; i++) {
                newpdu.pduDestination = btrPeer[i];
                Request( newpdu );
            }
            break;

        default:
            throw_1(5001); // should never get any other kind of address
    }
}

//
//  BACnetBTR::Confirmation
//
//  This is called when the endpoint has received a message.  It should 
//  simply be passed up to the router for processing.  Make sure it is 
//  from one of our peers.
//

void BACnetBTR::Confirmation( const BACnetPDU &pdu )
{
    // check the version
    if (pdu.pduData[0] != 0x01)
        return;

    // find a peer
    for (int i = 0; i < btrPeerLen; i++)
        if (pdu.pduSource == btrPeer[i]) {
            Response( pdu );
            break;
        }
}

//
//  BACnetBTR::AddPeer
//

void BACnetBTR::AddPeer( const BACnetAddress &ipAddr )
{
    BACnetAddress   *newPeerList
    ;

    newPeerList = new BACnetAddress[ btrPeerLen + 1 ];
    if (btrPeer)
        memcpy( newPeerList, btrPeer, (size_t)(btrPeerLen * sizeof(BACnetAddress)) );
    newPeerList[btrPeerLen++] = ipAddr;

    if (btrPeer)
        delete[] btrPeer;
    btrPeer = newPeerList;
}

//
//  BACnetBTR::DeletePeer
//

void BACnetBTR::DeletePeer( const BACnetAddress &ipAddr )
{
    int             i
    ;
    BACnetAddress   *newPeerList
    ;

    // find it in the list
    for (i = 0; i < btrPeerLen; i++)
        if (btrPeer[i] == ipAddr)
            break;
    if (i >= btrPeerLen)
        return;

    if (btrPeerLen == 1) {
        delete[] btrPeer;
        btrPeer = 0;
        btrPeerLen = 0;
    } else {
        // build a new list
        newPeerList = new BACnetAddress[ btrPeerLen - 1 ];

        if (i > 0)
            memcpy( newPeerList, btrPeer, (size_t)(i * sizeof(BACnetAddress)) );
        if (i <= (btrPeerLen - 1))
            memcpy( newPeerList+i, btrPeer + i + 1, (size_t)((btrPeerLen - i - 1) * sizeof(BACnetAddress)) );

        delete[] btrPeer;
        btrPeer = newPeerList;
        btrPeerLen -= 1;
    }
}

//
//  BACnetBTR::AddPeerNet
//
//  This function mocks up an I-Am-Router-To-Network message as if it came from an IP 
//  to tell the router to add this station as a path to the given network.  Note that the 
//  BTR object must already be bound to a router for this to do anything.
//

void BACnetBTR::AddPeerNet( const BACnetAddress &addr, int net )
{
    // mock up an I-Am-Router-To-Network message
    BACnetOctet
        msg[] = { 0x01, 0x80, (unsigned char)IAmRouterToNetwork
                , (net >> 8) & 0xFF, (net & 0xFF)
                };

    // build a packet
    BACnetPDU   pdu
    ;

    // set the source
    pdu.pduSource = addr;
    pdu.SetReference( msg, 5 );

    // send it upstream to the router
    Response( pdu );
}
