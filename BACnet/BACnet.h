//
//  BACnet
//

#ifndef _BACnet_
#define _BACnet_

//
//  General Typedefs
//

typedef unsigned char BACnetOctet, *BACnetOctetPtr;     // unsigned character

const int kVendorID = 15;                               // Cornell

//
//  BACnet Address
//
//  These address types are listed in increasing order of complexity
//

const int kMaxAddressLen = 8;                           // longest address supported
const unsigned short kBACnetIPDefaultPort = 0xBAC0;     // default IP port

enum BACnetAddressType
    { nullAddr
    , localBroadcastAddr
    , localStationAddr
    , remoteBroadcastAddr
    , remoteStationAddr
    , globalBroadcastAddr
    };

struct BACnetAddress {
    BACnetAddressType   addrType;
    unsigned short      addrNet;
    unsigned short      addrLen;
    unsigned char       addrAddr[kMaxAddressLen];

    BACnetAddress( const char *spec );
    BACnetAddress( unsigned long host, unsigned short port = kBACnetIPDefaultPort );
    BACnetAddress( const unsigned char *addr, const unsigned short len );
    BACnetAddress( const unsigned short net, const unsigned char *addr, const unsigned short len );
    BACnetAddress( const BACnetAddressType typ = nullAddr, const unsigned short net = 0, const unsigned char *addr = 0, const unsigned short len = 0 );

    BACnetAddress &operator =( const BACnetAddress &arg );

    // initializers (when constructor can't be used)
    void Station( const char *spec );

    void Null( void );
    void LocalStation( const BACnetOctet addr );
    void LocalStation( const unsigned char *addr, const unsigned short len );
    void LocalStation( const unsigned long ipAddr, const unsigned short port );
    void RemoteStation( const unsigned short net, const BACnetOctet addr );
    void RemoteStation( const unsigned short net, const unsigned char *addr, const unsigned short len );

    void LocalBroadcast( void );
    void RemoteBroadcast( const short net );
    void GlobalBroadcast( void );

    void Pack( unsigned long host, unsigned short port );
    void Unpack( unsigned long *hostp, unsigned short *portp ) const;

    static void Pack( BACnetOctet *rsltp, unsigned long host, unsigned short port );
    static void Unpack( const BACnetOctet *rsltp, unsigned long *hostp, unsigned short *portp );

    static void StringToHostPort( const char *str, unsigned long *hostp, unsigned long *maskp, unsigned short *portp );

    const char * ToString( void ) const;
    };

typedef BACnetAddress *BACnetAddressPtr;
const int kBACnetAddressSize = sizeof( BACnetAddress );

bool operator ==( const BACnetAddress &addr1, const BACnetAddress &addr2 );

//
//  BACnetError
//

class BACnetError {
    private:
        BACnetError( const BACnetError &err );
        BACnetError& operator =( const BACnetError &err );

    public:
        const char  *errFile;
        const int   errLine;
        const int   errError;
        char        *errParm;

        BACnetError( const char *file, const int line, const int err, const char *parm );
        ~BACnetError( void );

        const char *Description( void ) const;
    };

#define throw_1(x)      throw new BACnetError( __FILE__, __LINE__, x, 0 )
#define throw_2(x,y)    throw new BACnetError( __FILE__, __LINE__, x, y )

//
//  BACnetPDUData
//
//  The packet data structure contains a pointer to the beginning of a chunk of 
//  data and how long it is.  It also contains the amount that was allocated, so 
//  if copying data into the buffer needs to resize it, it can.  If the data 
//  buffer is not owned by the packet data structure, the allocated length is zero.
//
//  There are two basic constructors.  The first allocates a buffer and keeps a 
//  pointer to it.  This is used for building packets using the two Put() functions.
//  The second is used to extract data from a buffer usin the Get() functions.  Note
//  that you can't mix the two buffer types, one is for reading and one is for writing.
//
//  The default (parameterless) constructor makes something that can't be used until
//  AllocData() or SetRefernce() is called.  The copy constructor makes a read-only 
//  reference to the data which may or may not be owned.
//

struct BACnetPDUData {
    public:
        BACnetOctetPtr          pduData;
        int                     pduLen;
        int                     pduAllocLen;

        BACnetPDUData( void );
        ~BACnetPDUData( void );

        BACnetPDUData( int allocLen );                      // allocate a buffer
        BACnetPDUData( const BACnetPDUData &pkt );          // point to a packet
        BACnetPDUData( BACnetOctetPtr data, int len );      // point to a buffer

        void Allocate( int allocLen );                      // allocate a buffer
        void SetReference( const BACnetPDUData &pkt );      // point to a packet
        void SetReference( BACnetOctetPtr data, int len );  // point to a buffer
        void Flush( void );                                 // delete buffer

        void Copy( const BACnetPDUData &pkt );              // point to a packet
        void Copy( BACnetOctetPtr data, int len );          // point to a buffer
        
        BACnetOctet Get( void );                            // extract an octet
        unsigned short GetShort( void );                    // extract an unsigned short
        unsigned long GetLong( void );                      // extract an unsigned long
        void Get( BACnetOctet *buff, int len );             // raw copy into buffer

        void CheckSpace( int size );                        // check for enough space
        void Put( BACnetOctet ch );                         // simple copy, should be inline!
        void PutShort( unsigned short n );                  // put an unsigned short
        void PutLong( unsigned long n );                    // put an unsigned long
        void Put( const BACnetOctet *buff, int len );       // raw copy into buffer
    };

//
//	BACnetPDU
//

struct BACnetPDU : public BACnetPDUData {
    public:
        BACnetAddress           pduSource;
        BACnetAddress           pduDestination;

        int                     pduExpectingReply;          // see 6.2.2 (1 or 0)
        int                     pduNetworkPriority;         // see 6.2.2 (0, 1, 2, or 3)
        
        BACnetPDU &operator =( const BACnetPDU &arg );      // copy everything
    };

typedef BACnetPDU *BACnetPDUPtr;

//
//  BACnetVPDU
//

const BACnetOctet   kBVLCType   = 0x81;

enum BVLCFunction
        { bvlcResult                            = 0x00
        , bvlcWriteBroadcastDistributionTable   = 0x01
        , bvlcReadBroadcastDistributionTable    = 0x02
        , bvlcReadBroadcastDistributionTableAck = 0x03
        , bvlcForwardedNPDU                     = 0x04
        , bvlcRegisterForeignDevice             = 0x05
        , bvlcReadForeignDeviceTable            = 0x06
        , bvlcReadForeignDeviceTableAck         = 0x07
        , bvlcDeleteForeignDeviceTableEntry     = 0x08
        , bvlcDistributeBroadcastToNetwork      = 0x09
        , bvlcOriginalUnicastNPDU               = 0x0A
        , bvlcOriginalBroadcastNPDU             = 0x0B
        };

struct BACnetVPDU : public BACnetPDUData {
    public:
        BVLCFunction            bvlcFunction;       // -1 == not a BVLL message

        BACnetAddress           bvlcAddress;        // used with forwarded NPDU's
        int                     bvlcResultCode;     // success/failure result code
    };

//
//  BACnetNPDU
//
//  An NPDU extends the PDU to separate the network layer contents.  For upstream packets, the 
//  contents are decoded from the PDU.  For downstream packets the contents are encoded.
//

enum BACnetNetworkMessage
        {   WhoIsRouterToNetwork            = 0x00
        ,   IAmRouterToNetwork              = 0x01
        ,   ICouldBeRouterToNetwork         = 0x02
        ,   RejectMessageToNetwork          = 0x03
        ,   RouterBusyToNetwork             = 0x04
        ,   RouterAvailableToNetwork        = 0x05
        ,   InitializeRoutingTable          = 0x06
        ,   InitializeRoutingTableAck       = 0x07
        ,   EstablishConnectionToNetwork    = 0x08
        ,   DisconnectConnectionToNetwork   = 0x09
        };

struct BACnetNPDU : public BACnetPDU {

    BACnetNPDU( void );                             // clear everything out
    
    BACnetNetworkMessage    npduNetMessage;         // -1 == not a network layer message
    int                     npduVendorID;           // vendor ID for network message
    int                     npduHopCount;           // hop count

    BACnetAddress           npduSADR;               // NPDU source address
    BACnetAddress           npduDADR;               // NPDU destination address
    };

typedef BACnetNPDU *BACnetNPDUPtr;

void Encode( BACnetPDU &pdu, const BACnetNPDU &npdu );
void Decode( BACnetNPDU &npdu, const BACnetPDU &pdu );

//
//  BACnetAPDU
//
//  A BACnetAPDU is used with the application layer.
//

enum BACnetAPDUType
        { confirmedRequestPDU       = 0x00
        , unconfirmedRequestPDU     = 0x01
        , simpleAckPDU              = 0x02
        , complexAckPDU             = 0x03
        , segmentAckPDU             = 0x04
        , errorPDU                  = 0x05
        , rejectPDU                 = 0x06
        , abortPDU                  = 0x07
        };

struct BACnetAPDU : public BACnetPDUData {
    public:
        BACnetAPDUType      apduType;               // -1 == not an application layer message

        bool                apduSeg;                // segmented
        bool                apduMor;                // more follows
        bool                apduSA;                 // segmented response accepted
        bool                apduSrv;                // sent by server
        bool                apduNak;                // negative acknowledgement
        int                 apduSeq;                // sequence number
        int                 apduWin;                // actual/proposed window size
        int                 apduMaxResp;            // max response accepted (decoded)
        int                 apduService;
        int                 apduInvokeID;
        int                 apduAbortRejectReason;

        BACnetPDUData       apduData;               // application encoded data
    };

void Decode( const BACnetNPDU &npdu, BACnetAPDU &apdu );
void Encode( const BACnetAPDU &apdu, BACnetNPDU &npdu );

//
//  BACnetClient
//  BACnetServer
//

class BACnetClient;
typedef BACnetClient *BACnetClientPtr;

class BACnetServer;
typedef BACnetServer *BACnetServerPtr;

class BACnetClient {
        friend void Bind( BACnetClientPtr, BACnetServerPtr );
        friend void Unbind( BACnetClientPtr, BACnetServerPtr );
        friend bool IsBound( BACnetClientPtr, BACnetServerPtr );

    protected:
        BACnetServerPtr     clientPeer;

    public:
        BACnetClient( void );
        virtual ~BACnetClient( void );

        void Request( const BACnetPDU &pkt );
        virtual void Confirmation( const BACnetPDU &pkt ) = 0;
    };

class BACnetServer {
        friend void Bind( BACnetClientPtr, BACnetServerPtr );
        friend void Unbind( BACnetClientPtr, BACnetServerPtr );
        friend bool IsBound( BACnetClientPtr, BACnetServerPtr );

    protected:
        BACnetClientPtr     serverPeer;

    public:
        BACnetServer( void );
        virtual ~BACnetServer( void );

        virtual void Indication( const BACnetPDU &pkt ) = 0;
        void Response( const BACnetPDU &pkt );
    };

//
//  BACnetPDUList
//
//  There is a global list of packets that need to be processed, they have
//  been read in by some port.  Every BACnetPort also has a list of packets
//  that need to be written.  These lists are maintained in descending order
//  by pduNetworkPriority.
//
//  There is a global list of free packets to keep the number of malloc/free
//  calls to a minimum.
//

class BACnetPort;
typedef BACnetPort *BACnetPortPtr;

const int kDefaultListLen = 100;                    // how many to allocate
const int kDefaultListPDUSize = 2048;               // how much to allocate

struct BACnetPDUListElement {
    BACnetPDUListElement( void );
    
    BACnetPDUListElement    *listNext;              // next in list
    BACnetPortPtr           listPort;               // which port
    BACnetPDU               listPDU;                // PDU contents
    };

typedef BACnetPDUListElement *BACnetPDUListElementPtr;

class BACnetPDUList {
    protected:
        BACnetPDUListElementPtr     listHead;                   // list head
        BACnetPDUListElementPtr     listTail;                   // list tail

    public:
        BACnetPDUList( void );

        inline bool IsEmpty( void ) { return (listHead == 0); } // empty list
        
        void Write( BACnetPDUListElementPtr lep );              // add to the list
        BACnetPDUListElementPtr Read( void );                   // remove from list
    };

typedef BACnetPDUList *BACnetPDUListPtr;

extern BACnetPDUList gProcessPDUList;
extern BACnetPDUList gFreePDUList;

void AllocateFreePDUList( void );                   // allocate at startup

#endif
