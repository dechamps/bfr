//
//  BACnetAddressPool
//

#ifndef _BACnetAddressPool_
#define _BACnetAddressPool_

#include "BACnet.h"

//
//  BACnetAddrPoolElem
//
//  Subclass the pool element to create objects that can be added to 
//  a pool.
//

struct BACnetAddrPoolElem {
    BACnetAddress               elemAddress;
    BACnetAddrPoolElem          *elemNext;
    };

typedef BACnetAddrPoolElem *BACnetAddrPoolElemPtr;

//
//	BACnetAddressPool
//

class BACnetAddressPool {
        friend class BACnetAddressPoolIter;

    protected:
        BACnetAddrPoolElemPtr   poolTable;

    public:
        BACnetAddressPool( void );

        void AddElem( BACnetAddrPoolElemPtr ep );
        void RemoveElem( BACnetAddrPoolElemPtr ep );

        BACnetAddrPoolElemPtr operator []( const BACnetAddress &addr );
    };

typedef BACnetAddressPool *BACnetAddressPoolPtr;

//
//	BACnetAddressPoolIter
//

class BACnetAddressPoolIter {
    protected:
        BACnetAddrPoolElemPtr   iterElem;       // current element
        BACnetAddrPoolElemPtr   iterNext;       // next element

    public:
        BACnetAddressPoolIter( const BACnetAddressPool &pool );

        BACnetAddrPoolElemPtr operator *( void );

        void Next( void );
    };

typedef BACnetAddressPoolIter *BACnetAddressPoolIterPtr;

#endif
