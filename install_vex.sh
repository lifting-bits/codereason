#!/bin/sh

git clone https://github.com/trailofbits/libvex.git
cd libvex
sudo make install
cd ..
rm -rf libvex
