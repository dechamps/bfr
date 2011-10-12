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
    if (errParm)
        delete[] errParm;
}

//
//  BACnetError::Description
//

const char *BACnetError::Description( void ) const
{
    static char	line[256]
    ;

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
            sprintf( line, "unmatched environment variable, %s", errParm );
            return line;

        case 2001:
            return "invalid address type";
        case 2002:
            sprintf( line, "FDT overflow, %s", errParm );
            return line;
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
            sprintf( line, "registration maximum name length exceeded, %s", errParm );
            return line;
        case 9002:
            sprintf( line, "client already registered, %s", errParm );
            return line;
        case 9003:
            sprintf( line, "server already registered, %s", errParm );
            return line;
        case 9004:
            sprintf( line, "unmatched registration, %s", errParm );
            return line;

        case 10001:
            return "misconfigured switch port";

        case 11001:
            return "no matching adapter or description";
        case 11002:
            sprintf( line, "no such adapter available, %s", errParm );
            return line;
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
                sprintf( line, "unknown error %d, %s", errError, errParm );
            else
                sprintf( line, "unknown error %d", errError );
            return line;
    }
}
