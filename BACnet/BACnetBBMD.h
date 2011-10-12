//
//  BACnetBBMD
//

#ifndef _BACnetBBMD
#define _BACnetBBMD

#include "BACnet.h"
#include "BACnetTask.h"

//
//  BACnet Broadcast Distribution Table Entry
//

struct BACnetBDTEntry {
    BACnetAddress   bdtAddress;
    unsigned long   bdtIPAddr;
    unsigned short  bdtPort;
    unsigned long   bdtMask;
    };

//
//  BACnet Broadcast Distribution Table Entry
//

struct BACnetFDTEntry {
    BACnetAddress   fdAddress;
    unsigned long   fdIPAddr;
    unsigned short  fdPort;
    unsigned short  fdTTL;
    unsigned short  fdRemain;
    };

//
//  BACnetBBMD
//

const int   kBACnetBBMDMaxBDTSize = 50;
const int   kBACnetBBMDMaxFDTSize = 50;
const int   kBACnetBBMDForeignTTLExt = 5;

class BACnetBBMD : public BACnetClient, public BACnetServer, public BACnetTask {
        friend class BACnetBBMDFactory;

    public:
        BACnetBBMD( void );
        BACnetBBMD( const BACnetAddress &addr );
        virtual ~BACnetBBMD( void );

        void AddForeignDevice( char *spec, int ttl );
        void DeleteForeignDevice( char *spec );

        void AddPeer( char *spec );
        void DeletePeer( char *spec );

    protected:
        BACnetAddress   bbmdAddress;
        int             bbmdLocalIndex;
        bool            bbmdFDSupport;

        BACnetBDTEntry  bbmdBDT[kBACnetBBMDMaxBDTSize];
        int             bbmdBDTSize;
        BACnetFDTEntry  bbmdFDT[kBACnetBBMDMaxFDTSize];
        int             bbmdFDTSize;

        int RegisterForeignDevice( const BACnetAddress &addr, int ttl );

        virtual void Indication( const BACnetPDU &pdu );
        virtual void Confirmation( const BACnetPDU &pdu );

        virtual void ProcessTask( void );
    };

typedef BACnetBBMD *BACnetBBMDPtr;

#endif
