//
//  BFRBTR
//

#include <stdlib.h>
#include <string.h>

#include "BFRBTR.h"

#include "BFRRegistration.h"

//
//  BACnetBTRPeerFactory
//

class BACnetBTRPeerFactory : public BFRFactory {
    public:
        virtual voidPtr StartElement( const char *name, const MinML::AttributeList& attrs )
        {
            BACnetAddressPtr	ap = new BACnetAddress()
            ;

            // the address is the data
            ap->Station( SubstituteArgs(attrs["address"]) );

            return ap;
        }
    };

BACnetBTRPeerFactory gBACnetBTRPeerFactory;

//
//  BACnetBTRFactory
//

BFRFactoryChild gBACnetBTRFactoryChildren[] =
    { { "Peer", &gBACnetBTRPeerFactory }
//  , { "Accept", &gBFRFilterElemFactory }
//  , { "Reject", &gBFRFilterElemFactory }
    , { 0, 0 }
    };

BACnetBTRFactory gBACnetBTRFactory;

//
//  BACnetBTRFactory::BACnetBTRFactory
//

BACnetBTRFactory::BACnetBTRFactory( void )
    : BFRFactory( gBACnetBTRFactoryChildren )
{
}

//
//  BACnetBTRFactory::~BACnetBTRFactory
//

BACnetBTRFactory::~BACnetBTRFactory( void )
{
}

//
//  BACnetBTRFactory::StartElement
//

voidPtr BACnetBTRFactory::StartElement( const char *name, const MinML::AttributeList& attrs )
{
    BACnetBTRPtr    btrp = new BACnetBTR()
    ;

    // register it
    // TO-DO : Add in error checking, otherwise this will segfault if NULL
    gBFRRegistration.RegisterClient( SubstituteArgs(attrs["client"]), btrp );
    gBFRRegistration.RegisterServer( SubstituteArgs(attrs["server"]), btrp );

    // return the BTR
    return btrp;
}

//
//	BACnetBTRFactory::ChildElement
//

void BACnetBTRFactory::ChildElement( voidPtr ep, int id, voidPtr cp )
{
    BACnetBTRPtr        btrp = (BACnetBTRPtr)ep
    ;
    BACnetAddressPtr    ap = (BACnetAddressPtr)cp
    ;

    // add the peer
    btrp->AddPeer( *ap );

    // delete the address, we don't need it
    delete ap;
}
