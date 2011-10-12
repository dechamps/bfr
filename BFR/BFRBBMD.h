//
//  BFRBBMD
//

#ifndef _BFRBBMD_
#define _BFRBBMD_

#include "BFRParser.h"
#include "BACnetBBMD.h"

//
//  BACnetBBMDFactory
//

class BACnetBBMDFactory : public BFRFactory {
    public:
        BACnetBBMDFactory( void );
        virtual ~BACnetBBMDFactory( void );

        virtual voidPtr StartElement( const char *name, const MinML::AttributeList& attrs );
        virtual void ChildElement( voidPtr ep, int id, voidPtr cp );
    };

typedef BACnetBBMDFactory *BACnetBBMDFactoryPtr;

extern BACnetBBMDFactory gBACnetBBMDFactory;

#endif
