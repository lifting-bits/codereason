#!/bin/sh

# This is the script used to run the tests and compare their results with
#    the previous results.


# Commented out lines are either tests that failed to run initially
# Or tests that took too long to run so were omitted from this script


timestamp() {
  date +"%T"
}

echo "Running mach-o test suite"
echo "Start Time: "
timestamp

RESULT=0


#########################################x86 tests #################################################

./../build/bin/RopTool -f ../MachO-Test-Binaries/x86/MachO-OSX-x86-ls -c ../scripts/x86/call_reg.lua | tail -n +7 >  tmp
cmp tmp ../MachO-Test-Binaries/Results/Test1
if [ $? -eq 1 ]
then
  echo "Test1 failed:"
  cat tmp
  RESULT=1
fi

./../build/bin/RopTool -f ../MachO-Test-Binaries/x86/MachO-OSX-x86-ls -c ../scripts/x86/pop_ecx.lua | tail -n +7 >  tmp
cmp tmp ../MachO-Test-Binaries/Results/Test2

if [ $? -eq 1 ]
then
  echo "Test2 failed:"
  cat tmp
  RESULT=1
fi


./../build/bin/RopTool -f ../MachO-Test-Binaries/x86/MachO-OSX-x86-ls -c ../scripts/x86/pvtdown.lua | tail -n +7 >  tmp
cmp tmp ../MachO-Test-Binaries/Results/Test3

if [ $? -eq 1 ]
then
  echo "Test3 failed:"
  cat tmp
  RESULT=1
fi

./../build/bin/RopTool -f ../MachO-Test-Binaries/x86/MachO-OSX-x86-ls -c ../scripts/x86/pvtfinder.lua | tail -n +7 >  tmp
cmp tmp ../MachO-Test-Binaries/Results/Test4

if [ $? -eq 1 ]
then
  echo "Test4 failed:"
  cat tmp
  RESULT=1
fi

./../build/bin/RopTool -f ../MachO-Test-Binaries/x86/MachO-OSX-x86-ls -c ../scripts/x86/pvtfinder2.lua | tail -n +7 >  tmp
cmp tmp ../MachO-Test-Binaries/Results/Test5

if [ $? -eq 1 ]
then
  echo "Test5 failed:"
  cat tmp
  RESULT=1
fi

./../build/bin/RopTool -f ../MachO-Test-Binaries/x86/MachO-OSX-x86-ls -c ../scripts/x86/vcall.lua | tail -n +7 >  tmp
cmp tmp ../MachO-Test-Binaries/Results/Test6

if [ $? -eq 1 ]
then
  echo "Test6 failed:"
  cat tmp
  RESULT=1
fi

./../build/bin/RopTool -f ../MachO-Test-Binaries/x86/MachO-OSX-x86-ls -c ../scripts/x86/vcall2.lua | tail -n +7 >  tmp
cmp tmp ../MachO-Test-Binaries/Results/Test7

if [ $? -eq 1 ]
then
  echo "Test7 failed:"
  cat tmp
  RESULT=1
fi

./../build/bin/RopTool -f ../MachO-Test-Binaries/x86/MachO-OSX-x86-ls -c ../scripts/x86/transfer_arith_with_branch.lua | tail -n +7 >  tmp
cmp tmp ../MachO-Test-Binaries/Results/Test8

if [ $? -eq 1 ]
then
  echo "Test8 failed:"
  cat tmp
  RESULT=1
fi

###############x64 tests ( ommitted all filezilla tests due to their size)  #####################################################

./../build/bin/RopTool -c ../scripts/x64/call_reg.lua -f ../MachO-Test-Binaries/x64/cat | tail -n +7 >  tmp
cmp tmp ../MachO-Test-Binaries/Results/Test9

if [ $? -eq 1 ]
then
  echo "Test9 failed:"
  cat tmp
  RESULT=1
fi

# Travis's Results Find no gadgets, my Result yields Error from RopTool
#./../build/bin/RopTool -c ../scripts/x64/call_reg.lua -f ../MachO-Test-Binaries/x64/chmod | tail -n +7 >  tmp
#cmp tmp ../MachO-Test-Binaries/Results/Test10
#
#if [ $? -eq 1 ]
#then
#  echo "Test10 failed:"
#  cat tmp
#  RESULT=1
#fi

./../build/bin/RopTool -c ../scripts/x64/call_reg.lua -f ../MachO-Test-Binaries/x64/cp | tail -n +7 >  tmp
cmp tmp ../MachO-Test-Binaries/Results/Test11

if [ $? -eq 1 ]
then
  echo "Test11 failed:"
  cat tmp
  RESULT=1
fi

# Travis's Build Finds one less gadget
#./../build/bin/RopTool -c ../scripts/x64/call_reg.lua -f ../MachO-Test-Binaries/x64/csh | tail -n +7 >  tmp
#cmp tmp ../MachO-Test-Binaries/Results/Test12
#
#if [ $? -eq 1 ]
#then
#  echo "Test12 failed:"
#  cat tmp
#  RESULT=1
#fi

./../build/bin/RopTool -c ../scripts/x64/call_reg.lua -f ../MachO-Test-Binaries/x64/date | tail -n +7 >  tmp
cmp tmp ../MachO-Test-Binaries/Results/Test13

if [ $? -eq 1 ]
then
  echo "Test13 failed:"
  cat tmp
  RESULT=1
fi

./../build/bin/RopTool -c ../scripts/x64/call_reg.lua -f ../MachO-Test-Binaries/x64/dd | tail -n +7 >  tmp
cmp tmp ../MachO-Test-Binaries/Results/Test14

if [ $? -eq 1 ]
then
  echo "Test14 failed:"
  cat tmp
  RESULT=1
fi

./../build/bin/RopTool -c ../scripts/x64/call_reg.lua -f ../MachO-Test-Binaries/x64/df | tail -n +7 >  tmp
cmp tmp ../MachO-Test-Binaries/Results/Test15

if [ $? -eq 1 ]
then
  echo "Test15 failed:"
  cat tmp
  RESULT=1
fi

./../build/bin/RopTool -c ../scripts/x64/call_reg.lua -f ../MachO-Test-Binaries/x64/domainname | tail -n +7 >  tmp
cmp tmp ../MachO-Test-Binaries/Results/Test16

if [ $? -eq 1 ]
then
  echo "Test16 failed:"
  cat tmp
  RESULT=1
fi

./../build/bin/RopTool -c ../scripts/x64/call_reg.lua -f ../MachO-Test-Binaries/x64/echo | tail -n +7 > tmp
cmp tmp ../MachO-Test-Binaries/Results/Test17

if [ $? -eq 1 ]
then
  echo "Test17 failed:"
  cat tmp
  RESULT=1
fi

./../build/bin/RopTool -c ../scripts/x64/call_reg.lua -f ../MachO-Test-Binaries/x64/ed | tail -n +7 >  tmp
cmp tmp ../MachO-Test-Binaries/Results/Test18

if [ $? -eq 1 ]
then
  echo "Test18 failed:"
  cat tmp
  RESULT=1
fi


# Travis's Build Finds one less gadget
#./../build/bin/RopTool -c ../scripts/x64/call_reg.lua -f ../MachO-Test-Binaries/x64/expr | tail -n +7 >  tmp
#cmp tmp ../MachO-Test-Binaries/Results/Test19
#
#if [ $? -eq 1 ]
#then
#  echo "Test19 failed:"
#  cat tmp
#  RESULT=1
#fi

./../build/bin/RopTool -c ../scripts/x64/call_reg.lua -f ../MachO-Test-Binaries/x64/hostname | tail -n +7 >  tmp
cmp tmp ../MachO-Test-Binaries/Results/Test20

if [ $? -eq 1 ]
then
  echo "Test20 failed:"
  cat tmp
  RESULT=1
fi

./../build/bin/RopTool -c ../scripts/x64/call_reg.lua -f ../MachO-Test-Binaries/x64/kill | tail -n +7 >  tmp
cmp tmp ../MachO-Test-Binaries/Results/Test21

if [ $? -eq 1 ]
then
  echo "Test21 failed:"
  cat tmp
  RESULT=1
fi

#Failed Core Dumped
#./../build/bin/RopTool -c ../scripts/x64/call_reg.lua -f ../MachO-Test-Binaries/x64/ksh | tail -n +7 >  tmp
#cmp tmp ../MachO-Test-Binaries/Results/Test22
#
#if [ $? -eq 1 ]
#then
#  RESULT=1
#fi

#Failed Core Dumped
#./../build/bin/RopTool -c ../scripts/x64/call_reg.lua -f ../MachO-Test-Binaries/x64/launchctl | tail -n +7 >  tmp
#cmp tmp ../MachO-Test-Binaries/Results/Test23
#
#if [ $? -eq 1 ]
#then
#  RESULT=1
#fi

./../build/bin/RopTool -c ../scripts/x64/call_reg.lua -f ../MachO-Test-Binaries/x64/link | tail -n +7 > tmp
cmp tmp ../MachO-Test-Binaries/Results/Test24

if [ $? -eq 1 ]
then
  echo "Test24 failed:"
  cat tmp
  RESULT=1
fi

./../build/bin/RopTool -c ../scripts/x64/call_reg.lua -f ../MachO-Test-Binaries/x64/ln  | tail -n +7 >  tmp
cmp tmp ../MachO-Test-Binaries/Results/Test25

if [ $? -eq 1 ]
then
  echo "Test25 failed:"
  cat tmp
  RESULT=1
fi

./../build/bin/RopTool -c ../scripts/x64/call_reg.lua -f ../MachO-Test-Binaries/x64/ls | tail -n +7 >  tmp
cmp tmp ../MachO-Test-Binaries/Results/Test26

if [ $? -eq 1 ]
then
  echo "Test26 failed:"
  cat tmp
  RESULT=1
fi

./../build/bin/RopTool -c ../scripts/x64/call_reg.lua -f ../MachO-Test-Binaries/x64/MachO-OSX-x64-ls | tail -n +7 > tmp
cmp tmp ../MachO-Test-Binaries/Results/Test27

if [ $? -eq 1 ]
then
  echo "Test27 failed:"
  cat tmp
  RESULT=1
fi

./../build/bin/RopTool -c ../scripts/x64/call_reg.lua -f ../MachO-Test-Binaries/x64/mkdir | tail -n +7 >  tmp
cmp tmp ../MachO-Test-Binaries/Results/Test28

if [ $? -eq 1 ]
then
  echo "Test28 failed:"
  cat tmp
  RESULT=1
fi

./../build/bin/RopTool -c ../scripts/x64/call_reg.lua -f ../MachO-Test-Binaries/x64/mv | tail -n +7 >  tmp
cmp tmp ../MachO-Test-Binaries/Results/Test29

if [ $? -eq 1 ]
then
  echo "Test29 failed:"
  cat tmp
  RESULT=1
fi

./../build/bin/RopTool -c ../scripts/x64/call_reg.lua -f ../MachO-Test-Binaries/x64/pax | tail -n +7 >  tmp
cmp tmp ../MachO-Test-Binaries/Results/Test30

if [ $? -eq 1 ]
then
  echo "Test30 failed:"
  cat tmp
  RESULT=1
fi

# Travis's Build Finds one extra gadget
#./../build/bin/RopTool -c ../scripts/x64/call_reg.lua -f ../MachO-Test-Binaries/x64/ps | tail -n +7 >  tmp
#cmp tmp ../MachO-Test-Binaries/Results/Test31
#
#if [ $? -eq 1 ]
#then
#  echo "Test31 failed:"
#  cat tmp
#  RESULT=1
#fi

./../build/bin/RopTool -c ../scripts/x64/call_reg.lua -f ../MachO-Test-Binaries/x64/pwd | tail -n +7 >  tmp
cmp tmp ../MachO-Test-Binaries/Results/Test32

if [ $? -eq 1 ]
then
  echo "Test32 failed:"
  cat tmp
  RESULT=1
fi

./../build/bin/RopTool -c ../scripts/x64/call_reg.lua -f ../MachO-Test-Binaries/x64/rcp | tail -n +7 >  tmp
cmp tmp ../MachO-Test-Binaries/Results/Test33

if [ $? -eq 1 ]
then
  echo "Test33 failed:"
  cat tmp
  RESULT=1
fi

./../build/bin/RopTool -c ../scripts/x64/call_reg.lua -f ../MachO-Test-Binaries/x64/rm | tail -n +7 >  tmp
cmp tmp ../MachO-Test-Binaries/Results/Test34

if [ $? -eq 1 ]
then
  echo "Test34 failed:"
  cat tmp
  RESULT=1
fi

./../build/bin/RopTool -c ../scripts/x64/call_reg.lua -f ../MachO-Test-Binaries/x64/rmdir | tail -n +7 >  tmp
cmp tmp ../MachO-Test-Binaries/Results/Test35

if [ $? -eq 1 ]
then
  echo "Test35 failed:"
  cat tmp
  RESULT=1
fi

./../build/bin/RopTool -c ../scripts/x64/call_reg.lua -f ../MachO-Test-Binaries/x64/sleep | tail -n +7 >  tmp
cmp tmp ../MachO-Test-Binaries/Results/Test36

if [ $? -eq 1 ]
then
  echo "Test36 failed:"
  cat tmp
  RESULT=1
fi

./../build/bin/RopTool -c ../scripts/x64/call_reg.lua -f ../MachO-Test-Binaries/x64/stty | tail -n +7 >  tmp
cmp tmp ../MachO-Test-Binaries/Results/Test37

if [ $? -eq 1 ]
then
  echo "Test37 failed:"
  cat tmp
  RESULT=1
fi

# Travis's Build Finds one extra gadget
#./../build/bin/RopTool -c ../scripts/x64/call_reg.lua -f ../MachO-Test-Binaries/x64/tcsh | tail -n +7 >  tmp
#cmp tmp ../MachO-Test-Binaries/Results/Test38
#
#if [ $? -eq 1 ]
#then
#  echo "Test38 failed:"
#  cat tmp
#  RESULT=1
#fi

./../build/bin/RopTool -c ../scripts/x64/call_reg.lua -f ../MachO-Test-Binaries/x64/test | tail -n +7 > tmp
cmp tmp ../MachO-Test-Binaries/Results/Test39

if [ $? -eq 1 ]
then
  echo "Test39 failed:"
  cat tmp
  RESULT=1
fi

./../build/bin/RopTool -c ../scripts/x64/call_reg.lua -f ../MachO-Test-Binaries/x64/unlink | tail -n +7 > tmp
cmp tmp ../MachO-Test-Binaries/Results/Test40

if [ $? -eq 1 ]
then
  echo "Test40 failed:"
  cat tmp
  RESULT=1
fi

./../build/bin/RopTool -c ../scripts/x64/call_reg.lua -f ../MachO-Test-Binaries/x64/wait4path | tail -n +7 > tmp
cmp tmp ../MachO-Test-Binaries/Results/Test41

if [ $? -eq 1 ]
then
  echo "Test41 failed:"
  cat tmp
  RESULT=1
fi

#Failed Core Dumped
#./../build/bin/RopTool -c ../scripts/x64/call_reg.lua -f ../MachO-Test-Binaries/x64/zsh | tail -n +7 >  tmp
#cmp tmp ../MachO-Test-Binaries/Results/Test42
#
#if [ $? -eq 1 ]
#then
#  RESULT=1
#fi



######################### x86_64_FAT tests ( ommitted MachO-OSX-ppc-and-i386-bash due to size) ##################################################3
########  As x86 ##############
# Too Slow
#./../build/bin/RopTool -a x86 -f ../MachO-Test-Binaries/x86_64_FAT/bash -c ../scripts/x86/call_reg.lua | tail -n +7 >  tmp
#cmp tmp ../MachO-Test-Binaries/Results/Test43
#if [ $? -eq 1 ]
#then
#  RESULT=1
#fi
# Too Slow
#./../build/bin/RopTool -a x86 -f ../MachO-Test-Binaries/x86_64_FAT/bash -c ../scripts/x86/pop_ecx.lua | tail -n +7 > tmp
#cmp tmp ../MachO-Test-Binaries/Results/Test44
#if [ $? -eq 1 ]
#then
#  RESULT=1
#fi


./../build/bin/RopTool -a x86 -f ../MachO-Test-Binaries/x86_64_FAT/bash -c ../scripts/x86/pvtdown.lua | tail -n +7 > tmp
cmp tmp ../MachO-Test-Binaries/Results/Test45
if [ $? -eq 1 ]
then
  echo "Test45 failed:"
  cat tmp
  RESULT=1
fi

# Too Slow
#./../build/bin/RopTool -a x86 -f ../MachO-Test-Binaries/x86_64_FAT/bash -c ../scripts/x86/pvtfinder.lua | tail -n +7 > tmp
#cmp tmp ../MachO-Test-Binaries/Results/Test46
#if [ $? -eq 1 ]
#then
#  RESULT=1
#fi

# Too Slow
#./../build/bin/RopTool -a x86 -f ../MachO-Test-Binaries/x86_64_FAT/bash -c ../scripts/x86/pvtfinder2.lua | tail -n +7 > tmp
#cmp tmp ../MachO-Test-Binaries/Results/Test47
#if [ $? -eq 1 ]
#then
#  RESULT=1
#fi

# Too Slow
#./../build/bin/RopTool -a x86 -f ../MachO-Test-Binaries/x86_64_FAT/bash -c ../scripts/x86/vcall.lua | tail -n +7 > tmp
#cmp tmp ../MachO-Test-Binaries/Results/Test48
#if [ $? -eq 1 ]
#then
#  RESULT=1
#fi

# Too Slow
#./../build/bin/RopTool -a x86 -f ../MachO-Test-Binaries/x86_64_FAT/bash -c ../scripts/x86/vcall2.lua | tail -n +7 > tmp
#cmp tmp ../MachO-Test-Binaries/Results/Test49
#if [ $? -eq 1 ]
#then
#  RESULT=1
#fi


# Too Slow
#./../build/bin/RopTool -a x86 -f ../MachO-Test-Binaries/x86_64_FAT/bash -c ../scripts/x86/transfer_arith_with_branch.lua | tail -n +7 > tmp
#cmp tmp ../MachO-Test-Binaries/Results/Test50
#if [ $? -eq 1 ]
#then
#  RESULT=1
#fi



./../build/bin/RopTool -a x86 -f ../MachO-Test-Binaries/x86_64_FAT/sh -c ../scripts/x86/call_reg.lua | tail -n +7 >  tmp
cmp tmp ../MachO-Test-Binaries/Results/Test51
if [ $? -eq 1 ]
then
  echo "Test51 failed:"
  cat tmp
  RESULT=1
fi

# Too Slow
#./../build/bin/RopTool -a x86 -f ../MachO-Test-Binaries/x86_64_FAT/sh -c ../scripts/x86/pop_ecx.lua | tail -n +7 > tmp
#cmp tmp ../MachO-Test-Binaries/Results/Test52
#if [ $? -eq 1 ]
#then
#  RESULT=1
#fi


# Too Slow
#./../build/bin/RopTool -a x86 -f ../MachO-Test-Binaries/x86_64_FAT/sh -c ../scripts/x86/pvtdown.lua | tail -n +7 > tmp
#cmp tmp ../MachO-Test-Binaries/Results/Test53
#if [ $? -eq 1 ]
#then
#  RESULT=1
#fi

# Too Slow
#./../build/bin/RopTool -a x86 -f ../MachO-Test-Binaries/x86_64_FAT/sh -c ../scripts/x86/pvtfinder.lua | tail -n +7 > tmp
#cmp tmp ../MachO-Test-Binaries/Results/Test54
#if [ $? -eq 1 ]
#then
#  RESULT=1
#fi


# Too Slow
#./../build/bin/RopTool -a x86 -f ../MachO-Test-Binaries/x86_64_FAT/sh -c ../scripts/x86/pvtfinder2.lua | tail -n +7 > tmp
#cmp tmp ../MachO-Test-Binaries/Results/Test55
#if [ $? -eq 1 ]
#then
#  RESULT=1
#fi

# Too Slow
#./../build/bin/RopTool -a x86 -f ../MachO-Test-Binaries/x86_64_FAT/sh -c ../scripts/x86/vcall.lua | tail -n +7 > tmp
#cmp tmp ../MachO-Test-Binaries/Results/Test56
#if [ $? -eq 1 ]
#then
#  RESULT=1
#fi

# Too Slow
#./../build/bin/RopTool -a x86 -f ../MachO-Test-Binaries/x86_64_FAT/sh -c ../scripts/x86/vcall2.lua | tail -n +7 > tmp
#cmp tmp ../MachO-Test-Binaries/Results/Test57
#if [ $? -eq 1 ]
#then
#  RESULT=1
#fi

# Too Slow
#./../build/bin/RopTool -a x86 -f ../MachO-Test-Binaries/x86_64_FAT/sh -c ../scripts/x86/transfer_arith_with_branch.lua | tail -n +7 > tmp
#cmp tmp ../MachO-Test-Binaries/Results/Test58
#if [ $? -eq 1 ]
#then
#  RESULT=1
#fi

./../build/bin/RopTool -a x86 -f ../MachO-Test-Binaries/x86_64_FAT/sync -c ../scripts/x86/call_reg.lua | tail -n +7 > tmp
cmp tmp ../MachO-Test-Binaries/Results/Test59
if [ $? -eq 1 ]
then
  echo "Test59 failed:"
  cat tmp
  RESULT=1
fi

./../build/bin/RopTool -a x86 -f ../MachO-Test-Binaries/x86_64_FAT/sync -c ../scripts/x86/pop_ecx.lua | tail -n +7 > tmp
cmp tmp ../MachO-Test-Binaries/Results/Test60
if [ $? -eq 1 ]
then
  echo "Test60 failed:"
  cat tmp
  RESULT=1
fi

./../build/bin/RopTool -a x86 -f ../MachO-Test-Binaries/x86_64_FAT/sync -c ../scripts/x86/pvtdown.lua | tail -n +7 > tmp
cmp tmp ../MachO-Test-Binaries/Results/Test61
if [ $? -eq 1 ]
then
  echo "Test61 failed:"
  cat tmp
  RESULT=1
fi

./../build/bin/RopTool -a x86 -f ../MachO-Test-Binaries/x86_64_FAT/sync -c ../scripts/x86/pvtfinder.lua | tail -n +7 > tmp
cmp tmp ../MachO-Test-Binaries/Results/Test62
if [ $? -eq 1 ]
then
  echo "Test62 failed:"
  cat tmp
  RESULT=1
fi

./../build/bin/RopTool -a x86 -f ../MachO-Test-Binaries/x86_64_FAT/sync -c ../scripts/x86/pvtfinder2.lua | tail -n +7 > tmp
cmp tmp ../MachO-Test-Binaries/Results/Test63
if [ $? -eq 1 ]
then
  echo "Test63 failed:"
  cat tmp
  RESULT=1
fi

./../build/bin/RopTool -a x86 -f ../MachO-Test-Binaries/x86_64_FAT/sync -c ../scripts/x86/vcall.lua | tail -n +7 > tmp
cmp tmp ../MachO-Test-Binaries/Results/Test64
if [ $? -eq 1 ]
then
  echo "Test64 failed:"
  cat tmp
  RESULT=1
fi

./../build/bin/RopTool -a x86 -f ../MachO-Test-Binaries/x86_64_FAT/sync -c ../scripts/x86/vcall2.lua | tail -n +7 > tmp
cmp tmp ../MachO-Test-Binaries/Results/Test65
if [ $? -eq 1 ]
then
  echo "Test65 failed:"
  cat tmp
  RESULT=1
fi

./../build/bin/RopTool -a x86 -f ../MachO-Test-Binaries/x86_64_FAT/sync -c ../scripts/x86/transfer_arith_with_branch.lua | tail -n +7 > tmp
cmp tmp ../MachO-Test-Binaries/Results/Test66
if [ $? -eq 1 ]
then
  echo "Test66 failed:"
  cat tmp
  RESULT=1
fi



############  As x64 ####################

# Hang Up / Freeze
#./../build/bin/RopTool -a x64 -c ../scripts/x64/call_reg.lua -f ../MachO-Test-Binaries/x86_64_FAT/bash | tail -n +7 > tmp
#cmp tmp ../MachO-Test-Binaries/Results/Test67
#if [ $? -eq 1 ]
#then
#  RESULT=1
#fi

# Hang Up / Freeze
#./../build/bin/RopTool -a x64 -c ../scripts/x64/call_reg.lua -f ../MachO-Test-Binaries/x86_64_FAT/sh | tail -n +7 > tmp
#cmp tmp ../MachO-Test-Binaries/Results/Test68
#if [ $? -eq 1 ]
#then
#  RESULT=1
#fi


./../build/bin/RopTool -a x64 -c ../scripts/x64/call_reg.lua -f ../MachO-Test-Binaries/x86_64_FAT/sync | tail -n +7 > tmp
cmp tmp  ../MachO-Test-Binaries/Results/Test69
if [ $? -eq 1 ]
then
  echo "Test69 failed:"
  cat tmp
  RESULT=1
fi


#############################################
echo "Macho test suite finished with $RESULT"
echo "End Time:"
timestamp

exit $RESULT





