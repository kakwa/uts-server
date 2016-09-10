uts-server
==========

.. image:: https://travis-ci.org/kakwa/uts-server.svg?branch=master
    :target: https://travis-ci.org/kakwa/uts-server

Micro timestamp server (RFC 3161) written in C

Status
------

Alpha

Dependencies
------------

Runtime dependencies
~~~~~~~~~~~~~~~~~~~~

* OpenSSL (https://github.com/openssl/openssl)
* civetweb (https://github.com/civetweb/civetweb)

Build dependencies
~~~~~~~~~~~~~~~~~~

* cmake
* either gcc or clang

License
-------

Released under the MIT Public License

Usage
-----

.. sourcecode:: bash

    $ ./uts-server --help
    Usage: uts-server [OPTION...] -c CONFFILE [-d] [-D] [-p <pidfile>]
    
    UTS micro timestamp server (RFC 3161)
    
      -c, --conffile=CONFFILE    Path to configuration file
      -d, --daemonize            Launch as a daemon
      -D, --debug                STDOUT debugging
      -p, --pidfile=PIDFILE      Path to pid file
      -?, --help                 Give this help list
          --usage                Give a short usage message
      -V, --version              Print program version
    
    Mandatory or optional arguments to long options are also mandatory or optional
    for any corresponding short options.
    
    Report bugs to Pierre-Francois Carpentier <carpentier.pf@gmail.com>.

Configuration
-------------

main
~~~~

Main configuration section (mostly http configuration).

+-----------------------------+---------------------------------------------------------------------+--------------------------------------+
| param                       | description                                                         | example value                        |
+=============================+=====================================================================+======================================+
| access_control_allow_origin | Comma separated list of IP subnets to accept/deny                   | -0.0.0.0/0,+192.168/16               |
|                             |                                                                     |                                      |
|                             | Ex: -0.0.0.0/0,+192.168.0.0/16                                      |                                      |
|                             | (deny all accesses, only allow 192.168.0.0/16 subnet)               |                                      |
+-----------------------------+---------------------------------------------------------------------+--------------------------------------+
| enable_keep_alive           | Allows clients to reuse TCP connection for subsequent               | no                                   |
|                             | HTTP requests, which improves performance.                          |                                      |
+-----------------------------+---------------------------------------------------------------------+--------------------------------------+
| listening_ports             | Comma-separated list of ips:ports to listen on.                     | 127.0.0.1:2020                       |
|                             | If the port is SSL, a letter s must be appended.                    |                                      |
|                             | Ex: listening_ports = 80,443s                                       |                                      |
+-----------------------------+---------------------------------------------------------------------+--------------------------------------+
| log_level                   | Loglevel (debug, info, notice, warn, err, emerg, crit)              | info                                 |
+-----------------------------+---------------------------------------------------------------------+--------------------------------------+
| num_threads                 | Number of worker threads.                                           | 50                                   |
+-----------------------------+---------------------------------------------------------------------+--------------------------------------+
| request_timeout_ms          | Timeout for network read and network write operations.              | 30000                                |
|                             | In milliseconds.                                                    |                                      |
+-----------------------------+---------------------------------------------------------------------+--------------------------------------+
| run_as_user                 | Switch to given user credentials after startup.                     | uts-server                           |
|                             | Required to run on privileged ports as non root user.               |                                      |
+-----------------------------+---------------------------------------------------------------------+--------------------------------------+
| ssl_ca_file                 | Path to a .pem file containing trusted certificates.                | /etc/uts-server/ca.pem               |
|                             | The file may contain more than one certificate.                     |                                      |
+-----------------------------+---------------------------------------------------------------------+--------------------------------------+
| ssl_ca_path                 | Name of a directory containing trusted CA certificates.             | /etc/ssl/ca/                         |
+-----------------------------+---------------------------------------------------------------------+--------------------------------------+
| ssl_certificate             | Path to the SSL certificate file .                                  | /etc/uts-server/cert.pem             |
|                             | PEM format must contain private key and certificate.                |                                      |
+-----------------------------+---------------------------------------------------------------------+--------------------------------------+
| ssl_cipher_list             | See https://www.openssl.org/docs/manmaster/apps/ciphers.html        | ALL:!eNULL                           |
|                             | for more detailed                                                   |                                      |
+-----------------------------+---------------------------------------------------------------------+--------------------------------------+
| ssl_default_verify_paths    | Loads default trusted certificates                                  | yes                                  |
|                             | locations set at openssl compile time.                              |                                      |
+-----------------------------+---------------------------------------------------------------------+--------------------------------------+
| ssl_protocol_version        | Sets the minimal accepted version of SSL/TLS protocol               | 3                                    |
|                             | according to the table:                                             |                                      |
|                             |                                                                     |                                      |
|                             | SSL2+SSL3+TLS1.0+TLS1.1+TLS1.2 -> 0                                 |                                      |
|                             | SSL3+TLS1.0+TLS1.1+TLS1.2      -> 1                                 |                                      |
|                             | TLS1.0+TLS1.1+TLS1.2           -> 2                                 |                                      |
|                             | TLS1.1+TLS1.2                  -> 3                                 |                                      |
|                             | TLS1.2                         -> 4                                 |                                      |
+-----------------------------+---------------------------------------------------------------------+--------------------------------------+
| ssl_short_trust             | Enables the use of short lived certificates                         | no                                   |
+-----------------------------+---------------------------------------------------------------------+--------------------------------------+
| ssl_verify_depth            | Sets maximum depth of certificate chain.                            | 9                                    |
|                             | If client's certificate chain is longer                             |                                      |
|                             | than the depth set here connection is refused.                      |                                      |
+-----------------------------+---------------------------------------------------------------------+--------------------------------------+
| ssl_verify_peer             | Enable client's certificate verification by the server.             | yes                                  |
+-----------------------------+---------------------------------------------------------------------+--------------------------------------+
| tcp_nodelay                 | Enable TCP_NODELAY socket option on client connections.             | 0                                    |
+-----------------------------+---------------------------------------------------------------------+--------------------------------------+
| throttle                    | Limit download speed for clients.                                   | \*=0                                 |
|                             |                                                                     |                                      |
|                             | throttle is a comma-separated list of key=value pairs:              |                                      |
|                             | - \*            -> limit speed for all connections                  |                                      |
|                             | - x.x.x.x/mask ->  limit speed for specified subnet                 |                                      |
|                             |                                                                     |                                      |
|                             | The value is a floating-point number of bytes per second,           |                                      |
|                             | optionally followed by a k or m character                           |                                      |
|                             | meaning kilobytes and megabytes respectively.                       |                                      |
|                             | A limit of 0 means unlimited rate.                                  |                                      |
|                             | Ex: throttle = \*=1k,10.10.0.0/16=10m,10.20.0.0/16=0                |                                      |
+-----------------------------+---------------------------------------------------------------------+--------------------------------------+

oids
~~~~

Section for declarinG OID mapping. Just add <name> = <OID> pairs.

+-----------------------------+---------------------------------------------------------------------+--------------------------------------+
| param                       | description                                                         | example value                        |
+=============================+=====================================================================+======================================+
| tsa_policy1                 |                                                                     | 1.2.3.4.1                            |
+-----------------------------+---------------------------------------------------------------------+--------------------------------------+
| tsa_policy2                 |                                                                     | 1.2.3.4.5.6                          |
+-----------------------------+---------------------------------------------------------------------+--------------------------------------+
| tsa_policy3                 |                                                                     | 1.2.3.4.5.7                          |
+-----------------------------+---------------------------------------------------------------------+--------------------------------------+

tsa
~~~

Section defining which TSA section to use.

+-----------------------------+---------------------------------------------------------------------+--------------------------------------+
| param                       | description                                                         | example value                        |
+=============================+=====================================================================+======================================+
| default_tsa                 | Name of the TSA section to use as default.                          | tsa_config1                          |
+-----------------------------+---------------------------------------------------------------------+--------------------------------------+

tsa_config1
~~~~~~~~~~~

Example of timestamp section configuration.

+-----------------------------+---------------------------------------------------------------------+--------------------------------------+
| param                       | description                                                         | example value                        |
+=============================+=====================================================================+======================================+
| accuracy                    | Timestamp accuracy. (optional)                                      | secs:1, millisecs:500, microsecs:100 |
+-----------------------------+---------------------------------------------------------------------+--------------------------------------+
| certs                       | Certificate chain to include in reply. (optional)                   | $dir/cacert.pem                      |
+-----------------------------+---------------------------------------------------------------------+--------------------------------------+
| clock_precision_digits      | Number of decimals for timestamp. (optional)                        | 0                                    |
+-----------------------------+---------------------------------------------------------------------+--------------------------------------+
| crypto_device               | OpenSSL engine to use for signing.                                  | builtin                              |
+-----------------------------+---------------------------------------------------------------------+--------------------------------------+
| default_policy              | Policy if request did not specify it. (optional)                    | tsa_policy1                          |
+-----------------------------+---------------------------------------------------------------------+--------------------------------------+
| digests                     | Acceptable message digests. (mandatory)                             | md5, sha1                            |
+-----------------------------+---------------------------------------------------------------------+--------------------------------------+
| dir                         | TSA root directory.                                                 | /etc/uts-server/pki                  |
+-----------------------------+---------------------------------------------------------------------+--------------------------------------+
| ess_cert_id_chain           | Must the ESS cert id chain be included? (optional, default: no)     | no                                   |
+-----------------------------+---------------------------------------------------------------------+--------------------------------------+
| ordering                    | Is ordering defined for timestamps? (optional, default: no)         | yes                                  |
+-----------------------------+---------------------------------------------------------------------+--------------------------------------+
| other_policies              | Acceptable policies. (optional)                                     | tsa_policy2, tsa_policy3             |
+-----------------------------+---------------------------------------------------------------------+--------------------------------------+
| signer_cert                 | The TSA signing certificat. (optional)                              | $dir/tsacert.pem                     |
+-----------------------------+---------------------------------------------------------------------+--------------------------------------+
| signer_key                  | The TSA private key. (optional)                                     | $dir/private/tsakey.pem              |
+-----------------------------+---------------------------------------------------------------------+--------------------------------------+
| tsa_name                    | Must the TSA name be included in the reply? (optional, default: no) | yes                                  |
+-----------------------------+---------------------------------------------------------------------+--------------------------------------+


Building
--------

.. sourcecode:: bash

    $ cmake .
    $ make -j 2

Playing with it
---------------

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
