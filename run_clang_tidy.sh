#!/bin/bash
cd "/home/yosshii/Desktop/Webserv"
/usr/bin/git ls-files '*.cpp' '*.hpp' | grep -vE '(^test/|^build/)' | xargs /usr/bin/clang-tidy -p build
