//
//  BACnetTask
//

#ifndef _BACnetTask_
#define _BACnetTask_

#include <sys/time.h>

//
//  BACnetTask
//

class BACnetTask {
    public:
        enum BACnetTaskType
                { oneShotTask       = 0
                , oneShotDeleteTask = 1
                , recurringTask     = 2
                };

        BACnetTaskType  taskType;               // how to process
        long            taskInterval;           // how often to reschedule (ms)
        
        struct timeval  taskTime;               // when it is scheduled
        BACnetTask      *taskNext;              // next scheduled task

        BACnetTask( BACnetTaskType typ = oneShotTask, long delay = 0 );
        virtual ~BACnetTask(void);

        void InstallTask(void);                 // install into queue
        void UninstallTask(void);               // remove from execution queue

        virtual void ProcessTask(void) = 0;     // do something
    };

typedef BACnetTask *BACnetTaskPtr;

void InstallTask( BACnetTaskPtr tp );
void UninstallTask( BACnetTaskPtr tp );

void ProcessTasks( timeval &delay );

#endif
