//
//  BACnetPort
//

#ifndef _BACnetPort_
#define _BACnetPort_

#include <sys/time.h>

#include "BACnet.h"

//
//  BACnetPort
//
//  A BACnetPort is the lowest level of communications connection to the 
//  operating system.  It is a server, and the Indiction() member function 
//  transfers PDU content from the stack into a new portWriteList element.
//
//  A derived class of BACnet port must provide two functions:
//
//  The Read() function must read content from its socket and transfer it to
//  a new gProcessPDUList element.  It will be called when select() on the
//  socket indicates there is data to be read.
//
//  The Write() function must get a packet from its portWriteList and 
//  initiate sending it out the socket.  It will be called when there is 
//  a packet to send and the select() call indicates the socket is available 
//  for writing.
//

class BACnetPort : public BACnetServer {
        friend void ProcessPortIO(struct timeval &delay);

    protected:
        BACnetPort              *portNext;
        
    public:
        BACnetPort( void );
        virtual ~BACnetPort( void );

        int                     portStatus;         // non-zero iff something is wrong

        BACnetAddress           portLocalAddr;      // port has this address
        BACnetAddress           portBroadcastAddr;  // use this to broadcast (could be null)
        
        int                     portSocket;         // socket for communications
        BACnetPDUList           portWriteList;      // PDU's to be sent
        
        void Indication( const BACnetPDU &pkt );    // transfer to write list
        
        virtual void Init( void ) = 0;              // initialize as necessary
        
        virtual void Read( void ) = 0;              // read data from socket
        virtual void Write( void ) = 0;             // write data to socket
    };

typedef BACnetPort *BACnetPortPtr;

extern BACnetPortPtr gPortList;

void ProcessPortIO(struct timeval &delay);

#endif
