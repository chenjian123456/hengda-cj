#!/bin/bash
path_cur=$(cd `dirname $0`; pwd)

#-------------------------------------------------------------
# ENV
export FFMPEG_PATH=/opt/aarch64/ffmpeg

#-------------------------------------------------------------
# check DDK_HOME
if [ -z $DDK_HOME ];then
	echo "[ERROR] DDK_HOME does not exist! Please set environment variable: export DDK_HOME=<root folder of ddk>"
	echo "eg:  export DDK_HOME==/home/<ddk install user>/tools/che/ddk/ddk/"
	exit 0
fi

#-------------------------------------------------------------
# build
echo "================================================================"
echo "================> "$path_cur
echo "================> Build for $build_target($build_type)"
echo "================> "
echo "================> DDK_HOME:	$DDK_HOME"
echo "================> FFmpeg:	$FFMPEG_PATH"
echo "================> "
echo "================> Binary dir: $path_cur/out"
echo "================> Out dir: $HOME/"
echo "================================================================"
echo ""

build_type="Release"
build_target="A500"
path_build=$path_cur/out
path_toolchain=$path_cur/cmake/Euler.cmake

rm -rf $path_build
mkdir -p  $path_build
cd  $path_build

cmake -DCMAKE_BUILD_TARGET=$build_target -DCMAKE_BUILD_TYPE=$build_type -DCMAKE_TOOLCHAIN_FILE=$path_toolchain ..

make -j8

#-------------------------------------------------------------
cd $path_cur/

tar -czf out.tar out
rm -r ./out
#rm $HOME/out.tar
#mv ./out.tar $HOME

#echo "================================================================"
#ls -lh $HOME/out.tar 
#echo "================================================================"
