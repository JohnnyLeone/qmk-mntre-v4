#!/bin/bash

sudo picotool load "${1:-$(dirname "$0")/build/mntre_v4_default.uf2}"
sleep 1
sudo picotool reboot

