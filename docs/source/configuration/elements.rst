.. BFR Elements and Attributes

.. highlight:: xml

.. _elements-and-attributes:

Elements and Attributes
=======================

The BFR configuration file is an XML document::

    <BFR>
        Configuration and documentation.
    </BFR>

The XML child elements are names of BFR components that are "stacked" together.  The elements 
can be specified in any order, normally the files have their elements starting with the bottom 
of the stack.

Elements come in flavors of **clients**, **servers**, or both.  A :term:`server` is a kind of element 
that sits at the bottom layers of the stack and communicates via packets to a :term:`client`.  An element 
that provides communications on a UDP socket would be a server, and its associated client would 
do things with the packets that are received.  The client might also generate packets, in which 
case the server would take care of transmitting them via the socket library.

Packets that go from a server to a client are said to be going :term:`upstream`, going up the protocol 
stack.  Likewise packets that go from a client to a server are said to be going :term:`downstream`.

Binding Clients and Servers
---------------------------

The XML elements that can be servers have a ``server`` attribute which is a simple label.  Similarly, 
XML elements that are or can be a client have a ``client`` attribute.  When the server elements 
and client elements have matching labeles, they are said to be :term:`bound` together.

This is a sample configuration file::

    <BFR>
        Make a debug element.

        <Debug server="test"/>

        Now make a console element and bind it to the debug element.

        <Console client="test"/>
    </BFR>

As a service the ``Debug`` element dumps the contents of packets it receives from its connected 
client and can then pass the packet downstream to another service.  In this case there is no additional 
service so the packet is simply dropped.

As a client the ``Console`` element will dump the contents of the packets it receives upstream from 
its connected service, and it will provide a prompt to send packets downstream when requested by the 
user.  In this case the ``Debug`` element will not send any packets upstream.

Here the user has given this sample configuration to the BFR application, then requested that a packet 
of four octets be sent to a broadcast address, which is then printed by the ``Debug`` element::

    $ ../BFR/BFR sample.bfr
    * 01020304
    * <- null : 01.02.03.04.

Note that the ``Debug`` element shows the packet going downstream by the directional arrow.  This 
sample configuration file flips the client and server relationship::

    <BFR>
        Make a console element that is a server.

        <Console server="test"/>

        Make and bind a debugging client.

        <Debug client="test"/>
    </BFR>

Now packets that are "sent" by the ``Console`` go upstream rather than downstream.  This is a way 
to provide sample packets into the BFR stack as if they have been received by some other component.  In
this case the debugging shows the packet going upstream::

    $ ../BFR/BFR sample.bfr
    * 01020304
    * -> null : 01.02.03.04.

Client and Server Combinations
------------------------------

The samples above show the same kind of component acting as a client and a server.  Debugging components 
can be a client, or a server, or both, and it is one of the few components that have that behavior.  There 
can be as many debugging components as needed in the configuration.

The ``Console`` component can be a client or a server, but not both at the same time, and the configuration 
file can only have one instance.


