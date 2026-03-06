#!/usr/bin/env bash

echo "Downloading Sponza Model..."

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

git clone https://github.com/jimmiebergmann/Sponza.git "$SCRIPT_DIR/sponza"

echo "Download Complete!"