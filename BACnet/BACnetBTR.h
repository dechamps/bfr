//
//  BACnetBTR
//

#ifndef _BACnetBTR_
#define _BACnetBTR_

#include "BACnet.h"

//
//  BACnetBTR
//
//  This object is a client of an IP endpoint and a server to a BACnetRouter.
//  It provides a "shim" layer for providing BTR functionality.
//

class BACnetBTR : public BACnetClient, public BACnetServer {
    protected:
        BACnetAddress   *btrPeer;
        int             btrPeerLen;

    public:
        BACnetBTR( void );
        virtual ~BACnetBTR( void );

        virtual void Indication( const BACnetPDU &pdu );
        virtual void Confirmation( const BACnetPDU &pdu );

        void AddPeer( const BACnetAddress &ipAddr );
        void DeletePeer( const BACnetAddress &ipAddr );
    };

typedef BACnetBTR *BACnetBTRPtr;

#endif
