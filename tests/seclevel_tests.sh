#!/bin/bash

cd security/sha
make
cd ../../
for i in $(seq 1 1 1); do
	../build/X86/gem5.opt ../configs/example/se.py -c security/sha/sha --options="1 security/sha/our_input.asc" --caches --l2cache --cpu-type=O3_X86_skylake_1
	mv occupancy_data.csv occdat/sha_ums_nosec_occdat.csv
	../build/X86/gem5.opt ../configs/example/se.py -c security/sha/sha --options="1 security/sha/our_input.asc" --caches --l2cache --cpu-type=O3_X86_skylake_1 --useSecLevels
	mv occupancy_data.csv occdat/sha_ums_sec_occdat.csv
	#../build/X86/gem5.opt ../configs/example/se.py -c security/sha/sha --options="0 security/sha/our_input.asc" --caches --l2cache --cpu-type=O3_X86_skylake_1 --useSecLevels
	#mv occupancy_data.csv occdat/sha_umd_sec_occdat.csv
done

cd security/rijndael
make
cd ../../
for i in $(seq 1 1 1); do\
	rm security/rijndael/output*
	../build/X86/gem5.opt ../configs/example/se.py -c security/rijndael/rijndael --options="1" --caches --l2cache --cpu-type=O3_X86_skylake_1
	mv occupancy_data.csv occdat/rijndael_ums_nosec_occdat.csv
	../build/X86/gem5.opt ../configs/example/se.py -c security/rijndael/rijndael --options="1" --caches --l2cache --cpu-type=O3_X86_skylake_1 --useSecLevels
	mv occupancy_data.csv occdat/rijndael_ums_sec_occdat.csv
	../build/X86/gem5.opt ../configs/example/se.py -c security/rijndael/rijndael --options="0" --caches --l2cache --cpu-type=O3_X86_skylake_1 --useSecLevels
	mv occupancy_data.csv occdat/rijndael_umd_sec_occdat.csv
done

# cd security/pgp/src
# make linux
# cd ../../../
# #for i in $(seq 1 1 1); do\
# #	../build/X86/gem5.opt ../configs/example/se.py -c security/pgp/src/pgp --caches --cpu-type=O3_X86_skylake_1 --useSecLevels
# #done

cd security/blowfish
make
cd ../../
for i in $(seq 1 1 1); do\
	../build/X86/gem5.opt ../configs/example/se.py -c security/blowfish/bftest --options="1" --caches --l2cache --cpu-type=O3_X86_skylake_1
	mv occupancy_data.csv occdat/bf_ums_nosec_occdat.csv
	../build/X86/gem5.opt ../configs/example/se.py -c security/blowfish/bftest --options="1" --caches --l2cache --cpu-type=O3_X86_skylake_1 --useSecLevels
	mv occupancy_data.csv occdat/bf_ums_sec_occdat.csv
	../build/X86/gem5.opt ../configs/example/se.py -c security/blowfish/bftest --options="0" --caches --l2cache --cpu-type=O3_X86_skylake_1 --useSecLevels
	mv occupancy_data.csv occdat/bf_umd_sec_occdat.csv
done
