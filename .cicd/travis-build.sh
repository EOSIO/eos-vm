#!/usr/bin/env bash
set -eo pipefail
. ./.cicd/docker-hash.sh

function execute()
{
  echo "$@"
  "$@"
}

echo "travis_repo_slug == $TRAVIS_REPO_SLUG"
echo "travis_pr_branch == $TRAVIS_PULL_REQUEST_BRANCH"
exit 0
# execute docker run --rm -v $(pwd):/workdir -v /usr/lib/ccache -v $HOME/.ccache:/opt/.ccache -e CCACHE_DIR=/opt/.ccache $FULL_TAG