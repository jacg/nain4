#!/usr/bin/env sh

# NOTE: this must be sourced, not executed, for the `cd` to persist. Which
# probably precludes it from being a just recipe.

REPO_DIR= # TODO find location of this script for use as repo location
BRANCH=$1
TARGET_DIR=/tmp/n4-client-test/$BRANCH
echo $BRANCH
rm   -rf $TARGET_DIR
mkdir -p $TARGET_DIR
# TODO check that branch is checked out in repo
# TODO if so, patch client output flake to point at branch

nix run ${REPO_DIR}?branch=${BRANCH}
