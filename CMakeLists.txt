cmake_minimum_required(VERSION 3.10)
project(SnakeGame)

set(CMAKE_C_STANDARD 11)

# 设置MSYS2路径
set(MSYS2_PATH "C:/msys64/ucrt64")

# 设置包含目录
include_directories(
    ${MSYS2_PATH}/include/SDL2
)

# 设置库路径
link_directories(
    ${MSYS2_PATH}/lib
)

# 添加可执行文件
add_executable(snake_game main.c)

# 链接库 - 注意顺序很重要！
target_link_libraries(snake_game
    mingw32      # MinGW运行时库
    SDL2main     # SDL2的主库，必须在SDL2之前
    SDL2         # SDL2库
    SDL2_ttf     # SDL2_ttf库
)

# 添加Windows GUI标志（去掉控制台窗口）
set_target_properties(snake_game PROPERTIES
    LINK_FLAGS "-mwindows"
)

# 构建后自动复制DLL文件
add_custom_command(TARGET snake_game POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${MSYS2_PATH}/bin/SDL2.dll"
        "${CMAKE_CURRENT_BINARY_DIR}"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${MSYS2_PATH}/bin/SDL2_ttf.dll"
        "${CMAKE_CURRENT_BINARY_DIR}"
    COMMENT "Copying required DLL files..."
)
