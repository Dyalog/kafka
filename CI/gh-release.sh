#!/bin/bash
set -e 

GIT_BRANCH=${JOB_NAME#*/*/}
GIT_COMMIT=$(git rev-parse HEAD)

case $GIT_BRANCH in
    main|kafka[0-9]\.[0-9]|PR*)
        echo "creating ${GIT_BRANCH} release"
    ;;
    *)  
        echo "skipping creating release for ${GIT_BRANCH}"
        exit 0
    ;;
esac


# create JSON
TMP_JSON=/tmp/gh-publish.$$.json
GH_RELEASES=/tmp/gh-releases.$$.json
BASE_VERSION=`cat base-version.txt`
VERSION="${BASE_VERSION}.`git rev-list HEAD --count`"

if ! [ "$GHTOKEN" ]; then
    echo 'Please put your GitHub API Token in an environment variable named GHTOKEN'
    exit 1
fi

# Delete all the old draft releases, otherwise this gets filled up pretty fast as we create for every commit:
# but only if jq is available
if which jq >/dev/null 2>&1; then
    DRAFT=true
    C=0

    # Get the json from Github API
    curl -o $GH_RELEASES \
      --silent -H "Authorization: token $GHTOKEN" \
      https://api.github.com/repos/Dyalog/kafka/releases

    RELEASE_COUNT=`cat $GH_RELEASES | jq ". | length"`
    GH_VERSION_ND_LAST=0

    while [ $C -le $RELEASE_COUNT ] ; do
        DRAFT=`cat $GH_RELEASES | jq  ".[$C].draft"`
        ID=`cat $GH_RELEASES | jq  ".[$C].id"`
        GH_VERSION=$(cat $GH_RELEASES | jq ".[$C].name" | sed 's/"//g;s/^v//')
        GH_VERSION_ND=$(cat $GH_RELEASES | jq ".[$C].name" | sed 's/"//g;s/^v//;s/\.//g')
        GH_VERSION_AB=${GH_VERSION%.*}

        if [ "${GH_VERSION_AB}" = "${BASE_VERSION}" ]; then
            if [ "$DRAFT" = "true" ]; then
                echo -e -n "*** $(cat $GH_RELEASES | jq ".[$C].name" | sed 's/"//g') with id: $(cat $GH_RELEASES | jq  ".[$C].id") is a draft - Deleting.\n"
                curl -X "DELETE" -H "Authorization: token $GHTOKEN" https://api.github.com/repos/Dyalog/kafka/releases/${ID}
            else
                if [ $GH_VERSION_ND -gt $GH_VERSION_ND_LAST ]; then
                    COMMIT_SHA=`cat $GH_RELEASES | jq -r ".[$C].target_commitish"`
                    GH_VERSION_ND_LAST=$GH_VERSION_ND
                    PRERELEASE=`cat $GH_RELEASES | jq -r ".[$C].prerelease"`
                fi
            fi
        fi

        let C=$C+1
    done
    rm -f $GH_RELEASES

else
    echo jq not found, not removing draft releases
fi

echo "SHA: ${COMMIT_SHA}"

if [ $GH_VERSION_ND_LAST = 0 ]; then
    echo "No releases of $BASETVERSION found, not populating changelog"
    JSON_BODY=$(echo -e "Pre-Release of Dyalog-Kafka $BASE_VERSION\n\nWARNING: This is a pre-release version of Dyalog-Kafka $BASE_VERSION: it is possible that functionality may be added, removed or altered; we do not recommend using pre-release versions of Dyalog Kafka in production environment." | python -c 'import json,sys; print(json.dumps(sys.stdin.read()))')
    PRERELEASE=true
else
    echo using log from $COMMIT_SHA from $GH_VERSION_ND_LAST
    echo "Is Prerelease: ${PRERELEASE}"
    if [ "${PRERELEASE}" = "false" ]; then
        MSG_TEXT="Release Dyalog-Kafka ${BASE_VERSION}\n\n"
    else
        MSG_TEXT="Pre-Release of Dyalog-Kafka $BASE_VERSION\n\nWARNING: This is a pre-release version of Dyalog-Kafka $BASE_VERSION: it is possible that functionality may be added, removed or altered; we do not recommend using pre-release versions of Dyalog-Kafka in production environments.\n\n"
    fi
    JSON_BODY=$( ( echo -e "${MSG_TEXT}Changelog:"; git log --format='%s' ${COMMIT_SHA}.. ) | grep -v -i todo | python -c 'import json,sys; print(json.dumps(sys.stdin.read()))')
fi

cat >$TMP_JSON <<.
{
  "tag_name": "v$VERSION",
  "target_commitish": "${GIT_COMMIT}",
  "name": "v$VERSION",
  "body": $JSON_BODY,
  "draft": true,
  "prerelease": ${PRERELEASE}
}
.

cat $TMP_JSON

REPO=Dyalog/kafka # ideally this should be parsed from "git ls-remote --get-url origin"
TMP_RESPONSE=/tmp/gh-response.$$.json
curl -o $TMP_RESPONSE --data @$TMP_JSON -H "Authorization: token $GHTOKEN" -i https://api.github.com/repos/$REPO/releases

cat "$TMP_RESPONSE"

RELEASE_ID=`grep '"id"' $TMP_RESPONSE | head -1 | sed 's/.*: //;s/,//'`

echo "created release with id: $RELEASE_ID"

for F in /devt/builds/kafka/${GIT_BRANCH}/latest/*.zip; do
  # Check if the file exists and is not empty
  if [ -f "$F" ]; then
    echo "Uploading $F to Github"
    curl -o /dev/null -H "Authorization: token $GHTOKEN" \
      -H 'Accept: application/vnd.github.manifold-preview' \
      -H 'Content-Type: application/zip' \
      --data-binary @"$F" \
      https://uploads.github.com/repos/$REPO/releases/$RELEASE_ID/assets?name=$(basename "$F")
  fi
done

#rm -f $TMP_RESPONSE $TMP_JSON
