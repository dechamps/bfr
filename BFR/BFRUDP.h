//
//  BFRUDP
//

#ifndef _BFRUDP_
#define _BFRUDP_

#include "BACnet.h"
#include "BACnetPort.h"

#include "BFRParser.h"
#include "BFRRegistration.h"

//
//  BFRUDP
//

class BFRUDP : public BACnetPort {
        friend class BFRUDPFactory;
        friend class BFRUDPBroadcastListener;

    protected:
        unsigned long   ipHost;
        unsigned short  ipPort;

    public:
        BFRUDP( void );
        virtual ~BFRUDP( void );

        virtual void Init( void );                  // initialize

        virtual void Read( void );                  // read data from socket
        virtual void Write( void );                 // write data to socket
    };

typedef BFRUDP *BFRUDPPtr;

//
//  BFRUDPBroadcastListener
//

class BFRUDPBroadcastListener : public BACnetPort {
        friend class BFRUDPFactory;

    protected:
        BFRUDPPtr       ipPeer;
        unsigned long   ipHost;
        unsigned short  ipPort;

    public:
        BFRUDPBroadcastListener( BFRUDPPtr peer );
        virtual ~BFRUDPBroadcastListener( void );

        virtual void Init( void );                  // initialize

        virtual void Read( void );                  // read data from socket
        virtual void Write( void );                 // write data to socket
    };

typedef BFRUDPBroadcastListener *BFRUDPBroadcastListenerPtr;

//
//  BFRUDPFactory
//

class BFRUDPFactory : public BFRFactory {
    public:
        virtual voidPtr StartElement( const char *name, const MinML::AttributeList& attrs );
    };

typedef BFRUDPFactory *BFRUDPFactoryPtr;

extern BFRUDPFactory gBFRUDPFactory;

#endif
