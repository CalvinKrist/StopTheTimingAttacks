#!/bin/bash

cd security/sha
make
cd ../../
for i in $(seq 1 1 1); do
	../build/X86/gem5.opt ../configs/example/se.py -c security/sha/sha --options="security/sha/our_input.asc" --caches --cpu-type=O3_X86_skylake_1
	../build/X86/gem5.opt ../configs/example/se.py -c security/sha/sha --options="security/sha/our_input.asc" --caches --cpu-type=O3_X86_skylake_1 --useSecLevels
done

cd security/rijndael
make
cd ../../
for i in $(seq 1 1 1); do\
	rm security/rijndael/output*
	../build/X86/gem5.opt ../configs/example/se.py -c security/rijndael/rijndael --caches --cpu-type=O3_X86_skylake_1 --useSecLevels
done

cd security/pgp/src
make linux
cd ../../../
#for i in $(seq 1 1 1); do\
#	../build/X86/gem5.opt ../configs/example/se.py -c security/pgp/src/pgp --caches --cpu-type=O3_X86_skylake_1 --useSecLevels
#done

cd security/blowfish
make
cd ../../
for i in $(seq 1 1 1); do\
	../build/X86/gem5.opt ../configs/example/se.py -c security/blowfish/bftest --caches --cpu-type=O3_X86_skylake_1 --useSecLevels
done
