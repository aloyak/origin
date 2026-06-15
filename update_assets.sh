#!/bin/bash
rm -rf "$(dirname "$0")/build/game/assets"
cp -r "$(dirname "$0")/assets" "$(dirname "$0")/build/game/assets"
echo "Assets updated!"