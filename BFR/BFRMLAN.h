//
//  BFRMLAN
//

#ifndef _BFRMLAN_
#define _BFRMLAN_

#include "BFRParser.h"
#include "BACnetMLAN.h"

//
//  BACnetMLANFilterElemFactory
//
//  This class returns a filter element for Accept and Reject elements.
//

class BACnetMLANFilterElemFactory : public BFRFactory {
    public:
        virtual voidPtr StartElement( const char *name, const MinML::AttributeList& attrs );
    };

typedef BACnetMLANFilterElemFactory *BACnetMLANFilterElemFactoryPtr;

extern BACnetMLANFilterElemFactory gBACnetMLANFilterElemFactory;

//
//  BACnetMLANStaticFactory
//

class BACnetMLANStaticFactory : public BFRFactory {
    public:
        virtual voidPtr StartElement( const char *name, const MinML::AttributeList& attrs );
    };

typedef BACnetMLANStaticFactory *BACnetMLANStaticFactoryPtr;

extern BACnetMLANStaticFactory gBACnetMLANStaticFactory;

//
//  BACnetMLANAdapterFactory
//

class BACnetMLANAdapterFactory : public BFRFactory {
    public:
        BACnetMLANAdapterFactory( void );

        virtual voidPtr StartElement( const char *name, const MinML::AttributeList& attrs );
        virtual void ChildElement( voidPtr ep, int id, voidPtr cp );
    };

typedef BACnetMLANAdapterFactory *BACnetMLANAdapterFactoryPtr;

extern BACnetMLANAdapterFactory gBACnetMLANAdapterFactory;

//
//  BACnetMLANFactory
//

class BACnetMLANFactory : public BFRFactory {
    public:
        virtual voidPtr StartElement( const char *name, const MinML::AttributeList& attrs );
    };

typedef BACnetMLANFactory *BACnetMLANFactoryPtr;

extern BACnetMLANFactory gBACnetMLANFactory;

#endif
