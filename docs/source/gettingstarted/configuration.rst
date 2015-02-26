.. BFR Configuration

.. highlight:: xml

Configuration
=============

BFR applications are configured with a very simple XML document.  All of the configuration 
parameters are combinations of XML child elements and their attributes, the markup text and 
comments in the file is ignored, making it a great place for documentation::

    <BFR>
        Configuration elements go here, along with documentation.
    </BFR>

For embedded systems, the XML configuration file is usually stored with the application in the 
same directory, or on Linux systems the executable is stored in ``/usr/local/bin`` and the 
configuration in ``/etc``, for example::

    $ /usr/local/bin/BFR /etc/sample.bfr

There is no restriction on the configuration file name or location, and there is no default 
file name or location.  The sample applications use the **bfr** suffix.

Begin with the :ref:`elements-and-attributes` in the configuration documentation section for 
more.

