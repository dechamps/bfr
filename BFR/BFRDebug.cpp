//
//  BFRDebug
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "BFRDebug.h"

//
//  BFRDebug::BFRDebug
//

BFRDebug::BFRDebug( void )
    : debugPrefix(0)
{
}

//
//  BFRDebug::~BFRDebug
//

BFRDebug::~BFRDebug( void )
{
    if (debugPrefix)
        delete[] debugPrefix;
}

//
//  BFRDebug::Indication
//
//  This is called when there is a downstream message.  Note that the 
//  downstream side of this debug module doesn't have to be connected 
//  to anything, it will just print out the contents of the packet.
//  Good for testing routers.
//

void BFRDebug::Indication( const BACnetPDU &pdu )
{
    if (debugPrefix)
        printf( "%s ", debugPrefix );

    printf( "%s <- ", pdu.pduDestination.ToString() );
    printf( "%s : ", pdu.pduSource.ToString() );
    for (int i = 0; i < pdu.pduLen; i++)
        printf( "%02X.", pdu.pduData[i] );
    printf( "\n" );

    if (clientPeer)
        Request( pdu );
}

//
//  BFRDebug::Confirmation
//
//  This is called when there is an upstream message.  Note that the 
//  upstream side of this debug module doesn't have to be connected 
//  to a client, it will just print out the contents of the packet.
//  Good for testing adapters and filters.
//

void BFRDebug::Confirmation( const BACnetPDU &pdu )
{
    if (debugPrefix)
        printf( "%s ", debugPrefix );

    printf( "%s -> ", pdu.pduSource.ToString() );
    printf( "%s : ", pdu.pduDestination.ToString() );
    for (int i = 0; i < pdu.pduLen; i++)
        printf( "%02X.", pdu.pduData[i] );
    printf( "\n" );

    if (serverPeer)
        Response( pdu );
}

//
//  BFRDebugFactory
//

BFRDebugFactory gBFRDebugFactory;

//
//  BFRDebugFactory::StartElement
//

voidPtr BFRDebugFactory::StartElement( const char *name, const MinML::AttributeList& attrs )
{
    BFRDebugPtr     debugp = new BFRDebug()
    ;
    const char      *s
    ;

    // find the prefix
    if ((s = SubstituteArgs(attrs["prefix"])) != 0) {
        debugp->debugPrefix = new char[strlen(s)+1];
        strcpy( debugp->debugPrefix, s );
    }

    // register as a client, server, or both
    if ((s = SubstituteArgs(attrs["client"])) != 0)
        gBFRRegistration.RegisterClient( s, debugp );

    if ((s = SubstituteArgs(attrs["server"])) != 0)
        gBFRRegistration.RegisterServer( s, debugp );

    // return the btr
    return debugp;
}
