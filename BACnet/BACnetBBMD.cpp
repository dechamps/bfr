//
//  BACnetBBMD
//

#if _DEBUG_BBMD
#include <stdio.h>
#endif

#include <stdlib.h>
#include <string.h>

#include "BACnet.h"
#include "BACnetBBMD.h"

//
//  BACnetBBMD::BACnetBBMD
//

BACnetBBMD::BACnetBBMD( unsigned long host, unsigned short port )
    : BACnetTask( recurringTask, 1000 ), bbmdFDSupport(false)
    , bbmdAddress(host, port), bbmdLocalIndex(-1), bbmdBDTSize(0), bbmdFDTSize(0)
{
#if _DEBUG_BBMD
    printf("BACnetBBMD::BACnetBBMD %ld, %d\n", host, port);
#endif
}

//
//  BACnetBBMD::BACnetBBMD
//

BACnetBBMD::BACnetBBMD( const BACnetAddress &addr )
    : BACnetTask( recurringTask, 1000 ), bbmdFDSupport(false)
    , bbmdAddress(addr), bbmdLocalIndex(-1), bbmdBDTSize(0), bbmdFDTSize(0)
{
#if _DEBUG_BBMD
    printf("BACnetBBMD::BACnetBBMD %s\n", addr.ToString());
#endif
}

//
//  BACnetBBMD::~BACnetBBMD
//

BACnetBBMD::~BACnetBBMD( void )
{
#if _DEBUG_BBMD
    printf("BACnetBBMD::~BACnetBBMD\n");
#endif
}

//
//  BACnetBBMD::Indication
//
//  This is called when the appliction has a message to send out.  This function 
//  attaches the proper BIP header to the front and sends it down to the endpoint
//  for delivery.
//

void BACnetBBMD::Indication( const BACnetPDU &pdu )
{
    int             i, len
    ;
    BACnetOctet     *msg, *msgPtr
    ;
    BACnetPDU       newpdu
    ;

#if _DEBUG_BBMD
    printf("BACnetBBMD::Indication\n");
#endif

    // foreward the tags
    newpdu.pduTag = pdu.pduTag;

    switch (pdu.pduDestination.addrType) {
        case localStationAddr:
            len = 4 + pdu.pduLen;
            msg = msgPtr = new BACnetOctet[ len ];

            *msgPtr++ = kBVLCType;                      // BVLL message
            *msgPtr++ = bvlcOriginalUnicastNPDU;        // original unicast
            *msgPtr++ = (len >> 8) & 0xFF;              // length
            *msgPtr++ = (len & 0xFF);
            memcpy( msgPtr, pdu.pduData, pdu.pduLen );

            // build a new packet
            newpdu.pduDestination = pdu.pduDestination;
            newpdu.SetReference( msg, len );

            // send it along
            Request( newpdu );
            delete[] msg;
            break;

        case localBroadcastAddr:
            // first, send an original broadcast to the local network
            len = 4 + pdu.pduLen;
            msg = msgPtr = new BACnetOctet[ len ];

            *msgPtr++ = kBVLCType;                      // BVLL message
            *msgPtr++ = bvlcOriginalBroadcastNPDU;      // original broadcast
            *msgPtr++ = (len >> 8) & 0xFF;              // length
            *msgPtr++ = (len & 0xFF);
            memcpy( msgPtr, pdu.pduData, pdu.pduLen );

            // build a new broadcast packet
            newpdu.pduDestination.LocalBroadcast();
            newpdu.SetReference( msg, len );

            // send it along
            Request( newpdu );

            // done with this message
            delete[] msg;

            // create a forwarded NPDU message with self as the source
            len = 10 + pdu.pduLen;
            msg = msgPtr = new BACnetOctet[ len ];

            *msgPtr++ = kBVLCType;                      // BVLC Type
            *msgPtr++ = bvlcForwardedNPDU;              // Forwarded-NPDU
            *msgPtr++ = (len >> 8) & 0xFF;              // length
            *msgPtr++ = (len & 0xFF);
            memcpy( msgPtr, bbmdAddress.addrAddr, 6 );
            msgPtr+= 6;
            memcpy( msgPtr, pdu.pduData, pdu.pduLen );

            // set the data reference
            newpdu.SetReference( msg, len );

            // send it to all of the foreign devices
            if (pdu.pduTag & NoForeignDevicesTag) {
#if _DEBUG_BBMD
                printf("    - tag says no foreign devices\n");
#endif
            } else {
                for (i = 0; i < bbmdFDTSize; i++) {
                    newpdu.pduDestination = bbmdFDT[i].fdAddress;
                    Request( newpdu );
                }
            }

            // send to all of the BBMD peers (except myself)
            if (pdu.pduTag & NoPeersTag) {
#if _DEBUG_BBMD
                printf("    - tag says no peers\n");
#endif
            } else {
                for (i = 0; i < bbmdBDTSize; i++)
                    if (i != bbmdLocalIndex) {
                        newpdu.pduDestination.LocalStation( bbmdBDT[i].bdtIPAddr | (~bbmdBDT[i].bdtMask), bbmdBDT[i].bdtPort );
                        Request( newpdu );
                    }
            }

            // done with this message as well
            delete[] msg;
            break;

        default:
            throw_1(2001); // should never get any other kind of address
    }
}

//
//  BACnetBBMD::Confirmation
//
//  This is called when the endpoint has received a message.  The BIP header is 
//  examined and the results passed up to the application.  If it is not one of 
//  ours, dump it.
//

void BACnetBBMD::Confirmation( const BACnetPDU &pdu )
{
    int                 i, srcType, srcLen, len, ttl, rsltCode
    ;
    BACnetAddress       addr
    ;
    BACnetOctet         *srcPtr = pdu.pduData
    ;
    BACnetPDU           newpdu
    ;

#if _DEBUG_BBMD
    printf("BACnetBBMD::Confirmation\n");
#endif

    // check for one of our headers
    if ((pdu.pduLen < 4) || (*srcPtr++ != kBVLCType))
        return;

    srcType = *srcPtr++;
    srcLen = *srcPtr++;
    srcLen = (srcLen << 8) + *srcPtr++;

    // make sure the length is correct
    if (pdu.pduLen != srcLen)
        return;

    switch (srcType) {
        case bvlcWriteBroadcastDistributionTable:
#if _DEBUG_BBMD
            printf("    - write broadcast distribution table\n");
#endif
            break;

        case bvlcReadBroadcastDistributionTable:
#if _DEBUG_BBMD
            printf("    - read broadcast distribution table\n");
#endif

            // build a BDT response
            newpdu.pduSource.Null();
            newpdu.pduDestination = pdu.pduSource;

            len = 4 + (10 * bbmdBDTSize);
            newpdu.Allocate( len );

            newpdu.Put( kBVLCType );
            newpdu.Put( bvlcReadBroadcastDistributionTableAck );
            newpdu.PutShort( len );

            for (i = 0; i < bbmdBDTSize; i++) {
                // remember that bbmdBDT[i].bdtAddress has the host all 1's
                newpdu.PutLong( bbmdBDT[i].bdtIPAddr );
                newpdu.PutShort( bbmdBDT[i].bdtPort );
                newpdu.PutLong( bbmdBDT[i].bdtMask );
            }

            // send back result
            Request( newpdu );
            break;

        case bvlcForwardedNPDU:
#if _DEBUG_BBMD
            printf("    - forwarded npdu\n");
#endif
            // tag the packet
            newpdu.pduTag |= ForwardedNPDUTag;

            // ### check to see if this has been received from a peer BBMD

            // forward up to application, if there's one bound
            if (serverPeer) {
#if _DEBUG_BBMD
                printf("    - send upstream\n");
#endif
                newpdu.pduSource.LocalStation( srcPtr, 6 );
                newpdu.pduDestination = pdu.pduDestination;
                newpdu.SetReference( srcPtr + 6, srcLen - 10 );
                Response( newpdu );
            }

            // send it to all foreign devices
            for (i = 0; i < bbmdFDTSize; i++) {
#if _DEBUG_BBMD
                printf("    - send to foreign device: %s\n", bbmdFDT[i].fdAddress.ToString());
#endif
                newpdu.pduSource.Null();
                newpdu.pduDestination = bbmdFDT[i].fdAddress;
                newpdu.SetReference( pdu.pduData, pdu.pduLen );
                Request( newpdu );
            }

            // see if this should be broadcast on the local network, true iff my mask in the 
            // BDT says this is a two-hop network, otherwise, this was sent as a directed 
            // broadcast and other devices on this network are already seeing this message
            if ((bbmdLocalIndex >= 0) && (bbmdBDT[bbmdLocalIndex].bdtMask == 0xFFFFFFFF)) {
#if _DEBUG_BBMD
                printf("    - broadcast on local network\n");
#endif
                newpdu.pduSource.Null();
                newpdu.pduDestination.LocalBroadcast();
                newpdu.SetReference( pdu.pduData, pdu.pduLen );
                Request( newpdu );
            } else {
#if _DEBUG_BBMD
                printf("    - no local broadcast, local index: %d\n", bbmdLocalIndex);
#endif
            }
            break;

        case bvlcRegisterForeignDevice:
#if _DEBUG_BBMD
            printf("    - register foreign device\n");
#endif

            // drop it if its not supported
            if (!bbmdFDSupport) {
#if _DEBUG_BBMD
                printf("    - not supported\n");
#endif
                return;
            }

            // extract the requested time-to-live
            ttl = *srcPtr++;
            ttl = (ttl << 8) + *srcPtr++;

            // attempt to register
            rsltCode = RegisterForeignDevice( pdu.pduSource, ttl );
#if _DEBUG_BBMD
            printf("    - registration rsltCode: %d\n", rsltCode);
#endif

            // send back result code
            newpdu.pduSource.Null();
            newpdu.pduDestination = pdu.pduSource;

            newpdu.Allocate( 6 );
            newpdu.Put( kBVLCType );
            newpdu.Put( bvlcResult );
            newpdu.PutShort( 6 );
            newpdu.PutShort( rsltCode );

            Request( newpdu );
            break;

        case bvlcReadForeignDeviceTable:
#if _DEBUG_BBMD
            printf("    - read foreign device table\n");
#endif

            // send back table contents
            newpdu.pduSource.Null();
            newpdu.pduDestination = pdu.pduSource;

            len = (4 + 10 * bbmdFDTSize);
            newpdu.Allocate( len );

            newpdu.Put( kBVLCType );
            newpdu.Put( bvlcReadForeignDeviceTableAck );
            newpdu.PutShort( len );

            for (i = 0; i < bbmdFDTSize; i++) {
                newpdu.Put( bbmdFDT[i].fdAddress.addrAddr, 6 );
                newpdu.PutShort( bbmdFDT[i].fdTTL );
                newpdu.PutShort( bbmdFDT[i].fdRemain );
            }

            Request( newpdu );
            break;

        case bvlcDeleteForeignDeviceTableEntry:
#if _DEBUG_BBMD
            printf("    - delete foreign device table entry\n");
#endif

            break;

        case bvlcDistributeBroadcastToNetwork:
#if _DEBUG_BBMD
            printf("    - distribute broadcast to network\n");
#endif
            // tag the packet
            newpdu.pduTag |= ForeignDeviceTag;

            // allow application to get a copy, if there's one bound
            if (serverPeer) {
#if _DEBUG_BBMD
                printf("    - send upstream\n");
#endif
                newpdu.pduSource = pdu.pduSource;
                newpdu.pduDestination = pdu.pduDestination;
                newpdu.SetReference( srcPtr, srcLen - 4 );
                Response( newpdu );
            }

            // create a forwarded NPDU message
            len = 10 + pdu.pduLen - 4;
            newpdu.Allocate( len );

            newpdu.Put( kBVLCType );                    // BVLC Type
            newpdu.Put( bvlcForwardedNPDU );            // Forwarded-NPDU
            newpdu.PutShort( len );                     // length
            newpdu.Put( pdu.pduSource.addrAddr, 6 );    // copy the address where this came from

            // copy the message
            newpdu.Put( srcPtr, srcLen - 4 );

            // do a local broadcast
            newpdu.pduSource.Null();
            newpdu.pduDestination.LocalBroadcast();
            Request( newpdu );

            // send it to all foreign devices (except the one it came from)
            for (i = 0; i < bbmdFDTSize; i++)
                if (!(bbmdFDT[i].fdAddress == pdu.pduSource)) {
#if _DEBUG_BBMD
                    printf("    - send to foreign device: %s\n", bbmdFDT[i].fdAddress.ToString());
#endif
                    newpdu.pduDestination = bbmdFDT[i].fdAddress;
                    Request( newpdu );
                }

            // send to all of the BBMD peers (except myself)
            for (i = 0; i < bbmdBDTSize; i++)
                if (i != bbmdLocalIndex) {
#if _DEBUG_BBMD
                    printf("    - send to peer: %s\n", bbmdBDT[i].bdtAddress.ToString());
#endif
                    newpdu.pduDestination = bbmdBDT[i].bdtAddress;
                    Request( newpdu );
                }
            break;

        case bvlcOriginalUnicastNPDU:
#if _DEBUG_BBMD
            printf("    - original unicast npdu\n");
#endif
            // tag the packet
            newpdu.pduTag |= OriginalUnicastTag;

            // pass up to the next layer, keep the source that came up from the endpoint
            if (serverPeer) {
#if _DEBUG_BBMD
                printf("    - send upstream\n");
#endif
                newpdu.pduSource = pdu.pduSource;
                newpdu.pduDestination = pdu.pduDestination;
                newpdu.SetReference( srcPtr, srcLen - 4 );
                Response( newpdu );
            } else {
#if _DEBUG_BBMD
                printf("    - no upstream processing, dropped\n");
#endif
            }
            break;

        case bvlcOriginalBroadcastNPDU:
#if _DEBUG_BBMD
            printf("    - original broadcast npdu\n");
#endif
            // tag the packet
            newpdu.pduTag |= OriginalBroadcastTag;

            // allow application to get a copy
            if (serverPeer) {
#if _DEBUG_BBMD
                printf("    - send upstream\n");
#endif
                newpdu.pduSource = pdu.pduSource;
                newpdu.pduDestination = pdu.pduDestination;
                newpdu.SetReference( srcPtr, srcLen - 4 );
                Response( newpdu );
            } else {
#if _DEBUG_BBMD
                printf("    - no upstream processing\n");
#endif
            }

            // create a forwarded NPDU message
            len = 10 + pdu.pduLen - 4;
            newpdu.Allocate( len );

            newpdu.Put( kBVLCType );                    // BVLC Type
            newpdu.Put( bvlcForwardedNPDU );            // Forwarded-NPDU
            newpdu.PutShort( len );                     // length
            newpdu.Put( pdu.pduSource.addrAddr, 6 );    // copy the address where this came from

            // copy the message
            newpdu.Put( srcPtr, srcLen - 4 );

            // set up for distribution
            newpdu.pduSource.Null();

            // send it to all foreign devices
            for (i = 0; i < bbmdFDTSize; i++) {
#if _DEBUG_BBMD
                printf("    - send to foreign device: %s\n", bbmdFDT[i].fdAddress.ToString());
#endif
                newpdu.pduDestination = bbmdFDT[i].fdAddress;
                Request( newpdu );
            }

            // send to all of the BBMD peers (except myself)
            for (i = 0; i < bbmdBDTSize; i++)
                if (i != bbmdLocalIndex) {
#if _DEBUG_BBMD
                    printf("    - send to peer: %s\n", bbmdBDT[i].bdtAddress.ToString());
#endif
                    newpdu.pduDestination = bbmdBDT[i].bdtAddress;
                    Request( newpdu );
                }

            break;
    }
}

//
//	BACnetBBMD::AddForeignDevice
//

void BACnetBBMD::AddForeignDevice( char *spec, int ttl )
{
    unsigned long   ipAddr, ipMask
    ;
    unsigned short  ipPort
    ;

    if (bbmdFDTSize >= kBACnetBBMDMaxFDTSize)
        throw_2(2002,spec); // no more space

    // Parse the spec
    BACnetAddress::StringToHostPort( spec, &ipAddr, &ipMask, &ipPort );

    // load the table
    RegisterForeignDevice( BACnetAddress( ipAddr, ipPort ), ttl );
}

//
//	BACnetBBMD::DeleteForeignDevice
//

void BACnetBBMD::DeleteForeignDevice( char *spec )
{
    int             i
    ;
    unsigned long   ipAddr, ipMask
    ;
    unsigned short  ipPort
    ;

    // Parse the spec
    BACnetAddress::StringToHostPort( spec, &ipAddr, &ipMask, &ipPort );

    // find it
    for (i = 0; i < bbmdFDTSize; i++)
        if ((bbmdFDT[i].fdIPAddr == ipAddr) && (bbmdFDT[i].fdPort == ipPort))
            break;

    // if found, delete it
    if (i < bbmdFDTSize) {
        while (i < bbmdFDTSize - 1) {
            bbmdFDT[i] = bbmdFDT[ i + 1 ];
            i += 1;
        }
        bbmdFDTSize -= 1;
    }
}

//
//	BACnetBBMD::AddPeer
//

void BACnetBBMD::AddPeer( const char *spec )
{
    unsigned long   ipAddr, subnetMask
    ;
    unsigned short  port
    ;

#if _DEBUG_BBMD
    printf("BACnetBBMD::AddPeer '%s'\n", spec);
#endif

    if (bbmdBDTSize >= kBACnetBBMDMaxBDTSize)
        throw_1(2003);      // peer table overflow

    // parse the addess string
    BACnetAddress::StringToHostPort( spec, &ipAddr, &subnetMask, &port );

    // define the table elements
    bbmdBDT[bbmdBDTSize].bdtIPAddr = ipAddr;
    bbmdBDT[bbmdBDTSize].bdtPort = port;
    bbmdBDT[bbmdBDTSize].bdtMask = subnetMask;

    // address might be a directed broadcast
    bbmdBDT[bbmdBDTSize].bdtAddress = BACnetAddress( ipAddr | ~subnetMask, port );

    // see if this is my address
    if (BACnetAddress(ipAddr,port) == bbmdAddress) {
#if _DEBUG_BBMD
        printf("    - this is me\n");
#endif
        bbmdLocalIndex = bbmdBDTSize;
    }

    // update the count
    bbmdBDTSize += 1;
}

//
//  BACnetBBMD::DeletePeer
//

void BACnetBBMD::DeletePeer( const char *spec )
{
    unsigned long   ipAddr, subnetMask
    ;
    unsigned short  port
    ;

    if (bbmdBDTSize >= kBACnetBBMDMaxBDTSize)
        throw_1(2003);      // peer table overflow

    // parse the addess string
    BACnetAddress::StringToHostPort( spec, &ipAddr, &subnetMask, &port );

    for (int i = 0; i < bbmdBDTSize; i++)
        if ((ipAddr == bbmdBDT[i].bdtIPAddr) && (port == bbmdBDT[i].bdtPort)) {
            // delete it from the table
            for (int j = i+1; j < bbmdBDTSize; j++, i++)
                bbmdBDT[i] = bbmdBDT[j];
            bbmdBDTSize -= 1;

            // fix my local index
            if (bbmdLocalIndex == i)
                bbmdLocalIndex = -1;    // really?
            else
            if (bbmdLocalIndex > i)
                bbmdLocalIndex -= 1;

            // finished
            return;
        }

    // not found
    throw_1(2004);
}

//
//  BACnetBBMD::ProcessTask
//

void BACnetBBMD::ProcessTask( void )
{
    int             i
    ;
    BACnetFDTEntry  *src, *dst
    ;

#if 0
    printf("BACnetBBMD::ProcessTask\n");
#endif

    src = dst = bbmdFDT;
    for (i = 0; i < bbmdFDTSize; i++)
#if 0
        printf("    - fdRemain [%d]: %d\n", i, src->fdRemain);
#endif

        if (--src->fdRemain > 0) {
            if (src == dst) {
                dst++;
                src++;
            } else
                *dst++ = *src++;
        } else {
#if _DEBUG_BBMD
            printf("BACnetBBMD::ProcessTask - expired registration %s [%d]\n", src->fdAddress.ToString(), i);
#endif
            src += 1;
        }

    if (src != dst)
        bbmdFDTSize = (dst - bbmdFDT);
}

//
//  BACnetBBMD::RegisterForeignDevice
//

int BACnetBBMD::RegisterForeignDevice( const BACnetAddress &addr, int ttl )
{
    int         i
    ;

#if _DEBUG_BBMD
    printf("BACnetBBMD::RegisterForeignDevice %s %d\n", addr.ToString(), ttl);
#endif

    // table overflow
    if (bbmdFDTSize >= kBACnetBBMDMaxFDTSize) {
#if _DEBUG_BBMD
        printf("    - table overflow\n");
#endif
        return 0x0030;
    }

    // check to see if it's already in the table
    for (i = 0; i < bbmdFDTSize; i++)
        if (bbmdFDT[i].fdAddress == addr) {
#if _DEBUG_BBMD
            printf("    - re-registration\n");
#endif
            bbmdFDT[i].fdTTL = ttl;
            bbmdFDT[i].fdRemain = ttl + kBACnetBBMDForeignTTLExt;

            // success
            return 0;
        }

#if _DEBUG_BBMD
    printf("    - new registration [%d]\n", bbmdFDTSize);
#endif

    // load the table
    bbmdFDT[bbmdFDTSize].fdAddress = addr;
    addr.Unpack( &bbmdFDT[bbmdFDTSize].fdIPAddr
        , &bbmdFDT[bbmdFDTSize].fdPort
        );
    bbmdFDT[bbmdFDTSize].fdTTL = ttl;
    bbmdFDT[bbmdFDTSize].fdRemain = ttl;

    // update the count
    bbmdFDTSize += 1;

    // success
    return 0;
}

