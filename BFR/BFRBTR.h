//
//  BFRBTR
//

#ifndef _BFRBTR_
#define _BFRBTR_

#include "BFRParser.h"
#include "BACnetBTR.h"

//
//  BACnetBTRFactory
//

class BACnetBTRFactory : public BFRFactory {
    public:
        BACnetBTRFactory( void );
        virtual ~BACnetBTRFactory( void );

        virtual voidPtr StartElement( const char *name, const MinML::AttributeList& attrs );
        virtual void ChildElement( voidPtr ep, int id, voidPtr cp );
    };

typedef BACnetBTRFactory *BACnetBTRFactoryPtr;

extern BACnetBTRFactory gBACnetBTRFactory;

#endif
