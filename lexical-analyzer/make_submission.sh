#!/bin/bash

rm build/1905005 -r
mkdir build/1905005

cp src/ build/1905005/ -r
cp * build/1905005/

cd build
cd 1905005/
python3 rename.py
cd ..

rm 1905005.zip
7z a 1905005.zip 1905005/
