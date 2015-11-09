#!/bin/bash

killall -KILL qemu-system-i386
ME=`whoami`
echo $ME
IPCS_M=`ipcs -m | egrep "0x[0-9a-f]+ [0-9]+" | grep $ME | cut -f2 -d" "`
for id in $IPCS_M; do
  ipcrm shm $id;
done

