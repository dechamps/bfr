.. BACpypes glossary

Glossary
========

.. glossary::

    bound
        The tight coupling of a client with a service.  All clients must 
        be bound to some server and vice versa.

    client
        A component that consumes packets from a server to provide some 
        functionality.

    server
        A component that provides a service to a client.

    upstream
        Something going up a stack from a server to client.

    downstream
        Something going down a stack from a client to a server.

    stack
        A sequence of communication objects organized in a semi-linear sequence
        from the application layer at the top to the physical networking layer(s)
        at the bottom.

    discoverable
        Something that can be determined using a combination of BACnet objects,
        properties and services.  For example, discovering the network topology
        by using Who-Is-Router-To-Network, or knowing what objects are defined
        in a device by reading the *object-list* property.
