#!/bin/bash
set -e

[ "$JOB_NAME" ]
[ "$BUILD_NUMBER" ]

umask 002
mountpoint /devt; echo "Devt is mounted: good"

[ -d distribution ] || { echo "Error: distribution directory not found."; exit 1; }

BASE_VERSION=`cat base-version.txt`

VERSION="${BASE_VERSION%%.0}.`git rev-list HEAD --count`"  # "%%.0" strips trailing ".0"
if [ "${JOB_NAME:0:6}" = "Github" ]; then
  JOB_NAME=${JOB_NAME#*/}
fi

mkdir -p /devt/builds/$JOB_NAME/$BUILD_NUMBER
cp -r distribution/* /devt/builds/$JOB_NAME/$BUILD_NUMBER
rm -f /devt/builds/$JOB_NAME/latest
ln -s $BUILD_NUMBER /devt/builds/$JOB_NAME/latest

# For each directory X found in /devt/builds/$JOB_NAME/latest, create a zip file
for dir in /devt/builds/$JOB_NAME/latest/*; do
  if [ -d "$dir" ]; then
    dir_name=$(basename "$dir")
    
    if [ "$dir_name" = "mac" ]; then
      # Special handling for mac directory - create separate zips for each architecture
      pushd /devt/builds/$JOB_NAME/latest/mac >/dev/null
      for arch in arm64 x64; do
        if [ -d "$arch" ]; then
          zip_file="/devt/builds/$JOB_NAME/$BUILD_NUMBER/dyalog-kafka.mac-${arch}-${VERSION}.zip"
          echo "Creating zip file: $zip_file"
          zip -r "$zip_file" "$arch" || true
        fi
      done
      popd >/dev/null
    else
      zip_file="/devt/builds/$JOB_NAME/$BUILD_NUMBER/dyalog-kafka.${dir_name}.${VERSION}.zip"
      echo "Creating zip file: $zip_file"
      
      pushd /devt/builds/$JOB_NAME/latest >/dev/null
      zip -r "$zip_file" "$dir_name" || true
      popd >/dev/null
    fi
  fi
done

# Tidy up old builds
r=/devt/builds/${JOB_NAME}
ls "$r" | grep -v "latest" | sort -n | head -n-10 | while read x; do
  echo "deleting $r/$x"
  rm -rf "$r/$x" || true # Continue even if deletion fails
done