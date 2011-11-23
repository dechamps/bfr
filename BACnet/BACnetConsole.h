//
//  BACnetConsole
//

#ifndef _BACnetConsole_
#define _BACnetConsole_

#include "BACnet.h"
#include "BACnetPort.h"

//
//  BACnetConsole
//

class BACnetConsole : public BACnetPort, public BACnetClient {

    public:
        BACnetConsole( void );
        virtual ~BACnetConsole( void );

        void SetPrefix(const char *prefix);                 // change the prefix

        virtual void Indication( const BACnetPDU &pdu );    // override BACnetPort
        virtual void Confirmation( const BACnetPDU &pdu );  // can be a client

        void Send( const char *addr, const char *data );    // send as a client or server

        virtual void Init( void );                          // initialize as a port

        virtual void Read( void );                          // read data from stdin
        virtual void Write( void );                         // write data to stdout

    protected:
        char        *consolePrefix;

    };

typedef BACnetConsole *BACnetConsolePtr;

extern BACnetConsolePtr gBACnetConsole;

#endif

