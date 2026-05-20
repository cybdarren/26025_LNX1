#!/bin/bash
DEST_IP="$1"
DEBUG_PORT="$2"
BINARY="$3"
DEST_DIR="/root"

# kill gdbserver and delete old binary
echo "Killing old gdbserver instances on target and removing old binary"
ssh root@${DEST_IP} "sh -c '/usr/bin/killall -q gdbserver; rm -rf ${DEST_DIR}/${BINARY}  exit 0'"

# send binary to target
echo "Copying new binary to target"
scp ${BINARY} root@${DEST_IP}:${DEST_DIR}/${BINARY}

# start gdbserver on target
echo "Starting gdb server on target"
ssh -t root@${DEST_IP} "sh -c 'cd ${DEST_DIR}; gdbserver localhost:${DEBUG_PORT} ${BINARY}'"
