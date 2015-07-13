#!/bin/sh

git clone https://github.com/trailofbits/libvex.git
cd libvex
CFLAGS=-g VEX_INSTALL_DIR=./build make install
cd ..
