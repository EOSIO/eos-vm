#!/usr/bin/env bash
set -eo pipefail
cd $( dirname "${BASH_SOURCE[0]}" )/.. # Ensure we're in the repo root and not inside of scripts
. ./.cicd/.helpers

[[ -z $DETERMINED_HASH ]] && echo "DETERMINED_HASH empty! (check determine-hash function)" && exit 1
DOCKER_CLI_EXPERIMENTAL=enabled # needed for docker manifest inspect
if docker manifest inspect $FULL_TAG; then
    echo "$FULL_TAG already exists"
else
    generate_docker_image
fi