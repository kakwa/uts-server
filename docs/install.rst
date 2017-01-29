Dependencies
============

Runtime dependencies
--------------------

List of dependencies uts-server relies on to run:

* `OpenSSL <https://github.com/openssl/openssl>`_.
* `civetweb <https://github.com/civetweb/civetweb>`_.

Build dependencies
------------------

List of dependencies needed to build civetweb:

* cmake
* either gcc or clang

Compilation
===========

uts-server is compiled using cmake:

.. sourcecode:: bash

    # If civetweb is already present on the system
    $ cmake .
    $ make

    # If civetweb is not present
    # this will get the proper tag of civetweb from upstream and compile it
    $ cmake . -DBUNDLE_CIVETWEB=ON
    $ make

    # Compile with debug flags
    $ cmake . -DDEBUG=ON
    $ make

    # Compile statically
    # (in some cases, it might be necessary to still
    # link some libraries like dl or gcc_s, if necessary,
    # add -DDL_LINK=ON and/or -DGCC_S_LINK=ON)
    $ cmake . -DSTATIC=ON # -DDL_LINK=ON -DGCC_S_LINK=ON
    $ make
