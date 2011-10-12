//
//  BFRDebug
//

#ifndef _BFRDebug_
#define _BFRDebug_

#include "BACnet.h"

#include "BFRParser.h"
#include "BFRRegistration.h"

//
//  BFRDebug
//

class BFRDebug : public BACnetClient, public BACnetServer {
        friend class BFRDebugFactory;

    protected:
        char        *debugPrefix;

    public:
        BFRDebug( void );
        virtual ~BFRDebug( void );

        virtual void Indication( const BACnetPDU &pdu );
        virtual void Confirmation( const BACnetPDU &pdu );
    };

typedef BFRDebug *BFRDebugPtr;

//
//  BFRDebugFactory
//

class BFRDebugFactory : public BFRFactory {
    public:
        virtual voidPtr StartElement( const char *name, const MinML::AttributeList& attrs );
    };

typedef BFRDebugFactory *BFRDebugFactoryPtr;

extern BFRDebugFactory gBFRDebugFactory;

#endif
