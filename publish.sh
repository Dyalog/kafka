#!/bin/sh
set -e

[ "$JOB_NAME" ]
[ "$BUILD_NUMBER" ]

mkdir -p /devt/builds/$JOB_NAME/$BUILD_NUMBER
cp -r distribution/* /devt/builds/$JOB_NAME/$BUILD_NUMBER
rm -f /devt/builds/$JOB_NAME/latest
ln -s $BUILD_NUMBER /devt/builds/$JOB_NAME/latest
