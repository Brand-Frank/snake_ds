# 贪吃蛇游戏 (Snake Game)

![Release Build](https://github.com/Brand-Frank/snake_ds/actions/workflows/release.yml/badge.svg)

一个使用C语言和SDL2开发的经典贪吃蛇游戏，支持Windows平台。

## 功能特点

- 经典的贪吃蛇游戏玩法
- 美观的图形界面
- 分数系统和最高分记录
- 随分数增加游戏速度
- 开始界面、暂停功能和游戏结束界面
- 支持键盘方向键和WASD控制

## 开发环境

### 必需环境
- MSYS2 UCRT环境
- CMake (>= 3.10)
- SDL2 库
- SDL2_ttf 库

### 安装依赖 (MSYS2)

```bash
# 更新包数据库
pacman -Syu

# 安装编译工具
# pacman -S --needed base-devel mingw-w64-ucrt-x86_64-toolchain
pacman -S --needed base-devel mingw-w64-ucrt-x86_64-gcc

# 安装SDL2和相关库
pacman -S mingw-w64-ucrt-x86_64-SDL2 mingw-w64-ucrt-x86_64-SDL2_ttf

# 安装CMake
pacman -S mingw-w64-ucrt-x86_64-cmake
```

## 编译和运行
```bash
# 创建构建目录并进入
mkdir build && cd build

# 使用CMake生成构建文件
cmake .. -G "MinGW Makefiles"

# 编译项目
cmake --build .

# 运行游戏
./snake_game
```

## 提交代码
```bash
git add .
git commit -m "feat: initial commit"

git tag -a v0.0.6 -m "Repair bug of workflow"

git push
git push origin --tags
```
