# uts-server

Micro timestamp server (RFC 3161) written in C

## Status

Alpha

## Dependencies

* OpenSSL (https://github.com/openssl/openssl)
* civetweb (https://github.com/civetweb/civetweb)

## License

Released under the MIT Public License

## Playing with it


```bash
# create some test certificates
$ ./tests/cfg/pki/create_tsa_certs

# launching the timestamp server with test configuration (debug mode)
$ ./uts-server -c tests/cfg/uts-server.cnf -D

# in another shell, launching a timestamp client on the README.md file
$ ./goodies/timestamp-file.sh -i README.md -u http://localhost:2020 -r -O "-cert";
```

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

## Building

```bash
$ cmake .
$ make -j 2
```
