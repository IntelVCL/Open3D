#!/bin/bash

REAL_PATH=$(dirname $(realpath ${0}))

. ${REAL_PATH}/set_variables.sh

# download the image only if not found locally
if [ 1 -eq ${IMAGE_EXISTS} ]; then
    echo "downloading ${IMAGE_NAME}..."
    date
    docker pull ${IMAGE_NAME}
    echo
fi

# helps sync the container clock with the host clock
TIMEZONE=$(cat /etc/timezone)

echo "running the ${CONTAINER_HOSTNAME} container..."
date
docker container run \
    --rm \
    -d \
    -t \
    -e ENV_TYPE="${3}" \
    -e TZ="${TIMEZONE}" \
    -h ${CONTAINER_HOSTNAME} \
    --name ${CONTAINER_NAME} \
    ${IMAGE_NAME}

echo "attaching to the ${CONTAINER_HOSTNAME} container..."
date
docker container exec -it -w /root ${CONTAINER_NAME} bash -c 'bash'

echo "stopping the ${CONTAINER_HOSTNAME} container..."
docker container stop -t 0 ${CONTAINER_NAME} >/dev/null 2>&1
