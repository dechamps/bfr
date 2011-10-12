//
//  BFRRegistration
//

#ifndef _BFRRegistration_
#define _BFRRegistration_

#include "BACnet.h"

//
//  BFRRegistration
//

const int kBFRRegNameLen = 16;

class BFRRegistration {
    protected:
        struct BFRRegItem {
            char                regName[kBFRRegNameLen];
            BACnetClientPtr     regClient;
            BACnetServerPtr     regServer;

            BFRRegItem          *regNext;
            };

        typedef BFRRegItem *BFRRegItemPtr;

        BFRRegItemPtr           regList;

    public:
        BFRRegistration( void );

        void RegisterClient( const char *name, BACnetClientPtr cp );
        void RegisterServer( const char *name, BACnetServerPtr sp );

        void ValidateRegistrations( void );
    };

typedef BFRRegistration *BFRRegistrationPtr;

//

extern BFRRegistration gBFRRegistration;

#endif
