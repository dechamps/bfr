//
//  BACnetBIPForeign
//

#ifndef _BACnetBIPForeign
#define _BACnetBIPForeign

#include "BACnet.h"
#include "BACnetTask.h"

//
//  BACnetBIPForeign
//

class BACnetBIPForeign : public BACnetClient, public BACnetServer, public BACnetTask {
    public:
        int             foreignStatus;      // 0=dead, 1=alive
        int             foreignTimeToLive;  // time to live interval in seconds
        BACnetAddress   foreignBBMDAddr;    // address of BBMD to register with

        BACnetBIPForeign( void );
        virtual ~BACnetBIPForeign( void );

        void Register( void );
        void Register( unsigned long ipAddr, unsigned short port, int ttl );

        virtual void Indication( const BACnetPDU &pdu );
        virtual void Confirmation( const BACnetPDU &pdu );

        virtual void ProcessTask( void );
    };

typedef BACnetBIPForeign *BACnetBIPForeignPtr;

#endif
