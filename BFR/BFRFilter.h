//
//  BFRFilter
//

#ifndef _BFRFilter_
#define _BFRFilter_

#include "BACnet.h"

#include "BFRParser.h"

class BFRFilterElem;
typedef BFRFilterElem *BFRFilterElemPtr;

class BFRFilterSet;
typedef BFRFilterSet *BFRFilterSetPtr;

//
//  BFRAddressFilter
//

enum BFRAddressFilterType
        { noTest                // always passes
        , localStation          // n                    specific local station
                                // xx-xx-xx-xx-xx-xx    alternate format
        , ipLocalStation        // n.n.n.n[/n][:n]      specific ip local station (with network mask)
        , localNetwork          // :                    any local station
        , localBroadcast        // *                    local broadcast
        , remoteStation         // n:n                  specific remote station
                                // n:xx-xx-xx-xx-xx-xx  alternate format
        , ipRemoteStation       // n:n.n.n.n[/n][:n]    specific ip remote station
        , remoteNetwork         // n:                   any station on a specific remote network
        , remoteBroadcast       // n:*                  specific remote broadcast
        , remoteNetworkAny      // *:                   any remote station
        , globalBroadcast       // *:*                  global broadcast
        };

class BFRAddressFilter {
    public:
        BFRAddressFilterType    filterType;             // one of the match types above
        BACnetAddress           filterAddress;          // address components
        unsigned long           filterMask;             // for IP addresses
        unsigned short          filterPort;             // ibid

        BFRAddressFilter( void );

        void SetFilter( const char *desc );             // parse ASCII string

        bool Test( const BACnetAddress &addr );         // true iff passes the test
    };

typedef BFRAddressFilter *BFRAddressFilterPtr;

//
//  Keyword Table
//

struct KeywordTable {
    long    keyword;
    int     value;
    };

typedef KeywordTable *KeywordTablePtr;

int Lookup( long code, KeywordTablePtr tp );

//
//  BFRFunctionFilter
//

class BFRFunctionFilter {
    public:
        bool                    filtering;              // true iff something specified
        int                     filterBVLCMask;         // BVLL function mask
        int                     filterNPCIMask;         // network function mask
        int                     filterAPDUMask;         // APDU type mask
        int                     filterConfirmedMask;    // confirmed service functions
        int                     filterUnconfirmedMask;  // unconfirmed service functions

        BFRFunctionFilter( void );

        void SetFilter( const char *desc );             // parse ASCII string

        bool Test( BFRFilterSetPtr fsp );               // true iff passes the test
    };

typedef BFRFunctionFilter *BFRFunctionFilterPtr;

//
//  BFRFilterElem
//

class BFRFilterElem {
        friend class BFRFilterSet;

    protected:
        BFRFilterElemPtr        filterNext;             // next filter in list

    public:
        bool                    filterAccept;           // true iff 'accept' filter element

        BFRFilterElem( void );

        BFRAddressFilter        filterSource;           // source
        BFRAddressFilter        filterDestination;      // destination
        BFRFunctionFilter       filterFunction;         // function

        int Test( const BACnetPDU &pdu, BFRFilterSetPtr fsp );  // -1=reject, 0=no decision, 1=accept
    };

//
//  BFRFilterElemFactory
//
//  This class returns a filter element for Accept and Reject elements.
//

class BFRFilterElemFactory : public BFRFactory {
    public:
        virtual voidPtr StartElement( const char *name, const MinML::AttributeList& attrs );
    };

typedef BFRFilterElemFactory *BFRFilterElemFactoryPtr;

extern BFRFilterElemFactory gBFRFilterElemFactory;

//
//  BFRFilterSet
//

class BFRFilterSet {
        friend class BFRFilterElem;

    protected:
        BFRFilterElemPtr        filterFirst;                // first element in list

    public:
        int                     filterBVLCFn;               // BVLL function
        BACnetAddress           filterOriginatingDevice;    // for forwarded packets

        int                     filterNPCIFn;               // network function
        BACnetAddress           filterSADR;                 // source address
        BACnetAddress           filterDADR;                 // destination address

        int                     filterAPDUFn;               // APDU type
        int                     filterConfirmedFn;          // confirmed service
        int                     filterUnconfirmedFn;        // unconfirmed service

        BFRFilterSet( void );
        virtual ~BFRFilterSet( void );

        void AddElem( BFRFilterElemPtr fep );
        void RemoveElem( BFRFilterElemPtr fep );

        int Length( void );
        BFRFilterElemPtr GetElem( int i );

        void Analyze( const BACnetPDU &pdu );           // look for functions
        bool Test( const BACnetPDU &pdu );              // true iff packet passes
    };

//
//  BFRFilterSetFactory
//

class BFRFilterSetFactory : public BFRFactory {
    public:
        BFRFilterSetFactory( void );

        virtual voidPtr StartElement( const char *name, const MinML::AttributeList& attrs );
        virtual void ChildElement( voidPtr ep, int id, voidPtr cp );
    };

extern BFRFilterSetFactory gBFRFilterSetFactory;

//
//  BFRFilter
//

class BFRFilter : public BACnetClient, public BACnetServer {
    public:
        BFRFilterSetPtr     filterUpstream;
        BFRFilterSetPtr     filterDownstream;

        BFRFilter( void );
        virtual ~BFRFilter( void );

        virtual void Indication( const BACnetPDU &pdu );
        virtual void Confirmation( const BACnetPDU &pdu );
    };

typedef BFRFilter *BFRFilterPtr;

//
//	BFRFilterFactory
//

class BFRFilterFactory : public BFRFactory {
    public:
        BFRFilterFactory( void );

        virtual voidPtr StartElement( const char *name, const MinML::AttributeList& attrs );
        virtual void ChildElement( voidPtr ep, int id, voidPtr cp );
    };

extern BFRFilterFactory gBFRFilterFactory;

#endif
