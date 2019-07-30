#!/usr/bin/env bash
set -eo pipefail
. ./.cicd/docker-hash.sh

function execute()
{
  echo "$@"
  "$@"
}

if [[ $TRAVIS_REPO_SLUG == $ORG ]]; then
  echo "travis_repo_slug == $TRAVIS_REPO_SLUG"
  echo "org == $ORG"
  echo "Internal build, please see Buildkite."
  exit 0
else
  execute docker run --rm -v $(pwd):/workdir -v /usr/lib/ccache -v $HOME/.ccache:/opt/.ccache -e CCACHE_DIR=/opt/.ccache $FULL_TAG
fi