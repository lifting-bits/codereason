CodeReason
============
[![Build Status](https://magnum.travis-ci.com/trailofbits/codereason.svg?token=T1UToSpCvaMxn511Cddb&branch=master)](https://magnum.travis-ci.com/trailofbits/codereason)

## Building
CodeReason depends on on library such as boost, libvex, and Capstone. To see a full list of dependencies, please see the install_deps.sh file

Currently CodeReason supports Ubuntu 14.04
The following commands will build on Ubuntu 14.04 32-bit:
```
sudo ./install_deps.sh  
./make.sh  
```
Mac OS X 10.10 also compiles and runs but is not well tested yet.

Windows is not currently supported.

## Usage

### RopTool
RopTool is the main application used to interact with binaries. It takes a binary and a Lua script as inputs and will output results to stdout. An example 
```
    ./build/bin/RopTool -f ../putty.exe -c scripts/call_reg.lua
```
### BlockExtract
BlockExtract reads in a binary and outputs a database file containing block information. This can be useful when analyzing large binaries that take a long time to extract code blocks. Currently only 64-bit block extraction is supported.

Example usage:
```
    ./BlockExtract -f ../../tests/ELF/ls_x64 -a x64  --blocks-out ./blockdbfile
```

### BlockReader
BlockReader consumes the block database created by BlockExtract. It may be useful when debugging information stored inside of blocks. VEX output is printed to stdout.

Example usage:
```
    ./BlockReader -a ./blockdbfile
```

## ImgTool
ImgTool is a test program that prints information about executable code sections found in a binary.

Example usage:
```
    ./ImgTool -a x64 -f ../../tests/EXE/x64_calc.exe
```
Example output:
```
    In file ../../tests/EXE/x64_calc.exe
    found 1 +X sections
    ------------------
    Section of arch X86
    beginning at 0x401000 of size 0x5ae00
```

## References
[Semantic Analysis of Native Programs, introducing CodeReason](http://blog.trailofbits.com/2014/02/23/semantic-analysis-of-native-programs-introducing-codereason/)

## Authors
* Andrew Ruef
* Markus Gaasedelen
* Jay Little
