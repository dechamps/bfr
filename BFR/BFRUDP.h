//
//  BFRUDP
//

#ifndef _BFRUDP_
#define _BFRUDP_

#include "BACnet.h"
#include "BACnetPort.h"
#include "BACnetUDP.h"

#include "BFRParser.h"
#include "BFRRegistration.h"

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
