//
//  BFRMLAN
//

#include <stdlib.h>
#include <string.h>

#include "BFRMLAN.h"

#include "BFRRegistration.h"

//
//  BACnetMLANFilterElemFactory
//

BACnetMLANFilterElemFactory gBACnetMLANFilterElemFactory;

//
//  BACnetMLANFilterElemFactory::StartElement
//

voidPtr BACnetMLANFilterElemFactory::StartElement( const char *name, const MinML::AttributeList& attrs )
{
    const char              *valu
    ;
    BACnetMLANFilterElemPtr fep
    ;

    // build a filter element
    fep = new BACnetMLANFilterElem();

    // see if this is an accept element, this generator works for both accept and reject
    fep->filterAccept = (strcmp(name,"Accept") == 0);

    // check for component matching

    // get the address
    valu = SubstituteArgs(attrs["address"]);
    if (!valu)
        throw_1(99008);     // specification error
    fep->filterAddress.SetFilter( valu );

    // return the element
    return fep;
}

//
//  BACnetMLANStaticFactory
//

BACnetMLANStaticFactory gBACnetMLANStaticFactory;

//
//  BACnetMLANStaticFactory::StartElement
//

voidPtr BACnetMLANStaticFactory::StartElement( const char *name, const MinML::AttributeList& attrs )
{
    const char              *valu
    ;
    BACnetMLANStaticPtr     sp = new BACnetMLANStatic()
    ;

    // get the address
    valu = SubstituteArgs(attrs["address"]);
    if (!valu)
        throw_1(99008);     // specification error
    sp->staticAddress.Station( valu );

    // get the node
    valu = SubstituteArgs(attrs["node"]);
    if (!valu)
        throw_1(99009);     // specification error
    sp->staticNode = atoi(valu);

    // return the element
    return sp;
}

//
//  BACnetMLANAdapterFactory
//

BFRFactoryChild gBACnetMLANAdapterChildren[] =
    { { "Static", &gBACnetMLANStaticFactory }
    , { "Accept", &gBACnetMLANFilterElemFactory }
    , { "Reject", &gBACnetMLANFilterElemFactory }
    , { 0, 0 }
    };

BACnetMLANAdapterFactory gBACnetMLANAdapterFactory;

//
//  BACnetMLANAdapterFactory::BACnetMLANAdapterFactory
//

BACnetMLANAdapterFactory::BACnetMLANAdapterFactory( void )
    : BFRFactory(gBACnetMLANAdapterChildren)
{
}

//
//  BACnetMLANAdapterFactory::StartElement
//

voidPtr BACnetMLANAdapterFactory::StartElement( const char *name, const MinML::AttributeList& attrs )
{
    BACnetMLANAdapterPtr    map = new BACnetMLANAdapter()
    ;
    const char              *s
    ;

    // find the network
    if ((s = SubstituteArgs(attrs["net"])) != 0)
        map->adapterNet = atoi( s );

    // find the MLAN
    s = SubstituteArgs(attrs["mlan"]);
    if (!s)
        throw_1(99001);     // needs an MLAN

    // find it
    for (BACnetMLANPtr mp = gBACnetMLANList; mp; mp = mp->mlanNext)
        if (strcmp(s,mp->mlanName) == 0) {
            mp->AddAdapter( map );
            break;
        }

    // make sure one was found
    if (!map->mlanHost)
        throw_1(99002);     // host not found

    // return the adapter (to the BACnetRouterFactory!)
    return map;
}

//
//  BACnetMLANAdapterFactory::ChildElement
//

void BACnetMLANAdapterFactory::ChildElement( voidPtr ep, int id, voidPtr cp )
{
    BACnetMLANFilterElemPtr     *fepp
    ;
    BACnetMLANAdapterPtr        map = (BACnetMLANAdapterPtr)ep
    ;

    // see if this is a static mapping
    if (id == 0) {
        BACnetMLANStaticPtr sp = (BACnetMLANStaticPtr)cp;

        // tell the MLAN about the static mapping
        map->mlanHost->MapAddr( map, sp->staticAddress, sp->staticNode );

        // no longer needed
        delete sp;
    } else {
        // child elements are accept or reject filter elements

        // find the end of the list
        for (fepp = &map->mlanFilter; *fepp; fepp = &(*fepp)->filterNext)
            ;

        // add it to the end
        *fepp = (BACnetMLANFilterElemPtr)cp;
    }
}

//
//  BACnetMLANFactory
//

BACnetMLANFactory gBACnetMLANFactory;

//
//  BACnetMLANFactory::StartElement
//

voidPtr BACnetMLANFactory::StartElement( const char *name, const MinML::AttributeList& attrs )
{
    BACnetMLANPtr   mp = new BACnetMLAN()
    ;
    const char      *s
    ;

    // give it a name
    s = SubstituteArgs(attrs["mlan"]);
    if (!s)
        throw_1(99003);     // needs a name

    mp->mlanName = new char[strlen(s) + 1];
    strcpy( mp->mlanName, s );

    // return the MLAN
    return mp;
}
