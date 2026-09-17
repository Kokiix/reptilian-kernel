#!/bin/bash
set -e  # Exit immediately if any command returns a non-zero status

echo "===> [1/3] Compiling kernel..."
sudo make -j$(nproc)

echo "===> [2/3] Installing modules..."
sudo make modules_install

echo "===> [3/3] Installing kernel image..."
sudo make install

echo "===> SUCCESS: Kernel built and installed with exit code 0!"