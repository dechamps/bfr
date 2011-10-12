//
//  BFRSimple
//

#ifndef _BFRSimple_
#define _BFRSimple_

#include "BFRParser.h"
#include "BACnetBIPSimple.h"

//
//  BACnetBIPSimpleFactory
//

class BACnetBIPSimpleFactory : public BFRFactory {
    public:
        virtual voidPtr StartElement( const char *name, const MinML::AttributeList& attrs );
    };

typedef BACnetBIPSimpleFactory *BACnetBIPSimpleFactoryPtr;

extern BACnetBIPSimpleFactory gBACnetBIPSimpleFactory;

#endif
