#!/bin/sh

IMAGE_NAME="uvk5"

docker build -t $IMAGE_NAME .
docker run --rm -v "${PWD}/compiled-firmware:/app/compiled-firmware" $IMAGE_NAME /bin/bash -c "cd /app && make && cp f4hwn* PBA_packed_.bin compiled-firmware/"