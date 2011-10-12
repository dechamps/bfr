//
//  BFRConsole
//

#ifndef _BFRConsole_
#define _BFRConsole_

#include "BACnet.h"
#include "BACnetPort.h"

#include "BFRParser.h"
#include "BFRRegistration.h"

//
//  BFRConsole
//

class BFRConsole : public BACnetPort, public BACnetClient {
        friend class BFRConsoleFactory;

    protected:
        char        *consolePrefix;

    public:
        BFRConsole( void );
        virtual ~BFRConsole( void );

        virtual void Indication( const BACnetPDU &pdu );    // override BACnetPort
        virtual void Confirmation( const BACnetPDU &pdu );  // can be a client

        void Send( const char *addr, const char *data );    // send as a client or server
        
        virtual void Init( void );                          // initialize as a port

        virtual void Read( void );                          // read data from stdin
        virtual void Write( void );                         // write data to stdout
    };

typedef BFRConsole *BFRConsolePtr;

extern BFRConsolePtr gBFRConsole;

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
