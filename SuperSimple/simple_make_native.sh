#!/bin/sh
set -ex

mkdir -p build-darwin-64
cd build-darwin-64

cc -DGLEW_NO_GLU -DGLEW_STATIC -I.. -o glew.c.o -c ../glew/glew.c
[ -e "libglew.a" ] && rm libglew.a
ar cq libglew.a  glew.c.o
ranlib libglew.a

c++ -stdlib=libc++ -DGLM_COMPILER=0 -std=gnu++11 -o simple_test.cpp.o -c ../simple_test.cpp
c++ -stdlib=libc++ simple_test.cpp.o -o SimpleTest  libglew.a

