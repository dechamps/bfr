//
//  BFRSwitch
//

#ifndef _BFRSwitch_
#define _BFRSwitch_

#include "BACnet.h"
#include "BACnetAddressPool.h"

#include "BFRFilter.h"
#include "BFRParser.h"

//  Forward Declarations

class BFRSwitch;
typedef BFRSwitch *BFRSwitchPtr;

class BFRSwitchPort;
typedef BFRSwitchPort *BFRSwitchPortPtr;

//
//  BFRSwitch
//

class BFRSwitch {
    protected:
        BFRSwitchPortPtr    switchPorts;
        BACnetAddressPool   switchPool;

    public:
        BFRFilterSet        switchFilter;

        BFRSwitch( void );
        ~BFRSwitch( void );

        void AddPort( BFRSwitchPortPtr spp );
        void RemovePort( BFRSwitchPortPtr spp );

        void ProcessPacket( BFRSwitchPortPtr spp, const BACnetPDU &pdu );
    };

//
//  BFRSwitchFactory
//

class BFRSwitchFactory : public BFRFactory {
    public:
        BFRSwitchFactory( void );

        virtual voidPtr StartElement( const char *name, const MinML::AttributeList& attrs );
        virtual void ChildElement( voidPtr ep, int id, voidPtr cp );
    };

typedef BFRSwitchFactory *BFRSwitchFactoryPtr;

extern BFRSwitchFactory gBFRSwitchFactory;

//
//  BFRSwitchPort
//

class BFRSwitchPort : public BACnetClient {
        friend class BFRSwitch;

    protected:
        BFRSwitchPortPtr    portNext;

    public:
        BFRSwitchPtr        portSwitch;

        BFRSwitchPort( void );
        virtual ~BFRSwitchPort( void );

        virtual void Confirmation( const BACnetPDU &pdu );
    };

//
//  BFRSwitchPortFactory
//

class BFRSwitchPortFactory : public BFRFactory {
    public:
        virtual voidPtr StartElement( const char *name, const MinML::AttributeList& attrs );
    };

typedef BFRSwitchPortFactory *BFRSwitchPortFactoryPtr;

extern BFRSwitchPortFactory gBFRSwitchPortFactory;

#endif
