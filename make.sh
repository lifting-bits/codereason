#/bin/bash

# make Capstone
cd capstone-3.0
./setup_capstone.sh

# make vex
cd ../../vexTRUNK
make
cd ..

# make CodeReason
mkdir build
cd build
cmake ..
make -j8

