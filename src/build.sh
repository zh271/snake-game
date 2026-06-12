#!/bin/bash
# Snake Battle — Build Script
# Uses MinGW from C:\Users\lenovo\mingw64\mingw64

export PATH="/c/Users/lenovo/mingw64/mingw64/bin:$PATH"

RAYLIB_SRC="/c/Users/lenovo/raylib-6.0/src"
RAYLIB_LIB="$RAYLIB_SRC/libraylib.a"

echo "=== Building Snake Battle ==="
echo "Compiler: $(g++ --version | head -1)"

# If raylib not built yet, build it
if [ ! -f "$RAYLIB_LIB" ]; then
    echo "Building raylib first..."
    cd "$RAYLIB_SRC"
    mingw32-make PLATFORM=PLATFORM_DESKTOP_WIN32 \
      CFLAGS="-D_WIN32_WINNT=0x0A00 -DNTDDI_VERSION=0x0A000000 -DPLATFORM_DESKTOP_WIN32 -DGRAPHICS_API_OPENGL_33 -Wno-missing-braces -Werror=pointer-arith -fno-strict-aliasing -std=c99" \
      -j4
    cd /c/Users/lenovo/snake
fi

g++ -std=c++17 \
  -I"$RAYLIB_SRC" \
  -D_WIN32_WINNT=0x0A00 \
  -o snake_battle.exe \
  main.cpp ai.cpp game_core.cpp map.cpp powerup.cpp input.cpp renderer.cpp \
  "$RAYLIB_LIB" \
  -lopengl32 -lgdi32 -lwinmm \
  -static-libgcc -static-libstdc++

if [ $? -eq 0 ]; then
    echo "=== Build SUCCESS ==="
    ls -lh snake_battle.exe
else
    echo "=== Build FAILED ==="
fi
