//
//  BACnetTask
//

#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <math.h>

#include "BACnetTask.h"

//-----------------------------------------------------------------------------//

inline bool operator < ( const timeval &t1, const timeval &t2 )
{
  return t1.tv_sec < t2.tv_sec ||
         (t1.tv_sec == t2.tv_sec && t1.tv_usec < t2.tv_usec);
}

inline bool operator > ( const timeval &t1, const timeval &t2 )
{
  return t1.tv_sec > t2.tv_sec ||
         (t1.tv_sec == t2.tv_sec && t1.tv_usec > t2.tv_usec);
}

inline bool operator == ( const timeval &t1, const timeval &t2 )
{
  return (t1.tv_sec == t2.tv_sec && t1.tv_usec == t2.tv_usec);
}

inline timeval& operator << (timeval &t, double val)
{
    double  integral
    ;
    
    t.tv_usec = (long)(1000000 * modf(val,&integral) + 0.5);
    t.tv_sec  = (long)integral;
    
    return t;
}

inline timeval operator + ( const timeval &t1, const timeval &t2 )
{
    struct timeval  tmp
    ;
    
    tmp.tv_sec = t1.tv_sec + t2.tv_sec;
    if ((tmp.tv_usec = t1.tv_usec + t2.tv_usec) >= 1000000) {
        ++tmp.tv_sec;
        tmp.tv_usec -= 1000000;
    }

    return tmp;
}

inline timeval& operator += ( timeval &t1, const timeval &t2 )
{
    t1.tv_sec += t2.tv_sec;
    if ((t1.tv_usec += t2.tv_usec) >= 1000000) {
        ++t1.tv_sec;
        t1.tv_usec -= 1000000;
    }
    
    return t1;
}

inline timeval operator - ( const timeval &t1, const timeval &t2 )
{
    struct timeval  tmp
    ;
    
    tmp.tv_sec = t1.tv_sec - t2.tv_sec;
    if ( (tmp.tv_usec = t1.tv_usec - t2.tv_usec) < 0 ) {
        --tmp.tv_sec;
        tmp.tv_usec += 1000000;
    }
    
    return tmp;
}

inline timeval& operator -= ( timeval &t1, const timeval &t2 )
{
    t1.tv_sec -= t2.tv_sec;
    if ( (t1.tv_usec -= t2.tv_usec) < 0 ) {
        --t1.tv_sec;
        t1.tv_usec += 1000000;
    }
    
    return t1;
}

//-----------------------------------------------------------------------------//

const char* strtimeval(const timeval& t)
{
    static char buff[20]
    ;
    
    sprintf( buff, "%d.%06d", (int)t.tv_sec, (int)t.tv_usec );
    return buff;
}

//-----------------------------------------------------------------------------//

BACnetTaskPtr   gTaskList = 0;

//
//  BACnetTask::BACnetTask
//
//	Note that the task is NOT installed.  You can create a task of a specific type 
//	then install it when it needs to be scheduled.
//

BACnetTask::BACnetTask( BACnetTaskType typ, long delay )
{
	taskType = typ;
	taskInterval = delay;
}

//
//	When the task goes away, make sure it's not in the task list.
//

BACnetTask::~BACnetTask( void )
{
	::UninstallTask( this );
}

//
//	BACnetTask::InstallTask
//

void BACnetTask::InstallTask( void )
{
	::InstallTask( this );
}

//
//	BACnetTask::SuspendTask
//

void BACnetTask::UninstallTask( void )
{
	::UninstallTask( this );
}

//
//	InstallTask
//

void InstallTask( BACnetTaskPtr tp )
{
    struct timeval  delta
    ;
    BACnetTaskPtr   *th = &gTaskList
    ;

    // get current time of day
    gettimeofday( &tp->taskTime, NULL );
    
    // if there is an interval, add the offset
    if (tp->taskInterval != 0) {
        delta << (double)tp->taskInterval / 1000.0;
        tp->taskTime += delta;
    }
    
    // schedule it
    while (*th)
        if (tp->taskTime < (*th)->taskTime)
            break;
        else
            th = &(*th)->taskNext;

    // insert it
    tp->taskNext = *th;
    *th = tp;
}

//
//	UninstallTask
//

void UninstallTask( BACnetTaskPtr tp )
{
    BACnetTaskPtr  *th = &gTaskList
    ;

    // hunt through the list
    while (*th)
        if (*th == tp)
            break;
        else
            th = &(*th)->taskNext;

    // remove it
    if (*th)
        *th = tp->taskNext;
}

//
//  ProcessTasks
//
//  Let each task have a chance to run.  When they're all done,
//  update delta to reflect the amount of time that the application
//  should wait before processing the next round of tasks.
//

void ProcessTasks( timeval &delay )
{
    struct timeval  now, delta
    ;
    BACnetTaskPtr   tp, *th
    ;

    // get current time of day
    gettimeofday( &now, NULL );
    
    for (;;) {
        // look for something
        if (!gTaskList)
            break;

        // could be all done
        if (gTaskList->taskTime > now) {
            delta = gTaskList->taskTime - now;
            if (delta < delay)
                delta = delay;
            break;
        }
        
        // unlink it
        tp = gTaskList;
        gTaskList = tp->taskNext;
        tp->taskNext = 0;

        // run it
        tp->ProcessTask();

        // perhaps reschedule it
        if (tp->taskType == BACnetTask::recurringTask) {
            delta << (double)tp->taskInterval / 1000.0;
            tp->taskTime += delta;

            // schedule it
            th = &gTaskList;
            while (*th)
                if (tp->taskTime < (*th)->taskTime)
                    break;
                else
                    th = &(*th)->taskNext;

            // insert it
            tp->taskNext = *th;
            *th = tp;
        } else
        // perhaps delete it
        if (tp->taskType == BACnetTask::oneShotDeleteTask)
            free(tp);
    }
}
