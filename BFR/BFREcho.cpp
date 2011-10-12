//
//  BFREcho
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "BFREcho.h"

//
//  BFREcho::Indication
//

void BFREcho::Indication( const BACnetPDU &pdu )
{
    BACnetPDU   newpdu
    ;

    // the source is the destination
    newpdu.pduSource = pdu.pduDestination;
    newpdu.SetReference( pdu );

    // send it back to the client
    Response( newpdu );
}

//
//  BFREcho::Confirmation
//

void BFREcho::Confirmation( const BACnetPDU &pdu )
{
    BACnetPDU   newpdu
    ;

    // the destination is the source
    newpdu.pduDestination = pdu.pduSource;
    newpdu.SetReference( pdu );

    // send it back to the server
    Request( newpdu );
}

//
//  BFREchoFactory
//

BFREchoFactory gBFREchoFactory;

//
//  BFREchoFactory::StartElement
//

voidPtr BFREchoFactory::StartElement( const char *name, const MinML::AttributeList& attrs )
{
    BFREchoPtr      debugp = new BFREcho()
    ;
    const char      *s
    ;

    // register as a client, server, or both
    if ((s = SubstituteArgs(attrs["client"])) != 0)
        gBFRRegistration.RegisterClient( s, debugp );

    if ((s = SubstituteArgs(attrs["server"])) != 0)
        gBFRRegistration.RegisterServer( s, debugp );

    // return the btr
    return debugp;
}
