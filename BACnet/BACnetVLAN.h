//
//  BACnetVLAN
//

#ifndef _BACnetVLAN_
#define _BACnetVLAN_

#include "BACnet.h"

//  Forward Declarations

class BACnetVLANNode;
typedef BACnetVLANNode *BACnetVLANNodePtr;

class BACnetVLAN;
typedef BACnetVLAN *BACnetVLANPtr;

//
//  BACnetVLANNode
//

class BACnetVLANNode : public BACnetServer {
        friend class BACnetVLAN;

    protected:
        BACnetVLANNodePtr	nodeNext;

    public:
        BACnetVLANNode( void );
        ~BACnetVLANNode( void );

        BACnetAddress       nodeAddress;                // LAN address
        BACnetVLANPtr       nodeLAN;                    // install LAN
        bool                nodePromiscuous;            // also allows spoofing

        virtual void Indication( const BACnetPDU &pkt );
    };

//
//  BACnetVLAN
//

class BACnetVLAN {
        friend class BACnetVLANNode;

    protected:
        BACnetVLANNodePtr   vlanNodeList;

        void ProcessMessage( BACnetVLANNodePtr np, const BACnetPDU &pd );

    public:
        BACnetVLAN( void );
        ~BACnetVLAN( void );

        void AddNode( BACnetVLANNodePtr np );           // add a node to the list
        void RemoveNode( BACnetVLANNodePtr np );        // remove a node from the list
    };

#endif
