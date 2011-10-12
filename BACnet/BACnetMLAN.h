//
//  BACnetMLAN
//

#ifndef _BACnetMLAN_
#define _BACnetMLAN_

#include "BACnet.h"
#include "BACnetRouter.h"

//  Forward Declarations

class BFRAddressFilter;

class BACnetMLANFilterElem;
typedef BACnetMLANFilterElem *BACnetMLANFilterElemPtr;

//
//  BACnetMLANFilterElem
//

class BACnetMLANFilterElem {
    public:
        bool                    filterAccept;       // true iff 'accept' filter element
        BACnetMLANFilterElemPtr filterNext;         // next filter in list

        BACnetMLANFilterElem( void );

        BFRAddressFilter        filterAddress;      // source

        int Test( const BACnetAddress &addr );      // -1=reject, 0=no decision, 1=accept
    };

//
//  BACnetMLANStatic
//

struct BACnetMLANStatic {
    BACnetAddress       staticAddress;
    int                 staticNode;
    };

typedef BACnetMLANStatic *BACnetMLANStaticPtr;

//  Forward Declarations

class BACnetMLANAdapter;
typedef BACnetMLANAdapter *BACnetMLANAdapterPtr;

class BACnetMLAN;
typedef BACnetMLAN *BACnetMLANPtr;

//
//  BACnetMLANAddr
//

struct BACnetMLANAddr {
    BACnetAddress           mlanAddr;           // LAN specific address of router (localStationAddr)
    BACnetMLANAdapterPtr    mlanAdapter;        // which adapter to use
    };

//
//  BACnetMLANAdapter
//

class BACnetMLANAdapter : public BACnetRouterAdapter {
        friend class BACnetMLAN;
        friend class BACnetMLANAdapterFactory;

    protected:
        BACnetMLANAdapterPtr    mlanNext;       // next in list
        BACnetMLANFilterElemPtr mlanFilter;     // first in filter element list

    public:
        BACnetMLANAdapter( void );
        virtual ~BACnetMLANAdapter( void );

        BACnetMLANPtr           mlanHost;       // install LAN

        virtual void Indication( const BACnetNPDU &npdu );
        virtual void Confirmation( const BACnetPDU &pdu );
    };

//
//  BACnetMLAN
//

const int kBACnetMLANAddrListSize = 50;

class BACnetMLAN {
        friend class BACnetMLANAdapter;
        friend class BACnetMLANAdapterFactory;

    protected:
        BACnetMLANAddr          mlanAddrList[kBACnetMLANAddrListSize];
        int                     mlanAddrLen;

        BACnetMLANAdapterPtr    mlanAdapterList;        // list of bound adapters
        BACnetMLANPtr           mlanNext;               // list of MLANs

        void MapAddr( BACnetMLANAdapterPtr ap, const BACnetAddress &addr, int n );
        int FindAndMapAddr( BACnetMLANAdapterPtr ap, const BACnetAddress &addr );
        void ProcessMessage( BACnetMLANAdapterPtr ap, const BACnetNPDU &npdu );	// process a packet

    public:
        BACnetMLAN( void );
        ~BACnetMLAN( void );

        char                    *mlanName;              // MLAN name

        void AddAdapter( BACnetMLANAdapterPtr np );     // add a node to the list
        void RemoveAdapter( BACnetMLANAdapterPtr np );  // remove a node from the list
    };

extern BACnetMLANPtr gBACnetMLANList;

#endif
