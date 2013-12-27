#!/bin/sh
#tests=`find testcase -name "*.dml"`
tests=`ls testcase/*.dml`

for f in $tests; do
	./dmlc_systemc $f >/dev/null
	if [ $? -ne 0 ]; then
		echo "[systemc] error have happend on " $f
		exit
	fi
	./dmlc_qemu $f >/dev/null
	if [ $? -ne 0 ]; then
		echo "[qemu] error have happend on " $f
		exit
	fi
done
