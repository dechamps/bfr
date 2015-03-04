.. BFR Router

.. highlight:: xml

Router
======

*Router* elements are used to route traffic between BACnet networks.  Routers are traditionally  
used to manage packets between two different physical networks, such as an ARCNET and Ethernet 
network.

.. _Router:

Router
------

There are two forms of the router element.  The first form specifies a client and server::

    <Router client="client_label" server="server_label" />

This form will process the BACnet network layer and present application layer PDUs to higher 
layers.  It makes it easier to use the console to enter packets by adding an appropriate 
NPCI header to the front of downstream packets and stripping off the NPCI for packets coming 
upstream.

The second form has a collection of child ``Adapter`` elements, one for each of the connected 
networks::

    <Router>
        <Adapter client="client_label" net="network" />
        ...
        <MAdapter net="net_number" mlan="mlan_label" />
            ...
        </MAdapter>
        ...
    </Router>

There is no limit to the number of adapter elements.

.. _Adapter:

Adapter
~~~~~~~

The ``Adapter`` element associates the top of some stack of elements to a specific BACnet 
network::

    <Adapter client="client_label" net="network" />

The ``network`` value must be consistent with the other networks in the BACnet intranet.

MAdapter
~~~~~~~~

The :ref:`MAdapter` is for :ref:`MLAN` networks, refer to the MLAN section for more information.

Samples
-------

For these and other samples, the components are described in configuration starting from 
the bottom of a stack and proceeding up.

Router Application
~~~~~~~~~~~~~~~~~~

This sample configuration file demonstrates the simple network layer processing without 
the component doing any routing::

    <BFR>
        Create a network with three nodes:

        <VLAN>
            <Node address="1" server="a" promiscuous="y" />
            <Node address="2" server="b" />
            <Node address="3" server="c" />
        </VLAN>

        On the first node, bind a debugger:

        <Debug client="a" prefix="a" />

        On the second node, bind a router and a console:

        <Router client="b" server="ba" />
        <Console client="ba" />

        <Debug client="c" server="cd" prefix="c" />
        <Echo client="cd" />
    </BFR>

.. highlight:: text

Here is an example::

    $ bfr router_app.bfr
    * DEADBEEF
    c 2 -> * : 01.00.DE.AD.BE.EF.
    c 2 <- null : 01.00.DE.AD.BE.EF.
    3 -> 2 : DE.AD.BE.EF.
    a 3 -> 2 : 01.00.DE.AD.BE.EF.
    a 2 -> * : 01.00.DE.AD.BE.EF.

The user has requested that ``DEADBEEF``, the hex encoding of an application layer message, 
is to be sent as a local broadcast.  The router element adds NPCI header ``01.00`` and 
sends the message downstream.  Each of the other nodes process the complete message, and 
node ``c`` echos the complete packet back.

When the echoed packet is delivered upstream to the console the NPCI is removed.

The following sample shows the user using the ``*:*`` application layer destination 
address for generating global broadcast messsage.  Notice that the NPCI contains the 
DNET/DLEN/DADR fields appropriate for a global broadcast::

    $ bfr router_app.bfr
    *:* DEADBEEF
    c 2 -> * : 01.20.FF.FF.00.FF.DE.AD.BE.EF.
    c 2 <- null : 01.20.FF.FF.00.FF.DE.AD.BE.EF.
    3 -> *:* : DE.AD.BE.EF.
    a 3 -> 2 : 01.20.FF.FF.00.FF.DE.AD.BE.EF.
    a 2 -> * : 01.20.FF.FF.00.FF.DE.AD.BE.EF.

The user has requested that ``DEADBEEF``, the hex encoding of an application layer message, 
is to be sent as a global broadcast.  The processing is identical to the other sample.  Notice 
that when the echoed packet is delivered upstream to the console, the source is address ``3`` 
and the destination is a global broadcast.

Router Echo
~~~~~~~~~~~

.. highlight:: xml

This sample configuration file creates a virtual network with three nodes::

    <BFR>
        Create a network with three nodes:

        <VLAN>
            <Node address="1" server="a" promiscuous="y" />
            <Node address="2" server="b" />
            <Node address="3" server="c" />
        </VLAN>

        On the first node, bind a debugger:

        <Debug client="a" prefix="a" />

        On the second node, bind a console:

        <Debug client="b" server="bd" prefix="b" />
        <Router client="bd" server="ba" />
        <Console client="ba" />

        On the third node, bind a debugger, which will show the
        traffic going to the router from this network:

        <Debug client="c" server="cr" prefix="c" />

        Create a second network with three nodes:

        <VLAN>
            <Node address="1" server="x" promiscuous="y" />
            <Node address="2" server="y" />
            <Node address="3" server="z" />
        </VLAN>

        On the first node, bind a debugger:

        <Debug client="x" prefix="x" />

        On the second node, bind an echo:

        <Debug client="y" server="yd" prefix="y" />
        <Router client="yd" server="ya" />
        <Echo client="ya" />

        On the third node, bind a debugger, which will show the
        traffic going to the router from this network:

        <Debug client="z" server="zr" prefix="z" />

        Now create a router between the two networks:

        <Router>
            <Adapter client="cr" net="1" />
            <Adapter client="zr" net="2" />
        </Router>
    </BFR>

.. highlight:: text

When the application first starts, the router has a special task to broadcast 
an I-Am-Router-To-Network message on all of its connected networks.  There are two 
sets of messages, the first is on network ``1`` saying that ``c`` is the router 
to network ``2``, the second is on network ``2`` saying that ``z`` is the router 
to network ``1``::

    $ bfr router_echo.bfr 
    c * <- null : 01.80.01.00.02.
    b 3 -> * : 01.80.01.00.02.
    a 3 -> * : 01.80.01.00.02.
    z * <- null : 01.80.01.00.01.
    y 3 -> * : 01.80.01.00.01.
    x 3 -> * : 01.80.01.00.01.

Now the user is requesting a local broadcast message.  Both nodes ``c`` and ``a`` 
receive a copy of the message::

    * 01
    b * <- null : 01.00.01.
    c 2 -> * : 01.00.01.
    a 2 -> * : 01.00.01.

The console is on ``b`` on network ``1`` that is sending a message to ``x`` on 
network ``2``::

    2:1 02
    b * <- null : 01.20.00.02.01.01.FF.02.
    c 2 -> * : 01.20.00.02.01.01.FF.02.
    z 1 <- null : 01.08.00.01.01.02.02.
    x 3 -> 1 : 01.08.00.01.01.02.02.
    a 2 -> * : 01.20.00.02.01.01.FF.02.

The user is now requesting a packet be sent to node ``y`` which will be echoed 
back::

    2:2 03
    b * <- null : 01.20.00.02.01.02.FF.03.
    c 2 -> * : 01.20.00.02.01.02.FF.03.
    z 2 <- null : 01.08.00.01.01.02.03.
    y 3 -> 2 : 01.08.00.01.01.02.03.
    y * <- null : 01.20.00.01.01.02.FF.03.
    z 2 -> * : 01.20.00.01.01.02.FF.03.
    c 2 <- null : 01.08.00.02.01.02.03.
    b 3 -> 2 : 01.08.00.02.01.02.03.
    2:2 -> 2 : 03.
    a 3 -> 2 : 01.08.00.02.01.02.03.
    x 2 -> * : 01.20.00.01.01.02.FF.03.
    x 3 -> 2 : 01.08.00.01.01.02.03.
    a 2 -> * : 01.20.00.02.01.02.FF.03.

.. note:: Note that the network layer is substituting a local broadcast for the appropriate 
    destination address, it should have been ``3`` because the router component sitting just 
    below the console should already know the router to network ``2``.  This is a bug.

