#!/bin/sh

# Download latest version of linkit-7697-peripheral-drivers-for-arduino as driver_pack.tgz
# This is for the build script to copy these files into library folder.
curl -s https://api.github.com/repos/MediaTek-Labs/linkit-7697-peripheral-drivers-for-arduino/releases/latest | grep "tarball_url" | cut -d ':' -f 2,3 |  tr -d '[ ",]' | wget -O driver_pack.tgz -i -