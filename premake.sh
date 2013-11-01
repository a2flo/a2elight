#!/bin/sh

A2_OS="unknown"
A2_PLATFORM="x32"
A2_MAKE="make"
A2_MAKE_PLATFORM="32"
A2_ARGS=""
A2_CPU_COUNT=1
A2_USE_CLANG=1

for arg in "$@"; do
	case $arg in
		"gcc")
			A2_ARGS+=" --gcc"
			A2_USE_CLANG=0
			;;
		"cuda")
			A2_ARGS+=" --cuda"
			;;
		*)
			;;
	esac
done

if [[ $A2_USE_CLANG == 1 ]]; then
	A2_ARGS+=" --clang"
fi

case $( uname | tr [:upper:] [:lower:] ) in
	"darwin")
		A2_OS="macosx"
		A2_CPU_COUNT=$(sysctl -n hw.ncpu)
		;;
	"linux")
		A2_OS="linux"
		# note that this includes hyper-threading and multi-socket systems
		A2_CPU_COUNT=$(cat /proc/cpuinfo | grep "processor" | wc -l)
		;;
	[a-z0-9]*"BSD")
		A2_OS="bsd"
		A2_MAKE="gmake"
		A2_CPU_COUNT=$(sysctl -n hw.ncpu)
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

A2_PLATFORM_TEST_STRING=""
if [[ $A2_OS != "windows" ]]; then
	A2_PLATFORM_TEST_STRING=$( uname -m )
else
	A2_PLATFORM_TEST_STRING=$( gcc -dumpmachine | sed "s/-.*//" )
fi

case $A2_PLATFORM_TEST_STRING in
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

if [[ $A2_USE_CLANG == 1 ]]; then
	sed -i '1i export CC=clang' Makefile
	sed -i '1i export CXX=clang++' Makefile
fi

chmod +x src/build_version.sh

echo ""
echo "#########################################################"
echo "# NOTE: use '"${A2_MAKE}"' to build a2elight"
echo "#########################################################"
echo ""
