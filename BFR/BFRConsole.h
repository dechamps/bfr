//
//  BFRConsole
//

#ifndef _BFRConsole_
#define _BFRConsole_

#include "BACnet.h"
#include "BACnetPort.h"
#include "BACnetConsole.h"

#include "BFRParser.h"
#include "BFRRegistration.h"

//
//	BFRConsoleFactory
//

class BFRConsoleFactory : public BFRFactory {
    public:
        virtual voidPtr StartElement( const char *name, const MinML::AttributeList& attrs );
    };

typedef BFRConsoleFactory *BFRConsoleFactoryPtr;

extern BFRConsoleFactory gBFRConsoleFactory;

#endif
