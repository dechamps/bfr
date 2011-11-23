//
//  BFRDebug
//

#ifndef _BFRDebug_
#define _BFRDebug_

#include "BACnet.h"
#include "BACnetDebug.h"

#include "BFRParser.h"
#include "BFRRegistration.h"

//
//  BFRDebugFactory
//

class BFRDebugFactory : public BFRFactory {
    public:
        virtual voidPtr StartElement( const char *name, const MinML::AttributeList& attrs );
    };

typedef BFRDebugFactory *BFRDebugFactoryPtr;

extern BFRDebugFactory gBFRDebugFactory;

#endif
