//
//  BACnetError
//

#include <stdio.h>
#include <string.h>

#include "BACnet.h"

//
//  BACnetError::BACnetError
//

BACnetError::BACnetError( const char *file, const int line, const int err, const char *parm )
    : errFile(file), errLine(line), errError(err), errParm(0)
{
#if _DEBUG
    printf("BACnetError::BACnetError\n");
#endif
    if (parm) {
        errParm = new char[strlen(parm)+1];
        strcpy( errParm, parm );
    }
}

//
//  BACnetError::~BACnetError
//

BACnetError::~BACnetError( void )
{
#if _DEBUG
    printf("BACnetError::~BACnetError\n");
#endif
    if (errParm)
        delete[] errParm;
}

//
//  BACnetError::GetDescription
//

char BACnetError::buff[256];

const char *BACnetError::GetDescription( void ) const
{
    switch (errError) {
        case 1000:
            return "invalid address types for comparison";
        case 1001:
            return "invalid address format";
        case 1002:
            return "invalid IP address format";
        case 1003:
            return "invalid address filter format";
        case 1004:
            return "invalid buffer access";
        case 1005:
            return "unexpected end of data";
        case 1006:
            return "unbound client";
        case 1007:
            return "unbound server";
        case 1008:
            return "client or server already bound";
        case 1009:
            return "client or server already unbound";
        case 1010:
            return "not enough arguments";
        case 1011:
            sprintf( buff, "unmatched environment variable, %s", errParm );
            return buff;
        case 1012:
            sprintf( buff, "unknown option: %s", errParm );
            return buff;

        case 2001:
            return "invalid address type";
        case 2002:
            sprintf( buff, "FDT overflow, %s", errParm );
            return buff;
        case 2003:
            return "BBMD peer table overflow";
        case 2004:
            return "BBMD peer not found";
        case 2005:
            return "BBMD peer address required";

        case 3001:
            return "invalid address type (foreign)";

        case 4001:
            return "invalid address type (simple)";

        case 5001:
            return "register as a client";
        case 5002:
            return "out of adapter space";
        case 5003:
            return "already an adapter for this network";
        case 5004:
            return "malformed PDU, source can't be a global broadcast";
        case 5005:
            return "malformed PDU, source can't be a remote broadcast";
        case 5006:
            return "unknown network message type";
        case 5007:
            return "can't send to a null address";
        case 5008:
            return "no local network defined";
        case 5009:
            return "server required";
        case 5010:
            return "no adapter for source network";
        case 5011:
            sprintf( buff, "already a route to this network via %s", errParm );
            return buff;
        case 5012:
            return "routing table overflow";

        case 6001:
            return "unbound VLAN node";
        case 6002:
            return "spoofing attempt";

        case 7001:
            return "do not register as both a client and a server";
        case 7002:
            return "register as a client or a server";

        case 8001:
            return "invalid address format";
        case 8002:
            return "invalid IP address format";
        case 8003:
            return "already an upstream filter";
        case 8004:
            return "already a downstream filter";
        case 8005:
            return "unrecognized keyword";

        case 9001:
            sprintf( buff, "registration maximum name length exceeded, %s", errParm );
            return buff;
        case 9002:
            sprintf( buff, "client already registered, %s", errParm );
            return buff;
        case 9003:
            sprintf( buff, "server already registered, %s", errParm );
            return buff;
        case 9004:
            sprintf( buff, "unmatched registration, %s", errParm );
            return buff;

        case 10001:
            return "misconfigured switch port";

        case 11001:
            return "no matching adapter or description";
        case 11002:
            sprintf( buff, "no such adapter available, %s", errParm );
            return buff;
        case 11003:
            return "spoofing attempt";

        case 12001:
            return "address required";
        case 12002:
            return "server required";
        case 12003:
	       return "device required";

        default:
            if (errParm)
                sprintf( buff, "unknown error %d, %s", errError, errParm );
            else
                sprintf( buff, "unknown error %d", errError );
            return buff;
    }
}

