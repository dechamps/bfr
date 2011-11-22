//
//  BACnetRouter
//

#if _DEBUG_ROUTER
#include <stdio.h>
#endif

#include <stdlib.h>
#include <string.h>

#include "BACnet.h"
#include "BACnetRouter.h"

//
//  BACnetRouterAdapter::BACnetRouterAdapter
//

BACnetRouterAdapter::BACnetRouterAdapter( void )
    : adapterNet(0), adapterRouter(0)
{
#if _DEBUG_ROUTER
    printf("BACnetRouterAdapter::BACnetRouterAdapter(?)\n");
#endif
}

//
//  BACnetRouterAdapter::BACnetRouterAdapter
//

BACnetRouterAdapter::BACnetRouterAdapter( int net, BACnetRouterPtr router )
    : adapterNet(net), adapterRouter(router)
{
#if _DEBUG_ROUTER
    printf("BACnetRouterAdapter::BACnetRouterAdapter(%d)\n", net);
#endif
}

//
//  BACnetRouterAdapter::~BACnetRouterAdapter
//

BACnetRouterAdapter::~BACnetRouterAdapter( void )
{
#if _DEBUG_ROUTER
    printf("BACnetRouterAdapter::~BACnetRouterAdapter(%d)\n", adapterNet);
#endif
}

//
//  BACnetRouterAdapter::Indication
//
//  This function is called by BACnetRouter::ProcessNPDU or BACnetRouter::Indication
//  when there is a message that should be sent out.  It will encode the NPCI correctly 
//  and send it downstream.
//

void BACnetRouterAdapter::Indication( const BACnetNPDU &npdu )
{
    BACnetPDU   pdu
    ;

#if _DEBUG_ROUTER
    printf("BACnetRouterAdapter::Indication(%d)\n", adapterNet);
#endif

    Encode( pdu, npdu );
    Request( pdu );
}

//
//  BACnetRouterAdapter::Confirmation
//
//  This function is called when there is a message traveling up the stack.  It is 
//  decoded and then forwarded on to the router object for processing.
//

void BACnetRouterAdapter::Confirmation( const BACnetPDU &pdu )
{
    BACnetNPDU  npdu
    ;

#if _DEBUG_ROUTER
    printf("BACnetRouterAdapter::Confirmation(%d)\n", adapterNet);
#endif

    Decode( npdu, pdu );
    adapterRouter->ProcessNPDU( this, npdu );
}

//
//  BACnetRouter::BACnetRouter
//

BACnetRouter::BACnetRouter( void )
    : adapterListLen(0), routerListLen(0)
    , deviceLocalNetwork(kBACnetRouterLocalNetwork), deviceLocalAddress()
    , dynamicRouting(false)
{
#if _DEBUG_ROUTER
    printf("BACnetRouter::BACnetRouter\n");
#endif
}

//
//  BACnetRouter::~BACnetRouter
//

BACnetRouter::~BACnetRouter( void )
{
#if _DEBUG_ROUTER
    printf("BACnetRouter::~BACnetRouter\n");
#endif
}

//
//  BACnetRouter::SetLocalAddress
//

void BACnetRouter::SetLocalAddress( int net, const BACnetAddress &addr )
{
#if _DEBUG_ROUTER
    printf("BACnetRouter::SetLocalAddress %d %s\n", net, addr.ToString());
#endif

    deviceLocalNetwork = net;
    deviceLocalAddress = addr;
    deviceLocalAddress.addrType = remoteStationAddr;
    deviceLocalAddress.addrNet = net;
}

//
//  BACnetRouter::AddAdapter
//

void BACnetRouter::AddAdapter( BACnetRouterAdapterPtr rap )
{
    // tell the adapter to point back to this
    rap->adapterRouter = this;

    // add it to the list
    adapterList[adapterListLen++] = rap;
}

//
//  BACnetRouter::BindToEndpoint
//
//  Binding a router object to an endpoint needs an adapter to associate a network 
//  number with that endpoint.
//

void BACnetRouter::BindToEndpoint( BACnetServerPtr endp, int net )
{
    if (adapterListLen == kBACnetRouterMaxAdapters)
        throw_1(5002); // out of adapter space

    for (int i = 0; i < adapterListLen; i++)
        if (adapterList[i]->adapterNet == net)
            throw_1(5003); // already an adapter for this network

    // create a new adapter that refers back to this router
    BACnetRouterAdapterPtr	rap = new BACnetRouterAdapter( net, this );

    // add it to the list
    adapterList[adapterListLen++] = rap;

    // bind the adapter (client) to the endpoint (server)
    Bind( rap, endp );
}

//
//  BACnetRouter::UnbindFromEndpoint
//
//  Binding a router object to an endpoint needs an adapter to associate a network 
//  number with that endpoint.
//

void BACnetRouter::UnbindFromEndpoint( BACnetServerPtr endp )
{
    int     i, j, k
    ;

    // can we find it?
    for (i = 0; i < adapterListLen; i++)
        if (IsBound(adapterList[i],endp))
            break;

    if (i < adapterListLen) {
        // unbind the adapter (client) from the endpoint (server)
        Unbind( adapterList[i], endp );

        // find and delete all routing table entries for thie endpoint
        k = 0;
        for (j = 0; j < routerListLen; j++)
            if (routerList[j].routerAdapter != adapterList[i]) {
                if (j != k)
                    routerList[k] = routerList[j];
                k += 1;
            }

        // delete the adapter
        delete adapterList[i];

        // remove it from the list
        while (i < (adapterListLen - 1)) {
            adapterList[i] = adapterList[i+1];
            i += 1;
        }
        adapterListLen -= 1;
    }
}

//
//  BACnetRouter::FilterNPDU
//

bool BACnetRouter::FilterNPDU( BACnetNPDU &npdu, BACnetRouterAdapterPtr srcAdapter,  BACnetRouterAdapterPtr destAdapter )
{
#if _DEBUG_ROUTER
    printf( "BACnetRouter::FilterNPDU {%02X} [%d]->[%d]\n", npdu.pduTag
        , srcAdapter ? srcAdapter->adapterNet : 0
        , destAdapter ? destAdapter->adapterNet : 0
        );
#endif
    return true;
}

//
//  BACnetRouter::ProcessNPDU
//
//  This function is called by BACnetRouterAdapter::Confirmation() when it receives
//  a PDU coming up from the endpoint.  All it does is pass along the pdu with a 
//  pointer to itself so the router knows which endpoint it came from.
//
//  There are two types of messages: those specific to the network layer, and those 
//  that are repackaged as APDU's and sent up to the application layer.  The current 
//  version of the software only supports one application layer object to be attached 
//  to the router.  If it is necessary to have different BACnetAppClients on different 
//  BACnetNetServers, the two cannot share a BACnetRouter.
//

void BACnetRouter::ProcessNPDU( BACnetRouterAdapterPtr adapter, BACnetNPDU &npdu )
{
    bool                processLocally
    ,                   forwardMessage
    ;
    int                 i
    ;

#if _DEBUG_ROUTER
    printf( "BACnetRouter::ProcessNPDU\n" );
    printf( "    - adapter->adapterNet = %d\n", adapter->adapterNet );
    printf( "    - npdu.pduSource = %s\n", npdu.pduSource.ToString() );
    printf( "    - npdu.pduDestination = %s\n", npdu.pduDestination.ToString() );
    printf( "    - npdu.npduNetMessage = %d\n", npdu.npduNetMessage );
    printf( "    - npdu.npduSADR = %s\n", npdu.npduSADR.ToString() );
    printf( "    - npdu.npduDADR = %s\n", npdu.npduDADR.ToString() );
#endif
    
    // ### check to see if the source is legit
    
    // check to see if routing is legit
    if (npdu.npduSADR.addrType == remoteStationAddr) {
        // see if this is spoofing a directly connected network
        for (i = 0; i < adapterListLen; i++)
            if (adapterList[i]->adapterNet == npdu.npduSADR.addrNet)
                break;
        if (i < adapterListLen) {
#if _DEBUG_ROUTER
            printf( "    - spoofing a directly connected network\n" );
#endif
            return;
        }
        
        // see if this is spoofing an existing routing table entry
        for (i = 0; i < routerListLen; i++)
            if (routerList[i].routerNet == npdu.npduSADR.addrNet)
                break;
        if ((i < routerListLen) && (!(routerList[i].routerAddr == npdu.pduSource))) {
#if _DEBUG_ROUTER
            printf( "    - spoofing an existing router\n" );
#endif
            return;
        }
        
        // ### check to see if this source could be a router to the new network
        
        // ### check if it's OK to add an entry automatically
        if (i >= routerListLen) {
            if (!dynamicRouting) {
#if _DEBUG_ROUTER
                printf( "    - dynamic routing off\n" );
#endif
            } else {
                if (routerListLen >= kBACnetRouterMaxRouterListLen) {
#if _DEBUG_ROUTER
                    printf("    - no more routing table space\n");
#endif
                } else {
#if _DEBUG_ROUTER
                    printf( "    - add a routing table entry to network %d via %s\n"
                        , npdu.npduSADR.addrNet, npdu.pduSource.ToString()
                        );
                    printf( "    - new entry @%d, %d remaining\n", routerListLen, kBACnetRouterMaxRouterListLen - routerListLen - 1 );
#endif
                    // add a routing table entry saying it's reachable
                    BACnetRouterList	*dst
                    ;
                    
                    dst = routerList + routerListLen;
                    dst->routerNet = npdu.npduSADR.addrNet;
                    dst->routerStatus = 0;
                    dst->routerAddr = npdu.pduSource;
                    dst->routerAdapter = adapter;
                    routerListLen += 1;
                }
            }
        }
    }
    
    // ### what about a promiscuous upstream packet?  We shouldn't 
    // process packets that are correctly addressed to somebody else
    
    // check the destination
    switch (npdu.npduDADR.addrType) {
    
        case nullAddr:
            processLocally = true;
            forwardMessage = false;
            break;
        
        case remoteBroadcastAddr:
            if (npdu.npduDADR.addrNet == adapter->adapterNet) {
#if _DEBUG_ROUTER
                printf( "    - attempting to route to a network the device is already on" );
#endif
                return;
            }
            processLocally = (npdu.npduDADR.addrNet == deviceLocalNetwork);
            forwardMessage = true;
            break;
            
        case remoteStationAddr:
            if (npdu.npduDADR.addrNet == adapter->adapterNet) {
#if _DEBUG_ROUTER
                printf( "    - attempting to route to a network the device is already on" );
#endif
                return;
            }
            processLocally = (npdu.npduDADR.addrNet == deviceLocalNetwork)
                && (npdu.npduDADR == deviceLocalAddress)
                ;
            forwardMessage = !processLocally;
            break;
            
        case globalBroadcastAddr:
            processLocally = true;
            forwardMessage = true;
            break;
            
    }
    
#if _DEBUG_ROUTER
    printf( "    - process locally: %s\n", processLocally ? "true" : "false" );
    printf( "    - forward message: %s\n", forwardMessage ? "true" : "false" );
#endif
    
    // if this is an application layer message, see if it should be sent upstream
    if (npdu.npduNetMessage < 0) {
        // only forward to the device if we are bound to one
        if (serverPeer && processLocally) {
            BACnetPDU           pdu
            ;

            // see if it needs to look routed
            if ((adapterListLen >= 1) && (adapter->adapterNet != deviceLocalNetwork)) {
                // map the source address
                if (npdu.npduSADR.addrType == nullAddr)
                    pdu.pduSource.RemoteStation( adapter->adapterNet, npdu.pduSource.addrAddr, npdu.pduSource.addrLen );
                else
                    pdu.pduSource = npdu.npduSADR;
                    
                // map the destination
                if (npdu.npduDADR.addrType == globalBroadcastAddr)
                    pdu.pduDestination.GlobalBroadcast();
                else
                if (npdu.npduDADR.addrType == remoteBroadcastAddr)
                    pdu.pduDestination.LocalBroadcast();
                else
                    pdu.pduDestination = deviceLocalAddress;
            } else {
                // combine the source address
                if (npdu.npduSADR.addrType == nullAddr)
                    pdu.pduSource = npdu.pduSource;
                else
                    pdu.pduSource = npdu.npduSADR;
                    
                // pass along global broadcast knowledge if we have it
                if (npdu.npduDADR.addrType == globalBroadcastAddr)
                    pdu.pduDestination.GlobalBroadcast();
                else
                    pdu.pduDestination = npdu.pduDestination;
            }
            
            // copy the other fields
            pdu.pduExpectingReply = npdu.pduExpectingReply;
            pdu.pduNetworkPriority = npdu.pduNetworkPriority;
            pdu.pduTag = npdu.pduTag;
            pdu.SetReference( npdu );

            Response( pdu );
        }

        // don't continue processing if this was just for our benefit
        if (!forwardMessage)
            return;
    }

    // check to see if this is a network layer message
    if (npdu.npduNetMessage >= 0) {
        if (processLocally) {
            // check for vendor specific message
            if (npdu.npduVendorID == 0)
                ProcessNetMessage( adapter, npdu );
            else
            if (npdu.npduVendorID == kVendorID)
                ProcessVendorNetMessage( adapter, npdu );
            else {
#if _DEBUG_ROUTER
                printf( "    - vendor net message for vendor %d, not us\n", npdu.npduVendorID );
#endif
            }
        }
        
        // don't continue processing if this was just for our benefit
        if (!forwardMessage)
            return;
    }

    // if we have only one adapter, we're not much of a router
    if (adapterListLen == 1) {
#if _DEBUG_ROUTER
        printf( "    - not really a router\n" );
#endif
        return;
    }
    
    // make sure it hasn't been looped around
    if (npdu.npduHopCount == 0) {
#if _DEBUG_ROUTER
        printf( "    - hop count as run out\n" );
#endif
        return;
    }

#if _DEBUG_ROUTER
    printf( "    - forward ho!\n" );
#endif
    
    // build a message to send downstream
    BACnetNPDU      newpdu
    ;
    
    // copy the basic PDU fields
    newpdu.pduExpectingReply = npdu.pduExpectingReply;
    newpdu.pduNetworkPriority = npdu.pduNetworkPriority;
    newpdu.pduTag = npdu.pduTag;

    // copy the other network layer fields
    newpdu.npduNetMessage = npdu.npduNetMessage;
    newpdu.npduVendorID = npdu.npduVendorID;
    newpdu.npduHopCount = npdu.npduHopCount - 1;

    // fill in the appropriate SADR
    if (npdu.npduSADR.addrType == nullAddr)
        newpdu.npduSADR.RemoteStation( adapter->adapterNet, npdu.pduSource.addrAddr, npdu.pduSource.addrLen );
    else
        newpdu.npduSADR = npdu.npduSADR;
        
    // the PDU data is kept untouched
    newpdu.SetReference( npdu );
    
    // check the destination
    switch (npdu.npduDADR.addrType) {
    
        case globalBroadcastAddr:
            // send it to all adapters, except the one it came from
            newpdu.pduDestination.LocalBroadcast();
            newpdu.npduDADR.GlobalBroadcast();
            
            for (i = 0; i < adapterListLen; i++)
                if (adapter != adapterList[i]) {
                    if (FilterNPDU(newpdu, adapter, adapterList[i])) {
#if _DEBUG_ROUTER
                        printf( "    - filter passed\n" );
#endif
                        adapterList[i]->Indication( newpdu );
                    } else {
#if _DEBUG_ROUTER
                        printf( "    - filter failed\n" );
#endif
                    }
                }
            return;
            
        case remoteBroadcastAddr:
        case remoteStationAddr:
            //
            //	See if this message should go to one of our adapters.  If it does, include the 
            //	SNET/SADR from the source of this packet.
            //
            for (i = 0; i < adapterListLen; i++)
                if (adapterList[i]->adapterNet == npdu.npduDADR.addrNet)
                    break;
            if (i < adapterListLen) {
                // don't route to self adapter
                if (adapterList[i] != adapter) {
                    if (npdu.npduDADR.addrType == remoteBroadcastAddr)
                        newpdu.pduDestination.LocalBroadcast();
                    else
                        newpdu.pduDestination.LocalStation( npdu.npduDADR.addrAddr, npdu.npduDADR.addrLen );

                    if (FilterNPDU(newpdu, adapter, adapterList[i])) {
#if _DEBUG_ROUTER
                        printf( "    - filter passed\n" );
#endif
                        adapterList[i]->Indication( newpdu );
                    } else {
#if _DEBUG_ROUTER
                        printf( "    - filter failed\n" );
#endif
                    }
                }
                return;
            }

            //
            //	The packet can't be sent directly to a local station on our of our adapters,
            //	and there might be a router connected to us somehow that knows how 
            //	to deliver it.
            //
            for (i = 0; i < routerListLen; i++)
                if (routerList[i].routerNet == npdu.npduDADR.addrNet)
                    break;
            if (i < routerListLen) {
                if (routerList[i].routerStatus == 1) {
                    SendRejectMessageToNetwork( adapter, npdu, 0x01 );  // destination unreachable
                    return;
                } else
                if (routerList[i].routerStatus == 2) {
                    SendRejectMessageToNetwork( adapter, npdu, 0x02 );  // router busy
                    return;
                }

                // send directly to the router
                newpdu.pduDestination = routerList[i].routerAddr;
                newpdu.npduDADR = npdu.npduDADR;

                if (FilterNPDU(newpdu, adapter, routerList[i].routerAdapter)) {
#if _DEBUG_ROUTER
                    printf( "    - filter passed\n" );
#endif
                    routerList[i].routerAdapter->Indication( newpdu );
                } else {
#if _DEBUG_ROUTER
                    printf( "    - filter failed\n" );
#endif
                }
                return;
            }

            // ### check to see if this device is allowed to send messages to a unknown network

            if (!dynamicRouting) {
#if _DEBUG_ROUTER
                printf( "    - dynamic routing off\n" );
#endif
            } else {
#if _DEBUG_ROUTER
                printf( "    - hunting for path to network %d\n", npdu.npduDADR.addrNet );
#endif

                // if we are running out of space, purge unreachable networks
                if (routerListLen >= kBACnetRouterMaxRouterListLen) {
#if _DEBUG_ROUTER
                    printf("    - purge unreachable networks\n");
#endif
                }

                if (routerListLen < kBACnetRouterMaxRouterListLen) {
#if _DEBUG_ROUTER
                    printf( "    - unreachable entry @%d, %d remaining\n"
                        , routerListLen
                        , kBACnetRouterMaxRouterListLen - routerListLen - 1
                        );
#endif

                    // add a routing table entry saying it's unreachable, future 
                    // messages will be rejected until a path is found
                    BACnetRouterList	*dst
                    ;
                    
                    dst = routerList + routerListLen;
                    dst->routerNet = npdu.npduDADR.addrNet;
                    dst->routerStatus = 1;          // destination unreachable
                    dst->routerAddr.Null();
                    dst->routerAdapter = 0;
                    routerListLen += 1;
                } else {
#if _DEBUG_ROUTER
                    printf("    - no more routing table space\n");
#endif
                }

                // build a message looking for the router
                BACnetNPDU      huntpdu
                ;

                // destination is a local broadcast
                huntpdu.pduDestination.LocalBroadcast();

                // map the source address
                if (npdu.npduSADR.addrType == nullAddr)
                    huntpdu.npduSADR.RemoteStation( adapter->adapterNet, npdu.pduSource.addrAddr, npdu.pduSource.addrLen );
                else
                    huntpdu.npduSADR = npdu.npduSADR;
                    
                // set the message
                huntpdu.npduNetMessage = WhoIsRouterToNetwork;

                // allocate space for the network
                huntpdu.Allocate( 2 );

                // add the network
                huntpdu.PutShort( npdu.npduDADR.addrNet );

                // send it to all adapters, except the one it came from
                for (i = 0; i < adapterListLen; i++)
                    if (adapter != adapterList[i]) {
                        if (FilterNPDU(huntpdu, adapter, adapterList[i])) {
#if _DEBUG_ROUTER
                            printf( "    - filter passed\n" );
#endif
                            adapterList[i]->Indication( huntpdu );
                        } else {
#if _DEBUG_ROUTER
                            printf( "    - filter failed\n" );
#endif
                        }
                    }
            }
            return;
    }
    
    //
    // This is bad news for this packet.  Our last chance was to see if there was somebody 
    // that could deal with it, but there isn't.  If this was directed to us, send back a 
    // Reject-Message-To-Network.  If we received this because the requestor did some kind
    // of broadcast, don't bother flooding network with this message, he'll just fail.
    //
    if (npdu.pduDestination.addrType == localBroadcastAddr)
        return;

    // build a PDU, send it back the way it came
    if (npdu.pduDestination.addrType == localStationAddr) {
#if _DEBUG_ROUTER
        printf( "    - destination unreachable\n" );
#endif
        SendRejectMessageToNetwork( adapter, npdu, 0x01 );  // destination unreachable
        return;
    }

#if _DEBUG_ROUTER
    printf( "    - router processing error\n" );
#endif
}

//
//  BACnetRouter::ProcessVendorNetMessage
//

void BACnetRouter::ProcessVendorNetMessage( BACnetRouterAdapterPtr adapter, const BACnetNPDU &npdu )
{
#if _DEBUG_ROUTER
    printf( "BACnetRouter::ProcessVendorNetMessage(%d)\n", npdu.npduNetMessage );
#endif
}

//
//  BACnetRouter::ProcessNetMessage
//

void BACnetRouter::ProcessNetMessage( BACnetRouterAdapterPtr adapter, const BACnetNPDU &npdu )
{
    int             i, j, net, found, msgLen = npdu.pduLen
    ,               netListLen, *netList
    ;
    BACnetOctet     *msgPtr = npdu.pduData
    ;

#if _DEBUG_ROUTER
    printf( "BACnetRouter::ProcessNetMessage\n" );
#endif
    
    // interpret type
    switch (npdu.npduNetMessage) {

        case WhoIsRouterToNetwork:
#if _DEBUG_ROUTER
            printf( "    - WhoIsRouterToNetwork\n" );
#endif

            if (adapterListLen == 1) {
#if _DEBUG_ROUTER
                printf( "    - not really a router\n" );
#endif
                break;
            }
            
            // extract the network number
            if (msgLen == 0) {
                found = 1;
            } else {
                found = 0;
                net = (msgLen--,*msgPtr++);
                net = (net << 8) + (msgLen--,*msgPtr++);
            }

            // check for direct connected endpoints
            if (!found) {
                for (i = 0; i < adapterListLen; i++)
                    if (adapterList[i]->adapterNet == net) {
#if _DEBUG_ROUTER
                        printf( "    - found on directly connected network\n" );
#endif
                        found = 2;
                        break;
                    }
                if (found && (adapterList[i] == adapter)) {
#if _DEBUG_ROUTER
                    printf( "    - looking for the network it's on\n" );
#endif
                    break;
                }
            }

            // check for other routers I can get to
            if (!found) {
                for (i = 0; i < routerListLen; i++)
                    if (routerList[i].routerNet == net) {
#if _DEBUG_ROUTER
                        printf( "    - found on existing path\n" );
#endif
                        found = 2;
                        break;
                    }
                if (found && (routerList[i].routerAdapter == adapter)) {
#if _DEBUG_ROUTER
                    printf( "    - path reflected back\n" );
#endif
                    break;
                }
            }

            if (found) {
                if (found == 1) {
                    BroadcastRoutingTable( adapter );                   // found without searching, send everything
                } else {
                    BroadcastIAmRouterToNetwork( adapter, &net, 1 );    // send just the net
                }
            } else
            if (!dynamicRouting) {
#if _DEBUG_ROUTER
                printf( "    - dynamic routing off\n" );
#endif
            } else {
                // attempt to discover the router
                BACnetNPDU		newpdu
                ;

#if _DEBUG_ROUTER
                printf( "    - attempt to discover the router\n" );
#endif

                // map the source address
                if (npdu.npduSADR.addrType == nullAddr)
                    newpdu.npduSADR.RemoteStation( adapter->adapterNet, npdu.pduSource.addrAddr, npdu.pduSource.addrLen );
                else
                    newpdu.npduSADR = npdu.npduSADR;
                newpdu.pduDestination.LocalBroadcast();
                newpdu.npduNetMessage = WhoIsRouterToNetwork;

                // don't need much space
                newpdu.Allocate( 2 );
            
                // append the network to look for
                newpdu.PutShort( net );

                // send it to the other adapters
                for (i = 0; i < adapterListLen; i++)
                    if (adapterList[i] != adapter) {
                        adapterList[i]->Indication( newpdu );
                    }
            }
            break;

        case IAmRouterToNetwork:
            if (!dynamicRouting) {
#if _DEBUG_ROUTER
                printf( "    - IAmRouterToNetwork: dynamic routing off\n" );
#endif
                break;
            }
#if _DEBUG_ROUTER
            printf( "    - IAmRouterToNetwork:" );
#endif

            // build an array of the new route list
            netListLen = msgLen / 2;
            netList = new int[netListLen];

            for (i = 0; i < netListLen; i++) {
                netList[i] = *msgPtr++;
                netList[i] = (netList[i] << 8) + *msgPtr++;
#if _DEBUG_ROUTER
                printf( " %d", netList[i] );
#endif
            }
#if _DEBUG_ROUTER
            printf( "\n" );
#endif

            // delete existing entries
            BACnetRouterList	*src, *dst;
            src = dst = routerList;
            for (i = 0; i < routerListLen; i++) {
                for (j = 0; j < netListLen; j++)
                    if (src->routerNet == netList[j]) {
#if _DEBUG_ROUTER
                        printf( "    - entry @%d deleted\n", i );
#endif
                        break;
                    }
                if (j >= netListLen)
                    if (dst == src)
                        dst++, src++;
                    else
                        *dst++ = *src++;
            }

            // update the router list length
            if (dst != src)
                routerListLen = (dst - routerList);

            // if we are not going to have enough space for all of them, purge
            if ((routerListLen + netListLen) >= kBACnetRouterMaxRouterListLen) {
#if _DEBUG_ROUTER
                printf("    - purge unreachable networks\n");
#endif
            }

            // add these new entries
            for (i = 0; (i < netListLen) && (routerListLen < kBACnetRouterMaxRouterListLen); i++) {
#if _DEBUG_ROUTER
                printf( "    - entry @%d going to %d, %d remaining\n", routerListLen, netList[i], kBACnetRouterMaxRouterListLen - routerListLen - 1 );
#endif
                dst->routerNet = netList[i];
                dst->routerStatus = 0;
                dst->routerAddr = npdu.pduSource;
                dst->routerAdapter = adapter;
                dst += 1;
                routerListLen += 1;
            }

            // tell everybody else our new nets
            // ### we should build just one of these - what happens when the list is too long for a packet?
            for (i = 0; i < adapterListLen; i++)
                if (adapterList[i] != adapter)
                    BroadcastIAmRouterToNetwork( adapterList[i], netList, netListLen );

            // done with the list
            delete[] netList;
            break;

        case ICouldBeRouterToNetwork:
#if _DEBUG_ROUTER
            printf( "    - ICouldBeRouterToNetwork\n" );
#endif
            break;

        case RejectMessageToNetwork:
#if _DEBUG_ROUTER
            printf( "    - RejectMessageToNetwork\n" );
#endif
            // ### pass up to application somehow
            break;

        case RouterBusyToNetwork:
#if _DEBUG_ROUTER
            printf( "    - RouterBusyToNetwork\n" );
#endif
            // ### pass up to application somehow
            break;

        case RouterAvailableToNetwork:
#if _DEBUG_ROUTER
            printf( "    - RouterAvailableToNetwork\n" );
#endif
            // ### pass up to application somehow
            break;

        case InitializeRoutingTable:
#if _DEBUG_ROUTER
            printf( "    - InitializeRoutingTable\n" );
#endif
            // ### pass up to application somehow
            break;

        case InitializeRoutingTableAck:
#if _DEBUG_ROUTER
            printf( "    - InitializeRoutingTableAck\n" );
#endif
            // ### pass up to application somehow
            break;

        case EstablishConnectionToNetwork:
#if _DEBUG_ROUTER
            printf( "    - EstablishConnectionToNetwork\n" );
#endif
            break;

        case DisconnectConnectionToNetwork:
#if _DEBUG_ROUTER
            printf( "    - DisconnectConnectionToNetwork\n" );
#endif
            break;

        default:
            throw_1(5006); // unknown network message type
    }
}

//
//  BACnetRouter::SendRejectMessageToNetwork
//
//  This function builds a Reject=Message-To-Network message containing
//  the reason code provided.
//

void BACnetRouter::SendRejectMessageToNetwork( BACnetRouterAdapterPtr adapter, const BACnetNPDU &npdu, int rejectReason )
{
    BACnetNPDU  newpdu
    ;

    // don't need much space
    newpdu.Allocate( 4 );

    newpdu.pduDestination = npdu.pduSource;
    newpdu.npduDADR = npdu.npduSADR;

    newpdu.Put( RejectMessageToNetwork );           // Reject-Message-To-Network
    newpdu.Put( rejectReason );                     // reason
    newpdu.PutShort( npdu.npduDADR.addrNet );       // network number

    // send it back
    adapter->Indication( newpdu );
}

//
//  BACnetRouter::BroadcastIAmRouterToNetwork
//
//  This function builds an I-Am-Router-To-Network message containing the newly reachable
//  networks.
//

void BACnetRouter::BroadcastIAmRouterToNetwork( BACnetRouterAdapterPtr adapter, int *netList, int netListLen )
{
    int             i
    ;
    BACnetNPDU      npdu
    ;

    // build a message for this adapter
    npdu.pduSource.Null();
    npdu.pduDestination.LocalBroadcast();

    // build a message for this adapter
    npdu.Allocate( 2 * netListLen );

    // set the message
    npdu.npduNetMessage = IAmRouterToNetwork;

    // add the networks
    for (i = 0; i < netListLen; i++)
        npdu.PutShort( *netList++ );

    // send it out the adapter
    adapter->Indication( npdu );
}

//
//  BACnetRouter::BroadcastRoutingTable
//
//  This function builds an I-Am-Router-To-Network message containing all of the 
//  reachable networks for a particular adapter.  It is called by BroadcastRoutingTables
//  when during startup, and also in response to a Who-Is-Router-To-Network made 
//  by a device on one of the attached networks.
//

void BACnetRouter::BroadcastRoutingTable( BACnetRouterAdapterPtr adapter )
{
    int             i
    ;
    BACnetNPDU      npdu
    ;

    // build a message for this adapter
    npdu.pduSource.Null();
    npdu.pduDestination.LocalBroadcast();

    // build a message for this adapter
    npdu.Allocate( 2 * (adapterListLen + routerListLen) );

    // set the message
    npdu.npduNetMessage = IAmRouterToNetwork;

    // append those directly connected
    for (i = 0; i < adapterListLen; i++)
        if ((adapterList[i] != adapter) && (adapterList[i]->adapterNet != kBACnetRouterLocalNetwork))
            npdu.PutShort( adapterList[i]->adapterNet );

    // append those reachable
    for (i = 0; i < routerListLen; i++)
        if (routerList[i].routerAdapter != adapter)
            if (routerList[i].routerStatus == 0)
                npdu.PutShort( routerList[i].routerNet );

    // send it out the adapter
    adapter->Indication( npdu );
}

//
//  BACnetRouter::AddRoute
//

void BACnetRouter::AddRoute( int snet, int dnet, const BACnetAddress &addr )
{
    BACnetRouterAdapterPtr	rap
    ;

#if _DEBUG_ROUTER
    printf( "BACnetRouter::AddRoute %d %d %s\n", snet, dnet, addr.ToString() );
#endif

    rap = 0;
    for (int i = 0; i < adapterListLen; i++)
        if (adapterList[i]->adapterNet == snet) {
            rap = adapterList[i];
            break;
        }
    if (!rap) {
#if _DEBUG_ROUTER
        printf("    - no adapter for source network\n");
#endif
        throw_1(5010); // no adapter for source network
    }

    // check for a previously established route
    for (int j = 0; j < routerListLen; j++)
        if (routerList[j].routerNet == dnet) {
#if _DEBUG_ROUTER
            printf("    - already a route to this network via %s\n", routerList[j].routerAddr.ToString() );
#endif
            throw_2(5011, routerList[j].routerAddr.ToString() ); // already a route to this network
        }

    // if we are not going to have enough space for all of them, throw a fit
    if (routerListLen >= kBACnetRouterMaxRouterListLen) {
#if _DEBUG_ROUTER
        printf("    - routing table overflow\n");
#endif
        throw_1(5012); // routing table overflow
    }

    // add a routing table entry saying it's reachable
    BACnetRouterList	*dst
    ;

    dst = routerList + routerListLen;
    dst->routerNet = dnet;
    dst->routerStatus = 0;
    dst->routerAddr = addr;
    dst->routerAdapter = rap;
    routerListLen += 1;
}

//
//  BACnetRouter::BroadcastRoutingTables
//
//  This function broadcasts its routing tables to all adapters.  It is used during 
//  startup, and may also be called periodicly.
//

void BACnetRouter::BroadcastRoutingTables( void )
{
    // loop through the adapters
    for (int i = 0; i < adapterListLen; i++)
        BroadcastRoutingTable( adapterList[i] );
}

//
//  BACnetRouter::Indication
//
//  This PDU is coming down from a device and needs the properly formed NPDU header
//  attached to the front.
//

void BACnetRouter::Indication( const BACnetPDU &pdu )
{
    int             i
    ;
    BACnetNPDU      npdu
    ;

#if _DEBUG_ROUTER
    printf( "BACnetRouter::Indication\n" );
#endif

    // copy the tags
    npdu.pduTag = pdu.pduTag;

    // make sure the adapter knows this isn't a network message
    npdu.npduNetMessage = (BACnetNetworkMessage)-1;

    // the message will always come from us, no spoofing requests coming from the 
    // application, if you please!
    npdu.pduSource.Null();

    // set the hop count and reference to the data
    npdu.npduHopCount = 255;	// just in case it's going to be routed
    npdu.SetReference( pdu );

    // make sure the expecting reply and priority are passed down
    npdu.pduExpectingReply = pdu.pduExpectingReply;
    npdu.pduNetworkPriority = pdu.pduNetworkPriority;
    npdu.pduTag = pdu.pduTag;

    // figure out which source and destination address to use
    switch (pdu.pduDestination.addrType) {
        case nullAddr:
            throw_1(5007); // can't send to a null address

        case localStationAddr:
        case localBroadcastAddr:
            // find the adapter considered the local network
            for (i = 0; i < adapterListLen; i++)
                if (adapterList[i]->adapterNet == deviceLocalNetwork)
                    break;
            if (i >= adapterListLen)
                throw_1(5008); // no local network defined

            // the destination is exactly as requested
            npdu.pduDestination = pdu.pduDestination;

            // send the packet along to the local endpoint
            if (FilterNPDU(npdu, 0, adapterList[i])) {
#if _DEBUG_ROUTER
                printf( "    - filter passed\n" );
#endif
                adapterList[i]->Indication( npdu );
            } else {
#if _DEBUG_ROUTER
                printf( "    - filter failed\n" );
#endif
            }
            break;

        case remoteStationAddr:
            // check for an adapter for the DNET
            for (i = 0; i < adapterListLen; i++)
                if (adapterList[i]->adapterNet == pdu.pduDestination.addrNet)
                    break;
            if (i < adapterListLen) {
                if (adapterList[i]->adapterNet == deviceLocalNetwork) {
                    // this is really a local network
                    npdu.pduDestination.LocalStation( pdu.pduDestination.addrAddr, pdu.pduDestination.addrLen );
                } else {
                    // this should look like it's been routed
                    npdu.npduSADR = deviceLocalAddress;
                }

                // send it along
                if (FilterNPDU(npdu, 0, adapterList[i])) {
#if _DEBUG_ROUTER
                    printf( "    - filter passed\n" );
#endif
                    adapterList[i]->Indication( npdu );
                } else {
#if _DEBUG_ROUTER
                    printf( "    - filter failed\n" );
#endif
                }
            } else {
                // build a message to be routed
                npdu.npduDADR = pdu.pduDestination;

                // if this is a router, this message should look like it has been routed.
                if (deviceLocalNetwork >= 0)
                    npdu.npduSADR = deviceLocalAddress;

                // check for a router for the DNET
                for (i = 0; i < routerListLen; i++)
                    if (routerList[i].routerNet == pdu.pduDestination.addrNet)
                        break;
                if (i < routerListLen) {
                    // build a PDU, using the router's address (local to the adapter)
                    npdu.pduDestination = routerList[i].routerAddr;

                    // send it along
                    routerList[i].routerAdapter->Indication( npdu );
                    if (FilterNPDU(npdu, 0, routerList[i].routerAdapter)) {
#if _DEBUG_ROUTER
                        printf( "    - filter passed\n" );
#endif
                        routerList[i].routerAdapter->Indication( npdu );
                    } else {
#if _DEBUG_ROUTER
                        printf( "    - filter failed\n" );
#endif
                    }
                } else {
                    // build a PDU, using a broadcast address and send it to everybody
                    // in the hope that somebody will pick it up and carry it along
                    npdu.pduDestination.LocalBroadcast();

                    // send it to all adapters
                    for (i = 0; i < adapterListLen; i++) {
                        if (FilterNPDU(npdu, 0, adapterList[i])) {
#if _DEBUG_ROUTER
                            printf( "    - filter passed\n" );
#endif
                            adapterList[i]->Indication( npdu );
                        } else {
#if _DEBUG_ROUTER
                            printf( "    - filter failed\n" );
#endif
                        }
                    }
                }
            }
            break;

        case remoteBroadcastAddr:
            // check for an adapter for the DNET
            for (i = 0; i < adapterListLen; i++)
                if (adapterList[i]->adapterNet == pdu.pduDestination.addrNet)
                    break;
            if (i < adapterListLen) {
                // this is really a local broadcast
                npdu.pduDestination.LocalBroadcast();

                // send it along
                if (FilterNPDU(npdu, 0, adapterList[i])) {
#if _DEBUG_ROUTER
                    printf( "    - filter passed\n" );
#endif
                    adapterList[i]->Indication( npdu );
                } else {
#if _DEBUG_ROUTER
                    printf( "    - filter failed\n" );
#endif
                }
            } else {
                // build a message to be broadcast by the real router
                npdu.npduDADR = pdu.pduDestination;

                // this needs to look like it's been routed
                if (deviceLocalNetwork != kBACnetRouterLocalNetwork)
                    npdu.npduSADR = deviceLocalAddress;

                // check for a router for the DNET
                for (i = 0; i < routerListLen; i++)
                    if (routerList[i].routerNet == pdu.pduDestination.addrNet)
                        break;
                if (i < routerListLen) {
                    // build a PDU, using the router's address (local to the adapter)
                    npdu.pduDestination = routerList[i].routerAddr;

                    // send it along
                    if (FilterNPDU(npdu, 0, routerList[i].routerAdapter)) {
#if _DEBUG_ROUTER
                        printf( "    - filter passed\n" );
#endif
                        routerList[i].routerAdapter->Indication( npdu );
                    } else {
#if _DEBUG_ROUTER
                        printf( "    - filter failed\n" );
#endif
                    }
                } else {
                    // build a PDU, using a broadcast address and send it to everybody
                    // in the hope that somebody will pick it up and carry it along
                    npdu.pduDestination.LocalBroadcast();

                    // send it to all adapters
                    for (i = 0; i < adapterListLen; i++) {
                        if (FilterNPDU(npdu, 0, adapterList[i])) {
#if _DEBUG_ROUTER
                            printf( "    - filter passed\n" );
#endif
                            adapterList[i]->Indication( npdu );
                        } else {
#if _DEBUG_ROUTER
                            printf( "    - filter failed\n" );
#endif
                        }
                    }
                }
            }
            break;

        case globalBroadcastAddr:
            // build a message to be broadcast
            npdu.npduDADR = pdu.pduDestination;

            // use a local broadcast address for adapters
            npdu.pduDestination.LocalBroadcast();

            // send it to all local adapters (should only be one!)
            for (i = 0; i < adapterListLen; i++)
                if (adapterList[i]->adapterNet == deviceLocalNetwork) {
                    if (FilterNPDU(npdu, 0, adapterList[i])) {
#if _DEBUG_ROUTER
                        printf( "    - filter passed\n" );
#endif
                        adapterList[i]->Indication( npdu );
                    } else {
#if _DEBUG_ROUTER
                        printf( "    - filter failed\n" );
#endif
                    }
                }

            if (deviceLocalNetwork != kBACnetRouterLocalNetwork) {
                // change it to look like it has been 'routed'
                npdu.npduSADR = deviceLocalAddress;

                // send it to all non-local adapters
                for (i = 0; i < adapterListLen; i++)
                    if (adapterList[i]->adapterNet != deviceLocalNetwork) {
                        if (FilterNPDU(npdu, 0, adapterList[i])) {
#if _DEBUG_ROUTER
                            printf( "    - filter passed\n" );
#endif
                            adapterList[i]->Indication( npdu );
                        } else {
#if _DEBUG_ROUTER
                            printf( "    - filter failed\n" );
#endif
                        }
                    }
            }
            break;
    }
}

