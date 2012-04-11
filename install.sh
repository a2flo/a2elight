#!/bin/sh

A2_PLATFORM="x86"
A2_PLATFORM_TEST_STRING=""
if [[ $( uname -o ) != "Msys" ]]; then
	A2_PLATFORM_TEST_STRING=$( uname -m )
else
	A2_PLATFORM_TEST_STRING=$( gcc -dumpmachine | sed "s/-.*//" )
fi

case $A2_PLATFORM_TEST_STRING in
	"i386"|"i486"|"i586"|"i686")
		A2_PLATFORM="x86"
		;;
	"x86_64"|"amd64")
		A2_PLATFORM="x64"
		;;
	*)
		echo "unknown architecture - using "${A2_PLATFORM}
		exit;;
esac

declare -a paths=( cl core gui particle rendering scene threading scene/model rendering/renderer rendering/renderer/gl3 )
case $( uname | tr [:upper:] [:lower:] ) in
	"linux"|[a-z0-9]*"BSD")
		A2_INCLUDE_PATH="/usr/local/include"
		A2_BIN_PATH="/usr/local/bin"
		A2_LIB_PATH="/usr/local/lib"
	
		# remove old files and folders
		rm -Rf ${A2_INCLUDE_PATH}/a2elight
		rm -f ${A2_BIN_PATH}/liba2elight.so
		rm -f ${A2_BIN_PATH}/liba2elightd.so
		rm -f ${A2_LIB_PATH}/liba2elight.a
		rm -f ${A2_LIB_PATH}/liba2elightd.a
		
		# create/copy new files and folders
		mkdir ${A2_INCLUDE_PATH}/a2elight
		for val in ${paths[@]}; do
			mkdir -p ${A2_INCLUDE_PATH}/a2elight/${val}
		done
		cp *.h ${A2_INCLUDE_PATH}/a2elight/ 2>/dev/null
		cp *.hpp ${A2_INCLUDE_PATH}/a2elight/ 2>/dev/null
		for val in ${paths[@]}; do
			cp ${val}/*.h ${A2_INCLUDE_PATH}/a2elight/${val}/ 2>/dev/null
			cp ${val}/*.hpp ${A2_INCLUDE_PATH}/a2elight/${val}/ 2>/dev/null
		done
		
		cp ./../lib/liba2elight.so ${A2_BIN_PATH}/ 2>/dev/null
		cp ./../lib/liba2elightd.so ${A2_BIN_PATH}/ 2>/dev/null
		cp ./../lib/liba2elight.a ${A2_LIB_PATH}/ 2>/dev/null
		cp ./../lib/liba2elightd.a ${A2_LIB_PATH}/ 2>/dev/null
		;;
	"mingw"*)
		A2_INCLUDE_PATH="/c/MinGW/msys/1.0/local/include"
		A2_BIN_PATH="/c/MinGW/msys/1.0/local/bin"
		A2_LIB_PATH="/c/MinGW/msys/1.0/local/lib"
		if [[ $A2_PLATFORM == "x64" ]]; then
			A2_INCLUDE_PATH="/c/mingw64/msys/include"
			A2_BIN_PATH="/c/mingw64/msys/bin"
			A2_LIB_PATH="/c/mingw64/msys/lib"
		fi
	
		# remove old files and folders
		rm -Rf ${A2_INCLUDE_PATH}/a2elight
		rm -f ${A2_BIN_PATH}/a2elight.dll
		rm -f ${A2_BIN_PATH}/a2elightd.dll
		rm -f ${A2_LIB_PATH}/liba2elight.a
		rm -f ${A2_LIB_PATH}/liba2elightd.a
		
		# create/copy new files and folders
		mkdir ${A2_INCLUDE_PATH}/a2elight
		for val in ${paths[@]}; do
			mkdir -p ${A2_INCLUDE_PATH}/a2elight/${val}
		done
		cp *.h ${A2_INCLUDE_PATH}/a2elight/ 2>/dev/null
		cp *.hpp ${A2_INCLUDE_PATH}/a2elight/ 2>/dev/null
		for val in ${paths[@]}; do
			cp ${val}/*.h ${A2_INCLUDE_PATH}/a2elight/${val}/ 2>/dev/null
			cp ${val}/*.hpp ${A2_INCLUDE_PATH}/a2elight/${val}/ 2>/dev/null
		done
		
		cp ./../lib/${A2_PLATFORM}/a2elight.dll ${A2_BIN_PATH}/ 2>/dev/null
		cp ./../lib/${A2_PLATFORM}/a2elightd.dll ${A2_BIN_PATH}/ 2>/dev/null
		cp ./../lib/${A2_PLATFORM}/liba2elight.a ${A2_LIB_PATH}/ 2>/dev/null
		cp ./../lib/${A2_PLATFORM}/liba2elightd.a ${A2_LIB_PATH}/ 2>/dev/null
		;;
	*)
		echo "unknown operating system - exiting"
		exit
		;;
esac

echo ""
echo "#########################################################"
echo "# a2elight has been installed!"
echo "#########################################################"
echo ""
