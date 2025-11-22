#!/usr/bin/env bash
# ===============================================================
# update_fantasy_map_files.sh
#
# Pulls only selected files from the FantasyMapGenerator GitHub repo
# using Git sparse checkout in non-cone mode.
# ===============================================================

set -e

REPO_URL="git@github.com:rlguy/FantasyMapGenerator.git"
TARGET_DIR="external/fantasy_map_generator"
BRANCH="master"

# List of files to pull (absolute paths relative to repo root)
FILES=(
  "/src/dcel.h"
  "/src/dcel.cpp"
  "/src/mapgenerator.h"
  "/src/mapgenerator.cpp"
)

echo "=== Updating FantasyMapGenerator selective files ==="

# Ensure target directory exists
mkdir -p "$(dirname "$TARGET_DIR")"

# If directory already exists, remove it to avoid conflicts
if [ -d "$TARGET_DIR" ]; then
  echo "Removing existing checkout..."
  rm -rf "$TARGET_DIR"
fi

# Clone repo without checking out files
git clone --no-checkout "$REPO_URL" "$TARGET_DIR"
cd "$TARGET_DIR"

# Enable sparse checkout in non-cone mode
git sparse-checkout init --no-cone

# Set file list
echo "Setting sparse-checkout file list..."
git sparse-checkout set "${FILES[@]}"

# Checkout selected files
git checkout "$BRANCH"

echo "=== Done! Checked out selected files: ==="
git sparse-checkout list
