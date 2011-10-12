//
//  BFREthernet
//

#ifndef _BFREthernet_
#define _BFREthernet_

#include "BACnet.h"
#include "BACnetPort.h"

#include "BFRParser.h"
#include "BFRRegistration.h"

//
//  BFREthernet
//

class BFREthernet : public BACnetPort {
        friend class BFREthernetFactory;

    protected:
        char            *ethDevice;                 // device name, i.e., "eth0"
        bool            ethPromiscuous;             // all packets
        
    public:
        BFREthernet( void );
        virtual ~BFREthernet( void );
        
        virtual void Init( void );                  // initialize
        
        virtual void Read( void );                  // read data from socket
        virtual void Write( void );                 // write data to socket
    };

typedef BFREthernet *BFREthernetPtr;

//
//  BFREthernetFactory
//

class BFREthernetFactory : public BFRFactory {
    public:
        virtual voidPtr StartElement( const char *name, const MinML::AttributeList& attrs );
    };

typedef BFREthernetFactory *BFREthernetFactoryPtr;

extern BFREthernetFactory gBFREthernetFactory;

#endif
