#!/bin/bash
set -e
echo "Installing dependencies..."
sudo apt update
sudo apt install -y libopencv-dev cmake gcc g++ make
echo "Done."
