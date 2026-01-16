#!/bin/bash
# publish-release.sh

VERSION=$1
if [ -z "$VERSION" ]; then
    echo "Usage: $0 <version>"
    echo "Example: $0 v1.0.0"
    exit 1
fi

# 1. 清理并创建发布构建
echo "Building release version..."
rm -rf build_release
mkdir -p build_release
cd build_release

# 2. 编译
echo "Configuring with CMake..."
cmake -DCMAKE_BUILD_TYPE=Release ..
echo "Building..."
cmake --build . --config Release

# 3. 创建发布包
echo "Creating release package..."
mkdir -p ../release_package
cp snake.exe ../release_package/
cp -r ../assets ../release_package/ 2>/dev/null || true
cp ../README.md ../release_package/

# 4. 创建Release
echo "Creating GitHub release..."
cd ..
gh release create $VERSION \
    --title "Classic Snake Game $VERSION" \
    --notes-file <(echo "Release $VERSION of Classic Snake Game") \
    release_package/*

# 5. 清理
echo "Cleaning up..."
rm -rf release_package
echo "Release $VERSION published!"
