//
//  BACnetAddressPool
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "BACnetAddressPool.h"

//
//  BACnetAddressPool::BACnetAddressPool
//

BACnetAddressPool::BACnetAddressPool( void )
    : poolTable(0)
{
}

//
//  BACnetAddressPool::AddElem
//

void BACnetAddressPool::AddElem( BACnetAddrPoolElemPtr ep )
{
    // simple linked list
    ep->elemNext = poolTable;
    poolTable = ep;
}

//
//  BACnetAddressPool::RemoveElem
//

void BACnetAddressPool::RemoveElem( BACnetAddrPoolElemPtr ep )
{
    BACnetAddrPoolElemPtr   *pep
    ;

    for (pep = &poolTable; *pep; pep = &(*pep)->elemNext)
        if (*pep == ep) {
            *pep = ep->elemNext;
            ep->elemNext = 0;
            break;
        }
}

//
//  BACnetAddressPool::operator []
//

BACnetAddrPoolElemPtr BACnetAddressPool::operator []( const BACnetAddress &addr )
{
    BACnetAddrPoolElemPtr   ep
    ;

    // search the list
    for (ep = poolTable; ep; ep = ep->elemNext)
        if (ep->elemAddress == addr)
            return ep;

    // not there
    return 0;
}

//
//  BACnetAddressPoolIter::BACnetAddressPoolIter
//

BACnetAddressPoolIter::BACnetAddressPoolIter( const BACnetAddressPool &pool )
{
    iterElem = pool.poolTable;
    iterNext = (iterElem ? iterElem->elemNext : 0);
}

//
//  BACnetAddressPoolIter::operator *
//

BACnetAddrPoolElemPtr BACnetAddressPoolIter::operator *( void )
{
    return iterElem;
}

//
//  BACnetAddressPoolIter::Next
//

void BACnetAddressPoolIter::Next( void )
{
    iterElem = iterNext;
    iterNext = (iterElem ? iterElem->elemNext : 0);
}
