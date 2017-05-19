#!/bin/bash

NAME=keydrown
VERSION=0.1

make clean

cat >dkms.conf << EOL
PACKAGE_NAME="$NAME"
PACKAGE_VERSION="$VERSION"
CLEAN="make clean"
MAKE[0]="make all"
BUILT_MODULE_NAME[0]="keydrown"
DEST_MODULE_LOCATION[0]="/updates"
AUTOINSTALL="yes"
EOL

DEST=/usr/src/$NAME-$VERSION/
mkdir -p $DEST
cp Makefile $DEST
cp dkms.conf $DEST
cp -r source $DEST
cp -r include $DEST
cp keydrown.h $DEST
cp main.c $DEST

dkms build -m $NAME -v $VERSION
dkms mkdeb -m $NAME -v $VERSION
cp /var/lib/dkms/$NAME/$VERSION/deb/*.deb .
dkms remove -m $NAME -v $VERSION --all
