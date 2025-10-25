#!/bin/bash
# test_all.sh

ipaddr="127.0.0.101"
port=8081

# 共通オプション
CURL_OPTS="-v -s -w '%{http_code}\n'"

# テストケース一覧
tests=(
  "http://$ipaddr:$port/index.html"
  "http://$ipaddr:$port/hello.py"
  "http://$ipaddr:$port/notfound"
  "http://$ipaddr:$port/cgi-bin/test.py"
)

echo "Running GET curl tests against $ipaddr:$port"
echo "----------------------------------------"

for url in "${tests[@]}"; do
  echo "Testing: $url"
  code=$(curl $CURL_OPTS "$url")
  echo "→ HTTP Status: $code"
  echo
done
#heavy case/ CGI POST/ siege もできれば追加する