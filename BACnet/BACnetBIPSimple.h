//
//  BACnetBIPSimple
//

#ifndef _BACnetBIPSimple_
#define _BACnetBIPSimple_

#include "BACnet.h"

//
//  BACnetBIPSimple
//

class BACnetBIPSimple : public BACnetClient, public BACnetServer {
    public:
        virtual void Indication( const BACnetPDU &pdu );
        virtual void Confirmation( const BACnetPDU &pdu );
    };

typedef BACnetBIPSimple *BACnetBIPSimplePtr;

#endif
