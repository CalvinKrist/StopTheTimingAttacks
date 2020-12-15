#!/bin/bash

cd security/sha
make
cd ../../
#for i in $(seq 1 1 1); do
#	../build/X86/gem5.opt ../configs/example/se.py -c security/sha/sha --caches --cpu-type=O3_X86_skylake_1 --useSecLevels
#done

cd security/rijndael
make
cd ../../
for i in $(seq 1 1 1); do\
	rm security/rijndael/output*
	../build/X86/gem5.opt ../configs/example/se.py -c security/rijndael/rijndael --caches --cpu-type=O3_X86_skylake_1
done