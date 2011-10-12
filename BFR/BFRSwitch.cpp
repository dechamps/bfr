//
//  BFRSwitch
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "BFRSwitch.h"

#include "BFRRegistration.h"

//
//  BFRSwitchPortAddr
//

struct BFRSwitchPortAddr : BACnetAddrPoolElem {
    BFRSwitchPortPtr    addrPort;
    };

typedef BFRSwitchPortAddr *BFRSwitchPortAddrPtr;

//
//  BFRSwitch::BFRSwitch
//

BFRSwitch::BFRSwitch( void )
    : switchPorts(0), switchPool(), switchFilter()
{
}

//
//  BFRSwitch::~BFRSwitch
//

BFRSwitch::~BFRSwitch( void )
{
    // unbind all the ports
    BFRSwitchPortPtr    pp, np
    ;

    for (pp = switchPorts; pp; pp = np) {
        np = pp->portNext;

        pp->portSwitch = 0;
        pp->portNext = 0;
    }

    // delete all the addresses
    BACnetAddressPoolIter   iter(switchPool)
    ;
    BFRSwitchPortAddrPtr    ap
    ;

    // iterate through the addresses
    while ((ap = (BFRSwitchPortAddrPtr)*iter) != 0) {
        delete ap;
        iter.Next();
    }
}

//
//  BFRSwitch::AddPort
//

void BFRSwitch::AddPort( BFRSwitchPortPtr spp )
{
    spp->portSwitch = this;
    spp->portNext = switchPorts;
    switchPorts = spp;
}

//
//  BFRSwitch::RemovePort
//

void BFRSwitch::RemovePort( BFRSwitchPortPtr spp )
{
    BFRSwitchPortPtr    *ppp
    ;

    // remove the port
    for (ppp = &switchPorts; *ppp; ppp = &(*ppp)->portNext)
        if (*ppp == spp) {
            *ppp = spp->portNext;
            spp->portSwitch = 0;
            spp->portNext = 0;
            break;
        }

    // remove any addresses associated with the port
    BACnetAddressPoolIter   iter(switchPool)
    ;
    BFRSwitchPortAddrPtr    ap
    ;

    // iterate through the addresses
    while ((ap = (BFRSwitchPortAddrPtr)*iter) != 0) {
        // remove the ones associated with the port being removed
        if (ap->addrPort == spp) {
            switchPool.RemoveElem( ap );
            delete ap;
        }

        iter.Next();
    }
}

//
//  BFRSwitch::ProcessPacket
//

void BFRSwitch::ProcessPacket( BFRSwitchPortPtr spp, const BACnetPDU &pdu )
{
    BFRSwitchPortPtr        pp
    ;
    BFRSwitchPortAddrPtr    ap
    ;

    // test the packet, if it fails then toss it
    if (!switchFilter.Test(pdu))
        return;

    // locate the source address
    ap = (BFRSwitchPortAddrPtr)switchPool[ pdu.pduSource ];

    // if it doesn't exist, create an entry for it
    if (!ap) {
        ap = new BFRSwitchPortAddr();
        ap->elemAddress = pdu.pduSource;
        ap->addrPort = spp;

        switchPool.AddElem( ap );
    } else
    if (ap->addrPort != spp) {
#if 1
        // should a station be allowed to hop ports?
        return;
#else
        // accept it for now
        ap->addrPort = spp;
#endif
    }

    ap = 0;

    // check for a broadcast
    if (pdu.pduDestination.addrType != localBroadcastAddr) {
        // locate the destination address
        ap = (BFRSwitchPortAddrPtr)switchPool[ pdu.pduDestination ];

        // if it exists, send the packet, otherwise broadcast it
        if (ap && (ap->addrPort != spp))
            ap->addrPort->Request( pdu );
    }

    // check to broadcast it
    if (!ap) {
        for (pp = switchPorts; pp; pp = pp->portNext)
            if (pp != spp)
                pp->Request( pdu );
    }
}

//
//  BFRSwitchFactory
//

BFRFactoryChild gBFRSwitchFactoryChildren[] =
    { { "Port", &gBFRSwitchPortFactory }
    , { "Accept", &gBFRFilterElemFactory }
    , { "Reject", &gBFRFilterElemFactory }
    , { 0, 0 }
    };

BFRSwitchFactory gBFRSwitchFactory;

//
//  BFRSwitchFactory::BFRSwitchFactory
//

BFRSwitchFactory::BFRSwitchFactory( void )
    : BFRFactory(gBFRSwitchFactoryChildren)
{
}

//
//  BFRSwitchFactory::StartElement
//

voidPtr BFRSwitchFactory::StartElement( const char *name, const MinML::AttributeList& attrs )
{
    // return a new switch
    return new BFRSwitch();
}

//
//  BFRSwitchFactory::ChildElement
//

void BFRSwitchFactory::ChildElement( voidPtr ep, int id, voidPtr cp )
{
    BFRSwitchPtr    sp = (BFRSwitchPtr)ep
    ;

    switch (id) {
        case 0:
            // child elements are ports
            sp->AddPort( (BFRSwitchPortPtr)cp );
            break;
        case 1:
        case 2:
            // child elements are filters
            sp->switchFilter.AddElem( (BFRFilterElemPtr)cp );
    }
}

//
//  BFRSwitchPort::BFRSwitchPort
//

BFRSwitchPort::BFRSwitchPort( void )
    : portNext(0), portSwitch(0)
{
}

//
//  BFRSwitchPort::~BFRSwitchPort
//

BFRSwitchPort::~BFRSwitchPort( void )
{
    // if it's bound, tell the switch it is going away
    if (portSwitch)
        portSwitch->RemovePort( this );
}

//
//  BFRSwitchPort::Confirmation
//

void BFRSwitchPort::Confirmation( const BACnetPDU &pdu )
{
    // verify binding
    if (!portSwitch)
        throw_1(10001);     // misconfigured

    // send packet to the switch
    portSwitch->ProcessPacket( this, pdu );
}

//
//  BFRSwitchPortFactory
//

BFRSwitchPortFactory gBFRSwitchPortFactory;

//
//  BFRSwitchPortFactory::StartElement
//

voidPtr BFRSwitchPortFactory::StartElement( const char *name, const MinML::AttributeList& attrs )
{
    BFRSwitchPortPtr    pp
    ;

    // create a new port
    pp = new BFRSwitchPort();

    // register it
    gBFRRegistration.RegisterClient( SubstituteArgs(attrs["client"]), pp );

    // it will be added to the correct switch
    return pp;
}
