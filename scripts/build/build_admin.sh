#!/bin/bash
set -xe
DIR=`dirname ${BASH_SOURCE[0]}`
ROOTDIR=`realpath ${DIR}/../../`
GCP_PROJECT=$1
COMMIT_HASH=`git rev-parse HEAD`

cd "$ROOTDIR/client_ui/admin/config"
envsubst < config.json.tmpl > ${GCP_PROJECT}.json

cd "$ROOTDIR/client_ui/admin"
TAG=registry.gitlab.com/squadov/squadovapiserver/${GCP_PROJECT}/squadov_admin:${COMMIT_HASH}
docker build . --tag ${TAG}
docker push ${TAG}