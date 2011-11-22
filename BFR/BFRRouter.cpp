//
//  BFRRouter
//

#include <stdlib.h>
#include <string.h>

#include "BFRRouter.h"
#include "BFRMLAN.h"

#include "BFRRegistration.h"

//
//  BACnetRouterAdapterPeerFactory::BACnetRouterAdapterPeerFactory
//

BACnetRouterAdapterPeerFactory gBACnetRouterAdapterPeerFactory;

//
//  BACnetRouterAdapterPeerFactory::StartElement
//

voidPtr BACnetRouterAdapterPeerFactory::StartElement( const char *name, const MinML::AttributeList& attrs )
{
    int             status, dnet
    ;
    const char      *s, *caddr
    ;
    BACnetAddress   peerAddress
    ;

    // get the address
    if (!(caddr = SubstituteArgs(attrs["address"])))
        throw_1(5091);          // address is required
    // set the address value
    peerAddress.Station( caddr );

    // find the status, 0=OK, 1=busy
    if ((s = SubstituteArgs(attrs["status"])) != 0)
        status = atoi( s );
    else
        status = 0;

    // find the network
    if (!(s = SubstituteArgs(attrs["networks"])))
        throw_1(5092);          // networks is required

    while (*s) {
        while (isspace(*s)) s++;
        if (!isdigit(*s))
            throw_1(5093);      // integer required

        // build the network number
        dnet = 0;
        while (isdigit(*s))
            dnet = (dnet * 10) + (*s++ - '0');

        // check it for validity
        if (net >= 65535)
            throw_1(5094);      // invalid network number

        // ### add the path
        // AddRoute( int snet, dnet, peerAddress )

        // ready for the next one
        while (isspace(*s)) s++;
        if (*s == ',')
            s += 1;
    }
}

//
//  BACnetRouterAdapterFactory
//

BFRFactoryChild gBACnetRouterAdapterFactoryChildren[] =
    { { "Peer", &gBACnetRouterAdapterPeerFactory }
    , { 0, 0 }
    };

BACnetRouterAdapterFactory gBACnetRouterAdapterFactory;

BACnetRouterAdapterFactory::BACnetRouterAdapterFactory( void )
    : BFRFactory(gBACnetRouterAdapterFactoryChildren)
{
}

//
//  BACnetRouterAdapterFactory::~BACnetRouterAdapterFactory
//

BACnetRouterAdapterFactory::~BACnetRouterAdapterFactory( void )
{
}

//
//  BACnetRouterAdapterFactory::StartElement
//

voidPtr BACnetRouterAdapterFactory::StartElement( const char *name, const MinML::AttributeList& attrs )
{
    BACnetRouterAdapterPtr  rap = new BACnetRouterAdapter()
    ;
    const char              *s, *cname
    ;

    // find the network
    if ((s = SubstituteArgs(attrs["net"])) != 0)
        rap->adapterNet = atoi( s );

    // register as a client
    cname = SubstituteArgs(attrs["client"]);
    if (!cname)
        throw_1(5001);      // register as a client

    gBFRRegistration.RegisterClient( cname, rap );

    // return the adapter
    return rap;
}

//
//  BACnetRouterAdapterFactory::ChildElement
//

void BACnetRouterAdapterFactory::ChildElement( voidPtr ep, int id, voidPtr cp )
{
    BACnetRouterAdapterPtr      rap = (BACnetRouterAdapterPtr)ep
    ;
    BACnetRouterAdapterPeerPtr  rapp = (BACnetRouterAdapterPeerPtr)cp
    ;

    // link them together
    rapp->peerNext = rap->adapterPeerList;
    rap->adapterPeerList = rapp;
}

//
//  BACnetRouterFactory
//

BFRFactoryChild gBACnetRouterFactoryChildren[] =
    { { "Adapter", &gBACnetRouterAdapterFactory }
    , { "MAdapter", &gBACnetMLANAdapterFactory }
    , { 0, 0 }
    };

BACnetRouterFactory gBACnetRouterFactory;

//
//  BACnetRouterFactory::BACnetRouterFactory
//

BACnetRouterFactory::BACnetRouterFactory( void )
    : BFRFactory(gBACnetRouterFactoryChildren)
{
}

//
//  BACnetRouterFactory::~BACnetRouterFactory
//

BACnetRouterFactory::~BACnetRouterFactory( void )
{
}

//
//  BACnetRouterFactory::StartElement
//

voidPtr BACnetRouterFactory::StartElement( const char *name, const MinML::AttributeList& attrs )
{
    BACnetRouterPtr     rp = new BACnetRouter()
    ;
    const char          *sname, *cname
    ;

    // get the client and server name
    cname = SubstituteArgs(attrs["client"]);
    sname = SubstituteArgs(attrs["server"]);

    // if the client is specified, the server must be as well
    if (cname && !sname)
        throw_1(5009);      // server required

    // if this is a client, build one of our funny adapters
    if (cname) {
        BACnetRouterAdapterPtr	rap = new BACnetRouterAdapter()
        ;

        // set this as the local adapter
        rap->adapterNet = kBACnetRouterLocalNetwork;

        // add it to the router
        rp->AddAdapter( rap );

        // register as a client
        gBFRRegistration.RegisterClient( cname, rap );
    }

    // if this is a server there will be an APDU layer above it
    if (sname)
        gBFRRegistration.RegisterServer( sname, rp );

    // if there is no direct client, there will probably be more than one adapter
    if (!cname)
        new BACnetRouterBroadcastRoutingTablesTask( rp );

    // return a router
    return rp;
}

//
//	BACnetRouterFactory::ChildElement
//

void BACnetRouterFactory::ChildElement( voidPtr ep, int id, voidPtr cp )
{
    int                         i
    ;
    BACnetRouterPtr             rp = (BACnetRouterPtr)ep
    ;
    BACnetRouterAdapterPtr      rap = (BACnetRouterAdapterPtr)cp
    ;
    BACnetRouterAdapterPeerPtr  rapp = (BACnetRouterAdapterPeerPtr)cp
    ;
    BACnetRouterList            *dst
    ;

    // children are adapters
    rp->AddAdapter( rap );

    // add the peers
    rapp = rap->adapterPeerList;
    while (rapp) {
        for (i = 0; i < rapp->peerNetListLen; i++) {
            // ### check for existing table entries

            // add to the end of the list
            dst = rp->routerList + rp->routerListLen;
            dst->routerNet = rapp->peerNetList[i];
            dst->routerStatus = rapp->peerStatus;
            dst->routerAddr = rapp->peerAddress;
            dst->routerAdapter = rap;
            rp->routerListLen += 1;
        }

        // next peer element
        rapp = rapp->peerNext;
    }
}

