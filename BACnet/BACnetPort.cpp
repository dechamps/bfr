//
//  BACnetPort
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <ctype.h>
#include <error.h>
#include <errno.h>

#include "BACnet.h"
#include "BACnetPort.h"

//
//  The global port list is used to determine which ports have valid sockets 
//  to set up the bit mask for the select() call.
//

BACnetPortPtr gPortList = 0;

//
//  BACnetPort::BACnetPort
//
//  This ctor sets the port status to uninitialized.  It will be up to a derived 
//  class to clear the status when the port is up and running.
//

BACnetPort::BACnetPort( void )
    : portStatus(-1)
    , portSocket(0), portWriteList()
{
    // link it into the other ports
    portNext = gPortList;
    gPortList = this;
}

//
//  BACnetPort::~BACnetPort
//
//  This dtor doesn't look like it does much, but it is virtual.  So when a pointer 
//  to a BACnetPort is deleted, the real derived class dtor is called.  All classes 
//  that have a virtual member function should have a virtual dtor.
//

BACnetPort::~BACnetPort( void )
{
}

//
//  BACnetPort::Indication
//
//  This function transfers the content of a PDU into a list element that can later 
//  be sent out a socket.  There may be quite a few packets that get queued up, and 
//  because some function in the stack owns the content, this must copy the data 
//  to someplace the application won't touch.
//
//  The global free list of PDU buffers is used to keep the number of malloc's and 
//  free's to a minimum (assuming that these are time consuming).
//

void BACnetPort::Indication( const BACnetPDU &pkt )
{
    BACnetPDUListElementPtr lep
    ;
    
    // if our port is not OK, drop the request
    if (portStatus != 0)
        return;
        
    // get outselves a list element
    lep = gFreePDUList.Read();
    if (!lep) {
        lep = new BACnetPDUListElement();
        
        // could run out of RAM, packet dropped
        if (!lep) return;
    }
    
    // copy the packet contents
    lep->listPDU = pkt;
    lep->listPort = 0;
    
    // save it on our write list
    portWriteList.Write( lep );
}

//
//  ProcessPortIO
//

void ProcessPortIO(struct timeval &delay)
{
    int                 i, r, n
    ;
    fd_set              rd, wr, er
    ;
    BACnetPortPtr       pp
    ;
    BACnetPDUListElementPtr lep
    ;
    
    /* clear everything out */
    FD_ZERO( &rd );
    FD_ZERO( &wr );
    FD_ZERO( &er );

    /* max socket number */
    n = -1;
    
    // go through the ports
    pp = gPortList;
    for (pp = gPortList; pp; pp = pp->portNext) {
        // skip ports that aren't working
        if (pp->portStatus != 0) {
            // ### this should be logged
            continue;
        }
        
        // find the maximum socket number
        if (pp->portSocket > n)
            n = pp->portSocket;
            
        // set the read
        FD_SET( pp->portSocket, &rd );
        
        // set the bit if there's data to write
        if (!pp->portWriteList.IsEmpty())
            FD_SET( pp->portSocket, &wr );
            
        // set the exception bit
        FD_SET( pp->portSocket, &er );
    }
    
    // wait for something to happen
    r = select( n + 1, &rd, &wr, &er, &delay );
    if (r == -1 && errno == EINTR)
        return;
        
    // check for errors
    if (r < 0) {
        perror("select");
        exit (1);
    }
    
    // timer expired
    if (r == 0)
        return;
    
    // see which port can have activity
    pp = gPortList;
    for (pp = gPortList; pp; pp = pp->portNext) {
        // skip ports that aren't working
        if (pp->portStatus != 0)
            continue;
        
        // see if there is data to read
        if (FD_ISSET( pp->portSocket, &rd ))
            pp->Read();
            
        // see if there is data to write
        if (FD_ISSET( pp->portSocket, &wr ))
            pp->Write();
    }
    
    // process the packets
    while ((lep = gProcessPDUList.Read()) != 0) {
        // hand off the data to the port
        lep->listPort->Response( lep->listPDU );
        
        // return the buffer to the free list
        gFreePDUList.Write( lep );
    }
}
