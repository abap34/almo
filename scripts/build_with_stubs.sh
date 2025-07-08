#!/bin/bash
# Script to generate Python stub files for the almo module
# This script builds the extension and then generates stub files

set -e

echo "Building almo extension..."
python3 setup.py build_ext --inplace

echo "Generating stub files..."
python3 scripts/generate_stubs.py

echo "Stub generation completed!"