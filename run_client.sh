#!/bin/bash

pushd bin
if test -f ./SphereDefender.exe; then
    ./SphereDefender.exe
elif test -f ./SphereDefender; then
    ./SphereDefender
fi
popd
