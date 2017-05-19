uts-server 
==========

.. image:: https://github.com/kakwa/uts-server/blob/master/docs/assets/logo_64.png?raw=true

|

.. image:: https://travis-ci.org/kakwa/uts-server.svg?branch=master
    :target: https://travis-ci.org/kakwa/uts-server

.. image:: https://readthedocs.org/projects/uts-server/badge/?version=latest
    :target: http://uts-server.readthedocs.org/en/latest/?badge=latest
    :alt: Documentation Status

Micro `RFC 3161 Time-Stamp <https://www.ietf.org/rfc/rfc3161.txt>`_ server written in C.

----

:Doc:    `Uts-Server documentation on ReadTheDoc <http://uts-server.readthedocs.org/en/latest/>`_
:Dev:    `Uts-Server source code on GitHub <https://github.com/kakwa/uts-server>`_
:License: MIT
:Author:  Pierre-Francois Carpentier - copyright © 2016

----

License
-------

Released under the MIT Public License

What is RFC 3161?
-----------------

An RFC 3161 time-stamp is basically a cryptographic signature with a date attached.

Roughly, it works as follow:

1. A client application sends an hash of the data it wants to time-stamp to a Time-Stamp authority server.
2. The Time-Stamp authority server retrieves the current date, concatenates it with the hash and uses its private key to create the time-stamp (kind of like a signature).
3. The Time-Stamp authority server returns the generated time-stamp to the client application.

Then a client can verify the piece of data with the time-stamp using the Certificate Authority of the time-stamp key pair (X509 certificates).

It gives a cryptographic proof of a piece of data content, like a file, at a given time.

Some use cases:

* time-stamp log files at rotation time.
* time-stamp file at upload to prove it was delivered in due time or not.

Quick (and dirty) Testing
-------------------------

Here a few steps to quickly trying out uts-server, for production setup, please compile civetweb externally and create proper CA and certificates:

.. sourcecode:: bash

    # Building with civetweb embedded (will recover civetweb from github).
    # Note: the BUNDLE_CIVETWEB option is only here for fast testing purpose
    # The recommended way to deploy uts-server in production is to build civetweb
    # separatly and to link against it.
    $ cmake . -DBUNDLE_CIVETWEB=ON
    $ make
    
    # Create some test certificates.
    $ ./tests/cfg/pki/create_tsa_certs
    
    # Launching the time-stamp server with test configuration in debug mode.
    $ ./uts-server -c tests/cfg/uts-server.cnf -D
    
    # In another shell, launching a time-stamp script on the README.md file.
    $ ./goodies/timestamp-file.sh -i README.rst -u http://localhost:2020 -r -O "-cert";

    # Verify the time-stamp.
    $ openssl ts -verify -in README.rst.tsr -data README.rst -CAfile ./tests/cfg/pki/tsaca.pem

    # Display the time-stamp content.
    $ openssl ts -reply -in README.rst.tsr -text

Powered by
----------
    
.. image:: https://raw.githubusercontent.com/openssl/web/master/img/openssl-64.png
    :target: https://www.openssl.org/

.. image:: https://github.com/civetweb/civetweb/blob/658c8d48b3bcdb34338dae1b83167a8d7836e356/resources/civetweb_32x32@2.png?raw=true
    :target: https://github.com/civetweb/civetweb

