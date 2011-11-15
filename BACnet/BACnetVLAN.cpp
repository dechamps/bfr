//
//  BACnetVLAN
//

#if _DEBUG_VLAN
#include <stdio.h>
#endif

#include "BACnetVLAN.h"

//
//  BACnetVLANNode::BACnetVLANNode
//

BACnetVLANNode::BACnetVLANNode( void )
    : nodeAddress(), nodeLAN(0), nodePromiscuous(false), nodeNext(0)
{
}

//
//  BACnetVLANNode::~BACnetVLANNode
//

BACnetVLANNode::~BACnetVLANNode( void )
{
    // if bound to a LAN, remove it
    if (nodeLAN)
        nodeLAN->RemoveNode( this );
}

//
//  BACnetVLANNode::Indication
//

void BACnetVLANNode::Indication( const BACnetPDU &pdu )
{
    // check to see that we are bound to a VLAN
    if (!nodeLAN)
        throw_1(6001);      // unbound

    // put in our source address
    if (pdu.pduSource.addrType == nullAddr) {
        BACnetPDU newpdu(pdu)
        ;

        newpdu.pduSource = nodeAddress;
        nodeLAN->ProcessMessage( this, newpdu );
    } else {
        if (!nodePromiscuous && (!(pdu.pduSource == nodeAddress)))
            throw_1(6002);      // spoofing attempt

        // pass to the VLAN
        nodeLAN->ProcessMessage( this, pdu );
    }
}

//
//  BACnetVLAN::BACnetVLAN
//

BACnetVLAN::BACnetVLAN( void )
    : vlanNodeList(0)
{
}

//
//  BACnetVLAN::~BACnetVLAN
//

BACnetVLAN::~BACnetVLAN( void )
{
    BACnetVLANNodePtr   np, next
    ;

    // remove the nodes from the VLAN
    for (np = vlanNodeList; np; np = next ) {
        next = np->nodeNext;

        np->nodeLAN = 0;
        np->nodeNext = 0;
    }
}

//
//  BACnetVLAN::AddNode
//

void BACnetVLAN::AddNode( BACnetVLANNodePtr np )
{
    // link it in
    np->nodeNext = vlanNodeList;
    vlanNodeList = np;

    // bind it
    np->nodeLAN = this;
}

//
//  BACnetVLAN::RemoveNode
//

void BACnetVLAN::RemoveNode( BACnetVLANNodePtr np )
{
    BACnetVLANNodePtr   *npp
    ;

    // remove the nodes from the VLAN
    for (npp = &vlanNodeList; *npp; npp = &(*npp)->nodeNext )
        if (*npp == np) {
            // link over it
            (*npp)->nodeNext = np->nodeNext;

            // unbind
            np->nodeLAN = 0;
            np->nodeNext = 0;
            break;
        }
}

//
//  BACnetVLAN::ProcessMessage
//

void BACnetVLAN::ProcessMessage( BACnetVLANNodePtr np, const BACnetPDU &pdu )
{
    BACnetVLANNodePtr   destp
    ;

#if _DEBUG_VLAN
    printf( "BACnetVLAN::ProcessMessage" );
    printf( " %s",  pdu.pduSource.ToString() );
    printf( " --> %s\n",  pdu.pduDestination.ToString() );
#endif

    // look for a match
    if (pdu.pduDestination.addrType == localBroadcastAddr) {
        // give it to everyone
        for (destp = vlanNodeList; destp; destp = destp->nodeNext)
            if (destp->nodePromiscuous || (destp != np))
                destp->Response( pdu );
    } else {
        // find the first match
        for (destp = vlanNodeList; destp; destp = destp->nodeNext)
            if (destp->nodePromiscuous || ((destp != np) && (pdu.pduDestination == destp->nodeAddress)))
                destp->Response( pdu );
    }
}
