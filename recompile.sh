#/bin/bash

# make vex
cd ./vexTRUNK
#make clean
make
cd ..

# make CodeReason
mkdir build
cd build
cmake ..
make -j8


