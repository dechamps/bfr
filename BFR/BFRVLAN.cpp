//
//  BFRVLAN
//

#include <stdlib.h>
#include <string.h>

#include "BFRVLAN.h"

#include "BFRRegistration.h"

//
//  BACnetVLANNodeFactory
//

BACnetVLANNodeFactory gBACnetVLANNodeFactory;

//
//  BACnetVLANNodeFactory::StartElement
//

voidPtr BACnetVLANNodeFactory::StartElement( const char *name, const MinML::AttributeList& attrs )
{
    BACnetVLANNodePtr   np = new BACnetVLANNode()
    ;

    // set the address value
    np->nodeAddress.Station( SubstituteArgs(attrs["address"]) );

    // enable or disable spoofing
    const char *s = SubstituteArgs(attrs["promiscuous"]);
    if (s && ((*s == '1') || (*s == 'y') || (*s == 'Y')))
        np->nodePromiscuous = true;

    // register the server
    gBFRRegistration.RegisterServer( SubstituteArgs(attrs["server"]), np );

    // return the result
    return np;
}

//
//  BACnetVLANFactory
//

BFRFactoryChild gBACnetVLANFactoryChildren[] =
    { { "Node", &gBACnetVLANNodeFactory }
    , { 0, 0 }
    };

BACnetVLANFactory gBACnetVLANFactory;

//
//  BACnetVLANFactory::BACnetVLANFactory
//

BACnetVLANFactory::BACnetVLANFactory( void )
    : BFRFactory(gBACnetVLANFactoryChildren)
{
}

//
//  BACnetVLANFactory::StartElement
//

voidPtr BACnetVLANFactory::StartElement( const char *name, const MinML::AttributeList& attrs )
{
    // return the VLAN
    return new BACnetVLAN();
}

//
//  BACnetVLANFactory::ChildElement
//

void BACnetVLANFactory::ChildElement( voidPtr ep, int id, voidPtr cp )
{
    BACnetVLANPtr       vlanp = (BACnetVLANPtr)ep
    ;
    BACnetVLANNodePtr   np = (BACnetVLANNodePtr)cp
    ;

    // children are nodes on the lan addresses
    vlanp->AddNode( np );
}
