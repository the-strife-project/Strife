#!/bin/sh -xe

export PATH="$PWD/cross/bin:$PATH"

cd $GITHUB_WORKSPACE
make
