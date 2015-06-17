#/bin/bash

# make Capstone
rm -rf capstone-3.0.3
tar -xf capstone-3.0.3.tgz
cd capstone-3.0.3
cd include
mkdir capstone
cp -R *.h capstone
cd ..
./make.sh

