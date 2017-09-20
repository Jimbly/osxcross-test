#!/bin/sh
set -ex

mkdir -p build-darwin-64
cd build-darwin-64
x86_64-apple-darwin14-cc -DGLEW_NO_GLU -DGLEW_STATIC -I.. -o glew.c.o -c ../glew/glew.c
[ -e "libglew.a" ] && rm libglew.a
x86_64-apple-darwin14-ar cq libglew.a  glew.c.o
x86_64-apple-darwin14-ranlib libglew.a

x86_64-apple-darwin14-c++ -stdlib=libc++ -DGLM_COMPILER=0 -std=gnu++11 -o simple_test.cpp.o -c ../simple_test.cpp
x86_64-apple-darwin14-c++ -stdlib=libc++ simple_test.cpp.o -o SimpleTest  libglew.a
