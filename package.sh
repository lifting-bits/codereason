#!/bin/bash

echo "This script packages mcsema into a .deb package"
echo "Cleaning old diretories"
rm -rf ./deb-build

GIT_DATE=$(git log -n 1 --format="%ai")
DATE=$(date --utc --date="${GIT_DATE}" +%Y%m%d%H%M)
VERSION=0.1-${DATE}


echo "CodeReason Version is: ${VERSION}"

mkdir -p ./deb-build/codereason_${VERSION}/opt/codereason/bin/

cp -R ./debian ./deb-build/codereason_${VERSION}/DEBIAN
echo "Version: ${VERSION}" | cat - ./debian/control > ./deb-build/codereason_${VERSION}/DEBIAN/control

CR_BINS=$(find ./build/bin -executable -type f)

if [ "${CR_BINS}" == "" ]
then
    echo "Could not find mcsema binaries. Did you build CodeReason?"
    exit -1
fi

for BINFILE in ${CR_BINS}
do
    echo "Packaging ${BINFILE}..."
    cp ${BINFILE} ./deb-build/codereason_${VERSION}/opt/codereason/bin/
done


LLVM_BINS=$(find ./build/llvm-3.2 -executable -type f ! -name '*.so')
if [ "${LLVM_BINS}" == "" ]
then
    echo "Could not find LLVM binaries. Did you build CodeReason?"
    exit -1
fi

for BINFILE in ${LLVM_BINS}
do
    echo "Packaging ${BINFILE}..."
    cp ${BINFILE} ./deb-build/codereason_${VERSION}/opt/codereason/bin/
done

strip -s ./deb-build/codereason_${VERSION}/opt/codereason/bin/*

echo "Building .deb file..."
dpkg-deb -v --build ./deb-build/codereason_${VERSION}
