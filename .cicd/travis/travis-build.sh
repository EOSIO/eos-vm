#!/usr/bin/env bash
set -eo pipefail
. ../helpers/general.sh
. ./$HELPERS_DIR/execute.sh
. ./$HELPERS_DIR/docker-hash.sh
cd $ROOT_DIR
if [[ "$(uname)" == Darwin ]]; then
	mkdir build
	cd build
	execute ccache -s
	execute cmake ..
	execute make -j$JOBS
	if ${ENABLE_TESTS:-true}; then execute ctest -j$JOBS -V --output-on-failure -T Test; fi
else # linux
	execute docker run --rm -v $(pwd):/workdir -v /usr/lib/ccache -v $HOME/.ccache:/opt/.ccache -e JOBS -e CCACHE_DIR=/opt/.ccache $FULL_TAG
fi