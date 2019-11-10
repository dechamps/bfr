//
//  BACnetBBMD
//

#ifndef _BACnetBBMD_
#define _BACnetBBMD_

#include "BACnet.h"
#include "BACnetTask.h"

//
//  Array sizes
//

const int   kBACnetBBMDMaxBDTSize = 50;
const int   kBACnetBBMDMaxFDTSize = 50;
const int   kBACnetBBMDForeignTTLExt = 5;

//
//  Tags
//

// upstream tags
const unsigned int OriginalUnicastTag = (1 << 1);
const unsigned int OriginalBroadcastTag = (1 << 2);
const unsigned int ForwardedNPDUTag = (1 << 3);
const unsigned int ForeignDeviceTag = (1 << 4);

// downstream tags
const unsigned int NoForeignDevicesTag = (1 << 5);
const unsigned int NoPeersTag = (1 << 6);

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

class BACnetBBMD : public BACnetClient, public BACnetServer, public BACnetTask {

    public:
        bool            bbmdFDSupport;

        BACnetBBMD( unsigned long host, unsigned short port = kBACnetIPDefaultPort );
        BACnetBBMD( const BACnetAddress &addr );
        virtual ~BACnetBBMD( void );

        void AddForeignDevice( char *spec, int ttl );
        void DeleteForeignDevice( char *spec );

        void AddPeer( const char *spec );
        void DeletePeer( const char *spec );

    protected:
        BACnetAddress   bbmdAddress;
        int             bbmdLocalIndex;

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
