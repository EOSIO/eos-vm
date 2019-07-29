#!/usr/bin/env bash
set -eo pipefail
cd $( dirname "${BASH_SOURCE[0]}" )/.. # Ensure we're in the repo root and not inside of scripts
. ./.cicd/helpers.sh
echo "Looking for $FULL_TAG"
if [[ docker_tag_exists $FULL_TAG ]]; then
    echo "$FULL_TAG already exists"
else # if we cannot pull the image, we build and push it first
    docker login -u $DOCKERHUB_USERNAME -p $DOCKERHUB_PASSWORD
    cd ./.cicd
    docker build -t $FULL_TAG -f ./${IMAGE_TAG}.dockerfile .
    docker push $FULL_TAG
    cd ..
fi