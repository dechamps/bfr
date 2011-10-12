//
//  BFRVLAN
//

#ifndef _BFRVLAN_
#define _BFRVLAN_

#include "BACnetVLAN.h"
#include "BFRParser.h"

//
//  BACnetVLANNodeFactory
//

class BACnetVLANNodeFactory : public BFRFactory {
    public:
        virtual voidPtr StartElement( const char *name, const MinML::AttributeList& attrs );
    };

typedef BACnetVLANNodeFactory *BACnetVLANNodeFactoryPtr;

extern BACnetVLANNodeFactory gBACnetVLANNodeFactory;

//
//  BACnetVLANFactory
//

class BACnetVLANFactory : public BFRFactory {
    public:
        BACnetVLANFactory( void );

        virtual voidPtr StartElement( const char *name, const MinML::AttributeList& attrs );
        virtual void ChildElement( voidPtr ep, int id, voidPtr cp );
    };

typedef BACnetVLANFactory *BACnetVLANFactoryPtr;

extern BACnetVLANFactory gBACnetVLANFactory;

#endif
