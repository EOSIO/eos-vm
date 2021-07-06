#!/bin/bash
set -eou pipefail
VERSION=1

brew update && brew upgrade
brew install git cmake
