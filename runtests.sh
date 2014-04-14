#!/bin/sh

cd `dirname $0`

export BASENAME=${PWD##*/}
export SCRIPTDIR=$PWD
export BUILDDIR=$PWD/build

/bin/sh $SCRIPTDIR/build.sh debug
$BUILDDIR/bin/$BASENAME"tests" "$@"
