.. BFR Testing

Testing
=======

Apart from testing the functionality of the BFR application and underlying BACnet 
library, BFR testing involves sending a stream of BACnet packets at the application 
and verifying that the packets are being filtered and routed correctly.  There are 
a variety of testing tools such as 
`Visual Test Shell <http://sourceforge.net/projects/vts/>`_ which make it simple 
to interactively construct and send BACnet packets, and the tool displays the encoded 
packet while it is being built.

The BFR has a "console" feature that prints packets that have been received to stdout 
in hex, and accepts packets on stdin to transmit.  While the BFR is running on one 
workstation, another instance can be used to "stream" packets to it.  Both instances 
of the application can be running on the same workstation, provided they both don't 
attempt to open the same socket.

A testing framework, one that can both send sample packets and inspect the behavior 
of the BFR, with a variety of configuration files, would be a welcome addition to the 
project.


