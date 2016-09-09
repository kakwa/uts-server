# uts-server

[![Build Status](https://travis-ci.org/kakwa/uts-server.svg?branch=master)](https://travis-ci.org/kakwa/uts-server)

Micro timestamp server (RFC 3161) written in C

## Status

Alpha

## Dependencies

* OpenSSL (https://github.com/openssl/openssl)
* civetweb (https://github.com/civetweb/civetweb)

## License

Released under the MIT Public License

## Usage

```bash
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
```

## Configuration

### tsa

Section defining which TSA section to use.

* ```default_tsa```: Name of the TSA section to use as default.

### main

Main configuration section (mostly http configuration).

* ```enable_keep_alive```: Allows clients to reuse TCP connection for subsequent
  HTTP requests, which improves performance.
* ```num_threads```: Number of worker threads.
* ```ssl_ca_path```: Name of a directory containing trusted CA certificates.
* ```throttle```: Limit download speed for clients. throttle is a comma-separated list of key=value pairs:
  - *            -> limit speed for all connections
  - x.x.x.x/mask ->  limit speed for specified subnet
  The value is a floating-point number of bytes per second, optionally followed by a k or m character
  meaning kilobytes and megabytes respectively. A limit of 0 means unlimited rate.
  Ex: throttle = *=1k,10.10.0.0/16=10m,10.20.0.0/16=0
* ```ssl_verify_peer```: Enable client's certificate verification by the server.
* ```ssl_certificate```: Path to the SSL certificate file (PEM format containing private key and certificate).
* ```tcp_nodelay```: Enable TCP_NODELAY socket option on client connections.
* ```ssl_verify_depth```: Sets maximum depth of certificate chain.
  If client's certificate chain is longer than the depth set here connection is refused.
* ```ssl_short_trust```: Enables the use of short lived certificates
* ```request_timeout_ms```: Timeout for network read and network write operations, in milliseconds.
* ```ssl_protocol_version```: Sets the minimal accepted version of SSL/TLS protocol according to the table:
  SSL2+SSL3+TLS1.0+TLS1.1+TLS1.2  0
  SSL3+TLS1.0+TLS1.1+TLS1.2       1
  TLS1.0+TLS1.1+TLS1.2            2
  TLS1.1+TLS1.2                   3
  TLS1.2                          4
* ```ssl_ca_file```: Path to a .pem file containing trusted certificates. The file may contain more than one certificate.
* ```ssl_default_verify_paths```: Loads default trusted certificates locations set at openssl compile time.
* ```access_control_allow_origin```: Comma separated list of IP subnets to accept/deny
  Ex: -0.0.0.0/0,+192.168.0.0/16 (deny all accesses, only allow 192.168.0.0/16 subnet)
* ```log_level```: Loglevel (debug, info, notice, warn, err, emerg, crit)
* ```ssl_cipher_list```: See https://www.openssl.org/docs/manmaster/apps/ciphers.html for more detailed
* ```listening_ports```: Comma-separated list of ips:ports to listen on.
  If the port is SSL, a letter s must be appended.
  Ex: listening_ports = 80,443s
* ```run_as_user```: Switch to given user credentials after startup.
  Required to run on privileged ports and not be run as root.

### tsa_config1

Example of timestamp section configuration.

* ```clock_precision_digits```: Number of decimals for timestamp. (optional)
* ```tsa_name```: Must the TSA name be included in the reply? (optional, default: no)
* ```signer_key```: The TSA private key. (optional)
* ```signer_cert```: The TSA signing certificat. (optional)
* ```ordering```: Is ordering defined for timestamps? (optional, default: no)
* ```certs```: Certificate chain to include in reply. (optional)
* ```default_policy```: Policy if request did not specify it. (optional)
* ```other_policies```: Acceptable policies. (optional)
* ```crypto_device```: OpenSSL engine to use for signing.
* ```ess_cert_id_chain```: Must the ESS cert id chain be included? (optional, default: no)
* ```digests```: Acceptable message digests. (mandatory)
* ```dir```: TSA root directory.
* ```accuracy```: Timestamp accuracy. (optional)

### oids

Section for declaring OID mapping. Just add <name> = <OID> pairs.


## Building

```bash
$ cmake .
$ make -j 2
```

## Playing with it

```bash
# building with civetweb embedded (will recover civetweb from github)
$ cmake . -DBUNDLE_CIVETWEB=ON
$ make

# create some test certificates
$ ./tests/cfg/pki/create_tsa_certs

# launching the timestamp server with test configuration in debug mode
$ ./uts-server -c tests/cfg/uts-server.cnf -D

# in another shell, launching a timestamp script on the README.md file
$ ./goodies/timestamp-file.sh -i README.md -u http://localhost:2020 -r -O "-cert";
```
