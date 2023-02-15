#!/bin/bash

set -e

OPT="-O2"
DBG="-ggdb3 -DDEBUG"
#BAKAPIDIR="../bak_api"
CFLAGS="-Wall -Ibase64/include"
#CFLAGS+=" -DNOSCANF"
#CFLAGS+=" -I${BAKAPIDIR}"
OPTCFLAGS="${CFLAGS} ${OPT}"
DBGCFLAGS="${CFLAGS} ${DBG}"

pushd base64
make
popd

rm -f *.exe *.dbg

gcc ${OPTCFLAGS} -DPUBCODE pub.c getopts.c cJSON.c async_zmq_pub.c base64/lib/libbase64.o -lpthread -lzmq -o pub.exe
gcc ${DBGCFLAGS} -DPUBCODE pub.c getopts.c cJSON.c async_zmq_pub.c base64/lib/libbase64.o -lpthread -lzmq -o pub.dbg

gcc ${OPTCFLAGS} -DSUBCODE sub.c getopts.c cJSON.c async_zmq_sub.c hex2bin.c base64/lib/libbase64.o -lpthread -lzmq -o sub.exe
gcc ${DBGCFLAGS} -DSUBCODE sub.c getopts.c cJSON.c async_zmq_sub.c hex2bin.c base64/lib/libbase64.o -lpthread -lzmq -o sub.dbg

strip *.exe
