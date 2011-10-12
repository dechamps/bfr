//
//  BFRSimple
//

#include <stdlib.h>
#include <string.h>

#include "BFRSimple.h"

#include "BFRRegistration.h"

//
//  BACnetBIPSimpleFactory
//

BACnetBIPSimpleFactory gBACnetBIPSimpleFactory;

//
//  BACnetBIPSimpleFactory::StartElement
//

voidPtr BACnetBIPSimpleFactory::StartElement( const char *name, const MinML::AttributeList& attrs )
{
    BACnetBIPSimplePtr  sp = new BACnetBIPSimple()
    ;

    // register
    gBFRRegistration.RegisterClient( SubstituteArgs(attrs["client"]), sp );
    gBFRRegistration.RegisterServer( SubstituteArgs(attrs["server"]), sp );

    // return the btr
    return sp;
}
