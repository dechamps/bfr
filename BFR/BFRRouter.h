//
//  BFRRouter
//

#ifndef _BFRRouter_
#define _BFRRouter_

#include "BFRParser.h"
#include "BACnetRouter.h"

const int kBFRRouterAdapterMaxPeerNetListLen = 50;

//
//  BFRRouterAdapterPeer
//

struct BFRRouterAdapterPeer {
    int             peerNetList[kBFRRouterAdapterMaxPeerNetListLen];
    int             peerNetListLen;
    BACnetAddress   peerAddress;
    };

typedef BFRRouterAdapterPeer *BFRRouterAdapterPeerPtr;

//
//  BFRRouterAdapterPeerFactory
//

class BFRRouterAdapterPeerFactory : public BFRFactory {
    public:
        virtual voidPtr StartElement( const char *name, const MinML::AttributeList& attrs );
    };

typedef BFRRouterAdapterPeerFactory *BFRRouterAdapterPeerFactoryPtr;

extern BFRRouterAdapterPeerFactory gBFRRouterAdapterPeerFactory;

//
//  BFRRouterAdapterFactory
//

class BFRRouterAdapterFactory : public BFRFactory {
    public:
        BFRRouterAdapterFactory( void );
        virtual ~BFRRouterAdapterFactory( void );

        virtual voidPtr StartElement( const char *name, const MinML::AttributeList& attrs );
        virtual void ChildElement( voidPtr ep, int id, voidPtr cp );
    };

typedef BFRRouterAdapterFactory *BFRRouterAdapterFactoryPtr;

extern BFRRouterAdapterFactory gBFRRouterAdapterFactory;

//
//  BFRRouterFactory
//

class BFRRouterFactory : public BFRFactory {
    public:
        BFRRouterFactory( void );
        virtual ~BFRRouterFactory( void );

        virtual voidPtr StartElement( const char *name, const MinML::AttributeList& attrs );
        virtual void ChildElement( voidPtr ep, int id, voidPtr cp );
    };

typedef BFRRouterFactory *BFRRouterFactoryPtr;

extern BFRRouterFactory gBFRRouterFactory;

#endif
