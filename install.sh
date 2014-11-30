#!/bin/bash

if [[ $EUID -ne 0 ]]; then
   echo "This script must be run as root" 1>&2
   exit 1
fi
echo "installing"

sudo apt-get -y install build-essential g++ cmake libboost-dev libprotobuf-dev libprotobuf-lite7 libprotobuf7 libprotoc7 protobuf-compiler libboost-thread-dev libboost-system-dev libboost-filesystem-dev libboost-program-options-dev libboost-date-time-dev libboost-regex-dev

cd llvm-3.2
tar -xf llvm-3.2.src.tar.gz
cd llvm-3.2.src/
mkdir build
cd build/
cmake -DLLVM_REQUIRES_RTTI=1 -DCMAKE_BUILD_TYPE=RELEASE -DLLVM_TARGETS_TO_BUILD="X86;ARM" ..
make -j2
sudo make install
cd ../../../vexTRUNK
make
cd ..
mkdir build
cd build
cmake ..
make -j2
