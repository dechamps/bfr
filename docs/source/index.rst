.. BFR documentation master file

Welcome to the BACnet Firewall Router
=====================================

If you would like to participate in its development, please join the
`developers mailing list <https://lists.sourceforge.net/lists/listinfo/bfr-developers>`_
and use the `Trac <https://sourceforge.net/apps/trac/bfr>`_ to create 
tickets and monitor the project development.  There is also a
`Google+ <https://plus.google.com/101188874349659771056/posts>`_ page that you
can add to your circles and have release notifications show up in your 
stream.

For an overview of the current project activity, there is a 
`Trello <https://trello.com/board/bacnet-firewall-router/4ea603cd499d85823a0c640a>`_ board,
feel free to become a member and share what you are doing with the
rest of the BFR community.

Welcome aboard!

Getting Started
---------------

This section is a walk through of the process of building and installing the application, 
downloading the sample code and communicating with a test device.

.. toctree::
    :maxdepth: 2

    gettingstarted/organization.rst
    gettingstarted/configuration.rst
    gettingstarted/testing.rst
    gettingstarted/deployment.rst

Configuration
-------------

This section is a walk through of the process of building and installing the application, 
downloading the sample code and communicating with a test device.

.. toctree::
    :maxdepth: 2

    configuration/elements.rst
    configuration/debug.rst
    configuration/network.rst
    configuration/ip.rst
    configuration/vlan.rst
    configuration/router.rst
    configuration/filter.rst
    configuration/mlan.rst

Developers
----------

This documentation is intended for BFR developers.

.. toctree::
    :maxdepth: 2

    developers/index.rst

Missing Features
----------------

* Missing link layer; ARCNET - Most enterprise networks are composed of a collection of Ethernet and/or IP LAN's. 
  so the focus of the project has been on those two technologies. There are relatively few ARCNET cards with Linux drivers,
  so ARCNET support has been a low priority

* Missing link layer; MS/TP - almost all PC's come with a serial port, but it is very difficult to manage the 
  timing issues with MS/TP communications with many operating systems.  The floppy-boot Linux environment, particularly 
  where the BFR is the only application running, should have no problems keeping track of the necessary timers.

* Missing link layer; PTP - Enterprise networks are typically static, with dial-up connections becoming less 
  popular as full-time, high speed connections become available.  PTP does not have the same timing requirements 
  that are involved in MS/TP networks and the PTP protocol is relatively simple, so adding support for PTP should 
  not be difficult.

* Half-routers - Similar to PTP support, there are standard procedures for processing I-Could-Be-Router-To-Network,
  Establish-Connection-To-Network, and other similar network layer messages that are appropriate for networks that are 
  not "always on", the BFR application does not currently follow these procedures.

Glossary
--------

.. toctree::
    :maxdepth: 2

    glossary.rst

Indices and tables
==================

* :ref:`genindex`
* :ref:`modindex`
* :ref:`search`

