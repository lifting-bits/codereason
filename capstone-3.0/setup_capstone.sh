#/bin/bash

# make Capstone
tar -xf capstone-3.0.tgz
cd capstone-3.0
cd include
mkdir capstone
cp -R * capstone
cd ..
./make.sh

