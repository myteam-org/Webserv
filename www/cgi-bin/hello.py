# /var/www/cgi-bin/hello.py (実行権限 +x)
#!/usr/bin/env python3
print("Status: 200 OK")
print("Content-Type: text/plain; charset=utf-8")
print()
print("hello from cgi")
