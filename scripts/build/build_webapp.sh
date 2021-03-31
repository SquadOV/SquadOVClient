#!/bin/bash
set -xe
DIR=`dirname ${BASH_SOURCE[0]}`
ROOTDIR=`realpath ${DIR}/../../`
GCP_PROJECT=$1
COMMIT_HASH=`git rev-parse HEAD`

cd "$ROOTDIR/client_ui/web/config"
envsubst < config.json.tmpl > ${GCP_PROJECT}.json

cd "$ROOTDIR/client_ui"
TAG=registry.gitlab.com/squadov/squadovapiserver/${GCP_PROJECT}/squadov_webapp:${COMMIT_HASH}
docker build . --file Dockerfile.web --tag ${TAG} --build-arg GCP_PROJECT=${GCP_PROJECT}
docker push ${TAG}