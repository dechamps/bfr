//
//  BACnetUDP
//

#ifndef _BACnetUDP_
#define _BACnetUDP_

#include "BACnet.h"
#include "BACnetPort.h"

//
//  BACnetUDP
//

class BACnetUDP : public BACnetPort {
        friend class BACnetUDPBroadcastListener;

    public:
        unsigned long   ipHost;
        unsigned short  ipPort;

        BACnetUDP( void );
        BACnetUDP( const char *addr );
        virtual ~BACnetUDP( void );

        virtual void Init( void );                  // initialize

        virtual void Read( void );                  // read data from socket
        virtual void Write( void );                 // write data to socket
    };

typedef BACnetUDP *BACnetUDPPtr;

//
//  BACnetUDPBroadcastListener
//

class BACnetUDPBroadcastListener : public BACnetPort {
        friend class BACnetUDP;

    public:
        unsigned long   ipHost;
        unsigned short  ipPort;

        BACnetUDPBroadcastListener( BACnetUDPPtr peer );
        virtual ~BACnetUDPBroadcastListener( void );

        virtual void Init( void );                  // initialize

        virtual void Read( void );                  // read data from socket
        virtual void Write( void );                 // write data to socket

    protected:
        BACnetUDPPtr       ipPeer;

    };

typedef BACnetUDPBroadcastListener *BACnetUDPBroadcastListenerPtr;

#endif
