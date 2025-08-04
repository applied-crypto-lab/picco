#!/bin/bash

mkdir -p bin # making bin directory if does not exist
cd src/picco	
make clean # this isn't necessary to do every single time changes in src/picco are made
make
mv picco ../../bin/
cd ../web
make
mv picco-web ../../bin/
cd ../utility
make
mv picco-utility ../../bin/
