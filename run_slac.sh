#!/usr/bin/env bash
set -eu

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )"
BUILD_DIR=${SCRIPT_DIR}/build
DATASET_DIR=${SCRIPT_DIR}/examples/python/reconstruction_system/dataset/redwood_simulated/livingroom1-simulated

popd ${BUILD_DIR}

make -j SLAC
make -j SLACIntegrate

echo "Running: SLAC"
./bin/examples/SLAC \
    ${DATASET_DIR} \
    --device CUDA:0 \
    --voxel_size 0.05 \
    --method slac \
    --weight 1 \
    --distance_threshold 0.07 \
    --iterations 5
echo "Done: SLAC"

echo "Running: SLACIntegrate"
./bin/examples/SLACIntegrate \
    ${DATASET_DIR} \
    ${DATASET_DIR}/slac/0.050 \
    --device CUDA:0 \
    --mesh \
    --block_count 80000 \
    --color_subfolder image
echo "Done: SLACIntegrate"

popd