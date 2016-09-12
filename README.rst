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

Quick Start
-----------

.. sourcecode:: bash

    # Building with civetweb embedded (will recover civetweb from github).
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
