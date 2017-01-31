Dependencies
============

Runtime dependencies
--------------------

List of dependencies uts-server relies on to run:

* `OpenSSL <https://github.com/openssl/openssl>`_.
* `civetweb <https://github.com/civetweb/civetweb>`_.

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
    # add -DDL_LINK=ON and/or -DGCC_S_LINK=ON -DPTHREAD_LINK=ON)
    $ cmake . -DSTATIC=ON # -DDL_LINK=ON -DGCC_S_LINK=ON -DPTHREAD_LINK=ON
    $ make

.. warning::

    The BUNDLE_CIVETWEB exists only for developpment/testing purposes.

    Please compile civetweb externally for building a production binary.

    Using this option outside of developpment/testing is a bad idea for the
    following reasons:

    * having an external download in a build process is a bad idea
    * recovering the master branch ensures that the build may break randomly
    * a build proccess should be reproductible which is not the case with this option
