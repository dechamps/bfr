//
//  BFRRouter
//

#include <stdlib.h>
#include <string.h>

#include "BFRRouter.h"
#include "BFRMLAN.h"

#include "BFRRegistration.h"

//
//  BFRRouterAdapterPeerFactory::BFRRouterAdapterPeerFactory
//

BFRRouterAdapterPeerFactory gBFRRouterAdapterPeerFactory;

//
//  BFRRouterAdapterPeerFactory::StartElement
//

voidPtr BFRRouterAdapterPeerFactory::StartElement( const char *name, const MinML::AttributeList& attrs )
{
    BFRRouterAdapterPeerPtr app = new BFRRouterAdapterPeer()
    ;
    int             status, dnet
    ;
    const char      *s, *caddr
    ;

    // get the address
    if (!(caddr = SubstituteArgs(attrs["address"])))
        throw_1(5091);          // address is required
    // set the address value
    app->peerAddress.Station( caddr );

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
        if (dnet >= 65535)
            throw_1(5094);      // invalid network number

        // add the path to the list
        if (app->peerNetListLen >= kBFRRouterAdapterMaxPeerNetListLen)
            throw_1(5095);      // too many peer networks
        app->peerNetList[app->peerNetListLen++] = dnet;

        // ready for the next one
        while (isspace(*s)) s++;
        if (*s == ',')
            s += 1;
    }
}

//
//  BFRRouterAdapterFactory
//

BFRFactoryChild gBFRRouterAdapterFactoryChildren[] =
    { { "Peer", &gBFRRouterAdapterPeerFactory }
    , { 0, 0 }
    };

BFRRouterAdapterFactory gBFRRouterAdapterFactory;

BFRRouterAdapterFactory::BFRRouterAdapterFactory( void )
    : BFRFactory(gBFRRouterAdapterFactoryChildren)
{
}

//
//  BFRRouterAdapterFactory::~BFRRouterAdapterFactory
//

BFRRouterAdapterFactory::~BFRRouterAdapterFactory( void )
{
}

//
//  BFRRouterAdapterFactory::StartElement
//

voidPtr BFRRouterAdapterFactory::StartElement( const char *name, const MinML::AttributeList& attrs )
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
//  BFRRouterAdapterFactory::ChildElement
//

void BFRRouterAdapterFactory::ChildElement( voidPtr ep, int id, voidPtr cp )
{
    BACnetRouterAdapterPtr  rap = (BACnetRouterAdapterPtr)ep
    ;
    BFRRouterAdapterPeerPtr rapp = (BFRRouterAdapterPeerPtr)cp
    ;

    // ### tell the router of the adapter (also its parent) of the peers
}

//
//  BFRRouterFactory
//

BFRFactoryChild gBFRRouterFactoryChildren[] =
    { { "Adapter", &gBFRRouterAdapterFactory }
    , { "MAdapter", &gBACnetMLANAdapterFactory }
    , { 0, 0 }
    };

BFRRouterFactory gBFRRouterFactory;

//
//  BFRRouterFactory::BFRRouterFactory
//

BFRRouterFactory::BFRRouterFactory( void )
    : BFRFactory(gBFRRouterFactoryChildren)
{
}

//
//  BFRRouterFactory::~BFRRouterFactory
//

BFRRouterFactory::~BFRRouterFactory( void )
{
}

//
//  BFRRouterFactory::StartElement
//

voidPtr BFRRouterFactory::StartElement( const char *name, const MinML::AttributeList& attrs )
{
    BACnetRouterPtr     rp = new BACnetRouter()
    ;
    const char          *sname, *cname, *s
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

    // check for dynamic routing, turned off by default
    s = SubstituteArgs(attrs["dynamic"]);
    if (s && ((*s == '1') || (*s == 'y') || (*s == 'Y')))
        rp->dynamicRouting = true;

    // return a router
    return rp;
}

//
//	BFRRouterFactory::ChildElement
//

void BFRRouterFactory::ChildElement( voidPtr ep, int id, voidPtr cp )
{
    int                     i
    ;
    BACnetRouterPtr         rp = (BACnetRouterPtr)ep
    ;
    BACnetRouterAdapterPtr  rap = (BACnetRouterAdapterPtr)cp
    ;
    BFRRouterAdapterPeerPtr rapp = (BFRRouterAdapterPeerPtr)cp
    ;
    BACnetRouterList        *dst
    ;

    // children are adapters
    rp->AddAdapter( rap );

#if 0
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
#endif
}

