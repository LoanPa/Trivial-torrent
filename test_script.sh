#!/bin/bash

set -e

function finish {
	rm -f torrent_samples/client/test_file torrent_samples/client_output.tmp torrent_samples/server_output.tmp
	echo "##################################################################"
	echo "TESTS RESULT: $RESULT"
	echo "##################################################################"
}

trap finish EXIT

RESULT="FAIL"

echo "##################################################################"
echo "TRIVIAL TORRENT CLIENT (testing against reference server)"
echo "##################################################################"

echo "Starting Reference Server" 

reference_binary/ttorrent -l 8080 torrent_samples/server/test_file_server.ttorrent > torrent_samples/server_output.tmp 2>&1 &
SERVERPID=$!

echo "Waiting one second for it to start"

sleep 1

echo "Starting Client"

bin/ttorrent torrent_samples/client/test_file.ttorrent > torrent_samples/client_output.tmp 2>&1

echo "#################### Start of Server Output ####################"
cat torrent_samples/server_output.tmp
echo "#################### End of Server Output   ####################"
echo "#################### Start of Client Output ####################"
cat torrent_samples/client_output.tmp
echo "#################### End of Client Output   ####################"

echo "Killing server..."

kill -9 "$SERVERPID" 2> /dev/null > /dev/null || { echo "Server is dead but should be alive!" ; exit 1 ; }

echo "Comparing downloaded file"

cmp torrent_samples/server/test_file_server torrent_samples/client/test_file || {
	echo "ERROR: downloaded file does not match!" && false;
}

rm -f torrent_samples/client/test_file torrent_samples/client_output.tmp torrent_samples/server_output.tmp

echo "##################################################################"
echo "TRIVIAL TORRENT SERVER (testing against reference client)"
echo "##################################################################"

echo "Starting Server" 

bin/ttorrent -l 8080 torrent_samples/server/test_file_server.ttorrent > torrent_samples/server_output.tmp 2>&1 &
SERVERPID=$!

echo "Waiting one second for it to start"

sleep 1

echo "Starting Reference Client"

reference_binary/ttorrent torrent_samples/client/test_file.ttorrent > torrent_samples/client_output.tmp 2>&1

echo "#################### Start of Server Output ####################"
cat torrent_samples/server_output.tmp
echo "#################### End of Server Output   ####################"
echo "#################### Start of Client Output ####################"
cat torrent_samples/client_output.tmp
echo "#################### End of Client Output   ####################"

echo "Killing server..."

kill -9 "$SERVERPID" 2> /dev/null > /dev/null || { echo "Server is dead but should be alive!" ; exit 1 ; }

echo "Comparing downloaded file"

cmp torrent_samples/server/test_file_server torrent_samples/client/test_file || {
	echo "ERROR: downloaded file does not match!" && false;
}

rm -f torrent_samples/client/test_file torrent_samples/client_output.tmp torrent_samples/server_output.tmp

RESULT="PASS"

