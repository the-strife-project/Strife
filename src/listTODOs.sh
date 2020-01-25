#!/bin/bash

cd src
find . -type f -exec grep -Hn 'TODO' {} \;
#find . -type f -exec grep -Hn 'DEBUG' {} \;
cd ..
