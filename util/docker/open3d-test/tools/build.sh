#!/bin/bash

. set_variables.sh

# build the images this image depends on
if [ "$3" != "" ]; then
    ./build.sh ${1} ${2}
fi
if [ "$2" = "${bundle_type[1]}" ]; then
    ./build.sh ${1} ${bundle_type[0]} ${3}
fi

# download miniconda installer once
if [ "$3" = "mc2" ]; then
    if [ ! -f ../setup/$MC2_INSTALLER ]; then
        wget -P ../setup https://repo.anaconda.com/miniconda/$MC2_INSTALLER
    fi
elif [ "$3" = "mc3" ]; then
    if [ ! -f ../setup/$MC3_INSTALLER ]; then
        wget -P ../setup https://repo.anaconda.com/miniconda/$MC3_INSTALLER
    fi
fi

# build the image
echo "building $IMAGE_NAME..."
date
docker image build -t $IMAGE_NAME -f ../Dockerfiles/${1}/$DOCKERFILE ..
date
echo "done building $IMAGE_NAME..."
echo
