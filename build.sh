#!/usr/bin/env bash
# Fallback build script for systems without CMake. Produces build/play
# (interactive demo) and build/run_tests (automated tests).
set -e
cd "$(dirname "$0")"

mkdir -p build

SOURCES="src/engine/Board.cpp src/engine/Command.cpp src/engine/Game.cpp \
         src/chess/ChessPieces.cpp src/chess/ChessGame.cpp \
         src/checkers/CheckersPieces.cpp src/checkers/CheckersGame.cpp"

echo "Compiling engine..."
for f in $SOURCES; do
    obj="build/$(basename "${f%.cpp}").o"
    g++ -std=c++17 -O2 -Iinclude -c "$f" -o "$obj"
done

ar rcs build/libgame_engine.a build/*.o

echo "Building play (interactive demo)..."
g++ -std=c++17 -O2 -Iinclude -c src/main.cpp -o build/main.o
g++ build/main.o build/libgame_engine.a -o build/play

echo "Building run_tests..."
g++ -std=c++17 -O2 -Iinclude -c tests/EngineTests.cpp -o build/EngineTests.o
g++ build/EngineTests.o build/libgame_engine.a -o build/run_tests

echo ""
echo "Build complete."
echo "  Run the demo:  ./build/play"
echo "  Run tests:     ./build/run_tests"
