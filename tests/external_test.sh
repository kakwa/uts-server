#!/bin/sh

PORT=`awk -v min=10000 -v max=20000 'BEGIN{srand(); print int(min+rand()*(max-min+1))}'`

if which timeout >/dev/null 2>&1
then
    TO="timeout 120"
else
    TO=""
fi

export TMPDIR='./tests/cfg/'
CFG=`mktemp`

clean_exit(){
    rm -- "$CFG"
    kill `cat ./uts-server.pid` >/dev/null 2>&1
    rm -- "./uts-server.pid"
    exit $1
}

sed "s/2020/$PORT/" tests/cfg/uts-server.cnf >$CFG

$TO ./uts-server -c $CFG -D -p ./uts-server.pid &

sleep 1
./goodies/timestamp-file.sh -i README.rst -u http://localhost:$PORT -r -O "-cert" || clean_exit 1
./goodies/timestamp-file.sh -i README.rst -u http://localhost:$PORT -r -O "-cert" || clean_exit 1
./goodies/timestamp-file.sh -i README.rst -u http://localhost:$PORT -r -O "-cert" || clean_exit 1

kill `cat ./uts-server.pid`

sed "s/2020/$PORT/" tests/cfg/uts-server-ssl.cnf >$CFG

sleep 1

$TO ./uts-server -c $CFG -D -p ./uts-server.pid &

sleep 1
./goodies/timestamp-file.sh -i README.rst -u https://localhost:$PORT -r -O "-cert" -C '-k' || clean_exit 1
./goodies/timestamp-file.sh -i README.rst -u https://localhost:$PORT -r -O "-cert" -C '-k' || clean_exit 1
./goodies/timestamp-file.sh -i README.rst -u https://localhost:$PORT -r -O "-cert" -C '-k' || clean_exit 1

kill `cat ./uts-server.pid`

clean_exit 0
