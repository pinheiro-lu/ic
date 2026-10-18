#!/bin/bash
# 🎄 Kaggle Santa 2025: Christmas Tree Packing Visualizer Launcher

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$SCRIPT_DIR"

echo "=========================================================="
echo "🎄 Kaggle Santa 2025: Christmas Tree Packing Visualizer"
echo "=========================================================="

if ! command -v python3 &> /dev/null; then
    echo "Error: Python 3 is required but not found."
    exit 1
fi

PORT="${1:-8000}"

echo "Starting Web Visualizer on http://localhost:$PORT ..."
python3 visualizer.py --serve --port "$PORT"
