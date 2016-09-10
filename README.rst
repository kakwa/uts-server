uts-server
==========

.. image:: https://travis-ci.org/kakwa/uts-server.svg?branch=master
    :target: https://travis-ci.org/kakwa/uts-server

.. image:: https://readthedocs.org/projects/uts-server/badge/?version=latest
    :target: http://uts-server.readthedocs.org/en/latest/?badge=latest
    :alt: Documentation Status

Micro timestamp server (RFC 3161) written in C

----

:Doc:    `Uts-Server documentation on ReadTheDoc <http://uts-server.readthedocs.org/en/latest/>`_
:Dev:    `Uts-Server source code on GitHub <https://github.com/kakwa/uts-server>`_
:License: MIT
:Author:  Pierre-Francois Carpentier - copyright © 2016

----

Status
------

Alpha

License
-------

Released under the MIT Public License

Quick Start
-----------

.. sourcecode:: bash

    # building with civetweb embedded (will recover civetweb from github)
    $ cmake . -DBUNDLE_CIVETWEB=ON
    $ make
    
    # create some test certificates
    $ ./tests/cfg/pki/create_tsa_certs
    
    # launching the timestamp server with test configuration in debug mode
    $ ./uts-server -c tests/cfg/uts-server.cnf -D
    
    # in another shell, launching a timestamp script on the README.md file
    $ ./goodies/timestamp-file.sh -i README.md -u http://localhost:2020 -r -O "-cert";
