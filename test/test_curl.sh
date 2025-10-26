#!/bin/bash

ipaddr="127.0.0.101"
port=8081
siegetestsec=30

CURL_OPTS="-v -w '%{http_code}\n'"

dd if=/dev/zero of=bigfile_100M.bin bs=1M count=100
dd if=/dev/zero of=bigfile_1M.bin bs=1M count=1
dd if=/dev/zero of=middlefile_512K.bin bs=512K count=1
dd if=/dev/zero of=middlefile_1000000b.bin bs=999785 count=1

# GET テスト
tests=(
  "http://$ipaddr:$port/index.html"
  "http://$ipaddr:$port/hello.py"
  "http://$ipaddr:$port/notfound"
)

echo "Running GET tests..."
for url in "${tests[@]}"; do
  echo "Testing: $url"
  code=$(curl $CURL_OPTS "$url" -s -o /dev/null)
  echo "→ HTTP Status: $code"
  echo
done

# POST テスト
post_files=(
  "middlefile_512K.bin"
  "middlefile_1000000b.bin"
  "bigfile_1M.bin"
  "bigfile_100M.bin"
)

for f in "${post_files[@]}"; do
  echo "Testing upload: $f"
  code=$(curl $CURL_OPTS -X POST -F "file=@$f" "http://$ipaddr:$port/upload" -s -o /dev/null)
  echo "→ HTTP Status: $code"
  echo
done


# Siege テスト
# echo "Siege test will start. This takes 30sec..."
# siege -c 10 -t $siegetestsec"s" "http://$ipaddr:$port/"