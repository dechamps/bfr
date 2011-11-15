//
//  BACnetBTR
//

#if _DEBUG_BTR
#include <stdio.h>
#endif

#include <stdlib.h>
#include <string.h>

#include "BACnetBTR.h"

//
//  BACnetBTR::BACnetBTR
//

BACnetBTR::BACnetBTR( void )
    : btrPeer(0), btrPeerLen(0)
{
#if _DEBUG_BTR
    printf("BACnetBTR::BACnetBTR\n");
#endif
}

//
//	BACnetBTR::~BACnetBTR
//

BACnetBTR::~BACnetBTR( void )
{
#if _DEBUG_BTR
    printf("BACnetBTR::~BACnetBTR\n");
#endif
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

#if _DEBUG_BTR
    printf("BACnetBTR::Indication\n");
#endif

    switch (pdu.pduDestination.addrType) {
        case localStationAddr:
#if _DEBUG_BTR
            printf("    - local station\n");
#endif

            // make sure the destination is one of our peers
            for (i = 0; i < btrPeerLen; i++)
                if (pdu.pduDestination == btrPeer[i]) {
#if _DEBUG_BTR
                    printf("    - send to peer: %s\n", btrPeer[i].ToString());
#endif
                    Request( pdu );
                    break;
                }
            break;

        case localBroadcastAddr:
#if _DEBUG_BTR
            printf("    - local broadcast\n");
#endif

            // send a copy to every peer
            for (i = 0; i < btrPeerLen; i++) {
#if _DEBUG_BTR
                printf("    - send to peer: %s\n", btrPeer[i].ToString());
#endif

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
#if _DEBUG_BTR
    printf("BACnetBTR::Confirmation\n");
#endif

    // check the version
    if (pdu.pduData[0] != 0x01) {
#if _DEBUG_BTR
        printf("    - wrong version, dropped\n");
#endif
        return;
    }

    // find a peer
    for (int i = 0; i < btrPeerLen; i++)
        if (pdu.pduSource == btrPeer[i]) {
#if _DEBUG_BTR
            printf("    - matched a peer, deliver upstream\n");
#endif
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

#if _DEBUG_BTR
    printf("BACnetBTR::AddPeer %s\n", ipAddr.ToString());
#endif

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

#if _DEBUG_BTR
    printf("BACnetBTR::DeletePeer %s\n", ipAddr.ToString());
#endif

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

