Changelogs
==========

dev
---

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


