#/bin/bash

# make vex
cd ./vexTRUNK
make
cd ..

# make CodeReason
mkdir build
cd build
cmake ..
make -j8


