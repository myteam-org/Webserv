#!/usr/bin/env python3
import requests
import time
import os

URL = "http://127.0.0.101:8081/posttest.py"  # テスト対象の CGI URL
SIZES_MB = [5, 10, 20, 50]  # 段階的に送信するサイズ（MB単位）

def generate_data(size_mb):
    """指定したサイズ（MB）の擬似データを生成"""
    return os.urandom(size_mb * 1024 * 10)

def test_post(size_mb):
    data = generate_data(size_mb)
    headers = {"Content-Type": "application/octet-stream"}

    print(f"\n=== {size_mb}MB POST Test ===")
    print(f"Sending {len(data)} bytes...")

    start = time.time()
    try:
        res = requests.post(URL, headers=headers, data=data, timeout=60)
        elapsed = time.time() - start

        print(f"Status: {res.status_code}")
        print(f"Elapsed: {elapsed:.2f} sec")
        print(f"Response length: {len(res.content)} bytes")
        print("Body preview:", res.text[:200].replace("\n", "\\n"))
    except requests.exceptions.RequestException as e:
        elapsed = time.time() - start
        print(f"Error after {elapsed:.2f} sec: {e}")

def main():
    print("Starting large POST tests against:", URL)
    for size in SIZES_MB:
        test_post(size)

if __name__ == "__main__":
    main()
