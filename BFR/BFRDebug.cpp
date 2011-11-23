//
//  BFRDebug
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "BFRDebug.h"

//
//  BFRDebugFactory
//

BFRDebugFactory gBFRDebugFactory;

//
//  BFRDebugFactory::StartElement
//

voidPtr BFRDebugFactory::StartElement( const char *name, const MinML::AttributeList& attrs )
{
    BACnetDebugPtr  debugp = new BACnetDebug()
    ;
    const char      *s
    ;

    // find the prefix
    if ((s = SubstituteArgs(attrs["prefix"])) != 0)
        debugp->SetPrefix(s);

    // register as a client, server, or both
    if ((s = SubstituteArgs(attrs["client"])) != 0)
        gBFRRegistration.RegisterClient( s, debugp );

    if ((s = SubstituteArgs(attrs["server"])) != 0)
        gBFRRegistration.RegisterServer( s, debugp );

    // return the btr
    return debugp;
}

