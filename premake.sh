#!/bin/sh

A2_OS="unknown"
A2_PLATFORM="x32"
A2_MAKE="make"
A2_MAKE_PLATFORM="32"
A2_ARGS=""
A2_CPU_COUNT=1

if [[ $# > 0 && $1 == "gcc" ]]; then
	A2_ARGS="--gcc"
else
	A2_ARGS="--clang"
fi

case $( uname | tr [:upper:] [:lower:] ) in
	"darwin")
		A2_OS="macosx"
		A2_CPU_COUNT=$(sysctl -a | grep 'machdep.cpu.thread_count' | sed -E 's/.*(: )([:digit:]*)/\2/g')
		;;
	"linux")
		A2_OS="linux"
		A2_CPU_COUNT=$(cat /proc/cpuinfo | grep -m 1 'cpu cores' | sed -E 's/.*(: )([:digit:]*)/\2/g')
		;;
	[a-z0-9]*"BSD")
		A2_OS="bsd"
		A2_MAKE="gmake"
		# TODO: get cpu/thread count on *bsd
		;;
	"cygwin"*)
		A2_OS="windows"
		A2_ARGS+=" --env cygwin"
		A2_CPU_COUNT=$(env | grep 'NUMBER_OF_PROCESSORS' | sed -E 's/.*=([:digit:]*)/\1/g')
		;;
	"mingw"*)
		A2_OS="windows"
		A2_ARGS+=" --env mingw"
		A2_CPU_COUNT=$(env | grep 'NUMBER_OF_PROCESSORS' | sed -E 's/.*=([:digit:]*)/\1/g')
		;;
	*)
		echo "unknown operating system - exiting"
		exit
		;;
esac

case $( uname -m ) in
	"i386"|"i486"|"i586"|"i686")
		A2_PLATFORM="x32"
		A2_MAKE_PLATFORM="32"
		A2_ARGS+=" --platform x32"
		;;
	"x86_64"|"amd64")
		A2_PLATFORM="x64"
		A2_MAKE_PLATFORM="64"
		A2_ARGS+=" --platform x64"
		;;
	*)
		echo "unknown architecture - using "${A2_PLATFORM}
		exit;;
esac


echo "using: premake4 --cc=gcc --os="${A2_OS}" gmake "${A2_ARGS}

premake4 --cc=gcc --os=${A2_OS} gmake ${A2_ARGS}
sed -i -e 's/\${MAKE}/\${MAKE} -j '${A2_CPU_COUNT}'/' Makefile

chmod +x src/build_version.sh

echo ""
echo "#########################################################"
echo "# NOTE: use '"${A2_MAKE}"' to build a2elight"
echo "#########################################################"
echo ""
