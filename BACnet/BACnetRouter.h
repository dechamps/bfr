//
//  BACnetRouter
//

#ifndef _BACnetRouter_
#define _BACnetRouter_

#include "BACnet.h"
#include "BACnetTask.h"

//  Forward Declarations

class BACnetRouter;
typedef BACnetRouter *BACnetRouterPtr;

class BACnetRouterAdapter;
typedef BACnetRouterAdapter *BACnetRouterAdapterPtr;

//
//  BACnetRouterAdapter
//

class BACnetRouterAdapter : public BACnetClient {
    public:
        int                         adapterNet;
        BACnetRouterPtr             adapterRouter;

        BACnetRouterAdapter( void );
        BACnetRouterAdapter( int net, BACnetRouterPtr router );
        virtual ~BACnetRouterAdapter( void );

        virtual void Indication( const BACnetNPDU &npdu );
        virtual void Confirmation( const BACnetPDU &pdu );
    };

//
//	BACnetRouterList
//

struct BACnetRouterList {
    int                     routerNet;          // -1 == local network
    int                     routerStatus;       // 0 == normal, 1 == unreachable, 2 == busy
    BACnetAddress           routerAddr;         // LAN specific address of router (localStationAddr)
    BACnetRouterAdapterPtr  routerAdapter;      // which adapter to use
    };

//
//	BACnetRouter
//

const int kBACnetRouterMaxAdapters = 8;         // how many connected adapters
const int kBACnetRouterMaxRouterListLen = 50;   // how many networks
const int kBACnetRouterLocalNetwork = -1;       // local network, pass to BindToNetwork

class BACnetRouter : public BACnetServer {
        friend class BACnetRouterAdapter;
        friend class BACnetMLAN;
        friend class BACnetRouterFactory;

    protected:
        BACnetRouterAdapterPtr  adapterList[kBACnetRouterMaxAdapters];
        int                     adapterListLen;
        BACnetRouterList        routerList[kBACnetRouterMaxRouterListLen];
        int                     routerListLen;

        virtual bool FilterNPDU( BACnetNPDU &npdu, BACnetRouterAdapterPtr srcAdapter,  BACnetRouterAdapterPtr destAdapter );
        void ProcessNPDU( BACnetRouterAdapterPtr adapter, BACnetNPDU &npdu );

        virtual void ProcessVendorNetMessage( BACnetRouterAdapterPtr adapter, const BACnetNPDU &npdu );
        void ProcessNetMessage(	BACnetRouterAdapterPtr adapter, const BACnetNPDU &npdu );

        void BroadcastRoutingTable( BACnetRouterAdapterPtr adapter );
        void BroadcastIAmRouterToNetwork( BACnetRouterAdapterPtr adapter, int *netList, int netListLen );
        void SendRejectMessageToNetwork( BACnetRouterAdapterPtr adapter, const BACnetNPDU &npdu, int rejectReason );

        void Indication( const BACnetPDU &pdu );    // message from app

    public:
        BACnetRouter( void );
        virtual ~BACnetRouter( void );

        int                 deviceLocalNetwork;     // net number of "local" net, -1 == not routing
        BACnetAddress       deviceLocalAddress;     // address on the network
        bool                dynamicRouting;         // true iff routes are added dynamically

        void SetLocalAddress( int net, const BACnetAddress &addr );

        void AddAdapter( BACnetRouterAdapterPtr rap );

        void BindToEndpoint( BACnetServerPtr endp, int net );
        void UnbindFromEndpoint( BACnetServerPtr endp );

        void AddRoute( int snet, int dnet, const BACnetAddress &addr );

        void BroadcastRoutingTables( void );
    };

//
//  BACnetRouterBroadcastRoutingTablesTask
//

class BACnetRouterBroadcastRoutingTablesTask : public BACnetTask {
    public:
        BACnetRouterPtr     taskRouter;

        BACnetRouterBroadcastRoutingTablesTask( BACnetRouterPtr rp, int delay = 0 )
            : taskRouter(rp), BACnetTask( oneShotDeleteTask, delay )
        {
            InstallTask();
        }

        void ProcessTask( void )
        {
            taskRouter->BroadcastRoutingTables();
        }
    };

typedef BACnetRouterBroadcastRoutingTablesTask *BACnetRouterBroadcastRoutingTablesTaskPtr;

#endif
