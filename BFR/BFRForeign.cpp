//
//  BFRForeign
//

#include <stdlib.h>
#include <string.h>

#include "BFRForeign.h"

#include "BFRRegistration.h"

//
//  BACnetBIPForeignStarter
//

class BACnetBIPForeignStarter : public BACnetTask {
    public:
        BACnetBIPForeignPtr     fPtr;

        BACnetBIPForeignStarter( BACnetBIPForeignPtr fp )
            : BACnetTask( BACnetTask::oneShotDeleteTask ), fPtr(fp)
        {
            InstallTask();
        }

        virtual ~BACnetBIPForeignStarter( void )
        {
        }

        virtual void ProcessTask( void )
        {
            fPtr->Register();
        }
    };

//
//  BACnetBIPForeignFactory
//

BACnetBIPForeignFactory gBACnetBIPForeignFactory;

//
//  BACnetBIPForeignFactory::StartElement
//

voidPtr BACnetBIPForeignFactory::StartElement( const char *name, const MinML::AttributeList& attrs )
{
    BACnetBIPForeignPtr     fp = new BACnetBIPForeign()
    ;

    // set the BBMD address
    fp->foreignBBMDAddr.Station( SubstituteArgs(attrs["bbmd"]) );
    fp->foreignTimeToLive = atoi( SubstituteArgs(attrs["ttl"]) );

    // register
    gBFRRegistration.RegisterClient( SubstituteArgs(attrs["client"]), fp );
    gBFRRegistration.RegisterServer( SubstituteArgs(attrs["server"]), fp );

    // do the registration thing when the task manager is ready
    new BACnetBIPForeignStarter( fp );

    // return the object
    return fp;
}
