#!/bin/bash

# build device library
xmake f -p linux --sdk=${DDK_HOME}toolchains/aarch64-linux-gcc6.3
xmake -v -b SSDDetection
xmake -v -b JpegEncode
xmake -v -b VDecEngine

# build host library
xmake f -p linux --sdk=${DDK_HOME}toolchains/Euler_compile_env_cross/arm/cross_compile/install
xmake -v -b StreamPuller
xmake -v -b DstEngine
xmake -v -b main
