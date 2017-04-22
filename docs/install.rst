Dependencies
============

Runtime dependencies
--------------------

List of dependencies uts-server relies on to run:

* `OpenSSL <https://github.com/openssl/openssl>`_.
* `civetweb <https://github.com/civetweb/civetweb>`_.
* on none GNU LibC, `argp-standalone <https://www.lysator.liu.se/~nisse/misc/argp-standalone-1.3.tar.gz>`_

Build dependencies
------------------

List of dependencies needed to build uts-server:

* `CMake <https://cmake.org/>`_
* either `gcc <https://gcc.gnu.org/>`_ or `clang <https://clang.llvm.org/>`_

Compilation
===========

uts-server is compiled using cmake:

.. sourcecode:: bash

    # If civetweb is already present on the system
    $ cmake .
    $ make

    # If civetweb is not present.
    # this will get the master branch of civetweb from upstream and compile it.
    # Only for developpment/testing purposes
    $ cmake . -DBUNDLE_CIVETWEB=ON
    $ make

    # Compile with debug flags
    # Only for developpment/testing purposes
    $ cmake . -DDEBUG=ON
    $ make

    # Compile statically
    # (in some cases, it might be necessary to still
    # link some libraries like dl or gcc_s or pthread, if necessary,
    # add -DLINK_DL=ON and/or -DLINK_GCC_S=ON and/or -DLINK_PTHREAD=ON)
    $ cmake . -DSTATIC=ON  # -DLINK_DL=ON -DLINK_GCC_S=ON -DLINK_PTHREAD=ON
    $ make

.. warning::

    The BUNDLE_CIVETWEB exists only for developpment/testing purposes.

    Please compile civetweb externally for building a production binary.

    Using this option outside of developpment/testing is a bad idea for the
    following reasons:

    * having an external download in a build process is a bad idea
    * recovering the master branch ensures that the build may break randomly
    * a build proccess should be reproductible which is not the case with this option

OS specific tips
================

Debian
------

The installation requires installing the following packages:

.. sourcecode:: bash

    # build dependencies
    $ apt-get install libssl-dev cmake clang

CentOS/RHEL
-----------

The installation requires installing the following packages:

.. sourcecode:: bash

    # build dependencies
    $ yum insall cmake gcc gcc-c++ openssl-devel

FreeBSD
-------

The installation requires installing the following packages:

.. sourcecode:: bash

    # build dependencies
    $ pkg add argp-standalone cmake

OpenBSD
-------

The installation requires installing the following packages:

.. sourcecode:: bash

    # build dependencies
    $ pkg_add gcc g++ argp-standalone cmake

    # for the test scripts
    $ pkg_add python curl

To build you must egcc and eg++ (not the old 4.2 gcc in the base system)

.. sourcecode:: bash

    # set compilers
    $ export CC=/usr/local/bin/egcc
    $ export CXX=/usr/local/bin/ec++

    # then build normally
    $ cmake . -DBUNDLE_CIVETWEB=ON && make
