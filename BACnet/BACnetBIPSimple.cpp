//
//  BACnetBIPSimple
//

#include <stdlib.h>
#include <string.h>

#include "BACnet.h"
#include "BACnetBIPSimple.h"

//
//  BACnetBIPSimple::Indication
//
//  This is called when the application has a message to send out.  This function 
//  attaches the proper BIP header to the front and sends it down to the endpoint
//  for delivery.
//

void BACnetBIPSimple::Indication( const BACnetPDU &pdu )
{
    int             len
    ;
    BACnetOctet     *msg, *msgPtr
    ;
    BACnetPDU       newpdu( pdu )
    ;

    switch (pdu.pduDestination.addrType) {
        case localStationAddr:
            len = 4 + pdu.pduLen;
            msg = msgPtr = new BACnetOctet[ len ];

            *msgPtr++ = kBVLCType;					// BVLL message
            *msgPtr++ = bvlcOriginalUnicastNPDU;	// original unicast
            *msgPtr++ = (len >> 8) & 0xFF;			// length
            *msgPtr++ = (len & 0xFF);
            memcpy( msgPtr, pdu.pduData, pdu.pduLen );

            // source and destination remain the same
            newpdu.pduSource = pdu.pduSource;
            newpdu.pduDestination = pdu.pduDestination;

            // set to new data
            newpdu.SetReference( msg, len );

            // send it along
            Request( newpdu );

            // done with the buffer
            delete[] msg;
            break;

        case localBroadcastAddr:
            len = 4 + pdu.pduLen;
            msg = msgPtr = new BACnetOctet[ len ];

            *msgPtr++ = kBVLCType;					// BVLL message
            *msgPtr++ = bvlcOriginalBroadcastNPDU;	// original broadcast
            *msgPtr++ = (len >> 8) & 0xFF;			// length
            *msgPtr++ = (len & 0xFF);
            memcpy( msgPtr, pdu.pduData, pdu.pduLen );

            // source and destination remain the same
            newpdu.pduSource = pdu.pduSource;
            newpdu.pduDestination = pdu.pduDestination;

            // set to new data
            newpdu.SetReference( msg, len );

            // send it along
            Request( newpdu );

            // done with the buffer
            delete[] msg;
            break;

        default:
            throw_1(4001); // should never get any other kind of address
    }
}

//
//  BACnetBIPSimple::Confirmation
//
//  This is called when the endpoint has received a message.  The BIP header is 
//  examined and the results passed up to the application.  If it is not one of 
//  ours, dump it.
//

void BACnetBIPSimple::Confirmation( const BACnetPDU &pdu )
{
    int                 msgType, msgLen
    ;
    BACnetAddress       addr
    ;
    BACnetOctet         *msgPtr = pdu.pduData
    ;
    BACnetPDU           newpdu
    ;

    // check for one of our headers
    if ((pdu.pduLen < 4) || (*msgPtr++ != kBVLCType))
        return;

    msgType = *msgPtr++;
    msgLen = *msgPtr++;
    msgLen = (msgLen << 8) + *msgPtr++;

    // make sure the length is correct
    if (pdu.pduLen != msgLen)
        return;

    switch (msgType) {
        case bvlcForwardedNPDU:
            // make sure there's enough data
            if (msgLen < 10)
                return;

            // extract the address from the header, keep the destination
            newpdu.pduSource.LocalStation( msgPtr, 6 );
            newpdu.pduDestination = pdu.pduDestination;

            // get the rest of the data
            newpdu.SetReference( msgPtr + 6, msgLen - 10 );

            // use result to forward up to application
            Response( newpdu );
            break;

        case bvlcOriginalUnicastNPDU:
        case bvlcOriginalBroadcastNPDU:
            // keep the source and destination
            newpdu.pduSource = pdu.pduSource;
            newpdu.pduDestination = pdu.pduDestination;

            // get the rest of the data
            newpdu.SetReference( msgPtr, msgLen - 4 );

            // use result to forward up to application
            Response( newpdu );
            break;
    }
}
