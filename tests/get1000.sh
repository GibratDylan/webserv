#!/bin/bash

URL="http://127.0.0.1:8080/"
EXPECTED=200
TOTAL=1000
PARALLEL=100

seq $TOTAL | xargs -P$PARALLEL -I{} bash -c '
code=$(curl -s -o /dev/null -w "%{http_code}" '"$URL"')
if [ "$code" != "'"$EXPECTED"'" ]; then
    echo "ERROR: got $code"
fi
'

echo "GET test finished with $TOTAL requests and $PARALLEL parallel connections"