//
//  BFRBBMD
//

#include <stdlib.h>
#include <string.h>

#include "BFRBBMD.h"

#include "BFRRegistration.h"

//
//	BACnetBBMDPeerFactory
//

class BACnetBBMDPeerFactory : public BFRFactory {
    public:
        virtual voidPtr StartElement( const char *name, const MinML::AttributeList& attrs )
        {
            const char  *s = SubstituteArgs(attrs["address"])
            ;

            // make sure it's there
            if (!s || !*s)
                throw_1(2005);      // peer address required

            // copy the data
            char *rslt = new char[strlen(s)+1];
            strcpy( rslt, s );

            // return the string
            return rslt;
        }
    };

BACnetBBMDPeerFactory gBACnetBBMDPeerFactory;

//
//  BACnetBBMDFactory
//

BFRFactoryChild gBACnetBBMDFactoryChildren[] =
    { { "Peer", &gBACnetBBMDPeerFactory }
//  , { "Accept", &gBFRFilterElemFactory }
//  , { "Reject", &gBFRFilterElemFactory }
    , { 0, 0 }
    };

BACnetBBMDFactory gBACnetBBMDFactory;

//
//  BACnetBBMDFactory::BACnetBBMDFactory
//

BACnetBBMDFactory::BACnetBBMDFactory( void )
    : BFRFactory( gBACnetBBMDFactoryChildren )
{
}

//
//  BACnetBBMDFactory::~BACnetBBMDFactory
//

BACnetBBMDFactory::~BACnetBBMDFactory( void )
{
}

//
//  BACnetBBMDFactory::StartElement
//

voidPtr BACnetBBMDFactory::StartElement( const char *name, const MinML::AttributeList& attrs )
{
    // The address that we will include in the "originating device" field of Forwarded-NPDU messages.
    // Should be set to an address by which the BBMD can be reached, i.e. its local address, or perhaps a different address if NAT is used.
    const char *bbmd_address = SubstituteArgs(attrs["address"]);
    if (!bbmd_address || !*bbmd_address)
        throw_1(12001);     // address required

    BACnetBBMDPtr   bbmdp = new BACnetBBMD(BACnetAddress(bbmd_address))
    ;

    // find out if foreign device registration should be allowed
    const char *s = SubstituteArgs(attrs["foreign"]);
    if (s && ((*s == '1') || (*s == 'y') || (*s == 'Y')))
        bbmdp->bbmdFDSupport = true;

    // register it
    gBFRRegistration.RegisterClient( SubstituteArgs(attrs["client"]), bbmdp );
    gBFRRegistration.RegisterServer( SubstituteArgs(attrs["server"]), bbmdp );

    // return the BBMD
    return bbmdp;
}

//
//	BACnetBBMDFactory::ChildElement
//

void BACnetBBMDFactory::ChildElement( voidPtr ep, int id, voidPtr cp )
{
    BACnetBBMDPtr       bbmdp = (BACnetBBMDPtr)ep
    ;
    char                *ap = (char *)cp
    ;

    // add the peer
    bbmdp->AddPeer( ap );

    // delete the address, we don't need it
    delete[] ap;
}
