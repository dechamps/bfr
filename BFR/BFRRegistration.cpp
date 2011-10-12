//
//  BFRRegistration
//

#include <string.h>

#include "BFRRegistration.h"

//
//  Globals
//

BFRRegistration gBFRRegistration;

//
//  BFRRegistration::BFRRegistration
//

BFRRegistration::BFRRegistration( void )
    : regList(0)
{
}

//
//  BFRRegistration::RegisterClient
//

void BFRRegistration::RegisterClient( const char *name, BACnetClientPtr cp )
{
    BFRRegItemPtr   rip
    ;

    // check the name length
    if (strlen(name) >= kBFRRegNameLen)
        throw_2(9001,name);             // name length exceeded

    // look for an existing registration
    for (rip = regList; rip; rip = rip->regNext)
        if (strcmp(name,rip->regName) == 0) {
            if (rip->regClient)
                throw_2(9002,name);     // already registered

            // saved the client and bind
            rip->regClient = cp;
            Bind( cp, rip->regServer );

            break;
        }

    // nothing found
    if (!rip) {
        // new item
        rip = new BFRRegItem;
        strcpy( rip->regName, name );
        rip->regClient = cp;
        rip->regServer = 0;

        // link it in
        rip->regNext = regList;
        regList = rip;
    }
}

//
//  BFRRegistration::RegisterServer
//

void BFRRegistration::RegisterServer( const char *name, BACnetServerPtr sp )
{
    BFRRegItemPtr   rip
    ;

    // check the name length
    if (strlen(name) >= kBFRRegNameLen)
        throw_2(9001,name);             // name length exceeded

    // look for an existing registration
    for (rip = regList; rip; rip = rip->regNext)
        if (strcmp(name,rip->regName) == 0) {
            if (rip->regServer)
                throw_2(9003,name);     // already registered

            // saved the client and bind
            rip->regServer = sp;
            Bind( rip->regClient, sp );

            break;
        }

    // nothing found
    if (!rip) {
        // new item
        rip = new BFRRegItem;
        strcpy( rip->regName, name );
        rip->regClient = 0;
        rip->regServer = sp;

        // link it in
        rip->regNext = regList;
        regList = rip;
    }
}

//
//  BFRRegistration::ValidateRegistrations
//

void BFRRegistration::ValidateRegistrations( void )
{
    BFRRegItemPtr rip
    ;

    // look for unmatched pairs
    for (rip = regList; rip; rip = rip->regNext)
        if (!(rip->regClient && rip->regServer))
            throw_2(9004,rip->regName);     // unmatched registration
}
