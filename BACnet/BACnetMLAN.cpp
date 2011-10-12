//
//  BACnetMLAN
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "BACnetMLAN.h"

//
//  BACnetMLANFilterElem::BACnetMLANFilterElem
//

BACnetMLANFilterElem::BACnetMLANFilterElem( void )
    : filterNext(0), filterAccept(false)
    , filterAddress()
{
}

//
//  BACnetMLANFilterElem::Test
//

int BACnetMLANFilterElem::Test( const BACnetAddress &addr )
{
    // if the filter has been specified but there is no match, return no decision
    if (filterAddress.filterType && !filterAddress.Test(addr))
        return 0;

    // return accept or failure
    return (filterAccept ? 1 : -1);
}

//
//  BACnetMLANAdapter::BACnetMLANAdapter
//

BACnetMLANAdapter::BACnetMLANAdapter( void )
    : mlanHost(0), mlanNext(0), mlanFilter(0)
{
}

//
//  BACnetMLANAdapter::~BACnetMLANAdapter
//

BACnetMLANAdapter::~BACnetMLANAdapter( void )
{
    // if bound to an MLAN, remove it
    if (mlanHost)
        mlanHost->RemoveAdapter( this );
}

//
//  BACnetMLANAdapter::Indication
//

void BACnetMLANAdapter::Indication( const BACnetNPDU &npdu )
{
    int                         stat
    ;
    BACnetMLANFilterElemPtr     fep
    ;

    // check to see that we are bound to a MLAN
    if (!mlanHost)
        throw_1(99001);		// unbound

    // check the filters
    for (fep = mlanFilter; fep; fep = fep->filterNext) {
        stat = fep->Test( npdu.npduSADR );
        if (stat < 0)
            return;
        else
        if (stat > 0)
            break;
    }

    // forward it along
    mlanHost->ProcessMessage( this, npdu );
}

//
//  BACnetMLANAdapter::Confirmation
//

void BACnetMLANAdapter::Confirmation( const BACnetPDU &pdu )
{
    throw_1(99002);     // should never be called
}

//
//  BACnetMLAN::BACnetMLAN
//

BACnetMLANPtr gBACnetMLANList = 0;

BACnetMLAN::BACnetMLAN( void )
    : mlanAdapterList(0), mlanName(0), mlanAddrLen(0)
{
    mlanNext = gBACnetMLANList;
    gBACnetMLANList = this;

    // clear the address list
    for (int i = 0; i < kBACnetMLANAddrListSize; i++)
        mlanAddrList[i].mlanAdapter = 0;
}

//
//  BACnetMLAN::~BACnetMLAN
//

BACnetMLAN::~BACnetMLAN( void )
{
    BACnetMLANAdapterPtr    map, next
    ;
    BACnetMLANPtr           *mpp
    ;

    // remove the adapters from the MLAN
    for (map = mlanAdapterList; map; map = next ) {
        next = map->mlanNext;

        map->mlanHost = 0;
        map->mlanNext = 0;
    }

    // delete the name
    if (mlanName)
        delete[] mlanName;

    // remove this node from the list of MLANs
    for (mpp = &gBACnetMLANList; *mpp; mpp = &(*mpp)->mlanNext)
        if (*mpp == this) {
            *mpp = mlanNext;
            break;
        }
}

//
//  BACnetMLAN::AddAdapter
//

void BACnetMLAN::AddAdapter( BACnetMLANAdapterPtr map )
{
    // link it in
    map->mlanNext = mlanAdapterList;
    mlanAdapterList = map;

    // bind it
    map->mlanHost = this;
}

//
//  BACnetMLAN::RemoveAdapter
//

void BACnetMLAN::RemoveAdapter( BACnetMLANAdapterPtr map )
{
    BACnetMLANAdapterPtr    *mapp
    ;

    // remove the nodes from the MLAN
    for (mapp = &mlanAdapterList; *mapp; mapp = &(*mapp)->mlanNext )
        if (*mapp == map) {
            // link over it
            (*mapp)->mlanNext = map->mlanNext;

            // unbind
            map->mlanHost = 0;
            map->mlanNext = 0;
            break;
        }

    // disassociate the nodes
    for (int i = 0; i < mlanAddrLen; i++)
        if (mlanAddrList[i].mlanAdapter == map)
            mlanAddrList[i].mlanAdapter = 0;
}

//
//  BACnetMLAN::MapAddr
//

void BACnetMLAN::MapAddr( BACnetMLANAdapterPtr map, const BACnetAddress &addr, int n )
{
    // make we are not out of bounds
    if (n >= kBACnetMLANAddrListSize)
        throw_1(99005);		// invalid in available address space

    // make sure it is not already allocated
    if (mlanAddrList[n].mlanAdapter)
        throw_1(99007);		// address already mapped

    // map it
    mlanAddrList[n].mlanAdapter = map;
    mlanAddrList[n].mlanAddr = addr;

    // push up the node count if necessary
    if (n >= mlanAddrLen)
        mlanAddrLen = n + 1;
}

//
//  BACnetMLAN::FindAndMapAddr
//

int BACnetMLAN::FindAndMapAddr( BACnetMLANAdapterPtr map, const BACnetAddress &addr )
{
    int     i
    ;

    // find a match
    for (i = 0; i < mlanAddrLen; i++)
        if ((mlanAddrList[i].mlanAdapter == map) && (mlanAddrList[i].mlanAddr == addr))
            return i;

    // look for an open slot
    for (i = 0; i < mlanAddrLen; i++)
        if (!mlanAddrList[i].mlanAdapter) {
            mlanAddrList[i].mlanAdapter = map;
            mlanAddrList[i].mlanAddr = addr;
            return i;
        }

    // make sure there's enough space
    if (i >= kBACnetMLANAddrListSize)
        throw_1(99006);     // out of address space

    // add a new address
    mlanAddrList[i].mlanAdapter = map;
    mlanAddrList[i].mlanAddr = addr;
    mlanAddrLen += 1;

    // return the new one
    return i;
}

//
//  BACnetMLAN::ProcessMessage
//

void BACnetMLAN::ProcessMessage( BACnetMLANAdapterPtr map, const BACnetNPDU &npdu )
{
    BACnetMLANAdapterPtr    adapter
    ;
    BACnetOctet             sadr, dadr
    ;
    BACnetNPDU              xpdu
    ;

    // throw out network layer messages
    if (npdu.npduNetMessage >= 0)
        return;

    // map the destination into the network header and pdu destination
    switch (npdu.npduDADR.addrType) {
        case nullAddr:
        case localStationAddr:
        case localBroadcastAddr:
            throw_1(99006);         // code problem
            break;

        case remoteStationAddr:
            if (npdu.npduDADR.addrNet != map->adapterNet)
                return;
            break;

        case remoteBroadcastAddr:
            if (npdu.npduDADR.addrNet != map->adapterNet)
                return;
        case globalBroadcastAddr:
            break;
    }

    // find and map the source address
    sadr = FindAndMapAddr( map, npdu.npduSADR );

    // build a replacement npdu
    xpdu.pduSource.LocalStation( sadr );
    xpdu.pduExpectingReply = npdu.pduExpectingReply;
    xpdu.pduNetworkPriority = npdu.pduNetworkPriority;
    xpdu.SetReference( npdu );

    if (npdu.npduDADR.addrType == remoteStationAddr) {
        // give it to someone specific
        dadr = npdu.npduDADR.addrAddr[0];
        adapter = mlanAddrList[dadr].mlanAdapter;
        if ((dadr < mlanAddrLen) && adapter && (adapter != map)) {
            xpdu.npduSADR.RemoteStation( adapter->adapterNet, sadr );
            xpdu.npduDADR = mlanAddrList[dadr].mlanAddr;
            adapter->adapterRouter->ProcessNPDU( adapter, xpdu );
        }
    } else {
        // give it to everyone except the adapter it came from
        for (int i = 0; i < mlanAddrLen; i++) {
            adapter = mlanAddrList[i].mlanAdapter;
            if (adapter && (adapter != map)) {
                xpdu.npduSADR.RemoteStation( adapter->adapterNet, sadr );
                xpdu.npduDADR = mlanAddrList[i].mlanAddr;
                adapter->adapterRouter->ProcessNPDU( adapter, xpdu );
            }
        }
    }
}
