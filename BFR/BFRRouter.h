//
//  BFRRouter
//

#ifndef _BFRRouter_
#define _BFRRouter_

#include "BFRParser.h"
#include "BACnetRouter.h"

//
//  BACnetRouterAdapterPeerFactory
//

class BACnetRouterAdapterPeerFactory : public BFRFactory {
    public:
        virtual voidPtr StartElement( const char *name, const MinML::AttributeList& attrs );
    };

typedef BACnetRouterAdapterPeerFactory *BACnetRouterAdapterPeerFactoryPtr;

extern BACnetRouterAdapterPeerFactory gBACnetRouterAdapterPeerFactory;

//
//  BACnetRouterAdapterFactory
//

class BACnetRouterAdapterFactory : public BFRFactory {
    public:
        BACnetRouterAdapterFactory( void );
        virtual ~BACnetRouterAdapterFactory( void );

        virtual voidPtr StartElement( const char *name, const MinML::AttributeList& attrs );
        virtual void ChildElement( voidPtr ep, int id, voidPtr cp );
    };

typedef BACnetRouterAdapterFactory *BACnetRouterAdapterFactoryPtr;

extern BACnetRouterAdapterFactory gBACnetRouterAdapterFactory;

//
//  BACnetRouterFactory
//

class BACnetRouterFactory : public BFRFactory {
    public:
        BACnetRouterFactory( void );
        virtual ~BACnetRouterFactory( void );

        virtual voidPtr StartElement( const char *name, const MinML::AttributeList& attrs );
        virtual void ChildElement( voidPtr ep, int id, voidPtr cp );
    };

typedef BACnetRouterFactory *BACnetRouterFactoryPtr;

extern BACnetRouterFactory gBACnetRouterFactory;

#endif
