#!/bin/bash
set -xe
DIR=`dirname ${BASH_SOURCE[0]}`
ROOTDIR=`realpath ${DIR}/../../`
COMMIT_HASH=`git rev-parse HEAD`

cd "$ROOTDIR/client_ui/web/config"
envsubst < config.json.tmpl > ${DEPLOYMENT_ENVIRONMENT}.json


cd "$ROOTDIR/client_ui"
TAG=registry.gitlab.com/squadov/squadovapiserver/${DEPLOYMENT_ENVIRONMENT}/squadov_webapp:${COMMIT_HASH}
docker build . --file Dockerfile.web --tag ${TAG} --build-arg DEPLOYMENT_ENVIRONMENT=${DEPLOYMENT_ENVIRONMENT}
docker push ${TAG}