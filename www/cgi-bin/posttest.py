#!/usr/bin/env python3
import sys
import os

print("Status: 200 OK")
print("Content-Type: text/plain; charset=utf-8")
print()

# 環境変数の確認
print("REQUEST_METHOD:", os.environ.get("REQUEST_METHOD"))
print("CONTENT_TYPE:", os.environ.get("CONTENT_TYPE"))
print("CONTENT_LENGTH:", os.environ.get("CONTENT_LENGTH"))

# 標準入力（ボディ）を読み取る
body = sys.stdin.read()
print("BODY:", body)