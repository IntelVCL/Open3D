#!/bin/bash

. set_variables.sh

# build the image
./build.sh ${1} ${2} ${3}

TIMEZONE=$(cat /etc/timezone)

# run the container
docker container run \
    --rm \
    -d \
    -t \
    -e PYTHON=$PYTHON \
    -e TZ=$TIMEZONE \
    -h $CONTAINER_NAME \
    --name $CONTAINER_NAME \
    $IMAGE_NAME

# attach to the running container, clone & build Open3d
echo "testing $IMAGE_NAME..."
docker container exec -it -w /root $CONTAINER_NAME bash -c './test.sh'

# stop the container
docker container stop -t 0 $CONTAINER_NAME
