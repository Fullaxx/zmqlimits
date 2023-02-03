#!/bin/bash

set -e

OPT="-O2"
DBG="-ggdb3 -DDEBUG"
#BAKAPIDIR="../bak_api"
CFLAGS="-Wall"
#CFLAGS+=" -DNOSCANF"
#CFLAGS+=" -I${BAKAPIDIR}"
OPTCFLAGS="${CFLAGS} ${OPT}"
DBGCFLAGS="${CFLAGS} ${DBG}"

rm -f *.exe *.dbg

gcc ${OPTCFLAGS} -DPUBCODE pub.c getopts.c cJSON.c async_zmq_pub.c -lpthread -lzmq -o pub.exe
gcc ${DBGCFLAGS} -DPUBCODE pub.c getopts.c cJSON.c async_zmq_pub.c -lpthread -lzmq -o pub.dbg

gcc ${OPTCFLAGS} -DSUBCODE sub.c getopts.c cJSON.c async_zmq_sub.c hex2bin.c -lpthread -lzmq -o sub.exe
gcc ${DBGCFLAGS} -DSUBCODE sub.c getopts.c cJSON.c async_zmq_sub.c hex2bin.c -lpthread -lzmq -o sub.dbg

strip *.exe
