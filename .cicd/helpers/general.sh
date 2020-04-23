export ROOT_DIR=$( dirname "${BASH_SOURCE[0]}" )/../..
export CICD_DIR=$ROOT_DIR/.cicd
export JOBS=${JOBS:-"$(getconf _NPROCESSORS_ONLN)"}