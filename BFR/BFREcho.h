//
//  BFREcho
//

#ifndef _BFREcho_
#define _BFREcho_

#include "BACnet.h"

#include "BFRParser.h"
#include "BFRRegistration.h"

//
//  BFREcho
//

class BFREcho : public BACnetClient, public BACnetServer {
        friend class BFREchoFactory;

    public:
        virtual void Indication( const BACnetPDU &pdu );
        virtual void Confirmation( const BACnetPDU &pdu );
    };

typedef BFREcho *BFREchoPtr;

//
//  BFREchoFactory
//

class BFREchoFactory : public BFRFactory {
    public:
        virtual voidPtr StartElement( const char *name, const MinML::AttributeList& attrs );
    };

typedef BFREchoFactory *BFREchoFactoryPtr;

extern BFREchoFactory gBFREchoFactory;

#endif
