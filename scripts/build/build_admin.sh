#!/bin/bash
set -xe
DIR=`dirname ${BASH_SOURCE[0]}`
ROOTDIR=`realpath ${DIR}/../../`
COMMIT_HASH=`git rev-parse HEAD`

cd "$ROOTDIR/client_ui/admin/config"
envsubst < config.json.tmpl > ${DEPLOYMENT_ENVIRONMENT}.json

cd "$ROOTDIR/client_ui/admin"
TAG=registry.gitlab.com/squadov/squadovapiserver/${DEPLOYMENT_ENVIRONMENT}/squadov_admin:${COMMIT_HASH}
docker build . --tag ${TAG} --build-arg DEPLOYMENT_ENVIRONMENT=${DEPLOYMENT_ENVIRONMENT}
docker push ${TAG}