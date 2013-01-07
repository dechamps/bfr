//
//  BFRFilter
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "BFRFilter.h"
#include "BFRRegistration.h"

//
//  BIP Message Types
//

KeywordTable BIPKeywordTable[] =
    { { 0x7AD9DDA9,  0 }                    // BVLC-RESULT
    , { 0xDB2C653F,  1 }                    // WRITE-BROADCAST-DISTRIBUTION-TABLE
    , { 0xEABBFFDD,  1 }                    // WRITE-BDT
    , { 0xBE1E123D,  2 }                    // READ-BROADCAST-DISTRIBUTION-TABLE
    , { 0x22846274,  2 }                    // READ-BDT
    , { 0xAB90D4E9,  3 }                    // READ-BROADCAST-DISTRIBUTION-TABLE-ACK
    , { 0xAE10BC2C,  3 }                    // READ-BDT-ACK
    , { 0xA34641EF,  4 }                    // FORWARDED-NPDU
    , { 0xF3BF47A9,  5 }                    // REGISTER-FOREIGN-DEVICE
    , { 0x44BFC4B4,  6 }                    // READ-FOREIGN-DEVICE-TABLE
    , { 0xF2022D0C,  6 }                    // READ-FDT
    , { 0x0ACBEA40,  7 }                    // READ-FOREIGN-DEVICE-TABLE-ACK
    , { 0x7D8E86C4,  7 }                    // READ-FDT-ACK
    , { 0x5365DF07,  8 }                    // DELETE-FOREIGN-DEVICE-TABLE-ENTRY
    , { 0x9DBD6C6A,  8 }                    // DELETE-FDT-ENTRY
    , { 0x2BCD74BF,  9 }                    // DISTRIBUTE-BROADCAST-TO-NETWORK
    , { 0x5C99962D, 10 }                    // ORIGINAL-UNICAST-NPDU
    , { 0x911075CC, 11 }                    // ORIGINAL-BROADCAST-NPDU
    , { 0, 0 }
    };

//
//  NL Message Types
//

KeywordTable NPDUKeywordTable[] =
    { { 0x294E7816, 0 }                     // WHO-IS-ROUTER-TO-NETWORK
    , { 0x0154EC4F, 0 }                     // WHOS-RTN
    , { 0x89F9A2D5, 1 }                     // I-AM-ROUTER-TO-NETWORK
    , { 0x53C1FED2, 1 }                     // IM-RTN
    , { 0xF568D0CC, 2 }                     // I-COULD-BE-ROUTER-TO-NETWORK
    , { 0xDEDA7E5F, 2 }                     // I-CLD-BE-RTN
    , { 0x4ED9703F, 3 }                     // REJECT-MESSAGE-TO-NETWORK
    , { 0xC5D299CB, 3 }                     // REJ-MTN
    , { 0xCBCA6F6C, 4 }                     // ROUTER-BUSY-TO-NETWORK
    , { 0x39B8FAFE, 4 }                     // RBTN
    , { 0x5EEA137B, 5 }                     // ROUTER-AVAILABLE-TO-NETWORK
    , { 0x668F275C, 5 }                     // RATN
    , { 0x4B02A6C9, 6 }                     // INITIALIZE-ROUTING-TABLE
    , { 0xEF02FDF7, 6 }                     // IRT
    , { 0x0C092241, 7 }                     // INITIALIZE-ROUTING-TABLE-ACK
    , { 0xE6F311EB, 7 }                     // IRT-ACK
    , { 0xA35B5DA7, 8 }                     // ESTABLISH-CONNECTION-TO-NETWORK
    , { 0x080256D5, 8 }                     // ECTN
    , { 0xFDDDF574, 9 }                     // DISCONNECT-CONNECTION-TO-NETWORK
    , { 0x07A24D9E, 9 }                     // DCTN
    , { 0, 0 }
    };


//
//  AL Message Types
//

KeywordTable APDUKeywordTable[] =
    { { 0x566b432b, 0 }                     // CONFIRMED-REQUEST
    , { 0xa17e3788, 1 }                     // UNCONFIRMED-REQUEST
    , { 0x324dc228, 2 }                     // SIMPLEACK
    , { 0x38a36471, 3 }                     // COMPLEXACK
    , { 0x6c7c0630, 4 }                     // SEGMENTACK
    , { 0x71ad2a43, 5 }                     // ERROR
    , { 0xa9a5413f, 6 }                     // REJECT
    , { 0xa1e0029a, 7 }                     // ABORT
    , { 0, 0 }
    };

//
//  AL Confirmed Service Choices
//

KeywordTable ConfirmedServiceKeywordTable[] =
    { { 0xab6d1e6a,  0 }                    // ACKNOWLEDGEALARM
    , { 0x3cb39a42,  1 }                    // CONFIRMEDCOVNOTIFICATION
    , { 0xa3199d6d,  2 }                    // CONFIRMEDEVENTNOTIFICATION
    , { 0x82ef96f3,  3 }                    // GETALARMSUMMARY
    , { 0x215f7c25,  4 }                    // GETENROLLMENTSUMMARY
    , { 0xac01f937,  5 }                    // SUBSCRIBECOV
    , { 0x83874b40,  6 }                    // ATOMICREADFILE
    , { 0xced53372,  7 }                    // ATOMICWRITEFILE
    , { 0x14a62359,  8 }                    // ADDLISTELEMENT
    , { 0x3781ff05,  9 }                    // REMOVELISTELEMENT
    , { 0x9f0f989e, 10 }                    // CREATEOBJECT
    , { 0x130be39e, 11 }                    // DELETEOBJECT
    , { 0xc562eb57, 12 }                    // READPROPERTY
    , { 0x207267b4, 13 }                    // READPROPERTYCONDITIONAL
    , { 0x0f1b6c57, 14 }                    // READPROPERTYMULTIPLE
    , { 0xb988389a, 15 }                    // WRITEPROPERTY
    , { 0xeaf680ca, 16 }                    // WRITEPROPERTYMULTIPLE
    , { 0x3264db4b, 17 }                    // DEVICECOMMUNICATIONCONTROL
    , { 0x71e49612, 18 }                    // CONFIRMEDPRIVATETRANSFER
    , { 0x4f9f9daf, 19 }                    // CONFIRMEDTEXTMESSAGE
    , { 0x0b72caf4, 20 }                    // REINITIALIZEDEVICE
    , { 0xc48dddd0, 21 }                    // VTOPEN
    , { 0x7dd10cf5, 22 }                    // VTCLOSE
    , { 0x0fa4f51c, 23 }                    // VTDATA
    , { 0xd08d352b, 24 }                    // AUTHENTICATE
    , { 0x9c38b663, 25 }                    // REQUESTKEY
    , { 0x5F156F1A, 26 }                    // READRANGE
    , { 0xBC29635E, 29 }                    // GETEVENTINFORMATION
    , { 0, 0 }
    };

//
//  AL Unconfirmed Service Choices
//

KeywordTable UnconfirmedServiceKeywordTable[] =
    { { 0xac8ab1d2, 0 }                     // I-AM
    , { 0x9e0420f5, 1 }                     // I-HAVE
    , { 0x1c25f7cd, 2 }                     // UNCONFIRMEDCOVNOTIFICATION
    , { 0x2c0a8678, 3 }                     // UNCONFIRMEDEVENTNOTIFICATION
    , { 0x38121a2f, 4 }                     // UNCONFIRMEDPRIVATETRANSFER
    , { 0x6cb62a04, 5 }                     // UNCONFIRMEDTEXTMESSAGE
    , { 0x64a09d6a, 6 }                     // TIMESYNCHRONIZATION
    , { 0x474218f1, 7 }                     // WHO-HAS
    , { 0xb0e1c86d, 8 }                     // WHO-IS
    , { 0xf908f2f1, 9 }                     // UTCTIMESYNCHRONIZATION
    , { 0, 0 }
    };

//
//  Lookup
//

int Lookup( long code, KeywordTablePtr tp )
{
    while (tp->keyword)
        if (tp->keyword == code)
            return tp->value;
        else
            tp += 1;

    return -1;
}

//
//  BFRAddressFilter::BFRAddressFilter
//

BFRAddressFilter::BFRAddressFilter( void )
    : filterType(noTest), filterAddress()
{
}

//
//  BFRAddressFilter::SetFilter
//

void BFRAddressFilter::SetFilter( const char *desc )
{
    int         n, a
    ;
    char        c
    ;
    const char  *src = desc
    ;
    BACnetOctet addrBuff[8]
    ,           *dst = addrBuff
    ;

    // empty string means no test
    if (!*src) {
        filterType = noTest;
        return;
    }

    // simple check for local network ":"
    if ((*src == ':') && (!*(src+1))) {
        filterType = localNetwork;
        return;
    }

    // simple check for local broadcast "*"
    if ((*src == '*') && (!*(src+1))) {
        filterType = localBroadcast;
        filterAddress.LocalBroadcast();
        return;
    }

    // simple check for any remote station "*:"
    if ((*src == '*')
            && (*(src+1) == ':')
            && (!*(src+2)) ) {
        filterType = remoteNetworkAny;
        return;
    }

    // simple check for global broadcast "*:*"
    if ((*src == '*')
            && (*(src+1) == ':')
            && (*(src+2) == '*')
            && (!*(src+3)) ) {
        filterType = localBroadcast;
        filterAddress.GlobalBroadcast();
        return;
    }

    // might have hex or IP address
    dst = addrBuff;

    // get a number off the front
    for (n = 0; isdigit(*src); )
        n = (n * 10) + (*src++ - '0');

    if (!*src) {                                    // simple address
        // local station
        filterType = localStation;
        filterAddress.LocalStation( n );

        return;
    } else
    if ( ((*src >= 'a') && (*src <= 'f'))           // hex address
            || ((*src >= 'A') && (*src <= 'F'))
            || (*src == '-')
            ) {
        // local station
        filterType = localStation;

        // rewind
        src = desc;

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
        filterAddress.LocalStation( addrBuff, dst - addrBuff );

        // end-of-string
        if (*src)
            throw_1(8001);      // invalid address format

        return;
    } else
    if (*src == '.') {                              // ip address
        // special check may include subnet mask
        filterType = ipLocalStation;

        // save the first octet
        *dst++ = n;

        // get the other octets
        for (int i = 0;i < 3; i++) {
            if (*src != '.')
                throw_1(8002);      // invalid IP address format
            src += 1;

            for (n = 0; isdigit(*src); )
                n = (n * 10) + (*src++ - '0');

            *dst++ = n;
        }

        // assume everything is significant
        filterMask = (unsigned long)0xFFFFFFFF;

        // look for a subnet size
        if (*src == '/') {
            src += 1;

            for (n = 0; isdigit(*src); )
                n = (n * 10) + (*src++ - '0');

            // set the mask
            filterMask = (filterMask << (32 - n));
        }

        // any port
        filterPort = 0;

        // look for a port
        if (*src == ':') {
            src += 1;
            while (isdigit(*src))
                filterPort = (filterPort * 10) + (*src++ - '0');
        }

        // saved as simple local station
        filterAddress.LocalStation( addrBuff, dst - addrBuff );

        // end-of-string
        if (*src)
            throw_1(8001);      // invalid address format

        return;
    }

    // check for remote something
    if (*src++ != ':') {
        throw_1(8001);      // invalid address format
    }

    // remote network
    if (!*src) {
        filterType = remoteNetwork;
        filterAddress.addrNet = n;
        return;
    }

    // check for remote broadcast
    if ((*src == '*') && (!*(src+1))) {
        filterType = remoteBroadcast;
        filterAddress.RemoteBroadcast( n );
        return;
    }

    // toss the stuff we've seen so far
    desc = src;

    // get the address
    for (a = 0; isdigit(*src); )
        a = (a * 10) + (*src++ - '0');

    if (!*src) {                                    // simple address
        // local station
        filterType = remoteStation;

        filterAddress.RemoteStation( n, a );
    } else
    if ( ((*src >= 'a') && (*src <= 'f'))           // hex address
            || ((*src >= 'A') && (*src <= 'F'))
            || (*src == '-')
            ) {
        // local station
        filterType = remoteStation;
        // rewind
        src = desc;

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
        filterAddress.RemoteStation( n, addrBuff, dst - addrBuff );
    } else
    if (*src == '.') {
        // special check may include subnet mask
        filterType = ipRemoteStation;
        // save the first octet
        *dst++ = a;

        // get the other octets
        for (int i = 0;i < 3; i++) {
            if (*src != '.')
                throw_1(8002);      // invalid IP address format
            src += 1;

            for (a = 0; isdigit(*src); )
                a = (a * 10) + (*src++ - '0');

            *dst++ = a;
        }

        // assume everything is significant
        filterMask = (unsigned long)0xFFFFFFFF;

        // look for a subnet size
        if (*src == '/') {
            src += 1;

            for (a = 0; isdigit(*src); )
                a = (a * 10) + (*src++ - '0');

            // set the mask
            filterMask = (filterMask << (32 - a));
        }

        // any port
        filterPort = 0;

        // look for a port
        if (*src == ':') {
            src += 1;
            while (isdigit(*src))
                filterPort = (filterPort * 10) + (*src++ - '0');
        }

        // saved as simple local station
        filterAddress.LocalStation( addrBuff, dst - addrBuff );
    }

    // end of string expected
    if (*src)
        throw_1(8001);      // invalid address filter format
}

//
//  BFRAddressFilter::Test
//

bool BFRAddressFilter::Test( const BACnetAddress &addr )
{
    switch (filterType) {
        case noTest:                // always passes
            return true;

        case localNetwork:
            return (addr.addrType == localStationAddr);

        case remoteNetwork:
            return (addr.addrType == remoteStationAddr) && (addr.addrNet == filterAddress.addrNet);

        case remoteNetworkAny:
            return (addr.addrType == remoteStationAddr);

        case ipRemoteStation:
            if (addr.addrNet != filterAddress.addrNet) return false;
        case ipLocalStation:
            if (addr.addrLen != 6)
                return false;

            // match the address
            if ((addr.addrAddr[0] & (filterMask >> 24)) != filterAddress.addrAddr[0])
                return false;
            if ((addr.addrAddr[1] & (filterMask >> 16)) != filterAddress.addrAddr[1])
                return false;
            if ((addr.addrAddr[2] & (filterMask >> 8)) != filterAddress.addrAddr[2])
                return false;
            if ((addr.addrAddr[3] & filterMask) != filterAddress.addrAddr[3])
                return false;

            // match the port
            if (filterPort && (filterPort != ((addr.addrAddr[4] << 8) + addr.addrAddr[5])))
                return false;

            // success
            return true;

//      case localStation:
//      case localBroadcast:
//      case remoteStation:
//      case remoteBroadcast:
//      case globalBroadcast:
        default:
            return (addr == filterAddress);
    }
}

//
//  BFRFunctionFilter::BFRFunctionFilter
//

BFRFunctionFilter::BFRFunctionFilter( void )
    : filtering(false)
    , filterBVLCMask(0), filterNPCIMask(0), filterAPDUMask(0)
    , filterConfirmedMask(0), filterUnconfirmedMask(0)
{
}

//
//  BFRFunctionFilter::SetFilter
//

void BFRFunctionFilter::SetFilter( const char *desc )
{
#define LARGENUMBER     6293815 
 
    int         index, code, value
    ;
    const char  *src = desc
    ;
    long        sum, multiple
    ;

    while (*src) {
        // find the beginning of a keyword
        while (*src && !isalpha(*src)) src++;
        if (!*src) break;

        // build a hash code out of the keyword
        index = 1;
        code = 0;
        multiple = LARGENUMBER;
        while (*src && (isalpha(*src) || (*src == '-'))) {
            code += multiple * index++ * toupper(*src++);
            multiple *= LARGENUMBER;
        }

        // check the keyword tables
        value = Lookup(code, BIPKeywordTable);
        if (value >= 0) {
            filterBVLCMask |= (1 << value);
            continue;
        }
        value = Lookup(code, NPDUKeywordTable);
        if (value >= 0) {
            filterNPCIMask |= (1 << value);
            continue;
        }
        value = Lookup(code, APDUKeywordTable);
        if (value >= 0) {
            filterAPDUMask |= (1 << value);
            continue;
        }
        value = Lookup(code, ConfirmedServiceKeywordTable);
        if (value >= 0) {
            filterConfirmedMask |= (1 << value);
            continue;
        }
        value = Lookup(code, UnconfirmedServiceKeywordTable);
        if (value >= 0) {
            filterUnconfirmedMask |= (1 << value);
            continue;
        }

        throw_1(8005);      // unrecognized keyword
    }

    // set filtering flag to tell Test() something should be checked
    filtering = true;
}

//
//  BFRFunctionFilter::Test
//

bool BFRFunctionFilter::Test( BFRFilterSetPtr fsp )
{
    // if the function is set, check the coorisponding bit in the mask
    if ((fsp->filterBVLCFn >= 0) && (filterBVLCMask & (1 << fsp->filterBVLCFn)))
        return true;
    if ((fsp->filterNPCIFn >= 0) && (filterNPCIMask & (1 << fsp->filterNPCIFn)))
        return true;
    if ((fsp->filterAPDUFn >= 0) && (filterAPDUMask & (1 << fsp->filterAPDUFn)))
        return true;
    if ((fsp->filterConfirmedFn >= 0) && (filterConfirmedMask & (1 << fsp->filterConfirmedFn)))
        return true;
    if ((fsp->filterUnconfirmedFn >= 0) && (filterUnconfirmedMask & (1 << fsp->filterUnconfirmedFn)))
        return true;

    return false;
}

//
//  BFRFilterElem::BFRFilterElem
//

BFRFilterElem::BFRFilterElem( void )
    : filterNext(0), filterAccept(false)
    , filterSource(), filterDestination()
    , filterFunction()
{
}

//
//  BFRFilterElem::Test
//
//  Check each of the elements of the packet and see if they match.  If any of the tests fail,
//  the checking should continue to the next filter element, so return "no decision".  If all 
//  of the specified tests pass then the filter element should return "accept" or "reject", 
//  depending on the filterAccept setting.
//

int BFRFilterElem::Test( const BACnetPDU &pdu, BFRFilterSetPtr fsp )
{
    // check the source address
    if (filterSource.filterType && !filterSource.Test(pdu.pduSource))
        return 0;

    // check the destination address
    if (filterDestination.filterType && !filterDestination.Test(pdu.pduDestination))
        return 0;

    // check the function
    if (filterFunction.filtering && !filterFunction.Test(fsp))
        return 0;

    // accept or reject this packet
    return (filterAccept ? 1 : -1);
}

//
//  BFRFilterElemFactory
//

BFRFilterElemFactory gBFRFilterElemFactory;

//
//  BFRFilterElemFactory::StartElement
//

voidPtr BFRFilterElemFactory::StartElement( const char *name, const MinML::AttributeList& attrs )
{
    const char          *valu
    ;
    BFRFilterElemPtr    fep
    ;

    // build a filter element
    fep = new BFRFilterElem();

    // see if this is an accept element, this generator works for both accept and reject
    fep->filterAccept = (strcmp(name,"Accept") == 0);

    // check for component matching
    if ((valu = SubstituteArgs(attrs["source"])) != 0) {
        fep->filterSource.SetFilter( valu );
    }
    if ((valu = SubstituteArgs(attrs["destination"])) != 0) {
        fep->filterDestination.SetFilter( valu );
    }
    if ((valu = SubstituteArgs(attrs["function"])) != 0) {
        fep->filterFunction.SetFilter( valu );
    }

    // return the element
    return fep;
}

//
//  BFRFilterSet::BFRFilterSet
//

BFRFilterSet::BFRFilterSet( void )
    : filterFirst(0)
{
}

//
//  BFRFilterSet::~BFRFilterSet
//

BFRFilterSet::~BFRFilterSet( void )
{
}

//
//  BFRFilterSet::AddElem
//
//  The filter list is still a singe linked list, but unlike the other lists this
//  one needs to keep its order intact.
//

void BFRFilterSet::AddElem( BFRFilterElemPtr fep )
{
    BFRFilterElemPtr    *pfep = &filterFirst
    ;

    // find the end of the list
    while (*pfep)
        pfep = &(*pfep)->filterNext;

    // add it on the end
    *pfep = fep;
    fep->filterNext = 0;
}

//
//  BFRFilterSet::RemoveElem
//

void BFRFilterSet::RemoveElem( BFRFilterElemPtr fep )
{
    BFRFilterElemPtr    *pfep
    ;

    for (pfep = &filterFirst; *pfep; pfep = &(*pfep)->filterNext)
        if (*pfep == fep) {
            *pfep = fep->filterNext;
            fep->filterNext = 0;
            break;
        }
}

//
//  BFRFilterSet::Length
//

int BFRFilterSet::Length( void )
{
    int                 count = 0
    ;
    BFRFilterElemPtr    fep
    ;

    for (fep = filterFirst; fep; fep = fep->filterNext)
        count += 1;

    return count;
}

//
//  BFRFilterSet::GetElem
//

BFRFilterElemPtr BFRFilterSet::GetElem( int i )
{
    int                 count = 0
    ;
    BFRFilterElemPtr    fep
    ;

    for (fep = filterFirst; fep; fep = fep->filterNext) {
        if (count == i)
            break;
        count += 1;
    }

    return fep;
}

//
//  BFRFilterSet::Test
//

bool BFRFilterSet::Test( const BACnetPDU &pkt )
{
    int                 rslt
    ;
    BFRFilterElemPtr    fep
    ;

    // analyze the packet for functions
    Analyze(pkt);

    // test each of the filter elements
    for (fep = filterFirst; fep; fep = fep->filterNext) {
        rslt = fep->Test( pkt, this );
        if (rslt != 0)
            return (rslt < 0 ? false : true);
    }

    // packet passes if there are no matches
    return true;
}

//
//  BFRFilterSet::Analyze
//

void BFRFilterSet::Analyze( const BACnetPDU &pdu )
{
    const BACnetOctet   *pduData = pdu.pduData
    ;
    int                 pduLen = pdu.pduLen
    ;
    int                 netLayerMessage, dnetPresent, snetPresent
    ;

    // all functions unset
    filterBVLCFn = -1;
    filterOriginatingDevice.Null();

    filterNPCIFn = -1;
    filterSADR.Null();
    filterDADR.Null();

    filterAPDUFn = -1;
    filterConfirmedFn = -1;
    filterUnconfirmedFn = -1;

    // check for a BVLL header
    if (*pduData == 0x81) {
        pduLen--,pduData++;

        // extract the function
        filterBVLCFn = (pduLen--,*pduData++);

        // extract the length
        int len = (pduLen--,*pduData++);
        len = (len << 8) + (pduLen--,*pduData++);

        // set the function group
        switch ((BVLCFunction)filterBVLCFn) {
            case bvlcResult:
            case bvlcWriteBroadcastDistributionTable:
            case bvlcReadBroadcastDistributionTable:
            case bvlcReadBroadcastDistributionTableAck:
            case bvlcRegisterForeignDevice:
            case bvlcReadForeignDeviceTable:
            case bvlcReadForeignDeviceTableAck:
            case bvlcDeleteForeignDeviceTableEntry:
                // no deeper parsing
                return;

            case bvlcForwardedNPDU:
                // extract the original source
                filterOriginatingDevice.LocalStation( pduData, 6 );

                pduData += 6;
                pduLen -= 6;

                // keep digging
                break;

            case bvlcDistributeBroadcastToNetwork:
            case bvlcOriginalUnicastNPDU:
            case bvlcOriginalBroadcastNPDU:
                // keep digging
                break;
        }
    }

    // check the length
    if (pduLen < 2)
        return;
    
    // only version 1 messages supported
    if (*pduData++ != 0x01)
        return;
    
    // extract the flags
    netLayerMessage = (*pduData & 0x80);
    dnetPresent = (*pduData & 0x20);
    snetPresent = (*pduData & 0x08);
//  expectingReply = (*pduData & 0x04);         // might be nice to check these someday
//  networkPriority = (*pduData & 0x03);        // perhaps filter all critical messages?
    pduData += 1;
    pduLen -= 1;
    
    // extract the destination address
    if (dnetPresent) {
        int     dnet, dlen
        ;
        
        dnet = (pduLen--,*pduData++);
        dnet = (dnet << 8) + (pduLen--,*pduData++);
        dlen = (pduLen--,*pduData++);
        
        if (dnet == 0xFFFF)
            filterDADR.GlobalBroadcast();
        else {
            if (dlen == 0)
                filterDADR.RemoteBroadcast( dnet );
            else {
                filterDADR.RemoteStation( dnet, pduData, dlen );
                pduData += dlen;
            }
        }
    }
    
    // extract the source address, or copy the one from the endpoint
    if (snetPresent) {
        int     snet, slen
        ;
        
        snet = (pduLen--,*pduData++);
        snet = (snet << 8) + (pduLen--,*pduData++);
        slen = (pduLen--,*pduData++);
        
        filterSADR.RemoteStation( snet, pduData, slen );
        pduData += slen;
    }
    
    // all done for network layer messages
    if (netLayerMessage) {
        filterNPCIFn = *pduData;
        return;
    }

    // skip the hop count
    if (dnetPresent)
        pduLen--, pduData++;
    
    // sitting at the application layer
    filterAPDUFn = (*pduData) >> 4;

    // parse just a little deeper
    switch ((BACnetAPDUType)filterAPDUFn) {
        case confirmedRequestPDU:
            filterConfirmedFn = (pduData[0] & 0x08) ? pduData[5] : pduData[3];
            break;
        case unconfirmedRequestPDU:
            filterUnconfirmedFn = pduData[1];
            break;
        case simpleAckPDU:
            filterConfirmedFn = pduData[2];
            break;
        case complexAckPDU:
            filterConfirmedFn = (pduData[0] & 0x08) ? pduData[4] : pduData[2];
            break;

        case segmentAckPDU:
            // no service available
            break;

        case errorPDU:
            filterConfirmedFn = pduData[2];
            break;

        case rejectPDU:
        case abortPDU:
            break;
    }
}

//
//  BFRFilterSetFactory
//

BFRFactoryChild gBFRFilterSetChildren[] =
    { { "Accept", &gBFRFilterElemFactory }
    , { "Reject", &gBFRFilterElemFactory }
    , { 0, 0 }
    };

BFRFilterSetFactory gBFRFilterSetFactory;

//
//  BFRFilterSetFactory
//

BFRFilterSetFactory::BFRFilterSetFactory( void )
    : BFRFactory(gBFRFilterSetChildren)
{
}

//
//  BFRFilterSetFactory::StartElement
//

voidPtr BFRFilterSetFactory::StartElement( const char *name, const MinML::AttributeList& attrs )
{
    // return a new filter set
    return new BFRFilterSet();
}

//
//  BFRFilterSetFactory::ChildElement
//

void BFRFilterSetFactory::ChildElement( voidPtr ep, int id, voidPtr cp )
{
    BFRFilterSetPtr curFilter = (BFRFilterSetPtr)ep
    ;

    // child elements are accept or reject filter elements, just add them to the set
    curFilter->AddElem( (BFRFilterElemPtr)cp );
}

//
//  BFRFilter::BFRFilter
//

BFRFilter::BFRFilter( void )
    : filterUpstream(0), filterDownstream(0)
{
}

//
//	BFRFilter::~BFRFilter
//

BFRFilter::~BFRFilter( void )
{
}

//
//  BFRFilter::Indication
//
//  Filters get an indication when a client is requesting to send a packet.  If the 
//  downstream filter set passes, send it along to the server.
//

void BFRFilter::Indication( const BACnetPDU &pdu )
{
    if (filterDownstream && filterDownstream->Test(pdu))
        Request(pdu);
}

//
//  BFRFilter::Confirmation
//
//  Filters get a confirmation when a server is sending a packet upstream to a client.
//  if the upstream filter set passes, send it along to the client.
//

void BFRFilter::Confirmation( const BACnetPDU &pdu )
{
    if (filterUpstream && filterUpstream->Test(pdu))
        Response(pdu);
}

//
//  BFRFilterFactory
//

BFRFactoryChild gBFRFilterFactoryChildren[] =
    { { "Upstream", &gBFRFilterSetFactory }
    , { "Downstream", &gBFRFilterSetFactory }
    , { 0, 0 }
    };

BFRFilterFactory gBFRFilterFactory;

//
//  BFRFilterFactory::BFRFilterFactory
//

BFRFilterFactory::BFRFilterFactory( void )
    : BFRFactory( gBFRFilterFactoryChildren )
{
}

//
//  BFRFilterFactory::StartElement
//

voidPtr BFRFilterFactory::StartElement( const char *name, const MinML::AttributeList& attrs )
{
    BFRFilterPtr    curFilter = new BFRFilter()
    ;

    // register as a client and server
    gBFRRegistration.RegisterClient( SubstituteArgs(attrs["client"]), curFilter );
    gBFRRegistration.RegisterServer( SubstituteArgs(attrs["server"]), curFilter );

    // return a new filter
    return curFilter;
}

//
//  BFRFilterFactory::ChildElement
//

void BFRFilterFactory::ChildElement( voidPtr ep, int id, voidPtr cp )
{
    BFRFilterPtr	curFilter = (BFRFilterPtr)ep
    ;

    switch (id) {
        case 0:
            // already an upstream filter?
            if (curFilter->filterUpstream)
                throw_1(8003);      // already an upstream filter

            curFilter->filterUpstream = (BFRFilterSetPtr)cp;
            break;
        case 1:
            // already a downstream filter?
            if (curFilter->filterDownstream)
                throw_1(8004);      // already a downstream filter

            curFilter->filterDownstream = (BFRFilterSetPtr)cp;
            break;
    }
}
