#!/bin/bash
set -xe
GCP_PROJECT=$1

TAG=registry.gitlab.com/squadov/squadovapiserver/${GCP_PROJECT}/squadov_landing:latest
docker build . --tag ${TAG}
docker push ${TAG}