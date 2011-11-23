//
//  BACnetDebug
//

#ifndef _BACnetDebug_
#define _BACnetDebug_

#include "BACnet.h"

class BACnetDebug : public BACnetClient, public BACnetServer {

    public:
        BACnetDebug( const char *prefix = 0, const bool newLine = false );
        virtual ~BACnetDebug( void );

        void SetPrefix( const char *prefix );

        virtual void Indication( const BACnetPDU &pdu );
        virtual void Confirmation( const BACnetPDU &pdu );

    protected:
        char        *debugPrefix;
        bool        debugNewLine;

    };

typedef BACnetDebug *BACnetDebugPtr;

#endif

