#!/usr/bin/env bash
set -eo pipefail
cd $( dirname "${BASH_SOURCE[0]}" )/.. # Ensure we're in the repo root and not inside of scripts
. ./.cicd/.helpers

[[ -z $DETERMINED_HASH ]] && echo "DETERMINED_HASH empty! (check determine-hash function)" && exit 1
echo "Looking for $IMAGE_TAG-$DETERMINED_HASH"
if docker_tag_exists $FULL_TAG; then
    echo "$FULL_TAG already exists"
else
    generate_docker_image
fi