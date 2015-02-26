.. BFR MLAN

.. highlight:: xml

MLAN
====

*Masquerade Local Area Network* components provide a service that is a combination of 
a :ref:`VLAN` and the Network Address Translation service.  The basic feature is to 
"mask" the topology of one network from another connected network.

.. warning::This feature and related components are not in the BACnet standard.  This 
    should be considered experimantal.

MLANs can be configured to create a network with any arbitrary MAC address size.

.. _MLAN:

MLAN
----

The MLAN component creates a masquerade network with a collection of nodes::

    <MLAN mlan="mlan_label" />

The ``mlan_label`` uniquely identifies the ``MLAN`` in the configuration.

The relationship between a :ref:`Router` and an MLAN is made via a masquerade adapter.

.. _MAdapter:

MAdapter
~~~~~~~~

The *Masquerade Adapter* is a component that manages the relationship between an MLAN 
and how the nodes on that network are to be mapped::

    <MAdapter net="net_number" mlan="mlan_label">

        Static address mapping...

        <Static node="node_address" address="net_address" />

        Address filtering...

        <Accept address="address_pattern" />
        <Reject />
    </MAdapter>

The ``net_number`` is the network number of the connected BACnet network to the router. 
This is the unique number in the context of the BACnet intranet of the ``Router`` and 
its other attached networks.  This is indepedant of the network number of the other 
``MAdapter`` components that may be connected to the same ``MLAN``.

The ``mlan_label`` is the label of the "attached" ``MLAN`` component.

The ``Static`` elements are used to define specific mappings from the ``MLAN`` to network 
addresses.  The ``net_address`` is the BACnet intranet address of the original source 
address of a packet coming downstream from the ``Router``, and the ``node_addess`` is 
how that should be presented as a local station address on the ``MLAN``.

Addresses that are not staticly associated with specific nodes are given dynamic ``node_address`` 
values, subject to the ``Accept`` or ``Reject`` rules.

The ``Accept`` element describes an address pattern that is acceptable to map on the 
``MLAN``.  The form is typically ``n:*`` which will accept all of the addresses on a 
specific network.  If the ``address_pattern`` is not provided, the ``Accept`` functions 
like "accept all (others)".

The ``Reject`` element is the inverse of the ``Accept`` element, it describes an address 
pattern that is *not* acceptable to map on the ``MLAN``.  The form is typically ``n:*`` which 
will reject all of the addresses on a specific network.  If the ``address_pattern`` is not 
provided, the ``Reject`` functions like "reject all (others)".

The ``Accept`` and ``Reject`` elements are processed in the order they are given.

Samples
-------

For these and other samples, the components are described in configuration starting from 
the bottom of a stack and proceeding up.  In the coorisponding diagrams, upstream traffic 
goes from left to right, the downstream traffic from right to left.

MLAN Echo
~~~~~~~~~

This sample configuration file creates two virtual networks and two routers.  The two 
routers are connected to each other via an ``MLAN``::

    <BFR>
        Create a network with two nodes, this will be network 1:

        <VLAN>
            <Node address="1" server="a" />
            <Node address="2" server="b" />
        </VLAN>

        On the first node, bind a console, and on the second bind a debugger:

        <Console client="a" />
        <Debug client="b" prefix=" b:"/>

        Create another network with two nodes, this will be network 2:

        <VLAN>
            <Node address="1" server="c" />
            <Node address="2" server="d" />
        </VLAN>

        On the first node, bind a debugger, and on the second bind a echo:

        <Debug client="c" prefix=" c:" />
        <Echo client="d" />

        Create a masquerade LAN:

        <MLAN mlan="" />

        MORE GOES HERE

    </BFR>

Some text goes here.

