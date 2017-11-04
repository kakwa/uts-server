Changelogs
==========

0.1.10
-----

* [fix ] point to upstream civetweb (forked civetweb now removed)
* [fix ] using dynamic openssl loading for civetweb when bundling 
* [impr] add possibility to specify which tag used for civetweb bundling
* [impr] use same compiler for uts-server and civetweb when bundling

0.1.9
-----

* [fix ] add explicit C standard (C99), fixes compilation with olders gcc/cmake

0.1.8
-----

* [impr] add OpenBSD support
* [impr] add LibreSSL support

0.1.7
-----

* [doc ] add warnings to explicitely state the BUNDLE_CIVETWEB option as test/dev only
* [impr] add option for easily linking lib pthread (mainly for static linking)

0.1.6
-----

* [fix ] option declaration for LINK_GCC_S

0.1.5
-----

* [impr] add support for a static build

0.1.4
-----

* [impr] more portable code

0.1.3
-----

* [impr] add support for FreeBSD

0.1.2
-----

* [fix ] adding support for OpenSSL 1.1 (with compatibility with 1.0)

0.1.1
-----

* [fix ] correct compilation issues in older gcc/clang caused by missing -D_XOPEN_SOURCE, missing -std and missing headers
* [impr] exit at the first TS_RESP_CTX (OpenSSL TS response context) initialization failed.

0.1.0
-----

* [impr] adding various goodies (init scripts)
* [impr] safer crypto algorithm in configuration file
* [impr] removing useless default_tsa parameter in configuration file

0.0.3
-----

* [fix ] memleak on configuration parameters loading

0.0.2
-----

* [fix ] Fix loading of certificate in case of relative path

0.0.1
-----

* First version


