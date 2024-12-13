#!/bin/sh
#
# This script automates the process of creating GitHub releases and uploading
# assets for the Dyalog Kafka project. It performs the following actions:
#
# 1. Checks for required environment variables (JOB_NAME, BUILD_NUMBER, GHTOKEN).
# 2. Copies build artifacts from the 'distribution' directory to a dedicated
#    build directory based on the job and build number.
# 3. Creates a symbolic link 'latest' pointing to the latest build.
# 4. Extracts a version tag from the JOB_NAME (e.g., v1.2.3).
# 5. If a tag is found:
#    - Checks if a GitHub release with that tag already exists.
#    - If not, creates a new GitHub release with the tag, name, and description.
#    - Uploads all files in the 'latest' build directory as assets to the release.
# 6. Cleans up old build directories, keeping only the 10 most recent builds
#    (excluding the 'latest' symlink).
#
# It uses the GitHub API to interact with releases and assets.

set -e

[ "$JOB_NAME" ]
[ "$BUILD_NUMBER" ]

if ! [ "$GHTOKEN" ]; then
  echo 'Please put your GitHub API Token in an environment variable named GHTOKEN'
  exit 1
fi

[ -d distribution ] || { echo "Error: distribution directory not found."; exit 1; }

REPO_NAME="Dyalog/kafka"

mkdir -p /devt/builds/$JOB_NAME/$BUILD_NUMBER
cp -r distribution/* /devt/builds/$JOB_NAME/$BUILD_NUMBER
rm -f /devt/builds/$JOB_NAME/latest
ln -s $BUILD_NUMBER /devt/builds/$JOB_NAME/latest

# Extract tag if present (matches pattern like foo/v12.0.2)
VERSION=$(echo "$JOB_NAME" | grep -oE 'v[0-9]+\.[0-9]+\.[0-9]+$' || true)

if [ -n "$VERSION" ]; then
  echo "Tag $VERSION detected; creating new GitHub release..."

  # Check if the release already exists
  EXISTING_RELEASE=$(curl -s -H "Authorization: Bearer $GHTOKEN" \
    "https://api.github.com/repos/$REPO_NAME/releases/tags/${VERSION}")

  if echo "$EXISTING_RELEASE" | grep -q '"id":'; then
    echo "Release ${VERSION} already exists. Skipping creation."
    exit 0
  fi

  # Create the release
  RESPONSE=$(curl -L \
    -X POST \
    -H "Accept: application/vnd.github+json" \
    -H "Authorization: Bearer $GHTOKEN" \
    -H "X-GitHub-Api-Version: 2022-11-28" \
    "https://api.github.com/repos/$REPO_NAME/releases" \
    -d "{
        \"tag_name\": \"${VERSION}\",
        \"name\": \"Release ${VERSION}\",
        \"body\": \"Release of version ${VERSION}\",
        \"draft\": false,
        \"prerelease\": false
    }")

  # Extract release ID
  RELEASE_ID=$(echo "$RESPONSE" | grep -o '"id": [0-9]\+' | head -1 | cut -d' ' -f2)

  if [ -z "$RELEASE_ID" ]; then
    echo "Failed to create release: $RESPONSE"
    exit 1
  fi

  echo "Created release with ID: $RELEASE_ID"

  # Upload each file in the latest build directory as an asset
  cd /devt/builds/$JOB_NAME/latest
  for FILE in *; do
    if [ -f "$FILE" ]; then
      echo "Uploading $FILE..."
      curl -L \
        -X POST \
        -H "Accept: application/vnd.github+json" \
        -H "Authorization: Bearer $GHTOKEN" \
        -H "X-GitHub-Api-Version: 2022-11-28" \
        -H "Content-Type: application/octet-stream" \
        "https://uploads.github.com/repos/$REPO_NAME/releases/$RELEASE_ID/assets?name=$(echo "$FILE" | sed 's/ /%20/g')" \
        --data-binary "@$FILE"
    fi
  done

  echo "Asset upload complete"
else
  echo "No tag detected in job name; skipping GitHub release"
fi

# Tidy up old builds
r=/devt/builds/${JOB_NAME#*/}
ls "$r" | grep -v "latest" | sort -n | head -n-10 | while read x; do
  echo "deleting $r/$x"
  rm -rf "$r/$x" || true # Continue even if deletion fails
done