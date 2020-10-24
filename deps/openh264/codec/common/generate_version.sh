#!/bin/bash
SRC_PATH=$1

GIT_VERSION='"992c1c1"'

mkdir -p codec/common/inc
cat $SRC_PATH/codec/common/inc/version_gen.h.template | sed "s/\$FULL_VERSION/$GIT_VERSION/g" > codec/common/inc/version_gen.h.new
if cmp codec/common/inc/version_gen.h.new codec/common/inc/version_gen.h > /dev/null 2>&1; then
    # Identical to old version, don't touch it (to avoid unnecessary rebuilds)
    rm codec/common/inc/version_gen.h.new
    echo "Keeping existing codec/common/inc/version_gen.h"
    exit 0
fi
mv codec/common/inc/version_gen.h.new codec/common/inc/version_gen.h

echo "Generated codec/common/inc/version_gen.h"
