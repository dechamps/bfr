//
//  BFRConsole
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "BFRConsole.h"

//
//  BFRConsoleFactory
//

BFRConsoleFactory gBFRConsoleFactory;

//
//  BFRConsoleFactory::StartElement
//

voidPtr BFRConsoleFactory::StartElement( const char *name, const MinML::AttributeList& attrs )
{
    BACnetConsolePtr    cp = new BACnetConsole()
    ;
    const char          *s, *cname, *sname
    ;

    // find the prefix
    if ((s = SubstituteArgs(attrs["prefix"])) != 0)
        cp->SetPrefix(s);

    // register as a client or server, but not both
    cname = SubstituteArgs(attrs["client"]);
    sname = SubstituteArgs(attrs["server"]);
    if (cname && sname)
        throw_1(7001);      // do not register as both a client and a server
    if (!cname && !sname)
        throw_1(7002);      // register as a client or a server

    if (cname)
        gBFRRegistration.RegisterClient( cname, cp );
    if (sname)
        gBFRRegistration.RegisterServer( sname, cp );

    // initialize as a port
    cp->Init();

    // return the console
    return cp;
}
