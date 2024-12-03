#!/bin/sh
# mk_kafka.sh bits 
#exec >$0.$PLATFORM.$BITS.log 2>&1
set -x
set -e

Usage()
{
	cat <<!!
Usage: $0 <bits>  
Bits is 32 or 64

eg
$0   64
!!
	exit 1
}

[ 2 -ne $# ] && Usage

BITS=$2

case $BITS in
	32)
		;;
	64)
		;;
	*)	Usage
		;;
esac


 
case $PLATFORM in
	linux|pi)
		ARCH=`uname -m`
		MAKE="make"
		EXT=so
		;;
	darwin|mac|mac_arm)
		ARCH=`uname -m`
		MAKE="make"
		EXT=dylib
		;;
	aix)
		ARCH=`uname -p`
		MAKE="gmake"
		EXT=so
		;;
	*)	ARCH=
		MAKE="make"
		EXT=so
		;;
esac




 BITS=$BITS PLATFORM=$PLATFORM ARCH=$ARCH  $MAKE -f kafka.make

