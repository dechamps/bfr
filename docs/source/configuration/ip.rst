.. BFR B/IP Element

.. highlight:: xml

BACnet/IP
=========

BACnet/IP elements work on top of the UDP/IP layer. They process the BACnet Virtual 
Link Layer header according to the functionality of the component and provide an additional 
layer of abstraction to upper layers in the stack.

BIP
---

The ``BIP`` component interprets the BVLL as a simple device connection to a B/IP LAN::

    <BIP client="client_label" server="server_label" />

The B/IP packets it receives upstream from the UDP layer, such as Original-Unicast and Original-Broadcast
will continue up the stack with the same source and destination address.   A Forwarded-NPDU which 
has been sent by a BBMD on the LAN will have the original source address substituted from the 
BVLL header.

Foreign
-------

The ``Foreign`` component interprets the BVLL as a foreign device of a B/IP network::

    <Foreign client="client_label" server="server_label" 
        bbmd="bbmd_address" ttl="time_to_live"
        />

In addition to the ``client`` and ``server`` attributes, the ``Foreign`` component requires 
the ``bbmd`` and ``ttl`` attribtues. The ``bbmd`` is the IP address of the BBMD that manages the 
foreign device registration, and the ``ttl`` is the time to live in seconds.

BBMD
----

The BACnet/IP Broadcast Management Device (BBMD) component manages the redistribution of local 
broadcast traffic and maintains registrations for foreign devices::

    <BBMD client="client_label" server="server_label" foreign="flag">
        <Peer address="peer_address" />
    </BBMD>

The ``foreign`` flag is "y" or "Y" if the BBMD supports foreign device registration.

The ``Peer`` child element of the ``BBMD`` is a reference to a peer BBMD.  The ``peer_address`` is 
of the form *X.X.X.X/Y:Z*, where *X.X.X.X* is the IP address in the dot-quad notation, *Y* is 
the number of bits in the network portion, and *Z* is the port number.

The */Y* portion of the address is typically */32* indicating that the broadcast messages to 
be forwarded are sent to the peer BBMD to be rebroadcast, rather than using a directed broadcast 
which is typically not allowed.

If the *:Z* portion of the address is omitted, the port will default to 47808 (0xBAC0).

.. note:: There is no configuration validation, so it is possible to create a BBMD component that 
    has no Broadcast Distribution Table (BDT), or to create one that doesn't have itself as 
    an entry.

Samples
-------

For these and other samples, the components are described in configuration starting from 
the bottom of a stack and proceeding up.  In the coorisponding diagrams, upstream traffic 
goes from left to right, the downstream traffic from right to left.

BIP to Console
~~~~~~~~~~~~~~

This sample configuration file creates one of each of the ``UDP``, ``BIP``, and ``Console``
components and binds them together::

    <BFR>
        <UDP address="192.168.1.1/24" server="a" />
        <BIP client="a" server="b"/>
        <Console client="b" />
    </BFR>

This is a long line of text.

Foreign to Console
~~~~~~~~~~~~~~~~~~

This sample configuration file creates one of each of the ``UDP``, ``Foreign``, and ``Console``
components and binds them together::

    <BFR>
        <UDP address="192.168.1.1/24" server="a" />
        <Foreign client="a" server="b" bbmd="192.168.2.1" ttl="60" />
        <Console client="b" />
    </BFR>

This is a long line of text.

BBMD to Console
~~~~~~~~~~~~~~~

This sample configuration file creates one of each of the ``UDP``, ``BBMD``, and ``Console``
components and binds them together.  The BBMD is configured to have two peers::

    <BFR>
        <UDP address="192.168.1.1/24" server="a" />
        <BBMD client="a" server="b" foreign="n">
            <Peer address="192.168.1.1" />
            <Peer address="192.168.2.1" />
            <Peer address="192.168.3.1" />
        </BBMD>
        <Console client="b" />
    </BFR>

This is a long line of text.


