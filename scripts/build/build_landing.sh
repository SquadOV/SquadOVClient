#!/bin/bash
set -xe
DIR=`dirname ${BASH_SOURCE[0]}`
ROOTDIR=`realpath ${DIR}/../../`
GCP_PROJECT=$1

cd "$ROOTDIR/client_ui/landing"
TAG=registry.gitlab.com/squadov/squadovapiserver/${GCP_PROJECT}/squadov_landing:latest
docker build . --tag ${TAG}
docker push ${TAG}