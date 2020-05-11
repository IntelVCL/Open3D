#!/usr/bin/env bash
set -e

# References:
# https://gist.github.com/mderazon/5a5b50d92f4c4adaf44d5f49dd95d0ef
# https://github.com/GoogleCloudPlatform/google-cloud-eclipse/blob/master/.travis.yml

curr_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")" >/dev/null 2>&1 && pwd)"
echo "Running deploy_docs.sh..."

# Install gsutil
path_to_gcloud=$(which gcloud)
if [ -x "$path_to_gcloud" ]; then
    echo "gcloud found: $path_to_gcloud"
else
    rm -rf $HOME/google-cloud-sdk
    export CLOUDSDK_CORE_DISABLE_PROMPTS=1
    curl https://sdk.cloud.google.com | bash
    source $HOME/google-cloud-sdk/path.bash.inc
fi
gcloud --quiet version

if [ "$TRAVIS" = true ]; then
    # Decrypt key
    openssl aes-256-cbc \
        -K $encrypted_72034960ad12_key \
        -iv $encrypted_72034960ad12_iv \
        -in ${curr_dir}/open3d-ci-sa-key.json.enc \
        -out ${curr_dir}/open3d-ci-sa-key.json \
        -d
    echo ${curr_dir}/open3d-ci-sa-key.json # Trying to print key from a forked repo, do not merge this to the main repo.

    # Autenticate with Google cloud
    gcloud auth activate-service-account --key-file ${curr_dir}/open3d-ci-sa-key.json
    gcloud config set project isl-buckets
fi

# Compress and upload the docs
commit_id="$(git rev-parse HEAD)"
docs_out_dir="${curr_dir}/../docs/_out" # Docs in ${docs_out_dir}/html
tar_file="${commit_id}.tar.gz"
rm -rf ${tar_file}
tar -C ${docs_out_dir} -czvf ${tar_file} html
gsutil cp ${tar_file} gs://open3d-docs/${tar_file}
echo "Download the docs at: https://storage.googleapis.com/open3d-docs/${tar_file}"
