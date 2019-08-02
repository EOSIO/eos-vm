#!/usr/bin/env bash
set -eo pipefail
. ./.cicd/execute.sh
. ./.cicd/docker-hash.sh
if [[ "$(uname)" == Darwin ]]; then
	mkdir build
	cd build
	execute ccache -s
	execute cmake ..
	execute make -j $(nproc)
else # linux
	execute docker run --rm -v $(pwd):/workdir -v /usr/lib/ccache -v $HOME/.ccache:/opt/.ccache -e JOBS -e CCACHE_DIR=/opt/.ccache $FULL_TAG
fi