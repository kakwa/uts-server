Deploy
======

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

Running uts-sever
-----------------

To debug problems with uts-server, run it in the foreground in debug mode:

.. sourcecode:: bash

    # In debug mode with verbose debugging on stdout
    $ ./uts-server -c <path/to/conf> -D

To run it as a daemon:

.. sourcecode:: bash

    # In daemon mode
    $ ./uts-server -c <path/to/conf> -d -p <path/to/pidfile>

