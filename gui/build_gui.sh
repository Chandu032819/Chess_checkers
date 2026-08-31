#!/usr/bin/env bash
# Builds the Dear ImGui GUI frontend (build/gui_play).
#
# Prerequisites (one-time, not handled by this script):
#   Linux (Debian/Ubuntu):  sudo apt install libglfw3-dev libgl1-mesa-dev pkg-config
#   macOS (Homebrew):       brew install glfw
#   Windows:                easiest via MSYS2/MinGW: pacman -S mingw-w64-x86_64-glfw
#
# Dear ImGui itself is vendored in gui/imgui/ (cloned from
# https://github.com/ocornut/imgui, tag v1.91.5) so there's nothing else
# to install for the UI toolkit itself.
set -e
cd "$(dirname "$0")/.."   # project root

IMGUI=gui/imgui
mkdir -p build/gui_obj

ENGINE_SOURCES="src/engine/Board.cpp src/engine/Command.cpp src/engine/Game.cpp \
                src/chess/ChessPieces.cpp src/chess/ChessGame.cpp \
                src/checkers/CheckersPieces.cpp src/checkers/CheckersGame.cpp"

IMGUI_SOURCES="$IMGUI/imgui.cpp $IMGUI/imgui_draw.cpp $IMGUI/imgui_tables.cpp \
               $IMGUI/imgui_widgets.cpp \
               $IMGUI/backends/imgui_impl_glfw.cpp $IMGUI/backends/imgui_impl_opengl3.cpp"

GLFW_CFLAGS=$(pkg-config --cflags glfw3)
GLFW_LIBS=$(pkg-config --libs glfw3)

INCLUDES="-Iinclude -Igui -I$IMGUI -I$IMGUI/backends $GLFW_CFLAGS"

echo "Compiling engine..."
for f in $ENGINE_SOURCES; do
    obj="build/gui_obj/$(basename "${f%.cpp}").o"
    g++ -std=c++17 -O2 $INCLUDES -c "$f" -o "$obj"
done

echo "Compiling Dear ImGui (first build only takes a bit)..."
for f in $IMGUI_SOURCES; do
    obj="build/gui_obj/$(basename "${f%.cpp}").o"
    if [ ! -f "$obj" ] || [ "$f" -nt "$obj" ]; then
        g++ -std=c++17 -O2 $INCLUDES -c "$f" -o "$obj"
    fi
done

echo "Compiling GUI frontend..."
g++ -std=c++17 -O2 $INCLUDES -c gui/gui_main.cpp -o build/gui_obj/gui_main.o

echo "Linking..."
LIBS="$GLFW_LIBS -ldl -lpthread"
if [[ "$OSTYPE" == "darwin"* ]]; then
    LIBS="$GLFW_LIBS -framework OpenGL -framework Cocoa -framework IOKit -framework CoreVideo"
elif [[ "$OSTYPE" == "linux-gnu"* ]]; then
    LIBS="$LIBS -lGL"
fi

g++ build/gui_obj/*.o -o build/gui_play $LIBS

echo ""
echo "Build complete: ./build/gui_play"
