#/bin/bash

# make Capstone
cd capstone-3.0
tar -xf capstone-3.0.tgz
cd capstone-3.0
cd include
mkdir capstone
cp -R * capstone
cd ..
./make.sh

# make vex
cd ../../vexTRUNK
make
cd ..

# make CodeReason
mkdir build
cd build
cmake ..
make -j8

