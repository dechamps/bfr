//
//  BACnet.cpp
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include <netinet/in.h>

#include "BACnet.h"

//
//	BACnetAddress
//

BACnetAddress::BACnetAddress( const unsigned char *addr, const unsigned short len )
	: addrType(localStationAddr), addrNet(0), addrLen(len)
{
	if (len && addr)
		memcpy( addrAddr, addr, addrLen );
}

BACnetAddress::BACnetAddress( const unsigned short net, const unsigned char *addr, const unsigned short len )
	: addrType(remoteStationAddr), addrNet(net), addrLen(len)
{
	if (len && addr)
		memcpy( addrAddr, addr, addrLen );
}

BACnetAddress::BACnetAddress( const BACnetAddressType typ, const unsigned short net, const unsigned char *addr, const unsigned short len )
	: addrType(typ), addrNet(net), addrLen(len)
{
	if (len && addr)
		memcpy( addrAddr, addr, addrLen );
}

void BACnetAddress::Null( void )
{
	addrType = nullAddr;
	addrNet = 0;
	addrLen = 0;
}

void BACnetAddress::LocalStation( const BACnetOctet addr )
{
	addrType = localStationAddr;
	addrNet = 0;
	addrLen = 1;
	addrAddr[0] = addr;
}

void BACnetAddress::LocalStation( const unsigned char *addr, const unsigned short len )
{
	addrType = localStationAddr;
	addrNet = 0;
	addrLen = len;
	memcpy( addrAddr, addr, addrLen );
}

void BACnetAddress::LocalStation( const unsigned long ipAddr, const unsigned short port )
{
	addrType = localStationAddr;
	addrNet = 0;
	addrLen = 6;
	addrAddr[0] = (ipAddr >> 24) & 0xFF;
	addrAddr[1] = (ipAddr >> 16) & 0xFF;
	addrAddr[2] = (ipAddr >> 8) & 0xFF;
	addrAddr[3] = ipAddr & 0xFF;
	addrAddr[4] = port >> 8;
	addrAddr[5] = port & 0xFF;
}

void BACnetAddress::RemoteStation( const unsigned short net, const BACnetOctet addr )
{
	addrType = remoteStationAddr;
	addrNet = net;
	addrLen = 1;
	addrAddr[0] = addr;
}

void BACnetAddress::RemoteStation( const unsigned short net, const unsigned char *addr, const unsigned short len )
{
	addrType = remoteStationAddr;
	addrNet = net;
	addrLen = len;
	memcpy( addrAddr, addr, addrLen );
}

void BACnetAddress::LocalBroadcast( void )
{
	addrType = localBroadcastAddr;
	addrNet = 0;
	addrLen = 0;
}

void BACnetAddress::RemoteBroadcast( const short net )
{
	addrType = remoteBroadcastAddr;
	addrNet = net;
	addrLen = 0;
}

void BACnetAddress::GlobalBroadcast( void )
{
	addrType = globalBroadcastAddr;
	addrNet = 0;
	addrLen = 0;
}

BACnetAddress& BACnetAddress::operator =( const BACnetAddress &arg )
{
	addrType = arg.addrType;
	addrNet = arg.addrNet;
	addrLen = arg.addrLen;
	memcpy( addrAddr, arg.addrAddr, addrLen );
	return *this;
}

//
//	BACnetAddress == BACnetAddress
//

bool operator ==( const BACnetAddress &addr1, const BACnetAddress &addr2 )
{
	int			i
	;
	
	// address types must match
	if (addr1.addrType != addr2.addrType)
		return false;
	
	// remote broadcast and remote station have a network, localStation and remote
	// station have an address.
	switch (addr1.addrType) {
		case nullAddr:
		case localBroadcastAddr:
		case globalBroadcastAddr:
			break;
			
		case remoteBroadcastAddr:
			if (addr1.addrNet != addr1.addrNet) return false;
			break;
			
		case remoteStationAddr:
			if (addr1.addrNet != addr1.addrNet) return false;
		case localStationAddr:
			if (addr1.addrLen != addr2.addrLen) return false;
			for (i = 0; i < addr1.addrLen; i++)
				if (addr1.addrAddr[i] != addr2.addrAddr[i])
					return false;
			break;
			
		default:
			throw_1(1000); // no other address types allowed
	}
	
	// must be equal
	return true;
}

//
//	BACnetAddress::BACnetAddress
//

BACnetAddress::BACnetAddress( unsigned long host, unsigned short port )
{
	Pack( host, port );
}

//
//	BACnetAddress::BACnetAddress
//
//	Take an address in the form "x.x.x.x:x" and build a BACnet address 
//	out of it.
//

BACnetAddress::BACnetAddress( const char *spec )
{
	unsigned long	host
	;
	unsigned short	port
	;
	
	// parse the spec
	BACnetAddress::StringToHostPort( spec, &host, 0, &port );
	
	Pack( host, port );
}

//
//	BACnetAddress::LocalStation
//
//	Take an address in the form "n", "xx-xx-...", or "x.x.x.x:x" and build a BACnet address 
//	out of it.
//

void BACnetAddress::Station( const char *spec )
{
	int			n, a
	;
	char		c
	;
	const char	*src = spec
	;
	BACnetOctet	addrBuff[8]
	,			*dst = addrBuff
	;

	// empty string means no test
	if (!*src)
		throw_1(1001);		// invalid address format

	// simple check for local broadcast "*"
	if ((*src == '*') && (!*(src+1))) {
		LocalBroadcast();
		return;
	}

	// simple check for global broadcast "*:*"
	if ((*src == '*')
			&& (*(src+1) == ':')
			&& (*(src+2) == '*')
			&& (!*(src+3)) ) {
		GlobalBroadcast();
		return;
	}

	// might have hex or IP address
	dst = addrBuff;

	// get a number off the front
	for (n = 0; isdigit(*src); )
		n = (n * 10) + (*src++ - '0');

	// simple address
	if (!*src) {
		LocalStation( n );
		return;
	}

	// hex address
	if ( ((*src >= 'a') && (*src <= 'f'))
			|| ((*src >= 'A') && (*src <= 'F'))
			|| (*src == '-')
			) {
		// rewind
		src = spec;

		// suck out the hex char pairs
		while ((c = toupper(*src)) != 0) {
			// first hex digit
			if (isdigit(c)) {
				*dst = c - '0';
				src += 1;
			} else
			if ((c >= 'A') && (c <= 'F')) {
				*dst = c - 'A' + 10;
				src += 1;
			} else
				break;

			// next hex char
			c = toupper(*src);

			if (isdigit(c)) {
				*dst = (*dst << 4) + c - '0';
				src += 1;
			} else
			if ((c >= 'A') && (c <= 'F')) {
				*dst = (*dst << 4) + c - 'A' + 10;
				src += 1;
			} else
				;

			// this digit counts
			dst += 1;

			if (*src == '-')
				src += 1;
			else
				break;
		}

		// simple local station
		LocalStation( addrBuff, dst - addrBuff );

		// end-of-string
		if (*src)
			throw_1(1001);		// invalid address format

		return;
	}

	// ip address
	if (*src == '.') {
		// save the first octet
		*dst++ = n;

		// get the other octets
		for (int i = 0;i < 3; i++) {
			if (*src != '.')
				throw_1(1002);		// invalid IP address format
			src += 1;

			for (n = 0; isdigit(*src); )
				n = (n * 10) + (*src++ - '0');

			*dst++ = n;
		}

		// look for a port
		if (*src == ':') {
			src += 1;
			for (n = 0; isdigit(*src); )
				n = (n * 10) + (*src++ - '0');
		} else
			n = kBACnetIPDefaultPort;

		*dst++ = (n >> 8);
		*dst++ = (n & 0xFF);

		// saved as simple local station
		LocalStation( addrBuff, dst - addrBuff );

		// end-of-string
		if (*src)
			throw_1(1001);		// invalid address format

		return;
	}

	// check for remote something
	if (*src++ != ':')
		throw_1(1001);		// invalid address format

	// check for remote broadcast
	if ((*src == '*') && (!*(src+1))) {
		RemoteBroadcast( n );
		return;
	}

	// toss the stuff we've seen so far
	spec = src;

	// get the address
	for (a = 0; isdigit(*src); )
		a = (a * 10) + (*src++ - '0');

	// simple address
	if (!*src) {								
		RemoteStation( n, a );
		return;
	}

	// hex address
	if ( ((*src >= 'a') && (*src <= 'f'))
			|| ((*src >= 'A') && (*src <= 'F'))
			|| (*src == '-')
			) {
		// rewind
		src = spec;

		// suck out the hex char pairs
		while ((c = toupper(*src)) != 0) {
			// first hex digit
			if (isdigit(c)) {
				*dst = c - '0';
				src += 1;
			} else
			if ((c >= 'A') && (c <= 'F')) {
				*dst = c - 'A' + 10;
				src += 1;
			} else
				break;

			// next hex char
			c = toupper(*src);

			if (isdigit(c)) {
				*dst = (*dst << 4) + c - '0';
				src += 1;
			} else
			if ((c >= 'A') && (c <= 'F')) {
				*dst = (*dst << 4) + c - 'A' + 10;
				src += 1;
			} else
				;

			// this digit counts
			dst += 1;

			if (*src == '-')
				src += 1;
			else
				break;
		}

		// simple remote station
		RemoteStation( n, addrBuff, dst - addrBuff );
	} else
	if (*src == '.') {
		// save the first octet
		*dst++ = a;

		// get the other octets
		for (int i = 0;i < 3; i++) {
			if (*src != '.')
				throw_1(1002);		// invalid IP address format
			src += 1;

			for (a = 0; isdigit(*src); )
				a = (a * 10) + (*src++ - '0');

			*dst++ = a;
		}

		// look for a port
		if (*src == ':') {
			src += 1;
			for (a = 0; isdigit(*src); )
				a = (a * 10) + (*src++ - '0');
		} else
			a = kBACnetIPDefaultPort;

		*dst++ = (a >> 8);
		*dst++ = (a & 0xFF);

		// saved as simple local station
		RemoteStation( n, addrBuff, dst - addrBuff );
	}

	// end of string expected
	if (*src)
		throw_1(1003);		// invalid address filter format
}

//
//	BACnetAddress::Pack
//

void BACnetAddress::Pack( unsigned long host, unsigned short port )
{
	addrType = localStationAddr;
	addrLen = sizeof(unsigned long) + sizeof(unsigned short);
	Pack( addrAddr, host, port );
}

//
//	BACnetAddress::Unpack
//

void BACnetAddress::Unpack( unsigned long *hostp, unsigned short *portp )
	const
{
	Unpack( addrAddr, hostp, portp );
}

//
//	BACnetAddress::Pack
//

void BACnetAddress::Pack( BACnetOctet *rsltp, unsigned long host, unsigned short port )
{
	*(unsigned long *)rsltp = htonl(host);
	*(unsigned short *)(rsltp + 4) = htons(port);
}

//
//	BACnetAddress::Unpack
//

void BACnetAddress::Unpack( const BACnetOctet *rsltp, unsigned long *hostp, unsigned short *portp )
{
	if (hostp) *hostp = ntohl(*(unsigned long *)rsltp);
	if (portp) *portp = ntohs(*(unsigned short *)(rsltp + 4));
}

//
//	BACnetAddress::StringToHostPort
//

void BACnetAddress::StringToHostPort( const char *str, unsigned long *hostp, unsigned long *maskp, unsigned short *portp )
{
	int				n
	;
	unsigned long	host = 0
	;
	unsigned long	subnetmask = 0xFFFFFFFF
	;
	unsigned short	port = kBACnetIPDefaultPort
	;
	
	// first four decimals
	for (int i = 0; i < 4; i++ ) {
		BACnetOctet	byte = 0;
		while (*str && isdigit(*str))
			byte = (byte * 10) + (*str++ - '0');
		host = (host << 8) + byte;
		
		if (*str == '.')
			str += 1;
	}
	
	// check for number of bits in host
	if (*str == '/') {
		str += 1;
		
		// next is mask number
		if (isdigit(*str)) {
			n = 0;
			while (*str && isdigit(*str))
				n = (n * 10) + (*str++ - '0');
			
			if ((n > 0) && (n < 32))
				subnetmask = (subnetmask << (32 - n));
		}
	}
	
	// check for port number
	if (*str == ':') {
		str += 1;
		
		// next is port number
		if (isdigit(*str))
			if (toupper(*(str+1)) == 'X') {
				str += 2;
				for (port = 0; isxdigit(*str); str++)
					port = (port << 4) + (isdigit(*str) ? (*str - '0') : (toupper(*str) - 'A' + 10));
			} else {
				for (port = 0; isdigit(*str); str++)
					port = (port * 10) + (*str - '0');
			}
	}
	
	// return results
	if (hostp) *hostp = host;
	if (maskp) *maskp = subnetmask;
	if (portp) *portp = port;
}

//
//	BACnetAddress::operator char *
//

const char * BACnetAddress::ToString( void ) const
{
	static char	buff[64]
	;
	char		*dst = buff
	;
	
	switch (addrType) {
		case nullAddr:
			return "null";

		case localBroadcastAddr:
			return "*";

		case remoteBroadcastAddr:
			sprintf( buff, "%d:*", addrNet );
			return buff;

		case remoteStationAddr:
			sprintf( buff, "%d:", addrNet );
			while (*dst)
				dst++;

		case localStationAddr:
			if (addrLen == 0)
				*dst = 0;
			else
			if (addrLen == 1)
				sprintf( dst, "%d", addrAddr[0] );
			else
			if ((addrLen == 6) && (addrAddr[4] == 0xBA) && (addrAddr[5] == 0xC0))
				sprintf( dst, "%d.%d.%d.%d", addrAddr[0], addrAddr[1], addrAddr[2], addrAddr[3] );
			else {
				sprintf( dst, "%02X", addrAddr[0] );
				dst += 2;

				for (int i = 1; i < addrLen; i++) {
					sprintf( dst, "-%02X", addrAddr[i] );
					dst += 3;
				}
			}
			return buff;

		case globalBroadcastAddr:
			return "*:*";
	}


	return buff;
}

//
//	BACnetPDUData::BACnetPDUData
//

BACnetPDUData::BACnetPDUData( void )
	: pduData(0), pduLen(0), pduAllocLen(0)
{
}

//
//	BACnetPDUData::~BACnetPDUData
//

BACnetPDUData::~BACnetPDUData( void )
{
	if (pduAllocLen)
		Flush();
}

//
//	BACnetPDUData::BACnetPDUData
//

BACnetPDUData::BACnetPDUData( int allocLen )
	: pduData(0), pduLen(0), pduAllocLen(0)
{
	Allocate( allocLen );
}

//
//	BACnetPDUData::BACnetPDUData
//

BACnetPDUData::BACnetPDUData( const BACnetPDUData &pkt )
	: pduData(0), pduLen(0), pduAllocLen(0)
{
	SetReference( pkt.pduData, pkt.pduLen );
}

//
//	BACnetPDUData::BACnetPDUData
//

BACnetPDUData::BACnetPDUData( BACnetOctetPtr data, int len )
	: pduData(0), pduLen(0), pduAllocLen(0)
{
	SetReference( data, len );
}

//
//	BACnetPDUData::Allocate
//

void BACnetPDUData::Allocate( int allocLen )
{
	// already referenced or allocated?
	if (pduAllocLen)
		Flush();

	// allocate some data
	pduData = new BACnetOctet[allocLen];
	pduLen = 0;
	pduAllocLen = allocLen;
}

//
//	BACnetPDUData::SetReference
//

void BACnetPDUData::SetReference( const BACnetPDUData &pkt )
{
	SetReference( pkt.pduData, pkt.pduLen );
}

//
//	BACnetPDUData::SetReference
//

void BACnetPDUData::SetReference( BACnetOctetPtr data, int len )
{
	// already referenced or allocated?
	if (pduAllocLen)
		Flush();

	// reference the data
	pduData = data;
	pduLen = len;
	pduAllocLen = 0;
}

//
//	BACnetPDUData::Flush
//

void BACnetPDUData::Flush( void )
{
	if (pduAllocLen) {
		delete[] pduData;

		pduData = 0;
		pduAllocLen = 0;
	}
}

//
//	BACnetPDUData::Copy
//

void BACnetPDUData::Copy( const BACnetPDUData &pkt )
{
    Copy( pkt.pduData, pkt.pduLen );
}

//
//	BACnetPDUData::SetReference
//

void BACnetPDUData::Copy( BACnetOctetPtr data, int len )
{
    // if referenced or not enough allocated
    if (pduAllocLen < len) {
        Flush();
        Allocate(len);
    }
    
    // copy the data
    memcpy( pduData, data, len );
    pduLen = len;
}

//
//	BACnetPDUData::Get
//

BACnetOctet BACnetPDUData::Get( void )
{
	// trying to get from a put buffer?
	if (pduAllocLen)
		throw_1(1004);

	// trying to get something that's not there?
	if (!pduLen)
		throw_1(1005);

	return (pduLen--, *pduData++);
}

//
//	BACnetPDUData::Get
//

unsigned short BACnetPDUData::GetShort( void )
{
	unsigned short rslt
	;

	// trying to get from a put buffer?
	if (pduAllocLen)
		throw_1(1004);

	// trying to get something that's not there?
	if (pduLen < 2)
		throw_1(1005);

	rslt = (pduLen--, *pduData++);
	rslt = (rslt << 8) + (pduLen--, *pduData++);

	return rslt;
}

//
//	BACnetPDUData::GetLong
//

unsigned long BACnetPDUData::GetLong( void )
{
	unsigned long rslt
	;

	// trying to get from a put buffer?
	if (pduAllocLen)
		throw_1(1004);

	// trying to get something that's not there?
	if (pduLen < 4)
		throw_1(1005);

	rslt = *pduData++;
	rslt = (rslt << 8) + *pduData++;
	rslt = (rslt << 8) + *pduData++;
	rslt = (rslt << 8) + *pduData++;
	pduLen -= 4;

	return rslt;
}

//
//	BACnetPDUData::Get
//

void BACnetPDUData::Get( BACnetOctet *buff, int len )
{
	// trying to get from a put buffer?
	if (pduAllocLen)
		throw_1(1004);

	// trying to get something that's not there?
	if (pduLen < len)
		throw_1(1005);

	// copy the data
	memcpy( buff, pduData, len );

	// update
	pduData += len;
	pduLen -= len;
}

//
//	BACnetPDUData::CheckSpace
//
const int kPacketDataPad = 128;

void BACnetPDUData::CheckSpace( int size )
{
	// already enough?
	if (pduLen + size < pduAllocLen)
		return;

	// need a new buffer
	int			newLen = pduAllocLen + size + kPacketDataPad;
	BACnetOctet	*newData = new BACnetOctet[ newLen ];

	// copy and toss the old
	memcpy( newData, pduData, pduLen );
	delete[] pduData;

	// update
	pduData = newData;
	pduAllocLen = newLen;
}

//
//	BACnetPDUData::Put
//

void BACnetPDUData::Put( BACnetOctet ch )
{
	// trying to put to a get buffer?
	if (!pduAllocLen)
		throw_1(1004);

	// check for enough space
	CheckSpace( 1 );

	// copy the data
	*(pduData + pduLen) = ch;

	// update
	pduLen += 1;
}

//
//	BACnetPDUData::PutShort
//

void BACnetPDUData::PutShort( unsigned short n )
{
	// trying to put to a get buffer?
	if (!pduAllocLen)
		throw_1(1004);

	// check for enough space
	CheckSpace( 2 );

	// copy the data
	*(pduData + pduLen) = (n >> 8);
	*(pduData + pduLen + 1) = (n & 0xFF);

	// update
	pduLen += 2;
}

//
//	BACnetPDUData::PutLong
//

void BACnetPDUData::PutLong( unsigned long n )
{
	// trying to put to a get buffer?
	if (!pduAllocLen)
		throw_1(1004);

	// check for enough space
	CheckSpace( 4 );

	// copy the data
	BACnetOctet *dest = pduData + pduLen;

	*dest++ = (n >> 24);
	*dest++ = (n >> 16) & 0xFF;
	*dest++ = (n >> 8) & 0xFF;
	*dest++ = n & 0xFF;

	// update
	pduLen += 4;
}

//
//	BACnetPDUData::Put
//

void BACnetPDUData::Put( const BACnetOctet *buff, int len )
{
	// trying to put to a get buffer?
	if (!pduAllocLen)
		throw_1(1004);

	// check for enough space
	CheckSpace( len );

	// copy the data
	memcpy( pduData + pduLen, buff, len );

	// update
	pduLen += len;
}

//
//  BACnetPDU::BACnetPDU
//

BACnetPDU::BACnetPDU( void )
    : pduExpectingReply(0), pduNetworkPriority(0), pduTag(0)
{
}

//
//  BACnetPDU::operator =
//
//  Make a copy of the contents of a PDU.
//

BACnetPDU& BACnetPDU::operator =( const BACnetPDU &pkt )
{
    // copy the source and destination
    pduSource = pkt.pduSource;
    pduDestination = pkt.pduDestination;
    
    // copy the data
    BACnetPDUData::Copy( pkt );
    
    // copy the other flags
    pduNetworkPriority = pkt.pduNetworkPriority;
    pduExpectingReply = pkt.pduExpectingReply;

    // copy the tags
    pduTag = pkt.pduTag;

    // be friendly
    return *this;
}

//
//  BACnetNPDU::BACnetNPDU
//

BACnetNPDU::BACnetNPDU( void )
    : npduNetMessage((BACnetNetworkMessage)-1), npduVendorID(0), npduHopCount(0)
{
}

//
//	Encode
//
//	Encoding a PDU from an NPDU.
//
//	The downstream source address will always be null, I can't think of 
//	a reason (or a way) to do anything different.
//

void Encode( BACnetPDU &pdu, const BACnetNPDU &npdu )
{
	int		netLayerMessage, dnetPresent, snetPresent
	;

	// allocate some space
	pdu.Allocate( 32 + npdu.pduLen );

	// only version 1 messages supported
	pdu.Put( 0x01 );

	// build the flags
	netLayerMessage = (npdu.npduNetMessage >= 0 ? 0x80 : 0x00);
	dnetPresent = 0;

	// map the destination into the network header and pdu destination
	switch (npdu.npduDADR.addrType) {
		case nullAddr:
			break;

		case localStationAddr:
		case localBroadcastAddr:
			throw_1(1099);			// code problem, these two don't belong here
			break;

		case remoteStationAddr:
        case remoteBroadcastAddr:
        case globalBroadcastAddr:
            dnetPresent = 0x20;
			break;
	}
    // the destination has been provided by the router
    pdu.pduDestination = npdu.pduDestination;

	// the source must be null or a remote station
	snetPresent = (npdu.npduSADR.addrType == remoteStationAddr ? 0x08 : 0x00);
	pdu.pduSource.Null();

	// now we know how to encode the control octet
	pdu.Put( netLayerMessage | dnetPresent | snetPresent
		| (npdu.pduExpectingReply ? 0x04 : 0x00)
		| (npdu.pduNetworkPriority & 0x03)
		);

	// make sure expecting reply and priority get passed down
	pdu.pduExpectingReply = npdu.pduExpectingReply;
	pdu.pduNetworkPriority = npdu.pduNetworkPriority;

	// put the destination address
	if (dnetPresent)
		switch (npdu.npduDADR.addrType) {
			case remoteStationAddr:
				pdu.PutShort( npdu.npduDADR.addrNet );
				pdu.Put( npdu.npduDADR.addrLen );
				pdu.Put( npdu.npduDADR.addrAddr, npdu.npduDADR.addrLen );
				break;

			case remoteBroadcastAddr:
				pdu.PutShort( npdu.npduDADR.addrNet );
				pdu.Put( 0 );
				break;

			case globalBroadcastAddr:
				pdu.PutShort( 0xFFFF );
				pdu.Put( 0 );
				break;
		}

	// put the source address
	if (snetPresent) {
		pdu.PutShort( npdu.npduSADR.addrNet );
		pdu.Put( npdu.npduSADR.addrLen );
		pdu.Put( npdu.npduSADR.addrAddr, npdu.npduSADR.addrLen );
	}

	// put the hop count
	if (dnetPresent)
		pdu.Put( npdu.npduHopCount );
	
	// put the network layer message type (if present)
	if (netLayerMessage) {
		pdu.Put( npdu.npduNetMessage );
		// put the vendor ID
		if ((npdu.npduNetMessage >= 0x80) && (npdu.npduNetMessage <= 0xFF))
			pdu.PutShort( npdu.npduVendorID );
	}

	// everything else is data
	pdu.Put( npdu.pduData, npdu.pduLen );
}

//
//	Decode
//
//	Parse the contents of the pdu, and stuff the results into the npdu, within 
//	the context of the net.  If there is no network context to apply, the source 
//	is simply copied up.
//

void Decode( BACnetNPDU &npdu, const BACnetPDU &pdu )
{
	int					netLayerMessage, dnetPresent, snetPresent
	;
	const BACnetOctet	*npci = pdu.pduData
	;

	// copy the source and destination so that upper layers still have access to
	// the orginal source and destination.
	npdu.pduSource = pdu.pduSource;
	npdu.pduDestination = pdu.pduDestination;

	// check the length
	if (pdu.pduLen < 2) {
		return;	// invalid length
	}

	// only version 1 messages supported
	if (*npci++ != 0x01) {
		return; // version 1 only
	}

	// extract the flags
	netLayerMessage = (*npci & 0x80);
	dnetPresent = (*npci & 0x20);
	snetPresent = (*npci & 0x08);
	npdu.pduExpectingReply = (*npci & 0x04);
	npdu.pduNetworkPriority = (*npci & 0x03);
	npci += 1;
	
	// extract the destination address
	if (dnetPresent) {
		int		dnet, dlen
		;
		
		dnet = *npci++;
		dnet = (dnet << 8) + *npci++;
		dlen = *npci++;
		
		if (dnet == 0xFFFF)
			npdu.npduDADR.GlobalBroadcast();
		else {
			if (dlen == 0)
				npdu.npduDADR.RemoteBroadcast( dnet );
			else {
				npdu.npduDADR.RemoteStation( dnet, npci, dlen );
				npci += dlen;
			}
		}
	} else
		npdu.npduDADR.Null();         // unspecified
	
	// extract the source address, or copy the one from the endpoint
	if (snetPresent) {
		int		snet, slen
		;
		
		snet = *npci++;
		snet = (snet << 8) + *npci++;
		slen = *npci++;
		
		if (snet == 0xFFFF)
			throw_1(5004); // source can't be a global broadcast
		if (slen == 0)
			throw_1(5005); // source can't be a remote broadcast
		
		npdu.npduSADR.RemoteStation( snet, npci, slen );
		npci += slen;
	} else
        npdu.npduSADR.Null();         // unspecified
	
	// extract the hop count
	if (dnetPresent) {
		npdu.npduHopCount = *npci++;
	}

	// extract the network layer message type (if present)
	if (netLayerMessage) {
		npdu.npduNetMessage = (BACnetNetworkMessage)*npci++;
		if ((npdu.npduNetMessage >= 0x80) && (npdu.npduNetMessage <= 0xFF)) {
			// extract the vendor ID
			npdu.npduVendorID = *npci++;
			npdu.npduVendorID = (npdu.npduVendorID << 8) + *npci++;
		} else
			npdu.npduVendorID = 0;
	} else
		npdu.npduNetMessage = (BACnetNetworkMessage)-1;

	// everything else goes into the data
	npdu.SetReference( (BACnetOctet *)npci, pdu.pduLen - (npci - pdu.pduData) );
}

//
//	BACnetClient::BACnetClient
//

BACnetClient::BACnetClient( void )
	: clientPeer(0)
{
}

//
//	BACnetClient::~BACnetClient
//

BACnetClient::~BACnetClient( void )
{
	// unbind it if necessary
	if (clientPeer)
		Unbind( this, clientPeer );
}

//
//	BACnetClient::Request
//

void BACnetClient::Request( const BACnetPDU &pdu )
{
	// check to see if it's bound
	if (!clientPeer)
		throw_1(1006);

	// pass it along
	clientPeer->Indication( pdu );
}

//
//	BACnetServer::BACnetServer
//

BACnetServer::BACnetServer( void )
	: serverPeer(0)
{
}

//
//	BACnetServer::~BACnetServer
//

BACnetServer::~BACnetServer( void )
{
	// unbind it if necessary
	if (serverPeer)
		Unbind( serverPeer, this );
}

//
//	BACnetServer::Response
//

void BACnetServer::Response( const BACnetPDU &pdu )
{
	// check to see if it's bound
	if (!serverPeer)
		throw_1(1007);

	// pass it along
	serverPeer->Confirmation( pdu );
}

//
//	Bind
//

void Bind( BACnetClientPtr cp, BACnetServerPtr sp )
{
	// make sure they are unbound
	if (cp->clientPeer || sp->serverPeer)
		throw_1(1008);

	// and in the darkness...
	cp->clientPeer = sp;
	sp->serverPeer = cp;
}

//
//	Unbind
//

void Unbind( BACnetClientPtr cp, BACnetServerPtr sp )
{
	// make sure they are bound to each other
	if ((cp->clientPeer != sp) || (sp->serverPeer != cp))
		throw_1(1009);

	// unbind
	cp->clientPeer = 0;
	sp->serverPeer = 0;
}

//
//	IsBound
//

bool IsBound( BACnetClientPtr cp, BACnetServerPtr sp )
{
	return ((cp->clientPeer == sp) && (sp->serverPeer == cp));
}

//
//  BACnetPDUListElement::BACnetPDUListElement
//

BACnetPDUListElement::BACnetPDUListElement( void )
    : listNext(0), listPort(0)
{
    listPDU.Allocate( kDefaultListPDUSize );
}
    
//
//  This is a global list of PDU's that have been read by some 
//  port and are waiting to be processed by the port.  This is 
//  not necessarily the same order they were received, there 
//  may be a higher priority packet.
//

BACnetPDUList gProcessPDUList;

//
//  This is a global list of PDU buffers that have been allocated 
//  but are no longer being used.
//

BACnetPDUList gFreePDUList;

//
//  AllocateFreePDUList
//
//  This function is called once during startup to allocate 
//  the anticipated number of PDU buffers that will be needed.
//  If this isn't enough, more will be allocated as necessary, 
//  but none of them will be freed.
//

void AllocateFreePDUList( void )
{
    for (int i = 0; i < kDefaultListLen; i++)
        gFreePDUList.Write( new BACnetPDUListElement() );
}

//
//  BACnetPDUList::BACnetPDUList
//

BACnetPDUList::BACnetPDUList( void )
    : listHead(0), listTail(0)
{
}

//
//  BACnetPDUList::Write
//

void BACnetPDUList::Write( BACnetPDUListElementPtr lep )
{
    BACnetPDUListElementPtr *plep = &listHead
    ;
    
    // lowest priority is always append
    if (lep->listPDU.pduNetworkPriority == 0) {
        if (listTail)
            plep = &listTail->listNext;
    } else {
        // skip over higher or the same
        while (*plep && ((*plep)->listPDU.pduNetworkPriority >= lep->listPDU.pduNetworkPriority))
            plep = &(*plep)->listNext;
    }
    
    // link up next
    lep->listNext = *plep;
    
    // keep the tail pointing to the last element
    if (!*plep)
        listTail = lep;
        
    // stuff it in
    *plep = lep;
}

//
//  BACnetPDUList::Read
//

BACnetPDUListElementPtr BACnetPDUList::Read( void )
{
    BACnetPDUListElementPtr lep = listHead
    ;
    
    if (lep) {
        // unlink
        listHead = lep->listNext;
        lep->listNext = 0;
        
        // check the tail
        if (!listHead)
            listTail = 0;
            
        return lep;
    } else
        return 0;
}
