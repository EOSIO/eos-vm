#!/usr/bin/env bash
set -eo pipefail
. ./.cicd/docker-hash.sh

function execute()
{
  echo "$@"
  "$@"
}

execute docker run --rm -v $(pwd):/workdir -v /usr/lib/ccache -v $HOME/.ccache:/opt/.ccache -e CCACHE_DIR=/opt/.ccache $FULL_TAG