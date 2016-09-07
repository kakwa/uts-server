#!/bin/sh

timeout 20 ./uts-server -c tests/cfg/uts-server.cnf -D &

./goodies/timestamp-file.sh -i README.md -u http://localhost:2020 -r -O "-cert" || exit 1
./goodies/timestamp-file.sh -i README.md -u http://localhost:2020 -r -O "-cert" || exit 1
./goodies/timestamp-file.sh -i README.md -u http://localhost:2020 -r -O "-cert" || exit 1

