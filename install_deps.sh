#!/bin/bash

if [[ $EUID -ne 0 ]]; then
   echo "This script must be run as root" 1>&2
   exit 1
fi
echo "Installing dependencies"

sudo apt-get update
sudo apt-get -y install build-essential gcc g++ make cmake libboost-dev libprotobuf-dev protobuf-compiler libboost-thread-dev libboost-system-dev libboost-filesystem-dev libboost-program-options-dev libboost-date-time-dev libboost-regex-dev


