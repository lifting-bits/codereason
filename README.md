# CodeReason
[![Build Status](https://travis-ci.org/trailofbits/codereason.svg)](https://travis-ci.org/trailofbits/codereason)
[![Coverity Scan Build Status](https://scan.coverity.com/projects/5509/badge.svg)](https://scan.coverity.com/projects/5509)
[![Slack Chat](http://empireslacking.herokuapp.com/badge.svg)](https://empireslacking.herokuapp.com/)

CodeReason is a semantic binary code analysis framework and toolset. The tool RopTool discovers ROP gadgets in ARM, X86 and X86-64 binaries by providing pre- and post-conditions for the CPU and memory context using Lua scripts. Examples of other tools that can be created with CodeReason are available in the tools/ directory.

## Building
CodeReason builds on Linux and OS X. Windows are builds currently broken. [Help us fix them](https://github.com/trailofbits/codereason/issues/32)!

### Requirements
* [LibVEX](https://github.com/trailofbits/libvex) with custom patches to support static analysis
* [gtest](https://code.google.com/p/googletest/) for unit tests
* [lua](http://www.lua.org/home.html) for the user interface
* [protobuf](https://developers.google.com/protocol-buffers/)
* [boost](http://www.boost.org/)
* [capstone](http://www.capstone-engine.org/) for pretty printing disassembly

### Ubuntu
```
sudo ./install_deps.sh
./make.sh
```

### OS X
```
brew update && brew install cmake boost protobuf git
./install_vex.sh
./make.sh
```

Several helper scripts are available: `install_deps.sh` installs Ubuntu dependencies, `make.sh` creates a full build, `recompile.sh` recompiles CodeReason, and `package.sh` creates a debian package. See our [Travis-CI configuration](https://github.com/trailofbits/codereason/blob/master/.travis.yml) for more details about building.

## Usage

### Lua scripting
The Lua script bindings are defined in libs/VEE/VEElua.cpp. These bindings provide a way of describing CPU register values and memory contents to the VEX Execution Engine (VEE) which analyzes binary code.

The most common functions are:
* putreg - Writes value to a register `vee.putreg(v, R1, 32, 80808080)`
* putmem - Writes a value at an address `vee.putmem(v, 0x40000000, 32, 0x20202020)`
* getreg - Read value from a register `vee.getreg(v, R15, 32)`
* getmem - Read a value from memory `vee.getmem(v, 0x40000000, 32)`

For additional examples, check the scripts/ directory.

### RopTool
RopTool takes in a binary and a Lua script as input and will output results to stdout.

Example usage:
```
./build/bin/RopTool -a x64 -c ./scripts/x64/call_reg.lua -f ./tests/ELF/ls_x64
```

### BlockExtract
BlockExtract reads in a binary and outputs a database file containing block information. This can be useful when analyzing large binaries that take a long time to extract code blocks. Currently only 64-bit block extraction is supported.

Example usage:
```
./build/bin/BlockExtract -f ./tests/ELF/ls_x64 -a x64  --blocks-out ./blockdbfile
```

### BlockReader
BlockReader consumes the block database created by BlockExtract. It may be useful when debugging information stored inside of blocks. VEX output is printed to stdout.

Example usage:
```
./build/bin/BlockReader -d ./blockdbfile
```

### ImgTool
ImgTool is a test program that prints information about executable code sections found in a binary.

Example usage:
```
./build/bin/ImgTool -a x64 -f ./tests/MachO/ls_FAT_x86_x64
```
Example output:
```
In file ./tests/MachO/ls_FAT_x86_x64
found 6 +X sections
------------------
Section of arch AMD64
beginning at 0x1778 of size 0x3635
------------------
Section of arch AMD64
beginning at 0x4dae of size 0x1bc
------------------
Section of arch AMD64
beginning at 0x4f6c of size 0x2f4
------------------
Section of arch AMD64
beginning at 0x5260 of size 0x568
------------------
Section of arch AMD64
beginning at 0x57c8 of size 0x a0
------------------
Section of arch AMD64
beginning at 0x5868 of size 0x798
------------------
```

## References
[Semantic Analysis of Native Programs, introducing CodeReason](http://blog.trailofbits.com/2014/02/23/semantic-analysis-of-native-programs-introducing-codereason/)

## Authors
Originally developed by [Andrew Ruef](https://github.com/awruef) under contract for DARPA Cyber Fast Track.

Contributions made by:
* [Markus Gaasedelen](https://github.com/gaasedelen)
* [Jay Little](https://github.com/computerality)
* [Peter Goodman](https://github.com/pgoodman)
* Nick Anderson
* Santiago Torres
* Luke Mladek
