#!/bin/bash

URL="http://127.0.0.1:8080/upload.txt"
EXPECTED=201
TOTAL=1000


echo "data=test" > body.txt

for i in $(seq 1 $TOTAL); do
  code=$(curl -s -o /dev/null -w "%{http_code}" -X POST -d @body.txt $URL)
  if [ "$code" != "$EXPECTED" ]; then
    echo "POST ERROR: $code"
  fi
done

echo "POST test finished with $TOTAL requests "
