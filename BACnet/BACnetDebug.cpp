//
//  BACnetDebug
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "BACnetDebug.h"

//
//  BACnetDebug::BACnetDebug
//

BACnetDebug::BACnetDebug( const char *prefix, const bool newLine )
    : debugPrefix(0), debugNewLine(newLine)
{
    if (prefix)
        SetPrefix(prefix);
}

//
//  BACnetDebug::~BACnetDebug
//

BACnetDebug::~BACnetDebug( void )
{
    if (debugPrefix)
        delete[] debugPrefix;
}

//
//  BACnetDebug::SetPrefix
//

void BACnetDebug::SetPrefix( const char *prefix )
{
    debugPrefix = new char[strlen(prefix)+1];
    strcpy(debugPrefix, prefix);
}

//
//  BACnetDebug::Indication
//
//  This is called when there is a downstream message.  Note that the 
//  downstream side of this debug module doesn't have to be connected 
//  to anything, it will just print out the contents of the packet.
//  Good for testing routers.
//

void BACnetDebug::Indication( const BACnetPDU &pdu )
{
    if (debugPrefix)
        printf( "%s ", debugPrefix );

    printf( "%s <- ", pdu.pduDestination.ToString() );
    printf( "%s : ", pdu.pduSource.ToString() );
    if (pdu.pduTag)
        printf( "[%02X] : ", pdu.pduTag );
    for (int i = 0; i < pdu.pduLen; i++)
        printf( "%02X.", pdu.pduData[i] );
    printf( "\n" );

    if (clientPeer)
        Request( pdu );
}

//
//  BACnetDebug::Confirmation
//
//  This is called when there is an upstream message.  Note that the 
//  upstream side of this debug module doesn't have to be connected 
//  to a client, it will just print out the contents of the packet.
//  Good for testing adapters and filters.
//

void BACnetDebug::Confirmation( const BACnetPDU &pdu )
{
    if (debugNewLine)
        printf( "\n\n" );

    if (debugPrefix)
        printf( "%s ", debugPrefix );

    printf( "%s -> ", pdu.pduSource.ToString() );
    printf( "%s : ", pdu.pduDestination.ToString() );
    if (pdu.pduTag)
        printf( "[%02X] : ", pdu.pduTag );
    for (int i = 0; i < pdu.pduLen; i++)
        printf( "%02X.", pdu.pduData[i] );
    printf( "\n" );

    if (serverPeer)
        Response( pdu );
}

