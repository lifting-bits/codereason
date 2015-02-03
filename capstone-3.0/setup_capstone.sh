#/bin/bash

# make Capstone
rm -rf capstone-3.0
tar -xf capstone-3.0.tgz
cd capstone-3.0
cd include
mkdir capstone
cp -R * capstone
cd ..
./make.sh

