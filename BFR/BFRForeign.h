//
//  BFRForeign
//

#ifndef _BFRForeign_
#define _BFRForeign_

#include "BFRParser.h"
#include "BACnetBIPForeign.h"

//
//	BACnetBIPForeignFactory
//

class BACnetBIPForeignFactory : public BFRFactory {
    public:
        virtual voidPtr StartElement( const char *name, const MinML::AttributeList& attrs );
    };

typedef BACnetBIPForeignFactory *BACnetBIPForeignFactoryPtr;

extern BACnetBIPForeignFactory gBACnetBIPForeignFactory;

#endif
