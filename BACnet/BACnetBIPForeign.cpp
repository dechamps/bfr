//
//  BACnetBIPForeign
//

#include <stdlib.h>
#include <string.h>

#include "BACnet.h"
#include "BACnetBIPForeign.h"

//
//  BACnetBIPForeign::BACnetBIPForeign
//

BACnetBIPForeign::BACnetBIPForeign( void )
    : BACnetTask( recurringTask )
    , foreignStatus(0), foreignTimeToLive(0)
{
#if _DEBUG_FOREIGN
    printf("BACnetBIPForeign::BACnetBIPForeign\n");
#endif
}

//
//  BACnetBIPForeign::~BACnetBIPForeign
//

BACnetBIPForeign::~BACnetBIPForeign( void )
{
#if _DEBUG_FOREIGN
    printf("BACnetBIPForeign::~BACnetBIPForeign\n");
#endif
}

//
//  BACnetBIPForeign::Register
//

void BACnetBIPForeign::Register( unsigned long ipAddr, unsigned short port, int ttl )
{
#if _DEBUG_FOREIGN
    printf("BACnetBIPForeign::Register %08X %d %d\n", ipAddr, port, ttl);
#endif

    // set up the BBMD address
    foreignBBMDAddr.Pack( ipAddr, port );

    // set the time to live
    foreignTimeToLive = ttl;

    // proceed with the registration
    Register();
}

//
//  BACnetBIPForeign::Register
//

void BACnetBIPForeign::Register( void )
{
#if _DEBUG_FOREIGN
    printf("BACnetBIPForeign::Register\n");
#endif

    // we're dead until a response comes back
    foreignStatus = 0;

    // while we're alive, send registration requests at this interval
    taskInterval = foreignTimeToLive * 1000;
    InstallTask();

    // send off one right now while we're at it
    ProcessTask();
}

//
//  BACnetBIPForeign::Indication
//
//  This is called when the appliction has a message to send out.  This function 
//  attaches the proper BIP header to the front and sends it down to the endpoint
//  for delivery.
//

void BACnetBIPForeign::Indication( const BACnetPDU &pdu )
{
    int             len
    ;
    BACnetOctet     *msg, *msgPtr
    ;
    BACnetPDU       newpdu( pdu )
    ;

#if _DEBUG_FOREIGN
    printf("BACnetBIPForeign::Indication\n");
#endif

    if (!foreignStatus) {
#if _DEBUG_FOREIGN
        printf("    - not registered\n");
#endif
        return;
    }

    switch (pdu.pduDestination.addrType) {
        case localStationAddr:
#if _DEBUG_FOREIGN
            printf("    - local station\n");
#endif
            len = 4 + pdu.pduLen;
            msg = msgPtr = new BACnetOctet[ len ];

            *msgPtr++ = kBVLCType;						// BVLL message
            *msgPtr++ = bvlcOriginalUnicastNPDU;		// original unicast
            *msgPtr++ = (len >> 8) & 0xFF;				// length
            *msgPtr++ = (len & 0xFF);
            memcpy( msgPtr, pdu.pduData, pdu.pduLen );

            // save the data portion
// ###      newpdu.bvlcData = pdu.pduData;
            newpdu.SetReference( msg, len );

            // continue it downstream
            Request( newpdu );
            delete[] msg;
            break;

        case localBroadcastAddr:
#if _DEBUG_FOREIGN
            printf("    - local broadcast\n");
#endif
            len = 4 + pdu.pduLen;
            msg = msgPtr = new BACnetOctet[ len ];

            *msgPtr++ = kBVLCType;							// BVLL message
            *msgPtr++ = bvlcDistributeBroadcastToNetwork;	// distribute broadcast
            *msgPtr++ = (len >> 8) & 0xFF;					// length
            *msgPtr++ = (len & 0xFF);
            memcpy( msgPtr, pdu.pduData, pdu.pduLen );

            // save the data portion
// ###      pdu.bvlcData = pdu.pduData;
            newpdu.SetReference( msg, len );

            // continue it downstream
            Request( newpdu );
            delete[] msg;
            break;

        default:
            throw_1(3001); // no other address types allowed
    }
}

//
//	BACnetBIPForeign::Confirmation
//
//	This is called when the endpoint has received a message.  The BIP header is 
//	examined and the results passed up to the application.  If it is not one of 
//	ours, dump it.
//

void BACnetBIPForeign::Confirmation( const BACnetPDU &pdu )
{
    int             msgType, msgLen, rsltCode
    ;
    BACnetAddress   addr
    ;
    BACnetOctet     *msgPtr = pdu.pduData
    ;
    BACnetPDU       newpdu( pdu )
    ;

#if _DEBUG_FOREIGN
    printf("BACnetBIPForeign::Confirmation\n");
#endif

    // check for one of our headers
    if ((pdu.pduLen < 4) || (*msgPtr++ != kBVLCType)) {
#if _DEBUG_FOREIGN
        printf("    - not a BVLL message\n");
#endif
        return;
    }

    msgType = *msgPtr++;
    msgLen = *msgPtr++;
    msgLen = (msgLen << 8) + *msgPtr++;

    // make sure the length is correct
    if (pdu.pduLen != msgLen)
        return;

    switch (msgType) {
        case bvlcResult:
#if _DEBUG_FOREIGN
            printf("    - registration result\n");
#endif
            rsltCode = *msgPtr++;
            rsltCode = (rsltCode << 8) + *msgPtr++;

            // check the result code
            if (rsltCode == 0) {
#if _DEBUG_FOREIGN
                printf("    - success\n");
#endif
                foreignStatus = 1;
            } else
            if (rsltCode == 0x0030) {
#if _DEBUG_FOREIGN
                printf("    - failed: %d\n", rsltCode);
#endif
                foreignStatus = 0;
                UninstallTask();
            }
            break;

        case bvlcForwardedNPDU:
#if _DEBUG_FOREIGN
            printf("    - forwared npdu\n");
#endif
            // make sure there's enough data
            if (msgLen < 10)
                return;

            // save the original source in the header
// ###      newpdu.bvlcAddress = pdu.pduSource;
            newpdu.pduSource.LocalStation( msgPtr, 6 );

            // save the data portion
// ###      pdu.bvlcData = pdu.pduData;
            newpdu.SetReference( msgPtr + 6, msgLen - 10 );

            // use result to forward up to application
            Response( newpdu );
            break;

        case bvlcOriginalUnicastNPDU:
        case bvlcOriginalBroadcastNPDU:
#if _DEBUG_FOREIGN
            printf("    - original unicast/broadcast\n");
#endif
            // save the data portion
// ###      pdu.bvlcData = pdu.pduData;
            newpdu.SetReference( msgPtr, msgLen - 4 );

            // keep the source that came up from the endpoint
            Response( newpdu );
            break;
    }
}

//
//  BACnetBIPForeign::ProcessTask
//
//  This object sends a foreign device registration request to the BBMD.  It 
//  expects a confirmation (+) or (-) to maintain the foreignStatus.  If no 
//  response comes back in a reasonable amount of time the the device should 
//  send it again (more quickly than the TTL might otherwise specify).  If 
//  no confirmation comes back after a few attempts, this should go to an 
//  offline state.
//
//  Along the same lines, this object is expecting to go online soon after
//  Register() is called.  What happens when it can't?  How often (and how fast) 
//  should it continue the registration process before giving up (if ever)?
//
//  None of this behavior is specified in the standard...perhaps it should be.
//

void BACnetBIPForeign::ProcessTask( void )
{
    BACnetOctet     msg[6], *msgPtr = msg
    ;
    BACnetPDU       pdu
    ;

#if _DEBUG_FOREIGN
    printf("BACnetBIPForeign::ProcessTask\n");
#endif

    *msgPtr++ = kBVLCType;                          // type
    *msgPtr++ = bvlcRegisterForeignDevice;          // register foreign device
    *msgPtr++ = 0x00;                               // length
    *msgPtr++ = 0x06;
    *msgPtr++ = (foreignTimeToLive >> 8) & 0xFF;    // time to live
    *msgPtr++ = (foreignTimeToLive & 0xFF);

    // send the request to the BBMD
    pdu.pduDestination = foreignBBMDAddr;
    pdu.SetReference( msg, 6 );

    // send it along
    Request( pdu );
}
