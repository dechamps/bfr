//
//  BACnetConsole
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "BACnetConsole.h"

//

BACnetConsolePtr gBACnetConsole = 0;

//
//  BACnetConsole::BACnetConsole
//

BACnetConsole::BACnetConsole( void )
    : consolePrefix(0)
{
    gBACnetConsole = this;
}

//
//  BACnetConsole::~BACnetConsole
//

BACnetConsole::~BACnetConsole( void )
{
    if (consolePrefix)
        delete[] consolePrefix;
}

//
//  BACnetConsole::SetPrefix
//

void BACnetConsole::SetPrefix(const char *prefix)
{
    consolePrefix = new char[strlen(prefix)+1];
    strcpy(consolePrefix, prefix);
}

//
//  BACnetConsole::Send
//

void BACnetConsole::Send( const char *addr, const char *data )
{
    BACnetPDU       pdu
    ;
    BACnetAddress   conAddr
    ;
    BACnetOctet     msg[1024], *dst = msg
    ;
    int             n
    ;

    // load the address
    conAddr.Station( addr );

    // set the expecting reply and priority
    pdu.pduExpectingReply = 0;
    pdu.pduNetworkPriority = 0;

    // load the data
    while (*data) {
        if (!isxdigit(*data))
            break;

        n = (isdigit(*data) ? (*data - '0') : (toupper(*data) - 'A' + 10));
        data += 1;

        if (!isxdigit(*data))
            break;

        n = (n << 4) + (isdigit(*data) ? (*data - '0') : (toupper(*data) - 'A' + 10));
        data += 1;

        // save the octet
        *dst++ = n;
    }

    // set the packet reference
    pdu.SetReference( msg, dst - msg );	

    // if this is a client, send the request
    if (clientPeer) {
        pdu.pduDestination = conAddr;
        Request( pdu );
    }

    // if this is a server, send the response
    if (serverPeer) {
        pdu.pduSource = conAddr;
        Response( pdu );
    }
}

//
//  BACnetConsole::Indication
//
//  This is called when there is a downstream message.  Note that the 
//  downstream side of this console module isn't connected to anything.
//

void BACnetConsole::Indication( const BACnetPDU &pdu )
{
    if (consolePrefix)
        printf( "%s ", consolePrefix );

    printf( "%s <- ", pdu.pduDestination.ToString() );
    printf( "%s : ", pdu.pduSource.ToString() );
    for (int i = 0; i < pdu.pduLen; i++)
        printf( "%02X.", pdu.pduData[i] );
    printf( "\n" );
}

//
//  BACnetConsole::Confirmation
//
//  This is called when there is an upstream message.  Note that the 
//  upstream side of this console module isn't connected to anything.
//

void BACnetConsole::Confirmation( const BACnetPDU &pdu )
{
    if (consolePrefix)
        printf( "%s ", consolePrefix );

    printf( "%s -> ", pdu.pduSource.ToString() );
    printf( "%s : ", pdu.pduDestination.ToString() );
    for (int i = 0; i < pdu.pduLen; i++)
        printf( "%02X.", pdu.pduData[i] );
    printf( "\n" );
}

//
//  BACnetConsole::Init
//

void BACnetConsole::Init( void )
{
    // standard input file descriptor
    portSocket = 0;
    portStatus = 0;
}

//
//  ConsoleUsage
//

void ConsoleUsage( void )
{
    printf( "Console:\n" );
    printf( "   quit           terminate application\n" );
    printf( "   help           this description\n" );
    printf( "   <addr> <data>  console data\n" );
    printf( "\n" );
    printf( "<addr> is one of:\n" );
    printf( "   n              local station\n" );
    printf( "   xx-xx-xx       local station\n" );
    printf( "   *              local broadcast\n" );
    printf( "   n:*            remote broadcast\n" );
    printf( "   n:n            remote station\n" );
    printf( "   n:xx-xx-xx     remote station\n" );
    printf( "   *:*            global broadcast\n" );
    printf( "\n" );
    printf( "<data> is a sequence of hex digit pairs\n" );
    printf( "\n" );
}

//
//  BACnetConsole::Read
//
//  This function is called when the user has finished entering a 
//  command.
//

extern bool gBFRRunning;

void BACnetConsole::Read( void )
{
    static char     line[256]
    ;
    int             i, len, argc
    ;
    char            *c, *argv[256]
    ;

    // read the line
    c = fgets( line, sizeof(line), stdin );

    // trim off the \n
    len = strlen(line) - 1;
    if (line[len] == '\n') line[len] = 0;

    // chop it up into args
    argc = 0;
    while (*c) {
        // skip over whitespace
        while (*c && isspace(*c))
            c++;
        if (!*c)
            break;
            
        // save the arg
        argv[argc++] = c;
        
        // skip over non-whitespace
        while (*c && !isspace(*c))
            c++;
            
        // null terminate
        if (*c)
            *c++ = 0;
    }
    
    // nothing to do
    if (!argc)
        return;

    // check for quit
    if ((toupper(*argv[0]) == 'Q') || (toupper(*argv[0]) == 'X')) {
        gBFRRunning = false;
        return;
    }

    // skip if nothing available
    if ((toupper(*argv[0]) == 'H') || (argc < 2)) {
        ConsoleUsage();
        return;
    }

    // send this packet
    Send( argv[0], argv[1] );
}

//
//  BACnetConsole::Write
//
//  This function is never called because packets delivered to the 
//  console object (upstream or downstream) are printed and never 
//  queued to the portWriteList.
//

void BACnetConsole::Write( void )
{
}

