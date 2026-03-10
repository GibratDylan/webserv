#!/bin/bash

URL="http://127.0.0.1:8080/upload"
TOTAL=1000
PARALLEL=50
dd if=/dev/zero of=bigfile.bin bs=1k count=100

seq $TOTAL | xargs -n1 -P$PARALLEL -I{} \
curl -s -o /dev/null -X POST --data-binary @bigfile.bin $URL

echo "Upload stress test done"