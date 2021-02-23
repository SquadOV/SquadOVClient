#!/bin/bash
set -xe
DIR=`dirname ${BASH_SOURCE[0]}`
ROOTDIR=`realpath ${DIR}/../../`
GCP_PROJECT=$1
COMMIT_HASH=`git rev-parse HEAD`

cd "$ROOTDIR/client_ui/support"
TAG=registry.gitlab.com/squadov/squadovapiserver/${GCP_PROJECT}/squadov_support:${COMMIT_HASH}
docker build . --tag ${TAG}
docker push ${TAG}